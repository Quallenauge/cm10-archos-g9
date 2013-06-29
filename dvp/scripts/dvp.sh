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

# The definitions must be matched to the dvp_debug.h and sosal/debug.h
function zone_mask() {
    if [ "$1" == "print" ]; then
        echo DVP_DEBUG=${DVP_DEBUG}
        echo DVP_ZONE_MASK=${DVP_ZONE_MASK}
        echo SOSAL_DEBUG=${SOSAL_DEBUG}
        echo SOSAL_ZONE_MASK=${SOSAL_ZONE_MASK}
        echo OMAPRPC_DEBUG=${OMAPRPC_DEBUG}
        echo OMAPRPC_ZONE_MASK=${OMAPRPC_ZONE_MASK}
    fi
    if [ "$1" == "undef" ]; then    # Refer to Android.mk for defaults in this case
        unset DVP_DEBUG
        unset DVP_ZONE_MASK
        unset SOSAL_DEBUG
        unset SOSAL_ZONE_MASK
        unset OMAPRPC_DEBUG
        unset OMAPRPC_ZONE_MASK
    fi
    if [ "$1" == "runtime" ]; then
        export DVP_DEBUG=2
        unset DVP_ZONE_MASK
        export SOSAL_DEBUG=2
        unset SOSAL_ZONE_MASK
        export OMAPRPC_DEBUG=2
        unset OMAPRPC_ZONE_MASK
    fi
    if [ "$1" == "clear" ] || [ "$1" == "none" ] || [ "$1" == "release" ]; then    # All debugging turned off
        export DVP_DEBUG=0
        export DVP_ZONE_MASK=0
        export SOSAL_DEBUG=0
        export SOSAL_ZONE_MASK=0
        export OMAPRPC_DEBUG=0
        export OMAPRPC_ZONE_MASK=0
    fi
    if [ "$1" == "full" ]; then
        export DVP_DEBUG=2
        export DVP_ZONE_MASK=0xFFFFFFFF
        export SOSAL_DEBUG=2
        export SOSAL_ZONE_MASK=0xFFFFFFFF
        export OMAPRPC_DEBUG=2
        export OMAPRPC_ZONE_MASK=0xFFFF
    fi
    if [ "$1" == "min" ]; then
        export DVP_DEBUG=2
        export DVP_ZONE_MASK=0x0003
        export SOSAL_DEBUG=2
        export SOSAL_ZONE_MASK=0x0003
        export OMAPRPC_DEBUG=2
        export OMAPRPC_ZONE_MASK=0x3
    fi
    if [ "$1" == "errors" ]; then
        export DVP_DEBUG=2
        export DVP_ZONE_MASK=0x0001
        export SOSAL_DEBUG=2
        export SOSAL_ZONE_MASK=0x0001
        export OMAPRPC_DEBUG=2
        export OMAPRPC_ZONE_MASK=0x1
    fi
    if [ "$1" == "perf" ]; then
        export DVP_DEBUG=2
        export DVP_ZONE_MASK=0x8203
        export SOSAL_DEBUG=2
        export SOSAL_ZONE_MASK=0x0003
        export OMAPRPC_DEBUG=2
        export OMAPRPC_ZONE_MASK=0x21
    fi
    if [ "$1" == "rpc" ]; then
        export DVP_DEBUG=2
        export DVP_ZONE_MASK=0x0233
        export SOSAL_DEBUG=2
        export SOSAL_ZONE_MASK=0x0001
        export OMAPRPC_DEBUG=2
        export OMAPRPC_ZONE_MASK=0x1
    fi
    if [ "$1" == "debug" ]; then
        export DVP_DEBUG=2
        export DVP_ZONE_MASK=0xFFFF
        export SOSAL_DEBUG=2
        export SOSAL_ZONE_MASK=0x1
        export OMAPRPC_DEBUG=2
        export OMAPRPC_ZONE_MASK=0x1
    fi
    if [ "$1" == "dvpbenchmark" ]; then # For use by benchmark.pl scripts
        export DVP_DEBUG=1
        export DVP_ZONE_MASK=0x0203
        export SOSAL_DEBUG=1
        export SOSAL_ZONE_MASK=0x1
        export OMAPRPC_DEBUG=1
        export OMAPRPC_ZONE_MASK=0x1
    fi
    if [ "$1" == "dvptest" ]; then
        export DVP_DEBUG=2
        export DVP_ZONE_MASK=0x0273
        export SOSAL_DEBUG=2
        export SOSAL_ZONE_MASK=0x3
        export OMAPRPC_DEBUG=2
        export OMAPRPC_ZONE_MASK=0x1
    fi
    if [ "$1" == "fullprint" ]; then
        export DVP_DEBUG=1
        export DVP_ZONE_MASK=0xFFFFFFFF
        export SOSAL_DEBUG=1
        export SOSAL_ZONE_MASK=0xFFFFFFFF
        export OMAPRPC_DEBUG=1
        export OMAPRPC_ZONE_MASK=0xFFFFFFFF
    fi
}

if [ -n "${MYDROID}" ]; then
    . dvp_android.sh $*
fi

if [ -n "${QNX_TARGET}" ]; then
    . dvp_qnx.sh $*
fi

if [ -z "${QNX_TARGET}" ] && [ -z "${MYDROID}" ]; then
    . dvp_linux.sh $*
fi
