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
#include <stdio.h>
#include <string.h>

typedef struct _fourcc_names_t {
    fourcc_t fourcc;
    char *name;
    char *ext;
} FOURCC_Name_t;

FOURCC_Name_t fourccNames[] = {
    {FOURCC_UYVY, "UYVY",     "yuv"},
    {FOURCC_YUY2, "YUY2",     "yuv"},
    {FOURCC_YVYU, "YVYU",     "yuv"},
  //{FOURCC_VYUY, "VYUY",     "yuv"},
    {FOURCC_YUV9, "P411",     "yuv"},
    {FOURCC_YVU9, "P411",     "yuv"},
    {FOURCC_YV12, "P420",     "yuv"},
    {FOURCC_IYUV, "IYUV",     "yuv"},
    {FOURCC_YU16, "P422",     "yuv"},
    {FOURCC_YV16, "P422",     "yuv"},
    {FOURCC_YU24, "P444",     "yuv"},
    {FOURCC_YV24, "P444",     "yuv"},
    {FOURCC_Y800, "P400",     "bw"},
    {FOURCC_Y16,  "P400_16b", "bw"},
    {FOURCC_Y32,  "P400_32b", "bw"},
    {FOURCC_RGBP, "P444",     "rgb"},
    {FOURCC_BGR,  "I444",     "rgb"},
    {FOURCC_HSLP, "P444",     "hsl"},
    {FOURCC_RGBA, "I32",      "rgba"},
    {FOURCC_NV12, "P400",     "bw"},
    {FOURCC_NV21, "P400",     "bw"},
    {FOURCC_RGB565, "I565",   "rgb"},
    {FOURCC_BGR565, "I565",   "rgb"},
};
unsigned int numFourccNames = dimof(fourccNames);

char *PYUV_GetFileExtension(fourcc_t fourcc)
{
    unsigned int i = 0;
    for (i = 0; i < numFourccNames; i++)
    {
        if (fourcc == fourccNames[i].fourcc)
            return fourccNames[i].ext;
    }
    return NULL;
}

char *PYUV_GetColorSpaceName(fourcc_t fourcc)
{
    unsigned int i = 0;
    for (i = 0; i < numFourccNames; i++)
    {
        if (fourcc == fourccNames[i].fourcc)
            return fourccNames[i].name;
    }
    return NULL;
}

fourcc_t PYUV_GetColorSpace(char *name, char *ext)
{
    unsigned int i = 0;
    for (i = 0; i < numFourccNames; i++)
    {
        if (strcmp(fourccNames[i].name, name) == 0 && strcmp(fourccNames[i].ext,ext) == 0)
            return fourccNames[i].fourcc;
    }
    return FOURCC_Y800;
}

void PYUV_GetFilename(char *filename,
                      char *path,
                      char *name,
                      unsigned int width,
                      unsigned int height,
                      unsigned int fps,
                      fourcc_t fourcc)
{
    unsigned int modHeight = height;
    if(fourcc == FOURCC_NV12 || fourcc == FOURCC_NV21)
        modHeight = height*3/2;

    sprintf(filename, "%s%s_%ux%u_%uHz_%s.%s", path, name, width, modHeight, fps, PYUV_GetColorSpaceName(fourcc), PYUV_GetFileExtension(fourcc));
}

static char strindex(char *str, unsigned int index)
{
    return str[index];
}

void PYUV_DecodeFilename(char *filename,
                         uint32_t *width,
                         uint32_t *height,
                         uint32_t *fps,
                         fourcc_t *color)
{
    char ext[10];
    char fourcc[5];
    char shortname[MAX_PATH];
    char *p = filename;
    int count;

    *width = 0;
    *height = 0;
    *fps = 0;
    *color = FOURCC_NONE;

    // move through the path
    while (*p!='\0') {
        if (*p == strindex(PATH_DELIM,0)) {
            filename = p+1;
        }
        p++;
    }
    sscanf(filename, "%02u_%[a-zA-Z0-9]_%ux%u_%uHz_%[a-zA-Z0-9].%[a-z]", &count, shortname, width, height, fps, fourcc, ext);
    *color = PYUV_GetColorSpace(fourcc, ext);
}


