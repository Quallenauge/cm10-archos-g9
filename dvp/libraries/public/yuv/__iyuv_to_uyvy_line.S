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

.global __yuv420_to_yuv422_line

.include "yuv.inc"

__yuv420_to_yuv422_line:
yptr        .req r0
uptr        .req r1
vptr        .req r2
dstptr      .req r3
width       .req r4
ystride     .req r5
dststride   .req r6
tmp7        .req r7
tmp8        .req r8
tmp9        .req r9
tmp10       .req r10
tmp11       .req r11
tmp12       .req r12

        PROLOG r0, r12
        /* load the extra arguments from stack to the register*/
        ldr width, [sp, #(14 * 4)]
        ldr ystride, [sp, #(15 * 4)]
        ldr dststride, [sp, #(16 * 4)]

        mov tmp7, width    /* preserve width register */

        /* in a given iteration 16 bytes X 2 of yptr is processed, which means 8 x 2 macropixels */
__yuv420_to_yuv422_line_loop:

        pld         [yptr]
        pld         [yptr, #L2_LINE_SIZE]
        pld         [uptr]
        pld         [vptr]

        /* ROW1 processing */
        vld2.8  {d1,d3}, [yptr]            /* d1 = y0 y2 y4 y6 y8 yA yC yE */
                                           /* d3 = y1 y3 y5 y7 y9 yB yD yF */
        add     yptr, yptr, ystride
        vld1.u8  {d0}, [uptr]!              /* d0 = u01xx u23xx u45xx u67xx u89xx uABxx uCDxx uEFxx (xx = next line) */
        vld1.u8  {d2}, [vptr]!              /* d2 = v01xx v23xx v45xx v67xx v89xx vABxx vCDxx vEFxx */
        vst4.8  {d0,d1,d2,d3}, [dstptr]     /* u01xx y0 v01xx y2... */

        /* ROW2 processing */
        add     dstptr, dstptr, dststride
        vld2.8  {d1,d3}, [yptr]!           /* d1 = yx yx yx..., d3 = yx yx yx... */
        sub     yptr, yptr, ystride        /* peg back the yptr back to the original position + PROCESSBYTES */
        vst4.8  {d0,d1,d2,d3}, [dstptr]!   /* u01xx yx v01xx yx...  (ROW 2) */
        sub     dstptr, dstptr, dststride  /* peg back the yptr back to the original position + (PROCESSBYTES*2) */

        subs    tmp7, tmp7, #16
        bgt     __yuv420_to_yuv422_line_loop

        EPILOG  r0, r12
.unreq yptr
.unreq uptr
.unreq vptr
.unreq dstptr
.unreq width
.unreq ystride
.unreq dststride
.unreq tmp7
.unreq tmp8
.unreq tmp9
.unreq tmp10
.unreq tmp11
.unreq tmp12

.end
