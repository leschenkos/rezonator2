@echo off
setlocal

set PROJ_ROOT=%~dp0
set PROJ_ROOT=%PROJ_ROOT:~0,-1%
set PROJ_BIN=%PROJ_ROOT%\bin

echo -----------------------------------------------------------------
echo Prepare muparser

set MUPARSER=%PROJ_ROOT%\libs\muparser

:muparser_clone
echo ------------------------
echo Clone repo...

if exist %MUPARSER% rmdir /S /Q %MUPARSER%
git clone https://github.com/beltoforion/muparser %MUPARSER%
cd %MUPARSER%
git checkout v2.2.5

:muparser_build
echo ------------------------
echo Build...

cd %MUPARSER%\build
set MAKE=mingw32-make
where /Q %MAKE%
if %ERRORLEVEL% neq 0 (
	echo ERROR: %MAKE% was not found in PATH
	echo Ensure you have MinGW installed and update PATH, e.g.:
    echo set PATH=C:\Qt\Tools\mingw530_32\bin;%%PATH%%
	goto end
)
rem Build static lib because of dll does not provide C++ interface, only plain C
%MAKE% -fmakefile.mingw SHARED=0 DEBUG=0

:end
cd %PROJ_ROOT%
