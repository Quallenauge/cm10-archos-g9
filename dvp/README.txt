Distributred Vision Processing (DVP) for OMAP Devices
==============================================

This repository is composed of
1.) DVP Kernel Graph API is composed of
    a.) DVP Kernel Graph Boss - which divies up work to the
        1.) DVP Kernel Graph Managers (A9/DSP/M3) - which call the algorithms of
            a.) VLIB, RVM, YUV, etc...
2.) SOSAL - Simple Operating System Abstraction - simple wrapper for Win32/POSIX APIs (just what is needed)

Build Instructions (under Linux/Cygwin but not for Android) for PC:
===============================================================================
$ export TARGET_PLATFORM=PC
# To enable debugging statements
$ export DVP_DEBUG=1
$ export SOSAL_DEBUG=1
# To enable certain zones, set the hexidecimal number for each zone (bit) 0xFFFF is all zones
$ export DVP_ZONE_MASK=0xFFFFFFFF
$ export SOSAL_ZONE_MASK=0xFFFFFFFF
or you can run the
$ ./env.sh

then

$ make clean
$ make
$ make test

If you are building in Windows in a DOS prompt, use the MinGW's make.exe with the
same targets as above. (replace export with SET or run the env.bat)

Build Instructions (under Android):
===============================================================================
$ cd $(MYDROID)
$ hardware/ti/dvp/scripts/dvp.sh clean mm

Build Instructions (for QNX):
===============================================================================
Install the QNX Build System and Configure your environment, then:
$ export IPC_ROOT=<path to syslink>
$ export TILER_ROOT=<path to tiler>
$ export DVP_ROOT=<path to this folder>
$ export SOSAL_ROOT=$DVP_ROOT
$ export TARGET_PLATFORM=PLAYBOOK
$ make clean
$ make

DESCRIPTION:
===============================================================================
dvp.sh - build script for android
dvp.bat - Install script for Android phones which can only connect on Windows.
env.sh   - Bash script for development build environment
env.bat  - Batch script for development build environment
Android.mk - Android Top Level Build Makefile
Makefile - makefiles for non-Android builds


