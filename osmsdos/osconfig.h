/* config.h */

/* This file began as a copy of the "osmsdos\osconfig.h" file.  You should
 * modify "config.h" and leave "osmsdos\osconfig.h" untouched.
 */

/* The following determine which user interfaces are to be supported */
#undef	GUI_X11		/* simple X-windows interface */
#undef	GUI_CURSES	/* curses interface */
#define	GUI_TERMCAP	/* termcap interface */
#define	GUI_OPEN	/* only supports "open" mode and scripts */

/* The following control which display modes & other features are available */
#define	DISPLAY_SYNTAX	/* the new generic "syntax" colorizer */
#define	DISPLAY_HEX	/* the "hex" display mode */
#define	DISPLAY_MARKUP	/* the "html" and "man" modes */
#undef	PROTOCOL_HTTP	/* the "http" protocol -- no MSDOS code is provided */
#undef	PROTOCOL_FTP	/* the "ftp" protocol -- no MSDOS code is provided */
#undef	FEATURE_SHOWTAG	/* the "showtag" option */
#define	FEATURE_LPR	/* the ":lpr" command */
#define	FEATURE_ALIAS	/* the ":alias" command */
#define	FEATURE_MKEXRC	/* the ":mkexrc" command */
#undef	FEATURE_COMPLETE /* filename completion */
#define	FEATURE_MOUSE	/* allow the mouse to be used for selections & tags */
#undef	FEATURE_RAM	/* if invoked with "-f ram" then use XMS/EMS */
#undef	FEATURE_LITRE	/* faster searches for literal strings */

/* The following provide custom implementation of some common functions which
 * are either missing or poorly implemented on some systems.
 */
#undef	NEED_ABORT	/* replaces abort() with a simpler macro */
#undef	NEED_ASSERT	/* defines an custom assert() macro */
#define NEED_TGETENT	/* uses tinytcap instead of termcap library */
#define NEED_BC		/* requires guitcap.c to generate a BC variable */
#define NEED_CTYPE	/* use elvis' version of ctype macros */

/* The following controls debugging features */
#ifndef NDEBUG
# define NDEBUG		/* undef to enable assert() calls; define to disable */
#endif
#undef	DEBUG_ALLOC	/* define to debug memory allocations; undef to disable */
#undef	DEBUG_SCAN	/* define to debug character scans; undef to disable */
#undef	DEBUG_SESSION	/* define to debug the block cache; undef to disable */
#undef	DEBUG_EVENT	/* define to trace events; undef to disable */
#undef	DEBUG_MARKUP	/* define to debug markup display modes */

/* The following reduces the default block size, so elvis will use less RAM. */
#define	BLKSIZE	1024	/* default block size */
#define BLKCACHE 6	/* default number of blocks in the block cache */

/* We only want tinytcap.c to support the bare minumum number of terminals */
#define	TERM_COHERENT	/* Coherent's console */
#define	TERM_DOSANSI	/* PC with ANSI.SYS driver */
#define	TERM_NANSI	/* PC with NANSI.SYS driver, or BIOS */
#undef	TERM_RAINBOW	/* DEC Rainbow PC */
