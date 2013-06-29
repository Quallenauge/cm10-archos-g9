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

#include <gtkwindow.h>
#include <sosal/sosal.h>
#include <dvp/dvp_debug.h>
#include <sosal/pyuv.h>

long flen(FILE *f)
{
    long new_, old = ftell(f);
    fseek(f, 0, SEEK_END);
    new_ = ftell(f);
    fseek(f, old, SEEK_SET);
    return new_;
}

uint32_t dvp_zone_mask = 0xFFFF; // declare a local version for testing

int main(int argc, char *argv[])
{
    uint32_t count = 4;

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

    if (argc == 1)
    {
        GtkWindow_t *w = gtkwindow_open("GTK Test");
        gtkwindow_allocate(w, 640, 480, count, FOURCC_RGB);
        thread_msleep(5000);
        gtkwindow_free(w);
        gtkwindow_close(w);
    }
    else if (argc == 5)
    {
        char filename[MAX_PATH];
        char *name = argv[1];
        uint32_t width = atoi(argv[2]);
        uint32_t height = atoi(argv[3]);
        fourcc_t color = *(fourcc_t *)argv[4];
        PYUV_GetFilename(filename, "", name, width, height, 30, color);
        printf("Opening %s\n", filename);
        FILE *fp = fopen(filename, "rb");
        if (fp)
        {
            uint32_t numFrames, frameSize, f, i = 0;
            GtkWindow_t *w = gtkwindow_open("GTK Renderer");
            gtkwindow_allocate(w, width, height, count, color);
            frameSize = gtkwindow_length(w, 0);
            numFrames = flen(fp)/frameSize;
            DVP_PRINT(DVP_ZONE_ALWAYS, "There are %u frames in the file!\n", numFrames);
            for (f = 0; f < numFrames; f++)
            {
                uint32_t index;
                void *ptr = gtkwindow_acquire(w, &index);
                size_t b = fread(ptr, 1, frameSize, fp);
                DVP_PRINT(DVP_ZONE_ALWAYS, "Read "FMT_SIZE_T" bytes from file!\n", b);
                if (gtkwindow_queue(w, index)) {
                    gtkwindow_dequeue(w, &index); // blocking call
                    DVP_PRINT(DVP_ZONE_ALWAYS, "Dequeued index %u, had ptr %p\n", index, ptr);
                    gtkwindow_release(w, ptr, index);
                }
                thread_msleep(33);
            }
            gtkwindow_free(w);
            gtkwindow_close(w);
            fclose(fp);
        }
    }
    return 0;
}
