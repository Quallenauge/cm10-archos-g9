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

.global __uyvy_to_yuv420_image

.include "yuv.inc"

__uyvy_to_yuv420_image:
width       .req r0
height      .req r1
pSrc        .req r2
srcStride   .req r3
pY          .req r4
pU          .req r5
pV          .req r6
linesLeft   .req r7
pixelsLeft  .req r8
y_stride    .req r9
srcStep     .req r10
tmp         .req r11
uv_stride   .req r12
        PROLOG r0, r12
        ldr pY, [sp, #(14 * 4)]
        ldr pU, [sp, #(15 * 4)]
        ldr pV, [sp, #(16 * 4)]
        ldr y_stride, [sp, #(17 * 4)]
        ldr uv_stride, [sp, #(18 * 4)]
        add     tmp, width, width
        sub     srcStep, srcStride, tmp
        mov     linesLeft, height
__uyvy_to_iyuv_image_loop_line:
        mov     pixelsLeft, width
__uyvy_to_iyuv_image_loop_pixel:

        preload_L2_1x256bytes pSrc

        /* read in 8 macro pixels */
        vld4.8  {d0,d1,d2,d3},[pSrc]
                                        /* d0 = [u0,u1,u2,u3,u4,u5,u6,u7] */
                                        /* d1 = [y0,y2,y4,y6,y8,yA,yC,yE] */
                                        /* d2 = [v0,v1,v2,v3,v4,v5,v6,v7] */
                                        /* d3 = [y1,y3,y5,y7,y9,yB,yD,yF] */

        preload_L2_1x256bytes tmp

        add     tmp, pSrc, srcStride
        vld4.8  {d4,d5,d6,d7},[tmp]
                                        /* d4 = [u0,u1,u2,u3,u4,u5,u6,u7] */
                                        /* d5 = [y0,y2,y4,y6,y8,yA,yC,yE] */
                                        /* d6 = [v0,v1,v2,v3,v4,v5,v6,v7] */
                                        /* d7 = [y1,y3,y5,y7,y9,yB,yD,yF] */

        /* store the Y pixels */
        vst2.8  {d1,d3},[pY]
        add     tmp, pY, y_stride
        vst2.8  {d5,d7},[tmp]

        /* Create an average u and v pixels */
        vhadd.u8 d0, d0, d4
        vhadd.u8 d2, d2, d6

        /* Store the U/V pixels (and update the pointers) */
        vst1.8  {d0},[pU]!
        vst1.8  {d2},[pV]!

        /* Increment Pointers */
        add     pSrc, pSrc, #32
        add     pY,   pY,   #16

        subs    pixelsLeft, pixelsLeft, #16
        bgt     __uyvy_to_iyuv_image_loop_pixel

        add     pSrc, pSrc, srcStep
        add     pSrc, pSrc, srcStride   /* (skip the next line) */
        sub     tmp,  y_stride, width
        add     pY,   pY,   tmp
        add     pY,   pY,   y_stride    /* (skip the next line) */
        sub     tmp,  uv_stride, width, lsr #1 /* The UV plane is w/2 */
        add     pU,   pU,   tmp
        add     pV,   pV,   tmp

        subs    linesLeft, linesLeft, #2
        bgt     __uyvy_to_iyuv_image_loop_line

        EPILOG r0, r12
.unreq width
.unreq height
.unreq pSrc
.unreq srcStride
.unreq pY
.unreq pU
.unreq pV
.unreq y_stride
.unreq uv_stride
.unreq linesLeft
.unreq pixelsLeft
.unreq srcStep
.unreq tmp

.end
