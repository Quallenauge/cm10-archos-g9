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
#include "VisionCam.h"

class VisionCamFrameManager
{
public:
    enum FrameProperty_t{
        FrameBuffer_e,
        FrameTimeStamp_e,
        FrameNode_e
    };

    enum FrameOperation_t{
        Alloc_e     = 1 << 1,
        Dealloc_e   = 1 << 2,
        Set_e       = 1 << 3,
        Get_e       = 1 << 4,
        Add_e       = 1 << 5,
        Remove_e    = 1 << 6
    };

    enum OperationLock_t {
        NoneLock_e      = 0,
        ReadLock_e      = Alloc_e | Dealloc_e | Get_e | Remove_e,
        WriteLock_e     = Alloc_e | Dealloc_e | Set_e | Remove_e
    };

    VisionCamFrameManager();
    ~VisionCamFrameManager();

    bool_e Alloc    (VisionCamFrame*);
    bool_e Dealloc  (VisionCamFrame*);
    bool_e Add      (VisionCamFrame*);
    bool_e Remove   (VisionCamFrame*);
    bool_e Get      (VisionCamFrame*, FrameProperty_t, void*);
    bool_e Set      (VisionCamFrame*);
    bool_e Lock     (VisionCamFrame*, OperationLock_t);
    bool_e Unlock   (VisionCamFrame*, OperationLock_t);

private:
    void *AllocBuffer();
    bool_e DeallocBuffer(void*);

protected:
    struct FrameQueueNode_t {
        VisionCamFrame *frame;
        OperationLock_t lock;
    };
    queue_t *qFrames;
};

VisionCamFrameManager::VisionCamFrameManager( int32_t numFrms)
{
    qFrames = NULL;
    qFrames = queue_create( numFrms , sizeof(FrameQueueNode_t) );
}

VisionCamFrameManager::~VisionCamFrameManager()
{
    queue_destroy(qFrames);
}

bool_e VisionCamFrameManager::Alloc(VisionCamFrame* frm, FrameOperation_t op)
{
    if( frm )
    {
        if( !frm->mFrameBuff )
            frm->mFrameBuff = AllocBuffer();
    }
}

bool_e VisionCamFrameManager::Dealloc(VisionCamFrame* frm)
{
    bool_e ret = false_e;
    if( frm )
    {
//        Lock(frm, Dealloc_e);
        if( Remove(frm) )
            ret = DeallocBuffer(frm->mFrameBuff);
    }

    return ret;
}

bool_e VisionCamFrameManager::Add( VisionCamFrame* frm )
{
    bool_e ret = false_e;
    FrameQueueNode_t *fr = NULL;

    if( frm && qFrames )
    {
//        Lock( frm, Alloc_e | Add_e );
        fr = new FrameQueueNode_t;
        fr->frame   = frm;

        ret = queue_write( qFrames, false_e, fr);
    }

    return ret;
}

bool_e VisionCamFrameManager::Remove( VisionCamFrame* frm)
{
    bool_e ret = false_e;
    FrameQueueNode_t *fNode =  NULL;

    if( frm )
    {
        Lock(frm, ReadLock_e | WriteLock_e );
        if ( Get(frm, FrameNode_e, fNode) )
        {
            delete fNode;
            fNode = NULL;
        }
        // dont need to unlock because it won't exist anymore
    }

    return ret;
}

bool_e VisionCamFrameManager::Get(VisionCamFrame* frm, FrameProperty_t p, void* dest)
{
    bool_e ret = false_e;
    FrameQueueNode_t *fr = NULL;

    if( frm && qFrames )
    {
        Lock(frm, WriteLock_e );
        while( queue_length(qFrames ) )
        {
            queue_read( qFrames, false_e, fr);

            if( !fr )
                continue;

            switch(p)
            {
                case FrameBuffer_e:
                    if( fr->frame == frm->mFrameBuff )
                    {
                        dest = fr->frame;//->mFrameBuff;
                        ret = true_e;
                    }
                    break;

                case FrameTimeStamp_e:
                    if( fr->frame->mTimestamp == frm->mTimestamp )
                    {
                        dest = fr->frame;
                        ret = true_e;
                    }
                    break;

                case FrameNode_e:
                    if(fr->frame == frm->mFrameBuff)
                    {
                        dest = fr;
                        ret = true_e;
                    }
                    break;
            }
        }
        Unlock(frm, WriteLock_e );
    }

    return ret;
}

bool_e VisionCamFrameManager::Set(VisionCamFrame *fr)
{
    bool_e ret = false_e;
    FrameQueueNode_t *fNode = NULL;
    if( fr && qFrames )
    {
        return true_e;
        Lock(frm, ReadLock_e );
        while( queue_length(qFrames ) )
        {
            queue_read( qFrames, false_e, fNode);

            if( !fNode )
                continue;

            if( fNode && fNode->frame && fNode->frame->mFrameBuff == fr->mFrameBuff )
            {
                memcpy(fNode->frame, fr, sizeof(VisionCamFrame));/// @todo check other possibilities
            }
        }
        Unlock(frm, ReadLock_e );
    }

    return ret;
}

bool_e VisionCamFrameManager::Lock(VisionCamFrame *fr, OperationLock_t lock)
{
    bool_e ret = false_e;
    FrameQueueNode_t *fNode = NULL;

    if( fr && qFrames )
    {
        while( queue_length(qFrames) )
        {
            queue_read(qFrames, false_e, fNode);

            if( fNode && fNode->frame && fNode->frame== fr )
            {
                fNode->lock |= lock;
                ret = true_e;
            }
        }
    }

    return ret;
}

bool_e VisionCamFrameManager::Unlock(VisionCamFrame*, OperationLock_t lock)
{
    bool_e ret = false_e;
    FrameQueueNode_t *fNode = NULL;
    if( fr && qFrames )
    {
        while( queue_length(qFrames) )
        {
            queue_read(qFrames, false_e, fNode);

            if( fNode && fNode->frame == fr )
            {
                fNode->lock ^= lock;
                ret = true_e;
            }
        }
    }
    return ret;
}

void* VisionCamFrameManager::AllocBuffer()
{
    return NULL;
}

bool_e VisionCamFrameManager::DeallocBuffer(void*)
{
    bool_e ret = false_e;

    return ret;
}
