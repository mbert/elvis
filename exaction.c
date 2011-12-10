/* exaction.c */
/* Copyright 1995 by Steve Kirkendall */

char id_exaction[] = "$Id: exaction.c,v 2.59 1996/09/04 18:30:16 steve Exp $";

#include "elvis.h"

/* This function implements the :@x command. */
RESULT	ex_at(xinf)
	EXINFO	*xinf;
{
	BUFFER	cutbuf;
	MARKBUF	top, bottom;

	/* a cut buffer name is required */
	if (!xinf->cutbuf)
	{
		msg(MSG_ERROR, "cut buffer name required");
		return RESULT_ERROR;
	}

	/* find the cut buffer */
	cutbuf = cutbuffer(xinf->cutbuf, False);
	if (!cutbuf || o_bufchars(cutbuf) <= CUT_TYPELEN)
	{
		msg(MSG_ERROR, "[C]cut buffer $1 empty", xinf->cutbuf);
		return RESULT_ERROR;
	}

	/* execute the cut buffer */
	return experform(xinf->window, marktmp(top, cutbuf, CUT_TYPELEN),
		marktmp(bottom, cutbuf, o_bufchars(cutbuf)));
}


/* This function implements the :buffer command. */
RESULT	ex_buffer(xinf)
	EXINFO	*xinf;
{
	BUFFER	buf;		/* a buffer */
	WINDOW	win;		/* a window that shows a given buffer */
	CHAR	winlist[100];	/* buffer, holds string containing output line */
	int	len;		/* length of string in winlist[] */
	int	bnlen;		/* length of buffer name */

	assert(xinf->command == EX_BUFFER);

	/* do we have an argument? */
	if (xinf->rhs)
	{
		/* can't change the name of an internal buffer */
		buf = markbuffer(&xinf->defaddr);
		if (o_internal(buf))
		{
			msg(MSG_ERROR, "can't retitle internal buffers");
			return RESULT_ERROR;
		}

		/* change the name of this buffer */
		buftitle(markbuffer(&xinf->defaddr), xinf->rhs);
	}
	else
	{
		/* no arguments -- list the buffers */
		for (buf = buflist((BUFFER)0); buf; buf = buflist(buf))
		{
			/* if no ! given, then ignore internal buffers */
			if (o_internal(buf) && !xinf->bang)
			{
				continue;
			}

			/* make a list of the windows showing that editor */
			bnlen = CHARlen(o_bufname(buf));
			for (len = 0, win = winofbuf((WINDOW)0, buf);
			     len < (int)(QTY(winlist) - 5 - bnlen) && win;
			     win = winofbuf(win, buf))
			{
				/* add this window to the list */
				sprintf((char *)winlist + len, "%ld: ", o_windowid(win));
				len = CHARlen(winlist);
			}
			while (len < 6)
			{
				winlist[len] = ' ';
				len++;
			}
			winlist[len++] = (o_internal(buf) ? '-' : o_modified(buf) ? '*' : ' ');
			(void)CHARncpy(&winlist[len], o_bufname(buf), (size_t)bnlen);
			len += bnlen;
			winlist[len++] = '\n';

			/* output a line of info about the buffer */
			drawextext(windefault, winlist, len);
		}
	}
	return RESULT_COMPLETE;
}


RESULT ex_all(xinf)
	EXINFO	*xinf;
{
	BUFFER	buf;
	MARKBUF	bufline1;
	MARK	origcurs;
	RESULT	result;

	/* An RHS is required */
	if (!xinf->rhs)
	{
		msg(MSG_ERROR, "missing rhs");
		return RESULT_ERROR;
	}

	/* Remember the original cursor */
	if (xinf->window->state->pop)
		origcurs = xinf->window->state->pop->cursor;
	else
		origcurs = xinf->window->cursor;

	/* for each buffer... */
	for (buf = buflist((BUFFER)0), result = RESULT_COMPLETE;
	     buf && result == RESULT_COMPLETE;
	     buf = buflist(buf))
	{
		/* unless ":all!", ignore internal buffers */
		if (!xinf->bang && o_internal(buf))
			continue;

		/* Make this buffer the default buffer */
		bufoptions(buf);

		/* Temporarily move the window's cursor to the first line
		 * of this buffer.
		 */
		if (xinf->window->state->pop)
			xinf->window->state->pop->cursor = marktmp(bufline1, buf, 0);
		else
			xinf->window->cursor = marktmp(bufline1, buf, 0);

		/* execute the command */
		result = exstring(xinf->window, xinf->rhs);
	}

	/* Restore the original cursor */
	if (xinf->window->state->pop)
		xinf->window->state->pop->cursor = origcurs;
	else
		xinf->window->cursor = origcurs;

	return result;
}


/* This implements the :window command */
RESULT ex_window(xinf)
	EXINFO	*xinf;
{
	WINDOW	win;
	BUFFER	buf;
	char	idstr[20];
	long	id;

	/* if no argument, then list windows */
	if (!xinf->lhs)
	{
		for (win = winofbuf(NULL, NULL); win; win = winofbuf(win, NULL))
		{
			sprintf(idstr, "%6ld: ", o_windowid(win));
			drawextext(xinf->window, toCHAR(idstr), strlen(idstr));
			drawextext(xinf->window,
				o_bufname(markbuffer(win->cursor)),
				CHARlen(o_bufname(markbuffer(win->cursor))));
			drawextext(xinf->window, toCHAR("\n"), 1);
		}
		return RESULT_COMPLETE;
	}

	/* otherwise we are trying to switch windows... */
	if (calcnumber(xinf->lhs))
	{
		/* find a window with the given window id */
		id = atol(tochar8(xinf->lhs));
		for (win = winofbuf(NULL, NULL);
		     win && o_windowid(win) != id;
		     win = winofbuf(win, NULL))
		{
		}
	}
	else if (xinf->lhs[0] == '+')
	{
		/* move up 1 window */
		for (win = NULL; winofbuf(win, NULL) != xinf->window; )
		{
			win = winofbuf(win, NULL);
			if (!win)
				break;
		}
		if (!win && xinf->lhs[1])
		{
			for (win = winofbuf(NULL, NULL);
			     winofbuf(win, NULL);
			     win = winofbuf(win, NULL))
			{
			}
		}
	}
	else if (xinf->lhs[0] == '-')
	{
		/* move down 1 window */
		win = winofbuf(xinf->window, NULL);
		if (!win && xinf->lhs[1])
			win = winofbuf(NULL, NULL);
	}
	else
	{
		/* try to find a window showing a given buffer/file */
		buf = buffind(xinf->lhs);
		if (!buf)
		{
			msg(MSG_ERROR, "no such buffer");
			return RESULT_ERROR;
		}
		win = winofbuf(xinf->window, buf);
		if (!win)
		{
			win = winofbuf(NULL, buf);
		}
	}

	/* did we find the new window? */
	if (!win)
	{
		msg(MSG_ERROR, "no such window");
		return RESULT_ERROR;
	}

	/* switch to the window */
	if (gui->focusgw)
	{
		(*gui->focusgw)(win->gw);
	}
	else
	{
		eventfocus(win->gw);
	}
	return RESULT_COMPLETE;
}


RESULT	ex_cd(xinf)
	EXINFO	*xinf;
{
	BUFFER	buf;

	assert(xinf->command == EX_CD);

	/* check arguments */
	if (xinf->nfiles != 1)
	{
		msg(MSG_ERROR, "directory name required");
		return RESULT_ERROR;
	}

	/* if any user buffers have been modified but not saved, complain
	 * unless '!' was given.
	 */
	if (!xinf->bang)
	{
		for (buf = NULL; (buf = buflist(buf)) != NULL; )
		{
			if (!o_internal(buf) && o_modified(buf))
			{
				msg(MSG_ERROR, "[S]$1 modified, not saved", o_bufname(buf));
				return RESULT_ERROR;
			}
		}
	}

	/* try to switch to that directory */
	if (!dirchdir(xinf->file[0]))
	{
		msg(MSG_ERROR, "can't change directory");
		return RESULT_ERROR;
	}

	/* turn off the "edited" flag for all user buffers */
	for (buf = NULL; (buf = buflist(buf)) != NULL; )
	{
		if (!o_internal(buf))
			o_edited(buf) = False;
	}

	return RESULT_COMPLETE;
}


RESULT	ex_edit(xinf)
	EXINFO	*xinf;
{
	BUFFER	oldbuf;	/* buffer to switch from */
	BUFFER	newbuf;	/* buffer to switch to */
	STATE	*s;	/* used for stepping though state stack */
	CHAR	*p;	/* used for scanning movement string */
	EXINFO	xinfb;	/* dummy command buffer, used for parsing address */
	long	line=1;	/* default line number to start on */

	assert(xinf->command == EX_EDIT || xinf->command == EX_OPEN
		|| xinf->command == EX_VISUAL);

	/* These only work when the ex command acts on the main buffer */
	if (xinf->window->state->acton && xinf->window->state->acton->acton)
	{
		msg(MSG_ERROR, "[s]$1 only works on window's main buffer", xinf->cmdname);
		return RESULT_ERROR;
	}

	/* If visual mode isn't supported, then either fail or use open mode */
	if (gui->exonly && xinf->command != EX_EDIT)
	{
		msg(MSG_ERROR, "this gui only supports ex mode");
		return RESULT_ERROR;
	}
	else if (!gui->moveto && xinf->command == EX_VISUAL)
	{
		msg(MSG_WARNING, "using open mode");
		xinf->command = EX_OPEN;
	}

	/* Are we supposed to switch buffers? */
	if (xinf->nfiles > 0)
	{
		/* If we expect to load the "previousfile" then the default
		 * starting line is the "previousfileline".
		 */
		if (o_previousfile
		 && !CHARcmp(o_previousfile, xinf->file[0]))
		{
			line = o_previousfileline;
		}

		/* For now, set the "previous file" to the named file.  If the
		 * command is successful, this will be replaced by the name of
		 * the old file before ex_edit() returns.  Exception: ":e%"
		 * should not affect the "previous file".
		 */
		if (o_filename(markbuffer(xinf->window->cursor))
		 && strcmp(tochar8(o_filename(markbuffer(xinf->window->cursor))), xinf->file[0]))
		{
			if (o_previousfile)
				safefree(o_previousfile);
			optpreset(o_previousfile, CHARdup(toCHAR(xinf->file[0])), OPT_FREE);
			o_previousfileline = 1L;
		}

		/* If the buffer we'll be loading already exists, and has been
		 * modified, then fail unless "!" was given.
		 */
		newbuf = buffind(toCHAR(xinf->file[0]));
		if (newbuf && o_modified(newbuf) && !xinf->bang)
		{
			msg(MSG_ERROR, "[S]$1 modified, not saved", o_bufname(newbuf));
			return RESULT_ERROR;
		}

		/* None of these commands are ever supposed to save the old
		 * buffer.  When we eventually switch to the new buffer,
		 * the old buffer will be unloaded, so if it has been
		 * modified and isn't used by anything else, we should either
		 * fail (if no ! given) or reset the "modified" flag (if !)
		 */
		oldbuf = markbuffer(xinf->window->cursor);
		if (o_modified(oldbuf))
		{
			if (xinf->bang)
				o_modified(oldbuf) = False;
			else if (!o_autowrite || !bufsave(oldbuf, False, False))
			{
				msg(MSG_ERROR, "[S]$1 modified, not saved", o_bufname(oldbuf));
				return RESULT_ERROR;
			}
		}

		/* load the new buffer */
		newbuf = bufload(NULL, xinf->file[0], True);

		/* if the line number is invalid, ignore it */
		if (line < 0 || line > o_buflines(newbuf))
		{
			line = 1;
		}

		/* If we were given a command, and the buffer is either new or
		 * non-empty, then run the command.
		 */
		memset((char *)&xinfb, 0, sizeof xinfb);
		xinfb.defaddr.buffer = newbuf;
		xinfb.defaddr.offset = lowline(bufbufinfo(newbuf), line);
		if (xinf->lhs && (o_newfile(newbuf) || o_bufchars(newbuf) > 0))
		{
			(void)scanstring(&p, xinf->lhs);
			if (exparseaddress(&p, &xinfb))
			{
				xinfb.defaddr.offset = lowline(
					bufbufinfo(xinfb.defaddr.buffer),
					xinfb.to);
			}
			scanfree(&p);
		}

		/* change the buffer of this window. */
		xinf->newcurs = markdup(&xinfb.defaddr);
	}

	/* Set the main buffer to visual/open mode? */
	if (xinf->command != EX_EDIT)
	{
		/* exit "ex" mode */
		for (s = xinf->window->state; s != xinf->window->state->acton; s = s->pop)
			s->flags |= ELVIS_1LINE;

		/* force main edit state use use bottom line, or not to */
		if (xinf->command == EX_VISUAL)
		{
			for (; s; s = s->pop)
				s->flags &= ~ELVIS_BOTTOM;
		}
		else
		{
			for (; s; s = s->pop)
				s->flags |= ELVIS_BOTTOM;
		}
	}

	return RESULT_COMPLETE;
}


RESULT	ex_file(xinf)
	EXINFO	*xinf;
{
	long	lnum;
	MARK	cursor;
	BUFFER	buf;

	switch (xinf->command)
	{
	  case EX_EQUAL:
		if (xinf->from != xinf->to)
		{
			msg(MSG_INFO, "[ddd]$1,$2 = $3 lines",
			       xinf->from, xinf->to, xinf->to - xinf->from + 1);
		}
		else
		{
			msg(MSG_INFO, "[d]$1", xinf->from);
		}
		break;

	  default: /*  EX_FILE */
		/* who are we talking about here? */
		cursor = &xinf->defaddr;
		buf = markbuffer(cursor);
		lnum = markline(cursor);

		/* were we given a new name for this buffer? */
		if (xinf->nfiles == 1)
		{
			/* store the given filename as the new file of this buffer */
			o_edited(buf) = False;
			if (optflags(o_filename(buf)) & OPT_FREE)
			{
				safefree(o_filename(buf));
			}
			o_filename(buf) = CHARdup(toCHAR(xinf->file[0]));
			optflags(o_filename(buf)) |= OPT_FREE;
			buftitle(buf, o_filename(buf));
		}

		/* output statistics of this file */
		if (markbuffer(xinf->window->cursor) == buf && o_buflines(buf) > 0)
		{
			msg(MSG_INFO,
				"[dd](filename)(readonly?\" [READONLY]\")(modified?\" [MODIFIED]\")(!edited?\" [NOT EDITED]\")(newfile?\" [NEW FILE]\") ($1 * 100 / $2)%",
				lnum, o_buflines(buf));
		}
		else
		{
			msg(MSG_INFO,
				"(filename)(readonly?\" [READONLY]\")(modified?\" [MODIFIED]\")(!edited?\" [NOT EDITED]\")(newfile?\" [NEW FILE]\")");
		}
		break;
	}
	return RESULT_COMPLETE;
}


/* This function implements the :lpr command */
RESULT	ex_lpr(xinf)
	EXINFO	*xinf;
{
	RESULT	ret;
	CHAR	*origlp;
	
	/* if a filename/filter is given on command line, use it */
	origlp = o_lpout;
	if (xinf->rhs)
	{
		o_lpout = xinf->rhs;
	}
	else if (xinf->nfiles >= 1)
	{
		assert(xinf->nfiles == 1);
		o_lpout = toCHAR(xinf->file[0]);
	}

	/* print */
	ret = lp(xinf->window, xinf->fromaddr, xinf->toaddr, xinf->bang);

	/* restore lpout to its original value */
	o_lpout = origlp;

	return ret;
}


/* This function implements the :mark and :k commands */
RESULT	ex_mark(xinf)
	EXINFO	*xinf;
{
	/* check mark name */
	if (!xinf->lhs || *xinf->lhs < 'a' || *xinf->lhs > 'z')
	{
		msg(MSG_ERROR, "bad mark name");
		return RESULT_ERROR;
	}

	/* if mark already set, then free its old value. */
	if (namedmark[*xinf->lhs - 'a'])
	{
		markfree(namedmark[*xinf->lhs - 'a']);
	}

	/* set the mark */
	namedmark[*xinf->lhs - 'a'] = markdup(xinf->fromaddr);
	return RESULT_COMPLETE;
}


RESULT	ex_mkexrc(xinf)
	EXINFO	*xinf;
{
	BUFFER	buf = buffind(toCHAR(CUSTOM_BUF));
	MARKBUF	top, bottom;

	/* if no changes have been made, great! */
	if (!buf)
	{
		return RESULT_COMPLETE;
	}

	/* else write the buffer out to the given file, or CUSTOM_FILE
	 * by default.
	 */
	if (bufwrite(marktmp(top, buf, 0), marktmp(bottom, buf, o_bufchars(buf)),
		xinf->nfiles == 1 ? xinf->file[0] : CUSTOM_FILE, xinf->bang))
	{
		return RESULT_COMPLETE;
	}
	return RESULT_ERROR;
}


/* This implements the commands which read the args list... :next, :Next,
 * :previous, :rewind, :last, :wnext, :snext, :sNext, :sprevious, :srewind,
 * and :slast.
 */
RESULT	ex_next(xinf)
	EXINFO	*xinf;
{
	int	newargnext;	/* value argnext should have if successful */
	BUFFER	oldbuf;		/* the buffer we're leaving */
	BUFFER	newbuf;		/* the buffer we're entering */
	BOOLEAN	splitting;	/* are we going to create a new window? */
	BOOLEAN	closing;	/* are we going to delete the old buffer? */
	char	**tmp;		/* used for swapping args lists */

	assert(xinf->command == EX_NEXT || xinf->command == EX_PREVIOUS ||
		xinf->command == EX_LAST || xinf->command == EX_REWIND ||
		xinf->command == EX_SNEXT || xinf->command == EX_SPREVIOUS ||
		xinf->command == EX_SLAST || xinf->command == EX_SREWIND ||
		xinf->command == EX_WNEXT);

	/* initialize some variables */
	oldbuf = markbuffer(xinf->window->cursor);
	splitting = (BOOLEAN)(xinf->command == EX_SNEXT || xinf->command == EX_SPREVIOUS
		|| xinf->command == EX_SLAST || xinf->command == EX_SREWIND);
	closing = (BOOLEAN)(!splitting && (xinf->command == EX_WNEXT || (winofbuf(NULL, oldbuf) == xinf->window
		&& winofbuf(xinf->window, oldbuf) == NULL)));

	/* diddle with the args list.  Upon exit, arglist[argnext] is name
	 * of the next file to load.
	 */
	newargnext = argnext;
	if (xinf->command == EX_REWIND || xinf->command == EX_SREWIND)
	{
		newargnext = 0;
	}
	else if (xinf->command == EX_LAST || xinf->command == EX_SLAST)
	{
		for (newargnext = 0; arglist[newargnext] && arglist[newargnext + 1]; newargnext++)
		{
		}
	}
	else if (xinf->command == EX_PREVIOUS || xinf->command == EX_SPREVIOUS)
	{
		if (argnext < 2)
		{
			msg(MSG_ERROR, "no more files");
			return RESULT_ERROR;
		}
		newargnext -= 2;
	}
	else if (xinf->nfiles > 0)
	{
		/* swap the new args list with the old one.  When the ex
		 * command is freed, the old args list will be freed and the
		 * new one will remain.
		 */
		tmp = xinf->file;
		xinf->file = arglist;
		arglist = tmp;
		for (xinf->nfiles = 0; xinf->file[xinf->nfiles]; xinf->nfiles++)
		{
		}
		newargnext = argnext = 0;
	}

	/* if there is no next file, complain */
	if (!arglist[newargnext])
	{
		msg(MSG_ERROR, "no more files");
		return RESULT_ERROR;
	}

	/* if we'll be closing this buffer, be cautious */
	if (closing && o_modified(oldbuf) && !o_autowrite && xinf->command != EX_WNEXT)
	{
		/* Trying to leave a modifed file which wouldn't be saved */
		if (!xinf->bang)
		{
			msg(MSG_ERROR, "modified, not written");
			return RESULT_ERROR;
		}
		/* else ":n!", so turn off the modified flag */
		o_modified(oldbuf) = False;
	}
	if (!closing || bufsave(oldbuf, xinf->bang, (BOOLEAN)(xinf->command == EX_WNEXT)))
	{
		/* load the new buffer */
		newbuf = bufload(NULL, arglist[newargnext++], False);

		/* either create a new window, or change the buffer
		 * of this window.
		 */
		if (splitting)
		{
			if (!(*gui->creategw)(tochar8(o_bufname(newbuf)), ""))
			{
				return RESULT_ERROR;
			}
		}
		else
		{
			xinf->newcurs = markalloc(newbuf, 0);
		}
	}

	argnext = newargnext;
	return RESULT_COMPLETE;
}


RESULT	ex_pop(xinf)
	EXINFO	*xinf;
{
	int	i;

	/* if the tag stack is empty, fail */
	if (!xinf->window->tagstack[0].origin)
	{
		msg(MSG_ERROR, "tag stack empty");
		return RESULT_ERROR;
	}

	/* If the popped buffer is different from the current buffer, then
	 * we'll want to save the current buffer before switching.  If we
	 * can't switch, then fail unless ! given.
	 */
	if (markbuffer(xinf->window->tagstack[0].origin) != markbuffer(xinf->window->cursor)
	 && !xinf->bang
	 && (o_autowrite ? !bufsave(markbuffer(xinf->window->cursor), False, False)
	 		: o_modified(markbuffer(xinf->window->cursor))))
	{
		if (!o_autowrite)
		{
			msg(MSG_ERROR, "[S]$1 modified, not saved", o_bufname(markbuffer(xinf->window->cursor)));
		}
		return RESULT_ERROR;
	}

	/* set the "previous tag" back to what it was when this stack entry
	 * was pushed.
	 */
	assert(o_previoustag);
	safefree(o_previoustag);
	o_previoustag = xinf->window->tagstack[0].prevtag;

	/* change this window's display mode to what it was when tag pushed */
	(void)dispset(xinf->window, xinf->window->tagstack[0].display);

	/* cause the cursor to be moved to the position on top of tag stack */
	xinf->newcurs = xinf->window->tagstack[0].origin;

	/* delete the top item from the tag stack */
	for (i = 0; i < TAGSTK - 1; i++)
	{
		xinf->window->tagstack[i] = xinf->window->tagstack[i + 1];
	}
	xinf->window->tagstack[i].origin = NULL;
	xinf->window->tagstack[i].prevtag = NULL;

	return RESULT_COMPLETE;
}


RESULT	ex_bang(xinf)
	EXINFO	*xinf;
{
	MARK	mark;
	CHAR	*cp;
	char	*bangcmd;
	CHAR	iobuf[4096];
	int	len;
	BOOLEAN	origrefresh;

	assert(xinf->command == EX_BANG);

	/* rhs is required */
	if (!xinf->rhs)
	{
		msg(MSG_ERROR, "filter name is missing");
		return RESULT_ERROR;
	}

	/* if no lines were specified, then just execute the command and
	 * show its output in the window.
	 */
	if (!xinf->fromaddr)
	{
		/* If the GUI has prgopen()/prgclose(), then we can trust it to
		 * do good things with stdio.  Otherwise we'll need to fake it.
		 */
		if (gui->prgopen)
		{
#if 0
			origrefresh = o_exrefresh;
			o_exrefresh = True;
#endif
			drawopencomplete(xinf->window);
			assert(xinf->window->di->drawstate == DRAW_OPENOUTPUT);
#if 0
			o_exrefresh = origrefresh;
#endif

			assert(gui->prgclose);
			if (gui->flush)
				(*gui->flush)();
			if ((*gui->prgopen)((char *)xinf->rhs, False, False)
				&& prggo()
				&& (*gui->prgclose)() == 0)
			{
				return RESULT_COMPLETE;
			}
			return RESULT_ERROR;
		}
		else
		{
			origrefresh = o_exrefresh;
			o_exrefresh = True;
			
			bangcmd = (char *)safealloc((int)CHARlen(xinf->rhs) + 2, sizeof(char));
			bangcmd[0] = '!';
			strcpy(bangcmd + 1, tochar8(xinf->rhs));
			if (!ioopen(bangcmd, 'r', False, False, False))
			{
				o_exrefresh = origrefresh;
				return RESULT_ERROR;
			}
			while ((len = ioread(iobuf, QTY(iobuf))) > 0)
			{
				drawextext(xinf->window, iobuf, len);
			}
			safefree(bangcmd);
			o_exrefresh = origrefresh;
			return ioclose() ? RESULT_COMPLETE : RESULT_ERROR;
		}
	}

	/* prepare to start the filter program */
	if (gui->prgopen
		? !(*gui->prgopen)(tochar8(xinf->rhs), True, True)
		: !prgopen(tochar8(xinf->rhs), True, True))
	{
		return RESULT_ERROR;
	}

	/* write the original text from buffer to filter */
	mark = markdup(xinf->fromaddr);
	scanalloc(&cp, mark);
	do
	{
		len = scanright(&cp);
		if (markoffset(mark) + len > markoffset(xinf->toaddr))
		{
			len = (int)(markoffset(xinf->toaddr) - markoffset(mark));
		}
		if (prgwrite(cp, len) < len)
		{
			msg(MSG_ERROR, "broken pipe");
			ioclose();
			scanfree(&cp);
			markfree(mark);
			return RESULT_ERROR;
		}
		markaddoffset(mark, len);
		scanseek(&cp, mark);
	} while (cp != NULL && markoffset(mark) < markoffset(xinf->toaddr));
	scanfree(&cp);

	/* switch to the reading phase */
	if (!prggo())
	{
		markfree(mark);
		return RESULT_ERROR;
	}

	/* delete the old input lines */
	bufreplace(xinf->fromaddr, xinf->toaddr, NULL, 0);

	/* read the new input lines */
	while ((len = prgread(iobuf, QTY(iobuf))) > 0)
	{
		/* insert a chunk of text into buffer */
		bufreplace(mark, mark, iobuf, (long)len);
		markaddoffset(mark, len);
	}

	/* clean up & exit */
	markfree(mark);
	return (prgclose() == 0) ? RESULT_COMPLETE : RESULT_ERROR;
}


RESULT	ex_source(xinf)
	EXINFO	*xinf;
{
	BUFFER	buf;	/* temporary buffer, holds script */
	MARKBUF	start;	/* temporary mark, points to start of buf */
	MARKBUF	end;	/* temporary mark, points to end of buf */
	int	nbytes;	/* size of a chunk of text read from file */
	CHAR	*io;	/* I/O buffer, holds chunk of text from file */
	RESULT	result;	/* results of executing the commands from the file */
	BOOLEAN	origsafer;/* original value of "safer" option */

	assert(xinf->command == EX_SOURCE || xinf->command == EX_SAFER);

	/* the file name is REQUIRED! */
	if (xinf->nfiles != 1)
	{
		msg(MSG_ERROR, "[s]$1 requires a file name", xinf->cmdname);
		return RESULT_ERROR;
	}
		
	/* if ! appeared after the command name, and the file doesn't exist,
	 * then do nothing but return with no error.
	 */
	if (xinf->bang && dirperm(xinf->file[0]) == DIR_NEW)
	{
		return RESULT_COMPLETE;
	}

	/* open the file */
	if (!ioopen(xinf->file[0], 'r', False, False, False))
		return RESULT_ERROR;

	/* create a temp buffer */
	buf = bufalloc(NULL, 0);
	assert(buf != NULL);

	/* fill the temp buffer with text read from the file */
	io = safealloc(1024, sizeof(CHAR));
	while ((nbytes = ioread(io, 1024)) > 0)
	{
		bufreplace(marktmp(end, buf, o_bufchars(buf)), &end, io, nbytes);
	}
	safefree(io);
	(void)ioclose();

	/* if :safer command, then temporarily set the "safer" option */
	origsafer = o_safer;
	if (xinf->command == EX_SAFER)
	{
		o_safer = True;
	}

	/* execute the contents of the buffer as a series of ex commands */
	result = experform(xinf->window, marktmp(start, buf, 0),
					 marktmp(end, buf, o_bufchars(buf)));

	/* reset the "safer" option to its original value. */
	o_safer = origsafer;

	/* destroy the temporary buffer */
	buffree(buf);

	/* return the results from experform() */
	return result;
}


RESULT	ex_stack(xinf)
	EXINFO *xinf;
{
	int	i;

	assert(xinf->command == EX_STACK);

	for (i = 0; i < TAGSTK && xinf->window->tagstack[i].origin; i++)
	{
		msg(MSG_INFO, "[dSS]+$1 $2 $3",
			markline(xinf->window->tagstack[i].origin),
			o_bufname(markbuffer(xinf->window->tagstack[i].origin)),
			xinf->window->tagstack[i].prevtag ?
				xinf->window->tagstack[i].prevtag : toCHAR(""));
	}
	return RESULT_COMPLETE;
}


RESULT	ex_suspend(xinf)
	EXINFO	*xinf;
{
	EXINFO	banger;
	RESULT	result;

	/* Give the GUI a chance to do this in a non-portable way */
	if (gui->stop)
	{
		result = (*gui->stop)((BOOLEAN)(xinf->command == EX_SHELL));
		if (result != RESULT_MORE)
			return result;
		/* else the GUI wants the default implementation */
	}

	/* We need to spawn an interactive shell. */
	memset((char *)&banger, 0, sizeof banger);
	banger.window = xinf->window;
	banger.defaddr = xinf->defaddr;
	banger.command = EX_BANG;
	banger.rhs = o_shell;
	return ex_bang(&banger);
}


RESULT	ex_tag(xinf)
	EXINFO	*xinf;
{
	CHAR	*fromtag;
	MARK	tagdefn;
	RESULT	result = RESULT_COMPLETE;
	BUFFER	oldbuf;
	int	i;

	assert(xinf->command == EX_TAG || xinf->command == EX_STAG);

	/* save a copy of the previous tag, if there was one */
	fromtag = o_previoustag ? CHARdup(o_previoustag) : NULL;

	/* if a tagname was given, use it (else use previous tag name) */
	if (xinf->lhs)
	{
		if (o_previoustag)
			safefree(o_previoustag);
		o_previoustag = CHARkdup(xinf->lhs);
	}
	else if (!o_previoustag)
	{
		msg(MSG_ERROR, "no previous tag");
		result = RESULT_ERROR;
		goto Finish;
	}

	/* search for the tag */
	tagdefn = (*xinf->window->md->tagload)(o_previoustag, xinf->window->cursor);
	if (!tagdefn)
	{
		result = RESULT_ERROR;
		goto Finish;
	}

	/* maybe split off a new window at the tag's definition */
	markbuffer(tagdefn)->changepos = markoffset(tagdefn);
	if (xinf->command == EX_STAG
	 && (*gui->creategw)(tochar8(o_bufname(markbuffer(tagdefn))), ""))
	{
		goto Finish;
	}

	/* if switching buffers, and the current buffer has been modified,
	 * and no other window is also showing this buffer, then either
	 * complain or write the buffer.
	 */
	oldbuf = markbuffer(xinf->window->cursor);
	if (!xinf->bang
	 && markbuffer(tagdefn) != oldbuf
	 && o_modified(oldbuf)
	 && winofbuf(NULL, oldbuf) == xinf->window
	 && winofbuf(xinf->window, oldbuf) == NULL)
	{
		if (!o_autowrite)
		{
			msg(MSG_ERROR, "[S]$1 modified, not saved", o_bufname(oldbuf));
			result = RESULT_ERROR;
			goto Finish;
		}
		else if (!bufsave(oldbuf, False, False))
		{
			result = RESULT_ERROR;
			goto Finish;
		}
	}

	/* push the current cursor position and display mode onto tag stack */
	if (o_tagstack)
	{
		/* The oldest tag will be lost.  If it had pointers to any
		 * dynamically allocated memory, then free that memory now.
		 */
		if (xinf->window->tagstack[TAGSTK - 1].prevtag)
			safefree(xinf->window->tagstack[TAGSTK - 1].prevtag);
		if (xinf->window->tagstack[TAGSTK - 1].origin)
			markfree(xinf->window->tagstack[TAGSTK - 1].origin);

		for (i = TAGSTK - 1; i > 0; i--)
		{
			xinf->window->tagstack[i] = xinf->window->tagstack[i - 1];
		}
		xinf->window->tagstack[0].origin = markdup(xinf->window->cursor);
		xinf->window->tagstack[0].display = xinf->window->md->name;
		xinf->window->tagstack[0].prevtag = fromtag;
		fromtag = NULL;
	}

	/* arrange for the cursor to move to the tag position */
	xinf->newcurs = markdup(tagdefn);

Finish:
	if (fromtag)
		safefree(fromtag);
	return result;
}


RESULT	ex_split(xinf)
	EXINFO	*xinf;
{
	BUFFER	buffer;

	assert(xinf->command == EX_SPLIT || xinf->command == EX_SNEW);

	/* decide which buffer should appear in the new window */
	if (xinf->command == EX_SNEW)
		buffer = bufalloc(NULL, 0);
	else if (xinf->nfiles == 1)
		buffer = bufload(NULL, xinf->file[0], False);
	else
		buffer = markbuffer(&xinf->defaddr);

	/* ask the GUI to create the window */
	if (!(*gui->creategw)(tochar8(o_bufname(buffer)), ""))
	{
		/* failed! */
		if (xinf->command == EX_SNEW)
			buffree(buffer);
		return RESULT_ERROR;
	}
	return RESULT_COMPLETE;
}


RESULT	ex_sall(xinf)
	EXINFO	*xinf;
{
	int	i;
	BUFFER	buffer;

	assert(xinf->command == EX_SALL);

	/* for each arg... */
	for (i = 0; arglist && arglist[i]; i++)
	{
		/* load the arg into a buffer, if it isn't already loaded */
		buffer = bufload(NULL, arglist[i], False);

		/* skip buffers that already have a window */
		if (winofbuf(NULL, buffer))
			continue;

		/* Ask the GUI to create a window.  If it can't,
		 * then the GUI will emit an error message.
		 */
		if (!(*gui->creategw)(tochar8(o_bufname(buffer)), ""))
			return RESULT_ERROR;
	}
	return RESULT_COMPLETE;
}
