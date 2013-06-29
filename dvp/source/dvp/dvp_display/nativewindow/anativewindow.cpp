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

#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))

#include <sosal/sosal.h>
#include <dvp/anativewindow.h>
#include <dvp/dvp_debug.h>

#define ANW_ERROR(status, string) { \
    if (status < 0) { \
        DVP_PRINT(DVP_ZONE_ERROR, "%s[%u] %s status = %d\n", __FUNCTION__, __LINE__, string, status); \
    }\
}

void anativewindow_destroy(anativewindow_t **panw)
{
    if (panw && *panw)
    {
        anativewindow_t *anw = *panw;
        free(anw);
        *panw = NULL;
    }
}

anativewindow_t *anativewindow_create(const sp<ANativeWindow> &window)
{
    anativewindow_t *anw = (anativewindow_t *)calloc(1, sizeof(anativewindow_t));
    if (anw)
    {
        // clear the entire structure
        memset(anw, 0, sizeof(anativewindow_t));
        anw->m_window = window;
    }
    return anw;
}

void anativewindow_free(anativewindow_t *anw)
{
    status_t status;
    GraphicBufferMapper &mapper = GraphicBufferMapper::get();

    for (uint32_t i = 0; i < anw->m_numBuffers; i++)
    {
        if (anw->m_native_buffers[i])
        {
            status = mapper.unlock((buffer_handle_t)anw->m_native_buffers[i]->handle);
            ANW_ERROR(status,"UNLOCK");
            status = anw->m_window->cancelBuffer(anw->m_window.get(), anw->m_native_buffers[i]);
            ANW_ERROR(status,"CANCEL");
        }
    }
    // disconnect from the native window
    status = native_window_api_disconnect(anw->m_window.get(), NATIVE_WINDOW_API_CAMERA);
    ANW_ERROR(status,"DISCONNECT");

    anw->m_window.clear();
    free(anw->m_native_buffers);
}

bool_e anativewindow_allocate(anativewindow_t *anw,
                              uint32_t buffer_width,
                              uint32_t buffer_height,
                              int32_t numBuffers,
                              int32_t format,
                              bool flipH)
{
    if (format != ANW_NV12_FORMAT)
        return false_e;

    if (anw)
    {
        status_t status = 0;

        anw->m_format = format;
        anw->m_usage = GRALLOC_USAGE_HW_TEXTURE |
                       GRALLOC_USAGE_HW_RENDER |
                       GRALLOC_USAGE_SW_READ_OFTEN | // Non-cached ? If you use RARELY it will complain
                       GRALLOC_USAGE_SW_WRITE_NEVER; // can't access from UVA
        anw->m_numBuffers = numBuffers;
        anw->m_width = buffer_width;
        anw->m_height = buffer_height;
        anw->m_native_buffers = (ANativeWindowBuffer **)calloc(numBuffers, sizeof(ANativeWindowBuffer *));
        DVP_PRINT(DVP_ZONE_VIDEO, "Created %u pointers in native array %p\n", numBuffers, anw->m_native_buffers);
        if (anw->m_native_buffers == NULL) {
            return false_e;
        }

        // connect to the native window API
        status = native_window_api_connect(anw->m_window.get(), NATIVE_WINDOW_API_CAMERA);
        ANW_ERROR(status,"CONNECT");

        // set the scaling mode of the windows
        status = native_window_set_scaling_mode(anw->m_window.get(), NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
        ANW_ERROR(status,"SCALING");

#if defined(BLAZE_TABLET) && defined(ICS)
        // set the layout of the buffer
        status = native_window_set_buffers_layout(anw->m_window.get(), NATIVE_WINDOW_BUFFERS_LAYOUT_PROGRESSIVE); // progressive
        ANW_ERROR(status,"LAYOUT");
#endif

        // set the format of the buffer
        status = native_window_set_buffers_format(anw->m_window.get(), anw->m_format);
        ANW_ERROR(status,"FORMAT");

        // setup the dimensions
        status = native_window_set_buffers_dimensions(anw->m_window.get(), anw->m_width, anw->m_height);
        ANW_ERROR(status,"DIM");

        if (flipH) {
            // set the horizontal flip
            status = native_window_set_buffers_transform(anw->m_window.get(), NATIVE_WINDOW_TRANSFORM_FLIP_H);
            ANW_ERROR(status,"TRANSFORM FLIP HORIZONTAL");
        }

        // set the usage of the GRALLOC buffers
        status = native_window_set_usage(anw->m_window.get(), anw->m_usage);
        if (status < 0 ) {
            DVP_PRINT(DVP_ZONE_ERROR, "%s[%u] USAGE status = %d (0x%08x)\n", __FUNCTION__, __LINE__, status, anw->m_usage);
        }

        // set the number of buffers required.
        status = native_window_set_buffer_count(anw->m_window.get(), anw->m_numBuffers);
        ANW_ERROR(status,"COUNT");

        // get the number of dequeueable buffers
        status = anw->m_window->query(anw->m_window.get(), NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, (int*)&anw->m_nonqueue);
        ANW_ERROR(status,"QUERY NONQUEUE");

        for (uint32_t n = 0; n < anw->m_numBuffers; n++)
        {
            status = anw->m_window->dequeueBuffer(anw->m_window.get(), &anw->m_native_buffers[n]);
            if (status != 0) {
                DVP_PRINT(DVP_ZONE_ERROR, "%s[%u] %s status = %d\n", __FUNCTION__, __LINE__, "Failed window buffer dequeue!", status); \
                return false_e;
            }
            if (anw->m_native_buffers[n])
            {
                native_handle_t *hdl = (native_handle_t *)anw->m_native_buffers[n]->handle;
                hdl = hdl; // warnings
                DVP_PRINT(DVP_ZONE_VIDEO, "ANativeBuffer %p => dim %dx%d stride %d usage %d format %d handle %p numFds=%u\n",
                         anw->m_native_buffers[n],
                         anw->m_native_buffers[n]->width,
                         anw->m_native_buffers[n]->height,
                         anw->m_native_buffers[n]->stride,
                         anw->m_native_buffers[n]->usage,
                         anw->m_native_buffers[n]->format,
                         anw->m_native_buffers[n]->handle,
                         hdl->numFds);
            }
        }
        return true_e;
    }
    return false_e;
}

bool_e anativewindow_acquire(anativewindow_t *anw, void **pHandle, uint8_t **ptrs, int32_t *stride)
{
    bool_e ret = false_e;
    if (anw && anw->m_native_buffers && anw->m_curIdx < anw->m_numBuffers && anw->m_native_buffers[anw->m_curIdx])
    {
        status_t status = 0;
        GraphicBufferMapper &mapper = GraphicBufferMapper::get();
        Rect bounds;

        bounds.left = 0;
        bounds.right = anw->m_width;
        bounds.top = 0;
        bounds.bottom = anw->m_height;

        status = anw->m_window->lockBuffer(anw->m_window.get(), anw->m_native_buffers[anw->m_curIdx]);
        ANW_ERROR(status,"LOCK WINDOW");

        status = mapper.lock((buffer_handle_t)anw->m_native_buffers[anw->m_curIdx]->handle, anw->m_usage, bounds, (void **)ptrs);
        ANW_ERROR(status,"LOCK");
        if (status == NO_ERROR)
        {
            //DVP_PRINT(DVP_ZONE_VIDEO, "ptrs = {%p, %p, %p}\n", ptrs[0], ptrs[1], ptrs[2]);
            *pHandle = (void *)anw->m_native_buffers[anw->m_curIdx]->handle;
            *stride = anw->m_native_buffers[anw->m_curIdx]->stride;
            ret = true_e;
        }

        anw->m_curIdx++;
    }
    return ret;
}

bool_e anativewindow_release(anativewindow_t *anw, void *handle)
{
    if (anw && handle)
    {
        GraphicBufferMapper &mapper = GraphicBufferMapper::get();
        status_t status = NO_ERROR;

        for (uint32_t i = 0; i < anw->m_numBuffers; i++)
        {
            if (anw->m_native_buffers[i] && handle == anw->m_native_buffers[i]->handle)
            {
                status = mapper.unlock((buffer_handle_t)anw->m_native_buffers[i]->handle);
                ANW_ERROR(status,"UNLOCK");
                status = anw->m_window->cancelBuffer(anw->m_window.get(), anw->m_native_buffers[i]);
                ANW_ERROR(status,"CANCEL");
                anw->m_native_buffers[i] = NULL;
                return true_e;
            }
        }
    }
    return false_e;
}

bool_e anativewindow_enqueue(anativewindow_t *anw, void *handle)
{
    if (anw && handle)
    {
        GraphicBufferMapper &mapper = GraphicBufferMapper::get();
        for (uint32_t i = 0; i < anw->m_numBuffers; i++) {
            if (handle == anw->m_native_buffers[i]->handle) {
                status_t status;

                status = mapper.unlock((buffer_handle_t)anw->m_native_buffers[i]->handle);
                if (status < 0 ) {
                    DVP_PRINT(DVP_ZONE_ERROR, "%s[%u] %p UNLOCK status = %d\n", __FUNCTION__, __LINE__, anw->m_native_buffers[i], status);
                }
                status = anw->m_window->queueBuffer(anw->m_window.get(), anw->m_native_buffers[i]);
                if (status < 0 ) {
                    DVP_PRINT(DVP_ZONE_ERROR, "%s[%u] %p QUEUE status = %d\n", __FUNCTION__, __LINE__, anw->m_native_buffers[i], status);
                }
                return true_e;
            }
        }
    }
    return false_e;
}

bool_e anativewindow_dequeue(anativewindow_t *anw, void **pHandle)
{
    if (anw)
    {
        GraphicBufferMapper &mapper = GraphicBufferMapper::get();
        status_t status;
        uint8_t *ptrs[3] = {NULL, NULL, NULL};
        Rect bounds;
        uint32_t lockRetries = 10;
        ANativeWindowBuffer *nativeBuffer = NULL;

        bounds.left = 0;
        bounds.right = anw->m_width;
        bounds.top = 0;
        bounds.bottom = anw->m_height;

        // Now we're dequeuing a different buffer from the window...

        status = anw->m_window->dequeueBuffer(anw->m_window.get(), &nativeBuffer);
        if (status < 0 ) {
            DVP_PRINT(DVP_ZONE_ERROR, "%s[%u] %p DEQUEUE status = %d\n", __FUNCTION__, __LINE__, nativeBuffer, status);
        }

        // early exit
        if (nativeBuffer == NULL)
            return false_e;

        status = anw->m_window->lockBuffer(anw->m_window.get(), nativeBuffer);
        if (status < 0 ) {
            DVP_PRINT(DVP_ZONE_ERROR, "%s[%u] %p LOCKWINDOW status = %d\n", __FUNCTION__, __LINE__, nativeBuffer, status);
        }
        do {
            status = mapper.lock(nativeBuffer->handle, anw->m_usage, bounds, (void **)ptrs);
            if (status < 0 ) {
                DVP_PRINT(DVP_ZONE_ERROR, "%s[%u] %p LOCK status = %d\n", __FUNCTION__, __LINE__, nativeBuffer, status);
            }
        } while (status !=  NO_ERROR && lockRetries--);

        if (pHandle)
            *pHandle = (void *)nativeBuffer->handle;

        return true_e;
    }
    return false_e;
}

bool_e anativewindow_drop(anativewindow_t *anw, void *handle)
{
    if (anw && handle)
    {
        GraphicBufferMapper &mapper = GraphicBufferMapper::get();
        for (uint32_t i = 0; i < anw->m_numBuffers; i++) {
            if (handle == anw->m_native_buffers[i]->handle) {
                status_t status;

                status = mapper.unlock((buffer_handle_t)anw->m_native_buffers[i]->handle);
                if (status < 0 ) {
                    DVP_PRINT(DVP_ZONE_ERROR, "%s[%u] %p UNLOCK status = %d\n", __FUNCTION__, __LINE__, anw->m_native_buffers[i], status);
                }
                status = anw->m_window->cancelBuffer(anw->m_window.get(), anw->m_native_buffers[i]);
                if (status < 0 ) {
                    DVP_PRINT(DVP_ZONE_ERROR, "%s[%u] %p CANCEL status = %d\n", __FUNCTION__, __LINE__, anw->m_native_buffers[i], status);
                }
                if (status >= 0)
                    return true_e;
            }
        }
    }
    return false_e;
}

bool_e anativewindow_set_crop(anativewindow_t *anw,
                              uint32_t crop_left,
                              uint32_t crop_top,
                              uint32_t crop_width,
                              uint32_t crop_height)
{
    if (anw)
    {
        status_t status = 0;
        android_native_rect_t crop = {crop_left, crop_top, crop_width, crop_height};

        if (!crop_left && !crop_top && !crop_width && !crop_height)
            status = native_window_set_crop(anw->m_window.get(), NULL);
        else
            status = native_window_set_crop(anw->m_window.get(), &crop);

        ANW_ERROR(status,"CROP");

        return (status < 0 ? false_e : true_e);
    }
    return false_e;
}

#endif


