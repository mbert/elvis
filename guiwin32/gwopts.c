/*
**  gwopts.c  --  options processing for MS-Windows elvis.
**
** Copyright 1996, Serge Pirotte
*/

#define CHAR    Char
#define BOOLEAN Boolean
#include "elvis.h" 
#undef CHAR
#undef BOOLEAN

#if defined (GUI_WIN32)

#include <windows.h>
#include "winelvis.h"
#include "elvisres.h"

/* --------------------------------------------------------------------
**
** opt_parse_font  --  parse a font option.
*/

void opt_parse_font (unsigned char *option, LOGFONT *plf)

{
    register int    i = 0;
    int             font_size = 0;
    HDC             dc;

    /* get face name */
    while (*option != '\0') {
        if (*option == '*') {
            option++;
            break;
        }
        plf->lfFaceName[i++] = *option++;
    }
    plf->lfFaceName[i] = '\0';

    /* get font size */
    while (*option) {
        if (*option == '*') {
            option++;
            if (*option && (*option == 'o' || *option == 'O')) {
                plf->lfCharSet = OEM_CHARSET;
            }
            break;
        }
        font_size = (font_size * 10) + (*option - '0');
        option++;
    }
    dc = GetDC (NULL);
    plf->lfHeight = -MulDiv (font_size, GetDeviceCaps (dc, LOGPIXELSY), 72);
    ReleaseDC (NULL, dc);
}

/* --------------------------------------------------------------------
**
** opt_parse_attr  --  parse a font attribute.
*/

void opt_parse_attr (unsigned char *option, LOGFONT *plf)

{
    /* init to default values */
    plf->lfWeight= FW_NORMAL;
    plf->lfItalic = FALSE;
    plf->lfUnderline = FALSE;

    /* scan options */
    while (*option != '\0') {
        switch (*option) {
            case 'b':
            case 'B':
                plf->lfWeight = FW_BOLD;
                break;
            case 'i':
            case 'I':
                plf->lfItalic = TRUE;
                break;
            case 'u':
            case 'U':
                plf->lfUnderline = TRUE;
                break;
        }
        option++;
    }
}

/* --------------------------------------------------------------------
**
** optstoresb  --  store a scrollbar option.
*/

int optstoresb (OPTDESC *opt, OPTVAL *val, Char *newval)

{
    GUI_WINDOW      *gwp;
    Boolean         value = calctrue(newval) ? True : False;

    if (val->value.boolean == value)
        return 0;

    val->value.boolean = value;

    if ((gwp = gw_find_client (GetFocus ())) != NULL) {
        ShowScrollBar (gwp->clientHWnd, SB_VERT, value ? TRUE : FALSE);
		SendMessage (gwp->frameHWnd, WM_SIZE, 0, 0);
	}

    return 1;
}

/* --------------------------------------------------------------------
**
** optstoretb  --  store a toolbar option.
*/

int optstoretb (OPTDESC *opt, OPTVAL *val, Char *newval)

{
    GUI_WINDOW      *gwp;
    Boolean         value = calctrue(newval) ? True : False;

    if (val->value.boolean == value)
        return 0;

    val->value.boolean = value;

    if ((gwp = gw_find_client (GetFocus ())) != NULL) {
        if (value)
            gw_create_toolbar (gwp);
        else
            gw_destroy_toolbar (gwp);
		SendMessage (gwp->frameHWnd, WM_SIZE, 0, 0);
	}

    return 1;
}

/* --------------------------------------------------------------------
**
** optstorestb  --  store a statusbar option.
*/

int optstorestb (OPTDESC *opt, OPTVAL *val, Char *newval)

{
    GUI_WINDOW      *gwp;
    Boolean         value = calctrue(newval) ? True : False;

    if (val->value.boolean == value)
        return 0;

    val->value.boolean = value;

    if ((gwp = gw_find_client (GetFocus ())) != NULL) {
        if (value)
            gw_create_status_bar (gwp);
        else
            gw_destroy_status_bar (gwp);
		SendMessage (gwp->frameHWnd, WM_SIZE, 0, 0);
	}

    return 1;
}

/* --------------------------------------------------------------------
**
** optstoremnu  --  store a menubar option.
*/

int optstoremnu (OPTDESC *opt, OPTVAL *val, Char *newval)

{
    GUI_WINDOW      *gwp;
    Boolean         value = calctrue(newval) ? True : False;

    if (val->value.boolean == value)
        return 0;

    val->value.boolean = value;

    if ((gwp = gw_find_client (GetFocus ())) != NULL) {
        if (value)
            gwp->menuHndl = LoadMenu (hInst, MAKEINTRESOURCE (IDM_ELVIS));
        else if (gwp->menuHndl != NULL)
            DestroyMenu (gwp->menuHndl);
		SetMenu (gwp->frameHWnd, gwp->menuHndl);
		DrawMenuBar (gwp->frameHWnd);
		SendMessage (gwp->frameHWnd, WM_SIZE, 0, 0);
	}

    return 1;
}

/* --------------------------------------------------------------------
**
** optisfont  --  validate a font option.
*/

int optisfont (OPTDESC *opt, OPTVAL *val, Char *newval)

{
    GUI_WINDOW  *gwp;
    LOGFONT     lf;
    HFONT       hFont;
    Char        *p;

    if (strcmp (val->value.string, newval) == 0)
        return 0;

    memset (&lf, 0, sizeof (LOGFONT));
    opt_parse_font (newval, &lf);
    if ((hFont = CreateFontIndirect (&lf)) == NULL)
        return -1;

    DeleteObject (hFont);

    p = CHARdup(newval);
	safefree (val->value.string);
	val->value.string = p;

    if ((gwp = gw_find_client (GetFocus ())) != NULL) {
		HideCaret (gwp->clientHWnd);
		DestroyCaret ();
		gw_del_fonts (gwp);
		gw_set_fonts (gwp);
		gw_get_win_size (gwp);
		gw_set_cursor (gwp, True);
		ShowCaret (gwp->clientHWnd);
		eventresize ((GUIWIN *)gwp, gwp->numrows, gwp->numcols);
		InvalidateRect (gwp->clientHWnd, NULL, TRUE);
	}

	return 1;
}

/* --------------------------------------------------------------------
**
** optstoreattr  --  store a font attribute option.
*/

int optstoreattr (OPTDESC *opt, OPTVAL *val, Char *newval)

{
    GUI_WINDOW  *gwp;
    Char        *p;

    p = CHARdup(newval);
	safefree (val->value.string);
	val->value.string = p;

    if ((gwp = gw_find_client (GetFocus ())) != NULL) {
		gw_del_fonts (gwp);
		gw_set_fonts (gwp);
		InvalidateRect (gwp->clientHWnd, NULL, TRUE);
	}

	return 1;
}

/* --------------------------------------------------------------------
**
** optiswinsize  --  check if LINES or COLUMNS options are valid.
*/

int optiswinsize (OPTDESC *opt, OPTVAL *val, Char *newval)

{
    GUI_WINDOW      *gwp;
    long            value = atoi ((char *)newval);
    long            saved_val;

    /* any positive number will fit. */
    if (value <= 0)
        return -1;

    /* check if same value */
    if (value == val->value.number)
        return 0;

    /* save new value */
    saved_val = val->value.number;
    val->value.number = value;

	/* if a view is active, check if validi */
	if ((gwp = gw_find_client (GetFocus ())) != NULL) {
	    if (gw_set_win_size (gwp, 1) == 0) {
	        val->value.number = saved_val;
	        return -1;
		}
	}

	return 1;
}

#endif

