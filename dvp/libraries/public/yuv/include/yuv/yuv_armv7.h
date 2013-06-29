/*
 *  Copyright (C) 2009-2012 Texas Instruments, Inc.
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

/*! \file
 * \brief The YUV NEON Library
 * \author Erik Rainey <erik.rainey@ti.com>
 * \defgroup group_yuv Algos: YUV (NEON)
 * \brief This only works on ARMv7 NEON enabled cores.
 */

/*!
\page page_yuv YUV NEON Algorithm Library
This file implements YUV and RGB conversion and various other transforms using
the ARMv7 NEON SIMD instruction set.

Here's the BT601 equation of for YCbCr (Computer) conversion to RGB (Computer).
Taken from "Video Demystified, Fourth Edition" by Keith Jack (p.18-19)

Where R,G,B are [0,255]
Where Y is [16,235]
Where Cb,Cr is [0,255]

 \code
 R = 1.164*(Y - 16) + 1.596(Cr - 128)
 G = 1.164*(Y - 16) - 0.813(Cr - 128) - 0.391(Cb - 128)
 B = 1.164*(Y - 16) + 2.018(Cb - 128)
 \endcode

This can be rewritten in a more integer (thus SIMD) friendly form as:

  \code
  Y' = Y - 16
  Cb' = Cb - 128
  Cr' = Cr - 128

  R = (74*Y' + 102*Cr') >> 6
  G = (74*Y' - 52*Cr' - 25*Cb') >> 6
  B = (74*Y' + 129*Cb') >> 6
  \endcode

The inverse is:

  \code
  Y  =  0.257*R + 0.504*G + 0.098*B + 16
  Cb = -0.148*R - 0.291*G + 0.439*B + 128
  Cr =  0.439*R - 0.368*G - 0.071*B + 128
  \endcode

Where R,G,B is [0,255]

This is also rewritable in an integer format as:

  \code
  Y  = ( 16*R + 32*G +  6*B )>>6 + 16
  Cb = ( -9*R - 19*G + 28*B )>>6 + 128
  Cr = ( 28*R - 24*G +  5*B )>>6 + 128
  \endcode

Converting to integer has a disadvantage that there is a necessary loss of
precision but it has the upside of a significant gain in performance.

*/

#ifndef _YUV_ARMV7_H_
#define _YUV_ARMV7_H_

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Converts a UYVY formatted image to a YUV420 formatted image.
 * \param [in] width The width in pixels
 * \param [in] height The height in pixels
 * \param [in] pUYVY The UYVY image pointer.
 * \param [in] srcStride The stride in bytes of the UYVY image.
 * \param [out] pY The pointer to the LUMA plane.
 * \param [out] pU The pointer to the U/Cb plane
 * \param [out] pV The pointer to the V/Cr plane
 * \param [in] y_stride The stride in bytes of the LUMA plane
 * \param [in] uv_stride The stride in bytes of the U/V planes.
 * \ingroup group_yuv
 */
void __uyvy_to_yuv420_image(uint32_t width,
                            uint32_t height,
                            uint8_t *pUYVY,
                            int32_t srcStride,
                            uint8_t *pY,
                            uint8_t *pU,
                            uint8_t *pV,
                            int32_t y_stride,
                            int32_t uv_stride);


/*! \brief This function extracts LUMA (Y) data from a UYVY image.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pYUV The UYVY data pointer.
 * \param [in] srcStride The stride in bytes of the pYUV image.
 * \param [out] pY The LUMA data pointer.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __uyvy_luma_extract(uint32_t width,
                         uint32_t height,
                         uint8_t *pYUV,
                         int32_t srcStride,
                         uint8_t *pY,
                         int32_t dstStride);

/*! \brief Converts luma data back to UYVY format data. Inserts "zero" (not
 * actually zero) data into U/V channels.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pY The pointer to the LUMA image.
 * \param [in] srcStride The stride in bytes of the source image.
 * \param [out] pUYVY The poitner to the UYVY image.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __luma_to_uyvy_image(uint32_t width,
                          uint32_t height,
                          uint8_t *pY,
                          int32_t srcStride,
                          uint8_t *pUYVY,
                          int32_t dstStride);

/*! \brief This function converts a UYVY image to a RGBp image via the BT601 constants.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pSrc The pointer to the source image.
 * \param [in] srcStride The stride in bytes of the source image.
 * \param [out] pR The pointer to the red plane.
 * \param [out] pG The poitner to the green plane.
 * \param [out] pB The poitner to the blue plane.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __uyvy_to_rgbp_image_bt601(uint32_t width,
                                uint32_t height,
                                uint8_t *pSrc,
                                int32_t srcStride,
                                uint8_t *pR,
                                uint8_t *pG,
                                uint8_t *pB,
                                int32_t dstStride);

/*! \brief This function converts a UYVY image to a BGR (24 bit) image via the BT601 constants.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pSrc The pointer to the source image.
 * \param [in] srcStride The stride in bytes of the source image.
 * \param [out] pDst The pointer to the destintation image.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __uyvy_to_bgr_image_bt601(uint32_t width,
                               uint32_t height,
                               uint8_t *pSrc,
                               int32_t srcStride,
                               uint8_t *pDst,
                               int32_t dstStride);

/*! \brief
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pY The pointer the luma plane.
 * \param [in] pU The pointer to the Cb/U plane.
 * \param [in] pV The pointer to the Cr/V plane.
 * \param [out] pDst The pointer to the destintation image
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __iyuv_to_rgb565_image_bt601(uint32_t width,
                                  uint32_t height,
                                  uint8_t *pY,
                                  uint8_t *pU,
                                  uint8_t *pV,
                                  uint32_t *pDst,
                                  int32_t dstStride);

/*! \brief
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pY The pointer to the Luma plane.
 * \param [in] pU The pointer to the U/Cb plane.
 * \param [in] pV The pointer to the V/Cr plane.
 * \param [out] pDst The pointer the destination image.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __iyuv_to_bgr_image_bt601(uint32_t width,
                               uint32_t height,
                               uint8_t *pY,
                               uint8_t *pU,
                               uint8_t *pV,
                               uint8_t *pDst,
                               int32_t dstStride);

/*! \brief Converts YUV444 planar data to RGBp over an image using BT601.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pY The pointer to the Luma plane.
 * \param [in] pU The pointer to the U/Cb plane.
 * \param [in] pV The pointer to the V/Cr plane.
 * \param [in] srcStride The stride in bytes of the source image.
 * \param [out] pR The pointer to the red plane.
 * \param [out] pG The pointer to the green plane.
 * \param [out] pB The pointer to the blue plane.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __yuv444_to_rgbp_image_bt601(uint32_t width,
                                  uint32_t height,
                                  uint8_t *pY,
                                  uint8_t *pU,
                                  uint8_t *pV,
                                  int32_t srcStride,
                                  uint8_t *pR,
                                  uint8_t *pG,
                                  uint8_t *pB,
                                  int32_t dstStride);

/*! \brief Converts ARGB byte ordered data to UYVY over an image.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pSrc The pointer to the source image.
 * \param [in] srcStride The stride in bytes of the source image.
 * \param [out] pDst The pointer to the destination image.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __argb_to_uyvy_image_bt601(uint32_t width,
                                uint32_t height,
                                uint8_t *pSrc,
                                int32_t srcStride,
                                uint8_t *pDst,
                                int32_t dstStride);

/*! \brief Converts BGR byte ordered data to UYVY byte ordered data in an image.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pSrc The pointer to the source image.
 * \param [in] srcStride The stride in bytes of the source image.
 * \param [out] pDst The pointer to the destination image.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __bgr_to_uyvy_image_bt601(uint32_t width,
                               uint32_t height,
                               uint8_t *pSrc,
                               int32_t srcStride,
                               uint8_t *pDst,
                               int32_t dstStride);

/*! \brief Converts a BGR byte ordered data to IYUV formated image.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pSrc The pointer to the source image.
 * \param [in] srcStride The stride in bytes of the source image.
 * \param [out] pY The pointer to the Luma plane.
 * \param [out] pU The pointer to the U/Cb plane.
 * \param [out] pV The pointer to the V/Cr plane.
 * \ingroup group_yuv
 */
void __bgr_to_iyuv_image_bt601(uint32_t width,
                               uint32_t height,
                               uint8_t *pSrc,
                               int32_t srcStride,
                               uint8_t *pY,
                               uint8_t *pU,
                               uint8_t *pV);

/*! \brief Rotates a single plane of byte sized data clockwise 90 degrees.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pSrc The pointer to the source image.
 * \param [out] pDst The pointer to the destination image.
 * \ingroup group_yuv
 */
void __planar_rotate_cw90(uint32_t width,
                          uint32_t height,
                          uint8_t *pSrc,
                          uint8_t *pDst);

/*! \brief Rotates a single plane of byte sized data counter-clockwise 90 degrees.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pSrc The pointer to the source image.
 * \param [out] pDst The pointer to the destination image.
 * \ingroup group_yuv
 */
void __planar_rotate_ccw90(uint32_t width,
                           uint32_t height,
                           uint8_t *pSrc,
                           uint8_t *pDst);

/*! \brief A subroutine to scale down a UYVY image by 2.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pSrc The pointer to the source image.
 * \param [in] srcStride The stride in bytes of the source image.
 * \param [out] pDst The pointer to the destination image.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __uyvy_half_scale_image(uint32_t width,
                             uint32_t height,
                             uint8_t *pSrc,
                             int32_t srcStride,
                             uint8_t *pDst,
                             int32_t dstStride);

/*! \brief A subroutine to scale down a UYVY Image by 4.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pSrc The pointer to the source image.
 * \param [in] srcStride The stride in bytes of the source image.
 * \param [out] pDst The pointer to the destination image.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __uyvy_qtr_scale_image(uint32_t width,
                            uint32_t height,
                            uint8_t *pSrc,
                            int32_t srcStride,
                            uint8_t *pDst,
                            int32_t dstStride);

/*! \brief Horizontally mirrors a UYVY image (swaps Y channel only).
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pSrc The pointer to the source image.
 * \param [in] srcStride The stride in bytes of the source image.
 * \param [out] pDst The pointer to the destination image.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __uyvy_horizontal_mirror_image(uint32_t width,
                                    uint32_t height,
                                    uint8_t *pSrc,
                                    int32_t srcStride,
                                    uint8_t *pDst,
                                    int32_t dstStride);

/*! \brief Rotates a UYVY formated image counter-clockwise 90 degrees.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pSrc The pointer to the source image.
 * \param [out] pDst The pointer to the destination image.
 * \param [in] srcStride The stride in bytes of the source image.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __uyvy_rotate_ccw90(uint32_t width,
                         uint32_t height,
                         uint8_t *pSrc,
                         uint8_t *pDst,
                         int32_t srcStride,
                         int32_t dstStride);

/*! \brief Rotates a UYVY formated image clockwise 90 degrees.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pSrc The pointer to the source image.
 * \param [out] pDst The pointer to the destination image.
 * \param [in] srcStride The stride in bytes of the source image.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __uyvy_rotate_cw90(uint32_t width,
                        uint32_t height,
                        uint8_t *pSrc,
                        uint8_t *pDst,
                        int32_t srcStride,
                        int32_t dstStride);

/*! \brief Converts a UYVY formated image to a YUV444 planar image.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pSrc The pointer to the source image.
 * \param [in] srcStride The stride in bytes of the source image.
 * \param [out] pY The pointer to the Luma plane.
 * \param [out] pU The pointer to the U/Cb plane.
 * \param [out] pV The pointer to the V/Cr plane.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __uyvy_to_yuv444p_image(uint32_t width,
                             uint32_t height,
                             uint8_t *pSrc,
                             int32_t srcStride,
                             uint8_t *pY,
                             uint8_t *pU,
                             uint8_t *pV,
                             int32_t dstStride);

/*! \brief Converts an NV12 image to a half-scaled YUV444 planar image.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pY0 The pointer to the Luma plane.
 * \param [in] pUV The pointer to the interleaved U/V (Cb/Cr) plane.
 * \param [in] srcStride The stride in bytes of the source image.
 * \param [out] pY1 The pointer to the half scaled Luma plane.
 * \param [out] pU The pointer to the U/Cb plane.
 * \param [out] pV The pointer to the V/Cr plane.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __nv12_to_half_yuv444_image(uint32_t width,
                                 uint32_t height,
                                 uint8_t *pY0,
                                 uint8_t *pUV,
                                 int32_t srcStride,
                                 uint8_t *pY1,
                                 uint8_t *pU,
                                 uint8_t *pV,
                                 int32_t dstStride);

/*! \brief Converts a UYVY formatted image to 3 formats; RGBp, Luma, and YUV444p.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pUYVY The pointer to the source image.
 * \param [in] srcStride The stride in bytes of the source image.
 * \param [out] pR The pointer to the red plane.
 * \param [out] pG The pointer to the green plane.
 * \param [out] pB The pointer to the blue plane.
 * \param [out] pL The pointer to the Luma plane.
 * \param [out] pY The pointer to the YUV444 luma plane.
 * \param [out] pU The pointer to the U/Cb plane.
 * \param [out] pV The pointer to the V/Cr plane.
 * \ingroup group_yuv
 */
void __uyvy_to_rgbp_luma_yuv444p_image(uint32_t width,
                                       uint32_t height,
                                       uint8_t *pUYVY,
                                       uint32_t srcStride,
                                       uint8_t *pR,
                                       uint8_t *pG,
                                       uint8_t *pB,
                                       uint8_t *pL,
                                       uint8_t *pY,
                                       uint8_t *pU,
                                       uint8_t *pV);

/*! \brief Converts a pair of lines from YUV420 data to into YUV422.
 * \param [in] pY The pointer to the Luma plane.
 * \param [in] pU The pointer to the U/Cb plane.
 * \param [in] pV The pointer to the V/Cr plane.
 * \param [out] pDst The pointer to the UYVY plane.
 * \param [in] width The width in pixels.
 * \param [in] y_stride The stride in bytes of the source Luma plane. This will
 * also apply to the pU/pV plane.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __yuv420_to_yuv422_line(uint8_t *pY,
                             uint8_t *pU,
                             uint8_t *pV,
                             uint8_t *pDst,
                             uint32_t width,
                             int32_t y_stride,
                             int32_t dstStride);

/*! \brief This function extracts LUMA (Y) data from a YUYV image.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] pYUV The YUYV data pointer.
 * \param [in] srcStride The stride in bytes of the pYUV image.
 * \param [out] pY The LUMA data pointer.
 * \param [in] dstStride The stride in bytes of the destination image.
 * \ingroup group_yuv
 */
void __yuyv_luma_extract(uint32_t width,
                         uint32_t height,
                         uint8_t *pYUV,
                         int32_t srcStride,
                         uint8_t *pY,
                         int32_t dstStride);


#ifdef __cplusplus
}
#endif

#endif
