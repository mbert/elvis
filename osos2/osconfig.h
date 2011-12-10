/* config.h */

/* This file began a copy of the "osos2\osconfig.h" file.  You should
 * modify "config.h" and leave "osos2\osconfig.h" untouched.

/* The following determine which user interfaces are to be supported */
#undef	GUI_X11		/* simple X-windows interface */
#undef	GUI_PM		/* OS/2 Presentation Manager interface */
#define	GUI_VIO		/* OS/2 console interface */
#undef	GUI_CURSES	/* curses interface */
#undef	GUI_TERMCAP	/* termcap interface */
#undef	GUI_BIOS	/* IBM-clone BIOS calls */
#undef	GUI_OPEN	/* open-mode only, does nothing fancy */

/* These allow you to selectively disable the display modes.  You'll probably
 * want the "c" mode disabled, since the "syntax" mode can do the exact same
 * thing.  If you disable "html" then the help files won't look right.
 * #define to enable the mode, #undef to exclude it.
 */
#define	DISPLAY_HEX	/* hex    interactive hex dump */
#define	DISPLAY_HTML	/* html   formatted text */
#define	DISPLAY_MAN	/* man    formatted text */
#define	DISPLAY_TEX	/* tex    formatted text */
#define	DISPLAY_SYNTAX	/* syntax generic syntax coloring */

/* The following provide custom implementation of some common functions which
 * are either missing or poorly implemented on some systems.
 */
#undef	NEED_ABORT	/* replaces abort() with a simpler macro */
#undef	NEED_ASSERT	/* defines an custom assert() macro */
#define NEED_TGETENT	/* uses tinytcap instead of termcap library */
#define NEED_CTYPE	/* custom ctype macros -- digraph aware */

/* The following controls debugging features */
#define	NDEBUG		/* undef to enable assert() calls; define to disable */
#undef	DEBUG_ALLOC	/* define to debug memory allocations; undef to disable */
#undef	DEBUG_SCAN	/* define to debug character scans; undef to disable */
#undef	DEBUG_SESSION	/* define to debug the block cache; undef to disable */
#undef	DEBUG_EVENT	/* define to trace events; undef to disable */
#undef	DEBUG_MARKUP	/* define to debug markup display modes */

/* The following changes the default block size, so elvis will use less RAM. */
#define BLKSIZE	512	/* default block size */

/* We only want tinytcap.c to support the bare minumum number of terminals */
#undef TERM_COHERENT	/* Coherent's console */
#define TERM_DOSANSI	/* PC with ANSI.SYS driver */
#undef TERM_NANSI	/* PC with NANSI.SYS driver, or BIOS */
#undef TERM_RAINBOW	/* DEC Rainbow PC */
