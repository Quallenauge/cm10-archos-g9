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

#include <errno.h>
#include <tinyalsa/asoundlib.h>

#include <tiaudioutils/Log.h>
#include <tiaudioutils/ALSAPcm.h>

namespace tiaudioutils {

ALSAInPort::ALSAInPort(uint32_t card, uint32_t port, uint32_t period_count)
    : mCardId(card), mPortId(port), mPeriodCount(period_count), mPcm(NULL)
{
}

ALSAInPort::~ALSAInPort()
{
    if (!mPcm || !pcm_is_ready(mPcm))
        close();
}

int ALSAInPort::open(const PcmParams &params)
{
    struct pcm_config config;

    AutoMutex lock(mLock);

    /* fills rate, channels, format and period size */
    params.toPcmConfig(config);

    config.period_count = mPeriodCount;
    config.start_threshold = 1;
    config.stop_threshold = config.period_size * config.period_count;
    config.silence_threshold = 0;
    config.avail_min = 0;

    /* Save the period size and count to check later if were refined */
    uint32_t periodSize = config.period_size;
    uint32_t periodCount = config.period_count;

    ALOGI("ALSAInPort: open capture port hw:%u,%u", mCardId, mPortId);

    mPcm = pcm_open(mCardId, mPortId, PCM_IN, &config);
    if (!pcm_is_ready(mPcm)) {
        ALOGE("ALSAInPort: failed to open capture port hw:%u,%u: %s",
              mCardId, mPortId, pcm_get_error(mPcm));
        return -ENODEV;
    }

    if ((periodSize != config.period_size) || (periodCount != config.period_count)) {
        ALOGW("ALSAInPort: params were updated period_size=%u->%u periods=%u->%u",
              periodSize, config.period_size, periodCount, config.period_count);
    }

    return 0;
}

void ALSAInPort::close()
{
    ALOGI("ALSAInPort: close capture port hw:%u,%u", mCardId, mPortId);

    AutoMutex lock(mLock);
    if (mPcm && pcm_is_ready(mPcm)) {
        pcm_close(mPcm);
        mPcm = NULL;
    }
}

bool ALSAInPort::isOpen() const
{
    AutoMutex lock(mLock);

    if (mPcm && pcm_is_ready(mPcm))
        return true;
    else
        return false;
}

int ALSAInPort::read(void *buffer, size_t frames)
{
    AutoMutex lock(mLock);

    if (!mPcm || !pcm_is_ready(mPcm)) {
        ALOGE("ALSAInPort: port hw:%u,%u is closed, cannot read", mCardId, mPortId);
        return -EAGAIN;
    }

    uint32_t bytes = pcm_frames_to_bytes(mPcm, frames);

    ALOGVV("ALSAInPort: read %u frames (%u bytes) buffer %p", frames, bytes, buffer);

    int ret = pcm_read(mPcm, buffer, bytes);
    if (ret) {
        ALOGE("ALSAInPort: failed to read %d", ret);
        return ret;
    }

    return frames;
}

/* ---------------------------------------------------------------------------------------- */

ALSAOutPort::ALSAOutPort(uint32_t card, uint32_t port, uint32_t period_count)
    : mCardId(card), mPortId(port), mPeriodCount(period_count), mPcm(NULL)
{
}

ALSAOutPort::~ALSAOutPort()
{
    if (!mPcm || !pcm_is_ready(mPcm))
        close();
}

int ALSAOutPort::open(const PcmParams &params)
{
    struct pcm_config config;

    AutoMutex lock(mLock);

    /* fills rate, channels, format and period size */
    params.toPcmConfig(config);

    config.period_count = mPeriodCount;
    config.start_threshold = config.period_size;
    config.stop_threshold = config.period_size * config.period_count;
    config.silence_threshold = 0;
    config.avail_min = 0;

    /* Save the period size and count to check later if were refined */
    uint32_t periodSize = config.period_size;
    uint32_t periodCount = config.period_count;

    ALOGI("ALSAOutPort: open playback port hw:%u,%u", mCardId, mPortId);

    mPcm = pcm_open(mCardId, mPortId, PCM_OUT, &config);
    if (!pcm_is_ready(mPcm)) {
        ALOGE("ALSAOutPort: failed to open playback port hw:%u,%u: %s",
              mCardId, mPortId, pcm_get_error(mPcm));
        return -ENODEV;
    }

    if ((periodSize != config.period_size) || (periodCount != config.period_count)) {
        ALOGW("ALSAOutPort: params were updated period_size=%u->%u periods=%u->%u",
              periodSize, config.period_size, periodCount, config.period_count);
    }

    return 0;
}

void ALSAOutPort::close()
{
    ALOGI("ALSAOutPort: close playback port hw:%u,%u", mCardId, mPortId);

    AutoMutex lock(mLock);
    if (mPcm && pcm_is_ready(mPcm)) {
        pcm_close(mPcm);
        mPcm = NULL;
    }
}

bool ALSAOutPort::isOpen() const
{
    AutoMutex lock(mLock);

    if (mPcm && pcm_is_ready(mPcm))
        return true;
    else
        return false;
}

int ALSAOutPort::write(const void *buffer, size_t frames)
{
    AutoMutex lock(mLock);

    if (!mPcm || !pcm_is_ready(mPcm)) {
        ALOGE("ALSAOutPort: port hw:%u,%u is closed, cannot write", mCardId, mPortId);
        return -EAGAIN;
    }

    uint32_t bytes = pcm_frames_to_bytes(mPcm, frames);

    ALOGVV("ALSAOutPort: write %u frames (%u bytes) buffer %p", frames, bytes, buffer);

    int ret = pcm_write(mPcm, buffer, bytes);
    if (ret) {
        ALOGE("ALSAOutPort: failed to write %d", ret);
        return ret;
    }

    return frames;
}

} /* namespace tiaudioutils */
