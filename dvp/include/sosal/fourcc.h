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

#ifndef _SOSAL_FOURCC_H_
#define _SOSAL_FOURCC_H_

/*!
 * \file
 * \brief The List of Color Spaces expressed as a combination of 4 character codes.
 */

/*! \brief The list of supported FOURCC codes in the SOSAL.
 * \see http://www.fourcc.org
 * \ingroup group_fourcc
 */
enum _fourcc {

    FOURCC_NONE = 0,                /*!< An invalid code */
//#if !defined(WIN32) && !defined(CYGWIN)
    // RIFF and AVI Container codes
    FOURCC_RIFF = 0x46464952,       /*!< AVI File Header Type */
    FOURCC_LIST = 0x5453494C,       /*!< AVI List Code */
//#endif
    FOURCC_AVI  = 0x20495641,       /*!< AVI Code */
    FOURCC_HDRL = 0x6C726468,       /*!< AVI List Chunk Type */
    FOURCC_MOVI = 0x69766F6D,       /*!< AVI List Chunk Type */
    FOURCC_AVIH = 0x68697661,       /*!< AVI List Header Type */
    FOURCC_STRL = 0x6C727473,       /*!< AVI Stream List */
    FOURCC_STRH = 0x68727473,       /*!< AVI Stream Header */
    FOURCC_STRF = 0x66727473,       /*!< AVI Stread Format Header */
    FOURCC_VIDS = 0x73646976,       /*!< AVI Video Format */
    FOURCC_JUNK = 0x4B4E554A,       /*!< AVI Junk Section */

    // YUV

    // interleaved
    FOURCC_UYVY = 0x59565955,       /*!< 4:2:2 Interleaved YUV Macropixel in u0,y0,v0,y1 order */
    FOURCC_YUY2 = 0x32595559,       /*!< 4:2:2 Interleaved YUV Macropixel in y0,u0,y1,v0 order */
    FOURCC_YVYU = 0x55595659,       /*!< 4:2:2 Interleaved YUV Macropixel in y0,v0,y1,u0 order */
    FOURCC_VYUY = 0x59555956,       /*!< 4:2:2 Interleaved YUV Macropixel in v0,y0,u0,y1 order */

    // planar
    FOURCC_YUV9 = 0x39565559,       /*!< 4:1:1 Planar YUV */
    FOURCC_YVU9 = 0x39555659,       /*!< 4:1:1 Planar YVU */
    FOURCC_YV12 = 0x32315659,       /*!< 4:2:0 Planar YVU */
    FOURCC_IYUV = 0x56555949,       /*!< 4:2:0 Planar YUV */
    FOURCC_YV16 = 0x36315659,       /*!< 4:2:2 Planar YVU */
    FOURCC_YU16 = 0x36315559,       /*!< 4:2:2 Planar YUV (unoffical) */
    FOURCC_YV24 = 0x34325659,       /*!< 4:4:4 Planar YVU (unoffical) */
    FOURCC_YU24 = 0x34325559,       /*!< 4:4:4 Planar YUV (unoffical) */
    FOURCC_Y800 = 0x30303859,       /*!< 8 bit Luma only  'Y800' */
    FOURCC_Y16  = 0x20363159,       /*!< 16 bit Luma only 'Y16 ' */
    FOURCC_Y32  = 0x20323359,       /*!< 32 bit Luma only 'Y32 ' (unoffical) */
    FOURCC_Y64  = 0x20343659,       /*!< 64 bit Luma only 'Y64 ' (unoffical) */

    // semi-planar
    FOURCC_NV12 = 0x3231564E,       /*!< Y Plane + UV Plane packed with 2x2 subsampling */
    FOURCC_NV21 = 0x3132564E,       /*!< Y Plane + VU Plane packed with 2x2 subsampling */

    // RGB

    FOURCC_RGB565 = 0x35424752,     /*!< 5:6:5 Interleaved single plane RGB data. */
    FOURCC_BGR565 = 0x35524742,     /*!< 5:6:5 Interleaved single plane BGR data. */

    // interleaved
    FOURCC_BGR  = 0x33524742,       /*!< 24 bit (3 Interleaved 8 bit channels) BGR (unofficial) in b0,g0,r0 order */
    FOURCC_RGB  = 0x33424752,       /*!< 24 bit (3 Interleaved 8 bit channels) RGB (unofficial) in r0,g0,b0 order */
    FOURCC_RGBA = 0x41424752,       /*!< 32 bit (4 Interleaved 8 bit channels) RGBA in r0,g0,b0,a0 order */
    FOURCC_BGRA = 0x41524742,       /*!< 32 bit (4 Interleaved 8 bit channels) BGRA in b0,g0,r0,a0 order */
    FOURCC_ARGB = 0x42475241,       /*!< 32 bit (4 Interleaved 8 bit channels) ARGB in a0,r0,g0,b0 order */

    // planar
    FOURCC_RGBP = 0x50424752,       /*!< 8 bit Planar RGB (unoffical) */

    // HSL
    // planar
    FOURCC_HSLP = 0x504C5348,       /*!< 8 bit planar HSL (unoffical) */

    // BINARY
    FOURCC_BIN1 = 0x314E4942,       /*!< 1 bit (8 bits per byte) planar 'image' (unofficial) */
};

#if defined(WIN32) || defined(UNDER_CE) || defined(CYGWIN)
/*! \brief The fourcc_t type which is fixed to a 32 bit representation.
 * \ingroup group_fourcc
 */
typedef uint32_t fourcc_t;
#else
/*! \brief The fourcc_t type which is fixed to a 32 bit representation.
 * \ingroup group_fourcc
 */
typedef uint32_t fourcc_t;
#endif

/*! \brief A macro to cast a string into a fourcc_t code.
 * \ingroup group_fourcc
 */
#define FOURCC_STRING(x)            (*(fourcc_t *)x)

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief String to fourcc_t Code conversion routine to avoid Unaligned memory accesses.
 * \param [in] string The string to convert to a FOURCC code.
 * \ingroup group_fourcc
 */
fourcc_t strtofourcc(const char *string);

/*! \brief Converts a FOURCC code to a 4 character string plus null.
 * \param [in] code The FOURCC code to convert to a string.
 * \note The string returned is a static character array. Multithreaded access is not
 * guaranteed.
 * \ingroup group_fourcc
 */
char *fourcctostr(fourcc_t code);

#ifdef __cplusplus
}
#endif

#endif

