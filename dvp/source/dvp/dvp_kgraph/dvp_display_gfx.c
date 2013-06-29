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

#include <sosal/sosal.h>
#include <dvp/dvp.h>

#if !defined(SURFACE_FLINGER) && (defined(ICS) || defined(JELLYBEAN))

#include <gfx_display.h>

typedef struct _dvp_display_t {
    gfx_display_t *gfxd;
} dvp_display_t;

#define _DVP_DISPLAY_T
#include <sosal/pyuv.h>
#include <dvp/dvp_display.h>
#include <dvp/dvp_debug.h>
#if defined(ANDROID)
#include <hwc/hal_public.h>
#endif

void DVP_Display_Destroy(dvp_display_t **dvpd)
{
    if (dvpd && *dvpd)
    {
        if ((*dvpd)->gfxd)
        {
            gfx_display_free((*dvpd)->gfxd);
            gfx_display_destroy(&(*dvpd)->gfxd);
        }
        free((*dvpd));
        dvpd = NULL;
    }
}

dvp_display_t *DVP_Display_Create(uint32_t image_width __attribute__((unused)),
                                  uint32_t image_height __attribute__((unused)),
                                  uint32_t buffer_width,
                                  uint32_t buffer_height,
                                  uint32_t display_width __attribute__((unused)),
                                  uint32_t display_height __attribute__((unused)),
                                  uint32_t scaled_width __attribute__((unused)),
                                  uint32_t scaled_height __attribute__((unused)),
                                  uint32_t crop_top __attribute__((unused)),
                                  uint32_t crop_left __attribute__((unused)),
                                  fourcc_t color,
                                  int32_t rotation __attribute__((unused)),
                                  uint32_t numBuffers)
{
    DVP_PRINT(DVP_ZONE_VIDEO, "Creating GFX Display!\n");
    if (color == FOURCC_NV12)
    {
        dvp_display_t *dvpd = (dvp_display_t *)calloc(1, sizeof(dvp_display_t));
        if (dvpd)
        {
            char filename[MAX_PATH];
            PYUV_GetFilename(filename, "/sdcard/", "display", buffer_width, buffer_height, 30, color);
            dvpd->gfxd = gfx_display_create(filename);
            if (gfx_display_allocate(dvpd->gfxd, numBuffers, buffer_width, buffer_height, color) == true_e)
            {
                DVP_PRINT(DVP_ZONE_VIDEO, "Allocated %u buffers\n", numBuffers);
            }
        }
        return dvpd;
    }
    else
        return NULL;
}

void DVP_Display_Render(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    if (dvpd && dvpd->gfxd)
    {
        if (pImage->reserved)
        {
            void *ptr;
            gfx_display_queue(dvpd->gfxd, pImage->reserved);
            gfx_display_dequeue(dvpd->gfxd, &ptr);
            // @TODO did pImage->reserved and ptr ==?
        }
    }
}

DVP_BOOL DVP_Display_Alloc(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    DVP_BOOL allocated = DVP_FALSE;
    if (dvpd && dvpd->gfxd)
    {
        image_t *img = gfx_display_acquire(dvpd->gfxd, &(pImage->reserved));
        if (img)
        {
            switch (pImage->color)
            {
                case FOURCC_NV12:
                    pImage->memType = DVP_MTYPE_DISPLAY_2DTILED;
                    pImage->y_stride = img->plane[0].ystride;
                    pImage->pBuffer[0] = img->plane[0].ptr;
                    pImage->pBuffer[1] = img->plane[1].ptr;
                    pImage->pData[0] = pImage->pBuffer[0];
                    pImage->pData[1] = pImage->pBuffer[1];
                    break;
            }
            DVP_PRINT(DVP_ZONE_VIDEO,"Display buffer stride is %u\n", pImage->y_stride);
            allocated = DVP_TRUE;
        }
    }
    return allocated;
}

DVP_BOOL DVP_Display_Free(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    if (dvpd && dvpd->gfxd)
    {
        if (pImage->reserved)
        {
            gfx_display_release(dvpd->gfxd, NULL, pImage->reserved);
        }
    }
    return DVP_TRUE;
}

DVP_BOOL DVP_Display_Drop(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    if (dvpd && pImage)
        return DVP_TRUE;
    else
        return DVP_FALSE;
}

#endif

