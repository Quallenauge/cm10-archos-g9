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

#include <dvp/dvp.h>
#include <dvp/dvp_debug.h>

#include <dvp_kgm.h>

DVP_U32 DVP_KernelGraphManagerLoad(DVP_GraphManager_t *pManager)
{
    pManager->handle = module_load(pManager->dlname);
    if (pManager->handle != NULL)
    {
        module_error();
        pManager->calls.init        = (DVP_GraphManagerInit_f)       module_symbol(pManager->handle, "DVP_KernelGraphManagerInit");
        pManager->calls.manager     = (DVP_GraphManager_f)           module_symbol(pManager->handle, "DVP_KernelGraphManager");
        pManager->calls.getKernels  = (DVP_GetSupportedKernels_f)    module_symbol(pManager->handle, "DVP_GetSupportedKernels");
        pManager->calls.getLocal    = (DVP_GetSupportedLocalCalls_f) module_symbol(pManager->handle, "DVP_GetSupportedLocalCalls");
        pManager->calls.getRemote   = (DVP_GetSupportedRemoteCalls_f)module_symbol(pManager->handle, "DVP_GetSupportedRemoteCalls");
        pManager->calls.getCore     = (DVP_GetSupportedRemoteCore_f) module_symbol(pManager->handle, "DVP_GetSupportedRemoteCore");
        pManager->calls.getLoad     = (DVP_GetMaximumLoad_f)         module_symbol(pManager->handle, "DVP_GetMaximumLoad");
        pManager->calls.deinit      = (DVP_GraphManagerDeinit_f)     module_symbol(pManager->handle, "DVP_KernelGraphManagerDeinit");
        pManager->calls.restart     = (DVP_GraphManagerRestart_f)    module_symbol(pManager->handle, "DVP_KernelGraphManagerRestart");
        pManager->calls.verify      = (DVP_KernelGraphManagerVerify_f)module_symbol(pManager->handle, "DVP_KernelGraphManagerVerify");
        if (pManager->calls.init == NULL ||
            pManager->calls.manager == NULL ||
            pManager->calls.verify == NULL ||
            pManager->calls.getKernels == NULL ||
            pManager->calls.getLocal == NULL ||
            pManager->calls.getRemote == NULL ||
            pManager->calls.getCore == NULL ||
            pManager->calls.getLoad == NULL ||
            pManager->calls.deinit == NULL) // we don't check restart yet
        {
            module_unload(pManager->handle);
            pManager->handle = NULL;
            memset(&pManager->calls, 0xFE, sizeof(DVP_GraphManager_Calls_t));
            DVP_PRINT(DVP_ZONE_ERROR, "ERROR: Failed to load library %s, error=%s\n", pManager->dlname, module_error());
            return 0;
        }
        else
            return 1;
    }
    else
    {
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR: Failed to load library %s, error=%s\n",pManager->dlname,module_error());
        return 0;
    }
}

DVP_U32 DVP_KernelGraphManagerUnload(DVP_GraphManager_t *pManager)
{
    if (pManager->handle != NULL)
    {
        memset(&pManager->calls, 0, sizeof(pManager->calls));
        module_unload(pManager->handle);
        pManager->handle = NULL;
        pManager->enabled = false_e;
        pManager->kernels = NULL;
        pManager->numSupportedKernels = 0;
    }
    return 1;
}
