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

#ifndef _DVP_DISPLAY_H_
#define _DVP_DISPLAY_H_

/*!
 * \file dvp_display.h
 * \brief Contains the display code for previewing DVP processed images.
 * \author Erik Rainey
 */

#include <dvp/dvp_types.h>
#include <sosal/fourcc.h>

/*! The default number of display buffers
 * \ingroup group_display
 */
#define DVP_DISPLAY_NUM_BUFFERS (5)

// This may be platform without a display
#if !defined(SCREEN_DIM_X) && !defined(SCREEN_DIM_Y)

/*! The default display width.
 * \ingroup group_display
 */
#define DVP_DISPLAY_WIDTH       (1024)

/*! The default display height.
 * \ingroup group_display
 */
#define DVP_DISPLAY_HEIGHT      (768)

#else

/*! The default display width.
 * \ingroup group_display
 */
#define DVP_DISPLAY_WIDTH       (SCREEN_DIM_X)

/*! The default display height.
 * \ingroup group_display
 */
#define DVP_DISPLAY_HEIGHT      (SCREEN_DIM_Y)

#endif

#ifndef _DVP_DISPLAY_T
#define _DVP_DISPLAY_T
typedef uint32_t dvp_display_t; /*!< This type is internally defined to avoid cross-os dependencies. \ingroup group_display */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Creates a display which is internally HLOS technology dependant.
 * \param [in] image_width The width of the raw image
 * \param [in] image_height The height of the raw image
 * \param [in] buffer_width The width of the allocated buffer holding the image.
 * \param [in] buffer_height The height of the allocated buffer holding the image.
 * \param [in] display_width The width of the actual display
 * \param [in] display_height The height of the actual display
 * \param [in] scaled_width The desired rendered width.
 * \param [in] scaled_height The desired rendered height
 * \param [in] crop_top The pixel position of the top of the image.
 * \param [in] crop_left The pixel position of the left of the image.
 * \param [in] color The FOURCC color space to use.
 * \param [in] rotation The rotation of the display (0,90,180,270).
 * \param [in] numBuffers The number of desired buffers to internally allocate.
 * \ingroup group_display
 */
dvp_display_t *DVP_Display_Create(uint32_t image_width, uint32_t image_height,
                                  uint32_t buffer_width, uint32_t buffer_height,
                                  uint32_t display_width, uint32_t display_height,
                                  uint32_t scaled_width, uint32_t scaled_height,
                                  uint32_t crop_top,  uint32_t crop_left,
                                  fourcc_t color, int32_t rotation, uint32_t numBuffers);

/*!
 * \brief Destroys the DVP Display.
 * \param [in,out] dvpd The pointer to the pointer to the display structure. Will return as NULL.
 * \pre \ref DVP_Display_Create
 * \ingroup group_display
 */
void DVP_Display_Destroy(dvp_display_t **dvpd);

/*!
 * \brief Queues the image to be rendered by the underlying display technology.
 * \param [in] dvpd The pointer to the DVP Display Structure.
 * \param [in] pImage The pointer to the image to render. Must have been allocated by the \see DVP_Display_Alloc.
 * \pre \ref DVP_Display_Create
 * \ingroup group_display
 */
void DVP_Display_Render(dvp_display_t *dvpd, DVP_Image_t *pImage);

/*!
 * \brief Internally copies the attributes of an allocated buffer to the pImage structure.
 * \param [in] dvpd The pointer to the DVP Display Structure.
 * \param [in] pImage The pointer to the image structure to be filled in.
 * \note This can only be called equal to the number of allcoated buffers in \see DVP_Display_Create.
 * \note This memory is not yet mapped to all remote cores and must be passed to DVP_Image_Alloc with the memory types set to the type corresponding to display buffers.
 * \pre \ref DVP_Display_Create
 * \ingroup group_display
 */
DVP_BOOL DVP_Display_Alloc(dvp_display_t *dvpd, DVP_Image_t *pImage);

/*!
 * \brief Internally returns the image to the display pool.
 * \param [in] dvpd The pointer to the DVP Display structure.
 * \param [in] pImage The pointer to the image to be freed. The structure will be zeroed.
 * \pre \ref DVP_Display_Alloc
 * \ingroup group_display
 */
DVP_BOOL DVP_Display_Free(dvp_display_t *dvpd, DVP_Image_t *pImage);

/*!
 * \brief Internally this causes this frame to be marked as not-rendered
 * (if that is needed by the underlying tech).
 * \param [in] dvpd The pointer to the DVP Display Structure.
 * \param [in] pImage The image to drop.
 * \pre \ref DVP_Display_Create
 * \ingroup group_display
 */
DVP_BOOL DVP_Display_Drop(dvp_display_t *dvpd, DVP_Image_t *pImage);

#ifdef __cplusplus
}
#endif

#endif

