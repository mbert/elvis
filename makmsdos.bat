@echo off

REM   We need to set the PATH and a few other environment variables, but
REM   only once!
if %INCLUDE%X==X call \msvc\bin\msvcvars
if %TMP%X==X set TMP=C:\

REM   Copy the MS-DOS versions of the configuration files into the main
REM   elvis source directory.
echo Configuring...
if not exist elvis.mak copy osmsdos\elvis.mak
if not exist ctags.mak copy osmsdos\ctags.mak
if not exist ref.mak copy osmsdos\ref.mak
if not exist fmt.mak copy osmsdos\fmt.mak
if not exist vi.mak copy osmsdos\vi.mak
if not exist config.h copy osmsdos\osconfig.h config.h

REM   And one more for convenience!
if not exist make.bat copy makmsdos.bat make.bat

REM   Make the programs
echo Compiling...
nmake /nologo /s /f elvis.mak DEBUG=0
if errorlevel 1 goto Fail
nmake /nologo /s /f ctags.mak DEBUG=0
if errorlevel 1 goto Fail
nmake /nologo /s /f ref.mak DEBUG=0
if errorlevel 1 goto Fail
nmake /nologo /s /f fmt.mak DEBUG=0
if errorlevel 1 goto Fail
nmake /nologo /s /f vi.mak DEBUG=0
copy vi.exe ex.exe
copy vi.exe view.exe

:Fail
