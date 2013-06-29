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

#include <dvp/dvp.h>
#include <android/native_window_jni.h>

//#include <android/log.h>
//#define TAG "TRANSFORMERS"
//#define LOGD(string, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, string, ## __VA_ARGS__)

#define RGB_TO_Y(b, g, r, y) \
    y=(uint8_t)(((uint32_t)(30*r) + (uint32_t)(59*g) + (uint32_t)(11*b))/100)

#define RGB_TO_YUV(b, g, r, y, u, v) \
    RGB_TO_Y(b, g, r, y); \
    u=(uint8_t)(((uint32_t)(-17*r) - (uint32_t)(33*g) + (uint32_t)(50*b)+12800)/100); \
    v=(uint8_t)(((uint32_t)(50*r) - (uint32_t)(42*g) - (uint32_t)(8*b)+12800)/100)


void bgr_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input)
{
    uint8_t *outBuff = (uint8_t *)output->bits;
    uint8_t *inBuff = (uint8_t *)input->pData[0];

    uint8_t *y_line, *uv_line;
    uint8_t *rgb_index;
    uint32_t rgb_increment = 6;
    uint16_t u1,v1,u2,v2;

    for (uint32_t y = 0; y < output->height; ++ y)
    {
        y_line = outBuff + y * output->stride;
        uv_line = outBuff + (output->height + (y >> 1)) * output->stride;
        rgb_index = inBuff + y * input->y_stride;

        for (uint32_t x = 0; x < output->width; x+=2)
        {
            RGB_TO_YUV(rgb_index[0], rgb_index[1], rgb_index[2], y_line[0], u1, v1);
            RGB_TO_YUV(rgb_index[3], rgb_index[4], rgb_index[5], y_line[1], u2, v2);
            uv_line[0] = (uint8_t)((u1 + u2) >> 1);
            uv_line[1] = (uint8_t)((v1 + v2) >> 1);

            rgb_index += rgb_increment;
            y_line+=2;
            uv_line+=2;
        }
    }
}

void y800_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input)
{
    uint8_t *outBuff = (uint8_t *)output->bits;
    uint8_t *inBuff = (uint8_t *)input->pData[0];

    for (int32_t y = 0; y < output->height; y++)
    {
        uint32_t i = (y*input->y_stride);
        uint32_t j = (y*output->stride);
        memcpy(&outBuff[j], &inBuff[i], output->width);
    }
    for (int32_t y = output->height; y < output->height * 1.5; y++)
    {
        uint32_t j = (y*output->stride);
        memset(&outBuff[j], 0x80, output->width);
    }
}

void nv21_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input)
{
    uint8_t *outBuff = (uint8_t *)output->bits;
    uint8_t *inBuff = (uint8_t *)input->pData[0];

    for (int32_t y = 0; y < output->height; y++)
    {
        uint32_t i = (y*input->y_stride);
        uint32_t j = (y*output->stride);
        memcpy(&outBuff[j], &inBuff[i], output->width);
    }
    for (int32_t y = output->height; y < output->height*1.5; y++)
    {
        uint32_t i = (y*input->y_stride);
        uint32_t j = (y*output->stride);
        for (int32_t x=0; x < output->width; x+=2)
        {
            outBuff[j+x] = inBuff[i+x+1];
            outBuff[j+x+1] = inBuff[i+x];
        }
    }
}

void yuy2_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input)
{
    uint8_t *outBuff = (uint8_t *)output->bits;
    uint8_t *inBuff = (uint8_t *)input->pData[0];

    uint8_t *out_y, *out_uv, *in;

    for (uint32_t row = 0; row < output->height; ++ row)
    {
        out_y = &outBuff[row * output->stride];
        out_uv = &outBuff[(output->height + (row >> 1)) * output->stride];
        in = &inBuff[row * input->y_stride];

        for (uint32_t col = 0; col < output->width; ++ col)
        {
            *out_y = *in;

            if (row % 2)
            {
                *out_uv = (uint8_t)((uint16_t)((uint16_t)(*out_uv) + (uint16_t)(*(in+1))) >> 1);
            }
            else
            {
                *out_uv = *(in+1);
            }

            out_y ++;
            out_uv ++;
            in += 2;
        }
    }
}

void uyvy_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input)
{
    uint8_t *outBuff = (uint8_t *)output->bits;
    uint8_t *inBuff = (uint8_t *)input->pData[0];

    uint8_t *out_y, *out_uv, *in;

    for (uint32_t row = 0; row < output->height; ++ row)
    {
        out_y = &outBuff[row * output->stride];
        out_uv = &outBuff[(output->height + (row >> 1)) * output->stride];
        in = &inBuff[row * input->y_stride];

        for (uint32_t col = 0; col < output->width; ++ col)
        {
            *out_y = *(in+1);

            if (row % 2)
            {
                *out_uv = (uint8_t)((uint16_t)((uint16_t)(*out_uv) + (uint16_t)(*in)) >> 1);
            }
            else
            {
                *out_uv = *in;
            }

            out_y ++;
            out_uv ++;
            in += 2;
        }
    }
}

void rgb565_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input)
{
    uint8_t *outBuff = (uint8_t *)output->bits;
    uint8_t *inBuff = (uint8_t *)input->pData[0];

    uint8_t *y_line, *uv_line;
    uint8_t *rgb_index, b, g, r;
    uint32_t rgb_increment = 2;
    uint16_t u,v;

    for (uint32_t y = 0; y < output->height; ++ y)
    {
        y_line = outBuff + y * output->stride;
        uv_line = outBuff + (output->height + (y >> 1)) * output->stride;
        rgb_index = inBuff + y * input->y_stride;

        for (uint32_t x = 0; x < output->width; x+=2)
        {
//          b = (rgb_index[0] & 0xF8) >> 3;
//          g = ((rgb_index[0] & 0x7) << 3) | ((rgb_index[1] & 0xE0) >> 5);
//          r = (rgb_index[1] & 0x1F);
            //expand to 8bit
            b = (rgb_index[0] & 0xF8);
            g = ((rgb_index[0] & 0x7) << 5) | ((rgb_index[1] & 0xE0) >> 3);
            r = (rgb_index[1] & 0x1F) << 3;

            RGB_TO_YUV(b, g, r, y_line[0], u, v);
            rgb_index += rgb_increment;
            y_line++;

            b = (rgb_index[0] & 0xF8);
            g = ((rgb_index[0] & 0x7) << 5) | ((rgb_index[1] & 0xE0) >> 3);
            r = (rgb_index[1] & 0x1F) << 3;

            RGB_TO_YUV(b, g, r, y_line[0], uv_line[0], uv_line[1]);
            uv_line[0] = (uint8_t)((u + (uint16_t)uv_line[0]) / 2);
            uv_line[1] = (uint8_t)((v + (uint16_t)uv_line[1]) / 2);
            rgb_index += rgb_increment;

            y_line++;
            uv_line+=2;
        }
    }
}

void rgbp_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input)
{
    uint8_t *outBuff = (uint8_t *)output->bits;

    uint8_t *y_line, *uv_line;
    uint32_t rgb_index;
    uint16_t u1,v1,u2,v2;

    for (uint32_t y = 0; y < output->height; ++ y)
    {
        y_line = outBuff + y * output->stride;
        uv_line = outBuff + (output->height + (y >> 1)) * output->stride;
        rgb_index = y * input->y_stride;

        for (uint32_t x = 0; x < output->width; x+=2)
        {
            RGB_TO_YUV(input->pData[2][rgb_index],
                       input->pData[1][rgb_index],
                       input->pData[0][rgb_index],
                       y_line[0], u1, v1);
            rgb_index ++;

            RGB_TO_YUV(input->pData[2][rgb_index],
                       input->pData[1][rgb_index],
                       input->pData[0][rgb_index],
                       y_line[1], u2, v2);
            rgb_index ++;

            uv_line[0] = (uint8_t)((u1 + u2) >> 1);
            uv_line[1] = (uint8_t)((v1 + v2) >> 1);

            y_line+=2;
            uv_line+=2;
        }
    }
}

void yv12_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input)
{
    uint8_t *outBuff = (uint8_t *)output->bits;

    uint8_t *y_line, *uv_line, *y_plane;
    uint32_t uv_index;

    for (uint32_t y = 0; y < output->height; ++ y)
    {
        y_line = outBuff + y * output->stride;
        uv_line = outBuff + (output->height + (y >> 1)) * output->stride;

        y_plane = input->pData[0] + y * input->y_stride;
        uv_index = (y >> 1) * (input->y_stride / 2);

        for (uint32_t x = 0; x < output->width; x+=2)
        {
            y_line[0] = y_plane[0];
            y_line[1] = y_plane[1];

            y_line+=2;
            y_plane+=2;

            uv_line[0] = input->pData[2][uv_index];
            uv_line[1] = input->pData[1][uv_index];

            uv_line+=2;
            uv_index++;
        }
    }
}

void yv16_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input)
{
    uint8_t *outBuff = (uint8_t *)output->bits;

    uint8_t *y_line, *uv_line, *y_plane;
    uint32_t uv_index;

    for (uint32_t y = 0; y < output->height; ++ y)
    {
        y_line = outBuff + y * output->stride;
        uv_line = outBuff + (output->height + (y >> 1)) * output->stride;

        y_plane = input->pData[0] + y * input->y_stride;
        uv_index = y * (input->y_stride / 2);

        for (uint32_t x = 0; x < output->width; x+=2)
        {
            y_line[0] = y_plane[0];
            y_line[1] = y_plane[1];

            y_line+=2;
            y_plane+=2;

            if (y % 2)
            {
                uv_line[0] = (uint8_t)((uint16_t)((uint16_t)(uv_line[0]) + (uint16_t)(input->pData[2][uv_index])) >> 1);
                uv_line[1] = (uint8_t)((uint16_t)((uint16_t)(uv_line[1]) + (uint16_t)(input->pData[1][uv_index])) >> 1);
            }
            else
            {
                uv_line[0] = input->pData[2][uv_index];
                uv_line[1] = input->pData[1][uv_index];
            }

            uv_line+=2;
            uv_index++;
        }
    }
}

void yv24_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input)
{
    uint8_t *outBuff = (uint8_t *)output->bits;

    uint8_t *y_line, *uv_line;
    uint8_t *y_plane, *v_plane, *u_plane;
    uint32_t offset;

    for (uint32_t y = 0; y < output->height; ++ y)
    {
        y_line = outBuff + y * output->stride;
        uv_line = outBuff + (output->height + (y >> 1)) * output->stride;

        offset = y * input->y_stride;
        y_plane = input->pData[0] + offset;
        u_plane = input->pData[1] + offset;
        v_plane = input->pData[2] + offset;

        for (uint32_t x = 0; x < output->width; x+=2)
        {
            y_line[0] = y_plane[0];
            y_line[1] = y_plane[1];

            y_line+=2;
            y_plane+=2;

            if (y % 2)
            {
                uv_line[0] = (uint8_t)((uint16_t)((uint16_t)(uv_line[0]) + ((uint16_t)((uint16_t)(u_plane[0]) + (uint16_t)(u_plane[1])) >> 1)) >> 1);
                uv_line[1] = (uint8_t)((uint16_t)((uint16_t)(uv_line[1]) + ((uint16_t)((uint16_t)(v_plane[0]) + (uint16_t)(v_plane[1])) >> 1)) >> 1);
            }
            else
            {
                uv_line[0] = (uint8_t)((uint16_t)((uint16_t)(u_plane[0]) + (uint16_t)(u_plane[1])) >> 1);
                uv_line[1] = (uint8_t)((uint16_t)((uint16_t)(v_plane[0]) + (uint16_t)(v_plane[1])) >> 1);
            }

            uv_line+=2;
            u_plane+=2;
            v_plane+=2;
        }
    }
}

