/* exconfig.c */
/* Copyright 1995 by Steve Kirkendall */

char id_exconfig[] = "$Id: exconfig.c,v 2.84 1999/03/11 17:55:46 steve Exp $";

#include "elvis.h"



BOOLEAN	exthenflag;	/* set by ":if", tested by ":then" & ":else" */
CHAR	*exdotest;	/* set by ":while", tested by ":do" */



#ifdef FEATURE_ALIAS
/* These are used for storing aliases */
typedef struct alias_s
{
	struct alias_s	*next;		/* some other alias */
	char		*name;		/* name of this alias */
	CHAR		*command;	/* commands for this alias */
	BOOLEAN		inuse;		/* is this alias already being run? */
} alias_t;


BEGIN_EXTERNC
static void listalias P_((WINDOW win, alias_t *alias, BOOLEAN shortformat));
static void buildarg P_((CHAR **cmd, CHAR *arg, long len, CHAR *defarg, long deflen, BOOLEAN quote));
END_EXTERNC
static alias_t	*aliases;	/* This is the head of a list of aliases */


/* look up a name in the alias list.  The name can be terminated with any
 * non-alphanumeric character, not just '\0'.  Return its name if alias,
 * or NULL otherwise.  Optionally ignore if already in use.
 i
 */
char *exisalias(name, inuse)
	char	*name;	/* name of a command, maybe an alias */
	BOOLEAN	inuse;	/* find even if in use? (else hide in-use aliases) */
{
	alias_t	*alias;

	/* look for alias */
	for (alias = aliases; alias; alias = alias->next)
		if (!strcmp(name, alias->name))
			return (alias->inuse && !inuse) ? NULL : alias->name;
	return NULL;
}


/* list a single alias */
static void listalias(win, alias, shortformat)
	WINDOW	win;
	alias_t	*alias;
	BOOLEAN	shortformat;
{
	CHAR	ch[4];
	CHAR	*start;
	int	len;

	drawextext(win, toCHAR(alias->name), strlen(alias->name));
	if (CHARchr(alias->command, '\n') == alias->command + CHARlen(alias->command) - 1)
	{
		/* single-line command simply follows the alias name */
		drawextext(win, blanks, 10 - (CHARlen(alias->name) % 10));
		drawextext(win, alias->command, CHARlen(alias->command));
	}
	else if (shortformat)
	{
		/* multi-line command, but only show first line */
		drawextext(win, blanks, 10 - (CHARlen(alias->name) % 10));
		ch[0] = '{';
		ch[1] = ' ';
		drawextext(win, ch, 2);
		for (start = alias->command, len = 0;
		     *start++ != '\n' && len < o_columns(win) - 16;
		   len++)
		{
		}
		drawextext(win, alias->command, len);
		ch[0] = ch[1] = ch[2] = '.';
		ch[3] = '\n';
		drawextext(win, ch, 4);
	}
	else
	{
		/* multi-line command is output in a fancy way */
		ch[0] = ' ';
		ch[1] = '{';
		ch[2] = '\n';
		drawextext(win, ch, 3);
		for (start = alias->command, len = 0; *start; len++)
		{
			if (start[len] == '\n')
			{
				drawextext(win, blanks, 10);
				drawextext(win, start, len);
				ch[0] = '\n';
				drawextext(win, ch, 1);
				start += len + 1;
				len = -1; /* will be incremented to 0 by for()*/
			}
		}
		ch[0] = '}';
		ch[1] = '\n';
		drawextext(win, ch, 2);
	}
}


/* maintain the alias list */
RESULT	ex_alias(xinf)
	EXINFO	*xinf;
{
	alias_t	*newalias, *alias, *lag;
	int	i;

	/* if no aliases named, then list all */
	if (!xinf->lhs)
	{
		for (alias = aliases; alias; alias = alias->next)
		{
			listalias(xinf->window, alias, True);
		}
		return RESULT_COMPLETE;
	}

	/* Try to find the named alias */
	for (lag = NULL, alias = aliases;
	     alias && CHARcmp(xinf->lhs, toCHAR(alias->name));
	     lag = alias, alias = alias->next)
	{
	}

	/* Unaliasing? */
	if (xinf->command == EX_UNALIAS)
	{
		if (alias)
		{
			/* safety check */
			if (alias->inuse)
			{
				msg(MSG_ERROR, "[s]can't unalias $1 because it is in use", alias->name);
				return RESULT_ERROR;
			}

			/* remove it from the list, and free it */
			if (lag)
				lag->next = alias->next;
			else
				aliases = alias->next;
			safefree(alias->name);
			safefree(alias->command);
			safefree(alias);
		}
		return RESULT_COMPLETE;
	}

	/* listing one specific alias? */
	if (!xinf->rhs)
	{
		if (!alias)
			msg(MSG_WARNING, "[S]no alias named $1", xinf->lhs);
		else
			listalias(xinf->window, alias, False);
		return RESULT_COMPLETE;
	}

	/* safety check */
	if (alias && alias->inuse)
	{
		msg(MSG_ERROR, "[s]can't redefine $1 because it is in use", alias->name);
		return RESULT_ERROR;
	}

	/* verify that the name contains only alphanumeric characters */
	for (i = 0; xinf->lhs[i]; i++)
	{
		if (!isalnum(xinf->lhs[i]))
		{
			msg(MSG_ERROR, "alias names must be alphanumeric");
			return RESULT_ERROR;
		}
	}

	/* create or alter an alias */
	if (alias)
		safefree(alias->command);
	else
	{
		/* find the aliases before & after it, in ASCII order */
		for (lag = NULL, alias = aliases;
		     alias && CHARcmp(xinf->lhs, toCHAR(alias->name)) > 0;
		     lag = alias, alias = alias->next)
		{
		}

		/* allocate the new alias, and insert it into the list */
		newalias = (alias_t *)safekept(1, sizeof *alias);
		newalias->next = alias;
		if (lag)
			lag->next = newalias;
		else
			aliases = newalias;
		newalias->name = safekdup(tochar8(xinf->lhs));
		alias = newalias;
	}
#ifdef DEBUG_ALLOC
	alias->command = CHARkdup(xinf->rhs);
#else
	alias->command = xinf->rhs;
	xinf->rhs = NULL;
#endif
	return RESULT_COMPLETE;
}


/* Add an argument to cmd, by calling buildCHAR() repeatedly. */
static void buildarg(cmd, arg, len, defarg, deflen, quote)
	CHAR	**cmd;	/* the resulting string */
	CHAR	*arg;	/* the arg to add */
	long	len;	/* length of arg */
	CHAR	*defarg;/* default, used if len == 0 */
	long	deflen;	/* length of defarg */
	BOOLEAN	quote;	/* should backslashes be inserted? */
{
	long	i;

	/* if normal arg is empty, then use defarg without quoting */
	if (len == 0)
	{
		arg = defarg;
		len = deflen;
		quote = False;
	}

	/* copy characters, with optional quoting */
	for (i = 0; i < len; i++, arg++)
	{
		if (quote && CHARchr(toCHAR("/\\^$*[."), *arg))
			buildCHAR(cmd, '\\');
		buildCHAR(cmd, *arg);
	}
}


/* Execute an alias */
RESULT	ex_doalias(xinf)
	EXINFO	*xinf;
{
	alias_t	*alias;
	CHAR	*cmd, *str, *defarg;
	CHAR	*args[11];
	long	lens[11];
	long	deflen;
	int	i;
	char	num[24];
	BOOLEAN	inword;
	BOOLEAN	anyargs, anyaddr, anybang;
	BOOLEAN	multiline;
	BOOLEAN	quote;
	RESULT	result = RESULT_ERROR;

	/* Find the alias.  It *will* exist, and use the same name pointer */
	for (alias = aliases; alias->name != xinf->cmdname; alias = alias->next)
	{
	}

	/* parse the args, if any.  args[0] is the whole argument string, and
	 * args[1] through args[9] are the first 9 words from that string.
	 */
	memset(lens, 0, sizeof lens);
	if (xinf->rhs)
	{
		args[0] = xinf->rhs;
		lens[0] = CHARlen(args[0]);
		args[1] = args[0];
		for (i = 1, inword = True, str = args[0]; *str && i < 10; str++)
		{
			if (inword)
			{
				if (isspace(*str))
					inword = False;
				else
					lens[i]++;
			}
			else if (!isspace(*str))
			{
				args[++i] = str; 
				lens[i] = 1;
				inword = True;
			}
		}
	}

	/* Build a copy of the command string, with !0-!9 replaced by args[0]
	 * through args[9].
	 */
	anyargs = anyaddr = anybang = multiline = False;
	for (cmd = NULL, str = alias->command; *str; str++)
	{
		/* if not '!' then it can't be an arg substitution */
		if (*str != '!')
		{
			buildCHAR(&cmd, *str);
			if (*str == '\n' && str[1])
				multiline = True;
			continue;
		}

		/* Allow an optional ':' after the '!'.  Also allow an optional
		 * \ which causes backslashes to be inserted before certain
		 * characters in the expansion.
		 */
		quote = False;
		deflen = 0;
		defarg = args[0]; /* anything but NULL, really */
		str++;
		while (*str == ':' || *str == '\\' || *str == '(')
		{
			if (*str == ':')
				str++;
			else if (*str == '\\')
				str++, quote = True;
			else /* *str == '(' */
			{
				str++;
				defarg = str;
				for (deflen = 0; *str != ')'; deflen++)
				{
					if (!*str || *str == '\n')
					{
						msg(MSG_ERROR, "malformed !() in alias $1", alias->name);
						if (cmd)
							safefree(cmd);
						return RESULT_ERROR;
					}
					str++;
				}
				str++;
			}
		}

		/* which substitution is being requested? */
		switch (*str)
		{
		  case '1':
		  case '2':
		  case '3':
		  case '4':
		  case '5':
		  case '6':
		  case '7':
		  case '8':
		  case '9':
			/* insert an argument */
			i = *str - '0';
			buildarg(&cmd, args[i], lens[i], defarg, deflen, quote);
			anyargs = True;
			break;

		  case '*':
			/* insert the whole argument string */
			buildarg(&cmd, args[0], lens[0], defarg, deflen, quote);
			anyargs = True;
			break;

		  case '^':
			/* insert the first argument string */
			buildarg(&cmd, args[1], lens[1], defarg, deflen, quote);
			anyargs = True;
			break;

		  case '$':
			/* insert the last argument string */
			for (i = 1; i < QTY(lens) - 1 && lens[i + 1] > 0; i++)
			{
			}
			buildarg(&cmd, args[i], lens[i], defarg, deflen, quote);
			anyargs = True;
			break;

		  case '!':
			buildCHAR(&cmd, '!');
			break;

		  case '?':
			if (xinf->bang)
				buildCHAR(&cmd, '!');
			anybang = True;
			break;

		  case '<':
			if (xinf->anyaddr)
			{
				sprintf(num, "%ld", xinf->from);
				buildstr(&cmd, num);
			}
			else
				buildarg(&cmd, NULL, 0, defarg, deflen, quote);
			anyaddr = True;
			break;

		  case '>':
			if (xinf->anyaddr)
			{
				sprintf(num, "%ld", xinf->to);
				buildstr(&cmd, num);
			}
			else
				buildarg(&cmd, NULL, 0, defarg, deflen, quote);
			anyaddr = True;
			break;

		  case '%':
			if (xinf->anyaddr)
			{
				sprintf(num, "%ld,%ld", xinf->from, xinf->to);
				buildstr(&cmd, num);
			}
			else
				buildarg(&cmd, NULL, 0, defarg, deflen, quote);
			anyaddr = True;
			break;

		  default:
			/* no substitution -- use a literal ! character */
			if (str[-1] == ':')
				buildCHAR(&cmd, '!');
			buildCHAR(&cmd, str[-1]);
			buildCHAR(&cmd, *str);
		}
	}

	/* If command contained no !n strings, but the alias was invoked with
	 * arguments, then append the arguments to the last command line.
	 */
	if (!anyargs && !multiline && lens[0] > 0)
	{
		cmd[CHARlen(cmd) - 1] = ' '; /* convert newline to space */
		buildarg(&cmd, args[0], lens[0], args[0], lens[0], False);
		buildCHAR(&cmd, '\n');
		anyargs = True;
	}

	/* Detect usage errors */
	if (xinf->bang && !anybang)
		msg(MSG_ERROR, "[s]the $1 alias doesn't use a ! suffix", alias->name);
	else if (xinf->anyaddr && !anyaddr)
		msg(MSG_ERROR, "[s]the $1 alias doesn't use addresses", alias->name);
	else if (lens[0] > 0 && !anyargs)
		msg(MSG_ERROR, "[s]the $1 alias doesn't use arguments", alias->name);
	else
	{
		/* No errors - Run the command.  Mark it as being "in use"
		 * while it is running, to prevent recursion.
		 */
		alias->inuse = True;
		result = exstring(xinf->window, cmd, alias->name);
		alias->inuse = False;
	}

	/* Free the copy of the command string */
	safefree(cmd);

	return result;
}

#endif /* FEATURE_ALIAS */

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
	{"tool"},
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
			xinf->window->di->newmsg = True;
		}

		/* remember the chosen colors */
		if (colortbl[i].fg)
			safefree(colortbl[i].fg);
		colortbl[i].fg = args ? CHARkdup(args) : NULL;
		if (colortbl[i].bg)
			safefree(colortbl[i].bg);
		colortbl[i].bg = bg ? CHARkdup(bg) : NULL;

		return RESULT_COMPLETE;
	}
	return RESULT_ERROR;
}


# ifdef FEATURE_MKEXRC
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
# endif /* FEATURE_MKEXRC */


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


RESULT ex_message(xinf)
	EXINFO	*xinf;
{
	MSGIMP	imp;
	RESULT	result;

	/* choose an importance level for this message */
	switch (xinf->command)
	{
	  case EX_MESSAGE: imp = MSG_INFO;	result = RESULT_COMPLETE; break;
	  case EX_WARNING: imp = MSG_WARNING;	result = RESULT_COMPLETE; break;
	  case EX_ERROR:   imp = MSG_ERROR;	result = RESULT_ERROR;	  break;
	  default:
#ifndef NDEBUG
		abort();
#endif
		;
	}

	/* do we have a message? */
	if (!xinf->rhs)
	{
		/* no - fake it for :error, else just return */
		if (xinf->command == MSG_ERROR)
			xinf->rhs = CHARdup(toCHAR("error"));
		else
			return result;
	}

	/* don't allow bracket at the beginning -- would look like args */
	if (*xinf->rhs == '[')
		*xinf->rhs = '{';

	/* output the message, or queue it */
	msg(imp, tochar8(xinf->rhs));

	/* return the result code */
	return result;
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
#ifndef DISPLAY_HTML
	msg(MSG_ERROR, "help unavailable; html mode is disabled");
	return RESULT_ERROR;
#else
	CHAR	*topic;	/* topic to search for; a tag name */
	char	*section;/* name of help file containing topic */
	CHAR	*tag;	/* name of tag to search for -- section#topic */
	BUFFER	buf;	/* buffer containing help text */
	MARK	tagdefn;/* result of search; where cursor should move to */
	OPTDESC	*od;	/* description struct of an option */
	int	i;

	/* remove trailing whitespace from args */
	if (xinf->lhs)
		for (topic = &xinf->lhs[CHARlen(xinf->lhs)];
		     topic-- != xinf->lhs && isspace(*topic);
		     )
			*topic = '\0';
	if (xinf->rhs)
		for (topic = &xinf->rhs[CHARlen(xinf->rhs)];
		     topic-- != xinf->rhs && isspace(*topic);
		     )
			*topic = '\0';

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
		if (optgetstr(xinf->rhs, &od))
			topic = toCHAR(od->longname);
		else if (xinf->rhs[0] == 'n' && xinf->rhs[1] == 'o'
					&& (optgetstr(xinf->rhs + 2, &od)))
			topic = toCHAR(od->longname);
		else
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
	else if (optgetstr(xinf->lhs, &od) != NULL)
	{
		/* :help optionname */
		topic = toCHAR(od->longname);
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
		tag = (CHAR *)safealloc((int)(CHARlen(o_filename(buf)) + CHARlen(topic) + 2), sizeof(CHAR));
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
	bufwilldo(tagdefn, False);
	if ((*gui->creategw)(tochar8(o_bufname(markbuffer(tagdefn))), ""))
	{
		return RESULT_COMPLETE;
	}

	/* push the current cursor position and display mode onto tag stack */
	if (o_tagstack &&
		!o_internal(markbuffer(xinf->window->cursor)) &&
		o_filename(markbuffer(xinf->window->cursor)))
	{
		if (xinf->window->tagstack[TAGSTK - 1].prevtag)
			safefree(xinf->window->tagstack[TAGSTK - 1].prevtag);
		if (xinf->window->tagstack[TAGSTK - 1].origin)
			markfree(xinf->window->tagstack[TAGSTK - 1].origin);
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
		/* set "exthenflag" based on result of evaluation */
		exthenflag = calctrue(result);
		return RESULT_COMPLETE;
	}
	else /* command == EX_EVAL */
	{
		/* execute the result as an ex command */
		return exstring(xinf->window, result, NULL);
	}
}

RESULT	ex_then(xinf)
	EXINFO	*xinf;
{
	RESULT	result = RESULT_COMPLETE;
	BOOLEAN	washiding;

	assert(xinf->command == EX_THEN || xinf->command == EX_ELSE
		|| xinf->command == EX_TRY);

	/* If no commands, then do nothing */
	if (!xinf->rhs)
		return result;

	/* For :try, execute the commands unconditionally and then set the
	 * "exthenflag" to indicatge whether the command succeeded.  Otherwise
	 * (for :then and :else) execute the commands if "exthenflag" is set
	 * appropriately
	 */
	if (xinf->command == EX_TRY)
	{
		washiding = msghide(True);
		exthenflag = (BOOLEAN)(exstring(xinf->window, xinf->rhs, NULL) == RESULT_COMPLETE);
		(void)msghide(washiding);
	}
	else if (xinf->command == EX_THEN ? exthenflag : !exthenflag)
	{
		result = exstring(xinf->window, xinf->rhs, NULL);
	}

	return result;
}


RESULT ex_while(xinf)
	EXINFO	*xinf;
{
	/* expression is required */
	if (!xinf->rhs)
	{
		msg(MSG_ERROR, "missing rhs");
		return RESULT_ERROR;
	}

	/* If there was some other, unused test lying around, then free it.
	 *
	 * NOTE: while/do loops can be nested in certain circumstances.  Any
	 * code which pushes an older while test onto the stack also sets
	 * exdotest to NULL so this test won't free those tests.
	 */
	if (exdotest)
		safefree(exdotest);

	/* store the new test */
	exdotest = xinf->rhs;
	xinf->rhs = NULL;
	return RESULT_COMPLETE;
}


RESULT ex_do(xinf)
	EXINFO	*xinf;
{
	CHAR	*value;
	RESULT	result = RESULT_COMPLETE;

	/* if no :while was executed before this, then fail */
	if (!exdotest)
	{
		msg(MSG_ERROR, "missing :while");
		return RESULT_ERROR;
	}

	/* while the expression is true and valid... */
	while ((value = calculate(exdotest, NULL, False)) != NULL
	    && calctrue(value))
	{
		/* Run the command.  If no command, then display result */
		if (xinf->rhs)
			result = exstring(xinf->window, xinf->rhs, NULL);
		else
		{
			drawextext(xinf->window, value, CHARlen(value));
			drawextext(xinf->window, toCHAR("\n"), 1);
		}

		/* is the user getting bored? */
		if (guipoll(False))
			break;
	}

	/* free the exdotest expression */
	safefree(exdotest);
	exdotest = NULL;

	/* if test could not be evaluated, then this command fails */
	if (!value)
		return RESULT_ERROR;
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
	CHAR	*value;
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

		/* evaluate the expression */
		value = calculate(&xinf->rhs[i], NULL, False);
		if (!value)
			/* error message already given */
			return RESULT_ERROR;

		/* store the result */
		if (!optputstr(outbuf, value, xinf->bang))
			/* error message already given */
			return RESULT_ERROR;
	}
	else if (xinf->command == EX_LOCAL)
	{
		if (!xinf->rhs)
		{
			msg(MSG_ERROR, "missing rhs");
			return RESULT_ERROR;
		}
		if (!optset(xinf->bang, xinf->rhs, NULL, 0))
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
#ifdef COPY5
	msg(MSG_INFO, "[s]$1", COPY5);
#endif
#ifdef PORTEDBY
	msg(MSG_INFO, "[s]Ported to (os) by $1", PORTEDBY);
#endif
	return RESULT_COMPLETE;
}


RESULT	ex_qall(xinf)
	EXINFO	*xinf;
{
	WINDOW	win, except;
	WINDOW	orig;
	RESULT	result;
	BOOLEAN	didorig;

	assert(xinf->command == EX_QALL || xinf->command == EX_PRESERVE
		|| xinf->command == EX_ONLY);

	/* If :preserve, then turn off the tempsession flag */
	if (xinf->command == EX_PRESERVE)
	{
		o_tempsession = False;
	}

	/* if :only, then use EX_CLOSE but don't close this window */
	if (xinf->command == EX_ONLY)
	{
		xinf->command = EX_CLOSE;
		except = xinf->window;
	}
	else
	{
		xinf->command = EX_QUIT;
		except = NULL;
	}

	/* run the command on each window, except possibly this one */
	orig = xinf->window;
	for (win = winofbuf(NULL, NULL), result = RESULT_COMPLETE, didorig = False;
	     win;
	     win = winofbuf(win, NULL))
	{
		/* maybe skip the current window */
		if (win == except)
		{
			didorig = True;
			continue;
		}

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

	/* if :q on a modified buffer, and no other window is showing this
	 * buffer, then either (without !) complain or (with !) turn off the
	 * modified flag.
	 */
	if (xinf->command == EX_QUIT
	 && o_modified(buf)
	 && winofbuf(NULL, buf) == xinf->window
	 && winofbuf(xinf->window, buf) == NULL)
	{
		if (xinf->bang)
		{
			o_modified(buf) = False;
		}
		else
		{
			msg(MSG_ERROR, "[S]$1 modified, not saved", o_bufname(buf));
			return RESULT_ERROR;
		}
	}

	/* If this is the last window, then make sure *ALL* user buffers
	 * have been saved.  Exception: If :close! and this session isn't
	 * temporary, then we don't need to check buffers.
	 */
	if ((morebuf != markbuffer(xinf->window->cursor) || morebuf->changes != morechgs || xinf->command == EX_CLOSE)
	 && !(xinf->command == EX_CLOSE && xinf->bang && o_tempsession)
	 && winofbuf(NULL, NULL) == xinf->window && !winofbuf(xinf->window, NULL))
	{
		/* remember some stuff which should prevent us from warning
		 * the user more than once.
		 */
		morebuf = markbuffer(xinf->window->cursor);
		morechgs = morebuf->changes;

		/* check all buffers */
		for (b = elvis_buffers; b; b = buflist(b))
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

	/* If :close, then set the "retain" flag on the window's main buffer. */
	if (xinf->command == EX_CLOSE)
	{
		o_retain(markbuffer(xinf->window->cursor)) = True;
	}

	/* Arrange for the state stack to pop everything.  This will cause
	 * the window to be closed, eventually.
	 */
	for (state = xinf->window->state; state; state = state->pop)
	{
		state->flags |= ELVIS_POP;
	}
	return RESULT_COMPLETE;
}
