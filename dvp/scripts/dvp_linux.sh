#!/bin/bash

# Copyright (C) 2009-2012 Texas Instruments, Inc.
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


function get_symbols() {
    FILENAME=$1
    FILETYPE=$2
    OBJOPT="-C -d -g -f -l -S --include=${TARGET_ROOT}"
    echo "Processing filename"
    objdump ${OBJOPT} ${OUT_DIR}/${FILENAME} > ${OUT_DIR}/${FILENAME}.txt
}


if [ -z "${DVP_ROOT}" ]; then
    echo "You must set DVP_ROOT manually!"
    die 1
fi

# Make the TARGET_ env variables
cd ${DVP_ROOT}
LINES=$(make targets | grep TARGET_)
for line in ${LINES}; do
    echo ${line}
    export ${line}
done
cd -

export OUT_DIR=${DVP_ROOT}/out/${TARGET_OS}/${TARGET_CPU}
export CPUS=`cat /proc/cpuinfo | grep processor | wc -l`

export TARGET_PROJ=dvp
export TARGET_ROOT="${DVP_ROOT}"
export TARGET_LIBS="dvp dvp_kgm_cpu"
if [ "${TARGET_CPU}" == "ARM" ]; then
    export TARGET_LIBS="dvp dvp_kgm_cpu dvp_kgm_simcop dvp_kgm_dsp dvp_kgm_ocl omaprpc"
fi
export TARGET_SLIB="cthreaded VisionEngine pyuv sosal vcam v4l2 imgdbg vlib_${TARGET_CPU} rvm_${TARGET_CPU} yuv imglib_${TARGET_CPU} c6xsim"
export TARGET_BINS=""
export TARGET_TEST="dvp_demo dvp_simple dvp_test dein_test v4l2_test vcam_simple vcam_server sosal_test uinput_test"
export TARGET_APPS=""
export TARGET_PKGS=""
export TARGET_SRVC=""
export TARGET_SPKG=""
export OMX_INC=${OMX_ROOT}/omx_core/inc

if [ $# == 0 ]; then
    echo "Usage: $0 [clean|remoteclean|mm|vars|setup|install|test|images|videos|convert|baseimage|docs|gdb|ddd|run|]"
    echo ""
    echo " options: "
    echo "  clean       -  Deletes previously built libraries & intermediate files on the host machine"
    echo "  remoteclean -  Deletes previously built libraries & intermediate files on the target device"
    echo "  mm          -  Builds entire DVP project on the host machine"
    echo "  vars        -  Checks for your currently defined environment build variables relating to DVP."
    echo "  install     -  Installs successfully-built libraries and applications. Requires active network connection or SD card if on remote target!"
    echo "  test        -  Installs successfully-built test binaries. Requires active network connection or SD card if on remote target!"
    echo "                 -- You might want to run the 'DVP.sh setup' option first --"
    echo "  setup       -  Adds ssh public key to device.  Requires 'TARGET_IP' variable to be defined! (e.g. TARGET_IP=125.247.xx.yy)"
    echo "  images      -  Uploads input video files from host machine to root/raw/ on device"
    echo "  videos      -  Downloads processed video files from target device to DVP's root folder/raw/output on host machine"
    echo "  convert     -  Converts the downloaded video files that are binary images to images that can be viewed on pyuv player"
    echo "  run         -  Runs the DVP framework in various standalone test modes."
    echo "  baseimage   -  Installs the Ducati / Tesla binaries on the target device and reboot the target device."
    echo ""
    exit
fi

echo "Target platform is ${TARGET_PLATFORM}"

if [ "${TARGET_PLATFORM}" != "PANDA" ]; then
    export SUDO=sudo
else
    export SUDO=
fi
echo "SUDO is : ${SUDO}"

while [ $# -gt 0 ];
do
    if [ "${1}" == "vars" ]; then
        env | grep DVP
        env | grep VISION
        env | grep TARGET_IP
        env | grep ROOTFS_DIR
    fi
    if [ "${1}" == "setup" ]; then
        if [ "${TARGET_CPU}" == "ARM" ]; then
            if [ ! -f "${HOME}/.ssh/id_rsa.pub" ]; then
                echo "ERROR: SSH public key NOT found"
                exit 0
            fi
            echo "Copying local SSH RSA public key to Panda"
            ssh root@${TARGET_IP} "mkdir -p /root/.ssh"
            ssh root@${TARGET_IP} "chmod 644 /root/.ssh"
            scp ${HOME}/.ssh/id_rsa.pub root@${TARGET_IP}:/root/.ssh/authorized_keys
            exit 0
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
        chmod 755 scripts/*.sh
        chmod 755 scripts/*.bat
        chmod 755 prebuilt/*.x*
    fi
    if [ "${1}" == "unix" ]; then
        cd ${TARGET_ROOT}
        FILES=`find . -type f \( -name *.c -o -name *.cpp -o -name *.h -o -name *.java -o -name *.aidl -o -name *.xml -o -name *.mak \) -print`
        for f in ${FILES}; do
            fromdos -v -p -o $f
        done
        cd ${MYDROID}
    fi
    if [ "${1}" == "clean" ]; then
        rm -rf ${OUT_DIR}
    fi
    if [ "${1}" == "mm" ]; then
        cd ${TARGET_ROOT}
        make
    fi
    if [ "${1}" == "remoteclean" ]; then
        cd ${TARGET_ROOT}
        if [ "${TARGET_CPU}" == "ARM" ]; then
            if [ "${TARGET_IP}" != "" ]; then   # Network case
                for i in ${TARGET_BINS}; do
                    ssh root@${TARGET_IP} "rm /usr/bin/${i}"
                done
                for i in ${TARGET_TEST}; do
                    ssh root@${TARGET_IP} "rm /usr/bin/${i}"
                done
                for i in ${TARGET_LIBS}; do
                    ssh root@${TARGET_IP} "rm /usr/lib/lib${i}.so"
                done
                ssh root@${TARGET_IP} "rm /root/raw/*"
            elif [ "${ROOTFS_DIR}" != "" ]; then # SD Card case
                for i in ${TARGET_BINS}; do
                    ${SUDO} rm ${ROOTFS_DIR}/usr/bin/${i}
                done
                for i in ${TARGET_TEST}; do
                    ${SUDO} rm ${ROOTFS_DIR}/usr/bin/${i}
                done
                for i in ${TARGET_LIBS}; do
                    ${SUDO} rm ${ROOTFS_DIR}/usr/lib/lib${i}.so
                done
                ${SUDO} rm ${ROOTFS_DIR}/root/raw/*
            else
                echo "For network install: set TARGET_IP variable."
                echo "For SD card install: set ROOTFS_DIR variable."
                exit 0
            fi
        else    # PC Case
            # For running on PC, we MUST put the .so files in the /usr/lib folder because DVP is explicitly looking for
            #   dvp_kgm_* files to know what kgm's are available for loading.  Bins can safely remain in the OUT_DIR
            #   if called directly (or calling the dvp.sh run <bin> <args>).
            for i in ${TARGET_LIBS}; do
                ${SUDO} rm /usr/lib/lib${i}.so*
            done
        fi
    fi
    if [ "${1}" == "install" ]; then
        if [ "${TARGET_CPU}" == "ARM" ]; then
            if [ "${TARGET_IP}" != "" ]; then   # Network case
                cd ${TARGET_ROOT}
                for i in ${TARGET_BINS}; do
                    echo Installing ${i}
                    scp ${OUT_DIR}/${i} root@${TARGET_IP}:/usr/bin/
                done
                for i in ${TARGET_LIBS}; do
                    echo Installing lib${i}.so
                    scp ${OUT_DIR}/lib${i}.so root@${TARGET_IP}:/usr/lib/
                done
            elif [ "${ROOTFS_DIR}" != "" ]; then # SD Card case
                for i in ${TARGET_BINS}; do
                    echo Installing ${i}
                    ${SUDO} cp ${OUT_DIR}/${i} ${ROOTFS_DIR}/usr/bin/.
                done
                for i in ${TARGET_LIBS}; do
                    echo Installing lib${i}.so
                    ${SUDO} cp ${OUT_DIR}/lib${i}.so ${ROOTFS_DIR}/usr/lib/.
                done
                sync
            else
                echo "For network install: set TARGET_IP variable."
                echo "For SD card install: set ROOTFS_DIR variable."
                exit 0
            fi
        else    # PC Case
            # For running on PC, we MUST put the .so files in the /usr/lib folder because DVP is explicitly looking for
            #   dvp_kgm_* files to know what kgm's are available for loading.  Bins can safely remain in the OUT_DIR
            #   if called directly (or calling the dvp.sh run <bin> <args>).
            for i in ${TARGET_LIBS}; do
                echo Installing lib${i}.so
                ${SUDO} cp -d ${OUT_DIR}/lib${i}.so* /usr/lib/.
            done
        fi
    fi
    if [ "${1}" == "test" ]; then
       if [ "${TARGET_CPU}" == "ARM" ]; then
            if [ "${TARGET_IP}" != "" ]; then   # Network case
                ssh root@${TARGET_IP} mkdir -p /root/
                ssh root@${TARGET_IP} mkdir -p /root/raw
                cd ${TARGET_ROOT}
                for i in ${TARGET_TEST}; do
                    echo Installing ${i}
                    scp ${OUT_DIR}/${i} root@${TARGET_IP}:/usr/bin/
                done
            elif [ "${ROOTFS_DIR}" != "" ]; then # SD Card case
                ${SUDO} mkdir -p ${ROOTFS_DIR}/root
                ${SUDO} mkdir -p ${ROOTFS_DIR}/root/raw
                for i in ${TARGET_TEST}; do
                    echo Installing ${i}
                    ${SUDO} cp ${OUT_DIR}/${i} ${ROOTFS_DIR}/usr/bin/.
                done
                sync
            else
                echo "For network install: set TARGET_IP variable."
                echo "For SD card install: set ROOTFS_DIR variable."
                exit 0
            fi
        fi
    fi
    if [ "${1}" == "domxinstall" ]; then
        if [ "${TARGET_CPU}" == "ARM" ] && [ "${OMX_ROOT}" != "" ]; then
            if [ "${TARGET_IP}" != "" ]; then   # Network case
                echo Installing DOMX Libs
                scp ${OMX_ROOT}/domx/libdomx.so root@${TARGET_IP}:/usr/lib/
                scp ${OMX_ROOT}/omx_proxy_component/omx_camera/libOMX.TI.DUCATI1.VIDEO.CAMERA.so root@${TARGET_IP}:/usr/lib/
                scp ${OMX_ROOT}/omx_core/libomx_core.so root@${TARGET_IP}:/usr/lib/
                scp ${OMX_ROOT}/mm_osal/libmm_osal.so root@${TARGET_IP}:/usr/lib/
            elif [ "${ROOTFS_DIR}" != "" ]; then # SD Card case
                echo Installing DOMX Libs
                ${SUDO} cp ${OMX_ROOT}/domx/libdomx.so ${ROOTFS_DIR}/usr/lib/
                ${SUDO} cp ${OMX_ROOT}/omx_proxy_component/omx_camera/libOMX.TI.DUCATI1.VIDEO.CAMERA.so ${ROOTFS_DIR}/usr/lib/
                ${SUDO} cp ${OMX_ROOT}/omx_core/libomx_core.so ${ROOTFS_DIR}/usr/lib/
                ${SUDO} cp ${OMX_ROOT}/mm_osal/libmm_osal.so ${ROOTFS_DIR}/usr/lib/
                sync
            else
                echo "For network install: set TARGET_IP variable."
                echo "For SD card install: set ROOTFS_DIR variable."
                exit 0
            fi
        fi
    fi
    if [ "${1}" == "images" ]; then
       if [ "${TARGET_CPU}" == "ARM" ]; then
            if [ "${TARGET_IP}" != "" ]; then   # Network case
                ssh root@${TARGET_IP} mkdir -p /root/
                ssh root@${TARGET_IP} mkdir -p /root/raw
                echo Installing Input Test Videos
                scp ${TARGET_ROOT}/raw/input/hand* root@${TARGET_IP}:/root/raw/.
            elif [ "${ROOTFS_DIR}" != "" ]; then # SD Card case
                ${SUDO} mkdir -p ${ROOTFS_DIR}/root
                ${SUDO} mkdir -p ${ROOTFS_DIR}/root/raw
                echo Installing Input Test Videos
                ${SUDO} cp ${TARGET_ROOT}/raw/input/hand* ${ROOTFS_DIR}/root/raw/.
                sync
            else
                echo "For network install: set TARGET_IP variable."
                echo "For SD card install: set ROOTFS_DIR variable."
                exit 0
            fi
        fi
    fi
    if [ "${1}" == "baseimage" ]; then
       if [ "${TARGET_CPU}" == "ARM" ]; then
            if [ "${TARGET_IP}" != "" ]; then   # Network case
                ssh root@${TARGET_IP} mkdir -p /lib/firmware
                echo "Loading Ducati Base Image"
                scp ${TARGET_ROOT}/prebuilt/ANDROID/BLAZE/ducati-m3.bin root@${TARGET_IP}:/lib/firmware/.
                echo "Loading Tesla Base Image"
                scp ${TARGET_ROOT}/prebuilt/ANDROID/BLAZE/tesla-dsp.bin root@${TARGET_IP}:/lib/firmware/.
            elif [ "${ROOTFS_DIR}" != "" ]; then # SD Card case
                ${SUDO} mkdir ${ROOTFS_DIR}/lib/firmware
                echo "Loading Ducati Base Image"
                ${SUDO} cp ${TARGET_ROOT}/prebuilt/ANDROID/BLAZE/ducati-m3.bin ${ROOTFS_DIR}/lib/firmware/.
                echo "Loading Tesla Base Image"
                ${SUDO} cp ${TARGET_ROOT}/prebuilt/ANDROID/BLAZE/tesla-dsp.bin ${ROOTFS_DIR}/lib/firmware/.
                sync
            else
                echo "For network install: set TARGET_IP variable."
                echo "For SD card install: set ROOTFS_DIR variable."
                exit 0
            fi
        fi
    fi
    if [ "${1}" == "videos" ]; then
       if [ "${TARGET_CPU}" == "ARM" ]; then
            mkdir -p ${TARGET_ROOT}/raw/output/
            if [ "${TARGET_IP}" != "" ]; then   # Network case
                LISTING=`ssh root@${TARGET_IP} "ls /root/raw/" | grep "^[0-9][0-9]" | tr '\r' ' ' | tr '\n' ' '`
                for file in ${LISTING}
                do
                    echo "Copying ${file} from device to ${TARGET_ROOT}/raw/output/"
                    scp root@${TARGET_IP}:raw/${file} ${TARGET_ROOT}/raw/output/
                done
            elif [ "${ROOTFS_DIR}" != "" ]; then # SD Card case
                LISTING=`ls ${ROOTFS_DIR}/root/raw/ | grep "^[0-9][0-9]" | tr '\r' ' ' | tr '\n' ' '`
                for file in ${LISTING}
                do
                    echo "Copying ${file} from device to ${TARGET_ROOT}/raw/output/"
                    ${SUDO} cp ${ROOTFS_DIR}/raw/${file} ${TARGET_ROOT}/raw/output/
                done
            else
                echo "For network pull: set TARGET_IP variable."
                echo "For SD card pull: set ROOTFS_DIR variable."
                exit 0
            fi
        fi
    fi
    if [ "${1}" == "convert" ]; then
        LISTING=`ls ${TARGET_ROOT}/raw/output | grep "^[0-9][0-9]_bitimg" | tr '\r' ' ' | tr '\n' ' '`
        for file in ${LISTING}
        do
            echo "Converting ${file}"
            modfile=`echo ${file} | sed "s/bitimg/img/"`
            ${TARGET_ROOT}/out/LINUX/ARM/bitimg2img ${TARGET_ROOT}/raw/output/${file} ${TARGET_ROOT}/raw/output/${modfile}
        done
    fi
    if [ "${1}" == "docs" ]; then
        export PUBLIC_LIB_HEADER_LIST=" ${TARGET_ROOT}/libraries/public/yuv/include/yuv/dvp_kl_yuv.h
                                        ${TARGET_ROOT}/libraries/public/imgfilter/include/imgfilter/dvp_kl_imgfilter.h
                                        ${TARGET_ROOT}/libraries/public/ocl/include/ocl/dvp_kl_ocl.h
                                        ${TARGET_ROOT}/libraries/public/vrun/include/vrun/dvp_kl_vrun.h
                                        ${TARGET_ROOT}/libraries/public/dsplib/include/dsplib/dvp_kl_dsplib.h
                                        ${TARGET_ROOT}/libraries/public/dei/include/dei/dvp_kl_dei.h"
        export PRIVATE_LIB_HEADER_LIST="${VISION_ROOT}/libraries/protected/imglib/include/imglib/dvp_kl_imglib.h
                                        ${VISION_ROOT}/libraries/protected/vlib/include/vlib/dvp_kl_vlib.h
                                        ${VISION_ROOT}/libraries/protected/rvm/include/rvm/dvp_kl_rvm.h
                                        ${VISION_ROOT}/libraries/protected/tismo/include/tismo/dvp_kl_tismo.h
                                        ${VISION_ROOT}/libraries/protected/tismov02/include/tismov02/dvp_kl_tismov02.h"
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
            export PREDEF="PREDEFINED += LINUX DVP_USE_BO"

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
    if [ "${1}" == "debug" ]; then
        for i in $TARGET_TEST; do
            get_symbols ${i} bin
        done
        for i in $TARGET_LIBS; do
            get_symbols lib${i}.so lib
        done
    fi
    if [ "${1}" == "gdb" ] || [ "${1}" == "ddd" ]; then
        TYPE=${1}
        PROCESS=${2}
        shift # Remove "gdb"
        shift # Remove "process"
        if [ "${TARGET_IP}" != "" ]; then
            if [ "${TARGET_CPU}" == "ARM" ]; then   # Network case
                ssh root@${TARGET_IP} "gdbserver :5039 ${PROCESS} $*" &
                cat > gdb_cmds.txt << EOF
set solib-absolute-prefix ${OUT_DIR}
set solib-search-path ${OUT_DIR}
target remote ${TARGET_IP}:5039
EOF
                if [ "${TYPE}" == "gdb" ]; then
                    arm-none-linux-gnueabi-gdb ${OUT_DIR}/${PROCESS} --command=gdb_cmds.txt
                fi
                if [ "${TYPE}" == "ddd" ]; then
                    ddd --debugger arm-none-linux-gnueabi-gdb -command gdb_cmds.txt ${OUT_DIR}/${PROCESS} &
                fi
                #rm gdb_cmds.txt
                exit
            else
                echo "Unknown target cpu!"
                exit 0
            fi
        else    # Local CPU Case
            export SRC_DIRS=${TARGET_ROOT}
            for src in `find ${TARGET_ROOT}/source -type d | tr '\r' ' ' | tr '\n' ' '`; do
                export SRC_DIRS="${SRC_DIRS}:${src}"
            done
            export OBJ_DIRS=${TARGET_ROOT}
            for lib in `echo ${LD_LIBRARY_PATH} | tr ':' ' '`; do
                for obj in `find ${lib} -type d | tr '\r' ' ' | tr '\n' ' '`; do
                    export OBJ_DIRS="${OBJ_DIRS}:${obj}"
                done
            done
            cat > gdb_cmds.txt << EOF
set solib-absolute-prefix ${OUT_DIR}
set solib-search-path ${OUT_DIR}
directory ${SRC_DIRS}
path ${OBJ_DIRS}
EOF
            if [ "${TYPE}" == "gdb" ]; then
                gdb --command gdb_cmds.txt ${OUT_DIR}/$PROCESS
            fi
            if [ "${TYPE}" == "ddd" ]; then
                ddd --gdb --command gdb_cmds.txt ${OUT_DIR}/$PROCESS &
            fi
            exit
        fi
    fi
    if [ "${1}" == "zones" ] || [ "${1}" == "zone" ]; then
        if [ -n "${2}" ]; then
            zone_mask $2
            shift
        fi
        zone_mask print
    fi
    if [ "${1}" == "run" ]; then
        if [ -n "${2}" ]; then
            PROCESS=${2}
            shift
            shift
            if [ "${TARGET_CPU}" == "ARM" ]; then
                if [ "${TARGET_IP}" != "" ]; then   # Network case
                    ssh root@${TARGET_IP} ${PROCESS} $*
                else
                    echo "For network run: set TARGET_IP variable."
                    exit 0
                fi
            else
                export LD_LIBRARY_PATH=${OUT_DIR}
                ${OUT_DIR}/${PROCESS} $*
                exit
            fi
        fi
    fi
    if [ "${1}" == "shell" ]; then
        if [ "${TARGET_CPU}" == "ARM" ]; then
            if [ "${TARGET_IP}" != "" ]; then   # Network case
                ssh root@${TARGET_IP}
            fi
        fi
    fi
    shift
done
