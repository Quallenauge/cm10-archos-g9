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

.text
.arch armv7-a
.fpu neon

.global __3chan_tap_filter_image_3x1

.include "imgfilter.inc"

/* make the 3x1 data input array in U16 format from some input */
.macro MAKE_3x1_U16  dSrc, qA, qB, qC, dA_0, dC_0
    vshl.U64 \dA_0, \dSrc, #8
    vmovl.U8 \qA, \dA_0
    vmovl.U8 \qB, \dSrc
    vshr.U64 \dC_0, \dSrc, #8
    vmovl.U8 \qC, \dC_0
.endm

.macro LENGTHEN_DATA_COEFF dA0_0, dA0_1, qA0, qA1, dCoeff_0, dCoeff_1, qCoeff0, qCoeff1
    vmovl.U16 \qA0, \dA0_0
    vmovl.U16 \qA1, \dA0_1
    vmovl.S16 \qCoeff0, \dCoeff_0
    vmovl.S16 \qCoeff1, \dCoeff_1
.endm

/* Assumes input is a 3 pixel channel source like xRGB where the first byte is ignored.
 */
__3chan_tap_filter_image_3x1:
pSrc       .req r0
width      .req r1
height     .req r2
pDst       .req r3
pixelsLeft .req r4
linesLeft  .req r5
tmp        .req r6
srcStride  .req r7
dstStride  .req r8
srcStep    .req r7
dstStep    .req r8
coeff      .req r9
tmp2       .req r10
tmp3       .req r11

    PROLOG r0, r11

    /* load the strides */
    ldr srcStride, [sp, #(13 * 4)]
    ldr dstStride, [sp, #(14 * 4)]
    ldr coeff,     [sp, #(15 * 4)]

    /* initialize the height counter */
    mov linesLeft, height

    /* make tmp = width * 4; make the strides into steps */
    mov tmp, width, lsl #2
    sub srcStep, srcStride, tmp
    sub dstStep, dstStride, tmp

    /* load signed 16 bit coefficients into q14,q15,q16 registers */
    ldrsh tmp,      [coeff, #0]
    vdup.16 q13, tmp
    ldrsh tmp,      [coeff, #2]
    vdup.16 q14, tmp
    ldrsh tmp,      [coeff, #4]
    vdup.16 q15, tmp

__3chan_tap_filter_image_3x1_line:

    mov pixelsLeft, width
    sub pixelsLeft, #2

    pld [pSrc]
    mov tmp, #L2_LINE_SIZE
    pld [pSrc, tmp]
    add tmp, tmp, #L2_LINE_SIZE
    pld [pSrc, tmp]
    add tmp, tmp, #L2_LINE_SIZE

    # do the first pixel set from ARMv6
    ldrb tmp, [pSrc]
    strb tmp, [pDst]

    mov tmp, #128
    ldrb tmp2, [pSrc, #1]
    ldrsh tmp3, [coeff, #0]
    smlabb tmp, tmp3, tmp2, tmp
    ldrb tmp2, [pSrc, #1]
    ldrsh tmp3, [coeff, #2]
    smlabb tmp, tmp3, tmp2, tmp
    ldrb tmp2, [pSrc, #5]
    ldrsh tmp3, [coeff, #4]
    smlabb tmp, tmp3, tmp2, tmp
    usat  tmp, #8, tmp, asr #8
    strb tmp, [pDst, #1]

    mov tmp, #128
    ldrb tmp2, [pSrc, #2]
    ldrsh tmp3, [coeff, #0]
    smlabb tmp, tmp3, tmp2, tmp
    ldrb tmp2, [pSrc, #2]
    ldrsh tmp3, [coeff, #2]
    smlabb tmp, tmp3, tmp2, tmp
    ldrb tmp2, [pSrc, #6]
    ldrsh tmp3, [coeff, #4]
    smlabb tmp, tmp3, tmp2, tmp
    usat  tmp, #8, tmp, asr #8
    strb tmp, [pDst, #2]

    mov tmp, #128
    ldrb tmp2, [pSrc, #3]
    ldrsh tmp3, [coeff, #0]
    smlabb tmp, tmp3, tmp2, tmp
    ldrb tmp2, [pSrc, #3]
    ldrsh tmp3, [coeff, #2]
    smlabb tmp, tmp3, tmp2, tmp
    ldrb tmp2, [pSrc, #7]
    ldrsh tmp3, [coeff, #4]
    smlabb tmp, tmp3, tmp2, tmp
    usat  tmp, #8, tmp, asr #8
    strb tmp, [pDst, #3]

__3chan_tap_filter_image_3x1_pixels:

    mov tmp, #L2_LINE_SIZE
    add tmp, tmp, tmp, lsl #1 /* tmp = 3*L2size */
    pld [pSrc, tmp]

    vld4.8 {d0,d1,d2,d3}, [pSrc]

    /* ignore d0, the 'x' */

    vmov.U32 q9, #0
    vmov.U32 q10, #0
    MAKE_3x1_U16 d1, q2, q3, q4, d4, d8
    LENGTHEN_DATA_COEFF d4, d5, q5, q6, d26, d27, q7, q8
    vmla.I32 q9, q5, q7
    vmla.I32 q10, q6, q8
    LENGTHEN_DATA_COEFF d6, d7, q5, q6, d28, d29, q7, q8
    vmla.I32 q9, q5, q7
    vmla.I32 q10, q6, q8
    LENGTHEN_DATA_COEFF d8, d9, q5, q6, d30, d31, q7, q8
    vmla.I32 q9, q5, q7
    vmla.I32 q10, q6, q8
    vqrshrun.S32 d4, q9, #8
    vqrshrun.S32 d5, q10, #8
    vqrshrn.U16 d1, q2, #0

    vmov.U32 q9, #0
    vmov.U32 q10, #0
    MAKE_3x1_U16 d2, q2, q3, q4, d4, d8
    LENGTHEN_DATA_COEFF d4, d5, q5, q6, d26, d27, q7, q8
    vmla.I32 q9, q5, q7
    vmla.I32 q10, q6, q8
    LENGTHEN_DATA_COEFF d6, d7, q5, q6, d28, d29, q7, q8
    vmla.I32 q9, q5, q7
    vmla.I32 q10, q6, q8
    LENGTHEN_DATA_COEFF d8, d9, q5, q6, d30, d31, q7, q8
    vmla.I32 q9, q5, q7
    vmla.I32 q10, q6, q8
    vqrshrun.S32 d4, q9, #8
    vqrshrun.S32 d5, q10, #8
    vqrshrn.U16 d2, q2, #0

    vmov.U32 q9, #0
    vmov.U32 q10, #0
    MAKE_3x1_U16 d3, q2, q3, q4, d4, d8
    LENGTHEN_DATA_COEFF d4, d5, q5, q6, d26, d27, q7, q8
    vmla.I32 q9, q5, q7
    vmla.I32 q10, q6, q8
    LENGTHEN_DATA_COEFF d6, d7, q5, q6, d28, d29, q7, q8
    vmla.I32 q9, q5, q7
    vmla.I32 q10, q6, q8
    LENGTHEN_DATA_COEFF d8, d9, q5, q6, d30, d31, q7, q8
    vmla.I32 q9, q5, q7
    vmla.I32 q10, q6, q8
    vqrshrun.S32 d4, q9, #8
    vqrshrun.S32 d5, q10, #8
    vqrshrn.U16 d3, q2, #0

    add         tmp, pDst, #4

    /* write out 6 32 bit interleaved pixels from the d0,d1,d2,d3 set */
    vst1.8      {d0[1]}, [tmp]!
    vst1.8      {d1[1]}, [tmp]!
    vst1.8      {d2[1]}, [tmp]!
    vst1.8      {d3[1]}, [tmp]!

    vst1.8      {d0[2]}, [tmp]!
    vst1.8      {d1[2]}, [tmp]!
    vst1.8      {d2[2]}, [tmp]!
    vst1.8      {d3[2]}, [tmp]!

    vst1.8      {d0[3]}, [tmp]!
    vst1.8      {d1[3]}, [tmp]!
    vst1.8      {d2[3]}, [tmp]!
    vst1.8      {d3[3]}, [tmp]!

    vst1.8      {d0[4]}, [tmp]!
    vst1.8      {d1[4]}, [tmp]!
    vst1.8      {d2[4]}, [tmp]!
    vst1.8      {d3[4]}, [tmp]!

    vst1.8      {d0[5]}, [tmp]!
    vst1.8      {d1[5]}, [tmp]!
    vst1.8      {d2[5]}, [tmp]!
    vst1.8      {d3[5]}, [tmp]!

    vst1.8      {d0[6]}, [tmp]!
    vst1.8      {d1[6]}, [tmp]!
    vst1.8      {d2[6]}, [tmp]!
    vst1.8      {d3[6]}, [tmp]!

    subs        pixelsLeft, pixelsLeft, #6
    addeq       pSrc, pSrc, #8*4
    addeq       pDst, pDst, #8*4
    beq         __3chan_tap_filter_image_3x1_pixels_end
    cmp         pixelsLeft, #6
    addge       pSrc, pSrc, #6*4
    addge       pDst, pDst, #6*4
    bge         __3chan_tap_filter_image_3x1_pixels
    /* if there are less than 6 left, back up and redo part of them */
    movlt       tmp, #6
    sublt       tmp, tmp, pixelsLeft
    sublt       pSrc, pSrc, tmp, lsl #2
    sublt       pDst, pDst, tmp, lsl #2
    movlt       pixelsLeft, #6
    blt         __3chan_tap_filter_image_3x1_pixels

__3chan_tap_filter_image_3x1_pixels_end:

    # do the last pixel set from ARMv6
    sub pSrc, pSrc, #8
    sub pDst, pDst, #8

    ldrb tmp, [pSrc, #4]
    strb tmp, [pDst, #4]

    mov tmp, #128
    ldrb tmp2, [pSrc, #1]
    ldrsh tmp3, [coeff, #0]
    smlabb tmp, tmp3, tmp2, tmp
    ldrb tmp2, [pSrc, #5]
    ldrsh tmp3, [coeff, #2]
    smlabb tmp, tmp3, tmp2, tmp
    ldrb tmp2, [pSrc, #5]
    ldrsh tmp3, [coeff, #4]
    smlabb tmp, tmp3, tmp2, tmp
    usat  tmp, #8, tmp, asr #8
    strb tmp, [pDst, #5]

    mov tmp, #128
    ldrb tmp2, [pSrc, #2]
    ldrsh tmp3, [coeff, #0]
    smlabb tmp, tmp3, tmp2, tmp
    ldrb tmp2, [pSrc, #6]
    ldrsh tmp3, [coeff, #2]
    smlabb tmp, tmp3, tmp2, tmp
    ldrb tmp2, [pSrc, #6]
    ldrsh tmp3, [coeff, #4]
    smlabb tmp, tmp3, tmp2, tmp
    usat  tmp, #8, tmp, asr #8
    strb tmp, [pDst, #6]

    mov tmp, #128
    ldrb tmp2, [pSrc, #3]
    ldrsh tmp3, [coeff, #0]
    smlabb tmp, tmp3, tmp2, tmp
    ldrb tmp2, [pSrc, #7]
    ldrsh tmp3, [coeff, #2]
    smlabb tmp, tmp3, tmp2, tmp
    ldrb tmp2, [pSrc, #7]
    ldrsh tmp3, [coeff, #4]
    smlabb tmp, tmp3, tmp2, tmp
    usat  tmp, #8, tmp, asr #8
    strb tmp, [pDst, #7]

    add pSrc, pSrc, #8
    add pDst, pDst, #8

    # now do the line math

    add pSrc, pSrc, srcStep
    add pDst, pDst, dstStep

    subs linesLeft, linesLeft, #1
    bgt __3chan_tap_filter_image_3x1_line

    EPILOG r0, r11

.unreq tmp3
.unreq tmp2
.unreq coeff
.unreq dstStep
.unreq dstStride
.unreq srcStep
.unreq srcStride
.unreq tmp
.unreq pixelsLeft
.unreq linesLeft
.unreq pDst
.unreq height
.unreq width
.unreq pSrc

.end
