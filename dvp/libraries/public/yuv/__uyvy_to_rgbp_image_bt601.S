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

.global __uyvy_to_rgbp_image_bt601

.include "yuv.inc"

__uyvy_to_rgbp_image_bt601:
width       .req r0
height      .req r1
pSrc        .req r2
srcStride   .req r3
srcStep     .req r3
pR          .req r4
pG          .req r5
pB          .req r6
dstStride   .req r7
dstStep     .req r7
linesLeft   .req r8
pixelsLeft  .req r9
tmp         .req r10
        PROLOG  r0, r12
        ldr     pR, [sp, #(14 * 4)]
        ldr     pG, [sp, #(15 * 4)]
        ldr     pB, [sp, #(16 * 4)]
        ldr     dstStride, [sp, #(17 * 4)]
        add     tmp, width, width                       /* tmp = 2 * width */
        sub     srcStep, srcStride, tmp                 /* srcStep = srcStride - tmp */
        sub     dstStep, dstStride, width
        mov     linesLeft, height

        FILL_YCBCR_TO_RGB_BT601 q15,q14,q13,q12,q11,d21,d20

__uyvy_to_rgbp_image_bt601_line:
        mov         pixelsLeft, width
__uyvy_to_rgbp_image_bt601_pixel:

        preload_L2_1x256bytes pSrc

        vld4.8      {d0,d1,d2,d3},[pSrc]!
                                    /* d0 = {u0,u1,u2,u3,u4,u5,u6,u7] = Cb */
                                    /* d1 = {y0,y2,y4,y6,y8,yA,yC,yE] = Ye */
                                    /* d2 = [v0,v1,v2,v3,v4,v5,v6,v7] = Cr */
                                    /* d3 = [y1,y3,y5,y7,y9,yB,yD,yF] = Yo */

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

        vqshrun.s16 d0, q8, #6      /* Be >> 6 */
        vqshrun.s16 d3, q9, #6      /* Bo >> 6 */
        vqshrun.s16 d1, q6, #6      /* Ge >> 6 */
        vqshrun.s16 d4, q7, #6      /* Go >> 6 */
        vqshrun.s16 d2, q4, #6      /* Re >> 6 */
        vqshrun.s16 d5, q5, #6      /* Ro >> 6 */

        vzip.8      d0, d3          /* Be:Bo to Bl:Bh */
        vzip.8      d1, d4          /* Ge:Go to Gl:Gh */
        vzip.8      d2, d5          /* Re:Ro to Rl:Rh */

        vst1.8  {d0}, [pB]!
        vst1.8  {d3}, [pB]!
        vst1.8  {d1}, [pG]!
        vst1.8  {d4}, [pG]!
        vst1.8  {d2}, [pR]!
        vst1.8  {d5}, [pR]!

        subs        pixelsLeft, pixelsLeft, #16
        bgt         __uyvy_to_rgbp_image_bt601_pixel

        add         pSrc, pSrc, srcStep
        add         pR, pR, dstStep
        add         pG, pG, dstStep
        add         pB, pB, dstStep

        subs        linesLeft, linesLeft, #1
        bgt         __uyvy_to_rgbp_image_bt601_line

        EPILOG  r0, r12
.unreq tmp
.unreq pixelsLeft
.unreq linesLeft
.unreq dstStep
.unreq dstStride
.unreq pB
.unreq pG
.unreq pR
.unreq srcStep
.unreq srcStride
.unreq pSrc
.unreq height
.unreq width

.end
