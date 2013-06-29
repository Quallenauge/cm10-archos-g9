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

#ifndef _SHARED_MEMORY_SERVICE_
#define _SHARED_MEMORY_SERVICE_

#include <cutils/ashmem.h>
#include <sys/stat.h>
#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <utils/Timers.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include <sosal/shared.h>
#include <sosal/list.h>
#include <sosal/debug.h>

namespace android {

#define SHARED_MEMORY_SERVICE_NAME  "com.ti.sosal.shm_service"

/* --- Abstract Client Interface --- */

class ISharedMemory: public IInterface
{
public:
    DECLARE_META_INTERFACE(SharedMemory);
    virtual bool_e allocate(shared_t **pshm) = 0;
    virtual void deallocate(shared_t **pshm) = 0;
    virtual void forget(shared_t **pshm) = 0;
};

class ISharedMemoryClient: public IInterface
{
public:
    DECLARE_META_INTERFACE(SharedMemoryClient);
};

/* --- Client side --- */

class BpSharedMemory : public BpInterface<ISharedMemory>
{
public:
    BpSharedMemory(const sp<IBinder>& impl);
    ~BpSharedMemory();
    bool_e allocate(shared_t **pshm);
    void deallocate(shared_t **pshm);
    void forget(shared_t **pshm);
};

class SharedMemoryClient : public BnInterface<ISharedMemoryClient>
{
public:
    SharedMemoryClient();
    virtual ~SharedMemoryClient();
};

/* --- Abstract Service Interface --- */

class ISharedMemoryService : public IInterface
{
public:
    DECLARE_META_INTERFACE(SharedMemoryService);
};

typedef enum _shared_memory_service_code_e {
    SHM_SRVC_ALLOCATE,    /**< Used to allocate a new shared region or open an existing shared region */
    SHM_SRVC_REMOVE,      /**< Used to ask the service to close a shared region. This will only actually close once all clients close their local file descriptors */
} SharedMemoryServiceCode_e;

/* --- Server side --- */

class SharedMemoryEntry : public IBinder::DeathRecipient
{
public:
    SharedMemoryEntry(DefaultKeyedVector< unique_t, wp<SharedMemoryEntry> > *list);
    ~SharedMemoryEntry();

    status_t allocate(unique_t un, size_t size, sp<IBinder>& client);
    status_t addClient(sp<IBinder>& client);
    status_t removeClient(sp<IBinder>& client);
    void binderDied(const wp<IBinder>& who);

    int fd() { return mFd; }

    status_t dump(int fd, const Vector<String16>& args);
private:
    unique_t mUnique;
    size_t mSize;
    int mFd;
    DefaultKeyedVector< IBinder *, wp<IBinder> > mClients;
    DefaultKeyedVector< unique_t, wp<SharedMemoryEntry> > *mEntries;
};

class BpSharedMemoryClient : public BpInterface<ISharedMemoryClient>
{
public:
    BpSharedMemoryClient(const sp<IBinder>& impl);
    ~BpSharedMemoryClient();
};

class BnSharedMemoryService: public BnInterface<ISharedMemoryService>
{
};

class SharedMemoryService : public BnSharedMemoryService
{
public:
    SharedMemoryService();
    virtual ~SharedMemoryService();
    // Binder Interfaces
    static void instantiate();
    virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0);
    String16& getInterfaceDescriptor();
    status_t dump(int fd, const Vector<String16>& args);

private:
    DefaultKeyedVector< unique_t, wp<SharedMemoryEntry> > mEntries;
};

}; // namespace android

#endif /* _SHARED_MEMORY_SERVICE_ */
