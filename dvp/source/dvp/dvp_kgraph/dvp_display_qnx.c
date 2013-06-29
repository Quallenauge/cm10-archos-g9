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

#if defined(__QNX__)

#include <qnxscreen_api.h>

typedef struct _dvp_display_t {
    struct qnxscreen_api *m_qnxscreen;
    uint32_t width;
    uint32_t height;
    DVP_BOOL m_running;
    DVP_Perf_t m_perf;
} dvp_display_t;

#define _DVP_DISPLAY_T
#include <dvp/dvp_display.h>
#include <dvp/dvp_mem.h>
#include <dvp/dvp_debug.h>

#define INITIAL_OFFSET_X (0)
#define INITIAL_OFFSET_Y (0)

DVP_BOOL DVP_Display_Alloc(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    void *buffer = NULL;
    uint32_t index = 0;
    uint32_t length = 0;

    if (!dvpd || !pImage)
        return DVP_FALSE;

    buffer = qnxscreen_acquire(dvpd->m_qnxscreen, &index);
    if (!buffer)
        return DVP_FALSE;

    length = qnxscreen_length(dvpd->m_qnxscreen, index);
    pImage->pBuffer[0] = buffer;
    pImage->pData[0] = buffer;
    switch (pImage->color) {
    case FOURCC_NV12:
        pImage->y_stride = length / (pImage->bufHeight + pImage->bufHeight/2);
        pImage->pBuffer[1] = &pImage->pBuffer[0][(pImage->y_stride * pImage->bufHeight)];
        pImage->pData[1] = pImage->pBuffer[1];
        break;
    case FOURCC_UYVY:
    default:
        pImage->y_stride = length / pImage->bufHeight; // we know it's strided.
        break;
    }
    pImage->memType = DVP_MTYPE_DISPLAY_2DTILED;
    pImage->numBytes = length;

    DVP_PrintImage(DVP_ZONE_VIDEO, pImage);
    return DVP_TRUE;
}

DVP_BOOL DVP_Display_Free(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
        void *buffer;
        uint32_t index;

    if (!dvpd || !pImage)
        return DVP_FALSE;

        buffer = pImage->pBuffer[0];
        index = qnxscreen_search_buffer(dvpd->m_qnxscreen, buffer);
    if (qnxscreen_release(dvpd->m_qnxscreen, buffer, index) == false_e)
        return DVP_FALSE;

    memset(pImage, 0, sizeof(DVP_Image_t));
    pImage->memType = DVP_MTYPE_DEFAULT;
    return DVP_TRUE;
}

void DVP_Display_Render(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    uint32_t index = 0;

    if (!dvpd || !pImage)
        return;

    DVP_PerformanceStart(&dvpd->m_perf);
    DVP_PRINT(DVP_ZONE_VIDEO, "QNX Screen Rendering DVP_Image_t *%p Buffer %p\n", pImage, pImage->pBuffer[0]);
    // get the index of the buffer
    index = qnxscreen_search_buffer(dvpd->m_qnxscreen, pImage->pBuffer[0]);
    if (index == QNXSCREEN_INVALID_INDEX) {
        DVP_PRINT(DVP_ZONE_VIDEO, "ERROR: V4L2 Image is not allocated from display!\n");
        return;
    }
    qnxscreen_post(dvpd->m_qnxscreen, index);
    DVP_PerformanceStop(&dvpd->m_perf);
    DVP_PerformancePrint(&dvpd->m_perf, "QNXSCREEN");
}

void DVP_Display_Destroy(dvp_display_t **dvpd)
{
    if (!dvpd || !(*dvpd))
        return;

    qnxscreen_free((*dvpd)->m_qnxscreen); // unmap the memory
    qnxscreen_close(&(*dvpd)->m_qnxscreen); // close the driver
    free(*dvpd); // free the data structre
    *dvpd = NULL; // set the user's pointer to null.
}

dvp_display_t *DVP_Display_Create(uint32_t image_width, uint32_t image_height,
                                  uint32_t buffer_width, uint32_t buffer_height,
                                  uint32_t display_width, uint32_t display_height,
                                  uint32_t scaled_width, uint32_t scaled_height,
                                  uint32_t crop_top,  uint32_t crop_left,
                                  fourcc_t color, int32_t rotation, uint32_t numBuffers)
{
    dvp_display_t *dvpd = (dvp_display_t *)calloc(1,sizeof(dvp_display_t));
    uint32_t overlay_width = 0;
    uint32_t overlay_height = 0;
    // @TODO: figure out a better way to handle this for rotation
    uint32_t offset_x = 15;
    uint32_t offset_y = 600-(image_height+15);

    if (!dvpd)
        return NULL;

    DVP_PRINT(DVP_ZONE_VIDEO, "Image: %ux%u Display: %ux%u Scaled:%ux%u\n",
        image_width, image_height,
        display_width, display_height,
        scaled_width, scaled_height);

    dvpd->m_qnxscreen = qnxscreen_open();
    dvpd->width = image_width;
    dvpd->height = image_height;
    DVP_Perf_Clear(&dvpd->m_perf);

    if (qnxscreen_allocate(dvpd->m_qnxscreen, buffer_width, buffer_height, numBuffers, color) == false_e)
        goto unwind;

    // retrieve the information about the overlay
    qnxscreen_metrics(dvpd->m_qnxscreen, &overlay_width, &overlay_height);

    DVP_PRINT(DVP_ZONE_VIDEO, "OVERLAY: %ux%u %ux%u\n", overlay_width, overlay_height, display_width, display_height);

    if (offset_x > (display_width-scaled_width))
        offset_x = (display_width-scaled_width);
    if (offset_y > (display_height-scaled_height))
        offset_y = (display_height-scaled_height);

    qnxscreen_mirror(dvpd->m_qnxscreen, true_e);
    qnxscreen_transparency(dvpd->m_qnxscreen, 100);
    qnxscreen_position(dvpd->m_qnxscreen, offset_x, offset_y, scaled_width, scaled_height);

    // set the rotation of the overlay.
    qnxscreen_rotate(dvpd->m_qnxscreen, rotation);

    // insert a green/gray screen at the start
    {
        DVP_Image_t tmp;
        uint32_t y, i;

        DVP_Image_Init(&tmp, buffer_width, buffer_width, color);
        tmp.width = image_width;
        tmp.height = image_height;
        DVP_Display_Alloc(dvpd, &tmp);
        if (color == FOURCC_UYVY) {
            for (y = 0; y < tmp.height; y++) {
                i = (y * tmp.y_stride);
                memset(&tmp.pData[0][i], 0x80, tmp.width * 2);
            }
        } else if (color == FOURCC_NV12) {
            for (y = 0; y < tmp.height; y++) {
                i = (y * tmp.y_stride);
                memset(&tmp.pData[0][i], 0x80,
                    tmp.width * tmp.x_stride);
            }
            for (y = 0; y < tmp.height; y++) {
                i = (y/2 * tmp.y_stride);
                memset(&tmp.pData[1][i], 0x10,
                    tmp.width / 2 * tmp.x_stride * 2);
            }
        }
        DVP_Display_Render(dvpd, &tmp); // this will do a memcpy
        DVP_Display_Free(dvpd, &tmp);
    }

    return dvpd;
unwind:
    DVP_Display_Destroy(&dvpd);
    return NULL;
}

DVP_BOOL DVP_Display_Drop(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    if (dvpd && pImage)
        return DVP_TRUE;
    else
        return DVP_FALSE;
}

#endif

