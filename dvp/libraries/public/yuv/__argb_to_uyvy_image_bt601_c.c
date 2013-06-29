/*
 *  Copyright (C) 2009-2011 Texas Instruments, Inc.
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

/*! \file
 * \brief The "C" verison of an assembly optimized function.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include "yuv_c.h"

void __argb_to_uyvy_image_bt601(uint32_t width,
                                uint32_t height,
                                uint8_t *pSrc,
                                int32_t srcStride,
                                uint8_t *pDst,
                                int32_t dstStride)
{
    uint32_t x, y;
    int32_t i, j;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x+=2) // writing macro pixels
        {
            RGB8888 a,b;
            YUV422 yuv;
            uint8_t u0,u1,v0,v1;

            i = (y * srcStride) + (x * sizeof(RGB8888));
            j = (y * dstStride) + (x * sizeof(uint16_t));

            a = *(RGB8888 *)(&pSrc[i]);
            b = *(RGB8888 *)(&pSrc[i+sizeof(RGB8888)]);

            CONVERT_RGB_TO_YUV_BT601_Q14(a.ARGB.R,a.ARGB.G,a.ARGB.B,yuv.UYVY.Y0,u0,v0);
            CONVERT_RGB_TO_YUV_BT601_Q14(b.ARGB.R,b.ARGB.G,b.ARGB.B,yuv.UYVY.Y1,u1,v1);

            yuv.UYVY.U0 = (u0 + u1)>1;
            yuv.UYVY.V0 = (v0 + v1)>1;

            *(YUV422 *)(&pDst[j]) = yuv;
        }
    }
}
