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

.global __yuv444_to_rgbp_image_bt601

.include "yuv.inc"

__yuv444_to_rgbp_image_bt601:
width       .req r0
height      .req r1
pY          .req r2
pU          .req r3
pV          .req r4
srcStride   .req r5
srcStep     .req r5
pR          .req r6
pG          .req r7
pB          .req r8
dstStride   .req r9
dstStep     .req r9
pixelsLeft  .req r10
linesLeft   .req r11
tmp         .req r12

        PROLOG  r0, r12
        ldr     r4, [sp, #(14*4)] /* pV */
        ldr     r5, [sp, #(15*4)] /* srcStride */
        ldr     r6, [sp, #(16*4)] /* pR */
        ldr     r7, [sp, #(17*4)] /* pG */
        ldr     r8, [sp, #(18*4)] /* pB */
        ldr     r9, [sp, #(19*4)] /* dstStride */
        mov     linesLeft, height
        sub     srcStep, srcStride, width
        sub     dstStep, dstStride, width
        mov     tmp, #L2_LINE_SIZE
        FILL_YCBCR_TO_RGB_BT601_Q13 q15,q14,q13,q12,q11,q10

        /* load the offset and clamp values */
        vmov.u8    d18, #16
        vmov.u8    d19, #128
        vmov.u8    d17, #235
        vmov.u8    d16, #240

__yuv444_to_rgbp_image_bt601_line:
        mov     pixelsLeft, width
__yuv444_to_rgbp_image_bt601_pixels:
        /* preload the next set */
        pld     [pY, tmp]
        pld     [pU, tmp]
        pld     [pV, tmp]

       /* load the pixels */
        vld1.8  {d2}, [pY]!         /* d2 = [y0,y1,y2,y3,y4,y5,y6,y7] */
        vld1.8  {d1}, [pU]!         /* d1 = [u0,u1,u2,u3,u4,u5,u6,u7] */
        vld1.8  {d0}, [pV]!         /* d0 = [v0,v1,v2,v3,v4,v5,v6,v7] */

        /* clamp ranges: 16 <= Y <= 235 */
        vmax.u8     d2, d2, d18
        vmin.u8     d2, d2, d17

        /* clamp ranges: 16 <= Cb,Cr <= 240 */
        vmax.u8     d1, d1, d18
        vmax.u8     d0, d0, d18
        vmin.u8     d1, d1, d16
        vmin.u8     d0, d0, d16

        /* range change and length to 16 bits */
        vsubl.u8    q2, d2, d18     /* q2 = Y - 16 */
        vsubl.u8    q1, d1, d19     /* q1 = Cb - 128 */
        vsubl.u8    q0, d0, d19     /* q0 = Cr - 128 */

        /* we have to futher expand the s16 to s32 during the coeff multiply */
        vmull.s16   q3, d4, d30     /* yn = yk*ye */
        vmull.s16   q4, d5, d31

        vqshrn.u32 d4, q3, #13      /* yn >>= 13 */
        vqshrn.u32 d5, q4, #13      /* q2 = yn */

        vmull.s16   q3, d0, d24     /* rn = rk*Cr */
        vmull.s16   q4, d1, d25

        vqshrn.s32 d6, q3, #13      /* rn >>= 13 */
        vqshrn.s32 d7, q4, #13      /* q3 = rn */

        vmull.s16   q4, d2, d22     /* bn = bk*Cb */
        vmull.s16   q5, d3, d23

        vqshrn.s32 d8, q4, #13      /* bn >>= 13 */
        vqshrn.s32 d9, q5, #13      /* q4 = bn */

        vmull.s16   q5, d0, d26     /* gn1 = gk1*Cr */
        vmull.s16   q6, d1, d27

        vqshrn.s32 d10, q5, #13     /* gn1 >>= 13 */
        vqshrn.s32 d11, q6, #13     /* q5 = qn1 */

        vmull.s16   q6, d2, d28     /* gn2 = gk2*Cb */
        vmull.s16   q7, d3, d29

        vqshrn.s32 d12, q6, #13     /* gn2 >>= 13 */
        vqshrn.s32 d13, q7, #13     /* q6 = gn2 */

        vadd.s16    q3, q2, q3      /* R = yn + rn */
        vadd.s16    q4, q2, q4      /* B = yn + bn */
        vsub.s16    q5, q2, q5      /* G = yn - gn1 */
        vsub.s16    q5, q5, q6      /* G -= gn2 */

        vmov.i16    q6, #0
        vmov.i16    q7, #255

        /* clamp: 255 <= R,G,B */
        vmin.s16    q3, q3, q7
        vmin.s16    q4, q4, q7
        vmin.s16    q5, q5, q7

        /* clamp: 0 >= R,G,B */
        vmax.s16    q3, q3, q6
        vmax.s16    q4, q4, q6
        vmax.s16    q5, q5, q6

        /* narrow from u16 to u8 */
        vqshrun.s16 d2, q3, #0
        vqshrun.s16 d1, q5, #0
        vqshrun.s16 d0, q4, #0

        vst1.8  {d2}, [pR]!
        vst1.8  {d1}, [pG]!
        vst1.8  {d0}, [pB]!

        subs    pixelsLeft, pixelsLeft, #8
        bgt     __yuv444_to_rgbp_image_bt601_pixels
        add     pY, pY, srcStep
        add     pU, pU, srcStep
        add     pV, pV, srcStep
        add     pR, pR, dstStep
        add     pG, pG, dstStep
        add     pB, pB, dstStep
        subs    linesLeft, linesLeft, #1
        bgt     __yuv444_to_rgbp_image_bt601_line
        EPILOG  r0, r12

.unreq width
.unreq height
.unreq pY
.unreq pU
.unreq pV
.unreq srcStride
.unreq srcStep
.unreq pR
.unreq pG
.unreq pB
.unreq dstStride
.unreq dstStep
.unreq pixelsLeft
.unreq linesLeft
.unreq tmp

.end
