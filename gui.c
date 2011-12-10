/* gui.c */
/* Copyright 1995 by Steve Kirkendall */

char id_gui[] = "$Id: gui.c,v 2.12 1996/09/18 20:38:17 steve Exp $";

#include "elvis.h"

/* This is a pointer to the chosen GUI. */
GUI *gui;

/* This function calls the GUI's moveto() function.  This function performs a
 * tiny amount of optimization, however: if the cursor is already in the
 * correct position, it does nothing.
 */
void guimoveto(win, column, row)
	WINDOW	win;	/* window whose cursor is to be moved */
	int	column;	/* column where cursor should be placed */
	int	row;	/* row where cursor should be placed */
{
	/* perform the moveto, unless the cursor is already there */
#if 0
	if (column != win->cursx || row != win->cursy)
#endif
	{
		(*gui->moveto)(win->gw, column, row);
		win->cursx = column;
		win->cursy = row;
	}
}

/* This function calls the GUI's draw() function, and then updates elvis'
 * own idea of where the cursor is.  The guimove() function depends on your
 * calling guidraw() instead of (*gui->draw)().
 */
void guidraw(win, font, text, len)
	WINDOW	win;	/* window where text is to be drawn */
	_char_	font;	/* font to use for drawing */
	CHAR	*text;	/* text to be drawn */
	int	len;	/* number of characters in text */
{
	(*gui->draw)(win->gw, font, text, len);
	win->cursx += len;
}

/* This function calls the GUI's scroll() function, but only if the number of
 * lines to be deleted/inserted is smaller than the number of lines remaining.
 * And only if the GUI has a scroll() function, of course.
 *
 * Returns True if the scrolling happened as requested, else False.
 */
BOOLEAN guiscroll(win, qty, notlast)
	WINDOW	win;	/* window to be scrolled */
	int	qty;	/* rows to insert (may be negative to delete) */
	BOOLEAN	notlast;/* if True, scrolling shouldn't affect last row */
{
	/* if there is no gui->scroll() function, or if we're trying to
	 * insert/delete too many rows, then fail.
	 */
	if (gui->scroll == NULL || abs(qty) >= o_lines(win) - win->cursy)
	{
		return False;
	}

	/* else give the GUI a try */
	return (*gui->scroll)(win->gw, qty, notlast);
}

/* This function calls the GUI's shift() function, but only if the number of
 * characters to the right of the cursor is larger than the requested shift
 * amount.  And only if the GUI has a shift() function, of course.
 *
 * Returns True if the shifting happened as requested, else False.
 */
BOOLEAN guishift(win, qty, rows)
	WINDOW	win;	/* window to be shifted */
	int	qty;	/* columns to insert (may be negative to delete) */
	int	rows;	/* number of rows affected */
{
	/* if there is no gui->shift() function, or if we're trying to
	 * insert/delete too many characters, then fail.
	 */
	if (!gui->shift || abs(qty) >= o_columns(win) - win->cursx)
	{
		return False;
	}

	/* else give the GUI a try */
	return (*gui->shift)(win->gw, qty, rows);
}

/* This function calls the GUI's cltroeol() function.  If it doesn't exist,
 * or returns False, then this function writes enough space characters to
 * simulate a clrtoeol()
 */
void guiclrtoeol(win)
	WINDOW	win;	/* window whose row is to be cleared */
{
	static CHAR	blanks[10] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
	int		width;

	/* if already at EOL, we're done */
	width = o_columns(win) - win->cursx;
	if (width <= 0)
	{
		return;
	}

	/* try to make the GUI do it */
	if (gui->clrtoeol == NULL || !(*gui->clrtoeol)(win->gw))
	{
		/* No, we need to do it the hard way */
		(*gui->moveto)(win->gw, win->cursx, win->cursy);
		while (width > 10)
		{
			(*gui->draw)(win->gw, 'n', blanks, 10);
			width -= 10;
		}
		(*gui->draw)(win->gw, 'n', blanks, width);
		(*gui->moveto)(win->gw, win->cursx, win->cursy);
	}
}


/* This function calls the GUI's reset function (if it has one) and also
 * resets the portable GUI wrapper functions' variables.
 *
 * Why do this?  Because the wrapper functions, and some GUI drawing functions,
 * perform some internal optimizations by assuming that nothing else affects
 * the screen when we aren't looking; so we ^L is going to force the whole
 * screen to be redrawn, that assumption is unsafe.
 */
void guireset()
{
	WINDOW	w;

	/* if the GUI has a reset function call it */
	if (gui->reset)
	{
		(*gui->reset)();
	}

	/* reset the wrapper functions' variables */
	for (w = windows; w; w = w->next)
	{
		w->cursx = w->cursy = -1;
	}
}


/* This function calls the GUI's poll() function.  If it has no poll() function
 * then this function always returns False to indicate that the current work
 * should continue.  This function is also sensitive to the pollfrequency
 * option, to reduce the number of calls to poll() since poll() may be slow.
 *
 * Returns False if the current work should continue, or True if the user
 * has requested that it be cut short.
 */
BOOLEAN guipoll(reset)
	BOOLEAN	reset;	/* reset the pollctr variable? */
{
	static long	pollctr = 0;

	/* if just supposed to reset, then do that and then quit */
	if (reset)
	{
		pollctr = 0;
		if (gui && gui->poll)
			return (*gui->poll)(reset);
		else
			return False;
	}

	/* if there is no poll() function, or pollfrequency indicates that
	 * poll() shouldn't be called yet, then return False so the current
	 * operation will continue.
	 */
	if (!gui || !gui->poll || ++pollctr < o_pollfrequency)
	{
		return False;
	}

	/* reset the pollctr variable */
	pollctr = 0;

	/* call the GUI's poll() function, and return its value */
	reset = (*gui->poll)(reset);
	if (reset)
	{
		msg(MSG_ERROR, "aborted");
	}
	return reset;
}

/* ring the bell, if there is one.  Limit it to one ding per eventdraw() */
void guibeep(win)
	WINDOW	win;	/* window to ding, or NULL to indicate an eventdraw() */
{
	static BOOLEAN	dingable = True;

	if (!win)
	{
		dingable = True;
	}
	else if (gui && gui->beep && dingable)
	{
		(*gui->beep)(win->gw);
		dingable = False;
	}
}
