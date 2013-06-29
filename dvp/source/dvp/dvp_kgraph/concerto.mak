# Copyright (C) 2009-2011 Texas Instruments, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include $(PRELUDE)
TARGET=dvp
TARGETTYPE=dsmo
CSOURCES=dvp_kgraph.c dvp_kgb.c dvp_mem.c dvp_mem_int.c dvp_kmdl.c
DEFFILE=dvp.def
DEFS+=DVP_KGAPI_THREADED $(DVP_FEATURES)
IDIRS += $(IPC_INCS) $(MEM_INCS)
SYS_SHARED_LIBS += $(MEM_LIBS)
ifeq ($(TARGET_OS),LINUX)
    ifneq ($(GTK_PATH),)
        IDIRS+=$(GTK_INC)
        LDIRS+=$(GTK_LDIRS)
        STATIC_LIBS+=gtkwindow
        SYS_SHARED_LIBS+=$(GTK_LIBS)
        CSOURCES+=dvp_display_gtk.c
    else
        CSOURCES+=dvp_display_file.c
    endif
    SHARED_LIBS += $(IPC_LIBS)
    ifeq ($(TARGET_CPU),ARM)
        CSOURCES+=dvp_rpc_omaprpc.c
    else
        CSOURCES+=dvp_rpc_stub.c
    endif
else ifeq ($(TARGET_OS),Windows_NT)
    SYS_SHARED_LIBS += $(IPC_LIBS) $(PLATFORM_LIBS)
    CSOURCES+=dvp_display_file.c dvp_rpc_stub.c
else ifeq ($(TARGET_OS),__QNX__)
    STATIC_LIBS += qnxscreen
    SYS_SHARED_LIBS += $(IPC_LIBS) $(PLATFORM_LIBS)
    CSOURCES+=dvp_display_qnx.c dvp_rpc_rcm.c
else
    # in non-linux systems, are aren't using OMAPRPC
    SYS_SHARED_LIBS += $(IPC_LIBS) $(PLATFORM_LIBS)
    CSOURCES+=dvp_rpc_stub.c
endif
STATIC_LIBS+=sosal
include $(FINALE)
