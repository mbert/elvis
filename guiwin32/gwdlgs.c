/*
** gwdlgs.c  --  dialogs management for MS-Windows elvis.
**
** Copyright 1996, Serge Pirotte.
*/

#define CHAR    Char
#include "elvis.h" 
#undef CHAR
#undef ELVBOOL 

#if defined (GUI_WIN32)

#include <windows.h>
#include <direct.h>
#include "winelvis.h"
#include "wintools.h"
#include "elvisres.h"

typedef struct {
	CHAR			*fg;	/* name of foreground color */
	CHAR			*bg;	/* name of background color */
	CHAR			*like;	/* name of "like" font */
	unsigned short	bits;	/* other attributes */
	unsigned short	orig;	/* original settings of other attributes */
	char			font;	/* font code that these attributes are fore */
} CINFO;

#define MAX_HISTORY_LIST        10

struct history_list {
	char        *elem[MAX_HISTORY_LIST];
};

char                *gw_new_buffer;
static GUI_WINDOW   *gwp = NULL;

static struct history_list      search_history_list;
static struct history_list      repl1_history_list;
static struct history_list      repl2_history_list;
static struct history_list      winapp_history_list;

/* --------------------------------------------------------------------
**
** gw_fill_cb  --  fill a combobox.
*/

static void gw_fill_cb (HWND hwnd, WORD id, struct history_list *hlp)

{
	register int    i;

	for (i = 0; i < MAX_HISTORY_LIST; i++) {
		if (hlp->elem[i] != NULL)
			SendDlgItemMessage (hwnd, id, CB_ADDSTRING, 0,
								(LPARAM)hlp->elem[i]);
	}

	SendDlgItemMessage (hwnd, id, CB_SETCURSEL, 0, 0);
}

/* --------------------------------------------------------------------
**
** gw_add_history  --  add a string to a history list, no duplicates.
*/

static void gw_add_history (struct history_list *hlp, char *str)

{
	register int    i;
	char            *new_str;

	for (i = 0; i < MAX_HISTORY_LIST; i++)
		if (hlp->elem[i] != NULL && strcmp (hlp->elem[i], str) == 0) {
			new_str = hlp->elem[i];
			while (i > 0) {
				hlp->elem[i] = hlp->elem[i - 1];
				i--;
			}
			hlp->elem[0] = new_str;
			return;
		}

	if (hlp->elem[MAX_HISTORY_LIST - 1] != NULL)
		safefree (hlp->elem[MAX_HISTORY_LIST - 1]);
	for (i = MAX_HISTORY_LIST - 1; i > 0; i--)
		hlp->elem[i] = hlp->elem[i - 1];
	hlp->elem[0] = CHARdup (str);
}

/* --------------------------------------------------------------------
**
** DlgSearch  --  Search for a string.
*/

BOOL CALLBACK DlgSearch (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)

{
	char            cmd[80];
	ELVBOOL         old_ignorecase = o_ignorecase;
	ELVBOOL         old_magic = o_magic;
	ELVBOOL         old_autoselect = o_autoselect;
	ELVBOOL         old_wrapscan = o_wrapscan;

	switch (msg) {
		case WM_INITDIALOG:
			gwp = gw_find_frame (GetParent (hwnd));
			center_window (gwp->clientHWnd, hwnd);
			SendDlgItemMessage (hwnd, IDSR_FORWARD, BM_SETCHECK, 1, 0);
			SendDlgItemMessage (hwnd, IDSR_IGNORECASE, BM_SETCHECK,
								o_ignorecase ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDSR_MAGIC, BM_SETCHECK,
								o_magic ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDSR_AUTOSELECT, BM_SETCHECK,
								o_autoselect ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDSR_WRAPSCAN, BM_SETCHECK,
								o_wrapscan ? 1 : 0, 0);
			gw_fill_cb (hwnd, IDSR_STRING, &search_history_list);
			return TRUE;

		case WM_COMMAND:
			if (LOWORD (wParam) == IDOK) {
				cmd[0] = SendDlgItemMessage (hwnd, IDSR_FORWARD, BM_GETCHECK,
											 0, 0) ? '/' : '?';
				o_ignorecase = SendDlgItemMessage (hwnd, IDSR_IGNORECASE,
								   BM_GETCHECK, 0, 0) ?  ElvTrue : ElvFalse;
				o_magic = SendDlgItemMessage (hwnd, IDSR_MAGIC,
							  BM_GETCHECK, 0, 0) ?  ElvTrue : ElvFalse;
				o_autoselect = SendDlgItemMessage (hwnd, IDSR_AUTOSELECT,
								   BM_GETCHECK, 0, 0) ?  ElvTrue : ElvFalse;
				o_wrapscan = SendDlgItemMessage (hwnd, IDSR_WRAPSCAN,
								 BM_GETCHECK, 0, 0) ?  ElvTrue : ElvFalse;
				SendDlgItemMessage (hwnd, IDSR_STRING, WM_GETTEXT,
									sizeof (cmd) - 2, (LPARAM)&cmd[1]);
				EndDialog (hwnd, TRUE);
				gw_add_history (&search_history_list, &cmd[1]);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				o_ignorecase = old_ignorecase;
				o_magic = old_magic;
				o_autoselect = old_autoselect;
				o_wrapscan = old_wrapscan;
				return TRUE;
			}
			else if (LOWORD (wParam) == IDCANCEL) {
				EndDialog (hwnd, TRUE);
				return TRUE;
			}
			break;
	}

	return FALSE;
}

/* --------------------------------------------------------------------
**
** DlgReplace  --  :substitute command.
*/

BOOL CALLBACK DlgReplace (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)

{
	char            cmd[80];
	char            str[20];
	int             index;
	ELVBOOL         old_ignorecase = o_ignorecase;
	ELVBOOL         old_magic = o_magic;
	ELVBOOL         old_edcompatible = o_edcompatible;
	ELVBOOL         old_gdefault = o_gdefault;

	switch (msg) {
		case WM_INITDIALOG:
			gwp = gw_find_frame (GetParent (hwnd));
			center_window (gwp->clientHWnd, hwnd);
			SendDlgItemMessage (hwnd, IDREP_FROM, WM_SETTEXT,
								0, (LPARAM)"1");
			SendDlgItemMessage (hwnd, IDREP_TO, WM_SETTEXT,
								0, (LPARAM)"$");
			SendDlgItemMessage (hwnd, IDREP_MAGIC, BM_SETCHECK,
								o_magic ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDREP_IGNORECASE, BM_SETCHECK,
								o_ignorecase ? 1 : 0, 0);
			gw_fill_cb (hwnd, IDREP_STR1, &repl1_history_list);
			gw_fill_cb (hwnd, IDREP_STR2, &repl2_history_list);
			return TRUE;

		case WM_COMMAND:
			if (LOWORD (wParam) == IDOK) {
				strcpy (cmd, ":");
				SendDlgItemMessage (hwnd, IDREP_FROM, WM_GETTEXT,
									sizeof (cmd), (LPARAM)&cmd[1]);
				SendDlgItemMessage (hwnd, IDREP_TO, WM_GETTEXT,
									sizeof (str), (LPARAM)str);
				if (strlen (str) != 0) {
					strcat (cmd, ",");
					strcat (cmd, str);
				}
				strcat (cmd, " s/");
				index = strlen (cmd);
				SendDlgItemMessage (hwnd, IDREP_STR1, WM_GETTEXT,
									sizeof (cmd) - index,
									(LPARAM)&cmd[index]);
				gw_add_history (&repl1_history_list, &cmd[index]);
				strcat (cmd, "/");
				index = strlen (cmd);
				SendDlgItemMessage (hwnd, IDREP_STR2, WM_GETTEXT,
									sizeof (cmd) - index,
									(LPARAM)&cmd[index]);
				gw_add_history (&repl2_history_list, &cmd[index]);
				strcat (cmd, "/");
				if (SendDlgItemMessage (hwnd, IDREP_G, BM_GETCHECK, 0, 0))
					strcat (cmd, "g");
				if (SendDlgItemMessage (hwnd, IDREP_P, BM_GETCHECK, 0, 0))
					strcat (cmd, "p");
				if (SendDlgItemMessage (hwnd, IDREP_X, BM_GETCHECK, 0, 0))
					strcat (cmd, "x");
				o_magic = SendDlgItemMessage (hwnd, IDREP_MAGIC,
							  BM_GETCHECK, 0, 0) ?  ElvTrue : ElvFalse;
				o_ignorecase = SendDlgItemMessage (hwnd, IDREP_IGNORECASE,
								   BM_GETCHECK, 0, 0) ?  ElvTrue : ElvFalse;
				EndDialog (hwnd, TRUE);
				o_edcompatible = ElvFalse;
				o_gdefault = ElvFalse;
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				o_ignorecase = old_ignorecase;
				o_magic = old_magic;
				o_edcompatible = old_edcompatible;
				o_gdefault = old_gdefault;
				return TRUE;
			}
			else if (LOWORD (wParam) == IDCANCEL) {
				EndDialog (hwnd, TRUE);
				return TRUE;
			}
			break;
	}

	return FALSE;
}

/* --------------------------------------------------------------------
**
** DlgGoto  --  Goto a specified line.
*/

BOOL CALLBACK DlgGoto (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)

{
	char            cmd[80];

	switch (msg) {
		case WM_INITDIALOG:
			gwp = gw_find_frame (GetParent (hwnd));
			center_window (gwp->clientHWnd, hwnd);
			return TRUE;

		case WM_COMMAND:
			if (LOWORD (wParam) == IDOK) {
				SendDlgItemMessage (hwnd, IDGT_LINE, WM_GETTEXT,
									sizeof (cmd) - 5, (LPARAM)&cmd[1]);
				EndDialog (hwnd, TRUE);
				cmd[0] = ':';
				strcat (cmd, " go");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				return TRUE;
			}
			else if (LOWORD (wParam) == IDCANCEL) {
				EndDialog (hwnd, TRUE);
				return TRUE;
			}
			break;
	}

	return FALSE;
}

/* --------------------------------------------------------------------
**
** DlgTags  --  run WinTags.
*/

BOOL CALLBACK DlgTags (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)

{
	char            cmd[256];

	switch (msg) {
		case WM_INITDIALOG:
			gwp = gw_find_frame (GetParent (hwnd));
			center_window (gwp->clientHWnd, hwnd);
			SendDlgItemMessage (hwnd, IDTAG_FILES, WM_SETTEXT,
								0, (LPARAM)"*.h *.c *.cpp");
			SendDlgItemMessage (hwnd, IDTAG_STATIC, BM_SETCHECK, 1, 0);
			SendDlgItemMessage (hwnd, IDTAG_TYPEDEFS, BM_SETCHECK, 1, 0);
			SendDlgItemMessage (hwnd, IDTAG_VARIABLE, BM_SETCHECK, 1, 0);
			return TRUE;

		case WM_COMMAND:
			if (LOWORD (wParam) == IDOK) {
				strcpy (cmd, "WinTags ");
				if (SendDlgItemMessage (hwnd, IDTAG_USEQUEST, BM_GETCHECK, 0, 0))
					strcat (cmd, "-B ");
				if (SendDlgItemMessage (hwnd, IDTAG_LINENUMBERS, BM_GETCHECK, 0, 0))
					strcat (cmd, "-N ");
				if (SendDlgItemMessage (hwnd, IDTAG_STATIC, BM_GETCHECK, 0, 0))
					strcat (cmd, "-s ");
				if (SendDlgItemMessage (hwnd, IDTAG_INLINE, BM_GETCHECK, 0, 0))
					strcat (cmd, "-i ");
				if (SendDlgItemMessage (hwnd, IDTAG_TYPEDEFS, BM_GETCHECK, 0, 0))
					strcat (cmd, "-t ");
				if (SendDlgItemMessage (hwnd, IDTAG_VARIABLE, BM_GETCHECK, 0, 0))
					strcat (cmd, "-v ");
				if (SendDlgItemMessage (hwnd, IDTAG_REFS, BM_GETCHECK, 0, 0))
					strcat (cmd, "-r ");
				if (SendDlgItemMessage (hwnd, IDTAG_APPEND, BM_GETCHECK, 0, 0))
					strcat (cmd, "-a ");
				SendDlgItemMessage (hwnd, IDTAG_FILES, WM_GETTEXT,
									sizeof (cmd) - strlen (cmd),
									(LPARAM)&cmd[strlen (cmd)]);
				EndDialog (hwnd, TRUE);
				WinExec (cmd, SW_SHOW);
				return TRUE;
			}
			else if (LOWORD (wParam) == IDCANCEL) {
				EndDialog (hwnd, TRUE);
				return TRUE;
			}
			break;
	}

	return FALSE;
}

/* --------------------------------------------------------------------
**
** DlgRun  --  Run a external program.
*/

BOOL CALLBACK DlgRun (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)

{
	char            cmd[256];

	switch (msg) {
		case WM_INITDIALOG:
			gwp = gw_find_frame (GetParent (hwnd));
			center_window (gwp->clientHWnd, hwnd);
			gw_fill_cb (hwnd, IDRUN_CMDLINE, &winapp_history_list);
			return TRUE;

		case WM_COMMAND:
			if (LOWORD (wParam) == IDOK) {
				SendDlgItemMessage (hwnd, IDRUN_CMDLINE, WM_GETTEXT,
									sizeof (cmd), (LPARAM)cmd);
				EndDialog (hwnd, TRUE);
				gw_add_history (&winapp_history_list, cmd);
				WinExec (cmd, SW_SHOW);
				return TRUE;
			}
			else if (LOWORD (wParam) == IDCANCEL) {
				EndDialog (hwnd, TRUE);
				return TRUE;
			}
			break;
	}

	return FALSE;
}

/* --------------------------------------------------------------------
**
** OptGuiHelper -- respond to selection of a different text type.
** "old" should be NULL the first time, then previous "new" thereafter.
*/
static void OptGuiHelper(HWND hwnd, CINFO *old, CINFO *new)
{
	int	i;
	CHAR	*name;
	CHAR	buf[100];

	/* Is this the first time? */
	if (!old)
	{
		/* Yes -- store color list in combo boxes */
		for (i = 0; colortbl[i].name; i++)
		{
			SendDlgItemMessage(hwnd, IDC_GUI_FOREGROUND, CB_ADDSTRING,
							   0, (LPARAM)colortbl[i].name);
			SendDlgItemMessage(hwnd, IDC_GUI_BACKGROUND, CB_ADDSTRING,
							   0, (LPARAM)colortbl[i].name);
		}
	}
	else
	{
		/* No -- parse color settings for old text type */

		/* foreground */
		SendDlgItemMessage(hwnd, IDC_GUI_FOREGROUND, WM_GETTEXT, sizeof buf, (LPARAM)buf);
		for (name = buf; *name == ' '; name++)
			;
		if (CHARcmp(old->fg ? old->fg : toCHAR(""), name))
			old->bits |= COLOR_SET;
		if (old->fg)
			safefree(old->fg);
		old->fg = *name ? CHARdup(name) : NULL;

		/* background */
		SendDlgItemMessage(hwnd, IDC_GUI_BACKGROUND, WM_GETTEXT, sizeof buf, (LPARAM)buf);
		for (name = buf; *name == ' '; name++)
			;
		if (CHARcmp(old->bg ? old->bg : toCHAR(""), name))
			old->bits |= COLOR_SET;
		if (old->bg)
			safefree(old->bg);
		old->bg = *name ? CHARdup(name) : NULL;
	}

	/* Set flags for new text type */
	SendDlgItemMessage (hwnd, IDC_GUI_BOLD, BM_SETCHECK,
			new->bits & COLOR_BOLD ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage (hwnd, IDC_GUI_ITALIC, BM_SETCHECK,
			new->bits & COLOR_ITALIC ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage (hwnd, IDC_GUI_UNDERLINED, BM_SETCHECK,
			new->bits & COLOR_UNDERLINED ? BST_CHECKED : BST_UNCHECKED,0);
	SendDlgItemMessage (hwnd, IDC_GUI_BOXED, BM_SETCHECK,
			new->bits & COLOR_BOXED ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage (hwnd, IDC_GUI_GRAPHIC, BM_SETCHECK,
			new->bits & COLOR_GRAPHIC ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage (hwnd, IDC_GUI_FIXED, BM_SETCHECK,
			(new->bits & (COLOR_PROP|COLOR_PROPSET)) == COLOR_PROPSET ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage (hwnd, IDC_GUI_PROPORTIONAL, BM_SETCHECK,
			((new->bits & (COLOR_PROP|COLOR_PROPSET)) == (COLOR_PROP|COLOR_PROPSET)) ? BST_CHECKED : BST_UNCHECKED, 0);

	/* set combo box colors for new text type */
	SendDlgItemMessage (hwnd, IDC_GUI_FOREGROUND, WM_SETTEXT,
						0, (LPARAM)(new->fg ? tochar8(new->fg) : ""));
	SendDlgItemMessage (hwnd, IDC_GUI_BACKGROUND, WM_SETTEXT,
						0, (LPARAM)(new->bg ? tochar8(new->bg) : ""));
}

/* --------------------------------------------------------------------
**
** DlgOptGui  --  gui specific options.
*/

BOOL CALLBACK DlgOptGui (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)

{
	static CINFO	cinfo[QTY(colorinfo)];
	static int		qtyelem;
	static int      curelem;
	register int    i;
	char            cmd[100];
	LRESULT         res;

	switch (msg) {
		case WM_INITDIALOG:
			gwp = gw_find_frame (GetParent (hwnd));
			center_window (gwp->clientHWnd, hwnd);
			SendDlgItemMessage (hwnd, IDC_GUI_MENUBAR, BM_SETCHECK,
				calctrue(optgetstr("menubar", NULL)) ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage (hwnd, IDC_GUI_TOOLBAR, BM_SETCHECK,
				calctrue(optgetstr("toolbar", NULL)) ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage (hwnd, IDC_GUI_SCROLLBAR, BM_SETCHECK,
				calctrue(optgetstr("scrollbar", NULL)) ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage (hwnd, IDC_GUI_STATUSBAR, BM_SETCHECK,
				calctrue(optgetstr("statusbar", NULL)) ? BST_CHECKED : BST_UNCHECKED, 0);
			qtyelem = colornpermanent - 1;
			for (i = 0; i < qtyelem; i++) {
				cinfo[i].font = colorsortorder[i];
				colorparse(colorinfo[cinfo[i].font].descr, &cinfo[i].fg,
								&cinfo[i].bg, &cinfo[i].like, &cinfo[i].bits);
				cinfo[i].orig = cinfo[i].orig;
				SendDlgItemMessage (hwnd, IDC_GUI_FONTS, LB_ADDSTRING,
								0, (LPARAM)colorinfo[cinfo[i].font].name);
			}
			curelem = (int)SendDlgItemMessage (hwnd, IDC_GUI_FONTS, LB_SELECTSTRING,
											   (WPARAM)-1, (LPARAM)"normal");
			OptGuiHelper(hwnd, NULL, &cinfo[curelem]);
			return TRUE;

		case WM_COMMAND:
			if (LOWORD (wParam) == IDC_GUI_FONTS &&
				HIWORD (wParam) == LBN_SELCHANGE) {
				i = curelem;
				curelem = (int)SendDlgItemMessage (hwnd, IDC_GUI_FONTS, LB_GETCURSEL,
													0, 0);
				OptGuiHelper(hwnd, &cinfo[i], &cinfo[curelem]);
			}
			else if (LOWORD (wParam) == IDC_GUI_BOLD &&
					 HIWORD (wParam) == BN_CLICKED) {
				if (SendDlgItemMessage (hwnd, IDC_GUI_BOLD, BM_GETCHECK, 0, 0) == BST_CHECKED)
					cinfo[curelem].bits |= COLOR_BOLD;
				else
					cinfo[curelem].bits &= ~COLOR_BOLD;
			}
			else if (LOWORD (wParam) == IDC_GUI_ITALIC &&
					 HIWORD (wParam) == BN_CLICKED) {
				if (SendDlgItemMessage (hwnd, IDC_GUI_ITALIC, BM_GETCHECK, 0, 0) == BST_CHECKED)
					cinfo[curelem].bits |= COLOR_ITALIC;
				else
					cinfo[curelem].bits &= ~COLOR_ITALIC;
			}
			else if (LOWORD (wParam) == IDC_GUI_UNDERLINED &&
					 HIWORD (wParam) == BN_CLICKED) {
				if (SendDlgItemMessage (hwnd, IDC_GUI_UNDERLINED, BM_GETCHECK, 0, 0) == BST_CHECKED)
					cinfo[curelem].bits |= COLOR_UNDERLINED;
				else
					cinfo[curelem].bits &= ~COLOR_UNDERLINED;
			}
			else if (LOWORD (wParam) == IDC_GUI_BOXED &&
					 HIWORD (wParam) == BN_CLICKED) {
				if (SendDlgItemMessage (hwnd, IDC_GUI_BOXED, BM_GETCHECK, 0, 0) == BST_CHECKED)
					cinfo[curelem].bits |= COLOR_BOXED;
				else
					cinfo[curelem].bits &= ~COLOR_BOXED;
			}
			else if (LOWORD (wParam) == IDC_GUI_GRAPHIC &&
					 HIWORD (wParam) == BN_CLICKED) {
				if (SendDlgItemMessage (hwnd, IDC_GUI_GRAPHIC, BM_GETCHECK, 0, 0) == BST_CHECKED)
					cinfo[curelem].bits |= COLOR_GRAPHIC;
				else
					cinfo[curelem].bits &= ~COLOR_GRAPHIC;
			}
			else if (LOWORD (wParam) == IDC_GUI_FIXED &&
					 HIWORD (wParam) == BN_CLICKED) {
				if (SendDlgItemMessage (hwnd, IDC_GUI_FIXED, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					cinfo[curelem].bits &= ~COLOR_PROP;
					cinfo[curelem].bits |= COLOR_PROPSET;
					SendDlgItemMessage (hwnd, IDC_GUI_PROPORTIONAL, BM_SETCHECK,
							BST_UNCHECKED, 0);
				} else {
					if (SendDlgItemMessage (hwnd, IDC_GUI_PROPORTIONAL, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
						cinfo[curelem].bits &= ~COLOR_PROPSET;
				}
			}
			else if (LOWORD (wParam) == IDC_GUI_PROPORTIONAL &&
					 HIWORD (wParam) == BN_CLICKED) {
				if (SendDlgItemMessage (hwnd, IDC_GUI_PROPORTIONAL, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					cinfo[curelem].bits &= ~COLOR_PROP;
					cinfo[curelem].bits |= COLOR_PROP|COLOR_PROPSET;
					SendDlgItemMessage (hwnd, IDC_GUI_FIXED, BM_SETCHECK,
							BST_UNCHECKED, 0);
				} else {
					if (SendDlgItemMessage (hwnd, IDC_GUI_FIXED, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
						cinfo[curelem].bits &= ~(COLOR_PROP|COLOR_PROPSET);
				}
			}
			else if (LOWORD (wParam) == IDOK) {
				/* switch to the same color, just so colors are parsed */
				OptGuiHelper(hwnd, &cinfo[curelem], &cinfo[curelem]);

				/* close the dialog */
				EndDialog (hwnd, TRUE);

				/* fetch the window options */
				res = SendDlgItemMessage (hwnd, IDC_GUI_MENUBAR, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %smenubar", res == BST_CHECKED ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);

				res = SendDlgItemMessage (hwnd, IDC_GUI_TOOLBAR, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %stoolbar", res == BST_CHECKED ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);

				res = SendDlgItemMessage (hwnd, IDC_GUI_SCROLLBAR, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sscrollbar", res == BST_CHECKED ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);

				res = SendDlgItemMessage (hwnd, IDC_GUI_STATUSBAR, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sstatusbar", res == BST_CHECKED ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);

				/* Update the attributes of each text type */
				for (i = 0; i < qtyelem; i++) {
					if ((cinfo[i].bits & COLOR_SET)
					 || ((cinfo[i].bits ^ cinfo[i].orig)
				 	    & (COLOR_BOLD|COLOR_ITALIC|COLOR_UNDERLINED|COLOR_BOXED
							|COLOR_GRAPHIC|COLOR_PROP|COLOR_PROPSET)))
					{
						sprintf (cmd, "color %s %s%s%s%s%s%s",
								 colorinfo[cinfo[i].font].name,
								 cinfo[i].bits & COLOR_BOLD ? "bold " : "",
								 cinfo[i].bits & COLOR_ITALIC ? "italic " : "",
								 cinfo[i].bits & COLOR_UNDERLINED ?"underlined ":"",
								 cinfo[i].bits & COLOR_BOXED ? "boxed " : "",
								 cinfo[i].bits & COLOR_GRAPHIC ? "graphic " : "",
								 cinfo[i].bits & COLOR_PROPSET
									?  (cinfo[i].bits & COLOR_PROP
										? "proportional "
										: "fixed ")
									: "",
								 cinfo[i].fg ? cinfo[i].fg : "");
						if (cinfo[i].bg)
							sprintf(cmd + strlen(cmd), " on %s", cinfo[i].bg);
						eventex ((GUIWIN *)gwp, cmd, ElvFalse);
					}
					if (cinfo[i].fg) safefree(cinfo[i].fg);
					if (cinfo[i].bg) safefree(cinfo[i].bg);
					if (cinfo[i].like) safefree(cinfo[i].like);
				}

				return TRUE;
			}
			else if (LOWORD (wParam) == IDCANCEL) {
				EndDialog (hwnd, TRUE);
				for (i = 0; i < qtyelem; i++) {
					if (cinfo[i].fg) safefree(cinfo[i].fg);
					if (cinfo[i].bg) safefree(cinfo[i].bg);
					if (cinfo[i].like) safefree(cinfo[i].like);
				}
				return TRUE;
			}
			break;
	}
	return FALSE;
}

/* --------------------------------------------------------------------
**
** DlgOptBuffer  --  buffer specific options.
*/

BOOL CALLBACK DlgOptBuffer (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)

{
	char        cmd[100];
	char        optval[80];
	LRESULT     res;

	switch (msg) {
		case WM_INITDIALOG:
			gwp = gw_find_frame (GetParent (hwnd));
			center_window (gwp->clientHWnd, hwnd);
			SendDlgItemMessage (hwnd, IDC_BO_AUTOINDENT, BM_SETCHECK,
				calctrue(optgetstr("autoindent", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_BO_AUTOTAB, BM_SETCHECK,
				calctrue(optgetstr ("autotab", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_BO_MODIFIED, BM_SETCHECK,
				calctrue(optgetstr ("modified", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_BO_SHIFTWIDTH, WM_SETTEXT,
								0, (LPARAM)optgetstr ("shiftwidth", NULL));
			SendDlgItemMessage (hwnd, IDC_BO_TEXTWIDTH, WM_SETTEXT,
								0, (LPARAM)optgetstr ("textwidth", NULL));
			SendDlgItemMessage (hwnd, IDC_BO_TABSTOP, WM_SETTEXT,
								0, (LPARAM)optgetstr ("tabstop", NULL));
			SendDlgItemMessage (hwnd, IDC_BO_UNDOLEVELS, WM_SETTEXT,
								0, (LPARAM)optgetstr ("undolevels", NULL));
			SendDlgItemMessage (hwnd, IDC_BO_PARAGRAPHS, WM_SETTEXT,
								0, (LPARAM)optgetstr ("paragraphs", NULL));
			SendDlgItemMessage (hwnd, IDC_BO_SECTIONS, WM_SETTEXT,
								0, (LPARAM)optgetstr ("sections", NULL));
			SendDlgItemMessage (hwnd, IDC_BO_CCPRG, WM_SETTEXT,
								0, (LPARAM)optgetstr ("ccprg", NULL));
			SendDlgItemMessage (hwnd, IDC_BO_KEYWORDPRG, WM_SETTEXT,
								0, (LPARAM)optgetstr ("keywordprg", NULL));
			SendDlgItemMessage (hwnd, IDC_BO_EQUALPRG, WM_SETTEXT,
								0, (LPARAM)optgetstr ("equalprg", NULL));
			SendDlgItemMessage (hwnd, IDC_BO_MAKEPRG, WM_SETTEXT,
								0, (LPARAM)optgetstr ("makeprg", NULL));
			SendDlgItemMessage (hwnd, IDC_BO_INPUTTAB, WM_SETTEXT,
								0, (LPARAM)optgetstr ("inputtab", NULL));
			SendDlgItemMessage (hwnd, IDC_BO_READEOL, WM_SETTEXT,
								0, (LPARAM)optgetstr ("readeol", NULL));
			return TRUE;

		case WM_COMMAND:
			if (LOWORD (wParam) == IDOK) {
				EndDialog (hwnd, TRUE);
				res = SendDlgItemMessage (hwnd, IDC_BO_AUTOINDENT, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sautoindent", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_BO_AUTOTAB, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sautotab", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_BO_MODIFIED, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %smodified", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_BO_SHIFTWIDTH, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set shiftwidth=%s", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_BO_TEXTWIDTH, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set textwidth=%s", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_BO_TABSTOP, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set tabstop=%s", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_BO_UNDOLEVELS, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set undolevels=%s", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_BO_PARAGRAPHS, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set paragraphs=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_BO_SECTIONS, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set sections=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_BO_CCPRG, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set ccprg=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_BO_KEYWORDPRG, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set keywordprg=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_BO_EQUALPRG, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set equalprg=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_BO_MAKEPRG, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set makeprg=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_BO_INPUTTAB, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set inputtab=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_BO_READEOL, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set readeol=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				return TRUE;
			}
			else if (LOWORD (wParam) == IDCANCEL) {
				EndDialog (hwnd, TRUE);
				return TRUE;
			}
			break;
	}

	return FALSE;
}

/* --------------------------------------------------------------------
**
** DlgOptGlobal  --  global options.
*/

BOOL CALLBACK DlgOptGlobal (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)

{
	char        cmd[100];
	char        optval[80];
	LRESULT     res;

	switch (msg) {
		case WM_INITDIALOG:
			gwp = gw_find_frame (GetParent (hwnd));
			center_window (gwp->clientHWnd, hwnd);
			SendDlgItemMessage (hwnd, IDC_GO_ANYERROR, BM_SETCHECK,
				calctrue(optgetstr ("anyerror", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_AUTOPRINT, BM_SETCHECK,
				calctrue(optgetstr ("autoprint", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_AUTOSELECT, BM_SETCHECK,
				calctrue(optgetstr ("autoselect", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_AUTOWRITE, BM_SETCHECK,
				calctrue(optgetstr ("autowrite", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_BACKUP, BM_SETCHECK,
				calctrue(optgetstr ("backup", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_BEAUTIFY, BM_SETCHECK,
				calctrue(optgetstr ("beautify", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_DEFAULTREADONLY, BM_SETCHECK,
				calctrue(optgetstr ("defaultreadonly", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_DIGRAPH, BM_SETCHECK,
				calctrue(optgetstr ("digraph", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_EDCOMPATIBLE, BM_SETCHECK,
				calctrue(optgetstr ("edcompatible", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_ERRORBELLS, BM_SETCHECK,
				calctrue(optgetstr ("errorbells", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_EXRC, BM_SETCHECK,
				calctrue(optgetstr ("exrc", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_FLASH, BM_SETCHECK,
				calctrue(optgetstr ("flash", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_GDEFAULT, BM_SETCHECK,
				calctrue(optgetstr ("gdefault", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_IGNORECASE, BM_SETCHECK,
				calctrue(optgetstr ("ignorecase", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_MAGIC, BM_SETCHECK,
				calctrue(optgetstr ("magic", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_MESG, BM_SETCHECK,
				calctrue(optgetstr ("mesg", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_MODELINE, BM_SETCHECK,
				calctrue(optgetstr ("modeline", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_NOVICE, BM_SETCHECK,
				calctrue(optgetstr ("novice", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_OPTIMIZE, BM_SETCHECK,
				calctrue(optgetstr ("optimize", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_PROMPT, BM_SETCHECK,
				calctrue(optgetstr ("prompt", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_REMAP, BM_SETCHECK,
				calctrue(optgetstr ("remap", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_SAFER, BM_SETCHECK,
				calctrue(optgetstr ("safer", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_SHOWMARKUPS, BM_SETCHECK,
				calctrue(optgetstr ("showmarkups", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_SYNC, BM_SETCHECK,
				calctrue(optgetstr ("sync", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_TAGSTACK, BM_SETCHECK,
				calctrue(optgetstr ("tagstack", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_TERSE, BM_SETCHECK,
				calctrue(optgetstr ("terse", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_WARN, BM_SETCHECK,
				calctrue(optgetstr ("warn", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_WARNINGBELLS, BM_SETCHECK,
				calctrue(optgetstr ("warningbells", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_WRAPSCAN, BM_SETCHECK,
				calctrue(optgetstr ("wrapscan", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_WRITEANY, BM_SETCHECK,
				calctrue(optgetstr ("writeany", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_GO_MODELINES, WM_SETTEXT,
								0, (LPARAM)optgetstr ("modelines", NULL));
			SendDlgItemMessage (hwnd, IDC_GO_NEARSCROLL, WM_SETTEXT,
								0, (LPARAM)optgetstr ("nearscroll", NULL));
			SendDlgItemMessage (hwnd, IDC_GO_REPORT, WM_SETTEXT,
								0, (LPARAM)optgetstr ("report", NULL));
			SendDlgItemMessage (hwnd, IDC_GO_TAGLENGTH, WM_SETTEXT,
								0, (LPARAM)optgetstr ("taglength", NULL));
			SendDlgItemMessage (hwnd, IDC_GO_DIRECTORY, WM_SETTEXT,
								0, (LPARAM)optgetstr ("directory", NULL));
			SendDlgItemMessage (hwnd, IDC_GO_HOME, WM_SETTEXT,
								0, (LPARAM)optgetstr ("home", NULL));
			SendDlgItemMessage (hwnd, IDC_GO_NONASCII, WM_SETTEXT,
								0, (LPARAM)optgetstr ("nonascii", NULL));
			SendDlgItemMessage (hwnd, IDC_GO_SHELL, WM_SETTEXT,
								0, (LPARAM)optgetstr ("shell", NULL));
			SendDlgItemMessage (hwnd, IDC_GO_TAGS, WM_SETTEXT,
								0, (LPARAM)optgetstr ("tags", NULL));
			SendDlgItemMessage (hwnd, IDC_GO_WRITEEOL, WM_SETTEXT,
								0, (LPARAM)optgetstr ("writeeol", NULL));
			return TRUE;

		case WM_COMMAND:
			if (LOWORD (wParam) == IDOK) {
				EndDialog (hwnd, TRUE);
				res = SendDlgItemMessage (hwnd, IDC_GO_ANYERROR, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sanyerror", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_AUTOPRINT, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sautoprint", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_AUTOSELECT, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sautoselect", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_AUTOWRITE, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sautowrite", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_BACKUP, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sbackup", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_BEAUTIFY, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sbeautify", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_DEFAULTREADONLY, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sdefaultreadonly", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_DIGRAPH, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sdigraph", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_EDCOMPATIBLE, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sedcompatible", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_ERRORBELLS, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %serrorbells", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_EXRC, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sexrc", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_FLASH, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sflash", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_GDEFAULT, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sgdefault", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_IGNORECASE, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %signorecase", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_MAGIC, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %smagic", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_MESG, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %smesg", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_MODELINE, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %smodeline", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_NOVICE, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %snovice", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_OPTIMIZE, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %soptimize", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_PROMPT, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sprompt", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_REMAP, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sremap", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_SAFER, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %ssafer", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_SHOWMARKUPS, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sshowmarkups", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_SYNC, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %ssync", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_TAGSTACK, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %stagstack", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_TERSE, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sterse", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_WARN, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %swarn", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_WARNINGBELLS, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %swarningbells", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_WRAPSCAN, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %swrapscan", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_GO_WRITEANY, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %swriteany", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_GO_MODELINES, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set modelines=%s", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_GO_NEARSCROLL, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set nearscroll=%s", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_GO_REPORT, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set report=%s", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_GO_TAGLENGTH, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set taglength=%s", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_GO_DIRECTORY, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set directory=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_GO_HOME, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set home=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_GO_NONASCII, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set nonascii=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_GO_SHELL, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set shell=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_GO_TAGS, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set tags=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_GO_WRITEEOL, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set writeeol=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				return TRUE;
			}
			else if (LOWORD (wParam) == IDCANCEL) {
				EndDialog (hwnd, TRUE);
				return TRUE;
			}
			break;
	}

	return FALSE;
}

/* --------------------------------------------------------------------
**
** DlgOptWindow  --  window specific options.
*/

BOOL CALLBACK DlgOptWindow (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)

{
	char        cmd[100];
	char        optval[80];
	LRESULT     res;

	switch (msg) {
		case WM_INITDIALOG:
			gwp = gw_find_frame (GetParent (hwnd));
			center_window (gwp->clientHWnd, hwnd);
			SendDlgItemMessage (hwnd, IDC_WO_LIST, BM_SETCHECK,
				calctrue(optgetstr ("list", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_WO_NUMBER, BM_SETCHECK,
				calctrue(optgetstr ("number", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_WO_SHOWMATCH, BM_SETCHECK,
				calctrue(optgetstr ("showmatch", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_WO_WRAP, BM_SETCHECK,
				calctrue(optgetstr ("wrap", NULL)) ? 1 : 0, 0);
			SendDlgItemMessage (hwnd, IDC_WO_COLUMNS, WM_SETTEXT,
								0, (LPARAM)optgetstr ("columns", NULL));
			SendDlgItemMessage (hwnd, IDC_WO_LINES, WM_SETTEXT,
								0, (LPARAM)optgetstr ("lines", NULL));
			SendDlgItemMessage (hwnd, IDC_WO_SCROLL, WM_SETTEXT,
								0, (LPARAM)optgetstr ("scroll", NULL));
			SendDlgItemMessage (hwnd, IDC_WO_SIDESCROLL, WM_SETTEXT,
								0, (LPARAM)optgetstr ("sidescroll", NULL));
			SendDlgItemMessage (hwnd, IDC_WO_WRAPMARGIN, WM_SETTEXT,
								0, (LPARAM)optgetstr ("wrapmargin", NULL));
			return TRUE;

		case WM_COMMAND:
			if (LOWORD (wParam) == IDOK) {
				EndDialog (hwnd, TRUE);
				res = SendDlgItemMessage (hwnd, IDC_WO_LIST, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %slist", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_WO_NUMBER, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %snumber", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_WO_SHOWMATCH, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %sshowmatch", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				res = SendDlgItemMessage (hwnd, IDC_WO_WRAP, BM_GETCHECK, 0, 0);
				sprintf (cmd, ":set %swrap", res ? "" : "no");
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_WO_COLUMNS, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set columns=%s", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_WO_LINES, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set lines=%s", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_WO_SCROLL, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set scroll=%s", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_WO_SIDESCROLL, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set sidescroll=%s", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_WO_WRAPMARGIN, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set wrapmargin=%s", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				return TRUE;
			}
			else if (LOWORD (wParam) == IDCANCEL) {
				EndDialog (hwnd, TRUE);
				return TRUE;
			}
			break;
	}

	return FALSE;
}

/* --------------------------------------------------------------------
**
** DlgOptUser  --  user options.
*/

BOOL CALLBACK DlgOptUser (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)

{
	char        cmd[100];
	char        optval[80];

	switch (msg) {
		case WM_INITDIALOG:
			gwp = gw_find_frame (GetParent (hwnd));
			center_window (gwp->clientHWnd, hwnd);
			SendDlgItemMessage (hwnd, IDC_UO_A, WM_SETTEXT,
								0, (LPARAM)optgetstr ("a", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_B, WM_SETTEXT,
								0, (LPARAM)optgetstr ("b", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_C, WM_SETTEXT,
								0, (LPARAM)optgetstr ("c", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_D, WM_SETTEXT,
								0, (LPARAM)optgetstr ("d", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_E, WM_SETTEXT,
								0, (LPARAM)optgetstr ("e", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_F, WM_SETTEXT,
								0, (LPARAM)optgetstr ("f", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_G, WM_SETTEXT,
								0, (LPARAM)optgetstr ("g", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_H, WM_SETTEXT,
								0, (LPARAM)optgetstr ("h", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_I, WM_SETTEXT,
								0, (LPARAM)optgetstr ("i", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_J, WM_SETTEXT,
								0, (LPARAM)optgetstr ("j", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_K, WM_SETTEXT,
								0, (LPARAM)optgetstr ("k", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_L, WM_SETTEXT,
								0, (LPARAM)optgetstr ("l", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_M, WM_SETTEXT,
								0, (LPARAM)optgetstr ("m", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_N, WM_SETTEXT,
								0, (LPARAM)optgetstr ("n", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_O, WM_SETTEXT,
								0, (LPARAM)optgetstr ("o", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_P, WM_SETTEXT,
								0, (LPARAM)optgetstr ("p", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_Q, WM_SETTEXT,
								0, (LPARAM)optgetstr ("q", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_R, WM_SETTEXT,
								0, (LPARAM)optgetstr ("r", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_S, WM_SETTEXT,
								0, (LPARAM)optgetstr ("s", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_T, WM_SETTEXT,
								0, (LPARAM)optgetstr ("t", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_U, WM_SETTEXT,
								0, (LPARAM)optgetstr ("u", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_V, WM_SETTEXT,
								0, (LPARAM)optgetstr ("v", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_W, WM_SETTEXT,
								0, (LPARAM)optgetstr ("w", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_X, WM_SETTEXT,
								0, (LPARAM)optgetstr ("x", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_Y, WM_SETTEXT,
								0, (LPARAM)optgetstr ("y", NULL));
			SendDlgItemMessage (hwnd, IDC_UO_Z, WM_SETTEXT,
								0, (LPARAM)optgetstr ("z", NULL));
			return TRUE;

		case WM_COMMAND:
			if (LOWORD (wParam) == IDOK) {
				EndDialog (hwnd, TRUE);
				SendDlgItemMessage (hwnd, IDC_UO_A, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set a=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_B, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set b=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_C, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set c=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_D, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set d=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_E, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set e=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_F, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set f=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_G, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set g=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_H, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set h=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_I, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set i=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_J, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set j=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_K, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set k=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_L, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set l=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_M, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set m=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_N, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set n=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_O, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set o=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_P, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set p=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_Q, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set q=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_R, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set r=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_S, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set s=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_T, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set t=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_U, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set u=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_V, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set v=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_W, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set w=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_X, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set x=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_Y, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set y=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				SendDlgItemMessage (hwnd, IDC_UO_Z, WM_GETTEXT,
									sizeof (optval), (LPARAM)optval);
				sprintf (cmd, ":set z=\"%s\"", optval);
				eventex ((GUIWIN *)gwp, cmd, ElvFalse);
				return TRUE;
			}
			else if (LOWORD (wParam) == IDCANCEL) {
				EndDialog (hwnd, TRUE);
				return TRUE;
			}
			break;
	}

	return FALSE;
}

/* --------------------------------------------------------------------
**
** DlgAbout  --  process the About dialog.
*/

BOOL CALLBACK DlgAbout (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)

{
	char                str[80];

	switch (msg) {
		case WM_INITDIALOG:
			center_window (GetFocus (), hwnd);

			sprintf(str, "WinElvis Version %s", VERSION);
			SetDlgItemText (hwnd, IDAB_VERSION, str);

			return TRUE;

		case WM_COMMAND:
			if (LOWORD (wParam) == IDOK || LOWORD (wParam) == IDCANCEL) {
				EndDialog (hwnd, TRUE);
				return TRUE;
			}
			break;
	}

	return FALSE;
}

#endif
/* ex:se ts=4 sw=4: */
