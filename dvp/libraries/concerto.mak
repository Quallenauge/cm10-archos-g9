
# If vision folder root is defined scan it for algorithms.
ifdef VISION_ROOT
$(info Including VISION Folder in Build for DVP)
DIRECTORIES:=$(VISION_ROOT)/libraries
NO_TARGETS:=1
include $(VISION_ROOT)/concerto/rules.mak
NO_TARGETS:=

# Define the location to add to LDIRS if you want VISION algos
VISION_LIB := $(VISION_ROOT)/out/$(TARGET_OS)/$(TARGET_CPU)

# Add each algo present to the DVP_FEATURES and INC paths
ifneq (,$(findstring vlib,$(VISION_LIBRARIES)))
DVP_FEATURES += DVP_USE_VLIB
DVP_INC += $(VISION_ROOT)/libraries/protected/vlib/include
endif

ifneq (,$(findstring rvm,$(VISION_LIBRARIES)))
DVP_FEATURES += DVP_USE_RVM
DVP_INC += $(VISION_ROOT)/libraries/protected/rvm/include
ifneq (,$(findstring rvm_sunex,$(VISION_LIBRARIES)))
DVP_INC += $(VISION_ROOT)/libraries/protected/rvm_sunex/include
endif
endif

ifneq (,$(findstring tismo,$(VISION_LIBRARIES)))
DVP_FEATURES += DVP_USE_TISMO
DVP_INC += $(VISION_ROOT)/libraries/protected/tismo/include
endif

ifneq (,$(findstring tismov02,$(VISION_LIBRARIES)))
DVP_FEATURES += DVP_USE_TISMOV02
DVP_INC += $(VISION_ROOT)/libraries/protected/tismov02/include
endif

ifneq (,$(findstring imglib,$(VISION_LIBRARIES)))
DVP_FEATURES += DVP_USE_IMGLIB
DVP_INC += $(VISION_ROOT)/libraries/protected/imglib/include
endif

ifneq (,$(findstring ar,$(VISION_LIBRARIES)))
DVP_FEATURES += DVP_USE_AR
DVP_INC += $(VISION_ROOT)/libraries/protected/ar/include
endif

ifneq (,$(findstring orb,$(VISION_LIBRARIES)))
DVP_FEATURES += DVP_USE_ORB
DVP_INC += $(VISION_ROOT)/libraries/protected/orb/include
endif

ifeq ($(TARGET_SCPU),C64T)
ifneq (,$(findstring c6xsim,$(VISION_LIBRARIES)))
DVP_FEATURES += VLIB_TARGET_C6XSIM SIMULATION TMS320C64PX LITTLE_ENDIAN_HOST
DVP_INC += $(VISION_ROOT)/libraries/protected/c6xsim/include
endif
endif
endif

