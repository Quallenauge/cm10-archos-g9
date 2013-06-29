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

#ifndef _SOSAL_HISTOGRAM_H_
#define _SOSAL_HISTOGRAM_H_

/*! \file
 * \brief The Histogram API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */
 
#include <sosal/types.h>

/*! \brief A histogram with a fix number of bins.
 * \ingroup group_histograms
 */ 
typedef struct _histogram_t {
    uint32_t *bins;		/*!< \brief The array of bins */
    uint32_t numBins;	/*!< \brief The number of bins */
     int32_t binRange;	/*!< \brief The range of each bin. */
     int32_t min;		/*!< \brief The minimum value of the histogram */
     int32_t max;		/*!< \brief The maximum value of the histogram */
} histogram_t;

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief This initializes the histogram to a set number of bins and a min/max.
 * \param [in,out] histogram The histogram meta data. 
 * \param [in] bins The number of bins needed for the histogram.  
 * \param [in] min The minimum range the histogram covers.
 * \param [in] max The maximum range the histogram covers.
 * \ingroup group_histograms
 */
bool_e histogram_init(histogram_t *histogram, uint32_t bins, int32_t min, int32_t max);

/*! \brief This de-initializes the histogram.
 * \param [in] histogram The pointer to the histogram meta-data.
 * \ingroup group_histograms
 */
void histogram_deinit(histogram_t *histogram);

/*! \brief This increments the histogram bin which this value classifies into.
 * \param [in] histogram The pointer to the histogram meta-data.
 * \param [in] value The value to classify. 
 * \ingroup group_histograms
 */
void histogram_inc(histogram_t *histogram, int32_t value);

/*! \brief This gets the bin count in which the value would be added.
 * \param [in] histogram The pointer to the histogram meta-data.
 * \param [in] value The value to use to get the bin count.
 * \ingroup group_histograms
 */ 
uint32_t histogram_get(histogram_t *histogram, int32_t value);

#ifdef __cplusplus
}
#endif

#endif

