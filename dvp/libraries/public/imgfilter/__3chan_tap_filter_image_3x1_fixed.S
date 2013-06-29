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

.global __3chan_tap_filter_image_3x1_fixed

.include "imgfilter.inc"

/* Assumes input is a 3 pixel channel source like xRGB where the first byte is ignored.
 * uses fixed coefficients of -0.125, 1.25, -0.125. or a[0]>>4, a[1] + a[1]>>2, a[2]>>4
 */
__3chan_tap_filter_image_3x1_fixed:
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
tmp2       .req r9
tmp3       .req r10

    PROLOG r0, r10

    /* load the strides */
    ldr srcStride, [sp, #(12 * 4)]
    ldr dstStride, [sp, #(13 * 4)]

    /* initialize the height counter */
    mov linesLeft, height

    /* make tmp = width * 4; make the strides into steps */
    mov tmp, width, lsl #2
    sub srcStep, srcStride, tmp
    sub dstStep, dstStride, tmp

__3chan_tap_filter_image_3x1_fixed_line:

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

    ldrb tmp2, [pSrc, #1]
    add tmp, tmp2, tmp2, lsr #2
    sub tmp, tmp, tmp2, lsr #3
    ldrb tmp2, [pSrc, #5]
    sub tmp, tmp, tmp2, lsr #3
    usat tmp, #8, tmp, asr #0
    strb tmp, [pDst, #1]

    ldrb tmp2, [pSrc, #2]
    add tmp, tmp2, tmp2, lsr #2
    sub tmp, tmp, tmp2, lsr #3
    ldrb tmp2, [pSrc, #6]
    sub tmp, tmp, tmp2, lsr #3
    usat tmp, #8, tmp, asr #0
    strb tmp, [pDst, #2]

    ldrb tmp2, [pSrc, #3]
    add tmp, tmp2, tmp2, lsr #2
    sub tmp, tmp, tmp2, lsr #3
    ldrb tmp2, [pSrc, #7]
    sub tmp, tmp, tmp2, lsr #3
    usat tmp, #8, tmp, asr #0
    strb tmp, [pDst, #3]

__3chan_tap_filter_image_3x1_fixed_pixels:

    mov tmp, #L2_LINE_SIZE
    add tmp, tmp, tmp, lsl #1 /* tmp = 3*L2size */
    pld [pSrc, tmp]

    vld4.8 {d0,d1,d2,d3}, [pSrc]

    /* ignore d0, the 'x' */

    /* d1+d1>>2 = q13, etc */
    vshr.U8 d16, d1, #2
    vaddl.U8 q13, d1, d16
    vshr.U8 d16, d2, #2
    vaddl.U8 q14, d2, d16
    vshr.U8 d16, d3, #2
    vaddl.U8 q15, d3, d16

    /* d1>>3=d4, d2>>3=d5, d3>>3=d6 */
    vshr.U8 d4, d1, #3
    vshr.U8 d5, d2, #3
    vshr.U8 d6, d3, #3

    /* roll d4,5,6 to the left and up into d7,8,9 */
    vshl.U64 d7, d4, #8
    vshl.U64 d8, d5, #8
    vshl.U64 d9, d6, #8
    vmovl.U8 q10, d7
    vmovl.U8 q11, d8
    vmovl.U8 q12, d9

    /* roll d4,5,6  to the right and up into d10,11,12 */
    vshr.U64 d10, d4, #8
    vshr.U64 d11, d5, #8
    vshr.U64 d12, d6, #8
    vmovl.U8 q7, d10
    vmovl.U8 q8, d11
    vmovl.U8 q9, d12

    /* add the components together */
    vsub.I16 q2, q13, q7
    vsub.I16 q2, q2, q10
    vsub.I16 q3, q14, q8
    vsub.I16 q3, q3, q11
    vsub.I16 q4, q15, q9
    vsub.I16 q4, q4, q12

    /* narrow back down */
    vqrshrun.S16 d1, q2, #0
    vqrshrun.S16 d2, q3, #0
    vqrshrun.S16 d3, q4, #0

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
    beq         __3chan_tap_filter_image_3x1_fixed_pixels_end
    cmp         pixelsLeft, #6
    addge       pSrc, pSrc, #6*4
    addge       pDst, pDst, #6*4
    bge         __3chan_tap_filter_image_3x1_fixed_pixels
    /* if there are less than 6 left, back up and redo part of them */
    movlt       tmp, #6
    sublt       tmp, tmp, pixelsLeft
    sublt       pSrc, pSrc, tmp, lsl #2
    sublt       pDst, pDst, tmp, lsl #2
    movlt       pixelsLeft, #6
    blt         __3chan_tap_filter_image_3x1_fixed_pixels

__3chan_tap_filter_image_3x1_fixed_pixels_end:

    # do the last pixel set from ARMv6
    sub pSrc, pSrc, #8
    sub pDst, pDst, #8

    ldrb tmp, [pSrc, #4]
    strb tmp, [pDst, #4]

    mov tmp, #0
    ldrb tmp2, [pSrc, #1]
    sub tmp, tmp, tmp2, lsr #3
    ldrb tmp2, [pSrc, #5]
    add tmp2, tmp2, tmp2, lsr #2
    sub tmp2, tmp2, tmp2, lsr #3
    add tmp, tmp, tmp2
    usat tmp, #8, tmp, asr #0
    strb tmp, [pDst, #1]

    mov tmp, #0
    ldrb tmp2, [pSrc, #2]
    sub tmp, tmp, tmp2, lsr #3
    ldrb tmp2, [pSrc, #6]
    add tmp2, tmp2, tmp2, lsr #2
    sub tmp2, tmp2, tmp2, lsr #3
    add tmp, tmp, tmp2
    usat tmp, #8, tmp, asr #0
    strb tmp, [pDst, #2]

    mov tmp, #0
    ldrb tmp2, [pSrc, #3]
    sub tmp, tmp, tmp2, lsr #3
    ldrb tmp2, [pSrc, #7]
    add tmp2, tmp2, tmp2, lsr #2
    sub tmp2, tmp2, tmp2, lsr #3
    add tmp, tmp, tmp2
    usat tmp, #8, tmp, asr #0
    strb tmp, [pDst, #3]

    add pSrc, pSrc, #8
    add pDst, pDst, #8

    # now do the line math

    add pSrc, pSrc, srcStep
    add pDst, pDst, dstStep

    subs linesLeft, linesLeft, #1
    bgt __3chan_tap_filter_image_3x1_fixed_line

    EPILOG r0, r10

.unreq tmp3
.unreq tmp2
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
