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

#include <sosal/uinput.h>
#include <sosal/thread.h>
#include <sosal/debug.h>

#ifdef UINPUT_TEST
#undef SOSAL_ZONE_MASK
#define SOSAL_ZONE_MASK 0xFFFFFFFF
#endif

#if defined(_UINPUT_T_FUNCS)
#if defined(ANDROID) || defined(LINUX)

void set_input_event(struct input_event *ie, int type, int code, int data)
{
    memset(ie, 0, sizeof(struct input_event));
    ie->type = type;
    ie->code = code;
    ie->value = data;
}

#define UINPUT_SUPPORT_TOUCHSCREEN
#undef PLATFORM_HAS_MT
static const char *uidrv_names[] = {
    "/dev/uinput",
    "/dev/input/uinput",
    "/dev/misc/uintput",
};
static int numDrvNames = dimof(uidrv_names);

uinput_t *uinput_open(char *dev_name)
{
    uinput_t *p = (uinput_t *)malloc(sizeof(uinput_t));
    if (p != NULL)
    {
        int i;
        memset(p, 0, sizeof(uinput_t));
        for (i = 0; i < numDrvNames; i++)
        {
            p->ufile = open(uidrv_names[i], O_WRONLY|O_NDELAY);
            if (p->ufile == -1)
            {
                SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to open %s\n",uidrv_names[i]);
            }
            else
            {
                int retcode = 0;

                strncpy(p->uud.name, dev_name, UINPUT_MAX_NAME_SIZE);
                p->uud.id.version = 1;
                p->uud.id.product = 0x9000;
                p->uud.id.vendor = 0x0451; // TI's Vendor Code.
                p->uud.id.bustype = BUS_USB;

#ifdef UINPUT_SUPPORT_TOUCHSCREEN
#ifndef PLATFORM_HAS_MT
                p->uud.absmax[ABS_X] = UINPUT_MAX_TOUCH_X;
                p->uud.absmax[ABS_Y] = UINPUT_MAX_TOUCH_Y;
                p->uud.absmax[ABS_Z] = 255;
                p->uud.absmax[ABS_PRESSURE] = 255;
                p->uud.absmax[ABS_RZ] = 255;
                p->uud.absmax[ABS_HAT0X] = 255;
                p->uud.absmax[ABS_HAT0Y] = 255;
                p->uud.absmax[ABS_HAT1X] = 255;
                p->uud.absmax[ABS_HAT1Y] = 255;
                p->uud.absmax[ABS_TOOL_WIDTH] = 15;
                p->uud.absmax[ABS_VOLUME] = 255;
#endif
#endif
                retcode = write(p->ufile, &p->uud, sizeof(p->uud));
                if (retcode != sizeof(p->uud))
                {
                    SOSAL_PRINT(SOSAL_ZONE_ERROR, "Error! Write only wrote %d bytes (errno=%d) \n",retcode, errno);
                    close(p->ufile);
                    p->ufile = 0;
                    free(p);
                    p = NULL;
                }
                else
                {
                    // Configure the types of bits we can process
                    ioctl(p->ufile, UI_SET_EVBIT, EV_SYN);      // Synchronize
                    ioctl(p->ufile, UI_SET_EVBIT, EV_KEY);      // Keyboards/Mice/Touchscreen
                    ioctl(p->ufile, UI_SET_EVBIT, EV_REL);      // Mice
                    ioctl(p->ufile, UI_SET_EVBIT, EV_ABS);      // Touchscreen
                    for (i = 0; i < 256; i++)
                        ioctl(p->ufile, UI_SET_KEYBIT, i);      // All 8 bit chars

                    ioctl(p->ufile, UI_SET_RELBIT, REL_X);      // Mice
                    ioctl(p->ufile, UI_SET_RELBIT, REL_Y);      // Mice
                    ioctl(p->ufile, UI_SET_KEYBIT, BTN_MOUSE);  // Mice
#ifdef UINPUT_SUPPORT_TOUCHSCREEN
#ifndef PLATFORM_HAS_MT
                    ioctl(p->ufile, UI_SET_ABSBIT, ABS_X);          // Touchscreen
                    ioctl(p->ufile, UI_SET_ABSBIT, ABS_Y);          // Touchscreen
                    //ioctl(p->ufile, UI_SET_ABSBIT, ABS_Z);          // Touchscreen
                    //ioctl(p->ufile, UI_SET_ABSBIT, ABS_RZ);         // Touchscreen
                    //ioctl(p->ufile, UI_SET_ABSBIT, ABS_HAT0X);      // Touchscreen
                    //ioctl(p->ufile, UI_SET_ABSBIT, ABS_HAT0Y);      // Touchscreen
                    //ioctl(p->ufile, UI_SET_ABSBIT, ABS_HAT1X);      // Touchscreen
                    //ioctl(p->ufile, UI_SET_ABSBIT, ABS_HAT1Y);      // Touchscreen
                    //ioctl(p->ufile, UI_SET_ABSBIT, ABS_TOOL_WIDTH); // Touchscreen
                    //ioctl(p->ufile, UI_SET_ABSBIT, ABS_VOLUME);     // Touchscreen
                    //ioctl(p->ufile, UI_SET_ABSBIT, ABS_PRESSURE);   // Touchscreen
                    ioctl(p->ufile, UI_SET_KEYBIT, BTN_TOUCH);      // Touchscreen
#else
                    ioctl(p->ufile, UI_SET_ABSBIT, ABS_MT_SLOT);
                    ioctl(p->ufile, UI_SET_ABSBIT, ABS_MT_TOUCH_MAJOR); // Multitouch
                    ioctl(p->ufile, UI_SET_ABSBIT, ABS_MT_TOUCH_MINOR); // Multitouch
                    ioctl(p->ufile, UI_SET_ABSBIT, ABS_MT_WIDTH_MAJOR); // Multitouch
                    ioctl(p->ufile, UI_SET_ABSBIT, ABS_MT_WIDTH_MINOR); // Multitouch
                    ioctl(p->ufile, UI_SET_ABSBIT, ABS_MT_ORIENTATION); // Multitouch
                    ioctl(p->ufile, UI_SET_ABSBIT, ABS_MT_POSITION_X);  // Multitouch
                    ioctl(p->ufile, UI_SET_ABSBIT, ABS_MT_POSITION_Y);  // Multitouch
                    ioctl(p->ufile, UI_SET_ABSBIT, ABS_MT_TOOL_TYPE);   // Multitouch
                    ioctl(p->ufile, UI_SET_ABSBIT, ABS_MT_BLOB_ID);     // Multitouch
                    ioctl(p->ufile, UI_SET_ABSBIT, ABS_MT_TRACKING_ID); // Multitouch
                    ioctl(p->ufile, UI_SET_ABSBIT, ABS_MT_PRESSURE);    // Multitouch
#endif
#endif
                    retcode = ioctl(p->ufile, UI_DEV_CREATE);
                    if (retcode != 0)
                    {
                        SOSAL_PRINT(SOSAL_ZONE_ERROR, "Error! ioctl() => %d\n",retcode);
                        close(p->ufile);
                        p->ufile = 0;
                        free(p);
                        p = NULL;
                    }
                    else
                    {
                        SOSAL_PRINT(SOSAL_ZONE_UINPUT, "Create %s {%04x:%04x}\n", p->uud.name,p->uud.id.vendor,p->uud.id.product);
                    }
                }
                break;
            }
        }
        if (i == numDrvNames)
        {
            SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to open any uinput device\n");
            free(p);
            p = NULL;
        }
    }
    if (p == NULL) {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "Failed to open or configure the uinput device!\n");
    }
    return p;
}

bool_e uinput_close(uinput_t *p)
{
    if (p != NULL && p->ufile != 0)
    {
        ioctl(p->ufile, UI_DEV_DESTROY);
        close(p->ufile);
        p->ufile = 0;
    }
    if (p != NULL)
    {
        free(p);
        p = NULL;
    }
    return true_e;
}

static bool_e send_input_event(int descriptor, uint16_t type, uint16_t code, int32_t value, struct timeval *t)
{
    struct input_event event;
    size_t actual;

    memset(&event, 0, sizeof(event));
    t = t; //memcpy(&event.time, t, sizeof(struct timeval));
    event.type = type;
    event.code = code;
    event.value = value;
    SOSAL_PRINT(SOSAL_ZONE_UINPUT, "uinput => [%04x, %04x, %08x] @ %lu:%lu\n", event.type, event.code, event.value, event.time.tv_sec, event.time.tv_usec);
    actual = write(descriptor, &event, sizeof(event));
    if (actual == sizeof(event))
        return true_e;
    else
        return false_e;
}

bool_e uinput_send(uinput_t *p, input_t *in)
{
    if (p != NULL && p->ufile > 0)
    {
        struct timeval t;
        bool_e ret = false_e;

        gettimeofday(&t, NULL);

        if (in->type == INPUT_TYPE_KEY)
        {
            ret = send_input_event(p->ufile, EV_KEY, in->data.key.code, in->data.key.state, &t);
            ret = send_input_event(p->ufile, EV_SYN, SYN_REPORT, 0, &t);
        }
        if (in->type == INPUT_TYPE_MOUSE)
        {
            ret = send_input_event(p->ufile, EV_REL, REL_X, in->data.mouse.dx, &t);
            ret = send_input_event(p->ufile, EV_REL, REL_Y, in->data.mouse.dx, &t);
            ret = send_input_event(p->ufile, EV_SYN, SYN_REPORT, 0, &t);
        }
#ifdef UINPUT_SUPPORT_TOUCHSCREEN
        if (in->type == INPUT_TYPE_TOUCH)
        {
            int touched = (in->data.touch.pressure > 0 ? 1 : 0);
#ifndef PLATFORM_HAS_MT
            ret = send_input_event(p->ufile, EV_ABS, ABS_X, in->data.touch.location_x, &t);
            ret = send_input_event(p->ufile, EV_ABS, ABS_Y, in->data.touch.location_y, &t);
            /*
            ret = send_input_event(p->ufile, EV_ABS, ABS_Z, in->data.touch.pressure, &t);
            ret = send_input_event(p->ufile, EV_ABS, ABS_RZ, 0, &t);
            ret = send_input_event(p->ufile, EV_ABS, ABS_HAT0X, 0, &t);
            ret = send_input_event(p->ufile, EV_ABS, ABS_HAT0Y, 0, &t);
            ret = send_input_event(p->ufile, EV_ABS, ABS_HAT1X, 0, &t);
            ret = send_input_event(p->ufile, EV_ABS, ABS_HAT1Y, 0, &t);
            ret = send_input_event(p->ufile, EV_ABS, ABS_TOOL_WIDTH, in->data.touch.major_width * in->data.touch.minor_width, &t);
            ret = send_input_event(p->ufile, EV_ABS, ABS_VOLUME, 0, &t);
            */
            ret = send_input_event(p->ufile, EV_KEY, BTN_TOUCH, touched, &t);
            //ret = send_input_event(p->ufile, EV_ABS, ABS_PRESSURE, in->data.touch.pressure, &t);
            ret = send_input_event(p->ufile, EV_SYN, SYN_REPORT, 0, &t);
#else
            if (touched)
            {
                ret = send_input_event(p->ufile, EV_ABS, ABS_MT_POSITION_X, in->data.touch.location_x, &t);
                ret = send_input_event(p->ufile, EV_ABS, ABS_MT_POSITION_Y, in->data.touch.location_y, &t);
                ret = send_input_event(p->ufile, EV_ABS, ABS_MT_PRESSURE, in->data.touch.pressure, &t);
                ret = send_input_event(p->ufile, EV_ABS, ABS_MT_TOUCH_MAJOR, in->data.touch.major_width, &t);
                ret = send_input_event(p->ufile, EV_ABS, ABS_MT_TOUCH_MINOR, in->data.touch.minor_width, &t);
                ret = send_input_event(p->ufile, EV_ABS, ABS_MT_TOOL_TYPE, MT_TOOL_FINGER, &t);
            }
            ret = send_input_event(p->ufile, EV_SYN, SYN_MT_REPORT, 0, &t);
            ret = send_input_event(p->ufile, EV_SYN, SYN_REPORT, 0, &t);
#endif
        }
#endif
        if (in->type == INPUT_TYPE_JOYSTICK)
        {
            ret = send_input_event(p->ufile, EV_ABS, ABS_X, in->data.joy.vx, &t);
            ret = send_input_event(p->ufile, EV_ABS, ABS_Y, in->data.joy.vy, &t);
            ret = send_input_event(p->ufile, EV_SYN, SYN_REPORT, 0, &t);
        }
        return ret;
    }
    else
    {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR: uinput device is not initialized!\n");
        return false_e;
    }
}

#elif defined(WIN32) || defined(UNDER_CE) || defined(CYGWIN)
#include <winuser.h>
typedef struct _find_window_t {
    char *strName;
    HWND  hTarget;
} FindWindow_t;

BOOL CALLBACK FindWindowHandleCallback(HWND hWindow, LPARAM lParam)
{
    FindWindow_t *fWin = (FindWindow_t *)lParam;
    char className[MAX_PATH];
    char windowName[MAX_PATH];

    GetClassName(hWindow, className,sizeof(className));
    GetWindowText(hWindow, windowName,sizeof(windowName));

    if (className[0] == 0)
        return FALSE;

    SOSAL_PRINT(SOSAL_ZONE_UINPUT, "Window HWND %p %s:%s\n", hWindow, className, windowName);
    if (strcmp(fWin->strName, className) == 0)
    {
        fWin->hTarget = hWindow;
        return FALSE;
    }
    return TRUE;
}

HWND FindWindowByName(char *pName)
{
    FindWindow_t fWin;

    fWin.strName = pName;
    fWin.hTarget = NULL;

    if (EnumWindows(FindWindowHandleCallback, (LPARAM)&fWin))
    {
        if (fWin.hTarget != NULL) {
            SOSAL_PRINT(SOSAL_ZONE_UINPUT, "Found %s at %p\n", fWin.strName, fWin.hTarget);
        }
    }
    return fWin.hTarget;
}

uinput_t *uinput_open(char *dev_name)
{
    uinput_t *p = (uinput_t *)malloc(sizeof(uinput_t));
    if (p != NULL)
    {
        // find the requested window
        p->hWindow = FindWindowByName(dev_name);

        if (p->hWindow == NULL)
        {
            SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! Window %s not found!\n",dev_name);
            free(p);
            p = NULL;
        }
        else
        {
            SOSAL_PRINT(SOSAL_ZONE_UINPUT, "Retrievd HWND %p!\n", p->hWindow);
        }
    }
    return p;
}

bool_e uinput_close(uinput_t *p)
{
    if (p != NULL)
    {
        free(p);
    }
    return true_e;
}

bool_e uinput_send(uinput_t *p, input_t *in)
{
    INPUT input;

    memset(&input, 0, sizeof(input));

    if (in->type == INPUT_TYPE_KEY)
    {
        SetForegroundWindow(p->hWindow);
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = in->data.key.code;
        input.ki.wScan = 0;
        if (in->data.key.state == INPUT_KEY_STATE_UP)
            input.ki.dwFlags = KEYEVENTF_KEYUP;
        else
            input.ki.dwFlags = 0;
        input.ki.time = 0;
        input.ki.dwExtraInfo = GetMessageExtraInfo();
        SOSAL_PRINT(SOSAL_ZONE_UINPUT, "Keyboard: 0x%04x flags: 0%08x\n",input.ki.wVk,input.ki.dwFlags);
    }
    else if (in->type == INPUT_TYPE_MOUSE)
    {
        SetActiveWindow(p->hWindow);
        SetFocus(p->hWindow);
        input.type = INPUT_MOUSE;
        input.mi.dx = in->data.mouse.dx;
        input.mi.dy = in->data.mouse.dy;
        input.mi.mouseData = 0;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        input.mi.dwExtraInfo = GetMessageExtraInfo();
        SOSAL_PRINT(SOSAL_ZONE_UINPUT, "Mouse: (%d, %d} => {%d, %d}\n", in->data.mouse.dx, in->data.mouse.dy,input.mi.dx, input.mi.dy);
    }
    else if (input.type == INPUT_HARDWARE)
    {
        //input.hi.
    }
    return (int)SendInput(1, &input, sizeof(input));
}

#elif defined(__QNX__)

uinput_t *uinput_open(char *dev_name)
{
    int err;
    uinput_t *p = (uinput_t *)calloc(1, sizeof(uinput_t));
    if (p)
    {
        int nDisplays = 0;
        screen_display_t *displays = NULL;
        err = screen_create_context(&p->context, SCREEN_INPUT_PROVIDER_CONTEXT);
        err = screen_get_context_property_iv(p->context, SCREEN_PROPERTY_DISPLAY_COUNT, &nDisplays);
        displays = (screen_display_t *)calloc(nDisplays, sizeof(screen_display_t *));
        err = screen_get_context_property_pv(p->context, SCREEN_PROPERTY_DISPLAYS, (void **)displays);
        p->display = displays[0]; /// @todo take the first one?
    }
    return p;
}

bool_e uinput_close(uinput_t * p)
{
    if (p)
    {
        screen_destroy_context(p->context);
        free(p);
    }
    return true_e;
}

bool_e uinput_send(uinput_t * p, input_t * in)
{
    if (p)
    {
        if (in->type == INPUT_TYPE_KEY)
        {
            int err, val;
            bool_e ret = true_e;
            screen_event_t ev;
            err = screen_create_event(&ev);
            if (err) ret = false_e;
            val = SCREEN_EVENT_KEYBOARD;
            err = screen_set_event_property_iv(ev, SCREEN_PROPERTY_TYPE, &val);
            if (err) ret = false_e;
            val = KEY_SYM_VALID;
            if (in->data.key.state == INPUT_KEY_STATE_DOWN)
                val |= KEY_DOWN;
            err = screen_set_event_property_iv(ev, SCREEN_PROPERTY_KEY_FLAGS, &val);
            if (err) ret = false_e;
            val = in->data.key.code;
            err = screen_set_event_property_iv(ev, SCREEN_PROPERTY_KEY_SYM, &val);
            if (err) ret = false_e;
            err = screen_inject_event(p->display, ev);
            if (err) ret = false_e;
            screen_destroy_event(ev);
            SOSAL_PRINT(SOSAL_ZONE_UINPUT, "Key inject err=%d\n", err);
            return ret;
        }
        else
            return false_e;
    }
    else
        return false_e;
}

#endif

typedef struct _ascii_to_key_t {
    uint16_t key;
    char  ascii;
}  ASCII_to_Key_t;

ASCII_to_Key_t ktable[] = {
#if defined(LINUX)
    {KEY_1, '1'},{KEY_2, '2'},{KEY_3, '3'},{KEY_4, '4'},{KEY_5, '5'},{KEY_6, '6'},
    {KEY_7, '7'},{KEY_8, '8'},{KEY_9, '9'},{KEY_Q, 'Q'},{KEY_W, 'W'},{KEY_E, 'E'},
    {KEY_R, 'R'},{KEY_T, 'T'},{KEY_Y, 'Y'},{KEY_U, 'U'},{KEY_I, 'I'},{KEY_O, 'O'},
    {KEY_P, 'P'},{KEY_A, 'A'},{KEY_S, 'S'},{KEY_D, 'D'},{KEY_F, 'F'},{KEY_G, 'G'},
    {KEY_H, 'H'},{KEY_J, 'J'},{KEY_K, 'K'},{KEY_L, 'L'},{KEY_Z, 'Z'},{KEY_X, 'X'},
    {KEY_C, 'C'},{KEY_V, 'V'},{KEY_B, 'B'},{KEY_N, 'N'},{KEY_M, 'M'},{KEY_SPACE, ' '},
    {KEY_MINUS,'-'},{KEY_TAB,'\t'},{KEY_ENTER,'\n'},
#elif defined(WIN32) || defined(UNDER_CE) || defined(CYGWIN)
    {VK_TAB, '\t'},
    {VK_RETURN, '\n'},
    {VK_CLEAR, '\r'},
    {VK_SPACE, ' '},
    {VK_OEM_4, '['},
    {VK_OEM_6, ']'},
#elif defined(__QNX__)
    {KEYCODE_ONE, '1'},{KEYCODE_TWO, '2'},{KEYCODE_THREE, '3'},{KEYCODE_FOUR, '4'},{KEYCODE_FIVE, '5'},
    {KEYCODE_SIX, '6'},{KEYCODE_SEVEN, '7'},{KEYCODE_EIGHT, '8'},{KEYCODE_NINE, '9'},{KEYCODE_ZERO, '0'},
    {KEYCODE_A, 'A'},{KEYCODE_B, 'B'},{KEYCODE_C, 'C'},{KEYCODE_D, 'D'},{KEYCODE_E, 'E'},
    {KEYCODE_F, 'F'},{KEYCODE_G, 'G'},{KEYCODE_H, 'H'},{KEYCODE_I, 'I'},{KEYCODE_J, 'J'},
    {KEYCODE_K, 'K'},{KEYCODE_L, 'L'},{KEYCODE_M, 'M'},{KEYCODE_N, 'N'},{KEYCODE_O, 'O'},
    {KEYCODE_P, 'P'},{KEYCODE_Q, 'Q'},{KEYCODE_R, 'R'},{KEYCODE_S, 'S'},{KEYCODE_T, 'T'},
    {KEYCODE_U, 'U'},{KEYCODE_V, 'V'},{KEYCODE_W, 'W'},{KEYCODE_X, 'X'},{KEYCODE_Y, 'Y'},
    {KEYCODE_Z, 'Z'},
#endif
};
uint32_t numKTable = dimof(ktable);

static uint16_t GetKeyFromASCII(char k)
{
    uint32_t i;
    uint16_t key;
#if defined(LINUX)
    key = KEY_QUESTION;
    for (i = 0; i < numKTable; i++)
    {
        if (k == ktable[i].ascii)
        {
            key = ktable[i].key;
            break;
        }
    }
#elif defined(WIN32) || defined(UNDER_CE) || defined(CYGWIN)
    key = VK_SPACE;
    if (isdigit(k))
        return (uint16_t)k;
    if (isalpha(k) && isupper(k))
        return (uint16_t)k;
    if (isalpha(k) && islower(k))
        return (uint16_t)k - 0x20;
    for (i = 0; i < numKTable; i++)
    {
        if (ktable[i].ascii == k)
        {
            key = ktable[i].key;
            break;
        }
    }
#elif defined(__QNX__)
    key = KEYCODE_QUESTION;
    for (i = 0; i < numKTable; i++)
    {
        if (k == ktable[i].ascii)
        {
            key = ktable[i].key;
            break;
        }
    }
#endif
    return key;
}

int uinput_unittest(int argc, char *argv[])
{
    unsigned int i = 0;
    if (argc > 2 && strcmp(argv[1],"mouse") == 0)
    {
        float r = 20;
        float phi = 0;
        input_t in;
        uinput_t *p = uinput_open(argv[2]);

        in.type = INPUT_TYPE_MOUSE;
        for (phi = 0; phi < (2*3.14); phi += 0.1f)
        {
            in.data.mouse.dx = (int32_t)(r * cos(phi));
            in.data.mouse.dy = (int32_t)(r * sin(phi));
            SOSAL_PRINT(SOSAL_ZONE_UINPUT, "{x,y} = {%lf,%lf} => {%d, %d}\n",r*cos(phi),r*sin(phi),in.data.mouse.dx,in.data.mouse.dy);
            uinput_send(p, &in);
            thread_msleep(100);
        }
        uinput_close(p);
    }
    else if (argc > 2 && strcmp(argv[1],"keyboard") == 0)
    {
        input_t in;
        uinput_t *p = uinput_open(argv[2]);
        char cstr[] = "This is a string from the uinput library\n";
        in.type = INPUT_TYPE_KEY;

        if (p == NULL)
            return -1;

        SOSAL_PRINT(SOSAL_ZONE_UINPUT, "Sending string \"%s\"",cstr);

        for (i = 0; i < strlen(cstr); i++)
        {
            in.data.key.code = GetKeyFromASCII(cstr[i]);
            in.data.key.state = INPUT_KEY_STATE_DOWN;
            uinput_send(p, &in);
            thread_msleep(10);
            in.data.key.state = INPUT_KEY_STATE_UP;
            uinput_send(p, &in);
            thread_msleep(100);
        }
        uinput_close(p);
    }
    else if (argc > 2 && strcmp(argv[1],"input") == 0 )
    {
        char c;
        input_t in;
        uinput_t *p = uinput_open(argv[2]);
        printf("Enter a string (use ENTER to complete the string!):");
        in.type = INPUT_TYPE_KEY;
        do {
            c = getchar();
            in.data.key.code = GetKeyFromASCII(c);
            in.data.key.state = INPUT_KEY_STATE_DOWN;
            uinput_send(p, &in);
            in.data.key.state = INPUT_KEY_STATE_UP;
            uinput_send(p, &in);
        } while (c != '\n');
        uinput_close(p);
    }
    else
    {
        printf("Find the Class Name of this window and pass that as the first parameter to this program.\n");
        printf("If you don't know the class name, just type any string and all the classnames will be printed in an attempt to find that one.\n");
        printf("Example: $ %s keyboard ConsoleWindowClass\n",argv[0]);
    }
    return 0;
}
#else
int uinput_unittest(int argc, char *argv[])
{
	return 0;
}
#endif

#ifdef UINPUT_TEST
#ifdef _UINPUT_T_FUNCS
int main(int argc, char *argv[])
{
    //return uinput_unittest(argc, argv);
    uint32_t lifetime = 0xFFFFFFFF;
    uint32_t period = 10000; // 10 seconds
    uint32_t delay = 10; // 10 ms.
    char type[255];
    int i;
    uinput_t *in = NULL;

    strcpy(type, "HOME");

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

    SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "%s <lifetime=iter> <period=ms> <delay=ms> <type=KEY|TOUCH>\n", argv[0]);

    for (i = 0; i < argc; i++)
    {
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "ARGV[%u] = %s\n", i, argv[i]);
    }

    if (argc > 1) lifetime = atoi(argv[1]);
    if (argc > 2) period = atoi(argv[2]);
    if (argc > 3) delay = atoi(argv[3]);
    if (argc > 4) strcpy(type, argv[4]);

    in = uinput_open("KeepAwake");
    if (in)
    {
        while (lifetime-- > 0)
        {
            if (strcmp(type, "HOME")== 0)
            {
                input_t kick;
                SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Sending HOME key!\n");
                kick.type = INPUT_TYPE_KEY;
#if defined(LINUX)
                kick.data.key.code = KEY_HOME;
#elif defined(__QNX__)
                kick.data.key.code = KEYCODE_HOME;
#endif
                kick.data.key.state = INPUT_KEY_STATE_DOWN;
                uinput_send(in, &kick);
                thread_msleep(delay);
                kick.data.key.state = INPUT_KEY_STATE_UP;
                uinput_send(in, &kick);
            }
            else if (strcmp(type, "KEY") == 0)
            {
                input_t key;
                int i,len = strlen(argv[5]);
                SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Sending KEY Emulation\n");
                for (i = 0; i < len; i++)
                {
                    key.type = INPUT_TYPE_KEY;
                    key.data.key.code = GetKeyFromASCII(argv[5][i]);
                    key.data.key.state = INPUT_KEY_STATE_DOWN;
                    uinput_send(in, &key);
                    thread_msleep(delay);
                    key.data.key.state = INPUT_KEY_STATE_UP;
                    uinput_send(in, &key);
                }
            }
            else if (strcmp(type, "TOUCH") == 0)
            {
                input_t tap;
                int32_t px = (argc > 5 ? atoi(argv[5]) : 50);
                int32_t py = (argc > 6 ? atoi(argv[6]) : 50);
                uint32_t s = (argc > 7 ? atoi(argv[7]) : 16);
                uint32_t p = (argc > 8 ? atoi(argv[8]) : 10);
                int32_t ex = (argc > 9 ? atoi(argv[9]) : 50);
                int32_t ey = (argc > 10 ? atoi(argv[10]) : 50);
                int32_t inc = (argc > 11 ? atoi(argv[11]) : 10);
                int32_t i;
                int32_t dx, dy, dd, ux, uy;

                // we need to make sure that ux,uy aren't < 1
                dx = abs(ex-px);
                dy = abs(ey-py);
                dd = (dx>dy?dx:dy);
                dx = (ex-px);
                dy = (ey-py);
                if (inc > dd)
                    inc = dd;
                ux = dx/inc;
                uy = dy/inc;
                if (ux == 0 && dx > 0)
                    ux = 1;
                else if (ux == 0 && dx < 0)
                    ux = -1;
                if (uy == 0 && dy > 0)
                    uy = 1;
                else if (uy == 0 && dy < 0)
                    uy = -1;
                for (i = 0; i < inc; i++)
                {
                    SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Sending touchscreen tap on %u%%x%u%%\n",px,py);
                    tap.type = INPUT_TYPE_TOUCH;
                    tap.data.touch.type = INPUT_TS_FINGER;
                    tap.data.touch.location_x = (UINPUT_MAX_TOUCH_X * px) / 100;
                    tap.data.touch.location_y = (UINPUT_MAX_TOUCH_Y * py) / 100;
                    tap.data.touch.major_width = s;
                    tap.data.touch.minor_width = 1;
                    tap.data.touch.pressure = p;
                    tap.data.touch.orientation = 0;
                    uinput_send(in, &tap);
                    px += ux;
                    py += uy;
                    thread_msleep(delay);
                }
                tap.data.touch.pressure = 0;
                uinput_send(in, &tap);
            }
            else
                break;

            thread_msleep(period);

        }
        uinput_close(in);
    }
    return 0;
}
#else
int main(int argc, char *argv[])
{
	printf("This platform does not support uinput!");
}
#endif
#endif
