/* draw.c */
/* Copyright 1995 by Steve Kirkendall */

char id_draw[] = "$Id: draw.c,v 2.65 1996/09/26 01:05:52 steve Exp $";

#include "elvis.h"

#if defined (GUI_WIN32)
# define SLOPPY_ITALICS
#else
# undef SLOPPY_ITALICS
#endif

#if USE_PROTOTYPES
static void insimage(CHAR *ch, char *font, int qty, int extent);
static void delimage(CHAR *ch, char *font, int qty, int extent);
static void fillcell(_CHAR_ ch, _char_ font, long offset);
static void drawchar(CHAR *p, long qty, _char_ font, long offset);
static void compareimage(WINDOW win);
static void updateimage(WINDOW win);
static void genlastrow(WINDOW win);
static BOOLEAN drawquick(WINDOW win);
static void opentextline(WINDOW win, CHAR *text, int len);
static void openchar(CHAR *p, long qty, _char_ font, long offset);
static void openmove(WINDOW win, long oldcol, long newcol, CHAR *image, long len);
#endif

/* allocate a drawinfo struct, including the related arrays */
DRAWINFO *drawalloc(rows, columns)
	int	rows;	/* height of new window image */
	int	columns;/* width of new window image */
{
	DRAWINFO *newp;
	int	 i;

	/* allocate the stuff */
	newp = (DRAWINFO *)safealloc(1, sizeof *newp);
	newp->newrow = (DRAWROW *)safealloc(rows, sizeof(DRAWROW));
	newp->newline = (DRAWLINE *)safealloc(rows + 1, sizeof(DRAWLINE));
	newp->curline = (DRAWLINE *)safealloc(rows, sizeof(DRAWLINE));
	newp->newchar = (CHAR *)safealloc(rows * columns, sizeof(CHAR));
	newp->newfont = (char *)safealloc(rows * columns, sizeof(char));
	newp->curchar = (CHAR *)safealloc(rows * columns, sizeof(CHAR));
	newp->curfont = (char *)safealloc(rows * columns, sizeof(char));
	newp->offsets = (long *)safealloc(rows * columns, sizeof(long));

	/* clear the current image and the "new" image */
	for (i = rows * columns; --i >= 0; )
	{
		newp->newchar[i] = newp->curchar[i] = ' ';
		newp->newfont[i] = 'n';
		newp->curfont[i] = '?';
	}

	/* initialize the other variables */
	newp->rows = rows;
	newp->columns = columns;
	newp->logic = DRAW_SCRATCH;

	return newp;
}

/* free a drawinfo struct, including the related arrays */
void drawfree(di)
	DRAWINFO	*di;	/* window image to destroy */
{
	/* free the stuff */
	safefree(di->newrow);
	safefree(di->newline);
	safefree(di->curline);
	safefree(di->newchar);
	safefree(di->newfont);
	safefree(di->curchar);
	safefree(di->curfont);
	safefree(di->offsets);
	if (di->openimage)
	{
		safefree(di->openimage);
	}
	if (di->openline)
	{
		markfree(di->openline);
	}
	safefree(di);
}

/* re-output a rectangular part of a window's current image.  The top-left
 * corner of the window is (0,0).
 *
 * The redrawn area includes the interior and edges of the window.  This means,
 * for example, that if top==bottom, a single row will be (partially) redrawn.
 */
void drawexpose(win, top, left, bottom, right)
	WINDOW	win;	/* window that was partially exposed */
	int	top;	/* top edge to be redrawn */
	int	left;	/* left edge to be redrawn */
	int	bottom;	/* bottom edge to be redrawn */
	int	right;	/* right edge to be redrawn */
{
	int	row, column, same, base, nonblank;
	MARKBUF m;
	long	firstline, lastline;

	assert(win != NULL && top >= 0 && left >= 0 && bottom < o_lines(win)
		&& right < o_columns(win) && top <= bottom && left <= right);

#if 0
	/* if we must redraw anyway, then do nothing */
	if (win->di->logic == DRAW_SCRATCH)
	{
		return;
	}
#endif

	/* if this GUI has no moveto() function, then do nothing */
	if (!gui->moveto)
	{
		return;
	}

	/* for each row in the rectangle... */
	for (row = top; row <= bottom; row++)
	{
#if 1
		/* find the width of this row, ignoring trailing blanks */
		for (nonblank = o_columns(win), base = o_columns(win) * row;
		     nonblank > left
			&& win->di->curfont[base + nonblank - 1] == 'n'
			&& win->di->curchar[base + nonblank - 1] == ' ';
		     nonblank--)
		{
		}

		/* move to the first character we'll be redrawing in this row */
		guimoveto(win, left, row);

		/* for each segment of the row which shares the same font... */
		for (column = left, base += column;
		     column <= right && column < nonblank;
		     base += same, column += same)
		{
			/* find the width of the segment */
			for (same = 1;
			     column + same <= right
				&& column + same < nonblank
				&& win->di->curfont[base + same] == win->di->curfont[base];
			     same++)
			{
			}

			/* output the segment */
			guidraw(win, win->di->curfont[base], &win->di->curchar[base], same);
		}

		/* if necessary, do a clrtoeol */
		if (nonblank < right)
		{
			guiclrtoeol(win);
		}
#else
		/* for each column in the row */
		for (base = o_columns(win) * row + left, column = left;
		     column <= right;
		     base++, column++)
		{
			/* blot out elvis' idea of the current image.  This
			 * will cause elvis to re-output it the next time
			 * the image is updated.
			 */
			win->di->curfont[base] = '\0';
		}
#endif
	}

#if 1
	/* leave the cursor in the right place */
	guimoveto(win, win->di->curscol, win->di->cursrow);
#endif

	/* update the scrollbar, too */
	if (gui->scrollbar)
	{
		if (win->md->move == dmnormal.move)
		{
			/* find line numbers of first and last lines shown */
			firstline = markline(marktmp(m, markbuffer(win->cursor), win->di->topline)) - 1;
			lastline = markline(marktmp(m, markbuffer(win->cursor), win->di->bottomline)) - 1;

			/* Some scrolling commands temporarily set bottomline
			 * to the end of the buffer.  Ordinarily this causes
			 * no problems, but if an expose event occurs before
			 * the screen is redrawn, the the scrollbar's "thumb"
			 * will briefly extend to the bottom of the scrollbar.
			 * If we seem to be in that situation, then make a more
			 * reasonable guess about the screen bottom line number.
			 */
			if (lastline - firstline >= o_lines(win))
			{
				lastline = firstline + o_lines(win) - 1;
			}

			/* update the scrollbar */
			(*gui->scrollbar)(win->gw, firstline, lastline,
				o_buflines(markbuffer(win->cursor)));
		}
		else
		{
			(*gui->scrollbar)(win->gw, win->di->topline,
				win->di->bottomline, win->di->curnbytes);
		}
	}
	guiflush();
}


/*----------------------------------------------------------------------------*/
/* This marks the end of the easy stuff!  The remainder of this file consists
 * of the drawimage() function and its support functions and variables.
 */


static WINDOW	thiswin;	/* The window being drawn */
static long	thisline;	/* the line being drawn */
static char	thislnumstr[16];/* line number, as a string */
static int	linesshown;	/* number of different lines visible */
static int	thiscell;	/* index of next cell to draw */
static int	thiscol;	/* logical column number */
static int	leftcol;	/* leftmost column to actually draw */
static int	rightcol;	/* rightmost column to actully draw, plus 1 */
static int	maxrow;		/* number of rows to be drawn */
static int	wantcurs;	/* largest acceptable cursor row */
static int	thisscroll;	/* scroll distance while drawing this line */
static int	scrollrows;	/* #rows (i.e., area) scrolled by gui->scroll */
static int	maxcell;	/* number of cells to be drawn */
static int	seloffset;	/* offset of character, during selection */
static char	selfont;	/* font of character, during selection */



/* insert some blanks into an image */
static void insimage(ch, font, qty, extent)
	CHAR	*ch;	/* where character insertion should begin */
	char	*font;	/* parallel array of font codes for "ch" characters */
	int	qty;	/* number of blanks to be inserted */
	int	extent;	/* number of characters after the insertion point */
{
	register int i;

	/* shift the characters and font codes */
	for (i = extent; --i >= qty; )
	{
		ch[i] = ch[i - qty];
		font[i] = font[i - qty];
	}

	/* initialize the newly inserted cells */
	if (gui->newblank)
	{
		/* the newly inserted characters are all normal spaces */
		for (i = 0; i < qty; i++)
		{
			ch[i] = ' ';
			font[i] = 'n';
		}
	}
	else
	{
		/* the new cells' contents are undefined */
		for (i = 0; i < qty; i++)
		{
			font[i] = '?';
		}
	}
}

/* delete some characters from an image, and add blanks to the end. */
static void delimage(ch, font, qty, extent)
	CHAR	*ch;	/* where character deletion should begin */
	char	*font;	/* parallel array of font codes for "ch" characters */
	int	qty;	/* number of characters to delete */
	int	extent;	/* number of characters after the deletion point */
{
	register int i;

	/* shift the characters */
	for (i = 0; i < extent - qty; i++)
	{
		ch[i] = ch[i + qty];
		font[i] = font[i + qty];
	}

	/* we've dragged some normal characters onto the edge of the extent */
	if (gui->newblank)
	{
		/* the new cells are filled with normal blanks */
		for ( ; i < extent; i++)
		{
			ch[i] = ' ';
			font[i] = 'n';
		}
	}
	else
	{
		/* the new cells' contents are undefined */
		for ( ; i < extent; i++)
		{
			font[i] = '?';
		}
	}
}

/* Fill in a character cell.  This function is called by drawchar(), which
 * in turn is called by an edit mode's image() function.
 */
static void fillcell(ch, font, offset)
	_CHAR_	ch;	/* new character to place in the next cell */
	_char_	font;	/* font code of new character */
	long	offset;	/* buffer offset, or -1 if not from buffer */
{
	register int		i;
	register DRAWINFO	*di = thiswin->di;

	assert(thiscell <= maxcell);
	assert(maxcell <= maxrow * di->columns);

	/* if we've reached the end of the screen without finding the cursor,
	 * then we'll need to do a little slop scrolling.
	 */
	if (thiscell == maxcell && (di->cursrow < 0 || di->cursrow > wantcurs))
	{
		/* scroll up 1 row */
		delimage(di->newchar, di->newfont, (int)o_columns(thiswin), (int)(maxcell + o_columns(thiswin)));
		for (i = 0; i < maxcell - o_columns(thiswin); i++)
		{
			di->offsets[i] = di->offsets[i + o_columns(thiswin)];
		}
		thisscroll++;

		/* scroll the statistics, too */
		di->newrow[1].insrows += di->newrow[0].insrows;
		for (i = 0; i < maxrow - 1; i++)
		{
			di->newrow[i] = di->newrow[i + 1];
		}
		di->newrow[i].insrows =
			di->newrow[i].shiftright =
			di->newrow[i].inschars = 0;
		for (i = 0; i < linesshown; i++)
		{
			di->newline[i].startrow--;
		}
		assert(di->cursrow != 0);
		if (di->cursrow > 0)
			di->cursrow--;

		/* if the top /line/ has scrolled off screen, then scroll
		 * the line statistics, too.
		 */
		if (linesshown > 1 && di->newline[1].startrow == 0)
		{
			linesshown--;
			for (i = 0; i < linesshown; i++)
			{
				di->newline[i] = di->newline[i + 1];
			}
		}

		/* adjust limits, taking o_sidescroll into consideration */
		thiscell -= o_columns(thiswin);
		if (o_wrap(thiswin))
		{
			rightcol += o_columns(thiswin);
		}
	}

	assert(thiscell < maxcell || di->cursrow >= 0);

	/* if this is the cursor character, then the cursor belongs here */
	if (offset == markoffset(thiswin->cursor))
	{
		di->cursrow = thiscell / o_columns(thiswin);
	}


	/* if this cell really should be drawn, then draw it */
	if (thiscol >= leftcol && thiscol < rightcol && thiscell < maxcell)
	{
		di->newchar[thiscell] = ch;
		di->newfont[thiscell] = font;
		di->offsets[thiscell] = offset;
		thiscell++;
	}

	/* increment the column number */
	thiscol++;
}


/* Add consecutive characters to the new image.  This function is called by
 * an edit mode's image() function to generate a screen image.  In addition
 * to displayable characters, this function gives special processing to
 * the following:
 *	'\013' (VTAB) is interpretted as two '\n' characters.
 *	'\f' is interpretted as two '\n' characters.
 *	'\n' moves to the start of the next row.
 * Note that tabs and backspaces are not given special treatment here;
 * that's the responsibility of the edit mode's image() function.
 */
static void drawchar(p, qty, font, offset)
	CHAR	*p;	/* the characters to be added */
	long	qty;	/* number of characters to add */
	_char_	font;	/* font code of characters */
	long	offset;	/* buffer offset of character, or -1 if not from buffer */
{
	register 	  CHAR	ch;		/* a character from *p */
	register DRAWINFO *di = thiswin->di;	/* window drawing info */
	long		  delta;		/* value to add to "offset" */
	CHAR		  hifont;		/* possibly highlighted font */
	CHAR		  tmpch;
	int		  i;

	assert(offset >= -1 && strchr("bungifeBUNGIFE", font) != NULL);


	/* A negative qty value indicates that all characters have the same
	 * offset.  Otherwise the characters will have consecutive offsets.
	 */
	if (qty < 0)
	{
		qty = -qty;
		delta = 0;
	}
	else
	{
		delta = 1;
	}

	/* for each character... */
	for ( ; qty > 0; qty--, p += delta, offset += delta)
	{
		/* copy the next char into a register variable */
		ch = *p;

		/* Treat '\f' and '\013' (VTAB) as two '\n's in a row
		 * (one of them recursively)
		 */
		if (ch == '\f' || ch == '\013')
		{
			tmpch = ch = '\n';
			drawchar(&tmpch, 1, font, offset);
		}
		/* Handle non-ascii characters */
		else if (ch >= 0x80)
		{
			switch (o_nonascii)
			{
			  case 's':	/* strip */
				ch &= 0x7f;
				if (ch < ' ' || ch == 0x7f)
					ch = '.';
				break;

			  case 'n':	/* none */
				ch = '.';
				break;	/* !!it B4: to make cc happy */

			  case 'm':	/* most */
				if (ch <= 0x9f)
					ch = '.';
				break;	/* !!it B4: to make cc happy */

			  default:	/* all */
				/* no conversion necessary */;
			}
		}

		/* If we're showing line numbers, and this character is supposed
		 * to be displayed in column 0, then output the line number
		 * before this character.
		 */
		if (o_number(thiswin) && thiscol == leftcol)
		{
			for (i = 0; i < 8; i++)
			{
				fillcell((CHAR)thislnumstr[i], 'n', -1);
			}
		}

		/* If the offset is in the selected region for this window,
		 * then make the font letter uppercase so that it will be
		 * drawn highlighted.  Also, make wide characters be either
		 * totally highlighted or totally unhighlighted.
		 */
		hifont = font;
		if (thiswin->seltop)
		{
			if (offset == seloffset && seloffset != -1)
			{
				hifont = selfont;
			}
			else
			{
				i = thiscol;
				if (o_number(thiswin))
					i -= 8;
				if (markoffset(thiswin->seltop) <= offset
					&& offset <= markoffset(thiswin->selbottom)
					&& thiswin->selleft <= i
					&& i <= thiswin->selright
					&& (ch != '\n' || thiswin->seltype != 'r'))
				{
					hifont = toupper(font);
				}
			}
			selfont = hifont;
			seloffset = offset;
		}
		else if (thiswin->match == offset)
		{
			hifont = toupper(font);
		}

		/* remember where this line started */
		if (linesshown == 0 || di->newline[linesshown - 1].start != thisline)
		{
			assert(linesshown <= maxrow);
			assert((thiscell % o_columns(thiswin)) ==
			    (o_number(thiswin) && leftcol == 0 ? 8 : 0));
			di->newline[linesshown].start = thisline;
			di->newline[linesshown++].startrow = thiscell / o_columns(thiswin);
		}

		/* if this is a newline, then treat it as a bunch of spaces */
		if (ch == '\n')
		{
			/* remember this line's width.  If a single "line"
			 * involves multiple newlines, then the width of the
			 * last non-empty phsyical line is kept.
			 */
			if (thiscol != 0)
			{
				di->newline[linesshown].width = thiscol;
			}

			/* "leftcol" has no effect on newlines */
			if (thiscol < leftcol)
			{
				thiscol = leftcol;
			}

			/* Output a bunch of spaces.  Note that the looping
			 * condition is sensitive to both the column number and
			 * the cell number, so it'll correctly handle lines
			 * whose width==o_columns(win).  It emulates the :xw:
			 * brain-damaged newline, like a VT-100.
			 *
			 * This gets a bit tricky when you're appending to the
			 * end of a line, and you've just extended the line to
			 * reach edge of the screen.  In this case, an extra
			 * blank line *should* appear on the screen; the cursor
			 * will be there.  This test must be made outside the
			 * loop.
			 */
#if 0
			if (offset == markoffset(thiswin->state->cursor)
				&& thiscol % o_columns(thiswin) == 0)
			{
				thiscol = leftcol;
			}
			while (thiscol == leftcol || thiscell % o_columns(thiswin) != 0)
			{
				fillcell(' ', hifont, offset);
			}
#else
			i = o_columns(thiswin);
			if ((offset == markoffset(thiswin->state->cursor)
				&& thiscol % i == 0) || thiscol == leftcol)
			{
				fillcell(' ', hifont, offset);
			}
			/* Note: thiscell may've changed since previous "if" */
			if (thiscell % i != 0)
			{
				for (i -= thiscell % i; i > 0; i--)
				{
					di->newchar[thiscell] = ' ';
					di->newfont[thiscell] = hifont;
					di->offsets[thiscell] = offset;
					thiscol++, thiscell++;
				}
			}
#endif

			/* reset the virtual column number */
			thiscol = 0;
		}
		else /* normal character */
		{
			assert(ch >= ' ');

			/* draw the character */
			fillcell(ch, hifont, offset);
		}
	}
}

/* This function compares old lines to new lines, and determines how much
 * insert/deleting we should do, and approximately where we should do it.
 */
static void compareimage(win)
	WINDOW	win;	/* window to be processed */
{
	int	i, j, k, diff, totdiff;
	struct prevmatch_s
	{
		DRAWLINE *line;			/* ptr to new line's current info */
		enum {NEW, BEFORE, AFTER} type;	/* line position, relative to any change */
	}	*prev;

	/* if we're supposed to redraw from scratch, then we won't be doing
	 * any inserting/deleting.
	 */
	if (win->di->logic == DRAW_SCRATCH)
	{
		for (i = 0; i < maxrow; i++)
		{
			win->di->newrow[i].shiftright = 0;
			assert(win->di->newrow[i].insrows == 0 && win->di->newrow[i].inschars == 0);
		}

		/* also, ignore anything in current image */
		memset(win->di->curchar, 0, maxcell * sizeof(CHAR));
		memset(win->di->curfont, 0, maxcell * sizeof(char));

		return;
	}

	/* allocate an array for matching new lines to old lines */
	prev = (struct prevmatch_s *)safealloc(linesshown, sizeof *prev);

	/* Try to match each new line against current lines */
	/* try to match top lines as being BEFORE a change */
	i = 0;
	for (j = 0; j < win->di->nlines && win->di->newline[0].start != win->di->curline[j].start; j++)
	{
	}
	if (j < win->di->nlines)
	{
		do
		{
			prev[i].line = &win->di->curline[j];
			prev[i].type = BEFORE;
		} while (++i < linesshown
		      && ++j < win->di->nlines
		      && win->di->newline[i].start == win->di->curline[j].start);
	}

	/* try to match bottom lines as being AFTER a change */
	for (k = linesshown - 1; k >= i; k--)
	{
		for (j = win->di->nlines - 1;
		     j >= 0
			&& o_bufchars(markbuffer(win->cursor)) - win->di->newline[k].start
				!= win->di->curnbytes - win->di->curline[j].start;
		     j--)
		{
		}
		if (j >= 0)
		{
			do
			{
				prev[k].line = &win->di->curline[j];
				prev[k].type = AFTER;
			} while (--k >= i
			      && --j >= i
			      && o_bufchars(markbuffer(win->cursor)) - win->di->newline[k].start
					== win->di->curnbytes - win->di->curline[j].start);
			break;
		}
		prev[k].type = NEW;
	}

	/* any intervening lines are NEW */
	for (; i < k; i++)
	{
		prev[i].type = NEW;
	}

	/* Compute character shifts/inserts/deletes for each row... */
	for (i = 0; i < maxrow && win->di->newrow[i].lineoffset < o_bufchars(markbuffer(win->cursor)); i++)
	{
		/* find the line shown there (actually its index into newline[] and prev[]) */
		for (j = 0; win->di->newrow[i].lineoffset != win->di->newline[j].start; j++)
		{
			assert(j + 1 < linesshown);
		}

		/* if it is a new line, then don't shift it */
		if (prev[j].type == NEW)
		{
			win->di->newrow[i].shiftright = 0;
			assert(win->di->newrow[i].inschars == 0);
		}
		else /* recognized old line, maybe insert/delete chars or rows */
		{
			/* number of chars to insert/delete is computed by change in length */
			win->di->newrow[i].inschars = win->di->newline[j].width - prev[j].line->width;
		}
	}
	for ( ; i < maxrow; i++)
	{
		/* lines after the end of the buffer will show a tilde */
		win->di->newrow[i].shiftright = 0;
		assert(win->di->newrow[i].inschars == 0 && win->di->newrow[i].inschars == 0);
	}

	/* Compute row insertion/deletion for each line... */
	for (i = totdiff = 0; i < linesshown; i++, totdiff += diff)
	{
		/* We never need to insert/delete rows to make a NEW line
		 * appear in the correct position.
		 */
		if (prev[i].type == NEW)
		{
			diff = 0;
			continue;
		}

		/* Compute the positional difference for this line.  Also the
		 * number of lines inserted/delete for preceding lines should
		 * be taken into consideration.
		 */
		diff = win->di->newline[i].startrow - prev[i].line->startrow - totdiff;

		/* If inserting, we'll insert at the line's old starting row.
		 * If deleting, we'll delete on a row above the line's starting
		 * row so the line's current image ends up on the intended line
		 */
		j = prev[i].line->startrow + totdiff;
		if (diff < 0)
		{
			j += diff;
		}

		/* logical lines may start on negative rows, due to slop scrolling,
		 * but physically we can only address lines >= 0.  Any insertions
		 * or deletions for a negative row should be applied to row 0.
		 */
		if (j < 0)
		{
			j = 0;
		}

		/* add the insertions/deletions for the appropriate row. */
		win->di->newrow[j].insrows += diff;
	}

#if 0
for (i = 0; i < maxrow; i++) printf("newrow[%d] = {lineoffset=%ld, insrows=%d, shiftright=%d, inschars=%d}, type=%s\n",
i, win->di->newrow[i].lineoffset, win->di->newrow[i].insrows, win->di->newrow[i].shiftright, win->di->newrow[i].inschars,
prev[i].type==BEFORE?"BEFORE":prev[i].type==NEW?"NEW":prev[i].type==AFTER?"AFTER":"?");
printf("---------------------------------------------------------------------\n");
#endif

	/* we don't need the prev array anymore */
	safefree(prev);
}


/* after we've collected the image and some update hints, copy the new image to
 * both the "current" image and also the GUI.
 */
static void updateimage(win)
	WINDOW	win;	/* window to be updated */
{
	int	row, column, same, used;
	int	ncols = o_columns(win);
	register int	  i, base;
	register DRAWINFO *di = win->di;
	register int	  rowXncols;

	/* for each row... */
	for (row = rowXncols = 0; row < o_lines(win); row++, rowXncols += ncols)
	{
		assert(di->newrow[row].insrows == 0 || di->logic != DRAW_SCRATCH);

		/* do we want to insert/delete rows? */
		if (di->newrow[row].insrows != 0)
		{
			/* try to perform the insert/delete for the GUI */
			guimoveto(win, 0, row);
			if (guiscroll(win, di->newrow[row].insrows, (BOOLEAN)(maxrow < o_lines(win))))
			{
				/* perform the insert/delete on the "current" image */
				if (di->newrow[row].insrows > 0)
				{
					/* insert some rows */
					insimage(&di->curchar[rowXncols],
						&di->curfont[rowXncols],
						di->newrow[row].insrows * ncols,
						(scrollrows - row) * ncols);

					/* shift amount of new lines is always 0 */
					for (i = row; i < row + di->newrow[row].insrows; i++)
					{
						di->newrow[i].shiftright =
							di->newrow[i].inschars = 0;
					}
				}
				else
				{
					/* delete some rows */
					delimage(&di->curchar[rowXncols],
						&di->curfont[rowXncols],
						-di->newrow[row].insrows * ncols,
						(scrollrows - row) * ncols);

					/* shift amount of new lines is always 0 */
					for (i = maxrow + di->newrow[row].insrows; i < maxrow; i++)
					{
						di->newrow[i].shiftright =
							di->newrow[i].inschars = 0;
					}
				}
			}
			else /* either we can't scroll, or have a huge insert/delete amount */
			{
				/* don't do any more scrolling after this */
				for (i = row; i < o_lines(win); i++)
				{
					di->newrow[i].insrows = 0;
				}
			}
		}

		/* perform shifting, if any */
		if (di->newrow[row].shiftright != 0)
		{
			/* see how many following rows have the same shift */
			for (same = 1;
			     gui->shiftrows && 
			        row + same < di->rows - 1 &&
				di->newrow[row].shiftright == di->newrow[row + same].shiftright;
			     same++)
			{
			}

			/* make the GUI do its shift */
			guimoveto(win, 0, row);
			if (guishift(win, di->newrow[row].shiftright, same))
			{
				/* shift the current image, too */
				for (i = row; i < row + same; i++)
				{
					if (di->newrow[i].shiftright > 0)
					{
						/* shift right by inserting */
						insimage(&di->curchar[i * ncols],
							&di->curfont[i * ncols],
							di->newrow[i].shiftright,
							ncols);
					}
					else 
					{
						/* shift left by deleting */
						delimage(&di->curchar[i * ncols],
							&di->curfont[i * ncols],
							-di->newrow[i].shiftright,
							ncols);
					}
					di->newrow[i].shiftright = 0;
				}
			}
		}

		/* figure out how much of the line we'll be using */
		base = ncols * row;
		for (used = ncols;
		     used > 0
			&& di->newchar[base + used - 1] == ' '
			&& di->newfont[base + used - 1] == 'n';
		     used--)
		{
		}

#ifdef SLOPPY_ITALICS
		/* If the line ends with an italic character, then pretend
		 * we're using one additional character.  This is so the last
		 * italic character's right edge isn't clipped off.
		 */
		if (used < ncols && used >= 1 && di->newfont[base + used - 1] == 'i')
		{
			used++;
		}
#endif

		/* look for mismatched segments */
		for (column = 0; column < used; base += same, column += same)
		{
			/* if this cell matches, then skip to next */
			if (di->newchar[base] == di->curchar[base]
			 && di->newfont[base] == di->curfont[base])
			{
				same = 1;
				continue;
			}

			/* move the GUI cursor to the point of interest */
			guimoveto(win, column, row);

			/* perform this row's character insert/delete now,
			 * unless we just shifted this line to the right, and
			 * we're still on the inserted blanks at the start
			 * of the row.
			 */
			if (column >= di->newrow[row].shiftright
			  && di->newrow[row].inschars != 0)
			{
				if (guishift(win, di->newrow[row].inschars, 1))
				{
					/* shift the current image, too */
					if (di->newrow[row].inschars > 0)
					{
						/* shift right by inserting */
						insimage(&di->curchar[rowXncols + column],
							&di->curfont[rowXncols + column],
							di->newrow[row].inschars,
							ncols - column);
					}
					else 
					{
						/* shift left by deleting */
						delimage(&di->curchar[rowXncols + column],
							&di->curfont[rowXncols + column],
							-di->newrow[row].inschars,
							ncols - column);
					}

					/* for other rows of the same line, insertions
					 * will now appear to be shifts.
					 */
					for (i = row + 1; i < maxrow && di->newrow[i].lineoffset == di->newrow[row].lineoffset; i++)
					{
						di->newrow[i].shiftright += di->newrow[i].inschars;
						di->newrow[i].inschars = 0;
					}
				}

				/* we have now done all the inserting/deleting
				 * that we're ever going to do for this row.
				 */
				di->newrow[row].inschars = 0;
			}

			/* See how many mismatched cells we can find with same
			 * font.  Allow small spans of matched characters, if
			 * it costs less to redraw unchanged characters than
			 * moving the cursor would cost.
			 */
			for (same = 1, i = 0;
			     column + same < used
				&& (di->newchar[base + same] != di->curchar[base + same]
					|| di->newfont[base + same] != di->curfont[base + same]
					|| i < gui->movecost)
				&& (di->newfont[base] == di->newfont[base + same]
#ifdef SLOPPY_ITALICS
					|| (di->newfont[base + same - 1] == 'i' && di->newfont[base + same] == 'n' && di->newchar[base + same] == ' ')
#endif
										 );
			     same++)
			{
				if (di->newchar[base + same] != di->curchar[base + same]
					|| di->newfont[base + same] != di->curfont[base + same])
				{
					i = 0;
				}
				else
				{
					i++;
				}
			}
			same -= i;

#ifdef SLOPPY_ITALICS
			/* if the change is in italics, and the preceding
			 * characters were also italic, then include those
			 * preceding characters in the redrawn span; otherwise 
			 * they would be distorted by the new text.
			 */
			while (column > 0
			    && di->newfont[base] == 'i'
			    && di->newfont[base - 1] == 'i'
			    && di->newchar[base - 1] != ' ')
			{
				base--;
				column--;
				same++;
			}
			guimoveto(win, column, row);
#endif

			/* write the cells to the GUI */
			guidraw(win, di->newfont[base], &di->newchar[base], same);

			/* write the cells to the "current" image */
			for (i = 0; i < same; i++)
			{
				di->curchar[base + i] = di->newchar[base + i];
				di->curfont[base + i] = di->newfont[base + i];
			}
		}

		/* Do we need to clear to the end of the row? */
		base = ncols * row;
		for (i = used;
		     i < ncols
			&& di->curchar[base + i] == ' '
			&& di->curfont[base + i] == 'n';
		     i++)
		{
		}
		if (i < ncols)
		{
			/* Yes, we need to do a clrtoeol */

			/* We must clear at least from column "i", but the
			 * cursor is probably already at column "used".
			 * If the GUI doesn't say we should move to "i",
			 * then move to "used" instead.
			 */
			if (!gui->minimizeclr)
			{
				i = used;
			}
			guimoveto(win, i, row);

			/* perform the clrtoeol */
			guiclrtoeol(win);
			while (i < ncols)
			{
				di->curchar[base + i] = ' ';
				di->curfont[base + i] = 'n';
				i++;
			}
		}
	}
}


/* This function generates the image of the last row.  This is where "showmode"
 * and "ruler" come into play.  Also, part of the previous line may be retained
 */
static void genlastrow(win)
	WINDOW	win;	/* window whose last row is to be generated */
{
	int	i, j, base;
	char	*scan;
	char	buf[25];

	/* were there any rows inserted or deleted? */
	for (i = 0; !win->di->newmsg && i < o_lines(win) - 1 && win->di->newrow[i].insrows == 0; i++)
	{
	}
	base = (o_lines(win) - 1) * o_columns(win);
	if (!win->di->newmsg && i < o_lines(win) - 1)
	{
		/* Some rows were inserted/deleted, and there are no unread
		 * messages on the last row; Assume the last row should be
		 * wiped out.
		 */
		for (i = o_columns(win); --i >= 0; )
		{
			win->di->newchar[base + i] = ' ';
			win->di->newfont[base + i] = 'n';
		}
	}
	else
	{
		/* No unread messages, and no rows inserted/deleted;
		 * Assume most of the last row is unchanged.
		 */
		for (i = o_columns(win); --i >= 0; )
		{
			win->di->newchar[base + i] = win->di->curchar[base + i];
			win->di->newfont[base + i] = win->di->curfont[base + i];
		}
		win->di->newmsg = False;
	}

	/* does the GUI have a status function? */
	if (gui->status)
	{
		/* yes, call it with status info */
		(*gui->status)(win->gw,
			win->cmdchars,
			markline(win->state->cursor),
			(*win->md->mark2col)(win, win->state->cursor, viiscmd(win)) + 1,
			maplrnchar(o_modified(markbuffer(win->cursor)) ? '*' : ','),
			win->state->modename);
	}
	else
	{
		/* no, but maybe show status on the window's bottom row */

		/* if "showmode", then show it */
		if (o_showmode(win))
		{
			scan = win->state->modename;
			for (i = base + o_columns(win) - 10; i < base + o_columns(win) - 3; i++)
			{
				win->di->newchar[i] = *scan++;
				win->di->newfont[i] = 'E';	/* !!it E3: I prefer it emphasized */
				if (!*scan)
				{
					scan = " ";
				}
			}
		}

		/* if "ruler", then show it */
		if (o_ruler(win) && o_bufchars(markbuffer(win->cursor)) > 0)
		{
			sprintf(buf, " %6ld%c%-4ld",
				markline(win->state->cursor),
				maplrnchar(o_modified(markbuffer(win->cursor)) ? '*' : ','),
				(*win->md->mark2col)(win, win->state->cursor, viiscmd(win)) + 1);
	
			scan = buf;
			for (i = base + o_columns(win) - 10 - strlen(buf); *scan; i++)
			{
				win->di->newchar[i] = *scan++;
				win->di->newfont[i] = 'n';
			}
		}
		else
		{
			/* show the maplearn character anyway */
			i = base + o_columns(win) - 15;
			win->di->newchar[i] = maplrnchar(o_modified(markbuffer(win->cursor)) ? '*' : ',');
			if (win->di->newchar[i] == ',')
				win->di->newchar[i] = ' ';
			win->di->newfont[i] = 'n';
		}

		/* if "showcmd" then show it */
		if (o_showcmd(win))
		{
			i = base + o_columns(win) - 22 - QTY(win->cmdchars);
			for (j = 0; win->cmdchars[j]; j++, i++)
			{
				win->di->newchar[i] = win->cmdchars[j];
				win->di->newfont[i] = 'n';
			}
			for ( ; j < QTY(win->cmdchars) - 1; j++, i++)
			{
				win->di->newchar[i] = ' ';
				win->di->newfont[i] = 'n';
			}
		}
	}

	/* if "showstack", then show it */
	if (o_showstack(win))
	{
#if 0
		for (state = win->state, i = base + o_columns(win) - 30; state; state = state->pop)
		{
			win->di->newchar[i] = *state->modename;
			win->di->newfont[i++] = 'b';
			if (state->acton == state->pop)
			{
				win->di->newchar[i] = '.';
				win->di->newfont[i++] = 'b';
			}
		}
		win->di->newchar[i] = ' ';
		win->di->newfont[i++] = 'n';
		win->di->newchar[i] = ' ';
		win->di->newfont[i++] = 'n';
		win->di->newchar[i] = ' ';
		win->di->newfont[i++] = 'n';
#else
		sprintf(buf, "   %ld/%ld/%ld", markoffset(win->state->top), markoffset(win->state->cursor), markoffset(win->state->bottom));
		scan = buf;
		for (i = base + o_columns(win) - 20 - strlen(buf); *scan; i++)
		{
			win->di->newchar[i] = *scan++;
			win->di->newfont[i] = 'n';
		}
#endif
	}

	/* the last row is never inserted/deleted/shifted */
	win->di->newrow[o_lines(win) - 1].insrows = 0;
	win->di->newrow[o_lines(win) - 1].inschars = 0;
	win->di->newrow[o_lines(win) - 1].shiftright = 0;
}


/* This function checks to see if we can skip regenerating a window image.
 * If not, then it returns False without doing anything...
 * 
 * But if we can, it updates the bottom row (status line) and moves the cursor
 * to where it should be, and then returns True.
 */
static BOOLEAN drawquick(win)
	WINDOW	win;	/* window to be updated */
{
	DRAWINFO *di = win->di;
	long	cursoff = markoffset(win->cursor);
	int	i, j, col, base;

	/* if long lines wrap onto multiple rows, adjust curscol accordingly */
	col = di->curscol;
	if (o_number(win))
		col += 8;
	if (o_wrap(win))
		col %= di->columns;
	else
		col -= di->skipped;

	/* check for obvious reasons that we might need to redraw */
	if (!o_optimize ||					/* user doesn't want to optimize */
	    !win->md->canopt ||					/* display mode doesn't support optimization */
	    di->logic != DRAW_NORMAL ||				/* last command made a subtle change */
	    di->drawstate != DRAW_VISUAL ||			/* screen doesn't already show image */
	    di->curchgs != markbuffer(win->cursor)->changes ||	/* last command made a normal change */
	    cursoff < di->topline ||				/* cursor off top of screen */
	    cursoff >= di->bottomline ||			/* cursor off bottom of screen */
	    col < 0 ||						/* cursor off left edge of screen */
	    col >= di->columns ||				/* cursor off right edge of screen */
	    win->seltop)					/* visually selecting text */
	{
		return False;
	}

	/* scan all rows at that column, looking for the cursor's offset */
	for (i = 0, j = col; di->offsets[j] != cursoff; i++, j += di->columns)
	{
		if (i >= di->rows - 2)
			return False;
	}

	/* Hooray!  We can skip generating a new image */
	di->cursrow = i;
	di->curscol = col;

	/* Except for the last line */
	genlastrow(win);
	for (col = 0, i = base = (di->rows - 1) * di->columns, j = -1;
	     col < di->columns - 1;
	     col++, i++)
	{
		if (di->newchar[i] != di->curchar[i] || di->newfont[i] != di->curfont[i])
		{
			if (j < 0)
			{
				j = i;
			}
			else if (di->newfont[j] != di->newfont[i])
			{
				guimoveto(win, j - base, di->rows - 1);
				guidraw(win, di->newfont[j], &di->newchar[j], i - j);
				j = i;
			}
		}
		else if (j > 0)
		{
			guimoveto(win, j - base, di->rows - 1);
			guidraw(win, di->newfont[j], &di->newchar[j], i - j);
			j = -1;
		}
	}
	if (j > 0)
	{
		guimoveto(win, j - base, di->rows - 1);
		guidraw(win, di->newfont[j], &di->newchar[j], i - j);
		j = -1;
	}
	memcpy(&di->curfont[base], &di->newfont[base], di->columns * sizeof(*di->curfont));
	memcpy(&di->curchar[base], &di->newchar[base], di->columns * sizeof(*di->curchar));

	/* leave the cursor in the right place */
	guimoveto(win, di->curscol, di->cursrow);

	/* That should do it */
	return True;
}


/* update the image of a window to reflect changes to its buffer */
void drawimage(win)
	WINDOW	win;	/* window to be updated */
{
	MARKBUF first, last;
	MARK	next;
	int	i, row;
	CHAR	tmpch;
	DRAWLOGIC nextlogic = DRAW_NORMAL;

	assert(gui->moveto && gui->draw);

	/* unavoidable setup performed here */
	if (o_bufchars(markbuffer(win->cursor)) > 0)
	{
		win->di->curscol = (*win->md->mark2col)(win, marktmp(first, markbuffer(win->cursor), markoffset(win->cursor)), viiscmd(win));
	}
	else
	{
		win->di->curscol = 0;
	}
	if (win->di->curbuf != markbuffer(win->cursor))
	{
		/* place the cursor's line at the center of the screen */
		win->di->topline = markoffset(dispmove(win, -(o_lines(win) / 2), 0));
		win->di->bottomline = o_bufchars(markbuffer(win->cursor));
		win->di->logic = DRAW_CENTER;
		win->di->curbuf = markbuffer(win->cursor);
	}

	/* see if maybe everything else *is* avoidable*/
	if (drawquick(win))
	{
		return;
	}

	/* setup, and choose a starting point for the drawing */
	next = (*win->md->setup)(marktmp(first, markbuffer(win->cursor), win->di->topline),
		markoffset(win->cursor), marktmp(last, markbuffer(win->cursor), win->di->bottomline), win->mi);
	thiswin = win;
	thiscell = 0;
	thiscol = 0;
	maxrow = o_lines(win) - 1;
	maxcell = maxrow * o_columns(win);
	scrollrows = (gui->scrolllast ? maxrow + 1 : maxrow);
	linesshown = 0;
	if (o_number(win))
	{
		win->di->curscol += 8; /* since line numbers will push text to the right */
	}
	if (o_wrap(win))
	{
		/* we're doing traditional vi line wrapping */
		leftcol = 0;
		for (i = 0; i < maxrow; i++)
		{
			thiswin->di->newrow[i].insrows =
				thiswin->di->newrow[i].shiftright =
				thiswin->di->newrow[i].inschars = 0;
		}
		win->di->curscol %= o_columns(win);
	}
	else
	{
		/* we're doing side-scroll.  Adjust leftcol, if necessary, to
		 * make sure the cursor will be visible after drawing.
		 */
		leftcol = win->di->skipped;
		i = win->di->curscol;
		while (i >= leftcol + o_columns(win))
		{
			leftcol += o_sidescroll(win);
		}
		if (o_number(win))
			i -= 8; /* because line number pushed it over 8 cols */
		while (i < leftcol)
		{
			leftcol -= o_sidescroll(win);
		}
		if (leftcol < 0)
		{
			leftcol = 0;
		}
		for (i = 0; i < maxrow; i++)
		{
			thiswin->di->newrow[i].insrows =
				thiswin->di->newrow[i].inschars = 0;
			thiswin->di->newrow[i].shiftright = win->di->skipped - leftcol;
		}
		win->di->skipped = leftcol;
		win->di->curscol -= leftcol;
	}
	win->di->cursrow = -1;
	seloffset = -1;

	/* are we going to try and center the cursor? */
	if (win->di->logic == DRAW_CENTER)
		wantcurs = win->di->rows / 2;
	else
		wantcurs = win->di->rows;

	/* draw each line until we reach the last row */
	win->di->topline = markoffset(next);
	for (row = 0; thiscell < maxcell || win->di->cursrow < 0 || (win->di->cursrow > wantcurs && markoffset(next) < o_bufchars(markbuffer(win->cursor))); )
	{
		/* set column limits, taking o_sidescroll into consideration */
		if (o_wrap(win))
		{
			rightcol = maxcell - thiscell;
		}
		else
		{
			rightcol = leftcol + o_columns(win);
		}

		/* if we're at the end of the buffer, pad with tildes */
		if (markoffset(next) >= o_bufchars(markbuffer(win->cursor)))
		{
			/* never show numbers on ~ lines */
			if (o_number(win))
				strcpy(thislnumstr, "        ");
			thiscol = leftcol;
			if (row != 0)
			{
				tmpch = '~';
				drawchar(&tmpch, 1, 'n', -1);
			}
			tmpch = '\n';
			drawchar(&tmpch, 1, 'n', -1);
			if (win->di->cursrow < 0)
			{
				win->di->cursrow = row;
			}
			win->di->newrow[row++].lineoffset = o_bufchars(markbuffer(win->cursor));
		}
		else
		{
			/* if we show line numbers, then convert this line's
			 * offset into a line number in ASCII format.
			 */
			if (o_number(win))
				sprintf(thislnumstr, "%6ld  ", markline(next));

			/* draw the line & remember its width */
			thisline = markoffset(next);
			thiswin->di->newline[linesshown].width = 0;
			thisscroll = 0;
			next = (*win->md->image)(win, next, win->mi, drawchar);

			/* remember where each row started. */
			row -= thisscroll;
			if (row < 0)
			{
				row = 0;
				nextlogic = DRAW_CHANGED;
			}
			for (row = (row > thisscroll) ? row - thisscroll : 0;
			     row < thiscell / o_columns(win);
			     row++)
			{
				win->di->newrow[row].lineoffset = thisline;
			}

			/* If the cursor probably should have been on this line,
			 * then set cursrow to the row we just finished, even if
			 * we didn't see the exact character where it belongs.
			 */
			if (thiswin->di->cursrow < 0 && markoffset(thiswin->cursor) < markoffset(next))
			{
				assert(row > 0);
				thiswin->di->cursrow = row - 1;
			}
		}
	}
	win->di->bottomline = markoffset(next);
	assert(linesshown > 0 && linesshown <= maxrow);

	/* update the scrollbar */
	if (gui->scrollbar)
	{
		if (win->md->move == dmnormal.move)
		{
			(*gui->scrollbar)(win->gw,
				markline(marktmp(first, markbuffer(win->cursor), win->di->newline[0].start)) - 1,
				markline(marktmp(last, markbuffer(win->cursor), markoffset(next))) - 1,
				o_buflines(markbuffer(win->cursor)));
		}
		else
		{
			(*gui->scrollbar)(win->gw, win->di->newline[0].start,
				markoffset(next), o_bufchars(markbuffer(next)));
		}
		guiflush();
	}

	/* compute insert/delete info */
	compareimage(win);

	/* figure out what the last row looks like */
	genlastrow(win);

	/* copy the image to the window */
	updateimage(win);

	/* remember some final info about this image, to help us update it
	 * efficiently next time.
	 */
	win->di->logic = nextlogic;
	win->di->topline = win->di->newline[0].start;
	win->di->bottomline = markoffset(next);
	win->di->nlines = linesshown;
	win->di->curnbytes = o_bufchars(markbuffer(next));
	win->di->curchgs = markbuffer(next)->changes;
	for (i = 0; i < linesshown; i++)
	{
		win->di->curline[i] = win->di->newline[i];
	}

	/* we're left in visual mode */
	win->di->drawstate = DRAW_VISUAL;

	/* leave the cursor in the right place */
	guimoveto(win, win->di->curscol, win->di->cursrow);
}

/*----------------------------------------------------------------------------*/
/* The follow are line-oriented output functions */

/* This function either calls gui->textline, or simulates it via the usual
 * screen update functions.
 */
static void opentextline(win, text, len)
	WINDOW	win;	/* window where text line is to be output */
	CHAR	*text;	/* text to be output */
	int	len;	/* length of text */
{
	int	i, j;

	/* if the GUI has a textline function, great! */
	if (gui->textline)
	{
		(*gui->textline)(win->gw, text, len);
		return;
	}

	/* else we have to simulate textline */

	/* for each character... */
	for (i = 0; i < len; i++)
	{
		/* process this character */
		switch (text[i])
		{
		  case '\b':
			if (win->di->opencell > 0)
				win->di->opencell--;
			thiscol = win->di->opencell % win->di->columns;
			break;

		  case '\t':
			do
			{
				win->di->newchar[win->di->opencell] = ' ';
				win->di->newfont[win->di->opencell] = 'n';
				win->di->opencell++;
				thiscol++;
			} while (thiscol < win->di->columns && thiscol % 8 != 0);
			break;
				
		  case '\r':
			win->di->opencell -= thiscol;
			thiscol = 0;
			break;

		  case '\n':
			win->di->opencell += win->di->columns;
			break;

		  case '\f':
		  case '\013':	/* VTAB character */
			/* ignore */
			break;

		  default:
			win->di->newchar[win->di->opencell] = text[i];
			win->di->newfont[win->di->opencell] = 'n';
			win->di->opencell++;
			thiscol++;
		}

		/* protect thiscol against wrap-around */
		if (thiscol >= win->di->columns)
		{
			thiscol %= win->di->columns;
		}

		/* if reached end of window, then scroll */
		while (win->di->opencell >= win->di->rows * win->di->columns)
		{
			delimage(win->di->newchar,
				win->di->newfont,
				win->di->columns,
				win->di->rows * win->di->columns);
			if (win->di->logic != DRAW_SCRATCH)
				win->di->newrow[0].insrows--;
			win->di->opencell -= win->di->columns;
			assert((win->di->opencell - thiscol) % win->di->columns == 0);
			for (j = win->di->opencell - thiscol;
			     j < win->di->rows * win->di->columns;
			     j++)
			{
				win->di->newchar[j] = ' ';
				win->di->newfont[j] = 'n';
			}
		}
	}

	/* set the cursor coordinates */
	win->di->curscol = thiscol;
	win->di->cursrow = win->di->opencell / win->di->columns;
}

/* This function is called before outputting a different line.  If the screen
 * is in open-edit state, then the current line is output along with a newline.
 * If the screen is in visual state, then the cursor is moved to the bottom of
 * the screen and its line is cleared.  Otherwise it has no effect.
 */
void drawopencomplete(win)
	WINDOW	win;	/* window where output will take place */
{
	int	len, i;

	switch (win->di->drawstate)
	{
	  case DRAW_VISUAL:
		/* SET UP FOR SCROLLING */

		/* row hints initially say rows won't change */
		for (i = 0; i < win->di->rows; i++)
		{
			/* win->di->newrow[i].lineoffset remains unchanged */
			win->di->newrow[i].insrows =
				win->di->newrow[i].shiftright =
				win->di->newrow[i].inschars = 0;
		}

		/* move to the bottom row */
		win->di->opencell = win->di->rows * win->di->columns - win->di->columns;
		thiscol = 0;

		/* clear the bottom row */
		for (i = win->di->opencell; i < win->di->rows * win->di->columns; i++)
		{
			win->di->newchar[i] = ' ';
			win->di->newfont[i] = 'n';
		}
		break;

	  case DRAW_OPENEDIT:
		thiscol = win->di->opencell % win->di->columns;
		len = win->di->opencnt - win->di->opencursor;
		if (len > 0)
		{
			opentextline(win, win->di->openimage + win->di->opencursor, len);
		}
		opentextline(win, toCHAR("\r\n"), 2);
		break;

	  case DRAW_OPENOUTPUT:
		thiscol = win->di->opencell % win->di->columns;
		break;

	  case DRAW_VMSG:
		/* scroll upward, and leave cursor on bottom */
		win->di->opencell = (win->di->rows - 1) * win->di->columns;
		thiscol = 0;
		win->di->newrow[0].insrows = 0;
		opentextline(win, toCHAR("\r\n"), 2);
		break;
	}

	/* either way, the current image is gone */
	if (win->di->openimage)
	{
		safefree(win->di->openimage);
		win->di->openimage = NULL;
	}
	if (win->di->openline)
	{
		markfree(win->di->openline);
		win->di->openline = NULL;
	}
	win->di->drawstate = DRAW_OPENOUTPUT;
}

/* Draw a message on the bottom row of a window.  If the previous message's
 * importance is MSG_STATUS, then overwrite; else append.  Handle "[more]"
 * prompts.
 */
void drawmsg(win, imp, verbose, len)
	WINDOW	win;		/* window that the message pertains to */
	MSGIMP	imp;		/* type of message */
	CHAR	*verbose;	/* text of message */
	int	len;		/* length of text */
{
	int	i, base;

	/* if the window is in open mode, then just write the message with
	 * a newline.
	 */
	if (win->di->drawstate != DRAW_VISUAL)
	{
		drawextext(win, verbose, len);
		drawextext(win, toCHAR("\n"), 1);
	}
	else /* full-screen */
	{
		/* draw the message on the bottom row */
		/* for now, any message overwrites any other */
		base = (o_lines(win) - 1) * o_columns(win);
		for (i = 0; i < o_columns(win); i++)
		{
			if (i < len)
			{
				win->di->newchar[base + i] = win->di->curchar[base + i] = verbose[i];
			}
			else
			{
				win->di->newchar[base + i] = win->di->curchar[base + i] = ' ';
			}
			win->di->newfont[base + i] = win->di->curfont[base + i] = 'n';
		}
		drawexpose(win, (int)(o_lines(win)-1), 0, (int)(o_lines(win)-1), (int)(o_columns(win)-1));
		win->di->newmsg = True;
	}
	
	/* non-status messages force us out of DRAW_VISUAL state */
	if (imp != MSG_STATUS && win->di->logic != DRAW_SCRATCH && win->di->drawstate == DRAW_VISUAL)
	{
		win->di->drawstate = DRAW_VMSG;
	}
}

/* these are used while generating a line of text */
static long	opencnt;	/* column where next image character goes */
static long	openoffsetcurs;	/* offset of cursor */
static CHAR	*openimage;	/* buffer, holds new line image */
static long	opensize;	/* size of openimage */
static BOOLEAN	opencursfound;	/* has the cursor's cell been found yet? */
static BOOLEAN	openskipping;	/* has the line containing the cursor been completed? */
static void openchar(p, qty, font, offset)
	CHAR	*p;	/* characters to be output */
	long	qty;	/* number of characters */
	_char_	font;	/* font code of that character */
	long	offset;	/* buffer offset of the character, or -1 if not from buffer */
{
	register CHAR ch;
	CHAR	*newp;
	long	delta;

	/* negative qty indicates that the same character is repeated */
	if (qty < 0)
	{
		delta = 0;
		qty = -qty;
	}
	else
	{
		delta = 1;
	}

	/* for each character */
	for ( ; qty > 0; qty--, p += delta, offset += delta)
	{
		/* copy *p into a register variable */
		ch = *p;

		/* ignore formfeed and VT, and extra lines */
		if (ch == '\f' || ch == '\013' || openskipping)
		{
			continue;
		}

		/* is the cursor in this line? */
		if (offset >= openoffsetcurs)
		{
			opencursfound = True;
		}

		/* newline is generally ignored */
		if (ch == '\n')
		{
			if (opencursfound)
			{
				openskipping = True;
			}
			else
			{
				opencnt = 0;
			}
			continue;
		}

		/* expand the size of the buffer, if necessary */
		if (opencnt >= opensize)
		{
			newp = (CHAR *)safealloc((int)(opensize + 80), sizeof(CHAR));
			memcpy(newp, openimage, (size_t)opensize);
			safefree(openimage);
			openimage = newp;
			opensize += 80;
		}

		/* store the character */
		openimage[opencnt++] = ch;
	}
}

/* output a bunch of backspace characters or characters to move the cursor
 * one way or the other on the current line.
 */
static void openmove(win, oldcol, newcol, image, len)
	WINDOW	win;	/* window whose cursor should be moved */
	long	oldcol;	/* old position of the cursor, relative to image */
	long	newcol;	/* new position of the cursor, relative to image */
	CHAR	*image;	/* image of the line being edited */
	long	len;	/* length of the image */
{
	static CHAR	tenbs[10] = {'\b', '\b', '\b', '\b', '\b', '\b', '\b', '\b', '\b', '\b'};
	static CHAR	tensp[10] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};

	/* left or right? */
	if (oldcol < newcol)
	{
		/* move right by writing characters from image */
		if (len < newcol)
		{
			/* we'll need to pad the image with blanks */
			if (oldcol < len)
			{
				opentextline(win, &image[oldcol], (int)(len - oldcol));
			}
			while (len + 10 < newcol)
			{
				opentextline(win, tensp, 10);
				len += 10;
			}
			if (len < newcol)
			{
				opentextline(win, tensp, (int)(newcol - len));
			}
		}
		else
		{
			/* don't pad with blanks; just write characters */
			opentextline(win, &image[oldcol], (int)(newcol - oldcol));
		}
	}
	else if (newcol < oldcol)
	{
		/* move right by outputing backspaces */
		while (newcol + 10 < oldcol)
		{
			opentextline(win, tenbs, 10);
			oldcol -= 10;
		}
		if (newcol < oldcol)
		{
			opentextline(win, tenbs, (int)(oldcol - newcol));
		}
	}
	/* else no movement was necessary */
}

/* draw the window's current line from scratch, or just update it, in open mode. */
void drawopenedit(win)
	WINDOW	win;	/* window whose current line is to be output */
{
	MARK	curline;
	long	curcol;
	long	first, last;	/* differences */
	long	i, max;

	/* find the start of the cursor's line & its column */
	curline = dispmove(win, 0, 0);
	curcol = dispmark2col(win);

	/* if we were editing a different line before, then end it now */
	if (win->di->drawstate != DRAW_OPENEDIT
	 || !win->di->openline
	 || markbuffer(win->di->openline) != markbuffer(curline)
	 || (markoffset(win->di->openline) != markoffset(curline)
		&& !win->state->acton))
	{
		drawopencomplete(win);
		curline = win->di->openline = markdup(curline);
	}
	/* else we're editing the same line as last time */

	/* generate the new line image */
	opencursfound = openskipping = False;
	opencnt = 0;
	openoffsetcurs = markoffset(win->state->cursor);
	openimage = (CHAR *)safealloc(80, sizeof(CHAR));
	opensize = 80;
	if (win->state->acton)
	{
		curline = (*dmnormal.setup)(curline, openoffsetcurs, win->state->cursor, win->mi);
		curline = (*dmnormal.image)(win, curline, win->mi, openchar);
	}
	else
	{
		curline = (*win->md->setup)(curline, openoffsetcurs, win->state->cursor, win->mi);
		curline = (*win->md->image)(win, curline, win->mi, openchar);
	}

	/* do we have an old image? */
	if (win->di->openimage)
	{
		/* find the differences */
		first = last = -1;
		max = (opencnt > win->di->opencnt) ? opencnt : win->di->opencnt;
		for (i = 0; i < max; i++)
		{
			if (i < opencnt && win->di->openimage[i] == openimage[i])
			{
				continue;
			}
			if (first < 0)
			{
				first = i;
			}
			last = i;
		}
	}
	else
	{
		/* the whole line is different */
		first = 0;
		last = opencnt - 1;
		win->di->opencursor = 0;
	}

	/* output the different parts of the line */
	if (first >= 0)
	{
		openmove(win, win->di->opencursor, first, openimage, opencnt);
		openmove(win, first, last + 1, openimage, opencnt);
		win->di->opencursor = last + 1;
	}

	/* move the cursor back where it wants to be */
	openmove(win, win->di->opencursor, curcol, openimage, opencnt);
	win->di->opencursor = curcol;

	/* remember this line */
	if (win->di->openimage)
	{
		safefree(win->di->openimage);
	}
	win->di->openimage = openimage;
	win->di->opencnt = opencnt;

	/* this leaves us in open+edit state */
	win->di->drawstate = DRAW_OPENEDIT;

	/* if we're simulating the textline function, then we need to flush
	 * the image to the window.
	 */
	if (!gui->textline)
	{
		updateimage(win);
		win->di->curscol = win->di->opencell % win->di->columns;
		win->di->cursrow = win->di->opencell / win->di->columns;
		guimoveto(win, win->di->opencell % win->di->columns, win->di->cursrow);
		win->di->newrow[0].insrows = 0;
	}
}


/* ex commands use this function to output information */
void drawextext(win, text, len)
	WINDOW	win;	/* window where ex output should be drawn */
	CHAR	*text;	/* text to be output */
	int	len;	/* length of text */
{
	int	i, width;

	/* if no window specified, then throw away the text */
	if (!win || eventcounter <= 1)
	{
		fwrite(tochar8(text), len, sizeof(char), stdout);
		if (text[len - 1] == '\n')
		{
			fwrite("\r", 1, sizeof(char), stdout);
		}
		return;
	}

	/* if we were editing a line before, finish it now */
	drawopencomplete(win);

	/* output the text one line at a time, inserting CR before each LF */
	for (i = 0; i < len; i += width + 1)
	{
		for (width = 0; i + width < len && text[i + width] != '\n'; width++)
		{
		}
		if (width > 0)
		{
			opentextline(win, text + i, width);
		}
		if (i + width < len)
		{
			opentextline(win, toCHAR("\r\n"), 2);
		}
	}

	/* flush the image to the screen */
	if (o_exrefresh)
	{
		updateimage(win);
		win->di->curscol = win->di->opencell % win->di->columns;
		win->di->cursrow = win->di->opencell / win->di->columns;
		guimoveto(win, win->di->curscol, win->di->cursrow);
		win->di->newrow[0].insrows = 0;
		if (gui->flush)
		{
			(*gui->flush)();
		}
	}
}
