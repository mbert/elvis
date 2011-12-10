/* window.c */
/* Copyright 1995 by Steve Kirkendall */

char id_window[] = "$Id: window.c,v 2.35 1996/09/24 22:36:33 steve Exp $";

#include "elvis.h"

#if USE_PROTOTYPES
static int setwm(OPTDESC *desc, OPTVAL *val, CHAR *newval);
static CHAR *getwm(OPTDESC *desc, OPTVAL *val);
#endif


WINDOW		windows;	/* list of all windows */
WINDOW		windefault;	/* the window whose options are current */
WINDOWBUF	windefopts;	/* fake window, stores the default window options */

/* Set the wrapmargin value.  This actually sets the buffer's textwidth
 * option instead of the window's wrapmargin option.
 */
static int setwm(desc, val, newval)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
	CHAR	*newval;/* value the option should have (as a string) */
{
	WINDOW	win = (WINDOW)val->value.pointer;
	BUFFER	buf;
	long	l;

	assert(win == windefault || win == &windefopts);

	/* choose a buffer */
	buf = (win->cursor ? markbuffer(win->cursor) : bufdefopts);

	/* check for bad value */
	if (!calcnumber(newval))
	{
		msg(MSG_ERROR, "[s]$1 must be number", desc->longname);
		return -1;
	}
	l = CHAR2long(newval);
	if (l < 0 || l >= o_columns(win))
	{
		msg(MSG_ERROR, "[sd]$1 must be between 0 and $2", desc->longname, o_columns(win));
		return -1;
	}

	/* compute the new value of textwidth */
	if (l != 0)
	{
		l = o_columns(win) - l;
	}

	/* store the value, if changed */
	if (l != o_textwidth(buf))
	{
		o_textwidth(buf) = l;
		optflags(o_textwidth(buf)) |= OPT_SET;
	}

	/* wrapmargin itself never changes */
	return 0;
}

/* This value computes a value for wrapmargin, based on the window's "columns"
 * option and the buffer's "textwidth" option.  If the window's "columns" is
 * less than or equal to the buffer's "textwidth", this function will return
 * "wide" instead of a number because the "wrapmargin" option doesn't allow
 * setting textwidth to a value wider than the window.
 */
static CHAR *getwm(desc, val)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
{
	static CHAR str[12];
	WINDOW	win = (WINDOW)val->value.pointer;
	BUFFER	buf = markbuffer(win->cursor);

	assert(win == windefault);

	if (o_textwidth(buf) >= o_columns(win))
	{
		CHARcpy(str, toCHAR("wide"));
	}
	else if (o_textwidth(buf) == 0)
	{
		CHARcpy(str, toCHAR("0"));
	}
	else
	{
		long2CHAR(str, o_columns(win) - o_textwidth(buf));
	}
	return str;
}


static OPTDESC wdesc[] =
{
	{"windowid", "id",	optnstring,	optisnumber	},
#if defined (GUI_WIN32)
	{"columns", "cols",	optnstring,	optiswinsize	},
	{"lines", "rows",	optnstring,	optiswinsize	},
#else
	{"columns", "cols",	optnstring,	optisnumber	},
	{"lines", "rows",	optnstring,	optisnumber	},
#endif
	{"list", "li",		NULL,		NULL		},
	{"display", "mode",	optsstring,	optisstring	},
	{"number", "nu",	NULL,		NULL		},
	{"ruler", "ru",		NULL,		NULL		},
	{"scroll", "scr",	optnstring,	optisnumber	},
	{"showmatch", "sm",	NULL,		NULL		},
	{"showmode", "smd",	NULL,		NULL		},
	{"showstack", "sstk",	NULL,		NULL		},
	{"showcmd", "sc",	NULL,		NULL		},
	{"wrap", "wr",		NULL,		NULL		},
	{"sidescroll", "ss",	optnstring,	optisnumber,	"1:30"	},
	{"wrapmargin", "wm",	getwm,		setwm		}
};

/* Initialize the window module.  Mostly this makes the windefopts variable
 * act as the current window, so that window options can be set during
 * initialization, before any real windows have been created.
 */
void wininit()
{
	/* initialize the options */
	optpreset(o_display(&windefopts), toCHAR("normal"), OPT_LOCK);
	optflags(o_windowid(&windefopts)) = OPT_LOCK|OPT_HIDE;
#if defined (GUI_WIN32)
	optpreset(o_columns(&windefopts), 80, OPT_SET);
	optpreset(o_lines(&windefopts), 20, OPT_SET);
#else
	optpreset(o_columns(&windefopts), 80, OPT_SET|OPT_LOCK);
	optflags(o_lines(&windefopts)) = OPT_SET|OPT_LOCK;
#endif
	o_scroll(&windefopts) = 12;
	windefopts.wrapmargin.value.pointer = (void *)&windefopts;
	windefopts.wrapmargin.flags = OPT_REDRAW|OPT_HIDE;
	optflags(o_list(&windefopts)) = OPT_REDRAW;
	optflags(o_number(&windefopts)) = OPT_REDRAW;
	optpreset(o_wrap(&windefopts), True, OPT_REDRAW);
	o_sidescroll(&windefopts) = 8;

	/* make the options accessible to :set */
	optinsert("defwin", QTY(wdesc), wdesc, &windefopts.windowid);
}


/* Allocate a new window.  This function should only be called by the
 * EVENT module; other modules are expected to call the GUI's creategw()
 * function, which will ultimately cause EVENT to call this function.
 */
WINDOW winalloc(gw, gvals, buf, rows, columns)
	GUIWIN	*gw;	/* GUI's handle for the new window */
	OPTVAL	*gvals;	/* values of the GUI's window-dependent options */
	BUFFER	buf;	/* buffer to use in the new window */
	long	rows;	/* height of the new window */
	long	columns;/* width of the new window */
{
	static long	nextwindowid;	/* counter used for assigning windowid */
	WINDOW		newp;		/* pointer to the new window */

	/* allocate a window, and initialize it */
	newp = (WINDOW)safealloc(1, sizeof *newp);
	*newp = windefopts;
	newp->next = windows;
	windows = newp;
	newp->gw = gw;
	newp->guivals = gvals;
	newp->cursor = markalloc(buf, buf->changepos);
	newp->wantcol = 0;
	newp->cursx = newp->cursy = -1;

	/* Initialize options */
	o_windowid(newp) = ++nextwindowid;
	o_lines(newp) = rows;
	o_columns(newp) = columns;
	newp->wrapmargin.value.pointer = (void *)newp;

	/* allocate storage space for the screen images */
	newp->di = (DRAWINFO *)drawalloc((int)rows, (int)columns);

	/* choose the default display mode */
	if (!dispset(newp, tochar8(o_bufdisplay(buf))))
		(void)dispset(newp, NULL);

	/* no text is selected, initially */
	newp->seltop = newp->selbottom = NULL;

	/* there is initially no matching parenthesis */
	newp->match = -4;

	/* push the initial state */
	if (gui->moveto)
	{
	 	vipush(newp, 0, newp->cursor);
		newp->di->drawstate = DRAW_VISUAL;
	}
	else
	{
		vipush(newp, ELVIS_BOTTOM, newp->cursor);
		newp->di->drawstate = DRAW_OPENOUTPUT;
	}

	/* push an extra state or two, if we're supposed to */
	switch (o_initialstate)
	{
	  case 'i':
		bufwilldo(newp->cursor);
		inputpush(newp, newp->state->flags, 'i');
		break;

	  case 'r':
		bufwilldo(newp->cursor);
		inputpush(newp, newp->state->flags, 'R');
		break;

	  case 'e':
		/* push a whole new stratum! */
		statestratum(newp, toCHAR(EX_BUF), ':', exenter);
		o_internal(markbuffer(newp->state->cursor)) = True;
		newp->state->flags &= ~(ELVIS_POP|ELVIS_ONCE|ELVIS_1LINE);
		break;
	}

	/* if no other window is the default already, then make this the
	 * default and flush any accumulated messages.
	 */
	if (!windefault)
	{
		winoptions(newp);
		msgflush();
	}

	/* If this is the first window, then peform the -c command or -t tag */
	if (!newp->next)
	{
		mainfirstcmd(newp);
	}

	return newp;
}

/* Free a window.  This function should only be called by the EVENT module;
 * other modules are expected to call the GUI's destroygw() function, which
 * will ultimately cause EVENT to call this function.
 */
void winfree(win, force)
	WINDOW	win;	/* the window to be freed */
	BOOLEAN	force;	/* If True, try harder */
{
	WINDOW	scan, lag;
	int	i;

	/* remove this window from the list of windows */
	assert(windows);
	for (scan = windows, lag = NULL; scan != win; lag = scan, scan = scan->next)
	{
		assert(scan->next);
	}
	if (lag)
	{
		lag->next = scan->next;
	}
	else
	{
		windows = scan->next;
	}

	/* if this was the default, it isn't now! */
	if (windefault == win)
	{
		winoptions((WINDOW)0);
	}

	/* free the whole state stack */
	while (win->state)
	{
		statepop(win);
	}

	/* free the marks in the tagstack */
	for (i = 0; i < TAGSTK && win->tagstack[i].origin; i++)
	{
		markfree(win->tagstack[i].origin);
		if (win->tagstack[i].prevtag)
			safefree(win->tagstack[i].prevtag);
	}

	/* Unload the buffer.  This will make it disappear if that's safe.
	 * If it succeeds, it will also free the cursor; if it doesn't
	 * succeed, then we want to free the cursor marks anyway.
	 */
	if (!bufunload(markbuffer(win->cursor), force, False))
	{
		markfree(win->cursor);
		if (win->prevcursor)
		{
			markfree(win->prevcursor);
		}
		if (win->seltop)
		{
			markfree(win->seltop);
			markfree(win->selbottom);
		}
	}

	/* free the options' value strings */
	optfree(QTY(wdesc), &win->windowid);

	/* free the storage space for the window's image */
	drawfree(win->di);

	/* free any resources allocated for the display mode */
	if (win->md)
	{
		(*win->md->term)(win->mi);
	}

	/* free the window struct itself */
	safefree(win);
}

/* Change the size of the window.  This function should only be called by
 * the EVENT module; other modules aren't expected to worry about window size
 * changes.
 */
void winresize(win, rows, columns)
	WINDOW	win;	/* the window to be resized */
	long	rows;	/* new height of the window */
	long	columns;/* new width of the window */
{
	long	oldtop;

	/* update the options */
	o_lines(win) = rows;
	o_columns(win) = columns;
	if (!(optflags(o_scroll(win)) & OPT_SET))
	{
		o_scroll(win) = rows / 2;
	}

	/* free the old screen image, and allocate a new one */
	oldtop = win->di->topline;
	drawfree(win->di);
	win->di = drawalloc((int)rows, (int)columns);

	/* try to keep the old top row for the next refresh */
	win->di->topline = oldtop;
	win->di->bottomline = markoffset(win->cursor);
}

/* Cause a different buffer to be associated with this window.  This function
 * will call the GUI's retitle() function to change the window title to match
 * the new buffer's name.
 */
void winchgbuf(win, buf, force)
	WINDOW	win;	/* window that will use the new buffer */
	BUFFER	buf;	/* the new buffer */
	BOOLEAN	force;	/* if True, try harder */
{
	/* if same buffer, do nothing */
	if (buf == markbuffer(win->cursor))
	{
		return;
	}

	/* remember the old filename and line number */
	optprevfile(o_filename(markbuffer(win->cursor)), markline(win->cursor));

	/* free the prevcursor mark */
	if (win->prevcursor)
	{
		markfree(win->prevcursor);
		win->prevcursor = NULL;
	}

	/* cancel any selection */
	if (win->seltop)
	{
		markfree(win->seltop);
		markfree(win->selbottom);
		win->seltop = win->selbottom = NULL;
	}

	/* release the old buffer, and discard it if that's okay */ 
	if (bufunload(markbuffer(win->cursor), False, False) || force)
	{
		/* good!  now switch buffers */
		marksetbuffer(win->cursor, buf);
		marksetoffset(win->cursor, 0);
		if (windefault == win)
		{
			bufoptions(buf);
		}

		/* switch to the new buffer's preferred display mode */
		dispset(win, tochar8(o_bufdisplay(buf)));

		/* retitle the GUI window */
		if (gui->retitle)
		{
			(*gui->retitle)(win->gw, tochar8(o_bufname(buf)));
		}
	}
}

/* Make this window be the default window, and its associated buffer be the
 * default buffer.  If win is NULL, no window will be the default.
 */
void winoptions(win)
	WINDOW	win;	/* the new default window */
{
	/* if this window isn't already the default... */
	if (windefault != win)
	{
		/* delete the old default window's options */
		if (windefault)
		{
			optdelete(&windefault->windowid);
			if (gui->nopts > 0 && windefault->guivals)
			{
				optdelete(windefault->guivals);
			}
		}
		else if (windefopts.modename)
		{
			optdelete(&windefopts.windowid);
			windefopts.modename = (char *)1;
		}

		/* make this window's options the defaults */
		if (win)
		{
			optinsert("win", QTY(wdesc), wdesc, &win->windowid);
			if (gui->nopts > 0)
			{
				optinsert("guiwin", gui->nopts, gui->optdescs, win->guivals);
			}
		}

		windefault = win;
	}

	if (win)
	{
		/* make this window's buffer be the default buffer */
		bufoptions(markbuffer(win->cursor));

		/* make this window's mode be the default mode */
		dispoptions(win->md, win->mi);
	}
	else
	{
		/* no defaults for buffer or mode */
		bufoptions(NULL);
		dispoptions(NULL, NULL);
	}
}


/* Count the windows.  If buf is NULL, then count all windows; else only
 * count windows which contain buf in the state stack or tag stack.
 */
int wincount(buf)
	BUFFER	buf;	/* buffer of interest, or NULL for any buffer */
{
	int	i, j;
	WINDOW	scan;
	STATE	*s;

	for (scan = windows, i = 0; scan; scan = scan->next)
	{
		/* if no specific buf is requested, then count every window */
		if (!buf)
		{
			i++;
			continue;
		}

		/* else look for it in state stack or tag stack */
		for (s = scan->state; s; s = s->pop)
			if (markbuffer(s->cursor) == buf)
				goto Found;
		for (j = 0; j < TAGSTK && scan->tagstack[j].origin; j++)
			if (markbuffer(scan->tagstack[j].origin) == buf)
				goto Found;

		/* if we get here, then this window doesn't use the requested
		 * buffer and should therefore not be included in the count.
		 */
		continue;

Found:
		i++;
	}
	return i;
}


/* locate the WINDOW associated with GUIWIN "gw" */
WINDOW winofgw(gw)
	GUIWIN	*gw;	/* GUI's handle for the window */
{
	WINDOW	win;

	assert(windows);
	for (win = windows; win->gw != gw; win = win->next)
	{
		assert(win->next);
	}
	return win;
}


/* Locate a WINDOW associated with BUFFER "buf".  If "win" is NULL, then
 * searching begins at the start of the window list; else it begins after
 * the given window.  If "buf" is NULL, then any window matches regardless
 * of its buffer.  Returns the WINDOW if a match is found, or NULL of no
 * matches are found.
 */
WINDOW winofbuf(win, buf)
	WINDOW	win;	/* where to start searching; NULL for first search */
	BUFFER	buf;	/* the buffer of interest */
{
	/* if "win" is NULL, then start searching at first window */
	if (!win)
	{
		win = windows;
	}
	else
	{
		win = win->next;
	}

	/* search for "buf", or NULL */
	while (win && buf && markbuffer(win->cursor) != buf)
	{
		win = win->next;
	}
	return win;
}
