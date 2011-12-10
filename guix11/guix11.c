/* guix11.c */

/* Copyright 1995 by Steve Kirkendall */

char id_guix11[] = "$Id: guix11.c,v 2.37 1999/03/11 17:54:44 steve Exp $";

#include "elvis.h"
#ifdef GUI_X11
# include "guix11.h"
# include "gray.xbm"
# include "elvis.xbm"
# include "elvispin.xbm"

#define APPDEFAULTS	"/usr/lib/X11/app-defaults"
#define HOMEDEFAULTS	".Xdefaults"

/* default values of options */
#define DEFAULT_ICON		True
#define DEFAULT_STOPSHELL	"xterm &"
#define DEFAULT_OUTLINEMONO	2
#define DEFAULT_BORDERWIDTH	1
#define DEFAULT_SCROLLBARLEFT	False
#define DEFAULT_TOOLBAR		resources[0].boolean
#define DEFAULT_STATUSBAR	resources[1].boolean
#define DEFAULT_NORMALFONT	resources[2].string
#define DEFAULT_GEOMETRY	resources[3].string
#define DEFAULT_FOREGROUND	resources[4].string
#define DEFAULT_BACKGROUND	resources[5].string
#define DEFAULT_DBLCLICKTIME	resources[6].number
#define DEFAULT_CONTROLFONT	resources[7].string
#define DEFAULT_CURSORCOLOR	resources[8].string
#define DEFAULT_OWNCOLOR	resources[9].string
#define DEFAULT_BLINKTIME	resources[10].number
#define DEFAULT_TOOLFG		resources[11].string
#define DEFAULT_TOOLBG		resources[12].string
#define DEFAULT_SCROLLBARFG	resources[13].string
#define DEFAULT_SCROLLBARBG	resources[14].string
#define DEFAULT_SCROLLBARWIDTH	resources[15].number
#define DEFAULT_SCROLLBARTIME	resources[16].number
#define DEFAULT_SCROLLBAR	resources[17].boolean
static struct
{
	char	*class;
	char	*name;
	char	*string;
	long	number;
	BOOLEAN	boolean;
} resources[] =
{
    {"Elvis.Toolbar",		     "elvis.toolbar",		    "true"},
    {"Elvis.Statusbar",		     "elvis.Statusbar",		    "true"},
    {"Elvis.Font",	 	     "elvis.font",		    "fixed"},
    {"Elvis.Geometry",		     "elvis.geometry",		    "80x34"},
    {"Elvis.Foreground",	     "elvis.foreground",	    "black"},
    {"Elvis.Background",	     "elvis.background",	    "gray90"},
    {"Elvis.DblClickTime",	     "elvis.dblclicktime",	    "3", 3},
    {"Elvis.Control.Font",	     "elvis.control.font",	    "variable"},
    {"Elvis.Cursor.Foreground",	     "elvis.cursor.foreground",	    "red"},
    {"Elvis.Cursor.Selected",	     "elvis.cursor.selected",	    "red"},
    {"Elvis.Cursor.BlinkTime",	     "elvis.cursor.blinktime",	    "3"},
    {"Elvis.Tool.Foreground",	     "elvis.tool.foreground",	    "black"},
    {"Elvis.Tool.Background",	     "elvis.tool.background",	    "gray75"},
    {"Elvis.Scrollbar.Foreground",   "elvis.scrollbar.foreground",  "gray75"},
    {"Elvis.Scrollbar.Background",   "elvis.scrollbar.background",  "gray60"},
    {"Elvis.Scrollbar.Width",	     "elvis.scrollbar.width",	    "11"},
    {"Elvis.Scrollbar.Repeat",	     "elvis.scrollbar.repeat",	    "4"},
    {"Elvis.Scrollbar.Enabled",      "elvis.scrollbar.enabled",     "True"},
};


#if USE_PROTOTYPES
static void beep(GUIWIN *gw);
static int catchErrors(Display *disp, XErrorEvent *err);
static void moveto(GUIWIN *gw, int column, int row);
static BOOLEAN scroll(GUIWIN *gw, int qty, BOOLEAN notlast);
static BOOLEAN shift(GUIWIN *gw, int qty, int rows);
static BOOLEAN clientaction(int argc, char **argv);
static BOOLEAN clrtoeol(GUIWIN *gw);
static BOOLEAN color(GUIWIN *gw, _char_ font, CHAR *fg, CHAR *bg);
static BOOLEAN creategw(char *name,char * firstcmd);
static void destroygw(GUIWIN *gw, BOOLEAN force);
static void draw(GUIWIN *gw, _char_ font, CHAR *text, int len);
static int init(int argc, char **argv);
static int keylabel(CHAR *given, int givenlen, CHAR **label, CHAR **rawin);
static void loadresources(void);
static void loop(void);
static void retitle(GUIWIN *gw, char *name);
static void scrollbar(GUIWIN *gw, long top, long bottom, long total);
static BOOLEAN status(GUIWIN *gw, CHAR *cmd, long line, long column, _CHAR_ learn, char *mode);
static RESULT stop(BOOLEAN alwaysfork);
static void term(void);
static int test(void);
static void usage(void);
static BOOLEAN wpoll(BOOLEAN reset);
#endif
static BOOLEAN focusgw P_((GUIWIN *gw));
static int ignoreErrors P_((Display *disp, XErrorEvent *err));
static void flush P_((void));
static BOOLEAN guicmd P_((GUIWIN *gw, char *extra));
static int xoptisfont P_((OPTDESC *desc, OPTVAL *val, CHAR *newval));
static int xoptisnumber P_((OPTDESC *desc, OPTVAL *val, CHAR *newval));
#ifndef NO_XLOCALE
static XIC createic P_((Window window));
#endif

Display		*x_display;		/* X11 display */
int		x_screen;		/* screen number */
X11WIN		*x_winlist;		/* list of windows */
int		x_depth;		/* bits per pixel */
Colormap	x_colormap;		/* colormap shared by elvis windows */
unsigned long	x_white, x_black;	/* default color values */
BOOLEAN		x_mono;			/* is this a monochrome display? */
Time		x_now;			/* timestamp of current event */
Atom		x_elvis_cutbuffer;	/* value for ELVIS_CUTBUFFER atom */
Atom		x_wm_protocols;		/* value for WM_PROTOCOLS atom */
Atom		x_wm_delete_window;	/* value for WM_DELETE_WINDOW atom */
Atom		x_elvis_server;		/* value for ELVIS_SERVER atom */
Atom		x_resource_manager;	/* value for MANAGER_RESOURCES atom */
X_LOADEDFONT	*x_defaultnormal;	/* normal font */
X_LOADEDFONT	*x_defaultbold;		/* bold font, or NULL to fake it */
X_LOADEDFONT	*x_defaultitalic;	/* italic font, or NULL to fake it */
X_LOADEDFONT	*x_loadedcontrol;	/* toolbar font */
X11WIN		*x_hasfocus;		/* window with keyboard focus, or NULL */
Pixmap		x_gray;			/* gray pixmap, for mono screens */
Pixmap		x_elvis_icon;		/* elvis' window icon */
Pixmap		x_elvis_pin_icon;	/* elvis' window icon, with pushpin */
unsigned	x_elvis_icon_width;	/* width of x_elvis_icon */
unsigned	x_elvis_icon_height;	/* height of x_elvis_icon */

static jmp_buf	xerror_handler;		/* to recover from protocol errors */
static Window	root;			/* root window */
static int	rootheight, rootwidth;	/* size of root window */
static XrmDatabase database;		/* resources for this screen */
static char	*argv0;			/* name of program */
static Window	fromwin;		/* window which invoked elvis */


#define WIN2XW(win,xw)	for ((xw) = x_winlist;\
			     (xw) && (xw)->win != (win) && (xw)->textw != (win) && (xw)->toolw != (win);\
			     (xw) = (xw)->next)\
			{\
			}\
			if (!(xw)) break;

/* This table lists the keys which are mapped automatically */
static struct
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
	{"normalfont", "xfn",	optsstring,	xoptisfont	},
	{"boldfont", "xfb",	optsstring,	xoptisfont	},
	{"italicfont", "xfi",	optsstring,	xoptisfont	},
	{"controlfont", "xfc",	optsstring,	xoptisfont	},
	{"toolbar", "xtb",	NULL,		NULL		},
	{"scrollbarwidth","xsw",optnstring,	xoptisnumber,	"5:40"},
	{"scrollbartime", "xst",optnstring,	optisnumber,	"0:20"},
	{"scrollbarleft", "xsl",NULL,		NULL,		},
	{"scrollbar", "sb",	NULL,		NULL,		},
	{"statusbar", "xstat",	NULL,		NULL		},
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
	{"warpback", "xwb",	NULL,		NULL		},
	{"warpto", "wt",	opt1string,	optisoneof,	"don't scrollbar origin corners"},
	{"focusnew", "fn",	NULL,		NULL		},
	{"textcursor", "tc",	opt1string,	optisoneof,	"hollow opaque xor"},
	{"underline", "uln",	NULL,		NULL		},
	{"outlinemono", "om",	optnstring,	optisnumber,	"0:3"},
	{"borderwidth", "xbw",	optnstring,	xoptisnumber,	"0:5"},
	{"submit", "Submit",	optsstring,	optisstring	},
	{"cancel", "Cancel",	optsstring,	optisstring	},
	{"help", "Help",	optsstring,	optisstring	}
};

struct x_optvals_s x_optvals;

/* The following store names of colors */
CHAR x_background[50];	
CHAR x_foreground[50];
CHAR x_boldcolor[50];
CHAR x_emphcolor[50];
CHAR x_fixedcolor[50];
CHAR x_italiccolor[50];
CHAR x_underlinecolor[50];
CHAR x_cursorcolor[50];
CHAR x_owncolor[50];
CHAR x_toolfg[50];
CHAR x_toolbg[50];
CHAR x_scrollbarfg[50];
CHAR x_scrollbarbg[50];


/* This flag is set whenever an option is changed which would require all
 * windows to be reconfigured.  Exception: Boolean options (currently just
 * "toolbar") don't set this flag.
 */
static BOOLEAN allreconfig;


/* load the default values from the resource database */
static void loadresources P_((void))
{
	int	i;
	Atom	gottype;
	long	length, dummy;
	char	*type;
	XrmDatabase db;
	XrmValue value;

	if (o_verbose >= 2)
		fprintf(stderr, "Loading resources...\n");

	/* fetch the resource database */
	XrmInitialize();
#if 0
	database = XrmGetDatabase(x_display);
#else
	/* First read the global app-defaults file */
	type = getenv("XFILESEARCHPATH");
	type = iopath(type ? type : APPDEFAULTS, "Elvis", False);
	if (type)
		database = XrmGetFileDatabase(type);

	/* Then try the user app-defaults file */
	type = getenv("XUSERFILESEARCHPATH");
	if (type)
		type = iopath(type, "Elvis", False);
	if (type)
	{
		if (database)
			(void)XrmCombineFileDatabase(type, &database, True);
		else
			database = XrmGetFileDatabase(type);
	}

	/* then read the server or ~/.Xdefaults file */
	type = NULL;
	if (x_resource_manager != None)
	{
		XGetWindowProperty(x_display, root, x_resource_manager,
			0L, 65536L, False,
			XA_STRING, &gottype, &i,
			(unsigned long *)&length,
			(unsigned long *)&dummy,
			(unsigned char **)&type);
	}
	if (type && i == 8 && gottype == XA_STRING)
	{
		db = XrmGetStringDatabase(type);
		if (database && db)
			XrmMergeDatabases(db, &database);
		else if (!database)
			database = db;
		XFree(type);
	}
	else
	{
		type = getenv("HOME");
		type = dirpath(type ? type : ".", ".Xdefaults");
		if (database)
			(void)XrmCombineFileDatabase(type, &database, True);
		else
			database = XrmGetFileDatabase(type);
	}

	/* NOTE: We should check the SCREEN_RESOURCES property here,
	 * but it probably isn't worth the trouble.
	 */

	/* if XENVIRONMENT is set, then merge its file's contents */
	type = getenv("XENVIRONMENT");
	if (type)
	{
		if (database)
			(void)XrmCombineFileDatabase(type, &database, True);
		else
			database = XrmGetFileDatabase(type);
	}
#endif
	if (o_verbose >= 2)
		fprintf(stderr, "guix11.c:database=0x%lx\n", (long)database);

	/* for each resource that we care about... */
	for (i = 0; i < QTY(resources); i++)
	{
		/* try to get a value */
		if (XrmGetResource(database, resources[i].name, resources[i].class, &type, &value))
		{
			/* try to store it, depending on its type */
			if (!strcmp(type, "String"))
			{
				if (o_verbose >= 2)
					fprintf(stderr, "\t%s:\t%s\n", resources[i].name, value.addr);
				resources[i].string = strdup(value.addr);
			}
			else if (o_verbose >= 2)
			{
				fprintf(stderr, "\tXrmGetResource(database, \"%s\", \"%s\", ...) return a %s -- ignored\n", resources[i].name, resources[i].class, type);
			}
		}

		/* convert value from string to number and boolean */
		resources[i].number = atol(resources[i].string);
		resources[i].boolean = calctrue(toCHAR(resources[i].string));
	}
}


/* This function returns -1 for invalid fonts, 0 if the new value is the same
 * as the old, or 1 if it is legal and different.  In the latter case it also
 * stores the new value, and sets a flag so that we'll know we have to
 * reconfigure the windows.  If the font we're changing is the normal font,
 * then elvis also clobbers the bold and italic fonts.
 */
static int xoptisfont(desc, val, newval)
	OPTDESC	*desc;	/* description of an option to be changed */
	OPTVAL	*val;	/* value of that option */
	CHAR	*newval;/* the new value, as a string */
{
	X_LOADEDFONT *font;

	/* if the old value and new value are the same, then return 0 */
	if (val->value.string && !CHARcmp(val->value.string, newval))
	{
		return 0;
	}

	/* make sure we can load that font.  Exception: before the first window
	 * is created, we allow the font options to be set to anything; we'll
	 * check them when we try to create the first window.
	 */
	if (x_winlist)
	{
		font = x_loadfont(tochar8(newval));
		if (!font)
		{
			/* error message already given, by x_loadfont() */
			return -1;
		}
		x_unloadfont(font);
	}

	/* store the new value */
	(void)optisstring(desc, val, newval);

	/* if we changed normalfont, then clobber boldfont and italicfont */
	if (val->value.string == o_normalfont)
	{
		/* clobber boldfont */
		if (optflags(o_boldfont) & OPT_FREE)
			safefree(o_boldfont);
		o_boldfont = NULL;
		optflags(o_boldfont) &= ~(OPT_FREE|OPT_SET);

		/* clobber italicfont */
		if (optflags(o_italicfont) & OPT_FREE)
			safefree(o_italicfont);
		o_italicfont = NULL;
		optflags(o_normalfont) &= ~(OPT_FREE|OPT_SET);
	}

	/* set a flag so we know we must reconfigure */
	allreconfig = True;
	return 1;
}


/* This function returns -1 for invalid toolbar positions, 0 if the new value
 * is the same as the old, or 1 if it is legal and different.  In the latter
 * case it also stores the new value, and sets a flag so that we'll know we
 * have to reconfigure the windows.
 */
static int xoptisnumber(desc, val, newval)
	OPTDESC	*desc;	/* description of an option to be changed */
	OPTVAL	*val;	/* value of that option */
	CHAR	*newval;/* the new value, as a string */
{
	long	oldvalue;

	/* remember the old value */
	oldvalue = val->value.number;

	/* try to change the value.  If illegal, then fail */
	if (optisnumber(desc, val, newval) < 0)
		return -1;

	/* if value changed, then set the reconfiguring flag */
	if (oldvalue == val->value.number)
		return 0;
	allreconfig = True;
	return 1;
}


/* adjust a window in response to resizes, font changes, etc. */
void x_reconfig(xw, columns, rows)
	X11WIN		*xw;	/* window to be reconfigured */
	unsigned int	columns;/* new width, in characters */
	unsigned int	rows;	/* new height, in lines */
{
	XSizeHints hint;
	BOOLEAN	   resize;
	X_LOADEDFONT *font;

	/* Check the fonts; they may have changed.  However, even if they
	 * haven't changed in this x_reconfig() call, we can't assume that
	 * the window has the correct fonts because x_reconfig() is called
	 * in a loop, and an earlier iteration could have changed the fonts.
	 */
	/* normal font */
	font = x_loadfont(o_normalfont ? tochar8(o_normalfont) : DEFAULT_NORMALFONT);
	assert(font);
	if (x_defaultnormal != font)
	{
		x_unloadfont(x_defaultnormal);
		x_defaultnormal = font;
	}
	/* bold font */
	if (!o_boldfont)
	{
		if (x_defaultbold)
		{
			x_unloadfont(x_defaultbold);
			x_defaultbold = NULL;
		}
	}
	else
	{
		font = x_loadfont(tochar8(o_boldfont));
		assert(font);
		if (x_defaultbold != font)
		{
			if (x_defaultbold) x_unloadfont(x_defaultbold);
			x_defaultbold = font;
		}
	}
	/* italic font */
	if (!o_italicfont)
	{
		if (x_defaultitalic)
		{
			x_unloadfont(x_defaultitalic);
			x_defaultitalic = NULL;
		}
	}
	else
	{
		font = x_loadfont(tochar8(o_italicfont));
		assert(font);
		if (x_defaultitalic != font)
		{
			if (x_defaultitalic) x_unloadfont(x_defaultitalic);
			x_defaultitalic = font;
		}
	}
	/* control font */
	font = x_loadfont(o_controlfont ? tochar8(o_controlfont) : DEFAULT_CONTROLFONT);
	assert(font);
	if (x_loadedcontrol != font)
	{
		x_unloadfont(x_loadedcontrol);
		x_loadedcontrol = font;
	}

	/* is the text area being resized? */
	resize = (BOOLEAN)(xw->ta.columns != columns || xw->ta.rows != rows);

	/* destroy old widgets */
	x_ta_destroy(xw);
	x_sb_destroy(xw);
	x_tb_destroy(xw);
	x_st_destroy(xw);

	/* compute the sizes of new widgets */
	x_ta_predict(xw, columns, rows);
	x_sb_predict(xw, (unsigned int)o_scrollbarwidth, xw->ta.h);
	x_tb_predict(xw, xw->ta.w + xw->sb.w, 0);
	x_st_predict(xw, xw->ta.w + xw->sb.w, 0);

	/* send some generic window size hints, so the old size hints don't
	 * interfere with our efforts to adjust the window.
	 */
	hint.x = hint.y = 0;
	hint.flags = 0;
	XSetWMNormalHints(x_display, xw->win, &hint);

	/* default window size */
	hint.x = hint.y = 0;
	hint.width = xw->ta.w + xw->sb.w;
	hint.height = xw->tb.h + xw->ta.h + xw->st.h;
	hint.width_inc = xw->ta.cellw;
	hint.height_inc = xw->ta.cellh;
	hint.base_width = hint.width - columns * hint.width_inc;
	hint.base_height = hint.height - rows * hint.height_inc;
	hint.min_width = hint.base_width + 30 * hint.width_inc;
	hint.min_height = hint.base_height + 2 * hint.height_inc;
	hint.max_width = hint.base_width + 200 * hint.width_inc;
	hint.max_height = hint.base_height + 200 * hint.height_inc;
	hint.flags = PSize | PBaseSize | PMinSize | PResizeInc;

	/* resize the window */
	if (hint.width != xw->w || hint.height != xw->h)
	{
		xw->w = hint.width;
		xw->h = hint.height;
		XResizeWindow(x_display, xw->win, xw->w, xw->h);
	}

	/* send the window's new size hints to the window manager */
	XSetWMNormalHints(x_display, xw->win, &hint);

	/* create new widgets */
	x_tb_create(xw, 0, 0);
	if (!o_scrollbarleft)
	{
		x_ta_create(xw, 0, xw->tb.h);
		x_sb_create(xw, xw->ta.w, xw->tb.h);
	}
	else
	{
		x_sb_create(xw, 0, xw->tb.h);
		x_ta_create(xw, xw->sb.w, xw->tb.h);
	}
	x_st_create(xw, 0, xw->tb.h + xw->ta.h);

	/* window mapping -- assume top-level window is already mapped */
	XMapSubwindows(x_display, xw->win);

	/* maybe simulate a "window resize" event */
	if (resize)
		eventresize((GUIWIN *)xw, (int)xw->ta.rows, (int)xw->ta.columns);

	/* if window is mapped, then redraw all widgets */
	if (xw->ismapped)
	{
		x_tb_draw(xw, True);
		xw->ta.nextcursor = eventdraw((GUIWIN *)xw);
		x_sb_setstate(xw, X_SB_REDRAW);
	}
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
	 * of a functioning X server, even though I'm personally not the user
	 * who'se using that X server.
	 *
	 * To prevent my elvis window from appearing on his X screen, I check
	 * a couple environment variables.  If WINDOWID is set then I assume
	 * x11 is available; else I'll check the TERM variable against a list
	 * of known X terminals, and reject x11 unless TERM is in that list.
	 */
	if (!getenv("WINDOWID"))
	{
		tmp = getenv("TERM");
		if (tmp && strcmp(tmp, "xterm") && strcmp(tmp, "dtterm")
		  && strcmp(tmp, "iris-ansi") && strcmp(tmp, "aixterm")
		  && strcmp(tmp, "xterm-color") && strcmp(tmp, "hpterm"))
			return 0;
	}

	/* Try to contact the server.  If we can contact it, great! */
	if (getenv("DISPLAY") && (x_display = XOpenDisplay("")) != (Display *)0)
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
	XGetWindowProperty(x_display, root, x_elvis_server, 0L, 1L, False,
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
		XCloseDisplay(x_display);
		x_display = XOpenDisplay("");
		x_screen = DefaultScreen(x_display);
		root = RootWindow(x_display, x_screen);
		loadresources();
		return False;
	}
	XSetErrorHandler(catchErrors);

	/* hang the property on the server elvis window */
	XChangeProperty(x_display, srvwin, x_elvis_server, XA_STRING, 8,
		PropModeAppend, (unsigned char *)prop, strlen(prop));
	XFlush(x_display);

	/* shut down the client connection to the X11 display, and cause
	 * an eventual exit.
	 */
	XCloseDisplay(x_display);
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
	if (!x_display)
		x_display = XOpenDisplay("");
	if (!x_display)
		msg(MSG_FATAL, "could not contact X server");
	x_screen = DefaultScreen(x_display);
	root = RootWindow(x_display, x_screen);

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
			CHARcpy(x_foreground, toCHAR(argv[i + 1]));
			ndel = 2;
		}
		else if (!strcmp(argv[i], "-bg"))
		{
			CHARcpy(x_background, toCHAR(argv[i + 1]));
			ndel = 2;
		}
		else if (!strcmp(argv[i], "-mono"))
		{
			x_mono = True;
			ndel = 1;
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
			XSynchronize(x_display, 1);
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

	/* Define some atoms.  Note that we incorporate the effective user ID
	 * and host name into the name of the ELVIS_SERVER atom, so that it
	 * won't interfere with elvis processes running on other hosts, or
	 * for other effective user IDs.
	 */
	x_wm_protocols = XInternAtom(x_display, "WM_PROTOCOLS", False);
	x_wm_delete_window = XInternAtom(x_display, "WM_DELETE_WINDOW", False);
	x_elvis_cutbuffer = XInternAtom(x_display, "ELVIS_CUTBUFFER", False);
	x_resource_manager = XInternAtom(x_display, "RESOURCE_MANAGER", True);
	sprintf(raw, "ELVIS_SERVER_%d@", geteuid());
	gethostname(raw + strlen(raw), sizeof raw - strlen(raw));
	x_elvis_server = XInternAtom(x_display, raw, False);

	/* load the defaults from the resource database */
	loadresources();

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
		XCloseDisplay(x_display);
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
			x_display = XOpenDisplay("");
			x_screen = DefaultScreen(x_display);
			root = RootWindow(x_display, x_screen);
			loadresources();

			/* and then continue execution... */
			break;

		  default: /* PARENT */
			return -1;
		}
	}

	/* more initialization */
	rootheight = DisplayHeight(x_display, x_screen);
	rootwidth = DisplayWidth(x_display, x_screen);
	x_depth = DefaultDepth(x_display, x_screen);
	if (x_depth == 1)
		x_mono = True;
	x_colormap = DefaultColormap(x_display, x_screen);
	x_gray = XCreatePixmapFromBitmapData(x_display,
		DefaultRootWindow(x_display),
		(char *)gray_bits, gray_width, gray_height,
		BlackPixel(x_display, x_screen), WhitePixel(x_display, x_screen), x_depth);
	x_elvis_icon = XCreateBitmapFromData(x_display,
		DefaultRootWindow(x_display),
		(char *)elvis_bits, elvis_width, elvis_height);
	x_elvis_pin_icon = XCreateBitmapFromData(x_display,
		DefaultRootWindow(x_display),
		(char *)elvispin_bits, elvispin_width, elvispin_height);
	x_elvis_icon_width = elvis_width;
	x_elvis_icon_height = elvis_height;
	XGetInputFocus(x_display, &fromwin, &i);
	if (fromwin == None || fromwin == PointerRoot)
	{
		fromwin = root;
	}

	/* ignore any protocol errors. We expect errors from XSetInputFocus()
	 * in some situations, and no errors from anything else.
	 */
	XSetErrorHandler(ignoreErrors);

	/* These options can't really be initialized until after the elvis.msg
	 * file is loaded, so we'll initialize them in the creategw() function,
	 * but until then they need to have *some* value.
	 */
	optpreset(o_submit, toCHAR("Submit"), OPT_HIDE);
	optpreset(o_cancel, toCHAR("Cancel"), OPT_HIDE);
	optpreset(o_help, toCHAR("Help"), OPT_HIDE);

	/* initialize the options */
	optflags(o_normalfont) |= OPT_HIDE;
	optflags(o_boldfont) |= OPT_HIDE;
	optflags(o_italicfont) |= OPT_HIDE;
	optflags(o_controlfont) |= OPT_HIDE;
	optpreset(o_toolbar, DEFAULT_TOOLBAR, OPT_HIDE);
	optpreset(o_scrollbarwidth, DEFAULT_SCROLLBARWIDTH, OPT_HIDE);
	optpreset(o_scrollbartime, DEFAULT_SCROLLBARTIME, OPT_HIDE);
	optpreset(o_scrollbarleft, DEFAULT_SCROLLBARLEFT, OPT_HIDE);
	optpreset(o_scrollbar, DEFAULT_SCROLLBAR, OPT_HIDE);
	optpreset(o_statusbar, DEFAULT_STATUSBAR, OPT_HIDE);
	optpreset(o_dblclicktime, DEFAULT_DBLCLICKTIME, OPT_HIDE);
	optpreset(o_blinktime, DEFAULT_BLINKTIME, OPT_HIDE);
	optflags(o_firstx) |= OPT_HIDE|OPT_NODFLT;
	optflags(o_firsty) |= OPT_HIDE|OPT_NODFLT;
	switch (sscanf(DEFAULT_GEOMETRY, "%dx%d+%d+%d", &w, &h, &i, &j))
	{
	  case 4:
		o_firstx = i;
		o_firsty = h;
		/* fall through... */

	  case 2:
		optpreset(o_xrows, (long)h, OPT_HIDE);
		optpreset(o_xcolumns, (long)w, OPT_HIDE);
		break;
	}
	optpreset(o_icon, True, OPT_HIDE);
	optpreset(o_stopshell, toCHAR(DEFAULT_STOPSHELL), OPT_HIDE|OPT_UNSAFE);
	optpreset(o_altkey, 's', OPT_HIDE); /* setbit */
	optpreset(o_warpto, 'd', OPT_HIDE); /* don't */
	optpreset(o_focusnew, True, OPT_HIDE);
	optpreset(o_textcursor, 'x', OPT_HIDE); /* xor */
	optpreset(o_underline, True, OPT_HIDE|OPT_SCRATCH);
	optpreset(o_outlinemono, DEFAULT_OUTLINEMONO, OPT_HIDE);
	optpreset(o_borderwidth, DEFAULT_BORDERWIDTH, OPT_HIDE);
	optinsert("x11", QTY(x11desc), x11desc, (OPTVAL *)&x_optvals);

	/* initialize the colors */
	if (!*x_foreground) CHARcpy(x_foreground, toCHAR(DEFAULT_FOREGROUND));
	if (!*x_background) CHARcpy(x_background, toCHAR(DEFAULT_BACKGROUND));
	CHARcpy(x_cursorcolor, toCHAR(DEFAULT_CURSORCOLOR));
	CHARcpy(x_owncolor, toCHAR(DEFAULT_OWNCOLOR));
	CHARcpy(x_toolfg, toCHAR(DEFAULT_TOOLFG));
	CHARcpy(x_toolbg, toCHAR(DEFAULT_TOOLBG));
	CHARcpy(x_scrollbarfg, toCHAR(DEFAULT_SCROLLBARFG));
	CHARcpy(x_scrollbarbg, toCHAR(DEFAULT_SCROLLBARBG));

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
		flags = XGeometry(x_display, x_screen, geomstr,
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
		flags = XGeometry(x_display, x_screen, geomstr,
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
	msg(MSG_INFO, "       -courier size      Use three Courier fonts of given size");
	msg(MSG_INFO, "       -fc controlfont    Use \"controlfont\" in the toolbar");
	msg(MSG_INFO, "       -mono              Monochrome -- force all colors to black or white");
	msg(MSG_INFO, "       -fg color          Use \"color\" for the foreground \\(default black\\)");
	msg(MSG_INFO, "       -bg color          Use \"color\" for the background \\(default white\\)");
	msg(MSG_INFO, "[s]       -geometry WxH+X+Y  Set the window's size and/or position \\(default $1\\)", DEFAULT_GEOMETRY);
	msg(MSG_INFO, "       -noicon            Don't use built-in bitmap icon");
	msg(MSG_INFO, "       -iconic            First window should start iconified");
	msg(MSG_INFO, "       -sync              Disable X11 buffering, for debugging");
	msg(MSG_INFO, "       -fork              Run in background");
	msg(MSG_INFO, "       -client            Edit files via an existing elvis process");
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
	for (xw = x_winlist, prev = NULL; xw != (X11WIN *)gw; prev = xw, xw = xw->next)
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
		x_winlist = xw->next;
	}

	/* switch keyboard focus to another elvis window */
	if (x_winlist != NULL)
	{ /* nishi */
		/* Choose one which isn't iconified */
		for (prev = x_winlist; prev && !prev->ismapped; prev = prev->next)
		{
		}
		if (prev && o_focusnew)
		{
			focusgw((GUIWIN *)x_winlist);
		}

		/* Also make the other window be an elvis server window.  This
		 * is only significant if the doomed window used to be the
		 * server, but it is just as easy to change it every time.
		 */
		XChangeProperty(x_display, root, x_elvis_server, XA_WINDOW, 32,
			PropModeReplace, (unsigned char *)&x_winlist->win, 1);
	}

	/* destroy any dialogs which used this window */
	x_dl_destroy(xw);

	/* free the window's resources */
	x_tb_destroy(xw);
	x_sb_destroy(xw);
	x_ta_destroy(xw);
	x_st_destroy(xw);
	XFreeGC(x_display, xw->gc);
#ifndef NO_XLOCALE
	XDestroyIC(xw->ic);
#endif
	if (xw->win)
	{
		XDestroyWindow(x_display, xw->win);
	}
	safefree(xw->title);
	safefree(xw);
}



/* This function changes the keyboard focus to a specific window */
static BOOLEAN focusgw(gw)
	GUIWIN	*gw;	/* the window to receive keyboard focus */
{
	X11WIN	*xw = (X11WIN *)gw;
	X11WIN	*hadfocus;
	int	x1, y1, x2, y2;

	/* If the window is unmapped (iconfied) then map it.  Also, if the
	 * autoiconify option is set then unmap the previous window.
	 */
	if (!xw->ismapped)
	{
		XMapWindow(x_display, xw->win);
		
		if (o_autoiconify && windefault && windefault->gw != gw)
		{
			XIconifyWindow(x_display, ((X11WIN *)windefault->gw)->win, x_screen);
		}
		
		/* the rest of the focus change must wait until it is mapped */
		return True;
	}

	/* Raise the window. (I.e., make it fully visible.) */
	XRaiseWindow(x_display, xw->win);

	/* Move the pointer to some point in the window, so that if
	 * keyboard focus follows the mouse, this will switch focus.
	 */
	if (!xw->nowarp)
	{
		/* if the scrollbar is hidden, and we're supposed to warp to
		 * the middle of the scrollbar, then warp to origin instead.
		 */
		if (!o_scrollbar && o_warpto == 's')
			o_warpto = 'o';

		/* warp the pointer to the indicated spot */
		switch (o_warpto)
		{
		  case 'o': /* "origin" */
			XWarpPointer(x_display, None, xw->win, 0,0,0,0, 0,0);
			break;

		  case 's': /* scrollbar */
			XWarpPointer(x_display, None, xw->win, 0,0,0,0,
			    xw->sb.x + xw->sb.w/2, xw->sb.y + xw->sb.h/2);
			break;

		  case 'c': /* corners */

			/* set x1,y1 to the furthest corner, x2,y2 to nearest */
			if (xw->ta.cursx < xw->ta.columns / 2)
				x1 = xw->w - 1, x2 = 0;
			else
				x1 = 0, x2 = xw->w -1;
			if (xw->ta.cursy < xw->ta.rows / 2)
				y1 = xw->h - 1, y2 = 0;
			else
				y1 = 0, y2 = xw->h - 1;

			/* warp to furthest corner unless it's off the screen */
			if (y1 + xw->y < rootheight && x1 + xw->x < rootwidth)
				XWarpPointer(x_display, None, xw->win, 0,0,0,0, x1,y1);
			XFlush(x_display);

			/* warp to nearest corner */
			XWarpPointer(x_display, None, xw->win, 0,0,0,0, x2,y2);
			break;

		  /* case 'd': don't -- requires no action */
		}
	}
	xw->nowarp = False;

	/* if some other elvis window had focus, it doesn't now! */
	if (x_hasfocus && x_hasfocus != xw)
	{
		hadfocus = x_hasfocus;
		x_hasfocus = NULL;
		x_ta_erasecursor(hadfocus);
		x_ta_drawcursor(hadfocus);
	}

	/* Explicitly change the focus */
	XSetInputFocus(x_display, xw->win, RevertToParent, x_now);
	x_hasfocus = xw;
	return True;
}


/* In a loop, receive events from the GUI and call elvis
 * functions which will act on the event.  When this function
 * returns, elvis will call the GUI's term() function and then exit.
 * (This function should return only when the number of windows becomes 0.)
 */
static void loop P_((void))
{
	XEvent	*event;		/* an X event to process */
	X11WIN	*xw;
	BOOLEAN	oldtoolbar;
	BOOLEAN	oldstatusbar;
	BOOLEAN	oldscrollbar;
	BOOLEAN	oldscrollbarleft;

	/* loop until we don't have any windows left */
	oldtoolbar = o_toolbar;
	oldstatusbar = o_statusbar;
	oldscrollbar = o_scrollbar;
	oldscrollbarleft = o_scrollbarleft;
	x_didcmd = True;
	while (x_winlist)
	{
		/* draw new window images, if they may have changed */
		if ((x_repeating || XEventsQueued(x_display, QueuedAfterFlush) == 0)
			 && x_didcmd)
		{
			/* for each window... */
			for (xw = x_winlist; xw; xw = xw->next)
			{
				if (xw->ismapped)
				{
					xw->ta.nextcursor = eventdraw((GUIWIN *)xw);
					x_ta_drawcursor(xw);
					x_sb_setstate(xw, xw->ta.cursy == xw->ta.rows - 1 ? X_SB_BLANK : X_SB_NORMAL);
					x_tb_draw(xw, False);
				}
			}
			x_didcmd = False;
		}

		/* get an event */
		event = x_ev_wait();

		/* process the event */
		x_ev_process(event);

		/* Changing certain Boolean options (currently "toolbar",
		 * "statusbar", "scrollbar", and "scrollbarleft") don't set
		 * the allreconfig flag, but should.
		 */
		if (x_winlist && (o_toolbar != oldtoolbar
			       || o_statusbar != oldstatusbar
			       || o_scrollbar != oldscrollbar
			       || o_scrollbarleft != oldscrollbarleft))
		{
			oldtoolbar = o_toolbar;
			oldstatusbar = o_statusbar;
			oldscrollbar = o_scrollbar;
			oldscrollbarleft = o_scrollbarleft;
			allreconfig = True;
		}

		/* if the event processing changed window parameters, then
		 * reconfigure all windows
		 */
		if (x_winlist && allreconfig)
		{
			allreconfig = False;
			for (xw = x_winlist; xw; xw = xw->next)
			{
				x_reconfig(xw, xw->ta.columns, xw->ta.rows);
			}
		}
	}
}


/* Test for signs of boredom from the user, so we can cancel long operations.
 * Here, we check to see if user has clicked on the window.
 */
static BOOLEAN wpoll(reset)
	BOOLEAN reset;
{
	return x_ev_poll(reset);
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
	unsigned udummy;
	Window	wdummy;
	unsigned width, height;

	/* unload fonts */
	x_unloadfont(x_defaultnormal);
	if (x_defaultbold) x_unloadfont(x_defaultbold);
	if (x_defaultitalic) x_unloadfont(x_defaultitalic);
	x_unloadfont(x_loadedcontrol);

	/* forget any tools */
	guicmd(NULL, "newtoolbar");

	/* warp the cursor back to the original (non-Elvis) window */
	if (o_warpback && fromwin != root)
	{
		XGetInputFocus(x_display, &curfocus, &dummy);
	 	if (curfocus != fromwin)
		{
			width = 0; /* just in case XGetGeometry() fails */
			XGetGeometry(x_display, fromwin, &wdummy,
				&dummy, &dummy, &width, &height,
				&udummy, &udummy);

			/* move the pointer to opposite corners of the
			 * original window, so that if the screen scrolls
			 * (as in XFree86) the whole window becomes visible.
			 * Then leave the pointer in the middle of the
			 * window's top edge.
			 */
			XWarpPointer(x_display, None, fromwin,
				0, 0, 0, 0, width - 1, 0);
			XWarpPointer(x_display, None, fromwin,
				0, 0, 0, 0, 0, height - 1);
			XWarpPointer(x_display, None, fromwin,
				0, 0, 0, 0, width / 2, 0);
		}
	}
	XSetInputFocus(x_display, fromwin, RevertToParent, x_now);

	/* delete the server property from the root window */
	XDeleteProperty(x_display, root, x_elvis_server);

	/* close the connection to the display */
	XCloseDisplay(x_display);
}

#ifndef NO_XLOCALE
/* Create an input context for a given window.  The first time this function is
 * called, it will also initialize a few static variables.
 */
static XIC createic(window)
	Window	window;
{
	char	*p;
	XIMStyles *xim_styles = NULL;
	int	found;
 static XIM	xim = NULL;
 static XIMStyle input_style = 0;
 static int	firsttime = True;
	int	i;

	if (firsttime)
	{
		/* Open the input method. */
		if (((p = XSetLocaleModifiers("@im=none")) != NULL && *p)
		 || ((p = XSetLocaleModifiers("")) != NULL && *p))
			xim = XOpenIM(x_display, NULL, NULL, NULL);
		if (xim == NULL)
		{
			/* Failed to open input method */
			return NULL;
		}

		/* Check for input styles */
		if (XGetIMValues(xim, XNQueryInputStyle, &xim_styles, NULL)
		 || !xim_styles)
		{
			/* input method doesn't support any style */
			XCloseIM(xim);
			return NULL;
		}

		/* Use the "Root" preedit type */
		input_style = (XIMPreeditNothing | XIMStatusNothing);
		for (i = 0; i < xim_styles->count_styles; i++)
		{
			if (input_style == xim_styles->supported_styles[i])
			{
				found = 1;
				break;
			}
		}
		XFree(xim_styles);
		if (found == 0)
		{
			/* input method doesn't support my preedit type */
			XCloseIM(xim);
			return NULL;
		}

		/* we have now successfully chosen an xim and input_style */
		firsttime = False;
	}

	/* create the context */
	return XCreateIC(xim, XNInputStyle, input_style,
			      XNClientWindow, window,
			      XNFocusWindow, window,
			      NULL);
}
#endif	/* NO_XLOCALE */


/* Create a new window for the buffer named name.  If successful,
 * return TRUE and then simulate a "create" event later.  Return
 * FALSE if the GUIWIN can't be created, e.g., because the GUI doesn't
 * support multiple windows.  The msg() function should be called to
 * describe the reason for the failure.
 */
static BOOLEAN creategw(name, firstcmd)
	char	*name;		/* name of buffer for the new window */
	char	*firstcmd;	/* other window parameters, if any */
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
	if (!x_winlist)
	{
		/* allocate the fonts named in options. */
		x_defaultnormal = x_loadfont(o_normalfont ? tochar8(o_normalfont) : DEFAULT_NORMALFONT);
		if (!x_defaultnormal) return False;
		if (o_boldfont && !(x_defaultbold = x_loadfont(tochar8(o_boldfont))))
			return False;
		if (o_italicfont && !(x_defaultitalic = x_loadfont(tochar8(o_italicfont))))
			return False;
		x_loadedcontrol = x_loadfont(o_controlfont ? tochar8(o_controlfont) : DEFAULT_CONTROLFONT);
		if (!x_loadedcontrol) return False;

		/* lock the options that can only be changed during initialization */
		optflags(o_icon) |= OPT_LOCK;

		/* initialize the options that depend on elvis.msg */
		optpreset(o_submit, msgtranslate("Submit"), OPT_HIDE|OPT_FREE);
		optpreset(o_cancel, msgtranslate("Cancel"), OPT_HIDE|OPT_FREE);
		optpreset(o_help, msgtranslate("Help"), OPT_HIDE|OPT_FREE);
	}

	/* allocate storage space */
	xw = (X11WIN *)safealloc(1, sizeof(*xw));
	xw->title = safedup(name);
	xw->next = x_winlist;
	x_winlist = xw;

	/* default pixel values */
	xw->grexpose = False;
	x_white = xw->bg = WhitePixel(x_display, x_screen);
	x_black = xw->fg = BlackPixel(x_display, x_screen);

	/* compute the widget sizes */
	x_ta_predict(xw, (unsigned int)o_xcolumns, (unsigned int)o_xrows);
	x_sb_predict(xw, o_scrollbarwidth, xw->ta.h);
	x_tb_predict(xw, xw->ta.w + xw->sb.w, 0);
	x_st_predict(xw, xw->ta.w + xw->sb.w, 0);

	/* default window size */
	hint.x = hint.y = 0;
	hint.width = xw->ta.w + xw->sb.w;
	hint.height = xw->tb.h + xw->ta.h + xw->st.h;
	hint.width_inc = xw->ta.cellw;
	hint.height_inc = xw->ta.cellh;
	hint.base_width = hint.width - o_xcolumns * hint.width_inc;
	hint.base_height = hint.height - o_xrows * hint.height_inc;
	hint.min_width = hint.base_width + 30 * hint.width_inc;
	hint.min_height = hint.base_height + 2 * hint.height_inc;
	hint.max_width = hint.base_width + 200 * hint.width_inc;
	hint.max_height = hint.base_height + 200 * hint.height_inc;
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

	/* top-level application window creation */
	xw->w = hint.width;
	xw->h = hint.height;
	xw->win = XCreateSimpleWindow(x_display, root,
		hint.x, hint.y, (unsigned)xw->w, (unsigned)xw->h,
		5, xw->ta.bg, xw->ta.bg);

	/* GC creation and initialization */
	gcvalues.foreground = xw->fg;
	gcvalues.background = xw->bg;
	gcvalues.font = x_defaultnormal->fontinfo->fid;
	gcvalues.graphics_exposures = xw->grexpose;
	xw->gc = XCreateGC(x_display, xw->win,
		GCForeground|GCBackground|GCFont|GCGraphicsExposures, &gcvalues);

	/* widget creation */
	x_tb_create(xw, 0, 0);
	if (!o_scrollbarleft)
	{
		x_ta_create(xw, 0, xw->tb.h);
		x_sb_create(xw, xw->ta.w, xw->tb.h);
	}
	else
	{
		x_sb_create(xw, 0, xw->tb.h);
		x_ta_create(xw, xw->sb.w, xw->tb.h);
	}
	x_st_create(xw, 0, xw->tb.h + xw->ta.h);

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
		wmhint.icon_pixmap = x_elvis_icon;
		wmhint.flags = InputHint | StateHint | IconPixmapHint;
	}
	else
	{
		wmhint.flags = InputHint | StateHint;
	}
	XSetWMProperties(x_display, xw->win, &textprops[0], &textprops[1], argv, argc, &hint, &wmhint, NULL);
	XFree(textprops[0].value);
	XFree(textprops[1].value);

	/* set class hints */
	class.res_name = "elvis";
	class.res_class = "Elvis";
	XSetClassHint(x_display, xw->win, &class);

	/* allow window manager's "Delete" menu item to work */
	XSetWMProtocols(x_display, xw->win, &x_wm_delete_window, 1);
	
	/* make it work as an elvis server window */
	XChangeProperty(x_display, root, x_elvis_server, XA_WINDOW, 32,
		PropModeReplace, (unsigned char *)&xw->win, 1);

#ifndef NO_XLOCALE
	/* give it an input context */
	xw->ic = createic(xw->win);
#endif

	/* input event selection */
	XSelectInput(x_display, xw->win,
		KeyPressMask | KeyReleaseMask | FocusChangeMask
			| StructureNotifyMask | PropertyChangeMask);

	/* window mapping */
	XMapSubwindows(x_display, xw->win);
	XMapRaised(x_display, xw->win);
	xw->ismapped = (BOOLEAN)(wmhint.initial_state == NormalState);

	/* simulate a "window create" event */
	eventcreate((GUIWIN *)xw, NULL, name, (int)xw->ta.rows, (int)xw->ta.columns);

	/* if there is a firstcmd, then execute it */
	if (firstcmd)
	{
		winoptions(winofgw((GUIWIN *)xw));
		exstring(windefault, toCHAR(firstcmd), "+cmd");
	}

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
	XSetWMName(x_display, xw->win, &textprop);
	XFree(textprop.value);

	/* also change the icon name */
	name = dirfile(name);
	XStringListToTextProperty(&name, 1, &textprop);
	XSetWMIconName(x_display, xw->win, &textprop);
	XFree(textprop.value);
}


/* Flush all changes out to the screen */
static void flush()
{
	XFlush(x_display);
}


/* Move the cursor to a given character cell.  The upper left
 * character cell is designated column 0, row 0.
 */
static void moveto(gw, column, row)
	GUIWIN	*gw;	/* the window whose cursor is to be moved */
	int	column;	/* the column to move to */
	int	row;	/* the row to move to */
{
	x_ta_moveto((X11WIN *)gw, column, row);
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
	x_ta_draw((X11WIN *)gw, font, text, len);
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
	x_ta_shift((X11WIN *)gw, qty, rows);
	return True;
}

static BOOLEAN scroll(gw, qty, notlast)
	GUIWIN	*gw;	/* window to be scrolled */
	int	qty;	/* amount to scroll by (pos=downward, neg=upward) */
	BOOLEAN	notlast;/* if True, last row should not be affected */
{
	x_ta_scroll((X11WIN *)gw, qty, notlast);
	return True;
}

static BOOLEAN clrtoeol(gw)
	GUIWIN	*gw;	/* window whose row is to be cleared */
{
	x_ta_clrtoeol((X11WIN *)gw);
	return True;
}


/* Ring the bell */
static void beep(gw)
	GUIWIN	*gw;	/* window that generated a beep */
{
	struct timeval	timeout;

	if (o_flash && x_hasfocus)
	{
		/* invert the text area */
		XSetForeground(x_display, x_hasfocus->gc,
				x_hasfocus->ta.fgnormal ^ x_hasfocus->ta.bg);
		XSetFunction(x_display, x_hasfocus->gc, GXinvert);
		XFillRectangle(x_display, x_hasfocus->ta.win, x_hasfocus->gc,
			0, 0, x_hasfocus->ta.w, x_hasfocus->ta.h);

		/* wait 1/20 of a second */
		XFlush(x_display);
		timeout.tv_sec = 0L;
		timeout.tv_usec = 50000L; /* 0.1 seconds */
		(void)select(0, NULL, NULL, NULL, &timeout);

		/* invert the text area again, making it normal */
		XFillRectangle(x_display, x_hasfocus->ta.win, x_hasfocus->gc,
			0, 0, x_hasfocus->ta.w, x_hasfocus->ta.h);
		XSetForeground(x_display, x_hasfocus->gc, x_hasfocus->fg);
		XSetFunction(x_display, x_hasfocus->gc, GXcopy);
	}
	else
		XBell(x_display, 0);
}


/* draw the scrollbar */
static void scrollbar(gw, top, bottom, total)
	GUIWIN	*gw;	/* window whose scrollbar should be updated */
	long	top;	/* offset of char at top of screen */
	long	bottom;	/* offset of char at bottom of screen */
	long	total;	/* total number of characters in buffer */
{
	if (total > 0)
		x_sb_thumb((X11WIN *)gw, top, bottom, total);
	else
		x_sb_thumb((X11WIN *)gw, 0L, 1L, 1L);
}



/* maintain the list of toolbar buttons */
static BOOLEAN guicmd(gw, extra)
	GUIWIN	*gw;		/* window where command was typed (ignored) */
	char	*extra;		/* label, operator, and command/condition */
{
	char	*label, *end;
	char	op;
 static	BOOLEAN	gap;
	CHAR	*dump;

	/* if no "extra" string, then pretend it is "" */
	if (!extra)
		extra = "";

	/* Parse the label and operator.  Leave "extra" pointing to value */
	if (*extra == '~')
		op = *extra++;
	else
		op = '\0';
	for (label = extra; *extra && !strchr(":\";=?~", *extra); extra++)
	{
	}
	end = extra;
	if (*extra)
		op = *extra++;

	/* Trim whitespace */
	while (label < end && isspace(end[-1]))
		end--;
	if (*end)
		*end = '\0';
	if (op)
	{
		while (isspace(*extra))
		{
			extra++;
		}
	}

	/* check for some special words */
	if (!op)
	{
		/* not followed by operator -- maybe a special word? */
		if (!strcmp(label, "gap"))
		{
			gap = True;
			return True;
		}
		else if (!strcmp(label, "newtoolbar"))
		{
			x_tb_config(False, NULL, '\0', NULL);
			allreconfig = True;
			return True;
		}
		else /* empty or tool's label (with no operator), so dump it */
		{
			dump = x_tb_dump(label);
			if (dump)
			{
				drawextext(winofgw(gw), dump, CHARlen(dump));
				safefree(dump);
				return True;
			}
			msg(MSG_ERROR, "no tool");
			return False;
		}
	}

	/* configure the toolbar button */
	x_tb_config(gap, label, op, extra);

	/* clobber the "gap" flag -- either we just used it, or we never will */
	gap = False;

	/* set the allreconfig flag, so all windows will be reconfigured */
	allreconfig = True;

	return True;
}


/* adjust the statusbar */
static BOOLEAN status(gw, cmd, line, column, learn, mode)
	GUIWIN	*gw;	/* window whose status bar should be updated */
	CHAR	*cmd;	/* partial command keys */
	long	line;	/* line number */
	long	column;	/* column number */
	_CHAR_	learn;	/* learn letter, or '*' if modified, or ' ' otherwise */
	char	*mode;	/* mode name, e.g. "Command" or "Input" */
{
	/* if status bar is disabled, then fail */
	if (!o_statusbar)
		return False;

	/* change the window's info */
	x_st_status((X11WIN *)gw, cmd, line, column, learn, mode);
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


static BOOLEAN color(gw, font, fgname, bgname)
	GUIWIN	*gw;	/* window whose colors should be changed, or NULL */
	_char_	font;	/* font letter of font to change */
	CHAR	*fgname;/* name of new foreground color */
	CHAR	*bgname;/* name of new background color, or NULL */
{
	X11WIN	*xw = (X11WIN *)gw;
	CHAR	*fgptr, *bgptr;		/* pointers to color names */

	/* decide which colors we should change */
	bgptr = x_background;
	switch (font)
	{
	  case 'b': fgptr = x_boldcolor;				break;
	  case 'e': fgptr = x_emphcolor;				break;
	  case 'u': fgptr = x_underlinecolor;				break;
	  case 'i': fgptr = x_italiccolor;				break;
	  case 'f': fgptr = x_fixedcolor;				break;
	  case 'c': fgptr = x_cursorcolor;	bgptr = x_owncolor;	break;
	  case 's': fgptr = x_scrollbarfg;	bgptr = x_scrollbarbg;	break;
	  case 't': fgptr = x_toolfg;		bgptr = x_toolbg;	break;
	  default:  fgptr = x_foreground;
	}

	/* store the color names */
	CHARcpy(fgptr, fgname);
	if (bgname)
		CHARcpy(bgptr, bgname);

	/* inform the widgets of the new colors */
	if (gw)
	{
		x_ta_recolor(xw, font);
		x_sb_recolor(xw, font);
		x_tb_recolor(xw, font);
		x_st_recolor(xw, font);

		/* There is a 1-pixel gap around the text area.  Change it to
		 * match the text area's background.
		 */
		XSetWindowBackground(x_display, xw->win, xw->ta.bg);
		XClearWindow(x_display, xw->win);

		/* leave the foreground and background colors in the GC set to
		 * reasonable values.  (Their old values may have been freed.)
		 */
		XSetForeground(x_display, xw->gc, xw->ta.fgnormal);
		xw->fg = xw->ta.fgnormal;
		XSetBackground(x_display, xw->gc, xw->ta.bg);
		xw->bg = xw->ta.bg;
	}
	return True;
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
	status,
	keylabel,
	x_clipopen,
	x_clipwrite,
	x_clipread,
	x_clipclose,
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
