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

#if defined(SURFACE_FLINGER)

#include <surfaceflinger.h>

typedef struct _dvp_display_t {
    surfaceflinger_t *sfd;
} dvp_display_t;

#define _DVP_DISPLAY_T
#include <sosal/pyuv.h>
#include <dvp/dvp_display.h>
#include <dvp/dvp_debug.h>
#if defined(ANDROID)
#include <hwc/hal_public.h>
#endif

#define INITIAL_OFFSET_X (100)
#define INITIAL_OFFSET_Y (100)

void DVP_Display_Destroy(dvp_display_t **dvpd)
{
    if (dvpd && *dvpd)
    {
        if ((*dvpd)->sfd)
        {
            surfaceflinger_free((*dvpd)->sfd);
            surfaceflinger_destroy(&(*dvpd)->sfd);
        }
        free((*dvpd));
        dvpd = NULL;
    }
}

dvp_display_t *DVP_Display_Create(uint32_t image_width,
                                  uint32_t image_height,
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
    DVP_PRINT(DVP_ZONE_VIDEO, "Creating surfaceflinger Display!\n");
    if (color == FOURCC_NV12)
    {
        dvp_display_t *dvpd = (dvp_display_t *)calloc(1, sizeof(dvp_display_t));
        if (dvpd)
        {
            dvpd->sfd = surfaceflinger_create(buffer_width, buffer_height);
            if ( surfaceflinger_allocate(dvpd->sfd, buffer_width, buffer_height, numBuffers, color) )
            {
                DVP_PRINT(DVP_ZONE_VIDEO, "Allocated %u buffers\n", numBuffers);
            }

            surfaceflinger_set_crop(dvpd->sfd, 0, 0, image_width, image_height);

            uint32_t offset_x = INITIAL_OFFSET_X;
            uint32_t offset_y = INITIAL_OFFSET_Y;

            surfaceflinger_set_position(dvpd->sfd, offset_x, offset_y);
        }
        return dvpd;
    }
    else
        return NULL;
}

void DVP_Display_Render(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    if (dvpd && dvpd->sfd)
    {
        if (pImage->reserved)
        {
            void *ptr;
            surfaceflinger_enqueue(dvpd->sfd, pImage->reserved);
            surfaceflinger_dequeue(dvpd->sfd, &ptr);
            // @TODO did pImage->reserved and ptr ==?
        }
    }
}

DVP_BOOL DVP_Display_Alloc(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    DVP_BOOL allocated = DVP_FALSE;
    bool_e result = false_e;
    if (dvpd && dvpd->sfd)
    {
        int32_t stride;
        uint8_t *ptrs[3] = {NULL, NULL, NULL};
        result = surfaceflinger_acquire(dvpd->sfd, &(pImage->reserved), ptrs, &stride);
        if (result)
        {
            switch (pImage->color)
            {
                case FOURCC_NV12:
                    pImage->memType = DVP_MTYPE_DISPLAY_2DTILED;
                    pImage->y_stride = stride;
                    pImage->pBuffer[0] = ptrs[0];
                    pImage->pBuffer[1] = ptrs[1];
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
    if (dvpd && dvpd->sfd)
    {
        if (pImage->reserved)
        {
            surfaceflinger_release(dvpd->sfd, pImage->reserved);
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

