/*
 * Copyright (C) 2013 Texas Instruments
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// #define LOG_NDEBUG 0
// #define VERY_VERBOSE_LOGGING

#include <media/nbaio/NBAIO.h>
#include <media/nbaio/MonoPipe.h>
#include <media/nbaio/MonoPipeReader.h>
#include <media/AudioBufferProvider.h>

#include <tiaudioutils/Log.h>
#include <tiaudioutils/MonoPipe.h>

namespace tiaudioutils {

using android::NBAIO_Format;
using android::Format_from_SR_C;

MonoPipe::MonoPipe(const PcmParams &params, uint32_t frames)
    : mParams(params)
{
    if (frames < (kPipeSizeFactor * mParams.frameCount))
        frames = kPipeSizeFactor * mParams.frameCount;

    ALOGV("MonoPipe: Create pipe for %u frames (%u bytes)",
          frames, params.framesToBytes(frames));

    NBAIO_Format format = Format_from_SR_C(mParams.sampleRate, mParams.channels);

    NBAIO_Format offers[1] = {format};
    size_t numCounterOffers = 0;

    mSink = new android::MonoPipe(frames, format, true);
    if (mSink) {
        ssize_t index = mSink->negotiate(offers, 1, NULL, numCounterOffers);
        ALOG_ASSERT(index == 0);

        mSource = new android::MonoPipeReader(mSink);
        if (mSource) {
            numCounterOffers = 0;
            index = mSource->negotiate(offers, 1, NULL, numCounterOffers);
            ALOG_ASSERT(index == 0);
        }
    }
}

MonoPipe::~MonoPipe()
{
    if (mSink) {
        mSink->shutdown(true);
        delete mSink;
    }

    if (mSource)
        delete mSource;
}

bool MonoPipe::initCheck() const
{
    return ((mSink != NULL) && (mSource != NULL));
}

int MonoPipe::read(void *buffer, uint32_t frames)
{
    ssize_t read = availableToRead();

    /* Wait the time equivalent to the pending frames */
    if ((read >= 0) && (read < (ssize_t)frames))
        usleep(((frames - read) * 1000 * 1000) / mParams.sampleRate);

    /* NBAIO pipe reader doesn't block on read() */
    read = mSource->read(buffer, frames,
                         android::AudioBufferProvider::kInvalidPTS);
    if (read < 0)
        ALOGE("MonoPipe: failed to read from pipe %d", read);

    return read;
}

int MonoPipe::write(const void *buffer, uint32_t frames)
{
    ssize_t written = mSink->write(buffer, frames);
    if (written < 0)
        ALOGE("MonoPipe: failed to write to pipe %d", written);

    return written;
}

int MonoPipe::availableToRead() const
{
    return mSource->availableToRead();
}

int MonoPipe::availableToWrite() const
{
    return mSink->availableToWrite();
}

/* ---------------------------------------------------------------------------------------- */

PipeReader::PipeReader(MonoPipe *pipe)
    : mPipe(pipe)
{
}

PipeReader::~PipeReader()
{
    if (mBuffer.i8)
        delete [] mBuffer.i8;
}

int PipeReader::getNextBuffer(BufferProvider::Buffer *buffer)
{
    uint32_t frameSize = mPipe->getParams().frameSize();

    /* resize our internal buffer if needed */
    if (mBuffer.frameCount < buffer->frameCount) {
        if (mBuffer.i8)
            delete [] mBuffer.i8;
        mBuffer.i8 = new int8_t[buffer->frameCount * frameSize];
        if (mBuffer.i8 == NULL) {
            ALOGE("PipeReader: failed to resize internal buffer");
            return -ENOMEM;
        }
        mBuffer.frameCount = buffer->frameCount;
    }

    int8_t *buf = mBuffer.i8;
    int pending = buffer->frameCount;

    while (pending > 0) {
        int read = mPipe->read(buf, pending);
        if (read < 0) {
            ALOGE("PipeReader: failed to read from pipe %d", read);
            return read;
        } else if (read == 0) {
            ALOGW("PipeReader: underrun!");
            break;
        } else {
            pending -= read;
            buf += read * frameSize;
        }
    }

    buffer->frameCount -= pending;
    buffer->raw = mBuffer.raw;

    ALOGW_IF(pending, "PipeReader: unexpected %u pending frames", pending);

    return 0;
}

void PipeReader::releaseBuffer(BufferProvider::Buffer *buffer)
{
    /* Nothing to do to release the buffer, but must be implemented */
}

/* ---------------------------------------------------------------------------------------- */

PipeWriter::PipeWriter(MonoPipe *pipe)
    : mPipe(pipe)
{
}

PipeWriter::~PipeWriter()
{
    if (mBuffer.i8)
        delete [] mBuffer.i8;
}

int PipeWriter::getNextBuffer(BufferProvider::Buffer *buffer)
{
    uint32_t frameSize = mPipe->getParams().frameSize();

    /* resize our internal buffer if needed */
    if (mBuffer.frameCount < buffer->frameCount) {
        if (mBuffer.i8)
            delete [] mBuffer.i8;
        mBuffer.i8 = new int8_t[buffer->frameCount * frameSize];
        if (mBuffer.i8 == NULL) {
            ALOGE("PipeWriter: failed to resize internal buffer");
            return -ENOMEM;
        }
        mBuffer.frameCount = buffer->frameCount;
    }

    buffer->raw = mBuffer.raw;
    buffer->frameCount = mBuffer.frameCount;

    return 0;
}

void PipeWriter::releaseBuffer(BufferProvider::Buffer *buffer)
{
    int8_t *buf = buffer->i8;
    int pending = buffer->frameCount;
    uint32_t frameSize = mPipe->getParams().frameSize();

    while (pending > 0) {
        int written = mPipe->write(buf, pending);
        if (written < 0) {
            ALOGE("PipeWriter: failed to write to pipe %d", written);
            return;
        }
        pending -= written;
        buf += written * frameSize;
    }

    ALOGW_IF(pending, "PipeWriter: unexpected %u pending frames", pending);
}

} /* namespace tiaudioutils */
