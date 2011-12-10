/* xtext.c */

#include "elvis.h"
#ifdef GUI_X11
# include "guix11.h"

/* Graphic characters in a typical LATIN-1 font */
#define GCH	'\022'
#define GCV	'\031'
#define	GC1	'\016'
#define GC2	'\027'
#define GC3	'\013'
#define GC4	'\025'
#define GC5	'\017'
#define GC6	'\026'
#define GC7	'\015'
#define GC8	'\030'
#define GC9	'\014'


void x_ta_predict(xw, columns, rows)
	X11WIN		*xw;	/* top-level window to receive new text area */
	unsigned int	columns;/* width of the new text area */
	unsigned int	rows;	/* height of the new text area */
{
	/* remember font metrics */
	xw->ta.cellbase = x_defaultnormal->fontinfo->ascent;
	xw->ta.cellh = x_defaultnormal->height;
	xw->ta.cellw = x_defaultnormal->fontinfo->max_bounds.width;

	/* default window geometry */
	xw->ta.rows = rows;
	xw->ta.columns = columns;
	xw->ta.cursx = xw->ta.cursy = 0;
	xw->ta.w = xw->ta.columns * xw->ta.cellw + 2 * o_borderwidth;
	xw->ta.h = xw->ta.rows * xw->ta.cellh + 2 * o_borderwidth;

	/* default pixel values.  (We allocate these here, before windows
	 * are created, so that we can use them as background colors.)
	 */
	xw->ta.bg = x_loadcolor(x_background, x_white);
	xw->ta.fgnormal = x_loadcolor(x_foreground, x_black);
	xw->ta.fgfixed = x_loadcolor(x_fixedcolor, x_black);
	xw->ta.fgbold = x_loadcolor(x_boldcolor, xw->ta.fgnormal);
	xw->ta.fgemph = x_loadcolor(x_emphcolor, xw->ta.fgnormal);
	xw->ta.fgitalic = x_loadcolor(x_italiccolor, xw->ta.fgnormal);
	xw->ta.fgundln = x_loadcolor(x_underlinecolor, xw->ta.fgnormal);
	xw->ta.fgcursor = x_loadcolor(x_cursorcolor, xw->ta.fgnormal);
	xw->ta.owncursor = x_loadcolor(x_owncolor, xw->ta.fgnormal);
}

void x_ta_create(xw, x, y)
	X11WIN	*xw;	/* top-level window to receive new text area */
	int	x, y;	/* position of the text area within that window */
{
	/* create the widget window */
	xw->x = x;
	xw->y = y;
	xw->ta.win = XCreateSimpleWindow(x_display, xw->win,
		x + o_borderwidth, y + o_borderwidth,
		(unsigned)(xw->ta.w - 2 * o_borderwidth),
		(unsigned)(xw->ta.h - 2 * o_borderwidth),
		0, xw->ta.bg, xw->ta.bg);
	XSelectInput(x_display, xw->ta.win,
	    ButtonPressMask|ButtonMotionMask|ButtonReleaseMask|ExposureMask);


	/* pixmap creation, for storing image of character under cursor */
	xw->ta.undercurs = XCreatePixmap(x_display, xw->ta.win, xw->ta.cellw, xw->ta.cellh, (unsigned)x_depth);
	xw->ta.cursor = CURSOR_NONE;
	xw->ta.nextcursor = CURSOR_QUOTE;
}

void x_ta_destroy(xw)
	X11WIN	*xw;	/* top-level window whose text area should be destroyed */
{
	/* free the cursor pixmap */
	XFreePixmap(x_display, xw->ta.undercurs);

	/* free the widget window */
	XDestroyWindow(x_display, xw->ta.win);

	/* free the colors */
	x_unloadcolor(xw->ta.bg);
	x_unloadcolor(xw->ta.fgnormal);
	x_unloadcolor(xw->ta.fgfixed);
	x_unloadcolor(xw->ta.fgbold);
	x_unloadcolor(xw->ta.fgemph);
	x_unloadcolor(xw->ta.fgitalic);
	x_unloadcolor(xw->ta.fgundln);
	x_unloadcolor(xw->ta.fgcursor);
	x_unloadcolor(xw->ta.owncursor);
}


void x_ta_erasecursor(xw)
	X11WIN	*xw;	/* window whose cursor should be hidden */
{
	/* hide the cursor (if shown) */
	if (xw->ta.cursor != CURSOR_NONE)
	{
		if (xw->ismapped)
		{
			if (xw->grexpose)
			{
				XSetGraphicsExposures(x_display, xw->gc, False);
				xw->grexpose = False;
			}
			XCopyArea(x_display, xw->ta.undercurs, xw->ta.win, xw->gc,
				0, 0, xw->ta.cellw, xw->ta.cellh,
				(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh));
		}
		xw->ta.cursor = CURSOR_NONE;
	}
}

void x_ta_drawcursor(xw)
	X11WIN	*xw;	/* window whose cursor should be drawn */
{
	unsigned long	color;

	/* a NULL "xw" forces all cursors to be redrawn */
	if (!xw)
	{
		for (xw = x_winlist; xw; xw = xw->next)
		{
			xw->ta.nextcursor = xw->ta.cursor;
			x_ta_erasecursor(xw);
			x_ta_drawcursor(xw);
		}
		return;
	}

	/* if not mapped, then no cursor should be drawn */
	if (!xw->ismapped)
	{
		xw->ta.cursor = CURSOR_NONE;
		return;
	}

	/* if same as before, do nothing */
	if (xw->ta.nextcursor == xw->ta.cursor)
	{
		return;
	}

	/* choose a color */
	color = (x_ownselection ? xw->ta.owncursor : xw->ta.fgcursor);

	/* if some other cursor shape is already drawn there, then erase it */
	if (xw->ta.cursor != CURSOR_NONE)
	{
		x_ta_erasecursor(xw);
	}
	else /* save the image of the cursor where we'll draw the cursor */
	{
		if (xw->grexpose)
		{
			XSetGraphicsExposures(x_display, xw->gc, False);
			xw->grexpose = False;
		}
		XCopyArea(x_display, xw->ta.win, xw->ta.undercurs, xw->gc,
			(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
			xw->ta.cellw, xw->ta.cellh, 0, 0);
	}
	xw->ta.cursor = xw->ta.nextcursor;

	/* draw the cursor, using the cursor color */
	if (xw->grexpose)
	{
		XSetGraphicsExposures(x_display, xw->gc, False);
		xw->grexpose = False;
	}
	if (xw->fg != color)
	{
		XSetForeground(x_display, xw->gc, color);
		xw->fg = color;
	}
	switch (xw->ta.cursor)
	{
	  case CURSOR_INSERT:
		XFillRectangle(x_display, xw->ta.win, xw->gc,
			(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
			2, xw->ta.cellh);
		break;

	  case CURSOR_REPLACE:
	  case CURSOR_QUOTE:
		XFillRectangle(x_display, xw->ta.win, xw->gc,
			(int)(xw->ta.cursx * xw->ta.cellw), (int)((xw->ta.cursy + 1) * xw->ta.cellh - 2),
			xw->ta.cellw, 2);
		break;

	  case CURSOR_COMMAND:
		switch ((xw == x_hasfocus) ? o_textcursor : 'h')
		{
		  case 'h': /* hollow */
			XDrawRectangle(x_display, xw->ta.win, xw->gc,
				(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
				xw->ta.cellw - 1, xw->ta.cellh - 1);
			break;

		  case 'o': /* opaque */
			XFillRectangle(x_display, xw->ta.win, xw->gc,
				(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
				xw->ta.cellw, xw->ta.cellh);
			break;

		  case 'x': /* xor */
			xw->fg ^= xw->ta.bg;
			XSetForeground(x_display, xw->gc, xw->fg);
			XSetFunction(x_display, xw->gc, GXxor);
			XFillRectangle(x_display, xw->ta.win, xw->gc,
				(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
				xw->ta.cellw, xw->ta.cellh);
			XSetFunction(x_display, xw->gc, GXcopy);
			break;
		}
		break;

	  case CURSOR_NONE:
		break;
	}
}


/* Move the cursor to a given character cell.  The upper left
 * character cell is designated column 0, row 0.
 */
void x_ta_moveto(xw, column, row)
	X11WIN	*xw;	/* the window whose cursor is to be moved */
	int	column;	/* the column to move to */
	int	row;	/* the row to move to */
{
	if (xw->ta.cursx != column || xw->ta.cursy != row)
	{
		x_ta_erasecursor(xw);
		xw->ta.cursx = column;
		xw->ta.cursy = row;
	}
}


/* Displays text on the screen, starting at the cursor's
 * current position, in the given font.  The text string is
 * guaranteed to contain only printable characters.
 *
 * The font is indicated by a single letter.  The letter will
 * be lowercase normally, or uppercase to indicate that the
 * text should be visibly marked for the <v> and <V> commands.
 * The letters are:
 *	n/N	normal characters
 *	b/B	bold characters
 *	i/I	italic characters
 *	u/U	underlined characters
 *	g/G	graphic characters
 *
 * This function should move the text cursor to the end of
 * the output text.
 */
void x_ta_draw(xw, font, text, len)
	X11WIN	*xw;	/* the window where the text should be drawn */
	_char_	font;	/* the font code to use for drawing */
	CHAR	*text;	/* the text to draw */
	int	len;	/* number of characters in text */
{
	long		swapper;
	X_LOADEDFONT	*loaded;
	CHAR		*tmp = NULL;
	XGCValues	gcvalues;
	int		i;

	xw->ta.cursor = CURSOR_NONE;

	/* set the font & colors */
	switch (font)
	{
	  case 'b':
	  case 'B':
		gcvalues.foreground = xw->ta.fgbold;
		loaded = x_defaultbold;
		break;

	  case 'e':
	  case 'E':
		gcvalues.foreground = xw->ta.fgemph;
		loaded = x_defaultbold;
		break;

	  case 'i':
	  case 'I':
		gcvalues.foreground = xw->ta.fgitalic;
		loaded = x_defaultitalic;
		break;

	  case 'g':
	  case 'G':
		gcvalues.foreground = xw->ta.fgnormal;
		loaded = x_defaultnormal;
		tmp = safealloc(len, sizeof(CHAR));
		for (i = 0; i < len; i++)
		{
			if (loaded->fontinfo->min_char_or_byte2 <= '\013')
			{
				switch (text[i])
				{
				  case '-':	tmp[i] = GCH;	break;
				  case '|':	tmp[i] = GCV;	break;
				  case '1':	tmp[i] = GC1;	break;
				  case '2':	tmp[i] = GC2;	break;
				  case '3':	tmp[i] = GC3;	break;
				  case '4':	tmp[i] = GC4;	break;
				  case '5':	tmp[i] = GC5;	break;
				  case '6':	tmp[i] = GC6;	break;
				  case '7':	tmp[i] = GC7;	break;
				  case '8':	tmp[i] = GC8;	break;
				  case '9':	tmp[i] = GC9;	break;
				  default:	tmp[i] = text[i];
				}
			}
			else if (isdigit(text[i]))
			{
				tmp[i] = '+';
			}
			else
			{
				tmp[i] = text[i];
			}
		}
		text = tmp;
		break;

	  case 'u':
	  case 'U':
		gcvalues.foreground = xw->ta.fgundln;
		loaded = x_defaultnormal;
		break;

	  case 'f':
	  case 'F':
		gcvalues.foreground = xw->ta.fgfixed;
		loaded = x_defaultnormal;
		break;

	  default:
		gcvalues.foreground = xw->ta.fgnormal;
		loaded = x_defaultnormal;
	}
	gcvalues.background = xw->ta.bg;

	/* if font letter is uppercase, then swap foreground & background */
	if (isupper(font))
	{
		swapper = gcvalues.foreground;
		gcvalues.foreground = gcvalues.background;
		gcvalues.background = swapper;
	}

	/* set the GC values */
	gcvalues.font = (loaded ? loaded : x_defaultnormal)->fontinfo->fid;
	gcvalues.graphics_exposures = xw->grexpose = False;
	XChangeGC(x_display, xw->gc, 
		GCForeground|GCBackground|GCFont|GCGraphicsExposures, &gcvalues);
	xw->fg = gcvalues.foreground;
	xw->bg = gcvalues.background;

	/* draw the text */
	XDrawImageString(x_display, xw->ta.win, xw->gc,
		(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh + xw->ta.cellbase),
		tochar8(text), len);
	if ((font == 'b' || font == 'B' || font == 'e' || font == 'E') && !x_defaultbold)
	{
		XDrawString(x_display, xw->ta.win, xw->gc,
			(int)(xw->ta.cursx * xw->ta.cellw + 1), (int)(xw->ta.cursy * xw->ta.cellh + xw->ta.cellbase),
			tochar8(text), len);
	}
	if ((font == 'u' || font == 'U') && o_underline)
	{
		XFillRectangle(x_display, xw->ta.win, xw->gc,
			(int)(xw->ta.cursx * xw->ta.cellw), (int)((xw->ta.cursy + 1) * xw->ta.cellh - 1),
			len * xw->ta.cellw, 1);
	}
	if ((font == 'i' || font == 'I') && !x_defaultitalic)
	{
		XCopyArea(x_display, xw->ta.win, xw->ta.win, xw->gc,
			(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
			len * xw->ta.cellw - 1, (xw->ta.cellh + 1) / 2,
			(int)(xw->ta.cursx * xw->ta.cellw + 1), (int)(xw->ta.cursy * xw->ta.cellh));
	}

	/* free the temp string (if any) */
	if (tmp)
	{
		safefree(tmp);
	}

	/* leave the cursor after the text */
	xw->ta.cursx += len;
}

/* Insert "qty" characters into the current row, starting at
 * the current cursor position.  A negative "qty" value means
 * that characters should be deleted.
 *
 * This function is optional.  If omitted, elvis will rewrite
 * the text that would have been shifted.
 */
void x_ta_shift(xw, qty, rows)
	X11WIN	*xw;	/* window to be shifted */
	int	qty;	/* amount to shift by */
	int	rows;	/* number of rows affected */
{
	/* erase the cursor */
	x_ta_erasecursor(xw);

	/* make sure we have the right background */
	if (!xw->grexpose)
	{
		XSetGraphicsExposures(x_display, xw->gc, True);
		xw->grexpose = True;
	}
	if (xw->bg != xw->ta.bg)
	{
		XSetBackground(x_display, xw->gc, xw->ta.bg);
		xw->bg = xw->ta.bg;
	}

	if (qty > 0)
	{
		/* we'll be inserting */

		/* shift the characters */
		XCopyArea(x_display, xw->ta.win, xw->ta.win, xw->gc,
			(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
			xw->ta.cellw * (xw->ta.columns - xw->ta.cursx - qty), xw->ta.cellh * rows,
			(int)((xw->ta.cursx + qty) * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh));
	}
	else
	{
		/* we'll be deleting.  Convert qty to absolute value. */
		qty = -qty;

		/* shift the characters */
		XCopyArea(x_display, xw->ta.win, xw->ta.win, xw->gc,
			(int)((xw->ta.cursx + qty) * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
			xw->ta.cellw * (xw->ta.columns - xw->ta.cursx - qty), xw->ta.cellh * rows,
			(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh));
	}
}

void x_ta_scroll(xw, qty, notlast)
	X11WIN	*xw;	/* window to be scrolled */
	int	qty;	/* amount to scroll by (pos=downward, neg=upward) */
	BOOLEAN	notlast;/* if True, last row should not be affected */
{
	int	rows;

	/* erase the cursor */
	x_ta_erasecursor(xw);

	/* decide how many rows to scroll */
	rows = xw->ta.rows;
	if (notlast)
	{
		rows--;
	}

	/* make sure we have the right background */
	if (!xw->grexpose)
	{
		XSetGraphicsExposures(x_display, xw->gc, True);
		xw->grexpose = True;
	}
	if (xw->bg != xw->ta.bg)
	{
		XSetBackground(x_display, xw->gc, xw->ta.bg);
		xw->bg = xw->ta.bg;
	}

	if (qty > 0)
	{
		/* we'll be inserting */

		/* shift the rows */
		XCopyArea(x_display, xw->ta.win, xw->ta.win, xw->gc,
			0, (int)(xw->ta.cursy * xw->ta.cellh),
			xw->ta.cellw * xw->ta.columns, xw->ta.cellh * (rows - xw->ta.cursy - qty),
			0, (int)((xw->ta.cursy + qty) * xw->ta.cellh));
	}
	else
	{
		/* we'll be deleting.  Convert qty to absolute value. */
		qty = -qty;

		/* shift the rows */
		XCopyArea(x_display, xw->ta.win, xw->ta.win, xw->gc,
			0, (int)((xw->ta.cursy + qty) * xw->ta.cellh),
			xw->ta.cellw * xw->ta.columns, xw->ta.cellh * (rows - xw->ta.cursy - qty),
			0, (int)(xw->ta.cursy * xw->ta.cellh));
	}
}

void x_ta_clrtoeol(xw)
	X11WIN	*xw;	/* window whose row is to be cleared */
{
	/* make sure we have the right background */
	XSetForeground(x_display, xw->gc, xw->ta.bg);
	xw->fg = xw->ta.bg;
	if (xw->grexpose)
	{
		XSetGraphicsExposures(x_display, xw->gc, False);
		xw->grexpose = False;
	}

	/* whether or not the cursor was visible before, it'll be invisible
	 * after we erase the line.
	 */
	xw->ta.cursor = CURSOR_NONE;

	/* erase the line, from the cursor to the right edge */
	XFillRectangle(x_display, xw->ta.win, xw->gc,
		(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
		(xw->ta.columns - xw->ta.cursx) * xw->ta.cellw, xw->ta.cellh);
}


void x_ta_event(xw, event)
	X11WIN	*xw;	/* top-level window which received event */
	XEvent	*event;	/* the event */
{
	int	x, y, x2, y2;
 static int	prevx, prevy;
 static Time	firstclick;
 static BOOLEAN	marking;
 	long	offset;

	switch (event->type)
	{
	  case Expose:
		x = event->xexpose.x / xw->ta.cellw;
		y = event->xexpose.y / xw->ta.cellh;
		x2 = (event->xexpose.x + event->xexpose.width - 1) / xw->ta.cellw;
		y2 = (event->xexpose.y + event->xexpose.height - 1) / xw->ta.cellh;
		eventexpose((GUIWIN *)xw, y, x, y2, x2);
		x_ta_drawcursor(xw); /*!!!*/
		break;

	  case GraphicsExpose:
		x = event->xgraphicsexpose.x / xw->ta.cellw;
		y = event->xgraphicsexpose.y / xw->ta.cellh;
		x2 = (event->xgraphicsexpose.x + event->xgraphicsexpose.width - 1) / xw->ta.cellw;
		y2 = (event->xgraphicsexpose.y + event->xgraphicsexpose.height - 1) / xw->ta.cellh;
		eventexpose((GUIWIN *)xw, y, x, y2, x2);
		break;

	  case ButtonPress:
		/* determine which character cell was clicked on. */
		y = event->xbutton.y / xw->ta.cellh;
		x = event->xbutton.x / xw->ta.cellw;

		/* Distinguish between single-click and double-click */
		if (x_now - firstclick > o_dblclicktime * 100
			|| event->xbutton.button == Button2
			|| prevy != y
			|| prevx != x)
		{
			/* single-click */

			/* Buttons 1 & 2 cancel any pending selection, but
			 * button 3 does not.
			 */
			if (event->xbutton.button != Button3)
			{
				(void)eventclick((GUIWIN *)xw, -1, -1, CLICK_CANCEL);
			}

			/* Buttons 1 & 3 move the cursor to the clicked-on cell.
			 * NOTE that button 2's "paste from clipboard" operation
			 * is implemented in the ButtonRelease event, because we
			 * don't know whether a ButtonPress is a click or the
			 * start of a drag.
			 */
			if (event->xbutton.button != Button2)
			{
				if (firstclick != 1
				 || prevy != y
				 || prevx != x)
				{
					offset = eventclick((GUIWIN *)xw, y, x, CLICK_MOVE);
					x_didcmd |= (BOOLEAN)(offset >= 0);
				}
				firstclick = x_now;
			}

			/* Button 3 also copies the selected text (if any) into the
			 * clipboard.
			 */
			if (event->xbutton.button == Button3)
			{
				(void)eventclick((GUIWIN *)xw, -1, -1, CLICK_YANK);
			}

			/* Remember some info about this event, to help us
			 * detect drags and double-clicks.
			 */
			prevy = y;
			prevx = x;
			x_didcmd = True;
			/* paging = False; !!! */
		}
		else
		{
			/* double-click */

			/* Perform either a ^] or ^T tag command.  Note that
			 * for ^], the cursor is already located at the
			 * clicked-on character cell, because the first click
			 * moved it there.
			 */
			(void)eventclick((GUIWIN *)xw, -1, -1,
				event->xbutton.button == Button1 ? CLICK_TAG : CLICK_UNTAG);

			/* There is no such thing as a triple-click.  Clobber
			 * the "firstclick" variable so we're sure to think
			 * the next click is a first click.
			 */
			firstclick = 1;
			x_didcmd = True;
			/* paging = False; !!! */
		}
		break;

	  case MotionNotify:
		/* convert to character cell coordinates */
		y = event->xbutton.y / xw->ta.cellh;
		x = event->xbutton.x / xw->ta.cellw;

		/* if not the same cell as last time... */
		if (y != prevy || x != prevx)
		{
			offset = eventclick((GUIWIN *)xw, y, x, CLICK_MOVE);
			if (offset >= 0)
			{
				/* If moved off original character, start marking */
				if (!marking &&
					0 <= eventclick((GUIWIN *)xw, prevy, prevx,
					  (event->xmotion.state & Button2Mask) ? CLICK_SELRECT :
					  (event->xmotion.state & Button3Mask) ? CLICK_SELLINE :
					  CLICK_SELCHAR))
				{
					(void)eventclick((GUIWIN *)xw, y, x, CLICK_MOVE);
					marking = True;
				}
				prevy = y;
				prevx = x;
				x_didcmd = True;
			}
		}
		break;

	  case ButtonRelease:
		if (marking)
		{
			y = event->xbutton.y / xw->ta.cellh;
			x = event->xbutton.x / xw->ta.cellw;
			if (y != prevy || x != prevx)
				(void)eventclick((GUIWIN *)xw, y, x, CLICK_MOVE);
			eventclick((GUIWIN *)xw, y, x, CLICK_YANK);
		}
		else /* if (!paging)*/
		{
			/* end of a click - depends on button */
			if (event->xbutton.state & Button2Mask)
			{
				/* paste from "< buffer */
				eventclick((GUIWIN *)xw, -1, -1, CLICK_PASTE);
				x_didcmd = True;
			}
		}

		/* thumbing = False; paging = True; */
		marking = False;
		break;
	}
}


void x_ta_recolor(xw, font)
	X11WIN	*xw;	/* window to be changed */
	_char_	font;	/* font which was changed */
{
	/* Reload the foregroung color for the indicated font.  Also, the cursor
	 * is handled specially, and other non-font colors are detected here.
	 */
	switch (font)
	{
	  case 'n':
		x_unloadcolor(xw->ta.fgnormal);
		xw->ta.fgnormal = x_loadcolor(x_foreground, x_black);
		break;

	  case 'b':
		x_unloadcolor(xw->ta.fgbold);
		xw->ta.fgbold = x_loadcolor(x_boldcolor, x_black);
		break;

	  case 'i':
		x_unloadcolor(xw->ta.fgitalic);
		xw->ta.fgitalic = x_loadcolor(x_italiccolor, x_black);
		break;

	  case 'u':
		x_unloadcolor(xw->ta.fgundln);
		xw->ta.fgundln = x_loadcolor(x_underlinecolor, x_black);
		break;

	  case 'f':
		x_unloadcolor(xw->ta.fgfixed);
		xw->ta.fgfixed = x_loadcolor(x_fixedcolor, x_black);
		break;

	  case 'e':
		x_unloadcolor(xw->ta.fgemph);
		xw->ta.fgemph = x_loadcolor(x_emphcolor, x_black);
		break;

	  case 'c':
		x_unloadcolor(xw->ta.fgcursor);
		x_unloadcolor(xw->ta.owncursor);
		xw->ta.fgcursor = x_loadcolor(x_cursorcolor, x_black);
		xw->ta.owncursor = x_loadcolor(x_owncolor, x_black);
		/* next cursor action will cause a redraw */
		return;

	  default:
		/* nothing changed, so don't bother to refresh */
		return;
	}

	/* also change the overall background color, defaulting to white */
	x_unloadcolor(xw->ta.bg);
	xw->ta.bg = x_loadcolor(x_background, x_white);
	XSetWindowBackground(x_display, xw->ta.win, xw->ta.bg);

	/* redraw the window */
	eventexpose((GUIWIN *)xw, 0, 0, (int)xw->ta.rows-1, (int)xw->ta.columns-1);
}
#endif
