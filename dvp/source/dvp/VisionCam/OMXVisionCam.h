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

#ifndef _OMX_VISIONCAM_H_
#define _OMX_VISIONCAM_H_

#if defined(WIN32) || defined(UNDER_CE)
  #include <windows.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <math.h>
  #include <string.h>
#endif

#include <sosal/sosal.h>
#include <dvp/VisionCam.h>

#include <OMX_Types.h>
#include <OMX_Core.h>
#include <OMX_IVCommon.h>
#include <OMX_Component.h>
#include <OMX_Index.h>
#include <OMX_TI_Index.h>
#include <OMX_TI_IVCommon.h>
#include <OMX_TI_Common.h>
#include <OMX_CoreExt.h>
#include <OMX_IndexExt.h>

#if defined(EXPORTED_3A)
#include <OMXVisionCam_3A_Export.h>
#endif

#include "VisionCamUtils.h"

/** Enables some manual camera contols, like manual white balance color gains and gamma table coeficients. */
#if defined(ANDROID) && !defined(JELLYBEAN)
#define OMX_CAMERA_SUPPORTS_MANUAL_CONTROLS
#define OMX_CAMERA_SUPPORTS_GESTURES
#if !defined(TUNA) && !defined(MAGURO) && !defined(BLAZE_TABLET) && !defined(BLAZE)
#define OMX_CAMERA_SUPPORTS_MTIS
#define OMX_CAMERA_SUPPORTS_TI_MTIS
#define OMX_CAMERA_SUPPORTS_HMSGAMMA
#endif
#define OMX_CAMERA_SUPPORTS_HISTOGRAM
#endif
#ifndef __QNX__
#define OMX_CAMERA_SUPPORTS_WHITEBALANCE_OVERWRITE
#endif
#if defined(BLAZE) || defined(BLAZE_TABLET)
#define OMX_CAMERA_SUPPORTS_STEREO_GESTURES
#undef OMX_CAMERA_SUPPORTS_SIMULTANEOUS_SENSOR_GESTURES
//#define OMX_CAMERA_SUPPORTS_HMSGAMMA
#endif

#if !defined(AOSP) && !defined(CPCAM) && !defined(JELLYBEAN) && (defined(TUNA) || defined(MAGURO))
#define OMX_CAMERA_SUPPORTS_CAMERA_VIEW
#endif

/** Maximum camera output port count, not used. @see VisionCamPort_e */
const uint32_t USED_PORTS_NUMBER = VCAM_PORT_MAX;

/** Initial frame width. */
const uint32_t QVGA_WIDTH = 320;

/** Initial frame height. */
const uint32_t QVGA_HEIGHT = 240;

/** Initial frame rate. */
const uint32_t INITIAL_FRAMERATE = 30;

/** Minimum value for manual WB color gain. @see VCAM_PARAM_WB_COLOR_GAINS */
const uint32_t COLOR_GAIN_MIN = 512;

/** Maximum value for manual WB color gain. @see VCAM_PARAM_WB_COLOR_GAINS */
const uint32_t COLOR_GAIN_MAX = 2048;

/** Maximum white balance delay time. @see VCAM_PARAM_AWB_MIN_DELAY_TIME */
const uint32_t AWB_Delay_Time_Max = 10000;

/** Minimum white balance delay time. @see VCAM_PARAM_AWB_MIN_DELAY_TIME */
const uint32_t AWB_Delay_Time_Min = 0;

/** Maimum exposure gain delay time. @see VCAM_PARAM_AGC_MIN_DELAY_TIME */
const uint32_t AE_Delay_Time_Max = 10000;

/** Minimum exposure gain delay time. @see VCAM_PARAM_AGC_MIN_DELAY_TIME */
const uint32_t AE_Delay_Time_Min = 0;

/** Maximum count of face data that could be output bu face detction algotithm.
  * In othe words maximum count of face that could be detected.
*/
const uint32_t VCAM_Max_Gesture_Per_Frame = 35;

#define ARR_SIZE(arr) ( sizeof(arr)/sizeof(arr[0]) )

#if TIME_PROFILE
/** In case ot time profiling this is the start time
  * of a certain time measurement target.
*/
#define TIME_START( time_prof ) {\
 gettimeofday( &time_prof->mStart , NULL);\
}

/** In case of time profiling this is the ending time
  * of a certain time measurement target.
*/
#define TIME_END( time_prof ) {\
 gettimeofday( &time_prof->mEnd , NULL);\
}

/** @enum TimeProfileTargets
  * Defines time profiling targets - the commands for which execution time is measured.
  * This is just an indexing of of measured time windows.
*/
typedef enum _time_profile_targets{
    first,
    second,
    last,

    VCAM_TIME_TARGET_MAX
}TimeProfileTargets;

/** @class VisionCamTimeProfile
  * Implementation of time profiling.
  * Each measured time is an object of this type.
  * Measured time is dumped when this object is destroyed.
*/
class VisionCamTimePtofile{
  public:
    /** @fn Constructor
      * @param name     a string that will be used when measured time of dumped.
    */
    VisionCamTimePtofile( const char * name = NULL);
    /** @fn Destructor
      * Destroys a profiling object and dumps measured time in milliseconds.
    */
    ~VisionCamTimePtofile();

    /** @fn dump()
      * Prints the time measured for execution of command corresponding to this object.
      * This function, also sets the start and end moment to 0, so it can be used again.
    */
    void dump();

    struct timeval mStart;  /** System time at start. */
    struct timeval mEnd;    /** System time at end. */
    const char * mName;     /** User defined name of the command. Used when results are shown. */
};
#endif

/** @struct OMXVCAM_Msg_t
  * Message structure used for event handling for OMX Camera events.
*/
typedef struct _omxvcam_msg_t {
    OMX_EVENTTYPE eEvent;       /** Event, defined by OMX IL specification. */
    OMX_U32 nData1;             /** A data passed by OMX Camera. */
    OMX_U32 nData2;             /** A data passed by OMX Camera. */
    semaphore_t *semaphore;     /** Semaphore on which this event will be expected. */
    OMX_HANDLETYPE hComponent;  /** OMX Camera component. */
    OMX_U32 timeout;            /** Semaphore time out. */
} OMXVCAM_Msg_t;

/** @enum FrameEvents_e
  * Frame event type. These events are processed by frame processing thread.
  * @see FrameThreadFunc()
*/
typedef enum {
    EFrameNone,
    EFrameReceived,     /** Frame is received from OMX Camera and is ready to be passed to user. */
    EFrameReturned,     /** User has released the frame so it can be passed back to camera. */

    EFrameManagerExit   /** No frames are expected, both from user and from camera, so frame managing thread should exit. */
} FrameEvents_e;

/** @struct SimpleMsg_t
  * A simple message structure used by frame management thread.
*/
typedef struct _omx_vcam_simple_msg_t{
    FrameEvents_e event;    /** The frame event @see FrameEvents_e. */
//        semaphore_t sem;
    void *data;             /** Frame data. */
}SimpleMsg_t;

/** ExtraDataTypeLUT[][]
  * A look-up table used to translate VisoinCam extra data type to OMX extra data type.
  * For details about OMX defined extra data types, refer to OMX IL specification.
  * @see VisionCamExtraDataType_e
*/
const int ExtraDataTypeLUT[][ 2 ] =
{
    {   VCAM_EXTRA_DATA_NONE,               OMX_ExtraDataNone           },
    //   {  VCAM_EXIF_ATTRIBUTES            ,   VCAM_ExifAttributes          },
    {   VCAM_ANCILLARY_DATA             ,   OMX_AncillaryData           },
    {   VCAM_WHITE_BALANCE              ,   OMX_WhiteBalance            },
#if defined(OMX_CAMERA_SUPPORTS_WHITEBALANCE_OVERWRITE)
    {   VCAM_MANUAL_WHITE_BALANCE       ,   OMX_TI_WhiteBalanceOverWrite   },
#endif
    {   VCAM_UNSATURATED_REGIONS        ,   OMX_UnsaturatedRegions      },
    {   VCAM_FACE_DETECTION             ,   OMX_FaceDetection           },
    //   {  VCAM_BARCODE_DETECTION          ,   OMX_BarcodeDetection        },
    //   {  VCAM_FRONT_OBJECT_DETECTION     ,   OMX_FrontObjectDetection    },
    //   {  VCAM_MOTION_ESTIMATION          ,   OMX_MotionEstimation        },
#if defined(OMX_CAMERA_SUPPORTS_MTIS)
    {   VCAM_MTIS_TYPE                  ,   OMX_MTISType                },
#elif defined(OMX_CAMERA_SUPPORTS_TI_MTIS)
    {   VCAM_MTIS_TYPE                  ,   OMX_TI_MTISType             },
#endif
    //   {  VCAM_DISTANCE_ESTIMATION        ,   OMX_DistanceEstimation      },
#if defined(OMX_CAMERA_SUPPORTS_HISTOGRAM)
    {   VCAM_HISTOGRAM                  ,   OMX_Histogram               },
#endif
#if defined(OMX_CAMERA_SUPPORTS_HMSGAMMA)
    {   VCAM_HMS_GAMMA                  ,   OMX_TI_HMSGamma             },
#endif
    {   VCAM_FOCUS_REGION               ,   OMX_FocusRegion             },
    //   {  VCAM_EXTRA_DATA_PAN_AND_SCAN    ,   OMX_ExtraDataPanAndScan     },
    //   {  VCAM_RAW_FORMAT                 ,   OMX_RawFormat               },
//       {  VCAM_SENSOR_TYPE                ,   OMX_SensorType              },
    //   {  VCAM_SENSOR_CUSTOM_DATA_LENGTH  ,   OMX_SensorCustomDataLength  },
    //   {  VCAM_SENSOR_CUSTOM_DATA         ,   OMX_SensorCustomData        }
#if defined(OMX_CAMERA_SUPPORTS_FD_RAW)
    {   VCAM_FACE_DETECTION_RAW         ,   OMX_TI_FaceDetectionRaw     },
#endif
#if defined(OMX_CAMERA_SUPPORTS_IMAGE_PYRAMID)
    {   VCAM_IMAGE_PYRAMID              ,   OMX_TI_ImagePyramid         },
#endif
};

/** ColorFormatLUT[][]
  * A look-up table, used to translate fourcc_t color formats to OMX defined color formats.
  * @note FOURCC_RGB565 and FOURCC_BGR565 are supported only on video output port.
  * @see _fourcc
*/
const int ColorFormatLUT[][ 2 ] =
{
    {   FOURCC_UYVY ,   OMX_COLOR_FormatCbYCrY              },
    {   FOURCC_NV12 ,   OMX_COLOR_FormatYUV420SemiPlanar    },
    {   FOURCC_RGB565 , OMX_COLOR_Format16bitRGB565         },
    {   FOURCC_BGR565 , OMX_COLOR_Format16bitBGR565         },
    {   OMX_COLOR_FormatMax,    OMX_COLOR_FormatMax         }
};

/** CaptureModeLUT[][]
  * A look-up table, used to translate VisionCam operating mode to OMX Camera operating mode.
  * @see VisionCamCaptureMode for supported operating modes.
*/
const int CaptureModeLUT[][ 2 ] =
{
#if defined(GT_I9100G)
    { VCAM_VIDEO_NORMAL       , OMX_CaptureVideo                       },
#else // all other platforms
    { VCAM_VIDEO_NORMAL       , OMX_CaptureImageProfileBase            },
#endif
#if defined(GT_I9100G)
    { VCAM_VIDEO_HIGH_SPEED   , OMX_CaptureImageHighSpeedTemporalBracketing },
#else // all other platforms
    { VCAM_VIDEO_HIGH_SPEED   , OMX_CaptureHighSpeedVideo              },
#endif
    { VCAM_GESTURE_MODE       , OMX_TI_CaptureGestureRecognition       },
    { VCAM_STEREO_MODE        , OMX_CaptureStereoImageCapture          },
#ifdef OMX_CAMERA_SUPPORTS_STEREO_GESTURES
    { VCAM_STEREO_GEST_MODE   , OMX_TI_StereoGestureRecognition        },
#endif
#if defined(__QNX__)
    { VCAM_VIEWFINDER_MODE    , OMX_CaptureVideo                       },
#endif
#if defined(OMX_CAMERA_SUPPORTS_SIMULTANEOUS_SENSOR_GESTURES)
    { VCAM_GESTURE_DUAL_SENSOR_MODE, OMX_TI_SimultaneousSensorsGesture },
#endif
    { VCAM_CAP_MODE_MAX       , OMX_CamOperatingModeMax                }
};

/** FocusModeLUT[][]
  * A look-up table, use to translate focus modes, supported by VisionCam, to OMX defined focus modes.
  * @see VisionCamFocusMode for supported focus modes.
*/
const int FocusModeLUT[][ 2 ] =
{
    { VCAM_FOCUS_CONTROL_ON                     , OMX_IMAGE_FocusControlOn                   },
    { VCAM_FOCUS_CONTROL_OFF                    , OMX_IMAGE_FocusControlOff                  },
    { VCAM_FOCUS_CONTROL_AUTO                   , OMX_IMAGE_FocusControlAuto                 },
    { VCAM_FOCUS_CONTROL_AUTO_LOCK              , OMX_IMAGE_FocusControlAutoLock             },
#if !defined(__QNX__)
    { VCAM_FOCUS_CONTRO_AUTO_MACRO              , OMX_IMAGE_FocusControlAutoMacro             },
#endif
    { VCAM_FOCUS_CONTROL_AUTO_INFINITY          , OMX_IMAGE_FocusControlAutoInfinity         },
//    { VCAM_FOCUS_FACE_PRIORITY_MODE             , OMX_IMAGE_FocusFacePriorityMode            },
//    { VCAM_FOCUS_REGION_PRIORITY_MODE           , OMX_IMAGE_FocusRegionPriorityMode          },
    { VCAM_FOCUS_CONTROL_HYPERFOCAL             , OMX_IMAGE_FocusControlHyperfocal           },
    { VCAM_FOCUS_CONTROL_PORTRAIT               , OMX_IMAGE_FocusControlPortrait             },
    { VCAM_FOCUS_CONTROL_EXTENDED               , OMX_IMAGE_FocusControlExtended             },
    { VCAM_FOCUS_CONTROL_CONTINOUS_NORMAL       , OMX_IMAGE_FocusControlContinousNormal      },
#if !defined(__QNX__)
    { VCAM_FOCUS_CONTROL_CONTINOUS_EXTENDED     , OMX_IMAGE_FocusControlContinousExtended    },
#else
    { VCAM_FOCUS_CONTROL_CONTINOUS_EXTENDED     , OMX_TI_IMAGE_FocusControlContinuousExtended  },
#endif
//    { VCAM_FOCUS_FACE_PRIORITY_CONTINOUS_MODE   , OMX_IMAGE_FocusFacePriorityContinousMode   },
//    { VCAM_FOCUS_REGION_PRIORITY_CONTINOUS_MODE , OMX_IMAGE_FocusRegionPriorityContinousMode },
    { VCAM_FOCUS_CONTROL_MAX                    , OMX_IMAGE_FocusControlMax                  }
};

/** MirrorTypeLUT[][]
  * A look-up table, used to translate VisionCam supported mirroring modes, to OMX mirror modes.
  * @see VisionCamMirrorType
*/
const int MirrorTypeLUT[][ 2 ] =
{
    {   VCAM_MIRROR_NONE        ,   OMX_MirrorNone          },
    {   VCAM_MIRROR_VERTICAL    ,   OMX_MirrorVertical      },
    {   VCAM_MIRROR_HORIZONTAL  ,   OMX_MirrorHorizontal    },
    {   VCAM_MIRROR_BOTH        ,   OMX_MirrorBoth          },
    {   VCAM_MIRROR_MAX         ,   OMX_MirrorMax           }
};

#if ( defined(DUCATI_1_5) || defined(DUCATI_2_0) ) && defined(OMX_CAMERA_SUPPORTS_GESTURES)
/** GestureTypeLUT[][]
  * A look-up table, used to translate VisionCam defined gesture events, to OMX defined gesture events.
  * @see VisionCamGestureEvent_e
*/
const int GestureTypeLUT[][2] =
{
    { VCAM_GESTURE_EVENT_INVALID         , OMX_TI_GESTURE_NO_GESTURE     },
    { VCAM_GESTURE_EVENT_SWIPE_LEFT      , OMX_TI_GESTURE_SWIPE_LEFT     },
    { VCAM_GESTURE_EVENT_SWIPE_RIGHT     , OMX_TI_GESTURE_SWIPE_RIGHT    },
    { VCAM_GESTURE_EVENT_FIST_LEFT       , OMX_TI_GESTURE_FIST_LEFT      },
    { VCAM_GESTURE_EVENT_FIST_RIGHT      , OMX_TI_GESTURE_FIST_RIGHT     },
    { VCAM_GESTURE_EVENT_MAX             , OMX_TI_GESTURE_MAX            }
};

/** ObjectTypeLUT[][]
  * A look-up table, used to translate VisionCam gesture object type to coresponding OMX gesture object.
  * @see VisionCamObjectType
*/
const int ObjectTypeLUT[][2] =
{
    { VCAM_OBJECT_PALM , OMX_TI_OBJECT_PALM },
    { VCAM_OBJECT_FIST , OMX_TI_OBJECT_FIST },
    { VCAM_OBJECT_FACE , OMX_TI_OBJECT_FACE },
    { VCAM_OBJECT_MAX  , OMX_TI_OBJECT_MAX  }
};
#endif

/** StereoLayoutLUT[][]
  * A look-up table, used to translate VisionCam defined stereo layouts to OMX stereo layouts.
  * @see VisionCamStereoLayout
*/
const int StereoLayoutLUT[][2] =
{
    {   VCAM_STEREO_LAYOUT_TOPBOTTOM,   OMX_TI_StereoFrameLayoutTopBottom   },
    {   VCAM_STEREO_LAYOUT_LEFTRIGHT,   OMX_TI_StereoFrameLayoutLeftRight   },
    {   VCAM_STEREO_LAYOUT_MAX,         OMX_TI_StereoFrameLayoutMax         }
};

/** OMX index of preview frame output port. */
const int VCAM_CAMERA_PORT_VIDEO_OUT_PREVIEW = 2;

/** OMX index of video frame output port. */
const int VCAM_CAMERA_PORT_VIDEO_OUT_VIDEO = 3;

/** Needed for priority memagement. Camera Group ID is set DOMX layers. */
const unsigned int CAMERA_GROUP_ID = 8;

/** @fn thread_ret_t PreemptionThreadLauncher( void *arg )
  * A thread function that implements preemption mechanism for OMXVisionCam.
  * Preemption model used here is defined by OMX standard.
  * @param arg  The instance of OMXVisionCam that is preempted.
*/
thread_ret_t PreemptionThreadLauncher( void *arg );

/** thread_ret_t FrameThreadFunc(void *arg)
  * Thread function that implements frame transmit/receive mechanism between user and OMX camera.
  * @param arg  Thee instance of OMXVisinCam that this thead serves.
*/
thread_ret_t FrameThreadFunc(void *arg);

/** @class OMXVisionCam
  * @implements VisionCam
  *
  * Implements OMX Camera interface. Publicly inherits VisionCam.
  * This is the final point before camera specific software inplementaion.
*/
class OMXVisionCam : public VisionCam
{
protected:
    OMX_HANDLETYPE* mHandle;

private: // internal data types

    /** @class VCAM_PortParameters
      * Keeps all the data needed to control certain camera output port.
    */
    class VCAM_PortParameters
    {
    public:
//        OMX_U32                         mHostBufaddr[VCAM_NUM_BUFFERS]; /** The address of each buffer assigned to this port. This is only used for tailor buffers. @todo check if last is true (gingerbread, froyo) */
        OMX_BUFFERHEADERTYPE           *mBufferHeader[VCAM_NUM_BUFFERS];/** Buffer specific data, generated by OMX camera handle at the time a buffer is registered. */
        OMX_U32                         mWidth;                         /** Image width in pixels of a buffer. @see fourcc_t definitions to check pixel in bytes size for different formats. */
        OMX_U32                         mHeight;                        /** Image height in pixels of a buffer. */
        OMX_U32                         mStride;                        /** Byte distance from one pixel to the next pixel in the y direction. @see DVP_Image_t (DVP_Image_t::y_stride). */
        OMX_U8                          mNumBufs;                       /** Number of buffers assigned to this port. */
        OMX_U32                         mBufSize;                       /** The size of a buffer in bytes. */
        OMX_COLOR_FORMATTYPE            mColorFormat;                   /** The fourcc_t color fomat used to calculate buffer allocation sizes. This must be also the color format applied to OMX camera. */
//        OMX_PARAM_VIDEONOISEFILTERTYPE  mVNFMode;                       /** Video noise filtering mode: on, off or auto. @todo check if used */
//        OMX_PARAM_VIDEOYUVRANGETYPE     mYUVRange;                      /** Frame YUV Range. @todo check if used */
//        OMX_CONFIG_BOOLEANTYPE          mVidStabParam;                  /** Video stabilisation mode: enabled or disabled. @todo check if used and needed at all. */
//        OMX_CONFIG_FRAMESTABTYPE        mVidStabConfig;                 /** Video stabilisation mode: enabled or disabled. @todo check if used and needed at all. */
//        OMX_U32                         mCapFrame;                      /** @todo is it used. */
        OMX_U32                         mFrameRate;                     /** Frame rate. @note Frame rate is applied to all ports. */
        OMX_U32                         mRotation;                      /** Applied rotation. @see VisionCamRotation_e */
        bool_e                          mIsActive;                      /** Indicates if this port is currently outputting any data. */
    };

    /** @class VCAM_ComponentContext
      * Holds the current status of OMX camera handle.
    */
    class VCAM_ComponentContext
    {
    public:
        OMX_HANDLETYPE              mHandleComp;                            /** The camera handle. */
        OMX_U32                     mNumPorts;                              /** Number of ports that camera could use. */
        /*VisionCamPort_e*/ OMX_U32            mPortsInUse[VCAM_PORT_MAX];  /** Output port indexes, as they are defined in OMX camera. */
        OMX_U32                     mExtraDataPortIndex;                    /** @todo remove me! */
        VCAM_PortParameters         mCameraPortParams[VCAM_PORT_MAX];       /** The parameters for each port. */
    };

    /** @struct BuffersInUse_t
      * A structure that packs a buffer array and buffer count.
      @todo move to port parameters
      * @see DVP_Image_t
    */
    typedef struct BuffersInUse_t{
        DVP_Image_t *mBuffers;      /** The buffer aray. */
        uint32_t mNumberBuffers;    /** Number of buffers. */
    }BuffersInUse_t;

    /** @enum ValueTypeOrigin
      * Used to parse look-up tables.
      * Marks where exactly the value, for which a match is searched, is defined.
    */
    typedef enum
    {
        VCAM_VALUE_TYPE = 0,    /** Value is defined in VisionCam and matching OMX value is searched. */
        OMX_VALUE_TYPE = 1      /** Value is defined in OMX and matching VisionCam value is searched. */
    } ValueTypeOrigin ;

    /** @enum VisionCamPreemptionActivity_e
      * Used to define component preemption stage.
    */
    enum VisionCamPreemptionActivity_e {
        VCAM_PREEMPT_INACTIVE,      /** No preemption is started. */
        VCAM_PREEMPT_SUSPEND,       /** OMX camera was preempted by a process with a higher priority than VisionCam, so VisionCam has to be suspended. */
        VCAM_PREEMPT_RESUME,        /** VisionCam is ready to continue its work, after it has been suspended. */
        VCAM_PREEMPT_WAIT_TO_START  /** VisionCam is waiting to get the control over camera. */
    };

    /** @enum VisionCamFaceDetectionType_t
      * Enumaration that defines which type of face detection
    */
    typedef enum {
        VCAM_FACE_DETECTION_DISABLED    = 0,
        VCAM_FACE_DETECTION_MIN         = 1 << 0,
        VCAM_FACE_DETECTION_RAW         = 1 << 0,   /**  */
        VCAM_FACE_DETECTION             = 1 << 1,   /**  */

        VCAM_FACE_DETECTION_MAX,                    /**  */
        VCAM_FACE_DETECTION_ALL     = ( ((VCAM_FACE_DETECTION_MAX -1 ) << 1) -1 )
    } VisionCamFaceDetectionType_t;

    /** @typedef status_e(*serviceFunc)(OMXVisionCam *, void *)
      * Pointer to a serving function.
      * Serving functions are used in the cases where additional setup must be done
      * in order for a certain job to be finished by camera.
    */
    typedef status_e(*serviceFunc)(OMXVisionCam *, void *);

    /** @struct setFunctionParameters_t
     *    stores the data passed to set/getParameter() apis
     * @todo make better description
     */
    typedef struct {
        void *data;
        uint32_t size;
        VisionCamPort_e port;
    } setFunctionParameters_t;

// Inherited Methods from VisionCam
public:
    /** Constructor. */
    OMXVisionCam();
    /** Destructor. */
    ~OMXVisionCam();

    /** @fn status_e init(void * cookie)
      * Initialises OMXVisionCam, gets a camera handle and gives some default values.
    */
    status_e init(void * cookie);

    /** @fn status_e deinit()
      * Deinits OMXVisionCam, releases the camera and frees all available resources.
    */
    status_e deinit();

    /** @fn status_e useBuffers( DVP_Image_t *prvBufArr, uint32_t numPrvBuf, VisionCamPort_e port)
      * Registers and allocates (internal for OMXVisionCam) frame descriptor for each buffer passed.
      * This way, buffers are ready to be passed to OMX camera. Implements VisionCam::useBuffers().
      * @param *prvBufArr   array of buffers as thay are allocated.
      * @param numPrvBuf    number of the buffers in this array.
      * @param port         the port on which these buffers must be assigned to.
    */
    status_e useBuffers( DVP_Image_t *prvBufArr, uint32_t numPrvBuf, VisionCamPort_e port);

    /** @fn status_e releaseBuffers( VisionCamPort_e port )
      * Unregisters (internal for OMXVisionCam) all buffers registered with useBuffers() to OMXVisoinCam
      * and frees corresponding frame descriptors. Implements VisionCam::releaseBuffers().
      * @param port     the port on which these buffers are assigned.
    */
    status_e releaseBuffers( VisionCamPort_e port );

    /** @fn status_e flushBuffers( VisionCamPort_e port )
      *
    */
    status_e flushBuffers( VisionCamPort_e port );

    /** @fn status_e sendCommand( VisionCamCmd_e cmdId, void *param, uint32_t size, VisionCamPort_e port )
      * Requests camera to execute some additional functionality. Implements VisionCam::sendCommand().
      * @param cmdId    the command id that bust be executed.
      * @param param    pointer to additional data that refers to this command.
      * @param size     size of data pointed by param.
      * @param port     the port on which this must be executed.
      * @return         STATUS_SUCCESS on success and any other error code otherwise.
      * @see VisionCamCmd_e     for possible commands.
      * @see VisionCamPort_e    for available ports.
    */
    status_e sendCommand( VisionCamCmd_e cmdId, void *param, uint32_t size, VisionCamPort_e port );

    /** @fn status_e setParameter( VisionCamParam_e paramId, void* param, uint32_t size, VisionCamPort_e port)
      * Configure the camera with supplied parameter. Implements VisionCam::setParameter().
      * @param  paramId ID of the parameter that has to be applied.
      * @param  param   Pointer to parameter data.
      * @param  size    Size of parameter data.
      * @param  port    Output port for which this parameters will be applied.
      * @return         STATUS_SUCCESS on success and any other error code otherwise.
      * @see VisionCamParam_e   for possible parameters.
      * @see VisionCamPort_e    for available ports.
    */
    status_e setParameter( VisionCamParam_e paramId, void* param, uint32_t size, VisionCamPort_e port);

    /** @fn status_e getParameter( VisionCamParam_e paramId, void* param, uint32_t size, VisionCamPort_e port)
      * Reads camera configuration parameter. Implements VisionCam::getParameter().
      * @param  paramId ID of the parameter that has to be read.
      * @param  param   Pointer to user allocated space, where parameter value will be written.
      * @param  size    Size of parameter data.
      * @param  port    Output port for which this parameters are applied.
      * @return         STATUS_SUCCESS on success and any other error code otherwise.
      * @see VisionCamParam_e   for possible parameters.
      * @see VisionCamPort_e    for available ports.
    */
    status_e getParameter( VisionCamParam_e paramId, void* param, uint32_t size, VisionCamPort_e port);

    /** @fn status_e returnFrame( VisionCamFrame *cameraFrame )
      * This must be called after VisionCam client has finished his job with a frame buffer.
      * With this, user notifies VisionCam that this frame buffer is ready to be returned back to camera.
      * @param cameraFrame The pointer to the frame given to the user which is now completed.
      */
    status_e returnFrame( VisionCamFrame *cameraFrame );

private:
    // OMXVisionCam
    /** @fn status_e startPreview( VisionCamPort_e port )
      * Signals the camera to start a streaming a frames on a certain output port.
      * @param port the port on which this streaming shold be started.
      * @return         STATUS_SUCCESS on success and any other error code otherwise.
    */
    status_e startPreview( VisionCamPort_e port );

    /** @fn status_e stopPreview( VisionCamPort_e port )
      * Signals the camera to stop a streaming a frames on a certain output port.
      * @param port the port on which this streaming shold be stopped.
      * @return     STATUS_SUCCESS on success and any other error code otherwise.
    */
    status_e stopPreview( VisionCamPort_e port );

    /** @fn inline OMX_STATETYPE getComponentState()
      * Gets current state of OMX camera.
    */

#if defined(EXPORTED_3A)
    /** @fn status_e _3A_ApplyExported()
      * Invokes the camera hw to apply all the 3A settings that user has requested.
    */
    status_e _3A_ApplyExported();
#endif // EXPORTED_3A

    inline OMX_STATETYPE getComponentState();

    /** @fn inline OMX_ERRORTYPE initPortCheck( OMX_PARAM_PORTDEFINITIONTYPE * portCheck , OMX_U32 portIndex )
      * Read and initialises current configuration applied to a certain output port.
      * The configuratin structure is passed by a pointer so it can be used later for port reconfiguration.
      * @param portCheck    configuratin structure pointer that will be filled with appropriate data.
      * @param portIndex    one of VisionCamPort_e port indexes; this is the port which configuration is requested
      * @return OMX_ErrorNone on successful read.
    */
    inline OMX_ERRORTYPE initPortCheck( OMX_PARAM_PORTDEFINITIONTYPE * portCheck , OMX_U32 portIndex );

    /** @fn inline status_e ConvertError(OMX_ERRORTYPE error)
      * Converts OMX error codes to VisionCam error codes (status_e)
      * @param error one of OMX error codes.
      * @return  status_e value.
      * @see status_e
    */
    inline status_e ConvertError(OMX_ERRORTYPE error);

    /** @fn inline int getLutValue( int searchVal, ValueTypeOrigin origin, const int lut[][2], int lutSize)
      * Parses VisionCam <-> OMX look-up tables.
      * @param searchVal    The value for which a match is required.
      * @param origin       Indicates if searchVal is defined by VisionCam or by OMX.
      *                     Depending on that, returned value is respectively OMX or VisionCam defined.
      * @param lut[][2]     The look-up table to be parsed.
      * @param lutSize      Number of couples in this table.
      * @return             The value corresponding to searchVal.
    */
    inline int getLutValue( int searchVal, ValueTypeOrigin origin, const int lut[][2], int lutSize);


    /** @fn void GetDucatiVersion()
      * Gets currnet domx version.
    */
    void GetDucatiVersion();

    /** @fn OMX_ERRORTYPE RegisterForEvent(OMX_IN OMX_HANDLETYPE hComponent,
                                           OMX_IN OMX_EVENTTYPE eEvent,
                                           OMX_IN OMX_U32 nData1,
                                           OMX_IN OMX_U32 nData2,
                                           OMX_IN semaphore_t *semaphore,
                                           OMX_IN OMX_U32 timeout)
      * Creates an event message and adds it to the list of events that are expected to occur at OMX side.
    */
    OMX_ERRORTYPE RegisterForEvent(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_IN OMX_EVENTTYPE eEvent,
                                   OMX_IN OMX_U32 nData1,
                                   OMX_IN OMX_U32 nData2,
                                   OMX_IN semaphore_t *semaphore,
                                   OMX_IN OMX_U32 timeout);

    /** @fn     static OMX_ERRORTYPE EventHandler(OMX_IN OMX_HANDLETYPE hComponent,
                                              OMX_IN OMX_PTR pAppData,
                                              OMX_IN OMX_EVENTTYPE eEvent,
                                              OMX_IN OMX_U32 nData1,
                                              OMX_IN OMX_U32 nData2,
                                              OMX_IN OMX_PTR pEventData);
      *
      * OMX event handler. This is a callback function, registerd in OMX component, by the time of its initialization.
      * It is invoked every time an event is generated by camera.
      * The events handled are not only internal camera events,
      * but also a results ot some external processes that deal with camera.
    */
    static OMX_ERRORTYPE EventHandler(OMX_IN OMX_HANDLETYPE hComponent,
                                              OMX_IN OMX_PTR pAppData,
                                              OMX_IN OMX_EVENTTYPE eEvent,
                                              OMX_IN OMX_U32 nData1,
                                              OMX_IN OMX_U32 nData2,
                                              OMX_IN OMX_PTR pEventData);

    /** @fn static OMX_ERRORTYPE EmptyBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_IN OMX_PTR pAppData,
                                         OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);
      * A callback notifing that OMX camera has unregistered a buffer and has freed its buffer header.
    */
    static OMX_ERRORTYPE EmptyBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_IN OMX_PTR pAppData,
                                         OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

    /** @fn static OMX_ERRORTYPE FillBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_PTR pAppData,
                                        OMX_IN OMX_BUFFERHEADERTYPE* pBuffHeader);
      *
      * A callback notifing that OMX camera has wrote a frame buffer and it is ready to be passed to user.
    */
    static OMX_ERRORTYPE FillBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_PTR pAppData,
                                        OMX_IN OMX_BUFFERHEADERTYPE* pBuffHeader);

//    /** status_e frameReceiveService(OMX_IN OMX_BUFFERHEADERTYPE* pBuffHeader)
//      *
//    */
//    status_e frameReceiveService(OMX_IN OMX_BUFFERHEADERTYPE* pBuffHeader);

    /** @fn OMX_ERRORTYPE setPortDef( int portIndex )
      * Configures a port. Sets the data that port needs to get ready for image processing.
      * @param portIndex    VisionCam defined index of the port.
      * @see   VisionCamPort_e
      * @return OMX_ErrorNone on success.
    */
    OMX_ERRORTYPE setPortDef( int32_t portIndex/*, VCAM_PortParameters * portData*/);

    /** @fn status_e startAutoFocus( VisionCamFocusMode focusMode )
      * Starts auto focus in a certain mode. Starts a thread that will wait for focus event.
      * @param      focusMode   mode in which auto focus must start.
      * @see        VisionCamFocusMode
      * @return     STATUS_SUCCESS on success and any other error code otherwise.
    */
    status_e startAutoFocus(uint32_t inp);

    /** @fn status_e enableFaceDetect(VisionCamPort_e port)
      * This will configure the component to start/stop face detection.
      * Will also start/stop face detection extra data
      * faces coordinates will be written into camera frame received in preview callback.
      * @see        VisionCamFaceType
      * @see        VisionCamFrame::mFaces
      * @return     STATUS_SUCCESS on success and any other error code otherwise.
    */
    status_e enableFaceDetect(VisionCamPort_e port, VisionCamFaceDetectionType_t fdType );

    /** @fn void getFacesCoordinates( VisionCamFrame *frame)
      * Fill the face coordinates field in camera frame,
      * which will be received by OMXVisionCam client at each frame.
      * @param frame    a frame descripror.
    */
    void getFacesCoordinates( VisionCamFrame *frame);

    /** @fn void getMetadataPtrs( VisionCamFrame *frame)
      * Parse through the extra data type structure to find the pointer to the relevent
      * data type.  This is to abstract the port number, version, and packet structure from the client.
      * @param frame    pointer to a frame descripror.
    */
    void getMetadataPtrs( VisionCamFrame *frame);

    /** @fn OMX_ERRORTYPE getFocusStatus(OMX_FOCUSSTATUSTYPE *status)
      * Gest current status of focus command execution.
      * @param *status   points to the variable, where focus status will be written.
      * @return OMX_ErrorNone on success.
    */
    OMX_ERRORTYPE getFocusStatus(OMX_FOCUSSTATUSTYPE *status);

    /** @fn status_e transitToState(OMX_STATETYPE targetState, serviceFunc transitionService = NULL, void * data = NULL)
      * Sets OMX camera in a certain state. Camera state represent camera behaviour and configuration status.
      * @param targetState          the state that must be reached. Refer to OMX_STATETYPE in OMX IL specification.
      * @param transitionService    in some cases, OMXVisionCam must do additional steps in order for camera to reach requested state.
      *                             transitionService must point to the function that does this steps.
      * @param data                 any data (like buffers for example), that may be needed for this transition to finish.
      * @return     STATUS_SUCCESS on success and any other error code otherwise.
      * @see                        serviceFunc
    */
    status_e transitToState(OMX_STATETYPE targetState, serviceFunc transitionService = NULL, void * data = NULL);

    /** @fn status_e portEnableDisable( OMX_COMMANDTYPE enCmd, serviceFunc enablementService, VisionCamPort_e port )
      * Enables and disables a port. Enabled port transfers frames only if camera is in OMX_StateExecuting.
      * @param enCmd                shows whether to enable the port or to disable it.
      * @param enablementService    pointer to a function that will do the additinal steps that may be needed for this port to execute this enablement command.
      * @param port                 Index of the port that has to be enabled/disabled.
      * @return STATUS_SUCCESS on success and any other error code otherwise.
    */
    status_e portEnableDisable( OMX_COMMANDTYPE enCmd, serviceFunc enablementService, VisionCamPort_e port );

    /** @fn status_e PreemptionService()
      * Handle preemption events. Takes care of all the steps needed in order preempt camera without bothering its normal work.
      * @return STATUS_SUCCESS on success and any other error code otherwise.
    */
    status_e PreemptionService();

    /** @fn friend thread_ret_t PreemptionThreadLauncher( void *arg )
      * Started by OMXVisionCam::EventHandler(), this thread function, call preemption service.
    */
    friend thread_ret_t PreemptionThreadLauncher( void *arg );

    /** @fn status_e populatePort( VisionCamPort_e port )
      * Registers all buffers, that user has passed to OMXVisionCam, to OMX camera.
      * @param port     specifies tha port on which these buffers must be supplied.
      * @return STATUS_SUCCESS on success and any other error code otherwise.
    */
    status_e populatePort( VisionCamPort_e port );

    /** @fn static inline status_e populatePortSrvc( OMXVisionCam *inst, void *in )
      * Function that may be called during camera state transitions or port eneblement.
      * @return STATUS_SUCCESS on success and any other error code otherwise.
    */
    static inline status_e populatePortSrvc( OMXVisionCam *inst, void *in )
    {
        VisionCamPort_e whichPort = *(VisionCamPort_e*)in;
        return inst->populatePort( whichPort );
    }

    /** @fn status_e fillPortBuffers( VisionCamPort_e port )
      * Requests the camera to start wrtiting into buffers suppiled on a port.
      * This is needed in some state transitions and sometimes during port enable command, depending on camera state.
      * Basically this is needed when preview is started.
      * @param port     the port on which buffers are supplied.
      * @return STATUS_SUCCESS on success and any other error code otherwise.
    */
    status_e fillPortBuffers( VisionCamPort_e port );

    /** @fn static inline status_e populatePortSrvc( OMXVisionCam *inst, void *in )
      * Function that may be called during camera state transitions or port eneblement.
      * @return STATUS_SUCCESS on success and any other error code otherwise.
    */
    static inline status_e fillPortBuffersSrvc( OMXVisionCam *inst, void *in )
    {
        VisionCamPort_e whichPort = *(VisionCamPort_e*)in;
        return inst->fillPortBuffers( whichPort );
    }

    /** @fn status_e freePortBuffers( VisionCamPort_e port )
      * Request the camera to unregister all buffers supplied to a port.
      * Internally, the camera will  free its buffer headers.
      * Basically this is done by the time preview is stopped.
      * @param port     the port for which buffers must be released.
      * @return STATUS_SUCCESS on success and any other error code otherwise.
    */
    status_e freePortBuffers( VisionCamPort_e port );

    /** @fn static inline status_e freePortBuffersSrvc( OMXVisionCam *inst, void *in )
      * Function that may be called during camera state transitions or port eneblement.
      * @return STATUS_SUCCESS on success and any other error code otherwise.
    */
    static inline status_e freePortBuffersSrvc( OMXVisionCam *inst, void *in )
    {
        VisionCamPort_e whichPort = *((VisionCamPort_e*)in);
        return inst->freePortBuffers( whichPort );
    }

    /**
    */
    typedef status_e (OMXVisionCam::*setParameterFuncPtr_t)(void *param, size_t size, VisionCamPort_e port);
    VisionCamExecutionService <OMXVisionCam, OMXVisionCam::setParameterFuncPtr_t> *mSetParameterExecutor;
    bool_e initParameterSetters();
    bool_e registerParameterSetters(bool_e *checkTable);

    status_e setColorSpace          (void *param, size_t size, VisionCamPort_e port);
    status_e setBrightness          (void *param, size_t size, VisionCamPort_e port);
    status_e setContrast            (void *param, size_t size, VisionCamPort_e port);
    status_e setSharpness           (void *param, size_t size, VisionCamPort_e port);
    status_e setSaturation          (void *param, size_t size, VisionCamPort_e port);
    status_e setFrameRate_Fixed     (void *param, size_t size, VisionCamPort_e port);
    status_e setFrameRate_Variable  (void *param, size_t size, VisionCamPort_e port);
    status_e setFlicker             (void *param, size_t size, VisionCamPort_e port);
    status_e setCrop                (void *param, size_t size, VisionCamPort_e port);
    status_e setStereoInfo          (void *param, size_t size, VisionCamPort_e port);
    status_e setCameraOperatingMode (void *param, size_t size, VisionCamPort_e port);
    status_e setSensor              (void *param, size_t size, VisionCamPort_e port);
    status_e setExposureCompensation(void *param, size_t size, VisionCamPort_e port);
    status_e setResolution          (void *param, size_t size, VisionCamPort_e port);
    status_e setManualExporureTime  (void *param, size_t size, VisionCamPort_e port);
    status_e setISO                 (void *param, size_t size, VisionCamPort_e port);
    status_e setWhiteBalanceMode    (void *param, size_t size, VisionCamPort_e port);
    status_e setColorTemp           (void *param, size_t size, VisionCamPort_e port);
    status_e setMirror              (void *param, size_t size, VisionCamPort_e port);
    status_e setImagePyramid        (void *param, size_t size, VisionCamPort_e port);
    status_e getImagePyramid        (void *param, size_t size, VisionCamPort_e port);

#if ( defined(DUCATI_1_5) || defined(DUCATI_2_0) ) && defined(OMX_CAMERA_SUPPORTS_MANUAL_CONTROLS)
    status_e setAWBminDelayTime     (void *param, size_t size, VisionCamPort_e port);
    status_e setGestureInfo         (void *param, size_t size, VisionCamPort_e port);
    status_e setAGC_MinimumDelay    (void *param, size_t size, VisionCamPort_e port);
    status_e setAGC_LowThreshold    (void *param, size_t size, VisionCamPort_e port);
    status_e setAGC_HighThreshold   (void *param, size_t size, VisionCamPort_e port);
#endif // ((DUCATI_1_5) || defined(DUCATI_2_0) ) && (OMX_CAMERA_SUPPORTS_MANUAL_CONTROLS)

    status_e setPreviewHeight       (void* param, uint32_t size, VisionCamPort_e port);
    status_e setPreaviewWidth       (void* param, uint32_t size, VisionCamPort_e port);
    status_e startAutoFocus         (void* param, uint32_t size, VisionCamPort_e port);
    status_e startManualFocus       (void* param, uint32_t size, VisionCamPort_e port);
#ifndef EXPORTED_3A
    status_e setWBalColorGains      (void* param, uint32_t size, VisionCamPort_e port);
#endif // EXPORTED_3A
#ifndef EXPORTED_3A
    status_e setGammaTableColorGains(void* param, uint32_t size, VisionCamPort_e port);
#endif // EXPORTED_3A

    status_e setFormatRotation      (void* param, uint32_t size, VisionCamPort_e port);
    status_e setPreviewRotation     (void* param, uint32_t size, VisionCamPort_e port);

#if defined (EXPORTED_3A)
    status_e startCollectingMaual3AParams   (void* param, uint32_t size, VisionCamPort_e port);
    status_e setManual3AParam               (void* param, uint32_t size, VisionCamPort_e port);
    status_e applyCollectedManual3AParams   (void* param, uint32_t size, VisionCamPort_e port);
    status_e resetManual3AParams    (void* param, uint32_t size, VisionCamPort_e port);
#endif // EXPORTED_3A

public:
    /** @fn status_e waitForFocus()
      * Waits for focus reached event.
      * @return STATUS_SUCCESS on success and any other error code otherwise.
    */
    status_e waitForFocus();

    /** @fn void frameReceivedSrvc(void *data)
      * Called inside frame thread, when a frame arrives,
      * this function pareses the frame data and sends it to user via preview callback.
      * @note In cases when a frame packing option is enabled and more then one output ports are running,
      * then the frames from different ports are put into a packet and are sent to user only when all the ports have sent a frame.
      * @see thread_t mFrameThread
      * @see queue_t *mFrameMessageQ
    */
    void frameReceivedSrvc(void *data);

    /** @var thread_t mFrameThread
      * The thread that waits for frame event.
      * This thead is responsible for packing a frames, if needed, and sending them to VisionCam client.
      *
      * @see void frameReceivedSrvc(void *data)
      * @see queue_t *mFrameMessageQ
    */
    thread_t mFrameThread;

    /** @var queue_t *mFrameMessageQ
      *
    */
    queue_t *mFrameMessageQ;
//    SimpleMsg_t mFrameMessage;
    semaphore_t mFrameSem;
private:
    bool_e mFlushInProcess;
    VisionCamPreemptionActivity_e mPreemptionState;

    OMX_PRIORITYMGMTTYPE mVisionCamPriority;
    VisionCamFrame ** mFrameDescriptors[VCAM_PORT_MAX];
    VCAM_ComponentContext mCurGreContext;
    OMX_VERSIONTYPE *mLocalVersion;
    OMX_CALLBACKTYPE mGreCallbacks;

    BuffersInUse_t mBuffersInUse[VCAM_PORT_MAX];

    mutex_t mFrameBufferLock;
    mutex_t mUserRequestLock;

    semaphore_t mGreLocalSem;
    semaphore_t mGreFocusSem;
    semaphore_t mFrameReceivedSem;

    list_t *mEventSignalQ;

    uint32_t mManualFocusDistance;

    int32_t mFaceDetectionEnabled;
    bool_e mReturnToExecuting;

    VisionCamClientNotifier mClientNotifier;

    VisionCamFramePack mFramePackage;
    bool_e mUseFramePackaging;

#if defined(EXPORTED_3A)
    VisionCam_3A_Export m3A_Export;
#endif
    /** @var mSensorsAvailable[VCAM_Max_Supported_Sensor_Count]
      * Stores the indexes of all supported sensors attached to the device.
    */
    VCamSensorAvailable mDetectedSensors;
    int32_t getAvailableSensors( VCamSensorAvailable& s );

#define sen_dump
#ifdef sen_dump
    void dumpDetectedSensors();
#endif //sen_dump

#define GET_SENSOR_CAPS 0
#if GET_SENSOR_CAPS
    void getSensorCaps(VCamSensorInfo& s );
#endif // GET_SENSOR_CAPS
    void (OMXVisionCam::*pFrameReceivedSrvc)(void *);

    enum {
        ePending_Min,
        ePending_Focus = ePending_Min,
        ePending_Max
    };

//    typedef status_e (OMXVisionCam::*startAutofocusFuncPtr)(VisionCamFocusMode focusMode);
//    VisionCamExecutionService <OMXVisionCam, OMXVisionCam::startAutofocusFuncPtr>*PendingConfigs;

    typedef status_e (OMXVisionCam::*pendingConfigsFunPtr_t)(uint32_t /*focusMode*/);
    VisionCamExecutionService <OMXVisionCam, OMXVisionCam::pendingConfigsFunPtr_t>*mPendingConfigs;


    void FirstFrameFunc(void * data);

    void FrameReceivedFunc(void * data);

#ifndef EXPORTED_3A
#ifdef _USE_GAMMA_RESET_HC_
    bool_e mGammaResetPolulated;
#endif /// _USE_GAMMA_RESET_HC_
#endif // EXPORTED_3A

#if TIME_PROFILE
    VisionCamTimePtofile * mTimeProfiler[ VCAM_TIME_TARGET_MAX ];
    void PopulateTimeProfiler();
#endif
};

#endif
