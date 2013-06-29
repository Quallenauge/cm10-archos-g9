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

if [ "$DVP_ROOT" == "" ]; then
    echo You must DVP_ROOT environment variable.
    exit
fi

if [ $# == 0 ]; then
    echo "Usage: dvp_test_qnx.sh [setup|clean|imx|dsp|cpu|full|check]"
    exit
fi

if [ -z "`usb-devices | grep \"RIM Network Device\"`" ]; then
	echo "ERROR: PlayBook not found!"
	exit 0
fi

PB_HOST_IP=`ifconfig usb0 | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}'`

PB_TARGET_IP1=`echo $PB_HOST_IP | cut -d. -f1`
PB_TARGET_IP2=`echo $PB_HOST_IP | cut -d. -f2`
PB_TARGET_IP3=`echo $PB_HOST_IP | cut -d. -f3`
PB_TARGET_IP4=`echo $PB_HOST_IP | cut -d. -f4`
PB_TARGET_IP4=`echo ${PB_TARGET_IP4} - 1 | bc`
PB_TARGET_IP=`echo ${PB_TARGET_IP1}.${PB_TARGET_IP2}.${PB_TARGET_IP3}.${PB_TARGET_IP4}`

if [ -z $PB_TARGET_IP1 ]; then
	echo "ERROR: PlayBook link not giving any IP address!"
	exit 0
fi

export CPUS=`cat /proc/cpuinfo | grep processor | wc -l`

while [ $# -gt 0 ];
do
    if [ "$1" == "setup" ]; then
        # Compile, install
        export DVP_DEBUG=1
        export DVP_ZONE_MASK=0x10203
        $DVP_ROOT/scripts/dvp_qnx.sh clean mm install test

        #Load input videos
        scp $DVP_ROOT/raw/input/handqqvga_160x120_30Hz_UYVY.yuv root@${PB_TARGET_IP}:/root/raw/.
        scp $DVP_ROOT/raw/input/handqvga_320x240_30Hz_UYVY.yuv root@${PB_TARGET_IP}:/root/raw/.
        scp $DVP_ROOT/raw/input/handvga_640x480_30Hz_UYVY.yuv root@${PB_TARGET_IP}:/root/raw/.
        scp $DVP_ROOT/raw/input/hand4Xvga_1280x960_30Hz_UYVY.yuv root@${PB_TARGET_IP}:/root/raw/.

        #Prepare output directory
        mkdir $DVP_ROOT/raw/output
    fi

    if [ "$1" == "imx" ] || [ "$1" == "dsp" ] || [ "$1" == "cpu" ]; then

        if [ "$1" == "imx" ]; then
            export DVP_TEST_CORE=7
        fi
        if [ "$1" == "dsp" ]; then
            export DVP_TEST_CORE=8
        fi
        if [ "$1" == "cpu" ]; then
            export DVP_TEST_CORE=9
        fi

        #QQVGA Test
        ssh root@${PB_TARGET_IP} "rm /root/raw/0* /root/raw/1* /root/raw/2* /root/raw/3*"
        ssh root@${PB_TARGET_IP} "dvp_test handqqvga 160 120 30 UYVY 10 $DVP_TEST_CORE"
        $DVP_ROOT/scripts/dvp_qnx.sh videos

        if [ "$2" == "full" ] || [ $DVP_TEST_FULL ]; then
            #QVGA Test
            ssh root@${PB_TARGET_IP} "rm /root/raw/0* /root/raw/1* /root/raw/2* /sdcard/raw/3*"
            ssh root@${PB_TARGET_IP} "dvp_test handqvga 320 240 30 UYVY 2 $DVP_TEST_CORE"
            $DVP_ROOT/scripts/dvp_qnx.sh videos

            #VGA Test
            ssh root@${PB_TARGET_IP} "rm /root/raw/0* /root/raw/1* /root/raw/2* /sdcard/raw/3*"
            ssh root@${PB_TARGET_IP} "dvp_test handvga 640 480 30 UYVY 2 $DVP_TEST_CORE"
            $DVP_ROOT/scripts/dvp_qnx.sh videos

            #4xVGA Test
            ssh root@${PB_TARGET_IP} "rm /root/raw/0* /root/raw/1* /root/raw/2* /sdcard/raw/3*"
            #ssh root@${PB_TARGET_IP} "dvp_test hand4Xvga 1280 960 30 UYVY 2 $DVP_TEST_CORE"
            #$DVP_ROOT/scripts/dvp_qnx.sh videos
        fi
        $DVP_ROOT/scripts/dvp_qnx.sh convert
    fi
    if [ "$1" == "full" ]; then
        export DVP_TEST_FULL=1
    fi
    if [ "$1" == "check" ]; then
        echo "**********************************************************"
        echo "******************** TEST RESULTS ************************"
        echo "**********************************************************"
        echo
        echo QQVGA
        $DVP_ROOT/bin/imgDiff $DVP_ROOT/raw/ref/qqvga/ $DVP_ROOT/raw/output/ $DVP_ROOT/raw/ref/reference_config.txt 10
        if [ "$2" == "full" ] || [ $DVP_TEST_FULL ]; then
            echo QVGA
            $DVP_ROOT/bin/imgDiff $DVP_ROOT/raw/ref/qvga/ $DVP_ROOT/raw/output/ $DVP_ROOT/raw/ref/reference_config.txt 2
            echo VGA
            $DVP_ROOT/bin/imgDiff $DVP_ROOT/raw/ref/vga/ $DVP_ROOT/raw/output/ $DVP_ROOT/raw/ref/reference_config.txt 2
            #echo 4xVGA
            #$DVP_ROOT/bin/imgDiff $DVP_ROOT/raw/ref/4xvga/ $DVP_ROOT/raw/output/ $DVP_ROOT/raw/ref/reference_config.txt 2
        fi
    fi
    if [ "$1" == "clean" ]; then
        rm $DVP_ROOT/raw/output/0* $DVP_ROOT/raw/output/1* $DVP_ROOT/raw/output/2* $DVP_ROOT/raw/output/3*
    fi
    shift
done

