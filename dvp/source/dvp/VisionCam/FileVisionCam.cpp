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

#include <stdio.h>
#include <FileVisionCam.h>
#include <sosal/pyuv.h>
#include <sosal/sosal.h>
#include <time.h>
#include <dvp/dvp.h>
#include <dvp/dvp_debug.h>

char *fourcc_to_str(fourcc_t fcc)
{
    static char code[5];
    strncpy(code, (char *)&fcc, 4);
    code[4] = '\0';
    return code;
}

FileVisionCam::FileVisionCam()
    : CameraEmulator()
{
    m_file = NULL;
    m_type = FILECAM_TYPE_UNKNOWN;
    memset(m_path, 0, sizeof(m_path));
    DVP_PRINT(DVP_ZONE_API, "+FileVisionCam()\n");
}

FileVisionCam::~FileVisionCam()
{
    if (m_file)
        fclose(m_file);
    m_file = NULL;
    DVP_PRINT(DVP_ZONE_API, "-FileVisionCam()\n");
}

uint32_t FileVisionCam::avi_chunk_parse(AVI_Chunk_t *aChunk)
{
    uint32_t s=0,size = aChunk->size + sizeof(fourcc_t) + sizeof(uint32_t);
    DVP_PRINT(DVP_ZONE_CAM, "CHUNK %u %s\n", aChunk->size, fourcc_to_str(aChunk->chunkcode));
    if (aChunk->chunkcode == FOURCC_AVIH)
    {
        AVI_MainHeader_t mainHeader;
        s+=fread(&mainHeader, sizeof(AVI_MainHeader_t), 1, m_file);

        if (m_width != mainHeader.width ||
            m_height != mainHeader.height)
        {
            DVP_PRINT(DVP_ZONE_ERROR, "AVI File Video Dimensions do not match expectations!\n");
        }
    }
    else if (aChunk->chunkcode == FOURCC_STRH)
    {
        AVI_StreamHeader_t strmHeader;
        s+=fread(&strmHeader, sizeof(AVI_StreamHeader_t), 1, m_file);
        if (strmHeader.fccType != FOURCC_VIDS)
        {
            DVP_PRINT(DVP_ZONE_ERROR, "AVI Stream is not video!\n");
        }
        if (strmHeader.fccHandler == FOURCC_NONE && m_space != FOURCC_BGR)
        {
            // NONE is BI_BITFIELDS
            DVP_PRINT(DVP_ZONE_ERROR, "AVI Stream is not in the expected color format!\n");
        }
    }
    else // skip over unknown types
    {
        fseek(m_file, aChunk->size, SEEK_CUR);
        DVP_PRINT(DVP_ZONE_CAM, "\tUnknown CHUNK type %s, moving to %lu offset\n", fourcc_to_str(aChunk->chunkcode), ftell(m_file));
    }
    return size;
}

uint32_t FileVisionCam::avi_list_parse(AVI_List_t *aList)
{
    // we need to report to the caller the entire size of the list
    uint32_t s=0,size = aList->size + sizeof(fourcc_t) + sizeof(uint32_t);
    DVP_PRINT(DVP_ZONE_CAM, "LIST %u %s\n", aList->size, fourcc_to_str(aList->typecode));
    aList->size -= sizeof(fourcc_t); // remove the list type size since we've alread read it
    do
    {
        fourcc_t code; // next sub item
        s+=fread(&code, sizeof(fourcc_t), 1, m_file);
        fseek(m_file, -(long)sizeof(fourcc_t), SEEK_CUR); // back up over the code
        if (code == FOURCC_LIST)
        {
            AVI_List_t anotherList;
            s+=fread(&anotherList, sizeof(AVI_List_t), 1, m_file);
            aList->size -= avi_list_parse(&anotherList);
        }
        else
        {
            AVI_Chunk_t aChunk;
            s+=fread(&aChunk, sizeof(AVI_Chunk_t), 1, m_file);
            aList->size -= avi_chunk_parse(&aChunk);
        }
    } while (aList->size > 0);
    return size;
}

uint32_t FileVisionCam::avi_header_parse(AVI_Header_t *aHeader)
{
    uint32_t s=0;
    if (aHeader->riffcode == FOURCC_RIFF && aHeader->typecode == FOURCC_AVI)
    {
        DVP_PRINT(DVP_ZONE_CAM, "RIFF %u %s\n", aHeader->size, fourcc_to_str(aHeader->typecode));
        m_fileSize = aHeader->size;
        fourcc_t code;
        s+=fread(&code, sizeof(fourcc_t), 1, m_file);
        fseek(m_file,-(long)sizeof(fourcc_t), SEEK_CUR);
        if (code == FOURCC_LIST) // first header
        {
            AVI_List_t aList;
            s+=fread(&aList, sizeof(AVI_List_t), 1, m_file);
            return avi_list_parse(&aList);
        }
        else
        {
            AVI_Chunk_t aChunk;
            s+=fread(&aChunk, sizeof(AVI_Chunk_t), 1, m_file);
            return avi_chunk_parse(&aChunk);
        }
    }
    return 0;
}

uint32_t FileVisionCam::avi_stream_parse(AVI_List_t *aList, DVP_Image_t *pImage)
{
    uint32_t b=0;
    if (aList->listcode == FOURCC_LIST && aList->typecode == FOURCC_MOVI)
    {
        DVP_PRINT(DVP_ZONE_CAM, "Parsing Movie Section!\n");
        fourcc_t code;
        b+=fread(&code, sizeof(fourcc_t), 1, m_file);
        fseek(m_file, -(long)sizeof(fourcc_t), SEEK_CUR);
        if (code == FOURCC_LIST)
        {
            AVI_List_t aList;
            b+=fread(&aList, sizeof(AVI_List_t), 1, m_file);
            //fseek(m_file, aList.size - sizeof(uint32_t), SEEK_CUR);
            return avi_list_parse(&aList);
        }
        else // STREAM TAGS
        {
            AVI_Chunk_t aChunk;
            AVI_StreamTag_t tag;
            uint32_t s = 0;
            b+=fread(&aChunk, sizeof(AVI_Chunk_t), 1, m_file);
            memcpy(&tag, &aChunk.chunkcode, sizeof(tag));
            if (strncmp(tag.streamType,"db",2) == 0)
            {
                uint32_t i,l = 0;
                int32_t y = 0;
                if (m_space == FOURCC_BGR)
                {
                    for (y = pImage->height - 1; y >= 0 ; y--)
                    {
                        i = DVP_Image_PatchOffset(pImage, 0, y, 0);
                        l = DVP_Image_PatchLineSize(pImage, 0);
                        s += fread(&pImage->pData[0][i], 1, l, m_file);
                    }
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "Unsupported AVI to DVP_Image_t reading format (0x%08x)!\n", m_space);
                    fseek(m_file, aChunk.size, SEEK_CUR);
                }
                if (s < aChunk.size)
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "Failed to read enough image data from file! %u < %u\n", s, aChunk.size);
                }
            }
            else
                fseek(m_file, aChunk.size, SEEK_CUR);
            return s;
        }
    }
    else
    {
        DVP_PRINT(DVP_ZONE_CAM, "%s\n", fourcc_to_str(aList->listcode));
    }
    return 0;
}
status_e FileVisionCam::sendCommand(VisionCamCmd_e cmdId,
                                    void *param __attribute__ ((unused)),
                                    uint32_t size __attribute__ ((unused)),
                                    VisionCamPort_e port __attribute__ ((unused)) )
{
    size_t bytes = 0;
    status_e status = STATUS_SUCCESS;
    switch (cmdId)
    {
        case VCAM_CMD_PREVIEW_START:
            if (m_file == NULL)
            {
                char *ext;
                char filename[255];
                memset(filename, 0, sizeof(filename));

                DVP_PRINT(DVP_ZONE_CAM, "NAME: %s\n", m_name);
                DVP_PRINT(DVP_ZONE_CAM, "PATH: %s\n", m_path);
                DVP_PRINT(DVP_ZONE_CAM, "WIDTH: %u, HEIGHT: %u\n", m_width, m_height);

                ext = strchr(m_name, '.');
                if (ext == NULL) // no extension, we wanted to use the pyuv lib
                    m_type = FILECAM_TYPE_PYUV;
                else // there is an extension
                {
                    ext++;
                    if (strcmp(ext, "avi") == 0)
                        m_type = FILECAM_TYPE_AVI;
                    else if (strcmp(ext, "raw") == 0)
                        m_type = FILECAM_TYPE_RAW;
                    else
                        m_type = FILECAM_TYPE_UNKNOWN;
                }

                if (m_type == FILECAM_TYPE_PYUV)
                {
                    DVP_PRINT(DVP_ZONE_CAM, "Using PYUV Library to determine name!\n");
                    PYUV_GetFilename(filename, strcat(m_path, PATH_DELIM), m_name, m_width, m_height, m_fps, m_space);
                }
                else if (m_type == FILECAM_TYPE_AVI)
                {
                    DVP_PRINT(DVP_ZONE_CAM, "Using AVI Parsing!\n");
                    if (m_path[0] != '\0')
                        sprintf(filename, "%s"PATH_DELIM"%s", m_path, m_name);
                    else
                        sprintf(filename, "%s", m_name);
                }
                else if (m_type == FILECAM_TYPE_RAW)
                {
                    DVP_PRINT(DVP_ZONE_CAM, "Using RAW File!\n");
                    if (m_path[0] != '\0')
                        sprintf(filename, "%s"PATH_DELIM"%s", m_path, m_name);
                    else
                        sprintf(filename, "%s", m_name);
                }
                m_file = fopen(filename, (char *)"rb");
                if (m_file)
                {
                    DVP_PRINT(DVP_ZONE_CAM, "Opened file \"%s\"\n", filename);
                    if (m_type == FILECAM_TYPE_AVI)
                    {
                        AVI_Header_t hdr;
                        bytes+=fread(&hdr, sizeof(AVI_Header_t), 1, m_file);
                        avi_header_parse(&hdr);
                    }
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "Failed to open file \"%s\"\n", filename);
                    status = STATUS_FILE_NOT_FOUND;
                    break;
                }
                StartThread(this); // start regardless of file existence...
            }
            break;
        case VCAM_CMD_QUIT:
        case VCAM_CMD_PREVIEW_STOP:
            StopThread();
            if (m_file)
                fclose(m_file);
            m_file = NULL;
            break;
        case VCAM_EXTRA_DATA_START:
        case VCAM_EXTRA_DATA_STOP:
        case VCAM_CMD_LOCK_AE:
        case VCAM_CMD_LOCK_AWB:
        default:
            break;
    }
    return status;
}

thread_ret_t FileVisionCam::RunThread()
{
    int32_t i = 0;
    thread_ret_t err = 0;
    VisionCamFrame *pFrame = new VisionCamFrame();
    DVP_Image_t *pImage = NULL;
    AVI_List_t movList;
    size_t bytes = 0;
    size_t bytesRead;

    if (m_type == FILECAM_TYPE_AVI)
    {
        // read the next LIST, which should be the movie
        fourcc_t code;
        uint32_t b = 0;
        DVP_PRINT(DVP_ZONE_CAM, "Finding MOVI LIST from %lu\n",ftell(m_file));
        do {
            b = fread(&code, sizeof(fourcc_t), 1, m_file);
            if (b == 0) {
                thread_exit(0);
            }
            fseek(m_file, -(long)sizeof(fourcc_t), SEEK_CUR);
            DVP_PRINT(DVP_ZONE_CAM, "FOUND \"%s\"\n", fourcc_to_str(code));
            if (code == FOURCC_LIST)
            {
                bytes+=fread(&movList, sizeof(AVI_List_t), 1, m_file);
            }
            else
            {
                AVI_Chunk_t aChunk;
                bytes+=fread(&aChunk, sizeof(AVI_Chunk_t), 1, m_file);
                fseek(m_file, aChunk.size, SEEK_CUR);
            }
        } while (movList.listcode != FOURCC_LIST && movList.typecode != FOURCC_MOVI);
    }

    while (m_running)
    {
        // wait for the timer to expire
        DVP_PRINT(DVP_ZONE_CAM, "RunThread sleep period is %u msecs\n", m_timer);
        thread_msleep(m_timer);
        if (m_running == false)
            break;
        DVP_PRINT(DVP_ZONE_CAM, "Woke up, processing frame!\n");

        // grab a frame
        i = getNextFreeBuffer();
        if (i == -1)
        {
#ifdef FILE_SKIP_FRAMES
            // skip the frame in the file to simulate frame dropping...
            fseek(m_file, m_buffers[0].size, SEEK_CUR);
            DVP_PRINT(DVP_ZONE_WARNING, "WARNING: No usable buffers, skipping frame!\n");
#endif
            continue; // no free buffers
        }
        else // fill in the data
        {
            bytesRead = 0;
            uint32_t y = 0, p = 0; // don't use i

            pImage = (DVP_Image_t *)m_buffers[i].data;

            if (m_type == FILECAM_TYPE_RAW || m_type == FILECAM_TYPE_PYUV)
            {
                if (pImage->color == FOURCC_UYVY ||
                    pImage->color == FOURCC_NV12 ||
                    pImage->color == FOURCC_Y800)
                {
                    for (p = 0; p < pImage->planes; p++)
                    {
                        DVP_U32 len = DVP_Image_PatchLineSize(pImage, p);
                        DVP_U32 ydiv = DVP_Image_HeightDiv(pImage, p);
                        DVP_U08 *ptr = NULL;
                        for (y = 0; y < pImage->height/ydiv; y++)
                        {
                            ptr = DVP_Image_PatchAddressing(pImage, 0, y*ydiv, p);
                            if (m_file == NULL)
                            {
                                if (y < 1) // Limit the warning to only 1 line per frame, instead of each line
                                {
                                    DVP_PRINT(DVP_ZONE_WARNING, "WARNING: File %s does not exist, using GRAY image instead!\n", m_name);
                                }
                                memset(ptr, 0x80, len);
                                bytesRead += len;
                            }
                            else
                            {
                                bytesRead += fread(ptr, 1, len, m_file);
                            }
                        }
                    }
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "Color format not yet supported for file reader!\n");
                }
            }
            else if (m_type == FILECAM_TYPE_AVI)
            {
                bytesRead = avi_stream_parse(&movList, pImage);
            }
            DVP_PRINT(DVP_ZONE_CAM, "Read "FMT_SIZE_T" bytes from Camera (supposed to read "FMT_SIZE_T")\n",bytesRead,m_buffers[i].size);
        }

        // fill in the frame values
        pFrame->mFrameBuff = m_buffers[i].data;
        pFrame->mCookie = m_cookie;
        pFrame->mFd = 0;
        pFrame->mHeight = m_height;
        pFrame->mWidth = m_width;
        pFrame->mLength = bytesRead; // m_buffers[i].size;
        pFrame->mOffsetX = 0;
        pFrame->mOffsetY = 0;
        pFrame->mStartX = 0;
        pFrame->mStartY = 0;
        pFrame->mTimestamp = rtimer_now();
        m_buffers[i].used = 1;

        // pass it back to the client
        if (m_callback)
        {
            m_callback(pFrame);
        }
        else
        {
            DVP_PRINT(DVP_ZONE_WARNING, "WARNING: Callback to camera has been unregistered\n");
        }

        // client should return it before the next timer....(hopefully)
    }
    delete pFrame;
    DVP_PRINT(DVP_ZONE_CAM, "RunThread exiting!\n");
    return err;
}

status_e FileVisionCam::setParameter(VisionCamParam_e paramId, void* param, uint32_t size,VisionCamPort_e port __attribute__ ((unused)) )
{
    status_e status = STATUS_SUCCESS;
    if (paramId == VCAM_PARAM_PATH)
        strncpy(m_path, (char *)param, (size > MAX_PATH ? MAX_PATH : size));
    else
        status = CameraEmulator::setParameter(paramId, param, size);
    return status;
}

status_e FileVisionCam::getParameter(VisionCamParam_e paramId, void* param, uint32_t size, VisionCamPort_e port __attribute__ ((unused)) )
{
    status_e status = STATUS_SUCCESS;
    if (paramId == VCAM_PARAM_PATH)
        strncpy((char *)param, m_path, (size > MAX_PATH ? MAX_PATH : size));
    else
        status = CameraEmulator::getParameter(paramId, param, size);
    return status;
}


