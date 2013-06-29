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

typedef struct _surfaceflinger_t {
    anativewindow_t *anw;
    sp<SurfaceControl> mSurfaceControl;
} surfaceflinger_t;
#define _SURFACEFLINGER_T_

#include <surfaceflinger.h>

#if defined(ICS)
#include <surfaceflinger/SurfaceComposerClient.h>
#include <surfaceflinger/Surface.h>
#include <surfaceflinger/ISurface.h>
#else
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurface.h>
#include <gui/Surface.h>
#endif
#include <ui/GraphicBufferMapper.h>
#include <gui/SurfaceTextureClient.h>
#include <hwc/hal_public.h>
#include <system/window.h>

void surfaceflinger_destroy(surfaceflinger_t **psf)
{
    DVP_PRINT(DVP_ZONE_VIDEO, "surfaceflinger_destroy: ENTER psf = %p\n", psf);

    if (psf && *psf)
    {
        surfaceflinger_t *sf = *psf;

        if (sf->anw)
        {
            anativewindow_destroy(&(sf->anw));
        }

        // clear the entire structure
        memset(sf, 0, sizeof(surfaceflinger_t));
        free(sf);
        *psf = NULL;
    }
}

surfaceflinger_t *surfaceflinger_create(uint32_t width, uint32_t height)
{
    DVP_PRINT(DVP_ZONE_VIDEO, "surfaceflinger_create: ENTER width = %d, height = %d\n",
        width, height);

    surfaceflinger_t *sf = (surfaceflinger_t *)calloc(1, sizeof(surfaceflinger_t));
    if (sf == NULL)
        return NULL;

    sp<SurfaceComposerClient> client = new SurfaceComposerClient();
    sf->mSurfaceControl = client->createSurface(0, width, height, PIXEL_FORMAT_RGBX_8888);

    // This sets the z-order, choose a sufficiently big value so the
    // surface is on top of everything else
    SurfaceComposerClient::openGlobalTransaction();
    sf->mSurfaceControl->setLayer(100000);
    SurfaceComposerClient::closeGlobalTransaction();

    // Get the ANativeWindow
    sp<Surface> surface = sf->mSurfaceControl->getSurface();
    sp<ANativeWindow> window = surface.get();

    DVP_PRINT(DVP_ZONE_VIDEO, "surfaceflinger_create: window = %p\n", window.get());

    sf->anw = anativewindow_create(window);
    if (sf->anw == NULL)
    {
        free(sf);
        return NULL;
    }

    DVP_PRINT(DVP_ZONE_VIDEO, "surfaceflinger_create: SUCCESS sf = %p\n", sf);

    return sf;
}

void surfaceflinger_free(surfaceflinger_t *sf)
{
    DVP_PRINT(DVP_ZONE_VIDEO, "surfaceflinger_free: ENTER sf = %p\n", sf);

    if (sf == NULL)
        return;

    anativewindow_free(sf->anw);
}

bool_e surfaceflinger_allocate(surfaceflinger_t *sf,
                               uint32_t buffer_width,
                               uint32_t buffer_height,
                               int32_t numBuffers,
                               int32_t format)
{
    format = format; // warnings
    DVP_PRINT(DVP_ZONE_VIDEO, "surfaceflinger_allocate: ENTER sf = %p, width = %d, height = %d, num = %d, format = %d\n",
        sf, buffer_width, buffer_height, numBuffers, format);

    if (sf == NULL)
        return false_e;

    bool_e res = anativewindow_allocate(sf->anw,
        buffer_width,
        buffer_height,
        numBuffers,
        ANW_NV12_FORMAT,
        false);

    DVP_PRINT(DVP_ZONE_VIDEO, "surfaceflinger_allocate: EXIT res = %d\n", res);

    return res;
}

bool_e surfaceflinger_acquire(surfaceflinger_t *sf, void **pHandle, uint8_t **ptrs, int32_t *stride)
{
    DVP_PRINT(DVP_ZONE_VIDEO, "surfaceflinger_acquire: ENTER sf = %p\n", sf);

    if (sf == NULL)
        return false_e;

    bool_e res = anativewindow_acquire(sf->anw, pHandle, ptrs, stride);

    DVP_PRINT(DVP_ZONE_VIDEO, "surfaceflinger_acquire: EXIT res = %d, Handle = %p, ptrs = %p, stride = %d\n",
        res, *pHandle, *ptrs, *stride);

    return res;
}

bool_e surfaceflinger_release(surfaceflinger_t *sf, void *handle)
{
    DVP_PRINT(DVP_ZONE_VIDEO, "surfaceflinger_release: ENTER sf = %p\n", sf);

    if (sf == NULL)
        return false_e;

    return anativewindow_release(sf->anw, handle);
}

bool_e surfaceflinger_enqueue(surfaceflinger_t *sf, void *handle)
{
//    DVP_PRINT(DVP_ZONE_VIDEO, "surfaceflinger_enqueue: ENTER sf = %p\n", sf);

    if (sf == NULL)
        return false_e;

    return anativewindow_enqueue(sf->anw, handle);
}

bool_e surfaceflinger_dequeue(surfaceflinger_t *sf, void **pHandle)
{
//    DVP_PRINT(DVP_ZONE_VIDEO, "surfaceflinger_dequeue: ENTER sf = %p\n", sf);

    if (sf == NULL)
        return false_e;

    return anativewindow_dequeue(sf->anw, pHandle);
}

bool_e surfaceflinger_drop(surfaceflinger_t *sf, void *handle)
{
    DVP_PRINT(DVP_ZONE_VIDEO, "surfaceflinger_drop: ENTER sf = %p\n", sf);

    if (sf == NULL)
        return false_e;

    return anativewindow_drop(sf->anw, handle);
}

bool_e surfaceflinger_set_crop(surfaceflinger_t *sf,
                               uint32_t crop_left,
                               uint32_t crop_top,
                               uint32_t crop_width,
                               uint32_t crop_height)
{
    DVP_PRINT(DVP_ZONE_VIDEO, "surfaceflinger_set_crop: ENTER sf = %p\n", sf);

    if (sf == NULL)
        return false_e;

    return anativewindow_set_crop(sf->anw, crop_left, crop_top, crop_width, crop_height);
}

bool_e surfaceflinger_set_position(surfaceflinger_t *sf,
                                   uint32_t left,
                                   uint32_t top)
{
    SurfaceComposerClient::openGlobalTransaction();
    sf->mSurfaceControl->setPosition(left, top);
    SurfaceComposerClient::closeGlobalTransaction();

    return true_e;
}

#endif
