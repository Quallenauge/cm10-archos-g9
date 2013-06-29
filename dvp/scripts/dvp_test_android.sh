#!/bin/bash

# Copyright (C) 2011 Texas Insruments, Inc.
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

export DVP_ROOT=${MYDROID}/${TI_HW_ROOT}/dvp

function get_devices() {
    if [ -z "${TARGET_DEVICE}" ]; then
        NUM_DEV=`adb devices | grep -E "^[0-9A-Fa-f]+" | awk '{print $1}' | wc -l`
        if [ "${NUM_DEV}" == "1" ]; then
            export TARGET_DEVICE=`adb devices | grep -E "^[0-9A-Fa-f]+" | awk '{print $1}'`
        elif [ "${NUM_DEV}" -gt "1" ]; then
            echo "You must set your TARGET_DEVICE to the serial number of the device to program. You have ${NUM_DEV} devices connected now!"
            exit
        else
            echo "You must have a device plugged in to use ADB"
            exit
        fi
    fi
}

function remote_clean() {
    adb -s ${TARGET_DEVICE} shell "rm /sdcard/raw/0* /sdcard/raw/1* /sdcard/raw/2* /sdcard/raw/3* /sdcard/raw/4* /sdcard/raw/5* /sdcard/raw/6*"
}

while [ $# -gt 0 ];
do
    get_devices
    if [ "$1" == "setup" ]; then
        # Connect ADB, Compile, install
        . $DVP_ROOT/scripts/dvp.sh setup logcat zone dvptest clean mm install test

        #Load RVM binary
        echo Installing rvm_generate_lut_ARM
        adb -s ${TARGET_DEVICE} push ${ANDROID_PRODUCT_OUT}/system/bin/rvm_generate_lut_ARM /system/bin/rvm_generate_lut

        #Load input videos
        adb -s ${TARGET_DEVICE} push $DVP_ROOT/raw/input/handqqvga_160x120_30Hz_UYVY.yuv /sdcard/raw/.
        adb -s ${TARGET_DEVICE} push $DVP_ROOT/raw/input/handqvga_320x240_30Hz_UYVY.yuv /sdcard/raw/.
        adb -s ${TARGET_DEVICE} push $DVP_ROOT/raw/input/handvga_640x480_30Hz_UYVY.yuv /sdcard/raw/.
        adb -s ${TARGET_DEVICE} push $DVP_ROOT/raw/input/hand4Xvga_1280x960_30Hz_UYVY.yuv /sdcard/raw/.

        adb -s ${TARGET_DEVICE} push $DVP_ROOT/raw/input/handqqvga_160x360_30Hz_P400.bw /sdcard/raw/.
        adb -s ${TARGET_DEVICE} push $DVP_ROOT/raw/input/handqvga_320x720_30Hz_P400.bw /sdcard/raw/.
        adb -s ${TARGET_DEVICE} push $DVP_ROOT/raw/input/handvga_640x1440_30Hz_P400.bw /sdcard/raw/.

#        adb -s ${TARGET_DEVICE} push $DVP_ROOT/raw/input/rvm_lut_320x240_to_320x240_BL64.bin /sdcard/raw/.
        adb -s ${TARGET_DEVICE} push $DVP_ROOT/raw/input/rvm_calib_160x120_to_160x120.txt /sdcard/raw/.
        adb -s ${TARGET_DEVICE} push $DVP_ROOT/raw/input/rvm_calib_320x240_to_320x240.txt /sdcard/raw/.
        adb -s ${TARGET_DEVICE} push $DVP_ROOT/raw/input/rvm_calib_640x480_to_640x480.txt /sdcard/raw/.
#        adb -s ${TARGET_DEVICE} push $DVP_ROOT/raw/input/rvm_calib_1280x960_to_1280x960.txt /sdcard/raw/.

        #Prepare output directory
        mkdir -p $DVP_ROOT/raw/output
    fi

    if [ "$1" == "simcop" ] || [ "$1" == "dsp" ] || [ "$1" == "cpu" ]; then

        #QQVGA Test
        remote_clean
        echo TESTING ${DVP_TEST_GRAPH_STR} on $1 at QQVGA
        adb -s ${TARGET_DEVICE} shell "cd /sdcard && dvp_test handqqvga 160 120 30 UYVY 10 $DVP_TEST_GRAPH_NUM $1"
        $DVP_ROOT/scripts/dvp.sh videos

        if [ $DVP_TEST_FULL ]; then
            #QVGA Test
            remote_clean
            echo TESTING ${DVP_TEST_GRAPH_STR} on $1 at QVGA
            adb -s ${TARGET_DEVICE} shell "cd /sdcard && dvp_test handqvga 320 240 30 UYVY 2 $DVP_TEST_GRAPH_NUM $1"
            $DVP_ROOT/scripts/dvp.sh videos

            #VGA Test
            remote_clean
            echo TESTING ${DVP_TEST_GRAPH_STR} on $1 at VGA
            adb -s ${TARGET_DEVICE} shell "cd /sdcard && dvp_test handvga 640 480 30 UYVY 2 $DVP_TEST_GRAPH_NUM $1"
            $DVP_ROOT/scripts/dvp.sh videos

            #4xVGA Test
            #remote_clean
            #echo TESTING ${DVP_TEST_GRAPH_STR} on $1 at 4xVGA
            #adb -s ${TARGET_DEVICE} shell "cd /sdcard && dvp_test hand4Xvga 1280 960 30 UYVY 2 $DVP_TEST_GRAPH_NUM $1"
            #$DVP_ROOT/scripts/dvp.sh videos
        fi
        . $DVP_ROOT/scripts/dvp.sh convert

        if [ -f $DVP_ROOT/raw/ref/${DVP_TEST_GRAPH_STR}_reference_config.txt ]; then
            echo RESULTS of ${DVP_TEST_GRAPH_STR} on $1 at QQVGA  >> regression_results.txt
            $DVP_ROOT/bin/imgDiff $DVP_ROOT/raw/ref/$DVP_TEST_GRAPH_STR/qqvga/ $DVP_ROOT/raw/output/ $DVP_ROOT/raw/ref/${DVP_TEST_GRAPH_STR}_reference_config.txt 10 0  >> regression_results.txt
            if [ $DVP_TEST_FULL ]; then
                echo RESULTS of ${DVP_TEST_GRAPH_STR} on $1 at QVGA  >> regression_results.txt
                $DVP_ROOT/bin/imgDiff $DVP_ROOT/raw/ref/$DVP_TEST_GRAPH_STR/qvga/ $DVP_ROOT/raw/output/ $DVP_ROOT/raw/ref/${DVP_TEST_GRAPH_STR}_reference_config.txt 2 0 >> regression_results.txt
                echo RESULTS of ${DVP_TEST_GRAPH_STR} on $1 at VGA  >> regression_results.txt
                $DVP_ROOT/bin/imgDiff $DVP_ROOT/raw/ref/$DVP_TEST_GRAPH_STR/vga/ $DVP_ROOT/raw/output/ $DVP_ROOT/raw/ref/${DVP_TEST_GRAPH_STR}_reference_config.txt 2 0 >> regression_results.txt
                #echo RESULTS of ${DVP_TEST_GRAPH_STR} on $1 at 4xVGA  >> regression_results.txt
                #$DVP_ROOT/bin/imgDiff $DVP_ROOT/raw/ref/$DVP_TEST_GRAPH_STR/4xvga/ $DVP_ROOT/raw/output/ $DVP_ROOT/raw/ref/${DVP_TEST_GRAPH_STR}_reference_config.txt 2 0 >> regression_results.txt
            fi
            echo >> regression_results.txt
        else
            echo "$DVP_ROOT/raw/ref/${DVP_TEST_GRAPH_STR}_reference_config.txt does not exist, make sure you are using proper version of raw folder."
        fi
    fi
    if [ "$1" == "report" ]; then
        cat regression_results.txt
        mv regression_results.txt regression_results_${DVP_TEST_GRAPH_STR}.txt
    fi
    if [ "$1" == "clean" ]; then
        remote_clean
        rm $DVP_ROOT/raw/output/0* $DVP_ROOT/raw/output/1* $DVP_ROOT/raw/output/2* $DVP_ROOT/raw/output/3* $DVP_ROOT/raw/output/4* $DVP_ROOT/raw/output/5* $DVP_ROOT/raw/output/6*
    fi
    shift
done
