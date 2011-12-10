/*
** wintags.c  --  windows interface for the ctags program. 
**
** Copyright 1996, Serge Pirotte.
*/

#define CHAR    Char
#include "elvis.h" 
#undef CHAR
#undef ELVBOOL 

#if defined (GUI_WIN32)

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "wintools.h"
#include "wintags.h"

static HWND     tagsDlgWnd;
static char     *cmdLineP;

extern int main (int argc, char **argv);

static void get_messages (void);
static LRESULT CALLBACK TagsDlgProc (HWND hDlg, UINT message,
                                     WPARAM wParam, LPARAM lParam);

/* ---[ WinMain ]------------------------------------------------------ */

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow)

{
    /* parse the command line */
    cmdLineP = lpCmdLine;
    parse_cmd_line (lpCmdLine);

    /* create the dialog box */
    tagsDlgWnd = CreateDialog (hInstance, MAKEINTRESOURCE (IDD_TAGS),
                               NULL, (DLGPROC)TagsDlgProc);
    center_window (NULL, tagsDlgWnd);
    ShowWindow (tagsDlgWnd, SW_SHOW);
    get_messages ();

    /* call ctags */
    main (cmd_line_argc, cmd_line_argv);

    /* terminate dialog */
    DestroyWindow (tagsDlgWnd);
    get_messages ();
    return 0;
}

/* ---[ get_messages ]------------------------------------------------- */

static void get_messages (void)

{
    MSG     msg;

    while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
    }
}

/* ---[ set_current_file ]--------------------------------------------- */

void set_current_file (char *file_name)

{
    SetDlgItemText (tagsDlgWnd, ID_CUR_FILE, file_name);

    /* schedule messages */
    get_messages ();
}

/* ---[ set_current_tags ]--------------------------------------------- */

void set_current_tags (int num_tags)

{
    char        msg[20];

    sprintf (msg, "%d", num_tags);
    SetDlgItemText (tagsDlgWnd, ID_CUR_TAGS, msg);

    /* schedule messages */
    get_messages ();
}

/* ---[ set_total_tags ]----------------------------------------------- */

void set_total_tags (int num_tags)

{
    char        msg[20];

    sprintf (msg, "%d", num_tags);
    SetDlgItemText (tagsDlgWnd, ID_TOTAL_TAGS, msg);

    /* schedule messages */
    get_messages ();
}

/* ---[ TagsDlgProc ]-------------------------------------------------- */

static LRESULT CALLBACK TagsDlgProc (HWND hDlg, UINT message,
                                     WPARAM wParam, LPARAM lParam)

{
    char        msg[256];

    switch (message) {
        case WM_INITDIALOG:
            sprintf (msg, "WinTags %s", cmdLineP); 
            SetWindowText (hDlg, msg);
            return TRUE;

        case WM_COMMAND:
			if (LOWORD (wParam) == IDCANCEL) {
				DestroyWindow (tagsDlgWnd);
				get_messages ();
			    exit (1);
			}
			break;
    }

    return FALSE;
}

#endif
/* ex:se ts=4 sw=4: */
