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

.global __uyvy_to_rgb565_image_bt601

.include "yuv.inc"

/** This function converts a UYVY image to a RGB565 image with the BT601 constants. */
__uyvy_to_rgb565_image_bt601:
width       .req r0
height      .req r1
pSrc        .req r2
srcStride   .req r3
srcStep     .req r3
pDst        .req r4
dstStride   .req r5
dstStep     .req r5
pixelsLeft  .req r6
linesLeft   .req r7
tmp         .req r8

        PROLOG  r0, r12

        ldr         pDst,       [sp,#(14 * 4)]
        ldr         dstStride,  [sp,#(15 * 4)]
        mov         linesLeft,  height

        add         tmp, width, width                       /* tmp = 2 * width */
        sub         srcStep, srcStride, tmp                 /* srcStep = srcStride - tmp */
        sub         dstStep, dstStride, tmp                 /* dstStep = dstStride - tmp */

        FILL_YCBCR_TO_RGB_BT601 q15,q14,q13,q12,q11,d21,d20
__uyvy_to_rgb565_image_bt601_line:
        mov         pixelsLeft, width
__uyvy_to_rgb565_image_bt601_pixel:

/*
 Remember:
 r = 1.164*(y0 - 16) + 1.596(v0 - 128)
 g = 1.164*(y0 - 16) - 0.813(v0 - 128) - 0.391(u0 - 128)
 b = 1.164*(y0 - 16) + 2.018(u0 - 128)
*/
        preload_L2_1x256bytes pSrc

        vld4.8      {d0,d1,d2,d3},[pSrc]!
                                    /* d0 = {u0,u1,u2,u3,u4,u5,u6,u7] */
                                    /* d1 = {y0,y2,y4,y6,y8,yA,yC,yE] */
                                    /* d2 = [v0,v1,v2,v3,v4,v5,v6,v7] */
                                    /* d3 = [y1,y3,y5,y7,y9,yB,yD,yF] */

        vsubl.u8    q2, d1, d20     /* q2 = Ye - 16 */
        vsubl.u8    q3, d3, d20     /* q3 = Yo - 16 */
        vsubl.u8    q0, d0, d21     /* q0 = Cb - 128 */
        vsubl.u8    q1, d2, d21     /* q1 = Cr - 128 */

        vmul.s16    q4, q2, q15     /* Re = 74*Ye */
        vmla.s16    q4, q1, q12     /* Re += 102*Cr */
        vmul.s16    q5, q3, q15     /* Ro = 74*Yo */
        vmla.s16    q5, q1, q12     /* Ro += 102*Cr */
        vmul.s16    q6, q2, q15     /* Ge = 74*Ye */
        vmls.s16    q6, q1, q13     /* Ge -= 52*Cr */
        vmls.s16    q6, q0, q14     /* Ge -= 25*Cb */
        vmul.s16    q7, q3, q15     /* Go = 74*Yo */
        vmls.s16    q7, q1, q13     /* Go -= 52*Cr */
        vmls.s16    q7, q0, q14     /* Go -= 25*Cb */
        vmul.s16    q8, q2, q15     /* Be = 74*Ye */
        vmla.s16    q8, q0, q11     /* Be += 129*Cb */
        vmul.s16    q9, q3, q15     /* Bo = 74*Yo */
        vmla.s16    q9, q0, q11     /* Bo += 129*Cb */

        /* Create RGB888 */
        vqshrun.s16 d0, q8, #6      /* Be >> 6 */
        vqshrun.s16 d3, q9, #6      /* Bo >> 6 */
        vqshrun.s16 d1, q6, #6      /* Ge >> 6 */
        vqshrun.s16 d4, q7, #6      /* Go >> 6 */
        vqshrun.s16 d2, q4, #6      /* Re >> 6 */
        vqshrun.s16 d5, q5, #6      /* Ro >> 6 */

        vzip.8      d0, d3          /* Be:Bo to Bl:Bh */
        vzip.8      d1, d4          /* Ge:Go to Gl:Gh */
        vzip.8      d2, d5          /* Re:Ro to Rl:Rh */

        RGB888_TO_RGB565 d0, d1, d2, q8, q6, q4, q3
        RGB888_TO_RGB565 d3, d4, d5, q9, q7, q5, q4

        vst1.16     {d6}, [pDst]!
        vst1.16     {d7}, [pDst]!
        vst1.16     {d8}, [pDst]!
        vst1.16     {d9}, [pDst]!

        subs        pixelsLeft, pixelsLeft, #16
        bgt         __uyvy_to_rgb565_image_bt601_pixel

        add         pSrc, pSrc, srcStep
        add         pDst, pDst, dstStep

        subs        linesLeft, linesLeft, #1
        bgt         __uyvy_to_rgb565_image_bt601_line

        EPILOG r0, r12
.unreq width
.unreq height
.unreq pSrc
.unreq srcStride
.unreq srcStep
.unreq pDst
.unreq dstStride
.unreq dstStep
.unreq pixelsLeft
.unreq linesLeft
.unreq tmp

.end
