# This is a hand-made Makefile.  MSVC++ can't read it, but NMAKE can.
###############################################################################

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

# Macro definitions

RSC=rc.exe
CPP=cl.exe
LD=link.exe
C_DEFINES=/D _CRT_SECURE_NO_WARNINGS=1 /D _CRT_NONSTDC_NO_WARNINGS=1 /D WIN32
!IF "$(CFG)" == "WinElvis - Win32 Release"
INTDIR=GuiRel
CFLAGS=/nologo $(CRTFLAG_RELEASE) /W1 /O2 /I "." /I ".." /I "oswin32" \
 /I "..\oswin32" /D "NDEBUG" $(C_DEFINES) /D "_WINDOWS" /D "GUI_WIN32" \
 /Fo"$(INTDIR)/" /c 
RSC_PROJ=/l 0x409 /fo"..\$(INTDIR)\winelvis.res" /d "NDEBUG" 
LDFLAGS=/nologo /subsystem:windows /incremental:no /out:"WinElvis.exe" 
!ELSE
INTDIR=GuiDebug
CFLAGS=/nologo $(CRTFLAG_DEBUG) /W3 /Z7 /Od /I "oswin32" /I "." $(C_DEFINES) \
 /D "_DEBUG" /D "_WINDOWS" /D "GUI_WIN32" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
RSC_PROJ=/l 0x409 /fo"..\$(INTDIR)\winelvis.res"
LDFLAGS=/nologo /subsystem:windows /incremental:no /pdb:"elvis.pdb" \
 /debug /out:"WinElvis.exe" 
!ENDIF
CPP_OBJS=$(INTDIR)/
HDRS=autocmd.h buffer.h buffer2.h calc.h color.h config.h cut.h descr.h \
 digraph.h display.h display2.h draw.h draw2.h elvctype.h elvis.h elvisio.h \
 event.h ex.h fold.h gui.h gui2.h input.h lowbuf.h lp.h mark.h message.h \
 misc.h more.h move.h need.h operator.h opsys.h optglob.h options.h options2.h \
 oswin32\osdef.h regexp.h region.h safe.h scan.h session.h spell.h state.h \
 state2.h tag.h version.h vi.h vicmd.h window.h
GUIHDRS=guiwin32\winelvis.h guiwin32\wintools.h
LIBS=kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib \
 wsock32.lib
LINK32_OBJS= \
	"$(INTDIR)/guiwin.obj" \
	"$(INTDIR)/gwcmd.obj" \
	"$(INTDIR)/gwdlgs.obj" \
	"$(INTDIR)/gwmsg.obj" \
	"$(INTDIR)/gwopts.obj" \
	"$(INTDIR)/gwprint.obj" \
	"$(INTDIR)/gwstatb.obj" \
	"$(INTDIR)/gwtoolb.obj" \
	"$(INTDIR)/gwutil.obj" \
	"$(INTDIR)/gwimage.obj" \
	"$(INTDIR)/osblock.obj" \
	"$(INTDIR)/osdir.obj" \
	"$(INTDIR)/osprg.obj" \
	"$(INTDIR)/ostext.obj" \
	"$(INTDIR)/autocmd.obj" \
	"$(INTDIR)/buffer.obj" \
	"$(INTDIR)/calc.obj" \
	"$(INTDIR)/color.obj" \
	"$(INTDIR)/cut.obj" \
	"$(INTDIR)/descr.obj" \
	"$(INTDIR)/digraph.obj" \
	"$(INTDIR)/display.obj" \
	"$(INTDIR)/dmhex.obj" \
	"$(INTDIR)/dmmarkup.obj" \
	"$(INTDIR)/dmnormal.obj" \
	"$(INTDIR)/dmsyntax.obj" \
	"$(INTDIR)/draw.obj" \
	"$(INTDIR)/event.obj" \
	"$(INTDIR)/ex.obj" \
	"$(INTDIR)/exaction.obj" \
	"$(INTDIR)/exconfig.obj" \
	"$(INTDIR)/exedit.obj" \
	"$(INTDIR)/exsubst.obj" \
	"$(INTDIR)/exmake.obj" \
	"$(INTDIR)/fold.obj" \
	"$(INTDIR)/gui.obj" \
	"$(INTDIR)/input.obj" \
	"$(INTDIR)/io.obj" \
	"$(INTDIR)/lowbuf.obj" \
	"$(INTDIR)/lp.obj" \
	"$(INTDIR)/lpescape.obj" \
	"$(INTDIR)/lpovrtyp.obj" \
	"$(INTDIR)/lpps.obj" \
	"$(INTDIR)/main.obj" \
	"$(INTDIR)/map.obj" \
	"$(INTDIR)/mark.obj" \
	"$(INTDIR)/message.obj" \
	"$(INTDIR)/misc.obj" \
	"$(INTDIR)/more.obj" \
	"$(INTDIR)/move.obj" \
	"$(INTDIR)/need.obj" \
	"$(INTDIR)/operator.obj" \
	"$(INTDIR)/optglob.obj" \
	"$(INTDIR)/options.obj" \
	"$(INTDIR)/regexp.obj" \
	"$(INTDIR)/region.obj" \
	"$(INTDIR)/regsub.obj" \
	"$(INTDIR)/safe.obj" \
	"$(INTDIR)/scan.obj" \
	"$(INTDIR)/search.obj" \
	"$(INTDIR)/session.obj" \
	"$(INTDIR)/spell.obj" \
	"$(INTDIR)/state.obj" \
	"$(INTDIR)/tag.obj" \
	"$(INTDIR)/tagelvis.obj" \
	"$(INTDIR)/tagsrch.obj" \
	"$(INTDIR)/vi.obj" \
	"$(INTDIR)/vicmd.obj" \
	"$(INTDIR)/window.obj" \
	"$(INTDIR)/winelvis.obj" \
	"$(INTDIR)/wintools.obj" \
	"$(INTDIR)/osnet.obj" \
	"$(INTDIR)/url.obj" \
	"$(INTDIR)/http.obj" \
	"$(INTDIR)/ftp.obj" \
	"$(INTDIR)/winelvis.res"

################################################################################
# Standard Rules

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CFLAGS) $<  

{guiwin32/}.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CFLAGS) $<  

{oswin32/}.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CFLAGS) $<  

###############################################################################
# Targets

ALL : $(INTDIR) WinElvis.exe


WinElvis.exe : $(DEF_FILE) $(LINK32_OBJS)
    $(LD) @<<
  $(LIBS) $(LDFLAGS) $(LINK32_OBJS)
<<

$(INTDIR) :
	mkdir $(INTDIR)

################################################################################
# Header dependencies

$(INTDIR)\wintools.obj : guiwin32\wintools.c guiwin32\wintools.h

$(INTDIR)\winelvis.obj : guiwin32\winelvis.c $(HDRS) $(GUIHDRS)

$(INTDIR)\gwopts.obj : guiwin32\gwopts.c $(HDRS) guiwin32\winelvis.h

$(INTDIR)\gwprint.obj : guiwin32\gwprint.c $(HDRS) guiwin32\winelvis.h

$(INTDIR)\guiwin.obj : guiwin32\guiwin.c $(HDRS) guiwin32\winelvis.h

$(INTDIR)\gwmsg.obj : guiwin32\gwmsg.c $(HDRS) guiwin32\winelvis.h

$(INTDIR)\gwutil.obj : guiwin32\gwutil.c $(HDRS) guiwin32\winelvis.h

$(INTDIR)\gwstatb.obj : guiwin32\gwstatb.c $(HDRS) guiwin32\winelvis.h

$(INTDIR)\gwcmd.obj : guiwin32\gwcmd.c $(HDRS) guiwin32\winelvis.h

$(INTDIR)\gwdlgs.obj : guiwin32\gwdlgs.c $(HDRS) $(GUIHDRS)

$(INTDIR)\gwtoolb.obj : guiwin32\gwtoolb.c $(HDRS) guiwin32\winelvis.h

$(INTDIR)\gwimage.obj : guiwin32\gwimage.c $(HDRS) guiwin32\winelvis.h


$(INTDIR)\ostext.obj : oswin32\ostext.c $(HDRS)

$(INTDIR)\osdir.obj : oswin32\osdir.c $(HDRS)

$(INTDIR)\osprg.obj : oswin32\osprg.c $(HDRS)

$(INTDIR)\osblock.obj : oswin32\osblock.c $(HDRS)

$(INTDIR)\osnet.obj : oswin32\osnet.c $(HDRS)


$(INTDIR)\autocmd.obj : autocmd.c $(HDRS)

$(INTDIR)\buffer.obj : buffer.c $(HDRS)

$(INTDIR)\calc.obj : calc.c $(HDRS)

$(INTDIR)\color.obj : color.c $(HDRS)

$(INTDIR)\cut.obj : cut.c $(HDRS)

$(INTDIR)\descr.obj : descr.c $(HDRS)

$(INTDIR)\digraph.obj : digraph.c $(HDRS)

$(INTDIR)\display.obj : display.c $(HDRS)

$(INTDIR)\dmhex.obj : dmhex.c $(HDRS)

$(INTDIR)\dmmarkup.obj : dmmarkup.c $(HDRS)

$(INTDIR)\dmnormal.obj : dmnormal.c $(HDRS)

$(INTDIR)\dmsyntax.obj : dmsyntax.c $(HDRS)

$(INTDIR)\draw.obj : draw.c $(HDRS)

$(INTDIR)\event.obj : event.c $(HDRS)

$(INTDIR)\ex.obj : ex.c $(HDRS)

$(INTDIR)\exaction.obj : exaction.c $(HDRS)

$(INTDIR)\exconfig.obj : exconfig.c $(HDRS)

$(INTDIR)\exedit.obj : exedit.c $(HDRS)

$(INTDIR)\exsubst.obj : exsubst.c $(HDRS)

$(INTDIR)\exmake.obj : exmake.c $(HDRS)

$(INTDIR)\fold.obj : fold.c $(HDRS)

$(INTDIR)\ftp.obj : ftp.c $(HDRS)

$(INTDIR)\gui.obj : gui.c $(HDRS)

$(INTDIR)\http.obj : http.c $(HDRS)

$(INTDIR)\input.obj : input.c $(HDRS)

$(INTDIR)\io.obj : io.c $(HDRS)

$(INTDIR)\lowbuf.obj : lowbuf.c $(HDRS)

$(INTDIR)\lp.obj : lp.c $(HDRS)

$(INTDIR)\lpescape.obj : lpescape.c $(HDRS)

$(INTDIR)\lpovrtyp.obj : lpovrtyp.c $(HDRS)

$(INTDIR)\lpps.obj : lpps.c $(HDRS)

$(INTDIR)\main.obj : main.c $(HDRS)

$(INTDIR)\map.obj : map.c $(HDRS)

$(INTDIR)\mark.obj : mark.c $(HDRS)

$(INTDIR)\message.obj : message.c $(HDRS)

$(INTDIR)\misc.obj : misc.c $(HDRS)

$(INTDIR)\more.obj : more.c $(HDRS)

$(INTDIR)\move.obj : move.c $(HDRS)

$(INTDIR)\need.obj : need.c $(HDRS)

$(INTDIR)\operator.obj : operator.c $(HDRS)

$(INTDIR)\optglob.obj : optglob.c $(HDRS)

$(INTDIR)\options.obj : options.c $(HDRS)

$(INTDIR)\regexp.obj : regexp.c $(HDRS)

$(INTDIR)\region.obj : region.c $(HDRS)

$(INTDIR)\regsub.obj : regsub.c $(HDRS)

$(INTDIR)\safe.obj : safe.c $(HDRS)

$(INTDIR)\scan.obj : scan.c $(HDRS)

$(INTDIR)\search.obj : search.c $(HDRS)

$(INTDIR)\session.obj : session.c $(HDRS)

$(INTDIR)\spell.obj : spell.c $(HDRS)

$(INTDIR)\state.obj : state.c $(HDRS)

$(INTDIR)\tag.obj : tag.c $(HDRS)

$(INTDIR)\tagelvis.obj : tagelvis.c $(HDRS)

$(INTDIR)\tagsrch.obj : tagsrch.c $(HDRS)

$(INTDIR)\url.obj : url.c $(HDRS)

$(INTDIR)\vi.obj : vi.c $(HDRS)

$(INTDIR)\vicmd.obj : vicmd.c $(HDRS)

$(INTDIR)\window.obj : window.c $(HDRS)

################################################################################
# Resources

$(INTDIR)\winelvis.res : guiwin32\winelvis.rc $(DEP_RSC_WINELV) \
	guiwin32\elvis.ico guiwin32\cursor1.cur guiwin32\toolbar.bmp
   cd guiwin32
   $(RSC) $(RSC_PROJ) winelvis.rc
   cd ..
