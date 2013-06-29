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

ifneq ($(HOST_COMPILER),TMS470)

include $(PRELUDE)

VISION_LIBRARIES+=yuv
DVP_INC+=$(_MODPATH)/include
DVP_FEATURES+=DVP_USE_YUV

ifdef DVP_LOCAL_BUILD
TARGET=yuv
TARGETTYPE=library
ASSEMBLY:=$(all-S-files)
CSOURCES:=
endif # LOCAL

include $(FINALE)

endif # COMPILER

else # ARM

include $(PRELUDE)

# Don't expose this yet.
#VISION_LIBRARIES+=yuv
#DVP_INC+=$(_MODPATH)/include
#DVP_FEATURES+=DVP_USE_YUV

ifdef DVP_LOCAL_BUILD
TARGET=yuv
TARGETTYPE=library
CSOURCES:=$(all-c-files)
endif # LOCAL

include $(FINALE)

endif # ARM
