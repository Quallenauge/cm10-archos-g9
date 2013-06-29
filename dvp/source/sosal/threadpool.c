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

#include <sosal/threadpool.h>
#include <sosal/debug.h>

void threadpool_destroy(threadpool_t *pool)
{
    if (pool)
    {
        uint32_t i;
        for (i = 0; i < pool->numWorkers; i++)
        {
            queue_pop(pool->workers[i].queue);
            thread_join(pool->workers[i].handle);
            profiler_stop(&pool->workers[i].perf);
            SOSAL_PRINT(SOSAL_ZONE_THREAD, "Worker %u Thread "THREAD_FMT" took %lf sec to die\n", pool->workers[i].index, pool->workers[i].handle,  rtimer_to_sec(pool->workers[i].perf.tmpTime));
            pool->workers[i].handle = 0;
            queue_destroy(pool->workers[i].queue);
            pool->workers[i].queue = NULL;
            free(pool->workers[i].data);
            pool->workers[i].data = NULL;
        }
        free(pool->workers);
        pool->workers = NULL;
        semaphore_delete(&pool->sem);
        event_deinit(&pool->completed);
        free(pool);
    }
}

static thread_ret_t threadpool_worker(void *arg)
{
    threadpool_worker_t *worker = (threadpool_worker_t *)arg;

    profiler_stop(&worker->perf);

    SOSAL_PRINT(SOSAL_ZONE_THREAD, "Worker %u Thread "THREAD_FMT" running! (Launch took %lf sec)\n", worker->index, worker->handle,  rtimer_to_sec(worker->perf.avgTime));

    thread_nextaffinity();

    profiler_clear(&worker->perf);
    profiler_start(&worker->perf);

    while (queue_read(worker->queue, true_e, worker->data) == true_e)
    {
        worker->active = true_e;
        profiler_stop(&worker->perf);
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "Worker %u Thread "THREAD_FMT" took %lf sec to get message\n", worker->index, worker->handle, rtimer_to_sec(worker->perf.tmpTime));
        worker->function(worker); // <=== WORK IS DONE HERE
        semaphore_wait(&worker->pool->sem);
        worker->pool->numCurrentItems--;
        if (worker->pool->numCurrentItems <= 0)
            event_set(&worker->pool->completed);
        semaphore_post(&worker->pool->sem);
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "Worker %u Thread "THREAD_FMT" completed work, %u items left\n", worker->index, worker->handle, worker->pool->numCurrentItems);
        profiler_start(&worker->perf);
        worker->active = false_e;
    }
    SOSAL_PRINT(SOSAL_ZONE_THREAD, "Worker %u Thread "THREAD_FMT" exitting!\n", worker->index, worker->handle);
    thread_exit(0);
}

threadpool_t *threadpool_create(uint32_t numThreads, uint32_t numWorkItems, size_t sizeWorkItem, threadpool_f worker, void *arg)
{
    threadpool_t *pool = (threadpool_t *)calloc(1, sizeof(threadpool_t));
    if (pool)
    {
        uint32_t i;
        semaphore_create(&pool->sem, 1, false_e);
        pool->numWorkers = numThreads;
        pool->numWorkItems = numWorkItems;
        pool->sizeWorkItem = sizeWorkItem;
        event_init(&pool->completed, false_e);
        pool->workers = (threadpool_worker_t *)calloc(pool->numWorkers, sizeof(threadpool_worker_t));
        if (pool->workers)
        {
            for (i = 0; i < pool->numWorkers; i++)
            {
                pool->workers[i].data = calloc(1, sizeWorkItem);
                pool->workers[i].queue = queue_create(numWorkItems, sizeWorkItem);
                pool->workers[i].index = i;
                pool->workers[i].arg = arg;
                pool->workers[i].function = worker;
                pool->workers[i].pool = pool; // back reference to top level info
                profiler_clear(&pool->workers[i].perf);
                profiler_start(&pool->workers[i].perf);
                pool->workers[i].handle = thread_create(threadpool_worker, &pool->workers[i]);
            }
        }
    }
    return pool;
}

bool_e threadpool_issue(threadpool_t *pool, void *workitems[], uint32_t numWorkItems)
{
    uint32_t i;
    bool_e wrote = false_e;

    SOSAL_PRINT(SOSAL_ZONE_THREAD, "About to issue %u workitems!\n", numWorkItems);
    semaphore_wait(&pool->sem);
    event_reset(&pool->completed); // we're going to have items to work on, so clear the event
    for (i = 0; i < numWorkItems; i++)
    {
        uint32_t index = 0xFFFFFFFF;
        uint32_t count = 0; // cycle-detector
        // issue the work to the next worker, but don't wait if it's full
        do {
            count++;
            index = pool->nextWorkerIndex;
            pool->nextWorkerIndex = (pool->nextWorkerIndex + 1) % pool->numWorkers;
            pool->numCurrentItems++;
            wrote = queue_write(pool->workers[index].queue, false_e, workitems[i]);
            if (wrote == false_e) {
                pool->numCurrentItems--;
            }
            SOSAL_PRINT(SOSAL_ZONE_THREAD, "Issued workitem %p to worker %u (%s)\n", workitems[i], index, (wrote?"TRUE":"FALSE"));
        } while (wrote == false_e && count < pool->numWorkers);
        // there's too much work to do, there's an overflow condition. some of the work may have been issued, others not.
        if (wrote == false_e)
            break;
    }

    // if none of the writes worked for whatever reason, and we didn't have any previous pending work (how?) then set the completed event
    if (pool->numCurrentItems == 0)
        event_set(&pool->completed);

    semaphore_post(&pool->sem);
    return wrote;
}

bool_e threadpool_complete(threadpool_t *pool, bool_e blocking)
{
    if (blocking)
    {
        return event_wait(&pool->completed, EVENT_FOREVER);
    }
    else
    {
        if (pool->numCurrentItems == 0)
            return true_e;
        else
            return false_e;
    }
}

typedef struct _threadpool_data_t {
    uint32_t dummy;
} threadpool_data_t;

typedef struct _threadpool_test_t {
    semaphore_t sem;
    uint32_t count;
} threadpool_test_t;

static bool_e threadpool_test(threadpool_worker_t *worker)
{
    threadpool_test_t *test = (threadpool_test_t *)worker->arg;
#if defined(SOSAL_DEBUG)
    threadpool_data_t *data = (threadpool_data_t *)worker->data;
#endif
    semaphore_wait(&test->sem);
    test->count++;
    SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "\t[%u] Worker %u Thread "THREAD_FMT" Dummy Data %u\n", test->count, worker->index, worker->handle, data->dummy);
    semaphore_post(&test->sem);
    return true_e;
}

#ifdef THREADPOOL_TEST
uint32_t sosal_zone_mask; // declare a local version for testing
int main(int argc, char *argv[])
#else
bool_e threadpool_unittest(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
#endif
{
    threadpool_t *pool = NULL;
    threadpool_test_t test;
    uint32_t numWorkers = 5;
    threadpool_data_t dummydata[2][10] = {
        { {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}},
        {{10},{11},{12},{13},{14},{15},{16},{17},{18},{19}},
    };
    void *workitems[10];
    uint32_t i,j;

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

    // initialize test data
    semaphore_create(&test.sem, 1, false_e);
    test.count = 0;

    SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Creating %u workers, for "FMT_CONST" workitems, with "FMT_CONST" queue space each\n", numWorkers, dimof(workitems), dimof(workitems)/numWorkers);

    // create the pool
    pool = threadpool_create(numWorkers, dimof(workitems)/numWorkers, sizeof(threadpool_data_t), threadpool_test, &test);

    // let the threads startup and get attached to their queues.
    // thread_msleep(100);

    for (j = 0; j < 2; j++)
    {
        for (i = 0; i < dimof(dummydata[j]); i++)
            workitems[i] = &dummydata[j][i];

        if (threadpool_issue(pool, workitems, dimof(workitems)) == true_e)
        {
            if (threadpool_complete(pool, true_e) == true_e)
            {
                semaphore_wait(&test.sem);
                SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Work %u Complete!\n", j);
                semaphore_post(&test.sem);
            }
        }
    }
    threadpool_destroy(pool);
    semaphore_delete(&test.sem);
#ifdef THREADPOOL_TEST
    return 0;
#else
    return true_e;
#endif
}
