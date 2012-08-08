
# Makefile variables and C/C++ macros to recognize API level
ANDROID_API_JB_OR_LATER :=
ANDROID_API_ICS_OR_LATER :=
ANDROID_API_CFLAGS :=

ifeq ($(shell test $(PLATFORM_SDK_VERSION) -ge 16 || echo 1),)
    ANDROID_API_JB_OR_LATER := true
    ANDROID_API_CFLAGS += -DANDROID_API_JB_OR_LATER
endif
ifeq ($(shell test $(PLATFORM_SDK_VERSION) -ge 14 || echo 1),)
    ANDROID_API_ICS_OR_LATER := true
    ANDROID_API_CFLAGS += -DANDROID_API_ICS_OR_LATER
endif

define clear-android-api-vars
$(eval ANDROID_API_JB_OR_LATER:=) \
$(eval ANDROID_API_ICS_OR_LATER:=) \
$(eval ANDROID_API_CFLAGS:=) \
$(eval clear-android-api-vars:=)
endef
