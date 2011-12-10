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
PROJPATH = 
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
CFLAGS_R_DEXE = /nologo /Gs /G3 /ALu /O2 /Ob2 /D "NDEBUG" /D "_DOS" /I "osmsdos" /I "." /FR 
LFLAGS_D_DEXE = /NOLOGO /NOI /STACK:10240 /ONERROR:NOEXE /CO 
LFLAGS_R_DEXE = /NOLOGO /NOI /STACK:10240 /EXEPACK /ONERROR:NOEXE 
LIBS_D_DEXE = oldnames llibce 
LIBS_R_DEXE = oldnames llibce 
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
SBRS = BUFFER.SBR \
		CALC.SBR \
		CUT.SBR \
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
		SAFE.SBR \
		SCAN.SBR \
		SEARCH.SBR \
		SESSION.SBR \
		STATE.SBR \
		TINYTCAP.SBR \
		VI.SBR \
		VICMD.SBR \
		WINDOW.SBR \
		OSBLOCK.SBR \
		OSDIR.SBR \
		OSPRG.SBR \
		OSTEXT.SBR \
		TCAPHELP.SBR


BUFFER_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


CALC_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


CUT_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


DIGRAPH_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


DISPLAY_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


DMHEX_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


DMMARKUP_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


DMNORMAL_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


DMSYNTAX_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


DRAW_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


EVENT_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


EX_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


EXACTION_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


EXCONFIG_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


EXEDIT_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


EXMAKE_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


GUI_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


GUICURS_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


GUIOPEN_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


GUITCAP_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


INPUT_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


IO_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


LOWBUF_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


LP_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


LPESCAPE_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


LPOVRTYP_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


LPPS_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


MAIN_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


MAP_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


MARK_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


MESSAGE_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


MISC_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


MORE_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


MOVE_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


NEED_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


OPERATOR_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


OPTGLOB_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


OPTIONS_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


REGEXP_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


REGSUB_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


SAFE_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


SCAN_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


SEARCH_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


SESSION_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


STATE_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


TINYTCAP_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


VI_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


VICMD_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


WINDOW_DEP = elvis.h \
	config.h \
	version.h \
	safe.h \
	options.h \
	optglob.h \
	session.h \
	lowbuf.h \
	buffer.h \
	mark.h \
	buffer2.h \
	scan.h \
	message.h \
	opsys.h \
	map.h \
	gui.h \
	display.h \
	draw.h \
	state.h \
	window.h \
	gui2.h \
	display2.h \
	draw2.h \
	state2.h \
	event.h \
	input.h \
	vi.h \
	regexp.h \
	ex.h \
	move.h \
	vicmd.h \
	operator.h \
	cut.h \
	elvisio.h \
	lp.h \
	calc.h \
	more.h \
	digraph.h \
	need.h \
	misc.h


TCAPHELP_DEP = osmsdos\pcvideo.h


all:	$(PROJ).EXE

BUFFER.OBJ:	BUFFER.C $(BUFFER_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c BUFFER.C

CALC.OBJ:	CALC.C $(CALC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c CALC.C

CUT.OBJ:	CUT.C $(CUT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c CUT.C

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

STATE.OBJ:	STATE.C $(STATE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c STATE.C

TINYTCAP.OBJ:	TINYTCAP.C $(TINYTCAP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TINYTCAP.C

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

$(PROJ).EXE::	BUFFER.OBJ CALC.OBJ CUT.OBJ DIGRAPH.OBJ DISPLAY.OBJ DMHEX.OBJ \
	DMMARKUP.OBJ DMNORMAL.OBJ DMSYNTAX.OBJ DRAW.OBJ EVENT.OBJ EX.OBJ EXACTION.OBJ EXCONFIG.OBJ EXEDIT.OBJ \
	EXMAKE.OBJ GUI.OBJ GUICURS.OBJ GUIOPEN.OBJ GUITCAP.OBJ INPUT.OBJ IO.OBJ LOWBUF.OBJ \
	LP.OBJ LPESCAPE.OBJ LPOVRTYP.OBJ LPPS.OBJ MAIN.OBJ MAP.OBJ MARK.OBJ MESSAGE.OBJ MISC.OBJ \
	MORE.OBJ MOVE.OBJ NEED.OBJ OPERATOR.OBJ OPTGLOB.OBJ OPTIONS.OBJ REGEXP.OBJ REGSUB.OBJ \
	SAFE.OBJ SCAN.OBJ SEARCH.OBJ SESSION.OBJ STATE.OBJ TINYTCAP.OBJ VI.OBJ VICMD.OBJ \
	WINDOW.OBJ OSBLOCK.OBJ OSDIR.OBJ OSPRG.OBJ OSTEXT.OBJ TCAPHELP.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
BUFFER.OBJ +
CALC.OBJ +
CUT.OBJ +
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
REGSUB.OBJ +
SAFE.OBJ +
SCAN.OBJ +
SEARCH.OBJ +
SESSION.OBJ +
STATE.OBJ +
TINYTCAP.OBJ +
VI.OBJ +
VICMD.OBJ +
WINDOW.OBJ +
OSBLOCK.OBJ +
OSDIR.OBJ +
OSPRG.OBJ +
OSTEXT.OBJ +
TCAPHELP.OBJ +
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
