#!/bin/bash

# Copyright (C) 2012 Texas Insruments, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Set the appropriate path

if [ -n "${MYDROID}" ]; then
    if [ "${VISION_ROOT}" == "" ]; then
        export VISION_ROOT=hardware/ti/vision
    fi
    export VR=${MYDROID}/${VISION_ROOT}/libraries/protected/
fi

if [ -z "${MYDROID}" ] && [ -z "${QNX_TARGET}" ]; then
    if [ "${VISION_ROOT}" == "" ]; then
        echo You must set your VISION_ROOT path.
        exit
    fi
    export VR=${VISION_ROOT}/libraries/protected/
fi

dvp_test.sh clean setup

if [ -d ${VR}/vlib ] && [ -d ${VR}/imglib ]; then
    dvp_test.sh clean common simcop dsp cpu full report
fi

dvp_test.sh clean vrun simcop full report
dvp_test.sh clean vrun2 simcop full report
dvp_test.sh clean ldc simcop full report

if [ -d ${VR}/rvm ]; then
dvp_test.sh clean rvm dsp cpu full report
fi

if [ -d ${VR}/tismo ]; then
dvp_test.sh clean tismo1 dsp full report
fi

if [ -d ${VR}/tismov02 ]; then
dvp_test.sh clean tismo2 dsp full report
fi

#dvp_test.sh clean vlib dsp cpu full report
#dvp_test.sh clean imglib dsp cpu full report
cat regression_results* > regression_results_all.txt
