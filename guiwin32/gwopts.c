/*
**  gwopts.c  --  options processing for MS-Windows elvis.
**
** Copyright 1996, Serge Pirotte
*/

#define CHAR    Char
#include "elvis.h" 
#undef CHAR
#undef ELVBOOL

#if defined (GUI_WIN32)

#include <windows.h>
#include "winelvis.h"
#include "elvisres.h"

/* --------------------------------------------------------------------
**
** opt_parse_font  --  parse a font option.
*/

void opt_parse_font(unsigned char *option, LOGFONT *plf)
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
** optstoresb  --  store a scrollbar option.
*/

int optstoresb (OPTDESC *opt, OPTVAL *val, Char *newval)
{
    GUI_WINDOW      *gwp;
    ELVBOOL         value = calctrue(newval) ? ElvTrue : ElvFalse;

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
    ELVBOOL         value = calctrue(newval) ? ElvTrue : ElvFalse;

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
    ELVBOOL         value = calctrue(newval) ? ElvTrue : ElvFalse;

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
    ELVBOOL         value = calctrue(newval) ? ElvTrue : ElvFalse;

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
		gw_set_cursor (gwp, ElvTrue);
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

/* Change the icon image */
int optisicon (OPTDESC *opt, OPTVAL *val, Char *newval)
{
#ifdef FEATURE_IMAGE
    GUI_WINDOW	*gwp;
    HBITMAP		hbmColor, hbmMask;
    HICON		hicon;
    ICONINFO	iconinfo;
    char		*fullname;
    char		xpmname[400];
#endif

	/* if same value, then do nothing */
	if (val->value.string ? !CHARcmp(newval, val->value.string)
						  : *newval == '\0')
		return 0;

#ifdef FEATURE_IMAGE
	if (*newval)
	{
		/* locate the file, if not in current directory */
		fullname = tochar8(newval);
		if (dirperm(fullname) < DIR_READONLY)
		{
			strcpy(xpmname, "icons/");
			strcat(xpmname, tochar8(newval));
			fullname = iopath(tochar8(o_elvispath), xpmname, ElvFalse);
			if (!fullname)
			{
				strcat(xpmname, ".xpm");
				fullname = iopath(tochar8(o_elvispath), xpmname, ElvFalse);
			}
		}
		if (!fullname)
		{
			msg(MSG_ERROR, "[s]can't find icon $1", newval);
			return -1;
		}

		/* load the new pixmap, and convert it to an icon */
		hbmColor = gw_load_xpm(fullname, -1L, NULL, &hbmMask);
		if (!hbmColor)
		{
			msg(MSG_ERROR, "[s]can't load image from $1", fullname);
			return -1;
		}
		memset(&iconinfo, 0, sizeof iconinfo);
		iconinfo.fIcon = TRUE;
		iconinfo.hbmColor = hbmColor;
		iconinfo.hbmMask = hbmMask;
		hicon = CreateIconIndirect(&iconinfo);
		gw_unload_xpm(hbmColor);
		gw_unload_xpm(hbmMask);
	}
	else /* setting to "" */
	{
		/* use the default icon */
		hicon = LoadIcon (hInst, MAKEINTRESOURCE (IDI_ELVIS));
	}

	/* store it as an icon */
	if ((gwp = gw_find_client (GetFocus ())) != NULL)
#if _WIN64
		SetClassLongPtr(gwp->frameHWnd, GCLP_HICON, (LONG_PTR)hicon);
#else
		SetClassLong(gwp->frameHWnd, GCL_HICON, (long)hicon);
#endif

	/* free the old icon, and remember the new one */
	if (gwcustomicon)
		DestroyIcon(gwcustomicon);
	if (*newval)
		gwcustomicon = hicon;
	else
		gwcustomicon = NULL;
#endif

	/* free the old name, if any */
	if (val->value.string)
		safefree(val->value.string);

	/* store the new name */
	val->value.string = *newval ? CHARdup(newval) : NULL;

	/* the icon was successfully changed */
	return 1;
}

#endif
/* ex:se ts=4 sw=4: */
