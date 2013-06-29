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

ifeq ($(TARGET_CPU),ARM)

include $(PRELUDE)
TARGET=dvp_kgm_dsp
TARGETTYPE=dsmo
CSOURCES=dvp_kgm_dsp.c
DEFFILE=dvp_kgm.def
DEFS+=$(DVP_FEATURES)
IDIRS+=$(DVP_INC) $(IPC_INC) $(TILER_INC) $(DVP_INT_INC)
SHARED_LIBS+=dvp
ifeq ($(TARGET_OS),LINUX)
    ifeq ($(TARGET_CPU), ARM)
        SYS_SHARED_LIBS+=$(IPC_LIBS) $(TILER_LIB)
    endif
else ifeq ($(TARGET_OS),__QNX__)
    SYS_SHARED_LIBS+=syslink_client memmgr
endif

include $(FINALE)

endif

