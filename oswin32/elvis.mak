# Microsoft Developer Studio Generated NMAKE File, Format Version 40001
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=elvis - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to elvis - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "elvis - Win32 Release" && "$(CFG)" != "elvis - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "elvis.mak" CFG="elvis - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "elvis - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "elvis - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

# Check if the compiler supports single threaded statically linked CRT (-ML),
# and if so, use it. Newer compilers don't, so fall back to -MT (statically
# linked multithreaded.)
!IF [$(CC) -ML 2>&1 | find "D9002" >NUL]==0
CRTFLAG_RELEASE=/MT
CRTFLAG_DEBUG=/MTd
!ELSE
CRTFLAG_RELEASE=/ML
!IF [$(CC) -MLd 2>&1 | find "D4002" >NUL]==0
CRTFLAG_DEBUG=/ML
!ELSE
CRTFLAG_DEBUG=/MLd
!ENDIF
!ENDIF

################################################################################
# Begin Project
# PROP Target_Last_Scanned "elvis - Win32 Debug"
CPP=cl.exe
RSC=rc.exe

C_DEFINES=/D _CRT_SECURE_NO_WARNINGS=1 /D _CRT_NONSTDC_NO_WARNINGS=1 /D WIN32

!IF  "$(CFG)" == "elvis - Win32 Release"

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

ALL : $(INTDIR) "$(OUTDIR)\elvis.exe"

CLEAN : 
	-@erase ".\WinRel\elvis.exe"
	-@erase ".\WinRel\vi.obj"
	-@erase ".\WinRel\display.obj"
	-@erase ".\WinRel\guitcap.obj"
	-@erase ".\WinRel\need.obj"
	-@erase ".\WinRel\tinytcap.obj"
	-@erase ".\WinRel\draw.obj"
	-@erase ".\WinRel\guicurs.obj"
	-@erase ".\WinRel\cut.obj"
	-@erase ".\WinRel\regexp.obj"
	-@erase ".\WinRel\region.obj"
	-@erase ".\WinRel\url.obj"
	-@erase ".\WinRel\options.obj"
	-@erase ".\WinRel\dmnormal.obj"
	-@erase ".\WinRel\lpps.obj"
	-@erase ".\WinRel\misc.obj"
	-@erase ".\WinRel\lp.obj"
	-@erase ".\WinRel\exmake.obj"
	-@erase ".\WinRel\safe.obj"
	-@erase ".\WinRel\ostext.obj"
	-@erase ".\WinRel\dmsyntax.obj"
	-@erase ".\WinRel\io.obj"
	-@erase ".\WinRel\digraph.obj"
	-@erase ".\WinRel\state.obj"
	-@erase ".\WinRel\search.obj"
	-@erase ".\WinRel\exedit.obj"
	-@erase ".\WinRel\exsubst.obj"
	-@erase ".\WinRel\input.obj"
	-@erase ".\WinRel\dmmarkup.obj"
	-@erase ".\WinRel\tcaphelp.obj"
	-@erase ".\WinRel\move.obj"
	-@erase ".\WinRel\tagsrch.obj"
	-@erase ".\WinRel\ex.obj"
	-@erase ".\WinRel\regsub.obj"
	-@erase ".\WinRel\operator.obj"
	-@erase ".\WinRel\osnet.obj"
	-@erase ".\WinRel\http.obj"
	-@erase ".\WinRel\more.obj"
	-@erase ".\WinRel\map.obj"
	-@erase ".\WinRel\gui.obj"
	-@erase ".\WinRel\lowbuf.obj"
	-@erase ".\WinRel\vicmd.obj"
	-@erase ".\WinRel\buffer.obj"
	-@erase ".\WinRel\tagelvis.obj"
	-@erase ".\WinRel\exaction.obj"
	-@erase ".\WinRel\main.obj"
	-@erase ".\WinRel\event.obj"
	-@erase ".\WinRel\lpovrtyp.obj"
	-@erase ".\WinRel\calc.obj"
	-@erase ".\WinRel\osprg.obj"
	-@erase ".\WinRel\window.obj"
	-@erase ".\WinRel\scan.obj"
	-@erase ".\WinRel\tag.obj"
	-@erase ".\WinRel\session.obj"
	-@erase ".\WinRel\spell.obj"
	-@erase ".\WinRel\optglob.obj"
	-@erase ".\WinRel\guiopen.obj"
	-@erase ".\WinRel\mark.obj"
	-@erase ".\WinRel\lpescape.obj"
	-@erase ".\WinRel\osblock.obj"
	-@erase ".\WinRel\osdir.obj"
	-@erase ".\WinRel\message.obj"
	-@erase ".\WinRel\dmhex.obj"
	-@erase ".\WinRel\exconfig.obj"
	-@erase ".\WinRel\fold.obj"
	-@erase ".\WinRel\ftp.obj"
	-@erase ".\WinRel\color.obj"
	-@erase ".\WinRel\descr.obj"
	-@erase ".\WinRel\autocmd.obj"

"$(INTDIR)" : 
	if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo $(CRTFLAG_RELEASE) /O2 /I "oswin32" /I "." $(C_DEFINES) \
 /D "NDEBUG" /D "_CONSOLE" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\WinRel/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/elvis.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib wsock32.lib user32.lib /nologo /subsystem:console
# ADD LINK32 wsock32.lib kernel32.lib user32.lib /nologo /subsystem:console
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib /nologo /subsystem:console\
 /incremental:no /pdb:"$(OUTDIR)/elvis.pdb" /out:"$(OUTDIR)/elvis.exe"
LINK32_OBJS= \
	"$(INTDIR)/vi.obj" \
	"$(INTDIR)/display.obj" \
	"$(INTDIR)/guitcap.obj" \
	"$(INTDIR)/need.obj" \
	"$(INTDIR)/tinytcap.obj" \
	"$(INTDIR)/draw.obj" \
	"$(INTDIR)/guicurs.obj" \
	"$(INTDIR)/cut.obj" \
	"$(INTDIR)/regexp.obj" \
	"$(INTDIR)/region.obj" \
	"$(INTDIR)/url.obj" \
	"$(INTDIR)/options.obj" \
	"$(INTDIR)/dmnormal.obj" \
	"$(INTDIR)/lpps.obj" \
	"$(INTDIR)/misc.obj" \
	"$(INTDIR)/lp.obj" \
	"$(INTDIR)/exmake.obj" \
	"$(INTDIR)/safe.obj" \
	"$(INTDIR)/ostext.obj" \
	"$(INTDIR)/dmsyntax.obj" \
	"$(INTDIR)/io.obj" \
	"$(INTDIR)/digraph.obj" \
	"$(INTDIR)/state.obj" \
	"$(INTDIR)/search.obj" \
	"$(INTDIR)/exedit.obj" \
	"$(INTDIR)/exsubst.obj" \
	"$(INTDIR)/input.obj" \
	"$(INTDIR)/dmmarkup.obj" \
	"$(INTDIR)/tcaphelp.obj" \
	"$(INTDIR)/move.obj" \
	"$(INTDIR)/tagsrch.obj" \
	"$(INTDIR)/ex.obj" \
	"$(INTDIR)/regsub.obj" \
	"$(INTDIR)/operator.obj" \
	"$(INTDIR)/osnet.obj" \
	"$(INTDIR)/http.obj" \
	"$(INTDIR)/more.obj" \
	"$(INTDIR)/map.obj" \
	"$(INTDIR)/gui.obj" \
	"$(INTDIR)/lowbuf.obj" \
	"$(INTDIR)/vicmd.obj" \
	"$(INTDIR)/buffer.obj" \
	"$(INTDIR)/tagelvis.obj" \
	"$(INTDIR)/exaction.obj" \
	"$(INTDIR)/main.obj" \
	"$(INTDIR)/event.obj" \
	"$(INTDIR)/lpovrtyp.obj" \
	"$(INTDIR)/calc.obj" \
	"$(INTDIR)/osprg.obj" \
	"$(INTDIR)/window.obj" \
	"$(INTDIR)/scan.obj" \
	"$(INTDIR)/tag.obj" \
	"$(INTDIR)/session.obj" \
	"$(INTDIR)/spell.obj" \
	"$(INTDIR)/optglob.obj" \
	"$(INTDIR)/guiopen.obj" \
	"$(INTDIR)/mark.obj" \
	"$(INTDIR)/lpescape.obj" \
	"$(INTDIR)/osblock.obj" \
	"$(INTDIR)/osdir.obj" \
	"$(INTDIR)/message.obj" \
	"$(INTDIR)/dmhex.obj" \
	"$(INTDIR)/exconfig.obj" \
	"$(INTDIR)/fold.obj" \
	"$(INTDIR)/ftp.obj" \
	"$(INTDIR)/color.obj" \
	"$(INTDIR)/descr.obj" \
	"$(INTDIR)/autocmd.obj"

"$(OUTDIR)\elvis.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "elvis - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.
INTDIR=.\WinDebug

ALL : $(INTDIR) "$(OUTDIR)\elvis.exe"

CLEAN : 
	-@erase ".\WinDebug\vc40.pdb"
	-@erase ".\WinDebug\vc40.idb"
	-@erase ".\WinDebug\elvis.exe"
	-@erase ".\WinDebug\dmmarkup.obj"
	-@erase ".\WinDebug\tcaphelp.obj"
	-@erase ".\WinDebug\message.obj"
	-@erase ".\WinDebug\operator.obj"
	-@erase ".\WinDebug\lowbuf.obj"
	-@erase ".\WinDebug\buffer.obj"
	-@erase ".\WinDebug\event.obj"
	-@erase ".\WinDebug\display.obj"
	-@erase ".\WinDebug\need.obj"
	-@erase ".\WinDebug\guitcap.obj"
	-@erase ".\WinDebug\gui.obj"
	-@erase ".\WinDebug\draw.obj"
	-@erase ".\WinDebug\window.obj"
	-@erase ".\WinDebug\state.obj"
	-@erase ".\WinDebug\guicurs.obj"
	-@erase ".\WinDebug\vi.obj"
	-@erase ".\WinDebug\main.obj"
	-@erase ".\WinDebug\scan.obj"
	-@erase ".\WinDebug\options.obj"
	-@erase ".\WinDebug\cut.obj"
	-@erase ".\WinDebug\lpps.obj"
	-@erase ".\WinDebug\tagelvis.obj"
	-@erase ".\WinDebug\exmake.obj"
	-@erase ".\WinDebug\exaction.obj"
	-@erase ".\WinDebug\lpovrtyp.obj"
	-@erase ".\WinDebug\misc.obj"
	-@erase ".\WinDebug\osnet.obj"
	-@erase ".\WinDebug\guiopen.obj"
	-@erase ".\WinDebug\safe.obj"
	-@erase ".\WinDebug\digraph.obj"
	-@erase ".\WinDebug\vicmd.obj"
	-@erase ".\WinDebug\lpescape.obj"
	-@erase ".\WinDebug\lp.obj"
	-@erase ".\WinDebug\move.obj"
	-@erase ".\WinDebug\io.obj"
	-@erase ".\WinDebug\exconfig.obj"
	-@erase ".\WinDebug\tagsrch.obj"
	-@erase ".\WinDebug\osprg.obj"
	-@erase ".\WinDebug\regexp.obj"
	-@erase ".\WinDebug\region.obj"
	-@erase ".\WinDebug\http.obj"
	-@erase ".\WinDebug\more.obj"
	-@erase ".\WinDebug\ex.obj"
	-@erase ".\WinDebug\tinytcap.obj"
	-@erase ".\WinDebug\osdir.obj"
	-@erase ".\WinDebug\map.obj"
	-@erase ".\WinDebug\url.obj"
	-@erase ".\WinDebug\ostext.obj"
	-@erase ".\WinDebug\dmnormal.obj"
	-@erase ".\WinDebug\dmhex.obj"
	-@erase ".\WinDebug\calc.obj"
	-@erase ".\WinDebug\input.obj"
	-@erase ".\WinDebug\search.obj"
	-@erase ".\WinDebug\exedit.obj"
	-@erase ".\WinDebug\exsubst.obj"
	-@erase ".\WinDebug\session.obj"
	-@erase ".\WinDebug\spell.obj"
	-@erase ".\WinDebug\optglob.obj"
	-@erase ".\WinDebug\tag.obj"
	-@erase ".\WinDebug\mark.obj"
	-@erase ".\WinDebug\regsub.obj"
	-@erase ".\WinDebug\dmsyntax.obj"
	-@erase ".\WinDebug\osblock.obj"
	-@erase ".\WinDebug\fold.obj"
	-@erase ".\WinDebug\ftp.obj"
	-@erase ".\WinDebug\color.obj"
	-@erase ".\WinDebug\descr.obj"
	-@erase ".\WinDebug\autocmd.obj"
	-@erase ".\WinDebug\elvis.ilk"
	-@erase ".\WinDebug\elvis.pdb"

"$(INTDIR)" : 
	if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo $(CRTFLAG_DEBUG) /W3 /Z7 /Od /I "oswin32" /I "." $(C_DEFINES)\
 /D "_DEBUG" /D "_CONSOLE" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\WinDebug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/elvis.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib wsock32.lib user32.lib /nologo /subsystem:console /debug
# ADD LINK32 wsock32.lib kernel32.lib user32.lib /nologo /subsystem:console /debug
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib /nologo /subsystem:console\
 /incremental:no /pdb:"$(OUTDIR)/elvis.pdb" /debug /out:"$(OUTDIR)/elvis.exe"
LINK32_OBJS= \
	"$(INTDIR)/dmmarkup.obj" \
	"$(INTDIR)/tcaphelp.obj" \
	"$(INTDIR)/message.obj" \
	"$(INTDIR)/operator.obj" \
	"$(INTDIR)/lowbuf.obj" \
	"$(INTDIR)/buffer.obj" \
	"$(INTDIR)/event.obj" \
	"$(INTDIR)/display.obj" \
	"$(INTDIR)/need.obj" \
	"$(INTDIR)/guitcap.obj" \
	"$(INTDIR)/gui.obj" \
	"$(INTDIR)/draw.obj" \
	"$(INTDIR)/window.obj" \
	"$(INTDIR)/state.obj" \
	"$(INTDIR)/guicurs.obj" \
	"$(INTDIR)/vi.obj" \
	"$(INTDIR)/main.obj" \
	"$(INTDIR)/scan.obj" \
	"$(INTDIR)/options.obj" \
	"$(INTDIR)/cut.obj" \
	"$(INTDIR)/lpps.obj" \
	"$(INTDIR)/tagelvis.obj" \
	"$(INTDIR)/exmake.obj" \
	"$(INTDIR)/exaction.obj" \
	"$(INTDIR)/lpovrtyp.obj" \
	"$(INTDIR)/misc.obj" \
	"$(INTDIR)/osnet.obj" \
	"$(INTDIR)/guiopen.obj" \
	"$(INTDIR)/safe.obj" \
	"$(INTDIR)/digraph.obj" \
	"$(INTDIR)/vicmd.obj" \
	"$(INTDIR)/lpescape.obj" \
	"$(INTDIR)/lp.obj" \
	"$(INTDIR)/move.obj" \
	"$(INTDIR)/io.obj" \
	"$(INTDIR)/exconfig.obj" \
	"$(INTDIR)/tagsrch.obj" \
	"$(INTDIR)/osprg.obj" \
	"$(INTDIR)/regexp.obj" \
	"$(INTDIR)/region.obj" \
	"$(INTDIR)/http.obj" \
	"$(INTDIR)/more.obj" \
	"$(INTDIR)/ex.obj" \
	"$(INTDIR)/tinytcap.obj" \
	"$(INTDIR)/osdir.obj" \
	"$(INTDIR)/map.obj" \
	"$(INTDIR)/url.obj" \
	"$(INTDIR)/ostext.obj" \
	"$(INTDIR)/dmnormal.obj" \
	"$(INTDIR)/dmhex.obj" \
	"$(INTDIR)/calc.obj" \
	"$(INTDIR)/input.obj" \
	"$(INTDIR)/search.obj" \
	"$(INTDIR)/exedit.obj" \
	"$(INTDIR)/exsubst.obj" \
	"$(INTDIR)/session.obj" \
	"$(INTDIR)/spell.obj" \
	"$(INTDIR)/optglob.obj" \
	"$(INTDIR)/tag.obj" \
	"$(INTDIR)/mark.obj" \
	"$(INTDIR)/regsub.obj" \
	"$(INTDIR)/dmsyntax.obj" \
	"$(INTDIR)/osblock.obj" \
	"$(INTDIR)/fold.obj" \
	"$(INTDIR)/ftp.obj" \
	"$(INTDIR)/color.obj" \
	"$(INTDIR)/descr.obj" \
	"$(INTDIR)/autocmd.obj"

"$(OUTDIR)\elvis.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "elvis - Win32 Release"
# Name "elvis - Win32 Debug"

!IF  "$(CFG)" == "elvis - Win32 Release"

!ELSEIF  "$(CFG)" == "elvis - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\move.c
DEP_CPP_MOVE_=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\move.obj" : $(SOURCE) $(DEP_CPP_MOVE_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\search.c
DEP_CPP_SEARC=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\search.obj" : $(SOURCE) $(DEP_CPP_SEARC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\lp.c
DEP_CPP_LP_C4=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\lp.obj" : $(SOURCE) $(DEP_CPP_LP_C4) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\guitcap.c
DEP_CPP_GUITC=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\guitcap.obj" : $(SOURCE) $(DEP_CPP_GUITC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\display.c
DEP_CPP_DISPL=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\display.obj" : $(SOURCE) $(DEP_CPP_DISPL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\more.c
DEP_CPP_MORE_=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\more.obj" : $(SOURCE) $(DEP_CPP_MORE_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\digraph.c
DEP_CPP_DIGRA=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\digraph.obj" : $(SOURCE) $(DEP_CPP_DIGRA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\state.c
DEP_CPP_STATE=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\state.obj" : $(SOURCE) $(DEP_CPP_STATE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\oswin32\osprg.c
DEP_CPP_OSPRG=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\osprg.obj" : $(SOURCE) $(DEP_CPP_OSPRG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\guiopen.c
DEP_CPP_GUIOP=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\guiopen.obj" : $(SOURCE) $(DEP_CPP_GUIOP) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\oswin32\tcaphelp.c
DEP_CPP_TCAPH=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\tcaphelp.obj" : $(SOURCE) $(DEP_CPP_TCAPH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\gui.c
DEP_CPP_GUI_C=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\gui.obj" : $(SOURCE) $(DEP_CPP_GUI_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\vicmd.c
DEP_CPP_VICMD=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\vicmd.obj" : $(SOURCE) $(DEP_CPP_VICMD) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\misc.c
DEP_CPP_MISC_=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\misc.obj" : $(SOURCE) $(DEP_CPP_MISC_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\operator.c
DEP_CPP_OPERA=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\operator.obj" : $(SOURCE) $(DEP_CPP_OPERA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\exaction.c
DEP_CPP_EXACT=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\exaction.obj" : $(SOURCE) $(DEP_CPP_EXACT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\regexp.c
DEP_CPP_REGEX=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\regexp.obj" : $(SOURCE) $(DEP_CPP_REGEX) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\region.c
DEP_CPP_REGEX=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\region.obj" : $(SOURCE) $(DEP_CPP_REGEX) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\lpescape.c
DEP_CPP_LPESC=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\lpescape.obj" : $(SOURCE) $(DEP_CPP_LPESC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\input.c
DEP_CPP_INPUT=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\input.obj" : $(SOURCE) $(DEP_CPP_INPUT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\dmnormal.c
DEP_CPP_DMNOR=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\dmnormal.obj" : $(SOURCE) $(DEP_CPP_DMNOR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\dmsyntax.c
DEP_CPP_DMSYN=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\dmsyntax.obj" : $(SOURCE) $(DEP_CPP_DMSYN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\options.c
DEP_CPP_OPTIO=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\options.obj" : $(SOURCE) $(DEP_CPP_OPTIO) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\calc.c
DEP_CPP_CALC_=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\calc.obj" : $(SOURCE) $(DEP_CPP_CALC_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\oswin32\ostext.c
DEP_CPP_OSTEX=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\ostext.obj" : $(SOURCE) $(DEP_CPP_OSTEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\vi.c
DEP_CPP_VI_C30=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\vi.obj" : $(SOURCE) $(DEP_CPP_VI_C30) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\need.c
DEP_CPP_NEED_=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\need.obj" : $(SOURCE) $(DEP_CPP_NEED_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\window.c
DEP_CPP_WINDO=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\window.obj" : $(SOURCE) $(DEP_CPP_WINDO) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\io.c
DEP_CPP_IO_C36=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\io.obj" : $(SOURCE) $(DEP_CPP_IO_C36) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\oswin32\osdir.c
DEP_CPP_OSDIR=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\osdir.obj" : $(SOURCE) $(DEP_CPP_OSDIR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\safe.c
DEP_CPP_SAFE_=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\safe.obj" : $(SOURCE) $(DEP_CPP_SAFE_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\regsub.c
DEP_CPP_REGSU=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\regsub.obj" : $(SOURCE) $(DEP_CPP_REGSU) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ex.c
DEP_CPP_EX_C3e=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\ex.obj" : $(SOURCE) $(DEP_CPP_EX_C3e) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\buffer.c
DEP_CPP_BUFFE=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\buffer.obj" : $(SOURCE) $(DEP_CPP_BUFFE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\event.c
DEP_CPP_EVENT=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\event.obj" : $(SOURCE) $(DEP_CPP_EVENT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\session.c
DEP_CPP_SESSI=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\session.obj" : $(SOURCE) $(DEP_CPP_SESSI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spell.c
DEP_CPP_SPELL=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\spell.obj" : $(SOURCE) $(DEP_CPP_SPELL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\guicurs.c
DEP_CPP_GUICU=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\guicurs.obj" : $(SOURCE) $(DEP_CPP_GUICU) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\dmmarkup.c
DEP_CPP_DMMAR=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\dmmarkup.obj" : $(SOURCE) $(DEP_CPP_DMMAR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\mark.c
DEP_CPP_MARK_=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\mark.obj" : $(SOURCE) $(DEP_CPP_MARK_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\optglob.c
DEP_CPP_OPTGL=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\optglob.obj" : $(SOURCE) $(DEP_CPP_OPTGL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\lpps.c
DEP_CPP_LPPS_=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\lpps.obj" : $(SOURCE) $(DEP_CPP_LPPS_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\message.c
DEP_CPP_MESSA=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\message.obj" : $(SOURCE) $(DEP_CPP_MESSA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\dmhex.c
DEP_CPP_DMHEX=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\dmhex.obj" : $(SOURCE) $(DEP_CPP_DMHEX) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\cut.c
DEP_CPP_CUT_C=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\cut.obj" : $(SOURCE) $(DEP_CPP_CUT_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\exconfig.c
DEP_CPP_EXCON=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\exconfig.obj" : $(SOURCE) $(DEP_CPP_EXCON) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\exmake.c
DEP_CPP_EXMAK=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\exmake.obj" : $(SOURCE) $(DEP_CPP_EXMAK) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tinytcap.c
DEP_CPP_TINYT=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\tinytcap.obj" : $(SOURCE) $(DEP_CPP_TINYT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\lpovrtyp.c
DEP_CPP_LPOVR=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\lpovrtyp.obj" : $(SOURCE) $(DEP_CPP_LPOVR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\main.c
DEP_CPP_MAIN_=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\exedit.c
DEP_CPP_EXEDI=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\exedit.obj" : $(SOURCE) $(DEP_CPP_EXEDI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\exsubst.c
DEP_CPP_EXEDI=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\exsubst.obj" : $(SOURCE) $(DEP_CPP_EXEDI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\lowbuf.c
DEP_CPP_LOWBU=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\lowbuf.obj" : $(SOURCE) $(DEP_CPP_LOWBU) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\draw.c
DEP_CPP_DRAW_=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\draw.obj" : $(SOURCE) $(DEP_CPP_DRAW_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\scan.c
DEP_CPP_SCAN_=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\scan.obj" : $(SOURCE) $(DEP_CPP_SCAN_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\map.c
DEP_CPP_MAP_C=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\map.obj" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\oswin32\osblock.c
DEP_CPP_OSBLO=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\osblock.obj" : $(SOURCE) $(DEP_CPP_OSBLO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tagsrch.c
DEP_CPP_TAGSR=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\tagsrch.obj" : $(SOURCE) $(DEP_CPP_TAGSR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tag.c
DEP_CPP_TAG_C=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\tag.obj" : $(SOURCE) $(DEP_CPP_TAG_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tagelvis.c
DEP_CPP_TAGEL=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\tagelvis.obj" : $(SOURCE) $(DEP_CPP_TAGEL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\oswin32\osnet.c
DEP_CPP_OSNET=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\osnet.obj" : $(SOURCE) $(DEP_CPP_OSNET) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\url.c
DEP_CPP_URL_C=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\url.obj" : $(SOURCE) $(DEP_CPP_URL_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\http.c
DEP_CPP_HTTP_=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\http.obj" : $(SOURCE) $(DEP_CPP_HTTP_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\fold.c
DEP_CPP_FTP_C=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\fold.obj" : $(SOURCE) $(DEP_CPP_FTP_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ftp.c
DEP_CPP_FTP_C=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\ftp.obj" : $(SOURCE) $(DEP_CPP_FTP_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\color.c
DEP_CPP_COLOR=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\color.obj" : $(SOURCE) $(DEP_CPP_FTP_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\descr.c
DEP_CPP_COLOR=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\descr.obj" : $(SOURCE) $(DEP_CPP_FTP_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\autocmd.c
DEP_CPP_COLOR=\
	".\elvis.h"\
	".\config.h"\
	".\elvctype.h"\
	".\version.h"\
	".\oswin32\osdef.h"\
	".\safe.h"\
	".\options.h"\
	".\optglob.h"\
	".\session.h"\
	".\spell.h"\
	".\lowbuf.h"\
	".\message.h"\
	".\buffer.h"\
	".\mark.h"\
	".\buffer2.h"\
	".\options2.h"\
	".\scan.h"\
	".\opsys.h"\
	".\map.h"\
	".\gui.h"\
	".\display.h"\
	".\draw.h"\
	".\state.h"\
	".\window.h"\
	".\gui2.h"\
	".\display2.h"\
	".\draw2.h"\
	".\state2.h"\
	".\event.h"\
	".\input.h"\
	".\vi.h"\
	".\regexp.h"\
	".\region.h"\
	".\ex.h"\
	".\fold.h"\
	".\move.h"\
	".\vicmd.h"\
	".\operator.h"\
	".\cut.h"\
	".\elvisio.h"\
	".\lp.h"\
	".\calc.h"\
	".\more.h"\
	".\digraph.h"\
	".\tag.h"\
	".\tagsrch.h"\
	".\tagelvis.h"\
	".\color.h"\
	".\descr.h"\
	".\autocmd.h"\
	".\need.h"\
	".\misc.h"\
	

"$(INTDIR)\autocmd.obj" : $(SOURCE) $(DEP_CPP_FTP_C) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
