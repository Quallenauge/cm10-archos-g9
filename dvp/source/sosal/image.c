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

#include <sosal/types.h>
#include <sosal/image.h>
#include <sosal/debug.h>

//******************************************************************************
// IMAGE API
//******************************************************************************

static fourcc_t supportedColors[] = {
    FOURCC_RGB,
    FOURCC_BGR,
    FOURCC_Y800,
    FOURCC_UYVY,
    FOURCC_YUY2,
    FOURCC_NV12,
    FOURCC_RGBP,
    FOURCC_YU24,
    FOURCC_YV24,
};

uint8_t saturate_to_uint8(int32_t a)
{
    if (a > 255)
        a = 255;
    if (a < 0)
        a = 0;
    return (uint8_t)a;
}

bool_e image_query_format(fourcc_t color)
{
    uint32_t i = 0;
    for (i = 0; i < dimof(supportedColors); i++)
        if (supportedColors[i] == color)
            return true_e;
    if (i == dimof(supportedColors))
        return false_e;
    else
        return true_e;
}

void yuv_to_rgb_pixel(uint8_t y, uint8_t cb, uint8_t cr, uint8_t *r, uint8_t *g, uint8_t *b)
{
    *r = saturate_to_uint8((int32_t)(1.164*(double)((int32_t)y - 16) + 1.596*(double)((int32_t)cb - 128)));
    *g = saturate_to_uint8((int32_t)(1.164*(double)((int32_t)y - 16) - 0.813*(double)((int32_t)cr - 128) - 0.391*(double)((int32_t)cb - 128)));
    *b = saturate_to_uint8((int32_t)(1.164*(double)((int32_t)y - 16) + 2.018*(double)((int32_t)cr - 128)));
}

void rgb_to_yuv_pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t *y, uint8_t *u, uint8_t *v)
{
    *y = saturate_to_uint8(((int32_t)(0.257*(double)r + 0.504*(double)g + 0.098*(double)b) + 16));
    *u = saturate_to_uint8(((int32_t)(-0.148*(double)r - 0.291*(double)g + 0.439*(double)b) + 128));
    *v = saturate_to_uint8(((int32_t)(0.439*(double)r - 0.368*(double) - 0.071*(double)b) + 128));
}

void image_free(image_t **pimg)
{
    if (pimg)
    {
        image_t *img = *pimg;
        if (img)
        {
            memset(img, 0, sizeof(image_t));
            free(img);
        }
        *pimg = NULL;
    }
}

image_t *image_allocate(uint32_t width, uint32_t height, fourcc_t color)
{
    image_t *img = (image_t *)calloc(1, sizeof(image_t));
    if (img)
    {
        uint32_t p;
        img->color = color;
        switch (color)
        {
            // easy planar formats
            case FOURCC_RGBP:
            case FOURCC_YU24:
            case FOURCC_YV24:
                img->numPlanes = 3;
                for (p = 0; p < img->numPlanes; p++)
                {
                    img->plane[p].xdim = width;
                    img->plane[p].ydim = height;
                    img->plane[p].xstep = 1;
                    img->plane[p].xscale = 1;
                    img->plane[p].yscale = 1;
                    img->plane[p].xstride = 1;
                    img->plane[p].ystride = img->plane[p].xdim * img->plane[p].xstride;
                    img->plane[p].numBytes = img->plane[p].ydim * img->plane[p].ystride;
                    //img->plane[p].ptr = calloc(1, img->plane[p].numBytes);
                }
                break;
            case FOURCC_BGR:
            case FOURCC_RGB:
                img->numPlanes = 1;
                img->plane[0].xdim = width;
                img->plane[0].ydim = height;
                img->plane[0].xstep = 1;
                img->plane[0].xscale = 1;
                img->plane[0].yscale = 1;
                img->plane[0].xstride = 3;
                img->plane[0].ystride = img->plane[0].xdim * img->plane[0].xstride;
                img->plane[0].numBytes = img->plane[0].ydim * img->plane[0].ystride;
                //img->plane[0].ptr = calloc(1, img->plane[0].numBytes);
                break;
            case FOURCC_UYVY:
            case FOURCC_YUY2:
                img->numPlanes = 1;
                img->plane[0].xdim = width;
                img->plane[0].ydim = height;
                img->plane[0].xstep = 2; // macro pixel!
                img->plane[0].xscale = 1;
                img->plane[0].yscale = 1;
                img->plane[0].xstride = 2;
                img->plane[0].ystride = img->plane[0].xdim * img->plane[0].xstride;
                img->plane[0].numBytes = img->plane[0].ydim * img->plane[0].ystride;
                //img->plane[0].ptr = calloc(1, img->plane[0].numBytes);
                break;
            case FOURCC_Y800:
                img->numPlanes = 1;
                img->plane[0].xdim = width;
                img->plane[0].ydim = height;
                img->plane[0].xstep = 1;
                img->plane[0].xscale = 1;
                img->plane[0].yscale = 1;
                img->plane[0].xstride = 1;
                img->plane[0].ystride = img->plane[0].xdim * img->plane[0].xstride;
                img->plane[0].numBytes = img->plane[0].ydim * img->plane[0].ystride;
                //img->plane[0].ptr = calloc(1, img->plane[0].numBytes);
                break;
            case FOURCC_NV12: /// @todo should this remain in a single buffer, just like DVP_Image_t?
                img->numPlanes = 2;
                img->plane[0].xdim = width;
                img->plane[0].ydim = height;
                img->plane[0].xstep = 1;
                img->plane[0].xscale = 1;
                img->plane[0].yscale = 1;
                img->plane[0].xstride = 1;
                img->plane[0].ystride = img->plane[0].xdim * img->plane[0].xstride;
                img->plane[0].numBytes = img->plane[0].ydim * img->plane[0].ystride;
                img->plane[1].xdim = width;
                img->plane[1].ydim = height;
                img->plane[1].xstep = 1;
                img->plane[1].xscale = 2;
                img->plane[1].yscale = 2;
                img->plane[1].xstride = 2;
                img->plane[1].ystride = img->plane[1].xdim * img->plane[1].xstride;
                img->plane[1].numBytes = img->plane[1].ydim * img->plane[1].ystride;
                //img->plane[0].ptr = calloc(1, img->plane[0].numBytes + img->plane[1].numBytes);
                //img->plane[1].ptr = &img->plane[0].ptr[img->plane[0].ydim * img->plane[0].ystride];
                break;
            case FOURCC_IYUV:
            case FOURCC_YV12:
                img->numPlanes = 3;
                img->plane[0].xdim = width;
                img->plane[0].ydim = width;
                img->plane[0].xstep = 1;
                img->plane[0].xscale = 1;
                img->plane[0].yscale = 1;
                img->plane[0].xstride = 1;
                img->plane[0].ystride = img->plane[0].xdim * img->plane[0].xstride;
                img->plane[0].numBytes = img->plane[0].ydim * img->plane[0].ystride;
                img->plane[1].xdim = width/2;
                img->plane[1].ydim = width/2;
                img->plane[1].xstep = 1;
                img->plane[1].xscale = 2;
                img->plane[1].yscale = 2;
                img->plane[1].xstride = 1;
                img->plane[1].ystride = img->plane[1].xdim * img->plane[1].xstride;
                img->plane[1].numBytes = img->plane[1].ydim * img->plane[1].ystride;
                img->plane[2].xdim = width/2;
                img->plane[2].ydim = width/2;
                img->plane[2].xstep = 1;
                img->plane[2].xscale = 2;
                img->plane[2].yscale = 2;
                img->plane[2].xstride = 1;
                img->plane[2].ystride = img->plane[2].xdim * img->plane[2].xstride;
                img->plane[2].numBytes = img->plane[2].ydim * img->plane[2].ystride;
                break;
        }
    }
    return img;
}

void image_unback(image_t *img)
{
    if (img)
    {
        uint32_t p;
        if (img->color != FOURCC_NV12)
        {
            for (p = 0; p < img->numPlanes; p++)
            {
                free(img->plane[p].ptr);
            }
        }
        else
        {
            // NV12 has it's second plane as a calcuated pointer from the first
            free(img->plane[0].ptr);
        }
    }
}

bool_e image_back(image_t *img)
{
    bool_e allocated = true_e;
    if (img)
    {
        uint32_t p;
        if (img->color != FOURCC_NV12)
        {
            for (p = 0; p < img->numPlanes; p++) {
                img->plane[p].ptr = calloc(1, img->plane[p].numBytes);
                if (img->plane[p].ptr == NULL)
                    allocated = false_e;
            }
        }
        else
        {
            img->plane[0].ptr = calloc(1, img->plane[0].numBytes + img->plane[1].numBytes);
            if (img->plane[0].ptr == NULL)
                allocated = false_e;
            else
                img->plane[1].ptr = &img->plane[0].ptr[img->plane[0].ydim * img->plane[0].ystride];
        }
    }
    return allocated;
}

/**
 * If RGB data, a=R, b=G, c=B
 * If YUV data, a=Y, b=U, c=V
 */
void image_fill(image_t *img, uint8_t a, uint8_t b, uint8_t c)
{
    uint32_t x,y,p;
    uint8_t *ptr = NULL;
    if (img->color == FOURCC_BGR)
    {
        for (p = 0; p < img->numPlanes; p++)
        {
            for (y = 0; y < img->plane[p].ydim; y++)
            {
                for (x = 0; x < img->plane[p].xdim; x+=img->plane[p].xstep)
                {
                    ptr = image_pixel(img, p, x, y);
                    ptr[0] = c;
                    ptr[1] = b;
                    ptr[2] = a;
                }
            }
        }
    }
    else if (img->color == FOURCC_RGB)
    {
        for (p = 0; p < img->numPlanes; p++)
        {
            for (y = 0; y < img->plane[p].ydim; y++)
            {
                for (x = 0; x < img->plane[p].xdim; x+=img->plane[p].xstep)
                {
                    ptr = image_pixel(img, p, x, y);
                    ptr[0] = a;
                    ptr[1] = b;
                    ptr[2] = c;
                }
            }
        }
    }
    else if (img->color == FOURCC_UYVY)
    {
        for (p = 0; p < img->numPlanes; p++)
        {
            for (y = 0; y < img->plane[p].ydim; y++)
            {
                for (x = 0; x < img->plane[p].xdim; x+=img->plane[p].xstep)
                {
                    ptr = image_pixel(img, p, x, y);
                    ptr[0] = b;
                    ptr[1] = a;
                    ptr[2] = c;
                    ptr[3] = a;
                }
            }
        }
    }
    else if (img->color == FOURCC_YUY2)
    {
        for (p = 0; p < img->numPlanes; p++)
        {
            for (y = 0; y < img->plane[p].ydim; y++)
            {
                for (x = 0; x < img->plane[p].xdim; x+=img->plane[p].xstep)
                {
                    ptr = image_pixel(img, p, x, y);
                    ptr[0] = a;
                    ptr[1] = b;
                    ptr[2] = a;
                    ptr[3] = c;
                }
            }
        }
    }
    else if (img->color == FOURCC_NV12)
    {
        for (y = 0; y < img->plane[0].ydim; y++)
        {
            for (x = 0; x < img->plane[0].xdim; x+=img->plane[0].xstep)
            {
                ptr = image_pixel(img, 0, x, y);
                *ptr = a;
            }
        }
        for (y = 0; y < img->plane[1].ydim; y++)
        {
            for (x = 0; x < img->plane[1].xdim; x+=img->plane[1].xstep)
            {
                ptr = image_pixel(img, 1, x, y);
                ptr[0] = b;
                ptr[1] = c;
            }
        }
    }
    else if (img->color == FOURCC_Y800)
    {
        for (y = 0; y < img->plane[0].ydim; y++)
        {
            for (x = 0; x < img->plane[0].xdim; x+=img->plane[0].xstep)
            {
                ptr = image_pixel(img, 0, x, y);
                *ptr = a;
            }
        }
    }
}

uint32_t image_copy(image_t *dst, image_t *src)
{
    if (dst && src && dst->color == src->color)
    {
        uint32_t p,y,i,l,b=0;
        for (p = 0; p < dst->numPlanes && p < src->numPlanes; p++)
        {
            for (y = 0; y < dst->plane[p].ydim; y++)
            {
                l = src->plane[p].xstride * src->plane[p].xdim;
                i = (y * src->plane[p].ystride);
                memcpy(&dst->plane[p].ptr[i], &src->plane[p].ptr[i], l);
                b += l;
            }
        }
        return b;
    }
    else
        return 0;
}

void image_convert(image_t *dst, image_t *src)
{
    if (dst && src)
    {
        if (dst->color == src->color)
            image_copy(dst, src);
        else
        {
            uint32_t x,y;
            if (dst->color == FOURCC_BGR && src->color == FOURCC_UYVY)
            {
                for (y = 0; y < src->plane[0].ydim; y++) {
                    for (x = 0; x < src->plane[0].xdim; x+=src->plane[0].xstep) {
                        uint8_t *uyvy = image_pixel(src, 0, x, y);
                        uint8_t *bgr  = image_pixel(dst, 0, x, y);
                        yuv_to_rgb_pixel(uyvy[1], uyvy[0], uyvy[2],
                                         &bgr[2], &bgr[1], &bgr[0]);
                        yuv_to_rgb_pixel(uyvy[3], uyvy[0], uyvy[2],
                                         &bgr[5], &bgr[4], &bgr[3]);

                    }
                }
            }
            else if (dst->color == FOURCC_BGR && src->color == FOURCC_YUY2)
            {
                for (y = 0; y < src->plane[0].ydim; y++) {
                    for (x = 0; x < src->plane[0].xdim; x+=src->plane[0].xstep) {
                        uint8_t *yuyv = image_pixel(src, 0, x, y);
                        uint8_t *bgr  = image_pixel(dst, 0, x, y);
                        yuv_to_rgb_pixel(yuyv[0], yuyv[1], yuyv[3],
                                         &bgr[2], &bgr[1], &bgr[0]);
                        yuv_to_rgb_pixel(yuyv[2], yuyv[1], yuyv[3],
                                         &bgr[5], &bgr[4], &bgr[3]);
                    }
                }
            }
            else if (dst->color == FOURCC_BGR && src->color == FOURCC_NV12)
            {
                for (y = 0; y < src->plane[0].ydim; y++) {
                    for (x = 0; x < src->plane[0].xdim; x+=src->plane[0].xstep) {
                        uint8_t *luma = image_pixel(src, 0, x, y);
                        uint8_t *uv   = image_pixel(src, 1, x, y);
                        uint8_t *bgr  = image_pixel(dst, 0, x, y);
                        yuv_to_rgb_pixel(luma[0], uv[0], uv[1],
                                         &bgr[2], &bgr[1], &bgr[0]);
                    }
                }
            }
            else if (dst->color == FOURCC_UYVY && src->color == FOURCC_NV12)
            {
                for (y = 0; y < src->plane[0].ydim; y++) {
                    for (x = 0; x < src->plane[0].xdim; x+=dst->plane[0].xstep) {
                        uint8_t *luma = image_pixel(src, 0, x, y);
                        uint8_t *uv   = image_pixel(src, 1, x, y);
                        uint8_t *uyvy = image_pixel(dst, 0, x, y);
                        uyvy[0] = uv[0];
                        uyvy[1] = luma[0];
                        uyvy[2] = uv[1];
                        uyvy[3] = luma[1];
                    }
                }
            }
            else if (dst->color == FOURCC_BGR && (src->color == FOURCC_YU24 || src->color == FOURCC_YV24))
            {
                uint32_t p1 = 1, p2 = 2;
                if (src->color == FOURCC_YV24) {
                    p1 = 2;
                    p2 = 1;
                }
                for (y = 0; y < src->plane[0].ydim; y++) {
                    for (x = 0; x < src->plane[0].xdim; x+=src->plane[0].xstep) {
                        uint8_t *luma = image_pixel(src, 0, x, y);
                        uint8_t *cb   = image_pixel(src, p1, x, y);
                        uint8_t *cr   = image_pixel(src, p2, x, y);
                        uint8_t *bgr  = image_pixel(dst, 0, x, y);
                        yuv_to_rgb_pixel(luma[0], cb[0], cr[0],
                                         &bgr[2], &bgr[1], &bgr[0]);
                    }
                }
            }
            else if (dst->color == FOURCC_UYVY && (src->color == FOURCC_YU24 || src->color == FOURCC_YV24))
            {
                uint32_t p1 = 1, p2 = 2;
                if (src->color == FOURCC_YV24) {
                    p1 = 2;
                    p2 = 1;
                }
                for (y = 0; y < src->plane[0].ydim; y++) {
                    for (x = 0; x < src->plane[0].xdim; x+=dst->plane[0].xstep) {
                        uint8_t *luma = image_pixel(src, 0, x, y);
                        uint8_t *cb   = image_pixel(src, p1, x, y);
                        uint8_t *cr   = image_pixel(src, p2, x, y);
                        uint8_t *uyvy = image_pixel(dst, 0, x, y);
                        uyvy[0] = (cb[0] + cb[1])/2;
                        uyvy[1] = luma[0];
                        uyvy[2] = (cr[0] + cr[1])/2;
                        uyvy[3] = luma[1];
                    }
                }
            }
            else if (dst->color == FOURCC_BGR && src->color == FOURCC_Y800)
            {
                for (y = 0; y < src->plane[0].ydim; y++) {
                    for (x = 0; x < src->plane[0].xdim; x+=src->plane[0].xstep) {
                        uint8_t *luma = image_pixel(src, 0, x, y);
                        uint8_t *bgr  = image_pixel(dst, 0, x, y);
                        yuv_to_rgb_pixel(luma[0], 128, 128,
                                         &bgr[2], &bgr[1], &bgr[0]);
                    }
                }
            }
            else if (dst->color == FOURCC_BGR && src->color == FOURCC_RGB)
            {
                for (y = 0; y < src->plane[0].ydim; y++) {
                    for (x = 0; x < src->plane[0].xdim; x+=src->plane[0].xstep) {
                        uint8_t *rgb = image_pixel(src, 0, x, y);
                        uint8_t *bgr = image_pixel(dst, 0, x, y);
                        bgr[0] = rgb[2];
                        bgr[1] = rgb[1];
                        bgr[2] = rgb[0];
                    }
                }
            }
            else if (dst->color == FOURCC_UYVY && src->color == FOURCC_RGB)
            {
                for (y = 0; y < src->plane[0].ydim; y++) {
                    for (x = 0; x < src->plane[0].xdim; x+=src->plane[0].xstep*2) { //macro pixel
                        uint8_t *rgb = image_pixel(src, 0, x, y);
                        uint8_t *uyvy = image_pixel(dst, 0, x, y);
                        uint8_t u0,v0,u1,v1;
                        uint32_t ux, vx;
                        rgb_to_yuv_pixel(rgb[0], rgb[1], rgb[2],
                                         &uyvy[1], &u0, &v0);
                        rgb_to_yuv_pixel(rgb[3], rgb[4], rgb[5],
                                         &uyvy[3], &u1, &v1);
                        ux = (u0&0xFF); ux += (u1&0xFF); ux >>= 1;
                        vx = (v0&0xFF); vx += (v1&0xFF); vx >>= 1;
                        uyvy[0] = saturate_to_uint8(ux);
                        uyvy[2] = saturate_to_uint8(vx);
                        //printf("u0 = %u u1 = %u ux = %u\n", u0, u1, ux);
                        //printf("v0 = %u v1 = %u vx = %u\n", v0, v1, vx);
                    }
                }
            }
            else if (dst->color == FOURCC_NV12 && src->color == FOURCC_RGB)
            {
                for (y = 0; y < src->plane[0].ydim; y++) {
                    for (x = 0; x < src->plane[0].xdim; x+=src->plane[0].xstep) {
                        uint8_t *rgb = image_pixel(src, 0, x, y);
                        uint8_t *luma = image_pixel(dst, 0, x, y);
                        uint8_t *uv = image_pixel(dst, 1, x, y);
                        rgb_to_yuv_pixel(rgb[0], rgb[1], rgb[2],
                                         &luma[0], &uv[0], &uv[1]);
                    }
                }
            }
            else if (dst->color == FOURCC_UYVY && src->color == FOURCC_BGR)
            {
                for (y = 0; y < src->plane[0].ydim; y++) {
                    for (x = 0; x < src->plane[0].xdim; x+=src->plane[0].xstep*2) { //macro pixel
                        uint8_t *bgr = image_pixel(src, 0, x, y);
                        uint8_t *uyvy = image_pixel(dst, 0, x, y);
                        uint8_t u0,v0,u1,v1;
                        uint32_t ux, vx;
                        rgb_to_yuv_pixel(bgr[2], bgr[1], bgr[0],
                                         &uyvy[1], &u0, &v0);
                        rgb_to_yuv_pixel(bgr[5], bgr[4], bgr[3],
                                         &uyvy[3], &u1, &v1);
                        ux = (u0&0xFF); ux += (u1&0xFF); ux >>= 1;
                        vx = (v0&0xFF); vx += (v1&0xFF); vx >>= 1;
                        uyvy[0] = saturate_to_uint8(ux);
                        uyvy[2] = saturate_to_uint8(vx);
                    }
                }
            }
            else if (dst->color == FOURCC_NV12 && src->color == FOURCC_BGR)
            {
                for (y = 0; y < src->plane[0].ydim; y+=2) {
                    for (x = 0; x < src->plane[0].xdim; x+=2) {

                        uint8_t *bgr0 = image_pixel(src, 0, x, y);
                        uint8_t *bgr1 = image_pixel(dst, 0, x+1, y);
                        uint8_t *bgr2 = image_pixel(dst, 0, x, y+1);
                        uint8_t *bgr3 = image_pixel(dst, 0, x+1, y+1);

                        uint8_t *l0 = image_pixel(dst, 0, x, y);
                        uint8_t *l1 = image_pixel(dst, 0, x+1, y);
                        uint8_t *l2 = image_pixel(dst, 0, x, y+1);
                        uint8_t *l3 = image_pixel(dst, 0, x+1, y+1);

                        uint8_t *uv = image_pixel(dst, 1, x, y);

                        uint8_t u[4];
                        uint8_t v[4];

                        rgb_to_yuv_pixel(bgr0[2], bgr0[1], bgr0[0],
                                         &l0[0], &u[0], &v[0]);
                        rgb_to_yuv_pixel(bgr1[2], bgr1[1], bgr1[0],
                                         &l1[0], &u[1], &v[1]);
                        rgb_to_yuv_pixel(bgr2[2], bgr2[1], bgr2[0],
                                         &l2[0], &u[2], &v[2]);
                        rgb_to_yuv_pixel(bgr2[2], bgr3[1], bgr3[0],
                                         &l3[0], &u[3], &v[3]);

                        uv[0] = (u[0] + u[1] + u[2] + u[3])/4;
                        uv[1] = (v[0] + v[1] + v[2] + v[3])/4;

                    }
                }
            }
            else if (dst->color == FOURCC_IYUV && src->color == FOURCC_BGR)
            {
                for (y = 0; y < src->plane[0].ydim; y+=2) {
                    for (x = 0; x < src->plane[0].xdim; x+=2) {

                        uint8_t *bgr0 = image_pixel(src, 0, x, y);
                        uint8_t *bgr1 = image_pixel(dst, 0, x+1, y);
                        uint8_t *bgr2 = image_pixel(dst, 0, x, y+1);
                        uint8_t *bgr3 = image_pixel(dst, 0, x+1, y+1);

                        uint8_t *l0 = image_pixel(dst, 0, x, y);
                        uint8_t *l1 = image_pixel(dst, 0, x+1, y);
                        uint8_t *l2 = image_pixel(dst, 0, x, y+1);
                        uint8_t *l3 = image_pixel(dst, 0, x+1, y+1);

                        uint8_t *us = image_pixel(dst, 1, x, y);
                        uint8_t *vs = image_pixel(dst, 2, x, y);

                        uint8_t u[4];
                        uint8_t v[4];

                        rgb_to_yuv_pixel(bgr0[2], bgr0[1], bgr0[0],
                                         &l0[0], &u[0], &v[0]);
                        rgb_to_yuv_pixel(bgr1[2], bgr1[1], bgr1[0],
                                         &l1[0], &u[1], &v[1]);
                        rgb_to_yuv_pixel(bgr2[2], bgr2[1], bgr2[0],
                                         &l2[0], &u[2], &v[2]);
                        rgb_to_yuv_pixel(bgr2[2], bgr3[1], bgr3[0],
                                         &l3[0], &u[3], &v[3]);

                        us[0] = (u[0] + u[1] + u[2] + u[3])/4;
                        vs[1] = (v[0] + v[1] + v[2] + v[3])/4;

                    }
                }
            }
            else if (dst->color == FOURCC_YV12 && src->color == FOURCC_BGR)
            {
                for (y = 0; y < src->plane[0].ydim; y+=2) {
                    for (x = 0; x < src->plane[0].xdim; x+=2) {

                        uint8_t *bgr0 = image_pixel(src, 0, x, y);
                        uint8_t *bgr1 = image_pixel(dst, 0, x+1, y);
                        uint8_t *bgr2 = image_pixel(dst, 0, x, y+1);
                        uint8_t *bgr3 = image_pixel(dst, 0, x+1, y+1);

                        uint8_t *l0 = image_pixel(dst, 0, x, y);
                        uint8_t *l1 = image_pixel(dst, 0, x+1, y);
                        uint8_t *l2 = image_pixel(dst, 0, x, y+1);
                        uint8_t *l3 = image_pixel(dst, 0, x+1, y+1);

                        uint8_t *us = image_pixel(dst, 2, x, y);
                        uint8_t *vs = image_pixel(dst, 1, x, y);

                        uint8_t u[4];
                        uint8_t v[4];

                        rgb_to_yuv_pixel(bgr0[2], bgr0[1], bgr0[0],
                                         &l0[0], &u[0], &v[0]);
                        rgb_to_yuv_pixel(bgr1[2], bgr1[1], bgr1[0],
                                         &l1[0], &u[1], &v[1]);
                        rgb_to_yuv_pixel(bgr2[2], bgr2[1], bgr2[0],
                                         &l2[0], &u[2], &v[2]);
                        rgb_to_yuv_pixel(bgr2[2], bgr3[1], bgr3[0],
                                         &l3[0], &u[3], &v[3]);

                        us[0] = (u[0] + u[1] + u[2] + u[3])/4;
                        vs[1] = (v[0] + v[1] + v[2] + v[3])/4;

                    }
                }
            }
            else if (dst->color == FOURCC_YU24 && src->color == FOURCC_NV12)
            {
                for (y = 0; y < src->plane[0].ydim; y+=2) {
                    for (x = 0; x < src->plane[0].xdim; x+=src->plane[0].xstep*2) {
                        int32_t str = src->plane[0].ystride;
                        uint8_t *l0 = image_pixel(src, 0, x, y);
                        uint8_t *uv = image_pixel(src, 1, x, y);
                        uint8_t *lo = image_pixel(dst, 0, x, y);
                        uint8_t *uo = image_pixel(dst, 1, x, y);
                        uint8_t *vo = image_pixel(dst, 2, x, y);
                        lo[0] = (l0[0] + l0[1] + l0[str] + l0[str+1]) >> 2;
                        uo[0] = uv[0];
                        vo[0] = uv[1];
                    }
                }
            }
            else if (dst->color == FOURCC_YV24 && src->color == FOURCC_NV12)
            {
                for (y = 0; y < src->plane[0].ydim; y+=2) {
                    for (x = 0; x < src->plane[0].xdim; x+=src->plane[0].xstep*2) {
                        int32_t str = src->plane[0].ystride;
                        uint8_t *l0 = image_pixel(src, 0, x, y);
                        uint8_t *uv = image_pixel(src, 1, x, y);
                        uint8_t *lo = image_pixel(dst, 0, x, y);
                        uint8_t *uo = image_pixel(dst, 2, x, y);
                        uint8_t *vo = image_pixel(dst, 1, x, y);
                        lo[0] = (l0[0] + l0[1] + l0[str] + l0[str+1]) >> 2;
                        uo[0] = uv[0];
                        vo[0] = uv[1];
                    }
                }
            }
            else if (dst->color == FOURCC_RGBP && (src->color == FOURCC_YU24 || src->color == FOURCC_YV24))
            {
                uint32_t p1 = 1, p2 = 2;
                if (src->color == FOURCC_YV24) {
                    p1 = 2;
                    p2 = 1;
                }
                for (y = 0; y < src->plane[0].ydim; y++) {
                    for (x = 0; x < src->plane[0].xdim; x+=src->plane[0].xstep) {
                        uint8_t *luma = image_pixel(src, 0, x, y);
                        uint8_t *cb   = image_pixel(src, p1, x, y);
                        uint8_t *cr   = image_pixel(src, p2, x, y);
                        uint8_t *r    = image_pixel(dst, 0, x, y);
                        uint8_t *g    = image_pixel(dst, 1, x, y);
                        uint8_t *b    = image_pixel(dst, 2, x, y);
                        yuv_to_rgb_pixel(luma[0], cb[0], cr[0],
                                         r, g, b);
                    }
                }
            }
            else if (dst->color == FOURCC_UYVY && src->color == FOURCC_Y800)
            {
                for (y = 0; y < dst->plane[0].ydim; y++) {
                    for (x = 0; x < dst->plane[0].xdim; x+=dst->plane[0].xstep) {
                        uint8_t *luma = image_pixel(src, 0, x, y);
                        uint8_t *uyvy = image_pixel(dst, 0, x, y);
                        uyvy[0] = 0x80;
                        uyvy[1] = luma[0];
                        uyvy[2] = 0x80;
                        uyvy[3] = luma[1];
                    }
                }
            }
            else
            {
                SOSAL_PRINT(SOSAL_ZONE_ERROR, "NO CONVERSION FROM 0x%08x to 0x%08x\n", src->color, dst->color);
            }
        }
    }
}

void image_print(image_t *img)
{
    uint32_t p = 0;
    SOSAL_PRINT(SOSAL_ZONE_IMAGE, "image_t %p %s %u planes\n", img, fourcctostr(img->color), img->numPlanes);
    for (p = 0; p < img->numPlanes; p++)
    {
        SOSAL_PRINT(SOSAL_ZONE_IMAGE, "\t%p %ux%u step:%u, stride:%dx%d\n",
            img->plane[p].ptr,
            img->plane[p].xdim,
            img->plane[p].ydim,
            img->plane[p].xstep,
            img->plane[p].xstride,
            img->plane[p].ystride);
    }
}


