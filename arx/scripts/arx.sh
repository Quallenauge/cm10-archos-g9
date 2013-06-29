#!/bin/bash

# Copyright (C) 2012 Texas Instruments, Inc.
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

# The definitions must be matched to the Debug.h
function zone_mask()
{
    if [ "$1" == "print" ]; then
        echo ARX_DEBUG=$ARX_DEBUG
        echo ARX_ZONE_MASK=$ARX_ZONE_MASK
    fi
    if [ "$1" == "clear" ]; then
        unset ARX_DEBUG
        unset ARX_ZONE_MASK
    fi
    if [ "$1" == "full" ]; then
        export ARX_DEBUG=2
        export ARX_ZONE_MASK=0xFFFF
    fi
    if [ "$1" == "min" ]; then
        export ARX_DEBUG=2
        export ARX_ZONE_MASK=0x0003
    fi
    if [ "$1" == "none" ]; then
        export ARX_DEBUG=2
        export ARX_ZONE_MASK=0x0001
   fi
   if [ "$1" == "perf" ]; then
        export ARX_DEBUG=2
        export ARX_ZONE_MASK=0x0023
   fi
   if [ "$1" == "test" ]; then
        export ARX_DEBUG=2
        export ARX_ZONE_MASK=0x0103
   fi
   
}

if [ -n "${MYDROID}" ]; then
    . arx_android.sh $*
fi

