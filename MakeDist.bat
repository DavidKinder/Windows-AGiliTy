@echo off
"%ProgramFiles(x86)%\Zip\zip" -j \Temp\AGiliTyWin.zip COPYING Release\*.exe Release\*.dll Release\*.chm Release\*.cfg
"%ProgramFiles(x86)%\Zip\zip" \Temp\AGiliTyWinSource.zip *.* res\*.* Help\*.*
pushd \Programs
"%ProgramFiles(x86)%\Zip\zip" \Temp\AGiliTyWinSource.zip Libraries\mfc\*
popd

