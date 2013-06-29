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


function make_install_deplibs() {
    DEPS=$*
    for i in ${DEPS}; do
        echo Making ${i}
    done
    make -j${CPUS} ${DEPS}
    for i in ${DEPS}; do
        if [ "${i}" == "camera.omap4" ]; then
            adb -s ${TARGET_DEVICE} push ${ANDROID_PRODUCT_OUT}/system/lib/hw/${i}.so /system/lib/hw
        else
            adb -s ${TARGET_DEVICE} push ${ANDROID_PRODUCT_OUT}/system/lib/${i}.so /system/lib
        fi

    done
    adb -s ${TARGET_DEVICE} shell "sync"
}

function get_symbols() {
    FILENAME=${1}
    FILETYPE=${2}
    OBJOPT="-C -d -g -f -l -S --include=${TARGET_ROOT}"
    echo "Processing $filename"
    if [ "${FILETYPE}" == "lib" ] && [ -e ${ANDROID_PRODUCT_OUT}/symbols/system/lib/${FILENAME} ]; then
        arm-eabi-objdump ${OBJOPT} ${ANDROID_PRODUCT_OUT}/symbols/system/lib/${FILENAME} > ${TARGET_ROOT}/out/${FILENAME}.txt
    fi
    if [ "${FILETYPE}" == "bin" ] && [ -e ${ANDROID_PRODUCT_OUT}/symbols/system/bin/${FILENAME} ]; then
        arm-eabi-objdump ${OBJOPT} ${ANDROID_PRODUCT_OUT}/symbols/system/bin/${FILENAME} > ${TARGET_ROOT}/out/${FILENAME}.txt
    fi
}

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

if [ "${MYDROID}" == "" ]; then
    echo You must set your path to the Android Build in MYDROID.
    exit
fi

if [ $# == 0 ]; then
    echo "Usage: $0 [clean|build|vars|setup|install|videos|convert|test|rvm|vlib|baseimage|music|apps]"
    echo ""
    echo " options: "
    echo "  clean       -  Deletes previously built libraries & intermediate files on the host machine"
    echo "  remoteclean -  Deletes previously libraries & intermediate files on the target device."
    echo "  build       -  Builds entire DVP project"
    echo "  vars        -  Checks for your currently defined environment build variables relating to DVP."
    echo "  install     -  Installs successfully-built libraries and applications. Requires active adb connection!"
    echo "                 -- You might want to run the 'DVP.sh setup' option first --"
    echo "  setup       -  Configures your adb connection.  Requires 'ADB_HOST' variable to be defined! (e.g. ADB_HOST=125.247.xx.yy)"
    echo "  videos      -  Downloads processed video files from target device to DVP's root folder/raw on host machine"
    echo "  convert     -  "
    echo "  test        -  Runs the DVP framework in various standalone test modes."
    echo "  baseimage   -  Installs the Ducati / Tesla binaries & the Syslink on the target device and reboot the target device."
    echo ""
    exit
fi
################################################################################
if [ -z "${_PRODUCT_}" ]; then
    export _PRODUCT_=blaze_tablet
fi
echo "Target product is ${_PRODUCT_}"

if [ -z "${TI_HW_ROOT}" ]; then
    export TI_HW_ROOT=hardware/ti
fi
if [ -z "${TARGET_ANDROID}" ]; then
    export TARGET_ANDROID=ICS
    if [ "${_PRODUCT_}" == "blaze_tablet" ] || [ "${_PRODUCT_}" == "blaze" ] || [ "${_PRODUCT_}" == "omap5sevm" ]; then
        export TARGET_ANDROID=JB
    fi
fi
if [ "${_PRODUCT_}" == "maguro" ] || [ "${_PRODUCT_}" == "tuna" ]; then
    export TARGET_ANDROID=JB
fi
if [ "${_PRODUCT_}" == "p2" ]; then
    export TARGET_ANDROID=ICS
fi
################################################################################
echo "DVP Configured for ${TARGET_ANDROID}"
################################################################################
export ANDROID_PRODUCT_OUT=${MYDROID}/out/target/product/${_PRODUCT_}
export DVP_ROOT=${MYDROID}/${TI_HW_ROOT}/dvp
export CPUS=`cat /proc/cpuinfo | grep processor | wc -l`

export TARGET_PROJ=dvp
export TARGET_ROOT="${MYDROID}/${TI_HW_ROOT}/${TARGET_PROJ}"
export TARGET_LIBS="dvp dvp_kgm_cpu dvp_kgm_simcop dvp_kgm_dsp"
if [ "${TARGET_ANDROID}" == "ICS" ] || [ "${TARGET_ANDROID}" == "JB" ]; then
export TARGET_SLIB="cthreaded VisionEngine pyuv sosal vcam v4l2 imgdbg sosal_shm omaprpc gfxdisp anw sf"
export TARGET_TEST="dvp_demo dvp_dsplib dvp_simple dvp_test dvp_dein dvp_share dvp_crash_test dvp_info v4l2_test vcam_test vcam_simple vcam_server sosal_test imgfilter_test uinput_test gfxd_test modload_test dvp_unittest"
else
export TARGET_SLIB="cthreaded VisionEngine pyuv sosal vcam v4l2 imgdbg sosal_shm"
export TARGET_TEST="dvp_demo dvp_dsplib dvp_simple dvp_test dvp_dein dvp_share dvp_crash_test dvp_info v4l2_test vcam_test vcam_simple vcam_server sosal_test imgfilter_test uinput_test modload_test dvp_unittest"
fi
export TARGET_BINS="shm_service"
export TARGET_APPS=""
export TARGET_PKGS=""
export TARGET_SRVC=""
export TARGET_SPKG=""

while [ $# -gt 0 ];
do
    if [ "${1}" == "usbsetup" ]; then
        sudo mkdir -p /root/.android
        sudo echo "0x0451" > /root/.android/adb_usb.ini
        echo Be sure to enable USB Debugging on the Device and to enable usbfs
        echo if you are running versions of Ubuntu previous to 10.10.
    fi
    if [ "${1}" == "setup" ]; then
        adb kill-server
        killall adb
        adb start-server
        adb get-state
        adb devices
        get_devices
        echo "TARGET_DEVICE=${TARGET_DEVICE}"
        if [ -n "${TARGET_DEVICE}" ] && [ "${TARGET_ANDROID}" == "ICS" ] || [ "${TARGET_ANDROID}" == "JB" ]; then
            adb -s ${TARGET_DEVICE} root
            sleep 2
            adb -s ${TARGET_DEVICE} remount
        fi
    fi
    if [ "${1}" == "perms" ]; then
        cd ${TARGET_ROOT}
        # mark all directories as drwxr-xr-x
        find . -type d -exec chmod 755 {} \;
        # mark all files as rw-r--r--
        find . -type f -exec chmod 644 {} \;
        # mark all executable files (on PC) as rwxr-xr-x
        chmod 755 bin/*
        find . -type f -name "*\.bat" -exec chmod 755 {} \;
        find . -type f -name "*\.pl" -exec chmod 755 {} \;
        find . -type f -name "*\.x*" -exec chmod 755 {} \;
        find . -type f -name "*\.sh" -exec chmod 755 {} \;
        find . -type f -name "*\.out" -exec chmod 755 {} \;
        find . -type f -name "*\.rc" -exec chmod 644 {} \;
        find . -type f -name "*\.so" -exec chmod 755 {} \;
        cd ${MYDROID}
    fi
    if [ "${1}" == "unix" ]; then
        cd ${TARGET_ROOT}
        FILES=`find . -type f \( -name *.c -o -name *.cpp -o -name *.h -o -name *.java -o -name *.aidl -o -name *.xml \) -print`
        for f in ${FILES}; do
            fromdos -v -p -o $f
        done
        cd ${MYDROID}
    fi
    if [ "${1}" == "clean" ]; then
        for i in ${TARGET_SLIB}; do
            echo Removing Static Library $i in ${_PRODUCT_}
            rm -rf ${ANDROID_PRODUCT_OUT}/obj/STATIC_LIBRARIES/lib${i}_intermediates/
        done
        for i in ${TARGET_LIBS}; do
            echo Removing Shared Library $i in ${_PRODUCT_}
            rm -rf ${ANDROID_PRODUCT_OUT}/obj/SHARED_LIBRARIES/lib${i}_intermediates/
            rm -rf ${ANDROID_PRODUCT_OUT}/symbols/system/lib/lib${i}.so
            rm -rf ${ANDROID_PRODUCT_OUT}/system/lib/lib${i}.so
        done
        for i in ${TARGET_BINS}; do
            echo Removing Binary $i in ${_PRODUCT_}
            rm -rf ${ANDROID_PRODUCT_OUT}/obj/EXECUTABLES/${i}_intermediates/
            rm -rf ${ANDROID_PRODUCT_OUT}/symbols/system/bin/${i}
            rm -rf ${ANDROID_PRODUCT_OUT}/system/bin/${i}
        done
        for i in ${TARGET_TEST}; do
            echo Removing Binary $i in ${_PRODUCT_}
            rm -rf ${ANDROID_PRODUCT_OUT}/obj/EXECUTABLES/${i}_intermediates/
            rm -rf ${ANDROID_PRODUCT_OUT}/symbols/system/bin/${i}
            rm -rf ${ANDROID_PRODUCT_OUT}/system/bin/${i}
        done
        for i in ${TARGET_APPS}; do
            echo Removing Android App $i in ${_PRODUCT_}
            rm -rf out/target/common/obj/APPS/${i}_intermediates/
            rm -rf ${ANDROID_PRODUCT_OUT}/obj/APPS/${i}_intermediates/
        done
        for i in ${TARGET_SPKG}; do
            echo Removing Android Package $i in ${_PRODUCT_}
            rm -rf out/target/common/obj/JAVA_LIBRARIES/${i}_intermediates/
            rm -rf ${ANDROID_PRODUCT_OUT}/obj/JAVA_LIBRARIES/${i}_intermediates/
        done
        for i in ${TARGET_SRVC}; do
            echo Removing Android Service $i in ${_PRODUCT_}
            rm -rf out/target/common/obj/APPS/${i}_intermediates/
            rm -rf ${ANDROID_PRODUCT_OUT}/obj/APPS/${i}_intermediates/
        done
    fi
    if [ "${1}" == "remoteclean" ]; then
        cd ${MYDROID}
        get_devices
        for i in ${TARGET_BINS}; do
            adb -s ${TARGET_DEVICE} shell "rm /system/bin/${i}"
        done
        for i in ${TARGET_TEST}; do
            adb -s ${TARGET_DEVICE} shell "rm /system/bin/${i}"
        done
        for i in ${TARGET_LIBS}; do
            adb -s ${TARGET_DEVICE} shell "rm /system/lib/lib${i}.so"
        done
        for i in ${TARGET_PKGS}; do
            adb -s ${TARGET_DEVICE} uninstall $i
            adb -s ${TARGET_DEVICE} shell "rm -rf /data/data/${i}"
        done
        for i in ${TARGET_APPS}; do
            adb -s ${TARGET_DEVICE} shell "rm -rf /system/apps/${i}.apk"
        done
        for i in ${TARGET_SRVC}; do
            adb -s ${TARGET_DEVICE} uninstall $i
        done
        for i in ${TARGET_SPKG}; do
            adb -s ${TARGET_DEVICE} uninstall $i
        done
        adb -s ${TARGET_DEVICE} shell "rm /data/raw/*"
        adb -s ${TARGET_DEVICE} shell "sync"
    fi
    if [ "${1}" == "install" ]; then
        get_devices
        adb -s ${TARGET_DEVICE} remount
        for i in ${TARGET_BINS}; do
            echo Installing ${i}
            adb -s ${TARGET_DEVICE} push ${ANDROID_PRODUCT_OUT}/system/bin/${i} /system/bin/
            #adb -s ${TARGET_DEVICE} shell "chmod 755 /system/bin/${i}"
            #adb -s ${TARGET_DEVICE} shell "chown root /system/bin/${i}"
        done
        for i in ${TARGET_LIBS}; do
            echo Installing lib${i}.so
            adb -s ${TARGET_DEVICE} push ${ANDROID_PRODUCT_OUT}/system/lib/lib${i}.so /system/lib/
        done
        adb -s ${TARGET_DEVICE} shell "mkdir -p /sdcard/raw"
        adb -s ${TARGET_DEVICE} shell "sync"
    fi
    if [ "${1}" == "package" ]; then
        export TARGET_OUT=${TARGET_ROOT}/out/ANDROID/ARM/
        mkdir -p ${TARGET_OUT}
        for i in ${TARGET_BINS}; do
            mkdir -p ${TARGET_OUT}/system/bin/
            cp -Rfp ${ANDROID_PRODUCT_OUT}/system/bin/${i} ${TARGET_OUT}/system/bin
        done
        for i in ${TARGET_TEST}; do
            mkdir -p ${TARGET_OUT}/system/bin/
            cp -Rfp ${ANDROID_PRODUCT_OUT}/system/bin/${i} ${TARGET_OUT}/system/bin
        done
        for i in ${TARGET_LIBS}; do
            mkdir -p ${TARGET_OUT}/system/lib/
            cp -Rfp ${ANDROID_PRODUCT_OUT}/system/lib/lib${i}.so ${TARGET_OUT}/system/lib
        done
        cd ${TARGET_OUT}
        tar zcvf ${TARGET_PROJ}.tar.gz system/
        cd ${MYDROID}
    fi
    if [ "${1}" == "test" ]; then
        get_devices
        adb -s ${TARGET_DEVICE} remount
        for i in ${TARGET_TEST}; do
            echo Installing ${i}
            adb -s ${TARGET_DEVICE} push ${ANDROID_PRODUCT_OUT}/system/bin/${i} /system/bin/
        done
        adb -s ${TARGET_DEVICE} shell "sync"
    fi
    if [ "${1}" == "mm" ]; then
        cd ${MYDROID}
        source build/envsetup.sh
        cd ${TARGET_ROOT}
        if [ "${_PRODUCT_}" == "GT-I9100G" ]; then
            mm -j$CPUS PRODUCT-${_PRODUCT_}-eng
        else
            mm -j$CPUS
        fi
        cd ${MYDROID}
    fi
    if [ "${1}" == "apps" ]; then
        get_devices
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
    fi
    if [ "${1}" == "omxdeps" ]; then
        if [ "${TARGET_ANDROID}" == "GB" ]; then
            cd ${MYDROID}
            get_devices
            adb -s ${TARGET_DEVICE} remount
            make_install_deplibs libOMX_Core \
                                 libcamera \
                                 libomx_rpc \
                                 libomx_proxy_common \
                                 libOMX.TI.DUCATI1.VIDEO.CAMERA \
                                 libOMX.TI.DUCATI1.IMAGE.JPEGD
        fi
    fi
    if [ "${1}" == "syslinkdeps" ]; then
        if [ "${TARGET_ANDROID}" == "GB" ]; then
            cd ${MYDROID}
            get_devices
            adb -s ${TARGET_DEVICE} remount
            make_install_deplibs libipc libipcutils libsysmgr librcm libnotify
        fi
    fi
    if [ "${1}" == "alldeps" ]; then
        if [ "${TARGET_ANDROID}" == "GB" ]; then
            cd ${MYDROID}
            get_devices
            adb -s ${TARGET_DEVICE} remount
            make_install_deplibs libipc \
                                 libipcutils \
                                 libsysmgr \
                                 libmemmgr \
                                 librcm \
                                 libOMX_Core \
                                 libcamera \
                                 libOMX.TI.DUCATI1.VIDEO.CAMERA \
                                 libOMX.TI.DUCATI1.IMAGE.JPEGD \
                                 libVendor_ti_omx
        fi
    fi
    if [ "${1}" == "vars" ]; then
        env | grep DVP
        env | grep VLIB
        env | grep RVM
        env | grep OMX
        env | grep VCAM
        env | grep TARGET
    fi
    if [ "${1}" == "patches" ]; then
        if [ "${TARGET_ANDROID}" == "GB" ]; then
            cd ${MYDROID}/${TI_HW_ROOT}/omx
            git checkout -b DB482
            git checkout -b DB482-NUI
            git am < ${TARGET_ROOT}/patches/GB/domx/0001-Added-single-preview-mode-type.patch
            git am < ${TARGET_ROOT}/patches/GB/domx/0002-Added-Gesture-related-OMX-interface-changes.patch
            git am < ${TARGET_ROOT}/patches/GB/domx/0003-OMX-OMX-Cap-structure-extended-with-the-3D-capabilit.patch
            git am < ${TARGET_ROOT}/patches/GB/domx/0004-Malloc-Fix.patch
            git am < ${TARGET_ROOT}/patches/GB/domx/0005-ZSL-OMX-Interface.patch
            git am < ${TARGET_ROOT}/patches/GB/domx/0006-core-ti-Restore-OMX_TI_CONFIG_SHAREDBUFFER.patch
            git am < ${TARGET_ROOT}/patches/GB/domx/0007-OMX-OMX-changes-for-stereo-gesture-usecase.patch
            cd ${MYDROID}
        fi
        if [ "${TARGET_ANDROID}" == "ICS" ] || [ "${TARGET_ANDROID}" == "JB" ]; then
            # TUNA and MAGURO don't need to be patched for eCameraView
            if [ "${_PRODUCT_}" == "maguro" ] || [ "${_PRODUCT_}" == "tuna" ]; then
                cd ${MYDROID}/${TI_HW_ROOT}/omap4xxx/domx/omx_core
            else
                cd ${MYDROID}/${TI_HW_ROOT}/domx
            fi
            git am ${TARGET_ROOT}/patches/ICS/android/domx/*.patch
            cd ${MYDROID}/device/ti/blaze_tablet
            git am ${TARGET_ROOT}/patches/ICS/android/0001-Tablet-Add-startup-of-shm-and-ARX-services-in-init.patch
            cd ${MYDROID}/device/ti/blaze
            git am ${TARGET_ROOT}/patches/ICS/android/0001-Blaze-Add-startup-of-shm-and-ARX-services-in-init.patch
            cd ${MYDROID}
        fi
    fi

    if [ "${1}" == "kill" ]; then
        adb -s ${TARGET_DEVICE} kill-server
    fi
    if [ "${1}" == "logcat" ]; then
        adb -s ${TARGET_DEVICE} logcat *:D >> ${MYDROID}/logcat.log &
        echo "To see the logcat data, do a $ tail -f logcat.log"
    fi
    if [ "${1}" == "trace" ]; then
        adb -s ${TARGET_DEVICE} shell "syslink_trace_daemon.out" >> ${MYDROID}/syslink_trace.log &
        echo "To see the SYSLINK trace, do a $ tail -f syslink_trace.log"
    fi
    if [ "${1}" == "shell" ]; then
        get_devices
        if [ -z "${2}" ]; then
            adb -s ${TARGET_DEVICE} shell
        else
            shift # eat the $1
            echo "Executing ${*}"
            adb -s ${TARGET_DEVICE} shell "${*}" &
        fi
    fi
    if [ "${1}" == "baseimage" ]; then
        cd ${MYDROID}
        get_devices
        adb -s ${TARGET_DEVICE} root
        sleep 2
        adb -s ${TARGET_DEVICE} remount
        SUFFIX=""
        if [ "${TARGET_ANDROID}" == "JB" ]; then
            SUFFIX="-jb"
        fi
        if [ "${TARGET_ANDROID}" == "GB" ]; then
            echo "Loading Ducati Base Image"
            adb -s ${TARGET_DEVICE} push ${TARGET_ROOT}/prebuilt/base_image_app_m3.xem3 /system/vendor/firmware/.
            adb -s ${TARGET_DEVICE} push ${TARGET_ROOT}/prebuilt/base_image_sys_m3.xem3 /system/vendor/firmware/.
            echo "Loading Tesla Base Image"
            adb -s ${TARGET_DEVICE} push ${TARGET_ROOT}/prebuilt/base_image_dsp.xe64T   /system/vendor/firmware/.
            echo "Loading syslink_daemon.out"
            adb -s ${TARGET_DEVICE} push ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/syslink_daemon.out     /system/bin/.
            adb -s ${TARGET_DEVICE} shell chmod 755 /system/bin/syslink_daemon.out
            echo "Loading libsysmgr.so"
            adb -s ${TARGET_DEVICE} push ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/libsysmgr.so     /system/lib/.
        elif [ "${TARGET_ANDROID}" == "ICS" ] || [ "${TARGET_ANDROID}" == "JB" ] ; then
            echo "Loading Ducati Base Image"
            if [ "${_PRODUCT_}" == "omap5sevm" ]; then
                adb -s ${TARGET_DEVICE} push ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/ducati-m3-core0.xem3 /vendor/firmware/.
            else
                adb -s ${TARGET_DEVICE} push ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/ducati-m3${SUFFIX}.bin /vendor/firmware/ducati-m3.bin
            fi
            echo "Loading Tesla Base Image"
            adb -s ${TARGET_DEVICE} push ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/tesla-dsp${SUFFIX}.bin /vendor/firmware/tesla-dsp.bin
        fi
        adb -s ${TARGET_DEVICE} shell "sync"
    fi
    if [ "${1}" == "dcc" ]; then
        cd ${MYDROID}
        get_devices
        adb -s ${TARGET_DEVICE} root
        sleep 2
        adb -s ${TARGET_DEVICE} remount
        echo "Using Ducati Default DCC Files"
        if [ "${TARGET_ANDROID}" == "GB" ]; then
            adb -s ${TARGET_DEVICE} shell "mv /etc/omapcam /etc/omapcam.bak"
        elif [ "${TARGET_ANDROID}" == "ICS" ] || [ "${TARGET_ANDROID}" == "JB" ]; then
            adb -s ${TARGET_DEVICE} shell "mv /data/misc/camera /data/misc/camera.bak"
        fi
        adb -s ${TARGET_DEVICE} shell "sync"
    fi
    if [ "${1}" == "docs" ]; then
        export PUBLIC_LIB_HEADER_LIST=" ${TARGET_ROOT}/libraries/public/yuv/include/yuv/dvp_kl_yuv.h
                                        ${TARGET_ROOT}/libraries/public/imgfilter/include/imgfilter/dvp_kl_imgfilter.h
                                        ${TARGET_ROOT}/libraries/public/ocl/include/ocl/dvp_kl_ocl.h
                                        ${TARGET_ROOT}/libraries/public/vrun/include/vrun/dvp_kl_vrun.h
                                        ${TARGET_ROOT}/libraries/public/dsplib/include/dsplib/dvp_kl_dsplib.h
                                        ${TARGET_ROOT}/libraries/public/dei/include/dei/dvp_kl_dei.h"
        export PRIVATE_LIB_HEADER_LIST="${MYDROID}/${VISION_ROOT}/libraries/protected/imglib/include/imglib/dvp_kl_imglib.h
                                        ${MYDROID}/${VISION_ROOT}/libraries/protected/vlib/include/vlib/dvp_kl_vlib.h
                                        ${MYDROID}/${VISION_ROOT}/libraries/protected/rvm/include/rvm/dvp_kl_rvm.h
                                        ${MYDROID}/${VISION_ROOT}/libraries/protected/tismo/include/tismo/dvp_kl_tismo.h
                                        ${MYDROID}/${VISION_ROOT}/libraries/protected/tismov02/include/tismov02/dvp_kl_tismov02.h"
        export LIBHEADERS=
        export LIBDOCS=
        export doxygen_status=`dpkg-query -W -f='${Status}\n' doxygen`
        export graphviz_status=`dpkg-query -W -f='${Status}\n' graphviz`
        export mscgen_status=`dpkg-query -W -f='${Status}\n' mscgen`
        if [ "$doxygen_status"  == "install ok installed" ] &&
           [ "$graphviz_status" == "install ok installed" ] &&
           [ "$mscgen_status"   == "install ok installed" ]; then
            pushd ${TARGET_ROOT}
            export COMMIT=`git describe --tags --dirty`
            cp docs/dvp_sdk.txt docs/dvp_sdk.bak
            sed -i "s/VERSION/${COMMIT}/" docs/dvp_sdk.txt
            sed -i "s/DATE/`date`/" docs/dvp_sdk.txt
            if [ "${TARGET_ANDROID}" == "GB" ]; then
                export PREDEF="PREDEFINED += ANDROID GINGERBREAD DVP_USE_RCM"
            elif [ "${TARGET_ANDROID}" == "ICS" ]; then
                export PREDEF="PREDEFINED += ANDROID ICS DVP_USE_OMAPRPC"
            elif [ "${TARGET_ANDROID}" == "JB" ]; then
                export PREDEF="PREDEFINED += ANDROID JELLYBEAN DVP_USE_OMAPRPC"
            else
                export PREDEF="PREDEFINED += LINUX"
            fi

            # Only include the headers that exist from vision_prebuilt
            for i in ${PRIVATE_LIB_HEADER_LIST}; do
                if [ -f ${i} ]; then
                    LIBHEADERS+="${i} ";
                    echo "Library Header File Found: ${i}";
                    PREDEF+=" `echo ${i} | sed 's/.*dvp_kl_\([a-z0-9]*\).h/DVP_USE_\U\1/'` ";
                    export LIBDOC="`echo ${i} | sed 's/\.h$/\.txt/'` ";
                    if [ -f ${LIBDOC} ]; then
                        LIBDOCS+=${LIBDOC};
                    fi
                fi
            done

            # Only include the document files from libraries that exist in DVP repo
            for i in ${PUBLIC_LIB_HEADER_LIST}; do
                if [ -f ${i} ]; then
                    echo "Library Header File Found: ${i}";
                    export LIBDOC="`echo ${i} | sed 's/\.h$/\.txt/'` ";
                    if [ -f ${LIBDOC} ]; then
                        LIBDOCS+=${LIBDOC};
                    fi
                fi
            done
            scripts/kernel_doc.pl docs/dvp_kernel_autogen.txt ${TARGET_ROOT}/include/dvp/dvp_types.h ${LIBHEADERS} ${PUBLIC_LIB_HEADER_LIST}
            (cat docs/full/Doxyfile ; echo "PROJECT_NUMBER = \"DVP project git tag: \" $COMMIT" ; echo $PREDEF; \
                                      echo "INPUT+= ${LIBHEADERS} ${LIBDOCS}") | doxygen -
            mv docs/dvp_sdk.bak docs/dvp_sdk.txt
            popd
            echo "Look in ${TARGET_ROOT}/docs/ for the generated documentation"
        else
            echo "A required package is not installed on your machine!"
            echo "To generate ${TARGET_PROJ} docs, install the package (sudo apt-get install <missing package name>) and re-run this option."
        fi
    fi
    if [ "${1}" == "ramdisk" ]; then
        if [ "${EMMC_DIR}" == "" ]; then
            echo "You must set EMMC_DIR variable first."
        else
            if [ -d "${EMMC_DIR}" ]; then
                cd ${EMMC_DIR}

                mkdir root
                cd root/
                gunzip -c ../ramdisk.img | cpio -i
                cd ../

                SUFFIX=""
                if [ "${TARGET_ANDROID}" == "JB" ]; then
                    SUFFIX="-jb"
                fi
                echo "Replacing the original init.rc script with our modified one."
                if [ "${TARGET_ANDROID}" == "ICS" ] || [ "${TARGET_ANDROID}" == "JB" ] ; then
                    if [ "${_PRODUCT_}" == "maguro" ] || [ "${_PRODUCT_}" == "tuna" ]; then
                        cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/init.tuna.rc ${EMMC_DIR}/root/.
                        cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/init.rc ${EMMC_DIR}/root/.
                    elif [ "${_PRODUCT_}" == "omap5sevm" ]; then
                        cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/ueventd.omap5sevmboard.rc ${EMMC_DIR}/root/.
                        cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/init.omap5sevmboard.rc ${EMMC_DIR}/root/.
                    else
                        cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/init.omap4blazeboard${SUFFIX}.rc ${EMMC_DIR}/root/init.omap4blazeboard.rc
                        cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/ueventd.omap4blazeboard.rc ${EMMC_DIR}/root/.
                    fi
                else
                    cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/init.omap4430.rc ${EMMC_DIR}/root/.
                    cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/kos/* ${EMMC_DIR}/root/.
                fi
                ${MYDROID}/out/host/linux-x86/bin/mkbootfs root/ | ${MYDROID}/out/host/linux-x86/bin/minigzip >ramdisk.img
                if [ "${TARGET_ANDROID}" == "ICS" ] || [ "${TARGET_ANDROID}" == "JB" ] ; then
                    echo "Replacing our zImage to enable OMAPRPC."
                    cp -fv ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/zImage${SUFFIX} ${EMMC_DIR}/zImage
                    #./umulti2.sh
                    ./mkbootimg --kernel zImage --ramdisk ramdisk.img --base 0x80000000 --cmdline "" --board omap4 -o boot.img
                else
                    echo "Replacing our zImage to enable Tesla."
                    cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/zImage_sl2_ducati_debug_gadget ${EMMC_DIR}/zImage
                    cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/umulti.sh ${EMMC_DIR}/umulti.sh
                    ./umulti.sh
                fi

                rm -R root

                if [ "${_PRODUCT_}" != "omap5sevm" ]; then
                    echo "Reboot the device into fastboot mode"
                    sudo ./fastboot flash boot ./boot.img
                    echo "Rebooting Fastboot"
                    sudo ./fastboot reboot
                fi
            else
                echo "EMMC_DIR variable does not point to a valid directory."
            fi
        fi
    fi
    if [ "${1}" == "emmc" ]; then
        if [ "${EMMC_DIR}" == "" ]; then
            echo "You must set EMMC_DIR variable first."
        else
            if [ -d "${EMMC_DIR}" ]; then

                # Modifying boot.img
                echo "Modifying ${EMMC_DIR}"
                cd ${EMMC_DIR}
                mkdir root
                cd root/
                gunzip -c ../ramdisk.img | cpio -i
                cd ../

                SUFFIX=""
                if [ "${TARGET_ANDROID}" == "JB" ]; then
                    SUFFIX="-jb"
                fi

                echo "Replacing the original init.rc script with our modified one."
                if [ "${TARGET_ANDROID}" == "ICS" ] || [ "${TARGET_ANDROID}" == "JB" ]; then
                    if [ "${_PRODUCT_}" == "maguro" ] || [ "${_PRODUCT_}" == "tuna" ]; then
                        echo "target product is maguro (tuna)"
                        cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/init.tuna.rc ${EMMC_DIR}/root/.
                        cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/init.rc ${EMMC_DIR}/root/.
                    else
                        if [ "${_PRODUCT_}" == "omap5sevm" ]; then
                            cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/ueventd.omap5sevmboard.rc ${EMMC_DIR}/root/.
                            cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/init.omap5sevmboard.rc ${EMMC_DIR}/root/.
                        else
                            cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/init.omap4blazeboard.rc ${EMMC_DIR}/root/.
                        fi
                    fi
                else
                    cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/kos/* ${EMMC_DIR}/root/.
                    cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/init.omap4430.rc ${EMMC_DIR}/root/.
                fi
                ${MYDROID}/out/host/linux-x86/bin/mkbootfs root/ | ${MYDROID}/out/host/linux-x86/bin/minigzip >ramdisk.img
                if [ "${TARGET_ANDROID}" == "ICS" ] || [ "${TARGET_ANDROID}" == "JB" ] ; then
                    echo "Replacing our zImage to enable OMAPRPC."
                    cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/zImage${SUFFIX} ${EMMC_DIR}/zImage
                    #./umulti2.sh
                    ./mkbootimg --kernel zImage --ramdisk ramdisk.img --base 0x80000000 --cmdline "" --board omap4 -o boot.img
                else
                    echo "Replacing our zImage to enable Tesla."
                    cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/zImage_sl2_ducati_debug_gadget ${EMMC_DIR}/zImage
                    cp -f ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/umulti.sh ${EMMC_DIR}/umulti.sh
                    ./umulti.sh
                fi
                rm -R root

                # Extracting system partitions
                ./simg2img system.img system.img.raw
                mkdir tmp
                sudo mount -t ext4 -o loop system.img.raw tmp/
                sudo chmod -R a+w tmp

                # From "install"
                for i in $TARGET_BINS; do
                    echo Copying ${i}
                    cp ${ANDROID_PRODUCT_OUT}/system/bin/${i} tmp/bin/
                done
                for i in $TARGET_LIBS; do
                    echo Copying lib${i}.so
                    cp ${ANDROID_PRODUCT_OUT}/system/lib/lib${i}.so tmp/lib/
                done

                # From "test"
                for i in $TARGET_TEST; do
                    echo Copying ${i}
                    cp ${ANDROID_PRODUCT_OUT}/system/bin/${i} tmp/bin/
                done
                # From "baseimage"

                if [ "${TARGET_ANDROID}" == "ICS" ] || [ "${TARGET_ANDROID}" == "JB" ] ; then
                    echo "Copying Ducati and Tesla images"
                    cp ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/ducati-m3${SUFFIX}.bin tmp/vendor/firmware/ducati-m3.bin
                    cp ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/tesla-dsp${SUFFIX}.bin tmp/vendor/firmware/tesla-dsp.bin
                else
                    echo "Loading Ducati Base Image"
                    cp ${TARGET_ROOT}/prebuilt/base_image_app_m3.xem3 tmp/vendor/firmware/
                    cp ${TARGET_ROOT}/prebuilt/base_image_sys_m3.xem3 tmp/vendor/firmware/
                    echo "Loading Tesla Base Image"
                    cp ${TARGET_ROOT}/prebuilt/base_image_dsp.xe64T   tmp/vendor/firmware/
                    echo "Loading syslink_daemon.out"
                    cp ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/syslink_daemon.out  tmp/bin/
                    sudo chmod 755 tmp/bin/syslink_daemon.out
                    echo "Loading libsysmgr.so"
                    cp ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/libsysmgr.so     tmp/lib/
                fi
                sudo ./make_ext4fs -s -l 512M -a system system.img tmp/
                sudo umount tmp
                rm -rf tmp
                rm -rf system.img.raw

                # Extracting data partition for DCC file change
                ./simg2img userdata.img userdata.img.raw
                mkdir tmpData
                sudo mount -t ext4 -o loop userdata.img.raw tmpData/
                sudo chmod -R a+w tmpData

                if [ "${TARGET_ANDROID}" == "GB" ]; then
                    echo Gingerbread DCC file not needed to be changed.
                    #mv /etc/omapcam /etc/omapcam.bak
                elif [ "${TARGET_ANDROID}" == "ICS" ] || [ "${TARGET_ANDROID}" == "JB" ]; then
                    mv tmpData/misc/camera tmpData/misc/camera.bak
                fi

                sudo ./make_ext4fs -s -l 512M -a userdata userdata.img tmpData/
                ${MYDROID}/out/host/linux-x86/bin/mkuserimg.sh -s tmpData userdata.img ext4 data 512M
                sudo umount tmpData
                rm -rf tmpData
                rm -rf userdata.img.raw
                if [ "${TARGET_ANDROID}" == "ICS" ] || [ "${TARGET_ANDROID}" == "JB" ]; then
                    echo "Copying xloader"
                    cp ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/*MLO ./
                fi
            else
                echo EMMC_DIR variable does not point to a valid directory.
            fi
        fi
    fi
    if [ "${1}" == "debug" ]; then
        cd ${MYDROID}
        OBJOPT="-C -d -g -f -l -S --include=${TARGET_ROOT}"
        echo Outputing Obj-Dump info on ${TARGET_PROJ} Objects
        source build/envsetup.sh
        setpaths
        mkdir ${TARGET_ROOT}/out
        # System Objects
        #arm-eabi-objdump ${OBJOPT} ${ANDROID_PRODUCT_OUT}/symbols/system/bin/gdbserver > ${TARGET_ROOT}/out/gdbserver.txt
        arm-eabi-objdump ${OBJOPT} ${ANDROID_PRODUCT_OUT}/symbols/system/lib/libc.so > ${TARGET_ROOT}/out/libc.txt
        arm-eabi-objdump ${OBJOPT} ${ANDROID_PRODUCT_OUT}/symbols/system/lib/libOMX.TI.DUCATI1.VIDEO.CAMERA.so > ${TARGET_ROOT}/out/libOMX.TI.DUCATI1.VIDEO.CAMERA.txt
        #arm-eabi-objdump ${OBJOPT} ${ANDROID_PRODUCT_OUT}/symbols/system/lib/libopencore_common.so > ${TARGET_ROOT}/out/libopencore_common.txt
        # Unit Tests
        for i in ${TARGET_BINS}; do
            arm-eabi-objdump ${OBJOPT} ${ANDROID_PRODUCT_OUT}/symbols/system/bin/${i} > ${TARGET_ROOT}/out/${i}.txt
        done
        for i in ${TARGET_TEST}; do
            arm-eabi-objdump ${OBJOPT} ${ANDROID_PRODUCT_OUT}/symbols/system/bin/${i} > ${TARGET_ROOT}/out/${i}.txt
        done
        for i in ${TARGET_LIBS}; do
            arm-eabi-objdump ${OBJOPT} ${ANDROID_PRODUCT_OUT}/symbols/system/lib/lib${i}.so > ${TARGET_ROOT}/out/lib${i}.txt
        done
    fi
    if [ "${1}" == "tombstones" ]; then
        get_devices
        rm ${TARGET_ROOT}/out/tombstone_*
        TOMBSTONES=`adb -s ${TARGET_DEVICE} shell "ls /data/tombstones" | tr '\r' ' ' | tr '\n' ' '`
        echo ${TOMBSTONES}
        for file in ${TOMBSTONES}
        do
            echo "Copying file ${file} to PC"
            adb -s ${TARGET_DEVICE} pull /data/tombstones/${file} ${TARGET_ROOT}/out/
        done
        adb -s ${TARGET_DEVICE} shell "rm /data/tombstones/*"
    fi
    if [ "${1}" == "perf" ]; then
        get_devices
        adb -s ${TARGET_DEVICE} shell "echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
    fi
    if [ "${1}" == "videos" ]; then
        get_devices
        LISTING=`adb -s ${TARGET_DEVICE} shell "ls /sdcard/raw/" | grep "^[0-9][0-9]" | tr '\r' ' ' | tr '\n' ' '`
        mkdir -p ${TARGET_ROOT}/raw/output/
        for file in ${LISTING}
        do
            echo "Copying ${file} from device to ${TARGET_ROOT}/raw/output/"
            adb -s ${TARGET_DEVICE} pull /sdcard/raw/${file} ${TARGET_ROOT}/raw/output/
            #adb -s ${TARGET_DEVICE} shell rm -rf /sdcard/raw/${file}
        done
    fi
    if [ "${1}" == "convert" ]; then
        LISTING=`ls ${TARGET_ROOT}/raw/output | grep "^[0-9][0-9]_bitimg" | tr '\r' ' ' | tr '\n' ' '`
        for file in ${LISTING}
        do
            echo "Converting ${file}"
            modfile=`echo ${file} | sed "s/bitimg/img/"`
            ${TARGET_ROOT}/bin/bitimg2img ${TARGET_ROOT}/raw/output/${file} ${TARGET_ROOT}/raw/output/$modfile
        done
    fi
    if [ "${1}" == "unitest" ]; then
        adb -s ${TARGET_DEVICE} shell "sosal_test"
        adb -s ${TARGET_DEVICE} shell "dvp_test"
    fi
    if [ "${1}" == "gdb" ] || [ "${1}" == "ddd" ]; then
        get_devices
        TYPE=${1}
        if [ "${TARGET_ANDROID}" == "GB" ]; then
            adb -s ${TARGET_DEVICE} push ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/gdbserver /system/bin/
            adb -s ${TARGET_DEVICE} shell "chmod 777 /system/bin/gdbserver"
        fi
        PROCESS=${2}
        shift # Remove "gdb"
        shift # Remove "process"
        adb -s ${TARGET_DEVICE} forward tcp:5039 tcp:5039
        adb -s ${TARGET_DEVICE} shell "cd /sdcard && gdbserver :5039 /system/bin/${PROCESS} $*" &
        cat > gdb_cmds.txt << EOF
set solib-absolute-prefix ${ANDROID_PRODUCT_OUT}/symbols
set solib-search-path ${ANDROID_PRODUCT_OUT}/symbols/system/lib
target remote ${ADBHOST}:5039
EOF
        if [ "${TYPE}" == "gdb" ]; then
            arm-eabi-gdb ${ANDROID_PRODUCT_OUT}/symbols/system/bin/${PROCESS} --command=gdb_cmds.txt
        fi
        if [ "${TYPE}" == "ddd" ]; then
            ddd --debugger arm-eabi-gdb -x gdb_cmds.txt ${ANDROID_PRODUCT_OUT}/symbols/system/bin/${PROCESS} &
        fi
        #rm gdb_cmds.txt
        exit
    fi
    if [ "${1}" == "attach" ]; then
        if [ "${TARGET_ANDROID}" == "GB" ]; then
            adb -s ${TARGET_DEVICE} push ${TARGET_ROOT}/prebuilt/ANDROID/${TARGET_PLATFORM}/gdbserver /system/bin/
            adb -s ${TARGET_DEVICE} shell "chmod 777 /system/bin/gdbserver"
        fi
        PROCESS=${2}
        adb -s ${TARGET_DEVICE} forward tcp:5039 tcp:5039
        PID=`adb -s ${TARGET_DEVICE} shell "ps" | grep ${PROCESS} | awk '{print $2}'`
        echo "Connecting to ${PROCESS} PID ${PID}"
        adb -s ${TARGET_DEVICE} shell "gdbserver --attach :5039 ${PID}" &
        cat > gdb_cmds.txt << EOF
set solib-absolute-prefix ${ANDROID_PRODUCT_OUT}/symbols
set solib-search-path ${ANDROID_PRODUCT_OUT}/symbols/system/lib
target remote ${ADBHOST}:5039
EOF
        ddd --debugger arm-eabi-gdb -x gdb_cmds.txt ${ANDROID_PRODUCT_OUT}/symbols/system/bin/${PROCESS} &
    fi
    if [ "${1}" == "todo" ]; then
        cd ${MYDROID}
        source build/envsetup.sh
        cd ${TARGET_ROOT}
        cgrep TODO
        cd ${MYDROID}
    fi
    if [ "${1}" == "qdisp" ]; then
        adb -s ${TARGET_DEVICE} push ${TARGET_ROOT}/qdisp.sh /bin && adb -s ${TARGET_DEVICE} shell "chmod 755 /bin/qdisp.sh && qdisp.sh"
    fi
    if [ "${1}" == "vcam_server" ]; then
        if [ "${2}" == "local" ]; then
            adb -s ${TARGET_DEVICE} shell vcam_server &
            adb -s ${TARGET_DEVICE} shell "vcam_simple -t 3 -w 320 -h 240 -# 300 -s 1"
        else
            adb -s ${TARGET_DEVICE} forward tcp:8501 tcp:8501
            adb -s ${TARGET_DEVICE} forward tcp:8502 tcp:8502
            adb -s ${TARGET_DEVICE} shell vcam_server &
        fi
    fi
    if [ "${1}" == "zones" ] || [ "${1}" == "zone" ]; then
        if [ -n "${2}" ]; then
            zone_mask ${2}
            shift
        fi
        zone_mask print
    fi
    if [ "${1}" == "init" ]; then
        if [ "${TARGET_ANDROID}" == "ICS" ] || [ "${TARGET_ANDROID}" == "JB" ]; then
            if ["${_PRODUCT_}" == "omap5sevm"]; then
                INIT_SCRIPT=${MYDROID}/device/ti/${_PRODUCT_}/init.omap5sevmboard.rc
            else
                INIT_SCRIPT=${MYDROID}/device/ti/${_PRODUCT_}/init.omap4blazeboard.rc
            fi
        else
            INIT_SCRIPT=${MYDROID}/device/ti/${_PRODUCT_}/init.omap4430.rc
        fi
        TEXT=`cat $INIT_SCRIPT | grep shm_service`
        if [ -z "$TEXT" ]; then
            cat >> $INIT_SCRIPT << EOF
# Texas Instruments Shared Memory Service
service shm_service /system/bin/shm_service --no-fork
    class core
    user root
    group root
EOF
        fi
        if [ -n "$TEXT" ]; then
            echo "$INIT_SCRIPT already has entry for shm_service"
        fi
    fi
    if [ "${1}" == "fastboot" ]; then
        echo "Rebooting ${TARGET_DEVICE} into fastboot"
        adb -s ${TARGET_DEVICE} reboot bootloader
    fi
    if [ "${1}" == "reboot" ]; then
        echo "Rebooting ${TARGET_DEVICE}"
        adb -s ${TARGET_DEVICE} reboot
    fi
    if [ "${1}" == "wait" ]; then
        echo "Waiting for ${TARGET_DEVICE} to connect to ADB"
        adb -s ${TARGET_DEVICE} wait-for-device
    fi
    if [ "${1}" == "ndk" ]; then
        OUTDIR=${2}
        if [ -z "${OUTDIR}" ]; then
            OUTDIR=$MYDROID/../dvp_ndk/
        fi
        if [ ! -d "${OUTDIR}" ]; then
            mkdir -p ${OUTDIR}
        fi
        echo "Exporting DVP to NDK format in ${OUTDIR}"
        export INCDIRS="dvp sosal"
        for inc in ${INCDIRS}; do
            echo "Exporting Include Folder ${inc}"
            mkdir -p ${OUTDIR}/include/$inc
            cp -Rf ${DVP_ROOT}/include/$inc ${OUTDIR}/include/
        done
        # remove some files:
        rm -f ${OUTDIR}/include/dvp/Vision*.h
        rm -f ${OUTDIR}/include/dvp/Image*.h
        # Copy the outputs
        mkdir -p ${OUTDIR}/libs/armeabi/
        for lib in ${TARGET_LIBS}; do
            cp -f ${ANDROID_PRODUCT_OUT}/system/lib/lib$lib.so ${OUTDIR}/libs/armeabi/
        done
        #for lib in ${TARGET_SLIB}; do
            #cp -f ${ANDROID_PRODUCT_OUT}/system/lib/lib$lib.a ${OUTDIR}/libs/armeabi/
        #done
        # Copy the docs
        pushd ${DVP_ROOT}
        doxygen ${DVP_ROOT}/docs/ndk/Doxyfile
        mkdir -p ${OUTDIR}/docs/
        cp -Rf ${TARGET_ROOT}/docs/ndk/html/ ${OUTDIR}/docs/
        popd
    fi
    shift
done
