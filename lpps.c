/* lpps.c */
/* Copyright 1995 by Steve Kirkendall */

char id_lpps[] = "$Id: lpps.c,v 2.11 1996/07/11 15:33:20 steve Exp $";


/* This file contains a driver for Adobe PostScript.  The driver supports
 * printing text either 1-up or 2-up.
 */


#include "elvis.h"

#if USE_PROTOTYPES
static void out(char *str, char *param);
static void psmove(int newrow, int newcol);
static void psstring(_char_ newfont);
static void pagestart(void);
static void pageend(BOOLEAN final);
static void before(int minorno, void (*draw)(_CHAR_));
static void fontch(_char_ font, _CHAR_ ch);
static void page(int linesleft);
static void after(int linesleft);
#endif

/* These variables store the current state of the driver */
static int	elvrow;	/* Row where elvis thinks cursor is */
static int	elvcol;	/* Column where elvis thinks cursor is */
static char	elvfont;/* Current elvis font - one of n/b/i/u/g */
static int	psrow;	/* Row where cursor is located */
static int	pscol;	/* Column where cursor is located */
static char	psfont;	/* Current PS font - one of n/b/i */
static BOOLEAN	instr;	/* Are we in the middle of outputting a string? */
static int	width;	/* width of current string, measured in characters */
static BOOLEAN	twoup;	/* Print two logical pages on each piece of paper? */
static BOOLEAN	even;	/* True during printing of second logical page */
static long	pagenum;/* Physical page number */


/* This is a pointer to the draw() function to use for outputing individual
 * characters.  It is set by the before() function, and remains valid until
 * the after() function returns.
 */
static void (*prtchar) P_((_CHAR_ ch));


/* Output a constant string to the printer, optionally followed by another
 * string, and then an implied newline.  This is used mostly for outputting
 * the header.
 */
static void out(str, param)
	char	*str;	/* string to be output */
	char	*param;	/* optional second string */
{
	while (*str)
	{
		(*prtchar)((_CHAR_)*str++);
	}
	while (param && *param)
	{
		(*prtchar)((_CHAR_)*param++);
	}
	(*prtchar)((_CHAR_)'\n');
}


/* This function moves the PostScript cursor to a given row/column, if it
 * isn't there already.
 */
static void psmove(newrow, newcol)
	int	newrow;	/* new row number, with 0 being the top of the page */
	int	newcol;	/* column number, with 0 being the leftmost column */
{
	char	buf[50];
	int	psx, psy;	/* PostScript coordinates for position */

	/* If the PS cursor is already there, then do nothing */
	if (newrow == psrow && newcol == pscol)
		return;

	/* Convert row/column position to PostScript coordinates.  Here, we
	 * assume that each character is 1/10 of an inch wide and 1/6 of an
	 * inch high.  We also assume that the font's baseline is about 1/6
	 * of the way up the character cell.
	 */
	psx = newcol * 72;
	psy = (o_lplines - newrow) * 120 - 100;

	/* Output a "move" command */
	sprintf(buf, "%d.%d %d.%d moveto", psx/10, psx%10, psy/10, psy%10);
	out(buf, NULL);
	psrow = newrow;
	pscol = newcol;
}


/* This function starts and/or ends a string.  It also handles font changes */
static void psstring(newfont)
	_char_	newfont;	/* font character of font to load */
{
	char	newps;		/* new postscript font */
	int	oldcol;

	/* If fonts haven't changed, we don't need to do anything */
	if (newfont == elvfont && instr)
		return;

	/* If we were in a string before, we need to draw it now. */
	if (instr)
	{
		/* end the string */
		(*prtchar)(')');

		/* drawing underlined strings requires extra work */
		if (elvfont == 'u')
		{
			/* Output the string, and also find its width */
			out(" dup show stringwidth pop", NULL);

			/* Draw a horizontal line as wide as the string */
			out(" newpath", NULL);
			oldcol = pscol;
			pscol = -1;
			psmove(elvrow, oldcol);
			out(" 0 -1 rmoveto dup 0 rlineto neg 1 rmoveto stroke", NULL);

			/* That leaves the PS cursor position undefined */
			pscol = psrow = -1;
		}
		else
		{
			/* output the string */
			out(" show", NULL);
			pscol += width;
		}

		/* we are no longer in a string */
		instr = False;
		width = 0;
	}

	/* if no new font is specified, we're done */
	if (!newfont)
		return;

	/* Move the cursor to where the new string will be output */
	psmove(elvrow, elvcol);

	/* If necessary, switch postscript fonts */
	if (newfont == 'b' || newfont == 'e')
		newps = 'b';
	else if (newfont == 'i')
		newps = 'i';
	else
		newps = 'n';
	if (newps != psfont)
	{
		switch (newps)
		{
		  case 'n':	out("ElvisN", " setfont");	break;
		  case 'b':	out("ElvisB", " setfont");	break;
		  case 'i':	out("ElvisI", " setfont");	break;
		}
		psfont = newps;
	}
	elvfont = newfont;

	/* start outputing a new string */
	(*prtchar)('(');

	/* we are now in a string */
	instr = True;
}


/* Prepare for the next logical page.  For one-up printing, this is fairly
 * simple.  For two-up, it depends on whether this is the first logical page
 * on this sheet of paper, or the second logical page.
 */
static void pagestart()
{
	char	buf[12];

	/* reset variables */
	elvrow = elvcol = 0;
	elvfont = psfont = '\0';
	psrow = psfont = -1; /* to force movement the first time */
	instr = False;
	width = 0;

	/* output a comment, if at the top of a physical page */
	if (!twoup || !even)
	{
		sprintf(buf, "%ld", pagenum++);
		out("%%Page: page ", buf);
	}

	/* change graphic context, depending on printing style */
	if (!twoup)
	{
		out("gsave ElvisPage", NULL);
	}
	else if (!even)
	{
		out("gsave ElvisLeftPage", NULL);
	}
	else
	{
		out("gsave ElvisRightPage", NULL);
	}
}


/* This function ends a logical page */
static void pageend(final)
	BOOLEAN	final;	/* is this the last page of the print job? */
{
	/* end the current string, if any */
	psstring('\0');

	out("grestore", NULL);
	if (!twoup || even || final)
	{
		out("showpage", NULL);
	}
	even = (BOOLEAN)!even;
}



/* This is the before() function.  It sets the prtchar variable, and outputs
 * the PostScript header.
 */
static void before(minorno, draw)
	int	minorno;		/* ignored */
	void	(*draw) P_((_CHAR_));	/* function for sending single char to printer */
{
	char	*pathname;
	FILE	*fp;
	int	ch;
	char	paper[20];
	int	i, j;

	/* Set the ptype and out function */
	twoup = (BOOLEAN)(minorno == 2);
	prtchar = draw;

	/* Output the basic header */
	out("%!PS-Adobe-2.0", NULL);
	out("%%Creator: Elvis 2.0", NULL);
	out("%%EndComments", "\n");

	out("%%BeginProlog", NULL);

	/* output the paper size.  This can be used by the lib/elvis.ps file
	 * to adjust its size and positions.
	 */
	if (!o_lppaper || !*o_lppaper)
	{
		strcpy(paper, "(letter) def");
	}
	else
	{
		paper[0] = '(';
		for (i = 0, j = 1; o_lppaper[i]; i++)
		{
			if (isupper(o_lppaper[i]))
				paper[j++] = tolower(o_lppaper[i]);
			else if (isalnum(o_lppaper[i]))
				paper[j++] = o_lppaper[i];
		}
		strcat(paper, ") def");
	}
	out("/ElvisPaper ", paper);
	sprintf(paper, "%ld def", o_lplines);
	out("/ElvisLines ", paper);
	sprintf(paper, "%ld def", o_lpcolumns);
	out("/ElvisColumns ", paper);

	/* Define the fonts that we'll be using.  Note that we don't define
	 * fonts for 'g' and 'u' because they're done using graphics and the
	 * 'n' font.
	 *
	 * Also define some page positioning macros.
	 */
	pathname = iopath(tochar8(o_elvispath), "elvis.ps", False);
	if (pathname && (fp = fopen(pathname, "r")) != NULL)
	{
		/* Copy definitions from the "lib/elvis.ps" file */
		while ((ch = getc(fp)) != EOF)
		{
			(*prtchar)((_CHAR_)ch);
		}
		fclose(fp);
	}
	else
	{
		/* Use default definitions. These are not sensitive to the
		 * lppaper option.
		 */
		out("/ElvisN /Courier findfont 12 scalefont def", NULL);
		out("/ElvisB /Courier-Bold findfont 12 scalefont def", NULL);
		out("/ElvisI /Courier-Oblique findfont 12 scalefont def", NULL);
		out("/ElvisPage { 12 36 translate } def", NULL);
		out("/ElvisLeftPage { 12 750 translate -90 rotate 0.58 0.75 scale } def", NULL);
		out("/ElvisRightPage { newpath 12 394 moveto 576 0 rlineto stroke 12 366 translate -90 rotate 0.58 0.75 scale } def", NULL);
	}
	out("%%EndProlog", "\n");

	/* Prepare for first page */
	even = False;
	pagenum = 1;
	pagestart();
}



/* This function adds a character to the output, simulating the behavior of
 * control characters.  (The only possible control character is '\n'.)
 */
static void fontch(font, ch)
	_char_	font;	/* font of the next character from text image */
	_CHAR_	ch;	/* the next character */
{
	char	buf[10];

	if (ch == '\n')
	{
		/* end the current string, if any */
		psstring('\0');
		(*prtchar)('\n');

		/* move to the start of the next line */
		elvcol = 0;
		elvrow++;
	}
	else if (font == 'g' && strchr("123456789|-", (_char_)ch))
	{
		/* End any current string. */
		psstring('\0');

		/* Move to the center of the character cell */
		out("newpath", NULL);
		pscol = -1;
		psmove(elvrow, elvcol);
		out("3.6 4.0 rmoveto", NULL);

		/* Maybe draw an uptick */
		if (strchr("123456|", (_char_)ch))
			out(" 0 6.0 rlineto 0 -6.0 rmoveto", NULL);

		/* Maybe draw a downtick */
		if (strchr("456789|", (_char_)ch))
			out(" 0 -6.0 rlineto 0 6.0 rmoveto", NULL);

		/* Maybe draw a lefttick */
		if (strchr("235689-", (_char_)ch))
			out(" -3.6 0 rlineto 3.6 0 rmoveto", NULL);

		/* Maybe draw a righttick */
		if (strchr("124578-", (_char_)ch))
			out(" 3.6 0 rlineto -3.6 0 rmoveto", NULL);

		/* Draw those lines, and move to next character cell */
		out(" stroke", NULL);
		elvcol++;
		pscol = -1;
	}
	else
	{
		/* start a new string, if necessary */
		psstring(font);

		/* Add the character to the current string */
		if (ch == '(' || ch == ')' || ch == '\\')
		{
			(*prtchar)('\\');
			(*prtchar)(ch);
		}
		else if (ch >= 127)
		{
			sprintf(buf, "%03o", ch);
			(*prtchar)('\\');
			(*prtchar)((_CHAR_)buf[0]);
			(*prtchar)((_CHAR_)buf[1]);
			(*prtchar)((_CHAR_)buf[2]);
		}
		else
		{
			(*prtchar)(ch);
		}
		elvcol++;
	}
}


/* This function is called after every page except the last one */
static void page(linesleft)
	int	linesleft;	/* lines remaining on page */
{
	/* end the page */
	pageend(False);

	/* start the next page */
	pagestart();
}


/* This function is called at the end of the print job.  It can output a
 * final formfeed, restore fonts, or whatever.
 */
static void after(linesleft)
	int	linesleft;	/* lines remaining on final page */
{
	/* print the last page */
	pageend(True);

	/* output a trailer */
	out("%%Trailer", NULL);
}


/* These describe the printer types supported by these functions */
LPTYPE lpps =	{"ps", 1, True, before, fontch, page, after};
LPTYPE lpps2 =	{"ps2", 2, True, before, fontch, page, after};
