/*
** guiwin.c  --  MS-Windows support for elvis.
**
** Copyright 1996, Serge Pirotte.
*/

#define CHAR    Char
#include "elvis.h" 
#undef CHAR
#undef ELVBOOL 

#if defined (GUI_WIN32)

#include <windows.h>
#include <commctrl.h>
#include "winelvis.h"
#include "elvisres.h"

extern GUI guiquit;
GUI_WINDOW      gw_def_win;
#ifdef FEATURE_IMAGE
HBITMAP		normalimage;
HBITMAP		idleimage;
HICON		gwcustomicon;
#endif

GUI_COLORTBL colortbl[] =
{
	/* standard colors, should be available everywhere */
        { "black",       {  0,  0,  0} },
        { "blue",        {  0,  0,128} },
        { "cyan",        {  0,128,128} },
        { "green",       {  0,128,  0} },
        { "red",         {128,  0,  0} },
        { "magenta",     {128,  0,128} },
        { "brown",       {128,128,  0} },
        { "gray",        {128,128,128} },
        { "darkgray",    { 64, 64, 64} },
        { "lightblue",   {  0,  0,255} },
        { "lightcyan",   {  0,255,255} },
        { "lightgreen",  {  0,255,  0} },
        { "lightred",    {255,  0,  0} },
        { "lightgray",   {192,192,192} },
        { "yellow",      {255,255,  0} },
        { "white",       {255,255,255} },
        { "lightmagenta",{255,  0,255} },
	{ NULL }
};

static char vkeyUp[]     = { '\xFF',      VK_UP      , '\0' };
static char vkeyDown[]   = { '\xFF',      VK_DOWN    , '\0' };
static char vkeyLeft[]   = { '\xFF',      VK_LEFT    , '\0' };
static char vkeyRight[]  = { '\xFF',      VK_RIGHT   , '\0' };
static char vkeyCLeft[]  = { '\xFF', 'C', VK_LEFT    , '\0' };
static char vkeyCRight[] = { '\xFF', 'C', VK_RIGHT   , '\0' };
static char vkeyPgUp[]   = { '\xFF',      VK_PRIOR   , '\0' };
static char vkeyPgDn[]   = { '\xFF',      VK_NEXT    , '\0' };
static char vkeyHome[]   = { '\xFF',      VK_HOME    , '\0' };
static char vkeyEnd[]    = { '\xFF',      VK_END     , '\0' };
static char vkeyCPgUp[]  = { '\xFF', 'C', VK_PRIOR   , '\0' };
static char vkeyCPgDn[]  = { '\xFF', 'C', VK_NEXT    , '\0' };
static char vkeyCHome[]  = { '\xFF', 'C', VK_HOME    , '\0' };
static char vkeyCEnd[]   = { '\xFF', 'C', VK_END     , '\0' };
static char vkeyIns[]    = { '\xFF',      VK_INSERT  , '\0' };
static char vkeyDel[]    = { '\xFF',      VK_DELETE  , '\0' };
static char vkeySTab[]   = { '\xFF', 'S', VK_TAB     , '\0' };
static char vkeyF1[]     = { '\xFF',      VK_F1      , '\0' };
static char vkeyF2[]     = { '\xFF',      VK_F2      , '\0' };
static char vkeyF3[]     = { '\xFF',      VK_F3      , '\0' };
static char vkeyF4[]     = { '\xFF',      VK_F4      , '\0' };
static char vkeyF5[]     = { '\xFF',      VK_F5      , '\0' };
static char vkeyF6[]     = { '\xFF',      VK_F6      , '\0' };
static char vkeyF7[]     = { '\xFF',      VK_F7      , '\0' };
static char vkeyF8[]     = { '\xFF',      VK_F8      , '\0' };
static char vkeyF9[]     = { '\xFF',      VK_F9      , '\0' };
static char vkeyF10[]    = { '\xFF',      VK_F10     , '\0' };
static char vkeyF11[]    = { '\xFF',      VK_F11     , '\0' };
static char vkeyF12[]    = { '\xFF',      VK_F12     , '\0' };
static char vkeySF1[]    = { '\xFF', 'S', VK_F1      , '\0' };
static char vkeySF2[]    = { '\xFF', 'S', VK_F2      , '\0' };
static char vkeySF3[]    = { '\xFF', 'S', VK_F3      , '\0' };
static char vkeySF4[]    = { '\xFF', 'S', VK_F4      , '\0' };
static char vkeySF5[]    = { '\xFF', 'S', VK_F5      , '\0' };
static char vkeySF6[]    = { '\xFF', 'S', VK_F6      , '\0' };
static char vkeySF7[]    = { '\xFF', 'S', VK_F7      , '\0' };
static char vkeySF8[]    = { '\xFF', 'S', VK_F8      , '\0' };
static char vkeySF9[]    = { '\xFF', 'S', VK_F9      , '\0' };
static char vkeySF10[]   = { '\xFF', 'S', VK_F10     , '\0' };
static char vkeySF11[]   = { '\xFF', 'S', VK_F11     , '\0' };
static char vkeySF12[]   = { '\xFF', 'S', VK_F12     , '\0' };
static char vkeyCF1[]    = { '\xFF', 'C', VK_F1      , '\0' };
static char vkeyCF2[]    = { '\xFF', 'C', VK_F2      , '\0' };
static char vkeyCF3[]    = { '\xFF', 'C', VK_F3      , '\0' };
static char vkeyCF4[]    = { '\xFF', 'C', VK_F4      , '\0' };
static char vkeyCF5[]    = { '\xFF', 'C', VK_F5      , '\0' };
static char vkeyCF6[]    = { '\xFF', 'C', VK_F6      , '\0' };
static char vkeyCF7[]    = { '\xFF', 'C', VK_F7      , '\0' };
static char vkeyCF8[]    = { '\xFF', 'C', VK_F8      , '\0' };
static char vkeyCF9[]    = { '\xFF', 'C', VK_F9      , '\0' };
static char vkeyCF10[]   = { '\xFF', 'C', VK_F10     , '\0' };
static char vkeyCF11[]   = { '\xFF', 'C', VK_F11     , '\0' };
static char vkeyCF12[]   = { '\xFF', 'C', VK_F12     , '\0' };
static char vkeyAF1[]    = { '\xFF', 'A', VK_F1      , '\0' };
static char vkeyAF2[]    = { '\xFF', 'A', VK_F2      , '\0' };
static char vkeyAF3[]    = { '\xFF', 'A', VK_F3      , '\0' };
static char vkeyAF4[]    = { '\xFF', 'A', VK_F4      , '\0' };
static char vkeyAF5[]    = { '\xFF', 'A', VK_F5      , '\0' };
static char vkeyAF6[]    = { '\xFF', 'A', VK_F6      , '\0' };
static char vkeyAF7[]    = { '\xFF', 'A', VK_F7      , '\0' };
static char vkeyAF8[]    = { '\xFF', 'A', VK_F8      , '\0' };
static char vkeyAF9[]    = { '\xFF', 'A', VK_F9      , '\0' };
static char vkeyAF10[]   = { '\xFF', 'A', VK_F10     , '\0' };
static char vkeyAF11[]   = { '\xFF', 'A', VK_F11     , '\0' };
static char vkeyAF12[]   = { '\xFF', 'A', VK_F12     , '\0' };

static struct vkey gwkeys[] = {
        { "<Up>",         "k",     MAP_NOSAVE|MAP_ALL, vkeyUp        },
        { "<Down>",       "j",     MAP_NOSAVE|MAP_ALL, vkeyDown      },
        { "<Left>",       "h",     MAP_NOSAVE|MAP_ALL, vkeyLeft      },
        { "<Right>",      "l",     MAP_NOSAVE|MAP_ALL, vkeyRight     },
        { "<C-Left>",     "B",     MAP_NOSAVE|MAP_ALL, vkeyCLeft     },
        { "<C-Right>",    "W",     MAP_NOSAVE|MAP_ALL, vkeyCRight    },
        { "<PgUp>",       "\x02",  MAP_NOSAVE|MAP_ALL, vkeyPgUp      },
        { "<PgDn>",       "\x06",  MAP_NOSAVE|MAP_ALL, vkeyPgDn      },
        { "<Home>",       "^",     MAP_NOSAVE|MAP_ALL, vkeyHome      },
        { "<End>",        "$",     MAP_NOSAVE|MAP_ALL, vkeyEnd       },
        { "<C-PgUp>",     "1G",    MAP_NOSAVE|MAP_ALL, vkeyCPgUp     },
        { "<C-PgDn>",     "G",     MAP_NOSAVE|MAP_ALL, vkeyCPgDn     },
        { "<C-Home>",     "1G",    MAP_NOSAVE|MAP_ALL, vkeyCHome     },
        { "<C-End>",      "G",     MAP_NOSAVE|MAP_ALL, vkeyCEnd      },
        { "<Insert>",     "i",     MAP_NOSAVE|MAP_ALL, vkeyIns       },
        { "<Delete>",     "x",     MAP_NOSAVE|MAP_ALL, vkeyDel       },
        { "<S-Tab>",      "g\t",   MAP_NOSAVE|MAP_COMMAND, vkeySTab  },
        { "#1",           0,       MAP_NOSAVE|MAP_ALL, vkeyF1        },
        { "#2",           0,       MAP_NOSAVE|MAP_ALL, vkeyF2        },
        { "#3",           0,       MAP_NOSAVE|MAP_ALL, vkeyF3        },
        { "#4",           0,       MAP_NOSAVE|MAP_ALL, vkeyF4        },
        { "#5",           0,       MAP_NOSAVE|MAP_ALL, vkeyF5        },
        { "#6",           0,       MAP_NOSAVE|MAP_ALL, vkeyF6        },
        { "#7",           0,       MAP_NOSAVE|MAP_ALL, vkeyF7        },
        { "#8",           0,       MAP_NOSAVE|MAP_ALL, vkeyF8        },
        { "#9",           0,       MAP_NOSAVE|MAP_ALL, vkeyF9        },
        { "#10",          0,       MAP_NOSAVE|MAP_ALL, vkeyF10       },
        { "#11",          0,       MAP_NOSAVE|MAP_ALL, vkeyF11       },
        { "#12",          0,       MAP_NOSAVE|MAP_ALL, vkeyF12       },
        { "#1s",          0,       MAP_NOSAVE|MAP_ALL, vkeySF1       },
        { "#2s",          0,       MAP_NOSAVE|MAP_ALL, vkeySF2       },
        { "#3s",          0,       MAP_NOSAVE|MAP_ALL, vkeySF3       },
        { "#4s",          0,       MAP_NOSAVE|MAP_ALL, vkeySF4       },
        { "#5s",          0,       MAP_NOSAVE|MAP_ALL, vkeySF5       },
        { "#6s",          0,       MAP_NOSAVE|MAP_ALL, vkeySF6       },
        { "#7s",          0,       MAP_NOSAVE|MAP_ALL, vkeySF7       },
        { "#8s",          0,       MAP_NOSAVE|MAP_ALL, vkeySF8       },
        { "#9s",          0,       MAP_NOSAVE|MAP_ALL, vkeySF9       },
        { "#10s",         0,       MAP_NOSAVE|MAP_ALL, vkeySF10      },
        { "#11s",         0,       MAP_NOSAVE|MAP_ALL, vkeySF11      },
        { "#12s",         0,       MAP_NOSAVE|MAP_ALL, vkeySF12      },
        { "#1c",          0,       MAP_NOSAVE|MAP_ALL, vkeyCF1       },
        { "#2c",          0,       MAP_NOSAVE|MAP_ALL, vkeyCF2       },
        { "#3c",          0,       MAP_NOSAVE|MAP_ALL, vkeyCF3       },
        { "#4c",          0,       MAP_NOSAVE|MAP_ALL, vkeyCF4       },
        { "#5c",          0,       MAP_NOSAVE|MAP_ALL, vkeyCF5       },
        { "#6c",          0,       MAP_NOSAVE|MAP_ALL, vkeyCF6       },
        { "#7c",          0,       MAP_NOSAVE|MAP_ALL, vkeyCF7       },
        { "#8c",          0,       MAP_NOSAVE|MAP_ALL, vkeyCF8       },
        { "#9c",          0,       MAP_NOSAVE|MAP_ALL, vkeyCF9       },
        { "#10c",         0,       MAP_NOSAVE|MAP_ALL, vkeyCF10      },
        { "#11c",         0,       MAP_NOSAVE|MAP_ALL, vkeyCF11      },
        { "#12c",         0,       MAP_NOSAVE|MAP_ALL, vkeyCF12      },
        { "#1a",          0,       MAP_NOSAVE|MAP_ALL, vkeyAF1       },
        { "#2a",          0,       MAP_NOSAVE|MAP_ALL, vkeyAF2       },
        { "#3a",          0,       MAP_NOSAVE|MAP_ALL, vkeyAF3       },
        { "#4a",          0,       MAP_NOSAVE|MAP_ALL, vkeyAF4       },
        { "#5a",          0,       MAP_NOSAVE|MAP_ALL, vkeyAF5       },
        { "#6a",          0,       MAP_NOSAVE|MAP_ALL, vkeyAF6       },
        { "#7a",          0,       MAP_NOSAVE|MAP_ALL, vkeyAF7       },
        { "#8a",          0,       MAP_NOSAVE|MAP_ALL, vkeyAF8       },
        { "#9a",          0,       MAP_NOSAVE|MAP_ALL, vkeyAF9       },
        { "#10a",         0,       MAP_NOSAVE|MAP_ALL, vkeyAF10      },
        { "#11a",         0,       MAP_NOSAVE|MAP_ALL, vkeyAF11      },
        { "#12a",         0,       MAP_NOSAVE|MAP_ALL, vkeyAF12      },
        { 0,              0,       0,                  0             }
};

static OPTDESC gw_opt_desc[] = {
    { "scrollbar",      "sb",   NULL,       NULL, NULL, optstoresb },
    { "toolbar",        "tb",   NULL,       NULL, NULL, optstoretb },
    { "statusbar",      "stb",  NULL,       NULL, NULL, optstorestb },
    { "menubar",        "mbar", NULL,       NULL, NULL, optstoremnu },
    { "font",           "fnt",  optsstring, optisfont },
    { "propfont",       "pfnt", optsstring, optisfont },
    { "titleformat",	"title",optsstring, optisstring },
    { "scrollbgimage",	"sbi",	NULL,	    NULL}
};

static OPTDESC gw_optglob_desc[] = {
    { "iconimage",	"ii",	optsstring, optisicon }
};
static OPTVAL gw_optglob_val[QTY(gw_optglob_desc)];

static HGLOBAL  clip_hGlob = NULL;
static char     *clip_data = 0;
static int      clip_len = 0;
static int      clip_offset = 0;

static int      guidefont;

static void gwretitle (GUIWIN *gw, char *name);
static void gwscrollbar(GUIWIN *gw, long top, long bottom, long total);

/* --------------------------------------------------------------------------
**
** test  --  test if gui is available.
*/

static int gwtest (void)
{
    /* always available */
    return 1;
}

/* ------------------------------------------------------------------------
**
** init  --  init the gui.
*/

static int gwinit (int argc, char *argv[])
{
	register int        i;
	long		color;

	gw_def_win.nextp = NULL;

	/* map the virtual keys. */
	for (i = 0; gwkeys[i].label != 0; i++)
		if (gwkeys[i].cooked)
			mapinsert ((unsigned char *)gwkeys[i].rawin, 
			           strlen ((char *)gwkeys[i].rawin),
			           (unsigned char *)gwkeys[i].cooked, 
			           strlen ((char *)gwkeys[i].cooked),
			           (unsigned char *)gwkeys[i].label, 
			           gwkeys[i].flags, NULL);

	/* set default values for options */
	optpreset (o_scrollbar (&gw_def_win), ElvTrue, OPT_REDRAW|OPT_HIDE);
	optpreset (o_toolbar (&gw_def_win), ElvTrue, OPT_REDRAW|OPT_HIDE);
	optpreset (o_statusbar (&gw_def_win), ElvTrue, OPT_REDRAW|OPT_HIDE);
	optpreset (o_menubar (&gw_def_win), ElvTrue, OPT_REDRAW|OPT_HIDE);
	optpreset (o_font (&gw_def_win), DEFAULT_FONT, OPT_REDRAW|OPT_HIDE);
	optpreset (o_propfont (&gw_def_win), DEFAULT_PROPFONT, OPT_REDRAW|OPT_HIDE);
	optpreset (o_titleformat (&gw_def_win), DEFAULT_TITLEFORMAT, OPT_HIDE);
	optpreset (o_scrollbgimage (&gw_def_win), ElvTrue, OPT_REDRAW|OPT_HIDE);

	/* install the global options */
	optinsert("win32", QTY(gw_optglob_desc), gw_optglob_desc, gw_optglob_val);

	/* install default options */
	optinsert("guiwin", NUM_OPTIONS, gw_opt_desc, (OPTVAL *)&gw_def_win.options);

	/* install the default printer */
	gw_get_default_printer ();

	/* install default "normal" colors. */
	colorinfo[COLOR_FONT_NORMAL].fg = color = GetSysColor(COLOR_WINDOWTEXT);
	colorinfo[COLOR_FONT_NORMAL].da.fg_rgb[0] = color & 0xff;
	colorinfo[COLOR_FONT_NORMAL].da.fg_rgb[1] = (color >> 8) & 0xff;
	colorinfo[COLOR_FONT_NORMAL].da.fg_rgb[2] = (color >> 16) & 0xff;
	colorinfo[COLOR_FONT_NORMAL].bg = color = GetSysColor(COLOR_WINDOW);
	colorinfo[COLOR_FONT_NORMAL].da.bg_rgb[0] = color & 0xff;
	colorinfo[COLOR_FONT_NORMAL].da.bg_rgb[1] = (color >> 8) & 0xff;
	colorinfo[COLOR_FONT_NORMAL].da.bg_rgb[2] = (color >> 16) & 0xff;
	colorinfo[COLOR_FONT_NORMAL].da.bits = COLOR_FG|COLOR_BG;

	/* locate the "guide" color */
	guidefont = colorfind("guide");

	return argc;
}

/* ------------------------------------------------------------------------
**
** loop  --  process message loop.
*/

static void gwloop (void)
{
	MSG             msg;

	/* the default window options aren't editable once we have real windows */
	optdelete((OPTVAL *)&gw_def_win.options);

	/* peform the -c command or -t tag */
	if (mainfirstcmd(windefault))
		return;

	/* if -Gquit, and there were no errors, then try to destroy the windows */
	if (gui == &guiquit && o_exitcode == 0 && gw_def_win.nextp)
		eventex(gw_def_win.nextp, "qall", ElvFalse);

	msg.message = 0;
	while (gw_def_win.nextp != NULL)
	{
		/* process Windows messages */
		GetMessage (&msg, NULL, 0, 0);
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
}


/* ------------------------------------------------------------------------
**
** wpoll  -- check if some input pending from the keyboard.
*/

static ELVBOOL gwwpoll(ELVBOOL reset)
{
	MSG     msg;

	/* if reset, do nothing */
	if (reset)
		return ElvFalse;

	/* check if something on the message queue */
	if (PeekMessage (&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE))
		return ElvTrue;

	return ElvFalse;
}

/* ------------------------------------------------------------------------
**
** term  --  cleanup the gui.
*/

static void gwterm(void)
{
	if (gwcustomicon)
		DestroyIcon(gwcustomicon);
}

/* ------------------------------------------------------------------------
**
** creategw  --  create a new gui window.
*/

static ELVBOOL gwcreategw(char *name, char *firstcmd)
{
	GUI_WINDOW      *gwp;
	DWORD           dwStyle = WS_CHILD | WS_CLIPSIBLINGS |
	                          WS_BORDER | WS_VISIBLE;

	/* allocate a new GUI_WINDOW */
	if ((gwp = calloc (1, sizeof (GUI_WINDOW))) == NULL)
		return ElvFalse;
	gwp->nextp = gw_def_win.nextp;
	gw_def_win.nextp = gwp;
	gwp->bg = colorinfo[COLOR_FONT_NORMAL].bg;

	/* set default options */
	gwp->options = gw_def_win.options;
	o_font(gwp) = CHARdup(o_font(&gw_def_win));
	o_propfont(gwp) = CHARdup(o_propfont(&gw_def_win));
	o_titleformat(gwp) = CHARdup(o_titleformat(&gw_def_win));

	/* create the Windows windows */
	gwp->frameHWnd = CreateWindow ("ElvisFrameWnd", "WinElvis",
	                               WS_OVERLAPPEDWINDOW,
	                               CW_USEDEFAULT, 0,
	                               CW_USEDEFAULT, 0,
	                               NULL, NULL, hInst, NULL);
	gw_create_toolbar (gwp);
	gw_create_status_bar (gwp);
	if (o_scrollbar (gwp))
		dwStyle |= WS_VSCROLL;
	gwp->clientHWnd = CreateWindow ("ElvisClientWnd", NULL,
	                                dwStyle, 0, 0, 0, 0,
	                                gwp->frameHWnd, NULL, hInst, NULL);
	gwp->menuHndl = o_menubar (gwp) ?
		LoadMenu (hInst, MAKEINTRESOURCE (IDM_ELVIS)) : NULL;
	SetMenu (gwp->frameHWnd, gwp->menuHndl);
	if (gwp->menuHndl != NULL)
		DrawMenuBar (gwp->frameHWnd);

	/* resize client window */
	SendMessage (gwp->frameHWnd, WM_SIZE, 0, 0);

	/* set the cursor */
	ShowCursor (FALSE);
	SetCursor (hLeftArrow);
	ShowCursor (TRUE);

	/* set the fonts */
	gw_set_fonts (gwp);

	/* set new title */
	gwretitle ((GUIWIN *)gwp, name);

	/* set window size */
	gw_get_win_size (gwp);
	gw_set_win_size (gwp, 0);

	/* tell elvis that we have a new window */
	eventcreate((GUIWIN *)gwp, &gwp->options.scrollbar, name,
	            gwp->numrows, gwp->numcols);

	/* make window active */
	eventfocus((GUIWIN *)gwp, ElvTrue);
	gwp->active = 1;

	/* redraw the contents */
	gw_redraw_win (gwp);

	/* show the window */
	ShowWindow (gwp->frameHWnd, SW_SHOW);

	/* accept dragging of files */
	DragAcceptFiles (gwp->clientHWnd, TRUE);

	/* disable printing if no printing */
	if (!gw_printing_ok)
		gw_disable_printing (gwp);

	/* if there is a firstcmd, then execute it */
	if (firstcmd)
	{
		winoptions(winofgw((GUIWIN *)gwp));
		exstring(windefault, toCHAR(firstcmd), "+cmd");
	}

	return ElvTrue;
}

/* ------------------------------------------------------------------------
**
** destroygw  --  destroy a existing gui window.
*/

static void gwdestroygw (GUIWIN *gw, ELVBOOL force)

{
	GUI_WINDOW      *gwp = (GUI_WINDOW *)gw;
	GUI_WINDOW      *winp;

	/* not active anymore */
	DragAcceptFiles (gwp->clientHWnd, FALSE);
	gwp->active = 0;

	/* tell elvis that we destroy the window */
	eventdestroy (gw);

	/* delete the fonts */
	gw_del_fonts (gwp);

	/* destroy the Windows windows */
	if (gwp->menuHndl != NULL)
		DestroyMenu (gwp->menuHndl);
	gw_destroy_toolbar (gwp);
	gw_destroy_status_bar (gwp);
	DestroyWindow (gwp->clientHWnd);
	DestroyWindow (gwp->frameHWnd);

	/* reclaim the GUI_WINDOW */
	for (winp = &gw_def_win; winp->nextp != gwp; winp = winp->nextp)
		;
	winp->nextp = gwp->nextp;

	safefree (o_font (gwp));
	safefree (o_propfont (gwp));
	safefree (o_titleformat (gwp));
	free (gwp);
}

/* ------------------------------------------------------------------------
**
** focusg  --  set window focus.
*/

static ELVBOOL gwfocusgw (GUIWIN *gw)

{
	GUI_WINDOW      *gwp = (GUI_WINDOW *)gw;

	/* set the focus to the frame window, will do the rest itself */
	SetFocus (gwp->frameHWnd);

	return ElvTrue;
}

/* -----------------------------------------------------------------------
**
** retitle  --  set a new window title.
*/

static void gwretitle (GUIWIN *gw, char *name)

{
	GUI_WINDOW      *gwp = (GUI_WINDOW *)gw;
#if 0
	char            title[_MAX_PATH + 20];
	BUFFER	    buf;

	buf = buffind(toCHAR(name));
	if (buf && o_filename(buf))
		name = tochar8(o_filename(buf));
	sprintf (title, "WinElvis - [%s]", name);
	SetWindowText (gwp->frameHWnd, title);
#else
	Char *argv[2];
	Char *title;
	ELVBOOL wasmsg;

	/* evaluate the titleformat string */
	argv[0] = (Char *)name;
	argv[1] = NULL;
	title = o_titleformat(gwp);
	if (!title)
		title = (Char *)"$1";
	wasmsg = msghide(ElvTrue);
	title = calculate(title, argv, CALC_MSG);
	msghide(wasmsg);
	if (title && *title)
		name = (char *)title;

	/* change the window's title */
	SetWindowText (gwp->frameHWnd, name);
#endif
}

/* ------------------------------------------------------------------------
**
** moveto  --  move to some position in the window.
*/

static void gwmoveto(GUIWIN *gw, int column, int row)

{
	GUI_WINDOW      *gwp = (GUI_WINDOW *)gw;

	/* retain current position */
	gwp->currow = row;
	gwp->curcol = column;

	/* set new position if active view */
	if (gwp->cursor_type != CURSOR_NONE && gwp->clientHWnd == GetFocus ())
	{
		HideCaret (gwp->clientHWnd);
		gw_set_cursor(gwp, FALSE);
		ShowCaret (gwp->clientHWnd);
	}
}

/* ------------------------------------------------------------------------
**
** draw  --  display text on the screen.
*/

static void gwdraw(GUIWIN *gw, long fg, long bg, int bits, Char *text, int len)

{
	register int    i;
	GUI_WINDOW      *gwp = (GUI_WINDOW *)gw;
	COLORREF        fgc, bgc;
	HBRUSH          hBrush;
	HPEN            hPen, hPrevPen;
	HDC             hDC;
	HFONT           hFont;
	RECT            rect;
	int             ileft;
	int             xleft, xcenter, xright, ytop, ycenter, ybottom, radius;
	UINT            options;

	/* Italics are slanted rightward from the bottom of the character cell.
	 * We'd like for them to look slanted from the center of the characters,
	 * and we can achieve that effect by shifting italic text slightly leftward.
	 */
	ileft = 0;
	if ((bits & COLOR_GRAPHIC) != COLOR_GRAPHIC && (bits & COLOR_ITALIC))
		ileft = (gwp->ycsize - 3) / 6; /* just a guess */

	/* Convert fg and bg args into COLORREF values */
	fgc = (COLORREF)fg;
	bgc = (COLORREF)bg;

	/* compute the update RECT */
	rect.top = gwp->currow * gwp->ycsize;
	rect.left = gwp->curcol * gwp->xcsize + gwp->xcsize / 2;
	rect.bottom = rect.top + gwp->ycsize;
	rect.right = rect.left + gwp->xcsize * len;

	/* Get the window's DC */
	hDC = GetDC (gwp->clientHWnd);
	SetMapMode (hDC, MM_TEXT);

	/* hide caret */
	if (gwp->cursor_type != CURSOR_NONE && gwp->clientHWnd == GetFocus ())
	{
		HideCaret (gwp->clientHWnd);
		gwp->cursor_type = CURSOR_NONE;
	}

	/* graphic chars are a special case */
	if ((bits & COLOR_GRAPHIC) == COLOR_GRAPHIC)
	{
		/* Strip out the COLOR_GRAPHIC bits */
		bits &= ~COLOR_GRAPHIC;

		/* Erase the area */
#ifdef FEATURE_IMAGE
		if (normalimage && (long)bgc == colorinfo[COLOR_FONT_NORMAL].bg)
		{
			gw_erase_rect(hDC, &rect, normalimage, gwp->scrolled);
		}
		else if (idleimage && (long)bgc == colorinfo[COLOR_FONT_IDLE].bg)
		{
			gw_erase_rect(hDC, &rect, idleimage, gwp->scrolled);
		}
		else
#endif
		{
			hBrush = CreateSolidBrush (bgc);
			FillRect (hDC, &rect, hBrush);
			DeleteObject(hBrush);
		}

		/* Select the foreground color */
		hPen = CreatePen(PS_SOLID, 0, fgc);
		hPrevPen = SelectObject(hDC, hPen);

		/* Find special points in the first character cell */
		radius = gwp->xcsize / 3;
		xleft = rect.left;
		xright = xleft + gwp->xcsize;
		xcenter = (xleft + xright) / 2;
		ytop = rect.top;
		ybottom = rect.bottom;
		ycenter = (ytop + ybottom) / 2;

		/* For each graphic character... */
		for (i = 0; i < len; text++, i++)
		{
			/* Draw line segments, as appropriate for this character */
			if (strchr("123456|", *text))
			{
				MoveToEx(hDC, xcenter, ytop, NULL);
				LineTo(hDC, xcenter, ycenter);
			}
			if (strchr("456789|", *text))
			{
				MoveToEx(hDC, xcenter, ycenter, NULL);
				LineTo(hDC, xcenter, ybottom);
			}
			if (strchr("235689-", *text))
			{
				MoveToEx(hDC, xleft, ycenter, NULL);
				LineTo(hDC, xcenter, ycenter);
			}
			if (strchr("124578-", *text))
			{
				MoveToEx(hDC, xcenter, ycenter, NULL);
				LineTo(hDC, xright, ycenter);
			}
			if (*text == 'o')
			{
				Arc(hDC, xcenter - radius, ycenter - radius,
				xcenter + radius, ycenter + radius,
				xcenter - radius, ycenter,  xcenter - radius, ycenter);
			}
			if (*text == '*')
			{
				HBRUSH	oldbrush, newbrush;
				newbrush = CreateSolidBrush(fgc);
				oldbrush = SelectObject(hDC, newbrush);
				Pie(hDC, xcenter - radius, ycenter - radius,
				xcenter + radius, ycenter + radius,
				xcenter - radius, ycenter,  xcenter - radius, ycenter);
				SelectObject(hDC, oldbrush);
				DeleteObject(newbrush);
			}

			/* Advance the points to the next cell */
			xleft = xright;
			xcenter += gwp->xcsize;
			xright += gwp->xcsize;
		}

		/* Restore foreground color to its previous value, so we can delete
		 * the local hPen object.
		 */
		SelectObject(hDC, hPrevPen);
		DeleteObject(hPen);
	}
	else
	{
		/* Find a font with the right bold/italic/underlined attributes */
		i = 0;
		if (bits & COLOR_BOLD) i += 1;
		if (bits & COLOR_ITALIC) i += 2;
		if (bits & COLOR_UNDERLINED) i += 4;
		hFont = gwp->fonts[i];

		/* prepare DC & output text */
		SetTextColor(hDC, fgc);
		SetBkColor(hDC, bgc);
		SetBkMode(hDC, OPAQUE);
		SelectObject(hDC, hFont);
		options = ETO_OPAQUE | ETO_CLIPPED;
#ifdef FEATURE_IMAGE
		if (normalimage && (long)bgc == colorinfo[COLOR_FONT_NORMAL].bg)
		{
			gw_erase_rect(hDC, &rect, normalimage, gwp->scrolled);
			options = ETO_CLIPPED;
			SetBkMode(hDC, TRANSPARENT);
		}
		else if (idleimage && (long)bgc == colorinfo[COLOR_FONT_IDLE].bg)
		{
			gw_erase_rect(hDC, &rect, idleimage, gwp->scrolled);
			options = ETO_CLIPPED;
			SetBkMode(hDC, TRANSPARENT);
		}
#endif
		ExtTextOut(hDC, rect.left - ileft, rect.top, options, &rect,
			(char *)text, len, gwp->font_size_array);
#ifdef FEATURE_IMAGE
		SetBkColor(hDC, bgc);
		SetBkMode(hDC, OPAQUE);
#endif
	}

	/* If COLOR_BOXED then draw a rectangle around the text */
	if (bits & (COLOR_BOXED | COLOR_LEFTBOX | COLOR_RIGHTBOX))
	{
		/* Select the foreground color */
		hPen = CreatePen(PS_SOLID, 0, fgc);
		hPrevPen = SelectObject(hDC, hPen);

		/* Draw the rectangle */
		if (bits & COLOR_BOXED)
		{
			MoveToEx(hDC, rect.left, rect.top, NULL);
			LineTo(hDC, rect.right, rect.top);
			MoveToEx(hDC, rect.left, rect.bottom - 1, NULL);
			LineTo(hDC, rect.right, rect.bottom - 1);
		}
			if (bits & COLOR_RIGHTBOX)
		{
			MoveToEx(hDC, rect.right - 1, rect.top, NULL);
			LineTo(hDC, rect.right - 1, rect.bottom);
		}
			if (bits & COLOR_LEFTBOX)
		{
			MoveToEx(hDC, rect.left, rect.top, NULL);
			LineTo(hDC, rect.left, rect.bottom);
		}

		/* Restore foreground color to its previous value, so we can delete
		 * the local hPen object.
		 */
		SelectObject(hDC, hPrevPen);
		DeleteObject(hPen);
	}

	/* release the window's device context */
	ReleaseDC(gwp->clientHWnd, hDC);

	/* update cursor position */
	gwp->curcol += len;
}

/* ------------------------------------------------------------------------
**
** scroll  --  insert or delete rows into the window.
*/

static ELVBOOL gwscroll (GUIWIN *gw, int qty, ELVBOOL notlast)

{
	GUI_WINDOW      *gwp = (GUI_WINDOW *)gw;
	int             rows = gwp->numrows - gwp->currow;
	RECT            rect;

#ifdef FEATURE_IMAGE
	/* if we're using background images, then don't scroll because it would
	 * shred the background image.
	 */
	if ((normalimage || idleimage)
		&& (!o_scrollbgimage(gwp) || gwp->currow != 0 || !notlast))
		return ElvFalse;

	/* adjust the number of scrolled pixels, for the background image */
	gwp->scrolled -= qty * gwp->ycsize;
#endif

	/* adjust number of rows */
	if (notlast)
		rows--;

	/* compute update RECT */
	rect.top = gwp->currow * gwp->ycsize;
	rect.left = 0;
	rect.bottom = rect.top + rows * gwp->ycsize;
	rect.right = gwp->xsize;

	/* hide caret */
	if (gwp->cursor_type != CURSOR_NONE && gwp->clientHWnd == GetFocus())
	{
		HideCaret (gwp->clientHWnd);
		gwp->cursor_type = CURSOR_NONE;
	}

	/* scroll window */
	ScrollWindowEx (gwp->clientHWnd, 0, qty * gwp->ycsize, &rect,
	                &rect, NULL, NULL, SW_INVALIDATE);

	return ElvTrue;
}

/* ------------------------------------------------------------------------
**
** clrtoeol  --  clear to end of line.
*/

static ELVBOOL gwclrtoeol(GUIWIN *gw)

{
	GUI_WINDOW      *gwp = (GUI_WINDOW *)gw;
	RECT            rect;
	HBRUSH	    brush;
	HDC		    hDC;

	/* compute update RECT */
	rect.top = gwp->currow * gwp->ycsize;
	rect.left = gwp->curcol * gwp->xcsize + gwp->xcsize / 2;
	rect.bottom = rect.top + gwp->ycsize;
	rect.right = gwp->xsize;

	/* hide caret */
	if (gwp->cursor_type != CURSOR_NONE && gwp->clientHWnd == GetFocus ())
	{
		HideCaret (gwp->clientHWnd);
		gwp->cursor_type = CURSOR_NONE;
	}

	/* get the window's DC */
	hDC = GetDC (gwp->clientHWnd);
	SetMapMode (hDC, MM_TEXT);

#ifdef FEATURE_IMAGE
	if (normalimage && (long)gwp->bg == colorinfo[COLOR_FONT_NORMAL].bg)
	{
		gw_erase_rect(hDC, &rect, normalimage, gwp->scrolled);
	}
	else if (idleimage && (long)gwp->bg == colorinfo[COLOR_FONT_IDLE].bg)
		{
	gw_erase_rect(hDC, &rect, idleimage, gwp->scrolled);
	}
	else
#endif
	{
	/* fill the rectangle with the bg color */
		brush = CreateSolidBrush(gwp->bg);
		FillRect (hDC, &rect, brush);
		DeleteObject(brush);
	}

	ReleaseDC(gwp->clientHWnd, hDC);
	return ElvTrue;
}

/* ------------------------------------------------------------------------
**
**  beep.
*/

static void gwbeep (GUIWIN *gw)

{
	GUI_WINDOW	*gwp = (GUI_WINDOW *)gw;

	if (o_flash && gw != (GUIWIN *)1)
	{
		FlashWindow(gwp->frameHWnd, TRUE);
		Sleep(100);
		FlashWindow(gwp->frameHWnd, FALSE);
	}
	else
		MessageBeep (MB_OK);
}

/* ------------------------------------------------------------------------
**
**  scrollbar  --  draw the scrollbar.
*/

static void gwscrollbar(GUIWIN *gw, long top, long bottom, long total)

{
	GUI_WINDOW      *gwp = (GUI_WINDOW *)gw;
#if defined (SIF_ALL)
	SCROLLINFO      scrollinfo;
#endif

	if (!o_scrollbar (gwp))
		return;

	/* Prescale to make total < 65535, to work around Win32 limit */
	while (total >= 65535)
	{
		top >>= 1;
		bottom >>= 1;
		total >>= 1;
	}

	if (total < gwp->numrows || bottom <= 0)
	{
		total = gwp->numrows;
		top = 0;
		bottom = total - 1;
	}
	if (top == 0 && bottom >= total) /* Weird bug in Win32? */
		bottom = total - 1;
	gwp->scrollsize = total;

#if defined (SIF_ALL)
	scrollinfo.cbSize = sizeof (scrollinfo);
	scrollinfo.fMask = SIF_ALL;
	scrollinfo.nMin = 0;
	scrollinfo.nMax = total;
	scrollinfo.nPos = top;
	scrollinfo.nPage = bottom - top + 1; /* "+1" is because nMin=0 */
	if (scrollinfo.nPage > (unsigned)total)
		scrollinfo.nPage = (unsigned)total + 1; /* "+1" is because nMin=0 */
	scrollinfo.nTrackPos = top;
	gwp->thumbsize = scrollinfo.nPage;
	SetScrollInfo (gwp->clientHWnd, SB_VERT, &scrollinfo, TRUE);
	if (o_verbose >= 3)
		msg(MSG_STATUS, "[ddd], nMin=0, nMax=$1, nPage=$2, nPos=$3",
		    (long)scrollinfo.nMax, (long)scrollinfo.nPage, (long)scrollinfo.nPos);
#else /* Win16, or Win32 with MSVC++ 2.0 */
	SetScrollRange (gwp->clientHWnd, SB_VERT, 0, total, FALSE);
	SetScrollPos (gwp->clientHWnd, SB_VERT, top, TRUE);
#endif
}

/* ------------------------------------------------------------------------
**
** status  --  format the status line.
*/
static ELVBOOL gwstatus (GUIWIN *gw, Char *cmd, long line, long column, _CHAR_ learn, char *mode)

{
	GUI_WINDOW       *gwp = (GUI_WINDOW *)gw;

	if (!o_statusbar (gwp))
		return ElvFalse;

	gw_upd_status_bar (gwp, cmd, line, column, (char)learn, mode);
	return ElvTrue;
}

/* ------------------------------------------------------------------------
**
** keylabel  --  transform key labels into raw codes.
*/

static int gwkeylabel (Char *given, int givenlen, Char **label, Char **rawptr)

{
	register int        i;

	/* compare the given text to each key's strings */
	for (i = 0; gwkeys[i].label != 0; i++) {

		/* does given string match key label or raw characters? */
		if ((!strncmp (gwkeys[i].label, tochar8 (given), (size_t)givenlen) && !gwkeys[i].label[givenlen])
		 || (!strncmp (gwkeys[i].rawin, tochar8 (given), (size_t)givenlen) && !gwkeys[i].rawin[givenlen]))
		{

			/* Set the label and rawptr pointers, return rawlen */
			*label = (unsigned char *)gwkeys[i].label;
			*rawptr = (unsigned char *)gwkeys[i].rawin;
			return strlen ((char *)*rawptr);
		}
	}

	return 0;
}

/* ------------------------------------------------------------------------
**
** clipopen  --  open the clipboard.
*/

static ELVBOOL gwclipopen (ELVBOOL forwrite)

{
	GUI_WINDOW      *gwp;
	BUFFER          buf = cutbuffer ('>', ElvFalse);

	/* check is something to do */
	if (!forwrite &&
		!IsClipboardFormatAvailable (CF_TEXT) &&
		!IsClipboardFormatAvailable (CF_OEMTEXT))
		return ElvFalse;

	if (forwrite && buf == NULL)
		return ElvFalse;

	/* get the active view */
	if ((gwp = gw_find_client (GetFocus ())) == NULL)
		return ElvFalse;

	/* open the clipboard */
	if (!OpenClipboard (gwp->clientHWnd))
		return ElvFalse;

	/* allocate memory if writing */
	if (forwrite) {
		clip_len = o_bufchars (buf) + o_buflines (buf) + 1;
		clip_hGlob = GlobalAlloc (GMEM_MOVEABLE | GMEM_DDESHARE,
		                          (DWORD)clip_len + 1);
		if (clip_hGlob == NULL)
			return ElvFalse;

		clip_data = (char *)GlobalLock (clip_hGlob);
		clip_offset = 0;
		EmptyClipboard ();
	}

	/* indicate success */
	return ElvTrue;
}

/* -----------------------------------------------------------------------
**
** clipwrite  --  write to the clipboard.
*/

static int gwclipwrite (Char *text, int len)

{
	register char       *p = clip_data + clip_offset;
	register int        numchars = len;

	/* fill the allocated memory block */
	while (numchars-- > 0) {
		if (*text == '\n') {
			*p++ = '\r';
			clip_offset++;
		}
		*p++ = *text++;
		clip_offset++;
	}

	return len;
}

/* ------------------------------------------------------------------------
**
** clipread  --  read from the clipboard.
*/

static int gwclipread (Char *text, int len)

{
	register char       *p;
	register int        numchars = 0;

	/* first time, rerieve memory block */
	if (clip_hGlob == NULL) {
		if ((clip_hGlob = GetClipboardData (CF_TEXT)) == NULL && 
			(clip_hGlob = GetClipboardData (CF_OEMTEXT)) == NULL)
				return 0;
		
		clip_data = (char *)GlobalLock (clip_hGlob);
		clip_len = strlen (clip_data) - 1;
		clip_offset = 0;
	}

	/* fill caller's data */
	for (p = clip_data + clip_offset; *p != '\0'; p++) {
		if (numchars == len)
			break;
		if (*p != '\r') {
			*text++ = *p;
			numchars++;
		}
		clip_offset++;
	}

	/* unlock global memory when done */
	if (numchars == 0) {
		GlobalUnlock (clip_hGlob);
		clip_hGlob = 0;
	}

	return numchars;
}

/* ------------------------------------------------------------------------
**
** clipclose  --  close the clipboard.
*/

static void gwclipclose (void)

{
	/* write to clipboard if writing */
	if (clip_hGlob != NULL) {
		clip_data[clip_offset] = '\0'; /* !!! */
		GlobalUnlock (clip_hGlob);
		SetClipboardData (CF_TEXT, clip_hGlob);
		clip_hGlob = NULL;
	}

	/* close the clipboard */
	CloseClipboard ();
}

/* ------------------------------------------------------------------------
**
** color  -- Convert a color name to a COLORREF value.
*/

static ELVBOOL gwcolor (int fontcode, Char *colornam, ELVBOOL isfg, long *colorptr, unsigned char rgb[3])
{
	register int        i, j;
	int			r, g, b;
	char		*rgbfile;
	char		rgbname[100];
	FILE		*fp;
#ifdef FEATURE_IMAGE
	HBITMAP		newimg;
	long		average;
	char		*imagefile;

	/* split the name into a "colornam" part and an "imagefile" part */
	imagefile = colornam;
	if (*imagefile == '#')
		imagefile++;
	while (*imagefile && !elvpunct(*imagefile))
		imagefile++;
	while (*imagefile && imagefile > colornam && !elvspace(*imagefile))
		imagefile--;
	if (!*imagefile)
		imagefile = NULL;
	else if (imagefile > colornam)
		*imagefile++ = '\0';
	else
		colornam = "";
#endif

	/* parse the color name */
	if (*colornam == '#')
	{
		/* Accept X11's "#rrggbb" or "#rgb" notations */
		if (sscanf(tochar8(colornam), "#%2x%*2x%2x%*2x%2x", &r, &g, &b) == 3)
			/* do nothing */;
		else if (sscanf(tochar8(colornam), "#%2x%2x%2x", &r, &g, &b) == 3)
			/* do nothing */;
		else if (sscanf(tochar8(colornam), "#%1x%1x%1x", &r, &g, &b) == 3)
		{
			r *= 17;
			g *= 17;
			b *= 17;
		}
		else
		{
			msg(MSG_ERROR, "[S]bad color notation $1", colornam);
			return ElvFalse;
		}
	}
	else if (!*colornam)
	{
		r = g = b = -1;
	}
	else
	{
		/* Normalize the color name by converting to lowercase and removing
		 * whitespace.  We can safely modify the colornam[] buffer in-place.
		 */
		for (i = j = 0; colornam[i]; i++)
			if (!elvspace(colornam[i]))
				colornam[j++] = elvtolower(colornam[i]);
		colornam[j] = '\0';

		/* look up the color */
		for (i = 0;
			 colortbl[i].name && CHARcmp(toCHAR(colortbl[i].name), colornam);
			 i++)
		{
		}
		if (colortbl[i].name)
		{
			/* Use the found color */
			r = colortbl[i].rgb[0];
			g = colortbl[i].rgb[1];
			b = colortbl[i].rgb[2];
		}
		else /* not found -- try "rgb.txt" */
		{
			/* search for the color in the "rgb.txt" file */
			*rgbname = '\0';
			rgbfile = iopath(o_elvispath, "rgb.txt", ElvFalse);
			if (rgbfile) {
				fp = fopen(rgbfile, "r");
				if (fp) {
					while (fscanf(fp, "%d %d %d %s", &r, &g, &b, &rgbname) == 4
					       && CHARcmp(tochar8(rgbname), colornam))
					{
					}
					fclose(fp);
				}
			}

			/* if we didn't find it there, then fail */
			if (CHARcmp(tochar8(rgbname), colornam)) {
				if (isfg) {
					memcpy(rgb, colorinfo[COLOR_FONT_NORMAL].da.fg_rgb, 3);
					*colorptr = colorinfo[COLOR_FONT_NORMAL].fg;
				} else {
					memcpy(rgb, colorinfo[COLOR_FONT_NORMAL].da.bg_rgb, 3);
					*colorptr = colorinfo[COLOR_FONT_NORMAL].bg;
				}
				msg(MSG_ERROR, "[S]unknown color $1", colornam);
				return ElvFalse;
			}
		}
	}

#ifdef FEATURE_IMAGE
	if (imagefile && isfg)
	{
		msg(MSG_ERROR, "Can't use images for foreground");
		return ElvFalse;
	}
#endif

	/* if no image or color, then fail */
	if (
#ifdef FEATURE_IMAGE
		!imagefile &&
#endif
		  r < 0)
	{
		msg(MSG_ERROR, "missing color name");
		return ElvFalse;
	}

#ifdef FEATURE_IMAGE
	/* if image name was given for "normal" or "idle" font, then load image */
	if (imagefile && (fontcode==COLOR_FONT_NORMAL || fontcode==COLOR_FONT_IDLE))
	{
		/* decide whether to use a tint */
		if (r >= 0)
			average = RGB(r, g, b);
		else
			average = -1;

		/* load the image */
		newimg = gw_load_xpm(imagefile, average, &average, NULL);
		if (newimg)
		{
			/* use the average colors */
			r = GetRValue(average);
			g = GetGValue(average);
			b = GetBValue(average);

			/* if there was an old image, discard it now */
			if (fontcode == COLOR_FONT_NORMAL && normalimage)
			gw_unload_xpm(normalimage);
			if (fontcode == COLOR_FONT_IDLE && idleimage)
			gw_unload_xpm(idleimage);

			/* store the new image */
			if (fontcode == COLOR_FONT_NORMAL)
			normalimage = newimg;
			else
			idleimage = newimg;
		}
		else
		{
			return ElvFalse;
		}
	}
	if (!imagefile && fontcode==COLOR_FONT_NORMAL && normalimage && !isfg)
	{
		gw_unload_xpm(normalimage);
		normalimage = NULL;
	}
	if (!imagefile && fontcode==COLOR_FONT_IDLE && idleimage && !isfg)
	{
		gw_unload_xpm(idleimage);
		idleimage = NULL;
	}
#endif

	/* Success!  Store the color and return ElvTrue */
	*colorptr = RGB(r, g, b);
	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
	return ElvTrue;
}                               

/* --------------------------------------------------------------------
**
** gwsetbg  --  Change a window's background color.
*/

void gwsetbg(GUIWIN *gw, long bg)
{
	GUI_WINDOW  *gwp = (GUI_WINDOW *)gw;
	RECT        rect;
	HDC		hDC;
	HBRUSH	hBrush;

	/* store the new background color */
	gwp->bg = (COLORREF)bg;

	/* get the window's DC */
	hDC = GetDC (gwp->clientHWnd);
	SetMapMode (hDC, MM_TEXT);

	/* hide caret */
	if (gwp->cursor_type != CURSOR_NONE && gwp->clientHWnd == GetFocus ())
	{
		HideCaret (gwp->clientHWnd);
		gwp->cursor_type = CURSOR_NONE;
	}

	/* The portable part of elvis already knows it needs to redraw the text
	 * area, but it doesn't know about blank areas around the outside of the
	 * window.  The simplest way to color those areas is to simply fill the
	 * whole window.
	 */
	rect.left = 0;
	rect.top = 0;
	rect.right = gwp->xsize + gwp->xcsize * 2;
	rect.bottom = gwp->ysize + gwp->ycsize;
#ifdef FEATURE_IMAGE
	if (normalimage && bg == colorinfo[COLOR_FONT_NORMAL].bg)
	{
		gw_erase_rect(hDC, &rect, normalimage, gwp->scrolled);
	}
	else if (idleimage && bg == colorinfo[COLOR_FONT_IDLE].bg)
	{
		gw_erase_rect(hDC, &rect, idleimage, gwp->scrolled);
	}
	else
#endif
	{
		hBrush = CreateSolidBrush(gwp->bg);
		FillRect(hDC, &rect, hBrush);
		DeleteObject(hBrush);
	}

	/* release the window's DC */
	ReleaseDC(gwp->clientHWnd, hDC);
}

/* --------------------------------------------------------------------
**
** gwstop  --  start a shell.
*/

static RESULT gwstop (ELVBOOL alwaysform)

{
	PROCESS_INFORMATION	    proc;
	STARTUPINFO	            start;

	/* save the buffers, if we're supposed to */
	eventsuspend ();

	/* start the process */
	memset (&start, 0, sizeof (start));
	start.cb = sizeof (start);
	if (CreateProcess (NULL, o_shell ? o_shell : tochar8 (o_shell), NULL,
	                   NULL, FALSE, CREATE_DEFAULT_ERROR_MODE |
	                   CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP |
	                   NORMAL_PRIORITY_CLASS, NULL, NULL, &start, &proc))
		return RESULT_COMPLETE;

	/* dang! */
	return RESULT_ERROR;
}

/* -------------------------------------------------------------------- */


GUI guiwin32 =

{
   "windows",  /* name */
   "MS-Windows graphic interface",
   ElvFalse,   /* exonly */
   ElvFalse,   /* newblank */
   ElvTrue, /* minimizeclr */
   ElvFalse,   /* scrolllast */
   ElvFalse,   /* shiftrows */
   1, /* movecost */
   NUM_OPTIONS, /* nopts */
   gw_opt_desc, /* optdescs */
   gwtest,
   gwinit,
   NULL,
   gwloop,
   gwwpoll,
   gwterm,
   gwcreategw,
   gwdestroygw,
   gwfocusgw,
   gwretitle,
   NULL, /* reset */
   NULL,
   gwmoveto,
   gwdraw,
   NULL,
   gwscroll,
   gwclrtoeol,
   NULL, /* newline */
   gwbeep,
   NULL, /* msg */
   gwscrollbar,
   gwstatus,
   gwkeylabel,
   gwclipopen,
   gwclipwrite,
   gwclipread,
   gwclipclose,
   gwcolor,
   NULL, /* freecolor */
   gwsetbg,
   NULL, /* guicmd */
   NULL, /* tabcmd */
   NULL, /* save */
   NULL, /* wildcard */
   NULL, /* prgopen */
   NULL, /* prgclose */
   gwstop  /* gwstop */
};



/* structs of this type are used to describe each available GUI */
GUI guiquit =
{
   "quit",  /* name */
   "Run `-c cmd' in a window, then quit",
   ElvFalse,   /* exonly */
   ElvFalse,   /* newblank */
   ElvTrue, /* minimizeclr */
   ElvFalse,   /* scrolllast */
   ElvFalse,   /* shiftrows */
   1, /* movecost */
   NUM_OPTIONS, /* nopts */
   gw_opt_desc, /* optdescs */
   gwtest,
   gwinit,
   NULL,
   gwloop,
   gwwpoll,
   gwterm,
   gwcreategw,
   gwdestroygw,
   gwfocusgw,
   gwretitle,
   NULL, /* reset */
   NULL,
   gwmoveto,
   gwdraw,
   NULL,
   gwscroll,
   gwclrtoeol,
   NULL, /* newline */
   gwbeep,
   NULL, /* msg */
   gwscrollbar,
   gwstatus,
   gwkeylabel,
   gwclipopen,
   gwclipwrite,
   gwclipread,
   gwclipclose,
   gwcolor,
   NULL, /* freecolor */
   gwsetbg,
   NULL, /* guicmd */
   NULL, /* tabcmd */
   NULL, /* save */
   NULL, /* wildcard */
   NULL, /* prgopen */
   NULL, /* prgclose */
   gwstop  /* gwstop */
};
#endif
/* ex:se sw=4 smarttab: */
