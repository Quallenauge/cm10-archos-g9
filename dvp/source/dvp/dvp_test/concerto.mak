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
TARGET=dvp_test
TARGETTYPE=exe
DEFS+=$(DVP_FEATURES)
IDIRS+=$(DVP_INC)
CPPSOURCES=dvp_test.cpp TestVisionEngine.cpp
STATIC_LIBS+=VisionEngine sosal imgdbg vcam cthreaded
SHARED_LIBS+=dvp
IDIRS+=$(IPC_INCS) $(MEM_INCS) $(OMX_INCS)
SYS_SHARED_LIBS+=$(IPC_LIBS) $(MEM_LIBS) $(OMX_LIBS) $(PLATFORM_LIBS)
ifeq ($(TARGET_OS),LINUX)
	STATIC_LIBS+=v4l2
endif
include $(FINALE)

ifneq (,$(findstring dsplib,$(VISION_LIBRARIES)))
_MODULE=dvp_dsplib
include $(PRELUDE)
TARGET=dvp_dsplib
TARGETTYPE=exe
IDIRS+=$(DVP_INC) $(DVP_INT_INC) $(SOSAL_INC)
DEFS+=$(DVP_FEATURES)
CSOURCES=dvp_dsplib.c
SHARED_LIBS+=dvp
STATIC_LIBS+=imgdbg sosal
IDIRS += $(IPC_INCS) $(MEM_INCS)
SYS_SHARED_LIBS += $(IPC_LIBS) $(MEM_LIBS) $(PLATFORM_LIBS)
include $(FINALE)
endif

_MODULE=dvp_simple
include $(PRELUDE)
TARGET=dvp_simple
TARGETTYPE=exe
IDIRS+=$(DVP_INC) $(DVP_INT_INC) $(SOSAL_INC)
DEFS+=$(DVP_FEATURES)
CSOURCES=dvp_simple.c
SHARED_LIBS+=dvp
STATIC_LIBS+=imgdbg sosal
IDIRS += $(IPC_INCS) $(MEM_INCS)
SYS_SHARED_LIBS += $(IPC_LIBS) $(MEM_LIBS) $(PLATFORM_LIBS)
include $(FINALE)

_MODULE=dvp_info
include $(PRELUDE)
TARGET=dvp_info
TARGETTYPE=exe
IDIRS+=$(DVP_INC) $(DVP_INT_INC) $(SOSAL_INC)
DEFS+=$(DVP_FEATURES)
CSOURCES=dvp_info.c
SHARED_LIBS+=dvp
STATIC_LIBS+=imgdbg sosal
IDIRS += $(IPC_INCS) $(MEM_INCS)
SYS_SHARED_LIBS += $(IPC_LIBS) $(MEM_LIBS) $(PLATFORM_LIBS)
include $(FINALE)

_MODULE=dvp_unittest
include $(PRELUDE)
TARGET=dvp_unittest
TARGETTYPE=exe
IDIRS+=$(DVP_INC) $(DVP_INT_INC) $(SOSAL_INC)
DEFS+=$(DVP_FEATURES)
CSOURCES=dvp_unittest.c
SHARED_LIBS+=dvp
STATIC_LIBS+=imgdbg sosal
IDIRS += $(IPC_INCS) $(MEM_INCS)
SYS_SHARED_LIBS += $(IPC_LIBS) $(MEM_LIBS) $(PLATFORM_LIBS)
include $(FINALE)
