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

.global __bgr_to_iyuv_image_bt601

.include "yuv.inc"

/** This subroutine converts BGR (byte ordered) data to UYVY Data */
__bgr_to_iyuv_image_bt601:
width       .req r0
height      .req r1
pSrc        .req r2
srcStride   .req r3
pY          .req r4
pU          .req r5
pV          .req r6
pixelsLeft  .req r7
linesLeft   .req r8
tmp         .req r9
tmp2        .req r10
srcStep     .req r11
        PROLOG r0, r12

        ldr         pY, [sp,#(14 * 4)]
        ldr         pU, [sp,#(15 * 4)]
        ldr         pV, [sp,#(16 * 4)]

        mov         linesLeft,  height

        add         tmp, width, width, lsl #1               /* tmp = 3 * width */
        sub         srcStep, srcStride, tmp                 /* srcStep = srcStride - tmp */

        FILL_RGB_TO_YCBCR_BT601 q5, q14, q13, q12, q11, q10, q9, q8

__bgr_to_iyuv_image_bt601_line:
        mov         pixelsLeft, width
__bgr_to_iyuv_image_bt601_pixel:
        pld         [pSrc]
        pld         [pSrc, #L2_LINE_SIZE]
        pld         [pSrc, srcStride]
        add         tmp, srcStride, #L2_LINE_SIZE
        pld         [pSrc, tmp]

        vld3.8      {d1, d2, d3},[pSrc]
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
        vrshrn.i16  d6, q3, #6      /* d6 = ([y0,y1,y2,y3,y4,y5,y6,y7] >> 6) */
        vrshrn.i16  d8, q4, #6      /* d8 = ([u0,u1,u2,u3,u4,u5,u6,u7] >> 6) */
        vrshrn.i16  d9, q5, #6      /* d9 = ([v0,v1,v2,v3,v4,v5,v6,v7] >> 6) */

        /* Average the U's and V's */
        vtrn.8      d8, d9
        vhadd.u8    d8, d8, d9      /* d8 = [u0,v0,u2,v2,u4,v4,u6,v6] (average) */

        add         tmp, pSrc, srcStride
        vld3.8     {d1, d2, d3},[tmp]
                                    /* d1 = [b0,b1,b2,b3,b4,b5,b6,b7] */
                                    /* d2 = [g0,g1,g2,g3,g4,g5,g6,g7] */
                                    /* d3 = [r0,r1,r2,r3,r4,r5,r6,r7] */

        vmovl.u8    q2, d3          /* q2 = [r0,r1,r2,r3,r4,r5,r6,r7] */
        vmovl.u8    q1, d2          /* q1 = [g0,g1,g2,g3,g4,g5,g6,g7] */
        vmovl.u8    q0, d1          /* q0 = [b0,b1,b2,b3,b4,b5,b6,b7] */

        vmov.i16    q5, #1024       /* Y = 16 */
        vmov.i16    q6, #8192       /* Cb = 128 */
        vmov.i16    q7, #8192       /* Cr = 128 */

        vmla.s16    q5, q2, q15     /* Y += 0.257*R */
        vmla.s16    q5, q1, q14     /* Y += 0.504*G */
        vmla.s16    q5, q0, q13     /* Y += 0.098*B */

        vmls.s16    q6, q2, q12     /* Cb += -0.148*R */
        vmls.s16    q6, q1, q11     /* Cb += -0.291*G */
        vmla.s16    q6, q0, q10     /* Cb += 0.439*B */

        vmla.s16    q7, q2, q10     /* Cr += 0.439*R */
        vmls.s16    q7, q1, q9      /* Cr += -0.368*G */
        vmls.s16    q7, q0, q8      /* Cr += -0.071*B */

        /* compress the YUV >> 6 */
        vrshrn.i16  d7, q5, #6      /* d7  = ([y8,y9,yA,yB,yC,yD,yE,yF] >> 6) */
        vrshrn.i16  d10,q6, #6      /* d10 = ([u8,u9,uA,uB,uC,uD,uE,uF] >> 6) */
        vrshrn.i16  d11,q7, #6      /* d11 = ([v8,v9,vA,vB,vC,vD,vE,vF] >> 6) */

        /* Average the U's and V's */
        vtrn.8      d10,d11
        vhadd.u8    d10,d10,d11     /* d10 = [u8,v8,uA,vA,uC,vC,uE,vE] (average) */

        /* Store Y's */
        vst1.8      {d6}, [pY]
        add         tmp, pY, width
        vst1.8      {d7}, [tmp]

        /* Now Average the averages */
        vhadd.u8    d8, d8, d10     /* d8 = [u0,v0,u2,v2,u4,v4,u6,v6] (average) */

        /* Arrange the U's and V's for writing */
        vmov.u8     d9, #0
        vmov.u8     d10,#0
        vmov.u8     d11,#0
        vtrn.8      d8, d10         /* d8 = [u0,xx,u2,xx,u4,xx,u6,xx] */
                                    /* d10= [v0,xx,v2,xx,v4,xx,v6,xx] */
        vmovn.i16   d8, q4          /* d8 = [u0,u2,u4,u6,xx,xx,xx,xx] */
        vmovn.i16   d10,q5          /* d10= [v0,v2,v4,v6,xx,xx,xx,xx] */
        /* Store U's */
        vmov        tmp, tmp2, d8
        str         tmp, [pU]

        /* Store V's */
        vmov        tmp, tmp2, d10
        str         tmp, [pV]

        /* Pointer Arithmetic for pixel group */
        add         pSrc, pSrc, #24                     /* pSrc += (8 * sizeof(BGR)) */
        add         pY, pY, #8
        add         pU, pU, #4
        add         pV, pV, #4

        /* LOOP */
        subs        pixelsLeft, pixelsLeft, #8          /* process 8 pixels at a time. */
        bgt         __bgr_to_iyuv_image_bt601_pixel

        /* Pointer Arithmetic for the lines */
        add         pSrc, pSrc, srcStep                 /* pSrc += srcStep */
        add         pSrc, pSrc, srcStride               /* pSrc += srcStride (skips a line) */
        add         pY, pY, width                       /* pY += width (skips a line) */
        /* pU, and pV are in the correct places */

        subs        linesLeft, linesLeft, #2            /* process 2 lines at a time. */
        bgt         __bgr_to_iyuv_image_bt601_line

        EPILOG r0, r12
.unreq width
.unreq height
.unreq pSrc
.unreq srcStride
.unreq pY
.unreq pU
.unreq pV
.unreq pixelsLeft
.unreq linesLeft
.unreq tmp
.unreq tmp2
.unreq srcStep

.end
