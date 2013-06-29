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

.global __uyvy_to_yuv444p_image

.include "yuv.inc"

__uyvy_to_yuv444p_image:
width       .req r0
height      .req r1
linesLeft   .req r1
pYUV        .req r2
srcStride   .req r3
srcStep     .req r3
pY          .req r4
pU          .req r5
pV          .req r6
dstStride   .req r7
dstStep     .req r7
pixelsLeft  .req r8
tmp         .req r9
    PROLOG  r0, r12
    ldr     pY, [sp, #(14 * 4)]
    ldr     pU, [sp, #(15 * 4)]
    ldr     pV, [sp, #(16 * 4)]
    ldr     dstStride, [sp, #(17 * 4)]
    add     tmp, width, width
    sub     dstStep, dstStride, width
    sub     srcStep, srcStride, tmp
__uyvy_to_yuv444p_image_lines:
    mov     pixelsLeft, width
__uyvy_to_yuv444p_image_pixels:
    pld     [pYUV, #L2_LINE_SIZE]
    vld4.8  {d0,d1,d2,d3}, [pYUV]!
    vst2.8  {d1,d3}, [pY]!
    vmov    d1, d0
    vst2.8  {d0,d1}, [pU]!
    vmov    d3, d2
    vst2.8  {d2,d3}, [pV]!
    subs    pixelsLeft, pixelsLeft, #16
    bgt     __uyvy_to_yuv444p_image_pixels
    add     pYUV, pYUV, srcStep
    add     pY, pY, dstStep
    add     pU, pU, dstStep
    add     pV, pV, dstStep
   subs    linesLeft, linesLeft, #1
    bgt     __uyvy_to_yuv444p_image_lines
    EPILOG  r0, r12
.unreq width
.unreq height
.unreq pYUV
.unreq srcStride
.unreq srcStep
.unreq pY
.unreq pU
.unreq pV
.unreq dstStride
.unreq dstStep
.unreq linesLeft
.unreq pixelsLeft
.unreq tmp

.end
