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

#include <OMXVisionCam.h>

#undef VCAM_SET_FORMAT_ROTATION

#ifndef EXPORTED_3A
#define USE_WB_GAIN_PATCH // enable the hardcoded WB gain get/set apis
#endif // EXPORTED_3A

#define USE_SMALC_FOR_STEREO // uncomment this to enable stereo misalignment correction
//#define USE_VSTAB_FOR_STEREO // uncomment this to enable video stabilization

#ifdef USE_WB_GAIN_PATCH
    #include "OMXVisionCam_WB_patch.h"
#endif

//#define _USE_GAMMA_RESET_HC_ // enable the hard coded reset system for gamma tables

#include "OMXVisionCam_Gamma_Tbl.h"

#define HERE {printf("=======> OMX - %d <=======\n", __LINE__);fflush(stdout);}

#ifdef USE_WB_GAIN_PATCH
    #define RED 0
    #define GREEN_RED 1
    #define GREEN_BLUE 2
    #define BLUE 3

    #define CALCULATE_WB_GAINS_OFFSET(type,in,out) out = (type *)(in + 4 + in[4] + in[8])
#endif // USE_WB_GAIN_PATCH

#ifndef EXPORTED_3A
#define GAMMA_TABLE_SIZE (1024*sizeof(uint16_t))
#endif // EXPORTED_3A

#define OMX_CHECK(error, function)  {\
    error = function;\
    if (error != OMX_ErrorNone) {\
        DVP_PRINT(DVP_ZONE_ERROR, "OMX Error 0x%08x in "#function" on line # %u\n", error, __LINE__);\
    }\
}

#define OMX_CONVERT_RETURN_IF_ERROR(error, function) {\
    error = function;\
    if (error != OMX_ErrorNone) {\
        DVP_PRINT(DVP_ZONE_ERROR, "OMX Error 0x%08x in "#function" on line # %u\n", error, __LINE__);\
        return ConvertError(error);\
    }\
}

#define OMX_STRUCT_INIT(str, type, pVersion)  {\
    memset(&str, 0, sizeof(type));\
    str.nSize = sizeof(type);\
    memcpy(&str.nVersion, pVersion, sizeof(OMX_VERSIONTYPE));\
    str.nPortIndex = OMX_ALL;\
}

#define OMX_STRUCT_INIT_PTR(ptr, type, pVersion) {\
    memset(ptr, 0, sizeof(type));\
    ptr->nSize = sizeof(type);\
    memcpy(&ptr->nVersion, pVersion, sizeof(OMX_VERSIONTYPE));\
    ptr->nPortIndex = OMX_ALL;\
}

#define LOOP_PORTS( port, cnt ) \
        for(    cnt = ( VCAM_PORT_ALL == port ? VCAM_PORT_PREVIEW : port ); \
                cnt < ( VCAM_PORT_ALL == port ? VCAM_PORT_MAX : port + 1 ); \
                cnt++ \
            )

thread_ret_t FocusThreadLauncher(void *arg)
{
    OMXVisionCam *pCam = reinterpret_cast<OMXVisionCam *>(arg);
    pCam->waitForFocus();
    return 0;
}

thread_ret_t PreemptionThreadLauncher( void *arg )
{
    OMXVisionCam *pCam = reinterpret_cast<OMXVisionCam *>(arg);
    pCam->PreemptionService();
    return 0;
}

thread_ret_t FrameThreadFunc(void *arg)
{
    bool_e run = true_e;
    OMXVisionCam *pCam = (OMXVisionCam *)arg;
    SimpleMsg_t msg;

    semaphore_create(&pCam->mFrameSem, 1, false_e); // set max count
    semaphore_wait(&pCam->mFrameSem); // pre-decrement first time to get count to 0.
    while(run)
    {
        semaphore_wait(&pCam->mFrameSem);

        memset(&msg, 0, sizeof(SimpleMsg_t));
        if( queue_read(pCam->mFrameMessageQ, true_e, &msg) )
        {
            DVP_PRINT(DVP_ZONE_CAM, "Frame Recv Func got event %d\n",msg.event);
            switch(msg.event)
            {
                case EFrameReceived:
                    pCam->frameReceivedSrvc(msg.data);
                    break;
                case EFrameReturned:
                    break;

                case EFrameManagerExit:
                    run = false_e;
                    break;

                default:
                    break;

            }
        }
    }
    semaphore_delete(&pCam->mFrameSem);

    return NULL;
}

static void PrintOMXState(OMX_STATETYPE state)
{
    char *state_names[OMX_StateWaitForResources+1] = {
        "OMX_StateInvalid",
        "OMX_StateLoaded",
        "OMX_StateIdle",
        "OMX_StateExecuting",
        "OMX_StatePause",
        "OMX_StateWaitForResources"
    };
    if ((uint32_t)state < dimof(state_names)) {
        DVP_PRINT(DVP_ZONE_CAM, "OMX-CAMERA is in state %s\n", state_names[state]);
    }
}

OMX_U32 frameRates[] = {1, 5, 15, 24, 30, 60};
OMX_U32 numFrameRates = dimof(frameRates);


/* Constructor - Open the Shared Library, containing the Camera Adapter and
* gen an instance of the Adapter.
*/
OMXVisionCam::OMXVisionCam()
{
    m_callback = NULL;
    m_focuscallback = NULL;
    mutex_init(&mFrameBufferLock);
    mutex_init(&mUserRequestLock);
    mEventSignalQ = list_create();
    memset(&mCurGreContext, 0, sizeof(mCurGreContext));
}

status_e OMXVisionCam::init(void *cookie)
{
    mFlushInProcess = false_e;
    OMX_ERRORTYPE omxError = OMX_ErrorNone;
    status_e greError = STATUS_SUCCESS;

#if TIME_PROFILE
    PopulateTimeProfiler();
#endif // TIME_PROFILE

    DVP_PRINT(DVP_ZONE_CAM, "OMXVisionCam::OMXVisionCam\n");
    DVP_PRINT(DVP_ZONE_CAM, "OMXVisionCam compiled: %s, %s\n\n", __DATE__, __TIME__ );
    mLocalVersion = new OMX_VERSIONTYPE();
    m_callback = NULL;
    m_focuscallback = NULL;

    m_cookie = cookie; // save the cookie value;

    mLocalVersion->s.nVersionMajor = 1;
    mLocalVersion->s.nVersionMinor = 1;
    mLocalVersion->s.nRevision = 0 ;
    mLocalVersion->s.nStep =  0;

    semaphore_create(&mGreLocalSem, 1, false_e); // set the max count
    semaphore_wait(&mGreLocalSem); // pre-decrement the current count otherwise the first wait will pass early
    semaphore_create(&mGreFocusSem, 1, false_e); // set the max count
    semaphore_wait(&mGreFocusSem); // pre-decrement the current count otherwise the first wait will pass early

    // Initialize the Vision Core
    DVP_PRINT(DVP_ZONE_CAM, "Calling OMX_Init()\n");
    OMX_CONVERT_RETURN_IF_ERROR(omxError, OMX_Init());

    OMX_CALLBACKTYPE omxCallbacks;
    omxCallbacks.EventHandler    = OMXVisionCam::EventHandler;
    omxCallbacks.EmptyBufferDone = OMXVisionCam::EmptyBufferDone;
    omxCallbacks.FillBufferDone  = OMXVisionCam::FillBufferDone;

    memset( &(mCurGreContext), 0, sizeof( VCAM_ComponentContext ) );

    mSetParameterExecutor = NULL;
    initParameterSetters();

    mCurGreContext.mPortsInUse[VCAM_PORT_ALL] = OMX_ALL;
    mCurGreContext.mPortsInUse[VCAM_PORT_PREVIEW] = VCAM_CAMERA_PORT_VIDEO_OUT_PREVIEW;
    mCurGreContext.mPortsInUse[VCAM_PORT_VIDEO]   = VCAM_CAMERA_PORT_VIDEO_OUT_VIDEO;

    for( int i = VCAM_PORT_MIN; i < VCAM_PORT_MAX; i++ )
    {
        mBuffersInUse[i].mBuffers = NULL;
        mBuffersInUse[i].mNumberBuffers = 0;
        mCurGreContext.mCameraPortParams[i].mIsActive = false_e;
        mCurGreContext.mCameraPortParams[i].mColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
        mCurGreContext.mCameraPortParams[i].mWidth = QVGA_WIDTH;
        mCurGreContext.mCameraPortParams[i].mHeight = QVGA_HEIGHT;
        mCurGreContext.mCameraPortParams[i].mFrameRate = INITIAL_FRAMERATE;
        for(uint32_t buf = 0; buf < VCAM_NUM_BUFFERS; buf++ )
            mCurGreContext.mCameraPortParams[i].mBufferHeader[buf] = NULL;
        mFrameDescriptors[i] = NULL;
    }

    mCurGreContext.mHandleComp = NULL;

    getAvailableSensors(mDetectedSensors);

    DVP_PRINT(DVP_ZONE_CAM, "Calling OMX_GetHandle()\n");
    OMX_CONVERT_RETURN_IF_ERROR(omxError,OMX_GetHandle(&( mCurGreContext.mHandleComp ),
                                         (OMX_STRING)"OMX.TI.DUCATI1.VIDEO.CAMERA",
                                         this ,
                                         &omxCallbacks));
    GetDucatiVersion();

    OMX_CONVERT_RETURN_IF_ERROR(omxError,OMX_SendCommand(mCurGreContext.mHandleComp,
                                                         OMX_CommandPortDisable,
                                                         OMX_ALL,
                                                         NULL));


#if defined(BLAZE) || defined(SDP) || defined(BLAZE_TABLET)
    {
        OMX_U32 pr = 4;

        memcpy( &mVisionCamPriority.nVersion, mLocalVersion, sizeof(OMX_VERSIONTYPE));
        mVisionCamPriority.nGroupID = CAMERA_GROUP_ID;
        mVisionCamPriority.nGroupPriority = pr;
        mVisionCamPriority.nSize = sizeof(OMX_PRIORITYMGMTTYPE);

        omxError = OMX_SetParameter(mCurGreContext.mHandleComp, OMX_IndexParamPriorityMgmt ,&mVisionCamPriority);
    }
#else
    DVP_PRINT(DVP_ZONE_WARNING, "No Priority Management is enabled on this platform!\n");
#endif

    mFrameMessageQ = queue_create( VCAM_NUM_BUFFERS * VCAM_PORT_MAX, sizeof(SimpleMsg_t));

    mFrameThread = thread_create(FrameThreadFunc, this );
    mUseFramePackaging = false_e;

    m_frameNum = 0;
    mPreemptionState = VCAM_PREEMPT_INACTIVE;
    mFaceDetectionEnabled = VCAM_FACE_DETECTION_DISABLED;
    mReturnToExecuting = false_e;
#ifdef _USE_GAMMA_RESET_HC_
    mGammaResetPolulated = false_e;
#endif // _USE_GAMMA_RESET_HC_

//    mPendingConfigs = new VisionCamExecutionService<OMXVisionCam, OMXVisionCam::startAutofocusFuncPtr>(this);
    mPendingConfigs = new VisionCamExecutionService<OMXVisionCam, OMXVisionCam::pendingConfigsFunPtr_t>(this);

    return greError;
}

status_e OMXVisionCam::deinit()
{
    status_e greError = STATUS_SUCCESS;
    OMX_ERRORTYPE omxError = OMX_ErrorNone;

    // Free the handle for the Camera component
    if (mCurGreContext.mHandleComp)
    {
        // free the handle
        DVP_PRINT(DVP_ZONE_CAM, "Calling OMX_FreeHandle(0x%08x)\n", (unsigned)(mCurGreContext.mHandleComp));
        omxError = OMX_FreeHandle(mCurGreContext.mHandleComp);
        greError = ConvertError(omxError);
        mCurGreContext.mHandleComp = 0;
        if( STATUS_SUCCESS != greError )
        {
            DVP_PRINT(DVP_ZONE_ERROR, "ERROR: error freeing OMX handle.\nOMX_FreeHandle() returned 0x%x\n", omxError);
        }

        // Deinitialize the OMX Core
        DVP_PRINT(DVP_ZONE_CAM, "Calling OMX_Deinit()\n");
        omxError = OMX_Deinit();
        greError = ConvertError(omxError);
        if( STATUS_SUCCESS != greError )
        {
            DVP_PRINT(DVP_ZONE_ERROR, "ERROR: error in OMX_Deinit.OMX err: 0x%x\n", omxError);
        }
    }

    SimpleMsg_t msg;
    msg.event = EFrameManagerExit;
    msg.data = NULL;
    queue_write(mFrameMessageQ, true_e, &msg);
    semaphore_post(&mFrameSem);
    thread_join(mFrameThread);
    queue_destroy(mFrameMessageQ);

#if TIME_PROFILE
    for(int i = 0; i < VCAM_TIME_TARGET_MAX; i++)
        if( mTimeProfiler[i] )      // dump and clear
            delete mTimeProfiler[i];
#endif

    if(mLocalVersion)
    {

        delete mLocalVersion;
        mLocalVersion = NULL;
    }

    semaphore_delete(&mGreLocalSem);
    semaphore_delete(&mGreFocusSem);

    if( mSetParameterExecutor )
    {
        delete mSetParameterExecutor;
        mSetParameterExecutor = NULL;
    }

    if( mPendingConfigs )
    {
        delete mPendingConfigs;
        mPendingConfigs = NULL;
    }

    return greError;
}

/* Destructor - free all recources used by Vision Cam and from us */
OMXVisionCam::~OMXVisionCam()
{
    list_destroy(mEventSignalQ);
    mutex_deinit(&mFrameBufferLock);
    mutex_deinit(&mUserRequestLock);
    DVP_PRINT(DVP_ZONE_CAM, "OMX Vision Cam is destroyed!\n");
}

bool_e OMXVisionCam::initParameterSetters()
{
    bool_e  ret = false_e;
    mSetParameterExecutor = new VisionCamExecutionService<OMXVisionCam, OMXVisionCam::setParameterFuncPtr_t>(this);
    if( mSetParameterExecutor )
    {
        bool_e checkTable[VCAM_PARAM_MAX - VCAM_PARAM_MIN - 1];
        memset(checkTable, false_e, sizeof(checkTable));

        ret = registerParameterSetters(checkTable);

#ifdef DVP_DEBUG
        for( uint32_t i = 0; i < dimof(checkTable); i++ )
        {
            if( checkTable[i] != true_e )
            {
                int32_t paramId = VCAM_PARAM_MIN + 1 + i;
                DVP_PRINT(DVP_ZONE_ALWAYS, "Failed to register setter function for parameter with ID %d ( 0x%x )", paramId, paramId);
            }
        }
#endif // DVP_DEBUG

    }
    else
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Error: Could not initialize parameter setters!\n");
    }
    return ret;
}

bool_e OMXVisionCam::registerParameterSetters( bool_e *checkTable )
{
    bool_e ret = false_e;

    if( mSetParameterExecutor )
    {
#define FILL_CHECK_TABLE(i) checkTable[i - VCAM_PARAM_MIN - 1 ]

        FILL_CHECK_TABLE(VCAM_PARAM_COLOR_SPACE_FOURCC)     = mSetParameterExecutor->Register( VCAM_PARAM_COLOR_SPACE_FOURCC    , &OMXVisionCam::setColorSpace          , sizeof(fourcc_t)                  );
        FILL_CHECK_TABLE(VCAM_PARAM_BRIGHTNESS)             = mSetParameterExecutor->Register( VCAM_PARAM_BRIGHTNESS            , &OMXVisionCam::setBrightness          , sizeof(uint32_t)                  );
        FILL_CHECK_TABLE(VCAM_PARAM_CONTRAST)               = mSetParameterExecutor->Register( VCAM_PARAM_CONTRAST              , &OMXVisionCam::setContrast            , sizeof(int32_t)                   );
        FILL_CHECK_TABLE(VCAM_PARAM_SHARPNESS)              = mSetParameterExecutor->Register( VCAM_PARAM_SHARPNESS             , &OMXVisionCam::setSharpness           , sizeof(int32_t)                   );
        FILL_CHECK_TABLE(VCAM_PARAM_SATURATION)             = mSetParameterExecutor->Register( VCAM_PARAM_SATURATION            , &OMXVisionCam::setSaturation          , sizeof(int32_t)                   );
        FILL_CHECK_TABLE(VCAM_PARAM_FPS_FIXED)              = mSetParameterExecutor->Register( VCAM_PARAM_FPS_FIXED             , &OMXVisionCam::setFrameRate_Fixed     , sizeof(uint32_t)                  );
        FILL_CHECK_TABLE(VCAM_PARAM_FPS_VAR)                = mSetParameterExecutor->Register( VCAM_PARAM_FPS_VAR               , &OMXVisionCam::setFrameRate_Variable  , sizeof(VisionCamVarFramerateType) );
        FILL_CHECK_TABLE(VCAM_PARAM_FLICKER)                = mSetParameterExecutor->Register( VCAM_PARAM_FLICKER               , &OMXVisionCam::setFlicker             , sizeof(VisionCamFlickerType)      );
        FILL_CHECK_TABLE(VCAM_PARAM_CROP)                   = mSetParameterExecutor->Register( VCAM_PARAM_CROP                  , &OMXVisionCam::setCrop                , sizeof(VisionCamRectType)         );
        FILL_CHECK_TABLE(VCAM_PARAM_STEREO_INFO)            = mSetParameterExecutor->Register( VCAM_PARAM_STEREO_INFO           , &OMXVisionCam::setStereoInfo          , sizeof(VisionCamStereoInfo)       );
        FILL_CHECK_TABLE(VCAM_PARAM_CAP_MODE)               = mSetParameterExecutor->Register( VCAM_PARAM_CAP_MODE              , &OMXVisionCam::setCameraOperatingMode , sizeof(VisionCamCaptureMode)      );
        FILL_CHECK_TABLE(VCAM_PARAM_SENSOR_SELECT)          = mSetParameterExecutor->Register( VCAM_PARAM_SENSOR_SELECT         , &OMXVisionCam::setSensor              , sizeof(VisionCamSensorSelection)  );
        FILL_CHECK_TABLE(VCAM_PARAM_EXPOSURE_COMPENSATION)  = mSetParameterExecutor->Register( VCAM_PARAM_EXPOSURE_COMPENSATION , &OMXVisionCam::setExposureCompensation, sizeof(int32_t)                   );
        FILL_CHECK_TABLE(VCAM_PARAM_RESOLUTION)             = mSetParameterExecutor->Register( VCAM_PARAM_RESOLUTION            , &OMXVisionCam::setResolution          , sizeof(int32_t)                   );
        FILL_CHECK_TABLE(VCAM_PARAM_MANUAL_EXPOSURE)        = mSetParameterExecutor->Register( VCAM_PARAM_MANUAL_EXPOSURE       , &OMXVisionCam::setManualExporureTime  , sizeof(uint32_t)                  );
        FILL_CHECK_TABLE(VCAM_PARAM_EXPOSURE_ISO)           = mSetParameterExecutor->Register( VCAM_PARAM_EXPOSURE_ISO          , &OMXVisionCam::setISO                 , sizeof(uint32_t)                  );
        FILL_CHECK_TABLE(VCAM_PARAM_AWB_MODE)               = mSetParameterExecutor->Register( VCAM_PARAM_AWB_MODE              , &OMXVisionCam::setWhiteBalanceMode    , sizeof(VisionCamWhiteBalType)     );
        FILL_CHECK_TABLE(VCAM_PARAM_COLOR_TEMP)             = mSetParameterExecutor->Register( VCAM_PARAM_COLOR_TEMP            , &OMXVisionCam::setColorTemp           , sizeof(int32_t)                   );
        FILL_CHECK_TABLE(VCAM_PARAM_MIRROR)                 = mSetParameterExecutor->Register( VCAM_PARAM_MIRROR                , &OMXVisionCam::setMirror              , sizeof(int32_t)                   );

#if defined(OMX_CAMERA_SUPPORTS_IMAGE_PYRAMID)
        FILL_CHECK_TABLE(VCAM_PARAM_IMAGE_PYRAMID)          = mSetParameterExecutor->Register( VCAM_PARAM_IMAGE_PYRAMID         , &OMXVisionCam::setImagePyramid        , sizeof(VisionCamImagePyramidType) );
#endif // OMX_CAMERA_SUPPORTS_IMAGE_PYRAMID

#if ( defined(DUCATI_1_5) || defined(DUCATI_2_0) ) && defined(OMX_CAMERA_SUPPORTS_MANUAL_CONTROLS)
        FILL_CHECK_TABLE(VCAM_PARAM_AWB_MIN_DELAY_TIME)     = mSetParameterExecutor->Register( VCAM_PARAM_AWB_MIN_DELAY_TIME    , &OMXVisionCam::setAWBminDelayTime     , sizeof(uint32_t)                  );
        FILL_CHECK_TABLE(VCAM_PARAM_GESTURES_INFO)          = mSetParameterExecutor->Register( VCAM_PARAM_GESTURES_INFO         , &OMXVisionCam::setGestureInfo         , sizeof(VisionCamGestureInfo)      );
        FILL_CHECK_TABLE(VCAM_PARAM_AGC_MIN_DELAY_TIME)     = mSetParameterExecutor->Register( VCAM_PARAM_AGC_MIN_DELAY_TIME    , &OMXVisionCam::setAGC_MinimumDelay    , sizeof(int32_t)                   );
        FILL_CHECK_TABLE(VCAM_PARAM_AGC_LOW_TH)             = mSetParameterExecutor->Register( VCAM_PARAM_AGC_LOW_TH            , &OMXVisionCam::setAGC_LowThreshold    , sizeof(int32_t)                   );
        FILL_CHECK_TABLE(VCAM_PARAM_AGC_HIGH_TH)            = mSetParameterExecutor->Register( VCAM_PARAM_AGC_HIGH_TH           , &OMXVisionCam::setAGC_HighThreshold   , sizeof(int32_t)                   );
#endif // ((DUCATI_1_5) || defined(DUCATI_2_0) ) && (OMX_CAMERA_SUPPORTS_MANUAL_CONTROLS)

        FILL_CHECK_TABLE(VCAM_PARAM_HEIGHT)                 = mSetParameterExecutor->Register( VCAM_PARAM_HEIGHT                , &OMXVisionCam::setPreviewHeight        , sizeof(uint32_t)                 );
        FILL_CHECK_TABLE(VCAM_PARAM_WIDTH)                  = mSetParameterExecutor->Register( VCAM_PARAM_WIDTH                 , &OMXVisionCam::setPreaviewWidth        , sizeof(uint32_t)                 );
        FILL_CHECK_TABLE(VCAM_PARAM_DO_AUTOFOCUS)           = mSetParameterExecutor->Register( VCAM_PARAM_DO_AUTOFOCUS          , &OMXVisionCam::startAutoFocus          , sizeof(VisionCamFocusMode)       );
        FILL_CHECK_TABLE(VCAM_PARAM_DO_MANUALFOCUS)         = mSetParameterExecutor->Register( VCAM_PARAM_DO_MANUALFOCUS        , &OMXVisionCam::startManualFocus        , sizeof(uint32_t)                 );

#ifndef EXPORTED_3A
        FILL_CHECK_TABLE(VCAM_PARAM_WB_COLOR_GAINS)         = mSetParameterExecutor->Register( VCAM_PARAM_WB_COLOR_GAINS        , &OMXVisionCam::setWBalColorGains       , sizeof(VisionCamWhiteBalGains)   );
        FILL_CHECK_TABLE(VCAM_PARAM_GAMMA_TBLS)             = mSetParameterExecutor->Register( VCAM_PARAM_GAMMA_TBLS            , &OMXVisionCam::setGammaTableColorGains , sizeof(VisionCamGammaTableType)  );
#endif // EXPORTED_3A

#if defined(VCAM_SET_FORMAT_ROTATION)
        FILL_CHECK_TABLE(VCAM_PARAM_ROTATION)               = mSetParameterExecutor->Register( VCAM_PARAM_ROTATION              , &OMXVisionCam::setFormatRotation       , sizeof(int32_t)                  );
#else
        FILL_CHECK_TABLE(VCAM_PARAM_ROTATION)               = mSetParameterExecutor->Register( VCAM_PARAM_ROTATION              , &OMXVisionCam::setPreviewRotation      , sizeof(int32_t)                  );
#endif // VCAM_SET_FORMAT_ROTATION

#ifdef EXPORTED_3A
        FILL_CHECK_TABLE(VCAM_PARAM_EXPORTED_3A_HOLD)       = mSetParameterExecutor->Register( VCAM_PARAM_EXPORTED_3A_HOLD      , &OMXVisionCam::startCollectingMaual3AParams   , 0                         );
        FILL_CHECK_TABLE(VCAM_PARAM_EXPORTED_3A_SET)        = mSetParameterExecutor->Register( VCAM_PARAM_EXPORTED_3A_SET       , &OMXVisionCam::setManual3AParam               , sizeof(VisionCam_3Asettings_Base_t) );
        FILL_CHECK_TABLE(VCAM_PARAM_EXPORTED_3A_APPLY)      = mSetParameterExecutor->Register( VCAM_PARAM_EXPORTED_3A_APPLY     , &OMXVisionCam::applyCollectedManual3AParams   , 0                         );
        FILL_CHECK_TABLE(VCAM_PARAM_EXPORTED_3A_RESET)      = mSetParameterExecutor->Register( VCAM_PARAM_EXPORTED_3A_RESET     , &OMXVisionCam::resetManual3AParams            , 0                         );
#endif // EXPORTED_3A

#undef FILL_CHECK_TABLE
    }
    else
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Error: Parameter executor is NULL!\n");
        DVP_PRINT(DVP_ZONE_ERROR, "Error: Couldn't register parameter setters!\n");
    }

    return ret;
}


/**
* This is used to get esier the state of component.
*/
inline OMX_STATETYPE OMXVisionCam::getComponentState()
{
    OMX_STATETYPE state = OMX_StateInvalid;

    if( mCurGreContext.mHandleComp )
    {
        OMX_GetState( mCurGreContext.mHandleComp, &state );
    }
    PrintOMXState(state);
    return state;
}

int32_t OMXVisionCam::getAvailableSensors(VCamSensorAvailable &s)
{
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
    bool_e gotHandle = (mCurGreContext.mHandleComp ? true_e : false_e );

    OMX_CALLBACKTYPE omxCallbacks;
    omxCallbacks.EventHandler    = OMXVisionCam::EventHandler;
    omxCallbacks.EmptyBufferDone = OMXVisionCam::EmptyBufferDone;
    omxCallbacks.FillBufferDone  = OMXVisionCam::FillBufferDone;

    VisionCamCaptureMode monoModes[] = {
        VCAM_VIDEO_NORMAL,
        VCAM_VIDEO_HIGH_SPEED,
        VCAM_GESTURE_MODE
    };

    VisionCamCaptureMode stereoModes[] = {
        VCAM_STEREO_MODE,
#ifdef OMX_CAMERA_SUPPORTS_STEREO_GESTURES
        VCAM_STEREO_GEST_MODE,
#endif
#ifdef OMX_CAMERA_SUPPORTS_SIMULTANEOUS_SENSOR_GESTURES
        VCAM_GESTURE_DUAL_SENSOR_MODE
#endif
    };

    enum {
        mono,
        stereo,
        modesNum
    };

    struct targetMode {
        int32_t             numModes;
        VisionCamCaptureMode *modes;
    };

    struct targetMode targetModes[modesNum];
    targetModes[mono].numModes = ARR_SIZE(monoModes);
    targetModes[mono].modes = monoModes;

    targetModes[stereo].numModes = ARR_SIZE(stereoModes);
    targetModes[stereo].modes = stereoModes;
    /// @todo Must be tesetd on OMAP5 !!!
    for( int32_t channelMode = mono; channelMode < modesNum; channelMode++ )
    {
        // until this jod is done outside ducati, we could have a lot of issues here,
        // because we don't have on what to rely while we are detecting connected sensors !
        // (it is just the ducati specification that is changing...)
        // so we'll try to reduce iniproper detections as much as possible, but can never be sure
        // that all will go smoothly with this code !!!
        int32_t start  = ((channelMode == mono) ? OMX_PrimarySensor : OMX_TI_StereoSensor);
        int32_t end = (channelMode == mono ? OMX_SecondarySensor : OMX_TI_StereoSensor );
        for (int32_t sn = start; sn <= end; sn++)
        {
            if( mCurGreContext.mHandleComp == NULL )
            {
                // Get a handle to OMX component. This is workaround needed due to some Ducati issues.
                omxError = OMX_GetHandle(&( mCurGreContext.mHandleComp ), (OMX_STRING)"OMX.TI.DUCATI1.VIDEO.CAMERA", this , &omxCallbacks);
                if( OMX_ErrorNone != omxError )
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "%s returned error 0x%x at line %d\n", __func__, omxError, __LINE__);
                }
            }

            if( mCurGreContext.mHandleComp && OMX_ErrorNone == omxError )
            {
                omxError = OMX_SendCommand(mCurGreContext.mHandleComp, OMX_CommandPortDisable, OMX_ALL, NULL);
                if( OMX_ErrorNone != omxError )
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "%s returned error 0x%x at line %d\n", __func__, omxError, __LINE__);
                }
            }

            if( mCurGreContext.mHandleComp && OMX_ErrorNone == omxError )
            {
                OMX_CONFIG_SENSORSELECTTYPE sen;
                OMX_STRUCT_INIT(sen, OMX_CONFIG_SENSORSELECTTYPE, mLocalVersion);
                sen.nPortIndex = mCurGreContext.mPortsInUse[VCAM_PORT_ALL];
                sen.eSensor = (OMX_SENSORSELECT)sn;

                omxError = OMX_SetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexConfigSensorSelect, &sen );

                if( OMX_ErrorNone != omxError )
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "%s returned error 0x%x at line %d\n", __func__, omxError, __LINE__);
                    continue;
                }

                bool_e new_one = false_e;
                for( int32_t opModeCounter = 0; opModeCounter < targetModes[channelMode].numModes; opModeCounter++ )
                {
                    OMX_CONFIG_CAMOPERATINGMODETYPE opMode;
                    memcpy( &opMode.nVersion, mLocalVersion, sizeof(OMX_VERSIONTYPE) );
                    opMode.nSize = sizeof( OMX_CONFIG_CAMOPERATINGMODETYPE );
                    opMode.eCamOperatingMode = (OMX_CAMOPERATINGMODETYPE)getLutValue(   targetModes[channelMode].modes[opModeCounter],
                                                                                        VCAM_VALUE_TYPE,
                                                                                        CaptureModeLUT,
                                                                                        ARR_SIZE(CaptureModeLUT)
                                                                                    );

                    omxError = OMX_SetParameter( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_IndexCameraOperatingMode, &opMode );

                    if( OMX_ErrorNone != omxError )
                    {
                        continue;
                    }

#if GET_SENSOR_CAPS
                    if( OMX_ErrorNone == omxError )
                    {
                        if (STATUS_SUCCESS != transitToState( OMX_StateIdle ))
                        {
                            omxError = OMX_ErrorIncorrectStateTransition;
                        }
                    }

                    if (OMX_ErrorNone != omxError )
                    {
                        continue;
                    }
#endif // GET_SENSOR_CAPS

                    if( OMX_ErrorNone == omxError && (s.sensorCount < VCAM_Max_Supported_Sensor_Count ))
                    {
                        int32_t senInd = s.sensorCount;
                        int32_t modeInd = s.sensors[senInd].supportedModesCnt;
                        s.sensors[senInd].supportedModes[modeInd] = targetModes[channelMode].modes[opModeCounter];
                        s.sensors[senInd].supportedModesCnt++;

                        if( 1 == s.sensors[senInd].supportedModesCnt )
                        {
                            new_one = true_e;
                        }
                    }
#if GET_SENSOR_CAPS
                    if( OMX_ErrorNone == omxError )
                    {
                        getSensorCaps(s.sensors[s.sensorCount]);
                        transitToState(OMX_StateLoaded);
                    }
#endif // GET_SENSOR_CAPS
                }
                if( new_one )
                {
                    s.sensors[s.sensorCount].nIndex = sn;
                    s.sensorCount++;
                }
            }

            // Release OMX component handle
            if( mCurGreContext.mHandleComp )
            {
                OMX_FreeHandle(mCurGreContext.mHandleComp);
                mCurGreContext.mHandleComp = NULL;
            }
        }
    }

#ifdef sen_dump
    dumpDetectedSensors();
#endif // sen_dump

    // keep the handle if it has been present when we entered this code.
    if( gotHandle && mCurGreContext.mHandleComp == NULL )
    {
        // Get a handle to OMX component. This is workaround needed due to some Ducati issues.
        omxError = OMX_GetHandle(&( mCurGreContext.mHandleComp ), (OMX_STRING)"OMX.TI.DUCATI1.VIDEO.CAMERA", this , &omxCallbacks);
    }

    return s.sensorCount;
}

#ifdef sen_dump
void OMXVisionCam::dumpDetectedSensors()
{
    DVP_PRINT(DVP_ZONE_ALWAYS,"sensorCount = %d\n", mDetectedSensors.sensorCount);
    for( int idxSensor = 0; idxSensor < mDetectedSensors.sensorCount; idxSensor++)
    {
        DVP_PRINT(DVP_ZONE_ALWAYS,"\tsensors[%d].nIndex \t\t\t= %d\n",idxSensor, mDetectedSensors.sensors[idxSensor].nIndex);
        DVP_PRINT(DVP_ZONE_ALWAYS,"\tsensors[%d].supportedModesCnt \t\t\t= %d\n",idxSensor, mDetectedSensors.sensors[idxSensor].supportedModesCnt);
        for(int idxMode = 0; idxMode < mDetectedSensors.sensors[idxSensor].supportedModesCnt; idxMode++)
        {
            DVP_PRINT(DVP_ZONE_ALWAYS,"\tsensors[%d].supportedModes[%d] \t\t\t= %d\n",idxSensor, idxMode, mDetectedSensors.sensors[idxSensor].supportedModes[idxMode]);
        }
    }
}
#endif // sen_dump

/**
* Initialisation of OMX_PARAM_PORTDEFINITIONTYPE
* needed for various configurations
*/
inline OMX_ERRORTYPE OMXVisionCam::initPortCheck( OMX_PARAM_PORTDEFINITIONTYPE * portCheck , OMX_U32 portIndex )
{
    OMX_ERRORTYPE omxError = OMX_ErrorNone;
    OMX_STRUCT_INIT_PTR(portCheck, OMX_PARAM_PORTDEFINITIONTYPE, mLocalVersion)

    if( VCAM_PORT_ALL < portIndex && VCAM_PORT_MAX > portIndex )
    {
        portCheck->nPortIndex = mCurGreContext.mPortsInUse[portIndex];
        omxError = OMX_GetParameter(mCurGreContext.mHandleComp,
                                    OMX_IndexParamPortDefinition,
                                    portCheck);
        DVP_PRINT(DVP_ZONE_CAM, "PORT CHECK[%u], E:%d P:%d B#:%u %ux%u C:%d\n",
                  (uint32_t)portCheck->nPortIndex,
                  portCheck->bEnabled,
                  portCheck->bPopulated,
                  (uint32_t)portCheck->nBufferCountActual,
                  (uint32_t)portCheck->format.video.nFrameWidth,
                  (uint32_t)portCheck->format.video.nFrameHeight,
                  portCheck->format.video.eColorFormat);

    }
    else
        omxError = OMX_ErrorBadParameter;

    if( omxError != OMX_ErrorNone )
    {
        DVP_PRINT(DVP_ZONE_ERROR, "OMX_GetParameter - 0x%x in initPortCheck(%lu)\n",
                  omxError, mCurGreContext.mPortsInUse[portIndex]);
    }
    return omxError;
}
/**
   @brief Method to convert from OMX_ERRORTYPE to GesturError_e
   @param error Any of the standard OMX error codes defined in the OpenMAX 1.x Specification.
   @return status_e
 */
status_e OMXVisionCam::ConvertError(OMX_ERRORTYPE error)
{
    status_e status = STATUS_SUCCESS;
    switch(error)
    {
        case OMX_ErrorNone:
            status = STATUS_SUCCESS;
            break;
        case OMX_ErrorBadParameter:
            status = STATUS_INVALID_PARAMETER;
            break;
        case OMX_ErrorIncorrectStateOperation:
            status = STATUS_INVALID_STATE;
            break;
        case OMX_ErrorHardware:
            status = STATUS_CATASTROPHIC;
            break;
        default:
            status = STATUS_FAILURE;
            break;
    }
    if (error != OMX_ErrorNone) {
        DVP_PRINT(DVP_ZONE_ERROR, "Converting OMX Error 0x%08x to status_e %d\n", error, status);
    }
    return status;
}

int OMXVisionCam::getLutValue( int searchVal, ValueTypeOrigin origin, const int lut[][2], int lutSize)
{
    int idxToGet = (origin == VCAM_VALUE_TYPE ? OMX_VALUE_TYPE : VCAM_VALUE_TYPE );
    for( int i = 0; i < lutSize; i++ )
    {
        if( lut[i][origin] == searchVal )
            return lut[i][idxToGet];
    }

    return STATUS_INVALID_PARAMETER;
}

void OMXVisionCam::GetDucatiVersion()
{
    if( mCurGreContext.mHandleComp )
    {
        OMX_VERSIONTYPE compVersion;
        OMX_VERSIONTYPE specVersion;
        char compName[128];
        OMX_UUIDTYPE compUUID[128];
        OMX_ERRORTYPE omxError = OMX_ErrorNone;
        DVP_PRINT(DVP_ZONE_CAM, "Querying Component Version!\n");
        omxError = OMX_GetComponentVersion(mCurGreContext.mHandleComp,
                                           compName,
                                           &compVersion,
                                           &specVersion,
                                           compUUID);
        if (omxError == OMX_ErrorNone)
        {
            DVP_PRINT(DVP_ZONE_CAM, "\tComponent Name:    [%s]\n",   compName);
            DVP_PRINT(DVP_ZONE_CAM, "\tComponent Version: [%u]\n",   (unsigned int)compVersion.nVersion);
            DVP_PRINT(DVP_ZONE_CAM, "\tSpec Version:      [%u]\n",   (unsigned int)specVersion.nVersion);
            DVP_PRINT(DVP_ZONE_CAM, "\tComponent UUID:    [%s]\n\n", (char*)compUUID);
        }
    }
}

status_e OMXVisionCam::transitToState(OMX_STATETYPE targetState, serviceFunc transitionService, void * data )
{
    status_e greError = STATUS_SUCCESS;
    OMX_ERRORTYPE omxError = OMX_ErrorNone;
    semaphore_t sem; // this semaphore does not need to be global

    if( OMX_StateInvalid == targetState )
        return STATUS_INVALID_PARAMETER;

    semaphore_create(&sem, 1, false_e);
    semaphore_wait(&sem); // predecrement so that the later wait will actually block

    omxError = RegisterForEvent( mCurGreContext.mHandleComp,
                                    OMX_EventCmdComplete,
                                    OMX_CommandStateSet,
                                    targetState,
                                    &sem,
                                    -1 // Infinite timeout
                                );

    if( OMX_ErrorNone == omxError )
    {
        omxError = OMX_SendCommand( mCurGreContext.mHandleComp ,
                                                   OMX_CommandStateSet,
                                                   targetState,
                                                   NULL );
        switch( omxError )
        {
            case OMX_ErrorNone:
            {
                if( transitionService )
                {
                    greError = transitionService( this, data );
                }
                break;
            }

            case OMX_ErrorIncorrectStateTransition:
            {
                greError = STATUS_INVALID_PARAMETER;
                break;
            }

            case OMX_ErrorInsufficientResources:
            {
                if( OMX_StateIdle == targetState )
                {
                    mPreemptionState = VCAM_PREEMPT_WAIT_TO_START;
                    greError = PreemptionService();
                }
                break;
            }

            case OMX_ErrorSameState:
                greError = STATUS_SUCCESS;
                break;

            default:
                greError = ConvertError(omxError);
                break;
        }

        if( greError != STATUS_SUCCESS || OMX_ErrorSameState == omxError )
        {
            // unregister the event
            EventHandler( mCurGreContext.mHandleComp,
                          this,
                          (OMX_EVENTTYPE)OMX_EventCmdComplete,
                          (OMX_U32)OMX_CommandStateSet,
                          (OMX_U32)targetState,
                          NULL
                        );
        }
        else //( STATUS_SUCCESS == greError )
        {
            DVP_PRINT(DVP_ZONE_CAM, "Waiting for state transition. State requested: %d\n", (int)targetState);
            semaphore_wait(&sem);
            DVP_PRINT(DVP_ZONE_CAM, "Camera is now in state %d\n", (int)targetState);
        }
    }

    semaphore_delete(&sem);

    return greError;
}

status_e OMXVisionCam::portEnableDisable( OMX_COMMANDTYPE enCmd, serviceFunc enableService, VisionCamPort_e port )
{
    status_e greError = STATUS_SUCCESS;
    OMX_ERRORTYPE omxError = OMX_ErrorNone;

    OMX_PARAM_PORTDEFINITIONTYPE portCheck;
    VCAM_PortParameters * portData = NULL;

    int32_t p;
    LOOP_PORTS( port , p )
    {
        initPortCheck(&portCheck, p);
        portData = &mCurGreContext.mCameraPortParams[p];

        // check to see if the port is already enabled/disabled and that we wanted that state
        if( (OMX_TRUE == portCheck.bEnabled) && (enCmd == OMX_CommandPortEnable) )
        {
            portData->mIsActive = true_e;
            continue;
        }

        if( (OMX_FALSE == portCheck.bEnabled) && (enCmd == OMX_CommandPortDisable) )
        {
            portData->mIsActive = false_e;
            continue;
        }

        // we wanted to transition from one state to the other

        // ports with no buffers can't be enabled or disabled.
        if( portData->mNumBufs == 0 && OMX_StateLoaded < getComponentState() )
        {
            continue;
        }

        // we have buffers, so enable/disable the port!
        DVP_PRINT(DVP_ZONE_CAM, "Registering for port enable/disable event\n");
        omxError = RegisterForEvent(    mCurGreContext.mHandleComp,
                                        OMX_EventCmdComplete,
                                        enCmd,
                                        mCurGreContext.mPortsInUse[p],
                                        &mGreLocalSem,
                                        -1 /*Infinite timeout */
                                    );

        if( OMX_ErrorNone == omxError )
        {
            omxError = OMX_SendCommand( mCurGreContext.mHandleComp,
                                        enCmd,
                                        mCurGreContext.mPortsInUse[p],
                                        NULL
                                        );
        }

        if( enableService && OMX_ErrorNone == omxError )
        {
            greError = enableService( this, (void*)(&p) );
        }

        if( OMX_ErrorNone == omxError && STATUS_SUCCESS == greError )
        {
            semaphore_wait(&mGreLocalSem);
        }
        else
        {
            // unregister event
            EventHandler( mCurGreContext.mHandleComp,
                        this,
                        (OMX_EVENTTYPE)OMX_EventCmdComplete,
                        (OMX_U32)enCmd,
                        (OMX_U32)mCurGreContext.mPortsInUse[p],
                        NULL
                        );
        }

        initPortCheck(&portCheck, p);

        if( OMX_TRUE == portCheck.bEnabled )
        {
            portData->mIsActive = true_e;
            mFramePackage.mExpectedFrames[p] = true_e;
        }
        else
        {
            portData->mIsActive = false_e;
            mFramePackage.mExpectedFrames[p] = false_e;
        }
    }

    mCurGreContext.mCameraPortParams[VCAM_PORT_ALL].mIsActive = true_e;
    int32_t p2;
    LOOP_PORTS(VCAM_PORT_ALL, p2)
    {
        if( mCurGreContext.mCameraPortParams[p2].mIsActive == false_e )
        {
            mCurGreContext.mCameraPortParams[VCAM_PORT_ALL].mIsActive = false_e;
            break;
        }
    }

    return greError;
}

status_e OMXVisionCam::fillPortBuffers( VisionCamPort_e port )
{
    status_e greError = STATUS_SUCCESS;
    OMX_ERRORTYPE omxError = OMX_ErrorNone;
    VCAM_PortParameters *portData = NULL;

    portData = &mCurGreContext.mCameraPortParams[port];
    if (portData->mIsActive == true_e)
    {

        for( int index = 0; index < portData->mNumBufs; index++ )
        {
            if( portData && portData->mBufferHeader[index] )
            {
                DVP_PRINT(DVP_ZONE_CAM, "FILL BUFF HDR[%d]:%p PORT:%u\n", index, portData->mBufferHeader[index], (uint32_t)mCurGreContext.mPortsInUse[port]);
                omxError = OMX_FillThisBuffer(mCurGreContext.mHandleComp,
                                              (OMX_BUFFERHEADERTYPE*)( portData->mBufferHeader[index]));
                if (omxError != OMX_ErrorNone)
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "OMX_FillThisBuffer() returned error  0x%x\n", omxError );
                    greError = ConvertError(omxError);
                    break;
                }
            }
            else
            {
                greError = STATUS_NO_RESOURCES; // STATUS_NO_MAPPING;
            }
        }
    }
    else
    {
        greError = STATUS_NO_RESOURCES; // can't fill a port that isn't active
    }
    if (omxError != OMX_ErrorNone)
    {
        greError = ConvertError(omxError);
    }

    return greError;
}

status_e OMXVisionCam::freePortBuffers( VisionCamPort_e port )
{
    status_e greError = STATUS_SUCCESS;
    OMX_ERRORTYPE omxError = OMX_ErrorNone;
    VCAM_PortParameters *portData = NULL;

    SOSAL::AutoLock lock( &mFrameBufferLock );

    int32_t p;
    LOOP_PORTS( port , p )
    {
        portData = &mCurGreContext.mCameraPortParams[p];
        for( int buff = 0; buff < portData->mNumBufs; buff++ )
        {
            omxError = OMX_FreeBuffer( mCurGreContext.mHandleComp,
                                        mCurGreContext.mPortsInUse[p],
                                        portData->mBufferHeader[buff]
                                      );
            portData->mBufferHeader[buff] = NULL;
            if (omxError != OMX_ErrorNone)
            {
                DVP_PRINT(DVP_ZONE_ERROR, "OMX_FreeBuffer() returned error  0x%x\n", omxError );
                greError = ConvertError(omxError);
                break;
            }
        }
    }

    return greError;
}

status_e OMXVisionCam::populatePort( VisionCamPort_e port )
{
    status_e greError = STATUS_SUCCESS;
    OMX_ERRORTYPE omxError = OMX_ErrorNone;
    VCAM_PortParameters * data = NULL;

    int32_t p;
    LOOP_PORTS( port , p )
    {
#if defined(DVP_USE_GRALLOC) || defined(DVP_USE_ION)
        // @NOTE in ICS/JB, we have to send "gralloc" buffers which may be TILER 2D or SGX buffers
        OMX_TI_PARAMUSENATIVEBUFFER useNativeHandles;
        OMX_STRUCT_INIT(useNativeHandles, OMX_TI_PARAMUSENATIVEBUFFER, mLocalVersion);
        useNativeHandles.nPortIndex = mCurGreContext.mPortsInUse[p];
        useNativeHandles.bEnable = OMX_TRUE;
        omxError = OMX_SetParameter(mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexUseNativeBuffers, &useNativeHandles);
        DVP_PRINT(DVP_ZONE_CAM, "Configuring port %u for GRALLOC/ION handles (err=0x%08x)\n", p, omxError);
#elif defined(DVP_USE_BO)
        OMX_TI_PARAM_USEBUFFERDESCRIPTOR desc;
        OMX_STRUCT_INIT(desc, OMX_TI_PARAM_USEBUFFERDESCRIPTOR, mLocalVersion);
        // No idea
        desc.nPortIndex = mCurGreContext.mPortsInUse[p];
        // For 2D buffer
        desc.bEnabled = OMX_FALSE;
        omxError = OMX_SetParameter(mCurGreContext.mHandleComp, (OMX_INDEXTYPE) OMX_TI_IndexUseDmaBuffers, &desc);
        DVP_PRINT(DVP_ZONE_CAM, "Configuring port %u for DMAbuf handles (err=0x%08x)\n", p, omxError);
#endif
        data = &mCurGreContext.mCameraPortParams[p];

        for( int indBuff = 0; indBuff < data->mNumBufs; indBuff++ )
        {
            OMX_BUFFERHEADERTYPE *pBufferHdr = NULL;
            OMX_U8 *buffer = NULL;
#if defined(DVP_USE_BO)
            OMX_S32 fds[DVP_MAX_PLANES];
            OMX_U32 i;
            alloc_omap_bo_t **bos = NULL;

            memset(fds, 0, sizeof(fds));
#endif

            DVP_PrintImage(DVP_ZONE_CAM, &mBuffersInUse[p].mBuffers[indBuff]);
#if defined(DVP_USE_GRALLOC) || defined(DVP_USE_ION)
            // Pull the handle (Gralloc or ION) out of the DVP_Image_t structure reserved
            buffer = (OMX_U8 *)mBuffersInUse[p].mBuffers[indBuff].reserved;

            DVP_PRINT(DVP_ZONE_CAM, "VCAM: Using GRALLOC/ION handle %p\n", buffer);
#elif defined(DVP_USE_BO)
            bos = (alloc_omap_bo_t **)mBuffersInUse[p].mBuffers[indBuff].reserved;
            for (i = 0; i < mBuffersInUse[p].mBuffers[indBuff].planes; i++)
                fds[i] = bos[i]->dmabuf_fd;
            buffer = (OMX_U08 *)fds;
            DVP_PRINT(DVP_ZONE_CAM, "VCAM: Passing DMAbuf FDs in %p\n", buffer);
#endif
            omxError = OMX_UseBuffer( mCurGreContext.mHandleComp,
                                        &pBufferHdr,
                                        mCurGreContext.mPortsInUse[p],
                                        0,
                                        data->mBufSize,
                                        buffer);

            if( OMX_ErrorNone != omxError )
            {
                DVP_PRINT(DVP_ZONE_ERROR, "VCAM: ERROR: OMX_UseBuffer() returned 0x%x ( %d )\n", omxError, omxError);
                greError = ConvertError(omxError);
                break;
            }

            pBufferHdr->pAppPrivate = (OMX_PTR)&mBuffersInUse[p].mBuffers[indBuff];
            pBufferHdr->nSize = sizeof(OMX_BUFFERHEADERTYPE);
            memcpy( &(pBufferHdr->nVersion), mLocalVersion, sizeof( OMX_VERSIONTYPE ) );

            data->mBufferHeader[indBuff] = pBufferHdr;

            if( NULL == mFrameDescriptors[p][indBuff]->mFrameBuff )
                    mFrameDescriptors[p][indBuff]->mFrameBuff = pBufferHdr->pAppPrivate;
        }
    }

    return greError;
}

status_e OMXVisionCam::useBuffers( DVP_Image_t *pImages, uint32_t numImages, VisionCamPort_e port )
{
    status_e greError = STATUS_SUCCESS;

    if ( pImages == NULL || numImages == 0  || port <= VCAM_PORT_ALL || port >= VCAM_PORT_MAX)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Invalid parameters to useBuffers()\n");
        return STATUS_INVALID_PARAMETER;
    }

    // alloc the array for frame descriptors
    if( !mFrameDescriptors[port] )
    {
        mFrameDescriptors[port] = new VisionCamFrame*[numImages];
        if( NULL == mFrameDescriptors[port] )
        {
            greError = STATUS_NOT_ENOUGH_MEMORY;
        }

        for( uint32_t i = 0; i < numImages && STATUS_SUCCESS == greError; i++ )
        {
            mFrameDescriptors[port][i] = new VisionCamFrame();

            if( mFrameDescriptors[port][i] )
            {
                mFrameDescriptors[port][i]->clear();
            }
            else
            {
                while( i ) {
                    --i; // delete 0
                    delete mFrameDescriptors[port][i];
                }

                delete [] mFrameDescriptors[port];
                mFrameDescriptors[port] = NULL;

                greError = STATUS_NOT_ENOUGH_MEMORY;
                break;
            }
        }
    }

    if( STATUS_SUCCESS == greError )
    {
        VCAM_PortParameters * data = &mCurGreContext.mCameraPortParams[port];

        mBuffersInUse[port].mBuffers = pImages;
        mBuffersInUse[port].mNumberBuffers = (uint32_t)numImages;

        data->mBufSize      = pImages[0].numBytes;
        data->mNumBufs      = mBuffersInUse[port].mNumberBuffers;
        data->mStride       = mBuffersInUse[port].mBuffers[0].y_stride;

        if( OMX_StateIdle != getComponentState() )
        {
            greError = transitToState( OMX_StateIdle );
        }
    }

    return greError;
}

status_e OMXVisionCam::flushBuffers( VisionCamPort_e port)
{
    status_e greError = STATUS_SUCCESS;
    OMX_ERRORTYPE omxError = OMX_ErrorNone;

    if( OMX_StateExecuting != getComponentState() )
    {
        return greError;
    }

    int32_t p;
    LOOP_PORTS( port , p )
    {
        semaphore_t sem;
        semaphore_create(&sem, 1, false_e );
        semaphore_wait(&sem); // predecrement so that the next wait won't fire ahead of time.
        omxError = RegisterForEvent(mCurGreContext.mHandleComp,
                                    OMX_EventCmdComplete,
                                    OMX_CommandFlush,
                                    mCurGreContext.mPortsInUse[p],
                                    &sem,
                                    -1 /*Infinite timeout*/
                                    );

        if( OMX_ErrorNone == omxError )
        {
            omxError = OMX_SendCommand( mCurGreContext.mHandleComp,
                                        OMX_CommandFlush,
                                        mCurGreContext.mPortsInUse[p],
                                        NULL );
        }

        if( OMX_ErrorNone == omxError )
        {
            semaphore_wait(&sem);
        }

        semaphore_delete(&sem);
    }
    return greError;
}

/* Send command to Camera */
status_e OMXVisionCam::sendCommand( VisionCamCmd_e cmdId, void *param, uint32_t size, VisionCamPort_e port)
{
    status_e greError = STATUS_SUCCESS;
    OMX_ERRORTYPE omxError = OMX_ErrorNone;

    SOSAL::AutoLock lock( &mUserRequestLock );

    size = size; // warnings
    DVP_PRINT(DVP_ZONE_CAM, "SEND CMD: 0x%04x, %p, "FMT_SIZE_T", %d\n", cmdId, param, size, port);

    switch (cmdId)
    {
        case VCAM_CMD_PREVIEW_START:
        {
            greError = startPreview(port);
            break;
        }

        case VCAM_CMD_PREVIEW_STOP:
        {
            if( OMX_StateExecuting == getComponentState() )
                greError = stopPreview(port);
            else
                greError = STATUS_INVALID_STATE;
            break;
        }

        case VCAM_EXTRA_DATA_START:
        {
            int i, EDataType;
            for( i = 0; i < VCAM_EXTRA_DATA_TYPE_MAX; i++ )
            {
                if( ExtraDataTypeLUT[ i ][ 0 ]  == *( (int32_t*)param ) )
                {
                    EDataType = ExtraDataTypeLUT[ i ][ 1 ];
                    break;
                }
            }
            if( i == VCAM_EXTRA_DATA_TYPE_MAX )
            {
                greError = STATUS_INVALID_PARAMETER;
                break;
            }

            OMX_CONFIG_EXTRADATATYPE xData;
            OMX_STRUCT_INIT(xData, OMX_CONFIG_EXTRADATATYPE, mLocalVersion);
#if defined(OMX_CAMERA_SUPPORTS_CAMERA_VIEW)
            xData.eCameraView       = OMX_2D_Prv;
#endif
            if( OMX_ExtraDataNone == EDataType )
            {
              for( int i = 1; i < VCAM_EXTRA_DATA_TYPE_MAX; i++ )
              {// stop extra data transfer for all types of data
                  if( STATUS_SUCCESS == greError )
                  {
                    xData.eExtraDataType = (OMX_EXT_EXTRADATATYPE)ExtraDataTypeLUT[i][1];
                    xData.bEnable = OMX_FALSE;
                    int32_t p;
                    LOOP_PORTS( port, p )
                    {
                        xData.nPortIndex = p;
                        omxError = OMX_SetConfig( mCurGreContext.mHandleComp, ( OMX_INDEXTYPE )OMX_IndexConfigOtherExtraDataControl, &xData);
                    }
                  }
              }
            }
            else
            {
                if( STATUS_SUCCESS == greError )
                {
                    xData.eExtraDataType = ( OMX_EXT_EXTRADATATYPE )EDataType;
                    xData.bEnable = OMX_TRUE;

                    omxError = OMX_SetConfig( mCurGreContext.mHandleComp, ( OMX_INDEXTYPE )OMX_IndexConfigOtherExtraDataControl, &xData);
                }
            }
            break;
        }

        case VCAM_EXTRA_DATA_STOP:
        {
            int i, EDataType;
            for( i = 0; i < VCAM_EXTRA_DATA_TYPE_MAX; i++ )
            {
                if( ExtraDataTypeLUT[ i ][ 0 ]  == *( (int*)param ) )
                {
                    EDataType = ExtraDataTypeLUT[ i ][ 1 ];
                    break;
                }
            }
            if( i == VCAM_EXTRA_DATA_TYPE_MAX )
            {
                greError = STATUS_INVALID_PARAMETER;
                break;
            }

            OMX_CONFIG_EXTRADATATYPE xData;
            OMX_STRUCT_INIT(xData, OMX_CONFIG_EXTRADATATYPE, mLocalVersion);
#if defined(OMX_CAMERA_SUPPORTS_CAMERA_VIEW)
            xData.eCameraView       = OMX_2D_Prv;
#endif
            if( OMX_ExtraDataNone == EDataType )
            {
                  for( int i = 1; i < VCAM_EXTRA_DATA_TYPE_MAX; i++ )
                  {
    //                   greError = OMX_GetConfig( mCurGreContext.mHandleComp, ( OMX_INDEXTYPE )OMX_IndexConfigOtherExtraDataControl, &xData);
                      if( STATUS_SUCCESS == greError )
                      {
                          xData.eExtraDataType = (OMX_EXT_EXTRADATATYPE)ExtraDataTypeLUT[i][1];
                          xData.bEnable = OMX_FALSE;
                          int32_t p;
                          LOOP_PORTS(port, p)
                          {
                                xData.nPortIndex = p;
                                omxError = OMX_SetConfig( mCurGreContext.mHandleComp, ( OMX_INDEXTYPE )OMX_IndexConfigOtherExtraDataControl, &xData);
                          }
                      }
                  }
            }
            else
            {
                if( STATUS_SUCCESS == greError )
                {
                    xData.eExtraDataType = ( OMX_EXT_EXTRADATATYPE )EDataType;
//                     greError = OMX_GetConfig( mCurGreContext.mHandleComp, ( OMX_INDEXTYPE )OMX_IndexConfigOtherExtraDataControl, &xData);

                    xData.bEnable = OMX_FALSE;

                    omxError = OMX_SetConfig( mCurGreContext.mHandleComp, ( OMX_INDEXTYPE )OMX_IndexConfigOtherExtraDataControl, &xData);
                }
            }
            break;
        }

        case VCAM_CMD_LOCK_AE:
        case VCAM_CMD_LOCK_AWB:
        {
            OMX_INDEXTYPE index;
            OMX_IMAGE_CONFIG_LOCKTYPE lockCfg;
            OMX_STRUCT_INIT(lockCfg, OMX_IMAGE_CONFIG_LOCKTYPE, mLocalVersion);
            if (VCAM_CMD_LOCK_AE == cmdId)
            {
                index = (OMX_INDEXTYPE)OMX_IndexConfigImageExposureLock;
            }
            else if (VCAM_CMD_LOCK_AWB == cmdId)
            {
                index = (OMX_INDEXTYPE)OMX_IndexConfigImageWhiteBalanceLock;
            }
            else
            {
                greError = STATUS_INVALID_PARAMETER;
                break;
            }
            OMX_GetConfig(mCurGreContext.mHandleComp, index, &lockCfg);
            lockCfg.bLock = *((OMX_BOOL*)param);
            OMX_SetConfig(mCurGreContext.mHandleComp, index, &lockCfg);
            break;
        }
#if TIME_PROFILE
        case VCAM_DUMP_TIMES:
        {
            for( int i = 0; i < VCAM_TIME_TARGET_MAX; i++ )
                if( mTimeProfiler[i] )
                    mTimeProfiler[i]->dump();
            break;
        }
#endif // TIME_PROFILE
        case VCAM_CMD_FACE_DETECTION:
        {
            if( *((bool_e*)(param)) )
            {
                mFaceDetectionEnabled |= (int32_t)VCAM_FACE_DETECTION;
            }
            else
            {
                mFaceDetectionEnabled &= (int32_t)(~VCAM_FACE_DETECTION);
            }

            enableFaceDetect(port, VCAM_FACE_DETECTION);
            break;
        }

        case VCAM_CMD_FACE_DETECTION_RAW:
        {
            if( *((bool_e*)(param)) )
            {
                mFaceDetectionEnabled |= (int32_t)VCAM_FACE_DETECTION_RAW;
            }
            else
            {
                mFaceDetectionEnabled &= (int32_t)(~VCAM_FACE_DETECTION_RAW);
            }

            enableFaceDetect(port, VCAM_FACE_DETECTION_RAW);

            break;
        }
#if ( defined(DUCATI_1_5) || defined(DUCATI_2_0) ) && defined(OMX_CAMERA_SUPPORTS_MANUAL_CONTROLS)
        case VCAM_CMD_FREEZE_AWB_PARAMS:
        {
            OMX_TI_CONFIG_FREEZE_AWB wbFreeze;
            OMX_STRUCT_INIT( wbFreeze, OMX_TI_CONFIG_FREEZE_AWB, mLocalVersion);
            wbFreeze.nTimeDelay = *((uint32_t*)param);

            omxError = OMX_SetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexConfigFreezeAWB, &wbFreeze );
            break;
        }

        case VCAM_CMD_FREEZE_AGC_PARAMS:
        {
            OMX_TI_CONFIG_FREEZE_AE aeFreeze;
            OMX_STRUCT_INIT( aeFreeze, OMX_TI_CONFIG_FREEZE_AE, mLocalVersion);
            aeFreeze.nTimeDelay = *((uint32_t*)param);

            omxError = OMX_SetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexConfigFreezeAutoExp, &aeFreeze );
            break;
        }
#endif
        case VCAM_CMD_SET_CLIENT_NOTIFICATION_CALLBACK:
        {
            mClientNotifier.mNotificationCallback = (VisionCamClientNotifier::VisionCamClientNotifierCallback)(param);
            break;
        }

        case VCAM_CMD_PACK_FRAMES:
        {
            mUseFramePackaging = *((bool_e*)param);
            break;
        }

        default:
        {
            DVP_PRINT(DVP_ZONE_ERROR, "Impossible command id requested: %d\n", cmdId);
            DVP_PRINT(DVP_ZONE_ERROR, "see VisionCamParam_e for possible command ids\n");
            greError = STATUS_INVALID_PARAMETER;
        }
    }

    if (greError == STATUS_SUCCESS)
    {
        greError = ConvertError(omxError);
    }

    if (greError != STATUS_SUCCESS)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "OMXVisionCam::sendCommand() exits with error %d for command id %d\n", greError, cmdId);
    }

    return greError;
}

/*
*  APIs to configure Vision Cam
*/
status_e OMXVisionCam::setParameter( VisionCamParam_e paramId, void* param, uint32_t size, VisionCamPort_e port)
{
    status_e vcamError = STATUS_SUCCESS;

    SOSAL::AutoLock lock( &mUserRequestLock );

    if (param == NULL || size == 0 || port >= VCAM_PORT_MAX)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "NULL param pointer passed to %s()\n",__func__);
        return STATUS_INVALID_PARAMETER;
    }
    else
    {
        DVP_PRINT(DVP_ZONE_CAM, "SET PARAM: 0x%04x, %p, "FMT_SIZE_T" (0x%08x), %d\n", paramId, param, size, (size==4?*(uint32_t *)param:0), port);
    }

    if (getComponentState() == OMX_StateInvalid)
        return STATUS_INVALID_STATE;

    if( VCAM_PARAM_MIN  < paramId && paramId < VCAM_PARAM_MAX )
    {
        if( mSetParameterExecutor )
        {
            mSetParameterExecutor->setData(paramId, param); // save the data, so we could use it later
            setParameterFuncPtr_t execFn = mSetParameterExecutor->getFunc(paramId);
            if( execFn )
            {
                vcamError = (this->*execFn)(param, size, port);
            }
            else
            {
                DVP_PRINT(DVP_ZONE_WARNING, "No Execution service provided for this parameter.\n");
            }
        }
        else
        {
            DVP_PRINT(DVP_ZONE_WARNING, "No Execution service provided for all parameters.\n");
            vcamError = STATUS_NOT_IMPLEMENTED;
        }
    }
    else
    {
        DVP_PRINT( DVP_ZONE_ERROR, "No such parameter supported. Requested is %d ( 0x%x )" , paramId, paramId );
        DVP_PRINT( DVP_ZONE_ERROR, "Supported are between %d ( 0x%x ) and %d ( 0x%x )\n" ,
                                    VCAM_PARAM_MIN, VCAM_PARAM_MIN, VCAM_PARAM_MAX, VCAM_PARAM_MAX );
        vcamError = STATUS_INVALID_PARAMETER;
    }

    if( vcamError != STATUS_SUCCESS )
    {
        DVP_PRINT(DVP_ZONE_ERROR, "setParameter() exits with error 0x%x (dec: %d) for param id 0x%x\n", vcamError, vcamError, paramId);
    }

    return vcamError;
}

status_e OMXVisionCam::setColorSpace(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    status_e vcamError = STATUS_SUCCESS;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;

    if( param != NULL )
    {
        fourcc_t colorSpace = *((fourcc_t*)param);
        if( port != VCAM_PORT_VIDEO && ( colorSpace == FOURCC_RGB565 || colorSpace == FOURCC_BGR565) )
        {
            DVP_PRINT(DVP_ZONE_ERROR, "Requested Color format requested is not supported on this port!!\n");
            DVP_PRINT(DVP_ZONE_ERROR, "Leaving it unchanged.\n");
            vcamError  = STATUS_INVALID_PARAMETER;
            return vcamError;
        }

//        int i = 0;
//        while( ColorFormatLUT[ i ][ 1 ] != OMX_COLOR_FormatMax )
//        {
//            if( ColorFormatLUT[ i ][ 0 ] == (int)colorSpace )
//            {
//                    mCurGreContext.mCameraPortParams[port].mColorFormat = (OMX_COLOR_FORMATTYPE)ColorFormatLUT[ i ][ 1 ];
//                    break;
//            }
//            i++;
//        }

//        if( ColorFormatLUT[ i ][ 1 ] == OMX_COLOR_FormatMax )
//        {
//            DVP_PRINT(DVP_ZONE_ERROR, "Color format requested is not supported!!\n Leaving it unchanged.\n");
//            vcamError  = STATUS_INVALID_PARAMETER;
//        }

        int32_t format = (OMX_COLOR_FORMATTYPE)getLutValue( colorSpace, VCAM_VALUE_TYPE,
                                                            ColorFormatLUT, ARR_SIZE(ColorFormatLUT)
                                                            );
        if( STATUS_INVALID_PARAMETER == format )
        {
            vcamError = STATUS_INVALID_PARAMETER;
            return vcamError;
        }
        else
        {
            mCurGreContext.mCameraPortParams[port].mColorFormat = (OMX_COLOR_FORMATTYPE)format;

            if( STATUS_SUCCESS == vcamError )
            {
                OMX_STATETYPE compState = getComponentState();

                    if( OMX_StateExecuting == compState || OMX_StatePause == compState )
                    {
                        vcamError = STATUS_INVALID_STATE;
                    }
            }

            if( STATUS_SUCCESS == vcamError )
            {
                OMX_PARAM_PORTDEFINITIONTYPE portCheck;

                int32_t p;
                LOOP_PORTS( port , p )
                {
                    omxError = initPortCheck( &portCheck, p );
                    if( omxError == OMX_ErrorNone )
                    {
                        portCheck.format.video.eColorFormat = mCurGreContext.mCameraPortParams[p].mColorFormat;
                        omxError = OMX_SetParameter( mCurGreContext.mHandleComp,
                                                    OMX_IndexParamPortDefinition,
                                                    &portCheck
                                                  );
                        if ( omxError != OMX_ErrorNone ) {
                            DVP_PRINT(DVP_ZONE_ERROR, "ERROR! OMX_SetParameter returned 0x%08x\n",vcamError);
                        }
                    }
                    else
                    {
                        vcamError = ConvertError(omxError);
                    }
                }
            }
            else
            {
                vcamError = STATUS_INVALID_PARAMETER;
            }
        }
    }

    return vcamError;
}

status_e OMXVisionCam::setBrightness(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;

    OMX_CONFIG_BRIGHTNESSTYPE brightness;
    brightness.nSize = sizeof(OMX_CONFIG_BRIGHTNESSTYPE);
    brightness.nBrightness = *((uint32_t*)param);
    memcpy( &brightness.nVersion, mLocalVersion, sizeof(OMX_VERSIONTYPE) );

    int32_t p = port;
//            LOOP_PORTS( port , p )
    {
        brightness.nPortIndex = mCurGreContext.mPortsInUse[p];
        omxError = OMX_SetConfig( mCurGreContext.mHandleComp, OMX_IndexConfigCommonBrightness, &brightness);
    }

    return ConvertError(omxError);
}

status_e OMXVisionCam::setContrast(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;

    OMX_CONFIG_CONTRASTTYPE contrast;
    contrast.nSize = sizeof( OMX_CONFIG_CONTRASTTYPE );
    contrast.nContrast = *((int32_t*)param);
    memcpy( &contrast.nVersion, mLocalVersion, sizeof(OMX_VERSIONTYPE) );

    int32_t p = port;
//            LOOP_PORTS( port , p )
    {
        contrast.nPortIndex = mCurGreContext.mPortsInUse[p];
        omxError = OMX_SetConfig( mCurGreContext.mHandleComp, OMX_IndexConfigCommonContrast, &contrast);
    }
    return ConvertError(omxError);
}

status_e OMXVisionCam::setSharpness(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
    OMX_IMAGE_CONFIG_PROCESSINGLEVELTYPE procSharpness;
    procSharpness.nSize = sizeof( OMX_IMAGE_CONFIG_PROCESSINGLEVELTYPE );
    procSharpness.nLevel = *((int32_t*)param);
    memcpy( &procSharpness.nVersion, mLocalVersion, sizeof(OMX_VERSIONTYPE) );

    if( procSharpness.nLevel == 0 )
        procSharpness.bAuto = OMX_TRUE;
    else
        procSharpness.bAuto = OMX_FALSE;

    int32_t p = port;
//            LOOP_PORTS( port , p )
    {
        procSharpness.nPortIndex = mCurGreContext.mPortsInUse[p];
        omxError = OMX_SetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_IndexConfigSharpeningLevel, &procSharpness);
    }

    return ConvertError(omxError);
}

status_e OMXVisionCam::setSaturation(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
    OMX_CONFIG_SATURATIONTYPE saturation;
    saturation.nSize = sizeof(OMX_CONFIG_SATURATIONTYPE);
    saturation.nSaturation = *((int32_t*)param);
    memcpy( &saturation.nVersion, mLocalVersion, sizeof(OMX_VERSIONTYPE) );

    int32_t p = port;
//            LOOP_PORTS( port , p )
    {
        saturation.nPortIndex = mCurGreContext.mPortsInUse[p];
        omxError = OMX_SetConfig( mCurGreContext.mHandleComp,
                                  OMX_IndexConfigCommonSaturation,
                                  &saturation
                                );
    }
    return ConvertError(omxError);
}

status_e OMXVisionCam::setFrameRate_Fixed( void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port __attribute__ ((unused)))
{
    status_e vCamError = STATUS_BASE;

    bool_e enableAgain[VCAM_PORT_MAX];

    int32_t p;
    LOOP_PORTS(VCAM_PORT_ALL , p)
        enableAgain[p] = mCurGreContext.mCameraPortParams[p].mIsActive;

    vCamError = stopPreview(VCAM_PORT_ALL);
#if 0
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
        {
            // If this is a SMART Sensor, we have to "fix" the frame rate to a non-variable rate
            OMX_CONFIG_BOOLEANTYPE bOMX;
            bOMX.nSize = sizeof(OMX_CONFIG_BOOLEANTYPE);
            memcpy(&bOMX.nVersion, mLocalVersion, sizeof(OMX_VERSIONTYPE));
            bOMX.bEnabled = OMX_TRUE;
            omxError = OMX_SetParameter(mCurGreContext.mHandleComp,
                                        (OMX_INDEXTYPE)OMX_TI_IndexParamSensorVTMode,
                                        &bOMX);
        }
    vCamError = ConvertError(omxError);
#endif
    int32_t i;
    LOOP_PORTS(VCAM_PORT_ALL , i)
    {
        mCurGreContext.mCameraPortParams[i].mFrameRate = (*((uint32_t*)param));
        if( enableAgain[i] )
        {
            vCamError = startPreview( (VisionCamPort_e)i );
        }
    }
    return vCamError;
}

#if defined(OMX_CAMERA_SUPPORTS_IMAGE_PYRAMID)
status_e OMXVisionCam::setImagePyramid(void *param, size_t size, VisionCamPort_e port)
{
    OMX_TI_PARAM_IMAGEPYRAMIDTYPE imagePyramid;
    OMX_CONFIG_EXTRADATATYPE xData;
    OMX_ERRORTYPE omxError;

    if( (param == NULL) || (size != sizeof(VisionCamImagePyramidType)) ) {
        return STATUS_INVALID_PARAMETER;
    }

    memcpy( &imagePyramid.nVersion, mLocalVersion, sizeof(OMX_VERSIONTYPE) );
    imagePyramid.nSize          = sizeof( OMX_TI_PARAM_IMAGEPYRAMIDTYPE );
    imagePyramid.nPortIndex     = mCurGreContext.mPortsInUse[port];
    imagePyramid.nLevelsCount   = ((VisionCamImagePyramidType *)param)->mLevelsCount;
    imagePyramid.nScalingFactor = ((VisionCamImagePyramidType *)param)->mScalingFactor;

    omxError = OMX_SetParameter( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexParamImagePyramid, &imagePyramid );

    if ( OMX_ErrorNone == omxError )
    {
        OMX_STRUCT_INIT(xData, OMX_CONFIG_EXTRADATATYPE, mLocalVersion);
        xData.nPortIndex     = mCurGreContext.mPortsInUse[port];
        xData.eExtraDataType = OMX_TI_ImagePyramid;
        if( imagePyramid.nLevelsCount > 1 ) {
            xData.bEnable = OMX_TRUE;
        } else {
            xData.bEnable = OMX_FALSE;
        }
        omxError = OMX_SetConfig(mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_IndexConfigOtherExtraDataControl, &xData);
    }

    return ConvertError(omxError);
}

status_e OMXVisionCam::getImagePyramid(void *param, size_t size, VisionCamPort_e port)
{
    OMX_TI_PARAM_IMAGEPYRAMIDTYPE imagePyramid;
    OMX_ERRORTYPE omxError;

    if( (param == NULL) || (size != sizeof(VisionCamImagePyramidType)) ) {
        return STATUS_INVALID_PARAMETER;
    }

    memcpy( &imagePyramid.nVersion, mLocalVersion, sizeof(OMX_VERSIONTYPE) );
    imagePyramid.nSize          = sizeof( OMX_TI_PARAM_IMAGEPYRAMIDTYPE );
    imagePyramid.nPortIndex     = mCurGreContext.mPortsInUse[port];

    omxError = OMX_GetParameter( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexParamImagePyramid, &imagePyramid );

    ((VisionCamImagePyramidType *)param)->mLevelsCount   = imagePyramid.nLevelsCount;
    ((VisionCamImagePyramidType *)param)->mScalingFactor = imagePyramid.nScalingFactor;

    return ConvertError(omxError);
}
#endif

status_e OMXVisionCam::setFrameRate_Variable(void *param, size_t size, VisionCamPort_e port __attribute__ ((unused)))
{
    status_e vcamError = STATUS_BASE;
    VisionCamVarFramerateType varFrate;

    if( sizeof(VisionCamVarFramerateType) == size || 0 == size )
    {
        memcpy( &varFrate , param , sizeof( VisionCamVarFramerateType ) );
    }
    else
    {
        vcamError = STATUS_INVALID_PARAMETER;
    }

    bool_e enableAgain[VCAM_PORT_MAX];

    int32_t p;
    LOOP_PORTS( VCAM_PORT_ALL , p )
    {
        enableAgain[p] = mCurGreContext.mCameraPortParams[p].mIsActive;
    }

    vcamError = stopPreview(VCAM_PORT_ALL);

    if( varFrate.mMin != 0 && vcamError == STATUS_SUCCESS )
    {
      // @todo implement when needed OMX interface is present
        varFrate = varFrate;
    }

    int32_t i;
    LOOP_PORTS( VCAM_PORT_ALL , i )
    {
        mCurGreContext.mCameraPortParams[i].mFrameRate = varFrate.mMax;
        if( enableAgain[i] )
        {
            vcamError = startPreview( (VisionCamPort_e)i );
        }
    }
    return vcamError;
}

status_e OMXVisionCam::setFlicker(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;

    OMX_CONFIG_FLICKERCANCELTYPE flicker;
    flicker.nSize = sizeof( OMX_CONFIG_FLICKERCANCELTYPE );
    memcpy( &flicker.nVersion, mLocalVersion, sizeof(OMX_VERSIONTYPE) );
    flicker.eFlickerCancel = *(OMX_COMMONFLICKERCANCELTYPE *)param;

    int32_t p = port;
//            LOOP_PORTS( VCAM_PORT_ALL , p )
    {
        flicker.nPortIndex = mCurGreContext.mPortsInUse[p];
        omxError = OMX_SetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_IndexConfigFlickerCancel, &flicker );
    }

    return ConvertError(omxError);
}

status_e OMXVisionCam::setCrop(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;

    OMX_CONFIG_RECTTYPE crop;
    crop.nSize = sizeof( OMX_CONFIG_RECTTYPE );
    crop.nLeft = ((VisionCamRectType*)param)->mLeft;
    crop.nTop = ((VisionCamRectType*)param)->mTop;
    crop.nWidth = ((VisionCamRectType*)param)->mWidth;
    crop.nHeight = ((VisionCamRectType*)param)->mHeight;
    memcpy( &crop.nVersion,  mLocalVersion, sizeof(OMX_VERSIONTYPE) );

    int32_t p = port;
//            LOOP_PORTS( port , p )
    {
        crop.nPortIndex = mCurGreContext.mPortsInUse[p];
        omxError = OMX_SetConfig( mCurGreContext.mHandleComp, OMX_IndexConfigCommonOutputCrop, &crop );
    }
    return ConvertError(omxError);
}
status_e OMXVisionCam::setStereoInfo(void *param, size_t size, VisionCamPort_e port)
{
    status_e greError = STATUS_BASE;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;

    VisionCamStereoInfo *info = (VisionCamStereoInfo *)param;
    OMX_TI_FRAMELAYOUTTYPE frameLayout;
    OMX_STRUCT_INIT(frameLayout, OMX_TI_FRAMELAYOUTTYPE, mLocalVersion);
    OMX_TI_CONFIG_CONVERGENCETYPE acParams;
    OMX_STRUCT_INIT(acParams, OMX_TI_CONFIG_CONVERGENCETYPE, mLocalVersion);

#ifndef USE_SMALC_FOR_STEREO
    OMX_TI_CONFIG_MM mmSettings;
    OMX_STRUCT_INIT(mmSettings, OMX_TI_CONFIG_MM, mLocalVersion);
#endif
#ifndef USE_VSTAB_FOR_STEREO
    OMX_CONFIG_BOOLEANTYPE vssEnable;
    vssEnable.nSize = sizeof(OMX_CONFIG_BOOLEANTYPE);
    memcpy(&vssEnable.nVersion, mLocalVersion, sizeof(OMX_VERSIONTYPE));
#endif

    if ( info && size == sizeof(VisionCamStereoInfo))
    {
        int32_t p = port;
//                LOOP_PORTS( port , p )
        {
            int value = 0;
            frameLayout.nPortIndex = mCurGreContext.mPortsInUse[p];
            DVP_PRINT(DVP_ZONE_CAM, "Stereo Info: Layout %u, SubSampling: %u\n", info->layout, info->subsampling);

            value = getLutValue((int)info->layout, VCAM_VALUE_TYPE, StereoLayoutLUT, ARR_SIZE(StereoLayoutLUT));

            // some feature was compiled out.
            if (value == STATUS_INVALID_PARAMETER)
                return (status_e)value;

            frameLayout.eFrameLayout = (OMX_TI_STEREOFRAMELAYOUTTYPE)value;

//                    if (info->layout == VCAM_STEREO_LAYOUT_TOPBOTTOM)
//                        frameLayout.eFrameLayout = OMX_TI_StereoFrameLayoutTopBottom;
//                    else if (info->layout == VCAM_STEREO_LAYOUT_LEFTRIGHT)
//                        frameLayout.eFrameLayout = OMX_TI_StereoFrameLayoutLeftRight;
            frameLayout.nSubsampleRatio = info->subsampling << 7; // in Q15.7 format
            omxError = OMX_SetParameter( mCurGreContext.mHandleComp,
                                         (OMX_INDEXTYPE)OMX_TI_IndexParamStereoFrmLayout,
                                         &frameLayout
                                        );
            greError = ConvertError(omxError);
            if( OMX_ErrorNone == omxError )
            {
                acParams.nPortIndex = mCurGreContext.mPortsInUse[p];

                acParams.nManualConverence = 0;
                acParams.eACMode = OMX_TI_AutoConvergenceModeDisable;

                omxError = OMX_SetConfig( mCurGreContext.mHandleComp,
                                          (OMX_INDEXTYPE)OMX_TI_IndexConfigAutoConvergence,
                                          &acParams
                                         );
                greError = ConvertError(omxError);
                if( OMX_ErrorNone != omxError )
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR OMXVisionCam: Failed to disable auto convergence.\n");
                }

#ifndef USE_SMALC_FOR_STEREO
                // Turn off mechanical misalignment correction.
                mmSettings.nPortIndex = mCurGreContext.mPortsInUse[p];
                mmSettings.bMM = OMX_FALSE;
                omxError = OMX_SetConfig(  mCurGreContext.mHandleComp,
                                           (OMX_INDEXTYPE)OMX_TI_IndexConfigMechanicalMisalignment,
                                           &mmSettings
                                          );
                if( OMX_ErrorNone != omxError )
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR OMXVisionCam: Failed to disable mechanical misalignment correction.\n");
                }
#endif // USE_SMALC_FOR_STEREO

#ifndef USE_VSTAB_FOR_STEREO
                // Turn off video stabilization
                vssEnable.bEnabled = OMX_FALSE;
                omxError = OMX_SetConfig(  mCurGreContext.mHandleComp,
                                           (OMX_INDEXTYPE)OMX_IndexParamFrameStabilisation,
                                           &vssEnable
                                          );
                if( OMX_ErrorNone != omxError )
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR OMXVisionCam: Failed to disable video stabilization.\n");
                }
#endif // USE_VSTAB_FOR_STEREO
            }
            else
            {
                DVP_PRINT(DVP_ZONE_ERROR, "ERROR OMXVisionCam: Failed to set stereo layout\n");
            }
        }
#if 0   // This code is hanging around just to test the non-zero buffer offset for future compatibility
        OMX_CONFIG_BOOLEANTYPE ldc;
        ldc.nSize = sizeof(OMX_CONFIG_BOOLEANTYPE);
        memcpy(&ldc.nVersion, mLocalVersion, sizeof(OMX_VERSIONTYPE));
        ldc.bEnabled = OMX_TRUE; // Just for testing the nOffset
//                ldc.bEnabled = (info->bLensCorrection == true_e ? OMX_TRUE:OMX_FALSE);
        omxError = OMX_SetParameter( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_IndexParamLensDistortionCorrection, &ldc);

        OMX_PARAM_ISONOISEFILTERTYPE iso;
        OMX_STRUCT_INIT(iso, OMX_PARAM_ISONOISEFILTERTYPE, mLocalVersion);
        iso.nPortIndex = VCAM_PORT_ALL;
//                if (info->bNoiseFilter)
            iso.eMode = OMX_ISONoiseFilterModeOn;
//                else
//                    iso.eMode = OMX_ISONoiseFilterModeOff;
        omxError = OMX_SetParameter( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_IndexParamHighISONoiseFiler, &iso);
#endif
    }
    else
    {
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR OMXVisionCam: attempt to set param with id VCAM_PARAM_STEREO_INFO with an invalid args\n");
        greError = STATUS_INVALID_PARAMETER;
    }
    return greError;
}

status_e OMXVisionCam::setCameraOperatingMode(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port __attribute__ ((unused)))
{
    status_e greError = STATUS_BASE;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
    VisionCamCaptureMode mode = *(VisionCamCaptureMode *)param;

    for( int32_t sen = 0; sen < mDetectedSensors.sensorCount; sen++ )
    {
        for( int32_t mod = 0; mod < mDetectedSensors.sensors[sen].supportedModesCnt; mod++ )
        {
            if( mDetectedSensors.sensors[sen].supportedModes[mod] == mode )
            {
                greError = STATUS_SUCCESS;
            }
        }
    }

    if( VCAM_GESTURE_DUAL_SENSOR_MODE == mode && STATUS_SUCCESS == greError )
    {
        OMX_CONFIG_SENSORSELECTTYPE sensor;
        OMX_STRUCT_INIT(sensor, OMX_CONFIG_SENSORSELECTTYPE, mLocalVersion);
        omxError = OMX_GetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexConfigSensorSelect, &sensor);

        if( OMX_ErrorNone == omxError )
        {
            if ( OMX_PrimarySensor != sensor.eSensor )
            {
                greError = STATUS_INVALID_STATE;
                DVP_PRINT( DVP_ZONE_CAM, "You must select primary sensor to get in dual sensor mode!\n");
            }
        }
        else
            greError = STATUS_FAILURE;
    }

    if (param != NULL && STATUS_SUCCESS == greError )
    {
        int value = 0;
        OMX_CONFIG_CAMOPERATINGMODETYPE opMode;
        opMode.nSize = sizeof( OMX_CONFIG_CAMOPERATINGMODETYPE );
        memcpy( &opMode.nVersion, mLocalVersion, sizeof(OMX_VERSIONTYPE) );

        value = getLutValue(mode, VCAM_VALUE_TYPE, CaptureModeLUT, ARR_SIZE(CaptureModeLUT) );

        if (value == STATUS_INVALID_PARAMETER)
            return STATUS_INVALID_PARAMETER; // mode not supported.

        opMode.eCamOperatingMode = (OMX_CAMOPERATINGMODETYPE)value;

        DVP_PRINT(DVP_ZONE_CAM, "Requested VisionCamCaptureMode %d\n", mode);
        DVP_PRINT(DVP_ZONE_CAM, "Requested OMX_CAMOPERATINGMODETYPE %d\n", opMode.eCamOperatingMode);

        bool_e enableAgain[VCAM_PORT_MAX];

        int32_t p;// mark and stop all working ports
        LOOP_PORTS( VCAM_PORT_ALL , p )
        {
            enableAgain[p] = mCurGreContext.mCameraPortParams[p].mIsActive;
            if( mCurGreContext.mCameraPortParams[p].mIsActive )
                portEnableDisable(OMX_CommandPortDisable, freePortBuffersSrvc, (VisionCamPort_e)p );
        }

        int32_t oldstate = getComponentState();
        for(int32_t st = oldstate - 1; st >= OMX_StateLoaded; st--)
        {
            if( OMX_StatePause == oldstate  && OMX_StateExecuting == st )
                continue;

            transitToState( (OMX_STATETYPE)st );
        }

        if( OMX_StateLoaded == getComponentState() || OMX_StateWaitForResources == getComponentState() )
        {
            omxError = OMX_SetParameter( mCurGreContext.mHandleComp,
                                         (OMX_INDEXTYPE)OMX_IndexCameraOperatingMode,
                                         &opMode );

            greError = ConvertError(omxError);
        }

        for(int32_t st = (1 + getComponentState()); st <= oldstate; st++)
        {
            if( OMX_StatePause == oldstate && OMX_StateExecuting == st)
                continue;

            transitToState( (OMX_STATETYPE)st );
        }

        int32_t por;
        LOOP_PORTS( VCAM_PORT_ALL , por )
            if( enableAgain[por])
            {
                portEnableDisable(OMX_CommandPortEnable, populatePortSrvc, (VisionCamPort_e)por);
                fillPortBuffers((VisionCamPort_e)por);
            }
    }
    else
    {
        greError = STATUS_INVALID_PARAMETER;
    }

    return greError;
}

status_e OMXVisionCam::setSensor(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port __attribute__ ((unused)))
{
    status_e vcamError = STATUS_BASE;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
    if (param != NULL)
    {
        omxError = OMX_ErrorHardware;

        OMX_CONFIG_SENSORSELECTTYPE sensor;
        OMX_STRUCT_INIT(sensor, OMX_CONFIG_SENSORSELECTTYPE, mLocalVersion);
        sensor.nPortIndex = mCurGreContext.mPortsInUse[VCAM_PORT_ALL];
        sensor.eSensor = *((OMX_SENSORSELECT*)param);
        for( int32_t cnt = 0; cnt < mDetectedSensors.sensorCount; cnt++ )
        {
            if( sensor.eSensor == mDetectedSensors.sensors[cnt].nIndex )
            {
                DVP_PRINT(DVP_ZONE_CAM, "Selecting sensor index = %u\n", sensor.eSensor);
                omxError = OMX_SetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexConfigSensorSelect, &sensor);
                vcamError = ConvertError(omxError);
                break;
            }
        }
    }
    else
    {
        vcamError = STATUS_INVALID_PARAMETER;
    }

    return vcamError;
}

status_e OMXVisionCam::setExposureCompensation(void * param, size_t size __attribute__ ((unused)), VisionCamPort_e port )
{
    status_e vcamError = STATUS_SUCCESS;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
    OMX_CONFIG_EXPOSUREVALUETYPE expValues;
    OMX_STRUCT_INIT(expValues, OMX_CONFIG_EXPOSUREVALUETYPE, mLocalVersion);

    memcpy( &expValues.nVersion , mLocalVersion , sizeof(OMX_VERSIONTYPE) );
    omxError = OMX_GetConfig( mCurGreContext.mHandleComp , OMX_IndexConfigCommonExposureValue , &expValues );
    vcamError = ConvertError(omxError);

    if( STATUS_SUCCESS == vcamError )
    {
        int32_t compVal = *((int32_t*)param);
        expValues.xEVCompensation = ( compVal * ( 1 << 16 ) )  / 10;

        int32_t p = port;
//                LOOP_PORTS( port , p )
        {
            expValues.nPortIndex = mCurGreContext.mPortsInUse[p];
            omxError = OMX_SetConfig( mCurGreContext.mHandleComp , OMX_IndexConfigCommonExposureValue , &expValues );
            vcamError = ConvertError(omxError);
        }
    }
    return vcamError;
}

status_e OMXVisionCam::setResolution (void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    status_e vcamError = STATUS_SUCCESS;

    if( param != NULL )
    {

        if( *((int32_t*)param) < 0 || *((int32_t*)param) >= VCAM_RESOL_MAX )
        {
           vcamError = STATUS_INVALID_PARAMETER;
        }

        if( STATUS_SUCCESS == vcamError )
        {
            int32_t p = port;
            LOOP_PORTS( port , p )
            {
                int32_t res = *((int32_t*)param);
                if( STATUS_SUCCESS == vcamError )
                {
                    if( mCurGreContext.mCameraPortParams[p].mIsActive )
                    {
                        vcamError = STATUS_INVALID_STATE;
                        break;
                    }
                }

                if( STATUS_SUCCESS == vcamError )
                {
                    mCurGreContext.mCameraPortParams[p].mWidth = VisionCamResolutions[res].mWidth;
                    mCurGreContext.mCameraPortParams[p].mHeight = VisionCamResolutions[res].mHeight;
                }
            }
        }
    }
    else
    {
        vcamError = STATUS_INVALID_PARAMETER;
    }

    return vcamError;
}

status_e OMXVisionCam::setManualExporureTime(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    status_e vcamError = STATUS_BASE;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
    OMX_CONFIG_EXPOSUREVALUETYPE expValues;
    OMX_STRUCT_INIT(expValues, OMX_CONFIG_EXPOSUREVALUETYPE, mLocalVersion);

    int32_t p = port;

//            LOOP_PORTS( port , p )
    {
        expValues.nPortIndex = mCurGreContext.mPortsInUse[p];
        omxError = OMX_GetConfig( mCurGreContext.mHandleComp , OMX_IndexConfigCommonExposureValue , &expValues );
        vcamError = ConvertError(omxError);
        if( STATUS_SUCCESS == vcamError )
        {
            if( *(OMX_U32*)param )
            {
                expValues.nShutterSpeedMsec = *(OMX_U32*)param;
                expValues.bAutoShutterSpeed = OMX_FALSE;
            }
            else
            {
                expValues.bAutoShutterSpeed = OMX_TRUE;

            }

            omxError = OMX_SetConfig( mCurGreContext.mHandleComp , OMX_IndexConfigCommonExposureValue , &expValues );
            vcamError = ConvertError(omxError);
        }
    }
    return vcamError;
}

status_e OMXVisionCam::setISO(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    status_e vcamError = STATUS_BASE;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
    OMX_CONFIG_EXPOSUREVALUETYPE expValues;
    OMX_STRUCT_INIT(expValues, OMX_CONFIG_EXPOSUREVALUETYPE, mLocalVersion);

    int32_t p = port;
//            LOOP_PORTS( port , p )
    {

        expValues.nPortIndex = mCurGreContext.mPortsInUse[p];
        omxError = OMX_GetConfig(mCurGreContext.mHandleComp, OMX_IndexConfigCommonExposureValue, &expValues);
        vcamError = ConvertError(omxError);
        if( STATUS_SUCCESS == vcamError )
        {
            if( *(OMX_U32*)param )
            {
                expValues.nSensitivity = *(OMX_U32*)param;
                expValues.bAutoSensitivity = OMX_FALSE;
            }
            else
                expValues.bAutoSensitivity = OMX_TRUE;

            omxError = OMX_SetConfig( mCurGreContext.mHandleComp , OMX_IndexConfigCommonExposureValue , &expValues );
            vcamError = ConvertError(omxError);
        }
    }
    return vcamError;
}

status_e OMXVisionCam::setWhiteBalanceMode(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    status_e vcamError= STATUS_BASE;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;

    OMX_CONFIG_WHITEBALCONTROLTYPE wb;
    OMX_STRUCT_INIT(wb, OMX_CONFIG_WHITEBALCONTROLTYPE, mLocalVersion);

    int32_t p = port;
//            LOOP_PORTS( port , p )
    {
        wb.nPortIndex = mCurGreContext.mPortsInUse[p];
        wb.eWhiteBalControl = *(OMX_WHITEBALCONTROLTYPE *)param;
        omxError = OMX_SetConfig( mCurGreContext.mHandleComp,
                                OMX_IndexConfigCommonWhiteBalance,
                                &wb);
        vcamError = ConvertError(omxError);
    }
    return vcamError;
}

status_e OMXVisionCam::setColorTemp(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    status_e vcamError= STATUS_BASE;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
    OMX_CONFIG_WHITEBALCONTROLTYPE wb;
    OMX_STRUCT_INIT(wb, OMX_CONFIG_WHITEBALCONTROLTYPE, mLocalVersion);

    if( 0 == *(int32_t*)param )
    {
        wb.eWhiteBalControl = OMX_WhiteBalControlAuto;
    }
    else
    {
        wb.eWhiteBalControl = OMX_WhiteBalControlOff; // @todo change to manual when proper OMX headers arrive
    }

    int32_t p = port;
//            LOOP_PORTS( port , p )
    {
        wb.nPortIndex = mCurGreContext.mPortsInUse[p];
        omxError = OMX_SetConfig( mCurGreContext.mHandleComp,
                             OMX_IndexConfigCommonWhiteBalance,
                             &wb );
        vcamError = ConvertError(omxError);
        if( STATUS_SUCCESS == vcamError )
        {
#if 0
            OMX_CONFIG_WHITEBALANCECOLORTEMPTPYPE colTemp;
            OMX_STRUCT_INIT(colTemp, OMX_CONFIG_WHITEBALANCECOLORTEMPTPYPE, mLocalVersion);
            colTemp.nPortIndex = mCurGreContext.mPortsInUse[port];
            colTemp.nColorTemperature = *(OMX_U32 *)param;

            omxError = OMX_SetConfig(mCurGreContext.mHandleComp,
                                   OMX_IndexConfigWhiteBalanceManualColorTemp,
                                   &colTemp );
            vcamError = ConvertError(omxError);
#endif
        }
    }
    return vcamError;
}

status_e OMXVisionCam::setMirror(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    status_e vcamError = STATUS_BASE;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
    OMX_CONFIG_MIRRORTYPE mirror;
    OMX_STRUCT_INIT(mirror, OMX_CONFIG_MIRRORTYPE, mLocalVersion);

    int32_t result = (OMX_MIRRORTYPE)getLutValue( *(int32_t*)param, VCAM_VALUE_TYPE,
                                                  MirrorTypeLUT, ARR_SIZE(MirrorTypeLUT)
                                                );

    if( result != STATUS_INVALID_PARAMETER )
    {
        mirror.eMirror = (OMX_MIRRORTYPE)result;

        int32_t p;
        LOOP_PORTS( port , p )
        {
            mirror.nPortIndex = mCurGreContext.mPortsInUse[p];
            omxError = OMX_SetConfig( mCurGreContext.mHandleComp,
                                    OMX_IndexConfigCommonMirror,
                                    &mirror );
            vcamError = ConvertError(omxError);
            if( omxError != OMX_ErrorNone )
            {
                break;
            }
        }
    }
    else
    {
        vcamError = STATUS_INVALID_PARAMETER;
    }

    return vcamError;
}

#if ( defined(DUCATI_1_5) || defined(DUCATI_2_0) ) && defined(OMX_CAMERA_SUPPORTS_MANUAL_CONTROLS)
status_e OMXVisionCam::setAWBminDelayTime(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    status_e vcamError = STATUS_BASE;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;

    uint32_t timeDelay = *((uint32_t*)param);

    if( timeDelay > AE_Delay_Time_Max )
    {
        vcamError = STATUS_INVALID_PARAMETER;
    }
    else
    {
        OMX_TI_CONFIG_AWB_DELAY awbDelay;
        OMX_STRUCT_INIT( awbDelay, OMX_TI_CONFIG_AWB_DELAY, mLocalVersion );
        awbDelay.nDelayTime = timeDelay;

        int32_t p = port;
//                LOOP_PORTS( port , p )
        {
            awbDelay.nPortIndex = p;
            omxError = OMX_SetConfig( mCurGreContext.mHandleComp,
                                        ( OMX_INDEXTYPE )OMX_TI_IndexConfigAWBMinDelayTime,
                                        &awbDelay
                                    );
        }
        vcamError = ConvertError(omxError);
    }
    return vcamError;
}

status_e OMXVisionCam::setGestureInfo(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    status_e greError = STATUS_BASE;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;

    VisionCamGestureInfo *info = (VisionCamGestureInfo*)param;
    if( info->mGestureType >= VCAM_GESTURE_EVENT_MAX || info->mGestureType < VCAM_GESTURE_EVENT_INVALID )
    {
        greError= STATUS_INVALID_PARAMETER;
    }

    if( info->mRegionsNum >= VCAM_Max_Gesture_Per_Frame )
    {
        greError= STATUS_INVALID_PARAMETER;
    }

    if( STATUS_SUCCESS == greError )
    {
        int value = 0;
        OMX_TI_CONFIG_GESTURES_INFO gestInfo;
        OMX_STRUCT_INIT( gestInfo, OMX_TI_CONFIG_GESTURES_INFO , mLocalVersion );

        gestInfo.nTimeStamp = info->timeStamp;
        gestInfo.nNumDetectedGestures = info->mRegionsNum;
        value = getLutValue( info->mGestureType, VCAM_VALUE_TYPE,
                             GestureTypeLUT, ARR_SIZE(GestureTypeLUT)
                             );
        if ( value != STATUS_INVALID_PARAMETER )
        {
            gestInfo.eType = (OMX_TI_GESTURES_TYPE)value;
            for(uint32_t i = 0; i < info->mRegionsNum ; i++ )
            {
                OMX_STRUCT_INIT(gestInfo.nGestureAreas[i], OMX_CONFIG_OBJECT_RECT_TYPE , mLocalVersion );
                value = getLutValue( (int)(info->mRegions[i].mObjType), VCAM_VALUE_TYPE,
                                     ObjectTypeLUT, ARR_SIZE(ObjectTypeLUT)
                                   );
                if (value == STATUS_INVALID_PARAMETER)
                    continue;
                gestInfo.nGestureAreas[i].eType = (OMX_TI_OBJECT_TYPE)value;
                gestInfo.nGestureAreas[i].nTop = info->mRegions[i].mTop;
                gestInfo.nGestureAreas[i].nLeft = info->mRegions[i].mLeft;
                gestInfo.nGestureAreas[i].nWidth = info->mRegions[i].mWidth;
                gestInfo.nGestureAreas[i].nHeight = info->mRegions[i].mHeight;
            }

            if( OMX_TI_GESTURE_NO_GESTURE != gestInfo.eType)
            {
                int32_t p = port;
    //            LOOP_PORTS( port , p )
                {
                    gestInfo.nPortIndex = mCurGreContext.mPortsInUse[p];
                    omxError = OMX_SetConfig( mCurGreContext.mHandleComp,
                                            (OMX_INDEXTYPE)OMX_TI_IndexConfigDetectedGesturesInfo,
                                            &gestInfo
                                          );
                }
                greError = ConvertError(omxError);
            }
            else
            {
                greError = STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            greError = (status_e)value;
        }
    }
    return greError;
}

status_e OMXVisionCam::setAGC_MinimumDelay(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    status_e vcamError = STATUS_BASE;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
    int32_t delay = *((int32_t*)param);
    OMX_TI_CONFIG_AE_DELAY agcDelTime;
    OMX_STRUCT_INIT( agcDelTime, OMX_TI_CONFIG_AE_DELAY, mLocalVersion );
    agcDelTime.nDelayTime = delay;

    int32_t p = port;
//            LOOP_PORTS( port , p )
    {
        agcDelTime.nPortIndex = p;
        omxError = OMX_SetConfig( mCurGreContext.mHandleComp,
                                (OMX_INDEXTYPE)OMX_TI_IndexConfigAutoExpMinDelayTime,
                                &agcDelTime );
        vcamError = ConvertError(omxError);
    }
    return vcamError;
}

status_e OMXVisionCam::setAGC_LowThreshold(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    status_e vcamError = STATUS_BASE;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
    int32_t lowTH = *((int32_t*)param);
    OMX_TI_CONFIG_AE_THRESHOLD ae;

    OMX_STRUCT_INIT( ae, OMX_TI_CONFIG_AE_THRESHOLD, mLocalVersion );

    int32_t p = port;
//            LOOP_PORTS( port , p )
    {
        ae.nPortIndex = p;
        omxError = OMX_GetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexConfigAutoExpThreshold, &ae );

        vcamError = ConvertError(omxError);
        if( STATUS_SUCCESS == vcamError )
        {
            ae.uMinTH = lowTH;
            omxError = OMX_SetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexConfigAutoExpThreshold, &ae );
            vcamError = ConvertError(omxError);
        }
    }
    return vcamError;
}

status_e OMXVisionCam::setAGC_HighThreshold(void *param, size_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    status_e vcamError = STATUS_BASE;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
    int32_t highTH = *((int32_t*)param);

    OMX_TI_CONFIG_AE_THRESHOLD ae;
    OMX_STRUCT_INIT( ae, OMX_TI_CONFIG_AE_THRESHOLD, mLocalVersion );

    int32_t p = port;
//            LOOP_PORTS( port , p )
    {
        ae.nPortIndex = p;
        omxError = OMX_GetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexConfigAutoExpThreshold, &ae );
        vcamError = ConvertError(omxError);
        if( STATUS_SUCCESS == vcamError )
        {
            ae.uMaxTH = highTH;
            omxError = OMX_SetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexConfigAutoExpThreshold, &ae );
            vcamError = ConvertError(omxError);
        }
    }
    return vcamError;
}

#endif

#if defined(EXPORTED_3A)
status_e OMXVisionCam::_3A_ApplyExported()
{
    status_e ret = STATUS_BASE;
    bool_e user3Aheld = m3A_Export.isHeld();

    if ( false_e == user3Aheld )
    {
        OMX_ERRORTYPE omxStatus = OMX_ErrorUndefined;
        OMX_TI_CONFIG_SHAREDBUFFER skipBuffer;

        OMX_STRUCT_INIT( skipBuffer, OMX_TI_CONFIG_SHAREDBUFFER , mLocalVersion )

        skipBuffer.nPortIndex = OMX_ALL; // port;
        skipBuffer.pSharedBuff = (OMX_U8*)m3A_Export.d3A_Buffer->getData();
        skipBuffer.nSharedBuffSize = m3A_Export.d3A_Buffer->getSize();

        omxStatus = OMX_SetConfig( mCurGreContext.mHandleComp,
                                   (OMX_INDEXTYPE)OMX_TI_IndexConfigAAAskipBuffer,
                                   &skipBuffer
                                    );
        ret = ConvertError(omxStatus);
    }
    else
    {
        ret = STATUS_INVALID_STATE;
    }
    return ret;
}
#endif

status_e OMXVisionCam::setPreviewHeight(void* param, uint32_t size __attribute__((unused)), VisionCamPort_e port)
{
    int32_t p;
    LOOP_PORTS( port , p )
        mCurGreContext.mCameraPortParams[p].mHeight = *((OMX_U32*)param);
    return STATUS_SUCCESS;
}

status_e OMXVisionCam::setPreaviewWidth(void* param, uint32_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    int32_t p;
    LOOP_PORTS( port , p )
            mCurGreContext.mCameraPortParams[p].mWidth = *((OMX_U32*)param);
    return STATUS_SUCCESS;
}

status_e OMXVisionCam::startAutoFocus(void* param, uint32_t size __attribute__ ((unused)), VisionCamPort_e port __attribute__ ((unused)))
{
    return startAutoFocus( *((VisionCamFocusMode*)param) );
}

status_e OMXVisionCam::startManualFocus(void* param, uint32_t size __attribute__ ((unused)), VisionCamPort_e port __attribute__ ((unused)))
{
    mManualFocusDistance = *((uint32_t*)param);
    return startAutoFocus( VCAM_FOCUS_CONTROL_ON );
}

#ifndef EXPORTED_3A
status_e OMXVisionCam::setWBalColorGains(void* param, uint32_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    status_e vcamError = STATUS_BASE;
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
#ifdef USE_WB_GAIN_PATCH
    VisionCamWhiteBalGains wbGains = *((VisionCamWhiteBalGains*)param);
    uint16_t * tmp;
    CALCULATE_WB_GAINS_OFFSET(uint16_t,mWBbuffer,tmp);

    tmp[ RED ] = wbGains.mRed;
    tmp[ GREEN_RED ] = wbGains.mGreen_r;
    tmp[ GREEN_BLUE ] = wbGains.mGreen_b;
    tmp[ BLUE ] = wbGains.mBlue;

    OMX_TI_CONFIG_SHAREDBUFFER skipBuffer;
    skipBuffer.nSize = sizeof( OMX_TI_CONFIG_SHAREDBUFFER );
    memcpy( &skipBuffer.nVersion , mLocalVersion , sizeof(OMX_VERSIONTYPE) );

    if( wbGains.mRed >= COLOR_GAIN_MIN &&  wbGains.mRed <= COLOR_GAIN_MAX
        && wbGains.mGreen_b >= COLOR_GAIN_MIN && wbGains.mGreen_b <= COLOR_GAIN_MAX
        && wbGains.mGreen_r >= COLOR_GAIN_MIN && wbGains.mGreen_r <= COLOR_GAIN_MAX
        && wbGains.mBlue >= COLOR_GAIN_MIN && wbGains.mBlue <= COLOR_GAIN_MAX )
    {
        skipBuffer.pSharedBuff = (OMX_U8*)mWBbuffer;
        skipBuffer.nSharedBuffSize = sizeof(mWBbuffer);
    }
    else if( !wbGains.mRed && !wbGains.mGreen_b && !wbGains.mGreen_r && !wbGains.mBlue )
    {   /// all gains are zero => auto mode
        skipBuffer.pSharedBuff = (OMX_U8*)mWBresetBuffer;
        skipBuffer.nSharedBuffSize = sizeof(mWBresetBuffer);
    }
    else
    {
        vcamError = STATUS_INVALID_PARAMETER;
        return vcamError;
    }

    int32_t p = port;
    //            LOOP_PORTS(port, p)
    {
        skipBuffer.nPortIndex = p;
        omxError = OMX_SetConfig( mCurGreContext.mHandleComp,
                                (OMX_INDEXTYPE) OMX_TI_IndexConfigAAAskipBuffer,
                                &skipBuffer );
    }
    vcamError = ConvertError(omxError);
#endif // USE_WB_GAIN_PATCH
    return vcamError;
}
#endif // EXPORTED_3A

#ifndef EXPORTED_3A
status_e OMXVisionCam::setGammaTableColorGains(void* param, uint32_t size __attribute__ ((unused)), VisionCamPort_e port __attribute__ ((unused)))
{
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;

    VisionCamGammaTableType *gammaTbl = (VisionCamGammaTableType*)(param);
    OMX_TI_CONFIG_SHAREDBUFFER skipBuffer;
    OMX_STRUCT_INIT(skipBuffer, OMX_TI_CONFIG_SHAREDBUFFER, mLocalVersion);

    uint32_t* base = (uint32_t *)(mGammaTablesBuf + 12); // 12 bytes offset for first table
    uint16_t *redTbl      = (uint16_t*)(base[0] + (uint32_t)&base[2]);
    uint16_t *blueTbl     = (uint16_t*)(base[1] + (uint32_t)&base[2]);
    uint16_t *greenTbl    = (uint16_t*)(base[2] + (uint32_t)&base[2]);

#ifdef _USE_GAMMA_RESET_HC_
    if( !mGammaResetPolulated )
    {
        skipBuffer.pSharedBuff = (OMX_U8*)mGammaResetTablesBuf;
        skipBuffer.nSharedBuffSize = sizeof(mGammaResetTablesBuf);
        omxError = OMX_GetConfig( mCurGreContext.mHandleComp,
                            (OMX_INDEXTYPE) OMX_TI_IndexConfigAAAskipBuffer,
                            &skipBuffer );

        if( OMX_ErrorNone == omxError )
            mGammaResetPolulated = true;
    }
#endif // _USE_GAMMA_RESET_HC_

    if( gammaTbl->mRedTable && gammaTbl->mGreenTable && gammaTbl->mBlueTable )
    {
        if( gammaTbl->mRedTable != redTbl )
        {
            memcpy(redTbl, gammaTbl->mRedTable, GAMMA_TABLE_SIZE );
        }

        if( gammaTbl->mGreenTable != greenTbl )
        {
            memcpy(greenTbl, gammaTbl->mGreenTable, GAMMA_TABLE_SIZE );
        }

        if( gammaTbl->mBlueTable != blueTbl )
        {
            memcpy(blueTbl, gammaTbl->mBlueTable, GAMMA_TABLE_SIZE );
        }

        skipBuffer.pSharedBuff = (OMX_U8*)mGammaTablesBuf;
        skipBuffer.nSharedBuffSize = sizeof(mGammaTablesBuf)/sizeof(mGammaTablesBuf[0]);
    }
    else
    {
#ifdef _USE_GAMMA_RESET_HC_
    if( mGammaResetPolulated )
    {
        skipBuffer.pSharedBuff = (OMX_U8*)mGammaResetTablesBuf;
        skipBuffer.nSharedBuffSize = sizeof(mGammaResetTablesBuf)/sizeof(mGammaResetTablesBuf[0]);
    }
    else
    {
        DVP_PRINT(DVP_ZONE_WARNING, "No data present in reset Gamma Tables. Leaving Gamma unchanged!!!");
    }
#else
    skipBuffer.pSharedBuff = (OMX_U8*)mGammaResetTablesBuf;
    skipBuffer.nSharedBuffSize = sizeof(mGammaResetTablesBuf)/sizeof(mGammaResetTablesBuf[0]);
#endif // _USE_GAMMA_RESET_HC_
    }

    omxError = OMX_SetConfig( mCurGreContext.mHandleComp,
                            (OMX_INDEXTYPE) OMX_TI_IndexConfigAAAskipBuffer,
                            &skipBuffer );
    return ConvertError(omxError);

}
#endif // EXPORTED_3A

status_e OMXVisionCam::setFormatRotation(void* param, uint32_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    int32_t p;
    LOOP_PORTS( port , p )
    {
        mCurGreContext.mCameraPortParams[p].mRotation = *((OMX_S32 *)param);
    }
    return STATUS_SUCCESS;
}

status_e OMXVisionCam::setPreviewRotation(void* param, uint32_t size __attribute__ ((unused)), VisionCamPort_e port)
{
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
    OMX_CONFIG_ROTATIONTYPE rotation;
    OMX_STRUCT_INIT(rotation, OMX_CONFIG_ROTATIONTYPE, mLocalVersion);
    rotation.nRotation = *((OMX_S32*)param);

    int32_t p;
    LOOP_PORTS( port , p )
    {
        rotation.nPortIndex = mCurGreContext.mPortsInUse[p];
        OMX_CHECK(omxError, OMX_SetConfig(mCurGreContext.mHandleComp,
                                          OMX_IndexConfigCommonRotate,
                                          &rotation));
        DVP_PRINT(DVP_ZONE_CAM, "Setting Rotation to %ld (size: %u)\n", rotation.nRotation, sizeof(rotation));
    }
    return ConvertError(omxError);
}

#if defined (EXPORTED_3A)
status_e OMXVisionCam::startCollectingMaual3AParams(void* param __attribute__ ((unused)), uint32_t size __attribute__ ((unused)), VisionCamPort_e port __attribute__ ((unused)))
{
    return m3A_Export.hold_3A();
}

status_e OMXVisionCam::setManual3AParam(void* param, uint32_t size, VisionCamPort_e port __attribute__ ((unused)))
{
    VisionCam_3Asettings_Base_t* p = (VisionCam_3Asettings_Base_t* )param;
    status_e vcamError = m3A_Export.set( p->eParamType, p->pData, size );

    if( !m3A_Export.isHeld() && STATUS_SUCCESS == vcamError )
    {
        vcamError = _3A_ApplyExported();
    }
    return vcamError;
}

status_e OMXVisionCam::applyCollectedManual3AParams(void* param __attribute__ ((unused)), uint32_t size __attribute__ ((unused)), VisionCamPort_e port __attribute__ ((unused)))
{
    status_e vcamError = STATUS_BASE;
    vcamError = m3A_Export.release_3A();

    if( STATUS_SUCCESS == vcamError )
    {
        vcamError = _3A_ApplyExported();
    }
    return vcamError;
}

status_e OMXVisionCam::resetManual3AParams(void* param __attribute__ ((unused)), uint32_t size __attribute__ ((unused)), VisionCamPort_e port __attribute__ ((unused)))
{
    return m3A_Export.reset();
}
#endif //  EXPORTED_3A

/*
*  APIs to get configured Vision Cam parameters
*/
status_e OMXVisionCam::getParameter( VisionCamParam_e paramId, void* param, uint32_t size, VisionCamPort_e port)
{
    status_e greError = STATUS_SUCCESS;
    OMX_ERRORTYPE omxError = OMX_ErrorNone;

    SOSAL::AutoLock lock( &mUserRequestLock );

    if (param == NULL || size == 0 || port >= VCAM_PORT_MAX)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "NULL param pointer passed to %s\n", __func__);
        return STATUS_INVALID_PARAMETER;
    }
    else
    {
        DVP_PRINT(DVP_ZONE_CAM, "GET PARAM: 0x%04x, %p, %u (0x%08x)\n", paramId, param, size, (size==4?*(uint32_t *)param:0));
    }

    if( VCAM_PORT_ALL == port)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "%s called on port ALL. Please specifu a port!\n", __func__);
        return STATUS_INVALID_PARAMETER;
    }
    switch( paramId )
    {
        case VCAM_PARAM_HEIGHT:
        {
            OMX_PARAM_PORTDEFINITIONTYPE portCheck;
            omxError = initPortCheck( &portCheck , port );

            if( OMX_ErrorNone == omxError )
            {
                *(int*)param = portCheck.format.video.nFrameHeight;
            }
            break;
        }

        case VCAM_PARAM_WIDTH:
        {
            OMX_PARAM_PORTDEFINITIONTYPE portCheck;
            omxError = initPortCheck( &portCheck , port );

            if( OMX_ErrorNone == omxError )
            {
                *(int*)param = portCheck.format.video.nFrameWidth;
            }
            break;
        }

        case VCAM_PARAM_COLOR_SPACE_FOURCC:
        {
            int i = 0;
            OMX_PARAM_PORTDEFINITIONTYPE portCheck;
            omxError = initPortCheck( &portCheck, port );

            if( OMX_ErrorNone == omxError )
            {
                while( ColorFormatLUT[ i ][ 1 ] != OMX_COLOR_FormatMax )
                {
                    if( ColorFormatLUT[ i ][ 1 ] == portCheck.format.video.eColorFormat )
                    {
                        *( (int*)param) = ColorFormatLUT[ i ][ 0 ];
                        break;
                    }
                    i++;
                }
            }
            break;
        }

        case VCAM_PARAM_DO_AUTOFOCUS:
        {
            int i = 0;
            OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE focus;
            OMX_STRUCT_INIT(focus, OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE, mLocalVersion);

            omxError = OMX_GetConfig(mCurGreContext.mHandleComp, OMX_IndexConfigFocusControl, &focus);
            if( OMX_ErrorNone == omxError )
            {
                for( i = 0; i < VCAM_FOCUS_CONTROL_MAX; i++ )
                {
                    if ( FocusModeLUT[ i ][ 1 ] == focus.eFocusControl )
                    {
                        *(int*)param = FocusModeLUT[ i ][ 0 ];
                        break;
                    }
                }
                if( VCAM_FOCUS_CONTROL_MAX == i )
                    greError = STATUS_INVALID_PARAMETER;
            }

            break;
        }

        case VCAM_PARAM_DO_MANUALFOCUS:
        {
            OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE focus;
            OMX_STRUCT_INIT(focus, OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE, mLocalVersion);

            omxError = OMX_GetConfig(mCurGreContext.mHandleComp, OMX_IndexConfigFocusControl, &focus);
            if( OMX_ErrorNone == omxError )
            {
                (*(int*)param) = focus.nFocusSteps;
            }
            break;
        }

        case VCAM_PARAM_BRIGHTNESS:
        {
            OMX_CONFIG_BRIGHTNESSTYPE brightness;
            OMX_STRUCT_INIT(brightness, OMX_CONFIG_BRIGHTNESSTYPE, mLocalVersion );

            omxError = OMX_GetConfig( mCurGreContext.mHandleComp, OMX_IndexConfigCommonBrightness, &brightness);
            if( OMX_ErrorNone == omxError )
            {
                *((uint32_t*)param) = brightness.nBrightness;
            }
            break;
        }

        case VCAM_PARAM_CONTRAST:
        {
            OMX_CONFIG_CONTRASTTYPE contrast;
            OMX_STRUCT_INIT(contrast, OMX_CONFIG_CONTRASTTYPE, mLocalVersion );

            omxError = OMX_GetConfig( mCurGreContext.mHandleComp, OMX_IndexConfigCommonContrast, &contrast);
            if( OMX_ErrorNone == omxError )
            {
                *((int*)param) = contrast.nContrast;
            }
            break;
        }

        case VCAM_PARAM_SHARPNESS:
        {
            OMX_IMAGE_CONFIG_PROCESSINGLEVELTYPE procSharpness;
            OMX_STRUCT_INIT( procSharpness, OMX_IMAGE_CONFIG_PROCESSINGLEVELTYPE, mLocalVersion );

            omxError = OMX_GetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_IndexConfigSharpeningLevel, &procSharpness);

            if( OMX_ErrorNone == omxError )
            {
                if( OMX_TRUE == procSharpness.bAuto )
                    *((int*)param) = 0;
                else
                    *((int*)param) = procSharpness.nLevel;
            }
            break;
        }

        case VCAM_PARAM_SATURATION:
        {
            OMX_CONFIG_SATURATIONTYPE saturation;
            OMX_STRUCT_INIT(saturation, OMX_CONFIG_SATURATIONTYPE, mLocalVersion);
            omxError = OMX_GetConfig( mCurGreContext.mHandleComp, OMX_IndexConfigCommonSaturation, &saturation);

            if( OMX_ErrorNone == omxError )
            {
                *((int*)param) = saturation.nSaturation;
            }
            break;
        }

        case VCAM_PARAM_FPS_FIXED:
        {
            OMX_PARAM_PORTDEFINITIONTYPE portCheck;
            omxError = initPortCheck( &portCheck , port );

            if( OMX_ErrorNone == omxError )
            {
                *((uint32_t*)param) = portCheck.format.video.xFramerate >> 16;
            }
            break;
        }

        case VCAM_PARAM_FPS_VAR:
        {
            DVP_PRINT(DVP_ZONE_WARNING, "No Getting Api for this Parameter %d\n", paramId);
            break;
        }

        case VCAM_PARAM_FLICKER:
        {
            OMX_CONFIG_FLICKERCANCELTYPE flicker;
            OMX_STRUCT_INIT( flicker, OMX_CONFIG_FLICKERCANCELTYPE, mLocalVersion );
            omxError = OMX_GetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_IndexConfigFlickerCancel, &flicker );

            if( OMX_ErrorNone == omxError )
            {
                *((int32_t*)param) = flicker.eFlickerCancel;
            }
            break;
        }

        case VCAM_PARAM_CROP:
        {
            OMX_CONFIG_RECTTYPE crop;
            OMX_STRUCT_INIT(crop, OMX_CONFIG_RECTTYPE, mLocalVersion);

            omxError = OMX_GetConfig( mCurGreContext.mHandleComp, OMX_IndexConfigCommonOutputCrop, &crop );

            if( OMX_ErrorNone == omxError )
            {
                ((VisionCamRectType*)param)->mLeft = crop.nLeft;
                ((VisionCamRectType*)param)->mTop = crop.nTop;
                ((VisionCamRectType*)param)->mWidth = crop.nWidth;
                ((VisionCamRectType*)param)->mHeight = crop.nHeight;
            }
          break;
        }

        case VCAM_PARAM_CAP_MODE:
        {
            if (param != NULL)
            {
                int i = 0;
                OMX_CONFIG_CAMOPERATINGMODETYPE opMode;
                opMode.nSize = sizeof(OMX_CONFIG_CAMOPERATINGMODETYPE);
                memcpy(&opMode .nVersion, mLocalVersion , sizeof(OMX_VERSIONTYPE));

                omxError = OMX_GetParameter( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_IndexCameraOperatingMode, &opMode );

                if( OMX_ErrorNone == omxError )
                {
                    for (i = 0; i < VCAM_CAP_MODE_MAX; i++)
                    {
                        if (CaptureModeLUT[i][1] == opMode.eCamOperatingMode)
                        {
                            memcpy( param, &CaptureModeLUT[i][0], sizeof(int32_t) );
                            break;
                        }
                    }

                    if( VCAM_CAP_MODE_MAX == CaptureModeLUT[i][0])
                    {
                        greError = STATUS_INVALID_PARAMETER;
                        break;
                    }
                }
            }
            break;
        }

        case VCAM_PARAM_SENSOR_SELECT:
        {
            OMX_CONFIG_SENSORSELECTTYPE sensor;
            OMX_STRUCT_INIT(sensor, OMX_CONFIG_SENSORSELECTTYPE, mLocalVersion);

            omxError = OMX_GetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexConfigSensorSelect, &sensor);

            if( OMX_ErrorNone == omxError)
            {
                *((int*)param) = sensor.eSensor;
            }
            break;
        }

        case VCAM_PARAM_EXPOSURE_COMPENSATION:
        {
            OMX_CONFIG_EXPOSUREVALUETYPE expValues;
            OMX_STRUCT_INIT( expValues, OMX_CONFIG_EXPOSUREVALUETYPE, mLocalVersion );
            omxError = OMX_GetConfig( mCurGreContext.mHandleComp , OMX_IndexConfigCommonExposureValue , &expValues );

            if( OMX_ErrorNone == omxError )
            {
                expValues.xEVCompensation *= 10;
                int compVal = expValues.xEVCompensation / (1 << 16);
                *((int*)param) = compVal;
            }
            break;
        }

        case VCAM_PARAM_RESOLUTION:
        {
            int i = VCAM_RESOL_MAX;
            unsigned int width, height;
            OMX_PARAM_PORTDEFINITIONTYPE portCheck;
            omxError = initPortCheck( &portCheck , port );

            if( OMX_ErrorNone == omxError )
            {
                width = portCheck.format.video.nFrameWidth;
                height = portCheck.format.video.nFrameHeight;
                for( i = 0; i < VCAM_RESOL_MAX; i++)
                {
                    if( VisionCamResolutions[i].mWidth == width
                        && VisionCamResolutions[i].mHeight == height)
                    {
                        *((int*)param) = i;
                        break;
                    }
                }
                if( VCAM_RESOL_MAX == i )
                {
                    greError = STATUS_INVALID_PARAMETER;
                }
            }
            break;
        }

        case VCAM_PARAM_2DBUFFER_DIM:
        {
            OMX_CONFIG_RECTTYPE frame;
            OMX_STRUCT_INIT(frame, OMX_CONFIG_RECTTYPE, mLocalVersion);
            VisionCamResType *pRes = (VisionCamResType *)param;

            frame.nPortIndex = mCurGreContext.mPortsInUse[port];
            if (pRes && size == sizeof(VisionCamResType))
            {
                // Set the port definition to update width and height first.
                omxError = setPortDef( port );

                if( OMX_ErrorNone == omxError )
                {
                    omxError = OMX_GetParameter(mCurGreContext.mHandleComp,
                                                (OMX_INDEXTYPE)OMX_TI_IndexParam2DBufferAllocDimension,
                                                &frame);
                }

                if (OMX_ErrorNone == omxError)
                {
                    pRes->mWidth = frame.nWidth;
                    pRes->mHeight = frame.nHeight;
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "Failed to query the 2D Buffer Dimensions! (err=0x%08x)\n", omxError);
                    memset(pRes, 0, sizeof(VisionCamResType));
                    // omxError will convert to greError
                }
            }
            else
                greError = STATUS_INVALID_PARAMETER;
            break;
        }

        case VCAM_PARAM_MANUAL_EXPOSURE:
        {
            OMX_CONFIG_EXPOSUREVALUETYPE expValues;
            OMX_STRUCT_INIT(expValues, OMX_CONFIG_EXPOSUREVALUETYPE, mLocalVersion);

            omxError = OMX_GetConfig( mCurGreContext.mHandleComp , OMX_IndexConfigCommonExposureValue , &expValues );

            if( OMX_ErrorNone == omxError )
            {
                if( OMX_TRUE == expValues.bAutoShutterSpeed )
                {
                    *(OMX_U32*)param = 0;
                }
                else
                {
                    *(OMX_U32*)param = expValues.nShutterSpeedMsec;
                }
            }
            break;
        }

        case VCAM_PARAM_EXPOSURE_ISO:
        {
            OMX_CONFIG_EXPOSUREVALUETYPE expValues;
            OMX_STRUCT_INIT( expValues, OMX_CONFIG_EXPOSUREVALUETYPE, mLocalVersion );

            omxError = OMX_GetConfig( mCurGreContext.mHandleComp , OMX_IndexConfigCommonExposureValue , &expValues );

            if( OMX_ErrorNone == omxError )
            {
                if( OMX_TRUE == expValues.bAutoSensitivity )
                {
                    *(uint32_t*)param = 0;
                }
                else
                    *(uint32_t*)param = expValues.nSensitivity;
            }
            break;
        }

        case VCAM_PARAM_AWB_MODE:
        {
            OMX_CONFIG_WHITEBALCONTROLTYPE wb;
            OMX_STRUCT_INIT(wb, OMX_CONFIG_WHITEBALCONTROLTYPE, mLocalVersion );

            omxError = OMX_GetConfig( mCurGreContext.mHandleComp,
                                 OMX_IndexConfigCommonWhiteBalance,
                                 &wb);

            if( OMX_ErrorNone == omxError )
            {
                *(int *)param = (int)wb.eWhiteBalControl;
            }
            break;
        }

        case VCAM_PARAM_COLOR_TEMP:
        {
            DVP_PRINT(DVP_ZONE_WARNING, "No Getting Api for this Parameter %d\n", paramId);
            break;
        }

#ifndef EXPORTED_3A
        case VCAM_PARAM_WB_COLOR_GAINS:
        {
#ifdef USE_WB_GAIN_PATCH
            OMX_TI_CONFIG_SHAREDBUFFER skipBuffer;

            skipBuffer.nSize = sizeof( OMX_TI_CONFIG_SHAREDBUFFER );
            memcpy( &skipBuffer.nVersion , mLocalVersion , sizeof(OMX_VERSIONTYPE) );
            skipBuffer.pSharedBuff = (OMX_U8*)mWBbuffer;
            skipBuffer.nSharedBuffSize = sizeof(mWBbuffer);

            omxError = OMX_GetConfig( mCurGreContext.mHandleComp,
                                    (OMX_INDEXTYPE) OMX_TI_IndexConfigAAAskipBuffer,
                                    &skipBuffer );

            if( OMX_ErrorNone == omxError )
            {
                VisionCamWhiteBalGains wbGains;
                uint16_t * tmp;
                CALCULATE_WB_GAINS_OFFSET(uint16_t, mWBbuffer, tmp );

                wbGains.mRed = tmp[ RED ];
                wbGains.mGreen_r = tmp[ GREEN_RED ];
                wbGains.mGreen_b = tmp[ GREEN_BLUE ];
                wbGains.mBlue = tmp[ BLUE ];

                *((VisionCamWhiteBalGains*)param) = wbGains;
            }
#endif // USE_WB_GAIN_PATCH
            break;
        }

        case VCAM_PARAM_GAMMA_TBLS:
        {

            VisionCamGammaTableType *gammaTbl = (VisionCamGammaTableType*)param;
            OMX_TI_CONFIG_SHAREDBUFFER skipBuffer;
            skipBuffer.nSize = sizeof( OMX_TI_CONFIG_SHAREDBUFFER );
            memcpy( &skipBuffer.nVersion , mLocalVersion , sizeof(OMX_VERSIONTYPE) );

            skipBuffer.pSharedBuff = (OMX_U8*)mGammaTablesBuf;
            skipBuffer.nSharedBuffSize = sizeof(mGammaTablesBuf);

            omxError = OMX_GetConfig( mCurGreContext.mHandleComp,
                                (OMX_INDEXTYPE) OMX_TI_IndexConfigAAAskipBuffer,
                                &skipBuffer );

            if( OMX_ErrorNone == omxError )
            {
                gammaTbl->mTableSize     = GAMMA_TABLE_SIZE;

                uint32_t* base = (uint32_t *)(mGammaTablesBuf + 12); // 12 bytes offset for red table
                gammaTbl->mRedTable      = (uint16_t*)(base[0] + (uint32_t)&base[2]);
                gammaTbl->mBlueTable     = (uint16_t*)(base[1] + (uint32_t)&base[2]);
                gammaTbl->mGreenTable    = (uint16_t*)(base[2] + (uint32_t)&base[2]);
            }
            break;
        }
#endif // EXPORTED_3A

        case VCAM_PARAM_ROTATION:
        {
            OMX_CONFIG_ROTATIONTYPE rotation;
            OMX_STRUCT_INIT(rotation, OMX_CONFIG_ROTATIONTYPE, mLocalVersion);
            omxError = OMX_GetConfig(mCurGreContext.mHandleComp,
                                     OMX_IndexConfigCommonRotate,
                                     &rotation);
            if (OMX_ErrorNone == omxError)
                *((OMX_S32*)param) = rotation.nRotation;
            break;
        }

        case VCAM_PARAM_MIRROR:
        {
            if( VCAM_PORT_ALL == port )
            {
                greError = STATUS_INVALID_PARAMETER;
            }
            else
            {
                int i;
                OMX_CONFIG_MIRRORTYPE mirror;
                mirror.nSize = sizeof( OMX_CONFIG_MIRRORTYPE );
                mirror.nPortIndex = mCurGreContext.mPortsInUse[port];
                memcpy( &mirror.nVersion, mLocalVersion, sizeof(OMX_VERSIONTYPE));

                omxError = OMX_GetConfig( mCurGreContext.mHandleComp,
                                        OMX_IndexConfigCommonMirror,
                                        &mirror );

                if( OMX_ErrorNone == omxError )
                {
                    for( i = 0; i < VCAM_MIRROR_MAX; i++ )
                    {
                        if( MirrorTypeLUT[i][1] == mirror.eMirror )
                        {
                          *((VisionCamMirrorType*)param) = (VisionCamMirrorType) MirrorTypeLUT[i][0];
                          break;
                        }
                    }

                if( i >= VCAM_MIRROR_MAX )
                    greError = STATUS_INVALID_PARAMETER;
                }
            }
            break;
        }

#if ( defined(DUCATI_1_5) || defined(DUCATI_2_0) ) && defined(OMX_CAMERA_SUPPORTS_MANUAL_CONTROLS)
        case VCAM_PARAM_AWB_MIN_DELAY_TIME:
        {
            uint32_t *timeDelay = (uint32_t*)param;
            OMX_TI_CONFIG_AE_DELAY aeDelay;
            OMX_STRUCT_INIT( aeDelay, OMX_TI_CONFIG_AE_DELAY, mLocalVersion );

            omxError = OMX_GetConfig( mCurGreContext.mHandleComp,
                                    ( OMX_INDEXTYPE )OMX_TI_IndexConfigAutoExpMinDelayTime,
                                    &aeDelay
                                  );
            if( OMX_ErrorNone == omxError )
            {
                *timeDelay = aeDelay.nDelayTime;
            }
            break;
        }
#endif
#if ( defined(DUCATI_1_5) || defined(DUCATI_2_0) ) && defined(OMX_CAMERA_SUPPORTS_GESTURES)
        case VCAM_PARAM_GESTURES_INFO:
        {
            VisionCamGestureInfo *info = (VisionCamGestureInfo *)param;
            OMX_TI_CONFIG_GESTURES_INFO gestInfo;
            int ret = 0;

            OMX_STRUCT_INIT( gestInfo, OMX_TI_CONFIG_GESTURES_INFO , mLocalVersion );

            omxError = OMX_GetConfig( mCurGreContext.mHandleComp,
                                    (OMX_INDEXTYPE)OMX_TI_IndexConfigDetectedGesturesInfo,
                                    &gestInfo
                                  );

            if( OMX_ErrorNone != omxError )
                break;

            ret = getLutValue( gestInfo.eType, OMX_VALUE_TYPE,
                               GestureTypeLUT, ARR_SIZE(GestureTypeLUT)
                             );

            if( STATUS_INVALID_PARAMETER == ret )
            {
                greError = STATUS_INVALID_PARAMETER;
            }
            else
            {
                info->mGestureType = (VisionCamGestureEvent_e)ret;
                info->mRegionsNum = gestInfo.nNumDetectedGestures;
                if( info->mRegionsNum >= VCAM_Max_Gesture_Per_Frame )
                {
                    greError= STATUS_INVALID_PARAMETER;
                    break;
                }

            #ifdef __cplusplus
                info->mRegions = new VisionCamObjectRectType[ info->mRegionsNum ];
            #else
                if( NULL == info->mRegions )
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "Please allocate the mRegions buffer");
                    DVP_PRINT(DVP_ZONE_ERROR, "Check for necessery size in mRegionsNum");

                    greError= STATUS_NO_RESOURCES;
                    break;
                }
            #endif

                for(uint32_t i = 0; i < info->mRegionsNum; i++ )
                {
                    int value = getLutValue( (int)(gestInfo.nGestureAreas[i].eType),
                                             OMX_VALUE_TYPE,
                                             ObjectTypeLUT,
                                             ARR_SIZE(ObjectTypeLUT)
                                            );
                    if (value == STATUS_INVALID_PARAMETER)
                        continue;
                    info->mRegions[i].mObjType = (VisionCamObjectType)value;
                    gestInfo.nGestureAreas[i].nTop = info->mRegions[i].mTop;
                    gestInfo.nGestureAreas[i].nLeft = info->mRegions[i].mLeft;
                    gestInfo.nGestureAreas[i].nWidth = info->mRegions[i].mWidth;
                    gestInfo.nGestureAreas[i].nHeight = info->mRegions[i].mHeight;
                }

                info->timeStamp = gestInfo.nTimeStamp;
            }

            break;
        }
#endif
#if ( defined(DUCATI_1_5) || defined(DUCATI_2_0) ) && defined(OMX_CAMERA_SUPPORTS_MANUAL_CONTROLS)
        case VCAM_PARAM_AGC_MIN_DELAY_TIME:
        {
            uint32_t *delay = (uint32_t*)param;
            OMX_TI_CONFIG_AE_DELAY agcDelTime;
            OMX_STRUCT_INIT( agcDelTime, OMX_TI_CONFIG_AE_DELAY, mLocalVersion );

            omxError = OMX_GetConfig( mCurGreContext.mHandleComp,
                                    (OMX_INDEXTYPE)OMX_TI_IndexConfigAutoExpMinDelayTime,
                                    &agcDelTime );

            if( OMX_ErrorNone != omxError )
                break;

            *delay = agcDelTime.nDelayTime;
            break;
        }

        case VCAM_PARAM_AGC_LOW_TH:
        {
            uint32_t *lowTH = (uint32_t*)param;
            OMX_TI_CONFIG_AE_THRESHOLD ae;

            OMX_STRUCT_INIT( ae, OMX_TI_CONFIG_AE_THRESHOLD, mLocalVersion );

            omxError = OMX_GetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexConfigAutoExpThreshold, &ae );

            if( OMX_ErrorNone != omxError )
                break;

            *lowTH = ae.uMinTH;
            break;
        }

        case VCAM_PARAM_AGC_HIGH_TH:
        {
            uint32_t *highTH = (uint32_t*)param;

            OMX_TI_CONFIG_AE_THRESHOLD ae;
            OMX_STRUCT_INIT( ae, OMX_TI_CONFIG_AE_THRESHOLD, mLocalVersion );

            omxError = OMX_GetConfig( mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexConfigAutoExpThreshold, &ae );

            if( OMX_ErrorNone != omxError )
                break;

            *highTH = ae.uMaxTH;
            break;
        }
#endif
#if defined(OMX_CAMERA_SUPPORTS_IMAGE_PYRAMID)
        case VCAM_PARAM_IMAGE_PYRAMID:
        {
            greError = getImagePyramid(param, size, port);
            break;
        }
#endif
        default:
        {
            DVP_PRINT(DVP_ZONE_ERROR, "Impossible parameter id requested: %d\n", paramId);
            DVP_PRINT(DVP_ZONE_ERROR, "see VisionCamParam_e for possible parameter ids\n");
            greError = STATUS_INVALID_PARAMETER;
        }
    }

    if( OMX_ErrorNone != omxError )
    {
        greError = ConvertError(omxError);
    }

    if( greError != STATUS_SUCCESS )
    {
        DVP_PRINT(DVP_ZONE_ERROR, "getParameter() exits with error 0x%x (dec: %d) for param id %d\n", greError, greError, paramId);
    }

    return greError;
}

OMX_ERRORTYPE OMXVisionCam::getFocusStatus(OMX_FOCUSSTATUSTYPE *status)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_PARAM_FOCUSSTATUSTYPE focusStatus;
    OMX_STRUCT_INIT(focusStatus, OMX_PARAM_FOCUSSTATUSTYPE, mLocalVersion);

    // Get the focus status
    OMX_CHECK(eError,OMX_GetConfig(mCurGreContext.mHandleComp,
                                   OMX_IndexConfigCommonFocusStatus,
                                   &focusStatus));
    if (eError == OMX_ErrorNone)
        *status = focusStatus.eFocusStatus;
    return eError;
}

status_e OMXVisionCam::waitForFocus()
{
    OMX_ERRORTYPE omxError = OMX_ErrorNone;
    OMX_FOCUSSTATUSTYPE focusStatus;

    omxError = RegisterForEvent(mCurGreContext.mHandleComp,
                              (OMX_EVENTTYPE) OMX_EventIndexSettingChanged,
                              OMX_ALL,
                              OMX_IndexConfigCommonFocusStatus,
                              &mGreFocusSem,
                              -1);
    if (OMX_ErrorNone == omxError)
    {
        OMX_CONFIG_CALLBACKREQUESTTYPE focusRequestCallback;
        OMX_STRUCT_INIT(focusRequestCallback, OMX_CONFIG_CALLBACKREQUESTTYPE, mLocalVersion);
        focusRequestCallback.bEnable = OMX_TRUE;
        focusRequestCallback.nIndex = OMX_IndexConfigCommonFocusStatus;

        // subscribe for focus state changes
        OMX_CHECK(omxError,OMX_SetConfig(mCurGreContext.mHandleComp,
                                       (OMX_INDEXTYPE) OMX_IndexConfigCallbackRequest,
                                       &focusRequestCallback));
#ifdef VCAM_CAUTIOUS
        // make sure it's been registered
        OMX_CHECK(omxError,OMX_GetConfig(mCurGreContext.mHandleComp,
                                       (OMX_INDEXTYPE) OMX_IndexConfigCallbackRequest,
                                       &focusRequestCallback));

#endif
        if (OMX_ErrorNone == omxError && focusRequestCallback.bEnable == OMX_TRUE)
        {
            DVP_PRINT(DVP_ZONE_CAM, "Waiting for Focus Callback!\n");
            // wait for focus to arrive
            semaphore_wait(&mGreFocusSem);

            // Give the client the focus greError
            omxError = getFocusStatus(&focusStatus);
            DVP_PRINT(DVP_ZONE_CAM, "Focus Status: %u\n", focusStatus);
            if (OMX_ErrorNone == omxError)
            {
                if (m_focuscallback)
                    m_focuscallback((int)focusStatus);
            }
        }
    }
    return ConvertError(omxError);
}

//status_e OMXVisionCam::startAutoFocus( VisionCamFocusMode focusMode )
status_e OMXVisionCam::startAutoFocus( uint32_t inp )
{
    OMX_ERRORTYPE omxError = OMX_ErrorNone;
    bool_e hasEvent = true_e;

    VisionCamFocusMode focusMode = (VisionCamFocusMode)inp;
    OMX_CONFIG_BOOLEANTYPE enable;

//    for( int32_t port = VCAM_PORT_MIN; port < VCAM_PORT_MAX; port++ )
    {
        // @todo in case of two sensors running sumiltaneously check on which port to actiivate focus and for which to enqueue it.
        if( mPendingConfigs && !mPendingConfigs->getFunc(ePending_Focus) )
        {
            mPendingConfigs->Register(  ePending_Focus,
//                                        /*(VisionCamExecutionService<OMXVisionCam, OMXVisionCam::startAutofocusFuncPtr>::execFuncPrt_t)*/&OMXVisionCam::startAutoFocus,
                                        &OMXVisionCam::startAutoFocus,
                                        sizeof(VisionCamFocusMode)
                                     );
        }
    }

    if( mPendingConfigs && mPendingConfigs->getFunc(ePending_Focus))
    {
        mPendingConfigs->setData(ePending_Focus, &focusMode);
    }

    if( !mCurGreContext.mCameraPortParams[VCAM_PORT_PREVIEW].mIsActive )
    {
        return STATUS_SUCCESS;
    }

    enable.nSize = sizeof(OMX_CONFIG_BOOLEANTYPE);
    memcpy( &enable.nVersion, mLocalVersion, sizeof(OMX_VERSIONTYPE));
    if( focusMode == VCAM_FOCUS_CONTROL_OFF )
    {
        enable.bEnabled = OMX_FALSE;
    }
    else
    {
        enable.bEnabled = OMX_TRUE;
    }

    omxError = OMX_SetConfig(mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_TI_IndexConfigAutofocusEnable, &enable);

    if (focusMode == VCAM_FOCUS_CONTROL_OFF ||
        focusMode == VCAM_FOCUS_CONTROL_AUTO ||
        focusMode == VCAM_FOCUS_CONTROL_CONTINOUS_NORMAL ||
        focusMode == VCAM_FOCUS_CONTROL_CONTINOUS_EXTENDED)
    {
        hasEvent = false_e;
    }

    if( OMX_ErrorNone == omxError )
    {
        int value = 0;
        OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE focus;
        OMX_STRUCT_INIT(focus, OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE, mLocalVersion);
        value = getLutValue(focusMode, VCAM_VALUE_TYPE, FocusModeLUT, ARR_SIZE(FocusModeLUT));

        if (value == STATUS_INVALID_PARAMETER)
            return (status_e)value;

        focus.eFocusControl = ( OMX_IMAGE_FOCUSCONTROLTYPE )value;

        if( focus.eFocusControl == OMX_IMAGE_FocusControlOn )
            focus.nFocusSteps = mManualFocusDistance;

        DVP_PRINT(DVP_ZONE_CAM, "Focus Requested Steps @ %lu, Index @ %lu\n", focus.nFocusSteps, focus.nFocusStepIndex);

        // tell the OMX component to change the focus control mode
        omxError = OMX_SetConfig(mCurGreContext.mHandleComp, OMX_IndexConfigFocusControl, &focus);

#ifdef VCAM_CAUTIOUS
        if( OMX_ErrorNone == omxError )
        {
            // ask it what mode it's in now
            OMX_CHECK(omxError, OMX_GetConfig(mCurGreContext.mHandleComp, OMX_IndexConfigFocusControl, &focus));
        }
#endif
        DVP_PRINT(DVP_ZONE_CAM, "Focus Control Mode = 0x%08x\n", focus.eFocusControl);
    }

    if ( OMX_ErrorNone == omxError )
    {
        if (hasEvent)
        {
            thread_create(FocusThreadLauncher, this);
            // the thread will die by itself, we don't need to join it.
        }
    }

    return ConvertError(omxError);
}

/* This will set the preview buffer sizes, format, etc.
    */
OMX_ERRORTYPE OMXVisionCam::setPortDef( int32_t port )
{
    OMX_ERRORTYPE omxError = OMX_ErrorNone;

    VCAM_PortParameters * portData = NULL;
    OMX_PARAM_PORTDEFINITIONTYPE portCheck;

    int32_t p;
    LOOP_PORTS( port , p )
    {
        omxError = initPortCheck(&portCheck, p);

        if( omxError == OMX_ErrorNone )
        {
            portData =  &mCurGreContext.mCameraPortParams[p];
            portCheck.format.video.nFrameWidth  = portData->mWidth;
            portCheck.format.video.nFrameHeight = portData->mHeight;
            portCheck.format.video.eColorFormat = portData->mColorFormat;
            portCheck.format.video.nStride      = portData->mStride;
            portCheck.format.video.xFramerate   = portData->mFrameRate << 16;
            portCheck.nBufferCountActual        = portData->mNumBufs;
            omxError = OMX_SetParameter(mCurGreContext.mHandleComp,
                                      OMX_IndexParamPortDefinition,
                                      &portCheck);
        }

        if( omxError == OMX_ErrorNone )
        {
            omxError = initPortCheck(&portCheck, p);
        }

#if defined(VCAM_SET_FORMAT_ROTATION)
            // set the rotation type on the port
            if (omxError == OMX_ErrorNone)
            {
                OMX_CONFIG_ROTATIONTYPE rotType;
                OMX_STRUCT_INIT(rotType, OMX_CONFIG_ROTATIONTYPE, mLocalVersion);
                rotType.nRotation = mCurGreContext.mCameraPortParams[p].mRotation;
                rotType.nPortIndex = mCurGreContext.mPortsInUse[p];
                DVP_PRINT(DVP_ZONE_CAM, "VCAM: Configuring for Rotation %li\n",rotType.nRotation);
                OMX_CHECK(omxError, OMX_SetConfig(mCurGreContext.mHandleComp,
                                                  OMX_IndexConfigCommonRotate,
                                                  &rotType));
            }
#endif
    }

    return omxError;
}

/* This will start the preview.
 * Before that setParameter() sould be called, to configure the preview port.
 * This method is only called internal for OMXVisionCam
 * by sendCommand().
 */
status_e OMXVisionCam::startPreview( VisionCamPort_e port )
{
    status_e greError = STATUS_SUCCESS;
    OMX_ERRORTYPE omxError;

    if( port < VCAM_PORT_ALL || port > VCAM_PORT_MAX - 1 )
    {
        DVP_PRINT(DVP_ZONE_ERROR, "startPreview() called, but port is not specified properly.\n");
        return STATUS_INVALID_PARAMETER;
    }

    switch((int32_t)getComponentState())
    {
        case OMX_StateIdle:
        {
            greError = transitToState( OMX_StateExecuting );
            mReturnToExecuting = true_e;
            break;
        }
        case OMX_StateLoaded:
        case OMX_StateWaitForResources:
        case OMX_StateInvalid:
        {
            DVP_PRINT(DVP_ZONE_ERROR, "Calling startPreview() in an inproper state.\n");
            greError = STATUS_INVALID_STATE;
            break;
        }
        case OMX_StateExecuting:
        case OMX_StatePause:
        {
            break;
        }
        default:
            break;
    }

    SOSAL::AutoLock lock( &mFrameBufferLock );

    if( STATUS_SUCCESS == greError )
        mFlushInProcess = false_e;

    if( STATUS_SUCCESS == greError )
    {
        omxError = setPortDef( port );
        greError = ConvertError(omxError);
    }

    if( STATUS_SUCCESS == greError )
    {
        greError = portEnableDisable( OMX_CommandPortEnable, populatePortSrvc, port);
    }

    if( STATUS_SUCCESS == greError )
    {
        // Some configurations couldn't be set before preview has been started
        // so we need to call their executuion right after preview is started
        // let us make it by the time the very first frame is received !
        pFrameReceivedSrvc = &OMXVisionCam::FirstFrameFunc;

        // first fill the video port buffers, then preview port
        // otherwise preview port will start working right after it receive its frames
        // so it won't wait for video port to get ready
        if( VCAM_PORT_ALL == port || VCAM_PORT_VIDEO == port )
        {
            greError = fillPortBuffers(VCAM_PORT_VIDEO);
        }

        if( VCAM_PORT_ALL == port || VCAM_PORT_PREVIEW == port )
        {
            greError = fillPortBuffers(VCAM_PORT_PREVIEW);
        }
    }

    return greError;
}

/**
  * This will stop the preview
  *
  */
status_e OMXVisionCam::stopPreview( VisionCamPort_e port )
{
    status_e greError = STATUS_SUCCESS;
    bool_e goToIdle = true_e;

    mFlushInProcess = true_e;

    flushBuffers( port );
    greError = portEnableDisable( OMX_CommandPortDisable, freePortBuffersSrvc, port );

    if( STATUS_SUCCESS == greError )
    {
        for( int p = VCAM_PORT_MIN; p < VCAM_PORT_MAX; p++)
        {
            if( mCurGreContext.mCameraPortParams[p].mIsActive )
            {
                goToIdle = false_e;
                break;
            }
        }

        if ( goToIdle && OMX_StateExecuting == getComponentState() )
        {
            transitToState( OMX_StateIdle );
        }
    }

    return greError;
}

/**
  * This will free all the buffers on the preview port
  * and switch to loaded state.
  */
status_e OMXVisionCam::releaseBuffers( VisionCamPort_e port)
{
    status_e greError = STATUS_SUCCESS;

    int32_t p;
    LOOP_PORTS( port , p ) {
        if( mFrameDescriptors && mFrameDescriptors[p] ) {

            for( int32_t i = 0; i < mCurGreContext.mCameraPortParams[p].mNumBufs; i++ )
                delete mFrameDescriptors[p][i];

            delete []mFrameDescriptors[p];

            mFrameDescriptors[p] = NULL;
        }
    }

    if (OMX_StateIdle == getComponentState()) {
        greError = transitToState( OMX_StateLoaded, NULL, NULL );
    }

    return greError;
}

/** This will configure the component to start/stop face detection.
 * Will also start/stop face detection extra data
 * faces coordinates will be written into camera frame received
 * in preview callback
 */
status_e OMXVisionCam::enableFaceDetect(VisionCamPort_e port, VisionCamFaceDetectionType_t fdType )
{
    OMX_ERRORTYPE omxError = OMX_ErrorNone;
    OMX_CONFIG_OBJDETECTIONTYPE objDetection;
    OMX_STRUCT_INIT( objDetection, OMX_CONFIG_OBJDETECTIONTYPE ,mLocalVersion);
    objDetection.nPortIndex = mCurGreContext.mPortsInUse[port];

    if ( mFaceDetectionEnabled & VCAM_FACE_DETECTION_ALL )
    {
        objDetection.bEnable = OMX_TRUE;
    }
    else
    {
        objDetection.bEnable = OMX_FALSE;
    }

    omxError = OMX_SetConfig(mCurGreContext.mHandleComp, (OMX_INDEXTYPE) OMX_IndexConfigImageFaceDetection, &objDetection);

    if ( OMX_ErrorNone == omxError )
    {
        OMX_CONFIG_EXTRADATATYPE xData;
        OMX_STRUCT_INIT(xData, OMX_CONFIG_EXTRADATATYPE, mLocalVersion);

        xData.nPortIndex        = mCurGreContext.mPortsInUse[port];

#if defined(OMX_CAMERA_SUPPORTS_CAMERA_VIEW)
        xData.eCameraView       = OMX_2D_Prv;
#endif

        for( int32_t i = VCAM_FACE_DETECTION_MIN; i < VCAM_FACE_DETECTION_MAX; i <<= 1 )
        {
            if( i & fdType )
            {
                switch(i)
                {
                case VCAM_FACE_DETECTION:
                    xData.eExtraDataType = OMX_FaceDetection;
                    break;
#if defined(OMX_CAMERA_SUPPORTS_FD_RAW)
                case VCAM_FACE_DETECTION_RAW:
                    xData.eExtraDataType = OMX_TI_FaceDetectionRaw;
                    break;
#endif
                }

                xData.bEnable = ( (mFaceDetectionEnabled & fdType) ? OMX_TRUE : OMX_FALSE );
                omxError = OMX_SetConfig(mCurGreContext.mHandleComp, (OMX_INDEXTYPE)OMX_IndexConfigOtherExtraDataControl, &xData);

                if( OMX_ErrorNone != omxError )
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "Unable to start/stop face detect.\n");
                    break;
                }
            }
        }
    }

    return ConvertError(omxError);
}

/** Fill the face coordinates field in camera frame,
 *  which will be received by OMXVisionCam client at each frame.
 */
void OMXVisionCam::getFacesCoordinates( VisionCamFrame *frame)
{
    OMX_OTHER_EXTRADATATYPE* extraData = NULL;
    OMX_U8 *pExtraLimit = (OMX_U8 *)(frame->mExtraDataBuf) + frame->mExtraDataLength;
    OMX_FACEDETECTIONTYPE *facesData = NULL;

    OMX_EXT_EXTRADATATYPE expectedFD    = (OMX_EXT_EXTRADATATYPE)OMX_ExtraDataNone;
    uint32_t *frameFacesCount           = NULL;
    VisionCamFaceType *frameFacesData   = NULL;

    if( !frame->mExtraDataBuf )
    {
        frame->mDetectedFacesNum = 0;
        frame->mDetectedFacesNumRaw = 0;
        return;
    }

    for( int32_t fdSeek = VCAM_FACE_DETECTION_MIN; fdSeek < VCAM_FACE_DETECTION_ALL; fdSeek <<= 1 )
    {
        switch(fdSeek & mFaceDetectionEnabled)
        {
#if defined(OMX_CAMERA_SUPPORTS_FD_RAW)
            case VCAM_FACE_DETECTION_RAW:
            {
                expectedFD = OMX_TI_FaceDetectionRaw;
                frameFacesCount = &frame->mDetectedFacesNumRaw;
                frameFacesData = &frame->mFacesRaw[0];
                break;
            }
#endif
            case VCAM_FACE_DETECTION:
            {
                expectedFD = OMX_FaceDetection;
                frameFacesCount = &frame->mDetectedFacesNum;
                frameFacesData = &frame->mFaces[0];
                break;
            }
            default:
                continue;
        }

        *frameFacesCount = 0;
        memset( frameFacesData, 0, MAX_FACES_COUNT*sizeof(VisionCamFaceType) );

        extraData = (OMX_OTHER_EXTRADATATYPE*)frame->mExtraDataBuf;

        while( expectedFD != (OMX_EXT_EXTRADATATYPE)extraData->eType && extraData->data )
        {
            DVP_PRINT(DVP_ZONE_CAM, "Current Extra Data Section Size: %lu\n", extraData->nDataSize);
            extraData = (OMX_OTHER_EXTRADATATYPE*)(extraData->data + extraData->nDataSize);

            if( (OMX_U8 *)extraData >= pExtraLimit )
            {
                DVP_PRINT(DVP_ZONE_ERROR, "ERROR: METADATA: Bad size field in metadata. %p >= %p\n", extraData, pExtraLimit);
                return;
            }

            if( 0 == extraData->nDataSize )
            {
                DVP_PRINT(DVP_ZONE_WARNING, "METADATA: No face data detected!\n");
                return;
            }
        }

        if( extraData->data )
        {
            facesData = (OMX_FACEDETECTIONTYPE *)(extraData->data);
            *frameFacesCount = facesData->ulFaceCount;

            DVP_PRINT(DVP_ZONE_CAM, "METADATA: FACE # %d!\n", facesData->ulFaceCount);

            for(uint32_t i = 0; i < facesData->ulFaceCount; i++)
            {
                memcpy( &(frameFacesData[i]), &(facesData->tFacePosition[i].nScore), sizeof(VisionCamFaceType)) ;
            }
        }
    }
}

/** Parse through the extra data type structure to find the pointer to the relevent
 *  data type.  This is to abstract the port number, version, and packet structure from the client.
 */
void OMXVisionCam::getMetadataPtrs( VisionCamFrame *frame)
{
    OMX_OTHER_EXTRADATATYPE* extraData = (OMX_OTHER_EXTRADATATYPE*)frame->mExtraDataBuf;
    OMX_U8 *pExtraLimit = (OMX_U8 *)extraData + frame->mExtraDataLength;
    frame->mMetadata.mAutoWBGains = NULL;
    frame->mMetadata.mManualWBGains = NULL;
    frame->mMetadata.mAncillary = NULL;
    frame->mMetadata.mHistogram2D = NULL;
    frame->mMetadata.mHistogramL = NULL;
    frame->mMetadata.mHistogramR = NULL;
    frame->mMetadata.mMTIS_Data = NULL;
    frame->mMetadata.mUnsaturatedRegions = NULL;
    frame->mMetadata.mFocusRegionData = NULL;
    frame->mMetadata.mGamma2D = NULL;
    frame->mMetadata.mGammaL = NULL;
    frame->mMetadata.mGammaR = NULL;

    if( extraData == NULL )
    {
        return;
    }

    while( extraData->eType && extraData->nDataSize && extraData->data )   // keep looping while there is more extra data and double check size
    {
        switch( (OMX_EXT_EXTRADATATYPE)extraData->eType )
        {
            case OMX_AncillaryData:
            {
                OMX_TI_ANCILLARYDATATYPE *ancillaryData = NULL;
                DVP_PRINT(DVP_ZONE_CAM, "METADATA: Found Ancillary Data Section!\n");
                ancillaryData =  (OMX_TI_ANCILLARYDATATYPE  *)(extraData->data);
                frame->mMetadata.mAncillary= (VisionCamAncillary*)&ancillaryData->nAncillaryDataVersion;
                break;
            }

            case OMX_WhiteBalance:
            {
                OMX_TI_WHITEBALANCERESULTTYPE *wbData = NULL;
                DVP_PRINT(DVP_ZONE_CAM, "METADATA: Found White Balance Result Data Section!\n");
                wbData =  (OMX_TI_WHITEBALANCERESULTTYPE  *)(extraData->data);
                frame->mMetadata.mAutoWBGains = (VisionCamWhiteBalGains*)&wbData->nGainR;
                break;
            }
#if defined(OMX_CAMERA_SUPPORTS_WHITEBALANCE_OVERWRITE)
            case OMX_TI_WhiteBalanceOverWrite:
            {
                OMX_TI_WHITEBALANCERESULTTYPE *wbManData = NULL;
                DVP_PRINT(DVP_ZONE_CAM, "METADATA: Found White Balance Overwrite Data Section!\n");
                wbManData =  (OMX_TI_WHITEBALANCERESULTTYPE  *)(extraData->data);
                frame->mMetadata.mManualWBGains = (VisionCamWhiteBalGains*)&wbManData->nGainR;
                break;
            }
#endif
#if defined(OMX_CAMERA_SUPPORTS_HISTOGRAM)
            case OMX_Histogram:
            {
                OMX_TI_HISTOGRAMTYPE *histData = NULL;
                DVP_PRINT(DVP_ZONE_CAM, "METADATA: Found Histogram Result Data Section!\n");
                histData =  (OMX_TI_HISTOGRAMTYPE  *)(extraData->data);
#if defined(OMX_CAMERA_SUPPORTS_CAMERA_VIEW)
                if((OMX_TI_CAMERAVIEWTYPE)(histData->eCameraView) == OMX_2D_Prv)
                    frame->mMetadata.mHistogram2D = (VisionCamHistogram*)&histData->nBins;
                else if((OMX_TI_CAMERAVIEWTYPE)(histData->eCameraView) == OMX_3D_Left_Prv)
                    frame->mMetadata.mHistogramL = (VisionCamHistogram*)&histData->nBins;
                else if((OMX_TI_CAMERAVIEWTYPE)(histData->eCameraView) == OMX_3D_Right_Prv)
                    frame->mMetadata.mHistogramR = (VisionCamHistogram*)&histData->nBins;
#else
                frame->mMetadata.mHistogram2D = (VisionCamHistogram*)&histData->nBins;
#endif
                break;
            }
#endif
            case OMX_UnsaturatedRegions:
            {
                OMX_TI_UNSATURATEDREGIONSTYPE *usReg = NULL;
                DVP_PRINT(DVP_ZONE_CAM, "METADATA: Found Unsaturated Regions Data Section!\n");
                usReg = (OMX_TI_UNSATURATEDREGIONSTYPE *)(extraData->data);
                frame->mMetadata.mUnsaturatedRegions =
                        (VisionCamUnsaturatedRegions *)&usReg->nPaxelsX;
                break;
            }

#if defined(OMX_CAMERA_SUPPORTS_MTIS)
            case OMX_MTISType:
            {
                OMX_MTISTYPE *mtisData = NULL;
                DVP_PRINT(DVP_ZONE_CAM, "METADATA: Found MTIS Data Section!\n");
                mtisData = (OMX_MTISTYPE *)(extraData->data);
                frame->mMetadata.mMTIS_Data =(VisionCamMTIS*)&mtisData->eCameraView;
                break;
            }
#elif defined(OMX_CAMERA_SUPPORTS_TI_MTIS)
            case OMX_TI_MTISType:
            {
                OMX_TI_MTISTYPE *mtisData = NULL;
                DVP_PRINT(DVP_ZONE_CAM, "METADATA: Found MTIS Data Section!\n");
                mtisData = (OMX_TI_MTISTYPE *)(extraData->data);
                frame->mMetadata.mMTIS_Data =(VisionCamMTIS*)&mtisData->eCameraView;
                break;
            }
#endif
            case OMX_FocusRegion:
            {
                OMX_CONFIG_EXTFOCUSREGIONTYPE *focusRegData = NULL;
                DVP_PRINT(DVP_ZONE_CAM, "METADATA: Found Focus Region Data Section!\n");
                focusRegData = (OMX_CONFIG_EXTFOCUSREGIONTYPE *)(extraData->data);
                frame->mMetadata.mFocusRegionData =(VisionCamFocusRegion*)&focusRegData->nLeft;
                break;
            }

#if defined(OMX_CAMERA_SUPPORTS_HMSGAMMA)
            case OMX_TI_HMSGamma:
            {
                OMX_HMSGAMMATYPE *histGammaData = NULL;
                DVP_PRINT(DVP_ZONE_CAM, "METADATA: Found Gamma Data Section!\n");
                histGammaData = (OMX_HMSGAMMATYPE *)(extraData->data);
#if defined(OMX_CAMERA_SUPPORTS_CAMERA_VIEW)
                if((OMX_TI_CAMERAVIEWTYPE)(histGammaData->eCameraView) == OMX_2D_Prv)
                    frame->mMetadata.mGamma2D = (VisionCamGamma*)&histGammaData->nItems;
                else if((OMX_TI_CAMERAVIEWTYPE)(histGammaData->eCameraView) == OMX_3D_Left_Prv)
                    frame->mMetadata.mGammaL = (VisionCamGamma*)&histGammaData->nItems;
                else if((OMX_TI_CAMERAVIEWTYPE)(histGammaData->eCameraView) == OMX_3D_Right_Prv)
                    frame->mMetadata.mGammaR = (VisionCamGamma*)&histGammaData->nItems;
#else
                frame->mMetadata.mGamma2D = (VisionCamGamma*)&histGammaData->nItems;
#endif
                break;
            }
#endif
            case OMX_TI_ImagePyramid:
            {
                OMX_TI_IMAGEPYRAMIDTYPE     *pyramidData = NULL;
                DVP_PRINT(DVP_ZONE_CAM, "METADATA: found Image Pyramid Data Section!\n" );
                pyramidData = (OMX_TI_IMAGEPYRAMIDTYPE*)(extraData->data);
                frame->mMetadata.mImgPyramidData = (VisionCamImagePyramidData*)(&pyramidData->nLevelsCount);
                break;
            }


//            case OMX_FaceDetection:
            // done in getFacesCoordinates()
//            break;

//            case OMX_TI_FaceDetectionRaw:
            // done in getFacesCoordinates()
//            break;

            default:
            {
                break;
            }
        }
        DVP_PRINT(DVP_ZONE_CAM, "Current Extra Data Section Size: %lu\n", extraData->nDataSize);
        extraData = (OMX_OTHER_EXTRADATATYPE*)(extraData->data + extraData->nDataSize);
        if( (OMX_U8 *)extraData >= pExtraLimit )
        {
            DVP_PRINT(DVP_ZONE_ERROR, "ERROR: METADATA: Bad size field in metadata. %p >= %p\n", extraData, pExtraLimit);
            break;
        }
    }
}

/** This shall be called by the user of this class when it finishes working
 * with the frame and to notify VisionCam that the frame buffer is
 * free.
 */
status_e OMXVisionCam::returnFrame(VisionCamFrame *cameraFrame)
{
    OMX_ERRORTYPE omxError = OMX_ErrorNone;
    int32_t i = 0;
    if (mFlushInProcess)
        return STATUS_SUCCESS;

    if (cameraFrame == NULL)
        return STATUS_INVALID_PARAMETER;

    if (OMX_StateExecuting == getComponentState())
    {
        VisionCamPort_e port = cameraFrame->mFrameSource;
        VCAM_PortParameters * portData = &mCurGreContext.mCameraPortParams[port];
        for (i = 0; i < portData->mNumBufs; i++)
        {
            if (portData->mBufferHeader[i] &&
                portData->mBufferHeader[i]->pAppPrivate == cameraFrame->mFrameBuff)
            {
                omxError = OMX_FillThisBuffer(mCurGreContext.mHandleComp,
                                              mCurGreContext.mCameraPortParams[port].mBufferHeader[i]);
                if (OMX_ErrorNone != omxError)
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR: OMX_FillThisBuffer() returned 0x%x in %s\n", omxError, __func__);
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_CAM, "frame returned successfully in %s\n", __func__);
                }
                break;
            }
        }
        if( i == portData->mNumBufs)
        {
            DVP_PRINT(DVP_ZONE_ERROR, "ERROR: returned frame not found in %s\n", __func__);
        }
    }
    else
    {
        DVP_PRINT(DVP_ZONE_WARNING, "Returning Frame when OMX-CAMERA is in the wrong state or image is NULL\n");
    }

    return ConvertError(omxError);
}

status_e OMXVisionCam::PreemptionService(/*VisionCamPreemptionActivity_e activity*/)
{
    OMX_ERRORTYPE omxError = OMX_ErrorNone;
    VCAM_PortParameters * portData = NULL;

    switch( mPreemptionState )
    {
        case VCAM_PREEMPT_SUSPEND:
        {
            // Register for Loaded state switch event
            omxError = RegisterForEvent(mCurGreContext.mHandleComp,
                                         OMX_EventCmdComplete,
                                         OMX_CommandStateSet,
                                         OMX_StateLoaded,
                                         &mGreLocalSem,
                                         -1); // Infinite timeout

            if( OMX_ErrorNone == omxError )
            {
                for( int port = VCAM_PORT_PREVIEW; port < VCAM_PORT_MAX; port++ )
                {
                    // Free the OMX Buffers
                    portData = &mCurGreContext.mCameraPortParams[port];
                    for( int i = 0; i < portData->mNumBufs; i++ )
                    {
                        omxError = OMX_FreeBuffer(mCurGreContext.mHandleComp,
                                                        mCurGreContext.mPortsInUse[port],
                                                        portData->mBufferHeader[i]);

                        portData->mBufferHeader[i] = NULL;

                        if( OMX_ErrorNone != omxError )
                        {
                            DVP_PRINT(DVP_ZONE_ERROR,
                                    "Preemption Service: Error 0x%x while freeing buffers!", omxError);
                            break;
                        }
                    }
                }
            }

            if( OMX_ErrorNone == omxError )
                semaphore_wait(&mGreLocalSem);

            if( mClientNotifier.mNotificationCallback && OMX_ErrorNone == omxError )
                mClientNotifier.mNotificationCallback(VisionCamClientNotifier::VCAM_MESSAGE_PREEMPT_SUSPEND_ACTIVITY);

            // Register for WAIT state switch event
            omxError = RegisterForEvent(mCurGreContext.mHandleComp,
                                         OMX_EventCmdComplete,
                                         OMX_CommandStateSet,
                                         OMX_StateWaitForResources,
                                         &mGreLocalSem,
                                         -1); // Infinite timeout

            if( OMX_ErrorNone == omxError  )
            {
              omxError = OMX_SendCommand(mCurGreContext.mHandleComp,
                                            OMX_CommandStateSet,
                                            OMX_StateWaitForResources,
                                            NULL);
            }

            if( OMX_ErrorNone == omxError  )
              semaphore_wait(&mGreLocalSem);

            break;
        }
        case VCAM_PREEMPT_RESUME:
        {
            if( mClientNotifier.mNotificationCallback )
                mClientNotifier.mNotificationCallback(VisionCamClientNotifier::VCAM_MESSAGE_PREEMPT_RESUME_ACTIVITY);

            // Register for IDLE state switch event
            omxError = RegisterForEvent(mCurGreContext.mHandleComp,
                                           OMX_EventCmdComplete,
                                           OMX_CommandStateSet,
                                           OMX_StateIdle,
                                           &mGreLocalSem,
                                           -1); //Infinite timeout
            if( OMX_ErrorNone != omxError )
            {
                DVP_PRINT(DVP_ZONE_ERROR,
                          "Preemption Service: Error 0x%x while registering for Idle state wait.", omxError);
                break;
            }

            for( int port = VCAM_PORT_PREVIEW; port < VCAM_PORT_MAX; port++ )
            {
                portData = &mCurGreContext.mCameraPortParams[port];
                for( int buff = 0; buff < portData->mNumBufs; buff++ )
                {
                    OMX_BUFFERHEADERTYPE *pBufferHdr;
                    OMX_U8 *buffer = mBuffersInUse[port].mBuffers[buff].pData[0];

                    omxError = OMX_UseBuffer(   mCurGreContext.mHandleComp,
                                                &pBufferHdr,
                                                mCurGreContext.mPortsInUse[port],
                                                0,
                                                portData->mBufSize,
                                                buffer
                                            );

                    if( OMX_ErrorNone != omxError )
                    {
                        DVP_PRINT(DVP_ZONE_ERROR,
                                  "Preemption Service: Error 0x%x while passing buffers to OMX Component.", omxError);
                        break;
                    }

                    pBufferHdr->pAppPrivate = (OMX_PTR)&mBuffersInUse[port].mBuffers[buff];

                    pBufferHdr->nSize = sizeof(OMX_BUFFERHEADERTYPE);

                    memcpy( &(pBufferHdr->nVersion), mLocalVersion, sizeof(OMX_VERSIONTYPE) );

                    portData->mBufferHeader[buff] = pBufferHdr;
                }
            }

            semaphore_wait(&mGreLocalSem);

//            if (mReturnToExecuting)
//            {
                for( int port = VCAM_PORT_PREVIEW; port < VCAM_PORT_MAX; port++ )
                {
                    if( mCurGreContext.mCameraPortParams[port].mIsActive )
                        startPreview( (VisionCamPort_e)port );
                }
//            }
            break;
        }

        case VCAM_PREEMPT_WAIT_TO_START:
        {
            if( mClientNotifier.mNotificationCallback && OMX_ErrorNone == omxError )
                mClientNotifier.mNotificationCallback(VisionCamClientNotifier::VCAM_MESSAGE_PREEMPT_WAIT_RESOURCES);

            // Register for WAIT state switch event
            omxError = RegisterForEvent(mCurGreContext.mHandleComp,
                                           OMX_EventCmdComplete,
                                           OMX_CommandStateSet,
                                           OMX_StateWaitForResources,
                                           &mGreLocalSem,
                                           -1); // Infinite timeout

           if( OMX_ErrorNone == omxError  )
           {
               omxError = OMX_SendCommand(mCurGreContext.mHandleComp,
                                             OMX_CommandStateSet,
                                             OMX_StateWaitForResources,
                                             NULL);
           }

           if( OMX_ErrorNone == omxError  )
               semaphore_wait(&mGreLocalSem);

           // Register for WAIT state switch event
           omxError = RegisterForEvent(mCurGreContext.mHandleComp,
                                          OMX_EventResourcesAcquired,
                                          0,
                                          0,
                                          &mGreLocalSem,
                                          -1); // Infinite timeout

           if( OMX_ErrorNone == omxError  )
               semaphore_wait(&mGreLocalSem);

           if( mClientNotifier.mNotificationCallback && OMX_ErrorNone == omxError )
               mClientNotifier.mNotificationCallback(VisionCamClientNotifier::VCAM_MESSAGE_PREAMPT_RESOURCES_READY);

            break;
        }
        default:

            break;
    }

    if( OMX_ErrorNone == omxError  )
        mPreemptionState = VCAM_PREEMPT_INACTIVE;

    return ConvertError(omxError);
}

OMX_ERRORTYPE OMXVisionCam::RegisterForEvent(OMX_IN OMX_HANDLETYPE hComponent,
                                             OMX_IN OMX_EVENTTYPE eEvent,
                                             OMX_IN OMX_U32 nData1,
                                             OMX_IN OMX_U32 nData2,
                                             OMX_IN semaphore_t *semaphore,
                                             OMX_IN OMX_U32 timeout)
{
    OMXVCAM_Msg_t *msg = (OMXVCAM_Msg_t *)calloc(1, sizeof(OMXVCAM_Msg_t));
    node_t *node = (node_t *)calloc(1, sizeof(node_t));
    OMX_ERRORTYPE err = OMX_ErrorNone;
    if (msg && node)
    {
        msg->eEvent = eEvent;
        msg->nData1 = nData1;
        msg->nData2 = nData2;
        msg->semaphore = semaphore;
        msg->hComponent = hComponent;
        msg->timeout = timeout;
        node->data = (value_t)msg;
        list_append(mEventSignalQ, node);
        DVP_PRINT(DVP_ZONE_CAM, "Registering for Event %d (0x%08x)\n", eEvent,eEvent);
    }
    else
    {
        if (node)
            free(node);
        if (msg)
            free(msg);
        err = OMX_ErrorInsufficientResources;
    }
    return err;

}

extern "C" int node_compare_vcam_msg(node_t *a, node_t *b)
{
    OMXVCAM_Msg_t *msgA = (OMXVCAM_Msg_t *)a->data;
    OMXVCAM_Msg_t *msgB = (OMXVCAM_Msg_t *)b->data;

    DVP_PRINT(DVP_ZONE_CAM, "Comparing Node %p and Node %p\n",msgA, msgB);

    if (msgA->eEvent == msgB->eEvent &&
        msgA->nData1 == msgB->nData1 &&
        msgA->nData2 == msgB->nData2 &&
        msgA->hComponent == msgB->hComponent)
        return 0;
    else if (msgA->eEvent < msgB->eEvent)
        return -1;
    else // if (msgA->event > msgB->event)
        return 1;
}

/** OMXVisionCam Event Handler from OMX-CAMERA */
OMX_ERRORTYPE OMXVisionCam::EventHandler(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_IN OMX_PTR pAppData,
                                         OMX_IN OMX_EVENTTYPE eEvent,
                                         OMX_IN OMX_U32 nData1,
                                         OMX_IN OMX_U32 nData2,
                                         OMX_IN OMX_PTR pEventData)
{
    // make a local pointer to the instance class which registered this callback.
    OMXVisionCam *pOMXCam = (OMXVisionCam*)pAppData;
    size_t len = list_length(pOMXCam->mEventSignalQ);

    pEventData = pEventData; // warnings
    DVP_PRINT(DVP_ZONE_CAM, "OMXVisionCam::EventHandler() event=%d arg1=%08x arg2=%08x ptr=%p (len="FMT_SIZE_T")\n", eEvent, (uint32_t)nData1, (uint32_t)nData2, pEventData, len);
    // check the queue to see if we were waiting on an event
    if (len > 0)
    {
        OMXVCAM_Msg_t thisMsg = {eEvent, nData1, nData2, NULL, hComponent, -1};
        node_t thisNode = {NULL, NULL, (value_t)&thisMsg};

        // find the list event which matches this event
        node_t * found = list_search(pOMXCam->mEventSignalQ, &thisNode, node_compare_vcam_msg);
        if (found)
        {
            OMXVCAM_Msg_t *msg = (OMXVCAM_Msg_t *)found->data;
            DVP_PRINT(DVP_ZONE_CAM, "Found Event in List which matches incoming event!\n");
            if (msg->semaphore)
                semaphore_post(msg->semaphore);

            found = list_extract(pOMXCam->mEventSignalQ, found);
            free(found);
            free(msg);
        }
        else
        {
            DVP_PRINT(DVP_ZONE_CAM, "No matching event found in list!\n");
        }
    }

    switch( eEvent )
    {
        case OMX_EventError:
        {
            switch( nData1 )
            {
                case OMX_ErrorResourcesPreempted:
                    break;
                case OMX_ErrorResourcesLost:
                {
                    DVP_PRINT(DVP_ZONE_WARNING, "OMXVisionCam lost resources!\n");
                    pOMXCam->mPreemptionState = VCAM_PREEMPT_SUSPEND;
                    thread_create( PreemptionThreadLauncher, pOMXCam );
                    break;
                }
                case OMX_ErrorInsufficientResources:
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "OMXVisionCam has insufficient resources!\n");
                    pOMXCam->mPreemptionState = VCAM_PREEMPT_SUSPEND;
                    thread_create( PreemptionThreadLauncher, pOMXCam );
                    break;
                }
                case OMX_ErrorIncorrectStateOperation:
                {
                    break;
                }
                case OMX_ErrorInvalidState:
                {
                    break;
                }
                default:

                    break;
            }
            break;
        }
        case OMX_EventResourcesAcquired:
        {
            break;
        }
        case OMX_EventCmdComplete:
        {
            switch (nData1)
            {
                case OMX_CommandStateSet:
                    PrintOMXState((OMX_STATETYPE)nData2);
                    break;
                case OMX_CommandFlush:
                case OMX_CommandPortDisable:
                case OMX_CommandPortEnable:
                    break;
            }
            break;
        }
        case OMX_EventPortSettingsChanged:
        case OMX_EventIndexSettingChanged:
            DVP_PRINT(DVP_ZONE_CAM, "*** Port/Index Settings have Changed!\n");
            break;
        default:
          break;
    }
    return OMX_ErrorNone;
}

//GRE Empty buffer done callback
OMX_ERRORTYPE OMXVisionCam::EmptyBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                                             OMX_IN OMX_PTR pAppData,
                                             OMX_IN OMX_BUFFERHEADERTYPE* pBuffHeader)
{
//     OMXVisionCam *pOMXCam = (OMXVisionCam *)pAppData;
    pAppData = pAppData;
    hComponent = hComponent;
    pBuffHeader = pBuffHeader;
    return OMX_ErrorNotImplemented;
}

// #define PRINT_BUFFER_HEADER
#ifdef PRINT_BUFFER_HEADER
void printBufferHeader(OMX_BUFFERHEADERTYPE* pBuffHeader)
{
    static int32_t cnt = 0;

    if( cnt % 10 )
    {
        DVP_PRINT( DVP_ZONE_ALWAYS, "Dump OMX Buffer Header:\n");
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t nSize                   = %u\n",    pBuffHeader->nSize                  );
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t pBuffer                 = %p\n",    pBuffHeader->pBuffer                );
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t nAllocLen               = %u\n",    pBuffHeader->nAllocLen              );
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t nFilledLen              = %u\n",    pBuffHeader->nFilledLen             );
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t nOffset                 = %u\n",    pBuffHeader->nOffset                );
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t pAppPrivate             = %p\n",    pBuffHeader->pAppPrivate            );
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t pPlatformPrivate        = %p\n",    pBuffHeader->pPlatformPrivate       );
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t pInputPortPrivate       = %p\n",    pBuffHeader->pInputPortPrivate      );
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t pOutputPortPrivate      = %p\n",    pBuffHeader->pOutputPortPrivate     );
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t hMarkTargetComponent    = %p\n",    pBuffHeader->hMarkTargetComponent   );
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t pMarkData               = %p\n",    pBuffHeader->pMarkData              );
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t nTickCount              = %u\n",    pBuffHeader->nTickCount             );
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t nTimeStamp              = %ll\n",   pBuffHeader->nTimeStamp             );
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t nFlags                  = 0x%x\n",  pBuffHeader->nFlags                 );
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t nOutputPortIndex        = %u\n",    pBuffHeader->nOutputPortIndex       );
        DVP_PRINT( DVP_ZONE_ALWAYS, "\t nInputPortIndex         = %u\n",    pBuffHeader->nInputPortIndex        );
    }
    cnt++;
}
#endif // PRINT_BUFFER_HEADER

//GRE fill buffer done callback
OMX_ERRORTYPE OMXVisionCam::FillBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN OMX_PTR pAppData,
                                            OMX_IN OMX_BUFFERHEADERTYPE* pBuffHeader)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMXVisionCam *pOMXCam = (OMXVisionCam *)pAppData;

    hComponent = hComponent;
    DVP_PRINT(DVP_ZONE_CAM, "OMX-CAMERA has returned a frame (%p) Offset=%lu!\n", pBuffHeader->pAppPrivate,pBuffHeader->nOffset);

    SimpleMsg_t msg;
    msg.event = EFrameReceived;
    msg.data = (void*)pBuffHeader;

#ifdef PRINT_BUFFER_HEADER
    printBufferHeader(pBuffHeader);
#endif

    if( queue_write(pOMXCam->mFrameMessageQ, true_e, &msg) )
        semaphore_post(&pOMXCam->mFrameSem );
//    else HERE

    return eError;
}

void OMXVisionCam::frameReceivedSrvc(void *data)
{
    (this->*pFrameReceivedSrvc)(data);
}

void OMXVisionCam::FirstFrameFunc(void * data)
{
    if( mPendingConfigs )
    {
        for( int32_t pend = ePending_Min; pend < ePending_Max; pend++ )
        {
            switch( pend )
            {
            case ePending_Focus:
                VisionCamFocusMode *focData = (VisionCamFocusMode *)mPendingConfigs->getData(pend);
//                status_e (OMXVisionCam::*fn)(VisionCamFocusMode) = NULL;
//                fn = (status_e (OMXVisionCam::*)(VisionCamFocusMode))(mPendingConfigs->getFunc(pend));

                pendingConfigsFunPtr_t fn = mPendingConfigs->getFunc(pend);

                if( fn && focData )
                {
                    (this->*fn)(*focData);
                }
                break;
            }

        }
    }

    FrameReceivedFunc(data);
    pFrameReceivedSrvc = &OMXVisionCam::FrameReceivedFunc;
}

void OMXVisionCam::FrameReceivedFunc(void *data)
{
    OMX_BUFFERHEADERTYPE* pBuffHeader = (OMX_BUFFERHEADERTYPE* )data;
    VisionCamFrame *cFrame = NULL;
    VisionCamPort_e portInd = VCAM_PORT_MAX;

    if( !data )
    {
        return;
    }

    if( mFlushInProcess )
    {
        return;
    }

    SOSAL::AutoLock lock( &mFrameBufferLock );

    switch(pBuffHeader->nOutputPortIndex)
    {
        case VCAM_CAMERA_PORT_VIDEO_OUT_PREVIEW:
        {
            portInd = VCAM_PORT_PREVIEW;
            break;
        }
        case VCAM_CAMERA_PORT_VIDEO_OUT_VIDEO:
        {
            portInd = VCAM_PORT_VIDEO;
            break;
        }
        default:
        {
            portInd = VCAM_PORT_MAX;
            break;
        }
    }

    if( portInd > VCAM_PORT_ALL && portInd < VCAM_PORT_MAX && mCurGreContext.mCameraPortParams[portInd].mIsActive )
    {
        // find the frame descriptor corresponding to received buffer
        for( int i = 0; i < mCurGreContext.mCameraPortParams[portInd].mNumBufs; i++ )
        {
            if( mFrameDescriptors[portInd][i]->mFrameBuff == pBuffHeader->pAppPrivate )
            {
                cFrame = mFrameDescriptors[portInd][i];
                break;
            }
        }
    }

    if( cFrame )
    {
        cFrame->mFrameSource     = portInd;
        cFrame->mLength          = pBuffHeader->nFilledLen;
        cFrame->mTimestamp       = pBuffHeader->nTimeStamp;
        cFrame->mWidth           = mCurGreContext.mCameraPortParams[portInd].mWidth;
        cFrame->mHeight          = mCurGreContext.mCameraPortParams[portInd].mHeight;
        cFrame->mOffsetY         = pBuffHeader->nOffset/mBuffersInUse[portInd].mBuffers[0].y_stride;
        cFrame->mOffsetX         = pBuffHeader->nOffset%mBuffersInUse[portInd].mBuffers[0].y_stride;
        cFrame->mCookie          = m_cookie;
        cFrame->mContext         = this;

        DVP_Image_t *pImage = (DVP_Image_t *)cFrame->mFrameBuff;
        /*! \note FOURCC_BIN1 is not a possible Camera format yet, but this calc would be wrong if it were */
        pImage->x_start = cFrame->mOffsetX/pImage->x_stride;
        pImage->y_start = cFrame->mOffsetY;

        OMX_TI_PLATFORMPRIVATE *platformPrivate = (OMX_TI_PLATFORMPRIVATE *)(pBuffHeader->pPlatformPrivate);

        if( platformPrivate && platformPrivate->pMetaDataBuffer && platformPrivate->nMetaDataSize > 0)
        {
            OMX_OTHER_EXTRADATATYPE *pExtraData = (OMX_OTHER_EXTRADATATYPE *)platformPrivate->pMetaDataBuffer;
            if (pExtraData->eType >= OMX_ExtraDataVendorStartUnused && pExtraData->eType < OMX_ExtraDataMax)
            {
                DVP_PRINT(DVP_ZONE_CAM, "METADATA: Extra Data Type is TI Specific!\n");
                cFrame->mExtraDataBuf    = pExtraData;
                cFrame->mExtraDataLength = platformPrivate->nMetaDataSize; // use this to calculate a size limit

                if( mFaceDetectionEnabled & VCAM_FACE_DETECTION_ALL )
                {
                    getFacesCoordinates( cFrame );
                }

                getMetadataPtrs( cFrame );
            }
            else
            {
                if (pExtraData->eType != OMX_ExtraDataNone)
                {
                    DVP_PRINT(DVP_ZONE_WARNING, "METADATA: Unsupported extra data type from camera (0x%x)\n", pExtraData->eType);
                }
                platformPrivate = NULL;
            }
        }
        else
        {
            platformPrivate = NULL;
        }

        if (platformPrivate == NULL)
        {
            cFrame->mMetadata.mAutoWBGains = NULL;
            cFrame->mMetadata.mManualWBGains = NULL;
            cFrame->mMetadata.mAncillary = NULL;
            cFrame->mMetadata.mHistogram2D = NULL;
            cFrame->mMetadata.mHistogramL = NULL;
            cFrame->mMetadata.mHistogramR = NULL;
            cFrame->mMetadata.mGamma2D = NULL;
            cFrame->mMetadata.mGammaL = NULL;
            cFrame->mMetadata.mGammaR = NULL;
            cFrame->mExtraDataBuf    = NULL;
            cFrame->mExtraDataLength = 0;
            cFrame->mDetectedFacesNum = 0;
            memset( cFrame->mFaces, 0, MAX_FACES_COUNT * sizeof(VisionCamFaceType) );
            cFrame->mDetectedFacesNumRaw = 0;
            memset( cFrame->mFacesRaw, 0, MAX_FACES_COUNT * sizeof(VisionCamFaceType) );
        }

        if( VCAM_PORT_PREVIEW == cFrame->mFrameSource )
        {
            m_frameNum++;
        }

        if( mUseFramePackaging )
        {
            DVP_PRINT(DVP_ZONE_CAM, "Inside the Frame Packing case...\n" );
            if( m_pack_callback != NULL )
            {
                DVP_PRINT(DVP_ZONE_CAM, "Non null frame pack callback available...\n" );
                if( mFramePackage.mExpectedFrames[cFrame->mFrameSource] )
                {
                    if( mFramePackage.mIsEmpty )
                    {
                        mFramePackage.mIsEmpty = false_e;
                        mFramePackage.mTimestamp = cFrame->mTimestamp;
                    }

                    if( mFramePackage.mTimestamp == cFrame->mTimestamp )
                    {/// @todo check if a frame from this port and with this time stamp  has already arrived. This would mean a dicati issue
                       mFramePackage.mFrame[cFrame->mFrameSource] = cFrame;
                    }
                    else
                    {
                        /// @todo add to queue
                        DVP_PRINT(DVP_ZONE_CAM, "Loosing a frame here port:%d...\n", cFrame->mFrameSource);
                    }
                }

                bool_e ready = true_e;
                for(int32_t i = (VCAM_PORT_ALL + 1); i < VCAM_PORT_MAX; i++)
                {
                    if( !mFramePackage.mFrame[i] && mFramePackage.mExpectedFrames[i] )
                    {
                        ready = false_e;
                        break;
                    }
                }
                DVP_PRINT(DVP_ZONE_CAM, "Waiting to get ready with all frames...\n" );
                if(ready)
                {
                    DVP_PRINT(DVP_ZONE_CAM, "Ready, calling pack callback...\n" );
                    m_pack_callback(&mFramePackage);
                    for(int32_t i = (VCAM_PORT_ALL + 1); i < VCAM_PORT_MAX; i++)
                    {
                        mFramePackage.mFrame[i] = NULL;
                    }
                    mFramePackage.mIsEmpty = true_e;
                }
            }
        }
        else
        {
            if (m_callback != NULL)
            {
                m_callback(cFrame);
            }
        }
    }
    else
    {
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR: a frame buffer is received, but there is no matching buffer.\n" );
    }
}

#if GET_SENSOR_CAPS
void OMXVisionCam::getSensorCaps(VCamSensorInfo& s )
{
    OMX_ERRORTYPE omxError = OMX_ErrorUndefined;
    OMX_TI_CAPTYPE *caps = NULL;
    OMX_TI_CONFIG_SHAREDBUFFER sharedBuffer;
    DataBuffer_t *pBuffer = new DataBuffer_t(sizeof(OMX_TI_CAPTYPE));

    if( pBuffer )
    {
        OMX_STRUCT_INIT(sharedBuffer, OMX_TI_CONFIG_SHAREDBUFFER, mLocalVersion);
        sharedBuffer.nPortIndex         = mCurGreContext.mPortsInUse[VCAM_PORT_ALL];
        sharedBuffer.nSharedBuffSize    = pBuffer->getSize();
        sharedBuffer.pSharedBuff        = (OMX_U8*)pBuffer->getData();

        caps = (OMX_TI_CAPTYPE*)pBuffer->getData();
        OMX_STRUCT_INIT_PTR(caps, OMX_TI_CAPTYPE, mLocalVersion);

        // Get capabilities from OMX Camera
        omxError =  OMX_GetConfig(mCurGreContext.mHandleComp, (OMX_INDEXTYPE) OMX_TI_IndexConfigCamCapabilities, &sharedBuffer);
        if ( OMX_ErrorNone == omxError )
        {
    //        s.supportedModesCnt = caps->
        }

        delete pBuffer;
        pBuffer = NULL;
    }
}
#endif // GET_SENSOR_CAPS

#if TIME_PROFILE
void OMXVisionCam::PopulateTimeProfiler()
{
    mTimeProfiler[ first ] = new VisionCamTimePtofile("first");
    mTimeProfiler[ second ] = new VisionCamTimePtofile();
    mTimeProfiler[ last ] = new VisionCamTimePtofile();
}

VisionCamTimePtofile::VisionCamTimePtofile( const char * name ){
  memset( &mStart, 0, sizeof(struct timeval) );
  memset( &mEnd, 0, sizeof(struct timeval) );

  if( name )
    mName = name;
  else
    mName = "unknown";
};

VisionCamTimePtofile::~VisionCamTimePtofile(){
    dump();
};

void VisionCamTimePtofile::dump() {
    if( mEnd.tv_usec && mStart.tv_usec )
    {
        double time = (double)(mEnd.tv_usec = mStart.tv_usec) / (double)1000;
        printf("TIME: %s - %g [ms]\n\r", mName, time );
        memset( &mStart, 0, sizeof(struct timeval) );
        memset( &mEnd, 0, sizeof(struct timeval) );
    }
}

#endif // TIME_PROFILE
