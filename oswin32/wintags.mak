# hand-made NMAKE File.
CC=cl.exe
RC=rc.exe
LD=link.exe
INCL=/I "." /I ".." /I "oswin32" /I "..\oswin32" /I "guiwin32"
CFLAGS=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" \
	/D "GUI_WIN32" /Fo"$(INTDIR)/" $(INCL)
LDFLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
	advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
	/subsystem:windows /incremental:no /machine:I386
RCFLAGS=/l 0x409 /fo"$(INTDIR)/wintags.res" /d "NDEBUG" 
INTDIR=.\GuiRel
OBJS=$(INTDIR)\wintags.obj $(INTDIR)\wintools.obj $(INTDIR)\ctags.obj \
	$(INTDIR)\tag.obj $(INTDIR)\safe.obj $(INTDIR)\wintags.res
HDRS=elvis.h guiwin32\wintools.h config.h elvctype.h version.h oswin32\osdef.h \
	safe.h options.h optglob.h session.h lowbuf.h buffer.h mark.h \
	buffer2.h options2.h scan.h message.h opsys.h gui.h display.h draw.h \
	state.h window.h gui2.h display2.h draw2.h state2.h event.h input.h \
	vi.h regexp.h ex.h move.h vicmd.h operator.h cut.h elvisio.h lp.h \
	calc.h more.h digraph.h need.h misc.h
	
################################################################################

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
