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

#ifndef _VISION_CAM_3A_MANUAL_H_
#define _VISION_CAM_3A_MANUAL_H_

#include <sosal/sosal.h>
#include <OMX_TI_IVCommon.h>
#include <dvp/VisionCam_3A_ExportTypes.h>
#include <VisionCamUtils.h>
#include <dvp/dvp_debug.h>

/** @fn int nodeCompare(node_t *a, node_t *b)
  * used by getExecNode(), this function provides a comparasion method for ...
*/
int nodeCompare(node_t *a, node_t *b);

/** @class
  *
*/
class VisionCam_3A_Export {
public:
    VisionCam_3A_Export();
    ~VisionCam_3A_Export();

    /** @fn hold_3A()
      * Starts collecting 3A settings.
      * Use this option if various 3A setting must be applied.
      * Calling this method notifies VisionCam_3A_Export to start collecting 3A data.
      *
      * @see set() , unhold_3A()
    */
    status_e hold_3A();

    /** @fn release_3A()
      * Stops collecting 3A settings.
      * Calling this method notifies VisionCam_3A_Export to stop collecting 3A data
      * and to prepare the shared memory buffer.
      *
      * @see release_3A() , set()
    */
    status_e release_3A();

    /** @fn bool_e isHeld()
      * Tells the VisionCam_3A_Export usera whether 3A settings are currently held.
      *
      * @see hold_3A() , unhold_3A()
    */
    bool_e isHeld() { return bSettingsHeld; }

    /** @fn set()
      * Calls the apropriate seter function.
      *
      * @see hold_3A() , unhold_3A()
    */
    status_e set( SettingType_e, void*, size_t );

    /** Calls the apropriate getter function.
    */
    status_e get( SettingType_e, void* ); /// gets from hardware

    /** @fn status_e reset( SettingType_e param = e3A_ManualSetting_ALL)
      * Reset a given 3A configuration. By default, if 3A configuration type is not specified,
      * this will reset all 3A configurations.
    */
    status_e reset( SettingType_e param = e3A_ManualSetting_ALL);

    /** Keeps 3A parameters in hardware acceptable format.
    */
    DataBuffer_t *d3A_Buffer;

/**
  * Inner functions.
*/
private:
    /** @brief Constant, used to mark the proper sensor on which new 3A are going to be applied.
    */
    static const int32_t cSensorMask;// = 0x01000000;

    /** Dynamicaly allocated list of configurations to be applied.
      *
    */
    list_t *mSettingList;

    /** Marks if 3A settings will be sent in a single shot or on by one.
      * Raised by hold_3A() and cleared bu unhold_3A(), this flag indicates the state of 3A patameters collecting.
      * No settings will be provided to 3A framwork until this flag has been cleared.
    */
    bool_e bSettingsHeld;

    /** Keeps the size of all setting requested between hold_3A() and unhold_3A().
      * This size is the actual buffer allocatin size for DataBuffer_t::pData .
    */
    size_t nAllocatinSize;

    /** Locks public APIs.
    */
    mutex_t mLock;

    /** @fn status_e addToList(list_t *list, void *node, size_t size)
      * Adds any new configuration to a list of configuration settings.
      *
      * @param list_t *list - points to the list, used for internal data storage
      * @param void *data - the data to be added
      * @param size_t size - as named, this is the size in bytes of 'data'
      *
      * @see list_t
      * @see mSettingList
    */
    size_t addToList(list_t *list, void *data, size_t size);

    /** @fn bool_e VisionCam_3A_Export::create3Abuffer()
      * Create the 3A data buffer according to the size needed.
      * This memory must be mapped so that it could be read/written both by Camera HW and client code.
      *
      * @return true_e if buffer created successfuly, false_e otherwise.
    */
    bool_e create3Abuffer();

    /** @fn void collect3AbufferData()
      * @brief Copies the list of requested settings to a buffer which could be passed to HW specific layers.
    */
    void collect3AbufferData();

    /** @fn inline status_e VisionCam_3A_Export::sensorUsed( int32_t *data, VisionCamSensorSelection sen)
      * Checks the sensor for which a configuratin data should be applied.
      * This method is mostly written for convenience reasons during port.
    */
    inline status_e sensorUsed( int32_t *data, int32_t sen, OMX_TI_3ASKIP_TI_3ASKIPINDEXTYPE omxIndex );

    typedef status_e (VisionCam_3A_Export::*setFuncPtrType)(void*);

    /** Each of these functions, adds the requested parameter to settings list.
    */
    status_e Set_3A_Lsc2D( void *);
    status_e Set_3A_Clamp( void *);
    status_e Set_3A_GainOffset( void *);
    status_e Set_3A_Vlcd( void *);
    status_e Set_3A_Nf1( void *);
    status_e Set_3A_Nf2( void *);
    status_e Set_3A_GIC( void *);
    status_e Set_3A_WB( void *);
    status_e Set_3A_CFA( void *);
    status_e Set_3A_Gamma( void *);
    status_e Set_3A_Rgb2Rgb1( void *);
    status_e Set_3A_Rgb2Rgb2( void *);
    status_e Set_3A_Rgb2Yuv( void *);
    status_e Set_3A_GBCE( void *);
    status_e Set_3A_Yuv2Yuv( void *);
    status_e Set_3A_Ee( void *);
    status_e Set_3A_Car( void *);
    status_e Set_3A_Lsc( void *);
    status_e Set_3A_Histogram( void *);
    status_e Set_3A_Boxcar( void *);
    status_e Set_3A_Bsc( void *);
    status_e Set_3A_3Dcc( void *);
    status_e Set_3A_DpcOtf( void *);
    status_e Set_3A_DpcLut( void *);
    status_e Set_3A_Cgs( void *);
    status_e Set_3A_Dfs( void *);
    status_e Set_3A_Dpc1( void *);
    status_e Set_3A_Dpc2( void *);
    status_e Set_3A_Dpcm( void *);
    status_e Set_3A_HLpf( void *);
    status_e Set_3A_VLpf( void *);
    status_e Set_3A_H3aAewbParams( void *);
    status_e Set_3A_H3aAfParams( void *);
    status_e Set_3A_H3aCommonParams( void *);
    status_e Set_3A_CamControlExpGain( void *);

    VisionCamExecutionService <VisionCam_3A_Export, VisionCam_3A_Export::setFuncPtrType> *execSRVC;

#if 0
    /**
    */
    status_e Get_3A_ColorPattern( void *);
    status_e Get_3A_MsbPos( void *);
    status_e Get_3A_VpDevice( void *);
    status_e Get_3A_Lsc2D( void *);
    status_e Get_3A_Clamp( void *);
    status_e Get_3A_Flash( void *);
    status_e Get_3A_GainOffset( void *);
    status_e Get_3A_Vlcd( void *);
    status_e Get_3A_Nf1( void *);
    status_e Get_3A_Nf2( void *);
    status_e Get_3A_GIC( void *);
    status_e Get_3A_WB( void *);
    status_e Get_3A_CFA( void *);
    status_e Get_3A_Gamma( void *);
    status_e Get_3A_Rgb2Rgb1( void *);
    status_e Get_3A_Rgb2Rgb2( void *);
    status_e Get_3A_Rgb2Yuv( void *);
    status_e Get_3A_GBCE( void *);
    status_e Get_3A_Yuv2Yuv( void *);
    status_e Get_3A_Ee( void *);
    status_e Get_3A_Car( void *);
    status_e Get_3A_Lsc( void *);
    status_e Get_3A_Histogram( void *);
    status_e Get_3A_Boxcar( void *);
    status_e Get_3A_Bsc( void *);
    status_e Get_3A_3Dcc( void *);
    status_e Get_3A_DpcOtf( void *);
    status_e Get_3A_DpcLut( void *);
    status_e Get_3A_Cgs( void *);
    status_e Get_3A_Dfs( void *);
    status_e Get_3A_Dpc1( void *);
    status_e Get_3A_Dpc2( void *);
    status_e Get_3A_Dpcm( void *);
    status_e Get_3A_HLpf( void *);
    status_e Get_3A_VLpf( void *);
    status_e Get_3A_H3aAewbParams( void *);
    status_e Get_3A_H3aAfParams( void *);
    status_e Get_3A_H3aCommonParams( void *);
    status_e Get_3A_CamControlExpGain( void *);
#endif
};
#endif // _VISION_CAM_3A_MANUAL_H_
