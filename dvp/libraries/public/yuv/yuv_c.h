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

#ifndef _YUV_C_H_
#define _YUV_C_H_

/*! \file
 * \brief Defines the "C" model macros and defintions for YUV.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <stdint.h>

#define SIZEOF_PIXEL_YUV420 (1) // this is for each individual plane
#define SIZEOF_IMAGE_YUV420(w,h)    (((w)*(h)*3)/2)

typedef union _yuv4444 {
    struct {
        uint8_t V;
        uint8_t U;
        uint8_t Y;
        uint8_t A;
    } VUYA;     // FOURCC_VUYA
    struct {
        uint8_t A;
        uint8_t Y;
        uint8_t U;
        uint8_t V;
    } AYUV;     // FOURCC_AYUV
    uint32_t packed;
} YUV4444;

#define SIZEOF_PIXEL_YUV4444         (4)
#define SIZEOF_IMAGE_YUV4444(w,h)    ((w)*(h)*SIZEOF_PIXEL_YUV4444)

typedef union _yuv422 {
    struct {
        uint8_t   U0;
        uint8_t   Y0;
        uint8_t   V0;
        uint8_t   Y1;
    } UYVY; // and Y422, UYNV and HDYC
    struct {
        uint8_t   V0;
        uint8_t   Y0;
        uint8_t   U0;
        uint8_t   Y1;
    } VYUY;
    struct {
        uint8_t   Y0;
        uint8_t   U0;
        uint8_t   Y1;
        uint8_t   V0;
    } YUYV; // and YUNV, V422, YUY2
    struct {
        uint8_t   Y0;
        uint8_t   V0;
        uint8_t   Y1;
        uint8_t   U0;
    } YVYU;
    uint32_t packed;
} YUV422;

#define SIZEOF_PIXEL_YUV422   (2)   // since this is a macropixel format, we just want the pixel itself.
#define SIZEOF_IMAGE_YUV422(w,h)    ((w)*(h)*SIZEOF_PIXEL_YUV422)

/**
 * This structure defines the variations in LSB to MSB order.
 * BGRA is B as LSB and A as MSB (which is considered ARGB by most).
 */
typedef union _rgb8888 {
    struct {
        uint8_t B;
        uint8_t G;
        uint8_t R;
        uint8_t A;
    } BGRA;     // FOURCC_RGBA
    struct {
        uint8_t A;
        uint8_t R;
        uint8_t G;
        uint8_t B;
    } ARGB;     // FOURCC_BGRA
    struct {
        uint8_t A;
        uint8_t B;
        uint8_t G;
        uint8_t R;
    } ABGR;     // FOURCC_ABGR
    struct {
        uint8_t R;
        uint8_t G;
        uint8_t B;
        uint8_t A;
    } RGBA;     // FOURCC_ABGR
    uint32_t packed;
} RGB8888;

#define SIZEOF_PIXEL_RGB8888 (4)
#define SIZEOF_IMAGE_RGB8888(w,h)    ((w)*(h)*sizeof(RGB8888))

/*! \brief This is the RGB888 (24bit) structure.
 * \note This should be a sizeof(x) == 3 (due to "packed") union member
 */
typedef union _rgb888 {
    struct {
        uint8_t B;
        uint8_t G;
        uint8_t R;
    } BGR;
    struct {
        uint8_t R;
        uint8_t G;
        uint8_t B;
    } RGB;
    uint8_t packed[3];
} RGB888;

#define CAST_uint32_t_TO_RGB888(x,rgb)    {rgb.packed[0] = (x & 0x000000FF) >> 0;\
                                           rgb.packed[1] = (x & 0x0000FF00) >> 8;\
                                           rgb.packed[2] = (x & 0x00FF0000) >> 16;}
#define SIZEOF_PIXEL_RGB888         (3)
#define SIZEOF_IMAGE_RGB888(w,h)    ((w)*(h)*sizeof(RGB888))

/*! \brief This is the RGB565 (16 bit) structure.s
 * \note Check with your compiler to see if this actually becomes a sizeof 2
 */
typedef union _rgb565 {
#ifdef COMPILER_SUPPORTS_BITFIELDS
    struct {
        uint8_t B : 5;
        uint8_t G : 6;
        uint8_t R : 5;
    } BGR;
    struct {
        uint8_t R : 5;
        uint8_t G : 6;
        uint8_t B : 5;
    } RGB;
#endif
    uint16_t packed; // use the macros with this field
} RGB565;

#define SIZEOF_PIXEL_RGB565 (2)
#define SIZEOF_IMAGE_RGB565(w,h)    ((w)*(h)*sizeof(RGB565))


// If you use the "packed" variable above to get values out, use these macros
#define RGB565_GET_R(pixel)     ((pixel & 0xF800) >> 11)
#define RGB565_GET_G(pixel)     ((pixel & 0x07E0) >> 5)
#define RGB565_GET_B(pixel)     ((pixel & 0x001F) >> 0)

#define RGB565_SET_R(r)         ((((r) >> 3) & 0x1F) << 11)
#define RGB565_SET_G(g)         ((((g) >> 2) & 0x3F) << 5)
#define RGB565_SET_B(b)         ((((b) >> 3) & 0x1F) << 0)

/*! \brief The packed pair of RGB565 pixels.s
 * \note This should be sizeof(x) == 4
 */
typedef union _rgb565pair {
    struct {
        RGB565  lower;
        RGB565  upper;
    } halves;
    uint32_t packed;
} RGB565_PAIR;

/*! \brief Converts a YUV pixel to RGB using the BT601 variables in 14 bit precision.
 */
#define CONVERT_YUV_TO_RGB_BT601_Q14(y,u,v,r,g,b) { \
    y -= 16;\
    u -= 128;\
    v -= 128;\
    r = ((19070 * y) >> 14) + ((26148 * v) >> 14); \
    g = ((19070 * y) >> 14) - ((13320 * v) >> 14) - ((6406 * u) >> 14); \
    b = ((19070 * y) >> 14) + ((33062 * u) >> 14); \
}

/*! \brief Converts a RGB pixel to YUV using the BT601 variables in 14 bit precision.
 */
#define CONVERT_RGB_TO_YUV_BT601_Q14(r,g,b,y,u,v) { \
    y = (((3489 * r) >> 14) + ((8257 * g) >> 14) + ((1605 * b) >> 14) + 16); \
    if (y < 16) y = 16; if (y > 235) y = 235; \
    u = (((7192 * b) >> 14) - ((2424 * r) >> 14) - ((4767 * g) >> 14) + 128); \
    if (u < 128) u = 128; if (u > 240) u = 240; \
    v = (((7192 * r) >> 14) - ((6029 * g) >> 14) - ((1163 * b) >> 14) + 128); \
    if (v < 128) v = 128; if (v > 240) v = 240; \
}
#endif
