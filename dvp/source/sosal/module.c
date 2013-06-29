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

#include <sosal/module.h>
#include <sosal/debug.h>

#if defined(SYSBIOS_SL)

extern static_module_table_t dvp_kgm_cpu_table;
extern static_module_table_t dvp_kgm_dsp_table;
extern static_module_table_t dvp_kgm_simcop_table;

static static_module_table_t *static_modules[] = {
    &dvp_kgm_cpu_table,
    &dvp_kgm_dsp_table,
    &dvp_kgm_simcop_table
};

#endif

module_t module_load(const char *filename)
{
    module_t mod = 0;
#if defined(POSIX_DL)
    mod = dlopen(filename, RTLD_NOW|RTLD_GLOBAL);
#elif defined(WIN32_DL)
    mod = LoadLibrary(filename);
#elif defined(SYSBIOS_SL)
    uint32_t m = 0;
    for (m = 0; m < dimof(static_modules); m++)
    {
        if (strncmp(static_modules[m]->name, filename, MAX_PATH) == 0)
        {
            return static_modules[m];
        }
    }
#endif
    SOSAL_PRINT(SOSAL_ZONE_MODULE, "Loaded Module %s => %p\n", filename, mod);
    return mod;
}

symbol_t module_symbol(module_t mod, const char *symbol_name)
{
    symbol_t sym = NULL;
#if defined(POSIX_DL)
    char *str = NULL;
    sym = (symbol_t)dlsym(mod, symbol_name);
    str = (char *)dlerror();
    if (str) {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "%s %s\n", str, symbol_name);
        sym = NULL;
    }
#elif defined(WIN32_DL)
    sym = (symbol_t)GetProcAddress(mod, symbol_name);
#elif defined(SYSBIOS_SL)
    uint32_t s;
    static_module_table_t *modtable = mod;
    for (s = 0; s < modtable->numFunctions; s++)
    {
        if (strncmp(modtable->functions[s].name, symbol_name, MAX_PATH) == 0)
        {
            sym = modtable->functions[s].func_ptr;
            break;
        }
    }
#endif
    SOSAL_PRINT(SOSAL_ZONE_MODULE, "SYMBOL:%s => %p\n", symbol_name, sym);
    return sym;
}

char module_error_string[256];

char *module_error()
{
#if defined(POSIX_DL)
    return (char *)dlerror();
#elif defined(WIN32_DL)
    sprintf(module_error_string, "Error code %d\n", GetLastError());
    return module_error_string;
#elif defined(SYSBIOS_SL)
    sprintf(module_error_string, "Error string unsupported on SYSBIOS\n");
    return module_error_string;
#endif
}

void module_unload(module_t mod)
{
    SOSAL_PRINT(SOSAL_ZONE_MODULE, "Unloading Module %p\n", mod);
#if defined(POSIX_DL)
    dlclose(mod);
#elif defined(WIN32_DL)
    FreeLibrary(mod);
#elif defined(SYSBIOS_SL)
    // do nothing.
#endif
}

#ifdef MODULE_TEST
int main(int argc, char *argv[])
#else
bool_e module_unittest(int argc, char *argv[])
#endif
{
    bool_e ret = true_e;
    module_t mods[20];
    symbol_t syms[20];
    int32_t idxArg = 0;
    int32_t idxMod = -1;
    int32_t idxSym = -1;
    bool_e noClose = false_e;

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

    for (idxArg = 1, idxMod = 0; idxArg < argc && idxMod < (int32_t)dimof(mods); idxArg++)
    {
        if (strcmp(argv[idxArg], "-m") == 0 || strcmp(argv[idxArg], "-M") == 0)
        {
            char filename[MAX_PATH];
            if (argv[idxArg][1] == 'M')
                sprintf(filename, MODULE_NAME("%s"), argv[idxArg+1]);
            else
                strcpy(filename, argv[idxArg+1]);
            mods[++idxMod] = module_load(filename);
            if (mods[idxMod] == NULL)
            {
                printf("Error loading module, %s\n", module_error());
                ret = false_e;
                continue;
            }
            printf("Loaded %s (%p[%d])\n", argv[idxArg+1], mods[idxMod], idxMod);
        }
        if (strcmp(argv[idxArg], "-s") == 0)
        {
            printf("Trying to load symbol %s\n", argv[idxArg+1]);
            syms[++idxSym] = module_symbol(mods[idxMod], argv[idxArg+1]);
            if (syms[idxSym] == NULL)
            {
                printf("Error loading Symbol %s\n", argv[idxArg+1]);
                ret = false_e;
                continue;
            }
            printf("Loaded %s (%p[%d])\n", argv[idxArg+1], syms[idxSym], idxSym);
        }
        if (strcmp(argv[idxArg], "-nc") == 0)
            noClose = true_e;
    }
    for ( ; idxMod >= 0; idxMod--)
    {
        if (noClose == false_e)
        {
            printf("Attempting unload of %p[%d]\n", mods[idxMod],idxMod);
            module_unload(mods[idxMod]);
            printf("Unloaded %p[%d]\n", mods[idxMod],idxMod);
        }
    }
    printf("Completed all unloads\n");
    return ret;
}
