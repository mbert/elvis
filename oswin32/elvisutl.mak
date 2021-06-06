CPP=cl.exe
RSC=rc.exe
OUTDIR=.
INTDIR=.\WinRel

ALL : $(OUTDIR)\ctags.exe $(OUTDIR)\fmt.exe $(OUTDIR)\ref.exe $(OUTDIR)\ls.exe\
 $(OUTDIR)\vi.exe $(OUTDIR)\ex.exe $(OUTDIR)\view.exe

$(OUTDIR) : 
    if not exist $(OUTDIR)\nul mkdir $(OUTDIR)

CPP_PROJ=/nologo /ML /W3 /GX /YX /O2 /I "oswin32" /I "." /D "WIN32" /D "NDEBUG"\
 /D "_CONSOLE" /FR$(INTDIR)/ /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/

LINK32=link.exe
LINK32_FLAGS=kernel32.lib /NOLOGO /SUBSYSTEM:console /INCREMENTAL:no\
 /MACHINE:I386
DEF_FILE=oswin32\osdir.c

###############################################################################

$(OUTDIR)/ctags.exe : $(OUTDIR) $(DEF_FILE) \
	$(INTDIR)\tag.obj \
	$(INTDIR)\safe.obj \
	$(INTDIR)\ctags.obj
    $(LINK32) @<<
  $(LINK32_FLAGS) /OUT:$(OUTDIR)/ctags.exe
	$(INTDIR)\tag.obj
	$(INTDIR)\safe.obj
	$(INTDIR)\ctags.obj
<<

$(OUTDIR)/fmt.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)/fmt.obj
    $(LINK32) @<<
  $(LINK32_FLAGS) /OUT:$(OUTDIR)/fmt.exe $(INTDIR)/fmt.obj
<<

$(OUTDIR)/ref.exe : $(OUTDIR) $(DEF_FILE) \
	$(INTDIR)\tag.obj \
	$(INTDIR)\tagsrch.obj \
	$(INTDIR)\safe.obj \
	$(INTDIR)\ref.obj
    $(LINK32) @<<
  $(LINK32_FLAGS) /OUT:$(OUTDIR)/ref.exe
	$(INTDIR)\tag.obj
	$(INTDIR)\tagsrch.obj
	$(INTDIR)\safe.obj
	$(INTDIR)\ref.obj
<<

$(OUTDIR)/ls.exe : $(OUTDIR) $(DEF_FILE) \
	$(INTDIR)\ls.obj \
	$(INTDIR)\safe.obj
    $(LINK32) @<<
  $(LINK32_FLAGS) /OUT:$(OUTDIR)/ls.exe
	$(INTDIR)\ls.obj
	$(INTDIR)\safe.obj
<<

$(OUTDIR)\vi.exe : $(OUTDIR)
	cl /nologo /DARGV0=VI /Fe$(OUTDIR)\vi.exe /Fo$(INTDIR)\vi.obj /Ioswin32 alias.c

$(OUTDIR)\ex.exe : $(OUTDIR)
	cl /nologo /DARGV0=EX /Fe$(OUTDIR)\ex.exe /Fo$(INTDIR)\ex.obj /Ioswin32 alias.c

$(OUTDIR)\view.exe : $(OUTDIR)
	cl /nologo /DARGV0=VIEW /Fe$(OUTDIR)\view.exe /Fo$(INTDIR)\view.obj /Ioswin32 alias.c


###############################################################################

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

{oswin32/}.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

################################################################################

$(INTDIR)\ctags.obj :  ctags.c

$(INTDIR)\fmt.obj :  fmt.c

$(INTDIR)\ref.obj :  ref.c $(DEF_FILE)

$(INTDIR)\tag.obj :  tag.c

$(INTDIR)\tagsrch.obj :  tagsrch.c

$(INTDIR)\safe.obj :  safe.c

$(INTDIR)\ls.obj :  oswin32\ls.c

