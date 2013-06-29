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

function store_function()
{
    local orig=$(declare -f $1);
    if [ -n "$orig" ]; then
        local new="function orig_${orig}"
        eval "${new}"
    fi
}

function get_devices() 
{
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

function adb_remount() 
{
	get_devices
	adb -s ${TARGET_DEVICE} root
	adb "wait-for-device"
	adb -s ${TARGET_DEVICE} remount
}

function adb_pid()
{
	local pid=$(adb shell ps | grep $1 | awk '{print$2}')
	echo $pid
}

function adb_kill() 
{
	adb_remount
	local pid=$(adb_pid $1)
	adb shell kill $pid
}

function perms()
{
	cd ${TARGET_ROOT}
	# mark all directories as drwxr-xr-x
	find . -type d -exec chmod 755 {} \;
	# mark all files as rw-r--r--
	find . -type f -exec chmod 644 {} \;
	# mark all executable files (on PC) as rwxr-xr-x
	find . -type f -name "*\.bat" -exec chmod 755 {} \;
	find . -type f -name "*\.pl" -exec chmod 755 {} \;
	find . -type f -name "*\.x*" -exec chmod 755 {} \;
	find . -type f -name "*\.sh" -exec chmod 755 {} \;
	find . -type f -name "*\.out" -exec chmod 755 {} \;
	find . -type f -name "*\.rc" -exec chmod 644 {} \;
	find . -type f -name "*\.so" -exec chmod 755 {} \;
}

function unix()
{
	cd ${TARGET_ROOT}
	local FILES=`find . -type f \( -name *.c -o -name *.cpp -o -name *.h -o -name *.java -o -name *.aidl -o -name *.xml \) -print`
	for f in ${FILES}; do
		fromdos -v -p -o $f
	done
}

function clean()
{
	for i in ${TARGET_SLIB}; do
		echo Removing Static Library $i
		rm -rf ${ANDROID_PRODUCT_OUT}/obj/STATIC_LIBRARIES/${i}_intermediates/
	done
	for i in ${TARGET_LIBS}; do
		echo Removing Shared Library $i
		rm -rf ${ANDROID_PRODUCT_OUT}/obj/SHARED_LIBRARIES/${i}_intermediates/
		rm -rf ${ANDROID_PRODUCT_OUT}/symbols/system/lib/${i}.so
		rm -rf ${ANDROID_PRODUCT_OUT}/system/lib/${i}.so
	done
	for i in ${TARGET_BINS}; do
		echo Removing Binary $i
		rm -rf ${ANDROID_PRODUCT_OUT}/obj/EXECUTABLES/${i}_intermediates/
		rm -rf ${ANDROID_PRODUCT_OUT}/symbols/system/bin/${i}
		rm -rf ${ANDROID_PRODUCT_OUT}/system/bin/${i}
	done
	for i in ${TARGET_TEST}; do
		echo Removing Binary $i
		rm -rf ${ANDROID_PRODUCT_OUT}/obj/EXECUTABLES/${i}_intermediates/
		rm -rf ${ANDROID_PRODUCT_OUT}/symbols/system/bin/${i}
		rm -rf ${ANDROID_PRODUCT_OUT}/system/bin/${i}
	done
	for i in ${TARGET_APPS}; do
		echo Removing Android App $i
		rm -rf ${MYDROID}/out/target/common/obj/APPS/${i}_intermediates/
		rm -rf ${ANDROID_PRODUCT_OUT}/obj/APPS/${i}_intermediates/
	done
}

function remoteclean()
{
	adb_remount
	for i in ${TARGET_BINS}; do
		adb -s ${TARGET_DEVICE} shell "rm /system/bin/${i}"
	done
	for i in ${TARGET_TEST}; do
		adb -s ${TARGET_DEVICE} shell "rm /system/bin/${i}"
	done
	for i in ${TARGET_LIBS}; do
		adb -s ${TARGET_DEVICE} shell "rm /system/lib/${i}.so"
	done
	for i in ${TARGET_PKGS}; do
		adb -s ${TARGET_DEVICE} uninstall $i
		adb -s ${TARGET_DEVICE} shell "rm -rf /data/data/${i}"
	done
	for i in ${TARGET_APPS}; do
		adb -s ${TARGET_DEVICE} shell "rm -rf /system/apps/${i}.apk"
	done
	adb -s ${TARGET_DEVICE} shell "sync"
}

function install()
{
	adb_remount
	for i in ${TARGET_BINS}; do
		echo Installing ${i}
		adb -s ${TARGET_DEVICE} push ${ANDROID_PRODUCT_OUT}/system/bin/${i} /system/bin/
	done
	for i in ${TARGET_LIBS}; do
		echo Installing ${i}.so
		adb -s ${TARGET_DEVICE} push ${ANDROID_PRODUCT_OUT}/system/lib/${i}.so /system/lib/
	done
	adb -s ${TARGET_DEVICE} shell "sync"
}

function test()
{
	adb_remount
	for i in ${TARGET_TEST}; do
		echo Installing ${i}
		adb -s ${TARGET_DEVICE} push ${ANDROID_PRODUCT_OUT}/system/bin/${i} /system/bin/
	done
	adb -s ${TARGET_DEVICE} shell "sync"
}

function apps()
{
	adb_remount
	for i in ${TARGET_PKGS}; do
		echo Uninstalling ${i}
		adb -s ${TARGET_DEVICE} uninstall ${i}
		adb -s ${TARGET_DEVICE} shell "rm -rf /data/data/${i}"
	done
	for i in ${TARGET_APPS}; do
		echo Installing ${i}
		adb -s ${TARGET_DEVICE} shell "rm /system/app/${i}.apk"
		adb -s ${TARGET_DEVICE} install ${ANDROID_PRODUCT_OUT}/system/app/${i}.apk
	done
}

store_function mm
function mm()
{
	cd ${MYDROID}
    source build/envsetup.sh
    store_function mm
	cd ${TARGET_ROOT}
	orig_mm -j${CPUS}
}

function emmc()
{
	if [ ! -d "${EMMC_DIR}" ]; then
		echo "EMMC_DIR variable does not point to a valid directory."
		return 1;
	fi

	pushd ${EMMC_DIR}

	# Extracting system and data partitions
	./simg2img system.img system.img.raw
	mkdir tmp
	sudo mount -t ext4 -o loop system.img.raw tmp/
	sudo chmod -R a+w tmp
	# From "install"
	for i in ${TARGET_BINS}; do
		echo Copying ${i}
		cp ${ANDROID_PRODUCT_OUT}/system/bin/${i} tmp/bin/
	done
	for i in ${TARGET_LIBS}; do
		echo Copying lib${i}.so
		cp ${ANDROID_PRODUCT_OUT}/system/lib/${i}.so tmp/lib/
	done

	# From "test"
	for i in ${TARGET_TEST}; do
		echo Copying ${i}
		cp ${ANDROID_PRODUCT_OUT}/system/bin/${i} tmp/bin/
	done

	sudo ./make_ext4fs -s -l 512M -a system system.img tmp/
	sudo umount tmp
	rm -rf tmp
	rm -rf system.img.raw
	popd
}

function docs()
{
	export doxygen_status=`dpkg-query -W -f='${Status}\n' doxygen`
	if [ "$doxygen_status" == "install ok installed" ]; then
		cd ${TARGET_ROOT}
		local DATE=`date +%Y%m%d`
		local VERSION=$(git describe --tags | cut -c 9-)

		cd ${TARGET_ROOT}
    
    	(cat ${TARGET_ROOT}/docs/DoxyRules ; \
     	echo "PROJECT_NUMBER = \"r\" ${VERSION}" ;\
     	echo "OUTPUT_DIRECTORY = ${TARGET_ROOT}/docs/";) | doxygen -

		echo "Look in ${TARGET_ROOT}/docs/ for the generated documentation"
	else
		echo "doxygen package is not installed on your machine!"
		echo "To generate ${TARGET_PROJ} docs, install the package (sudo apt-get install doxygen) and re-run this option."
	fi
}

function zone()
{
	if [ -n "${1}" ]; then
        zone_mask ${1}
        post_arg_shift=1
    fi
    zone_mask print
}

function restart()
{
	local pid=$(adb_pid "arxd")
	echo "existing arxd pid: $pid"
	adb_kill "arxd"
	local pid=$(adb_pid "arxd")
	echo "new arxd pid: $pid"
}

if [ "${MYDROID}" == "" ]; then
    echo You must set your path to the Android Build in MYDROID.
    exit
fi

if [ $# == 0 ]; then
    echo "Usage: $0 [clean|remoteclean|mm|install|test|apps|restart|docs|jar]"
    echo ""
    echo " options: "
    echo "  clean       -  Deletes previously built libraries & intermediate files on the host machine"
    echo "  remoteclean -  Deletes previously libraries & intermediate files on the target device."
    echo "  mm          -  Builds entire TIARX project"
    echo "  install     -  Installs successfully-built libraries. Requires active adb connection!"
    echo "  test        -  Installs successfully-built unit tests. Requires active adb connection!"
    echo "  apps        -  Installs successfully-built apps. Requires active adb connection!"
    echo "  restart     -  Restarts arxd. Requires active adb connection!"
    echo "  docs     	-  Creates doxygen documentation for all of the tiarx project"
    echo ""
    exit
fi
################################################################################
echo "ARX: android out dir is ${ANDROID_PRODUCT_OUT}"
################################################################################
if [ -z "${TI_HW_ROOT}" ]; then
    export TI_HW_ROOT=hardware/ti
fi

export CPUS=`cat /proc/cpuinfo | grep processor | wc -l`

export TARGET_PROJ=arx
export TARGET_ROOT=${MYDROID}/${TI_HW_ROOT}/${TARGET_PROJ}
export TARGET_LIBS="libarx libarxbuf libarxengine libarxpose"
export TARGET_SLIB="libarxdaemon_ipc libarxclient_ipc libarxjni_util libarxengine_base"
export TARGET_BINS="arxd"
export TARGET_TEST="arx_test arxd_test"
export TARGET_APPS="ARCowboid ARXStreams"
export TARGET_PKGS="com.ti.arx.ARCowboid com.ti.arx.ARXStreams"

while [ $# -gt 0 ];
do
	cmd=$1
	shift
	arg_type=$(type -t ${cmd})
	post_arg_shift=0
	if [ "${arg_type}" == "function" ]; then
		eval "${cmd} $@"
		shift $post_arg_shift
	else
		echo "invalid command: ${cmd}"
	fi
done

