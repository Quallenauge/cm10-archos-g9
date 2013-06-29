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

/*!
 * \file
 * \brief The IMGFILTER NEON Library.
 * \author Erik Rainey <erik.rainey@ti.com>s
 * \defgroup group_imgfilter Algos: IMGFILTER (NEON)
 * \brief This only works on ARMv7 NEON enabled cores.
 */

/*!
\page page_imgfilter IMGFILTER NEON Library
This library implements a 3x3 convolution operation on 8 bit data in NEON.
*/

#ifndef _IMGFILTER_ARMV7_H_
#define _IMGFILTER_ARMV7_H_

/*! Range of Prewitt Kernel */
#define PREWITT_RANGE (721)

/*! Range of Sobel Kernel */
#define SOBEL_RANGE   (1081)

/*! Range of Scharr Kernel */
#define SCHARR_RANGE  (4688)

/*! Range of Kroon Kernel */
#define KROON_RANGE   (34259)

/*! Range of Tap Kernel */
#define TAP_RANGE     (256)

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * This function does an edge filter on the input image and writes the gradient
 * data out to the pDst pointer per pixel.
 * \param [in] width Image width in pixels
 * \param [in] height Image height in pixels
 * \param [in] pSrc The pointer to the planar image data.
 * \param [in] srcStride The number of bytes between each pixel in the y direction.
 * \param [in] a The values for the gradient matrix to convolve the image with. Gx and Gy will
 *          be created and used.
 * \param [out] pDst The output gradient data.
 * \param [in] dstStride The number of bytes between each pixel in the y direction.
 * \ingroup group_imgfilter
 */
void __planar_edge_filter_3x3(uint32_t width,
                              uint32_t height,
                              uint8_t *pSrc,
                              int32_t srcStride,
                              int32_t a[3],
                              uint8_t *pDst,
                              int32_t dstStride,
                              int32_t range,
                              int32_t limit);

/*!
 * \brief Computes "num" integer square roots of a number in input and stores it
 * in output.
 * \param [in] input The array of input integers.
 * \param [in] num The number of inputs to process.
 * \param [out] output The location to store the results.
 * \ingroup group_imgfilter
 */
void __dual_sqrti_array(int32_t *input, int32_t num, int32_t *output);

/*!
 * \brief Computes "num" float square roots of a number in input and stores it
 * in output.
 * \param [in] input The array of input floats.
 * \param [in] num The number of inputs to process.
 * \param [out] output The location to store the results.
 * \ingroup group_imgfilter
 */
void __dual_sqrtf_array(float *input, int32_t num, float *output);

/*!
 * \brief A 3x1 image filter of fixed coefficients  {-0.25, 1.5, -0.25}
 * \param pSrc The source image pointer. The image must be in FOURCC_ARGB format.
 * \param width The width of the image in pixels.
 * \param height The height of the image in pixels.
 * \param pDst The destination image pointer. The image will be in FOURCC_ARGB format.
 * \param srcStride The stride of the source image in bytes.
 * \param dstStride The stride of the destination image in bytes.
 * \ingroup group_imgfilter
 */
void __3chan_tap_filter_image_3x1_fixed(uint8_t *pSrc,
                                         uint32_t width,
                                         uint32_t height,
                                         uint8_t *pDst,
                                         uint32_t srcStride,
                                         uint32_t dstStride);

/*!
 * \brief A 3x1 image filter of computed fractional coefficients.
 * \detail Assumes input is a 3 pixel channel source like xRGB where the first byte is ignored.
 * uses symmetric fractional coefficients = [-k/64, (64+2k)/64, -k/64] where
 * 0 <= k <= 64. If a value of 11 is used this reduces to
 * [-8/64, (64+16)/64, -8/64] => [-0.125, 1.25, -0.125]. The benefit of this
 * reduced range is that the math never exceeds 16 bit representation.
 * \param pSrc The source image pointer. The image must be in FOURCC_ARGB format.
 * \param width The width of the image in pixels.
 * \param height The height of the image in pixels.
 * \param pDst The destination image pointer. The image will be in FOURCC_ARGB format.
 * \param srcStride The stride of the source image in bytes.
 * \param dstStride The stride of the destination image in bytes.
 * \param k The coefficient to create the 3tap filter with. 0<=k<=64.
 * \ingroup group_imgfilter
 */
void __3chan_tap_filter_image_3x1_fixed_k(uint8_t *pSrc,
                                           uint32_t width,
                                           uint32_t height,
                                           uint8_t *pDst,
                                           uint32_t srcStride,
                                           uint32_t dstStride,
                                           uint8_t k);

/*!
 * \brief A 3x1 image filter which can take arbitrary coefficients.
 * \param pSrc The source image pointer. The image must be in FOURCC_ARGB format.
 * \param width The width of the image in pixels.
 * \param height The height of the image in pixels.
 * \param pDst The destination image pointer. The image will be in FOURCC_ARGB format.
 * \param srcStride The stride of the source image in bytes.
 * \param dstStride The stride of the destination image in bytes.
 * \param coeff The 3 element coefficient matrix.
 * \ingroup group_imgfilter
 */
void __3chan_tap_filter_image_3x1(uint8_t *pSrc,
                                 uint32_t width,
                                 uint32_t height,
                                 uint8_t *pDst,
                                 uint32_t srcStride,
                                 uint32_t dstStride,
                                 int16_t coeff[3]);

#ifdef __cplusplus
}
#endif

#endif
