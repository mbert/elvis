/*
** gwprint.c -- printing support for MS-Windows elvis.
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
#include <commctrl.h>
#include "winelvis.h"
#include "elvisres.h"
#ifndef MB_ICONERROR
# define MB_ICONERROR 0
#endif

PRINTDLG        gwpdlg;
int             gw_printing_ok = 1;

GUI_WINDOW      *pgwp;
    
static char     *szDevice = NULL;
static char     *szDriver = NULL;
static char     *szOutput = NULL;

static HFONT    pnFont;
static HFONT    pfFont;
static HFONT    pbFont;
static HFONT    peFont;
static HFONT    piFont;
static HFONT    puFont;

static HFONT    pCurFont = NULL;
static int      pNewPage = 0;

static HDC      printerDC;
static int      oktoprint = 0;

static int      pxOffset;
static int      pyOffset;
static int      pxSize;
static int      pySize;
static int      pxCSize;
static int      pyCSize;
static int      pCurX;
static int      pCurY;

/* --------------------------------------------------------------------
**
** gw_disable_printing  --  disable printing items.
*/

void gw_disable_printing (GUI_WINDOW *gwp)

{
    HMENU       hMenu;

    hMenu = GetMenu (gwp->frameHWnd);
    EnableMenuItem (hMenu, IDM_FILE_PRINT, MF_GRAYED);
    EnableMenuItem (hMenu, IDM_FILE_PRINTSETUP, MF_GRAYED);
    if (gwp->toolbarHWnd)
        SendMessage (gwp->toolbarHWnd, TB_ENABLEBUTTON, IDM_FILE_PRINT,
                     MAKELONG (FALSE, 0));
}

/* --------------------------------------------------------------------
**
** gw_set_default_printer  --  set the default printer.
*/

void gw_set_default_printer (char *pDevice, char *pDriver, char *pOutput)

{
    if (szDevice != NULL) {
        free (szDevice);
        szDevice = NULL;
	}
	if (pDevice != NULL) {
        szDevice = malloc (strlen (pDevice) + 1);
        strcpy (szDevice, pDevice);
	}
    
    if (szDriver != NULL) {
        free (szDriver);
        szDriver = NULL;
	}
	if (pDriver != NULL) {
        szDriver = malloc (strlen (pDriver) + 1);
        strcpy (szDriver, pDriver);
	}

    if (szOutput != NULL) {
        free (szOutput);
        szOutput = NULL;
	}
	if (pOutput != NULL) {
        szOutput = malloc (strlen (pOutput) + 1);
        strcpy (szOutput, pOutput);
	}
}

/* --------------------------------------------------------------------
**
** gw_get_default_printer  --  get the default printer.
*/

void gw_get_default_printer (void)

{
    DEVNAMES        *dvnp;

    /* fill the structure */
    memset (&gwpdlg, 0, sizeof (PRINTDLG));
    gwpdlg.lStructSize = sizeof (PRINTDLG);
    gwpdlg.Flags = PD_RETURNDEFAULT;

    /* call the common dialog manager */
    if (PrintDlg (&gwpdlg)) {

        /* get device names */
        dvnp = GlobalLock (gwpdlg.hDevNames);
        gw_set_default_printer ((char *)dvnp + dvnp->wDeviceOffset,
                                (char *)dvnp + dvnp->wDriverOffset,
                                (char *)dvnp + dvnp->wOutputOffset);
        GlobalUnlock (gwpdlg.hDevNames);
    }
    else {

        /* disable printing */
        gw_printing_ok = 0;
    }
}

/* --------------------------------------------------------------------
**
** gwbefore  --  prepare a print job.
*/

static void gwbefore (int minor, void (*draw) (_CHAR_ ch))

{
    LOGFONT         lf;
    DOCINFO         di;
    WINDOW          pwin;
    TEXTMETRIC      tm;
    char            optstring[80];
    HDC             dc;
    CHOOSEFONT      cf;

    /* check that we have a active view */
    oktoprint = 0;
    if ((pgwp = gw_find_client (GetFocus ())) == NULL)
        return;

    /* get the associated window */
    pwin = winofgw ((GUIWIN *)pgwp);

    /* try to create a display context */
    if ((printerDC = CreateDC (szDriver, szDevice, szOutput, NULL)) == NULL)
        return;
	
    /*  make the display context use transparent background. */
    SetMapMode (printerDC, MM_TEXT);
    SetBkMode (printerDC, TRANSPARENT);

    /* get font type */
    memset (&lf, 0, sizeof (LOGFONT));
    opt_parse_font (o_font (pgwp), &lf);
    opt_parse_attr (o_lpcolor ? o_normalstyle (pgwp) : "", &lf);
   
    /* remap font to correct aspect ratio  */
    dc = GetDC (NULL);
    lf.lfHeight = MulDiv (lf.lfHeight, 72, GetDeviceCaps (dc, LOGPIXELSY));
    ReleaseDC (NULL, dc);
    lf.lfHeight = -MulDiv (lf.lfHeight,
                           GetDeviceCaps (printerDC, LOGPIXELSY), 72);

    /* create the fonts */
    pnFont = CreateFontIndirect (&lf);
    if (SelectObject (printerDC, pnFont) == NULL) {

        /* no corresponding user font, ask user to choose */
		DeleteObject (pnFont);
        memset (&cf, 0, sizeof (CHOOSEFONT));
        cf.lStructSize = sizeof (CHOOSEFONT);
        cf.hDC = printerDC;
        cf.lpLogFont = &lf;
        cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_PRINTERFONTS;
        ChooseFont (&cf);
        pnFont = CreateFontIndirect (&lf);
        if (SelectObject (printerDC, pnFont) == NULL) {
            ReleaseDC (NULL, printerDC);
            MessageBox (NULL, "Invalid Font For Printer", "WinElvis",
                        MB_ICONERROR | MB_OK);
            return;
        }
    }

    GetTextMetrics (printerDC, &tm);
    pxCSize = tm.tmAveCharWidth;
    pyCSize = tm.tmHeight + tm.tmExternalLeading;
    
    opt_parse_attr (o_lpcolor ? o_fixedstyle (pgwp) : "", &lf);
    pfFont = CreateFontIndirect (&lf);
    
    opt_parse_attr (o_lpcolor ? o_boldstyle (pgwp) : "b", &lf);
    pbFont = CreateFontIndirect (&lf);
    
    opt_parse_attr (o_lpcolor ? o_emphasizedstyle (pgwp) : "b", &lf);
    peFont = CreateFontIndirect (&lf);
    
    opt_parse_attr (o_lpcolor ? o_italicstyle (pgwp) : "i", &lf);
    piFont = CreateFontIndirect (&lf);
    
    opt_parse_attr (o_lpcolor ? o_underlinedstyle (pgwp) : "u", &lf);
    puFont = CreateFontIndirect (&lf);
    
    /* misc flags */
    pNewPage = 1;
    
    /* get page size */
    pxSize = GetDeviceCaps (printerDC, HORZRES);
    pySize = GetDeviceCaps (printerDC, VERTRES);
	pyOffset = GetDeviceCaps (printerDC, LOGPIXELSY) / 2;
	pxOffset = GetDeviceCaps (printerDC, LOGPIXELSX) / 2;
    
    /* set size options */
    sprintf (optstring, ":set! lplines=%d lpcolumns=%d\r",
             (pySize - pyOffset * 2) / pyCSize,
             (pxSize - pxOffset * 2) / pxCSize);
	eventex ((GUIWIN *)pgwp, optstring, False);
    pCurX = pxOffset;
    pCurY = pyOffset;
    
    /* inform printer of new job */
    memset (&di, 0, sizeof (DOCINFO));
    di.cbSize = sizeof (DOCINFO);
    di.lpszDocName = (char *)o_filename (pwin->cursor->buffer),
    di.lpszOutput = NULL;
    StartDoc (printerDC, &di);
    
    oktoprint = 1;
}

/* --------------------------------------------------------------------
**
** gwfontch  --  print a char.
*/

static void gwfontch (_char_ font, _CHAR_ ch)

{
    HFONT       pFont;
    COLORREF    fgc;

    /* check if we can print */
    if (!oktoprint)
        return;

	/* check for a new page */
	if (pNewPage) {
	    StartPage (printerDC);
	    pNewPage = 0;
	    pCurX = pxOffset;
	    pCurY = pyOffset;
	}

	/* select font */
	switch (font) {
        case 'f':
        case 'F':
            pFont = pfFont;
            fgc = pgwp->colors.ffgcolor;
            break;
        case 'b':
        case 'B':
            pFont = pbFont;
            fgc = pgwp->colors.bfgcolor;
            break;
        case 'e':
        case 'E':
            pFont = peFont;
            fgc = pgwp->colors.efgcolor;
            break;
        case 'i':
        case 'I':
            pFont = piFont;
            fgc = pgwp->colors.ifgcolor;
            break;
        case 'u':
        case 'U':
            pFont = puFont;
            fgc = pgwp->colors.ufgcolor;
            break;
		default:
	        pFont = pnFont;
            fgc = pgwp->colors.fgcolor;
	        break;
	}

	if (pFont != pCurFont) {
	    pCurFont = pFont;
	    SelectObject (printerDC, pCurFont);
	    if (o_lpcolor)
	        SetTextColor (printerDC, fgc);
	}

	/* print the char */
	if (ch == '\n') {
	    pCurX = pxOffset;
	    pCurY += pyCSize;
	}
	else if (ch != '\r') {
		if (font == 'g' || font == 'G') {
			if (CHARchr("|123456", ch)) {
				MoveToEx(printerDC, pCurX + pxCSize/2, pCurY + pyCSize/2, NULL);
				LineTo(printerDC, pCurX + pxCSize/2, pCurY);
			}
			if (CHARchr("|456789", ch)) {
				MoveToEx(printerDC, pCurX + pxCSize/2, pCurY + pyCSize/2, NULL);
				LineTo(printerDC, pCurX + pxCSize/2, pCurY + pyCSize);
			}
			if (CHARchr("-235689", ch)) {
				MoveToEx(printerDC, pCurX + pxCSize/2, pCurY + pyCSize/2, NULL);
				LineTo(printerDC, pCurX, pCurY + pyCSize/2);
			}
			if (CHARchr("-124578", ch)) {
				MoveToEx(printerDC, pCurX + pxCSize/2, pCurY + pyCSize/2, NULL);
				LineTo(printerDC, pCurX + pxCSize, pCurY + pyCSize/2);
			}
		} else {
			TextOut (printerDC, pCurX, pCurY, (char *)&ch, 1);
		}
	    pCurX += pxCSize;
	}
}

/* --------------------------------------------------------------------
**
** gwpage  --  start a new page.
*/

static void gwpage (int linesleft)

{
    if (oktoprint) {
	    EndPage (printerDC);
	    pNewPage = 1;
	}
}

/* --------------------------------------------------------------------
**
** gwafter  --  terminate print job.
*/

static void gwafter (int linesleft)

{
    if (oktoprint) {

        /* tell printer it's over */
		EndPage (printerDC);
		EndDoc (printerDC);
	
		/* misc flags */
		pNewPage = 0;
	
		/* delete the fonts. */
		pCurFont = 0;
		DeleteObject (pnFont);
		DeleteObject (pfFont);
		DeleteObject (pbFont);
		DeleteObject (peFont);
		DeleteObject (piFont);
		DeleteObject (puFont);
    
		/* delete the device context. */
		DeleteDC (printerDC);
		printerDC = NULL;
		oktoprint = 0;
	}
}

/* -------------------------------------------------------------------- */

LPTYPE  lpwindows = {
    "windows",
    0,
    False,
    gwbefore,
    gwfontch,
    gwpage,
    gwafter
};

#endif
/* ex:set ts=4 sw=4: */
