@echo off
rem *********************************************************************
rem Automatically convert the LCLint documentation from HTML to IPF and
rem then produce an OS/2 INF file.
rem *********************************************************************

if "%1"=="--no-ed" goto noed
if "%1"=="" goto withed
goto help

:withed
rem *********************************************************************
rem First fix all HTML files by removing any HREF we don't want to 
rem follow.
rem *********************************************************************
if exist ..\..\lib copy ..\..\lib\*

echo fixing elvis.html...
ed elvis.html <elvis.html.ed

echo fixing elvismsg.html...
ed elvismsg.html <elvismsg.html.ed >nul

echo fixing the rest (messages disabled)...
for %%i in (*.html) do call fixhtml %%i

:noed
rem *********************************************************************
rem Now start the converter and produce a (preliminary) IPF file.
rem *********************************************************************
echo creating ipf file...
call html2ipf -SORT- elvis.html elvis.ipf

rem *********************************************************************
rem Fix the IPF file by removing some stuff that should not be there
rem and then sort the sections. At last run ipfc to produce the INF
rem output.
rem *********************************************************************
echo fixing elvis.ipf...
ed -s elvis.ipf <elvis.ipf.ed
:ipfc
echo creating inf file...
wipfc -inf elvis.ipf 
goto finish
:help
echo usage: h2i-guide.cmd [--no-ed]
:finish
