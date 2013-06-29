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

#include <sosal/CThreaded.h>
#include <sosal/debug.h>

extern "C" thread_ret_t ThreadWrapper(void * subclassthis)
{
    thread_ret_t ret = (thread_ret_t)-1;
    CThreaded *ct = reinterpret_cast<CThreaded *>(subclassthis);
    SOSAL_PRINT(SOSAL_ZONE_THREAD, "CThreaded::ThreadWrapper Launched Thread in Subclass %p!\n", ct);
    ret = ct->RunThread();
    SOSAL_PRINT(SOSAL_ZONE_THREAD, "CThreaded::ThreadWrapper Returned Thread in Subclass %p with code "THREAD_RET_FMT"!\n", ct, ret);
    thread_exit(ret);
}

CThreaded::CThreaded()
{
    m_handle = 0;
    m_running = false;
    mutex_init(&m_mutex);
    SOSAL_PRINT(SOSAL_ZONE_API, "+CThreaded()\n");
}

CThreaded::~CThreaded()
{
    StopThread();
    mutex_unlock(&m_mutex);
    mutex_deinit(&m_mutex);
    SOSAL_PRINT(SOSAL_ZONE_API, "~CThreaded()\n");
}

void CThreaded::Lock()
{
    mutex_lock(&m_mutex);
}

void CThreaded::Unlock()
{
    mutex_unlock(&m_mutex);
}

bool CThreaded::StartThread(void *subclassthis)
{
    if (m_handle == 0 && m_running == false)
    {
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "Starting CThreaded Derived Thread %p through Wrapper!\n", subclassthis);
        m_running = true;
        m_handle = thread_create(ThreadWrapper, subclassthis);
        if (m_handle == 0)
        {
            m_running = false;
            SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Failed to create thread!\n");
        }
        else
        {
            SOSAL_PRINT(SOSAL_ZONE_THREAD, "CThreaded Thread Started "THREAD_FMT"\n", m_handle);
        }
    }
    else
    {
        SOSAL_PRINT(SOSAL_ZONE_THREAD, "Thread "THREAD_FMT" is already started or running %s\n", m_handle, (m_running?"TRUE":"FALSE"));
    }
    return m_running;
}

bool CThreaded::StopThread()
{
    if (m_running)
    {
        m_running = false;
        if (m_handle)
        {
            SOSAL_PRINT(SOSAL_ZONE_THREAD, "Stopping CThreaded Derived Thread "THREAD_FMT"\n", m_handle);
            Shutdown();
            thread_ret_t err = thread_join(m_handle);
            SOSAL_PRINT(SOSAL_ZONE_THREAD, "Derived CThreaded joined with code "THREAD_RET_FMT"!\n",err);
            err = err; // warnings
            m_handle = 0;
        }
    }
    return !m_running;
}

void CThreaded::Shutdown()
{
    return;
}

#ifdef _TEST_

class CThreadedTest : public CThreaded {
public:
    CThreadedTest() : CThreaded()
    {
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "+CThreadedTest()\n");
    }
    ~CThreadedTest()
    {
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "-CThreadedTest()\n");
    }

    thread_ret_t RunThread()
    {
        do
        {
            Lock();
            SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "CThreadedTest RunThread!!\n");
            Unlock();
            thread_msleep(100);
        } while (m_running);
        return 0;
    }
};

uint32_t sosal_zone_mask; // declare a local mask for testing.

int main(int argc, char *argv[])
{
#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

    CThreadedTest *ctt = new CThreadedTest();
    ctt->StartThread(ctt);
    thread_msleep(500);
    ctt->StopThread();
    delete ctt;
}
#endif
