
#include <CSVisionCam.h>

#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))

#ifdef KEY_ZOOM
#undef KEY_ZOOM
#endif

#include <camera/CameraParameters.h>

const char  PIXEL_FORMAT_YUV420SP[] = "yuv420sp";
const char  PIXEL_FORMAT_YUV422I[] = "yuv422i-yuyv";

const char KEY_SATURATION[] = "saturation";
const char KEY_BRIGHTNESS[] = "brightness";
const char KEY_CONTRAST[] = "contrast";
const char KEY_SHARPNESS[] = "sharpness";

const char * FLICKER_STR[] = {
    "FLICKER_OFF",
    "FLICKER_AUTO",
    "FLICKER_50Hz",
    "FLICKER_60Hz"
};

CSVisionCam::CSVisionCam() :
    mCamera(NULL),
    mSurfaceTexture(NULL),
    mFrameIndex(0),
    mConnected(false_e),
    mFaceDetectionEnabled(false_e),
    mWidth(0),
    mHeight(0),
    mColorSpace(FOURCC_YUY2)
{
    memset(&mVfr, 0, sizeof(mVfr));

    mContrast = 0;
    mSharpness = 0;
    mBrightness = 0;
    mSaturation = 0;
    mEVCompensation = 0;

    mFlicker = FLICKER_OFF;

    mInited = false_e;
}

CSVisionCam::~CSVisionCam()
{
}

// interface methods
status_e CSVisionCam::init(void *cookie)
{
    DVP_PRINT(DVP_ZONE_CAM, "init: cookie= %p\n", cookie);
    status_e status = initFrameDescriptors();
    if (status != STATUS_SUCCESS)
    {
        DVP_PRINT(DVP_ZONE_CAM, "init: initFrameDescriptors failed! error= %d\n", status);
        return status;
    }

    mInited = true_e;

    return STATUS_SUCCESS;
}

status_e CSVisionCam::deinit()
{
    if (mInited == false_e)
        return STATUS_INVALID_STATE;

    if (mCamera != NULL)
    {
        if (mConnected == true_e)
        {
            if (mCamera->previewEnabled())
                stopPreview();

            mCamera->disconnect();
            mConnected = false_e;
        }

        mCamera->unlock();
        mCamera.clear();
        mCamera = NULL;
    }

    deinitFrameDescriptors();

    return STATUS_SUCCESS;
}

status_e CSVisionCam::useBuffers(DVP_Image_t *prvBufArr __attribute__((unused)),
                                 uint32_t numPrvBuf __attribute__((unused)),
                                 VisionCamPort_e port __attribute__((unused)))
{
    return STATUS_NOT_IMPLEMENTED;
}

status_e CSVisionCam::releaseBuffers(VisionCamPort_e port __attribute__((unused)))
{
    return STATUS_NOT_IMPLEMENTED;
}

status_e CSVisionCam::flushBuffers(VisionCamPort_e port __attribute__((unused)))
{
    return STATUS_NOT_IMPLEMENTED;
}

status_e CSVisionCam::sendCommand(VisionCamCmd_e cmdId, void *param, uint32_t size, VisionCamPort_e port __attribute__((unused)))
{
    status_e status = STATUS_SUCCESS;
    DVP_PRINT(DVP_ZONE_CAM, "SEND CMD: 0x%04x, %p, "FMT_SIZE_T"\n", cmdId, param, size);
    switch (cmdId)
    {
        case VCAM_CMD_PREVIEW_START:
            status = startPreview();
            break;

        case VCAM_CMD_PREVIEW_STOP:
            status = stopPreview();
            break;

        case VCAM_EXTRA_DATA_START:
            break;

        case VCAM_EXTRA_DATA_STOP:
            break;

        case VCAM_CMD_LOCK_AWB:
            status = lockAutoWhitebalance(true_e);
            break;

        case VCAM_CMD_LOCK_AE:
            status = lockAutoExposure(true_e);
            break;

        case VCAM_CMD_FACE_DETECTION:
            status = toggleFaceDetection();
            break;

        default:
            status = STATUS_NOT_IMPLEMENTED;
            break;
    }
    return status;
}

status_e CSVisionCam::setParameter(VisionCamParam_e paramId, void* param, uint32_t size, VisionCamPort_e port __attribute__((unused)))
{
    status_e status = STATUS_SUCCESS;

    if (param == NULL || size == 0)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "NULL param pointer passed to %s()\n",__func__);
        return STATUS_INVALID_PARAMETER;
    }
    else
    {
        DVP_PRINT(DVP_ZONE_CAM, "SET PARAM: 0x%04x, %p, "FMT_SIZE_T" (0x%08x)\n", paramId, param, size, (size==4?*(uint32_t *)param:0));
    }

    switch (paramId)
    {
        case VCAM_PARAM_WIDTH:
            if (size == sizeof(uint32_t))
            {
                mWidth = *(uint32_t *)param;
            }
            else
                status = STATUS_INVALID_PARAMETER;
            break;

        case VCAM_PARAM_HEIGHT:
            if (size == sizeof(uint32_t))
            {
                mHeight = *(uint32_t *)param;
            }
            else
                status = STATUS_INVALID_PARAMETER;
            break;

        case VCAM_PARAM_FPS_VAR:
            if (size == sizeof(VisionCamVarFramerateType))
            {
                mVfr = *(VisionCamVarFramerateType *)param;
            }
            else
                status = STATUS_INVALID_PARAMETER;
            break;

        case VCAM_PARAM_COLOR_SPACE_FOURCC:
            if (size == sizeof(_fourcc) && colorSpaceToPreviewFormat(*((_fourcc*)param)))
            {
                mColorSpace = *((_fourcc*)param);
            }
            else
                status = STATUS_INVALID_PARAMETER;
            break;

        case VCAM_PARAM_SENSOR_SELECT:
            if (size == sizeof(VisionCamSensorSelection))
            {
                mCameraId = *(VisionCamSensorSelection *)param;
                status = connect(mCameraId);
                if (status != STATUS_SUCCESS)
                {
                    DVP_PRINT(DVP_ZONE_CAM, "Failed to connect to camera %d, error= %d\n",
                        mCameraId, status);
                }
            }
            else
                status = STATUS_INVALID_PARAMETER;
            break;

        case VCAM_PARAM_PREVIEW_TEXTURE:
            if (param != NULL)
            {
                mSurfaceTexture = reinterpret_cast<SurfaceTexture *>(param);
                DVP_PRINT(DVP_ZONE_CAM, "Using SurfaceTexture= %p\n", mSurfaceTexture.get());
            }
            else
                status = STATUS_INVALID_PARAMETER;
            break;

        case VCAM_PARAM_CONTRAST:
            if (size == sizeof(int32_t))
            {
                mContrast = *(int32_t *)param;
            }
            else
                status = STATUS_INVALID_PARAMETER;
            break;

        case VCAM_PARAM_SHARPNESS:
            if (size == sizeof(int32_t))
            {
                mSharpness = *(int32_t *)param;
            }
            else
                status = STATUS_INVALID_PARAMETER;
            break;

        case VCAM_PARAM_BRIGHTNESS:
            if (size == sizeof(int32_t))
            {
                mBrightness = *(int32_t *)param;
            }
            else
                status = STATUS_INVALID_PARAMETER;
            break;

        case VCAM_PARAM_SATURATION:
            if (size == sizeof(int32_t))
            {
                mSaturation = *(int32_t *)param;
            }
            else
                status = STATUS_INVALID_PARAMETER;
            break;

        case VCAM_PARAM_FLICKER:
            if (size == sizeof(VisionCamFlickerType))
            {
                status = setFlicker(*(VisionCamFlickerType *)param);
            }
            else
                status = STATUS_INVALID_PARAMETER;
            break;

        case VCAM_PARAM_EXPOSURE_COMPENSATION:
            if (size == sizeof(int32_t))
            {
                status = setExposureCompensation(*(int32_t *)param);
            }
            else
                status = STATUS_INVALID_PARAMETER;
            break;

        default:
            status = STATUS_NOT_IMPLEMENTED;
            break;
    }
    return status;
}

status_e CSVisionCam::getParameter(VisionCamParam_e paramId, void* param, uint32_t size, VisionCamPort_e port __attribute__((unused)))
{
    status_e status = STATUS_SUCCESS;

    if (param == NULL || size == 0)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "NULL param pointer passed to %s\n", __func__);
        return STATUS_INVALID_PARAMETER;
    }
    else
    {
        DVP_PRINT(DVP_ZONE_CAM, "GET PARAM: 0x%04x, %p, %u (0x%08x)\n", paramId, param, size, (size==4?*(uint32_t *)param:0));
    }

    switch (paramId)
    {
        case VCAM_PARAM_WIDTH:
            *(uint32_t*)param = mWidth;
            break;

        case VCAM_PARAM_HEIGHT:
            *(uint32_t*)param = mHeight;
            break;

        case VCAM_PARAM_FPS_VAR:
            *(VisionCamVarFramerateType *)param = mVfr;
            break;

        case VCAM_PARAM_COLOR_SPACE_FOURCC:
            *((_fourcc*)param) = mColorSpace;
            break;

        case VCAM_PARAM_SENSOR_SELECT:
            *(VisionCamSensorSelection *)param = mCameraId;
            break;

        case VCAM_PARAM_CONTRAST:
            *(int32_t*)param = mContrast;
            break;

        case VCAM_PARAM_SHARPNESS:
            *(int32_t*)param = mSharpness;
            break;

        case VCAM_PARAM_BRIGHTNESS:
            *(int32_t*)param = mBrightness;
            break;

        case VCAM_PARAM_SATURATION:
            *(int32_t*)param = mSaturation;
            break;

        case VCAM_PARAM_FLICKER:
            *(VisionCamFlickerType *)param = mFlicker;
            break;

        case VCAM_PARAM_EXPOSURE_COMPENSATION:
            *(int32_t*)param = mEVCompensation;
            break;

        default:
            status = STATUS_NOT_IMPLEMENTED;
            break;
    }
    return status;
}

status_e CSVisionCam::returnFrame(VisionCamFrame *cameraFrame)
{
    cameraFrame->clear();
    return STATUS_SUCCESS;
}

void CSVisionCam::dataCallback(int32_t msgType,
                               const sp<IMemory> &data,
                               camera_frame_metadata_t *metadata)
{
    VisionCamFrame *cFrame = mFrameDescriptors[mFrameIndex];

    mFrameIndex = (mFrameIndex + 1) % MAX_FRAMES;

    cFrame->mFrameSource     = VCAM_PORT_PREVIEW;
    cFrame->mFrameBuff       = data->pointer();
    cFrame->mLength          = data->size();
    cFrame->mTimestamp       = rtimer_now();
    cFrame->mWidth           = mWidth;
    cFrame->mHeight          = mHeight;
    cFrame->mCookie          = m_cookie;
    cFrame->mContext         = this;

    if (msgType & CAMERA_MSG_PREVIEW_METADATA)
    {
        if (mFaceDetectionEnabled && (metadata != NULL))
        {
            cFrame->mDetectedFacesNum = metadata->number_of_faces;
            for (int idx = 0 ; idx < metadata->number_of_faces ; idx++)
            {
                cFrame->mFaces[idx].mScore = metadata->faces[idx].score;

                cFrame->mFaces[idx].mFacesCoordinates.mLeft = metadata->faces[idx].rect[0];
                cFrame->mFaces[idx].mFacesCoordinates.mTop = metadata->faces[idx].rect[1];
                cFrame->mFaces[idx].mFacesCoordinates.mWidth = metadata->faces[idx].rect[2];
                cFrame->mFaces[idx].mFacesCoordinates.mHeight = metadata->faces[idx].rect[3];
            }
        }
    }

    // make sure IMemory is not freed prematurely !?

    if (m_callback != NULL)
    {
        m_callback(cFrame);
    }
    else
    {
        delete cFrame;
    }
}

void CSVisionCam::dataCallbackTimestamp(int64_t timestampUs __attribute__((unused)),
                                        int32_t msgType __attribute__((unused)),
                                        const sp<IMemory> &data __attribute__((unused)))
{
}

status_e CSVisionCam::connect(VisionCamSensorSelection cameraId)
{
    DVP_PRINT(DVP_ZONE_CAM, "connect: camera %d\n", cameraId);
    if ((mConnected == true_e) && (mCamera != NULL))
    {
        if (mCamera->previewEnabled())
            stopPreview();

        mCamera->disconnect();
    }

    mCamera = Camera::connect(cameraId);
    if (mCamera == NULL)
    {
        mConnected = false_e;
        return STATUS_FAILURE;
    }

    mCamera->lock();

    initCameraParameters();

    mConnected = true_e;

    return STATUS_SUCCESS;
}

status_e CSVisionCam::startPreview()
{
    status_t err;
    if (mCamera == NULL)
    {
        DVP_PRINT(DVP_ZONE_CAM, "startPreview camera is null\n");
        return STATUS_INVALID_STATE;
    }

    status_e status = setCameraParameters();
    if (status != STATUS_SUCCESS)
    {
        DVP_PRINT(DVP_ZONE_CAM, "startPreview: setParameters failed! status= 0x%x\n", status);
        return status;
    }

#if defined(JELLYBEAN)
    sp<BufferQueue> bufferQueue = NULL;
    if (mSurfaceTexture == NULL)
    {
        DVP_PRINT(DVP_ZONE_CAM, "startPreview: mSurfaceTexture is NULL!\n");
        return STATUS_FAILURE;
    }

    bufferQueue = mSurfaceTexture->getBufferQueue();
    err = mCamera->setPreviewTexture(bufferQueue);
#else
    err = mCamera->setPreviewTexture(mSurfaceTexture);
#endif
    if (err)
    {
        DVP_PRINT(DVP_ZONE_CAM, "startPreview: setPreviewTexture failed! error= %d\n", err);
        return STATUS_FAILURE;
    }

    mCamera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_CAMERA);

//    mCamera->setListener(new VisionCamListener(this));
    mCamera->setListener(this);

    DVP_PRINT(DVP_ZONE_CAM, "startPreview: starting preview %dx%d:0x%x",
              mWidth, mHeight, mColorSpace);

    err = mCamera->startPreview();
    if (err)
    {
        DVP_PRINT(DVP_ZONE_CAM, "startPreview: startPreview failed! error= %d\n", err);
        return STATUS_FAILURE;
    }

    return STATUS_SUCCESS;
}


status_e CSVisionCam::stopPreview()
{
    if (mCamera == NULL)
        return STATUS_INVALID_STATE;

    mCamera->stopPreview();

    mCamera->setListener(NULL);

    mCamera->setPreviewTexture(NULL);

    return STATUS_SUCCESS;
}

status_e CSVisionCam::toggleFaceDetection()
{
    status_t err;

    if (mCamera == NULL)
        return STATUS_INVALID_STATE;

    DVP_PRINT(DVP_ZONE_CAM, "toggleFaceDetection: %s face detection ...\n",
              mFaceDetectionEnabled?"stopping":"starting");

    if (mFaceDetectionEnabled)
    {
        err = mCamera->sendCommand(CAMERA_CMD_STOP_FACE_DETECTION, 0, 0);
    }
    else
    {
        err = mCamera->sendCommand(CAMERA_CMD_START_FACE_DETECTION, 0, 0);
    }

    if (err)
    {
        DVP_PRINT(DVP_ZONE_CAM, "toggleFaceDetection: %s Face Detection failed! error= %d\n",
                  mFaceDetectionEnabled?"stopping":"starting", err);
        return STATUS_FAILURE;
    }

    mFaceDetectionEnabled = (bool_e)!mFaceDetectionEnabled;

    return STATUS_SUCCESS;
}

status_e CSVisionCam::lockAutoWhitebalance(bool_e lock)
{
    status_t err;
    CameraParameters params(mCamera->getParameters());

    if (lock == true_e)
        params.set(android::CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK, android::CameraParameters::TRUE);
    else
        params.set(android::CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK, android::CameraParameters::FALSE);

    err = mCamera->setParameters(params.flatten());
    if (err)
    {
        DVP_PRINT(DVP_ZONE_CAM, "lockAutoWhitebalance: setParameters failed! error= %d\n", err);
        return STATUS_FAILURE;
    }

    return STATUS_SUCCESS;
}

status_e CSVisionCam::lockAutoExposure(bool_e lock)
{
    status_t err;
    CameraParameters params(mCamera->getParameters());

    if (lock == true_e)
        params.set(android::CameraParameters::KEY_AUTO_EXPOSURE_LOCK, android::CameraParameters::TRUE);
    else
        params.set(android::CameraParameters::KEY_AUTO_EXPOSURE_LOCK, android::CameraParameters::FALSE);

    err = mCamera->setParameters(params.flatten());
    if (err)
    {
        DVP_PRINT(DVP_ZONE_CAM, "lockAutoExposure: setParameters failed! error= %d\n", err);
        return STATUS_FAILURE;
    }

    return STATUS_SUCCESS;
}

status_e CSVisionCam::setFlicker(VisionCamFlickerType type)
{
    status_t err;
    CameraParameters params(mCamera->getParameters());

    const char *flicker = flickerToAntibanding(type);
    if (flicker == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }

    params.set(android::CameraParameters::KEY_ANTIBANDING, flicker);

    err = mCamera->setParameters(params.flatten());
    if (err)
    {
        DVP_PRINT(DVP_ZONE_CAM, "setFlicker: setParameters failed! error= %d\n", err);
        return STATUS_FAILURE;
    }

    mFlicker = type;

    return STATUS_SUCCESS;
}

status_e CSVisionCam::setExposureCompensation(int32_t compensation)
{
    status_t err;
    CameraParameters params(mCamera->getParameters());

    char tmpBuffer[16];
    sprintf(tmpBuffer, "%d", compensation);

    params.set(android::CameraParameters::KEY_EXPOSURE_COMPENSATION, tmpBuffer);

    err = mCamera->setParameters(params.flatten());
    if (err)
    {
        DVP_PRINT(DVP_ZONE_CAM, "setExposureCompensation: setParameters failed! error= %d\n", err);
        return STATUS_FAILURE;
    }

    mEVCompensation = compensation;

    return STATUS_SUCCESS;
}

status_e CSVisionCam::initFrameDescriptors()
{
    status_e error = STATUS_SUCCESS;

    for (uint32_t i = 0; i < MAX_FRAMES; i++)
    {
        mFrameDescriptors[i] = new VisionCamFrame();

        if (mFrameDescriptors[i])
        {
            mFrameDescriptors[i]->clear();
        }
        else
        {
            while (i)
                delete mFrameDescriptors[--i];

            error = STATUS_NOT_ENOUGH_MEMORY;
        }
    }

    return error;
}

status_e CSVisionCam::deinitFrameDescriptors()
{
    for (uint32_t i = 0; i < MAX_FRAMES; i++)
    {
        delete mFrameDescriptors[i];
    }

    return STATUS_SUCCESS;
}

status_e CSVisionCam::initCameraParameters()
{
    CameraParameters params(mCamera->getParameters());

    params.getPreviewSize((int *)&mWidth, (int *)&mHeight);
    params.getPreviewFpsRange((int *)&mVfr.mMin, (int *)&mVfr.mMax);

    mColorSpace = previewFormatToColorSpace(params.getPreviewFormat());
    mFlicker = antibandingToFlicker(params.get(android::CameraParameters::KEY_ANTIBANDING));
    mEVCompensation = (int32_t)params.getInt(android::CameraParameters::KEY_EXPOSURE_COMPENSATION);

    mContrast = (int32_t)params.getInt(KEY_CONTRAST);
    mSharpness = (int32_t)params.getInt(KEY_SHARPNESS);
    mBrightness = (int32_t)params.getInt(KEY_BRIGHTNESS);
    mSaturation = (int32_t)params.getInt(KEY_SATURATION);

    DVP_PRINT(DVP_ZONE_CAM, "initCameraParameters:\n");
    DVP_PRINT(DVP_ZONE_CAM, "       Width, Height= %dx%d\n", mWidth, mHeight);
    DVP_PRINT(DVP_ZONE_CAM, "       Fps Range= %d:%d\n", mVfr.mMin, mVfr.mMax);
    DVP_PRINT(DVP_ZONE_CAM, "       ColorSpace= %s\n", fourcctostr(mColorSpace));
    DVP_PRINT(DVP_ZONE_CAM, "       Flicker= %s\n", flickerToString(mFlicker));
    DVP_PRINT(DVP_ZONE_CAM, "       Exposure Compensation= %d\n", mEVCompensation);
    DVP_PRINT(DVP_ZONE_CAM, "       Contrast= %d\n", mContrast);
    DVP_PRINT(DVP_ZONE_CAM, "       Sharpness= %d\n", mSharpness);
    DVP_PRINT(DVP_ZONE_CAM, "       Brightness= %d\n", mBrightness);
    DVP_PRINT(DVP_ZONE_CAM, "       Saturation= %d\n", mSaturation);

    return STATUS_SUCCESS;
}

status_e CSVisionCam::setCameraParameters()
{
    status_t err;
    char tmpBuffer[64];
    CameraParameters params(mCamera->getParameters());

    params.setPreviewSize(mWidth, mHeight);

    const char *previewFormat = colorSpaceToPreviewFormat(mColorSpace);
    if (previewFormat == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }

    params.setPreviewFormat(previewFormat); // YUY2

    const char *flicker = flickerToAntibanding(mFlicker);
    if (flicker == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }

    params.set(android::CameraParameters::KEY_ANTIBANDING, flicker);

    sprintf(tmpBuffer, "%d", mEVCompensation);
    params.set(android::CameraParameters::KEY_EXPOSURE_COMPENSATION, tmpBuffer);

    sprintf(tmpBuffer, "%d,%d", mVfr.mMin * 1000, mVfr.mMax * 1000);
    params.set(android::CameraParameters::KEY_PREVIEW_FPS_RANGE, tmpBuffer);

    sprintf(tmpBuffer, "%d", mContrast);
    params.set(KEY_CONTRAST, tmpBuffer);

    sprintf(tmpBuffer, "%d", mSharpness);
    params.set(KEY_SHARPNESS, tmpBuffer);

    sprintf(tmpBuffer, "%d", mBrightness);
    params.set(KEY_BRIGHTNESS, tmpBuffer);

    sprintf(tmpBuffer, "%d", mSaturation);
    params.set(KEY_SATURATION, tmpBuffer);

    err = mCamera->setParameters(params.flatten());
    if (err)
    {
        return STATUS_FAILURE;
    }

    return STATUS_SUCCESS;
}

void CSVisionCam::notify(int32_t msgType, int32_t ext1, int32_t ext2)
{
    DVP_PRINT(DVP_ZONE_CAM, "notify(0x%x, %d, %d)", msgType, ext1, ext2);
}

void CSVisionCam::postData(int32_t msgType,
                           const sp<IMemory> &dataPtr,
                           camera_frame_metadata_t *metadata)
{
    DVP_PRINT(DVP_ZONE_CAM, "postData(0x%x, ptr:%p, size:%d, meta:%p)",
        msgType, dataPtr->pointer(), dataPtr->size(), metadata);

    if (msgType & CAMERA_MSG_PREVIEW_FRAME)
    {
        dataCallback(msgType, dataPtr, metadata);
    }
}

void CSVisionCam::postDataTimestamp(nsecs_t timestamp,
                                    int32_t msgType,
                                    const sp<IMemory>& dataPtr)
{
    DVP_PRINT(DVP_ZONE_CAM, "postDataTimestamp(0x%x, ptr:%p, size:%d)",
        msgType, dataPtr->pointer(), dataPtr->size());
    dataCallbackTimestamp(timestamp/1000, msgType, dataPtr);
}

const char* CSVisionCam::colorSpaceToPreviewFormat(_fourcc colorSpace)
{
    switch (colorSpace)
    {
        case FOURCC_YUY2:
        case FOURCC_UYVY:
            return PIXEL_FORMAT_YUV422I;
            break;
        case FOURCC_NV21:
            return PIXEL_FORMAT_YUV420SP;
            break;
        default:
            break;
    }

    return NULL;
}

_fourcc CSVisionCam::previewFormatToColorSpace(const char* previewFormat)
{
    if (!strcmp(previewFormat, PIXEL_FORMAT_YUV422I))
    {
        return FOURCC_NV21;
    }
    else if (!strcmp(previewFormat, PIXEL_FORMAT_YUV420SP))
    {
        return FOURCC_YUY2;
    }

    return FOURCC_NONE;
}

const char* CSVisionCam::flickerToAntibanding(VisionCamFlickerType type)
{
    switch (type)
    {
        case FLICKER_OFF:
            return android::CameraParameters::ANTIBANDING_OFF;
            break;
        case FLICKER_AUTO:
            return android::CameraParameters::ANTIBANDING_AUTO;
            break;
        case FLICKER_50Hz:
            return android::CameraParameters::ANTIBANDING_50HZ;
            break;
        case FLICKER_60Hz:
            return android::CameraParameters::ANTIBANDING_60HZ;
            break;
        default:
            break;
    }

    return NULL;
}

VisionCamFlickerType CSVisionCam::antibandingToFlicker(const char* antibanding)
{
    if (!strcmp(antibanding, android::CameraParameters::ANTIBANDING_AUTO))
    {
        return FLICKER_AUTO;
    }
    else if (!strcmp(antibanding, android::CameraParameters::ANTIBANDING_50HZ))
    {
        return FLICKER_50Hz;
    }
    else if (!strcmp(antibanding, android::CameraParameters::ANTIBANDING_60HZ))
    {
        return FLICKER_60Hz;
    }

    return FLICKER_OFF;
}

const char* CSVisionCam::flickerToString(VisionCamFlickerType flicker)
{
    if ((uint32_t)flicker >= sizeof(FLICKER_STR))
        return "UNKNOWN";

    return FLICKER_STR[flicker];
}

#endif // ANDROID