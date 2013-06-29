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

#ifndef __VCAM_TEST_DATA__
#define __VCAM_TEST_DATA__
#include "VisionCamTestTypes.h"

void setCropParams( void * input );
void setVariableFrameRate( void * input );
void setPreviewResolution( void * input );
void setFaceDetect( void * input );
void setImagePyramid( void * input );

#ifndef EXPORTED_3A
    void whiteBalGains( void * input );
    void whiteBalGains_get( void * input );
    void gammaTablesTest( void * input );
#endif // EXPORTED_3A

void setFormat( void * input );
void setStereoLayout( void * input );
void enableFramePack( void *input );

#ifdef EXPORTED_3A
    void startExported3A_Menu( void * input );
#endif // EXPORTED_3A

#define CAP_FRAME
#ifdef CAP_FRAME
void capFrame( void *input );
#endif // CAP_FRAME

void setDislpayableFrame(void *input);

static bool usePackaging = false;
static int32_t displayedFrame = VCAM_PORT_PREVIEW;
static int prevResIdx = VCAM_RES_QVGA;
static int colorSpacePeview = FOURCC_NV12;
static int colorSpaceVideo = FOURCC_NV12;
static int operatingMode = /*VCAM_GESTURE_DUAL_SENSOR_MODE;//*/VCAM_GESTURE_MODE;//VCAM_VIDEO_NORMAL;
static int currSensor = VCAM_SENSOR_PRIMARY;//VCAM_SENSOR_SECONDARY;
//static int layout = VCAM_STEREO_LAYOUT_TOPBOTTOM;
static VisionCamStereoInfo layout = {VCAM_STEREO_LAYOUT_TOPBOTTOM, 1};
static VisionCamPort_e currentPort = VCAM_PORT_ALL;
//static bool dumpExtraDataEnabled = false;
static bool faceDetectEnabled = false;
static VisionCamVarFramerateType varFrate;

static bool_e show_whole_buffer = true_e;

//static VisionCamPort_e framesDisplayed = VCAM_PORT_PREVIEW;
#ifdef CAP_FRAME
static bool recNextFrame = false;
#endif // CAP_FRAME
static VisionCamRectType cropParams = {
    0,
    0,
    VisionCamResolutions[prevResIdx].mWidth,
    VisionCamResolutions[prevResIdx].mHeight
};

/** Values for the menu input
  * entered by user
  */
entryKey KEY_PREVIEW_START              = "p";
entryKey KEY_PREVIEW_STOP               = "r";
entryKey KEY_PORT                       = "port";
// entryKey KEY_ETRA_DATA_Rx               = "e";
// entryKey KEY_ETRA_DATA_STOP_Rx          = "f";
entryKey KEY_EXTRA_DATA_START           = "e";
entryKey KEY_EXTRA_DATA_STOP            = "es";
entryKey KEY_BRIGHTNESS                 = "br";
entryKey KEY_CONTRAST                   = "co";
entryKey KEY_SHARPNESS                  = "sh";
entryKey KEY_SATURATION                 = "sa";
entryKey KEY_FLICKER                    = "fl";
entryKey KEY_CROP                       = "cr";
entryKey KEY_FRAME_RATE_FIXED           = "fr";
entryKey KEY_FRAME_RATE_VAR             = "fv";
entryKey KEY_CAPTURE_MODE               = "cm";
entryKey KEY_COLOR_FORMAT_MODE          = "cf";
entryKey KEY_SENSOR_SELECTION           = "se";
entryKey KEY_FOCUS                      = "fo";
entryKey KEY_FOCUS_MANUAL               = "mf";
entryKey KEY_MANUAL_EXPOSURE            = "ex";
entryKey KEY_EXPOSURE_ISO               = "so";
entryKey KEY_EXPOSURE_COMPENSATION      = "ec";
entryKey KEY_AE_LOCK                    = "le";
entryKey KEY_AWB_MODE                   = "wb";
entryKey KEY_AWB_LOCK                   = "lb";
entryKey KEY_COLOR_TEMPERATURE          = "ct";
entryKey KEY_RESOLUTION                 = "re";
entryKey KEY_FACE_DETECTION             = "fd";
entryKey KEY_GET_PARAMETER              = "gp";
entryKey KEY_WB_GAINS                   = "cg";
entryKey KEY_WB_GAINS_GET               = "gg";
entryKey KEY_WB_GAMMA_TBL               = "gt";
entryKey KEY_MIRROR                     = "mi";
entryKey KEY_ROTATION                   = "ro";
entryKey KEY_AWB_MIN_DELAY_TIME         = "wbd";
entryKey KEY_GESTURES_INFO              = "gfo";
entryKey KEY_AGC_MIN_DELAY_TIME         = "gd";
entryKey KEY_AGC_LOW_TH                 = "gmax";
entryKey KEY_AGC_HIGH_TH                = "gmin";
entryKey KEY_FREEZE_AGC_PARAMS          = "fgain";
entryKey KEY_FREEZE_AWB_PARAMS          = "fbal";
entryKey KEY_STEREO_LAYOUT              = "lout";
entryKey KEY_PACK_FRAMES                = "pack";
entryKey KEY_HOLD_EXPORTED_3A           = "3ahold";
entryKey KEY_SET_EXPORTED_3A            = "3aset";
entryKey KEY_APPLY_EXPORTED_3A          = "3apply";
entryKey KEY_SET_DISPLAYED_FRM          = "disp";
entryKey KEY_IMAGE_PYRAMID              = "ip";

#ifdef CAP_FRAME
entryKey KEY_CAP_FRAME                  = "cap";
#endif // CAP_FRAME

entryKey KEY_QUIT                       = "q";

#if TIME_PROFILE
  entryKey KEY_DUMP_TIMES                 = "du";
#endif

const int portOpts[] = {
  VCAM_PORT_PREVIEW,
  VCAM_PORT_VIDEO,
  VCAM_PORT_ALL
};

const char * portStrings[] = {
  "preview port",
  "video port",
  "all ports"
};

const subMenuEnrty portSubMenu = {
  portOpts,
  portStrings
};

const int extraDataOpts[] = {
    VCAM_EXTRA_DATA_NONE,
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
//     VCAM_SENSOR_CUSTOM_DATA
    VCAM_MANUAL_WHITE_BALANCE,
    VCAM_FACE_DETECTION_RAW,
    VCAM_HMS_GAMMA
};

const char * extraDataStrings[] = {
    "disable all",
//     "exif attributes",
    "ancillary data",
    "white balance",
    "unsaturated regions",
    "face detection",
//     "barcode detection",
//     "front object detection",
//     "motion estimation",
    "mtis type",
//     "distance estimation",
    "histogram",
    "focus region",
//     "extra data Pan and Scan",
//     "RAW format",
//     "sensor type",
//     "sensor custom data length",
//     "sensor custom data"
    "manual white balance data",
    "gamma (calculated hms gamma)"
};

const subMenuEnrty extraDataSubMenu = {
    extraDataOpts,
    extraDataStrings
};

const int flickerOpts[] = {
    FLICKER_OFF,
    FLICKER_AUTO,
    FLICKER_50Hz,
    FLICKER_60Hz,
    FLICKER_100Hz,
    FLICKER_120Hz
};

const char * flickerStrings[] = {
    "off - flicker",
    "auto - flicker",
    "50Hz flicker",
    "60Hz flicker",
    "100Hz flicker",
    "120Hz flicker"
};

const subMenuEnrty flickerSubMenu = {
    flickerOpts,
    flickerStrings
};

const int captureModeOpts[] = {
    VCAM_VIDEO_NORMAL,
    VCAM_VIDEO_HIGH_SPEED,
    VCAM_GESTURE_MODE,
    VCAM_STEREO_GEST_MODE,
    VCAM_STEREO_MODE,
    VCAM_GESTURE_DUAL_SENSOR_MODE
};

const char * captureModeStrings[] ={
    "Video normal ( default )",
    "Video high speed ( 120 fps )",
    "Gesture Operating Mode",
    "Stereo Gesture",
    "Stereo Normal",
    "Dual Sensor Gesture Mode"
};

const subMenuEnrty captureModeSubMenu = {
    captureModeOpts,
    captureModeStrings
};

const int colorFormatOpts[] = {
    FOURCC_UYVY,
    FOURCC_NV12,
    FOURCC_RGB565,
    FOURCC_BGR565
};

const char * colorFormatStrings[] = {
    "format UYVY",
    "format NV12",
    "format RGB 565",
    "format BGR 565"
};

const subMenuEnrty colorFormatSubMenu = {
    colorFormatOpts,
    colorFormatStrings
};


const int sensorSelOpts[] = {
    VCAM_SENSOR_PRIMARY,
    VCAM_SENSOR_SECONDARY,
    VCAM_SENSOR_STEREO,
};

const char * sensorSelStrings[] = {
    "Primary Sensor",
    "Secondary Sensor",
    "Stereo Sensors"
};

const subMenuEnrty sensorSelSubMenu = {
    sensorSelOpts,
    sensorSelStrings
};

const int focusOpts[] = {
    VCAM_FOCUS_CONTROL_ON,
    VCAM_FOCUS_CONTROL_OFF,
    VCAM_FOCUS_CONTROL_AUTO,
    VCAM_FOCUS_CONTROL_AUTO_LOCK,
    /// TI extensions follow
    VCAM_FOCUS_CONTRO_AUTO_MACRO,
    VCAM_FOCUS_CONTROL_AUTO_INFINITY,
    VCAM_FOCUS_FACE_PRIORITY_MODE,
    VCAM_FOCUS_REGION_PRIORITY_MODE,
    VCAM_FOCUS_CONTROL_HYPERFOCAL,
    VCAM_FOCUS_CONTROL_PORTRAIT,
    VCAM_FOCUS_CONTROL_EXTENDED,
    VCAM_FOCUS_CONTROL_CONTINOUS_NORMAL,
    VCAM_FOCUS_CONTROL_CONTINOUS_EXTENDED,
    VCAM_FOCUS_FACE_PRIORITY_CONTINOUS_MODE,
    VCAM_FOCUS_REGION_PRIORITY_CONTINOUS_MODE
};

const char *focusStrings[] = {
    "on",
    "off",
    "auto",
    "auto lock",
    "auto macro",
    "auto infinity",
    "face priority mode",
    "region priority mode",
    "hyperfocal",
    "portrait",
    "extended",
    "continous normal",
    "continous extended",
    "priority continous mode",
    "region priority continous mode"
};
const subMenuEnrty focusSubMenu = {
    focusOpts,
    focusStrings
};

const int lockOpts[] = {
    UNLOCKED,
    LOCKED
};

const char * lockStrings[] = {
    "Unlock",
    "Lock"
};

const subMenuEnrty lockSubMenu = {
    lockOpts,
    lockStrings
};

const int whiteBalOpts [] = {
    VCAM_WHITE_BAL_CONTROL_OFF,
    VCAM_WHITE_BAL_CONTROL_AUTO,
    VCAM_WHITE_BAL_CONTROL_SUNLIGHT,
    VCAM_WHITE_BAL_CONTROL_CLOUDY,
    VCAM_WHITE_BAL_CONTROL_SHADE,
    VCAM_WHITE_BAL_CONTROL_TUNGSTEN,
    VCAM_WHITE_BAL_CONTROL_FLUORESCENT,
    VCAM_WHITE_BAL_CONTROL_INCANDESCENT,
    VCAM_WHITE_BAL_CONTROL_FLASH,
    VCAM_WHITE_BAL_CONTROL_HORIZON,
    VCAM_WHITE_BAL_CONTROL_FACEPRIORITYMODE
};

const char * whiteBalStrings[] = {
    "off",
    "auto",
    "sunlight",
    "cloudy",
    "shade",
    "tungsten",
    "fluorescent",
    "incandescent",
    "flash",
    "horizon",
    "face priority mode"
};

const subMenuEnrty wbSubMenu = {
    whiteBalOpts,
    whiteBalStrings
};

const int mirrorOpts [] = {
    VCAM_MIRROR_NONE,
    VCAM_MIRROR_VERTICAL,
    VCAM_MIRROR_HORIZONTAL,
    VCAM_MIRROR_BOTH,
};

const char * mirrorStrings[] = {
    "No Mirroring",
    "Vertical",
    "Horizontal",
    "Horizontal and Vertical"
};

const subMenuEnrty mirrorSubMenu = {
    mirrorOpts,
    mirrorStrings
};

const int getParamIDs[] = {
    VCAM_PARAM_DO_AUTOFOCUS,
    VCAM_PARAM_DO_MANUALFOCUS,
    VCAM_PARAM_CONTRAST,
    VCAM_PARAM_SHARPNESS,
    VCAM_PARAM_BRIGHTNESS,
    VCAM_PARAM_SATURATION,
    VCAM_PARAM_HEIGHT,
    VCAM_PARAM_WIDTH,
    VCAM_PARAM_FPS_FIXED,
    VCAM_PARAM_FPS_VAR,
    VCAM_PARAM_COLOR_SPACE_FOURCC,
    VCAM_PARAM_FLICKER,
    VCAM_PARAM_CROP,
    VCAM_PARAM_CAP_MODE,
    VCAM_PARAM_SENSOR_SELECT,
    VCAM_PARAM_EXPOSURE_COMPENSATION,
    VCAM_PARAM_RESOLUTION,
    VCAM_PARAM_MANUAL_EXPOSURE,
    VCAM_PARAM_AWB_MODE,
    VCAM_PARAM_COLOR_TEMP,
    VCAM_PARAM_EXPOSURE_ISO,
    VCAM_PARAM_MIRROR,
};

const char * getParamstrings[] = {
    "focus mode",
    "manual focus distance",
    "contrast",
    "sharpness",
    "brightness",
    "saturation",
    "height",
    "width",
    "fps fixed",
    "fps var",
    "color space",
    "flicker",
    "crop",
    "cap mode",
    "sensor select",
    "exporure compensation",
    "resolution",
    "manual exposure",
    "awb mode",
    "color temp",
    "exposure iso",
    "get mirroring"
};

const subMenuEnrty getParamSubMenu = {
    getParamIDs,
    getParamstrings
};

const int stereloLayoutIDs[] = {
    VCAM_STEREO_LAYOUT_TOPBOTTOM,
    VCAM_STEREO_LAYOUT_LEFTRIGHT
};

const char * stereoLayoutStrings[] = {
    "Top - Bottom",
    "Side by Side"
};

const subMenuEnrty stereoLayoutSubMenu = {
    stereloLayoutIDs,
    stereoLayoutStrings
};

const menuEnrty startPreview = {
    KEY_PREVIEW_START,
    commandEntry,
    VCAM_CMD_PREVIEW_START,
    "Start preview",
    { 0, 0, NULL },
    NULL,
    NULL
};

const menuEnrty stopPreview = {
    KEY_PREVIEW_STOP,
    commandEntry,
    VCAM_CMD_PREVIEW_STOP,
    "Stop preview",
    { 0, 0, NULL  },
    NULL,
    NULL
};

const menuEnrty portUsage = {
    KEY_PORT,
    neutralEntry,
    0xFFFFFFFF,
    "Port",
    { 0, ARR_SIZE(portOpts) - 1, NULL  },
    &currentPort,
    (void*)&portSubMenu
};

const menuEnrty extraDataStart = {
    KEY_EXTRA_DATA_START,
    commandEntry,
    VCAM_EXTRA_DATA_START,
    "Extra data start (asks for data type).",
    { 0, ARR_SIZE(extraDataOpts) - 1, NULL },
    NULL,
    (void*)&extraDataSubMenu
};

const menuEnrty extraDataStop = {
    KEY_EXTRA_DATA_STOP,
    commandEntry,
    VCAM_EXTRA_DATA_STOP,
    "Extra data stop (asks for data type).",
    { 0, ARR_SIZE(extraDataOpts) - 1, NULL },
    NULL,
    (void*)&extraDataSubMenu
};

const menuEnrty brightness = {
    KEY_BRIGHTNESS,
    paramEntry_set,
    VCAM_PARAM_BRIGHTNESS,
    "Brightness.",
    { 0 , 200, NULL },
    NULL,
    NULL
};

const menuEnrty contrast = {
    KEY_CONTRAST,
    paramEntry_set,
    VCAM_PARAM_CONTRAST,
    "Contrast.",
    { -100 , 100, NULL },
    NULL,
    NULL
};

const menuEnrty sharpness = {
    KEY_SHARPNESS,
    paramEntry_set,
    VCAM_PARAM_SHARPNESS,
    "Sharpness.",
    { -100 , 100, NULL },
    NULL,
    NULL
};

const menuEnrty saturation = {
    KEY_SATURATION,
    paramEntry_set,
    VCAM_PARAM_SATURATION,
    "Saturation.",
    { -100 , 100, NULL },
    NULL,
    NULL
};

const menuEnrty flicker = {
    KEY_FLICKER,
    paramEntry_set,
    VCAM_PARAM_FLICKER,
    "Flicker",
    { 0, ARR_SIZE(flickerOpts) - 1 , NULL },
    NULL,
    (void*)&flickerSubMenu
};

const menuEnrty crop = {
    KEY_CROP,
    functionalEntry,
    VCAM_PARAM_CROP,
    "Crop",
    { 0 , 0, NULL },
    &cropParams,
    (void*)( (subMenuFn)setCropParams )
};

const menuEnrty frameRate_variable = {
    KEY_FRAME_RATE_VAR,
    functionalEntry,
    VCAM_PARAM_FPS_VAR,
    "Frame rate - variable",
    { 0 , 0, NULL },
    NULL,
    (void*)( (subMenuFn)setVariableFrameRate )
};

const menuEnrty frameRate_fixed = {
    KEY_FRAME_RATE_FIXED,
    paramEntry_set,
    VCAM_PARAM_FPS_FIXED,
    "Frame rate ( 0 for auto )",
    { 0 , 30, NULL },
    NULL,
    NULL
};

const menuEnrty captureMode = {
    KEY_CAPTURE_MODE,
    paramEntry_set,
    VCAM_PARAM_CAP_MODE,
    "Capture mode",
    { 0 , ARR_SIZE(captureModeOpts) - 1, NULL },
    &operatingMode,
    (void*)&captureModeSubMenu
};

const menuEnrty format  = {
    KEY_COLOR_FORMAT_MODE,
    functionalEntry, //paramEntry_set,
    VCAM_PARAM_COLOR_SPACE_FOURCC,
    "Color format",
    { 0 , (ARR_SIZE(colorFormatOpts) - 1), NULL },
    &colorSpacePeview,
    (void*)((subMenuFn)setFormat)
};

const menuEnrty sensor = {
    KEY_SENSOR_SELECTION,
    paramEntry_set,
    VCAM_PARAM_SENSOR_SELECT,
    "Sensor select",
    { 0 , ARR_SIZE(sensorSelOpts ) - 1, NULL },
    &currSensor,
    (void*)&sensorSelSubMenu
};

const menuEnrty expCompensation = {
    KEY_EXPOSURE_COMPENSATION,
    paramEntry_set,
    VCAM_PARAM_EXPOSURE_COMPENSATION,
    "Exposure Compensation ( *10 )",
    { -20 , 20, NULL },
    NULL,
    NULL
};

const menuEnrty focus = {
    KEY_FOCUS,
    paramEntry_set,
    VCAM_PARAM_DO_AUTOFOCUS,
    "Focus type (not all listed are available)",
    { VCAM_FOCUS_CONTROL_ON , ( VCAM_FOCUS_CONTROL_MAX - 1 ), NULL },
    NULL,
    (void *)&focusSubMenu
};

const menuEnrty focusManual = {
    KEY_FOCUS_MANUAL,
    paramEntry_set,
    VCAM_PARAM_DO_MANUALFOCUS,
    "Focus - manual ( in millimeters )",
    { 0 , 100, NULL },
    NULL,
    NULL
};

const menuEnrty manualExp = {
    KEY_MANUAL_EXPOSURE,
    paramEntry_set,
    VCAM_PARAM_MANUAL_EXPOSURE,
    "Exposure - manual ( changes frame rate; 0 - auto)",
    { 0, 100, NULL },
    NULL,
    NULL
};

const int isoGainEx[] = { 0 };

const exeptionValues expISO_exeptios = {
    isoGainEx,
    ARR_SIZE(isoGainEx)
};

const menuEnrty expISO_Gain = {
    KEY_EXPOSURE_ISO,
    paramEntry_set,
    VCAM_PARAM_EXPOSURE_ISO,
    "ISO / Gain ( gain = ISO / 100 )",
    { 100, 1600, &expISO_exeptios },
    NULL,
    NULL
};

const menuEnrty expLock = {
    KEY_AE_LOCK,
    commandEntry,
    VCAM_CMD_LOCK_AE,
    "Lock Exposure",
    { UNLOCKED , LOCKED, NULL },
    NULL,
    ( void *)&lockSubMenu
};

const menuEnrty awbLock = {
    KEY_AWB_LOCK,
    commandEntry,
    VCAM_CMD_LOCK_AWB,
    "Lock White Balance",
    { UNLOCKED , LOCKED, NULL },
    NULL,
    ( void *)&lockSubMenu
};

const menuEnrty awbMode = {
    KEY_AWB_MODE,
    paramEntry_set,
    VCAM_PARAM_AWB_MODE,
    "White Balance Mode",
    { VCAM_WHITE_BAL_CONTROL_OFF , (ARR_SIZE( whiteBalOpts) - 1), NULL },
    NULL,
    ( void *)&wbSubMenu
};

const int colorTempEx[] = { 0 };

const exeptionValues colorTempExeption = {
    colorTempEx,
    ARR_SIZE(colorTempEx)
};

const menuEnrty colorTemp = {
    KEY_COLOR_TEMPERATURE,
    paramEntry_set,
    VCAM_PARAM_COLOR_TEMP,
    "Corlor Temp ( Kelvins )",
    { 2020 , 7100, &colorTempExeption },
    NULL,
    NULL
};

const menuEnrty prvRes = {
    KEY_RESOLUTION,
    functionalEntry,
    VCAM_PARAM_RESOLUTION,
    "Resolution change",
    { 0 , ( VCAM_RESOL_MAX - 1 ), NULL },
    &prevResIdx,
    (void*)( (subMenuFn)setPreviewResolution )
};

#if TIME_PROFILE
const menuEnrty dumpTimes = {
    KEY_DUMP_TIMES,
    commandEntry,
    VCAM_DUMP_TIMES,
    "Get measured times.",
    { 0 , 0, NULL },
    NULL,
    NULL
};
#endif

const menuEnrty faceDet = {
    KEY_FACE_DETECTION,
    functionalEntry,
    VCAM_CMD_FACE_DETECTION,
    "Enable/Disable face detection",
    { 0 , 0, NULL },
    &faceDetectEnabled,
    (void*)( (subMenuFn)setFaceDetect )
};

const menuEnrty getParam = {
    KEY_GET_PARAMETER,
    paramEntry_get,
    (unsigned)(-1),
    "Get parameter value",
    { 0 , ARR_SIZE(getParamIDs) - 1, NULL },
    NULL,
    ( void *)&getParamSubMenu
};

const int wbGainEx[] = { 0 };

const exeptionValues wbGainExeptions = {
    wbGainEx,
    ARR_SIZE(wbGainEx)
};
#ifndef EXPORTED_3A
const menuEnrty wbGains = {
    KEY_WB_GAINS,
    functionalEntry,
    VCAM_PARAM_WB_COLOR_GAINS,
    "Set color gains (white balance color gains)",
    { 512 , 2048, &wbGainExeptions },
    NULL,
    (void*)( (subMenuFn)whiteBalGains)
};

const menuEnrty wbGains_get = {
    KEY_WB_GAINS_GET,
    functionalEntry,
    VCAM_PARAM_WB_COLOR_GAINS,
    "Get color gains (white balance color gains)",
    { 0 , 0, NULL },
    NULL,
    (void*)( (subMenuFn)whiteBalGains_get)
};

const menuEnrty gammaTbl_test = {
    KEY_WB_GAMMA_TBL,
    functionalEntry,
    VCAM_PARAM_GAMMA_TBLS,
    "Gamma test ( test passed if review goes negative )",
    { 0 , 0, NULL },
    NULL,
    (void*)( (subMenuFn)gammaTablesTest)
};
#endif // EXPORTED_3A

const menuEnrty mirroring = {
    KEY_MIRROR,
    paramEntry_set,
    VCAM_PARAM_MIRROR,
    "Mirror",
    { 0 , ARR_SIZE( mirrorOpts ) - 1 , NULL },
    NULL,
    (void*)&mirrorSubMenu
};

const menuEnrty rotation = {
    KEY_ROTATION,
    paramEntry_set,
    VCAM_PARAM_ROTATION,
    "Rotation (0 | 90 | 180 | 270)",
    { 0 , 270 , NULL }, /// @todo create a table with possible values !!!
    NULL,
    NULL
};

const menuEnrty awbMinDelayTime = {
    KEY_AWB_MIN_DELAY_TIME,
    paramEntry_set,
    VCAM_PARAM_AWB_MIN_DELAY_TIME,
    "AWB minimun delay time [ms]",
    { 0, 10000, NULL },
    NULL,
    NULL
};

const menuEnrty agcMinDelayTime = {
    KEY_AGC_MIN_DELAY_TIME,
    paramEntry_set,
    VCAM_PARAM_AGC_MIN_DELAY_TIME,
    "AGC (AE) minimun delay time [ms]",
    { 0, 10000, NULL },
    NULL,
    NULL
};

const menuEnrty agcLowTh = {
    KEY_AGC_LOW_TH,
    paramEntry_set,
    VCAM_PARAM_AGC_LOW_TH,
    "AGC (AE) low threshold",
    { 0, 10000, NULL },
    NULL,
    NULL
};

const menuEnrty agcHighTh = {
    KEY_AGC_HIGH_TH,
    paramEntry_set,
    VCAM_PARAM_AGC_HIGH_TH,
    "AGC (AE) high threshold",
    { 0, 10000, NULL },
    NULL,
    NULL
};

const menuEnrty gestureInfo = {
    KEY_GESTURES_INFO,
    functionalEntry,
    VCAM_PARAM_GESTURES_INFO,
    "Gestures info",
    { 0, 0, NULL },
    NULL,
    NULL
};

const menuEnrty freezeAWBparams = {
    KEY_FREEZE_AWB_PARAMS,
    commandEntry,
    VCAM_CMD_FREEZE_AWB_PARAMS,
    "Freeze AWB params",
    { 0, 10000, NULL },
    NULL,
    NULL
};

const menuEnrty freezeAGCparams = {
    KEY_FREEZE_AGC_PARAMS,
    commandEntry,
    VCAM_CMD_FREEZE_AGC_PARAMS,
    "Freeze AGC (AE) params",
    { 0, 10000, NULL },
    NULL,
    NULL
};

const menuEnrty stereoLayout = {
    KEY_STEREO_LAYOUT,
    functionalEntry,
    VCAM_PARAM_STEREO_INFO,
    "Layout (stereo layout)",
    { stereloLayoutIDs[0], (ARR_SIZE(stereloLayoutIDs) - 1), NULL },
    (void*)&layout,
    (void*)( (subMenuFn)setStereoLayout )
};

const menuEnrty packFrames = {
    KEY_PACK_FRAMES,
    functionalEntry,
    VCAM_PARAM_MAX,
    "Pack frames",
    { 0, 0, NULL },
    NULL,
    (void*)( (subMenuFn)enableFramePack )
};

#ifdef CAP_FRAME
const menuEnrty recordFrame = {
    KEY_CAP_FRAME,
    functionalEntry,
    VCAM_PARAM_MAX,
    "Capture Frame (only next one)",
    { 0, 0, NULL },
    (void*)recNextFrame,
    (void*)( (subMenuFn)capFrame )
};
#endif

const menuEnrty quit = {
    KEY_QUIT,
    neutralEntry,
    VCAM_CMD_QUIT,
    "Quit.",
    { 0 , 0, NULL },
    NULL,
    NULL
};

#ifdef EXPORTED_3A
const menuEnrty hold3AmanualSettings = {
    KEY_HOLD_EXPORTED_3A,
    paramEntry_set,
    VCAM_PARAM_EXPORTED_3A_HOLD,
    "Start (hold) manual 3A configuration.",
    { 0 , 0, NULL },
    NULL,
    NULL
};

const menuEnrty set3AmanualSettings = {
    KEY_SET_EXPORTED_3A,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Set 3A configuration.",
    { 0 , 0, NULL },
    NULL,
    (void*)( (subMenuFn)startExported3A_Menu )
};

const menuEnrty apply3AmanualSettings = {
    KEY_APPLY_EXPORTED_3A,
    paramEntry_set,
    VCAM_PARAM_EXPORTED_3A_APPLY,
    "Apply manual 3A configuration.",
    { 0 , 0, NULL },
    NULL,
    NULL
};
#endif // EXPORTED_3A

const int dispFrameOpts[] = {   VCAM_PORT_PREVIEW,  VCAM_PORT_VIDEO     };
const char *dispFrameStrings[] = { "Preview port frames" , "Video port frames" };
const subMenuEnrty dispFrameSubmenu = { dispFrameOpts, dispFrameStrings };

const menuEnrty setDispFrame = {
    KEY_SET_DISPLAYED_FRM,
    functionalEntry,
    VCAM_PARAM_MIN,
    "Displayed frame.",
    { 0 , ARR_SIZE(dispFrameOpts) - 1, NULL },
    &displayedFrame,
    (void*)( (subMenuFn)setDislpayableFrame )
};

const menuEnrty setImgPyramid = {
    KEY_IMAGE_PYRAMID,
    functionalEntry,
    VCAM_PARAM_IMAGE_PYRAMID,
    "Set Image Pyramid properties. Image pyramid level > 1 will enable this functionality.",
    { 0 , 0, NULL },
    NULL,
    (void*)( (subMenuFn)setImagePyramid )
};

const menuEnrty *menu[] =
{
    &startPreview,
    &stopPreview,
    &portUsage,
//     &extraData,
//     &startExtraDataTransfer,
//     &stopExtraDataTransfer,
    &extraDataStart,
    &extraDataStop,
    &brightness,
    &contrast,
    &sharpness,
    &saturation,
    &flicker,
    &crop,
    &frameRate_fixed,
    &frameRate_variable,
    &captureMode,
    &format,
    &sensor,
    &focus,
    &focusManual,
    &manualExp,
    &expISO_Gain,
    &expCompensation,
    &expLock,
    &awbMode,
    &colorTemp,
    &awbLock,
    &prvRes,

#if TIME_PROFILE
    &dumpTimes,
#endif

    &faceDet,
    &getParam,

#ifndef EXPORTED_3A
    &wbGains,
    &wbGains_get,
    &gammaTbl_test,
#endif // EXPORTED_3A

    &mirroring,
    &rotation,
    &awbMinDelayTime,
    &agcMinDelayTime,
    &agcLowTh,
    &agcHighTh,
    &gestureInfo,
    &freezeAGCparams,
    &freezeAWBparams,
    &stereoLayout,

#ifdef EXPORTED_3A
    &hold3AmanualSettings,
    &set3AmanualSettings,
    &apply3AmanualSettings,
#endif // EXPORTED_3A

#ifdef CAP_FRAME
    &recordFrame,
#endif // CAP_FRAME

    &setDispFrame,
    &setImgPyramid,

    &quit,
    NULL
};

const menuEnrty ok = {
    KEY_QUIT,
    neutralEntry,
    VCAM_CMD_QUIT,
    "OK ( use default if nothing is set )",
    { 0 , 0, NULL },
    NULL,
    NULL
};

const menuEnrty *menu_Primary[] =
{
    &packFrames,
    &portUsage,
    &format,
    &sensor,
    &stereoLayout,
    &captureMode,
    &prvRes,
    &ok,

    NULL
};

#endif /// __VCAM_TEST_DATA__
