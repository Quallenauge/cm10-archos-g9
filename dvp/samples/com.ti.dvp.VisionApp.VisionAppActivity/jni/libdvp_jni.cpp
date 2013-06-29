/*
 *  Copyright (C) 2009-2011 Texas Instruments, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dvp/dvp.h>
#include <dvp/VisionCam.h>

#include <linux/binder.h>
#include <jni.h>
#include <android/log.h>
#include <android/native_window_jni.h>

#include <transformers.h>

#define TAG "DVP_JNI"
#define LOGD(string, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, string, ## __VA_ARGS__)
#define LOGE(string, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, string, ## __VA_ARGS__)
#define LOGI(string, ...) __android_log_print(ANDROID_LOG_INFO,  TAG, string, ## __VA_ARGS__)
#define LOGV(string, ...) __android_log_print(ANDROID_LOG_VERBOSE,  TAG, string, ## __VA_ARGS__)
#define LOG_ASSERT(condition, string) if (!condition) LOGE(string)

#define RGB_TO_Y(b, g, r, y) \
    y=(uint8_t)(((uint32_t)(30*r) + (uint32_t)(59*g) + (uint32_t)(11*b))/100)

#define RGB_TO_YUV(b, g, r, y, u, v) \
    RGB_TO_Y(b, g, r, y); \
    u=(uint8_t)(((uint32_t)(-17*r) - (uint32_t)(33*g) + (uint32_t)(50*b)+12800)/100); \
    v=(uint8_t)(((uint32_t)(50*r) - (uint32_t)(42*g) - (uint32_t)(8*b)+12800)/100)

namespace android
{
    static int jniRegisterNativeMethods(JNIEnv* env,
                                         const char* className,
                                         const JNINativeMethod* gMethods,
                                         int numMethods)
    {
        jclass clazz;

        LOGV("Registering %s natives\n", className);
        clazz = env->FindClass(className);
        if (clazz == NULL) {
            LOGE("Native registration unable to find class '%s'\n", className);
            return -1;
        }
        if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
            LOGE("RegisterNatives failed for '%s'\n", className);
            return -1;
        }
        return 0;
    }

    static bool find_field(JNIEnv *env, const char *class_name, const char *field_name,
                          const char *field_type, jfieldID *jfield)
    {
        jclass clazz = env->FindClass(class_name);
        if (clazz == NULL) {
            LOGE("find_field: can't find class %s", class_name);
            return false;
        }

        jfieldID field = env->GetFieldID(clazz, field_name, field_type);
        if (field == NULL) {
            LOGE("find_field: can't find %s.%s", class_name, field_name);
            return false;
        }

        *jfield = field;

        return true;
    }

    static const char *className = "com/ti/dvp/VisionEngine";
    static uint32_t     m_camMin;
    static uint32_t     m_camMax;
    static uint32_t     m_camIdx;
    static fourcc_t     m_color;
    static uint32_t     m_width;
    static uint32_t     m_height;
    static DVP_Image_t *m_images = NULL;
    static uint32_t     m_numImages = 0;
    static DVP_KernelNode_t *m_nodes = NULL;
    static uint32_t 	m_numNodes = 0;
    static DVP_KernelGraph_t *graph = NULL;
    static uint32_t 	numSections = 0;
    static thread_t 	engine = 0;
    static event_t 		ready;
    static event_t 		complete;
    static bool_e  		m_nativeCamera = false_e;
    static bool_e 		m_firstNodeCopy = false_e;
    static event_t 		m_releaseCallback;
    static bool_e  		running = false_e;
    static ANativeWindow* m_window = NULL;
    static VisionCam* 	m_visionCam = NULL;
    static DVP_Handle 	m_dvpHandle = 0;

    static uint32_t counter = 0;

    static void GraphSectionCompleteCallback(void *cookie, DVP_KernelGraph_t *graph, DVP_U32 secIndex, DVP_U32 numNodesExecuted)
    {
        /*! \todo we can callback upper layers somwhow? */
        LOGI("%u Nodes in section %u completed in graph %p (cookie=%p)\n", numNodesExecuted, secIndex, graph, cookie);
        return;
    }

    static thread_ret_t threadEngine(void *arg)
    {
        DVP_Handle dvp = (DVP_Handle)arg;
        while (running == true_e)
        {
            if (event_wait(&ready, EVENT_FOREVER) == true_e && running == true_e)
            {
                event_reset(&ready);
                LOGD("Graph Processing!");
                DVP_U32 nodes = DVP_KernelGraph_Process(dvp, graph, NULL, GraphSectionCompleteCallback);
                LOGD("Graph Processed %u nodes!", nodes);
                event_set(&complete);
                if (m_nativeCamera)
                {
                    event_set(&m_releaseCallback);
                }
            }
            else
            {
                event_reset(&ready);
            }
        }
        thread_exit(0);
    }

    static void VisionCamCallback(VisionCamFrame *cameraFrame)
    {
        LOGD("VisionCamCallback: buffer= %p, size= %d",
             cameraFrame->mFrameBuff, cameraFrame->mLength);

        uint32_t bpp = 2;
        uint8_t *buf = (uint8_t *)cameraFrame->mFrameBuff;

        for (int idx = 0 ; idx < cameraFrame->mDetectedFacesNum ; idx++ )
        {
            LOGD("****** VisionCamCallback: Face %d at %d,%d %d,%d \n",
                   idx,
                   cameraFrame->mFaces[idx].mFacesCoordinates.mLeft,
                   cameraFrame->mFaces[idx].mFacesCoordinates.mTop,
                   cameraFrame->mFaces[idx].mFacesCoordinates.mWidth,
                   cameraFrame->mFaces[idx].mFacesCoordinates.mHeight);
        }

        DVP_Transform_t *pIO;
        if (m_firstNodeCopy == false_e)
        {
            pIO = dvp_knode_to(&m_nodes[0], DVP_Transform_t);
            pIO->input.pBuffer[0] = buf;

#if defined(DVP_USE_CAMERA_SERVICE)
            if (DVP_Image_Alloc(m_dvpHandle, &pIO->input, DVP_MTYPE_CAMERA_1DTILED) == DVP_FALSE)
#else
            if (DVP_Image_Alloc(m_dvpHandle, &pIO->input, DVP_MTYPE_DEFAULT) == DVP_FALSE)
#endif
            {
                LOGE("VisionCamCallback: Failed to allocate image[%u]", 0);
            }
        }
        else
        {
            memcpy(m_images[m_camIdx].pData[0], buf, cameraFrame->mLength);
        }

        m_visionCam->returnFrame(cameraFrame);

        event_set(&ready);

        event_wait(&m_releaseCallback, 3000);
        event_reset(&m_releaseCallback);

        if (m_firstNodeCopy == false_e)
        {
            if (DVP_Image_Free(m_dvpHandle, &pIO->input) == DVP_FALSE)
            {
                LOGE("VisionCamCallback: Failed to free image[%u]", 0);
            }
        }
    }

    static jint Init(JNIEnv *env __attribute__ ((unused)),
                     jobject obj __attribute__ ((unused)),
                     jboolean initCamera)
    {
        DVP_Handle dvp = DVP_KernelGraph_Init();
        LOGD("DVP is initialized!");

        m_nativeCamera = (bool_e)initCamera;
        if (m_nativeCamera)
        {
#if defined(DVP_USE_CAMERA_SERVICE)
            m_visionCam = VisionCamFactory(VISIONCAM_CS);
#else
			char filename[MAX_PATH];
			char path[MAX_PATH];
			sprintf(filename, "handqvga");
			sprintf(path, "/sdcard/raw/");
			m_visionCam = VisionCamFactory(VISIONCAM_FILE);
#endif
            m_visionCam->init(NULL);
            m_visionCam->enablePreviewCbk(VisionCamCallback);

#if !defined(DVP_USE_CAMERA_SERVICE)
			m_visionCam->setParameter(VCAM_PARAM_PATH, path, sizeof(path));
			m_visionCam->setParameter(VCAM_PARAM_NAME, filename, sizeof(filename));
#endif
            event_init(&m_releaseCallback, false_e);
        }
        else
        {
            m_firstNodeCopy = true_e;
        }

        jint handle = (jint)dvp;
        running = true_e;
        event_init(&ready, false_e);
        event_init(&complete, true_e);
        engine = thread_create(threadEngine, (void *)dvp);
        return handle;
    }

    static void Finalize(JNIEnv *env __attribute__ ((unused)),
                         jobject obj __attribute__ ((unused)),
                         jint handle)
    {
        DVP_Handle dvp = (DVP_Handle)handle;
        running = false_e;
        event_set(&ready);
        event_set(&complete);
        thread_join(engine);
        event_deinit(&ready);
        event_deinit(&complete);

        if (m_nativeCamera)
        {
            event_set(&m_releaseCallback);
            event_deinit(&m_releaseCallback);
        }

        engine = 0;
        DVP_KernelGraph_Deinit(dvp);
        if (m_window)
        {
            ANativeWindow_release(m_window);
            m_window = NULL;
        }
    }

    static jboolean GraphTeardown(JNIEnv *env __attribute__ ((unused)),
                                 jobject clazz __attribute__ ((unused)),
                                 jint handle,
                                 jint graphEnum)
    {
        // free all the memory and resources.
        DVP_Handle dvp = (DVP_Handle)handle;

        // stop the thread in the engine.
        running = false_e;
        event_set(&ready);

        switch (graphEnum)
        {
            default:
                break;
        }

        DVP_PrintPerformanceGraph(handle, graph);

        if (m_images)
        {
            for (uint32_t i = m_camIdx; i < m_numImages; i++)
                DVP_Image_Free(handle, &m_images[i]);
            free(m_images);
            m_images = NULL;
        }
        if (m_nodes)
        {
            DVP_KernelNode_Free(dvp, m_nodes, m_numNodes);
            m_nodes = NULL;
        }
        if (graph)
        {
            if (graph->sections)
                free(graph->sections);
            if (graph->order)
                free(graph->order);
            free(graph);
            graph = NULL;
        }
        LOGD("GraphTeardown complete!");
        return true;
    }

    static jboolean GraphSetup(JNIEnv *env,
                               jobject clazz,
                               jint handle,
                               jint graphEnum,
                               jint width,
                               jint height,
                               jint format)
    {
        DVP_Handle dvp = (DVP_Handle)handle;
        jboolean constructed = true;
        DVP_Transform_t *pIO = NULL;

        m_dvpHandle = dvp;

        m_camMin = 0;
        m_camMax = 4;
        m_camIdx = 2;

        m_width = width;
        m_height = height;

        switch (format)
        {
            case 0x10: // NV16
                return false;

            case 0x11: // NV21
                m_color = FOURCC_NV21;
                break;
            case 0x100: // NV12
                m_color = FOURCC_NV12;
                break;
            case 0x32315659: // YV12
                m_color = FOURCC_YV12;
                break;
            case 0x4:  // RGB565
                m_color = FOURCC_RGB565;
                break;
            case 0x14: // YUY2
                m_color = FOURCC_YUY2;
                break;
        }

        LOGD("Constructing Graph %d %ux%u:0x%x", graphEnum, m_width, m_height, format);

        // setup information
        switch (graphEnum)
        {
            default:
                numSections = 1;
                m_numNodes = 4;
                m_numImages = 4 + m_camMax - m_camMin;
                break;
        }
        // construct the graph
        graph = (DVP_KernelGraph_t *)calloc(1, sizeof(DVP_KernelGraph_t));
        if (graph == NULL)
            constructed = false;
        else
        {
            graph->numSections = numSections;
            graph->sections = (DVP_KernelGraphSection_t *)calloc(numSections, sizeof(DVP_KernelGraphSection_t));
            if (graph->sections == NULL)
                constructed = false;
            else
            {
                graph->order = (DVP_U32 *)calloc(numSections, sizeof(DVP_U32));
                if (graph->order == NULL)
                    constructed = false;
                else
                {
                    DVP_Perf_Clear(&graph->totalperf);
                }
            }
        }

        if (constructed)
            LOGD("Graph and Sections Allocated!");

        m_nodes = DVP_KernelNode_Alloc(dvp, m_numNodes);
        if (m_nodes == NULL)
            constructed = false;

        if (constructed)
            LOGD("%u Graph Nodes Allocated!", m_numNodes);

        // allocate meta info structures
        m_images = (DVP_Image_t *)calloc(m_numImages, sizeof(DVP_Image_t));
        if (m_images == NULL)
            constructed = false;

        if (constructed)
            LOGD("%d Image Structures Allocated", m_numImages);

        if (constructed == true)
        {
            switch (graphEnum)
            {
                default:
                    graph->sections[0].pNodes = &m_nodes[0];
                    graph->sections[0].numNodes = m_numNodes;
                    graph->order[0] = 0;

                    for (int index = m_camMin; index < m_camMax; index++)
                        DVP_Image_Init(&m_images[index], width, height, m_color);

                    DVP_Image_Init(&m_images[m_camMax+0], m_width, m_height, FOURCC_Y800);
                    DVP_Image_Init(&m_images[m_camMax+1], m_width, m_height, FOURCC_Y800);
                    DVP_Image_Init(&m_images[m_camMax+2], m_width, m_height, FOURCC_UYVY);
                    DVP_Image_Init(&m_images[m_camMax+3], m_width, m_height, FOURCC_BGR);

                    for (uint32_t i = m_camIdx; i < m_numImages; i++)
                    {
                        if (DVP_Image_Alloc(dvp, &m_images[i], DVP_MTYPE_DEFAULT) == DVP_FALSE)
                        {
                            LOGE("Failed to allocate image[%u]", i);
                        }
                    }

                    LOGD("%u Images Allocated!", m_numImages);

                    m_nodes[0].header.kernel = DVP_KN_YXYX_TO_Y800;
                    pIO = dvp_knode_to(&m_nodes[0], DVP_Transform_t);
                    if (m_firstNodeCopy)
                    {
                        pIO->input = m_images[m_camIdx];
                    }
                    else
                    {
                        m_nodes[0].header.affinity = DVP_CORE_CPU;
                        pIO->input = m_images[m_camMin];
                    }
                    pIO->output = m_images[m_camMax];

                    m_nodes[1].header.kernel = DVP_KN_SCHARR_8;
                    pIO = dvp_knode_to(&m_nodes[1], DVP_Transform_t);
                    pIO->input = m_images[m_camMax];
                    pIO->output = m_images[m_camMax+1];

                    m_nodes[2].header.kernel = DVP_KN_Y800_TO_XYXY;
                    pIO = dvp_knode_to(&m_nodes[2], DVP_Transform_t);
                    pIO->input = m_images[m_camMax+1];
                    pIO->output = m_images[m_camMax+2];

                    m_nodes[3].header.kernel = DVP_KN_UYVY_TO_BGR;
                    pIO = dvp_knode_to(&m_nodes[3], DVP_Transform_t);
                    pIO->input = m_images[m_camMax+2];
                    pIO->output = m_images[m_camMax+3];

                    LOGD("DVP Graph Initialized!");
                    break;
            }
        }
        else
        {
            GraphTeardown(env, clazz, handle, graphEnum);
        }
        return constructed;
    }

    static jboolean GraphUpdate(JNIEnv *env __attribute__ ((unused)),
                                jobject clazz __attribute__ ((unused)),
                                jint handle,
                                jint graphEnum,
                                jbyteArray buffer,
                                jint width,
                                jint height,
                                jint format)
    {
        DVP_Handle dvp = (DVP_Handle)handle;
        uint32_t index = 0;
        uint32_t bpp = 1;
        uint8_t *buf = NULL;
        jboolean isCopy;

        if (running == false_e)
        {
            LOGI("GraphUpdate: Graph is tearing down!\n");
            return false_e;
        }

        LOGD("GraphUpdate %p, %ux%u:%x\n", buffer, width, height, format);

        switch (format)
        {
            case 0x10: // NV16
            case 0x11: // NV21
            case 0x100: // NV12
            case 0x32315659: // YV12
                bpp = 1;
                break;
            case 0x4:  // RGB565
            case 0x14: // YUY2
                bpp = 2;
                break;
        }

        switch (graphEnum)
        {
            default:
                index = m_camIdx;
                break;
        }

        buf = (uint8_t *)env->GetByteArrayElements(buffer, &isCopy);

        memcpy(m_images[m_camIdx].pData[0], buf, width*bpp*height);

        env->ReleaseByteArrayElements(buffer, (jbyte *)buf, JNI_ABORT);

        event_set(&ready);
        return true;
    }

    static void WaitForComplete(JNIEnv *env __attribute__ ((unused)),
                                jobject clazz __attribute__ ((unused)))
    {
        event_wait(&complete, EVENT_FOREVER);

        if (running == false_e)
        {
            LOGI("WaitForComplete: Graph is tearing down!\n");
            return;
        }

        if (m_window == NULL)
        {
            LOGI("WaitForComplete: m_window is null!\n");
            return;
        }

        ANativeWindow_Buffer buff;
        if (!ANativeWindow_lock(m_window, &buff, NULL))
        {
            uint32_t index = m_camMax + 3; //m_camIdx, m_camMax

            // FOURCC_BGR -> FOURCC_NV12
            bgr_to_nv12(&buff, &m_images[index]);

            ANativeWindow_unlockAndPost(m_window);
        }
        else
        {
            LOGE("WaitForComplete: ANativeWindow_lock failed!\n");
        }
    }

    static void CancelWaitForComplete(JNIEnv *env __attribute__ ((unused)),
                                      jobject clazz __attribute__ ((unused)))
    {
        event_set(&complete);
    }

    static jboolean SetPreviewSurface(JNIEnv *env,
                                      jobject thiz __attribute__ ((unused)),
                                      jobject jSurface,
                                      jint width,
                                      jint height,
                                      jint format)
    {
        LOGD("SetPreviewSurface: %ux%u:%x\n", width, height, format);

        if (m_window)
        {
            LOGI("SetPreviewSurface: Releasing already acquired surface window 0x%x.\n", m_window);
            m_window = NULL;
        }

        m_window = ANativeWindow_fromSurface(env, jSurface);
        if (m_window == NULL)
        {
            LOGE("SetPreviewSurface: ANativeWindow_fromSurface failed!\n");
            return false;
        }

        int32_t err = ANativeWindow_setBuffersGeometry(m_window,
                                                       (int32_t)width,
                                                       (int32_t)height,
                                                       (int32_t)format);
        if (err)
        {
            LOGE("SetPreviewSurface: ANativeWindow_setBuffersGeometry failed! Error = %d\n", err);
            ANativeWindow_release(m_window);
            m_window = NULL;
            return false;
        }

        return true;
    }

    static jboolean SelectCamera(JNIEnv *env __attribute__ ((unused)),
                                 jobject thiz __attribute__ ((unused)),
                                 jint id)
    {
        LOGD("SelectCamera: camera id: %d\n", id);

        if (m_visionCam == NULL)
        {
            LOGE("SelectCamera: VisionCam is not initialized!\n");
            return false;
        }

        VisionCamSensorSelection sensor = (VisionCamSensorSelection)id;
        status_e err = m_visionCam->setParameter(VCAM_PARAM_SENSOR_SELECT, &sensor,
                                                 sizeof(VisionCamSensorSelection));
        if (err != STATUS_SUCCESS)
        {
            LOGE("SelectCamera: selecting sensor failed! Status= %d\n", err);
            return false;
        }

        return true;
    }

    static jboolean StartPreview(JNIEnv *env,
                                 jobject thiz __attribute__ ((unused)),
                                 jobject jSurfaceTexture)
    {
    	status_e err = STATUS_SUCCESS;
        if (m_visionCam == NULL)
        {
            LOGE("StartPreview: VisionCam is not initialized!\n");
            return false;
        }

        jfieldID surfaceTextureID;
        if (!find_field(env, "android/graphics/SurfaceTexture", "mSurfaceTexture", "I", &surfaceTextureID))
        {
            LOGE("StartPreview: Cannot get surface texture id!\n");
            return false;
        }

        jint surfaceTexture = env->GetIntField(jSurfaceTexture, surfaceTextureID);
        if (surfaceTexture == 0)
        {
            LOGE("StartPreview: Cannot get native surface texture!\n");
            return false;
        }

#if defined(DVP_USE_CAMERA_SERVICE)
        err = m_visionCam->setParameter(VCAM_PARAM_PREVIEW_TEXTURE,
                                                 (void *)surfaceTexture, sizeof(jint));
        if (err != STATUS_SUCCESS)
        {
            LOGE("StartPreview: setting preview display failed! Status= %d\n", err);
            return false;
        }

        VisionCamVarFramerateType frate = {24, 24};
        err = m_visionCam->setParameter(VCAM_PARAM_FPS_VAR, &frate,
                                        sizeof(VisionCamVarFramerateType));
        if (err != STATUS_SUCCESS)
        {
            LOGE("StartPreview: selecting sensor failed! Status= %d\n", err);
            return false;
        }
#else
		uint32_t rate = 30;
		err = m_visionCam->setParameter(VCAM_PARAM_FPS_FIXED, &rate, sizeof(rate));
		if (err != STATUS_SUCCESS)
		{
			LOGE("StartPreview: can't set fps! err=%d\n",err);
			return false_e;
		}
#endif

        err = m_visionCam->setParameter(VCAM_PARAM_WIDTH, &m_width,
                                        sizeof(uint32_t));
        if (err != STATUS_SUCCESS)
        {
            LOGE("StartPreview: setting width failed! Status= %d\n", err);
            return false;
        }

        err = m_visionCam->setParameter(VCAM_PARAM_HEIGHT, &m_height,
                                        sizeof(uint32_t));
        if (err != STATUS_SUCCESS)
        {
            LOGE("StartPreview: setting height failed! Status= %d\n", err);
            return false;
        }

        err = m_visionCam->setParameter(VCAM_PARAM_COLOR_SPACE_FOURCC, &m_color,
                                        sizeof(_fourcc));
        if (err != STATUS_SUCCESS)
        {
            LOGE("StartPreview: setting color space failed! Status= %d\n", err);
            return false;
        }

        err = m_visionCam->sendCommand(VCAM_CMD_PREVIEW_START, NULL, 0);
        if (err != STATUS_SUCCESS)
        {
            LOGE("StartPreview: starting preview failed! Status= %d\n", err);
            return false;
        }

        err = m_visionCam->sendCommand(VCAM_CMD_FACE_DETECTION, NULL, 0);
        if (err != STATUS_SUCCESS)
        {
            LOGE("StartPreview: starting face detection failed! Status= %d\n", err);
            return false;
        }

        return true;
    }

    static jboolean StopPreview(JNIEnv *env,
                                jobject thiz __attribute__ ((unused)))
    {
        if (m_visionCam == NULL)
        {
            LOGE("StopPreview: VisionCam is not initialized!\n");
            return false;
        }

        status_e err = m_visionCam->sendCommand(VCAM_CMD_PREVIEW_STOP, NULL, 0);
        if (err != STATUS_SUCCESS)
        {
            LOGE("StopPreview: stopping preview failed! Status= %d\n", err);
            return false;
        }

        return true;
    }

    static jbyteArray GetBuffers(JNIEnv *env __attribute__ ((unused)),
                                 jobject clazz __attribute__ ((unused)),
                                 jint buffID )
    {
        jbyteArray result = NULL;

        if (buffID > m_numImages)
        {
            LOGE("GetBuffers: buffer with ID %d was not found.\n", buffID);
            return NULL;
        }

        result = env->NewByteArray(m_images[buffID].numBytes);
        if (result == NULL)
        {
            return NULL;
        }

        env->SetByteArrayRegion(result, 0, m_images[buffID].numBytes,
                               (jbyte *)m_images[buffID].pData[0]);

        return result;
    }

    static void DumpCameraParameters(JNIEnv *env __attribute__ ((unused)),
                                     jobject clazz __attribute__ ((unused)))
    {
        if (m_visionCam == NULL)
        {
            LOGE("DumpCameraParameters: VisionCam is not initialized!\n");
            return;
        }

        uint32_t uintValue;
        status_e err = m_visionCam->getParameter(VCAM_PARAM_WIDTH, &uintValue,
                                                 sizeof(uint32_t));
        if (err != STATUS_SUCCESS)
        {
            LOGE("DumpCameraParameters: getting width failed! Status= %d\n", err);
        }
        LOGE("DumpCameraParameters: width= %d\n", uintValue);

        err = m_visionCam->getParameter(VCAM_PARAM_HEIGHT, &uintValue,
                                        sizeof(uint32_t));
        if (err != STATUS_SUCCESS)
        {
            LOGE("DumpCameraParameters: getting height failed! Status= %d\n", err);
        }
        LOGE("DumpCameraParameters: height= %d\n", uintValue);

        VisionCamVarFramerateType frate = {0, 0};
        err = m_visionCam->getParameter(VCAM_PARAM_FPS_VAR, &frate,
                                        sizeof(VisionCamVarFramerateType));
        if (err != STATUS_SUCCESS)
        {
            LOGE("DumpCameraParameters: getting var FPS failed! Status= %d\n", err);
        }
        LOGE("DumpCameraParameters: var fps= %d:%d\n", frate.mMin, frate.mMax);

        _fourcc color;
        err = m_visionCam->getParameter(VCAM_PARAM_COLOR_SPACE_FOURCC, &color,
                                        sizeof(_fourcc));
        if (err != STATUS_SUCCESS)
        {
            LOGE("DumpCameraParameters: getting color space failed! Status= %d\n", err);
        }
        LOGE("DumpCameraParameters: color space= %s\n", fourcctostr(color));

        VisionCamSensorSelection sensor;
        err = m_visionCam->getParameter(VCAM_PARAM_SENSOR_SELECT, &sensor,
                                        sizeof(VisionCamSensorSelection));
        if (err != STATUS_SUCCESS)
        {
            LOGE("DumpCameraParameters: getting sensor failed! Status= %d\n", err);
        }
        LOGE("DumpCameraParameters: sensor= %d\n", sensor);

        int32_t intValue;
        err = m_visionCam->getParameter(VCAM_PARAM_CONTRAST, &intValue,
                                        sizeof(int32_t));
        if (err != STATUS_SUCCESS)
        {
            LOGE("DumpCameraParameters: getting contrast failed! Status= %d\n", err);
        }
        LOGE("DumpCameraParameters: contrast= %d\n", intValue);

        err = m_visionCam->getParameter(VCAM_PARAM_SHARPNESS, &intValue,
                                        sizeof(int32_t));
        if (err != STATUS_SUCCESS)
        {
            LOGE("DumpCameraParameters: getting sharpness failed! Status= %d\n", err);
        }
        LOGE("DumpCameraParameters: sharpness= %d\n", intValue);

        err = m_visionCam->getParameter(VCAM_PARAM_BRIGHTNESS, &intValue,
                                        sizeof(int32_t));
        if (err != STATUS_SUCCESS)
        {
            LOGE("DumpCameraParameters: getting brightness failed! Status= %d\n", err);
        }
        LOGE("DumpCameraParameters: brightness= %d\n", intValue);

        err = m_visionCam->getParameter(VCAM_PARAM_SATURATION, &intValue,
                                        sizeof(int32_t));
        if (err != STATUS_SUCCESS)
        {
            LOGE("DumpCameraParameters: getting saturation failed! Status= %d\n", err);
        }
        LOGE("DumpCameraParameters: saturation= %d\n", intValue);

        VisionCamFlickerType flicker;
        err = m_visionCam->getParameter(VCAM_PARAM_FLICKER, &flicker,
                                        sizeof(VisionCamFlickerType));
        if (err != STATUS_SUCCESS)
        {
            LOGE("DumpCameraParameters: getting flicker failed! Status= %d\n", err);
        }
        LOGE("DumpCameraParameters: flicker= %d\n", flicker);

        err = m_visionCam->getParameter(VCAM_PARAM_EXPOSURE_COMPENSATION, &intValue,
                                        sizeof(int32_t));
        if (err != STATUS_SUCCESS)
        {
            LOGE("DumpCameraParameters: getting exposure compensation failed! Status= %d\n", err);
        }
        LOGE("DumpCameraParameters: exposure compensation= %d\n", intValue);

    }

    static JNINativeMethod method_table[] = {
        // { name,                         signature, function_pointer }
        { "InitNative",                    "(Z)I",        (void *)Init },
        { "FinalizeNative",                "(I)V",        (void *)Finalize },
        { "GraphSetupNative",              "(IIIII)Z",    (void *)GraphSetup },
        { "GraphUpdateNative",             "(II[BIII)Z",  (void *)GraphUpdate },
        { "GraphTeardownNative",           "(II)Z",       (void *)GraphTeardown },
        { "WaitForCompleteNative",         "()V",         (void *)WaitForComplete },
        { "CancelWaitForCompleteNative",   "()V",         (void *)CancelWaitForComplete },
        { "DumpCameraParametersNative",    "()V",         (void *)DumpCameraParameters },
        { "SelectCameraNative",            "(I)Z",        (void *)SelectCamera },
        { "GetBuffersNative",              "(I)[B",       (void *)GetBuffers },
        { "StopPreviewNative",             "()Z",         (void *)StopPreview },
        { "StartPreviewNative",            "(Landroid/graphics/SurfaceTexture;)Z", (void *)StartPreview },
        { "SetPreviewSurfaceNative",       "(Landroid/view/Surface;III)Z", (void *)SetPreviewSurface },
    };

    static int register_jni(JNIEnv *env)
    {
        return jniRegisterNativeMethods(env, className, method_table, dimof(method_table));
    }
};

using namespace android;

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved __attribute__((unused)))
{
    JNIEnv* env = NULL;
    jint result = -1;
    int ret = 0;
    // the 2nd void cast is to bypass gcc warning message.  Alternately,
    // the '-fno-strict-aliasing' flag can also be used at the expense of
    // preventing some optimizations.
    if (vm->GetEnv((void**) (void*) &env, JNI_VERSION_1_6) != JNI_OK) {
        LOGE("DVP: GetEnv failed!");
        return result;
    }
    LOG_ASSERT(env, "DVP: Could not retrieve the env!");
    ret = register_jni(env);
    return JNI_VERSION_1_6;
}

