/*
** gwprint.c -- printing support for MS-Windows elvis.
**
** Copyright 1996, Serge Pirotte
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
#ifndef MB_ICONERROR
# define MB_ICONERROR 0
#endif

PRINTDLG        gwpdlg;
int             gw_printing_ok = 1;

GUI_WINDOW      *pgwp;
    
static char     *szDevice = NULL;
static char     *szDriver = NULL;
static char     *szOutput = NULL;

/* This is an array of fonts with every combination of bold/italic/underline
 * attributes.  They are prescaled for the page format.
 */
static HFONT	pfonts[16];

/* This is the current font, from pfonts[16] */
static int      pNewPage = 0;

static HDC      printerDC;
static int      oktoprint = 0;

/* This following are sizes & position, in pixels */
static int      pxOffset, pyOffset;	/* position of upper-left corner */
static int      pxSize, pySize;		/* page size */
static int      pxCSize, pyCSize;	/* character size */
static int      pCurX, pCurY;		/* current cursor location */

/* The following are used to collect segments of chars in same font */
static CHAR	*segbuf;	/* dynamic buffer, holds up to 1 row of text */
static int	seglen;		/* number of chars currently in segbuf */
static int	segfont;	/* font code of chars in segbuf */
static int	segleftbox;	/* boolean: draw left edge of box? */

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
    CHOOSEFONT      cf;
    HFONT           basefont;
    int             i;

    /* check that we have an active Windows window for elvis */
    oktoprint = 0;
    if ((pgwp = gw_find_client (GetFocus ())) == NULL)
	return;

    /* get the associated elvis window, so we can check its options */
    pwin = winofgw ((GUIWIN *)pgwp);

    /* try to create a display context for the printer */
    if ((printerDC = CreateDC (szDriver, szDevice, szOutput, NULL)) == NULL)
	return;
    
    /*  make the display context use transparent background. */
    SetMapMode (printerDC, MM_TEXT);
    SetBkMode (printerDC, TRANSPARENT);

    /* get font type */
    memset (&lf, 0, sizeof (LOGFONT));
    opt_parse_font (o_font (pgwp), &lf);
    lf.lfWeight = FW_NORMAL;
    lf.lfItalic = FALSE;
    lf.lfUnderline = FALSE;
   
    /* Get page size.  Adjust the size to allow the desired number of rows
     * and columns to fit on the page.
     */
    pySize = GetDeviceCaps (printerDC, VERTRES);
    pxSize = GetDeviceCaps (printerDC, HORZRES);
    lf.lfHeight = pySize / o_lplines;
    lf.lfWidth = pxSize / o_lpcolumns;
#if 0
    pyOffset = GetDeviceCaps (printerDC, LOGPIXELSY) / 2;
    pxOffset = GetDeviceCaps (printerDC, LOGPIXELSX) / 2;
#else
    pyOffset = 0;
    pxOffset = 0;
#endif

    /* create the fonts */
    basefont = CreateFontIndirect (&lf);
    if (SelectObject (printerDC, basefont) == NULL)
    {
	/* no corresponding user font, ask user to choose */
	DeleteObject (basefont);
	memset (&cf, 0, sizeof (CHOOSEFONT));
	cf.lStructSize = sizeof (CHOOSEFONT);
	cf.hDC = printerDC;
	cf.lpLogFont = &lf;
	cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_PRINTERFONTS;
	ChooseFont (&cf);
	basefont = CreateFontIndirect (&lf);
	if (SelectObject (printerDC, basefont) == NULL)
	{
	    ReleaseDC (NULL, printerDC);
	    MessageBox (NULL, "Invalid Font For Printer", "WinElvis",
			MB_ICONERROR | MB_OK);
	    return;
	}
    }
    DeleteObject (basefont);

    /* Remember the size of the character cell. */
    GetTextMetrics (printerDC, &tm);
    pxCSize = tm.tmAveCharWidth;
    pyCSize = tm.tmHeight + tm.tmExternalLeading;
 
    /* Generate all varieties of the font with bold/italic/underline */
    for (i = 0; i < QTY(pfonts); i++)
    {
	if (i == 8)
	    opt_parse_font (o_propfont (pgwp), &lf);
	lf.lfWeight = (i & 1) ? FW_BOLD : FW_NORMAL;
	lf.lfItalic = (i & 2) ? TRUE : FALSE;
	lf.lfUnderline = (i & 4) ? TRUE : FALSE;
	
	pfonts[i] = CreateFontIndirect (&lf);

	/* if proportional font isn't supported, then use fixed */
	if (!pfonts[i] && i >= 8)
		pfonts[i] = pfonts[i - 8];
    }

    /* allocate a segment buffer */
    segbuf = (CHAR *)safealloc(o_lpcolumns, sizeof(CHAR));
    seglen = 0;
    segfont = 0;
    segleftbox = ElvFalse;

    /* The first output will begin a new page. */
    pNewPage = 1;
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
** gwsegment  -- draw a segment of chars in a given font.
*/

static void gwsegment(_char_ nextfont)
{
    int		bits;
    SIZE	size;
    LOGFONT	lf;
    HFONT	adjfont;
    COLORREF	fgc;
    int		i;
    unsigned char *rgb;

    /* if there is text to be drawn... */
    if (seglen > 0)
    {
	/* get the attribute bits.  Synthesize the LEFTBOX and RIGHTBOX bits */
	bits = colorinfo[segfont].da.bits;
	if (bits & COLOR_BOXED)
	{
	    if (segleftbox)
		bits |= COLOR_LEFTBOX;
	    if (~colorinfo[nextfont].da.bits & COLOR_BOXED)
		bits |= COLOR_RIGHTBOX;
	}

	/* choose the font */
	i = 0;
	if (bits & COLOR_BOLD) i += 1;
	if (bits & COLOR_ITALIC) i += 2;
	if (bits & COLOR_UNDERLINED) i += 4;
	if (bits & COLOR_PROP) i += 8;
	SelectObject (printerDC, pfonts[i]);

	/* select fg color */
	rgb = lpfg((_char_)segfont);
	fgc = RGB((long)rgb[0], (long)rgb[1], (long)rgb[2]);
	SetTextColor (printerDC, fgc);

	/* draw the text.  This is tricky for proportional text */
	if (~bits & COLOR_PROP)
	{
	    /* fixed pitch - draw the text */
	    TextOut (printerDC, pCurX, pCurY, (char *)segbuf, seglen);
	}
	else
	{
	    /* first, see how wide the text would be in unadjusted font */
	    GetTextExtentPoint32(printerDC, (char *)segbuf, seglen, &size);

	    /* create an adjusted font, which causes this particular segment
	     * to be as wide as it would have been if printed in fixed font
	     */
	    memset (&lf, 0, sizeof (LOGFONT));
	    opt_parse_font (o_propfont (pgwp), &lf);
	    lf.lfWeight = (bits & COLOR_BOLD) ? FW_BOLD : FW_NORMAL;
	    lf.lfItalic = (bits & COLOR_ITALIC) ? TRUE : FALSE;
	    lf.lfUnderline = (bits & COLOR_UNDERLINED) ? TRUE : FALSE;
	    lf.lfHeight = pyCSize;
	    lf.lfWidth = (pxSize * pxCSize * seglen) / (o_lpcolumns * size.cx);
	    adjfont = CreateFontIndirect (&lf);
	    SelectObject (printerDC, adjfont);

	    /* draw the text, using the adjusted font */
	    TextOut (printerDC, pCurX, pCurY, (char *)segbuf, seglen);

	    /* free the adjusted font */
	    SelectObject (printerDC, pfonts[i]);
	    DeleteObject (adjfont);
	}

	/* if boxed, then draw a box around it */
	if (bits & COLOR_BOXED)
	{
	    MoveToEx(printerDC, pCurX, pCurY, NULL);
	    LineTo(printerDC, pCurX + pxCSize * seglen, pCurY);
	    MoveToEx(printerDC, pCurX, pCurY + pyCSize, NULL);
	    LineTo(printerDC, pCurX + pxCSize * seglen, pCurY + pyCSize);
	    if (bits & COLOR_LEFTBOX)
	    {
		MoveToEx(printerDC, pCurX, pCurY, NULL);
		LineTo(printerDC, pCurX, pCurY + pyCSize);
	    }
	    if (bits & COLOR_RIGHTBOX)
	    {
		MoveToEx(printerDC, pCurX + pxCSize * seglen, pCurY, NULL);
		LineTo(printerDC, pCurX + pxCSize * seglen, pCurY + pyCSize);
	    }
	}
    }

    /* prepare for next segment */
    pCurX += pxCSize * seglen;
    segleftbox = (ELVBOOL)((colorinfo[segfont].da.bits & COLOR_BOXED) == 0
		        && (colorinfo[nextfont].da.bits & COLOR_BOXED) != 0);
    seglen = 0;
    segfont = nextfont;
}

/* --------------------------------------------------------------------
**
** gwfontch  --  print a char.
*/

static void gwfontch (_char_ fontcode, _CHAR_ ch)
{
    COLORREF    fgc;

    /* check if we can print */
    if (!oktoprint)
	return;

    /* check for a new page */
    if (pNewPage)
    {
	gwsegment(0);
	StartPage (printerDC);
	pNewPage = 0;
	pCurX = pxOffset;
	pCurY = pyOffset;
    }

    /* print the char */
    if (ch == '\n')
    {
	gwsegment(0);
	pCurX = pxOffset;
	pCurY += pyCSize;
    }
    else if ((colorinfo[fontcode].da.bits & COLOR_GRAPHIC) == COLOR_GRAPHIC)
    {
	gwsegment(0);

	/* select fg color */
	fgc = o_lpcolor ? RGB(colorinfo[segfont].lpfg_rgb[0],
			      colorinfo[segfont].lpfg_rgb[1],
			      colorinfo[segfont].lpfg_rgb[2])
			: RGB(0,0,0);
	SetTextColor (printerDC, fgc);
	
	/* draw the graphic character */
	if (CHARchr("|123456", ch))
	{
	    MoveToEx(printerDC, pCurX + pxCSize/2, pCurY + pyCSize/2, NULL);
	    LineTo(printerDC, pCurX + pxCSize/2, pCurY);
	}
	if (CHARchr("|456789", ch))
	{
	    MoveToEx(printerDC, pCurX + pxCSize/2, pCurY + pyCSize/2, NULL);
	    LineTo(printerDC, pCurX + pxCSize/2, pCurY + pyCSize);
	}
	if (CHARchr("-235689", ch))
	{
	    MoveToEx(printerDC, pCurX + pxCSize/2, pCurY + pyCSize/2, NULL);
	    LineTo(printerDC, pCurX, pCurY + pyCSize/2);
	}
	if (CHARchr("-124578", ch))
	{
	    MoveToEx(printerDC, pCurX + pxCSize/2, pCurY + pyCSize/2, NULL);
	    LineTo(printerDC, pCurX + pxCSize, pCurY + pyCSize/2);
	}
	if (ch == 'o')
	{
	    int radius = pxCSize / 3;
	    int xcenter = pCurX + pxCSize / 2;
	    int ycenter = pCurY + pyCSize / 2;

	    Arc(printerDC, xcenter - radius, ycenter - radius,
		xcenter + radius, ycenter + radius,
		xcenter - radius, ycenter, xcenter - radius, ycenter);
	}
	if (ch == '*')
	{
	    int radius = pxCSize / 3;
	    int xcenter = pCurX + pxCSize / 2;
	    int ycenter = pCurY + pyCSize / 2;
	    HBRUSH newbrush, oldbrush;

	    newbrush = CreateSolidBrush(fgc);
	    oldbrush = SelectObject(printerDC, newbrush);
	    Pie(printerDC, xcenter - radius, ycenter - radius,
		xcenter + radius, ycenter + radius,
		xcenter - radius, ycenter, xcenter - radius, ycenter);
	    SelectObject(printerDC, oldbrush);
	    DeleteObject(newbrush);
	}
	pCurX += pxCSize;
    }
    else
    {
	if (fontcode != segfont)
	    gwsegment(fontcode);
	segbuf[seglen++] = ch;
    }
}

/* --------------------------------------------------------------------
**
** gwpage  --  start a new page.
*/

static void gwpage (int linesleft)

{
    if (oktoprint)
    {
	gwsegment(0);
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
    int  i;

    if (oktoprint) {

	/* tell printer it's over */
	gwsegment(0);
	EndPage (printerDC);
	EndDoc (printerDC);
    
	/* misc flags */
	pNewPage = 0;
    
	/* delete the fonts. */
	for (i = 0; i < QTY(pfonts); i++)
	    DeleteObject (pfonts[i]);
    
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
    ElvFalse,
    gwbefore,
    gwfontch,
    gwpage,
    gwafter
};

#endif
/* ex:set ts=4 sw=4: */
