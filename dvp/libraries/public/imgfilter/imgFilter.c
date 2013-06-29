/*
 *  Copyright (C) 2009-2012 Texas Instruments, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sosal/sosal.h>
#include <imgfilter/imgFilter_armv7.h>

typedef struct _argb_pixel_t {
    union _pixel {
        uint32_t u32;
        uint8_t u8[4];
    } data;
} argb_pixel_t;

int16_t coeff[] = {-32, 320, -32};

void __3chan_tap_filter_image_3x1_c(uint8_t *pSrc,
                                 uint32_t width,
                                 uint32_t height,
                                 uint8_t *pDst,
                                 uint32_t srcStride,
                                 uint32_t dstStride,
                                 int16_t coeff[3])
{
    uint32_t x, y, c, i, j;
    int32_t l, k;
    const uint32_t coeff_q = 8;
    const int32_t tw = 3;
    const uint32_t nc = 4;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            i = (y * srcStride) + (x * sizeof(uint32_t));
            j = (y * dstStride) + (x * sizeof(uint32_t));

            for (c = 0; c < nc; c++)
            {
                if (c == 0)
                {
                    pDst[j] = pSrc[i]; // copy the 0th channel
                }
                else
                {
                    int32_t acc = 1 << (coeff_q-1); // the rounding padding

                    for (k = -(tw/2); k <= tw/2; k++)
                    {
                        l = x + k; // add channel offset later
                        if (l < 0)
                            l = 0;
                        else if (l >= (int32_t)width)
                            l = width - 1;

                        i = (y * srcStride) + (l * sizeof(uint32_t));
                        acc += (uint16_t)pSrc[i+c] * coeff[k+tw/2];
                    }
                    acc >>= coeff_q;
                    if (acc > ((1<<coeff_q)-1))
                        acc = ((1<<coeff_q)-1);
                    else if (acc < 0)
                        acc = 0;
                    pDst[j+c] = (uint8_t)acc;
                }
            }
        }
    }
}

typedef struct _filter_args_t
{
    uint8_t *input;
    uint32_t width;
    uint32_t height;
    uint8_t *output;
    uint32_t srcStride;
    uint32_t dstStride;
    uint32_t type;
} filter_args_t;

bool_e threadTapFilter(threadpool_worker_t *worker)
{
    if (worker && worker->data)
    {
        filter_args_t *args = (filter_args_t *)worker->data;
        //printf("Working on %ux%u %p to %p\n", args->width, args->height, args->input, args->output);
#if 0
        __3chan_tap_filter_image_3x1(args->input,
                                     args->width,
                                     args->height,
                                     args->output,
                                     args->srcStride,
                                     args->dstStride,
                                     coeff);
#else
        __3chan_tap_filter_image_3x1_fixed_k(args->input,
                                     args->width,
                                     args->height,
                                     args->output,
                                     args->srcStride,
                                     args->dstStride,
                                     8);
#endif
        return true_e;
    }
    return false_e;
}

bool_e threadEdgeFilter(threadpool_worker_t *worker)
{
    if (worker && worker->data)
    {
        uint32_t limit = 255;
        filter_args_t *args = (filter_args_t *)worker->data;
        if (args->type == 0)
        {
            int32_t sobel_3[3] = {1,2,1};
            int32_t range = SOBEL_RANGE;
            __planar_edge_filter_3x3(args->width,
                                     args->height,
                                     args->input,
                                     args->width,
                                     sobel_3,
                                     args->output,
                                     args->width,
                                     range,
                                     limit);
        }
        else if (args->type  == 1)
        {
            int32_t prewitt_3[3] = {1,1,1};
            int32_t range = PREWITT_RANGE;
            __planar_edge_filter_3x3(args->width,
                                     args->height,
                                     args->input,
                                     args->width,
                                     prewitt_3,
                                     args->output,
                                     args->width,
                                     range,
                                     limit);
        }
        else if (args->type == 2)
        {
            int32_t scharr_3[3] = {3,10,3};
            int32_t range = SCHARR_RANGE;
            __planar_edge_filter_3x3(args->width,
                                     args->height,
                                     args->input,
                                     args->width,
                                     scharr_3,
                                     args->output,
                                     args->width,
                                     range,
                                     limit);
        }
        else if (args->type == 3)
        {
            int32_t kroon_3[3] = {17,61,17};
            int32_t range = KROON_RANGE;
            __planar_edge_filter_3x3(args->width,
                                     args->height,
                                     args->input,
                                     args->width,
                                     kroon_3,
                                     args->output,
                                     args->width,
                                     range,
                                     limit);
        }
        return true_e;
    }
    return false_e;
}
int main(int argc, char *argv[])
{
    printf("argc=%u argv[1]=%s\n", argc, argv[1]);
    printf("$ %s <op> <w> <h> <fin> <fout> <freq> <nthreads>\n", argv[0]);
    if (argc == 8)
    {
        char op[10];
        uint32_t width = atoi(argv[2]);
        uint32_t height = atoi(argv[3]);
        FILE *fin = fopen(argv[4], "rb");
        FILE *fout = fopen(argv[5], "wb+");
        uint32_t freq = atoi(argv[6]);
        uint32_t nthreads = atoi(argv[7]);
        uint8_t *input = NULL;
        uint8_t *output = NULL;
        uint32_t size = width * height; // initial size
        size_t nb = 0;
        uint32_t limit = 255;
        uint32_t stride = width;
        profiler_t perf;
        uint32_t i = 0;

        profiler_clear(&perf);

        strncpy(op, argv[1], sizeof(op));

        if (strncmp(op,"tap", sizeof(op)) == 0 )
        {
            size *= sizeof(uint32_t);
            stride *= sizeof(uint32_t);
        }

        printf("$ %s %s %u %u %s %s %u %u\n", argv[0], op, width, height, argv[4], argv[5], freq, nthreads);

        input = (uint8_t *)calloc(1, size);
        output = (uint8_t *)calloc(1, size);

        if (input && output && fin && fout)
        {
            memset(input, 0x80, size);
            memset(output, 0x00, size);

            if (nthreads == 1)
            {
                while ((nb = fread(input, 1, size, fin)) > 0)
                {
                    printf("Read "FMT_SIZE_T" bytes\n", nb);
                    if (strncmp(op, "tap", sizeof(op)) == 0)
                    {
                        profiler_start(&perf);
#if 0
                        __3chan_tap_filter_image_3x1_c((uint8_t *)input,
                                                     width,
                                                     height,
                                                     (uint8_t *)output,
                                                     width*sizeof(uint32_t),
                                                     width*sizeof(uint32_t),
                                                     coeff);
#else
                        __3chan_tap_filter_image_3x1_fixed_k((uint8_t *)input,
                                                     width,
                                                     height,
                                                     (uint8_t *)output,
                                                     width*sizeof(uint32_t),
                                                     width*sizeof(uint32_t),
                                                     8);
#endif
                        profiler_stop(&perf);
                    }
                    else if (strncmp(op,"sobel",sizeof(op)) == 0)
                    {
                        int32_t sobel_3[3] = {1,2,1};
                        int32_t range = SOBEL_RANGE;
                        profiler_start(&perf);
                        __planar_edge_filter_3x3(width,
                                                 height,
                                                 input,
                                                 width,
                                                 sobel_3,
                                                 output,
                                                 width,
                                                 range,
                                                 limit);
                        profiler_stop(&perf);
                    }
                    else if (strncmp(op,"prewitt",sizeof(op)) == 0)
                    {
                        int32_t prewitt_3[3] = {1,1,1};
                        int32_t range = PREWITT_RANGE;
                        profiler_start(&perf);
                        __planar_edge_filter_3x3(width,
                                                 height,
                                                 input,
                                                 width,
                                                 prewitt_3,
                                                 output,
                                                 width,
                                                 range,
                                                 limit);
                        profiler_stop(&perf);
                    }
                    else if (strncmp(op,"scharr",sizeof(op)) == 0)
                    {
                        int32_t scharr_3[3] = {3,10,3};
                        int32_t range = SCHARR_RANGE;
                        profiler_start(&perf);
                        __planar_edge_filter_3x3(width,
                                                 height,
                                                 input,
                                                 width,
                                                 scharr_3,
                                                 output,
                                                 width,
                                                 range,
                                                 limit);
                        profiler_stop(&perf);
                    }
                    else if (strncmp(op, "kroon", sizeof(op)) == 0)
                    {
                        int32_t kroon_3[3] = {17,61,17};
                        int32_t range = KROON_RANGE;
                        profiler_start(&perf);
                        __planar_edge_filter_3x3(width,
                                                 height,
                                                 input,
                                                 width,
                                                 kroon_3,
                                                 output,
                                                 width,
                                                 range,
                                                 limit);
                        profiler_stop(&perf);
                    }
                    printf("Operations took %lf sec\n", (double)perf.tmpTime/perf.rate);
                    fwrite(output, 1, size, fout);
                }
            }
            else if (1 < nthreads && nthreads <= 8)
            {
                filter_args_t args[8];
                threadpool_t *pool;
                threadpool_f func = NULL;

                for (i = 0; i < nthreads; i++)
                {
                    uint32_t bar_height = height/nthreads;
                    uint32_t bar_index = i * bar_height;

                    args[i].input = &input[bar_index * stride];
                    args[i].width = width;
                    args[i].height = height/nthreads;
                    args[i].output = &output[bar_index * stride];
                    args[i].srcStride = stride;
                    args[i].dstStride = stride;
                    printf("%ux%u from %p to %p\n", args[i].width, args[i].height, args[i].input, args[i].output);
                }

                if (strncmp(op, "tap", sizeof(op)) == 0)
                    func = threadTapFilter;
                else
                    func = threadEdgeFilter;

                pool = threadpool_create(nthreads, 1, sizeof(filter_args_t), func, NULL);
                if (pool)
                {
                    while ((nb = fread(input, 1, size, fin)) > 0)
                    {
                        void *data[] = {&args[0], &args[1]};
                        printf("Read "FMT_SIZE_T" bytes\n", nb);

                        do
                        {
                            struct timespec t;
                            profiler_start(&perf);
                            if (true_e == threadpool_issue(pool, data, dimof(data)))
                                threadpool_complete(pool, true_e);
                            profiler_stop(&perf);
                            printf("Operation took %lf sec\n", (double)perf.tmpTime/perf.rate);
                            if (freq)
                            {
                                t.tv_sec = 0;
                                t.tv_nsec = (1000000000/freq) - perf.tmpTime;
                                printf("Sleeping for %lu ns\n", t.tv_nsec);
                                nanosleep(&t, &t);
                            }
                        } while (freq > 0);
                        fwrite(output, 1, size, fout);
                    }
                    threadpool_destroy(pool);
                }
            }
        }
        free(input);
        free(output);
        fclose(fin);
        fclose(fout);
        return 0;
    }
    else if (argc == 1)
    {
        uint32_t width = 1280;
        uint32_t height = 720;
        uint32_t *input = (uint32_t *)calloc(width*height, sizeof(uint32_t));
        uint32_t *output = (uint32_t *)calloc(width*height, sizeof(uint32_t));
        profiler_t perf;

        profiler_clear(&perf);

        if (input && output)
        {
            memset(input, 0x80, width*height*sizeof(uint32_t));
            memset(input, 0x00, width*height*sizeof(uint32_t));
            profiler_start(&perf);
            memcpy(output, input, width*height*sizeof(uint32_t));
            profiler_stop(&perf);
            printf("Memcpy of HD720 in %lf sec\n", (double)perf.tmpTime/perf.rate);
            free(input);
            free(output);
        }
        else
        {
            if( input )
            {
                free(input);
                input = NULL;
            }

            if( output )
            {
                free(output);
                output = NULL;
            }
        }
        return 0;
    }
    return -1;
}
