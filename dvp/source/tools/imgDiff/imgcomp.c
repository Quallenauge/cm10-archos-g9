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
 
#include <sosal/sosal.h>

enum equality_e {
    image_not_equal = -1,
    image_equal = 0,
};

typedef struct _fic_t {
    image_t *image;
    FILE *file;
} fic_t;

typedef struct _fic_compare_t {
    uint64_t numDiff;
     int64_t maxDiff;
    uint64_t range;
    uint64_t sumDiff;
} fic_compare_t;

void fic_open(fic_t *fic, char *filename)
{
    uint32_t width, height, fps;
    fourcc_t color;

    PYUV_DecodeFilename(filename, &width, &height, &fps, &color);
    printf("%ux%u of type %s\n", width, height, fourcctostr(color));
    fic->file = fopen(filename, "rb");
    if (fic->file)
    {
        fic->image = image_allocate(width, height, color);
        if (fic->image == NULL)
        {
            printf("Failed to allocate image of type %s\n",fourcctostr(color));
            fclose(fic->file);
            fic->file = NULL;
        }
        else
        {
            image_back(fic->image);
            image_print(fic->image);
        }
    }
    else
        printf("Failed to open file %s\n", filename);
}

size_t fic_read(fic_t *fic)
{
    size_t bytes = 0;
    if (fic && fic->file)
    {
        uint32_t p,y;
        for (p = 0; p < fic->image->numPlanes; p++)
        {
            for (y = 0;
                 y < fic->image->plane[p].ydim/fic->image->plane[p].yscale;
                 y += 1)
            {
                uint8_t *ptr = image_pixel(fic->image, 0, y, p);
                uint32_t size = fic->image->plane[p].xdim * fic->image->plane[p].xstride;
                bytes += fread(ptr, 1, size, fic->file);
            }
        }
    }
    else
    {
        printf("fic is invalid\n");
    }
    return bytes;
}

int fic_compare(fic_t *ref, fic_t *cmp, fic_compare_t *data)
{
    data->numDiff = 0;
    data->maxDiff = 0;
    data->range = 0;
    data->sumDiff = 0;

    if (ref->image->color == cmp->image->color &&
        ref->image->plane[0].xdim == cmp->image->plane[0].xdim &&
        ref->image->plane[0].ydim == cmp->image->plane[0].ydim)
    {
        uint32_t p,y,x;
        for (p = 0; p < ref->image->numPlanes; p++)
        {
            // range is in units.
            data->range += ref->image->plane[p].xdim * ref->image->plane[p].ydim;

            for (y = 0;
                 y < ref->image->plane[p].ydim/ref->image->plane[p].yscale;
                 y += 1)
            {
                for (x = 0;
                     x < ref->image->plane[p].xdim / ref->image->plane[p].xscale;
                     x += ref->image->plane[p].xstep)
                {
                    switch (ref->image->plane[p].xstride)
                    {
                        case 1:
                        {
                            uint8_t *pref = image_pixel(ref->image, p, x, y);
                            uint8_t *pcmp = image_pixel(cmp->image, p, x, y);

                            if (pref == NULL || pcmp == NULL)
                                break;

                            if (*pref != *pcmp)
                            {
                                data->numDiff++;
                                data->sumDiff += abs((int32_t)*pcmp - (int32_t)*pref);
                                if (abs(data->maxDiff) < abs((int32_t)*pcmp - (int32_t)*pref))
                                {
                                    data->maxDiff = (int32_t)*pcmp - (int32_t)*pref;
                                }
                            }
                            break;
                        }
                        case 2: // macro pixel or single
                        {
                            uint16_t *pref = (uint16_t *)image_pixel(ref->image, p, x, y);
                            uint16_t *pcmp = (uint16_t *)image_pixel(cmp->image, p, x, y);

                            if (pref == NULL || pcmp == NULL)
                                break;

                            if (*pref != *pcmp)
                            {
                                data->numDiff++;
                                data->sumDiff += abs((int32_t)*pcmp - (int32_t)*pref);
                                if (abs(data->maxDiff) < abs((int32_t)*pcmp - (int32_t)*pref))
                                {
                                    data->maxDiff = (int32_t)*pcmp - (int32_t)*pref;
                                }
                            }
                            break;
                        }
                        case 3:
                            break;
                        case 4:
                            break;
                        case 8:
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
    return 1;
}

int main(int argc, char *argv[])
{
    int equality = image_not_equal;

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

    printf("Image Compare Program (%d)\n", argc);

    if (argc == 3)
    {
        fic_t files[2];

        fic_open(&files[0], argv[1]);
        fic_open(&files[1], argv[2]);

        // skip the first frame
        //fic_read(&files[0]);
        //fic_read(&files[1]);

        while (fic_read(&files[0]) && fic_read(&files[1]))
        {
            fic_compare_t data;
            memset(&data, 0, sizeof(data));
            
            fic_compare(&files[0], &files[1], &data);

            if (data.numDiff > 0)
                printf("Images are not equal: "FMT_UINT64_T" diff, "FMT_INT64_T" max diff, "FMT_UINT64_T" pixels, "FMT_UINT64_T" sum diff, "FMT_UINT64_T" avg diff\n",
                    data.numDiff, data.maxDiff, data.range, data.sumDiff, data.sumDiff/data.numDiff);
            else
                printf("Images are equal\n");
        }
    }
    else
        printf("Usage: $ %s <filename1> <filename2>\n", argv[0]);

    return equality;
}

