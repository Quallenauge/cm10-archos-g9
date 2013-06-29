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

#include <w32window.h>
#define DVP_CONSOLE
#include <dvp/dvp_debug.h>

extern HINSTANCE g_hInstance;

//******************************************************************************
// INTERNAL FUNCTIONS
//******************************************************************************

static thread_ret_t threadw32Renderer(void *arg)
{
    W32Window_t *w = (W32Window_t *)arg;
    DVP_PRINT(DVP_ZONE_VIDEO, "w32: Render Loop Running!\n");
    while (w->bRendering)
    {
        image_t *img = NULL;
        if (queue_read(w->queue, true_e, &img) == true_e)
        {
            if (img != NULL)
            {
                RECT rect = {0, 0, w->width, w->height};
                DVP_PRINT(DVP_ZONE_VIDEO, "w32: Render Loop Received image_t %p of format 0x%08x!\n", img, img->color);
                w->fi = (w->fi + 1) % dimof(w->fb);
                image_convert(w->fb[w->fi], img);
                //InvalidateRect(w->window, NULL, TRUE);
                //if (UpdateWindow(w->window) == TRUE)
                if (RedrawWindow(w->window, &rect, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE) == FALSE)
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "Failed to redraw (error=%d)\n", GetLastError());
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_VIDEO, "Redraw returned success!\n");
                }
                queue_write(w->dequeue, false_e, &img);
            }
            else
            {
                DVP_PRINT(DVP_ZONE_ERROR, "w32: Render loop dequeued a NULL image!\n");
            }
        }
        else if (w->bRendering == false_e)
            break;
    }
    thread_exit(0);
}

// of type WNDPROC
static LRESULT w32window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lr = 0;
    HDC hDC = 0;
    static W32Window_t *w = NULL;
    DVP_PRINT(DVP_ZONE_VIDEO, "HWND %p msg:%u wParam:%p lParam:%p\n", hwnd, uMsg, wParam, lParam);
    switch (uMsg)
    {
        case WM_DESTROY:
            PostQuitMessage(WM_QUIT);
            break;
        case WM_CREATE:
        {
            CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
            w = cs->lpCreateParams;
            DVP_PRINT(DVP_ZONE_VIDEO, "Saved W32Window_t *%p\n", w);
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            uint32_t x,y;
            image_t *img = w->fb[w->fi];
            hDC = BeginPaint(hwnd, &ps);
            for (y = 0; y < img->plane[0].ydim; y++)
            {
                for (x = 0; x < img->plane[0].xdim; x+=img->plane[0].xstep)
                {
                    uint8_t *rgb = image_pixel(img, 0, x, y);
                    COLORREF pixel = RGB(rgb[0],rgb[1],rgb[2]);
                    SetPixel(hDC, x, y, pixel);
                }
            }
            EndPaint(hwnd, &ps);
            break;
        }
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return lr;
}

//******************************************************************************
// EXTERNAL API
//******************************************************************************

void w32window_close(W32Window_t *w)
{
    if (w)
    {
        UnregisterClass(w->className, w->class.hInstance);
        memset(w, 0, sizeof(W32Window_t));
        free(w);
    }
}

W32Window_t *w32window_open(const char *title)
{
    W32Window_t *w = (W32Window_t *)calloc(1, sizeof(W32Window_t));
    //DVP_PRINT(DVP_ZONE_VIDEO, "%s\n", __FUNCTION__);
    if (w)
    {
        char *className = "W32Window";
        w->class.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
        w->class.lpfnWndProc = (WNDPROC)w32window_proc;
        w->class.cbClsExtra = 0;
        w->class.cbWndExtra = 0;
        w->class.hInstance = g_hInstance;
        w->class.hIcon = LoadIcon(g_hInstance, IDI_APPLICATION);
        w->class.hCursor = LoadCursor(g_hInstance, IDC_ARROW);
        w->class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
        w->class.lpszClassName = w->className;
        w->class.lpszMenuName = NULL;
#ifdef UNICODE
        mbstowcs(w->className, className, strlen(className));
        mbstowcs(w->windowName, title, strlen(title));
#else
        strncpy(w->className, className, strlen(className));
        strncpy(w->windowName, title, strlen(title));
#endif
        w->atom = RegisterClass(&w->class);
        if (w->atom)
        {
            DVP_PRINT(DVP_ZONE_VIDEO, "Creating Window %s:%s\n", className, title);
        }
        else
        {
            DVP_PRINT(DVP_ZONE_ERROR, "Could not RegisterClass (error=%d)\n", GetLastError());
            free(w);
            w = NULL;
        }
    }
    return w;
}

bool_e w32window_release(W32Window_t *w, void *ptr, uint32_t index)
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

void *w32window_acquire(W32Window_t *w, uint32_t *index)
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

bool_e w32window_free(W32Window_t *w)
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
        DestroyWindow(w->window);
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

bool_e w32window_allocate(W32Window_t *w, uint32_t width, uint32_t height, uint32_t count, fourcc_t color)
{
    if (w)
    {
        uint32_t i;

        DVP_PRINT(DVP_ZONE_VIDEO, "Creating W32Window\n");
        w->window = CreateWindow(w->className,
                                 w->windowName,
                                 WS_VISIBLE | WS_OVERLAPPED,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 width, height,
                                 NULL,
                                 NULL,
                                 w->class.hInstance,
                                 w);
        if (w->window == NULL)
        {
            DVP_PRINT(DVP_ZONE_ERROR, "Failed to create Windows (error=%d)\n",GetLastError());
            return false_e;
        }
        else
        {
            if (image_query_format(color) == false_e)
                return false_e;

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

            // Start w32 Renderer
            w->bRendering = true_e;
            w->hRender = thread_create(threadw32Renderer, w);

            return true_e;
        }
    }
    else
        return false_e;
}

uint32_t w32window_search_index(W32Window_t *w, void *ptr)
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

void *w32window_search_buffer(W32Window_t *w, uint32_t index)
{
    if (w && index < w->numImg)
        return w->images[index]->plane[0].ptr;
    else
        return NULL;
}

bool_e w32window_queue(W32Window_t *w, uint32_t index)
{
    if (w && index < w->numImg)
    {
        DVP_PRINT(DVP_ZONE_VIDEO, "Queueing %u %p\n", index, w->images[index]);
        return queue_write(w->queue, false_e, &w->images[index]);
    }
    else
        return false_e;
}

bool_e w32window_dequeue(W32Window_t *w, uint32_t *index)
{
    if (w)
    {
        image_t *img;
        bool_e ret = false_e;
        DVP_PRINT(DVP_ZONE_VIDEO, "Attempting to dequeue from w32\n");
        ret = queue_read(w->dequeue, true_e, &img);
        if (ret)
        {
            DVP_PRINT(DVP_ZONE_VIDEO, "Dequeued %p\n", img);
            *index = w32window_search_index(w, img->plane[0].ptr);
            DVP_PRINT(DVP_ZONE_VIDEO, "Dequeueing %u\n", *index);
        }
        return ret;
    }
    else
        return false_e;
}

uint32_t w32window_length(W32Window_t *w, uint32_t index)
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



