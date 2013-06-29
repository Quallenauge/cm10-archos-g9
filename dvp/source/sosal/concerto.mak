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
TARGET=sosal
TARGETTYPE=library
DEFS += $(SOSAL_FLAGS) POOL_MAX_SIZE=0
IDIRS += $(SOSAL_INC) $(MEM_INCS)
ifeq ($(TARGET_OS),DARWIN)
IDIRS += /Library/Frameworks/CarbonCore/Headers
endif
CSOURCES=$(filter-out unittest.c,$(all-c-files))
include $(FINALE)

_MODULE=sosal_test
include $(PRELUDE)
TARGET=sosal_test
TARGETTYPE=exe
CSOURCES=unittest.c
DEFS+=SOSAL_UNITTEST
SYS_SHARED_LIBS+=$(IPC_LIBS) $(MEM_LIBS) $(PLATFORM_LIBS)
STATIC_LIBS+=sosal
include $(FINALE)

_MODULE=uinput_test
include $(PRELUDE)
TARGET=uinput_test
TARGETTYPE=exe
CSOURCES=event.c mutex.c options.c queue.c ring.c thread.c uinput.c debug.c
DEFS+=UINPUT_TEST
SYS_SHARED_LIBS+=$(PLATFORM_LIBS)
include $(FINALE)

_MODULE=CThreaded
include $(PRELUDE)
TARGET=cthreaded
TARGETTYPE=library
CPPSOURCES=CThreaded.cpp
include $(FINALE)

_MODULE=modload_test
include $(PRELUDE)
TARGET=modload_test
TARGETTYPE=exe
CSOURCES=module.c debug.c
DEFS+=MODULE_TEST
SYS_SHARED_LIBS+=$(PLATFORM_LIBS)
include $(FINALE)
