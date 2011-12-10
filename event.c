/* event.c */
/* Copyright 1995 by Steve Kirkendall */

char id_event[] = "$Id: event.c,v 2.38 1996/09/21 01:21:36 steve Exp $";

#include "elvis.h"

#ifndef DEBUG_EVENT
# define WATCH(x)
#else
# define WATCH(x)	x
#endif

#ifdef NDEBUG
# define USUAL_SUSPECTS
#else
# define USUAL_SUSPECTS {WINDOW w; for (w = windows; w; w = w->next){ \
	if (!w->state) continue; \
	assert(markbuffer(w->state->top) == markbuffer(w->state->cursor)); \
	assert(markbuffer(w->state->bottom) == markbuffer(w->state->cursor)); \
	assert(markoffset(w->state->top) <= markoffset(w->state->cursor)); \
	assert(markoffset(w->state->bottom) >= markoffset(w->state->cursor)); \
    } }
#endif

#ifdef USE_PROTOTYPES
static void setcursor(WINDOW win, long offset, BOOLEAN clean);
#endif

/* This counts the number of events that have been processed.  In particular,
 * if eventcounter==0 then we know we're still performing initialization.
 */
long eventcounter;

/* Some events move the cursor. E.g., clicking the mouse or moving the
 * scrollbar's thumb.  If the window is in input mode, then there may be
 * some superfluous text between the cursor and the end of the edit region
 * (usually marked with a '$') which needs to be deleted before the cursor
 * can be moved.
 *
 * This function detects those situations, and performs the necessary steps.
 * The eventXXX functions should always use this function to set the cursor
 * position.
 */
static void setcursor(win, offset, clean)
	WINDOW	win;	/* window whose cursor should be moved */
	long	offset;	/* new offset for the cursor */
	BOOLEAN	clean;	/* definitely delete superfluous text */
{
	/* if in input mode, and there are superfluous characters after the
	 * cursor (probably the end is marked with a '$' character) then we
	 * may need to delete those characters.  Exception: If the new
	 * location is still in the same edit region, then we don't need to
	 * worry about it.
	 */
	if ((win->state->flags & ELVIS_REGION) == ELVIS_REGION
	 && markoffset(win->state->bottom) != markoffset(win->state->cursor)
	 && (clean
	  || offset < markoffset(win->state->top)
	  || markoffset(win->state->bottom) <= offset))
	{
		/* tweak offset to compensate for the following bufreplace() */
		if (offset > markoffset(win->state->bottom))
			offset -= (markoffset(win->state->bottom) - markoffset(win->state->cursor));

		/* delete the superfluous text */
		bufreplace(win->state->cursor, win->state->bottom, NULL, 0L);
	}
	
	/* move the cursor, and maybe adjust the edit region too */
	marksetoffset(win->state->cursor, offset);
	if (offset < markoffset(win->state->top)
		 || markoffset(win->state->bottom) <= offset)
	{
		marksetoffset(win->state->top, offset);
		marksetoffset(win->state->bottom, offset);
	}
}

/* This function creates a WINDOW -- an internal data structure
 * used to describe the characteristics of a window.  The GUI
 * calls this function when an elvis window suddenly pops into
 * existence.  (Note: Each GUI has a creategw() function to request
 * that a new window be created.)
 *
 * The GUIWIN already exists, and the BUFFER named "name" must also
 * exist.  If anything goes wrong, this function issues an error message
 * via msg(), and returns False; the GUI should then destroy its window and
 * forget about it.  Returns True for successful creations.
 */
BOOLEAN eventcreate(gw, guivals, name, rows, columns)
	GUIWIN	*gw;		/* GUI's handle for new window */
	OPTVAL	*guivals;	/* values of GUI's window-dependent options */
	char	*name;		/* name of the new window */
	int	rows;		/* height of the new window */
	int	columns;	/* width of the new window */
{
	BUFFER	buf;
	WINDOW	win;

WATCH(fprintf(stderr, "eventcreate(..., name=\"%s\", rows=%d, columns=%d)\n", name, rows, columns));
	USUAL_SUSPECTS
	eventcounter++;

	/* find/create a buffer with the requested name */
	buf = buffind(toCHAR(name));
	if (!buf)
	{
		buf = bufload((CHAR *)0, name, False);
	}

	/* create a WINDOW */
	win = winalloc(gw, guivals, buf, rows, columns);

	USUAL_SUSPECTS
	return (BOOLEAN)(win != NULL);
}

/* This function frees the WINDOW associated with gw, and frees it. */
void eventdestroy(gw)
	GUIWIN	*gw;	/* GUI's handle for a window which has been deleted */
{
	WINDOW	win;
	BUFFER	msgq;

WATCH(fprintf(stderr, "eventdestroy(...)\n"));
	USUAL_SUSPECTS
	eventcounter++;

	/* find the window */
	win = winofgw(gw);
	assert(win);

	/* free it */
	winfree(win, False);

	/* if there are any messages pending, and we aren't about to exit,
	 * then flush the messages out to some other window.
	 */
	msgq = buffind(toCHAR(MSGQUEUE_BUF));
	if (msgq && o_bufchars(msgq) > 0L && winofbuf(NULL, NULL) != NULL)
	{
		win = winofbuf(NULL, NULL);
		winoptions(win);
		msgflush();
	}
	USUAL_SUSPECTS
}

/* Change the size of an existing window.  This doesn't automatically redraw
 * the window; later expose events or eventdraw() calls will take care of that.
 */
void eventresize(gw, rows, columns)
	GUIWIN	*gw;	/* GUI's handle for the window that was resized */
	int	rows;	/* new height of the window */
	int	columns;/* new width of the window */
{
	WINDOW	win;

	USUAL_SUSPECTS
WATCH(fprintf(stderr, "eventresize(..., rows=%d, columns=%d\n", rows, columns));
	eventcounter++;

	/* find the window */
	win = winofgw(gw);
	assert(win);

	/* resize it */
	winresize(win, rows, columns);

	/* announce the new size */
	msg(MSG_INFO, "[dd]$1 rows, $2 columns", (long)rows, (long)columns);
	USUAL_SUSPECTS
}

/* Replace the current buffer with some other buffer, for the WINDOW
 * associated with "gw", and then maybe free the old version.  If
 * anything goes wrong, then issue an error message instead.
 */
void eventreplace(gw, freeold, name)
	GUIWIN	*gw;	/* GUI's handle for window to be switched */
	BOOLEAN	freeold;/* if True, destroy old buffer; else retain it */
	char	*name;	/* name of new buffer to use */
{
	WINDOW	win;
	BUFFER	buf;

WATCH(fprintf(stderr, "eventreplace(..., freeold=%s, name=\"%s\")\n", freeold?"True":"False", name));
	USUAL_SUSPECTS
	eventcounter++;

	/* find/create a buffer with the requested name */
	buf = buffind(toCHAR(name));
	if (!buf)
	{
		buf = bufload((CHAR *)0, name, False);
	}

	/* find the window */
	win = winofgw(gw);
	assert(win);

	/* replace its buffer */
	winchgbuf(win, buf, True);
	USUAL_SUSPECTS
}

/* Redraw a portion of a window. */
void eventexpose(gw, top, left, bottom, right)
	GUIWIN	*gw;	/* GUI's handle for window which has been exposed */
	int	top;	/* top edge of exposed rectangle */
	int	left;	/* left edge of exposed rectangle */
	int	bottom;	/* bottom edge of exposed rectangle */
	int	right;	/* right edge of exposed rectangle */
{
	WINDOW	win = winofgw(gw);

WATCH(fprintf(stderr, "eventexpose(..., top=%d, left=%d, bottom=%d, right=%d)\n", top, left, bottom, right));
	USUAL_SUSPECTS
	eventcounter++;

	/* permit the bell to ring */
	guibeep(NULL);

	if (top < 0) top = 0;
	if (left < 0) left = 0;
	if (bottom >= o_lines(win)) bottom = o_lines(win) - 1;
	if (right >= o_columns(win)) right = o_columns(win) - 1;
	drawexpose(win, top, left, bottom, right);
	USUAL_SUSPECTS
}

/* Makes sure that a window shows a current image of its buffer's
 * text.  The GUI should only call this function when will have to
 * *wait* for the next event.  It returns the cursor shape.
 */
ELVCURSOR eventdraw(gw)
	GUIWIN	*gw;	/* GUI's handle for window to be updated */
{
	WINDOW	win = winofgw(gw);

WATCH(fprintf(stderr, "eventdraw(...)\n"));
	USUAL_SUSPECTS
	assert(win && win->state);
	eventcounter++;

	/* flush any messages */
	msgflush();

	/* permit the bell to ring */
	guibeep(NULL);

	/* call drawopenedit for open-mode windows, or drawimage for visual-mode */
	if (win->state->flags & ELVIS_BOTTOM)
	{
		/* line-at-a-time */
		drawopenedit(win);
	}
	else if (win->di->drawstate == DRAW_OPENOUTPUT)
	{
		/* push a "more" key state */
		morepush(win, win->state->morekey);

		/* display the message */
		drawopenedit(win);
	}
	else
	{
		/* draw the screen */
		drawimage(win);
	}
	USUAL_SUSPECTS
	return (win->state ? (*win->state->shape)(win) : CURSOR_NONE);
}

/* Make the WINDOW and BUFFER associated with "gw" be the defaults.  Returns
 * the cursor shape.  Since the the eventkeys() function does this itself,
 * the only forseeable use of this function is to return the current cursor
 * shape.
 */
ELVCURSOR eventfocus(gw)
	GUIWIN	*gw;	/* GUI's handle for window to become new default */
{
	WINDOW	win = winofgw(gw);

WATCH(fprintf(stderr, "eventfocus(...)\n"));
	USUAL_SUSPECTS
	assert(win && win->state);
	eventcounter++;

	winoptions(win);
	USUAL_SUSPECTS
	return (*win->state->shape)(win);
}

/* Convert a screen point (where (0,0) is the upper-left character cell)
 * to a buffer offset, and move the cursor there.  Optionally start visible
 * marking, depending on the value of "what".
 *
 * This function can also cancel visible marking, when "what" is CLICK_CANCEL.
 * In this case, the "row" and "column" parameters are ignored and the cursor
 * does not move.
 */
long eventclick(gw, row, column, what)
	GUIWIN	*gw;	/* GUI's handle for window that was clicked */
	int	row;	/* row where clicked */
	int	column;	/* column where clicked */
	CLICK	what;	/* action that the click should perform */
{
	WINDOW	win = winofgw(gw);
	VIINFO	vinfbuf;
	MARKBUF	tmp;
	MARK	newcurs;
	long	offset;

WATCH(fprintf(stderr, "eventclick(..., row=%d, column=%d,...)\n", row, column));
	USUAL_SUSPECTS
	eventcounter++;

	/* reset the poll frequency counter */
	guipoll(True);
	bufmsgtype = MSG_STATUS;

	/* make this window be the current window */
	winoptions(win);

	/* if the window is showing "Hit <Enter> to continue" then any click
	 * is treated as an <Enter> keystoke.
	 */
	if (!strcmp(win->state->modename, "More"))
	{
		(void)eventkeys(gw, toCHAR("\n"), 1);
		return markoffset(win->cursor);
	}

	/* for some operations, the position doesn't matter */
	switch (what)
	{
	  case CLICK_CANCEL:
		vinfbuf.command = ELVCTRL('[');
		(void)v_visible(win, &vinfbuf);
		USUAL_SUSPECTS
		return 0;

	  case CLICK_YANK:
		/* if no text is marked, then fail */
		if (!win->seltop)
		{
			USUAL_SUSPECTS
			return -1;
		}

		/* Do the yank */
		switch (win->seltype)
		{
		  case 'c':
			/* for character yanks, we need to tweak the "to" value
			 * so the last character is included.
			 */
			tmp = *win->selbottom;
			markaddoffset(&tmp, 1);
			cutyank((_CHAR_)'>', win->seltop, &tmp, win->seltype, False);
			break;

		  case 'l':
			/* for line yanks, we want to avoid readjusting the
			 * endpoints, so we'll pass 'L' as the type.
			 */
			cutyank((_CHAR_)'>', win->seltop, win->selbottom, 'L', False);
			break;

		  case 'r':
			tmp = *win->selbottom;
			markaddoffset(&tmp, 1);
			cutyank((_CHAR_)'>', win->seltop, &tmp, win->seltype, False);
			break;
		}
		USUAL_SUSPECTS
		return 0;

	  case CLICK_PASTE:
		/* end any pending selection */
		vinfbuf.command = ELVCTRL('[');
		(void)v_visible(win, &vinfbuf);

#if 1
		/* set the buffer's "willdo" flag so this paste is undoable */
		bufwilldo(win->state->cursor);

		/* paste the text */
		newcurs = cutput((_CHAR_)'<', win, win->state->cursor, False, True, True);
		if (newcurs)
		{
			setcursor(win, markoffset(newcurs) + 1, True);
		}
#else
		vinfbuf.command = '@';
		vinfbuf.key2 = '<';
		(void)v_at(win, &vinfbuf);
#endif
		USUAL_SUSPECTS
		return 0;

	  case CLICK_TAG:
		/* simulate a <Control-]> keystroke */
		vinfbuf.command = ELVCTRL(']');
		tmp = *win->state->cursor;
		(void)v_tag(win, &vinfbuf);
#if 0
		offset = markoffset(win->state->cursor);
		marksetoffset(win->state->cursor, markoffset(&tmp));
		setcursor(win, offset, True);
#endif
		USUAL_SUSPECTS
		return 0;
		
	  case CLICK_UNTAG:
	  	/* simulate a <Control-T> keystroke */
	  	vinfbuf.command = ELVCTRL('T');
		tmp = *win->state->cursor;
		(void)v_tag(win, &vinfbuf);
#if 0
		offset = markoffset(win->state->cursor);
		marksetoffset(win->state->cursor, markoffset(&tmp));
		setcursor(win, offset, True);
#endif
		USUAL_SUSPECTS
	  	return 0;

	  default:
		/* handled below... */
		;
	}

	/* bad positions are always -1.  If the screen is in ex mode, all
	 * positions are bad.
	 */
	if (win->di->drawstate == DRAW_OPENEDIT
	 || win->di->drawstate == DRAW_OPENOUTPUT
	 || row < 0 || row >= o_lines(win) - 1
	 || column < 0 || column >= o_columns(win))
	{
		USUAL_SUSPECTS
		return -1;
	}

	/* else look it up */
	offset =  win->di->offsets[row * o_columns(win) + column];
	if (offset < 0 || offset >= o_bufchars(markbuffer(win->state->cursor)))
	{
		USUAL_SUSPECTS
		return -1;
	}

	/* move the cursor to the click-on cell */
	win->wantcol = column;
	setcursor(win, offset, False);

	/* perform the requested operation */
	switch (what)
	{
	  case CLICK_MOVE:
		/* If a selection is in progress, then adjust one end of the
		 * selection to match the cursor position.
		 */
		if (win->seltop)
		{
			(void)v_visible(win, NULL);
		}
		break;

	  case CLICK_SELCHAR:
		/* start character selection */
		vinfbuf.command = ELVCTRL('[');
		(void)v_visible(win, &vinfbuf);
		vinfbuf.command = 'v';
		(void)v_visible(win, &vinfbuf);
		break;

	  case CLICK_SELLINE:
		/* start line selection */
		vinfbuf.command = ELVCTRL('[');
		(void)v_visible(win, &vinfbuf);
		vinfbuf.command = 'V';
		(void)v_visible(win, &vinfbuf);
		break;

	  case CLICK_SELRECT:
		/* start rectangle selection */
		vinfbuf.command = ELVCTRL('[');
		(void)v_visible(win, &vinfbuf);
		vinfbuf.command = ELVCTRL('V');
		(void)v_visible(win, &vinfbuf);
		break;

	  case CLICK_CANCEL:
	  case CLICK_NONE:
	  case CLICK_YANK:
	  case CLICK_PASTE:
	  case CLICK_TAG:
	  case CLICK_UNTAG:
		/*NOTREACHED*/
		;
	}
	USUAL_SUSPECTS
	return offset;
}

/* This function makes the window and buffer associated with "gw"
 * be the current window and buffer, and then interprets keystrokes.
 * Keystroke interpretation involves mapping, states, commands, and
 * all that stuff.
 */
MAPSTATE eventkeys(gw, key, nkeys)
	GUIWIN	*gw;	/* GUI's handle for window that received keypress event */
	CHAR	*key;	/* array of ASCII characters from key */
	int	nkeys;	/* number of ASCII characters */
{
	MAPSTATE mapstate;
WATCH(fprintf(stderr, "eventkeys(..., key={%d, ...}, nkeys=%d)\n", key[0], nkeys));
	USUAL_SUSPECTS
	eventcounter++;

	/* reset the poll frequency counter */
	guipoll(True);
	bufmsgtype = MSG_STATUS;

	/* Make this the default window, if it isn't already */
	winoptions(winofgw(gw));

	/* send the keys through the mapper */
	mapstate = mapdo(key, nkeys);
	USUAL_SUSPECTS
	return mapstate;
}


/* This scrolls the screen, and returns True if successful.  The screen's
 * image won't be adjusted to reflect this, though, until the next eventdraw().
 * This function is intended mostly to be used fpr processing mouse clicks on
 * the scrollbar.  It may also be useful for mouse draw-through operations
 * in which the mouse is dragged off the edge of the screen.
 */
BOOLEAN eventscroll(gw, scroll, count, denom)
	GUIWIN	*gw;	/* GUI's handle for window to be scrolled */
	SCROLL	scroll;	/* type of scrolling to perform */
	long	count;	/* amount to scroll */
	long	denom;	/* scrollbar height, for moving the "thumb" */
{
	WINDOW	win = winofgw(gw);
	VIINFO	cmd;
	RESULT	result;
	long	origoffset;
	long	newoffset;

	USUAL_SUSPECTS
	assert(win != NULL && (count > 0 || (count == 0 && scroll == SCROLL_PERCENT)));
	eventcounter++;

	/* reset the poll frequency counter */
	guipoll(True);

	/* if window is in open mode, this fails */
	if (win->state->flags & ELVIS_BOTTOM)
	{
		msg(MSG_WARNING, "not while in open mode");
		USUAL_SUSPECTS
		return False;
	}

	/* remember the cursor's original offset */
	origoffset = markoffset(win->state->cursor);

	/* build & execute a vi command */
	cmd.count = count;
	switch (scroll)
	{
	  case SCROLL_BACKSCR:
		cmd.command = ELVCTRL('B');
		result = m_scroll(win, &cmd);
		break;

	  case SCROLL_FWDSCR:
		cmd.command = ELVCTRL('F');
		result = m_scroll(win, &cmd);
		break;

	  case SCROLL_FWDLN:
		cmd.command = ELVCTRL('E');
		result = m_scroll(win, &cmd);
		break;

	  case SCROLL_BACKLN:
		cmd.command = ELVCTRL('Y');
		result = m_scroll(win, &cmd);
		break;

	  case SCROLL_COLUMN:
		cmd.command = ELVCTRL('|');
		result = m_column(win, &cmd);
		break;

	  case SCROLL_PERCENT:
		if (count > 0)
		{
			if (win->md->move == dmnormal.move && denom > 0)
			{
				cmd.command = 'G';
				cmd.count = o_buflines(markbuffer(win->cursor)) * count / denom;
				if (cmd.count == 0)
					cmd.count = 1;
				result = m_absolute(win, &cmd);
			}
			else
			{
				if (o_bufchars(markbuffer(win->cursor)) != 0)
					marksetoffset(win->cursor, (o_bufchars(markbuffer(win->cursor)) - 1) * count / denom);
				result = RESULT_COMPLETE;
			}
			if (result == RESULT_COMPLETE)
			{
				cmd.count = cmd.count2 = 0L;
				cmd.command = 'z';
				cmd.key2 = '+';
				result = m_z(win, &cmd);
				if (result == RESULT_COMPLETE)
				{
					cmd.count = win->wantcol + 1;
					cmd.command = ELVCTRL('|');
					result = m_column(win, &cmd);
				}
			}
		}
		else
		{
			cmd.command = ELVCTRL('G');
			cmd.count = 1L;
			result = m_absolute(win, &cmd);
		}
		break;

	  case SCROLL_LINE:
		cmd.command = 'G';
		result = m_absolute(win, &cmd);
		break;
	}

	/* Now we need to get clever about moving the cursor, due to the
	 * possibility of superfluous text after the old cursor position
	 * if we happened to be in input mode.  Force the cursor back to
	 * its old offset, and then use setcursor() to move it to its new
	 * offset in an input-mode-sensitive way.
	 */
	newoffset = markoffset(win->state->cursor);
	marksetoffset(win->state->cursor, origoffset);
	setcursor(win, newoffset, True);

	USUAL_SUSPECTS
	return False;
}


/* This function is called by the GUI when the user wants to suspend elvis */
void eventsuspend()
{
	BUFFER	buf;
	WINDOW	win;
	eventcounter++;

	USUAL_SUSPECTS

	/* if autowrite is False, then do nothing */
	if (!o_autowrite)
		return;

	/* If there is superfluous text after any cursor, delete it */
	for (win = winofbuf(NULL, NULL); win; win = winofbuf(win, NULL))
	{
		setcursor(win, markoffset(win->state->cursor), True);
	}

	/* save all user buffers */
	for (buf = buffers; buf; buf = buf->next)
	{
		if (!o_internal(buf))
			bufsave(buf, False, False);
	}

	USUAL_SUSPECTS
}

/* This function is called to execute ex command lines.  This may be a result
 * of selecting a menu item, clicking a button, or something else.  If the
 * command comes from an unreliable source (outside the process) then it should
 * be executed with the "safer" flag set, for security.
 */
void eventex(gw, cmd, safer)
	GUIWIN	*gw;	/* window where command should be run */
	char	*cmd;	/* an ex command to execute */
	BOOLEAN	safer;	/* temporarily set the o_safer option? */
{
	BOOLEAN	origsafer;

	USUAL_SUSPECTS

	/* reset the poll frequency counter */
	guipoll(True);
	bufmsgtype = MSG_STATUS;

	/* temporarily set the "safer" option appropriately */
	origsafer = o_safer;
	o_safer |= safer;

	/* Make this the default thing, if it isn't already */
	winoptions(winofgw(gw));

	/* If there is superfluous text after the cursor, delete it */
	setcursor(windefault, markoffset(windefault->state->cursor), True);

	/* execute the command */
	exstring(windefault, toCHAR(cmd));

	/* update the state as though a key had just been pressed */
	statekey((_CHAR_)-1);

	/* restore the "safer" option */
	o_safer = origsafer;

	USUAL_SUSPECTS
}
