/**
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

#if defined(__QNX__)

#include <qnxscreen_api.h>
#include <dvp/dvp_debug.h>

#define QNXSCREEN_EXTRA_DEBUG

void qnxscreen_close(struct qnxscreen_api **q)
{
    if (!q || (*q))
        return;

    /* This should free all the associated context allocated resources */
    screen_destroy_context((*q)->ctx);

    free(*q);
    *q = NULL;
}

struct qnxscreen_api *qnxscreen_open(void)
{
    struct qnxscreen_api *q;
    int ret;

    q = calloc(1, sizeof(struct qnxscreen_api));

    if (!q) {
            DVP_PRINT(DVP_ZONE_ERROR,
              "(%s) Failure to allocate base struct\n", __func__);
        return q;
    }

    mutex_init(&q->m_lock);

    ret = screen_create_context(&q->ctx, SCREEN_APPLICATION_CONTEXT);
    if (ret) {
            DVP_PRINT(DVP_ZONE_ERROR,
              "(%s) Failure on screen_create_context: (%d)\n",
              __func__, errno);
        goto fail;
    }

    return q;

fail:
    free(q);
    return NULL;
}

uint32_t qnxscreen_search_buffer(struct qnxscreen_api *q, void *buffer)
{
    uint32_t index = QNXSCREEN_INVALID_INDEX;

    for (index = 0; index < q->count; index++) {
        void *tmpbuf_dat;
        int ret;

        ret = screen_get_buffer_property_pv(q->buf[index],
                            SCREEN_PROPERTY_POINTER,
                            (void **)&tmpbuf_dat);
        if (ret)
            return QNXSCREEN_INVALID_INDEX;

        if (tmpbuf_dat == buffer)
            return index;
    }

    if (index >= q->count)
        index = QNXSCREEN_INVALID_INDEX;

    return index;
}

void *qnxscreen_search_index(struct qnxscreen_api *q, uint32_t index)
{
    void *tmpbuf_dat;
    int ret;

    if (index > q->count || index > QNXSCREEN_MAX_INDEX)
        return NULL;

    ret = screen_get_buffer_property_pv(q->buf[index], SCREEN_PROPERTY_POINTER,
                        (void **)&tmpbuf_dat);
    if (ret)
        return NULL;

    return tmpbuf_dat;
}

bool_e qnxscreen_post(struct qnxscreen_api *q, uint32_t index)
{
    int clipRect[4];

    if (!q) {
            DVP_PRINT(DVP_ZONE_ERROR,
              "(%s) OOPS: Invalid struct qnxscreen_api pointer."
              " Something is seriously wrong!\n",
              __func__);
        return false_e;
    }

    if (index > q->count || index > QNXSCREEN_MAX_INDEX) {
        DVP_PRINT(DVP_ZONE_ERROR, "Invalid index %d\n", index);
        return false_e;
    }

    clipRect[0] = q->metrics.left;
    clipRect[1] = q->metrics.top;
    clipRect[2] = q->metrics.left + q->metrics.width;
    clipRect[3] = q->metrics.top + q->metrics.height;

    /* Just flush out the only buffer we have */
    if (screen_post_window(q->win, q->buf[index], 1, clipRect, 0)) {
            DVP_PRINT(DVP_ZONE_ERROR,
              "(%s): Failure on screen_post_window (%d)\n",
              __func__, errno);
        return false_e;
    }

    return true_e;
}

uint32_t qnxscreen_length(struct qnxscreen_api *q, uint32_t index)
{
    uint32_t len = 0;
    int err;
    int pformat = 0;
    int stride = 0;

    err = screen_get_window_property_iv(q->win, SCREEN_PROPERTY_FORMAT, &pformat);
    if (err) {
        DVP_PRINT(DVP_ZONE_ERROR, "QNX Screen Length of Buffer is unknown\n");
        return 0;
    }

    err = screen_get_buffer_property_iv(q->buf[index],
                                        SCREEN_PROPERTY_STRIDE, &stride);
    if (err) {
        DVP_PRINT(DVP_ZONE_ERROR, "QNX Screen Stride of Buffer is unknown\n");
        return 0;
    } else {
        DVP_PRINT(DVP_ZONE_VIDEO, "QNX Screen Buffer Stride is %d bytes\n", stride);
    }

    switch (pformat) {
        case SCREEN_FORMAT_NV12:
            len = stride * q->metrics.height;
            len += len / 2; /// @note remember to reverse this outside the function.
            break;
        default:
            len = stride * q->metrics.height;
            break;
    }

    DVP_PRINT(DVP_ZONE_VIDEO, "QNX Screen Length of Buffer is %u bytes\n", len);
    return len;
}

void *qnxscreen_acquire(struct qnxscreen_api *q, uint32_t *index)
{
    void *buf = NULL;
    *index = QNXSCREEN_INVALID_INDEX;

    if (!q)
        return NULL;

    mutex_lock(&q->m_lock);
    if (bitfield_get(&q->used, index) == false_e) {
        mutex_unlock(&q->m_lock);
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Failed to acquire QNX Screen Buffer!\n");
        return NULL;
        }

    buf = qnxscreen_search_index(q, *index);
#ifdef QNXSCREEN_EXTRA_DEBUG
    DVP_PRINT(DVP_ZONE_VIDEO, "QNX Screen Acquiring %p index %u\n", buf, *index);
#endif
    mutex_unlock(&q->m_lock);

    return buf;
}

bool_e qnxscreen_release(struct qnxscreen_api *q, void *buffer, uint32_t index)
{
    bool_e ret = false_e;

    if (!q)
        return false_e;

    mutex_lock(&q->m_lock);
    if (index < q->count) {
#ifdef QNXSCREEN_EXTRA_DEBUG
        DVP_PRINT(DVP_ZONE_VIDEO, "QNX Screen Releasing %p index %u\n", buffer, index);
#endif
        if (bitfield_rls(&q->used, index) == false_e) {
            DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Failed to release QNX Screen Buffer!\n");
            ret = false_e;
        }
    } else if (index == QNXSCREEN_INVALID_INDEX) {
        index = qnxscreen_search_buffer(q, buffer);
        if (index != QNXSCREEN_INVALID_INDEX) {
#ifdef QNXSCREEN_EXTRA_DEBUG
            DVP_PRINT(DVP_ZONE_VIDEO, "QNX Screen Releasing %p index %u\n", buffer, index);
#endif
            if (bitfield_rls(&q->used, index) == false_e) {
                DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Failed to release QNX Screen Buffer!\n");
            } else {
                ret = true_e;
            }
        } else {
            DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Buffer was not a QNX Screen Buffer!\n");
        }
    } else {
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Buffer was not a QNX Screen Buffer!\n");
    }
    mutex_unlock(&q->m_lock);
    return ret;
}

bool_e qnxscreen_free(struct qnxscreen_api *q)
{
    uint32_t i;

    if (!q)
        return false_e;

    mutex_lock(&q->m_lock);
    screen_destroy_window(q->win);
    q->count = 0;
    bitfield_deinit(&q->used);
    mutex_unlock(&q->m_lock);
    return true_e;
}

bool_e qnxscreen_metrics(struct qnxscreen_api *q,
             uint32_t *width, uint32_t *height)
{
    int size[2] = {0};
    int ret;

    if (!q)
        return false_e;

    ret = screen_get_window_property_iv(q->win, SCREEN_PROPERTY_SIZE, size);
    if (ret)
        return false_e;

    *width = size[0];
    *height = size[1];
    return true_e;
}

bool_e qnxscreen_rotate(struct qnxscreen_api *q, int32_t angle)
{
    int ret;

    ret = screen_set_window_property_iv(q->win, SCREEN_PROPERTY_ROTATION, (int *)&angle);
    if (ret)
        return false_e;

    return true_e;
}

bool_e qnxscreen_mirror(struct qnxscreen_api *q, bool_e mirror)
{
    int ret;
    int val;

    if (mirror == true_e)
        val = 1;
    else
        val = 0;

    ret = screen_set_window_property_iv(q->win, SCREEN_PROPERTY_MIRROR, &val);
    if (ret)
        return false_e;

    return true_e;
}

bool_e qnxscreen_position(struct qnxscreen_api *q,
              uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    int size[2] = {(int)w, (int)h};
    int pos[2] = {(int)x, (int)y};
    int ret;

    ret = screen_set_window_property_iv(q->win, SCREEN_PROPERTY_POSITION, pos);
    if (ret)
        return false_e;

    ret = screen_set_window_property_iv(q->win, SCREEN_PROPERTY_SIZE, size);
    if (ret)
        return false_e;

    return true_e;
}

bool_e qnxscreen_transparency(struct qnxscreen_api *q, uint32_t alpha)
{
    int ret;

    if (alpha > 255)
        alpha = 255;

    ret = screen_set_window_property_iv(q->win, SCREEN_PROPERTY_GLOBAL_ALPHA, (int *)&alpha);
    if (ret)
        return false_e;

    return true_e;
}

typedef struct _qnxscreen_pix_to_fourcc_lut_t {
    int qnxscreen_fmt;
    uint32_t fourcc;
} QNXSCR_to_FOURCC_t;

QNXSCR_to_FOURCC_t codes[] = {
    {SCREEN_FORMAT_UYVY, FOURCC_UYVY},
    {SCREEN_FORMAT_NV12, FOURCC_NV12},
    {SCREEN_FORMAT_RGB888, FOURCC_BGR},
};
uint32_t numCodes = dimof(codes);

/* NOTE: Shouldn't this be more flexible? */
#define OMX_CAMERA_ALIGNMENT                128

bool_e qnxscreen_allocate(struct qnxscreen_api *q,
              uint32_t width, uint32_t height,
              uint32_t count, fourcc_t color)
{
    bool_e ret = false_e;
    int size[2] = {(int)width, (int)height};
    int bsize[2] = {(int)width, (int)height};
    int i;
    int pformat = 0;
    int err;
    int zOrder = 50;
    int dispCount = 0;
    int formatCount = 0;
    int sensitivity = SCREEN_SENSITIVITY_NEVER;
    int tranparency = SCREEN_TRANSPARENCY_NONE;
    int idleMode = SCREEN_IDLE_MODE_KEEP_AWAKE;
    screen_display_t *displays;
    screen_display_t display;
    int formats[SCREEN_FORMAT_NFORMATS];
    bool_e configured = false_e;
    const int usage = SCREEN_USAGE_WRITE | SCREEN_USAGE_NATIVE | SCREEN_USAGE_CAPTURE;

    mutex_lock(&q->m_lock);

    err = screen_create_window_type(&q->win, q->ctx, SCREEN_APPLICATION_WINDOW);
    if (err) {
            DVP_PRINT(DVP_ZONE_ERROR,
              "(%s) Failure on screen_create_window_type: (%d)\n",
              __func__, errno);
        goto leave;
    }

    err = screen_get_context_property_iv(q->ctx, SCREEN_PROPERTY_DISPLAY_COUNT, (void *)&dispCount);
    if (err) {
        DVP_PRINT(DVP_ZONE_ERROR, "Failed to query the number of displays! err=%d\n",err);
        goto leave2;
    }
    DVP_PRINT(DVP_ZONE_VIDEO, "There are %u displays\n", dispCount);

    err = screen_set_window_property_iv(q->win, SCREEN_PROPERTY_ZORDER, &zOrder);
    if (err) {
            DVP_PRINT(DVP_ZONE_ERROR,
              "(%s) Failure on screen_create_window_type: (%d)\n",
              __func__, errno);
        goto leave2;
    }

    // this tells the window to not receive input events
    err = screen_set_window_property_iv(q->win, SCREEN_PROPERTY_SENSITIVITY, &sensitivity);
    if (err) {
        DVP_PRINT(DVP_ZONE_ERROR, "Failed to set window sensitivity to zero (%d)\n", err);
        goto leave2;
    }

    // this is not the same as global alpha.
    err = screen_set_window_property_iv(q->win, SCREEN_PROPERTY_TRANSPARENCY, &tranparency);
    if (err) {
        DVP_PRINT(DVP_ZONE_ERROR, "Failed to set window transparency to none (%d)\n", err);
        goto leave2;
    }

    err = screen_set_window_property_iv(q->win, SCREEN_PROPERTY_USAGE, &usage);
    if (err) {
            DVP_PRINT(DVP_ZONE_ERROR,
              "(%s) Failure setting USAGE window property: (%d)\n",
              __func__, errno);
        goto leave2;
    }

    err = screen_set_window_property_iv(q->win, SCREEN_PROPERTY_IDLE_MODE, &idleMode);
    if (err) {
        DVP_PRINT(DVP_ZONE_ERROR, "Failed to set window idle mode to keep awake (%d)\n", err);
        goto leave2;
    }

    err = screen_set_window_property_iv(q->win, SCREEN_PROPERTY_SIZE, size);
    if (err)
        goto leave2;

    err = screen_set_window_property_iv(q->win, SCREEN_PROPERTY_SOURCE_SIZE, bsize);
    if (err) {
            DVP_PRINT(DVP_ZONE_ERROR,
              "(%s) Failure setting SOURCE_SIZE window property: (%d)\n",
              __func__, errno);
        goto leave2;
    }

    if (width % OMX_CAMERA_ALIGNMENT)
        bsize[0] = ( (width / OMX_CAMERA_ALIGNMENT) + 1) * OMX_CAMERA_ALIGNMENT;

    err = screen_set_window_property_iv(q->win, SCREEN_PROPERTY_BUFFER_SIZE, bsize);
    if (err) {
            DVP_PRINT(DVP_ZONE_ERROR,
              "(%s) Failure setting BUFFER_SIZE window property: (%d)\n",
              __func__, errno);
        goto leave2;
    }

    for (i = 0; i < numCodes; i++) {
        if (color == codes[i].fourcc)
            pformat = codes[i].qnxscreen_fmt;
    }
    if (pformat == 0)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Unknown FOURCC 0x%x\n", color);
        goto leave2;
    }
    else
    {
        DVP_PRINT(DVP_ZONE_VIDEO, "Configuring display for fmt:%d\n", pformat);
    }

    screen_get_window_property_pv(q->win, SCREEN_PROPERTY_DISPLAY, (void *)&display);
    screen_get_display_property_iv(display, SCREEN_PROPERTY_FORMAT_COUNT, (void *)&formatCount);
    screen_get_display_property_iv(display, SCREEN_PROPERTY_FORMATS, (void *)formats);
    for (i = 0; i < formatCount; i++)
    {
        DVP_PRINT(DVP_ZONE_VIDEO, "Display %p supports format %d\n", display, formats[i]);
    }
    configured = false_e;
    for (i = 0; i < formatCount; i++)
    {
        if (formats[i] == pformat)
        {
            err = screen_set_window_property_iv(q->win, SCREEN_PROPERTY_FORMAT, &pformat);
            if (err)
            {
                DVP_PRINT(DVP_ZONE_ERROR, "screen_set_window_property_iv() returned %d when setting format %d\n", err, pformat);
                goto leave2;
            }
            else
                configured = true_e;
        }
    }
    if (configured == false_e)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Display %p does not support format %d\n", display, pformat);
        goto leave2;
    }

    err = screen_create_window_buffers(q->win, (int)count);
    if (err) {
        DVP_PRINT(DVP_ZONE_ERROR, "screen_create_window_buffers() returned %d when requesting %u buffers\n", err, count);
        goto leave2;
    }

    err = screen_get_window_property_pv(q->win, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)q->buf);
    if (err) {
        perror("screen_get_window_property(SCREEN_PROPERTY_RENDER_BUFFERS)");
        goto leave2;
    }

    q->count = count;
    q->pfmt = pformat;
    q->metrics.left = 0;
    q->metrics.top = 0;
    q->metrics.width = size[0];
    q->metrics.height = size[1];
    bitfield_init(&q->used, q->count);

    DVP_PRINT(DVP_ZONE_VIDEO, "QNX Screen Configuring %u images for %ux%u fmt:%d\n",
            count, size[0], size[1], pformat);
    for (i = 0; i < q->count; i++)
    {
        void *ptr = NULL;
        screen_get_buffer_property_pv(q->buf[i], SCREEN_PROPERTY_POINTER, &ptr);
        DVP_PRINT(DVP_ZONE_VIDEO, "\t[%u] ptr=%p\n", i, ptr);
    }
    ret = true_e;
leave:
    mutex_unlock(&q->m_lock);
    return ret;

leave2:
    screen_destroy_window(q->win);
    mutex_unlock(&q->m_lock);
    return ret;
}

#endif

