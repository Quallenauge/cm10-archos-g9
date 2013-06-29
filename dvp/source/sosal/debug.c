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

#include <sosal/types.h>
#include <sosal/debug.h>

/*! \brief The runtime variable which controls which debugging zones are
 * enabled.
 * \ingroup group_sosal_debug
 */
uint32_t sosal_zone_mask = 0x3;

#if defined(SOSAL_RUNTIME_DEBUG)
#pragma message("SOSAL Debugging in RUNTIME Mode! export SOSAL_ZONE_MASK to affect.")
#endif

#if defined(POSIX)
void debug_get_zone_mask(char *mask_name, uint32_t *mask)
{
    char *value = getenv(mask_name);
    if (value)
    {
        sscanf(value, "%x", mask);
    }
}
#elif defined(WIN32)
void debug_get_zone_mask(char *mask_name, uint32_t *mask)
{
    char value[MAX_PATH];
    if (GetEnvironmentVariable(mask_name, value, MAX_PATH) > 0)
    {
        sscanf(value, "%x", mask);
    }
}
#endif
