/* ====================================================================
 *   Copyright (C) 2012 Texas Instruments Incorporated
 * ==================================================================== */

/*!
 * \file
 * \brief This file decribes the exact set of kernels supported by the VRUN
 * algorithm library as exported through DVP.
 * \note When using these enums, only the VRUN kernels will be used!
 * \defgroup group_algo_vrun Algos: VRUN (SIMCOP)
 * \brief This only works on platforms with a Ducati/Benelli firmware with
 * the enabled DVP KGM.
 */

#ifndef _DVP_KL_VRUN_H_
#define _DVP_KL_VRUN_H_

#ifdef DVP_USE_VRUN

#include <dvp/dvp_types.h>

/*! \brief The explicit list of kernels supported by the VRUN algorithm library.
 * \see DVP_Transform_t
 * \ingroup group_algo_vrun
 */
enum DVP_KernelNode_Vrun_e {
    DVP_KN_VRUN_BASE = DVP_KN_LIBRARY_BASE(DVP_KL_VRUN),

    /*!
     * Image Dilation using a 3x3 Cross Pattern : [0,1,0],[1,1,1],[0,1,0] (VLIB-API.Sec 14.0)\n
     * Configuration Structure: DVP_Morphology_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask   Not applicable
     */
    DVP_KN_VRUN_DILATE_CROSS,

    /*!
     * Image Dilation using a user specified 3x3 Mask pattern (VLIB-API.Sec 14.0)\n
     * Configuration Structure: DVP_Morphology_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask   Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_DILATE_MASK,

    /*!
     * Image Dilation using a 3x3 Square pattern : [1,1,1],[1,1,1],[1,1,1] (VLIB-API.Sec 14.0) \n
     * Configuration Structure: DVP_Morphology_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask   Not applicable
     */
    DVP_KN_VRUN_DILATE_SQUARE,

    /*!
     * Image Erosion using a 3x3 Cross Pattern : [0,1,0],[1,1,1],[0,1,0]  (VLIB-API.Sec 15.0) \n
     * Configuration Structure: DVP_Morphology_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask   Not applicable
     */
    DVP_KN_VRUN_ERODE_CROSS,

    /*!
     * Image Erosion using a user specified 3x3 Mask pattern (VLIB-API.Sec 15.0) \n
     * Configuration Structure: DVP_Morphology_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask   Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_ERODE_MASK,

    /*!
     * Image Erosion using a 3x3 Square pattern : [1,1,1],[1,1,1],[1,1,1]  (VLIB-API.Sec 15.0) \n
     * Configuration Structure: DVP_Morphology_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask   Not applicable
     */
    DVP_KN_VRUN_ERODE_SQUARE,

    /*!
     * Recursive First Order IIR Filter, Horizontal, FOURCC_Y800 input and output (VLIB-API.Sec 27.0) \n
     * Configuration Structure: DVP_IIR_t
     * \param [in] input   Image color type supported: FOURCC_Y800
     * \param [out] output  Image color type supported: FOURCC_Y800
     * \param [in] scratch Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_IIR_HORZ,

    /*!
     * Recursive First Order IIR Filter, Vertical, FOURCC_Y800 input and output (VLIB-API.Sec 29.0)\n
     * Configuration Structure: DVP_IIR_t
     * \param [in] input   Image color type supported: FOURCC_Y800
     * \param [out] output  Image color type supported: FOURCC_Y800
     * \param [in] scratch Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_IIR_VERT,

    /*!
     * Non-maximum suppression using 3x3 (VLIB-API.Sec 35.0)\n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input  Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_NONMAXSUPPRESS_3x3_S16,

    /*!
     * Non-maximum suppression using 5x5 (VLIB-API.Sec 35.0)\n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input  Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_NONMAXSUPPRESS_5x5_S16,

    /*!
     * Non-maximum suppression using 7x7 (VLIB-API.Sec 35.0)\n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input  Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_NONMAXSUPPRESS_7x7_S16,

    /*!
     * Image Convolution using an arbitrary 3x3 matrix on FOURCC_Y800 data (IMGLIB-API.Sec 6.1) \n
     * Configuration Structure: DVP_ImageConvolution_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask   Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_CONV_3x3,

    /*!
     * Image Convolution using an arbitrary 5x5 matrix on FOURCC_Y800 data (IMGLIB-API.Sec 6.4)\n
     * Configuration Structure: DVP_ImageConvolution_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask   Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_CONV_5x5,

    /*!
     * Image Convolution using an arbitrary 7x7 matrix on FOURCC_Y800 data (IMGLIB-API.Sec 6.7)\n
     * Configuration Structure: DVP_ImageConvolution_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask   Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_CONV_7x7,

    /*!
     * Image Convolution using an arbitrary MxN matrix on FOURCC_Y800 data\n
     * Configuration Structure: DVP_ImageConvolution_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask   Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_CONV_MxN,

    /*!
     * Image Thresholding : {out[i] = (in[i] >  thr) ?  255  : in[i]} for 8-bit  input (IMGLIB-API.Sec 5.20) \n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_THR_GT2MAX_8,

    /*!
     * Image Thresholding : {out[i] = (in[i] >  thr) ? 65535 : in[i]} for 16-bit input (IMGLIB-API.Sec 5.21) \n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input  Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_VRUN_THR_GT2MAX_16,

    /*!
     * Image Thresholding : {out[i] = (in[i] >  thr) ?  thr  : in[i]} for 8-bit  input (IMGLIB-API.Sec 5.22) \n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_THR_GT2THR_8,

    /*!
     * Image Thresholding : {out[i] = (in[i] >  thr) ?  thr  : in[i]} for 16-bit input (IMGLIB-API.Sec 5.23) \n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input  Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_VRUN_THR_GT2THR_16,

    /*!
     * Image Thresholding : {out[i] = (in[i] <= thr) ?   0   : in[i]} for 8-bit  input (IMGLIB-API.Sec 5.24) \n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_THR_LE2MIN_8,

    /*!
     * Image Thresholding : {out[i] = (in[i] <= thr) ?   0   : in[i]} for 16-bit input (IMGLIB-API.Sec 5.25) \n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input  Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_VRUN_THR_LE2MIN_16,

    /*!
     * Image Thresholding : {out[i] = (in[i] <= thr) ?  thr  : in[i]} for 8-bit  input (IMGLIB-API.Sec 5.26) \n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_THR_LE2THR_8,

    /*!
     * Image Thresholding : {out[i] = (in[i] <= thr) ?  thr  : in[i]} for 16-bit input (IMGLIB-API.Sec 5.27) \n
     * Configuration Structure: DVP_Threshold_t
     * \param [in] input  Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_VRUN_THR_LE2THR_16,

    /*!
     * Image Sobel : 3x3 for 8-bit  signed   input\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_SOBEL_3x3_8s,

    /*!
     * Image Sobel : 3x3 for 8-bit  unsigned input (IMGLIB-API.Sec 5.15)\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_SOBEL_3x3_8,

    /*!
     * Image Sobel : 3x3 for 16-bit signed   input (IMGLIB-API.Sec 5.16)\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_VRUN_SOBEL_3x3_16s,

    /*!
     * Image Sobel : 3x3 for 16-bit unsigned   input (IMGLIB-API.Sec 5.17)\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_SOBEL_3x3_16,

    /*!
     * Image Sobel : 5x5 for 8-bit  signed   input \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_SOBEL_5x5_8s,

    /*!
     * Image Sobel : 5x5 for 8-bit  unsigned input \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_SOBEL_5x5_8,

    /*!
     * Image Sobel : 5x5 for 16-bit signed   input (IMGLIB-API.Sec 5.18) \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_VRUN_SOBEL_5x5_16s,

    /*!
     * Image Sobel : 5x5 for 16-bit unsigned input \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_VRUN_SOBEL_5x5_16,

    /*!
     * Image Sobel : 7x7 for 8-bit  signed   input \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_SOBEL_7x7_8s,

    /*!
     * Image Sobel : 7x7 for 8-bit  unsigned input \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_SOBEL_7x7_8,

    /*!
     * Image Sobel : 7x7 for 16-bit signed   input (IMGLIB-API.Sec 5.19)\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_VRUN_SOBEL_7x7_16s,

    /*!
     * Image Sobel : 7x7 for 16-bit unsigned input\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y16
     * \param [out] output Image color type supported: FOURCC_Y16
     */
    DVP_KN_VRUN_SOBEL_7x7_16,

    /*!
     * Harris Corners\n
     * Configuration Structure: DVP_HarrisCorners_t
     * \param [in] input    Image color type supported: FOURCC_Y800
     * \param [out] output   Image color type supported: FOURCC_Y800
     * \param [in] tempBuf1 Image color type supported: FOURCC_Y16
     * \param [in] tempBuf2 Image color type supported: FOURCC_Y16
     * \param [in] tempBuf3 Image color type supported: FOURCC_Y16
     * \param [in] tempBuf4 Image color type supported: FOURCC_Y16
     */
    DVP_KN_VRUN_HARRIS_CORNERS,

    /*!
     * Harris Corner Score 7x7\n
     * Configuration Structure: DVP_Harris_t
     * \param [in] input       Image color type supported: FOURCC_Y800
     * \param [out] harrisScore Image color type supported: FOURCC_Y16
     */
    DVP_KN_VRUN_HARRIS_SCORE_7x7,

    /*!
     * Compute  block maxima,  FOURCC_Y16 input\n
     * Configuration Structure: DVP_BlockMaxima_t
     * \param [in] input          Image color type supported: FOURCC_Y16
     * \param [out] blockMaximaMAX Image color type supported: FOURCC_Y16
     * \param [out] blockMaximaX   Image color type supported: FOURCC_Y16
     * \param [out] blockMaximaY   Image color type supported: FOURCC_Y16
     */
    DVP_KN_VRUN_BLOCK_MAXIMA,

    /*!
     * Non maximum suppression step1 for 16-bit X and Y input from block_maxima\n
     * Configuration Structure: DVP_NMSStep1_t
     * \param [in] nmsStep1X   Image color type supported: FOURCC_Y16
     * \param [in] nmsStep1Y   Image color type supported: FOURCC_Y16
     * \param [in] nmsStep1Dst Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_NMS_STEP1,

    /*!
     * Image Convolution using an arbitrary 7x7 matrix on FOURCC_Y800 data (IMGLIB-API.Sec 6.7)\n
     * Configuration Structure: DVP_ImageConvolution_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] mask   Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_CANNY_IMAGE_SMOOTHING,

    /*!
     * Canny 2D Gradient from FOURCC_Y800 to FOURCC_Y16 (VLIB-API.Sec 19.0) \n
     * Configuration Structure: DVP_Canny2dGradient_t
     * \param [in] input    Image color type supported: FOURCC_Y800
     * \param [out] outGradX Image color type supported: FOURCC_Y16
     * \param [out] outGradY Image color type supported: FOURCC_Y16
     * \param [out] outMag   Image color type supported: FOURCC_Y16
     */
    DVP_KN_VRUN_CANNY_2D_GRADIENT,

    /*!
     * Canny Non-Maximum Suppression from FOURCC_Y16 data to FOURCC_Y800 data (VLIB-API.Sec 20.0) \n
     * Configuration Structure: DVP_CannyNonMaxSuppression_t
     * \param [in] inGradX Image color type supported: FOURCC_Y16
     * \param [in] inGradY Image color type supported: FOURCC_Y16
     * \param [in] inMag   Image color type supported: FOURCC_Y16
     * \param [out] output  Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_CANNY_NONMAX_SUPPRESSION,

    /*!
     * Canny Hysterisis Threshholding from FOURCC_Y800 edge map data to FOURCC_Y800 edge image (VLIB-API.Sec 21.0)  \n
     * Configuration Structure: DVP_CannyHystThresholding_t
     * \param [in] inMag     Image color type supported: FOURCC_Y16
     * \param [in] inEdgeMap Image color type supported: FOURCC_Y800
     * \param [out] output    Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_CANNY_HYST_THRESHHOLD,

    /*!
     * Converts FOURCC_UYVY format to FOURCC_YU24 or FOURCC_YV24 format.
     * Depending on the output size ratio to input size, chroma planes can upsampled or
     * luma plane can be downsampled.\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_UYVY
     * \param [out] output Image color type supported: FOURCC_YU24 or FOURCC_YV24
     */
    DVP_KN_VRUN_UYVY_TO_YUV444p,

    /*!
     * Converts FOURCC_UYVY format to 8 bits per pixels planar RGB using BT.601. (VLIB-API.Sec 58.0)\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_UYVY
     * \param [out] output Image color type supported: FOURCC_RGBP
     */
    DVP_KN_VRUN_UYVY_TO_RGBp,

    /*!
     * Converts FOURCC_UYVY format to FOURCC_IYUV or FOURCC_YV12 format. Chroma planes are downsampled vertically. (VLIB-API.Sec 55.0)\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_UYVY
     * \param [out] output Image color type supported: FOURCC_IYUV or FOURCC_YV12
     */
    DVP_KN_VRUN_UYVY_TO_YUV420p,

    /*!
     * Converts FOURCC_UYVY or FOURCC_VYUY to FOURCC_Y800 image (luma extraction)\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_UYVY or FOURCC_VYUY
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_XYXY_TO_Y800,

    /*!
     * Converts FOURCC_NV12 format to FOURCC_YU24 or FOURCC_YV24 format.
     * Depending on the output size ratio to input size, chroma planes can upsampled or
     * luma plane can be downsampled.\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_NV12
     * \param [out] output Image color type supported: FOURCC_YU24 or FOURCC_YV24
     */
    DVP_KN_VRUN_NV12_TO_YUV444p,

    /*!
     * Converts FOURCC_YU24 or FOURCC_YV24 format to FOURCC_YU24 or FOURCC_YV24 format to 8 bits per pixels planar RGB using BT.601\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_YU24 or FOURCC_YV24
     * \param [out] output Image color type supported: FOURCC_RGBP
     */
    DVP_KN_VRUN_YUV444p_TO_RGBp,

    /*!
     * Computes a Integral Image over the FOURCC_Y800 data. 32 bit output (VLIB-API.Sec 31.0)\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input   Image color type supported: FOURCC_Y800
     * \param [out] output  Image color type supported: FOURCC_Y32
     */
    DVP_KN_VRUN_INTEGRAL_IMAGE_8,

    /*!
     * Programmable affine Transformation \n
     * Configuration Structure: DVP_Ldc_t
     * \param [in] input  Image color type supported: FOURCC_UYVY or FOURCC_NV12
     * \param [out] output Image color type supported: FOURCC_UYVY or FOURCC_NV12
     */
    DVP_KN_LDC_AFFINE_TRANSFORM,

    /*!
     * SAD with 8x8 reference image 8-bit unsigned input \n
     * Configuration Structure: DVP_SAD_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] refImg Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_SAD_8x8,

    /*!
     * SAD with 16x16 reference image 8-bit unsigned input \n
     * Configuration Structure: DVP_SAD_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] refImg Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_SAD_16x16,

    /*!
     * SAD with 3x3 reference image 8-bit unsigned input \n
     * Configuration Structure: DVP_SAD_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] refImg Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_SAD_3x3,

    /*!
     * SAD with 5x5 reference image 8-bit unsigned input \n
     * Configuration Structure: DVP_SAD_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] refImg Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_SAD_5x5,

    /*!
     * SAD with 7x7 reference image 8-bit unsigned input \n
     * Configuration Structure: DVP_SAD_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [in] refImg Image color type supported: FOURCC_Y800
     */
    DVP_KN_VRUN_SAD_7x7,

    /*!
     * Programmable lens distortion correction \n
     * Configuration Structure: DVP_Ldc_t
     * \param [in] input  Image color type supported: FOURCC_UYVY or FOURCC_NV12
     * \param [out] output Image color type supported: FOURCC_UYVY or FOURCC_NV12
     */
    DVP_KN_LDC_DISTORTION_CORRECTION,

    /*!
     * Programmable lens distortion correction and affine Transformation \n
     * Configuration Structure: DVP_Ldc_t
     * \param [in] input  Image color type supported: FOURCC_UYVY or FOURCC_NV12
     * \param [out] output Image color type supported: FOURCC_UYVY or FOURCC_NV12
     */
    DVP_KN_LDC_DISTORTION_AND_AFFINE,

    /*!
     * Image Pyramid of 4 levels (input is level 0, output contains 3 downsampled levels), each level is a 2x2 subsample of the last. (VLIB-API.Sec 22.0)  \n
     * Configuration Structure: DVP_Pyramid_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Output buffer size should be width*height*21/64 bytes (w*h/4 + w*h/16 + w*h/64)
     */
    DVP_KN_VRUN_IMAGE_PYRAMID_8,

    /*!
     * Gaussian 3x3 Image Convolution Pyramid (one 2x2 downsampled level) with a fixed operator (VLIB-API.Sec 24.0)  \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input   Image color type supported: FOURCC_Y800
     * \param [out] output  Image color type supported: FOURCC_Y800 (Output buffer size should be width/2, height/2)
     */
    DVP_KN_VRUN_GAUSSIAN_3x3_PYRAMID_8,

    /*!
     * Gaussian 5x5 Image Convolution Pyramid (one 2x2 downsampled level) with a fixed operator (VLIB-API.Sec 24.0)  \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input   Image color type supported: FOURCC_Y800
     * \param [out] output  Image color type supported: FOURCC_Y800 (Output buffer size should be width/2, height/2)
     */
    DVP_KN_VRUN_GAUSSIAN_5x5_PYRAMID_8,

    /*!
     * Gaussian 7x7 Image Convolution Pyramid (one 2x2 downsampled level) with a fixed operator (VLIB-API.Sec 24.0)  \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input   Image color type supported: FOURCC_Y800
     * \param [out] output  Image color type supported: FOURCC_Y800 (Output buffer size should be width/2, height/2)
     */
    DVP_KN_VRUN_GAUSSIAN_7x7_PYRAMID_8,

    /*!
     * Gaussian 3x3 Image Convolution Pyramid (one 2x2 downsampled level) with only horizontal gradients (VLIB-API.Sec 26.0) \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input   Image color type supported: FOURCC_Y800
     * \param [out] output  Image color type supported: FOURCC_Y800 (Output buffer size should be width/2, height/2)
     */
    DVP_KN_VRUN_GRADIENT_H3x3_PYRAMID_8,

    /*!
     * Gaussian 5x5 Image Convolution Pyramid (one 2x2 downsampled level) with only horizontal gradients (VLIB-API.Sec 26.0) \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input   Image color type supported: FOURCC_Y800
     * \param [out] output  Image color type supported: FOURCC_Y800 (Output buffer size should be width/2, height/2)
     */
    DVP_KN_VRUN_GRADIENT_H5x5_PYRAMID_8,

    /*!
     * Gaussian 7x7 Image Convolution Pyramid (one 2x2 downsampled level) with only horizontal gradients (VLIB-API.Sec 26.0) \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input   Image color type supported: FOURCC_Y800
     * \param [out] output  Image color type supported: FOURCC_Y800 (Output buffer size should be width/2, height/2)
     */
    DVP_KN_VRUN_GRADIENT_H7x7_PYRAMID_8,

    /*!
     * Gaussian 3x3 Image Convolution Pyramid (one 2x2 downsampled level) with only vertical gradients (VLIB-API.Sec 26.0) \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input   Image color type supported: FOURCC_Y800
     * \param [out] output  Image color type supported: FOURCC_Y800 (Output buffer size should be width/2, height/2)
     */
    DVP_KN_VRUN_GRADIENT_V3x3_PYRAMID_8,

    /*!
     * Gaussian 5x5 Image Convolution Pyramid (one 2x2 downsampled level) with only vertical gradients (VLIB-API.Sec 26.0) \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input   Image color type supported: FOURCC_Y800
     * \param [out] output  Image color type supported: FOURCC_Y800 (Output buffer size should be width/2, height/2)
     */
    DVP_KN_VRUN_GRADIENT_V5x5_PYRAMID_8,

    /*!
     * Gaussian 7x7 Image Convolution Pyramid (one 2x2 downsampled level) with only vertical gradients (VLIB-API.Sec 26.0) \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input   Image color type supported: FOURCC_Y800
     * \param [out] output  Image color type supported: FOURCC_Y800 (Output buffer size should be width/2, height/2)
     */
    DVP_KN_VRUN_GRADIENT_V7x7_PYRAMID_8,
};

/*!
 * \brief This structure is used with LDC Kernels.
 * \ingroup group_algo_vrun
 */
typedef struct _dvp_ldc_t {
    DVP_Image_t input;                     /*!<  Input image (FOURCC_UYVY or FOURCC_NV12) */
    DVP_Image_t output;                    /*!<  Output image (must be same format as input) */
    DVP_S16     affine[6];                 /*!<  Affine parameters (will be ignored for DVP_KN_LDC_DISTORTION_CORRECTION) */
    /* ======================================================================= */
    /* affine[6] -  Affine transform parameters
     *  --       --     --                   --     --     --       --       --
     * | h_affine  | = | affine[0]   affine[1] | * | h_input |  +  | affine[2] |
     * | v_affine  |   | affine[3]   affine[4] |   | v_input |     | affine[5] |
     *  --       --     --                   --     --     --       --       --
     * where (h_affine, v_affine) denotes the tranformed coordinates after applying affine transformation
     *       (h_input, v_input) denotes the coordinates of the input pixels (can be distorted or distortion corrected pixels)
     *
     * If s & r denote the desired scaling factor and rotation angle that needs to be applied via affine transform to the input,
     *   affine[0] = s * cos(r);
     *   affine[1] = s * sin(r);
     *   affine[3] = -s * sin(r);
     *   affine[4] = s * cos(r);
     *   affine[2] = h_0 - h_0 * A - v_0 * B;
     *   affine[5] = v_0 - h_0 * D - v_0 * E;
     *
     * where (h_0, v_0) denotes the coordinates of the lens center
     */
    /* ======================================================================= */
    DVP_U16 pixpad;                        /*!<  Pixel padding parameter */
    DVP_U16 interpolationLuma;             /*!<  Interpolation mode (0->BICUBIC, 1->BILINEAR) */

    /* All of the following parameters will be ignored for DVP_KN_LDC_AFFINE_TRANSFORM */
    DVP_Buffer_t ldcLut;                   /*!<  LDC LUT (256 entries of size DVP_U16) */
    DVP_U16      ldcLensCenterX;           /*!<  Lens Center, X coordinate  */
    DVP_U16      ldcLensCenterY;           /*!<  Lens Center, Y coordinate */
    DVP_U08      ldcKhl;                   /*!<  Horizontal left radial scaling factor */
    DVP_U08      ldcKhr;                   /*!<  Horizontal right radial scaling factor */
    DVP_U08      ldcKvu;                   /*!<  Vertical upper radial scaling factor */
    DVP_U08      ldcKvl;                   /*!<  Vertical lower radial scaling factor */
    DVP_U16      ldcRth;                   /*!<  Radius Threshold */
    DVP_U08      ldcRightShiftBits;        /*!<  Downshift value */
} DVP_Ldc_t;

/*!
 * \brief This structure is used with Harris Corner Kernel in VRUN.
 * \ingroup group_algo_vrun
 */
typedef struct _dvp_harriscorner_t {
    DVP_Image_t input;       /*!<  Luma image */
    DVP_Image_t output;      /*!<  Harris Corner NMS step1 output */
    DVP_Image_t tempBuf1;
    DVP_Image_t tempBuf2;
    DVP_Image_t tempBuf3;
    DVP_Image_t tempBuf4;
    DVP_S16     nplus1;
    DVP_S16     k;           /*!<  Sensitivity parameter */
} DVP_HarrisCorners_t;

/*!
 * \brief This structure is used with Harris Corner Score Kernels.
 * \ingroup group_algo_vrun
 */
typedef struct _dvp_harris_t {
    DVP_Image_t input;           /*!<  Luma image */
    DVP_Image_t harrisScore;     /*!<  Harris (corneress) score */
    DVP_S16     k;               /*!<  Sensitivity parameter */
} DVP_Harris_t;

/*!
 * \brief This structure is used with Block Maxima Kernels.
 * \ingroup group_algo_vrun
 */
typedef struct _dvp_blockmaxima_t {
    DVP_Image_t input;           /*!<  Luma image */
    DVP_Image_t blockMaximaX;
    DVP_Image_t blockMaximaY;
    DVP_Image_t blockMaximaMAX;
    DVP_S16     nplus1;
} DVP_BlockMaxima_t;

/*!
 * \brief This structure is used with NMS_step1 Kernels.
 * \ingroup group_algo_vrun
 */
typedef struct _dvp_nmsstep1_t {
    DVP_Image_t nmsStep1X;
    DVP_Image_t nmsStep1Y;
    DVP_Image_t nmsStep1Dst;
    DVP_S16     nplus1;
} DVP_NMSStep1_t;

#endif // DVP_USE_VRUN

#endif // _DVP_KL_VRUN_H_

