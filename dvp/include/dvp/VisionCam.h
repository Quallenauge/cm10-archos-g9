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

#ifndef _VISION_CAM_H_
#define _VISION_CAM_H_

#include <sosal/sosal.h>
#include <dvp/dvp.h>
#include <dvp/dvp_debug.h>

#ifdef EXPORTED_3A
#include <dvp/VisionCam_3A_ExportTypes.h>
#endif // EXPORTED_3A

/** MAX_FACES_COUNT
  *
  * Maximum number of face coordinates
  * that face detect algorithm can output.
*/
const uint32_t MAX_FACES_COUNT = 35;

/** VCAM_NUM_BUFFERS
  *
  * The maximum number of buffers that
  * VisionCam would be able to operate with.
*/
const uint32_t VCAM_NUM_BUFFERS = 8;

#if (TARGET_BOARD_PLATFORM == omap4)
/** Maximum supported camera number.
*/
const int32_t VCAM_Max_Supported_Sensor_Count = 3;
#elif (TARGET_BOARD_PLATFORM == omap5)
/** Maximum supported camera number.
*/
const int32_t VCAM_Max_Supported_Sensor_Count = 4;
#endif

#define VCAM_DO_IF_FAILED(ret, statement, action)   {\
    ret = statement;\
    if (STATUS_FAILED(ret)) { \
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR: VCAM Failure %d in function %s on file:line %s:%u\n", ret, __FUNCTION__, __FILE__, __LINE__);\
        action;\
    }\
}

#define VCAM_BREAK_IF_FAILED(ret, statement)        VCAM_DO_IF_FAILED(ret, statement, break)
#define VCAM_RETURN_IF_FAILED(ret, statement)       VCAM_DO_IF_FAILED(ret, statement, return ret)
#define VCAM_CONTINUE_IF_FAILED(ret, statement)     VCAM_DO_IF_FAILED(ret, statement, continue)
#define VCAM_COMPLAIN_IF_FAILED(ret, statement)     VCAM_DO_IF_FAILED(ret, statement, )
#define VCAM_GOTO_IF_FAILED(ret, statement, label)  VCAM_DO_IF_FAILED(ret, statement, goto label)

#define TIME_PROFILE 0

/** @enum VisionCamCmd_e
  *
  * Enumeration containing the all valid commands
  * that could be accepted by VisionCam.
  * @see VisionCam::sendCommand().
*/
typedef enum _vcam_cmds_e
{
    VCAM_CMD_PREVIEW_START = 0, /**< Start preview command. */
    VCAM_CMD_PREVIEW_STOP,      /**< Stop preview command. */
    VCAM_EXTRA_DATA_START,      /**< Enable transfering of e certain image meta data type.
                                  @see VisionCamExtraDataType_e.
                                */

    VCAM_EXTRA_DATA_STOP,       /**< Disable transfering of e certain image meta data type.
                                  @see VisionCamExtraDataType_e.
                                */

    VCAM_CMD_LOCK_AWB,          /**< Lock of auto white balance coeficients. */
    VCAM_CMD_LOCK_AE,           /**< Lock exposure */
    VCAM_CMD_QUIT,              /**< */
#if TIME_PROFILE
    VCAM_DUMP_TIMES,            /**< Performance measurment option. */
#endif
    VCAM_CMD_FACE_DETECTION,    /**< Enable/Disable face detection algo.
                                  @see bool_e
                                */

    VCAM_CMD_FREEZE_AWB_PARAMS, /**< Freeze white balance parameters aliance.
                                  Auto white balance algorithm continues to work but its output is not applied,
                                  for a certain amount of time.
                                  After this time expires WB parameter are immediately set to the most recent calculated.
                                  A pointer to an unsigned integer representing a time in millseconds.
                                  Minimum freeze time is 0 and maximum 10000.
                                */

    VCAM_CMD_FREEZE_AGC_PARAMS, /**< Freeze exposure algorithm aliance.
                                  Auto white balance alorythm continues to work but its output is not applied,
                                  for a certain amount of time.
                                  After this time expires WB parameter are imediately set so the most recent calculated.
                                  A pointer to an unsigned integer representing a time in millseconds.
                                  Minimum freeze time is 0 and maximum 10000.
                                */

    VCAM_CMD_SET_CLIENT_NOTIFICATION_CALLBACK, /**< Set client notification handler.
                                                 @see VisionCamClientNotifierCallback
                                                */
    VCAM_CMD_PACK_FRAMES,       /**< Enables frame packing.
                                    In cases when more than one frame sources are selected
                                    (for example preiew and video port in OMX camera) this option ensures that
                                    the frames from all sources will be passed to VisionCam client
                                    in a single call to its preview callback.
                                    @see VisionCamFramePack
                                */
    VCAM_CMD_FACE_DETECTION_RAW,

    VCAM_CMD_MAX
} VisionCamCmd_e;

/** @enum VisionCamParam_e
*
*   Enumeration listing possible camera parameters.
*   These values are passed to VisionCam::setParameter().
*   @see VisionCam::setParameter().
*/
typedef enum _vcam_params_e
{
    VCAM_PARAM_MIN = 0x7F000001,    /**< To avoid collisions with VisionCamCmd_e. */

    VCAM_PARAM_DO_AUTOFOCUS,        /**< Start auto focus of a given type @see VisionCamFocusMode. */
    VCAM_PARAM_DO_MANUALFOCUS,      /**< Start manual focus; a value representing
                                         focal distance must be passed. */

    VCAM_PARAM_CONTRAST,            /**< Set the contrast parameter; a value,
                                         in the range of -100 to 100 must be passed. */

    VCAM_PARAM_SHARPNESS,           /**< Set the sharpness parameter; a value,
                                        in the range of -100 to 100 must be passed. */

    VCAM_PARAM_BRIGHTNESS,          /**< Brighness parameter; a value,
                                        in the range of 0 to 200 must be passed. */

    VCAM_PARAM_SATURATION,          /**< Saturation parameter; a value,
                                        in the range of -100 to 100 must be passed. */

    VCAM_PARAM_HEIGHT,              /**< Preview frame height.
                                        This will be applied before next start preview command.
                                        If the buffers allocated, at the momonet of next preview start, are not
                                        with corresponding dimensions, you might receive corrupted frames
                                        or even memory corruptions may appear.
                                        This option if not preffered since preview width and height are strongly dependent.
                                        use VCAM_PARAM_RESOLUTION instead.
                                        @see VCAM_PARAM_RESOLUTION .
                                        */

    VCAM_PARAM_WIDTH,               /**< Preview frame width.
                                        This will be applied before next start preview command.
                                        If the buffers allocated, at the momonet of next preview start, do not
                                        have corresponding dimensions, you might receive corrupted frames
                                        or even memory corruptions may appear.
                                        This option isn't preffered since preview width and height are strongly dependent to each other.
                                        use VCAM_PARAM_RESOLUTION instead.
                                        @see VCAM_PARAM_RESOLUTION .
                                        */

    VCAM_PARAM_FPS_FIXED,           /**< Frame rate. */

    VCAM_PARAM_FPS_VAR,             /**< Set the variable frame rate.
                                        A pointer to a structure of type VisionCamVarFramerateType must be passed
                                        to specify minimum and maximum fps.
                                        Variable frame rate means that fps is constantly recalculated
                                        according to current light conditions.
                                        @see VisionCamVarFramerateType .
                                    */

    VCAM_PARAM_COLOR_SPACE_FOURCC,  /**< Color space that will be used.
                                        This should match the color space option,
                                        passed to buffer allocating API.
                                        @see _fourcc
                                    */

    VCAM_PARAM_NAME,                /**< When the input video is read from a file, this is the file name */

    VCAM_PARAM_FLICKER,             /**< Set flicker avoiding parameter.
                                        @see VisionCamFlickerType .
                                    */

    VCAM_PARAM_CROP,                /**< Crop option for preview frame.
                                        @see VisionCamRectType .
                                    */
    VCAM_PARAM_CAP_MODE,            /**< Camera operating mode.
                                        different operating modes provide different camera functionalities.
                                        @see VisionCamCaptureMode .
                                    */

    VCAM_PARAM_SENSOR_SELECT,       /**< Active camera.
                                        This can be done either during preview or when preview is stopped.
                                        @see VisionCamSensorSelection.
                                        Where :
                                        primary sensor: the sensor at the back of the device.
                                        secondary sensor: the sensor at the front of the device (user facing sensor).
                                        stereo sensor: in case of stereo camera presence;
                                        this corresponds to the couple of sensors located at the same side.
                                    */

    VCAM_PARAM_EXPOSURE_COMPENSATION, /**< EV compensation measured in ISOs.*/

    VCAM_PARAM_RESOLUTION,          /**< Preview resolution.
                                        This requires preview stop and reallocating the image buffers.
                                        If preview is running: VisionCam::setParameter() retuns STATUS_INVALID_STATE.
                                        A pointer to a value of VisionCamResolIdex type must be passed.
                                        @see VisionCamResolIdex
                                        @see VisionCamResolutions
                                        @see VisionCamResType
                                    */

    VCAM_PARAM_MANUAL_EXPOSURE,     /**< Manual exposure time.
                                        This affects the frame rate.
                                        A pointer to an unsigned integer value must be passed.
                                        This value must be in the range of 30 to 100
                                        and represents the exposition time in milliseconds for each frame.
                                    */

    VCAM_PARAM_AWB_MODE,            /**< White balance mode.
                                        @see VisionCamWhiteBalType.
                                    */

    VCAM_PARAM_COLOR_TEMP,          /**< Color temperature in Kelvins. */

    VCAM_PARAM_EXPOSURE_ISO,        /**< Exposure ISO.
                                        An unsigned integer value, in the range of 100 to 800, must be passed.
                                    */

    VCAM_PARAM_WB_COLOR_GAINS,      /**< White balance color gain manual control.
                                        Refers to the color channels of a non-sompressed image.
                                        This is specific setting, valid only in gesture operating modes:
                                        VCAM_GESTURE_MODE and VCAM_STEREO_GEST_MODE (@see VisionCamCaptureMode).
                                        @see VisionCamWhiteBalGains.
                                        @note This functionallity might not be enabled for some devices.
                                    */

    VCAM_PARAM_GAMMA_TBLS,          /**< 3A gamma table coefficients manual control.
                                        This is specific setting, valid only in gesture operating modes:
                                        VCAM_GESTURE_MODE and VCAM_STEREO_GEST_MODE (@see VisionCamCaptureMode).
                                        @see VisionCamGammaTableType
                                    */
    VCAM_PARAM_ROTATION,            /**< Preview frame rotation.
                                        Pointer to an unsigned integer value must be passed,
                                        which will represent the degrees of rotation.
                                        Possible values are: 0, 90, 180, 270 in degrees.
                                        @see VisionCamRotation_e
                                    */

    VCAM_PARAM_MIRROR,              /**< Sensor mirroring depending on the sensor,
                                        this setting will provide HW or SW mirroring of the image.
                                        @see VisionCamMirrorType
                                    */

    VCAM_PARAM_STEREO_INFO,         /**< Stereo image layout and subsampling.
                                      @see VisionCamStereoInfo
                                    */

    VCAM_PARAM_AWB_MIN_DELAY_TIME,  /**< This will stop white balance algorithm for a certain period of time.
                                        When this time expires, WB algorithm will start again and will smoothly
                                        change WB parameters until they reach the values that suit to the light conditions.
                                        Parameter is pointer to unsigned integer value,
                                        which represents the delay time in millseconds.
                                    */

    VCAM_PARAM_GESTURES_INFO,       /**< Notify the camera auto algorithms that a gesture is detected, and gives location
                                        of the region of interest.
                                    */

    VCAM_PARAM_AGC_MIN_DELAY_TIME,  /**< This will stop exposure gain recalculations for a certain period of time.
                                        When this time expires, exposure algorithms will smooth
                                        the light conditions.
                                        Parameter is pointer to unsigned integer value,
                                        which represents the delay time in millseconds.
                                    */

    VCAM_PARAM_AGC_LOW_TH,          /**< Automatic gain control low threshold.
                                        Pointer to an integer value must be passed.
                                        Ragne is 0 - 10000.
                                    */

    VCAM_PARAM_AGC_HIGH_TH,         /**< Automatic gain control high threshold.
                                        Pointer to an integer value must be passed.
                                        Ragne is 0 - 10000.
                                    */

    VCAM_PARAM_2DBUFFER_DIM,        /**< Used to get buffer allocation information.
                                      This information is limited to 2D buffer dimensions (width and height).
                                      @see VisionCamResType
                                    */

    VCAM_PARAM_PATH,                /**<
                                    */
    VCAM_PARAM_EXPORTED_3A_HOLD,  /**<
                                          */

    VCAM_PARAM_EXPORTED_3A_SET,  /**<
                                          */

    VCAM_PARAM_EXPORTED_3A_APPLY,  /**<
                                          */

    VCAM_PARAM_EXPORTED_3A_RESET,  /**<
                                          */

    VCAM_PARAM_IMAGE_PYRAMID,      /**< Set image pyramid levels and step.
                                        A pointer to a structure of type VisionCamImagePyramidType must be passed
                                        to specify the levels count and scaling factor for image pyramids.
                                        @see VisionCamImagePyramidType.
                                   */

    VCAM_PARAM_PREVIEW_TEXTURE,     /*!< When using VCAM from a User process on Android, this allows for using a surfaceTexture as a capture buffer. */

    VCAM_PARAM_MAX
} VisionCamParam_e;

/** @enum VisionCamExtraDataType_e
  * Defines which meta data type to be transferred.
  * @see VCAM_EXTRA_DATA_START
  * @see VCAM_EXTRA_DATA_STOP
*/
typedef enum _vcam_extradatatypes_e
{
   VCAM_EXTRA_DATA_NONE = 0,
//     VCAM_EXIF_ATTRIBUTES,
    VCAM_ANCILLARY_DATA,
    VCAM_WHITE_BALANCE,
    VCAM_UNSATURATED_REGIONS,
    VCAM_FACE_DETECTION,
//     VCAM_BARCODE_DETECTION,
//     VCAM_FRONT_OBJECT_DETECTION,
//     VCAM_MOTION_ESTIMATION,
    VCAM_MTIS_TYPE,
//     VCAM_DISTANCE_ESTIMATION,
    VCAM_HISTOGRAM,
    VCAM_FOCUS_REGION,
//     VCAM_EXTRA_DATA_PAN_AND_SCAN,
//     VCAM_RAW_FORMAT,
//     VCAM_SENSOR_TYPE,
//     VCAM_SENSOR_CUSTOM_DATA_LENGTH,
//     VCAM_SENSOR_CUSTOM_DATA,
    VCAM_MANUAL_WHITE_BALANCE,
    VCAM_FACE_DETECTION_RAW,
    VCAM_HMS_GAMMA,
    VCAM_IMAGE_PYRAMID,

    VCAM_EXTRA_DATA_TYPE_MAX
} VisionCamExtraDataType_e;

/** @enum VisionCamFlickerType
  * Possible flicker avoidance modes.
  * @see VCAM_PARAM_FLICKER
*/
typedef enum _flicker_type
{
    FLICKER_OFF,                    /**< No flicker correction. */
    FLICKER_AUTO,                   /**< Automatic flicker correction. */
    FLICKER_50Hz,                   /**< 50 Hz flicker  correction. */
    FLICKER_60Hz,                   /**< 60 Hz flicker correction. */
    FLICKER_100Hz = FLICKER_50Hz,   /**< 100 Hz flicker correction. */
    FLICKER_120Hz = FLICKER_60Hz,   /**< 120 Hz flicker correction. */

    FLICKER_MAX = 0X7fffffff
}VisionCamFlickerType;

/** @enum VisionCamRotation_e
  * Enumatarion that is used to configure
  * HW rotation of preview frames.
  * @see VCAM_PARAM_ROTATION
*/
typedef enum _vcam_rotation_e {
    VCAM_ROTATION_0     = 0,    /**< No rotation. */
    VCAM_ROTATION_90    = 90,   /**< Rotate 90 degrees right. */
    VCAM_ROTATION_180   = 180,  /**< Rotate 180 degrees */
    VCAM_ROTATION_270   = 270   /**< Rotate 270 degrees right*/
}VisionCamRotation_e;

/** @enum VisionCamSensorSelection
  * Defines different sensors.
  * @see VCAM_PARAM_SENSOR_SELECT
*/
typedef enum _sensor_selection
{
    VCAM_SENSOR_PRIMARY = 0,    /**< Usually this is the sensor on the back of the device. */
    VCAM_SENSOR_SECONDARY = 1,  /**< usually, this is the sensor at the front of the device. */
    VCAM_SENSOR_STEREO = 2,     /**< In case os stereo cameras, this is stereo use case. */

    VCAM_SENSOR_MAX = 0x7fffffff
}VisionCamSensorSelection;

/** @enum VisionCamStereoLayout
  * Defines possible stereo imaging layouts.
  * @see VisionCamStereoInfo
*/
typedef enum _stereo_layout
{
    VCAM_STEREO_LAYOUT_TOPBOTTOM,   /**< Left and right frames are located one above the other. */
    VCAM_STEREO_LAYOUT_LEFTRIGHT,   /**< Left and right frames are located next to each other. */

    VCAM_STEREO_LAYOUT_MAX
} VisionCamStereoLayout;

/** @enum VisionCamCaptureMode
  * Defines possible camera operating modes.
*/
typedef enum _capture_mode
{
    VCAM_VIDEO_NORMAL,      /**< Normal vide processing pipe. */
    VCAM_VIDEO_HIGH_SPEED,  /**< High speed video processing pipe. */
    VCAM_GESTURE_MODE,      /**< Video pipe optimized for gesture computations. */
    VCAM_STEREO_MODE,       /**< Normal stereo pipe. */
    VCAM_STEREO_GEST_MODE,  /**< stereo pipe, optimized for gesture computations. */
#if defined(__QNX__)
    VCAM_VIEWFINDER_MODE,
#endif
    VCAM_GESTURE_DUAL_SENSOR_MODE,

    VCAM_CAP_MODE_MAX
}VisionCamCaptureMode;

/** @enum VisionCamGestureEvent_e
  * Defines different gesture events.
*/
typedef enum _vcam_gesture_event_e {
    VCAM_GESTURE_EVENT_INVALID = 0, /**< No or invalid gesture event. Could also mean incompatible software version. */
    VCAM_GESTURE_EVENT_SWIPE_LEFT,  /**< Swipe to the left. */
    VCAM_GESTURE_EVENT_SWIPE_RIGHT, /**< Swite to the right. */
    VCAM_GESTURE_EVENT_FIST_LEFT,   /**< Fist turned to left. */
    VCAM_GESTURE_EVENT_FIST_RIGHT,  /**< Fist turned to right. */

    VCAM_GESTURE_EVENT_MAX,
} VisionCamGestureEvent_e;

/** @enum VisionCamFocusMode
  * Defines focus modes.
*/
typedef enum _vcam_focus_mode
{
    VCAM_FOCUS_CONTROL_ON = 0,                  /**< Focus is enabled. */
    VCAM_FOCUS_CONTROL_OFF,                     /**< Focus is disabled. */
    VCAM_FOCUS_CONTROL_AUTO,                    /**< Continuous autofocus is enabled. */
    VCAM_FOCUS_CONTROL_AUTO_LOCK,               /**< Single autoficus enabled - once fous position of lens is reached, focus algorithm stops. */
    /// TI extension follow
    VCAM_FOCUS_CONTRO_AUTO_MACRO,               /**< Auto focus in macro mode. */
    VCAM_FOCUS_CONTROL_AUTO_INFINITY,           /**< Infinity focus mode. */
    VCAM_FOCUS_FACE_PRIORITY_MODE,              /**< Emphasis of focus algorithm is the output of face detect. */
    VCAM_FOCUS_REGION_PRIORITY_MODE,            /**< Focuses on given regions. */
    VCAM_FOCUS_CONTROL_HYPERFOCAL,              /**<  */
    VCAM_FOCUS_CONTROL_PORTRAIT,                /**<  */
    VCAM_FOCUS_CONTROL_EXTENDED,                /**<  */
    VCAM_FOCUS_CONTROL_CONTINOUS_NORMAL,        /**<  */
    VCAM_FOCUS_CONTROL_CONTINOUS_EXTENDED,      /**<  */
    VCAM_FOCUS_FACE_PRIORITY_CONTINOUS_MODE,    /**<  */
    VCAM_FOCUS_REGION_PRIORITY_CONTINOUS_MODE,  /**<  */
    VCAM_FOCUS_CONTROL_MAX
}VisionCamFocusMode;

/** @struct VisionCamStereoInfo
  * Configuration for stereo operating modes.
*/
typedef struct _vcam_stereo_info_t {
    VisionCamStereoLayout layout;       /**< Stereo image layout: top bottom or side by side.*/
    uint32_t               subsampling; /**< Set to 1 if you are setting your
                                            dimensions manually to double to desire value.
                                            Set to 2 if you want to compress the two images
                                            into a normal image.
                                        */
} VisionCamStereoInfo;

/** @enum VisionCamLockState
  * Defines enable and disable modes used for variety of settings.
*/
typedef enum _vcam_lock_state
{
    UNLOCKED    = false_e,
    LOCKED      = true_e
}VisionCamLockState;

/** @enum VisionCamResolIdex
  * Defines VisionCam resolutions.
  * @see VisionCamResolutions
  * @see VisionCamResType
*/
typedef enum _e_resol_index
{
    VCAM_RES_2MP = 0,   /**< 1600 x 1200 pixels */
    VCAM_RES_1_25MP,    /**< 1300 x 976 pixels */
    VCAM_RES_720p,      /**< 1280 x 720 pixels */
    VCAM_RES_SVGA,      /**< 800 x 480 pixels */
    VCAM_RES_D1PAL,     /**< 720 x 576 pixels */
    VCAM_RES_D1NTSC,    /**< 720 x 480 pixels */
    VCAM_RES_PAL,       /**< 768 x 576 pixels */
    VCAM_RES_VGA,       /**< 640 x 480 pixels */
    VCAM_RES_CIF,       /**< 352 x 288 pixels */
    VCAM_RES_QVGA,      /**< 320 x 240 pixels */
    VCAM_RES_QCIF,      /**< 176 x 144 pixels */
    VCAM_RES_QQVGA,     /**< 160 x 120 pixels */
    VCAM_RES_SQCIF,     /**< 128 x 96 pixels */
    VCAM_RES_SXVGA,     /**< 1280 x 960 pixels */

    VCAM_RESOL_MAX
}VisionCamResolIdex;

/** @struct VisionCamResType
  * Packs resolution index and resolutin sizes for supported resolutins.
  * @see VisionCamResolIdex
*/
typedef struct _res_type
{
    VisionCamResolIdex mResIdx; /**< VisionCam defined resolution index. */
    uint32_t            mWidth; /**< Width in pixels. */
    uint32_t            mHeight;/**< Height in pixels. */
}VisionCamResType;

/** VisionCamResolutions[]
  * Predefined resolution indexes and sizes.
  * The only possible for VisionCam.
*/
const VisionCamResType VisionCamResolutions[] = {
    { VCAM_RES_2MP    , 1600, 1200 },
    { VCAM_RES_1_25MP , 1300, 976  },
    { VCAM_RES_720p   , 1280, 720  },
    { VCAM_RES_SVGA   , 800 , 480  },
    { VCAM_RES_D1PAL  , 720 , 576  },
    { VCAM_RES_D1NTSC , 720 , 480  },
    { VCAM_RES_PAL    , 768 , 576  },
    { VCAM_RES_VGA    , 640 , 480  },
    { VCAM_RES_CIF    , 352 , 288  },
    { VCAM_RES_QVGA   , 320 , 240  },
    { VCAM_RES_QCIF   , 176 , 144  },
    { VCAM_RES_QQVGA  , 160 , 120  },
    { VCAM_RES_SQCIF  , 128 , 96   },
    { VCAM_RES_SXVGA  , 1280, 960  }
};

/** @enum VisionCamObjectType
  * Defines some gesture objects.
  * These are not the only supported.
*/
typedef enum _vcam_object_type {
    VCAM_OBJECT_PALM,   /**< Palm */
    VCAM_OBJECT_FIST,   /**< Fist */
    VCAM_OBJECT_FACE,   /**< Face */

    VCAM_OBJECT_MAX
}VisionCamObjectType;

/** @enum VisionCamWhiteBalType
  * Defines different white balance modes used by VisionCam
*/
typedef enum _vcam_white_bal_type {
    VCAM_WHITE_BAL_CONTROL_OFF = 0,         /**< White balance algorithm is turned off. */
    VCAM_WHITE_BAL_CONTROL_AUTO,            /**< Automatic white balance. */
    VCAM_WHITE_BAL_CONTROL_SUNLIGHT,        /**< WB optimized for sunny light conditions. */
    VCAM_WHITE_BAL_CONTROL_CLOUDY,          /**< WB optimized for cloudy light conditions. */
    VCAM_WHITE_BAL_CONTROL_SHADE,           /**< WB optimized for shaded light conditions. */
    VCAM_WHITE_BAL_CONTROL_TUNGSTEN,        /**< WB optimized for tungsten (worm) light conditions. */
    VCAM_WHITE_BAL_CONTROL_FLUORESCENT,     /**< WB optimized for flourescent light conditions. */
    VCAM_WHITE_BAL_CONTROL_INCANDESCENT,    /**< WB optimized for incadescent (warm) light conditions. */
    VCAM_WHITE_BAL_CONTROL_FLASH,           /**< WB optimized for camera flash. */
    VCAM_WHITE_BAL_CONTROL_HORIZON,         /**< WB optimized for landscape sunny conditions.. */
    VCAM_WHITE_BAL_CONTROL_FACEPRIORITYMODE = 0X7F000001,

    VCAM_WHITE_BAL_CONTROL_MAX
}VisionCamWhiteBalType;

/** @enum VisionCamMirrorType
  * Mirroring of image in frame buffer.
*/
typedef enum _vcam_mirror_type{
    VCAM_MIRROR_NONE,           /**< No mirror applied. */
    VCAM_MIRROR_VERTICAL,       /**< Mirror by vertical axis. */
    VCAM_MIRROR_HORIZONTAL,     /**< Mirror by horizontal axis. */
    VCAM_MIRROR_BOTH,           /**< Mirror by bith vertical and horizontal axis. */

    VCAM_MIRROR_MAX
}VisionCamMirrorType;

/** @enum VisionCamPort_e
  * Defines different camera output ports.
  * Sometimes useful as different ports may have different configurations,
  * also every port outputs a separate image, so one could be used at one process,
  * while another - on other process.
  * @note Each frame from a port has a twin on another port. Use frame's timestamp to find the matchin one.
  *       All ports are synchronised so frames are ouput sequentally.
  *       Also note that video port is sending frames ONLY when a preview port is sending.
*/
typedef enum _vision_cam_port_e {
    VCAM_PORT_ALL = 0,                  /**< All port listed below. */
    VCAM_PORT_MIN,
    VCAM_PORT_PREVIEW = VCAM_PORT_MIN,  /**< Preview port. */
    VCAM_PORT_VIDEO,                    /**< Video port. */

    VCAM_PORT_MAX,
    VCAM_PORT_NUM = VCAM_PORT_MAX -1
}VisionCamPort_e;

/** @enum VCAM_HISTCOMPONENTTYPE
  * Histogram types.
  * @see VisionCamHistogram
*/
typedef enum VCAM_HISTCOMPONENTTYPE {
    VCAM_HISTCOMP_Y = 0,        /**< Luminance histogram (Y) */
    VCAM_HISTCOMP_YLOG,         /**< Logarithmic luminance histogram (Y)*/
    VCAM_HISTCOMP_R,            /**< Red histogram component (R)*/
    VCAM_HISTCOMP_G,            /**< Green histogram component (G)*/
    VCAM_HISTCOMP_B,            /**< Blue histogram component (B)*/
    VCAM_HISTCOMP_Cb,           /**< Chroma blue histogram component (Cb)*/
    VCAM_HISTCOMP_Cr,           /**< Chroma red histogram component (Cr) */
    VCAM_HISTCOMP_32BIT_PATCH = 0x7FFFFFFF
}VCAM_HISTCOMPONENTTYPE;


/** @struct VisionCamWhiteBalGains
  * Manual white balance gain structure.
  * Contains gain for each color channel.
  * @see VCAM_PARAM_WB_COLOR_GAINS
*/
typedef struct _white_bal_gains_type{
    uint16_t mRed;      /**< Red color channel. */
    uint16_t mGreen_r;  /**< Difference between green and red color. */
    uint16_t mGreen_b;  /**< Difference between green and blue color. */
    uint16_t mBlue;     /**< Blue channel. */
}VisionCamWhiteBalGains;

/** @struct VisionCamAncillary
  * Ancillary data type definition.
*/
typedef  struct _ancillary_type {
    uint32_t             nAncillaryDataVersion;     /**< Version of the ancillary data definition implemented by the application in binary coded decimal format. */
    uint32_t             nFrameNumber;              /**< This is a free-running counter (wraps back to zero at overflow) that is applied to each frame from the sensor whether that frame is not sent or is sent in multiple formats to the host. */
    uint32_t             nShotNumber;               /**< Indicates the shot number in a multi-shot sequence, counting up from 1 */
    uint16_t             nInputImageHeight;         /**< Height in pixels of the input image (i.e. from sensor or video decoder) */
    uint16_t             nInputImageWidth;          /**< Width in pixels of the input image (i.e. from sensor or video decoder) */
    uint16_t             nOutputImageHeight;        /**< Height in pixels of the image sent to the host */
    uint16_t             nOutputImageWidth;         /**< Width in pixels of the image sent to the host  */
    uint16_t             nDigitalZoomFactor;        /**< Digital zoom factor: 10 bits decimal, 6 bits fractional.
                                                         @note Note: When non-square pixels are sent to the host, as indicated by the pixel aspect ratio ancillary data fields, the digital zoom factor applies in the direction that is the lesser of the pixel aspect ratio fields.
                                                    */
    int16_t               nCropCenterColumn;        /**< Defines the center of cropped region with regard to full sensor field of view.   All values are valid, 0x8000 is the left column and 0x7fff is the right column. 0 is the center column.  This is effectively a percentage ranging between +/- 50% */
    int16_t               nCropCenterRow;           /**< Defines the center of cropped region with regard to full sensor field of view.  All values are valid, 0x8000 is the top row and 0x7fff is the bottom row. 0 is the center row.  This is effectively a percentage ranging between +/- 50%.  */
    uint16_t             nOpticalZoomValue;         /**< Optical zoom factor: 4 bits decimal, 12 bits fractional */
    uint8_t               nFlashConfiguration;      /**< Number Logical Flash Sources – Maximum 4 logical Flashes. For each logical flash following two fields will be filled. */
    uint8_t               nFlashUsage;              /**< 1. Bit 0 and 1 (LSB) - Flash Light Type (00b = Not used; 01b = Xenon; 10b = LED; 11b = Other)
                                                         2. Bits 2 and 3 – Flash Charging Status (00b = Not charging; 01b = Charging; 10b = Charged; 11b = Error)
                                                         3. Bits 4 and 5 – Flash Strobe Detection ( 00b = Not supported; 01b = Reserved; 10b = Strobe return light not detected; 11b = Strobe return light detected)
                                                         4. Bit 6 and 7 - Reserved1. Bit 0 and 1 (LSB) - Flash Light Type (00b = Not used; 01b = Xenon; 10b = LED; 11b = Other)
                                                    */
    uint32_t             nFlashStatus;              /**< 1.Bits 0 to 7 -  Flash Mode (bit variable). Flash can support more than one mode.
                                                           Bit 0 = Capture Flash Mode
                                                           Bit 1 = Video Light (Torch) Mode
                                                           Bit 2 = Pre-flash Mode
                                                           Bit 3 = AF Assist Mode
                                                           Bit 4 = Privacy Light Mode
                                                           Bit 5 = Red-eye Reduction Mode
                                                           Bit 6 = Other Mode 1
                                                           Bit 7 = Other Mode 2
                                                        2. Bits 8 to 23 - Flash Control Mode. 2bits/Flash Light Mode
                                                           00b = Not used
                                                           01b = Compulsory flash firing
                                                           10b = Compulsory flash suppression
                                                           11b = Auto
                                                        3. Bits 24 to 31 - Flash Intensity
                                                           0 = Flash did not fire
                                                           1 = 255 - Flash Intensity or Flash Intensity Identifier1.Bits 0 to 7 -  Flash
                                                    */
    uint8_t               nAFStatus;                /**< Auto Focus status
                                                        FAILED is a single bit independent of the other values.

                                                        The other values are mutually exclusive
                                                        OFF=not present or N/A
                                                        LOCKED=manual
                                                        RUNNING=auto, active
                                                        FROZEN=suspended
                                                    */
    uint8_t               nAWBStatus;               /**< Auto White Balance status. See explanation in AF status. */
    uint8_t               nAEStatus;                /**< Auto Exposure status See explanation in AF status. */
    uint32_t             nExposureTime;             /**< Exposure time in microseconds. */
    uint16_t             nEVCompensation;           /**< Exposure compensation */
    uint8_t               nDigitalGainValue;        /**< Digital gain setting */
    uint8_t               nAnalogGainValue;         /**< Analog gain setting */
    uint16_t             nCurrentISO;               /**< ISO setting based on reference ISO */
    uint16_t             nReferenceISO;             /**< Reference ISO for the current sensor */
    uint8_t               nApertureValue;           /**< Aperture setting */
    uint8_t               nPixelRange;              /**< Indicates the range of pixel values */
    uint16_t             nPixelAspectRatio;         /**< Indicates the width corresponding to the aspect ratio (PAR) of the pixels (samples) in the frame to which this ancillary data is attached.  The PAR always indicates the true PAR, unless otherwise indicated in the application-specific documentation. */
    uint8_t               nCameraShake;             /**< Indicates risk of camera shake. For instance, if exposure time is greater than 1/f but less than 2/f, camera shake can be considered MEDIUM_RISK. If exposure time is greater than 2/f, camera_shake can be considered a HIGH_RISK. */
    uint16_t             nFocalDistance;            /**< Distance in cm from the camera lens if possible (14 lsbs).  Upper 2 msbs indicate if the lens is set to: infinity (’10), macro (’01), in between (’00), or moving for AF search (’11). */
    uint64_t             nParameterChangeFlags;     /**< A 32-bit bit mask where each bit represents a sub-set of parameters.  A bit value of ‘0’ indicates that the associated parameter set did not change from the previous frame. A bit value of ‘1’ indicates that the parameter set has changed from the previous frame.
                                                            0: SEN_EXPOSURE (sensor exp)
                                                            1: SEN_GAIN (sensor gain ->analogue and digital are in the same field.
                                                            2: LSC_2D (2d lsc)
                                                            3: VLDC (Vertical Line Defect Correction)
                                                            4: ISIF_GAIN
                                                            5: H3A_WB
                                                            6: FLASH
                                                            7: ISP_NOISE_FILTER_1 (iss noise filter)
                                                            8: GIC (green imbalance correction)
                                                            9: CFA
                                                            10: GBCE
                                                            11: YUV444TO422 (yuv444 to 422)
                                                            12: EDGE_ENH (Edge enhancement)
                                                            13: CAR (chromatic aberration reduction)
                                                            14: LSC_RAD (radial based one - useless)
                                                            15: DFS (Dark frame subtract)
                                                            16: DPC (defect pixel correction in ipipeif)
                                                            17: DPCM
                                                            18: LPF_H (Low pass filter params for Horizontal resizing)
                                                            19: LPF_V (Low pass filter params for Vertical resizing)
                                                            20: CLAMP (ISIF black clamp configuration)
                                                            21: WB_GAINS (wb gains)
                                                            22: GAMMA (Gamma)
                                                            23: RGB2RGB_1
                                                            24: RGB2YUV
                                                            25: 3D_LUT (3d lut)
                                                            26: DPC LUT
                                                            27: DPC OTF (in ipipe)
                                                            28: RGB2RGB_2 (after gamma)
                                                            29: ISP_NOISE_FILTER_2 (iss noise filter)
                                                            30: CGS (Chroma gain suppression)
                                                            31: IPIPE_IF_DPC1 (ISIF or VP)
                                                            32: IPIPE_IF_DPC2 (from SDRAM)
                                                            33-63: reserved
                                                    */
    uint8_t               nNumFacesDetected;        /**< When Face Detect is enabled, this contains the number of faces detected. When face detect is disabled, this number should be set to 0. */
    uint8_t               nConvergenceMode;         /**< Reserved for future usage. */
    uint8_t               nConvergenceStatus;       /**< Reserved for future usage. */
    uint8_t               nDCCStatus;               /**< Reserved for future usage. */
}VisionCamAncillary;

/** @struct VisionCamHistogram
  * Histogram data type definition.
*/
typedef struct _histogram_type{
    uint32_t nBins;                          /**< The number of bins in the histogram */
    VCAM_HISTCOMPONENTTYPE eComponentType;   /**< Specifies the type of the histogram bins according to enum.
                                                 It can be selected to generate multiple component types,
                                                 then the extradata struct is repeated for each component type */
    uint32_t data[1];
}VisionCamHistogram;


/** @struct VisionCamUnsaturatedRegions
 * Unsaturated Regions data
 * The extra data having unsaturated regions data is
 * described with the following structure..
 */
typedef struct unsaturated_regions_type {
uint16_t        nPaxelsX;       /**< The number of paxels in the horizontal direction */
uint16_t        nPaxelsY;       /**< The number of paxels in the vertical direction */
uint16_t        data[1];     /**< the first value of an array of values that represent
                                 the percentage of unsaturated pixels within the associated paxel */
}VisionCamUnsaturatedRegions;

/** @struct VisionCamMTIS
 * MTIS Vendor Specific Motion estimation
 * The extra data having MTIS motion estimation data is
 * described with the following structure.
 */
typedef struct mtis_meta_data_type {
    int32_t nMaxMVh;            /**< The maximum MV for horizontal direction */
    int32_t nMaxMVv;            /**< The maximum MV for vertical direction */
    uint16_t nMVRelY[9];         /**< The mask for MV reliability */
    uint16_t nMVRelX[9];         /**< The mask for MV reliability */
    int32_t nMVh[9];            /**< The MVs for horizontal direction */
    int32_t nMVv[9];            /**< The MVs for vertical direction */
}VisionCamMTIS;

/** @struct VisionCamFocusRegion
 * For Extended Focus region Type -
 */
typedef struct focus_region_data_type {
    int32_t nLeft;
    int32_t nTop;
    uint32_t nWidth;
    uint32_t nHeight;
} VisionCamFocusRegion;

typedef struct hms_gamma_data_type {
    uint32_t nItems;                          /**< The number of bins in the histogram */
    uint8_t data[1];
} VisionCamGamma;

/**
 *  Structure describes properties of one level in image pyramid
 *  nOffset : nOffset from start of buffer in bytes
 *  nWidth  : nWidth  in pixels
 *  nHeight : nHeight in pixels
 *  nStride : nStride in bytes
 */
typedef struct _vcam_image_pyramid_item_t {
    uint32_t    nOffset;
    uint16_t    nWidth;
    uint16_t    nHeight;
    uint32_t    nStride;
} VisionCamImagePyramidItem_t;

/**
 *  The extra data that stores image pyramid data,
 *  It is described with the following structure.
 *  nLevelsCount - Number of levels of pyramid
 *  PyramidData - first element of the array that contains
 *  description of each level of pyramid.
 *  Size of the array is defined by nLevelsCount.
 */
typedef struct _vcam_image_pyramid_data_t {
    uint32_t                    nLevelsCount;
    VisionCamImagePyramidItem_t PyramidData[1];
} VisionCamImagePyramidData;

/** @struct VisionCamMetadata
  * Image meta data type definition.
*/
typedef struct _metadata_type {
    VisionCamWhiteBalGains *mAutoWBGains;   /**< Info about auto white balance algorithm output: color gains applied by AWB. */
    VisionCamWhiteBalGains *mManualWBGains; /**< Info about manual white balance algorithm : color gains applied by user. */
    VisionCamAncillary     *mAncillary;     /**< Ancillary data. */
    VisionCamHistogram     *mHistogram2D;   /**< Histogram data for 2D use cases. */
    VisionCamHistogram     *mHistogramL;    /**< Histogram data for left channel in case of stereoscopic use cases. */
    VisionCamHistogram     *mHistogramR;    /**< Histogram data for right channel in case of stereoscopic use cases. */

    VisionCamGamma     *mGamma2D;   /**< Gamma data for 2D use cases. */
    VisionCamGamma     *mGammaL;    /**< Gamma data for left channel in case of stereoscopic use cases. */
    VisionCamGamma     *mGammaR;    /**< Gamma data for right channel in case of stereoscopic use cases. */

    VisionCamUnsaturatedRegions *mUnsaturatedRegions;
    VisionCamMTIS               *mMTIS_Data;
    VisionCamFocusRegion        *mFocusRegionData;

    VisionCamImagePyramidData   *mImgPyramidData;

}VisionCamMetadata;

/** @struct VisionCamGammaTableType
  * Defines the gamma table data type, that is used while setting manual gamma table coeficients.
*/
typedef struct _gamma_tbl_type {
    int32_t   mTableSize;   /**< The size of the table. */
    uint16_t *mRedTable;    /**< Address of the table with red color coeficients. */
    uint16_t *mGreenTable;  /**< Address of the table with green color coeficients. */
    uint16_t *mBlueTable;   /**< Address of the table with blue color coeficients. */
}VisionCamGammaTableType;

/** @struct VisionCamRectType
  * Definition of rectangle type.
*/
typedef struct _rect_type {
    int32_t mLeft;      /**< x-axis coordinate of top-left corner of a rectangle. */
    int32_t mTop;       /**< y-axis coordinate of top-left corner of a rectangle. */
    uint32_t mWidth;    /**< Width of a rectangle. */
    uint32_t mHeight;   /**< Height of a rectangle. */
}VisionCamRectType;

/** @struct VisionCamFaceType
  * Data type containing the output of face detection algorithm output.
*/
typedef struct _face_type {
    /**< Detection score between 0 and 100
         0   means unknown score,
         1   means least certain,
         100 means most certain the detection is correct */
    uint32_t mScore;
    /**< Coordinates of the face */
    VisionCamRectType mFacesCoordinates;
    /**< The orientation of the axis of the detected object.
         Here roll angle is defined as the angle between the vertical axis of face and the horizontal axis.
         All angles can have the value of -180 to 180 degree in Q16 format.
         Some face detection algorithm may not be able to fill in the angles */
    int32_t mOrientationRoll;
    int32_t mOrientationYaw;
    int32_t mOrientationPitch;
    /**< Represents priority of each object when there are multiple objects detected */
    uint32_t mPriority;
}VisionCamFaceType;

/** @struct VisionCamVarFramerateType
  * Defines the minimum and maximum fps in case a variable frame rate os selected.
  * Variable frame rate, means that expositions and respectively frame rate are recalculated at any moment.
*/
typedef struct _var_frame_rate_type {
    uint32_t mMin;  /**< Minimum fps. */
    uint32_t mMax;  /**< Maximul fps. */
}VisionCamVarFramerateType;

/** @struct VisionCamImagePyramidType
  * Defines the levels count and scaling factor for image pyramids.
  * mScalingFactor is in Q16 format.
  * Image pyramid is same images with different resolutions in common buffer.
*/
typedef struct _image_pyramid_type {
    uint32_t mLevelsCount;
    uint32_t mScalingFactor; // Q16
}VisionCamImagePyramidType;

/** @struct VisionCamObjectRectType
  * Definition of type, describing an area where an object is detected.
*/
typedef struct _vcam_object_rect_type {
    VisionCamObjectType mObjType;   /**< The type ofthe object. */
    int32_t mTop;                   /**< y-axis coordinate of top-left corner of a rectangle. */
    int32_t mLeft;                  /**< x-axis coordinate of top-left corner of a rectangle. */
    uint32_t mWidth;                /**< Width of a rectangle. */
    uint32_t mHeight;               /**< Height of a rectangle. */
}VisionCamObjectRectType;

#ifdef __cplusplus
/** @struct VisionCamGestureInfo
  * Structure containing information about gesures detected inside the stream.
*/
struct  VisionCamGestureInfo {
    ~VisionCamGestureInfo()
    {
        if( mRegions )
            delete [] mRegions;
    }
    VisionCamGestureEvent_e mGestureType;   /**< Type of the gesture event. */
    uint64_t timeStamp;                     /**< Timestamp of the frame in which this is detected. */
    uint32_t mRegionsNum;                   /**< The number of the regions in frame, where it is detected. */
    VisionCamObjectRectType *mRegions;      /**< Coordinateds and gesture object for each region. */
};
#else
/** @struct VisionCamGestureInfo
  * Structure containing information about gesures detected inside the stream.
*/
typedef struct  _vcam_gesture_info_type {
    VisionCamGestureEvent_e mGestureType;   /**< Type of the gesture event. */
    uint64_t timeStamp;                     /**< Timestamp of the frame in which this is detected. */
    uint32_t mRegionsNum;                   /**< The number of the regions in frame, where it is detected. */
    VisionCamObjectRectType *mRegions;      /**< Coordinateds and gesture object for each region. */
} VisionCamGestureInfo;
#endif // __cplusplus

/** @class VisionCamClientNotifier
  * User notification class.
  * This is used to notify client when an extarnal camera event occur.
*/
class VisionCamClientNotifier {
public:
    /**< @enum VisionCamClientNotificationMsg
      * Defines some possible events that may occur outside VisionCam.
    */
    typedef enum _vcam_client_notification_msg_type{
        VCAM_MESSAGE_EMPTY,                     /**< Unknown event occur. */
        VCAM_MESSAGE_ALLOCATE_V4L2_BUFFERS,     /**< User may allocate image resources (this is in case resources were previously freed, bacause of external event). */
        VCAM_MESSAGE_FREE_V4L2_BUFFERS,         /**< User must free allocated resources. */
        VCAM_MESSAGE_PREEMPT_SUSPEND_ACTIVITY,  /**< Camera has been preempted by a higher priority process, so VisionCam will suspend. This means frames will not be received for some time. */
        VCAM_MESSAGE_PREEMPT_RESUME_ACTIVITY,   /**< VisionCam has got back its control over camera. */
        VCAM_MESSAGE_PREEMPT_WAIT_RESOURCES,    /**<  */
        VCAM_MESSAGE_PREAMPT_RESOURCES_READY,  /**<  */
        VCAM_MESSAGE_STOP_STREAMING,            /**< Tells that VisionCam's client must stop Filebased stream, Preview etc.*/

        VCAM_MESSAGE_MAX
    }VisionCamClientNotificationMsg;

    typedef void *(*_vcam_client_notification_callback_type)( _vcam_client_notification_msg_type );
    typedef _vcam_client_notification_callback_type VisionCamClientNotifierCallback;

public:
    /// Constructor
    VisionCamClientNotifier()
    {
        mNotificationCallback = NULL;
    }

    /// The notification function, that user will register.
    VisionCamClientNotifierCallback mNotificationCallback;
};

/** @class VisionCamFrame
  *
  * Frame descriptor class. Holds preview a frame
  * and carries all the data related to it.
*/
class VisionCamFrame
{
public:

    /** default contrustor */
    VisionCamFrame();

    /** copy constructor */
    VisionCamFrame(const VisionCamFrame &frame) :
        mCookie(frame.mCookie),
        mFrameBuff(frame.mFrameBuff),
        mExtraDataBuf(frame.mExtraDataBuf),
        mFrameSource(frame.mFrameSource),
        mFd(frame.mStartY),
        mTimestamp(frame.mTimestamp),
        mWidth(frame.mWidth),
        mHeight(frame.mHeight),
        mOffsetX(frame.mOffsetX),
        mOffsetY(frame.mOffsetY),
        mLength(frame.mLength),
        mExtraDataLength(frame.mExtraDataLength),
        mMetadata(frame.mMetadata),
        mDetectedFacesNum(frame.mDetectedFacesNum),
        mDetectedFacesNumRaw(frame.mDetectedFacesNumRaw)
        {
            if( mDetectedFacesNum )
                memcpy( &mFaces, &frame.mFaces, mDetectedFacesNum*sizeof(VisionCamFaceType));
            else
                memset( &mFaces, 0, sizeof(VisionCamFaceType)*MAX_FACES_COUNT);

            if( mDetectedFacesNumRaw )
                memcpy( &mFacesRaw, &frame.mFacesRaw, mDetectedFacesNumRaw*sizeof(VisionCamFaceType));
            else
                memset( &mFacesRaw, 0, sizeof(VisionCamFaceType)*MAX_FACES_COUNT);
        };

    /** This clears all values */
    void clear();

    /** This serializes all data in the frame into a flat buffer */
    size_t serialize(uint8_t *buffer, size_t len);

    /** This unserializes all the data from a flat buffer back into the instance class object */
    size_t unserialize(uint8_t *buffer, size_t len);

    /** This returns the max possible size (as currently allocated) of the frame for the purpose of allocating flat memory for serialization */
    size_t maxSerialLength();

public:
    void *mContext;                 /**< Indicates in which context this frame is set (which VisionCam instance at all). */
    void *mCookie;                  /**< The cookie data. */
    void *mFrameBuff;               /**< The image buffer. @see DVP_Image_t */
    void *mExtraDataBuf;            /**< Extra data buffer. */
    VisionCamPort_e mFrameSource;   /**< The index of the port from which this frame has arrived. */
    int32_t mFd;                    /**<  */
    int64_t mTimestamp;             /**< The timestamp for the frame.
                                      * The timestanp is unique for each frame on a port
                                      * but when multiple ports are running, the timestamp
                                      * is the same for the matching frames on different ports.
                                    */
    uint32_t mWidth;                /**< Frame width. */
    uint32_t mHeight;               /**< Frame haight. */
    uint32_t mOffsetX;              /**< The offset in X-direction of the actual frame data inside the buffer. */
    uint32_t mOffsetY;              /**< The offset in Y-direction of the actual frame data inside the buffer. */
    uint32_t mStartX;               /**<  */
    uint32_t mStartY;               /**<  */
    uint32_t mLength;               /**< The image size in bytes. */
    uint32_t mExtraDataLength;      /**< Size in bytes if extra data buffer. */
    VisionCamMetadata mMetadata;    /**< Parsed meta data. */
    uint32_t mDetectedFacesNum;     /**< Number of faces detected in this frame. */
    VisionCamFaceType mFaces[ MAX_FACES_COUNT ];    /**< Informatin about detected faces. */
    uint32_t mDetectedFacesNumRaw;     /**< Number of faces detected in this frame as a raw data from FD algo. */
    VisionCamFaceType mFacesRaw[ MAX_FACES_COUNT ];    /**< Informatin about detected faces as a raw data from FD algo. */
};


/** @class VisionCamFramePack
  * A frame holder.
  * This structure keeps the matching frames from all possinble sources.
  * This is used when packaging option is enabled.
  * Packaging is based on frame timestamp.
*/
class VisionCamFramePack
{
public:
    VisionCamFramePack(){
        for( int32_t i = 0; i < VCAM_PORT_MAX; i++) {
            mFrame[i] = NULL;
            mExpectedFrames[i] = false_e;
        }
        mTimestamp = 0;
        mIsEmpty = true_e;
    }

    VisionCamFrame *mFrame[VCAM_PORT_MAX];  /**< Addresses of all the frames from all running sources */
    int64_t         mTimestamp;             /**< Timestamp of these frames. The same for all*/

    bool_e          mExpectedFrames[VCAM_PORT_MAX]; /**< Which frames must be written,
                                                        corresponds to the frame sourcs state (running or not).
                                                    */

    bool_e          mIsEmpty;                       /**< Read only field:
                                                        true_e when there is a valid data inside mFrame[]
                                                        false_e when there is no valid data inside mFrame[]
                                                    */
};

/** @typedef Frame callback type.
*/
typedef void (*FrameCallback_f)(VisionCamFrame *cameraFrame);

/** @typedef Frame callback type,
  * used to pass multiple frames in a single shot.
*/
typedef void (*FramePackCallback_f)(VisionCamFramePack *cameraFramePacket);

/** @typedef Focus callback type.
*/
typedef void ( * FocusCallback_t )(int);

/** @enum VisionCam_e
  * This one is used when a handle to VisionCam is created.
  * Those options correspond to the vision processing mode.
*/
typedef enum _vision_cam_e {
    VISIONCAM_FILE,     /**< Stream that has already been recorded. */
    VISIONCAM_OMX,      /**< Live stream through OMX camera. */
    VISIONCAM_USB,      /**< Live stream through USB, for example with a web camera. */
    VISIONCAM_SOCKET,   /**< Live stream usinf TPC IP communication */
    VISIONCAM_CS,       /**< Live stream using Android CameraService */

    VISIONCAM_MAX,
} VisionCam_e;

/** @class VCamSensorInfo
  * Sensor information type.
*/
class VCamSensorInfo{
public:
    VCamSensorInfo(){
        nIndex = -1;
        supportedModesCnt = 0;
        memset(&supportedModes, 0, sizeof(supportedModes));
    }

    int32_t                 nIndex;
    int32_t                 supportedModesCnt;
    VisionCamCaptureMode    supportedModes/*[modesNum]*/[VCAM_CAP_MODE_MAX];
};

/**
 * @brief The VCamSensorAvailable class
 * Contains data about all the cameras detected as attached to the device.
 */
class VCamSensorAvailable {
public:
    VCamSensorAvailable(){
        sensorCount = 0;
    }

    int32_t sensorCount;
    VCamSensorInfo sensors[VCAM_Max_Supported_Sensor_Count];
};

/** VisionCam is a Mixin pattern which defines the abstract interface to the
 * class which wraps the physical Camera and/or the USB/File based alternatives
 * while implementing some of the common features of the class.
 */
class VisionCam
{
protected:
    FrameCallback_f m_callback;             /**< The default callback for preview frames */
    FramePackCallback_f m_pack_callback;    /**< The callback for frames received from all ports and sent in a single call to user*/
    FocusCallback_t m_focuscallback;        /**< The default focus callback */
    void * m_cookie;                        /**< The protected cookie data to pass to the m_callback */
    uint32_t m_frameNum;                    /**< The current number of frames delivered since instantiation */
public:
    VisionCam();

    /** Deconstructor */
    virtual ~VisionCam();

    /** Initializing method */
    virtual status_e init(void *cookie) = 0;

    /** Deinitializer */
    virtual status_e deinit() = 0;

    /** This method informs the VisionCam to use the supplied buffer */
    virtual status_e useBuffers( DVP_Image_t *prvBufArr, uint32_t numPrvBuf, VisionCamPort_e port = VCAM_PORT_PREVIEW) = 0;

    /**  This method informs the VisionCam to stop using the supplied buffers */
    virtual status_e releaseBuffers( VisionCamPort_e port = VCAM_PORT_PREVIEW ) = 0;

    /** This method informs the VisionCam to recall any outstanding buffers from remote cores or to drop pending copies */
    virtual status_e flushBuffers( VisionCamPort_e port = VCAM_PORT_PREVIEW ) = 0;

    /** This method informs the VisionCam to execute some additional functionality */
    virtual status_e sendCommand( VisionCamCmd_e cmdId, void *param = NULL, uint32_t size = 0, VisionCamPort_e port = VCAM_PORT_PREVIEW) = 0;

    /** This method informs the VisionCam to configure itself with the supplied parameters */
    virtual status_e setParameter( VisionCamParam_e paramId, void* param = NULL, uint32_t size = 0, VisionCamPort_e port = VCAM_PORT_PREVIEW) = 0;

    /** This method informs the VisionCam to configure itself with the supplied parameters */
    virtual status_e getParameter( VisionCamParam_e paramId, void* param = NULL, uint32_t size = 0, VisionCamPort_e port = VCAM_PORT_PREVIEW) = 0;

    /** This method returns a used buffer to the VisionCam to use again in the future */
    virtual status_e returnFrame( VisionCamFrame *cameraFrame ) = 0;

public:
    // Implemented Functions in this Mixin

    /** This returns the number of frames read from the camera since initialization */
    uint32_t GetFrameNumber();

    /** This method informs the VisionCam to enable the callback to the client which informs the client about the preview frame */
    status_e enablePreviewCbk(FrameCallback_f callback);

    /** This method informs the VisionCam to enable the callback to the client which informs the client about all frames */
    status_e enablePackedFramesCbk(FramePackCallback_f callback);

    /** This method informs the VisionCam to disable the preview frame m_callback */
    status_e disablePreviewCbk(FrameCallback_f callback);

    /** This method informs the VisionCam to disable the packed frames callback */
    status_e disablePackedFramesCbk(FramePackCallback_f callback);

    /** This method give the focus callback to camera */
    status_e setFocusCallBack(FocusCallback_t callback);
};

typedef VisionCam *(*VisionCamFactory_f)(VisionCam_e type);

extern "C" VisionCam *VisionCamFactory(VisionCam_e type);

uint32_t GetImageSize(uint32_t width, uint32_t height, fourcc_t fourcc);

#endif

