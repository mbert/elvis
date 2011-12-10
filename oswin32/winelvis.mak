# This is a hand-made Makefile.  MSVC++ can't read it, but NMAKE can.
###############################################################################
# Macro definitions

RSC=rc.exe
CPP=cl.exe
LINK32=link.exe
INTDIR=GuiRel
CFLAGS=/nologo /ML /W1 /GX /O2 /I "." /I ".." /I "oswin32" /I "..\oswin32" \
 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GUI_WIN32" /Fo"$(INTDIR)/" /c 
CPP_OBJS=$(INTDIR)/
RSC_PROJ=/l 0x409 /fo"..\$(INTDIR)\winelvis.res" /d "NDEBUG" 
HDRS=buffer.h buffer2.h calc.h config.h cut.h digraph.h display.h display2.h \
 draw.h draw2.h elvctype.h elvis.h elvisio.h event.h ex.h gui.h gui2.h \
 input.h lowbuf.h lp.h mark.h message.h misc.h more.h move.h need.h \
 operator.h opsys.h optglob.h options.h options2.h oswin32\osdef.h regexp.h \
 safe.h scan.h session.h state.h state2.h version.h vi.h vicmd.h window.h
GUIHDRS=guiwin32\winelvis.h guiwin32\wintools.h
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /incremental:no /machine:I386\
 /out:"WinElvis.exe" 
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
	"$(INTDIR)/osblock.obj" \
	"$(INTDIR)/osdir.obj" \
	"$(INTDIR)/osprg.obj" \
	"$(INTDIR)/ostext.obj" \
	"$(INTDIR)/buffer.obj" \
	"$(INTDIR)/calc.obj" \
	"$(INTDIR)/cut.obj" \
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
	"$(INTDIR)/exmake.obj" \
	"$(INTDIR)/gui.obj" \
	"$(INTDIR)/guicurs.obj" \
	"$(INTDIR)/guiopen.obj" \
	"$(INTDIR)/guitcap.obj" \
	"$(INTDIR)/guix11.obj" \
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
	"$(INTDIR)/regsub.obj" \
	"$(INTDIR)/safe.obj" \
	"$(INTDIR)/scan.obj" \
	"$(INTDIR)/search.obj" \
	"$(INTDIR)/session.obj" \
	"$(INTDIR)/state.obj" \
	"$(INTDIR)/vi.obj" \
	"$(INTDIR)/vicmd.obj" \
	"$(INTDIR)/window.obj" \
	"$(INTDIR)/winelvis.obj" \
	"$(INTDIR)/wintools.obj" \
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
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
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


$(INTDIR)\ostext.obj : oswin32\ostext.c $(HDRS)

$(INTDIR)\osdir.obj : oswin32\osdir.c $(HDRS)

$(INTDIR)\osprg.obj : oswin32\osprg.c $(HDRS)

$(INTDIR)\osblock.obj : oswin32\osblock.c $(HDRS)


$(INTDIR)\buffer.obj : buffer.c $(HDR)

$(INTDIR)\calc.obj : calc.c $(HDR)

$(INTDIR)\cut.obj : cut.c $(HDR)

$(INTDIR)\digraph.obj : digraph.c $(HDR)

$(INTDIR)\display.obj : display.c $(HDR)

$(INTDIR)\dmhex.obj : dmhex.c $(HDR)

$(INTDIR)\dmmarkup.obj : dmmarkup.c $(HDR)

$(INTDIR)\dmnormal.obj : dmnormal.c $(HDR)

$(INTDIR)\dmsyntax.obj : dmsyntax.c $(HDR)

$(INTDIR)\draw.obj : draw.c $(HDR)

$(INTDIR)\event.obj : event.c $(HDR)

$(INTDIR)\ex.obj : ex.c $(HDR)

$(INTDIR)\exaction.obj : exaction.c $(HDR)

$(INTDIR)\exconfig.obj : exconfig.c $(HDR)

$(INTDIR)\exedit.obj : exedit.c $(HDR)

$(INTDIR)\exmake.obj : exmake.c $(HDR)

$(INTDIR)\gui.obj : gui.c $(HDR)

$(INTDIR)\guicurs.obj : guicurs.c $(HDR)

$(INTDIR)\guiopen.obj : guiopen.c $(HDR)

$(INTDIR)\guitcap.obj : guitcap.c $(HDR)

$(INTDIR)\guix11.obj : guix11.c $(HDR)

$(INTDIR)\input.obj : input.c $(HDR)

$(INTDIR)\io.obj : io.c $(HDR)

$(INTDIR)\lowbuf.obj : lowbuf.c $(HDR)

$(INTDIR)\lp.obj : lp.c $(HDR)

$(INTDIR)\lpescape.obj : lpescape.c $(HDR)

$(INTDIR)\lpovrtyp.obj : lpovrtyp.c $(HDR)

$(INTDIR)\lpps.obj : lpps.c $(HDR)

$(INTDIR)\main.obj : main.c $(HDR)

$(INTDIR)\map.obj : map.c $(HDR)

$(INTDIR)\mark.obj : mark.c $(HDR)

$(INTDIR)\message.obj : message.c $(HDR)

$(INTDIR)\misc.obj : misc.c $(HDR)

$(INTDIR)\more.obj : more.c $(HDR)

$(INTDIR)\move.obj : move.c $(HDR)

$(INTDIR)\need.obj : need.c $(HDR)

$(INTDIR)\operator.obj : operator.c $(HDR)

$(INTDIR)\optglob.obj : optglob.c $(HDR)

$(INTDIR)\options.obj : options.c $(HDR)

$(INTDIR)\regexp.obj : regexp.c $(HDR)

$(INTDIR)\regsub.obj : regsub.c $(HDR)

$(INTDIR)\safe.obj : safe.c $(HDR)

$(INTDIR)\scan.obj : scan.c $(HDR)

$(INTDIR)\search.obj : search.c $(HDR)

$(INTDIR)\session.obj : session.c $(HDR)

$(INTDIR)\state.obj : state.c $(HDR)

$(INTDIR)\vi.obj : vi.c $(HDR)

$(INTDIR)\vicmd.obj : vicmd.c $(HDR)

$(INTDIR)\window.obj : window.c $(HDR)

################################################################################
# Resources

$(INTDIR)\winelvis.res : guiwin32\winelvis.rc $(DEP_RSC_WINELV) \
	guiwin32\elvis.ico guiwin32\cursor1.cur guiwin32\toolbar.bmp
   cd guiwin32
   $(RSC) $(RSC_PROJ) winelvis.rc
   cd ..
