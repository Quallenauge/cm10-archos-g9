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

#ifndef _GTK_WINDOW_H_
#define _GTK_WINDOW_H_

#include <sosal/sosal.h>
#include <gtk/gtk.h>

#define GTK_MAX_IMAGES (10)

typedef struct _gtk_window_t {
    GtkWidget *window;
    GtkWidget *draw_area;
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
} GtkWindow_t;

#ifdef __cplusplus
extern "C" {
#endif

void gtkwindow_close(GtkWindow_t *w);
GtkWindow_t *gtkwindow_open(const char *title);

bool_e gtkwindow_free(GtkWindow_t *w);
bool_e gtkwindow_allocate(GtkWindow_t *w, uint32_t width, uint32_t height, fourcc_t color, uint32_t numImages);

void * gtkwindow_acquire(GtkWindow_t *w, uint32_t *index);
bool_e gtkwindow_release(GtkWindow_t *w, void *ptr, uint32_t index);

bool_e gtkwindow_queue(GtkWindow_t *w, uint32_t index);
bool_e gtkwindow_dequeue(GtkWindow_t *w, uint32_t *index);

uint32_t gtkwindow_length(GtkWindow_t *w, uint32_t index);
void *gtkwindow_search_buffer(GtkWindow_t *w, uint32_t index);
uint32_t gtkwindow_search_index(GtkWindow_t * w, void * ptr);

#ifdef __cplusplus
}
#endif

#endif

