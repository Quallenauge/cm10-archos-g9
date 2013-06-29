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

#ifndef _FILE_VISION_CAM_H_
#define _FILE_VISION_CAM_H_

#include <sosal/sosal.h>
#include <CameraEmulator.h>

typedef struct _avi_header_t {
    fourcc_t riffcode;
    uint32_t size;
    fourcc_t typecode;
} AVI_Header_t;

typedef struct _avi_list_t {
    fourcc_t listcode;
    uint32_t size;
    fourcc_t typecode;
} AVI_List_t;

typedef struct _avi_chunk_t {
    fourcc_t chunkcode;
    uint32_t size;
} AVI_Chunk_t;

typedef struct _avi_mainheader_t {
    uint32_t microSecPerFrame;
    uint32_t maxBytesPerSec;
    uint32_t paddingGranularity;
    uint32_t flags;
    uint32_t totalFrames;
    uint32_t initialFrames;
    uint32_t streams;
    uint32_t suggestedBufferSize;
    uint32_t width;
    uint32_t height;
    uint32_t reserved[4];
} AVI_MainHeader_t;

typedef struct _avistreamheader {
    fourcc_t fccType;
    fourcc_t fccHandler;
    uint32_t flags;
    uint16_t priority;
    uint16_t language;
    uint32_t initialFrames;
    uint32_t scale;
    uint32_t rate;
    uint32_t start;
    uint32_t length;
    uint32_t suggestedBufferSize;
    uint32_t quality;
    uint32_t sampleSize;
    struct {
        short int left;
        short int top;
        short int right;
        short int bottom;
    }  rcFrame;
} AVI_StreamHeader_t;

typedef struct _avi_stream_tag_t {
    uint16_t streamNum;
    char     streamType[2];
} AVI_StreamTag_t;

typedef enum _filecamtype_e {
    FILECAM_TYPE_UNKNOWN,
    FILECAM_TYPE_PYUV, // Use the PYUV Library to determine names and format info
    FILECAM_TYPE_AVI,
    FILECAM_TYPE_RAW,
} FileCamType_e;

typedef enum _filecamparam_e {
    FILECAM_PARAM_PATH = VCAM_PARAM_MAX,
    FILECAM_PARAM_MAX,
} FileCamParam_e;

class FileVisionCam : public CameraEmulator
{
protected:
    FILE          *m_file;
    FileCamType_e  m_type;
    char           m_path[MAX_PATH];
    size_t         m_fileSize;

    uint32_t avi_list_parse(AVI_List_t *aList);
    uint32_t avi_chunk_parse(AVI_Chunk_t *aChunk);
    uint32_t avi_header_parse(AVI_Header_t *aHeader);
    uint32_t avi_stream_parse(AVI_List_t *aList, DVP_Image_t *pImage);

public:
    FileVisionCam();
    virtual ~FileVisionCam();

    thread_ret_t RunThread();

    status_e sendCommand(VisionCamCmd_e cmdId, void *param, uint32_t size, VisionCamPort_e port = VCAM_PORT_ALL );
    status_e setParameter(VisionCamParam_e paramId, void *param, uint32_t size, VisionCamPort_e port = VCAM_PORT_ALL );
    status_e getParameter(VisionCamParam_e paramId, void *param, uint32_t size, VisionCamPort_e port = VCAM_PORT_ALL );
};

#endif

