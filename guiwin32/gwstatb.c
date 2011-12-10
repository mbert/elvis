/*
** gwstatb.c  --  status bar support for elvis.
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

#define SB_PART_POS         1
#define SB_PART_CHANGED     2
#define SB_PART_MODE        3
#define SB_PART_NUM         4
#define SB_PART_CAP         5
#define SB_PART_TIME        6

static int      size_pos;
static int      size_changed;
static int      size_mode;
static int      size_num;
static int      size_cap;
static int      size_time;

static int      cxborder;
static int      cyborder;

/* --------------------------------------------------------------------
**
** gw_status_upd_time  --  update time on status bar.
*/

static void gw_status_upd_time (HWND hwnd)

{
    SYSTEMTIME      sys_time;
    char            str[10];
    int             hour;

    GetLocalTime (&sys_time);
    if (sys_time.wHour == 0)
        hour = 12;
	else if (sys_time.wHour > 12)
	    hour = sys_time.wHour - 12;
	else
	    hour = sys_time.wHour;
    sprintf (str, "%d:%2.2d %s", hour, sys_time.wMinute,
             sys_time.wHour < 12 ? "AM" : "PM");
    SendMessage (hwnd, SB_SETTEXT, (WPARAM)SB_PART_TIME, (LPARAM)str);
}

/* --------------------------------------------------------------------
**
** gw_set_panes  --  set the panes' positions.
*/

static void gw_set_panes (GUI_WINDOW *gwp, int maximized)

{
    int         *parts;
    RECT        rect;
    int         borders[3];

    /* retrieve status bar size */
    SendMessage (gwp->statusbarHWnd, SB_GETBORDERS, (WPARAM)0, (LPARAM)&borders);
    GetClientRect (gwp->statusbarHWnd, &rect);
    if (!maximized)
		rect.right -= GetSystemMetrics (SM_CXVSCROLL);

    /* set the parts */
    parts = (int *)LocalAlloc (LPTR, sizeof (int) * 7);
    parts[6] = rect.right - borders[2];
    parts[5] = parts[6] - size_time - borders[2];
    parts[4] = parts[5] - size_cap - borders[2];
    parts[3] = parts[4] - size_num - borders[2];
    parts[2] = parts[3] - size_mode - borders[2];
    parts[1] = parts[2] - size_changed - borders[2];
    parts[0] = parts[1] - size_pos - borders[2];
    SendMessage (gwp->statusbarHWnd, SB_SETPARTS, (WPARAM)7, (LPARAM)parts);
    LocalFree (parts);
    SendMessage (gwp->statusbarHWnd, SB_SETTEXT, (WPARAM)SBT_NOBORDERS, (LPARAM)0);

    /* update time */
    gw_status_upd_time (gwp->statusbarHWnd);

    /* update key indicators */
    gw_upd_status_bar_ind (gwp, GetKeyState (VK_NUMLOCK) & 1,
                           GetKeyState (VK_CAPITAL) & 1);
}

/* --------------------------------------------------------------------
**
** gw_status_timer_proc  --  timer processing.
*/

static void CALLBACK gw_status_timer_proc (HWND hwnd, UINT uMsg,
                                           UINT idEvent, DWORD dwTime)

{
    /* update time */
    gw_status_upd_time (hwnd);
}

/* --------------------------------------------------------------------
**
** gw_create_status_bar  --  create a new status bar.
*/

void gw_create_status_bar (GUI_WINDOW *gwp)

{
    HDC     dc;
    SIZE    size;

    /* check that we can do it. */
    if (!o_statusbar (gwp))
        return;
    if (gwp->statusbarHWnd != NULL)
        return;

    /* get borders size */
    cxborder = GetSystemMetrics (SM_CXBORDER);
    cyborder = GetSystemMetrics (SM_CYBORDER);

    /* create the status bar */
    gwp->statusbarHWnd = CreateStatusWindow (WS_CHILD | WS_VISIBLE,
                                             NULL,
                                             gwp->frameHWnd,
                                             1);

    /* get text extents for the various parts */
    dc = GetDC (gwp->statusbarHWnd);
    GetTextExtentPoint32 (dc, " 0000000 ", 9, &size);
    size_pos = size.cx;
    GetTextExtentPoint32 (dc, "MOD", 3, &size);
    size_changed = size.cx;
    GetTextExtentPoint32 (dc, " Command ", 9, &size);
    size_mode = size.cx;
    GetTextExtentPoint32 (dc, " CAPS ", 6, &size);
    size_cap = size.cx;
    GetTextExtentPoint32 (dc, " NUM ", 5, &size);
    size_num = size.cx;
    GetTextExtentPoint32 (dc, " 00:00PM ", 9, &size);
    size_time = size.cx;
    ReleaseDC (gwp->statusbarHWnd, dc);

    /* set the panes positions */
    gw_set_panes (gwp, 0);

    /* start the timer */
    SetTimer (gwp->statusbarHWnd, 1, 5000, (TIMERPROC)gw_status_timer_proc);
}

/* --------------------------------------------------------------------
**
** gw_destroy_status_bar  --  destroy the status bar.
*/

extern void gw_destroy_status_bar (GUI_WINDOW *gwp)

{
    /* check that we can do it. */
    if (gwp->statusbarHWnd == NULL)
        return;

    /* kill the timer */
    KillTimer (gwp->statusbarHWnd, 1);

    /* destroy the window */
    DestroyWindow (gwp->statusbarHWnd);
    gwp->statusbarHWnd = NULL;
}

/* --------------------------------------------------------------------
**
** gw_size_status_bar  --  resize the status bar.
*/

int gw_size_status_bar (GUI_WINDOW *gwp, RECT *parentRect, int maximized)

{
    RECT        rect;

    /* check that we can do it. */
    if (gwp->statusbarHWnd == NULL)
        return 1;

    /* resize the status bar */
    GetClientRect (gwp->statusbarHWnd, &rect);
    SetWindowPos (gwp->statusbarHWnd, NULL, parentRect->left + cxborder,
                  parentRect->bottom - rect.bottom - 2 * cyborder,
                  parentRect->right - parentRect->left - 2 * cxborder,
                  rect.bottom, SWP_NOZORDER);

    /* set the panes positions */
    gw_set_panes (gwp, maximized);

    return rect.bottom + 1;
}

/* --------------------------------------------------------------------
**
** gw_upd_status_bar  --  update status bar panes.
*/

void gw_upd_status_bar (GUI_WINDOW *gwp, Char *cmd, long row, long col, char learn, char *mode)

{
    char    tmp[20];

    /* check that we can do it. */
    if (gwp->statusbarHWnd == NULL)
        return;

    SendMessage (gwp->statusbarHWnd, SB_SETTEXT, (WPARAM)SBT_NOBORDERS, (LPARAM)cmd);
    sprintf (tmp, "%ld,%ld", row, col);
    SendMessage (gwp->statusbarHWnd, SB_SETTEXT, (WPARAM)SB_PART_POS, (LPARAM)&tmp);
    switch (learn)
    {
      case ',':	strcpy(tmp, "   ");	break;
      case '*': strcpy(tmp, "MOD");	break;
      default:	sprintf(tmp, " %c ", learn);
    }
    SendMessage (gwp->statusbarHWnd, SB_SETTEXT, (WPARAM)SB_PART_CHANGED, (LPARAM)&tmp);
    SendMessage (gwp->statusbarHWnd, SB_SETTEXT, (WPARAM)SB_PART_MODE, (LPARAM)mode);
}

/* --------------------------------------------------------------------
**
** gw_upd_status_bar_ind  --  update key indicators.
*/

void gw_upd_status_bar_ind (GUI_WINDOW *gwp, int num_state, int cap_state)

{
    /* check that we can do it. */
    if (gwp->statusbarHWnd == NULL)
        return;

    SendMessage (gwp->statusbarHWnd, SB_SETTEXT, (WPARAM)SB_PART_NUM,
                 (LPARAM)(num_state ? "NUM" : NULL));
    SendMessage (gwp->statusbarHWnd, SB_SETTEXT, (WPARAM)SB_PART_CAP,
                 (LPARAM)(cap_state ? "CAPS" : NULL));
}

/* --------------------------------------------------------------------
**
** gw_status_bar_text  --  display text on 1st pane.
*/

void gw_status_bar_text (GUI_WINDOW *gwp, char *str)

{
    /* check that we can do it. */
    if (gwp->statusbarHWnd == NULL)
        return;

    SendMessage (gwp->statusbarHWnd, SB_SETTEXT, (WPARAM)SBT_NOBORDERS, (LPARAM)str);
}

#endif
/* ex:se ts=4 sw=4: */
