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

// external
#include <dvp/dvp.h>
#include <dvp/dvp_debug.h>
// internal
#include <dvp_kgm.h>
#include <dvp_kgb.h>

#define MAX_SECTIONS    (4) // a silly upper limit
#define MAX_QUEUE_DEPTH (2)

//******************************************************************************
// TYPEDEFS
//******************************************************************************

typedef struct _dvp_kgw_data_t {
    semaphore_t sem;
    uint32_t    count;
} DVP_KernelGraphWorker_t;

typedef struct _dvp_kgc_t {
    thread_t hThread;
    DVP_t *dvp;
    DVP_KernelGraph_t *graph;
    DVP_KernelGraphSection_t *section;
    DVP_U32 numNodes;
    DVP_U32 index;
    mutex_t *lock;
    void *cookie;
    DVP_SectionComplete_f callback;
} DVP_KernelGraphCollector_t;


//******************************************************************************
//  GLOBAL VARIABLES
//******************************************************************************

uint32_t dvp_zone_mask = 0x3;

#if defined(DVP_RUNTIME_DEBUG)
#pragma message("DVP Debugging in RUNTIME mode! export DVP_ZONE_MASK to affect.")
#endif

//******************************************************************************
//  LOCAL VARIABLES
//******************************************************************************

static threadpool_t *pool;
static DVP_KernelGraphCollector_t collectors[MAX_SECTIONS];
static DVP_KernelGraphWorker_t workData;

//******************************************************************************
// LOCAL FUNCTIONS
//******************************************************************************

static bool_e dvp_kernelgraph_worker(threadpool_worker_t *worker)
{
    DVP_KernelGraphWorker_t *kgw = (DVP_KernelGraphWorker_t *)worker->arg;
    DVP_KernelGraphCollector_t *collector = (DVP_KernelGraphCollector_t *)worker->data;
    DVP_U32 numNodesExecuted = 0;
    DVP_PRINT(DVP_ZONE_KGAPI, "DVP KGW %u Thread "THREAD_FMT" Running!\n", worker->index, worker->handle);

    numNodesExecuted = DVP_KernelGraphBoss_Process(collector->dvp,collector->section, DVP_FALSE);

    // make the callback thread-safe
    semaphore_wait(&kgw->sem);
    kgw->count++; // increment the number of sections processed.
    if (collector->callback)
        collector->callback(collector->cookie, collector->graph, collector->index, numNodesExecuted);
    DVP_PRINT(DVP_ZONE_KGAPI, "DVP KGW %u Thread "THREAD_FMT" Finished! (Nodes: %u Count: %u)\n", worker->index, worker->handle, numNodesExecuted, kgw->count);
    semaphore_post(&kgw->sem);

    if (numNodesExecuted > 0)
        return true_e;
    else
        return false_e;
}

static void dvp_graphlock_init(DVP_GraphLock_t *gl)
{
    gl->m_enabled = true_e;
    gl->m_count = 0;
    mutex_init(&gl->m_lock);
    event_init(&gl->m_wait, false_e);
    event_set(&gl->m_wait);
}

static DVP_BOOL dvp_graph_lock(DVP_GraphLock_t *gl)
{
    mutex_lock(&gl->m_lock);
    if (gl->m_enabled)
    {
        gl->m_count++;
        event_reset(&gl->m_wait);
    }
    mutex_unlock(&gl->m_lock);
    return (gl->m_enabled == true_e ? DVP_TRUE : DVP_FALSE);
}

static void dvp_graph_unlock(DVP_GraphLock_t *gl)
{
    mutex_lock(&gl->m_lock);
    if (gl->m_enabled)
    {
        gl->m_count--;
        if (gl->m_count == 0)
            event_set(&gl->m_wait);
    }
    mutex_unlock(&gl->m_lock);
}

static void dvp_graphlock_wait(DVP_GraphLock_t *gl)
{
    event_wait(&gl->m_wait, EVENT_FOREVER);
    mutex_lock(&gl->m_lock);
    gl->m_enabled = false_e;
    mutex_unlock(&gl->m_lock);
}

static void dvp_graphlock_deinit(DVP_GraphLock_t *gl)
{
    gl->m_enabled = false_e;
    gl->m_count = 0;
    event_deinit(&gl->m_wait);
    mutex_deinit(&gl->m_lock);
}

//******************************************************************************
// GLOBAL FUNCTIONS
//******************************************************************************

DVP_Handle DVP_KernelGraph_Init()
{
    DVP_t *dvp = NULL;
    semaphore_create(&workData.sem, 1, false_e);
    workData.count = 0;
    memset(collectors, 0, sizeof(DVP_KernelGraphCollector_t) * MAX_SECTIONS);
    pool = threadpool_create(MAX_SECTIONS, MAX_QUEUE_DEPTH, sizeof(DVP_KernelGraphCollector_t), dvp_kernelgraph_worker, &workData);
    dvp = DVP_KernelGraphBossInit(DVP_KGB_INIT_ALL);
    if (dvp)
        dvp_graphlock_init(&dvp->graphLock);
    return (DVP_Handle)dvp;
}

DVP_Handle DVP_MemImporter_Create()
{
    DVP_t *dvp = NULL;
    dvp = DVP_KernelGraphBossInit(DVP_KGB_INIT_MEM_MGR);
    return (DVP_Handle)dvp;
}

void DVP_KernelGraph_Deinit(DVP_Handle handle)
{
    DVP_t *dvp = (DVP_t *)handle;
    DVP_PRINT(DVP_ZONE_KGAPI, "Deconstructing DVP=%p! This will block until graphs have completed!\n",dvp);
    if (dvp) {
        dvp_graphlock_wait(&dvp->graphLock); // this will block until all graphs have exited.
        DVP_PRINT(DVP_ZONE_KGAPI, "Graphs should all be complete!\n");
        // if the thread pools are active, tear them down, then deinit DVP
        threadpool_destroy(pool);
        semaphore_delete(&workData.sem);
        dvp_graphlock_deinit(&dvp->graphLock);
        DVP_KernelGraphBossDeinit(dvp);
    }
}

void DVP_MemImporter_Free(DVP_Handle handle)
{
    DVP_t *dvp = (DVP_t *)handle;
    if (dvp) {
        DVP_KernelGraphBossDeinit(dvp);
    }
}

DVP_BOOL DVP_KernelGraph_Verify(DVP_Handle handle, DVP_KernelGraph_t *pGraph)
{
    DVP_t *dvp = (DVP_t *)handle;
    if (dvp && pGraph)
    {
        // make sure to "lock the graph", if we can't then we're tearing down.
        if (dvp_graph_lock(&dvp->graphLock) == DVP_FALSE)
            return DVP_FALSE;

        if (pGraph->verified == DVP_FALSE)
        {
            DVP_U32 s = 0;
            DVP_U32 numNodesVerified = 0;
            DVP_U32 numNodes = 0;
            for (s = 0; s < pGraph->numSections; s++)
            {
                numNodes += pGraph->sections[s].numNodes;
                numNodesVerified += DVP_KernelGraphBoss_Verify(dvp, &pGraph->sections[s]);
                DVP_PRINT(DVP_ZONE_KGAPI, "%u nodes pass verify on section %u\n", numNodesVerified, s);
            }

            if (numNodes == numNodesVerified)
                pGraph->verified = DVP_TRUE;

            DVP_PRINT(DVP_ZONE_KGAPI, "%u nodes of %u passed verification!\n", numNodesVerified, numNodes);
        }

        dvp_graph_unlock(&dvp->graphLock);

        return pGraph->verified;
    }
    else
        return DVP_FALSE;
}

DVP_U32 DVP_KernelGraph_Process(DVP_Handle handle, DVP_KernelGraph_t *pGraph, void *cookie, DVP_SectionComplete_f callback)
{
    DVP_t *dvp = (DVP_t *)handle;
    DVP_U32 numSectionsRun = 0;
    DVP_U32 section = 0;
    DVP_U32 order = 0;
    DVP_U32 numOrder = 0;
    DVP_KernelGraphSection_t *ppSections[MAX_SECTIONS]; // only up to MAX_SECTIONS in parallel.

    // make sure to "lock the graph", if we can't then we're tearing down.
    if (dvp_graph_lock(&dvp->graphLock) == DVP_FALSE)
        return 0;

    if (pGraph->verified == DVP_FALSE)
    {
        DVP_PRINT(DVP_ZONE_KGAPI, "Graph has not been verified!\n");
        dvp_graph_unlock(&dvp->graphLock);
        if (DVP_KernelGraph_Verify(handle, pGraph) == DVP_FALSE)
        {
            DVP_PRINT(DVP_ZONE_ERROR, "Graph failed verification!\n");
            return 0;
        }
        dvp_graph_lock(&dvp->graphLock);
    }

    DVP_PerformanceStart(&(pGraph->totalperf));

    for (order = 0; /* no order limit */; order++)
    {
        // initialize each cycle
        numOrder = 0;
        memset(ppSections, 0, sizeof(DVP_KernelGraphSection_t *)*MAX_SECTIONS);

        // determine how many graph want to execute in this order.
        for (section = 0; section < pGraph->numSections; section++)
        {
            // if the section is of the correct order, the number of orders is less than the max and if the section is not skipped
            if (pGraph->order[section] == order && numOrder < MAX_SECTIONS && pGraph->sections[section].skipSection == DVP_FALSE)
            {
               ppSections[numOrder] = &pGraph->sections[section];
               numOrder++;
            }
            else if (numOrder >= MAX_SECTIONS)
            {
                DVP_PRINT(DVP_ZONE_ERROR, "Can't run more than %u graphs in parallel at any given time\n", MAX_SECTIONS);
                return numSectionsRun;
            }
            else if (pGraph->sections[section].skipSection == DVP_TRUE)
            {
                DVP_PRINT(DVP_ZONE_KGAPI, "Skipping Graph Section %p[%u]\n", &pGraph->sections[section], section);
            }
        }

        DVP_PRINT(DVP_ZONE_KGAPI, "DVP_KernelGraphs => Order %u has %u sections\n", order, numOrder);

        if (numOrder > 1) // one or multiple sections are going to run in parallel.
        {
            DVP_U32 s,g = 0;
            void *workitems[dimof(collectors)];

            for (g = 0; g < numOrder; g++)
            {
                collectors[g].dvp = dvp;
                collectors[g].graph = pGraph;
                collectors[g].section = ppSections[g];
                for (s = 0; s < pGraph->numSections; s++)
                    if (collectors[g].section->pNodes == pGraph->sections[s].pNodes)
                        collectors[g].index = s;
                collectors[g].cookie = cookie;
                collectors[g].callback = callback;
                workitems[g] = &collectors[g];
            }

            if (threadpool_issue(pool, workitems, numOrder) == true_e)
            {
                DVP_PRINT(DVP_ZONE_KGAPI, "DVP_KernelGraphs issued %u sections to execute in parallel\n", numOrder);
            }

            if (threadpool_complete(pool, true_e) == true_e)
            {
                DVP_PRINT(DVP_ZONE_KGAPI, "DVP_KernelGraphs completed %u sections\n", numOrder);
                numSectionsRun += g;
            }
            else
            {
                DVP_PRINT(DVP_ZONE_ERROR, "ERROR: FAILED TO WAIT FOR SECTIONS TO COMPLETE!\n");
            }
        }
        else if (numOrder == 1) // special optimized case
        {
            DVP_U32 numNodesExecuted = 0;
            DVP_PRINT(DVP_ZONE_KGAPI, "Optimized single section mode!\n");

            // force inline or synchronous execution.
            // This thread will do the local computation or call remote cores.
            numNodesExecuted = DVP_KernelGraphBoss_Process(dvp,ppSections[0], DVP_TRUE);
            if (callback)
            {
                // find the section index
                for (section = 0; section < pGraph->numSections; section++)
                    if (ppSections[0] == &pGraph->sections[section])
                        break;
                callback(cookie, pGraph, section, numNodesExecuted);
            }
            // increment the number of graphs ran regardless of the
            // presence of a callback.
            numSectionsRun++;
        }
        else
            break; // if no graphs of this order, then exit.
    }
    DVP_PerformanceStop(&(pGraph->totalperf));
    dvp_graph_unlock(&dvp->graphLock);
    return numSectionsRun;
}


DVP_U32 DVP_GetCoreCapacity(DVP_Handle handle, DVP_Core_e core)
{
    if (handle)
    {
        DVP_t *dvp = (DVP_t *)handle;
        DVP_U32 hwMaxLoad = 0;
        DVP_U32 newMaxload = DVP_GetMaxLoad(dvp, core, &hwMaxLoad);
        if (hwMaxLoad != 0) {
            return (newMaxload * DVP_CAPACITY_DATA_RANGE)/hwMaxLoad;
        }
    }
    return 0;
}

void DVP_SetCoreCapacity(DVP_Handle handle, DVP_Core_e core, DVP_U32 limit)
{
    if (handle)
    {
        DVP_t *dvp = (DVP_t *)handle;
        DVP_U32 newMaxLoad, hwMaxLoad = 0;
        DVP_GetMaxLoad(dvp, core, &hwMaxLoad);
        if (limit > DVP_CAPACITY_DATA_RANGE)
            limit = DVP_CAPACITY_DATA_RANGE;
        newMaxLoad = (limit * hwMaxLoad)/DVP_CAPACITY_DATA_RANGE;
        DVP_SetMaxLoad(dvp, core, newMaxLoad);
    }
}

void DVP_Perf_Clear(DVP_Perf_t *perf)
{
    memset(perf, 0, sizeof(DVP_Perf_t));
    perf->rate = rtimer_freq();
    perf->minTime = MAX_RTIMER_T;
}

void DVP_PerformanceClear(DVP_Handle handle __attribute__ ((unused)), DVP_KernelNode_t *pNodes, DVP_U32 numNodes)
{
    DVP_U32 i = 0;
    for (i = 0; i < numNodes; i++)
    {
        DVP_Perf_Clear(&pNodes[i].header.perf);
    }
}

void DVP_PerformanceStart(DVP_Perf_t *pPerf)
{
    if (pPerf)
        pPerf->tmpTime = rtimer_now();
}

void DVP_PerformanceStop(DVP_Perf_t *pPerf)
{
    if (pPerf)
    {
        rtime_t now = rtimer_now();
        pPerf->tmpTime = (now - pPerf->tmpTime);
        pPerf->numTimes++;
        if (pPerf->minTime > pPerf->tmpTime)
            pPerf->minTime = pPerf->tmpTime;
        if (pPerf->maxTime < pPerf->tmpTime)
            pPerf->maxTime = pPerf->tmpTime;
        pPerf->sumTime += pPerf->tmpTime;
        pPerf->avgTime = pPerf->sumTime/pPerf->numTimes;
    }
}

void DVP_PerformancePrint(DVP_Perf_t *pPerf, const char *prefix)
{
    pPerf = pPerf; // warnings
    prefix = prefix; // warnings
    DVP_PRINT(DVP_ZONE_PERF, "%s: count=%9u (us) min="FMT_U64(14)" max="FMT_U64(14)" avg="FMT_U64(14)" sum="FMT_U64(14)" last="FMT_U64(14)"\n", prefix,
            pPerf->numTimes,
            (DVP_U64)rtimer_to_us(pPerf->minTime),
            (DVP_U64)rtimer_to_us(pPerf->maxTime),
            (DVP_U64)rtimer_to_us(pPerf->avgTime),
            (DVP_U64)rtimer_to_us(pPerf->sumTime),
            (DVP_U64)rtimer_to_us(pPerf->tmpTime));
}

void DVP_PrintPerformanceCSV(DVP_Handle handle, DVP_KernelNode_t *pNodes, DVP_U32 numNodes)
{
    DVP_U32 i = 0;
    DVP_t *dvp = (DVP_t *)handle;

    // print out the performance info
    dvp = dvp; // warnings
    pNodes = pNodes; // warnings

    DVP_PRINT(DVP_ZONE_PERF, "Node, %6s, %8s, %14s, %14s, %14s, %14s,\n",
           "Core", "Kernel", "Minmum","Maximum","Average","Summation");
    for (i = 0; i < numNodes; i++)
    {
        DVP_PRINT(DVP_ZONE_PERF, "%4u, %6s[%u], %8u, "FMT_U64(14)", "FMT_U64(14)", "FMT_U64(14)", "FMT_U64(14)",\n", i, dvp->managers[pNodes[i].header.mgrIndex].name, pNodes[i].header.mgrIndex, pNodes[i].header.kernel,
                  (DVP_U64)rtimer_from_rate_to_us(pNodes[i].header.perf.minTime, pNodes[i].header.perf.rate),
                  (DVP_U64)rtimer_from_rate_to_us(pNodes[i].header.perf.maxTime, pNodes[i].header.perf.rate),
                  (DVP_U64)rtimer_from_rate_to_us(pNodes[i].header.perf.avgTime, pNodes[i].header.perf.rate),
                  (DVP_U64)rtimer_from_rate_to_us(pNodes[i].header.perf.sumTime, pNodes[i].header.perf.rate));
    }
}

void DVP_PrintPerformanceSection(DVP_Handle handle, DVP_KernelGraphSection_t *pSection)
{
    DVP_t *dvp = (DVP_t *)handle;
    DVP_KernelNode_t *pNodes = pSection->pNodes;
    DVP_U32 numNodes = pSection->numNodes;
    DVP_Perf_t *perf = &pSection->perf;
    DVP_U32 i;
    DVP_Core_e c;
    DVP_Perf_t overhead;

    DVP_Perf_Clear(&overhead);
    overhead.minTime = 0;

    dvp = dvp; // warnings

    DVP_PRINT(DVP_ZONE_PERF, "rtimer_freq() = "FMT_RTIMER_T"\n", rtimer_freq());
    DVP_PRINT(DVP_ZONE_PERF, "Node Performance (us):\n");
    DVP_PRINT(DVP_ZONE_PERF, "Node, %9s, %10s, %14s, %14s, %14s, %14s, %14s\n",
           "Core", "Kernel", "Minmum","Maximum","Average","Summation", "Error Code");
    for (i = 0; i < numNodes; i++)
    {
        // if the minimum was never set, set it to zero now.
        if (pNodes[i].header.perf.minTime == MAX_RTIMER_T)
            pNodes[i].header.perf.minTime = 0;

        overhead.minTime += rtimer_from_rate_to_us(pNodes[i].header.perf.minTime, pNodes[i].header.perf.rate);
        overhead.avgTime += rtimer_from_rate_to_us(pNodes[i].header.perf.avgTime, pNodes[i].header.perf.rate);
        overhead.maxTime += rtimer_from_rate_to_us(pNodes[i].header.perf.maxTime, pNodes[i].header.perf.rate);
        overhead.sumTime += rtimer_from_rate_to_us(pNodes[i].header.perf.sumTime, pNodes[i].header.perf.rate);

        DVP_PRINT(DVP_ZONE_PERF, "%4u, %6s[%u], 0x%08x, "FMT_U64(14)", "FMT_U64(14)", "FMT_U64(14)", "FMT_U64(14)", %14d\n",
            i, dvp->managers[pNodes[i].header.mgrIndex].name,
            pNodes[i].header.mgrIndex,
            pNodes[i].header.kernel,
            (DVP_U64)rtimer_from_rate_to_us(pNodes[i].header.perf.minTime, pNodes[i].header.perf.rate),
            (DVP_U64)rtimer_from_rate_to_us(pNodes[i].header.perf.maxTime, pNodes[i].header.perf.rate),
            (DVP_U64)rtimer_from_rate_to_us(pNodes[i].header.perf.avgTime, pNodes[i].header.perf.rate),
            (DVP_U64)rtimer_from_rate_to_us(pNodes[i].header.perf.sumTime, pNodes[i].header.perf.rate),
            pNodes[i].header.error);
    }

    DVP_PRINT(DVP_ZONE_PERF, "Graph Section Perf (us):\n");
    DVP_PRINT(DVP_ZONE_PERF, "%4u, "FMT_U64(32)", "FMT_U64(14)", "FMT_U64(14)", "FMT_U64(14)",\n", numNodes,
            (DVP_U64)rtimer_to_us(perf->minTime),
            (DVP_U64)rtimer_to_us(perf->maxTime),
            (DVP_U64)rtimer_to_us(perf->avgTime),
            (DVP_U64)rtimer_to_us(perf->sumTime));
    overhead.minTime = rtimer_to_us(perf->minTime) - overhead.minTime;
    overhead.maxTime = rtimer_to_us(perf->maxTime) - overhead.maxTime;
    overhead.avgTime = rtimer_to_us(perf->avgTime) - overhead.avgTime;
    overhead.sumTime = rtimer_to_us(perf->sumTime) - overhead.sumTime;
    DVP_PRINT(DVP_ZONE_PERF, "Section Overhead (Threading + IPC):\n");
    DVP_PRINT(DVP_ZONE_PERF, "%4u, "FMT_U64(32)", "FMT_U64(14)", "FMT_U64(14)", "FMT_U64(14)",\n", numNodes,
            (DVP_U64)overhead.minTime,
            (DVP_U64)overhead.maxTime,
            (DVP_U64)overhead.avgTime,
            (DVP_U64)overhead.sumTime);

    DVP_PRINT(DVP_ZONE_PERF, "Graph Section Core Load:\n");
    for (c = DVP_CORE_MIN + 1; c < DVP_CORE_MAX; c++)
    {
        DVP_PRINT(DVP_ZONE_PERF, "[%u] %4u\n", c, pSection->coreLoad[c]);
    }
}

void DVP_PrintPerformanceGraph(DVP_Handle handle, DVP_KernelGraph_t *pGraph)
{
    DVP_Perf_t *perf = &pGraph->totalperf;
    DVP_U32 totalNumNodes = 0;
    DVP_U32 i = 0;

    perf = perf; // warnings

    for (i = 0; i < pGraph->numSections; i++)
    {
        DVP_PrintPerformanceSection(handle, &pGraph->sections[i]);
        totalNumNodes += pGraph->sections[i].numNodes;
    }

    DVP_PRINT(DVP_ZONE_PERF, "Overall Graph %p Perf (us):\n", pGraph);
    DVP_PRINT(DVP_ZONE_PERF, "%4u, "FMT_U64(32)", "FMT_U64(14)", "FMT_U64(14)", "FMT_U64(14)",\n", totalNumNodes,
            (DVP_U64)rtimer_to_us(perf->minTime),
            (DVP_U64)rtimer_to_us(perf->maxTime),
            (DVP_U64)rtimer_to_us(perf->avgTime),
            (DVP_U64)rtimer_to_us(perf->sumTime));

}

void DVP_PrintNode(DVP_U32 zone, DVP_KernelNode_t *node)
{
    if (zone && node)
    {
        DVP_PRINT(zone, "DVP_KernelNode_t:%p KERN:%08x AFFINITY:%d MGR:%u FXN:%u\n", node, node->header.kernel, node->header.affinity, node->header.mgrIndex, node->header.funcIndex);
    }
}

void DVP_PrintImage(DVP_U32 zone, DVP_Image_t *img)
{
    if (zone && img)
    {
        char fcc[5];
        strncpy(fcc, (char *)&img->color, sizeof(fcc));
        fcc[4] = '\0';
        DVP_PRINT(zone,
            "DVP_Image_t:%p TYPE:%d DATA[%u]{%p, %p, %p} BASE[%u]{%p, %p, %p} W,H=%u,%u dX,dY=%d,%d x0,y0=%u,%u sW,sH=%u,%u FOURCC=%4s (0x%08x) SIZE:%u RESV:%p FLUSH=%s INV=%s\n",
            img, img->memType, img->planes, img->pData[0], img->pData[1], img->pData[2],
            img->planes, img->pBuffer[0], img->pBuffer[1], img->pBuffer[2],
            img->bufWidth, img->bufHeight,
            img->x_stride, img->y_stride,
            img->x_start, img->y_start,
            img->width, img->height,
            fcc, img->color, img->numBytes,img->reserved,
            (img->skipCacheOpFlush?"FALSE":"TRUE"),
            (img->skipCacheOpInval?"FALSE":"TRUE"));
    }
}

void DVP_PrintBuffer(DVP_U32 zone, DVP_Buffer_t *buf)
{
    if (zone && buf)
    {
        DVP_PRINT(zone, "DVP_Buffer_t:%p TYPE:%d DATA:%p ELEM:%u BYTES:%u RESV:%p FLUSH=%s INV=%s\n",
                  buf,
                  buf->memType,
                  buf->pData,
                  buf->elemSize,
                  buf->numBytes,
                  buf->reserved,
                  (buf->skipCacheOpFlush?"FALSE":"TRUE"),
                  (buf->skipCacheOpInval?"FALSE":"TRUE"));
    }
}

void DVP_QuerySystem(DVP_Handle handle, const DVP_CoreInfo_t info[DVP_CORE_MAX])
{
    DVP_t *dvp = (DVP_t *)handle;
    DVP_Core_e c;
    for (c = DVP_CORE_MIN+1; c < DVP_CORE_MAX; c++)
    {
        DVP_QueryCore(dvp, c, (DVP_CoreInfo_t *)&info[c]);
    }
}

void DVP_QueryKernel(DVP_Handle handle, DVP_KernelNode_e kernel, DVP_BOOL present[DVP_CORE_MAX])
{
    DVP_t *dvp = (DVP_t *)handle;
    DVP_Core_e c;
    for (c = DVP_CORE_MIN+1; c < DVP_CORE_MAX; c++)
    {
        present[c] = DVP_QueryCoreForKernel(dvp, kernel, c);
    }
}

DVP_Error_e DVP_KernelGraphSection_Init(DVP_Handle handle,
                                      DVP_KernelGraph_t *graph,
                                      DVP_U32 sectionIndex,
                                      DVP_KernelNode_t *pNodes,
                                      DVP_U32 numNodes)
{
    DVP_t *dvp = (DVP_t *)handle;
    if (dvp && graph && sectionIndex < graph->numSections)
    {
        graph->sections[sectionIndex].pNodes = pNodes;
        graph->sections[sectionIndex].numNodes = numNodes;
        return DVP_SUCCESS;
    }
    else
    {
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR: Invalid Parameters to %s!\n", __FUNCTION__);
        return DVP_ERROR_INVALID_PARAMETER;
    }
}

void DVP_KernelGraph_Free(DVP_Handle handle, DVP_KernelGraph_t *graph)
{
    DVP_t *dvp = (DVP_t *)handle;
    if (dvp && graph)
    {
        free(graph->sections);
        free(graph->order);
        free(graph);
    }
}

DVP_KernelGraph_t *DVP_KernelGraph_Alloc(DVP_Handle handle, DVP_U32 numSections)
{
    DVP_t *dvp = (DVP_t *)handle;
    if (dvp)
    {
        DVP_KernelGraph_t *graph = (DVP_KernelGraph_t *)calloc(1, sizeof(DVP_KernelGraph_t));
        if (graph && numSections > 0)
        {
            DVP_Perf_t perf = DVP_PERF_INIT;
            graph->sections = (DVP_KernelGraphSection_t *)calloc(numSections, sizeof(DVP_KernelGraphSection_t));
            graph->numSections = numSections;
            graph->order = (DVP_U32 *)calloc(numSections, sizeof(DVP_U32));
            memcpy(&graph->totalperf, &perf, sizeof(perf));

            if (graph->sections == NULL || graph->order == NULL)
            {
                DVP_KernelGraph_Free(handle, graph);
                graph = NULL;
            }
            else
            {
                DVP_U32 s = 0;
                DVP_U32 coreLoads[DVP_CORE_MAX] = DVP_CORE_LOAD_INIT;
                for (s = 0; s < numSections; s++)
                {
                    memcpy(&graph->sections[s].perf, &perf, sizeof(perf));
                    memcpy(&graph->sections[s].coreLoad, coreLoads, sizeof(coreLoads));
                    //graph->order[s] = 0;
                }
            }
        }
        return graph;
    }
    return NULL;
}

void DVP_KernelGraph_ImageShiftAccum(DVP_Handle handle, DVP_KernelNode_t *node, dvp_image_shift_t *shift)
{
    DVP_t *dvp = (DVP_t *)handle;

    if (dvp == NULL || node == NULL || shift == NULL)
        return;

    DVP_PrintNode(DVP_ZONE_KGAPI, node);

    if (node->header.mgrIndex < DVP_CORE_MAX &&
        node->header.funcIndex <= dvp->managers[node->header.mgrIndex].numSupportedKernels)
    {
        dvp_image_shift_t *s = dvp->managers[node->header.mgrIndex].kernels[node->header.funcIndex].shift;
        dvp_image_shift_f fn = dvp->managers[node->header.mgrIndex].kernels[node->header.funcIndex].shift_func;

        DVP_PRINT(DVP_ZONE_KGAPI, "Accumulating Image Shift for %s\n",
                dvp->managers[node->header.mgrIndex].kernels[node->header.funcIndex].name);

        if (s != NULL)
        {
            shift->centerShiftHorz += s->centerShiftHorz;
            shift->centerShiftVert += s->centerShiftVert;
            shift->topBorder       += s->topBorder;
            shift->rightBorder     += s->rightBorder;
            shift->bottomBorder    += s->bottomBorder;
            shift->leftBorder      += s->leftBorder;
        }
        else if (fn != NULL)
        {
            fn(node, shift);
        }
    }
    else
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Handle, node or KGM index is invalid!\n");
    }
}

/******************************************************************************/
