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

#include <sosal/sosal.h>

unittest_t tests[] = {
    {"allocator",   allocator_unittest, false_e},
  // bitfield?
    {"cache",       cache_unittest,     false_e}, // needs explicit arguments
    {"events",      event_unittest,     true_e},
  // fourcc ?
  // fph ?
    {"hash",        hash_unittest,      true_e},
  //{"heap",        heap_unittest,      true_e},
    {"histogram",   histogram_unittest, true_e},
    {"ini",         ini_unittest,       true_e},
    {"list",        list_unittest,      true_e},
    {"module",      module_unittest,    false_e}, // needs explicit arguments
    // mutex ?
    {"options",     option_unittest,    true_e},
    // profiler ?
    {"queue",       queue_unittest,     true_e},
    // ring ?
    {"rpc",         rpc_unittest,       true_e},
    // semaphores ?
    // serial ?
    {"shared",      shared_unittest,    false_e}, // needs explicit arguments
    {"socket",      socket_unittest,    true_e},
    {"threads",     thread_unittest,    true_e},
    {"pool",        threadpool_unittest,true_e},
    {"uinput",      uinput_unittest,    false_e}, // needs explicit arguments
    {"btreelist",   btreelist_unittest, true_e},
    // vector ?
};
uint32_t numTests = dimof(tests);

int main(int argc, char *argv[])
{
    uint32_t i,j = 0;

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

    printf("SOSAL Unit Test!\n");
    if (argc == 1)
    {
        for (i = 0; i < numTests; i++)
        {
            if (tests[i].noArgs == true_e && tests[i].test(argc, argv) == true_e)
            {
                printf("[passed] unit test %s!\n", tests[i].name);
            }
            else if (tests[i].noArgs == false_e)
            {
                printf("[xxxxxx] unit test %s needs arguments to run, use by itself!\n", tests[i].name);
            }
            else
            {
                printf("[FAILED] unit test %s!\n", tests[i].name);
            }
        }
    }
    else
    {
        for (i = 0; i < (uint32_t)argc; i++)
        {
            for (j = 0; j < numTests; j++)
            {
                if (strcmp(tests[j].name, argv[i]) == 0)
                {
                    printf("Calling unit test for %s\n", tests[j].name);
                    if (tests[j].test(argc-i, &argv[i]) == true_e)
                    {
                        printf("[passed] unit test %s!\n", tests[j].name);
                    }
                    else
                    {
                        printf("[FAILED] unit test %s!\n", tests[j].name);
                    }
                }
            }
        }
    }
    return 0;
}
