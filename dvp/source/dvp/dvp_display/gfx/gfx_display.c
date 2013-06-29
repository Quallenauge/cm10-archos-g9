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
#include <dvp/dvp_debug.h>
#include <hwc/hal_public.h>

#undef GFX_DISPLAY_WRITE_TO_FILE

typedef struct _gfx_display_t {
#ifdef GFX_DISPLAY_WRITE_TO_FILE
    FILE *fp;
    char filename[MAX_PATH];
#endif
    bitfield_t used;
    alloc_device_t *allocator;
    IMG_gralloc_module_public_t *module;
    uint32_t numBuffers;
    IMG_native_handle_t **handles;  /**< An array of image native handles */
    uint8_t **buffers;
    image_t **images;
    queue_t *returnq;
    uint32_t width;
    uint32_t height;
     int32_t stride;
    fourcc_t color;
} gfx_display_t;
#define _GFX_DISPLAY_T

#include <gfx_display.h>

typedef struct _hal_to_fourcc_t {
    int         format;
    fourcc_t    color;
} HAL_to_FOURCC_t;

static HAL_to_FOURCC_t colors[] = {
    {HAL_PIXEL_FORMAT_RGB_565,      FOURCC_RGB565},
  //{HAL_PIXEL_FORMAT_RGB_888,      FOURCC_RGB},
    {HAL_PIXEL_FORMAT_RGBA_8888,    FOURCC_RGBA},
    {HAL_PIXEL_FORMAT_BGRA_8888,    FOURCC_BGRA},
  //{HAL_PIXEL_FORMAT_YCbCr_422_SP, FOURCC_NV16},
    {HAL_PIXEL_FORMAT_YCbCr_422_I,  FOURCC_YUY2},
  //{HAL_PIXEL_FORMAT_YCrCb_420_SP, FOURCC_NV21},
    {HAL_PIXEL_FORMAT_YV12,         FOURCC_YV12},
    {0x100,                         FOURCC_NV12},
};

void gfx_display_destroy(gfx_display_t **pgfxd)
{
    if (pgfxd && *pgfxd)
    {
#ifdef GFX_DISPLAY_WRITE_TO_FILE
        fclose((*pgfxd)->fp);
#endif
        free((*pgfxd));
        pgfxd = NULL;
    }
}

gfx_display_t *gfx_display_create(char *filename)
{
    gfx_display_t *gfxd = (gfx_display_t *)calloc(1, sizeof(gfx_display_t));
    if (gfxd)
    {
#ifdef GFX_DISPLAY_WRITE_TO_FILE
        strncpy(gfxd->filename, filename, MAX_PATH);
        gfxd->fp = fopen(gfxd->filename, "w+");
        if (gfxd->fp)
        {
            DVP_PRINT(DVP_ZONE_VIDEO, "Create GFX Display on File %s\n", filename);
        }
#else
        filename = filename;
#endif
    }
    return gfxd;
}

bool_e gfx_display_free(gfx_display_t *gfxd)
{
    bool_e freed = false_e;
    if (gfxd)
    {
        uint32_t i = 0;
        for (i = 0; i < gfxd->numBuffers; i++)
        {
            gfxd->allocator->free(gfxd->allocator, (buffer_handle_t)gfxd->handles[i]);
        }
        free(gfxd->handles);
        gfxd->numBuffers = 0;
        bitfield_deinit(&gfxd->used);
        queue_destroy(gfxd->returnq);
        gralloc_close(gfxd->allocator);
        freed = true_e;
    }
    return freed;
}

bool_e gfx_display_allocate(gfx_display_t *gfxd, uint32_t numBuf, uint32_t width, uint32_t height, fourcc_t color)
{
    bool_e allocated = false_e;
    if (gfxd)
    {
        int ret = 0;
        ret = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (const struct hw_module_t **)&gfxd->module);
        DVP_PRINT(DVP_ZONE_VIDEO, "hw_get_module(%s,...) returned %d\n", GRALLOC_HARDWARE_MODULE_ID, ret);
        if (ret < 0)
        {
            DVP_PRINT(DVP_ZONE_VIDEO, "Failed to open Gralloc HW\n");
            return false_e;
        }
        DVP_PRINT(DVP_ZONE_VIDEO, "hw_module_t *%p by %s\n", gfxd->module, gfxd->module->base.common.author);

        ret = gralloc_open((const struct hw_module_t *)gfxd->module, &gfxd->allocator);
        if (ret == 0)
        {
            DVP_PRINT(DVP_ZONE_VIDEO, "gralloc_open() passed!\n");
            gfxd->numBuffers = numBuf;
            gfxd->handles = (IMG_native_handle_t **)calloc(numBuf, sizeof(IMG_native_handle_t *));
            gfxd->images = (image_t **)calloc(numBuf, sizeof(image_t*));
            if (gfxd->handles && gfxd->images)
            {
                uint32_t i = 0;
                int format = 0;
                for (i = 0; i < dimof(colors); i++)
                {
                    if (color == colors[i].color)
                        format = colors[i].format;
                }
                if (format > 0)
                {
                    DVP_PRINT(DVP_ZONE_VIDEO, "Using format %d (0x%x) which is FOURCC 0x%x\n", format, format, color);
                    gfxd->color = color;
                    gfxd->width = width;
                    gfxd->height = height;
                    for (i = 0; i < numBuf; i++)
                    {
                        int usage = GRALLOC_USAGE_HW_TEXTURE |
                                    GRALLOC_USAGE_HW_RENDER |
                                    GRALLOC_USAGE_SW_READ_RARELY | // Non-cached ?
                                    GRALLOC_USAGE_SW_WRITE_NEVER;
                        const struct native_handle_t *handle = NULL;
                        DVP_PRINT(DVP_ZONE_VIDEO, "Asking for %ux%u fmt:%d usage:%d\n", width, height, format, usage);
                        ret = gfxd->allocator->alloc(gfxd->allocator, width, height, format, usage, (buffer_handle_t *)&handle, &gfxd->stride);
                        if (ret == 0)
                        {
                            gfxd->handles[i] = (IMG_native_handle_t *)handle;
                            DVP_PRINT(DVP_ZONE_VIDEO,
                                "Allocated GRALLOC buffer handle version:%d numFD=%d fd[0]=%d fd[1]=%d %ux%u fmt=%d bpp=%u stride=%d\n",
                                gfxd->handles[i]->base.version,
                                gfxd->handles[i]->base.numFds,
                                gfxd->handles[i]->fd[0],
                                gfxd->handles[i]->fd[1],
                                gfxd->handles[i]->iWidth,
                                gfxd->handles[i]->iHeight,
                                gfxd->handles[i]->iFormat,
                                gfxd->handles[i]->uiBpp,
                                gfxd->stride);
                            allocated = true_e;
                        }
                        else
                            allocated = false_e;
                    }
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_VIDEO, "No compatible format found!\n");
                }

                if (allocated)
                {
                    bitfield_init(&gfxd->used, numBuf);
                    gfxd->returnq = queue_create(numBuf, sizeof(void *));
                }
            }
        }
        else
        {
            DVP_PRINT(DVP_ZONE_VIDEO, "Failed to open Gralloc device!\n");
        }
    }
    return allocated;
}

void gfx_display_release(gfx_display_t *gfxd, image_t **pptr, void *handle)
{
    if (gfxd)
    {
        int ret = 0;
        uint32_t idx = 0;
        for (idx = 0; idx < gfxd->numBuffers; idx++)
        {
            if (gfxd->handles[idx] == handle) {
                bitfield_rls(&gfxd->used, idx);
                ret = gfxd->module->base.unlock((const struct gralloc_module_t *)gfxd->module, (buffer_handle_t)gfxd->handles[idx]);
                if (pptr) {
                    *pptr = NULL;
                }
                image_free(&gfxd->images[idx]);
                break;
            }
        }
    }
}

image_t *gfx_display_acquire(gfx_display_t *gfxd, void **pHandle)
{
    if (gfxd)
    {
        uint32_t idx = 0;
        if (bitfield_get(&gfxd->used, &idx))
        {
            uint8_t *bufs[4] = {0,0,0,0};
            int ret = 0;
            int usage = GRALLOC_USAGE_HW_TEXTURE |
                        GRALLOC_USAGE_HW_RENDER |
                        GRALLOC_USAGE_SW_READ_RARELY |
                        GRALLOC_USAGE_SW_WRITE_NEVER;

            // save the handle with the client
            *pHandle = (void *)gfxd->handles[idx];

            // allocate an image structure,
            gfxd->images[idx] = image_allocate(gfxd->width, gfxd->height, gfxd->color);

            // by locking the memory we are mapping it to our UVA
            ret = gfxd->module->base.lock((const struct gralloc_module_t *)gfxd->module, (buffer_handle_t)gfxd->handles[idx], usage, 0, 0, gfxd->width, gfxd->height, (void **)bufs);

            DVP_PRINT(DVP_ZONE_VIDEO, "Locking %p to {%p, %p, %p, %p} (ret=%d)\n", gfxd->handles[idx], bufs[0], bufs[1], bufs[2], bufs[3], ret);

            // fill in the details in the image_t structure...
            if (gfxd->color == FOURCC_NV12) {
                gfxd->images[idx]->plane[0].ptr = bufs[0];
                gfxd->images[idx]->plane[1].ptr = bufs[1];
                gfxd->images[idx]->plane[0].ystride = gfxd->stride;
                gfxd->images[idx]->plane[1].ystride = gfxd->stride;
            } else {
                gfxd->images[idx]->plane[0].ptr = bufs[0];
                gfxd->images[idx]->plane[0].ystride = gfxd->stride;
            }
            return gfxd->images[idx];
        }
    }
    return NULL;
}

bool_e gfx_display_queue(gfx_display_t *gfxd, void *handle)
{
#ifdef GFX_DISPLAY_WRITE_TO_FILE
    if (gfxd)
    {
        size_t bytes = 0;
        uint32_t i = 0;
        // match the handle to the actually mapped pointer.
        for (i = 0; i < gfxd->numBuffers; i++)
        {
            if (handle == gfxd->handles[i])
            {
                uint32_t p,l,y = 0;
                image_t *img = gfxd->images[i];
                for (p = 0; p < img->numPlanes; p++)
                {
                    for (y = 0; y < img->plane[p].ydim/img->plane[p].yscale; y++)
                    {
                        i = (y * img->plane[p].ystride);
                        l = (img->plane[p].xdim / img->plane[p].xscale) * img->plane[p].xstride;
                        bytes += fwrite(&img->plane[p].ptr[i], 1, l, gfxd->fp);
                    }
                }
                DVP_PRINT(DVP_ZONE_VIDEO, "Wrote %zu bytes to file!\n", bytes);

                // write the pointer into the queue
                return queue_write(gfxd->returnq, true_e, &handle);
            }
        }
    }
    return false_e;
#else
    if (gfxd && handle)
    {
        // write the pointer into the queue
        return queue_write(gfxd->returnq, true_e, &handle);
    }
    else
        return false_e;
#endif
}

bool_e gfx_display_dequeue(gfx_display_t *gfxd, void **phandle)
{
    if (gfxd)
    {
        // wait for buffer from queue
        return queue_read(gfxd->returnq, true_e, phandle);
    }
    return false_e;
}

uint32_t gfx_display_length(gfx_display_t *gfxd)
{
    switch (gfxd->color)
    {
        case FOURCC_NV12:
        case FOURCC_YV12:
            return gfxd->stride * ((gfxd->height * 3) / 2);
        default:
            return gfxd->width*gfxd->height;
    }
}

#ifdef TEST

uint32_t dvp_zone_mask = 0xFFFF; // declare a local version for testing...

int main(int argc, char *argv[])
{

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

    if (argc > 2)
    {
        const uint32_t numBuf = 4;
        uint32_t count = atoi(argv[1]);
        char filename[MAX_PATH];
        gfx_display_t *gfxd = NULL;
        image_t *ptrs[numBuf];
        void *hdls[numBuf];

        strncpy(filename, argv[2], MAX_PATH);
        gfxd = gfx_display_create(filename);
        if (gfxd)
        {
            printf("Opened GFX Display!\n");
            if (gfx_display_allocate(gfxd, numBuf, 320, 240, FOURCC_NV12))
            {
                uint32_t i,j;
                printf("Allocated buffers!\n");
                for (i = 0; i < numBuf; i++)
                {
                    ptrs[i] = gfx_display_acquire(gfxd, &hdls[i]);
                    if (ptrs[i])
                    {
                        printf("Acquired Buffer %p hdl %p\n", ptrs[i], hdls[i]);
                    }
                    else
                    {
                        printf("Failed to acquire buffer %u!\n",i);
                    }
                }

                for (j = 0; j < count; j++)
                {
                    for (i = 0; i < numBuf; i++)
                    {
                        // Queue to Display
                        gfx_display_queue(gfxd, hdls[i]);

                        // Remove from Display
                        gfx_display_dequeue(gfxd, &hdls[i]);
                    }
                }

                for (i = 0; i < numBuf; i++)
                {
                    printf("Release Buffer %p\n", ptrs[i]);
                    gfx_display_release(gfxd, NULL, hdls[i]);
                }

                gfx_display_free(gfxd);
            }
            else
            {
                printf("Failed to allocate!\n");
            }
            gfx_display_destroy(&gfxd);
        }
    }
    return 0;
}

#endif
