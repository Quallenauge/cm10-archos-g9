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
#include <dvp/dvp_debug.h>


//******************************************************************************
// INTERNAL FUNCTIONS
//******************************************************************************

static gboolean on_draw_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
    GtkWindow_t *w = (GtkWindow_t *)user_data;
    DVP_PRINT(DVP_ZONE_VIDEO, "**** RENDERING!!! fb[%u] *****\n", w->fi);
    gdk_draw_rgb_image(widget->window,
                       widget->style->fg_gc[GTK_STATE_NORMAL],
                       0, 0,
                       w->fb[w->fi]->plane[0].xdim,
                       w->fb[w->fi]->plane[0].ydim,
                       GDK_RGB_DITHER_MAX,
                       w->fb[w->fi]->plane[0].ptr,
                       w->fb[w->fi]->plane[0].ystride);
    return TRUE;
}

static thread_ret_t threadGtkRenderer(void *arg)
{
    GtkWindow_t *w = (GtkWindow_t *)arg;
    DVP_PRINT(DVP_ZONE_VIDEO, "GTK: Render Loop Running!\n");
    while (w->bRendering)
    {
        image_t *img = NULL;
        if (queue_read(w->queue, true_e, &img) == true_e)
        {
            if (img != NULL)
            {
                //GdkRectangle rect = {0,0,w->width, w->height};
                //GdkWindow *window = gtk_widget_get_root_window(w->window);
                DVP_PRINT(DVP_ZONE_VIDEO, "GTK: Render Loop Received image_t %p of format 0x%08x!\n", img, img->color);
                //gdk_window_freeze_updates(window);
                w->fi = (w->fi + 1) % dimof(w->fb);
                image_convert(w->fb[w->fi], img);
                //gdk_window_thaw_updates(window);
                //gdk_window_invalidate_rect(window, &rect, TRUE);
                gtk_widget_queue_draw_area(w->draw_area, 0, 0, w->width, w->height);
                queue_write(w->dequeue, false_e, &img);
            }
            else
            {
                DVP_PRINT(DVP_ZONE_ERROR, "GTK: Render loop dequeued a NULL image!\n");
            }
        }
        else if (w->bRendering == false_e)
            break;
    }
    thread_exit(0);
}

static thread_ret_t threadGtkMain(void *arg)
{
    GtkWindow_t *w = (GtkWindow_t *)arg;
    while (w->bRunning)
    {
        DVP_PRINT(DVP_ZONE_VIDEO, "+gtk_main()\n");
        event_set(&w->wait);
        gtk_main();
        DVP_PRINT(DVP_ZONE_VIDEO, "-gtk_main()\n");
    }
    thread_exit(0);
}

//******************************************************************************
// EXTERNAL API
//******************************************************************************

void gtkwindow_close(GtkWindow_t *w)
{
    if (w)
    {
        memset(w, 0, sizeof(GtkWindow_t));
        free(w);
    }
}

GtkWindow_t *gtkwindow_open(const char *title)
{
    GtkWindow_t *w = (GtkWindow_t *)calloc(1, sizeof(GtkWindow_t));
    if (w)
    {
        char *args[] = {
            "--gtk-module",
            "--gtk-debug",
            "--display",
            "--name",
            "--class",
            "--sync"
        };
        char **argv = args;
        int argc = dimof(argv);
        gtk_init(&argc, &argv);
        gdk_rgb_init();
        gdk_rgb_set_verbose(TRUE);
        w->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        DVP_PRINT(DVP_ZONE_VIDEO, "Creating Window %s\n", title);
        gtk_window_set_title(GTK_WINDOW(w->window), (const gchar *)title);
    }
    return w;
}

bool_e gtkwindow_release(GtkWindow_t *w, void *ptr, uint32_t index)
{
    if (w)
    {
        bool_e ret = false_e;
        mutex_lock(&w->lock);
        if (index < w->numImg)
        {
            ret = bitfield_rls(&w->active, index);
        }
        else if (index == 0xFFFFFFFF)
        {
            uint32_t i = 0;
            for (i = 0; i < w->numImg; i++)
            {
                if (w->images[i]->plane[0].ptr == ptr)
                {
                    ret = bitfield_rls(&w->active, i);
                }
            }
        }
        mutex_unlock(&w->lock);
        return ret;
    }
    return false_e;
}

void *gtkwindow_acquire(GtkWindow_t *w, uint32_t *index)
{
    if (w)
    {
        void *ptr = NULL;
        mutex_lock(&w->lock);
        if (bitfield_get(&w->active, index) == false_e)
        {
            DVP_PRINT(DVP_ZONE_VIDEO, "Failed to acquire an image!\n");
        }
        else
        {
            ptr = w->images[*index]->plane[0].ptr;
        }
        mutex_unlock(&w->lock);
        return ptr;
    }
    else
        return NULL;
}

bool_e gtkwindow_free(GtkWindow_t *w)
{
    if (w)
    {
        uint32_t i;
        w->bRendering = false_e;
        queue_pop(w->dequeue);
        queue_pop(w->queue);
        thread_join(w->hRender);
        queue_destroy(w->queue);
        queue_destroy(w->dequeue);

        w->bRunning = false_e;
        gtk_main_quit();
        gtk_widget_destroy(w->window);
        thread_join(w->hMain);

        for (i = 0; i < dimof(w->fb); i++) {
            image_unback(w->fb[i]);
            image_free(&w->fb[i]);
        }
        for (i = 0; i < w->numImg; i++) {
            image_unback(w->images[i]);
            image_free(&w->images[i]);
        }
        free(w->images);
        bitfield_deinit(&w->active);
        event_deinit(&w->wait);
        mutex_deinit(&w->lock);
        return true_e;
    }
    else
        return false_e;
}

bool_e gtkwindow_allocate(GtkWindow_t *w, uint32_t width, uint32_t height, uint32_t count, fourcc_t color)
{
    if (w)
    {
        uint32_t i;
        if (image_query_format(color) == false_e)
        {
            DVP_PRINT(DVP_ZONE_ERROR, "Invalid color space 0x%08x\n", color);
            return false_e;
        }

        mutex_init(&w->lock);
        event_init(&w->wait, true_e);

        // remember values
        w->width = width;
        w->height = height;

        DVP_PRINT(DVP_ZONE_VIDEO, "Allocating %lu Frame Buffer Images\n", dimof(w->fb));
        w->fi = 0;
        for (i = 0; i < dimof(w->fb); i++)
        {
            w->fb[i] = image_allocate(width, height, FOURCC_BGR);
            image_back(w->fb[i]);
            image_fill(w->fb[i], 0xFF, 0x00, 0x00); // RED
        }
        DVP_PRINT(DVP_ZONE_VIDEO, "Allocating %u User Images\n", count);
        w->numImg = count;
        w->images = (image_t **)calloc(count, sizeof(image_t *));
        for (i = 0 ; i < count; i++) {
            w->images[i] = image_allocate(width, height, color);
            image_back(w->images[i]);
            image_print(w->images[i]);
        }
        bitfield_init(&w->active, count);
        w->queue = queue_create(count, sizeof(image_t *));
        w->dequeue = queue_create(count, sizeof(image_t *));

        DVP_PRINT(DVP_ZONE_VIDEO, "Creating GdkDrawingArea (Count=%u)\n", count);
        w->draw_area = gtk_drawing_area_new();
        gtk_widget_set_size_request(w->draw_area, width, height);
        gtk_container_add(GTK_CONTAINER(w->window), w->draw_area);
        gtk_widget_set_app_paintable(w->draw_area, TRUE);
        gtk_signal_connect(GTK_OBJECT(w->draw_area), "expose-event", GTK_SIGNAL_FUNC(on_draw_expose), w);
        gtk_widget_set_events(w->draw_area, GDK_EXPOSURE_MASK);
        gtk_widget_show_all(w->window);

        // Start GTK Main
        w->bRunning = true_e;
        w->hMain = thread_create(threadGtkMain, w);

        event_wait(&w->wait, EVENT_FOREVER);

        // Start GTK Renderer
        w->bRendering = true_e;
        w->hRender = thread_create(threadGtkRenderer, w);

        return true_e;
    }
    else
        return false_e;
}

uint32_t gtkwindow_search_index(GtkWindow_t *w, void *ptr)
{
    if (w)
    {
        uint32_t i;
        for (i = 0; i < w->numImg; i++)
        {
            if (w->images[i]->plane[0].ptr == ptr)
                break;
        }
        if (i == w->numImg)
            return 0xFFFFFFFF;
        return i;
    }
    else
        return 0xFFFFFFFF;
}

void *gtkwindow_search_buffer(GtkWindow_t *w, uint32_t index)
{
    if (w && index < w->numImg)
        return w->images[index]->plane[0].ptr;
    else
        return NULL;
}

bool_e gtkwindow_queue(GtkWindow_t *w, uint32_t index)
{
    if (w && index < w->numImg)
    {
        DVP_PRINT(DVP_ZONE_VIDEO, "Queueing %u %p\n", index, w->images[index]);
        return queue_write(w->queue, false_e, &w->images[index]);
    }
    else
        return false_e;
}

bool_e gtkwindow_dequeue(GtkWindow_t *w, uint32_t *index)
{
    if (w)
    {
        image_t *img;
        bool_e ret = false_e;
        //DVP_PRINT(DVP_ZONE_VIDEO, "Attempting to dequeue from GTK\n");
        ret = queue_read(w->dequeue, true_e, &img);
        if (ret)
        {
            DVP_PRINT(DVP_ZONE_VIDEO, "Dequeued %p\n", img);
            *index = gtkwindow_search_index(w, img->plane[0].ptr);
            DVP_PRINT(DVP_ZONE_VIDEO, "Dequeueing %u\n", *index);
        }
        return ret;
    }
    else
        return false_e;
}

uint32_t gtkwindow_length(GtkWindow_t *w, uint32_t index)
{
    if (w)
    {
        uint32_t len=0, p = 0;
        for (p = 0; p < w->images[index]->numPlanes; p++)
        {
            len += w->images[index]->plane[p].numBytes;
        }
        return len;
    }
    else
        return 0;
}



