#!/bin/sh

# Temporary Script to make a recovery ramdisk
rm -v $ANDROID_PRODUCT_OUT/kernel
rm -rf $ANDROID_PRODUCT_OUT/obj/KERNEL_OBJ/
rm -v $ANDROID_BUILD_TOP/kernel/archos/archos_g9/usr/initramfs_data.cpio
cp -v $ANDROID_PRODUCT_OUT/ramdisk-recovery.cpio  $ANDROID_BUILD_TOP/kernel/archos/archos_g9/usr/initramfs_data.cpio 
