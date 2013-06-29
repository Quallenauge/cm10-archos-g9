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

#include <sys/wait.h>

#include <dvp/dvp.h>
#include <dvp/dvp_debug.h>

#ifdef ANDROID
#include <binder/BinderService.h>
using namespace android;
#endif

struct ImageShare_t {
    DVP_Image_t image;
    DVP_S32 fds[DVP_MAX_PLANES];
    DVP_VALUE hdls[DVP_MAX_PLANES];
};

struct BufferShare_t {
    DVP_Buffer_t buf;
    DVP_S32 fd;
    DVP_VALUE handle;
};

#ifdef ANDROID
namespace android {

class IDVPService: public IInterface
{
public:
    DECLARE_META_INTERFACE(DVPService);
    virtual bool allocate(DVP_Image_t *pImage, DVP_S32 *fds) = 0;
    virtual bool allocate(DVP_Buffer_t *pBuffer, DVP_S32 *fd) = 0;
    virtual void teardown() = 0;
};

class DVPService :
    public BnInterface<IDVPService>,
    public BinderService<DVPService>
{
public:
    enum {
        ALLOCATEIMG,
        ALLOCATEBUFFER,
        TEARDOWN
    };
    static char const* getServiceName() { return "DVPService"; }
    DVPService();
    virtual ~DVPService();
    virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0);
    virtual bool allocate(DVP_Image_t *pImage, DVP_S32 *fds);
    virtual bool allocate(DVP_Buffer_t *pBuffer, DVP_S32 *fd);
    virtual void teardown();

private:
    DVP_Handle mDvp;
    Vector<ImageShare_t *> mImageList;
    Vector<BufferShare_t *> mBufferList;
};

class BpDVPService : public BpInterface<IDVPService>
{
public:
    BpDVPService(const sp<IBinder>& impl) : BpInterface<IDVPService>(impl) {}
    bool allocate(DVP_Image_t *pImage, DVP_S32 *fds)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDVPService::getInterfaceDescriptor());
        data.writeInt32(pImage->width);
        data.writeInt32(pImage->height);
        data.writeInt32(pImage->color);
        data.writeInt32(pImage->memType);
        remote()->transact(DVPService::ALLOCATEIMG, data, &reply);
        bool ret = reply.readInt32();
        if (ret == true) {
            for (DVP_U32 i = 0; i < pImage->planes; i++) {
                if (pImage->memType == DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED) {
                    fds[i] = reply.readInt32();
                } else {
                    fds[i] = dup(reply.readFileDescriptor());
                }
                DVP_PRINT(DVP_ZONE_ALWAYS, "Client: unmarshaled fd:%d\n", fds[i]);
            }
        }
        return ret;
    }

    bool allocate(DVP_Buffer_t *pBuffer, DVP_S32 *fd)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDVPService::getInterfaceDescriptor());
        data.writeInt32(pBuffer->numBytes);
        data.writeInt32(pBuffer->memType);
        remote()->transact(DVPService::ALLOCATEBUFFER, data, &reply);
        bool ret = reply.readInt32();
        if (ret == true) {
            if (pBuffer->memType == DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED) {
                *fd = reply.readInt32();
            } else {
                int mfd = reply.readFileDescriptor();
                DVP_PRINT(DVP_ZONE_ALWAYS, "Client: unmarshaled raw fd:%d\n", mfd);
                *fd = dup(mfd);
            }
            DVP_PRINT(DVP_ZONE_ALWAYS, "Client: unmarshaled dup fd:%d\n", *fd);
        }
        return ret;
    }

    void teardown()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDVPService::getInterfaceDescriptor());
        remote()->transact(DVPService::TEARDOWN, data, &reply);
    }
};

IMPLEMENT_META_INTERFACE(DVPService, "com.ti.test.IDVPService");

DVPService::DVPService()
{
    DVP_PRINT(DVP_ZONE_ALWAYS, "Created %s\n", __FUNCTION__);
    mDvp = 0;
}

DVPService::~DVPService()
{
    DVP_PRINT(DVP_ZONE_ALWAYS, "Destroying %s\n", __FUNCTION__);
    teardown();
}

void DVPService::teardown()
{
    if (mDvp) {

        for (size_t i = 0; i < mImageList.size(); i++) {
            ImageShare_t *img = mImageList.itemAt(i);
            for (DVP_U32 i = 0; i < img->image.planes; i++) {
                close(img->fds[i]);
            }
            DVP_Image_Free(mDvp, &img->image);
        }

        for (size_t i = 0; i < mBufferList.size(); i++) {
            BufferShare_t *buffer = mBufferList.itemAt(i);
            close(buffer->fd);
            DVP_Buffer_Free(mDvp, &buffer->buf);
        }

        DVP_KernelGraph_Deinit(mDvp);
        mDvp = 0;
    }
}

bool DVPService::allocate(DVP_Image_t *pImage, DVP_S32 *fds)
{
    DVP_PRINT(DVP_ZONE_ALWAYS, "DVPService: %s\n", __FUNCTION__);
    if (mDvp == 0) {
        mDvp = DVP_KernelGraph_Init();
    }
    if (DVP_Image_Alloc(mDvp, pImage, static_cast<DVP_MemType_e>(pImage->memType)))
    {
        DVP_PRINT(DVP_ZONE_ALWAYS, "Allocated image!\n");
        DVP_PrintImage(DVP_ZONE_ALWAYS, pImage);
        if (DVP_Image_Share(mDvp, pImage, fds))
        {
            DVP_PRINT(DVP_ZONE_ALWAYS, "Shared image - shared_fd:%d\n", fds[0]);
            return true;
        } else {
            DVP_PRINT(DVP_ZONE_ERROR, "ERROR: could not share image!\n");
            DVP_Image_Free(mDvp, pImage);
        }
    }
    DVP_PRINT(DVP_ZONE_ERROR, "Failed to allocate image!\n");
    return false;
}

bool DVPService::allocate(DVP_Buffer_t *pBuffer, DVP_S32 *fd)
{
    DVP_PRINT(DVP_ZONE_ALWAYS, "DVPService: %s\n", __FUNCTION__);
    if (mDvp == 0) {
        mDvp = DVP_KernelGraph_Init();
    }
    if (DVP_Buffer_Alloc(mDvp, pBuffer, static_cast<DVP_MemType_e>(pBuffer->memType)))
    {
        DVP_PRINT(DVP_ZONE_ALWAYS, "Allocated buffer!\n");
        DVP_PrintBuffer(DVP_ZONE_ALWAYS, pBuffer);
        if (DVP_Buffer_Share(mDvp, pBuffer, fd))
        {
            DVP_PRINT(DVP_ZONE_ALWAYS, "Shared buffer - shared_fd:%d\n", *fd);
            return true;
        } else {
            DVP_PRINT(DVP_ZONE_ERROR, "ERROR: could not share buffer!\n");
            DVP_Buffer_Free(mDvp, pBuffer);
        }
    }
    DVP_PRINT(DVP_ZONE_ERROR, "Failed to allocate buffer!\n");
    return false;
}

status_t DVPService::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
    status_t status = NO_ERROR;
    switch (code) {
        case ALLOCATEIMG:
        {
            CHECK_INTERFACE(IDVPService, data, reply);
            ImageShare_t *img = (ImageShare_t *)calloc(1, sizeof(ImageShare_t));

            uint32_t w = data.readInt32();
            uint32_t h = data.readInt32();
            uint32_t fourcc =  data.readInt32();
            uint32_t memType = data.readInt32();
            DVP_Image_Init(&img->image, w, h, fourcc);
            img->image.memType = memType;

            bool ret = allocate(&img->image, img->fds);
            reply->writeInt32(ret);

            if (ret == true) {
                for (DVP_U32 i = 0; i < img->image.planes; i++) {
                    DVP_PRINT(DVP_ZONE_ALWAYS, "DVPService: marshaling fd:%d\n", img->fds[i]);
                    if (img->image.memType == DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED) {
                        reply->writeInt32(img->fds[i]);
                    } else {
                        reply->writeFileDescriptor(img->fds[i]);
                    }
                }
                mImageList.add(img);
            } else {
                free(img);
            }

            break;
        }
        case ALLOCATEBUFFER:
        {
            CHECK_INTERFACE(IDVPService, data, reply);
            BufferShare_t *buffer = (BufferShare_t *)calloc(1, sizeof(BufferShare_t));

            uint32_t size = data.readInt32();
            uint32_t memType = data.readInt32();
            DVP_Buffer_Init(&buffer->buf, 1, size);
            buffer->buf.memType = memType;

            bool ret = allocate(&buffer->buf, &buffer->fd);
            reply->writeInt32(ret);
            if (ret == true) {
                DVP_PRINT(DVP_ZONE_ALWAYS, "DVPService: marshaling fd:%d\n", buffer->fd);
                if (buffer->buf.memType == DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED) {
                    reply->writeInt32(buffer->fd);
                } else {
                    reply->writeFileDescriptor(buffer->fd);
                }
                mBufferList.add(buffer);
            } else {
                free(buffer);
            }
            break;
        }
        case TEARDOWN:
            teardown();
            break;
        default :
            return (BBinder::onTransact(code, data, reply, flags));
    }
    return status;
}

} // namespace
#endif

static void server()
{
#ifdef ANDROID
    DVP_PRINT(DVP_ZONE_ALWAYS, "Starting DVPService!\n");
    DVPService::publishAndJoinThreadPool();
#endif
}

static uint32_t num_tests = 0;
static uint32_t num_tests_passed = 0;

static void test_image_import(const sp<IDVPService>& srv, DVP_Handle hDvp, fourcc_t fourcc, DVP_MemType_e type)
{
    num_tests++;

    ImageShare_t img;
    DVP_Image_Init(&img.image, 320, 240, fourcc);
    img.image.memType = type;

    DVP_PRINT(DVP_ZONE_ALWAYS, "\n==============");
    DVP_PRINT(DVP_ZONE_ALWAYS, "Testing importing type:%d FOURCC:(0x%08x)\n", type, fourcc);
    if (srv->allocate(&img.image, img.fds))
    {
        DVP_PRINT(DVP_ZONE_ALWAYS, "Importing memory...\n");
        if (DVP_Image_Import(hDvp, &img.image, img.fds, img.hdls)) {
            DVP_PrintImage(DVP_ZONE_ALWAYS, &img.image);
            DVP_PRINT(DVP_ZONE_ALWAYS, "Success testing import!\n");
            if(!DVP_Image_Free_Import(hDvp, &img.image, img.hdls)) {
                DVP_PRINT(DVP_ZONE_ERROR, "error freeing imported image!\n");
            } else {
                num_tests_passed++;
            }
        } else {
            DVP_PRINT(DVP_ZONE_ERROR, "Failed importing memory!\n");
        }
        for (DVP_U32 i = 0; i < img.image.planes; i++) {
            close(img.fds[i]);
        }
    } else {
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR: service failed to allocate image!\n");
    }
    DVP_PRINT(DVP_ZONE_ALWAYS, "==============\n");
}

static void test_buffer_import(const sp<IDVPService>& srv, DVP_Handle hDvp, DVP_MemType_e type)
{
    num_tests++;

    BufferShare_t b;
    DVP_Buffer_Init(&b.buf, 1, 1024);
    b.buf.memType = type;

    DVP_PRINT(DVP_ZONE_ALWAYS, "\n==============");
    DVP_PRINT(DVP_ZONE_ALWAYS, "Testing importing buffer type:%d\n", type);
    if (srv->allocate(&b.buf, &b.fd))
    {
        DVP_PRINT(DVP_ZONE_ALWAYS, "Importing buffer memory fd:%d\n", b.fd);
        if (DVP_Buffer_Import(hDvp, &b.buf, b.fd, &b.handle)) {
            DVP_PrintBuffer(DVP_ZONE_ALWAYS, &b.buf);
            DVP_PRINT(DVP_ZONE_ALWAYS, "Success testing buffer import!\n");
            if(!DVP_Buffer_Free_Import(hDvp, &b.buf, b.handle)) {
                DVP_PRINT(DVP_ZONE_ERROR, "error freeing imported buffer!\n");
            } else {
                num_tests_passed++;
            }
        } else {
            DVP_PRINT(DVP_ZONE_ERROR, "Failed importing buffer memory!\n");
        }
        close(b.fd);
    } else {
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR: service failed to allocate buffer!\n");
    }
    DVP_PRINT(DVP_ZONE_ALWAYS, "==============\n");
}

static void client(pid_t srv_pid)
{
    DVP_PRINT(DVP_ZONE_ALWAYS, "Started DVPService client...\n");
#ifdef ANDROID
    { //scope for srv strong pointer
        sp<IDVPService> srv;
        status_t err = getService(String16("DVPService"), &srv);
        if (err != NO_ERROR) {
            DVP_PRINT(DVP_ZONE_ERROR, "ERROR: failed getting DVPService interface with error=0x%x\n", err);
            return;
        }

        DVP_Handle hDvp = DVP_MemImporter_Create();
        if (hDvp) {
            test_image_import(srv, hDvp, FOURCC_Y800, DVP_MTYPE_DEFAULT);
            test_image_import(srv, hDvp, FOURCC_NV12, DVP_MTYPE_DEFAULT);

            test_image_import(srv, hDvp, FOURCC_Y800, DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED);
            test_image_import(srv, hDvp, FOURCC_NV12, DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED);
#if defined(DVP_USE_ION)
            test_image_import(srv, hDvp, FOURCC_Y800, DVP_MTYPE_MPUNONCACHED_1DTILED);
            test_image_import(srv, hDvp, FOURCC_NV12, DVP_MTYPE_MPUNONCACHED_1DTILED);

            test_image_import(srv, hDvp, FOURCC_Y800, DVP_MTYPE_MPUNONCACHED_2DTILED);
            test_image_import(srv, hDvp, FOURCC_NV12, DVP_MTYPE_MPUNONCACHED_2DTILED);
#endif
            test_buffer_import(srv, hDvp, DVP_MTYPE_DEFAULT);
            test_buffer_import(srv, hDvp, DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED);
#if defined(DVP_USE_ION)
            test_buffer_import(srv, hDvp, DVP_MTYPE_MPUNONCACHED_1DTILED);
#endif
            DVP_MemImporter_Free(hDvp);
        } else {
            DVP_PRINT(DVP_ZONE_ERROR, "Client failed creating dvp handle!\n");
        }

        srv->teardown();
    }
#endif
    if(kill(srv_pid, SIGTERM) != 0) {
        DVP_PRINT(DVP_ZONE_ERROR, "Error trying to kill DVPService\n");
    }
    DVP_PRINT(DVP_ZONE_ERROR, "Client: Waiting for DVPService to exit...\n");
    int status;
    waitpid(srv_pid, &status, 0);

    DVP_PRINT(DVP_ZONE_ERROR, "Client: DVPService exit_status(%d)\n", status);

    DVP_PRINT(DVP_ZONE_ALWAYS, "===============");
    DVP_PRINT(DVP_ZONE_ALWAYS, "Passed: %d/%d\n", num_tests_passed, num_tests);
    DVP_PRINT(DVP_ZONE_ALWAYS, "Failed: %d/%d\n", num_tests-num_tests_passed, num_tests);
    DVP_PRINT(DVP_ZONE_ALWAYS, "===============\n");
}

int main()
{
#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

    pid_t pid;
    pid = fork();
    if (pid == 0)
        server();
    else
        client(pid);
    return 0;
}
