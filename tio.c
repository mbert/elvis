/* tio.c */

/* Author:
 *	Steve Kirkendall
 *	1500 SW Park #326
 *	Portland OR, 97201
 *	kirkenda@cs.pdx.edu
 */


/* This file contains terminal I/O functions */

#include <string.h>
#include "config.h"
#include "vi.h"
#include "ctype.h"
extern char *printable();

static int showmsg P_((void));

/* This function reads in a line from the terminal.  It simulates the normal
 * line editing for cooked input, with support for backspace, ^U, and ^V.
 *
 * Support for ^W and ^P added by sdw, Nov. 1993.
 *
 * It tries to hide the extra ^O that a "visual" map inserts before each
 * character.  When it reads a ^O, it discards it and reads the next character.
 * Then only exception is after a ^V which itself was not preceded by a ^O, the
 * character immediately following the ^V is accepted even if it is a ^O.
 *
 * Eventually I hope to make it use ^O to access a history of previously
 * entered commands.  ^Ok to move back, ^Ol to move forward, etc.  This way,
 * the standard arrow key mappings can be used to access history easily, and
 * users who don't have the benefit of arrow keys will still be able to use
 * history.  But that hasn't happened yet.
 */
int vgets(prompt, buf, bsize)
	int	prompt;	/* the prompt character, or '\0' for none */
	char	*buf;	/* buffer into which the string is read */
	int	bsize;	/* size of the buffer */
{
	int	len;	/* how much we've read so far */
	int	ch;	/* a character from the user */
	int	quoted;	/* is the next char quoted? */
	int	tab;	/* column position of cursor */
	char	widths[132];	/* widths of characters */
	int	word;	/* index of first letter of word */
#ifndef NO_DIGRAPH
	int	erased;	/* 0, or first char of a digraph */
#endif
#ifndef NO_EXTENSIONS
	int	ctrlO;	/* boolean: was last character ^O ? */
#endif
#if 1 /* [sdw] */
	int	cbsize;	/* size of cut buffer to be pasted */
#endif

	/* show the prompt */
	move(LINES - 1, 0);
	tab = 0;
	if (prompt)
	{
		addch(prompt);
		tab = 1;
	}
	clrtoeol();
	refresh();

	/* read in the line */
#ifndef NO_DIGRAPH
	erased =
#endif
#ifndef NO_EXTENSIONS
	ctrlO =
#endif
	quoted = len = 0;
	for (;;)
	{
#ifndef NO_ABBR
		if (quoted || mode == MODE_EX)
		{
			ch = getkey(0);
		}
		else
		{
			/* maybe expand an abbreviation while getting key */
			ch = getabkey(WHEN_EX, buf, len);
		}
#else
		ch = getkey(0);
#endif
#ifndef NO_EXTENSIONS
		if (ctrlO || !quoted && ch == ctrl('O'))
		{
			ch = getkey(quoted ? 0 : WHEN_EX);
			if (ch == ctrl('V'))
			{
				ctrlO = TRUE;
			}
		}
#endif

		/* some special conversions */
#if 0
		if (ch == ctrl('D') && len == 0)
			ch = ctrl('[');
#endif
#ifndef NO_DIGRAPH
		if (*o_digraph && erased != 0 && ch != '\b')
		{
			ch = digraph(erased, ch);
			erased = 0;
		}
#endif

		/* inhibit detection of special chars (except ^J) after a ^V */
		if (quoted && ch != '\n')
		{
			ch |= 256;
		}

		/* process the character */
		switch(ch)
		{
		  case ctrl('V'):
			qaddch('^');
			qaddch('\b');
			quoted = TRUE;
			break;

		  case ctrl('D'):
			return -1;

		  case ctrl('['):
		  case '\n':
#if OSK
		  case '\l':
#else
		  case '\r':
#endif
			clrtoeol();
			goto BreakBreak;

#ifndef CRUNCH
		  case ctrl('U'):
			while (len > 0)
			{
				len--;
				while (widths[len]-- > 0)
				{
					qaddch('\b');
					qaddch(' ');
					qaddch('\b');
				}
			}
			break;

		  /* [sdw] -- verbose but functional... */
		  /* erase over previous Word */
		  case ctrl('W'):
			if (len == 0)
			{
				return -1;
			}
			while (len > 0
				&& (buf[len-1] == ' ' || buf[len-1] == '\t'))
			{
				len--;
#  ifndef NO_DIGRAPH
				erased = buf[len];
#  endif
				for (ch = widths[len]; ch > 0; ch--)
					addch('\b');
				tab -= widths[len];
			}
			while (len > 0
				&& buf[len-1] != ' ' && buf[len-1] != '\t')
			{
				len--;
#  ifndef NO_DIGRAPH
				erased = buf[len];
#  endif
				for (ch = widths[len]; ch > 0; ch--)
					addch('\b');
				tab -= widths[len];
			}
			if (mode == MODE_EX)
			{
				clrtoeol();
			}
			break;
#endif

		  case '\b':
			if (len > 0)
			{
				len--;
#ifndef NO_DIGRAPH
				erased = buf[len];
#endif
				for (ch = widths[len]; ch > 0; ch--)
					addch('\b');
				if (mode == MODE_EX)
				{
					clrtoeol();
				}
				tab -= widths[len];
			}
			else
			{
				return -1;
			}
			break;

#if 1 /* [sdw] */
		  /* paste in contents of anonymous buffer */
		  case ctrl('P'):
			cbsize = cb2str(0, tmpblk.c, BLKSIZE);
			if (cbsize > 0 && cbsize != BLKSIZE)
			{
				execmap(0, tmpblk.c, FALSE);
			}
			break;
#endif

		  default:
			/* strip off quotation bit */
			if (ch & 256)
			{
				ch &= ~256;
				qaddch(' ');
				qaddch('\b');
			}

			/* add & echo the char */
			if (len < bsize - 1)
			{
				if (ch == '\t' && !quoted)
				{
					widths[len] = *o_tabstop - (tab % *o_tabstop);
					addstr("        " + 8 - widths[len]);
					tab += widths[len];
				}
				else if (ch > 0 && ch < ' ') /* > 0 by GB */
				{
					addch('^');
					addch(ch + '@');
					widths[len] = 2;
					tab += 2;
				}
				else if (ch == '\177')
				{
					addch('^');
					addch('?');
					widths[len] = 2;
					tab += 2;
				}
				else
				{
					addch(ch);
					widths[len] = 1;
					tab++;
				}
				buf[len++] = ch;
			}
			else
			{
				beep();
			}
#ifndef NO_EXTENSIONS
			ctrlO =
#endif
			quoted = FALSE;
		}
	}
BreakBreak:
	refresh();
	buf[len] = '\0';
	return len;
}


static int	manymsgs; /* This variable keeps msgs from overwriting each other */
static char	pmsg[80]; /* previous message (waiting to be displayed) */


static int showmsg()
{
	/* if there is no message to show, then don't */
	if (!manymsgs)
		return FALSE;

	/* display the message */
	move(LINES - 1, 0);
	if (*pmsg)
	{
		standout();
		qaddch(' ');
		qaddstr(pmsg);
		qaddch(' ');
		standend();
	}
	clrtoeol();

	manymsgs = FALSE;
	return TRUE;
}


void endmsgs()
{
	if (manymsgs)
	{
		showmsg();
		addch('\n');
	}
}

/* Write a message in an appropriate way.  This should really be a varargs
 * function, but there is no such thing as vwprintw.  Hack!!!
 *
 * In MODE_EX or MODE_COLON, the message is written immediately, with a
 * newline at the end.
 *
 * In MODE_VI, the message is stored in a character buffer.  It is not
 * displayed until getkey() is called.  msg() will call getkey() itself,
 * if necessary, to prevent messages from being lost.
 *
 * msg("")		- clears the message line
 * msg("%s %d", ...)	- does a printf onto the message line
 */
#if NEWSTYLE
void msg (char *fmt, ...)
{
	va_list	ap;
	va_start (ap, fmt);
#else
void msg(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
	char	*fmt;
	long	arg1, arg2, arg3, arg4, arg5, arg6, arg7;
{
#endif
	if (mode != MODE_VI)
	{
#if NEWSTYLE
		vsprintf (pmsg, fmt, ap);
#else
		sprintf(pmsg, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
#endif
		qaddstr(pmsg);
		addch('\n');
		exrefresh();
	}
	else
	{
		/* wait for keypress between consecutive msgs */
		if (manymsgs)
		{
			getkey(WHEN_MSG);
		}

		/* real message */
#if NEWSTYLE
		vsprintf (pmsg, fmt, ap);
#else
		sprintf(pmsg, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
#endif
		if (*fmt)
		{
			manymsgs = TRUE;
		}
	}
#ifdef	__STDC__
	va_end (ap);
#endif
}


/* This function calls refresh() if the option exrefresh is set */
void exrefresh()
{
	char	*scan;

	/* If this ex command wrote ANYTHING set exwrote so vi's  :  command
	 * can tell that it must wait for a user keystroke before redrawing.
	 */
	for (scan=kbuf; scan<stdscr; scan++)
		if (*scan == '\n')
			exwrote = TRUE;

	/* now we do the refresh thing */
	if (*o_exrefresh)
	{
		refresh();
	}
	else
	{
		wqrefresh();
	}
	if (mode != MODE_VI && *o_more)
	{
		manymsgs = FALSE;
	}
}


/* This structure is used to store maps and abbreviations.  The distinction
 * between them is that maps are stored in the list referenced by the "maps"
 * pointer, while abbreviations are referenced by the "abbrs" pointer.
 */
typedef struct _map
{
	struct _map	*next;	/* another abbreviation */
	short		len;	/* length of the "rawin" characters */
	short		flags;	/* various flags */
	char		*label;	/* label of the map/abbr, or NULL */
	char		*rawin;	/* the "rawin" characters */
	char		*cooked;/* the "cooked" characters */
} MAP;

static char	keybuf[KEYBUFSIZE];
static int	cend;	/* end of input characters */
static int	user;	/* from user through cend are chars typed by user */
static int	next;	/* index of the next character to be returned */
static MAP	*match;	/* the matching map, found by countmatch() */
static MAP	*maps;	/* the map table */
#ifndef NO_ABBR
static MAP	*abbrs;	/* the abbreviation table */
#endif



/* ring the terminal's bell */
void beep()
{
	/* do a visible/audible bell */
	if (*o_flash)
	{
		do_VB();
		refresh();
	}
	else if (*o_errorbells)
	{
		do_beep();
	}

	/* discard any buffered input, and abort macros */
	next = user = cend;
}



/* This function replaces a "rawin" character sequence with the "cooked" version,
 * by modifying the internal type-ahead buffer.
 */
void execmap(rawlen, cookedstr, visual)
	int	rawlen;		/* length of rawin text -- string to delete */
	char	*cookedstr;	/* the cooked text -- string to insert */
	int	visual;		/* boolean -- chars to be executed in visual mode? */
{
	int	cookedlen;
	char	*src, *dst;
	int	i;

	/* find the length of the cooked string */
	cookedlen = strlen(cookedstr);
#ifndef NO_EXTENSIONS
	if (visual)
	{
		cookedlen *= 2;
	}
#endif

	/* if too big to fit in type-ahead buffer, then don't do it */
	if (cookedlen + (cend - next) - rawlen > KEYBUFSIZE)
	{
		return;
	}

	/* shift to make room for cookedstr at the front of keybuf */
	src = &keybuf[next + rawlen];
	dst = &keybuf[cookedlen];
	i = cend - (next + rawlen);
	if (src >= dst)
	{
		while (i-- > 0)
		{
			*dst++ = *src++;
		}
	}
	else
	{
		src += i;
		dst += i;
		while (i-- > 0)
		{
			*--dst = *--src;
		}
	}

	/* insert cookedstr, and adjust offsets */
	cend += cookedlen - rawlen - next;
	user += cookedlen - rawlen - next;
	next = 0;
	for (dst = keybuf, src = cookedstr; *src; )
	{
#ifndef NO_EXTENSIONS
		if (visual)
		{
			*dst++ = ctrl('O');
			cookedlen--;
		}
#endif
		*dst++ = *src++;
	}

#ifdef DEBUG2
	{
#include <stdio.h>
		FILE	*debout;
		int		i;

		debout = fopen("debug.out", "a");
		fprintf(debout, "After execmap(%d, \"%s\", %d)...\n", rawlen, cookedstr, visual);
		for (i = 0; i < cend; i++)
		{
			if (i == next) fprintf(debout, "(next)");
			if (i == user) fprintf(debout, "(user)");
			if (UCHAR(keybuf[i]) < ' ')
				fprintf(debout, "^%c", keybuf[i] ^ '@');
			else
				fprintf(debout, "%c", keybuf[i]);
		}
		fprintf(debout, "(end)\n");
		fclose(debout);
	}
#endif
}

#ifndef NO_CURSORSHAPE
/* made global so that suspend_curses() can reset it.  -nox */
int	oldcurs;
#endif
/* This function calls ttyread().  If necessary, it will also redraw the screen,
 * change the cursor shape, display the mode, and update the ruler.  If the
 * number of characters read is 0, and we didn't time-out, then it exits because
 * we've apparently reached the end of an EX script.
 */
static int fillkeybuf(when, timeout)
	int	when;	/* mixture of WHEN_XXX flags */
	int	timeout;/* timeout in 1/10 second increments, or 0 */
{
	int	nkeys;
#ifndef NO_SHOWMODE
	static int	oldwhen;	/* "when" from last time */
	static int	oldleft;
	static long	oldtop;
	static long	oldnlines;
	char		*str;
#endif

#ifdef DEBUG
	watch();
#endif


#ifndef NO_CURSORSHAPE
	/* make sure the cursor is the right shape */
	if (has_CQ)
	{
		if (when != oldcurs)
		{
			switch (when)
			{
			  case WHEN_EX:		do_CX();	break;
			  case WHEN_VICMD:	do_CV();	break;
			  case WHEN_VIINP:	do_CI();	break;
			  case WHEN_VIREP:	do_CR();	break;
			}
			oldcurs = when;
		}
	}
#endif

#ifndef NO_SHOWMODE
	/* if "showmode" then say which mode we're in */
	if (*o_smd && (when & WHENMASK))
	{
		/* redraw the screen before we check to see whether the
		 * "showmode" message needs to be redrawn.
		 */
		redraw(cursor, !(when & WHEN_VICMD));

		/* now the "topline" test should be valid */
		if (when != oldwhen
		 || topline != oldtop
		 || leftcol != oldleft
		 || nlines != oldnlines)
		{
			oldwhen = when;
			oldtop = topline;
			oldleft = leftcol;
			oldnlines = nlines;

			if (when & WHEN_VICMD)	    str = "Command";
			else if (when & WHEN_VIINP) str = " Input ";
			else if (when & WHEN_VIREP) str = "Replace";
			else if (when & WHEN_REP1)  str = "Replc 1";
			else if (when & WHEN_CUT)   str = "Buffer ";
			else if (when & WHEN_MARK)  str = " Mark  ";
			else if (when & WHEN_CHAR)  str = "Dest Ch";
			else			    str = (char *)0;

			if (str)
			{
				move(LINES - 1, COLS - 10);
				standout();
				qaddstr(str);
				standend();
			}
		}
	}
#endif

#ifndef NO_EXTENSIONS
	/* maybe display the ruler */
	if (*o_ruler && (when & (WHEN_VICMD|WHEN_VIINP|WHEN_VIREP)))
	{
		char	buf[20];

		redraw(cursor, !(when & WHEN_VICMD));
		pfetch(markline(cursor));
# ifndef NO_LEARN
		if (learn)
			sprintf(buf, "%7ld%c%-4d", markline(cursor), learn, 1 + idx2col(cursor, ptext, when & (WHEN_VIINP|WHEN_VIREP)));
		else
# endif
		sprintf(buf, "%7ld,%-4d", markline(cursor), 1 + idx2col(cursor, ptext, when & (WHEN_VIINP|WHEN_VIREP)));
		move(LINES - 1, COLS - 22);
		addstr(buf);
	}
#ifndef NO_LEARN
	else if (when & (WHEN_VICMD|WHEN_VIINP|WHEN_VIREP))
	{
		move(LINES - 1, COLS - 15);
		if (learn)
			addch(learn);
		else
			addch(' ');
	}
#endif
#endif

	/* redraw, so the cursor is in the right place */
	if (when & WHENMASK)
	{
		redraw(cursor, !(when & (WHENMASK & ~(WHEN_VIREP|WHEN_VIINP))));
	}

	/* Okay, now we can finally read the rawin keystrokes */
	refresh();
	nkeys = ttyread(keybuf + cend, sizeof keybuf - cend, timeout);

	/* if nkeys == 0 then we've reached EOF of an ex script. */
	if (nkeys == 0 && timeout == 0)
	{
		tmpabort(TRUE);
		move(LINES - 1, 0);
		clrtoeol();
		refresh();
		endwin();
		exit(exitcode);
	}

	cend += nkeys;
#if 0 /* [sdw] this looks like a bug... */
	user += nkeys;
#endif
	return nkeys;
}


/* This function counts the number of maps that could match the characters
 * between &keybuf[next] and &keybuf[cend], including incomplete matches.
 * The longest comlete match is remembered via the "match" variable.
 */
static int countmatch(when)
	int	when;	/* mixture of WHEN_XXX flags */
{
	MAP	*map;
	int	count;

	/* clear the "match" variable */
	match = (MAP *)0;

	/* check every map */
	for (count = 0, map = maps; map; map = map->next)
	{
		/* can't match if wrong mode */
		if ((map->flags & when) == 0)
		{
			continue;
		}

		/* would this be a complete match? */
		if (map->len <= cend - next)
		{
			/* Yes, it would be.  Now does it really match? */
			if (!strncmp(map->rawin, &keybuf[next], map->len))
			{
				count++;

				/* if this is the longest complete match,
				 * then remember it.
				 */
				if (!match || match->len < map->len)
				{
					match = map;
				}
			}
		}
		else
		{
			/* No, it wouldn't.  But check for partial match */
			if (!strncmp(map->rawin, &keybuf[next], cend - next))
			{
				/* increment by 2 instead of 1 so that, in the
				 * event that we have a partial match with a
				 * single map, we don't mistakenly assume we
				 * have resolved the map yet.
				 */
				count += 2;
			}
		}
	}
	return count;
}


#ifndef NO_ABBR
/* This function checks to see whether a word is an abbreviation.  If it is,
 * then an appropriate number of backspoace characters is inserted into the
 * type-ahead buffer, followed by the expanded form of the abbreviation.
 */
static void expandabbr(line, llen)
	char	*line;
	int	llen;
{
	MAP	*abbr;

	/* if the next character wouldn't end the word, then don't expand */
	if (isalnum(keybuf[next]) || keybuf[next] == ctrl('V') || keybuf[next] == '\b')
	{
		return;
	}

	/* find the abbreviation, if any */
	for (abbr = abbrs;
	     abbr && (abbr->len > llen	/* abbreviation longer than line */
		|| (abbr->len < llen && isalnum(line[llen - abbr->len - 1]))
					/* text would be preceded by alnum */
		|| strncmp(abbr->rawin, line + llen - abbr->len, abbr->len));
					/* text doesn't match abbr */
	     abbr = abbr->next)
	{
	}

	/* If an abbreviation was found, then expand it by inserting the long
	 * version into the type-ahead buffer, and then inserting (in front of
	 * the long version) enough backspaces to erase to the short version.
	 */
	if (abbr)
	{
		llen = abbr->len;
		execmap(0, abbr->cooked, FALSE);
		while (llen > 15)
		{
			execmap(0, "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b", FALSE);
			llen -= 15;
		}
		if (llen > 0)
		{
			execmap(0, "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b" + 15 - llen, FALSE);
		}
	}
}
#endif


/* This function calls getabkey() without attempting to expand abbreviations */
int getkey(when)
	int	when;	/* mixture of WHEN_XXX flags */
{
	return getabkey(when, "", 0);
}


/* This is it.  This function returns keystrokes one-at-a-time, after mapping
 * and abbreviations have been taken into account.
 */
int getabkey(when, line, llen)
	int	when;	/* mixture of WHEN_XXX flags */
	char	*line;	/* a line that may need to be expanded as an abbr */
	int	llen;	/* length of "line" -- since "line" might not have \0 */
{
	int	matches;
#ifdef DEBUG
	static long 	prevchg;
	static int	nslow;
	static char	slow[80];
#endif

	/* if not reading an EX command, and we're not optimizing, then redraw
	 * the display.
	 */
#ifndef CRUNCH
	if (!*o_optimize && (when & WHENMASK))
	{
		redraw(cursor, !(when & (WHENMASK & ~(WHEN_VIREP|WHEN_VIINP))));
	}
#endif

	/* if this key is needed for delay between multiple error messages,
	 * then reset the manymsgs flag and abort any mapped key sequence.
	 */
	if (showmsg())
	{
		if (when == WHEN_MSG)
		{
#ifndef CRUNCH
			if (!*o_more)
			{
				refresh();
				return ' ';
			}
#endif
			qaddstr("[More...]");
			refresh();
			execmap(user, "", FALSE);
		}
	}

#ifdef DEBUG
	/* periodically check for screwed up internal tables */
	watch();
#endif

	/* if buffer empty, read some characters without timeout */
	if (next >= cend)
	{
		next = user = cend = 0;
		fillkeybuf(when, 0);
	}

	/* try to map the key, unless already mapped and not ":set noremap" */
	if (next <= user || *o_remap)
	{
		do
		{
			/* read keystrokes until we have either eliminated
			 * all possible matching maps, or have found exactly
			 * one complete match and have eliminated all partial
			 * maps.
			 */
			do
			{
				matches = countmatch(when);
			} while (matches > 1 && fillkeybuf(when, *o_keytime) > 0);

			/* if we have 1 complete match, then map it */
			if (matches == 1)
			{
				execmap(match->len, match->cooked,
					(match->flags & WHEN_INMV) != 0 
					 && (when & (WHEN_VIINP|WHEN_VIREP)) != 0);
			}
		} while (*o_remap && matches == 1);
	}

	/* ERASEKEY should always be mapped to '\b'. */
	if (keybuf[next] == ERASEKEY)
	{
		keybuf[next] = '\b';
	}

#ifndef NO_LEARN
	learnkey(keybuf[next]);
#endif

#ifndef NO_ABBR
	/* try to expand an abbreviation, except in visual command mode */
	if (llen > 0 && (mode & (WHEN_EX|WHEN_VIINP|WHEN_VIREP)) != 0)
	{
		expandabbr(line, llen);
	}
#endif

#ifdef DEBUG
	/* if slowmacro is set, then show keystroke before executing anything */
	if (*o_slowmacro && next < user)
	{
		/* if previous command changed something, then pause */
		if (changes != prevchg)
		{
			prevchg = changes;
			redraw(cursor, !(when & (WHENMASK & ~(WHEN_VIREP|WHEN_VIINP))));
			slow[nslow] = 0;
			move(LINES - 1, 0);
			qaddstr(printable(slow));
			clrtoeol();
			refresh();
			redraw(cursor, !(when & (WHENMASK & ~(WHEN_VIREP|WHEN_VIINP))));
			sleep(1);
			nslow = 0;
		}

		/* display the next key to be processed */
		slow[nslow++] = keybuf[next];
		slow[nslow] = 0;
		move(LINES - 1, 0);
		qaddstr(printable(slow));
		clrtoeol();
		refresh();
		if (nslow > 50)
		{
			nslow = 0;
		}
	}
	else
	{
		nslow = 0;
		prevchg = changes;
	}
#endif /* DEBUG */

	/* return the next key */
	return keybuf[next++];
}

/* This function maps or unmaps a key */
void mapkey(rawin, cooked, when, name)
	char	*rawin;	/* the input key sequence, before mapping */
	char	*cooked;/* after mapping -- or NULL to remove map */
	int	when;	/* bitmap of when mapping should happen */
	char	*name;	/* name of the key, NULL for no name, "abbr" for abbr */
{
	MAP	**head;	/* head of list of maps or abbreviations */
	MAP	*scan;	/* used for scanning through the list */
	MAP	*prev;	/* used during deletions */

	/* Is this a map or an abbreviation?  Choose the right list. */
#ifndef NO_ABBR
	head = ((!name || strcmp(name, "abbr")) ? &maps : &abbrs);
#else
	head = &maps;
#endif

	/* try to find the map in the list.  For maps, rawin must match the
	 * map's rawin; for abbreviations, the rawin may match either the
	 * abbreviation's rawin or its cooked string.
	 */
	for (scan = *head, prev = (MAP *)0;
#ifndef NO_ABBR
	     scan && (strcmp(rawin, scan->rawin) &&
			(head != &abbrs || strcmp(rawin, scan->cooked)) ||
		!(scan->flags & when & (WHEN_EX|WHEN_VICMD|WHEN_VIINP|WHEN_VIREP)));
#else
	     scan && (strcmp(rawin, scan->rawin) ||
		!(scan->flags & when & (WHEN_EX|WHEN_VICMD|WHEN_VIINP|WHEN_VIREP)));
#endif
	     prev = scan, scan = scan->next)
	{
	}

	/* trying to map? (not unmap) */
	if (cooked && *cooked)
	{
		/* if map starts with "visual ", then mark it as a visual map */
		if (head == &maps && !strncmp(cooked, "visual ", 7))
		{
			cooked += 7;
			when |= WHEN_INMV;
		}

		/* "visual" maps always work in input mode */
		if (when & WHEN_INMV)
		{
			when |= WHEN_VIINP|WHEN_VIREP|WHEN_POPUP;
		}

		/* if not already in the list, then allocate a new structure */
		if (!scan)
		{
			scan = (MAP *)malloc(sizeof(MAP));
			scan->len = strlen(rawin);
			scan->rawin = malloc((unsigned)(scan->len + 1));
			strcpy(scan->rawin, rawin);
#if 0 /* [sdw] see below */
			scan->flags = when;
#endif
			scan->label = name;
			if (*head)
			{
				prev->next = scan;
			}
			else
			{
				*head = scan;
			}
			scan->next = (MAP *)0;
		}
		else /* recycle old structure */
		{
			_free_(scan->cooked);
		}
		scan->cooked = malloc((unsigned)(strlen(cooked) + 1));
		strcpy(scan->cooked, cooked);
		/* [sdw] set flags here since WHEN_INMV status might change */
		scan->flags = when;
	}
	else /* unmapping */
	{
		/* if nothing to unmap, then exit silently */
		if (!scan)
		{
			return;
		}

		/* unlink the structure from the list */
		if (prev)
		{
			prev->next = scan->next;
		}
		else
		{
			*head = scan->next;
		}

		/* free it, and the strings that it refers to */
		_free_(scan->rawin);
		_free_(scan->cooked);
		_free_(scan);
	}
}


/* This function returns a printable version of a string.  It uses tmpblk.c */
char *printable(str)
	char	*str;	/* the string to convert */
{
	char	*build;	/* used for building the string */

	for (build = tmpblk.c; *str; str++)
	{
#if AMIGA
		if (*str == '\233')
		{
			*build++ = '<';
			*build++ = 'C';
			*build++ = 'S';
			*build++ = 'I';
			*build++ = '>';
		} else 
#endif
		if (UCHAR(*str) < ' ' || *str == '\177')
		{
			*build++ = '^';
			*build++ = *str ^ '@';
		}
		else
		{
			*build++ = *str;
		}
	}
	*build = '\0';
	return tmpblk.c;
}

/* This function displays the contents of either the map table or the
 * abbreviation table.  User commands call this function as follows:
 *	:map	dumpkey(WHEN_VICMD, FALSE);
 *	:map!	dumpkey(WHEN_VIREP|WHEN_VIINP, FALSE);
 *	:abbr	dumpkey(WHEN_VIINP|WHEN_VIREP, TRUE);
 *	:abbr!	dumpkey(WHEN_EX|WHEN_VIINP|WHEN_VIREP, TRUE);
 */
void dumpkey(when, abbr)
	int	when;	/* WHEN_XXXX of mappings to be dumped */
	int	abbr;	/* boolean: dump abbreviations instead of maps? */
{
	MAP	*scan;
	char	*str;
	int	len;

#ifndef NO_ABBR
	for (scan = (abbr ? abbrs : maps); scan; scan = scan->next)
#else
	for (scan = maps; scan; scan = scan->next)
#endif
	{
		/* skip entries that don't match "when" */
		if ((scan->flags & when) == 0)
		{
			continue;
		}

		/* dump the key label, if any */
		if (!abbr)
		{
			len = 8;
			if (scan->label)
			{
				qaddstr(scan->label);
				len -= strlen(scan->label);
			}
			do
			{
				qaddch(' ');
			} while (len-- > 0);
		}

		/* dump the rawin version */
		str = printable(scan->rawin);
		qaddstr(str);
		len = strlen(str);
		do
		{
			qaddch(' ');
		} while (len++ < 8);
			
		/* dump the mapped version */
#ifndef NO_EXTENSIONS
		if ((scan->flags & WHEN_INMV) && (when & (WHEN_VIINP|WHEN_VIREP)))
		{
			qaddstr("visual ");
		}
#endif
		str = printable(scan->cooked);
		qaddstr(str);
		addch('\n');
		exrefresh();
	}
}

#ifndef NO_MKEXRC

static void safequote(str)
	char	*str;
{
	char	*build;

	build = tmpblk.c + strlen(tmpblk.c);
	while (*str)
	{
		if (*str <= ' ' && *str >= 1 || *str == '|')
		{
			*build++ = ctrl('V');
		}
		*build++ = *str++;
	}
	*build = '\0';
}

/* This function saves the contents of either the map table or the
 * abbreviation table into a file.  Both the "bang" and "no bang" versions
 * are saved.
 *	:map	dumpkey(WHEN_VICMD, FALSE);
 *	:map!	dumpkey(WHEN_VIREP|WHEN_VIINP, FALSE);
 *	:abbr	dumpkey(WHEN_VIINP|WHEN_VIREP, TRUE);
 *	:abbr!	dumpkey(WHEN_EX|WHEN_VIINP|WHEN_VIREP, TRUE);
 */
void
savemaps(fd, abbr)
	int	fd;	/* file descriptor of an open file to write to */
	int	abbr;	/* boolean: do abbr table? (else do map table) */
{
	MAP	*scan;
	int	bang;
	int	when;

# ifndef NO_ABBR
	for (scan = (abbr ? abbrs : maps); scan; scan = scan->next)
# else
	for (scan = maps; scan; scan = scan->next)
# endif
	{
		/* skip maps that have labels, except for fkeys & abbrevs */
		if (scan->label && *scan->label != '#'
#ifndef NO_ABBR
							&& !abbr
#endif
								)
		{
			continue;
		}

		for (bang = 0; bang < 2; bang++)
		{
			/* decide which "when" flags we want */
# ifndef NO_ABBR
			if (abbr)
				when = (bang ? WHEN_EX|WHEN_VIINP|WHEN_VIREP : WHEN_VIINP|WHEN_VIREP);
			else
# endif
				when = (bang ? WHEN_VIREP|WHEN_VIINP : WHEN_VICMD);

			/* skip entries that don't match "when" */
			if ((scan->flags & when) == 0)
			{
				continue;
			}

# ifndef NO_ABBR
			/* for abbreviations, if we have "bang" version then
			 * we don't need the "no bang" version.
			 */
			if (abbr && !bang && (scan->flags & WHEN_EX) != 0)
			{
				continue;
			}
# endif

			/* write a "map" or "abbr" command name */
# ifndef NO_ABBR
			if (abbr)
				strcpy(tmpblk.c, "abbr");
			else
# endif
				strcpy(tmpblk.c, "map");

			/* maybe write a bang.  Definitely write a space */
			if (bang)
				strcat(tmpblk.c, "! ");
			else
				strcat(tmpblk.c, " ");

			/* write the rawin version */
# ifndef NO_FKEY
			if (scan->label
#  ifndef NO_ABBR
					&& !abbr
#  endif
						)
				strcat(tmpblk.c, scan->label);
			else
# endif
				safequote(scan->rawin);
			strcat(tmpblk.c, " ");
				
			/* dump the mapped version */
# ifndef NO_EXTENSIONS
			if ((scan->flags & WHEN_INMV) && (when & (WHEN_VIINP|WHEN_VIREP)))
			{
				strcat(tmpblk.c, "visual ");
			}
# endif
			safequote(scan->cooked);
			strcat(tmpblk.c, "\n");
			twrite(fd, tmpblk.c, strlen(tmpblk.c));
		}
	}
}
#endif
