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

/*! \file
 * \brief The DVP Kernel Graph Manager Dynamic Loader.
 * \defgroup group_dvp_kmdl DVP Kernel Graph Manager Dynamic Loader API.
 */

#ifndef _DVP_KGMDL_H_
#define _DVP_KGMDL_H_

#include <dvp_kgm.h>

/*! \brief This function loads a manager from the information inside the
 * manager structure.
 * \param [in] pManager The pointer the manager data structure.
 * \ingroup group_dvp_kmdl
 */
DVP_U32 DVP_KernelGraphManagerLoad(DVP_GraphManager_t   *pManager);

/*! \brief This function unloads a manager from the information inside the
 * manager structure.
 * \param [in] pManager The pointer the manager data structure.
 * \ingroup group_dvp_kmdl
 */
DVP_U32 DVP_KernelGraphManagerUnload(DVP_GraphManager_t *pManager);

#endif
