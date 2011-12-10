CPP=cl.exe
RSC=rc.exe
OUTDIR=.
INTDIR=.\WinRel

ALL : $(OUTDIR)\ctags.exe $(OUTDIR)\fmt.exe $(OUTDIR)\ref.exe

$(OUTDIR) : 
    if not exist $(OUTDIR)\nul mkdir $(OUTDIR)

CPP_PROJ=/nologo /ML /W3 /GX /YX /O2 /I "oswin32" /I "." /D "WIN32" /D "NDEBUG"\
 /D "_CONSOLE" /FR$(INTDIR)/ /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /NOLOGO /SUBSYSTEM:console /INCREMENTAL:no\
 /MACHINE:I386
DEF_FILE=oswin32\osdir.c

###############################################################################

$(OUTDIR)/ctags.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)/ctags.obj
    $(LINK32) @<<
  $(LINK32_FLAGS) /OUT:$(OUTDIR)/ctags.exe $(INTDIR)/ctags.obj
<<

$(OUTDIR)/fmt.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)/fmt.obj
    $(LINK32) @<<
  $(LINK32_FLAGS) /OUT:$(OUTDIR)/fmt.exe $(INTDIR)/fmt.obj
<<

$(OUTDIR)/ref.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)/ref.obj
    $(LINK32) @<<
  $(LINK32_FLAGS) /OUT:$(OUTDIR)/ref.exe $(INTDIR)/ref.obj
<<

###############################################################################

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

################################################################################

$(INTDIR)\ctags.obj :  ctags.c

$(INTDIR)\fmt.obj :  fmt.c

$(INTDIR)\ref.obj :  ref.c
