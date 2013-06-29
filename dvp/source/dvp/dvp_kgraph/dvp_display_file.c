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

#if defined(DVP_USE_FS)

#if !defined(WIN32)
#warning "Building DVP Display in FILE output mode!"
#else
#pragma message("***** Building DVP Display in FILE output mode!")
#endif

typedef struct _dvp_display_t {
    FILE *out;
} dvp_display_t;

#define _DVP_DISPLAY_T
#include <sosal/pyuv.h>
#include <dvp/dvp_display.h>
#include <dvp/dvp_debug.h>


dvp_display_t *DVP_Display_Create(uint32_t image_width, uint32_t image_height,
                                  uint32_t buffer_width, uint32_t buffer_height,
                                  uint32_t display_width, uint32_t display_height,
                                  uint32_t scaled_width, uint32_t scaled_height,
                                  uint32_t crop_top,  uint32_t crop_left,
                                  fourcc_t color, int32_t rotation, uint32_t numBuffers)
{
    DVP_PRINT(DVP_ZONE_VIDEO, "Creating File Based DVP Display!\n");
    if (color == FOURCC_UYVY || color == FOURCC_NV12 || color == FOURCC_YUY2)
    {
        dvp_display_t *dvpd = (dvp_display_t *)calloc(1, sizeof(dvp_display_t));
        if (dvpd)
        {
            char display_name[255];
            PYUV_GetFilename(display_name, "", "display", image_width, image_height, 30, color);
            //sprintf(display_name, "display_%ux%u_%s.yuv", image_width, image_height, (color==FOURCC_UYVY?"UYVY":"NV12"));
            dvpd->out = fopen(display_name, "wb+");
            DVP_PRINT(DVP_ZONE_VIDEO, "Opened file %s\n", display_name);
        }
        return dvpd;
    }
    else
        return NULL;
}

void DVP_Display_Destroy(dvp_display_t **dvpd)
{
    if (dvpd && *dvpd)
    {
        if ((*dvpd)->out)
            fclose((*dvpd)->out);
        free((*dvpd));
        *dvpd = NULL;
    }
}

void DVP_Display_Render(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    uint32_t y, j, n = 0, len = 0;
    if (dvpd == NULL || dvpd->out == NULL)
        return;
    if (pImage->color == FOURCC_UYVY || pImage->color == FOURCC_YUY2)
    {
        for (y = 0; y < pImage->height; y++)    // loop for each line
        {
            len = (pImage->x_stride * pImage->width);
            j = (y * pImage->y_stride);
            n += (uint32_t)fwrite(&pImage->pData[0][j], 1, len, dvpd->out);
        }
        fflush(dvpd->out);
    }
    else if (pImage->color == FOURCC_NV12)
    {
        for (y = 0; y < pImage->height ; y++)
        {
            len = (pImage->x_stride * pImage->width);
            j = (y * pImage->y_stride);
            n+= (DVP_U32)fwrite(&pImage->pData[0][j], 1, len, dvpd->out);
        }
        for (y = 0; y < pImage->height/2; y++)
        {
            len = (pImage->x_stride * pImage->width); // half-width but double pixels
            j = (y * pImage->y_stride);
            n+= (DVP_U32)fwrite(&pImage->pData[1][j], 1, len, dvpd->out);
        }
        fflush(dvpd->out);
    }
}

DVP_BOOL DVP_Display_Alloc(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    uint32_t p = 0;
    if (pImage->color == FOURCC_UYVY || pImage->color == FOURCC_YUY2)
    {
        uint32_t planeSize = pImage->bufWidth * pImage->bufHeight * pImage->x_stride;
        pImage->pBuffer[0] = calloc(1, planeSize);
        pImage->pData[0] = pImage->pBuffer[0];
        pImage->numBytes = planeSize;
    }
    else if (pImage->color == FOURCC_NV12)
    {
        uint32_t numBytes = (pImage->bufWidth * pImage->bufHeight * 3)/2;
        pImage->pBuffer[0] = calloc(1, numBytes);
        pImage->pBuffer[1] = &pImage->pBuffer[0][pImage->height * pImage->width];
        pImage->pData[0] = pImage->pBuffer[0];
        pImage->pData[1] = pImage->pBuffer[1];
    }
    pImage->y_stride = pImage->width * pImage->x_stride;
    pImage->memType = DVP_MTYPE_MPUCACHED_VIRTUAL;
    if (pImage->pBuffer[0])
        return DVP_TRUE;
    else
        return DVP_FALSE;
}

DVP_BOOL DVP_Display_Free(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    free(pImage->pBuffer[0]);
    pImage->pBuffer[0] = NULL;
    pImage->pBuffer[1] = NULL;
    pImage->pData[0] = NULL;
    pImage->pData[1] = NULL;
    pImage->numBytes = 0;
    pImage->y_stride = 0;
    pImage->memType = DVP_MTYPE_DEFAULT;
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

