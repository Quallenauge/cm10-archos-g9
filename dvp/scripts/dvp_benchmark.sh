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

if [ "${MYDROID}" == "" ]; then
    echo You must set your path to the Android Build in MYDROID.
    exit
fi
if [ -z "${TI_HW_ROOT}" ]; then
    export TI_HW_ROOT=hardware/ti
fi
if [ -z "${VISION_ROOT}" ]; then
    export VISION_ROOT=hardware/ti/vision
    echo "VISION_ROOT not defined, using $VISION_ROOT"
fi
if [ $# == 0 ]; then
    echo "Usage: dvp_benchmark.sh [name <directory name> setup <graph_name> <core> report]"
    echo "  graph_name: common|vrun|vrun2|ldc|imglib|vlib|rvm|tismo1|tismo2"
    echo "  core      : simcop|dsp|cpu|all "
    echo "Example: benchmark.sh name 120418_ICS setup common all report "
    exit
fi

export DVP_ROOT=${MYDROID}/${TI_HW_ROOT}/dvp
export HDR_FILE=${DVP_ROOT}/include/dvp/dvp_types.h
export PUBLIC_LIB_HEADER_LIST=" ${DVP_ROOT}/libraries/public/yuv/include/yuv/dvp_kl_yuv.h
                                ${DVP_ROOT}/libraries/public/imgfilter/include/imgfilter/dvp_kl_imgfilter.h
                                ${DVP_ROOT}/libraries/public/ocl/include/ocl/dvp_kl_ocl.h
                                ${DVP_ROOT}/libraries/public/vrun/include/vrun/dvp_kl_vrun.h
                                ${DVP_ROOT}/libraries/public/dsplib/include/dsplib/dvp_kl_dsplib.h
                                ${DVP_ROOT}/libraries/public/dei/include/dei/dvp_kl_dei.h"
export PRIVATE_LIB_HEADER_LIST="${MYDROID}/${VISION_ROOT}/libraries/protected/imglib/include/imglib/dvp_kl_imglib.h
                                ${MYDROID}/${VISION_ROOT}/libraries/protected/vlib/include/vlib/dvp_kl_vlib.h
                                ${MYDROID}/${VISION_ROOT}/libraries/protected/rvm/include/rvm/dvp_kl_rvm.h
                                ${MYDROID}/${VISION_ROOT}/libraries/protected/tismo/include/tismo/dvp_kl_tismo.h
                                ${MYDROID}/${VISION_ROOT}/libraries/protected/tismov02/include/tismov02/dvp_kl_tismov02.h"
export LIBHEADERS=

export REPORT_DEBUG=    #set to something to debug the report generation without running the test
export FRAMES=2         #set to number of frames to run
export OMX=             #set to something to run the camera as the input, otherwise it will use file based input

if [ ${OMX} ]; then
    export QQVGA_FILE=OMX
    export QVGA_FILE=OMX
    export VGA_FILE=OMX
    export x4VGA_FILE=OMX
else
    export QQVGA_FILE=handqqvga
    export QVGA_FILE=handqvga
    export VGA_FILE=handvga
    export x4VGA_FILE=hand4Xvga
fi

if [[ "$@" =~ "common" ]]; then
    export DVP_TEST_GRAPH_NUM=7
    export DVP_TEST_GRAPH_STR="common"
fi
if [[ "$@" =~ "vrun" ]]; then
    export DVP_TEST_GRAPH_NUM=8
    export DVP_TEST_GRAPH_STR="vrun"
fi
if [[ "$@" =~ "vrun2" ]]; then
    export DVP_TEST_GRAPH_NUM=9
    export DVP_TEST_GRAPH_STR="vrun2"
fi
if [[ "$@" =~ "ldc" ]]; then
    export DVP_TEST_GRAPH_NUM=17
    export DVP_TEST_GRAPH_STR="ldc"
fi
if [[ "$@" =~ "imglib" ]]; then
    export DVP_TEST_GRAPH_NUM=16
    export DVP_TEST_GRAPH_STR="imglib"
fi
if [[ "$@" =~ "vlib" ]]; then
    export DVP_TEST_GRAPH_NUM=22
    export DVP_TEST_GRAPH_STR="vlib"
fi
if [[ "$@" =~ "rvm" ]]; then
    export DVP_TEST_GRAPH_NUM=1
    export DVP_TEST_GRAPH_STR="rvm"
fi
if [[ "$@" =~ "tismo1" ]]; then
    export DVP_TEST_GRAPH_NUM=15
    export DVP_TEST_GRAPH_STR="tismo1"
fi
if [[ "$@" =~ "tismo2" ]]; then
    export DVP_TEST_GRAPH_NUM=27
    export DVP_TEST_GRAPH_STR="tismo2"
fi

while [ $# -gt 0 ];
do
    if [ "${1}" == "name" ]; then
        shift
        export DVP_BENCH_ROOT=${MYDROID}/${TI_HW_ROOT}/dvp/benchmarking/$1
        if [ ! -d "${MYDROID}/${TI_HW_ROOT}/dvp/benchmarking" ]; then
            mkdir ${MYDROID}/${TI_HW_ROOT}/dvp/benchmarking
        fi
        if [ ! -d "${DVP_BENCH_ROOT}" ]; then
            echo "Creating directories for results at /dvp/benchmarking"
            mkdir ${DVP_BENCH_ROOT}
            mkdir ${DVP_BENCH_ROOT}/simcop
            mkdir ${DVP_BENCH_ROOT}/simcop/qqvga
            mkdir ${DVP_BENCH_ROOT}/simcop/qvga
            mkdir ${DVP_BENCH_ROOT}/simcop/vga
            #mkdir ${DVP_BENCH_ROOT}/simcop/4xvga
            mkdir ${DVP_BENCH_ROOT}/dsp
            mkdir ${DVP_BENCH_ROOT}/dsp/qqvga
            mkdir ${DVP_BENCH_ROOT}/dsp/qvga
            mkdir ${DVP_BENCH_ROOT}/dsp/vga
            #mkdir ${DVP_BENCH_ROOT}/dsp/4xvga
            mkdir ${DVP_BENCH_ROOT}/cpu
            mkdir ${DVP_BENCH_ROOT}/cpu/qqvga
            mkdir ${DVP_BENCH_ROOT}/cpu/qvga
            mkdir ${DVP_BENCH_ROOT}/cpu/vga
            #mkdir ${DVP_BENCH_ROOT}/cpu/4xvga
        else
            echo "Directory /dvp/benchmarking exists, will overwrite previous data"
        fi
    fi
    if [ "${1}" == "setup" ]; then
        # Connect ADB, Compile, install
        $DVP_ROOT/scripts/dvp.sh setup zone dvpbenchmark clean mm install test

        #Load input videos
        adb push $DVP_ROOT/raw/input/handqqvga_160x120_30Hz_UYVY.yuv /sdcard/raw/.
        adb push $DVP_ROOT/raw/input/handqvga_320x240_30Hz_UYVY.yuv /sdcard/raw/.
        adb push $DVP_ROOT/raw/input/handvga_640x480_30Hz_UYVY.yuv /sdcard/raw/.
        adb push $DVP_ROOT/raw/input/hand4Xvga_1280x960_30Hz_UYVY.yuv /sdcard/raw/.
        adb push $DVP_ROOT/raw/input/handqqvga_160x360_30Hz_P400.bw /sdcard/raw/.
        adb push $DVP_ROOT/raw/input/handqvga_320x720_30Hz_P400.bw /sdcard/raw/.
        adb push $DVP_ROOT/raw/input/handvga_640x1440_30Hz_P400.bw /sdcard/raw/.
        adb push $DVP_ROOT/raw/input/rvm_calib_160x120_to_160x120.txt /sdcard/raw/.
        adb push $DVP_ROOT/raw/input/rvm_calib_320x240_to_320x240.txt /sdcard/raw/.
        adb push $DVP_ROOT/raw/input/rvm_calib_640x480_to_640x480.txt /sdcard/raw/.

        adb shell "echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
        export A9FREQ=`adb shell "cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq"`
        echo "Set the A9 Frequency to $A9FREQ"
    fi
    if [ "${1}" == "simcop" ] || [ "${1}" == "all" ]; then
        export SIMCOP_FLAG=1
        echo Running SIMCOP QQVGA from ${QQVGA_FILE}
        adb shell "cd /sdcard && dvp_test ${QQVGA_FILE} 160 120 30 UYVY ${FRAMES} ${DVP_TEST_GRAPH_NUM} simcop" > ${DVP_BENCH_ROOT}/simcop/qqvga/log_${DVP_TEST_GRAPH_STR}
        echo Running SIMCOP QVGA from ${QVGA_FILE}
        adb shell "cd /sdcard && dvp_test ${QVGA_FILE} 320 240 30 UYVY ${FRAMES} ${DVP_TEST_GRAPH_NUM} simcop" > ${DVP_BENCH_ROOT}/simcop/qvga/log_${DVP_TEST_GRAPH_STR}
        echo Running SIMCOP VGA from ${VGA_FILE}
        adb shell "cd /sdcard && dvp_test ${VGA_FILE} 640 480 30 UYVY ${FRAMES} ${DVP_TEST_GRAPH_NUM} simcop" > ${DVP_BENCH_ROOT}/simcop/vga/log_${DVP_TEST_GRAPH_STR}
        #echo Running SIMCOP 4XVGA from ${x4VGA_FILE}
        #adb shell "cd /sdcard && dvp_test ${x4VGA_FILE} 1280 960 30 UYVY ${FRAMES} 7 simcop" > ${DVP_BENCH_ROOT}/simcop/4xvga/log_${DVP_TEST_GRAPH_STR}
    fi
    if [ "${1}" == "dsp" ] || [ "${1}" == "all" ]; then
        export DSP_FLAG=1
        echo Running DSP QQVGA from ${QQVGA_FILE}
        adb shell "cd /sdcard && dvp_test ${QQVGA_FILE} 160 120 30 UYVY ${FRAMES} ${DVP_TEST_GRAPH_NUM} dsp" > ${DVP_BENCH_ROOT}/dsp/qqvga/log_${DVP_TEST_GRAPH_STR}
        echo Running DSP QVGA from ${QVGA_FILE}
        adb shell "cd /sdcard && dvp_test ${QVGA_FILE} 320 240 30 UYVY ${FRAMES} ${DVP_TEST_GRAPH_NUM} dsp" > ${DVP_BENCH_ROOT}/dsp/qvga/log_${DVP_TEST_GRAPH_STR}
        echo Running DSP VGA from ${VGA_FILE}
        adb shell "cd /sdcard && dvp_test ${VGA_FILE} 640 480 30 UYVY ${FRAMES} ${DVP_TEST_GRAPH_NUM} dsp" > ${DVP_BENCH_ROOT}/dsp/vga/log_${DVP_TEST_GRAPH_STR}
        #echo Running DSP 4XVGA from ${x4VGA_FILE}
        #adb shell "cd /sdcard && dvp_test ${x4VGA_FILE} 1280 960 30 UYVY ${FRAMES} ${DVP_TEST_GRAPH_NUM} dsp" > ${DVP_BENCH_ROOT}/dsp/4xvga/log_${DVP_TEST_GRAPH_STR}
    fi
    if [ "${1}" == "cpu" ] || [ "${1}" == "all" ]; then
        export CPU_FLAG=1
        echo Running CPU QQVGA from ${QQVGA_FILE}
        adb shell "cd /sdcard && dvp_test ${QQVGA_FILE} 160 120 30 UYVY ${FRAMES} ${DVP_TEST_GRAPH_NUM} cpu" > ${DVP_BENCH_ROOT}/cpu/qqvga/log_${DVP_TEST_GRAPH_STR}
        echo Running CPU QVGA from ${QVGA_FILE}
        adb shell "cd /sdcard && dvp_test ${QVGA_FILE} 320 240 30 UYVY ${FRAMES} ${DVP_TEST_GRAPH_NUM} cpu" > ${DVP_BENCH_ROOT}/cpu/qvga/log_${DVP_TEST_GRAPH_STR}
        echo Running CPU VGA from ${VGA_FILE}
        adb shell "cd /sdcard && dvp_test ${VGA_FILE} 640 480 30 UYVY ${FRAMES} ${DVP_TEST_GRAPH_NUM} cpu" > ${DVP_BENCH_ROOT}/cpu/vga/log_${DVP_TEST_GRAPH_STR}
        #echo Running CPU 4XVGA from ${x4VGA_FILE}
        #adb shell "cd /sdcard && dvp_test ${x4VGA_FILE} 1280 960 30 UYVY ${FRAMES} ${DVP_TEST_GRAPH_NUM} cpu" > ${DVP_BENCH_ROOT}/cpu/4xvga/log_${DVP_TEST_GRAPH_STR}
    fi
    if [ "${1}" == "report" ]; then
        # Only include the headers that exist from vision_prebuilt
        for i in ${PRIVATE_LIB_HEADER_LIST}; do
            if [ -f ${i} ]; then
                LIBHEADERS+="${i} ";
                echo "Library Header File Found: ${i}";
            fi
        done
        if [ ${REPORT_DEBUG} ]; then
            export SIMCOP_FLAG=1
            export DSP_FLAG=1
            export CPU_FLAG=1
        fi
        if [ -f "${DVP_BENCH_ROOT}/simcop/qqvga/log_${DVP_TEST_GRAPH_STR}" ] && [ ${SIMCOP_FLAG} == 1 ]; then
            unset SIMCOP_FLAG
            echo "Generating SIMCOP report"
            report.pl ${DVP_TEST_GRAPH_STR} ${HDR_FILE} ${DVP_BENCH_ROOT}/simcop 1 ${FRAMES} ${LIBHEADERS} ${PUBLIC_LIB_HEADER_LIST}
        fi
        if [ -f "${DVP_BENCH_ROOT}/dsp/qqvga/log_${DVP_TEST_GRAPH_STR}" ] && [ ${DSP_FLAG} == 1 ]; then
            unset DSP_FLAG
            echo "Generating DSP report"
            report.pl ${DVP_TEST_GRAPH_STR} ${HDR_FILE} ${DVP_BENCH_ROOT}/dsp 1 ${FRAMES} ${LIBHEADERS} ${PUBLIC_LIB_HEADER_LIST}
        fi
        if [ -f "${DVP_BENCH_ROOT}/cpu/qqvga/log_${DVP_TEST_GRAPH_STR}" ] && [ ${CPU_FLAG} == 1 ]; then
            unset CPU_FLAG
            echo "Generating CPU report"
            report.pl ${DVP_TEST_GRAPH_STR} ${HDR_FILE} ${DVP_BENCH_ROOT}/cpu 1 ${FRAMES} ${LIBHEADERS} ${PUBLIC_LIB_HEADER_LIST}
        fi
    fi
    shift
done
