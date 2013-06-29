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

#ifndef _IMAGE_DEBUGGING_H_
#define _IMAGE_DEBUGGING_H_

#include <dvp/dvp.h>

/*! The structure used to track image debugging information
 * \ingroup group_debugging
 */
typedef struct _image_debug_t {
    char filename[MAX_PATH];       /**< The name of the debug file */
#if defined(DVP_USE_FS)
    FILE *debug;                   /**< Used to maintain a debug file output of this image */
#endif
    DVP_Image_t *pImg;
    DVP_Buffer_t *pBuf;
} ImageDebug_t;

#ifdef __cplusplus
extern "C" {
#endif


/*! \brief Used to initialize an image debugging structure
 * \ingroup group_debugging
 */
void ImageDebug_Init(ImageDebug_t *pImgd, DVP_Image_t *pImage, const char *path, const char *name);

/*! \brief Used to initialize a buffer debugging structure
 * \ingroup group_debugging
 */
void BufferDebug_Init(ImageDebug_t *pImgd, DVP_Buffer_t *pBuffer, const char *path, const char *name, const char *ext);

/*! \brief Used to open the files associated with the image debugging structure.
 * \ingroup group_debugging
 */
void ImageDebug_Open(ImageDebug_t *pImgd, DVP_U32 numImg);

/*! \brief Used to write the current frame out to the file.
 * \ingroup group_debugging
 */
void ImageDebug_Write(ImageDebug_t *pImgd, DVP_U32 numImg);

/*! \brief Used to close the file and deinitialize an image debugging structure
 * \ingroup group_debugging
 */
void ImageDebug_Close(ImageDebug_t *pImgd, DVP_U32 numImg);

#ifdef __cplusplus
}
#endif
#endif

