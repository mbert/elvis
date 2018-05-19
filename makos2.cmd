@echo off
if not exist osos2\config-with-tcp.h goto usage
if "%1"=="" goto withtcp
if "%1"=="--with-tcp" goto withtcp
if "%1"=="--no-tcp" goto notcp
if "%1"=="--with-gcc" goto gcc
if "%1"=="--with-emx" goto emx
if "%1"=="--with-debug" goto debug
if "%1"=="--with-x11" goto x11
if "%1"=="--with-all" goto all
if "%1"=="package" goto package
if "%1"=="manual" goto manual

:usage
echo usage: makos2 [--with-tcp --no-tcp --with-emx --with-x11 --with-gcc --with-all]
echo   the option "--no-tcp" will produce binaries not using the OS/2 TCP/IP APIs.
echo   the option "--with-gcc" is like "--no-tcp", but gcc is used instead of icc.
echo   the option "--with-emx" will use EMX including TCP/IP and Termcap.
echo   the option "--with-x11" will use EMX including TCP/IP, X11 and Termcap.
echo   the option "--with-all" will run all of the above plus the INF docs.
echo   else the standard executables with http/ftp abilities will be built,
echo   like if "--with-tcp" was specified.
echo this batch file must be started from within the elvis-2.1 directory.
goto done

:notcp
echo copying config file for non tcp/ip compile...
copy osos2\config-no-tcp.h config.h
echo deleting some object files, don't worry if they do not exist...
del calc.obj dmmarkup.obj ex.obj http.obj io.obj osnet.obj url.obj ftp.obj buffer.obj
echo building non-tcp/ip elvis...
make -f osos2\Makefile.os2 wcc-no-tcp
goto done

:gcc
echo copying config file for non tcp/ip compile using gcc...
rem copy osos2\config-no-tcp.h config.h
copy osos2\config-with-tcp.h config.h
echo building non-tcp/ip elvis with emx/gcc...
make -f osos2\Makefile.os2 gcc
goto done

:emx
echo copying config file for emx version...
copy osos2\config-with-tcp.h config.h
echo building emx elvis with emx/gcc...
make -j 4 -f osos2\Makefile.os2 emx
goto done

:debug
echo copying config file for debug version...
copy osos2\config-with-tcp.h config.h
echo building x11 elvis with emx/gcc...
make -f osos2\Makefile.os2 debug
goto done

:x11
echo copying config file for x11 version...
copy osos2\config-with-tcp.h config.h
echo building x11 elvis with emx/gcc...
make -f osos2\Makefile.os2 x11
goto done

:withtcp
echo copying config file for tcp/ip compile...
copy osos2\config-with-tcp.h config.h
echo building elvis and misc programs...
make -f osos2\Makefile.os2 wcc
goto done

:package
cd exeos2
del *gcc.exe
cd ..
del elvis-2.2_1-os2.tar.gz
make -f osos2/Makefile.os2 elvis-2.2_1-os2.tar.gz
goto done

:manual
if exist osos2\manual cmd /c del /n osos2\manual\*
if not exist osos2\manual mkdir osos2\manual
copy osos2\manual-scripts\* osos2\manual
copy doc\* osos2\manual
cd osos2\manual
cmd /c h2i-guide.cmd
cd ..\..
if not exist doc mkdir doc
copy osos2\manual\elvis.INF doc
goto done

:all
del *.obj *.o >nul
call makos2.cmd --with-tcp
if errorlevel 1 goto error
del *.obj >nul
call makos2.cmd --no-tcp
if errorlevel 1 goto error
del *.obj >nul
rem call makos2.cmd --with-gcc
rem if errorlevel 1 goto error
rem del *.obj >nul
call makos2.cmd --with-emx
if errorlevel 1 goto error
del *.o >nul
call makos2.cmd --with-x11
if errorlevel 1 goto error
del *.o >nul
call makos2.cmd manual
if errorlevel 1 goto error

echo success.
goto done

:error
echo build failed, aborted.

:done
