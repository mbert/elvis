/* dmmarkup.c */
/* Copyright 1995 by Steve Kirkendall */

char id_dmmarkup[] = "$Id: dmmarkup.c,v 2.43 1996/09/18 16:42:14 steve Exp $";

/* This file contains some fairly generic text formatting code -- generic
 * in the sense that it can be easily tweaked to format a variety of types
 * marked-up text.  Currently, it supports useful subsets of NROFF and HTML
 * instructions.
 */

#include "elvis.h"
#ifdef DISPLAY_MARKUP



#define GRANULARITY 64	/* number of LINEINFOs to allocate at a time */



typedef struct
{
	CHAR	text[200];	/* raw text of token */
	long	offset[200];	/* offsets of characters */
	int	nchars;		/* number of characters in text[] */
	int	width;		/* normal displayed width of text[] */
	struct markup_s	*markup;/* info about markup token */
} TOKEN;

typedef struct markup_s
{
	char	*name;		/* name of the markup */
	char	attr[8];	/* attributes of markup */
	BOOLEAN	(*fn)P_((TOKEN *));/* ptr to special function */	
} MARKUP;
#define TITLE	attr[0]		/* in title: -, N, Y */
#define BREAKLN	attr[1]		/* line break: -, 0, 1, 2, c, or p */
#define INDENT	attr[2]		/* -, <, >, or a number */
#define LIST	attr[3]		/* in list: -, N, Y, # */
#define FONT	attr[4]		/* font: -, =, n, b, u, i, f, e, N, B, U, I, F, E */
#define FILL	attr[5]		/* Y=fill, N=preformatted, -=no chg. */
#define DEST	attr[6]		/* S=section, P=paragraph, T=<tab> key */

typedef struct
{
	long	 offset;	/* offset of start of line */
	struct
	{
	    unsigned indent : 8;  /* indentation amount, in spaces */
	    unsigned listcnt : 8; /* Counter for nest#1 numbered list; 0=not numbered */
	    unsigned nest : 6;	  /* nesting level of list/menu; 0=not in list*/
	    unsigned prefmt : 1;  /* 1=literal whitespace, 0=fill */
	    unsigned graphic : 1; /* 1=replace |-^. with graphic chars */
	    unsigned midline: 1;  /* 1=after a newline, 0=after other char */
	    unsigned reduce : 1;  /* 1=fewer newlines, 0=normal qty newlines */
	    unsigned deffont : 3; /* index into "nbiufe" of default font char */
	    unsigned curfont : 3; /* index into "nbiufe" of current font char */
	}	state;
} LINEINFO;

typedef struct
{
	TOKEN	*(*get)P_((CHAR **));	/* mode-dependent get() */
	void	(*escape)P_((TOKEN *));	/* mode-dependent escape() */
	LINEINFO *line;			/* line array */
	long	nlines;			/* number of lines in line array */
	long	endtitle;		/* offset of the end of the title */
	CHAR	*title;			/* title of document, or NULL */
	CHAR	**defs;			/* macros within the text */
} MUINFO;

static BOOLEAN	first;	/* is this the first token on this line? */
static BOOLEAN	anyspc;	/* has whitespace been encountered? */
static BOOLEAN	title;	/* collecting characters of the title */
static BOOLEAN	list;	/* o_list */
static int	textwidth;/* o_columns */
static int	tabstop;/* o_tabstop */
static int	listind;/* o_shiftwidth/2, or 2 if shiftwidth<=4 */
static int	col;	/* logical column number */
static MUINFO	*mui;	/* pointer to muinfo */

static BOOLEAN	prefmt;	/* True=literal whitespace, False=fill */
static BOOLEAN	graphic;/* True=replace |-^. with graphic chars */
static BOOLEAN	midline;/* False=after newline, True=after other character */
static BOOLEAN	reduce;	/* True=fewer newlines, False=normal qty newlines */
static char	deffont;/* default font */
static char	curfont;/* current font */
static int	indent;	/* indentation amount */
static int	nest;	/* nesting level of list/menu; 0=not in list */
static int	listcnt;/* Counter for nest#1 numbered list; 0=not numbered */

/* These variables store the string and font collected by the manarg() function
 * and output by the manput() function.  The "manlen" variable should be
 * initialized to 0 before the first call to manarg().  NOTE: These variables
 * and the manput() function are also used by htmlimg().
 */
static int	manlen;		/* length of "mantext" string */
static CHAR	mantext[80];	/* buffer, holds args from .XX macro */
static long	manoffset[80];	/* holds offsets of mantext[] chars */
static char	manfont[80];	/* holds fonts of mantext[] chars */

/* Forward declarations of some functions which are static to this file */
static void	htmlescape P_((TOKEN *tok));
static BOOLEAN	htmlhr P_((TOKEN *token));
static BOOLEAN	htmlimg P_((TOKEN *token));
static BOOLEAN	htmlpre P_((TOKEN *token));
static BOOLEAN	htmlli P_((TOKEN *token));
static BOOLEAN	htmlinput P_((TOKEN *token));
static void	htmlmarkup P_((TOKEN *token));
static TOKEN	*htmlget P_((CHAR **refp));
static DMINFO	*htmlinit P_((WINDOW win));
static CHAR	*htmltagatcursor P_((WINDOW win, MARK cursor));
static MARK	htmltagload P_((CHAR *tagname, MARK from));
static MARK	htmltagnext P_((MARK cursor));
static void	manescape P_((TOKEN *tok));
static int	manarg P_((TOKEN *token, int start, _char_ font, BOOLEAN spc));
static BOOLEAN	manput P_((void));
static BOOLEAN	manTH P_((TOKEN *token));
static BOOLEAN	manSH P_((TOKEN *token));
static BOOLEAN	manBI P_((TOKEN *token));
static BOOLEAN	manIP P_((TOKEN *token));
static void	manmarkup P_((TOKEN *token));
static TOKEN	*manget P_((CHAR **refp));
static DMINFO	*maninit P_((WINDOW win));
static void	countchar P_((CHAR *p, long qty, _char_ font, long offset));
static BOOLEAN	put P_((TOKEN *token));
static void	term P_((DMINFO *info));
static long	mark2col P_((WINDOW w, MARK mark, BOOLEAN cmd));
static MARK	move P_((WINDOW w, MARK from, long linedelta, long column, BOOLEAN cmd));
static MARK	setup P_((MARK top, long cursor, MARK bottom, DMINFO *info));
static MARK	image P_((WINDOW w, MARK line, DMINFO *info, void (*draw)(CHAR *p, long qty, _char_ font, long offset)));
static void	header P_((WINDOW w, int pagenum, DMINFO *info, void (*draw)(CHAR *p, long qty, _char_ font, long offset)));
static int	start P_((WINDOW win, MARK from, void (*draw)(CHAR *p, long qty, _char_ font, long offset)));
static void	storestate P_((long offset, LINEINFO *dest));
static void	findtitle P_((BUFFER buf));

/* Only a single TOKEN is ever really needed at one time */
static TOKEN	rettok;

/* Offset of cursor.  This affects the expansion of escapes, and the visibility
 * of markups.
 */
static long	cursoff;

/* Offset of a space character, if "anyspc" is True. */
static long	spcoffset;

/* This the drawchar pointer points to a function for outputting a single
 * character.
 */
static void	(*drawchar) P_((CHAR *p, long qty, _char_ font, long offset));

/* Special characters.  These are stored in variables rather than macros so
 * that we can pass their address to (*drawchar)().
 */
static CHAR	hyphen = '-';
static CHAR	newline = '\n';
static CHAR	formfeed = '\f';
static CHAR	vtab = '\013';
static CHAR	space = ' ';
static CHAR	bullet = '*';


/*----------------------------------------------------------------------------*/
/* HTML-specific functions and variables                                      */

/* Replace entities such as &lt; with their single-character equivelent. */
static void htmlescape(token)
	TOKEN	*token;	/* a token whose text is to be expanded */
{
	char	*src, *dst;
	long	*off;
	size_t	len;
	int	width;

	/* step through the string */
	for (src = dst = (char *)token->text, off = token->offset, width = 0;
	     src < (char *)&token->text[token->nchars];
	     src++, off++, width++)
	{
		/* if not a &, then this can't be an escape */
		if (*src != '&')
			goto NoEscape;

		/* find the length of this escape's name */
		for (len = 1; src[len] != ';'; len++)
		{
			if (!src[len] || isspace(src[len]))
				goto NoEscape;
		}
		len++;

		/* if the cursor is on this escape, then don't expand it */
		if ((o_showmarkups && off[0] <= cursoff && cursoff <= off[len - 1]) || list)
		{
			/* Tweak the value of "width" so that after all of
			 * this escape's characters have been counted, "width"
			 * will have been incremented by only 1 since that's
			 * how wide the escape would normally be.
			 */
			width -= (len - 1);

			/* Don't expand the escape */
			goto NoEscape;
		}

		/* recognize it? */
		if (len == 4 && (!strncmp(&src[1], "lt", 2) || !strncmp(&src[1], "LT", 2)))
			*dst++ = '<';
		else if (len == 4 && (!strncmp(&src[1], "gt", 2) || !strncmp(&src[1], "GT", 2)))
			*dst++ = '>';
		else if (len == 5 && (!strncmp(&src[1], "amp", 3) || !strncmp(&src[1], "AMP", 3)))
			*dst++ = '&';
		else if (len == 6 && (!strncmp(&src[1], "quot", 4) || !strncmp(&src[1], "QUOT", 4)))
			*dst++ = '"';
		else if (len == 6 && (!strncmp(&src[1], "nbsp", 4) || !strncmp(&src[1], "NBSP", 4)))
			*dst++ = ' ';
		else if (len == 7 && !strncmp(&src[1], "AElig", 5))
			*dst++ = digraph('E', 'A');
		else if (len == 7 && !strncmp(&src[1], "aelig", 5))
			*dst++ = digraph('e', 'a');
		else if (len == 5 && !strncmp(&src[1], "ETH", 3))
			*dst++ = digraph('-', 'D');
		else if (len == 5 && !strncmp(&src[1], "eth", 3))
			*dst++ = digraph('-', 'd');
		else if (len == 7 && !strncmp(&src[1], "THORN", 5))
			*dst++ = digraph('T', 'P');
		else if (len == 7 && !strncmp(&src[1], "thorn", 5))
			*dst++ = digraph('t', 'p');
		else if (len == 7 && !strncmp(&src[1], "szlig", 5))
			*dst++ = digraph('s', 'z');
		else if (len == 8 && !strncmp(&src[2], "grave", 5))
			*dst++ = digraph('`', (_CHAR_)src[1]);
		else if (len == 8 && !strncmp(&src[2], "acute", 5))
			*dst++ = digraph('\'', (_CHAR_)src[1]);
		else if (len == 7 && !strncmp(&src[2], "circ", 4))
			*dst++ = digraph('^', (_CHAR_)src[1]);
		else if (len == 8 && !strncmp(&src[2], "tilde", 5))
			*dst++ = digraph('~', (_CHAR_)src[1]);
		else if (len == 6 && !strncmp(&src[2], "uml", 3))
			*dst++ = digraph('"', (_CHAR_)src[1]);
		else if (len == 7 && !strncmp(&src[2], "ring", 4))
			*dst++ = digraph('*', (_CHAR_)src[1]);
		else if (len == 8 && !strncmp(&src[2], "cedil", 5))
			*dst++ = digraph(',', (_CHAR_)src[1]);
		else if (len == 8 && !strncmp(&src[2], "slash", 5))
			*dst++ = digraph('/', (_CHAR_)src[1]);
		else if (src[1] == '#')
			*dst++ = atoi(&src[2]);
		else if (len == 6 && (!strncmp(&src[1], "copy", 4) || !strncmp(&src[1], "COPY", 4)))
			*dst++ = digraph('O', 'c');
		else if (len == 5 && (!strncmp(&src[1], "reg", 3) || !strncmp(&src[1], "REG", 3)))
			*dst++ = digraph('O', 'r');
		else if (len == 7 && (!strncmp(&src[1], "ldquo", 5) || !strncmp(&src[1], "LDQUO", 5)
				|| !strncmp(&src[1], "rdquo", 5) || !strncmp(&src[1], "RDQUO", 5)))
			*dst++ = '"';
		else if (len == 7 && (!strncmp(&src[1], "lsquo", 5) || !strncmp(&src[1], "LSQUO", 5)))
			*dst++ = '`';
		else if (len == 7 && (!strncmp(&src[1], "rsquo", 5) || !strncmp(&src[1], "RSQUO", 5)))
			*dst++ = '\'';
		else
		{
			/* Tweak the value of "width" so that after all of
			 * this escape's characters have been counted, "width"
			 * will have been incremented by only 1 since that's
			 * how wide the escape would normally be if it was
			 * recognized.
			 */
			width -= (len - 1);

			/* Don't expand the escape */
			goto NoEscape;
		}

		/* Skip past the escape sequence. */
		token->offset[(int)(dst - (char *)token->text) - 1] = *off;
		src += len - 1;
		off += len - 1;
		/* plus one more at the for-loop */
		continue;

NoEscape:
		/* Not an escape -- copy it literally */
		*dst++ = *src;
		token->offset[(int)(dst - (char *)token->text) - 1] = *off;
	}

	/* compute the new length */
	token->nchars = (int)(dst - (char *)token->text);
	token->text[token->nchars] = '\0';
	token->width = width;
}

/* output a horizontal rule */
static BOOLEAN htmlhr(token)
	TOKEN	*token;
{
	int	len;
	long	offset;

	assert(col == 0 && textwidth > 0);

	/* Normally the hrule is as wide as the line, but if the cursor is on
	 * the "<hr>" token then we want to shrink it slightly so that "<hr>"
	 * will fit on the same row of the window.  This allows the window to
	 * be updated more efficiently.
	 */
	len = textwidth;
	if (list || (o_showmarkups && token->offset[0] <= cursoff && cursoff <= token->offset[token->nchars - 1]))
	{
		/* leave room for showing the <hr> tag */
		len -= textwidth - token->nchars;
		offset = -1;
	}
	else
	{
		/* make it as wide as the screen; the <hr> won't show */
		len = textwidth;
		offset = token->offset[0];
	}
	if (len < 3)
		len = 3;

	/* draw the hrule */
	(*drawchar)(&space, -1, 'g', -1);
	(*drawchar)(&hyphen, -(len - 2), 'g', offset);
	col = len - 1;
	return False;
}

/* Output the "alt" text from an <img> tag */
static BOOLEAN htmlimg(token)
	TOKEN	*token;
{
	int	i, j;

	/* look for an "alt=..." argument */
	for (i = 5; i < token->nchars && CHARncmp(&token->text[i - 5], toCHAR(" alt="), 5); i++)
	{
	}

	/* decide how to display this image */
	if (i >= token->nchars)
	{
		/* there is no "alt=..." string, so display the whole tag */
		i = 0;
		j = token->nchars;
	}
	else if (token->text[i] == '"')
	{
		/* the "alt=" argument has a quoted argument */
		i++;
		for (j = i; j < token->nchars && token->text[j] != '"'; j++)
		{
		}
	}
	else
	{
		/* the "alt=" argument is unquoted */
		for (j = i; j < token->nchars && token->text[j] != ' ' && token->text[j] != '>'; j++)
		{
		}
	}

	/* copy it into the manput() arguments, and then put it. */
	for (manlen = 0; i < j && manlen < QTY(mantext); i++, manlen++)
	{
		mantext[manlen] = token->text[i];
		manoffset[manlen] = token->offset[i];
		manfont[manlen] = 'b';
	}
	return manput();
}


/* Set the modeinfo's "graphic" flag if <pre graphic> */
static BOOLEAN htmlpre(token)
	TOKEN	*token;
{
	int	i;

	for (graphic = False, i = 0; i < token->nchars; i++)
	{
		if (token->text[i] == 'g')
		{
			graphic = True;
			break;
		}
	}
	return False;
}

/* List items are preceded by a less-indented number or bullet */
static BOOLEAN htmlli(token)
	TOKEN	*token;
{
	CHAR	buf[10];
	int	len;

	assert(col == 0);

	/* output a bullet or count */
	if (nest == 1 && listcnt > 0)
	{
		/* convert item# to characters */
		long2CHAR(buf, (long)listcnt++);
		CHARcat(buf, toCHAR(")"));
		len = CHARlen(buf);

		/* output whitespace for indentation */
		if (indent - len > 1)
		{
			(*drawchar)(&space, 1 + len - indent, 'n', -1);
			col += indent - len - 1;
		}

		/* output the item number */
		(*drawchar)(buf, len, 'n', -1);
		col += len;
	}
	else
	{
		/* output whitespace for indentation */
		if (indent > 2)
		{
			(*drawchar)(&space, 2 - indent, 'n', -1);
			col += indent - 2;
		}

		/* output a bullet */
		(*drawchar)(&bullet, 1, 'g', -1);
		col++;
	}

	/* Note: We would like to do an assert(mui->col == mui->indent - 1)
	 * here, but if the number/bullet doesn't fit within the indentation
	 * space then our indentation might be off.  So we won't.
	 */
	return False;
}

/* Form elements are shown as reverse-video areas */
static BOOLEAN htmlinput(token)
	TOKEN	*token;
{
	int	height;	/* 1 for input, 2 for textarea */
	int	width;	/* displayed width of item */
	int	vallen;	/* length of the value */
	int	validx;	/* index into token->text[] of initial value */
	BOOLEAN	button;	/* does this form item appear to be a button? */
	BOOLEAN	radio;	/* does this form item appear to be a radio button? */
	char	font;	/* font - 'E' for buttons, or 'N' for any other */
	int	mycol;
	int	i;

	/* parse the arguments */
	height = (token->text[1] == 't') ? 3 : 1;
	width = vallen = validx = 0;
	button = radio = False;
	font = 'u';
	for (i = 4; i < token->nchars; i++)
	{
		if (!CHARncmp(&token->text[i], toCHAR("value="), 6))
		{
			i += 6;
			if (token->text[i] == '"')
			{
				i++;
				validx = i;
				for (vallen = i; i < token->nchars && token->text[i] != '"'; i++)
				{
				}
			}
			else
			{
				validx = i;
				for (vallen = i; i < token->nchars && !isspace(token->text[i]); i++)
				{
				}
			}
			vallen = i - vallen;
		}
		else if (!CHARncmp(&token->text[i], toCHAR("size="), 5)
		      || !CHARncmp(&token->text[i], toCHAR("cols="), 5))
		{
			i += 5;
			width = atoi(tochar8(&token->text[i]));
		}
		else if (!CHARncmp(&token->text[i], toCHAR("type="), 5))
		{
			i += 5;
			if (token->text[i] == '"')
				i++;
			if (!CHARncmp(&token->text[i], toCHAR("checkbox"), 8)
			 || !CHARncmp(&token->text[i], toCHAR("CHECKBOX"), 8))
			{
				/* CHECKBOX button */
				button = radio = True;
				i += 8;
			}
			else if (!CHARncmp(&token->text[i], toCHAR("hidden"), 6)
			      || !CHARncmp(&token->text[i], toCHAR("HIDDEN"), 6))
			{
				/* HIDDEN field -- do nothing with it */
				return False;
			}
			else if (!CHARncmp(&token->text[i], toCHAR("radio"), 5)
			      || !CHARncmp(&token->text[i], toCHAR("RADIO"), 5))
			{
				/* RADIO button */
				button = radio = True;
				i += 5;
			}
			else if (token->text[i] != 't' && token->text[i] != 'T')
			{
				/* not TEXT, probably SUBMIT or RESET button */
				button = True;
				font = 'B';
			}
		}
		else if (!CHARncmp(&token->text[i], toCHAR("checked"), 7))
		{
			font = 'N';
			i += 7;
		}
	}

	/* Most buttons are always as wide as their value, but radio & checkbox
	 * buttons only need to show a single character.
	 */
	if (radio)
	{
		vallen = 1;
	}
	if (button)
	{
		width = vallen;
	}

	/* remember the column */
	mycol = col;
	if (mycol < indent)
		mycol = indent;
	else if (anyspc)
	{
		mycol++;
		anyspc = False;
	}

	/* will it fit on this line? */
	if (!first && mycol + width > textwidth)
	{
		/* no it won't */
		(*drawchar)(&newline, 1, 'n', -1);
		col = 0;
		return True;
	}

	/* output the image */
	for (i = 1; i <= height; i++)
	{
		if (col > mycol)
		{
			(*drawchar)(&newline, 1, 'n', -1);
			col = 0;
		}
		if (col < mycol)
		{
			(*drawchar)(&space, col - mycol , 'n', -1);
			col = mycol;
		}
		if (vallen > 0)
			(*drawchar)(&token->text[validx], vallen, font, token->offset[validx]);
		(*drawchar)(&space, vallen - width, font, token->offset[token->nchars - 1]);
		col += width;
	}
	anyspc = False;

	return False;
}

/* Look up an html markup token in a table */
static void htmlmarkup(token)
	TOKEN	*token;	/* the token to lookup */
{
	static MARKUP	tbl[] =
	{

		/* Tag		 Effects	Function	*/
		/*               TBILFFD                        */
		{ "html",	"Y-2-NY-"			},
		{ "/html",	"N-2-NY-"			},
		{ "head",	"Y-2-NY-"			},
		{ "/head",	"N-2-NY-"			},
		{ "title",	"Y-2-NY-"			},
		{ "/title",	"N-2-NY-"			},
		{ "body",	"N-2-NY-"			},
		{ "/body",	"N-2-NY-"			},
		{ "h1",		"Np0-BYS"			},
		{ "/h1",	"N12-NY-"			},
		{ "h2",		"Nc1-BYS"			},
		{ "/h2",	"N12-NY-"			},
		{ "h3",		"N12-BYS"			},
		{ "/h3",	"N02-NY-"			},
		{ "h4",		"N12-IY-"			},
		{ "/h4",	"N02-NY-"			},
		{ "h5",		"N12-IY-"			},
		{ "/h5",	"N02-NY-"			},
		{ "h6",		"N12-IY-"			},
		{ "/h6",	"N02-NY-"			},
		{ "p",		"N1--NYP"			},
		{ "hr",		"N02--Y-",	htmlhr		},
		{ "img",	"N------",	htmlimg		},
		{ "br",		"N0---Y-"			},
		{ "table",	"N02-NY-"			},
		{ "/table",	"N02-NY-"			},
		{ "tr",		"N02--Y-"			},
		{ "th",		"N-=-BY-"			},
		{ "td",		"N-=-NY-"			},
		{ "blockquote",	"N14-NYP"			},
		{ "/blockquote","N12-NY-"			},
		{ "pre",	"N0--FNP",	htmlpre		},
		{ "/pre",	"N0--NY-"			},
		{ "dir",	"N0>-FNP",	htmlpre		},
		{ "/dir",	"N0<-NY-"			},
		{ "xmp",	"N0>-FN-",	htmlpre		},
		{ "/xmp",	"N0<-NY-"			},
		{ "dl",		"N-2-NYS"			},
		{ "/dl",	"N02-NY-"			},
		{ "dt",		"N12-BYP"			},
		{ "dd",		"N03-NY-"			},
		{ "ol",		"N->#-YP"			},
		{ "/ol",	"N0<N-Y-"			},
		{ "ul",		"N->Y-YP"			},
		{ "/ul",	"N0<N-Y-"			},
		{ "menu",	"N->Y-Y-"			},
		{ "/menu",	"N-<N-Y-"			},
		{ "li",		"N0-----",	htmlli		},
		{ "input",	"N-----T",	htmlinput	},
		{ "textarea",	"N-----T",	htmlinput	},
		{ "a",		"N---u-T"			},
		{ "/a",		"N---=--"			},
		{ "cite",	"N---i--"			},
		{ "/cite",	"N---=--"			},
		{ "dfn",	"N---i--"			},
		{ "/dfn",	"N---=--"			},
		{ "em",		"N---i--"			},
		{ "/em",	"N---=--"			},
		{ "kbd",	"N---b--"			},
		{ "/kbd",	"N---=--"			},
		{ "strong",	"N---b--"			},
		{ "/strong",	"N---=--"			},
		{ "var",	"N---i--"			},
		{ "/var",	"N---=--"			},
		{ "address",	"N---i--"			},
		{ "/address",	"N---=--"			},
		{ "b",		"N---b--"			},
		{ "/b",		"N---=--"			},
		{ "i",		"N---i--"			},
		{ "/i",		"N---=--"			},
		{ "u",		"N---u--"			},
		{ "/u",		"N---=--"			},
		{ "tt",		"N---f--"			},
		{ "/tt",	"N---=--"			},
		{ (char *)0,	"N------"			}
	};
	MARKUP	*scan;	/* used for scanning the tbl[] array */
	int	len;	/* length of the markup */

	/* find the length of the markup's name */
	assert(token->nchars > 1 && token->text[0] == '<');
	for (len = 1;
	     len < token->nchars &&
		((len == 1 && token->text[len] == '/') || isalnum(token->text[len]));
	     len++)
	{
	}
	len--; /* since we started at 1 */

	/* look it up in the table */
	for (scan = tbl;
	     scan->name &&
		(strlen(scan->name) != (unsigned)len ||
		    strncmp(scan->name, (char *)token->text + 1, (size_t)len));
	     scan++)
	{
	}

	/* remember it */
	token->markup = scan;
}

/* Read the next token. */
static TOKEN *htmlget(refp)
	CHAR	**refp;	/* address of a (CHAR *) used for scanning */
{
	long	offset;
	BOOLEAN lower;

	/* if the CHAR pointer is NULL, then return NULL */
	if (!*refp)
		return NULL;

	/* Get first character of token */
	offset = markoffset(scanmark(refp));
	rettok.text[0] = **refp;
	rettok.offset[0] = offset++;
	rettok.nchars = 1;
	rettok.markup = NULL;
	scannext(refp);

	/* If '<' then token is a markup */
	if (rettok.text[0] == '<')
	{
		/* This is a markup.  Collect characters up to next '>' */
		for (lower = True; *refp && **refp != '>'; offset++, scannext(refp))
		{
			/* if token text is full, then skip this char */
			if (rettok.nchars >= QTY(rettok.text) - 2)
				continue;

			/* Store the character.  This is a little complex
			 * because we want to convert uppercase tags and
			 * parameter names to lowercase, but leave parameter
			 * values unchanged.  Also, any whitespace character
			 * should be displayed as a space.
			 */
			if (**refp == '=')
				lower = False;
			else if (isspace(**refp))
				lower = True;

			if (**refp <= ' ')
				rettok.text[rettok.nchars] = ' ';
			else if (isupper(**refp) && lower)
				rettok.text[rettok.nchars] = tolower(**refp);
			else
				rettok.text[rettok.nchars] = **refp;
			rettok.offset[rettok.nchars++] = offset;
		}

		/* store the terminating '>' */
		rettok.text[rettok.nchars] = '>';
		rettok.offset[rettok.nchars] = offset;
		rettok.nchars++;
		rettok.text[rettok.nchars] = '\0';
		if (*refp)
			scannext(refp);

		/* lookup the markup */
		htmlmarkup(&rettok);

		/* when computing line breaks, assume this markup is hidden */
		rettok.width = 0;
	}
	else if (rettok.text[0] <= ' ')
	{
		/* This is a whitespace token.  Each whitespace token contains
		 * a SINGLE whitespace character.  Control characters other
		 * than '\t' and '\n' are treated as spaces.
		 */
		if (rettok.text[0] != '\t' && rettok.text[0] != '\n')
		{
			rettok.text[0] = ' ';
		}

		/* when computing line breaks, assume this whitespace shows */
		rettok.width = 1;
	}
	else
	{
		/* This is a word.  Collect characters up to next whitespace */
		for (;
		     *refp 
			&& rettok.nchars < QTY(rettok.text) - 1
			&& !isspace(**refp)
			&& **refp != '<';
		     offset++, scannext(refp))
		{
			rettok.text[rettok.nchars] = **refp;
			rettok.offset[rettok.nchars] = offset;
			rettok.nchars++;
		}

		/* For now, when computing line breaks, assume each character
		 * of this word is normally visible.  When character escapes
		 * are processed, this may change.
		 */
		rettok.width = rettok.nchars;
	}

	/* Mark the end of the text, and return the token */
	rettok.text[rettok.nchars] = '\0';
	return &rettok;
}


/* start the mode, and allocate dminfo */
static DMINFO *htmlinit(win)
	WINDOW	win;
{
	long	cursoffset;	/* offset of cursor */
	TOKEN	*token;
	MARKBUF	top;
	CHAR	*p;

	/* inherit some functions from dmnormal */
	dmhtml.wordmove = dmnormal.wordmove;

	/* allocate the info struct */
	mui = (MUINFO *)safealloc(1, sizeof(MUINFO));
	mui->get = htmlget;
	mui->escape = htmlescape;
	mui->line = safealloc(GRANULARITY, sizeof(LINEINFO));
	mui->nlines = 1; /* every buffer has at least one line */

	/* temporarily move the cursor someplace harmless */
	cursoffset = markoffset(win->cursor);
	marksetoffset(win->cursor, o_bufchars(markbuffer(win->cursor)));

	/* format the buffer */
	win->mi = (DMINFO *)mui;
	(void)start(win, marktmp(top, markbuffer(win->cursor), 0), NULL);
	p = scanalloc(&p, &top);
	for (token = (*mui->get)(&p);
	     token;
	     token = (*mui->get)(&p))
	{
		/* if cursor is at position 0, and this is a text token,
		 * then move the cursor here.  This is done because otherwise
		 * the cursor would always start on an ugly formatting code.
		 */
		if (cursoffset == 0L 
		 && !title
		 && !isspace(token->text[0])
		 && !token->markup)
		{
			cursoffset = token->offset[0];
		}

		/* output the token.  If it forces a new line, remember where
		 * that new line started.
		 */
		if (put(token))
		{
			assert(first == True && col == 0);
			storestate(token->offset[0], NULL);
			(void)put(token);
		}
	}
	scanfree(&p);

	/* locate the title */
	findtitle(markbuffer(win->cursor));

	/* set the cursor back to its original position, or its adjusted one */
	marksetoffset(win->cursor, cursoffset);

	/* Done! */
	return (DMINFO *)mui;
}


/* Return a dynamically-allocated string containing the name of the tag at
 * the cursor, or NULL if the cursor isn't on a tag.
 */
static CHAR *htmltagatcursor(win, cursor)
	WINDOW	win;
	MARK	cursor;
{
	CHAR	*ret;	/* the return value */
	CHAR	*p;	/* used for scanning */
	TOKEN	*token;	/* a token from the buffer */
	TOKEN	anchor;	/* copy of last <a...> or </a> token */
	long	endoffset;/* where to stop scanning */
	int	i;	/* index into mui->line */
	MARKBUF	tmp;

	/* We need to find the last <a ...> or </a> tag which occurs before
	 * before the cursor.  Since we can't read tokens backward, we must
	 * start scanning at the beginning of the line and read forward until
	 * we pass the cursor, remembering the last <a ...> or </a> as we go.
	 * If we haven't found one by the time we pass the cursor, then we
	 * need to restart scanning at the beginning of the preceding line.
	 */

	/* find the start of the cursor's line */
	mui = (MUINFO *)win->mi;
	if (mui->nlines == 0)
		return NULL;
	endoffset = markoffset(cursor) + 1;
	for (i = 1; i < mui->nlines && mui->line[i].offset < endoffset; i++)
	{
	}
	i--; /* the above loop went one line too far */

	/* read forward from the start of the line, watching for <a ...> and
	 * </a> tags.  Stop when we know we've found the last one before the
	 * cursor.
	 */
	anchor.text[0] = 0;
	do
	{
		scanalloc(&p, marktmp(tmp, markbuffer(cursor), mui->line[i].offset));
		while ((token = htmlget(&p)) != NULL && token->offset[0] < endoffset)
		{
			if (token->markup && (!CHARncmp(token->text, toCHAR("<a href="), 8) || !CHARcmp(token->text, toCHAR("</a>"))))
			{
				anchor = *token;
			}
		}
		scanfree(&p);
		endoffset = mui->line[i].offset;
	} while (anchor.text[0] == 0 && --i >= 0);

	/* If we found an <a ...> tag with an href parameter, then generate
	 * a dynamically-allocated copy of the URL.
	 */
	ret = NULL;
	if (!CHARncmp(anchor.text, toCHAR("<a href="), 8))
	{
		p = &anchor.text[8];
		if (*p == '"')
		{
			while (*++p != '"')
				buildCHAR(&ret, *p);
		}
		else
		{
			do
			{
				buildCHAR(&ret, *p);
			} while (*++p != ' ' && *p != '>');
		}
	}

	/* return the URL or a NULL pointer */
	return ret;
}


static MARK htmltagload(tagname, from)
	CHAR	*tagname;
	MARK	from;		/* where the cursor is while we're loading */
{
 static	MARKBUF	retmark;	/* the return value */
	char	*filename;	/* name of file containing tag */
	CHAR	*anchorname;	/* name of tag's anchor */
	CHAR	*p;
	TOKEN	*token;
	int	i, j;

	/* separate the tagname into filename and anchorname */
	filename = safealloc((int)CHARlen(tagname) + 1, sizeof(char));
	for (anchorname = tagname, i = 0;
	     *anchorname && *anchorname != '#';
	     anchorname++, i++)
	{
		filename[i] = *anchorname;
		if (!CHARncmp(anchorname, toCHAR("://"), 3))
		{
			msg(MSG_ERROR, "non-local URLs aren't supported yet");
			safefree(filename);
			return NULL;
		}
	}
	filename[i] = 0;

	/* Skip the '#' at the start of the anchor name, if any */
	if (*anchorname == '#')
		anchorname++;

	/* Load the file into a buffer.  If the filename isn't a full pathname
	 * then assume that the file is in the same directory as the current
	 * file.  In fact, check to see if it is the same buffer first!
	 */
	if (!filename[0]
	    || (from
		&& o_filename(markbuffer(from))
		&& !CHARcmp(o_filename(markbuffer(from)), toCHAR(filename))))
	{
		marktmp(retmark, markbuffer(from), 0);
	}
	else if (from && isalpha(*filename) && o_filename(markbuffer(from)))
	{
		marktmp(retmark,
			bufload(NULL,
				dirpath(dirdir(tochar8(o_filename(markbuffer(from)))),
					filename), False),
			0);
	}
	else
	{
		marktmp(retmark, bufload(NULL, filename, False), 0);
	}
	safefree(filename);
	if (!markbuffer(&retmark) || o_bufchars(markbuffer(&retmark)) == 0)
	{
		return NULL;
	}

	/* scan forward from top for an anchor token with this name */
	if (*anchorname)
	{
		i = (int)CHARlen(anchorname);
		scanalloc(&p, &retmark);
#if 0
		for (token = htmlget(&p);
		     token
			&& (CHARncmp(token->text, toCHAR("<a name="), 8)
				|| (token->text[8] == '"'
					? CHARncmp(&token->text[9], anchorname, (size_t)i) || token->text[9 + i] != '"'
					: CHARncmp(&token->text[8], anchorname, (size_t)i) || token->text[8 + i] != '>'));
		     token = htmlget(&p))
		{
		}
#else
		for (token = htmlget(&p); token; token = htmlget(&p))
		{
			/* ignore if not markup or not a possible target */
			if (!token->markup || token->markup->DEST == '-')
				continue;

			/* scan for "id=" or "name=" */
			for (j = 2; j < token->nchars; j++)
			{
				if (!CHARncmp(&token->text[j], toCHAR(" name="), 6))
				{
					j += 6;
					break;
				}
				if (!CHARncmp(&token->text[j], toCHAR(" id="), 4))
				{
					j += 4;
					break;
				}
			}
			if (j >= token->nchars)
				continue;

			/* compare to sought name.  Beware of quotes */
			if (token->text[j] == '"'
			    ? !CHARncmp(&token->text[++j], anchorname, (size_t)i)
				&& token->text[j + i] == '"'
			    : !CHARncmp(&token->text[j], anchorname, (size_t)i)
				&& (token->text[j + i] == '>' || token->text[j + i] == ' '))
			{
				break;
			}
		}
#endif

		/* skip to the following non-whitespace text token */
		while (token && (token->markup || isspace(token->text[0])))
		{
			token = htmlget(&p);
		}

		/* did we find the tag? */
		if (token)
		{
			marksetoffset(&retmark, token->offset[0]);
		}
		else
		{
			msg(MSG_WARNING, "[S]anchor $1 not found", anchorname);
		}
		scanfree(&p);
	}

	return &retmark;
}


static MARK htmltagnext(cursor)
	MARK	cursor;
{
	CHAR	*p;	/* used for scanning */
	TOKEN	*token;	/* a token from the text */
 static MARKBUF	ret;	/* return value */

	/* read forward to next <a ...> token with an href parameter */
	for (scanalloc(&p, cursor);
	     (token = htmlget(&p)) != NULL
		&& (!token->markup
			|| token->markup->DEST != 'T'
			|| !CHARncmp(token->text, "<a name=", 8)
			|| token->offset[0] == markoffset(cursor));
	     )
	{
	}

	/* if we found an <a ...> token, then skip ahead to the next text token
	 * or the </a> token.
	 */
	if (p && !CHARncmp(token->text, toCHAR("<a "), 3))
	{
		while ((token = htmlget(&p)) != NULL
			&& isspace(token->text[0]))
		{
		}
	}
	scanfree(&p);

	/* if we couldn't find anyplace, return NULL */
	if (!token)
	{
		return NULL;
	}

	/* else construct a mark for the start of the last token */
	return marktmp(ret, markbuffer(cursor), token->offset[0]);
}

/*----------------------------------------------------------------------------*/
/* "man" mode functions                                                       */

/* This function interprets codes in a text token, converting them to regular
 * text.  Note that none of the codes can change the font.
 */
static void manescape(token)
	TOKEN	*token;
{
	int	i, j;
	TOKEN	temp;

	/* if the cursor is in this somewhere, then don't change it but we
	 * still need to compute the width so change a temporary copy.
	 */
	if (cursoff >= token->offset[0] && cursoff <= token->offset[token->nchars - 1])
	{
		temp = *token;
		temp.offset[0] = cursoff + 1;
		manescape(&temp);
		token->width = temp.width;
		return;
	}

	/* for each character... */
	for (i = j = 0; i < token->nchars; i++)
	{
		/* most characters are copied as-is */
		if (token->text[i] != '\\')
		{
			token->text[j] = token->text[i];
			token->offset[j++] = token->offset[i];
		}
		else if (i + 1 < token->nchars)
		{
			switch (token->text[++i])
			{
			  case '|':
			  case '&':
				/* delete the \| or \& */
				break;

			  case 's':
				/* delete the \s and the number that follows */
				if (token->text[i + 1] == '+' || token->text[i + 1] == '-')
					i++;
				do
				{
					i++;
				} while (isdigit(token->text[i]));
				i--;
				break;

			  case '*':
			  case 'n':
				/* keep the \* or \n as-is */
				token->text[j] = '\\';
				token->offset[j++] = token->offset[i - 1];
				token->text[j] = token->text[i];
				token->offset[j++] = token->offset[i];
				break;

			  case 'e':
				/* convert \e to backslash */
				token->text[j] = '\\';
				token->offset[j++] = token->offset[i];
				break;

			  default:
				/* convert \X to just plain X */
				token->text[j] = token->text[i];
				token->offset[j++] = token->offset[i];
			}
		}
	}

	/* mark the end of the string */
	token ->width = token->nchars = j;
	token->text[j] = '\0';
}

/* This function is used to collect the arguments to a .XX macro together
 * as a string with a parallel array storing the font.
 */
static int manarg(token, start, font, spc)
	TOKEN	*token;	/* the token to parse */
	int	start;	/* where to begin scanning */
	_char_	font;	/* initial font of arg */
	BOOLEAN	spc;	/* insert a space before the word? */
{
	BOOLEAN	quote;	/* is this arg enclosed in quotes? */

	/* skip leading whitespace */
	while (isspace(token->text[start]))
	{
		start++;
	}

	/* is this arg quoted? */
	quote = (BOOLEAN)(token->text[start] == '"');
	if (quote)
		start++;

	/* are we supposed to insert a space? */
	if (spc && start < token->nchars - 2)
	{
		assert(start > 0);
		mantext[manlen] = ' ';
		manoffset[manlen] = token->offset[start - 1];
		manfont[manlen] = 'b';
		manlen++;
	}

	/* collect text to end of arg, or until mantext[] is full */
	while (manlen < QTY(mantext) - 1
		&& start < token->nchars - 2
		&& (quote ? token->text[start] != '"' : !isspace(token->text[start])))
	{
		/* handle \fX */
		if (token->text[start] == '\\' && token->text[start + 1] == 'f')
		{
			switch (token->text[start + 2])
			{
			  case '1':
			  case 'B':	font = 'b';	break;
			  case '2':
			  case 'I':	font = 'i';	break;
			  default:	font = 'n';	break;
			}
			start += 3;
		}
		else if (token->text[start] == '\\' && start < token->nchars - 2)
		{
			switch (token->text[++start])
			{
			  case '|':
			  case '&':
				break;

			  case 'e':
				mantext[manlen] = '\\';
				manoffset[manlen] = token->offset[start];
				manfont[manlen] = font;
				manlen++;
				break;

			  case 's':
				/* delete the \s and the number that follows */
				if (token->text[start] == '+' || token->text[start] == '-')
					start++;
				do
				{
					start++;
				} while (isdigit(token->text[start]));
				start--;
				break;

			  default:
				mantext[manlen] = token->text[start];
				manoffset[manlen] = token->offset[start];
				manfont[manlen] = font;
				manlen++;
			}
			start++;
		}
		else if (token->text[start] <= ' ')
		{
			mantext[manlen] = ' ';
			manoffset[manlen] = token->offset[start];
			manfont[manlen] = font;
			manlen++;
			start++;
		}
		else
		{
			mantext[manlen] = token->text[start];
			manoffset[manlen] = token->offset[start];
			manfont[manlen] = font;
			manlen++;
			start++;
		}
	}

	/* skip the closing quote (if quoted) */
	if (quote && token->text[start] == '"')
		start++;

	/* return the index of the end of the arg */
	return start;
}


/* This function either outputs mantext[] and returns False, or (if mantext[]
 * is too wide to fit on this line) it outputs a newline and returns True.
 */
static BOOLEAN manput()
{
	int	i, start;

	/* if it won't fit, then output a newline instead */
	if (!first && col + manlen > textwidth - listind)
	{
		/* output a newline */
		(*drawchar)(&newline, 1, 'n', anyspc ? spcoffset : -1);
		col = 0;
		first = True;
		return True;
	}

	/* It will fit.  If we need to adjust our indent, do it now */
	if (col < indent)
	{
		(*drawchar)(&space, col - indent, 'n', anyspc ? spcoffset : -1);
		col = indent;
		anyspc = False;
	}

	/* Output a space between tokens, usually. */
	if (anyspc)
	{
		(*drawchar)(&space, 1, 'n', spcoffset);
		col++;
		anyspc = False;
	}

	/* Output mantext[] in as few chunks as possible */
	for (start = 0; start < manlen; start = i)
	{
		for (i = start + 1;
		     i < manlen
			&& manfont[i] == manfont[start]
			&& manoffset[i] == manoffset[i - 1] + 1;
		     i++)
		{
		}
		(*drawchar)(&mantext[start], (long)(i - start), manfont[start], manoffset[start]);
	}
	col += manlen;

	return False;
}

/* This function implements the .TH macro, which declares the page's title */
static BOOLEAN manTH(token)
	TOKEN	*token;
{
	int	i;

	/* combine the first & second args as the document name */
	manlen = 0;
	i = manarg(token, 3, 'n', False);
	mantext[manlen++] = '(';
	(void)manarg(token, i, 'n', False);
	mantext[manlen++] = ')';
	mantext[manlen] = '\0';

	/* If the title is different, then store it now */
	if (!mui->title || CHARcmp(mui->title, mantext))
	{
		if (mui->title)
			safefree(mui->title);
		mui->title = CHARdup(mantext);
	}

	return False;
}

/* This function implements the .SH and .SS macros */
static BOOLEAN manSH(token)
	TOKEN	*token;
{
	long	nloff;	/* offset of newline */
	int	i;

	/* Get the arguments, as the section title */
	manlen = 0;
	for (i = 4; i < token->nchars; i++)
	{
		i = manarg(token, i, 'b', (BOOLEAN)(i != 4));
	}

	/* If the cursor is located in this token, or we're doing some sort
	 * of move operation, then pretend that the offsets of all the
	 * generated characters are -1.
	 */
	nloff = token->offset[token->nchars - 1];
	if ((o_showmarkups && token->offset[0] <= cursoff && cursoff <= token->offset[token->nchars - 1])
		|| list || drawchar == countchar)
	{
		for (i = 0; i < manlen; i++)
			manoffset[i] = -1;
		nloff = -1;
	}

	/* output the title, followed by a newline */
	anyspc = False;
	manput();
	(*drawchar)(&newline, 1, 'n', nloff);
	col = 0;
	first = True;
	anyspc = False;
	reduce = True;

	/* force the indentation to be the default */
	indent = listind * 2;

	/* Return False even though we did output a newline.  The newline
	 * (and the text before it) is considered to be part of the same
	 * "line" as far as movement is concerned.
	 */
	return False;
}

/* This function implements the font-changing macros such as .BI, .RB, and
 * so on.  The resulting text is treated as a single word; if it can't fit
 * on the current line, then this function merely outputs a newline.
 */
static BOOLEAN manBI(token)
	TOKEN	*token;
{
	char	font1, font2;
	int	start;

	/* choose the fonts & start, based on macro name */
	switch (token->text[1])
	{
	  case 'B':	font1 = 'b';	break;
	  case 'I':	font1 = 'i';	break;
	  default:	font1 = 'n';
	}
	switch (token->text[2])
	{
	  case 'B':	font2 = 'b';	start = 4;	break;
	  case 'I':	font2 = 'i';	start = 4;	break;
	  case 'S':
	  case 'R':	font2 = 'n';	start = 4;	break;
	  default:	font2 = font1;	start = 3;
	}

	/* collect the args, with their fonts */
	manlen = 0;
	start = manarg(token, start, font1, False);
	start = manarg(token, start, font2, (BOOLEAN)(font1 == font2));
	start = manarg(token, start, font1, (BOOLEAN)(font1 == font2));
	start = manarg(token, start, font2, (BOOLEAN)(font1 == font2));
	start = manarg(token, start, font1, (BOOLEAN)(font1 == font2));
	start = manarg(token, start, font2, (BOOLEAN)(font1 == font2));

	/* If the cursor is on this token, then just tweak its width and
	 * return False so the remaining token-putting code will handle
	 * line-wrap.  Else perform the token's output.
	 */
	if (list || (o_showmarkups && token->offset[0] <= cursoff && cursoff <= token->offset[token->nchars - 1]))
	{
		token->width = manlen + 1; /* "+ 1" to allow for whitespace */
		return False;
	}
	else if (manput())
	{
		return True;
	}

	/* assume there should be some whitespace after this */
	anyspc = True;
	spcoffset = token->offset[token->nchars - 1];
	return False;
}

/* This function implements the .IP macro.  It outputs its first argument,
 * and then increases the indentation.  If necessary, it will then output
 * a newline character, but this newline character isn't considered to be
 * the end of "line" as far as moving the cursor is concerned; it is just
 * a funny type of wrapping.
 */
static BOOLEAN manIP(token)
	TOKEN	*token;
{
	int	i, start;
	char	font, font2;

	/* if this token is going to be displayed anyway, then don't bother
	 * performing its output.
	 */
	if (list || (o_showmarkups && token->offset[0] <= cursoff && cursoff <= token->offset[token->nchars - 1]))
		return False;

	/* get the first arg, as the paragraph tag */
	manlen = 0;
	start = manarg(token, 4, 'n', False);

	/* For .TP (but not .IP) get other args as part of the tag, too */
	if (token->text[1] == 'T')
	{
		/* if first word was .B or .I, then change font */
		if (manlen == 2 && mantext[0] == '.' && (mantext[1] == 'B' || mantext[1] == 'I'))
		{
			font = font2 = tolower(mantext[1]);
			manlen = 0;
		}
		else if (manlen == 3 && mantext[0] == '.')
		{
			font = tolower(mantext[1]);
			if (font != 'b' && font != 'i')
				font = 'n';
			font2 = tolower(mantext[2]);
			if (font2 != 'b' && font2 != 'i')
				font2 = 'n';
			manlen = 0;
		}
		else
		{
			font = font2 = 'n';
		}

		/* Copy the remaining args, in the desired font[s].  If the
		 * fonts are different, then delete whitespace from between
		 * arguments.
		 */
		for (i = 0; i < 5; i++)
		{
			start = manarg(token, start, (i & 1) ? font2 : font, (BOOLEAN)(font == font2 && manlen > 0));
		}
	}

	/* output the paragraph tag */
	(void)manput();

	/* increase the indentation by one full tab for .IP, two for .TP */
	if (token->text[1] == 'T')
		indent += 4 * listind;
	else
		indent += 2 * listind;

	/* If necessary, output a newline as part of the text */
	if (col >= indent)
	{
		(*drawchar)(&newline, 1, 'n', token->offset[token->nchars - 1]);
		col = 0;
	}

	/* return False, even if we output a newline */
	return False;
}

/* Look up a man markup token in a table */
static void manmarkup(token)
	TOKEN	*token;	/* the token to lookup */
{
	static MARKUP	tbl[] =
	{

		/* Tag		 Effects	Function	*/
		/*               TBILFFD                        */
		{ "\\\"",	"Y------"			},
		{ "TH",		"Y----Y-",	manTH		},
		{ "SH",		"Nc0-NYS",	manSH		},
		{ "SS",		"N11-NYS",	manSH		},
		{ "B",		"N------",	manBI		},
		{ "I",		"N------",	manBI		},
		{ "SM",		"N------",	manBI		},
		{ "BI",		"N------",	manBI		},
		{ "IB",		"N------",	manBI		},
		{ "RI",		"N------",	manBI		},
		{ "IR",		"N------",	manBI		},
		{ "BR",		"N------",	manBI		},
		{ "RB",		"N------",	manBI		},
		{ "BS",		"N------",	manBI		},
		{ "SB",		"N------",	manBI		},
		{ "IP",		"N12-NYP",	manIP		},
		{ "TP",		"N12-NYP",	manIP		},
		{ "PP",		"N12-NYP"			},
		{ "P",		"N12-NYP"			},
		{ "LP",		"N12-NYP"			},
		{ "RS",		"N->----"			},
		{ "RE",		"N-<----"			},
		{ "br",		"N0-----"			},
		{ "sp",		"N1-----"			},
		{ "nf",		"N0---N-"			},
		{ "fi",		"N0---Y-"			},
		{ "DS",		"N1---N-"			},
		{ "DE",		"N0---Y-"			},
		{ (char *)0,	"N------"			}
	};
	MARKUP	*scan;	/* used for scanning the tbl[] array */

	/* look it up in the table */
	for (scan = tbl;
	     scan->name &&
		(strncmp(scan->name, (char *)token->text + 1, strlen(scan->name))
		|| isalnum(token->text[strlen(scan->name) + 1]));
	     scan++)
	{
	}

	/* remember it */
	token->markup = scan;
}


/* Get the next token from a "man" document, or return NULL at end. */
static TOKEN *manget(refp)
	CHAR	**refp;
{
	MARK	back;	/* address of a backslash */
	long	offset;	/* offset of character that *refp points to */
 static	MARKUP	fontchg;/* describes font change markups */

	/* Initialize "back" just to silence a compiler warning */
	back = NULL;

	/* if the CHAR pointer is NULL, then return NULL */
	if (!*refp)
		return NULL;

	/* Get first character of token */
	offset = markoffset(scanmark(refp));
	rettok.text[0] = **refp;
	rettok.offset[0] = offset++;
	rettok.nchars = 1;
	rettok.width = 0;
	rettok.markup = NULL;
	scannext(refp);

	/* If '.' at the start of a line, then token is a markup */
	if ((rettok.text[0] == '.' || rettok.text[0] == '\'') && !midline)
	{
		/* This is a markup.  Collect characters up to next '\n' */
		for (;
		     *refp && rettok.nchars < QTY(rettok.text) - 3 && **refp != '\n';
		     offset++, scannext(refp))
		{
			rettok.text[rettok.nchars] = **refp;
			rettok.offset[rettok.nchars++] = offset;
		}

		/* If this is ".TP" then read the following line as part of
		 * this token.
		 */
		if (!CHARncmp(rettok.text, toCHAR(".TP"), 3)
		 && *refp
		 && **refp == '\n'
		 && scannext(refp))
		{
			/* the newline after ".TP" becomes a space */
			rettok.nchars = 3;
			rettok.text[rettok.nchars] = ' ';
			rettok.offset[rettok.nchars] = offset;
			rettok.nchars++;
			offset++;

			for (;
			     *refp && rettok.nchars < QTY(rettok.text) - 3 && **refp != '\n';
			     offset++, scannext(refp))
			{
				rettok.text[rettok.nchars] = **refp;
				rettok.offset[rettok.nchars++] = offset;
			}
		}

		/* store the terminating '\n' as "^J" */
		rettok.text[rettok.nchars] = '^';
		rettok.offset[rettok.nchars] = offset;
		rettok.nchars++;
		rettok.text[rettok.nchars] = 'J';
		rettok.offset[rettok.nchars] = offset;
		rettok.nchars++;
		rettok.text[rettok.nchars] = '\0';
		if (*refp)
			scannext(refp);

		/* lookup the markup */
		manmarkup(&rettok);

		/* remember that we stopped after a newline */
		midline = False;
	}
	else if (*refp && rettok.text[0] == '\\' && **refp == 'f')
	{
		/* This is a font-change macro.  Get the final character */
		rettok.text[1] = 'f';
		rettok.offset[1] = offset++;
		scannext(refp);
		rettok.text[2] = (*refp ? **refp : 'R');
		rettok.offset[2] = offset++;
		rettok.nchars = 3;
		if (*refp)
			scannext(refp);

		/* construct a simple MARKUP struct */
		fontchg.name = tochar8(rettok.text);
		strcpy(fontchg.attr, "----n--");
		switch (rettok.text[2])
		{
		  case '1':
		  case 'B':	fontchg.FONT = 'b';	break;
		  case '2':
		  case 'I':	fontchg.FONT = 'i';	break;
		  case 'P':	fontchg.FONT = '=';	break;
		  default:	fontchg.FONT = 'n';	break;
		}
		rettok.markup = &fontchg;

		/* no newline at the end of this token! */
		midline = True;
	}
	else if (rettok.text[0] <= ' ')
	{
		/* This is a whitespace token.  Each whitespace token contains
		 * a SINGLE whitespace character.  Control characters other
		 * than '\t' and '\n' are treated as spaces.
		 */
		if (rettok.text[0] != '\t' && rettok.text[0] != '\n')
			rettok.text[0] = ' ';

		/* remember if this is a newline or not */
		midline = (BOOLEAN)(rettok.text[0] != '\n');

		/* assume this whitespace will show, for computing line break */
		rettok.width = 1;
	}
	else
	{
		/* This is a word.  Collect chars up to next whitespace or
		 * "\fX" string.
		 */
		for (;
		     *refp 
			&& rettok.nchars < QTY(rettok.text) - 1
			&& !isspace(**refp)
			&& (rettok.nchars < 2
				|| rettok.text[rettok.nchars - 2] != '\\'
				|| rettok.text[rettok.nchars - 1] != 'f');
		     offset++, scannext(refp))
		{
			if (**refp == '\\')
				back = scanmark(refp);
			rettok.text[rettok.nchars] = **refp;
			rettok.offset[rettok.nchars] = offset;
			rettok.nchars++;
		}

		/* if this ended with a \fX then we need to adjust *refp */
		if (rettok.nchars >= 2
			&& rettok.text[rettok.nchars - 2] == '\\'
			&& rettok.text[rettok.nchars - 1] == 'f')
		{
			scanseek(refp, back);
			rettok.nchars -= 2;
		}

		/* this didn't end with a newline */
		midline = True;

		/* For now, assume all characters of this work will be visible.
		 * When escapes are processed, this may change.
		 */
		rettok.width = rettok.nchars;
	}

	/* Mark the end of the text, and return the token */
	rettok.text[rettok.nchars] = '\0';
	return &rettok;
}

static DMINFO *maninit(win)
	WINDOW	win;
{
	TOKEN	*token;
	MARKBUF	top;
	CHAR	*p;
	long	cursoffset;

	/* inherit some functions from dmnormal */
	dmman.wordmove = dmnormal.wordmove;
	dmman.tagatcursor = dmnormal.tagatcursor;
	dmman.tagload = dmnormal.tagload;
	dmman.tagnext = dmnormal.tagnext;

	/* allocate the info struct */
	mui = (MUINFO *)safealloc(1, sizeof(MUINFO));
	mui->get = manget;
	mui->escape = manescape;
	mui->line = safealloc(GRANULARITY, sizeof(LINEINFO));
	mui->nlines = 1; /* every buffer has at least one line */

	/* move the cursor someplace harmless, so it won't affect formatting */
	cursoffset = markoffset(win->cursor);
	marksetoffset(win->cursor, o_bufchars(markbuffer(win->cursor)));

	/* format the buffer */
	win->mi = (DMINFO *)mui;
	(void)start(win, marktmp(top, markbuffer(win->cursor), 0), NULL);
	p = scanalloc(&p, &top);
	for (token = (*mui->get)(&p);
	     token;
	     token = (*mui->get)(&p))
	{
		/* if cursor is at position 0, and this is a text token,
		 * then move the cursor here.  This is done because otherwise
		 * the cursor would always start on an ugly formatting code.
		 */
		if (cursoffset == 0L 
		 && !title
		 && !isspace(token->text[0])
		 && !token->markup)
		{
			cursoffset = token->offset[0];
		}

		/* output the token.  If it forces a new line, remember where
		 * that new line started.
		 */
		if (put(token))
		{
			assert(first == True && col == 0);
			storestate(token->offset[0], NULL);
			(void)put(token);
		}
	}
	scanfree(&p);

	/* restore the cursor position */
	marksetoffset(win->cursor, cursoffset);

	/* Done! */
	return (DMINFO *)mui;
}

/*----------------------------------------------------------------------------*/
/* Generic markup functions                                                   */

/* This function performs a single token.  If the token is markup, then the
 * side effects are performed and the function (if any) is called.  If text,
 * or if the cursor is in the token, then the text of the token is output.
 * It is assumed that drawchar and cursoff are set before this function is
 * called.  Returns False if the token fits on this line.  If it didn't fit,
 * or was a markup that caused a line break, then it returns True in which
 * case put() should be called again for the same token to start generating
 * the next line.
 */
static BOOLEAN put(token)
	TOKEN	*token;
{
	char	tmpfont;
	CHAR	tmpch, lch, rch;
	int	i, origcol;
	BOOLEAN hascursor;	/* indicates whether this token contains cursor */

	/* determine whether the cursor is in this token.  If the "list"
	 * option is set, then pretend that all tokens contain the cursor.
	 */
	hascursor = (BOOLEAN)(list || (o_showmarkups && token->offset[0] <= cursoff && cursoff <= token->offset[token->nchars - 1]));

	/* is it a markup? */
	if (token->markup)
	{
		/* if this token will cause a line break, and no line break
		 * was already pending anyway, then do the line break and
		 * nothing else.  We'll do the rest of this markup's job
		 * at the start of the next line.
		 */
		if (token->markup->BREAKLN != '-' && !first)
		{
			(*drawchar)(&newline, 1, 'n', anyspc ? spcoffset : -1);
			reduce = (BOOLEAN)(col == 0);
			col = 0;
			first = True;
			return True;
		}

		/* do all the standard effects */
		switch (token->markup->TITLE)
		{
		  case 'Y': title = True;	break;
		  case 'N': title = False;	break;
		}
		switch (token->markup->BREAKLN)
		{
		  case '0':
			reduce = True;
			break;

		  case '1':
			if (!reduce)
				(*drawchar)(&newline, 1, 'n', anyspc ? spcoffset : -1);
			reduce = True;
			break;

		  case '2':
			(*drawchar)(&newline, reduce ? 1 : 2, 'n', anyspc ? spcoffset : -1);
			reduce = True;
			break;

		  case 'c':
			(*drawchar)(&vtab, 1, 'n', -1);
			reduce = True;
			break;

		  case 'p':
			(*drawchar)(&formfeed, 1, 'n', -1);
			reduce = True;
			break;
		}
		switch (token->markup->INDENT)
		{
		  case '<':
			indent = (indent < listind ? 0 : indent - listind);
			break;

		  case '>':
			indent += listind;
			break;

		  case '=':
			if (col < tabstop)
				indent = tabstop;
			else
				indent = col + 2 * tabstop - ((col + tabstop) % (2 * tabstop));
			break;

		  case '0':
		  case '1':
		  case '2':
		  case '3':
		  case '4':
			indent = (token->markup->INDENT - '0') * listind;
			break;
		}
		switch (token->markup->LIST)
		{
		  case 'N':
			if (nest > 0)
				nest--;
			break;

		  case 'Y':
			nest++;
			if (nest == 1)
				listcnt = 0;
			break;

		  case '#':
			nest++;
			if (nest == 1)
				listcnt = 1;
			break;
		}
		switch (token->markup->FONT)
		{
		  case '=': curfont = deffont;				break;
		  case 'n':
		  case 'b':
		  case 'u':
		  case 'f':
		  case 'e':
		  case 'i': curfont = token->markup->FONT;		break;
		  case 'N':
		  case 'B':
		  case 'U':
		  case 'F':
		  case 'E':
		  case 'I': curfont = deffont = tolower(token->markup->FONT); break;
		}
		switch (token->markup->FILL)
		{
		  case 'Y': graphic = prefmt = False;	break;
		  case 'N': prefmt = True;		break;
		}

		/* If there is a function, call it too.  If the function
		 * returns True, then act as though the markup caused a
		 * newline.
		 */
		if (token->markup->fn)
		{
			if ((*token->markup->fn)(token))
				return True;
		}

		/* if the cursor isn't on this token, don't show it */
		if (!hascursor)
		{
			first = False;
			return False;
		}
	}
	else
	{
		reduce = False;
	}

	/* no token causes visible output when in "title" mode, unless the
	 * cursor happens to be located on it.
	 */
	if (title && !hascursor)
	{
		first = False;
		return False;
	}

	/* Newlines are handled differently that other whitespace, when in
	 * prefmt mode.  This is because newlines mark the end of one line,
	 * so they become the first token of the next line... but when we
	 * get to the next line we don't want to process the newline again,
	 * or we'd get into an endless loop.  Here we catch the case were
	 * we are calling put() the second time.
	 */
	if (prefmt && token->text[0] == '\n' && first)
	{
		first = False;
		anyspc = True;
		spcoffset = token->offset[0];
		return False;
	}

	/* Expand any escape sequences in the token's text */
	if (mui->escape && !token->markup)
		(*mui->escape)(token);

	/* Is it whitespace?  Are we supposed to adjust the text formatting? */
	if (isspace(token->text[0]) && !prefmt)
	{
		/* Just set a flag indicating that a space has been
		 * encountered.  Also remember its offset if it is the first
		 * space encountered, or if the cursor is on this space.
		 */
		if (!anyspc || hascursor)
			spcoffset = token->offset[0];
		anyspc = True;
		first = False;
		return False;
	}

	/* if not a markup, and it won't fit on the current line, then output
	 * a '\n' and return True so it'll appear on the next line.
	 */
	else if (col + token->width + 1 > textwidth - 4
		&& !prefmt && col != 0 && !token->markup)
	{
		(*drawchar)(&newline, 1, 'n', anyspc ? spcoffset : -1);
		col = 0;
		anyspc = False;
		first = True;
		return True;
	}

	/* if we need to adjust our indent, do it now */
	if (col < indent)
	{
		(*drawchar)(&space, col - indent, 'n', anyspc ? spcoffset : -1);
		col = indent;
		anyspc = False;
	}

	/* Output a space between tokens, usually. */
	if (anyspc)
	{
		(*drawchar)(&space, 1, 'n', spcoffset);
		col++;
		anyspc = False;
	}

	/* remember our current column, so we can pretend visible markups are
	 * zero characters wide later.
	 */
	origcol = col;

	/* Output the token's text.  Watch for whitespace and maybe graphics. */
	for (i = 0; i < token->nchars; i++)
	{
		switch (token->text[i])
		{
		  case '\t':
			/* convert to spaces */
			col = tabstop - (col - indent) % tabstop;
			(*drawchar)(&space, -col, curfont, token->offset[0]);
			col += origcol;
			break;

		  case '\n':
			/* newlines are tricky.  If we get here, then we must
			 * be in no-fill mode, so the newline should be output.
			 * However, we'll need to be on the lookout for this
			 * same newline next time.
			 */
			(*drawchar)(&newline, 1, 'n', -1);
			col = 0;
			anyspc = True;
			first = True;
			return True;

		  case '|':
		  case '.':
		  case '^':
		  case '-':
			tmpfont = curfont;
			tmpch = token->text[i];
			if (graphic)
			{
				lch = (i > 0 ? token->text[i - 1] : ' ');
				rch = (i < token->nchars - 1 ? token->text[i + 1] : ' ');
				if (lch == '-' && rch == '-')
				{
					if (tmpch == '|')	tmpch = '5';
					else if (tmpch == '.')	tmpch = '8';
					else if (tmpch == '^')	tmpch = '2';
					tmpfont = 'g';
				}
				else if (lch == '-')
				{
					if (tmpch == '|')	tmpch = '6';
					else if (tmpch == '.')	tmpch = '9';
					else if (tmpch == '^')	tmpch = '3';
					tmpfont = 'g';
				}
				else if (rch == '-')
				{
					if (tmpch == '|')	tmpch = '4';
					else if (tmpch == '.')	tmpch = '7';
					else if (tmpch == '^')	tmpch = '1';
					tmpfont = 'g';
				}
				else if (tmpch == '|')
				{
					tmpfont = 'g';
				}
			}
			(*drawchar)(&tmpch, 1, tmpfont, token->offset[i]);
			col++;
			break;

		  default:
			(*drawchar)(&token->text[i], 1, token->markup ? 'e' : curfont, token->offset[i]);
			col++;
		}
	}

	/* we won't need any more implied spaces between tokens until we hit
	 * the next whitespace token in fill mode.
	 */
	anyspc = False;

	/* markup tokens aren't supposed to affect the logical column number */
#if 0
	if (token->markup)
	{
		col = origcol;
	}
#else
	col = origcol + token->width;
#endif

	/* Done! */
	first = False;
	return False;
}



/* This is a version of drawchar() which does nothing except count the column
 * position.  It is used by mark2col() to figure out which column the cursor
 * is on, and move() to find the offset for a given column.
 */
static long physcol;	/* physical column counter - initialize to 0 */
static long wantoffset;	/* desired offset, whose column we'll be tracking */
static long outcol;	/* physical column of wantoffset - initialize to -1 */
static long wantcol;	/* desired column, whose offsets we'll be tracking */
static long outoffset;	/* offset of char at a given column - init to -1 */
static long prevoffset;	/* previous offset which was >= 0 */

static void countchar(p, qty, font, offset)
	CHAR	*p;	/* the character being output */
	long	qty;	/* quantity of characters */
	_char_	font;	/* the font to show it in (ignored) */
	long	offset;	/* which buffer char this corresponds to */
{
	long	delta;
	register CHAR ch;

	/* a negative qty indicates that the same character repeats */
	if (qty < 0)
	{
		delta = 0;
		qty = -qty;
	}
	else
	{
		delta = 1;
	}

	/* for each character... */
	for ( ; qty > 0; qty--, p += delta, offset += delta)
	{
		/* copy *p into a register variable */
		ch = *p;

		/* if this is the desired offset, remember its column */
		if (offset >= wantoffset && outcol < 0)
			outcol = physcol;

		/* if this offset >= 0, then remember it */
		if (offset >= 0)
			prevoffset = offset;

		/* process the character */
		if (ch == '\n' || ch == '\f' || ch == '\013')
		{
			if (outoffset == -1)
			{
				outoffset = prevoffset;
			}
			physcol = 0;
		}
		else
		{
			if (physcol >= wantcol && outoffset < 0)
			{
				outoffset = prevoffset;
			}
			physcol++;
		}
	}
}


/*----------------------------------------------------------------------------*/
/* Elvis display mode functions                                               */

/* end the mode, and free the modeinfo */
static void term(info)
	DMINFO	*info;	/* window-specific info about mode */
{
	MUINFO	*mui = (MUINFO *)info;

	if (mui->title)
		safefree(mui->title);
	if (mui->line)
		safefree(mui->line);
	safefree(info);
}

/* Convert a mark to a screen-relative column number */
static long mark2col(w, mark, cmd)
	WINDOW	w;	/* window where buffer is shown */
	MARK	mark;	/* mark to convert */
	BOOLEAN	cmd;	/* if True, we're in command mode; else input mode */
{
	CHAR	*p;	/* used for scanning */
	TOKEN	*token;
	MARKBUF	tmp;
	int	i;

	/* Start scanning at the start of the line which contains this mark.
	 * Count characters until we find the mark's offset, or until the
	 * end of the line is reached.
	 */
	i = start(w, mark, NULL);
	p = scanalloc(&p, marktmp(tmp, markbuffer(mark), mui->line[i].offset));
	while ((token = (*mui->get)(&p)) != NULL && !put(token) && outcol < 0)
	{
	}
	scanfree(&p);
	return (outcol>=0 ? outcol : physcol);
}


/* Move vertically, and to a given column (or as close to column as possible) */
static MARK move(w, from, linedelta, column, cmd)
	WINDOW	w;		/* window where buffer is shown */
	MARK	from;		/* old location */
	long	linedelta;	/* line movement */
	long	column;		/* desired column number */
	BOOLEAN	cmd;		/* if True, we're in command mode; else input mode */
{
	int	i, j;
	CHAR	*p;
	TOKEN	*token;
 static MARKBUF	tmp;

	/* find the start of the line containing the mark */
	i = start(w, from, NULL);

	/* Add linedelta to the index, being careful not to move outside
	 * the array.
	 */
	i += linedelta;
	if (i >= mui->nlines)
		i = mui->nlines - 1;
	if (i < 0)
		i = 0;

	/* Move to the given column, or as close as possible */
	if (column == 0)
	{
		outoffset = mui->line[i].offset;
	}
	else if (column >= textwidth)
	{
		if (i < mui->nlines - 1)
			outoffset = mui->line[i + 1].offset - 1;
		else
			outoffset = o_bufchars(markbuffer(from)) - 1;
	}
	else
	{
		j = start(w, marktmp(tmp, markbuffer(from), mui->line[i].offset), NULL);
		assert(j == i);
		wantcol = column;
		scanalloc(&p, marktmp(tmp, markbuffer(from), mui->line[j].offset));
		while ((token = (*mui->get)(&p)) != NULL && !put(token) && outoffset < 0)
		{
		}
		scanfree(&p);
	}

	/* return the found mark */
	if (outoffset < mui->line[i].offset)
		outoffset = mui->line[i].offset;
	return marktmp(tmp, markbuffer(from), outoffset);
}

/* Choose a line to appear at the top of the screen, and return its mark.
 * Also, initialize the info for the next line.
 */
static MARK setup(top, cursor, bottom, info)
	MARK	top;	/* where previous image started */
	long	cursor;	/* offset of cursor */
	MARK	bottom;	/* where previous image ended */
	DMINFO	*info;	/* window-specific info about mode */
{
	WINDOW	win;	/* window we're probably doing this for */
	int	topi, bottomi; /* line indicies of the top & bottom */
 static MARKBUF	tmp;
	int	i;

	/* I kind of wish I has passed "win" as an argument.  Oh, well,
	 * its easy enough to find the window.
	 */
	win = NULL;
	do
	{
		win = winofbuf(win, markbuffer(top));
		assert(win);
	} while (win->mi != info);

	/* we can optimize if "nolist noshowmarkup" */
	dmhtml.canopt = dmman.canopt = (BOOLEAN)(!o_list(win) && !o_showmarkups);
	
	/* find the line indicies of the top & bottom marks, and the cursor */
	/* NOTE: This could have been implemented more efficiently! */
	topi = start(win, top, NULL);
	bottomi = start(win, bottom, NULL);
	i = start(win, marktmp(tmp, markbuffer(top), cursor), NULL);

	/* if the cursor is between top & bottom, great!  Else we need to
	 * either scroll or jump.
	 */
	if (topi <= i && i < bottomi)
	{
		/* great! */
	}
	else if (topi - o_nearscroll <= i && i < topi)
	{
		topi = i;
	}
	else if (bottomi <= i && i < bottomi + o_nearscroll)
	{
		topi = i - o_lines(win) - 1;
		/* NOTE: if sidescrolling is disabled and the window is narrow,
		 * then the above computation may set topi back farther than
		 * necessary.  This is inefficient, but doesn't result in any
		 * visible problems because the Draw module will compensate by
		 * slop scrolling.
		 */
	}
	else
	{
		topi = i - o_lines(win) / 2;
		win->di->logic = DRAW_CENTER;
	}

	/* topi should never be negative */
	if (topi < 0)
		topi = 0;

	/* return the top of the starting line */
	return marktmp(tmp, markbuffer(top), mui->line[topi].offset);
}

static MARK image(w, line, info, draw)
	WINDOW	w;		/* window where drawing will go */
	MARK	line;		/* start of line to draw */
	DMINFO	*info;		/* window-specific info about mode */
	void	(*draw)P_((CHAR *p, long qty, _char_ font, long offset));
				/* function for drawing a single character */
{
	CHAR	*p;
	TOKEN	*token;
 static MARKBUF	tmp;

	/* generate the line image, using the given "draw" function */
	(void)start(w, line, draw);
	scanalloc(&p, line);
	for (token = (*mui->get)(&p); token && !put(token); token = (*mui->get)(&p))
	{
	}
	scanfree(&p);

	/* if we hit the end of the buffer, and didn't output a newline,
	 * then output a newline now.
	 */
	if (!token)
	{
		(*draw)(&newline, 1, 'n', anyspc ? spcoffset : -1);
	}

	/* return the offset of the first token of the next line */
	if (token)
		return marktmp(tmp, markbuffer(line), token->offset[0]);
	else
		return marktmp(tmp, markbuffer(line), o_bufchars(markbuffer(line)));
}


static void header(w, pagenum, info, draw)
	WINDOW	w;	/* window from which we're printing */
	int	pagenum;/* page number */
	DMINFO	*info;	/* drawing state */
	void	(*draw)P_((CHAR *p, long qty, _char_ font, long offset));
{
	CHAR	pg[20];	/* page number, as a text string */
	CHAR	*title;	/* title of the document */
	CHAR	*sides;	/* text to show on left & right sides of page */
	CHAR	*middle;/* text to show in middle of page */
	int	slen;	/* length of side string */
	int	mlen;	/* length of middle string */
	long	gap1;	/* width of gap between left side & middle */
	long	gap2;	/* width of gap between middle & right side */

	assert(info == (DMINFO *)mui);

	/* covert page number to text */
	long2CHAR(pg, (long)pagenum);

	/* find the title of the document */
	title = mui->title ? mui->title
			   : o_bufname(markbuffer(w->cursor));

	/* Find the widths of things */

	/* figure out what goes where */
	if (mui->escape == manescape)
	{
		sides = title;
		middle = pg;
	}
	else
	{
		sides = pg;
		middle = title;
	}

	/* find the lengths of things */
	slen = CHARlen(sides);
	mlen = CHARlen(middle);
	if (slen + 2 + mlen + 2 + slen > textwidth)
	{
		mlen = textwidth - 4 - 2 * slen;
		if (mlen <= 0)
		{
			mlen = CHARlen(middle);
			slen = (textwidth - 4 - mlen) / 2;
		}
	}
	gap1 = (textwidth - mlen) / 2 - slen;
	gap2 = textwidth - mlen - 2 * slen - gap1;

	/* Output the parts of the headings */
	(*draw)(sides, slen, 'n', 0);
	(*draw)(&space, -gap1, 'n', -1);
	(*draw)(middle, mlen, 'n', 0);
	(*draw)(&space, -gap2, 'n', -1);
	(*draw)(sides, slen, 'n', 0);

	/* End the header line, and then skip one or two more lines */
	(*draw)(&newline, pagenum==1 ? -2L : -3L, 'n', -1);
	reduce = True;
}


/*----------------------------------------------------------------------------*/
/* Maintenance functions                                                      */

/* store the offset and parse state in "dest"... or append it to mui->line
 * if "dest" is a NULL pointer.
 */
static void storestate(offset, dest)
	long	offset;
	LINEINFO *dest;
{
	/* if "dest" is NULL, then we'll be appending */
	if (!dest)
	{
		/* we may need to reallocate mui->line */
		if (mui->nlines % GRANULARITY == 0)
		{
			dest = mui->line;
			mui->line = safealloc((int)(mui->nlines + GRANULARITY), sizeof(LINEINFO));
			if (dest)
			{
				memcpy(mui->line, dest, (size_t)mui->nlines * sizeof(LINEINFO));
				safefree(dest);
			}
		}

		/* locate the end of mui->line[], and increment mui->nlines */
		dest = &mui->line[mui->nlines++];
	}

	/* store the info */
	dest->offset = offset;
	dest->state.prefmt = prefmt ? 1 : 0;
	dest->state.graphic = graphic ? 1 : 0;
	dest->state.midline = midline ? 1 : 0;
	dest->state.reduce = reduce ? 1 : 0;
	switch (deffont)
	{
	  case 'b':	dest->state.deffont = 1;	break;
	  case 'u':	dest->state.deffont = 2;	break;
	  case 'i':	dest->state.deffont = 3;	break;
	  case 'f':	dest->state.deffont = 4;	break;
	  case 'e':	dest->state.deffont = 5;	break;
	  default:	dest->state.deffont = 0;
	}
	switch (curfont)
	{
	  case 'b':	dest->state.curfont = 1;	break;
	  case 'u':	dest->state.curfont = 2;	break;
	  case 'i':	dest->state.curfont = 3;	break;
	  case 'f':	dest->state.curfont = 4;	break;
	  case 'e':	dest->state.curfont = 5;	break;
	  default:	dest->state.curfont = 0;
	}
	dest->state.indent = indent;
	dest->state.nest = nest;
	dest->state.listcnt = listcnt;
}


/* This function copies sets "mui" and the other parsing variables from the
 * line information of a given window, for a given starting point.  This
 * function also sets "drawchar" function pointer to a given value; if no
 * "drawchar" function is given, then it uses the internal dummy function
 * and sets up that function's initial state.
 *
 * Returns the index into mui->line[] of the starting line
 */
static int start(win, from, draw)
	WINDOW	win;	/* window to draw for */
	MARK	from;	/* starting point */
	void	(*draw) P_((CHAR *p, long qty, _char_ font, long offset));
{
	int	i;

	/* find the start of the line which contains "from" */
	mui = (MUINFO *)win->mi;
	for (i = 0; i < mui->nlines && mui->line[i].offset <= markoffset(from); i++)
	{
	}
	if (i > 0) i--; /* the above loop took us one line too far */

	/* copy that line's parsing state into parsing variables */
	prefmt = (BOOLEAN)mui->line[i].state.prefmt;
	graphic = (BOOLEAN)mui->line[i].state.graphic;
	midline = (BOOLEAN)mui->line[i].state.midline;
	reduce = (BOOLEAN)mui->line[i].state.reduce;
	deffont = "nbuife"[mui->line[i].state.deffont];
	curfont = "nbuife"[mui->line[i].state.curfont];
	indent = mui->line[i].state.indent;
	nest = mui->line[i].state.nest;
	listcnt = mui->line[i].state.listcnt;

	/* initialize other variables, too */
	first = True;
	anyspc = False;
	title = False; /* nothing that causes a linebreak can appear in title */
	list = o_list(win);
#if 1
	textwidth = o_columns(win);
#else
	textwidth = o_textwidth(markbuffer(win->cursor));
	if (textwidth == 0)
		textwidth = 80;
#endif
	tabstop = o_tabstop(markbuffer(win->cursor));
	listind = o_shiftwidth(markbuffer(win->cursor)) / 2;
	if (listind < 2)
		listind = 2;
	col = 0;
	cursoff = markoffset(win->cursor);

	/* set up the drawchar function pointer */
	if (draw)
	{
		drawchar = draw;
	}
	else
	{
		drawchar = countchar;
		physcol = 0;
		wantoffset = markoffset(from);
		outcol = -1;
		wantcol = 0;
		outoffset = -1;
		prevoffset = -1;
	}

	return i;
}


/* Locate the title in a given buffer, and store it in mui->title.  Also
 * sets mui->endtitle to an appropriate value.  It is assumed that the mui
 * pointer has been initialized before this function is called.
 */
static void findtitle(buf)
	BUFFER	buf;	/* the buffer to scan */
{
	CHAR	*p;	/* used for scanning the buffer */
	TOKEN	*token;	/* a token from the buffer */
	MARKBUF	tmp;
	int	i;

	/* free the old title, if any */
	if (mui->title)
	{
		safefree(mui->title);
		mui->title = NULL;
	}

	/* read the first non-whitespace token from the buffer */
	scanalloc(&p, marktmp(tmp, buf, 0));
	do
	{
		token = (*mui->get)(&p);
	} while (token && isspace(token->text[0]));

	/* if no token, then no title */
	if (!token)
	{
		mui->endtitle = o_bufchars(buf);
		scanfree(&p);
		return;
	}

	/* if token doesn't explicitly allow title to follow, then no title */
	if (!token->markup || token->markup->TITLE != 'Y')
	{
		mui->endtitle = token->offset[0];
		scanfree(&p);
		return;
	}

	/* Collect characters from text tokens, up to next markup which
	 * can't appear in the title.
	 */
	while ((token = (*mui->get)(&p)) != NULL && (!token->markup || token->markup->TITLE == 'Y'))
	{
		/* if text, then append it to title, with a leading blank */
		if (!token->markup && !isspace(token->text[0]))
		{
			if (mui->title)
				buildCHAR(&mui->title, ' ');
			for (i = 0; i < token->nchars; i++)
				buildCHAR(&mui->title, token->text[i]);
		}
	}
	scanfree(&p);
}


/* Adjust the line tables of any window whose main buffer has been altered.
 * This function is meant to be called only from bufreplace().
 */
void dmmuadjust(from, to, delta)
	MARK	from;	/* old start of text */
	MARK	to;	/* old end of text */
	long	delta;	/* difference between old "to" and new "to" offsets */
{
	WINDOW	win;	/* used for scanning windows */
	CHAR	*p;	/* used for scanning text */
	TOKEN	*token;	/* used for scanning text */
	LINEINFO li;	/* buffer for generating LINEINFO record */
	LINEINFO *old;	/* old value of mui->line (previous line array) */
#if 0
	int	noldlines;/* old value of mui->nlines */
#endif
	MARKBUF	tmp;
	int	i;

#ifdef DEBUG_MARKUP
	fprintf(stderr, "dmmuadjust({%s,%ld}, {%s,%ld}, %ld)\n",
		o_bufname(markbuffer(from)), markoffset(from),
		o_bufname(markbuffer(to)), markoffset(to),
		delta);
#endif

	/* for each window that uses this buffer... */
	for (win = winofbuf(NULL, markbuffer(from));
	     win;
	     win = winofbuf(win, markbuffer(from)))
	{
		/* ignore if not in a markup display mode */
		if (win->md->setup != setup)
		{
			continue;
		}

		/* If necessary, rescan the title of this buffer */
		mui = (MUINFO *)win->mi;
		if (mui->endtitle <= markoffset(from))
			findtitle(markbuffer(from));

		/* Start on the line BEFORE the one where changes begin.
		 * This is because if we insert a small word at the front of
		 * a line, it might fit at the end of the preceding line.
		 */
		i = start(win, from, (void(*)P_((CHAR *,long,_char_,long)))0);
		if (i > 0) i--;

		/* Pretend the cursor is someplace harmless.  This is done
		 * because the cursor position affects the appearance of the
		 * text, and sometimes this affects where line breaks occur.
		 */
		cursoff = o_bufchars(markbuffer(from));

		/* regenerate that preceding line */
		p = scanalloc(&p, marktmp(tmp, markbuffer(from), mui->line[i].offset));
		do
		{
			token = (*mui->get)(&p);
		} while (token && !put(token));

		/* regenerate the line where the change took place */
		if (token && token->offset[token->nchars - 1] != o_bufchars(markbuffer(from)) - 1)
		{
			/* but first, store results of preceding line's scan */
			i++;
			storestate(token->offset[0], &mui->line[i]);

			/* okay, now scan the changed line.  We already have
			 * the first token
			 */
			while (token && !put(token))
			{
				token = (*mui->get)(&p);
			}
		}

		/* If we hit the end of the buffer, then no adjustments are
		 * necessary.  This may leave superfluous data at the end of
		 * the array, but who cares? 
		 */
		if (!token || token->offset[token->nchars - 1] == o_bufchars(markbuffer(from)) - 1)
		{
			mui->nlines = i + 1;
			scanfree(&p);
#ifdef DEBUG_MARKUP
			fprintf(stderr, "\tNo adjustment needed; hit end of buffer\n");
#endif
			continue;
		}

		/* If the new line ended with the same (adjusted) offset and
		 * same state, then no adjustments are necessary except to
		 * the offsets.
		 */
		storestate(token->offset[0], &li);
		if (i + 1 < mui->nlines
		 && mui->line[i + 1].offset + delta == token->offset[0]
		 && !memcmp(&li.state, &mui->line[i + 1].state, sizeof li.state)
		 && mui->line[i + 1].offset >= markoffset(to))
		{
			if (delta == 0)
			{
#ifdef DEBUG_MARKUP
				fprintf(stderr, "\tNo adjustment needed; same state and delta=0\n");
#endif
				scanfree(&p);
				continue;
			}

			for (i++; i < mui->nlines; i++)
			{
				mui->line[i].offset += delta;
			}
			scanfree(&p);
#ifdef DEBUG_MARKUP
			fprintf(stderr, "\tAdjusted the offsets only!\n");
#endif
			continue;
		}
		
		/* Adjustments are going to be complex.  Start a new array,
		 * but don't throw the old one out yet because it may save
		 * us some work.
		 */
		old = mui->line;
#if 0
		noldlines = mui->nlines;
#endif
		mui->line = safealloc((i + 1 + GRANULARITY) - ((i + 1) % GRANULARITY), sizeof(LINEINFO));
		for (mui->nlines = 0; mui->nlines <= i; mui->nlines++)
		{
			mui->line[mui->nlines] = old[mui->nlines];
		}
		mui->line[mui->nlines++] = li;

		/* regenerate lines until we have same (adjusted) offset and
		 * state as we had before the change.
		 */ 
		do
		{
			/* Regenerate a line */
			put(token);
			do
			{
				token = (*mui->get)(&p);
			} while (token && !put(token));
			if (!token)
			{
				break;
			}

			/* Append its state to the new array.  */
			storestate(token->offset[0], (LINEINFO *)0);

			/* if the line's offset comes after the adjusted "to"
			 * offset, and there is an old state & adjusted offset
			 * which match, then just copy the old info.
			 */
			/*!!!*/
		} while (token);
		scanfree(&p);
		safefree(old);
#ifdef DEBUG_MARKUP
		fprintf(stderr, "\tRegenerated info for every following line\n");
#endif
	}
}


/*----------------------------------------------------------------------------*/
DISPMODE dmhtml =
{
	"html",
	"WWW hypertext",
	False,	/* display generating can't be optimized */
	False,	/* shouldn't use standard wordwrap */
	0,	/* no window options */
	NULL,
	0,	/* no global options */
	NULL,
	NULL,
	htmlinit,
	term,
	mark2col,
	move,
	NULL,	/* wordmove will be set to dmnormal.wordmove in init() */
	setup,
	image,
	header,
	NULL,	/* no autoindent */
	htmltagatcursor,
	htmltagload,
	htmltagnext
};

DISPMODE dmman =
{
	"man",
	"nroff -man",
	False,	/* display generating can't be optimized */
	False,	/* shouldn't use standard wordwrap */
	0,	/* no window options */
	NULL,
	0,	/* no global options */
	NULL,
	NULL,
	maninit,
	term,
	mark2col,
	move,
	NULL,	/* wordmove will be set to dmnormal.wordmove in init() */
	setup,
	image,
	header,
	NULL,	/* no autoindent */
	NULL,	/* tagatcursor will be set to dmnormal.tagatcursor in init() */
	NULL,	/* tagload will be set to dmnormal.tagload in init() */
	NULL,	/* tagnext will be set to dmnormal.tagnext in init() */
};
#endif /* DISPLAY_MARKUP */
