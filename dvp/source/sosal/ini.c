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

#include <sosal/ini.h>

typedef enum _ini_state_e {
    INI_NONE,
    INI_SEC,
    INI_VAR,
    INI_VAL,
    INI_COM
} ini_state_e;

int ini_parse(char *buffer, size_t len, ini_callback_f callback, ini_chars_e char_case, void *arg)
{
#define clear(x)    memset(x, 0, sizeof(x))
    ini_state_e last = INI_NONE, state = INI_NONE;
    size_t i;
    char c;
    char section[255];  uint32_t s1 = 0;
    char variable[255]; uint32_t s2 = 0;
    char value[255];    uint32_t s3 = 0;

    SOSAL_PRINT(SOSAL_ZONE_INI, "Parsing buffer of "FMT_SIZE_T" characters for INI formatted settings\n", len);

    clear(variable);
    clear(value);
    strcpy(section, "GLOBAL");
    state = INI_SEC;

    for (i = 0; i < len; i++) {
        last = state;
        c = buffer[i];
        if (char_case == INI_UPPER)
        {
            if ('a' <= c && c <= 'z')
                c = (c - 'a') + 'A';
        }
        else if (char_case == INI_LOWER)
        {
            if ('A' <= c && c <= 'Z')
                c = (c - 'A') + 'a';
        }
        switch(c) {
            case '#': case ';': state = INI_COM; break;
            case '[': state = INI_SEC; s1 = 0; clear(section); clear(variable); clear(value); break;
            case ']': state = INI_VAR; s2 = 0; clear(variable); clear(value); break;
            case '=': state = INI_VAL; s3 = 0; clear(value); break;
            case '\t': case ' ': case '\"': break; // do nothings
            case '\n': case '\r': case '\0':
                if (state == INI_VAL) {
                    if (section[0] && variable[0] && value[0]) {
                        callback(arg, section, variable, value);
                        clear(variable); s2 = 0;
                        clear(value); s3 = 0;
                    }
                    state = INI_VAR;
                }
                break;
            default:
                switch(state) {
                    case INI_NONE: case INI_COM: break;
                    case INI_SEC: if (s1 < sizeof(section)) section[s1++] = c; break;
                    case INI_VAR: if (s2 < sizeof(variable)) variable[s2++] = c; break;
                    case INI_VAL: if (s3 < sizeof(value)) value[s3++] = c; break;
                    default: break;
                }
                break;
        }
        //SOSAL_PRINT(SOSAL_ZONE_INI, "%c => %u (%u) section=%s var=%s val=%s\n", c, state, last, section, variable, value);
    }
    // if the file didn't end with a newline or linefeed
    if (section[0] && variable[0] && value[0]) {
        callback(arg, section, variable, value);
        clear(variable); s2 = 0;
        clear(value); s3 = 0;
    }
    return 0;
}

static size_t flen(FILE *f)
{
    size_t cur = ftell(f);
    size_t len = 0;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, cur, SEEK_SET);
    return len;
}

static void print_ini(void *arg, char *section, char *variable, char *value)
{
    printf("%p [%s] %s = %s\n", arg, section, variable, value);
}

bool_e ini_unittest(int argc, char *argv[])
{
    if (argc > 1)
    {
        FILE *f = fopen(argv[1], "r");
        if (f)
        {
            size_t len = flen(f);
            char * buffer = malloc(len);
            if (buffer)
            {
                if (fread(buffer, len, sizeof(char), f) > 0)
                {
                    ini_parse(buffer, len, print_ini, INI_NATURAL, NULL);
                }
                free(buffer);
            }
            fclose(f);
        }
    } else {
        char *test = "[SECTION1]\n\rVARIABLE =\t\"VALUE\"\n\r#comment\n\r[section2]\n\rBOB=uncle\n\r";
        ini_parse(test, strlen(test), print_ini, INI_NATURAL, NULL);
    }
    return true_e;

}

