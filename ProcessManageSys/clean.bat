@echo off
FOR /d %%A IN (objchk_*) DO rmdir /s /q %%A
FOR /d %%A IN (objfre_*) DO rmdir /s /q %%A
FOR /d %%A IN (i386*,adm64*,Win32*,x64*,WinDDk*,ipch*) DO rmdir /s /q %%A
FOR /r %%A IN (*.ncb *.user *.suo *.obj *.pch  *.ilk *.idb *.sdf *.tlog *.ipch *.log *err *.xml) DO del /s /q %%A
if "%1"=="pause" pause