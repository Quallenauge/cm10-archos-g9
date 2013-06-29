/*
 *  Copyright (C) 2012 Texas Instruments, Inc.
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

#include "jni.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <imgfilter/imgFilter_armv7.h>
#include <errno.h>
#include <pthread.h>
#include <android/log.h>

#ifndef dimof
#define dimof(x) (sizeof(x)/sizeof(x[0]))
#endif

typedef struct _event_t {
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
	pthread_condattr_t attr;
	int value;
} event_t;

typedef struct _imagefilter_t {
	uint8_t *pSrc;
	uint32_t width;
	uint32_t height;
	uint8_t *pDst;
	int32_t srcStride;
	int32_t dstStride;
	uint8_t k;
	uint32_t index;
	event_t start;
	event_t stop;
} imagefilter_t;

#define NUM_THREADS (2)
static pthread_t threads[NUM_THREADS];
static imagefilter_t data[NUM_THREADS];

#define TAG "IMG_JNI"
#define LOGD(string, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, string, ## __VA_ARGS__)
#define LOGE(string, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, string, ## __VA_ARGS__)
#define LOGI(string, ...) __android_log_print(ANDROID_LOG_INFO,  TAG, string, ## __VA_ARGS__)
#define LOGV(string, ...) __android_log_print(ANDROID_LOG_VERBOSE,  TAG, string, ## __VA_ARGS__)
#define LOG_ASSERT(condition, string) if (!condition) LOGE(string)

void event_init(event_t *e)
{
	int err = 0;
	err |= pthread_mutex_init(&e->mutex, NULL);
	err |= pthread_condattr_init(&e->attr);
	err |= pthread_cond_init(&e->cond, &e->attr);
}

void event_deinit(event_t *e)
{
	int err = 0;
	do {
		err = pthread_cond_destroy(&e->cond);
		if (err == EBUSY) {
			pthread_mutex_lock(&e->mutex);
			e->value = 1;
			pthread_cond_broadcast(&e->cond);
			pthread_mutex_unlock(&e->mutex);
		}
		e->value = 0;
	} while (err != 0);
	pthread_condattr_destroy(&e->attr);
	pthread_mutex_destroy(&e->mutex);
}

void event_set(event_t *e)
{
	pthread_mutex_lock(&e->mutex);
	e->value = 1;
	pthread_cond_broadcast(&e->cond);
	pthread_mutex_unlock(&e->mutex);
}

int event_wait(event_t *e)
{
	int retcode;
	pthread_mutex_lock(&e->mutex);
	if (e->value == 0) // unsignalled
	{
		retcode = pthread_cond_wait(&e->cond, &e->mutex);
		//LOGD("e=%p ret=%d, value=%d\n", e, retcode, e->value);
		if (retcode == 0 && e->value == 1)
			retcode = 1;
		else
			retcode = 0;
	}
	else
		retcode = 1;
	e->value = 0; // reset it.
	pthread_mutex_unlock(&e->mutex);
	return retcode;
}

void *thread_filter(void *arg)
{
	imagefilter_t *img = (imagefilter_t *)arg;
	int32_t retcode;
	int32_t run = 0;
	while (1)
	{
		// wait for the signal
		//LOGD("[%u] waiting for signal\n", img->index);
		if (event_wait(&img->start))
		{
			// we're signalled
			//LOGD("[%u] signal!\n",img->index);
			// exit condition
			if (img->pSrc == NULL || img->pDst == NULL)
				break;
			//LOGD("[%u] %p {%ux%u} to %p\n", img->index, img->pSrc, img->width, img->height, img->pDst);
			__3chan_tap_filter_image_3x1_fixed_k(img->pSrc,
												 img->width,
												 img->height,
												 img->pDst,
												 img->srcStride,
												 img->dstStride,
												 img->k);
			//LOGD("[%u] work done!\n", img->index);
			event_set(&img->stop);
		}
	}
	LOGD("[%u] exiting\n", img->index);
	return;
}

void Java_com_ti_dvp_ImageFilter_init(JNIEnv *env, jclass thiz)
{
	uint32_t t;
	int32_t err;
	for (t = 0; t < dimof(threads); t++)
	{
		data[t].index = t;
		event_init(&data[t].start);
		event_init(&data[t].stop);
		err |= pthread_create(&threads[t], NULL, thread_filter, &data[t]);
	}
}

void Java_com_ti_dvp_ImageFilter_deinit(JNIEnv *env, jclass thiz)
{
	uint32_t t;
	int32_t err;
	void *ret;
	for (t = 0; t < dimof(threads); t++)
	{
		LOGD("[%u] killing!\n",t);
		data[t].pSrc = NULL;
		event_set(&data[t].start);
		LOGD("[%u] joining\n", t);
		err = pthread_join(threads[t], &ret);
		LOGD("[%u] joined %d\n", t, err);
		event_deinit(&data[t].stop);
		event_deinit(&data[t].start);
	}
	LOGD("ImageFilter deinit!\n");
}

void Java_com_ti_dvp_ImageFilter_filter3x1(JNIEnv *env,
										   jclass thiz,
										   jbyteArray in,
										   jint width,
										   jint height,
										   jbyteArray out,
										   jint k)
{
	uint32_t srcStride = width * sizeof(uint32_t);
	uint32_t dstStride = width * sizeof(uint32_t);
	uint8_t *input = (uint8_t *)(*env)->GetByteArrayElements(env, in, 0);
	uint8_t *output = (uint8_t *)(*env)->GetByteArrayElements(env, out, 0);
	uint32_t t = 0;
	int32_t retcode;
	struct timespec tm1, tm2;
	uint64_t start_time, end_time;

	clock_gettime(CLOCK_MONOTONIC, &tm1);

	// initialize the data splits
	for (t = 0; t < dimof(threads); t++)
	{
		data[t].pSrc = &input[t * (height/dimof(threads)) * width * sizeof(uint32_t)];
		data[t].width = width;
		data[t].height = height/dimof(threads);
		data[t].pDst = &output[t * (height/dimof(threads)) * width * sizeof(uint32_t)];
		data[t].srcStride = width * sizeof(uint32_t);
		data[t].dstStride = width * sizeof(uint32_t);
		data[t].k = k;
	}

	for (t = 0; t < dimof(threads); t++)
		event_set(&data[t].start);

	// work happens here

	for (t = 0; t < dimof(threads); t++) {
		if (event_wait(&data[t].stop))	{
			//LOGD("[%u] received work\n", t);
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &tm2);

	start_time = 1000000000*tm1.tv_sec + tm1.tv_nsec;
	end_time = 1000000000*tm2.tv_sec + tm2.tv_nsec;

	// we're all done..
	LOGD("filter3x1 done! %llu usec\n", (end_time-start_time)/1000);

	(*env)->ReleaseByteArrayElements(env, in, (jbyte *)input, 0);
	(*env)->ReleaseByteArrayElements(env, out, (jbyte *)output, 0);
}
