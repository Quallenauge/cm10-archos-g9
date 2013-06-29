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

.global __3chan_tap_filter_image_3x1_fixed_k

.include "imgfilter.inc"

/* Assumes input is a 3 pixel channel source like xRGB where the first byte is ignored.
 * uses symmetric fractional coefficients = [-k/64, (64+2k)/64, -k/64] where
 * 0 <= k <= 64. If a value of 11 is used this reduces to
 * [-8/64, (64+16)/64, -8/64] => [-0.125, 1.25, -0.125]. The benifit of this
 * reduced range is that the math never exceeds 16 bit representation.
 */
__3chan_tap_filter_image_3x1_fixed_k:
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
k1         .req r10
k          .req r11

    PROLOG r0, r11

    /* load the strides */
    ldr srcStride, [sp, #(13 * 4)]
    ldr dstStride, [sp, #(14 * 4)]
    ldr k,         [sp, #(15 * 4)]

    /* clamp k to 64, and make the (1+2k) version */
    mov k1, #64
    cmp k, k1
    movgt k, #64
    mov tmp2, #2
    smlabb k1, k, tmp2, k1
    vdup.U08  d28, k
    vdup.U16  q15, k1

    /* initialize the height counter */
    mov linesLeft, height

    /* make tmp = width * 4; make the strides into steps */
    mov tmp, width, lsl #2
    sub srcStep, srcStride, tmp
    sub dstStep, dstStride, tmp

__3chan_tap_filter_image_3x1_fixed_k_line:

    mov pixelsLeft, width
    sub pixelsLeft, #2

    pld [pSrc]
    mov tmp, #L2_LINE_SIZE
    pld [pSrc, tmp]
    add tmp, tmp, #L2_LINE_SIZE
    pld [pSrc, tmp]
    add tmp, tmp, #L2_LINE_SIZE

.ifeq SUPPORT_EDGES
    # do the first pixel set from ARMv6
    ldrb tmp, [pSrc]
    strb tmp, [pDst]

    ldrb tmp, [pSrc, #1]
    mul  tmp2, tmp, k1          /* tmp2 = tmp*tmp3 */
    ldrb tmp, [pSrc, #5]
    mul  tmp, tmp, k            /* tmp = tmp*k */
    sub  tmp, tmp2, tmp         /* tmp = tmp2-tmp */
    usat tmp2, #8, tmp, asr #6  /* tmp = usat(tmp>>6, 8) */
    strb tmp2, [pDst, #1]

    ldrb tmp, [pSrc, #2]
    mul  tmp2, tmp, k1          /* tmp2 = tmp*tmp3 */
    ldrb tmp, [pSrc, #6]
    mul  tmp, tmp, k            /* tmp = tmp*k */
    sub  tmp, tmp2, tmp         /* tmp = tmp2-tmp */
    usat tmp2, #8, tmp, asr #6  /* tmp = usat(tmp>>6, 8) */
    strb tmp2, [pDst, #2]

    ldrb tmp, [pSrc, #3]
    mul  tmp2, tmp, k1          /* tmp2 = tmp*tmp3 */
    ldrb tmp, [pSrc, #7]
    mul  tmp, tmp, k            /* tmp = tmp*k */
    sub  tmp, tmp2, tmp         /* tmp = tmp2-tmp */
    usat tmp2, #8, tmp, asr #6  /* tmp = usat(tmp>>6, 8) */
    strb tmp2, [pDst, #3]
.endif

__3chan_tap_filter_image_3x1_fixed_k_pixels:

    mov tmp, #L2_LINE_SIZE
    add tmp, tmp, tmp, lsl #1 /* tmp = 3*L2size */
    pld [pSrc, tmp]

    vld4.8 {d0,d1,d2,d3}, [pSrc]

    /*********************************/
    /*     |00|01|02|03|04|05|06|07| */
    /*     |-----------------------| */
    /* d00 |a0|a1|a2|a3|a4|a5|a6|a7| */
    /* d01 |r0|r1|r2|r3|r4|r5|r6|r7| */
    /* d02 |g0|g1|g2|g3|g4|g5|g6|g7| */
    /* d03 |b0|b1|b2|b3|b4|b5|b6|b7| */
    /* d04 |   r0|   r1|   r2|   r3| */
    /* d05 |   r4|   r5|   r6|   r7| */
    /* d06 |   g0|   g1|   g2|   g3| */
    /* d07 |   g4|   g5|   g6|   g7| */
    /* d08 |   b0|   b1|   b2|   b3| */
    /* d09 |   b4|   b5|   b6|   b7| */
    /* d10 |r0*k1|r1*k1|r2*k1|r3*k1| */
    /* d11 |r4*k1|r5*k1|r6*k1|r7*k1| */
    /* d12 |g0*k1|g1*k1|g2*k1|g3*k1| */
    /* d13 |g4*k1|g5*k1|g6*k1|g7*k1| */
    /* d14 |b0*k1|b1*k1|b2*k1|b3*k1| */
    /* d15 |b4*k1|b5*k1|b6*k1|b7*k1| */
    /* d16 | r1*k| r2*k| r3*k| r4*k| */
    /* d17 | r5*k| r6*k| r7*k| 00*k| */
    /* d18 | g1*k| g2*k| g3*k| g4*k| */
    /* d19 | g5*k| g6*k| g7*k| 00*k| */
    /* d20 | b1*k| b2*k| b3*k| b4*k| */
    /* d21 | b5*k| b6*k| b7*k| 00*k| */
    /* d22 | 00*k| r0*k| r1*k| r2*k| */
    /* d23 | r3*k| r4*k| r5*k| r6*k| */
    /* d24 | 00*k| g0*k| g1*k| g2*k| */
    /* d25 | g3*k| g4*k| g5*k| g6*k| */
    /* d26 | 00*k| b0*k| b1*k| b2*k| */
    /* d27 | b3*k| b4*k| b5*k| b6*k| */
    /* d28 | k| k| k| k| k| k| k| k| */
    /* d29 ... */
    /* d30 |   k1|   k1|   k1|   k1| */
    /* d31 |   k1|   k1|   k1|   k1| */
    /*********************************/

    /* expand dx to qx */
    vmovl.U08   q2, d1
    vmovl.U08   q3, d2
    vmovl.U08   q4, d3

    /* multiply k1*qx */
    vmul.U16    q5,  q2,  q15
    vmul.U16    q6,  q3,  q15
    vmul.U16    q7,  q4,  q15

    /* roll dx to the right */
    vshr.U64    d16, d1, #8
    vshr.U64    d18, d2, #8
    vshr.U64    d20, d3, #8

    /* roll dx to the left */
    vshl.U64    d22, d1, #8
    vshl.U64    d24, d2, #8
    vshl.U64    d26, d3, #8

    /* multiply k*dx (right) and upconvert */
    vmull.U08   q8,  d16, d28
    vmull.U08   q9,  d18, d28
    vmull.U08   q10, d20, d28

    /* multiply k*dx (left) and upconvert */
    vmull.U08   q11, d22, d28
    vmull.U08   q12, d24, d28
    vmull.U08   q13, d26, d28

    /* subtract qx[-1,1] from qx[0] */
    vqsub.S16   q5,  q5,  q8
    vqsub.S16   q6,  q6,  q9
    vqsub.S16   q7,  q7,  q10
    vqsub.S16   q5,  q5,  q11
    vqsub.S16   q6,  q6,  q12
    vqsub.S16   q7,  q7,  q13

    /* narrow qx to dx */
    vqrshrun.S16   d1, q5, #6
    vqrshrun.S16   d2, q6, #6
    vqrshrun.S16   d3, q7, #6

.ifeq SUPPORT_EDGES
    add         tmp, pDst, #4
.endif

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

.ifeq SUPPORT_EDGES
    subs        pixelsLeft, pixelsLeft, #6
    addeq       pSrc, pSrc, #8*4
    addeq       pDst, pDst, #8*4
    beq         __3chan_tap_filter_image_3x1_fixed_k_pixels_end
    cmp         pixelsLeft, #6
    addge       pSrc, pSrc, #6*4
    addge       pDst, pDst, #6*4
    bge         __3chan_tap_filter_image_3x1_fixed_k_pixels
    /* if there are less than 6 left, back up and redo part of them */
    movlt       tmp, #6
    sublt       tmp, tmp, pixelsLeft
    sublt       pSrc, pSrc, tmp, lsl #2
    sublt       pDst, pDst, tmp, lsl #2
    movlt       pixelsLeft, #6
    blt         __3chan_tap_filter_image_3x1_fixed_k_pixels
.endif

__3chan_tap_filter_image_3x1_fixed_k_pixels_end:

.ifeq SUPPORT_EDGES
    # do the last pixel set from ARMv6
    sub pSrc, pSrc, #8
    sub pDst, pDst, #8

    ldrb tmp, [pSrc, #4]
    strb tmp, [pDst, #4]

    ldrb tmp, [pSrc, #1]
    mul tmp2, tmp, k
    ldrb tmp, [pSrc, #5]
    mul tmp, tmp, k1
    sub tmp, tmp, tmp2
    usat tmp, #8, tmp, asr #6
    strb tmp, [pDst, #1]

    ldrb tmp2, [pSrc, #2]
    mul tmp2, tmp, k
    ldrb tmp2, [pSrc, #6]
    mul tmp, tmp, k1
    sub tmp, tmp, tmp2
    usat tmp, #8, tmp, asr #6
    strb tmp, [pDst, #2]

    ldrb tmp2, [pSrc, #3]
    mul tmp2, tmp, k
    ldrb tmp2, [pSrc, #7]
    mul tmp, tmp, k1
    sub tmp, tmp, tmp2
    usat tmp, #8, tmp, asr #6
    strb tmp, [pDst, #3]
.endif

    add pSrc, pSrc, #8
    add pDst, pDst, #8

    # now do the line math

    add pSrc, pSrc, srcStep
    add pDst, pDst, dstStep

    subs linesLeft, linesLeft, #1
    bgt __3chan_tap_filter_image_3x1_fixed_k_line

    EPILOG r0, r11

.unreq k
.unreq k1
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
