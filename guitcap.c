/* guitcap.c */

#include "elvis.h"
#ifdef GUI_TERMCAP
#include <signal.h>

/* This file contains a termcap-based user interface.  It is derived from the
 * "curses.c" file of elvis 1.8.
 */

#define	MINHEIGHT	4

/* Some termcap packages require the application code to supply a "BC"
 * variable.  Others (particularly ncurses) forbid it.  The nice ones
 * supply one if you don't, so they'll work either way.
 */
#ifdef NEED_BC
       char	*BC;	/* :bc=: move cursor left */
#else
extern char	*BC;	/* :bc=: move cursor left */
#endif

/* HP-UX, and maybe some others, require the application code to supply
 * an "ospeed" variable.
 */
#ifdef NEED_OSPEED
# ifdef NEED_SPEED_T
	speed_t ospeed;
# else
	short	ospeed;
# endif
#endif

/* Structs of this type are used to remember the location and size of each 
 * window.  In the termcap interface, all windows must be as wide as the
 * screen, and the sum of all windows' heights must equal the screen size.
 */
typedef struct twin_s
{
	struct twin_s	*next;		/* some other window on this screen */
	int		height;		/* size of the window */
	int		pos;		/* position of the window */
	int		newheight;	/* height after screen is rearranged */
	int		newpos;		/* position after screen is rearranged */
	int		cursx, cursy;	/* logical cursor position */
	ELVCURSOR	shape;		/* logical cursor shape */

	/* The following buffers hold the escape codes that switch
	 * between fonts.  The initial values are taken from the termcap
	 * strings above, but after colors have been set they'll all
	 * contain computed strings which switch colors in addition to
	 * fonts.
	 *
	 * Note that there is no endnormal[] array.  This is because
	 * starnormal[] is used in a special way.  Before any colors are
	 * set, startnormal[] contains an empty string, and the endXXX
	 * strings point to termcap strings.  After colors have been set,
	 * though, startnormal is loaded with the escape sequence for
	 * switching to the normal colors, and the endXXX strings are
	 * loaded with copies of this (except for endunderline, which
	 * contains that string plus the regular UE termcap string).
	 */
	char	startnormal[30];
	char	startfixed[30], endfixed[30];
	char	startbold[30], endbold[30];
	char	startemph[30], endemph[30];
	char	startitalic[30], enditalic[30];
	char	startunderline[30], endunderline[30];
	char	starthilite[30], endhilite[30]; /* regardless of font */
} TWIN;


#if USE_PROTOTYPES
static BOOLEAN ansi_color(TWIN *tw, _char_ font, char *fgname, char *bgname);
static BOOLEAN clrtoeol(GUIWIN *gw);
static BOOLEAN color(GUIWIN *gw, _char_ font, CHAR *fg, CHAR *bg);
static BOOLEAN creategw(char *name,char * attributes);
static BOOLEAN focusgw(GUIWIN *gw);
static BOOLEAN scroll(GUIWIN *gw, int qty, BOOLEAN notlast);
static BOOLEAN shift(GUIWIN *gw, int qty, int rows);
static BOOLEAN tabcmd(GUIWIN *gw, _CHAR_ key2, long count);
static char *manynames(char *names);
static int init(int argc, char **argv);
static int keylabel(CHAR *given, int givenlen, CHAR **label, CHAR **rawptr);
static int test(void);
static int ttych(int ch);
static void beep(GUIWIN *gw);
static void destroygw(GUIWIN *gw, BOOLEAN force);
static void drawgraphic(GUIWIN *gw, _char_ font, CHAR *text, int len);
static void draw(GUIWIN *gw, _char_ font, CHAR *text, int len);
static void drawborder(TWIN *tw);
static void endtcap(void);
static void flush(void);
static void loop(void);
static void mayhave(char **T, char *s);
static void movecurs(TWIN *tw);
static void moveto(GUIWIN *gw, int column, int row);
static void musthave(char **T, char *s);
static void pair(char **T, char **U, char *sT, char *sU);
static void revert(TWIN *tw);
static void starttcap(void);
static void term(void);
static void ttyflush(void);
static void ttygetsize(void);
static BOOLEAN ttyprgopen(char *command, BOOLEAN willwrite, BOOLEAN willread);
static int ttyprgclose(void);
static RESULT stop(BOOLEAN alwaysfork);
#endif

static void reset P_((void));
static void chgsize P_((TWIN *tw, int newheight, BOOLEAN winch));
static void cursorshape P_((ELVCURSOR shape));

/* termcap values */
static BOOLEAN	AM;		/* :am:  boolean: auto margins? */
static BOOLEAN	PT;		/* :pt:  boolean: physical tabs? */
       char	PC;		/* :pc=: pad character (not a string var!) */
static char	*VB;		/* :vb=: visible bell */
       char	*UP;		/* :up=: move cursor up */
static char	*SO;		/* :so=: standout start */
static char	*SE;		/* :se=: standout end */
static char	*US;		/* :us=: underline start */
static char	*UE;		/* :ue=: underline end */
static char	*MD;		/* :md=: bold start */
static char	*ME;		/* :me=: bold end */
static char	*MH;		/* :mh=: half-bright start (end with :me=:) */
static char	*CM;		/* :cm=: cursor movement */
static char	*CE;		/* :ce=: clear to end of line */
static char	*AL;		/* :al=: add a line */
static char	*ALmany;	/* :AL=: add many lines */
static char	*DL;		/* :dl=: delete a line */
static char	*DLmany;	/* :DL=: delete many lines */
static char	*SRev;		/* :sr=: scroll reverse */
static char	*KS;		/* :ks=: init string for cursor */
static char	*KE;		/* :ke=: restore string for cursor */
static char	*IC;		/* :ic=: insert following char */
static char	*ICmany;	/* :IC=: insert many characters */
static char	*DC;		/* :dc=: delete a character */
static char	*DCmany;	/* :DC=: delete many characters */
static char	*TI;		/* :ti=: terminal init */
static char	*TE;		/* :te=: terminal exit */
static char	*CQ;		/* :cQ=: normal cursor */
static char	*CX;		/* :cX=: cursor used for EX command/entry */
static char	*CV;		/* :cV=: cursor used for VI command mode */
static char	*CI;		/* :cI=: cursor used for VI input mode */
static char	*CR;		/* :cR=: cursor used for VI replace mode */
static char	*GS;		/* :GS=:as=: start graphic character mode */
static char	*GE;		/* :GE=:ae=: end graphic character mode */
static char	GC_V;		/* vertical bar character */
static char	GC_H;		/* horizontal bar character */
static char	GC_1;		/* lower left corner character */
static char	GC_2;		/* horizontal line with up-tick character */
static char	GC_3;		/* lower right corner character */
static char	GC_4;		/* vertical line with right-tick character */
static char	GC_5;		/* center cross character */
static char	GC_6;		/* vertical line with left-tick character */
static char	GC_7;		/* upper left corner character */
static char	GC_8;		/* horizontal line with down-tick character */
static char	GC_9;		/* upper right corner character */



/* This is a table of keys which should be mapped, if present */
static struct
{
	char	*label;		/* keytop legend of the key */
	char	*capnames;	/* name(s) of the key's capability */
	char	*cooked;	/* what the key should map to (if anything) */
	MAPFLAGS flags;		/* when the map should be effective */
	char	*rawin;		/* raw characters sent by key */
}
	keys[] =
{
	{"<Up>",	"ku",		"k",	MAP_ALL},
	{"<Down>",	"kd",		"j",	MAP_ALL},
	{"<Left>",	"kl",		"h",	MAP_ALL},
	{"<Right>",	"kr",		"l",	MAP_ALL},
	{"<PgUp>",	"PUkPk2",	"\002", MAP_ALL},
	{"<PgDn>",	"PDkNk5",	"\006", MAP_ALL},
	{"<Home>",	"HMkhK1",	"^",	MAP_ALL},
	{"<End>",	"ENkHK5",	"$",	MAP_ALL},
	{"<Insert>",	"kI",		"i",	MAP_ALL},
	{"<Delete>",	"kD",		"x",	MAP_ALL},
	{"#1",		"k1"},
	{"#2",		"k2"},
	{"#3",		"k3"},
	{"#4",		"k4"},
	{"#5",		"k5"},
	{"#6",		"k6"},
	{"#7",		"k7"},
	{"#8",		"k8"},
	{"#9",		"k9"},
	{"#10",		"k0kak;"},
	{"#1s",		"s1"},
	{"#2s",		"s2"},
	{"#3s",		"s3"},
	{"#4s",		"s4"},
	{"#5s",		"s5"},
	{"#6s",		"s6"},
	{"#7s",		"s7"},
	{"#8s",		"s8"},
	{"#9s",		"s9"},
	{"#10s",	"s0"},
	{"#1c",		"c1"},
	{"#2c",		"c2"},
	{"#3c",		"c3"},
	{"#4c",		"c4"},
	{"#5c",		"c5"},
	{"#6c",		"c6"},
	{"#7c",		"c7"},
	{"#8c",		"c8"},
	{"#9c",		"c9"},
	{"#10c",	"c0"},
	{"#1a",		"a1"},
	{"#2a",		"a2"},
	{"#3a",		"a3"},
	{"#4a",		"a4"},
	{"#5a",		"a5"},
	{"#6a",		"a6"},
	{"#7a",		"a7"},
	{"#8a",		"a8"},
	{"#9a",		"a9"},
	{"#10a",	"a0"}
};

/* These are GUI-dependent global options */
static struct
{
	OPTVAL	term;		/* string - terminal type */
	OPTVAL	ttyrows;	/* number - rows of screen */
	OPTVAL	ttycolumns;	/* number - columns of screen */
	OPTVAL	ttyunderline;	/* boolean - whether colors and underline mix */
} goptvals;
static OPTDESC goptdesc[] =
{
	{"term", "termtype",	optsstring,	optisstring},
	{"ttyrows", "ttylines",	optnstring,	optisnumber},
	{"ttycolumns", "ttycolumns",optnstring,	optisnumber},
	{"ttyunderline", "ttyu",NULL,		NULL	   },
};
#define o_term		goptvals.term.value.string
#define o_ttyrows	goptvals.ttyrows.value.number
#define o_ttycolumns	goptvals.ttycolumns.value.number
#define o_ttyunderline	goptvals.ttyunderline.value.boolean


/*----------------------------------------------------------------------------*/
/* These are mid-level terminal I/O functions.  They buffer the output, but
 * don't do much more than that.
 */
static char ttybuf[1500];	/* the output buffer */
static int  ttycount;		/* number of characters in ttybuf */
static char ttyerasekey;	/* taken from the ioctl structure */
static char *startfont="";	/* string for starting current font */
static char *endfont="";	/* string for ending current font */
long        ttycaught;		/* bitmap of recently-received signals */

/* This function writes the contents of ttybuf() to the screen */
static void ttyflush()
{
	if (ttycount > 0)
	{
		ttywrite(ttybuf, ttycount);
		ttycount = 0;
	}
}

/* This function is used internally.  It is passed to the tputs() function
 * which uses it to output individual characters.  This function saves the
 * characters in a buffer and outputs them in a bunch.
 */
static int ttych(ch)
	int	ch;	/* character to be output */
{
	ttybuf[ttycount++] = ch;
	if (ttycount >= QTY(ttybuf))
		ttyflush();
	return ch;
}


/* Revert to the normal font for a given window... or just end any special
 * fonts if no window is given.
 */
static void revert(tw)
	TWIN	*tw;	/* window whose normal font is to be used, or NULL */
{
	/* revert to the normal font */
	if (*endfont)
	{
		tputs(endfont, 0, ttych);
		startfont = endfont = "";
	}

	/* If a window is specified, and its normal font string isn't empty,
	 * then output it.
	 */
	if (tw && tw->startnormal[0] != '\0')
	{
		tputs(tw->startnormal, 0, ttych);
	}
}


/* Send any required termination strings.  Turn off "raw" mode. */
void ttysuspend()
{
	/* revert to the normal font */
	revert(NULL);

	if (CQ)
	{
		tputs(CQ, 1, ttych);
	}
	if (TE)
	{
		tputs(TE, 1, ttych);
	}
	if (KE)
	{
		tputs(KE, 1, ttych);
	}
	ttyflush();

	/* change the terminal mode back the way it was */
	ttynormal();
}

/* Put the terminal in RAW mode.  Send any required strings */
void ttyresume(sendstr)
	BOOLEAN	sendstr;	/* send strings? */
{
	/* change the terminal mode to cbreak/noecho */
	ttyerasekey = ELVCTRL('H');/* the default */
	ttyraw(&ttyerasekey);

	/* send the initialization strings */
	if (sendstr)
	{
		ttych('\r');
		tputs(CE, (int)o_ttycolumns, ttych);
		if (TI)
		{
			tputs(TI, 1, ttych);
		}
		if (KS)
		{
			tputs(KS, 1, ttych);
		}

		/* reset, so we don't try any suspicious optimizations */
		reset();
	}
}

/* This function determines the screen size.  It does this by calling the
 * OS-dependent ttysize() function if possible, or from the termcap entry
 * otherwise.
 */
static void ttygetsize()
{
	int	lines;
	int	cols;

	/* get the window size, one way or another. */
	lines = cols = 0;
	if (!ttysize(&lines, &cols) || lines < 2 || cols < 30)
	{
		lines = tgetnum("li");
		if (lines <= 0) lines = 24;
		cols = tgetnum("co");
		if (cols <= 0) cols = 80;
	}

	/* did we get a realistic value? */
	if (lines >= 2 && cols >= 30)
	{
		o_ttyrows = lines;
		o_ttycolumns = cols;
	}
}


/* end of low-level terminal control */
/*----------------------------------------------------------------------------*/
/* start of termcap operations */

static char	*capbuf;/* used for allocation space for termcap strings */

/* This function fetches an optional string from termcap */
static void mayhave(T, s)
	char	**T;	/* where to store the returned pointer */
	char	*s;	/* name of the capability */
{
	char	*val;

	val = tgetstr(s, &capbuf);
	if (val)
	{
		*T = val;
	}
}


/* This function fetches a required string from termcap */
static void musthave(T, s)
	char	**T;	/* where to store the returned pointer */
	char	*s;	/* name of the capability */
{
	mayhave(T, s);
	if (!*T)
	{
		msg(MSG_FATAL, "[s]termcap needs $1", s);
	}
}


/* This function fetches a pair of strings from termcap.  If one of them is
 * missing, then the other one is ignored.
 */
static void pair(T, U, sT, sU)
	char	**T;	/* where to store the first pointer */
	char	**U;	/* where to store the second pointer */
	char	*sT;	/* name of the first capability */
	char	*sU;	/* name of the second capability */
{
	mayhave(T, sT);
	mayhave(U, sU);
	if (!*T || !*U)
	{
		*T = *U = "";
	}
}

/* This function gets a single termcap string in a special static buffer.
 * Returns the string if successful, or NULL if unsuccessful.
 */
static char *manynames(names)
	char	*names; /* possible names (each pair of chars is one name) */
{
	char	name[3];
	int	i;
	char	*value;

	/* for each possible name... */
	for (i = 0; names[i]; i += 2)
	{
		/* see if the termcap string can be found */
		name[0] = names[i];
		name[1] = names[i + 1];
		name[2] = '\0';
		value = tgetstr(name, &capbuf);
		if (value)
		{
			/* found! */
			return value;
		}
	}
	return NULL;
}


/* get termcap values */
static void starttcap()
{
	static char	cbmem[800];
	char		*str;
	int		i;

	/* make sure TERM variable is set */
	o_term = toCHAR(ttytermtype());
	if (!o_term)
	{
		o_term = toCHAR("unknown");
	}

	/* allocate memory for capbuf */
	capbuf = cbmem;

	/* get the termcap entry */
	switch (tgetent(ttybuf, tochar8(o_term)))
	{
	  case -1:	msg(MSG_FATAL, "termcap database unreadable");
	  case 0:	msg(MSG_FATAL, "[S]TERM=$1 unknown", o_term);
	}

	/* get strings */
	musthave(&UP, "up");
	BC = "\b";
	mayhave(&BC, "bc");
	mayhave(&VB, "vb");
	musthave(&CM, "cm");
	mayhave(&TI, "ti");
	mayhave(&TE, "te");
	pair(&KS, &KE, "ks", "ke");	/* keypad enable/disable */
	if (tgetnum("sg") <= 0)
	{
		pair(&SO, &SE, "so", "se");
	}
	if (tgetnum("ug") <= 0)
	{
		pair(&US, &UE, "us", "ue");
		pair(&MD, &ME, "md", "me");
		if (ME)
		{
			mayhave(&MH, "mh");
		}
	}
	mayhave(&ICmany, "IC");
	mayhave(&IC, "ic");
	mayhave(&DCmany, "DC");
	mayhave(&DC, "dc");
	mayhave(&ALmany, "AL");
	mayhave(&AL, "al");
	mayhave(&DLmany, "DL");
	mayhave(&DL, "dl");
	musthave(&CE, "ce");
	mayhave(&SRev, "sr");

	/* cursor shapes */
	CQ = tgetstr("cQ", &capbuf);
	if (CQ)
	{
		CX = tgetstr("cX", &capbuf);
		if (!CX) CX = CQ;
		CV = tgetstr("cV", &capbuf);
		if (!CV) CV = CQ;
		CI = tgetstr("cI", &capbuf);
		if (!CI) CI = CQ;
		CR = tgetstr("cR", &capbuf);
		if (!CR) CR = CQ;
	}
	else
	{
		CQ = CV = "";
		pair(&CQ, &CV, "ve", "vs");
		CX = CI = CQ;
		CR = CV;
	}

	/* graphic characters */
	str = tgetstr("ac", &capbuf);
	if (str)
	{
		/* apparently we're using the :as=:ae=:ac=: style */
		pair(&GS, &GE, "as", "ae");
		for (i = 0; str[i] && str[i + 1]; i += 2)
		{
			switch (str[i])
			{
			  case 'q':	GC_H = str[i + 1];	break;
			  case 'x':	GC_V = str[i + 1];	break;
			  case 'm':	GC_1 = str[i + 1];	break;
			  case 'v':	GC_2 = str[i + 1];	break;
			  case 'j':	GC_3 = str[i + 1];	break;
			  case 't':	GC_4 = str[i + 1];	break;
			  case 'n':	GC_5 = str[i + 1];	break;
			  case 'u':	GC_6 = str[i + 1];	break;
			  case 'l':	GC_7 = str[i + 1];	break;
			  case 'w':	GC_8 = str[i + 1];	break;
			  case 'k':	GC_9 = str[i + 1];	break;
			}
		}
	}
	else
	{
		/* maybe we have :GH=:GV=:... strings? */
		if ((str = tgetstr("GH", &capbuf)) != NULL)	GC_H = *str;
		if ((str = tgetstr("GV", &capbuf)) != NULL)	GC_V = *str;
		if ((str = tgetstr("G3", &capbuf)) != NULL)	GC_1 = *str;
		if ((str = tgetstr("GU", &capbuf)) != NULL)	GC_2 = *str;
		if ((str = tgetstr("G4", &capbuf)) != NULL)	GC_3 = *str;
		if ((str = tgetstr("GR", &capbuf)) != NULL)	GC_4 = *str;
		if ((str = tgetstr("GC", &capbuf)) != NULL)	GC_5 = *str;
		if ((str = tgetstr("GL", &capbuf)) != NULL)	GC_6 = *str;
		if ((str = tgetstr("G2", &capbuf)) != NULL)	GC_7 = *str;
		if ((str = tgetstr("GD", &capbuf)) != NULL)	GC_8 = *str;
		if ((str = tgetstr("G1", &capbuf)) != NULL)	GC_9 = *str;
		pair(&GS, &GE, "GS", "GE");

		/* if we have no :GS=:GE=: strings, then set MSB of chars */
		if (!GS || !*GS)
		{
			if (GC_H) GC_H |= 0x80;
			if (GC_V) GC_V |= 0x80;
			if (GC_1) GC_1 |= 0x80;
			if (GC_2) GC_2 |= 0x80;
			if (GC_3) GC_3 |= 0x80;
			if (GC_4) GC_4 |= 0x80;
			if (GC_5) GC_5 |= 0x80;
			if (GC_6) GC_6 |= 0x80;
			if (GC_7) GC_7 |= 0x80;
			if (GC_8) GC_8 |= 0x80;
			if (GC_9) GC_9 |= 0x80;
		}
	}

	/* key strings */
	for (i = 0; i < QTY(keys); i++)
	{
		keys[i].rawin = manynames(keys[i].capnames);
	}

	/* other termcap stuff */
	AM = (BOOLEAN)(tgetflag("am") && !tgetflag("xn"));
	PT = (BOOLEAN)tgetflag("pt");

	/* change the terminal mode to cbreak/noecho */
	ttyinit();
	ttyresume(True);

	/* try to get true screen size, from the operating system */
	ttygetsize();
}

static void endtcap()
{
	/* change the terminal mode back the way it was */
	ttysuspend();
}


/* end of termcap operations */
/*----------------------------------------------------------------------------*/
/* start of GUI functions */


static int	afterprg;	/* expose windows (after running prg) */
static int	afterscrl;	/* number of status lines (after running prg) */
static BOOLEAN	fgcolored;	/* have foreground colors been set? */
static BOOLEAN	bgcolored;	/* have background colors been set? */
static int	physx, physy;	/* physical cursor position */
static TWIN	*twins;		/* list of windows */
static TWIN	*current;	/* window with keyboard focus */
static TWIN	defcolors;	/* default color strings */
static int	nwindows;	/* number of windows allocated */

/*----------------------------------------------------------------------------*/
/* The following are used for replacing character attributes with colors.
 * Currently only "ansi" colors are supported.  This is the color scheme used
 * by the ANSI.SYS MS-DOS driver, and by most other PC operating systems.
 */

static struct
{
	char	*name;	/* name of the color */
	int	ansi;	/* ANSI.SYS color; if >10, then set "bold" attribute */
} colors[] =
{
	{"black",	0},
	{"red",		1},
	{"green",	2},
	{"brown",	3},
	{"blue",	4},
	{"magenta",	5},
	{"cyan",	6},
	{"white",	7},
	{"gray",	10},
	{"grey",	10},
	{"yellow",	13},
	{(char *)0}
};

static BOOLEAN ansi_color(tw, font, fgname, bgname)
	TWIN	*tw;	/* window whose colors are to be set */
	_char_	font;	/* font code, one of n/b/i/u/e/o/N */
	char	*fgname;/* foreground color */
	char	*bgname;/* background color name, or NULL */
{
	BOOLEAN	bright;	/* set the brightness bit? */
	int	fg, bg;
	char	*build;
	static	int	normbg = -1;

	/* see if we're supposed to set the brightness bit */
	bright = False;
	if (!strncmp(fgname, "light", 5)) bright = True, fgname += 5;
	if (!strncmp(fgname, "lt", 2)) bright = True, fgname += 2;
	if (!strncmp(fgname, "bright", 6)) bright = True, fgname += 6;

	/* skip leading garbage characters */
	while (*fgname && !isalpha(*fgname))
	{
		fgname++;
	}

	/* try to find the foreground color */
	for (fg = 0; colors[fg].name && strcmp(colors[fg].name, fgname); fg++)
	{
	}
	if (!colors[fg].name)
	{
		msg(MSG_ERROR, "[s]invalid color $1", fgname);
		return False;
	}
	fg = colors[fg].ansi;

	/* try to find the background color, if given */
	if (bgname && *bgname)
	{
		for (bg = 0; colors[bg].name && strcmp(colors[bg].name, bgname); bg++)
		{
		}
		if (!colors[bg].name)
		{
			msg(MSG_ERROR, "[s]invalid color $1", bgname);
			return False;
		}
		bg = colors[bg].ansi;
	}
	else /* no background specified */
	{
		/* use "normal" background color, if defined */
		bg = normbg;
	}

	/* if foreground color implies "bold", remember that! */
	if (fg >= 10)
	{
		bright = True;
		fg -= 10;
	}

	/* background can't be bold */
	if (bg >= 0)
	{
		if (colors[bg].ansi >= 10)
		{
			msg(MSG_ERROR, "background can't be bright");
			return False;
		}
	}

	/* build the string */
	switch (font)
	{
	  case 'n':	build = tw->startnormal;	break;
	  case 'f':	build = tw->startfixed;		break;
	  case 'b':	build = tw->startbold;		break;
	  case 'e':	build = tw->startemph;		break;
	  case 'i':	build = tw->startitalic;	break;
	  case 'u':	build = tw->startunderline;	break;
	  default:	build = tw->starthilite;	break;
	}
	if (bg >= 0)
	{
		sprintf(build, "\033[0;%dm\033[%s%dm",
			40 + bg,		/* background color */
			bright ? "1;" : "",	/* brightness */
			30 + fg);		/* foreground color */
		if (o_ttyunderline && US && font == 'u')
			strcat(build, US);
	}
	else /* no background specified; only affect foreground */
	{
		sprintf(build, "\033[0;%s%dm",
			bright ? "1;" : "",	/* brightness */
			30 + fg);		/* foreground color */
		if (o_ttyunderline && US && font == 'u')
			strcat(build, US);
#if 0
		if (SO && build == tw->starthilite)
			strcat(build, SO);
#endif
	}

	/* if 'n' font, copy startnormal into endXXX */
	if (font == 'n')
	{
		/* but first! if this the first color setting, then choose
		 * defaults for all other attributes.
		 */
		if (!fgcolored || (bg >= 0 && !bgcolored))
		{
			/* If we have a background color... */
			if (bg >= 0)
			{
				/* if normal is bright, then bold must be bright white */
				if (bright)
				{
					sprintf(tw->startbold, "\033[0;%dm\033[1;37m", 40 + bg);
				}
				else
				{
					sprintf(tw->startbold, "\033[0;%dm\033[1;%dm", 40 + fg, 30 + bg);
				}

				/* emphasized is same as bold */
				strcpy(tw->startemph, tw->startbold);

				/* italic is a dim version of normal */
				sprintf(tw->startitalic, "\033[0;%d;%dm", 30 + fg, 40 + bg);

				/* underline is dim underlined version of normal */
				sprintf(tw->startunderline, "\033[0;%d;%dm%s",
					30 + fg, 40 + bg, (o_ttyunderline && US) ? US : "");

				/* reverse video swaps foreground & background; always dim */
				sprintf(tw->starthilite, "\033[0;%d;%dm", 30 + bg, 40 + fg);

				/* other font is same as normal */
				strcpy(tw->startfixed, tw->startnormal);
			}
			else /* no background color */
			{
				/* if normal is bright, then bold must be bright white */
				if (bright)
				{
					strcpy(tw->startbold, "\033[0;1;37m");
				}
				else
				{
					sprintf(tw->startbold, "\033[0;1;%dm", 30 + fg);
				}

				/* emphasized is same as bold */
				strcpy(tw->startemph, tw->startbold);

				/* italic is a dim version of normal */
				sprintf(tw->startitalic, "\033[0;%dm", 30 + fg);

				/* underline is dim underlined version of normal */
				sprintf(tw->startunderline, "\033[0;%dm%s",
					30 + fg, (o_ttyunderline && US) ? US : "");

				/* reverse video is reverse+normal colors */
				sprintf(tw->starthilite, "\033[0;%d;7m", 30 + fg);

				/* other font is same as normal */
				strcpy(tw->startfixed, tw->startnormal);
			}
		}

		/* remember the new background color (if any) */
		normbg = bg;

		/* copy the normal string to each font's endXXX string */
		strcpy(tw->endbold, tw->startnormal);
		strcpy(tw->endemph, tw->startnormal);
		strcpy(tw->enditalic, tw->startnormal);
		*tw->endunderline = '\0';
			if (UE) strcpy(tw->endunderline, UE);
			strcat(tw->endunderline, tw->startnormal);
#if 0
		*tw->endhilite = '\0';
			if (SE && bg < 0) strcpy(tw->endhilite, SE);
			strcat(tw->endhilite, tw->startnormal);
#else
		strcpy(tw->endhilite, tw->startnormal);
#endif
		strcpy(tw->endfixed, tw->startnormal);
	}

	/* success! */
	return True;
}

/* This is an internal function which moves the physical cursor to the logical
 * position of the cursor in a given window, if it isn't there already.
 */
static void movecurs(tw)
	TWIN	*tw;	/* window whose cursor is to be moved */
{
	int	y = tw->pos + tw->cursy;
	int	i;

	/* maybe we don't need to move at all? */
	if ((afterprg > 0 && y <= o_ttyrows - afterscrl)
		|| (y == physy && tw->cursx == physx))
	{
		/* already there */
		return;
	}

	/* Try some simple alternatives to the CM string */
	if (y >= physy && y - physy < gui->movecost && (tw->cursx == 0 || tw->cursx == physx))
	{
		/* output a bunch of newlines, and maybe a carriage return */
		for (i = y - physy; i > 0; i--)
			ttych('\n');
		if (tw->cursx != physx)
			ttych('\r');
	}
	else if (y == physy && tw->cursx < physx && physx - tw->cursx < gui->movecost)
	{
		/* output a bunch of backspaces */
		for (i = physx - tw->cursx; i > 0; i--)
			tputs(BC, 0, ttych);
	}
	/* many other special cases could be handled here */
	else
	{
#if 1
		/* revert to the normal font */
		revert(tw);
#endif

		/* move it the hard way */
		tputs(tgoto(CM, tw->cursx, y), 1, ttych);
	}

	/* done! */
	physx = tw->cursx;
	physy = tw->cursy + tw->pos;
}

/* clear to end of line */
static BOOLEAN clrtoeol(gw)
	GUIWIN	*gw;	/* window whose row is to be cleared */
{
	TWIN	*tw = (TWIN *)gw;

	/* after running a program, disable the :ce: string for a while. */
	if (afterprg)
		return True;

	/* if we're on the bottom row of a window which doesn't end at the
	 * bottom of the screen, then fail.  This will cause elvis to output
	 * a bunch of spaces instead.  The draw() function will convert those
	 * spaces to underscore characters so the window has a border.
	 */
	if (tw->cursy == tw->height - 1 && tw->pos + tw->height != o_ttyrows)
	{
		return False;
	}

	/* revert to the normal font */
	revert(tw);

	/* move the physical cursor to where the window thinks it should be */
	movecurs(tw);

	/* output the clear-to-eol string */
	tputs(CE, (int)(o_ttycolumns - tw->cursx), ttych);

	return True;
}

/* insert or delete columns */
static BOOLEAN shift(gw, qty, rows)
	GUIWIN	*gw;	/* window to be shifted */
	int	qty;	/* columns to insert (may be negative to delete) */
	int	rows;	/* number of rows affected (always 1 for this GUI) */
{
	/* revert to the normal font */
	revert((TWIN *)gw);

	/* move the physical cursor to where this window thinks it is */
	movecurs((TWIN *)gw);

	if (qty > 0)
	{
		/* can we do many at once? */
		if (qty > 1 && ICmany)
		{
			tputs(tgoto(ICmany, qty, qty), 1, ttych);
		}
		else if (IC)
		{
			for (; qty > 0; qty--)
			{
				tputs(IC, 1, ttych);
			}
		}
		else
		{
			/* don't know how to insert */
			return False;
		}
	}
	else
	{
		/* take the absolute value of qty */
		qty = -qty;

		/* can we do many deletions at once? */
		if (qty > 1 && DCmany)
		{
			tputs(tgoto(DCmany, qty, qty), 1, ttych);
		}
		else if (DC)
		{
			for (; qty > 0; qty--)
			{
				tputs(DC, 1, ttych);
			}
		}
		else
		{
			/* don't know how to delete */
			return False;
		}
	}
	return True;
}

/* insert or delete rows.  qty is positive to insert, negative to delete */
static BOOLEAN scroll(gw, qty, notlast)
	GUIWIN	*gw;	/* window to be scrolled */
	int	qty;	/* rows to insert (may be nagative to delete) */
	BOOLEAN notlast;/* if True, then leave last row unchanged */
{
	TWIN	*tw = (TWIN *)gw;
	char	*op;

	/* Mentally adjust the number of rows used for messages.  This is only
	 * significant immediately after running an external program, and is
	 * used for hiding any premature attempts to redraw the window's text
	 * but still show the window's messages.
	 */
	afterscrl -= qty;

	/* If this window isn't the only window, then fail.
	 * Later, this function may be smart enough to use scrolling regions,
	 * or do the idlok() kind of thing, but not yet.
	 */
	if (twins->next)
	{
		return False;
	}

	/* revert to the normal font */
	revert(tw);

	/* move the physical cursor to where the window thinks it should be */
	movecurs(tw);

	if (qty > 0)
	{
		/* we'll be inserting.  Can we do it all at once? */
		if (ALmany && !(AL && qty == 1))
		{
			/* all at once */
			tputs(tgoto(ALmany, qty, qty), tw->height - tw->cursy, ttych);
		}
		else
		{
			/* try to use SRev */
			op = (tw->cursy == 0 && tw->pos == 0 && SRev) ? SRev : AL;

			/* if we don't know how to do this, we're screwed */
			if (!op)
			{
				return False;
			}

			/* a bunch of little insertions */
			while (qty > 0)
			{
				tputs(op, tw->height - tw->cursy, ttych);
				qty--;
			}
		}
	}
	else
	{
		/* take the absolute value of qty */
		qty = -qty;

		/* we'll be deleting.  Can we do it all at once? */
		if (DLmany && !(DL && qty == 1))
		{
			/* all at once */
			tputs(tgoto(DLmany, qty, qty), tw->height - tw->cursy, ttych);
		}
		else
		{
			/* try to use newline */
			op = DL; /* but don't try very hard, for now! */

			/* if we don't know how to do this, we're screwed */
			if (!op)
			{
				return False;
			}

			/* a bunch of little deletions */
			while (qty > 0)
			{
				tputs(op, tw->height - tw->cursy, ttych);
				qty--;
			}
		}
	}
	return True;
}

/* Forget where the cursor is, and which mode we're in */
static void reset P_((void))
{
	physx = physy = 9999;
	revert(&defcolors);
}


/* Flush any changes out to the display */
static void flush P_((void))
{
	if (current)
	{
		movecurs(current);
	}
	ttyflush();
}

/* Set a window's cursor position. */
static void moveto(gw, column, row)
	GUIWIN	*gw;	/* window whose cursor is to be moved */
	int	column;	/* new column of cursor */
	int	row;	/* new row of cursor */
{
	((TWIN *)gw)->cursx = column;
	((TWIN *)gw)->cursy = row;
}


/* put graphic characters.  This function is called only from draw() */
static void drawgraphic(gw, font, text, len)
	GUIWIN	*gw;	/* window where text should be drawn */
	_char_	font;	/* font to use for drawing this text - 'g' or 'G' */
	CHAR	*text;	/* plain chars to be mapped to graphic chars */
	int	len;	/* length of text */
{
	TWIN	*tw = (TWIN *)gw;
	int	i;
	BOOLEAN	graf;
	char	gc;

	/* if this is supposed to be hilighted, and we aren't already in
	 * standout mode, then switch to standout mode now.
	 */
	if (font == 'G' && startfont != tw->starthilite)
	{
		/* end the previous */
		if (*endfont)
			tputs(endfont, 1, ttych); 
			
		/* start the new one */
		if (*tw->starthilite)
			tputs(tw->starthilite, 1, ttych);

		/* remember the font */
		startfont = tw->starthilite;
		endfont = tw->endhilite;
	}
	else if (font == 'g' && *endfont)
	{
		/* end the previous */
		tputs(endfont, 1, ttych);
		startfont = endfont = "";
	}

	/* draw each character */
	for (graf = False, i = 0; i < len; i++)
	{
		/* try to convert plain character to graphic character */
		switch (text[i])
		{
		  case '-':	gc = GC_H;	break;
		  case '|':	gc = GC_V;	break;
		  case '1':	gc = GC_1;	break;
		  case '2':	gc = GC_2;	break;
		  case '3':	gc = GC_3;	break;
		  case '4':	gc = GC_4;	break;
		  case '5':	gc = GC_5;	break;
		  case '6':	gc = GC_6;	break;
		  case '7':	gc = GC_7;	break;
		  case '8':	gc = GC_8;	break;
		  case '9':	gc = GC_9;	break;
		  default:	gc = 0;
		}
		
		/* did we get a graphic character? */
		if (gc)
		{
			/* output the graphic character in graphic mode */
			if (!graf && *GS)
			{
				tputs(GS, 1, ttych);
				graf = True;
			}
			ttych(gc);
		}
		else
		{
			/* output elvis' plain character in text mode */
			if (graf)
			{
				tputs(GE, 1, ttych);
				graf = False;
			}
			if (isdigit(text[i]))
				ttych('+');
			else
				ttych(text[i]);
		}
	}

	/* if still in graphic mode, then revert to text mode now */
	if (graf && GE)
	{
		tputs(GE, 1, ttych);
	}

	/* drawing the characters has the side-effect of moving the cursor */
	tw->cursx += len;
	physx += len;
	if (physx == o_ttycolumns && AM)
	{
		physx = 0;
		physy++;
	}
}


/* put characters: first move, then set attribute, then execute char.
 */
static void draw(gw, font, text, len)
	GUIWIN	*gw;	/* window where text should be drawn */
	_char_	font;	/* font to use for drawing this text */
	CHAR	*text;	/* text to draw */
	int	len;	/* length of text */
{
	TWIN	*tw = (GUIWIN *)gw;
	char	*startf, *endf; /* font control strings */
	int	i;
#ifndef NDEBUG
	TWIN	*scan;

	for (scan = twins; scan != tw && scan; scan = scan->next)
	{
	}
	assert(scan);
#endif

	/* After a program, don't output any text except messages for a while.
	 * This is mostly an optimization; the window is about to be redrawn
	 * from scratch anyway.  But it also prevents the screen from doing
	 * strange, unexpected things.
	 */
	if (afterprg > 0)
	{
#if 0
fprintf(stderr, "draw(\"%.*s\"), tw->cursy=%d, tw->height=%d, physy=%d, afterscrl=%d\n", len, tochar8(text), tw->cursy, tw->height, physy, afterscrl);
#endif
		if (tw->cursy < tw->height - afterscrl - 1)
			return;
		else if (tw->cursx > 0)
		{
			ttych('\r');
			ttych('\n');
			ttyflush();
			font = 'n';
		}
	}

	/* if this terminal has :am: automargins (without :xm:), then we
	 * must never draw a character in the last column of the last row.
	 */
	if (AM
		&& tw->pos + tw->cursy == o_ttyrows - 1
		&& tw->cursx + len >= o_ttycolumns)
	{
		len = o_ttycolumns - 1 - tw->cursx;
		if (len <= 0)
		{
			return;
		}
	}

	/* move the cursor to where this window thinks it is */
	movecurs(tw);

	/* if graphic characters, then handle specially */
	if (font == 'g' || font == 'G')
	{
		drawgraphic(gw, font, text, len);
		return;
	}

	/* If we're on the bottom row of a window (except the last window)
	 * then any normal characters should be underlined.  This will give
	 * us the effect of a window border.
	 */
	if (tw->cursy == tw->height - 1 && physy < o_ttyrows - 1 && !isupper(font))
	{
		font = 'u';
	}

	/* find the font strings */
	if (isupper(font))
	{
		startf = tw->starthilite;
		endf = tw->endhilite;
	}
	else
	{
		switch (font)
		{
		  case 'b':	startf = tw->startbold; endf = tw->endbold; break;
		  case 'e':	startf = tw->startemph; endf = tw->endemph; break;
		  case 'i':	startf = tw->startitalic; endf = tw->enditalic; break;
		  case 'u':	startf = tw->startunderline; endf = tw->endunderline; break;
		  case 'f':	startf = tw->startfixed; endf = tw->endfixed; break;
			/* 'g' is handled separately, above */
		  case 'p':	startf = tw->starthilite; endf = tw->endhilite; break;
		  default:	startf = endf = "";
		}
	}

	/* if font is different, then change it */
	if ((startf != startfont && (*startf != '\0' || *startfont != '\0'))
	 || (endf != endfont && (*endf != '\0' || *endfont != '\0')))
	{
		/* end the previous */
		if (*endfont)
			tputs(endfont, 1, ttych); 

		/* start the new one */
		if (*startf)
			tputs(startf, 1, ttych);

		/* remember the font */
		startfont = startf;
		startfont = endfont = endf;
	}

	/* draw each character.  If this is the bottom row of any window except
	 * the bottom window, then also replace any blanks with '_' characters.
	 * This will provide a window border in case the terminal can't do
	 * real underlining.
	 */
	if (tw->cursy == tw->height - 1 && physy < o_ttyrows - 1 && !isupper(font))
	{
		for (i = 0; i < len; i++)
		{
			ttych(text[i] == ' ' ? '_' : text[i]);
		}
	}
	else /* normal row */
	{
		for (i = 0; i < len; i++)
		{
			ttych(text[i]);
		}
	}

	/* drawing the characters has the side-effect of moving the cursor */
	tw->cursx += len;
	physx += len;
	if (physx == o_ttycolumns && AM)
	{
		physx = 0;
		physy++;
	}
}



/* return True if termcap is available. */
static int test P_((void))
{
	return ttytermtype() ? 1 : 0;
}


/* initialize the PC BIOS interface. */
static int init(argc, argv)
	int	argc;	/* number of command-line arguments */
	char	**argv;	/* values of command-line arguments */
{
	int	i;

	/* initialize the termcap stuff */
	starttcap();

	/* pretend the cursor is in an impossible place, so we're guaranteed
	 * to move it on the first moveto() or draw() call.
	 */
	physx = physy = -100;

	/* map the arrow keys */
	for (i = 0; i < QTY(keys); i++)
	{
		if (keys[i].cooked && keys[i].rawin)
		{
			mapinsert(toCHAR(keys[i].rawin), (int)strlen(keys[i].rawin),
				toCHAR(keys[i].cooked), (int)strlen(keys[i].cooked),
				toCHAR(keys[i].label),
				keys[i].flags);
		}
	}

	/* add the global options to the list known to :set */
	o_ttyunderline = True;
	optinsert("tcap", QTY(goptdesc), goptdesc, &goptvals.term);

	return argc;
}

/* change the shape of the cursor */
static void cursorshape(shape)
	ELVCURSOR	shape;	/* desired cursor shape */
{
	char		*esc;		/* desired shape */
 static char		*prevesc;	/* current shape */

	/* find the desired shape's escape sequence */
	switch (shape)
	{
	  case CURSOR_INSERT:	esc = CI;	break;
	  case CURSOR_REPLACE:	esc = CR;	break;
	  case CURSOR_COMMAND:	esc = CV;	break;
	  case CURSOR_QUOTE:	esc = CX;	break;
	  default:		esc = CQ;	break;
	}

	/* output it, if changed and non-empty */
	if (esc != prevesc && *esc)
	{
		tputs(esc, 1, ttych);
		prevesc = esc;
	}
}

/* Repeatedly get events (keystrokes), and call elvis' event functions */
static void loop P_((void))
{
	char	buf[20];
	int	len;
	int	timeout = 0;
	MAPSTATE mst = MAP_CLEAR;
	TWIN	*scan;

	while (twins)
	{
		/* reset the ttycaught bitmap */
		ttycaught = 0;

		/* if no window is current, then make the newest current */
		if (!current)
		{
			current = twins;
		}

		/* redraw the window(s) */
		{
			/* redraw each window; the current one last */
			for (scan = twins; scan; scan = scan->next)
			{ 
				if (scan != current)
				{
					scan->shape = eventdraw((GUIWIN *)scan);
				}
			}
			current->shape = eventdraw((GUIWIN *)current);
			movecurs(current);

#if 1
			/* make the cursor be this window's shape */
			cursorshape(current->shape);
#endif
		}

		/* choose a timeout value */
		switch (mst)
		{
		  case MAP_CLEAR:	timeout = 0;		break;
		  case MAP_USER:	timeout = o_usertime;	break;
		  case MAP_KEY:		timeout = o_keytime;	break;
		}

		/* read events */
		ttyflush();
		len = ttyread(buf, sizeof buf, timeout);

		/* process keystroke data */
		if (len == -2)
		{
			/* ttyread() itself did something.  We don't need to
			 * do anything except the usual screen updates.
			 */
		}
		else if (len == -1)
		{
			/* Maybe the screen was resized?  Get new size */
			ttygetsize();

			/* Resize the windows to match the new screen.  The
			 * easiest way to do this is to "change" the size of the
			 * current window to its original size and force the
			 * other windows to compensate.  If there is only one
			 * window, then should be resized to the screen size.
			 */
			chgsize(current, twins->next ? current->height : (int)o_ttyrows, True);
		}
		else
		{
			mst = eventkeys((GUIWIN *)current, toCHAR(buf), len);

			/* if first keystroke after running an external
			 * program, then we need to expose every window.
			 */
			if (afterprg == 1)
			{
				/* reset the flag BEFORE exposing windows,
				 * or else the eventexpose() won't work right.
				 */
				afterprg = 0;
				ttyresume(True);
				for (scan = twins; scan; scan = scan->next)
				{ 
					eventexpose((GUIWIN *)scan, 0, 0,
					    scan->height - 1, (int)(o_ttycolumns - 1));
				}
			}
			else if (afterprg == 2)
			{
				/* it became 2 while processing the earlier
				 * keystrokes.  Set it to 1 now, so we'll
				 * read one more keystroke before exposing
				 * all the windows.
				 */
				afterprg = 1;
			}
		}
	}
}

/* shut down the termcap interface */
static void term P_((void))
{
	cursorshape(CURSOR_NONE);
	endtcap();
	ttyflush();
}


/* This draws a bunch of underscores on the physical screen on the bottom
 * row of a window, if that window doesn't end at the bottom of the screen.
 * This should be called after a window is resized or moved.
 */
static void drawborder(tw)
	TWIN	*tw;	/* window whose border needs to be redrawn */
{
	int	col;

	/* if this window ends at the bottom of the screen, then do nothing */
	if (tw->pos + tw->height == o_ttyrows)
	{
		return;
	}

	/* move the physical cursor to the bottom of the window */
	tw->cursx = 0;
	tw->cursy = tw->height - 1;
	movecurs(tw);
	if (startfont != tw->startunderline)
	{
		if (*endfont)
		{
			tputs(endfont, 1, ttych);
		}
		startfont = tw->startunderline;
		endfont = tw->endunderline;
		tputs(startfont, 1, ttych);
	}
	for (col = 0; col < o_ttycolumns; col++)
	{
		ttych('_');
	}

	/* figure out where the physical cursor would be after that */
	if (AM)
	{
		physy++;
	}
	else
	{
		physx = o_ttycolumns - 1;
	}
}

/* This function changes the height of a given window.  The total heights of
 * all windows must be o_ttyrows and the minimum height of each window is
 * MINHEIGHT.
 */
static void chgsize(tw, newheight, winch)
	TWIN	*tw;		/* window to be resized */
	int	newheight;	/* desired height of window */
	BOOLEAN	winch;		/* Did the whole screen change size? */
{
	TWIN	*scan;
	int	pos;
	int	otherheight;
	int	oldheight;
	int	toosmall;

	/* if the current window can't be as large as requested, then reduce
	 * the requested size.
	 */
	if ((nwindows - 1) * MINHEIGHT + newheight > o_ttyrows)
	{
		newheight = o_ttyrows - (nwindows - 1) * MINHEIGHT;
	}

	/* if window is already the requested height, we're done */
	if (tw->height == newheight && !winch)
	{
		return;
	}

	/* Set the size of the current window.  Also, adjust the sizes of other
	 * windows, and maybe their positions.  If any window other than the
	 * requested one is moved, expose it.  If any window other than the
	 * requested one is resized, then resize it.
	 */
	toosmall = 0;
	do
	{
		for (oldheight = tw->height, pos = 0, scan = twins;
		     scan;
		     pos += scan->newheight, scan = scan->next)
		{
			/* the requested window? */
			if (scan == tw)
			{
				/* yes, set it */
				scan->newpos = pos;
				scan->newheight = newheight;
			}
			else
			{
				/* no, some other window */

				/* compute the size that this window should be */
				if (!scan->next)
				{
					scan->newheight = o_ttyrows - pos;
					toosmall = MINHEIGHT - scan->newheight;
				}
				else if (scan->next == tw && !scan->next->next)
				{
					scan->newheight = o_ttyrows - pos - newheight;
					toosmall = MINHEIGHT - scan->newheight;
				}
				else
				{
					if (winch)
						otherheight = MINHEIGHT;
					else
						otherheight = scan->height * (o_ttyrows - newheight) / (o_ttyrows - oldheight);
					if (otherheight < MINHEIGHT)
					{
						scan->newheight = MINHEIGHT;
					}
					else
					{
						scan->newheight = otherheight - toosmall;
						if (scan->newheight < MINHEIGHT)
						{
							scan->newheight = MINHEIGHT;
						}
						toosmall -= otherheight - scan->newheight;
					}
				}
				scan->newpos = pos;
			}
		}
	} while (toosmall > 0);

	/* resize/redraw the windows, as necessary */
	for (scan = twins; scan; scan = scan->next)
	{
		/* set the size & position of this window.  If its
		 * size has changed then resize the window; else if
		 * its position has changed resize the window.
		 */
		if (scan == tw && !winch)
		{
			/* just remember new stats.  Calling function will
			 * call eventredraw() or eventdraw(), as necessary.
			 */
			scan->height = scan->newheight;
			scan->pos = scan->newpos;
		}
		else if (scan->newheight != scan->height || winch)
		{
			scan->height = scan->newheight;
			scan->pos = scan->newpos;
			if (scan->pos + scan->height < o_ttyrows)
			{
				drawborder(scan);
			}
			else
			{
				/* draw the border the hard way: erase last row */
				if (*endfont)
				{
					tputs(endfont, 1, ttych);
					startfont = endfont = "";
				}
				physy = o_ttyrows - 1;
				physx = 0;
				tputs(tgoto(CM, physx, physy), 1, ttych);
				tputs(CE, 1, ttych);
			}
			eventresize((GUIWIN *)scan, scan->height, (int)o_ttycolumns);
		}
		else if (scan->newpos != scan->pos)
		{
			scan->pos = scan->newpos;
			drawborder(scan);
			eventexpose((GUIWIN *)scan, 0, 0, scan->height - 1, (int)(o_ttycolumns - 1));
		}
	}
}


/* This function creates a window */
static BOOLEAN creategw(name, attributes)
	char	*name;		/* name of new window's buffer */
	char	*attributes;	/* other window parameters, if any */
{
	TWIN	*newp;

	/* if we don't have room for any more windows, then fail */
	if (o_ttyrows / (nwindows + 1) < MINHEIGHT)
	{
		return False;
	}

	/* create a window */
	newp = safealloc(1, sizeof(TWIN));

	/* initialize the window */
	if (twins)
	{
		newp->height = 0;
		newp->pos = o_ttyrows;
	}
	else
	{
		newp->height = o_ttyrows;
		newp->pos = 0;
	}
	newp->cursx = newp->cursy = 0;
	newp->shape = CURSOR_NONE;
	if (fgcolored)
	{
		/* copy font-switch strings from current window */
		strcpy(newp->startnormal, defcolors.startnormal);
		strcpy(newp->startbold, defcolors.startbold);
		strcpy(newp->endbold, defcolors.endbold);
		strcpy(newp->startitalic, defcolors.startitalic);
		strcpy(newp->enditalic, defcolors.enditalic);
		strcpy(newp->startunderline, defcolors.startunderline);
		strcpy(newp->endunderline, defcolors.endunderline);
		strcpy(newp->starthilite, defcolors.starthilite);
		strcpy(newp->endhilite, defcolors.endhilite);
	}
	else
	{
		/* set the initial font-switch strings from termcap strings */
		if (MD) strcpy(newp->startbold, MD), strcpy(newp->endbold, ME);
		if (US) strcpy(newp->startunderline, US), strcpy(newp->endunderline, UE);
		if (MH) strcpy(newp->startitalic, MH), strcpy(newp->enditalic, ME);
		if (SO) strcpy(newp->starthilite, SO), strcpy(newp->endhilite, SE);
	}
	strcpy(newp->startfixed, defcolors.startfixed);
	strcpy(newp->endfixed, defcolors.endfixed);
	strcpy(newp->startemph, defcolors.startemph);
	strcpy(newp->endemph, defcolors.endemph);

	/* insert the new window into the list of windows */
	newp->next = twins;
	twins = newp;
	nwindows++;

	/* adjust the heights of the other windows to make room for this one */
	chgsize(newp, (int)(o_ttyrows / nwindows), False);
	drawborder(newp);

	/* make elvis do its own initialization */
	if (!eventcreate((GUIWIN *)newp, NULL, name, newp->height, (int)o_ttycolumns))
	{
		/* elvis can't make it -- fail */
		safefree(newp);
		return False;
	}

	/* make the new window be the current window */
	current = newp;

	return True;
}


/* This function deletes a window */
static void destroygw(gw, force)
	GUIWIN	*gw;	/* window to be destroyed */
	BOOLEAN	force;	/* if True, try harder */
{
	TWIN	*scan, *lag;

	/* delete the window from the list of windows */
	for (lag = NULL, scan = twins; scan != (TWIN *)gw; lag = scan, scan = scan->next)
	{
	}
	if (lag)
	{
		lag->next = scan->next;
	}
	else
	{
		twins = scan->next;
	}

	/* if it was the current window, it isn't now */
	if (scan == current)
	{
		current = twins;
	}

	/* adjust the sizes of other windows (if any) */
	nwindows--;
	if (nwindows > 0)
	{
		chgsize((TWIN *)gw, 0, False);
	}

	/* If this is the last window, move the cursor to the last line, and
	 * erase it.  If the buffer is going to be written, this is where the
	 * "wrote..." message will appear.
	 */
	if (nwindows == 0)
	{
		revert(NULL);
		tputs(tgoto(CM, 0, (int)(o_ttyrows - 1)), 1, ttych);
		tputs(CE, 1, ttych);
	}

	/* simulate a "destroy" event */
	eventdestroy(gw);

	/* free the storage */
	safefree(gw);
}


/* This function changes window focus */
static BOOLEAN focusgw(gw)
	GUIWIN	*gw;	/* window to be the new "current" window */
{
	current = (TWIN *)gw;
	return True;
}


/* This function handles the visual <Tab> command */
static BOOLEAN tabcmd(gw, key2, count)
	GUIWIN	*gw;	/* window that the command should affect */
	_CHAR_	key2;	/* second key of <Tab> command */
	long	count;	/* argument of the <Tab> command */
{
	TWIN	*tw = (GUIWIN *)gw;
	int	newheight;
	int	oldheight;
	int	oldpos;

	/* if only one window, then we can't change its size */
	if (nwindows == 1)
		return False;

	/* remember the old position */
	newheight = oldheight = tw->height;
	oldpos = tw->pos;

	switch (key2)
	{
	  case '=':
		if (count >= MINHEIGHT)
		{
			newheight = count;
			break;
		}
		/* else fall through... */

	  case '+':
		newheight += (count ? count : 1);
		break;

	  case '-':
		newheight -= (count ? count : 1);
		if (newheight < MINHEIGHT)
		{
			newheight = MINHEIGHT;
		}
		break;

	  case '\\':
		newheight = o_ttyrows; /* will be reduced later */
		break;

	  default:	return False;
	}

	/* try to change the heights of other windows to make this one fit */
	chgsize(tw, newheight, False);
	newheight = tw->height;

	/* resize/expose this window */
	if (newheight != oldheight)
	{
		drawborder(tw);
		eventresize(tw, tw->height, (int)o_ttycolumns);
	}
	else if (tw->pos != oldpos)
	{
		drawborder(tw);
		eventexpose(tw, 0, 0, newheight - 1, (int)(o_ttycolumns - 1));
	}
	return True;
}

/* This function rings the bell */
static void beep(gw)
	GUIWIN	*gw;	/* window that generated the beep */
{
	if (VB)
		tputs(VB, 0, ttych);
	else
		ttych('\007');
}

/* This function converts key labels to raw codes */
static int keylabel(given, givenlen, label, rawptr)
	CHAR	*given;		/* what the user typed in as the key name */
	int	givenlen;	/* length of the "given" string */
	CHAR	**label;	/* standard name for that key */
	CHAR	**rawptr;	/* control code sent by that key */
{
	int	i;

	/* compare the given text to each key's strings */
	for (i = 0; i < QTY(keys); i++)
	{
		/* ignore unsupported keys */
		if (!keys[i].rawin)
			continue;

		/* does given string match key label or raw characters? */
		if ((!strncmp(keys[i].label, tochar8(given), (size_t)givenlen) && !keys[i].label[givenlen])
		 || (!strncmp(keys[i].rawin, tochar8(given), (size_t)givenlen) && !keys[i].rawin[givenlen]))
		{

			/* Set the label and rawptr pointers, return rawlen */
			*label = toCHAR(keys[i].label);
			*rawptr = toCHAR(keys[i].rawin);
			return CHARlen(*rawptr);
		}
	}

	/* We reached the end of the keys[] array without finding a match,
	 * so this given string is not a key.
	 */
	return 0;
}

/* This function defines colors for fonts */
static BOOLEAN color(gw, font, fg, bg)
	GUIWIN	*gw;	/* window whose colors are being set */
	_char_	font;	/* font being changed: n/b/i/u else highlighted */
	CHAR	*fg;	/* name of desired foreground color */
	CHAR	*bg;	/* name of desired background color */
{
	TWIN	*tw = (TWIN *)gw;
	TWIN	*other; /* some other window */
	BOOLEAN	ret;	/* return code -- True if successful */

	/* we must set normal colors first */
	if ((!fgcolored || (bg && !bgcolored)) && font != 'n')
	{
		msg(MSG_ERROR, "must set normal colors first");
		return False;
	}

	/* if no window specified, or this is the first :color command,
	 * then we're setting the default colors.
	 */
	if (!tw || !fgcolored)
	{
		tw = &defcolors;
	}

	/* revert to normal font now; if we wait until after setting colors,
	 * we might not know how to do it anymore!
	 */
	revert(tw);

	/* set the colors */
	ret = ansi_color(tw, font, tochar8(fg), tochar8(bg));

	/* if colors weren't set before, then copy colors to all windows */
	if (!fgcolored || (!bgcolored && bg && *bg))
	{
		for (other = twins; other; other = other->next)
		{
			/* skip the window that we just set */
			if (other == tw)
			{
				continue;
			}

			/* copy the colors */
			strcpy(other->startnormal, tw->startnormal);
			strcpy(other->startfixed, tw->startfixed);
			strcpy(other->endfixed, tw->endfixed);
			strcpy(other->startbold, tw->startbold);
			strcpy(other->endbold, tw->endbold);
			strcpy(other->startemph, tw->startemph);
			strcpy(other->endemph, tw->endemph);
			strcpy(other->startitalic, tw->startitalic);
			strcpy(other->enditalic, tw->enditalic);
			strcpy(other->startunderline, tw->startunderline);
			strcpy(other->endunderline, tw->endunderline);
			strcpy(other->starthilite, tw->starthilite);
			strcpy(other->endhilite, tw->endhilite);
		}

		/* we've set colors now! */
		fgcolored = True;
		if (bg && *bg)
		{
			bgcolored = True;
		}
	}

	/* remember the current window's colors, to use them as the default
	 * for any window that gets created after this.
	 */
	if (tw != &defcolors)
	{
		defcolors = *tw;
	}

	/* We probably need to reset the screen's current attribute */
	revert(tw);

	return ret;
}


static BOOLEAN isfilter;

/* Suspend curses while running an external program */
static BOOLEAN ttyprgopen(command, willwrite, willread)
	char	*command;	/* the shell command to run */
	BOOLEAN	willwrite;	/* redirect stdin from elvis */
	BOOLEAN	willread;	/* redirect stdiout back to elvis */
{
	/* unless both stdin and stdout/stderr are going to be redirected,
	 * move the cursor to the bottom of the screen before running program.
	 */
	isfilter = (BOOLEAN)(willwrite && willread);
	if (!isfilter)
	{
		tputs(tgoto(CM, 0, (int)o_ttyrows - 1), 0, ttych);
#if 0
		if (CE)
			tputs(CE, 0, ttych);
		else
#endif
			ttych('\n');
		reset();
		ttyflush();
	
		/* suspend curses */
		ttysuspend();
	}

	/* try to call the regular prgopen(); if it fails, then clean up */
	if (!prgopen(command, willwrite, willread))
	{
		if (!isfilter)
			ttyresume(True);
		return False;
	}

	return True;
}


/* After running a program, resume curses and redraw all screens */
static int ttyprgclose P_((void))
{
	int	status;

	/* wait for the program to terminate */
	status = prgclose();

	/* resume curses */
	if (!isfilter)
	{
		ttyresume(False);

		/* Okay, now we're in a weird sort of situation.  The screen is
		 * about to be forced to display "Hit <Enter> to continue" on
		 * the bottom of the window in open mode, and then wait for a
		 * keystroke.  That's a Good Thing.  But there are two problems
		 * we need to address:
		 *    * We want that prompt to appear at the bottom of the
		 *	screen, not the bottom of the window.
		 *    * After the user hits a key, we want to redraw all
		 *	windows.
		 *
		 * We'll set a flag indicating this situation.  The movecurs()
		 * function will test for that flag, and merely pretend to move
		 * the cursor when it is set.  The loop() function will test
		 * that flag after each keystroke, and expose all windows if
		 * it is set.
		 */
		afterprg = 2;
		afterscrl = 0;
	}

	return status;
}


#ifdef SIGSTOP
/* This function starts an interactive shell.  It is called with the argument
 * (True) for the :sh command, or (False) for a :stop or :suspend command.
 * If successful it returns RESULT_COMPLETE after the shell exits; if
 * unsuccessful it issues an error message and returns RESULT_ERROR.  It
 * could also return RESULT_MORE to defer processing to the portable code
 * in ex_suspend().
 */
static RESULT stop(alwaysfork)
	BOOLEAN	alwaysfork;	/* fork even if SIGSTOP would work? */
{
	RESULT	result;

	/* move the cursor to the bottom of the screen, and scroll up */
	tputs(tgoto(CM, 0, (int)o_ttyrows - 1), 0, ttych);
	ttych('\n');
	reset();
	ttyflush();

	/* if we want to fork, then the default processing is good enough */
	if (alwaysfork)
		return RESULT_MORE;

	/* call the OS-dependent function for stopping the process */
	result = ttystop();
	if (result == RESULT_MORE)
		return RESULT_MORE;

	/* arrange for all windows to be refreshed */
	afterprg = 1;
	return result;
}
#endif


/* This function converts screen coordinates into a window, and coordinates
 * within that window.
 */
GUIWIN *ttywindow(ttyrow, ttycol, winrow, wincol)
	int	ttyrow, ttycol;		/* screen coordinates in */
	int	*winrow, *wincol;	/* window coordinates out */
{
	TWIN	*tw;

	if (ttycol < 0 || ttycol >= o_ttycolumns)
		return NULL;
	for (tw = twins;
	     tw && (tw->pos > ttyrow || ttyrow >= tw->pos + tw->height);
	     tw = tw->next)
	{
	}
	if (tw)
	{
		*winrow = ttyrow - tw->pos;
		*wincol = ttycol;
	}
	return (GUIWIN *)tw;
}


/* structs of this type are used to describe each available GUI */
GUI guitermcap =
{
	"termcap",	/* name */
	"Termcap/Terminfo interface with windows & color",
	False,		/* exonly */
	False,		/* newblank */
	False,		/* minimizeclr */
	True,		/* scrolllast */
	False,		/* shiftrows */
	3,		/* movecost */
	0,		/* opts */
	NULL,		/* optdescs */
	test,
	init,
	NULL,		/* usage */
	loop,
	ttypoll,
	term,
	creategw,
	destroygw,
	focusgw,
	NULL,		/* retitle */
	reset,
	flush,
	moveto,
	draw,
	shift,
	scroll,
	clrtoeol,
	NULL,		/* newline */
	beep,		/* beep */
	NULL,		/* msg */
	NULL,		/* scrollbar */
	NULL,		/* status */
	keylabel,
	NULL,		/* clipopen */
	NULL,		/* clipwrite */
	NULL,		/* clipread */
	NULL,		/* clipclose */
	color,		/* color */
	NULL,		/* guicmd */
	tabcmd,
	NULL,		/* save */
	NULL,		/* wildcard */
	ttyprgopen,
	ttyprgclose,
#ifdef SIGSTOP
	stop
#else
	NULL		/* stop */
#endif
};
#endif
