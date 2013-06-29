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

#include <buffer/ImageBuffer.h>
#include <buffer/IImageBufferMgr.h>
#include <buffer/BnImageBufferMgr.h>

#if defined(JELLYBEAN) || defined(JELLYBEANMR1)
#include <gui/ISurface.h>
#include <gui/Surface.h>
#else
#include <surfaceflinger/ISurface.h>
#include <surfaceflinger/Surface.h>
#endif
#include <binder/Parcel.h>

using namespace android;

namespace tiarx {

class BpImageBufferMgr : public BpBufferMgr<IImageBufferMgr>
{
public:
    BpImageBufferMgr(const sp<IBinder>& impl)
        : BpBufferMgr<IImageBufferMgr>(impl)
    {
    }

    arxstatus_t bindSurface(const sp<Surface>& surface)
    {
        Parcel data, reply;
        data.writeInterfaceToken(getInterfaceDescriptor());
        Surface::writeToParcel(surface, &data);
        remote()->transact(BnImageBufferMgr::BINDSURFACE, data, &reply);
        return reply.readInt32();
    }

    arxstatus_t bindSurfaceTexture(const sp<ISurfaceTexture>& surfaceTexture)
    {
        Parcel data, reply;
        data.writeInterfaceToken(getInterfaceDescriptor());
        sp<IBinder> b(surfaceTexture->asBinder());
        data.writeStrongBinder(b);
        remote()->transact(BnImageBufferMgr::BINDSURFACETEXTURE, data, &reply);
        return reply.readInt32();
    }

    arxstatus_t getImage(uint32_t index, sp<ImageBuffer> *image)
    {
        Parcel data, reply;
        data.writeInterfaceToken(getInterfaceDescriptor());
        data.writeInt32(index);
        remote()->transact(BnImageBufferMgr::GETIMAGE, data, &reply);
        arxstatus_t status = reply.readInt32();
        if (status == NOERROR) {
            *image = ImageBuffer::readFromParcel(reply);
        }
        return status;
    }

    arxstatus_t render(uint32_t index)
    {
        Parcel data, reply;
        data.writeInterfaceToken(getInterfaceDescriptor());
        data.writeInt32(index);
        remote()->transact(BnImageBufferMgr::RENDER, data, &reply);
        return reply.readInt32();
    }

    arxstatus_t render(uint64_t timestamp)
    {
        Parcel data, reply;
        data.writeInterfaceToken(getInterfaceDescriptor());
        data.writeInt64(timestamp);
        remote()->transact(BnImageBufferMgr::TSRENDER, data, &reply);
        return reply.readInt32();
    }

    arxstatus_t hold(bool enable)
    {
        Parcel data, reply;
        data.writeInterfaceToken(getInterfaceDescriptor());
        data.writeInt32(enable);
        remote()->transact(BnImageBufferMgr::HOLD, data, &reply);
        return reply.readInt32();
    }

    arxstatus_t setSize(uint32_t width, uint32_t height)
    {
        Parcel data, reply;
        data.writeInterfaceToken(getInterfaceDescriptor());
        data.writeInt32(width);
        data.writeInt32(height);
        remote()->transact(BnImageBufferMgr::SETSIZE, data, &reply);
        return reply.readInt32();
    }

    arxstatus_t setFormat(uint32_t format)
    {
        Parcel data, reply;
        data.writeInterfaceToken(getInterfaceDescriptor());
        data.writeInt32(format);
        remote()->transact(BnImageBufferMgr::SETFORMAT, data, &reply);
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(ImageBufferMgr, "ti.arx.IImageBufferMgr");

// ----------------------------------------------------------------------

status_t BnImageBufferMgr::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
    status_t status = NO_ERROR;

    switch (code) {
        case BINDSURFACE:
        {
            CHECK_INTERFACE(IBufferMgr, data, reply);
            sp<Surface> surface = Surface::readFromParcel(data);
            arxstatus_t ret = bindSurface(surface);
            reply->writeInt32(ret);
            break;
        }
        case BINDSURFACETEXTURE:
        {
            CHECK_INTERFACE(IBufferMgr, data, reply);
            sp<ISurfaceTexture> surfaceTexture = interface_cast<ISurfaceTexture>(data.readStrongBinder());
            arxstatus_t ret = bindSurfaceTexture(surfaceTexture);
            reply->writeInt32(ret);
            break;
        }
        case GETIMAGE:
        {
            CHECK_INTERFACE(IFlatBufferMgr, data, reply);
            uint32_t index = data.readInt32();
            sp<ImageBuffer> image;
            arxstatus_t ret = getImage(index, &image);
            reply->writeInt32(ret);
            if (ret == NOERROR && image != NULL) {
                ret = ImageBuffer::writeToParcel(image, reply);
            }
            break;
        }
        case RENDER:
        {
            CHECK_INTERFACE(IBufferMgr, data, reply);
            uint32_t index = data.readInt32();
            arxstatus_t ret = render(index);
            reply->writeInt32(ret);
            break;
        }
        case TSRENDER:
        {
            CHECK_INTERFACE(IBufferMgr, data, reply);
            uint64_t timestamp = data.readInt64();
            arxstatus_t ret = render(timestamp);
            reply->writeInt32(ret);
            break;
        }
        case HOLD:
        {
            CHECK_INTERFACE(IBufferMgr, data, reply);
            bool enable = data.readInt32();
            arxstatus_t ret = hold(enable);
            reply->writeInt32(ret);
            break;
        }
        case SETSIZE:
        {
            CHECK_INTERFACE(IBufferMgr, data, reply);
            uint32_t width = data.readInt32();
            uint32_t height = data.readInt32();
            arxstatus_t ret = setSize(width, height);
            reply->writeInt32(ret);
            break;
        }
        case SETFORMAT:
        {
            CHECK_INTERFACE(IBufferMgr, data, reply);
            uint32_t format = data.readInt32();
            arxstatus_t ret = setFormat(format);
            reply->writeInt32(ret);
            break;
        }
        default :
            return BnBufferMgr<IImageBufferMgr>::onTransact(code, data, reply, flags);
    }
    return status;
}

} // namespace
