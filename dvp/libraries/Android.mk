include $(call all-subdir-makefiles)

VISION_ROOT ?= $(TI_HW_ROOT)/vision
-include $(VISION_ROOT)/libraries/Android.mk

ifneq (,$(findstring vlib,$(VISION_LIBRARIES)))
DVP_FEATURES += -DDVP_USE_VLIB
DVP_INCLUDES += $(VISION_ROOT)/libraries/protected/vlib/include
endif

ifneq (,$(findstring rvm,$(VISION_LIBRARIES)))
DVP_FEATURES += -DDVP_USE_RVM
DVP_INCLUDES += $(VISION_ROOT)/libraries/protected/rvm/include
ifneq (,$(findstring rvm_sunex,$(VISION_LIBRARIES)))
DVP_INCLUDES += $(VISION_ROOT)/libraries/protected/rvm_sunex/include
endif
endif

ifneq (,$(findstring tismo,$(VISION_LIBRARIES)))
DVP_FEATURES += -DDVP_USE_TISMO
DVP_INCLUDES += $(VISION_ROOT)/libraries/protected/tismo/include
endif

ifneq (,$(findstring tismov02,$(VISION_LIBRARIES)))
DVP_FEATURES += -DDVP_USE_TISMOV02
DVP_INCLUDES += $(VISION_ROOT)/libraries/protected/tismov02/include
endif

ifneq (,$(findstring orb,$(VISION_LIBRARIES)))
DVP_FEATURES += -DDVP_USE_ORB
DVP_INCLUDES += $(VISION_ROOT)/libraries/protected/orb/include
endif

ifneq (,$(findstring imglib,$(VISION_LIBRARIES)))
DVP_FEATURES += -DDVP_USE_IMGLIB
DVP_INCLUDES += $(VISION_ROOT)/libraries/protected/imglib/include
endif

ifneq (,$(findstring dsplib,$(VISION_LIBRARIES)))
DVP_FEATURES += -DDVP_USE_DSPLIB
DVP_INCLUDES += $(DVP_ROOT)/libraries/public/dsplib/include
endif

ifneq (,$(findstring ar,$(VISION_LIBRARIES)))
DVP_FEATURES += -DDVP_USE_AR
DVP_INCLUDES += $(VISION_ROOT)/libraries/protected/ar/include
endif

ifneq (,$(findstring xtr_ll,$(VISION_LIBRARIES)))
DVP_FEATURES += -DDVP_USE_XTR
DVP_INCLUDES += $(VISION_ROOT)/libraries/protected/xtr_ll/include
endif

ifneq (,$(findstring vrun,$(VISION_LIBRARIES)))
DVP_FEATURES += -DDVP_USE_VRUN
DVP_INCLUDES += $(DVP_ROOT)/libraries/public/vrun/include
endif

ifneq (,$(findstring dei,$(VISION_LIBRARIES)))
DVP_FEATURES += -DDVP_USE_DEI
DVP_INCLUDES += $(DVP_ROOT)/libraries/public/dei/include
endif

ifneq (,$(findstring dsplib,$(VISION_LIBRARIES)))
DVP_FEATURES += -DDVP_USE_DSPLIB
DVP_INCLUDES += $(VISION_ROOT)/libraries/protected/dsplib/include
endif

ifneq (,$(findstring orb,$(VISION_LIBRARIES)))
DVP_FEATURES += -DDVP_USE_ORB
DVP_INCLUDES += $(VISION_ROOT)/libraries/protected/orb/include
endif

ifneq (,$(findstring imgfilter,$(VISION_LIBRARIES)))
DVP_FEATURES += -DDVP_USE_IMGFILTER
DVP_INCLUDES += $(DVP_ROOT)/libraries/public/imgfilter/include
endif

ifneq (,$(findstring yuv,$(VISION_LIBRARIES)))
DVP_FEATURES += -DDVP_USE_YUV
DVP_INCLUDES += $(DVP_ROOT)/libraries/public/yuv/include
endif

ifeq ($(TARGET_SCPU),C64T)
ifneq (,$(findstring c6xsim,$(VISION_LIBRARIES)))
DVP_FEATURES += -DVLIB_TARGET_C6XSIM -DSIMULATION -DTMS320C64PX -DLITTLE_ENDIAN_HOST
DVP_INCLUDES += $(VISION_ROOT)/libraries/protected/c6xsim/include
endif
endif

# All the DVP_LIBRARIES should be set by now
$(info VISION_LIBRARIES = $(VISION_LIBRARIES))
$(info DVP_FEATURES = $(DVP_FEATURES))

