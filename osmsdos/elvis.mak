# Microsoft Visual C++ generated build script - Do not modify

PROJ = ELVIS
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
FIRSTC = BUFFER.C    
FIRSTCPP =             
RC = rc
CFLAGS_D_DEXE = /nologo /G2 /Zi /ALu /Od /D "_DEBUG" /D "_DOS" /I "osmsdos" /I "." /Fd"ELVIS.PDB"
CFLAGS_R_DEXE = /nologo /Gs /G2 /Gf /ALu /O2 /Ob1 /D "NDEBUG" /D "_DOS" /I "osmsdos" /I "." 
LFLAGS_D_DEXE = /NOLOGO /NOI /STACK:16384 /ONERROR:NOEXE /CO 
LFLAGS_R_DEXE = /NOLOGO /NOI /STACK:16384 /EXEPACK /ONERROR:NOEXE 
LIBS_D_DEXE = oldnames llibce 
LIBS_R_DEXE = oldnames llibce 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
OBJS_EXT = 
LIBS_EXT = OSMSDOS\DOEXEC.LIB 
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
SBRS = BUFFER.SBR \
		CALC.SBR \
		COLOR.SBR \
		CUT.SBR \
		DESCR.SBR \
		DIGRAPH.SBR \
		DISPLAY.SBR \
		DMHEX.SBR \
		DMMARKUP.SBR \
		DMNORMAL.SBR \
		DMSYNTAX.SBR \
		DRAW.SBR \
		EVENT.SBR \
		EX.SBR \
		EXACTION.SBR \
		EXCONFIG.SBR \
		EXEDIT.SBR \
		EXMAKE.SBR \
		EXSUBST.SBR \
		FOLD.SBR \
		GUI.SBR \
		GUICURS.SBR \
		GUIOPEN.SBR \
		GUITCAP.SBR \
		INPUT.SBR \
		IO.SBR \
		LOWBUF.SBR \
		LP.SBR \
		LPESCAPE.SBR \
		LPOVRTYP.SBR \
		LPPS.SBR \
		MAIN.SBR \
		MAP.SBR \
		MARK.SBR \
		MESSAGE.SBR \
		MISC.SBR \
		MORE.SBR \
		MOVE.SBR \
		NEED.SBR \
		OPERATOR.SBR \
		OPTGLOB.SBR \
		OPTIONS.SBR \
		REGEXP.SBR \
		REGSUB.SBR \
		REGION.SBR \
		SAFE.SBR \
		SCAN.SBR \
		SEARCH.SBR \
		SESSION.SBR \
		SPELL.SBR \
		STATE.SBR \
		TINYTCAP.SBR \
		URL.SBR \
		VI.SBR \
		VICMD.SBR \
		WINDOW.SBR \
		OSBLOCK.SBR \
		OSDIR.SBR \
		OSPRG.SBR \
		OSTEXT.SBR \
		TCAPHELP.SBR \
		TAG.SBR \
		TAGELVIS.SBR \
		TAGSRCH.SBR


DOEXEC_DEP = 

BUFFER_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


CALC_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


COLOR_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


CUT_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


DESCR_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


DIGRAPH_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


DISPLAY_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


DMHEX_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


DMMARKUP_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


DMNORMAL_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


DMSYNTAX_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


DRAW_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


EVENT_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


EX_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


EXACTION_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


EXCONFIG_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


EXEDIT_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


EXMAKE_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


EXSUBST_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


FOLD_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


GUI_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


GUICURS_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


GUIOPEN_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


GUITCAP_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


INPUT_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


IO_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


LOWBUF_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


LP_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


LPESCAPE_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


LPOVRTYP_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


LPPS_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


MAIN_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


MAP_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


MARK_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


MESSAGE_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


MISC_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


MORE_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


MOVE_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


NEED_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


OPERATOR_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


OPTGLOB_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


OPTIONS_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


REGEXP_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


REGION_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


REGSUB_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


SAFE_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


SCAN_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


SEARCH_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


SESSION_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


STATE_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


TINYTCAP_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


URL_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


VI_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


VICMD_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


WINDOW_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


OSPRG_DEP = .\osmsdos\doexec.h


TCAPHELP_DEP = .\osmsdos\pcvideo.h


TCAPHELP_DEP = .\osmsdos\mouse.c


TCAPHELP_DEP = .\osmsdos\mouse.h


TAG_DEP = .\elvis.h


TAG_DEP = .\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


TAGELVIS_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


TAGSRCH_DEP = .\elvis.h \
	.\config.h \
	.\version.h \
	.\elvctype.h \
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
	.\misc.h \
	.\fold.h \
	.\spell.h \
	.\region.h


all:	$(PROJ).EXE

BUFFER.OBJ:	BUFFER.C $(BUFFER_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c BUFFER.C

CALC.OBJ:	CALC.C $(CALC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c CALC.C

COLOR.OBJ:	COLOR.C $(COLOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c COLOR.C

CUT.OBJ:	CUT.C $(CUT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c CUT.C

DESCR.OBJ:	DESCR.C $(DESCR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DESCR.C

DIGRAPH.OBJ:	DIGRAPH.C $(DIGRAPH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DIGRAPH.C

DISPLAY.OBJ:	DISPLAY.C $(DISPLAY_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DISPLAY.C

DMHEX.OBJ:	DMHEX.C $(DMHEX_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DMHEX.C

DMMARKUP.OBJ:	DMMARKUP.C $(DMMARKUP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DMMARKUP.C

DMNORMAL.OBJ:	DMNORMAL.C $(DMNORMAL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DMNORMAL.C

DMSYNTAX.OBJ:	DMSYNTAX.C $(DMSYNTAX_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DMSYNTAX.C

DRAW.OBJ:	DRAW.C $(DRAW_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DRAW.C

EVENT.OBJ:	EVENT.C $(EVENT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c EVENT.C

EX.OBJ:	EX.C $(EX_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c EX.C

EXACTION.OBJ:	EXACTION.C $(EXACTION_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c EXACTION.C

EXCONFIG.OBJ:	EXCONFIG.C $(EXCONFIG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c EXCONFIG.C

EXEDIT.OBJ:	EXEDIT.C $(EXEDIT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c EXEDIT.C

EXMAKE.OBJ:	EXMAKE.C $(EXMAKE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c EXMAKE.C

EXSUBST.OBJ:	EXSUBST.C $(EXSUBST_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c EXSUBST.C

FOLD.OBJ:	FOLD.C $(FOLD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c FOLD.C

GUI.OBJ:	GUI.C $(GUI_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GUI.C

GUICURS.OBJ:	GUICURS.C $(GUICURS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GUICURS.C

GUIOPEN.OBJ:	GUIOPEN.C $(GUIOPEN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GUIOPEN.C

GUITCAP.OBJ:	GUITCAP.C $(GUITCAP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GUITCAP.C

INPUT.OBJ:	INPUT.C $(INPUT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c INPUT.C

IO.OBJ:	IO.C $(IO_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c IO.C

LOWBUF.OBJ:	LOWBUF.C $(LOWBUF_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c LOWBUF.C

LP.OBJ:	LP.C $(LP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c LP.C

LPESCAPE.OBJ:	LPESCAPE.C $(LPESCAPE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c LPESCAPE.C

LPOVRTYP.OBJ:	LPOVRTYP.C $(LPOVRTYP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c LPOVRTYP.C

LPPS.OBJ:	LPPS.C $(LPPS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c LPPS.C

MAIN.OBJ:	MAIN.C $(MAIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MAIN.C

MAP.OBJ:	MAP.C $(MAP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MAP.C

MARK.OBJ:	MARK.C $(MARK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MARK.C

MESSAGE.OBJ:	MESSAGE.C $(MESSAGE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MESSAGE.C

MISC.OBJ:	MISC.C $(MISC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MISC.C

MORE.OBJ:	MORE.C $(MORE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MORE.C

MOVE.OBJ:	MOVE.C $(MOVE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MOVE.C

NEED.OBJ:	NEED.C $(NEED_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c NEED.C

OPERATOR.OBJ:	OPERATOR.C $(OPERATOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c OPERATOR.C

OPTGLOB.OBJ:	OPTGLOB.C $(OPTGLOB_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c OPTGLOB.C

OPTIONS.OBJ:	OPTIONS.C $(OPTIONS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c OPTIONS.C

REGEXP.OBJ:	REGEXP.C $(REGEXP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c REGEXP.C

REGION.OBJ:	REGION.C $(REGION_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c REGION.C

REGSUB.OBJ:	REGSUB.C $(REGSUB_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c REGSUB.C

SAFE.OBJ:	SAFE.C $(SAFE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SAFE.C

SCAN.OBJ:	SCAN.C $(SCAN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SCAN.C

SEARCH.OBJ:	SEARCH.C $(SEARCH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SEARCH.C

SESSION.OBJ:	SESSION.C $(SESSION_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SESSION.C

SPELL.OBJ:	SPELL.C $(SPELL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SPELL.C

STATE.OBJ:	STATE.C $(STATE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c STATE.C

TINYTCAP.OBJ:	TINYTCAP.C $(TINYTCAP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TINYTCAP.C

URL.OBJ:	URL.C $(URL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c URL.C

VI.OBJ:	VI.C $(VI_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c VI.C

VICMD.OBJ:	VICMD.C $(VICMD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c VICMD.C

WINDOW.OBJ:	WINDOW.C $(WINDOW_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WINDOW.C

OSBLOCK.OBJ:	OSMSDOS\OSBLOCK.C $(OSBLOCK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c OSMSDOS\OSBLOCK.C

OSDIR.OBJ:	OSMSDOS\OSDIR.C $(OSDIR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c OSMSDOS\OSDIR.C

OSPRG.OBJ:	OSMSDOS\OSPRG.C $(OSPRG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c OSMSDOS\OSPRG.C

OSTEXT.OBJ:	OSMSDOS\OSTEXT.C $(OSTEXT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c OSMSDOS\OSTEXT.C

TCAPHELP.OBJ:	OSMSDOS\TCAPHELP.C $(TCAPHELP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c OSMSDOS\TCAPHELP.C

TAG.OBJ:	TAG.C $(TAG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TAG.C

TAGELVIS.OBJ:	TAGELVIS.C $(TAGELVIS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TAGELVIS.C

TAGSRCH.OBJ:	TAGSRCH.C $(TAGSRCH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TAGSRCH.C

$(PROJ).EXE::	BUFFER.OBJ CALC.OBJ COLOR.OBJ CUT.OBJ DESCR.OBJ DIGRAPH.OBJ DISPLAY.OBJ DMHEX.OBJ \
	DMMARKUP.OBJ DMNORMAL.OBJ DMSYNTAX.OBJ DRAW.OBJ EVENT.OBJ EX.OBJ EXACTION.OBJ EXCONFIG.OBJ \
	EXEDIT.OBJ EXMAKE.OBJ EXSUBST.OBJ GUI.OBJ GUICURS.OBJ GUIOPEN.OBJ GUITCAP.OBJ INPUT.OBJ \
	IO.OBJ LOWBUF.OBJ LP.OBJ LPESCAPE.OBJ LPOVRTYP.OBJ LPPS.OBJ MAIN.OBJ MAP.OBJ MARK.OBJ \
	MESSAGE.OBJ MISC.OBJ MORE.OBJ MOVE.OBJ NEED.OBJ OPERATOR.OBJ OPTGLOB.OBJ OPTIONS.OBJ \
	REGEXP.OBJ REGION.OBJ REGSUB.OBJ SAFE.OBJ SCAN.OBJ SEARCH.OBJ SESSION.OBJ SPELL.OBJ \
	STATE.OBJ TINYTCAP.OBJ URL.OBJ VI.OBJ VICMD.OBJ WINDOW.OBJ OSBLOCK.OBJ OSDIR.OBJ \
	OSPRG.OBJ OSTEXT.OBJ TCAPHELP.OBJ TAG.OBJ TAGELVIS.OBJ TAGSRCH.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
BUFFER.OBJ +
CALC.OBJ +
COLOR.OBJ +
CUT.OBJ +
DESCR.OBJ +
DIGRAPH.OBJ +
DISPLAY.OBJ +
DMHEX.OBJ +
DMMARKUP.OBJ +
DMNORMAL.OBJ +
DMSYNTAX.OBJ +
DRAW.OBJ +
EVENT.OBJ +
EX.OBJ +
EXACTION.OBJ +
EXCONFIG.OBJ +
EXEDIT.OBJ +
EXMAKE.OBJ +
EXSUBST.OBJ +
GUI.OBJ +
GUICURS.OBJ +
GUIOPEN.OBJ +
GUITCAP.OBJ +
INPUT.OBJ +
IO.OBJ +
LOWBUF.OBJ +
LP.OBJ +
LPESCAPE.OBJ +
LPOVRTYP.OBJ +
LPPS.OBJ +
MAIN.OBJ +
MAP.OBJ +
MARK.OBJ +
MESSAGE.OBJ +
MISC.OBJ +
MORE.OBJ +
MOVE.OBJ +
NEED.OBJ +
OPERATOR.OBJ +
OPTGLOB.OBJ +
OPTIONS.OBJ +
REGEXP.OBJ +
REGION.OBJ +
REGSUB.OBJ +
SAFE.OBJ +
SCAN.OBJ +
SEARCH.OBJ +
SESSION.OBJ +
SPELL.OBJ +
STATE.OBJ +
TINYTCAP.OBJ +
URL.OBJ +
VI.OBJ +
VICMD.OBJ +
WINDOW.OBJ +
OSBLOCK.OBJ +
OSDIR.OBJ +
OSPRG.OBJ +
OSTEXT.OBJ +
TCAPHELP.OBJ +
TAG.OBJ +
TAGELVIS.OBJ +
TAGSRCH.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
OSMSDOS\DOEXEC.LIB+
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
