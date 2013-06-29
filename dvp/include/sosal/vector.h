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

#ifndef _SOSAL_VECTOR_H_
#define _SOSAL_VECTOR_H_

/*! \file
 * \brief A Set of simple vector functions.
 */

#include <sosal/types.h>

/*! \brief The storage for a 4D vector.
 * \ingroup group_vectors
 */
typedef union _vector_t {
	/*! \brief 4D point representation */
    struct {
        float x; /*!< \brief x dimension */
        float y; /*!< \brief y dimension */
        float z; /*!< \brief z dimension */
        float t; /*!< \brief time dimension */
    } p;
    /*! \brief 4D dimensional representation */
    struct {
        float width;  /*!< \brief pixel width */
        float height; /*!< \brief pixel height */
        float depth;  /*!< \brief pixel depth */
        float age;    /*!< \brief time */
    } dim;
    float v[4]; /*!< \brief plain array of floats */
} vector_t;

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief a = a + b.
 * \param [in,out] a Source and destination vector.
 * \param [in] b Additive Vector.
 * \ingroup group_vectors
 */
void vector_add(vector_t *a, vector_t *b);

/*! \brief Multiplies each element in a by b and puts it back in a.
 * \param [in,out] a Source and destination vector.
 * \param [in] b Scalar value
 * \group group_vectors
 */
void vector_scale(vector_t *a, float b);

/*! \brief a = 1/a.
 * \param [in,out] a Source and destination vector.
 * \ingroup group_vectors
 */
void vector_inverse(vector_t *a);

/*! \brief Computes the square root of the sum of squares of the components.
 * \param [in] v The vector to calculate a magnitude from.
 * \ingroup group_vectors
 */
float vector_mag(vector_t *v);

#ifdef __cplusplus
}
#endif

#endif

