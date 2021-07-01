/*
** gwcmd.c  --  menu support for elvis.
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
#include "elvisres.h"

extern char     *gw_new_buffer;

/* --------------------------------------------------------------------
**
** gwcmd_file_new  --  create a new file.
*/

static void gwcmd_file_new (GUI_WINDOW *gwp)

{
	BUFFER      pbuf = bufalloc (NULL, 0, ElvFalse);

	if (pbuf != NULL)
		eventreplace ((GUIWIN *)gwp, ElvFalse, o_bufname (pbuf));

	gw_redraw_win (gwp);
}

/* --------------------------------------------------------------------
**
** gwcmd_file_open  --  open a existing file.
*/

static void gwcmd_file_open (GUI_WINDOW *gwp)

{
	static char         *filters = "All Files\0*.*\0"
	                               "Text Files\0*.TXT\0"
	                               "C Files\0*.c;*.cpp;*.h\0"
	                               "HTML Files\0*.htm;*.html";
	OPENFILENAME        ofn;
	char                cmd[_MAX_PATH + 20];
	Char		*quoted;

	strcpy (cmd, ":e ");
	memset (&ofn, 0, sizeof (ofn));
	ofn.lStructSize = sizeof (OPENFILENAME);
	ofn.lpstrFilter = (LPCTSTR)filters;
	ofn.lpstrFile = &cmd[3];
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrTitle = "Open...";
#if _MSC_VER > 900
	ofn.Flags = OFN_LONGNAMES | OFN_NOCHANGEDIR;
#else
	ofn.Flags = OFN_NOCHANGEDIR;
#endif

	if (GetOpenFileName (&ofn) == TRUE) {
		quoted = addquotes(toCHAR("#% ()$"), toCHAR(&cmd[3]));
		strcpy(&cmd[3], tochar8(quoted));
		safefree(quoted);
		eventex ((GUIWIN *)gwp, cmd, ElvFalse);
		gw_redraw_win (gwp);
	}
}

/* --------------------------------------------------------------------
**
** gwcmd_file_split  --  open a existing file into a new window.
*/

static void gwcmd_file_split (GUI_WINDOW *gwp)

{
	static char         *filters = "All Files\0*.*\0"
	                               "Text Files\0*.TXT\0"
	                               "C Files\0*.c;*.cpp;*.h\0"
	                               "HTML Files\0*.htm;*.html";
	OPENFILENAME        ofn;
	char                cmd[_MAX_PATH + 20];
	Char		*quoted;

	strcpy (cmd, ":sp ");
	memset (&ofn, 0, sizeof (ofn));
	ofn.lStructSize = sizeof (OPENFILENAME);
	ofn.lpstrFilter = (LPCTSTR)filters;
	ofn.lpstrFile = &cmd[4];
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrTitle = "Split...";
#if _MSC_VER > 900
	ofn.Flags = OFN_LONGNAMES | OFN_NOCHANGEDIR;
#else
	ofn.Flags = OFN_NOCHANGEDIR;
#endif

	if (GetOpenFileName (&ofn) == TRUE) {
		quoted = addquotes(toCHAR("#% ()$"), toCHAR(&cmd[4]));
		strcpy(&cmd[4], tochar8(quoted));
		safefree(quoted);
		eventex ((GUIWIN *)gwp, cmd, ElvFalse);
		gw_redraw_win (gwp);
	}
}

/* --------------------------------------------------------------------
**
** gwcmd_file_save  --  save current buffer.
*/

static void gwcmd_file_save (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":w", ElvFalse);
}

/* --------------------------------------------------------------------
**
** gwcmd_file_saveas  --  save the current buffer.
*/

static void gwcmd_file_saveas (GUI_WINDOW *gwp)

{
	OPENFILENAME        ofn;
	char                cmd[_MAX_PATH + 20];
	Char		*quoted;

	strcpy (cmd, ":w! ");
	memset (&ofn, 0, sizeof (ofn));
	ofn.lStructSize = sizeof (OPENFILENAME);
	ofn.lpstrFile = &cmd[4];
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrTitle = "Save As...";
#if _MSC_VER > 900
	ofn.Flags = OFN_LONGNAMES | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;
#else
	ofn.Flags = OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;
#endif

	if (GetOpenFileName (&ofn) == TRUE) {
		quoted = addquotes(toCHAR("#% ()$"), toCHAR(&cmd[4]));
		strcpy(&cmd[4], tochar8(quoted));
		safefree(quoted);
		eventex ((GUIWIN *)gwp, cmd, ElvFalse);
	}
}

/* --------------------------------------------------------------------
**
** gwcmd_file_cd  --  change current directory.
*/

static void gwcmd_file_cd (GUI_WINDOW *gwp)

{
	OPENFILENAME        ofn;
	char                cmd[_MAX_PATH + 5];
	register int        i;
	Char		*quoted;

	strcpy (cmd, ":cd x");
	memset (&ofn, 0, sizeof (ofn));
	ofn.lStructSize = sizeof (OPENFILENAME);
	ofn.hInstance = hInst;
	ofn.lpstrFile = &cmd[4];
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrTitle = "Change Directory...";
#if _MSC_VER > 900
	ofn.Flags = OFN_LONGNAMES | OFN_NOCHANGEDIR | OFN_ENABLETEMPLATE;
#else
	ofn.Flags = OFN_NOCHANGEDIR | OFN_ENABLETEMPLATE;
#endif
	ofn.lpTemplateName = MAKEINTRESOURCE (IDD_DIR_SELECT);

	if (GetOpenFileName (&ofn) == TRUE) {
		/* remove the last element of the return name -- we only want dir */
		for (i = strlen (cmd); cmd[i] != '\\'; i--)
			;
		cmd[i] = '\0';

		/* quote the dangerous chars */
		quoted = addquotes(toCHAR("#% ()$"), toCHAR(&cmd[4]));
		strcpy(&cmd[4], tochar8(quoted));
		safefree(quoted);

		eventex ((GUIWIN *)gwp, cmd, ElvFalse);
	}
}

/* --------------------------------------------------------------------
**
** gwcmd_file_print  --  exit current buffer.
*/

static void gwcmd_file_print (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":lpr", ElvFalse);
}

/* --------------------------------------------------------------------
**
** gwcmd_file_printsetup  --  setup current printer.
*/

static void gwcmd_file_printsetup (GUI_WINDOW *gwp)

{
	DEVNAMES        *dvnp;

	gwpdlg.hDevMode = NULL;
	gwpdlg.Flags = PD_PRINTSETUP;

	if (PrintDlg (&gwpdlg)) {
		dvnp = GlobalLock (gwpdlg.hDevNames);
		gw_set_default_printer ((char *)dvnp + dvnp->wDeviceOffset,
								(char *)dvnp + dvnp->wDriverOffset,
								(char *)dvnp + dvnp->wOutputOffset);
		GlobalUnlock (gwpdlg.hDevNames);
	}
}

/* --------------------------------------------------------------------
**
** gwcmd_file_exit  --  exit current window.
*/

static void gwcmd_file_exit (GUI_WINDOW *gwp)

{
#if 0
	static unsigned char        chr[] = "\x1B:q\r";

	eventkeys ((GUIWIN *)gwp, &chr[0], 1);
	eventkeys ((GUIWIN *)gwp, &chr[1], 1);
	eventkeys ((GUIWIN *)gwp, &chr[2], 1);
	eventkeys ((GUIWIN *)gwp, &chr[3], 1);
#else
	eventex ((GUIWIN *)gwp, ":q", ElvFalse);
#endif
}

/* --------------------------------------------------------------------
**
** gwcmd_edit_undo  --  undo.
*/

static void gwcmd_edit_undo (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":undo", ElvFalse);
	gw_redraw_win (gwp);
}

/* --------------------------------------------------------------------
**
** gwcmd_edit_redo  --  redo.
*/

static void gwcmd_edit_redo (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":redo", ElvFalse);
	gw_redraw_win (gwp);
}

/* --------------------------------------------------------------------
**
** gwcmd_edit_cut  --  cut.
*/

static void gwcmd_edit_cut (GUI_WINDOW *gwp)

{
#if 0
	eventex ((GUIWIN *)gwp, ":d >", ElvFalse);
#else
	if (eventclick ((GUIWIN *)gwp, -1, -1, CLICK_YANK) >= 0)
	{
		(void)eventkeys ((GUIWIN *)gwp, toCHAR("\017d"), 2);
		(void)eventclick ((GUIWIN *)gwp, -1, -1, CLICK_CANCEL);
	}
#endif
	gw_redraw_win (gwp);
}

/* --------------------------------------------------------------------
**
** gwcmd_edit_copy  --  copy.
*/

static void gwcmd_edit_copy (GUI_WINDOW *gwp)

{
#if 0
	eventex ((GUIWIN *)gwp, ":y >", ElvFalse);
#else
	if (eventclick ((GUIWIN *)gwp, -1, -1, CLICK_YANK) >= 0)
	{
		(void)eventclick ((GUIWIN *)gwp, -1, -1, CLICK_CANCEL);
	}
#endif
	gw_redraw_win (gwp);
}

/* --------------------------------------------------------------------
**
** gwcmd_edit_paste  --  paste.
*/

static void gwcmd_edit_paste (GUI_WINDOW *gwp)

{
#if 0
	eventex ((GUIWIN *)gwp, ":pu <", ElvFalse);
#else
	(void)eventclick ((GUIWIN *)gwp, -1, -1, CLICK_PASTE);
#endif
	gw_redraw_win (gwp);
}

/* --------------------------------------------------------------------
**
** gwcmd_search_again  --  repeat last search.
*/

static void gwcmd_search_again (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, "/", ElvFalse);
	gw_redraw_win (gwp);
}

/* --------------------------------------------------------------------
**
** gwcmd_search_next_error  --  search for next error.
*/

static void gwcmd_search_next_error (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":errlist", ElvFalse);
	gw_redraw_win (gwp);
}

/* --------------------------------------------------------------------
**
** gwcmd_window_new  --  create a new window.
*/

static void gwcmd_window_new (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":split", ElvFalse);
	gw_redraw_win (gwp);
}

/* --------------------------------------------------------------------
**
** gwcmd_window_next  --  goto the next window.
*/

static void gwcmd_window_next (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":window ++", ElvFalse);
	gw_redraw_win (gwp);
}

/* --------------------------------------------------------------------
**
** gwcmd_window_previous  --  goto the previous window.
*/

static void gwcmd_window_previous (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":window --", ElvFalse);
	gw_redraw_win (gwp);
}

/* --------------------------------------------------------------------
**
** gwcmd_window_nfile  --  edit the :next file.
*/

static void gwcmd_window_nfile (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":next", ElvFalse);
	gw_redraw_win (gwp);
}

/* --------------------------------------------------------------------
**
** gwcmd_window_buffer  --  give a buffer list to select.
*/

static void gwcmd_window_buffer (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":bb", ElvFalse);
	gw_redraw_win (gwp);
}

/* --------------------------------------------------------------------
**
** gwcmd_window_pfile  --  edit the :previous file.
*/

static void gwcmd_window_pfile (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":prev", ElvFalse);
	gw_redraw_win (gwp);
}

/* --------------------------------------------------------------------
**
** gwcmd_options_font  --  select a font.
*/

static void gwcmd_options_font (GUI_WINDOW *gwp)

{
	CHOOSEFONT      font;
	LOGFONT         lf;
	HDC             dc;
	char            str[80];

	memset (&font, 0, sizeof (CHOOSEFONT));
	font.lStructSize = sizeof (CHOOSEFONT);
	font.lpLogFont = &lf;
	font.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_FIXEDPITCHONLY;

	memset (&lf, 0, sizeof (LOGFONT));
	opt_parse_font (o_font (gwp), &lf);

	if (ChooseFont (&font)) {
		dc = GetDC (NULL);
		lf.lfHeight = -MulDiv (lf.lfHeight, 72, GetDeviceCaps (dc, LOGPIXELSY));
		ReleaseDC (NULL, dc);
		sprintf (str, ":set font=\"%s*%d\"", lf.lfFaceName, lf.lfHeight);
		eventex ((GUIWIN *)gwp, str, ElvFalse);
		gw_redraw_win (gwp);
	}
}

/* --------------------------------------------------------------------
**
** gwcmd_options_save  --  save current options.
*/

static void gwcmd_options_save (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":mkexrc", ElvFalse);
}

/* --------------------------------------------------------------------
**
** gwcmd_tools_compile  --  compile current buffer.
*/

static void gwcmd_tools_compile (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":cc", ElvFalse);
}

/* --------------------------------------------------------------------
**
** gwcmd_tools_make  --  run the make utility.
*/

static void gwcmd_tools_make (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":make", ElvFalse);
}

/* --------------------------------------------------------------------
**
** gwcmd_tools_shell  --  start a subshell
*/

static void gwcmd_tools_shell (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":sh", ElvFalse);
}

/* --------------------------------------------------------------------
**
** gwcmd_help_index  --  open a help window.
*/

static void gwcmd_help_index (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":help", ElvFalse);
}

/* --------------------------------------------------------------------
**
** gwcmd_help_ex  --  open a ex help window.
*/

static void gwcmd_help_ex (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":help ex", ElvFalse);
}

/* --------------------------------------------------------------------
**
** gwcmd_help_vi  --  open a vi help window.
*/

static void gwcmd_help_vi (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":help vi", ElvFalse);
}

/* --------------------------------------------------------------------
**
** gwcmd_help_options  --  open a options help window.
*/

static void gwcmd_help_options (GUI_WINDOW *gwp)

{
	eventex ((GUIWIN *)gwp, ":help set all", ElvFalse);
}

/* --------------------------------------------------------------------

**
** gwcmd  --  process a menu command.
*/

LRESULT gwcmd (GUI_WINDOW *gwp, WPARAM wParam)

{
	switch (LOWORD (wParam)) {
		case IDM_FILE_NEW:
			gwcmd_file_new (gwp);
			break;
		case IDM_FILE_OPEN:
			gwcmd_file_open (gwp);
			break;
		case IDM_FILE_SPLIT:
			gwcmd_file_split (gwp);
			break;
		case IDM_FILE_SAVE:
			gwcmd_file_save (gwp);
			break;
		case IDM_FILE_SAVEAS:
			gwcmd_file_saveas (gwp);
			break;
		case IDM_FILE_CD:
			gwcmd_file_cd (gwp);
			break;
		case IDM_FILE_PRINT:
			gwcmd_file_print (gwp);
			break;
		case IDM_FILE_PRINTSETUP:
			gwcmd_file_printsetup (gwp);
			break;
		case IDM_FILE_EXIT:
			gwcmd_file_exit (gwp);
			break;
		case IDM_EDIT_UNDO:
			gwcmd_edit_undo (gwp);
			break;
		case IDM_EDIT_REDO:
			gwcmd_edit_redo (gwp);
			break;
		case IDM_EDIT_CUT:
			gwcmd_edit_cut (gwp);
			break;
		case IDM_EDIT_COPY:
			gwcmd_edit_copy (gwp);
			break;
		case IDM_EDIT_PASTE:
			gwcmd_edit_paste (gwp);
			break;
		case IDM_SEARCH_SEARCH:
			DialogBox (hInst, MAKEINTRESOURCE (IDD_SEARCH), gwp->frameHWnd,
			           (DLGPROC)DlgSearch);
			break;
		case IDM_SEARCH_AGAIN:
			gwcmd_search_again (gwp);
			break;
		case IDM_SEARCH_REPLACE:
			DialogBox (hInst, MAKEINTRESOURCE (IDD_REPLACE), gwp->frameHWnd,
			           (DLGPROC)DlgReplace);
			break;
		case IDM_SEARCH_GOTO:
			DialogBox (hInst, MAKEINTRESOURCE (IDD_GOTO), gwp->frameHWnd,
			           (DLGPROC)DlgGoto);
			break;
		case IDM_SEARCH_NEXT_ERROR:
			gwcmd_search_next_error (gwp);
			break;
		case IDM_WINDOW_NEW:
			gwcmd_window_new (gwp);
			break;
		case IDM_WINDOW_NEXT:
			gwcmd_window_next (gwp);
			break;
		case IDM_WINDOW_PREVIOUS:
			gwcmd_window_previous (gwp);
			break;
		case IDM_WINDOW_NFILE:
			gwcmd_window_nfile (gwp);
			break;
		case IDM_WINDOW_PFILE:
			gwcmd_window_pfile (gwp);
			break;
		case IDM_WINDOW_BUFFER:
			gwcmd_window_buffer (gwp);
			break;
		case IDM_OPTIONS_FONT:
			gwcmd_options_font (gwp);
			break;
		case IDM_OPTIONS_GUI:
			DialogBox (hInst, MAKEINTRESOURCE (IDD_OPT_GUI), gwp->frameHWnd,
			           (DLGPROC)DlgOptGui);
			break;
		case IDM_OPTIONS_BUFFER:
			DialogBox (hInst, MAKEINTRESOURCE (IDD_OPT_BUFFER), gwp->frameHWnd,
			           (DLGPROC)DlgOptBuffer);
			break;
		case IDM_OPTIONS_GLOBAL:
			DialogBox (hInst, MAKEINTRESOURCE (IDD_OPT_GLOBAL), gwp->frameHWnd,
			           (DLGPROC)DlgOptGlobal);
			break;
		case IDM_OPTIONS_WINDOW:
			DialogBox (hInst, MAKEINTRESOURCE (IDD_OPT_WINDOW), gwp->frameHWnd,
			           (DLGPROC)DlgOptWindow);
			break;
		case IDM_OPTIONS_USER:
			DialogBox (hInst, MAKEINTRESOURCE (IDD_OPT_USER), gwp->frameHWnd,
			           (DLGPROC)DlgOptUser);
			break;
		case IDM_OPTIONS_SAVE:
			gwcmd_options_save (gwp);
			break;
		case IDM_TOOLS_COMPILE:
			gwcmd_tools_compile (gwp);
			break;
		case IDM_TOOLS_MAKE:
			gwcmd_tools_make (gwp);
			break;
		case IDM_TOOLS_TAGS:
			DialogBox (hInst, MAKEINTRESOURCE (IDD_TAGS), gwp->frameHWnd,
			           (DLGPROC)DlgTags);
			break;
		case IDM_TOOLS_RUN:
			DialogBox (hInst, MAKEINTRESOURCE (IDD_RUN), gwp->frameHWnd,
			           (DLGPROC)DlgRun);
			break;
		case IDM_TOOLS_SHELL:
			gwcmd_tools_shell (gwp);
			break;
		case IDM_HELP_INDEX:
			gwcmd_help_index (gwp);
			break;
		case IDM_HELP_EX:
			gwcmd_help_ex (gwp);
			break;
		case IDM_HELP_VI:
			gwcmd_help_vi (gwp);
			break;
		case IDM_HELP_OPTIONS:
			gwcmd_help_options (gwp);
			break;
		case IDM_HELP_ABOUT:
			DialogBox (hInst, MAKEINTRESOURCE (IDD_ABOUT), gwp->frameHWnd,
			           (DLGPROC)DlgAbout);
			break;
	}

	return 0;
}

#endif
/* ex:se ts=4 sw=4: */
