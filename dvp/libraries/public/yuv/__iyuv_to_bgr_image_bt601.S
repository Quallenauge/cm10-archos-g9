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

.global __iyuv_to_bgr_image_bt601

.include "yuv.inc"

__iyuv_to_bgr_image_bt601:
width       .req r0
height      .req r1
pY          .req r2
pU          .req r3
pV          .req r4
pDst        .req r5
dstStride   .req r6
dstStep     .req r7
pixelsLeft  .req r8
linesLeft   .req r9
uvIndex     .req r10
tmp         .req r11
tmp2        .req r12
        PROLOG  r0, r12
        ldr         pV,         [sp,#(14 * 4)]
        ldr         pDst,       [sp,#(15 * 4)]
        ldr         dstStride,  [sp,#(16 * 4)]
        mov         linesLeft,  height
        mov         uvIndex,    #0
        mov         tmp2,       #0
        add         tmp, width, width                       /* tmp = 2 * width */
        sub         dstStep, dstStride, tmp                 /* dstStep = dstStride - tmp */

        FILL_YCBCR_TO_RGB_BT601 q15,q14,q13,q12,q11,d21,d20
__iyuv_to_bgr_image_bt601_line:
        mov         pixelsLeft, width
__iyuv_to_bgr_image_bt601_pixel:

        pld         [pY]
        pld         [pY, #L2_LINE_SIZE]
        pld         [pU]
        pld         [pV]

        vld1.u8      {d2}, [pY]!         /* d2 = [y0,y1,y2,y3,y4,y5,y6,y7] */
        ldr         tmp, [pU, uvIndex]
        vmov        d1, tmp, tmp2       /* d1 = [u0,u2,u4,u6,xx,xx,xx,xx] */
        DUPLICATE_EACH_BYTE d1,d3       /* d1 = [u0,u0,u2,u2,v4,u4,u6,u6] */
        ldr         tmp, [pV, uvIndex]
        vmov        d0, tmp, tmp2       /* d0 = [v0,v2,v4,v6,xx,xx,xx,xx] */
        DUPLICATE_EACH_BYTE d0,d3       /* d0 = [v0,v0,v2,v2,v4,v4,v6,v6] */

        vsubl.u8    q2, d2, d20     /* q2 = Y - 16 */
        vsubl.u8    q1, d1, d21     /* q1 = Cb - 128 */
        vsubl.u8    q0, d0, d21     /* q0 = Cr - 128 */

        vmul.s16    q4, q2, q15     /* R = 74*Ye */
        vmla.s16    q4, q0, q12     /* R += 102*Cr */
        vmul.s16    q5, q2, q15     /* G = 74*Ye */
        vmls.s16    q5, q0, q13     /* G -= 52*Cr */
        vmls.s16    q5, q1, q14     /* G -= 25*Cb */
        vmul.s16    q6, q2, q15     /* B = 74*Ye */
        vmla.s16    q6, q1, q11     /* B += 129*Cb */

        /* Create RGB888 */
        vqshrun.s16 d0, q6, #6      /* B >> 6 */
        vqshrun.s16 d1, q5, #6      /* G >> 6 */
        vqshrun.s16 d2, q4, #6      /* R >> 6 */

        vst3.8     {d0,d1,d2},[pDst]!

        add         uvIndex, uvIndex, #4

        subs        pixelsLeft, pixelsLeft, #8
        bgt         __iyuv_to_bgr_image_bt601_pixel

        sub         tmp, height, linesLeft
        mov         tmp, tmp, asr #1
        mul         tmp, tmp, width
        mov         uvIndex, tmp, asr #1    /* reset the uvIndex to the right spot (we'll double process each U/V line) */
        add         pDst, pDst, dstStep

        subs        linesLeft, linesLeft, #1
        bgt         __iyuv_to_bgr_image_bt601_line

        EPILOG  r0, r12
.unreq width
.unreq height
.unreq pY
.unreq pU
.unreq pV
.unreq pDst
.unreq dstStride
.unreq dstStep
.unreq pixelsLeft
.unreq linesLeft
.unreq uvIndex
.unreq tmp
.unreq tmp2

.end
