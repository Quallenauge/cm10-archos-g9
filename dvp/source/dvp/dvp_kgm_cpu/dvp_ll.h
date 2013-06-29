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

#ifndef _DVP_LL_H_
#define _DVP_LL_H_

#include <dvp/dvp_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _dvp_imgfilter_e {
    DVP_IMGFILTER_SOBEL,
    DVP_IMGFILTER_SCHARR,
    DVP_IMGFILTER_KROON,
    DVP_IMGFILTER_PREWITT,
} DVP_ImageFilter_e;

void DVP_imgFilter(DVP_Image_t *pLuma, DVP_ImageFilter_e type, DVP_Image_t *pO);

#ifdef __cplusplus
}
#endif

#endif

