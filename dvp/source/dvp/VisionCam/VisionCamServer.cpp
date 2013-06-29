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

#include <stdio.h>
#if defined(LINUX)
#include <signal.h>
#endif
#include <ServerVisionCam.h>

static ServerVisionCam *f_pSVCam;

#if defined(LINUX)
static void signal_handler(int sig)
{
    signal(sig, NULL);
    if (sig == SIGTERM ||
        sig == SIGINT)
        f_pSVCam->SignalCompletion();
    signal(sig, signal_handler);
}
#endif

int main(int argc, char *argv[])
{
    VisionCam_e camType = VISIONCAM_OMX;
    char ipaddr[MAX_PATH];
    strcpy(ipaddr, "127.0.0.1");

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

#if defined(LINUX)
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
#endif

    if (argc > 1)
        strncpy(ipaddr, argv[1], MAX_PATH);
    if (argc > 2)
        camType = (VisionCam_e)atoi(argv[2]);

    DVP_PRINT(DVP_ZONE_CAM, "Serving on %s VisionCam Type %u\n", ipaddr, camType);
    f_pSVCam = ServerVisionCam::instantiate(ipaddr, camType);
    if (f_pSVCam)
    {
        f_pSVCam->WaitForCompletion();
        delete f_pSVCam;
        return 0;
    }
#if defined(LINUX)
    else
        return ENAVAIL;
#else
    else
        return -1;
#endif
}
