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

#ifndef _VISION_CAM_3A_EXPORT_TYPES_H_
#define _VISION_CAM_3A_EXPORT_TYPES_H_

#include <sosal/sosal.h>

/*! \brief 16 Extended to allow precise H3A for preview */
#define VCAM_MAX_WINHC                       (36)
/*! \brief 32 Extended to allow precise H3A for preview */
#define VCAM_MAX_WINVC                       (128)
/*! \brief One paxel is 32 Bytes + on every 8 paxel 8*2 Bytes for number of unsaturated pixels in previouse 8 paxels */
#define VCAM_AWWB_H3A_PAXEL_SIZE_BYTES       (32 + 2)
/*! \brief Max buffer size for AEWB H3A */
#define VCAM_AEWB_H3A_MAX_BUFF_SIZE          (1) //(VCAM_MAX_WINHC * VCAM_MAX_WINVC * VCAM_AWWB_H3A_PAXEL_SIZE_BYTES)
/*! \brief AF SCM Range Constants */
#define VCAM_AF_PAXEL_VERTICAL_COUNT_MAX     (127)
/*! \brief AF SCM Range Constants */
#define VCAM_AF_PAXEL_HORIZONTAL_COUNT_MAX   (35)
/*! \brief Max buffer size for AF H3A */
#define VCAM_AF_H3A_MAX_BUFF_SIZE            (1) //(VCAM_AF_PAXEL_VERTICAL_COUNT_MAX * VCAM_AF_PAXEL_HORIZONTAL_COUNT_MAX * 48)
/*! \brief Max sensor with */
#define VCAM_ISP_IN_WIDTH                    (4032)
/*! \brief Max sensor height */
#define VCAM_ISP_IN_HEIGHT                   (3024)
/*! \brief Max buffer size for IPIPE BOXCAR */
#define VCAM_IPIPE_BOXCAR_MAX_BUFF_SIZE      (1) //(((VCAM_ISP_IN_WIDTH / 8) * (VCAM_ISP_IN_HEIGHT / 8)) * 8)


/*! \brief Noise filter number of THR coefficients */
#define VCAM_ISS_NF_THR_COUNT        (8)
/*! \brief Noise filter number of STR coefficients */
#define VCAM_ISS_NF_STR_COUNT        (8)
/*! \brief Noise filter number of SPR coefficients */
#define VCAM_ISS_NF_SPR_COUNT        (8)
/*! \brief Numbers of values in Gamma table */
#define VCAM_ISS_PREV_GAMMA_TABLE    (1024)
/*! \brief Number of offset values on table */
#define VCAM_ISS_PREV_RGB2RGB_OFFSET (3)
/*! \brief Numbers of values in rows and colomns */
#define VCAM_ISS_PREV_RGB2RGB_MATRIX (3)
/*! \brief GBCE enhancement table size */
#define VCAM_ISS_GBCE_TABLE_SIZE     (1024)
/*! \brief Edge enhancement table size */
#define VCAM_ISS_EE_TABLE_SIZE       (1024)
/*! \brief Edge enhancement number of coefficients */
#define VCAM_ISS_COEFF               (9)
/*! \brief Histogram dims count */
#define VCAM_ISS_HIST_DIMS_COUNT     (4)
/*! \brief Histogram gain table size */
#define VCAM_ISS_HIST_GAIN_TBL       (4)
/*! \brief 3d lut table size */
#define VCAM_ISS_3D_LUT_SIZE         (1) //(729)
/*! \brief Number of offset values on table */
#define VCAM_ISS_PREV_RGB2YUV_OFFSET (3)
/*! \brief Numbers of values in rows and colomns */
#define VCAM_ISS_PREV_RGB2YUV_MATRIX (3)

/*! \brief Defines which camera 3A configuration parameters,
 * could be modified by VisionCam's client.
 */
enum SettingType_e{
    VCAM_3A_Start = 0,

    VCAM_3A_Lsc2D = VCAM_3A_Start,      /*!< \see VisionCam_3A_2D_LscConfig_t */
    VCAM_3A_Clamp,                      /*!< \see VisionCam_3A_Clamp_t */
    VCAM_3A_GainOffset,                 /*!< \see VisionCam_3A_GainOffset_t */
    VCAM_3A_Vldc,                       /*!< \see VisionCam_3A_VDLC_t */
    VCAM_3A_Nf1,                        /*!< \see VisionCam_3A_NoiseFilterConfig_t */
    VCAM_3A_Nf2,                        /*!< \see VisionCam_3A_NoiseFilterConfig_t */
    VCAM_3A_GIC,                        /*!< \see VisionCam_3A_GIC_Config_t */
    VCAM_3A_WB,                         /*!< \see VisionCam_3A_WB_Config_t */
    VCAM_3A_CFA,                        /*!< \see VisionCam_3A_CFA_Config_t */
    VCAM_3A_Gamma,                      /*!< \see VisionCam_3A_GammaTableConfig_t */
    VCAM_3A_Rgb2Rgb1,                   /*!< \see VisionCam_3A_RGB2RGB_t */
    VCAM_3A_Rgb2Rgb2,                   /*!< \see VisionCam_3A_RGB2RGB_t */
    VCAM_3A_Rgb2Yuv,                    /*!< \see VisionCam_3A_RGB2YUV_t */
    VCAM_3A_GBCE,                       /*!< \see VisionCam_3A_GBCE_Config_t */
    VCAM_3A_Yuv2Yuv,                    /*!< \see VisionCam_3A_Yuv444_toYuv422_Config_t */
    VCAM_3A_Ee,                         /*!< \see VisionCam_3A_EdgeEnhancement_config_t */
    VCAM_3A_Car,                        /*!< \see VisionCam_3A_CAR_config_t */
    VCAM_3A_Lsc,                        /*!< \see VisionCam_3A_LscConfig_t */
    VCAM_3A_Histogram,                  /*!< \see VisionCam_3A_HistogramConfig_t */
    VCAM_3A_Boxcar,                     /*!< \see VisionCam_3A_BoxCarConfig_t */
    VCAM_3A_Bsc,                        /*!< \see VisionCam_3A_BscConfig_t */
    VCAM_3A_3Dcc,                       /*!< \see VisionCam_3A_3DCC_config_t */
    VCAM_3A_DpcOtf,                     /*!< \see VisionCam_3A_DpcOtfConfig_t */
    VCAM_3A_DpcLut,                     /*!< \see VisionCam_3A_DPC_Lut_t */
    VCAM_3A_Cgs,                        /*!< \see VisionCam_3A_CGS_Config_t */
    VCAM_3A_Dfs,                        /*!< \see VisionCam_3A_DfsConfig_t */
    VCAM_3A_Dpc1,                       /*!< \see VisionCam_3A_DpcConfig_t */
    VCAM_3A_Dpc2,                       /*!< \see VisionCam_3A_DpcConfig_t */
    VCAM_3A_Dpcm,                       /*!< \see VisionCam_3A_DPCM_Config_t */
    VCAM_3A_HLpf,                       /*!< \see VisionCam_3A_ResizerLowPassFilter_t */
    VCAM_3A_VLpf,                       /*!< \see VisionCam_3A_ResizerLowPassFilter_t */
    VCAM_3A_H3aAewbParams,              /*!< \see VisionCam_3A_AEWB_config_t */
    VCAM_3A_H3aAfParams,                /*!< \see VisionCam_3A_H3A_AutoFocusParam_t */
    VCAM_3A_H3aCommonParams,            /*!< \see VisionCam_3A_H3A_CommonConfig_t */
    VCAM_3A_CamControlExpGain,          /*!< \see VisionCam_3A_ExpGainControl_t */

    e3A_ManualSetting_ALL,
    e3A_ManualSetting_MAX = e3A_ManualSetting_ALL
};

/*! \todo comment this */
typedef enum _vcam_3a_lsc_gain_mode_e {
    VCAM_ISIF_LSC_8_PIXEL       = 3,
    VCAM_ISIF_LSC_16_PIXEL      = 4,
    VCAM_ISIF_LSC_32_PIXEL      = 5,
    VCAM_ISIF_LSC_64_PIXEL      = 6,
    VCAM_ISIF_LSC_128_PIXEL     = 7,
    VCAM_ISIF_LSC_128_PIXEL_MAX = 0x7FFFFFFF
} VCAM_3ASKIP_ISIF_LSC_GAIN_MODE_MNTYPE;

/*! \todo comment this */
typedef enum _vcam_3a_lsc_format_e {
    VCAM_LSC_GAIN_0Q8        = 0,
    VCAM_LSC_GAIN_0Q8_PLUS_1 = 1,
    VCAM_LSC_GAIN_1Q7        = 2,
    VCAM_LSC_GAIN_1Q7_PLUS_1 = 3,
    VCAM_LSC_GAIN_2Q6        = 4,
    VCAM_LSC_GAIN_2Q6_PLUS_1 = 5,
    VCAM_LSC_GAIN_3Q5        = 6,
    VCAM_LSC_GAIN_3Q5_PLUS_1 = 7,
    VCAM_LSC_GAIN_MAX        = 0x7FFFFFFF
} VCAM_3ASKIP_ISIF_LSC_GAIN_FORMATTYPE;

/*! \todo comment this */
typedef enum _vcam_3a_lsc_offset_e {
    VCAM_ISIF_LSC_OFFSET_NO_SHIFT         = 0,
    VCAM_ISIF_LSC_OFFSET_1_LEFT_SHIFT     = 1,
    VCAM_ISIF_LSC_OFFSET_2_LEFT_SHIFT     = 2,
    VCAM_ISIF_LSC_OFFSET_3_LEFT_SHIFT     = 3,
    VCAM_ISIF_LSC_OFFSET_4_LEFT_SHIFT     = 4,
    VCAM_ISIF_LSC_OFFSET_5_LEFT_SHIFT     = 5,
    VCAM_ISIF_LSC_OFFSET_5_LEFT_SHIFT_MAX = 0x7FFFFFFF
} VCAM_3ASKIP_ISIF_LSC_OFFSET_SHIFTTYPE;

/*! \todo comment this */
typedef enum _vcam_3a_lsc_offset_enable_e {
    VCAM_ISIF_LSC_OFFSET_OFF = 0,
    VCAM_ISIF_LSC_OFFSET_ON  = 1,
    VCAM_ISIF_LSC_OFFSET_MAX = 0x7FFFFFFF
} VCAM_3ASKIP_ISIF_LSC_OFFSET_ENABLETYPE;

/*! \todo comment this */
typedef struct {
    uint8_t nEnable;            //!< DLSCCFG ENABLE-- enable 2d LSC module
    uint16_t nLscHSize;         //!< HVAL LSCHVAL--LSC HSIZE
    uint16_t nLscVSize;         //!< HVAL LSCHVAL--LSC VSIZE
    uint16_t nHDirDataOffset;   //!< HOFST LSCHOFST DATAHOFST 0-16383-- H-direction data offset
    uint16_t nVDirDataOffset;   //!< VOFST LSCVOFST DATAHOFST 0-16383-- V-direction data offset
    uint8_t nHPosInPaxel;       //!< X DLSCINI   6:0-- H-position of the paxel
    uint8_t nVPosInPaxel;       //!< Y DLSCINI   6:0-- V-position of the paxel
    VCAM_3ASKIP_ISIF_LSC_GAIN_MODE_MNTYPE ePaxHeight;   //!< GAIN_MODE_M DLSCCFG
    VCAM_3ASKIP_ISIF_LSC_GAIN_MODE_MNTYPE ePaxLength;   //!< GAIN_MODE_N DLSCCFG
    VCAM_3ASKIP_ISIF_LSC_GAIN_FORMATTYPE eGainFormat;   //!< GAIN_FORMAT DLSCCFG
    uint8_t nOffsetScalingFactor;                       //!< offset scaling factor
    VCAM_3ASKIP_ISIF_LSC_OFFSET_SHIFTTYPE eOffsetShiftVal;  //!< OFSTSFT DLSCOFST--offset shift value
    VCAM_3ASKIP_ISIF_LSC_OFFSET_ENABLETYPE eOffsetEnable;   //!< OFSTSFT DLSCOFST--offset enable value
    uint32_t pGainTableAddress[256];                        //!< gain table address--32 bit aligned
    uint16_t nGainTableLength;                              //!< gain table length
    uint32_t pOffsetTableAddress[256];                      //!< offset table address
    uint16_t nOffsetTableLength;                            //!< offset table length
} VCAM_3ASKIP_ISIF_2DLSC_CFGTYPE;

/*! \todo comment this */
typedef struct _manual_3A_setting_base_t {
    SettingType_e               eParamType;
    void                        *pData;
} VisionCam_3Asettings_Base_t;

/*! \todo comment this */
typedef struct _vcam_3a_2dlsc_cfg_t
{
    int32_t eSensor;
    VCAM_3ASKIP_ISIF_2DLSC_CFGTYPE m2D_LscConfig;
} VisionCam_3A_2D_LscConfig_t;

/*! \todo comment this */
typedef enum {
    VCAM_ISIF_HORIZONTAL_CLAMP_DISABLED             = 0,
    VCAM_ISIF_HORIZONTAL_CLAMP_ENABLED              = 1,
    VCAM_ISIF_PREVIOUS_HORIZONTAL_CLAMP_ENABLED     = 2,
    VCAM_ISIF_CLAMP_MAX                             = 0x7FFFFFFF
} VCAM_3ASKIP_ISIF_HORIZONTAL_CLAMP_MODETYPE;

/*! \todo comment this */
typedef struct _vcam_3a_horizontal_clamp_t
{
    int32_t eSensor;
    VCAM_3ASKIP_ISIF_HORIZONTAL_CLAMP_MODETYPE eHorizontalClamp;
} VisionCam_3A_HorizontalClamp_t;

/*! \todo comment this */
typedef enum {
    VCAM_ISIF_ONE_COLOR_CLAMP    = 0,
    VCAM_ISIF_FOUR_COLOR_CLAMP   = 1,
    VCAM_ISIF_COLOR_CLAMP_MAX    = 0x7FFFFFFF
} VCAM_3ASKIP_ISIF_BLACK_CLAMP_MODETYPE;

/*! \todo comment this */
typedef struct _vcam_3a_black_clamp_mode_t
{
    int32_t eSensor;
    VCAM_3ASKIP_ISIF_BLACK_CLAMP_MODETYPE eBlackClamp;
} VisionCam_3A_BlackClamp_t;

/*! \todo comment this */
typedef enum {
    VCAM_ISIF_HBLACK_2PIXEL_TALL    = 0,
    VCAM_ISIF_HBLACK_4PIXEL_TALL    = 1,
    VCAM_ISIF_HBLACK_8PIXEL_TALL    = 2,
    VCAM_ISIF_HBLACK_16PIXEL_TALL   = 3,
    VCAM_ISIF_HBLACK_PIXEL_TALL_MAX = 0x7FFFFFFF
} VCAM_3ASKIP_ISIF_HBLACK_PIXEL_HEIGHTTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_black_pixel_height_t
{
    int32_t eSensor;
    VCAM_3ASKIP_ISIF_HBLACK_PIXEL_HEIGHTTYPE eBlackPixHeight;
} VisionCam_3A_BlackPixelHeight_t;

/*! \todo comment this */
typedef enum {
    VCAM_ISIF_HBLACK_32PIXEL_WIDE   = 0,
    VCAM_ISIF_HBLACK_64PIXEL_WIDE   = 1,
    VCAM_ISIF_HBLACK_128PIXEL_WIDE  = 2,
    VCAM_ISIF_HBLACK_256PIXEL_WIDE  = 3,
    VCAM_ISIF_HBLACK_PIXEL_WIDE_MAX = 0x7FFFFFFF
} VCAM_3ASKIP_ISIF_HBLACK_PIXEL_WIDTHTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_horiz_black_pixel_width_t
{
    int32_t eSensor;
    VCAM_3ASKIP_ISIF_HBLACK_PIXEL_WIDTHTYPE eBlackPixWidth;
} VisionCam_3A_HorizBlackPixelWidth_t;

/*! \todo comment this */
typedef enum {
    VCAM_ISIF_VBLACK_PIXEL_NOT_LIMITED = 0,
    VCAM_ISIF_VBLACK_PIXEL_LIMITED     = 1,
    VCAM_ISIF_VBLACK_PIXEL_LIMITED_MAX = 0x7FFFFFFF
} VCAM_3ASKIP_ISIF_HBLACK_PIXEL_VALUE_LIMTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_black_pixel_value_t
{
    int32_t eSensor;
    VCAM_3ASKIP_ISIF_HBLACK_PIXEL_WIDTHTYPE eBlackPixValue;
} VisionCam_3A_BlackValue_t;

/*! \todo comment this */
typedef enum {
    VCAM_ISIF_VBLACK_BASE_WINDOW_LEFT  = 0,
    VCAM_ISIF_VBLACK_BASE_WINDOW_RIGHT = 1,
    VCAM_ISIF_VBLACK_BASE_WINDOW_MAX   = 0x7FFFFFFF
} VCAM_3ASKIP_ISIF_HBLACK_BASE_WINDOWTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_black_base_window_t
{
    int32_t eSensor;
    VCAM_3ASKIP_ISIF_HBLACK_BASE_WINDOWTYPE eBlackBaseWin;
} VisionCam_3A_BlackBaseWindow_t;

/*! \todo comment this */
typedef struct {
    uint16_t nVPos;                                     //!< Window Start position (V).Range: 0 - 8191
    VCAM_3ASKIP_ISIF_HBLACK_PIXEL_HEIGHTTYPE eVSize;    //!< Vertical dimension of a Window (2^N).
    uint16_t nHPos;                                     //!< Window Start position (H).Range: 0 - 8191
    VCAM_3ASKIP_ISIF_HBLACK_PIXEL_WIDTHTYPE eHSize;     //!< Horizontal dimension of a Window (2^M).
    VCAM_3ASKIP_ISIF_HBLACK_PIXEL_VALUE_LIMTYPE ePixelValueLimit;   //!< Pixel value limitation for the Horizontal clamp value calculation
    VCAM_3ASKIP_ISIF_HBLACK_BASE_WINDOWTYPE eRightWindow;   //!< Base Window select
    uint8_t nWindowCountPerColor;       //!< Window count per color. Window count = CLHWC+1. Range: 1 - 32
} VCAM_3ASKIP_ISIFHBLACKPARAMSTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_black_params_horiz_t
{
    int32_t eSensor;
    VCAM_3ASKIP_ISIFHBLACKPARAMSTYPE mBlackParams_hor;
} VisionCam_3A_BlackParams_Horiz_t;

/*! \todo comment this */
typedef enum {
    VCAM_ISIF_VBLACK_2PIXEL_WIDE  = 0,
    VCAM_ISIF_VBLACK_4PIXEL_WIDE  = 1,
    VCAM_ISIF_VBLACK_8PIXEL_WIDE  = 2,
    VCAM_ISIF_VBLACK_16PIXEL_WIDE = 3,
    VCAM_ISIF_VBLACK_32PIXEL_WIDE = 4,
    VCAM_ISIF_VBLACK_64PIXEL_WIDE = 5,
    VCAM_ISIF_VBLACK_PIXEL_WIDE   = 0x7FFFFFFF
} VCAM_3ASKIP_ISIF_VBLACK_PIXEL_WIDTHTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_vert_black_pixel_width_t
{
    int32_t eSensor;
    VCAM_3ASKIP_ISIF_VBLACK_PIXEL_WIDTHTYPE eBlackPixWidth_Vert;
} VisionCam_3A_VertBlackPixelWidth_t;

/*! \todo comment this */
typedef enum {
    VCAM_ISIF_VALUE_HORIZONTAL_DIRECTION = 0,
    VCAM_ISIF_VALUE_CONFIG_REGISTER      = 1,
    VCAM_ISIF_VALUE_NOUPDATE             = 2,
    VCAM_ISIF_VALUE_MAX                  = 0x7FFFFFFF
} VCAM_3ASKIP_ISIF_VBLACK_RESET_MODETYPE;

/*! \todo comment this */
typedef struct _vcam_3a_black_reset_vert_t
{
    int32_t eSensor;
    VCAM_3ASKIP_ISIF_VBLACK_RESET_MODETYPE eBlackReset_vert;
} VisionCam_3A_BlacReset_Vert_t;

/*! \brief Vertical black clamp. */
typedef struct {
    uint16_t nVPos;     //!< Window Start position (V).Range: 0 - 8191
    uint16_t nVSize;    //!< Optical black V valid (V).Range: 0 - 8191
    uint16_t nHPos;     //!< position (H).Range: 0 - 8191
    VCAM_3ASKIP_ISIF_VBLACK_PIXEL_WIDTHTYPE HSize;      //!< Optical Black H valid (2^L).
    uint8_t line_avg_coef;                              //!< Line average coefficient (k).
    VCAM_3ASKIP_ISIF_VBLACK_RESET_MODETYPE reset_mode;  //!< Select the reset value for the Clamp value of the Previous line
    uint16_t reset_value;                               //!< Vertical black clamp reset value (U12) Range: 0 to 4095
} VCAM_3ASKIP_ISIF_VERTICAL_BLACK_PARAMSTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_black_params_vert_t
{
    int32_t eSensor;
    VCAM_3ASKIP_ISIF_VERTICAL_BLACK_PARAMSTYPE mBlackParams_vert;
} VisionCam_3A_BlacParams_Vert_t;

/*! \todo comment this */
typedef struct {
    uint8_t nEnable;                                                //!< clamp module enablement
    VCAM_3ASKIP_ISIF_HORIZONTAL_CLAMP_MODETYPE eHClampMode;         //!< horizontal clamp mode
    VCAM_3ASKIP_ISIF_BLACK_CLAMP_MODETYPE eBlackClampMode;          //!< black clamp mode
    uint16_t nDCOffsetClampVal;                                     //!< clamp dc-offset value
    uint16_t nBlackClampVStartPos;                                  //!< black clamp v-start position (range 0-8191)
    VCAM_3ASKIP_ISIF_VERTICAL_BLACK_PARAMSTYPE tHorizontalBlack;    //!< horizontal black clamp parameters
    VCAM_3ASKIP_ISIF_VERTICAL_BLACK_PARAMSTYPE tVerticalBlack;      //!< vertical black clamp parameters
} VCAM_3ASKIP_ISIF_CLAMP_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_black_clamp_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_ISIF_CLAMP_CFGTYPE mClamp;
} VisionCam_3A_Clamp_t;

/*! \todo comment this */
typedef struct {
    uint8_t gain_offset_featureflag;    //!< gain offset feature-flag
    uint16_t gain_r;    //!< gain R
    uint16_t gain_gr;   //!< gain GR
    uint16_t gain_gb;   //!< gain GB
    uint16_t gain_bg;   //!< gain BG
    uint16_t offset;    //!< offset

} VCAM_3ASKIP_ISIF_GAINOFFSET_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_gain_offset_t
{
    int32_t eSensor;
    VCAM_3ASKIP_ISIF_GAINOFFSET_CFGTYPE mGainOffset;
} VisionCam_3A_GainOffset_t;

/*! \todo comment this */
typedef enum {
    VCAM_ISIF_VDLC_WHOLE_LINE                = 0,
    VCAM_ISIF_VDLC_DISABLE_ABOVE_UPPER_PIXEL = 1,
    VCAM_ISIF_VDLC_MAX                       = 0x7FFFFFFF

} VCAM_3ASKIP_ISIF_VDLC_PIXEL_DEPENDENCYTYPE;

/*! \todo comment this */
typedef enum {
    VCAM_ISIF_VLDC_FED_THRO_ONSATURATION                 = 0,
    VCAM_ISIF_VLDC_HORIZONTAL_INTERPOLATION_ONSATURATION = 1,
    VCAM_ISIF_VLDC_HORIZONTAL_INTERPOLATION              = 2,
    VCAM_ISIF_VLDC_MAX                                   = 0x7FFFFFFF
} VCAM_3ASKIP_ISIF_VLDC_MODE_SELECTTYPE;

/*! \todo comment this */
typedef struct {
    uint16_t nVerticalDefectPosition;   //!< Vertical Defect position
    uint16_t nHorizontalDefectPosition; //!< horizontal defect position
    uint8_t nSub1ValueVldc;             //!< Defect correction Memory 2
    uint8_t nSub2LessThanVldc;          //!< Defect correction Memory 3
    uint8_t nSub3GreaterThanVldc;       //!< Defect correction Memory 4
} VCAM_3ASKIP_ISIF_VLDCDEFECT_LINEPARAMSTYPE;

/*! \todo comment this */
typedef struct {
    uint8_t nEnable;                                                    //!< enable VLDC module
    VCAM_3ASKIP_ISIF_VDLC_PIXEL_DEPENDENCYTYPE eDisableVldcUpperPixels; //!< pixel dependency
    uint8_t nVldcShiftVal;                                              //!< VLDC shift values
    VCAM_3ASKIP_ISIF_VLDC_MODE_SELECTTYPE eVldcModeSelect;              //!< VLDC mode select
    uint16_t nVldcSaturationLvl;                                        //!< VLDC saturation level: U12 range 0 - 4095
    uint8_t nDefectLines;                                               //!< number of defect lines-maximum 8
    VCAM_3ASKIP_ISIF_VLDCDEFECT_LINEPARAMSTYPE tVldcDefectLineParams;   //!< defect line paramaters
} VCAM_3ASKIP_ISIF_VLDC_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_vdlc_t
{
    int32_t eSensor;
    VCAM_3ASKIP_ISIF_VLDC_CFGTYPE mVLDC;
} VisionCam_3A_VDLC_t;

/*! \todo comment this */
typedef enum {
    VCAM_NOISE_FILTER_1   = 1,
    VCAM_NOISE_FILTER_2   = 2,
    VCAM_NOISE_FILTER_MAX = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_NOISE_FILTERTYPE;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_NF_SPR_SINGLE = 0,
    VCAM_IPIPE_NF_SPR_LUT    = 1,
    VCAM_IPIPE_NF_SPR_MAX    = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_NF_SELTYPE;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_NF_LSC_GAIN_OFF  = 0,
    VCAM_IPIPE_NF_LSC_GAIN_ON   = 1,
    VCAM_IPIPE_NF_LSC_GAIN_MAX  = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_NF_LSC_GAINTYPE;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_NF_SAMPLE_BOX     = 0,
    VCAM_IPIPE_NF_SAMPLE_DIAMOND = 1,
    VCAM_IPIPE_NF_SAMPLE_MAX     = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_NF_SAMPLE_METHODTYPE;

/*! \todo comment this */
typedef struct {
    uint8_t nEnable;                                //!< Enable-disable for Noise filter in pipe
    VCAM_3ASKIP_IPIPE_NOISE_FILTERTYPE eNFNum;      //!< Noise filter number
    VCAM_3ASKIP_IPIPE_NF_SELTYPE eSel;              //!< Selecting the spread in NF
    VCAM_3ASKIP_IPIPE_NF_LSC_GAINTYPE eLscGain;     //!< Controling the lsc gain applied in Noise Filter
    VCAM_3ASKIP_IPIPE_NF_SAMPLE_METHODTYPE eTyp;    //!< Selecting the sampling method
    uint8_t                                   nDownShiftVal;
    uint8_t                                   nSpread;
    uint16_t                                  pThr[VCAM_ISS_NF_THR_COUNT];
    uint8_t                                   pStr[VCAM_ISS_NF_STR_COUNT];
    uint8_t                                   pSpr[VCAM_ISS_NF_SPR_COUNT];
    uint16_t                                  nEdgeMin;
    uint16_t                                  nEdgeMax;
} VCAM_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_noise_filter_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE eNoiseFilterConfig;
} VisionCam_3A_NoiseFilterConfig_t;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_GIC_LSC_GAIN_OFF = 0,
    VCAM_IPIPE_GIC_LSC_GAIN_ON  = 1,
    VCAM_IPIPE_GIC_LSC_GAIN_MAX = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_GIC_LSC_GAINTYPE;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_GIC_GICTHR = 0,
    VCAM_IPIPE_GIC_NF2THR = 1,
    VCAM_IPIPE_GIC_MAX    = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_GIC_SELTYPE;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_GIC_DIFF_INDEX = 0,
    VCAM_IPIPE_GIC_HPD_INDEX  = 1,
    VCAM_IPIPE_GIC_INDEX_MAX  = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_GIC_INDEXTYPE;

/*! \brief GIC: Green Inbalance Correction */
typedef struct {
    /* only if data format is GR,GB */
    uint16_t nEnable;                               //!< Enable-disable for diffrerent components in ipipe
    VCAM_3ASKIP_IPIPE_GIC_LSC_GAINTYPE eLscGain;    //!< Selecting LSC gain in GIC
    VCAM_3ASKIP_IPIPE_GIC_SELTYPE eSel;             //!< Slection of threshold vaue in GIC filter
    VCAM_3ASKIP_IPIPE_GIC_INDEXTYPE eTyp;           //!< Selecting the index in GIC
    uint8_t                            nGicGain;
    uint8_t                            nGicNfGain;
    uint16_t                           nGicThr;
    uint16_t                           nGicSlope;
} VCAM_3ASKIP_IPIPE_GIC_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_gic_cfg_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_GIC_CFGTYPE mGIC_config;
} VisionCam_3A_GIC_Config_t;

/*! \todo comment this */
typedef struct {
    /*offseet after R,GR,GB,B*/
    uint16_t pOffset[4];
    /*gain for R gr gb B*/
    uint16_t pGain[4];
} VCAM_3ASKIP_IPIPE_WB_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_wb_cfg_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_WB_CFGTYPE eWB_config;
} VisionCam_3A_WB_Config_t;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_CFA_MODE_2DIR    = 0,
    VCAM_IPIPE_CFA_MODE_2DIR_DA = 1,
    VCAM_IPIPE_CFA_MODE_DAA     = 2,
    VCAM_IPIPE_CFA_MODE_MAX     = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_CFA_MODETYPE;

/*! \todo comment this */
typedef struct _vcam_3a_cfa_mode_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_CFA_MODETYPE eCFA_mode;
} VisionCam_3A_CFA_mode_t;

/*! \todo comment this */
typedef struct {
    uint16_t nHpfThr;
    uint16_t nHpfSlope;
    uint16_t nMixThr;
    uint16_t nMixSlope;
    uint16_t nDirThr;
    uint16_t nDirSlope;
    uint16_t nDirNdwt;
} VCAM_3ASKIP_IPIPE_CFA_DIRTYPE;

/*! \todo comment this */
typedef struct {
    uint8_t  nMonoHueFra;
    uint8_t  nMonoEdgThr;
    uint16_t nMonoThrMin;
    uint16_t nMonoThrSlope;
    uint16_t nMonoSlpMin;
    uint16_t nMonoSlpSlp;
    uint16_t nMonoLpwt;
} VCAM_3ASKIP_IPIPE_CFA_DAATYPE;

/*! \todo comment this */
typedef struct {
    uint8_t                           nEnable;
    VCAM_3ASKIP_IPIPE_CFA_MODETYPE eMode;
    VCAM_3ASKIP_IPIPE_CFA_DIRTYPE  tDir;
    VCAM_3ASKIP_IPIPE_CFA_DAATYPE  tDaa;
} VCAM_3ASKIP_IPIPE_CFA_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_cfa_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_CFA_CFGTYPE mCFA_config;
} VisionCam_3A_CFA_Config_t;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_GAMMA_TBL_64  = 0,
    VCAM_IPIPE_GAMMA_TBL_128 = 1,
    VCAM_IPIPE_GAMMA_TBL_256 = 2,
    VCAM_IPIPE_GAMMA_TBL_512 = 3,
    VCAM_IPIPE_GAMMA_TBL_MAX = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_GAMMA_TABLE_SIZETYPE;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_GAMMA_BYPASS_ENABLE  = 1,
    VCAM_IPIPE_GAMMA_BYPASS_DISABLE = 0,
    VCAM_IPIPE_GAMMA_BYPASS_MAX     = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_GAMMA_BYPASSTYPE;

/*! \todo comment this */
typedef struct {
    uint8_t                                   nEnable;
    VCAM_3ASKIP_IPIPE_GAMMA_TABLE_SIZETYPE eGammaTblSize;
    uint8_t nTbl;    //!< May not be needed, since table is always in RAM
    //!< o not bypassed
    VCAM_3ASKIP_IPIPE_GAMMA_BYPASSTYPE eBypassB;
    VCAM_3ASKIP_IPIPE_GAMMA_BYPASSTYPE eBypassG;
    VCAM_3ASKIP_IPIPE_GAMMA_BYPASSTYPE eBypassR;
    /*poVCAM_Ser to red gamma table      Red gamma table -   (U8Q0)
      *                                 Blue gamma table -  (U8Q0)
      *                                 Green gamma table - (U8Q0)
      */
    int8_t pRedTable[VCAM_ISS_PREV_GAMMA_TABLE];
    int8_t pBlueTable[VCAM_ISS_PREV_GAMMA_TABLE];
    int8_t pGreenTable[VCAM_ISS_PREV_GAMMA_TABLE];
} VCAM_3ASKIP_IPIPE_GAMMA_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_gamma_table_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_GAMMA_CFGTYPE mGammaTable;
} VisionCam_3A_GammaTableConfig_t;

/*! \todo comment this */
typedef struct {
    /*      [RR] [GR] [BR]
     *      [RG] [GG] [BG]
     *      [RB] [GB] [BB]*/
    /* Blending values(S12Q8 format) */
    /*RR,GR,BR,RG,GG,BG,RB,GB,BB each 11 bits*/
    uint16_t pMulOff[VCAM_ISS_PREV_RGB2RGB_MATRIX][VCAM_ISS_PREV_RGB2RGB_MATRIX];
    /* Blending offset value for R,G,B - (S10Q0) */
    /*R,G,B each 13 bits*/
    uint16_t pOft[VCAM_ISS_PREV_RGB2RGB_OFFSET];
} VCAM_3ASKIP_IPIPE_RGBRGB_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_rgb_to_rgb_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_RGBRGB_CFGTYPE mRGB_config;
} VisionCam_3A_RGB2RGB_t;

/*! \todo comment this */
typedef struct {
    uint8_t nBrightness;
    uint8_t nContrast;
    /*      [CSCRY]   [CSCGY]  [CSCBY]
     *      [CSCRCB] [CSCGCB] [CSCBCB]
     *      [CSCRCR] [CSCGCR] [CSCBCR] */
    /* Color space conversion coefficients(S10Q8) */
    /*RY,GY,BY,RCB,GCB,BCB ,RCR,GCR,BCR 12 bits*/
    int16_t pMulVal[VCAM_ISS_PREV_RGB2YUV_MATRIX][VCAM_ISS_PREV_RGB2YUV_MATRIX];
    /* CSC offset values for Y offset, CB offset
     *  and CR offset respectively (S8Q0) */
    /*Y,CB,CR -11bits*/
    int16_t pOffset[VCAM_ISS_PREV_RGB2YUV_OFFSET];
} VCAM_3ASKIP_IPIPE_RGBYUV_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_rgb_to_yuv_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_RGBYUV_CFGTYPE mRGB2YUV;
} VisionCam_3A_RGB2YUV_t;

/*! \todo comment this */
typedef enum {
    /*! Cr CB unmodified */
    VCAM_IPIPE_GBCE_METHOD_Y_VALUE  = 0,
    VCAM_IPIPE_GBCE_METHOD_GAIN_TBL = 1,
    VCAM_IPIPE_GBCE_METHOD_MAX      = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_GBCE_METHODTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_gbce_method_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_GBCE_METHODTYPE eGBCE_method;
} VisionCam_3A_GBCE_Method_t;

/*! \todo comment this */
typedef struct {
    uint16_t nEnable;                                   //!< Enable-disable for GBCE in ipipe
    VCAM_3ASKIP_IPIPE_GBCE_METHODTYPE nTyp;             //!< Selecting the type of GBCE method
    uint16_t pLookupTable[VCAM_ISS_GBCE_TABLE_SIZE];    //!< GBCE LookUp Tabale
} VCAM_3ASKIP_IPIPE_GBCE_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_gbce_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_GBCE_CFGTYPE mGBCE_config;
} VisionCam_3A_GBCE_Config_t;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_PROC_COMPR_NO,
    VCAM_IPIPE_PROC_COMPR_DPCM,
    VCAM_IPIPE_PROC_COMPR_ALAW,
    VCAM_IPIPE_PROC_COMPR_PACK,
    VCAM_IPIPE_PROC_COMPR_MAX = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_PROC_COMPRESSIONTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_conpression_type_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_PROC_COMPRESSIONTYPE eCompression;
} VisionCam_3A_CompressionType_t;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_YUV_PHS_POS_COSITED  = 0,
    VCAM_IPIPE_YUV_PHS_POS_CENTERED = 1
} VCAM_3ASKIP_IPIPE_YUV_PHASE_POSTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_yuv_phase_position_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_YUV_PHASE_POSTYPE eYuvPhasePosition;
} VisionCam_3A_YuvPhasePosition_t;

/*! \brief ISS configuration structure which controls the operation of yuv444 to yuv 422 */
typedef struct {
    VCAM_3ASKIP_IPIPE_YUV_PHASE_POSTYPE ePos;
    uint8_t                                nLpfEn;
} VCAM_3ASKIP_IPIPE_YUV444YUV422_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_yuv444_to_yuv422_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_YUV444YUV422_CFGTYPE mYuvToYuv_config;
} VisionCam_3A_Yuv444_toYuv422_Config_t;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_HALO_REDUCTION_ENABLE  = 1,
    VCAM_IPIPE_HALO_REDUCTION_DISABLE = 0,
    VCAM_IPIPE_HALO_REDUCTION_MAX     = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_EE_HALO_CTRLTYPE;

/*! \brief ISS configuration structure Edge enhancement */
typedef struct {
    /*! Defect Correction Enable */
    uint16_t nEnable;
    /*! Enable-disable for halo reduction in Edge enhancement */
    VCAM_3ASKIP_IPIPE_EE_HALO_CTRLTYPE eHaloReduction;
    /*! 9 coefficients */
    int16_t pMulVal[VCAM_ISS_COEFF];
    uint8_t  nSel;
    uint8_t  nShiftHp;
    uint16_t nThreshold;
    uint16_t nGain;
    uint16_t nHpfLowThr;
    uint8_t  nHpfHighThr;
    uint8_t  nHpfGradientGain;
    uint8_t  nHpfgradientOffset;
    int16_t pEeTable[VCAM_ISS_EE_TABLE_SIZE];
} VCAM_3ASKIP_IPIPE_EE_CFGTYPE;

typedef struct _vcam_3a_halo_reduction_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_EE_CFGTYPE eEdgeEnhConfig;
} VisionCam_3A_EdgeEnhancement_config_t;

/*! \brief ISS configuration structure for CAR module in ipipe */
typedef struct {
    /*! Enable-disable for CAR module in ipipe */
    uint8_t  nEnable;
    uint8_t  nTyp;
    uint8_t  nSw0Thr;
    uint8_t  nSw1Thr;
    uint8_t  nHpfType;
    uint8_t  nHpfShift;
    uint8_t  nHpfThr;
    uint8_t  nGn1Gain;
    uint8_t  nGn1Shift;
    uint16_t nGn1Min;
    uint8_t  nGn2Gain;
    uint8_t  nGn2Shift;
    uint16_t nGn2Min;
} VCAM_3ASKIP_IPIPE_CAR_CFGTYPE;

typedef struct _vcam_3a_car_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_CAR_CFGTYPE mCar;
} VisionCam_3A_CAR_config_t;

/*! \brief ISS configuration structure for LSC */
typedef struct {
    uint16_t nVOffset;
    int16_t nVLinearCoeff;
    int16_t nVQuadraticCoeff;
    uint8_t nVLinearShift;
    uint8_t  nVQuadraticShift;
    uint16_t nHOffset;
    int16_t nHLinearCoeff;
    int16_t nHQuadraticCoeff;
    uint8_t nHLinearShift;
    uint8_t nHQuadraticShift;
    uint8_t nGainR;     //!< Gain value for R
    uint8_t nGainGR;    //!< Gain value for GR
    uint8_t nGainGB;    //!< Gain value for GB
    uint8_t nGainB;     //!< Gain value for B
    uint8_t nOffR;      //!< Offset value for R
    uint8_t nOffGR;     //!< Offset value for GR
    uint8_t nOffGB;     //!< Offset value for GB
    uint8_t nOffB;      //!< Offset value for B
    uint8_t nShift;
    uint16_t nMax;
} VCAM_3ASKIP_IPIPE_LSC_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_lsc_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_LSC_CFGTYPE mLsc;
} VisionCam_3A_LscConfig_t;

/*! \todo comment this */
typedef struct {
    uint16_t nVPos;
    uint16_t nVSize;
    uint16_t nHPos;
    uint16_t nHSize;
} VCAM_3ASKIP_IPIPE_HIST_DIMTYPE;

/*! \brief ISS configuration structure for Histogram in ipipe */
typedef struct {
    uint8_t nEnable;    //!< Enable-disable for Histogram in pipe
    uint8_t nOst;
    uint8_t nSel;
    uint8_t nType;
    uint8_t nBins;
    uint8_t nShift;
    uint8_t nCol;       //!< Bits [3:0], 0 is disable
    uint8_t nRegions;   //!< [3:0], 0 is disable
    VCAM_3ASKIP_IPIPE_HIST_DIMTYPE pHistDim[VCAM_ISS_HIST_DIMS_COUNT];    //!< Pointer to array of 4 structs
    uint8_t                           nClearTable;
    uint8_t                           nTableSel;
    uint8_t pGainTbl[VCAM_ISS_HIST_GAIN_TBL];    //!< r,gr,gb,b
} VCAM_3ASKIP_IPIPE_HIST_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_histogram_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_HIST_CFGTYPE mHistogramConfig;
} VisionCam_3A_HistogramConfig_t;

/*! \todo comment this */
typedef enum {
    VCAM_BOXCAR_DISABLED     = 0,
    VCAM_BOXCAR_ENABLED      = 1,
    VCAM_BOXCAR_ENB_DSB_MAX  = 0x7FFFFFFF
} VCAM_3ASKIP_BOXCAR_ENABLETYPE;

/*! \todo comment this */
typedef struct _vcam_3a_box_car_t
{
    int32_t eSensor;
    VCAM_3ASKIP_BOXCAR_ENABLETYPE eBoxCarEnable;
} VisionCam_3A_BoxCAR_t;

/*! \todo comment this */
typedef enum {
    VCAM_BOXCAR_FREE_RUN     = 0,
    VCAM_BOXCAR_ONE_SHOT     = 1,
    VCAM_BOXCAR_FREE_ONE_MAX = 0x7FFFFFFF
} VCAM_3ASKIP_BOXCAR_MODETYPE;

/*! \todo comment this */
typedef struct _vcam_3a_box_car_mode_t
{
    int32_t eSensor;
    VCAM_3ASKIP_BOXCAR_MODETYPE eBoxCarMode;
} VisionCam_3A_BoxCarMode_t;

/*! \todo comment this */
typedef enum {
    VCAM_BOXCAR_8x8      = 0,
    VCAM_BOXCAR_16x16    = 1,
    VCAM_BOXCAR_SIZE_MAX = 0x7FFFFFFF
} VCAM_3ASKIP_BOXCAR_SIZETYPE;

/*! \todo comment this */
typedef struct _vcam_3a_box_car_size_t
{
    int32_t eSensor;
    VCAM_3ASKIP_BOXCAR_SIZETYPE eBoxCarSize;
} VisionCam_3A_BoxCarSize_t;

/*! \todo comment this */
typedef struct {
    uint8_t nEnable;
    uint8_t nOst;    //!< One shot or free run
    uint8_t nBoxSize;
    uint8_t nShift;
    uint32_t pAddr[VCAM_IPIPE_BOXCAR_MAX_BUFF_SIZE];
} VCAM_3ASKIP_IPIPE_BOXCAR_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_box_car_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_BOXCAR_CFGTYPE eBoxCarConfig;
} VisionCam_3A_BoxCarConfig_t;

/*! \todo comment this */
typedef struct {
    uint8_t nVectors;
    uint8_t nShift;
    uint16_t nVPos;    //!< Vertical position
    uint16_t nHPos;    //!< Horizontal position
    uint16_t nVNum;    //!< Vertical number
    uint16_t nHNum;    //!< Horizontal number
    uint8_t nVSkip;    //!< Horizontal skip
    uint8_t nHSkip;    //!< Vertical skip
} VCAM_3ASKIP_IPIPE_BSCPOS_PARAMSTYPE;

/*! \todo comment this */
typedef struct {
    uint8_t nEnable;    //!< Enable-disable for BSC in pipe
    uint8_t nMode;      //!< BSC mode in ipipe
    uint8_t nColSample; //!< BSC Color Sample
    uint8_t nRowSample; //!< BSC Row Sample
    uint8_t nElement;   //!< Y or CB or CR
    VCAM_3ASKIP_IPIPE_BSCPOS_PARAMSTYPE nColPos;    //!< Color Position parameters
    VCAM_3ASKIP_IPIPE_BSCPOS_PARAMSTYPE nRowPos;    //!< Row Position parameters
} VCAM_3ASKIP_IPIPE_BSC_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_bsc_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_BSC_CFGTYPE mBscConfig;
} VisionCam_3A_BscConfig_t;

/*! \brief Enable-disable enum for DFS in ipipeif */
typedef enum {
    VCAM_IPIPEIF_FEATURE_ENABLE  = 1,
    VCAM_IPIPEIF_FEATURE_DISABLE = 0,
    VCAM_IPIPEIF_FEATURE_MAX     = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE;

/*! \todo comment this */
typedef struct {
    VCAM_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE eDfsGainEn;    //!< Enable-disable for DFS in ipipeif
    uint16_t nDfsGainVal;   //!< Valid only if eDfsGainEn = VCAM_IPIPEIF_FEATURE_ENABLE
    uint16_t nDfsGainThr;   //!< Valid only if eDfsGainEn = VCAM_IPIPEIF_FEATURE_ENABLE
    uint16_t nOclip;        //!< Valid only if eDfsGainEn = VCAM_IPIPEIF_FEATURE_ENABLE
    uint8_t nDfsDir;        //!< Set to 0 if Sensor Parallel interface data is to be subtracted by DRK frm in SDRAM
} VCAM_3ASKIP_IPIPEIF_DFS_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_dfs_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPEIF_DFS_CFGTYPE mDfsConfig;
} VisionCam_3A_DfsConfig_t;

/*! \brief ISS struct to control defect pixel corrction in ipipeif */
typedef struct {
    VCAM_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE eDpcEn;    //!< Enable-disable for DPC in ipipeif
    uint16_t                                  eDpcThr;
} VCAM_3ASKIP_IPIPEIF_DPC_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_dpc_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPEIF_DPC_CFGTYPE mDpcConfig;
} VisionCam_3A_DpcConfig_t;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPEIF_DPCM_PREDICTION_SIMPLE   = 0,
    VCAM_IPIPEIF_DPCM_PREDICTION_ADVANCED = 1,
    VCAM_IPIPEIF_DPCM_PREDICTION_MAX      = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPEIF_DPCM_PRED_TYPTYPE;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPEIF_DPCM_BIT_SIZE_8_10 = 0,
    VCAM_IPIPEIF_DPCM_BIT_SIZE_8_12 = 1,
    VCAM_IPIPEIF_DPCM_BIT_SIZE_MAX = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPEIF_DPCM_BIT_SIZETYPE;

/*! \todo comment this */
typedef struct {
    //!< Enable-disable for DPCM in ipipeif
    VCAM_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE nDpcmEn;
    //!< Valid only if DPCM is enabled; dpcm_en=1
    VCAM_3ASKIP_IPIPEIF_DPCM_PRED_TYPTYPE nDpcmPredictor;
    //!< Valid only if DPCM is enabled; dpcm_en=1
    VCAM_3ASKIP_IPIPEIF_DPCM_BIT_SIZETYPE nDpcmBitSize;
} VCAM_3ASKIP_IPIPEIF_DPCM_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_dpcm_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPEIF_DPCM_CFGTYPE mDPCM;
} VisionCam_3A_DPCM_Config_t;

/*! \todo comment this */
typedef enum {
    VCAM_RSZ_IP_IPIPE   = 0,
    VCAM_RSZ_IP_IPIPEIF = 1,
    VCAM_RSZ_IP_MAX     = 0x7FFFFFFF
} VCAM_3ASKIP_RSZ_IP_PORT_SELTYPE;

/*! \todo comment this */
typedef enum {
    VCAM_MEM_INPUT_IPIPEIF,
    VCAM_MEM_INPUT_CCP,
    VCAM_MEM_INPUT_MAX     = 0x7FFFFFFF
} VCAM_3ASKIP_MEM_PROC_INPUT_SELECTTYPE;

/*!< \brief ISS low pass filter params for Horizontal resizing */
typedef struct {
    uint8_t nCIntensity;
    uint8_t nYIntensity;
} VCAM_3ASKIP_RSZ_LPF_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_resizer_lps_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_RSZ_LPF_CFGTYPE mRszLpfConfig;
} VisionCam_3A_ResizerLowPassFilter_t;

/*! \brief Enable-Disable H3A Features */
typedef enum {
    VCAM_H3A_FEATURE_DISABLE = 0,
    VCAM_H3A_FEATURE_ENABLE  = 1,
    VCAM_H3A_FEATURE_MAX     = 0x7FFFFFFF
} VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE;

/*! \brief H3A Pixel Configuration */
typedef struct {
    uint16_t nVPos;    //!< AEWINSTART WINSV  AFPAXSTART PAXSV
    uint8_t nVSize;    //!< AEWWIN1 WINW      AFPAX1 PAXH
    uint16_t nHPos;    //!< AEWINSTART WINSH  AFPAXSTART PAXSH
    uint8_t nHSize;    //!< AEWWIN1 WINH      AFPAX1 PAXW
    uint8_t nVCount;    //!< AEWWIN1 WINVC     AFPAX2 PAXVC
    uint8_t nVIncr;    //!< AEWSUBWIN AEWINCV AFPAX2 AFINCV
    uint8_t nHCount;    //!< AEWWIN1 WINHC     AFPAX2 PAXHC
    uint8_t nHIncr;    //!< AEWSUBWIN AEWINCH AFPAX2 AFINCH
} VCAM_3ASKIP_H3A_PAXEL_CFGTYPE;

/*! \todo comment this */
typedef struct {
    uint16_t nVPos;    //!< AEWINBLK WINSV single row of black line vpos
    uint16_t nHPos;    //!< AEWINBLK WINH  win height
} VCAM_3ASKIP_H3A_AEWB_BLKDIMSTYPE;

/*! \todo comment this */
typedef enum {
    VCAM_H3A_AEWB_OP_FMT_SUM_OF_SQR = 0,
    VCAM_H3A_AEWB_OP_FMT_MINMAX     = 1,
    VCAM_H3A_AEWB_OP_FMT_SUM_ONLY   = 2,
    VCAM_H3A_AEWB_OP_FMT_MAX        = 0x7FFFFFFF
} VCAM_3ASKIP_H3A_AEWB_OP_FMTTYPE;


/*! \todo comment this */
typedef struct {
    VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE eAewbEnable;    //!< Enable-disable the Aewb engine
    VCAM_3ASKIP_H3A_PAXEL_CFGTYPE tAewbPaxelWin;    //!< Paxel config
    VCAM_3ASKIP_H3A_AEWB_BLKDIMSTYPE tBlkWinDims;    //!< AEWB Block Dimensions
    VCAM_3ASKIP_H3A_AEWB_OP_FMTTYPE eAeOpFmt;    //!< AEWCFG AEFMT
    uint8_t nShiftValue;    //!< AEWCFG SUMFST
    uint16_t nSaturationLimit;    //PCR AVE2LMT
    uint32_t pAewbOpAddr[VCAM_AEWB_H3A_MAX_BUFF_SIZE];
} VCAM_3ASKIP_H3A_AEWB_PARAMTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_aewb_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_H3A_AEWB_PARAMTYPE mAEWB_config;
} VisionCam_3A_AEWB_config_t;

/*! \todo comment this */
typedef enum {
    VCAM_H3A_AF_RGBPOS_GR_GB_BAYER  = 0,
    VCAM_H3A_AF_RGBPOS_RG_GB_BAYER  = 1,
    VCAM_H3A_AF_RGBPOS_GR_BG_BAYER  = 2,
    VCAM_H3A_AF_RGBPOS_RG_BG_BAYER  = 3,
    VCAM_H3A_AF_RGBPOS_GG_RB_CUSTOM = 4,
    VCAM_H3A_AF_RGBPOS_RB_GG_CUSTOM = 5,
    VCAM_H3A_AF_RGBPOS_MAX          = 0x7FFFFFFF
} VCAM_3ASKIP_H3A_RGB_POSTYPE;

/*! \todo comment this */
typedef struct {
    uint16_t pIirCoef[11];
    uint16_t nHfvThres;
} VCAM_3ASKIP_H3A_AF_IIRPARAMTYPE;

/*! \todo comment this */
typedef struct {
    uint8_t pFirCoef[5];
    uint8_t nVfvThres;
} VCAM_3ASKIP_H3A_AF_FIRPARAMTYPE;

/*! \todo comment this */
typedef struct {
    VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE eAfEnable;       //!< Enable-Disable the AF engine
    VCAM_3ASKIP_H3A_RGB_POSTYPE eRgbPos;                //!< Valid only if vertical focus is enabled
    VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE ePeakModeEn;     //!< Enable-Disable the PeakMode engine
    VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE eVerticalFocusEn;    //!< Enable-Disable the VerticalFocus engine
    uint16_t nIirStartPos;                              //!< AFIIRSH IIRSH
    VCAM_3ASKIP_H3A_PAXEL_CFGTYPE tAfPaxelWin;          //!< Paxel config
    VCAM_3ASKIP_H3A_AF_IIRPARAMTYPE tIir1;
    VCAM_3ASKIP_H3A_AF_IIRPARAMTYPE tIir2;
    VCAM_3ASKIP_H3A_AF_FIRPARAMTYPE tFir1;
    VCAM_3ASKIP_H3A_AF_FIRPARAMTYPE tFir2;
    uint32_t pAfOpAddr[VCAM_AF_H3A_MAX_BUFF_SIZE];       //!< AEWBUFST AEWBUFST, 64 bit aligned address
} VCAM_3ASKIP_H3A_AF_PARAMTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_h3a_af_param_t
{
    int32_t eSensor;
    VCAM_3ASKIP_H3A_AF_PARAMTYPE mH3A_AutoFocusParam;
} VisionCam_3A_H3A_AutoFocusParam_t;

/*! \todo comment this */
typedef struct {
    VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE eAfMedianEn;     //!< Enable-disable for H3A AF Median engine
    VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE eAewbMedianEn;   //!< Enable-disable for H3A AEWB Median engine
    uint8_t nMedianFilterThreshold;                     //!< Valid only if eAfMedianEn is set to VCAM_H3A_FEATURE_ENABLE
    VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE eAfAlawEn;       //!< Enable-disable for AFAlaw engine
    VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE eAewbAlawEn;     //!< Enable-disable for AEWBAlaw engine
    VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE eIpipeifAveFiltEn;   //!< Enable-disable for IPIPEIFAve filter engine
    VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE eH3aDecimEnable;     //!< Enable-disable for H3A decimation
    uint32_t nReserved;    //!< Reserved
} VCAM_3ASKIP_H3A_COMMON_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_h3a_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_H3A_COMMON_CFGTYPE mH3A_CommonConfig;
} VisionCam_3A_H3A_CommonConfig_t;

/*! \todo comment this */
typedef struct {
    uint8_t enable;    //Enable-disable for 3Dcc
    uint32_t pInBiffData[VCAM_ISS_3D_LUT_SIZE];    //3D LUT
} VCAM_3ASKIP_IPIPE_3DCC_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_3dcc_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_3DCC_CFGTYPE m3dccConfig;
} VisionCam_3A_3DCC_config_t;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_DPC_LUT_REPLACE_BLACK = 0,
    VCAM_IPIPE_DPC_LUT_REPLACE_WHITE = 1,
    VCAM_IPIPE_DPC_LUT_REPLACE_MAX   = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_DPC_LUT_REPLACEMENTTYPE;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_DPC_LUT_TBL_SIZE_1024 = 0,
    VCAM_IPIPE_DPC_LUT_TBL_SIZE_INF  = 1,
    VCAM_IPIPE_DPC_LUT_TBL_SIZE_MAX  = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_DPC_LUT_TBL_SIZETYPE;

/*! \todo comment this */
typedef struct {
    /*! Enable-disable for DPC Lut */
    uint16_t                                     nEnable;
    VCAM_3ASKIP_IPIPE_DPC_LUT_TBL_SIZETYPE    eTableType;
    VCAM_3ASKIP_IPIPE_DPC_LUT_REPLACEMENTTYPE eReplaceType;
    uint16_t                                     pLutValidAddr[256];
    uint16_t                                     nLutSize;
    uint32_t                                     pLutTable0Addr[256];
    uint32_t                                     pLutTable1Addr[256];
} VCAM_3ASKIP_IPIPE_DPCLUT_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_dpc_lut_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_DPCLUT_CFGTYPE mDPC_lut;
} VisionCam_3A_DPC_Lut_t;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_DPC_OTF_ALG_MINMAX2 = 0,
    VCAM_IPIPE_DPC_OTF_ALG_MINMAX3 = 1,
    VCAM_IPIPE_DPC_OTF_ALG_MAX     = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_DPC_ALGOTYPE;

/*! \todo comment this */
typedef enum {
    VCAM_IPIPE_DPC_OTF_MAX1_MIN1 = 0,
    VCAM_IPIPE_DPC_OTF_MAX2_MIN2 = 1,
    VCAM_IPIPE_DPC_OTF_MAX       = 0x7FFFFFFF
} VCAM_3ASKIP_IPIPE_DPC_OTFTYPE;

/*! \todo comment this */
typedef struct {
    uint16_t thr_cor_r;
    uint16_t thr_cor_gr;
    uint16_t thr_cor_gb;
    uint16_t thr_cor_b;

    uint16_t thr_det_r;
    uint16_t thr_det_gr;
    uint16_t thr_det_gb;
    uint16_t thr_det_b;
} VCAM_3ASKIP_IPIPE_DPCOTF_DPC2TYPE;

/*! \todo comment this */
typedef struct {
    uint8_t  nDThr;
    uint8_t  nDSlp;
    uint16_t nDMin;
    uint16_t nDMax;
} VCAM_3ASKIP_IPIPE_DPCOFT_FILTERTYPE;

/*! \todo comment this */
typedef struct {
    uint8_t                                eShift;
    VCAM_3ASKIP_IPIPE_DPCOFT_FILTERTYPE eOtfCorr;
    VCAM_3ASKIP_IPIPE_DPCOFT_FILTERTYPE eOtfDett;
} VCAM_3ASKIP_IPIPE_DPCOTF_DPC3TYPE;

/*! \brief Union which helps selec either dpc2-dpc 3 params */
typedef union {
    VCAM_3ASKIP_IPIPE_DPCOTF_DPC2TYPE tDpc2Params;
    VCAM_3ASKIP_IPIPE_DPCOTF_DPC3TYPE tDpc3Params;
} VCAM_3ASKIP_IPIPE_DPCOTF_FILTER_PARAMSTYPE;

/*! \todo comment this */
typedef struct {
    uint8_t nEnable;    //!< Enable-disable for DPC
    VCAM_3ASKIP_IPIPE_DPC_OTFTYPE eType;    //!< ISS dpc otf type
    VCAM_3ASKIP_IPIPE_DPC_ALGOTYPE eAlgo;    //!< ISS dpc otf definitions
    VCAM_3ASKIP_IPIPE_DPCOTF_FILTER_PARAMSTYPE tDpcData;    //!< Union which helps selec either dpc2-dpc 3 params
} VCAM_3ASKIP_IPIPE_DPCOTF_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_dpc_otf_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_DPCOTF_CFGTYPE mDPC;
} VisionCam_3A_DpcOtfConfig_t;

/*! \todo comment this */
typedef struct {
    uint16_t thr;
    uint16_t gain;
    uint16_t shift;
    uint16_t min;
} VCAM_3ASKIP_IPIPE_CHROMA_PARAMSTYPE;

/*! \todo comment this */
typedef struct {
    //!< Enable-disable for CGC in ipipe
    uint16_t                               enable;
    VCAM_3ASKIP_IPIPE_CHROMA_PARAMSTYPE y_chroma_low;
    VCAM_3ASKIP_IPIPE_CHROMA_PARAMSTYPE y_chroma_high;
    VCAM_3ASKIP_IPIPE_CHROMA_PARAMSTYPE c_chroma;
} VCAM_3ASKIP_IPIPE_CGS_CFGTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_cgs_config_t
{
    int32_t eSensor;
    VCAM_3ASKIP_IPIPE_CGS_CFGTYPE mCgs;
} VisionCam_3A_CGS_Config_t;

/*! \todo comment this */
typedef enum {
    VCAM_TRANSFER_ONLINE  = 0,
    VCAM_TRANSFER_OFFLINE = 1,
    VCAM_TRANSFER_MAX     = 0x7FFFFFFF
} VCAM_3ASKIP_CAM_TRANSFERTYPE;

/*! \todo comment this */
typedef struct {
    uint32_t exp;
    uint32_t a_gain;
    uint8_t  mask;
    uint32_t nAgainErr;
    uint32_t nDigitalISPGain;
} VCAM_3ASKIP_CAM_CONTROL_EXPGAINTYPE;

/*! \todo comment this */
typedef struct _vcam_3a_exposure_gain_control_t
{
    int32_t eSensor;
    VCAM_3ASKIP_CAM_CONTROL_EXPGAINTYPE mExpGainControl;
} VisionCam_3A_ExpGainControl_t;


#endif // _VISION_CAM_3A_EXPORT_TYPES_H_
