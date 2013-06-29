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

// On ANDROID GB or FROYO and on Linux (like Ubuntu)
#if defined(V4L2_SUPPORT)
#include <v4l2_api.h>

typedef struct _dvp_display_t {
    v4l2_api_t *m_v4l2;
    uint32_t width;
    uint32_t height;
    thread_t m_renderer;
    DVP_BOOL m_running;
    DVP_Perf_t m_perf;
    event_t  m_gate;
} dvp_display_t;

#define _DVP_DISPLAY_T
#include <dvp/dvp_display.h>
#include <dvp/dvp_debug.h>

#if defined(BLAZE)  || defined(SDP)
#define INITIAL_OFFSET_X (0)
#define INITIAL_OFFSET_Y (0)
#else
#define INITIAL_OFFSET_X (480)
#define INITIAL_OFFSET_Y (77) // put it below the systray and title bar on some phones
#endif

DVP_BOOL DVP_Display_Alloc(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    if (dvpd && pImage)
    {
        void *buffer = NULL;
        uint32_t index = 0;
        uint32_t length = 0;
        buffer = v4l2_acquire(dvpd->m_v4l2, &index);
        if (buffer)
        {
            length = v4l2_length(dvpd->m_v4l2, index);
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

#if (defined(ANDROID) || defined(LINUX)) && defined(DVP_USE_TILER)
            // on Android and Linux, V4L2 display buffers are allocated out of the tiler.
            pImage->memType = DVP_MTYPE_DISPLAY_2DTILED;
            pImage->skipCacheOpFlush = DVP_TRUE; // uncached
            pImage->skipCacheOpInval = DVP_TRUE; // uncached
#endif
            DVP_PrintImage(DVP_ZONE_VIDEO, pImage);
            return DVP_TRUE;
        }
        else
            return DVP_FALSE;
    }
    else
        return DVP_FALSE;
}

DVP_BOOL DVP_Display_Free(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    if (dvpd && pImage)
    {
        void *buffer = pImage->pBuffer[0];
        uint32_t index = v4l2_search_buffer(dvpd->m_v4l2, buffer);
        if (v4l2_release(dvpd->m_v4l2, buffer, index) == false_e)
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

void sysfs_put(const char *key, const char *value)
{
    size_t len = strlen(value);
    int fd = open(key, O_RDWR);
    if (fd)
    {
        DVP_PRINT(DVP_ZONE_VIDEO, "%s => %s\n", value, key);
        write(fd, value, len);
        close(fd);
    }
}

void sysfs_get(const char *key, char *value, size_t len)
{
    int fd = open(key, O_RDWR);
    if (fd)
    {
        read(fd, value, len);
        close(fd);
        DVP_PRINT(DVP_ZONE_VIDEO, "%s => \"%s\"\n", key, value);
    }
}

void set_rgb_color(uint8_t r, uint8_t g, uint8_t b)
{
    char rv[4];
    char gv[4];
    char bv[4];
    snprintf(rv, sizeof(rv), "%03u", r);
    snprintf(gv, sizeof(gv), "%03u", g);
    snprintf(bv, sizeof(bv), "%03u", b);
#if defined(BLAZE)
    sysfs_put("/sys/class/leds/blue/brightness",  rv);
    sysfs_put("/sys/class/leds/red/brightness",   gv);
    sysfs_put("/sys/class/leds/green/brightness", bv);
#endif
}

void enable_secondary_display()
{
#if defined(ANDROID) && (defined(BLAZE) || defined(SDP))
    char value[20];
    memset(value, 0, sizeof(value));
#if defined(FROYO)
    sysfs_put("/sys/devices/platform/omapdss/overlay2/manager", "2lcd");
    sysfs_put("/sys/devices/platform/omapdss/overlay2/zorder", "3");
    sysfs_put("/sys/devices/platform/omapdss/overlay1/zorder", "2");
    sysfs_get("/sys/devices/platform/omapdss/overlay2/zorder", value, sizeof(value));
#elif defined(GINGERBREAD)
    sysfs_put("/sys/devices/platform/omapdss/display1/enabled", "1");
    sysfs_put("/sys/devices/platform/omapdss/overlay2/manager", "2lcd");
    sysfs_put("/sys/devices/platform/omapdss/overlay2/zorder", "3");
    sysfs_put("/sys/devices/platform/omapdss/overlay1/zorder", "2");
    sysfs_put("/sys/devices/platform/omapdss/overlay0/zorder", "3");
#endif
#endif
}

void disable_secondary_display()
{
#if defined(ANDROID) && (defined(BLAZE) || defined(SDP))
    char value[20];
    memset(value, 0, sizeof(value));
#if defined(FROYO)
    sysfs_put("/sys/devices/platform/omapdss/overlay2/manager", "lcd");
    sysfs_put("/sys/devices/platform/omapdss/overlay2/zorder", "0");
    sysfs_put("/sys/devices/platform/omapdss/overlay1/zorder", "2");
    sysfs_put("/sys/devices/platform/omapdss/overlay0/zorder", "3");
    sysfs_get("/sys/devices/platform/omapdss/overlay2/zorder", value, sizeof(value));
#elif defined(GINGERBREAD)
    sysfs_put("/sys/devices/platform/omapdss/display1/enabled", "0");
    sysfs_put("/sys/devices/platform/omapdss/overlay2/manager", "lcd");
    sysfs_put("/sys/devices/platform/omapdss/overlay2/zorder", "1");
    sysfs_put("/sys/devices/platform/omapdss/overlay1/zorder", "2");
    sysfs_put("/sys/devices/platform/omapdss/overlay0/zorder", "3");
#endif
#endif
}

void enable_primary_display()
{
#if !(defined(BLAZE)  || defined(SDP)) && defined(ANDROID)
    sysfs_put("/sys/devices/platform/omapdss/overlay2/manager", "2lcd");
    sysfs_put("/sys/devices/platform/omapdss/overlay2/rotation", "0");
    sysfs_put("/sys/devices/platform/omapdss/overlay0/zorder", "2");
    sysfs_put("/sys/devices/platform/omapdss/overlay1/zorder", "1");
    sysfs_put("/sys/devices/platform/omapdss/overlay2/zorder", "3");
    sysfs_put("/sys/devices/platform/omapdss/overlay2/global_alpha", "100");
    sysfs_put("/sys/devices/platform/omapdss/overlay2/enabled", "1");
    sysfs_put("/sys/devices/platform/omapdss/manager2/trans_key_enable", "0");
    sysfs_put("/sys/devices/platform/omapdss/manager2/alpha_blending_enabled", "1");
    sysfs_put("/sys/devices/platform/omapdss/manager2/trans_key_value", "0");
    sysfs_put("/sys/devices/platform/omapdss/manager2/trans_key_type", "video-source");
#endif
}

void disable_primary_display()
{
#if !(defined(BLAZE) || defined(SDP)) && defined(ANDROID)
    sysfs_put("/sys/devices/platform/omapdss/overlay2/manager", "2lcd");
    sysfs_put("/sys/devices/platform/omapdss/overlay2/rotation", "0");
    sysfs_put("/sys/devices/platform/omapdss/overlay0/zorder", "3");
    sysfs_put("/sys/devices/platform/omapdss/overlay1/zorder", "1");
    sysfs_put("/sys/devices/platform/omapdss/overlay2/zorder", "1");
    sysfs_put("/sys/devices/platform/omapdss/overlay2/global_alpha", "255");
    sysfs_put("/sys/devices/platform/omapdss/overlay2/enabled", "0");
    sysfs_put("/sys/devices/platform/omapdss/manager2/trans_key_enable", "0");
    sysfs_put("/sys/devices/platform/omapdss/manager2/alpha_blending_enabled", "0");
    sysfs_put("/sys/devices/platform/omapdss/manager2/trans_key_value", "0");
    sysfs_put("/sys/devices/platform/omapdss/manager2/trans_key_type", "gfx-destination");
#endif
}

void DVP_Display_Render(dvp_display_t *dvpd, DVP_Image_t *pImage)
{
    if (dvpd)
    {
        uint32_t index = 0;

        DVP_PerformanceStart(&dvpd->m_perf);

        DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 Rendering DVP_Image_t *%p Buffer %p\n", pImage, pImage->pBuffer[0]);
        // get the index of the buffer
        index = v4l2_search_buffer(dvpd->m_v4l2, pImage->pBuffer[0]);
        if (index != V4L2_INVALID_INDEX)
        {
            //The crop is breaking the Tismo graph since the display is bigger than the image
            //It is only necessary when the x_start and y_start becomes non-zero (no cases yet)
            //v4l2_crop(dvpd->m_v4l2, pImage->x_start, pImage->y_start, pImage->width, pImage->height);
            v4l2_queue(dvpd->m_v4l2, index, V4L2_BUF_TYPE_VIDEO_OUTPUT);
            v4l2_start(dvpd->m_v4l2, V4L2_BUF_TYPE_VIDEO_OUTPUT);
            event_set(&dvpd->m_gate); // unblock the dequeue thread

            DVP_PerformanceStop(&dvpd->m_perf);
            DVP_PerformancePrint(&dvpd->m_perf, "V4L2");
        }
        else
        {
            DVP_PRINT(DVP_ZONE_VIDEO, "ERROR: V4L2 Image is not allocated from display!\n");
        }
    }
}

thread_ret_t DVP_Display_Thread(void *arg)
{
    dvp_display_t *dvpd = (dvp_display_t *)arg;
    thread_ret_t ret = 0;
    DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 Dequeue Thread Launched! (Waiting)\n");
    event_wait(&dvpd->m_gate, EVENT_FOREVER);
    DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 Dequeue Thread Running!\n");
    while (dvpd->m_running == DVP_TRUE)
    {
        void *buffer;
        uint32_t index;
        bool_e dequeued = true_e;
        do {
            if (v4l2_wait(dvpd->m_v4l2) > 0)
            {
                dequeued = v4l2_dequeue(dvpd->m_v4l2, &index, V4L2_BUF_TYPE_VIDEO_OUTPUT);
                if (dequeued)
                {
                    // the actual buffer dequeued
                    buffer = v4l2_search_index(dvpd->m_v4l2, index);
                    DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 Dequeued Buffer %p [%u]\n", buffer, index);
                    break;
                }
                else
                {
                    //DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 Dequeue did not work!\n");
                    // there's just nothing to dequeue yet, sleep a while until
                    // there is something to remove...
                    // this should really be propotional to some frame rate parameter
                    thread_msleep(33);
                }
            }
            else
            {
                DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 Wait failure!\n");
                dequeued = false_e;
            }
        } while (dequeued == false_e && dvpd->m_running == DVP_TRUE);
    }
    DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 Display Thread Exitting\n");
    thread_exit(ret);
}

void DVP_Display_Destroy(dvp_display_t **dvpd)
{
    if (dvpd && *dvpd)
    {
        dvp_display_t *tmp = *dvpd; // convienence pointer

        tmp->m_running = DVP_FALSE;
        event_set(&tmp->m_gate);
        v4l2_stop(tmp->m_v4l2, V4L2_BUF_TYPE_VIDEO_OUTPUT); // this dequeues everything implicitly
        DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 Closing Renderer Thread\n");
        thread_join(tmp->m_renderer); // wait for the thread to finish
        DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 Renderer Closed!\n");
        v4l2_free(tmp->m_v4l2); // unmap the memory
        v4l2_close(&tmp->m_v4l2); // close the driver
        event_deinit(&tmp->m_gate);
        free(tmp); // free the data structre
        *dvpd = NULL; // set the user's pointer to null.

        disable_secondary_display();
        disable_primary_display();
    }
}

dvp_display_t *DVP_Display_Create(uint32_t image_width,   uint32_t image_height,
                                  uint32_t buffer_width,   uint32_t buffer_height,
                                  uint32_t display_width, uint32_t display_height,
                                  uint32_t scaled_width,  uint32_t scaled_height,
                                  uint32_t crop_top,  uint32_t crop_left,
                                  fourcc_t color,
                                  int32_t rotation,
                                  uint32_t numBuffers)
{
    DVP_BOOL unwind = DVP_FALSE;
    dvp_display_t *dvpd = (dvp_display_t *)calloc(1,sizeof(dvp_display_t));
    if (dvpd)
    {
        DVP_PRINT(DVP_ZONE_VIDEO, "Image: %ux%u Display: %ux%u Scaled:%ux%u\n",
                  image_width, image_height,
                  display_width, display_height,
                  scaled_width, scaled_height);
        enable_secondary_display();
        enable_primary_display();
        set_rgb_color(0,0,0); // turn off the LED so that we don't get the light in the camera

        event_init(&dvpd->m_gate, false_e); // a one-shot event.
        dvpd->m_v4l2 = v4l2_open(V4L2_DISPLAY_DEVICE, V4L2_CAP_STREAMING, true_e);
        dvpd->width = image_width;
        dvpd->height = image_height;
        dvpd->m_perf.minTime = 0xFFFFFFFF;
        if (v4l2_allocate(dvpd->m_v4l2, buffer_width, buffer_height, numBuffers, color, V4L2_BUF_TYPE_VIDEO_OUTPUT) == false_e)
            unwind = DVP_TRUE;
        else
        {
            uint32_t overlay_width = 0;
            uint32_t overlay_height = 0;
            uint32_t offset_x = INITIAL_OFFSET_X;
            uint32_t offset_y = INITIAL_OFFSET_Y;

            // retrieve the information about the overlay
            v4l2_metrics(dvpd->m_v4l2, &overlay_width, &overlay_height);

            DVP_PRINT(DVP_ZONE_VIDEO, "OVERLAY: %ux%u %ux%u\n", overlay_width, overlay_height, display_width, display_height);

            if (offset_x > (display_width-scaled_width))
                offset_x = (display_width-scaled_width);
            if (offset_y > (display_height-scaled_height))
                offset_y = (display_height-scaled_height);

#if (defined(BLAZE) || defined(SDP)) && defined(ANDROID)
            v4l2_mirror(dvpd->m_v4l2, true_e);
            v4l2_crop(dvpd->m_v4l2, crop_left, crop_top, image_width, image_height);
            v4l2_position(dvpd->m_v4l2, offset_x, offset_y, scaled_width, scaled_height);
#endif

#if !(defined(BLAZE) || defined(SDP)) && defined(ANDROID)
            // no mirror, it's not supported.
            v4l2_position(dvpd->m_v4l2, offset_x, offset_y, scaled_width, scaled_height);
            //if (color == FOURCC_NV12)
            //    v4l2_crop(dvpd->m_v4l2, 0, 32, image_width, image_height);
            v4l2_crop(dvpd->m_v4l2, crop_left, crop_top, image_width, image_height);
            if (color != FOURCC_NV12)
                v4l2_transparency(dvpd->m_v4l2, V4L2_FBUF_FLAG_CHROMAKEY, 0);
#endif
            // set the rotation of the overlay.
            v4l2_rotate(dvpd->m_v4l2, rotation);

            // create the renderer
            dvpd->m_running = DVP_TRUE;
            dvpd->m_renderer = thread_create(DVP_Display_Thread, (void *)dvpd);

            // insert a green/gray screen at the start
            {
                DVP_Image_t tmp;
                uint32_t y, i;

                DVP_Image_Init(&tmp, buffer_width, buffer_height, color);
                tmp.width = image_width;
                tmp.height = image_height;
                DVP_Display_Alloc(dvpd, &tmp);
                if (color == FOURCC_UYVY) {
                    for (y = 0; y < tmp.height; y++) {
                        i = (y * tmp.y_stride);
                        memset(&tmp.pData[0][i], 0x80, tmp.width*tmp.x_stride);
                    }
                }
                else if (color == FOURCC_NV12)
                {
                    for (y = 0; y < tmp.height; y++) {
                        i = (y * tmp.y_stride);
                        memset(&tmp.pData[0][i], 0x80, tmp.width*tmp.x_stride);
                    }
                    for (y = 0; y < tmp.height; y++) {
                        i = (y/2 * tmp.y_stride);
                        memset(&tmp.pData[1][i], 0x10, tmp.width/2*tmp.x_stride*2);
                    }
                }
                else if (color == FOURCC_RGB || color == FOURCC_BGR)
                {
                    for (y = 0; y < tmp.height; y++) {
                        i = (y * tmp.y_stride);
                        memset(&tmp.pData[0][i], 0x80, tmp.width*tmp.x_stride);
                    }
                }
                DVP_Display_Render(dvpd, &tmp); // this will do a memcpy
                DVP_Display_Free(dvpd, &tmp);
            }
#if !(defined(BLAZE)  || defined(SDP))
            // send the alpha value again!
            sysfs_put("/sys/devices/platform/omapdss/overlay2/global_alpha", "100");
#endif
        }
    }
    if (unwind)
        DVP_Display_Destroy(&dvpd);
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

