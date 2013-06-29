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
#include <sosal/sosal.h>
#include <sosal/pyuv.h>

HINSTANCE g_hInstance;

#define DVP_CONSOLE
#include <dvp/dvp_debug.h>

thread_ret_t threadMessages(void *arg)
{
    MSG Msg;
    DVP_PRINT(DVP_ZONE_VIDEO, "Windows Messaging Thread Started!\n");
    while(GetMessage(&Msg, NULL, 0, 0))
    {
        DVP_PRINT(DVP_ZONE_VIDEO, "MSG THREAD: HWND:%p msg:%u\n", Msg.hwnd, Msg.message);
        TranslateMessage(&Msg);
        DispatchMessage( &Msg);
    }
    DVP_PRINT(DVP_ZONE_VIDEO, "Windows Messaging Thread Stopping!\n");
    thread_exit((int)(Msg.wParam));
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
    uint32_t i,count = 4,iter = 1000;
    uint32_t width = 640;
    uint32_t height = 480;
    uint8_t *ptr = NULL;
    W32Window_t *w = NULL;
    image_t *rgb = NULL;
    thread_t msgs = NULL;
    char c;

    AllocConsole();

    DVP_PRINT(DVP_ZONE_ALWAYS, "Starting in WinMain(0x%08x)\n", hInstance);

    rgb = image_allocate(width, height, FOURCC_RGB);
    image_back(rgb);
    ptr = rgb->plane[0].ptr; // save it
    g_hInstance = hInstance;
    w = w32window_open("W32Test");
    if (w32window_allocate(w, width, height, count, FOURCC_RGB))
    {
        msgs = thread_create(threadMessages, NULL);

        for (i = 0; i < iter; i++)
        {
            uint32_t index;
            void *buffer = w32window_acquire(w, &index);
            if (buffer != NULL)
            {
                rgb->plane[0].ptr = (uint8_t *)buffer;
                image_fill(rgb, 0x34, 0x00, 0xD0);
                if (w32window_queue(w, index))
                {
                    thread_msleep(33);

                    DVP_PRINT(DVP_ZONE_ALWAYS, "Press a button to quit\n");
                    ReadConsoleA(GetStdHandle(STD_INPUT_HANDLE), &c, 1, NULL, NULL);

                    if (w32window_dequeue(w, &index))
                    {
                        w32window_release(w, buffer, index);
                    }
                }
            }
        }
        rgb->plane[0].ptr = ptr;
        image_unback(rgb);
        image_free(&rgb);
        w32window_free(w);
    }
    w32window_close(w);

    DVP_PRINT(DVP_ZONE_ALWAYS, "Press a button to quit\n");
    ReadConsoleA(GetStdHandle(STD_INPUT_HANDLE), &c, 1, NULL, NULL);

    FreeConsole();

    return 9;
}

