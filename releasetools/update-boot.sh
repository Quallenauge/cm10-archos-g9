#!/bin/bash

echo $1
if [ "$1" == "extractcopy" ] ; then
    bootimage-extract boot.img -k kernel -i ramdisk.img
    cp -v kernel /media/android/A80S_REC/zImage
    cp -v ramdisk.img /media/android/A80S_REC/initramfs.cpio.lzo
fi

if [ "$1" == "copyfile" ] ; then
    bootimage-extract boot.img -k kernel -i ramdisk.img
    cp -v $2 /media/android/A80S_REC/zImage
    cp -v $3 /media/android/A80S_REC/initramfs.cpio.lzo
fi


if [ "$1" == "copy" ] ; then
    cp -v zImage /media/android/A80S_REC/zImage
    cp -v initramfs.cpio.lzo /media/android/A80S_REC/initramfs.cpio.lzo
fi
if [ "$1" == "create" ] ; then
    bootimage-create boot.img -k zImage -i initramfs.cpio.lzo
    7zip update update-boot.zip boot.img
fi

if [ "$1" == "update" ] ; then
    7zip update update-boot.zip boot.img
fi



