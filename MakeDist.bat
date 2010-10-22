@echo off
"\Program Files\Zip\zip" -j \Temp\AGiliTyWin.zip COPYING Release\*.exe Release\*.chm Release\*.cfg
"\Program Files\Zip\zip" \Temp\AGiliTyWinSource.zip *.* res\*.* Help\*.*
pushd \Programs
"\Program Files\Zip\zip" \Temp\AGiliTyWinSource.zip Libraries\mfc\*
popd

