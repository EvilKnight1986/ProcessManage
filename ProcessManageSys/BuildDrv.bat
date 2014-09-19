@echo off
set mod=%1
@echo.
if "%1"=="" (
 @Rem.默认生成的是 32 位的驱动程序
 set mod=x86 
 @echo.      ┌────────────────────────────────┐
 @echo.      │用法:                                                           │
 @echo.      │    构建 x86 驱动: BuildDrv.bat [x86 [win7/wnet/wlh/wxp]]       │
 @echo.      │    构建 amd64 驱动: BuildDrv.bat [amd64 [win7/wnet/wlh/wxp]]   │
 @echo.      └────────────────────────────────┘
)

@Rem::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@Rem::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@Rem     更改编译版本        ::
@Rem          ::
@Rem  如果要生成 Release 版的驱动,请把下面一行改成 set release=true ::
   set release=false
@Rem::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@Rem          ::
@Rem 系统代号:        ::
@Rem windows xp=wxp        ::
@Rem windows 2003=wnet       ::
@Rem windows 2008/vista=wlh       ::
@Rem windows 7/windows 2008 r2=win7      ::
@Rem 默认生成的是 win7 系统的驱动程序,请把下面的 target_os 改成相应的值. ::
 set target_os=win7

if not "%2"=="" set target_os=%2
@Rem::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@Rem::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
set chk_fre=chk
@echo.
if "%release%"=="true" set chk_fre=fre

@Rem::::::::::::::::::::: 先删除原来的文件夹  ::::::::::::::::::
if not "%mod%"=="amd64" ( 
 set mod=x86
 FOR /d %%A IN (obj%chk_fre%_%target_os%_%mod%) DO @echo. delete "%%A" ... && rmdir /s /q %%A >NUL
) else ( 
 FOR /d %%A IN (obj%chk_fre%_%target_os%_%mod%) DO @echo. delete "%%A" ... && rmdir /s /q %%A >NUL
 set mod=x64
)
@Rem:::::::::::::::::::::    设置编译环境   :::::::::::::::::
@echo. 构建 %mod% 驱动中...
pushd.
call %WLHBASE%\bin\setenv.bat %WLHBASE% %chk_fre% %mod% %target_os% no_oacr
popd.
@Rem::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
if "%mod%"=="x64" set mod=amd64
@echo.____________________________________________________________________________
if "%1"=="" (
 build  /g /F
) else (
 nmake
)
@echo.____________________________________________________________________________

@Rem:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

set /A is_success=0
set _path=obj%chk_fre%_%target_os%_%mod%\%mod%
if "%mod%"=="x86" (  
 set _path=obj%chk_fre%_%target_os%_%mod%\i386
)
@echo.
if exist %_path%\*.sys set /A is_success=1

@Rem::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

set msg=

if %is_success%==1 (
 @echo.编译成功!!!
 FOR /r %%A IN (*.log,*.tlog,*.lastbuildstate,*vc*.pdb,*result.dat,*list.txt,*.err,*.wrn) DO del /s /q %%A >NUL
 if "%mod%"=="x86" ( 
@Rem:::::::::::::如果编译成功,把文件拷贝到虚拟机::::::::::::::::::::::::::::::::::::
@Rem::::::::::::       :::::::::::::::::::
  if exist \\Win2k3\Driver\  (
   @echo.
   @echo.拷贝下面的文件到虚拟机共享目录: "\\win2k3\Driver\"
   copy /y %_path%\*.sys \\Win2k3\Driver\
  )

  @Rem 拷贝符文件到符号目录
  if exist E:\driver\ (
   if not exist e:\driver\symbols md e:\driver\Symbols
   copy /y %_path%\*.pdb e:\driver\symbols\ >NUL
  )
 )
       
 set msg=build for %target_os% %mod% %chk_fre% driver success.
) else (        
 @Rem.if exist *.err TYPE *.err 
 set msg=build for %target_os% %mod% %chk_fre% driver failed.
 FOR /r %%A IN (*.log,*.tlog,*.lastbuildstate,*vc*.pdb,*result.dat,*list.txt,*.err,*.wrn) DO del /s /q %%A >NUL
)
@echo.
@echo.++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
@echo.
@echo.    %msg%
@echo.
@echo.++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if %is_success%==0 pause
