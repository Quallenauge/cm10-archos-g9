/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android/log.h>
#define LOG_TAG "LiveCam"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "libnav", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "libnav", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "libnav", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "libnav", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "libnav", __VA_ARGS__)

//#include <GLES2/gl2.h>
//#include <GLES2/gl2ext.h>
#include <jni.h>
//#include <android/surface_texture_data_jni.h>
//#include <android/camera_buffer.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <android/bitmap.h>
#include <android/camera_buffer_context.h>
#include "jni.h"
#include "JNIHelp.h"


extern "C"
{

void com_ti_omap4_android_camera_CPCam_processPicture(JNIEnv *env,
	      jobject thiz, jobject jSurfaceTexture, jobject jbitmap, jint processing)
{
	uint8_t *inBuffer = NULL;
	jbyte* outBuffer = NULL;
	//unsigned char* pixels = NULL;
	uint8_t* pixels=NULL;
	unsigned char val = 0;
	unsigned char min=0xFF, max=0x00;
	int width, height, stride;
	uint32_t cdf_min=0xFF, cdf_max=0x00;
	uint32_t pixel_count = 0;
	uint32_t bigval = 0;
	uint32_t cdf_v = 0;
	float num=0.0f, denom=1.0f, fval=0.0f;
	CameraBufferContext* context = NULL;

	LOGI("processPicture: starting");

	// Obtain CameraBufferContext associated to the SurfaceTexture
	context = CameraBufferContext_new(env, jSurfaceTexture);
	if (context == NULL) {
		LOGE("processPicture: no context obtained, leaving");
		goto exit;
	}

	// Acquire buffer of pixels associated to this SurfaceTexture
	CameraBufferContext_getBuffer(context, (void**) &inBuffer);
	if (inBuffer == NULL) {
		LOGE("processPicture: NULL buffer, exiting function");
		goto exit;
	}

	width = CameraBufferContext_getBufferWidth(context);
	height = CameraBufferContext_getBufferHeight(context);
	stride = CameraBufferContext_getBufferStride(context);

	LOGI("processPicture: inbuffer: %d", inBuffer);

	// Get output buffer pointer
	AndroidBitmap_lockPixels(env, jbitmap, (void**) &pixels);
	if (pixels == NULL) {
	  LOGE("Error locking bitmap buffer");
	  goto exit;
	}
	LOGI("processPicture: obtained output buffer pointer");

	/* TODO: insert your processing here */
	if (processing == 0) {
		/* very basic "processing" */
		pixel_count = width*height;
		for (int i=0; i<pixel_count; i++) {
			  val = inBuffer[i];
			  pixels[4*i] = val;
			  pixels[4*i+1] = val;
			  pixels[4*i+2] = val;
			  pixels[4*i+3] = (unsigned char) 0xFF;
		}
	}
	else if (processing == 1) {
		/* Unoptimized histogram equalization
		*
		* Fill up outBuffer with the processed image data in ARGB_8888 format
		* The sample algorithm here is an unoptimized histogram equalization
		* Output is "converted" into RGB by simply copying the Y value to R, G, and B
		* to get a grey value bitmap
		*/
		// Calculate histogram
		int i;
		uint32_t histogram[256];
		uint32_t cumul_histogram[256];
		for (i=0; i<256; i++) histogram[i] = 0;	// initialize to zeros
		for (i=0; i<256; i++) cumul_histogram[i] = 0;

		pixel_count = width*height;
		for (i=0; i<pixel_count; i++)
		{
			val = inBuffer[i];
			histogram[val] += 1;

			// also find out image's min and max at the same pass
			if ( (unsigned char) val < min) min = val;
			if ( (unsigned char) val > max) max = val;
		}

		// Calculate cumulative histogram
		cumul_histogram[0] = histogram[0];
		for (i=1; i<256; i++) {
			cumul_histogram[i] = cumul_histogram[i-1] + histogram[i];
		}

		for (i=0; i<256; i++) {	// 256 = bin count
			// find out histogram's min and max
			bigval = cumul_histogram[i];
			if (bigval > 0) {
				cdf_min = bigval;
				break;
			}
		}

		// remap image to output bitmap buffer
		for (int i=0; i<pixel_count; i++) {
			  val = inBuffer[i];
			  cdf_v = cumul_histogram[val];
			  num = (float) cdf_v - (float) cdf_min;
			  denom = (float)pixel_count - (float) cdf_min;
			  fval = (num / denom) * (255.0f);
			  val = (unsigned char) fval;
			  pixels[4*i] = val;
			  pixels[4*i+1] = val;
			  pixels[4*i+2] = val;
			  pixels[4*i+3] = (unsigned char) 0xFF;
		}
	}
	LOGI("processPicture: processing complete");
	/* End of processing */

	/* Unlock bitmap */
	AndroidBitmap_unlockPixels(env, jbitmap);

exit:
	if (context != NULL){
		LOGI("processPicture: releasing buffer");
		if (inBuffer      != NULL) CameraBufferContext_releaseBuffer(context);
		LOGI("processPicture: deleting context");
		CameraBufferContext_delete(context);
	}

	outBuffer = NULL;
	LOGI("processPicture: exiting function");
	}
static JNINativeMethod methods[] = {
{"processPicture", "(Landroid/graphics/SurfaceTexture;Landroid/graphics/Bitmap;I)V", (void*)com_ti_omap4_android_camera_CPCam_processPicture}
};

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed\n");
        goto bail;
    }

    jniRegisterNativeMethods(env, "com/ti/omap4/android/camera/CPCam",
        methods, NELEM(methods));

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

bail:
    return result;
}
};
