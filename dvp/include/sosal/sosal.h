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

#ifndef _SOSAL_H_
#define _SOSAL_H_

/*!
 * \file
 * \brief The top level Simple Operating System Abstraction Layer header.
 * \author Erik Rainey <erik.rainey@ti.com>
 * \defgroup group_threadpools SOSAL Threadpool
 * \defgroup group_rtimers SOSAL RTimer
 * \defgroup group_image_t SOSAL Image
 * \defgroup group_threads SOSAL Thread
 * \defgroup group_modules SOSAL Module
 * \defgroup group_mutexes SOSAL Mutex
 * \defgroup group_serial SOSAL Serial Ports
 * \defgroup group_events SOSAL Events
 * \defgroup group_semaphores SOSAL Semaphore
 * \defgroup group_uinput SOSAL User Input
 * \defgroup group_lists SOSAL List
 * \defgroup group_rings SOSAL Ring
 * \defgroup group_queues SOSAL Queue
 * \defgroup group_hashes SOSAL Hash
 * \defgroup group_options SOSAL Command-line Options
 * \defgroup group_cache SOSAL Cache
 * \defgroup group_vectors SOSAL Vector
 * \defgroup group_bitfields SOSAL Bitfield
 * \defgroup group_fph SOSAL Fast Pointer Hash
 * \defgroup group_ini SOSAL INI Parser
 * \defgroup group_profilers SOSAL Profiler
 * \defgroup group_sockets SOSAL Sockets
 * \defgroup group_rpc_sockets SOSAL RPC Sockets
 * \defgroup group_shared SOSAL Shared Memory
 * \defgroup group_pipes SOSAL Pipe
 * \defgroup group_btreelists SOSAL Binary Tree List
 * \defgroup group_allocators SOSAL Allocators
 * \defgroup group_pyuv SOSAL PYUV
 * \defgroup group_sosal_debug SOSAL Debugging
 * \defgroup group_fourcc SOSAL FOURCC Codes
 * \defgroup group_unittest SOSAL Unit Tests
 * \defgroup group_histograms SOSAL Histogram
 * \defgroup group_heaps SOSAL Heaps
 * \defgroup group_sosal_pool SOSAL Pool Memory
 */
#include <sosal/types.h>
#include <sosal/status.h>
#include <sosal/rtimer.h>
#include <sosal/fourcc.h>
#include <sosal/image.h>
#include <sosal/thread.h>
#include <sosal/module.h>
#include <sosal/mutex.h>
#include <sosal/serial.h>
#include <sosal/event.h>
#include <sosal/semaphore.h>
#include <sosal/uinput.h>
#include <sosal/list.h>
#include <sosal/ring.h>
#include <sosal/queue.h>
#include <sosal/hash.h>
#include <sosal/options.h>
#include <sosal/cache.h>
#include <sosal/vector.h>
#include <sosal/bitfield.h>
#include <sosal/fph.h>
#include <sosal/ini.h>
#include <sosal/profiler.h>
#include <sosal/socket.h>
#include <sosal/rpc_socket.h>
#include <sosal/shared.h>
#include <sosal/threadpool.h>
#include <sosal/pipe.h>
#include <sosal/btreelist.h>
#include <sosal/allocator.h>
#include <sosal/pyuv.h>

#if defined(ANDROID)
#include <sosal/allocator_binder.h>
#endif

#ifdef SOSAL_UNITTEST
#include <sosal/unittest.h>
#endif

#endif // _SOSAL_H_
