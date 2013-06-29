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

#include <sosal/sosal.h>
#include <dvp_ll.h>
#include <dvp/dvp_debug.h>

#ifdef DVP_USE_IMGFILTER
#include <imgfilter/imgFilter_armv7.h>
#endif

void DVP_imgFilter(DVP_Image_t *pLuma,
                   DVP_ImageFilter_e type,
                   DVP_Image_t *pOut)
{
    int limit = 255;
#ifdef DVP_USE_IMGFILTER
    if (type == DVP_IMGFILTER_SOBEL)
    {
        int sobel_3[3] = {1,2,1};
        int range = SOBEL_RANGE;
        __planar_edge_filter_3x3(pLuma->width,
                                 pLuma->height,
                                 pLuma->pData[0],
                                 pLuma->y_stride,
                                 sobel_3,
                                 pOut->pData[0],
                                 pOut->y_stride,
                                 range, limit);
    }
    else if (type == DVP_IMGFILTER_SCHARR)
    {
        int scharr_3[3] = {3,10,3};
        int range = SCHARR_RANGE;
        __planar_edge_filter_3x3(pLuma->width,
                                 pLuma->height,
                                 pLuma->pData[0],
                                 pLuma->y_stride,
                                 scharr_3,
                                 pOut->pData[0],
                                 pOut->y_stride,
                                 range, limit);
    }
    else if (type == DVP_IMGFILTER_KROON)
    {
        int kroon_3[3] = {17,61,17};
        int range = KROON_RANGE;
        __planar_edge_filter_3x3(pLuma->width,
                                 pLuma->height,
                                 pLuma->pData[0],
                                 pLuma->y_stride,
                                 kroon_3,
                                 pOut->pData[0],
                                 pOut->y_stride,
                                 range, limit);
    }
    else if (type == DVP_IMGFILTER_PREWITT)
    {
        int prewitt_3[3] = {1,1,1};
        int range = PREWITT_RANGE;
        __planar_edge_filter_3x3(pLuma->width,
                                 pLuma->height,
                                 pLuma->pData[0],
                                 pLuma->y_stride,
                                 prewitt_3,
                                 pOut->pData[0],
                                 pOut->y_stride,
                                 range, limit);
    }
#endif
}

