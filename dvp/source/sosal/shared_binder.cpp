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

#include <shared_binder.h>

#ifdef SHARED_MEMORY_CLIENT
namespace android {

BpSharedMemoryClient::BpSharedMemoryClient(const sp<IBinder>& impl)
    : BpInterface<ISharedMemoryClient>(impl)
{
    SOSAL_PRINT(SOSAL_ZONE_API, "+BpSharedMemoryClient()\n");
}

BpSharedMemoryClient::~BpSharedMemoryClient()
{
    SOSAL_PRINT(SOSAL_ZONE_API, "~BpSharedMemoryClient()\n");
}

IMPLEMENT_META_INTERFACE(SharedMemoryClient, "com.ti.sosal.shm_client");

SharedMemoryClient::SharedMemoryClient()
{
    SOSAL_PRINT(SOSAL_ZONE_API, "+SharedMemoryClient()\n");
}

SharedMemoryClient::~SharedMemoryClient()
{
    SOSAL_PRINT(SOSAL_ZONE_API, "~SharedMemoryClient()\n");
}

BpSharedMemory::BpSharedMemory(const sp<IBinder>& impl)
    : BpInterface<ISharedMemory>(impl)
{
    SOSAL_PRINT(SOSAL_ZONE_API, "+BpSharedMemory()\n");
}

BpSharedMemory::~BpSharedMemory()
{
    SOSAL_PRINT(SOSAL_ZONE_API, "~BpSharedMemory()\n");
}

bool_e BpSharedMemory::allocate(shared_t **pshm)
{
    shared_t *shm = *pshm;
    if (shm)
    {
        Parcel data, reply;
        sp<SharedMemoryClient> client = new SharedMemoryClient();
        data.writeInterfaceToken(ISharedMemory::getInterfaceDescriptor());
        data.writeInt32(shm->un);
        data.writeInt64(shm->size);
        data.writeStrongBinder(client->asBinder());
        remote()->transact(SHM_SRVC_ALLOCATE, data, &reply);
        shm->fd = dup(reply.readFileDescriptor());
        if (shm->fd < 0)
        {
            free(shm);
            shm = NULL;
            return false_e;
        }
        else
        {
            // success
            shm->data = mmap(NULL, shm->size, PROT_READ|PROT_WRITE, MAP_SHARED, shm->fd, 0);
            if (shm->data == MAP_FAILED)
            {
                close(shm->fd);
                free(shm);
                shm = NULL;
                return false_e;
            }
            client->incStrong(this);
            shm->reserved = client.get();
        }
    }
    return true_e;
}

void BpSharedMemory::deallocate(shared_t **pshm)
{
    if (pshm && *pshm)
    {
        shared_t *shm = *pshm;
        munmap(shm->data, shm->size);
        close(shm->fd);
        sp<SharedMemoryClient> client = (SharedMemoryClient *)shm->reserved;
        client->decStrong(this);
        free(shm);
        *pshm = NULL;
    }
}

void BpSharedMemory::forget(shared_t **pshm)
{
    if (pshm && *pshm )
    {
        shared_t *shm = *pshm;
        Parcel data, reply;
        data.writeInterfaceToken(ISharedMemory::getInterfaceDescriptor());
        data.writeInt32(shm->un);
        sp<SharedMemoryClient> client = (SharedMemoryClient *)shm->reserved;
        data.writeStrongBinder(client);
        remote()->transact(SHM_SRVC_REMOVE, data, &reply);
        deallocate(pshm);
    }
}

IMPLEMENT_META_INTERFACE(SharedMemory, SHARED_MEMORY_SERVICE_NAME);

}

#endif // SHARED_MEMORY_CLIENT
#ifdef SHARED_MEMORY_SERVICE

#define SHARED_MEMORY_NAME_FMT  "com.ti.sosal.shm.%08x"

namespace android {

SharedMemoryEntry::SharedMemoryEntry(DefaultKeyedVector< unique_t, wp<SharedMemoryEntry> > *list)
{
    mUnique = 0;
    mSize = 0;
    mFd = 0;
    mEntries = list;
}

SharedMemoryEntry::~SharedMemoryEntry()
{
    close(mFd);
    mEntries->removeItem(mUnique);
    for (size_t i = 0; i < mClients.size(); i++) {
        sp<IBinder> client = mClients.valueAt(i).promote();
        if (client != NULL) {
            client->unlinkToDeath(this);
        }
    }
    mClients.clear();
}

status_t SharedMemoryEntry::allocate(unique_t un, size_t size, sp<IBinder>& client)
{
    char name[MAX_PATH];
    sprintf(name, SHARED_MEMORY_NAME_FMT, un);

    mFd = ashmem_create_region(name, size);
    if (mFd < 0) {
        return NO_MEMORY;
    }

    addClient(client);
    mUnique = un;
    mSize = size;

    return NO_ERROR;
}

status_t SharedMemoryEntry::addClient(sp<IBinder>& client)
{
    mClients.add(client.get(), client);
    client->linkToDeath(this);
    incStrong(this);
    return NO_ERROR;
}

status_t SharedMemoryEntry::removeClient(sp<IBinder>& client)
{
    mClients.removeItem(client.get());
    client->unlinkToDeath(this);
    decStrong(this);
    return NO_ERROR;
}

void SharedMemoryEntry::binderDied(const wp<IBinder>& who)
{
    mClients.removeItem(who.unsafe_get());
    decStrong(this);
}

status_t SharedMemoryEntry::dump(int fd, const Vector<String16>&)
{
    const size_t SIZE = 4096;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, "   [un:0x%x, size:%d, fd:%d]\n", mUnique, mSize, mFd);
    result.append(buffer);

    for (size_t i = 0; i < mClients.size(); i++) {
        const wp<IBinder>& client = mClients.valueAt(i);
        if (client != NULL) {
            snprintf(buffer, SIZE, "     - client:%p\n", client.unsafe_get());
            result.append(buffer);
        }
    }

    write(fd, result.string(), result.size());
    return NO_ERROR;
}


const String16 ISharedMemoryService::descriptor(SHARED_MEMORY_SERVICE_NAME);
ISharedMemoryService::ISharedMemoryService() {};
ISharedMemoryService::~ISharedMemoryService() {};
const String16& ISharedMemoryService::getInterfaceDescriptor() const {
    return ISharedMemoryService::descriptor;
}

const String16 ISharedMemoryClient::descriptor(SHARED_MEMORY_SERVICE_NAME);
ISharedMemoryClient::ISharedMemoryClient() {};
ISharedMemoryClient::~ISharedMemoryClient() {};
const String16& ISharedMemoryClient::getInterfaceDescriptor() const {
    return ISharedMemoryClient::descriptor;
}

void SharedMemoryService::instantiate()
{
    status_t status;
    status = defaultServiceManager()->addService(String16(SHARED_MEMORY_SERVICE_NAME), new SharedMemoryService());
}

SharedMemoryService::SharedMemoryService() {
    SOSAL_PRINT(SOSAL_ZONE_API, "+SharedMemoryService()\n");
}

SharedMemoryService::~SharedMemoryService() {

}

status_t SharedMemoryService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    status_t status = NO_ERROR;
    switch (code)
    {
        case SHM_SRVC_ALLOCATE:
        {
            CHECK_INTERFACE(SharedMemoryService, data, reply);
            unique_t un = data.readInt32();
            size_t size = data.readInt64();
            sp<IBinder> client = data.readStrongBinder();

            sp<SharedMemoryEntry> entry = mEntries.valueFor(un).promote();
            if (entry == NULL) {
                SOSAL_PRINT(SOSAL_ZONE_SHARED, "SHM SRVC: Creating new Shared Region\n");
                sp<SharedMemoryEntry> entry = new SharedMemoryEntry(&mEntries);
                status = entry->allocate(un, size, client);
                if (status == NO_ERROR) {
                    mEntries.add(un, entry);
                    reply->writeFileDescriptor(entry->fd());
                }
            } else {
                SOSAL_PRINT(SOSAL_ZONE_SHARED, "SHM SRVC: Found old Shared Region\n");
                entry->addClient(client);
                reply->writeFileDescriptor(entry->fd());
            }
            break;
        }
        case SHM_SRVC_REMOVE:
        {
            SOSAL_PRINT(SOSAL_ZONE_SHARED, "SHM SRVC: SHM_SRVC_REMOVE\n");
            CHECK_INTERFACE(SharedMemoryService, data, reply);
            unique_t un = data.readInt32();
            sp<IBinder> client = data.readStrongBinder();
            sp<SharedMemoryEntry> entry = mEntries.valueFor(un).promote();
            if (entry != NULL) {
                entry->removeClient(client);
            } else {
                status = NAME_NOT_FOUND;
            }
            break;
        }
        default:
            BBinder::onTransact(code, data, reply, flags);
            break;
    }
    return status;
}

status_t SharedMemoryService::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 4096;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, "SHM entries:\n");
    result.append(buffer);
    write(fd, result.string(), result.size());

    for (size_t i = 0; i < mEntries.size(); i++) {
        sp<SharedMemoryEntry> entry = mEntries.valueAt(i).promote();
        if (entry != NULL) {
            entry->dump(fd, args);
        }
    }

    return NO_ERROR;
}

} // end of namespace

using namespace android;

static void signal_handler(int sig)
{
    // prevent re-entry
    signal(sig, NULL);
    if (sig == SIGTERM)
    {
         // someone gave us a kill -9 or killall
         SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "SIGTERM\n");
    }
    else if (sig == SIGINT)
    {
         // someone gave us a Ctrl+C
         SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "SIGINT\n");
    }
    else if (sig == SIGABRT)
    {
         // clean up before an abort kills the process
         SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "SIGABT\n");
    }
    else if (sig == SIGBUS)
    {
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "SIGBUS\n");
    }
    // reinstall
    signal(sig, signal_handler);
}

int daemonize()
{
    int err = 0;
    pid_t child_pid;
    pid_t child_sid;

    // fork off the parent process so that we can properly be a background service
    child_pid = fork();
    if (child_pid == 0)
    {
        SOSAL_PRINT(SOSAL_ZONE_SHARED, "Child Forked\n");

        // change the file mode of the process to no access
        umask(0);

        // change to a new session so that we're no longer tied to the previous one
        child_sid = setsid();
        if (child_sid > 0)
        {
            SOSAL_PRINT(SOSAL_ZONE_SHARED, "New Session Established!\n");
            // move our working directory to root
            err = chdir("/");
            if (err == 0)
            {
                SOSAL_PRINT(SOSAL_ZONE_SHARED, "Set Working Directory to root\n");

                // install the signal handlers
                if (signal(SIGTERM, signal_handler) == SIG_ERR)
                    err = -1;
                if (signal(SIGINT,  signal_handler) == SIG_ERR)
                    err = -1;
                if (signal(SIGBUS,  signal_handler) == SIG_ERR)
                    err = -1;
                if (err == 0) {
                    SOSAL_PRINT(SOSAL_ZONE_SHARED, "Installed Signal Handlers!\n");
                }
            }
        }
        else
        {
            err = errno;
        }
    }
    else
        err = 1; // parent returns non-zero
    return err;
}

int main(int argc, char** argv)
{
    int err = 0;
    bool_e daemon = true_e;

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

    SOSAL_PRINT(SOSAL_ZONE_API, "+shm_service\n");

    if (argc > 1 && strcmp(argv[1], "--no-fork") == 0)
        daemon = false_e;

    if (daemon == true_e)
        err = daemonize();

    if (err == 0)
    {
        SharedMemoryService::instantiate();
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Starting Texas Instruments Shared Memory Service\n");
        ProcessState::self()->startThreadPool();
        IPCThreadState::self()->joinThreadPool();
    }
    else
    {
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "SHM SERVICE: Parent Shell Dying!\n");
    }
    return err;
}
#endif
