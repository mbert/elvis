/*
** wintools.c  --  various windows tools.
**
** Copyright 1996, Serge Pirotte.
*/

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "wintools.h"

#if defined (GUI_WIN32)

int         cmd_line_argc = 0;
char        **cmd_line_argv;

static char exe_name[_MAX_PATH];
static int  num_alloc = 0;
static char *cmd_line;

/* ---[ parse_cmd_line ]----------------------------------------------- */

void parse_cmd_line (char *cmdp)

{
    register char       *chp;

    /* allocate argv[] */
    num_alloc = 16;
    if ((cmd_line_argv = malloc (num_alloc * sizeof (char *))) == NULL)
        return;

    /* get program name */
   GetModuleFileName (NULL, exe_name, sizeof (exe_name));
   cmd_line_argv[0] = exe_name;
   cmd_line_argc = 1;

    /* if no command line, return */
    if (cmdp == NULL)
        return;

	/* make a copy of the command line */
	if ((cmd_line = malloc (strlen (cmdp) + 1)) == NULL)
	    return;
	strcpy (cmd_line, cmdp);

	/* parse the command line */
	chp = cmd_line;
	while (*chp != '\0') {
	    if (cmd_line_argc >= num_alloc) {
	        num_alloc += 16;
	        if ((cmd_line_argv = realloc (cmd_line_argv, num_alloc * sizeof (char *))) == NULL)
	            return;
	    }
	    if (*chp == '"') {
			cmd_line_argv[cmd_line_argc++] = chp + 1;
	        while (*++chp != '"')
	            if (*chp == '\0')
	                return;
		}
	    else {
			cmd_line_argv[cmd_line_argc++] = chp;
			while (*++chp != ' ')
				if (*chp == '\0')
					return;
		}
		*chp = '\0';
		while (*++chp == ' ')
		    ;
	}
}

/* ---[ center_window ]------------------------------------------------ */

void center_window (HWND parent_window, HWND this_window)

{
    RECT        rChild;
    RECT        rParent;
    RECT        rWorkArea;
    int         wChild;
    int         hChild;
    int         wParent;
    int         hParent;
    int         xNew;
    int         yNew;

    /* get height and width of child window */
    GetWindowRect (this_window, &rChild);
    wChild = rChild.right - rChild.left;
    hChild = rChild.bottom - rChild.top;

    /* get workarea limits */
    rWorkArea.left = rWorkArea.top = 0;
    rWorkArea.right = GetSystemMetrics (SM_CXSCREEN);
    rWorkArea.bottom = GetSystemMetrics (SM_CYSCREEN);

    /* get height and width of child window */
    if (parent_window == NULL)
        rParent = rWorkArea;
    else
        GetWindowRect (parent_window, &rParent);
    wParent = rParent.right - rParent.left;
    hParent = rParent.bottom - rParent.top;

    /* calculate new position */
	xNew = rParent.left + ((wParent - wChild) /2);
    if (xNew < rWorkArea.left)
        xNew = rWorkArea.left;
    else if ((xNew+wChild) > rWorkArea.right)
        xNew = rWorkArea.right - wChild;

    yNew = rParent.top  + ((hParent - hChild) /2);
    if (yNew < rWorkArea.top)
        yNew = rWorkArea.top;
	else if ((yNew+hChild) > rWorkArea.bottom)
        yNew = rWorkArea.bottom - hChild;

	/* set new position */
    SetWindowPos (this_window, NULL, xNew, yNew, 0, 0,
                  SWP_NOSIZE | SWP_NOZORDER);
}

#endif
/* ex:se ts=4 sw=4: */
