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
#define FEATURE_SHOWTAG	/* the "showtag" option */
#define FEATURE_LPR	/* the ":lpr" command */
#define	FEATURE_ALIAS	/* the ":alias" command */
#define	FEATURE_MKEXRC	/* the ":mkexrc" command */
#define FEATURE_COMPLETE /* filename completion */
#undef	FEATURE_RAM	/* store edit buffers in RAM if "-f ram" */
#define	FEATURE_LITRE	/* faster searches for literal strings */
#define FEATURE_BACKTICK /* the `program` notation in file names */

/* The following provide custom implementation of some common functions which
 * are either missing or poorly implemented on some systems.
 */
#undef	NEED_ABORT	/* replaces abort() with a simpler macro */
#undef	NEED_ASSERT	/* defines an custom assert() macro */
#define NEED_BC		/* requires guitcap.c to generate a BC variable */
#if !defined(GUI_WIN32)
#define NEED_TGETENT	/* uses tinytcap instead of termcap library */
#endif
#define NEED_CTYPE

/* The following controls debugging features */
/* (NDEBUG is handled by "e2.mak" - defined for WinRel, undef'ed for WinDebug */
#undef	DEBUG_ALLOC	/* define to debug memory allocations; undef to disable */
#undef	DEBUG_SCAN	/* define to debug character scans; undef to disable */
#undef	DEBUG_SESSION	/* define to debug the block cache; undef to disable */
#undef	DEBUG_EVENT	/* define to trace events; undef to disable */
#undef	DEBUG_MARKUP	/* define to debug markup display modes */

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
