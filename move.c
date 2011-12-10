/* move.c */
/* Copyright 1995 by Steve Kirkendall */

char id_move[] = "$Id: move.c,v 2.39 1996/09/10 16:51:38 steve Exp $";

#include "elvis.h"


/* This function implements the "h" command */
RESULT m_left(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	tmp;
	DEFAULT(1);

	/* find the start of this line */
	tmp = dispmove(win, 0L, 0L);

	/* if already at the start of the line, then fail */
	if (markoffset(tmp) == markoffset(win->state->cursor))
	{
		return RESULT_ERROR;
	}

	/* move either the requested number of characters left, or to the
	 * start of the line, whichever is closer
	 */
	if (markoffset(win->state->cursor) - vinf->count > markoffset(tmp))
	{
		markaddoffset(win->state->cursor, -vinf->count);
	}
	else
	{
		marksetoffset(win->state->cursor, markoffset(tmp));
	}

	return RESULT_COMPLETE;
}


/* This function implements the "l" command */
RESULT m_right(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	tmp;
	DEFAULT(1);

	/* find the end of this line.  This is complicated by the fact that
	 * when used as the target of an operator, the l command can move
	 * past the end of the line.
	 */
	if (vinf->oper && !win->state->acton)
		tmp = (*win->md->move)(win, win->cursor, 0L, INFINITY, False);
	else
		tmp = dispmove(win, 0L, INFINITY);

	/* if already at the end of the line, then fail */
	if (markoffset(tmp) == markoffset(win->state->cursor))
	{
		return RESULT_ERROR;
	}

	/* move either the requested number of characters right, or to the
	 * end of the line, whichever is closer
	 */
	if (markoffset(win->state->cursor) + vinf->count < markoffset(tmp))
	{
		markaddoffset(win->state->cursor, vinf->count);
	}
	else
	{
		marksetoffset(win->state->cursor, markoffset(tmp));
	}

	return RESULT_COMPLETE;
}


/* This function implements the "j" and "k" functions */
RESULT m_updown(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	tmp = NULL;
	DEFAULT(1);

	/* do the move */
	switch (vinf->command)
	{
	  case '_':
		/* decremement count & then treat like <Enter>... */
		vinf->count--;
		/* fall through... */

	  case ELVCTRL('J'):
	  case ELVCTRL('M'):
	  case ELVCTRL('N'):
	  case '+':
	  case 'j':
		tmp = dispmove(win, vinf->count, win->wantcol);
		break;

	  case ELVCTRL('P'):
	  case '-':
	  case 'k':
		tmp = dispmove(win, -vinf->count, win->wantcol);
		break;

#ifndef NDEBUG
	  default:
		abort();
#endif
	}

	/* check for goofy return values */
	if (markoffset(tmp) < 0
	 || markoffset(tmp) >= o_bufchars(markbuffer(win->state->cursor))
	 || (markoffset(tmp) == markoffset(win->state->cursor) && vinf->count != 0))
	{
		return RESULT_ERROR;
	}

	/* It's good! */
	marksetoffset(win->state->cursor, markoffset(tmp));
	return RESULT_COMPLETE;
}


/* This function implements the "^" function */
RESULT m_front(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	CHAR	*cp;

	/* scan from the start of the line, to the first non-space */
	scanalloc(&cp, dispmove(win, 0, 0));
	if (cp && (*cp == '\t' || *cp == ' '))
	{
		do
		{
			scannext(&cp);
		} while (cp && (*cp == ' ' || *cp == '\t'));
		if (*cp == '\n')
		{
			scanprev(&cp);
		}
	}
	if (!cp)
	{
		return RESULT_ERROR;
	}
	marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
	scanfree(&cp);
	return RESULT_COMPLETE;
}

/* This function implements the <Shift-G>, <Control-G>, and <%> commands, which
 * move the cursor to a specific line, character, or percentage of the buffer.
 * The number is given in the "count" field; if no number is given, then either
 * move to the last line, show buffer statistics, or move to matching bracket,
 * respectively.
 */
RESULT m_absolute(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	BUFFER	buf = markbuffer(win->state->cursor);
	CHAR	match;	/* the parenthesis we want (for <%> command) */
	CHAR	nest = 0;/* the starting parenthesis */
	CHAR	*cp;	/* used for scanning through text */
	long	count;	/* nesting depth */
	EXINFO	xinfb;	/* an ex command */

	assert(vinf->command == 'G' || vinf->command == ELVCTRL('G') || vinf->command == '%');

	switch (vinf->command)
	{
	  case 'G':
		DEFAULT(o_buflines(buf));

		/* Try to go to the requestedc line.  Catch errors, including a
		 * numberless <G> in an empty buffer.
		 */
		if (!marksetline(win->state->cursor, vinf->count))
		{
			msg(MSG_ERROR, "[d]only $1 lines", o_buflines(buf));
			return RESULT_ERROR;
		}
		break;

	  case ELVCTRL('G'):
		if (!vinf->count)
		{
			/* no count, just show buffer status */
			memset((char *)&xinfb, 0, sizeof xinfb);
			xinfb.window = win;
			xinfb.defaddr = *win->state->cursor;
			xinfb.command = EX_FILE;
			ex_file(&xinfb);
		}
		else if (vinf->count < 0 || vinf->count > o_bufchars(buf))
		{
			/* request offset is out of range */
			return RESULT_ERROR;
		}
		else
		{
			/* set the cursor to the requested offset */
			marksetoffset(win->state->cursor, vinf->count - 1);
		}
		break;

	  case '%':
		if (!vinf->count)
		{
			/* search forward within line for one of "[](){}" */
			for (match = '\0', scanalloc(&cp, win->state->cursor); !match;)
			{
				/* if hit end-of-line or end-of-buffer without
				 * finding a parenthesis, then fail.
				 */
				if (!cp || *cp == '\n')
				{
					scanfree(&cp);
					return RESULT_ERROR;
				}

				/* if parenthesis, great! else keep looking */
				switch (*cp)
				{
				  case '[':	match = ']';	break;
				  case ']':	match = '[';	break;
				  case '(':	match = ')';	break;
				  case ')':	match = '(';	break;
				  case '{':	match = '}';	break;
				  case '}':	match = '{';	break;
				  default:	scannext(&cp);
				}
			}
			assert(cp != NULL);
			nest = *cp;

			/* search forward or backward for match */
			if (match == '(' || match == '[' || match == '{')/*)]}*/
			{
				/* search backward */
				for (count = 1; count > 0; )
				{
					/* back up 1 char; give up at top of buffer */
					if (!scanprev(&cp))
					{
						break;
					}

					/* check the char */
					if (*cp == match)
						count--;
					else if (*cp == nest)
						count++;
				}
			}
			else
			{
				/* search forward */
				for (count = 1; count > 0; )
				{
					/* advance 1 char; give up at end of buffer */
					if (!scannext(&cp))
					{
						break;
					}

					/* check the char */
					if (*cp == match)
						count--;
					else if (*cp == nest)
						count++;
				}
			}

			/* if we hit the end of the buffer, then fail */
			if (!cp)
			{
				scanfree(&cp);
				return RESULT_ERROR;
			}

			/* move the cursor to the match */
			marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
			scanfree(&cp);
		}
		else if (vinf->count < 1 || vinf->count > 100)
		{
			msg(MSG_ERROR, "bad percentage");
			return RESULT_ERROR;
		}
		else
		{
			/* Compute the character offset, given the percentage.
			 * I'm slightly careful here to avoid overflowing
			 * the long int which stores the offset.
			 */
			if (o_bufchars(buf) > 1000000L)
			{
				marksetoffset(win->state->cursor,
					(o_bufchars(buf) / 100) * vinf->count);
			}
			else
			{
				marksetoffset(win->state->cursor,
					(o_bufchars(buf) * vinf->count) / 100);
			}
			vinf->tweak |= TWEAK_FRONT;
		}
		return RESULT_COMPLETE;
	}

	return RESULT_COMPLETE;
}

/* Move to a mark.  This function implements the <'> and <`> commands. */
RESULT m_mark(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	newmark;

	/* check for <'><'> or <`><`> */
	if (vinf->command == vinf->key2)
	{
		if (win->prevcursor)
		{
			assert(markbuffer(win->state->cursor) == markbuffer(win->prevcursor));
			marksetoffset(win->state->cursor, markoffset(win->prevcursor));
			return RESULT_COMPLETE;
		}
		return RESULT_ERROR;
	}

	/* else key2 had better be a lowercase ASCII letter */
	if (vinf->key2 < 'a' || vinf->key2 > 'z')
	{
		return RESULT_ERROR;
	}

	/* look up the named mark */
	newmark = namedmark[vinf->key2 - 'a'];
	if (!newmark)
	{
		msg(MSG_ERROR, "[C]'$1 unset", vinf->key2);
		return RESULT_ERROR;
	}

	/* if the named mark is in a different buffer, fail. */
	/* (A later version of elvis may be able to do this!) */
	if (markbuffer(newmark) != markbuffer(win->state->cursor))
	{
		msg(MSG_ERROR, "[C]'$1 in other buffer", vinf->key2);
		return RESULT_ERROR;
	}

	/* move to the named mark */
	marksetoffset(win->state->cursor, markoffset(newmark));
	return RESULT_COMPLETE;
}

/* This function implements the whitespace-delimited word movement commands:
 * <Shift-W>, <Shift-B>, and <Shift-E>.
 */
RESULT m_bigword(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	BOOLEAN	whitespace;	/* do we include following whitespace? */
	BOOLEAN backward;	/* are we searching backwards? */
	long	offset;		/* offset of *cp character */
	long	count;		/* number of words to skip */
	long	end;		/* offset of the end of the buffer */
	BOOLEAN	inword;		/* are we currently in a word? */
	CHAR	*cp;		/* used for scanning chars of buffer */

	DEFAULT(1);

	/* start the scan */
	offset = markoffset(win->state->cursor);
	scanalloc(&cp, win->state->cursor);
	assert(cp != NULL);
	count = vinf->count;
	end = o_bufchars(markbuffer(win->state->cursor));

	/* figure out which type of movement we'll be doing */
	switch (vinf->command)
	{
	  case 'B':
		backward = True;
		whitespace = False;
		inword = False;
		break;

	  case 'E':
		backward = False;
		whitespace = False;
		inword = False;
		break;

	  default:
		backward = False;
		inword = (BOOLEAN)!isspace(*cp);
		if (vinf->oper == 'c')
		{
			/* "cW" acts like "cE", pretty much */
			whitespace = False;
			vinf->tweak |= TWEAK_INCL;

			/* starting on whitespace? */
			if (!inword)
			{
				/* When "cW" starts on whitespace, it changes
				 * one less word than normal.  If it would
				 * normally change just one word, then it
				 * should change a single whitespace character.
				 */
				vinf->count--;
				if (vinf->count == 0)
				{
					scanfree(&cp);
					return RESULT_COMPLETE;
				}
			}
			else if (markoffset(win->state->cursor) > 0)
			{
				/* When "cW" starts on the last character of a
				 * word, then it should change just that last
				 * character.  By temporarily moving the
				 * cursor back one char, we can achieve this
				 * effect without affecting the results of any
				 * other movement.
				 */
				scanprev(&cp);
				offset--;
			}
		}
		else
		{
			whitespace = True;
		}
		break;
	}

	/* continue... */
	if (backward)
	{
		/* move backward until we hit the top of the buffer, or
		 * the start of the desired word.
		 */
		while (count > 0 && offset > 0)
		{
			scanprev(&cp);
			assert(cp != NULL);
			if (isspace(*cp))
			{
				if (inword)
				{
					count--;
				}
				inword = False;
			}
			else
			{
				inword = True;
			}
			if (count > 0)
			{
				offset--;
				if (offset == 0 && count == 1)
				{
					count = 0;
				}
			}
		}
	}
	else
	{
		/* move forward until we hit the end of the buffer, or
		 * the start of the desired word.
		 */
		while (count > 0 && offset < end - 1)
		{
			scannext(&cp);
			assert(cp != NULL);
			if (isspace(*cp))
			{
				if (vinf->oper && *cp == '\n' && count == 1)
				{
					count = 0;
					if (!(vinf->tweak & TWEAK_INCL))
						offset++;
				}
				else if (inword && !whitespace)
				{
					count--;
				}
				inword = False;
				if (count > 0)
				{
					offset++;
				}
			}
			else
			{
				if (!inword && whitespace)
				{
					count--;
				}
				inword = True;
				offset++;
			}
		}
	}

	/* cleanup */
	scanfree(&cp);

	/* if the count didn't reach 0, we failed */
	if (count > 0)
	{
		return RESULT_ERROR;
	}

	/* else set the cursor's offset */
	assert(offset < end && offset >= 0);
	marksetoffset(win->state->cursor, offset);
	return RESULT_COMPLETE;
}

/* This function implements the <b>, <e>, and <w> word movement commands
 * by calling the mode-dependent wordmove function.
 */
RESULT m_word(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	BOOLEAN	whitespace;	/* include trailing whitespace? */
	BOOLEAN backward;	/* are we moving backward? */
	long	span;		/* offset of starting position */
	long	newline;	/* offset of newline */
	CHAR	*cp;		/* used for scanning backward for newline */
	CHAR	atcursor;	/* character at the cursor position */

	DEFAULT(1);

	/* figure out which type of movement we'll be doing */
	switch (vinf->command)
	{
	  case 'b':
		backward = True;
		whitespace = False;
		break;

	  case 'e':
		backward = False;
		whitespace = False;
		break;

	  default:
		backward = False;
		if (vinf->oper == 'c')
		{
			/* "cw" acts like "ce", pretty much */
			whitespace = False;
			vinf->tweak |= TWEAK_INCL;

			/* starting on whitespace? */
			atcursor = scanchar(win->state->cursor);
			if (atcursor == ' ' || atcursor == '\t')
			{
				/* When "cw" starts on whitespace, it changes
				 * one less word than normal.  If it would
				 * normally change just one word, then it
				 * should change a single whitespace character.
				 */
				vinf->count--;
				if (vinf->count == 0)
				{
					return RESULT_COMPLETE;
				}
			}
			else if (markoffset(win->state->cursor) > 0)
			{
				/* When "cw" starts on the last character of a
				 * word, then it should change just that last
				 * character.  By temporarily moving the
				 * cursor back one char, we can achieve this
				 * effect without affecting the results of any
				 * other movement.
				 */
				markaddoffset(win->state->cursor, -1);
			}
		}
		else
		{
			whitespace = True;
		}
		break;
	}

	/* remember the starting position */
	span = markoffset(win->state->cursor);

	/* Call the mode-dependent function.  If we're editing a history buffer
	 * then always use dmnormal's version; else (for the window's main
	 * buffer) use the window's mode's function.
	 */
	if (!(*(win->state->acton ? dmnormal.wordmove : win->md->wordmove))
		(win->state->cursor, vinf->count, backward, whitespace))
	{
		/* movement failed */
		return RESULT_ERROR;
	}

	/* NOTE: If we get here, then the word movement succeeded and the
	 * cursor has been moved.
	 */

	/* We need to avoid newlines for <w> movements that are used as the
	 * target of an operator (except for <c><w> which doesn't include
	 * whitespace).
	 */
	if (whitespace && vinf->oper && vinf->oper != 'c')
	{
		newline = markoffset(win->state->cursor);
		span = newline - span;
		scanalloc(&cp, win->state->cursor);
		while (scanprev(&cp) && span-- > 0)
		{
			if (*cp == '\n')
				newline = markoffset(scanmark(&cp));
		}
		scanfree(&cp);
		marksetoffset(win->state->cursor, newline);
	}
	else if (whitespace && !vinf->oper && scanchar(win->state->cursor) == '\n')
	{
		/* tried a plain old "w" command at end of file -- 
		 * move cursor back to starting point and fail.
		 */
		marksetoffset(win->state->cursor, span);
		return RESULT_ERROR;
	}

	return RESULT_COMPLETE;
}

/* This function scrolls the screen, implementing the <Control-E>, <Control-Y>,
 * <Control-F>, <Control-B>, <Control-D>, and <Control-U> commands.
 */
RESULT	m_scroll(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARKBUF	tmp;

	assert(!win->state->acton);
	assert(!(win->state->flags & ELVIS_BOTTOM) || vinf->command == ELVCTRL('D'));

	/* adjust the count */
	if (vinf->command == ELVCTRL('U') || vinf->command == ELVCTRL('D'))
	{
		if (vinf->count == 0)
		{
			vinf->count = o_scroll(win);
		}
		else
		{
			if (vinf->count > o_lines(win) - 1)
			{
				vinf->count = o_lines(win) - 1;
			}
			o_scroll(win) = vinf->count;
		}
	}
	else
	{
		DEFAULT(1);
	}

	/* Do the scroll */
	switch (vinf->command)
	{
	  case ELVCTRL('U'):
		win->di->topline = markoffset((*win->md->move)(win, marktmp(tmp, markbuffer(win->cursor), win->di->topline), -vinf->count, 0, True));
		win->di->bottomline = markoffset((*win->md->move)(win, marktmp(tmp, markbuffer(win->cursor), win->di->bottomline), -vinf->count, 0, True));
		marksetoffset(win->cursor, markoffset(dispmove(win, -vinf->count, 0)));
		break;

	  case ELVCTRL('D'):
		win->di->topline = markoffset((*win->md->move)(win, marktmp(tmp, markbuffer(win->cursor), win->di->topline), vinf->count, 0, True));
		win->di->bottomline = markoffset((*win->md->move)(win, marktmp(tmp, markbuffer(win->cursor), win->di->bottomline), vinf->count, 0, True));
		marksetoffset(win->cursor, markoffset(dispmove(win, vinf->count, 0)));
		break;

	  case ELVCTRL('Y'):
		win->di->topline = markoffset((*win->md->move)(win, marktmp(tmp, markbuffer(win->cursor), win->di->topline), -vinf->count, 0, True));
		win->di->bottomline = markoffset((*win->md->move)(win, marktmp(tmp, markbuffer(win->cursor), win->di->bottomline), -vinf->count, 0, True));
		marksetoffset(win->cursor, markoffset(dispmove(win, 0, win->wantcol)));
		if (markoffset(win->cursor) >= win->di->bottomline)
		{
			marksetoffset(win->cursor, win->di->bottomline);
			marksetoffset(win->cursor, markoffset(dispmove(win, -1, win->wantcol)));
		}
		break;

	  case ELVCTRL('E'):
		win->di->topline = markoffset((*win->md->move)(win, marktmp(tmp, markbuffer(win->cursor), win->di->topline), vinf->count, 0, True));
		win->di->bottomline = markoffset((*win->md->move)(win, marktmp(tmp, markbuffer(win->cursor), win->di->bottomline), vinf->count, 0, True));
		if (markoffset(win->cursor) < win->di->topline)
		{
			marksetoffset(win->cursor, win->di->topline);
			marksetoffset(win->cursor, markoffset(dispmove(win, 0, win->wantcol)));
		}
		break;

	  case ELVCTRL('F'):
		marksetoffset(win->cursor, win->di->bottomline);
		win->di->topline = markoffset(dispmove(win, -1, 0));
		marksetoffset(win->cursor, win->di->topline);
		win->di->bottomline = markoffset((*win->md->move)(win, marktmp(tmp, markbuffer(win->cursor), win->di->topline), o_lines(win), 0, True));
		break;

	  case ELVCTRL('B'):
		/* note: this adjustment of topline can be sloppy, because
		 * the drawimage() function will perform slop scrolling, if
		 * necessary, to keep the cursor in the screen.
		 */
		marksetoffset(win->cursor, win->di->topline);
		win->di->topline = markoffset(dispmove(win, -o_lines(win), 0));
		win->di->bottomline = markoffset((*win->md->move)(win, marktmp(tmp, markbuffer(win->cursor), win->di->topline), o_lines(win), 0, True));
		break;
	}

	/* partially disable optimization for the next redraw - it doesn't
	 * automatically realize that scrolling is a type of change.
	 */
	win->di->logic = DRAW_CHANGED;

	return RESULT_COMPLETE;
}

/* This function moves the cursor to a given column.  The window's desired
 * column ("wantcol") is set to the requested column.  This implements the
 * <|>, <0>, <Control-X>, and <$> commands.
 */
RESULT	m_column(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	dest;

	/* choose a column number */
	switch (vinf->command)
	{
	  case '|':
	  case '0':
		DEFAULT(1);
		break;

	  case ELVCTRL('X'):
		DEFAULT(o_columns(win));
		vinf->count += win->di->skipped;
		break;

	  case '$':
		vinf->count = INFINITY;
		break;
	}

	/* move to the requested column (or as close as possible). */
	dest = dispmove(win, 0L, vinf->count - 1);
	marksetoffset(win->state->cursor, markoffset(dest));

	/* if the window is editing the main buffer, set wantcol...
	 * even if the cursor didn't quite make it to the requested column.
	 */
	win->wantcol = vinf->count - 1;

	return RESULT_COMPLETE;
}


/* This function implements the character-search commands: f, t, F, T, comma,
 * and semicolon.
 */
RESULT	m_csearch(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	static CHAR	prevcmd;	/* previous command, for <,> and <;> */
	static CHAR	prevtarget;	/* previous target character */
	CHAR		*cp;		/* used for scanning text */

	DEFAULT(1);

	assert(strchr("fFtT,;", vinf->command));

	/* comma and semicolon recall the previous character search */
	if (vinf->command == ';' || vinf->command == ',')
	{
		/* fail if there was no previous command */
		if (!prevcmd)
		{
			msg(MSG_ERROR, "no previous char search");
			return RESULT_ERROR;
		}

		/* use the previous command, or its opposite */
		if (vinf->command == ';')
		{
			vinf->command = prevcmd;
		}
		else if (isupper(prevcmd))
		{
			vinf->command = tolower(prevcmd);
		}
		else
		{
			vinf->command = toupper(prevcmd);
		}

		/* use the previous target character, too */
		vinf->key2 = prevtarget;
	}
	else /* not comma or semicolon */
	{
		/* remember this command so it can be repeated later */
		prevcmd = vinf->command;
		prevtarget = vinf->key2;
	}

	/* Which way should we scan?  Forward or backward? */
	if (islower(vinf->command))
	{
		/* scan forward */
		for (scanalloc(&cp, win->state->cursor);
		     vinf->count > 0 && scannext(&cp) && *cp != '\n';
		     )
		{
			if (*cp == vinf->key2)
			{
				vinf->count--;
			}
		}
	}
	else
	{
		/* scan backward */
		for (scanalloc(&cp, win->state->cursor);
		     vinf->count > 0 && scanprev(&cp) && *cp != '\n';
		     )
		{
			if (*cp == vinf->key2)
			{
				vinf->count--;
			}
		}
	}

	/* if hit EOF or newline, then fail */
	if (!cp || *cp == '\n')
	{
		scanfree(&cp);
		return RESULT_ERROR;
	}

	/* if <t> or <T> then back off one character */
	if (vinf->command == 't')
	{
		scanprev(&cp);
	}
	else if (vinf->command == 'T')
	{
		scannext(&cp);
	}

	/* move the cursor */
	marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
	scanfree(&cp);
	return RESULT_COMPLETE;
}

/* This funtion moves the cursor to the next tag in the current buffer */
RESULT m_tag(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	next;	/* where the next tag is located */

	/* This only works when editing the main stratum */
	if (win->state->acton)
		return RESULT_ERROR;

	/* If the display mode has no "next tag" function, then fail */
	if (!win->md->tagnext)
		return RESULT_ERROR;

	/* else call the "next tag" function */
	next = (*win->md->tagnext)(win->cursor);
	if (!next)
		return RESULT_ERROR;

	/* move the cursor to the next tag */
	assert(markbuffer(next) == markbuffer(win->state->cursor));
	marksetoffset(win->state->cursor, markoffset(next));
	return RESULT_COMPLETE;
}


/* This function implements [[ and { movement commands */
RESULT m_bsection(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	BUFFER	buf;	/* buffer being addressed */
	CHAR	*cp;
	CHAR	nroff[3];	/* characters after current position */
	CHAR	*codes;
	BOOLEAN	sect;		/* are we looking through section? */

	DEFAULT(1);

	assert(vinf->command == '[' || vinf->command == '{');

	/* if this is the start of a "learn" mode, do that! */
	if (vinf->command == '[' && vinf->key2 != '[')
	{
		return maplearn(vinf->key2, True);
	}

	/* search backward for a section or paragraph */
	buf = markbuffer(win->state->cursor);
	scanalloc(&cp, win->state->cursor);
	memset(nroff, 0, sizeof nroff);
	nroff[0] = (*cp == '{' ? ' ' : *cp);
	nroff[1] = '\n';
	do
	{
		/* move back one character */
		if (!scanprev(&cp))
			break;

		/* if this is a newline, look for special stuff... */
		if (*cp == '\n')
		{
			if (nroff[0] == '{')
				vinf->count--;
			else if (nroff[1] != '\n' && nroff[0] == '\n' && vinf->command == '{')
				vinf->count--;
			else if (nroff[0] == '.')
			{
				for (codes = o_sections(buf), sect = (BOOLEAN)(vinf->command == '{');
				     codes && *codes;
				     )
				{
					if (codes[0] == nroff[1] &&
						(codes[1] == nroff[2] ||
						    (!isalnum(nroff[2]) &&
							(!codes[1] ||
							    codes[1] == ' '
							)
						     )
						)
					   )
					{
						vinf->count--;
						break;
					}

					/* after section, chain to paragraph */
					if ((!codes[1] || !codes[2]) && sect)
					{
						codes = o_paragraphs(buf);
						sect = False;
					}
					else if (!codes[1])
						codes++;
					else
						codes += 2;
				}
			}
		}

		/* shift this character into "nroff" string */
		nroff[2] = nroff[1];
		nroff[1] = nroff[0];
		nroff[0] = *cp;

	} while (vinf->count > 0);

	/* At this point, cp either points to the newline before a section,
	 * or it is NULL because we hit the top of the buffer.  If it is NULL
	 * and we were only looking to go back one more section/paragraph, and
	 * the cursor wasn't already at the top of the buffer, then we should
	 * move the cursor to the top of the buffer.  Otherwise a NULL cp
	 * indicates an error.  A non-NULL cp should cause the cursor to be
	 * left after the newline that it points to.
	 */

	if (!cp && vinf->count == 1 && markoffset(win->state->cursor) != 0)
	{
		marksetoffset(win->state->cursor, 0);
	}
	else if (!cp)
	{
		scanfree(&cp);
		return RESULT_ERROR;
	}
	else
	{
		marksetoffset(win->state->cursor, markoffset(scanmark(&cp)) + 1);
	}
	scanfree(&cp);
	return RESULT_COMPLETE;
}



/* This function implements ]] and } movement commands */
RESULT m_fsection(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	BUFFER	buf;	/* buffer being addressed */
	CHAR	*cp;
	CHAR	nroff[3];	/* characters after current position */
	CHAR	*codes;
	long	offset;		/* offset of potential destination */
	BOOLEAN	sect;		/* are we looking through section? */

	DEFAULT(1);

	assert(vinf->command == ']' || vinf->command == '}');

	/* Initialize "offset" just to silence a compiler warning */
	offset = 0;

	/* if this is the end of a "learn" mode, do that! */
	if (vinf->command == ']' && vinf->key2 != ']')
	{
		return maplearn(vinf->key2, False);
	}

	/* search forward for a section or paragraph */
	buf = markbuffer(win->state->cursor);
	scanalloc(&cp, win->state->cursor);
	memset(nroff, 0, sizeof nroff);
	nroff[2] = *cp;
	nroff[1] = (*cp == '.' ? '\0' : '\n');
	do
	{
		/* move ahead one character */
		if (!scannext(&cp))
			break;

		/* look for special stuff... */
		if (nroff[2] == '\n' && *cp == '{')
		{
			offset = markoffset(scanmark(&cp));
			vinf->count--;
		}
		else if (nroff[1] != '\n' && nroff[2] == '\n' && *cp == '\n' && vinf->command == '}')
		{
			offset = markoffset(scanmark(&cp));
			vinf->count--;
		}
		else if (nroff[0] == '\n' && nroff[1] == '.')
		{
			for (codes = o_sections(buf), sect = (BOOLEAN)(vinf->command == '}');
			     codes && *codes;
			     )
			{
				if (codes[0] == nroff[2] &&
					(codes[1] == *cp ||
					    (!isalnum(*cp) &&
						(!codes[1] || codes[1] == ' ')
					    )
					)
				   )
				{
					offset = markoffset(scanmark(&cp)) - 2;
					vinf->count--;
					break;
				}

				/* after section, chain to paragraph */
				if ((!codes[1] || !codes[2]) && sect)
				{
					codes = o_paragraphs(buf);
					sect = False;
				}
				else if (!codes[1])
					codes++;
				else
					codes += 2;
			}
		}

		/* shift this character into "nroff" string */
		nroff[0] = nroff[1];
		nroff[1] = nroff[2];
		nroff[2] = *cp;

	} while (vinf->count > 0);

	/* At this point, cp either points to the last character of a section
	 * header (and "offset" is the start of that header), or cp is NULL
	 * because we hit the end of the buffer before finding a section.
	 * If it is NULL and we were only looking to go forward 1 more section,
	 * then move the cursor to the end of the buffer.  Otherwise a NULL
	 * cp indicates an error.  A non-NULL cp should cause the cursor to be
	 * left at the "offset" value.
	 */ 

	if (!cp && vinf->count == 1
		&& markoffset(win->state->cursor) < o_bufchars(buf) - 2)
	{
		/* leave cursor on the character before the final newline,
		 * unless the final line consists of only a newline character;
		 * then leave it on that newline.
		 */
		marksetoffset(win->state->cursor, o_bufchars(buf) - 2);
		if (scanchar(win->state->cursor) == '\n')
		{
			markaddoffset(win->state->cursor, 1);
		}

		/* doing a line-mode operator? */
		if (vinf->oper)
		{
			/* include the final line */
			vinf->tweak |= TWEAK_INCL;
		}
	}
	else if (!cp)
	{
		scanfree(&cp);
		return RESULT_ERROR;
	}
	else
	{
		marksetoffset(win->state->cursor, offset);
	}
	scanfree(&cp);
	return RESULT_COMPLETE;
}


/* This implements the screen-relative movement commands: H, M, and L */
RESULT m_scrnrel(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	long	delta;	/* number of lines to move forward */
	long	srcoff;	/* offset of source line */
	MARKBUF	srcbuf;	/* mark of line that we're moving relative to */
	MARK	tmp;	/* value returned by display mode's move() function */
	int	rows;	/* number of rows showing something other than "~" */

	assert(vinf->command == 'H' || vinf->command == 'M' || vinf->command == 'L');
	assert(win->di && win->di->rows > 1);

	DEFAULT(1);

	/* see how many rows are visible */
	for (rows = win->di->rows - 2;
	     rows > 1 && win->di->newrow[rows].lineoffset >= o_bufchars(markbuffer(win->state->cursor));
	     rows--)
	{
	}

	/* choose a source offset and line delta, depending on command */
	switch (vinf->command)
	{
	  case 'H':
		delta = vinf->count - 1;
		srcoff = win->di->newrow[0].lineoffset;
		break;

	  case 'M':
		delta = 0;
		srcoff = win->di->newrow[rows / 2].lineoffset;
		break;

	  default: /* 'L' */
		delta = 1 - vinf->count;
		srcoff = win->di->newrow[rows].lineoffset;
		break;
	}

	/* if bad offset, then fail */
	if (srcoff < 0 || srcoff >= o_bufchars(markbuffer(win->state->cursor)))
		return RESULT_ERROR;

	/* maybe move forward or backward from that line */
	if (delta != 0)
	{
		(void)marktmp(srcbuf, markbuffer(win->state->cursor), srcoff);
		tmp = (*win->md->move)(win, &srcbuf, delta, 0, False);
		if (!tmp)
			return RESULT_ERROR;
		srcoff = markoffset(tmp);
	}

	/* move the cursor to that line */
	marksetoffset(win->state->cursor, srcoff);
	return RESULT_COMPLETE;
}


RESULT m_z(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	/* This only works on the window's primary buffer */
	if (win->state->cursor != win->cursor)
		return RESULT_ERROR;

	/* If a count is given, then move the cursor to that line */
	if (vinf->count > 0 && vinf->count < o_buflines(markbuffer(win->cursor)))
	{
		marksetoffset(win->cursor,
		    lowline(bufbufinfo(markbuffer(win->cursor)), vinf->count));
	}

	/* tweak the window's top & bottom to force the current line to
	 * appear in a given location on the screen.
	 */
	switch (vinf->key2)
	{
	  case '\n':
	  case '\r':
	  case '+':
		/* The current line should appear at the top of the screen.
		 * We'll tweak the top & bottom so they both refer to this
		 * line.  When the window is redrawn, the redrawing logic
		 * will cause this line to be output first, and then it'll
		 * just continue showing lines until the bottom of the
		 * screen.
		 */
		win->di->topline = markoffset(dispmove(win, 0L, 0));
		win->di->bottomline = o_bufchars(markbuffer(win->cursor));
		win->di->logic = DRAW_CHANGED;
		break;

	  case '.':
	  case 'z':
		/* The current line should appear in the middle of the screen.
		 * To do this, we'll set the top half a screenful's number of
		 * lines back, and the bottom some point after the cursor.
		 * We'll also set the drawing logic to perform slop-scrolling
		 * until the cursor is in the top half of the screen, just in
		 * case the lines at the top of the screen fill more than one
		 * row.
		 */
		win->di->topline = markoffset(dispmove(win, -(o_lines(win) / 2), 0));
		win->di->bottomline = o_bufchars(markbuffer(win->cursor));
		win->di->logic = DRAW_CENTER;
		break;

	  case '-':
		/* The current line should appear at the bottom of the screen.
		 * To do this, we'll set the top back a whole screenload's
		 * number of lines before the cursor line, and the bottom 
		 * to some point after the current line.  When the window is
		 * redrawn, the drawing logic will start drawing from the
		 * computed top, and then scroll the window if necessary to
		 * bring the current line onto the screen.
		 */
		win->di->topline = markoffset(dispmove(win, -o_lines(win), 0));
		win->di->bottomline = markoffset(win->cursor) + 1;
		win->di->logic = DRAW_CHANGED;
		break;
	}

	return RESULT_COMPLETE;
}


RESULT m_fsentence(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	BOOLEAN	ending;	/* have we seen at least one sentence ender? */
	BOOLEAN	didpara;/* between paragraph and first sentence in paragraph */
	int	spaces;	/* number of spaces seen so far */
	CHAR	*cp;	/* used for scanning through text */
	CHAR	*end;	/* characters that end a sentence */
	CHAR	*quote;	/* quote/parenthesis character that may appear at end */
	CHAR	oper;	/* operator command, or '\0' */
	long	newline;/* offset of first newline in trailing whitespace */
	long	para;
	long	offset;
	long	count;

	DEFAULT(1);

	/* If sentenceend and sentencequote are unset, use default values */
	end = o_sentenceend ? o_sentenceend : toCHAR(".?!");
	quote = o_sentencequote ? o_sentencequote : toCHAR("\")]");

	count = vinf->count;
	oper = vinf->oper;

	/* detect whether we're at the start of a paragraph */
	offset = markoffset(win->state->cursor);
	scanalloc(&cp, win->state->cursor);
	if (*cp != '\n')
		scanprev(&cp);
	else
		while (cp && *cp == '\n')
		{
			scanprev(&cp);
		}
	if (cp)
	{
		marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
		vinf->count = 1;
		vinf->command = '}';
		m_fsection(win, vinf);
		para = markoffset(win->state->cursor);
	}
	else
	{
		para = 0;
	}
	marksetoffset(win->state->cursor, offset);
	if (para == offset)
		count++;
	else if (para < offset)
	{
		/* find the next paragraph */
		marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
		vinf->count = 1;
		vinf->command = '}';
		m_fsection(win, vinf);
		para = markoffset(win->state->cursor);
	}

	/* for each count... */
	newline = -1;
	scanseek(&cp, win->state->cursor);
	for (; cp && count > 0; count--)
	{
		/* for each character in the sentence... */
		for (ending = didpara = False, spaces = 0;
		     cp && (!ending || spaces < o_sentencegap || isspace(*cp));
		     scannext(&cp), offset++)
		{
			/* if paragraph, then... */
			if (offset == para)
			{
				/* if still more sentences to skip... */
				if (count > 1)
				{
					/* count this as a sentence, and
					 * arrange for next line to also be a
					 * sentence.
					 */
					count--;
					newline = -1;
					ending = True;
					if (*cp == '\n')
					{
						didpara = False;
						spaces = o_sentencegap;
					}
					else
					{
						didpara = True;
						spaces = 0;
					}

					/* oh, and we need to find the next
					 * paragraph, too.
					 */
					marksetoffset(win->state->cursor, offset);
					vinf->count = 1;
					vinf->command = '}';/*{*/
					if (m_fsection(win, vinf) == RESULT_COMPLETE)
						para = markoffset(win->state->cursor);
					else
						para = -1;

					continue;
				}
				else
				{
					/* we're here! */
					break;
				}
			}

			/* check the character */
			if (*cp == '\n')
			{
				spaces = o_sentencegap;
				didpara = False;
				if (newline < 0)
					newline = markoffset(scanmark(&cp));
			}
			else if (didpara)
				/* skip characters in a ".P" line */;
			else if (isspace(*cp))
				spaces++;
			else if (CHARchr(end, *cp))
				newline = -1, ending = True, spaces = 0;
			else if (!CHARchr(quote, *cp))
				newline = -1, ending = False, spaces = 0;
			else /* quote character */
				newline = -1;
		}
	}

	/* did we find it? */
	if (cp)
	{
		/* if target of operator, and a newline was encountered in the
		 * trailing whitespace, then move the cursor to that newline;
		 * else move the cursor to the start of the following sentence.
		 */
		if (oper && newline >= 0)
			marksetoffset(win->state->cursor, newline);
		else
			marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
	}
	scanfree(&cp);
	return cp ? RESULT_COMPLETE : RESULT_ERROR;
}

RESULT m_bsentence(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	BOOLEAN	first;	/* True until we pass some mid-sentence stuff */
	BOOLEAN	ending;	/* have we seen at least one sentence ender? */
	BOOLEAN	anynext;/* any text seen on following line */
	BOOLEAN	anythis;/* any text seen on this line */
	int	spaces;	/* number of spaces seen so far */
	CHAR	*cp;	/* used for scanning through text */
	CHAR	*end;	/* characters that end a sentence */
	CHAR	*quote;	/* quote/parenthesis character that may appear at end */
	long	count;	/* sentences to move over */
	long	para;	/* top of current paragraph */
	long	offset;
	RESULT	result;

	DEFAULT(1);

	/* If sentenceend and sentencequote are unset, use default values */
	end = o_sentenceend ? o_sentenceend : toCHAR(".?!");
	quote = o_sentencequote ? o_sentencequote : toCHAR("\")]");

	/* misc initialization */
	anynext = anythis = False;
	offset = markoffset(win->state->cursor);

	/* NOTE: The "first" variable is used to handle the situation where
	 * we start at the beginning of one sentence.  From here, we want
	 * to go back one extra sentence-end; otherwise <(> would just move
	 * us to the start of the same sentence.
	 */
	first = True;

	/* Start scanning at the cursor location */
	scanalloc(&cp, win->state->cursor);
	count = vinf->count;

	/* Find the start of this paragraph.  That counts as a "sentence" */
	vinf->command = '{';
	vinf->count = 1;
	para = (m_bsection(win, vinf) == RESULT_COMPLETE) ? markoffset(win->state->cursor) : -1;

	/* For each count... */
	for (; cp && count > 0; count--)
	{
		/* for each character in the sentence... */
		for (ending = True, anythis = anynext = False, spaces = 0,
			scanprev(&cp), offset = markoffset(scanmark(&cp));
		     cp && offset != para &&
			(!ending || spaces<o_sentencegap || !CHARchr(end,*cp));
		     scanprev(&cp), offset--)
		{
			if (*cp == '\n')
			{
				spaces = o_sentencegap;
				ending = True;
				anynext = anythis;
				anythis = False;
			}
			else if (isspace(*cp))
			{
				spaces++,
				ending = True;
			}
			else if (!CHARchr(quote, *cp))
			{
				first = ending = False;
				anythis = True;
				spaces = 0;
			}
			else
			{
				anythis = True;
			}
		}

		/* If this sentence is actually a paragraph start, and we
		 * still have more sentences to move over, then find the
		 * next higher paragraph.
		 */
		if (offset == para && count > 1)
		{
			vinf->count = 1;
			para = (m_bsection(win, vinf) == RESULT_COMPLETE) ? markoffset(win->state->cursor) : -1;
		}

		/* If this sentence ender was encountered before any
		 * mid-sentence text (i.e., if we started at the front of
		 * a sentence) then we should go back one extra sentence-end.
		 */
		if (first && offset != para)
		{
			count++;
		}
	}

	/* If we hit a paragraph top which occurs on a blank line, then we
	 * need to do a little extra processing because we exited the loop
	 * a little too soon.
	 */
	if (offset == para && cp && *cp == '\n')
	{
		anynext = anythis;
	}

	/* did we find it? */
	if (cp)
	{
		/* move the cursor to the start of the next sentence */
		if (offset > para)
		{
			/* found a sentence end -- move the cursor to the to
			 * start of the following sentence.
			 */
			do
			{
				scannext(&cp);
				assert(cp);
			} while (isspace(*cp) || CHARchr(quote, *cp));
			marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
		}
		else if (anynext)
		{
			/* bumped into a paragraph start after scanning some
			 * sentence text -- move the cursor to the first text
			 * on the next non-blank line.
			 */
			while (cp && *cp != '\n')
			{
				scannext(&cp);
			}
			while (cp && isspace(*cp))
			{
				scannext(&cp);
			}
			if (&cp)
				marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
			else
			{
				marksetoffset(win->state->cursor, o_bufchars(markbuffer(win->state->cursor)) - 2);
				if (markoffset(win->state->cursor) < 0)
					marksetoffset(win->state->cursor, 0);
			}
		}
		else
		{
			/* found a paragraph start -- leave the cursor there */
			offset = para;
			marksetoffset(win->state->cursor, para);
		}
		result = RESULT_COMPLETE;
	}
	else if (count == 0)
	{
		/* move the cursor to the first character of the buffer */
		marksetoffset(win->state->cursor, 0);
		result = RESULT_COMPLETE;
	}
	else
	{
		/* tried to move past beginning of buffer */
		result = RESULT_ERROR;
	}
	scanfree(&cp);
	return result;
}
