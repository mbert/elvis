/*
** gwtoolb.c  --  toolbar support for elvis.
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

static TBBUTTON buttons[] = {
    { 0, IDM_FILE_NEW,       TBSTATE_ENABLED, TBSTYLE_BUTTON },
    { 1, IDM_FILE_OPEN,      TBSTATE_ENABLED, TBSTYLE_BUTTON },
    { 2, IDM_FILE_SAVE,      TBSTATE_ENABLED, TBSTYLE_BUTTON },
    { 0, 0,                  TBSTATE_ENABLED, TBSTYLE_SEP },
    { 3, IDM_EDIT_CUT,       TBSTATE_ENABLED, TBSTYLE_BUTTON },
    { 4, IDM_EDIT_COPY,      TBSTATE_ENABLED, TBSTYLE_BUTTON },
    { 5, IDM_EDIT_PASTE,     TBSTATE_ENABLED, TBSTYLE_BUTTON },
    { 0, 0,                  TBSTATE_ENABLED, TBSTYLE_SEP },
    { 6, IDM_EDIT_UNDO,      TBSTATE_ENABLED, TBSTYLE_BUTTON },
    { 7, IDM_EDIT_REDO,      TBSTATE_ENABLED, TBSTYLE_BUTTON },
    { 0, 0,                  TBSTATE_ENABLED, TBSTYLE_SEP },
    { 8, IDM_WINDOW_NFILE,   TBSTATE_ENABLED, TBSTYLE_BUTTON },
    { 9, IDM_WINDOW_PFILE,   TBSTATE_ENABLED, TBSTYLE_BUTTON },
    { 0, 0,                  TBSTATE_ENABLED, TBSTYLE_SEP },
    {10, IDM_WINDOW_NEW,     TBSTATE_ENABLED, TBSTYLE_BUTTON },
    {11, IDM_WINDOW_NEXT,    TBSTATE_ENABLED, TBSTYLE_BUTTON },
    {12, IDM_WINDOW_PREVIOUS,TBSTATE_ENABLED, TBSTYLE_BUTTON },
    { 0, 0,                  TBSTATE_ENABLED, TBSTYLE_SEP },
    {13, IDM_FILE_PRINT,     TBSTATE_ENABLED, TBSTYLE_BUTTON },
    { 0, 0,                  TBSTATE_ENABLED, TBSTYLE_SEP },
    {14, IDM_HELP_INDEX,     TBSTATE_ENABLED, TBSTYLE_BUTTON },
};

static int              cxborder;
static int              cyborder;

/* --------------------------------------------------------------------
**
** gw_create_toolbar  --  create the application toolbar.
*/

void gw_create_toolbar (GUI_WINDOW *gwp)

{
    RECT        rect;
#if _MSC_VER <= 900
    int         i;
#endif
    
    /* check that we can do it. */
    if (!o_toolbar (gwp))
        return;
    if (gwp->toolbarHWnd != NULL)
        return;

    /* get borders size */
    cxborder = GetSystemMetrics (SM_CXBORDER);
    cyborder = GetSystemMetrics (SM_CYBORDER);

    /* create the toolbar */
#if _MSC_VER > 900
    gwp->toolbarHWnd = CreateToolbarEx (gwp->frameHWnd,
                                       WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS,
                                       1, 15, hInst, IDT_ELVIS,
                                       buttons, 21, 16, 15, 15, 14,
                                       sizeof (TBBUTTON));
#else
    gwp->toolbarHWnd = CreateToolbar (gwp->frameHWnd,
                                     WS_CHILD | WS_VISIBLE,
                                     1, 15, hInst, IDT_ELVIS,
                                     buttons, 1);
    for (i = 1; i < 21; i++)
        SendMessage (gwp->toolbarHWnd, TB_ADDBUTTONS, 1, (LPARAM)&buttons[i]);
#endif

    /* size the toolbar */
    GetClientRect (gwp->frameHWnd, &rect);
    gw_size_toolbar (gwp, &rect);
}

/* --------------------------------------------------------------------
**
** gw_destroy_toolbar  --  destroy the toolbar.
*/

void gw_destroy_toolbar (GUI_WINDOW *gwp)

{
    /* check that we have a toolbar */
    if (gwp->toolbarHWnd == NULL)
        return;

    /* destroy the window */
    DestroyWindow (gwp->toolbarHWnd);
    gwp->toolbarHWnd = NULL;
}

/* --------------------------------------------------------------------
**
** gw_size_toolbar  --  resize the toolbar.
*/

int gw_size_toolbar (GUI_WINDOW *gwp, RECT *parentRect)

{
    RECT        rect;

    /* check that the toolbar exist */
    if (gwp->toolbarHWnd == NULL)
        return 0;

    /* resize the toolbar */
    GetClientRect (gwp->toolbarHWnd, &rect);
    SetWindowPos (gwp->toolbarHWnd, NULL, parentRect->left + cxborder,
                  parentRect->top + cyborder,
                  parentRect->right - parentRect->left - 2 * cxborder,
                  rect.bottom + 2 * cyborder, SWP_NOZORDER);

    return rect.bottom + 2 * cyborder + 1;
}

/* --------------------------------------------------------------------
**
** gw_toolbar_tooltip  --  get tooltip text.
*/

#if _MSC_VER > 900
void gw_toolbar_tooltip (GUI_WINDOW *gwp, LPARAM lParam)

{
    LPTOOLTIPTEXT   lptext;
    static char     buffer[80];
    char            *p;

    lptext = (LPTOOLTIPTEXT)lParam;
    if (lptext->hdr.code == TTN_NEEDTEXT) {
        LoadString (hInst, (UINT)lptext->hdr.idFrom, buffer, sizeof (buffer));
        if ((p = strchr (buffer, '\n')) != NULL)
            p++;
        lptext->lpszText = p;
    }
}
#endif

#endif
/* ex:se ts=4 sw=4: */
