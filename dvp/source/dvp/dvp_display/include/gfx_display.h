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

#ifndef _GFX_FILE_H_
#define _GFX_FILE_H_

#include <sosal/sosal.h>

#ifndef _GFX_DISPLAY_T
#define _GFX_DISPLAY_T
typedef void * gfx_display_t;
#endif

void gfx_display_destroy(gfx_display_t **pgfxd);

gfx_display_t *gfx_display_create(char *filename);

bool_e gfx_display_allocate(gfx_display_t *gfxd, uint32_t numBuf, uint32_t width, uint32_t height, fourcc_t color);

bool_e gfx_display_free(gfx_display_t *gfxd);

image_t *gfx_display_acquire(gfx_display_t *gfxd, void **pHandle);

void gfx_display_release(gfx_display_t *gfxd, image_t **pimg, void *handle);

bool_e gfx_display_queue(gfx_display_t *gfxd, void *ptr);

bool_e gfx_display_dequeue(gfx_display_t *gfxd, void **pptr);

uint32_t gfx_display_length(gfx_display_t *gfxd);

#endif


