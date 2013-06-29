/*
 *  Copyright (C) 2012 Texas Instruments, Inc.
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

#include <arx_debug.h>
#include <ARXDaemon.h>

#if defined(ARX_VALGRIND_DEBUG)

#include <signal.h>
event_t g_exitEvt;
static void signal_handler(int sig)
{
    ARX_PRINT(ARX_ZONE_ALWAYS, "TI ARX Service received signal %d!\n", sig);
    event_set(&g_exitEvt);
}

#endif

using namespace android;
using namespace tiarx;

int main()
{
    ARX_PRINT(ARX_ZONE_ALWAYS, "Starting TI ARX Service!\n");
#if !defined(ARX_VALGRIND_DEBUG)
    ARXDaemon::publishAndJoinThreadPool();
#else
    event_init(&g_exitEvt, false_e);
    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        ARX_PRINT(ARX_ZONE_ERROR, "ARX Service: Failed to hook SIGTERM signal!\n");
    }

    ARXDaemon *daemon = new ARXDaemon();

    ProcessState::self()->startThreadPool();
    sp<IServiceManager> sm(defaultServiceManager());
    sm->addService(String16(daemon->getServiceName()), daemon);

    event_wait(&g_exitEvt, EVENT_FOREVER);

    //Unfortunately, the service manager does not have a way to remove a service
    //we are forcing a ref count decrease here to destroy the daemon to diminish
    //the amount of false posive memory leaks reported by valgrind
    daemon->decStrong(daemon);

    event_deinit(&g_exitEvt);

    ARX_PRINT(ARX_ZONE_ALWAYS, "Exiting TI ARX Service!\n");
#endif
}
