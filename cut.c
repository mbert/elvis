/* cut.c */
/* Copyright 1995 by Steve Kirkendall */

char id_cut[] = "$Id: cut.c,v 2.24 1996/06/28 01:33:43 steve Exp $";

#include "elvis.h"
#if USE_PROTOTYPES
static void shiftbufs(void);
#endif



/* This is the name of the most recently named buffer.  It is used to
 * implement the "" and "@ buffer names, and for incrementing "1 (etc.)
 * when pasting from numbered cut buffers.
 */
static CHAR previous;

/* This function locates or creates the BUFFER used for storing the contents
 * of a given cut buffer.  "cbname" is the single-character name of the cut
 * buffer.
 *
 * The cutyank() and cutput() functions both use this function to locate the
 * buffer, and then perform other name-dependent operations to determine how
 * the buffer should be used.  For example, 'a' and 'A' both refer to the same
 * buffer here, but cutyank() will treat them differently.
 */
BUFFER cutbuffer(cbname, create)
	_CHAR_	cbname;	/* name of cut buffer, or '\0' for anonymous */
	BOOLEAN	create;	/* create the edit buffer if it doesn't already exist? */
{
	char	tmpname[50];
	char	*bufname;
	BUFFER	buf;

	/* handle the "" buffer */
	if (cbname == '"' || cbname == '@')
	{
		if (!previous)
		{
			msg(MSG_ERROR, "no previous cut buffer");
			return NULL;
		}
		cbname = previous;
	}

	switch (cbname)
	{
	  case '\0':
		bufname = CUTANON_BUF;
		break;

	  case '<':
	  case '>':
		bufname = CUTEXTERN_BUF;
		break;

	  case '.':
		bufname = CUTINPUT_BUF;
		break;

	  default:
		if ((cbname >= '1' && cbname <= '9') || islower(cbname))
		{
			sprintf(tmpname, CUTNAMED_BUF, cbname);
			bufname = tmpname;
		}
		else if (isupper(cbname))
		{
			sprintf(tmpname, CUTNAMED_BUF, tolower((char)cbname));
			bufname = tmpname;
		}
		else
		{
			msg(MSG_ERROR, "[C]bad cutbuf $1", cbname);
			return NULL;
		}
	}

	/* find the buffer, or create it */
	previous = cbname;
	buf = (create ? bufalloc(toCHAR(bufname), 0) : buffind(toCHAR(bufname)));
	if (buf)
		o_internal(buf) = True;
	return buf;
}


/* This function shifts the numbered cut buffers by renaming them. */
static void shiftbufs()
{
	CHAR	cbname;	/* buffer currently being considered. */
	BUFFER	buf;	/* the edit buffer used to store a cut buffer's contents */
	char	tmpname[50];

	/* We would like to delete "9 after this, but if it has any marks
	 * referring to it then we must leave it, and delete "8 instead.
	 * But "8 may have marks, forcing us to leave it too... search back
	 * until we find a buffer we can delete.
	 */
	for (cbname = '9'; cbname > '1'; cbname--)
	{
		/* Try to find the buffer.  If it doesn't exist then we
		 * won't really need to delete ANY numbered cut buffer!
		 */
		buf = cutbuffer(cbname, False);
		if (!buf)
			break;

		/* If any marks refer to this buffer, then we can't
		 * delete this buffer.
		 */
		if (buf->marks)
			continue;

		/* Okay, this is the one!  Delete it and break out of loop */
		buffree(buf);
		break;
	}

	/* shift the lower-numbered buffers by renaming them */
	while (cbname > '1')
	{
		/* generate the name new name that the buffer should have */
		sprintf(tmpname, CUTNAMED_BUF, cbname);

		/* find the preceding-numbered buffer */
		cbname--;
		buf = cutbuffer(cbname, False);

		/* if the buffer exists, rename it one number higher */
		if (buf)
		{
			buftitle(buf, toCHAR(tmpname));
		}
	}

	/* At this point, the buffers have been shifted and there probably
	 * is no "1 buffer.  The only way there could be a "1 buffer would be
	 * if every cut buffer from "1 to "9 was referred to by a mark and
	 * therefore undeleteable.  Even this case should be safe, though,
	 * since the cutyank() function will just replace the old contents
	 * of "1 with the new contents, causing the marks to be adjusted...
	 * to safe (though probably useless) offsets.
	 */
}


/* This function copies text between two marks into a cut buffer.  "cbname"
 * is the single-character name of the cut buffer.  "from" and "to" delimit
 * the source of the text.  "type" is 'c' for character cuts, 'l' for line
 * cuts, and 'r' for rectangular cuts; for rectangular cuts only, the left
 * and right limits are taken from the current window.
 *
 * "type" can also be 'L' for line-mode cuts which come from visual command
 * mode operators.  This is different from 'l' in that 'L' boundaries have
 * already been adjusted to match line boundaries, but for 'l' the cutyank()
 * function will need to adjust the boundaries itself.
 */
void cutyank(cbname, from, to, type, del)
	_CHAR_	cbname;	/* name of cut buffer to yank into */
	MARK	from;	/* start of source */
	MARK	to;	/* end of source */
	_CHAR_	type;	/* yank style: c=character, l=line, r=rectangle */
	BOOLEAN	del;	/* if True, the source text is deleted after it is yanked */
{
	BUFFER	dest;		/* cut buffer we're writing into */
	MARKBUF	dfrom, dto;	/* region of destination buffer */
	MARKBUF	sfrom, sto;	/* region of source buffer */
	MARK	line;		/* end of current line, when type='r' */
	long	prevline;	/* used for detecting failed move of "line" */
	long	origlines;	/* number of lines in cut buffer before yank */
	CHAR	*cp;

	assert(markbuffer(from) == markbuffer(to) && markoffset(from) <= markoffset(to));
	assert(type == 'c' || type == 'l' || type == 'r' || type == 'L');

	/* if yanking into the anonymous cut buffer, then shift numbered */
	if (!cbname)
		shiftbufs();

	/* If this is a character-mode cut, and both ends happen to be the
	 * start of lines, then treat this as a line-mode cut.  Note that
	 * we really should know what display mode is being used, but that
	 * wasn't passed as an argument so we'll have to fudge it a little.
	 */
	if (type == 'c')
	{
		if (windefault && markbuffer(from) == markbuffer(windefault->cursor))
		{
			if (markoffset((*windefault->md->move)(windefault, from, 0L, 0L, True)) == markoffset(from)
			 && (markoffset(to) == o_bufchars(markbuffer(to))
				|| markoffset((*windefault->md->move)(windefault, to, 0L, 0L, True)) == markoffset(to)))
			{
				type = 'L';
			}
		}
		else
		{
			if (markoffset((*dmnormal.move)(windefault, from, 0L, 0L, True)) == markoffset(from)
			 && (markoffset(to) == o_bufchars(markbuffer(to))
				|| markoffset((*dmnormal.move)(windefault, to, 0L, 0L, True)) == markoffset(to)))
			{
				type = 'L';
			}
		}
	}

	/* find the cut buffer */
	dest = cutbuffer(cbname, True);
	if (!dest)
	{
		return;
	}

	/* discard the old contents, unless we want to append */
	if (!isupper(cbname))
	{
		(void)marktmp(dfrom, dest, 0);
		(void)marktmp(dto, dest, o_bufchars(dest));
		switch (type)
		{
		  case 'c': bufreplace(&dfrom, &dto, toCHAR("character\n"), CUT_TYPELEN); break;
		  case 'L':
		  case 'l': bufreplace(&dfrom, &dto, toCHAR("line     \n"), CUT_TYPELEN); break;
		  case 'r': bufreplace(&dfrom, &dto, toCHAR("rectangle\n"), CUT_TYPELEN); break;
		}
		origlines = 1;
	}
	else
	{
		(void)marktmp(dfrom, dest, o_bufchars(dest));
		origlines = o_buflines(dest);
	}

	/* copy the text into the buffer. */
	(void)marktmp(dfrom, dest, o_bufchars(dest));
	switch (type)
	{
	  case 'c':
		bufpaste(&dfrom, from, to);
		if (del)
		{
			bufreplace(from, to, NULL, 0);
		}
		break;

	  case 'l':
		sfrom = *(*dmnormal.move)(windefault, from, 0, 0, True);
		markaddoffset(to, -1);
		sto = *(*dmnormal.move)(windefault, to, 1, INFINITY, True);
		markaddoffset(&sto, 1);
		bufpaste(&dfrom, &sfrom, &sto);
		if (del)
		{
			bufreplace(&sfrom, &sto, NULL, 0);
		}
		break;

	  case 'L':
		bufpaste(&dfrom, from, to);
		if (del)
		{
			bufreplace(from, to, NULL, 0);
		}
		break;

	  case 'r':
		/* NOTE: the only way to yank a rectangle is by visibly
		 * selecting it.  So we know that we're yanking from the
		 * current window, and can find the left & right limits
		 * there, and use the window's edit mode to determine how
		 * the text is formatted.
		 */
		assert(windefault && from && markbuffer(from) == markbuffer(windefault->cursor));

		/* we'll start at the bottom and work backward.  All text
		 * will therefore be inserted into the cut-buffer at what
		 * is currently its end.
		 */
		(void)marktmp(dfrom, dest, o_bufchars(dest));

		/* The "to" mark is actually the start of the line *AFTER* the
		 * last line to be included in the cut.  This makes display
		 * updates easier, but we need to decrement the "to" mark
		 * here or else we'll be cutting one line too many.
		 */
		line = markdup(to);
		marksetoffset(line, markoffset((*windefault->md->move)(windefault, line, -1, INFINITY, True)));

		/* for each line of the rectangle... */
		do
		{
			/* Choose the starting point on this line.  Make sure
			 * the left edge of the character is in the rectangle
			 */
			sfrom = *(*windefault->md->move)(windefault, line, 0, windefault->selleft, False);
			if ((*windefault->md->mark2col)(windefault, &sfrom, False) < windefault->selleft)
			{
				markaddoffset(&sfrom, 1);
			}

			/* Choose the ending point on this line.  Add 1 so that
			 * the final character is included in the yanking, but
			 * be careful never to yank a newline.
			 */
			sto = *(*windefault->md->move)(windefault, line, 0, windefault->selright, False);
			if (scanchar(&sto) != '\n')
			{
				markaddoffset(&sto, 1);
			}

			/* append this slice of the rectangle */
			bufreplace(&dfrom, &dfrom, toCHAR("\n"), 1);
			if (markoffset(&sfrom) < markoffset(&sto))
			{
				bufpaste(&dfrom, &sfrom, &sto);
				if (del)
				{
					bufreplace(&sfrom, &sto, NULL, 0);
				}
			}

			/* locate the next line */
			prevline = markoffset(line);
			marksetoffset(line, markoffset((*windefault->md->move)(windefault, line, -1, INFINITY, True)));
			if (prevline == markoffset(line))
			{
				marksetoffset(line, markoffset(from));
			}

		} while (markoffset(line) > markoffset(from));
		markfree(line);
		break;
	}

	/* if this the external cut buffer, then write it */
	if (cbname == '>' && gui->clipopen && (*gui->clipopen)(True))
	{
		for (scanalloc(&cp, marktmp(dfrom, dest, CUT_TYPELEN));
		     cp;
		     markaddoffset(&dfrom, scanright(&cp)), scanseek(&cp, &dfrom))
		{
			(*gui->clipwrite)(cp, scanright(&cp));
		}
		(*gui->clipclose)();
		scanfree(&cp);
	}

	/* Report.  Except that we don't need to report how many new input
	 * lines we've copied to the ELVIS_PREVIOUS_INPUT buffer.  Also, when
	 * the mouse is used to mark text under X11, it is immediately copied
	 * to the clipboard and we don't want to report that.
	 */
	if (o_buflines(dest) - origlines >= o_report
	 && cbname != '.'
	 && (cbname != '>' || !windefault || !windefault->seltop))
	{
		if (del)
			msg(MSG_INFO, "[d]$1 lines deleted", o_buflines(dest) - origlines);
		else if (isupper(cbname))
			msg(MSG_INFO, "[d]$1 more lines yanked", o_buflines(dest) - origlines);
		else
			msg(MSG_INFO, "[d]$1 lines yanked", o_buflines(dest) - origlines);
	}
}

/* This function pastes text that was yanked by cutyank.  Returns NULL on
 * errors, or the final cursor position if successful.
 */
MARK cutput(cbname, win, at, after, cretend, lretend)
	_CHAR_	cbname;	/* cut buffer name */
	WINDOW	win;	/* window showing that buffer */
	MARK	at;	/* where to insert the text */
	BOOLEAN	after;	/* if True, insert after "at"; else insert before */
	BOOLEAN	cretend;/* if character-mode: True=return first, False=return last */
	BOOLEAN	lretend;/* if not character-mode: True=return first, False=return last */
{
	BUFFER	src;
	CHAR	iobuf[1000];
	CHAR	type;
	CHAR	*cp;
	MARKBUF	sfrom, sto;
	static MARKBUF ret;
	int	i;
	long	line, col;
	BOOLEAN	cmd;

	/* If anonymous buffer, and most recent paste was from a numbered
	 * cut buffer, then use the successive numbered buffer by default.
	 */
	if (!cbname)
	{
		if (previous >= '1' && previous < '9')
			cbname = previous + 1;
		else if (previous == '9')
			cbname = '9';
	}

	/* find the cut buffer */
	src = cutbuffer(cbname, True);
	if (!src)
	{
		return NULL;
	}

	/* if external cut buffer, then fill it from GUI */
	if (cbname == '<' && gui->clipopen && (*gui->clipopen)(False))
	{
		bufreplace(marktmp(sfrom, src, 0), marktmp(sto, src, o_bufchars(src)), toCHAR("character\n"), CUT_TYPELEN);
		while ((i = (*gui->clipread)(iobuf, sizeof(iobuf))) > 0)
		{
			bufreplace(marktmp(sfrom, src, CUT_TYPELEN), &sfrom, iobuf, i);
		}
		(*gui->clipclose)();
	}

	/* if the buffer is empty, fail */
	if (o_bufchars(src) <= CUT_TYPELEN)
	{
		/* well, the '.' buffer is okay, but all others fail */
		if (cbname == '.')
		{
			ret = *at;
			return &ret;
		}
		msg(MSG_ERROR, "[C]cut buffer $1 empty", cbname);
		return NULL;
	}

	/* figure out what type of yank this was */
	type = scanchar(marktmp(sfrom, src, 0));

	/* do the paste */
	switch (type)
	{
	  case 'c': /* CHARACTER MODE */
		/* choose the insertion point */
		ret = *at;
		if (after && scanchar(at) != '\n')
		{
			markaddoffset(&ret, 1);
		}

		/* paste it & set "ret" to the new cursor cursor */
		bufpaste(&ret, marktmp(sfrom, src, CUT_TYPELEN), marktmp(sto, src, o_bufchars(src)));
		if (cretend)
		{
			markaddoffset(&ret, o_bufchars(src) - CUT_TYPELEN - 1);
		}
		break;

	  case 'l': /* LINE MODE */
		/* choose the insertion point */
		if (after)
		{
			ret = *(win->md->move)(win, at, 0, INFINITY, False);
			markaddoffset(&ret, 1);
		}
		else
		{
			ret = *(win->md->move)(win, at, 0, 0, False);
		}

		/* paste it & set "ret" to the start of the new cursor line */
		bufpaste(&ret, marktmp(sfrom, src, CUT_TYPELEN), marktmp(sto, src, o_bufchars(src)));
		if (lretend)
		{
			markaddoffset(&ret, o_bufchars(src) - CUT_TYPELEN);
			ret = *(win->md->move)(win, &ret, -1, 0, True);
		}

		/* move new cursor past any whitespace at start of line */
		for (scanalloc(&cp, &ret);
		     cp && (*cp == '\t' || *cp == ' ');
		     scannext(&cp))
		{
		}
		if (cp)
			ret = *scanmark(&cp);
		scanfree(&cp);
		break;

	  case 'r': /* RECTANGLE MODE */
		/* choose a starting point, and a column to try for */
		if (after)
		{
			cmd = True;
			col = (*win->md->mark2col)(win, at, cmd) + 1;
		}
		else
		{
			cmd = False;
			col = (*win->md->mark2col)(win, at, cmd);
		}
		ret = *(*win->md->move)(win, at, 0, col, cmd);
		(void)marktmp(sto, src, lowline(bufbufinfo(src), 2) - 1);

		/* for each data line in the cut buffer... */
		for (line = 2;
		     line <= o_buflines(src) && markoffset(&ret) < o_bufchars(markbuffer(&ret));
		     line++)
		{
			/* delimit the contents of the next line in this cutbuf */
			sfrom = sto;
			markaddoffset(&sfrom, 1);
			(void)marktmp(sto, src, lowline(bufbufinfo(src), line + 1) - 1);

			/* paste it */
			bufpaste(&ret, &sfrom, &sto);

			/* move to the next line in destination buffer */
			ret = *(*win->md->move)(win, &ret, 1, col, cmd);
		}
		if (!lretend)
		{
			ret = *at;
		}
		break;

	  default:
		msg(MSG_ERROR, "[C]cut buffer $1 scrambled", cbname);
		return NULL;
	}


	/* report */
	if (o_buflines(src) - 1 >= o_report && cbname != '.')
	{
		msg(MSG_INFO, "[d]$1 lines pasted", o_buflines(src) - 1);
	}

	return &ret;
}

/* This function copies the contents of a cut buffer into RAM.  The memory
 * image contains no hint as to whether it was a line mode cut, or character
 * cut, or rectangle.  The calling function is responsible for calling
 * safefree() when the memory image is no longer needed.  Returns NULL if
 * the buffer is empty, doesn't exist, or appears to be corrupt.  The
 * "< cut buffer is illegal in this contents, and will also return NULL.
 */
CHAR *cutmemory(cbname)
	_CHAR_	cbname;	/* cut buffer name */
{
	BUFFER	src;
	MARKBUF	from, to;

	/* Find the cut buffer.  If it looks wrong, then return NULL. */
	src = cutbuffer(cbname, False);
	if (cbname == '<' || !src || o_bufchars(src) <= CUT_TYPELEN)
	{
		return NULL;
	}

	/* copy the contents into the memory */
	return bufmemory(marktmp(from, src, CUT_TYPELEN), marktmp(to, src, o_bufchars(src)));
}
