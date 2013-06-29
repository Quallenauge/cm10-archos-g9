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

#ifndef _DVP_MEM_H_
#define _DVP_MEM_H_

/*! \file
 * \brief The DVP Memory API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <dvp/dvp_types.h>
#include <sosal/fourcc.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Unmaps and frees memory from remote cores.
 * \param [in] handle The handle to the DVP system.
 * \param [in] ptr The pointer to free.
 * \param [in] numElem The number of elements in the array.
 * \param [in] sizeElem The size of each element.
 * \ingroup group_memory
 */
void dvp_free(DVP_Handle handle, DVP_PTR ptr, DVP_U32 numElem, DVP_U32 sizeElem);

/*!
 * \brief Allocates and clears virtual memory, internally takes care of mapping to remote cores.
 * \param [in] handle The handle to the DVP system.
 * \param [in] numElem The number of elements to allocate.
 * \param [in] sizeElem The size of each element in bytes.
 * \ingroup group_memory
 */
DVP_PTR dvp_calloc(DVP_Handle handle, DVP_U32 numElem, DVP_U32 sizeElem);

/*!
 * \brief Initializes the image structure to the correct parameters for the color format.
 * \note Does not allocate any memory!
 * \param [in] pImage The pointer to the structure to be filled in.
 * \param [in] width The desired width in pixels.
 * \param [in] height The desired height in pixels.
 * \param [in] fourcc The desired color space.
 * \ingroup group_images
 */
void DVP_Image_Init(DVP_Image_t *pImage, DVP_U32 width, DVP_U32 height, fourcc_t fourcc);

/*!
 * \brief Allocates and maps an image to all enabled remote cores.
 * \param [in] handle The handle to DVP.
 * \param [in] pImage The pointer to the initialized image structure.
 * \param [in] dvpMemType The desired memory allocation type.
 * \note Display buffers are already allocated if acquired through DVP_Display_Alloc. Use an appropriate mem type which will not cause a reallocation.
 * \ingroup group_images
 */
DVP_BOOL DVP_Image_Alloc(DVP_Handle handle, DVP_Image_t *pImage, DVP_MemType_e dvpMemType);

/*!
 * \brief Unmaps and Frees the allocated memory in the image.
 * \param [in] handle The handle to DVP.
 * \param [in] pImage The pointer to the image structure whose data will be freed. The structure will NOT be cleared afterward.
 * \ingroup group_images
 */
DVP_BOOL DVP_Image_Free(DVP_Handle handle, DVP_Image_t *pImage);

/*!
 * \brief Performs the necessary operations to share this image with another process
 * \param [in] handle The handle to DVP.
 * \param [in] pImage The pointer to the image structure whose data will be shared.
 * \param [in] shared_fds the file descriptors that can be used to import this image in a different process
 * \note the shared_fds array size must match the number of planes in the image
 * \ingroup group_images
 */
DVP_BOOL DVP_Image_Share(DVP_Handle handle, DVP_Image_t *pImage, DVP_S32 *shared_fds);

/*!
 * \brief Import the image into the calling process
 * \param [in] handle The handle to DVP.
 * \param [in] pImage The pointer to the image structure whose data will be imported.
 * \param [in] shared_fds the file descriptors obtained from DVP_Image_Share
 * \param [in] hdls internal handles, needed to free the image through DVP_Image_Free_Import
 * \note the hdls array size must match the number of planes in the image
 * \note This image can only be freed through DVP_Image_Free_Import
 * \ingroup group_images
 */
DVP_BOOL DVP_Image_Import(DVP_Handle handle, DVP_Image_t *pImage, DVP_S32 *shared_fds, DVP_VALUE *hdls);

/*!
 * \brief Free an imported image
 * \param [in] handle The handle to DVP.
 * \param [in] pImage The pointer to the image structure whose data will be freed.
 * \param [in] hdls the internal handles obtained during DVP_Image_Import
 * \note only imported images can be freed through this API
 * \ingroup group_images
 */
DVP_BOOL DVP_Image_Free_Import(DVP_Handle handle, DVP_Image_t *pImage, DVP_VALUE *hdls);

/*!
 * \brief Deinitializes the image structure.
 * \param [in] pImage the data structure to clear.
 * \ingroup group_images
 */
void DVP_Image_Deinit(DVP_Image_t *pImage);

/*!
 * \brief Copies a flat buffer into a DVP Image structure.
 * \param pImage The image to fill.
 * \param ptr The pointer to the data to fill from.
 * \param size The number of bytes in ptr.
 * \ingroup group_images
 */
void DVP_Image_Fill(DVP_Image_t *pImage, DVP_S08 *ptr, uint32_t size);

/*!
 * \brief Copies the source image to the destination image with no conversions other than stride.
 * \param [in] pDst The destination image.
 * \param [in] pSrc The source image.
 * \ingroup group_images
 */
DVP_BOOL DVP_Image_Copy(DVP_Image_t *pDst, DVP_Image_t *pSrc);

/*!
 * \brief Duplicates the source DVP_Image_t structure into another structure. No image data is copied, only metadata.
 * \param [in] dst The destintation image metadata structure.
 * \param [in] src The source image metadata structure.
 * \ingroup group_images
 */
void DVP_Image_Dup(DVP_Image_t* dst, DVP_Image_t *src);

/*!
 * \brief Serializes the meta data about the image and the image itself into a flat buffer.
 * \param [in] pImage The image to serialize.
 * \param [in] buffer The buffer to serialize into.
 * \param [in] len The length of the buffer to prevent overflows.
 * \ingroup group_images
 */
size_t DVP_Image_Serialize(DVP_Image_t *pImage, uint8_t *buffer, size_t len);

/*!
 * \brief Unserializes the meta data and the image from a flat buffer.
 * \param [in] pImage The image to unserialize.
 * \param [in] buffer The buffer to unserialize from.
 * \param [in] len The length of the buffer to prevent overflows.
 * \ingroup group_images
 */
size_t DVP_Image_Unserialize(DVP_Image_t *pImage, uint8_t *buffer, size_t len);

/*! \brief Returns the height divisor for that plane index of the image.
 * \param [in] pImage The image structure.
 * \param [in] plane The index of the plane desired.
 * \ingroup group_images
 */
DVP_U32 DVP_Image_HeightDiv(DVP_Image_t *pImage, DVP_U32 plane);

/*! \brief Returns the width divisor for that plane index of the image.
 * \param [in] pImage The image structure.
 * \param [in] plane The index of the plane desired.
 * \ingroup group_images
 */
DVP_U32 DVP_Image_WidthDiv(DVP_Image_t *pImage, DVP_U32 plane);

/*! \brief Calculates the size the patch plane's lines in bytes.
 * The patch is the sub-image defined by {x_start,y_start} by {width,height}.
 * \param [in] pImage the image meta-pointer.
 * \param [in] plane The plane index.
 * \return Returns the lineSize in bytes.
 * \ingroup group_images
 */
DVP_U32 DVP_Image_PatchLineSize(DVP_Image_t *pImage, DVP_U32 plane);

/*!
 * \brief Calculates the size of the patch plane indexed as given.
 * The patch is the sub-image defined by {x_start,y_start} by {width,height}.
 * \param [in] pImage The image pointer to calculate the plane size of.
 * \param [in] plane The plane index to use to get the size from.
 * \ingroup group_images
 */
DVP_U32 DVP_Image_PatchPlaneSize(DVP_Image_t *pImage, DVP_U32 plane);

/*!
 * \brief Calculates the byte size of all the patch data in the image.
 * The patch is the sub-image defined by {x_start,y_start} by {width,height}.
 * \param [in] pImage The pointer to the image structure.
 * \ingroup group_images
 */
DVP_U32 DVP_Image_PatchSize(DVP_Image_t *pImage);

/*! \brief Calculates the size the plane's lines in bytes.
 * \param [in] pImage the image meta-pointer.
 * \param [in] plane The plane index.
 * \return Returns the lineSize in bytes.
 * \ingroup group_images
 */
DVP_U32 DVP_Image_LineSize(DVP_Image_t *pImage, DVP_U32 plane);

/*!
 * \brief Calculates the size of the plane indexed as given.
 * \param [in] pImage The image pointer to calculate the plane size of.
 * \param [in] plane The plane index to use to get the size from.
 * \ingroup group_images
 */
DVP_U32 DVP_Image_PlaneSize(DVP_Image_t *pImage, DVP_U32 plane);

/*!
 * \brief Calculates the byte size of all the data in the image.
 * \param [in] pImage The pointer to the image structure.
 * \ingroup group_images
 */
DVP_U32 DVP_Image_Size(DVP_Image_t *pImage);

/*! \brief Calculates the range the plane's lines in bytes.
 * \param [in] pImage the image meta-pointer.
 * \param [in] plane The plane index.
 * \return Returns the lineSize in bytes.
 * \ingroup group_images
 */
DVP_U32 DVP_Image_LineRange(DVP_Image_t *pImage, DVP_U32 plane);

/*!
 * \brief Calculates the range of the plane indexed as given.
 * \param [in] pImage The image pointer to calculate the plane size of.
 * \param [in] plane The plane index to use to get the size from.
 * \ingroup group_images
 */
DVP_U32 DVP_Image_PlaneRange(DVP_Image_t *pImage, DVP_U32 plane);

/*!
 * \brief Calculates the byte range of all the data in the image.
 * \param [in] pImage The pointer to the image structure.
 * \ingroup group_images
 */
DVP_U32 DVP_Image_Range(DVP_Image_t *pImage);

/*! \brief Computes the pointer to an X,Y location on a plane of the image.
 * \param [in] pImage The pointer to the image structure.
 * \param [in] x The x coordinate.
 * \param [in] y The y coordinate.
 * \param [in] p The plane index.
 * \returns Returns a pointer to the requested pixel.
 * \note This pointer is offset from the image patch, not the allocated base.
 */
DVP_U08 *DVP_Image_PatchAddressing(DVP_Image_t *pImage, DVP_U32 x, DVP_U32 y, DVP_U32 p);

/*! \brief Computes the pointer to an X,Y location on a plane of the image.
 * \param [in] pImage The pointer to the image structure.
 * \param [in] x The x coordinate.
 * \param [in] y The y coordinate.
 * \param [in] p The plane index.
 * \returns Returns a pointer to the requested pixel.
 * \note This pointer is offset from the allocated base of the image, not the patch.
 */
DVP_U08 *DVP_Image_Addressing(DVP_Image_t *pImage, DVP_U32 x, DVP_U32 y, DVP_U32 p);

/*! \brief Computes the per plane offset to an X,Y location of the image patch.
 * \param [in] pImage The pointer to the image structure.
 * \param [in] x The x coordinate.
 * \param [in] y The y coordinate.
 * \param [in] p The plane index.
 * \returns Returns an offset to the requested pixel on a plane.
 * \note Patch offsets are bounded by width and height, not bufWidth and bufHeight.
 */
DVP_U32 DVP_Image_PatchOffset(DVP_Image_t *pImage, DVP_U32 x, DVP_U32 y, DVP_U32 p);

/*! \brief Computes the per plane offset to an X,Y location of the image.
 * \param [in] pImage The pointer to the image structure.
 * \param [in] x The x coordinate.
 * \param [in] y The y coordinate.
 * \param [in] p The plane index.
 * \returns Returns an offset to the requested pixel on a plane.
 */
DVP_U32 DVP_Image_Offset(DVP_Image_t *pImage, DVP_U32 x, DVP_U32 y, DVP_U32 p);

/*! \brief Alters the image structure to set the image patch sub-image.
 * \param [in] pImage The pointer to the image structure.
 * \param [in] x_start The start x coordinate.
 * \param [in] y_start The start y coordinate.
 * \param [in] width The new sub-image width.
 * \param [in] height The new sub-image height.
 * \note Once this patch has been set, you must use the patch appropriate APIs.
 * \ingroup group_images
 * return Returns DVP_FALSE if the parameters are outside the allocated size of the image.
 */
DVP_BOOL DVP_Image_SetPatch(DVP_Image_t *pImage, DVP_U32 x_start, DVP_U32 y_start, DVP_U32 width, DVP_U32 height);

/*!
 * \brief Validates the values of the various members of the Image structure.
 * \param [in] pImage The pointer to the Image structure.
 * \param [in] bufAlign The pointer alignment value in bytes.
 * Typically 4 for most architectures, but can be 1 if the type is bytes.
 * \param [in] strideMultiple The minimum multiple of the stride in bytes.
 * Typically 1 byte.
 * \param [in] widthMultiple The minimum multiple of the width in pixels.
 * Typically 1 pixel, but can be 2, 4, 16, etc for SIMD architectures.
 * \param [in] heightMulitple The minimum multiple of the height in pixels.
 * Typically 1 line of pixels, but can be 2, 4, etc for SIMD architectures.
 * \param [in] colors The array of acceptable color formats. At least one must match.
 * \param [in] numColors The number of items in the colors array.
 * \retval DVP_TRUE all member variables are valid.
 * \retval DVP_FALSE some member variable is invalid.
 * \ingroup group_images
\code

    fourcc_t valid_colors[] = {FOURCC_Y800};
    DVP_Image_Validate(&image, 1, 1, 8, 1, valid_colors, dimof(valid_colors));

\endcode
 */
DVP_BOOL DVP_Image_Validate(DVP_Image_t *pImage,
                            DVP_U32 bufAlign,
                            DVP_U32 strideMultiple,
                            DVP_U32 widthMultiple,
                            DVP_U32 heigthMultiple,
                            fourcc_t *colors,
                            DVP_U32 numColors);

/*! \brief Determines if two images are bit equal regardless of the memory layout
 * of each image.
 * \param [in] pDst The image to determine if equal.
 * \param [in] pSrc The image to compare against.
 * \return DVP_BOOL
 * \ingroup group_images
 */
DVP_BOOL DVP_Image_Equal(DVP_Image_t *pDst, DVP_Image_t *pSrc);

/*! \brief Determines if two image patches are bit equal regardless of the memory layout
 * of each image.
 * \param [in] pDst The image patch to determine if equal.
 * \param [in] pSrc The image patch to compare against.
 * \return DVP_BOOL
 * \ingroup group_images
 */
DVP_BOOL DVP_Image_PatchEqual(DVP_Image_t *pDst, DVP_Image_t *pSrc);

/*!
 * \brief Allocates and maps a flat buffer to all remote cores.
 * \param [in] handle The handle to DVP.
 * \param [in] pBuffer The pointer to the buffer structure to fill in.
 * \param [in] dvpMemType The memory type requested.
 * \ingroup group_buffers
 */
DVP_BOOL DVP_Buffer_Alloc(DVP_Handle handle, DVP_Buffer_t *pBuffer, DVP_MemType_e dvpMemType);

/*!
 * \brief Unmaps and Frees the allocated buffer.
 * \param [in] handle The handle to DVP.
 * \param [in] pBuffer the pointer to the buffer structure to free.
 * \ingroup group_buffers
 */
DVP_BOOL DVP_Buffer_Free(DVP_Handle handle, DVP_Buffer_t *pBuffer);

/*!
 * \brief Performs the necessary operations to share this buffer with another process
 * \param [in] handle The handle to DVP.
 * \param [in] pBuffer The pointer to the buffer structure whose data will be shared.
 * \param [in] shared_fd the file descriptor that can be used to import this buffer in a different process
 * \ingroup group_buffers
 */
DVP_BOOL DVP_Buffer_Share(DVP_Handle handle, DVP_Buffer_t *pBuffer, DVP_S32 *shared_fd);

/*!
 * \brief Import the buffer into the calling process
 * \param [in] handle The handle to DVP.
 * \param [in] pBuffer The pointer to the buffer structure whose data will be imported.
 * \param [in] shared_fd the file descriptor obtained from DVP_Buffer_Share
 * \param [in] hdl internal buffer handle, needed to free the image through DVP_Buffer_Free_Import
 * \note This buffer can only be freed through DVP_Buffer_Free_Import
 * \ingroup group_buffers
 */
DVP_BOOL DVP_Buffer_Import(DVP_Handle handle, DVP_Buffer_t *pBuffer, DVP_S32 shared_fd, DVP_VALUE *hdl);

/*!
 * \brief Free an imported buffer
 * \param [in] handle The handle to DVP.
 * \param [in] pBuffer The pointer to the buffer structure whose data will be freed.
 * \param [in] hdl the internal buffer handle obtained during DVP_Buffer_Import
 * \note only imported buffers can be freed through this API
 * \ingroup group_buffers
 */
DVP_BOOL DVP_Buffer_Free_Import(DVP_Handle handle, DVP_Buffer_t *pBuffer, DVP_VALUE hdl);

/*!
 * \brief Initializes the buffer structure to the correct parameters.
 * \note Does not allocate any memory!
 * \param [in] pBuffer The pointer to the structure to be initialized.
 * \param [in] elemSize The size of an individual element.
 * \param [in] numElems The total number of bytes to be requested.
 * \ingroup group_buffers
 */
void DVP_Buffer_Init(DVP_Buffer_t *pBuffer, DVP_U32 elemSize, DVP_U32 numElems);

/*!
 * \brief Deinitializes the image structure.
 * \param [in] pBuffer the data structure to clear.
 * \ingroup group_buffers
 */
void DVP_Buffer_Deinit(DVP_Buffer_t *pBuffer);


/*!
 * \brief Validates the values of the various members of the Buffer structure.
 * \param [in] pImage The pointer to the Buffer structure.
 * \retval DVP_TRUE all member variables are valid.
 * \retval DVP_FALSE some member variable is invalid.
 * \ingroup group_buffers
 */
DVP_BOOL DVP_Buffer_Validate(DVP_Buffer_t *pBuffer);

/*!
 * \brief The function allocates the requested number of DVP_KernelNode_t structures in shared memory and maps them to all enabled remote cores.
 * \param [in] handle The handle to DVP.
 * \param [in] numNodes The desired number of contigiuous node structures.
 * \return DVP_KernelNode_t Returns a mapped array of Nodes.
 * \note The nodes will be allocated with \ref DVP_MTYPE_KERNELGRAPH.
 * \ingroup group_nodes
 */
DVP_KernelNode_t *DVP_KernelNode_Alloc(DVP_Handle handle, DVP_U32 numNodes);

/*!
 * \brief Unmaps and Frees the supplied node array for the number of nodes requested.
 * \param [in] handle The handle to DVP.
 * \param [in] pNodes The array of nodes.
 * \param [in] numNodes The number of nodes that pNodes points to.
 * \ingroup group_nodes
 */
void DVP_KernelNode_Free(DVP_Handle handle, DVP_KernelNode_t *pNodes, DVP_U32 numNodes);

#ifdef __cplusplus
}
#endif

#endif

