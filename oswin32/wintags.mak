# hand-made NMAKE File.

# Check if the compiler supports single threaded statically linked CRT (-ML),
# and if so, use it. Newer compilers don't, so fall back to -MT (statically
# linked multithreaded.)
!IF [$(CC) -ML 2>&1 | find "D9002" >NUL]==0
CRTFLAG_RELEASE=/MT
!ELSE
CRTFLAG_RELEASE=/ML
!ENDIF

RC=rc.exe
LD=link.exe
INTDIR=.\GuiRel
INCL=/I "." /I ".." /I "oswin32" /I "..\oswin32" /I "guiwin32"
C_DEFINES=/D _CRT_SECURE_NO_WARNINGS=1 /D _CRT_NONSTDC_NO_WARNINGS=1 /D WIN32
CFLAGS=/nologo $(CRTFLAG_RELEASE) /W3 /O2 /D "NDEBUG" $(C_DEFINES) \
    /D "_WINDOWS" /D "GUI_WIN32" /Fo"$(INTDIR)/" $(INCL)
LDFLAGS=kernel32.lib user32.lib /nologo /subsystem:windows /incremental:no
RCFLAGS=/l 0x409 /fo"$(INTDIR)/wintags.res" /d "NDEBUG" 
OBJS=$(INTDIR)\wintags.obj $(INTDIR)\wintools.obj $(INTDIR)\ctags.obj \
	$(INTDIR)\tag.obj $(INTDIR)\safe.obj $(INTDIR)\wintags.res
HDRS=elvis.h guiwin32\wintools.h config.h elvctype.h version.h oswin32\osdef.h \
	safe.h options.h optglob.h session.h lowbuf.h buffer.h mark.h \
	buffer2.h options2.h scan.h message.h opsys.h gui.h display.h draw.h \
	state.h window.h gui2.h display2.h draw2.h state2.h event.h input.h \
	vi.h regexp.h ex.h move.h vicmd.h operator.h cut.h elvisio.h lp.h \
	calc.h more.h digraph.h need.h misc.h
	
################################################################################

all: $(INTDIR) WinTags.exe

$(INTDIR) : 
	if not exist $(INTDIR)/nul mkdir $(INTDIR)

WinTags.exe: $(OBJS)
	$(LD) @<<
  $(LDFLAGS) /out:WinTags.exe $(OBJS)
<<

$(INTDIR)\ctags.obj: ctags.c $(HDRS) $(INTDIR)
	$(CC) $(CFLAGS) /c ctags.c

$(INTDIR)\tag.obj: tag.c $(HDRS) $(INTDIR)
	$(CC) $(CFLAGS) /c tag.c

$(INTDIR)\safe.obj: safe.c $(HDRS) $(INTDIR)
	$(CC) $(CFLAGS) /c safe.c

$(INTDIR)\wintags.obj: guiwin32\wintags.c $(HDRS) $(INTDIR)
	$(CC) $(CFLAGS) /c guiwin32\wintags.c

$(INTDIR)\wintools.obj: guiwin32\wintools.c guiwin32\wintools.h $(INTDIR)
	$(CC) $(CFLAGS) /c guiwin32\wintools.c

$(INTDIR)\wintags.res: guiwin32\wintags.rc $(INTDIR)
	$(RC) $(RCFLAGS) guiwin32\wintags.rc
