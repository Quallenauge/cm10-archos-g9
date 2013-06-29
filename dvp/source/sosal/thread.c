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

#include <time.h>
#ifdef SYSBIOS
#include <sosal/mutex.h>
//#include <sosal/allocator.h>  //default bios malloc,... used
//#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#endif
#include <sosal/thread.h>
#include <sosal/event.h> // unit test
#include <sosal/queue.h> // unit test
#include <sosal/options.h> // unit test
#include <sosal/debug.h>

#if defined(ANDROID)
#include <linux/unistd.h>
#include <sys/syscall.h>
int sched_setaffinity(int pid, size_t len, unsigned long *cpusetp)
{
    return syscall(__NR_sched_setaffinity, pid, len, cpusetp);
}
#elif defined(LINUX)
#include <sched.h>
#endif

#ifdef POSIX
static bool_e f_initialized = false_e;
#endif

#if defined(POSIX) || defined(ANDROID) || defined(LINUX)
static mutex_t f_lock;
#endif

void thread_nextaffinity()
{
#if defined(ANDROID)
    static unsigned long cpu_mask = ((1 << TARGET_NUM_CORES) - 1);
    static unsigned long cpu_inc = 1;
    unsigned long mask;
    mutex_lock(&f_lock);
    mask = 0;
    mask = cpu_inc;
    cpu_inc = (cpu_inc + 1) & cpu_mask;
    if (cpu_inc == 0)
        cpu_inc = 1;
    sched_setaffinity(gettid(), sizeof(mask), &mask);
    mutex_unlock(&f_lock);
#elif defined(LINUX)
    static unsigned long cpu_mask = ((1 << TARGET_NUM_CORES) - 1);
    static unsigned long cpu_inc = 1;
    unsigned long s;
    cpu_set_t mask;
    mutex_lock(&f_lock);
    CPU_ZERO(&mask);
    s = CPU_SET(cpu_inc, &mask);
    cpu_inc = (cpu_inc + 1) & cpu_mask;
    if (cpu_inc == 0)
        cpu_inc = 1;
    sched_setaffinity(getpid(), sizeof(mask), &mask);
    mutex_unlock(&f_lock);
#endif
}

#ifdef SYSBIOS
#ifndef JOIN_SEMAPHORE
ack_t* sosal_add_ack(thread_t  pThrdHdl)
{
    ack_t* pNew;

    pNew = malloc(sizeof(ack_t));
    if(!pNew) {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Can't allocate memory for new join ACK!\n");
        return NULL;
    }

    pNew->next = pThrdHdl->pAckList;
    pThrdHdl->pAckList = pNew;

    pNew->exit_event_ack = malloc(sizeof(event_t));
    if(!pNew->exit_event_ack) {
        free(pNew);
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Can't allocate memory for new join ACK event!\n");
        return NULL;
    }

    event_init(pNew->exit_event_ack, false_e);
    return (pNew);
}

void sosal_free_ack(ack_t *pAck)
{
    event_deinit(pAck->exit_event_ack);
    free(pAck->exit_event_ack);
    free(pAck);
}
#endif

void thread_main_func_translate(uint32_t argc, void* argv)
{
    thread_t  sosal_thread_hdl = argv;

    /* Run client main func */
    sosal_thread_hdl->exit_code = sosal_thread_hdl->client_func(sosal_thread_hdl->client_arg);

    sosal_thread_hdl->bTaskReady = true_e;
#ifdef JOIN_SEMAPHORE
    {
        semaphore_t         *semaphore_hdl;

        semaphore_hdl = sosal_thread_hdl->join_semaphore;
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "thread_main_func_translate semaphore post (%d)\n", sosal_thread_hdl->join_number);
        while(sosal_thread_hdl->join_number) {
            semaphore_post(semaphore_hdl);  /* signal next join task in the queue to continue */
            sosal_thread_hdl->join_number--;
        }
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "thread_main_func_translate semaphore delete (%d)\n", sosal_thread_hdl->join_number);
        semaphore_delete(semaphore_hdl);
        mutex_deinit(sosal_thread_hdl->join_mutex);
    }
    /* extra task memory releasing */
    free(sosal_thread_hdl->join_semaphore);
    sosal_thread_hdl->join_semaphore = NULL;
    free(sosal_thread_hdl->join_mutex);
#else
    /* Event mode signaling */
    //SOSAL_PRINT(SOSAL_ZONE_THREAD, "thread_main_func_translate send ready event\n");
    event_set(sosal_thread_hdl->exit_event);

    // Wait all task to receive join event
    while (sosal_thread_hdl->pAckList) {
        ack_t     *pAck;
        pAck = sosal_thread_hdl->pAckList;
        //SOSAL_PRINT(SOSAL_ZONE_THREAD, "thread_main_func_translate waits for ack\n");
        event_wait(pAck->exit_event_ack, EVENT_FOREVER);
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "thread_main_func_translate ack received\n");
        sosal_thread_hdl->pAckList = pAck->next;
        sosal_free_ack(pAck);
    }
    event_deinit(sosal_thread_hdl->exit_event);
    //SOSAL_PRINT(SOSAL_ZONE_THREAD, "thread_main_func_translate event deinited\n");

    /* extra task memory releasing */
    free(sosal_thread_hdl->exit_event);
#endif


    free(sosal_thread_hdl);
    sosal_thread_hdl = NULL;
    SOSAL_PRINT(SOSAL_ZONE_THREAD, "thread_main_func_translate clean up\n");
}

/* TODO: an alternative for task killing is to add task delete function and hook in XDC
void thread_delete(thread_t *sosal_thread_hdl)
{

    Task_delete(&((*sosal_thread_hdl)->thread_hdl));
    (*sosal_thread_hdl)->thread_hdl = NULL;

    free((*sosal_thread_hdl)->exit_event);
    free((*sosal_thread_hdl)->join_semaphore);
    // free((*sosal_thread_hdl)->stack_ptr);
    free((*sosal_thread_hdl));
}

void exitFxn(Task_Handle task)
{
    thread_t    sosal_thread_hdl;

    sosal_thread_hdl = task->arg1;

    thread_delete(sosal_thread_hdl);
}
*/
#endif

thread_t thread_create(thread_f func, void * arg)
{
#ifdef POSIX
    thread_t handle = 0;
    pthread_t p;

    if (f_initialized == false_e)
    {
        mutex_init(&f_lock);
        f_initialized = true_e;
    }

    int err = pthread_create(&p, NULL, func, arg);
    if (err == EINVAL) {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Invalid settings in pthread_attr\n");
    } else if (err == EAGAIN) {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Insufficient resources to start thread!\n");
    }
    handle = p;
    return handle;
#elif defined(SYSBIOS)
    thread_t            sosal_thread_hdl;
#ifdef JOIN_SEMAPHORE
    semaphore_t         *semaphore_hdl;
    mutex_t             *mutex_hdl;
#else
    event_t             *event_hdl;
#endif
    /* void                *stack_ptr; */
    bool_e               ret_val;
    Task_Params          taskParams;
    long                 pid = 0;

    sosal_thread_hdl = malloc(sizeof(*sosal_thread_hdl));
    if(!sosal_thread_hdl) {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Can't allocate memory for thread SOSAL hdl!\n");
        return NULL;
    }

#ifdef JOIN_SEMAPHORE
    semaphore_hdl = malloc(sizeof(semaphore_t));
    if(!semaphore_hdl) {
        free(sosal_thread_hdl);
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Can't create join semaphore!\n");
        return NULL;
    }

    ret_val = semaphore_create(semaphore_hdl, 0, (bool_e)0);
    if(!ret_val) {
        free(semaphore_hdl);
        free(sosal_thread_hdl);
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Can't create exit event!\n");
        return NULL;
    }

    sosal_thread_hdl->join_semaphore = semaphore_hdl;

    mutex_hdl = malloc(sizeof(mutex_t));
    if(!mutex_hdl) {
        free(semaphore_hdl);
        free(sosal_thread_hdl);
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Can't create join semaphore!\n");
        return NULL;
    }

    mutex_init(mutex_hdl);

    sosal_thread_hdl->join_mutex = mutex_hdl;
    sosal_thread_hdl->join_number = 0;
#else
    event_hdl = malloc(sizeof(event_t));
    if(!event_hdl) {
        free(sosal_thread_hdl);
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Can't allocate memory for exit event!\n");
        return NULL;
    }

/* TODO: It isn't necessary to allocate stack memory
    stack_ptr = malloc(SYSBIOS_DEFAULT_STASK_SIZE);
    if(!stack_ptr) {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Can't allocate memory for thread stack!\n");
        free(sosal_thread_hdl->exit_event);
        free(sosal_thread_hdl);
        return NULL;
    }
    sosal_thread_hdl->stack_ptr = stack_ptr;
*/
    ret_val = event_init(event_hdl, false_e);
    if(!ret_val) {
        /* free(sosal_thread_hdl->stack_ptr); */
        free(event_hdl);
        free(sosal_thread_hdl);
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Can't create exit event!\n");
        return NULL;
    }

    sosal_thread_hdl->exit_event = event_hdl;
    sosal_thread_hdl->pAckList = NULL;
#endif

    sosal_thread_hdl->client_arg = arg;
    sosal_thread_hdl->client_func = func;

    Task_Params_init(&taskParams);

    taskParams.arg0 = 1;
    taskParams.arg1 = (UArg)(sosal_thread_hdl);
    taskParams.env = NULL;
    taskParams.priority = SYSBIOS_DEFAULT_PRIORITY;
    /* taskParams.stack = (void *)stack_ptr;*/
    taskParams.stackSize = SYSBIOS_DEFAULT_STASK_SIZE;
    taskParams.instance->name = NULL;

    pid = (long) Task_getEnv(Task_self());
    taskParams.env = (void *) pid;

    sosal_thread_hdl->thread_hdl = Task_create((Task_FuncPtr)thread_main_func_translate, &taskParams, NULL);
    sosal_thread_hdl->bTaskReady = false_e;
    return sosal_thread_hdl;
#else
    thread_t handle = 0;
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, CREATE_SUSPENDED, NULL);
    if (hThread)
    {
        ResumeThread(hThread);
    }
    handle = hThread;
    return handle;
#endif
}

void thread_return(thread_ret_t r)
{
#ifdef POSIX
    pthread_exit(r);
#endif
}

thread_ret_t thread_join(thread_t t)
{
    thread_ret_t r = 0;
#ifdef POSIX
    int err = pthread_join(t, &r);
    SOSAL_PRINT(SOSAL_ZONE_THREAD, "thread_join("THREAD_FMT") returned %d ("THREAD_RET_FMT")\n", t, err, r);
    if (err == EINVAL) {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Invalid Thread Handle or Thread is not joinable or another thread is joining with this thread\n");
    } else if (err == ESRCH) {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! No Thread with that Handle\n");
    } else if (err == EDEADLK) {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Thread Deadlock Detected!\n");
    }
#ifndef ANDROID
    if (r == PTHREAD_CANCELED) {
        SOSAL_PRINT(SOSAL_ZONE_WARNING, "WARNING! Thread was cancelled!\n");
    }
#endif
#elif defined(SYSBIOS)

    if (t != NULL) {
        if (t->bTaskReady == false_e) {
#ifdef JOIN_SEMAPHORE
            semaphore_t         *semaphore_hdl;
            mutex_t             *mutex_hdl;
            semaphore_hdl = t->join_semaphore;
            mutex_hdl = t->join_mutex;
            mutex_lock(mutex_hdl);
            t->join_number++;
            mutex_unlock(mutex_hdl);

            SOSAL_PRINT(SOSAL_ZONE_THREAD, "thread_join waits for semaphore\n");
            semaphore_wait(semaphore_hdl);
            r = t->exit_code;
#else
            ack_t   *pAck;
            SOSAL_PRINT(SOSAL_ZONE_THREAD, "thread_join add ack\n");
            pAck = sosal_add_ack(t);
            SOSAL_PRINT(SOSAL_ZONE_THREAD, "thread_join start to wait for ack\n");
            event_wait(t->exit_event, EVENT_FOREVER);
            r = t->exit_code;
            SOSAL_PRINT(SOSAL_ZONE_THREAD, "thread_join got ack about TaskReady and raise event for ack\n");
            event_set(pAck->exit_event_ack);
#endif
        } else {
            r = t->exit_code;
            SOSAL_PRINT(SOSAL_ZONE_THREAD, "thread_join TaskReady found\n");
        }
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "thread_join ready exitcode=%d\n", r);
    } else {
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "thread_join ERROR - null task\n");
        r = false_e;
    }
#else
    DWORD status = WaitForSingleObject(t, INFINITE);
    if (status == WAIT_OBJECT_0) {
        GetExitCodeThread(t, &r);
    } else if (status == ERROR_INVALID_HANDLE) {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Invalid Thread Handle\n");
    }
#endif
    return r;
}

void thread_msleep(int milliseconds)
{
#ifdef POSIX
    struct timespec time_spec;
    struct timespec remainder;
    int err = 0;
    time_spec.tv_sec = milliseconds / 1000;
    time_spec.tv_nsec = (milliseconds % 1000) * 1000000;
    do {
        //SOSAL_PRINT(SOSAL_ZONE_THREAD, "Sleeping until %lu sec:%lu ns\n",time_spec.tv_sec, time_spec.tv_nsec);
        err = nanosleep(&time_spec, &remainder);
        if (err == EINTR) {
            SOSAL_PRINT(SOSAL_ZONE_WARNING, "WARNING! Thread was interrupted during sleep, need to sleep for %lu more ns\n", remainder.tv_nsec);
            memcpy(&time_spec, &remainder, sizeof(struct timespec));
            memset(&remainder, 0, sizeof(struct timespec));
        }
        else if (err != 0)
        {
            SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Nanosleep returned %d\n", err);
        }
    } while (err == EINTR);

#elif defined(SYSBIOS)
    /*
     *  Don't let nonzero timeout round to 0 - semantically very
     *  different
     */
    uint32_t bios6_timeout;

    /* convert to us and transfer to ticks (Clock_tickPeriod in us) */
    bios6_timeout = (milliseconds*1000) / Clock_tickPeriod;
    /*
     *  Don't let nonzero timeout round to 0 - semantically very
     *  different
     */
    if (milliseconds && (!bios6_timeout)) {
        bios6_timeout = 1;
    }
    Task_sleep(bios6_timeout);
#else
    Sleep(milliseconds);
#endif
}

static const thread_ret_t TEST_RET_VALUE = (thread_ret_t)42;

static thread_ret_t thread_test(void *arg)
{
    uint32_t *pCounter = (uint32_t *)arg;
    do
    {
//    	SOSAL_PRINT(SOSAL_ZONE_THREAD, "main:thread_test is running back %d pass!\n", *pCounter);
        thread_msleep(100);
    } while ((*pCounter)--);
    return TEST_RET_VALUE;
}

typedef struct _pingpongevent_t {
    event_t ping;
    event_t pong;
} pingpongevent_t;

static thread_ret_t thread_test_events(void *arg)
{
    pingpongevent_t *p = (pingpongevent_t *)arg;

    event_set(&p->ping);

    //SOSAL_PRINT(SOSAL_ZONE_THREAD, "Thread is waiting for event!\n");
    if (event_wait(&p->pong, 500))
    {
        //SOSAL_PRINT(SOSAL_ZONE_THREAD, "Thread received event!\n");
        return TEST_RET_VALUE;
    }
    else
    {
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "ERROR! Thread TIMEDOUT!!\n");
    }
    return 0;
}

typedef struct _msg_t {
    value_t code;
} msg_t;

typedef struct _msg_thread_t {
    queue_t *msgs;
    bool_e   running;
    event_t  signal;
} msg_thread_t;

static thread_ret_t thread_test_msgs(void *arg)
{
    msg_thread_t *msgt = (msg_thread_t *)arg;
    msg_t msg;
    event_set(&msgt->signal);
    while (msgt->running)
    {
        //SOSAL_PRINT(SOSAL_ZONE_THREAD, "Waiting for queue read!\n");
        if (queue_read(msgt->msgs, true_e, &msg))
        {
            //SOSAL_PRINT(SOSAL_ZONE_THREAD, "Msg: %u\n", msg.code);
            if (msg.code == (value_t)TEST_RET_VALUE)
                msgt->running = false_e;
        }
        else
        {
            SOSAL_PRINT(SOSAL_ZONE_THREAD, "ERROR! Failed to read from queue!\n");
            msgt->running = false_e;
        }
    }
    return (thread_ret_t)msg.code;
}

bool_e thread_unittest(int argc, char *argv[])
{
    uint32_t i, counter = 10;
    pingpongevent_t events;
    int numErrors = 0;
    bool_e verbose = false_e;
    option_t hash_opts[] = {
        {OPTION_TYPE_BOOL, &verbose, sizeof(bool_e), "-v", "--verbose", "Used to print out debugging information"},
    };
    msg_t msgs[] = {{3}, {9283}, {27}, {42}}; // last value == TEST_RET_VALUE
    msg_thread_t msgt;
    thread_t t;
    thread_ret_t r;

    option_process(argc, argv, hash_opts, dimof(hash_opts));

    if (verbose) {
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "===THREAD TEST===\n");
    }

    // simple start, stop test
    t = thread_create(thread_test, &counter);
    r = thread_join(t);
    if (r != TEST_RET_VALUE)
        numErrors++;

    if (verbose) {
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "===PING PONG TEST===\n");
    }

    // event test across threads
    event_init(&events.ping, false_e);
    event_init(&events.pong, false_e);

    t = thread_create(thread_test_events, &events);
    if (verbose) {
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "Test is waiting for thread to wakeup!\n");
    }
    if (event_wait(&events.ping, 500))
    {
        if (verbose)
            SOSAL_PRINT(SOSAL_ZONE_THREAD, "Received Event from Thread!\n");
    }
    else
    {
        if (verbose)
            SOSAL_PRINT(SOSAL_ZONE_THREAD, "ERROR! Thread Message Timedout!\n");
    }
    if (verbose) SOSAL_PRINT(SOSAL_ZONE_THREAD, "Sending Event to Thread!\n");

    event_set(&events.pong);

    r = thread_join(t);
    SOSAL_PRINT(SOSAL_ZONE_THREAD, "=== THREAD JOIN exit = "THREAD_RET_FMT" ===\n", r);
    if (r != TEST_RET_VALUE)
        numErrors++;
    event_deinit(&events.ping);
    event_deinit(&events.pong);

    if (verbose) {
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "===QUEUE TEST===\n");
    }

    msgt.msgs = queue_create(2, sizeof(msg_t));
    msgt.running = true_e;
    event_init(&msgt.signal, false_e);
    t = thread_create(thread_test_msgs, &msgt);
    if (verbose)
    {
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "Waiting for thread to start!\n");
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "Thread Started?(%s!)\n", (event_wait(&msgt.signal, 1000)?"true":"false"));
    }
    for (i = 0; i < dimof(msgs); i++) {
        if (queue_write(msgt.msgs, true_e, &msgs[i]) == false_e) {
            SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Failed to write to queue!\n");
        } else {
            if (verbose) SOSAL_PRINT(SOSAL_ZONE_THREAD, "Wrote message "FMT_SIZE_T" to the Queue!\n", msgs[i].code);
        }
    }
    r = thread_join(t);
    SOSAL_PRINT(SOSAL_ZONE_THREAD, "=== THREAD JOIN exit = "THREAD_RET_FMT" ===\n", r);
    queue_destroy(msgt.msgs);
    event_deinit(&msgt.signal);

    if (verbose) {
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "===THREAD TEST EXIT Errors=%d ===\n", numErrors);
    }

    if (numErrors > 0)
        return false_e;
    else
        return true_e;
}

