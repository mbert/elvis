/* vicmd.c */
/* Copyright 1995 by Steve Kirkendall */

char id_vicmd[] = "$Id: vicmd.c,v 2.44 1996/09/18 19:16:11 steve Exp $";

#include "elvis.h"



/* This implements the visual @x command, which uses the contents of a
 * cut buffer as simulated keystrokes.
 */
RESULT v_at(win, vinf)
	WINDOW	win;
	VIINFO	*vinf;
{
	CHAR	*keys;

	/* if this buffer is in learn mode, then stop it */
	if (isalpha(vinf->key2))
		maplearn(vinf->key2, False);

	/* copy the cut buffer contents into memory */
	keys = cutmemory(vinf->key2);
	if (!keys)
		return RESULT_ERROR;

	/* Treat the cut buffer contents like keystrokes. */
	mapunget(keys, (int)CHARlen(keys), True);
	safefree(keys);

	return RESULT_COMPLETE;
}


/* This function implements the <Z><Z> and <Z><Q> commands */
RESULT v_quit(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	RESULT result = RESULT_COMPLETE;

	switch (vinf->key2)
	{
	  case 'Z':
		result = exstring(win, toCHAR("x"));
		break;

	  case 'Q':
		result = exstring(win, toCHAR("q!"));
		break;
	}

	return result;
}

/* This function implements the <Control-R> function.  It does this simply by
 * setting the window's "logic" flag to DRAW_SCRATCH.
 */
RESULT v_expose(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	/* reset the GUI, to bypass any optimizations */
	guireset();

	/* Force the screen to be redrawn from scratch next time */
	win->di->logic = DRAW_SCRATCH;

	return RESULT_COMPLETE;
}


/* This function implements the insert/replace commands. */
RESULT v_input(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	tmp;
	long	offset;
	long	topoff;
	CHAR	newline[1];
	char	cmd;
	BUFFER	dotbuf;
	RESULT	result = RESULT_COMPLETE;

	DEFAULT(1);

	/* if given as a ^O command, this can only toggle input/replace mode */
	if (win->state->flags & ELVIS_ONCE)
	{
		/* we aren't the original vi state -- modify the
		 * input/replace flag of the input state that we
		 * came from.
		 */
		inputtoggle(win, (char)(vinf->command == 'R' ? 'R' : 't'));
		return RESULT_COMPLETE;
	}

	/* If this is a "more" invocation (used to come back to this function
	 * after the user inputs text, if count>1) then pretend that this is
	 * a <.> command with a count of one less than the actual count.
	 */
	cmd = vinf->command;
	if (win->state->flags & ELVIS_MORE)
	{
		vinf->tweak |= TWEAK_DOTTING;
		if (cmd == 'i')
		{
			cmd = 'a';
		}
	}

	/* if the buffer is empty, then the only legal command is <Shift-O> */
	if (o_bufchars(markbuffer(win->state->cursor)) == 0)
	{
		cmd = 'O';
	}

	/* some variations of input require preparation */
	topoff = -1;
	switch (cmd)
	{
	  case 'a':
		/* if not zero-length line, then move left 1 char */
		tmp = dispmove(win, 0, 0);
		if (scanchar(tmp) != '\n')
		{
			markaddoffset(win->state->cursor, 1);
		}
		break;

	  case 'A':
		/* go to end of line, and start inserting there */
		tmp = dispmove(win, 0, INFINITY);
		marksetoffset(win->state->cursor, markoffset(tmp));
		if (scanchar(tmp) != '\n')
		{
			markaddoffset(win->state->cursor, 1);
		}
		break;

	  case 'I':
		/* go to the front of the line */
		m_front(win, vinf);
		break;

	  case 'O':
		/* insert a new line before this one */
		tmp = dispmove(win, 0, 0);
		newline[0] = '\n';
		bufreplace(tmp, tmp, newline, 1);
		topoff = markoffset(tmp);
		marksetoffset(win->state->cursor, topoff);
		if ((vinf->tweak & TWEAK_DOTTING) == 0)
		{
			dispindent(win, win->state->cursor, 1);
		}
		break;

	  case 'o':
		/* insert a new line after this one */
		tmp = dispmove(win, 0, INFINITY);
		if (scanchar(tmp) != '\n')
		{
			markaddoffset(tmp, 1);
		}
		newline[0] = '\n';
		bufreplace(tmp, tmp, newline, 1);
		topoff = markoffset(tmp) + 1;
		marksetoffset(win->state->cursor, topoff);
		if ((vinf->tweak & TWEAK_DOTTING) == 0)
		{
			dispindent(win, win->state->cursor, -1);
		}
		break;

	  /* 'i' and 'R' need no special preparation */
	}

	/* shrink the current segment around the cursor */
	offset = markoffset(win->state->cursor);
	marksetoffset(win->state->top, topoff >= 0 ? topoff : offset);
	marksetoffset(win->state->bottom, offset);

	/* if we're doing a <.> command, then don't do interactive stuff.
	 * Instead, just paste a copy of the previous input.
	 */
	if (vinf->tweak & TWEAK_DOTTING)
	{
		/* If R command, then delete old characters */
		if (cmd == 'R')
		{
			dotbuf = cutbuffer('.', False);
			if (dotbuf && o_bufchars(dotbuf) > CUT_TYPELEN)
			{
				offset = vinf->count * (o_bufchars(dotbuf) - CUT_TYPELEN)
					+ markoffset(win->state->cursor);
				tmp = dispmove(win, 0L, INFINITY);
				if (offset > markoffset(tmp))
				{
					offset = markoffset(tmp);
					if (scanchar(tmp) != '\n')
					{
						offset++;
					}
				}
				marksetoffset(tmp, offset);
				bufreplace(win->state->cursor, tmp, NULL, 0);
			}
		}

		/* insert copies of the previous text */
		tmp = win->state->cursor;
		do
		{
			tmp = cutput('.', win, tmp, False, True, True);
			if (!tmp) return RESULT_ERROR;
			markaddoffset(tmp, 1);
		} while (--vinf->count > 0);
		markaddoffset(tmp, -1);
		marksetoffset(win->state->cursor, markoffset(tmp));
	}
	else /* not doing <.> */
	{
		/* really go into input mode */
		inputpush(win, 0, cmd);

		/* if we have more copies to input, remember that */
		result = (--vinf->count > 0) ? RESULT_MORE : RESULT_COMPLETE;
	}

	return result;
}


/* set a named mark */
RESULT v_setmark(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	/* check mark name */
	if (vinf->key2 < 'a' || vinf->key2 > 'z')
	{
		return RESULT_ERROR;
	}

	/* if mark already set, then free its old value. */
	if (namedmark[vinf->key2 - 'a'])
	{
		markfree(namedmark[vinf->key2 - 'a']);
	}

	/* set the mark */
	namedmark[vinf->key2 - 'a'] = markdup(win->state->cursor);
	return RESULT_COMPLETE;
}

/* undo a change */
RESULT v_undo(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	long	offset;

	/* choose an appropriate default */
	DEFAULT(vinf->command == 'U' ? 0 : 1 );

	/* if redo, then negate the undo level */
	if (vinf->command == ELVCTRL('R'))
		vinf->count = -vinf->count;

	/* try to switch to the undo version */
	offset = bufundo(win->state->cursor, vinf->count);

	/* did we succeed? */
	if (offset >= 0)
	{
		/* yes! move the cursor to the position of the undone change */
		assert(offset <= o_bufchars(markbuffer(win->state->cursor)));
		marksetoffset(win->state->cursor, offset);
		return RESULT_COMPLETE;
	}
	else
	{
		/* no, failed */
		return RESULT_ERROR;
	}
}


/* Delete/replace characters from the current line.  This implements the <x>,
 * <Shift-X>, <r>c and <~> commands.
 */
RESULT v_delchar(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	long	front, end;
	MARKBUF	tmp;
	long	curs;
	CHAR	*repstr;
	long	replen;
	CHAR	*cp;
	long	i;
	long	travel;

	DEFAULT(1);

	/* Find the endpoints of this line.  Note that we need to be careful
	 * about the end of the line, because we don't want to allow the
	 * newline character to be deleted.
	 */
	front = markoffset(dispmove(win, 0, 0));
	if (win->state->acton)
	{
		end = markoffset((*dmnormal.move)(win, win->state->cursor, 0, INFINITY, True));
	}
	else
	{
		end = markoffset((*win->md->move)(win, win->state->cursor, 0, INFINITY, True));
	}

	/* choose a starting offset */
	if (vinf->command == 'X')
	{
		curs = markoffset(win->state->cursor) - vinf->count;
	}
	else
	{
		curs = markoffset(win->state->cursor);
		if (curs + vinf->count > end)
		{
			/* this may be a zero-length line.  Check! */
			if (front == end && scanchar(win->state->cursor) == '\n')
			{
				return RESULT_ERROR;
			}

			/* nope, okay to delete */
			curs = end - vinf->count + 1;
		}
	}

	/* construct a replacement string */
	if (vinf->command == 'r')
	{
		if (vinf->key2 == '\r')
		{
			vinf->key2 = '\n';
			travel = replen = 1;
		}
		else
		{
			replen = vinf->count;
			travel = replen - 1;
		}
		repstr = safealloc((int)replen, sizeof(CHAR));
		for (i = 0; i < replen; i++)
		{
			repstr[i] = vinf->key2;
		}
	}
	else if (vinf->command == '~')
	{
		replen = vinf->count;
		repstr = safealloc((int)replen, sizeof(CHAR));
		travel = replen;
		for (i = 0, scanalloc(&cp, marktmp(tmp, markbuffer(win->state->cursor), curs));
		     i < replen; i++, scannext(&cp))
		{
			if (isupper(*cp))
			{
				repstr[i] = tolower(*cp);
			}
			else if (islower(*cp))
			{
				repstr[i] = toupper(*cp);
			}
			else
			{
				repstr[i] = *cp;
			}
		}
		scanfree(&cp);
	}
	else
	{
		/* we'll just delete the chars */
		repstr = NULL;
		replen = 0;
		travel = (curs + vinf->count  - 1 == end && front != curs) ? -1 : 0;
	}

	/* if the starting offset is on a different line, fail */
	if (curs < front)
	{
		return RESULT_ERROR;
	}

	/* else move the cursor & replace/delete the characters */
	marksetoffset(win->state->cursor, curs);
	cutyank(vinf->cutbuf, win->state->cursor,
		marktmp(tmp, markbuffer(win->state->cursor), curs + vinf->count),
		'c', False);
	bufreplace(win->state->cursor, &tmp, repstr, replen);

	/* if a replacement string was allocated, free it now */
	if (repstr)
	{
		safefree(repstr);
	}

	/* move to the right (maybe) */
	markaddoffset(win->state->cursor, travel);

	/* if <r><Enter>, then worry about autoindent */
	if (vinf->command == 'r' && vinf->key2 == '\n')
	{
		dispindent(win, win->state->cursor, -replen);
	}

	return RESULT_COMPLETE;
}


/* This function calls the GUI's "tabcmd" function, if it has one. */
RESULT v_window(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	WINDOW	next;

	switch (vinf->key2)
	{
	  case 'w':
	  case ELVCTRL('W'):
		/* were we given a window number? */
		if (vinf->count == 0)
		{
			/* go to window after this one */
			next = winofbuf(win, NULL);
			if (!next)
			{
				next = winofbuf(NULL, NULL);
			}
		}
		else
		{
			/* go to window number 'n' */
			for (next = winofbuf(NULL, NULL);
			     next && o_windowid(next) != vinf->count; /* nishi */
			     next = winofbuf(next, NULL))
			{
			}
		}
		break;

	  case 'k':
		/* move up 1 window */
		for (next = NULL; winofbuf(next, NULL) != win; )
		{
			next = winofbuf(next, NULL);
			if (!next)
				break;
		}
		break;

	  case 'j':
		/* move down 1 window */
		next = winofbuf(win, NULL);
		break;

	  case 's':
		return exstring(win, toCHAR("split"));

	  case 'n':
		return exstring(win, toCHAR("snew"));

	  case 'q':
		return exstring(win, toCHAR("xit"));

	  case 'c':
		return exstring(win, toCHAR("close"));

	  case ']':
	  case ELVCTRL(']'):
		/* Perform a tag lookup.  The v_tag function is clever enough
		 * to realize that this is the splitting style of tag lookup.
		 */
		return v_tag(win, vinf);

	  case 'd':
		if (strcmp(tochar8(o_display(win)), "normal"))
			dispset(win, "normal");
		else if (!strcmp(tochar8(o_bufdisplay(markbuffer(win->cursor))), "normal"))
			dispset(win, "hex");
		else
			dispset(win, tochar8(o_bufdisplay(markbuffer(win->cursor))));
		win->di->logic = DRAW_CHANGED;
		return RESULT_COMPLETE;

	  case 'S':
		o_wrap(win) = (BOOLEAN)!o_wrap(win);
		win->di->logic = DRAW_CHANGED;
		return RESULT_COMPLETE;

	  default:
		/* run the GUI's tabcmd function.  If it doesn't have one, or
		 * if it has one but it returns False, then fail.
		 */
		if (!gui->tabcmd || !(*gui->tabcmd)(win->gw, vinf->key2, vinf->count))
		{
			return RESULT_ERROR;
		}
		return RESULT_COMPLETE;
	}

	/* did we find a window? */
	if (!next)
	{
		msg(MSG_ERROR, "no such window");
		return RESULT_ERROR;
	}

	/* go to the requested window */
	if (gui->focusgw)
	{
		(*gui->focusgw)(next->gw);
	}
	else
	{
		eventfocus(next->gw);
	}
	return RESULT_COMPLETE;
}


/* This function implements the visual commands which deal with tags */
RESULT v_tag(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	CHAR	cmd[200];
	CHAR	*tagname;

	assert(vinf->command == ELVCTRL('T') || vinf->command == ELVCTRL(']')
		|| vinf->command == ELVCTRL('W'));

	/* These commands only work on the first stratum */
	if (win->state->acton)
	{
		msg(MSG_ERROR, "only works on window's default buffer");
		return RESULT_ERROR;
	}

	/* construct the command */
	switch (vinf->command)
	{
	  case ELVCTRL('T'):
		CHARcpy(cmd, toCHAR("pop"));
		break;

	  case ELVCTRL('W'):
		tagname = (*win->md->tagatcursor)(win, win->cursor);
		if (!tagname) return RESULT_ERROR;
		CHARcpy(cmd, toCHAR("stag "));
		CHARcat(cmd, tagname);
		safefree(tagname);
		break;

	  case ELVCTRL(']'):
		tagname = (*win->md->tagatcursor)(win, win->cursor);
		if (!tagname) return RESULT_ERROR;
		CHARcpy(cmd, toCHAR("tag "));
		CHARcat(cmd, tagname);
		safefree(tagname);
		break;
	}

	/* run the command */
	return exstring(win, cmd);
}


/* This function starts or cancels visible marking.  It can also be called
 * with vinf=NULL to adjust the other endpoint of an existing mark.
 */
RESULT v_visible(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	long	col;

	assert(win->seltop || vinf);
	/*assert(!win->state->acton);*/

	/* Whenever a visible selection is in progress, redrawing is implied.
	 * We only need to force the screen to be regenerated when we're
	 * cancelling a visible selection.
	 */
	if (vinf && win->seltop)
		win->di->logic = DRAW_CHANGED;

	/* are we supposed to be adjusting the visible marking? */
	if (!vinf)
	{
		/* change "selattop" if appropriate */
		if (markoffset(win->cursor) < markoffset(win->seltop))
		{
			if (!win->selattop)
			{
				if (win->seltype != 'c')
				{
					/* set the bottom mark to the end of
					 * the line which contains the former
					 * top mark.
					 */
					marksetoffset(win->selbottom, markoffset(win->md->move(win, win->seltop, 0, INFINITY, False)));
				}
				else
				{
					/* former top mark becomes new bottom */
					marksetoffset(win->selbottom, markoffset(win->seltop));
				}
				win->selattop = True;
			}
		}
		else if (markoffset(win->selbottom) < markoffset(win->cursor))
		{
			if (win->selattop)
			{
				if (win->seltype != 'c')
				{
					/* set the top mark to the start of
					 * the line which contains the former
					 * bottom mark.
					 */
					marksetoffset(win->seltop, markoffset(win->md->move(win, win->selbottom, 0, 0, False)));
				}
				else
				{
					/* former bottom mark becomes new top */
					marksetoffset(win->seltop, markoffset(win->selbottom));
				}
				win->selattop = False;
			}
		}

		/* adjust the appropriate endpoint */
		if (win->selattop)
		{
			/* set the top limit */
			if (win->seltype != 'c')
			{
				/* set top to start of cursor line */
				marksetoffset(win->seltop, markoffset(win->md->move(win, win->cursor, 0, 0, False)));
			}
			else
			{
				/* set top to cursor position */
				marksetoffset(win->seltop, markoffset(win->cursor));
			}
		}
		else /* we're adjusting the bottom of the marked region */
		{
			/* set the bottom limit */
			if (win->seltype != 'c')
			{
				/* set bottom to end of cursor line */
				marksetoffset(win->selbottom, markoffset(win->md->move(win, win->cursor, 0, INFINITY, False)));
			}
			else
			{
				/* set bottom to cursor position */
				marksetoffset(win->selbottom, markoffset(win->cursor));
			}
		}

		/* if rectangular, then we also need to adjust column limits */
		if (win->seltype == 'r')
		{
			col = win->md->mark2col(win, win->cursor, True);
			if (win->selorigcol < col)
			{
				win->selleft = win->selorigcol;
				win->selright = col;
			}
			else
			{
				win->selleft = col;
				win->selright = win->selorigcol;
			}
		}

		/* Whew!  That was hard. */
		return RESULT_COMPLETE;
	}

	/* if already visibly marking, then cancel the marking */
	if (win->seltop)
	{
		markfree(win->seltop);
		markfree(win->selbottom);
		win->seltop = win->selbottom = NULL;
		return RESULT_COMPLETE;
	}

	/* else we need to start marking characters, lines, or a rectangle,
	 * depending on what the command key was.
	 */
	switch (vinf->command)
	{
	  case 'v':
		win->seltop = markdup(win->cursor);
		win->selbottom = markdup(win->cursor);
		win->selleft = 0;
		win->selright = INFINITY;
		win->selattop = False;
		win->selorigcol = 0;
		win->seltype = 'c';
		break;

	  case 'V':
		win->seltop = markdup(win->md->move(win, win->cursor, 0, 0, False));
		win->selbottom = markdup(win->md->move(win, win->cursor, 0, INFINITY, False));
		win->selleft = 0;
		win->selright = INFINITY;
		win->selorigcol = 0;
		win->selattop = False;
		win->seltype = 'l';
		break;

	  case ELVCTRL('V'):
		win->seltop = markdup(win->md->move(win, win->cursor, 0, 0, False));
		win->selbottom = markdup(win->md->move(win, win->cursor, 0, INFINITY, False));
		win->selleft = win->md->mark2col(win, win->cursor, True);
		win->selright = win->selleft;
		win->selorigcol = win->selleft;
		win->selattop = False;
		win->seltype = 'r';
		break;

	  case ELVCTRL('['):
		/* <Esc> was supposed to cancel visible marking.  If we get
		 * here instead, then no marking was in progress so <Esc>
		 * should beep.
		 */
		return RESULT_ERROR;
	}
	return RESULT_COMPLETE;
}


/* This function pastes text.  It implements the <p> and <Shift-P> commands. */
RESULT v_paste(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	dest;

	/* If repeating a paste from a numbered cut buffer, then allow the
	 * cutput function to locate the next numbered cut buffer itself.
	 * Else use the same cut buffer as in original comand.
	 */
	if ((vinf->tweak & TWEAK_DOTTING) != 0 && isdigit(vinf->cutbuf))
	{
		dest = cutput('\0', win, win->state->cursor, (BOOLEAN)(vinf->command == 'p'), True, False);
	}
	else
	{
		dest = cutput(vinf->cutbuf, win, win->state->cursor, (BOOLEAN)(vinf->command == 'p'), True, False);
	}

	/* check for failure or success. */
	if (!dest)
	{
		return RESULT_ERROR;
	}
	marksetoffset(win->state->cursor, markoffset(dest));
	return RESULT_COMPLETE;
}


/* This function implements the <Shift-Q> and <:> commands. */
RESULT v_ex(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	/* push a stratum that does ex commands */
	statestratum(win, toCHAR(EX_BUF), ':', exenter);
	o_internal(markbuffer(win->state->cursor)) = True;

	/* The statetratum() function pushes a state which exits after a
	 * single command.  If the command was <Shift-Q>, then we want to
	 * stay in the state, so we need to tweak the flags.
	 */
	if (vinf->command == 'Q')
	{
		win->state->flags &= ~(ELVIS_POP|ELVIS_ONCE|ELVIS_1LINE);
	}

	return RESULT_COMPLETE;
}


/* This function implements commands which are shortcuts for operator commands:
 * <s>, <Shift-C>, <Shift-D>, <Shift-S>, and <Shift-Y>.
 */
RESULT v_notop(win, vinf)
	WINDOW	win;
	VIINFO	*vinf;
{
	switch (vinf->command)
	{
	  case 's':
		vinf->oper = 'c';
		vinf->command = 'l';
		break;

	  case 'C':
		vinf->oper = 'c';
		vinf->command = '$';
		break;

	  case 'D':
		vinf->oper = 'd';
		vinf->command = '$';
		break;

	  case 'S':
		vinf->oper = 'c';
		vinf->command = '_';
		break;

	  case 'Y':
		vinf->oper = 'y';
		vinf->command = '_';
		break;
	}
	return viperform(win, vinf);
}


/* This function implements visual commands which are short-cuts for certain
 * common ex commands.
 */
RESULT v_notex(win, vinf)
	WINDOW	win;
	VIINFO	*vinf;
{
	EXINFO	xinfb;
	MARK	tmpmark;
	CHAR	*word;
	RESULT	result;
	int	i;

	DEFAULT(2);
	assert(vinf->command == ELVCTRL('^') || vinf->command == '&'
		|| vinf->command == '*' || vinf->command == 'J'
		|| vinf->command == 'K' || vinf->command == ELVCTRL('Z'));

	/* build & execute an ex command */
	memset((char *)&xinfb, 0, sizeof xinfb);
	xinfb.window = win;
	xinfb.defaddr = *win->state->cursor;
	switch (vinf->command)
	{
	  case ELVCTRL('Z'):
		result = exstring(win, toCHAR("stop"));
		break;

	  case ELVCTRL('^'):
		result = exstring(win, toCHAR("e #"));
		break;

	  case '&':
		result = exstring(win, toCHAR("&"));
		break;

	  case '*':
		result = exstring(win, toCHAR("errlist"));
		break;

	  case 'J':
		/* ":join" */
		xinfb.fromaddr = win->state->cursor;
		xinfb.from = markline(xinfb.fromaddr);
		xinfb.to = xinfb.from + vinf->count - 1;
		if (xinfb.to > o_buflines(markbuffer(xinfb.fromaddr)))
		{
			msg(MSG_ERROR, "not that many lines in buffer");
			return RESULT_ERROR;
		}
		result = ex_join(&xinfb);
		if (result == RESULT_COMPLETE && xinfb.newcurs)
		{
			marksetoffset(win->cursor, markoffset(xinfb.newcurs));
			markfree(xinfb.newcurs);
		}
		break;

	  default: /* 'K' */
		/* ":!ref word" */
		if (!o_keywordprg(markbuffer(win->state->cursor)))
		{
			msg(MSG_ERROR, "keywordprg not set");
			return RESULT_ERROR;
		}
		tmpmark = wordatcursor(&xinfb.defaddr);
		if (!tmpmark)
			return RESULT_ERROR;
		word = bufmemory(tmpmark, &xinfb.defaddr);
		i = CHARlen(o_keywordprg(markbuffer(win->state->cursor)));
		xinfb.rhs = safealloc(sizeof(CHAR), i + CHARlen(word) + 2);
		CHARcpy(xinfb.rhs, o_keywordprg(markbuffer(win->state->cursor)));
		xinfb.rhs[i] = ' ';
		CHARcpy(&xinfb.rhs[i + 1], word);
		xinfb.command = EX_BANG;
		result = ex_bang(&xinfb);
		safefree(xinfb.rhs);
		break;
	}

	return result;
}


/* This function implements the # command, which adds or subtracts a value
 * to the number that the cursor is on (if it is on a cursor)
 */
RESULT v_number(win, vinf)
	WINDOW	win;
	VIINFO	*vinf;
{
	CHAR	*p;		/* used for scanning text */
	MARKBUF	start, end;	/* start of the number */
	long	number;		/* the value of the number */
	char	asc[12];	/* the result value, as a string */

	DEFAULT(1);

	/* if not on a number, then fail */
	if (!isdigit(scanchar(win->state->cursor)))
	{
		return RESULT_ERROR;
	}

	/* locate the start of the number */
	for (scanalloc(&p, win->state->cursor); p && isdigit(*p); scanprev(&p))
	{
	}
	if (p)
	{
		scannext(&p);
		start = *scanmark(&p);
	}
	else
	{
		start = *win->state->cursor;
		marksetoffset(&start, 0);
		scanseek(&p, &start);
	}

	/* fetch the value of the number */
	for (number = 0; p && isdigit(*p); scannext(&p))
	{
		number = number * 10 + *p - '0';
	}

	/* remember where the number ended */
	if (p)
	{
		end = *scanmark(&p);
	}
	else
	{
		end = *win->state->cursor;
		marksetoffset(win->state->cursor, o_bufchars(markbuffer(&end)));
	}
	scanfree(&p);

	/* add the argument to the number */
	switch (vinf->key2)
	{
	  case '-':	number -= vinf->count;	break;
	  case '=':	number = vinf->count;	break;
	  default:	number += vinf->count;
	}

	/* convert the result back to a character string */
	sprintf(asc, "%ld", number);

	/* replace the old value with the new value */
	bufreplace(&start, &end, toCHAR(asc), (int)strlen(asc));

	/* leave the cursor at the start of the number */
	marksetoffset(win->state->cursor, markoffset(&start));

	return RESULT_COMPLETE;
}
