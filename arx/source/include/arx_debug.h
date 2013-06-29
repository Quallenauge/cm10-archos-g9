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

#ifndef _ARX_DEBUG_H_
#define _ARX_DEBUG_H_

#ifdef ARX_DEBUG

#include <stdio.h>
#include <utils/Log.h>
#include <android/log.h>

#define ARX_PRINT(conditional, string, ...) if (conditional) { __android_log_print(ANDROID_LOG_DEBUG, #conditional, string, ## __VA_ARGS__); }

#ifndef ARX_ZONE_MASK
#define ARX_ZONE_MASK       (0x00000003) // This enables ERRORs and WARNINGs
#endif

#else //ARX_DEBUG

#ifndef ARX_ZONE_MASK
#define ARX_ZONE_MASK       (0x00000000)
#endif

#define ARX_PRINT(conditional, string, ...)  {}

#endif //ARX_DEBUG

#define ARX_BIT(x)          (1 << (x))
#define ARX_ZONE_ERROR      (ARX_BIT(0) & ARX_ZONE_MASK) /**< Intended for error cases in all the code */
#define ARX_ZONE_WARNING    (ARX_BIT(1) & ARX_ZONE_MASK) /**< Intended for warning in any code */
#define ARX_ZONE_API        (ARX_BIT(2) & ARX_ZONE_MASK) /**< Intended for API tracing in any code */
#define ARX_ZONE_DAEMON     (ARX_BIT(3) & ARX_ZONE_MASK) /**< Intended for informational purposes in Daemon code only. */
#define ARX_ZONE_ENGINE     (ARX_BIT(4) & ARX_ZONE_MASK) /**< Intended for informational purposes in Engine code only. */
#define ARX_ZONE_PERF       (ARX_BIT(5) & ARX_ZONE_MASK) /**< Intended for performance metrics output only */
#define ARX_ZONE_CLIENT     (ARX_BIT(6) & ARX_ZONE_MASK) /**< Intended for informational purposes in Client code only. */
#define ARX_ZONE_BUFFER     (ARX_BIT(7) & ARX_ZONE_MASK) /**< Intended for informational purposes in buffer code only. */
#define ARX_ZONE_TEST       (ARX_BIT(8) & ARX_ZONE_MASK) /**< Intended for informational purposes in testing code only. */
#define ARX_ZONE_JNI        (ARX_BIT(9) & ARX_ZONE_MASK) /**< Intended for informational purposes in JNI code only. */
#define ARX_ZONE_ALWAYS     (1)

#endif
