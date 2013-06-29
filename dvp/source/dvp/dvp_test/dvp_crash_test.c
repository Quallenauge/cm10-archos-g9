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

#include <sys/wait.h>

#include <dvp/dvp.h>
#include <dvp/dvp_debug.h>

static int dvp_process(bool_e wait)
{
    DVP_PRINT(DVP_ZONE_ALWAYS, "Initializing DVP...\n");
    DVP_Handle handle = DVP_KernelGraph_Init();
    if (!handle) {
        DVP_PRINT(DVP_ZONE_ERROR, "Failed initializing DVP!\n");
        return 1;
    }
    if (wait == true_e) {
        DVP_PRINT(DVP_ZONE_ALWAYS, "Waiting to be killed...\n");
        sleep(1000);
    }
    DVP_KernelGraph_Deinit(handle);
    DVP_PRINT(DVP_ZONE_ALWAYS, "Tore down DVP!\n");
    return 0;
}

static int wait_for_dvp(pid_t pid, bool_e killdvp)
{
    int status = -1;
    int count = 10;
    if (killdvp == true_e) {
        DVP_PRINT(DVP_ZONE_ALWAYS, "Wait for DVP to initialize...\n");
        //give dvp proccess a chance to initialize
        sleep(2);
        DVP_PRINT(DVP_ZONE_ALWAYS, "killing process %d...\n", pid);
        if(kill(pid, SIGKILL) != 0) {
            DVP_PRINT(DVP_ZONE_ERROR, "Error trying to kill process %d\n", pid);
        }
    }
    DVP_PRINT(DVP_ZONE_ALWAYS, "Waiting for DVP process(%d) to exit...\n", pid);
    do {
        pid_t wpid = waitpid(pid, &status, WNOHANG);
        if (wpid == -1) {
            DVP_PRINT(DVP_ZONE_ERROR, "error waiting for pid(%d)\n", status);
            break;
        }
        if (wpid == 0) {
            if (--count > 0) {
                sleep(1);
                DVP_PRINT(DVP_ZONE_ALWAYS, "Waiting for DVP process(%d) to exit...\n", pid);
                continue;
            } else {
                DVP_PRINT(DVP_ZONE_ALWAYS, "Timed out waiting for DVP process (%d)\n", pid);
                return -1;
            }
        }
        if (WIFEXITED(status)) {
            DVP_PRINT(DVP_ZONE_ALWAYS, "DVP process exited, status=%d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            DVP_PRINT(DVP_ZONE_ALWAYS, "DVP process exited due to signal %d)\n", WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            DVP_PRINT(DVP_ZONE_ALWAYS, "DVP process stopped (signal %d)\n", WSTOPSIG(status));
        }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    return status;
}

int main()
{
    int exitCode = 0;
    pid_t pid;

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

    pid = fork();
    if (pid == 0) {
        exitCode = dvp_process(true_e);
    } else {
        //kill it to test if dvp can recover on a second try
        wait_for_dvp(pid, true_e);
        pid = fork();
        if (pid == 0) {
            //instantiate dvp again, but let it teardown normally
            exitCode = dvp_process(false_e);
        } else {
            int status = wait_for_dvp(pid, false_e);
            if (!WIFEXITED(status) || (WIFEXITED(status) && status != 0)) {
                DVP_PRINT(DVP_ZONE_ERROR, "Failed DVP teardown test! (%d)\n", status);
            } else {
                DVP_PRINT(DVP_ZONE_ALWAYS, "Passed DVP teardown test! (%d)\n", status);
            }
        }
    }
    return exitCode;
}
