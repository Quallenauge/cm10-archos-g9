/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VTC_TEST_COMMON_H
#define VTC_TEST_COMMON_H

#include <cutils/log.h>




// logging macros
#define VTC_LOGD(f, ...) LOG_PRI(ANDROID_LOG_DEBUG,   LOG_TAG, f, ##__VA_ARGS__)
#define VTC_LOGV(f, ...) LOG_PRI(ANDROID_LOG_VERBOSE, LOG_TAG, f, ##__VA_ARGS__)
#define VTC_LOGI(f, ...) LOG_PRI(ANDROID_LOG_INFO,    LOG_TAG, f, ##__VA_ARGS__)
#define VTC_LOGW(f, ...) LOG_PRI(ANDROID_LOG_WARN,    LOG_TAG, f, ##__VA_ARGS__)
#define VTC_LOGE(f, ...) LOG_PRI(ANDROID_LOG_ERROR,   LOG_TAG, f, ##__VA_ARGS__)


// function macros
#ifdef LOG_FUNCTION_NAME_ENTRY
#   undef LOG_FUNCTION_NAME_ENTRY
#endif

#ifdef LOG_FUNCTION_NAME_EXIT
#   undef LOG_FUNCTION_NAME_EXIT
#endif

#define LOG_FUNCTION_NAME_ENTRY VTC_LOGV("\n ENTER %s \n", __FUNCTION__);
#define LOG_FUNCTION_NAME_EXIT  VTC_LOGV("\n EXIT %s \n", __FUNCTION__);




#endif // VTC_TEST_COMMON_H
