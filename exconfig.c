/* exconfig.c */
/* Copyright 1995 by Steve Kirkendall */

char id_exconfig[] = "$Id: exconfig.c,v 2.50 1996/09/21 02:12:31 steve Exp $";

#include "elvis.h"


static BOOLEAN thenflag;	/* set by ":if", tested by ":then" & ":else" */



RESULT	ex_args(xinf)
	EXINFO	*xinf;
{
	int	i, col, len;
	char	**tmp;

	/* were we given a new args list? */
	if (xinf->nfiles > 0)
	{
		/* yes, use it */
		tmp = arglist;
		arglist = xinf->file;
		xinf->file = tmp;
		for (xinf->nfiles = 0; xinf->file[xinf->nfiles]; xinf->nfiles++)
		{
		}
		argnext = 0;
	}
	else
	{
		/* show current args list */
		for (i = col = 0; arglist[i]; i++)
		{
			len = strlen(arglist[i]);

			/* whitespace between args */
			if (i == 0)
				; /* no space is needed */
			else if (col + len + 4 > o_columns(xinf->window))
			{
				drawextext(xinf->window, toCHAR("\n"), 1);
				col = 0;
			}
			else
			{
				drawextext(xinf->window, toCHAR(" "), 1);
				col++;
			}

			/* Output the arg.  If current, enclose in '[' */
			if (i == argnext - 1)
				drawextext(xinf->window, toCHAR("["), 1);
			drawextext(xinf->window, toCHAR(arglist[i]), len);
			if (i == argnext - 1)
				drawextext(xinf->window, toCHAR("]"), 1);
			col += len;
		}

		/* the final newline */
		drawextext(xinf->window, toCHAR("\n"), 1);
	}
	return RESULT_COMPLETE;
}


/* This table stores the names of the fonts, and the colors assigned to
 * them.  Initially the colors are NULL.  A foreground and background are
 * stored for each, although some GUIs don't allow that much control.
 */
static struct
{
	char	*name;	/* name of the font to be colorized */
	CHAR	*fg;	/* name of foreground color */
	CHAR	*bg;	/* name of background color */
} colortbl[] =
{
	{"normal"}, {"bold"}, {"emphasized"}, {"italic"},
	{"underlined"}, {"fixed"}, {"cursor"}, {"standout"},
	{"scrollbar"} /* "scrollbar" must be last */
};


/* This function implements the :color command. */
RESULT	ex_color(xinf)
	EXINFO	*xinf;
{
	CHAR	*args, *cp, *bg;
	int	i, j;

	if (!gui->color)
	{
		msg(MSG_ERROR, "no color support");
		return RESULT_ERROR;
	}

	if (!xinf->rhs)
	{
		/* list any colors which have been set */
		for (i = j = 0; i < QTY(colortbl); i++)
		{
			if (colortbl[i].fg && colortbl[i].bg)
				msg(MSG_INFO, "[sSS]color $1 $2 on $3",
					colortbl[i].name, colortbl[i].fg,
					colortbl[i].bg);
			else if (colortbl[i].fg)
				msg(MSG_INFO, "[sS]color $1 $2",
					colortbl[i].name, colortbl[i].fg);
			else
				j++;
		}
		if (i == j)
			msg(MSG_WARNING, "no colors have been set");
		return RESULT_COMPLETE;
	}

	/* PARSE ARGS INTO FONT, FOREGROUND & BACKGROUND */

	/* font... */
	args = xinf->rhs;
	if (!CHARncmp(args, toCHAR("s "), 2) && gui->scrollbar)
	{
		i = QTY(colortbl) - 1;
	}
	else
	{
		for (i = 0;
		     i < QTY(colortbl)
			&& (CHARncmp(toCHAR(colortbl[i].name), args, strlen(colortbl[i].name)) || args[strlen(colortbl[i].name)] != ' ')
			&& !(args[0] == (CHAR)colortbl[i].name[0] && args[1] == ' ');
		     i++)
		{
		}
	}
	if (i >= QTY(colortbl))
	{
		i = 0; /* if no name given, assume "normal" */
	}
	else
	{
		/* skip past the name */
		while (*args != ' ')
		{
			args++;
		}
		while (*args == ' ')
		{
			args++;
		}
	}

	/* foreground & background... */
	for (bg = args; *bg && strncmp(tochar8(bg), " on ", 4); bg++)
	{
	}
	if (*bg)
	{
		*bg = '\0';
		for (bg += 4; *bg && isspace(*bg); bg++)
		{
		}
	}
	if (!*bg)
	{
		bg = (CHAR *)0;
	}

	/* trim trailing whitespace */
	for (cp = args + CHARlen(args); --cp >= args && isspace(*cp); )
	{
		*cp = '\0';
	}
	if (!*args)
	{
		msg(MSG_ERROR, "missing foreground color");
		return RESULT_ERROR;
	}

	/* call the GUI's color function */
	if ((*gui->color)(xinf->window ? xinf->window->gw : (GUIWIN *)0, colortbl[i].name[0], args, bg))
	{
		/* redraw the window */
		if (xinf->window)
		{
			xinf->window->di->logic = DRAW_SCRATCH;
		}

		/* remember the chosen colors */
		if (colortbl[i].fg)
			safefree(colortbl[i].fg);
		colortbl[i].fg = args ? CHARdup(args) : NULL;
		if (colortbl[i].bg)
			safefree(colortbl[i].bg);
		colortbl[i].bg = bg ? CHARdup(bg) : NULL;

		return RESULT_COMPLETE;
	}
	return RESULT_ERROR;
}


void colorsave(custom)
	BUFFER	custom;	/* where to stuff the color commands */
{
	MARKBUF	m;
	int	i;
	char	tmp[80];

	/* try to make colors GUI-sensitive */
	sprintf(tmp, "if gui==\"%s\"\n", tochar8(o_gui));
	bufreplace(marktmp(m, custom, o_bufchars(custom)), &m, toCHAR(tmp), (long)strlen(tmp));

	/* store the color commands */
	for (i = 0; i < QTY(colortbl); i++)
	{
		if (colortbl[i].fg)
		{
			sprintf(tmp, "then color %s %s", colortbl[i].name, tochar8(colortbl[i].fg));
			if (colortbl[i].bg)
			{
				strcat(tmp, " on ");
				strcat(tmp, tochar8(colortbl[i].bg));
			}
			strcat(tmp, "\n");
			bufreplace(marktmp(m, custom, o_bufchars(custom)), &m, toCHAR(tmp), (long)strlen(tmp));
		}
	}
}


RESULT	ex_comment(xinf)
	EXINFO	*xinf;
{
	CHAR	*result;

	assert(xinf->command == EX_COMMENT || xinf->command == EX_ECHO
		|| xinf->command == EX_CALC || xinf->command == EX_GOTO);

	if (xinf->command == EX_ECHO && xinf->rhs)
	{
		drawextext(xinf->window, xinf->rhs, (int)CHARlen(xinf->rhs));
		drawextext(xinf->window, toCHAR("\n"), 1);
	}
	else if (xinf->command == EX_CALC && xinf->rhs)
	{
		result = calculate(xinf->rhs, NULL, False);
		if (!result)
		{
			return RESULT_ERROR;
		}
		drawextext(xinf->window, result, (int)CHARlen(result));
		drawextext(xinf->window, toCHAR("\n"), 1);
	}
	else if (xinf->command == EX_GOTO && xinf->fromaddr)
	{
		if (xinf->fromoffset > markoffset(xinf->fromaddr)
		 && xinf->fromoffset <= markoffset(xinf->toaddr))
			xinf->newcurs = markalloc(markbuffer(xinf->fromaddr), xinf->fromoffset);
		else
			xinf->newcurs = markdup(xinf->fromaddr);
	}
	return RESULT_COMPLETE;
}


RESULT	ex_digraph(xinf)
	EXINFO	*xinf;
{
	digaction(xinf->window, xinf->bang, xinf->rhs);
	return RESULT_COMPLETE;
}


RESULT	ex_display(xinf)
	EXINFO	*xinf;
{
	if (xinf->command == EX_DISPLAY && !xinf->rhs)
	{
		displist(xinf->window);
	}
	else if (!dispset(xinf->window, tochar8(xinf->rhs)))
	{
		return RESULT_ERROR;
	}
	xinf->window->di->logic = DRAW_CHANGED;
	return RESULT_COMPLETE;
}


RESULT	ex_gui(xinf)
	EXINFO	*xinf;
{
	if (!gui->guicmd)
	{
		msg(MSG_ERROR, "gui-specific commands not supported");
		return RESULT_ERROR;
	}

	return (*gui->guicmd)(xinf->window ? xinf->window->gw : NULL, tochar8(xinf->rhs))
		? RESULT_COMPLETE
		: RESULT_ERROR;
}


RESULT	ex_help(xinf)
	EXINFO	*xinf;
{
#ifndef DISPLAY_MARKUP
	msg(MSG_ERROR, "help unavailable; html mode is disabled");
	return RESULT_ERROR;
#else
	CHAR	*topic;	/* topic to search for; a tag name */
	char	*section;/* name of help file containing topic */
	CHAR	*tag;	/* name of tag to search for -- section#topic */
	BUFFER	buf;	/* buffer containing help text */
	MARK	tagdefn;/* result of search; where cursor should move to */
	int	i;

	/* construct a tag name for the requested topic */
	topic = NULL;
	if (!xinf->lhs)
	{
		/* :help */
		topic = toCHAR("CONTENTS");
		section = "elvis.html";
	}
	else if (!CHARcmp(xinf->lhs, toCHAR("ex")))
	{
		/* :help ex */
		section = "elvisex.html";
	}
	else if (!CHARcmp(xinf->lhs, toCHAR("vi")))
	{
		/* :help vi */
		section = "elvisvi.html";
	}
	else if ((!CHARncmp(xinf->lhs, toCHAR("se"), 2)
			|| !CHARncmp(xinf->lhs, toCHAR(":se"), 3))
		&& xinf->rhs)
	{
		/* :help set optionname */
		topic = optname(xinf->rhs);
		if (!topic)
			topic = toCHAR("GROUP");
		section = "elvisopt.html";
	}
	else if (CHARlen(xinf->lhs) > 1 && xinf->lhs[0] == ':')
	{
		/* :help :exname */
		topic = exname(xinf->lhs + 1);
		if (!topic)
			topic = toCHAR("GROUP");
		section = "elvisex.html";
	}
	else if ((topic = viname(xinf->lhs)) != NULL)
	{
		/* :help c  (where c is a vi command, usually single-char) */
		section = "elvisvi.html";
	}
	else if ((topic = optname(xinf->lhs)) != NULL)
	{
		/* :help optionname */
		section = "elvisopt.html";
	}
	else if ((topic = exname(xinf->lhs)) != NULL)
	{
		/* :help exname */
		section = "elvisex.html";
	}
	else
	{
		/* Can't tell what user is looking for; perhaps the user
		 * doesn't know the syntax of :help ?  Teach them!
		 */
		topic = toCHAR("help");
		section = "elvisex.html";
	}

	/* if help text not found, then give up */
	buf = bufpath(o_elvispath, section, toCHAR(section));
	if (!buf)
	{
		msg(MSG_ERROR, "[s]help not available; couldn't load $1", section);
		return RESULT_ERROR;
	}

	/* help text uses "html" display mode */
	if (optflags(o_bufdisplay(buf)) & OPT_FREE)
	{
		safefree(o_bufdisplay(buf));
		optflags(o_bufdisplay(buf)) &= ~OPT_FREE;
	}
	o_bufdisplay(buf) = toCHAR("html");

	/* combine section name and topic name to form a tag */
	if (topic)
	{
		tag = safealloc((int)(CHARlen(o_filename(buf)) + CHARlen(topic) + 2), sizeof(CHAR));
		CHARcpy(tag, o_filename(buf));
		CHARcat(tag, toCHAR("#"));
		CHARcat(tag, topic);
	}
	else
	{
		tag = CHARdup(toCHAR(section));
	}

	/* perform tag lookup to find the the topic in the help file */
	tagdefn = (*dmhtml.tagload)(tag, NULL);
	if (!tagdefn)
	{
		msg(MSG_ERROR, "[S]no help available for $1", topic);
		safefree(tag);
		return RESULT_ERROR;
	}
	safefree(tag);

	/* Try to create a new window for the help text.  If that doesn't
	 * work, then use the original window and push the old cursor onto
	 * the tag stack.
	 */
	markbuffer(tagdefn)->changepos = markoffset(tagdefn);
	if ((*gui->creategw)(tochar8(o_bufname(markbuffer(tagdefn))), ""))
	{
		return RESULT_COMPLETE;
	}

	/* push the current cursor position and display mode onto tag stack */
	if (o_tagstack &&
		!o_internal(markbuffer(xinf->window->cursor)) &&
		o_filename(markbuffer(xinf->window->cursor)))
	{
		for (i = TAGSTK - 1; i > 0; i--)
		{
			xinf->window->tagstack[i] = xinf->window->tagstack[i - 1];
		}
		xinf->window->tagstack[0].prevtag = (o_previoustag ? CHARdup(o_previoustag) : NULL);
		xinf->window->tagstack[0].origin = markdup(xinf->window->cursor);
		xinf->window->tagstack[0].display = xinf->window->md->name;
	}

	/* arrange for the cursor to move to the tag position */
	xinf->newcurs = markdup(tagdefn);
	return RESULT_COMPLETE;
#endif
}


/* Evaluate an expression, and set the "then" flag according to result */
RESULT	ex_if(xinf)
	EXINFO	*xinf;
{
	CHAR	*result;

	/* expression is required */
	if (!xinf->rhs)
	{
		msg(MSG_ERROR, "missing rhs");
		return RESULT_ERROR;
	}

	/* evaluate expression */
	result = calculate(xinf->rhs, NULL, (BOOLEAN)(xinf->command == EX_EVAL));
	if (!result)
	{
		return RESULT_ERROR;
	}

	if (xinf->command == EX_IF)
	{
		/* set "thenflag" based on result of evaluation */
		thenflag = calctrue(result);
		return RESULT_COMPLETE;
	}
	else /* command == EX_EVAL */
	{
		/* execute the result as an ex command */
		return exstring(xinf->window, result);
	}
}

RESULT	ex_then(xinf)
	EXINFO	*xinf;
{
	BOOLEAN origthen;
	RESULT	result;

	origthen = thenflag;
	result = RESULT_COMPLETE;

	/* Execute commands, if "thenflag" is set appropriately */
	if (xinf->command == EX_THEN ? thenflag : !thenflag)
	{
		result = exstring(xinf->window, xinf->rhs);
	}

	/* If the command line was enclosed in { .... } (as indicated by the
	 * presence of a newline; without curlies the command couldn't possibly
	 * contain any newlines) then restore the "thenflag" to its original
	 * value.
	 */
	if (CHARchr(xinf->rhs, '\n'))
	{
		thenflag = origthen;
	}

	return result;
}




/* This implemented :map, :unmap, :abbr, :unabbr, :break, and :unbreak */
RESULT	ex_map(xinf)
	EXINFO	*xinf;
{
	CHAR	*line;
	MAPFLAGS flags;
	int	len;

	assert(xinf->command == EX_MAP || xinf->command == EX_ABBR
		|| xinf->command == EX_UNMAP || xinf->command == EX_UNABBR
		|| xinf->command == EX_BREAK || xinf->command == EX_UNBREAK);

	/* check for missing mandatory arguments */
	if ((xinf->command == EX_MAP || xinf->command == EX_ABBR)
		&& xinf->lhs && !xinf->rhs)
	{
		msg(MSG_ERROR, "missing rhs");
		return RESULT_ERROR;
	}
	if ((xinf->command == EX_UNMAP || xinf->command == EX_UNABBR
		   || xinf->command == EX_BREAK || xinf->command == EX_UNBREAK)
		&& !xinf->lhs)
	{
		msg(MSG_ERROR, "missing lhs");
		return RESULT_ERROR;
	}

	/* choose which flags to map */
	if (xinf->command == EX_ABBR || xinf->command == EX_UNABBR)
	{
		flags = MAP_ABBR|(xinf->bang ? MAP_COMMAND : MAP_INPUT);
	}
	else if (xinf->rhs && !CHARncmp(xinf->rhs, toCHAR("visual "), 7))
	{
		flags = (MAP_INPUT|MAP_ASCMD|MAP_OPEN);
		if (!xinf->bang)
			flags |= MAP_COMMAND;
	}
	else
	{
		flags = xinf->bang ? (MAP_INPUT|MAP_OPEN) : MAP_COMMAND;
	}

	/* either list, unmap, or map */
	if (!xinf->lhs)
	{
		while ((line = maplist(flags & (MAP_INPUT|MAP_COMMAND|MAP_ABBR), &len)) != (CHAR *)0)
		{
			/* can't list maps before the first window is created */
			if (xinf->window)
				drawextext(xinf->window, line, len);
		}
	}
	else if (!xinf->rhs)
	{
		(void)mapdelete(xinf->lhs, (int)CHARlen(xinf->lhs), flags,
			(BOOLEAN)(xinf->command == EX_UNMAP || xinf->command == EX_UNABBR),
			(BOOLEAN)(xinf->command == EX_BREAK));
	}
	else 
	{
		mapinsert(xinf->lhs, (int)CHARlen(xinf->lhs), xinf->rhs, (int)CHARlen(xinf->rhs), (CHAR *)0, flags);
	}
	return RESULT_COMPLETE;
}


RESULT	ex_set(xinf)
	EXINFO	*xinf;
{
	CHAR	outbuf[5000];
	static CHAR empty[1];
	int	i;

	if (xinf->command == EX_LET)
	{
		i = 0;
		if (!xinf->rhs)
		{
			goto MissingRHS;
		}

		/* copy name into outbuf[], so we can nul-terminate it */
		for ( ; xinf->rhs && isalnum(xinf->rhs[i]); i++)
		{
			outbuf[i] = xinf->rhs[i];
		}
		outbuf[i] = '\0';

		/* skip whitespace */
		while (isspace(xinf->rhs[i]))
		{
			i++;
		}

		/* skip '=' */
		if (xinf->rhs[i] != '=' || !outbuf[0])
		{
			goto MissingRHS;
		}
		i++;

		/* skip whitespace after the '=' */
		while (isspace(xinf->rhs[i]))
		{
			i++;
		}
		if (!xinf->rhs[i])
		{
MissingRHS:
			msg(MSG_ERROR, "missing rhs");
			return RESULT_ERROR;
		}

		/* evaluate & store the result */
		if (!optputstr(outbuf, calculate(&xinf->rhs[i], NULL, False)))
		{
			return RESULT_ERROR;
		}
	}
	else /* command == EX_SET */
	{
		if (!optset(xinf->bang, xinf->rhs ? xinf->rhs : empty, outbuf, QTY(outbuf)))
		{
			return RESULT_ERROR;
		}
		if (*outbuf)
		{
			drawextext(windefault, outbuf, (int)CHARlen(outbuf));
		}
	}
	return RESULT_COMPLETE;
}


RESULT	ex_version(xinf)
	EXINFO	*xinf;
{
	msg(MSG_INFO, "[s]elvis $1", VERSION);
#ifdef COPY1
	msg(MSG_INFO, "[s]$1", COPY1);
#endif
#ifdef COPY2
	msg(MSG_INFO, "[s]$1", COPY2);
#endif
#ifdef COPY3
	msg(MSG_INFO, "[s]$1", COPY3);
#endif
#ifdef COPY4
	msg(MSG_INFO, "[s]$1", COPY4);
#endif
#ifdef PORTEDBY
	msg(MSG_INFO, "[s]$1", PORTEDBY);
#endif
	return RESULT_COMPLETE;
}


RESULT	ex_qall(xinf)
	EXINFO	*xinf;
{
	WINDOW	win;
	WINDOW	orig;
	RESULT	result;
	BOOLEAN	didorig;

	assert(xinf->command == EX_QALL || xinf->command == EX_PRESERVE);

	/* If :preserve, then turn off the tempsession flag */
	if (xinf->command == EX_PRESERVE)
	{
		o_tempsession = False;
	}

	/* Perform a :quit command on each window in turn. */
	xinf->command = EX_QUIT;
	orig = xinf->window;
	for (win = winofbuf(NULL, NULL), result = RESULT_COMPLETE, didorig = False;
	     win;
	     win = winofbuf(win, NULL))
	{
		xinf->window = win;
		if (ex_xit(xinf) != RESULT_COMPLETE)
		{
			result = RESULT_ERROR;
		}
		else if (win != orig)
		{
			/* Need to explicitly delete all windows except the
			 * current one.  The current one will go away
			 * automatically when the ex_qall() function exits.
			 */
			(*gui->destroygw)(win->gw, False);
			win = didorig ? orig : NULL;
		}
		else
		{
			didorig = True;
		}
	}
	return result;
}


RESULT	ex_xit(xinf)
	EXINFO	*xinf;
{
	BUFFER	buf;		/* the buffer to be saved */
	MARKBUF	top, bottom;
	STATE	*state;
	BUFFER	b;		/* other buffers */
 static	long	morechgs;	/* change counter at last "more files" warning */
 static	BUFFER	morebuf;	/* buffer which morechgs value refers to */

	assert(xinf->command == EX_CLOSE || xinf->command == EX_QUIT
		|| xinf->command == EX_XIT || xinf->command == EX_WQUIT);

	/* Save the buffer, if :wquit or modified and :xit */
	buf = markbuffer(xinf->window->cursor);
	if (xinf->command == EX_WQUIT ||
		(o_modified(buf) && xinf->command == EX_XIT))
	{
		/* Write to named file or the buffer's original file.
		 * If can't write, then fail.
		 */
		if (xinf->nfiles == 1
			? !bufwrite(marktmp(top, buf, 0), marktmp(bottom, buf, o_bufchars(buf)), xinf->file[0], xinf->bang)
			: !bufsave(buf, xinf->bang, True))
		{
			/* an error message has already been output */
			return RESULT_ERROR;
		}
	}

	/* if :q without a !, then complain if the buffer is modified */
	if (xinf->command == EX_QUIT && !xinf->bang && o_modified(buf))
	{
		msg(MSG_ERROR, "[S]$1 modified, not saved", o_bufname(buf));
		return RESULT_ERROR;
	}

	/* If this is the last window, then make sure *ALL* user buffers
	 * have been saved.  Exception: If :close! and this session isn't
	 * temporary, then we don't need to check buffers.
	 */
	if ((morebuf != markbuffer(xinf->window->cursor) || morebuf->changes != morechgs)
	 && !(xinf->command == EX_CLOSE && xinf->bang && o_tempsession)
	 && winofbuf(NULL, NULL) == xinf->window && !winofbuf(xinf->window, NULL))
	{
		/* remember some stuff which should prevent us from warning
		 * the user more than once.
		 */
		morebuf = markbuffer(xinf->window->cursor);
		morechgs = morebuf->changes;

		/* check all buffers */
		for (b = buffers; b; b = buflist(b))
		{
			if (!o_internal(b) && o_modified(b)
				&& (xinf->command == EX_CLOSE || b != buf))
			{
				/* We've found a modified, unsaved user buffer.
				 * If the command is :q! then we want to
				 * discard all buffers; otherwise we want to
				 * warn the user that other buffers need to
				 * be checked.
				 */
				if (xinf->command == EX_QUIT && xinf->bang)
					o_modified(b) = False;
				else
				{
					msg(MSG_ERROR, "check other buffers");
					return RESULT_ERROR;
				}
			}
		}

		/* also check for more files to edit */
		if (arglist && (argnext < 0 || arglist[argnext])
		 && (xinf->command != EX_QUIT || !xinf->bang))
		{
			msg(MSG_WARNING, "more files");
			return RESULT_ERROR;
		}
	}

	/* Arrange for the state stack to pop everything */
	for (state = xinf->window->state; state; state = state->pop)
	{
		state->flags |= ELVIS_POP;
	}
	return RESULT_COMPLETE;
}
