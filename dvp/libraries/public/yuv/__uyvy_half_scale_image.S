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

.global __uyvy_half_scale_image

.include "yuv.inc"

/**
 * A subroutine to scale down a UYVY Image by half (in a 32x2 pixel block)
 */
__uyvy_half_scale_image:
width       .req r0
height      .req r1
pSrc        .req r2
srcStride   .req r3
pDst        .req r4
dstStride   .req r5
dstStep     .req r5
pSrcNext    .req r6
linesLeft   .req r7
pixelsLeft  .req r8
subwidth    .req r9
tmp         .req r10
tmp2        .req r11
srcStep     .req r12
        PROLOG r0, r12

        ldr     pDst,       [sp, #(14 * 4)]
        ldr     dstStride,  [sp, #(15 * 4)]
        mov     linesLeft, height
        mov     tmp, width, lsl #1              /* tmp = 2 * width */
        sub     srcStep, srcStride, tmp         /* srcStep = srcStride - (2 * width) */
        sub     dstStep, dstStride, width       /* dstStep = dstStride - (2 * width/2) */

__uyvy_half_scale_image_loop_line:
        add     pSrcNext, pSrc, srcStride
        mov     tmp, #0x1F
        and     subwidth, width, tmp            /* subwidth = width % 32 (tells us if there's a problem with doing this by 32) */

        /* Update the number of pixels left for this line. */
        sub     pixelsLeft, width, subwidth     /* pixelsLeft = width - subwidth */

        /* Does this width even divide by 32? if not we'll operate on a 4 pixel group until we clear the subwidth */
        mov     tmp, #0
        cmp     subwidth, tmp
        beq     __uyvy_half_scale_image_loop_32pixels

__uyvy_half_scale_image_loop_4pixels:

        pld         [pSrc]
        pld         [pSrc, #L2_LINE_SIZE]
        pld         [pSrcNext]
        pld         [pSrcNext, #L2_LINE_SIZE]

        /* can't use ARMv6 code as we don't have enough registers and we don't want to have to push/pop here */
        vld1.64     {d0},[pSrc]!        /* Load 2 macro pixels on row n */
        vld1.64     {d1},[pSrcNext]!    /* Load 2 macro pixels on row n+1 */

        vhadd.u8    d2, d0, d1  /* Average of Rows U/V */
        vmov.u8     d3, #0      /* Clear out D1 */
        vtrn.32     d2, d3
        vhadd.u8    d4, d2, d3  /* Average of Columns U/V */
        vtrn.16     d2, d3
        vhadd.u8    d5, d2, d3  /* Average of Columns Y's */
        vshr.u16    d5, d5, #8  /* Move the Y's down */
        vtrn.8      d4, d5
        vmov        tmp, tmp2, d4
        str         tmp, [pDst]!

        subs    subwidth, subwidth, #4
        bgt     __uyvy_half_scale_image_loop_4pixels

__uyvy_half_scale_image_loop_32pixels:

        pld         [pSrc]
        pld         [pSrc, #L2_LINE_SIZE]
        pld         [pSrcNext]
        pld         [pSrcNext, #L2_LINE_SIZE]

        vld4.8  {d0, d1, d2, d3},[pSrc]!
        vld4.8  {d4, d5, d6, d7},[pSrc]!
        vld4.8  {d8, d9, d10,d11},[pSrcNext]!
        vld4.8  {d12,d13,d14,d15},[pSrcNext]!

        /* See macro for load/store conventions. */
        uyvy_32x2_average d0,d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13,d14,d15

        vst2.16     {d0,d1}, [pDst]!
        vst2.16     {d4,d5}, [pDst]!

        /* END OF LOOP */

        subs    pixelsLeft, pixelsLeft, #32
        bgt     __uyvy_half_scale_image_loop_32pixels

        add     pSrc, pSrc, srcStep
        add     pSrc, pSrc, srcStride
        add     pDst, pDst, dstStep

        subs    linesLeft, linesLeft, #2
        bgt     __uyvy_half_scale_image_loop_line

        EPILOG r0, r12
.unreq width
.unreq height
.unreq pSrc
.unreq srcStride
.unreq pDst
.unreq dstStride
.unreq dstStep
.unreq pSrcNext
.unreq linesLeft
.unreq pixelsLeft
.unreq subwidth
.unreq tmp
.unreq tmp2
.unreq srcStep

.end
