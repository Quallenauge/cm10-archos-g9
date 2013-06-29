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
        /*.pInBiffData = */{ 0 }
};

static VCAM_3ASKIP_IPIPE_BOXCAR_CFGTYPE boxCarDump = {
    /*.nEnable = */0,
    /*.nOst = */1,
    /*.nBoxSize = */1,
    /*.nShift = */0,
    /*.pAddr = */{ 0 }
};

static VCAM_3ASKIP_IPIPE_BSC_CFGTYPE bscDump = {
    /*.nEnable = */1,
    /*.nMode = */1,
    /*.nColSample = */1,
    /*.nRowSample = */1,
    /*.nElement = */0,
    {
    /*.nColPos.nVectors = */3,
    /*.nColPos.nShift = */0,
    /*.nColPos.nVPos = */0,
    /*.nColPos.nHPos = */0,
    /*.nColPos.nVNum = */144,
    /*.nColPos.nHNum = */636,
    /*.nColPos.nVSkip = */2,
    /*.nColPos.nHSkip = */2
    },
    {
    /*.nRowPos.nVectors = */3,
    /*.nRowPos.nShift = */0,
    /*.nRowPos.nVPos = */0,
    /*.nRowPos.nHPos = */0,
    /*.nRowPos.nVNum = */432,
    /*.nRowPos.nHNum = */212,
    /*.nRowPos.nVSkip = */2,
    /*.nRowPos.nHSkip = */2
    }
};

static VCAM_3ASKIP_IPIPE_CAR_CFGTYPE carDump = {
    /*.nEnable = */0,
    /*.nTyp = */3,
    /*.nSw0Thr = */255,
    /*.nSw1Thr = */192,
    /*.nHpfType = */3,
    /*.nHpfShift = */0,
    /*.nHpfThr = */32,
    /*.nGn1Gain = */255,
    /*.nGn1Shift = */5,
    /*.nGn1Min = */128,
    /*.nGn2Gain = */255,
    /*.nGn2Shift = */12,
    /*.nGn2Min = */128
};

static VCAM_3ASKIP_IPIPE_CFA_CFGTYPE cfaDump = {
    /*.nEnable = */1,
    /*.eMode = */(VCAM_3ASKIP_IPIPE_CFA_MODETYPE)0,
    {
    /*.tDir.nHpfThr = */600,
    /*.tDir.nHpfSlope = */57,
    /*.tDir.nMixThr = */10,
    /*.tDir.nMixSlope = */10,
    /*.tDir.nDirThr = */16,
    /*.tDir.nDirSlope = */10,
    /*.tDir.nDirNdwt = */16
    },
    {
    /*.tDaa.nMonoHueFra = */16,
    /*.tDaa.nMonoEdgThr = */25,
    /*.tDaa.nMonoThrMin = */25,
    /*.tDaa.nMonoThrSlope = */20,
    /*.tDaa.nMonoSlpMin = */500,
    /*.tDaa.nMonoSlpSlp = */20,
    /*.tDaa.nMonoLpwt = */24
    }
};

static VCAM_3ASKIP_IPIPE_CGS_CFGTYPE cgsDump = {
    /*.enable = */0,
    {
    /*.y_chroma_low.thr = */0,
    /*.y_chroma_low.gain = */250,
    /*.y_chroma_low.shift = */0,
    /*.y_chroma_low.min = */10
    },
    {
    /*.y_chroma_high.thr = */106,
    /*.y_chroma_high.gain = */100,
    /*.y_chroma_high.shift = */0,
    /*.y_chroma_high.min = */50
    },
    {
    /*.c_chroma.thr = */22,
    /*.c_chroma.gain = */10,
    /*.c_chroma.shift = */0,
    /*.c_chroma.min = */50
    }
};

static VCAM_3ASKIP_ISIF_CLAMP_CFGTYPE clampDump = {
    /*.nEnable = */0,
    /*.eHClampMode = */(VCAM_3ASKIP_ISIF_HORIZONTAL_CLAMP_MODETYPE)0,
    /*.eBlackClampMode = */(VCAM_3ASKIP_ISIF_BLACK_CLAMP_MODETYPE)0,
    /*.nDCOffsetClampVal = */65472,
    /*.nBlackClampVStartPos = */0,
    {
    /*.tHorizontalBlack.nVPos = */0,
    /*.tHorizontalBlack.eVSize = */0,
    /*.tHorizontalBlack.nHPos = */0,
    /*.tHorizontalBlack.eHSize = */(VCAM_3ASKIP_ISIF_VBLACK_PIXEL_WIDTHTYPE)0,
    /*.tHorizontalBlack.line_avg_coef = */0,
    /*.tHorizontalBlack.reset_mode = */(VCAM_3ASKIP_ISIF_VBLACK_RESET_MODETYPE)0,
    /*.tHorizontalBlack.reset_value = */0
    },
    {
    /*.tVerticalBlack.nVPos = */0,
    /*.tVerticalBlack.nVSize = */0,
    /*.tVerticalBlack.nHPos = */0,
    /*.tVerticalBlack.HSize = */(VCAM_3ASKIP_ISIF_VBLACK_PIXEL_WIDTHTYPE)0,
    /*.tVerticalBlack.line_avg_coef = */0,
    /*.tVerticalBlack.reset_mode = */(VCAM_3ASKIP_ISIF_VBLACK_RESET_MODETYPE)0,
    /*.tVerticalBlack.reset_value = */0
    }
};

static VCAM_3ASKIP_IPIPEIF_DFS_CFGTYPE dfsDump = {
    /*.eDfsGainEn = */(VCAM_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE)1,
    /*.nDfsGainVal = */512,
    /*.nDfsGainThr = */16,
    /*.nOclip = */255,
    /*.nDfsDir = */0
};

static VCAM_3ASKIP_IPIPEIF_DPC_CFGTYPE dpc1Dump = {
    /*.eDpcEn = */(VCAM_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE)1,
    /*.eDpcThr = */20
};

static VCAM_3ASKIP_IPIPEIF_DPC_CFGTYPE dpc2Dump = {
    /*.eDpcEn = */(VCAM_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE)1,
    /*.eDpcThr = */20
};

static VCAM_3ASKIP_IPIPEIF_DPCM_CFGTYPE dpcmDump = {
    /*.nDpcmBitSize = */    (VCAM_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE)1,
    /*.nDpcmEn = */         (VCAM_3ASKIP_IPIPEIF_DPCM_PRED_TYPTYPE)1,
    /*.nDpcmPredictor = */  (VCAM_3ASKIP_IPIPEIF_DPCM_BIT_SIZETYPE)1
};


static VCAM_3ASKIP_IPIPE_DPCLUT_CFGTYPE dpcLutDump = {
    /*.nEnable = */0,
    /*.eTableType = */(VCAM_3ASKIP_IPIPE_DPC_LUT_TBL_SIZETYPE)0,
    /*.eReplaceType = */(VCAM_3ASKIP_IPIPE_DPC_LUT_REPLACEMENTTYPE)0,
    /*.nLutValidAddr = */ { 0 },
    /*.nLutSize = */0,
    /*.nLutTable0Addr = */{ 0 },
    /*.nLutTable1Addr = */{ 0 }
};

//*(ptDpcm) = {...}
//	nDpcmEn = 0
//	nDpcmPredictor = 0
//	nDpcmBitSize = 0

static VCAM_3ASKIP_IPIPE_DPCOTF_CFGTYPE dpcOtfDump;

static VCAM_3ASKIP_ISIF_GAINOFFSET_CFGTYPE gainOffsetDump = {
        /*.gain_offset_featureflag = */6,
        /*.gain_r = */520,
        /*.gain_gr = */520,
        /*.gain_gb = */520,
        /*.gain_bg = */520,
        /*.offset = */0
};


static VCAM_3ASKIP_IPIPE_GIC_CFGTYPE gicDump = {
    /*.nEnable = */1,
    /*.eLscGain = */(VCAM_3ASKIP_IPIPE_GIC_LSC_GAINTYPE)0,
    /*.eSel = */(VCAM_3ASKIP_IPIPE_GIC_SELTYPE)0,
    /*.eTyp = */(VCAM_3ASKIP_IPIPE_GIC_INDEXTYPE)0,
    /*.nGicGain = */128,
    /*.nGicNfGain = */128,
    /*.nGicThr = */64,
    /*.nGicSlope = */20
};

static VCAM_3ASKIP_H3A_AEWB_PARAMTYPE h3aAewbDump = {
    /*.eAewbEnable = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)1,
    /*.ptAewbPaxelWin = */{ (uint16_t)0, (uint8_t)0, (uint16_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0 },
    /*.ptBlkWinDims = */{ 0, 0 },
    /*.eAeOpFmt = */(VCAM_3ASKIP_H3A_AEWB_OP_FMTTYPE)2,
    /*.nShiftValue = */0,
    /*.nSaturationLimit = */1022,
    /*.nAewbOpAddr = */{ 0 }
};

static VCAM_3ASKIP_H3A_COMMON_CFGTYPE h3aCommonDump = {
    /*.eAfMedianEn = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)0,
    /*.eAewbMedianEn = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)0,
    /*.nMedianFilterThreshold = */0,
    /*.eAfAlawEn = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)1,
    /*.eAewbAlawEn = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)0,
    /*.eIpipeifAveFiltEn = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)0,
    /*.eH3aDecimEnable = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)0,
    /*.nReserved = */ 0
};

static VCAM_3ASKIP_IPIPE_HIST_CFGTYPE histDump = {
    /*.nEnable = */0,
    /*.nOst = */0,
    /*.nSel = */0,
    /*.nType = */0,
    /*.nBins = */0,
    /*.nShift = */0,
    /*.nCol = */0,
    /*.nRegions = */0,
    /*.ptHistDim = */
    {
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0 }
    },
    /*.nClearTable = */0,
    /*.nTableSel = */0,
    /*.pnGainTbl = */{ 0, 0, 0, 0 }
};

static VCAM_3ASKIP_RSZ_LPF_CFGTYPE hlpfDump = {
    /*.nCIntensity = */22,
    /*.nYIntensity = */21
};

static VCAM_3ASKIP_ISIF_2DLSC_CFGTYPE lsc2dDump = {
    /*.nEnable = */0,
    /*.nLscHSize = */0,
    /*.nLscVSize = */0,
    /*.nHDirDataOffset = */16,
    /*.nVDirDataOffset = */2,
    /*.nHPosInPaxel = */0,
    /*.nVPosInPaxel = */0,
    /*.ePaxHeight = */(VCAM_3ASKIP_ISIF_LSC_GAIN_MODE_MNTYPE)3,
    /*.ePaxLength = */(VCAM_3ASKIP_ISIF_LSC_GAIN_MODE_MNTYPE)3,
    /*.eGainFormat = */(VCAM_3ASKIP_ISIF_LSC_GAIN_FORMATTYPE)0,
    /*.nOffsetScalingFactor = */0,
    /*.eOffsetShiftVal = */(VCAM_3ASKIP_ISIF_LSC_OFFSET_SHIFTTYPE)0,
    /*.eOffsetEnable = */(VCAM_3ASKIP_ISIF_LSC_OFFSET_ENABLETYPE)0,
    /*.nGainTableAddress = */{ 0 },
    /*.nGainTableLength = */255,
    /*.nOffsetTableAddress = */{ 0 },
    /*.nOffsetTableLength = */255
};

static VCAM_3ASKIP_IPIPE_LSC_CFGTYPE lscDump = {
    /*.nVOffset = */0,
    /*.nVLinearCoeff = */0,
    /*.nVQuadraticCoeff = */3720,
    /*.nVLinearShift = */11,
    /*.nVQuadraticShift = */11,
    /*.nHOffset = */0,
    /*.nHLinearCoeff = */0,
    /*.nHQuadraticCoeff = */3601,
    /*.nHLinearShift = */11,
    /*.nHQuadraticShift = */11,
    /*.nGainR = */57,
    /*.nGainGR = */64,
    /*.nGainGB = */64,
    /*.nGainB = */50,
    /*.nOffR = */227,
    /*.nOffGR = */245,
    /*.nOffGB = */246,
    /*.nOffB = */207,
    /*.nShift = */9,
    /*.nMax = */511
};

static VCAM_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE nf1Dump = {
    /*.nEnable = */0,
    /*.eNFNum = */(VCAM_3ASKIP_IPIPE_NOISE_FILTERTYPE)2,
    /*.eSel = */(VCAM_3ASKIP_IPIPE_NF_SELTYPE)0,
    /*.eLscGain = */(VCAM_3ASKIP_IPIPE_NF_LSC_GAINTYPE)0,
    /*.eTyp = */(VCAM_3ASKIP_IPIPE_NF_SAMPLE_METHODTYPE)0,
    /*.nDownShiftVal = */0,
    /*.nSpread = */0,

    /*.pnThr = */
    {
                0x0041,
                0x003C,
                0x004B,
                0x0046,
                0x0055,
                0x0050,
                0x0064,
                0x005A
    },

    /*.pnStr = */
    {
                0x08,
                0x08,
                0x08,
                0x08,
                0x08,
                0x08,
                0x08,
                0x08
    },

    /*.pnSpr = */
    {
                0x08,
                0x08,
                0x08,
                0x08,
                0x08,
                0x08,
                0x08,
                0x08
    },
    /*.nEdgeMin = */0,
    /*.nEdgeMax = */0
};

static VCAM_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE nf2Dump = {
    /*.nEnable = */0,
    /*.eNFNum = */(VCAM_3ASKIP_IPIPE_NOISE_FILTERTYPE)2,
    /*.eSel = */(VCAM_3ASKIP_IPIPE_NF_SELTYPE)0,
    /*.eLscGain = */(VCAM_3ASKIP_IPIPE_NF_LSC_GAINTYPE)0,
    /*.eTyp = */(VCAM_3ASKIP_IPIPE_NF_SAMPLE_METHODTYPE)0,
    /*.nDownShiftVal = */0,
    /*.nSpread = */0,

    /*.pnThr = */
    {
        0x0041,
        0x003C,
        0x004B,
        0x0046,
        0x0055,
        0x0050,
        0x0064,
        0x005A
    },

    /*.pnStr = */
    {
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08
    },

    /*.pnSpr = */
    {
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
    },

    /*.nEdgeMin = */0,
    /*.nEdgeMax = */0
};

static VCAM_3ASKIP_IPIPE_RGBRGB_CFGTYPE rgb2rgb1Dump = {
    /*.pnMulOff = */
    {
        {
            0xFFBA,
            0x016A,
            0xFFAA
        },
        {
            0xFFDC,
            0xFFFD,
            0x0159
        },
        {
            0xFF1B,
            0xFFFD,
            0x0100
        }
    },
    /*.pnOft = */
    {
        0x0000,
        0x0000,
        0x0000
    }
};


static VCAM_3ASKIP_IPIPE_RGBRGB_CFGTYPE rgb2rgb2Dump = {
    /*.pnMulOff = */
    {
        {
            0xFFBA,
            0x016A,
            0xFFAA
        },
        {
            0xFFDC,
            0xFFFD,
            0x0159
        },
        {
            0xFF1B,
            0xFFFD,
            0x0100
        }
    },
    /*.pnOft = */
    {
        0x0000,
        0x0000,
        0x0000
    }
};

static VCAM_3ASKIP_IPIPE_RGBYUV_CFGTYPE rgb2yuvDump = {
    /*.nBrightness = */0,
    /*.nContrast = */16,
    /*.pnMulVal = */
    {
        {
            0x004D,
            0x0096,
            0x001D
        },
        {
            0xFFD5,
            0xFFAB,
            0x0080
        },
        {
            0x0080,
            0xFF95,
            0xFFEB
        }
    },
    /*.pnOffset = */
    {
        0x0000,
        0x0080,
        0x0080
    },
};

static VCAM_3ASKIP_ISIF_VLDC_CFGTYPE vldcDump = {
    /*.nEnable = */0,
    /*.eDisableVldcUpperPixels = */(VCAM_3ASKIP_ISIF_VDLC_PIXEL_DEPENDENCYTYPE)1,
    /*.nVldcShiftVal = */3,
    /*.eVldcModeSelect = */(VCAM_3ASKIP_ISIF_VLDC_MODE_SELECTTYPE)2,
    /*.nVldcSaturationLvl = */511,
    /*.nDefectLines = */1,
    {
        /*.tVldcDefectLineParams.nVerticalDefectPosition = */0,
        /*.tVldcDefectLineParams.nHorizontalDefectPosition = */0,
        /*.tVldcDefectLineParams.nSub1ValueVldc = */0,
        /*.tVldcDefectLineParams.nSub2LessThanVldc = */0,
        /*.tVldcDefectLineParams.nSub3GreaterThanVldc = */0
    }
};

static VCAM_3ASKIP_RSZ_LPF_CFGTYPE vlpDump = {
    /*.nCIntensity = */15,
    /*.nYIntensity = */14
};

static VCAM_3ASKIP_IPIPE_WB_CFGTYPE wbDump = {
    /*.pnOffset = */
    {
        0x0000,
        0x0000,
        0x0000,
        0x0000
    },
    /*.pnGain = */
    {
        0x0200,
        0x0278,
        0x03D8,
        0x0200
    }
};

static VCAM_3ASKIP_IPIPE_YUV444YUV422_CFGTYPE yuv2yuvDump = {
    /*.ePos = */(VCAM_3ASKIP_IPIPE_YUV_PHASE_POSTYPE)0,
    /*.nLpfEn = */1
};



static VCAM_3ASKIP_H3A_AF_PARAMTYPE h3aAfDump = {
        /*eAfEnable = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)1,
        /*eRgbPos = */(VCAM_3ASKIP_H3A_RGB_POSTYPE)3,
        /*ePeakModeEn = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)1,
        /*eVerticalFocusEn = */(VCAM_3ASKIP_H3A_FEATURE_ENABLETYPE)0,
        /*nIirStartPos = */0,
        /*ptAfPaxelWin = */
    {
        0 ,0, 0, 0, 0, 0, 0, 0
    },
    /*ptIir1 = */
    {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
          0
    },
    /*ptIir2 = */
    {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
          0
    },
    /*ptFir1 = */
    {
        { 0, 0, 0, 0, 0 },
        0
    },
    /*ptFir2 = */
    {
        { 0, 0, 0, 0, 0 },
        0
    },
    /*nAfOpAddr = */
    { 0 }
};

static VCAM_3ASKIP_CAM_CONTROL_EXPGAINTYPE expCtrlDump = {
    /*exp =*/ 0xA,
    /*a_gain = */0xB,
    /*mask = */0xC,
    /*nAgainErr = */0xD,
    /*nDigitalISPGain = */0xE
};

static VCAM_3ASKIP_IPIPE_EE_CFGTYPE edgeEnhDump = {
    /*nEnable = */0,
    /*eHaloReduction = */(VCAM_3ASKIP_IPIPE_EE_HALO_CTRLTYPE)0,
    /*pnMulVal =*/
    {
        0x000C,
        0x0030,
        0x000C,
        0xFFF6,
        0xFFFA,
        0x0000,
        0xFFFA,
        0xFFF6,
        0x0000
    },
    /*nSel = */0,
    /*nShiftHp = */3,
    /*nThreshold = */0,
    /*nGain = */0,
    /*nHpfLowThr = */0,
    /*nHpfHighThr = */0,
    /*nHpfGradientGain = */0,
    /*nHpfgradientOffset = */0,
    /*pnEeTable =*/
    {
             0,      0,      0,      0,      0,      0,      0,      0,      1,      1,      1,      1,      1,      2,      2,      2,
             2,      2,      3,      3,      3,      3,      3,      4,      4,      4,      4,      4,      5,      5,      5,      5,
             5,      6,      6,      6,      6,      6,      7,      7,      7,      7,      7,      8,      8,      8,      8,      8,
             9,      9,      9,      9,      9,     10,     10,     10,     10,     10,     11,     11,     11,     11,     11,     12,
            12,     12,     12,     12,     13,     13,     13,     13,     13,     14,     14,     14,     14,     14,     15,     15,
            15,     15,     15,     16,     16,     16,     16,     16,     17,     17,     17,     17,     17,     18,     18,     18,
            18,     18,     19,     19,     19,     19,     19,     20,     20,     20,     20,     20,     21,     21,     21,     21,
            21,     22,     22,     22,     22,     22,     23,     23,     23,     23,     23,     24,     24,     24,     24,     24,
            25,     25,     25,     25,     25,     26,     26,     26,     26,     26,     27,     27,     27,     27,     27,     28,
            28,     28,     28,     28,     29,     29,     29,     29,     29,     30,     30,     30,     30,     30,     31,     31,
            31,     31,     31,     32,     32,     32,     32,     32,     33,     33,     33,     33,     33,     34,     34,     34,
            34,     34,     35,     35,     35,     35,     35,     36,     36,     36,     36,     36,     37,     37,     37,     37,
            37,     38,     38,     38,     38,     38,     39,     39,     39,     39,     39,     40,     40,     40,     40,     40,
            41,     41,     41,     41,     41,     41,     42,     42,     42,     42,     42,     42,     43,     43,     43,     43,
            43,     43,     43,     44,     44,     44,     44,     44,     44,     44,     44,     45,     45,     45,     45,     45,
            45,     45,     45,     45,     45,     46,     46,     46,     46,     46,     46,     46,     46,     46,     46,     46,
            46,     46,     46,     46,     46,     46,     47,     47,     47,     47,     47,     47,     47,     47,     47,     47,
            47,     47,     47,     47,     47,     47,     47,     47,     46,     46,     46,     46,     46,     46,     46,     46,
            46,     46,     46,     46,     46,     46,     46,     46,     46,     45,     45,     45,     45,     45,     45,     45,
            45,     45,     45,     44,     44,     44,     44,     44,     44,     44,     43,     43,     43,     43,     43,     43,
            43,     42,     42,     42,     42,     42,     42,     41,     41,     41,     41,     41,     40,     40,     40,     40,
            40,     39,     39,     39,     39,     39,     38,     38,     38,     38,     37,     37,     37,     37,     36,     36,
            36,     36,     35,     35,     35,     35,     34,     34,     34,     34,     33,     33,     33,     33,     32,     32,
            32,     31,     31,     31,     31,     30,     30,     30,     29,     29,     29,     29,     28,     28,     28,     27,
            27,     27,     27,     26,     26,     26,     25,     25,     25,     24,     24,     24,     24,     23,     23,     23,
            22,     22,     22,     21,     21,     21,     21,     20,     20,     20,     19,     19,     19,     18,     18,     18,
            18,     17,     17,     17,     16,     16,     16,     15,     15,     15,     15,     14,     14,     14,     13,     13,
            13,     13,     12,     12,     12,     12,     11,     11,     11,     10,     10,     10,     10,      9,      9,      9,
             9,      8,      8,      8,      8,      7,      7,      7,      7,      7,      6,      6,      6,      6,      5,      5,
             5,      5,      5,      4,      4,      4,      4,      4,      4,      3,      3,      3,      3,      3,      3,      2,
             2,      2,      2,      2,      2,      2,      2,      1,      1,      1,      1,      1,      1,      1,      1,      1,
             1,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
             0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     -1,
            -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -2,     -2,     -2,     -2,     -2,     -2,     -2,
            -2,     -3,     -3,     -3,     -3,     -3,     -3,     -4,     -4,     -4,     -4,     -4,     -4,     -5,     -5,     -5,
            -5,     -5,     -6,     -6,     -6,     -6,     -7,     -7,     -7,     -7,     -7,     -8,     -8,     -8,     -8,     -9,
            -9,     -9,     -9,    -10,    -10,    -10,    -10,    -11,    -11,    -11,    -12,    -12,    -12,    -12,    -13,    -13,
           -13,    -13,    -14,    -14,    -14,    -15,    -15,    -15,    -15,    -16,    -16,    -16,    -17,    -17,    -17,    -18,
           -18,    -18,    -18,    -19,    -19,    -19,    -20,    -20,    -20,    -21,    -21,    -21,    -21,    -22,    -22,    -22,
           -23,    -23,    -23,    -24,    -24,    -24,    -24,    -25,    -25,    -25,    -26,    -26,    -26,    -27,    -27,    -27,
           -27,    -28,    -28,    -28,    -29,    -29,    -29,    -29,    -30,    -30,    -30,    -31,    -31,    -31,    -31,    -32,
           -32,    -32,    -33,    -33,    -33,    -33,    -34,    -34,    -34,    -34,    -35,    -35,    -35,    -35,    -36,    -36,
           -36,    -36,    -37,    -37,    -37,    -37,    -38,    -38,    -38,    -38,    -39,    -39,    -39,    -39,    -39,    -40,
           -40,    -40,    -40,    -40,    -41,    -41,    -41,    -41,    -41,    -42,    -42,    -42,    -42,    -42,    -42,    -43,
           -43,    -43,    -43,    -43,    -43,    -43,    -44,    -44,    -44,    -44,    -44,    -44,    -44,    -45,    -45,    -45,
           -45,    -45,    -45,    -45,    -45,    -45,    -45,    -46,    -46,    -46,    -46,    -46,    -46,    -46,    -46,    -46,
           -46,    -46,    -46,    -46,    -46,    -46,    -46,    -46,    -47,    -47,    -47,    -47,    -47,    -47,    -47,    -47,
           -47,    -47,    -47,    -47,    -47,    -47,    -47,    -47,    -47,    -47,    -46,    -46,    -46,    -46,    -46,    -46,
           -46,    -46,    -46,    -46,    -46,    -46,    -46,    -46,    -46,    -46,    -46,    -45,    -45,    -45,    -45,    -45,
           -45,    -45,    -45,    -45,    -45,    -44,    -44,    -44,    -44,    -44,    -44,    -44,    -44,    -43,    -43,    -43,
           -43,    -43,    -43,    -43,    -42,    -42,    -42,    -42,    -42,    -42,    -41,    -41,    -41,    -41,    -41,    -41,
           -40,    -40,    -40,    -40,    -40,    -39,    -39,    -39,    -39,    -39,    -38,    -38,    -38,    -38,    -38,    -37,
           -37,    -37,    -37,    -37,    -36,    -36,    -36,    -36,    -36,    -35,    -35,    -35,    -35,    -35,    -34,    -34,
           -34,    -34,    -34,    -33,    -33,    -33,    -33,    -33,    -32,    -32,    -32,    -32,    -32,    -31,    -31,    -31,
           -31,    -31,    -30,    -30,    -30,    -30,    -30,    -29,    -29,    -29,    -29,    -29,    -28,    -28,    -28,    -28,
           -28,    -27,    -27,    -27,    -27,    -27,    -26,    -26,    -26,    -26,    -26,    -25,    -25,    -25,    -25,    -25,
           -24,    -24,    -24,    -24,    -24,    -23,    -23,    -23,    -23,    -23,    -22,    -22,    -22,    -22,    -22,    -21,
           -21,    -21,    -21,    -21,    -20,    -20,    -20,    -20,    -20,    -19,    -19,    -19,    -19,    -19,    -18,    -18,
           -18,    -18,    -18,    -17,    -17,    -17,    -17,    -17,    -16,    -16,    -16,    -16,    -16,    -15,    -15,    -15,
           -15,    -15,    -14,    -14,    -14,    -14,    -14,    -13,    -13,    -13,    -13,    -13,    -12,    -12,    -12,    -12,
           -12,    -11,    -11,    -11,    -11,    -11,    -10,    -10,    -10,    -10,    -10,     -9,     -9,     -9,     -9,     -9,
            -8,     -8,     -8,     -8,     -8,     -7,     -7,     -7,     -7,     -7,     -6,     -6,     -6,     -6,     -6,     -5,
            -5,     -5,     -5,     -5,     -4,     -4,     -4,     -4,     -4,     -3,     -3,     -3,     -3,     -3,     -2,     -2,
            -2,     -2,     -2,     -1,     -1,     -1,     -1,     -1,      0,      0,      0,      0,      0,      0,      0,      0
    }
};

static VCAM_3ASKIP_IPIPE_GAMMA_CFGTYPE gammaDump = {
    /*nEnable = */0,
    /*eGammaTblSize = */(VCAM_3ASKIP_IPIPE_GAMMA_TABLE_SIZETYPE)3,
    /*nTbl = */0,
    /*eBypassB = */(VCAM_3ASKIP_IPIPE_GAMMA_BYPASSTYPE)0,
    /*eBypassG = */(VCAM_3ASKIP_IPIPE_GAMMA_BYPASSTYPE)0,
    /*eBypassR = */(VCAM_3ASKIP_IPIPE_GAMMA_BYPASSTYPE)0,
    /*pnRedTable =*/
    {
           0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    4,    0,
           4,    0,    0,    0,    4,    0,    0,    0,    4,    0,    4,    0,    8,    0,    0,    0,
           8,    0,    0,    0,    8,    0,    4,    0,   12,    0,    0,    0,   12,    0,    4,    0,
          16,    0,    0,    0,   16,    0,    4,    0,   20,    0,    0,    0,   20,    0,    4,    0,
          24,    0,    4,    0,   28,    0,    4,    0,   32,    0,    8,    0,   40,    0,    8,    0,
          48,    0,    8,    0,   56,    0,    8,    0,   64,    0,    4,    0,   68,    0,    8,    0,
          76,    0,    4,    0,   80,    0,    8,    0,   88,    0,    8,    0,   96,    0,    4,    0,
         100,    0,    8,    0,  108,    0,    8,    0,  116,    0,    4,    0,  120,    0,   12,    0,
        -124,    0,    8,    0, -116,    0,    4,    0, -112,    0,    8,    0, -104,    0,    4,    0,
        -100,    0,    8,    0,  -92,    0,    8,    0,  -84,    0,    8,    0,  -76,    0,    8,    0,
         -68,    0,    8,    0,  -60,    0,    8,    0,  -52,    0,    8,    0,  -44,    0,    8,    0,
         -36,    0,    8,    0,  -28,    0,    8,    0,  -20,    0,    8,    0,  -12,    0,    8,    0,
          -4,    0,    8,    0,    4,    1,    8,    0,   12,    1,    8,    0,   20,    1,    8,    0,
          28,    1,    8,    0,   36,    1,    8,    0,   44,    1,    8,    0,   52,    1,    8,    0,
          60,    1,    8,    0,   68,    1,    8,    0,   76,    1,    4,    0,   80,    1,    8,    0,
          88,    1,    4,    0,   92,    1,    4,    0,   96,    1,    8,    0,  104,    1,    4,    0,
         108,    1,    4,    0,  112,    1,    8,    0,  120,    1,    4,    0,  124,    1,    4,    0,
        -128,    1,    8,    0, -120,    1,    4,    0, -116,    1,    4,    0, -112,    1,    4,    0,
        -108,    1,    4,    0, -104,    1,    4,    0, -100,    1,    4,    0,  -96,    1,    0,    0,
         -96,    1,    4,    0,  -92,    1,    4,    0,  -88,    1,    4,    0,  -84,    1,    4,    0,
         -80,    1,    4,    0,  -76,    1,    4,    0,  -72,    1,    4,    0,  -68,    1,    4,    0,
         -64,    1,    4,    0,  -60,    1,    0,    0,  -60,    1,    4,    0,  -56,    1,    4,    0,
         -52,    1,    0,    0,  -52,    1,    4,    0,  -48,    1,    4,    0,  -44,    1,    0,    0,
         -44,    1,    4,    0,  -40,    1,    4,    0,  -36,    1,    0,    0,  -36,    1,    4,    0,
         -32,    1,    4,    0,  -28,    1,    0,    0,  -28,    1,    0,    0,  -28,    1,    4,    0,
         -24,    1,    0,    0,  -24,    1,    4,    0,  -20,    1,    4,    0,  -16,    1,    0,    0,
         -16,    1,    4,    0,  -12,    1,    4,    0,   -8,    1,    0,    0,   -8,    1,    4,    0,
          -4,    1,    0,    0,   -4,    1,    4,    0,    0,    2,    0,    0,    0,    2,    4,    0,
           4,    2,    4,    0,    8,    2,    0,    0,    8,    2,    4,    0,   12,    2,    0,    0,
          12,    2,    0,    0,   12,    2,    0,    0,   12,    2,    4,    0,   16,    2,    0,    0,
          16,    2,    4,    0,   20,    2,    0,    0,   20,    2,    4,    0,   24,    2,    4,    0,
          28,    2,    0,    0,   28,    2,    4,    0,   32,    2,    0,    0,   32,    2,    4,    0,
          36,    2,    0,    0,   36,    2,    0,    0,   36,    2,    4,    0,   40,    2,    0,    0,
          40,    2,    4,    0,   44,    2,    0,    0,   44,    2,    4,    0,   48,    2,    0,    0,
          48,    2,    4,    0,   52,    2,    0,    0,   52,    2,    4,    0,   56,    2,    0,    0,
          56,    2,    4,    0,   60,    2,    0,    0,   60,    2,    4,    0,   64,    2,    0,    0,
          64,    2,    0,    0,   64,    2,    4,    0,   68,    2,    0,    0,   68,    2,    4,    0,
          72,    2,    0,    0,   72,    2,    4,    0,   76,    2,    0,    0,   76,    2,    4,    0,
          80,    2,    0,    0,   80,    2,    0,    0,   80,    2,    4,    0,   84,    2,    0,    0,
          84,    2,    4,    0,   88,    2,    0,    0,   88,    2,    4,    0,   92,    2,    0,    0,
          92,    2,    4,    0,   96,    2,    0,    0,   96,    2,    4,    0,  100,    2,    0,    0,
         100,    2,    0,    0,  100,    2,    4,    0,  104,    2,    0,    0,  104,    2,    4,    0,
         108,    2,    0,    0,  108,    2,    4,    0,  112,    2,    0,    0,  112,    2,    0,    0,
         112,    2,    4,    0,  116,    2,    0,    0,  116,    2,    0,    0,  116,    2,    4,    0,
         120,    2,    0,    0,  120,    2,    4,    0,  124,    2,    0,    0,  124,    2,    4,    0,
        -128,    2,    0,    0, -128,    2,    0,    0, -128,    2,    4,    0, -124,    2,    0,    0,
        -124,    2,    0,    0, -124,    2,    4,    0, -120,    2,    0,    0, -120,    2,    4,    0,
        -116,    2,    0,    0, -116,    2,    0,    0, -116,    2,    4,    0, -112,    2,    0,    0,
        -112,    2,    4,    0, -108,    2,    0,    0, -108,    2,    4,    0, -104,    2,    0,    0,
        -104,    2,    0,    0, -104,    2,    4,    0, -100,    2,    0,    0, -100,    2,    0,    0,
        -100,    2,    4,    0,  -96,    2,    0,    0,  -96,    2,    0,    0,  -96,    2,    4,    0,
         -92,    2,    0,    0,  -92,    2,    4,    0,  -88,    2,    0,    0,  -88,    2,    0,    0,
         -88,    2,    4,    0,  -84,    2,    0,    0,  -84,    2,    0,    0,  -84,    2,    0,    0,
         -84,    2,    4,    0,  -80,    2,    0,    0,  -80,    2,    4,    0,  -76,    2,    0,    0,
         -76,    2,    4,    0,  -72,    2,    0,    0,  -72,    2,    0,    0,  -72,    2,    4,    0,
         -68,    2,    0,    0,  -68,    2,    0,    0,  -68,    2,    4,    0,  -64,    2,    0,    0,
         -64,    2,    0,    0,  -64,    2,    4,    0,  -60,    2,    0,    0,  -60,    2,    0,    0,
         -60,    2,    4,    0,  -56,    2,    0,    0,  -56,    2,    0,    0,  -56,    2,    4,    0,
         -52,    2,    0,    0,  -52,    2,    4,    0,  -48,    2,    0,    0,  -48,    2,    0,    0,
         -48,    2,    4,    0,  -44,    2,    0,    0,  -44,    2,    0,    0,  -44,    2,    4,    0,
         -40,    2,    0,    0,  -40,    2,    0,    0,  -40,    2,    0,    0,  -40,    2,    4,    0,
         -36,    2,    0,    0,  -36,    2,    4,    0,  -32,    2,    0,    0,  -32,    2,    0,    0,
         -32,    2,    4,    0,  -28,    2,    0,    0,  -28,    2,    0,    0,  -28,    2,    4,    0,
         -24,    2,    0,    0,  -24,    2,    0,    0,  -24,    2,    4,    0,  -20,    2,    0,    0//,
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
           0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    4,    0,
           4,    0,    0,    0,    4,    0,    0,    0,    4,    0,    4,    0,    8,    0,    0,    0,
           8,    0,    0,    0,    8,    0,    4,    0,   12,    0,    0,    0,   12,    0,    4,    0,
          16,    0,    0,    0,   16,    0,    4,    0,   20,    0,    0,    0,   20,    0,    4,    0,
          24,    0,    4,    0,   28,    0,    4,    0,   32,    0,    8,    0,   40,    0,    8,    0,
          48,    0,    8,    0,   56,    0,    8,    0,   64,    0,    4,    0,   68,    0,    8,    0,
          76,    0,    4,    0,   80,    0,    8,    0,   88,    0,    8,    0,   96,    0,    4,    0,
         100,    0,    8,    0,  108,    0,    8,    0,  116,    0,    4,    0,  120,    0,   12,    0,
        -124,    0,    8,    0, -116,    0,    4,    0, -112,    0,    8,    0, -104,    0,    4,    0,
        -100,    0,    8,    0,  -92,    0,    8,    0,  -84,    0,    8,    0,  -76,    0,    8,    0,
         -68,    0,    8,    0,  -60,    0,    8,    0,  -52,    0,    8,    0,  -44,    0,    8,    0,
         -36,    0,    8,    0,  -28,    0,    8,    0,  -20,    0,    8,    0,  -12,    0,    8,    0,
          -4,    0,    8,    0,    4,    1,    8,    0,   12,    1,    8,    0,   20,    1,    8,    0,
          28,    1,    8,    0,   36,    1,    8,    0,   44,    1,    8,    0,   52,    1,    8,    0,
          60,    1,    8,    0,   68,    1,    8,    0,   76,    1,    4,    0,   80,    1,    8,    0,
          88,    1,    4,    0,   92,    1,    4,    0,   96,    1,    8,    0,  104,    1,    4,    0,
         108,    1,    4,    0,  112,    1,    8,    0,  120,    1,    4,    0,  124,    1,    4,    0,
        -128,    1,    8,    0, -120,    1,    4,    0, -116,    1,    4,    0, -112,    1,    4,    0,
        -108,    1,    4,    0, -104,    1,    4,    0, -100,    1,    4,    0,  -96,    1,    0,    0,
         -96,    1,    4,    0,  -92,    1,    4,    0,  -88,    1,    4,    0,  -84,    1,    4,    0,
         -80,    1,    4,    0,  -76,    1,    4,    0,  -72,    1,    4,    0,  -68,    1,    4,    0,
         -64,    1,    4,    0,  -60,    1,    0,    0,  -60,    1,    4,    0,  -56,    1,    4,    0,
         -52,    1,    0,    0,  -52,    1,    4,    0,  -48,    1,    4,    0,  -44,    1,    0,    0,
         -44,    1,    4,    0,  -40,    1,    4,    0,  -36,    1,    0,    0,  -36,    1,    4,    0,
         -32,    1,    4,    0,  -28,    1,    0,    0,  -28,    1,    0,    0,  -28,    1,    4,    0,
         -24,    1,    0,    0,  -24,    1,    4,    0,  -20,    1,    4,    0,  -16,    1,    0,    0,
         -16,    1,    4,    0,  -12,    1,    4,    0,   -8,    1,    0,    0,   -8,    1,    4,    0,
          -4,    1,    0,    0,   -4,    1,    4,    0,    0,    2,    0,    0,    0,    2,    4,    0,
           4,    2,    4,    0,    8,    2,    0,    0,    8,    2,    4,    0,   12,    2,    0,    0,
          12,    2,    0,    0,   12,    2,    0,    0,   12,    2,    4,    0,   16,    2,    0,    0,
          16,    2,    4,    0,   20,    2,    0,    0,   20,    2,    4,    0,   24,    2,    4,    0,
          28,    2,    0,    0,   28,    2,    4,    0,   32,    2,    0,    0,   32,    2,    4,    0,
          36,    2,    0,    0,   36,    2,    0,    0,   36,    2,    4,    0,   40,    2,    0,    0,
          40,    2,    4,    0,   44,    2,    0,    0,   44,    2,    4,    0,   48,    2,    0,    0,
          48,    2,    4,    0,   52,    2,    0,    0,   52,    2,    4,    0,   56,    2,    0,    0,
          56,    2,    4,    0,   60,    2,    0,    0,   60,    2,    4,    0,   64,    2,    0,    0,
          64,    2,    0,    0,   64,    2,    4,    0,   68,    2,    0,    0,   68,    2,    4,    0,
          72,    2,    0,    0,   72,    2,    4,    0,   76,    2,    0,    0,   76,    2,    4,    0,
          80,    2,    0,    0,   80,    2,    0,    0,   80,    2,    4,    0,   84,    2,    0,    0,
          84,    2,    4,    0,   88,    2,    0,    0,   88,    2,    4,    0,   92,    2,    0,    0,
          92,    2,    4,    0,   96,    2,    0,    0,   96,    2,    4,    0,  100,    2,    0,    0,
         100,    2,    0,    0,  100,    2,    4,    0,  104,    2,    0,    0,  104,    2,    4,    0,
         108,    2,    0,    0,  108,    2,    4,    0,  112,    2,    0,    0,  112,    2,    0,    0,
         112,    2,    4,    0,  116,    2,    0,    0,  116,    2,    0,    0,  116,    2,    4,    0,
         120,    2,    0,    0,  120,    2,    4,    0,  124,    2,    0,    0,  124,    2,    4,    0,
        -128,    2,    0,    0, -128,    2,    0,    0, -128,    2,    4,    0, -124,    2,    0,    0,
        -124,    2,    0,    0, -124,    2,    4,    0, -120,    2,    0,    0, -120,    2,    4,    0,
        -116,    2,    0,    0, -116,    2,    0,    0, -116,    2,    4,    0, -112,    2,    0,    0,
        -112,    2,    4,    0, -108,    2,    0,    0, -108,    2,    4,    0, -104,    2,    0,    0,
        -104,    2,    0,    0, -104,    2,    4,    0, -100,    2,    0,    0, -100,    2,    0,    0,
        -100,    2,    4,    0,  -96,    2,    0,    0,  -96,    2,    0,    0,  -96,    2,    4,    0,
         -92,    2,    0,    0,  -92,    2,    4,    0,  -88,    2,    0,    0,  -88,    2,    0,    0,
         -88,    2,    4,    0,  -84,    2,    0,    0,  -84,    2,    0,    0,  -84,    2,    0,    0,
         -84,    2,    4,    0,  -80,    2,    0,    0,  -80,    2,    4,    0,  -76,    2,    0,    0,
         -76,    2,    4,    0,  -72,    2,    0,    0,  -72,    2,    0,    0,  -72,    2,    4,    0,
         -68,    2,    0,    0,  -68,    2,    0,    0,  -68,    2,    4,    0,  -64,    2,    0,    0,
         -64,    2,    0,    0,  -64,    2,    4,    0,  -60,    2,    0,    0,  -60,    2,    0,    0,
         -60,    2,    4,    0,  -56,    2,    0,    0,  -56,    2,    0,    0,  -56,    2,    4,    0,
         -52,    2,    0,    0,  -52,    2,    4,    0,  -48,    2,    0,    0,  -48,    2,    0,    0,
         -48,    2,    4,    0,  -44,    2,    0,    0,  -44,    2,    0,    0,  -44,    2,    4,    0,
         -40,    2,    0,    0,  -40,    2,    0,    0,  -40,    2,    0,    0,  -40,    2,    4,    0,
         -36,    2,    0,    0,  -36,    2,    4,    0,  -32,    2,    0,    0,  -32,    2,    0,    0,
         -32,    2,    4,    0,  -28,    2,    0,    0,  -28,    2,    0,    0,  -28,    2,    4,    0,
         -24,    2,    0,    0,  -24,    2,    0,    0,  -24,    2,    4,    0,  -20,    2,    0,    0//,
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
           0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    4,    0,
           4,    0,    0,    0,    4,    0,    0,    0,    4,    0,    4,    0,    8,    0,    0,    0,
           8,    0,    0,    0,    8,    0,    4,    0,   12,    0,    0,    0,   12,    0,    4,    0,
          16,    0,    0,    0,   16,    0,    4,    0,   20,    0,    0,    0,   20,    0,    4,    0,
          24,    0,    4,    0,   28,    0,    4,    0,   32,    0,    8,    0,   40,    0,    8,    0,
          48,    0,    8,    0,   56,    0,    8,    0,   64,    0,    4,    0,   68,    0,    8,    0,
          76,    0,    4,    0,   80,    0,    8,    0,   88,    0,    8,    0,   96,    0,    4,    0,
         100,    0,    8,    0,  108,    0,    8,    0,  116,    0,    4,    0,  120,    0,   12,    0,
        -124,    0,    8,    0, -116,    0,    4,    0, -112,    0,    8,    0, -104,    0,    4,    0,
        -100,    0,    8,    0,  -92,    0,    8,    0,  -84,    0,    8,    0,  -76,    0,    8,    0,
         -68,    0,    8,    0,  -60,    0,    8,    0,  -52,    0,    8,    0,  -44,    0,    8,    0,
         -36,    0,    8,    0,  -28,    0,    8,    0,  -20,    0,    8,    0,  -12,    0,    8,    0,
          -4,    0,    8,    0,    4,    1,    8,    0,   12,    1,    8,    0,   20,    1,    8,    0,
          28,    1,    8,    0,   36,    1,    8,    0,   44,    1,    8,    0,   52,    1,    8,    0,
          60,    1,    8,    0,   68,    1,    8,    0,   76,    1,    4,    0,   80,    1,    8,    0,
          88,    1,    4,    0,   92,    1,    4,    0,   96,    1,    8,    0,  104,    1,    4,    0,
         108,    1,    4,    0,  112,    1,    8,    0,  120,    1,    4,    0,  124,    1,    4,    0,
        -128,    1,    8,    0, -120,    1,    4,    0, -116,    1,    4,    0, -112,    1,    4,    0,
        -108,    1,    4,    0, -104,    1,    4,    0, -100,    1,    4,    0,  -96,    1,    0,    0,
         -96,    1,    4,    0,  -92,    1,    4,    0,  -88,    1,    4,    0,  -84,    1,    4,    0,
         -80,    1,    4,    0,  -76,    1,    4,    0,  -72,    1,    4,    0,  -68,    1,    4,    0,
         -64,    1,    4,    0,  -60,    1,    0,    0,  -60,    1,    4,    0,  -56,    1,    4,    0,
         -52,    1,    0,    0,  -52,    1,    4,    0,  -48,    1,    4,    0,  -44,    1,    0,    0,
         -44,    1,    4,    0,  -40,    1,    4,    0,  -36,    1,    0,    0,  -36,    1,    4,    0,
         -32,    1,    4,    0,  -28,    1,    0,    0,  -28,    1,    0,    0,  -28,    1,    4,    0,
         -24,    1,    0,    0,  -24,    1,    4,    0,  -20,    1,    4,    0,  -16,    1,    0,    0,
         -16,    1,    4,    0,  -12,    1,    4,    0,   -8,    1,    0,    0,   -8,    1,    4,    0,
          -4,    1,    0,    0,   -4,    1,    4,    0,    0,    2,    0,    0,    0,    2,    4,    0,
           4,    2,    4,    0,    8,    2,    0,    0,    8,    2,    4,    0,   12,    2,    0,    0,
          12,    2,    0,    0,   12,    2,    0,    0,   12,    2,    4,    0,   16,    2,    0,    0,
          16,    2,    4,    0,   20,    2,    0,    0,   20,    2,    4,    0,   24,    2,    4,    0,
          28,    2,    0,    0,   28,    2,    4,    0,   32,    2,    0,    0,   32,    2,    4,    0,
          36,    2,    0,    0,   36,    2,    0,    0,   36,    2,    4,    0,   40,    2,    0,    0,
          40,    2,    4,    0,   44,    2,    0,    0,   44,    2,    4,    0,   48,    2,    0,    0,
          48,    2,    4,    0,   52,    2,    0,    0,   52,    2,    4,    0,   56,    2,    0,    0,
          56,    2,    4,    0,   60,    2,    0,    0,   60,    2,    4,    0,   64,    2,    0,    0,
          64,    2,    0,    0,   64,    2,    4,    0,   68,    2,    0,    0,   68,    2,    4,    0,
          72,    2,    0,    0,   72,    2,    4,    0,   76,    2,    0,    0,   76,    2,    4,    0,
          80,    2,    0,    0,   80,    2,    0,    0,   80,    2,    4,    0,   84,    2,    0,    0,
          84,    2,    4,    0,   88,    2,    0,    0,   88,    2,    4,    0,   92,    2,    0,    0,
          92,    2,    4,    0,   96,    2,    0,    0,   96,    2,    4,    0,  100,    2,    0,    0,
         100,    2,    0,    0,  100,    2,    4,    0,  104,    2,    0,    0,  104,    2,    4,    0,
         108,    2,    0,    0,  108,    2,    4,    0,  112,    2,    0,    0,  112,    2,    0,    0,
         112,    2,    4,    0,  116,    2,    0,    0,  116,    2,    0,    0,  116,    2,    4,    0,
         120,    2,    0,    0,  120,    2,    4,    0,  124,    2,    0,    0,  124,    2,    4,    0,
        -128,    2,    0,    0, -128,    2,    0,    0, -128,    2,    4,    0, -124,    2,    0,    0,
        -124,    2,    0,    0, -124,    2,    4,    0, -120,    2,    0,    0, -120,    2,    4,    0,
        -116,    2,    0,    0, -116,    2,    0,    0, -116,    2,    4,    0, -112,    2,    0,    0,
        -112,    2,    4,    0, -108,    2,    0,    0, -108,    2,    4,    0, -104,    2,    0,    0,
        -104,    2,    0,    0, -104,    2,    4,    0, -100,    2,    0,    0, -100,    2,    0,    0,
        -100,    2,    4,    0,  -96,    2,    0,    0,  -96,    2,    0,    0,  -96,    2,    4,    0,
         -92,    2,    0,    0,  -92,    2,    4,    0,  -88,    2,    0,    0,  -88,    2,    0,    0,
         -88,    2,    4,    0,  -84,    2,    0,    0,  -84,    2,    0,    0,  -84,    2,    0,    0,
         -84,    2,    4,    0,  -80,    2,    0,    0,  -80,    2,    4,    0,  -76,    2,    0,    0,
         -76,    2,    4,    0,  -72,    2,    0,    0,  -72,    2,    0,    0,  -72,    2,    4,    0,
         -68,    2,    0,    0,  -68,    2,    0,    0,  -68,    2,    4,    0,  -64,    2,    0,    0,
         -64,    2,    0,    0,  -64,    2,    4,    0,  -60,    2,    0,    0,  -60,    2,    0,    0,
         -60,    2,    4,    0,  -56,    2,    0,    0,  -56,    2,    0,    0,  -56,    2,    4,    0,
         -52,    2,    0,    0,  -52,    2,    4,    0,  -48,    2,    0,    0,  -48,    2,    0,    0,
         -48,    2,    4,    0,  -44,    2,    0,    0,  -44,    2,    0,    0,  -44,    2,    4,    0,
         -40,    2,    0,    0,  -40,    2,    0,    0,  -40,    2,    0,    0,  -40,    2,    4,    0,
         -36,    2,    0,    0,  -36,    2,    4,    0,  -32,    2,    0,    0,  -32,    2,    0,    0
        //,
//         -32,    2,    4,    0,  -28,    2,    0,    0,  -28,    2,    0,    0,  -28,    2,    4,    0,
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
    /*nTyp = */(VCAM_3ASKIP_IPIPE_GBCE_METHODTYPE)1,
    /*LookupTable =*/
    {
        0x0000, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0004, 0x0005, 0x0005, 0x0006, 0x0006, 0x0007, 0x0007, 0x0008,
        0x0008, 0x0009, 0x0009, 0x000a, 0x000a, 0x000b, 0x000b, 0x000c, 0x000c, 0x000d, 0x000d, 0x000e, 0x000e, 0x000f, 0x000f, 0x0010,
        0x0010, 0x0011, 0x0011, 0x0012, 0x0012, 0x0013, 0x0013, 0x0014, 0x0014, 0x0015, 0x0015, 0x0016, 0x0016, 0x0017, 0x0017, 0x0018,
        0x0018, 0x0019, 0x0019, 0x001a, 0x001a, 0x001b, 0x001b, 0x001c, 0x001c, 0x001d, 0x001d, 0x001e, 0x001e, 0x001f, 0x001f, 0x0020,
        0x0020, 0x0021, 0x0021, 0x0022, 0x0022, 0x0023, 0x0023, 0x0024, 0x0024, 0x0025, 0x0025, 0x0026, 0x0026, 0x0027, 0x0027, 0x0028,
        0x0028, 0x0029, 0x0029, 0x002a, 0x002a, 0x002b, 0x002b, 0x002c, 0x002c, 0x002d, 0x002d, 0x002e, 0x002e, 0x002f, 0x002f, 0x0030,
        0x0030, 0x0031, 0x0031, 0x0032, 0x0032, 0x0033, 0x0033, 0x0034, 0x0034, 0x0035, 0x0035, 0x0036, 0x0036, 0x0037, 0x0037, 0x0038,
        0x0038, 0x0039, 0x0039, 0x003a, 0x003a, 0x003b, 0x003b, 0x003c, 0x003c, 0x003c, 0x003c, 0x003c, 0x003c, 0x003d, 0x003d, 0x003d,
        0x003d, 0x003d, 0x003e, 0x003e, 0x003e, 0x003e, 0x003f, 0x003f, 0x003f, 0x003f, 0x003f, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040,
        0x0041, 0x0041, 0x0041, 0x0041, 0x0042, 0x0042, 0x0042, 0x0042, 0x0042, 0x0043, 0x0043, 0x0043, 0x0043, 0x0044, 0x0044, 0x0044,
        0x0044, 0x0044, 0x0045, 0x0045, 0x0045, 0x0045, 0x0045, 0x0046, 0x0046, 0x0046, 0x0046, 0x0047, 0x0047, 0x0047, 0x0047, 0x0047,
        0x0048, 0x0048, 0x0048, 0x0048, 0x0049, 0x0049, 0x0049, 0x0049, 0x0049, 0x004a, 0x004a, 0x004a, 0x004a, 0x004a, 0x004b, 0x004b,
        0x004b, 0x004b, 0x004c, 0x004c, 0x004c, 0x004c, 0x004c, 0x004d, 0x004d, 0x004d, 0x004d, 0x004e, 0x004e, 0x004e, 0x004e, 0x004e,
        0x004f, 0x004f, 0x004f, 0x004f, 0x004f, 0x0050, 0x0050, 0x0050, 0x0050, 0x0051, 0x0051, 0x0051, 0x0051, 0x0051, 0x0052, 0x0052,
        0x0052, 0x0052, 0x0053, 0x0053, 0x0053, 0x0053, 0x0053, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 0x0055, 0x0055, 0x0055, 0x0055,
        0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0057, 0x0057, 0x0057, 0x0057, 0x0057, 0x0058, 0x0058, 0x0058, 0x0058, 0x0059, 0x0059,
        0x0059, 0x0059, 0x0059, 0x005a, 0x005a, 0x005a, 0x005a, 0x005b, 0x005b, 0x005b, 0x005b, 0x005b, 0x005c, 0x005c, 0x005c, 0x005c,
        0x005c, 0x005d, 0x005d, 0x005d, 0x005d, 0x005e, 0x005e, 0x005e, 0x005e, 0x005e, 0x005f, 0x005f, 0x005f, 0x005f, 0x0060, 0x0060,
        0x0060, 0x0060, 0x0060, 0x0061, 0x0061, 0x0061, 0x0061, 0x0061, 0x0062, 0x0062, 0x0062, 0x0062, 0x0063, 0x0063, 0x0063, 0x0063,
        0x0063, 0x0064, 0x0064, 0x0064, 0x0064, 0x0065, 0x0065, 0x0065, 0x0065, 0x0065, 0x0066, 0x0066, 0x0066, 0x0066, 0x0066, 0x0067,
        0x0067, 0x0067, 0x0067, 0x0068, 0x0068, 0x0068, 0x0068, 0x0068, 0x0069, 0x0069, 0x0069, 0x0069, 0x006a, 0x006a, 0x006a, 0x006a,
        0x006a, 0x006b, 0x006b, 0x006b, 0x006b, 0x006b, 0x006c, 0x006c, 0x006c, 0x006c, 0x006d, 0x006d, 0x006d, 0x006d, 0x006d, 0x006e,
        0x006e, 0x006e, 0x006e, 0x006e, 0x006f, 0x006f, 0x006f, 0x006f, 0x0070, 0x0070, 0x0070, 0x0070, 0x0070, 0x0071, 0x0071, 0x0071,
        0x0071, 0x0072, 0x0072, 0x0072, 0x0072, 0x0072, 0x0073, 0x0073, 0x0073, 0x0073, 0x0073, 0x0074, 0x0074, 0x0074, 0x0074, 0x0075,
        0x0075, 0x0075, 0x0075, 0x0075, 0x0076, 0x0076, 0x0076, 0x0076, 0x0077, 0x0077, 0x0077, 0x0077, 0x0077, 0x0078, 0x0078, 0x0078,
        0x0078, 0x0078, 0x0079, 0x0079, 0x0079, 0x0079, 0x007a, 0x007a, 0x007a, 0x007a, 0x007a, 0x007b, 0x007b, 0x007b, 0x007b, 0x007c,
        0x007c, 0x007c, 0x007c, 0x007c, 0x007d, 0x007d, 0x007d, 0x007d, 0x007d, 0x007e, 0x007e, 0x007e, 0x007e, 0x007f, 0x007f, 0x007f,
        0x007f, 0x007f, 0x0080, 0x0080, 0x0080, 0x0080, 0x0081, 0x0081, 0x0081, 0x0081, 0x0081, 0x0082, 0x0082, 0x0082, 0x0082, 0x0082,
        0x0083, 0x0083, 0x0083, 0x0083, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0085, 0x0085, 0x0085, 0x0085, 0x0085, 0x0086, 0x0086,
        0x0086, 0x0086, 0x0087, 0x0087, 0x0087, 0x0087, 0x0087, 0x0088, 0x0088, 0x0088, 0x0088, 0x0089, 0x0089, 0x0089, 0x0089, 0x0089,
        0x008a, 0x008a, 0x008a, 0x008a, 0x008a, 0x008b, 0x008b, 0x008b, 0x008b, 0x008c, 0x008c, 0x008c, 0x008c, 0x008c, 0x008d, 0x008d,
        0x008d, 0x008d, 0x008e, 0x008e, 0x008e, 0x008e, 0x008e, 0x008f, 0x008f, 0x008f, 0x008f, 0x008f, 0x0090, 0x0090, 0x0090, 0x0090,
        0x0091, 0x0091, 0x0091, 0x0091, 0x0091, 0x0092, 0x0092, 0x0092, 0x0092, 0x0093, 0x0093, 0x0093, 0x0093, 0x0093, 0x0094, 0x0094,
        0x0094, 0x0094, 0x0094, 0x0095, 0x0095, 0x0095, 0x0095, 0x0096, 0x0096, 0x0096, 0x0096, 0x0096, 0x0097, 0x0097, 0x0097, 0x0097,
        0x0098, 0x0098, 0x0098, 0x0098, 0x0098, 0x0099, 0x0099, 0x0099, 0x0099, 0x0099, 0x009a, 0x009a, 0x009a, 0x009a, 0x009b, 0x009b,
        0x009b, 0x009b, 0x009b, 0x009c, 0x009c, 0x009c, 0x009c, 0x009c, 0x009d, 0x009d, 0x009d, 0x009d, 0x009e, 0x009e, 0x009e, 0x009e,
        0x009e, 0x009f, 0x009f, 0x009f, 0x009f, 0x00a0, 0x00a0, 0x00a0, 0x00a0, 0x00a0, 0x00a1, 0x00a1, 0x00a1, 0x00a1, 0x00a1, 0x00a2,
        0x00a2, 0x00a2, 0x00a2, 0x00a3, 0x00a3, 0x00a3, 0x00a3, 0x00a3, 0x00a4, 0x00a4, 0x00a4, 0x00a4, 0x00a5, 0x00a5, 0x00a5, 0x00a5,
        0x00a5, 0x00a6, 0x00a6, 0x00a6, 0x00a6, 0x00a6, 0x00a7, 0x00a7, 0x00a7, 0x00a7, 0x00a8, 0x00a8, 0x00a8, 0x00a8, 0x00a8, 0x00a9,
        0x00a9, 0x00a9, 0x00a9, 0x00aa, 0x00aa, 0x00aa, 0x00aa, 0x00aa, 0x00ab, 0x00ab, 0x00ab, 0x00ab, 0x00ab, 0x00ac, 0x00ac, 0x00ac,
        0x00ac, 0x00ad, 0x00ad, 0x00ad, 0x00ad, 0x00ad, 0x00ae, 0x00ae, 0x00ae, 0x00ae, 0x00af, 0x00af, 0x00af, 0x00af, 0x00af, 0x00b0,
        0x00b0, 0x00b0, 0x00b0, 0x00b0, 0x00b1, 0x00b1, 0x00b1, 0x00b1, 0x00b2, 0x00b2, 0x00b2, 0x00b2, 0x00b2, 0x00b3, 0x00b3, 0x00b3,
        0x00b3, 0x00b4, 0x00b4, 0x00b4, 0x00b4, 0x00b4, 0x00b5, 0x00b5, 0x00b5, 0x00b5, 0x00b5, 0x00b6, 0x00b6, 0x00b6, 0x00b6, 0x00b7,
        0x00b7, 0x00b7, 0x00b7, 0x00b7, 0x00b8, 0x00b8, 0x00b8, 0x00b8, 0x00b8, 0x00b9, 0x00b9, 0x00b9, 0x00b9, 0x00ba, 0x00ba, 0x00ba,
        0x00ba, 0x00ba, 0x00bb, 0x00bb, 0x00bb, 0x00bb, 0x00bc, 0x00bc, 0x00bc, 0x00bc, 0x00bc, 0x00bd, 0x00bd, 0x00bd, 0x00bd, 0x00bd,
        0x00be, 0x00be, 0x00be, 0x00be, 0x00bf, 0x00bf, 0x00bf, 0x00bf, 0x00bf, 0x00c0, 0x00c0, 0x00c0, 0x00c0, 0x00c1, 0x00c1, 0x00c1,
        0x00c1, 0x00c1, 0x00c2, 0x00c2, 0x00c2, 0x00c2, 0x00c2, 0x00c3, 0x00c3, 0x00c3, 0x00c3, 0x00c4, 0x00c4, 0x00c4, 0x00c4, 0x00c4,
        0x00c5, 0x00c5, 0x00c5, 0x00c5, 0x00c6, 0x00c6, 0x00c6, 0x00c6, 0x00c6, 0x00c7, 0x00c7, 0x00c7, 0x00c7, 0x00c7, 0x00c8, 0x00c8,
        0x00c8, 0x00c8, 0x00c9, 0x00c9, 0x00c9, 0x00c9, 0x00c9, 0x00ca, 0x00ca, 0x00ca, 0x00ca, 0x00cb, 0x00cb, 0x00cb, 0x00cb, 0x00cb,
        0x00cc, 0x00cc, 0x00cc, 0x00cc, 0x00cc, 0x00cd, 0x00cd, 0x00cd, 0x00cd, 0x00ce, 0x00ce, 0x00ce, 0x00ce, 0x00ce, 0x00cf, 0x00cf,
        0x00cf, 0x00cf, 0x00cf, 0x00d0, 0x00d0, 0x00d0, 0x00d0, 0x00d1, 0x00d1, 0x00d1, 0x00d1, 0x00d1, 0x00d2, 0x00d2, 0x00d2, 0x00d2,
        0x00d3, 0x00d3, 0x00d3, 0x00d3, 0x00d3, 0x00d4, 0x00d4, 0x00d4, 0x00d4, 0x00d4, 0x00d5, 0x00d5, 0x00d5, 0x00d5, 0x00d6, 0x00d6,
        0x00d6, 0x00d6, 0x00d6, 0x00d7, 0x00d7, 0x00d7, 0x00d7, 0x00d8, 0x00d8, 0x00d8, 0x00d8, 0x00d8, 0x00d9, 0x00d9, 0x00d9, 0x00d9,
        0x00d9, 0x00da, 0x00da, 0x00da, 0x00da, 0x00db, 0x00db, 0x00db, 0x00db, 0x00db, 0x00dc, 0x00dc, 0x00dc, 0x00dc, 0x00dd, 0x00dd,
        0x00dd, 0x00dd, 0x00dd, 0x00de, 0x00de, 0x00de, 0x00de, 0x00de, 0x00df, 0x00df, 0x00df, 0x00df, 0x00e0, 0x00e0, 0x00e0, 0x00e0,
        0x00e0, 0x00e1, 0x00e1, 0x00e1, 0x00e1, 0x00e2, 0x00e2, 0x00e2, 0x00e2, 0x00e2, 0x00e3, 0x00e3, 0x00e3, 0x00e3, 0x00e3, 0x00e4,
        0x00e4, 0x00e4, 0x00e4, 0x00e5, 0x00e5, 0x00e5, 0x00e5, 0x00e5, 0x00e6, 0x00e6, 0x00e6, 0x00e6, 0x00e6, 0x00e7, 0x00e7, 0x00e7,
        0x00e7, 0x00e8, 0x00e8, 0x00e8, 0x00e8, 0x00e8, 0x00e9, 0x00e9, 0x00e9, 0x00e9, 0x00ea, 0x00ea, 0x00ea, 0x00ea, 0x00ea, 0x00eb,
        0x00eb, 0x00eb, 0x00eb, 0x00eb, 0x00ec, 0x00ec, 0x00ec, 0x00ec, 0x00ed, 0x00ed, 0x00ed, 0x00ed, 0x00ed, 0x00ee, 0x00ee, 0x00ee,
        0x00ee, 0x00ef, 0x00ef, 0x00ef, 0x00ef, 0x00ef, 0x00f0, 0x00f0, 0x00f0, 0x00f0, 0x00f0, 0x00f1, 0x00f1, 0x00f1, 0x00f1, 0x00f2,
        0x00f2, 0x00f2, 0x00f2, 0x00f2, 0x00f3, 0x00f3, 0x00f3, 0x00f3, 0x00f4, 0x00f4, 0x00f4, 0x00f4, 0x00f4, 0x00f5, 0x00f5, 0x00f5,
        0x00f5, 0x00f5, 0x00f6, 0x00f6, 0x00f6, 0x00f6, 0x00f7, 0x00f7, 0x00f7, 0x00f7, 0x00f7, 0x00f8, 0x00f8, 0x00f8, 0x00f8, 0x00f9,
        0x00f9, 0x00f9, 0x00f9, 0x00f9, 0x00fa, 0x00fa, 0x00fa, 0x00fa, 0x00fa, 0x00fb, 0x00fb, 0x00fb, 0x00fb, 0x00fc, 0x00fc, 0x00fc,
        0x00fc, 0x00fc, 0x00fd, 0x00fd, 0x00fd, 0x00fd, 0x00fd, 0x00fe, 0x00fe, 0x00fe, 0x00fe, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff
    }
};
