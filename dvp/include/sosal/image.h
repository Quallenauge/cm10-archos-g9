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

#ifndef _SOSAL_IMAGE_H_
#define _SOSAL_IMAGE_H_

/*! \file
 * \brief The SOSAL Image API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */ 

#include <sosal/types.h>
#include <sosal/fourcc.h>

/*! \brief The \ref image_t plane definition. 
 * \ingroup group_image_t
 */
typedef struct _plane_t {
    uint8_t *ptr;     /**< Plane Data Pointer */
    uint32_t xdim;    /**< Plane x dimension in pixels */
    uint32_t ydim;    /**< Plane y dimension in pixels */
    uint32_t xscale;  /**< Plane x scale */
    uint32_t yscale;  /**< Plane y scale */
     int32_t xstride; /**< Plane x stride between pixels in bytes */
     int32_t ystride; /**< Plane y stride between pixels in bytes */
     int32_t xstep;   /**< Plane x step between pixels, if macro pixel, then > 1 */
     uint32_t numBytes;/**< Number of bytes in the plane */
} plane_t;

/*! \brief The definition of a multiplanar, multidimensional image.
 * \ingroup group_image_t
 */
typedef struct _image_t {
    fourcc_t color;     /*!< \brief Color space. \see fourcc_t */
    plane_t  plane[4];  /*!< \brief Plane information */
    uint32_t numPlanes; /*!< \brief This indicates the active number of planes */
} image_t;

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief This computes the pixel pointer at the given co-ordinates.
 * \param [in] img The image pointer
 * \param [in] p The plane index
 * \param [in] x The x dimension
 * \param [in] y The y dimension
 * \pre \ref image_allocate
 * \pre \ref image_back
 * \ingroup group_image_t
 */
#define  image_pixel(img, p, x, y) &img->plane[p].ptr[(img->plane[p].ystride*(y/img->plane[p].yscale))+(img->plane[p].xstride*(x/img->plane[p].xscale))]

/*! \brief This function is used to determine if an image_t can be made of a particular format.
 * \param [in] color The desired format.
 * \ingroup group_image_t
 */
bool_e   image_query_format(fourcc_t color);

/*! \brief Frees an image and destroys the pointer.
 * \param [in,out] pimg The pointer to the image_t pointer. This will be NULLed by the function.
 * \pre \ref image_unback
 * \ingroup group_image_t
 */
void     image_free(image_t **pimg);

/*! \brief Allocates an image meta-data structure with the given attributes.
 * \param [in] width Width in pixels.
 * \param [in] height Height in pixels.
 * \param [in] color The FOURCC color code.
 * \pre \ref image_query_format To determine if the format is possible.
 * \post \ref image_back
 * \ingroup group_image_t
 * \retval NULL The allocation failed.
 */
image_t *image_allocate(uint32_t width, uint32_t height, fourcc_t color);

/*! \brief This function will fill the image with the data in a, b, or c based
 * on the format of the image.
 * \param [in] img The pointer to the image.
 * \param [in] a Most formats will use this field. FOURCC_Y800 will only use this field.
 * \param [in] b Most formats will use this field.
 * \param [in] c Most formats will use this field.
 * \pre \ref image_back
 * \ingroup group_image_t
 */
void     image_fill(image_t *img, uint8_t a, uint8_t b, uint8_t c);

/*! \brief This functions copies an image from source to destintation.
 * \note If a conversion is required, some conversion will occur. See \iref image_convert for details.
 * \param [out] dst Destination image. Must be pre-allocated.
 * \param [in] src Source image.
 * \pre \ref image_back
 * \ingroup group_image_t
 */
uint32_t image_copy(image_t *dst, image_t *src);

/*! \brief This function will convert one image format into another if it supports this.
 * \param [out] dst The Destination image, must be pre-allocated.
 * \param [in] src The source image.
 * \pre \ref image_back
 * \ingroup group_image_t
 */
void     image_convert(image_t *dst, image_t *src);

/*! \brief This function prints the meta-data of the image.
 * \param [in] img The image to print.
 * \pre \ref image_back
 * \ingroup group_image_t
 */
void     image_print(image_t *img);

/*! \brief This function "backs" an an image meta-data structure with actual memory.
 * \param [in] img The image to "back".
 * \pre \ref image_allocate
 * \post \ref image_unback
 * \ingroup group_image_t
 */
bool_e   image_back(image_t *img);

/*! \brief This function removes the memory "backing" the image meta-data structure.
 * \param [in] img The image to "unback".
 * \pre \ref image_back
 * \post \ref image_free
 * \ingroup group_image_t
 */
void     image_unback(image_t *img);

#ifdef __cplusplus
}
#endif

#endif
