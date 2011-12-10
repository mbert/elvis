/* vi.c */
/* Copyright 1995 by Steve Kirkendall */

char id_vi[] = "$Id: vi.c,v 2.52 1996/09/21 00:27:55 steve Exp $";

#include "elvis.h"

#if USE_PROTOTYPES
static RESULT parse(_CHAR_ key, void *info);
static ELVCURSOR shape(WINDOW win);
#endif

#define FIX_EMPTY	/* when command would fail on empty buffer: define=fix buffer */

/* This variable stores the command which <.> is supposed to repeat.
 * If the command is supposed to switch to input mode, then those
 * commands will be smart enough to behave differently when executed
 * for the <.> command.  They'll paste from the "Elvis previous input"
 * buffer.
 */
VIINFO	dotcmd;
BOOLEAN	dotviz;	/* was the dotcmd a visible operator? */
long	dotlines;/* number of lines for visible operator, or 0L if char/rect */

/* These are the "when" conditions in the array below */
#define WHEN_SEL	0x0001	/* legal while text is selected */
#define WHEN_ONCE	0x0002	/* legal during ^O */
#define WHEN_OPEN	0x0004	/* legal in "open" mode */
#define WHEN_HIST	0x0008	/* legal in "history" buffers */
#define WHEN_MOVE	0x0010	/* legal as target of an operator */
#define WHEN_ANY	0x001f	/* all of the above */
#define WHEN_NEVER	0x0000	/* none of the above */
#define WHEN_MORE	0x0020	/* legal as part of a "more" invocation */
#define WHEN_EMPTY	0x0040	/* legal when buffer is empty */

#define WHEN_SEL_ONCE		(WHEN_SEL|WHEN_ONCE)
#define WHEN_ONCE_OPEN		(WHEN_ONCE|WHEN_OPEN)
#define WHEN_ONCE_OPEN_HIST	(WHEN_ONCE|WHEN_OPEN|WHEN_HIST)
#define WHEN_SEL_ONCE_OPEN_HIST	(WHEN_SEL|WHEN_ONCE|WHEN_OPEN|WHEN_HIST)
#define WHEN_SEL_ONCE_OPEN	(WHEN_SEL|WHEN_ONCE|WHEN_OPEN)
#define WHEN_SEL_ONCE_MOVE	(WHEN_SEL|WHEN_ONCE|WHEN_MOVE)
#define WHEN_SEL_ONCE_OPEN_MOVE	(WHEN_SEL|WHEN_ONCE|WHEN_OPEN|WHEN_MOVE)
#define WHEN_SEL_ONCE_HIST_MOVE	(WHEN_SEL|WHEN_ONCE|WHEN_HIST|WHEN_MOVE)
#define WHEN_OPEN_HIST		(WHEN_OPEN|WHEN_HIST)

/* This array describes each command */
static struct
{
	RESULT		(*func) P_((WINDOW win, VIINFO *vinf));	/* performs command */
	unsigned short	when;					/* when command is legal */
	unsigned short	tweak;					/* misc features */
	char		*helptopic;				/* label in "elvisvi.html" */
}
vikeys[] =
{
/* NUL not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE,		"nul"	},
/* ^A  search for word	 */	{ m_search,	WHEN_ANY,			TWEAK_MARK,		"^A"	},
/* ^B  page backward	 */	{ m_scroll,	WHEN_SEL_ONCE,			TWEAK_FRONT,		"^B"	},
/* ^C  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE,		"^C"	},
/* ^D  scroll dn 1/2page */	{ m_scroll,	WHEN_SEL_ONCE_OPEN,		TWEAK_FIXCOL,		"^D"	},
/* ^E  scroll up	 */	{ m_scroll,	WHEN_SEL_ONCE,			TWEAK_FIXCOL,		"^E"	},
/* ^F  page forward	 */	{ m_scroll,	WHEN_SEL_ONCE,			TWEAK_FRONT,		"^F"	},
/* ^G  show file status	 */	{ m_absolute,	WHEN_SEL_ONCE_OPEN|WHEN_EMPTY,	TWEAK_NONE,		"^G"	},
/* ^H  move left, like h */	{ m_left,	WHEN_ANY,			TWEAK_NONE,		"^H"	},
/* ^I  move to next tag  */	{ m_tag,	WHEN_ONCE_OPEN,			TWEAK_NONE,		"^I"	},
/* ^J  move down	 */	{ m_updown,	WHEN_ANY,			TWEAK_IGNCOL_INCL_LINE,	"^J"	},
/* ^K  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE,		"^K"	},
/* ^L  redraw screen	 */	{ v_expose,	WHEN_SEL_ONCE|WHEN_EMPTY,	TWEAK_NONE,		"^L"	},
/* ^M  mv front next ln  */	{ m_updown,	WHEN_ANY,			TWEAK_FRONT_INCL_LINE,	"^M"	},
/* ^N  move down	 */	{ m_updown,	WHEN_ANY,			TWEAK_IGNCOL_INCL_LINE,	"^N"	},
/* ^O  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE,		"^O"	},
/* ^P  move up		 */	{ m_updown,	WHEN_ANY,			TWEAK_IGNCOL_INCL_LINE,	"^P"	},
/* ^Q  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE,		"^Q"	},
/* ^R  redo change	 */	{ v_undo,	WHEN_ONCE_OPEN|WHEN_EMPTY,	TWEAK_NONE,		"^R"	},
/* ^S  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE,		"^S"	},
/* ^T  pop tagstack	 */	{ v_tag,	WHEN_OPEN|WHEN_EMPTY,		TWEAK_NONE,		"^T"	},
/* ^U  scroll up 1/2page */	{ m_scroll,	WHEN_SEL_ONCE_OPEN,		TWEAK_FIXCOL,		"^U"	},
/* ^V  start visible	 */	{ v_visible,	WHEN_SEL_ONCE,			TWEAK_LINE,		"^V"	},
/* ^W  window operations */	{ v_window,	WHEN_SEL_ONCE_OPEN_HIST|WHEN_EMPTY,TWEAK_NONE,		"^W"	},
/* ^X  move to phys col	 */	{ m_column,	WHEN_SEL_ONCE_OPEN_HIST|WHEN_MOVE,TWEAK_IGNCOL_MARK,	"^X"	},
/* ^Y  scroll down	 */	{ m_scroll,	WHEN_SEL_ONCE,			TWEAK_FIXCOL,		"^Y"	},
/* ^Z  not defined	 */	{ v_notex,	WHEN_ANY,			TWEAK_NONE,		"^Z"	},
/* ESC end visible mark	 */	{ v_visible,	WHEN_SEL,			TWEAK_NONE,		"^obra"	},
/* ^\  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE,		"^bksl"	},
/* ^]  keyword is tag	 */	{ v_tag,	WHEN_ONCE_OPEN,			TWEAK_NONE,		"^cbra"	},
/* ^^  previous file	 */	{ v_notex,	WHEN_ONCE_OPEN|WHEN_EMPTY,		TWEAK_FRONT,		"^^"	},
/* ^_  move to row	 */	{ NULL,		WHEN_SEL_ONCE,			TWEAK_FIXCOL_INCL,	"^_"	},
/* SPC move right,like l */	{ m_right,	WHEN_ANY,			TWEAK_NONE,		"l"	},
/*  !  run thru filter	 */	{ NULL,		WHEN_NEVER,			TWEAK_FRONT_LINE_DOT_OPER_UNDO,	"bang"	},
/*  "  select cut buffer */	{ NULL,		0,/* This is handled by the command parser */0,		"quote"	},
/*  #  increment number	 */	{ v_number,	WHEN_ONCE_OPEN_HIST,		TWEAK_DOT_UNDO,		"hash"	},
/*  $  move to rear	 */	{ m_column,	WHEN_ANY,			TWEAK_IGNCOL_INCL,	"dollar"},
/*  %  move to match	 */	{ m_absolute,	WHEN_ANY,			TWEAK_INCL,		"pct"	},
/*  &  repeat subst	 */	{ v_notex,	WHEN_NEVER,			TWEAK_FIXCOL_INCL_LINE_DOT_UNDO, "amp"},
/*  '  move to a mark	 */	{ m_mark,	WHEN_SEL_ONCE_OPEN_MOVE,	TWEAK_FRONT_INCL_MARK_LINE, "apost"},
/*  (  mv back sentence	 */	{ m_bsentence,	WHEN_ANY,			TWEAK_NONE,		"open"	},
/*  )  mv fwd sentence	 */	{ m_fsentence,	WHEN_ANY,			TWEAK_NONE,		"close"	},
/*  *  errlist		 */	{ v_notex,	WHEN_OPEN|WHEN_EMPTY,		TWEAK_FRONT,			},
/*  +  mv front next ln  */	{ m_updown,	WHEN_ANY,			TWEAK_FRONT_INCL_LINE,		},
/*  ,  reverse [fFtT] cmd*/	{ m_csearch,	WHEN_ANY,			TWEAK_INCL,		"comma"	},
/*  -  mv front prev ln	 */	{ m_updown,	WHEN_ANY,			TWEAK_FRONT_INCL_LINE		},
/*  .  special...	 */	{ NULL,		0,/* This is handled by the command parser */0,		"stop"	},
/*  /  forward search	 */	{ m_search,	WHEN_ANY|WHEN_MORE,		TWEAK_MARK,		"slash"	},
/*  0  part of count?	 */	{ m_column,	WHEN_ANY,			TWEAK_IGNCOL_MARK		},
/*  1  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  2  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  3  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  4  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  5  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  6  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  7  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  8  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  9  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  :  run single EX cmd */	{ v_ex,		WHEN_SEL_ONCE_OPEN_HIST|WHEN_EMPTY, TWEAK_NONE,		"colon"	},
/*  ;  repeat [fFtT] cmd */	{ m_csearch,	WHEN_ANY,			TWEAK_INCL,		"semi"	},
/*  <  shift text left	 */	{ NULL,		WHEN_OPEN,			TWEAK_FRONT_LINE_DOT_OPER_UNDO,"lt"},
/*  =  preset filter	 */	{ NULL,		WHEN_OPEN,			TWEAK_DOT_OPER_UNDO		},
/*  >  shift text right	 */	{ NULL,		WHEN_OPEN,			TWEAK_FRONT_LINE_DOT_OPER_UNDO,"gt"},
/*  ?  backward search	 */	{ m_search,	WHEN_ANY|WHEN_MORE,		TWEAK_MARK,		"quest"	},
/*  @  execute a cutbuf  */	{ v_at,		WHEN_SEL_ONCE_OPEN_HIST|WHEN_EMPTY,TWEAK_NONE,		"at"	},
/*  A  append at EOL	 */	{ v_input,	WHEN_ONCE_OPEN_HIST|WHEN_EMPTY|WHEN_MORE, TWEAK_DOT_UNDO	},
/*  B  move back Word	 */	{ m_bigword,	WHEN_ANY,			TWEAK_NONE			},
/*  C  change to EOL	 */	{ v_notop,	WHEN_OPEN,			TWEAK_DOT_UNDO			},
/*  D  delete to EOL	 */	{ v_notop,	WHEN_OPEN,			TWEAK_DOT_UNDO			},
/*  E  move end of Word	 */	{ m_bigword,	WHEN_ANY,			TWEAK_INCL			},
/*  F  move bk to char	 */	{ m_csearch,	WHEN_ANY,			TWEAK_INCL			},
/*  G  move to line #	 */	{ m_absolute,	WHEN_ANY,			TWEAK_FRONT_INCL_MARK_LINE	},
/*  H  move to row	 */	{ m_scrnrel,	WHEN_SEL_ONCE_MOVE,		TWEAK_FRONT_INCL_LINE		},
/*  I  insert at front	 */	{ v_input,	WHEN_ONCE_OPEN_HIST|WHEN_EMPTY|WHEN_MORE, TWEAK_DOT_UNDO	},
/*  J  join lines	 */	{ v_notex,	WHEN_OPEN,			TWEAK_DOT_UNDO			},
/*  K  look up keyword	 */	{ v_notex,	WHEN_OPEN,			TWEAK_NONE			},
/*  L  move to last row	 */	{ m_scrnrel,	WHEN_SEL_ONCE_MOVE,		TWEAK_FRONT_INCL_LINE		},
/*  M  move to mid row	 */	{ m_scrnrel,	WHEN_SEL_ONCE_MOVE,		TWEAK_FRONT_INCL_LINE		},
/*  N  reverse prev srch */	{ m_search,	WHEN_ANY,			TWEAK_MARK			},
/*  O  insert above line */	{ v_input,	WHEN_ONCE_OPEN_HIST|WHEN_EMPTY|WHEN_MORE, TWEAK_DOT_UNDO	},
/*  P  paste before	 */	{ v_paste,	WHEN_ONCE_OPEN_HIST|WHEN_EMPTY,	TWEAK_DOT_UNDO			},
/*  Q  quit to EX mode	 */	{ v_ex,		WHEN_OPEN|WHEN_EMPTY,		TWEAK_NONE			},
/*  R  overtype		 */	{ v_input,	WHEN_ONCE_OPEN_HIST|WHEN_MORE,	TWEAK_DOT_UNDO			},
/*  S  change line	 */	{ v_notop,	WHEN_OPEN,			TWEAK_DOT_UNDO			},
/*  T  move bk to char	 */	{ m_csearch,	WHEN_ANY,			TWEAK_INCL			},
/*  U  undo whole line	 */	{ v_undo,	WHEN_OPEN_HIST|WHEN_EMPTY,	TWEAK_FRONT_UNDO		},
/*  V  start visible	 */	{ v_visible,	WHEN_SEL_ONCE,			TWEAK_LINE			},
/*  W  move forward Word */	{ m_bigword,	WHEN_ANY,			TWEAK_NONE			},
/*  X  delete to left	 */	{ v_delchar,	WHEN_ONCE_OPEN_HIST,		TWEAK_DOT_UNDO			},
/*  Y  yank text	 */	{ v_notop,	WHEN_OPEN,			TWEAK_NONE			},
/*  Z  save file & exit	 */	{ v_quit,	WHEN_ONCE_OPEN|WHEN_EMPTY,	TWEAK_NONE			},
/*  [  move back section */	{ m_bsection,	WHEN_ANY,			TWEAK_LINE_MARK,	"obra"	},
/*  \  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE,		"bksl"	},
/*  ]  move fwd section  */	{ m_fsection,	WHEN_ANY,			TWEAK_LINE_MARK,	"cbra"	},
/*  ^  move to front	 */	{ m_front,	WHEN_ANY,			TWEAK_NONE			},
/*  _  current line	 */	{ m_updown,	WHEN_ANY,			TWEAK_FIXCOL_INCL_LINE		},
/*  `  move to mark	 */	{ m_mark,	WHEN_SEL_ONCE_OPEN_MOVE,	TWEAK_MARK,		"grave"	},
/*  a  append at cursor	 */	{ v_input,	WHEN_OPEN_HIST|WHEN_EMPTY|WHEN_MORE, TWEAK_DOT_UNDO		},
/*  b  move back word	 */	{ m_word,	WHEN_ANY,			TWEAK_NONE			},
/*  c  change text	 */	{ NULL,		WHEN_OPEN,			TWEAK_DOT_OPER_UNDO		},
/*  d  delete op	 */	{ NULL,		WHEN_OPEN,			TWEAK_DOT_OPER_UNDO		},
/*  e  move end word	 */	{ m_word,	WHEN_ANY,			TWEAK_INCL			},
/*  f  move fwd for char */	{ m_csearch,	WHEN_ANY,			TWEAK_INCL			},
/*  g  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/*  h  move left	 */	{ m_left,	WHEN_ANY,			TWEAK_NONE			},
/*  i  insert at cursor	 */	{ v_input,	WHEN_ONCE_OPEN_HIST|WHEN_EMPTY|WHEN_MORE, TWEAK_DOT_UNDO|TWEAK_IGNCOL	},
/*  j  move down	 */	{ m_updown,	WHEN_ANY,			TWEAK_IGNCOL_INCL_LINE		},
/*  k  move up		 */	{ m_updown,	WHEN_ANY,			TWEAK_IGNCOL_INCL_LINE		},
/*  l  move right	 */	{ m_right,	WHEN_ANY,			TWEAK_NONE			},
/*  m  define a mark	 */	{ v_setmark,	WHEN_ONCE_OPEN|WHEN_OPEN,	TWEAK_NONE			},
/*  n  repeat prev srch	 */	{ m_search,	WHEN_ANY,			TWEAK_MARK			},
/*  o  insert below line */	{ v_input,	WHEN_OPEN_HIST|WHEN_EMPTY|WHEN_MORE, TWEAK_DOT_UNDO		},
/*  p  paste after	 */	{ v_paste,	WHEN_ONCE_OPEN_HIST,		TWEAK_DOT_UNDO			},
/*  q  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/*  r  replace chars	 */	{ v_delchar,	WHEN_ONCE_OPEN_HIST,		TWEAK_DOT_UNDO			},
/*  s  subst N chars	 */	{ v_notop,	WHEN_OPEN,			TWEAK_DOT_UNDO			},
/*  t  move fwd to char	 */	{ m_csearch,	WHEN_ANY,			TWEAK_INCL			},
/*  u  undo		 */	{ v_undo,	WHEN_ONCE_OPEN|WHEN_EMPTY,	TWEAK_NONE			},
/*  v  start visible	 */	{ v_visible,	WHEN_SEL_ONCE,			TWEAK_NONE			},
/*  w  move fwd word	 */	{ m_word,	WHEN_ANY,			TWEAK_NONE			},
/*  x  delete character	 */	{ v_delchar,	WHEN_ONCE_OPEN_HIST,		TWEAK_DOT_UNDO			},
/*  y  yank text	 */	{ NULL,		WHEN_OPEN,			TWEAK_OPER			},
/*  z  adjust scrn row	 */	{ m_z,		WHEN_SEL_ONCE,			TWEAK_FIXCOL			},
/*  {  back paragraph	 */	{ m_bsection,	WHEN_ANY,			TWEAK_NONE,		"ocur"	},
/*  |  move to column	 */	{ m_column,	WHEN_ANY,			TWEAK_IGNCOL_MARK,	"bar"	},
/*  }  fwd paragraph	 */	{ m_fsection,	WHEN_ANY,			TWEAK_NONE,		"ccur"	},
/*  ~  upper/lowercase	 */	{ v_delchar,	WHEN_ONCE_OPEN_HIST,		TWEAK_DOT_UNDO			},
/* DEL not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			}
};



/* This initializes the "info" field to represent the start of a new command */
void viinitcmd(info)
	VIINFO	*info;	/* vi command to be blanked */
{
	info->phase = VI_START;
	info->count = info->count2 = 0;
	info->cutbuf = info->oper = info->command = info->key2 = '\0';
}


/* This performs a single vi command for a given window.  The vi command
 * is stored in the current state.
 */
RESULT _viperform(win)
	WINDOW	win;	/* window whose command is to be executed */
{
	return viperform(win, (VIINFO *)win->state->info);
}

/* This performs a single vi command, and returns RESULT_COMPLETE if
 * successful, RESULT_ERROR if not error, or RESULT_MORE if it has pushed
 * a new state and can't be resolved until that mode is popped.
 */
RESULT	viperform(win, vinf)
	WINDOW	win;	/* window where the command should be performed */
	VIINFO	*vinf;	/* the command itself */
{
	STATE	*state = win->state;
	RESULT	result = RESULT_COMPLETE;
	MARKBUF	from, to;
	unsigned short flags;
	long	tmp;

	/* If command is ^M and this display mode has a tagnext() function,
	 * then pretend this is a ^] command (which calls tagatcursor() ).
	 * This makes hypertext easier to use.
	 */
	if (win->md->tagnext && vinf->command == '\r')
	{
		vinf->command = ELVCTRL(']');
	}

	/* if the command is '.' then recall the previous repeatable command */
	vinf->tweak = vikeys[vinf->command].tweak;
	if (vinf->command == '.')
	{
		/* change the count, if a new count was supplied */
		if (vinf->count)
		{
			dotcmd.count = vinf->count;
		}

		/* recall the command */
		*vinf = dotcmd;

		/* If it was a visible operator the first time, then it must
		 * be visible now too.  Exception: if the earlier command was
		 * applied to whole lines, then reapply it now to the same
		 * quantity of whole lines.
		 */
		if (dotviz && !win->seltop)
		{
			if (dotlines)
			{
				vinf->count = dotlines;
			}
			else
			{
				return RESULT_ERROR;
			}
		}
	}

	/* see if the command is legal in this context */
	flags = vikeys[vinf->command].when;
	if ((win->seltop && !(flags & WHEN_SEL))
	 || ((state->flags & (ELVIS_ONCE | ELVIS_POP)) && !(flags & WHEN_ONCE))
	 || ((state->flags & ELVIS_BOTTOM) && !(flags & WHEN_OPEN))
	 || (state->acton && !(flags & WHEN_HIST))
	 || (vinf->oper && !(flags & WHEN_MOVE))
#ifndef FIX_EMPTY
	 || (o_bufchars(markbuffer(state->cursor)) == 0 && !(flags & WHEN_EMPTY))
#endif
	 || vikeys[vinf->command].func == NULL)
	{
		viinitcmd(vinf);
		return RESULT_ERROR;
	}

#ifdef FIX_EMPTY
	/* if the buffer is empty, and this command doesn't work on an empty
	 * buffer, then stuff a newline into the buffer.
	 */
	if (o_bufchars(markbuffer(state->cursor)) == 0 && !(flags & WHEN_EMPTY))
	{
		/* bufwilldo(state->cursor); */
		bufreplace(marktmp(from, markbuffer(state->cursor), 0L), &from, toCHAR("\n"), 1L);
		o_modified(markbuffer(state->cursor)) = False;
	}
#endif

	/* If we're using an operator, then combine the operator's flags
	 * with the movement command's flags.
	 */
	if (vinf->oper)
	{
		vinf->tweak |= vikeys[vinf->oper].tweak;
	}

	/* If the command doesn't accept two separate counts, and we got two,
	 * then multiply them and store the result as the count.
	 */
	if ((vinf->tweak & TWEAK_TWONUM) == 0 && vinf->count2 != 0)
	{
		if (vinf->count)
		{
			vinf->count *= vinf->count2;
		}
		else
		{
			vinf->count = vinf->count2;
		}
	}

	/* if TWEAK_UNDO, then set the buffer's "willdo" flag.  This will cause
	 * an "undo" version of the buffer to be created if the command does
	 * indeed modify the buffer.  Exception: If this is the second phase
	 * of a complex command, then we don't want to set the flag because
	 * we already set it in the first phase.
	 */
	if ((vinf->tweak & TWEAK_UNDO) != 0 && (win->state->flags & ELVIS_MORE) == 0)
	{
		bufwilldo(state->cursor);
	}

	/* unless we're applying an operator to a visible selection... */
	if (!vinf->oper || !win->seltop)
	{
		/* save the cursor position in a local buffer, in case we need
		 * to do a TWEAK_MARK after calling the function, or if this
		 * command will be used as the target of an operator.
		 */
		from = *state->cursor;

		/* if TWEAK_DOT, on the main buffer, then remember command. */
		if ((vinf->tweak & TWEAK_DOT) != 0 && state->acton == NULL)
		{
			dotcmd = *vinf;
			dotcmd.tweak |= TWEAK_DOTTING;
			dotviz = False;
			dotlines = 0L;
		}

		/* call the function */
		switch ((*vikeys[vinf->command].func)(win, vinf))
		{
		  case RESULT_ERROR:
			viinitcmd(vinf);
			return RESULT_ERROR;

		  case RESULT_MORE:
			assert(vikeys[vinf->command].when & WHEN_MORE);
			return RESULT_MORE;

		  case RESULT_COMPLETE:
			;
		}

		/* apply the tweaks */
		flags = vinf->tweak;
		if (flags & TWEAK_FRONT)
		{
			(void)m_front(win, vinf);
		}
		if (flags & TWEAK_FIXCOL)
		{
			marksetoffset(state->cursor, markoffset((*win->md->move)(win, state->cursor, 0, win->wantcol, viiscmd(win))));
		}
		else if (!vinf->oper && !(flags & TWEAK_IGNCOL))
		{
			win->wantcol = (*win->md->mark2col)(win, state->cursor, viiscmd(win));
		}

		/* use the final location as the target of an operator, maybe */
		to = *state->cursor;
	}
	else /* visible operator */
	{
		/* Remember this command, and the fact that it is being
		 * applied to a visible selection.
		 */
		if (state->acton == NULL)
		{
			dotcmd = *vinf;
			dotcmd.tweak |= TWEAK_DOTTING;
			dotviz = True;
			if (win->seltype == 'l' || win->seltype == 'L')
			{
				dotlines = markline(win->selbottom) - markline(win->seltop) + 1;
			}
			else
			{
				dotlines = 0L;
			}

		}

		from = *win->seltop;
		to = *win->selbottom;
	}

	/* if TWEAK_MARK, on main buffer, then remember the cursor position */
	if ((vinf->tweak & TWEAK_MARK) != 0 && state->acton == NULL)
	{
		if (win->prevcursor)
		{
			markfree(win->prevcursor);
		}
		win->prevcursor = markdup(&from);
	}

	/* if we have an operator, apply it */
	if (vinf->oper)
	{
		/* make sure both endpoints are in the same buffer */
		if (to.buffer != from.buffer)
		{
			msg(MSG_ERROR, "would span buffers");
			viinitcmd(vinf);
			return RESULT_ERROR;
		}

		/* swap endpoints, if necessary, to make from <= to */
		if (to.offset < from.offset)
		{
			tmp = to.offset;
			to.offset = from.offset;
			from.offset = tmp;
		}

		/* move the cursor to the top of the affected region */
		marksetoffset(state->cursor, from.offset);

		/* The = operator needs special treatment.  If a multi-line
		 * region is selected, then = needs to work in line mode;
		 * otherwise it should work in character mode.
		 */
		if (vinf->oper == '='
		 && markoffset(dispmove(win, 1L, 0L)) <= to.offset)
		{
			vinf->tweak |= TWEAK_LINE|TWEAK_FRONT;
		}

		/* do we need to adjust for line-mode or inclusion? */
		if (vinf->tweak & TWEAK_LINE)
		{
			from.offset = markoffset((*win->md->move)(win, &from, 0, 0, True));
			if (vinf->tweak & TWEAK_INCL)
			{
				to.offset = markoffset((*win->md->move)(win, &to, 0, INFINITY, False)) + 1;
			}
			else
			{
				to.offset = markoffset((*win->md->move)(win, &to, 0, 0, True));
			}
		}
		else
		{
			if ((vinf->tweak & TWEAK_INCL) != 0 && scanchar(&to) != '\n')/*!!!*/
			{
				to.offset++;
			}
		}

		/* do the operator */
		result = oper(win, vinf, &from, &to);

		/* if we were doing visible marking before, end it now */
		if (win->seltop)
		{
			(void)v_visible(win, vinf);
		}

		/* set the desired cursor column to its current column */
		if (!state->acton)
		{
			win->wantcol = (*win->md->mark2col)(win, state->cursor, viiscmd(win));
		}
	}

	/* done! */
	if (result != RESULT_MORE)
		viinitcmd(vinf);
	return result;
}


/* This parses a keystroke as part of a vi command. */
static RESULT parse(key, info)
	_CHAR_	key;	/* keystroke from user */
	void	*info;	/* current vi command parsing state */
{
	VIINFO	*vinf = (VIINFO *)info;
	CHAR	digit;


	/* if ^O, and we aren't already in a ^O, then this ^O does nothing */
	if (key == ELVCTRL('O') && !vinf->control_o)
	{
		vinf->control_o = True;
		windefault->state->mapflags |= MAP_DISABLE;
		return RESULT_MORE;
	}
	else
	{
		vinf->control_o = False;
	}

	if (vinf->phase == VI_CUTBUF)
	{
		if (key == '\033')
		{
			viinitcmd(vinf);
			return RESULT_ERROR;
		}
		vinf->cutbuf = key;
		vinf->phase = (vinf->count == 0 ? VI_START : VI_COUNT2);
	}
	else if (vinf->phase == VI_COUNT2 && isdigit(key))
	{
		vinf->count2 = vinf->count2 * 10 + key - '0';
	}
	else if (vinf->phase == VI_START && vinf->count == 0 && key == '0')
	{
		vinf->command = key;
		vinf->phase = VI_COMPLETE;
	}
	else if (vinf->phase == VI_START && isdigit(key))
	{
		vinf->count = vinf->count * 10 + key - '0';
	}
	else if (vinf->phase == VI_COUNT2 || vinf->phase == VI_KEY2)
	{
		switch (key)
		{
		  case '\033':
			viinitcmd(vinf);
			return RESULT_ERROR;

		  case ELVCTRL('V'):
			vinf->phase = VI_QUOTE;
		  	windefault->state->mapflags |= MAP_DISABLE;
			break;

		  case ELVCTRL('X'):
			vinf->phase = VI_HEX1;
		  	windefault->state->mapflags |= MAP_DISABLE;
			break;

		  default:
			vinf->key2 = key;
			vinf->phase = VI_COMPLETE;
		}
	}
	else if (vinf->phase == VI_QUOTE)
	{
		vinf->key2 = key;
		vinf->phase = VI_COMPLETE;
	}
	else if (vinf->phase == VI_HEX1 || vinf->phase == VI_HEX2)
	{
		/* convert digit from ASCII to binary */
		if (key >= '0' && key <= '9')
		{
			digit = key - '0';
		}
		else if (key >= 'a' && key <= 'f')
		{
			digit = key - 'a' + 10;
		}
		else if (key >= 'A' && key <= 'F')
		{
			digit = key - 'A' + 10;
		}
		else
		{
			viinitcmd(vinf);
			return RESULT_ERROR;
		}

		/* merge it into the key */
		if (vinf->phase == VI_HEX1)
		{
			vinf->key2 = digit << 4;
			vinf->phase = VI_HEX2;
		  	windefault->state->mapflags |= MAP_DISABLE;
		}
		else
		{
			vinf->key2 |= digit;
			vinf->phase = VI_COMPLETE;
		}
	}
	else if (strchr("cdy<=>!\\", (char)key))
	{
		if (windefault && windefault->seltop)
		{
			vinf->oper = key;
			vinf->command = (windefault->seltype == 'c' ? 'e' : '_');
			vinf->phase = VI_COMPLETE;
		}
		else if (vinf->oper == key)
		{
			vinf->command = '_';
			vinf->phase = VI_COMPLETE;
		}
		else if (vinf->oper == '\0')
		{
			vinf->oper = key;
			assert(vinf->phase == VI_START);
		}
		else
		{
			viinitcmd(vinf);
			return RESULT_ERROR;
		}
	}
	else if (key == '"')
	{
		vinf->phase = VI_CUTBUF;
	  	windefault->state->mapflags |= MAP_DISABLE;
	}
	else if (key == '#' || key == 'z')
	{
		vinf->command = key;
		vinf->phase = VI_COUNT2;
	  	windefault->state->mapflags |= MAP_DISABLE;
	}
	else if (strchr("[]@#`'rtfmTF\"Z\027", (char)key))	/* \027 is ^W */
	{
		vinf->command = key;
		vinf->phase = VI_KEY2;
	  	windefault->state->mapflags |= MAP_DISABLE;
	}
	else if (key <= '\177')
	{
		vinf->command = key;
		vinf->phase = VI_COMPLETE;
	}
	else
	{
		/* non-ascii keys can't be a vi command */
		viinitcmd(vinf);
		return RESULT_ERROR;
	}

	return (vinf->phase == VI_COMPLETE ? RESULT_COMPLETE : RESULT_MORE);
}

/* This function decides what shape the cursor should be */
static ELVCURSOR shape(win)
	WINDOW	win;	/* the window whose shape should be fetched */
{
	return CURSOR_COMMAND;
}


/* Push a vi command state onto the stack */
void vipush(win, flags, cursor)
	WINDOW		win;	/* the window to receive the new state */
	ELVISSTATE	flags;	/* flags of the new state */
	MARK		cursor;	/* the cursor to use in the new state */
{
	/* push a state */
	statepush(win, flags);

	/* initialize it to look like a vi state */
	win->state->parse = parse;
	win->state->perform = _viperform;
	win->state->shape = shape;
	win->state->info = safealloc(1, sizeof (VIINFO));
	win->state->modename = viiscmd(win) ? "Command" : "One Cmd";
	win->state->mapflags |= MAP_COMMAND;
	viinitcmd((VIINFO *)win->state->info);

	/* if a specific cursor was given, use it */
	if (cursor)
	{
		win->state->cursor = cursor;
		win->state->top = markdup(cursor);
		win->state->bottom = markdup(cursor);
	}

	/* If this is the main buffer for this window, then make the cursor's
	 * current column the default.
	 */
	if (!win->state->acton && o_bufchars(markbuffer(win->state->cursor)) > 0)
	{
		win->wantcol = (*win->md->mark2col)(win, win->state->cursor, True);
	}
}


/* This function translates a potentially-abbreviated vi key name into 
 * standardized name which can be used as a help topic.  If the name can't
 * be standardized, then it returns NULL instead.
 */
CHAR *viname(name)
	CHAR	*name;
{
	CHAR	key;

	if (name[0] == '^' && name[1] && !name[2])
	{
		/* It is a control character in "^c" format? */
		key = name[1] & 0x1f;
	}
	else if (CHARlen(name) == 1 && (name[0] & ~0x7f) == 0)
	{
		/* It is a simple key */
		key = name[0];
	}
	else
	{
		/* It had better be the topic name of some key.  If it
		 * isn't then reject it.
		 */
		for (key = 0;
		     key < 0x7f && (!vikeys[key].helptopic
				|| CHARcmp(name, toCHAR(vikeys[key].helptopic)));
		     key++)
		{
		}
		if (key >= 0x7f)
			return NULL;
	}
	
	if (toCHAR(vikeys[key].helptopic))
		return toCHAR(vikeys[key].helptopic);
	else
		return name;
}
