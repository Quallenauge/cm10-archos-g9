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

#include <sosal/types.h>
#include <sosal/fourcc.h>
#include <sosal/debug.h>

fourcc_t strtofourcc(const char *string)
{
    fourcc_t fourcc = 0;
    fourcc |= ((uint8_t)string[0])<<0;
    fourcc |= ((uint8_t)string[1])<<8;
    fourcc |= ((uint8_t)string[2])<<16;
    fourcc |= ((uint8_t)string[3])<<24;
    return fourcc;
}

char *fourcctostr(fourcc_t fcc)
{
    static char code[5];
    strncpy(code, (char *)&fcc, 4);
    code[4] = '\0';
    return code;
}

bool_e fourcc_unittest(int argc, char *argv[])
{
    int i = 0;
    for (i = 0; i < argc; i++)
    {
        printf("0x%08x == %s\n", strtofourcc(argv[i]), argv[i]);
    }
    return true_e;
}

