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
#define	DISPLAY_HTML	/* the "html" display mode */
#define	DISPLAY_MAN	/* the "man" display mode */
#undef	DISPLAY_TEX	/* the "tex" display mode */
#undef	PROTOCOL_HTTP	/* the "http" protocol -- no MSDOS code is provided */
#undef	PROTOCOL_FTP	/* the "ftp" protocol -- no MSDOS code is provided */
#undef	FEATURE_ALIAS	/* the ":alias" command */
#undef	FEATURE_ARRAY	/* subscripts in expressions */
#undef	FEATURE_BACKTICK /* the `program` notation in file names */
#undef	FEATURE_BROWSE	/* the :browse and :sbrowse commands */
#undef	FEATURE_CACHEDESC /* store syntax/markup descriptions in RAM */
#define FEATURE_CALC	/* the calculator, and all cmds that use it */
#undef	FEATURE_COMPLETE /* filename completion */
#undef	FEATURE_FOLD	/* the :fold and :unfold commands */
#undef	FEATURE_G	/* most of the visual 'g' commands */
#undef	FEATURE_HLOBJECT /* the hlobject and hllayers options */
#undef	FEATURE_HLSEARCH  /* the hlsearch option */
#undef	FEATURE_INCSEARCH /* the incsearch option */
#undef	FEATURE_LISTCHARS /* the listchars option */
#undef	FEATURE_LITRE	/* faster searches for literal strings */
#define	FEATURE_LPR	/* the ":lpr" command */
#define	FEATURE_MAKE	/* the :make and :cc commands */
#undef	FEATURE_MAPDB	/* the map debugger */
#define	FEATURE_MISC	/* lots of little things */
#undef	FEATURE_MKEXRC	/* the ":mkexrc" command */
#define	FEATURE_MOUSE	/* allow the mouse to be used for selections & tags */
#undef	FEATURE_NORMAL	/* vim-style :normal command */
#undef	FEATURE_PERSIST	/* the persistfile option */
#undef	FEATURE_PROTO	/* using aliases to add new protocols */
#undef	FEATURE_RAM	/* if invoked with "-f ram" then use XMS/EMS */
#undef	FEATURE_RCSID	/* include RCS Id strings for all source files */
#undef	FEATURE_REGION	/* the :region command */
#undef	FEATURE_SHOWTAG	/* the "showtag" option */
#undef	FEATURE_SMARTARGS /* show arguments when inputting a function call */
#undef	FEATURE_SPELL	/* spell checking */
#define	FEATURE_SPLIT	/* :split and friends */
#undef	FEATURE_STDIN	/* ability to use "-" as a file, to read stdin */
#define	FEATURE_TAGS	/* :tag command -- undef'ing will break ref & ctags */
#undef	FEATURE_TEXTOBJ	/* text objects */
#define	FEATURE_V	/* the v/V/^V marking commands */
#undef	FEATURE_XFT	/* antialiased fonts in X11 -- useless in MSDOS */

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
#undef	DEBUG_REGEXP	/* define to debug regular expressions */

/* The following reduces the default block size, so elvis will use less RAM. */
#define	BLKSIZE	1024	/* default block size */
#define BLKCACHE 6	/* default number of blocks in the block cache */

/* We only want tinytcap.c to support the bare minumum number of terminals */
#define	TERM_COHERENT	/* Coherent's console */
#define	TERM_DOSANSI	/* PC with ANSI.SYS driver */
#define	TERM_NANSI	/* PC with NANSI.SYS driver, or BIOS */
#undef	TERM_RAINBOW	/* DEC Rainbow PC */
