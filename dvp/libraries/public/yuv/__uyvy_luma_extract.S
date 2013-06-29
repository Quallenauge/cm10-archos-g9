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

.global __uyvy_luma_extract

.include "yuv.inc"

__uyvy_luma_extract:

width       .req r0
height      .req r1
pYUV        .req r2
srcStride   .req r3
pY          .req r4
dstStride   .req r5
linesLeft   .req r6
srcStep     .req r7
dstStep     .req r8
tmp         .req r9
pixelsLeft  .req r10
    PROLOG  r0, r10
    ldr     pY,         [sp, #(12 * 4)]
    ldr     dstStride,  [sp, #(13 * 4)]
    mov     linesLeft,  height
    add     tmp, width, width
    sub     srcStep, srcStride, tmp
    sub     dstStep, dstStride, width
__uyvy_luma_extract_lines:
    mov     pixelsLeft, width
    pld     [pYUV]
    pld     [pYUV, srcStride]
__uyvy_luma_extract_pixels:
    pld     [pYUV, #L2_LINE_SIZE]
    add     tmp, pYUV, srcStride
    pld     [tmp,  #L2_LINE_SIZE]
    vld4.8  {d0,d1,d2,d3}, [pYUV]!
    vst2.8  {d1,d3}, [pY]!
    subs    pixelsLeft, #16
    bgt     __uyvy_luma_extract_pixels
    add     pYUV, pYUV, srcStep
    add     pY, pY, dstStep
    subs    linesLeft, linesLeft, #1
    bgt     __uyvy_luma_extract_lines
    EPILOG  r0, r10
.unreq width
.unreq height
.unreq pYUV
.unreq srcStride
.unreq pY
.unreq dstStride
.unreq linesLeft
.unreq srcStep
.unreq dstStep
.unreq tmp
.unreq pixelsLeft

.end
