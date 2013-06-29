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

#if defined(ANDROID) || defined(LINUX)

#include <v4l2_api.h>
#include <dvp/dvp_debug.h>

#define V4L2_NONBLOCKING
#define V4L2_POLLING
#if !(defined(BLAZE) || defined(SDP))
#define V4L2_SUPPORT_HFLIP
#endif

#ifdef V4L2_POLLING
#if defined(ANDROID)
#include <poll.h>
#elif defined(LINUX)
#include <sys/poll.h>
#ifdef POLLIN
#define POLL_IN POLLIN
#endif
#ifdef POLLPRI
#define POLL_PRI POLLPRI
#endif
#endif
#endif


#define V4L2_IOCTL(dev, flag, arg) v4l2_ioctl(dev, flag, #flag, arg)

static bool_e v4l2_ioctl(int dev, int flag, char *name, void *arg)
{
    int err = 0;
    name = name; // warnings
    DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 IOCTL %s arg: %p\n", name, arg);
    do {
        err = ioctl(dev, flag, arg);
        if (err == 0)
            return true_e;
        else
        {
            if (errno != EINTR && errno != EAGAIN)
            {
                DVP_PRINT(DVP_ZONE_ERROR, "ERROR! V4L2 IOCTL %s arg %p returned %d (errno=%d)\n", name, arg, err, errno);
#if defined(V4L2_EXTRA_DEBUG) && (DVP_DEBUG == 1)
                perror("V4L2 IOCTL");
#endif
                return false_e;
            }
            else if (errno == EINTR)
            {
                // try to call the ioctl again, we were interrupted.
                DVP_PRINT(DVP_ZONE_WARNING, "V4L2 IOCTL %s arg %p Interrupted!\n", name, arg);
            }
            else if (errno == EAGAIN)
            {
                // nothing ready now, call again later
                //DVP_PRINT(DVP_ZONE_WARNING, "V4L2 IOCTL %s arg %p Call Again Later!\n", name, arg);
                return false_e;
            }
        }
    } while (errno == EINTR);
    return false_e;
}

static void v4l2_print_fmt(struct v4l2_format *fmt)
{
    DVP_PRINT(DVP_ZONE_VIDEO, "struct v4l2_format type:%u ",fmt->type);
    if (fmt->type == V4L2_BUF_TYPE_VIDEO_OUTPUT ||
        fmt->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
    {
        DVP_PRINT(DVP_ZONE_VIDEO, "pix %ux%u fmt:0x%08x field:%u size:%u\n",
            fmt->fmt.pix.width,
            fmt->fmt.pix.height,
            fmt->fmt.pix.pixelformat,
            fmt->fmt.pix.field,
            fmt->fmt.pix.sizeimage);
    }
    else
    {
        DVP_PRINT(DVP_ZONE_VIDEO, "\n");
    }
}

static void v4l2_print_index(v4l2_api_t *v, uint32_t index)
{
    if (v && index < v->count)
    {
        struct v4l2_buffer *desc = &v->bufDesc[index];
        void *buf = v->buffers[index];
        buf = buf; // warnings
        desc = desc; // warnings
        DVP_PRINT(DVP_ZONE_VIDEO, "struct v4l2_buffer [%u] type:%u used:%u len:%u flags=0x%08x field:%u memory:%u offset:%u buf:%p\n",
                desc->index,
                desc->type,
                desc->bytesused,
                desc->length,
                desc->flags,
                desc->field,
                desc->memory,
                desc->m.offset,
                buf);
    }
}

/*
static void v4l2_print_buffer(v4l2_api_t *v, uint32_t index)
{
    if (V4L2_IOCTL(v->device, VIDIOC_QUERYBUF, &v->bufDesc[index]) == true_e)
    {
        v4l2_print_index(v, index);
    }
}

static void v4l2_print_buffers(v4l2_api_t *v)
{
    uint32_t i;
    for (i = 0; i < v->count; i++)
    {
        v4l2_print_buffer(v,i);
    }
}
*/

static void v4l2_print_formats(v4l2_api_t *v, uint32_t type)
{
    uint32_t i = 0;
    bool_e ret = false_e;
    do {
        struct v4l2_fmtdesc fmtdesc;
        memset(&fmtdesc, 0, sizeof(fmtdesc));
        fmtdesc.type = type;
        fmtdesc.index = i++;
        ret = V4L2_IOCTL(v->device, VIDIOC_ENUM_FMT, &fmtdesc);
        if (ret == true_e)
        {
            DVP_PRINT(DVP_ZONE_VIDEO, "[%u] Type: %u Flags: 0x%08x FMT: 0x%08x Desc: %s\n", fmtdesc.index, fmtdesc.type, fmtdesc.flags, fmtdesc.pixelformat, fmtdesc.description);
        }
    } while (ret == true_e);
}

void v4l2_close(v4l2_api_t **v)
{
    if ((*v)->device > 0)
    {
        close((*v)->device);
        (*v)->device = 0;
    }
    free((*v));
    *v = NULL;
}

v4l2_api_t *v4l2_open(int deviceNum, int capFlags, bool_e nonblocking)
{
    bool_e unwind = false_e;
    int devFlags = O_RDWR;
    char dev_name[40];
    v4l2_api_t *v = (v4l2_api_t *)calloc(1,sizeof(v4l2_api_t));
    if (v)
    {
        if (nonblocking)
            devFlags |= O_NONBLOCK;
        v->nonblocking = nonblocking;
        sprintf(dev_name, "/dev/video%d", deviceNum);
        DVP_PRINT(DVP_ZONE_VIDEO, "OPEN %s\n", dev_name);
        v->device = open(dev_name, devFlags);
        if (v->device > 0)
        {
            if (V4L2_IOCTL(v->device, VIDIOC_QUERYCAP, &v->capability) == false_e)
                unwind = true_e;
            else
            {
                DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 Driver: %s\n", v->capability.driver);
                DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 Card: %s\n", v->capability.card);
                DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 Bus: %s\n", v->capability.bus_info);
                DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 Caps: 0x%08x\n", v->capability.capabilities);
                if ((v->capability.capabilities & (__u32)capFlags) != (__u32)capFlags)
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR: VIDIOC_QUERYCAP indicated that driver is not capable of required devFlags 0x%x\n", capFlags);
                    unwind = true_e;
                }
            }
        }
        else
        {
            DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Failed to open %s\n", dev_name);
            unwind = true_e;
        }
    }
    if (unwind)
        v4l2_close(&v);
    return v;
}

uint32_t v4l2_search_buffer(v4l2_api_t *v, void *buffer)
{
    uint32_t index = 0;
    for (index = 0; index < v->count; index++)
    {
        if (v->buffers[index] == buffer)
            return index;
    }
    return V4L2_INVALID_INDEX;
}

void *v4l2_search_index(v4l2_api_t *v, uint32_t index)
{
    if (index < v->count)
        return v->buffers[index];
    else
        return NULL;
}

int v4l2_wait(v4l2_api_t *v)
{
    if (v)
    {
        if (v->nonblocking)
        {
            int ret = 0;
#ifdef V4L2_POLLING
            struct pollfd pds;

            pds.fd = v->device;
            pds.events = POLL_IN|POLL_PRI;
            ret = poll(&pds, 1, 1000); // 1 sec
            if (ret == 0)
            {
                DVP_PRINT(DVP_ZONE_WARNING, "V4L2 Wait Timeout\n");
                return 0;
            }
            else if (ret > 0)
            {
                if (pds.revents & (POLL_IN|POLL_PRI))
                    return 1;
                else
                    return -1;
            }
#else
            fd_set fds;
            struct timeval timer;

            FD_ZERO(&fds);
            FD_SET(v->device, &fds);
            timer.tv_sec = 2; // 2 seconds
            timer.tv_usec = 0;

            // "Highest watched file descriptor + 1"
            ret = select(v->device+1, &fds, NULL, NULL, &timer);
            if (ret == 0) // zero descriptors set
            {
                DVP_PRINT(DVP_ZONE_WARNING, "V4L2 Wait Timeout!\n");
                return 0; // TIMEOUT
            }
            else if (ret > 0) // some descriptors set!
            {
                if (FD_ISSET(v->device, &fds))
                    return 1;
                else
                    return -1;
            }
#endif
        }
        else
            return 1; // in the blocking mode, just return 1
    }
    else
        return -1; // if v == NULL, return error
    return 1;
}

bool_e v4l2_control_get(v4l2_api_t *v, int control, int *value)
{
    struct v4l2_control ctrl;
    struct v4l2_queryctrl qctrl;
    memset(&qctrl, 0, sizeof(qctrl));
    qctrl.id = control;
    if (V4L2_IOCTL(v->device, VIDIOC_QUERYCTRL, &qctrl))
    {
        if (qctrl.flags & V4L2_CTRL_TYPE_BOOLEAN ||
            qctrl.type  & V4L2_CTRL_TYPE_INTEGER)
        {
#if defined(DVP_DEBUG)
            int min = qctrl.minimum;
            int max = qctrl.maximum;
            int step = qctrl.step;
            DVP_PRINT(DVP_ZONE_VIDEO, "Ctrl: %d Min:%d Max:%d Step:%d\n",control,min,max,step);
#endif
            ctrl.id = control;
            if (V4L2_IOCTL(v->device, VIDIOC_G_CTRL, &ctrl))
            {
                *value = ctrl.value;
                return true_e;
            }
            else
                return false_e;
        }
        else
            return false_e;
    }
    else
        return false_e;
}

bool_e v4l2_control_set(v4l2_api_t *v, int control, int value)
{
    struct v4l2_control ctrl;
    struct v4l2_queryctrl qctrl;
    memset(&qctrl, 0, sizeof(qctrl));
    qctrl.id = control;
    if (V4L2_IOCTL(v->device, VIDIOC_QUERYCTRL, &qctrl))
    {
        if (qctrl.flags & V4L2_CTRL_TYPE_BOOLEAN ||
            qctrl.type  & V4L2_CTRL_TYPE_INTEGER)
        {
            int min = qctrl.minimum;
            int max = qctrl.maximum;
            int step = qctrl.step;
            DVP_PRINT(DVP_ZONE_VIDEO, "Ctrl: %d Min:%d Max:%d Step:%d\n",control,min,max,step);
            if ((min <= value) && (value <= max))
            {
                if (step > 1)
                    value = value - (value % step);
            }
            else
                value = qctrl.default_value;
            ctrl.id = control;
            ctrl.value = value;
            if (V4L2_IOCTL(v->device, VIDIOC_S_CTRL, &ctrl))
                return true_e;
            else
                return false_e;
        }
        else
            return false_e;
    }
    else
        return false_e;
}

bool_e v4l2_queue(v4l2_api_t *v, uint32_t index, uint32_t type)
{
    if (v && index < v->count)
    {
        struct v4l2_buffer *buffer = &v->bufDesc[index];
        buffer->index = index;
        buffer->timestamp.tv_sec = 0;
        buffer->timestamp.tv_usec = 0; // render ASAP
        if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT)
        {
            buffer->bytesused = buffer->length;
            buffer->field = V4L2_FIELD_NONE;
        }
        else if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
        {
            buffer->bytesused = 0;
            buffer->field = V4L2_FIELD_ANY;
        }
        v4l2_print_index(v, index);
        if (V4L2_IOCTL(v->device, VIDIOC_QBUF, buffer) == false_e)
            return false_e;
        else
        {
            mutex_lock(&v->m_lock);
            v->active++;
            v4l2_print_index(v, index);
            mutex_unlock(&v->m_lock);
            return true_e;
        }
    }
    return false_e;
}

bool_e v4l2_dequeue(v4l2_api_t *v, uint32_t *index, uint32_t type)
{
    if (v && index)
    {
        struct v4l2_buffer buffer;
        memset(&buffer, 0, sizeof(buffer));
        buffer.type = type;
        buffer.memory = V4L2_MEMORY_MMAP;
        if (V4L2_IOCTL(v->device, VIDIOC_DQBUF, &buffer) == false_e)
            return false_e;
        *index = buffer.index;
        mutex_lock(&v->m_lock);
        v->active--;
        v4l2_print_index(v, buffer.index);
        mutex_unlock(&v->m_lock);
        return true_e;
    }
    return false_e;
}

bool_e v4l2_start(v4l2_api_t *v, uint32_t type)
{
    bool_e ret = true_e;

    if (v == NULL)
        return false_e;

    mutex_lock(&v->m_lock);
    if (v->active >= 1 && v->streaming == false_e)
    {
        if (V4L2_IOCTL(v->device, VIDIOC_STREAMON, &type) == false_e)
            ret =  false_e;
        else
            v->streaming = true_e;
    }
    mutex_unlock(&v->m_lock);
    return ret;
}

bool_e v4l2_stop(v4l2_api_t *v, uint32_t type)
{
    int ret = true_e;

    if (v == NULL)
        return false_e;

    mutex_lock(&v->m_lock);
    if (v->streaming == true_e)
    {
        if (V4L2_IOCTL(v->device, VIDIOC_STREAMOFF, &type) == false_e)
            ret = false_e;
        else
            v->streaming = false_e;
    }
    mutex_unlock(&v->m_lock);
    return ret;
}

uint32_t v4l2_length(v4l2_api_t *v, uint32_t index)
{
    uint32_t len = 0;
    if (v && index < v->count)
    {
         len = v->bufDesc[index].length;
    }
    DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 Length of Buffer is %u bytes\n", len);
    return len;
}

void *v4l2_acquire(v4l2_api_t *v, uint32_t *index)
{
    void *buf = NULL;
    *index = V4L2_INVALID_INDEX;

    if (v)
    {
        mutex_lock(&v->m_lock);
        if (bitfield_get(&v->used, index) == false_e)
        {
            DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Failed to acquire V4L2 Buffer!\n");
        }
        else
        {
            buf = v->buffers[*index];
        }
        mutex_unlock(&v->m_lock);
    }
    return buf;
}

bool_e v4l2_release(v4l2_api_t *v, void *buffer, uint32_t index)
{
    int ret = true_e;
    if (v && index < v->count)
    {
        mutex_lock(&v->m_lock);
        if (index < v->count)
        {
            if (bitfield_rls(&v->used, index) == false_e)
            {
                DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Failed to release V4L2 Buffer!\n");
                ret = false_e;
            }
        }
        else if (index == V4L2_INVALID_INDEX)
        {
            // search for index of buffer
            for (index = 0; index < v->count; index++)
            {
                if (buffer == v->buffers[index])
                {
                    if (bitfield_rls(&v->used, index) == false_e)
                    {
                        DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Failed to release V4L2 Buffer!\n");
                        ret = false_e;
                    }
                    break;
                }
            }
        }
        else
        {
            DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Buffer was not a V4L2 Buffer!\n");
            ret = false_e;
        }
        mutex_unlock(&v->m_lock);
    }
    return ret;
}

bool_e v4l2_free(v4l2_api_t *v)
{
    if (v)
    {
        uint32_t i;
        mutex_lock(&v->m_lock);
        DVP_PRINT(DVP_ZONE_VIDEO, "V4L2: Freeing display memory! %u buffers, (%u active)\n", v->count, v->active);
        if (v->active) {
            DVP_PRINT(DVP_ZONE_WARNING, "V4L2: WARNING! There are still %u buffers active\n", v->active);
        }
        for (i = 0; i < v->count; i++)
        {
            if (munmap(v->buffers[i], v->bufDesc[i].length))
            {
                DVP_PRINT(DVP_ZONE_ERROR, "ERROR! Failed to unmap display buffer!\n");
            }
            else
            {
                DVP_PRINT(DVP_ZONE_VIDEO, "V4L2 Unmapped buffer %p for %u bytes (errno=%d)\n", v->buffers[i], v->bufDesc[i].length, errno);
            }
        }
        if (v->buffers)
        {
            free(v->buffers);
            v->buffers = NULL;
        }
        if (v->bufDesc)
        {
            free(v->bufDesc);
            v->bufDesc = NULL;
        }
        v->count = 0;
        v->active = 0;
        bitfield_deinit(&v->used);
        mutex_unlock(&v->m_lock);
        return true_e;
    }
    else
        return false_e;
}

bool_e v4l2_metrics(v4l2_api_t *v, uint32_t *width, uint32_t *height)
{
    if (v)
    {
        struct v4l2_format format;

        // get the current position of the overlay
        format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
        if (V4L2_IOCTL(v->device, VIDIOC_G_FMT, &format) == true_e)
        {
            *width = format.fmt.pix.width;
            *height = format.fmt.pix.height;
            return true_e;
        }
    }
    return false_e;
}

bool_e v4l2_rotate(v4l2_api_t *v, int32_t angle)
{
    struct v4l2_control ctrl;

#ifdef V4L2_CID_ROTATE
    ctrl.id = V4L2_CID_ROTATE;
#endif
#ifndef V4L2_SUPPORT_HFLIP
    V4L2_IOCTL(v->device, VIDIOC_G_CTRL, &ctrl);
    if (ctrl.value == 180)
    {
        // "fake" HFLIP is on since the kernel doesn't support it.
        if (angle == 0)
            return true_e;
        else if (angle == 90)
            ctrl.value = 270;
        else if (angle == 270)
            ctrl.value = 90;
        else if (angle == 180)
            ctrl.value = 0;
        else
            return false_e;
    }
#else
    if (angle == 0 || angle == 90 || angle == 180 || angle == 270)
        ctrl.value = angle;
    else
        ctrl.value = 0;
#endif
    return V4L2_IOCTL(v->device, VIDIOC_S_CTRL, &ctrl);
}

bool_e v4l2_mirror(v4l2_api_t *v, bool_e mirror)
{
    struct v4l2_control ctrl;

#ifdef V4L2_SUPPORT_HFLIP

    ctrl.id = V4L2_CID_HFLIP;
    ctrl.value = (mirror == true_e ? 1 : 0);
    return V4L2_IOCTL(v->device, VIDIOC_S_CTRL, &ctrl);

#else
    __u32 vflip, rotate;

    vflip = V4L2_CID_VFLIP;
    rotate = V4L2_CID_ROTATE;

    ctrl.id = vflip;
    if (mirror)
        ctrl.value = 1;
    else
        ctrl.value = 0;

    if (V4L2_IOCTL(v->device, VIDIOC_S_CTRL, &ctrl) == true_e)
    {
        ctrl.id = rotate;
        ctrl.value = 180;
        return V4L2_IOCTL(v->device, VIDIOC_S_CTRL, &ctrl);
    }
    else
        return false_e;
#endif
}

bool_e v4l2_position(v4l2_api_t *v, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    struct v4l2_format format;

    // get the current position of the overlay
    format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
    if (V4L2_IOCTL(v->device, VIDIOC_G_FMT, &format) == true_e)
    {
        // configure the position of the overlay
        format.fmt.win.w.left = x;
        format.fmt.win.w.top = y;
        format.fmt.win.w.width = w;
        format.fmt.win.w.height = h;
        return V4L2_IOCTL(v->device, VIDIOC_S_FMT, &format);
    }
    return false_e;
}

bool_e v4l2_crop(v4l2_api_t *v, uint32_t left, uint32_t top, uint32_t width, uint32_t height)
{
    struct v4l2_crop crop;

    crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    crop.c.left = left;
    crop.c.top  = top;
    crop.c.width = width;
    crop.c.height = height;
    DVP_PRINT(DVP_ZONE_VIDEO, "Setting Crop Rect to {%d,%d for %d,%d}\n",
        crop.c.left,
        crop.c.top,
        crop.c.width,
        crop.c.height);
    return V4L2_IOCTL(v->device, VIDIOC_S_CROP, &crop);
}

bool_e v4l2_transparency(v4l2_api_t *v, uint32_t flags, uint32_t colorkey)
{
    struct v4l2_framebuffer fbuf;
    memset(&fbuf, 0, sizeof(fbuf));
    if (V4L2_IOCTL(v->device, VIDIOC_G_FBUF, &fbuf) == true_e)
    {
#ifdef V4L2_FBUF_FLAG_SRC_CHROMAKEY
        if (flags & V4L2_FBUF_FLAG_SRC_CHROMAKEY)
            fbuf.flags |= V4L2_FBUF_FLAG_SRC_CHROMAKEY;
        else
            fbuf.flags &= ~V4L2_FBUF_FLAG_SRC_CHROMAKEY;
#endif
        if (flags & V4L2_FBUF_FLAG_CHROMAKEY)
            fbuf.flags |= V4L2_FBUF_FLAG_CHROMAKEY;
        else
            fbuf.flags &= ~V4L2_FBUF_FLAG_CHROMAKEY;

        // now set the format information.
        if (V4L2_IOCTL(v->device, VIDIOC_S_FBUF, &fbuf) == true_e)
        {
            // get the format info and set the color key color
            struct v4l2_format fmt;
            memset(&fmt, 0, sizeof(fmt));
            fmt.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
            if (V4L2_IOCTL(v->device, VIDIOC_G_FMT, &fmt) == true_e)
            {
                fmt.fmt.win.chromakey = colorkey & 0xFFFFFF; // 24 bit mask (for RGB 24 plane)
                return V4L2_IOCTL(v->device, VIDIOC_S_FMT, &fmt);
            }
        }
    }
    // catch all return
    return false_e;
}

typedef struct _v4l2_pix_to_fourcc_lut_t {
    uint32_t v4l2;
    uint32_t fourcc;
} V4L2_to_FOURCC_t;

V4L2_to_FOURCC_t codes[] = {
    {V4L2_PIX_FMT_UYVY,  FOURCC_UYVY},
    {V4L2_PIX_FMT_YUYV,  FOURCC_YUY2}, // capture
    {V4L2_PIX_FMT_NV12,  FOURCC_NV12},
  //{V4L2_PIX_FMT_BGR24, FOURCC_BGR}, // I don't think this is supported by capture or displays we use
    {V4L2_PIX_FMT_RGB24, FOURCC_RGB},
    {V4L2_PIX_FMT_RGB32, FOURCC_RGBA}, // we'll use a global alpha
};
uint32_t numCodes = dimof(codes);

bool_e v4l2_allocate(v4l2_api_t *v, uint32_t width, uint32_t height, uint32_t count, fourcc_t color, int fmtType)
{
    bool_e unwind = false_e;
    bool_e ret = false_e;
    uint32_t i;

    mutex_lock(&v->m_lock);

    v4l2_print_formats(v, fmtType);

    v->format.type = fmtType;
    // Get the current format...
    if (V4L2_IOCTL(v->device, VIDIOC_G_FMT, &v->format))
    {
        v4l2_print_fmt(&v->format);
    }
    v->format.fmt.pix.width = width;
    v->format.fmt.pix.height = height;
    v->format.fmt.pix.pixelformat = 0;
    v->format.fmt.pix.field = V4L2_FIELD_ANY;
    for (i = 0; i < numCodes; i++)
    {
        if (color == codes[i].fourcc)
            v->format.fmt.pix.pixelformat = codes[i].v4l2;
    }
    if (v->format.fmt.pix.pixelformat == 0)
        goto leave;

    v4l2_print_fmt(&v->format);
    if (V4L2_IOCTL(v->device, VIDIOC_S_FMT, &v->format) == false_e)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR: Failed to negotiate the format of the display!\n");
        goto leave;
    }

    v->reqbuf.type   = fmtType;
    v->reqbuf.memory = V4L2_MEMORY_MMAP;
    v->reqbuf.count  = count;
    if (V4L2_IOCTL(v->device, VIDIOC_REQBUFS, &v->reqbuf) == false_e)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR: Failed to negotiate the number of buffers for the display\n");
        goto leave;
    }
    if (v->reqbuf.count != count)
    {
        DVP_PRINT(DVP_ZONE_WARNING, "ERROR: Driver only allocated %d buffers, we wanted %d instead\n", v->reqbuf.count, count);
    }
    v->count = v->reqbuf.count;

    bitfield_init(&v->used, v->count);

    v->bufDesc = (struct v4l2_buffer *)calloc(1, sizeof(struct v4l2_buffer) * v->count);
    if (v->bufDesc)
    {
        v->buffers = calloc(1, sizeof(void *) * v->count);
        if (v->buffers)
        {
            for (i = 0; i < v->count; i++)
            {
                v->bufDesc[i].type   = v->reqbuf.type;
                v->bufDesc[i].memory = v->reqbuf.memory;
                v->bufDesc[i].index  = i;

                if (V4L2_IOCTL(v->device, VIDIOC_QUERYBUF, &v->bufDesc[i]) == false_e)
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR: Failed to get the address of buffer %u!\n",i);
                    unwind = true_e;
                    break;
                }

                if (v->bufDesc[i].flags != V4L2_BUF_FLAG_MAPPED)
                {
                    v->buffers[i] = mmap(NULL, v->bufDesc[i].length,
                                         PROT_READ | PROT_WRITE,
                                         MAP_SHARED,
                                         v->device,
                                         v->bufDesc[i].m.offset);
                    if (v->buffers[i] == MAP_FAILED)
                    {
                        DVP_PRINT(DVP_ZONE_ERROR, "ERROR: mmap failed\n");
                        unwind = true_e;
                        break;
                    }
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "Buffer already mmaped!\n");
                }
                v4l2_print_index(v,i);
            }
            DVP_PRINT(DVP_ZONE_VIDEO, "V4L2: %u buffers allocated and mapped\n", v->count);
            ret = true_e;
        }
        else
            unwind = true_e;
    }
    else
        unwind = true_e;
leave:
    mutex_unlock(&v->m_lock);
    if (unwind) {
        v4l2_free(v);
        ret = false_e; // make sure we're clear that we failed.
    }
    return ret;

}

#endif

