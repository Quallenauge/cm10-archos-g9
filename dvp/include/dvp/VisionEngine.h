/*
 *  Copyright (C) 2009-2012 Texas Instruments, Inc.
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

#ifndef _VISION_ENGINE_H_
#define _VISION_ENGINE_H_

#include <sosal/sosal.h>

#include <dvp/dvp.h>
#include <dvp/dvp_debug.h>
#include <dvp/dvp_display.h>

#include <dvp/ImageDebugging.h>
#include <sosal/CThreaded.h>
#include <dvp/VisionCam.h>

#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))

#include <binder/IBinder.h>
#include <binder/BinderService.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <hardware/hardware.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/String16.h>

#include <dvp/anativewindow.h>
using namespace android;

#endif

/** This structure correlates the array indices of m_images back to the camera port they are assigned to. */
typedef struct _vcam_port_desc_t {
    VisionCamPort_e port;
    uint32_t width;
    uint32_t height;
    fourcc_t color;
    uint32_t fps;
    VisionCamRotation_e rotate;
    VisionCamMirrorType mirror;
    uint32_t minIdx;
    uint32_t curIdx;
    uint32_t maxIdx;
    uint32_t req_width; //Output - width+padding needed by the camera
    uint32_t req_height;//Output - height+padding needed by the camera
} VisionCamPortDesc_t;

/** This structure allows us to correlate a particular kernel graph with a particular port from the VisionCam */
typedef struct _kgraph_to_image_correlation_t {
    VisionCamPort_e   portIdx;
} KGraph_Image_Corr_t;

class VisionEngine : public CThreaded
{
protected:
    // Structural Variables
    bool_e              m_active;
    bool_e              m_processing;
    event_t             m_wait;
    queue_t            *m_framequeue;
    semaphore_t         m_engineLock;

    // Display Parameters
    dvp_display_t      *m_display;       /**< The DVP Debug Display Interface */
    bool_e              m_display_enabled;
    uint32_t            m_display_width;
    uint32_t            m_display_height;
    uint32_t            m_display_rotation;
    bool_e              m_display_special;
#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))
    anativewindow_t    *m_anw;
    sp<IBinder>         m_surface;
    sp<ANativeWindow>   m_window;
#endif

    // Camera Parameters
    uint32_t            m_width;
    uint32_t            m_height;
    uint32_t            m_fps;
    uint32_t            m_fourcc;
    uint32_t            m_numFrames;        /**< The desired number of frames to capture. */
    uint32_t            m_capFrames;        /**< The actual number of frames captured */
    uint32_t            m_camera_rotation;
    uint32_t            m_droppedFrames;
    VisionCamSensorSelection m_sensorIndex;
    VisionCamWhiteBalType m_whitemode;
    VisionCamFlickerType m_flicker;
    VisionCamFocusMode m_focus;
    VisionCamCaptureMode m_capmode;
    bool_e              m_face_detect;
    bool_e              m_face_detect_raw;
    VisionCamStereoInfo m_stereo_info;
    uint32_t            m_focus_delay;
     int32_t            m_focusDepth;
    
    DVP_Perf_t          m_capPerf;    /**< Used to calculate the average capture rate */

    char                m_name[MAX_PATH]; /**< For file based cameras, this is the prefix of the filename */
    char                m_path[MAX_PATH]; /**< For file based cameras, this is the path to the file */

    DVP_Handle          m_hDVP;       /**< The handle to the DVP context */
    DVP_KernelGraph_t  *m_graphs;     /**< The pointer to the graph array */
    uint32_t            m_numGraphs;  /**< The number of graphs allocated in m_graphs */
    DVP_KernelNode_t   *m_pNodes;     /**< The pointer to the kernel node array */
    uint32_t            m_numNodes;   /**< The number of nodes in the kernel graph */
    DVP_Image_t        *m_images;     /**< The pointer to the allocated images */
    uint32_t            m_numImages;  /**< The number of images  */
    bool_e              m_imgdbg_enabled; /** Enables file writing */
    char                m_imgdbg_path[MAX_PATH]; /**< Path to write the files to */
    ImageDebug_t       *m_imgdbg;     /**< The pointer to the debugging structures for images */
    uint32_t            m_numImgDbg;  /**< The number of debugging images */
    DVP_Buffer_t       *m_buffers;    /**< The pointer to the allocated buffers */
    uint32_t            m_numBuffers; /**< The number of buffers */
    VisionCam          *m_pCam;       /**< The pointer to the camera object */
    uint32_t            m_camIdx;     /**< Indicates which buffer we're currently using for camera buffer */
    uint32_t            m_camMin;     /**< Inidactes the minimum index to give to the camrea. */
    uint32_t            m_camMax;     /**< Indicates how many total camera buffers we have */
    uint32_t            m_dispIdx;    /**< Indicates which buffer we're currently using for display */
    uint32_t            m_dispMin;    /**< Indicates the minimum index to display */
    uint32_t            m_dispMax;    /**< Indicates how many total display buffers we have */
    uint32_t            m_vidMin;     /**< Indicates the minimum index of the video port images */
    uint32_t            m_vidIdx;     /**< Indicates the current index of the video port image */
    uint32_t            m_vidMax;     /**< Indicates the maximum index of the video port images */
    uint32_t            m_procMin;    /**< Indicates the minimum index of the processing buffers/images */
    uint32_t            m_procIdx;    /**< Indicates the current index of the processing buffers/images */
    uint32_t            m_procMax;    /**< Indicates the maximum index of the processing buffers/images */
    FrameCallback_f     m_camcallback;
    FramePackCallback_f m_pack_callback;
    VisionCam_e         m_camtype;    /**< Indicates which GesureCam to attach to */
    KGraph_Image_Corr_t*m_correlation;/**< The allows us to match a graph to a camera port */
    uint32_t            m_numPorts;   /**< An indication of the number of ports enabled on the VisionCam */

public:
    /** Default Constructor */
    VisionEngine();

    /** Complex Constructor */
    VisionEngine(uint32_t width, uint32_t height, uint32_t fps, fourcc_t color, char *camName, uint32_t numFrames);

    /** Deconstructor */
    virtual ~VisionEngine();

    /** Thread Wrapper */
    thread_ret_t RunThread();

    /** This method causes the thread to start and the engine to run */
    virtual bool Startup();

    /** This method causes the Engine loop to exit */
    virtual void Shutdown();

    /** This is the main loop which processes the vision */
    virtual status_e Engine();

    /** This initalizes the VisionCam with a simple interface */
    virtual status_e CameraInit(void *cookie, fourcc_t color);

    /** This configures the VisionCam with a more complex set of parameters */
    virtual status_e CameraInit(void *cookie, VisionCamPortDesc_t *desc, uint32_t numDesc, bool_e sendBuffers = true_e, bool_e startCam = true_e);

    /** This sends all buffers to VisionCam */
    virtual status_e SendCameraBuffers(VisionCamPortDesc_t *desc, uint32_t numDesc);

    /** This starts VisionCam */
    virtual status_e StartCamera();

    /** This deinitializes the VisionCam */
    virtual status_e CameraDeinit();

    /** This method sets up the graph */
    virtual status_e GraphSetup() = 0;

    /** This method updates the DVP Kernel Graph with the delivered image */
    virtual status_e GraphUpdate(VisionCamFrame *cameraFrame) = 0;

    /** This method processes the DVP Kerel Graph */
    virtual DVP_U32 GraphExecute(DVP_KernelGraph_t *graph);

    /** This deinitalizes the DVP Kernel Graph */
    virtual status_e GraphTeardown();

    /** This method processes camera image */
    virtual status_e ProcessImage(VisionCamFrame * cameraFrame);

    /** The receives the image from the VisionCam and places it into an internal queue. */
    virtual void ReceiveImage(VisionCamFrame * cameraFrame);

    /** This removes the camera frame from the internal queue. Child classes can overload to change frame-dropper */
    virtual VisionCamFrame *DequeueImage();

    /** This method is run directly after the Kernel Graphs are executed */
    virtual status_e PostProcessImage(VisionCamFrame *cameraFrame, uint32_t numSections);

    /** This method allows the child classes to overload and implement their own focus settings */
    virtual status_e DelayedCameraFocusSetting();

    /** This method allows the child classes to overload image rendering */
    virtual status_e RenderImage(VisionCamFrame *cameraFrame);

    /** This method allow child classes to overload when the camera frame is returned to the camera */
    virtual status_e ReturnCameraFrame(VisionCamFrame *cameraFrame);

    /** This method is invoked by the DVP Section Completion Callback */
    virtual void GraphSectionComplete(DVP_KernelGraph_t * graph, DVP_U32 sectionIndex, DVP_U32 numNodesExecuted) = 0;

    /** This method allows the user to wait until all the frames are processed */
    bool WaitForCompletion(uint32_t timeout = EVENT_FOREVER);

    /** This method allows the user to limit the maximum capacity of a core */
    virtual void SetCoreCapacity(DVP_Core_e core, DVP_U32 limit);

    /** This method allows the user to remove any previously set limits on the core */
    virtual DVP_U32 GetCoreCapacity(DVP_Core_e core);

    // Support APIs

    /** This allows the engine to allocate images out of the shared region */
    bool AllocateImageStructs(uint32_t numImages);

    /** This frees the allocated images from the shared region */
    void FreeImageStructs();

    /** This allocates kernel nodes out of the shared region of memory */
    bool AllocateNodes(uint32_t numNodes);

    /** This frees the kernel nodes allocated out of the shared region of memory */
    void FreeNodes();

    /** This allocates a number of sections for the graph */
    bool AllocateSections(DVP_KernelGraph_t *graph, uint32_t numSections);

    /** This frees the sections and order */
    void FreeSections(DVP_KernelGraph_t *graph);

    /** This allocates a number of Kernel Graphs */
    bool AllocateGraphs(uint32_t numGraphs);

    /** This frees the Kernel graph structures */
    void FreeGraphs();

    /** This allocates the data structures used to debug intermediate output */
    bool AllocateImageDebug(uint32_t numImgDbg);

    /** This frees the intermediate output debug data structure */
    void FreeImageDebug();

    /** This allows the engine to allocate buffers out of the shared region */
    bool AllocateBufferStructs(uint32_t numBuffers);

    /** This frees the allocated buffers from the shared region */
    void FreeBufferStructs();

    // Display API

    /** The convienence call wraps the DVP Display "C" API or uses the ICS special mechanisms */
    bool DisplayCreate(uint32_t image_width,  uint32_t image_height,
                       uint32_t buffer_width, uint32_t buffer_height,
                       uint32_t scaled_width, uint32_t scaled_height,
                       uint32_t crop_top,     uint32_t crop_left);

    /** The convienence call wraps the DVP Display "C" API or uses the ICS special mechanisms */
    bool DisplayAllocate(DVP_Image_t *pImage);

    /** The convienence call wraps the DVP Display "C" API or uses the ICS special mechanisms */
    void DisplayFree(DVP_Image_t *pImage);

    /** The convienence call wraps the DVP Display "C" API or uses the ICS special mechanisms */
    void DisplayDestroy();

    /** The convienence call wraps the DVP Display "C" API or uses the ICS special mechanisms */
    virtual void DisplayRender(DVP_Image_t *pImage);

    /** The method by which to return a buffer to it's display mechanism to prevent out of order rendering */
    void DisplayDrop(DVP_Image_t *pImage);

#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))
    /** If used, this method will be called before the Create/Allocate calls */
    virtual void DisplayOnWindow(const sp<IBinder>& binder, const sp<ANativeWindow>& window);
    virtual void DisplayOnSurface(const sp<Surface>& surface);
    virtual void DisplayOnTexture(const sp<ISurfaceTexture>& texture);
#endif

protected:
    /** A protected constructor for this class and subclasses to call to initialize */
    virtual void Constructor(uint32_t width, uint32_t height, uint32_t fps, fourcc_t color, char *camName, uint32_t numFrames);

};

#endif

