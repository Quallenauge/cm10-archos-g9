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

#ifndef _V4L2_API_H_
#define _V4L2_API_H_

#include <sosal/sosal.h>

#if defined(ANDROID) || defined(LINUX)
#include <linux/videodev2.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

// must also set:
// $ echo "2lcd" > /sys/devices/platform/omapdss/overlay2/manager
// $ echo "3" > /sys/devices/platform/omapdss/overlay2/zorder
// $ echo "2" > /sys/devices/platform/omapdss/overlay1/zorder
// $ echo "1" > /sys/devices/platform/omapdss/overlay0/zorder
#if defined(ANDROID) && (defined(BLAZE)  || defined(SDP))
#define V4L2_DISPLAY_DEVICE 1 // /dev/video1 == vid2 == overlay2 (bug in 2.6.32/35?)
#define V4L2_DISPLAY_COLOR  FOURCC_UYVY
#elif defined(LINUX) && (defined(BLAZE)  || defined(SDP))
#define V4L2_DISPLAY_DEVICE 2 // /dev/video2 == overlay2
#define V4L2_DISPLAY_COLOR  FOURCC_UYVY
#else
#define V4L2_DISPLAY_DEVICE 1 // /dev/video1 == vid2 == overlay2
#define V4L2_DISPLAY_COLOR  FOURCC_UYVY
#endif
#define V4L2_INVALID_INDEX  (0xFFFFFFFF)

typedef struct _v4l2_api_t
{
    int device;
    bool_e nonblocking;
    struct v4l2_format format;
    struct v4l2_requestbuffers reqbuf;
    struct v4l2_capability capability;
    struct v4l2_buffer *bufDesc;
    void **buffers;
    uint32_t count;
    bool_e streaming;/**< Indicates if streaming is on currently */
    bitfield_t used;
    uint32_t active; /**< Number of Active Buffers */
    mutex_t m_lock;
} v4l2_api_t;

#ifdef __cplusplus
extern "C" {
#endif

v4l2_api_t *v4l2_open(int deviceNum, int flags, bool_e nonblocking);
void v4l2_close(v4l2_api_t **v);
int v4l2_wait(v4l2_api_t *v);
bool_e v4l2_queue(v4l2_api_t *v, uint32_t index, uint32_t type);
bool_e v4l2_dequeue(v4l2_api_t *v, uint32_t *index, uint32_t type);
bool_e v4l2_start(v4l2_api_t *v, uint32_t type);
bool_e v4l2_stop(v4l2_api_t *v, uint32_t type);
bool_e v4l2_control_get(v4l2_api_t *v, int control, int *value);
bool_e v4l2_control_set(v4l2_api_t *v, int control, int value);
bool_e v4l2_crop(v4l2_api_t *v, uint32_t left, uint32_t top, uint32_t width, uint32_t height);
bool_e v4l2_free(v4l2_api_t *v);
bool_e v4l2_allocate(v4l2_api_t *v, uint32_t width, uint32_t height, uint32_t count, fourcc_t color, int fmtType);
void *v4l2_acquire(v4l2_api_t *v, uint32_t *index);
bool_e v4l2_release(v4l2_api_t *v, void *buffer, uint32_t index);
void v4l2_query_print(v4l2_api_t *v);
uint32_t v4l2_length(v4l2_api_t *v, uint32_t index);
uint32_t v4l2_search_buffer(v4l2_api_t *v, void *buffer);
void *v4l2_search_index(v4l2_api_t *v, uint32_t index);
bool_e v4l2_rotate(v4l2_api_t *v, int32_t angle);
bool_e v4l2_mirror(v4l2_api_t *v, bool_e mirror);
bool_e v4l2_position(v4l2_api_t *v, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
bool_e v4l2_transparency(v4l2_api_t *v, uint32_t flags, uint32_t colorkey);
bool_e v4l2_metrics(v4l2_api_t *v, uint32_t *width, uint32_t *height);
void v4l2_query_formats(v4l2_api_t *v, uint32_t type);

#ifdef __cplusplus
}
#endif

#endif

#endif

