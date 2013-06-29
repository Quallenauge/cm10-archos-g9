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

.global __nv12_to_half_yuv444_image

.include "yuv.inc"

/** This function takes a NV12 image and 1/2 scales the Luma plane and separates the UV plane. The function assumes a packed set of Y/UV planes */
__nv12_to_half_yuv444_image:
width       .req r0
height      .req r1
pY0         .req r2
pUV         .req r3
srcStride   .req r4
pY          .req r5
pU          .req r6
pV          .req r7
dstStride   .req r8
linesLeft   .req r9
pixelsLeft  .req r10
tmp         .req r11
preLoadIdx  .req r12
        PROLOG  r0, r12
        ldr     srcStride, [sp, #(14 * 4)]
        ldr     pY, [sp, #(15 * 4)]
        ldr     pU, [sp, #(16 * 4)]
        ldr     pV, [sp, #(17 * 4)]
        ldr     dstStride, [sp, #(18 * 4)]

        /* Use the height of the destination to control the loop */
        mov     linesLeft, height, lsr #1

__nv12_to_half_yuv444_image_lines:

        /* Use the width of the destination to control the loop */
        mov     pixelsLeft, width, lsr #1

        /* Force the preload on the next block */
        mov     preLoadIdx, #(L2_LINE_SIZE/16)

        /* Preload first block for 3 lines worth */
        mov     tmp, pY0
        pld     [tmp]
        add     tmp, pY0, srcStride
        pld     [tmp]
        mov     tmp, pUV
        pld     [tmp]
        ldr     tmp, [tmp] /* wait for the preload to finish */

__nv12_to_half_yuv444_image_pixels:

        /* Preload the next block if the index is correct */
        subs    preLoadIdx, preLoadIdx, #(L2_LINE_SIZE/16)
        bne     __nv12_to_half_yuv444_image_algo
        add     tmp, pY0, #L2_LINE_SIZE
        pld     [tmp]
        add     tmp, tmp, srcStride
        pld     [tmp]
        add     tmp, pUV, #L2_LINE_SIZE
        pld     [tmp]

__nv12_to_half_yuv444_image_algo:
        add     preLoadIdx, preLoadIdx, #1

        add     tmp, pY0, srcStride
        vld1.8  {d0}, [pY0]!
        vld1.8  {d1}, [pY0]!
        vld1.8  {d2}, [tmp]!
        vld1.8  {d3}, [tmp]!

        /* Average 16x2 to 8x1 */
        vpaddl.u8 d0, d0
        vpaddl.u8 d1, d1
        vpaddl.u8 d2, d2
        vpaddl.u8 d3, d3
        vqadd.u16 d0, d0, d2
        vqadd.u16 d1, d1, d3
        vshrn.u16 d0, q0, #2
        /* Store 8x1 */
        vst1.8  {d0}, [pY]!

        /* Split UV plane into U/V planes */
        vld2.8  {d4, d5}, [pUV]!
        vst1.8  {d4}, [pU]!
        vst1.8  {d5}, [pV]!

        subs    pixelsLeft, pixelsLeft, #8
        bgt     __nv12_to_half_yuv444_image_pixels

        /* update the pY0, pUV with the correct step */
        sub     tmp, srcStride, width
        add     pY0, pY0, tmp
        add     pY0, pY0, srcStride
        add     pUV, pUV, tmp           /* since this is a packed plane, the "step" is the same */

        /* update the pY,pU,pV pointers */
        sub     tmp, dstStride, width, lsr #1
        add     pY, pY, tmp
        add     pU, pU, tmp
        add     pV, pV, tmp

        subs    linesLeft, linesLeft, #1
        bgt     __nv12_to_half_yuv444_image_lines

        EPILOG      r0, r12

.unreq preLoadIdx
.unreq tmp
.unreq pixelsLeft
.unreq linesLeft
.unreq dstStride
.unreq pY
.unreq pU
.unreq pV
.unreq srcStride
.unreq pUV
.unreq pY0
.unreq height
.unreq width

.end
