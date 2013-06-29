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

#include <VisionCamTest_3A_Data.h>
#include <Params3Adump.h>

void startExported3A_Menu( void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    entryIndex index = -1;

    if( vCam )
    {
        while( 1 )
        {
            index = Menu( MenuManual3A );
            if( index == -1 )
            {
                continue;
            }

            /// 'q' button (quit) is pressed
            if( MenuManual3A[ index ]->ID == VCAM_CMD_QUIT )
            {
                break;
            }
            executeEntry( MenuManual3A[ index ] , vCam );
        }
    }
}

void getData(void *out, void *in, size_t size)
{
    if( out && in && size )
    {
        if( bZero )
        {
            memset(out, 0, size );
            puts("\nZeroing ENABLED !!!");
        }
        else
        {
            puts("\nZeroing DISABLED !!!");
            memcpy(out, in, size );
        }
    }
}

bool_e getData(void *data, size_t size, const char *testFileName )
{
    bool_e ret = false_e;

    FILE *f = NULL;
    f = fopen(testFileName, "rb");
    if( f )
    {
        size_t fSize = 0;
        fseek(f, 0L, SEEK_END);
        fSize = (size_t)ftell(f);
        fseek(f, 0L, SEEK_SET);

        if( size == fSize )
        {
            fread(data, 1, fSize, f);
            ret = true_e;
        }
        fclose(f);
    }
    else
    {
        puts(__FILE__);
        printf("ERROR: file %s does not exist ! \n", testFileName);
        ret = false_e;
    }

    return ret;
}

void man3AsensorFunc(void *input __attribute__((unused)) )
{
    int sensorSelOpts[] = {
        VCAM_SENSOR_PRIMARY,
        VCAM_SENSOR_SECONDARY,
        VCAM_SENSOR_STEREO,
    };

    const char * sensorSelStrings[] = {
        "Primary Sensor",
        "Secondary Sensor",
        "Stereo Sensors"
    };

    subMenuEnrty sensorSelSubMenu = {
        sensorSelOpts,
        sensorSelStrings
    };

    menuEnrty sen;
    memcpy(&sen, &man3Asensor, sizeof(menuEnrty));
    sen.subMenu = (void*)&sensorSelSubMenu;

    eSensor = (VisionCamSensorSelection)getValue((const menuEnrty*)&sen);
}

void lsc2dFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_2D_LscConfig_t*)lsc2D.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_2D_LscConfig_t*)lsc2D.localValue)->m2D_LscConfig),
                (void*)&lsc2dDump,
                sizeof(VCAM_3ASKIP_ISIF_2DLSC_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Lsc2D;
    param.pData = lsc2D.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param) );
}

void clampFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_Clamp_t*)(clamp.localValue))->eSensor = eSensor;
    getData(    &(((VisionCam_3A_Clamp_t*)(clamp.localValue))->mClamp),
                (void*)&clampDump,
                sizeof(VCAM_3ASKIP_ISIF_CLAMP_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Clamp;
    param.pData = clamp.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void gainOffsetFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_GainOffset_t*)(gainOffset.localValue))->eSensor = eSensor;
    getData(    &(((VisionCam_3A_GainOffset_t*)(gainOffset.localValue))->mGainOffset),
                (void*)&gainOffsetDump,
                sizeof(VCAM_3ASKIP_ISIF_GAINOFFSET_CFGTYPE)
            );

    param.eParamType = VCAM_3A_GainOffset;
    param.pData = gainOffset.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void vldcFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_VDLC_t*)vldc.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_VDLC_t*)vldc.localValue)->mVLDC),
                (void*)&vldcDump,
                sizeof(VCAM_3ASKIP_ISIF_VLDC_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Vldc;
    param.pData = vldc.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void nf1Func(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_NoiseFilterConfig_t*)nf1.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_NoiseFilterConfig_t*)nf1.localValue)->eNoiseFilterConfig),
                (void*)&nf1Dump,
                sizeof(VCAM_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Nf1;
    param.pData = nf1.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void nf2Func(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_NoiseFilterConfig_t*)nf2.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_NoiseFilterConfig_t*)nf2.localValue)->eNoiseFilterConfig),
                (void*)&nf2Dump,
                sizeof(VCAM_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Nf2;
    param.pData = nf2.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void gicFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_GIC_Config_t*)gic.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_GIC_Config_t*)gic.localValue)->mGIC_config),
                (void*)&gicDump,
                sizeof(VCAM_3ASKIP_IPIPE_GIC_CFGTYPE)
            );

    param.eParamType = VCAM_3A_GIC;
    param.pData = gic.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void wbFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_WB_Config_t*)wb.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_WB_Config_t*)wb.localValue)->eWB_config),
                (void*)&wbDump,
                sizeof(VCAM_3ASKIP_IPIPE_WB_CFGTYPE)
            );

    param.eParamType = VCAM_3A_WB;
    param.pData = wb.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void cfaFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_CFA_mode_t*)cfa.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_CFA_Config_t*)cfa.localValue)->mCFA_config),
                (void*)&cfaDump,
                sizeof(VCAM_3ASKIP_IPIPE_CFA_CFGTYPE)
            );

    param.eParamType = VCAM_3A_CFA;
    param.pData = cfa.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void gammaFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_GammaTableConfig_t*)manualGamma.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_GammaTableConfig_t*)manualGamma.localValue)->mGammaTable),
                &gammaDump,
                sizeof(VCAM_3ASKIP_IPIPE_GAMMA_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Gamma;
    param.pData = manualGamma.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void rgb2rgb1Func(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_RGB2RGB_t*)rgb2rgb_1.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_RGB2RGB_t*)rgb2rgb_1.localValue)->mRGB_config),
                &rgb2rgb1Dump,
                sizeof(VCAM_3ASKIP_IPIPE_RGBRGB_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Rgb2Rgb1;
    param.pData = rgb2rgb_1.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void rgb2rgb2Func(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_RGB2RGB_t*)rgb2rgb_2.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_RGB2RGB_t*)rgb2rgb_2.localValue)->mRGB_config),
                &rgb2rgb2Dump,
                sizeof(VCAM_3ASKIP_IPIPE_RGBRGB_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Rgb2Rgb2;
    param.pData = rgb2rgb_2.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void rgb2yuvFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_RGB2YUV_t*)rgb2yuv.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_RGB2YUV_t*)rgb2yuv.localValue)->mRGB2YUV),
                &rgb2yuvDump,
                sizeof(VCAM_3ASKIP_IPIPE_RGBYUV_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Rgb2Yuv;
    param.pData = rgb2yuv.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void gbceFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_GBCE_Config_t*)gbce.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_GBCE_Config_t*)gbce.localValue)->mGBCE_config),
                &gbceDump,
                sizeof(VCAM_3ASKIP_IPIPE_GBCE_CFGTYPE)
            );

    param.eParamType = VCAM_3A_GBCE;
    param.pData = gbce.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void yuv2yuvFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_Yuv444_toYuv422_Config_t*)yuv2yuv.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_Yuv444_toYuv422_Config_t*)yuv2yuv.localValue)->mYuvToYuv_config),
                &yuv2yuvDump,
                sizeof(VCAM_3ASKIP_IPIPE_YUV444YUV422_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Yuv2Yuv;
    param.pData = yuv2yuv.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void edgeEnhFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_EdgeEnhancement_config_t*)edgeEnh.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_EdgeEnhancement_config_t*)edgeEnh.localValue)->eEdgeEnhConfig),
                &edgeEnhDump,
                sizeof(VCAM_3ASKIP_IPIPE_EE_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Ee;
    param.pData = edgeEnh.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void carFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_CAR_config_t*)car.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_CAR_config_t*)car.localValue)->mCar),
                &carDump,
                sizeof(VCAM_3ASKIP_IPIPE_CAR_CFGTYPE)
            );

    param.eParamType =VCAM_3A_Car;
    param.pData = car.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void lscFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_LscConfig_t*)lsc.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_LscConfig_t*)lsc.localValue)->mLsc),
                &lscDump,
                sizeof(VCAM_3ASKIP_IPIPE_LSC_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Lsc;
    param.pData = lsc.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void histogramFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_HistogramConfig_t*)histogram.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_HistogramConfig_t*)histogram.localValue)->mHistogramConfig),
                &histDump,
                sizeof(VCAM_3ASKIP_IPIPE_HIST_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Histogram;
    param.pData = histogram.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void boxcarFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_BoxCarConfig_t*)boxCar.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_BoxCarConfig_t*)boxCar.localValue)->eBoxCarConfig),
                &boxCarDump,
                sizeof(VCAM_3ASKIP_IPIPE_BOXCAR_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Boxcar;
    param.pData = boxCar.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void bscFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_BscConfig_t*)bsc.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_BscConfig_t*)bsc.localValue)->mBscConfig),
                &bscDump,
                sizeof(VCAM_3ASKIP_IPIPE_BSC_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Bsc;
    param.pData = bsc.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void f3DccFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_3DCC_config_t*)m3Dcc.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_3DCC_config_t*)m3Dcc.localValue)->m3dccConfig),
                &t3dccDump,
                sizeof(VCAM_3ASKIP_IPIPE_3DCC_CFGTYPE)
            );

    param.eParamType = VCAM_3A_3Dcc;
    param.pData = m3Dcc.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void dpcOtfFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;
{
    dpcOtfDump.nEnable = 0;
    dpcOtfDump.eType = (VCAM_3ASKIP_IPIPE_DPC_OTFTYPE)1;
    dpcOtfDump.eAlgo = (VCAM_3ASKIP_IPIPE_DPC_ALGOTYPE)1;
    dpcOtfDump.tDpcData.tDpc2Params.thr_cor_r = 8;
    dpcOtfDump.tDpcData.tDpc2Params.thr_cor_gr = 8;
    dpcOtfDump.tDpcData.tDpc2Params.thr_cor_gb = 8;
    dpcOtfDump.tDpcData.tDpc2Params.thr_cor_b = 8;
    dpcOtfDump.tDpcData.tDpc2Params.thr_det_r = 8;
    dpcOtfDump.tDpcData.tDpc2Params.thr_det_gr = 8;
    dpcOtfDump.tDpcData.tDpc2Params.thr_det_gb = 8;
    dpcOtfDump.tDpcData.tDpc2Params.thr_det_b = 8;
    dpcOtfDump.tDpcData.tDpc3Params.eShift = 1;
    dpcOtfDump.tDpcData.tDpc3Params.eOtfCorr.nDThr = 8;
    dpcOtfDump.tDpcData.tDpc3Params.eOtfCorr.nDSlp = 8;
    dpcOtfDump.tDpcData.tDpc3Params.eOtfCorr.nDMin = 8;
    dpcOtfDump.tDpcData.tDpc3Params.eOtfCorr.nDMax = 8;
    dpcOtfDump.tDpcData.tDpc3Params.eOtfDett.nDThr = 8;
    dpcOtfDump.tDpcData.tDpc3Params.eOtfDett.nDSlp = 8;
    dpcOtfDump.tDpcData.tDpc3Params.eOtfDett.nDMin = 8;
    dpcOtfDump.tDpcData.tDpc3Params.eOtfDett.nDMax = 8;
}
    ((VisionCam_3A_DpcOtfConfig_t*)dpcOtf.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_DpcOtfConfig_t*)dpcOtf.localValue)->mDPC),
                &dpcOtfDump,
                sizeof(VCAM_3ASKIP_IPIPE_DPCOTF_CFGTYPE)
            );

    param.eParamType = VCAM_3A_DpcOtf;
    param.pData = dpcOtf.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void dpcLutFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_DPC_Lut_t*)dpcLut.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_DPC_Lut_t*)dpcLut.localValue)->mDPC_lut),
                &dpcLutDump,
                sizeof(VCAM_3ASKIP_IPIPE_DPCLUT_CFGTYPE)
            );

    param.eParamType = VCAM_3A_DpcLut;
    param.pData = dpcLut.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void cgsFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_CGS_Config_t*)cgs.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_CGS_Config_t*)cgs.localValue)->mCgs),
                &cgsDump,
                sizeof(VCAM_3ASKIP_IPIPE_CGS_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Cgs;
    param.pData = cgs.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void dfsFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_DfsConfig_t*)dfs.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_DfsConfig_t*)dfs.localValue)->mDfsConfig),
                &dfsDump,
                sizeof(VCAM_3ASKIP_IPIPEIF_DFS_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Dfs;
    param.pData = dfs.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void dpc1Func(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_DpcConfig_t*)dpc1.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_DpcConfig_t*)dpc1.localValue)->mDpcConfig),
                &dpc1Dump,
                sizeof(VCAM_3ASKIP_IPIPEIF_DPC_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Dpc1;
    param.pData = dpc1.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void dpc2Func(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_DpcConfig_t*)dpc2.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_DpcConfig_t*)dpc2.localValue)->mDpcConfig),
                &dpc2Dump,
                sizeof(VCAM_3ASKIP_IPIPEIF_DPC_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Dpc2;
    param.pData = dpc2.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void dpcmFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_DPCM_Config_t*)dpcm.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_DPCM_Config_t*)dpcm.localValue)->mDPCM),
                &dpcmDump,
                sizeof(VCAM_3ASKIP_IPIPEIF_DPCM_CFGTYPE)
            );

    param.eParamType = VCAM_3A_Dpcm;
    param.pData = dpcm.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void horLPFfunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_ResizerLowPassFilter_t*)horLPF.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_ResizerLowPassFilter_t*)horLPF.localValue)->mRszLpfConfig),
                &hlpfDump,
                sizeof(VCAM_3ASKIP_RSZ_LPF_CFGTYPE)
            );

    param.eParamType = VCAM_3A_HLpf;
    param.pData = horLPF.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void verLPFfunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_ResizerLowPassFilter_t*)verLPF.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_ResizerLowPassFilter_t*)verLPF.localValue)->mRszLpfConfig),
                &vlpDump,
                sizeof(VCAM_3ASKIP_RSZ_LPF_CFGTYPE)
            );

    param.eParamType = VCAM_3A_VLpf;
    param.pData = verLPF.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void h3aAFfunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_H3A_AutoFocusParam_t*)h3aAF.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_H3A_AutoFocusParam_t*)h3aAF.localValue)->mH3A_AutoFocusParam),
                &h3aAfDump,
                sizeof(VCAM_3ASKIP_H3A_AF_PARAMTYPE)
            );

    param.eParamType = VCAM_3A_H3aAfParams;
    param.pData = h3aAF.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void h3aAewbFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_AEWB_config_t*)h3aAewb.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_AEWB_config_t*)h3aAewb.localValue)->mAEWB_config),
                &h3aAewbDump,
                sizeof(VCAM_3ASKIP_H3A_AEWB_PARAMTYPE)
            );

    param.eParamType = VCAM_3A_H3aAewbParams;
    param.pData = h3aAewb.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void h3aComFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_H3A_CommonConfig_t*)h3aCom.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_H3A_CommonConfig_t*)h3aCom.localValue)->mH3A_CommonConfig),
                &h3aCommonDump,
                sizeof(VCAM_3ASKIP_H3A_COMMON_CFGTYPE)
            );

    param.eParamType = VCAM_3A_H3aCommonParams;
    param.pData = h3aCom.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void expGainCtrlFunc(void *input)
{
    VisionCam *vCam = (VisionCam*)input;
    VisionCam_3Asettings_Base_t param;

    ((VisionCam_3A_ExpGainControl_t*)expGainCtrl.localValue)->eSensor = eSensor;
    getData(    &(((VisionCam_3A_ExpGainControl_t*)expGainCtrl.localValue)->mExpGainControl),
                &expCtrlDump,
                sizeof(VCAM_3ASKIP_CAM_CONTROL_EXPGAINTYPE)
            );

    param.eParamType = VCAM_3A_CamControlExpGain;
    param.pData = expGainCtrl.localValue;

    vCam->setParameter(VCAM_PARAM_EXPORTED_3A_SET, (void*)&param, sizeof(param));
}

void zeroAllStructs( void* input __attribute__((unused)) )
{
    bZero = (bool_e)!bZero;
}
