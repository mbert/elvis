/* oswin32/osconfig.h */

/* This file began a copy of the "oswin32\osconfig.h" file.  You should
 * modify "config.h" and leave "oswin32\osconfig.h" untouched.
 */

/* The following determine which user interfaces are to be supported */
#if !defined (GUI_WIN32)
#undef	GUI_X11		/* simple X-windows interface */
#undef	GUI_CURSES	/* curses interface */
#define	GUI_TERMCAP	/* termcap interface */
#define	GUI_OPEN	/* open-mode only, does nothing fancy */
#endif

/* The following control which display modes, network protocols, and other
 * features are available.  #define to enable, #undef to disable.
 */
#define	DISPLAY_SYNTAX	/* the generic "syntax" colorizer */
#define	DISPLAY_HTML	/* the "html" modes */
#define	DISPLAY_MAN	/* the "man" modes */
#define	DISPLAY_TEX	/* the "tex" modes */
#define	DISPLAY_HEX	/* the "hex" display mode */
#define	PROTOCOL_HTTP	/* the "http" network protocol */
#define	PROTOCOL_FTP	/* the "ftp" network protocol */
#define	FEATURE_ALIAS	/* the ":alias" command */
#define	FEATURE_ARRAY	/* subscripts in expressions */
#define	FEATURE_AUTOCMD	/* the :autocmd command */
#define	FEATURE_BACKTICK /* the `program` notation in file names */
#define	FEATURE_BROWSE	/* the :browse and :sbrowse commands */
#define	FEATURE_CACHEDESC /* store syntax/markup descriptions in RAM */
#define FEATURE_CALC	/* the calculator, and all cmds that use it */
#define	FEATURE_COMPLETE /* filename completion */
#define	FEATURE_EQUALTILDE /* :let option =~ excmdline */
#define	FEATURE_FOLD	/* the :fold and :unfold commands */
#define	FEATURE_G	/* most of the visual 'g' commands */
#define	FEATURE_HLOBJECT /* the hlobject and hllayers options */
#define	FEATURE_HLSEARCH  /* the hlsearch option */
#ifdef GUI_WIN32
#define FEATURE_IMAGE	/* gui can use background image */
#endif
#define	FEATURE_INCSEARCH /* the incsearch option */
#define FEATURE_LISTCHARS /* the listchars option */
#define	FEATURE_LITRE	/* faster searches for literal strings */
#define	FEATURE_LPR	/* the ":lpr" command */
#define	FEATURE_MAKE	/* the :make and :cc commands */
#define	FEATURE_MAPDB	/* the map debugger */
#define	FEATURE_MISC	/* lots of little things */
#define	FEATURE_MKEXRC	/* the ":mkexrc" command */
#define	FEATURE_NORMAL	/* vim-style :normal command */
#define	FEATURE_PERSIST	/* the persistfile option */
#define	FEATURE_PROTO	/* using aliases to add new protocols */
#undef	FEATURE_RAM	/* store edit buffers in RAM if "-f ram" */
#undef	FEATURE_RCSID	/* include RCS Id strings for all source files */
#define	FEATURE_REGION	/* the :region command */
#define	FEATURE_SHOWTAG	/* the "showtag" option */
#define	FEATURE_SMARTARGS /* show arguments when inputting a function call */	
#define	FEATURE_STDIN	/* ability to use "-" as a file, to read stdin */
#define	FEATURE_SPELL	/* spell checking */
#define	FEATURE_SPLIT	/* :split and friends */
#define	FEATURE_TAGS	/* :tag command -- undef'ing will break ref & ctags */
#define	FEATURE_TEXTOBJ	/* text objects */
#define	FEATURE_V	/* the v/V/^V marking commands */
#undef	FEATURE_XFT	/* antialiased fonts in X11 -- not useful for Windows */

/* The following provide custom implementation of some common functions which
 * are either missing or poorly implemented on some systems.
 */
#undef	NEED_ABORT	/* replaces abort() with a simpler macro */
#undef	NEED_ASSERT	/* defines an custom assert() macro */
#define NEED_BC		/* requires guitcap.c to generate a BC variable */
#if !defined(GUI_WIN32)
#define NEED_TGETENT	/* uses tinytcap instead of termcap library */
#endif
#define NEED_CTYPE	/* use elvis' digraph-sensitive version of <ctype.h> */

/* The following controls debugging features */
/* (NDEBUG is handled by "e2.mak" - defined for WinRel, undef'ed for WinDebug */
#undef	DEBUG_ALLOC	/* define to debug memory allocations; undef to disable */
#undef	DEBUG_SCAN	/* define to debug character scans; undef to disable */
#undef	DEBUG_SESSION	/* define to debug the block cache; undef to disable */
#undef	DEBUG_EVENT	/* define to trace events; undef to disable */
#undef	DEBUG_MARKUP	/* define to debug markup display modes */
#undef	DEBUG_REGEXP	/* define to debug regular expressions */

/* The following terminal descriptions are used in tinytcap.c */
#undef	TERM_925	/* 925, and many other non-ANSI terminals */
#undef	TERM_AMIGA	/* Amiga'a console emulator */
#undef	TERM_ATARI	/* Atari's console emulator */
#undef	TERM_COHERENT	/* Coherent's console */
#define	TERM_DOSANSI	/* PC with ANSI.SYS driver */
#undef	TERM_MINIX	/* Minix console, regardless of computer type */
#define	TERM_NANSI	/* PC with NANSI.SYS driver, or BIOS */
#define	TERM_CONSOLE	/* Win32 console */
#undef	TERM_RAINBOW	/* DEC Rainbow PC */
#define	TERM_VT100	/* DEC VT100 terminal, 80-column mode */
#define	TERM_VT100W	/* DEC VT100 terminal, 132-column mode */
#define	TERM_VT52	/* DEC VT52 terminal */

#if defined (GUI_WIN32)
# define USE_CTL3D	/* 3D controls in dialog boxes */
#endif

#if defined (NEED_CTYPE)
# define _INC_CTYPE	/* else <windows.h> will try to include its ctype */
#endif

#ifdef GUI_WIN32
# define PORTEDBY	"Serge Pirotte"
#endif
