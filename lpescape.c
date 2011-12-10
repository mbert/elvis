/* lpescape.c */
/* Copyright 1995 by Steve Kirkendall */

char id_lpescape[] = "$Id: lpescape.c,v 2.13 1996/07/11 15:33:20 steve Exp $";


/* This file contains a driver for printer types which use escape sequences
 * to select fonts.  This includes the "epson", "hp", and "dumb" printer types.
 */


#include "elvis.h"


/* These values are used to index into the following table of escape codes */
typedef enum {BOLD, NOBOLD, UNDLN, NOUNDLN, ITALIC, NOITALIC, BEFORE, AFTER, GCMAP} CODE;

#if USE_PROTOTYPES
static void endfont(void);
static void before(int minorno, void (*draw)(_CHAR_));
static void fontch(_char_ font, _CHAR_ ch);
static void page(int linesleft);
static void after(int linesleft);
#endif

/* This table lists the escape codes used by each printer type */
static char *codes[][9] =
{	/* BOLD	NOBOLD	UNDLN	NOUNDLN	ITALIC	NOITALIC    BEFORE    	AFTER	    GCMAP */
/*epson*/{"E",	"F",	"-1",	"-0",	"4",	"5",	    NULL,	NULL,	    "+++++++++-|*"},
/*pana*/ {"E",	"F",	"-1",	"-0",	"4",	"5",	    "\033t1",	"\033t0",   "\300\301\331\303\305\264\332\302\277\304\263\371"},
/*ibm*/	 {"E",	"F",	"-1",	"-0",	"4",	"5",	    NULL,	NULL,	    "\300\301\331\303\305\264\332\302\277\304\263\371"},
/*hp*/	 {"(s3B","(s0B","&d1D",	"&d@",	"(s1S",	"(s0S",	    "\033(10U",	NULL,	    "\300\301\331\303\305\264\332\302\277\304\263\371"},
/*dumb*/ {NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	    NULL,	NULL,	    "+++++++++-|*"}
};

/* This table is used for converting Latin-1 characters to PC-8 characters.
 * This is necessary because the printer is placed in PC-8 mode so that it can
 * output the graphic characters used by the <pre graphic> tag, and most
 * computers these days use Latin-1 internally.  This table only maps the
 * Latin-1 characters within the range 0xa0-0xff; others don't need conversion.
 * Some conversions are not exact.
 */
static unsigned char topc8[] =
{
	' ',  0xad, 0x9b, 0x9c, '*',  0x9d, '|',  0xf5, '"',  0xe9, 0xa6, 0xae, 0xaa, '-',  'R',  '~',
	0xf8, 0xf1, 0xfd, '3',  '\'', 0xe6, 0xf4, 0xf9, ',',  '1',  0xa7, 0xaf, 0xac, 0xab, '3',  0xa8,
	'A',  'A',  'A',  'A',  0x8e, 0x8f, 0x92, 0x80, 'E',  0x90, 'E',  'E',  'I',  'I',  'I',  'I',
	'D',  0xa5, 'O',  'O',  'O',  'O',  0x99, 'x',  'O',  'U',  'U',  'U',  0x9a, 'Y',  0xe8, 0xe1,
	0x85, 0xa0, 0x83, 'a',  0x84, 0x86, 0x91, 0x87, 0x8a, 0x82, 0x88, 0x89, 0x8d, 0xa1, 0x8c, 0x8b,
	0xeb, 0xa4, 0x95, 0xa2, 0x93, 'o',  0x94, 0xf6, 'o',  0x97, 0xa3, 0x97, 0x81, 'y',  0xe7, 'y'
};
static unsigned char *convert;

/* This value is the minor number of the driver being used.  It is set by the
 * before() function, and remains valid until the after() function returns.
 */
static int ptype;

/* This is a pointer to the draw() function to use for outputing individual
 * characters.  It is set by the before() function, and remains valid until
 * the after() function returns.
 */
static void (*prtchar) P_((_CHAR_ ch));

/* This stores the font currently set for output */
static char curfont = 'n';


/* This function switches off a non-normal font */
static void endfont()
{
	char	*scan;

	switch (curfont)
	{
	  case 'e':
	  case 'b':	scan = codes[ptype][NOBOLD];	break;
	  case 'u':	scan = codes[ptype][NOUNDLN];	break;
	  case 'i':	scan = codes[ptype][NOITALIC];	break;
	  default:	scan = NULL;
	}
	if (scan)
	{
		(*prtchar)('\033');
		while (*scan)
		{
			(*prtchar)((_CHAR_)*scan);
			scan++;
		}
	}
}

/* This is the before() function.  It sets the ptype index, outputs the
 * BEFORE string if it isn't NULL, and sets the convert variable.
 */
static void before(minorno, draw)
	int	minorno;		/* which control codes to use */
	void	(*draw) P_((_CHAR_));	/* function for printing a single character */
{
	char	*scan;

	assert(minorno < QTY(codes));

	/* set the ptype and out function */
	ptype = minorno;
	prtchar = draw;
	curfont = '\n';

	/* if there is a BEFORE string, output it now */
	for (scan = codes[ptype][BEFORE]; scan && *scan; scan++)
	{
		(*prtchar)((_CHAR_)*scan);
	}

	/* if the file appears to use Latin-1, and the lpconvert option is set,
	 * then set the convert pointer to point to the topc8[] array; else
	 * set the convert pointer to NULL.
	 */
	if (o_lpconvert && digraph('A', 'E') == 0xc6)
	{
		convert = topc8;
	}
	else
	{
		convert = NULL;
	}
}

/* This function outputs font-change strings, if necessary, and then outputs
 * a character.
 */
static void fontch(font, ch)
	_char_	font;	/* font of the next character from text image */
	_CHAR_	ch;	/* the next character */
{
	char	*scan;

	/* is the font changing? */
	if (font != curfont)
	{
		/* switch off a non-normal font, if we're in one */
		endfont();

		/* switch on a non-normal font, if we should be in one */
		switch (font)
		{
		  case 'e':
		  case 'b':	scan = codes[ptype][BOLD];	break;
		  case 'u':	scan = codes[ptype][UNDLN];	break;
		  case 'i':	scan = codes[ptype][ITALIC];	break;
		  default:	scan = NULL;
		}
		if (scan)
		{
			(*prtchar)('\033');
			while (*scan)
			{
				(*prtchar)((_CHAR_)*scan);
				scan++;
			}
		}

		/* remember what font we just switched to. */
		curfont = font;
	}

	/* if in graphic mode, convert graphic characters */
	if (font == 'g' && codes[ptype][GCMAP])
	{
		switch (ch)
		{
		  case '1':	ch = codes[ptype][GCMAP][0];	break;
		  case '2':	ch = codes[ptype][GCMAP][1];	break;
		  case '3':	ch = codes[ptype][GCMAP][2];	break;
		  case '4':	ch = codes[ptype][GCMAP][3];	break;
		  case '5':	ch = codes[ptype][GCMAP][4];	break;
		  case '6':	ch = codes[ptype][GCMAP][5];	break;
		  case '7':	ch = codes[ptype][GCMAP][6];	break;
		  case '8':	ch = codes[ptype][GCMAP][7];	break;
		  case '9':	ch = codes[ptype][GCMAP][8];	break;
		  case '-':	ch = codes[ptype][GCMAP][9];	break;
		  case '|':	ch = codes[ptype][GCMAP][10];	break;
		  case '*':	ch = codes[ptype][GCMAP][11];	break;
		}
	}
	else if (convert && ch >= 0xa0)
	{
		ch = convert[ch - 0xa0];
	}

	/* output the character */
	(*prtchar)(ch);
}

/* This function is called after every page except the last one */
static void page(linesleft)
	int	linesleft;	/* number of lines remaining on page */
{
	/* output a formfeed character */
	(*prtchar)('\f');
}

/* This function is called at the end of the print job.  It can output a
 * final formfeed, restore fonts, or whatever.  Here, it just outputs the
 * AFTER string, if there is one.
 */
static void after(linesleft)
	int	linesleft;	/* number of lines remaining on final page */
{
	char	*scan;

	/* if there is an AFTER string, output it now */
	for (scan = codes[ptype][AFTER]; scan && *scan; scan++)
	{
		(*prtchar)((_CHAR_)*scan);
	}

	/* and maybe output a formfeed, too */
	if (o_lpformfeed)
	{
		(*prtchar)((_CHAR_)'\f');
	}
}

/* These describe the printer types supported by these functions */
LPTYPE lpepson ={"epson", 0, True, before, fontch, page, after};
LPTYPE lppana =	{"pana", 1, True, before, fontch, page, after};
LPTYPE lpibm =	{"ibm", 2, True, before, fontch, page, after};
LPTYPE lphp =	{"hp", 3, True, before, fontch, page, after};
LPTYPE lpdumb =	{"dumb", 4, True, before, fontch, page, after};
