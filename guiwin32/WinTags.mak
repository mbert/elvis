# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=WinTags - Win32 Release
!MESSAGE No configuration specified.  Defaulting to WinTags - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "WinTags - Win32 Release" && "$(CFG)" !=\
 "WinTags - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "WinTags.mak" CFG="WinTags - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WinTags - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "WinTags - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "WinTags - Win32 Release"
MTL=mktyplib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WinTags - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\WinTags.exe"

CLEAN : 
	-@erase ".\Release\WinTags.exe"
	-@erase ".\Release\wintags.obj"
	-@erase ".\Release\wintools.obj"
	-@erase ".\Release\ctags.obj"
	-@erase ".\Release\wintags.res"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "." /I ".." /I "..\oswin32" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GUI_WIN32" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "." /I ".." /I "..\oswin32" /D "NDEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "GUI_WIN32" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/wintags.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/WinTags.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /incremental:no /pdb:"$(OUTDIR)/WinTags.pdb" /machine:I386\
 /out:"$(OUTDIR)/WinTags.exe" 
LINK32_OBJS= \
	"$(INTDIR)/wintags.obj" \
	"$(INTDIR)/wintools.obj" \
	"$(INTDIR)/ctags.obj" \
	"$(INTDIR)/wintags.res"

"$(OUTDIR)\WinTags.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "WinTags - Win32 Debug"

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

ALL : "$(OUTDIR)\WinTags.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\WinTags.exe"
	-@erase ".\Debug\wintools.obj"
	-@erase ".\Debug\wintags.obj"
	-@erase ".\Debug\ctags.obj"
	-@erase ".\Debug\wintags.res"
	-@erase ".\Debug\WinTags.ilk"
	-@erase ".\Debug\WinTags.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "." /I ".." /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GUI_WIN32" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "." /I ".." /D "_DEBUG" /D "WIN32"\
 /D "_WINDOWS" /D "GUI_WIN32" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/wintags.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/WinTags.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)/WinTags.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/WinTags.exe" 
LINK32_OBJS= \
	"$(INTDIR)/wintools.obj" \
	"$(INTDIR)/wintags.obj" \
	"$(INTDIR)/ctags.obj" \
	"$(INTDIR)/wintags.res"

"$(OUTDIR)\WinTags.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "WinTags - Win32 Release"
# Name "WinTags - Win32 Debug"

!IF  "$(CFG)" == "WinTags - Win32 Release"

!ELSEIF  "$(CFG)" == "WinTags - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\wintags.c
DEP_CPP_WINTA=\
	".\..\elvis.h"\
	".\wintools.h"\
	".\..\config.h"\
	".\..\elvctype.h"\
	".\..\version.h"\
	"..\osdef.h"\
	".\..\safe.h"\
	".\..\options.h"\
	".\..\optglob.h"\
	".\..\session.h"\
	".\..\lowbuf.h"\
	".\..\buffer.h"\
	".\..\mark.h"\
	".\..\buffer2.h"\
	".\..\options2.h"\
	".\..\scan.h"\
	".\..\message.h"\
	".\..\opsys.h"\
	".\..\gui.h"\
	".\..\display.h"\
	".\..\draw.h"\
	".\..\state.h"\
	".\..\window.h"\
	".\..\gui2.h"\
	".\..\display2.h"\
	".\..\draw2.h"\
	".\..\state2.h"\
	".\..\event.h"\
	".\..\input.h"\
	".\..\vi.h"\
	".\..\regexp.h"\
	".\..\ex.h"\
	".\..\move.h"\
	".\..\vicmd.h"\
	".\..\operator.h"\
	".\..\cut.h"\
	".\..\elvisio.h"\
	".\..\lp.h"\
	".\..\calc.h"\
	".\..\more.h"\
	".\..\digraph.h"\
	".\..\need.h"\
	".\..\misc.h"\
	

"$(INTDIR)\wintags.obj" : $(SOURCE) $(DEP_CPP_WINTA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=..\ctags.c

!IF  "$(CFG)" == "WinTags - Win32 Release"

DEP_CPP_CTAGS=\
	".\..\elvis.h"\
	".\..\oswin32\osdir.c"\
	".\..\config.h"\
	".\..\elvctype.h"\
	".\..\version.h"\
	"..\osdef.h"\
	".\..\safe.h"\
	".\..\options.h"\
	".\..\optglob.h"\
	".\..\session.h"\
	".\..\lowbuf.h"\
	".\..\buffer.h"\
	".\..\mark.h"\
	".\..\buffer2.h"\
	".\..\options2.h"\
	".\..\scan.h"\
	".\..\message.h"\
	".\..\opsys.h"\
	".\..\gui.h"\
	".\..\display.h"\
	".\..\draw.h"\
	".\..\state.h"\
	".\..\window.h"\
	".\..\gui2.h"\
	".\..\display2.h"\
	".\..\draw2.h"\
	".\..\state2.h"\
	".\..\event.h"\
	".\..\input.h"\
	".\..\vi.h"\
	".\..\regexp.h"\
	".\..\ex.h"\
	".\..\move.h"\
	".\..\vicmd.h"\
	".\..\operator.h"\
	".\..\cut.h"\
	".\..\elvisio.h"\
	".\..\lp.h"\
	".\..\calc.h"\
	".\..\more.h"\
	".\..\digraph.h"\
	".\..\need.h"\
	".\..\misc.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	{$(INCLUDE)}"\sys\Stat.h"\
	

"$(INTDIR)\ctags.obj" : $(SOURCE) $(DEP_CPP_CTAGS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "WinTags - Win32 Debug"

DEP_CPP_CTAGS=\
	".\..\elvis.h"\
	".\..\config.h"\
	".\..\elvctype.h"\
	".\..\version.h"\
	"..\osdef.h"\
	".\..\safe.h"\
	".\..\options.h"\
	".\..\optglob.h"\
	".\..\session.h"\
	".\..\lowbuf.h"\
	".\..\buffer.h"\
	".\..\mark.h"\
	".\..\buffer2.h"\
	".\..\options2.h"\
	".\..\scan.h"\
	".\..\message.h"\
	".\..\opsys.h"\
	".\..\gui.h"\
	".\..\display.h"\
	".\..\draw.h"\
	".\..\state.h"\
	".\..\window.h"\
	".\..\gui2.h"\
	".\..\display2.h"\
	".\..\draw2.h"\
	".\..\state2.h"\
	".\..\event.h"\
	".\..\input.h"\
	".\..\vi.h"\
	".\..\regexp.h"\
	".\..\ex.h"\
	".\..\move.h"\
	".\..\vicmd.h"\
	".\..\operator.h"\
	".\..\cut.h"\
	".\..\elvisio.h"\
	".\..\lp.h"\
	".\..\calc.h"\
	".\..\more.h"\
	".\..\digraph.h"\
	".\..\need.h"\
	".\..\misc.h"\
	
NODEP_CPP_CTAGS=\
	".\..\osdir.c"\
	

"$(INTDIR)\ctags.obj" : $(SOURCE) $(DEP_CPP_CTAGS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wintags.rc

"$(INTDIR)\wintags.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\wintools.c
DEP_CPP_WINTO=\
	".\wintools.h"\
	

"$(INTDIR)\wintools.obj" : $(SOURCE) $(DEP_CPP_WINTO) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
