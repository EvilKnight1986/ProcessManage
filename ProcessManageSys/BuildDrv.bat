@echo off
set mod=%1
@echo.
if "%1"=="" (
 @Rem.Ĭ�����ɵ��� 32 λ����������
 set mod=x86 
 @echo.      ��������������������������������������������������������������������
 @echo.      ���÷�:                                                           ��
 @echo.      ��    ���� x86 ����: BuildDrv.bat [x86 [win7/wnet/wlh/wxp]]       ��
 @echo.      ��    ���� amd64 ����: BuildDrv.bat [amd64 [win7/wnet/wlh/wxp]]   ��
 @echo.      ��������������������������������������������������������������������
)

@Rem::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@Rem::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@Rem     ���ı���汾        ::
@Rem          ::
@Rem  ���Ҫ���� Release �������,�������һ�иĳ� set release=true ::
   set release=false
@Rem::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@Rem          ::
@Rem ϵͳ����:        ::
@Rem windows xp=wxp        ::
@Rem windows 2003=wnet       ::
@Rem windows 2008/vista=wlh       ::
@Rem windows 7/windows 2008 r2=win7      ::
@Rem Ĭ�����ɵ��� win7 ϵͳ����������,�������� target_os �ĳ���Ӧ��ֵ. ::
 set target_os=win7

if not "%2"=="" set target_os=%2
@Rem::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@Rem::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
set chk_fre=chk
@echo.
if "%release%"=="true" set chk_fre=fre

@Rem::::::::::::::::::::: ��ɾ��ԭ�����ļ���  ::::::::::::::::::
if not "%mod%"=="amd64" ( 
 set mod=x86
 FOR /d %%A IN (obj%chk_fre%_%target_os%_%mod%) DO @echo. delete "%%A" ... && rmdir /s /q %%A >NUL
) else ( 
 FOR /d %%A IN (obj%chk_fre%_%target_os%_%mod%) DO @echo. delete "%%A" ... && rmdir /s /q %%A >NUL
 set mod=x64
)
@Rem:::::::::::::::::::::    ���ñ��뻷��   :::::::::::::::::
@echo. ���� %mod% ������...
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
 @echo.����ɹ�!!!
 FOR /r %%A IN (*.log,*.tlog,*.lastbuildstate,*vc*.pdb,*result.dat,*list.txt,*.err,*.wrn) DO del /s /q %%A >NUL
 if "%mod%"=="x86" ( 
@Rem:::::::::::::�������ɹ�,���ļ������������::::::::::::::::::::::::::::::::::::
@Rem::::::::::::       :::::::::::::::::::
  if exist \\Win2k3\Driver\  (
   @echo.
   @echo.����������ļ������������Ŀ¼: "\\win2k3\Driver\"
   copy /y %_path%\*.sys \\Win2k3\Driver\
  )

  @Rem �������ļ�������Ŀ¼
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
