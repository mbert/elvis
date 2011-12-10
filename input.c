/* input.c */
/* Copyright 1995 by Steve Kirkendall */

char id_input[] = "$Id: input.c,v 2.46 1996/09/21 05:26:35 steve Exp $";

#include "elvis.h"

/* These data types are used to store the parsing state for input mode
 * commands.  This is very simple, since most commands are only one keystroke
 * long. (The only exceptions are that INP_QUOTE is two keystrokes long, and
 * INP_HEX1/INP_HEX2 is three keystrokes long.)
 */
typedef enum {
	INP_NORMAL,	/* a normal character to insert/replace */
	INP_NEWLINE,	/* insert a newline */
	INP_QUOTE,	/* we're in the middle of a ^V sequence */
	INP_HEX1,	/* we're expecting the first of two hex digits */
	INP_HEX2,	/* we're expecting the second of two hex digits */
	INP_DIG1,	/* we're expecting the first of two digraph chars */
	INP_DIG2,	/* we're expecting the second of two digraph chars */
	INP_TAB,	/* ^I - insert a bunch of spaces to look like a tab */
	INP_ONEVI,	/* ^O - execute one vi command */
	INP_MULTIVI,	/* ESC - execute many vi commands */
	INP_BACKSPACE,	/* ^H - backspace one character */
	INP_BACKWORD,	/* ^W - backspace one word */
	INP_BACKLINE,	/* ^U - backspace one line */
	INP_SHIFTL,	/* ^D - reduce indentation */
	INP_SHIFTR,	/* ^T - increase indentation */
	INP_EXPOSE,	/* ^R/^L - redraw the screen */
	INP_PREVIOUS,	/* ^@ - insert a copy of previous text, then exit */
	INP_AGAIN,	/* ^A - insert a copy of previous text, continue */
	INP_PUT		/* ^P - insert a copy of anonymous cut buffer */
} INPCMD;
typedef struct
{
	BOOLEAN	setbottom;	/* Set bottom = cursor before drawing cursor? */
	BOOLEAN	replacing;	/* True if we're in "replace" mode */
	INPCMD	cmd;		/* the command to perform */
	CHAR	arg;		/* argument -- usually a key to insert */
	CHAR	backsp;		/* char backspaced over, or '\0' */
	CHAR	prev;		/* previously input char, or '\0' */
} INPUTINFO;


#if USE_PROTOTYPES
static void cleanup(WINDOW win, BOOLEAN del, BOOLEAN backsp, BOOLEAN yank);
static void addchar(MARK cursor, MARK top, MARK bottom, INPUTINFO *info);
static BOOLEAN tryabbr(WINDOW win, _CHAR_ nextc);
static RESULT perform(WINDOW win);
static RESULT parse(_CHAR_ key, void *info);
static ELVCURSOR shape(WINDOW win);
#endif


/* This function deletes everything between "cursor" and "bottom" of the
 * current state.  This is used, for example, when the user hits <Esc>
 * after using "cw" to change a long word into a short one.  It should be
 * called for the INP_ONEVI command with backsp=False, and before INP_MULTIVI
 * backup=True.
 */
static void cleanup(win, del, backsp, yank)
	WINDOW	win;	/* window where input took place */
	BOOLEAN	del;	/* if True, delete text after the cursor */
	BOOLEAN	backsp;	/* if True, move to the left */
	BOOLEAN	yank;	/* if True, yank input text into ". buffer */
{
	/* delete the excess in the edited region */
	if (del && markoffset(win->state->cursor) < markoffset(win->state->bottom))
	{
		bufreplace(win->state->cursor, win->state->bottom, NULL, 0);
	}
	else
	{
		marksetoffset(win->state->bottom, markoffset(win->state->cursor));
	}
	assert(markoffset(win->state->cursor) == markoffset(win->state->bottom));

	/* save the newly input text in the "previous input" buffer */
	if (yank)
	{
		cutyank('.', win->state->top, win->state->bottom, 'c', False);
	}

	/* move the cursor back one character, unless it is already at the
	 * start of a line.
	 */
	if (backsp && markoffset(win->state->cursor) > 0)
	{
		markaddoffset(win->state->cursor, -1);
		if (scanchar(win->state->cursor) == '\n')
		{
			markaddoffset(win->state->cursor, 1);
		}
		marksetoffset(win->state->top, markoffset(win->state->cursor));
		marksetoffset(win->state->bottom, markoffset(win->state->cursor));
	}

	/* Force the screen to be regenerated */
	if (win->di->logic == DRAW_NORMAL)
		win->di->logic = DRAW_CHANGED;
}

/* This function inserts/replaces a single character in a buffer, and
 * advances the cursor and (if necessary) bottom mark.
 */
static void addchar(cursor, top, bottom, info)
	MARK		cursor;	/* where to add a character */
	MARK		top;	/* start of edit bounds */
	MARK		bottom;	/* end of edit bounds */
	INPUTINFO	*info;	/* other info, including char to be inserted */
{
	MARKBUF	replaced;

	/* decide whether to insert or replace */
	replaced = *cursor;
	if (markoffset(cursor) < markoffset(bottom))
	{
		replaced.offset++;
	}
	else if (info->replacing && markoffset(cursor) < o_bufchars(markbuffer(cursor)))
	{
		if (scanchar(cursor) != '\n')
		{
			replaced.offset++;
		}
	}

	/* do it */
	bufreplace(cursor, &replaced, &info->arg, 1);

	/* we need to advance the cursor, and maybe bottom */
	markaddoffset(cursor, 1);
	if (markoffset(cursor) > markoffset(bottom))
	{
		marksetoffset(bottom, markoffset(cursor));
	}
}


/* This function attempts to expand an abbreviation at the cursor location,
 * if there is one.  If so, it deletes the short form, and pushes the long
 * form an following character back onto the type-ahead buffer.  Else it
 * returns False.
 */
static BOOLEAN tryabbr(win, nextc)
	WINDOW	win;	/* window where abbreviation may need expanding */
	_CHAR_	nextc;	/* character after the word */
{
	MARKBUF	from, to;
	CHAR	*cp, *build;
	long	slen, llen;
	CHAR	cbuf[1];

	/* Try to do an abbreviation.  To do this, we collect
	 * characters backward to the preceding whitespace.  We
	 * go to the preceding whitespace because abbreviations
	 * can't contain whitespace; we know we'll never need
	 * more characters to recognize an abbreviation.  We
	 * collect the characters backwards just because it is
	 * easier.
	 */
	for (scanalloc(&cp, win->state->cursor), build = NULL;
	     cp && scanprev(&cp) && !isspace(*cp);
	     )
	{
		buildCHAR(&build, *cp);
	}
	scanfree(&cp);
	if (build)
	{
		cp = mapabbr(build, &slen, &llen, (BOOLEAN)(win->state->acton != NULL));
		if (cp)
		{
			/* yes, we have an abbreviation! */

			/* delete the short form */
			cleanup(win, True, False, False);
			(void)marktmp(from, markbuffer(win->state->cursor), markoffset(win->state->cursor) - slen);
			(void)marktmp(to, markbuffer(win->state->cursor), markoffset(win->state->cursor));
			bufreplace(&from, &to, NULL, 0);

			/* stuff the long form, and the user's
			 * non-alnum character, into the queue
			 */
			if (nextc)
			{
				cbuf[0] = nextc;
				mapunget(cbuf, 1, False);
			}
			mapunget(cp, (int)llen, False);

			/* move cursor to where word goes */
			marksetoffset(win->state->bottom, markoffset(&from));
			marksetoffset(win->state->cursor, markoffset(&from));
			safefree(build);
			return True;
		}
		safefree(build);
	}
	return False;
}


/* This function performs an input-mode command.  Usually, this will be a
 * character to insert/replace in the text.
 */
static RESULT perform(win)
	WINDOW	win;	/* window where inputting should be done */
{
	STATE	  *state = win->state;
	INPUTINFO *ii = (INPUTINFO *)state->info;
	MARK	  cursor = state->cursor;
	MARK	  tmp;
	MARKBUF	  from, to;
	CHAR	  *cp;
	char	  *littlecp, ch;
	EXINFO	  xinfb;
	VIINFO	  vinfb;
	union
	{
		char	  partial[256];
		CHAR	  full[256];
	}	  name;
	long	  col, len;

	safeinspect();

	/* initially assume there is no matching parenthesis */
	win->match = -4;

	/* if cursor has been moved outside the top & bottom, then reset
	 * the top & bottom to match cursor
	 */
	if (markbuffer(state->top) != markbuffer(state->cursor))
	{
		marksetbuffer(state->top, markbuffer(state->cursor));
		marksetoffset(state->top, markoffset(state->cursor));
		marksetbuffer(state->bottom, markbuffer(state->cursor));
		marksetoffset(state->bottom, markoffset(state->cursor));
	}
	else if (markoffset(state->top) > markoffset(state->cursor)
	    || markoffset(state->cursor) > markoffset(state->bottom))
	{
		marksetoffset(state->top, markoffset(state->cursor));
		marksetoffset(state->bottom, markoffset(state->cursor));
	}

	/* process the keystroke */
	switch (ii->cmd)
	{
	  case INP_NORMAL:
		/* maybe try to do a digraph */
		if (ii->backsp && o_digraph)
		{
			ii->arg = digraph(ii->backsp, ii->arg);
		}

		/* If next character is non-alphanumeric, check for abbrev.
		 * (Note: Since we never expand abbreviations except in the
		 * main buffer or the ex history buffer, we can skip it if
		 * we're editing some other buffer such as regexp history.
		 * Assume only the ex history buffer has inputtab=filename.)
		 */
		if (!isalnum(ii->arg)
		 && ii->arg != '_'
		 && (state->acton == NULL || o_inputtab(markbuffer(cursor)) == 'f'))
		{
			if (tryabbr(win, ii->arg))
				break;
		}
		/* fall through... */

	  case INP_HEX1:	/* can't happen */
	  case INP_HEX2:
	  case INP_DIG1:	/* can't happen */
	  case INP_DIG2:
	  case INP_QUOTE:
		/* add the character */
		addchar(cursor, state->top, state->bottom, ii);

		/* if it wasn't whitespace, then maybe do wordwrap */
		if (!isspace(ii->arg)
		 && cursor == win->cursor
		 && win->md->wordwrap
		 && o_textwidth(markbuffer(cursor)) > 0
		 && dispmark2col(win) >= o_textwidth(markbuffer(cursor)))
		{
			/* Figure out where the current word started */
			for (scanalloc(&cp, cursor);
			     scanprev(&cp) && !isspace(*cp);
			     )
			{
			}
			if (cp)
			{
				to = *scanmark(&cp);
				markaddoffset(&to, 1L);
			}

			/* Locate the front of this line.  We won't look past
			 * back before this character.
			 */
			tmp = dispmove(win, 0L, 0L);

			/* scan backward over any whitespace */
			for (len = markoffset(&to) - markoffset(tmp);
			     len > 0 && scanprev(&cp) && isspace(*cp);
			     len--)
			{
			}
			if (cp)
			{
					from = *scanmark(&cp);
					markaddoffset(&from, 1L);
			}
			assert(cp || len <= 0);
			scanfree(&cp);

			/* We can only do the word wrap stuff if the current
			 * word isn't the line's first word.
			 */
			if (len > 0)
			{
				/* replace the whitespace with a newline */
				bufreplace(&from, &to, toCHAR("\n"), 1L);
				marksetoffset(&to, markoffset(&from) + 1);

				/* handle autoindent */
				dispindent(win, &to, -1L);
			}
		}

		/* remember digraph hints */
		ii->backsp = '\0';
		ii->prev = ii->arg;
		
		/* If the character was a parenthesis, and the showmatch
		 * option is set, then try to locate its match.
		 */
		if (o_showmatch(win) && CHARchr(toCHAR(")}]"), ii->arg))
		{
			from = *cursor;
			assert(markoffset(cursor) > 0);
			markaddoffset(cursor, -1);
			memset((char *)&vinfb, 0, sizeof vinfb);
			vinfb.command = '%';
			if (m_absolute(win, &vinfb) == RESULT_COMPLETE)
			{
				win->match = markoffset(cursor);
			}
			assert(markbuffer(cursor) == markbuffer(&from));
			*cursor = from;
		}
		break;

	  case INP_NEWLINE:
		cleanup(win, True, False, False);
		if (!tryabbr(win, '\n'))
		{
			ii->arg = '\n';
			ii->cmd = INP_NORMAL;
			perform(win);
			dispindent(win, cursor, -1);
		}
		ii->backsp = ii->prev = '\0';
		break;

	  case INP_TAB:
		if (!tryabbr(win, '\n'))
		{
			switch (o_inputtab(markbuffer(cursor)))
			{
			  case 't':
				/* insert a tab */
				addchar(cursor, state->top, state->bottom, ii);
				break;

			  case 's':
				/* insert enough spaces to look like a tab */
				col = dispmark2col(win);
				ii->arg = ' ';
				do
				{
					addchar(cursor, state->top, state->bottom, ii);
				} while ((++col) % o_tabstop(markbuffer(cursor)) != 0);
				break;

			  case 'f':
				/* filename completion */

				/* if at start of input, then fail */
				if (markoffset(cursor) == markoffset(state->top))
				{
					guibeep(win);
					break;
				}

				/* locate the previous character */
				tmp = markdup(cursor);
				markaddoffset(tmp, -1);

				/* if previous is whitespace, then fail */
				if (isspace(scanchar(tmp)) || scanchar(tmp) == '(')
				{
					markfree(tmp);
					guibeep(win);
					break;
				}

				/* collect the partial name into char array */
				littlecp = &name.partial[QTY(name.partial)];
				*--littlecp = '\0';
				ch = scanchar(tmp);
				do
				{
					*--littlecp = ch;
					markaddoffset(tmp, -1);
					ch = (markoffset(tmp) >= markoffset(state->top)) ? scanchar(tmp) : ' ';
				} while (!isspace(ch) && ch != '(');
				markaddoffset(tmp, 1);

				/* try to expand the filename */
				littlecp = iofilename(littlecp, (ch == '(') ? ')' : '\t');
				if (!littlecp)
				{
					markfree(tmp);
					guibeep(win);
					break;
				}

				/* name found -- replace old word with expanded name */
				for (cp = name.full, col = 0;
				     (*cp++ = *littlecp++) != '\0'; /* yes, ASSIGNMENT! */
				     col++)
				{
				}
				bufreplace(tmp, win->state->bottom, name.full, col);
				marksetoffset(cursor, markoffset(tmp) + col);
				marksetoffset(win->state->bottom, markoffset(cursor));
				markfree(tmp);
			}
		}
		ii->backsp = ii->prev = '\0';
		break;

	  case INP_ONEVI:
		cleanup(win, True, False, True);
		vipush(win, ELVIS_ONCE, NULL);
		ii->backsp = ii->prev = '\0';
		ii->setbottom = True;
		break;

	  case INP_MULTIVI:
		cleanup(win, True, True, True);
		win->state->flags |= ELVIS_POP;
		ii->backsp = ii->prev = '\0';

#if 1
		/* if blank line in autoindent mode, then delete any whitespace */
		if (o_autoindent(markbuffer(cursor)))
		{
			for (from = *dispmove(win,0L,0L), scanalloc(&cp, &from);
			     cp && (*cp == ' ' || *cp == '\t');
			     scannext(&cp))
			{
			}
			if (cp && *cp == '\n')
			{
				to = *scanmark(&cp);
				scanfree(&cp);
				if (markoffset(&to) > markoffset(&from) &&
				    markoffset(&to) - 1 == markoffset(cursor))
				{
					bufreplace(&from, &to, NULL, 0L);
				}
			}
			else
			{
				scanfree(&cp);
			}
		}
#endif
		break;

	  case INP_BACKSPACE:
		ii->backsp = '\0';
		if (markoffset(win->state->top) < markoffset(cursor))
		{
			/* backspace within the newly typed text */
			markaddoffset(cursor, -1);
			ii->backsp = ii->prev;
		}
		else if (win->state->acton != NULL
			&& (win->state->flags & ELVIS_1LINE) != 0)
		{
			/* backspace out of an ex command line or regexp line */
			cleanup(win, True, True, True);
			win->state->flags |= ELVIS_POP;
			win->state->pop->flags &= ~(ELVIS_MORE | ELVIS_ONCE);
			if (win->state->pop->perform == _viperform)
			{
				viinitcmd((VIINFO *)win->state->pop->info);
			}
			ii->backsp = '\0';
		}
		else
		{
			/* bump into left edge of new text */
			guibeep(win);
		}
		ii->prev = '\0';
		break;

	  case INP_BACKWORD:
		if (markoffset(win->state->top) < markoffset(cursor))
		{
			/* expect to back up at least one character */
			markaddoffset(cursor, -1);
			scanalloc(&cp, cursor);

			/* if on whitespace, then back up to non-whitespace */
			while (markoffset(win->state->top) < markoffset(win->state->cursor)
			    && isspace(*cp))
			{
				markaddoffset(cursor, -1);
				scanprev(&cp);
			}

			/* back up to whitespace */
			while (markoffset(win->state->top) < markoffset(win->state->cursor)
			    && !isspace(*cp))
			{
				markaddoffset(cursor, -1);
				scanprev(&cp);
			}

			/* if we hit whitespace, then leave cursor after it */
			if (isspace(*cp))
			{
				markaddoffset(cursor, 1);
			}
			scanfree(&cp);
		}
		else
		{
			guibeep(win);
		}
		ii->backsp = ii->prev = '\0';
		break;

	  case INP_BACKLINE:
		/* find the start of this line, or if the cursor is already
		 * there, then the start of the preceding line.
		 */
		tmp = (*win->md->move)(win, cursor, 0, 0, False);
		if (markoffset(tmp) == markoffset(cursor))
		{
			tmp = (*win->md->move)(win, cursor, -1, 0, False);
		}

		/* move to either the start of the line or the top of the
		 * edited region, whichever is closer.
		 */
		if (markoffset(tmp) > markoffset(win->state->top))
		{
			marksetoffset(cursor, markoffset(tmp));
		}
		else if (markoffset(state->top) < markoffset(cursor))
		{
			marksetoffset(cursor, markoffset(state->top));
		}
		else
		{
			guibeep(win);
		}
		ii->backsp = ii->prev = '\0';
		break;

	  case INP_SHIFTL:
	  case INP_SHIFTR:
		/* build a :< or :> ex command */
		memset((char *)&xinfb, 0, sizeof xinfb);
		xinfb.defaddr = *cursor;
		xinfb.from = xinfb.to = markline(cursor);
		xinfb.fromaddr = marktmp(from, markbuffer(cursor), lowline(bufbufinfo(markbuffer(cursor)), xinfb.from));
		xinfb.toaddr = marktmp(to, markbuffer(cursor), lowline(bufbufinfo(markbuffer(cursor)), xinfb.to + 1));;
		xinfb.command = (ii->cmd == INP_SHIFTL) ? EX_SHIFTL : EX_SHIFTR;
		xinfb.multi = 1;
		xinfb.bang = True;

		/* execute the command */
		len = o_bufchars(markbuffer(cursor)) - markoffset(cursor);
		assert(len >= 0);
		(void)ex_shift(&xinfb);
		ii->backsp = ii->prev = '\0';
		assert(o_bufchars(markbuffer(cursor)) >= len);
		marksetoffset(cursor, o_bufchars(markbuffer(cursor)) - len);
		break;

	  case INP_EXPOSE:
		drawexpose(win, 0, 0, (int)(o_lines(win) - 1), (int)(o_columns(win) - 1));
		break;

	  case INP_PREVIOUS:
	  case INP_AGAIN:
	  case INP_PUT:
		cleanup(win, True, False, False);

		/* Copy the text.  Be careful not to change the "top" mark. */
		from = *state->top;
		tmp = cutput((ii->cmd == INP_PUT ? '1' : '.'),
					win, cursor, False, True, True);
		marksetoffset(state->top, markoffset(&from));

		/* if successful, tweak the "cursor" and "bottom" marks. */
		if (tmp)
		{
			marksetoffset(cursor, markoffset(tmp) + 1);
			marksetoffset(state->bottom, markoffset(cursor));
			if (ii->cmd == INP_PREVIOUS)
			{
				cleanup(win, True, True, True);
				state->flags |= ELVIS_POP;
			}
		}
		ii->backsp = ii->prev = '\0';
		break;

	}

	/* set wantcol to the cursor's current column */
	win->wantcol = dispmark2col(win);

	/* prepare for next command */
	ii->cmd = INP_NORMAL;

	return RESULT_COMPLETE;
}

/* This function parses a command.  This involves remembering whether we're
 * in the middle of a ^V quoted character, and also recognizing some special
 * characters.
 */
static RESULT parse(key, info)
	_CHAR_	key;	/* next keystroke */
	void	*info;	/* current parsing state */
{
	INPUTINFO *ii = (INPUTINFO *)info;

	/* parse the input command */
	if (ii->cmd == INP_HEX1 || ii->cmd == INP_HEX2)
	{
		/* convert hex digit from ASCII to binary */
		if (key >= '0' && key <= '9')
		{
			key -= '0';
		}
		else if (key >= 'a' && key <= 'f')
		{
			key -= 'a' - 10;
		}
		else if (key >= 'A' && key <= 'F')
		{
			key -= 'A' - 10;
		}
		else
		{
			/* this command is invalid; prepare for next command */
			ii->cmd = INP_NORMAL;
			return RESULT_ERROR;
		}

		/* merge into arg */
		if (ii->cmd == INP_HEX1)
		{
			ii->arg = (key << 4);
			ii->cmd = INP_HEX2;
			windefault->state->mapflags |= MAP_DISABLE;
			return RESULT_MORE;
		}
		else
		{
			ii->arg |= key;
			return RESULT_COMPLETE;
		}
	}
	else if (ii->cmd == INP_DIG1 || ii->cmd == INP_DIG2)
	{
		if (ii->cmd == INP_DIG1)
		{
			ii->arg = key;
			ii->cmd = INP_DIG2;
			windefault->state->mapflags |= MAP_DISABLE;
			return RESULT_MORE;
		}
		else
		{
			ii->arg = digraph(ii->arg, key);
			return RESULT_COMPLETE;
		}
	}
	else if (ii->cmd == INP_QUOTE)
	{
		ii->arg = key;
	}
	else
	{
		ii->arg = key;
		switch (key)
		{
		  case ELVCTRL('@'):	ii->cmd = INP_PREVIOUS;		break;
		  case ELVCTRL('A'):	ii->cmd = INP_AGAIN;		break;
		  case ELVCTRL('D'):	ii->cmd = INP_SHIFTL;		break;
		  case '\177': /* usually mapped to "visual x", else... */
		  case ELVCTRL('H'):	ii->cmd = INP_BACKSPACE;	break;
		  case ELVCTRL('I'):	ii->cmd = INP_TAB;		break;
		  case ELVCTRL('J'):	ii->cmd = INP_NEWLINE;		break;
		  case ELVCTRL('K'):	ii->cmd = INP_DIG1;		break;
		  case ELVCTRL('L'):	ii->cmd = INP_EXPOSE;		break;
		  case ELVCTRL('M'):	ii->cmd = INP_NEWLINE;		break;
		  case ELVCTRL('O'):	ii->cmd = INP_ONEVI;		break;
		  case ELVCTRL('P'):	ii->cmd = INP_PUT;		break;
		  case ELVCTRL('R'):	ii->cmd = INP_EXPOSE;		break;
		  case ELVCTRL('T'):	ii->cmd = INP_SHIFTR;		break;
		  case ELVCTRL('U'):	ii->cmd = INP_BACKLINE;		break;
		  case ELVCTRL('V'):	ii->cmd = INP_QUOTE;		break;
		  case ELVCTRL('W'):	ii->cmd = INP_BACKWORD;		break;
		  case ELVCTRL('X'):	ii->cmd = INP_HEX1;		break;
		  case ELVCTRL('['):	ii->cmd = INP_MULTIVI;		break;
		  default:		ii->cmd = INP_NORMAL;
		}

		/* ^V, ^X, and ^K require more keystrokes... */
		if (ii->cmd == INP_QUOTE || ii->cmd == INP_HEX1 || ii->cmd == INP_DIG1)
		{
			windefault->state->mapflags |= MAP_DISABLE;
			return RESULT_MORE;
		}
	}

	/* the command is complete */
	return RESULT_COMPLETE;
}

/* This function decides on a cursor shape */
static ELVCURSOR shape(win)
	WINDOW	win;	/* window whose shape should be returned */
{
	STATE	*state = win->state;
	INPUTINFO *info = (INPUTINFO *)state->info;
	MARK	cursor = state->cursor;

	/* if in the middle of ^V, then always CURSOR_QUOTE */
	if (info->cmd == INP_QUOTE)
	{
		state->mapflags &= ~(MAP_INPUT|MAP_COMMAND);
		return CURSOR_QUOTE;
	}
	state->mapflags |= MAP_INPUT;

	/* decide whether to insert or replace */
	if (markoffset(state->top) <= markoffset(cursor)
	 && markoffset(cursor) < markoffset(state->bottom))
	{
		if (info->setbottom)
		{
			marksetoffset(state->bottom, markoffset(cursor));
			info->setbottom = False;
			return CURSOR_INSERT;
		}
		return CURSOR_REPLACE;
	}
	else if (info->replacing && markoffset(cursor) < o_bufchars(markbuffer(cursor)))
	{
		if (scanchar(cursor) != '\n')
		{
			info->setbottom = False;
			return CURSOR_REPLACE;
		}
	}
	info->setbottom = False;
	return CURSOR_INSERT;
}


/* This function pushes a state onto the state stack, and then initializes it
 * to be either an input or replace state, with the cursor at a given location.
 * The "mode" argument can be 'R' for replace, or anything else for input.
 * The input cursor is "cursor", which should generally the result of a
 * markalloc() or markdup() function call.  Ditto for the top and bottom of
 * the edit region.
 */
void inputpush(win, flags, mode)
	WINDOW		win;	/* window that should be switched to input mode */
	ELVISSTATE	flags;	/* flags describing this state */
	_char_		mode;	/* 'R' for replace, or anything else for insert */
{
	/* push the state */
	flags |= ELVIS_REGION;
	statepush(win, flags);

	/* initialize the state */
	win->state->parse = parse;
	win->state->perform = perform;
	win->state->shape = shape;
	win->state->info = safealloc(1, sizeof (INPUTINFO));
	win->state->mapflags |= MAP_INPUT;
	if (mode == 'R')
	{
		((INPUTINFO *)win->state->info)->replacing = True;
		win->state->modename = "Replace";
	}
	else
	{
		((INPUTINFO *)win->state->info)->replacing = False;
		win->state->modename = " Input ";
	}
}

/* This function tweaks the most recent "input" or "replace" state.
 * The "mode" can be 't' to toggle "input" to "replace" or vice verse,
 * 'R' to force the mode to be "replace", or anything else to force the
 * mode to be "input".  This function is called by vi mode's perform()
 * function.
 */
void inputtoggle(win, mode)
	WINDOW	win;	/* window to be toggled */
	_char_	mode;	/* 'R' for replace, 't' to toggle, else insert */
{
	STATE	*state;

	/* find the most recent "input" or "replace" state */
	for (state = win->state; state && state->perform != perform; state = state->pop)
	{
	}
	assert(state != NULL);

	/* change the mode */
	switch (mode)
	{
	  case 't':
		((INPUTINFO *)state->info)->replacing = (BOOLEAN)!((INPUTINFO *)state->info)->replacing;
		break;

	  case 'R':
		((INPUTINFO *)state->info)->replacing = True;
		break;

	  default:
		((INPUTINFO *)state->info)->replacing = False;
	}

	if (((INPUTINFO *)state->info)->replacing)
	{
		state->modename = "Replace";
	}
	else
	{
		state->modename = " Input";
	}
}

/* This function sets the edit boundaries of an "input" state.  If there
 * is no input state on the state stack, then this function will push one.
 * This function is used to implement the <c> operator, among other things.
 */
void inputchange(win, from, to, linemd)
	WINDOW	win;	/* window to be affected */
	MARK	from;	/* new start of edit bounds */
	MARK	to;	/* new end of edit bounds */
	BOOLEAN	linemd;	/* replace old text with a new line? */
{
	MARKBUF	tmp;
	CHAR	ch;

	assert(markbuffer(from) == markbuffer(win->state->cursor));
	assert(markbuffer(from) == markbuffer(to));
	assert(markoffset(from) <= markoffset(to));

	/* Was this command issued via <Control-O> from input mode?
	 * If not, then we'll need to push one.
	 */
	if (!win->state->pop)
	{
		inputpush(win, 0, 'i');
	}

	/* replace the last char with '$', if there is a last char
	 * and it is on the same line.  If it is on a different line,
	 * then delete the old text.  If from==to, then do nothing.
	 */
	if (markoffset(from) == markoffset(to))
	{
		/* do nothing */
	}
	else if (markoffset(to) > markoffset(dispmove(win, 0, INFINITY)))
	{
		/* delete the old text */
		if (linemd)
		{
			if (o_autoindent(markbuffer(from)))
			{
				for (ch = scanchar(from);
				     markoffset(from) < markoffset(to) && (ch == ' ' || ch == '\t');
				     markaddoffset(from, 1), ch = scanchar(from))
				{
				}
			}
			bufreplace(from, to, toCHAR("\n"), 1);
		}
		else
		{
			bufreplace(from, to, NULL, 0);
		}
		marksetoffset(to, markoffset(from));
	}
	else
	{
		/* replace the last character with a '$' */
		tmp = *to;
		tmp.offset--;
		bufreplace(&tmp, to, toCHAR("$"), 1);
	}

	/* set the edit boundaries and the cursor */
	marksetbuffer(win->state->top, markbuffer(from));
	marksetbuffer(win->state->bottom, markbuffer(to));
	marksetoffset(win->state->top, markoffset(from));
	marksetoffset(win->state->bottom, markoffset(to));
	marksetoffset(win->state->cursor, markoffset(from));
}


/* This function is called by statekey() when the user hits <Enter>, before
 * calling the stratum's enter() function.  This function deletes extra
 * characters after the cursor, and adjusts the endpoints of the edited
 * region to make them be whole lines.
 */
void inputbeforeenter(win)
	WINDOW	win;	/* window where <Enter> was just pressed */
{
	/* Make sure "win->state->bottom" includes the cursor position */
	if (markoffset(win->state->bottom) < markoffset(win->state->cursor))
	{
		marksetoffset(win->state->bottom, markoffset(win->state->cursor));
	}

	/* Delete stuff from after the cursor */
	cleanup(win, True, False, False);

	/* adjust the endpoints of the edited area to be whole lines */
	marksetoffset(win->state->top,
		markoffset((*dmnormal.move)(win, win->state->top, 0, 0, False)));
	marksetoffset(win->state->bottom,
		markoffset((*dmnormal.move)(win, win->state->bottom, 0, INFINITY, False)));
}
