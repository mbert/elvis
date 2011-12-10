@echo off

echo Configuring...
if not exist make.bat copy makwin32.bat make.bat
if not exist ctags.mak copy oswin32\ctags.mak
if not exist elvis.mak copy oswin32\elvis.mak
if not exist elvisutl.mak copy oswin32\elvisutl.mak
if not exist fmt.mak copy oswin32\fmt.mak
if not exist ref.mak copy oswin32\ref.mak
if not exist config.h copy oswin32\osconfig.h config.h
if not exist WinRel\nul md WinRel

echo Compiling...
nmake /nologo /s /f elvis.mak "CFG=Win32 Release" elvis.exe
nmake /nologo /s /f elvisutl.mak
