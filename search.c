/* search.c */
/* Copyright 1995 by Steve Kirkendall */

char id_search[] = "$Id: search.c,v 2.17 1996/07/17 01:32:27 steve Exp $";

#include "elvis.h"

#if USE_PROTOTYPES
static RESULT searchenter(WINDOW win);
static RESULT forward(WINDOW win);
static RESULT backward(WINDOW win);
#endif


#define REGEXSIZE	150

/* This is the previous regular expression.  We need to remember it after
 * it has been used, so we can implement the <n> and <shift-N> commands.
 */
regexp	*searchre;	/* the regular expression itself */
BOOLEAN searchforward;	/* is the searching being done in a forward direction? */
BOOLEAN	searchhasdelta;	/* is the regular expression followed by a line delta? */
long	searchdelta;	/* if searchhasdelta, then this is the line offset value */

/* This function is called when the user hits <Enter> after typing in a
 * regular expression for the visual </> and <?> commands.  It compiles
 * the regular expression, and then matches it in either a forward or
 * backward direction.
 */
static RESULT	searchenter(win)
	WINDOW	win;	/* window where a regexp search line has been entered */
{
	CHAR	regex[REGEXSIZE];	/* holds regular expression, as string */
	CHAR	delim;			/* delimiter, either '?' or '/' */
	CHAR	*cp;			/* used when copying */
	STATE	*state = win->state;
	int	i;

	assert(markoffset(state->top) <= markoffset(state->cursor));
	assert(markoffset(state->cursor) <= markoffset(state->bottom));

	/* copy the command line into a buffer */
	for (scanalloc(&cp, state->top), i = 0;
	     cp != NULL && i < markoffset(state->bottom) - markoffset(state->top);
	     scannext(&cp), i++)
	{
		regex[i] = *cp;
	}
	regex[i] = '\0';
	scanfree(&cp);

	/* skip leading '/' or '?' character, but remember which */
	delim = regex[0];
	if (delim != '/' && delim != '?')
	{
		msg(MSG_ERROR, "bad search");
		return RESULT_ERROR;
	}

	/* locate terminating delimiter, if any */
	for (cp = &regex[1]; *cp && *cp != delim; cp++)
	{
		/* skip backslash-quoted characters */
		if (*cp == '\\' && *(cp + 1))
		{
			cp++;
		}
	}

	/* If there was a terminator, then mark the end of the regular
	 * expression with a '\0' and leave cp pointing to whatever came
	 * after the regular expression.
	 */
	if (*cp)
	{
		*cp++ = '\0';
	}

	/* Compile the regular expression.  An empty regular expression is
	 * identical to the current regular expression, so we don't need to
	 * compile it in that case.  If there are any errors, then fail.
	 * The regcomp function will have already output an error message.
	 */
	if (regex[1])
	{
		/* free the previous regular expression, if any */
		if (searchre)
		{
			safefree((void *)searchre);
		}

		/* compile the new one */
		searchre = regcomp(&regex[1], win->cursor);
	}
	else if (!searchre)
	{
		msg(MSG_ERROR, "no previous RE");
	}
	if (!searchre)
	{
		return RESULT_ERROR;
	}

	/* '/' implies forward search, '?' implies backward search */
	searchforward = (BOOLEAN)(delim == '/');

	/* Check for 'v' to force "autoselect" on, or 'n' to force it off.
	 * Also force it off if it was turned on by a previous 'v' but never
	 * explicitly addressed by the :set command, and there is no 'v'.
	 */
	switch (*cp)
	{
	  case 'v':
		o_autoselect = True;
		cp++;
		break;

	  case 'n':
		o_autoselect = False;
		cp++;
		break;

	  default:
		if ((optflags(o_autoselect) & OPT_SET) == 0)
		{
			o_autoselect = False;
		}
	}

	/* check for a line delta after the regular expression */
	if (*cp == '+' || *cp == '-')
	{
		searchhasdelta = True;
		searchdelta = atol(tochar8(cp));
	}
	else
	{
		searchhasdelta = False;
	}

	/* Now we can do this search exactly like an <n> search command, except
	 * that it'll be applied to the previous stratum instead of this one.
	 */
	return RESULT_COMPLETE;
}

/* search forward from the cursor position for the previously compiled
 * regular expression.
 */
static RESULT forward(win)
	WINDOW	win;	/* the window to search in */
{
	MARKBUF	sbuf;	/* buffer, holds search mark */
	long	lnum;	/* line number -- used for scanning through buffer */
	BLKNO	bi;	/* bufinfo block of the buffer being searched */
	BOOLEAN	bol;	/* are we at the beginning of a line? */
	BOOLEAN	wrapped;/* True if we've wrapped at the end of the buffer */
	long	start;	/* line where we started (detects failure after wrap) */

	/* setup: determine the line number, whether the cursor is at the
	 * beginning of the line, and where the buffer ends.
	 */
	sbuf = *win->state->cursor;
	if (markoffset(&sbuf) + 1 >= o_bufchars(markbuffer(&sbuf)))
		marksetoffset(&sbuf, 0L);
	else
		markaddoffset(&sbuf, 1);
	bi = bufbufinfo(markbuffer(&sbuf));
	lnum = markline(&sbuf);
	bol = (BOOLEAN)(lowline(bi, lnum) == markoffset(&sbuf));
	wrapped = False;
	start = lnum;

	/* search */
	for (;
	     !regexec(searchre, &sbuf, bol);
	     marksetoffset(&sbuf, searchre->nextlinep), bol = True)
	{
		/* If user gives up, then fail */
		if (guipoll(False))
		{
			return RESULT_ERROR;
		}

		/* Advance to next line.  Maybe wrap at the bottom of the
		 * buffer.  This is also where failed searches are detected.
		 */
		lnum++;
		if (wrapped && lnum > start)
		{
			msg(MSG_ERROR, "no match");
			return RESULT_ERROR;
		}
		else if (lnum > o_buflines(markbuffer(&sbuf)))
		{
			if (o_wrapscan)
			{
				lnum = 1;
				searchre->nextlinep = 0L;
				wrapped = True;
			}
			else
			{
				msg(MSG_ERROR, "no match below");
				return RESULT_ERROR;
			}
		}
	}

	/* if we get here, then the search succeded */
	assert(searchre->leavep >= 0);
	marksetoffset(win->state->cursor, searchre->leavep);
	if (wrapped)
		msg(MSG_STATUS, "wrapped");
	return RESULT_COMPLETE;
}

/* search backward from the cursor position for the previously compiled
 * regular expression.
 */
static RESULT backward(win)
	WINDOW	win;	/* the window to search in */
{
	MARKBUF	sbuf;	/* buffer, holds search mark */
	BLKNO	bi;	/* bufinfo block of the buffer being searched */
	long	lnum;	/* line number -- used for scanning through buffer */
	BOOLEAN	wrapped;/* True if we've wrapped at the end of the buffer */
	long	start;	/* line where we started (detects failure after wrap) */
	long	endln;	/* offset of the end of a line (really start of next line) */
	long	last;	/* offset of last match found in a line */
	long	laststartp;/* offset of the start of the last match in a line */
	long	lastendp;/* offset of the end of the last match in a line */
	CHAR	*cp;	/* used for scanning for newline characters */

	/* setup: determine the line number, whether the cursor is at the
	 * beginning of the line, and where the buffer ends.
	 */
	sbuf = *win->state->cursor;
	bi = bufbufinfo(markbuffer(&sbuf));
	lnum = markline(&sbuf);
	marksetoffset(&sbuf, lowline(bi, lnum));
	endln = (lnum == o_buflines(markbuffer(&sbuf))
		? o_bufchars(markbuffer(&sbuf)) : lowline(bi, lnum + 1));
	wrapped = False;
	start = lnum;

	/* check for match in same line, to left of cursor */
	if (regexec(searchre, &sbuf, True)
	 && searchre->leavep < markoffset(win->state->cursor))
	{
		/* find the last match in the line that occurs before the
		 * current position.
		 */
		do
		{
			last = searchre->leavep;
			laststartp = searchre->startp[0];
			lastendp = searchre->endp[0];
			marksetoffset(&sbuf, last + 1);
		} while (last + 1 < endln
			&& regexec(searchre, &sbuf, False)
			&& searchre->leavep < markoffset(win->state->cursor));
		marksetoffset(win->state->cursor, last);
		searchre->startp[0] = laststartp;
		searchre->endp[0] = lastendp;
		return RESULT_COMPLETE;
	}

	/* search for some other line with a match */
	scanalloc(&cp, &sbuf);
	do
	{
		/* If user gives up, then fail */
		if (guipoll(False))
		{
			scanfree(&cp);
			return RESULT_ERROR;
		}

		/* Advance to preceding line.  Maybe wrap at the top of the
		 * buffer.  This is also where failed searches are detected.
		 */
		lnum--;
		if (wrapped && lnum < start)
		{
			scanfree(&cp);
			msg(MSG_ERROR, "no match");
			return RESULT_ERROR;
		}
		else if (lnum < 1)
		{
			if (o_wrapscan)
			{
				lnum = o_buflines(markbuffer(&sbuf));
				endln = o_bufchars(markbuffer(&sbuf));
				wrapped = True;
			}
			else
			{
				scanfree(&cp);
				msg(MSG_ERROR, "no match above");
				return RESULT_ERROR;
			}

			/* find the start of the last line */
			marksetoffset(&sbuf, lowline(bi, lnum));
			scanseek(&cp, &sbuf);
		}
		else
		{
			endln = markoffset(&sbuf);
			scanprev(&cp);
			assert(cp && *cp == '\n');
			do
			{
				scanprev(&cp);
			} while (cp && *cp != '\n');
			if (cp)
			{
				scannext(&cp);
				sbuf = *scanmark(&cp);
			}
			else
			{
				marksetoffset(&sbuf, 0L);
				scanseek(&cp, &sbuf);
			}
		}

	} while (!regexec(searchre, &sbuf, True));
	scanfree(&cp);

	/* If we get here, then the search succeded -- meaning we have found
	 * a line which contains at least one match.  Now we need to locate
	 * the LAST match in that line.
	 */
	do
	{
		last = searchre->leavep;
		laststartp = searchre->startp[0];
		lastendp = searchre->endp[0];
		marksetoffset(&sbuf, last + 1);
	} while (last + 1 < endln
		&& regexec(searchre, &sbuf, False));

	/* move to the last match */
	marksetoffset(win->state->cursor, last);
	searchre->startp[0] = laststartp;
	searchre->endp[0] = lastendp;
	if (wrapped)
		msg(MSG_STATUS, "wrapped");
	return RESULT_COMPLETE;
}

RESULT m_search(win, vinf)
	WINDOW	win;	/* window where operation should take place */
	VIINFO	*vinf;	/* the command to execute */
{
	BOOLEAN	fwd;	/* is this search going to go forward? */
	BOOLEAN	hint;	/* show a "/" or "?" during the search? */
	RESULT	rc;	/* return code */
	VIINFO	vinfbuf;/* used for constructing a vi command for selections */

	assert(vinf->command == 'n' || vinf->command == 'N'
		|| vinf->command == '/' || vinf->command == '?'
		|| vinf->command == ELVCTRL('A'));

	/* If repeating an operator with / or ?, then use n instead.  (Since
	 * search commands don't change the buffer themselves, the only way <.>
	 * would repeat one is in conjunction with an operator.  So we don't
	 * need do explicitly check for an operator; we KNOW there is one.)
	 */
	if ((vinf->tweak & TWEAK_DOTTING) != 0
			&& (vinf->command == '/' || vinf->command == '?'))
	{
		vinf->command = 'n';
	}

	hint = False;
	switch (vinf->command)
	{
	  case 'n':
		fwd = searchforward;
		hint = True;
		break;

	  case 'N':
		/* reverse the direction of the search */
		fwd = (BOOLEAN)!searchforward;
		hint = True;
		break;

	  case ELVCTRL('A'):
		/* Free the previous regular expression, if any */
		if (searchre)
		{
			safefree((void *)searchre);
		}

		/* Compile the regexp /\<\@\>/ */
		searchre = regcomp(toCHAR("\\<\\@\\>"), win->cursor);
		if (!searchre)
			return RESULT_ERROR;

		/* always search in the forward direction */
		fwd = searchforward = True;
		searchhasdelta = False;
		searchdelta = 0L;
		hint = True;
		break;

	  default:
		/* There are two modes of operation here.  If the "more"
		 * flag is not set, then we need to push a new stratum for
		 * reading a regular expression to search for.  If "more"
		 * is set, then we act just like 'n'.
		 */
		if (win->state->flags & ELVIS_MORE)
		{
			win->state->flags &= ~ELVIS_MORE;
			fwd = searchforward;
			break;
		}
		statestratum(win, toCHAR(REGEXP_BUF), vinf->command, searchenter);
		o_internal(markbuffer(win->state->cursor)) = True;
		return RESULT_MORE;
	}

	/* If we get here, we need to do an 'n' search */

	/* This'll only work if we have a regexp */
	if (!searchre)
	{
		msg(MSG_ERROR, "no previous search");
		return RESULT_ERROR;
	}

	/* give a hint that we're searching, if necessary */
	if (hint)
	{
		msg(MSG_STATUS, fwd ? "/" : "?");
	}

	/* do the search */
	if (fwd)
	{
		rc = forward(win);
	}
	else
	{
		rc = backward(win);
	}

	/* if the search was successful, take care of autoselect & line delta */
	if (rc == RESULT_COMPLETE)
	{
		/* autoselect, maybe with line delta */
		if (o_autoselect)
		{
			/* Cancel any previous selection */
			vinfbuf.command = ELVCTRL('[');
			(void)v_visible(win, &vinfbuf);

			/* Move to the end of the matched text, and start
			 * marking characters from there.  Note that since
			 * visible selections include the last character, and
			 * endp[0] contains the offset of the first character
			 * AFTER the matched text, we usually want to subtract
			 * 1 from endp[0].  However, if the matching text is
			 * is zero-length, we select nothing and give a warning.
			 */
			if (searchre->startp[0] < searchre->endp[0])
			{
				marksetoffset(win->state->cursor, searchre->endp[0] - 1);
				vinfbuf.command = (searchhasdelta ? 'V' : 'v');
				(void)v_visible(win, &vinfbuf);
			}
			else if (searchhasdelta)
			{
				marksetoffset(win->state->cursor, searchre->endp[0] - 1);
				vinfbuf.command = (searchhasdelta ? 'V' : 'v');
				(void)v_visible(win, &vinfbuf);
			}
			else
			{
				msg(MSG_INFO, "match is zero-length");
			}

			/* Move back to the beginning of the matched text,
			 * stretching the selected region to follow the cursor.
			 * This is also a good time to factor in the line-delta
			 * if any.
			 */
			if (searchhasdelta)
			{
				/* non-zero deltas move the cursor to the front
				 * of a relative line.
				 */
				if (searchdelta != 0)
				{
					/* move the cursor some number of whole lines */
					marksetoffset(win->state->cursor,
						markoffset(dispmove(win, searchdelta, 0)));

					/* leave the cursor at the front of that line */
					vinf->tweak |= TWEAK_FRONT;
				}
				(void)v_visible(win, NULL);
			}
			else if (searchre->startp[0] < searchre->endp[0])
			{
				marksetoffset(win->state->cursor, searchre->startp[0]);
				(void)v_visible(win, NULL);
			}
			/* else we already gave a warning instead of selecting text */
		}
		else
		/* no autoselect, but maybe still a line delta */
		if (searchhasdelta)
		{
			/* All line deltas have the side effect of making the
			 * regexp search command be a line-oriented command
			 * instead of a character-oriented command.
			 */
			vinf->tweak |= TWEAK_LINE;

			/* non-zero deltas move the cursor to the front
			 * of a relative line.
			 */
			if (searchdelta != 0)
			{
				/* move the cursor some number of whole lines */
				marksetoffset(win->state->cursor,
					markoffset(dispmove(win, searchdelta, 0)));

				/* leave the cursor at the front of that line */
				vinf->tweak |= TWEAK_FRONT;
			}
		}
	}

	return rc;
}
