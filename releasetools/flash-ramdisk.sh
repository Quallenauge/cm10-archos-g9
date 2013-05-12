#!/bin/sh

##
##  flash-ramdisk.sh - test script for quick ramdisk kernel flashing
##  make ramdisk
##  cd ${ANDROID_PRODUCT_OUT}
##  adb push ramdisk.img .
##  adb push kernel .
##  adb shell kd_flasher -i ramdisk.img -k kernel
##  adb reboot

adb push ramdisk.img .
adb push kernel .
adb shell kd_flasher -i ramdisk.img -k kernel
adb reboot