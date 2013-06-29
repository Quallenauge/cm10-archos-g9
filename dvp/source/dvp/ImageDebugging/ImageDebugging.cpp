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

#include <dvp/ImageDebugging.h>
#include <dvp/dvp_debug.h>

#ifdef DVP_USE_TILER
#undef PAGE_SIZE // this is remove a warning
#include <tilermem_utils.h>
#endif

void ImageDebug_Init(ImageDebug_t *pImgd, DVP_Image_t *pImage, const char *path, const char *name)
{
    if (pImgd && pImage)
    {
        PYUV_GetFilename(pImgd->filename, (char *)path, (char *)name, pImage->width, pImage->height, 30, (fourcc_t)pImage->color);
#if defined(DVP_USE_FS)
        pImgd->debug = NULL;
#endif
        pImgd->pImg = pImage;
        pImgd->pBuf = NULL;
        DVP_PRINT(DVP_ZONE_IMGDBG, "Filename = %s\n", pImgd->filename);
    }
    else
    {
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR: ImageDgb = %p, pImage = %p\n", pImgd, pImage);
    }
}

void BufferDebug_Init(ImageDebug_t *pImgd, DVP_Buffer_t *pBuffer, const char *path, const char *name, const char *ext)
{
    if (pImgd && pBuffer)
    {
        strcpy(pImgd->filename,(char *)path);
        strcat(pImgd->filename,(char *)name);
        strcat(pImgd->filename,(char *)ext);
#if defined(DVP_USE_FS)
        pImgd->debug = NULL;
#endif
        pImgd->pImg = NULL;
        pImgd->pBuf = pBuffer;
        DVP_PRINT(DVP_ZONE_IMGDBG, "Filename = %s\n", pImgd->filename);
    }
}

void ImageDebug_Open(ImageDebug_t *pImgd, DVP_U32 numImg)
{
#if defined(DVP_USE_FS)
    DVP_U32 i = 0;
    // open all the files for writing
    for (i = 0; i < numImg; i++)
    {
        pImgd[i].debug = fopen(pImgd[i].filename, "w+");
        if (pImgd[i].debug == NULL) {
            DVP_PRINT(DVP_ZONE_ERROR, "Error opening file [%u]%s for writing\n",i,pImgd[i].filename);
        } else {
            DVP_PRINT(DVP_ZONE_IMGDBG, "Opening file %s for debug output!\n",pImgd[i].filename);
        }
    }
#endif
}

void ImageDebug_Write(ImageDebug_t *pImgd, DVP_U32 numImg)
{
#if defined(DVP_USE_FS)
    if (pImgd)
    {
        DVP_U32 i,y,p,len,n = 0;

        // write out all the data
        for (i = 0; i < numImg; i++)
        {
            DVP_Image_t *pImage = pImgd[i].pImg;
            DVP_Buffer_t *pBuffer = pImgd[i].pBuf;
            DVP_U08 *ptr = NULL;

            n = 0; // initialize for each debug image;

            if (pBuffer)
            {
                n += (uint32_t)fwrite(pBuffer->pData, 1, pBuffer->numBytes, pImgd[i].debug);
                fflush(pImgd[i].debug);
            }
            else if(pImage)
            {
                // write out each plane patch.
                for (p = 0; p < pImage->planes; p++)
                {
                    DVP_U32 ydiv = DVP_Image_HeightDiv(pImage, p);
                    len = DVP_Image_PatchLineSize(pImage, p);
                    for (y = 0; y < pImage->height/ydiv; y++)
                    {
                        ptr = DVP_Image_PatchAddressing(pImage, 0, y*ydiv, p);
                        n += (uint32_t)fwrite(ptr, 1, len, pImgd[i].debug);
                    }
                    fflush(pImgd[i].debug);
                }
            }
            else
            {
                DVP_PRINT(DVP_ZONE_ERROR, "DEBUG image/buffer pointers are NULL for file %s\n", pImgd[i].filename);
            }
            DVP_PRINT(DVP_ZONE_IMGDBG, "Wrote %u bytes to %s\n", n, pImgd[i].filename);
        }
    }
#endif
}

void ImageDebug_Close(ImageDebug_t *pImgd, DVP_U32 numImg)
{
#if defined(DVP_USE_FS)
    DVP_U32 i = 0;
    for (i = 0; i < numImg; i++)
        fclose(pImgd[i].debug);
#endif
}

