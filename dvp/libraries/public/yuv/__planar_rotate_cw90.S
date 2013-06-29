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

.global __planar_rotate_cw90

.include "yuv.inc"

/** These are the VTBL constants for a 90 clockwise rotation. */
CLOCKWISE_90_ROTATE:
    .long 0x00081018
    .long 0x01091119
    .long 0x020A121A
    .long 0x030B131B
    .long 0x040C141C
    .long 0x050D151D
    .long 0x060E161E
    .long 0x070F171F

/**
 * This subroutine rotates YUV420P data in 8x8 byte blocks. This means that the
 * U,V fields will be rotated this way as well which means that the image
 * diensions must be a multiple of 16.
 */
__planar_rotate_cw90:
width       .req r0
dstHeight   .req r0
height      .req r1
dstWidth    .req r1
pSrc        .req r2
pDst        .req r3
pixelsLeft  .req r4
linesLeft   .req r5
tmp         .req r6
dstStride   .req r7
pTmp1       .req r8
pTmp2       .req r9
pTmp3       .req r10
pTmp4       .req r11
count       .req r12
        PROLOG r0, r12

        mov     pixelsLeft, width
        mov     linesLeft, height

        ldr     tmp, =CLOCKWISE_90_ROTATE
        vld1.u8  {d8}, [tmp]!
        vld1.u8  {d9}, [tmp]!
        vld1.u8  {d10},[tmp]!
        vld1.u8  {d11},[tmp]!

        /* dstStride = height# */
        mov     dstStride, height

__planar_rotate_cw90_height:

        /* move the pTmp4 to the correct column of the pDst. */
        sub     tmp, linesLeft, #8
        add     pTmp4, pDst, tmp

        /* reset pixelsLeft */
        mov     pixelsLeft, width

__planar_rotate_cw90_width:

        mov     pTmp2, pSrc
        mov     pTmp3, pSrc
        preload_L2_4x64bytes pTmp3, width, tmp

        /* load the first half of the block */
        vld1.u8  {d0}, [pTmp2], width
        vld1.u8  {d1}, [pTmp2], width
        vld1.u8  {d2}, [pTmp2], width
        vld1.u8  {d3}, [pTmp2], width

        mov     pTmp3, pTmp2
        preload_L2_4x64bytes pTmp3, width, tmp

        /* start the transpose */
        vtbl.8  d13, {d0,d1,d2,d3}, d8
        vtbl.8  d15, {d0,d1,d2,d3}, d9
        vtbl.8  d17, {d0,d1,d2,d3}, d10
        vtbl.8  d19, {d0,d1,d2,d3}, d11

        /* load the second half of the block */
        vld1.u8  {d4}, [pTmp2], width
        vld1.u8  {d5}, [pTmp2], width
        vld1.u8  {d6}, [pTmp2], width
        vld1.u8  {d7}, [pTmp2]

        /* complete the transpose of the block indexes */
        vtbl.8  d12, {d4,d5,d6,d7}, d8
        vtbl.8  d14, {d4,d5,d6,d7}, d9
        vtbl.8  d16, {d4,d5,d6,d7}, d10
        vtbl.8  d18, {d4,d5,d6,d7}, d11

        /* swap the indexes around */
        vtrn.32 d12, d13
        vtrn.32 d14, d15
        vtrn.32 d16, d17
        vtrn.32 d18, d19

        /* now store the rotated data */
        vst1.8  {d12}, [pTmp4], dstStride
        vst1.8  {d13}, [pTmp4], dstStride
        vst1.8  {d14}, [pTmp4], dstStride
        vst1.8  {d15}, [pTmp4], dstStride
        vst1.8  {d16}, [pTmp4], dstStride
        vst1.8  {d17}, [pTmp4], dstStride
        vst1.8  {d18}, [pTmp4], dstStride
        vst1.8  {d19}, [pTmp4], dstStride

        add     count, count, #1

        /* pTmp4 will simply continue to increment in the vst1.8 above */
        /* no extra fixup is needed. */

        /* move source pointer */
        add     pSrc, pSrc, #8

        subs    pixelsLeft, pixelsLeft, #8
        bgt     __planar_rotate_cw90_width

        /* move the source pointer around the line */
        mov     tmp, #7
        mul     tmp, tmp, width
        add     pSrc, pSrc, tmp

        subs    linesLeft, linesLeft, #8
        bgt     __planar_rotate_cw90_height
        mov     r0, count

        EPILOG r0, r12
.unreq width
.unreq dstHeight
.unreq height
.unreq dstWidth
.unreq pSrc
.unreq pDst
.unreq pixelsLeft
.unreq linesLeft
.unreq tmp
.unreq dstStride
.unreq pTmp1
.unreq pTmp2
.unreq pTmp3
.unreq pTmp4
.unreq count

.end
