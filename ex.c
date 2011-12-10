/* ex.c */

/* Author:
 *	Steve Kirkendall
 *	1500 SW Park #326
 *	Portland OR, 97201
 *	kirkenda@cs.pdx.edu
 */


/* This file contains the code for reading ex commands. */

#include "config.h"
#include "ctype.h"
#include "vi.h"

/* the usual "min" macro.  Note that the minumum argument is evaluated twice. */
#define MIN(a, b)	((a)<=(b) ? (a) : (b))

/* This data type is used to describe the possible argument combinations */
typedef unsigned short ARGT;
#define FROM	1		/* allow a linespec */
#define	TO	2		/* allow a second linespec */
#define BANG	4		/* allow a ! after the command name */
#define EXTRA	8		/* allow extra args after command name */
#define XFILE	16		/* expand wildcards in extra part */
#define NOSPC	32		/* no spaces allowed in the extra part */
#define	DFLALL	64		/* default file range is 1,$ */
#define DFLNONE	128		/* no default file range */
#define NODFL	256		/* do not default to the current file name */
#define EXRCOK	512		/* can be in a .exrc file */
#define NL	1024		/* if mode!=MODE_EX, then write a newline first */
#define PLUS	2048		/* allow a line number, as in ":e +32 foo" */
#define ZERO	4096		/* allow 0 to be given as a line number */
#define NOBAR	8192		/* treat following '|' chars as normal */
#define UNSAFE	16384		/* don't allow in modelines or ./.exrc */
#define APRINT	32768		/* maybe autoprint after these commands */
#define FILES	(XFILE + EXTRA)	/* multiple extra files allowed */
#define WORD1	(EXTRA + NOSPC)	/* one extra word allowed */
#define FILE1	(FILES + NOSPC)	/* 1 file allowed, defaults to current file */
#define NAMEDF	(FILE1 + NODFL)	/* 1 file allowed, defaults to "" */
#define NAMEDFS	(FILES + NODFL)	/* multiple files allowed, default is "" */
#define RANGE	(FROM + TO)	/* range of linespecs allowed */
#define NONE	0		/* no args allowed at all */

/* This array maps ex command names to command codes. The order in which
 * command names are listed below is significant -- ambiguous abbreviations
 * are always resolved to be the first possible match.  (e.g. "r" is taken
 * to mean "read", not "rewind", because "read" comes before "rewind")
 */
static struct
{
	char	*name;	/* name of the command */
	CMD	code;	/* enum code of the command */
	void	(*fn) P_((MARK, MARK, CMD, int, char *));
			/* function which executes the command */
	ARGT	argt;	/* command line arguments permitted/needed/used */
}
	cmdnames[] =
{   /*	cmd name	cmd code	function	arguments */
	{"print",	CMD_PRINT,	cmd_print,	RANGE+NL	},

	{"append",	CMD_APPEND,	cmd_append,	FROM+ZERO+BANG	},
#ifdef DEBUG
	{"bug",		CMD_DEBUG,	cmd_debug,	RANGE+BANG+EXTRA+NL},
#endif
	{"change",	CMD_CHANGE,	cmd_append,	RANGE+BANG	},
	{"delete",	CMD_DELETE,	cmd_delete,	RANGE+WORD1+APRINT},
	{"edit",	CMD_EDIT,	cmd_edit,	BANG+FILE1+PLUS	},
	{"file",	CMD_FILE,	cmd_file,	NAMEDF+UNSAFE	},
	{"global",	CMD_GLOBAL,	cmd_global,	RANGE+BANG+EXTRA+DFLALL+NOBAR},
	{"insert",	CMD_INSERT,	cmd_append,	FROM+BANG	},
	{"join",	CMD_INSERT,	cmd_join,	RANGE+BANG+APRINT},
	{"k",		CMD_MARK,	cmd_mark,	FROM+WORD1	},
	{"list",	CMD_LIST,	cmd_print,	RANGE+NL	},
	{"move",	CMD_MOVE,	cmd_move,	RANGE+EXTRA+APRINT},
	{"next",	CMD_NEXT,	cmd_next,	BANG+NAMEDFS+UNSAFE},
	{"Next",	CMD_PREVIOUS,	cmd_next,	BANG+UNSAFE	},
	{"quit",	CMD_QUIT,	cmd_xit,	BANG		},
	{"read",	CMD_READ,	cmd_read,	FROM+ZERO+NAMEDF},
	{"substitute",	CMD_SUBSTITUTE,	cmd_substitute,	RANGE+EXTRA+NOBAR+APRINT},
	{"to",		CMD_COPY,	cmd_move,	RANGE+EXTRA+APRINT},
	{"undo",	CMD_UNDO,	cmd_undo,	NONE+APRINT	},
	{"vglobal",	CMD_VGLOBAL,	cmd_global,	RANGE+EXTRA+DFLALL+NOBAR},
	{"write",	CMD_WRITE,	cmd_write,	RANGE+BANG+FILE1+DFLALL+UNSAFE},
	{"xit",		CMD_XIT,	cmd_xit,	BANG+NL		},
	{"yank",	CMD_YANK,	cmd_delete,	RANGE+WORD1	},

	{"!",		CMD_BANG,	cmd_shell,	EXRCOK+RANGE+NAMEDFS+DFLNONE+NL+NOBAR+UNSAFE},
	{"\"",		CMD_COMMENT,	cmd_comment,	EXRCOK+BANG+EXTRA+NOBAR},
	{"#",		CMD_NUMBER,	cmd_print,	RANGE+NL	},
	{"<",		CMD_SHIFTL,	cmd_shift,	RANGE+EXTRA+APRINT},
	{">",		CMD_SHIFTR,	cmd_shift,	RANGE+EXTRA+APRINT},
	{"=",		CMD_EQUAL,	cmd_file,	RANGE		},
	{"&",		CMD_SUBAGAIN,	cmd_substitute,	RANGE		},
	{"~",		CMD_SUBAGAIN,	cmd_substitute,	RANGE		},
#ifndef NO_AT
	{"@",		CMD_AT,		cmd_at,		EXTRA		},
#endif

#ifndef NO_ABBR
	{"abbreviate",	CMD_ABBR,	cmd_map,	EXRCOK+BANG+EXTRA+UNSAFE},
#endif
#ifndef NO_IF
	{"and",		CMD_AND,	cmd_if,		EXRCOK+EXTRA	},
#endif
	{"args",	CMD_ARGS,	cmd_args,	EXRCOK+NAMEDFS+UNSAFE},
#ifndef NO_ERRLIST
	{"cc",		CMD_CC,		cmd_make,	BANG+FILES+UNSAFE},
#endif
	{"cd",		CMD_CD,		cmd_cd,		EXRCOK+BANG+NAMEDF+UNSAFE},
	{"copy",	CMD_COPY,	cmd_move,	RANGE+EXTRA+APRINT},
#ifndef NO_DIGRAPH
	{"digraph",	CMD_DIGRAPH,	cmd_digraph,	EXRCOK+BANG+EXTRA},
#endif
#ifndef NO_IF
	{"else",	CMD_ELSE,	cmd_then,	EXRCOK+EXTRA+NOBAR },
#endif
#ifndef NO_ERRLIST
	{"errlist",	CMD_ERRLIST,	cmd_errlist,	BANG+NAMEDF	},
#endif
	{"ex",		CMD_EDIT,	cmd_edit,	BANG+FILE1+UNSAFE},
#ifndef NO_IF
	{"if",		CMD_IF,		cmd_if,		EXRCOK+EXTRA	},
#endif
	{"mark",	CMD_MARK,	cmd_mark,	FROM+WORD1	},
#ifndef NO_MKEXRC
	{"mkexrc",	CMD_MKEXRC,	cmd_mkexrc,	NAMEDF+UNSAFE	},
#endif
	{"number",	CMD_NUMBER,	cmd_print,	RANGE+NL	},
#ifndef NO_IF
	{"or",		CMD_OR,		cmd_if,		EXRCOK+EXTRA	},
#endif
#ifndef NO_TAGSTACK
	{"pop",		CMD_POP,	cmd_pop,	BANG+WORD1+UNSAFE},
#endif
	{"put",		CMD_PUT,	cmd_put,	FROM+ZERO+WORD1	},
	{"set",		CMD_SET,	cmd_set,	EXRCOK+EXTRA	},
	{"shell",	CMD_SHELL,	cmd_shell,	NL+UNSAFE	},
	{"source",	CMD_SOURCE,	cmd_source,	EXRCOK+NAMEDF	},
#ifdef SIGTSTP
	{"stop",	CMD_STOP,	cmd_suspend,	NONE+UNSAFE	},
#endif
	{"tag",		CMD_TAG,	cmd_tag,	BANG+WORD1+UNSAFE},
#ifndef NO_IF
	{"then",	CMD_THEN,	cmd_then,	EXRCOK+EXTRA+NOBAR },
#endif
	{"version",	CMD_VERSION,	cmd_version,	EXRCOK+NONE	},
	{"visual",	CMD_VISUAL,	cmd_edit,	BANG+NAMEDF+UNSAFE},
	{"wq",		CMD_WQUIT,	cmd_xit,	NL+BANG		},

#ifdef DEBUG
	{"debug",	CMD_DEBUG,	cmd_debug,	RANGE+BANG+EXTRA+NL},
	{"validate",	CMD_VALIDATE,	cmd_validate,	BANG+NL		},
#endif
	{"chdir",	CMD_CD,		cmd_cd,		EXRCOK+BANG+NAMEDF+UNSAFE},
#ifndef NO_COLOR
	{"color",	CMD_COLOR,	cmd_color,	EXRCOK+EXTRA},
#endif
#ifndef NO_ERRLIST
	{"make",	CMD_MAKE,	cmd_make,	BANG+NAMEDFS+UNSAFE},
#endif
	{"map",		CMD_MAP,	cmd_map,	EXRCOK+BANG+EXTRA+UNSAFE},
	{"previous",	CMD_PREVIOUS,	cmd_next,	BANG+UNSAFE	},
	{"rewind",	CMD_REWIND,	cmd_next,	BANG+UNSAFE	},
#ifdef SIGTSTP
	{"suspend",	CMD_SUSPEND,	cmd_suspend,	NONE+UNSAFE	},
#endif
	{"unmap",	CMD_UNMAP,	cmd_map,	EXRCOK+BANG+EXTRA+UNSAFE},
#ifndef NO_ABBR
	{"unabbreviate",CMD_UNABBR,	cmd_map,	EXRCOK+EXTRA+UNSAFE},
#endif

	{(char *)0}
};


/* This function parses a search pattern - given a pointer to a / or ?,
 * it replaces the ending / or ? with a \0, and returns a pointer to the
 * stuff that came after the pattern.
 */
char	*parseptrn(ptrn)
	REG char	*ptrn;
{
	REG char 	*scan;

	for (scan = ptrn + 1;
	     *scan && *scan != *ptrn;
	     scan++)
	{
		/* allow backslashed versions of / and ? in the pattern */
		if (*scan == '\\' && scan[1] != '\0')
		{
			scan++;
			if (*scan == '[' && scan[1] != '\0')	/*-g.t.*/
			{					/*-g.t.*/
				scan++;				/*-g.t.*/
			}					/*-g.t.*/
		}
		/* allow / and ? between [ and ] */		/*-g.t.*/
		if (*scan == '[' && scan[1] != '\0')		/*-g.t.*/
		{						/*-g.t.*/
			scan++;					/*-g.t.*/
			while (*scan != ']' && scan[1] != '\0')	/*-g.t.*/
			{					/*-g.t.*/
				scan++;				/*-g.t.*/
			}					/*-g.t.*/
		}
	}
	if (*scan)
	{
		*scan++ = '\0';
	}

	return scan;
}


/* This function parses a line specifier for ex commands */
char *linespec(s, markptr)
	REG char	*s;		/* start of the line specifier */
	MARK		*markptr;	/* where to store the mark's value */
{
	long		num;
	REG char	*t;

	/* parse each ;-delimited clause of this linespec */
	do
	{
		/* skip an initial ';', if any */
		if (*s == ';')
		{
			s++;
		}

		/* skip leading spaces */
		while (isspace(*s))
		{
			s++;
		}

		/* dot means current position */
		if (*s == '.')
		{
			s++;
			*markptr = cursor;
		}
		/* '$' means the last line */
		else if (*s == '$')
		{
			s++;
			*markptr = MARK_LAST;
		}
		/* digit means an absolute line number */
		else if (isdigit(*s))
		{
			for (num = 0; isdigit(*s); s++)
			{
				num = num * 10 + *s - '0';
			}
			*markptr = MARK_AT_LINE(num);
		}
		/* appostrophe means go to a set mark */
		else if (*s == '\'')
		{
			s++;
			*markptr = m_tomark(cursor, 1L, (int)*s);
			s++;
		}
		/* slash means do a search */
		else if (*s == '/' || *s == '?')
		{
			/* put a '\0' at the end of the search pattern */
			t = parseptrn(s);

			/* search for the pattern */
			*markptr &= ~(BLKSIZE - 1);
			if (*s == '/')
			{
				pfetch(markline(*markptr));
				if (plen > 0)
					*markptr += plen - 1;
				*markptr = m_fsrch(*markptr, s);
			}
			else
			{
				*markptr = m_bsrch(*markptr, s);
			}

			/* adjust command string pointer */
			s = t;
		}

		/* if linespec was faulty, quit now */
		if (!*markptr)
		{
			beep();
			return s;
		}

		/* maybe add an offset */
		t = s;
		if (*t == '-' || *t == '+')
		{
			s++;
			for (num = 0; isdigit(*s); s++)
			{
				num = num * 10 + *s - '0';
			}
			if (num == 0)
			{
				num = 1;
			}
			*markptr = m_updnto(*markptr, num, *t);
		}
	} while (*s == ';' || *s == '+' || *s == '-');

	/* protect against invalid line numbers */
	num = markline(*markptr);
	if (num < 1L || num > nlines)
	{
		msg("Invalid line number -- must be from 1 to %ld", nlines);
		*markptr = MARK_UNSET;
	}

	return s;
}



/* This function reads an ex command and executes it. */
void ex()
{
	char		cmdbuf[150];
	REG int		cmdlen;
	static long	oldline;
	int		autoprint;

	significant = FALSE;
	oldline = markline(cursor);

	while (mode == MODE_EX)
	{
		/* read a line */
#ifdef CRUNCH
		cmdlen = vgets(':', cmdbuf, sizeof(cmdbuf));
#else
		cmdlen = vgets(*o_prompt ? ':' : '\0', cmdbuf, sizeof(cmdbuf));
#endif
		if (cmdlen < 0)
		{
			return;
		}

		/* if empty line, assume ".+1" */
		if (cmdlen == 0)
		{
			strcpy(cmdbuf, ".+1");
			qaddch('\r');
			clrtoeol();
		}
		else
		{
			addch('\n');
		}
		refresh();

		/* parse & execute the command */
		autoprint = doexcmd(cmdbuf, '\\');

		/* handle autoprint */
		if (significant || markline(cursor) != oldline)
		{
			significant = FALSE;
			oldline = markline(cursor);
			if (*o_autoprint && mode == MODE_EX && autoprint)
			{
				cmd_print(cursor, cursor, CMD_PRINT, FALSE, "");
			}
		}
	}
}



/* This function executes a single command line.  The '\n' at the end of the
 * line should be replaced by '\0'.  The line may contain multiple commands,
 * separated by '|' characters.  To pass a '|' character as part of an
 * argument to a command, precede the '|' with a quote character.
 *
 * Returns TRUE if the command allows autoprint, FALSE if it doesn't.
 *
 * NOTE: The cmdbuf string may be altered.
 */
int doexcmd(cmdbuf, qchar)
	char		*cmdbuf;	/* string containing an ex command */
	int		qchar;		/* quote character for '|' */
{
	REG char	*scan;		/* used to scan thru cmdbuf */
	MARK		frommark;	/* first linespec */
	MARK		tomark;		/* second linespec */
	REG int		cmdlen;		/* length of the command name given */
	CMD		cmd;		/* what command is this? */
	ARGT		argt;		/* argument types for this command */
	short		forceit;	/* bang version of a command? */
	REG int		cmdidx;		/* index of command */
	REG char	*build;		/* used while copying filenames */
	char		*tmp;
	int		iswild;		/* boolean: filenames use wildcards? */
	int		isdfl;		/* using default line ranges? */
	int		didsub;		/* did we substitute file names for % or # */
	char		*nextcmd;	/* next command in this same string */
#ifndef NO_VISIBLE
	long		chgd;		/* used to detect change to value of "changes" */
#endif
	int		autoprint = FALSE;/* Boolean: allow autoprint? */

	/* ex commands can't be undone via the shift-U command */
	U_line = 0L;

	/* permit extra colons at the start of the line */
	for (; *cmdbuf == ':'; cmdbuf++)
	{
	}

	/* execute all '|'-delimited commands in cmdbuf, one at a time */
	for (nextcmd = (char *)0; cmdbuf; cmdbuf = nextcmd, nextcmd = (char *)0)
	{
		/* parse the line specifier */
		scan = cmdbuf;
		if (nlines < 1)
		{
			/* no file, so don't allow addresses */
		}
		else if (*scan == '%')
		{
			/* '%' means all lines */
			frommark = MARK_FIRST;
			tomark = MARK_LAST;
			scan++;
		}
		else if (*scan == '0')
		{
			scan++;
			frommark = tomark = (*scan ? MARK_UNSET : MARK_FIRST);
		}
		else
		{
			frommark = cursor;
			scan = linespec(scan, &frommark);
			tomark = frommark;
			if (frommark && *scan == ',')
			{
				scan++;
				tomark = cursor;
				scan = linespec(scan, &tomark);
			}
			if (!tomark)
			{
				/* faulty line spec -- fault already described */
				return;
			}
			if (frommark > tomark)
			{
				msg("first address exceeds the second");
				return;
			}
		}
		isdfl = (scan == cmdbuf);

		/* skip whitespace */
		while (isspace(*scan))
		{
			scan++;
		}

		/* Figure out how long the command name is.  If no command, then the
		 * length is 0, which will match the "print" command.
		 */ 
		if (!*scan)
		{
			/* if both endpoints are at the same line, then just
			 * move to the start of that line without printing.
			 */
			if (frommark == tomark)
			{
				if (tomark != MARK_UNSET)
					cursor = tomark;
				return;
			}
			cmdlen = 0;
		}
		else if (!isalpha(*scan))
		{
			cmdlen = 1;
		}
		else
		{
			for (cmdlen = 1;
			     isalpha(scan[cmdlen]);
			     cmdlen++)
			{
			}
		}

		/* Lookup the command code.
		 *
		 * If the given command name is shorter than the cannonical
		 * form of the name, then only compare the given portion in
		 * order to allow abbreviations to be recognized.
		 *
		 * However, if the given command is longer than the
		 * cannonical form, then only compare the canonical
		 * portion so that arguments can be appended to complete
		 * command names without requiring intervening whitespace;
		 * this is mostly so that users can type ":ka" when they
		 * mean ":k a".
		 */
		for (cmdidx = 0;
		     cmdnames[cmdidx].name && strncmp(scan, cmdnames[cmdidx].name, MIN(cmdlen, strlen(cmdnames[cmdidx].name)));
		     cmdidx++)
		{
		}
		argt = cmdnames[cmdidx].argt;
		cmd = cmdnames[cmdidx].code;
		if (cmd == CMD_NULL)
		{
			msg("Unknown command \"%.*s\"", cmdlen, scan);
			return;
		}

#ifndef NO_SAFER
		if ((argt & UNSAFE) && *o_safer)
		{
			msg("command \"%.*s\" is unsafe", cmdlen, scan);
			return;
		}
#endif

		/* if the command doesn't have NOBAR set, then replace | with \0 */
		if (!(argt & NOBAR))
		{
			/* find the next unquoted '|'.  For any quoted '|',
			 * delete the quote character but leave the '|'.
			 */
			for (build = nextcmd = scan; *nextcmd && *nextcmd != '|'; nextcmd++)
			{
				if (nextcmd[0] == qchar && nextcmd[1] == '|')
				{
					nextcmd++;
				}
				*build++ = *nextcmd;
			}

			/* was a '|' found? */
			if (*nextcmd)
			{
				/* Yes!  Leave nextcmd pointing to char after '|' */
				nextcmd++;
			}
			else
			{
				/* No!  Set nextcmd to NULL so we stop after this */
				nextcmd = (char *)0;
			}

			/* mark the end of this particular command */
			*build = '\0';
		}

		/* if the command name ended with a bang, set the forceit flag */
		scan += MIN(cmdlen, strlen(cmdnames[cmdidx].name));
		if ((argt & BANG) && *scan == '!')
		{
			scan++;
			forceit = 1;
		}
		else
		{
			forceit = 0;
		}

		/* skip any more whitespace, to leave scan pointing to arguments */
		while (isspace(*scan))
		{
			scan++;
		}

		/* For "read" and "write" commands, if a !program is given
		 * instead of a filename, then don't complain about whitespace
		 * in the command, and don't treat '|' as command separator.
		 */
		if ((cmd == CMD_READ || cmd == CMD_WRITE) && *scan == '!')
		{
			argt &= ~NOSPC;
			argt |= NOBAR;
		}

		/* a couple of special cases for filenames */
		if (argt & XFILE)
		{
			/* if names were given, process them */
			if (*scan)
			{
				for (build = tmpblk.c, iswild = didsub = FALSE; *scan; scan++)
				{
					switch (*scan)
					{
					  case '\\':
						if (scan[1] == '\\' || scan[1] == '%' || scan[1] == '#')
						{
							*build++ = *++scan;
						}
						else if (scan[1] == '@')
						{
							tmp = get_cursor_word(cursor);
							if (!tmp)
							{
								tmp = "@";
							}
							strcpy(build, tmp);
							build += strlen(build);
							scan++;
						}
						else
						{
							*build++ = '\\';
						}
						break;

					  case '%':
						if (!*origname)
						{
							msg("No filename to substitute for %%");
							return;
						}
						strcpy(build, origname);
						while (*build)
						{
							build++;
						}
						didsub = TRUE;
						break;

					  case '#':
						if (!*prevorig)
						{
							msg("No filename to substitute for #");
							return;
						}
						strcpy(build, prevorig);
						while (*build)
						{
							build++;
						}
						didsub = TRUE;
						break;

					  case '*':
					  case '?':
#if !(MSDOS || TOS)
					  case '[':
					  case '`':
					  case '{': /* } */
					  case '$':
					  case '~':
#endif
						*build++ = *scan;
						iswild = TRUE;
						break;

					  default:
						*build++ = *scan;
					}
				}
				*build = '\0';

				if (cmd == CMD_BANG
				 || cmd == CMD_READ && tmpblk.c[0] == '!'
				 || cmd == CMD_WRITE && tmpblk.c[0] == '!')
				{
#ifndef NO_SAFER
					if (*o_safer)
					{
						msg("unsafe to run external program");
						return;
					}
#endif
					if (didsub)
					{
						if (mode != MODE_EX)
						{
							addch('\n');
						}
						addstr(tmpblk.c);
						addch('\n');
						exrefresh();
					}
				}
#ifndef NO_SAFER
				else if (iswild && *o_safer)
				{
					msg("wildcard expansion is unsafe");
					return;
				}
#endif
				else
				{
					if (iswild && tmpblk.c[0] != '>')
					{
						scan = wildcard(tmpblk.c);
					}
				}
			}
			else /* no names given, maybe assume origname */
			{
				if (!(argt & NODFL))
				{
					strcpy(tmpblk.c, origname);
				}
				else
				{
					*tmpblk.c = '\0';
				}
			}

			scan = tmpblk.c;
		}

		/* bad arguments? */
		if (!(argt & EXRCOK) && nlines < 1L)
		{
			msg("Can't use the \"%s\" command in a %s file", cmdnames[cmdidx].name, EXRC);
			return;
		}
		if (!(argt & (ZERO | EXRCOK)) && frommark == MARK_UNSET)
		{
			msg("Can't use address 0 with \"%s\" command.", cmdnames[cmdidx].name);
			return;
		}
		if (!(argt & FROM) && frommark != cursor && nlines >= 1L)
		{
			msg("Can't use address with \"%s\" command.", cmdnames[cmdidx].name);
			return;
		}
		if (!(argt & TO) && tomark != frommark && nlines >= 1L)
		{
			msg("Can't use a range with \"%s\" command.", cmdnames[cmdidx].name);
			return;
		}
		if (!(argt & EXTRA) && *scan)
		{
			msg("Extra characters after \"%s\" command.", cmdnames[cmdidx].name);
			return;
		}
		if ((argt & NOSPC) && !(cmd == CMD_READ && (forceit || *scan == '!')))
		{
			build = scan;
#ifndef CRUNCH
			if ((argt & PLUS) && *build == '+')
			{
				while (*build && !isspace(*build))
				{
					build++;
				}
				while (*build && isspace(*build))
				{
					build++;
				}
			}
#endif /* not CRUNCH */
			for (; *build; build++)
			{
				if (isspace(*build))
				{
					msg("Too many %s to \"%s\" command.",
						(argt & XFILE) ? "filenames" : "arguments",
						cmdnames[cmdidx].name);
					return;
				}
			}
		}

		/* some commands have special default ranges */
		if (isdfl)
		{
#ifndef NO_VISIBLE
			if (V_from && (argt & RANGE))
			{
				if (cursor < V_from)
				{
					frommark = cursor;
					tomark = V_from;
				}
				else
				{
					frommark = V_from;
					tomark = cursor;
				}
			}
			else
#endif
			if (argt & DFLALL)
			{
				frommark = MARK_FIRST;
				tomark = MARK_LAST;
			}
			else if (argt & DFLNONE)
			{
				frommark = tomark = 0L;
			}
		}

		/* write a newline if called from visual mode */
		if ((argt & NL) && mode != MODE_EX && !exwrote)
		{
			addch('\n');
			exrefresh();
		}

		/* act on the command */
#ifndef NO_VISIBLE
		chgd = changes;
#endif
		(*cmdnames[cmdidx].fn)(frommark, tomark, cmd, forceit, scan);
#ifndef NO_VISIBLE
		/* Commands that change the file could interact with visible
		 * line marking in funny ways.  If lines are visibly marked, 
		 * and the file has changed, then we'd better not execute any
		 * remaining commands, for safety's sake.
		 */
		if (V_from && chgd != changes && nextcmd)
		{
			msg("Skipping \"%s\" after visible mark lost", nextcmd);
			break;
		}
#endif
		/* if the command allows autoprint, remember that! */
		if (argt & APRINT)
		{
			autoprint = TRUE;
		}
	}
#ifndef NO_VISIBLE
	V_from = MARK_UNSET;
#endif
	return autoprint;
}


/* This function executes EX commands from a file.  It returns 1 normally, or
 * 0 if the file could not be opened for reading.
 */
int doexrc(filename)
	char	*filename;	/* name of a ".exrc" file */
{
	int	fd;		/* file descriptor */
	int	len;		/* bytes in the buffer */
	char	*scan;		/* current position in the buffer */
	char	*build;		/* used during buffer shifting */

	/* !!! kludge: we use U_text as the buffer.  This has the side-effect
	 * of interfering with the shift-U visual command.  Disable shift-U.
	 */
	U_line = 0L;

	/* open the file */
	fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
		return 0;
	}

	/* read the first block from the file */
	len = tread(fd, U_text, sizeof U_text);

	/* for each command line in the buffer... */
	while (len > 0)
	{
		/* locate the end of the command. */
		for (scan = U_text; scan < &U_text[len] && *scan != '\n'; scan++)
		{
		}
		if (scan >= &U_text[len])
		{
			/* No newline found!  Stop reading the .exrc */
			break;
		}

		/* convert the '\n' to '\0' at the end */
		*scan++ = '\0';

		/* Skip leading whitespace.  This is done mostly so that lines
		 * which contain ONLY whitespace can be recognized as blank.
		 */
		for (build = U_text; *build && isspace(*build); build++)
		{
		}

		/* Execute the command line, unless it is blank.  Blank lines
		 * in a .exrc file are ignored.
		 */
		if (*build)
		{
			doexcmd(build, ctrl('V'));
		}

		/* try to shift more of the file into the buffer */
		for (build = U_text; scan < &U_text[len]; )
		{
			*build++ = *scan++;
		}
		len = (int)(build - U_text);
		len += tread(fd, build, sizeof U_text - len);
	}

	/* close the file */
	close(fd);

	return 1;
}
