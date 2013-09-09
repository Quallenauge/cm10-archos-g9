#!/bin/bash
# This script will update the archive contents from a DDK build

SGX_BIN_LOC=$1
if [ -z "${SGX_BIN_LOC}" ] ; then
echo "usage: $0 <sgx installed binary path>"
exit
fi

DIRS="etc modules lib bin"

mkdir old.$$
for d in ${DIRS}; do
	mv $d old.$$
done

set -x
cp -r ${SGX_BIN_LOC}/system/etc .
cp -r ${SGX_BIN_LOC}/system/lib/modules .
cp -r ${SGX_BIN_LOC}/system/vendor/lib .
cp -r ${SGX_BIN_LOC}/system/vendor/bin .

set +x
rm -rf old.$$

# sanity check - check for a subset of binaries
#
# Criteria:
# Different GPUs/platforms
# Kernel modules
# EGL modules

BINS="lib/hw/gralloc.omap4470.so \
 lib/hw/gralloc.omap4460.so \
 lib/hw/gralloc.omap4430.so \
 lib/egl/libEGL_POWERVR_SGX540_120.so \
 lib/egl/libEGL_POWERVR_SGX544_112.so \
 modules/pvrsrvkm_sgx544_112.ko \
 modules/pvrsrvkm_sgx540_120.ko"

GOOD=true
for b in ${BINS} ; do
	if [ ! -f $b ] ; then
		echo Missing: $b
		GOOD=
	else
		echo Checked: $b
	fi
done

# Checking for unexpected dirs
CURDIRS=`ls -A`
for i in ${CURDIRS} ; do
	if [ -d "${i}" ] ; then
		matched=
		for j in ${DIRS} ; do
			if [ "$j" = "$i" ] ; then
				matched=$j
				break;
			fi
		done
		if [ -z "$matched" ] ; then
			echo Unknown dir!!! $i
			GOOD=
		fi
	fi
done

if [ -z "${GOOD}" ] ; then
	echo There is a problem, checking warnings
else
	echo DONE! Now zip me up with
        echo "tar -czvf sgx.tgz sgx"
fi
