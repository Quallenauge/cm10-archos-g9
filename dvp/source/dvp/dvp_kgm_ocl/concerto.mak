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

ifeq ($(CL_AVAILABLE),true)

include $(PRELUDE)
TARGET=dvp_kgm_ocl
TARGETTYPE=dsmo
CSOURCES=$(call all-c-files)
DEFS += $(DVP_FEATURES)
IDIRS += $(DVP_INC)
STATIC_LIBS += ocl
SHARED_LIBS += dvp
ifeq ($(TARGET_OS),DARWIN)
    LDFLAGS += -framework OpenCL
else ifeq ($(TARGET_PLATFORM),PANDA)
    SYS_SHARED_LIBS += PVROCL
else
    SYS_SHARED_LIBS += OpenCL
endif
include $(FINALE)

endif

