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

#ifndef _TRANSFORMERS_H_
#define _TRANSFORMERS_H_

void bgr_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input);
void y800_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input);
void nv21_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input);
void yuy2_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input);
void uyvy_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input);
void rgb565_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input);
void rgbp_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input);
void yv12_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input);
void yv16_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input);
void yv24_to_nv12(ANativeWindow_Buffer *output, DVP_Image_t* input);

#endif // _TRANSFORMERS_H_
