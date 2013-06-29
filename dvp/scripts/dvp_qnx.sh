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

function target_ip() {

    if [ -z ${PB_TARGET_IP} ]; then

    if [ -z "`usb-devices | grep \"RIM Network Device\"`" ]; then
        echo "ERROR: PlayBook not found!"
        exit 0
    fi
    PB_HOST_IP=`ifconfig usb0 | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}'`
    PB_TARGET_IP1=`echo ${PB_HOST_IP} | cut -d. -f1`
    PB_TARGET_IP2=`echo ${PB_HOST_IP} | cut -d. -f2`
    PB_TARGET_IP3=`echo ${PB_HOST_IP} | cut -d. -f3`
    PB_TARGET_IP4=`echo ${PB_HOST_IP} | cut -d. -f4`
    PB_TARGET_IP4=`echo ${PB_TARGET_IP4} - 1 | bc`
    PB_TARGET_IP=`echo ${PB_TARGET_IP1}.${PB_TARGET_IP2}.${PB_TARGET_IP3}.${PB_TARGET_IP4}`
    if [ -z ${PB_TARGET_IP1} ]; then
        echo "ERROR: PlayBook link not giving any IP address!"
        exit 0
    fi
    echo PlayBook IP is: ${PB_TARGET_IP}
    fi
}

if [ $# == 0 ]; then
    echo "Usage: ${0} [docs|clean|mm|build|setup|remoteclean|install|videos|convert|test|baseimage]"
    echo ""
    echo " options: "
    echo " docs         - Uses Doxygen to generate the documentation from the code."
    echo "  setup       -  Installs your SSH public key for further loginless ssh operations."
    echo "  clean       -  Deletes previously built libraries & intermediate files on the host machine"
    echo "  remoteclean -  Deletes previously libraries & intermediate files on the target device."
    echo "  build|mm    -  Builds entire ${TARGET_PROJECT} project"
    echo "  install     -  Installs successfully-built libraries and applications."
    echo "  test        -  Runs the ${TARGET_PROJECT} framework in various standalone test modes."
    echo "  baseimage   -  Installs the Ducati / Tesla binaries & the Syslink on the target device and reboot the target device."
    echo "  videos      -  Downloads processed video files from target device to DVP's root folder/raw on host machine"
    echo "  convert     -  "
    echo ""
    exit
fi

export TARGET_PROJECT=DVP
export TARGET_ROOT=${DVP_ROOT}
export TARGET_LIBS="dvp dvp_kgm_cpu dvp_kgm_simcop dvp_kgm_dsp"
export TARGET_SLIB=
export TARGET_BINS=
export TARGET_TEST="dvp_simple dvp_demo dvp_test uinput_test sosal_test qnxscreen_test vcam_simple vcam_server"
export CROSS_COMPILE=ntoarmv7-

if [ -z "${TARGET_ROOT}" ]; then
    echo You must set your ${TARGET_PROJECT}_ROOT to the ${TARGET_PROJECT} build for QNX.
    exit
fi

export TARGET_OUT="${TARGET_ROOT}/out/__QNX__/ARM"

while [ $# -gt 0 ];
do
    if [ "${1}" == "setup" ]; then
        target_ip
        if [ ! -f "${HOME}/.ssh/id_rsa.pub" ]; then
            echo "ERROR: SSH public key NOT found"
            exit 0
        fi
        echo "Copying local SSH RSA public key to PlayBook"
        scp ${TARGET_ROOT}/prebuilt/__QNX__/etc/ssh/sshd_config root@${PB_TARGET_IP}:/etc/ssh/
        scp ${HOME}/.ssh/id_rsa.pub root@${PB_TARGET_IP}:/etc/ssh/authorized_keys
        echo "Done. Please restart your PlayBook for the changes to take effect."
        exit 0
    fi
    if [ "${1}" == "clean" ]; then
        cd ${TARGET_ROOT}
        make DEBUG=1 TARGET_PLATFORM=PLAYBOOK clean
    fi
    if [ "${1}" == "mm" ] || [ "${1}" == "build" ]; then
        cd ${TARGET_ROOT}
        make DEBUG=1 TARGET_PLATFORM=PLAYBOOK
    fi
    if [ "${1}" == "remoteclean" ]; then
        target_ip
        cd ${TARGET_ROOT}
        for i in ${TARGET_BINS}; do
            ssh root@${PB_TARGET_IP} "rm /usr/bin/${i}"
        done
        for i in ${TARGET_TEST}; do
            ssh root@${PB_TARGET_IP} "rm /usr/bin/${i}"
        done
        for i in ${TARGET_LIBS}; do
            ssh root@${PB_TARGET_IP} "rm /usr/lib/lib${i}.so"
        done
        ssh root@${PB_TARGET_IP} "rm /root/raw/*"
    fi
    if [ "${1}" == "install" ]; then
        target_ip
        echo Changing PlayBook filesystem permissions to Read/Write...
        ssh root@${PB_TARGET_IP} mount -uw /base
        if [ -z ${TARGET_OUT} ]; then
            echo "ERROR(${1}): There's no recognizable build output folders found"
            exit 0
        fi
        cd ${TARGET_ROOT}
        for i in ${TARGET_BINS}; do
            echo Installing ${i}
            scp ${TARGET_OUT}/${i} root@${PB_TARGET_IP}:/usr/bin/
        done
        for i in ${TARGET_LIBS}; do
            echo Installing lib${i}.so
            scp ${TARGET_OUT}/lib${i}.so root@${PB_TARGET_IP}:/usr/lib/
        done
        ssh root@${PB_TARGET_IP} mkdir -p /root/raw
    fi
    if [ "${1}" == "test" ]; then
        target_ip
        echo Changing PlayBook filesystem permissions to Read/Write...
        ssh root@${PB_TARGET_IP} mount -uw /base
        if [ -z ${TARGET_OUT} ]; then
            echo "ERROR(${1}): There's no recognizable build output folders found"
            exit 0
        fi
        cd ${TARGET_ROOT}
        for i in ${TARGET_TEST}; do
            echo Installing ${i}
            scp ${TARGET_OUT}/${i} root@${PB_TARGET_IP}:/usr/bin/
        done
    fi
    if [ "${1}" == "docs" ]; then
        export doxygen_status=`dpkg-query -W -f='${Status}\n' doxygen`
        if [ "${doxygen_status}" == "install ok installed" ]; then
            pushd ${TARGET_ROOT}
            doxygen docs/DoxyRules
            popd
            echo "Look in ${TARGET_ROOT}/docs/ for the generated documentation"
        else
            echo "doxygen package is not installed on your machine!"
            echo "To generate ${TARGET_PROJECT} docs, install the package (sudo apt-get install doxygen) and re-run this option."
        fi
    fi
    if [ "${1}" == "videos" ]; then
        target_ip
        LISTING=`ssh root@${PB_TARGET_IP} "ls /root/raw/" | grep "^[0-9][0-9]" | tr '\r' ' ' | tr '\n' ' '`
        mkdir -p ${TARGET_ROOT}/raw/output/
        for file in ${LISTING}
        do
            echo "Copying ${file} from device to ${TARGET_ROOT}/raw/output/"
            scp root@${PB_TARGET_IP}:raw/${file} ${TARGET_ROOT}/raw/output/
        done
    fi
    if [ "${1}" == "convert" ]; then
        target_ip
        LISTING=`ls ${TARGET_ROOT}/raw/output | grep "^[0-9][0-9]_bitimg" | tr '\r' ' ' | tr '\n' ' '`
        for file in ${LISTING}
        do
            echo "Converting ${file}"
            modfile=`echo ${file} | sed "s/bitimg/img/"`
            ${TARGET_ROOT}/bin/bitimg2img ${TARGET_ROOT}/raw/output/${file} ${TARGET_ROOT}/raw/output/${modfile}
        done
    fi
    if [ "${1}" == "unitest" ]; then
        target_ip
        ssh root@${PB_TARGET_IP} "sosal_test"
        ssh root@${PB_TARGET_IP} "dvp_test"
    fi
    if [ "${1}" == "ip" ]; then
        target_ip
    fi
    if [ "${1}" == "ducati" ]; then
        target_ip
        echo Changing PlayBook filesystem permissions to Read/Write...
        ssh root@${PB_TARGET_IP} mount -uw /base
        echo "Replacing Ducati Base Image"
        #scp root@${PB_TARGET_IP}:/lib/firmware/ducati/base_image_app_m3.xem3 ${TARGET_ROOT}/prebuilt/__QNX__/.
        scp ${TARGET_ROOT}/prebuilt/__QNX__/lib/firmware/ducati/base_image_app_m3.xem3 root@${PB_TARGET_IP}:/lib/firmware/ducati/
   fi
    if [ "${1}" == "baseimage" ]; then
        target_ip
        echo Changing PlayBook filesystem permissions to Read/Write...
        ssh root@${PB_TARGET_IP} mount -uw /base

        if [ -z "${QCONF_OVERRIDE}" ]; then
            INSTALL_DIR=${QNX_TARGET}
        else
            INSTALL_DIR=`cat ${QCONF_OVERRIDE} | grep INSTALL_ROOT_nto | cut -d= -f2`
        fi

        echo "Killing services before library updates (tiler mmsynclite syslink camera-omap)"
        ssh root@${PB_TARGET_IP} slay tiler mmsynclite syslink camera-omap
        echo "Loading Ducati Base Image"
        scp ${TARGET_ROOT}/prebuilt/__QNX__/lib/firmware/ducati/*.xem3 root@${PB_TARGET_IP}:/lib/firmware/ducati/
        echo "Loading Tesla Base Image (TODO)"
        echo "Loading binaries"
        scp ${INSTALL_DIR}/armle-v7/sbin/tiler* root@${PB_TARGET_IP}:/sbin/
        scp ${INSTALL_DIR}/armle-v7/bin/camera-omap root@${PB_TARGET_IP}:/bin/
        scp ${INSTALL_DIR}/armle-v7/bin/syslink* root@${PB_TARGET_IP}:/bin/
        echo "Loading libraries"
        ssh root@${PB_TARGET_IP} rm /usr/lib/libmemmgr* /usr/lib/libtiler* /usr/lib/lib[Ss]yslink* /usr/lib/libdce.so* /usr/lib/libaoi.so* /usr/lib/librocket.so* /usr/lib/libcamdrv.so*
        scp ${INSTALL_DIR}/armle-v7/usr/lib/libmmfomx.so* root@${PB_TARGET_IP}:/usr/lib/
        scp ${INSTALL_DIR}/armle-v7/usr/lib/libdce.so* root@${PB_TARGET_IP}:/usr/lib/
        scp ${INSTALL_DIR}/armle-v7/usr/lib/libOMX*.so* root@${PB_TARGET_IP}:/usr/lib/
        scp ${INSTALL_DIR}/armle-v7/usr/lib/libdomx.so* root@${PB_TARGET_IP}:/usr/lib/
        scp ${INSTALL_DIR}/armle-v7/usr/lib/libmmosal.so* root@${PB_TARGET_IP}:/usr/lib/
        scp ${INSTALL_DIR}/armle-v7/usr/lib/libaoi.so* root@${PB_TARGET_IP}:/usr/lib/
        scp ${INSTALL_DIR}/armle-v7/usr/lib/lib[Ss]yslink*.so* root@${PB_TARGET_IP}:/usr/lib/
        scp ${INSTALL_DIR}/armle-v7/usr/lib/libtilerusr.so* root@${PB_TARGET_IP}:/usr/lib/
        scp ${INSTALL_DIR}/armle-v7/usr/lib/libmemmgr*.so* root@${PB_TARGET_IP}:/usr/lib/
        scp ${INSTALL_DIR}/armle-v7/usr/lib/libcamdrv.so* root@${PB_TARGET_IP}:/usr/lib/
        scp ${INSTALL_DIR}/armle-v7/usr/lib/librocket.so* root@${PB_TARGET_IP}:/usr/lib/
        echo "Done. Please restart your PlayBook for the changes to take effect."
    fi
    if [ "${1}" == "cores" ]; then
        target_ip
        rm ${TARGET_ROOT}/out/*.core
        export CORE_LOC="/root /var/log"
        for loc in ${CORE_LOC}; do
            CORES=`ssh root@${PB_TARGET_IP} "cd ${loc} && ls *.core" | tr '\r' ' ' | tr '\n' ' '`
            echo ${CORES}
            for file in ${CORES}
            do
                echo "Copying file ${file} to PC"
                scp root@${PB_TARGET_IP}:${loc}/${file} ${TARGET_ROOT}/out/
            done
            ssh root@${PB_TARGET_IP} "rm ${loc}/*.core"
        done
    fi
    if [ "${1}" == "debug" ]; then
        OBJOPT="-C -d -g -f -l -S --include=${TARGET_ROOT}"
        echo Outputing Obj-Dump info on ${TARGET_PROJECT} objects
        for i in ${TARGET_BINS}; do
            ${CROSS_COMPILE}objdump ${OBJOPT} ${TARGET_OUT}/${i} > ${TARGET_ROOT}/out/${i}.txt
        done
        for i in ${TARGET_TEST}; do
            ${CROSS_COMPILE}objdump ${OBJOPT} ${TARGET_OUT}/${i} > ${TARGET_ROOT}/out/${i}.txt
        done
        for i in ${TARGET_LIBS}; do
            ${CROSS_COMPILE}objdump ${OBJOPT} ${TARGET_OUT}/lib${i}.so > ${TARGET_ROOT}/out/lib${i}.txt
        done
    fi
    if [ "${1}" == "gdb" ]; then
        target_ip
        # The GDB already knows the ${QNX_TARGET}/armle-v7 path
        cat > ${TARGET_OUT}/gdb_cmds.txt << EOF
set solib-search-path ${TARGET_OUT}
EOF
        ntoarm-gdb -cd=${TARGET_OUT} -x gdb_cmds.txt --readnow --exec=${TARGET_OUT}/${2} --core=${TARGET_ROOT}/out/${2}.core
    fi
    if [ "${1}" == "shell" ]; then
        target_ip
        if [ -z "${2}" ]; then
            ssh root@${PB_TARGET_IP}
        else
            shift # eat ${1}
            echo "Executing ${*} as command"
            ssh root@${PB_TARGET_IP} "$*"
        fi
    fi
    if [ "${1}" == "vcam_server" ]; then
        target_ip
        ssh -N -f root@${PB_TARGET_IP} -L 8501:localhost:8501 -L 8502:localhost:8502
        ssh root@${PB_TARGET_IP} vcam_server &
    fi
    if [ "${1}" == "tunnel" ]; then
        target_ip
        ssh -N -f root@${PB_TARGET_IP} -L 8501:localhost:8501 -L 8502:localhost:8502
    fi
    if [ "${1}" == "zones" ] || [ "${1}" == "zone" ]; then
        if [ -n "${2}" ]; then
            zone_mask ${2}
            shift
        fi
        zone_mask print
    fi
    shift
done

