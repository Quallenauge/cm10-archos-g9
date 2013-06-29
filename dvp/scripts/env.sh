#!/bin/bash

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

if [ -n "$QNX_TARGET" ]; then
    if [ -z "$IPC_ROOT" ]; then
        echo Must set IPC_ROOT
    fi
    if [ -z "$TILER_ROOT" ]; then
        echo Must set TILER_ROOT
    fi
    env | grep QNX
    echo Use TARGET_PLATFORM=PLAYBOOK on the make line to build for QNX.
fi
if [ -n "$MYDROID" ]; then
    # Nothing... the Android Build has defaults
    echo Android Build has default values already.
fi
if [ -z "$QNX_TARGET" ] && [ -z "$MYDROID" ]; then
    # This is a PC build of some sort.
    export TARGET_CPU=`uname -m`
    export TARGET_PLATFORM=PC
    export DVP_DEBUG=1
    export DVP_ZONE_MASK=0x0003
    export SOSAL_DEBUG=1
    export SOSAL_ZONE_MASK=0x3
fi


