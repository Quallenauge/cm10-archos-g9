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

.global __uyvy_rotate_ccw90

.include "yuv.inc"

/**
 * These indexes allow the YUV422 CW90 rotate for UV values to work, use with VTBX
 * TABLE (LSB)
 * d[n+0] = XX Y0 XX Y1 XX Y2 XX Y3
 * d[n+1] = XX Y8 XX Y9 XX YA XX YB
 * d[n+2] = U0 XX V0 XX U2 XX V2 XX
 */
XYXY_CCW90_INDEXES:
/* (MSB) */
    .long 0x0F160710  /*          LSB                          MSB */
    .long 0xFFFFFFFF  /* d[m+0] = [10][07][16][0F][XX][XX][XX][XX] */
    .long 0x0D160510
    .long 0xFFFFFFFF  /* d[m+1] = [10][05][16][0D][XX][XX][XX][XX] */
    .long 0x0B120314
    .long 0xFFFFFFFF  /* d[m+2] = [14][03][12][0B][XX][XX][XX][XX] */
    .long 0x09120114
    .long 0xFFFFFFFF  /* d[m+3] = [14][01][12][09][XX][XX][XX][XX] */
/* (MSB) */
    .long 0xFFFFFFFF  /*          LSB                          MSB */
    .long 0x0F160710  /* d[m+4] = [XX][XX][XX][XX][10][07][16][0F] */
    .long 0xFFFFFFFF
    .long 0x0D160510  /* d[m+5] = [XX][XX][XX][XX][10][05][16][0D] */
    .long 0xFFFFFFFF
    .long 0x0B120314  /* d[m+6] = [XX][XX][XX][XX][14][03][12][0B] */
    .long 0xFFFFFFFF
    .long 0x09120114  /* d[m+7] = [XX][XX][XX][XX][14][01][12][09] */

/**
 * This subroutine rotates YUV422I data in 4x8 pixel blocks.
 */
__uyvy_rotate_ccw90:
width       .req r0
dstHeight   .req r0
height      .req r1
dstWidth    .req r1
pSrc        .req r2
pDst        .req r3
srcStride   .req r4
dstStride   .req r5
pixelsLeft  .req r6
linesLeft   .req r7
tmp         .req r8
pSrcTmp     .req r9
pDstTmp     .req r10
count       .req r11
        PROLOG r0, r12
        ldr     srcStride, [sp, #(14 * 4)]
        ldr     dstStride, [sp, #(15 * 4)]
        mov     pixelsLeft, width
        mov     linesLeft, height
        mov     count, #0

        ldr     tmp, =XYXY_CCW90_INDEXES
        vld1.u8  {d12}, [tmp]!
        vld1.u8  {d13}, [tmp]!
        vld1.u8  {d14}, [tmp]!
        vld1.u8  {d15}, [tmp]!
        vld1.u8  {d16}, [tmp]!
        vld1.u8  {d17}, [tmp]!
        vld1.u8  {d18}, [tmp]!
        vld1.u8  {d19}, [tmp]!

        /* move the pDst to the bottom of the first column */
        sub     tmp, dstHeight, #1
        mul     tmp, tmp, dstStride
        add     pDst, pDst, tmp

        /* invert the dstStride */
        mov     tmp, #0
        sub     dstStride, tmp, dstStride

__uyvy_rotate_ccw90_height:

        /* move the Destination pointer to it's location */
        sub     tmp, height, linesLeft
        mov     tmp, tmp, lsl #1
        add     pDstTmp, pDst, tmp

        /* reset the number of pixels left on each line */
        mov     pixelsLeft, width

__uyvy_rotate_ccw90_width:

        mov     pSrcTmp, pSrc

        /* preload the data */
        pld     [pSrcTmp]
        pld     [pSrcTmp, srcStride]

        /* load the line */
        vld1.u8  {d0,d1}, [pSrcTmp], srcStride
        vld1.u8  {d3,d4}, [pSrcTmp], srcStride

        /* preload the next 2 lines */
        pld     [pSrcTmp]
        pld     [pSrcTmp, srcStride]

        /* create the average U/V's */
        vswp.8   d1, d3 /* prep the data */
        vhadd.u8 d2, d0, d1
        vhadd.u8 d5, d3, d4

        /* index the data into the destination YUV422 buffer */
        vtbx.8  d24, {d0,d1,d2}, d12
        vtbx.8  d25, {d0,d1,d2}, d13
        vtbx.8  d26, {d0,d1,d2}, d14
        vtbx.8  d27, {d0,d1,d2}, d15
        /* index the second half of the output */
        vtbx.8  d20, {d3,d4,d5}, d12
        vtbx.8  d21, {d3,d4,d5}, d13
        vtbx.8  d22, {d3,d4,d5}, d14
        vtbx.8  d23, {d3,d4,d5}, d15

        /* load the next set of input data */
        vld1.u8 {d0,d1}, [pSrcTmp], srcStride
        vld1.u8 {d3,d4}, [pSrcTmp], srcStride

        /* create the average U/V's */
        vswp.8   d1, d3 /* prep the data */
        vhadd.u8 d2, d0, d1
        vhadd.u8 d5, d3, d4

        /* index the data into the destination YUV422 buffer */
        vtbx.8  d24, {d0,d1,d2}, d16
        vtbx.8  d25, {d0,d1,d2}, d17
        vtbx.8  d26, {d0,d1,d2}, d18
        vtbx.8  d27, {d0,d1,d2}, d19
        /* index the second half of the output */
        vtbx.8  d20, {d3,d4,d5}, d16
        vtbx.8  d21, {d3,d4,d5}, d17
        vtbx.8  d22, {d3,d4,d5}, d18
        vtbx.8  d23, {d3,d4,d5}, d19

        /* store the rotated data (and increment the destintation pointer) */
        vst1.8  {d27}, [pDstTmp], dstStride
        vst1.8  {d26}, [pDstTmp], dstStride
        vst1.8  {d25}, [pDstTmp], dstStride
        vst1.8  {d24}, [pDstTmp], dstStride
        vst1.8  {d23}, [pDstTmp], dstStride
        vst1.8  {d22}, [pDstTmp], dstStride
        vst1.8  {d21}, [pDstTmp], dstStride
        vst1.8  {d20}, [pDstTmp], dstStride

        add     pSrc, pSrc, #16
        add     count, count, #1

        subs    pixelsLeft, pixelsLeft, #8
        bgt     __uyvy_rotate_ccw90_width

        /* wrap the source pointer to the next line */
        mov     tmp, width, lsl #1
        sub     tmp, srcStride, tmp     /* tmp = srcStride - (width << 1) */
        add     pSrc, pSrc, tmp         /* pSrc += tmp */
        add     tmp, srcStride, srcStride
        add     tmp, tmp, srcStride
        add     pSrc, pSrc, tmp         /* move the line 3 lines down */

        subs    linesLeft, linesLeft, #4
        bgt     __uyvy_rotate_ccw90_height

        EPILOG r0, r12
.unreq width
.unreq dstHeight
.unreq height
.unreq dstWidth
.unreq pSrc
.unreq pDst
.unreq srcStride
.unreq dstStride
.unreq pixelsLeft
.unreq linesLeft
.unreq tmp
.unreq pSrcTmp
.unreq pDstTmp
.unreq count

.end
