/*
 *  Copyright (C) 2012 Texas Instruments, Inc.
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

#ifndef _ARXIMAGEBUFFER_H_
#define _ARXIMAGEBUFFER_H_

#include <dvp/dvp_types.h>
#include <arx/ARXBuffer.h>

namespace tiarx {

/*!
 * \brief An image buffer interface
 */
class ARXImageBuffer : public ARXBuffer
{
public:
    /*!
     * If this buffer is bounded to a surface or sufaceTexture, render displays the buffer and transfers
     * ownership back to its buffer manager. If there is no bounded surface, this behaves exactly like release.
     */
    virtual arxstatus_t render() = 0;

    virtual arxstatus_t copyInfo(DVP_Image_t *pImage) = 0;

    /*!
     * Queries the width of the image buffer
     */
    inline uint32_t w() const { return mImage.width; }
    /*!
     * Queries the height of the image buffer
     */
    inline uint32_t h() const { return mImage.height; }
    /*!
     * Queries the bits per pixel of the image buffer
     */
    inline uint32_t bpp() const { return mImage.x_stride; }
    /*!
     * Queries the stride of the image buffer
     */
    inline uint32_t stride() const { return mImage.y_stride; }
    /*!
     * Queries the number of bytes used for the image buffer
     */
    inline uint32_t size() const { return mImage.numBytes; }
    /*!
     * Queries the number of planes used by the image buffer.
     * For example NV12 has 2 planes, Planar RGB has 3 planes, Interleaved RGB has 1 plane, luma only has 1 plane.
     */
    inline uint32_t planes() const { return mImage.planes; }
    /*!
     * Queries the fourcc format used for this image buffer.
     * @see ARXFourcc_e
     */
    inline uint32_t format() const { return mImage.color; }
    /*!
     * Queries for the raw pointer for a plane of the image buffer.
     */
    inline void *data(uint32_t plane) const { return mImage.pData[plane]; }

protected:
    /*!
     * Non-public constructor.
     */
    ARXImageBuffer() {};
    /*!
     * Non-public copy constructor.
     */
    ARXImageBuffer(const ARXImageBuffer&);
    /*!
     * Non-public assignment operator.
     */
    ARXImageBuffer& operator=(const ARXImageBuffer&);
    /*!
     * Non-public destructor.
     */
    virtual ~ARXImageBuffer() {};

    DVP_Image_t mImage;
};

}
#endif //_ARXIMAGEBUFFER_H_
