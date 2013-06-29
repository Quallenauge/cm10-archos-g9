/**
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

#if defined(ANDROID) || defined(LINUX) || defined(__QNX__) || defined(CYGWIN) || defined(DARWIN)
#if !defined(__QNX__) && !defined(DARWIN)
#include <features.h>
#else
#define __EXT_UNIX_MISC //Needed by QNX version of dirent.h to include scandir()
#endif
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>
#endif

#include <dvp/dvp.h>
#include <dvp/dvp_debug.h>

#include <dvp_kgm.h>
#include <dvp_kgb.h>
#include <dvp_kmdl.h>

#define DVP_OPTIMIZED_GRAPHS
#if defined(ANDROID) || defined(__QNX__) || defined(LINUX)
#define DVP_USE_LOAD_TABLE
#endif

#if defined(ANDROID)
#define DVP_PROPERTY_KEY        "log.ti.dvp.load_table_users"
#define DVP_PROPERTY_VAL_FMT    "count=%d"
#define DVP_PROPERTY_VAL_DFT    "count=0"
#include <cutils/properties.h>
#endif

#if defined(ANDROID)
/** The path to the Kernel Graph Manager Libraries */
#define DVP_KGM_PATH    "/system/lib/"
#elif defined(LINUX) || defined(CYGWIN) || defined(__QNX__)
/** The path to the Kernel Graph Manager Libraries */
#define DVP_KGM_PATH    "/usr/lib/"
#elif defined(DARWIN)
/** The path to the Kernel Graph Manager Libraries */
#define DVP_KGM_PATH    "/opt/local/lib/"
#else // WIN32 or SYSBIOS
/** The path to the Kernel Graph Manager Libraries */
#define DVP_KGM_PATH    ""
#endif

/*!
 * This structure maps the core to it's predefined priority.
 */
typedef struct _dvp_core_priority_t {
    DVP_Core_e core;
    DVP_U32    priority;
} DVP_Core_Priority_t;

#if defined(TARGET_DVP_OMAP4)
/** This defines the list of priorities for the predefined cores */
DVP_Core_Priority_t f_corePriorities[DVP_CORE_MAX] = {
    {DVP_CORE_DSP,  DVP_PRIORITY_HIGHEST+DVP_PRIORITY_INC},
    {DVP_CORE_SIMCOP,  DVP_PRIORITY_HIGHEST},
    {DVP_CORE_MCU0, DVP_PRIORITY_NONE},
    {DVP_CORE_MCU1, DVP_PRIORITY_NONE},
    {DVP_CORE_EVE,  DVP_PRIORITY_NONE},
    {DVP_CORE_GPU,  DVP_PRIORITY_NONE},
    {DVP_CORE_CPU,  DVP_PRIORITY_LOWEST},
};
#elif defined(TARGET_DVP_OMAP5)
/** This defines the list of priorities for the predefined cores */
DVP_Core_Priority_t f_corePriorities[DVP_CORE_MAX] = {
    {DVP_CORE_DSP,  DVP_PRIORITY_HIGHEST+DVP_PRIORITY_INC},
    {DVP_CORE_SIMCOP,  DVP_PRIORITY_HIGHEST},
    {DVP_CORE_MCU0, DVP_PRIORITY_NONE},
    {DVP_CORE_MCU1, DVP_PRIORITY_NONE},
    {DVP_CORE_EVE,  DVP_PRIORITY_NONE},
    {DVP_CORE_GPU,  DVP_PRIORITY_NONE},
    {DVP_CORE_CPU,  DVP_PRIORITY_LOWEST},
};
#elif defined(TARGET_DVP_OMAP6)
/** This defines the list of priorities for the predefined cores */
DVP_Core_Priority_t f_corePriorities[DVP_CORE_MAX] = {
    {DVP_CORE_DSP,  DVP_PRIORITY_HIGHEST+(2*DVP_PRIORITY_INC)},
    {DVP_CORE_SIMCOP,  DVP_PRIORITY_HIGHEST+(1*DVP_PRIORITY_INC)},
    {DVP_CORE_MCU0, DVP_PRIORITY_NONE},
    {DVP_CORE_MCU1, DVP_PRIORITY_NONE},
    {DVP_CORE_EVE,  DVP_PRIORITY_HIGHEST},
    {DVP_CORE_GPU,  DVP_PRIORITY_NONE},
    {DVP_CORE_CPU,  DVP_PRIORITY_LOWEST},
};
#else
/** This defines the list of priorities for the predefined cores */
DVP_Core_Priority_t f_corePriorities[DVP_CORE_MAX] = {
    {DVP_CORE_DSP,  DVP_PRIORITY_NONE},
    {DVP_CORE_SIMCOP,  DVP_PRIORITY_NONE},
    {DVP_CORE_MCU0, DVP_PRIORITY_NONE},
    {DVP_CORE_MCU1, DVP_PRIORITY_NONE},
    {DVP_CORE_EVE,  DVP_PRIORITY_NONE},
    {DVP_CORE_GPU,  DVP_PRIORITY_HIGHEST},
    {DVP_CORE_CPU,  DVP_PRIORITY_LOWEST},
};

#endif

#if !defined(DVP_STATIC_MANAGERS)
static int DVP_FilenameSelector(const struct dirent *de)
{
    //DVP_PRINT(DVP_ZONE_KGB, "Checking for %s against %s\n", MODULE_NAME("dvp_kgm_*"), de->d_name);
    if (de && 0 == fnmatch(MODULE_NAME("dvp_kgm_*"), de->d_name,
#if defined(__QNX__) || defined(DARWIN)
    FNM_PERIOD|FNM_PATHNAME))
#else
    FNM_PERIOD|FNM_FILE_NAME))
#endif
        return 1;
    else
        return 0;
}

#if defined(__QNX__)
typedef int (*sorting_f)(const void *, const void *);
#else
typedef int (*sorting_f)(const struct dirent **, const struct dirent **);
#endif

static int dirent_sort(const struct dirent **a, const struct dirent **b)
{
    //DVP_PRINT(DVP_ZONE_KGB, "Compare %s to %s\n", (*a)->d_name, (*b)->d_name);
    return strcmp((*a)->d_name, (*b)->d_name);
}

/** A dynamic loader for DVP KGM's. */
static DVP_BOOL DVP_FindManagers(DVP_t *dvp)
{
    DVP_U32 i = 0;
    struct dirent **names = NULL;
    mutex_init(&dvp->mgrLock);
    dvp->numMgrs = scandir(DVP_KGM_PATH, &names, DVP_FilenameSelector,(sorting_f)dirent_sort);
    DVP_PRINT(DVP_ZONE_KGB, "Found %d DVP Kernel Graph Managers in the system library path %s\n", dvp->numMgrs, DVP_KGM_PATH);
    dvp->managers = (DVP_GraphManager_t *)calloc(dvp->numMgrs, sizeof(DVP_GraphManager_t));
    if (dvp->managers == NULL)
    {
        // ERROR
        for (i = 0; i < dvp->numMgrs; i++)
            free(names[i]);
        if (names)
            free(names);
        return DVP_FALSE;
    }

    for (i = 0; i < dvp->numMgrs; i++)
    {
        // Fill in the variables... pull the name of the KGM type out of the filename
        sscanf(names[i]->d_name, MODULE_NAME("dvp_kgm_%[^.]"), dvp->managers[i].name);
        strncpy(dvp->managers[i].dlname, names[i]->d_name, MAX_PATH);
        DVP_PRINT(DVP_ZONE_KGB, "Found DVP KGM file %s (%s)\n", dvp->managers[i].dlname, dvp->managers[i].name);
        free(names[i]);
        names[i] = NULL;
    }
    free(names);
    return DVP_TRUE;
}
#else

#define DVP_KGM_CPU_NAME    (DVP_KGM_PATH MODULE_NAME("dvp_kgm_cpu"))
#define DVP_KGM_SIMCOP_NAME (DVP_KGM_PATH MODULE_NAME("dvp_kgm_simcop"))
#define DVP_KGM_DSP_NAME    (DVP_KGM_PATH MODULE_NAME("dvp_kgm_dsp"))
#define DVP_KGM_OCL_NAME    (DVP_KGM_PATH MODULE_NAME("dvp_kgm_ocl"))

/** The non-dynamic version of the loader */
static DVP_BOOL DVP_FindManagers(DVP_t *dvp)
{
    DVP_U32 i = 0;
#if defined(TARGET_DVP_OMAP4)
    dvp->numMgrs = 3;
#else
    dvp->numMgrs = 2;
#endif
    dvp->managers = (DVP_GraphManager_t *)calloc(dvp->numMgrs, sizeof(DVP_GraphManager_t));
    if (dvp->managers != NULL)
    {
#if defined(TARGET_DVP_OMAP4)
        strncpy(dvp->managers[0].dlname, DVP_KGM_SIMCOP_NAME, MAX_PATH);
        strncpy(dvp->managers[1].dlname, DVP_KGM_DSP_NAME, MAX_PATH);
        strncpy(dvp->managers[2].dlname, DVP_KGM_CPU_NAME, MAX_PATH);
#else
        strncpy(dvp->managers[0].dlname, DVP_KGM_CPU_NAME, MAX_PATH);
        strncpy(dvp->managers[1].dlname, DVP_KGM_OCL_NAME, MAX_PATH);
#endif

        for (i = 0; i < dvp->numMgrs; i++)
        {
            // Fill in the variables... pull the name of the KGM type out of the filename
            sscanf(dvp->managers[i].dlname, DVP_KGM_PATH MODULE_NAME("dvp_kgm_%[^.]"), dvp->managers[i].name);
            DVP_PRINT(DVP_ZONE_KGB, "Found DVP KGM file %s (%s)\n", dvp->managers[i].dlname, dvp->managers[i].name);
        }
       return DVP_TRUE;
    }
    else
        return DVP_FALSE;
}
#endif

/**
 * This will get the next highest priority enabled core in the managers list given the priority you gave it.
 */
static DVP_U32 DVP_GetNextManagerIndex(DVP_t *dvp, DVP_U32 priority, DVP_U32 *pIndex)
{
    DVP_U32 i = 0;
    DVP_U32 nextHighestIndex = 0;
    DVP_U32 nextHighestPriority = DVP_PRIORITY_MAX; // start off with a high number
    DVP_PRINT(DVP_ZONE_KGB, "DVP Finding Next Highest Priority Manager from %u\n",priority);
    for (i = 0; i < dvp->numMgrs; i++)
    {
        DVP_PRINT(DVP_ZONE_KGB, "DVP Checking Core %u of priority %u (%u)\n",i,dvp->managers[i].priority,nextHighestPriority);
        if (dvp->managers[i].enabled == true_e && dvp->managers[i].priority <= nextHighestPriority && dvp->managers[i].priority > priority)
        {
            nextHighestIndex = i;
            nextHighestPriority = dvp->managers[i].priority;
        }
    }
    if (nextHighestPriority == DVP_PRIORITY_MAX)
        return 0; // no lower priority core exists, return a "false"
    else
    {
        *pIndex = nextHighestIndex;
        return nextHighestPriority;
    }
}

/**
 * Finds a kernel within the Kernel Graph Manager's Kernel List.
 * @param pManager The manager to search.
 * @param kernel The kernel to find.
 * @param pIndex The output pointer to the found index if return DVP_TRUE.
 */
static DVP_BOOL DVP_GetManagerKernelIndex(DVP_GraphManager_t *pManager, DVP_KernelNode_e kernel, DVP_U32 *pIndex)
{
    DVP_U32 i = 0;
    if (pManager->enabled == false_e || pManager->kernels == NULL || pManager->handle == NULL)
    {
        DVP_PRINT(DVP_ZONE_WARNING, "Manager %s is disabled or has no kernels!\n", pManager->name);
        return DVP_FALSE;
    }
    for (i = 0; i < pManager->numSupportedKernels; i++)
    {
        //DVP_PRINT(DVP_ZONE_KGB, "Looking for kernel %u at %s[%u]=>%u (%s)\n", kernel, pManager->name, i, pManager->kernels[i].kernel, pManager->kernels[i].name);
        if ( kernel == (DVP_U32)(pManager->kernels[i].kernel) )
        {
            DVP_PRINT(DVP_ZONE_KGB, "Manager %s supports kernel %s at index %u\n", pManager->name, pManager->kernels[i].name, i);
            *pIndex = i;
            return DVP_TRUE;
        }
    }
    return DVP_FALSE;
}


void DVP_KernelGraphBossDeinit(DVP_t *dvp)
{
    if (dvp)
    {
        DVP_U32 i = 0;

        for (i = 0; i < dvp->numMgrs; i++)
        {
            if (dvp->managers[i].enabled == true_e || dvp->managers[i].rpcc )
            {
                DVP_PRINT(DVP_ZONE_KGB, "Shutting down %s Manager.\n",dvp->managers[i].name);
                // deinitialize the proxy manager

                dvp->managers[i].calls.deinit();

                // disconnect the RPC mechanism
                dvp_rpc_core_deinit(dvp->rpc, &dvp->managers[i].rpcc);

                // disable
                dvp->managers[i].enabled = false_e;
            }

            // unload the proxy manager module
            DVP_KernelGraphManagerUnload(&dvp->managers[i]);
        }

        free(dvp->managers);
        mutex_deinit(&dvp->mgrLock);
        dvp_rpc_deinit(&dvp->rpc);
        dvp_mem_deinit(&dvp->mem);

#if defined(ANDROID)
        if (dvp->loads) {
            char value[PROPERTY_VALUE_MAX];
            int count;
            property_get(DVP_PROPERTY_KEY, value, DVP_PROPERTY_VAL_DFT);
            sscanf(value, DVP_PROPERTY_VAL_FMT, &count);
            count--;
            sprintf(value, DVP_PROPERTY_VAL_FMT, count);
            property_set(DVP_PROPERTY_KEY, value);
            DVP_PRINT(DVP_ZONE_KGB, "There are currently %d users of the DVP Load Table\n", count);
            if (count == 0) // no one is using it
            {
                // we need to reinitialize the table.
                DVP_PRINT(DVP_ZONE_KGB, "Zeroing DVP Load Table.\n");

                dvp->loads->initialized = false_e;
                semaphore_delete(&dvp->loads->sem);
                memset(dvp->loads->cores, 0, sizeof(DVP_Core_Load_t) * DVP_CORE_MAX);
            }
        }
#endif
        shared_free(&dvp->shared_memory);
        memset(dvp, 0xEF, sizeof(DVP_t));
        free(dvp);

    }
}

DVP_t *DVP_KernelGraphBossInit(DVP_U32 mask)
{
    DVP_U32 errors = 0;
    DVP_U32 i,m = 0;
    DVP_t *dvp = (DVP_t*)calloc(1, sizeof(DVP_t));

    if ((mask & DVP_KGB_INIT_KGMS) && dvp && DVP_FindManagers(dvp))
    {
        for (i = 0; i < dvp->numMgrs; i++)
        {
            DVP_PRINT(DVP_ZONE_KGB, "Possible Manager: %s\n", dvp->managers[i].dlname);
        }

        dvp->shared_memory = shared_alloc(DVP_LOAD_TABLE_NAME, sizeof(DVP_Load_t));
        if (dvp->shared_memory)
        {
            dvp->loads = dvp->shared_memory->data;
            if (dvp->loads->initialized == false_e)
            {
                DVP_PRINT(DVP_ZONE_KGB, "Initialized DVP Load Table\n");
                semaphore_create(&dvp->loads->sem, 1, true_e); // only 1 process can access at a time
                dvp->loads->initialized = true_e;
            }
            else
            {
                DVP_PRINT(DVP_ZONE_KGB, "DVP Load Table already exists!\n");
                // it's already been initialized, but is it stale?
            }
#if defined(ANDROID)
            {
                char value[PROPERTY_VALUE_MAX];
                int count;
                property_get(DVP_PROPERTY_KEY, value, DVP_PROPERTY_VAL_DFT);
                sscanf(value, DVP_PROPERTY_VAL_FMT, &count);
                count++;
                sprintf(value, DVP_PROPERTY_VAL_FMT, count);
                property_set(DVP_PROPERTY_KEY, value);
                DVP_PRINT(DVP_ZONE_KGB, "There are currently %d users of the DVP Load Table\n", count);
                if (count == 1) // we're the only one
                {
                    // we need to reinitialize the table.
                    DVP_PRINT(DVP_ZONE_KGB, "Reinitializing Core Load Table Values, as we are the only users\n");
                    memset(dvp->loads->cores, 0, sizeof(DVP_Core_Load_t) * DVP_CORE_MAX);
                }
            }
#endif
        }
        else
        {
#if !defined(ANDROID)
            dvp->shared_memory = calloc(1, sizeof(DVP_Load_t));
            if (dvp->shared_memory)
            {
                dvp->loads = (DVP_Load_t *)dvp->shared_memory;
                semaphore_create(&dvp->loads->sem, 1, false_e);
                dvp->loads->initialized = true_e;
            }
#else
            DVP_PRINT(DVP_ZONE_ERROR, "Could not allocate shared memory, is the shm_service running?\n");
            free(dvp);
            return NULL;
#endif
        }

        // initializes the shared RPC elements, memory, sysmgr, etc.
        dvp->rpc = dvp_rpc_init();

        // initialize the back reference for the sublibraries.
        dvp->rpc->handle = (DVP_Handle)dvp;

        // initialize all the managers
        for (m = 0; m < dvp->numMgrs; m++)
        {
            if (DVP_KernelGraphManagerLoad(&dvp->managers[m]) == 0)
                errors++;
            else
            {
                // get the kernels that the manager supports
                dvp->managers[m].numSupportedKernels = dvp->managers[m].calls.getKernels(&dvp->managers[m].kernels);

                // get the local functions for RCM Server that the manager supports if any
                dvp->managers[m].calls.getLocal(&dvp->managers[m].rpci.localFunctions, &dvp->managers[m].rpci.numLocalFunctions);

                // get the remote functions for the RCM Clients that the manager will call if any
                dvp->managers[m].calls.getRemote(&dvp->managers[m].rpci.remoteFunctions, &dvp->managers[m].rpci.numRemoteFunctions);

                // get the value of the remote core which this code works with
                dvp->managers[m].rpci.coreEnum = dvp->managers[m].calls.getCore();

                // set the kgm priority
                dvp->managers[m].priority = DVP_PRIORITY_USER_BASE; // assume it might be a user supplied kgm
                for (i = 0; i < dimof(f_corePriorities); i++)
                    if (dvp->managers[m].rpci.coreEnum == f_corePriorities[i].core)
                        dvp->managers[m].priority = f_corePriorities[i].priority;

                // create the RPC for this manager
                dvp->managers[m].rpcc = dvp_rpc_core_init(dvp->rpc, &dvp->managers[m].rpci);

                // initialize the maximum load of the core which this manager controls.
                if (dvp->shared_memory)
                    dvp->loads->cores[dvp->managers[m].rpci.coreEnum].maximumLoad = dvp->managers[m].calls.getLoad();

                // register the KGM's restart function with RPC
                dvp_rpc_restart_callback_register(dvp->rpc, dvp->managers[m].rpcc, dvp->managers[m].calls.restart, NULL);

                // give the proxy it's handles (rpcc will be NULL for A9)
                if (dvp->managers[m].calls.init(dvp->rpc, dvp->managers[m].rpcc) == DVP_FALSE)
                {
                    DVP_PRINT(DVP_ZONE_WARNING, "WARNING: DVP Kernel Graph Manager %s failed to initialize!\n", dvp->managers[m].name);
                    errors++;
                    dvp->managers[m].enabled = false_e;
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_KGB, "KGB: %s manager has %u kernels\n",dvp->managers[m].name, dvp->managers[m].numSupportedKernels);
                    dvp->managers[m].enabled = true_e;
                }
            }
        }
    }

    if ((mask & DVP_KGB_INIT_MEM_MGR) && dvp)
    {
        dvp->mem = dvp_mem_init();
        if (dvp->mem == NULL) {
            if ((mask & DVP_KGB_INIT_KGMS))
                errors+=dvp->numMgrs; // force an error condition
            else
                errors++;
        }
    }

    DVP_PRINT(DVP_ZONE_KGB, "There were %d issues during initialization!\n", errors);

    if ( (errors >= m && (mask & DVP_KGB_INIT_KGMS)) ||
         (errors > 0 && !(mask & DVP_KGB_INIT_KGMS)))
    {
        // there's no cores available.
        DVP_PRINT(DVP_ZONE_ERROR, "No cores are available for DVP to use!\n");
        DVP_KernelGraphBossDeinit(dvp);
        dvp = NULL;
    }

    // debugging print for comparing what the ARM and other cores think the size of the node structure is
    DVP_PRINT(DVP_ZONE_KGB, "sizeof(DVP_KernelNode_t)="FMT_SIZE_T" dvp=%p\n",sizeof(DVP_KernelNode_t),dvp);

    return dvp;
}

DVP_BOOL DVP_CommitLoad(DVP_t *dvp, DVP_KernelGraphSection_t *section)
{
    if (dvp == NULL || section == NULL || section->numNodes == 0)
        return DVP_FALSE;
    else
    {
        DVP_BOOL ret = DVP_TRUE;
#ifdef DVP_USE_LOAD_TABLE
        DVP_Core_e c;
        DVP_PRINT(DVP_ZONE_LOAD, "LOAD: Grabbing Shared Memory Semaphore for section %p!\n",section);
        semaphore_wait(&dvp->loads->sem);
        // accumulate each coreLoad in the section into
        // the load's currentLoad as long as it's below the maximumLoad.
        for (c = DVP_CORE_MIN + 1; c < DVP_CORE_MAX; c++)
        {
            dvp->loads->cores[c].currentLoad += section->coreLoad[c];
            DVP_PRINT(DVP_ZONE_LOAD, "LOAD: Attempting to commit core %u load %4u\n", c, section->coreLoad[c]);
            if (dvp->loads->cores[c].currentLoad > dvp->loads->cores[c].maximumLoad)
            {
                DVP_PRINT(DVP_ZONE_ERROR, "ERROR: Could not commit core load of %d as it exceeds maximum value of %d!\n", section->coreLoad[c], dvp->loads->cores[c].maximumLoad);
                // unwind the loads
                for (/*c=c*/; c > DVP_CORE_MIN; c--)
                    dvp->loads->cores[c].currentLoad -= section->coreLoad[c];
                ret = DVP_FALSE;
                break;
            }
        }
        semaphore_post(&dvp->loads->sem);
#endif

        return ret;
    }
}

void DVP_DecommitLoad(DVP_t *dvp, DVP_KernelGraphSection_t *section)
{
    if (dvp == NULL || section == NULL)
        return;
    else
    {
#ifdef DVP_USE_LOAD_TABLE
        DVP_Core_e c;
        semaphore_wait(&dvp->loads->sem);
        // remove the section's load from the cores
        for (c = DVP_CORE_MIN + 1; c < DVP_CORE_MAX; c++)
            dvp->loads->cores[c].currentLoad -= section->coreLoad[c];
        semaphore_post(&dvp->loads->sem);
#endif
    }
}

DVP_U32 DVP_ConfigureNodes(DVP_t *dvp, DVP_KernelGraphSection_t *section, DVP_BOOL force)
{
    DVP_U32 m = 0, n = 0, f = 0, faults = 0;
    DVP_KernelNode_t *pNodes = section->pNodes;
    DVP_U32 numNodes = section->numNodes;
    clock_t total = 0;

    if (dvp == NULL || section == NULL)
        return 1;

    for (n = 0; n < numNodes; n++)
    {
        DVP_PrintNode(DVP_ZONE_KGB, &section->pNodes[n]);
        if (pNodes[n].header.configured == DVP_FALSE || force == DVP_TRUE)
        {
            DVP_U32 triedMgrs = 0;
            DVP_U32 mgrPriority = 0;
            DVP_BOOL isKernelSupported = DVP_FALSE;
            rtime_t diff, start = rtimer_now();

            // did the user intentionally set an affinity?
            if (pNodes[n].header.affinity != DVP_CORE_MIN)
            {
                DVP_BOOL found = DVP_FALSE; // haven't found the right manager yet

                // find which manager works on the desired core
                for (m = 0; m < dvp->numMgrs; m++)
                {
                    if (dvp->managers[m].calls.getCore() == pNodes[n].header.affinity)
                    {
                        // get the function index
                        isKernelSupported = DVP_GetManagerKernelIndex(&dvp->managers[m], pNodes[n].header.kernel, &f);
                        if (isKernelSupported == DVP_TRUE)
                        {
                            DVP_PRINT(DVP_ZONE_KGB, "Node[%u] will be executed on %s core as %s [AFFINITY]\n", n, dvp->managers[m].name, dvp->managers[m].kernels[f].name);
                            found = DVP_TRUE;
                            // accumulate the load value for this kernel into the section's core loads
                            section->coreLoad[pNodes[n].header.affinity] += dvp->managers[m].kernels[f].load;
                            // remember the index
                            pNodes[n].header.mgrIndex = m;
                            // remember the function index too!
                            pNodes[n].header.funcIndex = f;
                            break;
                        }
                    }
                }
                if (found == DVP_TRUE)
                {
                    pNodes[n].header.configured = DVP_TRUE;
                    continue; // don't try to set the manager based on priority or load
                }
                else
                {
                    // if we've run out of managers to check (or nothing matched)
                    // we ignore the affinity in future checks
                    pNodes[n].header.affinity = DVP_CORE_MIN;
                }
            }
            // determine which manager will execute the function based on the priority of the manager then of the kernel.
            do {
                mgrPriority = DVP_GetNextManagerIndex(dvp, mgrPriority, &m);
                if (mgrPriority != 0)
                    isKernelSupported = DVP_GetManagerKernelIndex(&dvp->managers[m], pNodes[n].header.kernel, &f);
                if (mgrPriority == 0 || isKernelSupported == DVP_FALSE)
                {
                    DVP_PRINT(DVP_ZONE_WARNING, "KGB: WARNING! Kernel %u is not supported on %s manager\n", pNodes[n].header.kernel,dvp->managers[m].name);
                    triedMgrs++;
                }
                else
                {
                    // remember the manager index and function index.
                    pNodes[n].header.mgrIndex = m;
                    pNodes[n].header.funcIndex = f;

                    // accumulate the load into the section's coreLoad
                    section->coreLoad[dvp->managers[m].calls.getCore()] += dvp->managers[m].kernels[f].load;

                    DVP_PRINT(DVP_ZONE_KGB, "Node[%u] will be executed on %s core as %s with %u Mhz\n", n, dvp->managers[m].name, dvp->managers[m].kernels[f].name, dvp->managers[m].kernels[f].load);
                    break;
                }
            } while (triedMgrs < dvp->numMgrs);
            if (triedMgrs >= dvp->numMgrs)
            {
                DVP_PRINT(DVP_ZONE_ERROR, "ERROR! No core supports kernel %u!\n", pNodes[n].header.kernel);
                pNodes[n].header.error = DVP_ERROR_NOT_IMPLEMENTED;
                faults++;
                break;
            }

            // once it's configured don't do it again! (loads may change that though)
            pNodes[n].header.configured = DVP_TRUE;

            diff = rtimer_now() - start;
            total += diff;
        }
    }
    if (total > 0)
    {
        DVP_PRINT(DVP_ZONE_PERF, "KGB: Graph Setup took %lf secs\n", (float)total/rtimer_freq());
    }
    return faults;
}

DVP_U32 DVP_KernelGraphBoss_Verify(DVP_t *dvp,  DVP_KernelGraphSection_t *section)
{
    DVP_U32 n = 0;
    DVP_U32 faults = 0;
    DVP_U32 verified = 0;
    DVP_KernelNode_t *pNodes = section->pNodes;
    DVP_U32 numNodes = section->numNodes;

    DVP_PRINT(DVP_ZONE_KGB, "Verifing Section %p with %u nodes\n", section, section->numNodes);

    // configure the nodes
    faults = DVP_ConfigureNodes(dvp, section, DVP_FALSE);

    if (faults == 0)
    {
        DVP_PRINT(DVP_ZONE_KGB, "KGB: Verifying Graph Section with %u nodes!\n", numNodes);
        for (n = 0; n < numNodes; /* no inc */)
        {
            DVP_U32 targetMgrIndex = pNodes[n].header.mgrIndex;
            DVP_U32 subgraphNumNodes = 1; // at least one node is on this core...
            DVP_U32 numNodeVerified = 0;
#ifdef DVP_OPTIMIZED_GRAPHS
            DVP_U32 i;
            // determine how many nodes forward of the current node are on the same core...
            for (i = n+1; i < numNodes; i++)
            {
                if (pNodes[i].header.mgrIndex == targetMgrIndex)
                    subgraphNumNodes++;
                else
                    break;
            }
#endif
            DVP_PRINT(DVP_ZONE_KGB, "KGB: Verifying %u nodes on %s core\n", subgraphNumNodes, dvp->managers[pNodes[n].header.mgrIndex].name);
            numNodeVerified = dvp->managers[pNodes[n].header.mgrIndex].calls.verify(pNodes,n,subgraphNumNodes);
            DVP_PRINT(DVP_ZONE_KGB, "KGB: Verified %u nodes on %s core\n", numNodeVerified, dvp->managers[pNodes[n].header.mgrIndex].name);
            verified += numNodeVerified;
            n += subgraphNumNodes;
        }
        return verified;
    }
    else
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Section %p has faults in affinity or kernel availability\n", section);
        return 0;
    }
}

DVP_U32 DVP_KernelGraphBoss_Process(DVP_t *dvp, DVP_KernelGraphSection_t *section, DVP_BOOL sync)
{
    DVP_U32 n = 0; // node index
    DVP_U32 faults = 0;
    DVP_U32 processed = 0;
    DVP_KernelNode_t *pNodes = section->pNodes;
    DVP_U32 numNodes = section->numNodes;
    DVP_Perf_t *perf = &(section->perf);

    DVP_PRINT(DVP_ZONE_KGB, "Executing Section %p with %u nodes (%s)\n", section, section->numNodes, (sync?"SYNC":"QUEUED"));

    DVP_PerformanceStart(perf);

    // we're going to try this over and over until we can execute with no faults in configuration.
    do {
        if (faults == 0 && DVP_CommitLoad(dvp, section) == DVP_TRUE)
        {
            DVP_PRINT(DVP_ZONE_KGB, "KGB: Executing Graph!\n");
            for (n = 0; n < numNodes; /* no inc */)
            {
                DVP_U32 targetMgrIndex = pNodes[n].header.mgrIndex;
                DVP_U32 subgraphNumNodes = 1; // at least one node is on this core...
                DVP_U32 numNodesProcessed = 0;
#ifdef DVP_OPTIMIZED_GRAPHS
                DVP_U32 i;
                // determine how many nodes forward of the current node are on the same core...
                for (i = n+1; i < numNodes; i++)
                {
                    if (pNodes[i].header.mgrIndex == targetMgrIndex)
                        subgraphNumNodes++;
                    else
                        break;
                }
#endif
                DVP_PRINT(DVP_ZONE_KGB, "KGB: Executing %u nodes on %s core\n", subgraphNumNodes, dvp->managers[pNodes[n].header.mgrIndex].name);
                numNodesProcessed = dvp->managers[pNodes[n].header.mgrIndex].calls.manager(pNodes,n,subgraphNumNodes, sync);
                // increment the processed by the number literally processed (regardless of errors)
                processed += numNodesProcessed;
                DVP_PRINT(DVP_ZONE_KGB, "KGB: Executed %u nodes on %s core (%u total processed)\n", numNodesProcessed, dvp->managers[pNodes[n].header.mgrIndex].name, processed);
                // increment the graph by the number of nodes given the subgraph!
                n += subgraphNumNodes; // we can't use the processed number here.
#ifndef DVP_CONTINUE_ON_ERRORS
                if (numNodesProcessed != subgraphNumNodes)
                    break; // something went wrong in the graph
#endif
            }
            DVP_DecommitLoad(dvp, section);
            // success!
            break;
        }
        else
        {
            DVP_PRINT(DVP_ZONE_WARNING, "KGB-RM: Graph could not be executed due to a shortage of resources.\n");
            // reconfigure the graph
            faults = DVP_ConfigureNodes(dvp, section, DVP_TRUE);
        }
    } while (faults == 0);

    // if there are faults it should stop the processing of the graph.
    if (faults > 0)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR! There are unhandled nodes in the graph. Processing can not continue!\n");
    }

    DVP_PerformanceStop(perf);

    DVP_PRINT(DVP_ZONE_KGB, "KGB: Processed %u nodes!\n", processed);

#if defined(GRE_DEBUG) && (DVP_ZONE_LOAD != 0)
    {
        DVP_Core_e c;
        //semaphore_wait(&dvp->loads->sem);
        for (c = DVP_CORE_MIN + 1; c < DVP_CORE_MAX; c++)
        {
            DVP_PRINT(DVP_ZONE_LOAD, "Core[%u] Current Load=%4u Maximum Load=%4u\n", c, dvp->loads->cores[c].currentLoad, dvp->loads->cores[c].maximumLoad);
        }
        //semaphore_post(&dvp->loads->sem);
    }
#endif

    return processed;
}


void DVP_SetMaxLoad(DVP_t *dvp, DVP_Core_e core, DVP_U32 load)
{
    if(dvp)
    {
        DVP_U32 m;
        for (m = 0; m < dvp->numMgrs; m++)
        {
            if (dvp->managers[m].enabled && dvp->managers[m].calls.getCore() == core)
            {
#if defined(DVP_USE_LOAD_TABLE)
                if (dvp->loads->initialized)
                {
                    semaphore_wait(&dvp->loads->sem);
                    dvp->loads->cores[core].maximumLoad = load;
                    semaphore_post(&dvp->loads->sem);
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR: Load Table is not initialized!\n");
                }
#endif
                DVP_PRINT(DVP_ZONE_LOAD, "DVP: Core[%u] now has table max load of %u\n", core, load);
            }
        }
    }
}

DVP_U32 DVP_GetMaxLoad(DVP_t *dvp, DVP_Core_e core, DVP_U32 *pCoreMax)
{
    DVP_U32 load = 0;
    if(dvp)
    {
        DVP_U32 m;
        for (m = 0; m < dvp->numMgrs; m++)
        {
            if (dvp->managers[m].enabled && dvp->managers[m].calls.getCore() == core)
            {
                *pCoreMax = dvp->managers[m].calls.getLoad();
#if defined(DVP_USE_LOAD_TABLE)
                if (dvp->loads->initialized)
                {
                    semaphore_wait(&dvp->loads->sem);
                    load = dvp->loads->cores[core].maximumLoad;
                    semaphore_post(&dvp->loads->sem);
                }
#endif
                DVP_PRINT(DVP_ZONE_LOAD, "DVP: Core[%u] has table max load of %u and HW max load of %u\n", core, load, *pCoreMax);
            }
        }
    }
    return load;
}

void DVP_QueryCore(DVP_t *dvp, DVP_Core_e core, DVP_CoreInfo_t *info)
{
    DVP_U32 m = 0;
    DVP_BOOL found = DVP_FALSE;
    if (info)
    {
        for (m = 0; m < dvp->numMgrs; m++)
        {
            DVP_PRINT(DVP_ZONE_KGB, "Quering Mgr %u for Core %d\n", m, core);
            if (dvp->managers[m].enabled == true_e && dvp->managers[m].calls.getCore() == core)
            {
                DVP_U32 k = 0;
                info->enabled = DVP_TRUE;
                info->numKernels += dvp->managers[m].numSupportedKernels;
                info->kernels = dvp->managers[m].kernels;
                found = DVP_TRUE;
                DVP_PRINT(DVP_ZONE_KGB, "Scanning Mgr: %u for which kernel libraries are supported out of %u kernels\n",
                          m, dvp->managers[m].numSupportedKernels);
                for (k = 0; k < dvp->managers[m].numSupportedKernels; k++)
                {
                    DVP_KernelNode_e kernel = dvp->managers[m].kernels[k].kernel;
                    DVP_PRINT(DVP_ZONE_KGB, "\tKernel: 0x%08x\n", kernel);
                    if (kernel > DVP_KN_LIBRARY_START)
                    {
                        DVP_U32 lib = DVP_KN_LIBRARY_MASK(kernel);
                        DVP_PRINT(DVP_ZONE_KGB, "Mgr: %u Kernel:%x is in Library: 0x%x\n", m, kernel, lib);
                        info->libraries[lib] = DVP_TRUE;
                    }
                }
            }
        }
    }
    if (found == DVP_FALSE)
    {
        info->enabled = DVP_FALSE;
    }
}

DVP_BOOL DVP_QueryCoreForKernel(DVP_t *dvp, DVP_KernelNode_e kernel, DVP_Core_e core)
{
    DVP_U32 m = 0;
    for (m = 0; m < dvp->numMgrs; m++)
    {
        DVP_PRINT(DVP_ZONE_KGB, "Checking Mgr %u for Core %d\n", m, core);
        if (dvp->managers[m].enabled == true_e && dvp->managers[m].calls.getCore() == core)
        {
            DVP_U32 k;
            for (k = 0; k < dvp->managers[m].numSupportedKernels; k++)
            {
                if (kernel == (DVP_U32)(dvp->managers[m].kernels[k].kernel) )
                {
                    return DVP_TRUE;
                }
            }
        }
    }
    return DVP_FALSE;
}
