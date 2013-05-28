/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/vfs.h>
#include <libgen.h>
#include "mounts.h"
#include <linux/loop.h>
#include "edify/expr.h"
#include "make_ext4fs.h"
#ifdef HAVE_SELINUX
#include <selinux/selinux.h>
#include <selinux/label.h>
#else
struct selabel_handle;
#endif
extern struct selabel_handle *sehandle;
// make_ext4fs(fs_size, filename)
//
//    fs_size=<bytes> filename=<ext4 image file>
//    if fs_size == 0, then make_ext4fs uses the remaining free space.
Value* MakeExt4fsFn(const char* name, State* state, int argc, Expr* argv[]) {
    char* result = NULL;
    if (argc != 2) {
        return ErrorAbort(state, "%s() expects 2 args, got %d", name, argc);
    }
    char* filename;
    char* fs_size;
    
    if (ReadArgs(state, argv, 2, &fs_size, &filename) < 0) {
        return NULL;
    }

    if (strlen(filename) == 0) {
        ErrorAbort(state, "location argument to %s() can't be empty", name);
        goto done;
    }

    if (strlen(fs_size) == 0) {
        ErrorAbort(state, "fs_size argument to %s() can't be empty", name);
        goto done;
    }
    
    // Work out the remaining free space
    long long fssize =  atoll(fs_size);
    if(fssize==0){
        
        unsigned char padding[(1024*8092)] = { 0, };
        struct statfs st;
        FILE * fp = fopen(filename,"w+b");
        fwrite(padding,1,(1024*8092),fp ) ;
        fclose(fp) ;
        if (statfs(filename, &st) < 0) {
             goto done;
        }
        //fssize = (long long)st.f_bfree * (long long)st.f_bsize;
        //fssize -= 200000000;
        fssize = 4094967295; 
        fp = fopen(filename,"w+b");
        fssize = fssize / (1024*8092);
        
        int i = 0;
        for(i = 0 ; i <= fssize ; i ++){
        write(fileno(fp),padding,(1024*8092) ) ;
        }
        fsync(fileno(fp));
        fclose(fp) ;
        int filefd = open(filename,O_RDWR);
        if(filefd < 0 ){
            fprintf(stderr, "%s: make_ext4fs failed to open file %s",name,filename);
            result = strdup("");
            goto done;
        }
        char * loopdevice = "/dev/block/loop5";
        int devicefd = open(loopdevice,O_RDWR);
        if(devicefd < 0 ){
            fprintf(stderr, "%s: make_ext4fs failed to open loopback file %s",name,loopdevice);
            result = strdup("");
            close(filefd);
            goto done;
        }
        
        if (ioctl(devicefd, LOOP_SET_FD, filefd) < 0){
            fprintf(stderr, "%s: make_ext4fs failed to create loop back device - %s",name,loopdevice);
            result = strdup("");
            close(devicefd);
            close(filefd);
            goto done;
        }
        int status = make_ext4fs(loopdevice, 0, NULL, sehandle);
        if (status != 0) {
            fprintf(stderr, "%s: make_ext4fs failed (%d) on %s",name, status, loopdevice);
            result = strdup("");
            goto done;
        }
        result = filename;
        goto done;
    }
       
    int status = make_ext4fs(filename, fssize, NULL, sehandle);
    if (status != 0) {
        fprintf(stderr, "%s: make_ext4fs failed (%d) on %s", name, status, filename);
            result = strdup("");
            goto done;
    }
    result = filename;

done:
    if (result != filename) free(filename);
    return StringValue(result);
}

void Register_librecovery_updater_toro() {
    fprintf(stderr, "installing archos updater extensions for g9s\n");

    RegisterFunction("archos.make_ext4fs", MakeExt4fsFn);
}
