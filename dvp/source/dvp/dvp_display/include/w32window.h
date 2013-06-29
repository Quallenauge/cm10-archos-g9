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

#ifndef _W32_WINDOW_H_
#define _W32_WINDOW_H_

#include <sosal/sosal.h>

#define W32_MAX_IMAGES (10)

typedef struct _w32_window_t {
    WNDCLASS   class;
    HWND       window;
    ATOM       atom;
    TCHAR      className[MAX_PATH];
    TCHAR      windowName[MAX_PATH];
    uint32_t   width;
    uint32_t   height;
    image_t   *fb[2];   /**< Internal framebuffers */
    uint32_t   fi;      /**< active frame buffer index */
    image_t  **images;
    bitfield_t active;
    queue_t   *queue;
    queue_t   *dequeue;
    uint32_t   curImg;
    uint32_t   numImg;
    mutex_t    lock;
    bool_e     bRunning;
    thread_t   hMain;
    bool_e     bRendering;
    thread_t   hRender;
    event_t    wait;
} W32Window_t;

#ifdef __cplusplus
extern "C" {
#endif

void w32window_close(W32Window_t *w);
W32Window_t *w32window_open(const char *title);

bool_e w32window_free(W32Window_t *w);
bool_e w32window_allocate(W32Window_t *w, uint32_t width, uint32_t height, fourcc_t color, uint32_t numImages);

void * w32window_acquire(W32Window_t *w, uint32_t *index);
bool_e w32window_release(W32Window_t *w, void *ptr, uint32_t index);

bool_e w32window_queue(W32Window_t *w, uint32_t index);
bool_e w32window_dequeue(W32Window_t *w, uint32_t *index);

uint32_t w32window_length(W32Window_t *w, uint32_t index);
void *w32window_search_buffer(W32Window_t *w, uint32_t index);
uint32_t w32window_search_index(W32Window_t * w, void * ptr);

#ifdef __cplusplus
}
#endif

#endif

