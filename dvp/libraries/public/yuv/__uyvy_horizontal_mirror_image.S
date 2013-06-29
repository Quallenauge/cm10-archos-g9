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

.global __uyvy_horizontal_mirror_image

.include "yuv.inc"

/** This function mirror's a UYVY image horizontally. */
__uyvy_horizontal_mirror_image:
width       .req r0
height      .req r1
pSrc        .req r2
srcStride   .req r3
pDst        .req r4
dstStride   .req r5
linesLeft   .req r6
pixelsLeft  .req r7
tmp         .req r8
dstStep     .req r9
tmp2        .req r10
srcStep     .req r11

        PROLOG  r0, r12

        ldr     pDst,       [sp, #(14 * 4)]
        ldr     dstStride,  [sp, #(15 * 4)]

        /* inititalize the lines count */
        mov     linesLeft, height

        /* initialize the srcStep */
        mov     tmp, width
        mov     tmp, tmp, lsl #1
        sub     srcStep, srcStride, tmp

__uyvy_horizontal_mirror_image_line:

        preload_L2_1x256bytes pSrc

        /* reset the pixels */
        mov     pixelsLeft, width

        /* move pDst to the end of the line (last UYVY) */
        sub     tmp, width, #2
        mov     tmp, tmp, lsl #1
        add     pDst, pDst, tmp  /* pDst += ((width-2)*2) */

        /* find out if there's an non-multiple number of units */
        mov     tmp, #0x1F
        and     tmp2, width, tmp /* tmp2 = width%32 */

        /* if there arent't then go to pre_blocks */
        mov     tmp, #0
        cmp     tmp2, tmp
        beq     __uyvy_horizontal_mirror_image_pre_blocks

__uyvy_horizontal_mirror_image_spare:

        /* reverse a single DWORD and write it out */
        ldr     tmp, [pSrc]      /* tmp = *pSrc */
        add     pSrc, pSrc, #4
                                 /* tmp = U0 Y0 V0 Y1 */
        rev16   tmp, tmp         /* tmp = Y0 U0 Y1 V0 */
        mov     tmp, tmp, ror #8 /* tmp = U0 Y1 V0 Y0 */
        str     tmp, [pDst]      /* *pDst = tmp */

        sub     pDst, pDst, #4
        sub     pixelsLeft, pixelsLeft, #2
        subs    tmp2, tmp2, #2
        bgt     __uyvy_horizontal_mirror_image_spare

__uyvy_horizontal_mirror_image_pre_blocks:

        /* move pDst back 28 more bytes to account for the block size */
        sub     pDst, pDst, #28
        mov     tmp, #-32

__uyvy_horizontal_mirror_image_blocks:

        mov     tmp2, #0xFF
        add     tmp2, tmp2, #1
        add     tmp2, pSrc, tmp2
        preload_L2_1x256bytes tmp2

        vld4.8   {d0,d1,d2,d3}, [pSrc]!
        vswp.8    d1,d3
        REVERSE   d0, d0
        REVERSE   d1, d1
        REVERSE   d2, d2
        REVERSE   d3, d3
        vst4.8   {d0,d1,d2,d3}, [pDst], tmp

        subs    pixelsLeft, pixelsLeft, #16
        bgt     __uyvy_horizontal_mirror_image_blocks

        /* add a stride to the pDst */
        mov     tmp, #0x20              /* tmp = 32 */
        add     tmp, dstStride, tmp     /* tmp += dstStride */
        add     pDst, pDst, tmp         /* pDst += tmp */
        add     pSrc, pSrc, srcStep     /* pSrc += srcStep */

        subs    linesLeft, linesLeft, #1
        bgt     __uyvy_horizontal_mirror_image_line

        EPILOG  r0, r12
.unreq width
.unreq height
.unreq pSrc
.unreq srcStride
.unreq pDst
.unreq dstStride
.unreq linesLeft
.unreq pixelsLeft
.unreq tmp
.unreq dstStep
.unreq tmp2
.unreq srcStep

.end
