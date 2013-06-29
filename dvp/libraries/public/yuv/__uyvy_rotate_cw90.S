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

.global __uyvy_rotate_cw90

.include "yuv.inc"

/**
 * These indexes allow the YUV422 CW90 rotate for Y values to work, use with VTBX
 * TABLE (LSB)
 * d[n+0] = Y0 Y2 Y4 Y6 Y8 YA YC YE = 00 02 04 06 08 0A 0C 0E
 * d[n+1] = Y1 Y3 Y5 Y7 Y9 YB YD YF = 01 03 05 07 09 0B 0D 0F
 * d[n+2] = Y0 Y2 Y4 Y6 Y8 YA YC YE = 10 12 14 16 18 1A 1C 1E
 * d[n+3] = Y1 Y3 Y5 Y7 Y9 YB YD YF = 11 13 15 17 19 1B 1D 1F
 *
 * d[m+0] = XX 18 XX 10 XX 08 XX 00
 * d[m+1] = XX 19 XX 11 XX 09 XX 01
 * d[m+2] = XX 1A XX 12 XX 0A XX 02
 * d[m+3] = XX 1B XX 13 XX 0B XX 03
 *
 * d[m+4] = XX 1C XX 14 XX 0C XX 04
 * d[m+5] = XX 1D XX 15 XX 0D XX 05
 * d[m+6] = XX 1E XX 16 XX 0E XX 06
 * d[m+7] = XX 1F XX 17 XX 0F XX 07
 */
Y_CW90_INDEXES:
/* (MSB) */
    .long 0x00FF08FF
    .long 0x10FF18FF
    .long 0x01FF09FF
    .long 0x11FF19FF
    .long 0x02FF0AFF
    .long 0x12FF1AFF
    .long 0x03FF0BFF
    .long 0x13FF1BFF

    .long 0x04FF0CFF
    .long 0x14FF1CFF
    .long 0x05FF0DFF
    .long 0x15FF1DFF
    .long 0x06FF0EFF
    .long 0x16FF1EFF
    .long 0x07FF0FFF
    .long 0x17FF1FFF

/**
 * These indexes allow the YUV422 CW90 rotate for UV values to work, use with VTBX
 * TABLE (LSB)
 * d[n+0] = U0 U1 U2 U3 U4 U5 U6 U7 (averaged
 * d[n+1] = U8 U9 UA UB UC UD UE UF
 * d[n+2] = V0 V1 V2 V3 V4 V5 V6 V7
 * d[n+3] = V8 V9 VA VB VC VD VE VF
 *
 * d[m+0] = XX 18 XX 10 XX 08 XX 00
 * d[m+1] = XX 19 XX 11 XX 09 XX 01
 * d[m+2] = XX 1A XX 12 XX 0A XX 02
 * d[m+3] = XX 1B XX 13 XX 0B XX 03
 *
 * d[m+4] = XX 1C XX 14 XX 0C XX 04
 * d[m+5] = XX 1D XX 15 XX 0D XX 05
 * d[m+6] = XX 1E XX 16 XX 0E XX 06
 * d[m+7] = XX 1F XX 17 XX 0F XX 07
 */
UV_CW90_INDEXES:
/* (MSB) */
    .long 0x00FF08FF
    .long 0x10FF18FF
    .long 0x01FF09FF
    .long 0x11FF19FF
    .long 0x02FF0AFF
    .long 0x12FF1AFF
    .long 0x03FF0BFF
    .long 0x13FF1BFF

    .long 0x04FF0CFF
    .long 0x14FF1CFF
    .long 0x05FF0DFF
    .long 0x15FF1DFF
    .long 0x06FF0EFF
    .long 0x16FF1EFF
    .long 0x07FF0FFF
    .long 0x17FF1FFF

/**
 * These indexes allow the YUV422 CW90 rotate for UV values to work, use with VTBX
 * TABLE (LSB)
 * d[n+0] = XX Y0 XX Y1 XX Y2 XX Y3
 * d[n+1] = XX Y8 XX Y9 XX YA XX YB
 * d[n+2] = U0 XX V0 XX U2 XX V2 XX
 */
XYXY_CW90_INDEXES:
/* (MSB) */
    .long 0xFFFFFFFF  /*          LSB                          MSB */
    .long 0x01120910  /* d[m+0] = [XX][XX][XX][XX][10][09][12][01] */
    .long 0xFFFFFFFF
    .long 0x03120B10  /* d[m+1] = [XX][XX][XX][XX][10][0B][12][03] */
    .long 0xFFFFFFFF
    .long 0x05160D14  /* d[m+2] = [XX][XX][XX][XX][14][0D][16][05] */
    .long 0xFFFFFFFF
    .long 0x07160F14  /* d[m+3] = [XX][XX][XX][XX][14][0F][16][07] */
/* (MSB) */
    .long 0x01120910  /*          LSB                          MSB */
    .long 0xFFFFFFFF  /* d[m+4] = [10][09][12][01][XX][XX][XX][XX] */
    .long 0x03120B10
    .long 0xFFFFFFFF  /* d[m+5] = [10][0B][12][03][XX][XX][XX][XX] */
    .long 0x05160D14
    .long 0xFFFFFFFF  /* d[m+6] = [14][0D][16][05][XX][XX][XX][XX] */
    .long 0x07160F14
    .long 0xFFFFFFFF  /* d[m+7] = [14][0F][16][07][XX][XX][XX][XX] */

/**
 * This subroutine rotates YUV422I data in 4x8 pixel blocks.
 */
__uyvy_rotate_cw90:
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
        PROLOG  r0, r12
        ldr     srcStride, [sp, #(14 * 4)]
        ldr     dstStride, [sp, #(15 * 4)]
        mov     pixelsLeft, width
        mov     linesLeft, height
        mov     count, #0

        ldr     tmp, =XYXY_CW90_INDEXES
        vld1.u8  {d12}, [tmp]!
        vld1.u8  {d13}, [tmp]!
        vld1.u8  {d14}, [tmp]!
        vld1.u8  {d15}, [tmp]!
        vld1.u8  {d16}, [tmp]!
        vld1.u8  {d17}, [tmp]!
        vld1.u8  {d18}, [tmp]!
        vld1.u8  {d19}, [tmp]!


__uyvy_rotate_cw90_height:

        /* move the Destination pointer to it's location */
        mov     tmp, linesLeft, lsl #1
        sub     tmp, tmp, #8
        add     pDstTmp, pDst, tmp

        /* reset the number of pixels left on each line */
        mov     pixelsLeft, width

__uyvy_rotate_cw90_width:

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
        vtbx.8  d20, {d0,d1,d2}, d12
        vtbx.8  d21, {d0,d1,d2}, d13
        vtbx.8  d22, {d0,d1,d2}, d14
        vtbx.8  d23, {d0,d1,d2}, d15
        /* index the second half of the output */
        vtbx.8  d24, {d3,d4,d5}, d12
        vtbx.8  d25, {d3,d4,d5}, d13
        vtbx.8  d26, {d3,d4,d5}, d14
        vtbx.8  d27, {d3,d4,d5}, d15

        /* load the next set of input data */
        vld1.u8 {d0,d1}, [pSrcTmp], srcStride
        vld1.u8 {d3,d4}, [pSrcTmp], srcStride

        /* create the average U/V's */
        vswp.8   d1, d3 /* prep the data */
        vhadd.u8 d2, d0, d1
        vhadd.u8 d5, d3, d4

        /* index the data into the destination YUV422 buffer */
        vtbx.8  d20, {d0,d1,d2}, d16
        vtbx.8  d21, {d0,d1,d2}, d17
        vtbx.8  d22, {d0,d1,d2}, d18
        vtbx.8  d23, {d0,d1,d2}, d19
        /* index the second half of the output */
        vtbx.8  d24, {d3,d4,d5}, d16
        vtbx.8  d25, {d3,d4,d5}, d17
        vtbx.8  d26, {d3,d4,d5}, d18
        vtbx.8  d27, {d3,d4,d5}, d19

        /* store the rotated data (and increment the destintation pointer) */
        vst1.8  {d20}, [pDstTmp], dstStride
        vst1.8  {d21}, [pDstTmp], dstStride
        vst1.8  {d22}, [pDstTmp], dstStride
        vst1.8  {d23}, [pDstTmp], dstStride
        vst1.8  {d24}, [pDstTmp], dstStride
        vst1.8  {d25}, [pDstTmp], dstStride
        vst1.8  {d26}, [pDstTmp], dstStride
        vst1.8  {d27}, [pDstTmp], dstStride

        add     pSrc, pSrc, #16
        add     count, count, #1

        subs    pixelsLeft, pixelsLeft, #8
        bgt     __uyvy_rotate_cw90_width

        /* wrap the source pointer to the next line */
        mov     tmp, width, lsl #1
        sub     tmp, srcStride, tmp     /* tmp = srcStride - (width << 1) */
        add     pSrc, pSrc, tmp         /* pSrc += tmp */
        add     tmp, srcStride, srcStride
        add     tmp, tmp, srcStride
        add     pSrc, pSrc, tmp         /* move the line 3 lines down */

        subs    linesLeft, linesLeft, #4
        bgt     __uyvy_rotate_cw90_height

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
