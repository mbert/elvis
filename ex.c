/* ex.c */
/* Copyright 1995 by Steve Kirkendall */

char id_ex[] = "$Id: ex.c,v 2.147 1998/11/21 01:36:11 steve Exp $";

#include "elvis.h"

#if USE_PROTOTYPES
static void skipwhitespace(CHAR **refp);
static BOOLEAN parsewindowid(CHAR **refp, EXINFO *xinf);
static BOOLEAN parsebuffername(CHAR **refp, EXINFO *xinf);
static BOOLEAN parseregexp(CHAR **refp, EXINFO *xinf, long flags);
static BOOLEAN parsecommandname(CHAR **refp, EXINFO *xinf);
static void parsemulti(CHAR **refp, EXINFO *xinf);
static void parsebang(CHAR **refp, EXINFO *xinf, long flags);
static void parseplus(CHAR **refp, EXINFO *xinf, long flags);
static void parseprintflag(CHAR **refp, EXINFO *xinf, long flags);
static void parselhs(CHAR **refp, EXINFO *xinf, long flags);
static void parserhs(CHAR **refp, EXINFO *xinf, long flags);
static void parsecutbuffer(CHAR **refp, EXINFO *xinf, long flags);
static void parsecount(CHAR **refp, EXINFO *xinf, long flags);
static RESULT parsecmds(CHAR **refp, EXINFO *xinf, long flags);
static BOOLEAN parsefileargs(CHAR **refp, EXINFO *xinf, long flags);
static RESULT parse(WINDOW win, CHAR **refp, EXINFO *xinf);
static RESULT execute(EXINFO *xinf);
#endif

/* Minimum number of filename pointers to allocate at a time. */
#define FILEGRAN	8



/* These are the possible arguments for a command.  These may be combined
 * via the bitwise OR operator to describe arbitrary combinations of these.
 */
#define a_Line	  0x00000001L	/* single line specifier */
#define a_Range	  0x00000002L	/* range of lines */
#define a_Multi	  0x00000004L	/* command character may be stuttered */
#define a_Bang	  0x00000008L	/* '!' after the command name */
#define a_Target  0x00000010L	/* extra line specifier after the command name */
#define a_Lhs	  0x00000020L	/* single word after the command */
#define a_Rhs	  0x00000040L	/* extra words (after Lhs, if Lhs allowed) */
#define a_Buffer  0x00000080L	/* cut buffer name */
#define a_Count	  0x00000100L	/* number of lines affected */
#define a_Pflag	  0x00000200L	/* print flag (p, l, or #) */
#define a_Plus	  0x00000400L	/* line offset (as for ":e +20 foo") */
#define a_Append  0x00000800L	/* ">>" indicating append mode */
#define a_Filter  0x00001000L	/* "!cmd", where cmd may include '|' characters */
#define a_File	  0x00002000L	/* a single file name */
#define a_Files	  0x00004000L	/* Zero or more filenames */
#define a_Cmds	  0x00008000L	/* text which may contain '|' characters */
#define a_RegExp  0x00010000L	/* regular expression */
#define a_RegSub  0x00020000L	/* substitution text (requires RegExp) */
#define a_Text	  0x00040000L	/* If no Rhs, following lines are part of cmd */

/* The following values indicate the default values of arguments */
#define d_All	  0x00080000L	/* default range is all lines */
#define d_2Lines  0x00100000L	/* default range is two lines (for :join) */
#define d_None	  0x00180000L	/* default range is no lines (else current line) */
#define d_LnMask  0x00180000L	/* mask value for extracting line defaults */
#define d_File	  0x00200000L	/* default file is current file (else no default) */

/* The following indicate other quirks of commands */
#define q_Unsafe  0x00400000L	/* command can't be executed in .exrc script */
#define q_Autop	  0x00800000L	/* autoprint current line */
#define q_Zero	  0x01000000L	/* allow Target or Line to be 0 */
#define q_Exrc	  0x02000000L	/* command may be run in a .exrc file */
#define q_Undo	  0x04000000L	/* save an "undo" version before this command */
#define q_Custom  0x08000000L	/* save options/maps after command */
#define q_Ex	  0x10000000L	/* only allowed in ex mode, not vi's <:> cmd */
#define q_CtrlV	  0x20000000L	/* use ^V for quote char, instead of \ */
#define q_MayQuit 0x40000000L	/* may cause window to disappear */
#define q_SwitchB 0x80000000L	/* may move cursor to a different buffer */


/* This array maps ex command names to command codes. The order in which
 * command names are listed below is significant --  ambiguous abbreviations
 * are always resolved to be the first possible match.  (e.g. "r" is taken
 * to mean "read", not "rewind", because "read" comes before "rewind")
 *
 * Also, commands which share the same first letter are grouped together.
 * Similarly, within each one-letter group, the commands are ordered so that
 * the commands with the same two letters are grouped together, and those
 * groups are then divided into 3-letter groups, and so on.  This allows
 * the command list to be searched faster.
 * 
 * The comment at the start of each line below gives the shortest abbreviation.
 * HOWEVER, YOU CAN'T SIMPLY SORT THOSE ABBREVIATIONS to produce the correct
 * order for the commands.  Consider the change/chdir/calculate commands, for
 * an example of why that wouldn't work.
 */
static struct
{
	char	*name;	/* name of the command */
	EXCMD	code;	/* enum code of the command */
	RESULT	(*fn) P_((EXINFO *));
			/* function which executes the command */
	unsigned long	flags;	/* command line arguments, defaults, quirks */
}
	cmdnames[] =
{   /*	 cmd name	cmd code	function	arguments */
/*!   */{"!",		EX_BANG,	ex_bang,	a_Range | a_Cmds | d_None | q_Exrc | q_Unsafe | q_Undo			},
/*#   */{"#",		EX_NUMBER,	ex_print,	a_Range | a_Count | a_Pflag						},
/*&   */{"&",		EX_SUBAGAIN,	ex_substitute,	a_Range | a_Rhs | q_Undo						},
/*<   */{"<",		EX_SHIFTL,	ex_shift,	a_Range | a_Multi | a_Bang | a_Count | q_Autop | q_Undo			},
/*=   */{"=",		EX_EQUAL,	ex_file,	a_Range									},
/*>   */{">",		EX_SHIFTR,	ex_shift,	a_Range | a_Multi | a_Bang | a_Count | q_Autop | q_Undo			},
/*@   */{"@",		EX_AT,		ex_at,		a_Buffer								},
/*N   */{"Next",	EX_PREVIOUS,	ex_next,	a_Bang | q_Unsafe | q_SwitchB						},
/*"   */{"\"",		EX_COMMENT,	ex_comment,	a_Cmds | q_Exrc								},
/*a   */{"append",	EX_APPEND,	ex_append,	a_Line | a_Bang | a_Cmds | a_Text | q_Zero | q_Undo | q_Ex		},
/*ab  */{"abbreviate",	EX_ABBR,	ex_map,		a_Bang | a_Lhs | a_Rhs | q_Exrc | q_Custom | q_Unsafe | q_CtrlV		},
/*al  */{"all",		EX_ALL,		ex_all,		a_Bang | a_Cmds								},
#ifdef FEATURE_ALIAS
/*ali */{"alias",	EX_ALIAS,	ex_alias,	a_Lhs | a_Cmds | q_Exrc | q_Unsafe | q_Custom				},
#endif
/*ar  */{"args",	EX_ARGS,	ex_args,	a_Files | q_Exrc | q_Unsafe						},
/*b   */{"buffer",	EX_BUFFER,	ex_buffer,	a_Bang | a_Rhs | q_SwitchB						},
/*bb  */{"bbrowse",	EX_BBROWSE,	ex_buffer,	a_Bang | q_SwitchB							},
/*br  */{"browse",	EX_BROWSE,	ex_browse,	a_Bang|a_Rhs | q_SwitchB						},
/*bro */{"break",	EX_BREAK,	ex_map,		a_Bang | a_Lhs | q_CtrlV						},
/*c   */{"change",	EX_CHANGE,	ex_append,	a_Range | a_Bang | a_Count | a_Text | q_Undo | q_Ex			},
/*cha */{"chdir",	EX_CD,		ex_cd,		a_Bang | a_File | q_Exrc | q_Unsafe					},
/*ca  */{"calculate",	EX_CALC,	ex_comment,	a_Cmds | q_Exrc								},
/*cc  */{"cc",		EX_CC,		ex_make,	a_Bang | a_Rhs | q_Unsafe | q_SwitchB					},
/*cd  */{"cd",		EX_CD,		ex_cd,		a_Bang | a_File | q_Exrc | q_Unsafe					},
/*cl  */{"close",	EX_CLOSE,	ex_xit,		a_Bang | q_MayQuit							},
/*co  */{"copy",	EX_COPY,	ex_move,	a_Range | a_Target | a_Pflag | q_Autop | q_Undo				},
/*col */{"color",	EX_COLOR,	ex_color,	a_Rhs | q_Exrc | q_Custom						},
/*d   */{"delete",	EX_DELETE,	ex_delete,	a_Range | a_Buffer | a_Count | a_Pflag | q_Undo | q_Autop		},
/*di  */{"display",	EX_DISPLAY,	ex_display,	a_Rhs									},
/*dig */{"digraph",	EX_DIGRAPH,	ex_digraph,	a_Bang | a_Rhs | q_Exrc | q_Custom					},
/*do  */{"do",		EX_DO,		ex_do,		a_Cmds | q_Exrc			 					},
/*e   */{"edit",	EX_EDIT,	ex_edit,	a_Bang | a_Plus | a_File | d_File | q_SwitchB				},
/*ec  */{"echo",	EX_ECHO,	ex_comment,	a_Rhs | q_Exrc								},
/*el  */{"else",	EX_ELSE,	ex_then,	a_Cmds | q_Exrc								},
/*er  */{"errlist",	EX_ERRLIST,	ex_errlist,	a_Bang | a_File | a_Filter | q_SwitchB					},
/*erro*/{"error",	EX_ERROR,	ex_message,	a_Rhs | q_Exrc								},
/*ev  */{"eval",	EX_EVAL,	ex_if,		a_Cmds | q_Exrc								},
/*ex  */{"ex",		EX_EDIT,	ex_edit,	a_Bang | a_Plus | a_File | d_File | q_Unsafe | q_SwitchB		},
/*f   */{"file",	EX_FILE,	ex_file,	a_File | q_Unsafe							},
/*g   */{"global",	EX_GLOBAL,	ex_global,	a_Range | a_Bang | a_RegExp | a_Cmds | d_All | q_Undo			},
/*go  */{"goto",	EX_GOTO,	ex_comment,	a_Line | q_Zero | q_Autop | q_SwitchB					},
/*gu  */{"gui",		EX_GUI,		ex_gui,		a_Cmds | q_Exrc								},
/*h   */{"help",	EX_HELP,	ex_help,	a_Lhs | a_Rhs | q_SwitchB						},
/*i   */{"insert",	EX_INSERT,	ex_append,	a_Line | a_Bang | a_Cmds | a_Text | q_Undo | q_Ex			},
/*if  */{"if",		EX_IF,		ex_if,		a_Cmds | q_Exrc								},
/*j   */{"join",	EX_INSERT,	ex_join,	a_Range | a_Bang | a_Count | d_2Lines | q_Autop | q_Undo		},
/*k   */{"k",		EX_MARK,	ex_mark,	a_Line | a_Lhs								},
/*l   */{"list",	EX_LIST,	ex_print,	a_Range | a_Count | a_Pflag						},
/*la  */{"last",	EX_LAST,	ex_next,	q_Unsafe | q_SwitchB							},
/*le  */{"let",		EX_LET,		ex_set,		a_Bang | a_Cmds | q_Exrc | q_Custom					},
/*se  */{"local",	EX_LOCAL,	ex_set,		a_Rhs | q_Exrc				 				},
/*lp  */{"lpr",		EX_LPR,		ex_lpr,		a_Range | a_Bang | a_Append | a_File | a_Filter | d_All | q_Unsafe			},
/*m   */{"move",	EX_MOVE,	ex_move,	a_Range | a_Target | q_Autop | q_Undo					},
/*ma  */{"mark",	EX_MARK,	ex_mark,	a_Line | a_Lhs								},
/*mak */{"make",	EX_MAKE,	ex_make,	a_Bang | a_Rhs | q_Unsafe | q_SwitchB					},
/*map */{"map",		EX_MAP,		ex_map,		a_Bang | a_Lhs | a_Rhs | q_Exrc | q_Custom | q_Unsafe | q_CtrlV		},
/*me  */{"message",	EX_MESSAGE,	ex_message,	a_Rhs | q_Exrc								},
/*mk  */{"mkexrc",	EX_MKEXRC,	ex_mkexrc,	a_Bang | a_File | q_Unsafe						},
/*n   */{"next",	EX_NEXT,	ex_next,	a_Bang | a_Files | q_Unsafe | q_SwitchB					},
/*ne  */{"new",		EX_SNEW,	ex_split,	q_Unsafe								},
/*no  */{"normal",	EX_NORMAL,	ex_display										},
/*nu  */{"number",	EX_NUMBER,	ex_print,	a_Range | a_Count | a_Pflag						},
/*o   */{"open",	EX_OPEN,	ex_edit,	a_Bang | a_Plus | a_File						},
/*on  */{"only",	EX_ONLY,	ex_qall,	a_Bang | d_None | q_Unsafe						},
/*p   */{"print",	EX_PRINT,	ex_print,	a_Range | a_Count | a_Pflag						},
/*pre */{"previous",	EX_PREVIOUS,	ex_next,	a_Bang | q_Unsafe | q_SwitchB						},
/*pres*/{"preserve",	EX_PRESERVE,	ex_qall,	d_None | q_Unsafe | q_MayQuit						},
/*po  */{"pop",		EX_POP,		ex_pop,		a_Bang | q_Unsafe | q_SwitchB						},
/*pu  */{"put",		EX_PUT,		ex_put,		a_Line | q_Zero | a_Buffer | q_Undo					},
/*q   */{"quit",	EX_QUIT,	ex_xit,		a_Bang | q_MayQuit							},
/*qa  */{"qall",	EX_QALL,	ex_qall,	a_Bang | q_MayQuit							},
/*r   */{"read",	EX_READ,	ex_read,	a_Line | a_File | a_Filter | q_Zero | q_Undo				},
/*red */{"redo",	EX_REDO,	ex_undo,	a_Lhs | q_Autop								},
/*rew */{"rewind",	EX_REWIND,	ex_next,	a_Bang | q_Unsafe | q_SwitchB						},
/*s   */{"substitute",	EX_SUBSTITUTE,	ex_substitute,	a_Range|a_RegExp|a_RegSub|a_Rhs|a_Count|a_Pflag|q_Autop|q_Undo|q_Exrc	},
/*sN  */{"sNext",	EX_SPREVIOUS,	ex_next,	q_Unsafe								},
/*sa  */{"sall",	EX_SALL,	ex_sall,	q_Unsafe								},
/*saf */{"safer",	EX_SAFER,	ex_source,	a_Bang | a_File | q_Exrc						},
/*sbb */{"sbbrowse",	EX_SBBROWSE,	ex_buffer,	a_Bang									},
/*sbr */{"sbrowse",	EX_SBROWSE,	ex_browse,	a_Bang|a_Rhs								},
/*se  */{"set",		EX_SET,		ex_set,		a_Bang | a_Rhs | q_Exrc | q_Custom					},
/*sh  */{"shell",	EX_SHELL,	ex_suspend,	q_Unsafe								},
/*sl  */{"slast",	EX_SLAST,	ex_next,	q_Unsafe								},
/*sn  */{"snext",	EX_SNEXT,	ex_next,	a_Files									},
/*snew*/{"snew",	EX_SNEW,	ex_split,	q_Unsafe								},
/*so  */{"source",	EX_SOURCE,	ex_source,	a_Bang | a_File | q_Exrc						},
/*sp  */{"split",	EX_SPLIT,	ex_split,	a_Line | a_Plus | a_File | a_Filter					},
/*sr  */{"srewind",	EX_SREWIND,	ex_next,	a_Bang | q_Unsafe							},
/*st  */{"stop",	EX_STOP,	ex_suspend,	a_Bang | q_Unsafe							},
/*sta */{"stag",	EX_STAG,	ex_tag,		a_Rhs									},
/*stac*/{"stack",	EX_STACK,	ex_stack,	d_None									},
/*su  */{"suspend",	EX_SUSPEND,	ex_suspend,	a_Bang | q_Unsafe							},
/*t   */{"to",		EX_COPY,	ex_move,	a_Range | a_Target | a_Pflag | q_Autop | q_Undo				},
/*ta  */{"tag",		EX_TAG,		ex_tag,		a_Bang | a_Rhs | q_Unsafe | q_SwitchB					},
/*th  */{"then",	EX_THEN,	ex_then,	a_Cmds | q_Exrc			 					},
/*try */{"try",		EX_TRY,		ex_then,	a_Cmds | q_Exrc			 					},
/*u   */{"undo",	EX_UNDO,	ex_undo,	a_Lhs | q_Autop								},
/*una */{"unabbreviate",EX_UNABBR,	ex_map,		a_Bang | a_Lhs | q_Exrc | q_Custom | q_CtrlV				},
#ifdef FEATURE_ALIAS
/*unal*/{"unalias",	EX_UNALIAS,	ex_alias,	a_Lhs | q_Exrc | q_Custom						},
#endif
/*unb */{"unbreak",	EX_UNBREAK,	ex_map,		a_Bang | a_Lhs | q_CtrlV						},
/*unm */{"unmap",	EX_UNMAP,	ex_map,		a_Bang | a_Lhs | q_Exrc | q_Custom | q_CtrlV				},
/*v   */{"vglobal",	EX_VGLOBAL,	ex_global,	a_Range | a_Bang | a_RegExp | a_Cmds | d_All | q_Undo			},
/*ve  */{"version",	EX_VERSION,	ex_version,	q_Exrc									},
/*vi  */{"visual",	EX_VISUAL,	ex_edit,	a_Bang | a_Plus | a_File | q_SwitchB					},
/*w   */{"write",	EX_WRITE,	ex_write,	a_Range | a_Bang | a_Append | a_File | a_Filter | d_All | q_Unsafe	},
/*wa  */{"warning",	EX_WARNING,	ex_message,	a_Rhs | q_Exrc								},
/*if  */{"while",	EX_WHILE,	ex_while,	a_Cmds | q_Exrc								},
/*wi  */{"window",	EX_WINDOW,	ex_window,	a_Lhs									},
/*wn  */{"wnext",	EX_WNEXT,	ex_next,	a_Bang | q_Unsafe | q_SwitchB						},
/*wq  */{"wquit",	EX_WQUIT,	ex_xit,		a_Bang | a_File | d_File | q_MayQuit					},
/*x   */{"xit",		EX_XIT,		ex_xit,		a_Bang | a_File  | q_MayQuit						},
/*y   */{"yank",	EX_YANK,	ex_delete,	a_Range | a_Buffer | a_Count						},
/*z   */{"z",		EX_Z,		ex_z,		a_Line | a_Rhs								},
/*~   */{"~",		EX_SUBRECENT,	ex_substitute,	a_Range | a_Rhs | q_Undo						},
#ifdef FEATURE_ALIAS
/*~   */{" "/*dummy name*/,EX_DOALIAS,	ex_doalias,	a_Range | a_Bang | a_Rhs | q_Exrc 					},
#endif
};


/* This variable is used for detecting nested global statements */
static int 	globaldepth;


/* This function discards info from an EXINFO struct.  The struct itself
 * is not freed, since it is usually just a local variable in some function.
 */
void exfree(xinf)
	EXINFO	*xinf;	/* the command to be freed */
{
	int	i;

	if (xinf->fromaddr)	markfree(xinf->fromaddr);
	if (xinf->toaddr)	markfree(xinf->toaddr);
	if (xinf->destaddr)	markfree(xinf->destaddr);
	if (xinf->re)		safefree(xinf->re);
	if (xinf->lhs)		safefree(xinf->lhs);
	if (xinf->rhs)		safefree(xinf->rhs);
	for (i = 0; i < xinf->nfiles; i++)
	{
		assert(xinf->file && xinf->file[i]);
		safefree(xinf->file[i]);
	}
	if (xinf->file)		safefree(xinf->file);
}



/* This function skips over blanks and tabs */
static void skipwhitespace(refp)
	CHAR	**refp;	/* pointer to scan variable */
{
	while (*refp && (**refp == ' ' || **refp == '\t'))
	{
		scannext(refp);
	}
}




/* This function attempts to parse a window ID.  If there is no window ID,
 * then it leaves xinf->win unchanged; else it sets xinf->win to the given
 * window.  Returns True unless there was an error.
 *
 * This function also sets the default buffer to the default window's state
 * buffer, or the specified window's main buffer.  It is assumed that
 * xinf->window has already been initialized to the default window.
 */
static BOOLEAN parsewindowid(refp, xinf)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
{
	long	num;
	MARK	start;
	WINDOW	win;

	/* set default buffer, assuming default window. */
	if (xinf->window->state->pop)
		xinf->defaddr = *xinf->window->state->pop->cursor;
	else
		xinf->defaddr = *xinf->window->cursor;
	if (markbuffer(&xinf->defaddr) != bufdefault)
	{
		xinf->defaddr.buffer = bufdefault;
		xinf->defaddr.offset = 0L;
	}

	/* if doesn't start with a digit, ignore it */
	if (!*refp || !isdigit(**refp))
	{
		return True;
	}

	/* convert the number */
	start = scanmark(refp);
	for (num = 0; *refp && isdigit(**refp); scannext(refp))
	{
		num = num * 10 + **refp - '0';
	}

	/* if doesn't end with a ':', then it's not meant to be a window id */
	if (!*refp || **refp != ':' || num <= 0)
	{
		scanseek(refp, start);
		return True;
	}

	/* eat the ':' character */
	scannext(refp);

	/* convert the number to a WINDOW */
	for (win = winofbuf((WINDOW)0, (BUFFER)0);
	     win && o_windowid(win) != num;
	     win = winofbuf(win, (BUFFER)0))
	{
	}
	if (!win)
	{
		msg(MSG_ERROR, "bad window");
		return False;
	}

	/* use the named window */
	xinf->window = win;
	xinf->defaddr = *win->cursor;
	return True;
}

/* This function attempts to parse a single buffer name.  It returns True
 * unless an invalid buffer is specified.
 *
 * If an explicit buffer is named, then it sets the default address to that
 * buffer's undo point.  (Else it is left set window's cursor, as set by
 * the parsewindowid() function.)
 */
static BOOLEAN parsebuffername(refp, xinf)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
{
	CHAR	bufname[100];/* buffer name, as a string */
	int	len;
	BUFFER	buf;

	/* if the first character isn't '(' then this isn't a buffer name */
	if (!*refp || **refp != '(')
	{
		return True;
	}

	/* collect the characters into buffer */
	for (len = 0;
	     scannext(refp) && **refp != ')' && **refp != '\n'
		&& **refp != '|' && len < QTY(bufname) - 1;
	     )
	{
		bufname[len++] = **refp;
	}
	bufname[len] = '\0';

	/* consume the closing ')', if any */
	while (*refp && **refp != ')' && **refp != '\n' && **refp != '|')
	{
		scannext(refp);
	}
	if (*refp && **refp == ')')
	{
		scannext(refp);
	}

	/* try to find the buffer */
	buf = buffind(bufname);
	if (!buf)
	{
		msg(MSG_ERROR, "[S]no buffer named $1", bufname);
		return False;
	}
	xinf->defaddr.buffer = buf;
	marksetoffset(&xinf->defaddr, buf->docursor);
	return True;
}


/* If the command supports a regular expression, then this function parses
 * that regular expression from the command line and compiles it.  If the
 * command also supports replacement text, then that is parsed too.
 */
static BOOLEAN parseregexp(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	CHAR	delim;	/* delimiter character */
	CHAR	*retext;/* source code of the regular expression */

	/* If this command doesn't use regular expressions, then do nothing.
	 * (By the way, not using regexps implies that it doesn't use
	 * replacement text either.)
	 */
	if ((flags & a_RegExp) == 0)
	{
		return True;
	}

	/* get the delimiter character.  Can be any punctuation character
	 * except '|'.  If no delimiter is given, then use an empty string.
	 */
	if (*refp && ispunct(**refp) && **refp != '|')
	{
		/* remember the delimiter */
		delim = **refp;
		scannext(refp);

		/* collect the characters of the regexp source code */
		retext = (*refp && **refp) ? regbuild(delim, refp) : NULL;

		/* if an empty regexp was explicitly given (":s//" rather than
		 * ":s"), then remember that.  We need to distinguish between
		 * an explicit empty regexp and an implicit one, because if
		 * there is no explicit regexp then there can't be any
		 * replacement text either.
		 */
		if (!retext)
		{
			retext = empty;
			empty[0] = (CHAR)0;
		}
	}
	else
	{
		retext = NULL;
		delim = '\n'; /* illegal delimiter, affects a_RegSub text below */
		if (xinf->command == EX_SUBSTITUTE)
			xinf->command = EX_SUBAGAIN;
	}

	/* compile the regular expression */
	xinf->re = regcomp(retext ? retext : empty,
			   xinf->window ? xinf->window->state->cursor : NULL);

	/* We don't need the source to the regexp anymore.  If the source was
	 * anything other than the empty string, then free its memory.
	 */
	if (retext && retext != empty)
		safefree(retext);

	/* detect errors in the regexp */
	if (!xinf->re)
	{
		/* error message already written out by regcomp() */
		return False;
	}

	/* if no substitution text is needed, then we're done. */
	if ((flags & a_RegSub) == 0)
	{
		return True;
	}

	/* We do use replacement text.  If there was no regexp, then the
	 * replacement text is implied to be "~" -- so :s with no args will
	 * repeat the previous replacement command.
	 */
	if (!retext)
	{
		xinf->lhs = CHARdup(toCHAR(o_magic ? "~" : "\\~"));
		return True;
	}

	/* Collect characters up to the next delimiter to be the replacement
	 * text.  Same rules as the regular expression.  The first delimiter
	 * has already been comsumed.
	 */
	if (delim == '\n')
	{
		/* no delimiter implies that there is no replacement text */
		xinf->lhs = (CHAR *)safealloc(1, sizeof(CHAR));
	}
	else
	{
		for (; *refp && **refp != '\n' && **refp != delim; scannext(refp))
		{
			/* if backslash followed by delimiter, then just add
			 * delimiter.  Else add both the backslash and whatever
			 * other character followed it.
			 */
			if (**refp == '\\')
			{
				if (!scannext(refp))
				{
					buildCHAR(&xinf->lhs, '\\');
				}
				else if (**refp == delim)
				{
					buildCHAR(&xinf->lhs, delim);
				}
				else
				{
					buildCHAR(&xinf->lhs, '\\');
					buildCHAR(&xinf->lhs, **refp);
				}
			}
			else
			{
				buildCHAR(&xinf->lhs, **refp);
			}
		}
		if (!xinf->lhs)
		{
			/* zero-length string, if nothing else */
			xinf->lhs = (CHAR *)safealloc(1, sizeof(CHAR));
		}

		/* consume the closing delimiter */
		if (*refp && **refp == delim)
		{
			scannext(refp);
		}
	}

	return True;
}


/* This function parses an address, and places the results in xinf->to.
 * Returns True unless there is an error.
 */
BOOLEAN exparseaddress(refp, xinf)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
{
	BLKNO	bi;	/* bufinfo block of the buffer being addressed */
	long	lnum;	/* line number */
	long	delta;	/* movement amount */
	CHAR	sign;	/* '+' or '-' for delta */
	long	start;	/* where search stared, so we can detect wrap */
	long	buflines;/* number of lines in buffer */
	MARKBUF	m;	/* start of a line */
	CHAR	ch;
	regexp	*re;

	/* if nothing, do nothing */
	if (!*refp)
		return True;

	/* find the default line number */
	bi = bufbufinfo(markbuffer(&xinf->defaddr));
	lnum = markline(&xinf->defaddr);
	xinf->fromoffset = 0;

	/* parse the address */
	switch (**refp)
	{
	  case '.':
		/* use the line containing the default address */
		scannext(refp);
		break;

	  case '/':
	  case '?':
		/* if buffer is empty then fail */
		if (o_bufchars(markbuffer(&xinf->defaddr)) <= 1)
		{
			msg(MSG_ERROR, "no match");
			return False;
		}

		/* parse & compile the regular expression */
		delta = (**refp == '?') ? -1 : 1;
		if (!parseregexp(refp, xinf, a_RegExp))
		{
			return False;
		}

		/* allow the visual 'n' and 'N' commands to search for the
		 * same regexp, unless the "saveregexp" option is turned off.
		 */
		re = xinf->re;
		if (o_saveregexp)
		{
			if (searchre)
			{
				safefree(searchre);
			}
			searchre = xinf->re;
			searchforward = (BOOLEAN)(delta > 0);
			searchhasdelta = False;
			xinf->re = NULL;
		}

		/* search for the regular expression */
		start = lnum;
		buflines = o_buflines(markbuffer(&xinf->defaddr));
		do
		{
			/* find next line */
			lnum += delta;
			if (o_wrapscan)
			{
				if (lnum == 0)
					lnum = buflines;
				else if (lnum > buflines)
					lnum = 1;
			}
			else if (lnum == 0)
			{
				msg(MSG_ERROR, "no match above");
				return False;
			}
			else if (lnum > buflines)
			{
				msg(MSG_ERROR, "no match below");
				return False;
			}

			/* see if this line contains the regexp */
			if (regexec(re, marktmp(m, markbuffer(&xinf->defaddr), lowline(bi, lnum)), True))
			{
				delta = 0;
			}

			/* if we've wrapped back to our starting point,
			 * then we've failed.
			 */
			if (lnum == start)
			{
				msg(MSG_ERROR, "no match");
				return False;
			}

			/* did the user cancel the search? */
			if (guipoll(False))
			{
				return False;
			}

		} while (delta != 0);

		/* If we get here, then we've found a match, and lnum is set
		 * to its line number.  Good!
		 */
		xinf->fromoffset = (re->leavep >= 0 ? re->leavep : re->startp[0]);
		break;

	  case '\'':
	  case '`':
		ch = **refp;
		if (!scannext(refp) || **refp < 'a' || **refp > 'z')
		{
			msg(MSG_ERROR, "bad mark name");
			return False;
		}
		else if (!namedmark[**refp - 'a'])
		{
			msg(MSG_ERROR, "[C]'$1 unset", **refp);
			return False;
		}
		else if (markbuffer(&xinf->defaddr) != markbuffer(namedmark[**refp - 'a']))
		{
			if (xinf->anyaddr)
			{
				msg(MSG_ERROR, "would span buffers");
				return False;
			}
			xinf->defaddr = *namedmark[**refp - 'a'];
		}
		lnum = markline(namedmark[**refp - 'a']);
		if (ch == '`')
			xinf->fromoffset = markoffset(namedmark[**refp - 'a']);
		scannext(refp);
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
		for (lnum = 0; *refp && isdigit(**refp); scannext(refp))
		{
			lnum = lnum * 10 + **refp - '0';
		}
		if (lnum < 0 || lnum > o_buflines(markbuffer(&xinf->defaddr)))
		{
			msg(MSG_ERROR, "bad line number");
			return False;
		}
		break;

	  case '$':
		scannext(refp);
		lnum = o_buflines(markbuffer(&xinf->defaddr));
		break;

	  default:
		/* use the default address, but don't consume any chars */
		lnum = markline(&xinf->defaddr);
	}

	/* followed by a delta? */
	skipwhitespace(refp);
	if (*refp && (**refp == '+' || **refp == '-' || isdigit(**refp)))
	{
		/* delta implies whole-line addressing */
		xinf->fromoffset = 0;

		/* find the sign & magnitude of the delta */
		sign = **refp;
		if (isdigit(sign))
			sign = '+';
		else
			scannext(refp);
		for (delta = 1; **refp == sign; scannext(refp), delta++)
		{
		}
		if (delta == 1 && *refp && isdigit(**refp))
		{
			for (delta = **refp - '0';
			     scannext(refp) && isdigit(**refp);
			     delta = delta * 10 + **refp - '0')
			{
			}
		}

		/* add the delta to the line number */
		if (sign == '+')
			lnum += delta;
		else
			lnum -= delta;

		/* if sum is invalid, complain */
		if (lnum < 1 || lnum > o_buflines(markbuffer(&xinf->defaddr)))
		{
			msg(MSG_ERROR, "bad delta");
		}
	}

	xinf->to = lnum;
	return True;
}


/* This parses a command name, and sets xinf->command accordingly.  Returns
 * True if everything is okay, or False if the command is unrecognized or
 * was given addresses but doesn't allow addresses.
 */
static BOOLEAN parsecommandname(refp, xinf)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
{
	BOOLEAN	anymatch;
	int	matches;	/* number of commands that match so far */
	int	firstmatch;	/* first command that matched */
	char	cmdname[20];	/* command name */
	int	len;		/* number of characters in name so far */
	MARK	start;		/* where the command name started */
	int	i;

	/* if no command, then assume either "goto" or comment */
	if (!*refp || **refp == '\n' || **refp == '|')
	{
		strcpy(cmdname, (xinf->anyaddr || (xinf->window && markbuffer(xinf->window->cursor) != xinf->defaddr.buffer)) ? "goto" : "\"");
		for (firstmatch = QTY(cmdnames) - 1;
		     firstmatch >= 0 && strcmp(cmdnames[firstmatch].name, cmdname);
		     firstmatch--)
		{
		}
		goto Found;
	}

	/* begin by checking for aliases */
	start = scanmark(refp);
#ifdef FEATURE_ALIAS
	for (len = 0; len < QTY(cmdname) - 1 && *refp && isalnum(**refp); scannext(refp))
	{
		cmdname[len++] = **refp;
	}
	cmdname[len] = '\0';
	xinf->cmdname = exisalias(cmdname, False);
	if (xinf->cmdname)
	{
		xinf->cmdidx = QTY(cmdnames) - 1;
		xinf->command = EX_DOALIAS;
		return True;
	}
	scanseek(refp, start);
#endif /* FEATURE_DOALIAS */

	/* start with shortest possible command name, and extend the command
	 * name as much as possible without eliminating all commands from
	 * matching.  When we get it as long as possible, then the first
	 * matching name is the one we'll use.
	 */
	len = 0;
	firstmatch = 0;
	anymatch = False;
	do
	{
		/* copy the first (or next) character of the name into a buffer */
		cmdname[len++] = **refp;

		/* see how many commands match this command so far */
		for (matches = 0, i = firstmatch; i < QTY(cmdnames); i++)
		{
			/* In the cmdnames[] array, commands with the same
			 * initial letters are grouped together.  Have we
			 * reached the end of the group that interests us?
			 */
			if (len > 1 && (cmdnames[i].name[0] != cmdname[0] || strncmp(cmdnames[i].name, cmdname, (size_t)(len - 1))))
			{
				/* no more matches are possible -- we reached
				 * the end of this (len-1) letter group.
				 */
				break;
			}
			if ((CHAR)cmdnames[i].name[len - 1] == **refp)
			{
				/* Partial match, but keep looking.  If this
				 * was the first match with this length then
				 * remember it.
				 */
				matches++;
				if (matches == 1)
				{
					firstmatch = i;
					anymatch = True;
				}
			}
		}

	} while (matches > 0 && scannext(refp) && isalnum(**refp));

	/* at this point, if "matches" is zero and "firstmatch" has been set,
	 * then we may have read one too many characters for the command name,
	 * so we need to adjust the position of the *refp.
	 */
	if (matches == 0
		&& anymatch
		&& (strlen(cmdnames[firstmatch].name) == (unsigned)(len - 1)
			|| !*refp
			|| !isalpha(**refp)))
	{
		len--;
		markaddoffset(start, len);
		scanseek(refp, start);
		matches = 1;
	}

	/* If we still haven't found anything, give up! */
	if (matches == 0)
	{
		cmdname[len] = '\0';
		msg(MSG_ERROR, "[s]bad command name $1", cmdname);
		return False;
	}

	/* so I guess we found a match. */
Found:
	assert(firstmatch >= 0);
	xinf->cmdidx = firstmatch;
	xinf->command = cmdnames[firstmatch].code;
	xinf->cmdname = cmdnames[firstmatch].name;
	return True;
}


/* This function parses multiplied command names, as in :<<<<.  This can't
 * possibly fail, so this isn't a boolean function.
 */
static void parsemulti(refp, xinf)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
{
	/* the default for all commands is 1. */
	xinf->multi = 1;

	/* if this command can be multiplied, then see how many we have */
	if (cmdnames[xinf->cmdidx].flags & a_Multi)
	{
		while (*refp && **refp == (CHAR)xinf->cmdname[0])
		{
			xinf->multi++;
			scannext(refp);
		}
	}
}

/* This function parses an optional '!' character for some commands.  This
 * can't possibly fail, so this isn't a boolean function.
 */
static void parsebang(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	/* if this supports '!', and a '!' character really is given, then
	 * consume the '!' character and set the "bang" flag.
	 */
	if ((flags & a_Bang) != 0 && *refp && **refp == '!')
	{
		scannext(refp);
		xinf->bang = True;
	}
}


/* This function parses an optional "+line" argument, for commands which
 * support it.
 */
static void parseplus(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	/* if the command doesn't support "+line" or the next character isn't
	 * '+' then do nothing.
	 */
	if ((flags & a_Plus) == 0 || !*refp || **refp != '+')
	{
		return;
	}

	/* nothing else should have used the xinf->lhs field yet */
	assert(!xinf->lhs);

	/* skip the '+' */
	scannext(refp);

	/* collect following characters up to next whitespace or '|' */
	while (*refp && !isspace(**refp) && **refp != '|')
	{
		buildCHAR(&xinf->lhs, **refp);
		scannext(refp);
	}
}


/* This function parses an optional print flag and delta, if the command
 * supports them.
 */
static void parseprintflag(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	CHAR		sign;
	static PFLAG	pflag = PF_NONE;

	/* if no print flag is possible, skip parsing this */
	if (!*refp || (flags & a_Pflag) == 0)
	{
		goto NoFlag;
	}

	/* try to parse a print flag here */
	switch (**refp)
	{
	  case 'p':
		scannext(refp);
		xinf->pflag = PF_PRINT;
		break;

	  case 'l':
		scannext(refp);
		if (*refp && **refp == '#')
		{
			scannext(refp);
			xinf->pflag = PF_NUMLIST;
		}
		else
		{
			xinf->pflag = PF_LIST;
		}
		break;

	  case '#':
		scannext(refp);
		if (*refp && **refp == 'l')
		{
			scannext(refp);
			xinf->pflag = PF_NUMLIST;
		}
		else
		{
			xinf->pflag = PF_NUMBER;
		}
		break;

	  default:
		goto NoFlag;
	}

	/* we have a print flag -- tweak it via "list" and "number" options */
	if (o_number(xinf->window))
		xinf->pflag = (xinf->pflag == PF_LIST || xinf->pflag == PF_NUMLIST) ? PF_NUMLIST : PF_NUMBER;
	if (o_list(xinf->window))
		xinf->pflag = (xinf->pflag == PF_NUMBER || xinf->pflag == PF_NUMLIST) ? PF_NUMLIST : PF_LIST;

	/* now see if we have a delta */
	if (refp && (**refp == '+' || **refp == '-'))
	{
		sign = **refp;
		while (scannext(refp) && isdigit(**refp))
		{
			xinf->delta = xinf->delta * 10 + **refp - '0';
		}
		if (sign == '-')
		{
			xinf->delta = -xinf->delta;
		}
	}

	/* remember this print flag as the default for next time */
	pflag = xinf->pflag;
	return;

NoFlag:	/* see if we're supposed to autoprint this command.  If so, then
	 * assume the previous print flag is the default to use here.
	 */
	if ((flags & q_Autop) != 0
	 && o_autoprint
	 && globaldepth == 0
	 && xinf->window
	 && xinf->window->state
	 && (xinf->window->state->flags & (ELVIS_POP|ELVIS_1LINE)) == 0
	 && xinf->window->state->enter
	 && markbuffer(scanmark(refp)) == buffind(toCHAR(EX_BUF)))
	{
		if (pflag == PF_NONE)
			pflag = PF_PRINT;
		xinf->pflag = pflag;
	}
}

/* This function parses single word which doesn't contain any unquoted
 * whitespace (if the command supports this).
 */
static void parselhs(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	CHAR	quote;	/* quote character -- either ^V or \ */

	/* if the command doesn't use lhs, then do nothing */
	if ((flags & a_Lhs) == 0)
		return;

	/* choose the proper quote character */
	quote = (flags & q_CtrlV) ? ELVCTRL('V') : '\\';

	/* collect characters up to next whitespace or end of command */
	while (*refp && **refp != ' ' && **refp != '\t' && **refp != '\n'
		&& (**refp != '|' || (*refp)[1] == '\n'))
	{
		/* backslash followed by whitespace becomes just whitespace */
		if (**refp == quote)
		{
			scannext(refp);
			if (!*refp)
			{
				buildCHAR(&xinf->lhs, quote);
			}
			else if (**refp == ' ' || **refp == '\t' || **refp == '|' || quote == ELVCTRL('V'))
			{
				buildCHAR(&xinf->lhs, **refp);
			}
			else
			{
				buildCHAR(&xinf->lhs, quote);
				buildCHAR(&xinf->lhs, **refp);
			}
		}
		else
		{
			/* unquoted character */
			buildCHAR(&xinf->lhs, **refp);
		}
		scannext(refp);
	}
}

/* This function parses an optional print flag and delta, if the command
 * supports them.
 */
static void parserhs(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	CHAR	quote;	/* quote character -- either ^V or \ */

	/* if the command doesn't use rhs, then do nothing */
	if ((flags & a_Rhs) == 0)
		return;

	/* choose the proper quote character */
	quote = (flags & q_CtrlV) ? ELVCTRL('V') : '\\';

	/* collect characters up to end of command */
	while (*refp && **refp != '\n' && **refp != '|')
	{
		/* backslash followed by whitespace becomes just whitespace */
		if (**refp == quote)
		{
			scannext(refp);
			if (!*refp)
			{
				buildCHAR(&xinf->rhs, quote);
			}
			else if (**refp == '|' || quote == ELVCTRL('V'))
			{
				buildCHAR(&xinf->rhs, **refp);
			}
			else
			{
				buildCHAR(&xinf->rhs, quote);
				buildCHAR(&xinf->rhs, **refp);
			}
		}
		else
		{
			/* unquoted character */
			buildCHAR(&xinf->rhs, **refp);
		}
		scannext(refp);
	}
}

/* This function parses an optional cut buffer name, if the command
 * supports them.  A cut buffer name is a single letter, or a " followed
 * a letter, digit, or another ".
 */
static void parsecutbuffer(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	/* if the command doesn't support cut buffer names, do nothing */
	if ((flags & a_Buffer) == 0)
		return;

	/* if @ then use the anonymous cut buffer */
	if (*refp && **refp == '@')
	{
		xinf->cutbuf = '1';
		scannext(refp);
	}

	/* if double-quote, then following character is buffer name.  Also,
	 * letters letters, digits, <, and > don't require a " character.
	 */
	if (*refp && (isalnum(**refp) || **refp == '<' || **refp == '>' || (**refp == '"' && scannext(refp))))
	{
		xinf->cutbuf = **refp;
		scannext(refp);
	}
}

/* This function parses an optional count, if the command supports them.
 * A count is a series of digits.  If no count is given, then the count
 * field is set to -1.
 */
static void parsecount(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	if ((flags & a_Count) == 0 || !*refp || !isdigit(**refp))
	{
		/* no count given */
		xinf->count = -1;
	}
	else
	{
		/* count given -- convert it to binary */
		do
		{
			xinf->count = xinf->count * 10 + **refp - '0';
			scannext(refp);
		} while (*refp && isdigit(**refp));
	}
}


/* This function parses an optional command list, if the command supports them.
 * A command list is a string which may contain any characters except newline.
 * (In particular, '|' is allowed.)  Returns RESULT_COMPLETE if successful,
 * RESULT_ERROR if unsuccessful, or RESULT_MORE if we hit the end of the buffer
 * while looking for the end of a multi-line command.
 */
static RESULT parsecmds(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	BOOLEAN	shell;	/* is this command for a shell? */
	BOOLEAN	bol;	/* at front of a line? */
	CHAR	*scan;
	CHAR	ch;
	MARKBUF	end;
	MARK	start;
	int	i;
	int	nest;

	/* if the command doesn't use cmds, then do nothing */
	if ((flags & a_Cmds) == 0)
		return RESULT_COMPLETE;

	/* determine whether this command is for a shell or for ex */
	shell = (BOOLEAN)(xinf->command == EX_BANG || (flags & a_Filter) != 0);

	/* skip whitespace */
	skipwhitespace(refp);

	/* ex commands allow a { ... } syntax */
	if (!shell && *refp && **refp == '{')
	{
		/* collect characters up to next } character which appears
		 * at the front of a line.  Ignore blank lines.
		 */
		for (ch = '\n', nest = 0; scannext(refp); )
		{
			if (ch == '\n' && **refp == '}')
			{
				if (nest == 0)
					break;
				nest--;
			}
			if (ch == '{' && **refp == '\n')
				nest++;
			if (ch == '\n' && (**refp == ' ' || **refp == '\t'))
				continue;
			if (ch != '\n' || **refp != '\n')
				buildCHAR(&xinf->rhs, **refp);
			ch = **refp;
		}

		/* if we hit end without finding } then we need more. */
		if (!*refp)
		{
			return RESULT_MORE;
		}

		/* eat the closing } */
		scannext(refp);

		return RESULT_COMPLETE;
	}

	/* ex commands allow a ...backslash-newline... syntax */
	if (!shell)
	{
		for (bol = False; *refp && **refp && **refp != '\n'; scannext(refp))
		{
			/* if backslash which preceeds newline, then delete
			 * the backslash and treat the newline literally.
			 */
			bol = False;
			if (**refp == '\\')
			{
				if (!scannext(refp))
					continue;
				if (**refp != '\n')
					buildCHAR(&xinf->rhs, '\\');
				else
					bol = True;
			}
			buildCHAR(&xinf->rhs, **refp);
		}
		if (xinf->rhs)
			buildCHAR(&xinf->rhs, '\n');

		return bol ? RESULT_MORE : RESULT_COMPLETE;
	}

	/* shell commands are more complex, due to substitutions... */
	bol = True;
	while (*refp && **refp && **refp != '\n')
	{
		/* copy most characters literally, but perform
		 * substitutions for !, %, #
		 */
		ch = **refp;
		switch (ch)
		{
		  case '%':
			scan = o_filename(markbuffer(&xinf->defaddr));
			break;

		  case '#':
			scan = buffilenumber(refp);
			break;

		  case '!':
			if (bol)
			{
				scan = o_previouscommand;
			}
			else
			{
				buildCHAR(&xinf->rhs, **refp);
				scan = xinf->rhs;
			}
			break;

		  case '\\':
			scannext(refp);
			if (*refp && !CHARchr(toCHAR("%#!@"), **refp)) /* !!!removed '\\' */
			{
				buildCHAR(&xinf->rhs, (_CHAR_)'\\');
				ch = **refp;
			}
			else if (*refp && **refp == '@')
			{
				if (!xinf->window)
				{
					msg(MSG_ERROR, "can't use \\\\@ during initialization");
				}
				end = *xinf->window->cursor;
				start = wordatcursor(&end);
				if (!start)
				{
					msg(MSG_ERROR, "cursor not on word");
					return RESULT_ERROR;
				}
				for (i = markoffset(&end) - markoffset(start),
					scanalloc(&scan, start);
				     scan && i > 0;
				     scannext(&scan), i--)
				{
					buildCHAR(&xinf->rhs, *scan);
				}
				scanfree(&scan);

				/* don't add anything else */
				scan = xinf->rhs;
				break;
			}
			/* fall through for all but \@ ... */

		  default:
			buildCHAR(&xinf->rhs, **refp);
			scan = xinf->rhs;
		}

		/* are we trying to add a string? */
		if (scan != xinf->rhs)
		{
			/* we want to... but do we have the string? */
			if (scan)
			{
				for ( ; *scan; scan++)
					buildCHAR(&xinf->rhs, *scan);
			}
			else
			{
				msg(MSG_ERROR, "[C]no value to substitute for $1", ch);
				return RESULT_ERROR;
			}
		}

		/* Move on to the next character.  Note that we need to
		 * check (*refp) because it may have become NULL during
		 * the processing of a backslash.
		 */
		if (*refp)
			scannext(refp);
		bol = False;
	}

	/* if shell command, then remember it for later ! substitutions */
	if (shell)
	{
		if (!xinf->rhs)
			return RESULT_ERROR; /* blank commands are illegal */
		if (o_previouscommand)
			safefree(o_previouscommand);
		o_previouscommand = CHARkdup(xinf->rhs);
	}

	return RESULT_COMPLETE;
}


/* This is a "helper" function for parsefileargs().  This function adds a
 * single item to the list of filename arguments.  Optionally, it can
 * attempt to expand wildcards and recursively add each matching filename.
 * Returns True if successful, False if error.
 */
BOOLEAN exaddfilearg(file, nfiles, filename, wild)
	char	***file;	/* ptr to a dynamic array of char* pointers */
	int	*nfiles;	/* number of files in file array */
	char	*filename;	/* the filename (or wildcard expression) to add */
	BOOLEAN	wild;		/* if True, expand wildcards */
{
	char	*match;		/* a name matching a wildcard */
	char	**old;		/* previous value of xinf->file pointer */
	int	start;		/* index into xinf->file of wildcard's expansion */
	BOOLEAN	mustfree=False;	/* if True, then free "match" before returning*/
	char	slash;		/* local slash character */
	char	tmp[256];
	int	i;

	/* never use wildcards for URLs */
	if (((match = urllocal(filename)) != NULL && match != filename)
#if defined(PROTOCOL_HTTP) || defined(PROTOCOL_FTP)
		|| urlremote(filename)
#endif
	)
	{
		wild = False;
	}

	/* are we supposed to match wildcards? */
	if (wild)
	{
		/* expand any environment variables */
		filename = tochar8(calculate(toCHAR(filename), NULL, True));
		if (!filename)
			return False;

		/* If this operating system uses backslashes between names,
		 * then replace any forward slashes with backslashes.  This is
		 * intended to allow MS-DOS users to type forward slashes in
		 * their filenames, if they so prefer.
		 */
		slash = dirpath("a", "b")[1];
		if (slash == '\\')
		{
			for (i = 0; filename[i]; i++)
			{
				if (filename[i] == '/')
				{
					filename[i] = '\\';
				}
			}
		}

		/* replace ~ with the name of the home directory */
#if 1
		match = NULL;
		if (filename[0] == '~' && filename[1] == '+')
			match = dircwd(), i = 2;
		else if (filename[0] == '~' && filename[1] == '-')
			match = tochar8(o_previousdir), i = 2;
		else if (filename[0] == '~')
			match = tochar8(o_home), i = 1;
		if (match && filename[i] == slash && filename[i + 1])
		{
			filename = safedup(dirpath(match, filename + i + 1));
			mustfree = True;
		}
		else if (match && (filename[i] == slash || !filename[i]))
		{
			filename = safedup(match);
			mustfree = True;
		}
# if ANY_UNIX
		else if (filename[0] == '~' && isalpha(filename[1]))
		{
			filename = safedup(expanduserhome(tochar8(filename), tmp));
			mustfree = True;
		}
#endif /*ANY_UNIX*/

#else
		if (filename[0] == '~' && !isalpha(filename[1]))
		{
			filename = safedup(filename[1]
				    ? dirpath(tochar8(o_home), &filename[2])
				    : tochar8(o_home));
			mustfree = True;
		}
#endif

		/* If there are any matches... */
		if ((match = dirfirst(filename, False)) != NULL)
		{
			/* for each match... */
			start = *nfiles;
			do
			{
				/* add the matching name */
				exaddfilearg(file, nfiles, match, False);

				/* ripple the new name back, to keep things sorted */
				for (i = *nfiles - 1;
				     i > start && strcmp((*file)[i], (*file)[i - 1]) < 0;
				     i--)
				{
					match = (*file)[i];
					(*file)[i] = (*file)[i - 1];
					(*file)[i - 1] = match;
				}
			} while ((match = dirnext()) != (char *)0);
		}

		/* if necessary, free the filename */
		if (mustfree)
		{
			safefree(filename);
		}
	}
	else /* literal filename */
	{
		/* [re-]allocate the *files array if necessary.  Note that
		 * we've arranged it so there will always be at least one
		 * NULL pointer at the end of the list.
		 */
		if (*nfiles == 0 || (*nfiles + 1) % FILEGRAN == 0)
		{
			old = *file;
			*file = (char **)safealloc(*nfiles + 1 + FILEGRAN, sizeof(char *));
			if (old)
			{
				memcpy(*file, old, *nfiles * sizeof(char *));
				safefree(old);
			}
		}

		/* append the new filename */
		(*file)[(*nfiles)++] = safedup(filename);
	}
	return True;
}

/* This function parses filenames, if the command supports them. */
static BOOLEAN parsefileargs(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	CHAR	*filename;
	CHAR	*scan;

	/* if no filenames expected, or hit end of cmd, then do nothing */
	if ((flags & (a_File|a_Files|a_Filter|a_Append)) == 0 || !*refp)
	{
		return True;
	}

	/* If filter is allowed, and next character is a '!' the we have
	 * a filter command.
	 */
	if ((flags & a_Filter) != 0 && **refp == '!')
	{
		/* skip the initial '!' */
		if (!scannext(refp))
			return True;	/* missing filter will be detected later */

		/* begin the rhs argument with a bang */
		buildCHAR(&xinf->rhs, (_CHAR_)'!');

		/* collect characters up to next newline */
		return (BOOLEAN)(RESULT_COMPLETE == parsecmds(refp, xinf, flags | a_Cmds));
	}

	/* An initial backslash could have been used to quote a ! or > at
	 * the start of a filename.  If the first character is backslash,
	 * and the second is either ! or > then skip the backslash.  Also,
	 * if the second is another backslash and backslash isn't used as
	 * the directory separator on this operating system, then skip the
	 * backslash; this should allow "\\foo" to be "\foo" under UNIX,
	 * while still allowing "\\machine\dir\file" to be parsed as a UNC
	 * name under Win32.
	 */
	if (*refp && **refp == '\\')
	{
		scannext(refp);
		if (!*refp)
		{
			msg(MSG_ERROR, "oops");
			return False;
		}
		if (**refp != '!' && **refp != '>' &&
			(**refp != '\\' || dirpath("a", "b")[1] == '\\'))
		{
			scanprev(refp);
		}
	}

	/* collect each whitespace-delimited filename argument */
	for (filename = NULL; *refp && **refp != '|' && **refp != '\n'; scannext(refp))
	{
		/* if whitespace, then process filename (if any) and then skip */
		switch (**refp)
		{
		  case ' ':
		  case '\t':
			/* do we have a filename? */
			if (filename)
			{
				/* store the name */
				if (!exaddfilearg(&xinf->file, &xinf->nfiles, tochar8(filename), True))
					goto Error;

				/* free the buildCHAR copy of the name */
				safefree(filename);
				filename = NULL;
			}
			break;

		  case '\\':
			/* backslash can be used to quote some characters. */
			scannext(refp);
			if (!*refp)
				break; /* hit end of string */
			if (**refp == '*' && dirpath("a", "b")[1] == '\\')
			{
				/* This operating system seems use backslashes
				 * in filenames.  So an expression such as
				 * "foo\*.c" should NOT become "foo*.c"
				 */
				scanprev(refp);
			}
			else if (!CHARchr(toCHAR(" \t\\#!*"), **refp))
			{
				/* backslash isn't followed by a char that might
				 * require backslash quoting, so the backslash
				 * must be here for some other reason.  Keep it.
				 */
				scanprev(refp);
			}
			else if (**refp == '\\' && filename == NULL)
			{
				/* A double-backslash at the start of a name
				 * is kept, so Win32 users can have names like
				 * \\machine\directory\file
				 */
				scanprev(refp);
			}
			buildCHAR(&filename, **refp);
			break;

		  case '#':
			scan = buffilenumber(refp);
			if (!scan)
			{
				msg(MSG_ERROR, "[c]no value to substitute for $1", '#');
				goto Error;
			}
			for ( ; *scan; scan++)
			{
				buildCHAR(&filename, *scan);
			}
			break;

		  case '%':
			scan = o_filename(markbuffer(&xinf->defaddr));
			if (scan)
			{
				for ( ; *scan; scan++)
				{
					buildCHAR(&filename, *scan);
				}
			}
			else
			{
				msg(MSG_ERROR, "[c]no value to substitute for $1", '%');
				goto Error;
			}
			break;

		  case '\0':
			msg(MSG_ERROR, "NUL not allowed in file name");
			goto Error;

		  default:
			/* Append the character to the name.  If this results
			 * in a partial name of ">>" and either this isn't the
			 * first file, or this command doesn't support appending
			 * then complain.
			 */
			if (buildCHAR(&filename, **refp) == 2
			 && !CHARcmp(filename, ">>")
			 && (xinf->nfiles > 0 || (flags & a_Append) == 0))
			{
				msg(MSG_ERROR, "bad >>", xinf->cmdname);
				goto Error;
			}
		}
	}

	/* if we were working on a filename when we ended the loop, add it */
	if (filename)
	{
		if (!exaddfilearg(&xinf->file, &xinf->nfiles, tochar8(filename), True))
			goto Error;
		safefree(filename);
		filename = (CHAR *)0;
	}

	/* complain if we have multiple filenames and only support one */
	if (xinf->nfiles > 1 && (flags & a_Files) == 0)
	{
		msg(MSG_ERROR, "too many files");
		goto Error;
	}

	/* If no files named, maybe go for a default name */
	if (xinf->nfiles == 0 && (flags & d_File) != 0)
	{
		if (o_filename(markbuffer(&xinf->defaddr)) == NULL)
		{ /* nishi */
			msg(MSG_ERROR, "no file name");
			goto Error;
		}
		exaddfilearg(&xinf->file, &xinf->nfiles, tochar8(o_filename(markbuffer(&xinf->defaddr))), False);
	}

	return True;


Error:
	/* If we were in the middle of a filename, free the filename.
	 * Other stuff will be freed by the calling function.
	 */
	if (filename)
	{
		safefree(filename);
	}
	return False;
}


/* Parse a single command, and leave *refp pointing past the last character of
 * the command.  Return RESULT_COMPLETE normally, RESULT_MORE if the command is
 * incomplete, or RESULT_ERROR for an error (after outputting an error message).
 */
static RESULT parse(win, refp, xinf)
	WINDOW	win;	/* window that the command applies to */
	CHAR	**refp;	/* pointer to (CHAR *) used for scanning the command */
	EXINFO	*xinf;	/* where to place the results of the parse */
{
	CHAR	sep;	/* separator character, from scanning */
	MARKBUF	orig;	/* original mark of "refp", so we can seek back later */
	MARKBUF	rngdef;	/* default range */
	long	rngfrom;/* "from" line number of range */
	long	rngto;	/* "to" line number of range */
	long	flags;	/* bitmap of command attributes */
	BOOLEAN	sel;	/* did address come from selection? */
	CHAR	*p2;	/* a second scanning variable */
	CHAR	*lntext;/* text of current line, used for trace */
	RESULT	result;	/* result of parsing */
	BOOLEAN	twoaddrs;/* have two addresses been seen on this line? */
	BUFFER	log;	/* log buffer, for debugging */
	MARKBUF	logstart; /* start of the log buffer, for debugging */
	MARKBUF	logend;	/* end of the log buffer, for debugging */
	long	loglen;	/* length of log message */
	CHAR	*logstr;/* start of string command, or NULL if from buffer */
	int	i;

	/* if verbose, then display this line on stderr */
	if (o_verbose >= 3 && !win)
	{
		lntext = NULL;
		for (scandup(&p2, refp); p2 && *p2 != '\n'; scannext(&p2))
		{
			buildCHAR(&lntext, *p2);
		}
		scanfree(&p2);
		if (lntext)
		{
			msg(MSG_INFO, "[S]:$1", lntext);
			safefree(lntext);
		}
	}

	/* set defaults */
	memset((char *)xinf, 0, sizeof *xinf);
	xinf->window = win;
	twoaddrs = False;

	/* skip leading ':' characters and whitespace */
	while (*refp && (**refp == ':' || **refp == ' ' || **refp == '\t'))
	{
		scannext(refp);
	}

	/* remember where this command started */
	orig = *scanmark(refp);
	logstr = markbuffer(&orig) ? NULL : *refp;

	/* If no command was entered, then assume the command line should be
	 * "+p", so the user can simply hit <Enter> to step through the file.
	 * Only do this for interactively entered commands, though!
	 */
	if ((!*refp || **refp == '\n')
		&& markbuffer(&orig) == buffind(toCHAR(EX_BUF)))
	{
		/* parse the "+p" command, unless at end of buffer */
		if (markline(win->cursor) < o_buflines(markbuffer(win->cursor)))
		{
			scanstring(&p2, toCHAR("+p"));
			result = parse(win, &p2, xinf);
			scanfree(&p2);
		}
		else
		{
			msg(MSG_ERROR, "no more lines");
			result = RESULT_ERROR;
		}

		return result;
	}

	/* parse the window id */
	if (win)
	{
		skipwhitespace(refp);
		if (!parsewindowid(refp, xinf))
		{
			return RESULT_ERROR;
		}
	}
	else
	{
		xinf->defaddr.buffer = bufdefault;
	}

	/* parse the buffer name (unless window has visible selection) */
	skipwhitespace(refp);
	if ((!xinf->window || !xinf->window->seltop) && !parsebuffername(refp, xinf))
	{
		return RESULT_ERROR;
	}

	/* parse addresses */
	sel = False;
	if (xinf->defaddr.buffer)
	{
		skipwhitespace(refp);
		if (xinf->window && xinf->window->seltop)
		{
			xinf->defaddr = *xinf->window->seltop;
			sel = True;
		}
		xinf->to = markline(&xinf->defaddr);
		xinf->from = xinf->to;
		if (*refp && **refp == '%')
		{
			scannext(refp);
			xinf->from = 1;
			xinf->to = o_buflines(markbuffer(&xinf->defaddr));
			xinf->anyaddr = True;
			sel = False;
		}
		else if ((xinf->window && xinf->window->seltop)
			|| (*refp && strchr("./?'0123456789$+-,;", (char)**refp)))
		{
			do
			{
				/* Shift addresses, so we always remember the
				 * last two addresses.
				 */
				xinf->from = xinf->to;

				/* Parse the address */
				if (!exparseaddress(refp, xinf))
				{
					return RESULT_ERROR;
				}

				/* We now have at least one address.  If this
				 * is the first address we've encountered,
				 * then copy it into "from" as well.
				 */
				if (!xinf->anyaddr)
				{
					xinf->from = xinf->to;
					if (xinf->window && xinf->window->selbottom)
						xinf->to = markline(xinf->window->selbottom);
					xinf->anyaddr = True;
				}
				else
				{
					twoaddrs = True;
				}

				/* if followed by a semicolon, then this
				 * address becomes default.
				 */
				skipwhitespace(refp);
				if (*refp)
				{
					sep = **refp;
				}
				else
				{
					sep = '\0';
				}
				if (sep == ';')
				{
					marksetoffset(&xinf->defaddr,
						lowline(bufbufinfo(markbuffer(&xinf->defaddr)), xinf->to));
				}
				else if (xinf->window && xinf->window->selbottom)
				{
					xinf->defaddr = *xinf->window->selbottom;
				}
			} while (*refp && (**refp == ',' || **refp == ';') && scannext(refp));

			/* "from" can't come after "to" */
			if (xinf->from > xinf->to)
			{
				msg(MSG_ERROR, "bad range");
				return RESULT_ERROR;
			}
		}

		/* if we used visible selection, then unmark it now */
		if (xinf->window && xinf->window->seltop)
		{
			markfree(xinf->window->seltop);
			markfree(xinf->window->selbottom);
			xinf->window->seltop = xinf->window->selbottom = NULL;
		}
	}

	/* parse command name */
	skipwhitespace(refp);
	if (!parsecommandname(refp, xinf))
	{
		return RESULT_ERROR;
	}
	flags = cmdnames[xinf->cmdidx].flags;

	/* is the command legal in this context? */
	if (!win && 0 == (flags & q_Exrc))
	{
		msg(MSG_ERROR, "[s]$1 is illegal during initialization", xinf->cmdname);
		return RESULT_ERROR;
	}
	if (o_safer && 0 != (flags & q_Unsafe))
	{
		msg(MSG_ERROR, "[s]$1 is unsafe", xinf->cmdname);
		return RESULT_ERROR;
	}

	/* if given addresses for a command which doesn't support addresses,
	 * then complain.  If the addresses were due to a visible selection,
	 * then ignore them silently.
	 */
	if (xinf->anyaddr && 0 == (flags & (a_Line|a_Range)))
	{
		if (sel)
		{
			/* visible selection; pretend no addresses were given */
			xinf->anyaddr = False;
		}
		else
		{
			/* explicit addresses; complain */
			msg(MSG_ERROR, "[s]$1 doesn't use addresses", xinf->cmdname);
			return RESULT_ERROR;
		}
	}

	/* if "from" is 0 for a command which doesn't allow 0, then
	 * complain.
	 */
	if (xinf->anyaddr && xinf->from == 0 && 0 == (flags & q_Zero))
	{
		msg(MSG_ERROR, "[s]$1 doesn't allow address 0", xinf->cmdname);
		return RESULT_ERROR;
	}

	/* parse multiplied command names */
	parsemulti(refp, xinf);

	/* If a command allows both a print flag and a buffer name
	 * then parsing them can be tricky because this can lead
	 * to ambiguous situations.  According to POSIX docs,
	 * if there is no space after the command name (:dp) then
	 * the print flag is next; in all other situations, the
	 * cut buffer name comes first.  THEREFORE, if this command
	 * allows both a print flag and a buffer, and we appear to
	 * have a print flag appended to the command name, then
	 * we should parse the print flag now.
	 */
	if ((flags & (a_Pflag|a_Buffer)) == (a_Pflag|a_Buffer)
		&& *refp && (**refp == 'p' || **refp == 'l' || **refp == '#'))
	{
		parseprintflag(refp, xinf, flags);
	}

	/* parse an optional '!' appended to the name */
	parsebang(refp, xinf, flags);

	/* maybe parse a regular expression & replacement text */
	skipwhitespace(refp);
	if (!parseregexp(refp, xinf, flags))
	{
		return RESULT_ERROR;
	}

	/* maybe parse a target buffer and address */
	if (flags & a_Target)
	{
		/* The target allows a window, a buffer, and an
		 * address all to be specified.  Parsing these
		 * could clobber the source range fields, so we'll
		 * copy them into local variables while we parse.
		 */
		rngdef = xinf->defaddr;
		rngfrom = xinf->from;
		rngto = xinf->to;

		/* parse the window id */
		skipwhitespace(refp);
		if (!parsewindowid(refp, xinf))
		{
			return RESULT_ERROR;
		}

		/* parse the buffer name */
		skipwhitespace(refp);
		if (!parsebuffername(refp, xinf))
		{
			return RESULT_ERROR;
		}

		/* parse an address.  Note that we don't allow
		 * looping here, and the address is mandatory
		 * for commands that allow it.
		 */
		skipwhitespace(refp);
		if (*refp && (**refp == '\n' || **refp == '|'))
		{
			msg(MSG_ERROR, "[s]$1 requires destination", xinf->cmdname);
			return RESULT_ERROR;
		}
		if (!exparseaddress(refp, xinf))
		{
			return RESULT_ERROR;
		}

		/* create the "destaddr" mark */
		xinf->destaddr = markalloc(markbuffer(&xinf->defaddr),
			lowline(bufbufinfo(markbuffer(&xinf->defaddr)), xinf->to + 1));

		/* move the range info back where it belongs */
		xinf->defaddr = rngdef;
		xinf->from = rngfrom;
		xinf->to = rngto;
	}

	/* for some commands, parse an LHS and RHS */
	parselhs(refp, xinf, flags);
	skipwhitespace(refp);
	parserhs(refp, xinf, flags);

	/* for some commands, parse an optional cut buffer name,
	 * optional count, optional "+arg", and optional print flag.
	 */
	skipwhitespace(refp);
	parsecutbuffer(refp, xinf, flags);
	skipwhitespace(refp);
	parsecount(refp, xinf, flags);
	skipwhitespace(refp);
	parseplus(refp, xinf, flags);
	skipwhitespace(refp);
	parseprintflag(refp, xinf, flags);

	/* for some commands, parse file arguments.  This includes
	 * filenames, "!cmd" filter commands, ">>" append tokens,
	 * and wildcard expansion.
	 */
	skipwhitespace(refp);
	if (!parsefileargs(refp, xinf, flags))
	{
		return RESULT_ERROR;
	}

	/* for some commands, parse a long argument string which
	 * may include the '|' character but not newline
	 */
	skipwhitespace(refp);
	result = parsecmds(refp, xinf, flags);
	if (result != RESULT_COMPLETE)
	{
		return result;
	}

	/* By this point, there should be no more arguments on this line. */
	skipwhitespace(refp);
	if (*refp && **refp != '|' && **refp != '\n')
	{
		msg(MSG_ERROR, "[s]too many arguments for $1", xinf->cmdname);
		return RESULT_ERROR;
	}

	/* beware of EX-only commands */
	if ((!win || 0 != (win->state->flags & (ELVIS_POP|ELVIS_ONCE|ELVIS_1LINE)))
		&& 0 != (flags & q_Ex)
		&& !xinf->rhs)
	{
		msg(MSG_ERROR, "[s]$1 is illegal in vi mode", xinf->cmdname);
		return RESULT_ERROR;
	}

	/* If the buffer is locked, then complain about edit commands.  Note
	 * that ":s/???/???/x" and ":!cmd" (with no addresses) are specifically
	 * permitted.
	 */
	if (o_locked(markbuffer(&xinf->defaddr))
	 && 0 != (flags & q_Undo)
	 && !(xinf->command == EX_SUBSTITUTE && xinf->rhs && CHARchr(xinf->rhs, 'x'))
	 && !(xinf->command == EX_BANG && !xinf->anyaddr) )
	{
		msg(MSG_ERROR, "[s]buffer $1 is locked", o_bufname(markbuffer(&xinf->defaddr)));
		return RESULT_ERROR;
	}

	/* Maybe parse extra text lines which are arguments to this command */
	if (!xinf->rhs && 0 != (flags & a_Text) && *refp)
	{
		/* skip past the newline on the command line */
		assert(**refp == '\n');
		scannext(refp);

		/* collect characters up to the next line containing only "." */
		for (i = 0;
		     *refp
			&& (**refp != '\n'
			    || (i >= 1 && xinf->rhs[i-1] != '.')
			    || (i >= 2 && xinf->rhs[i-2] != '\n'));
		    i++)
		{
			buildCHAR(&xinf->rhs, **refp);
			scannext(refp);
		}

		/* if all went well, then strip the "." from the end */
		if (*refp)
		{
			assert(i > 2);
			xinf->rhs[i - 1] = '\0';
		}
		else if (markbuffer(&orig) != NULL) /* not scanning string */
		{
			/* end not found, need more text */
			scanseek(refp, &orig);
			drawopencomplete(win);
			return RESULT_MORE;
		}
		assert(!*refp || **refp == '\n');
	}

	/* convert line numbers to marks (if there are any) */
	if ((flags & (a_Line|a_Range)) != 0
		&& (xinf->anyaddr || (flags & d_LnMask) != d_None))
	{
		/* if no lines, set the default */
		if (!xinf->anyaddr && (flags & d_LnMask) == d_All)
		{
			xinf->from = 1;
			xinf->to = o_buflines(markbuffer(&xinf->defaddr));
		}

		/* if only one address was given, and the default range
		 * include should include 2 lines, then add second line.
		 */
		if (!twoaddrs && (flags & d_LnMask) == d_2Lines)
		{
			xinf->to = xinf->from + 1;
		}

		/* if there was a count, add it to "from" to make "to" */
		if (xinf->count > 0)
		{
			xinf->to = xinf->from + xinf->count - 1;
		}

		/* create the "fromaddr" mark -- start of "from" line */
		xinf->fromaddr = markalloc(markbuffer(&xinf->defaddr), 
			lowline(bufbufinfo(markbuffer(&xinf->defaddr)), xinf->from));

		/* create the "toaddr" mark -- end of "to" line.  If that's
		 * the last line, then the computation can be tricky.
		 */
		if (xinf->to == o_buflines(markbuffer(&xinf->defaddr)))
			xinf->toaddr = markalloc(markbuffer(&xinf->defaddr),
					o_bufchars(markbuffer(&xinf->defaddr)));
		else
			xinf->toaddr = markalloc(markbuffer(&xinf->defaddr),
					lowline(bufbufinfo(markbuffer(&xinf->defaddr)), xinf->to + 1));
	}
	else
	{
		/* the cursor won't move */
		xinf->newcurs = (MARK)0;
	}

	/* maybe add the commands to the map log */
	if (o_maplog != 'o'
		&& (logstr || !o_internal(markbuffer(scanmark(refp))) ) )
	{
		log = bufalloc(toCHAR(TRACE_BUF), 0, True);
		if (o_maplog == 'r')
		{
			bufreplace(marktmp(logstart, log, 0L), marktmp(logend, log, o_bufchars(log)), NULL, 0L);
			o_maplog = 'a';
		}
		bufreplace(marktmp(logend, log, o_bufchars(log)), &logend, toCHAR(":"), 1L);
		if (logstr)
		{
			if (*refp)
				loglen = (int)(*refp - logstr);
			else
				loglen = CHARlen(logstr);
			assert(loglen > 0L);
			bufreplace(marktmp(logend, log, o_bufchars(log)),
					&logend, logstr, loglen);
		}
		else
			bufpaste(marktmp(logend, log, o_bufchars(log)),
					&orig, scanmark(refp));
		bufreplace(marktmp(logend, log, o_bufchars(log)), &logend, toCHAR("\n"), 1L);
	}

	/* move the scan point past the command separator */
	if (*refp)
		scannext(refp);

	return RESULT_COMPLETE;
}


/* Execute an ex command, after it has been parsed by the parse() function.
 * Return RESULT_COMPLETE normally, or RESULT_ERROR for errors (after outputting
 * an error message).
 */
static RESULT execute(xinf)
	EXINFO	*xinf;	/* the parsed command to execute */
{
	MARK		pline;	/* line to autoprint */
	BUFFER		custom;	/* the CUSTOM_BUF buffer */
	MARKBUF		top, bottom;
	RESULT		ret;
	BUFFER		origdef;/* original value of bufdefault */
	struct state_s	*state;
	unsigned long	flags;
	WINDOW		found;

	/* If we need to save an "undo" version of the buffer, then
	 * remember that fact.
	 */
	flags = cmdnames[xinf->cmdidx].flags;
	if (xinf->window && xinf->window->cursor && globaldepth == 0)
		bufwilldo(xinf->window->cursor, False);
	if (globaldepth == 0 && (flags & q_Undo))
	{
		if (xinf->destaddr)
			bufwilldo(xinf->destaddr, True);
		else if (xinf->window)
			bufwilldo(xinf->window->cursor, True);
		else if (xinf->toaddr)
			bufwilldo(xinf->toaddr, True);
	}

	/* if global command, then increment globaldepth */
	if (xinf->command == EX_GLOBAL || xinf->command == EX_VGLOBAL)
	{
		globaldepth++;
	}

	/* if quit command, then cause "wrote..." messages to be displayed
	 * as INFO instead of the normal STATUS.  This is so they'll be queued
	 * and can be printed someplace else after the window is closed.
	 */
	if (flags & q_MayQuit)
	{
		bufmsgtype = MSG_INFO;
	}

	/* if the command's default buffer isn't the window's default buffer,
	 * then make the command's default buffer be the one used by :set.
	 */
	origdef = bufdefault;
	if (xinf->window && markbuffer(&xinf->defaddr) !=
		markbuffer(xinf->window->state->pop ? xinf->window->state->pop->cursor : xinf->window->cursor))
	{
		bufoptions(markbuffer(&xinf->defaddr));
	}

	/* Hooray!  Now all we need to do is execute the damn thing */
	ret = (*cmdnames[xinf->cmdidx].fn)(xinf);

	/* restore the options buffer to what it was before */
	bufoptions(origdef);

	/* if global command, then decrement globaldepth */
	if (xinf->command == EX_GLOBAL || xinf->command == EX_VGLOBAL)
	{
		globaldepth--;
	}

	/* if command failed, we're done. */
	if (ret != RESULT_COMPLETE)
	{
		/* !!! Should we call exfree(xinf) here? */
		return RESULT_ERROR;
	}

	/* if the window was deleted, we're done */
	if (xinf->window)
	{
		for (found = winofbuf(NULL, NULL);
		     found && found != xinf->window;
		     found = winofbuf(found, NULL))
		{
		}
		if (!found)
		{
			exfree(xinf);
			return RESULT_COMPLETE;
		}
	}

	/* most commands aren't allowed to switch buffers */
	if (xinf->newcurs
	 && markbuffer(xinf->window->cursor) != markbuffer(xinf->newcurs)
	 && !(flags & q_SwitchB))
	{
		markfree(xinf->newcurs);
		xinf->newcurs = NULL;
	}

	/* move the cursor to where it wants to be */
	if (xinf->newcurs)
	{
		/* find the window's main state */
		for (state = xinf->window->state; state->acton; state = state->acton)
		{
		}

		/* if we're switching buffers, then we need to
		 * switch names, too.
		 */
		if (markbuffer(xinf->window->cursor) != markbuffer(xinf->newcurs))
		{
			optprevfile(o_filename(markbuffer(xinf->window->cursor)),
				    markline(xinf->window->cursor));
			if (gui->retitle)
			{
				(*gui->retitle)(xinf->window->gw, tochar8(o_bufname(markbuffer(xinf->newcurs))));
			}
			marksetbuffer(state->cursor, markbuffer(xinf->newcurs));
			dispset(xinf->window, tochar8(o_bufdisplay(markbuffer(xinf->newcurs))));
			bufoptions(markbuffer(xinf->newcurs));
		}

		/* other stuff is easy */
		marksetoffset(state->cursor, markoffset(xinf->newcurs));
		marksetbuffer(state->top, markbuffer(xinf->newcurs));
		marksetoffset(state->top, markoffset(xinf->newcurs));
		marksetbuffer(state->bottom, markbuffer(xinf->newcurs));
		marksetoffset(state->bottom, markoffset(xinf->newcurs));
		markfree(xinf->newcurs);
		xinf->window->wantcol = state->wantcol = (*xinf->window->md->mark2col)(xinf->window, xinf->window->cursor, True);

		assert(markbuffer(state->cursor) == markbuffer(state->top));
		assert(markbuffer(state->cursor) == markbuffer(state->bottom));
	}

	/* if the new cursor position is off the end of the buffer,
	 * then move it to the start of the last line, instead.
	 */
	if (xinf->window && markoffset(xinf->window->cursor) >= o_bufchars(markbuffer(xinf->window->cursor)))
	{
		marksetoffset(xinf->window->cursor, o_bufchars(markbuffer(xinf->window->cursor)));
		if (markoffset(xinf->window->cursor) > 0L)
		{
			markaddoffset(xinf->window->cursor, -1L);
			marksetoffset(xinf->window->cursor,
				markoffset((*xinf->window->md->move)
					(xinf->window, xinf->window->cursor, 0L, 0L, False)));
		}
	}

	/* print flags and autoprinting happen here */
	if (xinf->pflag != PF_NONE)
	{
		/* Find the start of the line which contains the cursor.
		 * Also add delta.
		 */
		pline = (*xinf->window->md->move)(xinf->window,
				xinf->window->cursor, xinf->delta, 0L, False);

		/* print the line */
		exprintlines(xinf->window, pline, 1, xinf->pflag);
	}

# ifdef FEATURE_MKEXRC
	/* if we're supposed to regenerate CUSTOM_BUF, then do so now */
	if (flags & q_Custom)
	{
		/* find/create the buffer */
		custom = bufalloc(toCHAR(CUSTOM_BUF), 0, True);
		if (o_bufchars(custom) != 0)
		{
			bufreplace(marktmp(top, custom, 0),
				marktmp(bottom, custom, o_bufchars(custom)),
				NULL, 0);
		}

		/* add stuff into it */
		optsave(custom);
		mapsave(custom);
		digsave(custom);
		colorsave(custom);
		/* abbrsave(custom); */
		/* aliassave(custom); */
		if (gui && gui->save)
		{
			(*gui->save)(custom, xinf->window->gw);
		}
	}
# endif /* FEATURE_MKEXRC */

	/* free the data associated with that command */
	exfree(xinf);

	return RESULT_COMPLETE;
}


/* This function parses one or more ex commands, and executes them.   It
 * returns RESULT_COMPLETE if successful, RESULT_ERROR (after printing an
 * error message) if unsuccessful, or RESULT_MORE if the command is
 * incomplete as entered.  As a side-effect, the offset of the "top" mark
 * is moved passed any commands which have been completely parsed and
 * executed.
 *
 * If the "win" argument is NULL, then the parsing uses the parsing style
 * of a ".exrc" file: no window or line numbers are allowed, only commands
 * which have q_Exrc set and q_Unsafe cleared are allowed, the | character
 * is quoted via ^V instead of backslash, and blank lines are ignored (instead
 * of being interpretted as "+p").
 */
RESULT experform(win, top, bottom)
	WINDOW	win;	/* default window (implies default buffer) */
	MARK	top;	/* start of commands */
	MARK	bottom;	/* end of commands */
{
	EXINFO	xinfb;	/* buffer, holds info about command being parsed */
	CHAR	*p;	/* pointer used for scanning command line */
	long	next;	/* where the next command starts */

	/* start reading commands */
	scanalloc(&p, top);

	/* for each command... */
	while (p && markoffset(top) < markoffset(bottom))
	{
		/* parse an ex command */
		switch (parse(win, &p, &xinfb))
		{
		  case RESULT_ERROR:
			goto Fail;

		  case RESULT_MORE:
			goto More;

		  case RESULT_COMPLETE:
			; /* continue processing */
		}

		/* Suspend the scanning while we execute this command.
		 * We aren't allowed to change text while scanning.
		 */
		next = (p ? markoffset(scanmark(&p)) : markoffset(bottom));
		scanfree(&p);

		/* execute the command */
		if (execute(&xinfb) != RESULT_COMPLETE)
		{
			goto Fail2;
		}

		/* adjust "top" to point after the command, and resume scan */
		marksetoffset(top, next);
		scanalloc(&p, top);
	}
	scanfree(&p);
	return RESULT_COMPLETE;

Fail:
	scanfree(&p);
Fail2:
	exfree(&xinfb);
	return RESULT_ERROR;

More:
	scanfree(&p);
	exfree(&xinfb);
	return RESULT_MORE;
}

/* This function resembles experform(), except that this function parses
 * from a string instead of from a buffer.
 */
RESULT exstring(win, str)
	WINDOW	win;	/* default window (implies default buffer) */
	CHAR	*str;	/* the string containing ex commands */
{
	EXINFO	xinfb;	/* buffer, holds info about command being parsed */
	CHAR	*p;	/* pointer used for scanning command line */
	BOOLEAN	oldthenflag;
	CHAR	*olddotest;
	RESULT	result;
	void	*locals;

	/* commands run this way don't affect the :if or :while expressions */
	oldthenflag = exthenflag;
	olddotest = exdotest;
	exdotest = NULL;
	locals = optlocal(NULL);

	/* start reading commands */
	scanstring(&p, str);

	/* for each command... */
	while (p && *p)
	{
		/* parse and execute one ex command */
		if (parse(win, &p, &xinfb) != RESULT_COMPLETE
		 || execute(&xinfb) != RESULT_COMPLETE)
		{
			goto Fail;
		}
	}
	scanfree(&p);
	result = RESULT_COMPLETE;
	goto Done;

Fail:
	scanfree(&p);
	exfree(&xinfb);
	result = RESULT_ERROR;
	/* and fall through... */

Done:
	(void)optlocal(locals);
	exthenflag = oldthenflag;
	exdotest = olddotest;
	return result;
}



/* This function checks to see whether a given string is an acceptable
 * abbreviation for a command name.  If so, it returns the full command name;
 * if not, it returns NULL
 */
CHAR *exname(name)
	CHAR	*name;	/* possible name of an ex command */
{
	int	i, len;

	/* non-alphabetic names get special treatment */
	len = CHARlen(name);
	if (len == 1 && !isalpha(*name))
	{
		switch (*name)
		{
		  case '!':	return toCHAR("BANG");
		  case '"':	return toCHAR("QUOTE");
		  case '#':	return toCHAR("HASH");
		  case '<':	return toCHAR("LT");
		  case '=':	return toCHAR("EQ");
		  case '>':	return toCHAR("GT");
		  case '&':	return toCHAR("AMP");
		  case '~':	return toCHAR("TILDE");
		  case '@':	return toCHAR("AT");
		  case '(':	return toCHAR("OPEN"); /* not a real command */
		  case '{':	return toCHAR("OCUR"); /* not a real command */
		}
	}

	/* else look up the name in the cmdnames[] array */
	for (i = 0;
	     i < QTY(cmdnames) && strncmp(cmdnames[i].name, tochar8(name), (size_t)len);
	     i++)
	{
	}
	if (i < QTY(cmdnames))
	{
		return toCHAR(cmdnames[i].name);
	}
	return NULL;
}


/* This function is called when the user hits <Enter> after entering an
 * ex command line.  It returns RESULT_COMPLETE if successful, RESULT_ERROR
 * (after printing an error message) if unsuccessful, or RESULT_MORE if
 * the command is incomplete as entered.  As a side-effect, the offset of
 * the "top" mark is moved passed any commands which have been completely
 * parsed and executed.
 */
RESULT exenter(win)
	WINDOW	win;	/* window where an ex command has been entered */
{
	STATE	*state = win->state;

	return experform(win, state->top, state->bottom);
}


/* This function prints single line as ex output text.  If "number" is true,
 * it will precede each line with a line number.  If "list" is true, it will
 * make all characters visible (including tab) and show a '$' at the end of
 * each line.  Returns the offset of the last line output.
 */
long exprintlines(win, mark, qty, pflag)
	WINDOW	win;	/* window to write to */
	MARK	mark;	/* start of line to output */
	long	qty;	/* number of lines to print */
	PFLAG	pflag;	/* controls how the line is printed */
{
	CHAR	tmp[24];/* temp strings */
	CHAR	*scan;	/* used for scanning */
	long	last;	/* offset of start of last line */
	long	lnum;	/* line number */
	long	col;	/* output column number */
	long	tabstop;/* size of tabs */
	BOOLEAN	number;	/* show line number? */
	BOOLEAN	list;	/* show all characters? */
	long	i;

	/* initialize "last" just to silence a compiler warning */
	last = 0;

	/* figure out how we'll show the lines */
	if (pflag == PF_NONE)
	{
		return markoffset(mark);
	}
	number = (BOOLEAN)(pflag == PF_NUMBER || pflag == PF_NUMLIST);
	list = (BOOLEAN)(pflag == PF_LIST || pflag == PF_NUMLIST);

	/* If we'll be showing line numbers, then find the first one now */
	if (number)
	{
		(void)lowoffset(bufbufinfo(markbuffer(mark)), markoffset(mark),
			(COUNT *)0, (COUNT *)0, (LBLKNO *)0, &lnum);
	}

	/* compute tab size */
	tabstop = o_tabstop(markbuffer(mark));

	/* for each line... */
	for (scanalloc(&scan, mark); scan && qty > 0 && !guipoll(False); scannext(&scan), qty--)
	{
		/* remember where this line started */
		last = markoffset(scanmark(&scan));

		/* output the line number, if we're supposed to */
		if (number)
		{
			memset(tmp, ' ', QTY(tmp));
			long2CHAR(tmp + 8, lnum);
			CHARcat(tmp, toCHAR("  "));
			drawextext(win, tmp + CHARlen(tmp) - 8, 8);
			lnum++;
		}

		/* scan the line and output each character */
		col = 0;
		for (; scan && *scan != '\n'; scannext(&scan))
		{
			if (*scan == '\t' && !list)
			{
				/* expand tab into a bunch of spaces */
				i = tabstop - (col % tabstop);
				col += i;
				memset(tmp, ' ', QTY(tmp));
				while (i > QTY(tmp))
				{
					drawextext(win, tmp, QTY(tmp));
					i -= QTY(tmp);
				}
				if (i > 0)
				{
					drawextext(win, tmp, (int)i);
				}
			}
			else if (*scan < ' ' || *scan == '\177')
			{
				tmp[0] = '^';
				tmp[1] = ELVCTRL(*scan);
				drawextext(win, tmp, 2);
				col += 2;
			}
			else if (*scan > '\177' && list)
			{
				sprintf((char *)tmp, "\\x%02x", *scan);
				i = CHARlen(tmp);
				drawextext(win, tmp, (int)i);
				col += i;
			}
			else
			{
				/* count consecutive normal chars */
				for (i = 1; i < scanright(&scan) && scan[i] >= ' ' && scan[i] < '\177'; i++)
				{
				}
				drawextext(win, scan, (int)i);
				col += i;
				scan += i - 1; /* plus one more @ top of loop */
			}
		}

		/* for "list", append a '$' */
		if (list)
		{
			tmp[0] = '$';
			tmp[1] = '\n';
			drawextext(win, tmp, 2);
		}
		else
		{
			tmp[0] = '\n';
			drawextext(win, tmp, 1);
		}
	}
	scanfree(&scan);
	return last;
}


#ifdef FEATURE_COMPLETE
CHAR *excomplete(win, from, to)
	WINDOW	win;	/* window where multiple matches are listed */
	MARK	from;	/* start of the line where completion is needed */
	MARK	to;	/* position of the cursor within that line */
{
	CHAR	*s;
	long	offset;
 static CHAR	tab[2] = "\t";
 static CHAR	common[10];/* long enough for any ex command name */
 	MARKBUF	tmp;
	CHAR	*cmdname;
	int	len, mlen;
	int	i, j, match;

	(void)scanalloc(&s, from);
	offset = markoffset(from);

	/* skip leading ':' characters and whitespace */
	while (*s && offset < markoffset(to) && (*s == ':' || isspace(*s)))
	{
		scannext(&s);
		offset++;
	}

	/* if '(' then skip forward for matching ')'.  If we hit the cursor
	 * before that, then we want to perform filename completion (really
	 * buffername completion)
	 */
	if (*s == '(')
	{
		while (*s && offset < markoffset(to) && *s != ')')
		{
			scannext(&s);
			offset++;
		}
		if (offset == markoffset(to))
		{
			scanfree(&s);
			return NULL;
		}
	}

	/* if at start of line, or next char isn't a command, then always
	 * use a literal tab.  If at start of a ! command, then do filename
	 * completion.
	 */
	if (!s || offset >= markoffset(to) || !isalpha(*s))
	{
		if (s && offset < markoffset(to) && *s == '!')
			cmdname = NULL;
		else
			cmdname = tab;
		scanfree(&s);
		return cmdname;
	}

	/* collect the characters of the command name */
	cmdname = NULL;
	do
	{
		len = buildCHAR(&cmdname, *s);
		offset++;
	} while (scannext(&s) && offset < markoffset(to) && isalpha(*s));
	scanfree(&s);

	/* if cursor is at end of command name, then try to expand the name.
	 * This doesn't know about aliases!
	 */
	if (offset == markoffset(to))
	{
		/* count the matches */
		for (i = j = 0; i < QTY(cmdnames); i++)
			if (!CHARncmp(cmdname, toCHAR(cmdnames[i].name), len))
			{
				if (j == 0)
				{
					match = i;
					mlen = strlen(cmdnames[match].name);
				}
				else
				{
					while (strncmp(cmdnames[i].name, cmdnames[match].name, mlen))
						mlen--;
				}
				j++;
			}

		/* don't need our copy of the partial name anymore */
		safefree(cmdname);

		/* if no matches, then do a literal tab */
		if (j == 0)
			return tab;

		/* If one match, then return the untyped portion of it followed
		 * by a space.  The user may want to back up over the space
		 * to append a ! but the space is still nice because it
		 * reassures the user that the name really is complete.
		 */
		if (j == 1)
		{
			for (i = 0, j = len; cmdnames[match].name[j]; i++, j++)
				common[i] = cmdnames[match].name[j];
			common[i++] = ' ';
			common[i] = '\0';
			return common;
		}

		/* Else multiple matches.  Can we add to their common chars?
		 * If so, great!  If not, then list all matches.
		 */
		for (i = 0, j = len; j < mlen; j++, i++)
			common[i] = cmdnames[match].name[j];
		common[i] = '\0';
		if (mlen == len)
		{
			for (i = j = 0; i < QTY(cmdnames); i++)
				if (!strncmp(cmdnames[match].name, cmdnames[i].name, len))
				{
					mlen = strlen(cmdnames[i].name);
					if (j + mlen + 1 >= o_columns(win))
					{
						drawextext(win, toCHAR("\n"), 1);
						j = 0;
					}
					else if (j != 0)
						drawextext(win, blanks, 1);
					drawextext(win,
						toCHAR(cmdnames[i].name),
						mlen);
					j += mlen + 1;
				}
			if (j != 0)
				drawextext(win, toCHAR("\n"), 1);
		}
		return common;
	}

	/* if we get here, then we're completing the args of a command,
	 * instead of the command itself.
	 */

#ifdef FEATURE_ALIAS
	/* macros probably take filenames for args */
	if (exisalias(tochar8(cmdname), False))
	{
		safefree(cmdname);
		return NULL;
	}
#endif

	/* look up the command */
	for (i = 0; i < QTY(cmdnames) && CHARncmp(cmdname, toCHAR(cmdnames[i].name), len); i++)
	{
	}
	safefree(cmdname);
	if (i >= QTY(cmdnames))
	{
		/* Unknown command.  This is probably either a typo (in which
		 * case the behavior of <Tab> isn't that critical) or it is
		 * really a shell command via the recursive excomplete() call,
		 * below.  In the latter case, filename completion is the
		 * best action, so we do that.
		 */
		return NULL;
	}

	/* the :set command completes option names */
	if (cmdnames[i].code == EX_SET)
		return optcomplete(win, to);

	/* The :tag and :stag commands complete tag tags.  I'm tempted to
	 * add :browse and :sbrowse here, but I suspect that filename
	 * completion might be more useful for them.
	 */
	if (cmdnames[i].code == EX_TAG || cmdnames[i].code == EX_STAG)
		return tagcomplete(win, to);

	/* Many commands take ex commands as their arguments.  The a_Cmds
	 * flag is set for these commands, but it is also set for a few
	 * others.  Oh well, it's close enough.  Also, the :help command's
	 * arguments resemble an ex command.
	 */
	if ((cmdnames[i].flags & a_Cmds) != 0 || cmdnames[i].code == EX_HELP)
		return  excomplete(win, marktmp(tmp, markbuffer(to), offset), to);

	/* does the command take filename arguments or a target address? */
	if ((cmdnames[i].flags & (a_Target | a_Filter | a_File | a_Files)) != 0)
		return NULL;/* do filename expansion */

	/* if all else fails, just treat it literally */
	return tab;
}
#endif /* FEATURE_COMPLETE */
