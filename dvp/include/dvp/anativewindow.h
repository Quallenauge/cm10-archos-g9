/*
 *  Copyright (C) 2009-2012 Texas Instruments, Inc.
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

#ifndef _DVPANATIVEWINDOW_H_
#define _DVPANATIVEWINDOW_H_

#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))

#if defined(ICS)
#include <surfaceflinger/Surface.h>
#include <surfaceflinger/ISurface.h>
#else
#include <gui/Surface.h>
#include <gui/ISurface.h>
#endif
#include <ui/GraphicBufferMapper.h>
#include <gui/SurfaceTextureClient.h>
#include <system/window.h>

/*! \brief The "special" code for Android whcih indicates NV12.
 * \ingroup group_display
 */
#define ANW_NV12_FORMAT (0x100)

using namespace android;

/*!
 * \brief A book-keeping structure used to manage the ANativeWindow interface.
 * \ingroup group_display
 */
typedef struct _anativewindow_t {
    sp<ANativeWindow>     m_window;
    ANativeWindowBuffer **m_native_buffers;
    int32_t               m_format;
    uint32_t              m_numBuffers;
    int32_t               m_usage;
    uint32_t              m_nonqueue;
    uint32_t              m_width;
    uint32_t              m_height;
    uint32_t              m_curIdx;
} anativewindow_t;

/*!
 * \brief
 * \param panw The pointer to the pointer to the book-keeping structure
 * \ingroup group_display
 */
void anativewindow_destroy(anativewindow_t **panw);

/*!
 * \brief
 * \ingroup group_display
 */
anativewindow_t *anativewindow_create(const sp<ANativeWindow> &window);

/*!
 * \brief
 * \param anw The pointer to the book-keeping structure
 * \ingroup group_display
 */
void anativewindow_free(anativewindow_t *anw);

/*!
 * \brief
 * \param anw The pointer to the book-keeping structure
 * \ingroup group_display
 */
bool_e anativewindow_allocate(anativewindow_t *anw,
                              uint32_t buffer_width,
                              uint32_t buffer_height,
                              int32_t numBuffers,
                              int32_t format,
                              bool flipH);

/*!
 * \brief
 * \param anw The pointer to the book-keeping structure
 * \ingroup group_display
 */
bool_e anativewindow_acquire(anativewindow_t *anw, void **pHandle, uint8_t **ptrs, int32_t *stride);

/*!
 * \brief
 * \param anw The pointer to the book-keeping structure
 * \ingroup group_display
 */
bool_e anativewindow_release(anativewindow_t *anw, void *handle);

/*!
 * \brief
 * \param anw The pointer to the book-keeping structure
 * \ingroup group_display
 */
bool_e anativewindow_enqueue(anativewindow_t *anw, void *handle);

/*!
 * \brief
 * \param anw The pointer to the book-keeping structure
 * \ingroup group_display
 */
bool_e anativewindow_dequeue(anativewindow_t *anw, void **pHandle);

/*!
 * \brief This causes a single frame to be dropped from the render queue.
 * \param anw The pointer to the book-keeping structure
 * \param handle
 * \ingroup group_display
 */
bool_e anativewindow_drop(anativewindow_t *anw, void *handle);

/*!
 * \brief Sets the cropping rectangle onthe native window.
 * \param anw The pointer to the book-keeping structure
 * \param crop_left
 * \param crop_top
 * \param crop_width
 * \param crop_height
 * \ingroup group_display
 */
bool_e anativewindow_set_crop(anativewindow_t *anw,
                              uint32_t crop_left,
                              uint32_t crop_top,
                              uint32_t crop_width,
                              uint32_t crop_height);


#endif

#endif
