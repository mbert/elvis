/* elvis.h */
/* Copyright 1995 by Steve Kirkendall */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"

#define QTY(array)	(sizeof(array) / sizeof((array)[0]))
#define ELVCTRL(ch)	((ch) ^ 0x40)

/* Names of some special buffers */
#define INIT_BUF	"Elvis initialization"
#define CUSTOM_BUF	"Elvis custom initialization"
#define BEFOREREAD_BUF	"Elvis before reading"
#define AFTERREAD_BUF	"Elvis after reading"
#define BEFOREWRITE_BUF	"Elvis before writing"
#define AFTERWRITE_BUF	"Elvis after writing"
#define MSG_BUF		"Elvis messages"
#define MSGQUEUE_BUF	"Elvis message queue"
#define UNTITLED_BUF	"Elvis untitled #%d"
#define EX_BUF		"Elvis ex history"
#define REGEXP_BUF	"Elvis regexp history"
#define FILTER_BUF	"Elvis filter history"
#define MORE_BUF	"Elvis more"
#define CUTANON_BUF	"Elvis cut buffer 1"	/* anonymous cut buffer */
#define CUTNAMED_BUF	"Elvis cut buffer %c"	/* cut buffers a-z and 1-9 */
#define CUTEXTERN_BUF	"Elvis clipboard"	/* cut buffers < and > */
#define CUTINPUT_BUF	"Elvis previous input"	/* cut buffer . */
#define DEFAULT_BUF	"Elvis default options"
#define HELP_BUF	"Elvis documentation"
#define ERRLIST_BUF	"Elvis error list"
#define TRACE_BUF	"Elvis map log"

/* Names of files that store default contents of buffers */
#define INIT_FILE	"elvis.ini"	/* executed before first file is loaded */
#define CUSTOM_FILE	"elvis.rc"	/* custom file for each user */
#define BEFOREREAD_FILE	"elvis.brf"	/* executed before each file is loaded */
#define AFTERREAD_FILE	"elvis.arf"	/* executed after each file is loaded */
#define BEFOREWRITE_FILE "elvis.bwf"	/* executed before writing a file */
#define AFTERWRITE_FILE	"elvis.awf"	/* executed after writing a file */
#define MSG_FILE	"elvis.msg"	/* verbose message translations */
#define HELP_FILE	"elvis.html"	/* elvis online documentation */
#define SYNTAX_FILE	"elvis.syn"	/* descriptions of languages */

/* a very large number */
#define INFINITY	2147483647L

/* default size of the tag stack (for each window) */
#define TAGSTK		10

typedef enum {False, True} BOOLEAN;
typedef enum { RESULT_COMPLETE, RESULT_MORE, RESULT_ERROR } RESULT;
typedef unsigned char CHAR;
typedef unsigned short COUNT;
typedef unsigned int	_COUNT_;
typedef unsigned int	_CHAR_;
typedef int		_char_;

#ifdef NEED_CTYPE
# include "elvctype.h"
#else
# include <ctype.h>
#endif

/* Character conversions, and other operations */
#define toCHAR(s)	((CHAR *)(s))
#define tochar8(s)	((char *)(s))
#define CHARcpy(d,s)	((void)strcpy((char *)(d), (char *)(s)))
#define CHARcat(d,s)	((void)strcat((char *)(d), (char *)(s)))
#define CHARncpy(d,s,n)	((void)strncpy((char *)(d), (char *)(s), (n)))
#define CHARlen(s)	strlen((char *)(s))
#define CHARchr(s,c)	((CHAR *)strchr((char *)(s), (char)(c)))
#define CHARcmp(s,t)	(strcmp((char *)(s), (char *)(t)))
#define CHARncmp(s,t,n) (strncmp((char *)(s), (char *)(t), (n)))
#define CHARdup(s)	((CHAR *)safedup(tochar8(s)))
#define CHARkdup(s)	((CHAR *)safekdup(tochar8(s)))
#define long2CHAR(s,l)	((void)sprintf((char *)(s), "%ld", (l)))
#define CHAR2long(s)	(atol(tochar8(s)))

/* Module header files */
#include "version.h"
#include "osdef.h"
#ifndef USE_PROTOTYPES
# if defined(__STDC__) || defined(__cplusplus)
#  define USE_PROTOTYPES	1
# else
#  define USE_PROTOTYPES	0
# endif
#endif
#if USE_PROTOTYPES
# define P_(args)	args
#else
# define P_(args)	()
#endif

#if defined (__cplusplus)
#define BEGIN_EXTERNC	extern "C" {
#define END_EXTERNC	}
#else
#define BEGIN_EXTERNC
#define END_EXTERNC
#endif

#include "safe.h"
#include "options.h"
#include "optglob.h"
#include "session.h"
#include "lowbuf.h"
#include "message.h"
#include "buffer.h"
#include "mark.h"
#include "buffer2.h"
#include "options2.h"
#include "scan.h"
#include "opsys.h"
#include "map.h"
#include "gui.h"
#include "display.h"
#include "draw.h"
#include "state.h"
#include "window.h"
#include "gui2.h"
#include "display2.h"
#include "draw2.h"
#include "state2.h"
#include "event.h"
#include "input.h"
#include "vi.h"
#include "regexp.h"
#include "ex.h"
#include "move.h"
#include "vicmd.h"
#include "operator.h"
#include "cut.h"
#include "elvisio.h"
#include "lp.h"
#include "calc.h"
#include "more.h"
#include "digraph.h"
#include "need.h"
#include "misc.h"

/* The following are defined in main.c */
extern GUI *chosengui;
BEGIN_EXTERNC
extern void mainfirstcmd P_((WINDOW win));
END_EXTERNC
