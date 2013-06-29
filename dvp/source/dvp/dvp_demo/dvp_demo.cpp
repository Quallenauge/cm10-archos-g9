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

/*! \file
 * \brief This is a complex example of how to use DVP to do vision processing
 * into a series of display buffers previewed on the dvp_display subsystem.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#if defined(DVP_USE_IMGLIB)
#include <imglib/dvp_kl_imglib.h>
#endif
#if defined(DVP_USE_VLIB)
#include <vlib/dvp_kl_vlib.h>
#endif
#include <dvp/VisionCam.h>
#include <dvp/dvp_display.h>

#if defined(PC)
#define DVP_DEMO_MAX_TIMEOUTS (10000)
#else
#define DVP_DEMO_MAX_TIMEOUTS (10)
#endif

typedef struct _dvp_demo_t {
#ifdef VCAM_AS_SHARED
    module_t            mod;
#endif
    uint32_t            numDisplayImages;
    VisionCamFactory_f  factory;
    VisionCam          *pCam;
    queue_t            *frameq;
    dvp_display_t      *dvpd;
    DVP_Image_t        *displays;      /**< Buffers from the DVP Display */
    DVP_Image_t        *subImages;     /**< Subsections of the Display Image */
    DVP_Image_t        *camImages;     /**< GCam Images */
    DVP_Image_t        *images;        /**< DVP Graph Images */
    DVP_Handle          dvp;
    DVP_KernelNode_t   *nodes;         /**< The Kernel Graph Nodes */
    DVP_KernelGraph_t  *graph;
    uint32_t            numNodes;
} DVP_Demo_t;

void DVPCallback(void *cookie, DVP_KernelGraph_t *graph, DVP_U32 sectionIndex, DVP_U32 numNodesExecuted)
{
    cookie = cookie; // warnings
    graph = graph; // warnings
    sectionIndex = sectionIndex; // warnings
    numNodesExecuted = numNodesExecuted; // warnings
    DVP_PRINT(DVP_ZONE_ALWAYS, "Cookie %p Graph %p Graph Section %u Completed %u nodes\n", cookie, graph, sectionIndex, numNodesExecuted);
}

void VisionCamCallback(VisionCamFrame * cameraFrame)
{
    DVP_Image_t *pImage = (DVP_Image_t *)cameraFrame->mFrameBuff;
    queue_t *frameq = (queue_t *)cameraFrame->mCookie;

    DVP_PRINT(DVP_ZONE_CAM, "Writing Frame %p into Queue %p\n", cameraFrame, frameq);
    DVP_PrintImage(DVP_ZONE_CAM, pImage);
    if (queue_write(frameq, true_e, &cameraFrame) == false_e)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Failed to write frame to queue\n");
    }
}

bool_e VisionCamInit(DVP_Demo_t *demo, VisionCam_e camType, uint32_t width, uint32_t height, uint32_t fps, uint32_t rotation, uint32_t color)
{
    int32_t ret = 0;
#ifdef VCAM_AS_SHARED
    demo->mod = module_load(CAMERA_NAME);
    if (demo->mod)
    {
        demo->factory = (VisionCamFactory_f)module_symbol(demo->mod, "VisionCamFactory");
        if (demo->factory)
        {
            demo->pCam = demo->factory(VISIONCAM_OMX);
            if (demo->pCam)
            {
#else
    demo->pCam = VisionCamFactory(camType);
    if (demo->pCam)
    {
#endif
        VisionCamSensorSelection sensorIndex = VCAM_SENSOR_SECONDARY;
#if defined(DUCATI_1_5) || defined(DUCATI_2_0)
        VisionCamCaptureMode capmode = VCAM_GESTURE_MODE;
#else
        VisionCamCaptureMode capmode = VCAM_VIDEO_NORMAL;
#endif
        VisionCamFlickerType flicker = FLICKER_60Hz;
        VisionCamFocusMode focus = VCAM_FOCUS_CONTROL_AUTO;
        VisionCamWhiteBalType white = VCAM_WHITE_BAL_CONTROL_AUTO;
        int32_t brightness = 50;

        // initialize the VisionCam
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->init(demo->frameq));

        // configure the parameters
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->setParameter(VCAM_PARAM_WIDTH, &width, sizeof(width)));
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->setParameter(VCAM_PARAM_HEIGHT, &height, sizeof(height)));
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->setParameter(VCAM_PARAM_COLOR_SPACE_FOURCC, &color, sizeof(color)));
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->setParameter(VCAM_PARAM_CAP_MODE, &capmode, sizeof(capmode)));
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->setParameter(VCAM_PARAM_SENSOR_SELECT, &sensorIndex, sizeof(sensorIndex)));
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->setParameter(VCAM_PARAM_FPS_FIXED, &fps, sizeof(fps)));
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->setParameter(VCAM_PARAM_FLICKER, &flicker, sizeof(flicker)));
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->setParameter(VCAM_PARAM_BRIGHTNESS, &brightness, sizeof(brightness)));
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->setParameter(VCAM_PARAM_AWB_MODE, &white, sizeof(white)));

        // configure the buffers (the first X images are for the camera)
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->useBuffers(demo->camImages, demo->numDisplayImages));

        // @todo BUG: Can't set rotation until after useBuffers
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->setParameter(VCAM_PARAM_ROTATION, &rotation, sizeof(rotation)));

        // register the callback
        VCAM_COMPLAIN_IF_FAILED(ret,  demo->pCam->enablePreviewCbk(VisionCamCallback));

        // start the preview
        VCAM_COMPLAIN_IF_FAILED(ret,  demo->pCam->sendCommand(VCAM_CMD_PREVIEW_START));

        // do the autofocus
        VCAM_COMPLAIN_IF_FAILED(ret,  demo->pCam->setParameter(VCAM_PARAM_DO_AUTOFOCUS, &focus, sizeof(focus)));

        return true_e;
    }

#ifdef VCAM_AS_SHARED
            }
        }
    }
#endif
    return false_e;
}

bool_e VisionCamDeinit(DVP_Demo_t *demo)
{
    int32_t ret = 0;
    if (demo->pCam)
    {
        // destroy the camera
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->sendCommand(VCAM_CMD_PREVIEW_STOP));
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->disablePreviewCbk(VisionCamCallback));
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->releaseBuffers());
        VCAM_COMPLAIN_IF_FAILED(ret, demo->pCam->deinit());
        delete demo->pCam;
        demo->pCam = NULL;
    }
#ifdef VCAM_AS_SHARED
    module_unload(demo->mod);
#endif
    if (ret != 0)
        return false_e;
    else
        return true_e;
}

bool_e SubsectionImage(DVP_Image_t *dispFrame, DVP_Image_t *image, uint32_t index)
{
    if (dispFrame->planes == image->planes &&
        dispFrame->color  == image->color)
    {
        uint32_t limit_i = dispFrame->width / image->width;
        uint32_t limit_j = dispFrame->height / image->height;
        uint32_t i = index % limit_i;
        uint32_t j = index / limit_i;

        DVP_PRINT(DVP_ZONE_ALWAYS, "Requested Index %u in Image of {%u,%u} (%ux%u => %ux%u)\n",index, i,j, dispFrame->width, dispFrame->height, image->width, image->height);
        DVP_PrintImage(DVP_ZONE_ALWAYS, dispFrame);
        if (j > limit_j)
            return false_e;
        else
        {
            uint32_t p = 0;

             // make sure th strides are transfered.
            image->y_stride = dispFrame->y_stride;

            // create each subimage plane from the display frame.
            for (p = 0; p < dispFrame->planes; p++)
            {
                uint32_t k = (j * (image->height * dispFrame->y_stride)) +
                             (i * (image->width * dispFrame->x_stride));
                image->pData[p] = &dispFrame->pData[p][k];
                image->pBuffer[p] = &dispFrame->pData[p][k];
                DVP_PrintImage(DVP_ZONE_ALWAYS, image);
            }
            return true_e;
        }
    }
    else
        return false_e;
}

int main(int argc, char *argv[])
{
    int ret = 0;
#if (defined(DVP_USE_VLIB) || defined(DVP_USE_YUV)) && defined(DVP_USE_IMGLIB)
    DVP_Demo_t *demo = (DVP_Demo_t *)calloc(1, sizeof(DVP_Demo_t));

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

    if (demo && argc >= 1)
    {
        uint32_t i,j,k,n;
        uint32_t display_width =  (argc > 1?atoi(argv[1]):640);
        uint32_t display_height = (argc > 2?atoi(argv[2]):480);
        uint32_t width =          (argc > 3?atoi(argv[3]):160);
        uint32_t height =         (argc > 4?atoi(argv[4]):120);
        uint32_t fps = 30;
        uint32_t numFrames =      (argc > 5?atoi(argv[5]):100); // how many frames to display before quitting
        uint32_t numSubImages = (display_width/width) * (display_height/height);
        uint32_t numGraphImages = 20; /// @note make sure this matches the numbers used below
        int32_t focusDepth = 10;
        int32_t frameLock = 100;
        demo->numDisplayImages = DVP_DISPLAY_NUM_BUFFERS - 1;
        uint32_t numImages = numSubImages * demo->numDisplayImages;
        VisionCam_e camType = VISIONCAM_OMX;

#if defined(PC)
        camType = VISIONCAM_USB;
#endif

        demo->frameq = queue_create(demo->numDisplayImages * VCAM_PORT_MAX, sizeof(VisionCamFrame *));
        demo->dvpd   = DVP_Display_Create(display_width, display_height, display_width, display_height, DVP_DISPLAY_WIDTH, DVP_DISPLAY_HEIGHT, display_width, display_height, 0, 0, FOURCC_UYVY, 0, DVP_DISPLAY_NUM_BUFFERS);
        demo->subImages = (DVP_Image_t *)calloc(numImages, sizeof(DVP_Image_t));
        demo->displays = (DVP_Image_t *)calloc(demo->numDisplayImages, sizeof(DVP_Image_t));
        demo->camImages = (DVP_Image_t *)calloc(demo->numDisplayImages, sizeof(DVP_Image_t));
        demo->images = (DVP_Image_t *)calloc(numGraphImages, sizeof(DVP_Image_t));
        demo->dvp = DVP_KernelGraph_Init();
        if (demo->frameq && demo->dvpd && demo->subImages && demo->displays && demo->camImages && demo->dvp && demo->images)
        {
            // initialize the display buffers
            for (n = 0; n < demo->numDisplayImages; n++)
            {
                DVP_Image_Init(&demo->displays[n], display_width, display_height, FOURCC_UYVY);
                DVP_Display_Alloc(demo->dvpd, &demo->displays[n]);
                DVP_Image_Alloc(demo->dvp, &demo->displays[n], (DVP_MemType_e)demo->displays[n].memType);
                DVP_Image_Init(&demo->camImages[n], width, height, FOURCC_UYVY);
                // Blank the Images
                for (i = 0; i < demo->displays[n].planes; i++)
                    for (j = 0; j < demo->displays[n].height; j++)
                        memset(DVP_Image_Addressing(&demo->displays[n], 0, j, i),
                               0x80,
                               DVP_Image_LineSize(&demo->displays[n], i));
                // initialize images which are the subsections of the display buffers
                for (i = 0; i < numSubImages; i++)
                {
                    uint32_t k = (n * numSubImages) + i;
                    DVP_Image_Init(&demo->subImages[k], width, height, FOURCC_UYVY);
                    SubsectionImage(&demo->displays[n], &demo->subImages[k], i);
                    if (i == 0)
                    {
                        // if this is the first index of the subsections,
                        // use this as the camera buffer
                        memcpy(&demo->camImages[n], &demo->subImages[k],sizeof(DVP_Image_t));
                    }
                }
            }

            // initialize the DVP Nodes and Graphs
            i = 0;
            uint32_t idx_a9, len_a9;
            uint32_t idx_dsp, len_dsp;
            uint32_t idx_m3, len_m3;
            uint32_t idx_conv, len_conv;
            uint32_t idx_mask_x, idx_scratch, idx_mask_y, idx_scratch2;

            // A9
            idx_a9 = i;
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800); // LUMA
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800); // EDGE SOBEL
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800); // EDGE PREWITT
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800); // EDGE SCHARR
            len_a9 = i - idx_a9;
            for (j = 0; j < len_a9; j++) {
                if (DVP_Image_Alloc(demo->dvp, &demo->images[idx_a9+j], DVP_MTYPE_MPUCACHED_VIRTUAL) == DVP_FALSE) {
                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR: Failed to allocate A9 image\n");
                }
            }

            // DSP
            idx_dsp = i;
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800); // LUMA
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800); // CONV 3x3 Gx
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800); // CONV 3x3 Gy
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800); // DILATE
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800); // IIR H
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800); // IIR V
            len_dsp = i - idx_dsp;
            for (j = 0; j < len_dsp; j++) {
#if defined(DVP_USE_TILER)
                if (DVP_Image_Alloc(demo->dvp, &demo->images[idx_dsp+j], DVP_MTYPE_MPUCACHED_1DTILED) == DVP_FALSE) {
                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR: Failed to allocate DSP image\n");
                }
#else
                if (DVP_Image_Alloc(demo->dvp, &demo->images[idx_dsp+j], DVP_MTYPE_DEFAULT) == DVP_FALSE) {
                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR: Failed to allocate DSP image\n");
                }
#endif
            }

            // SIMCOP
            idx_m3 = i;
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800); // LUMA
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800); // SOBEL Gx
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800); // SOBEL Gy
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800); // IIR H
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800); // IIR V
            len_m3 = i - idx_m3;
            for (j = 0; j < len_m3; j++) {
#if defined(DVP_USE_TILER)
                if (DVP_Image_Alloc(demo->dvp, &demo->images[idx_m3+j], DVP_MTYPE_MPUNONCACHED_2DTILED) == DVP_FALSE) {
                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR: Failed to allocate M3 image\n");
                }
#else
                if (DVP_Image_Alloc(demo->dvp, &demo->images[idx_m3+j], DVP_MTYPE_DEFAULT) == DVP_FALSE) {
                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR: Failed to allocate M3 image\n");
                }
#endif
            }

            idx_conv = i; // the display conversion images start here
            len_conv = i; // we want to convert all these images;

            // Mask & Scratch
            idx_mask_x = i;
            DVP_Image_Init(&demo->images[i++], 3, 3, FOURCC_Y800);
            DVP_Image_Alloc(demo->dvp, &demo->images[idx_mask_x], DVP_MTYPE_MPUCACHED_VIRTUAL);
            idx_mask_y = i;
            DVP_Image_Init(&demo->images[i++], 3, 3, FOURCC_Y800);
            DVP_Image_Alloc(demo->dvp, &demo->images[idx_mask_y], DVP_MTYPE_MPUCACHED_VIRTUAL);
            idx_scratch = i;
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800);
            DVP_Image_Alloc(demo->dvp, &demo->images[idx_scratch], DVP_MTYPE_MPUCACHED_VIRTUAL);
            idx_scratch2 = i;
            DVP_Image_Init(&demo->images[i++], width, height, FOURCC_Y800);
            DVP_Image_Alloc(demo->dvp, &demo->images[idx_scratch2], DVP_MTYPE_MPUCACHED_VIRTUAL);

            // fill in the mask with the SOBEL X Gradient edge filter
            demo->images[idx_mask_x].pData[0][0] = (uint8_t)-1;
            demo->images[idx_mask_x].pData[0][1] = (uint8_t) 0;
            demo->images[idx_mask_x].pData[0][2] = (uint8_t) 1;
            demo->images[idx_mask_x].pData[0][3] = (uint8_t)-2;
            demo->images[idx_mask_x].pData[0][4] = (uint8_t) 0;
            demo->images[idx_mask_x].pData[0][5] = (uint8_t) 2;
            demo->images[idx_mask_x].pData[0][6] = (uint8_t)-1;
            demo->images[idx_mask_x].pData[0][7] = (uint8_t) 0;
            demo->images[idx_mask_x].pData[0][8] = (uint8_t) 1;

            // fill in the mask with the SOBEL Y Gradient edge filter
            demo->images[idx_mask_y].pData[0][0] = (uint8_t)-1;
            demo->images[idx_mask_y].pData[0][1] = (uint8_t)-2;
            demo->images[idx_mask_y].pData[0][2] = (uint8_t)-1;
            demo->images[idx_mask_y].pData[0][3] = (uint8_t) 0;
            demo->images[idx_mask_y].pData[0][4] = (uint8_t) 0;
            demo->images[idx_mask_y].pData[0][5] = (uint8_t) 0;
            demo->images[idx_mask_y].pData[0][6] = (uint8_t) 1;
            demo->images[idx_mask_y].pData[0][7] = (uint8_t) 2;
            demo->images[idx_mask_y].pData[0][8] = (uint8_t) 1;

            demo->numNodes = len_a9 + len_dsp + len_m3 + len_conv;

            DVP_PRINT(DVP_ZONE_ALWAYS, "Allocating %u %ux%u images\n", demo->numNodes, width, height);

            // Allocate the Nodes
            demo->nodes = DVP_KernelNode_Alloc(demo->dvp, demo->numNodes);
            if (demo->nodes == NULL)
                return STATUS_NOT_ENOUGH_MEMORY;

            DVP_KernelGraphSection_t sections[] = {
                {&demo->nodes[idx_a9], len_a9, DVP_PERF_INIT, DVP_CORE_LOAD_INIT, DVP_FALSE},
                {&demo->nodes[idx_dsp], len_dsp, DVP_PERF_INIT, DVP_CORE_LOAD_INIT, DVP_FALSE},
                {&demo->nodes[idx_m3], len_m3, DVP_PERF_INIT, DVP_CORE_LOAD_INIT, DVP_FALSE},
                {&demo->nodes[idx_conv], len_conv, DVP_PERF_INIT, DVP_CORE_LOAD_INIT, DVP_FALSE},
            };
            DVP_U32 order[] = {0,0,0,1}; // 3 parallel then 1 series
            DVP_KernelGraph_t graph = {
                sections,
                dimof(sections),
                order,
                DVP_PERF_INIT,
                DVP_FALSE,
            };
            DVP_Transform_t *io = NULL;
            DVP_Morphology_t *morph = NULL;
            DVP_ImageConvolution_t *img = NULL;
            DVP_IIR_t *iir = NULL;
            demo->graph = &graph;

            i = 0;
            // Now initialize the node structures for exactly what we want

            // A9 Section
            demo->nodes[i].header.kernel = DVP_KN_XYXY_TO_Y800;
            demo->nodes[i].header.affinity = DVP_CORE_CPU;
            io = dvp_knode_to(&demo->nodes[i], DVP_Transform_t);
            io->input = demo->camImages[0];
            io->output = demo->images[idx_a9];
            i++;

            demo->nodes[i].header.kernel = DVP_KN_SOBEL_8;
            demo->nodes[i].header.affinity = DVP_CORE_CPU;
            io = dvp_knode_to(&demo->nodes[i], DVP_Transform_t);
            io->input = demo->images[idx_a9];
            io->output = demo->images[idx_a9+1];
            i++;

            demo->nodes[i].header.kernel = DVP_KN_PREWITT_8;
            demo->nodes[i].header.affinity = DVP_CORE_CPU;
            io = dvp_knode_to(&demo->nodes[i], DVP_Transform_t);
            io->input = demo->images[idx_a9];
            io->output = demo->images[idx_a9+2];
            i++;

            demo->nodes[i].header.kernel = DVP_KN_SCHARR_8;
            demo->nodes[i].header.affinity = DVP_CORE_CPU;
            io = dvp_knode_to(&demo->nodes[i], DVP_Transform_t);
            io->input = demo->images[idx_a9];
            io->output = demo->images[idx_a9+3];
            i++;

            // DSP Section
            demo->nodes[i].header.kernel = DVP_KN_XYXY_TO_Y800;
            demo->nodes[i].header.affinity = DVP_CORE_DSP;
            io = dvp_knode_to(&demo->nodes[i], DVP_Transform_t);
            io->input = demo->camImages[0];
            io->output = demo->images[idx_dsp];
            i++;

            demo->nodes[i].header.kernel = DVP_KN_IMG_CONV_3x3;
            demo->nodes[i].header.affinity = DVP_CORE_DSP;
            img = dvp_knode_to(&demo->nodes[i], DVP_ImageConvolution_t);
            img->input = demo->images[idx_dsp];
            img->output = demo->images[idx_dsp+1];
            img->mask = demo->images[idx_mask_x];
            i++;

            demo->nodes[i].header.kernel = DVP_KN_IMG_CONV_3x3;
            demo->nodes[i].header.affinity = DVP_CORE_DSP;
            img = dvp_knode_to(&demo->nodes[i], DVP_ImageConvolution_t);
            img->input = demo->images[idx_dsp+1];
            img->output = demo->images[idx_dsp+2];
            img->mask = demo->images[idx_mask_y];
            i++;

            demo->nodes[i].header.kernel = DVP_KN_DILATE_CROSS;
            demo->nodes[i].header.affinity = DVP_CORE_DSP;
            morph = dvp_knode_to(&demo->nodes[i], DVP_Morphology_t);
            morph->input = demo->images[idx_dsp+2];
            morph->output = demo->images[idx_dsp+3];
            i++;

            demo->nodes[i].header.kernel = DVP_KN_IIR_HORZ;
            demo->nodes[i].header.affinity = DVP_CORE_DSP;
            iir = dvp_knode_to(&demo->nodes[i], DVP_IIR_t);
            iir->input = demo->images[idx_dsp];
            iir->output = demo->images[idx_dsp+4];
            iir->scratch = demo->images[idx_scratch];
            iir->weight = 2000;
            i++;

            demo->nodes[i].header.kernel = DVP_KN_IIR_VERT;
            demo->nodes[i].header.affinity = DVP_CORE_DSP;
            iir = dvp_knode_to(&demo->nodes[i], DVP_IIR_t);
            iir->input = demo->images[idx_dsp];
            iir->output = demo->images[idx_dsp+5];
            iir->scratch = demo->images[idx_scratch];
            iir->weight = 2000;
            i++;

            // SIMCOP Section
            demo->nodes[i].header.kernel = DVP_KN_XYXY_TO_Y800;
            demo->nodes[i].header.affinity = DVP_CORE_SIMCOP;
            io = dvp_knode_to(&demo->nodes[i], DVP_Transform_t);
            io->input = demo->camImages[0];
            io->output = demo->images[idx_m3];
            i++;

            demo->nodes[i].header.kernel = DVP_KN_IMG_CONV_3x3;
            demo->nodes[i].header.affinity = DVP_CORE_SIMCOP;
            img = dvp_knode_to(&demo->nodes[i], DVP_ImageConvolution_t);
            img->input = demo->images[idx_m3];
            img->output = demo->images[idx_m3+1];
            img->mask = demo->images[idx_mask_x];
            i++;

            demo->nodes[i].header.kernel = DVP_KN_IMG_CONV_3x3;
            demo->nodes[i].header.affinity = DVP_CORE_SIMCOP;
            img = dvp_knode_to(&demo->nodes[i], DVP_ImageConvolution_t);
            img->input = demo->images[idx_m3+1];
            img->output = demo->images[idx_m3+2];
            img->mask = demo->images[idx_mask_x];
            i++;

            demo->nodes[i].header.kernel = DVP_KN_IIR_HORZ;
            demo->nodes[i].header.affinity = DVP_CORE_SIMCOP;
            iir = dvp_knode_to(&demo->nodes[i], DVP_IIR_t);
            iir->input = demo->images[idx_m3];
            iir->output = demo->images[idx_m3+3];
            iir->scratch = demo->images[idx_scratch2];
            iir->weight = 2000;
            i++;

            demo->nodes[i].header.kernel = DVP_KN_IIR_VERT;
            demo->nodes[i].header.affinity = DVP_CORE_SIMCOP;
            iir = dvp_knode_to(&demo->nodes[i], DVP_IIR_t);
            iir->input = demo->images[idx_m3];
            iir->output = demo->images[idx_m3+4];
            iir->scratch = demo->images[idx_scratch2];
            iir->weight = 2000;
            i++;

            // CONVERSION for Display Graph
            for (j = i, k = 0; j < demo->numNodes; j++, k++) {
                demo->nodes[j].header.kernel = DVP_KN_Y800_TO_XYXY;
                demo->nodes[j].header.affinity = DVP_CORE_CPU;
                io = dvp_knode_to(&demo->nodes[i], DVP_Transform_t);
                io->input = demo->images[k];
                //io->output = demo->subImages[k]; // this will get replaced as soon as a buffer is returned
            }

            // initialize the camera
            if (VisionCamInit(demo, camType, width, height, fps, 0, FOURCC_UYVY))
            {
                VisionCamFrame *cameraFrame = NULL;
                DVP_Image_t *pImage = NULL;
                uint32_t recvFrames = 0;
                uint32_t timeouts = 0;

                thread_msleep(1000/fps); // wait 1 frame period.

                DVP_PRINT(DVP_ZONE_ALWAYS, "VisionCam is initialized, entering queue read loop!\n");
                // read from the queue and display the images
                do {
                    bool_e ret = queue_read(demo->frameq, false_e, &cameraFrame);
                    if (ret == true_e && cameraFrame != NULL)
                    {
                        uint32_t idx_disp = 0;

                        pImage = (DVP_Image_t *)cameraFrame->mFrameBuff;

                        timeouts = 0;

                        DVP_PRINT(DVP_ZONE_ALWAYS, "Received Frame %p (%p) from camera\n", pImage, pImage->pData[0]);

                        // match the pImage with a displays
                        for (idx_disp = 0; idx_disp < demo->numDisplayImages; idx_disp++)
                            if (pImage->pData[0] == demo->camImages[idx_disp].pData[0])
                                break;

                        DVP_PRINT(DVP_ZONE_ALWAYS, "Image Correlates to Display Buffer %u (%p->%p)\n", idx_disp, &demo->displays[idx_disp], demo->displays[idx_disp].pData[0]);

                        // update the DVP Graphs with the new camera image
                        dvp_knode_to(&demo->nodes[idx_a9], DVP_Transform_t)->input = *pImage;
                        dvp_knode_to(&demo->nodes[idx_dsp], DVP_Transform_t)->input = *pImage;
                        dvp_knode_to(&demo->nodes[idx_m3], DVP_Transform_t)->input = *pImage;

                        // update the conversion array
                        for (i = 0; i < len_conv; i++) {
                            // add one to the subImages index to skip the camera preview in that
                            // frame.
                            dvp_knode_to(&demo->nodes[idx_conv+i], DVP_Transform_t)->output = demo->subImages[(idx_disp * numSubImages) + i + 1];
                        }

                        // run the DVP Kernel Graph
                        DVP_KernelGraph_Process(demo->dvp, demo->graph, demo, DVPCallback);

                        // update the display
                        DVP_Display_Render(demo->dvpd, &demo->displays[idx_disp]);

                        demo->pCam->returnFrame(cameraFrame);

                        recvFrames++;
                        if (recvFrames > numFrames)
                            break;
                        if (focusDepth >= 0) {
                            if (recvFrames == fps) { // after 1 second
                                demo->pCam->setParameter(VCAM_PARAM_DO_MANUALFOCUS, &focusDepth, sizeof(focusDepth));
                            }
                        }
                        if (frameLock > 0) {
                            if (recvFrames == (uint32_t)frameLock) {
                                bool_e lock = true_e;
                                demo->pCam->sendCommand(VCAM_CMD_LOCK_AE, &lock, sizeof(lock));
                                demo->pCam->sendCommand(VCAM_CMD_LOCK_AWB, &lock, sizeof(lock));
                            }
                        }
                    }
                    else
                    {
                        DVP_PRINT(DVP_ZONE_ERROR, "Timedout waiting for buffer from Camera!\n");
                        timeouts++;
                        thread_msleep(1000/fps);
                    }
                } while (timeouts < DVP_DEMO_MAX_TIMEOUTS);
            }
            else
            {
                DVP_PRINT(DVP_ZONE_ERROR, "DVP_DEMO Failed during camera initialization\n");
                ret = STATUS_NO_RESOURCES;
            }
            DVP_PrintPerformanceGraph(demo->dvp, demo->graph);
            DVP_KernelNode_Free(demo->dvp, demo->nodes, demo->numNodes);
            VisionCamDeinit(demo);
        }
        else
        {
            DVP_PRINT(DVP_ZONE_ERROR, "DVP_DEMO Failed during data structure initialization\n");
        }
        if (demo->dvp) {
            DVP_KernelGraph_Deinit(demo->dvp);
        }
        if (demo->camImages)
            free(demo->camImages);
        if (demo->displays) {
            for (n = 0; n < demo->numDisplayImages; n++) {
                DVP_Display_Free(demo->dvpd, &demo->displays[n]);
            }
            free(demo->displays);
        }
        if (demo->subImages)
            free(demo->subImages);
        if (demo->images)
            free(demo->images);
        if (demo->dvpd)
            DVP_Display_Destroy(&demo->dvpd);
        if (demo->frameq)
            queue_destroy(demo->frameq);
    }
#else
    DVP_PRINT(DVP_ZONE_ERROR, "Required libraries are not present!\n");
    argc |= 1;
    argv[0] = argv[0];
    ret = -1;
#endif
    return ret;
}
