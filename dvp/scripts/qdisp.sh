#!/system/bin/sh

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

export PATH=$PATH:/data/busybox/bin
chmod 755 /data/busybox/bin/*

OMAPDSS=/sys/devices/platform/omapdss

DISPLAYS="0 1 2 3"
OVERLAYS="0 1 2 3"
MANAGERS="0 1 2"

#function sysfs_read {
#    cd $1
#    echo -n $1
#    for i in `ls`; do
#        if [ -f $i ]; then
#            VALUE=`cat $i`
#            echo -n " $i=$VALUE"
#        fi
#    done
#}

for i in $DISPLAYS ;
do
    NAME=`cat $OMAPDSS/display$i/name`
    ENABLED=`cat $OMAPDSS/display$i/enabled`
    DRIVER=`cat $OMAPDSS/display$i/driver/name`
    #if [ $ENABLED == "true" ]; then
        echo "DISPLAY[$i]: is named $NAME uses driver $DRIVER enabled $ENABLED"
    #fi
done

for i in $MANAGERS ;
do
    NAME=`cat $OMAPDSS/manager$i/name`
    DISPLAY=`cat $OMAPDSS/manager$i/display`
    TRANS=`cat $OMAPDSS/manager$i/trans_key_enabled`
    TYPE=`cat $OMAPDSS/manager$i/trans_key_type`
    echo "MANAGER[$i]: is named $NAME uses display $DISPLAY t:$TRANS $TYPE"
done

for i in $OVERLAYS ;
do
    NAME=`cat $OMAPDSS/overlay$i/name`
    ENABLED=`cat $OMAPDSS/overlay$i/enabled`
    MANAGER=`cat $OMAPDSS/overlay$i/manager`
    ORDER=`cat $OMAPDSS/overlay$i/zorder`
    #if [ $ENABLED == "true" ]; then
        echo "OVERLAY[$i]: is named $NAME uses manager $MANAGER with zorder of $ORDER enabled $ENABLED"
    #fi
done

#sysfs_read $OMAPDSS/overlay2

