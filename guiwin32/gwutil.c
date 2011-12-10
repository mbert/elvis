/*
** gwutil.c  --  utility functions for MS-Windows elvis.
**
** Copyright 1996, Serge Pirotte.
*/

#define CHAR    Char
#include "elvis.h" 
#undef CHAR
#undef ELVBOOL 

#if defined (GUI_WIN32)

#include <windows.h>
#include "winelvis.h"

/* --------------------------------------------------------------------
**
** gw_winofgw  --  find a elvis window.
*/

static WINDOW gw_winofgw (GUIWIN *pwin)

{
    extern WINDOWBUF    windefopts;
    extern WINDOW       windows;
    WINDOW              win;

	for (win = windows; win != NULL && win->gw != pwin; win = win->next)
	    ;

	return (win != NULL) ? win : &windefopts;
}

/* --------------------------------------------------------------------
**
** gw_find_client -- find a GUI_WINDOW by client HWND.
*/

GUI_WINDOW *gw_find_client (HWND hWnd)

{
    register GUI_WINDOW     *gwp = gw_def_win.nextp;

    while (gwp != NULL) {
        if (gwp->clientHWnd == hWnd)
            return gwp;
		gwp = gwp->nextp;
	}

    return NULL;
}

/* --------------------------------------------------------------------
**
** gw_find_frame -- find a GUI_WINDOW by frame HWND.
*/

GUI_WINDOW *gw_find_frame (HWND hWnd)

{
    register GUI_WINDOW     *gwp = gw_def_win.nextp;

    while (gwp != NULL) {
        if (gwp->frameHWnd == hWnd)
            return gwp;
		gwp = gwp->nextp;
	}

    return NULL;
}

/* --------------------------------------------------------------------
**
** gw_get_win_size  --  get the size of a window.
*/

void gw_get_win_size (GUI_WINDOW *gwp)

{
    register int    i;
    HDC             dc;
    TEXTMETRIC      tm;
    RECT            rect;

    /* get client size */
    GetClientRect (gwp->clientHWnd, &rect);              
    gwp->xsize = rect.right;
    gwp->ysize = rect.bottom;

    /* get current font size */
    dc = GetDC (gwp->clientHWnd);
    SetMapMode (dc, MM_TEXT);
    SelectObject (dc, gwp->fonts[0]);
    GetTextMetrics (dc, &tm);
    ReleaseDC (gwp->clientHWnd, dc);
    gwp->xcsize = tm.tmAveCharWidth;
    gwp->ycsize = tm.tmHeight;

    for (i = 0; i < 256; i++)
        gwp->font_size_array[i] = gwp->xcsize;

    // compute number of rows and cols
    gwp->numrows = rect.bottom / gwp->ycsize;
    gwp->numcols = (rect.right - gwp->xcsize / 2) / gwp->xcsize;
}

/* --------------------------------------------------------------------
**
** gw_set_win_size  --  set in size based on LINES & COLUMNS.
*/

int gw_set_win_size (GUI_WINDOW *gwp, int check)

{
    WINDOW          win = gw_winofgw ((GUIWIN *)gwp);
    RECT            frameRect;
    RECT            viewRect;
    RECT            desktopRect;
    HWND            desktopWnd;
    int             xSize;
    int             ySize;

    /* check for minimal window size (2 lines, 30 cols) */
    if (o_columns (win) < 30 || o_lines (win) < 2) {
        if (check)
            return 0;
        if (o_columns (win) < 30)
            o_columns (win) = 30;
        if (o_lines (win) < 2)
            o_lines (win) = 2;
    }
    
    /* compute new view client rect. */
    xSize = o_columns (win) * gwp->xcsize + gwp->xcsize / 2 + 2;
    ySize = o_lines (win) * gwp->ycsize + 2;

    /* compute new frame rect. */
    GetWindowRect (gwp->frameHWnd, &frameRect);
    frameRect.right -= frameRect.left;
    frameRect.bottom -= frameRect.top;
    GetClientRect (gwp->clientHWnd, &viewRect);
    frameRect.left -= (xSize - viewRect.right) / 2;
    frameRect.top -= (ySize - viewRect.bottom) / 2;
    frameRect.right = frameRect.right + (xSize - viewRect.right);
    frameRect.bottom = frameRect.bottom + (ySize - viewRect.bottom);

    /* check if it will fit */
    desktopWnd = GetDesktopWindow ();
    GetClientRect (desktopWnd, &desktopRect);
    if (frameRect.right > desktopRect.right ||
        frameRect.bottom > desktopRect.bottom) {
        if (frameRect.right > desktopRect.right)
	    frameRect.right = desktopRect.right;
        if (frameRect.bottom > desktopRect.bottom)
	    frameRect.bottom = desktopRect.bottom;
        if (check)
            return 0;
    }

    /* compute new frame rectangle */
    if (frameRect.top + frameRect.bottom > desktopRect.bottom) {
        frameRect.top -= (frameRect.top + frameRect.bottom - desktopRect.bottom);
    }

    if (frameRect.right > desktopRect.right) {
	frameRect.left -= (frameRect.left + frameRect.right - desktopRect.right);
    }

    if (frameRect.left < 0) {
        frameRect.left = 0;
    }
    if (frameRect.top < 0) {
	frameRect.top = 0;
    }

    /* change the window */
    SetWindowPos (gwp->frameHWnd, NULL, frameRect.left, frameRect.top,
                  frameRect.right, frameRect.bottom, SWP_NOZORDER);
    SetFocus (gwp->clientHWnd);

    /* get new size parameters */
    gw_get_win_size (gwp);

    return 1;
}

/* --------------------------------------------------------------------
**
** gw_set_fonts  --  initialize the fonts for a view.
*/

void gw_set_fonts (GUI_WINDOW *gwp)

{
    LOGFONT         lf;
    int             i;

    /* reset the structure */
    memset (&lf, 0, sizeof (LOGFONT));

    /* create the fonts */
    opt_parse_font (o_font (gwp), &lf);
    for (i = 0; i < QTY(gwp->fonts); i++)
    {
	lf.lfWeight = (i & 1) ? FW_BOLD : FW_NORMAL;
	lf.lfItalic = (i & 2) ? TRUE : FALSE;
	lf.lfUnderline = (i & 4) ? TRUE : FALSE;
	gwp->fonts[i] = CreateFontIndirect (&lf);
    }
}

/* --------------------------------------------------------------------
**
** gw_del_fonts  --  delete the fonts for a given view.
*/

void gw_del_fonts (GUI_WINDOW *gwp)

{
    int i;

    for (i = 0; i < QTY(gwp->fonts); i++)
	DeleteObject (gwp->fonts[i]);
}

/* --------------------------------------------------------------------
**
** gw_set_cursor -- set the shape and position of a window's caret.
**                  This does *NOT* call ShowCaret(), though.
*/

void gw_set_cursor (GUI_WINDOW *gwp, BOOLEAN chgshape)
{
    int	down;

    if (chgshape)
    {
	switch (gwp->cursor_type)
	{
	    case CURSOR_INSERT:
		CreateCaret (gwp->clientHWnd, NULL, gwp->xcsize / 4, gwp->ycsize);
		break;

	    case CURSOR_REPLACE:
		CreateCaret (gwp->clientHWnd, NULL, gwp->xcsize, gwp->ycsize / 4);
		break;

	    default:
		CreateCaret (gwp->clientHWnd, NULL, gwp->xcsize, gwp->ycsize - 4);
	}
    }

    switch (gwp->cursor_type)
    {
	case CURSOR_INSERT:
	    down = 0;
	    break;

	case CURSOR_REPLACE:
	    down = gwp->ycsize * 3 / 4;
	    break;

	default:
	    down = 2;
    }

    SetCaretPos (gwp->curcol * gwp->xcsize + gwp->xcsize / 2,
				 gwp->currow * gwp->ycsize + down);
}

/* --------------------------------------------------------------------
**
** gw_redraw_win  --  redraw a window.
*/

void gw_redraw_win (GUI_WINDOW *gwp)

{
    ELVCURSOR       cursor_type;

    if (gwp->active && !IsIconic (gwp->frameHWnd)) {
		cursor_type = eventdraw ((GUIWIN *)gwp);
		UpdateWindow(gwp->clientHWnd);
		if (gwp->clientHWnd == GetFocus () && cursor_type != gwp->cursor_type) {
			if (gwp->cursor_type != CURSOR_NONE)
				HideCaret (gwp->clientHWnd);
			gwp->cursor_type = cursor_type;
			DestroyCaret ();
			gw_set_cursor (gwp, ElvTrue);
			ShowCaret (gwp->clientHWnd);
		}
    }
}

#endif
/* ex:se sw=4 smarttab: */
