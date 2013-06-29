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

#include <dvp/VisionCam_3A_ExportTypes.h>

static VCAM_3ASKIP_IPIPE_3DCC_CFGTYPE t3dccDump = {
        /*.enable = */0,
        /*.pInBiffData = */NULL
};

static VCAM_3ASKIP_IPIPE_BOXCAR_CFGTYPE boxCarDump = {
    /*.nEnable = */0,
    /*.nOst = */1,
    /*.nBoxSize = */1,
    /*.nShift = */1,
    /*.pAddr = */0
};

static VCAM_3ASKIP_IPIPE_BSC_CFGTYPE bscDump = {
    /*.nEnable = */0,
    /*.nMode = */1,
    /*.nColSample = */1,
    /*.nRowSample = */1,
    /*.nElement = */0,
    {
    /*.nColPos.nVectors = */    8,
    /*.nColPos.nShift = */      8,
    /*.nColPos.nVPos = */       8,
    /*.nColPos.nHPos = */       8,
    /*.nColPos.nVNum = */       8,
    /*.nColPos.nHNum = */       8,
    /*.nColPos.nVSkip = */      8,
    /*.nColPos.nHSkip = */      8
    },
    {
    /*.nRowPos.nVectors = */    8,
    /*.nRowPos.nShift = */      8,
    /*.nRowPos.nVPos = */       8,
    /*.nRowPos.nHPos = */       8,
    /*.nRowPos.nVNum = */       8,
    /*.nRowPos.nHNum = */       8,
    /*.nRowPos.nVSkip = */      8,
    /*.nRowPos.nHSkip = */      8
    }
};

static VCAM_3ASKIP_IPIPE_CAR_CFGTYPE carDump = {
    /*.nEnable = */     0,
    /*.nTyp = */        1,
    /*.nSw0Thr = */     1,
    /*.nSw1Thr = */     1,
    /*.nHpfType = */    1,
    /*.nHpfShift = */   1,
    /*.nHpfThr = */     1,
    /*.nGn1Gain = */    1,
    /*.nGn1Shift = */   1,
    /*.nGn1Min = */     1,
    /*.nGn2Gain = */    1,
    /*.nGn2Shift = */   1,
    /*.nGn2Min = */     1
};

static VCAM_3ASKIP_IPIPE_CFA_CFGTYPE cfaDump = {
    /*.nEnable = */0,
    /*.eMode = */(VCAM_3ASKIP_IPIPE_CFA_MODETYPE)1,
    {
    /*.tDir.nHpfThr = */    16,
    /*.tDir.nHpfSlope = */  16,
    /*.tDir.nMixThr = */    16,
    /*.tDir.nMixSlope = */  16,
    /*.tDir.nDirThr = */    16,
    /*.tDir.nDirSlope = */  16,
    /*.tDir.nDirNdwt = */   16
    },
    {
    /*.tDaa.nMonoHueFra = */    8,
    /*.tDaa.nMonoEdgThr = */    8,
    /*.tDaa.nMonoThrMin = */    8,
    /*.tDaa.nMonoThrSlope = */  8,
    /*.tDaa.nMonoSlpMin = */    8,
    /*.tDaa.nMonoSlpSlp = */    8,
    /*.tDaa.nMonoLpwt = */      8
    }
};

static VCAM_3ASKIP_IPIPE_CGS_CFGTYPE cgsDump = {
    /*.enable = */0,
    {
    /*.y_chroma_low.thr = */    8,
    /*.y_chroma_low.gain = */   16,
    /*.y_chroma_low.shift = */  16,
    /*.y_chroma_low.min = */    16
    },
    {
    /*.y_chroma_high.thr = */   8,
    /*.y_chroma_high.gain = */  16,
    /*.y_chroma_high.shift = */ 16,
    /*.y_chroma_high.min = */   16
    },
    {
    /*.c_chroma.thr = */    8,
    /*.c_chroma.gain = */   16,
    /*.c_chroma.shift = */  16,
    /*.c_chroma.min = */    16
    }
};

static VCAM_3ASKIP_ISIF_CLAMP_CFGTYPE clampDump = {
    /*.nEnable = */0,
    /*.eHClampMode = */(VCAM_3ASKIP_ISIF_HORIZONTAL_CLAMP_MODETYPE)1,
    /*.eBlackClampMode = */(VCAM_3ASKIP_ISIF_BLACK_CLAMP_MODETYPE)1,
    /*.nDCOffsetClampVal = */   128,
    /*.nBlackClampVStartPos = */128,
    {
    /*.tHorizontalBlack.nVPos = */  64,
    /*.tHorizontalBlack.eVSize = */ 64,
    /*.tHorizontalBlack.nHPos = */  64,
    /*.tHorizontalBlack.eHSize = */(VCAM_3ASKIP_ISIF_VBLACK_PIXEL_WIDTHTYPE)0,
    /*.tHorizontalBlack.line_avg_coef = */64,
    /*.tHorizontalBlack.reset_mode = */(VCAM_3ASKIP_ISIF_VBLACK_RESET_MODETYPE)2,
    /*.tHorizontalBlack.reset_value = */2
    },
    {
    /*.tVerticalBlack.nVPos = */    64,
    /*.tVerticalBlack.nVSize = */   64,
    /*.tVerticalBlack.nHPos = */    64,
    /*.tVerticalBlack.HSize = */(VCAM_3ASKIP_ISIF_VBLACK_PIXEL_WIDTHTYPE)0,
    /*.tVerticalBlack.line_avg_coef = */64,
    /*.tVerticalBlack.reset_mode = */(VCAM_3ASKIP_ISIF_VBLACK_RESET_MODETYPE)2,
    /*.tVerticalBlack.reset_value = */2
    }
};

static VCAM_3ASKIP_IPIPEIF_DFS_CFGTYPE dfsDump = {
    /*.eDfsGainEn = */(VCAM_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE)1,
    /*.nDfsGainVal = */ 128,
    /*.nDfsGainThr = */ 64,
    /*.nOclip = */      16,
    /*.nDfsDir = */     16
};

static VCAM_3ASKIP_IPIPEIF_DPC_CFGTYPE dpc1Dump = {
    /*.eDpcEn = */(VCAM_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE)1,
    /*.eDpcThr = */1
};

static VCAM_3ASKIP_IPIPEIF_DPC_CFGTYPE dpc2Dump = {
    /*.eDpcEn = */(VCAM_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE)1,
    /*.eDpcThr = */1
};

static VCAM_3ASKIP_IPIPEIF_DPCM_CFGTYPE dpcmDump = {
    /*.nDpcmBitSize = */    (VCAM_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE)1,
    /*.nDpcmEn = */         (VCAM_3ASKIP_IPIPEIF_DPCM_PRED_TYPTYPE)1,
    /*.nDpcmPredictor = */  (VCAM_3ASKIP_IPIPEIF_DPCM_BIT_SIZETYPE)1
};

static VCAM_3ASKIP_IPIPE_DPCLUT_CFGTYPE dpcLutDump = {
    /*.nEnable = */0,
    /*.eTableType = */(VCAM_3ASKIP_IPIPE_DPC_LUT_TBL_SIZETYPE)1,
    /*.eReplaceType = */(VCAM_3ASKIP_IPIPE_DPC_LUT_REPLACEMENTTYPE)1,
    /*.nLutValidAddr = */0,
    /*.nLutSize = */256,
    /*.nLutTable0Addr = */0,
    /*.nLutTable1Addr = */0
};


//VCAM_3ASKIP_IPIPEIF_DPCM_CFGTYPE ptDpcm {
//OMX_TI_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE nDpcmEn
//OMX_TI_3ASKIP_IPIPEIF_DPCM_PRED_TYPTYPE nDpcmPredictor
//OMX_TI_3ASKIP_IPIPEIF_DPCM_BIT_SIZETYPE nDpcmBitSize
//};

static VCAM_3ASKIP_IPIPE_DPCOTF_CFGTYPE dpcOtfDump;

static VCAM_3ASKIP_ISIF_GAINOFFSET_CFGTYPE gainOffsetDump = {
        /*.gain_offset_featureflag = */256,
        /*.gain_r = */  128,
        /*.gain_gr = */ 128,
        /*.gain_gb = */ 128,
        /*.gain_bg = */ 128,
        /*.offset = */  128
};


static VCAM_3ASKIP_IPIPE_GIC_CFGTYPE gicDump = {
    /*.nEnable = */0,
    /*.eLscGain = */(VCAM_3ASKIP_IPIPE_GIC_LSC_GAINTYPE)1,
    /*.eSel = */(VCAM_3ASKIP_IPIPE_GIC_SELTYPE)1,
    /*.eTyp = */(VCAM_3ASKIP_IPIPE_GIC_INDEXTYPE)1,
    /*.nGicGain = */1,
    /*.nGicNfGain = */1,
    /*.nGicThr = */1,
    /*.nGicSlope = */1
};

static VCAM_3ASKIP_H3A_AEWB_PARAMTYPE h3aAewbDump = {
    /*.eAewbEnable = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)0,
    /*.ptAewbPaxelWin = */0,
    /*.ptBlkWinDims = */0,
    /*.eAeOpFmt = */(VCAM_3ASKIP_H3A_AEWB_OP_FMTTYPE)1,
    /*.nShiftValue = */128,
    /*.nSaturationLimit = */50,
    /*.nAewbOpAddr = */0
};

static VCAM_3ASKIP_H3A_COMMON_CFGTYPE h3aCommonDump = {
    /*.eAfMedianEn = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)  1,
    /*.eAewbMedianEn = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)1,
    /*.nMedianFilterThreshold = */8,
    /*.eAfAlawEn = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)1,
    /*.eAewbAlawEn = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)1,
    /*.eIpipeifAveFiltEn = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)1,
    /*.eH3aDecimEnable = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)1
    /*.nReserved = */ // 0
};

static VCAM_3ASKIP_IPIPE_HIST_CFGTYPE histDump = {
    /*.nEnable = */     0,
    /*.nOst = */        8,
    /*.nSel = */        8,
    /*.nType = */       8,
    /*.nBins = */       8,
    /*.nShift = */      8,
    /*.nCol = */        8,
    /*.nRegions = */    8,
    /*.ptHistDim = */   8,
    /*.nClearTable = */ 8,
    /*.nTableSel = */   8,
    /*.pnGainTbl = */   8
};

static VCAM_3ASKIP_RSZ_LPF_CFGTYPE hlpfDump = {
    /*.nCIntensity = */1,
    /*.nYIntensity = */1
};

static VCAM_3ASKIP_ISIF_2DLSC_CFGTYPE lsc2dDump = {
    /*.nEnable = */0,
    /*.nLscHSize = */8,
    /*.nLscVSize = */8,
    /*.nHDirDataOffset = */16,
    /*.nVDirDataOffset = */16,
    /*.nHPosInPaxel = */16,
    /*.nVPosInPaxel = */16,
    /*.ePaxHeight = */(VCAM_3ASKIP_ISIF_LSC_GAIN_MODE_MNTYPE)1,
    /*.ePaxLength = */(VCAM_3ASKIP_ISIF_LSC_GAIN_MODE_MNTYPE)1,
    /*.eGainFormat = */(VCAM_3ASKIP_ISIF_LSC_GAIN_FORMATTYPE)1,
    /*.nOffsetScalingFactor = */16,
    /*.eOffsetShiftVal = */(VCAM_3ASKIP_ISIF_LSC_OFFSET_SHIFTTYPE)1,
    /*.eOffsetEnable = */(VCAM_3ASKIP_ISIF_LSC_OFFSET_ENABLETYPE)1,
    /*.nGainTableAddress = */   16, //4294967040
    /*.nGainTableLength = */    16,
    /*.nOffsetTableAddress = */ 16, //4294967040
    /*.nOffsetTableLength = */  16
};

static VCAM_3ASKIP_IPIPE_LSC_CFGTYPE lscDump = {
    /*.nVOffset = */            8,
    /*.nVLinearCoeff = */       8,
    /*.nVQuadraticCoeff = */    8,
    /*.nVLinearShift = */       8,
    /*.nVQuadraticShift = */    8,
    /*.nHOffset = */            8,
    /*.nHLinearCoeff = */       8,
    /*.nHQuadraticCoeff = */    8,
    /*.nHLinearShift = */       8,
    /*.nHQuadraticShift = */    8,
    /*.nGainR = */              8,
    /*.nGainGR = */             8,
    /*.nGainGB = */             8,
    /*.nGainB = */              8,
    /*.nOffR = */               8,
    /*.nOffGR = */              8,
    /*.nOffGB = */              8,
    /*.nOffB = */               8,
    /*.nShift = */              8,
    /*.nMax = */                8
};

static VCAM_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE nf1Dump = {
    /*.nEnable = */0,
    /*.eNFNum = */(VCAM_3ASKIP_IPIPE_NOISE_FILTERTYPE)1,
    /*.eSel = */(VCAM_3ASKIP_IPIPE_NF_SELTYPE)1,
    /*.eLscGain = */(VCAM_3ASKIP_IPIPE_NF_LSC_GAINTYPE)1,
    /*.eTyp = */(VCAM_3ASKIP_IPIPE_NF_SAMPLE_METHODTYPE)1,
    /*.nDownShiftVal = */1,
    /*.nSpread = */1,

    /*.pnThr = */
    {
                0x000F,
                0x000F,
                0x000F,
                0x000F,
                0x000F,
                0x000F,
                0x000F,
                0x000F
    },

    /*.pnStr = */
    {
                0x0F,
                0x0F,
                0x0F,
                0x0F,
                0x0F,
                0x0F,
                0x0F,
                0x0F
    },

    /*.pnSpr = */
    {
                0x0F,
                0x0F,
                0x0F,
                0x0F,
                0x0F,
                0x0F,
                0x0F,
                0x0F
    },
    /*.nEdgeMin = */15,
    /*.nEdgeMax = */15
};

static VCAM_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE nf2Dump = {
    /*.nEnable = */0,
    /*.eNFNum = */(VCAM_3ASKIP_IPIPE_NOISE_FILTERTYPE)1,
    /*.eSel = */(VCAM_3ASKIP_IPIPE_NF_SELTYPE)1,
    /*.eLscGain = */(VCAM_3ASKIP_IPIPE_NF_LSC_GAINTYPE)1,
    /*.eTyp = */(VCAM_3ASKIP_IPIPE_NF_SAMPLE_METHODTYPE)1,
    /*.nDownShiftVal = */1,
    /*.nSpread = */1,

    /*.pnThr = */
    {
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F
    },

    /*.pnStr = */
    {
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x0F
    },

    /*.pnSpr = */
    {
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x0F,
    },

    /*.nEdgeMin = */15,
    /*.nEdgeMax = */15
};

static VCAM_3ASKIP_IPIPE_RGBRGB_CFGTYPE rgb2rgb1Dump = {
    /*.pnMulOff = */
    {
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F
    },
    /*.pnOft = */
    {
        0x000F,
        0x000F,
        0x000F
    }
};


static VCAM_3ASKIP_IPIPE_RGBRGB_CFGTYPE rgb2rgb2Dump = {
    /*.pnMulOff = */
    {
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F
    },
    /*.pnOft = */
    {
        0x000F,
        0x000F,
        0x000F
    }
};

static VCAM_3ASKIP_IPIPE_RGBYUV_CFGTYPE rgb2yuvDump = {
    /*.nBrightness = */50,
    /*.nContrast = */50,
    /*.pnMulVal = */
    {
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F,
        0x000F
    },
    /*.pnOffset = */
    {
        0x000F,
        0x000F,
        0x000F
    },
};

static VCAM_3ASKIP_ISIF_VLDC_CFGTYPE vldcDump = {
    /*.nEnable = */0,
    /*.eDisableVldcUpperPixels = */(VCAM_3ASKIP_ISIF_VDLC_PIXEL_DEPENDENCYTYPE)1,
    /*.nVldcShiftVal = */64,
    /*.eVldcModeSelect = */(VCAM_3ASKIP_ISIF_VLDC_MODE_SELECTTYPE)1,
    /*.nVldcSaturationLvl = */64,
    /*.nDefectLines = */4,
    {
        /*.tVldcDefectLineParams.nVerticalDefectPosition = */   8,
        /*.tVldcDefectLineParams.nHorizontalDefectPosition = */ 8,
        /*.tVldcDefectLineParams.nSub1ValueVldc = */            8,
        /*.tVldcDefectLineParams.nSub2LessThanVldc = */         8,
        /*.tVldcDefectLineParams.nSub3GreaterThanVldc = */      8
    }
};

static VCAM_3ASKIP_RSZ_LPF_CFGTYPE vlpDump = {
    /*.nCIntensity = */1,
    /*.nYIntensity = */1
};

static VCAM_3ASKIP_IPIPE_WB_CFGTYPE wbDump = {
    /*.pnOffset = */
    {
        0x0200,
        0x0200,
        0x0400,
        0x0400
    },
    /*.pnGain = */
    {
        0x0200,
        0x0200,
        0x0400,
        0x0400
    }
};

static VCAM_3ASKIP_IPIPE_YUV444YUV422_CFGTYPE yuv2yuvDump = {
    /*.ePos = */(VCAM_3ASKIP_IPIPE_YUV_PHASE_POSTYPE)1,
    /*.nLpfEn = */15
};



static VCAM_3ASKIP_H3A_AF_PARAMTYPE h3aAfDump = {
        /*eAfEnable = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)1,
        /*eRgbPos = */(VCAM_3ASKIP_H3A_RGB_POSTYPE)1,
        /*ePeakModeEn = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)1,
        /*eVerticalFocusEn = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)1,
        /*nIirStartPos = */1,
        /*ptAfPaxelWin = */
    {
        0 ,0, 0, 0, 0, 0, 0, 0
    },
        /*ptIir1 = */ {0, 0},
        /*ptIir2 = */{0, 0},
        /*ptFir1 = */{0, 0},
        /*ptFir2 = */{0, 0},
        /*nAfOpAddr = */0
};


static VCAM_3ASKIP_CAM_CONTROL_EXPGAINTYPE expCtrlDump = {
    /*exp =*/ 8,
    /*a_gain = */8,
    /*mask = */8,
    /*nAgainErr = */8,
    /*nDigitalISPGain = */8
};

static VCAM_3ASKIP_IPIPE_EE_CFGTYPE edgeEnhDump = {
    /*nEnable = */0,
    /*eHaloReduction = */(VCAM_3ASKIP_IPIPE_EE_HALO_CTRLTYPE)0,
    /*pnMulVal =*/
    {
        0x0008,
        0x0008,
        0x0008,
        0x0008,
        0x0008,
        0x0008,
        0x0008,
        0x0008,
        0x0008
    },
    /*nSel = */1,
    /*nShiftHp = */1,
    /*nThreshold = */4,
    /*nGain = */8,
    /*nHpfLowThr = */8,
    /*nHpfHighThr = */8,
    /*nHpfGradientGain = */8,
    /*nHpfgradientOffset = */8,
    /*pnEeTable =*/
    {
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
           128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    77
    }
};

static VCAM_3ASKIP_IPIPE_GAMMA_CFGTYPE gammaDump = {
    /*nEnable = */0,
    /*eGammaTblSize = */(VCAM_3ASKIP_IPIPE_GAMMA_TABLE_SIZETYPE)256,
    /*nTbl = */256,
    /*eBypassB = */(VCAM_3ASKIP_IPIPE_GAMMA_BYPASSTYPE)1,
    /*eBypassG = */(VCAM_3ASKIP_IPIPE_GAMMA_BYPASSTYPE)1,
    /*eBypassR = */(VCAM_3ASKIP_IPIPE_GAMMA_BYPASSTYPE)1,
    /*pnRedTable =*/
    {
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    77
//         -20,    2,    0,    0,  -20,    2,    0,    0,  -20,    2,    4,    0,  -16,    2,    0,    0,
//         -16,    2,    0,    0,  -16,    2,    4,    0,  -12,    2,    0,    0,  -12,    2,    4,    0,
//          -8,    2,    0,    0,   -8,    2,    0,    0,   -8,    2,    0,    0,   -8,    2,    4,    0,
//          -4,    2,    0,    0,   -4,    2,    4,    0,    0,    3,    0,    0,    0,    3,    0,    0,
//           0,    3,    4,    0,    4,    3,    0,    0,    4,    3,    0,    0,    4,    3,    0,    0,
//           4,    3,    4,    0,    8,    3,    0,    0,    8,    3,    0,    0,    8,    3,    0,    0,
//           8,    3,    4,    0,   12,    3,    0,    0,   12,    3,    4,    0,   16,    3,    0,    0,
//          16,    3,    0,    0,   16,    3,    4,    0,   20,    3,    0,    0,   20,    3,    0,    0,
//          20,    3,    4,    0,   24,    3,    0,    0,   24,    3,    0,    0,   24,    3,    0,    0,
//          24,    3,    4,    0,   28,    3,    0,    0,   28,    3,    0,    0,   28,    3,    0,    0,
//          28,    3,    4,    0,   32,    3,    0,    0,   32,    3,    4,    0,   36,    3,    0,    0,
//          36,    3,    0,    0,   36,    3,    4,    0,   40,    3,    0,    0,   40,    3,    0,    0,
//          40,    3,    0,    0,   40,    3,    4,    0,   44,    3,    0,    0,   44,    3,    0,    0,
//          44,    3,    4,    0,   48,    3,    0,    0,   48,    3,    0,    0,   48,    3,    0,    0,
//          48,    3,    4,    0,   52,    3,    0,    0,   52,    3,    0,    0,   52,    3,    0,    0,
//          52,    3,    4,    0,   56,    3,    0,    0,   56,    3,    0,    0,   56,    3,    4,    0,
//          60,    3,    0,    0,   60,    3,    0,    0,   60,    3,    4,    0,   64,    3,    0,    0,
//          64,    3,    4,    0,   68,    3,    0,    0,   68,    3,    0,    0,   68,    3,    0,    0,
//          68,    3,    4,    0,   72,    3,    0,    0,   72,    3,    0,    0,   72,    3,    0,    0,
//          72,    3,    4,    0,   76,    3,    0,    0,   76,    3,    0,    0,   76,    3,    4,    0,
//          80,    3,    0,    0,   80,    3,    0,    0,   80,    3,    0,    0,   80,    3,    4,    0,
//          84,    3,    0,    0,   84,    3,    0,    0,   84,    3,    0,    0,   84,    3,    4,    0,
//          88,    3,    0,    0,   88,    3,    0,    0,   88,    3,    4,    0,   92,    3,    0,    0,
//          92,    3,    0,    0,   92,    3,    0,    0,   92,    3,    4,    0,   96,    3,    0,    0,
//          96,    3,    0,    0,   96,    3,    0,    0,   96,    3,    4,    0,  100,    3,    0,    0,
//         100,    3,    0,    0,  100,    3,    4,    0,  104,    3,    0,    0,  104,    3,    0,    0,
//         104,    3,    0,    0,  104,    3,    4,    0,  108,    3,    0,    0,  108,    3,    0,    0,
//         108,    3,    4,    0,  112,    3,    0,    0,  112,    3,    0,    0,  112,    3,    0,    0,
//         112,    3,    4,    0,  116,    3,    0,    0,  116,    3,    0,    0,  116,    3,    0,    0,
//         116,    3,    0,    0,  116,    3,    4,    0,  120,    3,    0,    0,  120,    3,    0,    0,
//         120,    3,    0,    0,  120,    3,    4,    0,  124,    3,    0,    0,  124,    3,    0,    0,
//         124,    3,    0,    0,  124,    3,    4,    0, -128,    3,    0,    0, -128,    3,    0,    0,
//        -128,    3,    4,    0, -124,    3,    0,    0, -124,    3,    0,    0, -124,    3,    4,    0,
//        -120,    3,    0,    0, -120,    3,    0,    0, -120,    3,    0,    0, -120,    3,    4,    0,
//        -116,    3,    0,    0, -116,    3,    0,    0, -116,    3,    4,    0, -112,    3,    0,    0,
//        -112,    3,    0,    0, -112,    3,    0,    0, -112,    3,    4,    0, -108,    3,    0,    0,
//        -108,    3,    0,    0, -108,    3,    0,    0, -108,    3,    0,    0, -108,    3,    4,    0,
//        -104,    3,    0,    0, -104,    3,    0,    0, -104,    3,    4,    0, -100,    3,    0,    0,
//        -100,    3,    0,    0, -100,    3,    4,    0,  -96,    3,    0,    0,  -96,    3,    0,    0,
//         -96,    3,    0,    0,  -96,    3,    4,    0,  -92,    3,    0,    0,  -92,    3,    0,    0,
//         -92,    3,    0,    0,  -92,    3,    0,    0,  -92,    3,    4,    0,  -88,    3,    0,    0,
//         -88,    3,    0,    0,  -88,    3,    4,    0,  -84,    3,    0,    0,  -84,    3,    0,    0,
//         -84,    3,    0,    0,  -84,    3,    4,    0,  -80,    3,    0,    0,  -80,    3,    0,    0,
//         -80,    3,    0,    0,  -80,    3,    0,    0,  -80,    3,    4,    0,  -76,    3,    0,    0,
//         -76,    3,    0,    0,  -76,    3,    4,    0,  -72,    3,    0,    0,  -72,    3,    0,    0,
//         -72,    3,    0,    0,  -72,    3,    0,    0,  -72,    3,    4,    0,  -68,    3,    0,    0,
//         -68,    3,    0,    0,  -68,    3,    0,    0,  -68,    3,    4,    0,  -64,    3,    0,    0,
//         -64,    3,    0,    0,  -64,    3,    4,    0,  -60,    3,    0,    0,  -60,    3,    0,    0,
//         -60,    3,    0,    0,  -60,    3,    0,    0,  -60,    3,    4,    0,  -56,    3,    0,    0,
//         -56,    3,    0,    0,  -56,    3,    0,    0,  -56,    3,    4,    0,  -52,    3,    0,    0,
//         -52,    3,    0,    0,  -52,    3,    0,    0,  -52,    3,    4,    0,  -48,    3,    0,    0,
//         -48,    3,    0,    0,  -48,    3,    0,    0,  -48,    3,    4,    0,  -44,    3,    0,    0,
//         -44,    3,    0,    0,  -44,    3,    0,    0,  -44,    3,    4,    0,  -40,    3,    0,    0,
//         -40,    3,    0,    0,  -40,    3,    0,    0,  -40,    3,    4,    0,  -36,    3,    0,    0,
//         -36,    3,    0,    0,  -36,    3,    0,    0,  -36,    3,    4,    0,  -32,    3,    0,    0,
//         -32,    3,    0,    0,  -32,    3,    0,    0,  -32,    3,    0,    0,  -32,    3,    4,    0,
//         -28,    3,    0,    0,  -28,    3,    0,    0,  -28,    3,    0,    0,  -28,    3,    0,    0,
//         -28,    3,    4,    0,  -24,    3,    0,    0,  -24,    3,    0,    0,  -24,    3,    4,    0,
//         -20,    3,    0,    0,  -20,    3,    0,    0,  -20,    3,    0,    0,  -20,    3,    0,    0,
//         -20,    3,    4,    0,  -16,    3,    0,    0,  -16,    3,    0,    0,  -16,    3,    0,    0,
//         -16,    3,    4,    0,  -12,    3,    0,    0,  -12,    3,    0,    0,  -12,    3,    0,    0,
//         -12,    3,    4,    0,   -8,    3,    0,    0,   -8,    3,    0,    0,   -8,    3,    0,    0,
//          -8,    3,    0,    0,   -8,    3,    4,    0,   -4,    3,    0,    0,   -4,    3,    0,    0,
//          -4,    3,    0,    0,   -4,    3,    0,    0,   -4,    3,    0,    0,   -4,    3,    0,    0
    },

    /*pnBlueTable =*/
    {
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    77//,
//         -20,    2,    0,    0,  -20,    2,    0,    0,  -20,    2,    4,    0,  -16,    2,    0,    0,
//         -16,    2,    0,    0,  -16,    2,    4,    0,  -12,    2,    0,    0,  -12,    2,    4,    0,
//          -8,    2,    0,    0,   -8,    2,    0,    0,   -8,    2,    0,    0,   -8,    2,    4,    0,
//          -4,    2,    0,    0,   -4,    2,    4,    0,    0,    3,    0,    0,    0,    3,    0,    0,
//           0,    3,    4,    0,    4,    3,    0,    0,    4,    3,    0,    0,    4,    3,    0,    0,
//           4,    3,    4,    0,    8,    3,    0,    0,    8,    3,    0,    0,    8,    3,    0,    0,
//           8,    3,    4,    0,   12,    3,    0,    0,   12,    3,    4,    0,   16,    3,    0,    0,
//          16,    3,    0,    0,   16,    3,    4,    0,   20,    3,    0,    0,   20,    3,    0,    0,
//          20,    3,    4,    0,   24,    3,    0,    0,   24,    3,    0,    0,   24,    3,    0,    0,
//          24,    3,    4,    0,   28,    3,    0,    0,   28,    3,    0,    0,   28,    3,    0,    0,
//          28,    3,    4,    0,   32,    3,    0,    0,   32,    3,    4,    0,   36,    3,    0,    0,
//          36,    3,    0,    0,   36,    3,    4,    0,   40,    3,    0,    0,   40,    3,    0,    0,
//          40,    3,    0,    0,   40,    3,    4,    0,   44,    3,    0,    0,   44,    3,    0,    0,
//          44,    3,    4,    0,   48,    3,    0,    0,   48,    3,    0,    0,   48,    3,    0,    0,
//          48,    3,    4,    0,   52,    3,    0,    0,   52,    3,    0,    0,   52,    3,    0,    0,
//          52,    3,    4,    0,   56,    3,    0,    0,   56,    3,    0,    0,   56,    3,    4,    0,
//          60,    3,    0,    0,   60,    3,    0,    0,   60,    3,    4,    0,   64,    3,    0,    0,
//          64,    3,    4,    0,   68,    3,    0,    0,   68,    3,    0,    0,   68,    3,    0,    0,
//          68,    3,    4,    0,   72,    3,    0,    0,   72,    3,    0,    0,   72,    3,    0,    0,
//          72,    3,    4,    0,   76,    3,    0,    0,   76,    3,    0,    0,   76,    3,    4,    0,
//          80,    3,    0,    0,   80,    3,    0,    0,   80,    3,    0,    0,   80,    3,    4,    0,
//          84,    3,    0,    0,   84,    3,    0,    0,   84,    3,    0,    0,   84,    3,    4,    0,
//          88,    3,    0,    0,   88,    3,    0,    0,   88,    3,    4,    0,   92,    3,    0,    0,
//          92,    3,    0,    0,   92,    3,    0,    0,   92,    3,    4,    0,   96,    3,    0,    0,
//          96,    3,    0,    0,   96,    3,    0,    0,   96,    3,    4,    0,  100,    3,    0,    0,
//         100,    3,    0,    0,  100,    3,    4,    0,  104,    3,    0,    0,  104,    3,    0,    0,
//         104,    3,    0,    0,  104,    3,    4,    0,  108,    3,    0,    0,  108,    3,    0,    0,
//         108,    3,    4,    0,  112,    3,    0,    0,  112,    3,    0,    0,  112,    3,    0,    0,
//         112,    3,    4,    0,  116,    3,    0,    0,  116,    3,    0,    0,  116,    3,    0,    0,
//         116,    3,    0,    0,  116,    3,    4,    0,  120,    3,    0,    0,  120,    3,    0,    0,
//         120,    3,    0,    0,  120,    3,    4,    0,  124,    3,    0,    0,  124,    3,    0,    0,
//         124,    3,    0,    0,  124,    3,    4,    0, -128,    3,    0,    0, -128,    3,    0,    0,
//        -128,    3,    4,    0, -124,    3,    0,    0, -124,    3,    0,    0, -124,    3,    4,    0,
//        -120,    3,    0,    0, -120,    3,    0,    0, -120,    3,    0,    0, -120,    3,    4,    0,
//        -116,    3,    0,    0, -116,    3,    0,    0, -116,    3,    4,    0, -112,    3,    0,    0,
//        -112,    3,    0,    0, -112,    3,    0,    0, -112,    3,    4,    0, -108,    3,    0,    0,
//        -108,    3,    0,    0, -108,    3,    0,    0, -108,    3,    0,    0, -108,    3,    4,    0,
//        -104,    3,    0,    0, -104,    3,    0,    0, -104,    3,    4,    0, -100,    3,    0,    0,
//        -100,    3,    0,    0, -100,    3,    4,    0,  -96,    3,    0,    0,  -96,    3,    0,    0,
//         -96,    3,    0,    0,  -96,    3,    4,    0,  -92,    3,    0,    0,  -92,    3,    0,    0,
//         -92,    3,    0,    0,  -92,    3,    0,    0,  -92,    3,    4,    0,  -88,    3,    0,    0,
//         -88,    3,    0,    0,  -88,    3,    4,    0,  -84,    3,    0,    0,  -84,    3,    0,    0,
//         -84,    3,    0,    0,  -84,    3,    4,    0,  -80,    3,    0,    0,  -80,    3,    0,    0,
//         -80,    3,    0,    0,  -80,    3,    0,    0,  -80,    3,    4,    0,  -76,    3,    0,    0,
//         -76,    3,    0,    0,  -76,    3,    4,    0,  -72,    3,    0,    0,  -72,    3,    0,    0,
//         -72,    3,    0,    0,  -72,    3,    0,    0,  -72,    3,    4,    0,  -68,    3,    0,    0,
//         -68,    3,    0,    0,  -68,    3,    0,    0,  -68,    3,    4,    0,  -64,    3,    0,    0,
//         -64,    3,    0,    0,  -64,    3,    4,    0,  -60,    3,    0,    0,  -60,    3,    0,    0,
//         -60,    3,    0,    0,  -60,    3,    0,    0,  -60,    3,    4,    0,  -56,    3,    0,    0,
//         -56,    3,    0,    0,  -56,    3,    0,    0,  -56,    3,    4,    0,  -52,    3,    0,    0,
//         -52,    3,    0,    0,  -52,    3,    0,    0,  -52,    3,    4,    0,  -48,    3,    0,    0,
//         -48,    3,    0,    0,  -48,    3,    0,    0,  -48,    3,    4,    0,  -44,    3,    0,    0,
//         -44,    3,    0,    0,  -44,    3,    0,    0,  -44,    3,    4,    0,  -40,    3,    0,    0,
//         -40,    3,    0,    0,  -40,    3,    0,    0,  -40,    3,    4,    0,  -36,    3,    0,    0,
//         -36,    3,    0,    0,  -36,    3,    0,    0,  -36,    3,    4,    0,  -32,    3,    0,    0,
//         -32,    3,    0,    0,  -32,    3,    0,    0,  -32,    3,    0,    0,  -32,    3,    4,    0,
//         -28,    3,    0,    0,  -28,    3,    0,    0,  -28,    3,    0,    0,  -28,    3,    0,    0,
//         -28,    3,    4,    0,  -24,    3,    0,    0,  -24,    3,    0,    0,  -24,    3,    4,    0,
//         -20,    3,    0,    0,  -20,    3,    0,    0,  -20,    3,    0,    0,  -20,    3,    0,    0,
//         -20,    3,    4,    0,  -16,    3,    0,    0,  -16,    3,    0,    0,  -16,    3,    0,    0,
//         -16,    3,    4,    0,  -12,    3,    0,    0,  -12,    3,    0,    0,  -12,    3,    0,    0,
//         -12,    3,    4,    0,   -8,    3,    0,    0,   -8,    3,    0,    0,   -8,    3,    0,    0,
//          -8,    3,    0,    0,   -8,    3,    4,    0,   -4,    3,    0,    0,   -4,    3,    0,    0,
//          -4,    3,    0,    0,   -4,    3,    0,    0,   -4,    3,    0,    0,   -4,    3,    0,    0
    },

    /*pnGreenTable =*/
    {
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255,
        128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    77//,
//      128,    128,    128,    128,    128,    128,    128,    255,    255,    255,    255,    255,    255,    255,    255,    255   -32,    2,    4,    0,  -28,    2,    0,    0,  -28,    2,    0,    0,  -28,    2,    4,    0,
//         -24,    2,    0,    0,  -24,    2,    0,    0,  -24,    2,    4,    0,  -20,    2,    0,    0,
//         -20,    2,    0,    0,  -20,    2,    0,    0,  -20,    2,    4,    0,  -16,    2,    0,    0,
//         -16,    2,    0,    0,  -16,    2,    4,    0,  -12,    2,    0,    0,  -12,    2,    4,    0,
//          -8,    2,    0,    0,   -8,    2,    0,    0,   -8,    2,    0,    0,   -8,    2,    4,    0,
//          -4,    2,    0,    0,   -4,    2,    4,    0,    0,    3,    0,    0,    0,    3,    0,    0,
//           0,    3,    4,    0,    4,    3,    0,    0,    4,    3,    0,    0,    4,    3,    0,    0,
//           4,    3,    4,    0,    8,    3,    0,    0,    8,    3,    0,    0,    8,    3,    0,    0,
//           8,    3,    4,    0,   12,    3,    0,    0,   12,    3,    4,    0,   16,    3,    0,    0,
//          16,    3,    0,    0,   16,    3,    4,    0,   20,    3,    0,    0,   20,    3,    0,    0,
//          20,    3,    4,    0,   24,    3,    0,    0,   24,    3,    0,    0,   24,    3,    0,    0,
//          24,    3,    4,    0,   28,    3,    0,    0,   28,    3,    0,    0,   28,    3,    0,    0,
//          28,    3,    4,    0,   32,    3,    0,    0,   32,    3,    4,    0,   36,    3,    0,    0,
//          36,    3,    0,    0,   36,    3,    4,    0,   40,    3,    0,    0,   40,    3,    0,    0,
//          40,    3,    0,    0,   40,    3,    4,    0,   44,    3,    0,    0,   44,    3,    0,    0,
//          44,    3,    4,    0,   48,    3,    0,    0,   48,    3,    0,    0,   48,    3,    0,    0,
//          48,    3,    4,    0,   52,    3,    0,    0,   52,    3,    0,    0,   52,    3,    0,    0,
//          52,    3,    4,    0,   56,    3,    0,    0,   56,    3,    0,    0,   56,    3,    4,    0,
//          60,    3,    0,    0,   60,    3,    0,    0,   60,    3,    4,    0,   64,    3,    0,    0,
//          64,    3,    4,    0,   68,    3,    0,    0,   68,    3,    0,    0,   68,    3,    0,    0,
//          68,    3,    4,    0,   72,    3,    0,    0,   72,    3,    0,    0,   72,    3,    0,    0,
//          72,    3,    4,    0,   76,    3,    0,    0,   76,    3,    0,    0,   76,    3,    4,    0,
//          80,    3,    0,    0,   80,    3,    0,    0,   80,    3,    0,    0,   80,    3,    4,    0,
//          84,    3,    0,    0,   84,    3,    0,    0,   84,    3,    0,    0,   84,    3,    4,    0,
//          88,    3,    0,    0,   88,    3,    0,    0,   88,    3,    4,    0,   92,    3,    0,    0,
//          92,    3,    0,    0,   92,    3,    0,    0,   92,    3,    4,    0,   96,    3,    0,    0,
//          96,    3,    0,    0,   96,    3,    0,    0,   96,    3,    4,    0,  100,    3,    0,    0,

//         100,    3,    0,    0,  100,    3,    4,    0,  104,    3,    0,    0,  104,    3,    0,    0,
//         104,    3,    0,    0,  104,    3,    4,    0,  108,    3,    0,    0,  108,    3,    0,    0,
//         108,    3,    4,    0,  112,    3,    0,    0,  112,    3,    0,    0,  112,    3,    0,    0,
//         112,    3,    4,    0,  116,    3,    0,    0,  116,    3,    0,    0,  116,    3,    0,    0,
//         116,    3,    0,    0,  116,    3,    4,    0,  120,    3,    0,    0,  120,    3,    0,    0,
//         120,    3,    0,    0,  120,    3,    4,    0,  124,    3,    0,    0,  124,    3,    0,    0,
//         124,    3,    0,    0,  124,    3,    4,    0, -128,    3,    0,    0, -128,    3,    0,    0,
//        -128,    3,    4,    0, -124,    3,    0,    0, -124,    3,    0,    0, -124,    3,    4,    0,
//        -120,    3,    0,    0, -120,    3,    0,    0, -120,    3,    0,    0, -120,    3,    4,    0,
//        -116,    3,    0,    0, -116,    3,    0,    0, -116,    3,    4,    0, -112,    3,    0,    0,
//        -112,    3,    0,    0, -112,    3,    0,    0, -112,    3,    4,    0, -108,    3,    0,    0,
//        -108,    3,    0,    0, -108,    3,    0,    0, -108,    3,    0,    0, -108,    3,    4,    0,
//        -104,    3,    0,    0, -104,    3,    0,    0, -104,    3,    4,    0, -100,    3,    0,    0,
//        -100,    3,    0,    0, -100,    3,    4,    0,  -96,    3,    0,    0,  -96,    3,    0,    0,
//         -96,    3,    0,    0,  -96,    3,    4,    0,  -92,    3,    0,    0,  -92,    3,    0,    0,
//         -92,    3,    0,    0,  -92,    3,    0,    0,  -92,    3,    4,    0,  -88,    3,    0,    0,
//         -88,    3,    0,    0,  -88,    3,    4,    0,  -84,    3,    0,    0,  -84,    3,    0,    0,
//         -84,    3,    0,    0,  -84,    3,    4,    0,  -80,    3,    0,    0,  -80,    3,    0,    0,
//         -80,    3,    0,    0,  -80,    3,    0,    0,  -80,    3,    4,    0,  -76,    3,    0,    0,
//         -76,    3,    0,    0,  -76,    3,    4,    0,  -72,    3,    0,    0,  -72,    3,    0,    0,
//         -72,    3,    0,    0,  -72,    3,    0,    0,  -72,    3,    4,    0,  -68,    3,    0,    0,
//         -68,    3,    0,    0,  -68,    3,    0,    0,  -68,    3,    4,    0,  -64,    3,    0,    0,
//         -64,    3,    0,    0,  -64,    3,    4,    0,  -60,    3,    0,    0,  -60,    3,    0,    0,
//         -60,    3,    0,    0,  -60,    3,    0,    0,  -60,    3,    4,    0,  -56,    3,    0,    0,
//         -56,    3,    0,    0,  -56,    3,    0,    0,  -56,    3,    4,    0,  -52,    3,    0,    0,
//         -52,    3,    0,    0,  -52,    3,    0,    0,  -52,    3,    4,    0,  -48,    3,    0,    0,
//         -48,    3,    0,    0,  -48,    3,    0,    0,  -48,    3,    4,    0,  -44,    3,    0,    0,
//         -44,    3,    0,    0,  -44,    3,    0,    0,  -44,    3,    4,    0,  -40,    3,    0,    0,
//         -40,    3,    0,    0,  -40,    3,    0,    0,  -40,    3,    4,    0,  -36,    3,    0,    0,
//         -36,    3,    0,    0,  -36,    3,    0,    0,  -36,    3,    4,    0,  -32,    3,    0,    0,
//         -32,    3,    0,    0,  -32,    3,    0,    0,  -32,    3,    0,    0,  -32,    3,    4,    0,
//         -28,    3,    0,    0,  -28,    3,    0,    0,  -28,    3,    0,    0,  -28,    3,    0,    0,
//         -28,    3,    4,    0,  -24,    3,    0,    0,  -24,    3,    0,    0,  -24,    3,    4,    0,
//         -20,    3,    0,    0,  -20,    3,    0,    0,  -20,    3,    0,    0,  -20,    3,    0,    0,
//         -20,    3,    4,    0,  -16,    3,    0,    0,  -16,    3,    0,    0,  -16,    3,    0,    0,
//         -16,    3,    4,    0,  -12,    3,    0,    0,  -12,    3,    0,    0,  -12,    3,    0,    0,
//         -12,    3,    4,    0,   -8,    3,    0,    0,   -8,    3,    0,    0,   -8,    3,    0,    0,
//          -8,    3,    0,    0,   -8,    3,    4,    0,   -4,    3,    0,    0,   -4,    3,    0,    0,
//          -4,    3,    0,    0,   -4,    3,    0,    0,   -4,    3,    0,    0,   -4,    3,    0,    0
    }
};

static VCAM_3ASKIP_IPIPE_GBCE_CFGTYPE gbceDump = {
    /*nEnable = */0,
    /*nTyp = */(VCAM_3ASKIP_IPIPE_GBCE_METHODTYPE)2,
    /*LookupTable =*/
    {
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    256,
        128,    128,    128,    128,    128,    128,    128,    256,    256,    256,    256,    256,    256,    256,    256,    77,

    }
};
