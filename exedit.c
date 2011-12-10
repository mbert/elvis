/* exedit.c */
/* Copyright 1995 by Steve Kirkendall */

char id_exedit[] = "$Id: exedit.c,v 2.36 1996/10/01 19:46:29 steve Exp $";

#include "elvis.h"


/* This command implements the :insert, :append, and :change commands */
RESULT	ex_append(xinf)
	EXINFO	*xinf;
{
	MARK	where;

	assert(xinf->command == EX_APPEND || xinf->command == EX_CHANGE
		|| xinf->command == EX_INSERT);

	/* this only works on the window's main buffer */
	if (!xinf->window->state->acton)
	{
		msg(MSG_ERROR, "not main buffer");
		return RESULT_ERROR;
	}

	/* different behavior, depending on the command... */
	if (xinf->command == EX_CHANGE)
	{
		cutyank('\0', xinf->fromaddr, xinf->toaddr, 'L', True);
		where = markdup(xinf->fromaddr);
	}
	else if (xinf->command == EX_APPEND)
	{
		where = markdup(xinf->toaddr);
	}
	else
	{
		where = markdup(xinf->fromaddr);
	}

	/* Was the new text given as an argument to the command? */
	if (xinf->rhs)
	{
		/* yes, insert the new text at the appropriate place */
		bufreplace(where, where, xinf->rhs, (long)CHARlen(xinf->rhs));
		markaddoffset(where, CHARlen(xinf->rhs));
		bufreplace(where, where, toCHAR("\n"), 1);
		markaddoffset(where, 1);
	}
	xinf->newcurs = where;
	return RESULT_COMPLETE;
}


/* This function implements the :delete and :yank command */
RESULT	ex_delete(xinf)
	EXINFO	*xinf;
{
	/* check the cut buffer name */
	if (xinf->cutbuf && !isalnum(xinf->cutbuf) && xinf->cutbuf != '<' && xinf->cutbuf != '>')
	{
		msg(MSG_ERROR, "bad cut buffer");
		return RESULT_ERROR;
	}

	/* do the command */
	cutyank(xinf->cutbuf, xinf->fromaddr, xinf->toaddr, (CHAR)'L', (BOOLEAN)(xinf->command == EX_DELETE));
	return RESULT_COMPLETE;
}


RESULT	ex_global(xinf)
	EXINFO	*xinf;
{
	CHAR	*cp;
	long	lenln, endln;
	RESULT	ret = RESULT_COMPLETE;
	MARK	cursor;
	MARK	orig;
	MARKBUF	thisln;

	assert(xinf->command == EX_GLOBAL || xinf->command == EX_VGLOBAL);

	/* a command is required */
	if (!xinf->rhs)
	{
		msg(MSG_ERROR, "[s]$1 requires a command", xinf->cmdname);
		return RESULT_ERROR;
	}

	/* only works when applied to window's main buffer */
	if (!xinf->window->state->acton || xinf->window->state->acton->acton)
	{
		msg(MSG_ERROR, "[s]$1 only works on main buffer", xinf->cmdname);
		return RESULT_ERROR;
	}

	/* ":g!" is like ":v" */
	if (xinf->bang)
		xinf->command = EX_VGLOBAL;

	/* remember the cursor's original position.  Inside the following loop,
	 * we'll force the cursor onto each matching line; when we're done,
	 * we'll move the cursor back and return the final position so the
	 * experform() function can move the cursor there in the conventional
	 * way -- that will be important if we switch buffers.
	 */
	if (xinf->window->state->acton)
		cursor = xinf->window->state->acton->cursor;
	else
		cursor = xinf->window->cursor;
	if (markbuffer(cursor) != markbuffer(xinf->fromaddr))
	{
		marksetbuffer(cursor, markbuffer(xinf->fromaddr));
		marksetoffset(cursor, markoffset(xinf->fromaddr));
	}
	orig = markdup(cursor);

	/* for each line... */
	(void)scanalloc(&cp, xinf->fromaddr);
	ret = RESULT_COMPLETE;
	while (cp && markoffset(xinf->fromaddr) < markoffset(xinf->toaddr))
	{
		/* find the end of this line */
		for (lenln = 0; cp && *cp != '\n'; lenln++)
			(void)scannext(&cp);
		if (cp)
		{
			(void)scannext(&cp);
			lenln++;
		}

		/* move "fromaddr" to end of this line (start of next line) */
		thisln = *xinf->fromaddr;
		marksetoffset(xinf->fromaddr,
		    cp ? markoffset(scanmark(&cp)) : markoffset(xinf->toaddr));

		/* is this a selected line? */
		if ((regexec(xinf->re, &thisln, True) ? EX_GLOBAL : EX_VGLOBAL)
			== xinf->command)
		{

			/* move the cursor to the matching line */
			endln = markoffset(scanmark(&cp));
			marksetoffset(cursor, endln - lenln);

			/* free the scan pointer -- can't make changes
			 * while scanning.
			 */
			scanfree(&cp);

			/* execute the command */
			ret = exstring(xinf->window, xinf->rhs);

			/* reallocate the scan pointer */
			(void)scanalloc(&cp, xinf->fromaddr);

			/* if the ex command failed, then exit */
			if (ret != RESULT_COMPLETE)
				break;
		}

		/* if user wants to abort operation, then exit */
		if (guipoll(False))
			break;
	}
	scanfree(&cp);

	/* move the cursor back to its original position, and then return
	 * the final position so the cursor will be moved there in a graceful
	 * way.
	 */
	xinf->newcurs = markdup(cursor);
	if (markbuffer(cursor) != markbuffer(orig))
		marksetbuffer(cursor, markbuffer(orig));
	marksetoffset(cursor, markoffset(orig));
	markfree(orig);
	return ret;
}


RESULT	ex_join(xinf)
	EXINFO	*xinf;
{
	CHAR	prevchar;	/* character before newline */
	CHAR	*cp;		/* used while scanning for newlines */
	long	newlines;	/* number of newlines to be clobbered */
	long	nspaces;	/* number of spaces to insert */
	MARK	start, end;	/* region around a newline */
	long	offset;		/* position of last change */
	CHAR	*endlist;	/* string of sentence ending punctuation */
 static CHAR	spaces[3] = {' ', ' ', ' '};

	/* initialize "offset" just to silence a compiler warning */
	offset = 0;

	/* initialize endlist from options (if set) or literals */
	endlist = (o_sentenceend ? o_sentenceend : toCHAR(".!?"));

	/* We're going to be replacing newlines with blanks.  The number of
	 * newlines we want to replace is equal to the number lines affected
	 * minus one... except that if the user requested a "join" of a single
	 * line then we should assume we're supposed to join two lines.
	 */
	newlines = (xinf->from == xinf->to) ? 1 : (xinf->to - xinf->from);
	if (xinf->from + newlines > o_buflines(markbuffer(xinf->fromaddr)))
	{
		msg(MSG_ERROR, "nothing to join with this line");
		return RESULT_ERROR;;
	}

	/* scan the text for newlines */
	prevchar = ' ';
	for (scanalloc(&cp, xinf->fromaddr); cp && newlines > 0; scannext(&cp))
	{
		/* if newline, then clobber it */
		if (*cp == '\n')
		{
			start = markdup(scanmark(&cp));
			offset = markoffset(start);
			if (scannext(&cp))
			{
				/* figure out how many spaces to insert */
				if (xinf->bang || *cp == ')' || isspace(prevchar))
					nspaces = 0;
				else if (CHARchr(toCHAR(endlist), prevchar))
					nspaces = o_sentencegap;
				else
					nspaces = 1;

				/* skip any leading whitespace in next line */
				while (!xinf->bang && cp && (*cp == ' ' || *cp == '\t'))
				{
					scannext(&cp);
				}

				/* Find the end mark */
				end = (cp ? markdup(scanmark(&cp))
					  : markalloc(markbuffer(start), o_bufchars(markbuffer(start))));

				/* free the scan context during the change;
				 * can't mix scanning & updates.
				 */
				scanfree(&cp);

				/* replace the newline (and trailing whitespace)
				 * with a given number of spaces.
				 */
				bufreplace(start, end, spaces, nspaces);
				marksetoffset(end, offset + nspaces - 1);
					/* NOTE: the "- 1" is to compensate for
					 * the scannext() at the top of the loop
					 */

				/* resume scanning */
				scanalloc(&cp, end);
				markfree(end);
				if (nspaces > 0)
				{
					prevchar = ' ';
				}
			}
			markfree(start);

			/* All that to clobber one newline! */
			newlines--;
		}
		else if ((*cp != '"' && *cp != ')') || isspace(prevchar))
		{
			/* remember the character.  When we hit a newline,
			 * the previous character will be checked to determine
			 * how many spaces to insert.  Note that we ignore
			 * quote and parenthesis characters except after a
			 * blank.
			 */
			prevchar = *cp;
		}
	}
	scanfree(&cp);

	/* Choose a new cursor position: at the start of last joint */
	xinf->newcurs = markalloc(markbuffer(xinf->fromaddr), offset);

	return RESULT_COMPLETE;
}


RESULT	ex_move(xinf)
	EXINFO	*xinf;
{
	long	oldto;

	/* detect errors */
	if (markbuffer(xinf->fromaddr) == markbuffer(xinf->destaddr)
	 && markoffset(xinf->fromaddr) <= markoffset(xinf->destaddr)
	 && markoffset(xinf->destaddr) < markoffset(xinf->toaddr))
	{
		msg(MSG_ERROR, "destination can't be inside source");
		return RESULT_ERROR;
	}

	/* copy the text */
	xinf->newcurs = markdup(xinf->destaddr);
	markaddoffset(xinf->newcurs, 1);
	oldto = markoffset(xinf->toaddr);
	bufpaste(xinf->destaddr, xinf->fromaddr, xinf->toaddr);

	/* leave the cursor on the last line of the destination */
	if (markoffset(xinf->newcurs) > 1)
		markaddoffset(xinf->newcurs, -2);
	marksetoffset(xinf->newcurs, markoffset(
		(*dmnormal.move)(xinf->window, xinf->newcurs, 0L, 0L, False)));

	/* If moving (not copying) then delete source */
	if (xinf->command == EX_MOVE)
	{
		/* be careful about the "to" offset.  If the destination was
		 * immediately after the source, then we just inserted the
		 * text at "to", so "to" was adjusted... but we only want to
		 * delete up to the old value of "to".
		 */
		if (markbuffer(xinf->toaddr) == markbuffer(xinf->destaddr)
		 && markoffset(xinf->toaddr) == markoffset(xinf->destaddr))
		{
			marksetoffset(xinf->toaddr, oldto);
		}

		bufreplace(xinf->fromaddr, xinf->toaddr, NULL, 0);
	}

	return RESULT_COMPLETE;
}


RESULT	ex_print(xinf)
	EXINFO	*xinf;
{
	long	last;	/* offset of start of last line */
	PFLAG	pflag;	/* how to print */

	/* generate a pflag from the command name and any supplied pflag */
	switch (xinf->pflag)
	{
	  case PF_NONE:
	  case PF_PRINT:
		pflag = (xinf->command == EX_NUMBER
			    ? (xinf->command == EX_LIST ? PF_NUMLIST : PF_NUMBER)
			    : (xinf->command == EX_LIST ? PF_LIST : PF_PRINT));
		break;

	  case PF_LIST:
		pflag = (xinf->command == EX_NUMBER ? PF_NUMLIST : PF_LIST);
		break;

	  case PF_NUMBER:
		pflag = (xinf->command == EX_LIST ? PF_NUMLIST : PF_NUMBER);
		break;	/* !!it B4: this break seems to be needed here */

	  default:
		pflag = PF_NUMLIST;
		break;
	}

	/* print the lines */
	last = exprintlines(xinf->window, xinf->fromaddr, xinf->to - xinf->from + 1, pflag);

	/* leave the cursor at the start of the last line */
	xinf->newcurs = markalloc(markbuffer(xinf->fromaddr), last);
	return RESULT_COMPLETE;
}


RESULT	ex_put(xinf)
	EXINFO	*xinf;
{
	MARK	newcurs;

	newcurs = cutput(xinf->cutbuf, xinf->window, xinf->fromaddr, (BOOLEAN)(xinf->from > 0), False, False);
	if (newcurs)
	{
		xinf->newcurs = markdup(newcurs);
		return RESULT_COMPLETE;
	}
	return RESULT_ERROR;
}


RESULT	ex_read(xinf)
	EXINFO	*xinf;
{
	long	offset;

	if (!xinf->rhs && xinf->nfiles != 1)
	{
		msg(MSG_ERROR, "filename required");
		return RESULT_ERROR;
	}

	/* remember where we started inserting */
	offset = markoffset(xinf->toaddr);
	xinf->newcurs = markdup(xinf->toaddr);

	/* read in the text */
	if (!bufread(xinf->toaddr, xinf->rhs ? tochar8(xinf->rhs) : xinf->file[0]))
	{
		return RESULT_ERROR;
	}

	/* Choose a place to leave the cursor.  If reading due to visual <:>
	 * command, this should be the start of the first line read; else it
	 * should be the start of the last line read.
	 */
	if (xinf->window->state->flags & ELVIS_1LINE)
	{
		marksetoffset(xinf->newcurs, offset);
	}
	else
	{
		marksetoffset(xinf->newcurs, markoffset(
			(*xinf->window->md->move)(xinf->window, xinf->newcurs, -1, 0, False)));
	}
	return RESULT_COMPLETE;
}


/* This function implements the :< and :> commands.  It is also used to do
 * the real work for the visual <<> and <>> operators.
 */
RESULT	ex_shift(xinf)
	EXINFO	*xinf;
{
	long	shift;	/* amount to shift by */
	long	ws;	/* amount of whitespace currently */
	CHAR	*cp;	/* used for scanning through a line's whitespace */
	long	line;	/* used for counting through line numbers */
	MARKBUF	start;	/* start of the line */
	MARKBUF	end;	/* end of the line's whitespace */
	CHAR	str[50];/* buffer for holding whitespace */
	long	i;

	/* compute the amount of shifting required */
	shift = o_shiftwidth(markbuffer(&xinf->defaddr)) * xinf->multi;
	if (xinf->command == EX_SHIFTL)
	{
		shift = -shift;
	}

	/* for each line... */
	start = xinf->defaddr;
	for (line = xinf->from; line <= xinf->to; line++)
	{
		/* count the current whitespace */
		scanalloc(&cp, marksetline(&start, line));
		for (ws = 0; cp && (*cp == ' ' || *cp == '\t'); scannext(&cp))
		{
			if (*cp == ' ')
			{
				ws++;
			}
			else
			{
				ws = ws + o_tabstop(markbuffer(&xinf->defaddr))
					- ws % o_tabstop(markbuffer(&xinf->defaddr));
			}
		}
		end = *scanmark(&cp);

		/* if this is an empty line, and no ! was given on the command
		 * line, then do nothing to this line.
		 */
		if (ws == 0 && *cp == '\n' && !xinf->bang)
		{
			scanfree(&cp);
			continue;
		}
		scanfree(&cp);

		/* compute the amount of whitespace we want to have */
		ws += shift;
		if (ws < 0)
		{
			ws = 0;
		}

		/* Replace the old whitespace with new whitespace.  Since our
		 * buffer for holding new whitespace is of limited size, we
		 * may need to make several bufreplace() calls to do this.
		 */
		while (markoffset(&start) != markoffset(&end) || ws > 0)
		{
			/* build new whitespace (as much of it as possible) */
			i = 0;
			if (o_autotab(markbuffer(&xinf->defaddr)))
			{
				for (;
				     ws >= o_tabstop(markbuffer(&xinf->defaddr))
					&& i < QTY(str);
				     i++, ws -= o_tabstop(markbuffer(&xinf->defaddr)))
				{
					str[i] = '\t';
				}
			}
			for (; ws > 0 && i < QTY(str); i++, ws--)
			{
				str[i] = ' ';
			}

			/* replace old whitespace with new */
			bufreplace(&start, &end, str, i);
			markaddoffset(&start, i);
			marksetoffset(&end, markoffset(&start));
		}
	}

	if (xinf->to - xinf->from + 1 >= o_report)	/* !!it E5: report on shift */
		msg(MSG_INFO,"[dC]$1 lines $2ed",xinf->to - xinf->from + 1,(xinf->command == EX_SHIFTL)? '<' : '>');

	return RESULT_COMPLETE;
}


/* This function implements the :substitute command, and the :& and :~
 * variations of that command.  It is also used to perform the real work
 * of visual <&> command.
 */
RESULT	ex_substitute(xinf)
	EXINFO	*xinf;
{
	CHAR	*opt;	/* substitution options */
	long	chline;	/* # of lines changed */
	long	chsub;	/* # of substitutions made */
	long	cursoff;/* offset where cursor should be moved to */
 static PFLAG	pflag;	/* printing flag */
 static BOOLEAN	optg;	/* boolean option: substitute globally in line? */
 static BOOLEAN	optx;	/* boolean option: execute instead of substitute? */
 static long	count;	/* numeric option: which instance in each line to sub */
	MARKBUF	posn;	/* position within a line to be replaced */
	long	instance;/* number of instances matched so far within line */
	CHAR	*newp;	/* replacement text */
	CHAR	*scan;	/* used for scanning to find end of line */
	int	match;


	assert(xinf->command == EX_SUBSTITUTE || xinf->command == EX_SUBAGAIN);

	/* initialize "cursoff" just to silence a compiler warning */
	cursoff = 0;

	/* ":s" is equivalent to ":&". */
	if (!xinf->re)
		xinf->command = EX_SUBAGAIN;

	if (xinf->command == EX_SUBAGAIN)
	{
		/* same regular expression as last time */
		xinf->re = regcomp(toCHAR(""), xinf->window->state->cursor);
		if (!xinf->re)
		{
			/* error message already given by regcomp() */
			return RESULT_ERROR;
		}

		/* same replacement text as last time */
		newp = regtilde(toCHAR(o_magic ? "~" : "\\~"));

		/* if visual "&", then turn off the "p" and "c" options */
		if (xinf->bang)
		{
			pflag= PF_NONE;
		}
	}
	else /* xinf->command == CMD_SUBSTITUTE */
	{
		/* generate the new text */
		newp = regtilde(xinf->lhs ? xinf->lhs : toCHAR(""));

		/* analyse the option string */
		if (!o_edcompatible)
		{
			pflag = xinf->pflag;
			optg = optx = False;
			count = 0;
		}
		for (opt = xinf->rhs; opt && *opt; opt++)
		{
			switch (*opt)
			{
			  case 'g':
				optg = (BOOLEAN)!optg;
				break;

			  case 'x':
				optx = (BOOLEAN)!optx;
				break;

			  case 'p':
				pflag = (pflag==PF_PRINT) ? PF_PRINT : PF_NONE;
				break;

			  case 'l':
				pflag = (pflag==PF_LIST) ? PF_LIST : PF_NONE;
				break;

			  case '#':
				pflag = (pflag==PF_NUMBER) ? PF_NUMBER : PF_NONE;
				break;

			  case '0':
			  case '1':
			  case '2':
			  case '3':
			  case '4':
			  case '5':
			  case '6':
			  case '7':
			  case '8':
			  case '9':
				count = 0;
				do
				{
					count = count * 10 + *opt++ - '0';
				} while (isdigit(*opt));
				opt--;
				break;

			  default:
				if (!isspace(*opt))
				{
					msg(MSG_ERROR, "[C]unsupported flag '$1'", *opt);
					return RESULT_ERROR;
				}
			}
		}

		/* sanity checks */
		if (count != 0 && optg)
		{
			msg(MSG_ERROR, "can't mix number and 'g' flag");
			return RESULT_ERROR;
		}

		/* default behavior is to either replace only first instance,
		 * or all instances, depending on the "gdefault" option.
		 */
		if (count == 0 && !optg)
		{
			if (o_gdefault && !o_edcompatible)
				optg = True;
			else
				count = 1;
		}
	}

	/* if no replacement text, fail */
	if (!newp)
	{
		return RESULT_ERROR;
	}

	/* this command does its own printing; disable auto printing */
	xinf->pflag = PF_NONE;

	/* reset the change counters */
	chline = chsub = 0L;

	/* for each line in the range... */
	for (posn = *xinf->fromaddr; markoffset(&posn) < markoffset(xinf->toaddr); )
	{
		/* for each instance within the line... */
		for (instance = 0, match = regexec(xinf->re, &posn, True);
		     match && (optg || instance < count);
		     match = regexec(xinf->re, &posn, False))
		{
			/* increment the substitution change counter */
			chsub++;
			instance++;

			/* if this is an instance we care about... */
			if (optg || instance == count)
			{
				/* Either execute the replacement, or perform
				 * the substitution
				 */
				opt = regsub(xinf->re, newp, (BOOLEAN)!optx);
				if (!opt)
				{
					return RESULT_ERROR;
				}
				if (optx)
				{
					exstring(xinf->window, opt);
				}
				safefree(opt);

				/* remember the offset of this change so we can
				 * move the cursor there later.
				 */
				cursoff = xinf->re->startp[0];
			}

			/* Move "posn" to the end of the matched region.  If
			 * the regexp could conceivably match a zero-length
			 * string, then skip one character.
			 */
			marksetoffset(&posn, xinf->re->endp[0]);
			if (xinf->re->minlen == 0)
			{
				/* markaddoffset(&posn, 1);*/
				if (scanchar(&posn) == '\n')
					break;
			}
		}

		/* if any changes were made, then increment chline */
		if (optg ? instance > 0 : instance == count)
		{
			chline++;
		}

		/* scan forward for the end of the line */
		for (scanalloc(&scan, &posn); scan && *scan != '\n'; scannext(&scan))
		{
		}
		if (scan)
			scannext(&scan);
		if (scan)
			posn = *scanmark(&scan);
		else
			marksetoffset(&posn, o_bufchars(markbuffer(xinf->fromaddr)));
		scanfree(&scan);
	}

	/* If done from within a ":g" command, or used with "x" flag,
	 * then finish silently.
	 */
	if (xinf->global || optx)
	{
#if 0
		rptlines = chline;
		rptlabel = "changed";
#endif
		return RESULT_COMPLETE;
	}

	/* Reporting */
	if (chsub == 0)
	{
		msg(MSG_WARNING, "substitution failed");
	}
	else if (chline >= o_report)
	{
		msg(MSG_INFO, "[dd]$1 substitutions on $2 lines", chsub, chline);
		xinf->newcurs = markalloc(xinf->re->buffer, cursoff);
	}
	return RESULT_COMPLETE;
}


RESULT	ex_undo(xinf)
	EXINFO	*xinf;
{
	long	l = 1;

	assert(xinf->command == EX_UNDO || xinf->command == EX_REDO);

	/* choose an undo/redo level to recover */
	if (xinf->lhs)
	{
		if (!calcnumber(xinf->lhs) || (l = CHAR2long(xinf->lhs)) < 1)
		{
			msg(MSG_ERROR, "bad undo level");
			return RESULT_ERROR;
		}
	}

	/* if redo, then negate the undo value */
	if (xinf->command == EX_REDO)
		l = -l;

	/* try to revert to the undo level */
	l = bufundo(xinf->window->cursor, l);

	/* if successful, adjust the cursor position */
	if (l >= 0)
	{
		marksetoffset(xinf->window->cursor, l);
		return RESULT_COMPLETE;
	}

	return RESULT_ERROR;
}


RESULT	ex_write(xinf)
	EXINFO	*xinf;
{
	char	*name;
	BOOLEAN	success;

	if (xinf->rhs)
	{
		name = tochar8(xinf->rhs);
	}
	else if (xinf->nfiles >= 1)
	{
		assert(xinf->nfiles == 1);
		name = xinf->file[0];
	}
	else
	{
		name = tochar8(o_filename(markbuffer(xinf->fromaddr)));
		if (!name)
		{
			msg(MSG_ERROR, "no file name");
			return RESULT_ERROR;	/* nishi */
		}
	}

	/* if writing to a different filename, remember that name */
	if (name[0] != '!'
	 && o_filename(markbuffer(xinf->fromaddr))
	 && CHARcmp(name, o_filename(markbuffer(xinf->fromaddr))))
	{
		optprevfile(toCHAR(name), 1);
	}

	/* actually write the file */
	success = bufwrite(xinf->fromaddr, xinf->toaddr, name, xinf->bang);
	return success ? RESULT_COMPLETE : RESULT_ERROR;
}


RESULT	ex_z(xinf)
	EXINFO	*xinf;
{
	CHAR	type = '+';		/* type of window to show */
	long	count = o_window;	/* number of lines to show */
	PFLAG	pflag = PF_PRINT;	/* how to show */
	long	line = xinf->from;	/* first line to show */
	long	offset;
	CHAR	*scan;

	/* If we were given arguments, then parse them */
	for (scan = xinf->rhs; scan && *scan; scan++)
	{
		switch (*scan)
		{
		  case '-':
		  case '+':
		  case '.':
		  case '^':
		  case '=':
			type = *scan;
			break;

		  case '0':
		  case '1':
		  case '2':
		  case '3':
		  case '4':
		  case '5':
		  case '6':
		  case '7':
		  case '8':
		  case '9':
			for (count = 0; isdigit(*scan); scan++)
			{
				count = count * 10 + *scan - '0';
			}
			scan--; /* we went one character too far */
			break;

		  case 'l':
			if (pflag == PF_NUMBER || pflag == PF_NUMLIST)
				pflag = PF_NUMLIST;
			else
				pflag = PF_LIST;
			break;

		  case '#':
			if (pflag == PF_LIST || pflag == PF_NUMLIST)
				pflag = PF_NUMLIST;
			else
				pflag = PF_NUMBER;
			break;

		  case ' ':
		  case '\t':
		  case 'p':
			/* ignore */
			break;

		  default:
			msg(MSG_ERROR, "bad argument to :z");
			return RESULT_ERROR;
		}
	}

	/* choose the first line, based on the type */
	switch (type)
	{
	  case '-': /* show the given line at the bottom */
		line = xinf->from - count + 1;
		break;

	  case '+': /* show the given line at the top */
		line = xinf->from;
		break;

	  case '.': /* show the given line in the middle */
		line = xinf->from - count/2;
		break;

	  case '^': /* show the window before the current line */
		line = xinf->from - count * 2 + 1;
		break;
		
	  case '=': /* show it in the middle, surrounded by lines of hyphens */
		count -= 2;
		line = xinf->from - count / 2;
		break;
	}

	/* protect against readed past top or bottom of buffer */
	if (line < 1)
	{
		count -= 1 - line;
		line = 1;
	}
	if (line + count > o_buflines(markbuffer(xinf->fromaddr)))
	{
		count -= line - o_buflines(markbuffer(xinf->fromaddr));
	}

	/* construct a mark for the first line */
	xinf->newcurs = markdup(xinf->fromaddr);
	marksetline(xinf->newcurs, line);

	/* print the lines */
	if (type == '=')
	{
		/* for '=', we need to add lines of hyphens around given line */
		if (line < xinf->from)
		{
			exprintlines(xinf->window, xinf->newcurs, xinf->from - line, pflag);
		}
		drawextext(xinf->window, toCHAR("-------------------------------------------------------------------------------\n"), 80);
		exprintlines(xinf->window, xinf->fromaddr, 1, pflag);
		drawextext(xinf->window, toCHAR("-------------------------------------------------------------------------------\n"), 80);
		count -= (xinf->from - line) + 1;
		if (count > 0)
		{
			marksetline(xinf->newcurs, xinf->from + 1);
			exprintlines(xinf->window, xinf->newcurs, count, pflag);
		}

		/* leave the cursor on the given line */
		marksetoffset(xinf->newcurs, markoffset(xinf->fromaddr));
	}
	else
	{
		/* print the lines all at once */
		offset = exprintlines(xinf->window, xinf->newcurs, count, pflag);

		/* leave the cursor at the start of the last line */
		marksetoffset(xinf->newcurs, offset);
	}

	return RESULT_COMPLETE;
}
