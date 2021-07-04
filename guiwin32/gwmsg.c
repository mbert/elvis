/*
** gwmsg.c  --  MS-Windows message processing for elvis.
**
** Copyright 1996, Serge Pirotte.
*/

#define CHAR    Char
#include "elvis.h" 
#undef CHAR
#undef ELVBOOL 

#if defined (GUI_WIN32)

#define _WIN32_WINNT    0x400
#include <windows.h>
#include "winelvis.h"
#include "elvisres.h"

#define MOUSE_SEL_NONE      0
#define MOUSE_SEL_CHAR      1
#define MOUSE_SEL_LINE      2
#define MOUSE_SEL_RECT      3

static int      state_shift = 0;
static int      state_ctrl = 0;
static int      dblclick = 0;
static int      mouse_init_row = 0;
static int      mouse_init_col = 0;
static int      mouse_moved = 0;
static int      mouse_down = 0;
static int      mouse_selection = MOUSE_SEL_NONE;

/* --------------------------------------------------------------------
**
** gwframe_WM_GETMINMAXINFO
*/

LRESULT gwframe_WM_GETMINMAXINFO (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	RECT            viewRect;
	RECT            frameRect;
	MINMAXINFO      *mmi = (MINMAXINFO *)lParam;

	if (gwp == NULL)
		return 1;

	mmi->ptMinTrackSize.x = 30 * gwp->xcsize + gwp->xcsize / 2 + 2;
	mmi->ptMinTrackSize.y = 2 * gwp->ycsize + 2;

	GetWindowRect (gwp->frameHWnd, &frameRect);
	frameRect.right -= frameRect.left;
	frameRect.bottom -= frameRect.top;
	GetClientRect (gwp->clientHWnd, &viewRect);

	mmi->ptMinTrackSize.x += (frameRect.right - viewRect.right);
	mmi->ptMinTrackSize.y += (frameRect.bottom - viewRect.bottom);

	return 0;
}

/* --------------------------------------------------------------------
**
** gwframe_WM_INITMENU
*/

LRESULT gwframe_WM_INITMENU (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	return 1;
}

/* --------------------------------------------------------------------
**
** gwframe_WM_MENUSELECT
*/

LRESULT gwframe_WM_MENUSELECT (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	char        str[80];
	char        *p;

	if (lParam == 0)
		gw_status_bar_text (gwp, NULL);
	else {
		if (LoadString (hInst, LOWORD (wParam), str, sizeof (str)) > 0) {
			if ((p = strchr (str, '\n')) != NULL)
				*p = '\0';
			gw_status_bar_text (gwp, str);
		}
		else
			gw_status_bar_text (gwp, NULL);
	}

	return 0;
}

/* --------------------------------------------------------------------
**
** gwframe_WM_NOTIFY
*/

#if _MSC_VER > 900
LRESULT gwframe_WM_NOTIFY (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	if (gwp->toolbarHWnd != NULL)
		gw_toolbar_tooltip (gwp, lParam);

	return 0;
}
#endif

/* --------------------------------------------------------------------
**
** gwframe_WM_PAINT
*/

LRESULT gwframe_WM_PAINT (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	PAINTSTRUCT     ps;

	BeginPaint (gwp->frameHWnd, &ps);
	EndPaint (gwp->frameHWnd, &ps);

	return 0;
}

/* --------------------------------------------------------------------
**
** gframe_WM_SETFOCUS
*/

LRESULT gwframe_WM_SETFOCUS (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	if (gwp->active)
		SetFocus (gwp->clientHWnd);

	return 0;
}

/* --------------------------------------------------------------------
**
** gframe_WM_SIZE
*/

LRESULT gwframe_WM_SIZE (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	RECT        rect;

	GetClientRect (gwp->frameHWnd, &rect);
	rect.top += gw_size_toolbar (gwp, &rect);
	rect.bottom -= gw_size_status_bar (gwp, &rect, wParam == SIZE_MAXIMIZED);
	if (wParam != SIZE_MINIMIZED) {
		SetWindowPos (gwp->clientHWnd, NULL, rect.left, rect.top,
		              rect.right - rect.left, rect.bottom - rect.top,
		              SWP_NOZORDER);
	}

	return 0;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_CHAR
*/

LRESULT gwclient_WM_CHAR (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	unsigned char       chr = (unsigned char)wParam;
	int		        i;

	/* skip <Shift-Tab> -- it is handled in gwclient_WM_KEYDOWN() */
	if (chr == '\t' && state_shift)
		return 1;

	/* otherwise pass the key to elvis */
	for (i = lParam & 0xFFFF; i > 0; i--)
		eventkeys((GUIWIN *)gwp, &chr, 1);

	gw_redraw_win (gwp);

	return 0;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_DROPFILES
*/

LRESULT gwclient_WM_DROPFILES (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	char       cmd[_MAX_PATH + 4];
	char       *quoted;
	UINT       i;
	UINT       numfiles;

	numfiles = DragQueryFile ((HANDLE)wParam, (UINT)-1, NULL, 0);
	for (i = 0; i < numfiles; i++) {
		strcpy (cmd, ":e ");
		DragQueryFile ((HANDLE)wParam, i, &cmd[3], _MAX_PATH);
		quoted = tochar8(addquotes(toCHAR("#% ()$"), toCHAR(&cmd[3])));
		strcpy(&cmd[3], quoted);
		safefree(quoted);
		eventex ((GUIWIN *)gwp, cmd, ElvFalse);
	}
	DragFinish ((HANDLE)wParam);

	gw_redraw_win (gwp);

	return 0;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_ERASEBKGND
*/

LRESULT gwclient_WM_ERASEBKGND (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	RECT        rect;
	HBRUSH      brush, prevbrush;
	HDC         dc;

	dc = GetDC (gwp->clientHWnd);
	GetUpdateRect (gwp->clientHWnd, &rect, FALSE);
#ifdef FEATURE_IMAGE
	if (normalimage && gwp->bg == (COLORREF)colorinfo[COLOR_FONT_NORMAL].bg)
	{
		gw_erase_rect(dc, &rect, normalimage, gwp->scrolled);
	}
	else if (idleimage && gwp->bg == (COLORREF)colorinfo[COLOR_FONT_IDLE].bg)
	{
		gw_erase_rect(dc, &rect, idleimage, gwp->scrolled);
	}
	else
#endif
	{
		brush = CreateSolidBrush (gwp->bg);
		prevbrush = SelectObject (dc, brush);
		FillRect (dc, &rect, brush);
		prevbrush = SelectObject (dc, prevbrush);
		DeleteObject (brush);
	}
	ReleaseDC (gwp->clientHWnd, dc);

	return 0;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_KEYDOWN
*/

LRESULT gwclient_WM_KEYDOWN (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	unsigned char       chr[3];
	int			        i;

	if (wParam == VK_SHIFT)
		state_shift = 1;
	else if (wParam == VK_CONTROL)
		state_ctrl = 1;
	else if (wParam == VK_NUMLOCK || wParam == VK_CAPITAL) {
		gw_upd_status_bar_ind (gwp, GetKeyState (VK_NUMLOCK) & 1,
			GetKeyState (VK_CAPITAL) & 1);
	}
	else {
		if (state_ctrl) {
			switch (wParam) {
				case VK_LEFT:
				case VK_RIGHT:
				case VK_PRIOR:
				case VK_NEXT:
				case VK_HOME:
				case VK_END:
				case VK_F1:
				case VK_F2:
				case VK_F3:
				case VK_F4:
				case VK_F5:
				case VK_F6:
				case VK_F7:
				case VK_F8:
				case VK_F9:
				case VK_F10:
				case VK_F11:
				case VK_F12:
					chr[0] = (unsigned char)'\xFF';
					chr[1] = 'C';
					chr[2] = (unsigned char)wParam;
					for (i = lParam & 0xFFFF; i > 0; i--)
						eventkeys ((GUIWIN *)gwp, chr, 3);
					gw_redraw_win (gwp);
					return 0;
			}
		}
		else if (state_shift) {
			switch (wParam) {
				case VK_UP:
				case VK_DOWN:
					eventclick((GUIWIN *)gwp, 0, 0, CLICK_SSLINE);
					chr[0] = (unsigned char)'\xFF';
					chr[1] = (unsigned char)wParam;
					for (i = lParam & 0xFFFF; i > 0; i--)
						eventkeys ((GUIWIN *)gwp, chr, 2);
					gw_redraw_win (gwp);
					return 0;

				case VK_LEFT:
				case VK_RIGHT:
				case VK_HOME:
				case VK_END:
					eventclick((GUIWIN *)gwp, 0, 0, CLICK_SSCHAR);
					chr[0] = (unsigned char)'\xFF';
					chr[1] = (unsigned char)wParam;
					for (i = lParam & 0xFFFF; i > 0; i--)
						eventkeys ((GUIWIN *)gwp, chr, 2);
					gw_redraw_win (gwp);
					return 0;

				case VK_F1:
				case VK_F2:
				case VK_F3:
				case VK_F4:
				case VK_F5:
				case VK_F6:
				case VK_F7:
				case VK_F8:
				case VK_F9:
				case VK_F10:
				case VK_F11:
				case VK_F12:
				case VK_TAB:
					chr[0] = (unsigned char)'\xFF';
					chr[1] = 'S';
					chr[2] = (unsigned char)wParam;
					for (i = lParam & 0xFFFF; i > 0; i--)
						eventkeys ((GUIWIN *)gwp, chr, 3);
					gw_redraw_win (gwp);
					return 0;
			}
		}
		else {
			switch (wParam) {
				case VK_UP:
				case VK_DOWN:
				case VK_LEFT:
				case VK_RIGHT:
				case VK_PRIOR:
				case VK_NEXT:
				case VK_HOME:
				case VK_END:
				case VK_INSERT:
				case VK_DELETE:
				case VK_F1:
				case VK_F2:
				case VK_F3:
				case VK_F4:
				case VK_F5:
				case VK_F6:
				case VK_F7:
				case VK_F8:
				case VK_F9:
				case VK_F10:
				case VK_F11:
				case VK_F12:
					chr[0] = (unsigned char)'\xFF';
					chr[1] = (unsigned char)wParam;
					for (i = lParam & 0xFFFF; i > 0; i--)
						eventkeys ((GUIWIN *)gwp, chr, 2);
					gw_redraw_win (gwp);
					return 0;
			}
		}
	}

	return 1;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_KEYUP
*/

LRESULT gwclient_WM_KEYUP (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	if (wParam == VK_SHIFT)
		state_shift = 0;
	else if (wParam == VK_CONTROL)
		state_ctrl = 0;
	else if (wParam == VK_NUMLOCK || wParam == VK_CAPITAL)
		gw_upd_status_bar_ind (gwp, GetKeyState (VK_NUMLOCK) & 1,
				GetKeyState (VK_CAPITAL) & 1);

	return 1;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_KILLFOCUS
*/

LRESULT gwclient_WM_KILLFOCUS (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	HideCaret (gwp->clientHWnd);
	DestroyCaret ();
	gwp->caret = 0;
	eventfocus(NULL, ElvTrue);
	gw_redraw_win (gwp);

	return 0;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_LBUTTONDBLCLK
*/

LRESULT gwclient_WM_LBUTTONDBLCLK (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	int     row = HIWORD (lParam) / gwp->ycsize;
	int     col = (LOWORD (lParam) - gwp->xcsize / 2) / gwp->xcsize;

	eventclick ((GUIWIN *)gwp, row, col, CLICK_MOVE);
	eventclick ((GUIWIN *)gwp, row, col, CLICK_TAG);
	dblclick = 1;
	mouse_selection = MOUSE_SEL_NONE;
	gw_redraw_win (gwp);

	return 0;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_LBUTTONDOWN
*/

LRESULT gwclient_WM_LBUTTONDOWN (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	mouse_down = 1;
	dblclick = 0;
	mouse_init_row = HIWORD (lParam) / gwp->ycsize;
	if (LOWORD (lParam) > (gwp->xcsize / 2))
		mouse_init_col = (LOWORD (lParam) - gwp->xcsize / 2) / gwp->xcsize;
	else
		mouse_init_col = 0;
	mouse_moved = 0;
	if (LOWORD (lParam) < gwp->xcsize / 2)
		mouse_selection = MOUSE_SEL_LINE;
	else if (GetKeyState(VK_MENU))
		mouse_selection = MOUSE_SEL_RECT;
	else
		mouse_selection = MOUSE_SEL_CHAR;
	eventclick ((GUIWIN *)gwp, -1, -1, CLICK_CANCEL);
	gw_redraw_win (gwp);

	return 0;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_LBUTTONUP
*/

LRESULT gwclient_WM_LBUTTONUP (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	mouse_down = 0;
	mouse_moved = 0;
	if (dblclick == 0)
	{
		eventclick ((GUIWIN *)gwp, HIWORD (lParam) / gwp->ycsize, 
			(LOWORD (lParam) - gwp->xcsize / 2) / gwp->xcsize,
			CLICK_MOVE);
		gw_redraw_win (gwp);
	}
	dblclick = 0;

	return 0;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_MOUSEMOVE
*/

LRESULT gwclient_WM_MOUSEMOVE (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	int     row;
	int     col;

#if 1
	//If the mouse is moved from the edge into the window suddenly, then
	// selectedCursor will be invalid.  Make sure it becomes valid.
	selectedCursor = GetCursor ();
#endif

	row = HIWORD (lParam) / gwp->ycsize;
	if (LOWORD (lParam) > (gwp->xcsize / 2)) {
		col = (LOWORD (lParam) - gwp->xcsize / 2) / gwp->xcsize;
		if (selectedCursor != hLeftArrow && !mouse_down) {
			ShowCursor (FALSE);
			SetCursor (hLeftArrow);
			ShowCursor (TRUE);
			selectedCursor = hLeftArrow;
		}
	}
	else {
		col = 0;
		if (selectedCursor != hRightArrow && !mouse_down) {
			ShowCursor (FALSE);
			SetCursor (hRightArrow);
			ShowCursor (TRUE);
			selectedCursor = hRightArrow;
		}
	}

	if (!mouse_down)
		return 0;

	if (row == mouse_init_row && col == mouse_init_col &&
		mouse_selection != MOUSE_SEL_LINE)
		return 0;

	if (mouse_moved) {
		eventclick ((GUIWIN *)gwp, row, col, CLICK_MOVE);
		gw_redraw_win (gwp);
	}
	else if (mouse_selection == MOUSE_SEL_LINE) {
		eventclick ((GUIWIN *)gwp, mouse_init_row, mouse_init_col,
		            CLICK_SELLINE);
		mouse_moved = 1;
		gw_redraw_win (gwp);
	}
	else if (mouse_selection == MOUSE_SEL_CHAR) {
		eventclick ((GUIWIN *)gwp, mouse_init_row, mouse_init_col,
		            CLICK_SELCHAR);
		mouse_moved = 1;
		gw_redraw_win (gwp);
	}
	else if (mouse_selection == MOUSE_SEL_RECT) {
		eventclick ((GUIWIN *)gwp, mouse_init_row, mouse_init_col,
		            CLICK_SELRECT);
		mouse_moved = 1;
		gw_redraw_win (gwp);
	}

	return 0;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_MOUSEWHEEL
*/

#if defined (WM_MOUSEWHEEL)

LRESULT gwclient_WM_MOUSEWHEEL (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	short        delta = HIWORD (wParam);
	
	delta = (delta / WHEEL_DELTA) * 4;
	if (delta > 0)
		eventscroll ((GUIWIN *)gwp, SCROLL_BACKLN, delta, 0L);
	else
		eventscroll ((GUIWIN *)gwp, SCROLL_FWDLN, -delta, 0L);
	gw_redraw_win (gwp);

	return 0;
}

#endif

/* --------------------------------------------------------------------
**
** gwclient_WM_PAINT
*/

LRESULT gwclient_WM_PAINT (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	PAINTSTRUCT ps;
	HBRUSH      brush;
	int         left;
	int         top;

	BeginPaint (gwp->clientHWnd, &ps);
#ifdef FEATURE_IMAGE
	if (normalimage && gwp->bg == (COLORREF)colorinfo[COLOR_FONT_NORMAL].bg)
	{
		gw_erase_rect(ps.hdc, &ps.rcPaint, normalimage, gwp->scrolled);
	}
	else if (idleimage && gwp->bg == (COLORREF)colorinfo[COLOR_FONT_IDLE].bg)
	{
		gw_erase_rect(ps.hdc, &ps.rcPaint, idleimage, gwp->scrolled);
	}
	else
#endif
	{
		brush = CreateSolidBrush (gwp->bg);
		FillRect (ps.hdc, &ps.rcPaint, brush);
		DeleteObject (brush);
	}
	if (gwp->active) {
		SetMapMode (ps.hdc, MM_TEXT);
		if ((left = ps.rcPaint.left - gwp->xcsize / 2) < 0)
			left = 0;
		left = left / gwp->xcsize;
		top = ps.rcPaint.top / gwp->ycsize;
		if (left < gwp->numcols && top < gwp->numrows )
			eventexpose ((GUIWIN *)gwp, top, left,
						 (ps.rcPaint.bottom + gwp->ycsize) / gwp->ycsize,
						 (ps.rcPaint.right + gwp->xcsize) / gwp->xcsize);
	}
	EndPaint (gwp->clientHWnd, &ps);

	return 0;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_RBUTTONDBLCLK
*/

LRESULT gwclient_WM_RBUTTONDBLCLK (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	int     row = HIWORD (lParam) / gwp->ycsize;
	int     col = (LOWORD (lParam) - gwp->xcsize / 2) / gwp->xcsize;

	eventclick ((GUIWIN *)gwp, row, col, CLICK_MOVE);
	eventclick ((GUIWIN *)gwp, row, col, CLICK_UNTAG);
	dblclick = 1;
	mouse_selection = MOUSE_SEL_NONE;
	gw_redraw_win (gwp);

	return 0;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_RBUTTONDOWN
*/

LRESULT gwclient_WM_RBUTTONDOWN (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	WINDOW      pwin = winofgw ((GUIWIN *)gwp);

	mouse_down = 1;
	if (pwin->seltop == NULL) {
		mouse_moved = 0;
		mouse_selection = MOUSE_SEL_RECT;
		mouse_init_row = HIWORD (lParam) / gwp->ycsize;
		if (LOWORD (lParam) > (gwp->xcsize / 2))
			mouse_init_col = (LOWORD (lParam) - gwp->xcsize / 2) / gwp->xcsize;
		else
			mouse_init_col = 0;
	}
	dblclick = 0;
	gw_redraw_win (gwp);

	return 0;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_RBUTTONUP
*/

LRESULT gwclient_WM_RBUTTONUP (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	WINDOW      pwin = winofgw ((GUIWIN *)gwp);
	int         row = HIWORD (lParam) / gwp->ycsize;
	int         col = (LOWORD (lParam) - gwp->xcsize / 2) / gwp->xcsize;
    
	mouse_down = 0;
	if (dblclick == 0) {
		if (pwin->seltop == NULL) {
			eventclick ((GUIWIN *)gwp, -1, -1, CLICK_CANCEL);
			mouse_moved = 0;
			mouse_selection = MOUSE_SEL_NONE;
			mouse_init_row = 0;
			mouse_init_col = 0;
		}
		eventclick ((GUIWIN *)gwp, row, col, CLICK_MOVE);
	}
	dblclick = 0;
	gw_redraw_win (gwp);

	return 0;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_SETFOCUS
*/

LRESULT gwclient_WM_SETFOCUS (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	dblclick = 0;
	state_shift = GetKeyState (VK_SHIFT) < 0;
	state_ctrl = GetKeyState (VK_CONTROL) < 0;

	if (gwp->active) {
		gw_upd_status_bar_ind (gwp, GetKeyState (VK_NUMLOCK) & 1,
			   GetKeyState (VK_CAPITAL) & 1);
		gwp->cursor_type = eventfocus ((GUIWIN *)gwp, ElvTrue);
	}

	if (gwp->caret) {
		HideCaret (gwp->clientHWnd);
		DestroyCaret ();
	}
	gw_set_cursor(gwp, TRUE);
	ShowCaret (gwp->clientHWnd);

	gwp->caret = 1;
	gw_redraw_win (gwp);

	return 0;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_SIZE
*/

LRESULT gwclient_WM_SIZE (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	int     oldrows;
	int     oldcols;

	if (gwp == NULL)
		return 1;

	if (gwp->active) {
		oldrows = gwp->numrows;
		oldcols = gwp->numcols;
		gw_get_win_size (gwp);
		if (oldrows != gwp->numrows || oldcols != gwp->numcols) {
			eventresize ((GUIWIN *)gwp, gwp->numrows, gwp->numcols);
			gw_redraw_win (gwp);
		}
	}

	return 0;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_SYSKEYDOWN
*/

LRESULT gwclient_WM_SYSKEYDOWN (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	unsigned char       chr[3];
	int                 i;

	switch (wParam) {
		case VK_F1:
		case VK_F2:
		case VK_F3:
		case VK_F4:
		case VK_F5:
		case VK_F6:
		case VK_F7:
		case VK_F8:
		case VK_F9:
		case VK_F10:
		case VK_F11:
		case VK_F12:
			chr[0] = (unsigned char)'\xFF';
			chr[1] = 'A';
			chr[2] = (unsigned char)wParam;
			for (i = lParam & 0xFFFF; i > 0; i--)
				eventkeys ((GUIWIN *)gwp, chr, 3);
			gw_redraw_win (gwp);
			return 0;
	}

	return 1;
}

/* --------------------------------------------------------------------
**
** gwclient_WM_VSCROLL
*/

LRESULT gwclient_WM_VSCROLL (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam)

{
	long    nPos = HIWORD (wParam);
	
	switch (LOWORD (wParam)) {
		case SB_LINEDOWN:
			eventscroll ((GUIWIN *)gwp, SCROLL_FWDLN, 1L, 0L);
			break;
		case SB_LINEUP:
			eventscroll ((GUIWIN *)gwp, SCROLL_BACKLN, 1L, 0L);
			break;
		case SB_PAGEDOWN:
			eventscroll ((GUIWIN *)gwp, SCROLL_FWDSCR, 1L, 0L);
			break;
		case SB_PAGEUP:
			eventscroll ((GUIWIN *)gwp, SCROLL_BACKSCR, 1L, 0L);
			break;
		case SB_THUMBTRACK:
#if 0
			if (nPos == 0)
				nPos = 1;
#endif
			eventscroll ((GUIWIN *)gwp, SCROLL_PERCENT, nPos, gwp->scrollsize);
			break; 
	}
	gw_redraw_win (gwp);
	
	return 0;
}

#endif
/* ex:se sw=4 smarttab: */
