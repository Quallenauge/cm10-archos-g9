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

#ifndef _VISION_CAM_TEST_3A_DATA_
#define _VISION_CAM_TEST_3A_DATA_

#include "VisionCamTestTypes.h"
#include <dvp/VisionCam_3A_ExportTypes.h>

entryKey KEY_3A_SENSOR_SELECTION    = "se";
entryKey KEY_3A_LSC_2D              = "lsc-2d";
entryKey KEY_3A_CLAMP               = "clamp";
entryKey KEY_3A_GAIN_OFFSET         = "gain-offs";
entryKey KEY_3A_VLDC                = "vldc";
entryKey KEY_3A_NF1                 = "nf-1";
entryKey KEY_3A_NF2                 = "nf-2";
entryKey KEY_3A_GIC                 = "gic";
entryKey KEY_3A_WB                  = "wb";
entryKey KEY_3A_CFA                 = "cfa";
entryKey KEY_3A_GAMMA               = "gamma";
entryKey KEY_3A_RGB_to_RGB_1        = "rgb2rgb-1";
entryKey KEY_3A_RGB_to_RGB_2        = "rgb2rgb-2";
entryKey KEY_3A_RGB_to_YUV          = "rgb2yuv";
entryKey KEY_3A_GBCE                = "gbce";
entryKey KEY_3A_YUV_to_YUV          = "yuv2yuv";
entryKey KEY_3A_EE                  = "edge";
entryKey KEY_3A_CAR                 = "car";
entryKey KEY_3A_LSC                 = "lsc";
entryKey KEY_3A_HISTOGRAM           = "hist";
entryKey KEY_3A_BOXCAR              = "boxcar";
entryKey KEY_3A_BSC                 = "bsc";
entryKey KEY_3A_3DCC                = "3dcc";
entryKey KEY_3A_DPC_OTF             = "dpc-otf";
entryKey KEY_3A_DPC_LUT             = "dpc-lut";
entryKey KEY_3A_CGS                 = "cgs";
entryKey KEY_3A_DFS                 = "dfs";
entryKey KEY_3A_DPC_1               = "dpc-1";
entryKey KEY_3A_DPC_2               = "dpc-2";
entryKey KEY_3A_DPCM                = "dpcm";
entryKey KEY_3A_HLpf                = "h-lpf";
entryKey KEY_3A_VLpf                = "v-lpf";
entryKey KEY_3A_H3A_AEWB_PARAMS     = "h3a-wb";
entryKey KEY_3A_H3A_AF_PARAMS       = "h3a-af";
entryKey KEY_3A_H3A_COMMON_PARAMS   = "h3a-comm";
entryKey KEY_3A_EXP_GAIN_CONTROL    = "exp-ctrl";
entryKey KEY_3A_ZERO_NEXT           = "zero";
entryKey KEY_3A_QUIT                = "q";

//typedef const char * fileName_t;

//fileName_t FILE_3A_LSC_2D              = "";
//fileName_t FILE_3A_CLAMP               = "";
//fileName_t FILE_3A_GAIN_OFFSET         = "";
//fileName_t FILE_3A_VLDC                = "";
//fileName_t FILE_3A_NF1                 = "";
//fileName_t FILE_3A_NF2                 = "";
//fileName_t FILE_3A_GIC                 = "";
//fileName_t FILE_3A_WB                  = "";
//fileName_t FILE_3A_CFA                 = "";
//fileName_t FILE_3A_GAMMA               = "";
//fileName_t FILE_3A_RGB_to_RGB_1        = "";
//fileName_t FILE_3A_RGB_to_RGB_2        = "";
//fileName_t FILE_3A_RGB_to_YUV          = "";
//fileName_t FILE_3A_GBCE                = "";
//fileName_t FILE_3A_YUV_to_YUV          = "";
//fileName_t FILE_3A_EE                  = "";
//fileName_t FILE_3A_CAR                 = "";
//fileName_t FILE_3A_LSC                 = "";
//fileName_t FILE_3A_HISTOGRAM           = "";
//fileName_t FILE_3A_BOXCAR              = "";
//fileName_t FILE_3A_BSC                 = "";
//fileName_t FILE_3A_3DCC                = "";
//fileName_t FILE_3A_DPC_OTF             = "";
//fileName_t FILE_3A_DPC_LUT             = "";
//fileName_t FILE_3A_CGS                 = "";
//fileName_t FILE_3A_DFS                 = "";
//fileName_t FILE_3A_DPC_1               = "";
//fileName_t FILE_3A_DPC_2               = "";
//fileName_t FILE_3A_HLpf                = "";
//fileName_t FILE_3A_VLpf                = "";
//fileName_t FILE_3A_H3A_AEWB_PARAMS     = "";
//fileName_t FILE_3A_H3A_AF_PARAMS       = "";
//fileName_t FILE_3A_H3A_COMMON_PARAMS   = "";
//fileName_t FILE_3A_EXP_GAIN_CONTROL    = "";

bool_e getData(void *data, size_t size, const char *testFileName );
void getData(void*out, void* in, size_t size);

void man3AsensorFunc(void*);
void lsc2dFunc(void*);
void clampFunc(void*);
void gainOffsetFunc(void *);
void vldcFunc(void*);
void nf1Func(void *);
void nf2Func(void*);
void gicFunc(void *);
void wbFunc(void*);
void cfaFunc(void*);
void gammaFunc(void*);
void rgb2rgb1Func(void*);
void rgb2rgb2Func(void*);
void rgb2yuvFunc(void*);
void gbceFunc(void*);
void yuv2yuvFunc(void*);
void edgeEnhFunc(void*);
void carFunc(void*);
void lscFunc(void*);
void histogramFunc(void*);
void boxcarFunc(void*);
void bscFunc(void*);
void f3DccFunc(void*);
void dpcOtfFunc(void*);
void dpcLutFunc(void*);
void cgsFunc(void*);
void dfsFunc(void*);
void dpc1Func(void*);
void dpc2Func(void*);
void dpcmFunc(void *input);
void horLPFfunc(void*);
void verLPFfunc(void*);
void h3aAFfunc(void*);
void h3aAewbFunc(void*);
void h3aComFunc(void*);
void expGainCtrlFunc(void*);
void zeroAllStructs(void*);

static VisionCamSensorSelection eSensor = VCAM_SENSOR_PRIMARY;
static VisionCam_3A_2D_LscConfig_t lsc2DCfgLocal;
static VisionCam_3A_Clamp_t clampLocal;
static VisionCam_3A_GainOffset_t gainOffst;
static VisionCam_3A_VDLC_t vldcLocal;
static VisionCam_3A_NoiseFilterConfig_t nf1Local;
static VisionCam_3A_NoiseFilterConfig_t nf2Local;
static VisionCam_3A_GIC_Config_t gicLocal;
static VisionCam_3A_WB_Config_t wbLocal;
static VisionCam_3A_CFA_Config_t cfaLocal;
static VisionCam_3A_GammaTableConfig_t gammaLocal;
static VisionCam_3A_RGB2RGB_t rgb2rgb_1_Local;
static VisionCam_3A_RGB2RGB_t rgb2rgb_2_Local;
static VisionCam_3A_RGB2YUV_t rgb2yuvLocal;
static VisionCam_3A_GBCE_Config_t gbceLocal;
static VisionCam_3A_Yuv444_toYuv422_Config_t yuv2yuvLocal;
static VisionCam_3A_EdgeEnhancement_config_t edgeEnhLocal;
static VisionCam_3A_CAR_config_t carCfgLocal;
static VisionCam_3A_LscConfig_t lscLocal;
static VisionCam_3A_HistogramConfig_t histogramLocal;
static VisionCam_3A_BoxCarConfig_t boxCarLocal;
static VisionCam_3A_BscConfig_t bscCfgLocal;
static VisionCam_3A_3DCC_config_t m3DccLocal;
static VisionCam_3A_DpcOtfConfig_t dpcOtfLocal;
static VisionCam_3A_DPC_Lut_t dpcLutLocal;
static VisionCam_3A_CGS_Config_t cgsLocal;
static VisionCam_3A_DfsConfig_t dfsLocal;
static VisionCam_3A_DpcConfig_t dpc1Local;
static VisionCam_3A_DpcConfig_t dpc2Local;
static VisionCam_3A_DPCM_Config_t dpcmLocal;
static VisionCam_3A_ResizerLowPassFilter_t horizLPFlocal;
static VisionCam_3A_ResizerLowPassFilter_t vertLPFlocal;
static VisionCam_3A_H3A_AutoFocusParam_t h3aAFlocal;
static VisionCam_3A_AEWB_config_t h3aAEWB_Local;
static VisionCam_3A_H3A_CommonConfig_t h3aCommonLocal;
static VisionCam_3A_ExpGainControl_t expGainLocal;
static bool_e bZero = false_e;

const int man3AsensorSelOpts[] = {
    VCAM_SENSOR_PRIMARY,
    VCAM_SENSOR_SECONDARY,
    VCAM_SENSOR_STEREO,
};

const char * man3AsensorSelStrings[] = {
    "Primary Sensor",
    "Secondary Sensor",
    "Stereo Sensors"
};

const subMenuEnrty man3AsensorSelSubMenu = {
    man3AsensorSelOpts,
    man3AsensorSelStrings
};

const menuEnrty man3Asensor = {
    KEY_3A_SENSOR_SELECTION,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Sensor (on which configurations will be allpied).",
    { 0 , ARR_SIZE(man3AsensorSelOpts ) - 1, NULL },
    &eSensor,
    (void*)( (subMenuFn)man3AsensorFunc)
};

const menuEnrty lsc2D= {
    KEY_3A_LSC_2D,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "LSC 2d",
    { 0, 0, NULL },
    (void*)&lsc2DCfgLocal,
    (void*)( (subMenuFn)lsc2dFunc)
};

const menuEnrty clamp= {
    KEY_3A_CLAMP,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Clamp",
    { 0, 0, NULL },
    (void*)&clampLocal,
    (void*)( (subMenuFn)clampFunc)
};

const menuEnrty gainOffset = {
    KEY_3A_GAIN_OFFSET,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Gain Offset",
    { 0, 0, NULL },
    (void*)&gainOffst,
    (void*)( (subMenuFn)gainOffsetFunc)
};

const menuEnrty vldc = {
    KEY_3A_VLDC,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "VLDC",
    { 0, 0, NULL },
    (void*)&vldcLocal,
    (void*)( (subMenuFn)vldcFunc)
};

const menuEnrty nf1 = {
    KEY_3A_NF1,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Noise Filter 1",
    { 0, 0, NULL },
    (void*)&nf1Local,
    (void*)( (subMenuFn)nf1Func)
};

const menuEnrty nf2 = {
    KEY_3A_NF2,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Noise Filter 2",
    { 0, 0, NULL },
    (void*)&nf2Local,
    (void*)( (subMenuFn)nf2Func)
};

const menuEnrty gic = {
    KEY_3A_GIC,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Gic",
    { 0, 0, NULL },
    (void*)&gicLocal,
    (void*)( (subMenuFn)gicFunc)
};

const menuEnrty wb = {
    KEY_3A_WB,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "White Balance",
    { 0, 0, NULL },
    (void*)&wbLocal,
    (void*)( (subMenuFn)wbFunc)
};

const int cfaOpts[] = {
    VCAM_IPIPE_CFA_MODE_2DIR,
    VCAM_IPIPE_CFA_MODE_2DIR_DA,
    VCAM_IPIPE_CFA_MODE_DAA
};

const char * cfaStrings[] = {
    "2DIR mode",
    "2DIR \"DA\" mode",
    "DAA mode"
};

const subMenuEnrty cfaSubMenu = {
    cfaOpts,
    cfaStrings
};

const menuEnrty cfa = {
    KEY_3A_CFA,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "CFA",
    { 0, ARR_SIZE(cfaOpts), NULL },
    (void*)&cfaLocal,
    (void*)( (subMenuFn)cfaFunc)
};

const menuEnrty manualGamma = {
    KEY_3A_GAMMA,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Gamma Table",
    { 0, 0, NULL },
    (void*)&gammaLocal,
    (void*)( (subMenuFn)gammaFunc)
};

const menuEnrty rgb2rgb_1 = {
    KEY_3A_RGB_to_RGB_1,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "RGB 2 RGB matrix 1",
    { 0, 0, NULL },
    (void*)&rgb2rgb_1_Local,
    (void*)( (subMenuFn)rgb2rgb1Func)
};

const menuEnrty rgb2rgb_2 = {
    KEY_3A_RGB_to_RGB_2,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "RGB 2 RGB matrix 2",
    { 0, 0, NULL },
    (void*)&rgb2rgb_2_Local,
    (void*)( (subMenuFn)rgb2rgb2Func)
};

const menuEnrty rgb2yuv = {
    KEY_3A_RGB_to_YUV,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "RGB 2 YUV",
    { 0, 0, NULL },
    (void*)&rgb2yuvLocal,
    (void*)( (subMenuFn)rgb2yuvFunc)
};

const menuEnrty gbce = {
    KEY_3A_GBCE,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "GBCE",
    { 0, 0, NULL },
    (void*)&gbceLocal,
    (void*)( (subMenuFn)gbceFunc)
};

const menuEnrty yuv2yuv = {
    KEY_3A_YUV_to_YUV,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "yuv 2 yuv matrix",
    { 0, 0, NULL },
    (void*)&yuv2yuvLocal,
    (void*)( (subMenuFn)yuv2yuvFunc)
};

const menuEnrty edgeEnh = {
    KEY_3A_EE,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Edge Enhancement Method",
    { 0, 0, NULL },
    (void *)&edgeEnhLocal,
    (void*)( (subMenuFn)edgeEnhFunc)
};

const menuEnrty car = {
    KEY_3A_CAR,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Car",
    { 0, 0, NULL },
    (void*)&carCfgLocal,
    (void*)( (subMenuFn)carFunc)
};

const menuEnrty lsc = {
    KEY_3A_LSC,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Lens Shading Correction",
    { 0, 0, NULL },
    (void*)&lscLocal,
    (void*)( (subMenuFn)lscFunc)
};

const menuEnrty histogram = {
    KEY_3A_HISTOGRAM,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Histogram",
    { 0, 0, NULL },
    (void*)&histogramLocal,
    (void*)( (subMenuFn)histogramFunc)
};

const menuEnrty boxCar = {
    KEY_3A_BOXCAR,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Boxcar",
    { 0, 0, NULL },
    (void*)&boxCarLocal,
    (void*)( (subMenuFn)boxcarFunc)
};

const menuEnrty bsc = {
    KEY_3A_BSC,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Bsc",
    { 0, 0, NULL },
    (void*)&bscCfgLocal,
    (void*)( (subMenuFn)bscFunc)
};

const menuEnrty m3Dcc = {
    KEY_3A_3DCC,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "3D cc",
    { 0, 0, NULL },
    (void*)&m3DccLocal,
    (void*)( (subMenuFn)f3DccFunc)
};

const menuEnrty dpcOtf = {
    KEY_3A_DPC_OTF,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "DPC Otf",
    { 0, 0, NULL },
    (void*)&dpcOtfLocal,
    (void*)( (subMenuFn)dpcOtfFunc)
};

const menuEnrty dpcLut = {
    KEY_3A_DPC_LUT,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "DPC Lut",
    { 0, 0, NULL },
    (void*)&dpcLutLocal,
    (void*)( (subMenuFn)dpcLutFunc)
};

const menuEnrty cgs = {
    KEY_3A_CGS,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "CGS",
    { 0, 0, NULL },
    (void*)&cgsLocal,
    (void*)( (subMenuFn)cgsFunc)
};

const menuEnrty dfs = {
    KEY_3A_DFS,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "DFS",
    { 0, 0, NULL },
    (void*)&dfsLocal,
    (void*)( (subMenuFn)dfsFunc)
};

const menuEnrty dpc1 = {
    KEY_3A_DPC_1,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "DPC 1",
    { 0, 0, NULL },
    (void*)&dpc1Local,
    (void*)( (subMenuFn)dpc1Func)
};

const menuEnrty dpc2 = {
    KEY_3A_DPC_2,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "DPC 2",
    { 0, 0, NULL },
    (void*)&dpc2Local,
    (void*)( (subMenuFn)dpc2Func)
};

const menuEnrty dpcm = {
    KEY_3A_DPCM,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "DPCM",
    { 0, 0, NULL },
    (void*)&dpcmLocal,
    (void*)( (subMenuFn)dpcmFunc)
};

const menuEnrty horLPF = {
    KEY_3A_HLpf,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Horiz Low Pass Filter",
    { 0, 0, NULL },
    (void*)&horizLPFlocal,
    (void*)( (subMenuFn)horLPFfunc)
};

const menuEnrty verLPF = {
    KEY_3A_VLpf,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Vertical Low Pass Filter",
    { 0, 0, NULL },
    (void*)&vertLPFlocal,
    (void*)( (subMenuFn)verLPFfunc)
};

const menuEnrty h3aAF = {
    KEY_3A_H3A_AF_PARAMS,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "H3a AF params",
    { 0, 0, NULL },
    (void*)&h3aAFlocal,
    (void*)( (subMenuFn)h3aAFfunc)
};

const menuEnrty h3aAewb = {
    KEY_3A_H3A_AEWB_PARAMS,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "H3a AEWB params",
    { 0, 0, NULL },
    (void*)&h3aAEWB_Local,
    (void*)( (subMenuFn)h3aAewbFunc)
};

const menuEnrty h3aCom = {
    KEY_3A_H3A_COMMON_PARAMS,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "H3a common params",
    { 0, 0, NULL },
    (void*)&h3aCommonLocal,
    (void*)( (subMenuFn)h3aComFunc)
};

const menuEnrty expGainCtrl = {
    KEY_3A_EXP_GAIN_CONTROL,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Exposure & Gain Control",
    { 0, 0, NULL },
    (void*)&expGainLocal,
    (void*)( (subMenuFn)expGainCtrlFunc)
};

const menuEnrty zeroStructs = {
    KEY_3A_ZERO_NEXT,
    functionalEntry,
    VCAM_PARAM_EXPORTED_3A_SET,
    "Zero/Unzero Next Values",
    { 0, 0, NULL },
    (void*)&bZero,
    (void*)( (subMenuFn)zeroAllStructs)
};

/// Used to exit Manual 3A submenu and return to main test app menu.
const menuEnrty quit = {
    KEY_3A_QUIT,
    neutralEntry,
    VCAM_CMD_QUIT,
    "Quit.",
    { 0 , 0, NULL },
    NULL,
    NULL
};

const menuEnrty * MenuManual3A[] = {
    &man3Asensor,
    &lsc2D,
    &clamp,
    &gainOffset,
    &vldc,
    &nf1,
    &nf2,
    &gic,
    &wb,
    &cfa,
    &manualGamma,
    &rgb2rgb_1,
    &rgb2rgb_2,
    &rgb2yuv,
    &gbce,
    &yuv2yuv,
    &edgeEnh,
    &car,
    &lsc,
    &histogram,
    &boxCar,
    &bsc,
    &m3Dcc,
    &dpcOtf,
    &dpcLut,
    &cgs,
    &dfs,
    &dpc1,
    &dpc2,
    &dpcm,
    &horLPF,
    &verLPF,
    &h3aAF,
    &h3aAewb,
    &h3aCom,
    &expGainCtrl,
    &zeroStructs,

    &quit,
    NULL
};

#endif // _VISION_CAM_TEST_3A_DATA_
