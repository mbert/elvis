@echo off
if not exist osos2\config-with-tcp.h goto usage
if "%1"=="" goto withtcp
if "%1"=="--no-tcp" goto notcp
if "%1"=="--with-gcc" goto gcc
if "%1"=="--with-emx" goto emx
if "%1"=="--with-debug" goto debug
if "%1"=="--with-x11" goto x11

:usage
echo usage: makos2 [--no-tcp, --with-emx, --with-gcc]
echo   the option "--no-tcp" will produce binaries not using the OS/2 TCP/IP APIs.
echo   the option "--with-gcc" is like "--no-tcp", but gcc is used instead of icc.
echo   the option "--with-emx" will use EMX including TCP/IP and Termcap.
echo   the option "--with-x11" will use EMX including TCP/IP, X11 and Termcap.
echo   else the standard executables with http/ftp abilities will be built.
echo this batch file must be started from within the elvis-2.1 directory.
goto done

:notcp
echo copying config file for non tcp/ip compile...
copy osos2\config-no-tcp.h config.h
echo deleting some object files, don't worry if they do not exist...
del calc.obj dmmarkup.obj ex.obj http.obj io.obj osnet.obj url.obj ftp.obj buffer.obj
echo building non-tcp/ip elvis...
make -f osos2\makefile.os2 ALL="elvis-no-tcp.exe ref.exe ctags.exe fmt.exe
goto done

:gcc
echo copying config file for non tcp/ip compile using gcc...
copy osos2\config-no-tcp.h config.h
echo building non-tcp/ip elvis with emx/gcc...
make -f osos2\makefile.os2 gcc
goto done

:emx
echo copying config file for emx version...
copy osos2\config-with-tcp.h config.h
echo building emx elvis with emx/gcc...
make -f osos2\makefile.os2 emx
goto done

:debug
echo copying config file for debug version...
copy osos2\config-with-tcp.h config.h
echo building x11 elvis with emx/gcc...
make -f osos2\makefile.os2 debug
goto done

:x11
echo copying config file for x11 version...
copy osos2\config-with-tcp.h config.h
echo building x11 elvis with emx/gcc...
make -f osos2\makefile.os2 x11
goto done

:withtcp
echo copying config file for tcp/ip compile...
copy osos2\config-with-tcp.h config.h
echo building elvis and misc programs...
make -f osos2\makefile.os2 all
goto done

:done
