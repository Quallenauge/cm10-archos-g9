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

.global __uyvy_qtr_scale_image

.include "yuv.inc"

__uyvy_qtr_scale_image:
width       .req r0
height      .req r1
pSrc        .req r2
srcStride   .req r3
pDst        .req r4
dstStride   .req r5
dstStep     .req r5
preLoadIdx  .req r6
linesLeft   .req r7
pixelsLeft  .req r8
tmp         .req r9
tmp2        .req r10
tmp3        .req r11
srcStep     .req r12
        PROLOG r0, r12

        /* FAULT
        mov     tmp, #0
        ldr     tmp, [tmp]
        */
        ldr     pDst,       [sp, #(14 * 4)]
        ldr     dstStride,  [sp, #(15 * 4)]
        mov     linesLeft, height
        mov     tmp, width, lsl #1              /* tmp = 2 * width */
        sub     srcStep, srcStride, tmp         /* srcStep = srcStride - (2 * width) */
        mov     tmp, tmp, lsr #2                /* tmp = (2 * width) / 4 */
        sub     dstStep, dstStride, tmp         /* dstStep = dstStride - (2 * width / 4) */

__uyvy_qtr_scale_image_loop_line:

        /* Update the number of pixels left for this line. */
        mov     pixelsLeft, width

        /* Force the preload on the next block */
        mov     preLoadIdx, #(L2_LINE_SIZE/16)

        /* Preload first block for 4 lines worth */
        mov     tmp, pSrc
        pld     [tmp]
        add     tmp, tmp, srcStride
        pld     [tmp]
        add     tmp, tmp, srcStride
        pld     [tmp]
        add     tmp, tmp, srcStride
        pld     [tmp]
        ldr     tmp2, [tmp] /* wait for the preload to finish */

__uyvy_qtr_scale_image_loop_pixels:

        /* Preload the next block if the index is correct */
        subs    preLoadIdx, preLoadIdx, #(L2_LINE_SIZE/16)
        bne     __uyvy_qtr_scale_image_loop_algo
        add     tmp, pSrc, #L2_LINE_SIZE
        pld     [tmp]
        add     tmp, tmp, srcStride
        pld     [tmp]
        add     tmp, tmp, srcStride
        pld     [tmp]
        add     tmp, tmp, srcStride
        pld     [tmp]
__uyvy_qtr_scale_image_loop_algo:
        add     preLoadIdx, preLoadIdx, #1

        /* Loads U/V into D(even), Y0,Y1 into D(odd) */
        mov         tmp, pSrc
        vld2.8      {d0, d1}, [tmp]
        add         tmp, tmp,  srcStride
        vld2.8      {d2, d3}, [tmp]
        add         tmp, tmp,  srcStride
        vld2.8      {d4, d5}, [tmp]
        add         tmp, tmp,  srcStride
        vld2.8      {d6, d7}, [tmp]
        add         pSrc, pSrc, #16

        /* Horizontally Average the Y values */
        vpaddl.u8   d1, d1
        vpaddl.u8   d3, d3
        vpaddl.u8   d5, d5
        vpaddl.u8   d7, d7
        vpaddl.u16  d1, d1
        vpaddl.u16  d3, d3
        vpaddl.u16  d5, d5
        vpaddl.u16  d7, d7

        /* Vertically Average the Y's */
        vadd.u32    d1, d3, d1
        vadd.u32    d1, d5, d1
        vadd.u32    d1, d7, d1
        vshr.u32    d1, d1, #4

        /* Y0 and Y1 are in D1 as U32's but should be u8 values */

        /* Rearrange the U/V to U's and V's */
        vuzp.u8     d0, d2
        vuzp.u8     d4, d6

        /* Average all U's and all V's into one U0, V0 value */
        vpaddl.u8   d0, d0
        vpaddl.u8   d2, d2
        vpaddl.u8   d4, d4
        vpaddl.u8   d6, d6
        vadd.u16    d0, d4, d0
        vadd.u16    d2, d6, d2
        vpaddl.u16  d0, d0
        vpaddl.u16  d2, d2
        vpaddl.u32  d0, d0
        vpaddl.u32  d2, d2
        vshr.u64    d0, d0, #4
        vshr.u64    d2, d2, #4

        /* U0 is in D0 as a u64 but should be u8 value */
        /* V0 is in D2 as a u64 but should be u8 value */

        /* Transfer the UYVY Data to an ARM register */
        vmov    tmp, tmp2, d1       /* Move Y0 to tmp, Y1 to tmp */
        usat    tmp, #8, tmp
        usat    tmp2, #8, tmp2
        mov     tmp, tmp, lsl #8
        mov     tmp2, tmp2, lsl #24
        orr     tmp, tmp, tmp2
        vmov    tmp2, tmp3, d0      /* Move U0 to tmp2 */
        usat    tmp2, #8, tmp2
        orr     tmp, tmp, tmp2
        vmov    tmp2, tmp3, d2      /* Move V0 to tmp2 */
        usat    tmp2, #8, tmp2
        mov     tmp2, tmp2, lsl #16
        orr     tmp, tmp, tmp2

        str     tmp, [pDst]     /* *pDst = tmp */
        add     pDst, pDst, #4  /* pDst += 4 */

        /* END OF LOOP */

        subs    pixelsLeft, pixelsLeft, #8
        bgt     __uyvy_qtr_scale_image_loop_pixels

        add     pSrc, pSrc, srcStep
        add     pSrc, pSrc, srcStride
        add     pSrc, pSrc, srcStride
        add     pSrc, pSrc, srcStride /* skip 3 extra lines */
        add     pDst, pDst, dstStep

        subs    linesLeft, linesLeft, #4
        bgt     __uyvy_qtr_scale_image_loop_line

        EPILOG r0, r12
.unreq width
.unreq height
.unreq pSrc
.unreq srcStride
.unreq pDst
.unreq dstStride
.unreq dstStep
.unreq linesLeft
.unreq pixelsLeft
.unreq tmp
.unreq tmp2
.unreq tmp3
.unreq srcStep

.end
