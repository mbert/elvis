# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=WinElvis - Win32 Release
!MESSAGE No configuration specified.  Defaulting to WinElvis - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "WinElvis - Win32 Release" && "$(CFG)" !=\
 "WinElvis - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "WinElvis.mak" CFG="WinElvis - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WinElvis - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "WinElvis - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "WinElvis - Win32 Debug"
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "WinElvis - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinElvis"
# PROP BASE Intermediate_Dir "WinElvis"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\WinElvis.exe"

CLEAN : 
	-@erase "$(INTDIR)\buffer.obj"
	-@erase "$(INTDIR)\calc.obj"
	-@erase "$(INTDIR)\cut.obj"
	-@erase "$(INTDIR)\digraph.obj"
	-@erase "$(INTDIR)\display.obj"
	-@erase "$(INTDIR)\dmhex.obj"
	-@erase "$(INTDIR)\dmmarkup.obj"
	-@erase "$(INTDIR)\dmnormal.obj"
	-@erase "$(INTDIR)\dmsyntax.obj"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\event.obj"
	-@erase "$(INTDIR)\ex.obj"
	-@erase "$(INTDIR)\exaction.obj"
	-@erase "$(INTDIR)\exconfig.obj"
	-@erase "$(INTDIR)\exedit.obj"
	-@erase "$(INTDIR)\exmake.obj"
	-@erase "$(INTDIR)\exsubst.obj"
	-@erase "$(INTDIR)\gui.obj"
	-@erase "$(INTDIR)\guiwin.obj"
	-@erase "$(INTDIR)\gwcmd.obj"
	-@erase "$(INTDIR)\gwdlgs.obj"
	-@erase "$(INTDIR)\gwmsg.obj"
	-@erase "$(INTDIR)\gwopts.obj"
	-@erase "$(INTDIR)\gwprint.obj"
	-@erase "$(INTDIR)\gwstatb.obj"
	-@erase "$(INTDIR)\gwtoolb.obj"
	-@erase "$(INTDIR)\gwutil.obj"
	-@erase "$(INTDIR)\input.obj"
	-@erase "$(INTDIR)\io.obj"
	-@erase "$(INTDIR)\lowbuf.obj"
	-@erase "$(INTDIR)\lp.obj"
	-@erase "$(INTDIR)\lpescape.obj"
	-@erase "$(INTDIR)\lpovrtyp.obj"
	-@erase "$(INTDIR)\lpps.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\map.obj"
	-@erase "$(INTDIR)\mark.obj"
	-@erase "$(INTDIR)\message.obj"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\more.obj"
	-@erase "$(INTDIR)\move.obj"
	-@erase "$(INTDIR)\need.obj"
	-@erase "$(INTDIR)\operator.obj"
	-@erase "$(INTDIR)\optglob.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\osblock.obj"
	-@erase "$(INTDIR)\osdir.obj"
	-@erase "$(INTDIR)\osprg.obj"
	-@erase "$(INTDIR)\ostext.obj"
	-@erase "$(INTDIR)\regexp.obj"
	-@erase "$(INTDIR)\regsub.obj"
	-@erase "$(INTDIR)\safe.obj"
	-@erase "$(INTDIR)\scan.obj"
	-@erase "$(INTDIR)\search.obj"
	-@erase "$(INTDIR)\session.obj"
	-@erase "$(INTDIR)\state.obj"
	-@erase "$(INTDIR)\vi.obj"
	-@erase "$(INTDIR)\vicmd.obj"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(INTDIR)\winelvis.obj"
	-@erase "$(INTDIR)\winelvis.res"
	-@erase "$(INTDIR)\wintools.obj"
	-@erase "$(OUTDIR)\WinElvis.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "." /I ".." /I "..\oswin32" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GUI_WIN32" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "." /I ".." /I "..\oswin32" /D "NDEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "GUI_WIN32" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/winelvis.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/WinElvis.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /map
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /incremental:no /pdb:"$(OUTDIR)/WinElvis.pdb" /machine:I386\
 /out:"$(OUTDIR)/WinElvis.exe" 
LINK32_OBJS= \
	"$(INTDIR)\buffer.obj" \
	"$(INTDIR)\calc.obj" \
	"$(INTDIR)\cut.obj" \
	"$(INTDIR)\digraph.obj" \
	"$(INTDIR)\display.obj" \
	"$(INTDIR)\dmhex.obj" \
	"$(INTDIR)\dmmarkup.obj" \
	"$(INTDIR)\dmnormal.obj" \
	"$(INTDIR)\dmsyntax.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\event.obj" \
	"$(INTDIR)\ex.obj" \
	"$(INTDIR)\exaction.obj" \
	"$(INTDIR)\exconfig.obj" \
	"$(INTDIR)\exedit.obj" \
	"$(INTDIR)\exmake.obj" \
	"$(INTDIR)\exsubst.obj" \
	"$(INTDIR)\gui.obj" \
	"$(INTDIR)\guiwin.obj" \
	"$(INTDIR)\gwcmd.obj" \
	"$(INTDIR)\gwdlgs.obj" \
	"$(INTDIR)\gwmsg.obj" \
	"$(INTDIR)\gwopts.obj" \
	"$(INTDIR)\gwprint.obj" \
	"$(INTDIR)\gwstatb.obj" \
	"$(INTDIR)\gwtoolb.obj" \
	"$(INTDIR)\gwutil.obj" \
	"$(INTDIR)\input.obj" \
	"$(INTDIR)\io.obj" \
	"$(INTDIR)\lowbuf.obj" \
	"$(INTDIR)\lp.obj" \
	"$(INTDIR)\lpescape.obj" \
	"$(INTDIR)\lpovrtyp.obj" \
	"$(INTDIR)\lpps.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\mark.obj" \
	"$(INTDIR)\message.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\more.obj" \
	"$(INTDIR)\move.obj" \
	"$(INTDIR)\need.obj" \
	"$(INTDIR)\operator.obj" \
	"$(INTDIR)\optglob.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\osblock.obj" \
	"$(INTDIR)\osdir.obj" \
	"$(INTDIR)\osprg.obj" \
	"$(INTDIR)\ostext.obj" \
	"$(INTDIR)\regexp.obj" \
	"$(INTDIR)\regsub.obj" \
	"$(INTDIR)\safe.obj" \
	"$(INTDIR)\scan.obj" \
	"$(INTDIR)\search.obj" \
	"$(INTDIR)\session.obj" \
	"$(INTDIR)\state.obj" \
	"$(INTDIR)\vi.obj" \
	"$(INTDIR)\vicmd.obj" \
	"$(INTDIR)\window.obj" \
	"$(INTDIR)\winelvis.obj" \
	"$(INTDIR)\winelvis.res" \
	"$(INTDIR)\wintools.obj"

"$(OUTDIR)\WinElvis.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "WinElvis - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\WinElvis.exe"

CLEAN : 
	-@erase "$(INTDIR)\buffer.obj"
	-@erase "$(INTDIR)\calc.obj"
	-@erase "$(INTDIR)\cut.obj"
	-@erase "$(INTDIR)\digraph.obj"
	-@erase "$(INTDIR)\display.obj"
	-@erase "$(INTDIR)\dmhex.obj"
	-@erase "$(INTDIR)\dmmarkup.obj"
	-@erase "$(INTDIR)\dmnormal.obj"
	-@erase "$(INTDIR)\dmsyntax.obj"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\event.obj"
	-@erase "$(INTDIR)\ex.obj"
	-@erase "$(INTDIR)\exaction.obj"
	-@erase "$(INTDIR)\exconfig.obj"
	-@erase "$(INTDIR)\exedit.obj"
	-@erase "$(INTDIR)\exmake.obj"
	-@erase "$(INTDIR)\exsubst.obj"
	-@erase "$(INTDIR)\gui.obj"
	-@erase "$(INTDIR)\guiwin.obj"
	-@erase "$(INTDIR)\gwcmd.obj"
	-@erase "$(INTDIR)\gwdlgs.obj"
	-@erase "$(INTDIR)\gwmsg.obj"
	-@erase "$(INTDIR)\gwopts.obj"
	-@erase "$(INTDIR)\gwprint.obj"
	-@erase "$(INTDIR)\gwstatb.obj"
	-@erase "$(INTDIR)\gwtoolb.obj"
	-@erase "$(INTDIR)\gwutil.obj"
	-@erase "$(INTDIR)\input.obj"
	-@erase "$(INTDIR)\io.obj"
	-@erase "$(INTDIR)\lowbuf.obj"
	-@erase "$(INTDIR)\lp.obj"
	-@erase "$(INTDIR)\lpescape.obj"
	-@erase "$(INTDIR)\lpovrtyp.obj"
	-@erase "$(INTDIR)\lpps.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\map.obj"
	-@erase "$(INTDIR)\mark.obj"
	-@erase "$(INTDIR)\message.obj"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\more.obj"
	-@erase "$(INTDIR)\move.obj"
	-@erase "$(INTDIR)\need.obj"
	-@erase "$(INTDIR)\operator.obj"
	-@erase "$(INTDIR)\optglob.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\osblock.obj"
	-@erase "$(INTDIR)\osdir.obj"
	-@erase "$(INTDIR)\osprg.obj"
	-@erase "$(INTDIR)\ostext.obj"
	-@erase "$(INTDIR)\regexp.obj"
	-@erase "$(INTDIR)\regsub.obj"
	-@erase "$(INTDIR)\safe.obj"
	-@erase "$(INTDIR)\scan.obj"
	-@erase "$(INTDIR)\search.obj"
	-@erase "$(INTDIR)\session.obj"
	-@erase "$(INTDIR)\state.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\vi.obj"
	-@erase "$(INTDIR)\vicmd.obj"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(INTDIR)\winelvis.obj"
	-@erase "$(INTDIR)\winelvis.res"
	-@erase "$(INTDIR)\wintools.obj"
	-@erase "$(OUTDIR)\WinElvis.exe"
	-@erase "$(OUTDIR)\WinElvis.ilk"
	-@erase "$(OUTDIR)\WinElvis.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "." /I ".." /I "..\oswin32" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GUI_WIN32" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "." /I ".." /I "..\oswin32" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GUI_WIN32" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/winelvis.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/WinElvis.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)/WinElvis.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/WinElvis.exe" 
LINK32_OBJS= \
	"$(INTDIR)\buffer.obj" \
	"$(INTDIR)\calc.obj" \
	"$(INTDIR)\cut.obj" \
	"$(INTDIR)\digraph.obj" \
	"$(INTDIR)\display.obj" \
	"$(INTDIR)\dmhex.obj" \
	"$(INTDIR)\dmmarkup.obj" \
	"$(INTDIR)\dmnormal.obj" \
	"$(INTDIR)\dmsyntax.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\event.obj" \
	"$(INTDIR)\ex.obj" \
	"$(INTDIR)\exaction.obj" \
	"$(INTDIR)\exconfig.obj" \
	"$(INTDIR)\exedit.obj" \
	"$(INTDIR)\exmake.obj" \
	"$(INTDIR)\exsubst.obj" \
	"$(INTDIR)\gui.obj" \
	"$(INTDIR)\guiwin.obj" \
	"$(INTDIR)\gwcmd.obj" \
	"$(INTDIR)\gwdlgs.obj" \
	"$(INTDIR)\gwmsg.obj" \
	"$(INTDIR)\gwopts.obj" \
	"$(INTDIR)\gwprint.obj" \
	"$(INTDIR)\gwstatb.obj" \
	"$(INTDIR)\gwtoolb.obj" \
	"$(INTDIR)\gwutil.obj" \
	"$(INTDIR)\input.obj" \
	"$(INTDIR)\io.obj" \
	"$(INTDIR)\lowbuf.obj" \
	"$(INTDIR)\lp.obj" \
	"$(INTDIR)\lpescape.obj" \
	"$(INTDIR)\lpovrtyp.obj" \
	"$(INTDIR)\lpps.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\mark.obj" \
	"$(INTDIR)\message.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\more.obj" \
	"$(INTDIR)\move.obj" \
	"$(INTDIR)\need.obj" \
	"$(INTDIR)\operator.obj" \
	"$(INTDIR)\optglob.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\osblock.obj" \
	"$(INTDIR)\osdir.obj" \
	"$(INTDIR)\osprg.obj" \
	"$(INTDIR)\ostext.obj" \
	"$(INTDIR)\regexp.obj" \
	"$(INTDIR)\regsub.obj" \
	"$(INTDIR)\safe.obj" \
	"$(INTDIR)\scan.obj" \
	"$(INTDIR)\search.obj" \
	"$(INTDIR)\session.obj" \
	"$(INTDIR)\state.obj" \
	"$(INTDIR)\vi.obj" \
	"$(INTDIR)\vicmd.obj" \
	"$(INTDIR)\window.obj" \
	"$(INTDIR)\winelvis.obj" \
	"$(INTDIR)\winelvis.res" \
	"$(INTDIR)\wintools.obj"

"$(OUTDIR)\WinElvis.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "WinElvis - Win32 Release"
# Name "WinElvis - Win32 Debug"

!IF  "$(CFG)" == "WinElvis - Win32 Release"

!ELSEIF  "$(CFG)" == "WinElvis - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\wintools.c
DEP_CPP_WINTO=\
	".\wintools.h"\
	

"$(INTDIR)\wintools.obj" : $(SOURCE) $(DEP_CPP_WINTO) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\winelvis.c
DEP_CPP_WINEL=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	".\winelvis.h"\
	".\wintools.h"\
	

"$(INTDIR)\winelvis.obj" : $(SOURCE) $(DEP_CPP_WINEL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=..\oswin32\ostext.c
DEP_CPP_OSTEX=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

"$(INTDIR)\ostext.obj" : $(SOURCE) $(DEP_CPP_OSTEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\oswin32\osdir.c
DEP_CPP_OSDIR=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	{$(INCLUDE)}"\sys\Stat.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

"$(INTDIR)\osdir.obj" : $(SOURCE) $(DEP_CPP_OSDIR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\oswin32\osprg.c
DEP_CPP_OSPRG=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\osprg.obj" : $(SOURCE) $(DEP_CPP_OSPRG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\oswin32\osblock.c
DEP_CPP_OSBLO=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

"$(INTDIR)\osblock.obj" : $(SOURCE) $(DEP_CPP_OSBLO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\window.c
DEP_CPP_WINDO=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\window.obj" : $(SOURCE) $(DEP_CPP_WINDO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\calc.c
DEP_CPP_CALC_=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\calc.obj" : $(SOURCE) $(DEP_CPP_CALC_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\cut.c
DEP_CPP_CUT_C=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\cut.obj" : $(SOURCE) $(DEP_CPP_CUT_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\digraph.c
DEP_CPP_DIGRA=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\digraph.obj" : $(SOURCE) $(DEP_CPP_DIGRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\display.c
DEP_CPP_DISPL=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\display.obj" : $(SOURCE) $(DEP_CPP_DISPL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\dmhex.c
DEP_CPP_DMHEX=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\dmhex.obj" : $(SOURCE) $(DEP_CPP_DMHEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\dmmarkup.c
DEP_CPP_DMMAR=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\dmmarkup.obj" : $(SOURCE) $(DEP_CPP_DMMAR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\dmnormal.c
DEP_CPP_DMNOR=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\dmnormal.obj" : $(SOURCE) $(DEP_CPP_DMNOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\dmsyntax.c
DEP_CPP_DMSYN=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\dmsyntax.obj" : $(SOURCE) $(DEP_CPP_DMSYN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\draw.c
DEP_CPP_DRAW_=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\draw.obj" : $(SOURCE) $(DEP_CPP_DRAW_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\event.c
DEP_CPP_EVENT=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\event.obj" : $(SOURCE) $(DEP_CPP_EVENT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\ex.c
DEP_CPP_EX_C22=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\ex.obj" : $(SOURCE) $(DEP_CPP_EX_C22) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\exaction.c
DEP_CPP_EXACT=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\exaction.obj" : $(SOURCE) $(DEP_CPP_EXACT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\exconfig.c
DEP_CPP_EXCON=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\exconfig.obj" : $(SOURCE) $(DEP_CPP_EXCON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\exedit.c
DEP_CPP_EXEDI=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	
"$(INTDIR)\exedit.obj" : $(SOURCE) $(DEP_CPP_EXEDI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\exmake.c
DEP_CPP_EXMAK=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\exmake.obj" : $(SOURCE) $(DEP_CPP_EXMAK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\exsubst.c
DEP_CPP_EXEDI=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	
"$(INTDIR)\exsubst.obj" : $(SOURCE) $(DEP_CPP_EXEDI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\gui.c
DEP_CPP_GUI_C=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\gui.obj" : $(SOURCE) $(DEP_CPP_GUI_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\input.c
DEP_CPP_INPUT=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\input.obj" : $(SOURCE) $(DEP_CPP_INPUT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\io.c
DEP_CPP_IO_C30=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\io.obj" : $(SOURCE) $(DEP_CPP_IO_C30) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\lowbuf.c
DEP_CPP_LOWBU=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\lowbuf.obj" : $(SOURCE) $(DEP_CPP_LOWBU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\lp.c
DEP_CPP_LP_C34=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\lp.obj" : $(SOURCE) $(DEP_CPP_LP_C34) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\lpescape.c
DEP_CPP_LPESC=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\lpescape.obj" : $(SOURCE) $(DEP_CPP_LPESC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\lpovrtyp.c
DEP_CPP_LPOVR=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\lpovrtyp.obj" : $(SOURCE) $(DEP_CPP_LPOVR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\lpps.c
DEP_CPP_LPPS_=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\lpps.obj" : $(SOURCE) $(DEP_CPP_LPPS_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\main.c
DEP_CPP_MAIN_=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\map.c
DEP_CPP_MAP_C=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\map.obj" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\mark.c
DEP_CPP_MARK_=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\mark.obj" : $(SOURCE) $(DEP_CPP_MARK_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\message.c
DEP_CPP_MESSA=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\message.obj" : $(SOURCE) $(DEP_CPP_MESSA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\misc.c
DEP_CPP_MISC_=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\misc.obj" : $(SOURCE) $(DEP_CPP_MISC_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\more.c
DEP_CPP_MORE_=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\more.obj" : $(SOURCE) $(DEP_CPP_MORE_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\move.c
DEP_CPP_MOVE_=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\move.obj" : $(SOURCE) $(DEP_CPP_MOVE_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\need.c
DEP_CPP_NEED_=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\need.obj" : $(SOURCE) $(DEP_CPP_NEED_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\operator.c
DEP_CPP_OPERA=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\operator.obj" : $(SOURCE) $(DEP_CPP_OPERA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\optglob.c
DEP_CPP_OPTGL=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\optglob.obj" : $(SOURCE) $(DEP_CPP_OPTGL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\options.c
DEP_CPP_OPTIO=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\options.obj" : $(SOURCE) $(DEP_CPP_OPTIO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\regexp.c
DEP_CPP_REGEX=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\regexp.obj" : $(SOURCE) $(DEP_CPP_REGEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\regsub.c
DEP_CPP_REGSU=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\regsub.obj" : $(SOURCE) $(DEP_CPP_REGSU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\safe.c
DEP_CPP_SAFE_=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\safe.obj" : $(SOURCE) $(DEP_CPP_SAFE_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\scan.c
DEP_CPP_SCAN_=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\scan.obj" : $(SOURCE) $(DEP_CPP_SCAN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\search.c
DEP_CPP_SEARC=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\search.obj" : $(SOURCE) $(DEP_CPP_SEARC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\session.c
DEP_CPP_SESSI=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\session.obj" : $(SOURCE) $(DEP_CPP_SESSI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\state.c
DEP_CPP_STATE=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\state.obj" : $(SOURCE) $(DEP_CPP_STATE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\vi.c
DEP_CPP_VI_C60=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\vi.obj" : $(SOURCE) $(DEP_CPP_VI_C60) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\vicmd.c
DEP_CPP_VICMD=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\vicmd.obj" : $(SOURCE) $(DEP_CPP_VICMD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\buffer.c
DEP_CPP_BUFFE=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	

"$(INTDIR)\buffer.obj" : $(SOURCE) $(DEP_CPP_BUFFE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\gwopts.c
DEP_CPP_GWOPT=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	".\winelvis.h"\
	

"$(INTDIR)\gwopts.obj" : $(SOURCE) $(DEP_CPP_GWOPT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\gwprint.c

!IF  "$(CFG)" == "WinElvis - Win32 Release"

DEP_CPP_GWPRI=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\config.h"\
	".\..\elvctype.h"\
	".\..\lowbuf.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\version.h"\
	".\winelvis.h"\
	

"$(INTDIR)\gwprint.obj" : $(SOURCE) $(DEP_CPP_GWPRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinElvis - Win32 Debug"

DEP_CPP_GWPRI=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	".\winelvis.h"\
	

"$(INTDIR)\gwprint.obj" : $(SOURCE) $(DEP_CPP_GWPRI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\guiwin.c
DEP_CPP_GUIWI=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	".\winelvis.h"\
	

"$(INTDIR)\guiwin.obj" : $(SOURCE) $(DEP_CPP_GUIWI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\gwmsg.c
DEP_CPP_GWMSG=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	".\winelvis.h"\
	

"$(INTDIR)\gwmsg.obj" : $(SOURCE) $(DEP_CPP_GWMSG) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\winelvis.rc
DEP_RSC_WINELV=\
	".\elvis.ico"\
	".\toolbar.bmp"\
	".\arrow.cur"\
	

"$(INTDIR)\winelvis.res" : $(SOURCE) $(DEP_RSC_WINELV) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\gwutil.c
DEP_CPP_GWUTI=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	".\winelvis.h"\
	

"$(INTDIR)\gwutil.obj" : $(SOURCE) $(DEP_CPP_GWUTI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\gwstatb.c
DEP_CPP_GWSTA=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	".\winelvis.h"\
	

"$(INTDIR)\gwstatb.obj" : $(SOURCE) $(DEP_CPP_GWSTA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\gwcmd.c

!IF  "$(CFG)" == "WinElvis - Win32 Release"

DEP_CPP_GWCMD=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\config.h"\
	".\..\elvctype.h"\
	".\..\lowbuf.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\version.h"\
	".\winelvis.h"\
	

"$(INTDIR)\gwcmd.obj" : $(SOURCE) $(DEP_CPP_GWCMD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinElvis - Win32 Debug"

DEP_CPP_GWCMD=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	".\winelvis.h"\
	

"$(INTDIR)\gwcmd.obj" : $(SOURCE) $(DEP_CPP_GWCMD) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gwdlgs.c
DEP_CPP_GWDLG=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	".\winelvis.h"\
	".\wintools.h"\
	

"$(INTDIR)\gwdlgs.obj" : $(SOURCE) $(DEP_CPP_GWDLG) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\gwtoolb.c
DEP_CPP_GWTOO=\
	"..\elvis.h"\
	"..\map.h"\
	"..\oswin32\osdef.h"\
	".\..\buffer.h"\
	".\..\buffer2.h"\
	".\..\calc.h"\
	".\..\config.h"\
	".\..\cut.h"\
	".\..\digraph.h"\
	".\..\display.h"\
	".\..\display2.h"\
	".\..\draw.h"\
	".\..\draw2.h"\
	".\..\elvctype.h"\
	".\..\elvisio.h"\
	".\..\event.h"\
	".\..\ex.h"\
	".\..\gui.h"\
	".\..\gui2.h"\
	".\..\input.h"\
	".\..\lowbuf.h"\
	".\..\lp.h"\
	".\..\mark.h"\
	".\..\message.h"\
	".\..\misc.h"\
	".\..\more.h"\
	".\..\move.h"\
	".\..\need.h"\
	".\..\operator.h"\
	".\..\opsys.h"\
	".\..\optglob.h"\
	".\..\options.h"\
	".\..\options2.h"\
	".\..\regexp.h"\
	".\..\safe.h"\
	".\..\scan.h"\
	".\..\session.h"\
	".\..\state.h"\
	".\..\state2.h"\
	".\..\version.h"\
	".\..\vi.h"\
	".\..\vicmd.h"\
	".\..\window.h"\
	".\winelvis.h"\
	

"$(INTDIR)\gwtoolb.obj" : $(SOURCE) $(DEP_CPP_GWTOO) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
