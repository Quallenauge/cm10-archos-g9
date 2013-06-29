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
TARGET=dvp_demo
TARGETTYPE=exe
DEFS+=$(DVP_FEATURES)
IDIRS+=$(DVP_INC)
CPPSOURCES=dvp_demo.cpp
STATIC_LIBS+=sosal imgdbg vcam cthreaded
SHARED_LIBS+=dvp
SYS_SHARED_LIBS += $(IPC_LIBS) $(MEM_LIBS) $(OMX_LIBS) $(PLATFORM_LIBS)
ifeq ($(TARGET_OS),LINUX)
	STATIC_LIBS+=v4l2
endif
include $(FINALE)

