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
TARGET=dvp_kgm_cpu
DEFS+=$(DVP_FEATURES) DVP_USE_IMAGE
TARGETTYPE=dsmo
CSOURCES+=dvp_kgm_cpu.c dvp_ll.c
DEFFILE=dvp_kgm.def
SHARED_LIBS=dvp
STATIC_LIBS=sosal
IDIRS+=$(DVP_INC) $(DVP_INT_INC)
LDIRS+=$(VISION_LIB)

ifeq ($(TARGET_CPU),ARM)
    IDIRS+=$(MEM_INCS)
endif

ifneq (,$(findstring vlib,$(VISION_LIBRARIES)))
SYS_STATIC_LIBS += vlib_$(TARGET_CPU)
endif

ifneq (,$(findstring rvm,$(VISION_LIBRARIES)))
SYS_STATIC_LIBS += rvm_$(TARGET_CPU)
endif

ifneq (,$(findstring rvm_sunex,$(VISION_LIBRARIES)))
SYS_STATIC_LIBS += rvm_sunex_$(TARGET_CPU)
endif

ifneq (,$(findstring imglib,$(VISION_LIBRARIES)))
SYS_STATIC_LIBS += imglib_$(TARGET_CPU)
endif

ifneq (,$(findstring orb,$(VISION_LIBRARIES)))
SYS_STATIC_LIBS += orb_$(TARGET_CPU)
endif

ifneq (,$(findstring tismo,$(VISION_LIBRARIES)))
#SYS_STATIC_LIBS += tismo
endif

ifneq (,$(findstring yuv,$(VISION_LIBRARIES)))
SYS_STATIC_LIBS += yuv
endif

ifneq (,$(findstring imgfilter,$(VISION_LIBRARIES)))
SYS_STATIC_LIBS += imgfilter
endif

include $(FINALE)

