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

.global __bgr_to_uyvy_image_bt601

.include "yuv.inc"

/** This subroutine converts BGR (byte ordered) data to UYVY Data */
__bgr_to_uyvy_image_bt601:
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

        PROLOG r0, r12

        ldr         pDst,       [sp,#(14 * 4)]
        ldr         dstStride,  [sp,#(15 * 4)]
        mov         linesLeft,  height

        add         tmp, width, width, lsl #1               /* tmp = 3 * width */
        sub         srcStep, srcStride, tmp                 /* srcStep = srcStride - tmp */
        add         tmp, width, width                       /* tmp = 2 * width */
        sub         dstStep, dstStride, tmp                 /* dstStep = dstStride - tmp */

        FILL_RGB_TO_YCBCR_BT601 q15, q14, q13, q12, q11, q10, q9, q8

__bgr_to_uyvy_image_bt601_line:
        mov         pixelsLeft, width
__bgr_to_uyvy_image_bt601_pixel:

        pld         [pSrc]
        pld         [pSrc, #L2_LINE_SIZE]
        pld         [pSrc, srcStride]
        add         tmp, srcStride, #L2_LINE_SIZE
        pld         [pSrc, tmp]

        vld3.8      {d1, d2, d3},[pSrc]!
                                    /* d1 = [b0,b1,b2,b3,b4,b5,b6,b7] */
                                    /* d2 = [g0,g1,g2,g3,g4,g5,g6,g7] */
                                    /* d3 = [r0,r1,r2,r3,r4,r5,r6,r7] */

        vmovl.u8    q2, d3          /* q2 = [r0,r1,r2,r3,r4,r5,r6,r7] */
        vmovl.u8    q1, d2          /* q1 = [g0,g1,g2,g3,g4,g5,g6,g7] */
        vmovl.u8    q0, d1          /* q0 = [b0,b1,b2,b3,b4,b5,b6,b7] */

        vmov.i16    q3, #1024       /* Y = 16 */
        vmov.i16    q4, #8192       /* Cb = 128 */
        vmov.i16    q5, #8192       /* Cr = 128 */

        vmla.s16    q3, q2, q15     /* Y += 0.257*R */
        vmla.s16    q3, q1, q14     /* Y += 0.504*G */
        vmla.s16    q3, q0, q13     /* Y += 0.098*B */

        vmls.s16    q4, q2, q12     /* Cb += -0.148*R */
        vmls.s16    q4, q1, q11     /* Cb += -0.291*G */
        vmla.s16    q4, q0, q10     /* Cb += 0.439*B */

        vmla.s16    q5, q2, q10     /* Cr += 0.439*R */
        vmls.s16    q5, q1, q9      /* Cr += -0.368*G */
        vmls.s16    q5, q0, q8      /* Cr += -0.071*B */

        /* compress the YUV >> 6 */
        vrshrn.i16  d1, q3, #6      /* d1 = [y0,y1,y2,y3,y4,y5,y6,y7] = q3 >> 6 */
        vrshrn.i16  d7, q4, #6      /* d7 = [u0,u1,u2,u3,u4,u5,u6,u7] = q4 >> 6 */
        vrshrn.i16  d8, q5, #6      /* d8 = [v0,v1,v2,v3,v4,v5,v6,v7] = q5 >> 6 */

        /* Interleave */
        vtrn.8      d7, d8          /* d7 = [u0,v0,u2,v2,u4,v4,u6,v6] */
                                    /* d8 = [u1,v1,u3,v3,u5,v5,u7,v7] */
        vhadd.u8    d0, d7, d8      /* d7 = [u0,v0,u2,v2,u4,v4,u6,v6] (average) */
                                    /* d6 = [y0,y1,y2,y3,y4,y5,y6,y7] */
        vtrn.8      d0, d1          /* d0 = [u0,y0,u2,y2,u4,y4,u6,y6] */
                                    /* d1 = [v0,y1,v2,y3,v4,y5,v4,y7] */
        vtrn.16     d0, d1          /* d0 = [u0,y0,v0,y1,u4,y4,v4,y5] */
                                    /* d1 = [u2,y2,v2,y3,u6,y6,v6,y7] */
        /* Writeback */
        vst2.32    {d0, d1}, [pDst]!

        /* LOOP */

        subs        pixelsLeft, pixelsLeft, #8
        bgt         __bgr_to_uyvy_image_bt601_pixel

        add         pSrc, pSrc, srcStep
        add         pDst, pDst, dstStep

        subs        linesLeft, linesLeft, #1
        bgt         __bgr_to_uyvy_image_bt601_line

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
