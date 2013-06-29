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

/*! \file
 * \brief This is a simple, dumbed down version of the vcam_test which only uses
 * command line paramters and simple settings in order to test out system
 * connectivity, this is *not* intended to replace the more complicated
 * VisionCamTest.cpp.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <dvp/VisionCam.h>
#include <dvp/dvp.h>
#include <dvp/dvp_display.h>
#include <sosal/sosal.h>
#include <dvp/ImageDebugging.h>
#include <SocketVisionCam.h>

#define CAMERA_NAME MODULE_NAME("vcam")
int32_t width;
int32_t height;
int32_t allocWidth;
int32_t allocHeight;
int32_t fps;
uint32_t color;
char fourcc_str[5];
int32_t type;
int32_t numImages;
int32_t recvFrames;
int32_t numFrames;
int32_t sensor;
int32_t frameLock;
int32_t focusDepth;
int32_t mode;
int32_t camera_rotation;
uint32_t screen_rotation;
uint32_t dw,sw;
uint32_t dh,sh;
int32_t subsample;
bool_e topBottom;
char filename[MAX_PATH];
char name[MAX_PATH];
uint32_t numTimeouts;
profiler_t capPerf;
uint32_t white;
uint32_t brightness;
uint32_t iso;
uint32_t exposure;
bool_e manual;
uint32_t repeats;
option_t opts[] = {
    {OPTION_TYPE_INT, &width, sizeof(width),              "-w",  "--width", "Width of Image"},
    {OPTION_TYPE_INT, &height, sizeof(height),            "-h",  "--height", "Height of Image"},
    {OPTION_TYPE_INT, &dw, sizeof(dw),                    "-dw", "--dispwidth", "Width of Display"},
    {OPTION_TYPE_INT, &dh, sizeof(dh),                    "-dh", "--dispheight", "Height of Display"},
    {OPTION_TYPE_INT, &sw, sizeof(sw),                    "-sw", "--scaleWidth", "Scaled Width of Display"},
    {OPTION_TYPE_INT, &sh, sizeof(sh),                    "-sh", "--scaleHeight", "Scaled Height of Display"},
    {OPTION_TYPE_INT, &fps, sizeof(fps),                  "-f",  "--fps", "Frame Rate"},
    {OPTION_TYPE_BOOL, &topBottom, sizeof(topBottom),     "-tb", "--topbottom", "Orient the stereo image as top/bottom"},
    {OPTION_TYPE_STRING, &fourcc_str, sizeof(fourcc_str), "-c",  "--fourcc", "FOURCC Code as string (UYVY) "},
    {OPTION_TYPE_INT, &type, sizeof(type),                "-t",  "--type", "Determines the type of GestureCam instantiated"},
    {OPTION_TYPE_STRING, name, sizeof(name),              "-n",  "--name", "Name of file to read"},
    {OPTION_TYPE_INT, &numImages, sizeof(numImages),      "-i",  "--images", "Number of images to use"},
    {OPTION_TYPE_INT, &numFrames, sizeof(numFrames),      "-#",  "--frames", "Number of frames to process"},
    {OPTION_TYPE_INT, &sensor, sizeof(sensor),            "-s",  "--sensor", "Selects the sensor (0,1,2)"},
    {OPTION_TYPE_INT, &frameLock, sizeof(frameLock),      "-l",  "--lock-after", "Locks AE/AWB after specified frame count"},
    {OPTION_TYPE_INT, &focusDepth, sizeof(focusDepth),    "-fd", "--focus-depth", "Specific Focus Depth [0-150]"},
    {OPTION_TYPE_INT, &mode, sizeof(mode),                "-p",  "--mode", "Capture Mode"},
    {OPTION_TYPE_INT, &camera_rotation, sizeof(camera_rotation),"-cr", "--camera_rotation", "Rotates the captured image in the camera"},
    {OPTION_TYPE_INT, &screen_rotation, sizeof(screen_rotation),"-sr", "--screen_rotation", "Rotates the display image"},
    {OPTION_TYPE_INT, &subsample, sizeof(subsample),      "-sb", "--subsample", "Subsampled ratio, defaults to 1"},
    {OPTION_TYPE_STRING, &filename, sizeof(filename),     "-o",  "--out", "File to write captured frames to"},
    {OPTION_TYPE_BOOL, &manual, sizeof(manual),           "-m",  "--manual", "Use manual settings"},
    {OPTION_TYPE_INT, &white, sizeof(white),              "-wb", "--white", "White Balance Mode"},
    {OPTION_TYPE_INT, &brightness, sizeof(brightness),    "-br", "--bright", "Brightness Value"},
    {OPTION_TYPE_INT, &iso, sizeof(iso),                  "-is", "--iso", "ISO Value"},
    {OPTION_TYPE_INT, &exposure, sizeof(exposure),        "-ex", "--exposure", "Manual Exposure Value"},
    {OPTION_TYPE_INT, &numTimeouts, sizeof(numTimeouts),  "-to", "--timeouts", "Set the number of frame timeout which can occur before the camera halts"},
    {OPTION_TYPE_INT, &repeats, sizeof(repeats),          "-r",  "--repeat", "Sets the number of repeat iterations, default is 1."},
};
void VisionCamTestCallback(VisionCamFrame * cameraFrame)
{
    DVP_Image_t *pImage = (DVP_Image_t *)cameraFrame->mFrameBuff;
    queue_t *frameq = (queue_t *)cameraFrame->mCookie;

    DVP_PRINT(DVP_ZONE_CAM, "Frame has an offset of %ux%u\n", cameraFrame->mOffsetX, cameraFrame->mOffsetY);
    profiler_stop(&capPerf);
    DVP_PRINT(DVP_ZONE_PERF, "Last Capture for Frame %u took %lf sec Average: %lf\n", recvFrames, rtimer_to_sec(capPerf.tmpTime), rtimer_to_sec(capPerf.avgTime));
    profiler_start(&capPerf);
    DVP_PRINT(DVP_ZONE_CAM, "Writing Frame %p into Queue %p\n", pImage, frameq);
    DVP_PrintImage(DVP_ZONE_CAM, pImage);
    if (queue_write(frameq, true_e, &cameraFrame) == false_e)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Failed to write frame to queue\n");
    }
}

int main(int argc, char *argv[])
{
    uint32_t r = 0;
    int32_t i = 0;
    VisionCamSensorSelection sensorIndex = VCAM_SENSOR_SECONDARY;
    VisionCamCaptureMode capmode = VCAM_VIDEO_NORMAL;
    VisionCamFlickerType flicker = FLICKER_60Hz;
    VisionCamFocusMode focus = VCAM_FOCUS_CONTROL_AUTO;
    VisionCamStereoInfo info;
    VisionCamResType res;
    ImageDebug_t imgdbg;

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

    // default values
    white = VCAM_WHITE_BAL_CONTROL_AUTO;
    brightness = 50; // [0-200]
    iso = 100;       // [100-1600]
    exposure = 50;   // [0-100]
    manual = false_e;
    recvFrames = 0;
    width = 320;
    height = 240;
    fps = 30;
    color = FOURCC_NV12;
    strcpy(fourcc_str, "NV12");
    strcpy(name, "car");
    numImages = DVP_DISPLAY_NUM_BUFFERS;
    sensor = 1;
    numFrames = 100;
    frameLock = 0xFFFFFFFF;
    focusDepth = -1;
    mode = VCAM_VIDEO_NORMAL;
    camera_rotation = 0;
    screen_rotation = 0;
    dw = DVP_DISPLAY_WIDTH;
    dh = DVP_DISPLAY_HEIGHT;
    sw = 0;
    sh = 0;
    type = VISIONCAM_OMX;
    topBottom = true_e;
    subsample = 1;
    memset(filename, 0, sizeof(filename));
    numTimeouts = 10;
    repeats = 1;

    profiler_clear(&capPerf);

    option_process(argc, argv, opts, dimof(opts));

    // check for bad input
    if (width <= 0) width = 160;
    if (height <= 0) height = 120;
    if (dw <= 0) dw = DVP_DISPLAY_WIDTH;
    if (dh <= 0) dh = DVP_DISPLAY_HEIGHT;
    if (sw <= 0) sw = width;
    if (sh <= 0) sh = height;
    if (numImages <= 2) numImages = 2;
    if (fps <= 15) fps = 15;
    if (numFrames <= 100) numFrames = 100;
    if (frameLock > numFrames) frameLock = -1;
    if (focusDepth > 150) focusDepth = 75;
    if (mode >= VCAM_CAP_MODE_MAX) mode = VCAM_GESTURE_MODE;
    if (sensor > 2) sensor = 2;
    if (camera_rotation != 0 && camera_rotation != 90 && camera_rotation != 180 && camera_rotation != 270)
        camera_rotation = 0;
    else if (camera_rotation == 90 || camera_rotation == 270)
    {
        uint32_t t = sw;
        sw = sh;
        sh = t;
    }
    if (type >= VISIONCAM_MAX || type < 0)
        type = VISIONCAM_OMX;
    if (subsample <= 0 || subsample > 4)
        subsample = 1;
    if (brightness > 200)
        brightness = 200;
    if (iso < 100) iso = 100;
    if (iso > 1600) iso = 1600;
    if (exposure > 100) exposure = 100;
    if (repeats == 0)
        repeats = 1;

    color = strtofourcc(fourcc_str);

    DVP_PRINT(DVP_ZONE_ALWAYS, "Requested Color %08x\n", color);

    DVP_PRINT(DVP_ZONE_ALWAYS, "Type %d requested!\n", type);

    // different types of cameras have different restrictions...
    if (type == VISIONCAM_SOCKET)
    {
        // don't let the network latency kill the connection.
        numTimeouts = 0xFFFFFFFF;
        // if stereo mode, modify the variables
        if (mode == VCAM_STEREO_MODE || sensorIndex == VCAM_SENSOR_STEREO)
        {
            DVP_PRINT(DVP_ZONE_ALWAYS, "Enabling Stereo Use Case!\n");
            sensorIndex = VCAM_SENSOR_STEREO;
            mode = VCAM_STEREO_MODE;
            memset(&info, 0, sizeof(info));
            if (topBottom)
                info.layout = VCAM_STEREO_LAYOUT_TOPBOTTOM;
            else
                info.layout = VCAM_STEREO_LAYOUT_LEFTRIGHT;
            info.subsampling = subsample;
            color = FOURCC_NV12; // only NV12 is supported during stereo capture.
        }
    }
    else if (type == VISIONCAM_USB)
    {
        // USB may take a few hundred ms to initialize, don't let it exit early
        if (numTimeouts < 100)
            numTimeouts = 100;
    }
    else if (type == VISIONCAM_OMX) // this only really applies to local OMX cameras, remote cameras or USB will not mess with this.
    {
        switch (sensor)
        {
#ifndef SDP
            case 2:
                DVP_PRINT(DVP_ZONE_ALWAYS, "Enabling Stereo Use Case!\n");
                sensorIndex = VCAM_SENSOR_STEREO;
                mode = VCAM_STEREO_MODE;
                memset(&info, 0, sizeof(info));
                if (topBottom)
                    info.layout = VCAM_STEREO_LAYOUT_TOPBOTTOM;
                else
                    info.layout = VCAM_STEREO_LAYOUT_LEFTRIGHT;
                info.subsampling = subsample;
                color = FOURCC_NV12; // only NV12 is supported during stereo capture.
                break;
            case 1:
                DVP_PRINT(DVP_ZONE_ALWAYS, "Enabling Vision Mode on Front Camera!\n");
#ifdef PLAYBOOK
                sensorIndex = VCAM_SENSOR_PRIMARY;
#else
                sensorIndex = VCAM_SENSOR_SECONDARY;
#endif
#if defined(DUCATI_1_5) || defined(DUCATI_2_0)
                mode = VCAM_GESTURE_MODE;
#else
# if defined(__QNX__)
                mode = VCAM_VIEWFINDER_MODE;
# else
                mode = VCAM_VIDEO_NORMAL;
# endif
#endif
                break;
            case 0:
#endif
            default: sensorIndex = VCAM_SENSOR_PRIMARY;  break;
        }
    }
    capmode = (VisionCamCaptureMode)mode;
    allocWidth = width;
    allocHeight = height;

    // using 1 to <= so prints will make sense
    for (r = 1; r <= repeats; r++)
    {
        DVP_PRINT(DVP_ZONE_ALWAYS, "Iteration %u of %u\n", r, repeats);
        recvFrames = 0;
#ifdef VCAM_AS_SHARED
        module_t mod = module_load(CAMERA_NAME);
        if (mod)
        {
            VisionCamFactory_f factory = (VisionCamFactory_f)module_symbol(mod, "VisionCamFactory");
            if (factory)
            {
                VisionCam *pCam = factory((VisionCam_e)type);
                if (pCam)
                {
#else
        VisionCam *pCam = VisionCamFactory((VisionCam_e)type);
        if (pCam)
        {
#endif
                    queue_t *frameq = queue_create(numImages * VCAM_PORT_MAX, sizeof(VisionCamFrame *));
                    DVP_Image_t *images = (DVP_Image_t *)calloc(numImages, sizeof(DVP_Image_t));

                    if (images)
                    {
                        VisionCamFrame *cameraFrame = NULL;
                        DVP_Image_t *pImage = NULL;
                        status_e greError = STATUS_SUCCESS;
#if defined(VCAM_SUPPORTS_SOCKETCAM)
                        // set the hostname before we connect.
                        if (type == VISIONCAM_SOCKET)
                            pCam->setParameter((VisionCamParam_e)SOCKETVCAM_PARAM_HOSTNAME, name, sizeof(name));
#endif
                        VCAM_RETURN_IF_FAILED(greError, pCam->init(frameq));
                        VCAM_COMPLAIN_IF_FAILED(greError, pCam->setParameter(VCAM_PARAM_WIDTH, &width, sizeof(width)));
                        VCAM_COMPLAIN_IF_FAILED(greError, pCam->setParameter(VCAM_PARAM_HEIGHT, &height, sizeof(height)));
                        VCAM_COMPLAIN_IF_FAILED(greError, pCam->setParameter(VCAM_PARAM_COLOR_SPACE_FOURCC, &color, sizeof(color)));
                        // Can't set ROTATION here, see below
                        VCAM_COMPLAIN_IF_FAILED(greError, pCam->setParameter(VCAM_PARAM_FPS_FIXED, &fps, sizeof(fps)));
                        VCAM_COMPLAIN_IF_FAILED(greError, pCam->setParameter(VCAM_PARAM_CAP_MODE, &capmode, sizeof(capmode)));
                        VCAM_COMPLAIN_IF_FAILED(greError, pCam->setParameter(VCAM_PARAM_SENSOR_SELECT, &sensorIndex, sizeof(sensorIndex)));
                        if (capmode == VCAM_STEREO_MODE)
                        {
                            VCAM_COMPLAIN_IF_FAILED(greError, pCam->setParameter(VCAM_PARAM_STEREO_INFO, &info, sizeof(info)));
                        }

                        memset(&res, 0, sizeof(res));
                        // VCAM_PARAM_2DBUFFER_DIM should only be called after resolutions, color space, cap mode, and
                        //   optionally stereo information is set.
                        VCAM_COMPLAIN_IF_FAILED(greError, pCam->getParameter(VCAM_PARAM_2DBUFFER_DIM, &res, sizeof(res)));
                        // if the camera requires a bigger buffer than the requrested resolution ...
                        if (res.mHeight != (uint32_t)height || res.mWidth != (uint32_t)width)
                        {
                            allocWidth = res.mWidth;
                            allocHeight = res.mHeight;
                        }

                        dvp_display_t *dvpd = DVP_Display_Create(width, height, allocWidth, allocHeight, dw, dh, allocWidth, allocHeight, 0, 0, color, screen_rotation, numImages);
                        if(dvpd)
                        {
                            VCAM_COMPLAIN_IF_FAILED(greError, pCam->setParameter(VCAM_PARAM_FLICKER, &flicker, sizeof(flicker)));
                            VCAM_COMPLAIN_IF_FAILED(greError, pCam->setParameter(VCAM_PARAM_AWB_MODE, &white, sizeof(white)));
                            if (manual)
                            {
                                VCAM_COMPLAIN_IF_FAILED(greError, pCam->setParameter(VCAM_PARAM_BRIGHTNESS, &brightness, sizeof(brightness)));
                                VCAM_COMPLAIN_IF_FAILED(greError, pCam->setParameter(VCAM_PARAM_EXPOSURE_ISO, &iso, sizeof(iso)));
                                VCAM_COMPLAIN_IF_FAILED(greError, pCam->setParameter(VCAM_PARAM_MANUAL_EXPOSURE, &exposure, sizeof(exposure)));
                            }
                            VCAM_COMPLAIN_IF_FAILED(greError, pCam->setParameter(VCAM_PARAM_NAME, name, sizeof(name)));

                            for (i = 0; i < numImages; i++)
                            {
                                DVP_Image_Init(&images[i], allocWidth, allocHeight, color);
                                // The image dimentions may be smaller than the buffer dimentions
                                images[i].width = width;
                                images[i].height = height;
                                DVP_Display_Alloc(dvpd, &images[i]);
                            }
                            // tell the camera to use all the camera index buffers
                            VCAM_COMPLAIN_IF_FAILED(greError,  pCam->useBuffers(images, numImages));

                            /** @todo Since the OMX Camera is probably being used by
                                the VCAM_SIMPLE test, we have to understand that
                                the OMX-CAMERA has a bug in the camera_rotation when used
                                during LOADED state. We have to wait until IDLE
                                (post useBuffers) or EXECUTING (post PREVIEW)
                                before rotating. */
                            /** @todo Additionally, OMX-CAMERA STEREO mode can't handle the rotation values! */
                            if (capmode != VCAM_STEREO_MODE)
                                VCAM_COMPLAIN_IF_FAILED(greError, pCam->setParameter(VCAM_PARAM_ROTATION, &camera_rotation, sizeof(camera_rotation)));

                            // register the engine callback with the camera
                            VCAM_COMPLAIN_IF_FAILED(greError,  pCam->enablePreviewCbk(VisionCamTestCallback));
                            profiler_start(&capPerf);
                            VCAM_COMPLAIN_IF_FAILED(greError,  pCam->sendCommand(VCAM_CMD_PREVIEW_START));

                            if (focusDepth == -1) { // begin auto-focus
                                VCAM_COMPLAIN_IF_FAILED(greError,  pCam->setParameter(VCAM_PARAM_DO_AUTOFOCUS, &focus, sizeof(focus)));
                            }

                            if (filename[0] != '\0') {
                                DVP_Image_t local;
                                local = images[0];
                                ImageDebug_Init(&imgdbg, &local, "raw"PATH_DELIM, filename);
                                ImageDebug_Open(&imgdbg, 1);
                            }

                            thread_msleep(1000/fps); // wait 1 frame period.
                            if (greError == STATUS_SUCCESS)
                            {
                                uint32_t timeouts = 0;
                                DVP_PRINT(DVP_ZONE_CAM, "VisionCam is initialized, entering queue read loop!\n");
                                // read from the queue and display the images
                                do {
                                    bool_e ret = queue_read(frameq, false_e, &cameraFrame);
                                    if (ret == true_e && cameraFrame != NULL)
                                    {
                                        pImage = (DVP_Image_t *)cameraFrame->mFrameBuff;
                                        timeouts = 0;
                                        DVP_Display_Render(dvpd, pImage);
                                        if (filename[0] != '\0') {
                                            DVP_Image_t copy = *pImage;
                                            imgdbg.pImg = &copy;
                                            ImageDebug_Write(&imgdbg, 1);
                                        }
                                        pCam->returnFrame(cameraFrame);
                                        recvFrames++;
                                        if (recvFrames > numFrames)
                                            break;
                                        if (focusDepth >= 0) {
                                            if (recvFrames == fps) { // after 1 second
                                                VCAM_COMPLAIN_IF_FAILED(greError, pCam->setParameter(VCAM_PARAM_DO_MANUALFOCUS, &focusDepth, sizeof(focusDepth)));
                                            }
                                        }
                                        if (frameLock > 0) {
                                            if (recvFrames == frameLock) {
                                                bool_e lock = true_e;
                                                VCAM_COMPLAIN_IF_FAILED(greError, pCam->sendCommand(VCAM_CMD_LOCK_AE, &lock, sizeof(lock)));
                                                VCAM_COMPLAIN_IF_FAILED(greError, pCam->sendCommand(VCAM_CMD_LOCK_AWB, &lock, sizeof(lock)));
                                            }
                                        }
                                    }
                                    else
                                    {
                                        DVP_PRINT(DVP_ZONE_ERROR, "Timedout waiting for buffer from Camera!\n");
                                        timeouts++;
                                        thread_msleep(1000/fps);
                                    }
                                } while (timeouts < numTimeouts);
                            }
                            else
                            {
                                DVP_PRINT(DVP_ZONE_ERROR, "VCAM_TEST Failed during initialization (greError = %d, 0x%08x)!\n", greError, greError);
                            }

                            if (filename[0] != '\0')
                                ImageDebug_Close(&imgdbg, 1);

                            // destroy the camera
                            VCAM_COMPLAIN_IF_FAILED(greError, pCam->sendCommand(VCAM_CMD_PREVIEW_STOP));
                            VCAM_COMPLAIN_IF_FAILED(greError, pCam->disablePreviewCbk(VisionCamTestCallback));
                            VCAM_COMPLAIN_IF_FAILED(greError, pCam->releaseBuffers());
                            VCAM_COMPLAIN_IF_FAILED(greError, pCam->deinit());

                            // free the images
                            for (i = 0; i < numImages; i++)
                                DVP_Display_Free(dvpd, &images[i]);

                            DVP_Display_Destroy(&dvpd);
                        }
                        free(images);
                    }

                    delete pCam;
                    pCam = NULL;

                    queue_destroy(frameq);
                    frameq = NULL;
#ifdef VCAM_AS_SHARED
                }
            }
            module_unload(mod);
        }
        else
        {
            DVP_PRINT(DVP_ZONE_ERROR, "Could not find %s\n", CAMERA_NAME);
        }
#else
        }
#endif
}
    return 0;
}
