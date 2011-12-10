/* dmnormal.c */
/* Copyright 1995 by Steve Kirkendall */

char id_dmnormal[] = "$Id: dmnormal.c,v 2.29 1996/09/21 01:21:36 steve Exp $";

#include "elvis.h"

#if USE_PROTOTYPES
static DMINFO *init(WINDOW win);
static void term(DMINFO *info);
static MARK move(WINDOW w, MARK from, long linedelta, long column, BOOLEAN cmd);
static MARK wordmove(MARK cursor, long count, BOOLEAN backward, BOOLEAN whitespace);
static long mark2col(WINDOW w, MARK mark, BOOLEAN cmd);
static MARK setup(MARK top, long cursor, MARK bottom, DMINFO *info);
static MARK image(WINDOW w, MARK line, DMINFO *info, void (*draw)(CHAR *p, long qty, _char_ font, long offset));
static void indent(WINDOW w, MARK line, long linedelta);
static CHAR *tagatcursor(WINDOW win, MARK cursor);
static MARK tagload(CHAR *tagname, MARK from);
#endif

/* start the mode, and allocate modeinfo */
static DMINFO *init(win)
	WINDOW	win;
{
	return NULL;
}

/* end the mode, and free the modeinfo */
static void term(info)
	DMINFO	*info;	/* window-specific info about mode */
{
}

/* Move vertically, and to a given column (or as close to column as possible) */
static MARK move(w, from, linedelta, column, cmd)
	WINDOW	w;		/* window where buffer is shown */
	MARK	from;		/* old position */
	long	linedelta;	/* change in line number */
	long	column;		/* desired column */
	BOOLEAN	cmd;		/* if True, we're in command mode; else input mode */
{
	static MARKBUF	tmp;
	long	col, lnum;
	long	offset;
	CHAR	*cp;

	assert(w != NULL || column == 0);

	/* move forward/back to the start of the line + linedelta */
	lnum = markline(from) + linedelta;
	if (lnum < 1)
		lnum = 1;
	else if (lnum > o_buflines(markbuffer(from)))
		lnum = o_buflines(markbuffer(from));
	offset = lowline(bufbufinfo(markbuffer(from)), lnum);

	/* now move to the left far enough to find the desired column */
	(void)scanalloc(&cp, marktmp(tmp, markbuffer(from), offset));
	for (col = 0; w && cp && *cp != '\n' && col <= column; offset++, scannext(&cp))
	{
		/* add the width of this character */
		if (*cp == '\t' && (!o_list(w) || w->state->acton))
		{
			col = col + o_tabstop(markbuffer(w->cursor)) - (col % o_tabstop(markbuffer(w->cursor)));
		}
		else if (*cp < ' ' || *cp == 127)
		{
			col += 2;
		}
		else
		{
			col++;
		}
	}

	/* The above loop normally exits when we've PASSED the desired column,
	 * so we normally want to back up one character.  Two important
	 * exceptions: In input mode, if we break out of that loop because we
	 * hit '\n' then we want to leave the cursor on the '\n' character.
	 * If we hit the end of the buffer, then we want to leave the cursor
	 * on the last character of the buffer but we also need to be careful
	 * about empty buffers.
	 */
	if (col > 0 && !(!cmd && (!cp || *cp == '\n') && col <= column))
	{
		offset--;
	}

	/* return the mark */
	scanfree(&cp);
	return marktmp(tmp, markbuffer(from), offset);
}


/* Convert a mark to a column number */
static long mark2col(w, mark, cmd)
	WINDOW	w;	/* window where buffer is shown */
	MARK	mark;	/* mark to be converted */
	BOOLEAN	cmd;	/* if True, we're in command mode; else input mode */
{
	long	col;
	CHAR	*cp;
	MARK	front;
	long	nchars;

	/* if the buffer is empty, the column must be 0 */
	if (o_bufchars(markbuffer(mark)) == 0)
	{
		return 0;
	}

	/* find the front of the line */
	front = move(w, mark, 0, 0, cmd);
	nchars = markoffset(mark) - markoffset(front);

	/* in command mode, we leave the cursor on the last cell of any
	 * wide characters such as tabs.  To accomplish this, we'll find
	 * the column of the following character, and then subtract 1.
	 */
	if (cmd)
	{
		nchars++;
	}

	/* count character widths until we find the requested mark */
	for (scanalloc(&cp, front), col = 0; cp && nchars > 0; nchars--, scannext(&cp))
	{
		if (*cp == '\t' && (!o_list(w) || w->state->acton))
		{
			col = col + o_tabstop(markbuffer(w->cursor)) - (col % o_tabstop(markbuffer(w->cursor)));
		}
		else if (*cp == '\n')
		{
#if 1
			col++;
#else
			/* no change to col */
#endif
		}
		else if (*cp < ' ' || *cp == 127)
		{
			col += 2;
		}
		else
		{
			col++;
		}
	}
	scanfree(&cp);

	/* the other half of the "cmd" hack */
	if (cmd && col > 0)
	{
		col--;
	}

	return col;
}


/* This function implements most of the logic for the visual <b>, <e>, and
 * <w> commands.  If it succedes, it adjusts the starting mark and returns
 * it; if it fails, it returns NULL and leaves the starting mark unchanged.
 */
static MARK wordmove(cursor, count, backward, whitespace)
	MARK	cursor;		/* starting position */
	long	count;		/* number of words to move by */
	BOOLEAN	backward;	/* if True, move backward; else forward */
	BOOLEAN	whitespace;	/* if True, trailing whitespace is included */
{
	BOOLEAN inword, inpunct;
	CHAR	*cp;
	long	offset;
	long	end;

	/* start the scan */
	scanalloc(&cp, cursor);
	offset = markoffset(cursor);
	assert(cp != NULL);
	end = o_bufchars(markbuffer(cursor));

	/* figure out if we're in the middle of a word */
	if (backward || !whitespace || isspace(*cp))
	{
		inword = inpunct = False;
	}
	else if (isalnum(*cp) || *cp == '_')
	{
		inword = True;
		inpunct = False;
	}
	else
	{
		inword = False;
		inpunct = True;
	}

	/* continue... */
	if (backward)
	{
		/* move backward until we hit the top of the buffer, or
		 * the start of the desired word.
		 */
		while (count > 0 && offset > 0)
		{
			scanprev(&cp);
			assert(cp != NULL);
			if (isspace(*cp))
			{
				if (inword || inpunct)
				{
					count--;
				}
				inpunct = inword = False;
			}
			else if (isalnum(*cp) || *cp == '_')
			{
				if (inpunct)
				{
					count--;
				}
				inword = True;
				inpunct = False;
			}
			else
			{
				if (inword)
				{
					count--;
				}
				inword = False;
				inpunct = True;
			}
			if (count > 0)
			{
				offset--;
			}
		}

		/* if we hit offset==0 and were in a word, we found the start
		 * of the first word.  Count it here.
		 */
		if (offset == 0 && (inword || inpunct))
		{
			count--;
		}
	}
	else
	{
		/* move forward until we hit the end of the buffer, or
		 * the start of the desired word.
		 */
		while (count > 0 && offset < end - 1)
		{
			scannext(&cp);
			assert(cp != NULL);
			if (isspace(*cp))
			{
				if ((inword || inpunct) && !whitespace)
				{
					count--;
				}
				inword = inpunct = False;
				if (count > 0)
				{
					offset++;
				}
			}
			else if (isalnum(*cp) || *cp == '_')
			{
				if ((!inword && whitespace) || inpunct)
				{
					count--;
				}
				inword = True;
				inpunct = False;
				if (count > 0 || whitespace)
				{
					offset++;
				}
			}
			else
			{
				if ((!inpunct && whitespace) || inword)
				{
					count--;
				}
				inword = False;
				inpunct = True;
				if (count > 0 || whitespace)
				{
					offset++;
				}
			}
		}
	}

	/* cleanup */
	scanfree(&cp);

	/* If we were moving forward and had only one more word to go, then
	 * move the cursor to the last character in the buffer -- usually a
	 * newline character.  Pretend the count was decremented to 0.
	 */
	if (count == 1 && !backward && whitespace && end > 0)
	{
		offset = end - 1;
		count = 0;
	}

	/* if the count didn't reach 0, we failed */
	if (count > 0)
	{
		return NULL;
	}

	/* else set the cursor's offset */
	assert(offset < end && offset >= 0);
	marksetoffset(cursor, offset);
	return cursor;
}

/* Choose a line to appear at the top of the screen, and return its mark.
 * Also, initialize the info for the next line.
 */
static MARK setup(top, cursor, bottom, info)
	MARK	top;	/* where previous image started */
	long	cursor;	/* offset of cursor position */
	MARK	bottom;	/* where previous image ended */
	DMINFO	*info;	/* window-specific info about mode */
{
	static MARKBUF tmp;
	long	topoff;
	long	bottomoff;
	long	other;
	long	i;

	/* if the cursor is still on the screen (or very near the bottom)
	 * then use the same top.
	 */
	topoff = markoffset(move((WINDOW)0, top, 0, 0, True));
	bottomoff = markoffset(move((WINDOW)0, bottom, o_nearscroll, 0, True));
	if (cursor >= topoff && (cursor < bottomoff || bottomoff < markoffset(bottom) + 1))
	{
		return marktmp(tmp, markbuffer(top), topoff);
	}

	/* if the cursor is on the line before the top, then scroll back */
	if (topoff > 0)
	{
		for (i = 1; i < o_nearscroll; i++)
		{
			other = markoffset(move((WINDOW)0, top, -i, 0, True));
			if (cursor >= other && cursor < topoff)
			{
				return marktmp(tmp, markbuffer(top), other);
			}
		}
	}

	/* else try to center the line in the window */
	other = cursor - (bottomoff - topoff) / 2;
	if (other < 0)
	{
		other = 0;
	}
	other = markoffset(move((WINDOW)0, marktmp(tmp, markbuffer(top), other), 0, 0, True));
	return marktmp(tmp, markbuffer(top), other);
}

static MARK image(w, line, info, draw)
	WINDOW	w;		/* window where drawing will go */
	MARK	line;		/* start of line to draw next */
	DMINFO	*info;		/* window-specific info about mode */
	void	(*draw)P_((CHAR *p, long qty, _char_ font, long offset));
				/* function for drawing a single character */
{
	int	col;
	CHAR	*cp;
	CHAR	tmpchar;
	long	offset;
	static MARKBUF tmp;
	long	startoffset;	/* offset of first contiguous normal char */
	int	qty;		/* number of contiguous normal chars */
	CHAR	buf[100];	/* buffer, holds the contiguous normal chars */
	int	i;

	/* initialize startoffset just to silence a compiler warning */
	startoffset = 0;

	/* for each character in the line... */
	qty = 0;
	for (col = 0, offset = markoffset(line), scanalloc(&cp, line); cp && *cp != '\n'; offset++, scannext(&cp))
	{
		/* some characters are handled specially */
		if (*cp == '\f' && markoffset(w->cursor) == o_bufchars(markbuffer(w->cursor)))
		{
			/* when printing, a formfeed ends the line (and page) */
			break;
		}
		else if (*cp == '\t' && (!o_list(w) || w->state->acton))
		{
			/* output any preceding normal characters */
			if (qty > 0)
			{
				(*draw)(buf, qty, 'n', startoffset);
				qty = 0;
			}

			/* display the tab character as a bunch of spaces */
			i = o_tabstop(markbuffer(w->cursor));
			i -= (col % i);
			tmpchar = ' ';
			(*draw)(&tmpchar, -i, 'n', offset);
			col += i;
		}
		else if (*cp < ' ' || *cp == 127)
		{
			/* output any preceding normal characters */
			if (qty > 0)
			{
				(*draw)(buf, qty, 'n', startoffset);
				qty = 0;
			}

			/* control characters */
			tmpchar = '^';
			(*draw)(&tmpchar, 1, 'n', offset);
			tmpchar = *cp ^ 0x40;
			(*draw)(&tmpchar, 1, 'n', offset);
			col += 2;
		}
		else
		{
			/* starting a new string of contiguous normal chars? */
			if (qty == 0)
			{
				startoffset = offset;
			}

			/* add this char to the string */
			buf[qty++] = *cp;
			col++;

			/* if buf[] is full, flush it now */
			if (qty == QTY(buf))
			{
				(*draw)(buf, qty, 'n', startoffset);
				qty = 0;
			}
		}
	}

	/* output any normal chars from the end of the line */
	if (qty > 0)
	{
		(*draw)(buf, qty, 'n', startoffset);
		qty = 0;
	}

	/* end the line */
	if (o_list(w) && !w->state->acton && *cp == '\n')
	{
		(*draw)(toCHAR("$"), 1, 'n', offset);
	}
	(*draw)(cp ? cp : toCHAR("\n"), 1, 'n', offset);
	if (cp)
	{
		offset++;
	}
	else
	{
		offset = o_bufchars(markbuffer(w->cursor));
	}
	scanfree(&cp);
	return marktmp(tmp, markbuffer(w->cursor), offset);
}

/* This function implements autoindent.  Given the MARK of a newly created
 * line, insert a copy of the indentation from another line.  The line whose
 * indentation is to be copied is specified as a line delta.  Usually, this
 * will be -1 so the new line has the same indentation as a previous line.
 * The <Shift-O> command uses a linedelta of 1 so the new line will have the
 * same indentation as the following line.
 */
static void indent(w, line, linedelta)
	WINDOW	w;		/* windows whose options are used */
	MARK	line;		/* new line to adjust */
	long	linedelta;	/* -1 to copy from previous line, etc. */
{
	MARKBUF	from, to;	/* bounds of whitespace in source line */
	MARKBUF	bline;		/* copy of the "line" argument */
	CHAR	*cp;		/* used for scanning whitespace */
	BOOLEAN	srcblank;	/* is source indentation from a blank line? */

	assert(o_autoindent(markbuffer(line)));
	assert(markbuffer(w->cursor) == markbuffer(line));
	/*assert(scanchar(w->cursor) == '\n');*/

	/* find the start of the source line */
	bline = *line;
	from = *dispmove(w, linedelta, 0);
	if (markoffset(&from) == markoffset(&bline))
	{
		/* can't find source line -- at edge of buffer, maybe? */
		return;
	}

	/* find the end of the source line's whitespace */
	for (scanalloc(&cp, &from); cp && (*cp == ' ' || *cp == '\t'); scannext(&cp))
	{
	}
	if (!cp)
	{
		/* hit end of buffer without finding end of line -- do nothing */
		scanfree(&cp);
		return;
	}
	to = *scanmark(&cp);
	srcblank = (BOOLEAN)(*cp == '\n');
	scanfree(&cp);

	if (markoffset(&from) != markoffset(&to))
	{
		/* copy the source whitespace into the new line */
		bufpaste(&bline, &from, &to);
	
		/* if the source line was blank, then delete its whitespace */
		if (srcblank)
		{
			if (linedelta > 0L)
			{
				/* tweak from & to, due to bufpaste() */
				markaddoffset(&to, markoffset(&to) - markoffset(&from));
				marksetoffset(&to, markoffset(&from));
			}
			else
			{
				/* tweak bline for the following bufreplace() */
				markaddoffset(&bline, markoffset(&from) - markoffset(&to));
			}
			bufreplace(&from, &to, NULL, 0L);
		}

		/* tweak the argument mark, to leave cursor after whitespace */
		marksetoffset(line, markoffset(&bline) + markoffset(&to) - markoffset(&from));
		bline = *line;
	}

	/* if the line had some other indentation before, then delete that */
	for (scanalloc(&cp, line); cp && (*cp == ' ' || *cp == '\t'); scannext(&cp))
	{
	}
	if (cp)
	{
		bufreplace(&bline, scanmark(&cp), NULL, 0);
	}
	scanfree(&cp);
}


/* Return a dynamically-allocated string containing the name of the tag at the
 * cursor, or NULL if the cursor isn't on a tag.
 */
static CHAR *tagatcursor(win, cursor)
	WINDOW	win;
	MARK	cursor;
{
	MARKBUF	curscopy;	/* a copy of the cursor */
	MARK	word;		/* mark for the front of the word */

	/* find the ends of the word */
	curscopy = *cursor;
	word = wordatcursor(&curscopy);

	/* if not on a word, then return NULL */
	if (!word)
		return NULL;

	/* copy the word into RAM, and return it */
	return bufmemory(word, &curscopy);
}


/* Lookup a tag name, load the file where that tag was defined, and return
 * the MARK of its position within that buffer.  If the tag can't be found
 * or loaded for any reason, then issue an error message and return NULL.
 */
static MARK tagload(tagname, from)
	CHAR	*tagname;	/* name of tag to lookup */
	MARK	from;		/* initial position of the cursor */
{
 static MARKBUF	retmark;	/* the return value */
	BUFFER	buf;		/* the buffer containing the tag */
	CHAR	tagline[1000];	/* a line from the tags file */
	int	bytes;		/* number of bytes in I/O buffer */
	char	*pathname;	/* full pathname of current "tags" file */
	int	len;		/* significant length of tagname */
	BOOLEAN	fulllen;	/* len is the full length of the tag */
	BOOLEAN	allnext;	/* is the whole next line in the buffer? */
	char	*loadname;	/* name of file to load */
	EXINFO	xinfb;		/* dummy ex command, for parsing tag address */
	BOOLEAN	wasmagic;	/* stores the normal value of o_magic */
	CHAR	*src, *dst;

	/* find the significant length of tagname */
	len = CHARlen(tagname);
	fulllen = True;
	if (len > o_taglength && o_taglength > 0)
	{
		len = o_taglength;
		fulllen = False;
	}

	/* for each tags file named in the "tags" option... */
	for (pathname = iopath(tochar8(o_tags), "tags", True);
	     pathname;
	     pathname = iopath(NULL, "tags", True))
	{
		/* open the tags file */
		if (!ioopen(pathname, 'r', False, False, False))
		{
			return NULL;
		}

		/* Compare the tag of each line against the tagname */
		bytes = ioread(tagline, QTY(tagline) - 1);
		while (bytes > len
			&& (CHARncmp(tagname, tagline, (size_t)len)
				|| (fulllen && !isspace(tagline[len]))))
		{
			/* delete this line from tagline[] */
			for (src = tagline; src < &tagline[bytes] && *src != '\n'; src++)
			{
			}
			for (dst = tagline, src++, allnext = False; src < &tagline[bytes]; )
			{
				if (*src == '\n')
					allnext = True;
				*dst++ = *src++;
			}
			bytes = (int)(dst - tagline);

			/* if the next line is incomplete, read some more text
			 * from the tags file.
			 */
			if (!allnext || bytes <= len)
			{
				bytes += ioread(dst, (int)QTY(tagline) - bytes - 1);
			}
		}
		(void)ioclose();

		/* did we find the tag? */
		if (bytes > len)
		{
			goto Found;
		}
	}
	msg(MSG_ERROR, "[S]tag $1 not found", tagname);
	return NULL;

Found:
	/* skip past the tagname to find the start of the filename */
	for (src = tagline; src < &tagline[bytes] && !isspace(*src); src++)
	{
	}
	for ( ; src < &tagline[bytes] && isspace(*src); src++)
	{
	}

	/* locate the end of the filename, and mark it with a NUL byte */
	for (dst = src; dst < &tagline[bytes] && !isspace(*dst); dst++)
	{
	}
	*dst++ = '\0';

	/* if the filename isn't a full pathname, then assume it is in the
	 * same directory as the "tags" file.
	 */
	if (!isalnum(*src))
	{
		loadname = tochar8(src);
	}
	else
	{
		loadname = dirpath(dirdir(pathname), tochar8(src));
	}

	/* find a buffer containing the file, or load the file into a buffer */
	buf = buffind(toCHAR(loadname));
	if (!buf)
	{
		if (dirperm(loadname) == DIR_NEW)
		{
			msg(MSG_ERROR, "[s]$1 doesn't exist", loadname);
			return NULL;
		}
		buf = bufload(NULL, loadname, False);
		if (!buf)
		{
			/* bufload() already gave error message */
			return NULL;
		}
	}
	if (o_bufchars(buf) == 0)
	{
		goto NotFound;
	}

	/* locate the tag address field */
	for (src = dst; src < &tagline[bytes] && isspace(*src); src++)
	{
	}
	for (dst = src; dst < &tagline[bytes] && *dst != '\n'; dst++)
	{
	}
	*dst = '\0';

	/* convert the tag address into a line number */
	scanstring(&dst, src);
	memset((char *)&xinfb, 0, sizeof xinfb);
	(void)marktmp(xinfb.defaddr, buf, 0);
	wasmagic = o_magic;
	o_magic = False;
	if (!exparseaddress(&dst, &xinfb))
	{
		scanfree(&dst);
		o_magic = wasmagic;
		goto NotFound;
	}
	scanfree(&dst);
	o_magic = wasmagic;
	(void)marktmp(retmark, buf, lowline(bufbufinfo(buf), xinfb.to));
	exfree(&xinfb);

	return &retmark;

NotFound:
	msg(MSG_WARNING, "tag address out of date");
	return marktmp(retmark, buf, 0L);
}


DISPMODE dmnormal =
{
	"normal",
	"Standard vi",
	True,	/* display generating can be optimized */
	True,	/* should use normal wordwrap */
	0,	/* no window options */
	NULL,	/* no descriptions of window options */
	0,	/* no global options */
	NULL,	/* no descriptions of global options */
	NULL,	/* no values of global options */
	init,
	term,
	mark2col,
	move,
	wordmove,
	setup,
	image,
	NULL,	/* doesn't need a header */
	indent,
	tagatcursor,
	tagload,
	NULL	/* no tagnext() function */
};
