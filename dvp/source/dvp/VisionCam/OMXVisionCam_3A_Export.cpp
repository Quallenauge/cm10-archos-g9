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

#include <OMXVisionCam_3A_Export.h>
#include <OMX_TI_IVCommon.h>

/// @def HERE - debug print: shows function and line number
#define HERE { printf("\n%s:%d\n", __func__, __LINE__); fflush(stdout); }
//#define return HERE return

/// We have 4 bytes reserved for sensor ID inside configuration data buffer, for each parameter.
#define PTR_OFFSET_32bit(mem) (sizeof(int32_t)/sizeof(mem[0]))

#define ALLOC_LIST_NODE(var, omxType) \
    omxType *var = NULL; \
    var = (omxType*)malloc( sizeof(size_t) + sizeof(OMX_TI_3ASKIP_TI_3ASKIPINDEXTYPE) + sizeof(omxType) );

/// Constructs the buffer data needed by OMX interface
#define CONSTRUCT_DATA(data, ind, sen, par, omx_type)                                   \
{                                                                                       \
    ret = sensorUsed( (int32_t*)((uint32_t)data + sizeof(size_t) ) , sen, ind );        \
    if( STATUS_SUCCESS == ret )                                                         \
    {                                                                                   \
        if( sizeof(par) == sizeof(omx_type) )                                           \
        {   /**  */                                                                     \
            void *dst = (void*)( (uint32_t)data + sizeof(size_t) + sizeof(ind) );       \
            memcpy( dst, &par, sizeof(omx_type ));                                      \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            DVP_PRINT(DVP_ZONE_ERROR, "VisionCam and OMX structures doesn't match.\n"); \
            ret = STATUS_NO_MAPPING; /** Need to be copied manually */                  \
        }                                                                               \
    }                                                                                   \
}


#define ADD_TO_3A_LIST(list, data, list_size ) { \
                list_size += addToList( list, (void*)data, sizeof(*data) + sizeof(OMX_TI_3ASKIP_TI_3ASKIPINDEXTYPE) ); \
                DVP_PRINT(DVP_ZONE_CAM, "Total size of 3A list data: %d bytes. \n", list_size); \
}


/** Mask allpied to 3a_skip_index, to mark left/tight sensor
  * where:
  * right_sensor = index | cSensorMask
  * left_sensor = index & (~cSensorMask)
*/
const int32_t VisionCam_3A_Export::cSensorMask = 0x01000000;

/**
*/
VisionCam_3A_Export::VisionCam_3A_Export()
{
    nAllocatinSize = 0;
    bSettingsHeld = false_e;
    d3A_Buffer = NULL;
    mutex_init(&mLock);
    mSettingList = list_create();

    execSRVC = new VisionCamExecutionService<VisionCam_3A_Export, VisionCam_3A_Export::setFuncPtrType>(this);

    execSRVC->Register( VCAM_3A_Lsc2D               , &VisionCam_3A_Export::Set_3A_Lsc2D            , sizeof(OMX_TI_3ASKIP_ISIF_2DLSC_CFGTYPE)        );
    execSRVC->Register( VCAM_3A_Clamp               , &VisionCam_3A_Export::Set_3A_Clamp            , sizeof(OMX_TI_3ASKIP_ISIF_CLAMP_CFGTYPE)        );
    execSRVC->Register( VCAM_3A_GainOffset          , &VisionCam_3A_Export::Set_3A_GainOffset       , sizeof(OMX_TI_3ASKIP_ISIF_GAINOFFSET_CFGTYPE)   );
    execSRVC->Register( VCAM_3A_Vldc                , &VisionCam_3A_Export::Set_3A_Vlcd             , sizeof(OMX_TI_3ASKIP_ISIF_VLDC_CFGTYPE)         );
    execSRVC->Register( VCAM_3A_Nf1                 , &VisionCam_3A_Export::Set_3A_Nf1              , sizeof(OMX_TI_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE));
    execSRVC->Register( VCAM_3A_Nf2                 , &VisionCam_3A_Export::Set_3A_Nf2              , sizeof(OMX_TI_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE));
    execSRVC->Register( VCAM_3A_GIC                 , &VisionCam_3A_Export::Set_3A_GIC              , sizeof(OMX_TI_3ASKIP_IPIPE_GIC_CFGTYPE)         );
    execSRVC->Register( VCAM_3A_WB                  , &VisionCam_3A_Export::Set_3A_WB               , sizeof(OMX_TI_3ASKIP_IPIPE_WB_CFGTYPE)          );
    execSRVC->Register( VCAM_3A_CFA                 , &VisionCam_3A_Export::Set_3A_CFA              , sizeof(OMX_TI_3ASKIP_IPIPE_CFA_CFGTYPE)         );
    execSRVC->Register( VCAM_3A_Gamma               , &VisionCam_3A_Export::Set_3A_Gamma            , sizeof(OMX_TI_3ASKIP_IPIPE_GAMMA_CFGTYPE)       );
    execSRVC->Register( VCAM_3A_Rgb2Rgb1            , &VisionCam_3A_Export::Set_3A_Rgb2Rgb1         , sizeof(OMX_TI_3ASKIP_IPIPE_RGBRGB_CFGTYPE)      );
    execSRVC->Register( VCAM_3A_Rgb2Rgb2            , &VisionCam_3A_Export::Set_3A_Rgb2Rgb2         , sizeof(OMX_TI_3ASKIP_IPIPE_RGBRGB_CFGTYPE)      );
    execSRVC->Register( VCAM_3A_Rgb2Yuv             , &VisionCam_3A_Export::Set_3A_Rgb2Yuv          , sizeof(OMX_TI_3ASKIP_IPIPE_RGBYUV_CFGTYPE)      );
    execSRVC->Register( VCAM_3A_GBCE                , &VisionCam_3A_Export::Set_3A_GBCE             , sizeof(OMX_TI_3ASKIP_IPIPE_GBCE_CFGTYPE)        );
    execSRVC->Register( VCAM_3A_Yuv2Yuv             , &VisionCam_3A_Export::Set_3A_Yuv2Yuv          , sizeof(OMX_TI_3ASKIP_IPIPE_YUV444YUV422_CFGTYPE));
    execSRVC->Register( VCAM_3A_Ee                  , &VisionCam_3A_Export::Set_3A_Ee               , sizeof(OMX_TI_3ASKIP_IPIPE_EE_CFGTYPE)          );
    execSRVC->Register( VCAM_3A_Car                 , &VisionCam_3A_Export::Set_3A_Car              , sizeof(OMX_TI_3ASKIP_IPIPE_CAR_CFGTYPE)         );
    execSRVC->Register( VCAM_3A_Lsc                 , &VisionCam_3A_Export::Set_3A_Lsc              , sizeof(OMX_TI_3ASKIP_IPIPE_LSC_CFGTYPE)         );
    execSRVC->Register( VCAM_3A_Histogram           , &VisionCam_3A_Export::Set_3A_Histogram        , sizeof(OMX_TI_3ASKIP_IPIPE_HIST_CFGTYPE)        );
    execSRVC->Register( VCAM_3A_Bsc                 , &VisionCam_3A_Export::Set_3A_Bsc              , sizeof(OMX_TI_3ASKIP_IPIPE_BSC_CFGTYPE)         );
    execSRVC->Register( VCAM_3A_DpcOtf              , &VisionCam_3A_Export::Set_3A_DpcOtf           , sizeof(OMX_TI_3ASKIP_IPIPE_DPCOTF_CFGTYPE)      );
    execSRVC->Register( VCAM_3A_Cgs                 , &VisionCam_3A_Export::Set_3A_Cgs              , sizeof(OMX_TI_3ASKIP_IPIPE_CGS_CFGTYPE)         );
    execSRVC->Register( VCAM_3A_Dfs                 , &VisionCam_3A_Export::Set_3A_Dfs              , sizeof(OMX_TI_3ASKIP_IPIPEIF_DFS_CFGTYPE)       );
    execSRVC->Register( VCAM_3A_Dpc1                , &VisionCam_3A_Export::Set_3A_Dpc1             , sizeof(OMX_TI_3ASKIP_IPIPEIF_DPC_CFGTYPE)       );
    execSRVC->Register( VCAM_3A_Dpc2                , &VisionCam_3A_Export::Set_3A_Dpc2             , sizeof(OMX_TI_3ASKIP_IPIPEIF_DPC_CFGTYPE)       );
    execSRVC->Register( VCAM_3A_Dpcm                , &VisionCam_3A_Export::Set_3A_Dpcm             , sizeof(VCAM_3ASKIP_IPIPEIF_DPCM_CFGTYPE)        );
    execSRVC->Register( VCAM_3A_HLpf                , &VisionCam_3A_Export::Set_3A_HLpf             , sizeof(OMX_TI_3ASKIP_RSZ_LPF_CFGTYPE)           );
    execSRVC->Register( VCAM_3A_VLpf                , &VisionCam_3A_Export::Set_3A_VLpf             , sizeof(OMX_TI_3ASKIP_RSZ_LPF_CFGTYPE)           );
    execSRVC->Register( VCAM_3A_H3aCommonParams     , &VisionCam_3A_Export::Set_3A_H3aCommonParams  , sizeof(OMX_TI_3ASKIP_H3A_COMMON_CFGTYPE)        );
    execSRVC->Register( VCAM_3A_CamControlExpGain   , &VisionCam_3A_Export::Set_3A_CamControlExpGain, sizeof(OMX_TI_3ASKIP_CAM_CONTROL_EXPGAINTYPE)   );
#ifdef OLD_3A_EXPORT /// @todo those shold be checked: which are still presens but with a different type names and which are no present at all
    execSRVC->Register( VCAM_3A_3Dcc                , &VisionCam_3A_Export::Set_3A_3Dcc             , sizeof(OMX_TI_3ASKIP_IPIPE_3DCC_CFGTYPE)        );
    execSRVC->Register( VCAM_3A_DpcLut              , &VisionCam_3A_Export::Set_3A_DpcLut           , sizeof(OMX_TI_3ASKIP_IPIPE_DPCLUT_CFGTYPE)      );
    execSRVC->Register( VCAM_3A_Boxcar              , &VisionCam_3A_Export::Set_3A_Boxcar           , sizeof(OMX_TI_3ASKIP_IPIPE_BOXCAR_CFGTYPE)      );
    execSRVC->Register( VCAM_3A_H3aAewbParams       , &VisionCam_3A_Export::Set_3A_H3aAewbParams    , sizeof(OMX_TI_3ASKIP_H3A_AEWB_PARAMTYPE)        );
    execSRVC->Register( VCAM_3A_H3aAfParams         , &VisionCam_3A_Export::Set_3A_H3aAfParams      , sizeof(OMX_TI_3ASKIP_H3A_AF_PARAMTYPE)          );
#endif // OLD_3A_EXPORT
}

VisionCam_3A_Export::~VisionCam_3A_Export()
{
    nAllocatinSize = 0;
    bSettingsHeld = false_e;
    mutex_deinit(&mLock);
    list_destroy( mSettingList );
    delete execSRVC;
}

size_t VisionCam_3A_Export::addToList(list_t *list, void *data, size_t size)
{
    node_t *node = (node_t*)calloc(1, sizeof(node_t));

    ((size_t*)(data))[0] = size;
    DVP_PRINT(DVP_ZONE_CAM, "Appending %d bytes to List\n", (((size_t*)(data))[0]));
    node->data = (value_t)(data);

    list_append(list, node);

    return size;
}

status_e VisionCam_3A_Export::sensorUsed( int32_t *data, int32_t sen, OMX_TI_3ASKIP_TI_3ASKIPINDEXTYPE omxIndex )
{
    status_e ret = STATUS_SUCCESS;

    /// we depend on the requirement that the very first four bytes in any manual 3A configuration,
    /// are used to save OMX index of the configuration and the sensor id ( left or right )
    if( OMX_PrimarySensor == (OMX_SENSORSELECT)sen )
    {
        data[0] = (int32_t)omxIndex & (~cSensorMask);
    }
    else if( OMX_SecondarySensor == (OMX_SENSORSELECT)sen )
    {
        data[0] = (int32_t)omxIndex | cSensorMask;
    }
    else
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Requested stereo sensors configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::hold_3A()
{
    status_e ret = STATUS_SUCCESS;

    SOSAL::AutoLock lock( &mLock);

    if( bSettingsHeld )
    {
        ret = STATUS_INVALID_STATE;
        DVP_PRINT(DVP_ZONE_CAM, "Requested Hold on 3A manual settins, but they are already held.");
        DVP_PRINT(DVP_ZONE_CAM, "Returning STATUS_INVALID_STATE");
    }
    else
    {
        if( d3A_Buffer )
        {
            delete d3A_Buffer;
            d3A_Buffer = NULL;
        }

        bSettingsHeld = true_e;
    }

    return ret;
}

status_e VisionCam_3A_Export::release_3A()
{
    status_e ret = STATUS_SUCCESS;

    SOSAL::AutoLock lock( &mLock);

    if( bSettingsHeld )
    {
        bSettingsHeld = false_e;

        if ( create3Abuffer() )
        {
            collect3AbufferData();
        }
        else
        {
            ret = STATUS_NOT_ENOUGH_MEMORY;
            DVP_PRINT(DVP_ZONE_ERROR, "Couldn't create 3A data buffer.\n");
        }
    }
    else
    {
        ret = STATUS_INVALID_STATE;
        DVP_PRINT(DVP_ZONE_ERROR, "Requested Unhold on 3A manual settins, but they are not held.\n");
        DVP_PRINT(DVP_ZONE_ERROR, "Returning STATUS_INVALID_STATE\n");
    }

    return ret;
}

bool_e VisionCam_3A_Export::create3Abuffer()
{
    bool_e ret = false_e;

    if( false_e == bSettingsHeld )
    {
        if( d3A_Buffer )
        {
            DVP_PRINT(DVP_ZONE_CAM, "Found old 3A Data Buffer.\n Freeing it!\n");
            delete d3A_Buffer;
            DVP_PRINT(DVP_ZONE_CAM, "Old 3A Data Buffer freed.\n");
            d3A_Buffer = NULL;
        }

        if( nAllocatinSize > 0 )
        {
            d3A_Buffer = new DataBuffer_t(nAllocatinSize);

            if( d3A_Buffer && d3A_Buffer->getData() )
            {
                DVP_PRINT(DVP_ZONE_CAM, "New 3A Data Buffer allocated with size %d.\n", nAllocatinSize);
                nAllocatinSize = 0;
                ret = true_e;
            }
            else
            {
                if( d3A_Buffer )
                {
                    DVP_PRINT(DVP_ZONE_CAM, "3A Data Buffer culdn't be allocated.\n");
                    delete d3A_Buffer;
                }
            }

        }
        else
        {
            DVP_PRINT( DVP_ZONE_CAM, "3A data buffer could not be allocated: improper size calculated !!!\n" );
        }
    }
    else
    {
        ret = false_e;
    }

    return ret;
}

/// @todo void VisionCam_3A_Export::collect3AbufferData(): Make me non-void !!!!!!
void VisionCam_3A_Export::collect3AbufferData()
{
    node_t *node = NULL;
    size_t curSize = 0;

    if( false_e == bSettingsHeld )
    {
        node = list_pop( mSettingList );

        while( node )
        {
            curSize += ((size_t*)(node->data))[0];
            DVP_PRINT(DVP_ZONE_CAM, "Copying %d bytes into 3A data buffer.\n",((size_t*)(node->data))[0]);
            DVP_PRINT(DVP_ZONE_CAM, "Buffer size is %d \n", nAllocatinSize);

    //        d3A_Buffer->push((void*)( (uint32_t)(node->data) + sizeof(size_t) ), ((size_t*)(node->data))[0] );
            d3A_Buffer->push((void*) &( ((size_t*)(node->data))[1]), ((size_t*)(node->data))[0] );
            free( (void*)(node->data) );
            free(node);
            node = NULL;
            node = list_pop( mSettingList );
        }

        if( curSize != nAllocatinSize )
        {
            DVP_PRINT(DVP_ZONE_ERROR, "Copied %d bytes into 3A data buffer; buffer size is %d \n",curSize, nAllocatinSize);
        }
    }
}

status_e VisionCam_3A_Export::set( SettingType_e paramType, void*  paramValue, size_t size __attribute__ ((unused)) )
{
    status_e ret = STATUS_SUCCESS;

    SOSAL::AutoLock lock( &mLock);

    if( VCAM_3A_Start <= paramType &&  paramType < e3A_ManualSetting_MAX )
    {
//        VisionCamExecutionService<VisionCam_3A_Export, VisionCam_3A_Export::setFuncPtrType>::execFuncPrt_t fun = execSRVC->getFunc(paramType);
        VisionCam_3A_Export::setFuncPtrType fun = execSRVC->getFunc(paramType);
        if( fun )
        {
            ret = (this->*(fun))(paramValue);
        }
        else
        {
            // This case should never appear.
            // If this happens anyway, possibly you have missed to add an execution functin against this param ID.
            // Use execSRVC->Register() to add it.
            DVP_PRINT(DVP_ZONE_ERROR, "%s : requested parameter with ID %d", __func__, paramType );
            DVP_PRINT(DVP_ZONE_ERROR, "Such parameter doesn't have implemented api. Or is not registered.\n" );
            ret = STATUS_NO_MAPPING;
        }
    }
    else
    {
        DVP_PRINT(DVP_ZONE_CAM, "3A user export set(): received invalid parameter.\n" );
        DVP_PRINT(DVP_ZONE_CAM, "Requested parameter with index %d ( 0x%x ) is invalid; possible are between %d ( 0x%x ) and %d ( 0x%x )\n",
                  paramType, paramType,
                  VCAM_3A_Start, VCAM_3A_Start,
                  (e3A_ManualSetting_ALL - 1), (e3A_ManualSetting_ALL - 1) );

        ret = STATUS_INVALID_PARAMETER;
    }

    if ( STATUS_SUCCESS == ret && !bSettingsHeld )
    {
        bool_e bufferCreated = create3Abuffer();

        if( bufferCreated )
            collect3AbufferData();
    }

    if( STATUS_SUCCESS == ret )
    {
        DVP_PRINT(DVP_ZONE_CAM, "3A configuration wtritten successfully to queue.\n");
    }
    else
    {
        DVP_PRINT(DVP_ZONE_CAM, "Error Writing 3A configuratin to queue. Returning err code : %d ( 0x%x )", ret, ret );
    }

    return ret;
}

status_e VisionCam_3A_Export::reset( SettingType_e param )
{
    status_e ret = STATUS_BASE;

    SOSAL::AutoLock lock( &mLock);
    DVP_PRINT(DVP_ZONE_CAM,  "%s is not implemented; returning error.\n", __func__);

    int32_t start = ( param == e3A_ManualSetting_ALL ? VCAM_3A_Start : param );
    int32_t end = ( param == e3A_ManualSetting_ALL ? e3A_ManualSetting_ALL : param + 1);

    for( int32_t i = start; i < end ; i++)
    {
        void* data = calloc(1, execSRVC->getDataSize(i));
        ret = (this->*(execSRVC->getFunc(i)))(data);

        free(data);
        if( ret != STATUS_SUCCESS )
        {
            /// @todo set some debug prints in case of error, but don't leave the loop !!
        }
    }

    return ret;
}

status_e VisionCam_3A_Export::get( SettingType_e sett __attribute__ ((unused)) , void *data __attribute__ ((unused)))
{
    status_e ret = STATUS_NOT_IMPLEMENTED;

    SOSAL::AutoLock lock( &mLock);
    DVP_PRINT(DVP_ZONE_CAM,  "%s is not implemented; returning error.\n", __func__);

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Lsc2D( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_2D_LscConfig_t *vcamParam = (VisionCam_3A_2D_LscConfig_t*)param;

    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_ISIF_2DLSC_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_Lsc2D, vcamParam->eSensor, vcamParam->m2D_LscConfig, OMX_TI_3ASKIP_ISIF_2DLSC_CFGTYPE );

        if( STATUS_SUCCESS == ret )
        {
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
        }
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Clamp( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_Clamp_t *vcamParam = (VisionCam_3A_Clamp_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_ISIF_CLAMP_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_Clamp, vcamParam->eSensor, vcamParam->mClamp, OMX_TI_3ASKIP_ISIF_CLAMP_CFGTYPE);

        if( STATUS_SUCCESS == ret )
        {
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
        }
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_GainOffset( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_GainOffset_t *vcamParam = (VisionCam_3A_GainOffset_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_ISIF_GAINOFFSET_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_GainOffset, vcamParam->eSensor, vcamParam->mGainOffset, OMX_TI_3ASKIP_ISIF_GAINOFFSET_CFGTYPE);

        if( STATUS_SUCCESS == ret )
        {
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
        }
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Vlcd( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_VDLC_t *vcamParam = (VisionCam_3A_VDLC_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_ISIF_VLDC_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_Vlcd, vcamParam->eSensor, vcamParam->mVLDC, OMX_TI_3ASKIP_ISIF_VLDC_CFGTYPE);

        if( STATUS_SUCCESS == ret )
        {
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
        }
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Nf1( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_NoiseFilterConfig_t *vcamParam = (VisionCam_3A_NoiseFilterConfig_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_Nf1, vcamParam->eSensor, vcamParam->eNoiseFilterConfig, OMX_TI_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE);

        if( STATUS_SUCCESS == ret )
        {
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
        }
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Nf2( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_NoiseFilterConfig_t *vcamParam = (VisionCam_3A_NoiseFilterConfig_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_Nf2, vcamParam->eSensor, vcamParam->eNoiseFilterConfig, OMX_TI_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE);

        if( STATUS_SUCCESS == ret )
        {
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
        }
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_GIC( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_GIC_Config_t *vcamParam = (VisionCam_3A_GIC_Config_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_GIC_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_GIC, vcamParam->eSensor, vcamParam->mGIC_config, OMX_TI_3ASKIP_IPIPE_GIC_CFGTYPE);

        if( STATUS_SUCCESS == ret )
        {
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
        }
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_WB( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_WB_Config_t *vcamParam = (VisionCam_3A_WB_Config_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_WB_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_WB , vcamParam->eSensor, vcamParam->eWB_config, OMX_TI_3ASKIP_IPIPE_WB_CFGTYPE );

        if( STATUS_SUCCESS == ret )
        {
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
        }
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_CFA( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_CFA_Config_t *vcamParam = (VisionCam_3A_CFA_Config_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_CFA_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_CFA, vcamParam->eSensor, vcamParam->mCFA_config, OMX_TI_3ASKIP_IPIPE_CFA_CFGTYPE);

        if( STATUS_SUCCESS == ret )
        {
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
        }
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Gamma( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_GammaTableConfig_t *vcamParam = (VisionCam_3A_GammaTableConfig_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_GAMMA_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_Gamma, vcamParam->eSensor, vcamParam->mGammaTable, OMX_TI_3ASKIP_IPIPE_GAMMA_CFGTYPE);

        if( STATUS_SUCCESS == ret )
        {
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
        }
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Rgb2Rgb1( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_RGB2RGB_t *vcamParam = (VisionCam_3A_RGB2RGB_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_RGBRGB_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_Rgb2Rgb1, vcamParam->eSensor, vcamParam->mRGB_config, OMX_TI_3ASKIP_IPIPE_RGBRGB_CFGTYPE);

        if( STATUS_SUCCESS == ret )
        {
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
        }
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Rgb2Rgb2( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_RGB2RGB_t *vcamParam = (VisionCam_3A_RGB2RGB_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_RGBRGB_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_Rgb2Rgb2, vcamParam->eSensor, vcamParam->mRGB_config, OMX_TI_3ASKIP_IPIPE_RGBRGB_CFGTYPE);

        if( STATUS_SUCCESS == ret )
        {
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
        }
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Rgb2Yuv( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_RGB2YUV_t *vcamParam = (VisionCam_3A_RGB2YUV_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_RGBYUV_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_Rgb2Yuv, vcamParam->eSensor, vcamParam->mRGB2YUV, OMX_TI_3ASKIP_IPIPE_RGBYUV_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_GBCE( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_GBCE_Config_t *vcamParam = (VisionCam_3A_GBCE_Config_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_GBCE_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_GBCE, vcamParam->eSensor, vcamParam->mGBCE_config, OMX_TI_3ASKIP_IPIPE_GBCE_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Yuv2Yuv( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_Yuv444_toYuv422_Config_t *vcamParam = (VisionCam_3A_Yuv444_toYuv422_Config_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_YUV444YUV422_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_Yuv2Yuv, vcamParam->eSensor, vcamParam->mYuvToYuv_config, OMX_TI_3ASKIP_IPIPE_YUV444YUV422_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Ee( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_EdgeEnhancement_config_t *vcamParam = (VisionCam_3A_EdgeEnhancement_config_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_EE_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_Ee, vcamParam->eSensor, vcamParam->eEdgeEnhConfig, OMX_TI_3ASKIP_IPIPE_EE_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Car( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_CAR_config_t *vcamParam = (VisionCam_3A_CAR_config_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_CAR_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_Car, vcamParam->eSensor, vcamParam->mCar, OMX_TI_3ASKIP_IPIPE_CAR_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Lsc( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_LscConfig_t *vcamParam = (VisionCam_3A_LscConfig_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_LSC_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_Lsc, vcamParam->eSensor, vcamParam->mLsc, OMX_TI_3ASKIP_IPIPE_LSC_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Histogram( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_HistogramConfig_t *vcamParam = (VisionCam_3A_HistogramConfig_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_HIST_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_Histogram, vcamParam->eSensor, vcamParam->mHistogramConfig, OMX_TI_3ASKIP_IPIPE_HIST_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

#ifdef OLD_3A_EXPORT
status_e VisionCam_3A_Export::Set_3A_Boxcar( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_BoxCarConfig_t *vcamParam = (VisionCam_3A_BoxCarConfig_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_BOXCAR_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_Boxcar, vcamParam->eSensor, vcamParam->eBoxCarConfig, OMX_TI_3ASKIP_IPIPE_BOXCAR_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}
#endif // #ifdef OLD_3A_EXPORT

status_e VisionCam_3A_Export::Set_3A_Bsc( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_BscConfig_t *vcamParam = (VisionCam_3A_BscConfig_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_BSC_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_Bsc, vcamParam->eSensor, vcamParam->mBscConfig, OMX_TI_3ASKIP_IPIPE_BSC_CFGTYPE );

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

#ifdef OLD_3A_EXPORT
status_e VisionCam_3A_Export::Set_3A_3Dcc( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_3DCC_config_t *vcamParam = (VisionCam_3A_3DCC_config_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_3DCC_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_3Dcc, vcamParam->eSensor, vcamParam->m3dccConfig, OMX_TI_3ASKIP_IPIPE_3DCC_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}
#endif // OLD_3A_EXPORT
status_e VisionCam_3A_Export::Set_3A_DpcOtf( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_DpcOtfConfig_t *vcamParam = (VisionCam_3A_DpcOtfConfig_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_DPCOTF_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_DpcOtf, vcamParam->eSensor, vcamParam->mDPC, OMX_TI_3ASKIP_IPIPE_DPCOTF_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

#ifdef OLD_3A_EXPORT
status_e VisionCam_3A_Export::Set_3A_DpcLut( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_DPC_Lut_t *vcamParam = (VisionCam_3A_DPC_Lut_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_DPCLUT_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_DpcLut, vcamParam->eSensor, vcamParam->mDPC_lut, OMX_TI_3ASKIP_IPIPE_DPCLUT_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}
#endif // #ifdef OLD_3A_EXPORT

status_e VisionCam_3A_Export::Set_3A_Cgs( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_CGS_Config_t *vcamParam = (VisionCam_3A_CGS_Config_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPE_CGS_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_Cgs, vcamParam->eSensor, vcamParam->mCgs, OMX_TI_3ASKIP_IPIPE_CGS_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Dfs( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_DfsConfig_t* vcamParam = (VisionCam_3A_DfsConfig_t*)param;
    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPEIF_DFS_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_Dfs, vcamParam->eSensor, vcamParam->mDfsConfig, OMX_TI_3ASKIP_IPIPEIF_DFS_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Dpc1( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_DpcConfig_t *vcamParam = (VisionCam_3A_DpcConfig_t*)param;

    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPEIF_DPC_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_Dpc1, vcamParam->eSensor, vcamParam->mDpcConfig, OMX_TI_3ASKIP_IPIPEIF_DPC_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Dpc2( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_DpcConfig_t *vcamParam = (VisionCam_3A_DpcConfig_t*)param;

    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_IPIPEIF_DPC_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_Dpc2, vcamParam->eSensor, vcamParam->mDpcConfig, OMX_TI_3ASKIP_IPIPEIF_DPC_CFGTYPE );

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_Dpcm( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_DPCM_Config_t *vcamParam = (VisionCam_3A_DPCM_Config_t*)param;

    ALLOC_LIST_NODE(data, VCAM_3ASKIP_IPIPEIF_DPCM_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_Dpcm, vcamParam->eSensor, vcamParam->mDPCM, VCAM_3ASKIP_IPIPEIF_DPCM_CFGTYPE );

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_HLpf( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_ResizerLowPassFilter_t *vcamParam = (VisionCam_3A_ResizerLowPassFilter_t *)param;

    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_RSZ_LPF_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_HLpf, vcamParam->eSensor, vcamParam->mRszLpfConfig, OMX_TI_3ASKIP_RSZ_LPF_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }


    return ret;
}

status_e VisionCam_3A_Export::Set_3A_VLpf( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_ResizerLowPassFilter_t *vcamParam = (VisionCam_3A_ResizerLowPassFilter_t *)param;

    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_RSZ_LPF_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexR_VLpf, vcamParam->eSensor, vcamParam->mRszLpfConfig, OMX_TI_3ASKIP_RSZ_LPF_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

#ifdef OLD_3A_EXPORT
status_e VisionCam_3A_Export::Set_3A_H3aAewbParams( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_AEWB_config_t *vcamParam = (VisionCam_3A_AEWB_config_t*)param;

    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_H3A_AEWB_PARAMTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_H3aAewbParams, vcamParam->eSensor, vcamParam->mAEWB_config, OMX_TI_3ASKIP_H3A_AEWB_PARAMTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}
#endif // #ifdef OLD_3A_EXPORT

#ifdef OLD_3A_EXPORT
status_e VisionCam_3A_Export::Set_3A_H3aAfParams( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_H3A_AutoFocusParam_t *vcamParam = (VisionCam_3A_H3A_AutoFocusParam_t *)param;

    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_H3A_AF_PARAMTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_H3aAfParams, vcamParam->eSensor, vcamParam->mH3A_AutoFocusParam, OMX_TI_3ASKIP_H3A_AF_PARAMTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}
#endif // #ifdef OLD_3A_EXPORT

status_e VisionCam_3A_Export::Set_3A_H3aCommonParams( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_H3A_CommonConfig_t *vcamParam = (VisionCam_3A_H3A_CommonConfig_t *)param;

    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_H3A_COMMON_CFGTYPE);

    if( data )
    {
        CONSTRUCT_DATA( data, OMX_TI_3aSkipIndexL_H3aCommonParams, vcamParam->eSensor, vcamParam->mH3A_CommonConfig, OMX_TI_3ASKIP_H3A_COMMON_CFGTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}

status_e VisionCam_3A_Export::Set_3A_CamControlExpGain( void *param )
{
    status_e ret = STATUS_SUCCESS;
    VisionCam_3A_ExpGainControl_t *vcamParam = (VisionCam_3A_ExpGainControl_t *)param;

    ALLOC_LIST_NODE(data, OMX_TI_3ASKIP_CAM_CONTROL_EXPGAINTYPE);

    if( data )
    {
        CONSTRUCT_DATA(data, OMX_TI_3aSkipIndexL_CamControlExpGain, vcamParam->eSensor, vcamParam->mExpGainControl, OMX_TI_3ASKIP_CAM_CONTROL_EXPGAINTYPE);

        if( STATUS_SUCCESS == ret )
            ADD_TO_3A_LIST(mSettingList, data, nAllocatinSize);
    }
    else
    {
        ret = STATUS_NOT_ENOUGH_MEMORY;
        DVP_PRINT(DVP_ZONE_CAM, "Not enough memory to write this configuration.");
    }

    return ret;
}
