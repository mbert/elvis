@echo off

if "%1"=="clean" goto Clean

echo Compiling text-mode binary
make -f Makefile.mingw
if errorlevel 1 goto Fail
goto Done

:Clean
del /F /Q *.o
cd oswin32
del /F /Q *.o
cd ..

:Fail
:Done
