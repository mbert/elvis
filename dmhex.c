/* dmhex.c */
/* Copyright 1995 by Steve Kirkendall */

char id_dmhex[] = "$Id: dmhex.c,v 2.13 1996/05/24 00:53:01 steve Exp $";

#include "elvis.h"
#ifdef DISPLAY_HEX

#ifdef CHAR16
"hex mode doesn't support 16-bit characters"
#endif

#if USE_PROTOTYPES
static DMINFO *init(WINDOW win);
static void term(DMINFO *info);
static long mark2col(WINDOW w, MARK mark, BOOLEAN cmd);
static MARK move(WINDOW w, MARK from, long linedelta, long column, BOOLEAN cmd);
static MARK setup(MARK top, long cursor, MARK bottom, DMINFO *info);
static MARK image(WINDOW w, MARK line, DMINFO *info, void (*draw)(CHAR *p, long qty, _char_ font, long offset));
#endif

/* Lines look like this:
_offset____0__1__2__3___4__5__6__7___8__9__a__b___c__d__e__f__0123456789abcdef
00000000  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  0000000000000000
00000000  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  0000000000000000
00000000  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  0000000000000000
00000000  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  0000000000000000
00000000  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  0000000000000000
00000000  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  0000000000000000
*/

/* This array is used to convert a column number to an offset within a line */
static char col2offset[] = "00000000000011122233334445556667777888999aaabbbbcccdddeeeffff00123456789abcdef";

/* start the mode, and allocate dminfo */
static DMINFO *init(win)
	WINDOW	win;
{
	/* inherit some functions from dmnormal */
	if (!dmhex.wordmove)
	{
		dmhex.wordmove = dmnormal.wordmove;
		dmhex.tagatcursor = dmnormal.tagatcursor;
		dmhex.tagload = dmnormal.tagload;
		dmhex.tagnext = dmnormal.tagnext;
	}

	return NULL;
}

/* end the mode, and free the modeinfo */
static void term(info)
	DMINFO	*info;	/* window-specific info about mode */
{
}

/* Convert a mark to a screen-relative column number */
static long mark2col(w, mark, cmd)
	WINDOW	w;	/* window where buffer is shown */
	MARK	mark;	/* mark to convert */
	BOOLEAN	cmd;	/* if True, we're in command mode; else input mode */
{
	return 62 + (markoffset(mark) & 0xf);
}

/* Move vertically, and to a given column (or as close to column as possible) */
static MARK move(w, from, linedelta, column, cmd)
	WINDOW	w;		/* window where buffer is shown */
	MARK	from;		/* old location */
	long	linedelta;	/* line movement */
	long	column;		/* desired column number */
	BOOLEAN	cmd;		/* if True, we're in command mode; else input mode */
{
	static MARKBUF	mark;
	long		offset;
	long		coloff;

	/* compute line movement first */
	offset = (markoffset(from) & ~0xf) + 0x10 * linedelta;

	/* convert requested column to nearest buffer offset */
	if (column >= (int)strlen(col2offset))
	{
		coloff = 0xf;
	}
	else if (col2offset[column] >= '0' && col2offset[column] <= '9')
	{
		coloff = col2offset[column] - '0';
	}
	else
	{
		coloff = col2offset[column] - 'a' + 0xa;
	}
	offset += coloff;

	/* never return an offset past the end of the buffer, or before the
	 * beginning.
	 */
	if (offset >= o_bufchars(markbuffer(from)))
	{
		if (o_bufchars(markbuffer(from)) == 0)
		{
			offset = 0;
		}
		else
		{
			offset = o_bufchars(markbuffer(from)) - 1;
		}
	}
	else if (offset < 0)
	{
		offset = coloff;
	}

	return marktmp(mark, markbuffer(from), offset);
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
	static MARKBUF	mark;
	long		topoff, bottomoff;

	/* extract the offsets.  Round down to multiple of 16 */
	topoff = (top ? (markoffset(top) & ~0xf) : -1);
	bottomoff = (bottom ? (markoffset(bottom) & ~0xf) : -1);
	cursor &= ~0xf;

	/* if cursor is on the screen, or very near the bottom, then
	 * keep the current top
	 */
	if (cursor >= topoff && cursor <= bottomoff + 16)
	{
		return marktmp(mark, markbuffer(top), topoff);
	}

	/* if the cursor is on the line before the top, then make the cursor's
	 * line become the new top line.
	 */
	if (cursor == topoff - 16)
	{
		return marktmp(mark, markbuffer(top), cursor);
	}

	/* else it is distantly before or after the the old screen.  Center
	 * the cursor's line in the screen.
 	 */
	topoff = (cursor - (bottomoff - topoff) / 2) & ~0xf;
	if (topoff < 0)
	{
		topoff = 0;
	}
	return marktmp(mark, markbuffer(top), topoff);
}

static MARK image(w, line, info, draw)
	WINDOW	w;		/* window where drawing will go */
	MARK	line;		/* start of line to draw */
	DMINFO	*info;		/* window-specific info about mode */
	void	(*draw)P_((CHAR *p, long qty, _char_ font, long offset));
				/* function for drawing a single character */
{
	char	*c8p;
	CHAR	*cp;
	CHAR	tmp;
	CHAR	space;	/* usually a space character, maybe bracket character */
	char	buf[10];
	int	i, j;

	/* output headings, if necessary */
	if ((markoffset(line) & 0xf0) == 0)
	{
		for (c8p = " offset    0  1  2  3   4  5  6  7   8  9  a  b   c  d  e  f  0123456789abcdef";
		     *c8p; c8p++)
		{
			tmp = *c8p;
			(*draw)(&tmp, 1, 'u', -1);
		}
		tmp = '\n';
		(*draw)(&tmp, 1, 'n', -1);
	}

	/* output the line offset */
	sprintf(buf, "%08lx", markoffset(line));
	for (i = 0; buf[i]; i++)
	{
		tmp = buf[i];
		(*draw)(&tmp, 1, 'n', -1);
	}

	/* output the hex codes of the line */
	j = markoffset(w->cursor) - markoffset(line);
	space = ' ';
	for ((void)scanalloc(&cp, line), i = 0; i < 16 && cp; scannext(&cp), i++)
	{
		if ((i & 0x03) == 0)
		{
			(*draw)(&space, 1, 'n', -1);
			space = ' ';
		}
		if (j == i)
		{
			tmp = '<';
			(*draw)(&tmp, 1, 'n', -1);
			space = '>';
		}
		else
		{
			(*draw)(&space, 1, 'n', -1);
			space = ' ';
		}
		sprintf(buf, "%02x", *cp);
		tmp = buf[0];
		(*draw)(&tmp, 1, (char)(j==i ? 'u' : 'n'), markoffset(line) + i);
		tmp = buf[1];
		(*draw)(&tmp, 1, (char)(j==i ? 'u' : 'n'), markoffset(line) + i);
	}
	(*draw)(&space, 1, 'n', -1);

	/* pad with blanks, if necessary */
	space = ' ';
	while (i < 16)
	{
		(*draw)(&space, ((i & 0x03) == 0) ? -4 : -3, 'n', -1);
		i++;
	}
	(*draw)(&space, 1, 'n', -1);

	/* output the characters */
	tmp = '.';
	for ((void)scanseek(&cp, line), i = 0; i < 16 && cp; scannext(&cp), i++)
	{
		if (*cp < ' ' || *cp == '\177')
		{
			(*draw)(&tmp, 1, 'n', markoffset(line) + i);
		}
		else
		{
			(*draw)(cp, 1, 'n', markoffset(line) + i);
		}
	}
	scanfree(&cp);
	tmp = '\n';
	(*draw)(&tmp, 1, 'n', -1);

	/* output a blank line after every 16th data line */
	if ((markoffset(line) & 0xf0) == 0xf0)
	{
		(*draw)(&tmp, 1, 'n', -1);
	}

	markoffset(line) += i;
	return line;
}

DISPMODE dmhex =
{
	"hex",
	"Binary hex dump",
	False,	/* display generating can't be optimized */
	False,	/* shouldn't do normal wordwrap */
	0,	/* no window options */
	NULL,
	0,	/* no global options */
	NULL,
	NULL,
	init,
	term,
	mark2col,
	move,
	NULL,	/* wordmove will be set to dmnormal.wordmove in init() */
	setup,
	image,
	NULL,	/* doesn't need a header */
	NULL	/* no autoindent */
};
#endif /* DISPLAY_HEX */
