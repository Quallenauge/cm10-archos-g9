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

#include <sosal/options.h>
#include <sosal/debug.h>

size_t option_process(int argc, char *argv[], option_t *opts, size_t numOpts)
{
    int i;
    size_t j,c = 0;
    for (i = 0; i < argc; i++)
    {
        for (j = 0; j < numOpts; j++)
        {
            if ((opts[j].short_switch && strcmp(opts[j].short_switch, argv[i]) == 0) ||
                (opts[j].long_switch && strcmp(opts[j].long_switch, argv[i]) == 0))
            {
                switch (opts[j].type)
                {
                    case OPTION_TYPE_BOOL:
                        *(bool_e *)opts[j].datum = true_e;
                        c++;
                        break;
                    case OPTION_TYPE_INT:
                        if (i+1 < argc && opts[j].size == sizeof(int)) {
                            i += 1;
                            sscanf(argv[i],"%d",(int *)opts[j].datum);
                            c++;
                        }
                        break;
                    case OPTION_TYPE_HEX:
                        if (i+1 < argc && opts[j].size == sizeof(uint32_t)) {
                            i += 1;
                            sscanf(argv[i],"%x",(uint32_t *)opts[j].datum);
                            c++;
                        }
                        break;
                    case OPTION_TYPE_FLOAT:
                        if (i+1 < argc && opts[j].size == sizeof(float)) {
                            i += 1;
                            sscanf(argv[i],"%f",(float *)opts[j].datum);
                            c++;
                        }
                        break;
                    case OPTION_TYPE_STRING:
                        if (i+1 < argc) {
                            i += 1;
                            strncpy(opts[j].datum, argv[i], opts[j].size);
                            c++;
                        }
                        break;
                }
                break; // process next argv
            }
        }
    }
    return c;
}

static bool_e opt_toggle;
static int    opt_int;
static uint32_t opt_hex;
static float opt_float;
static char opt_string[256];
static option_t opts[] = {
    {OPTION_TYPE_BOOL, &opt_toggle, sizeof(opt_toggle), "-t", "--toggle", "Toggles a Boolean"},
    {OPTION_TYPE_INT, &opt_int, sizeof(opt_int), "-i", "--integer", "An integer value"},
    {OPTION_TYPE_HEX, &opt_hex, sizeof(opt_hex), "-h", "--hex", "A 32 bit hexidecimal value"},
    {OPTION_TYPE_FLOAT, &opt_float, sizeof(opt_float), "-f", "--float", "A 32 bit floating point value"},
    {OPTION_TYPE_STRING, opt_string, 256, "-s", "--string", "A \"quoted\" character string"},
};
static size_t numOpts = dimof(opts);

bool_e option_unittest(int argc, char *argv[])
{
    opt_toggle = false_e;
    opt_int = -42;
    opt_hex = 0xdeadbeef;
    opt_float = 3.1415f;
    strcpy(opt_string, "This is a test string");
    option_process(argc, argv, opts, numOpts);
    printf("opt_toggle = %d\n", opt_toggle);
    printf("opt_int = %d\n", opt_int);
    printf("opt_hex = %x\n", opt_hex);
    printf("opt_float = %lf\n", opt_float);
    printf("opt_string = %s\n", opt_string);
    return true_e;
}

