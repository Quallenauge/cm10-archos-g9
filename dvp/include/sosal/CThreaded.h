/*
 *  Copyright (C) 2009-2012 Texas Instruments, Inc.
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

#ifndef _CTHREADED_H_
#define _CTHREADED_H_

/*!
 * \file
 * \brief The C++ Wrapper around thread_t's.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/sosal.h>

/*! \brief The C function which launches the \ref CThreaded::RunThread */
extern "C" thread_ret_t ThreadWrapper(void *arg);

/*!
 * \brief This class is a wrapper around the \ref thread_t API for C++ objects.
 * \note When inheriting from CThreaded, always make it the FIRST class you subclass from,
 * otherwise the \ref CThreaded::RunThread won't work.
 * \ingroup group_threads
 */
class CThreaded {
protected:
    thread_t m_handle;      /*!< \brief The handle to the threads which runs \ref CThreaded::RunThread */
    mutex_t  m_mutex;       /*!< \brief The mutex which backs the Lock/Unlock method. */
    bool     m_running;     /*!< \brief A boolean to indicate whether the thread should be running or not. */
public:
    /*! \brief The constructor */
    CThreaded();

    /*! \brief The deconstrctor */
    virtual ~CThreaded();

    /*! \brief The lock method for protecting operations in the thread.*/
    virtual void Lock();

    /*! \brief The unlock method for protecting operations in the thread. */
    virtual void Unlock();

    /*! \brief The wrapping method which launches the \ref CThreaded::RunThread method in a thread.
     * \param [in] subclassthis The pointer to a subclass instance of CThreaded.
     */
    virtual bool StartThread(void *subclassthis);

    /*! \brief The method which causes the thread to shutdown asynchronously.
     * \note This method must be overloaded by subclasses in order to stop threads which
     * are waiting on external events or causes.
     */
    virtual void Shutdown();

    /*! \brief The interface to stop the \ref CThreaded::RunThread method. */
    virtual bool StopThread();

    /*! \brief The method which will run in it's own thread when
     * \ref CThreaded::StartThread is called.
     */
    virtual thread_ret_t RunThread() = 0;
};

#endif

