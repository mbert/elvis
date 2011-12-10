/*
** guiwin.c  --  MS-Windows support for elvis.
**
** Copyright 1996, Serge Pirotte.
*/

#define CHAR    Char
#define BOOLEAN Boolean
#include "elvis.h" 
#undef CHAR
#undef BOOLEAN 

#if defined (GUI_WIN32)

#include <windows.h>
#include <commctrl.h>
#include "winelvis.h"
#include "elvisres.h"

extern GUI guiquit;
GUI_WINDOW      gw_def_win;

static struct {
    char            *name;
    COLORREF        color;
} color_table[] = {
        { "black",      RGB (  0,  0,  0) },
        { "blue",       RGB (  0,  0,128) },
        { "cyan",       RGB (  0,128,128) },
        { "green",      RGB (  0,128,  0) },
        { "red",        RGB (128,  0,  0) },
        { "magenta",    RGB (128,  0,128) },
        { "brown",      RGB (128,128,  0) },
        { "gray",       RGB (128,128,128) },
        { "darkgray",   RGB ( 64, 64, 64) },
        { "lightblue",  RGB (  0,  0,255) },
        { "lightcyan",  RGB (  0,255,255) },
        { "lightgreen", RGB (  0,255,  0) },
        { "lightred",   RGB (255,  0,  0) },
        { "lightgray",  RGB (192,192,192) },
        { "yellow",     RGB (255,255,  0) },
        { "white",      RGB (255,255,255) },
        { "lightmagenta",RGB (255,  0,  255) },
        { 0,            RGB (  0,  0,  0) },
        /*  ADB - Add as temp store for bgc */
        { 0,            RGB (  0,  0,  0) }
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
        { "<Up>",         "k",     MAP_ALL, vkeyUp        },
        { "<Down>",       "j",     MAP_ALL, vkeyDown      },
        { "<Left>",       "h",     MAP_ALL, vkeyLeft      },
        { "<Right>",      "l",     MAP_ALL, vkeyRight     },
        { "<CLeft>",      "B",     MAP_ALL, vkeyCLeft     },
        { "<CRight>",     "W",     MAP_ALL, vkeyCRight    },
        { "<PgUp>",       "\x02",  MAP_ALL, vkeyPgUp      },
        { "<PgDn>",       "\x06",  MAP_ALL, vkeyPgDn      },
        { "<Home>",       "^",     MAP_ALL, vkeyHome      },
        { "<End>",        "$",     MAP_ALL, vkeyEnd       },
        { "<CPgUp>",      "1G",    MAP_ALL, vkeyCPgUp     },
        { "<CPgDn>",      "G",     MAP_ALL, vkeyCPgDn     },
        { "<CHome>",      "1G",    MAP_ALL, vkeyCHome     },
        { "<CEnd>",       "G",     MAP_ALL, vkeyCEnd      },
        { "<Insert>",     "i",     MAP_ALL, vkeyIns       },
        { "<Delete>",     "x",     MAP_ALL, vkeyDel       },
        { "#1",           0,       MAP_ALL, vkeyF1        },
        { "#2",           0,       MAP_ALL, vkeyF2        },
        { "#3",           0,       MAP_ALL, vkeyF3        },
        { "#4",           0,       MAP_ALL, vkeyF4        },
        { "#5",           0,       MAP_ALL, vkeyF5        },
        { "#6",           0,       MAP_ALL, vkeyF6        },
        { "#7",           0,       MAP_ALL, vkeyF7        },
        { "#8",           0,       MAP_ALL, vkeyF8        },
        { "#9",           0,       MAP_ALL, vkeyF9        },
        { "#10",          0,       MAP_ALL, vkeyF10       },
        { "#11",          0,       MAP_ALL, vkeyF11       },
        { "#12",          0,       MAP_ALL, vkeyF12       },
        { "#1s",          0,       MAP_ALL, vkeySF1       },
        { "#2s",          0,       MAP_ALL, vkeySF2       },
        { "#3s",          0,       MAP_ALL, vkeySF3       },
        { "#4s",          0,       MAP_ALL, vkeySF4       },
        { "#5s",          0,       MAP_ALL, vkeySF5       },
        { "#6s",          0,       MAP_ALL, vkeySF6       },
        { "#7s",          0,       MAP_ALL, vkeySF7       },
        { "#8s",          0,       MAP_ALL, vkeySF8       },
        { "#9s",          0,       MAP_ALL, vkeySF9       },
        { "#10s",         0,       MAP_ALL, vkeySF10      },
        { "#11s",         0,       MAP_ALL, vkeySF11      },
        { "#12s",         0,       MAP_ALL, vkeySF12      },
        { "#1c",          0,       MAP_ALL, vkeyCF1       },
        { "#2c",          0,       MAP_ALL, vkeyCF2       },
        { "#3c",          0,       MAP_ALL, vkeyCF3       },
        { "#4c",          0,       MAP_ALL, vkeyCF4       },
        { "#5c",          0,       MAP_ALL, vkeyCF5       },
        { "#6c",          0,       MAP_ALL, vkeyCF6       },
        { "#7c",          0,       MAP_ALL, vkeyCF7       },
        { "#8c",          0,       MAP_ALL, vkeyCF8       },
        { "#9c",          0,       MAP_ALL, vkeyCF9       },
        { "#10c",         0,       MAP_ALL, vkeyCF10      },
        { "#11c",         0,       MAP_ALL, vkeyCF11      },
        { "#12c",         0,       MAP_ALL, vkeyCF12      },
        { "#1a",          0,       MAP_ALL, vkeyAF1       },
        { "#2a",          0,       MAP_ALL, vkeyAF2       },
        { "#3a",          0,       MAP_ALL, vkeyAF3       },
        { "#4a",          0,       MAP_ALL, vkeyAF4       },
        { "#5a",          0,       MAP_ALL, vkeyAF5       },
        { "#6a",          0,       MAP_ALL, vkeyAF6       },
        { "#7a",          0,       MAP_ALL, vkeyAF7       },
        { "#8a",          0,       MAP_ALL, vkeyAF8       },
        { "#9a",          0,       MAP_ALL, vkeyAF9       },
        { "#10a",         0,       MAP_ALL, vkeyAF10      },
        { "#11a",         0,       MAP_ALL, vkeyAF11      },
        { "#12a",         0,       MAP_ALL, vkeyAF12      },
    { 0,              0,       0,       0             }
};

static OPTDESC gw_opt_desc[] = {
    { "scrollbar",      "sb",   NULL,       NULL, NULL, optstoresb },
    { "toolbar",        "tb",   NULL,       NULL, NULL, optstoretb },
    { "statusbar",      "stb",  NULL,       NULL, NULL, optstorestb },
    { "menubar",        "mbar", NULL,       NULL, NULL, optstoremnu },
    { "font",           "fnt",  optsstring, optisfont },
    { "normalstyle",    "nfn",  optsstring, optisstring, "n b i u bi bu iu biu", optstoreattr },
    { "fixedstyle",     "ffn",  optsstring, optisstring, "n b i u bi bu iu biu", optstoreattr },
    { "boldstyle",      "bfn",  optsstring, optisstring, "n b i u bi bu iu biu", optstoreattr },
    { "emphasizedstyle","efn",  optsstring, optisstring, "n b i u bi bu iu biu", optstoreattr },
    { "italicstyle",    "ifn",  optsstring, optisstring, "n b i u bi bu iu biu", optstoreattr },
    { "underlinedstyle","ufn",  optsstring, optisstring, "n b i u bi bu iu biu", optstoreattr }
};

static HGLOBAL  clip_hGlob = NULL;
static char     *clip_data = 0;
static int      clip_len = 0;
static int      clip_offset = 0;

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
    COLORREF            bgwincolor;

    gw_def_win.nextp = NULL;

    /* map the virtual keys. */
    for (i = 0; gwkeys[i].label != 0; i++)
        if (gwkeys[i].cooked)
            mapinsert ((unsigned char *)gwkeys[i].rawin, 
                       strlen ((char *)gwkeys[i].rawin),
                       (unsigned char *)gwkeys[i].cooked, 
                       strlen ((char *)gwkeys[i].cooked),
                       (unsigned char *)gwkeys[i].label, 
                       gwkeys[i].flags);

    /* set default values for options */
    optpreset (o_scrollbar (&gw_def_win), True, OPT_REDRAW);
    optpreset (o_toolbar (&gw_def_win), True, OPT_REDRAW);
    optpreset (o_statusbar (&gw_def_win), True, OPT_REDRAW);
    optpreset (o_menubar (&gw_def_win), True, OPT_REDRAW);
    optpreset (o_font (&gw_def_win), CHARdup (DEFAULT_FONT), OPT_REDRAW);
    optpreset (o_normalstyle (&gw_def_win), CHARdup( "n"), OPT_REDRAW);
    optpreset (o_fixedstyle (&gw_def_win), CHARdup ("n"), OPT_REDRAW);
    optpreset (o_boldstyle (&gw_def_win), CHARdup ("b"), OPT_REDRAW);
    optpreset (o_emphasizedstyle (&gw_def_win), CHARdup ("b"), OPT_REDRAW);
    optpreset (o_italicstyle (&gw_def_win), CHARdup ("i"), OPT_REDRAW);
    optpreset (o_underlinedstyle (&gw_def_win), CHARdup ("u"), OPT_REDRAW);

    /* install default options */
    optinsert("guiwin", NUM_OPTIONS, gw_opt_desc, (OPTVAL *)&gw_def_win.options);

    /* set the default colors */
    bgwincolor = (COLORREF)GetSysColor (COLOR_WINDOW);
    gw_def_win.colors.fgcolor = (COLORREF)0x00000000;
    gw_def_win.colors.bgcolor = (COLORREF)bgwincolor;
    gw_def_win.colors.ffgcolor = (COLORREF)0x00000000;
    gw_def_win.colors.fbgcolor = (COLORREF)bgwincolor;
    gw_def_win.colors.bfgcolor = (COLORREF)0x00000000;
    gw_def_win.colors.bbgcolor = (COLORREF)bgwincolor;
    gw_def_win.colors.efgcolor = (COLORREF)0x00000000;
    gw_def_win.colors.ebgcolor = (COLORREF)bgwincolor;
    gw_def_win.colors.ifgcolor = (COLORREF)0x00000000;
    gw_def_win.colors.ibgcolor = (COLORREF)bgwincolor;
    gw_def_win.colors.ufgcolor = (COLORREF)0x00000000;
    gw_def_win.colors.ubgcolor = (COLORREF)bgwincolor;

    /* install the default printer */
    gw_get_default_printer ();
    
    return argc;
}

/* ------------------------------------------------------------------------
**
** loop  --  process message loop.
*/

static void gwloop (void)

{
    MSG             msg;
    GUI_WINDOW      *gwp;

	/* if -Gquit, and there were no errors, then try to destroy the windows */
    if (gui == &guiquit && o_exitcode == 0 && gw_def_win.nextp)
		eventex(gw_def_win.nextp, "qall", False);

    while (gw_def_win.nextp != NULL) {

        /* process Windows messages */
        GetMessage (&msg, NULL, 0, 0);
		TranslateMessage (&msg);
		DispatchMessage (&msg);

        /* repaint the windows */
        if (msg.message != WM_PAINT)
			for (gwp = gw_def_win.nextp; gwp != NULL; gwp = gwp->nextp)
				gw_redraw_win (gwp);
    }
}


/* ------------------------------------------------------------------------
**
** wpoll  -- check if some input pending from the keyboard.
*/

static Boolean gwwpoll (Boolean reset)

{
    MSG     msg;

    /* if reset, do nothing */
    if (reset)
        return False;

    /* check if something on the message queue */
    if (PeekMessage (&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE))
        return True;

    return False;
}

/* ------------------------------------------------------------------------
**
** term  --  cleanup the gui.
*/

static void gwterm (void)

{
}

/* ------------------------------------------------------------------------
**
** creategw  --  create a new gui window.
*/

static Boolean gwcreategw (char *name, char *firstcmd)

{
    GUI_WINDOW      *gwp;
    DWORD           dwStyle = WS_CHILD | WS_CLIPSIBLINGS |
                              WS_BORDER | WS_VISIBLE;

    /* allocate a new GUI_WINDOW */
    if ((gwp = calloc (1, sizeof (GUI_WINDOW))) == NULL)
        return False;

    gwp->nextp = gw_def_win.nextp;
    gw_def_win.nextp = gwp;

    /* set default options */
    gwp->colors = gw_def_win.colors;
    gwp->options = gw_def_win.options;
    o_font (gwp) = CHARdup (o_font (&gw_def_win));
    o_normalstyle (gwp) = CHARdup (o_normalstyle (&gw_def_win));
    o_fixedstyle (gwp) = CHARdup (o_fixedstyle (&gw_def_win));
    o_boldstyle (gwp) = CHARdup (o_boldstyle (&gw_def_win));
    o_emphasizedstyle (gwp) = CHARdup (o_emphasizedstyle (&gw_def_win));
    o_italicstyle (gwp) = CHARdup (o_italicstyle (&gw_def_win));
    o_underlinedstyle (gwp) = CHARdup (o_underlinedstyle (&gw_def_win));

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
    gwp->active = 1;

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

    return True;
}

/* ------------------------------------------------------------------------
**
** destroygw  --  destroy a existing gui window.
*/

static void gwdestroygw (GUIWIN *gw, Boolean force)

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
    safefree (o_normalstyle (gwp));
    safefree (o_fixedstyle (gwp));
    safefree (o_boldstyle (gwp));
    safefree (o_emphasizedstyle (gwp));
    safefree (o_italicstyle (gwp));
    safefree (o_underlinedstyle (gwp));
    free (gwp);
}

/* ------------------------------------------------------------------------
**
** focusg  --  set window focus.
*/

static Boolean gwfocusgw (GUIWIN *gw)

{
    GUI_WINDOW      *gwp = (GUI_WINDOW *)gw;

    /* set the focus to the frame window, will do the rest itself */
    SetFocus (gwp->frameHWnd);

    return True;
}

/* -----------------------------------------------------------------------
**
** retitle  --  set a new window title.
*/

static void gwretitle (GUIWIN *gw, char *name)

{
    GUI_WINDOW      *gwp = (GUI_WINDOW *)gw;
    char            title[_MAX_PATH + 20];

	sprintf (title, "WinElvis - [%s]", name);
    SetWindowText (gwp->frameHWnd, title);
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
    if (gwp->cursor_type != CURSOR_NONE && gwp->clientHWnd == GetFocus ()) {
		HideCaret (gwp->clientHWnd);
		gw_set_cursor(gwp, FALSE);
		ShowCaret (gwp->clientHWnd);
    }
}

/* ------------------------------------------------------------------------
**
** draw  --  display text on the screen.
*/

static void gwdraw(GUIWIN *gw, _char_ font, Char *text, int len)

{
    register int    i;
    GUI_WINDOW      *gwp = (GUI_WINDOW *)gw;
    COLORREF        fgc;
    COLORREF        bgc;
    COLORREF        tmpc;
    HFONT           hFont;
    Char            ungraphic[256];
    RECT            rect;
    int             ileft;

    /* select font & colors */
    ileft = 0;
    switch (font) {
        case 'b':
            fgc = gwp->colors.bfgcolor;
            bgc = gwp->colors.bbgcolor;
            hFont = gwp->fonts.bfont;
            break;
        case 'f':
            fgc = gwp->colors.ffgcolor;
            bgc = gwp->colors.fbgcolor;
            hFont = gwp->fonts.ffont;
            break;
        case 'e':
            fgc = gwp->colors.efgcolor;
            bgc = gwp->colors.ebgcolor;
            hFont = gwp->fonts.efont;
            break;
        case 'i':
            fgc = gwp->colors.ifgcolor;
            bgc = gwp->colors.ibgcolor;
            hFont = gwp->fonts.ifont;
            if (o_italicstyle(gwp) && strchr(o_italicstyle(gwp), 'i'))
                ileft = (gwp->ycsize - 3) / 6; /* just a guess */
            break;
        case 'u':
            fgc = gwp->colors.ufgcolor;
            bgc = gwp->colors.ubgcolor;
            hFont = gwp->fonts.ufont;
            break;
        case 'g':
            fgc = gwp->colors.fgcolor;
            bgc = gwp->colors.bgcolor;
            hFont = gwp->fonts.nfont;
            for (i = 0; i < len && i < QTY (ungraphic); i++)
                if (text[i] < '1' || text[i] > '9')
                    ungraphic[i] = text[i];
                else
                    ungraphic[i] = '+';
            len = i;
            text = ungraphic;
            break;
        default:
            fgc = gwp->colors.fgcolor;
            bgc = gwp->colors.bgcolor;
            hFont = gwp->fonts.nfont;
            break;
    }
  
    /* check if inverse attribute */
    if (isupper (font)) {
        tmpc = fgc;
        fgc = bgc;
        bgc = tmpc;
    }

    /* compute update RECT */
    rect.top = gwp->currow * gwp->ycsize;
    rect.left = gwp->curcol * gwp->xcsize + gwp->xcsize / 2;
    rect.bottom = rect.top + gwp->ycsize;
    rect.right = rect.left + gwp->xcsize * len;

    /* create a DC */
    if (gwp->dc == NULL) {
        gwp->dc = GetDC (gwp->clientHWnd);
        SetMapMode (gwp->dc, MM_TEXT);
	}

    /* create a brush */
    if (gwp->hBrush == NULL)
		gwp->hBrush = CreateSolidBrush (gwp->colors.bgcolor);

    /* hide caret */
    if (gwp->cursor_type != CURSOR_NONE && gwp->clientHWnd == GetFocus ())
	{
		HideCaret (gwp->clientHWnd);
		gwp->cursor_type = CURSOR_NONE;
	}

    /* prepare DC & output text */
	SetTextColor (gwp->dc, fgc);
	SetBkColor (gwp->dc, bgc);
	SelectObject (gwp->dc, hFont);
	ExtTextOut (gwp->dc, rect.left - ileft, rect.top, ETO_OPAQUE | ETO_CLIPPED,
	           &rect, (char *)text, len, gwp->font_size_array);

    /* update cursor position */
    gwp->curcol += len;
#if 0
    if (gwp->clientHWnd == GetFocus ()) {
		gw_set_cursor (gwp, FALSE);
		ShowCaret (gwp->clientHWnd);
	}
#endif
}

/* ------------------------------------------------------------------------
**
** scroll  --  insert or delete rows into the window.
*/

static Boolean gwscroll (GUIWIN *gw, int qty, Boolean notlast)

{
    GUI_WINDOW      *gwp = (GUI_WINDOW *)gw;
    int             rows = gwp->numrows - gwp->currow;
    RECT            rect;

    /* adjust number of rows */
    if (notlast)
        rows--;

    /* compute update RECT */
    rect.top = gwp->currow * gwp->ycsize;
    rect.left = 0;
    rect.bottom = rect.top + rows * gwp->ycsize;
    rect.right = gwp->xsize;

    /* hide caret */
    if (gwp->cursor_type == CURSOR_NONE && gwp->clientHWnd == GetFocus ())
	{
		HideCaret (gwp->clientHWnd);
		gwp->cursor_type = CURSOR_NONE;
	}

    /* scroll window */
    ScrollWindowEx (gwp->clientHWnd, 0, qty * gwp->ycsize, &rect,
                    &rect, NULL, NULL, 0);

    return True;
}

/* ------------------------------------------------------------------------
**
** clrtoeol  --  clear to end of line.
*/

static Boolean gwclrtoeol(GUIWIN *gw)

{
    GUI_WINDOW      *gwp = (GUI_WINDOW *)gw;
    RECT            rect;

    /* compute update RECT */
    rect.top = gwp->currow * gwp->ycsize;
    rect.left = gwp->curcol * gwp->xcsize + gwp->xcsize / 2;
    rect.bottom = rect.top + gwp->ycsize;
    rect.right = gwp->xsize;

    /* create a DC */
    if (gwp->dc == NULL) {
        gwp->dc = GetDC (gwp->clientHWnd);
        SetMapMode (gwp->dc, MM_TEXT);
	}

    /* create a brush */
    if (gwp->hBrush == NULL)
		gwp->hBrush = CreateSolidBrush (gwp->colors.bgcolor);

    /* hide caret */
    if (gwp->cursor_type != CURSOR_NONE && gwp->clientHWnd == GetFocus ())
	{
		HideCaret (gwp->clientHWnd);
		gwp->cursor_type = CURSOR_NONE;
	}

    /* fill the rectangle with the bg color */
    SelectObject (gwp->dc, gwp->hBrush);
    FillRect (gwp->dc, &rect, gwp->hBrush);

    return True;
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
static Boolean gwstatus (GUIWIN *gw, Char *cmd, long line, long column, _CHAR_ learn, char *mode)

{
    GUI_WINDOW       *gwp = (GUI_WINDOW *)gw;

    if (!o_statusbar (gwp))
        return False;

    gw_upd_status_bar (gwp, cmd, line, column, (char)learn, mode);
    return True;
}

/* ------------------------------------------------------------------------
**
** keylabel  --  transform key labels into raw codes.
*/

static int gwkeylabel (Char *given, int givenlen, Char **label, Char **rawptr)

{
    register int        i;

    // compare the given text to each key's strings
    for (i = 0; gwkeys[i].label != 0; i++) {
    
        // does given string match key label or raw characters?
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

static Boolean gwclipopen (Boolean forwrite)

{
    GUI_WINDOW      *gwp;
    BUFFER          buf = cutbuffer ('>', False);

    /* check is something to do */
    if (!forwrite &&
        !IsClipboardFormatAvailable (CF_TEXT) &&
        !IsClipboardFormatAvailable (CF_OEMTEXT))
        return False;

    if (forwrite && buf == NULL)
        return False;

    /* get the active view */
    if ((gwp = gw_find_client (GetFocus ())) == NULL)
        return False;

    /* open the clipboard */
    if (!OpenClipboard (gwp->clientHWnd))
        return False;

    /* allocate memory if writing */
	if (forwrite) {
        clip_len = o_bufchars (buf) + o_buflines (buf) + 1;
        clip_hGlob = GlobalAlloc (GMEM_MOVEABLE | GMEM_DDESHARE,
                                  (DWORD)clip_len + 1);
        if (clip_hGlob == NULL)
            return False;

        clip_data = (char *)GlobalLock (clip_hGlob);
        clip_offset = 0;
        EmptyClipboard ();
    }

    /* indicate success */
    return True;
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
** color  --
*/

static Boolean gwcolor (GUIWIN *gw, _char_ font, Char *fg, Char *bg)

{
    GUI_WINDOW          *gwp = (GUI_WINDOW *)gw;
    register int        i;
    int                 fgc = -1,
                        bgc = -1;

    /* select the window */
    if (gwp == NULL)
        gwp = &gw_def_win;

    // select FG color
    if (fg == NULL)
        return False;
    for (i = 0; color_table[i].name != 0; i++)
        if (strcmp (color_table[i].name, fg) == 0) {
            fgc = i;
            break;
        }
#if 1
    /*  ADB 22/12/98 - No name specified.  Check RGB setting.  If valid then
        store in penultimate array slot.  RGB settings in array are not
        relevant if name=NULL so we can use it to save settings.  It is
        assigned to correct font colour lower down */
    /*  SK 25/1/99 - Tweaked to use X11-style color specifiers: #RRGGBB in hex*/
    if (fgc == -1)
    {
        unsigned int    R, G, B;
        int             Colours;

        Colours = sscanf(fg, "#%2x%2x%2x", &R, &G, &B);
        if (Colours == 3 && R <=255 && G <= 255 & B <= 255)
        {
            fgc = i;
            color_table[fgc].color = RGB(R, G, B);
        }
    }
#endif
    if (fgc == -1)
        return False;

    // select BG color
    if (bg != NULL) {
        for (i = 0; color_table[i].name != 0; i++)
            if (strcmp (color_table[i].name, bg) == 0) {
                bgc = i;
                break;
            }
#if 1
        /*  ADB 22/12/98 - No name specified.  Check RGB setting.  If valid then
            store in penultimate array slot.  RGB settings in array are not
            relevant if name=NULL so we can use it to save settings.  It is
            assigned to correct font colour lower down */
	/*  SK 25/1/99 - Tweaked to use X11-style color specifiers: #RRGGBB */
        if (bgc == -1)
        {
            unsigned int    R, G, B;
            int             Colours;

            Colours = sscanf(bg, "#%2x%2x%2x", &R, &G, &B);
            if (Colours == 3 && R <=255 && G <= 255 & B <= 255)
            {
                bgc = i + 1;
                color_table[bgc].color = RGB(R, G, B);
            }
        }
#endif
        if (bgc == -1)
            return False;
    }
    
    // save the new colors.
    switch (font) {
        case 'b':
            gwp->colors.bfgcolor = color_table[fgc].color;
            if (bgc != -1)
                gwp->colors.bbgcolor = color_table[bgc].color;
            break;
        case 'f':
            gwp->colors.ffgcolor = color_table[fgc].color;
            if (bgc != -1)
                gwp->colors.fbgcolor = color_table[bgc].color;
            break;
        case 'e':
            gwp->colors.efgcolor = color_table[fgc].color;
            if (bgc != -1)
                gwp->colors.ebgcolor = color_table[bgc].color;
            break;
        case 'i':
            gwp->colors.ifgcolor = color_table[fgc].color;
            if (bgc != -1)
                gwp->colors.ibgcolor = color_table[bgc].color;
            break;
        case 'u':
            gwp->colors.ufgcolor = color_table[fgc].color;
            if (bgc != -1)
                gwp->colors.ubgcolor = color_table[bgc].color;
            break;
        default:
            gwp->colors.fgcolor = color_table[fgc].color;
            if (bgc != -1)
                gwp->colors.bgcolor = color_table[bgc].color;
            break;
    }

    /* redraw the window. */
    if (gw != NULL)
        InvalidateRect (gwp->clientHWnd, NULL, TRUE);

    return True;
}                               

/* --------------------------------------------------------------------
**
** gwstop  --  start a shell.
*/

static RESULT gwstop (Boolean alwaysform)

{
    PROCESS_INFORMATION	    proc;
    STARTUPINFO		        start;

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
   False,   /* exonly */
   False,   /* newblank */
   True, /* minimizeclr */
   False,   /* scrolllast */
   False,   /* shiftrows */
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
   gwcolor,   /* color */
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
   False,   /* exonly */
   False,   /* newblank */
   True, /* minimizeclr */
   False,   /* scrolllast */
   False,   /* shiftrows */
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
   gwcolor,   /* color */
   NULL, /* guicmd */
   NULL, /* tabcmd */
   NULL, /* save */
   NULL, /* wildcard */
   NULL, /* prgopen */
   NULL, /* prgclose */
   gwstop  /* gwstop */
};
#endif
/* ex:se ts=4 sw=4: */
