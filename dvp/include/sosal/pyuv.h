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

#ifndef _PYUV_H_
#define _PYUV_H_

/*! \file
 * \brief The SOSAL PYUV interfacing module for naming files.
 * \author Erik Rainey <erik.rainey@ti.com>
 * \see http://dsplab.diei.unipg.it/pyuv_raw_video_sequence_player_original_one
 */

#include <sosal/sosal.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Gets the extension which the supplied FOURCC code would use.
 * \param [in] fourcc The \ref _fourcc code.
 * \return Returns a pointer to an extension.
 * \ingroup group_pyuv
 */
char *PYUV_GetFileExtension(fourcc_t fourcc);

/*! \brief Gets the name of the color space from the FOURCC code.
 * \param [in] fourcc The \ref _fourcc code.
 * \return Returns a pointer to a color space name.
 * \ingroup group_pyuv
 */
char *PYUV_GetColorSpaceName(fourcc_t fourcc);

/*! \brief Gets the FOURCC code which co-relates to the name and ext provided.
 * \param [in] name The name of the color space.
 * \param [in] ext The extension of the file.
 * \return Returns the FOURCC code.
 * \ingroup group_pyuv
 */
fourcc_t PYUV_GetColorSpace(char *name, char *ext);

/*! \brief Constructs a filename based on the parameters supplied to the
 * function which allows the pYUV player to load the RAW file appropriately.
 * \param [out] filename The final filename.
 * \param [in] path The path to place the file in.
 * \param [in] name The short name of the file.
 * \param [in] width The width in pixels.
 * \param [in] height The height in pixels.
 * \param [in] fps The frame rate of the RAW sequence.
 * \param [in] fourcc The FOURCC format of the data.
 * \ingroup group_pyuv
 */
void PYUV_GetFilename(char *filename,
                      char *path,
                      char *name,
                      unsigned int width,
                      unsigned int height,
                      unsigned int fps,
                      fourcc_t fourcc);

/*! \brief Decodes a pYUV compatible filename into attributes.
 * \param [in] filename
 * \param [out] width The pointer to the width.
 * \param [out] height The pointer to the height.
 * \param [out] fps The pointer to the fps.
 * \param [out] color The pointer to the fourcc_t.
 * \ingroup group_pyuv
 */
void PYUV_DecodeFilename(char *filename,
                         uint32_t *width,
                         uint32_t *height,
                         uint32_t *fps,
                         fourcc_t *color);
#ifdef __cplusplus
}
#endif

#endif

