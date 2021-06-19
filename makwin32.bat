@echo off

if "%1"=="clean" goto Clean
echo Checking environment...
if "%INCLUDE%"=="" goto EnvNeeded
echo Environment variables already set
goto EnvDone
:EnvNeeded
if exist \msdev\bin\vcvars32.bat call \msdev\bin\vcvars32
if exist "C:\Program Files\Microsoft Visual Studio\VC98\Bin\vcvars32.bat" call "C:\Program Files\Microsoft Visual Studio\VC98\Bin\vcvars32"
if "%INCLUDE%"=="" call vcvars32
:EnvDone

echo Configuring...
if not exist make.bat copy makwin32.bat make.bat
if not exist elvis.mak copy oswin32\elvis.mak
if not exist elvisutl.mak copy oswin32\elvisutl.mak
if not exist winelvis.mak copy oswin32\winelvis.mak
if not exist wintags.mak copy oswin32\wintags.mak
if not exist config.h copy oswin32\osconfig.h config.h

echo Compiling text-mode utilities...
nmake /nologo /s /f elvis.mak "CFG=elvis - Win32 Release"
if errorlevel 1 goto Fail
nmake /nologo /s /f elvisutl.mak
if errorlevel 1 goto Fail

echo Compiling graphical utilities...
nmake /nologo /s /f winelvis.mak CFG="WinElvis - Win32 Release"
if errorlevel 1 goto Fail
nmake /nologo /s /f wintags.mak CFG="WinTags - Win32 Release"
if errorlevel 1 goto Fail
goto Done

:Clean
if "%OS%"=="Windows_NT" goto Clean_NT
deltree /y winrel
deltree /y guirel
goto Done

:Clean_NT
for %%i in (WinRel WinDebug GuiRel GuiDebug) do if exist %%i rd /s/q %%i
for %%i in (WinElvis.exe WinTags.exe ctags.exe elvis.exe ex.exe fmt.exe ls.exe ref.exe vi.exe view.exe) do if exist %%i erase %%i
if exist *.pdb erase *.pdb
if exist *.ilk erase *.ilk

:Fail
:Done
