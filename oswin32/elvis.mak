# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "elvis.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "."
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.
INTDIR=.\WinRel

ALL : "elvis.exe" ".\WinRel\elvis.bsc"

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /ML /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FR /c
# ADD CPP /nologo /ML /W1 /GX /YX /O2 /I "oswin32" /I "." /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FR /c
CPP_PROJ=/nologo /ML /W1 /GX /YX /O2 /I "oswin32" /I "." /D "WIN32" /D "NDEBUG"\
 /D "_CONSOLE" /FR$(INTDIR)/ /Fp$(OUTDIR)/"elvis.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"elvis.bsc" 
BSC32_SBRS= \
	".\WinRel\move.sbr" \
	".\WinRel\search.sbr" \
	".\WinRel\lp.sbr" \
	".\WinRel\guitcap.sbr" \
	".\WinRel\display.sbr" \
	".\WinRel\more.sbr" \
	".\WinRel\digraph.sbr" \
	".\WinRel\state.sbr" \
	".\WinRel\osprg.sbr" \
	".\WinRel\guiopen.sbr" \
	".\WinRel\tcaphelp.sbr" \
	".\WinRel\gui.sbr" \
	".\WinRel\vicmd.sbr" \
	".\WinRel\misc.sbr" \
	".\WinRel\operator.sbr" \
	".\WinRel\exaction.sbr" \
	".\WinRel\regexp.sbr" \
	".\WinRel\lpescape.sbr" \
	".\WinRel\input.sbr" \
	".\WinRel\dmnormal.sbr" \
	".\WinRel\dmsyntax.sbr" \
	".\WinRel\options.sbr" \
	".\WinRel\calc.sbr" \
	".\WinRel\ostext.sbr" \
	".\WinRel\vi.sbr" \
	".\WinRel\need.sbr" \
	".\WinRel\window.sbr" \
	".\WinRel\io.sbr" \
	".\WinRel\osdir.sbr" \
	".\WinRel\safe.sbr" \
	".\WinRel\regsub.sbr" \
	".\WinRel\ex.sbr" \
	".\WinRel\buffer.sbr" \
	".\WinRel\event.sbr" \
	".\WinRel\session.sbr" \
	".\WinRel\guicurs.sbr" \
	".\WinRel\dmmarkup.sbr" \
	".\WinRel\mark.sbr" \
	".\WinRel\optglob.sbr" \
	".\WinRel\lpps.sbr" \
	".\WinRel\guix11.sbr" \
	".\WinRel\message.sbr" \
	".\WinRel\dmhex.sbr" \
	".\WinRel\cut.sbr" \
	".\WinRel\exconfig.sbr" \
	".\WinRel\exmake.sbr" \
	".\WinRel\tinytcap.sbr" \
	".\WinRel\lpovrtyp.sbr" \
	".\WinRel\main.sbr" \
	".\WinRel\exedit.sbr" \
	".\WinRel\lowbuf.sbr" \
	".\WinRel\draw.sbr" \
	".\WinRel\scan.sbr" \
	".\WinRel\map.sbr" \
	".\WinRel\osblock.sbr"

".\WinRel\elvis.bsc" : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:console /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:console /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /NOLOGO /SUBSYSTEM:console /INCREMENTAL:no\
 /PDB:$(OUTDIR)/"elvis.pdb" /MACHINE:I386 /OUT:$(OUTDIR)/"elvis.exe" 
DEF_FILE=
LINK32_OBJS= \
	".\WinRel\move.obj" \
	".\WinRel\search.obj" \
	".\WinRel\lp.obj" \
	".\WinRel\guitcap.obj" \
	".\WinRel\display.obj" \
	".\WinRel\more.obj" \
	".\WinRel\digraph.obj" \
	".\WinRel\state.obj" \
	".\WinRel\osprg.obj" \
	".\WinRel\guiopen.obj" \
	".\WinRel\tcaphelp.obj" \
	".\WinRel\gui.obj" \
	".\WinRel\vicmd.obj" \
	".\WinRel\misc.obj" \
	".\WinRel\operator.obj" \
	".\WinRel\exaction.obj" \
	".\WinRel\regexp.obj" \
	".\WinRel\lpescape.obj" \
	".\WinRel\input.obj" \
	".\WinRel\dmnormal.obj" \
	".\WinRel\dmsyntax.obj" \
	".\WinRel\options.obj" \
	".\WinRel\calc.obj" \
	".\WinRel\ostext.obj" \
	".\WinRel\vi.obj" \
	".\WinRel\need.obj" \
	".\WinRel\window.obj" \
	".\WinRel\io.obj" \
	".\WinRel\osdir.obj" \
	".\WinRel\safe.obj" \
	".\WinRel\regsub.obj" \
	".\WinRel\ex.obj" \
	".\WinRel\buffer.obj" \
	".\WinRel\event.obj" \
	".\WinRel\session.obj" \
	".\WinRel\guicurs.obj" \
	".\WinRel\dmmarkup.obj" \
	".\WinRel\mark.obj" \
	".\WinRel\optglob.obj" \
	".\WinRel\lpps.obj" \
	".\WinRel\guix11.obj" \
	".\WinRel\message.obj" \
	".\WinRel\dmhex.obj" \
	".\WinRel\cut.obj" \
	".\WinRel\exconfig.obj" \
	".\WinRel\exmake.obj" \
	".\WinRel\tinytcap.obj" \
	".\WinRel\lpovrtyp.obj" \
	".\WinRel\main.obj" \
	".\WinRel\exedit.obj" \
	".\WinRel\lowbuf.obj" \
	".\WinRel\draw.obj" \
	".\WinRel\scan.obj" \
	".\WinRel\map.obj" \
	".\WinRel\osblock.obj"

"elvis.exe" : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : ".\WinDebug\elvis.exe" ".\WinDebug\elvis.bsc"

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /ML /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /c
# ADD CPP /nologo /ML /W3 /GX /Zi /YX /Od /I "oswin32" /I "." /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /c
CPP_PROJ=/nologo /ML /W3 /GX /Zi /YX /Od /I "oswin32" /I "." /D "WIN32" /D\
 "_DEBUG" /D "_CONSOLE" /FR$(INTDIR)/ /Fp$(OUTDIR)/"elvis.pch" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"elvis.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"elvis.bsc" 
BSC32_SBRS= \
	".\WinDebug\move.sbr" \
	".\WinDebug\search.sbr" \
	".\WinDebug\lp.sbr" \
	".\WinDebug\guitcap.sbr" \
	".\WinDebug\display.sbr" \
	".\WinDebug\more.sbr" \
	".\WinDebug\digraph.sbr" \
	".\WinDebug\state.sbr" \
	".\WinDebug\osprg.sbr" \
	".\WinDebug\guiopen.sbr" \
	".\WinDebug\tcaphelp.sbr" \
	".\WinDebug\gui.sbr" \
	".\WinDebug\vicmd.sbr" \
	".\WinDebug\misc.sbr" \
	".\WinDebug\operator.sbr" \
	".\WinDebug\exaction.sbr" \
	".\WinDebug\regexp.sbr" \
	".\WinDebug\lpescape.sbr" \
	".\WinDebug\input.sbr" \
	".\WinDebug\dmnormal.sbr" \
	".\WinDebug\dmsyntax.sbr" \
	".\WinDebug\options.sbr" \
	".\WinDebug\calc.sbr" \
	".\WinDebug\ostext.sbr" \
	".\WinDebug\vi.sbr" \
	".\WinDebug\need.sbr" \
	".\WinDebug\window.sbr" \
	".\WinDebug\io.sbr" \
	".\WinDebug\osdir.sbr" \
	".\WinDebug\safe.sbr" \
	".\WinDebug\regsub.sbr" \
	".\WinDebug\ex.sbr" \
	".\WinDebug\buffer.sbr" \
	".\WinDebug\event.sbr" \
	".\WinDebug\session.sbr" \
	".\WinDebug\guicurs.sbr" \
	".\WinDebug\dmmarkup.sbr" \
	".\WinDebug\mark.sbr" \
	".\WinDebug\optglob.sbr" \
	".\WinDebug\lpps.sbr" \
	".\WinDebug\guix11.sbr" \
	".\WinDebug\message.sbr" \
	".\WinDebug\dmhex.sbr" \
	".\WinDebug\cut.sbr" \
	".\WinDebug\exconfig.sbr" \
	".\WinDebug\exmake.sbr" \
	".\WinDebug\tinytcap.sbr" \
	".\WinDebug\lpovrtyp.sbr" \
	".\WinDebug\main.sbr" \
	".\WinDebug\exedit.sbr" \
	".\WinDebug\lowbuf.sbr" \
	".\WinDebug\draw.sbr" \
	".\WinDebug\scan.sbr" \
	".\WinDebug\map.sbr" \
	".\WinDebug\osblock.sbr"

".\WinDebug\elvis.bsc" : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:console /DEBUG /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:console /DEBUG /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /NOLOGO /SUBSYSTEM:console /INCREMENTAL:yes\
 /PDB:$(OUTDIR)/"elvis.pdb" /DEBUG /MACHINE:I386 /OUT:$(OUTDIR)/"elvis.exe" 
DEF_FILE=
LINK32_OBJS= \
	".\WinDebug\move.obj" \
	".\WinDebug\search.obj" \
	".\WinDebug\lp.obj" \
	".\WinDebug\guitcap.obj" \
	".\WinDebug\display.obj" \
	".\WinDebug\more.obj" \
	".\WinDebug\digraph.obj" \
	".\WinDebug\state.obj" \
	".\WinDebug\osprg.obj" \
	".\WinDebug\guiopen.obj" \
	".\WinDebug\tcaphelp.obj" \
	".\WinDebug\gui.obj" \
	".\WinDebug\vicmd.obj" \
	".\WinDebug\misc.obj" \
	".\WinDebug\operator.obj" \
	".\WinDebug\exaction.obj" \
	".\WinDebug\regexp.obj" \
	".\WinDebug\lpescape.obj" \
	".\WinDebug\input.obj" \
	".\WinDebug\dmnormal.obj" \
	".\WinDebug\dmsyntax.obj" \
	".\WinDebug\options.obj" \
	".\WinDebug\calc.obj" \
	".\WinDebug\ostext.obj" \
	".\WinDebug\vi.obj" \
	".\WinDebug\need.obj" \
	".\WinDebug\window.obj" \
	".\WinDebug\io.obj" \
	".\WinDebug\osdir.obj" \
	".\WinDebug\safe.obj" \
	".\WinDebug\regsub.obj" \
	".\WinDebug\ex.obj" \
	".\WinDebug\buffer.obj" \
	".\WinDebug\event.obj" \
	".\WinDebug\session.obj" \
	".\WinDebug\guicurs.obj" \
	".\WinDebug\dmmarkup.obj" \
	".\WinDebug\mark.obj" \
	".\WinDebug\optglob.obj" \
	".\WinDebug\lpps.obj" \
	".\WinDebug\guix11.obj" \
	".\WinDebug\message.obj" \
	".\WinDebug\dmhex.obj" \
	".\WinDebug\cut.obj" \
	".\WinDebug\exconfig.obj" \
	".\WinDebug\exmake.obj" \
	".\WinDebug\tinytcap.obj" \
	".\WinDebug\lpovrtyp.obj" \
	".\WinDebug\main.obj" \
	".\WinDebug\exedit.obj" \
	".\WinDebug\lowbuf.obj" \
	".\WinDebug\draw.obj" \
	".\WinDebug\scan.obj" \
	".\WinDebug\map.obj" \
	".\WinDebug\osblock.obj"

".\WinDebug\elvis.exe" : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\move.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\move.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\move.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\search.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\search.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\search.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lp.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\lp.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\lp.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\guitcap.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\guitcap.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\guitcap.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\display.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\display.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\display.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\more.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\more.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\more.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\digraph.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\digraph.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\digraph.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\state.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\state.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\state.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\oswin32\osprg.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\osprg.obj" :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\osprg.obj" :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\guiopen.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\guiopen.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\guiopen.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\oswin32\tcaphelp.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\tcaphelp.obj" :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\tcaphelp.obj" :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gui.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\gui.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\gui.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\vicmd.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\vicmd.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\vicmd.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\misc.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\misc.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\misc.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\operator.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\operator.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\operator.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\exaction.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\exaction.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\exaction.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\regexp.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\regexp.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\regexp.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lpescape.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\lpescape.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\lpescape.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\input.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\input.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\input.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dmnormal.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\dmnormal.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\dmnormal.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dmsyntax.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\dmsyntax.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\dmsyntax.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\options.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\options.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\options.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\calc.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\calc.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\calc.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\oswin32\ostext.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\ostext.obj" :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\ostext.obj" :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\vi.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\vi.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\vi.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\need.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\need.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\need.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\window.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\window.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\window.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\io.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\io.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\io.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\oswin32\osdir.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\osdir.obj" :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\osdir.obj" :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\safe.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\safe.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\safe.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\regsub.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\regsub.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\regsub.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ex.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\ex.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\ex.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\buffer.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\buffer.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\buffer.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\event.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\event.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\event.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\session.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\session.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\session.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\guicurs.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\guicurs.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\guicurs.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dmmarkup.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\dmmarkup.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\dmmarkup.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mark.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\mark.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\mark.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\optglob.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\optglob.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\optglob.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lpps.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\lpps.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\lpps.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\guix11.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\guix11.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\guix11.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\message.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\message.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\message.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dmhex.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\dmhex.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\dmhex.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cut.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\cut.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\cut.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\exconfig.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\exconfig.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\exconfig.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\exmake.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\exmake.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\exmake.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tinytcap.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\tinytcap.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\tinytcap.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lpovrtyp.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\lpovrtyp.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\lpovrtyp.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\main.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\main.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\main.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\exedit.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\exedit.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\exedit.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lowbuf.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\lowbuf.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\lowbuf.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\draw.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\draw.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\draw.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\scan.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\scan.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\scan.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\map.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\map.obj" :  $(SOURCE)  $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\map.obj" :  $(SOURCE)  $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\oswin32\osblock.c

!IF  "$(CFG)" == "Win32 Release"

".\WinRel\osblock.obj" :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

".\WinDebug\osblock.obj" :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
# End Group
# End Project
################################################################################
