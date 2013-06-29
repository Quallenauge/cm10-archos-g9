@ECHO OFF

@REM Copyright (C) 2011 Texas Insruments, Inc.
@REM
@REM Licensed under the Apache License, Version 2.0 (the "License");
@REM you may not use this file except in compliance with the License.
@REM You may obtain a copy of the License at
@REM
@REM      http://www.apache.org/licenses/LICENSE-2.0
@REM
@REM Unless required by applicable law or agreed to in writing, software
@REM distributed under the License is distributed on an "AS IS" BASIS,
@REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@REM See the License for the specific language governing permissions and
@REM limitations under the License.


@IF "%MYDROID%" == "" @(
    @ECHO "You must set the MYDROID variable!
    @GOTO END
)

@IF "%_PRODUCT_%" == "" @(
    SET _PRODUCT_=blaze
)

@IF "%TI_HW_ROOT%" == "" @(
    SET TI_HW_ROOT=hardware\ti
)

@SET OUT_DIR=%MYDROID%\out\target\product\%_PRODUCT_%
@SET DVP_ROOT=%MYDROID%\%TI_HW_ROOT%\dvp
@SET TARGET_LIBS=dvp dvp_kgm_cpu dvp_kgm_imx dvp_kgm_dsp
@SET TARGET_BINS=shm_service
@SET TARGET_APPS=
@SET TARGET_PACKAGES=
@SET TARGET_TEST=vcam_simple vcam_test vcam_server dvp_simple dvp_demo sosal_test uinput_test
@SET TARGET_SRVC=
@SET TARGET_SPKG=
@SET TARGET_DOMX=camera OMX_Core omx_rpc omx_proxy_common OMX.TI.DUCATI1.VIDEO.CAMERA OMX.TI.DUCATI1.IMAGE.JPEGD

:LOOP
    @IF "%1" == "setup" @(
        adb kill-server
        adb start-server
        adb get-state
        adb devices
    )
    @IF "%1" == "logcat" @(
        start /min adb logcat
    )
    @IF "%1" == "omx" @(
        adb remount
        @FOR %%A IN (%TARGET_DOMX%) DO @(
            adb shell "rm /system/lib/lib%%A.so"
            adb push %OUT_DIR%\system\lib\lib%%A.so /system/lib
        )
    )
    @IF "%1" == "test" @(
        adb remount
        @FOR %%A IN (%TARGET_TEST%) DO @(
            @ECHO Reinstalling %%A
            adb shell "rm /system/bin/%%A"
            adb push %OUT_DIR%\system\bin\%%A /system/bin
            adb shell "chmod 755 /system/bin/%%A"
            adb shell "chown root /system/bin/%%A"
            @REM adb shell "chgrp shell /system/bin/%%A"
        )
    )
    @IF "%1" == "export" @(
        @FOR %%A IN (%TARGET_LIBS%) DO @(
            @echo Copying lib%%A.so
            copy %OUT_DIR%\system\lib\lib%%A.so %2
        )
        @FOR %%A IN (%TARGET_BINS%) DO @(
            @echo Copying %%A
            copy %OUT_DIR%\system\bin\%%A %2
        )
        @FOR %%A IN (%TARGET_APPS%) DO @(
            @echo Copying %%A.apk
            copy %OUT_DIR%\system\app\%%A.apk %2
        )
        @FOR %%A IN (%TARGET_SRVC%) DO @(
            @echo Copying %%A.apk
            copy %OUT_DIR%\system\app\%%A.apk %2
        )
    )
    @IF "%1" == "install" @(
        adb remount
        @FOR %%A IN (%TARGET_LIBS%) DO @(
            @ECHO Installing %%A
            adb push %OUT_DIR%\system\lib\lib%%A.so /system/lib
        )
        @FOR %%A IN (%TARGET_BINS%) DO @(
            @ECHO Installing %%A
            adb push %OUT_DIR%\system\bin\%%A /system/bin
            adb shell "chmod 755 /system/bin/%%A"
            adb shell "chown root /system/bin/%%A"
            @REM adb shell "chgrp shell /system/bin/%%A"
        )
    )
    @IF "%1" == "remoteclean" @(
        adb remount
        @FOR %%A IN (%TARGET_LIBS%) DO @(
            @ECHO Uninstalling %%A
            adb shell "rm /system/lib/lib%%A.so"
        )
        @FOR %%A IN (%TARGET_BINS%) DO @(
            @ECHO Uninstalling %%A
            adb shell "rm /system/bin/%%A"
        )
        @FOR %%A IN (%TARGET_PACKAGES%) DO @(
            @ECHO Uninstalling %%A
            adb shell "rm -rf /data/data/%%A"
            adb uninstall %%A
        )
        @FOR %%A IN (%TARGET_SPKG%) DO @(
            @ECHO Uninstalling %%A
            adb shell "rm -rf /data/data/%%A"
            adb uninstall %%A
        )
    )
    @IF "%1" == "qdisp" @(
        adb push %DVP_ROOT%\scripts\qdisp.sh /data
        adb shell "chmod 755 /data/qdisp.sh"
        adb shell "/data/qdisp.sh"
    )
    @IF "%1" == "v4l2" @(
        adb shell "echo \"2lcd\" > /sys/devices/platform/omapdss/overlay2/manager"
        adb shell "echo 2 > /sys/devices/platform/omapdss/overlay0/zorder"
        adb shell "echo 1 > /sys/devices/platform/omapdss/overlay1/zorder"
        adb shell "echo 3 > /sys/devices/platform/omapdss/overlay2/zorder"
        adb shell "echo 1 > /sys/devices/platform/omapdss/overlay2/enabled"
        adb shell "echo 0 > /sys/devices/platform/omapdss/manager2/trans_key_value"
        adb shell "echo \"video-source\" > /sys/devices/platform/omapdss/manager2/trans_key_type"
    )
    @IF "%1" == "perf" @(
        adb shell "echo \"ondemand\" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
        adb shell "echo 1 > /sys/devices/system/cpu/cpu1/online
        adb shell "echo \"performance\" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
        adb shell "cat /sys/devices/system/cpu/cpu0/online"
        adb shell "cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
        adb shell "cat /sys/devices/system/cpu/cpu1/online"
        adb shell "cat /sys/devices/system/cpu/cpu1/cpufreq/scaling_governor"
    )
    @IF "%1" == "service" (
        start /b adb shell "shm_service"
        adb shell "ps" | find "shm_service"
    )
    @IF "%1" == "tombstones" @(
    	adb pull /data/tombstones/ .
    	adb shell "rm -rf /data/tombstones/*"
    )
    @IF "%1" == "vcam_server" @(
    	adb forward tcp:8501 tcp:8501
    	adb forward tcp:8502 tcp:8502
    	adb shell vcam_server
    )
    @SHIFT
    @IF "%1" == "" GOTO END
@GOTO LOOP

:END

