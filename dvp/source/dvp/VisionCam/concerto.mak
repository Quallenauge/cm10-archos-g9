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
TARGET=vcam
TARGETTYPE=library
CPPSOURCES=VisionCamFactory.cpp CameraEmulator.cpp FileVisionCam.cpp VisionCamFrame.cpp
ifdef UVC_INC
    IDIRS+=$(UVC_INC)
    CPPSOURCES+=UVCVisionCam.cpp
    DEFS+=VCAM_USE_USB
endif
ifdef OMX_ROOT
    IDIRS+=$(OMX_INCS)
    SYS_SHARED_LIBS+=$(OMX_LIBS)
    DEFS+=VCAM_USE_OMX
    CPPSOURCES+=OMXVisionCam.cpp
endif
ifneq ($(TARGET_OS),SYSBIOS)
	CPPSOURCES+=SocketVisionCam.cpp
endif
DEFS+=$(DVP_DEBUGGING) $(DVP_CPPFLAGS)
include $(FINALE)

_MODULE=vcam_simple
include $(PRELUDE)
TARGET=vcam_simple
TARGETTYPE=exe
CPPSOURCES=VisionCamSimpleTest.cpp
STATIC_LIBS=vcam sosal cthreaded imgdbg
SHARED_LIBS=dvp
SYS_SHARED_LIBS += $(IPC_LIBS) $(MEM_LIBS) $(OMX_LIBS) $(PLATFORM_LIBS)
ifeq ($(TARGET_OS),LINUX)
    STATIC_LIBS+=v4l2
endif
include $(FINALE)

ifneq ($(TARGET_OS),SYSBIOS)
_MODULE=vcam_server
include $(PRELUDE)
TARGET=vcam_server
TARGETTYPE=exe
CPPSOURCES=ServerVisionCam.cpp VisionCamServer.cpp
STATIC_LIBS=vcam sosal cthreaded imgdbg
SHARED_LIBS=dvp
SYS_SHARED_LIBS += $(IPC_LIBS) $(MEM_LIBS) $(OMX_LIBS) $(PLATFORM_LIBS)
ifeq ($(TARGET_OS),LINUX)
    STATIC_LIBS+=v4l2
endif
include $(FINALE)
endif

