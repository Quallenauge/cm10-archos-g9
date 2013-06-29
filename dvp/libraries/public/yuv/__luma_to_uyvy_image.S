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

.global __luma_to_uyvy_image

.include "yuv.inc"

__luma_to_uyvy_image:
width       .req r0
height      .req r1
pY          .req r2
srcStride   .req r3
pUYVY       .req r4
dstStride   .req r5
linesLeft   .req r6
srcStep     .req r7
dstStep     .req r8
tmp         .req r9
pixelsLeft  .req r10
    PROLOG  r0, r10
    ldr     pUYVY,      [sp, #(12 * 4)]
    ldr     dstStride,  [sp, #(13 * 4)]
    mov     linesLeft,  height
    add     tmp, width, width
    sub     srcStep, srcStride, width
    sub     dstStep, dstStride, tmp
    /* Prime q1 with U/V values that are "zero" */
    vmov.u16    q2, #0x0080
__luma_to_uyvy_image_lines:
    mov     pixelsLeft, width
    pld     [pY]
    pld     [pY, srcStride]
__luma_to_uyvy_image_pixels:
    pld     [pY, #L2_LINE_SIZE]
    add     tmp, pY, srcStride
    pld     [tmp,  #L2_LINE_SIZE]
    vld1.8  {d0}, [pY]!
    vld1.8  {d1}, [pY]!
    /* Load q1 first so that d1 isn't corrupted */
    vshll.u16 q1, d1, #8
    vshll.u16 q0, d0, #8
    vorr.16 q1, q1, q2
    vorr.16 q0, q0, q2
    vst1.8  {d0}, [pUYVY]!
    vst1.8  {d1}, [pUYVY]!
    vst1.8  {d2}, [pUYVY]!
    vst1.8  {d3}, [pUYVY]!
    subs    pixelsLeft, #16
    bgt     __luma_to_uyvy_image_pixels
    add     pY, pY, srcStep
    add     pUYVY, pUYVY, dstStep
    subs    linesLeft, linesLeft, #1
    bgt     __luma_to_uyvy_image_lines
    EPILOG  r0, r10
.unreq width
.unreq height
.unreq pY
.unreq srcStride
.unreq pUYVY
.unreq dstStride
.unreq linesLeft
.unreq srcStep
.unreq dstStep
.unreq tmp
.unreq pixelsLeft

.end
