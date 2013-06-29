/**
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

#include <v4l2_api.h>
#include <dvp/dvp_debug.h>

uint32_t dvp_zone_mask; // declare a local version for testing.

uint32_t width;
uint32_t height;
uint32_t count;
uint32_t fourcc;
uint32_t iter;
uint32_t fps;
bool_e   f_running = false_e;
char     filename[255];
option_t opts[] = {
    {OPTION_TYPE_INT, &width, sizeof(width), "-w", "--width", "Width of buffer"},
    {OPTION_TYPE_INT, &height, sizeof(height), "-h", "--height", "Height of buffer"},
    {OPTION_TYPE_INT, &count, sizeof(count), "-#", "--count", "Number of buffers"},
    {OPTION_TYPE_INT, &fps, sizeof(fps), "-f", "--fps", "Frame rate"},
    {OPTION_TYPE_HEX, &fourcc, sizeof(fourcc), "-c", "--color", "Color Space of Buffer"},
    {OPTION_TYPE_INT, &iter, sizeof(iter), "-i", "--iterations", "Number of alloc/free cycles"},
    {OPTION_TYPE_STRING, filename, sizeof(filename), "-f", "--file", "Uses a file to render to the screen"},
};
size_t numOpts = dimof(opts);

#define SET_UYVY(y,cb,cr)   ((y << 24)|(cr << 16)|(y << 8)|cb)

uint32_t sdtv_colors[] = {
    SET_UYVY(180,128,128),
    SET_UYVY(162, 44,142),
    SET_UYVY(131,156, 44),
    SET_UYVY(112, 72, 58),
    SET_UYVY( 84,184,198),
    SET_UYVY( 65,100,212),
    SET_UYVY( 35,212,114),
    SET_UYVY( 16,128,128),
};
uint32_t numSDTVColors = dimof(sdtv_colors);

void v4l2_test_file(uint8_t *buffer, uint32_t width, uint32_t height, uint32_t stride, FILE *f)
{
    uint32_t y,i,l;
    for (y = 0; y < height; y++)
    {
        i = (y * stride);
        l = width * sizeof(uint16_t);
        if (fread(&buffer[i], 1, l, f) == 0)
            printf("Can't read file\n");
    }
}

void v4l2_test_fill(uint8_t *buffer, uint32_t width, uint32_t height, uint32_t stride)
{
    uint32_t bar_width = width / numSDTVColors;
    uint32_t x,y,i;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x+=2)
        {
            uint32_t *p32 = NULL;
            i = (y * stride) + (x * sizeof(uint16_t));
            p32 = (uint32_t *)&buffer[i];
            *p32 = sdtv_colors[width/bar_width];
        }
    }
}

bool_e running;
event_t wait_for_frame;

thread_ret_t dequeuer(void *arg)
{
    v4l2_api_t *v = (v4l2_api_t *)arg;
    uint32_t index = 0;
    void *buffer = NULL;

    while (running)
    {
        event_wait(&wait_for_frame, 100);
        if (v4l2_wait(v))
        {
            // dequeue the buffer
            if (v4l2_dequeue(v, &index, V4L2_BUF_TYPE_VIDEO_OUTPUT) == true_e)
            {
                // set the buffer pointer (though not strictly necessary)
                buffer = v->buffers[index];

                // release the buffer
                v4l2_release(v, buffer, index);
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int err = 0;
    bool_e useFile = false_e;
    FILE *f = NULL;
    event_t timer;

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

    width = 640;
    height = 480;
    count = 4;
    fourcc = FOURCC_UYVY;
    iter = 1;
    fps = 30;

    event_init(&timer, true_e);

    option_process(argc, argv, opts, numOpts);

    if (filename[0] != '\0')
    {
        useFile = true_e;
        f = fopen(filename, "rb");
        if (f == NULL)
            useFile = false_e;
    }
    event_init(&wait_for_frame, true_e);
    v4l2_api_t *v = v4l2_open(V4L2_DISPLAY_DEVICE, V4L2_CAP_STREAMING, true_e);
    if (v)
    {
        uint32_t c,i = 0;
        for (i = 0; i < iter; i++)
        {
            // allocate a bunch of memory
            if (v4l2_allocate(v, width, height, count, fourcc, V4L2_BUF_TYPE_VIDEO_OUTPUT) == true_e)
            {
                thread_t t;

                running = true_e;
                t = thread_create(dequeuer, v);

                for (c = 0; c < count; c++)
                {
                    uint32_t index = 0;
                    void *buffer = v4l2_acquire(v, &index);
                    if (buffer != NULL)
                    {
                        uint32_t stride = v4l2_length(v, index) / height;

                        if (useFile)
                        {
                            // read from the file
                            v4l2_test_file(buffer, width, height, stride, f);
                        }
                        else
                        {
                            // fill the buffer
                            v4l2_test_fill(buffer, width, height, stride);
                        }

                        // queue the buffer
                        v4l2_queue(v, index, V4L2_BUF_TYPE_VIDEO_OUTPUT);
                        v4l2_start(v, V4L2_BUF_TYPE_VIDEO_OUTPUT);
                        event_set(&wait_for_frame);
                    }
                    else
                    {
                        DVP_PRINT(DVP_ZONE_ALWAYS, "Failed to Acquire V4L2 Buffer!\n");
                    }

                    event_wait(&timer, 1000/fps);
                }

                running = false_e;
                v4l2_stop(v, V4L2_BUF_TYPE_VIDEO_OUTPUT);
                thread_join(t);

                // free that memory
                if (v4l2_free(v) == false_e) {
                    DVP_PRINT(DVP_ZONE_ALWAYS, "Failed to free buffers!\n");
                    err++;
                }
            }
            else
            {
                DVP_PRINT(DVP_ZONE_ALWAYS, "Failed to allocate buffers!\n");
                err++;
            }
        }
        v4l2_close(&v);
    }
    event_deinit(&wait_for_frame);
    event_deinit(&timer);
    if (useFile) {
        fclose(f);
        f = NULL;
    }
    DVP_PRINT(DVP_ZONE_ALWAYS, "Number of errors = %u\n", err);
    return err;
}
