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

#include <qnxscreen_api.h>
#include <dvp/dvp_debug.h>

uint32_t width;
uint32_t height;
uint32_t count;
uint32_t fourcc;
char fourcc_str[5];
uint32_t iter;
uint32_t fps;
bool_e   f_running = false_e;
char     filename[255];
uint32_t alpha;
option_t opts[] = {
    {OPTION_TYPE_INT, &width, sizeof(width),         "-w", "--width", "Width of buffer"},
    {OPTION_TYPE_INT, &height, sizeof(height),       "-h", "--height", "Height of buffer"},
    {OPTION_TYPE_INT, &count, sizeof(count),         "-#", "--count", "Number of buffers"},
    {OPTION_TYPE_INT, &fps, sizeof(fps),             "-r", "--fps", "Frame rate"},
    {OPTION_TYPE_STRING, &fourcc_str, sizeof(fourcc_str), "-c",  "--fourcc", "FOURCC Code as string (UYVY|NV12|BGR3) "},
    {OPTION_TYPE_INT, &iter, sizeof(iter),           "-i", "--iterations", "Number of alloc/free cycles"},
    {OPTION_TYPE_STRING, filename, sizeof(filename), "-f", "--file", "Uses a file to render to the screen"},
    {OPTION_TYPE_INT, &alpha, sizeof(alpha),         "-a", "--alpha", "Set the global alpha on the screen"},
};
size_t numOpts = dimof(opts);

typedef struct _yuv_color_t {
    uint32_t uyvy;
    uint8_t  y;
    uint8_t  cb;
    uint8_t  cr;
} yuv_color_t;

#define SET_UYVY(y,cb,cr)   ((y << 24)|(cr << 16)|(y << 8)|cb)

yuv_color_t sdtv_colors[] = {
    {SET_UYVY(180,128,128), 180, 128, 128}, // white
    {SET_UYVY(162, 44,142), 162,  44, 142}, // Yellow
    {SET_UYVY(131,156, 44), 131, 156,  44}, // Cyan
    {SET_UYVY(112, 72, 58), 112,  72,  58}, // Green
    {SET_UYVY( 84,184,198),  84, 184, 198}, // Magenta
    {SET_UYVY( 65,100,212),  65, 100, 212}, // Red
    {SET_UYVY( 35,212,114),  35, 212, 114}, // Blue
    {SET_UYVY( 16,128,128),  16, 128, 128}, // Black
};
uint32_t numSDTVColors = dimof(sdtv_colors);

yuv_color_t hdtv_colors[] = {
    {SET_UYVY(180,128,128), 180, 128, 128}, // White
    {SET_UYVY(168, 44,136), 168,  44, 136}, // Yellow
    {SET_UYVY(145,147, 44), 145, 147,  44}, // Cyan
    {SET_UYVY(133, 63, 52), 133,  63,  52}, // Green
    {SET_UYVY( 63,193,204),  64, 193, 204}, // Magenta
    {SET_UYVY( 51,109,212),  51, 109, 212}, // Red
    {SET_UYVY( 28,212,120),  28, 212, 120}, // Blue
    {SET_UYVY( 16,128,128),  16, 128, 128}, // Black
};
uint32_t numHDTVColors = dimof(hdtv_colors);

void qnxscreen_test_file(uint8_t *buffer, uint32_t width, uint32_t height, uint32_t stride, FILE *f)
{
    uint32_t y, i, l;

    for (y = 0; y < height; y++) {
        i = (y * stride);
        l = width * sizeof(uint16_t);
        if (fread(&buffer[i], 1, l, f) == 0)
            printf("Can't read file\n");
    }
}

void qnxscreen_test_fill(uint8_t *buffer, uint32_t width, uint32_t height, uint32_t stride, fourcc_t color)
{
    uint32_t bar_width = width / numSDTVColors;
    uint32_t x, y, i;
    //printf("bar_width = %u\n", bar_width);
    if (color == FOURCC_NV12) {
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                i = (y * stride) + (x * sizeof(uint8_t));
                buffer[i] = sdtv_colors[x / bar_width].y;
            }
        }
        for (y = 0; y < height/2; y++) {
#if 1
            for (x = 0; x < width/2; x++) {
                i = (height * stride) + (y * stride) + (x * sizeof(uint16_t));
                buffer[i+0] = sdtv_colors[(x*2) / bar_width].cb;
                buffer[i+1] = sdtv_colors[(x*2) / bar_width].cr;
            }
#else
            i = (height * stride) + (y * stride);
            memset(&buffer[i], 128, width);
#endif
        }
    } else if (color == FOURCC_UYVY) {
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x += 2) { // increment over pixel pairs
                uint32_t *p32 = NULL;
                i = (y * stride) + (x * sizeof(uint16_t)); // account that each "pixel" is 2 bytes
                p32 = (uint32_t *)&buffer[i];
                *p32 = sdtv_colors[x / bar_width].uyvy;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int err = 0;
    bool_e useFile = false_e;
    FILE *f = NULL;
    event_t timer;
    struct qnxscreen_api *q;
    uint32_t c, i = 0;

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

    width = 640;
    height = 480;
    count = 4;
    iter = 1;
    fps = 30;
    alpha = 255;
    strcpy(fourcc_str, "NV12");
    memset(filename, 0, sizeof(filename));
    event_init(&timer, true_e);

    option_process(argc, argv, opts, numOpts);
    fourcc_str[4] = '\0';
    fourcc = FOURCC_STRING(fourcc_str);

    if (width > SCREEN_DIM_X)
        width = SCREEN_DIM_X;
    if (height > SCREEN_DIM_Y)
        height = SCREEN_DIM_Y;
    if (iter == 0) iter = 1;

    DVP_PRINT(DVP_ZONE_ALWAYS, "Rendering %u images %u times to display %ux%u @ %u fps in %s\n",count,iter,width,height,fps,fourcc_str);

    if (filename[0] != '\0') {
        useFile = true_e;
        DVP_PRINT(DVP_ZONE_ALWAYS, "Opening file %s\n", filename);
        f = fopen(filename, "rb");
        if (f == NULL)
            useFile = false_e;
    }

    q = qnxscreen_open();
    if (!q) {
        err++;
        goto out;
    }

    for (i = 0; i < iter; i++) {

        // allocate a bunch of memory
        if (qnxscreen_allocate(q, width, height, count, fourcc) != true_e) {
            DVP_PRINT(DVP_ZONE_ALWAYS, "Failed to allocate buffers!\n");
            err++;
            continue;
        }

        qnxscreen_transparency(q, alpha);

        /* Ensure we're starting the file read from the start */
        if (useFile) rewind(f);

        for (c = 0; c < count; c++) {
            uint32_t index = c;
            void *buffer = qnxscreen_acquire(q, &index);
            uint32_t length;
            uint32_t stride;

            if (!buffer) {
                DVP_PRINT(DVP_ZONE_ERROR, "Failed to Acquire QNX Screen Buffer!\n");
                continue;
            }

            length = qnxscreen_length(q, index);
            if (fourcc == FOURCC_UYVY || fourcc == FOURCC_BGR)
                stride = length / height;
            else if (fourcc == FOURCC_NV12)
                stride = length / (height + height/2);
            DVP_PRINT(DVP_ZONE_ALWAYS, "Stride of buffer is %u\n", stride);
            if (useFile) {
                // read from the file
                qnxscreen_test_file(buffer, width, height, stride, f);
            } else if (fourcc == FOURCC_UYVY || fourcc == FOURCC_NV12) {
                // fill the buffer
                DVP_PRINT(DVP_ZONE_ALWAYS, "Rendering Test Pattern into Buffer!\n");
                qnxscreen_test_fill(buffer, width, height, stride, fourcc);
            } else if (fourcc == FOURCC_BGR) {
                uint32_t x,y,i;
                uint8_t *pixels = buffer;
                for (y = 0; y < height; y++)
                {
                    for (x = 0; x < width; x++)
                    {
                        i = (y * stride) + (x * 3);
                        pixels[i+0] = 0xFF; // B
                        pixels[i+1] = 0x00; // G
                        pixels[i+2] = 0x00; // R
                    }
                }
            }

            // post the buffer
            qnxscreen_post(q, index);

            event_wait(&timer, 1000 / fps);
        }

        // free that memory
        if (qnxscreen_free(q) == false_e) {
            DVP_PRINT(DVP_ZONE_ERROR, "Failed to free buffers!\n");
            err++;
        }
    }
    qnxscreen_close(&q);

out:
    event_deinit(&timer);
    if (useFile) {
        fclose(f);
        f = NULL;
    }
    DVP_PRINT(DVP_ZONE_ALWAYS, "Number of errors = %u\n", err);
    return err;
}
