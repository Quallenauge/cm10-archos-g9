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

function move_videos {
    LISTING=`ls raw/ | grep "^[0-9][0-9]" | tr '\r' ' ' | tr '\n' ' '`
    for file in ${LISTING};
    do
        mv raw/$file raw/output/
    done
}

if [ -z "${DVP_ROOT}" ]; then
    echo "DVP_ROOT must be set"
    exit
fi

pushd ${DVP_ROOT}

while [ $# -gt 0 ];
do
    if [ "$1" == "setup" ]; then
        make clean
        make -j3
        make install

        #Prepare output directory
        mkdir $DVP_ROOT/raw/output
    fi

    if [ "$1" == "simcop" ] || [ "$1" == "dsp" ] || [ "$1" == "cpu" ]; then

        #QQVGA Test
        rm raw/0* raw/1* raw/2* raw/3* raw/4* raw/5* raw/6*
        echo TESTING ${DVP_TEST_GRAPH_STR} on $1 at QQVGA
        dvp_test handqqvga 160 120 30 UYVY 10 $DVP_TEST_GRAPH_NUM $1
        move_videos

        if [ $DVP_TEST_FULL ]; then
            #QVGA Test
            rm raw/0* raw/1* raw/2* raw/3* raw/4* raw/5* raw/6*
            echo TESTING ${DVP_TEST_GRAPH_STR} on $1 at QVGA
            dvp_test handqvga 320 240 30 UYVY 2 $DVP_TEST_GRAPH_NUM $1
            move_videos

            #VGA Test
            rm raw/0* raw/1* raw/2* raw/3* raw/4* raw/5* raw/6*
            echo TESTING ${DVP_TEST_GRAPH_STR} on $1 at VGA
            dvp_test handvga 640 480 30 UYVY 2 $DVP_TEST_GRAPH_NUM $1
            move_videos

            #4xVGA Test
            # rm raw/0* raw/1* raw/2* raw/3* raw/4* raw/5* raw/6*
            #echo TESTING ${DVP_TEST_GRAPH_STR} on $1 at 4xVGA
            # dvp_test hand4Xvga 1280 960 30 UYVY 2 $DVP_TEST_GRAPH_NUM $1
            # move_videos
        fi
        . $DVP_ROOT/scripts/dvp.sh convert

        echo RESULTS of ${DVP_TEST_GRAPH_STR} on $1 at QQVGA  >> regression_results.txt
        $DVP_ROOT/out/LINUX/ARM/imgDiff $DVP_ROOT/raw/ref/${DVP_TEST_GRAPH_STR}/qqvga/ $DVP_ROOT/raw/output/ $DVP_ROOT/raw/ref/${DVP_TEST_GRAPH_STR}_reference_config.txt 10 0 >> regression_results.txt
        if [ $DVP_TEST_FULL ]; then
            echo RESULTS of ${DVP_TEST_GRAPH_STR} on $1 at QVGA  >> regression_results.txt
            $DVP_ROOT/out/LINUX/ARM/imgDiff $DVP_ROOT/raw/ref/${DVP_TEST_GRAPH_STR}/qvga/ $DVP_ROOT/raw/output/ $DVP_ROOT/raw/ref/${DVP_TEST_GRAPH_STR}_reference_config.txt 2 0 >> regression_results.txt
            echo RESULTS of ${DVP_TEST_GRAPH_STR} on $1 at VGA  >> regression_results.txt
            $DVP_ROOT/out/LINUX/ARM/imgDiff $DVP_ROOT/raw/ref/${DVP_TEST_GRAPH_STR}/vga/ $DVP_ROOT/raw/output/ $DVP_ROOT/raw/ref/${DVP_TEST_GRAPH_STR}_reference_config.txt 2 0 >> regression_results.txt
            #echo RESULTS of ${DVP_TEST_GRAPH_STR} on $1 at 4xVGA  >> regression_results.txt
            #$DVP_ROOT/bin/imgDiff $DVP_ROOT/raw/ref/${DVP_TEST_GRAPH_STR}/4xvga/ $DVP_ROOT/raw/output/ $DVP_ROOT/raw/ref/${DVP_TEST_GRAPH_STR}_reference_config.txt 2 0 >> regression_results.txt
        fi
        echo >> regression_results.txt
    fi
    if [ "$1" == "report" ]; then
        cat regression_results.txt
        mv regression_results.txt regression_results_${DVP_TEST_GRAPH_STR}.txt
    fi
    if [ "$1" == "move" ]; then
        move_videos
    fi
    if [ "$1" == "clean" ]; then
        rm $DVP_ROOT/raw/0* $DVP_ROOT/raw/1* $DVP_ROOT/raw/2* $DVP_ROOT/raw/3* $DVP_ROOT/raw/4* $DVP_ROOT/raw/5* $DVP_ROOT/raw/6*
        rm $DVP_ROOT/raw/output/0* $DVP_ROOT/raw/output/1* $DVP_ROOT/raw/output/2* $DVP_ROOT/raw/output/3* $DVP_ROOT/raw/output/4* $DVP_ROOT/raw/output/5* $DVP_ROOT/raw/output/6*
    fi
    shift
done

popd
