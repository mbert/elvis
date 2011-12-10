/* config.h */

/* This file began a copy of the "osmsdos\osconfig.h" file.  You should
 * modify "config.h" and leave "osmsdos\osconfig.h" untouched.
 */

/* The following determine which user interfaces are to be supported */
#undef	GUI_X11		/* simple X-windows interface */
#undef	GUI_CURSES	/* curses interface */
#define	GUI_TERMCAP	/* termcap interface */
#undef	GUI_BIOS	/* IBM-clone BIOS calls */
#define	GUI_OPEN	/* open-mode only, does nothing fancy */

/* The following control which display modes are available */
#undef	DISPLAY_C	/* the old "c" mode */
#define	DISPLAY_SYNTAX	/* the new generic "syntax" colorizer */
#define	DISPLAY_MARKUP	/* the "html" and "man" modes */
#define	DISPLAY_HEX	/* the "hex" display mode */

/* The following provide custom implementation of some common functions which
 * are either missing or poorly implemented on some systems.
 */
#undef	NEED_ABORT	/* replaces abort() with a simpler macro */
#undef	NEED_ASSERT	/* defines an custom assert() macro */
#define NEED_TGETENT	/* uses tinytcap instead of termcap library */
#define NEED_BC		/* requires guitcap.c to generate a BC variable */

/* The following controls debugging features */
#ifndef NDEBUG
# define NDEBUG		/* undef to enable assert() calls; define to disable */
#endif
#undef	DEBUG_ALLOC	/* define to debug memory allocations; undef to disable */
#undef	DEBUG_SCAN	/* define to debug character scans; undef to disable */
#undef	DEBUG_SESSION	/* define to debug the block cache; undef to disable */
#undef	DEBUG_EVENT	/* define to trace events; undef to disable */
#undef	DEBUG_MARKUP	/* define to debug markup display modes */

/* The following changes the default block size, so elvis will use less RAM. */
#define BLKSIZE	512	/* default block size */

/* We only want tinytcap.c to support the bare minumum number of terminals */
#define TERM_COHERENT	/* Coherent's console */
#define TERM_DOSANSI	/* PC with ANSI.SYS driver */
#define TERM_NANSI	/* PC with NANSI.SYS driver, or BIOS */
#define TERM_RAINBOW	/* DEC Rainbow PC */
