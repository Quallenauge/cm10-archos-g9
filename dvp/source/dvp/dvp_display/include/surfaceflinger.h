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

#ifndef _SURFACEFLINGER_H_
#define _SURFACEFLINGER_H_

#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _SURFACEFLINGER_T_
#define _SURFACEFLINGER_T_
typedef void * surfaceflinger_t;
#endif

void surfaceflinger_destroy(surfaceflinger_t **psf);

surfaceflinger_t *surfaceflinger_create(uint32_t width,
                                        uint32_t height);

void surfaceflinger_free(surfaceflinger_t *sf);

bool_e surfaceflinger_allocate(surfaceflinger_t *sf,
                               uint32_t buffer_width,
                               uint32_t buffer_height,
                               int32_t numBuffers,
                               int32_t format);

bool_e surfaceflinger_acquire(surfaceflinger_t *sf, void **pHandle, uint8_t **ptrs, int32_t *stride);

bool_e surfaceflinger_release(surfaceflinger_t *sf, void *handle);

bool_e surfaceflinger_enqueue(surfaceflinger_t *sf, void *handle);

bool_e surfaceflinger_dequeue(surfaceflinger_t *sf, void **pHandle);

bool_e surfaceflinger_drop(surfaceflinger_t *sf, void *handle);

bool_e surfaceflinger_set_crop(surfaceflinger_t *sf,
                               uint32_t crop_left,
                               uint32_t crop_top,
                               uint32_t crop_width,
                               uint32_t crop_height);

bool_e surfaceflinger_set_position(surfaceflinger_t *sf,
                                   uint32_t left,
                                   uint32_t top);

#ifdef __cplusplus
}
#endif

#endif

#endif // _SURFACEFLINGER_H_

