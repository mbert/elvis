# Microsoft Visual C++ generated build script - Do not modify

PROJ = FMT
DEBUG = 0
PROGTYPE = 6
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = .\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = FMT.C       
FIRSTCPP =             
RC = rc
CFLAGS_D_DEXE = /nologo /G2 /Zi /Od /D "_DEBUG" /D "_DOS" /I "osmsdos" /I "." /FR /Fd"FMT.PDB"
CFLAGS_R_DEXE = /nologo /Gs /G2 /Ox /D "NDEBUG" /D "_DOS" /I "osmsdos" /I "." 
LFLAGS_D_DEXE = /NOLOGO /ONERROR:NOEXE /NOI /CO /STACK:5120
LFLAGS_R_DEXE = /NOLOGO /ONERROR:NOEXE /NOI /STACK:5120
LIBS_D_DEXE = oldnames slibce
LIBS_R_DEXE = oldnames slibce
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_DEXE)
LFLAGS = $(LFLAGS_D_DEXE)
LIBS = $(LIBS_D_DEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_DEXE)
LFLAGS = $(LFLAGS_R_DEXE)
LIBS = $(LIBS_R_DEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = FMT.SBR


FMT_DEP = .\config.h \
	.\elvis.h \
	.\elvctype.h \
	.\version.h \
	.\safe.h \
	.\options.h \
	.\optglob.h \
	.\session.h \
	.\lowbuf.h \
	.\message.h \
	.\buffer.h \
	.\mark.h \
	.\buffer2.h \
	.\options2.h \
	.\scan.h \
	.\opsys.h \
	.\map.h \
	.\gui.h \
	.\display.h \
	.\draw.h \
	.\state.h \
	.\window.h \
	.\gui2.h \
	.\display2.h \
	.\draw2.h \
	.\state2.h \
	.\event.h \
	.\input.h \
	.\vi.h \
	.\regexp.h \
	.\ex.h \
	.\move.h \
	.\vicmd.h \
	.\operator.h \
	.\cut.h \
	.\elvisio.h \
	.\lp.h \
	.\calc.h \
	.\more.h \
	.\digraph.h \
	.\tag.h \
	.\tagsrch.h \
	.\tagelvis.h \
	.\need.h \
	.\misc.h


all:	$(PROJ).EXE

FMT.OBJ:	FMT.C $(FMT_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c FMT.C

$(PROJ).EXE::	FMT.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
FMT.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
d:\msvc15\lib\+
d:\msvc15\mfc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
