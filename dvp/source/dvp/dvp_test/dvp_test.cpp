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

#include <TestVisionEngine.h>

int main(int argc, char *argv[])
{
#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

    if (argc > 1)
    {
        char name[MAX_PATH];

        strncpy(name, argv[1], MAX_PATH);
        uint32_t width  = (argc > 2 ? atoi(argv[2]) : 160);
        uint32_t height = (argc > 3 ? atoi(argv[3]) : 120);
        uint32_t fps    = (argc > 4 ? atoi(argv[4]) : 15);
        fourcc_t color    = (argc > 5 ? FOURCC_STRING(argv[5]) : (fourcc_t)FOURCC_UYVY);
        uint32_t numFrames = (argc > 6 ? atoi(argv[6]) : 100);
        uint32_t graph  = (argc > 7 ? atoi(argv[7]) : 0);
        DVP_Core_e  affinity = DVP_CORE_MIN;

        if(argc > 8)
        {
            if (strcmp(argv[8], "cpu") == 0)            affinity = DVP_CORE_CPU;
            else if (strcmp(argv[8], "dsp") == 0)       affinity = DVP_CORE_DSP;
            else if (strcmp(argv[8], "simcop") == 0)    affinity = DVP_CORE_SIMCOP;
            else if (strcmp(argv[8], "gpu") == 0)       affinity = DVP_CORE_GPU;
        }

        TestVisionEngine engine(width, height, fps, color, name, numFrames, graph, affinity);
        engine.Startup();
        if (engine.WaitForCompletion() == false)
            engine.Shutdown();
    }
    else
    {
        printf("$ %s <shortname> <width> <height> <fps> <FOURCC> <numFrames> <graph#> [cpu|dsp|simcop|gpu]\n", argv[0]);
        DVP_PRINT(DVP_ZONE_ALWAYS, "$ %s <shortname> <width> <height> <fps> <FOURCC> <numFrames> <graph#> [cpu|dsp|simcop|gpu]\n", argv[0]);
    }
    return 0;
}
