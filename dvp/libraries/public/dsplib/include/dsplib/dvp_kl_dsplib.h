/* ====================================================================
 *   Copyright (C) 2012 Texas Instruments Incorporated
 * ==================================================================== */

/*!
 * \file
 * \brief This file decribes the exact set of kernels supported by the DSPLIB
 * algorithm library as exported through DVP.
 * \note When using these enums, only the DSPLIB kernels will be used!
 * \defgroup group_algo_dsplib Algos: DSP Library
 * \brief This only works on platforms with DSP firmware with the
 * enabled DVP KGM.
 */

#ifndef _DVP_KL_DSPLIB_H_
#define _DVP_KL_DSPLIB_H_

#ifdef DVP_USE_DSPLIB

#include <dvp/dvp_types.h>
//#define DVP_DSPLIB_TEST1 1
//#define DVP_DSPLIB_TEST2 1

/*!
 * \brief The explicit list of kernels supported by DSPLIB.
 * \see DVP_DSPFunc
 * \ingroup group_algo_dsplib
 */
enum DVP_KernelNode_Dsplib_e {
    DVP_KN_DSP_BASE = DVP_KN_LIBRARY_BASE(DVP_KL_DSPLIB),

    /*!
     * performs addition of two 16-bit vectors\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: int or S32
     */
    DVP_KN_DSP_ADD16,

    /*!
     * performs addition of two 32-bit vectors\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: int or S32
     * \param [out] output Output data type supported: int or S32
     */
    DVP_KN_DSP_ADD32,

    /*!
     * Performs auto correlation of two 16-bit vectors\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_AUTOCORR16,

    /*!
     * Computes the exponents of all values in the input vector and returns the minimum exponent\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Not applicable
     * \param [in] val1 Minumum exponent value: short or S16
     */
    DVP_KN_DSP_BITEXP32,

    /*!
     * Performs Endian swap of a block of 16-bit values\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_BLKESWAP16,

    /*!
     * Performs Endian swap of a block of 32-bit values\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: int or S32
     * \param [out] output Output data type supported: int or S32
     */
    DVP_KN_DSP_BLKESWAP32,

    /*!
     * Performs Endian swap of a block of 64-bit values\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: int or S32
     * \param [out] output Output data type supported: int or S32
     */
    DVP_KN_DSP_BLKESWAP64,

    /*!
     * Performs block move of 16-bit values from source to destination buffer\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_BLKMOVE,

    /*!
     * Performs dot product square of two 16-bit arrays\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_DOTPRODSQR,

    /*!
     * Performs dot product of two 16-bit arrays\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_DOTPROD,

    /*!
     * Computes a complex forward mixed radix FFT\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_FFT_16x16,

    /*!
     * Computes a complex forward mixed radix FFT with rounding\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_FFT_16x16R,

    /*!
     * Computes a complex forward mixed radix FFT with rounding and digit reversal. Each complex value is stored with interleaved real and imaginary parts\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_FFT_16x16_IMRE,

    /*!
     * Computes a complex forward mixed radix FFT with rounding. Coefficients are 16-bit\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: int or S32
     * \param [out] output Output data type supported: int or S32
     */
    DVP_KN_DSP_FFT_16x32,

    /*!
     * Computes a complex forward mixed radix FFT with rounding. Coefficients are 32-bit\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: int or S32
     * \param [out] output Output data type supported: int or S32
    */
    DVP_KN_DSP_FFT_32x32,

    /*!
     * Computes a complex forward mixed radix FFT with rounding and scaling. Coefficients are 32-bit\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: int or S32
     * \param [out] output Output data type supported: int or S32
     */
    DVP_KN_DSP_FFT_32x32s,

    /*!
     * Performs FIR filter for complex input data\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_FIR_CPLX,

    /*!
     * Performs FIR filter for complex input data. Filter coefficients are complex. Number of coefficients are in multiples of 4\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_FIR_HM4x4,

    /*!
     * Performs Real FIR filter\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_FIR_GEN,

    /*!
     * Performs Real FIR filter. Number of outputs must be multiple of 8\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_FIR_HM17_RA8x8,

    /*!
     * Performs Least Mean Square Adaptive filtering. \n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_FIR_LMS2,

    /*!
     * Performs FIR filtering when number of coefficients is a multiple of 4\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_FIR_R4,

    /*!
     * Performs FIR filtering when number of coefficients is a multiple of 8\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_FIR_R8,

    /*!
     * Performs real FIR filtering when number of coefficients is a multiple of 8 and greater than or equal to 16\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_FIR_HM16_RM8A8x8,

    /*!
     * Performs symmetric filtering to the input samples\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_FIR_SYM,

    /*!
     * Computes a complex inverse mixed radix IFFT with rounding and digit reversal\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_IFFT_16x16,

    /*!
     * Computes a complex inverse mixed radix IFFT with rounding and digit reversal and reversed positions for real and imaginary components\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input    Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_IFFT_16x16_IMRE,

    /*!
     * Computes computes an extended precision complex inverse mixed radix FFT with rounding and digit reversal. Coeffecients are 16-bit\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: int or S32
     * \param [out] output Output data type supported: int or S32
     */
    DVP_KN_DSP_IFFT_16x32,

    /*!
     * Computes computes an extended precision complex inverse mixed radix FFT with rounding and digit reversal. Coeffecients are 32-bit\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: int or S32
     * \param [out] output Output data type supported: int or S32
     */
    DVP_KN_DSP_IFFT_32x32,

    /*!
     * Performs IIR filtering with 5 coefficients\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
    */
    DVP_KN_DSP_IIR,

    /*!
     * Performs All pole IIR lattice filtering\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_IIR_LAT,

    /*!
     * Performs single sample  IIR filtering\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_IIR_SS,

    /*!
     * Performs a matrix multiplication on a pair of input vectors\n
     * Configuration Structure: DVP_DSP_MatMul
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_MUL,

    /*!
     * Performs a matrix multiplication on a pair of complex input vectors\n
     * Configuration Structure: DVP_DSP_MatMul
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_MUL_CPLX,

    /*!
     * Computes the transpose of a matrix\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_MAT_TRANS,

    /*!
     * Finds the index of the maximum element of a vector\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Not applicable
     * \param [in] val1 Index to the maximum element of the input vector
     */
    DVP_KN_DSP_MAXIDX,

    /*!
     * Finds the maximum value of a vector and returns the value\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Not applicable
     * \param [in] val1 Maximum value in the input vector
     */
    DVP_KN_DSP_MAXVAL,

    /*!
     * Performs a minimum energy error search\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Not applicable
     * \param [in] val2 Maximum dot product result
     */
    DVP_KN_DSP_MINERROR,

    /*!
     * Finds the minimum value of a vector and returns the value\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Not applicable
     * \param [val1] Minimum value in the input vector
     */
    DVP_KN_DSP_MINVAL,

    /*!
     * Performs a matrix multiplication on a pair of 32-bit input vectors\n
     * Configuration Structure: DVP_DSP_MatMul
     * \param [in] input  Input data type supported: int or S32
     * \param [out] output Output data type supported: int or S32
     */
    DVP_KN_DSP_MUL32,

    /*!
     * This function negates the elements of a vector\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: int or S32
     * \param [out] output Output data type supported: int or S32
     */
    DVP_KN_DSP_NEG32,

    /*!
     * This function converts Q15 values stored in the input vector to IEEE Floating point. Not supported on C64x\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_Q15TOFL,

    /*!
     * This routine returns the fractional and exponential portion of the reciprocal of the input array of Q15 numbers. Not supported on C64x+\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_RECIP16,

    /*!
     * This routine returns the sum of squares of the elements contained in the input vector\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Not applicable
     * \param [in] val1 Sum of squares: int or S32
     */
    DVP_KN_DSP_VECSUMSQ,

    /*!
     * This routine is used to obtain the weighted vector sum of the elements contained in the input vector\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Output data type supported: short or S16
     */
    DVP_KN_DSP_W_VEC,

    /*!
     * This routine is used to generate 16x16 twiddle factors needed for FFT operations\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Not applicable
     * \param [in] val2 Twiddle factor:  short or S16
     */
    DVP_KN_GEN_TWIDDLE_FFT_16X16,

    /*!
     * This routine is used to generate 16x16 twiddle factors needed for FFT IMRE operations\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Not applicable
     * \param [in] val2 Twiddle factor: short or S16
     */
    DVP_KN_GEN_TWIDDLE_FFT_16X16_IMRE,

    /*!
     * This routine is used to generate 16x32 twiddle factors needed for FFT operations\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Not applicable
     * \param [in] val2 Twiddle factor :int or S32
     */
    DVP_KN_GEN_TWIDDLE_FFT_16X32,

    /*!
     * This routine is used to generate 32x32 twiddle factors needed for FFT operations\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: int or S32
     * \param [out] output Not applicable
     * \param [in] val2 Twiddle factor :int or S32
     */
    DVP_KN_GEN_TWIDDLE_FFT_32x32,

    /*!
     * This routine is used to generate 16x16 twiddle factors needed for IFFT operations\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Not applicable
     * \param [in] val2 Twiddle factor :short or S16
     */
    DVP_KN_GEN_TWIDDLE_IFFT_16X16,

    /*!
     * This routine is used to generate 16x16 twiddle factors needed for IFFT IMRE operations\n
     * Configuration Structure: DVP_DSPFunc
     * \param [in] input  Input data type supported: short or S16
     * \param [out] output Not applicable
     * \param [in] val2 Twiddle factor :short or S16
     */
    DVP_KN_GEN_TWIDDLE_IFFT_16X16_IMRE
};

/*! 
 * \brief This is not a generic function. It can find if x is the power of no.
 * \note No has to be multiple of 2 and 4.
 * \param [in] x The value to determine.
 * \param [in] no The
 * \ingroup group_algo_dsplib
 */
static inline DVP_U16 isPowerOf (DVP_U32 x, DVP_U16  no )
{
    if ( no == 2)
    {
        return (
       x == 8 || x == 16 || x == 32 ||
       x == 64 || x == 128 || x == 256 || x == 512 || x == 1024 ||
       x == 2048 || x == 4096 || x == 8192 || x == 16384) ;
    }
    if ( no == 4)
    {
        return(x == 16 || x == 64 || x == 256 || x == 1024 ||
        x == 4096 || x == 16384 ) ;
    }
    else
         return 0;
}

/*! \brief Multiplication strucutre
 * \ingroup group_algo_dsplib
 */
typedef struct  _dsp_matmul{
    DVP_Buffer_t input0;  /*!<  Input Buffer 0 */
    DVP_Buffer_t input1;  /*!<  Input Buffer 1 */
    DVP_Buffer_t output0; /*!<  Output Buffer */
    DVP_S32 r1;           /*!<  Rows of Input Buffer 0 */
    DVP_S32 c1;           /*!<  Columns of Input Buffer 0 */
    DVP_S32 r2;           /*!<  Rows of Input Buffer 1 */
    DVP_S32 c2;           /*!<  Columns of Input Buffer 0 */
    DVP_S32 qs;           /*!<  User defined shift */
}DVP_DSP_MatMul;

/*! \brief Generic Function Parameters to DSPLIB functions.
 * \ingroup group_algo_dsplib
 */
typedef struct  _dsp_func{
    DVP_Buffer_t input0;  /*!<  Input Buffer 0: generic input buffer  */
    DVP_Buffer_t input1;  /*!<  Input Buffer 1: generic input buffer  */
    DVP_Buffer_t twoway;  /*!<  In/Out Buffer: typically used to store filter co-efficients */
    DVP_Buffer_t output0; /*!<  Output Buffer 0: generic output buffer  */
    DVP_Buffer_t output1; /*!<  Output Buffer 1: generic output buffer  */
    DVP_S32 val1;         /*!<  User defined value 1: typically used to store special integer arguments */
    DVP_S32 val2;         /*!<  User defined value 2: typically used to store special integer arguments */
    DVP_S32 val3;         /*!<  User defined value 3: typically used to store special integer arguments */
}DVP_DSPFunc;

#endif // DVP_USE_DSPLIB

#endif // _DVP_KL_DSPLIB_H_
