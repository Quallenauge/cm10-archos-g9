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

#include <sosal/histogram.h>
#include <sosal/debug.h>

bool_e histogram_init(histogram_t *histogram, uint32_t bins, int32_t min, int32_t max)
{
    bool_e ret = true_e;
    histogram->binRange = (max - min)/bins;
    if (((max-min) % bins) > 0)
        histogram->binRange++;
    histogram->numBins = bins;
    histogram->min = min;
    histogram->max = max;
    histogram->bins = calloc(sizeof(uint32_t),bins);
    if (histogram->bins == NULL)
        ret = false_e;
    return ret;
}

void histogram_deinit(histogram_t *histogram)
{
    memset(histogram->bins, 0, histogram->numBins * sizeof(uint32_t));
}

void histogram_inc(histogram_t *histogram, int32_t value)
{
    // is it in range?
    if (histogram->min <= value && value < histogram->max)
    {
        // get the index in the bins
        uint32_t binIdx = (value - histogram->min)/histogram->binRange;
        if (binIdx < histogram->numBins)
        {
            // increment the frequency
            histogram->bins[binIdx]++;
        }
    }
}

uint32_t histogram_get(histogram_t *histogram, int32_t value)
{
    uint32_t freq = 0;
    // is it in range?
    if (histogram->min <= value && value < histogram->max)
    {
        // get the index in the bins
        uint32_t binIdx = (value - histogram->min)/histogram->binRange;
        if (binIdx < histogram->numBins)
        {
            // get the frequency
            freq = histogram->bins[binIdx];
        }
    }
    return freq;
}

bool_e histogram_unittest(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    bool_e ret = true_e;
    histogram_t histogram;
    int32_t min = -100;
    int32_t max = 100;
    uint32_t bins = 17;
    bool_e verbose = false_e;

    SOSAL_PRINT(SOSAL_ZONE_API, "histogram_unittest(%u, %p)\n", argc, argv);
    if (histogram_init(&histogram, bins, min, max))
    {
        int32_t count = abs(rand()) % 10000;
        int32_t i = 0;

        for (i = 0; i < count; i++)
        {
            int32_t value = (rand()%(max-min)) + min;
            histogram_inc(&histogram, value);
        }
        if (verbose)
        {
            for (i = min; i <= max; i++)
            {
                printf("Bin[%d] = %u\n", i, histogram_get(&histogram, i));
            }
        }
        histogram_deinit(&histogram);
    }

    return ret;
}


