# Check if the compiler supports single threaded statically linked CRT (-ML),
# and if so, use it. Newer compilers don't, so fall back to -MT (statically
# linked multithreaded.)
!IF [$(CC) -ML 2>&1 | find "D9002" >NUL]==0
CRTFLAG_RELEASE=/MT
!ELSE
CRTFLAG_RELEASE=/ML
!ENDIF

RSC=rc.exe
OUTDIR=.
INTDIR=.\WinRel

C_DEFINES=/D _CRT_SECURE_NO_WARNINGS=1 /D _CRT_NONSTDC_NO_WARNINGS=1 /D WIN32

ALL : $(INTDIR) $(OUTDIR)\ctags.exe $(OUTDIR)\fmt.exe $(OUTDIR)\ref.exe\
 $(OUTDIR)\ls.exe $(OUTDIR)\vi.exe $(OUTDIR)\ex.exe $(OUTDIR)\view.exe

$(INTDIR) : 
	if not exist $(INTDIR)\nul mkdir $(INTDIR)

$(OUTDIR) : 
	if not exist $(OUTDIR)\nul mkdir $(OUTDIR)

CPP_PROJ=/nologo $(CRTFLAG_RELEASE) /W3 /O2 /I "oswin32" /I "." $(C_DEFINES) \
 /D "NDEBUG" /D "_CONSOLE" /FR$(INTDIR)/ /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/

LINK32=link.exe
LINK32_FLAGS=kernel32.lib /NOLOGO /SUBSYSTEM:console /INCREMENTAL:no
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
	$(CC) /nologo /DARGV0=VI /Fe$(OUTDIR)\vi.exe /Fo$(INTDIR)\vi.obj /Ioswin32 alias.c

$(OUTDIR)\ex.exe : $(OUTDIR)
	$(CC) /nologo /DARGV0=EX /Fe$(OUTDIR)\ex.exe /Fo$(INTDIR)\ex.obj /Ioswin32 alias.c

$(OUTDIR)\view.exe : $(OUTDIR)
	$(CC) /nologo /DARGV0=VIEW /Fe$(OUTDIR)\view.exe /Fo$(INTDIR)\view.obj /Ioswin32 alias.c


###############################################################################

.c{$(CPP_OBJS)}.obj:
   $(CC) $(CPP_PROJ) $<  

{oswin32/}.c{$(CPP_OBJS)}.obj:
   $(CC) $(CPP_PROJ) $<  

################################################################################

$(INTDIR)\ctags.obj :  ctags.c

$(INTDIR)\fmt.obj :  fmt.c

$(INTDIR)\ref.obj :  ref.c $(DEF_FILE)

$(INTDIR)\tag.obj :  tag.c

$(INTDIR)\tagsrch.obj :  tagsrch.c

$(INTDIR)\safe.obj :  safe.c

$(INTDIR)\ls.obj :  oswin32\ls.c

