/* guix11.c */

/* Copyright 1995 by Steve Kirkendall */

char id_guix11[] = "$Id: guix11.c,v 2.87 1996/10/01 19:45:58 steve Exp $";

#include "elvis.h"
#ifdef GUI_X11

#include <sys/types.h>
#include <sys/time.h>
#ifdef NEED_SELECT_H
# include <sys/select.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#include <X11/Xatom.h>
#include "lib/elvis.xbm"

/* default values of options */
#define DEFAULT_TOOLBAR		True
#define DEFAULT_CONTROLFONT	"variable"
#define DEFAULT_NORMALFONT	"fixed"
#define DEFAULT_CURSORCOLOR	"red"
#define DEFAULT_OWNCOLOR	"red"
#define DEFAULT_SCROLLBARFG	"gray75"
#define DEFAULT_SCROLLBARBG	"gray60"
#define DEFAULT_GEOMETRY	"80x34"
#define DEFAULT_XCOLUMNS	80
#define DEFAULT_XROWS		34
#define DEFAULT_SCROLLBARWIDTH	14
#define DEFAULT_SCROLLBARTIME	4
#define DEFAULT_DBLCLICKTIME	3
#define DEFAULT_ICON		True
#define DEFAULT_BLINKTIME	3
#define DEFAULT_STOPSHELL	"xterm &"


/* scroll bar appearance */
#define SB_GAP			2	/* blank pixels between buttons */
#define SB_BEVEL		2	/* width of 3-D shading for buttons */
#define TB_HEIGHT		(o_toolbar ? (2 + 2 * SB_BEVEL + loadedcontrol->height) : 0)

/* Graphic characters in a typical LATIN-1 font */
#define GCH	'\022'
#define GCV	'\031'
#define	GC1	'\016'
#define GC2	'\027'
#define GC3	'\013'
#define GC4	'\025'
#define GC5	'\017'
#define GC6	'\026'
#define GC7	'\015'
#define GC8	'\030'
#define GC9	'\014'

typedef enum { SB_REDRAW, SB_NORMAL, SB_BLANK, SB_STOP } SBSTATE;

typedef struct loadedfont_s
{
	struct loadedfont_s *next;	/* next font in linked list */
	XFontStruct	*fontinfo;	/* X font structure */
	char		*name;		/* name of the font */
	int		height;		/* height of font (ascent + descent) */
	int		links;		/* number of windows using this font */
} LOADEDFONT;

typedef struct loadedcolor_s
{
	struct loadedcolor_s *next;	/* next color in linked list */
	unsigned long	pixel;		/* the color code */
	CHAR		*name;		/* name of the color */
	int		links;		/* number of uses for this color */
} LOADEDCOLOR;

typedef struct x11win_s
{
	struct x11win_s	*next;		/* pointer to some other window */
	Window		window;		/* top-level X window */
	Window		textw;		/* subwindow for text & scrollbar */
	Window		toolw;		/* subwindow for toolbar */
	Pixmap		undercurs;	/* image of character under cursor */
	GC		gc;		/* graphic context for this window */
	unsigned long	fg, bg;		/* current foreground & background */
	unsigned long	bgnormal;	/* normal background color */
	unsigned long	fgnormal;	/* foreground color for normal text */
	unsigned long	fgfixed;	/* foreground color of fixed text */
	unsigned long	fgbold;		/* foreground color for bold text */
	unsigned long	fgemph;		/* foreground color of emphasized text */
	unsigned long	fgitalic;	/* foreground color for italic text */
	unsigned long	fgundln;	/* foreground color for underlined text */
	unsigned long	fgcursor;	/* foreground color for cursor */
	unsigned long	owncursor;	/* foreground for cursor when elvis owns selection */
	unsigned long	bgscroll;	/* background color of scrollbar */
	unsigned long	fgscroll;	/* foreground color of scrollbar */
	BOOLEAN		grexpose;	/* are graphic exposures allow now? */
	char		*title;		/* name of the window */
	int		cursx, cursy;	/* cursor position */
	unsigned	cellw, cellh;	/* size of a character cell, in pixels */
	int		cellbase;	/* baseline of glyphs, relative to top of cell */
	unsigned	rows, columns;	/* size of the window, in cells */
	ELVCURSOR	cursor;		/* current state of cursor */
	ELVCURSOR	nextcursor;	/* next state of cursor */
	BOOLEAN		ismapped;	/* is window visible? */
	ELVISSTATE	state;		/* command state of window */
	int		sbheight;	/* total height of scrollbar */
	int		sbtop;		/* top of thumb */
	int		sbbottom;	/* bottom of thumb */
	SBSTATE		sbstate;	/* scrollbar state */
	char		toolstate[100];	/* states of buttons */
	int		x, y;		/* position of window */
	BOOLEAN		nowarp;		/* don't warp pointer into this window */
} X11WIN;

/* This stores the definition of toolbar buttons */
typedef struct tool_s
{
	struct tool_s	*next;		/* another toolbar button */
	char		*label;		/* button legend */
	char		*excmd;		/* command to execute */
	CHAR		*when;		/* condition when button works */
	CHAR		*in;		/* condition when button drawn as "in" */
	int		x;		/* button's position within toolbar */
	int		width;		/* button's width */
	int		textx, texty;	/* where to start drawing text */
	int		id;		/* index into xw->toolstate[] */
	BOOLEAN		safer;		/* run with "safer" flag? */
} TOOL;


#if USE_PROTOTYPES
static BOOLEAN clipopen(BOOLEAN forwrite);
static BOOLEAN clrtoeol(GUIWIN *gw);
static BOOLEAN color(GUIWIN *gw, _char_ font, CHAR *fg, CHAR *bg);
static BOOLEAN creategw(char *name,char * attributes);
static BOOLEAN scroll(GUIWIN *gw, int qty, BOOLEAN notlast);
static BOOLEAN shift(GUIWIN *gw, int qty, int rows);
static BOOLEAN wpoll(BOOLEAN reset);
static LOADEDFONT *loadfont(char *name);
static RESULT stop(BOOLEAN alwaysfork);
static int clipread(CHAR *text, int len);
static int clipwrite(CHAR *text, int len);
static int init(int argc, char **argv);
static int keylabel(CHAR *given, int givenlen, CHAR **label, CHAR **rawin);
static int test(void);
static BOOLEAN clientaction(int argc, char **argv);
static unsigned long loadcolor(CHAR *name, unsigned long def);
static void beep(GUIWIN *gw);
static void clipclose(void);
static void destroygw(GUIWIN *gw, BOOLEAN force);
static void draw(GUIWIN *gw, _char_ font, CHAR *text, int len);
static void drawcursor(X11WIN *xw);
static void erasecursor(X11WIN *xw);
static void loop(void);
static void moveto(GUIWIN *gw, int column, int row);
static void retitle(GUIWIN *gw, char *name);
static void scrollbar(GUIWIN *gw, long top, long bottom, long total);
static int catchErrors(Display *disp, XErrorEvent *err);
static void term(void);
static void unloadcolor(unsigned long pixel);
static void unloadfont(LOADEDFONT *font);
static void usage(void);
static void bevelrect(X11WIN *xw, Window win, int x, int y, unsigned w, unsigned h, BOOLEAN in);
#endif
static BOOLEAN focusgw P_((GUIWIN *gw));
static void setsbstate P_((X11WIN *xw, SBSTATE newstate));
static void draw1tool P_((X11WIN *xw, TOOL *tool, _char_ newstate));
static void drawtoolbar P_((X11WIN *xw, BOOLEAN fromscratch));
static int ignoreErrors P_((Display *disp, XErrorEvent *err));
static void flush P_((void));
static BOOLEAN guicmd P_((GUIWIN *gw, char *extra));

static jmp_buf	xerror_handler;		/* used to recover from protocol errors */
static Display	*display;		/* X11 display */
static int	screen;			/* screen number */
static Window	root;			/* root window */
static int	rootheight, rootwidth;	/* size of root window */
static int	depth;			/* bits per pixel */
static X11WIN	*winlist;		/* list of windows */
static TOOL	*toollist;		/* list of toolbar buttons */
static Colormap	colormap;		/* colormap shared by elvis windows */
static Pixmap	elvis_icon;		/* elvis' window icon */
static char	*argv0;			/* name of program */
static LOADEDFONT *fonts;		/* list of allocated fonts */
static LOADEDCOLOR*colors;		/* list of allocated colors */
static LOADEDFONT *defaultnormal;	/* normal font */
static LOADEDFONT *defaultbold;		/* bold font, or NULL to fake it */
static LOADEDFONT *defaultitalic;	/* italic font, or NULL to fake it */
static LOADEDFONT *loadedcontrol;	/* toolbar font */
static Atom	wm_protocols;		/* value for WM_PROTOCOLS atom */
static Atom	wm_delete_window;	/* value for WM_DELETE_WINDOW atom */
static Atom	elvis_cutbuffer;	/* value for ELVIS_CUTBUFFER atom */
static Atom	elvis_server;		/* value for ELVIS_SERVER atom */
static unsigned long white, black;	/* default color values */
static Time	now;			/* timestamp of current event */
static Time	firstclick;		/* timestamp of previous click */
static BOOLEAN	ownselection;		/* does elvis own the X11 selection? */
static char	*clipbuf;		/* pointer to malloc'ed buffer of chars to/from X */
static long	clipsize;		/* total number of bytes in clipbuf */
static long	clipused;		/* if reading, total number of bytes read previously */
static BOOLEAN	clipwriting;		/* True if cutting to X, False if pasting from X */
static Window	fromwin;		/* window which invoked elvis */
static X11WIN	*hasfocus;		/* window with keyboard focus, or NULL */


#define WIN2XW(win,xw)	for ((xw) = winlist;\
			     (xw) && (xw)->window != (win) && (xw)->textw != (win) && (xw)->toolw != (win);\
			     (xw) = (xw)->next)\
			{\
			}\
			if (!(xw)) break;

/* This table lists the keys which are mapped automatically */
struct
{
	char	*label;
	KeySym	sym;
	char	*cooked;
	MAPFLAGS flags;
} keys[] =
{
	{ "<Up>",	XK_Up,		"k",	MAP_ALL },
	{ "<Down>",	XK_Down,	"j",	MAP_ALL },
	{ "<Left>",	XK_Left,	"h",	MAP_ALL },
	{ "<Right>",	XK_Right,	"l",	MAP_ALL },
	{ "<Prior>",	XK_Prior,	"\002",	MAP_ALL_VISUAL },
	{ "<Next>",	XK_Next,	"\006",	MAP_ALL_VISUAL },
	{ "<Home>",	XK_Home,	"^",	MAP_ALL },
	{ "<Begin>",	XK_Begin,	"^",	MAP_ALL },
	{ "<End>",	XK_End,		"$",	MAP_ALL },
	{ "<Insert>",	XK_Insert,	"i",	MAP_ALL },
	{ "<Delete>",	XK_Delete,	"x",	MAP_ALL },
	{ "<Undo>",	XK_Undo,	"u",	MAP_ALL_VISUAL },
	{ "<Help>",	XK_Help,	":help\r",MAP_ALL_VISUAL },
	{ "<Multi_key>",XK_Multi_key,	"\013",	MAP_INPUT|MAP_OPEN }
};


static OPTDESC x11desc[] =
{
	{"normalfont", "xfn",	optsstring,	optisstring	},
	{"boldfont", "xfb",	optsstring,	optisstring	},
	{"italicfont", "xfi",	optsstring,	optisstring	},
	{"controlfont", "xfc",	optsstring,	optisstring	},
	{"toolbar", "xtb",	NULL,		NULL		},
	{"scrollbarwidth","xsw",optnstring,	optisnumber,	"5:40"},
	{"scrollbartime", "xst",optnstring,	optisnumber,	"0:20"},
	{"dblclicktime", "xdct",optnstring,	optisnumber,	"1:10"},
	{"blinktime", "xbt",	optnstring,	optisnumber,	"0:10"},
	{"xrows", "xlines",	optnstring,	optisnumber,	"3:200"},
	{"xcolumns", "xcols",	optnstring,	optisnumber,	"30:200"},
	{"firstx", "xpos",	optnstring,	optisnumber,	"-2000:2000"},
	{"firsty", "ypos",	optnstring,	optisnumber,	"-2000:2000"},
	{"icon", "icon",	NULL,		NULL		},
	{"iconic", "iconic",	NULL,		NULL		},
	{"stopshell", "ssh",	optsstring,	optisstring	},
	{"autoiconify", "aic",	NULL,		NULL		},
	{"altkey", "metakey",	opt1string,	optisoneof,	"control-O setbit ignore"},
	{"stagger", "step",	optnstring,	optisnumber,	"0:200"},
	{"warpback", "wb",	NULL,		NULL		},
	{"warpto", "wt",	opt1string,	optisoneof,	"don't scrollbar origin corners"},
	{"focusnew", "fn",	NULL,		NULL		}
};
static struct
{
	OPTVAL	normalfont, boldfont, italicfont, controlfont, toolbar,
		scrollbarwidth, scrollbartime, dblclicktime, blinktime,
		xrows, xcolumns, firstx, firsty, icon, iconic, stopshell,
		autoiconify, altkey, stagger, warpback, warpto, focusnew;
} x11opt;
#define o_normalfont	x11opt.normalfont.value.string
#define o_boldfont	x11opt.boldfont.value.string
#define o_italicfont	x11opt.italicfont.value.string
#define o_controlfont	x11opt.controlfont.value.string
#define o_toolbar	x11opt.toolbar.value.boolean
#define o_scrollbarwidth x11opt.scrollbarwidth.value.number
#define o_scrollbartime	x11opt.scrollbartime.value.number
#define o_dblclicktime	x11opt.dblclicktime.value.number
#define o_blinktime	x11opt.blinktime.value.number
#define o_xrows		x11opt.xrows.value.number
#define o_xcolumns	x11opt.xcolumns.value.number
#define o_firstx	x11opt.firstx.value.number
#define o_firsty	x11opt.firsty.value.number
#define o_icon		x11opt.icon.value.boolean
#define o_iconic	x11opt.iconic.value.boolean
#define o_stopshell	x11opt.stopshell.value.string
#define o_autoiconify	x11opt.autoiconify.value.boolean
#define o_altkey	x11opt.altkey.value.character
#define o_stagger	x11opt.stagger.value.number
#define o_warpback	x11opt.warpback.value.boolean
#define o_warpto	x11opt.warpto.value.character
#define o_focusnew	x11opt.focusnew.value.boolean


/* The following store names of colors */
static CHAR background[50];	
static CHAR foreground[50];
static CHAR boldcolor[50];
static CHAR emphcolor[50];
static CHAR fixedcolor[50];
static CHAR italiccolor[50];
static CHAR underlinecolor[50];
static CHAR cursorcolor[50];
static CHAR owncolor[50];
static CHAR scrollbarfg[50];
static CHAR scrollbarbg[50];

/* This function loads a font if it isn't already loaded. */
static LOADEDFONT *loadfont(name)
	char	*name;	/* name of the font */
{
	LOADEDFONT *font;
	XFontStruct *info;

	/* see if it is already loaded */
	for (font = fonts; font && strcmp(name, font->name); font = font->next)
	{
	}

	/* if already loaded, then just increment its count */
	if (font)
	{
		font->links++;
		return font;
	}

	/* else load the font into a new stucture */
	info = XLoadQueryFont(display, name);
	if (!info)
	{
		msg(MSG_ERROR, "[s]can't load font $1", name);
		return NULL;
	}
	font = (LOADEDFONT *)safealloc(1, sizeof(LOADEDFONT));
	font->fontinfo = info;
	font->name = safedup(name);
	font->height = info->descent + info->ascent;
	font->links = 1;

	/* link the new structure into the list, and return it */
	font->next = fonts;
	fonts = font;
	return font;
}


/* This function unloads a font.  If no window is using the font after that,
 * then the storage space is freed.
 */
static void unloadfont(font)
	LOADEDFONT	*font;	/* font to be freed */
{
	LOADEDFONT	*scan;

	assert(fonts != NULL);

	/* If no font was given, do nothing */
	if (!font)
	{
		return;
	}

	/* decrement the count.  If other windows are still using this
	 * font, then that's all we should for now.
	 */
	if (--font->links > 0)
	{
		return;
	}

	/* delete the font from the list of fonts */
	if (fonts == font)
	{
		fonts = font->next;
	}
	else
	{
		for (scan = fonts; scan->next != fonts; scan = scan->next)
		{
			assert(scan->next);
		}
		scan->next = font->next;
	}

	/* free its resources */
	safefree(font->name);
	XFreeFont(display, font->fontinfo);
	safefree(font);
}


/* This function allocates a color */
static unsigned long loadcolor(name, def)
	CHAR		*name;	/* name of color to load */
	unsigned long	def;	/* default color, if can't load named color */
{
	XColor		exact, color;
	LOADEDCOLOR	*scan;

	/* if no name, then just use the default */
	if (!name || !*name)
		goto UseDefault;

	/* was this color name used before? */
	for (scan = colors; scan; scan = scan->next)
	{
		if (!CHARcmp(name, scan->name))
		{
			scan->links++;
			return scan->pixel;
		}
	}

	/* try to load the color */
	if (!XAllocNamedColor(display, colormap, tochar8(name), &exact, &color))
	{
		msg(MSG_WARNING, "[S]could not allocate color $1", name);
		goto UseDefault;
	}

	/* was it rounded to a previous color? */
	for (scan = colors; scan; scan = scan->next)
	{
		if (scan->pixel == color.pixel)
		{
			scan->links++;
			return scan->pixel;
		}
	}

	/* new color -- remember it */
	scan = safealloc(1, sizeof(*scan));
	scan->name = CHARdup(name);
	scan->pixel = color.pixel;
	scan->links = 1;
	scan->next = colors;
	colors = scan;
	return color.pixel;

UseDefault:
	/* look for the default pixel code in the list */
	for (scan = colors; scan; scan = scan->next)
	{
		if (scan->pixel == def)
		{
			scan->links++;
			return def;
		}
	}

	/* not in list -- add it */
	scan = safealloc(1, sizeof *scan);
	scan->name = toCHAR(def == black ? "black" : "white");
	scan->pixel = def;
	scan->links = 1;
	scan->next = colors;
	colors = scan;
	return def;
}


static void unloadcolor(pixel)
	unsigned long	pixel;	/* a color to free */
{
	LOADEDCOLOR	*scan, *lag;

	/* never load/unload the default white & black values */
	if (pixel == white || pixel == black)
	{
		return;
	}

	/* find the color */
	for (lag = NULL, scan = colors; scan->pixel != pixel; lag = scan, scan = scan->next)
	{
		assert(scan->next);
	}

	/* decrement the link counter.  If other windows are using it,
	 * then leave it.
	 */
	if (--scan->links > 0)
	{
		return;
	}

	/* free it */
	XFreeColors(display, colormap, &pixel, 1, 0);
	safefree(scan->name);
	if (lag)
		lag->next = scan->next;
	else
		colors = scan->next;
	safefree(scan);
}


/* Test whether this GUI is available in this environment.
 * Returns 0 if the GUI is unavailable, or 1 if available.
 * This should not have any visible side-effects.  If the
 * GUI can't be tested without side-effects, then this
 * function should return 2 to indicate "maybe available".
 */
static int test()
{
	char	*tmp;

	/* I've had some problems in which DISPLAY gets set to the valid name
	 * of a functioning X server, even though I'm personally not the person
	 * who'se using that X terminal.  To prevent my elvis window from
	 * appearing on his X screen, I'll assume x11 is unavailable if the
	 * TERM environment variable is set, and is initialized to anything
	 * other than "xterm".
	 */
	tmp = getenv("TERM");
	if (tmp && strcmp(tmp, "xterm"))
		return 0;

	/* Try to contact the server.  If we can contact it, great! */
	if (getenv("DISPLAY") && (display = XOpenDisplay("")) != (Display *)0)
		return 1;
	return 0;
}

static int catchErrors(disp, err)
	Display		*disp;
	XErrorEvent	*err;
{
	longjmp(xerror_handler, 1);
}

/* This function transmits the filenames to another elvis processes */
static BOOLEAN clientaction(argc, argv)
	int	argc;	/* number of command-line arguments */
	char	**argv;	/* values of command-line arguments */
{
	Window		srvwin;	/* a window of the server elvis */
	Atom		type;
	unsigned long	ul, dummy;
	int		format;
	unsigned char	*data;
	char		prop[6000];
	char		*cwd;
	int		i;
	char		*tagname = NULL;
	char		*excommand = NULL;

	/* try to find the server window.  If we can't find it, then we
	 * can't do the client thing.
	 */
	XGetWindowProperty(display, root, elvis_server, 0L, 1L, False,
		XA_WINDOW, &type, &format, &ul, &dummy, &data);
	if (ul != 1 || type != XA_WINDOW || format != 32)
		return False;
	srvwin = *(Window *)data;
	XFree(data);

	/* stuff arguments into a property, as ex command strings */
	cwd = dircwd();
	memset(prop, 0, QTY(prop));
	for (i = 1; i < argc && prop[QTY(prop) - 300] == '\0'; i++)
	{
		if (argv[i][0] == '-')
		{
			switch (argv[i][1])
			{
			  case 'c':
				if (argv[i][2])
					excommand = &argv[i][2];
				else if (i + 1 < argc)
					excommand = argv[++i];
				break;

			  case 't':
				if (argv[i][2])
					tagname = &argv[i][2];
				else if (i + 1 < argc)
					tagname = argv[++i];
				break;

			  default:
				fprintf(stderr, "%s: %s not supported with -client\n", argv[0], argv[i]);
				return -1;
			}
		}
		else if (argv[i][0] == '+')
		{
			excommand = (argv[i][1] ? &argv[i][1] : "$");
		}
		else
		{
			strcat(prop, "split ");
			if (argv[i][0] == '/' /* assumes unix-style filenames */
			 || (argv[i][0] == '~' && argv[i][1] == '/'))
				strcat(prop, argv[i]);
			else
				strcat(prop, dirpath(cwd, argv[i]));
			strcat(prop, "\n");
		}
	}
	if (excommand)
	{
		strcat(prop, excommand);
		strcat(prop, "\n");
	}
	if (tagname)
	{
		strcat(prop, "stag ");
		strcat(prop, tagname);
		strcat(prop, "\n");
	}

	/* if the following causes an error, then don't do the client thing */
	if (setjmp(xerror_handler))
	{
		/* we'd better reinitialize the connection */
		XCloseDisplay(display);
		display = XOpenDisplay("");
		screen = DefaultScreen(display);
		root = RootWindow(display, screen);
		return False;
	}
	XSetErrorHandler(catchErrors);

	/* hang the property on the server elvis window */
	XChangeProperty(display, srvwin, elvis_server, XA_STRING, 8,
		PropModeAppend, (unsigned char *)prop, strlen(prop));
	XFlush(display);

	/* shut down the client connection to the X11 display, and cause
	 * an eventual exit.
	 */
	XCloseDisplay(display);
	return True;
}


/* Start the GUI.
 *
 * argc and argv are the command line arguments.  The GUI
 * may scan the arguments for GUI-specific options; if it
 * finds any, then they should be deleted from the argv list.
 * The resulting value of argc should be returned normally.
 * If the GUI couldn't initialize itself, it should emit an
 * error message and return -1.
 *
 * Other than "name" and "test", no other fields of the GUI
 * structure are accessed before this function has been called.
 */
static int init(argc, argv)
	int	argc;	/* number of command-line arguments */
	char	**argv;	/* values of command-line arguments */
{
	int	i, j, ndel;
	int	x, y, flags;
	int	h, w;
	char	raw[50];
	BOOLEAN	client = False;
	BOOLEAN	mustfork = False;
	char	*geomstr = NULL;

	/* initialization */
	if (!display)
		display = XOpenDisplay("");
	if (!display)
		msg(MSG_FATAL, "could not contact X server");
	screen = DefaultScreen(display);
	root = RootWindow(display, screen);

	/* parse the command-line flags */
	argv0 = argv[0];
	for (i = 1, ndel = 0; i < argc; i = (ndel==0 ? i+1 : i), ndel = 0)
	{
		if (!strcmp(argv[i], "-font") || !strcmp(argv[i], "-fn"))
		{
			optpreset(o_normalfont, toCHAR(argv[i + 1]), OPT_LOCK);
			ndel = 2;
		}
		else if (!strcmp(argv[i], "-fb"))
		{
			optpreset(o_boldfont, toCHAR(argv[i + 1]), OPT_LOCK);
			ndel = 2;
		}
		else if (!strcmp(argv[i], "-fi"))
		{
			optpreset(o_italicfont, toCHAR(argv[i + 1]), OPT_LOCK);
			ndel = 2;
		}
		else if (!strcmp(argv[i], "-courier"))
		{
			if (i + 1 >= argc)
			{
				msg(MSG_ERROR, "-courier requires a font size");
				return -1;
			}
			sprintf(raw, "*-courier-medium-r-*-%s-*", argv[i + 1]);
			optpreset(o_normalfont, CHARdup(toCHAR(raw)), OPT_LOCK|OPT_FREE);
			sprintf(raw, "*-courier-bold-r-*-%s-*", argv[i + 1]);
			optpreset(o_boldfont, CHARdup(toCHAR(raw)), OPT_LOCK|OPT_FREE);
			sprintf(raw, "*-courier-medium-o-*-%s-*", argv[i + 1]);
			optpreset(o_italicfont, CHARdup(toCHAR(raw)), OPT_LOCK|OPT_FREE);
			ndel = 2;
		}
		else if (!strcmp(argv[i], "-fc"))
		{
			optpreset(o_controlfont, toCHAR(argv[i + 1]), OPT_LOCK);
			ndel = 2;
		}
		else if (!strcmp(argv[i], "-fg"))
		{
			CHARcpy(foreground, toCHAR(argv[i + 1]));
			ndel = 2;
		}
		else if (!strcmp(argv[i], "-bg"))
		{
			CHARcpy(background, toCHAR(argv[i + 1]));
			ndel = 2;
		}
		else if (!strcmp(argv[i], "-geometry") || !strcmp(argv[i], "-g"))
		{
			if (i + 1 >= argc)
			{
				msg(MSG_ERROR, "-geometry requires a size and/or position");
				return -1;
			}
			geomstr = argv[i + 1];
			ndel = 2;
		}
		else if (!strcmp(argv[i], "-noicon"))
		{
			o_icon = False;
			ndel = 1;
		}
		else if (!strcmp(argv[i], "-iconic"))
		{
			o_iconic = True;
			ndel = 1;
		}
		else if (!strcmp(argv[i], "-sync"))
		{
			XSynchronize(display, 1);
			ndel = 1;
		}
		else if (!strcmp(argv[i], "-client"))
		{
			mustfork = client = True;
			ndel = 1;
		}
		else if (!strcmp(argv[i], "-fork"))
		{
			mustfork = True;
			ndel = 1;
		}

		/* if we used some arguments, then delete them */
		if (i + ndel > argc)
		{
			msg(MSG_FATAL, "[s]$1 requires an argument", argv[i]);
		}
		if (ndel > 0)
		{
			for (j = i; j < argc - ndel; j++)
			{
				argv[j] = argv[j + ndel];
			}
			argc -= ndel;
		}
	}

	/* Define some atoms */
	wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
	wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
	elvis_cutbuffer = XInternAtom(display, "ELVIS_CUTBUFFER", False);
	elvis_server = XInternAtom(display, "ELVIS_SERVER", False);

	/* if we're supposed to be a client of an existing elvis, then
	 * do the client thing.  If that fails, then ignore -client.
	 */
	if (client && clientaction(argc, argv))
	{
		return -1;
	}

	/* if we're supposed to fork, then do that */
	if (mustfork)
	{
		XCloseDisplay(display);
		switch (fork())
		{
		  case -1: /* ERROR */
			perror("elvis: could not fork");
			return -1;

		  case 0: /* CHILD */
			/* become immune to terminal signals */
#ifdef NEED_SETPGID
			setpgrp();
#else
			setpgid(0, 0);
#endif

			/* reinitialize the connection */
			display = XOpenDisplay("");
			screen = DefaultScreen(display);
			root = RootWindow(display, screen);

			/* and then continue execution... */
			break;

		  default: /* PARENT */
			return -1;
		}
	}

	/* more initialization */
	rootheight = DisplayHeight(display, screen);
	rootwidth = DisplayWidth(display, screen);
	depth = DefaultDepth(display, screen);
	colormap = DefaultColormap(display, screen);
	elvis_icon = XCreateBitmapFromData(display, DefaultRootWindow(display),
                (char *)elvis_bits, elvis_width, elvis_height);
	XGetInputFocus(display, &fromwin, &i);
	if (fromwin == None || fromwin == PointerRoot)
	{
		fromwin = root;
	}

	/* ignore any protocol errors. We expect errors from XSetInputFocus()
	 * in some situations, and no errors from anything else.
	 */
	XSetErrorHandler(ignoreErrors);

	/* initialize the options */
	optflags(o_normalfont) |= OPT_HIDE;
	optflags(o_boldfont) |= OPT_HIDE;
	optflags(o_italicfont) |= OPT_HIDE;
	optflags(o_controlfont) |= OPT_HIDE;
	optpreset(o_toolbar, DEFAULT_TOOLBAR, OPT_HIDE);
	optpreset(o_scrollbarwidth, DEFAULT_SCROLLBARWIDTH, OPT_HIDE);
	optpreset(o_scrollbartime, DEFAULT_SCROLLBARTIME, OPT_HIDE);
	optpreset(o_dblclicktime, DEFAULT_DBLCLICKTIME, OPT_HIDE);
	optpreset(o_blinktime, DEFAULT_BLINKTIME, OPT_HIDE);
	optpreset(o_xrows, DEFAULT_XROWS, OPT_HIDE);
	optpreset(o_xcolumns, DEFAULT_XCOLUMNS, OPT_HIDE);
	optpreset(o_icon, True, OPT_HIDE);
	optpreset(o_stopshell, toCHAR(DEFAULT_STOPSHELL), OPT_HIDE);
	optpreset(o_altkey, 's', OPT_HIDE);
	optpreset(o_warpto, 'd', OPT_HIDE);
	optpreset(o_focusnew, True, OPT_HIDE);
	optinsert("x11", QTY(x11desc), x11desc, (OPTVAL *)&x11opt);

	/* initialize the colors */
	CHARcpy(cursorcolor, toCHAR(DEFAULT_CURSORCOLOR));
	CHARcpy(owncolor, toCHAR(DEFAULT_OWNCOLOR));
	CHARcpy(scrollbarfg, toCHAR(DEFAULT_SCROLLBARFG));
	CHARcpy(scrollbarbg, toCHAR(DEFAULT_SCROLLBARBG));

	/* convert geometry string, if given */
	if (geomstr)
	{
		/* Note: We're doing something weird here.  Since we
		 * don't know yet how large the character cells will be,
		 * we can't compute the window size and position
		 * correctly.  We'll compute it incorrectly here, and
		 * remember the details so we can make corrections
		 * later.
		 */
		flags = XGeometry(display, screen, geomstr,
			DEFAULT_GEOMETRY, 0, 1, 1, 0, 0, &x, &y, &w, &h);
		if ((flags & (WidthValue|HeightValue)) == (WidthValue|HeightValue))
		{
			o_xcolumns = w;
			o_xrows = h;
		}
		if ((flags & (XValue|YValue)) == (XValue|YValue))
		{
			if (flags & XNegative)
				optpreset(o_firstx, x + o_xcolumns - rootwidth, OPT_SET);
			else
				optpreset(o_firstx, x, OPT_SET);
			if (flags & YNegative)
				optpreset(o_firsty, y + o_xrows - rootheight, OPT_SET);
			else
				optpreset(o_firsty, y, OPT_SET);
		}

		/* now, if size was really given, then mark xrows and
		 * xcolumns as having been explicitly "set".
		 */
		flags = XGeometry(display, screen, geomstr,
			NULL, 0, 1, 1, 0, 0, &x, &y, &w, &h);
		if ((flags & (WidthValue|HeightValue)) == (WidthValue|HeightValue))
		{
			optflags(o_xrows) |= OPT_SET;
			optflags(o_xcolumns) |= OPT_SET;
		}
	}

	/* map the cursor keypad keys */
	for (i = 0; i < QTY(keys); i++)
	{
		if (keys[i].sym == XK_Delete)
			sprintf(raw, "%c", ELVCTRL('?'));
		else
			sprintf(raw, "%c%04lx",ELVCTRL('K'), (long)keys[i].sym);
		mapinsert(toCHAR(raw), (int)strlen(raw), toCHAR(keys[i].cooked), (int)strlen(keys[i].cooked), toCHAR(keys[i].label), keys[i].flags);
	}

	/* Redirect stdin to come from /dev/null.  This will only affect
	 * filter programs, and programs started via the ! command.  Without
	 * this, programs which attempted to read keystrokes would wait
	 * forever because the keystrokes would have to come from the ASCII
	 * terminal where elvis was invoked, NOT from elvis' own window.
	 */
	if (close(0) == 0)
		open("/dev/null", O_RDONLY);

	return argc;
}


/* output gui-dependent options */
static void usage()
{
	msg(MSG_INFO, "       -font normalfont   Use \"normalfont\" as normal font");
	msg(MSG_INFO, "       -fn normalfont     Same as -font normalfont");
	msg(MSG_INFO, "       -fb boldfont       Use \"boldfont\" \\(else derive from normal font\\)");
	msg(MSG_INFO, "       -fi italicfont     Use \"italicfont\" \\(else derive from normal font\\)");
	msg(MSG_INFO, "       -courier size      Use three courier fonts of given size");
	msg(MSG_INFO, "       -fc controlfont    Use \"controlfont\" in the toolbar");
	msg(MSG_INFO, "       -fg color          Use \"color\" for the foreground \\(default black\\)");
	msg(MSG_INFO, "       -bg color          Use \"color\" for the background \\(default white\\)");
	msg(MSG_INFO, "[s]       -geometry WxH+X+Y  Set the window's size and/or position \\(default $1\\)", DEFAULT_GEOMETRY);
	msg(MSG_INFO, "       -noicon            Don't use built-in bitmap icon");
	msg(MSG_INFO, "       -iconic            First window should start iconified");
	msg(MSG_INFO, "       -sync              Disable X11 buffering, for debugging");
	msg(MSG_INFO, "       -fork              Run in background");
	msg(MSG_INFO, "       -client            Edit files via an existing elvis process");
}


static void erasecursor(xw)
	X11WIN	*xw;	/* window whose cursor should be hidden */
{
	/* hide the cursor (if shown) */
	if (xw->cursor != CURSOR_NONE)
	{
		if (xw->ismapped)
		{
			if (xw->grexpose)
			{
				XSetGraphicsExposures(display, xw->gc, False);
				xw->grexpose = False;
			}
			XCopyArea(display, xw->undercurs, xw->textw, xw->gc,
				0, 0, xw->cellw, xw->cellh,
				(int)(xw->cursx * xw->cellw), (int)(xw->cursy * xw->cellh));
		}
		xw->cursor = CURSOR_NONE;
	}
}

static void drawcursor(xw)
	X11WIN	*xw;	/* window whose cursor should be drawn */
{
	unsigned long	color;

	/* a NULL "xw" forces all cursors to be redrawn */
	if (!xw)
	{
		for (xw = winlist; xw; xw = xw->next)
		{
			xw->nextcursor = xw->cursor;
			erasecursor(xw);
			drawcursor(xw);
		}
		return;
	}

	/* if not mapped, then no cursor should be drawn */
	if (!xw->ismapped)
	{
		xw->cursor = CURSOR_NONE;
		return;
	}

	/* if same as before, do nothing */
	if (xw->nextcursor == xw->cursor)
	{
		return;
	}

	/* choose a color */
	color = (ownselection ? xw->owncursor : xw->fgcursor);

	/* if some other cursor shape is already drawn there, then erase it */
	if (xw->cursor != CURSOR_NONE)
	{
		erasecursor(xw);
	}
	else /* save the image of the cursor where we'll draw the cursor */
	{
		if (xw->grexpose)
		{
			XSetGraphicsExposures(display, xw->gc, False);
			xw->grexpose = False;
		}
		XCopyArea(display, xw->textw, xw->undercurs, xw->gc,
			(int)(xw->cursx * xw->cellw), (int)(xw->cursy * xw->cellh),
			xw->cellw, xw->cellh, 0, 0);
	}
	xw->cursor = xw->nextcursor;

	/* draw the cursor, using the cursor color */
	if (xw->grexpose)
	{
		XSetGraphicsExposures(display, xw->gc, False);
		xw->grexpose = False;
	}
	if (xw->fg != color)
	{
		XSetForeground(display, xw->gc, color);
		xw->fg = color;
	}
	switch (xw->cursor)
	{
	  case CURSOR_INSERT:
		XFillRectangle(display, xw->textw, xw->gc,
			(int)(xw->cursx * xw->cellw), (int)(xw->cursy * xw->cellh),
			2, xw->cellh);
		break;

	  case CURSOR_REPLACE:
	  case CURSOR_QUOTE:
		XFillRectangle(display, xw->textw, xw->gc,
			(int)(xw->cursx * xw->cellw), (int)((xw->cursy + 1) * xw->cellh - 2),
			xw->cellw, 2);
		break;

	  case CURSOR_COMMAND:
		XDrawRectangle(display, xw->textw, xw->gc,
			(int)(xw->cursx * xw->cellw), (int)(xw->cursy * xw->cellh),
			xw->cellw - 1, xw->cellh - 1);
		break;

	  case CURSOR_NONE:
		break;
	}
}


/* Simulate a "destroy" event for the window, or do the cleanup work after
 * a real destroy notify event.
 */
static void destroygw(gw, force)
	GUIWIN	*gw;	/* the window to be destroyed */
	BOOLEAN	force;	/* if True, try harder */
{
	X11WIN	*xw, *prev;

	/* find the doomed window */
	for (xw = winlist, prev = NULL; xw != (X11WIN *)gw; prev = xw, xw = xw->next)
	{
		assert(xw->next != NULL);
	}

	eventdestroy((GUIWIN *)xw);

	/* delete the window from the list of existing windows */
	if (prev)
	{
		prev->next = xw->next;
	}
	else
	{
		winlist = xw->next;
	}

	/* switch keyboard focus to another elvis window */
	if (winlist != NULL)
	{ /* nishi */
		/* Choose one which isn't iconified */
		for (prev = winlist; prev && !prev->ismapped; prev = prev->next)
		{
		}
		if (prev && o_focusnew)
		{
			focusgw((GUIWIN *)winlist);
		}

		/* Also make the other window be an elvis server window.  This
		 * is only significant if the doomed window used to be the
		 * server, but it is just as easy to change it every time.
		 */
		XChangeProperty(display, root, elvis_server, XA_WINDOW, 32,
			PropModeReplace, (unsigned char *)&winlist->window, 1);
	}

	/* free the window's resources */
	XFreePixmap(display, xw->undercurs);
	XFreeGC(display, xw->gc);
	if (xw->window)
	{
		XDestroySubwindows(display, xw->window);
		XDestroyWindow(display, xw->window);
	}
	unloadcolor(xw->fgnormal);
	unloadcolor(xw->bgnormal);
	unloadcolor(xw->fgfixed);
	unloadcolor(xw->fgbold);
	unloadcolor(xw->fgemph);
	unloadcolor(xw->fgitalic);
	unloadcolor(xw->fgundln);
	unloadcolor(xw->fgcursor);
	unloadcolor(xw->bgscroll);
	unloadcolor(xw->fgscroll);
	safefree(xw->title);
	safefree(xw);
}



/* This function changes the keyboard focus to a specific window */
static BOOLEAN focusgw(gw)
	GUIWIN	*gw;	/* the window to receive keyboard focus */
{
	X11WIN	*xw = (X11WIN *)gw;
	int	x1, y1, x2, y2;

	/* If the window is unmapped (iconfied) then map it.  Also, if the
	 * autoiconify option is set then unmap the previous window.
	 */
	if (!xw->ismapped)
	{
		XMapWindow(display, xw->window);
		
		if (o_autoiconify && windefault && windefault->gw != gw)
		{
			XIconifyWindow(display, ((X11WIN *)windefault->gw)->window, screen);
		}
		
		/* the rest of the focus change must wait until it is mapped */
		return True;
	}

	/* Raise the window. (I.e., make it fully visible.) */
	XRaiseWindow(display, xw->window);

	/* Move the pointer to some point in the window, so that if
	 * keyboard focus follows the mouse, this will switch focus.
	 */
	if (!xw->nowarp)
	{
		switch (o_warpto)
		{
		  case 'o': /* "origin" */
			XWarpPointer(display, None, xw->window, 0,0,0,0, 0,0);
			break;

		  case 's': /* scrollbar */
			XWarpPointer(display, None, xw->window, 0,0,0,0,
			    (int)(xw->cellw * xw->columns + o_scrollbarwidth/2),
			    (xw->sbtop + xw->sbbottom)/2);
			break;

		  case 'c': /* corners */

			/* set x1,y1 to the furthest corner, x2,y2 to nearest */
			if (xw->cursx < xw->columns / 2)
				x1 = xw->cellw * xw->columns + o_scrollbarwidth - 1, x2 = 0;
			else
				x1 = 0, x2 = xw->cellw * xw->columns + o_scrollbarwidth - 1;
			if (xw->cursy < xw->rows / 2)
				y1 = xw->cellh * xw->rows - 1, y2 = 0;
			else
				y1 = 0, y2 = xw->cellh * xw->rows - 1;

			/* warp to furthest corner unless it's off the screen */
			if (y1 + xw->y < rootheight && x1 + xw->x < rootwidth)
				XWarpPointer(display, None, xw->window, 0,0,0,0, x1,y1);
			XFlush(display);

			/* warp to nearest corner */
			XWarpPointer(display, None, xw->window, 0,0,0,0, x2,y2);
			break;

		  /* case 'd': don't -- requires no action */
		}
	}
	xw->nowarp = False;

	/* Explicitly change the focus */
	XSetInputFocus(display, xw->window, RevertToParent, now);
	hasfocus = xw;
	return True;
}


/* In a loop, receive events from the GUI and call elvis
 * functions which will act on the event.  When this function
 * returns, elvis will call the GUI's term() function and then exit.
 * (This function should return only when the number of windows becomes 0.)
 */
static void loop()
{
	XEvent	event;		/* an X event to process */
	XEvent	notify;		/* used in selections */
	XEvent	rptevent;	/* event to repeat */
	long	rpttime = -1;	/* amount of time to allow before repeat */
	KeySym	mykey;
	Window	owner;
	int	i, j;
	char	text[20];
	X11WIN	*xw;
	int	top, bottom, left, right;
	int	ptrx, ptry, prevptrx, prevptry;
	BOOLEAN	didcmd;
	BOOLEAN	marking;	/* dragging mouse in text area */
	BOOLEAN	thumbing;	/* dragging scrollbar's "thumb" */
	BOOLEAN	paging;		/* holding mouse button on scollbar's buttons */
	ELVISSTATE state = 0;
	long	offset;
	CHAR	modifier;
	fd_set	rfds, wfds, efds;
	struct timeval timeout;
	char	*excmd;
	unsigned long	exlen, ldummy;
	Atom	gottype;
	TOOL	*tool;

	/* loop until we don't have any windows left */
	didcmd = True;
	prevptrx = prevptry = -1;
	marking = thumbing = paging = False;
	while (winlist)
	{
		/* draw new window images, if they may have changed */
		if ((rpttime >= 0 || XEventsQueued(display, QueuedAfterFlush) == 0)
			 && didcmd)
		{
			/* for each window... */
			for (xw = winlist; xw; xw = xw->next)
			{
				if (xw->ismapped)
				{
					xw->nextcursor = eventdraw((GUIWIN *)xw);
					drawcursor(xw);
					setsbstate(xw, xw->cursy == xw->rows - 1 ? SB_BLANK : SB_NORMAL);
					drawtoolbar(xw, False);
				}
			}
			didcmd = False;
		}

		/* Read the next event.  This is complicated by the need to
		 * autorepeat some scrollbar events.
		 */
		if (rpttime >= 0 && XPending(display) <= 0)
		{
			/* wait for either rpttime to expire, or for an event
			 * other than MotionNotify to be received.  If rpttime
			 * expires first, then pretend the rptevent was received
			 * again.
			 */
			XFlush(display);
			do
			{
				FD_ZERO(&rfds);
				FD_ZERO(&wfds);
				FD_ZERO(&efds);
				FD_SET(ConnectionNumber(display), &rfds);
				timeout.tv_sec = rpttime / 10;
				timeout.tv_usec = (rpttime % 10) * 100000 + 33333;
				i = select(ConnectionNumber(display) + 1,
						&rfds, &wfds, &efds, &timeout);
				if (i > 0)
				{
					do
					{
						XNextEvent(display, &event);
					} while (event.type == MotionNotify && XPending(display));
				}
			} while (i > 0 && event.type == MotionNotify);
			if (i == 0)
			{
				event = rptevent;
			}
		}
		else
		{
			/* blink the cursor until an event is available */
			if (hasfocus && XPending(display) == 0 && o_blinktime > 0)
			{
				do
				{
					FD_ZERO(&rfds);
					FD_ZERO(&wfds);
					FD_ZERO(&efds);
					FD_SET(ConnectionNumber(display), &rfds);
					timeout.tv_sec = o_blinktime / 10;
					timeout.tv_usec = (o_blinktime % 10) * 100000;
					i = select(ConnectionNumber(display) + 1,
							&rfds, &wfds, &efds, &timeout);
					if (i == 0)
					{
						if (hasfocus->cursor == CURSOR_NONE)
							drawcursor(hasfocus);
						else
							erasecursor(hasfocus);
						XFlush(display);
					}
				} while (i == 0);

				/* make cursor visible while processing event */
				if (hasfocus->cursor == CURSOR_NONE)
					drawcursor(hasfocus);
			}

			/* read the next event */
			XNextEvent(display, &event);

			/* compress MotionNotify events */
			if (event.type == MotionNotify)
			{
				while (XEventsQueued(display, QueuedAfterReading) > 0)
				{
					XPeekEvent(display, &notify);
					if (notify.type != MotionNotify)
						break;
					XNextEvent(display, &event);
				}
			}
		}

		/* process the event */
		switch (event.type)
		{
		  case Expose:
			/* repaint window on Expose events */

			/* find the window */
			WIN2XW(event.xexpose.window, xw);

			/* toolbar is easy */
			if (event.xexpose.window == xw->toolw)
			{
				drawtoolbar(xw, True);
				break;
			}

			/* draw the image */
			xw->ismapped = True;
			xw->sbheight = 0;
			erasecursor(xw);
			top = event.xexpose.y / xw->cellh;
			left = event.xexpose.x / xw->cellw;
			bottom = (event.xexpose.y + event.xexpose.height) / xw->cellh;
			right = (event.xexpose.x + event.xexpose.width) / xw->cellw;
			if (bottom < 0)			/* just toolbar exposed? */
				bottom = 0;		/* must redraw something */
			if (left >= xw->columns)	/* just scrollbar exposed? */
				left = xw->columns - 1;	/* must redraw something */
			eventexpose((GUIWIN *)xw, top, left, bottom, right);
			xw->nextcursor = eventfocus((GUIWIN *)xw);
			drawcursor(xw);
			break;

		  case MapNotify:
			WIN2XW(event.xexpose.window, xw);
			xw->ismapped = True;
			if (o_focusnew)
				(void)focusgw((GUIWIN *)xw);
			break;

		  case UnmapNotify:
			WIN2XW(event.xexpose.window, xw);
			xw->ismapped = False;
			break;

		  case GraphicsExpose:
			WIN2XW(event.xexpose.window, xw);
			top = event.xexpose.y / xw->cellh;
			left = event.xexpose.x / xw->cellw;
			bottom = (event.xexpose.y + event.xexpose.height) / xw->cellh;
			right = (event.xexpose.x + event.xexpose.width) / xw->cellw;
			eventexpose((GUIWIN *)xw, top, left, bottom, right);
			break;

		  case MappingNotify:
			/* process keyboard mapping changes */
			XRefreshKeyboardMapping(&event.xmapping);
			break;

		  case FocusIn:
			WIN2XW(event.xfocus.window, xw);
			hasfocus = xw;
			break;

		  case FocusOut:
			WIN2XW(event.xfocus.window, xw);
			if (xw == hasfocus)
				hasfocus = NULL;
			break;

		  case ButtonPress:
			/* process mouse-button presses */
			WIN2XW(event.xbutton.window, xw);
			now = event.xbutton.time;

			/* check for toolbar event */
			if (event.xbutton.window == xw->toolw)
			{
				/* determine which tool button was pressed */
				for (tool = toollist;
				     tool && (event.xbutton.x < tool->x
					|| tool->x + tool->width < event.xbutton.x);
				     tool = tool->next)
				{
				}

				/* if a non-disabled button was pressed... */
				if (tool && tool->excmd && xw->toolstate[tool->id] != 'f')
				{
					/* temporarily draw it "pushed in" */
					draw1tool(xw, tool, 'i');
					flush();

					/* execute its ex command */
					eventex((GUIWIN *)xw, tool->excmd, tool->safer);
					didcmd = True;
				}
				break;
			}

			/* If this is a text-area button press, then we'll
			 * need to know which character cell was clicked.
			 * It's convenient to compute it here.
			 */
			ptry = event.xbutton.y / xw->cellh;
			ptrx = event.xbutton.x / xw->cellw;

			/* check for scrollbar event */
			xw->x = event.xbutton.x_root - event.xbutton.x;
			xw->y = event.xbutton.y_root - event.xbutton.y;
			if (ptrx >= xw->columns)
			{
				/* scrollbar event */
				rptevent = event;
				if (event.xbutton.y < o_scrollbarwidth + SB_GAP)
				{
					/* clicked on the up arrow */
					(void)eventscroll((GUIWIN *)xw, SCROLL_BACKLN, 1L, 0L);
					rpttime = (rpttime < 0 ? o_scrollbartime : 0);
					paging = True;
				}
				else if (event.xbutton.y >= xw->sbheight - (o_scrollbarwidth + SB_GAP))
				{
					/* clicked on the down arrow */
					(void)eventscroll((GUIWIN *)xw, SCROLL_FWDLN, 1L, 0L);
					rpttime = (rpttime < 0 ? o_scrollbartime : 0);
					paging = True;
				}
				else if (event.xbutton.y < xw->sbtop)
				{
					/* clicked before the thumb */
					(void)eventscroll((GUIWIN *)xw, SCROLL_BACKSCR, 1L, 0L);
					rpttime = (rpttime < 0 ? o_scrollbartime : 0);
					paging = True;
				}
				else if (event.xbutton.y > xw->sbbottom)
				{
					/* clicked after the thumb */
					(void)eventscroll((GUIWIN *)xw, SCROLL_FWDSCR, 1L, 0L);
					rpttime = (rpttime < 0 ? o_scrollbartime : 0);
					paging = True;
				}
				else
				{
					/* clicked on the thumb */
					prevptry = event.xbutton.y - xw->sbtop
						+ (o_scrollbarwidth + SB_GAP);
					prevptrx = -1;
					thumbing = True;
					rpttime = -1;
				}
				didcmd = True;
			}
			else if (now - firstclick > o_dblclicktime * 100
				|| event.xbutton.button == Button2
				|| prevptry != ptry
				|| prevptrx != ptrx)
			{
				/* single-click */
				if (event.xbutton.button != Button3)
				{
					(void)eventclick((GUIWIN *)xw, -1, -1, CLICK_CANCEL);
				}
				if (event.xbutton.button != Button2)
				{
					if (firstclick != 1
					 || prevptry != ptry
					 || prevptrx != ptrx)
					{
						offset = eventclick((GUIWIN *)xw, ptry, ptrx, CLICK_MOVE);
						didcmd |= (BOOLEAN)(offset >= 0);
					}
					firstclick = now;
				}
				if (event.xbutton.button == Button3)
				{
					(void)eventclick((GUIWIN *)xw, -1, -1, CLICK_YANK);
				}
				prevptry = ptry;
				prevptrx = ptrx;
				didcmd = True;
			}
			else
			{
				/* double-click */
				(void)eventclick((GUIWIN *)xw, -1, -1,
					event.xbutton.button == Button1 ? CLICK_TAG : CLICK_UNTAG);
				firstclick = 1;
				didcmd = True;
			}
			break;

		  case MotionNotify:
			/* ignore if wiggly click on a scrollbar button */
			if (paging)
				break;

			/* process mouse movement while button held down */
			WIN2XW(event.xbutton.window, xw);
			now = event.xmotion.time;
			xw->x = event.xbutton.x_root - event.xbutton.x;
			xw->y = event.xbutton.y_root - event.xbutton.y;

			/* moving the scrollbar's thumb? */
			if (thumbing)
			{
				/* compute the percentage */
				ptry = event.xbutton.y - prevptry + 1;
				i = xw->sbheight - 2 * (o_scrollbarwidth + SB_GAP);
				if (i <= 0)
					break;
				if (ptry < 0)
					ptry = 0;
				else if (ptry > i)
					ptry = i;
				if (ptry != prevptrx)
				{
					(void)eventscroll((GUIWIN *)xw, SCROLL_PERCENT, (long)ptry, (long)i);
					didcmd = True;
					prevptrx = ptry;
				}
				break;
			}

			/* convert to character cell coordinates */
			ptry = event.xbutton.y / xw->cellh;
			ptrx = event.xbutton.x / xw->cellw;

			/* ignore wigglies when clicking on the scrollbar */
			if (ptrx >= xw->columns)
			{
				break;
			}

			/* if not the same cell as last time... */
			if (ptry != prevptry || ptrx != prevptrx)
			{
				offset = eventclick((GUIWIN *)xw, ptry, ptrx, CLICK_MOVE);
				if (offset >= 0)
				{
					/* If moved off original character, start marking */
					if (!marking &&
						0 <= eventclick((GUIWIN *)xw, prevptry, prevptrx,
						  (event.xmotion.state & Button2Mask) ? CLICK_SELRECT :
						  (event.xmotion.state & Button3Mask) ? CLICK_SELLINE :
						  CLICK_SELCHAR))
					{
						(void)eventclick((GUIWIN *)xw, ptry, ptrx, CLICK_MOVE);
						marking = True;
					}
					prevptry = ptry;
					prevptrx = ptrx;
					didcmd = True;
				}
			}
			break;

		  case ButtonRelease:
			/* find the guix11 window structure */
			WIN2XW(event.xbutton.window, xw);
			now = event.xbutton.time;
			xw->x = event.xbutton.x_root - event.xbutton.x;
			xw->y = event.xbutton.y_root - event.xbutton.y;

			/* Is this the end of a mark, or end of a click? */
			if (thumbing)
			{
				/* nothing to do here */
			}
			else if (marking)
			{
				/* end of a mark - yank the selected text and
				 * then leave it marked.
				 */
				eventclick((GUIWIN *)xw, -1, -1, CLICK_YANK);
			}
			else if (rpttime >= 0)
			{
				/* stop repeating a scroll button press */
				rpttime = -1;
			}
			else
			{
				/* end of a click - depends on button */
				if (event.xbutton.state & Button2Mask)
				{
					/* paste from "< buffer */
					eventclick((GUIWIN *)xw, -1, -1, CLICK_PASTE);
					didcmd = True;
				}
			}
#if 0
			prevptrx = prevptry = -1;
#endif
			thumbing = marking = paging = False;
			break;

		  case KeyPress:
			/* get some standard info from the event */
			WIN2XW(event.xkey.window, xw);
			now = event.xkey.time;
			xw->x = event.xkey.x_root - event.xkey.x;
			xw->y = event.xkey.y_root - event.xkey.y;

			/* check for modifier keys */
			if (event.xkey.state & Mod1Mask)
				modifier = o_altkey;
			else
				modifier = '\0';

			/* convert the keypress to a KeySym and string */
			i = XLookupString(&event.xkey, text, sizeof text, &mykey, 0);
#if 1
			/* THIS IS A HACK!  Some versions of XFree86 come with
			 * a default map which causes the backspace keycode to
			 * be translated to XK_Delete instead of XK_BackSpace.
			 * This causes big problems for elvis, since elvis would
			 * like to make the <backspace> and <delete> keys do
			 * different things.  If the current keystroke appears
			 * to be a backspace keycode which has been mapped to
			 * XK_Delete, then we force it to be mapped to
			 * XK_BackSpace instead.
			 *
			 * This problem could also be solved by the user running
			 * `xmodmap -e "keycode 22 = BackSpace"` after starting
			 * the X server.
			 */
			if (mykey == XK_Delete && event.xkey.keycode == 22)
			{
				mykey = XK_BackSpace;
				text[0] = '\b';
				text[1] = '\0';
			}
#endif
			if (i == 0)
			{
				if (!IsModifierKey(mykey) && mykey != XK_Mode_switch)
				{
					/* function keys become a control sequence */
					sprintf(text, "%c%04x", ELVCTRL('K'), (int)mykey);
					i = strlen(text);
				}
			}
			if (i > 0)
			{
				switch (modifier)
				{
				  case 'c':
					(void)eventkeys((GUIWIN *)xw, toCHAR("\017"), 1);
					break;

				  case 's':
					if (i == 1) text[0] |= 0x80;
					break;

				  default: ;/* do nothing */
				}
				state = eventkeys((GUIWIN *)xw, toCHAR(text), i);

				/* The "xw" window may have been deleted during
				 * the eventkeys() call above.  If the window
				 * still exists then we have some work to do.
				 */
#if 1
				WIN2XW(event.xkey.window, xw);
#else
				for (xw = winlist; xw && xw->window != event.xkey.window && ; xw = xw->next)
				{
				}
				if (xw)
#endif
				{
					/* user maps never time out (Darn!) but
					 * key maps timeout immediately.
					 */
					xw->state = state;
					if (xw->state == MAP_KEY)
					{
						xw->state = eventkeys((GUIWIN *)xw, toCHAR(text), 0);
					}
					didcmd = True;
				}
			}
			break;

		  case ConfigureNotify:
			WIN2XW(event.xconfigure.window, xw);
			i = (event.xconfigure.width - o_scrollbarwidth) / xw->cellw;
			j = (event.xconfigure.height - TB_HEIGHT) / xw->cellh;
			if (i != xw->columns || j != xw->rows)
			{
				xw->columns = i;
				xw->rows = j;
				XResizeWindow(display, xw->textw,
					(unsigned)event.xconfigure.width,
					(unsigned)(event.xconfigure.height - TB_HEIGHT));
				if (o_toolbar)
				{
					XResizeWindow(display, xw->toolw,
						(unsigned)event.xconfigure.width,
						(unsigned)TB_HEIGHT);
				}
				eventresize((GUIWIN *)xw, (int)xw->rows, (int)xw->columns);
				didcmd = True;
			}
			break;

		  case ClientMessage:
			/* if WM_DELETE_WINDOW from the window manager, then
			 * destroy the X11 window.  The server will then send
			 * us a DestroyNotify message so we can finish
			 * cleaning up.
			 */
			if (event.xclient.message_type == wm_protocols
			 && event.xclient.format == 32
			 && event.xclient.data.l[0] == wm_delete_window
			 && winlist->next)
			{
				XDestroyWindow(display, event.xclient.window);
			}
			break;

		  case DestroyNotify:
			/* fetch id of window to destroy */
			offset = (event.type == DestroyNotify
				? event.xdestroywindow.window
				: event.xclient.window);
	
			/* locate the corresponding X11WIN structure */
			WIN2XW(offset, xw);

			/* The X window itself is already gone */
			xw->window = 0;

			/* other stuff still needs to be taken care of */
			destroygw((GUIWIN *)xw, True);
			break;

		  case PropertyNotify:
			WIN2XW(event.xproperty.window, xw);
			if (event.xproperty.atom == elvis_server
				&& state == PropertyNewValue)
			{
				XGetWindowProperty(display,
					xw->window, elvis_server, 0L, 8192,
					True, XA_STRING, &gottype,
					&i, (unsigned long *)&exlen,
					(unsigned long *)&ldummy,
					(unsigned char **)&excmd);
				if (exlen > 0)
				{
					if (gottype == XA_STRING && i == 8)
					{
						excmd[exlen] = '\0';
						eventex((GUIWIN *)xw, excmd, True);
						didcmd = True;
					}
					XFree(excmd);
				}

				/* never leave old data in the property */
				if (ldummy > 0)
				{
					XDeleteProperty(display, xw->window, elvis_server);
				}
			}
			break;

		  case SelectionClear:
			/* We may have lost our old selection yet still be
			 * responsible for the new selection.  Check to see
			 * if we still own the current selection.
			 */
			owner = XGetSelectionOwner(display, XA_PRIMARY);
			for (xw = winlist; xw && xw->window != owner; xw = xw->next)
			{
			}
			if (ownselection && !xw)
			{
				/* free the selection */
				if (clipbuf)
				{
					safefree(clipbuf);
					clipbuf = NULL;
				}
				ownselection = False;
				drawcursor(NULL);
			}
			break;

		  case SelectionRequest:
			/* create a SelectionNotify event for requestor */
			notify.type = SelectionNotify;
			notify.xselection.requestor = event.xselectionrequest.requestor;
			notify.xselection.selection = event.xselectionrequest.selection;
			notify.xselection.target = event.xselectionrequest.target;
			notify.xselection.time = event.xselectionrequest.time;

			/* try to convert the selection */
			if (event.xselectionrequest.selection == XA_PRIMARY
			 && event.xselectionrequest.target == XA_STRING)
			{
				/* store the selection's value into the property */
				XChangeProperty(display,
					event.xselectionrequest.requestor,
					event.xselectionrequest.property,
					event.xselectionrequest.target,
					8, PropModeReplace,
					(unsigned char *)(clipbuf ? clipbuf : ""),
					clipsize);
				notify.xselection.property = event.xselectionrequest.property;
			}
			else
			{
				/* can't convert */
				notify.xselection.property = None;
			}

			/* notify the requestor */
			XSendEvent(display, notify.xselection.requestor,
				False, 0L, &notify);
		}
	}
}


/* Test for signs of boredom from the user, so we can cancel long operations.
 * Here, we check to see if user has clicked on the window.
 */
static BOOLEAN wpoll(reset)
	BOOLEAN reset;
{
	XEvent	event;
	X11WIN	*scan;

	/* ignore if simply trying to reset */
	if (reset)
		return False;

	/* Check every window to see if it has a ButtonPress event pending */
	if (XPending(display) > 0
	 && XCheckTypedEvent(display, ButtonPress, &event))
	{
		return True;
	}

	/* Since we're polling, redraw all scrollbars to show stop signs so
	 * the user knows that a click will abort.
	 */
	for (scan = winlist; scan; scan = scan->next)
	{
		setsbstate(scan, SB_STOP);
	}

	return False;
}


/* dummy X11 error handler */
static int ignoreErrors(disp, err)
	Display		*disp;
	XErrorEvent	*err;
{
	return 0;
}

/* End the GUI.  For "termcap" this means switching the
 * the terminal back into "cooked" mode.  For "x11", the
 * window should be deleted and any other X resources freed.
 *
 * This function is called after all windows have been deleted
 * by delwin(), when elvis is about to terminate.
 */
static void term()
{
	Window	curfocus;
	int	dummy;

	/* unload any fonts */
	while (fonts)
	{
		unloadfont(fonts);
	}

	/* forget any tools */
	guicmd(NULL, "newtoolbar");

	/* warp the cursor back to the original (non-Elvis) window */
	if (fromwin != root
	 && (!XGetInputFocus(display, &curfocus, &dummy) || curfocus != fromwin))
	{
		if (o_warpback)
			XWarpPointer(display, None, fromwin, 0, 0, 0, 0, 0, 0);
		if (o_warpback || o_focusnew)
			XSetInputFocus(display, fromwin, RevertToParent, now);
	}

	/* delete the server property from the root window */
	XDeleteProperty(display, root, elvis_server);

	/* close the connection to the display */
	XCloseDisplay(display);
}


/* Create a new window for the buffer named name.  If successful,
 * return TRUE and then simulate a "create" event later.  Return
 * FALSE if the GUIWIN can't be created, e.g., because the GUI doesn't
 * support multiple windows.  The msg() function should be called to
 * describe the reason for the failure.
 */
static BOOLEAN creategw(name, attributes)
	char	*name;		/* name of buffer for the new window */
	char	*attributes;	/* other window parameters, if any */
{
	XSizeHints hint;
	XWMHints   wmhint;
	XClassHint class;
	XTextProperty textprops[2];
	X11WIN	   *xw;
	XGCValues  gcvalues;
	char	   *argv[5];
	int	   argc;

	/* is this the first time? */
	if (!winlist)
	{
		/* allocate the fonts named in options. */
		defaultnormal = loadfont(o_normalfont ? tochar8(o_normalfont) : DEFAULT_NORMALFONT);
		if (!defaultnormal) return False;
		if (o_boldfont && !(defaultbold = loadfont(tochar8(o_boldfont))))
			return False;
		if (o_italicfont && !(defaultitalic = loadfont(tochar8(o_italicfont))))
			return False;
		loadedcontrol = loadfont(o_controlfont ? tochar8(o_controlfont) : DEFAULT_CONTROLFONT);
		if (!loadedcontrol) return False;

		/* lock the options that can only be changed during initialization */
		optflags(o_normalfont) |= OPT_LOCK;
		optflags(o_boldfont) |= OPT_LOCK;
		optflags(o_italicfont) |= OPT_LOCK;
		optflags(o_scrollbarwidth) |= OPT_LOCK;
		optflags(o_icon) |= OPT_LOCK;
		optflags(o_toolbar) |= OPT_LOCK;
	}

	/* allocate storage space */
	xw = (X11WIN *)safealloc(1, sizeof(*xw));
	xw->title = safedup(name);
	xw->next = winlist;
	winlist = xw;

	/* default pixel values */
	white = xw->bg = WhitePixel(display, screen);
	black = xw->bg = BlackPixel(display, screen);
	xw->fgnormal = loadcolor(foreground, black);
	xw->fgfixed = loadcolor(fixedcolor, black);
	xw->fgbold = loadcolor(boldcolor, xw->fgnormal);
	xw->fgemph = loadcolor(emphcolor, xw->fgnormal);
	xw->fgitalic = loadcolor(italiccolor, xw->fgnormal);
	xw->fgundln = loadcolor(underlinecolor, xw->fgnormal);
	xw->fgcursor = loadcolor(cursorcolor, xw->fgnormal);
	xw->owncursor = loadcolor(owncolor, xw->fgnormal);
	xw->bgscroll = loadcolor(scrollbarbg, xw->fgnormal);
	xw->bgnormal = loadcolor(background, white);
	xw->fgscroll = loadcolor(scrollbarfg, xw->bgnormal);
	xw->grexpose = False;

	/* remember font metrics */
	xw->cellbase = defaultnormal->fontinfo->ascent;
	xw->cellh = defaultnormal->height;
	xw->cellw = defaultnormal->fontinfo->max_bounds.width;

	/* default window geometry */
	xw->rows = o_xrows;
	xw->columns = o_xcolumns;
	xw->cursx = xw->cursy = 0;

	/* initial scrollbar appearance */
	xw->sbheight = xw->rows * xw->cellh;
	xw->sbtop = o_scrollbarwidth + SB_GAP;
	xw->sbbottom = xw->sbheight - (o_scrollbarwidth + SB_GAP);
	xw->sbstate = SB_BLANK;

	/* default window size */
	hint.x = hint.y = 0;
	hint.width = xw->columns * xw->cellw + o_scrollbarwidth;
	hint.height = xw->rows * xw->cellh + TB_HEIGHT;
	hint.base_width = o_scrollbarwidth;
	hint.base_height = TB_HEIGHT;
	hint.width_inc = xw->cellw;
	hint.height_inc = xw->cellh;
	hint.min_width = 20 * xw->cellw + o_scrollbarwidth;
	hint.min_height = 4 * xw->cellh;
	hint.max_width = 200 * xw->cellw + o_scrollbarwidth;
	hint.max_height = 200 * xw->cellh;
	hint.flags = PSize | PBaseSize | PMinSize | PResizeInc;

	/* maybe set the window position, too */
	if (!xw->next && (optflags(o_firstx) & OPT_SET) != 0)
	{
		/* first window -- use firstx & firsty */
		xw->x = hint.x = (o_firstx >= 0) ? o_firstx : rootwidth + o_firstx - hint.width - 6;
		xw->y = hint.y = (o_firsty >= 0) ? o_firsty : rootheight + o_firsty - hint.height - 12;
		hint.flags |= PPosition | USPosition;
	}
	else if (o_stagger > 0 && xw->next)
	{
		/* not first window -- use stagger */
		xw->x = hint.x = xw->next->x + o_stagger;
		if (hint.x + hint.width >= rootwidth)
			xw->x = hint.x = 0;
		xw->y = hint.y = xw->next->y + o_stagger;
		if (hint.y + hint.height >= rootheight)
			xw->y = hint.y = 0;
		hint.flags |= PPosition | USPosition;
	}
	else
	{
		/* probably have to position it manually -- don't warp pointer*/
		xw->nowarp = True;
	}

	/* window creation */
	xw->window = XCreateSimpleWindow(display, root,
		hint.x, hint.y, (unsigned)hint.width, (unsigned)hint.height,
		5, xw->fgnormal, xw->fgscroll);
	xw->textw = XCreateSimpleWindow(display, xw->window, 0, TB_HEIGHT,
		(unsigned)hint.width, (unsigned)(xw->rows * xw->cellh),
		0, xw->fgnormal, xw->bgnormal);
	if (o_toolbar)
	{
		xw->toolw = XCreateSimpleWindow(display, xw->window, 0, 0,
			(unsigned)hint.width, (unsigned)TB_HEIGHT,
			0, xw->fgnormal, xw->bgscroll);
	}

	/* Set the standard properties */
	argv[0] = xw->title;
	XStringListToTextProperty(argv, 1, &textprops[0]);
	argv[0] = dirfile(xw->title);
	XStringListToTextProperty(argv, 1, &textprops[1]);
	argc = 0;
	argv[argc++] = argv0;
	if (o_session)
	{
		argv[argc++] = "-s";
		argv[argc++] = tochar8(o_session);
	}
	wmhint.input = True;
	wmhint.initial_state = (o_iconic && !xw->next) ? IconicState : NormalState;
	if (o_icon)
	{
		wmhint.icon_pixmap = elvis_icon;
		wmhint.flags = InputHint | StateHint | IconPixmapHint;
	}
	else
	{
		wmhint.flags = InputHint | StateHint;
	}
	XSetWMProperties(display, xw->window, &textprops[0], &textprops[1], argv, argc, &hint, &wmhint, NULL);
	XFree(textprops[0].value);
	XFree(textprops[1].value);

	/* set class hints */
	class.res_name = "elvis";
	class.res_class = "Elvis";
	XSetClassHint(display, xw->window, &class);

	/* allow window manager's "Delete" menu item to work */
	XSetWMProtocols(display, xw->window, &wm_delete_window, 1);
	
	/* GC creation and initialization */
	gcvalues.foreground = xw->fg;
	gcvalues.background = xw->bg;
	gcvalues.font = defaultnormal->fontinfo->fid;
	gcvalues.graphics_exposures = xw->grexpose;
	xw->gc = XCreateGC(display, xw->window,
		GCForeground|GCBackground|GCFont|GCGraphicsExposures, &gcvalues);

	/* pixmap creation, for storing image of character under cursor */
	xw->undercurs = XCreatePixmap(display, xw->window, xw->cellw, xw->cellh, (unsigned)depth);
	xw->cursor = CURSOR_NONE;
	xw->nextcursor = CURSOR_QUOTE;

	/* make it work as an elvis server window */
	XChangeProperty(display, root, elvis_server, XA_WINDOW, 32,
		PropModeReplace, (unsigned char *)&xw->window, 1);

	/* input event selection */
	XSelectInput(display, xw->window,
	    KeyPressMask|KeyReleaseMask|FocusChangeMask|StructureNotifyMask|PropertyChangeMask);
	XSelectInput(display, xw->textw,
	    ButtonPressMask|ButtonMotionMask|ButtonReleaseMask|ExposureMask);
	if (o_toolbar)
	{
		XSelectInput(display, xw->toolw, ButtonPressMask|ExposureMask);
	}

	/* window mapping */
	XMapSubwindows(display, xw->window);
	XMapRaised(display, xw->window);
	xw->ismapped = (BOOLEAN)(wmhint.initial_state == NormalState);

	/* simulate a "window create" event */
	eventcreate((GUIWIN *)xw, NULL, name, (int)xw->rows, (int)xw->columns);

	return True;
}


/* Change the title of the window.  This function is called when a
 * buffer's name changes, or different buffer becomes associated with
 * a window.  The name argument is the new buffer name.
 */
static void retitle(gw, name)
	GUIWIN		*gw;	/* the window to be retitled */
	char		*name;	/* the new title of the window */
{
	X11WIN		*xw = (X11WIN *)gw;
	XTextProperty	textprop;

	/* free the old title, remember the new title */
	safefree(xw->title);
	xw->title = safedup(name);

	/* inform the window manager of the new name */
	XStringListToTextProperty(&name, 1, &textprop);
	XSetWMName(display, xw->window, &textprop);
	XFree(textprop.value);

	/* also change the icon name */
	name = dirfile(name);
	XStringListToTextProperty(&name, 1, &textprop);
	XSetWMIconName(display, xw->window, &textprop);
	XFree(textprop.value);
}


/* Flush all changes out to the screen */
static void flush()
{
	XFlush(display);
}


/* Move the cursor to a given character cell.  The upper left
 * character cell is designated column 0, row 0.
 */
static void moveto(gw, column, row)
	GUIWIN	*gw;	/* the window whose cursor is to be moved */
	int	column;	/* the column to move to */
	int	row;	/* the row to move to */
{
	X11WIN	*xw = (X11WIN *)gw;

	if (xw->cursx != column || xw->cursy != row)
	{
		erasecursor(xw);
		xw->cursx = column;
		xw->cursy = row;
	}
}


/* Displays text on the screen, starting at the cursor's
 * current position, in the given font.  The text string is
 * guaranteed to contain only printable characters.
 *
 * The font is indicated by a single letter.  The letter will
 * be lowercase normally, or uppercase to indicate that the
 * text should be visibly marked for the <v> and <V> commands.
 * The letters are:
 *	n/N	normal characters
 *	b/B	bold characters
 *	i/I	italic characters
 *	u/U	underlined characters
 *	g/G	graphic characters
 *	s	standout (used for messages on bottom row)
 *	p	popup menu
 *
 * This function should move the text cursor to the end of
 * the output text.
 */
static void draw(gw, font, text, len)
	GUIWIN	*gw;	/* the window where the text should be drawn */
	_char_	font;	/* the font code to use for drawing */
	CHAR	*text;	/* the text to draw */
	int	len;	/* number of characters in text */
{
	X11WIN		*xw;
	long		swapper;
	LOADEDFONT	*loaded;
	CHAR		*tmp = NULL;
	XGCValues	gcvalues;
	int		i;

	xw = (X11WIN *)gw;
	xw->cursor = CURSOR_NONE;

	/* set the font & colors */
	switch (font)
	{
	  case 'b':
	  case 'B':
		gcvalues.foreground = xw->fgbold;
		loaded = defaultbold;
		break;

	  case 'e':
	  case 'E':
		gcvalues.foreground = xw->fgemph;
		loaded = defaultbold;
		break;

	  case 'i':
	  case 'I':
		gcvalues.foreground = xw->fgitalic;
		loaded = defaultitalic;
		break;

	  case 'g':
	  case 'G':
		gcvalues.foreground = xw->fgnormal;
		loaded = defaultnormal;
		tmp = safealloc(len, sizeof(CHAR));
		for (i = 0; i < len; i++)
		{
			if (loaded->fontinfo->min_char_or_byte2 <= '\013')
			{
				switch (text[i])
				{
				  case '-':	tmp[i] = GCH;	break;
				  case '|':	tmp[i] = GCV;	break;
				  case '1':	tmp[i] = GC1;	break;
				  case '2':	tmp[i] = GC2;	break;
				  case '3':	tmp[i] = GC3;	break;
				  case '4':	tmp[i] = GC4;	break;
				  case '5':	tmp[i] = GC5;	break;
				  case '6':	tmp[i] = GC6;	break;
				  case '7':	tmp[i] = GC7;	break;
				  case '8':	tmp[i] = GC8;	break;
				  case '9':	tmp[i] = GC9;	break;
				  default:	tmp[i] = text[i];
				}
			}
			else if (isdigit(text[i]))
			{
				tmp[i] = '+';
			}
			else
			{
				tmp[i] = text[i];
			}
		}
		text = tmp;
		break;

	  case 'u':
	  case 'U':
		gcvalues.foreground = xw->fgundln;
		loaded = defaultnormal;
		break;

	  case 'f':
	  case 'F':
		gcvalues.foreground = xw->fgfixed;
		loaded = defaultnormal;
		break;

	  default:
		gcvalues.foreground = xw->fgnormal;
		loaded = defaultnormal;
	}
	gcvalues.background = xw->bgnormal;

	/* if font letter is uppercase, then swap foreground & background */
	if (isupper(font))
	{
		swapper = gcvalues.foreground;
		gcvalues.foreground = gcvalues.background;
		gcvalues.background = swapper;
	}

	/* set the GC values */
	gcvalues.font = (loaded ? loaded : defaultnormal)->fontinfo->fid;
	gcvalues.graphics_exposures = xw->grexpose = False;
	XChangeGC(display, xw->gc, 
		GCForeground|GCBackground|GCFont|GCGraphicsExposures, &gcvalues);
	xw->fg = gcvalues.foreground;
	xw->bg = gcvalues.background;

	/* draw the text */
	XDrawImageString(display, xw->textw, xw->gc,
		(int)(xw->cursx * xw->cellw), (int)(xw->cursy * xw->cellh + xw->cellbase),
		tochar8(text), len);
	if ((font == 'b' || font == 'B' || font == 'e' || font == 'E') && !defaultbold)
	{
		XDrawString(display, xw->textw, xw->gc,
			(int)(xw->cursx * xw->cellw + 1), (int)(xw->cursy * xw->cellh + xw->cellbase),
			tochar8(text), len);
	}
	if (font == 'u' || font == 'U')
	{
		XFillRectangle(display, xw->textw, xw->gc,
			(int)(xw->cursx * xw->cellw), (int)((xw->cursy + 1) * xw->cellh - 1),
			len * xw->cellw, 1);
	}
	if ((font == 'i' || font == 'I') && !defaultitalic)
	{
		XCopyArea(display, xw->textw, xw->textw, xw->gc,
			(int)(xw->cursx * xw->cellw), (int)(xw->cursy * xw->cellh),
			len * xw->cellw - 1, (xw->cellh + 1) / 2,
			(int)(xw->cursx * xw->cellw + 1), (int)(xw->cursy * xw->cellh));
	}

	/* free the temp string (if any) */
	if (tmp)
	{
		safefree(tmp);
	}

	/* leave the cursor after the text */
	xw->cursx += len;
}

/* Insert "qty" characters into the current row, starting at
 * the current cursor position.  A negative "qty" value means
 * that characters should be deleted.
 *
 * This function is optional.  If omitted, elvis will rewrite
 * the text that would have been shifted.
 */
static BOOLEAN shift(gw, qty, rows)
	GUIWIN	*gw;	/* window to be shifted */
	int	qty;	/* amount to shift by */
	int	rows;	/* number of rows affected */
{
	X11WIN	*xw = (X11WIN *)gw;

#if 1
	/* erase the cursor */
	erasecursor(xw);
#endif

	/* make sure we have the right background */
	if (!xw->grexpose)
	{
		XSetGraphicsExposures(display, xw->gc, True);
		xw->grexpose = True;
	}
	if (xw->bg != xw->bgnormal)
	{
		XSetBackground(display, xw->gc, xw->bgnormal);
		xw->bg = xw->bgnormal;
	}

	if (qty > 0)
	{
		/* we'll be inserting */

		/* shift the characters */
		XCopyArea(display, xw->textw, xw->textw, xw->gc,
			(int)(xw->cursx * xw->cellw), (int)(xw->cursy * xw->cellh),
			xw->cellw * (xw->columns - xw->cursx - qty), xw->cellh * rows,
			(int)((xw->cursx + qty) * xw->cellw), (int)(xw->cursy * xw->cellh));
	}
	else
	{
		/* we'll be deleting.  Convert qty to absolute value. */
		qty = -qty;

		/* shift the characters */
		XCopyArea(display, xw->textw, xw->textw, xw->gc,
			(int)((xw->cursx + qty) * xw->cellw), (int)(xw->cursy * xw->cellh),
			xw->cellw * (xw->columns - xw->cursx - qty), xw->cellh * rows,
			(int)(xw->cursx * xw->cellw), (int)(xw->cursy * xw->cellh));
	}

	return True;
}

static BOOLEAN scroll(gw, qty, notlast)
	GUIWIN	*gw;	/* window to be scrolled */
	int	qty;	/* amount to scroll by (pos=downward, neg=upward) */
	BOOLEAN	notlast;/* if True, last row should not be affected */
{
	X11WIN	*xw = (X11WIN *)gw;
	int	rows;

#if 1
	/* erase the cursor */
	erasecursor(xw);
#endif

	/* decide how many rows to scroll */
	rows = xw->rows;
	if (notlast)
	{
		rows--;
	}

	/* make sure we have the right background */
	if (!xw->grexpose)
	{
		XSetGraphicsExposures(display, xw->gc, True);
		xw->grexpose = True;
	}
	if (xw->bg != xw->bgnormal)
	{
		XSetBackground(display, xw->gc, xw->bgnormal);
		xw->bg = xw->bgnormal;
	}

	if (qty > 0)
	{
		/* we'll be inserting */

		/* shift the rows */
		XCopyArea(display, xw->textw, xw->textw, xw->gc,
			0, (int)(xw->cursy * xw->cellh),
			xw->cellw * xw->columns, xw->cellh * (rows - xw->cursy - qty),
			0, (int)((xw->cursy + qty) * xw->cellh));
	}
	else
	{
		/* we'll be deleting.  Convert qty to absolute value. */
		qty = -qty;

		/* shift the rows */
		XCopyArea(display, xw->textw, xw->textw, xw->gc,
			0, (int)((xw->cursy + qty) * xw->cellh),
			xw->cellw * xw->columns, xw->cellh * (rows - xw->cursy - qty),
			0, (int)(xw->cursy * xw->cellh));
	}

	return True;
}

static BOOLEAN clrtoeol(gw)
	GUIWIN	*gw;	/* window whose row is to be cleared */
{
	X11WIN	*xw = (X11WIN *)gw;

	/* make sure we have the right background */
	if (xw->fg != xw->bgnormal)
	{
		XSetForeground(display, xw->gc, xw->bgnormal);
		xw->fg = xw->bgnormal;
	}
	if (xw->grexpose)
	{
		XSetGraphicsExposures(display, xw->gc, False);
		xw->grexpose = False;
	}

	/* whether or not the cursor was visible before, it'll be invisible
	 * after we erase the line.
	 */
	xw->cursor = CURSOR_NONE;

	/* erase the line, from the cursor to the right edge */
	XFillRectangle(display, xw->textw, xw->gc,
		(int)(xw->cursx * xw->cellw), (int)(xw->cursy * xw->cellh),
		(xw->columns - xw->cursx) * xw->cellw, xw->cellh);

	return True;
}

/* Ring the bell */
static void beep(gw)
	GUIWIN	*gw;	/* window that generated a beep */
{
	XBell(display, 0);
}


/* draw a bevelled rectangle */
static void bevelrect(xw, win, x, y, w, h, in)
	X11WIN		*xw;	/* top-level app window to draw in */
	Window		win;	/* widget where drawing should occur */
	int		x, y;	/* upper-left corner of the rectangle */
	unsigned	w, h;	/* width and height of the rectangle */
	BOOLEAN		in;	/* swap edge colors so it appears pushed in? */
{
	XGCValues	gcv;
#if SB_BEVEL >= 1
	int		b, r;	/* bottom & right edges */
	unsigned long	topleft;
	unsigned long	bottomright;
	int		i;
#endif

	/* draw the button's face */
	gcv.foreground = xw->fgscroll;
	gcv.fill_style = FillSolid;
	XChangeGC(display, xw->gc, GCForeground|GCFillStyle, &gcv);
	xw->fg = xw->fgscroll;
	XFillRectangle(display, win, xw->gc, x, y, w, h);

#if SB_BEVEL >= 1
	/* choose the edge colors */
	if (in)
	{
		topleft = xw->fgnormal;
		bottomright = xw->bgnormal;
	}
	else
	{
		topleft = xw->bgnormal;
		bottomright = xw->fgnormal;
	}

	/* locate the bottom & right edges */
	b = y + h - 1;
	r = x + w - 1;

	/* draw the outermost bevel sides */
	for (i = SB_BEVEL; i > 0; i--)
	{
		/* draw the top & left bevel edges */
		XSetForeground(display, xw->gc, topleft);
		XDrawLine(display, win, xw->gc, x, y, r, y);
		XDrawLine(display, win, xw->gc, x, y, x, b);

		/* draw the bottom & right bevel edges */
		XSetForeground(display, xw->gc, bottomright);
		xw->fg = bottomright;
		XDrawLine(display, win, xw->gc, x, b, r, b);
		XDrawLine(display, win, xw->gc, r, y, r, b);

		/* move the edges in slightly, to continue the bevel */
		x++;
		y++;
		r--;
		b--;
	}
#endif /* SB_BEVEL >= 2 */
}


/* draw the scrollbar */
static void scrollbar(gw, top, bottom, total)
	GUIWIN	*gw;	/* window whose scrollbar should be updated */
	long	top;	/* offset of char at top of screen */
	long	bottom;	/* offset of char at bottom of screen */
	long	total;	/* total number of characters in buffer */
{
	X11WIN	*xw = (X11WIN *)gw;
	int	newtop;		/* top of thumb */
	int	newbottom;	/* bottom of thumb */
	int	newheight;	/* total height of window */
	int	newthumb;	/* height of thumb scrolling region */
	XGCValues gcv;		/* GC Values */
	XPoint	vertex[3];	/* points of arrowhead */

	/* compute new scrollbar regions */
	newheight = xw->cellh * xw->rows;
	if (total <= 0)
	{
		newtop = xw->sbtop;
		newbottom = xw->sbbottom;
		xw->sbstate = SB_REDRAW;
	}
	else
	{
		if (newheight < 1 + 2 * (o_scrollbarwidth + SB_GAP))
			newheight = 1 + 2 * (o_scrollbarwidth + SB_GAP);
		newthumb = newheight - 2 * (o_scrollbarwidth + SB_GAP);
		newtop = o_scrollbarwidth + SB_GAP + top * newthumb / total;
		newbottom = o_scrollbarwidth +  SB_GAP + bottom * newthumb / total;

		/* make sure the thumb doesn't disappear completely */
		if (newbottom - newtop < 2)
			newbottom = newtop + 2;

		/* make sure the thumb doesn't go outside the scroll area */
		if (newbottom >= newheight - (o_scrollbarwidth + SB_GAP))
		{
			newtop -= (newbottom - (newheight - (o_scrollbarwidth + SB_GAP)));
			newbottom = newheight - (o_scrollbarwidth + SB_GAP);
		}
		if (newtop < o_scrollbarwidth + SB_GAP)
			newtop = o_scrollbarwidth + SB_GAP;
	}

	/* if geometries haven't changed, then don't bother redrawing */
	if (newheight == xw->sbheight
		&& newtop == xw->sbtop
		&& newbottom == xw->sbbottom
		&& xw->sbstate == SB_NORMAL)
	{
		return;
	}

#if 1
	/* if not supposed to change, then don't -- but remember shape anyway */
	if (xw->sbstate != SB_NORMAL && xw->sbstate != SB_REDRAW)
	{
		xw->sbheight = newheight;
		xw->sbtop = newtop;
		xw->sbbottom = newbottom;
		return;
	}
#endif

	/* choose the erase color */
	if (xw->fgscroll == xw->bgscroll)
	{
		/* foreground & background are the same - use a fill
		 * pattern.
		 */
		gcv.foreground = xw->fgnormal;
		gcv.background = xw->bgnormal;
		gcv.fill_style = FillOpaqueStippled;
		gcv.stipple = elvis_icon;
		XChangeGC(display, xw->gc,
			GCForeground|GCBackground|GCFillStyle|GCStipple, &gcv);
	}
	else
	{
		/* foreground & background are different */
		XSetForeground(display, xw->gc, xw->bgscroll);
	}

	if (newheight == xw->sbheight && xw->sbstate == SB_NORMAL)
	{
		/* erase the old thumb */
		XFillRectangle(display, xw->textw, xw->gc, (int)(xw->cellw * xw->columns + SB_GAP),
			xw->sbtop, (unsigned)(o_scrollbarwidth - 2 * SB_GAP + 1), (unsigned)(xw->sbbottom - xw->sbtop + 1));
	}
	else
	{
		/* erase the old scrollbar */
		XFillRectangle(display, xw->textw, xw->gc, (int)(xw->cellw * xw->columns),
			0, (unsigned)o_scrollbarwidth, xw->cellh * xw->rows);
		
		/* redraw the borders */
		gcv.foreground = xw->fgnormal;
		gcv.fill_style = FillSolid;
		XChangeGC(display, xw->gc, GCForeground|GCFillStyle, &gcv);
		XDrawLine(display, xw->textw, xw->gc,
			(int)(xw->cellw * xw->columns), 0,
			(int)(xw->cellw * xw->columns), newheight - 1);
		XDrawLine(display, xw->textw, xw->gc,
			(int)(xw->cellw * xw->columns), 0,
			(int)(xw->cellw * xw->columns + o_scrollbarwidth - 1), 0);
		XSetForeground(display, xw->gc, xw->bgnormal);
		XDrawLine(display, xw->textw, xw->gc,
			(int)(xw->cellw * xw->columns + o_scrollbarwidth - 1), 0,
			(int)(xw->cellw * xw->columns + o_scrollbarwidth - 1), newheight - 1);
		XDrawLine(display, xw->textw, xw->gc,
			(int)(xw->cellw * xw->columns), newheight - 1,
			(int)(xw->cellw * xw->columns + o_scrollbarwidth - 1), newheight - 1);

		/* draw the "up" arrowhead */
		vertex[0].x = xw->cellw * xw->columns + SB_GAP;
		vertex[0].y = o_scrollbarwidth; /* lower left */
		vertex[1].x = vertex[0].x + o_scrollbarwidth / 2 - SB_GAP;
		vertex[1].y = SB_GAP;		/* top */
		vertex[2].x = vertex[0].x + o_scrollbarwidth - 2 * SB_GAP;
		vertex[2].y = vertex[0].y;	/* lower right */
		XSetForeground(display, xw->gc, xw->fgscroll);
		XFillPolygon(display, xw->textw, xw->gc, vertex, QTY(vertex),
			Convex, CoordModeOrigin);
#if SB_BEVEL >= 2
		XSetForeground(display, xw->gc, xw->fgnormal);
		XDrawLine(display, xw->textw, xw->gc,
			vertex[1].x, vertex[1].y+1, vertex[2].x-1, vertex[2].y-1);
		XDrawLine(display, xw->textw, xw->gc,
			vertex[2].x-1, vertex[2].y-1, vertex[0].x+1, vertex[0].y-1);
		XSetForeground(display, xw->gc, xw->bgnormal);
		XDrawLine(display, xw->textw, xw->gc,
			vertex[0].x+1, vertex[0].y-1, vertex[1].x, vertex[1].y+1);
#endif /* SB_BEVEL >= 2 */
#if SB_BEVEL >= 1
		XSetForeground(display, xw->gc, xw->fgnormal);
		XDrawLine(display, xw->textw, xw->gc,
			vertex[1].x, vertex[1].y, vertex[2].x, vertex[2].y);
		XDrawLine(display, xw->textw, xw->gc,
			vertex[2].x, vertex[2].y, vertex[0].x, vertex[0].y);
		XSetForeground(display, xw->gc, xw->bgnormal);
		XDrawLine(display, xw->textw, xw->gc,
			vertex[0].x, vertex[0].y, vertex[1].x, vertex[1].y);
#endif /* SB_BEVEL >= 1 */

		/* draw the "down" arrowhead */
		vertex[0].x = xw->cellw * xw->columns + SB_GAP; /* top left */
		vertex[0].y = xw->cellh * xw->rows - o_scrollbarwidth;
		vertex[1].x = vertex[0].x + o_scrollbarwidth - 2 * SB_GAP;
		vertex[1].y = vertex[0].y;			/* top right */
		vertex[2].x = vertex[0].x + o_scrollbarwidth / 2 - SB_GAP;
		vertex[2].y = xw->cellh * xw->rows - SB_GAP;	/* bottom */
		XSetForeground(display, xw->gc, xw->fgscroll);
		XFillPolygon(display, xw->textw, xw->gc, vertex, QTY(vertex),
			Convex, CoordModeOrigin);
#if SB_BEVEL >= 2
		XSetForeground(display, xw->gc, xw->fgnormal);
		XDrawLine(display, xw->textw, xw->gc,
			vertex[1].x-1, vertex[1].y+1, vertex[2].x, vertex[2].y-1);
		XSetForeground(display, xw->gc, xw->bgnormal);
		XDrawLine(display, xw->textw, xw->gc,
			vertex[0].x+1, vertex[0].y+1, vertex[1].x-1, vertex[1].y+1);
		XDrawLine(display, xw->textw, xw->gc,
			vertex[2].x, vertex[2].y-1, vertex[0].x+1, vertex[0].y+1);
#endif /* SB_BEVEL >= 2 */
#if SB_BEVEL >= 1
		XSetForeground(display, xw->gc, xw->fgnormal);
		XDrawLine(display, xw->textw, xw->gc,
			vertex[1].x, vertex[1].y, vertex[2].x, vertex[2].y);
		XSetForeground(display, xw->gc, xw->bgnormal);
		XDrawLine(display, xw->textw, xw->gc,
			vertex[2].x, vertex[2].y, vertex[0].x, vertex[0].y);
		XDrawLine(display, xw->textw, xw->gc,
			vertex[0].x, vertex[0].y, vertex[1].x, vertex[1].y);
#endif /* SB_BEVEL >= 1 */
	}

	/* draw the thumb */
	bevelrect(xw, xw->textw, (int)(xw->cellw * xw->columns + SB_GAP), newtop,
		(unsigned)(o_scrollbarwidth - 2 * SB_GAP), (unsigned)(newbottom - newtop), False) ;

	/* store info about the scrollbar */
	xw->sbheight = newheight;
	xw->sbtop = newtop;
	xw->sbbottom = newbottom;
	xw->sbstate = SB_NORMAL;
	xw->fg = xw->fgnormal;
}


/* change scrollbar state.  This is used to blank out the scrollbar or make
 * it show a stop-sign.
 */
static void setsbstate(xw, newstate)
	X11WIN	*xw;		/* the window whose state should be changed */
	SBSTATE	newstate;	/* the state is should be changed to */
{
	XPoint	vertex[9];	/* corners of a stop sign */
	int	i;

	/* if no change needed, then do nothing */
	if (xw->sbstate == newstate)
		return;

	/* else switch to the requested state */
	if (newstate == SB_NORMAL)
	{
		scrollbar((GUIWIN *)xw, 0, 0, 0);
	}
	else
	{
		/* blank the scrollbar area */
		XSetForeground(display, xw->gc, xw->bgscroll);
		xw->fg = xw->bgscroll;
		XFillRectangle(display, xw->textw, xw->gc,
			(int)(xw->cellw * xw->columns + 1), 1,
			(unsigned)o_scrollbarwidth - 2, (unsigned)(xw->cellh * xw->rows) - 2);

		/* if supposed to draw a little Stop sign, then do that. */
		if (newstate == SB_STOP)
		{
			/* compute vertices of stop sign */
			vertex[0].x = -(o_scrollbarwidth - SB_GAP) * 5/24;
			vertex[0].y = (o_scrollbarwidth - SB_GAP) / 2 - 1;
			vertex[1].x = -vertex[0].y;
			vertex[1].y = -vertex[0].x;
			vertex[2].x = -vertex[0].y;
			vertex[2].y = vertex[0].x;
			vertex[3].x = vertex[0].x;
			vertex[3].y = -vertex[0].y;
			for (i = 4; i < 9; i++)
			{
				vertex[i].x = -vertex[i - 4].x;
				vertex[i].y = -vertex[i - 4].y;
			}
			for (i = 0; i < 9; i++)
			{
				vertex[i].x += o_scrollbarwidth / 2 + xw->cellw * xw->columns;
				vertex[i].y += (xw->cellh * xw->rows) / 2;
			}

			/* draw the stop sign */
			XSetForeground(display, xw->gc, xw->fgcursor);
			XFillPolygon(display, xw->textw, xw->gc, vertex, 8,
				Convex, CoordModeOrigin);
			XSetForeground(display, xw->gc, xw->bgnormal);
			XDrawLines(display, xw->textw, xw->gc, vertex, 5, CoordModeOrigin);
			XSetForeground(display, xw->gc, xw->fgnormal);
			XDrawLines(display, xw->textw, xw->gc, vertex + 4, 5, CoordModeOrigin);
			xw->fg = xw->fgnormal;

			/* We only draw the stop sign when we expect elvis to
			 * be busy for a long time, not receiving new events.
			 * We should flush the X queue so the stop sign will
			 * be drawn immediately.
			 */
			XFlush(display);
		}
	}

	/* we've changed the state */
	xw->sbstate = newstate;
}


/* draw a single toolbar button */
static void draw1tool(xw, tool, newstate)
	X11WIN	*xw;	/* window where tool will be drawn */
	TOOL	*tool;	/* the tool to draw */
	_char_	newstate;/* appearance the button should have */
{
	int	h;

	/* if new state is same as old state, do nothing */
	if (newstate == xw->toolstate[tool->id])
		return;

	/* compute the button height */
	h = loadedcontrol->height + 2 * SB_GAP;

	/* draw the button's background and bevel */
	if (newstate == 'f')
	{
		/* draw the button in the window's background color */
		XClearArea(display, xw->toolw, tool->x, 1,
			(unsigned)tool->width, (unsigned)h,
			False);
		XSetBackground(display, xw->gc, xw->bgscroll);
		xw->bg = xw->bgscroll;
	}
	else
	{
		/* draw the button's face in the scrollbar foreground
		 * color, with bevelled edges.
		 */
		bevelrect(xw, xw->toolw, tool->x, 1, 
			(unsigned)tool->width, (unsigned)h,
			(BOOLEAN)(newstate == 'i'));
	}

	/* draw the button's label */
	XSetForeground(display, xw->gc, xw->fgnormal);
	XSetFont(display, xw->gc, loadedcontrol->fontinfo->fid);
	XDrawString(display, xw->toolw, xw->gc,
		tool->x + tool->textx, tool->texty,
		tool->label, strlen(tool->label));
	xw->fg = xw->fgnormal;

	/* remember this button's drawn state */
	xw->toolstate[tool->id] = newstate;
}

/* draw a window's toolbar */
static void drawtoolbar(xw, fromscratch)
	X11WIN	*xw;		/* window whose toolbar should be drawn */
	BOOLEAN	fromscratch;	/* redraw background too? */
{
	TOOL	*tool, *lag;
	char	newstate;
	CHAR	*str;
	int	dummy;
	XCharStruct size;

	/* if no toolbar is visible, then don't bother */
	if (!o_toolbar)
	{
		return;
	}

	/* supposed to draw from scratch? */
	if (fromscratch)
	{
		/* draw the background */
		XClearWindow(display, xw->toolw);

		/* reset the states for all buttons */
		memset(xw->toolstate, 0, sizeof xw->toolstate);
	}

	/* make sure this window is the "current" window.  Otherwise the
	 * calculate() calls below will be looking at the wrong window's
	 * options.
	 */
	(void)eventfocus((GUIWIN *)xw);

	/* for each button... */
	for (lag = NULL, tool = toollist; tool; lag = tool, tool = tool->next)
	{
		/* if new button, then assign it a position & size */
		if (tool->width == 0)
		{
			XTextExtents(loadedcontrol->fontinfo,
				tool->label, strlen(tool->label),
				&dummy, &dummy, &dummy, &size);
			tool->width = size.rbearing - size.lbearing + 2 * SB_BEVEL + 2;
			tool->textx = -size.lbearing + SB_BEVEL + 1;
			tool->texty = loadedcontrol->fontinfo->ascent + SB_BEVEL + 1;
			if (lag)
				tool->x += lag->x + lag->width + SB_GAP;
			else
				tool->x = SB_GAP / 2;
		}

		/* compute the button's new state */
		newstate = '\0';
		if (xw->sbstate != SB_NORMAL
		 || tool->excmd == NULL)
		{
			newstate = 'f'; /* flat, inactive */
		}
		else if (tool->when)
		{
			str = calculate(tool->when, NULL, False);
			if (!str)
			{
				/* error - disable the button semi-permanently */
				safefree(tool->when);
				str = toCHAR("0");
				tool->when = CHARdup(str);
			}
			if (!calctrue(str))
				newstate = 'f'; /* flat, inactive */
		}
		if (newstate != 'f' && !tool->in)
		{
			newstate = 'o'; /* out, active */
		}
		else if (newstate != 'f')
		{
			str = calculate(tool->in, NULL, False);
			if (!str)
			{
				/* error - forget condition, always display out */
				safefree(tool->in);
				tool->in = NULL;
				newstate = 'o'; /* out, active */
			}
			else if (calctrue(str))
				newstate = 'i'; /* in, inactive */
			else
				newstate = 'o'; /* out, active */
		}

		/* update the button's appearance */
		draw1tool(xw, tool, newstate);
	}
}


/* maintain the list of toolbar buttons */
static BOOLEAN guicmd(gw, extra)
	GUIWIN	*gw;		/* window where command was typed (ignored) */
	char	*extra;		/* label, operator, and command/condition */
{
	TOOL	*scan, *lag;
	char	*label, *end;
	char	operator;
	int	i;
	X11WIN	*xw;
	static	gap;

	/* Parse the label and operator.  Leave extra pointing to command/condition */
	for (label = extra; *extra && (*extra == *label || isalnum(*extra)); extra++)
	{
	}
	end = extra;
	while (isspace(*extra))
	{
		extra++;
	}
	if (*extra != ':' && *extra != '=' && *extra != '?')
	{
		/* maybe a special word? */
		if (!strcmp(label, "gap"))
		{
			gap += 6;
			return True;
		}
		else if (!strcmp(label, "newtoolbar"))
		{
			/* delete all tools */
			while (toollist)
			{
				lag = toollist->next;
				safefree(toollist->label);
				if (toollist->excmd) safefree(toollist->excmd);
				if (toollist->when) safefree(toollist->when);
				if (toollist->in) safefree(toollist->in);
				safefree(toollist);
				toollist = lag;
			}

			/* redraw toolbar from scratch, on all windows */
			for (xw = winlist; xw; xw = xw->next)
			{
				drawtoolbar(xw, True);
			}
			return True;
		}
		return False;
	}
	operator = *extra++;
	*end = '\0';
	while (isspace(*extra))
	{
		extra++;
	}

	/* search for the button.  If not found, then create it. */
	for (lag = NULL, scan = toollist, i = 0;
	     scan && strcmp(scan->label, label);
	     lag = scan, scan = scan->next, i++)
	{
	}
	if (!scan)
	{
		scan = (TOOL *)safealloc(1, sizeof(TOOL));
		scan->label = safedup(label);
		scan->safer = o_safer;
		scan->id = i;
		scan->x = gap;
		gap = 0;
		if (lag)
			lag->next = scan;
		else
			toollist = scan;
	}

	/* process the operator */
	switch (operator)
	{
	  case ':':
		if (scan->excmd) safefree(scan->excmd);
		scan->excmd = (*extra ? safedup(extra) : NULL);
		break;

	  case '?':
		if (scan->when) safefree(scan->when);
		scan->when = (*extra ? CHARdup(toCHAR(extra)) : NULL);
		break;

	  case '=':
		if (scan->in) safefree(scan->in);
		scan->in = (*extra ? CHARdup(toCHAR(extra)) : NULL);
		break;
	}

	return True;
}


/* Translate keylabels into raw codes, or vice versa.  Returns length of raw
 * codes if successful, or 0 if unrecognized text.
 */
static int keylabel(given, givenlen, label, rawin)
	CHAR *given;	/* the string typed in by user */
	int givenlen;	/* length of the user's string */
	CHAR **label;	/* pointer to (CHAR *) to set to key label */
	CHAR **rawin;	/* pointer to (CHAR *) to set to raw codes */
{
	static CHAR rawbuf[10];	/* buffer, holds raw byte string */
	CHAR	    lblbuf[20];	/* buffer, holds label string */
	char	    *name;
	KeySym      key;
	int	    i;

	/* no single-character string can be a key label */
	if (givenlen < 2)
		return 0;

	/* could this be the raw codes of a key? */
	if (givenlen == 5 && *given == ELVCTRL('K'))
	{
		/* convert key value into a KeySym */
		for (i = 1, key = 0; i <= 4; i++)
		{
			key <<= 4;
			if (isdigit(given[i]))
			{
				key += given[i] - '0';
			}
			else if (isxdigit(given[i]))
			{
				key += (given[i] & 0xf) + 9;
			}
			else
			{
				return 0;
			}
		}

		/* See if the KeySym has a name */
		name = XKeysymToString(key);
		if (!name)
		{
			return 0;
		}

		goto Found;
	}

	/* Maybe it is a label in foo or <foo> format? */
	if (given[0] == '<' && given[givenlen - 1] == '>' && givenlen < QTY(lblbuf)-1)
	{
		/* Convert <foo> name to foo */
		CHARncpy(lblbuf, given, (size_t)givenlen);
		givenlen -= 2;
	}
	else if (given[0] == '#' && givenlen < QTY(lblbuf) - 2)
	{
		/* standardize the format of the #nn string */
		lblbuf[1] = 'F';
		CHARncpy(lblbuf + 2, given + 1, (size_t)givenlen);
	}
	else if (givenlen < QTY(lblbuf)-3)
	{
		/* standardize the format of the foo string */
		CHARncpy(lblbuf + 1, given, (size_t)givenlen);
	}
	else
	{
		/* too long to be a key label */
		return 0;
	}

	/* convert label to KeySym */
	lblbuf[givenlen + 1] = '\0';
	name = tochar8(lblbuf + 1);
	key = XStringToKeysym(name);
	if (key == NoSymbol)
	{
		return 0;
	}

Found:	/* We have a key!  At this point, "key" and "name" are the only
	 * variables we can trust.
	 */

	/* if function key, then convert label to #n format (else <foo>) */
	if (key >= XK_F1 && key <= XK_F10)
	{
		sprintf((char *)lblbuf, "#%ld", (long)(key - XK_F1 + 1));
	}
	else
	{
		lblbuf[0] = '<';
		CHARcpy(&lblbuf[1], name);
		CHARcat(lblbuf, toCHAR(">"));
	}

	/* convert the KeySym into raw code, and return it. */
	sprintf((char *)rawbuf, "%c%04lx", ELVCTRL('K'), (long)key);
	*label = CHARdup(lblbuf);
	*rawin = rawbuf;
	return CHARlen(rawbuf);
}


static BOOLEAN color(gw, font, fg, bg)
	GUIWIN	*gw;	/* window whose colors should be changed */
	_char_	font;	/* font letter of font to change */
	CHAR	*fg;	/* name of new foreground color */
	CHAR	*bg;	/* background color for whole window */
{
	X11WIN	*xw = (X11WIN *)gw;
	unsigned long *fgp, *bgp;	/* pointers to colors to change */
	CHAR	*fname, *bname;		/* pointers to color names */

	/* decide which colors we should change */
	bgp = &xw->bgnormal;
	bname = background;
	switch (font)
	{
	  case 'b': fgp = &xw->fgbold, fname = boldcolor;	break;

	  case 'e': fgp = &xw->fgemph, fname = emphcolor;	break;

	  case 'u': fgp = &xw->fgundln, fname = underlinecolor;	break;

	  case 'i': fgp = &xw->fgitalic, fname = italiccolor;	break;

	  case 'c': fgp = &xw->fgcursor, fname = cursorcolor;
		    bgp = &xw->owncursor, bname = owncolor;	break;

	  case 's': fgp = &xw->fgscroll, fname = scrollbarfg;
		    bgp = &xw->bgscroll, bname = scrollbarbg;	break;

	  case 'f': fgp = &xw->fgfixed, fname = fixedcolor;	break;

	  default:  fgp = &xw->fgnormal, fname = foreground;	break;
	}

	/* Change the foreground colors */
	if (fg)
	{
		if (xw)
		{
			unloadcolor(*fgp);
			*fgp = loadcolor(fg, (font != 's') ? black : white);
		}
		CHARcpy(fname, fg);
	}

	/* Change the background colors */
	if (bg)
	{
		if (xw)
		{
			unloadcolor(*bgp);
			if (font == 's')
			{
				/* scrollbar and toolbar */
				*bgp = loadcolor(bg, black);
				XSetWindowBackground(display, xw->toolw, xw->bgscroll);
			}
			else
			{
				/* text */
				*bgp = loadcolor(bg, white);
				XSetWindowBackground(display, xw->textw, xw->bgnormal);
			}
		}
		CHARcpy(bname, bg);
	}

	/* We'll probably need to redraw the scrollbar.  Since any :color
	 * command causes the screen to be redrawn, all we need to do is
	 * bypass the scrollbar's optimization.
	 *
	 * Additionally, if the toolbar is shown then it should be redrawn
	 */
	if (xw)
	{
		xw->sbheight = 0;
		drawtoolbar(xw, True);
	}

	return True;
}

/*----------------------------------------------------------------------------*/

/* open an X cut buffer for reading or writing.  Returns True if successful */
static BOOLEAN	clipopen(forwrite)
	BOOLEAN	forwrite;	/* True for writing, False for reading */
{
	XEvent	event;
	Atom	gottype;
	long	extra;
	int	i;

	/* free the old clipbuf, if there was one (except when reading from
	 * own own selection).
	 */
	if (clipbuf && (forwrite || !ownselection))
	{
		safefree(clipbuf);
		clipbuf = NULL;
	}

	if (forwrite)
	{
		/* prepare to collect bytes as clipwrite() gets called */
		clipwriting = True;
		clipsize = 0;
	}
	else
	{
		/* does elvis own the selection? */
		if (ownselection)
		{
			/* yes -- clipbuf already contains it */
		}
		else
		{
			/* no -- try to fetch text from X two ways... */

			/* is there a selection owner? */
			if (XGetSelectionOwner(display, XA_PRIMARY) == None)
			{
				/* don't bother to try getting selection */
				event.type = ButtonPress;
			}
			else
			{
				/* Try to fetch the selection */
				XConvertSelection(display,
					XA_PRIMARY, XA_STRING, elvis_cutbuffer,
					((X11WIN *)windefault->gw)->window, now);
				
				/* Wait for the selection to arrive, or for a
				 * button press.  Eat any other events.  (The
				 * button press event gives the user a way to
				 * abort the operation.)
				 */
				do
				{
					XNextEvent(display, &event);
				} while (event.type != SelectionNotify
					&& event.type != ButtonPress);
			}

			/* did we succeed in fetching the selection? */
			if (event.type == SelectionNotify
			 && event.xselection.property != None)
			{
				/* Yes -- fetch bytes from X property */
				XGetWindowProperty(display,
					event.xselection.requestor,
					event.xselection.property,
					0L, 65536L, True,
					event.xselection.target, &gottype, &i,
					(unsigned long *)&clipsize,
					(unsigned long *)&extra,
					(unsigned char **)&clipbuf);
			}
			else
			{
				/* No -- fetch bytes from X cut buffer */
				clipbuf = XFetchBytes(display, &i);
				clipsize = i;
				if (!clipbuf)
				{
					return False;
				}
			}
		}

		/* prepare to receive bytes as clipread() gets called */
		clipwriting = False;
		clipused = 0;
	}
	return True;
}

/* add text to the buffer */
static int clipwrite(text, len)
	CHAR	*text;	/* pointer to buffer containing some bytes */
	int	len;	/* number of bytes to add */
{
	char	*newp;

	assert(clipwriting);

	/* combine old text (if any) with new text in a malloc'ed buffer */
	newp = safealloc(len + clipsize, sizeof(char));
	if (clipsize > 0)
	{
		memcpy(newp, clipbuf, (size_t)clipsize);
		safefree(clipbuf);
	}
	clipbuf = newp;
	memcpy(clipbuf + clipsize, text, (size_t)len);
	clipsize += len;

	return len;
}

/* extract text from the buffer */
static int clipread(text, len)
	CHAR	*text;	/* pointer to buffer where bytes should go */
	int	len;	/* number of bytes to read this time */
{
	assert(!clipwriting);

	if (!clipbuf || clipused >= clipsize)
	{
		/* everything already sent; return 0 */
		return 0;
	}
	else if (clipused + len >= clipsize)
	{
		/* if everything fits, return everything */
		len = clipsize - clipused;
	}
	memcpy(text, clipbuf, (size_t)len);
	clipused += len;
	return len;
}

/* end the cut/paste operation */
static void clipclose()
{
	if (clipwriting)
	{
		/* send bytes to the X server */
		XStoreBytes(display, clipbuf ? clipbuf : "", clipsize);

		/* claim ownership of the selection */
		XSetSelectionOwner(display, XA_PRIMARY,
			((X11WIN *)windefault->gw)->window, now);

		/* was the claim successful? */
		if (XGetSelectionOwner(display, XA_PRIMARY) ==
			((X11WIN *)windefault->gw)->window)
		{
			ownselection = True;
			drawcursor(NULL);
		}
		else
		{
			/* elvis doesn't own the selection */
			ownselection = False;
			drawcursor(NULL);
			safefree(clipbuf);
			clipbuf = NULL;
		}
	}
	else
	{
		if (!ownselection)
		{
			/* free Xlib's copy of the cut buffer */
			XFree(clipbuf);
			clipbuf = NULL;
		}
	}
}

/*----------------------------------------------------------------------------*/

/* This function starts an interactive shell.  It is called with the argument
 * (True) for the :sh command, or (False) for a :stop or :suspend command.
 * If successful it returns RESULT_COMPLETE after the shell exits; if
 * unsuccessful it issues an error message and returns RESULT_ERROR.  It
 * could also return RESULT_MORE to defer processing to the portable code
 * in ex_suspend().
 */
static RESULT stop(alwaysfork)
	BOOLEAN	alwaysfork;	/* ignored; X11 always forks anyway */
{
	/* save the buffers, if we're supposed to */
	eventsuspend();

	/* start an xterm with a shell in it */
	system(o_stopshell ? tochar8(o_stopshell) : "xterm &");
	return RESULT_COMPLETE;
}


/*----------------------------------------------------------------------------*/

/* NOTE: The X11 headers #define True and False for their own purposes, but now
 * we need use elvis' enum versions of them.  This probably isn't important,
 * really, since both sets of symbols use the same value, but some compilers
 * will complain if "False" becomes "0" in the initializer of the guix11 struct
 * and we want to keep compilers happy.
 */
#ifdef True
# undef True
# undef False
#endif

GUI guix11 =
{
	"x11",	/* name */
	"Simple X11 graphic interface",
	False,	/* exonly */
	False,	/* newblank */
	True,	/* minimizeclr */
	False,	/* scrolllast */
	True,	/* shiftrows */
	0,	/* movecost */
	0,	/* nopts */
	NULL,	/* optdescs */
	test,
	init,
	usage,
	loop,
	wpoll,
	term,
	creategw,
	destroygw,
	focusgw,
	retitle,
	NULL,	/* reset */
	flush,
	moveto,
	draw,
	shift,
	scroll,
	clrtoeol,
	NULL,	/* newline */
	beep,
	NULL,	/* msg */
	scrollbar,
	NULL,	/* status */
	keylabel,
	clipopen,
	clipwrite,
	clipread,
	clipclose,
	color,	/* color */
	guicmd,	/* guicmd */
	NULL,	/* tabcmd */
	NULL,	/* save */
	NULL,	/* wildcard */
	NULL,	/* prgopen */
	NULL,	/* prgclose */
	stop
};
#endif
