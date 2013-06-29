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

#ifndef _ARXBUFFERTYPES_H_
#define _ARXBUFFERTYPES_H_

namespace tiarx {

/*!
 * \brief Enumeration of all supported data streams
 */
enum BufferTypes_e {
    /*!
     * Start of the Image buffer ID section
     */
    BUFF_IMAGE_START = 0,
    /*!
     * Main camera output - this is typically associated with high resolution preview
     * The color format supported is only FOURCC_NV12
     */
    BUFF_CAMOUT = BUFF_IMAGE_START,
    /*!
     * Secondary camera output - this is typically configured as a lower resolution
     * version of the main camera output
     * The color format supported is only FOURCC_NV12
     */
    BUFF_CAMOUT2,
    /*!
     * Sobel 3x3 operator applied to BUFF_CAMOUT2
     */
    BUFF_SOBEL_3X3,
    /*!
     * Canny 2D x-gradient operator applied to BUFF_CAMOUT2
     * Output is in 16-bit format
     */
    BUFF_CANNY2D_GRADIENT_X,
    /*!
     * Canny 2D y-gradient operator applied to BUFF_CAMOUT2
     * Output is in 16-bit format
     */
    BUFF_CANNY2D_GRADIENT_Y,
    /*!
     * Canny 2D operator (magnitude) applied to BUFF_CAMOUT2
     * Output is in 16-bit format
     */
    BUFF_CANNY2D_GRADIENT_MAG,
    /*!
     * Harris score determined by using BUFF_CAMOUT2, BUFF_CANNY2D_GRADIENT_X and BUFF_CANNY2D_GRADIENT_Y
     * The score output is in 16-bit format
     */
    BUFF_HARRIS_SCORE,
    /*!
     * End of the Image buffer ID section
     */
    BUFF_IMAGE_END = 0xFFFF,

    /*!
     * Start of the generic buffer section
     */
    BUFF_GENERIC_START = 0x10000,
    /*!
     * Buffer ID associated with face detect information
     * @see ARXFaceDetectInfo
     */
    BUFF_FACEDETECT = BUFF_GENERIC_START,
    /*!
    * Buffer ID associated with face detect information
    * This version has a smoothing filtered applied
    * @see ARXFaceDetectInfo
    */
    BUFF_FACEDETECT_FILTERED,
    /*!
     * Camera pose estimation.
     * This is not supported in the current release
     */
    BUFF_CAMERA_POSE,
    /*!
     * Buffer ID associated with facial information (coordinates and orientation).
     * @see ARXFacialPartsInfo
     */
    BUFF_FACEINFO,
     /*!
     * End of the Generic buffer ID section
     */
    BUFF_GENERIC_END = 0x1FFFF,

    /*!
     * Start of the custom buffer ID section
     */
    BUFF_CUSTOM_START = 0x80000000,
};

/*!
 * \brief Face detect information for a single face
 */
struct ARXFace {
    /*!
     * Detection score between 0 and 100
     *    0   means unknown score,
     *    1   means least certain,
     *    100 means most certain the detection is correct
     */
    uint32_t score;
    /*!
     * Top coordinate in pixels of the detected face.
     * This is relative to the size configured for BUFF_CAMOUT
     * NOTE: Face detect locations are always calculated with no mirroring.
     * Mirroring will have to be accounted for by the client
     */
    uint32_t top;
    /*!
     * Left coordinate in pixels of the detected face.
     * This is relative to the size configured for BUFF_CAMOUT
     * NOTE: Face detect locations are always calculated with no mirroring.
     * Mirroring will have to be accounted for by the client
     */
    uint32_t left;
    /*!
     * The width in pixels of the detected face.
     * This is relative to the size configured for BUFF_CAMOUT
     */
    uint32_t w;
    /*!
     * The height in pixels of the detected face.
     * This is relative to the size configured for BUFF_CAMOUT
     */
    uint32_t h;

    /*!
     * The roll angle in degrees (from -30 to 30)
     * The roll angle is defined as the angle between the vertical axis of face and the horizontal axis.
     */
    int32_t roll;

    /*!
     * The priority of each face when multiple faces are detected
     * This is only valid for filtered face detect results; for unfiltered face detect stats this is always set to 0.
     */
    uint32_t priority;
};

/*!
 * \brief Face detect information for all detected faces
 */
struct ARXFaceDetectInfo {
    enum {
        MAX_FACES = 35
    };
    /*!
     * The number of faces detected
     */
    uint32_t numFaces;
    /*!
     * The face detect information for all faces detected
     */
    ARXFace faces[MAX_FACES];
};

/*!
 * \brief 2D plane coordinates
 */
struct ARXCoords {
    int32_t x;
    int32_t y;
};

/*!
 * \brief Facial Parts detect information for a single face
 *        Detects nine parts of the face in a 2D plane.
 */
struct ARXFacialParts {
    /*!
     * Yaw angle of the face
     */
    int32_t yaw;
    /*!
     * Pitch angle of the face
     */
    int32_t pitch;
    /*!
     * Roll angle of the face
     */
    int32_t roll;
    /*!
     * Detection coordinates of a facial part
     */
    ARXCoords lefteyeout;
    /*!
     * Detection coordinates of a facial part
     */
    ARXCoords lefteyein;
    /*!
     * Detection coordinates of a facial part
     */
    ARXCoords righteyeout;
    /*!
     * Detection coordinates of a facial part
     */
    ARXCoords righteyein;
    /*!
     * Detection coordinates of a facial part
     */
    ARXCoords leftnose;
    /*!
     * Detection coordinates of a facial part
     */
    ARXCoords rightnose;
    /*!
     * Detection coordinates of a facial part
     */
    ARXCoords leftmouth;
    /*!
     * Detection coordinates of a facial part
     */
    ARXCoords rightmouth;
    /*!
     * Detection coordinates of a facial part
     */
    ARXCoords centertopmouth;
};

/*!
 * \brief Facial Parts detect information for all detected faces
 */
struct ARXFacialPartsInfo {
    enum {
        MAX_FACES = 35
    };
    /*!
     * The number of faces detected
     */
    uint32_t numFaces;
    /*!
     * The facial detect information for all faces detected
     */
    ARXFacialParts faces[MAX_FACES];
};

struct ARXCameraPoseMatrix {
    float matrix[12];
    int32_t status;
};

}
#endif //_ARXPROPERTIES_H_
