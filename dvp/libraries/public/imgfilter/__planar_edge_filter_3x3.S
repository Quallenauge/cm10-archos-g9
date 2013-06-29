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

.global __planar_edge_filter_3x3

.include "imgfilter.inc"

__planar_edge_filter_3x3:
/* areas where the coefficient is zero, thus useless, are not included */
width      .req r0
height     .req r1
pSrc       .req r2
srcStride  .req r3
pA         .req r4
pDst       .req r5
dstStride  .req r6
dstStep    .req r6
pixelsLeft .req r7
tmp        .req r8
range      .req r9
limit      .req r10
tmp1       .req r11

row0_u8    .req d0
row1_u8    .req d1
row2_u8    .req d2
dtmp1      .req d3

row0_s16   .req q2
row1_s16   .req q3
row2_s16   .req q4

Gx00       .req d10
/* no Gx01, it's zero */
Gx02       .req d11
Gx10       .req d12
/* no Gx11, it's zero */
Gx12       .req d13
Gx20       .req d14
/* no Gx21, it's zero */
Gx22       .req d15

Gy00       .req d16
Gy01       .req d17
Gy02       .req d18
/* no Gx10, it's zero */
/* no Gx11, it's zero */
/* no Gx12, it's zero */
Gy20       .req d19
Gy21       .req d20
Gy22       .req d21

dtmp2      .req d22
dtmp3      .req d23
qtmp0      .req q11 /* THIS OVERWRITES dtmp2 and dtmp3 !!! BE CAREFUL! */

qtmp1      .req q12
qtmp2      .req q13

GxA        .req q14
GyA        .req q15

GxA_00     .req d28
GxA_01     .req d29

GyA_00     .req d30
GyA_01     .req d31

    PROLOG      r0, r12

    /* load other parameters */
    ldr         pA,         [sp, #(14 * 4)]
    ldr         pDst,       [sp, #(15 * 4)]
    ldr         dstStride,  [sp, #(16 * 4)]
    ldr         range,      [sp, #(17 * 4)]
    ldr         limit,      [sp, #(18 * 4)]

    /* load the matricies */
    mov     tmp, #0
    vdup.8  dtmp3, tmp

    /*      {-pA[0], 0, pA[0]} */
    /* Gx = {-pA[1], 0, pA[1]} */
    /*      {-pA[2], 0, pA[2]} */

    /*      {-pA[0], -pA[1],-pA[2]} */
    /* Gy = {     0,      0,     0} */
    /*      { pA[0],  pA[1], pA[2]} */

    ldr     tmp, [pA]
    vdup.8  Gx02, tmp
    vsub.I8 Gx00, dtmp3, Gx02
    ldr     tmp, [pA, #4]
    vdup.8  Gx12, tmp
    vsub.I8 Gx10, dtmp3, Gx12
    ldr     tmp, [pA, #8]
    vdup.8  Gx22, tmp
    vsub.I8 Gx20, dtmp3, Gx22
    vmov.8  Gy00, Gx00
    vmov.8  Gy01, Gx10
    vmov.8  Gy02, Gx20
    vmov.8  Gy20, Gx02
    vmov.8  Gy21, Gx12
    vmov.8  Gy22, Gx22

    /* Move pDst to the second line and remove the first and last line from consideration */
    add     pDst, pDst, dstStride
    sub     height, #2

__planar_edge_filter_lines:

    /* reinitalize the row counter */
    mov         pixelsLeft, width
    sub         pixelsLeft, #2      /* the first and last pixel will not be processed */

__planar_edge_filter_pixels:

    /* preload the next block */
    add         tmp, pSrc, #L2_LINE_SIZE /* cache line size on A9 is 32 */
    pld         [tmp]
    add         tmp, tmp, srcStride
    pld         [tmp]
    add         tmp, tmp, srcStride
    pld         [tmp]

    /* initialize the accumulators */
    mov         tmp, #0
    vdup.16     GxA, tmp
    vdup.16     GyA, tmp

    /* load 8x3 pixels */
    /* expand pixels from middle colum */
    vld1.8      {row0_u8}, [pSrc]
    vmovl.U8    row0_s16, row0_u8
    add         tmp, pSrc, srcStride
    vld1.8      {row1_u8}, [tmp]
    vmovl.U8    row1_s16, row1_u8
    add         tmp, tmp, srcStride
    vld1.8      {row2_u8}, [tmp]
    vmovl.U8    row2_s16, row2_u8

    /* middle column second matrix MLA */
    vmovl.S8    qtmp1, Gy01
    vmovl.S8    qtmp2, Gy21
    vmla.I16    GyA, row0_s16, qtmp1
    vmla.I16    GyA, row2_s16, qtmp2

    /* create the shifted values for left column MLA */
    vshl.U64    dtmp1, row0_u8, #8
    vshl.U64    dtmp2, row1_u8, #8
    vshl.U64    dtmp3, row2_u8, #8
    vmovl.U8    row0_s16, dtmp1
    vmovl.U8    row1_s16, dtmp2
    vmovl.U8    row2_s16, dtmp3

    /* left column first matrix MLA */
    vshl.U64    dtmp1, Gx00, #8
    vshl.U64    dtmp2, Gx10, #8
    vshl.U64    dtmp3, Gx20, #8
    vmovl.S8    qtmp1, dtmp1
    vmovl.S8    qtmp2, dtmp2
    vmovl.S8    qtmp0, dtmp3
    vmla.I16    GxA, row0_s16, qtmp1
    vmla.I16    GxA, row1_s16, qtmp2
    vmla.I16    GxA, row2_s16, qtmp0

    /* left column second matrix MLA */
    vshl.U64    dtmp1, Gy00, #8
    vshl.U64    dtmp2, Gy20, #8
    vmovl.S8    qtmp1, dtmp1
    vmovl.S8    qtmp2, dtmp2
    vmla.I16    GyA, row0_s16, qtmp1
    vmla.I16    GyA, row2_s16, qtmp2

    /* create the shifted values for right column MLA */
    vshr.U64    dtmp1, row0_u8, #8
    vshr.U64    dtmp2, row1_u8, #8
    vshr.U64    dtmp3, row2_u8, #8
    vmovl.U8    row0_s16, dtmp1
    vmovl.U8    row1_s16, dtmp2
    vmovl.U8    row2_s16, dtmp3

    /* right column first matrix MLA */
    vshr.U64    dtmp1, Gx02, #8
    vshr.U64    dtmp2, Gx12, #8
    vshr.U64    dtmp3, Gx22, #8
    vmovl.S8    qtmp1, dtmp1
    vmovl.S8    qtmp2, dtmp2
    vmovl.S8    qtmp0, dtmp3
    vmla.I16    GxA, row0_s16, qtmp1
    vmla.I16    GxA, row1_s16, qtmp2
    vmla.I16    GxA, row2_s16, qtmp0

    /* right column second matrix MLA */
    vshr.U64    dtmp1, Gy02, #8
    vshr.U64    dtmp2, Gy22, #8
    vmovl.S8    qtmp1, dtmp1
    vmovl.S8    qtmp2, dtmp2
    vmla.I16    GyA, row0_s16, qtmp1
    vmla.I16    GyA, row2_s16, qtmp2

    /* square each gradient and expand each field from 16 to 32 */
    vmull.S16   q0, GxA_00, GxA_00
    vmull.S16   q1, GxA_01, GxA_01
    vmull.S16   q2, GyA_00, GyA_00
    vmull.S16   q3, GyA_01, GyA_01

    /* sum top and bottom */
    vadd.S32    q0, q0, q2
    vadd.S32    q1, q1, q3

    /* convert to float */
    vcvt.F32.S32 q0, q0
    vcvt.F32.S32 q1, q1

    /* Square Root these Sum of Squares */
    SQRTF        q0, q2, q3, q4
    SQRTF        q1, q2, q3, q4

    /* load the range and limit */
    vdup.32     q2, range
    vcvt.F32.S32 q2, q2
    vdup.32     q3, limit
    vcvt.F32.S32 q3, q3

    /* divide by the range */
    vdiv.F32    s0, s0, s8
    vdiv.F32    s1, s1, s9
    vdiv.F32    s2, s2, s10
    vdiv.F32    s3, s3, s11
    vdiv.F32    s4, s4, s8
    vdiv.F32    s5, s5, s9
    vdiv.F32    s6, s6, s10
    vdiv.F32    s7, s7, s11

    /* multiply by the limit */
    vmul.F32    s0, s0, s12
    vmul.F32    s1, s1, s13
    vmul.F32    s2, s2, s14
    vmul.F32    s3, s3, s15
    vmul.F32    s4, s4, s12
    vmul.F32    s5, s5, s13
    vmul.F32    s6, s6, s14
    vmul.F32    s7, s7, s15

    /* convert back to int */
    vcvt.S32.F32 q0, q0
    vcvt.S32.F32 q1, q1

    /* move back down to U8 (32->16->8) */
    vqmovn.U32   d0, q0
    vqmovn.U32   d1, q1
    vqmovn.U16   d0, q0

    /* store */
    add         tmp, pDst, #1
    vst1.8      {d0[1]}, [tmp]!
    vst1.8      {d0[2]}, [tmp]!
    vst1.8      {d0[3]}, [tmp]!
    vst1.8      {d0[4]}, [tmp]!
    vst1.8      {d0[5]}, [tmp]!
    vst1.8      {d0[6]}, [tmp]!

    /* LOOP LOGIC */
    /*
    pixelsLeft -= 6;
    if (pixelsLeft == 0) {
        pSrc += 8;
        pDst += 8;
        pixelsLeft = 0; // pointless
        break;
    } else if (pixelsLeft >= 6) {
        pSrc += 6;
        pDst += 6;
        continue;
    } else if (pixelsLeft < 6) {
        pSrc -= 6 - pixelsLeft;
        pDst -= 6 - pixelsLeft;
        pixelsLeft = 6;
        continue;
    } */
    subs        pixelsLeft, pixelsLeft, #6
    addeq       pSrc, pSrc, #8
    addeq       pDst, pDst, #8
    /* moveq       pixelsLeft, #0 */
    beq         __planar_edge_filter_pixels_end
    cmp         pixelsLeft, #6
    addge       pSrc, pSrc, #6
    addge       pDst, pDst, #6
    bge         __planar_edge_filter_pixels
    movlt       tmp, #6
    sublt       tmp, tmp, pixelsLeft
    sublt       pSrc, pSrc, tmp
    sublt       pDst, pDst, tmp
    movlt       pixelsLeft, #6
    blt         __planar_edge_filter_pixels


__planar_edge_filter_pixels_end:
    /* calculate the steps and apply */
    sub         tmp,  srcStride, width
    add         pSrc, pSrc, tmp
    sub         tmp,  dstStride, width
    add         pDst, pDst, tmp

    /* loop the lines */
    subs        height, height, #1
    bgt         __planar_edge_filter_lines

    EPILOG      r0, r12
.unreq width
.unreq height
.unreq pSrc
.unreq srcStride
.unreq pA
.unreq pDst
.unreq dstStride
.unreq pixelsLeft
.unreq tmp
.unreq dstStep

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


/* Assumes input is a 3 pixel channel source like xRGB where the first byte is ignored.
 * uses fixed coefficients of -0.25, 1.5, -0.25. or a[0]>>2, a[1]*3>>1, a[2]>>2
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
    ldrb tmp, [pSrc, #0]
    strb tmp, [pDst, #0]

    ldrb tmp, [pSrc, #1]
    add tmp2, tmp, tmp, lsr #1
    sub tmp2, tmp2, tmp, lsr #2
    ldrb tmp, [pSrc, #5]
    sub tmp2, tmp2, tmp, lsr #2
    strb tmp2, [pDst, #1]

    ldrb tmp, [pSrc, #2]
    add tmp2, tmp, tmp, lsr #1
    sub tmp2, tmp2, tmp, lsr #2
    ldrb tmp, [pSrc, #6]
    sub tmp2, tmp2, tmp, lsr #2
    strb tmp2, [pDst, #2]

    ldrb tmp, [pSrc, #3]
    add tmp2, tmp, tmp, lsr #1
    sub tmp2, tmp2, tmp, lsr #2
    ldrb tmp, [pSrc, #7]
    sub tmp2, tmp2, tmp, lsr #2
    strb tmp2, [pDst, #3]

__3chan_tap_filter_image_3x1_fixed_pixels:

    pld [pSrc, tmp]

    vld4.8 {d0,d1,d2,d3}, [pSrc]

    /* ignore d0, the 'x' */

    /* d1>>2=d4, d2>>2=d5, d3>>2=d6 */
    vshr.U8 d4, d1, #2
    vshr.U8 d5, d2, #2
    vshr.U8 d6, d3, #2

    /* d1*3/2=q13, d2*3/2=q14, d3*3/2=q15 */
    vmov.U8 d16, #3
    vmull.U16 q13, d16, d1
    vmull.U16 q14, d16, d2
    vmull.U16 q15, d16, d3

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
    vsub.U16 q2, q13, q7
    vsub.U16 q2, q2, q10
    vsub.U16 q3, q14, q8
    vsub.U16 q3, q3, q11
    vsub.U16 q4, q15, q9
    vsub.U16 q4, q4, q12

    /* narrow back down */
    vqmovn.U16 d1, q2
    vqmovn.U16 d2, q3
    vqmovn.U16 d3, q4

    add tmp, pDst, #4

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

    ldrb tmp, [pSrc, #1]
    sub tmp2, tmp2, tmp, lsr #2
    ldrb tmp, [pSrc, #5]
    add tmp2, tmp, tmp, lsr #1
    sub tmp2, tmp2, tmp, lsr #2
    strb tmp2, [pDst, #1]

    ldrb tmp, [pSrc, #2]
    sub tmp2, tmp2, tmp, lsr #2
    ldrb tmp, [pSrc, #6]
    add tmp2, tmp, tmp, lsr #1
    sub tmp2, tmp2, tmp, lsr #2
    strb tmp2, [pDst, #2]

    ldrb tmp, [pSrc, #3]
    sub tmp2, tmp2, tmp, lsr #2
    ldrb tmp, [pSrc, #7]
    add tmp2, tmp, tmp, lsr #1
    sub tmp2, tmp2, tmp, lsr #2
    strb tmp2, [pDst, #3]

    add pSrc, pSrc, #8
    add pDst, pDst, #8

    add pSrc, pSrc, srcStep
    add pDst, pDst, dstStep

    subs linesLeft, linesLeft, #1
    bgt __3chan_tap_filter_image_3x1_fixed_line

    EPILOG r0, r8

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
