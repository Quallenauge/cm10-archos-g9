@ECHO ON

@IF "%ROOTDIR%" == "" @GOTO EXIT
@IF "%DVP_ROOT%" == "" @GOTO EXIT

@SET INC_DIRS=include\sosal include\dvp
@SET SRC_DIRS=source\sosal source\dvp\dvp_kgraph source\dvp\dvp_kgm_cpu 
@SET TARGET=C:\sdks\av_bios_sdk_00_06_00_00\avsdk_00_06_00_00\demos\avsk_app\a8

@SET XCOPY_OPTS=/I /Y
xcopy %DVP_ROOT%\include\sosal\*.h %TARGET%\inc\sosal\ %XCOPY_OPTS%
xcopy %DVP_ROOT%\include\dvp\*.h %TARGET%\inc\dvp\ %XCOPY_OPTS%
xcopy %DVP_ROOT%\source\dvp\dvp_kgraph\include\*.h %TARGET%\inc\dvp\ %XCOPY_OPTS%
xcopy %DVP_ROOT%\source\dvp\dvp_kgm_cpu\*.h %TARGET%\inc\dvp\ %XCOPY_OPTS%
xcopy %DVP_ROOT%\source\sosal\*.c %TARGET%\src\sosal\ %XCOPY_OPTS%
xcopy %DVP_ROOT%\source\dvp\dvp_kgraph\*.c %TARGET%\src\dvp\ %XCOPY_OPTS%
xcopy %DVP_ROOT%\source\dvp\dvp_kgm_cpu\*.c %TARGET%\src\dvp\ %XCOPY_OPTS%

:EXIT