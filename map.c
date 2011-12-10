/* map.c */
/* Copyright 1995 by Steve Kirkendall */

char id_map[] = "$Id: map.c,v 2.26 1996/09/21 00:01:46 steve Exp $";

#include "elvis.h"

static void trace P_((char *where));

/* This structure is used to store maps and abbreviations.  The distinction
 * between them is that maps are stored in the list referenced by the "maps"
 * pointer, while abbreviations are referenced by the "abbrs" pointer.
 */
typedef struct _map
{
	struct _map	*next;	/* another mapping */
	CHAR		*label;	/* label of the map/abbr, or NULL */
	CHAR		*rawin;	/* the "rawin" characters */
	CHAR		*cooked;/* the "cooked" characters */
	short		rawlen;	/* length of the "rawin" characters */
	short		cooklen;/* length of the "cooked" characters */
	MAPFLAGS	flags;	/* various flags */
	BOOLEAN		invoked;/* has the map been used lately? */
} MAP;

static MAP	*maps;	/* the map table */
static MAP	*abbrs;	/* the abbreviation table */



/* This function adds a map to the map table, or replaces an existing map.
 * New maps will be added to the end of the list of maps.
 *
 * "rawin" is the first argument to a ":map" command and "cooked" is the
 * second.  "rawlen" and "cooklen" are their lengths; the strings don't need
 * to be NUL terminated.  The "rawin" string may be either the actual characters
 * to be mapped, or a key label which is supported by the GUI; this function
 * will call the GUI's keylabel() function to perform the conversion.  This
 * function will allocate copies of both of these strings, so the calling
 * function can discard its own copies as soon as this function returns.
 * 
 *"label" is either NULL or a key label.  The :map command always passes NULL,
 * and the GUI's init will usually call this function with a label string.  The
 * label should be NUL-terminated, and its storage space cannot be discarded
 * after the map because this function does *NOT* make a copy of the label.
 *
 * "flags" indicates when the map should be effective.
 */
void mapinsert(rawin, rawlen, cooked, cooklen, label, flags)
	CHAR	*rawin;	/* characters sent by key */
	int	rawlen;	/* length of rawin */
	CHAR	*cooked;/* characters that the key should appear to send */
	int	cooklen;/* length of cooked */
	CHAR	*label;	/* label of the key */
	MAPFLAGS flags;	/* when the map should take effect */
{
	MAP	*scan, *lag;
	MAP	**head;	/* points to either "maps" or "abbrs" */
	int	i;
	MAPFLAGS parser;

	/* Determine whether this will be a map or abbreviation */
	head = (flags & MAP_ABBR) ? &abbrs : &maps;

	/* If the cooked string starts with "visual " then the command should
	 * always be executed as VI commands.  This also implies that the map
	 * should be defined for input mode as well.
	 */
	if (cooklen > 7 && !CHARncmp(cooked, toCHAR("visual "), 7))
	{
		flags |= (MAP_INPUT|MAP_ASCMD);
		cooklen -= 7;
		cooked += 7;
	}

	/* extract the parser bits from flags */
	parser = (flags & (MAP_INPUT|MAP_COMMAND));
	assert(parser);

	/* If more that one parser was requested, then recursively map each
	 * one separately.
	 */
	if (parser != MAP_INPUT && parser != MAP_COMMAND)
	{
		flags &= ~(MAP_INPUT|MAP_COMMAND);
		if (parser & MAP_INPUT)
			mapinsert(rawin, rawlen, cooked, cooklen, label, flags|MAP_INPUT);
		if (parser & MAP_COMMAND)
			mapinsert(rawin, rawlen, cooked, cooklen, label, flags|MAP_COMMAND);
		return;
	}

	/* In MAP_COMMAND mode, MAP_ASCMD is unnecessary */
	if (parser & MAP_COMMAND)
	{
		flags &= ~MAP_ASCMD;
	}

	/* if no label was supplied, maybe we should try to find one? */
	if (head == &maps && !label && gui->keylabel)
	{
		i = (*gui->keylabel)(rawin, rawlen, &label, &rawin);
		if (i > 0)
		{
			rawlen = i;
		}
	}

	/* see if this is already mapped */
	for (lag = NULL, scan = *head;
	     scan &&
		((scan->flags & (MAP_INPUT|MAP_COMMAND)) != parser ||
		scan->rawlen != rawlen ||
			memcmp(scan->rawin, rawin, rawlen * sizeof(CHAR)));
	     lag = scan, scan = scan->next)
	{
	}

	/* if not mapped, then create a map */
	if (!scan)
	{
		/* allocate & initialize a MAP struct */
		scan = (MAP *)safekept(1, sizeof(MAP));
		scan->label = label;
		scan->rawin = (CHAR *)safekept(rawlen, sizeof(CHAR));
		memcpy(scan->rawin, rawin, rawlen * sizeof(CHAR));
		scan->rawlen = rawlen;

		/* link it into the list of maps. */
		if (lag)
		{
			lag->next = scan;
		}
		else
		{
			*head = scan;
		}
	}
	else /* recycle an old map */
	{
		/* free the old cooked string */
		safefree(scan->cooked);
	}

	/* save a copy of the new cooked string */
	scan->cooked = safekept(cooklen, sizeof(CHAR));
	memcpy(scan->cooked, cooked, cooklen * sizeof(CHAR));
	scan->cooklen = cooklen;
	scan->flags = flags;
}

/* This function deletes a map, or changes its break flag.  It is used by the
 * :unmap, :break, and :unbreak commands.  The "rawin" string can be either
 * the label or rawin string.  Returns True if succesful, or False if the map
 * couldn't be found.
 */
BOOLEAN mapdelete(rawin, rawlen, flags, del, brk)
	CHAR	*rawin;	/* the key to be unmapped */
	int	rawlen;	/* length of rawin */
	MAPFLAGS flags;	/* when the key is mapped now */
	BOOLEAN	del;	/* delete the map? (else adjust break flag) */
	BOOLEAN	brk;	/* what to set the break flag to */
{
	MAP	*scan, *lag;
	MAP	**head;
	CHAR	*label;
	int	i;

	/* Determine whether this will be a map or abbreviation */
	head = (flags & MAP_ABBR) ? &abbrs : &maps;

	/* When unmapping, we only care about the keystroke parser bits */
	flags &= (MAP_INPUT|MAP_COMMAND);

	/* if no label was supplied, maybe we should try to find one? */
	if (head == &maps && gui->keylabel)
	{
		i = (*gui->keylabel)(rawin, rawlen, &label, &rawin);
		if (i > 0)
		{
			rawlen = i;
		}
	}

	/* see if this is already mapped */
	for (lag = NULL, scan = *head;
	     scan && (
		(scan->flags & (MAP_INPUT|MAP_COMMAND)) != flags ||
		scan->rawlen != rawlen ||
		memcmp(scan->rawin, rawin, rawlen * sizeof(CHAR)));
	     lag = scan, scan = scan->next)
	{
	}

	/* if not mapped, then fail */
	if (!scan)
	{
		return False;
	}

	/* perform the action... */
	if (del)
	{
		/* remove the map from the list of maps */
		if (lag)
		{
			lag->next = scan->next;
		}
		else
		{
			*head = scan->next;
		}
	
		/* free the map */
		safefree(scan->rawin);
		safefree(scan->cooked);
		safefree(scan);
	}
	else if (brk)
	{
		scan->flags |= MAP_BREAK;
	}
	else
	{
		scan->flags &= ~MAP_BREAK;
	}

	return True;
}


/* These two variables are used to store characters which have been read but
 * not yet parsed, and maybe not even mapped.
 */
static CHAR	queue[500];	/* the mapping queue */
static int	qty = 0;	/* number of keys in the queue */
static int	resolved = 0;	/* number of resolved keys (no mapping needed) */
static long	learning;	/* bitmap of "learn" buffers */
static CHAR	traceimg[60];	/* image of queue, for maptrace option */
static BOOLEAN	tracereal;	/* any real keys since last trace? */
static BOOLEAN	tracestep;	/* don't queue next keystroke */

/* build the trace image, and show it */
static void trace(where)
	char	*where;
{
	int	i, j, end;
	BUFFER	log;
	MARKBUF	logend;
	MARKBUF	logstart;
	CHAR	ch[1];

	/* if not tracing, then do nothing */
	if (o_maptrace == 'o')
		return;

	/* reset tracereal */
	tracereal = False;

	/* Decide how much of the queue to show. */
	for (end = qty; end >= 25; end -= 10)
	{
	}

	/* generate the image */
	for (i = j = 0; i < end; i++, j += 2)
	{
		if (iscntrl(queue[i]))
		{
			traceimg[j] = '^';
			traceimg[j + 1] = ELVCTRL(queue[i]);
		}
		else
		{
			traceimg[j] = ' ';
			traceimg[j + 1] = queue[i];
		}
	}
	traceimg[j] = '\0';

	/* show the image */
	msg(MSG_STATUS, "[sSdd]$1:($2>>50)", where, traceimg, resolved, qty);
	(void)eventdraw(windefault->gw);
	guiflush();

	/* maybe log it */
	if (o_maplog != 'o')
	{
		log = bufalloc(toCHAR(TRACE_BUF), 0);
		if (o_maplog == 'r')
		{
			bufreplace(marktmp(logstart, log, 0L), marktmp(logend, log, o_bufchars(log)), NULL, 0L);
			o_maplog = 'a';
		}
		o_internal(log) = True;
		bufreplace(marktmp(logend, log, o_bufchars(log)), &logend, toCHAR(where), (long)strlen(where));
		ch[0] = ':';
		bufreplace(marktmp(logend, log, o_bufchars(log)), &logend, ch, 1L);
		bufreplace(marktmp(logend, log, o_bufchars(log)), &logend, traceimg, (long)CHARlen(traceimg));
		ch[0] = '\n';
		bufreplace(marktmp(logend, log, o_bufchars(log)), &logend, ch, 1L);
	}

	/* maybe arrange for single-stepping to occur on next keystroke */
	if (o_maptrace == 's'
	 && !(windefault && (windefault->state->mapflags & MAP_DISABLE)))
	{
		tracestep = True;
	}
}

/* This function implements mapping.  It is called with either 1 or more new
 * characters from keypress events, or with 0 to indicate that a timeout
 * occurred.  It calls the current window's keystroke parser; we assume that
 * the windefault variable is set correctly.
 */
MAPSTATE mapdo(keys, nkeys)
	CHAR	*keys;	/* characters from the keyboard */
	int	nkeys;	/* number of keys */
{
	MAP		*scan;		/* used for scanning through maps */
	int		ambkey, ambuser;/* ambiguous key maps and user maps */
	MAP		*match;		/* longest fully matching map */
	BOOLEAN		ascmd = False;	/* did we just resolve an ASCMD map? */
	BOOLEAN		didtimeout;	/* did we timeout? */
	MAPFLAGS	now;		/* current keystroke parsing state */
	BUFFER		buf;		/* a cut buffer that is in "learn" mode */
	CHAR		cbname;		/* name of cut buffer */
	MARKBUF		mark;		/* points to the end of buf */
	int		i, j;

	assert(0 <= resolved && resolved <= qty);
	assert(windefault);

	/* if nkeys==0 then we timed out */
	didtimeout = (BOOLEAN)(nkeys == 0);

	/* tracing */
	if (!tracereal && guipoll(False))
	{
		tracereal = True;
		mapalert();
	}
	if (tracestep)
	{
		if (keys[0] == ELVCTRL('C'))
		{
			tracereal = True;
			mapalert();
		}
		else if (keys[0] == 'r')
		{
			o_maptrace = 'r';
		}
		nkeys = 0;
		tracestep = False;
	}
	if (nkeys > 0)
	{
		tracereal = True;
	}

	/* append the keys to any cutbuffer that is in "learn" mode */
	if (learning)
	{
		for (cbname = 'a'; cbname <= 'z'; cbname++)
		{
			if (learning & (1 << (cbname & 0x1f)))
			{
				buf = cutbuffer(cbname, False);
				if (buf)
				{
					bufreplace(marktmp(mark, buf,
						o_bufchars(buf)), &mark, keys, (long)nkeys);
				}
			}
		}
	}

	/* Add the new keys to the end of the queue, being careful
	 * to avoid overflow.
	 */
	while (qty < QTY(queue) && nkeys > 0)
	{
		queue[qty++] = *keys++;
		nkeys--;
	}

	/* repeatedly apply maps and then parse resolved keys */
	for (;;)
	{
		/* send any resolved keys to the current window's parser */
		if (resolved > 0)
		{
			while (resolved > 0)
			{
				/* If there aren't any more windows, stop! */
				if (!windows)
					return MAP_CLEAR;

				assert(windefault);

				/* maybe show trace */
				if (!tracereal) trace("cmd");

				/* Delete the next keystroke from the queue */
				resolved--;
				qty--;
				j = queue[0];
				for (i = 0; i < qty; i++)
				{
					queue[i] = queue[i + 1];
				}

				/* If the key is supposed to be treated as a command,
				 * then send a ^O before the keystroke.  This is a
				 * kludgy way to implement the input-mode ^O command.
				 */
				if (ascmd)
				{
					statekey(ELVCTRL('O'));
				}

				/* Make sure the MAP_DISABLE flag is turned off.
				 * Any character of the cooked string can force it
				 * on, but we only care if the *last* one forces
				 * it on.  By forcing it off before handling each
				 * cooked keystroke, we can ignore all but the last.
				 */
				windefault->state->mapflags &= ~MAP_DISABLE;

				/* Send the keystroke to the parser. */
				statekey((_CHAR_)j);

				/* if single-stepping, then we're done for now */
				if (tracestep)
				{
					return MAP_CLEAR;
				}
			}
			ascmd = False;
		}

		/* if all keys have been processed, then return MAP_CLEAR */
		if (qty == 0)
		{
			assert(resolved == 0);
			for (scan = maps; scan; scan = scan->next)
			{
				scan->invoked = False;
			}
			return MAP_CLEAR;
		}

		/* figure out what the current map context is */
		if (windefault->state->mapflags & MAP_DISABLE)
		{
			now = 0;
			windefault->state->mapflags &= ~MAP_DISABLE;
		}
		else
		{
			now = (windefault->state->mapflags & (MAP_INPUT|MAP_COMMAND|MAP_OPEN));
		}

		/* try to match the remaining keys to each map */
		ambkey = ambuser = 0;
		match = NULL;
		if (now & (MAP_INPUT|MAP_COMMAND)) /* if mapping is allowed... */
		{
			for (scan = maps; scan; scan = scan->next)
			{
				/* ignore maps for a different context */
				if ((scan->flags & now) != now)
				{
					continue;
				}

				/* is it an ambiguous (incomplete) match? */
				if (!didtimeout
				 && scan->rawlen > qty
				 && !memcmp(scan->rawin, queue, qty * sizeof(CHAR)))
				{
					/* increment an ambiguous match counter */
					if (scan->label)
						ambkey++;
					else
						ambuser++;
				}

				/* is it a complete match, and either the first such or
				 * longer than any previous complete matches?
				 */
				if (scan->rawlen <= qty
					&& !memcmp(scan->rawin, queue, scan->rawlen * sizeof(CHAR))
					&& (!match || match->rawlen < scan->rawlen))
				{
					/* remember this match */
					match = scan;
				}
			}
		}

		/* if ambiguous, then return MAP_USER or MAP_KEY */
		if (ambuser > 0)
			return MAP_USER;
		else if (ambkey > 0)
			return MAP_KEY;

		/* if any complete map, then apply it */
		if (match)
		{
			/* detect animation macros, so we can update the screen
			 * while they run.  Note that we bypass this for key
			 * maps, since an autorepeated arrow key shouldn't make
			 * us update the screen for each line scrolled.
			 */
			if (!match->label && o_optimize)
			{
				if (!match->invoked)
				{
					match->invoked = True;
				}
				else
				{
					drawimage(windefault);
					guiflush();
				}
			}

			/* maybe show the the map queue before */
			if ((match->flags & MAP_BREAK) && o_maptrace == 'r')
				o_maptrace = 's';
			trace("map");

			/* shift the contents of the queue to allow for cooked
			 * strings that are of a different length than rawin.
			 */
			if (match->rawlen < match->cooklen)
			{
				/* insert some room */
				for (i = qty + match->cooklen - match->rawlen, j = qty;
				     j > match->rawlen;
				     )
				{
					queue[--i] = queue[--j];
				}
			}
			else if (match->rawlen > match->cooklen)
			{
				/* delete some keys */
				for (i = match->cooklen, j = match->rawlen; i < qty; )
				{
					queue[i++] = queue[j++];
				}
			}
			qty += match->cooklen - match->rawlen;
			ascmd = (BOOLEAN)((match->flags & MAP_ASCMD) != 0);

			/* copy the cooked string into the queue */
			memcpy(queue, match->cooked, match->cooklen * sizeof(CHAR));

			/* if the "remap" option is off, then the cooked chars
			 * have now been resolved.
			 */
			if (!o_remap)
			{
				resolved = match->cooklen;
			}

			/* if single-stepping, then we're done for now */
			if (tracestep)
			{
				return MAP_CLEAR;
			}
		}
		else /* no matches of any kind */
		{
			/* first char is resolved: not mapped */
			resolved = 1;
		}
	}
	/*NOTREACHED*/
}


/* This function attempts to place one or more characters back into the
 * keyboard's typeahead queue.  If this would cause the typeahead queue
 * to overflow, then this function has no effect.
 */
void mapunget(keys, nkeys, remap)
	CHAR	*keys;	/* keys to stuff into the type-ahead buffer */
	int	nkeys;	/* number of keys */
	BOOLEAN	remap;	/* are the ungotten keys subject to key maps? */
{
	int	i;

	/* if this would cause overflow, then do nothing */
	if (nkeys + qty > QTY(queue))
	{
		return;
	}

	/* maybe show trace */
	trace("ung");

	/* shift old characters to make room for new characters */
	if (qty > 0)
	{
		for (i = qty; --i >= 0; )
		{
			queue[i + nkeys] = queue[i];
		}
	}

	/* copy the new characters into the queue */
	for (i = 0; i < nkeys; i++)
	{
		queue[i] = keys[i];
	}
	qty += nkeys;

	/* Should the new characters be subjected to mapping? */
	if (!remap)
	{
		/* no -- assume they're resolved */
		resolved += nkeys;
	}
	else
	{
		/* yes -- assume they're unresolved, along with any following
		 * characters.
		 */
		resolved = 0;
	}
}


/* This function is used for listing the contents of the map table in a
 * human-readable format.  Each call returns a single line of text in a
 * static CHAR array, or NULL after the last line has been output.  After
 * calling this function once, you *MUST* call it repeatedly until it
 * returns NULL.  No other map functions should be called during this time.
 */
CHAR *maplist(flags, reflen)
	MAPFLAGS flags;	/* which maps to output */
	int	 *reflen;/* where to store length, or NULL if don't care */
{
	static MAP *m;	/* used for scanning map list */
	static CHAR buf[200];
	CHAR	*scan, *build;
	int	i;

	/* find first/next map item */
	m = (m ? m->next : (flags & MAP_ABBR) ? abbrs : maps);
	flags &= ~MAP_ABBR;
	while (m && (m->flags & flags) == 0)
	{
		m = m->next;
	}

	/* if no more items, return NULL */
	if (!m)
	{
		return (CHAR *)0;
	}

	memset(buf, ' ', sizeof buf);
	if (m->label)
	{
		i = CHARlen(m->label);
		CHARncpy(buf, m->label, (size_t)(i>9 ? 9 : i));
	}
	for (scan = m->rawin, i = m->rawlen, build = &buf[10];
	     i > 0 && build < &buf[QTY(buf)-4];
	     i--, scan++)
	{
		if (*scan < ' ' || *scan == '\177')
		{
			*build++ = '^';
			*build++ = ELVCTRL(*scan);
		}
		else
		{
			*build++ = *scan;
		}
	}
	do
	{
		*build++ = ' ';
	} while (build < &buf[20]);
	if (m->flags & MAP_ASCMD)
	{
		CHARncpy(build, toCHAR("visual "), 7);
		build += 7;
	}
	for (scan = m->cooked, i = m->cooklen;
	     i > 0 && build < &buf[QTY(buf)-3];
	     i--, scan++)
	{
		if (*scan < ' ' || *scan == '\177')
		{
			*build++ = '^';
			*build++ = ELVCTRL(*scan);
		}
		else
		{
			*build++ = *scan;
		}
	}
	*build++ = '\n';
	*build = '\0';

	/* return the line */
	if (reflen)
	{
		*reflen = (long)(build - buf);
	}
	return buf;
}


/* This function causes future keystrokes to be stored in a cut buffer */
RESULT maplearn(cbname, start)
	_CHAR_	cbname;
	BOOLEAN	start;
{
	long	bit;
	MARKBUF	tmp, end;
	BUFFER	buf;
	CHAR	cmd;
	
	/* reject if not a letter */
	if (!((cbname >= 'a' && cbname <= 'z')
		|| (cbname >= 'A' && cbname <= 'Z')))
	{
		return RESULT_ERROR;
	}
	
	/* Set/reset the "learn" bit for the named cut buffer.  Note that we
	 * return RESULT_COMPLETE if you stop recording keystrokes on a buffer
	 * that wasn't recording to begin with.
	 */
	bit = (1 << (cbname & 0x1f));
	if (start)
		learning |= bit;
	else if (!(learning & bit))
		return RESULT_COMPLETE;
	else
		learning ^= bit;
	
	/* If we're starting and the cut buffer name is lowercase,
	 * then we need to reset the cut buffer to 0 characters
	 */
	if (start && islower(cbname))
	{
		/* reset the cut buffer to zero characters */
		cutyank(cbname, marktmp(tmp, bufdefault, 0), &tmp, 'c', False);
	}

	/* If we're ending and the last two characters were "]a" or "@a" (or
	 * whatever the buffer name was) then delete the last two characters.
	 */
	if (!start)
	{
		buf = cutbuffer(cbname, False);
		if (!buf)
			return RESULT_ERROR;
		if (o_bufchars(buf) >= CUT_TYPELEN + 2
		 && scanchar(marktmp(tmp, buf, o_bufchars(buf) - 1)) == cbname
		 && ((cmd = scanchar(marktmp(tmp, buf, o_bufchars(buf) - 2))) == ']'
			|| cmd == '@'))
		{
			bufreplace(marktmp(tmp, buf, o_bufchars(buf) - 2),
				   marktmp(end, buf, o_bufchars(buf)), NULL, 0);
		}
	}

	return RESULT_COMPLETE;
}


/* This function returns a character indicating the current learn state.
 * This will be ',' if no cut buffers are in learn mode, or the name of
 * the first buffer which is in learn mode.
 */
CHAR maplrnchar(dflt)
	_CHAR_	dflt;
{
	CHAR	cbname;

	if (!learning)
		return dflt;
	for (cbname = 'a'; (learning & (1 << (cbname & 0x1f))) == 0; cbname++)
	{
		assert(cbname < 'z');
	}
	return cbname;
}


/* This function implements a POSIX "terminal alert."  This involves discarding
 * any pending keytrokes, and aborting @ macros and maps.  And then the GUI's
 * bell must be rung.
 */
void mapalert()
{
	/* maybe display log info */
	if (!tracereal) trace(":::");

	/* cancel all pending key states, etc. */
	qty = resolved = learning = 0;
}


CHAR *mapabbr(bkwd, oldptr, newptr, exline)
	CHAR	*bkwd;	/* possible abbreviation, BACKWARDS */
	long	*oldptr;/* where to store the length of short form */
	long	*newptr;/* where to store the length of long form */
	BOOLEAN	exline;	/* inputting an ex command line? (else normal input) */
{
	MAP	*m;	/* used for scanning the abbr list */
	MAP	*match;	/* longest matching abbreviation */
	int	i, j;

	/* compare against all abbreviations */
	match = NULL;
	for (m = abbrs; m; m = m->next)
	{
		/* Skip this abbr if it is for the wrong context */
		if ((m->flags & MAP_INPUT) == (exline ? MAP_INPUT : 0))
		{
			continue;
		}

		/* Compare each character.  This is a little tricky since the
		 * input word is backwards.
		 */
		for (i = 0, j = m->rawlen - 1;
		     j >= 0 && bkwd[i] == m->rawin[j]; i++, j--)
		{
		}
		
		/* If all characters matched, and the preceding character in the
		 * raw text wasn't alphanumeric, then we have a match.  If this
		 * match is longer than any previous match, then remember it.
		 */
		if (j < 0
		 && !isalnum(bkwd[i])
		 && (!match || match->rawlen < i))
		{
			match = m;
		}
	}

	/* If we found a match, return it. */
	if (match)
	{
		*oldptr = match->rawlen;
		*newptr = match->cooklen;
		return match->cooked;
	}
	return NULL;
}

#ifndef NO_MKEXRC
/* This function is used for saving the current map table as a series of
 * :map commands.  It is used by the :mkexrc command.
 */
void mapsave(buf)
	BUFFER	buf;	/* the buffer to append to */
{
	MARKBUF	append;	/* where to put the command */
	static MAP *m;	/* used for scanning map list */
	static CHAR text[200];
	long	len;
	CHAR	*scan;
	int	i;

	(void)marktmp(append, buf, o_bufchars(buf));

	/* for each map... */
	for (m = maps; m; m = m->next)
	{
		/* if for a GUI-specific key, ignore it */
		if (m->label && *m->label != '#')
		{
			continue;
		}

		/* construct a "map" command */
		CHARcpy(text, toCHAR("map"));
		len = 3;
		if ((m->flags & MAP_INPUT) != 0)
		{
			text[len++] = '!';
		}
		text[len++] = ' ';

		/* Append the raw code.  Use function label if possible */
		if (m->label)
		{
			CHARcpy(&text[len], m->label);
			len += CHARlen(m->label);
		}
		else
		{
			for (scan = m->rawin, i = m->rawlen;
			     i > 0 && len < QTY(text) - 4;
			     i--, scan++)
			{
				if (*scan == ' ' || *scan == '\t' || *scan == '|')
				{
					text[len++] = ELVCTRL('V');
				}
				text[len++] = *scan;
			}
		}
		text[len++] = ' ';

		/* construct the "cooked" string */
		if (m->flags & MAP_ASCMD)
		{
			CHARncpy(&text[len], toCHAR("visual "), 7);
			len += 7;
		}
		for (scan = m->cooked, i = m->cooklen;
		     i > 0 && len < QTY(text) - 3;
		     i--, scan++)
		{
			if (*scan == '|')
			{
				text[len++] = ELVCTRL('V');
			}
			text[len++] = *scan;
		}
		text[len++] = '\n';

		/* append the command to the buffer */
		bufreplace(&append, &append, text, len);
		markaddoffset(&append, len);
	}
}
#endif /* not NO_MKEXRC */
