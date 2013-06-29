
#ifndef _CS_VISION_CAM_H_
#define _CS_VISION_CAM_H_

#include <dvp/VisionCam.h>

#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))

#include <binder/IMemory.h>
#include <camera/ICamera.h>
#include <gui/SurfaceTexture.h>

using namespace android;

/*! Maximum number of buffers that can be used for returning camera frames. */
const uint32_t MAX_FRAMES = 8;

/*! \brief Implements access to the camera using Android's Camera Service.
 * \implements VisionCam
 *
 */
class CSVisionCam : public VisionCam,
                    public CameraListener,
                    public virtual RefBase
{
public:
    /*! Constructor. */
    CSVisionCam();
    /*! Destructor. */
    virtual ~CSVisionCam();

    // implementation of VisionCam interface methods

    /*! \brief Initialises CameraService VisionCam.
     *
     * Also allocates frame buffers.
     * \param [in] cookie A token later passed back to the client.
     * \retval STATUS_SUCCESS on success.
     * \retval STATUS_NOT_ENOUGH_MEMORY if no memory could be internally allocated.
     * \post \ref deinit
     */
    status_e init(void * cookie);

    /*! \brief Deinits CameraService VisionCam.
     *
     * This method also stops preview if running, disconnects from the camera
     * and frees the frame buffers.
     * \retval STATUS_SUCCESS on success.
     * \retval STATUS_INVALID_STATE if it has not been inited first.
     * \pre \ref init
     */
    status_e deinit();

    /*! \brief Registers buffers used for frame descriptoprs.
     *
     * Not implemented because CameraService allocates its own buffers.
     * \param [in] *prvBufArr array of buffers as they are allocated.
     * \param [in] numPrvBuf  number of the buffers in this array.
     * \param [in] port       the port on which these buffers must be assigned to.
     * \retval STATUS_NOT_IMPLEMENTED.
     * \post \ref releaseBuffers
     *
     * \see VisionCamPort_e for available ports.
     */
    status_e useBuffers(DVP_Image_t *prvBufArr, uint32_t numPrvBuf, VisionCamPort_e port=VCAM_PORT_PREVIEW);

    /*! \brief Unregisters previously registered buffers.
     *
     * Not implemented because CameraService uses its own buffers.
     * \param [in] port the port on which these buffers are assigned.
     * \retval STATUS_NOT_IMPLEMENTED.
     * \pre \ref useBuffers
     *
     * \see VisionCamPort_e for available ports.
     */
    status_e releaseBuffers(VisionCamPort_e port=VCAM_PORT_PREVIEW);

    /*! \brief Flushes the buffers used in VisionCam
     *
     * Not implemented because CameraService uses its own buffers.
     * \param [in] port the port on which these buffers are assigned.
     * \retval STATUS_NOT_IMPLEMENTED.
     * \pre \ref useBuffers
     *
     * \see VisionCamPort_e for available ports.
     */
    status_e flushBuffers(VisionCamPort_e port=VCAM_PORT_PREVIEW);

    /*! \brief Requests camera to execute some additional functionality.
     *
     * \param [in] cmdId the command id that bust be executed.
     * \param [in] param pointer to additional data that refers to this command.
     * \param [in] size  size of data pointed by param.
     * \param [in] port  the port on which this must be executed.
     * \retval STATUS_SUCCESS on success.
     * \retval STATUS_FAILURE if unable to complete the command successfully.
     * \retval STATUS_NOT_IMPLEMENTED if the requested command id is not found.
     * \pre \ref init
     *
     * \see VisionCamCmd_e  for possible commands.
     * \see VisionCamPort_e for available ports.
     */
    status_e sendCommand(VisionCamCmd_e cmdId, void *param, uint32_t size, VisionCamPort_e port=VCAM_PORT_PREVIEW);

    /*! \brief Configures the camera with the supplied parameter.
     *
     * \param [in] paramId ID of the parameter that has to be applied.
     * \param [in] param   Pointer to parameter data.
     * \param [in] size    Size of parameter data.
     * \param [in] port    Output port for which this parameters will be applied.
     * \retval STATUS_SUCCESS on success
     * \retval STATUS_FAILURE if unable to set the parameter successfully.
     * \retval STATUS_INVALID_PARAMETER if param and value are not expected for this paramId.
     * \retval STATUS_NOT_IMPLEMENTED if the requested parameter id is not found.
     * \pre \ref init
     *
     * \see VisionCamParam_e for possible parameters.
     * \see VisionCamPort_e  for available ports.
     */
    status_e setParameter(VisionCamParam_e paramId, void* param, uint32_t size, VisionCamPort_e port=VCAM_PORT_PREVIEW);

    /*! \brief Reads a camera configuration parameter.
     *
     * \param [in]  paramId ID of the parameter that has to be read.
     * \param [out] param   Pointer to user allocated space, where parameter value will be written.
     * \param [in]  size    Size of parameter data.
     * \param [in]  port    Output port for which this parameters are applied.
     * \retval STATUS_SUCCESS on success
     * \retval STATUS_INVALID_PARAMETER if param and value are not expected for this paramId.
     * \retval STATUS_NOT_IMPLEMENTED if the requested parameter id is not found.
     * \pre \ref init
     *
     * \see VisionCamParam_e for possible parameters.
     * \see VisionCamPort_e  for available ports.
     */
    status_e getParameter(VisionCamParam_e paramId, void* param, uint32_t size, VisionCamPort_e port=VCAM_PORT_PREVIEW);

    /*! \brief Returns a frame buffer the user has received from the VisionCam.
     *
     * This must be called after VisionCam client has finished his job with a frame buffer.
     * \param [in] cameraFrame The pointer to the frame given to the user which is now completed.
     * \retval STATUS_SUCCESS on success
     * \pre \ref init
     */
    status_e returnFrame(VisionCamFrame *cameraFrame);

    // implementation of android::CameraListener methods

    /*! \brief Receives notifications from CameraService.
     *
     * This is implementation of android::CameraListener::notify method. With
     * this method the camera notifies the user of errors or events like Zoom,
     * Focus, Shutter, etc.
     * \param [in] msgType  the type of the message received.
     * \param [in] ext1     message specific information.
     * \param [in] ext2     message specific information.
     */
    virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2);

    /*! \brief Receives callback from CameraService when preview frame or image is ready.
     *
     * This is implementation of android::CameraListener::postData method.
     * \param [in] msgType  the type of the message received.
     * \param [in] dataPtr  pointer to the preview frame or image buffer.
     * \param [in] metadata metadata related to the frame or buffer received.
     */
    virtual void postData(int32_t msgType, const sp<IMemory> &dataPtr,
                          camera_frame_metadata_t *metadata);

    /*! \brief Receives callback from CameraService when video frame is ready.
     *
     * This is implementation of android::CameraListener::postDataTimestamp method.
     * \param [in] timestamp time stamp.
     * \param [in] msgType   the type of the message received.
     * \param [in] dataPtr   pointer to the video frame buffer.
     */
    virtual void postDataTimestamp(
            nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr);

private:

    /*! \brief Connects to the selected camera.
     *
     * If the camera is already connected stops any running preview and reconnect.
     * After successfully connected gets the default settings from CameraService.
     * \param [in] cameraId A number indentifying the camera.
     * \retval STATUS_SUCCESS on success
     * \retval STATUS_FAILURE if connection can't be established.
     *
     * \see VisionCamSensorSelection for possible values.
     */
    status_e connect(VisionCamSensorSelection cameraId);

    /*! \brief Signals the camera to start a streaming of frames on a certain output port.
     *
     * \param [in] port the port on which this streaming shold be started.
     * \retval STATUS_SUCCESS on success
     * \retval STATUS_FAILURE if preview can't be start.
     * \retval STATUS_INVALID_STATE if cammera has not been connected
     * \retval STATUS_INVALID_PARAMETER if cannot set the parameters needed for starting preview.
     * \pre \ref connect
     */
    status_e startPreview();

    /*! \brief Signals the camera to stop the streaming of frames on a certain output port.
     *
     * \param [in] port the port on which this streaming shold be stopped.
     * \retval STATUS_SUCCESS on success
     * \retval STATUS_INVALID_STATE if cammera has not been connected
     * \pre \ref connect
     */
    status_e stopPreview();

    /*! \brief Signals the camera to start/stop the returning of face detection metadata.
     *
     * \retval STATUS_SUCCESS on success.
     * \retval STATUS_FAILURE if metadata can't be start.
     * \retval STATUS_INVALID_STATE if cammera has not been connected
     * \pre \ref connect
     */
    status_e toggleFaceDetection();

    /*! \brief Requests camera to lock Auto Whitebalance coeficients.
     *
     * Configures KEY_AUTO_WHITEBALANCE_LOCK camera parameter.
     * \param [in] lock a true value locks autowhitebalance, false will unlock.
     * \retval STATUS_SUCCESS on success.
     * \retval STATUS_FAILURE if the parameter can't be set.
     */
    status_e lockAutoWhitebalance(bool_e lock);

    /*! \brief Requests camera to lock Auto Exposure coeficients.
     *
     * Configures KEY_AUTO_EXPOSURE_LOCK camera parameter.
     * \param [in] lock a true value locks auto exposure, false will unlock.
     * \retval STATUS_SUCCESS on success.
     * \retval STATUS_FAILURE if the parameter can't be set.
     */
    status_e lockAutoExposure(bool_e lock);

    /*! \brief Request camera to start flicker correction.
     *
     * Configures KEY_ANTIBANDING camera parameter.
     * \param [in] type The antibanding value.
     * \retval STATUS_SUCCESS on success.
     * \retval STATUS_FAILURE if the parameter can't be set.
     *
     * \see VisionCamFlickerType for possible parameters.
     */
    status_e setFlicker(VisionCamFlickerType type);

    /*! \brief Sets EV compensation value.
     *
     * Configures KEY_EXPOSURE_COMPENSATION camera parameter.
     * \param [in] type the exposure value measured in ISOs.
     * \retval STATUS_SUCCESS on success.
     * \retval STATUS_FAILURE if the parameter can't be set.
     */
    status_e setExposureCompensation(int32_t compensation);

    /*! \brief Applies VisionCam parameters to CameraService.
     *
     * \retval STATUS_SUCCESS on success
     * \retval STATUS_FAILURE if the parameters can't be set.
     * \retval STATUS_INVALID_PARAMETER if some of the parameters has invalid value.
     */
    status_e setCameraParameters();

    /*! \brief Gets the default camera parameters from CameraService.
     *
     * \retval STATUS_SUCCESS on success.
     */
    status_e initCameraParameters();

    /*! \brief Allocates the frame buffers used to return camera preview frames to the user.
     *
     * \retval STATUS_SUCCESS on success.
     * \retval STATUS_NOT_ENOUGH_MEMORY if no memory could be internally allocated.
     * \post \ref initFrameDescriptors
     */
    status_e initFrameDescriptors();

    /*! \brief Frees the frame buffers used to return camera preview frames to the user.
     *
     * \retval STATUS_SUCCESS on success.
     * \pre \ref initFrameDescriptors
     */
    status_e deinitFrameDescriptors();

    /*! \brief Converts fourcc_t color format to preview format used by CameraService.
     *
     * \param [in] colorSpace the fourcc_t code.
     * \return string representing the format.
     *
     * \see _fourcc for possible parameters.
     */
    const char* colorSpaceToPreviewFormat(_fourcc colorSpace);

    /*! \brief Converts the preview format used by CameraService to fourcc_t color format.
     *
     * \param [in] previewFormat the string representing the preview format.
     * \return fourcc_t.
     *
     * \see _fourcc for possible parameters.
     */
    _fourcc previewFormatToColorSpace(const char* previewFormat);

    /*! \brief Converts VisionCamFlickerType type to antibading type used by CameraService.
     *
     * \param [in] type the VisionCamFlickerType code.
     * \return string representing the antibanding.
     *
     * \see VisionCamFlickerType for possible parameters.
     */
    const char* flickerToAntibanding(VisionCamFlickerType type);

    /*! \brief Converts the antibanding used by CameraService to VisionCamFlickerType type.
     *
     * \param [in] antibanding the string representing the antibanding format.
     * \return VisionCamFlickerType.
     *
     * \see VisionCamFlickerType for possible parameters.
     */
    VisionCamFlickerType antibandingToFlicker(const char* antibanding);

    /*! \brief Converts VisionCamFlickerType type to a user friendly string.
     *
     * \param [in] type the VisionCamFlickerType code.
     * \return string corresponding to VisionCamFlickerType code.
     *
     * \see VisionCamFlickerType   for possible parameters.
     */
    const char* flickerToString(VisionCamFlickerType flicker);

    // Callback called when still camera raw data is available.
    void dataCallback(int32_t msgType, const sp<IMemory> &data, camera_frame_metadata_t *metadata);
    void dataCallbackTimestamp(int64_t timestampUs, int32_t msgType, const sp<IMemory> &data);

private:
    /*! \brief Stores the id of the currently selected camera.
     *
     * \see VisionCamSensorSelection for possible values.
     */
    VisionCamSensorSelection        mCameraId;

    /*! \brief Access point to Android's CameraService. */
    sp<Camera>                      mCamera;

    /*! \brief The SurfaceTexture CameraService uses for preview. */
    sp<SurfaceTexture>              mSurfaceTexture;

    /*! \brief Stores the index of the next available frame in \ref mFrameDescriptors. */
    uint32_t mFrameIndex;

    /*! \brief An array of preallocated frames ready to be returned to the user.
     *
     * \see VisionCamFrame for possible values.
     */
    VisionCamFrame * mFrameDescriptors[MAX_FRAMES];

    /*! \brief Indicate whether CSVisionCam is inited successfully. */
    bool_e   mInited;

    /*! \brief Indicate whether CSVisionCam is connected Android's CameraService. */
    bool_e   mConnected;

    /*! \brief Indicate whether returning face detection metadata is enabled. */
    bool_e   mFaceDetectionEnabled;

    /*! \brief Camera frame width. */
    uint32_t mWidth;

    /*! \brief Camera frame height. */
    uint32_t mHeight;

    /*! \brief Camera contrast parameter. */
    int32_t  mContrast;

    /*! \brief Camera sharpness parameter. */
    int32_t  mSharpness;

    /*! \brief Camera brightness parameter. */
    int32_t  mBrightness;

    /*! \brief Camera saturation parameter. */
    int32_t  mSaturation;

    /*! \brief Camera exposure compensation parameter. */
    int32_t  mEVCompensation;

    /*! \brief Stores the flicker camera parameter.
     *
     * \see VisionCamFlickerType for possible values.
     */
    VisionCamFlickerType mFlicker;

    /*! \brief Stores the color format camera parameter.
     *
     * \see _fourcc for possible values.
     */
    _fourcc mColorSpace;

    /*! \brief Stores the variable frame rate camera parameter.
     *
     * \see VisionCamVarFramerateType for possible values.
     */
    VisionCamVarFramerateType mVfr;
};

#endif // ANDROID

#endif // _CS_VISION_CAM_H_
