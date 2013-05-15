# Copyright (C) 2012 The Android Open Source Project
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

import common
import os

TARGET_DIR = os.getenv('OUT')
UTILITIES_DIR = os.path.join(TARGET_DIR, 'utilities')
def FullOTA_InstallBegin(info):


    info["fstab"] = [partition for partition in info["fstab"] if not "boot" in cmd]
    info.output_zip.write(os.path.join(UTILITIES_DIR, "busybox"), "busybox")
    info.output_zip.write(os.path.join(UTILITIES_DIR, "abcbox"), "abcbox")
    info.output_zip.write(os.path.join(UTILITIES_DIR, "make_ext4fs"), "make_ext4fs")
    
    info.output_zip.write(os.path.join(TARGET_DIR, "kernel"), "kernel")
    info.output_zip.write(os.path.join(TARGET_DIR, "ramdisk.img"), "ramdisk.img")
    
    info.script.AppendExtra('package_extract_file("busybox", "/tmp/busybox");')
    info.script.AppendExtra('package_extract_file("abcbox", "/tmp/abcbox");')
    info.script.AppendExtra('package_extract_file("make_ext4fs", "/tmp/make_ext4fs");')
    
    info.script.AppendExtra('set_perm(0, 0, 0755, "/tmp/busybox");')
    info.script.AppendExtra('set_perm(0, 0, 0755, "/tmp/make_ext4fs");')
    info.script.AppendExtra('set_perm(0, 0, 0755, "/tmp/abcbox");')
    info.script.AppendExtra('symlink("/tmp/abcbox","/tmp/kd_flasher","/tmp/flash_partition_erase","/tmp/reboot_into");')
    info.script.AppendExtra('symlink("/tmp/busybox","/tmp/mount","/tmp/mkdir");')
    
    info.script.AppendExtra('if (!is_mounted("/mnt/rawfs")) then')
    info.script.AppendExtra('ui_print("mounting rawfs");')
    info.script.AppendExtra('assert(run_program("/tmp/utils/mount","-trawfs", "/dev/block/mmcblk0p1", "/mnt/rawfs"));')
    info.script.AppendExtra('endif;')
    
    info.script.AppendExtra('ui_print("*****        FLASHING BOOT IMAGE        *****");')
    info.script.AppendExtra('package_extract_file("kernel", "/tmp/kernel");')
    info.script.AppendExtra('package_extract_file("ramdisk.img", "/tmp/ramdisk.img");')
    info.script.AppendExtra('assert(run_program("/tmp/utils/kd_flasher","-i", "/tmp/ramdisk.img", "-k" , "/tmp/kernel" ));')
    info.script.AppendExtra('ui_print("*****               DONE                *****");')
    info.script.AppendExtra('ui_print("");')
    info.script.AppendExtra('ui_print("*****        INSTALLING SYSTEM          *****");')
    
    
    
def FullOTA_InstallEnd(info):
  # Remove writing boot.img from script (we do it in updater.sh)
  info.script.script = [cmd for cmd in info.script.script if not "boot.img" in cmd]