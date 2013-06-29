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

#ifndef _QNXSCREEN_API_H_
#define _QNXSCREEN_API_H_

#include <sosal/sosal.h>

#if defined(__QNX__)
#include <screen/screen.h>
#include <sys/types.h>

#define QNXSCREEN_INVALID_INDEX  (0xFFFFFFFF)
#define QNXSCREEN_MAX_INDEX	32

struct qnxscreen_img_metrics {
	int left;
	int top;
	int width;
	int height;
};

struct qnxscreen_api {
	screen_window_t win;
	screen_context_t ctx;
	screen_buffer_t buf[QNXSCREEN_MAX_INDEX];
	struct qnxscreen_img_metrics metrics;
	int pfmt;
	uint32_t count;
	bitfield_t used;
	mutex_t m_lock;
};

#ifdef __cplusplus
extern "C" {
#endif

struct qnxscreen_api *qnxscreen_open(void);
void qnxscreen_close(struct qnxscreen_api **q);
bool_e qnxscreen_post(struct qnxscreen_api *q, uint32_t index);
bool_e qnxscreen_free(struct qnxscreen_api *q);
bool_e qnxscreen_allocate(struct qnxscreen_api *q,
			  uint32_t width, uint32_t height,
			  uint32_t count, fourcc_t color);
void *qnxscreen_acquire(struct qnxscreen_api *q, uint32_t *index);
bool_e qnxscreen_release(struct qnxscreen_api *q, void *buffer, uint32_t index);
uint32_t qnxscreen_length(struct qnxscreen_api *q, uint32_t index);
uint32_t qnxscreen_search_buffer(struct qnxscreen_api *q, void *buffer);
void *qnxscreen_search_index(struct qnxscreen_api *q, uint32_t index);
bool_e qnxscreen_rotate(struct qnxscreen_api *q, int32_t angle);
bool_e qnxscreen_mirror(struct qnxscreen_api *q, bool_e mirror);
bool_e qnxscreen_position(struct qnxscreen_api *q,
			  uint32_t x, uint32_t y, uint32_t w, uint32_t h);
bool_e qnxscreen_transparency(struct qnxscreen_api *q, uint32_t alpha);
bool_e qnxscreen_metrics(struct qnxscreen_api *q,
			 uint32_t *width, uint32_t *height);

#ifdef __cplusplus
}
#endif

#endif

#endif

