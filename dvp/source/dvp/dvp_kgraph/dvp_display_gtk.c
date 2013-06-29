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

#if defined(GTK_SUPPORT)

#include <gtkwindow.h>
#include <dvp/dvp_debug.h>

typedef struct _dvp_diplay_t {
    GtkWindow_t *w;
    uint32_t width;
    uint32_t height;
    thread_t m_renderer;
    bool_e   m_running;
} dvp_display_t;

DVP_BOOL DVP_Display_Alloc(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    if (dvpd)
    {
        uint32_t index;
        void *buffer = gtkwindow_acquire(dvpd->w, &index);
        if (buffer)
        {
            uint32_t length = gtkwindow_length(dvpd->w, index);
            pImage->pBuffer[0] = buffer;
            pImage->pData[0] = buffer;
            if (pImage->planes == 1)
                pImage->y_stride = length / pImage->bufHeight;
            else if (pImage->color == FOURCC_NV12)
            {
                pImage->y_stride = length / (pImage->bufHeight + pImage->bufHeight/2);
                pImage->pBuffer[1] = &pImage->pBuffer[0][(pImage->y_stride*pImage->bufHeight)];
                pImage->pData[1] = pImage->pBuffer[1];
            }
            // it is important to set this to the length returned from V4L2, not a calculated value.
            pImage->numBytes = length;

#if defined(LINUX) && defined(DVP_USE_TILER)
            // on Android and Linux, V4L2 display buffers are allocated out of the tiler.
            pImage->memType = DVP_MTYPE_DISPLAY_2DTILED;
#endif
            DVP_PrintImage(DVP_ZONE_VIDEO, pImage);
            return DVP_TRUE;
        }
    }
    return DVP_FALSE;
}

DVP_BOOL DVP_Display_Free(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    if (dvpd)
    {
        void *buffer = pImage->pBuffer[0];
        uint32_t index = gtkwindow_search_index(dvpd->w, buffer);
        if (gtkwindow_release(dvpd->w, buffer, index) == false_e)
            return DVP_FALSE;
        else
        {
            memset(pImage, 0, sizeof(DVP_Image_t));
            pImage->memType = DVP_MTYPE_DEFAULT;
            return DVP_TRUE;
        }
    }
    return DVP_FALSE;
}


thread_ret_t DVP_Display_Thread(void *arg)
{
    dvp_display_t *dvpd = (dvp_display_t *)arg;
    while (dvpd->m_running)
    {
        uint32_t index;
        gtkwindow_dequeue(dvpd->w, &index);
    }
    thread_exit(0);
}


void DVP_Display_Render(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    if (dvpd)
    {
        uint32_t index = gtkwindow_search_index(dvpd->w, pImage->pData[0]);
        gtkwindow_queue(dvpd->w, index);
    }
}

void DVP_Display_Destroy(dvp_display_t **pdvpd)
{
    if (pdvpd)
    {
        dvp_display_t *dvpd = *pdvpd;
        if (dvpd)
        {
            dvpd->m_running = false_e;
            gtkwindow_free(dvpd->w);
            gtkwindow_close(dvpd->w);
            thread_join(dvpd->m_renderer);
            free(dvpd);
        }
        *pdvpd = NULL;
    }
}

dvp_display_t *DVP_Display_Create(uint32_t image_width, uint32_t image_height,
                                  uint32_t buffer_width, uint32_t buffer_height,
                                  uint32_t display_width, uint32_t display_height,
                                  uint32_t scaled_width, uint32_t scaled_height,
                                  uint32_t crop_top,  uint32_t crop_left,
                                  fourcc_t color, int32_t rotation, uint32_t numBuffers)
{
    DVP_BOOL unwind = DVP_FALSE;
    dvp_display_t *dvpd = (dvp_display_t *)calloc(1,sizeof(dvp_display_t));
    if (dvpd)
    {
        DVP_PRINT(DVP_ZONE_VIDEO, "DISPLAY: Requesting Color Space %08x\n", color);
        dvpd->width = buffer_width;
        dvpd->height = buffer_height;
        dvpd->w = gtkwindow_open("DVP Display");
        if (dvpd->w && gtkwindow_allocate(dvpd->w, buffer_width, buffer_height, numBuffers, color))
        {
            DVP_PRINT(DVP_ZONE_VIDEO, "DISPLAY: Allocated %u images\n", numBuffers);
            dvpd->m_running = true_e;
            dvpd->m_renderer = thread_create(DVP_Display_Thread, dvpd);
        }
        else
        {
            gtkwindow_close(dvpd->w);
            free(dvpd);
            dvpd = NULL;
        }
    }
    return dvpd;
}

DVP_BOOL DVP_Display_Drop(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    if (dvpd && pImage)
        return DVP_TRUE;
    else
        return DVP_FALSE;
}

#endif

