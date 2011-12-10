/* buffer.c */
/* Copyright 1995 by Steve Kirkendall */

char id_buffer[] = "$Id: buffer.c,v 2.68 1996/09/26 01:05:21 steve Exp $";

#include "elvis.h"

#define swaplong(x,y)	{long tmp; tmp = (x); (x) = (y); (y) = tmp;}

#if USE_PROTOTYPES
static void proc(_BLKNO_ bufinfo, long nchars, long nlines, long changes, long prevloc, CHAR *name);
static void freeundo(BUFFER buffer);
static void bufdo(BUFFER buf, BOOLEAN wipe);
# ifdef DEBUG_ALLOC
static void checkundo(char *where);
static void removeundo(struct undo_s *undo);
# endif
#endif

/* This variable points to the head of a linked list of buffers */
BUFFER buffers;

/* This is the default buffer.  Its options have been inserted into the
 * list accessible via optset().  This variable should only be changed by
 * the bufoptions() function.
 */
BUFFER bufdefault;

/* This stores the message type that will be used for reporting the number
 * of lines read or written.  It is normally MSG_STATUS so that other messages
 * will be allowed to overwrite it; however, when quitting it is set to
 * MSG_INFO so messages will be queued and eventually displayed somewhere
 * else after the window is closed.  It is also set to MSG_INFO during
 * initialization so the "read..." message appears in the new window.
 */
MSGIMP bufmsgtype = MSG_INFO;

/* This buffer's contents are irrelevent.  The values of its options, though,
 * are significant because the values of its options are used as the default 
 * values of any new buffer.  This buffer is also used as the default buffer
 * during execution of the initialization scripts.
 */
BUFFER bufdefopts;

/* This array describes buffer options */
static OPTDESC bdesc[] =
{
	{"filename", "file",	optsstring,	optisstring	},
	{"bufname", "buffer",	optsstring,	optisstring	},
	{"buflines", "bl",	optnstring,	optisnumber	},
	{"bufchars", "bc",	optnstring,	optisnumber	},
	{"retain", "ret",	NULL,		NULL		},
	{"modified", "mod",	NULL,		NULL,		},
	{"edited", "samename",	NULL,		NULL,		},
	{"newfile", "new",	NULL,		NULL,		},
	{"readonly", "ro",	NULL,		NULL,		},
	{"autoindent", "ai",	NULL,		NULL,		},
	{"inputtab", "itab",	opt1string,	optisoneof,	"tab spaces filename"},
	{"autotab", "at",	NULL,		NULL,		},
	{"tabstop", "ts",	optnstring,	optisnumber,	"1:100"},
	{"ccprg", "cp",		optsstring,	optisstring	},
	{"equalprg", "ep",	optsstring,	optisstring	},
	{"keywordprg", "kp",	optsstring,	optisstring	},
	{"makeprg", "mp",	optsstring,	optisstring	},
	{"paragraphs", "para",	optsstring,	optisstring	},
	{"sections", "sect",	optsstring,	optisstring	},
	{"shiftwidth", "sw",	optnstring,	optisnumber	},
	{"undolevels", "ul",	optnstring,	optisnumber	},
	{"textwidth", "tw",	optnstring,	optisnumber	},
	{"internal", "internal",NULL,		NULL		},
	{"bufdisplay", "bd",    optsstring,	optisstring	},
	{"errlines", "errlines",optnstring,	optisnumber	},
	{"binary", "bin",	NULL,		NULL		}
};

#ifdef DEBUG_ALLOC
/* This are used for maintaining a linked list of all undo versions. */
struct undo_s *undohead, *undotail;

/* This function is called after code which is suspected of leaking memory.
 * It checks all of the undo versions, making sure that each one is still
 * accessible via some buffer.
 */
static void checkundo(where)
	char	*where;
{
	struct undo_s	*scan, *undo;
	BUFFER		buf;

	/* for each undo version... */
	for (scan = undohead; scan; scan = scan->link1)
	{
		/* make sure the buffer still exists */
		for (buf = buffers; buf != scan->buf; buf = buf->next)
		{
			if (!buf)
				msg(MSG_FATAL, "[s]$1 - buffer disappeared, undo/redo not freed", where);
		}

		/* make sure this is an undo/redo for this buffer */
		if (scan->undoredo == 'l')
		{
			if (scan != buf->undolnptr)
			{
				msg(MSG_FATAL, "[s]$1 - undolnptr version leaked", where);
			}
		}
		else
		{
			for (undo = scan->undoredo=='u' ? buf->undo : buf->redo;
			     undo != scan;
			     undo = undo->next)
			{
				if (!undo)
					msg(MSG_FATAL, "[ss]$1 - $2 version leaked", where, scan->undoredo=='u'?"undo":"redo");
			}
		}
	}
}

static void removeundo(undo)
	struct undo_s	*undo;
{
	if (undo->link1)
		undo->link1->link2 = undo->link2;
	else
		undotail = undo->link2;
	if (undo->link2)
		undo->link2->link1 = undo->link1;
	else
		undohead = undo->link1;
}
#else
# define checkundo(s)
#endif

/* This function is called during session file initialization.  It creates
 * a BUFFER struct for the buffer, and collects the undo versions.
 */
static void proc(bufinfo, nchars, nlines, changes, prevloc, name)
	_BLKNO_	bufinfo;	/* block describing the buffer */
	long	nchars;		/* number of characters in buffer */
	long	nlines;		/* number of lines in buffer */
	long	changes;	/* value of "changes" counter */
	long	prevloc;	/* offset of most recent change to buffer */
	CHAR	*name;		/* name of the buffer */
{
	BUFFER		buf;
	BLKNO		tmp;
	struct undo_s	*undo, *scan, *lag;

	/* try to find a buffer by this name */
	for (buf = buffers; buf && CHARcmp(o_bufname(buf), name); buf = buf->next)
	{
	}

	/* if no buffer exists yet, then create one and make it use the old
	 * bufinfo block.
	 */
	if (!buf)
	{
		buf = bufalloc(name, bufinfo);
		buf->bufinfo = bufinfo;
		o_buflines(buf) = nlines;
		o_bufchars(buf) = nchars;
		buf->changes = changes;
		buf->changepos = prevloc;

		/* guess some values for a few other critical options */
		if (!CHARncmp(name, toCHAR("Elvis "), 6) &&
			CHARncmp(name, toCHAR("Elvis untitled"), 14))
		{
			/* probably an internal buffer */
			optpreset(o_internal(buf), True, OPT_HIDE);
			optpreset(o_modified(buf), False, OPT_HIDE);
		}
		else
		{
			/* the filename is probably the same as the buffer name */
			optpreset(o_filename(buf), CHARdup(name), OPT_FREE|OPT_HIDE);
			optpreset(o_internal(buf), False, OPT_HIDE);

			/* Mark it as readonly so the user will have to think
			 * before clobbering an existing file.
			 */
			o_readonly(buf) = True;

			/* Mark it as modified, so the user has to think
			 * before exitting and losing this session file.
			 */
			optpreset(o_modified(buf), True, OPT_HIDE);
		}
		return;
	}

	/* We found the buffer.  Is this the newest version found so far? */
	if (changes > buf->changes)
	{
		/* yes, this is the newest.  Swap this one with the version
		 * currently in the buf struct.  That will leave this version
		 * (the newest) as the current version, and the current
		 * (second newest) in the arguements and ready to be added
		 * to the undo list.
		 */
		tmp = buf->bufinfo;
		buf->bufinfo = bufinfo;
		bufinfo = tmp;
		swaplong(o_buflines(buf), nlines);
		swaplong(o_bufchars(buf), nchars);
		swaplong(buf->changes, changes);
		swaplong(buf->changepos, prevloc);
	}

	/* insert as an "undo" version */
	undo = (struct undo_s *)safealloc(1, sizeof *undo);
	undo->changes = changes;
	undo->changepos = prevloc;
	undo->buflines = nlines;
	undo->bufchars = nchars;
	undo->bufinfo = bufinfo;
	for (scan = buf->undo, lag = NULL;
	     scan && scan->changes > changes;
	     lag = scan, scan = scan->next)
	{
	} 
	undo->next = scan;
	if (lag)
	{
		lag->next = undo;
	}
	else
	{
		buf->undo = undo;
	}
#ifdef DEBUG_ALLOC
	undo->link1 = undohead;
	undohead = undo;
	undo->link2 = NULL;
	if (undo->link1)
		undo->link1->link2 = undo;
	else
		undotail = undo;
	undo->buf = buf;
	undo->undoredo = 'u';
#endif
}

/* Restart a session */
void bufinit()
{
	assert(BUFOPTQTY == QTY(bdesc));

	/* find any buffers left over from a previous edit */
	lowinit(proc);

	/* create the default options buffer, if it doesn't exist already */
	bufdefopts = bufalloc(toCHAR(DEFAULT_BUF), 0);
	o_internal(bufdefopts) = True;
	bufoptions(bufdefopts);
}

/* Create a buffer with a given name.  The buffer will initially be empty;
 * if it is meant to be associated with a particular file, then the file must
 * be copied into the buffer in a separate operation.  If there is already
 * a buffer with the desired name, it returns a pointer to the old buffer
 * instead of creating a new one.
 */
BUFFER bufalloc(name, bufinfo)
	CHAR	*name;	/* name of the buffer */
	_BLKNO_	bufinfo;/* block number describing the buffer (0 to create) */
{
	BUFFER	buffer;
	BUFFER	scan, lag;	/* used for inserting new buffer */
	char	unique[255];	/* name of untitled buffer */
	int	i = 1;		/* for generating a name for untitled buffer */

	/* if no name was specified, generate a unique untitled name */
	if (!name)
	{
		do
		{
			sprintf(unique, UNTITLED_BUF, i++);
		} while (buffind(toCHAR(unique)));
		name = toCHAR(unique);
	}

	/* see if there's already a buffer with that name */
	buffer = buffind(name);
	if (buffer)
	{
		return buffer;
	}

	/* allocate buffer struct */
	buffer = (BUFFER)safekept(1, sizeof(*buffer));

	/* create a low-level buffer */
	if (bufinfo)
	{
		buffer->bufinfo = bufinfo;
	}
	else
	{
		buffer->bufinfo = lowalloc(tochar8(name));
	}

	/* initialize the buffer options */
	optpreset(o_readonly(buffer), o_defaultreadonly, OPT_HIDE);
	if (bufdefopts)
	{
		/* copy all options except the following: filename bufname
		 * buflines bufchars modified edited newfile internal autotab
		 */
		optpreset(buffer->retain, bufdefopts->retain, OPT_HIDE);
		buffer->autoindent = bufdefopts->autoindent;
		buffer->inputtab = bufdefopts->inputtab;
		buffer->tabstop = bufdefopts->tabstop;
		buffer->shiftwidth = bufdefopts->shiftwidth;
		buffer->undolevels = bufdefopts->undolevels;
		buffer->textwidth = bufdefopts->textwidth;
		buffer->autotab = bufdefopts->autotab;

		/* Strings are tricky, because we may need to allocate a
		 * duplicate of the value.
		 */
		buffer->cc = bufdefopts->cc;
		if (buffer->cc.flags & OPT_FREE)
			o_cc(buffer) = CHARdup(o_cc(bufdefopts));
		buffer->cc.flags |= OPT_HIDE;
		buffer->equalprg = bufdefopts->equalprg;
		if (buffer->equalprg.flags & OPT_FREE)
			o_equalprg(buffer) = CHARdup(o_equalprg(bufdefopts));
		buffer->keywordprg = bufdefopts->keywordprg;
		if (buffer->keywordprg.flags & OPT_FREE)
			o_keywordprg(buffer) = CHARdup(o_keywordprg(bufdefopts));
		buffer->make = bufdefopts->make;
		if (buffer->make.flags & OPT_FREE)
			o_make(buffer) = CHARdup(o_make(bufdefopts));
		buffer->make.flags |= OPT_HIDE;
		buffer->paragraphs = bufdefopts->paragraphs;
		if (buffer->paragraphs.flags & OPT_FREE)
			o_paragraphs(buffer) = CHARdup(o_paragraphs(bufdefopts));
		buffer->sections = bufdefopts->sections;
		if (buffer->sections.flags & OPT_FREE)
			o_sections(buffer) = CHARdup(o_sections(bufdefopts));
		buffer->bufdisplay = bufdefopts->bufdisplay;
		if (buffer->bufdisplay.flags & OPT_FREE)
			o_bufdisplay(buffer) = CHARdup(o_bufdisplay(bufdefopts));
	}
	else /* no default options -- set them explicitly */
	{
		o_inputtab(buffer) = 't';
		o_autotab(buffer) = True;
		o_tabstop(buffer) = 8;
#ifdef OSCCPRG
		o_cc(buffer) = toCHAR(OSCCPRG);
#else
		o_cc(buffer) = toCHAR("cc ($1?$1:$2)");
#endif
		o_equalprg(buffer) = toCHAR("fmt");
		o_keywordprg(buffer) = toCHAR("ref");
#ifdef OSMAKEPRG
		o_make(buffer) = toCHAR(OSMAKEPRG);
#else
		o_make(buffer) = toCHAR("make $1");
#endif
		o_paragraphs(buffer) = toCHAR("PPppIPLPQPP");
		o_sections(buffer) = toCHAR("NHSHSSSEse");
		o_shiftwidth(buffer) = 8;
		o_undolevels(buffer) = 0;
		o_bufdisplay(buffer) = toCHAR("normal");
		optflags(o_textwidth(buffer)) = OPT_REDRAW;
	}

	/* set the name of this buffer, and limit access to some options */
	optpreset(o_bufname(buffer), CHARkdup(name), OPT_HIDE|OPT_LOCK|OPT_FREE);
	optflags(o_buflines(buffer)) = OPT_HIDE|OPT_LOCK;
	optflags(o_bufchars(buffer)) = OPT_HIDE|OPT_LOCK;

	/* Add the buffer to the linked list of buffers.  Keep it sorted. */
	for (lag = (BUFFER)0, scan = buffers;
	     scan && CHARcmp(o_bufname(scan), o_bufname(buffer)) < 0;
	     lag = scan, scan = scan->next)
	{
	}
	buffer->next = scan;
	if (lag)
	{
		lag->next = buffer;
	}
	else
	{
		buffers = buffer;
	}

	/* return the new buffer */
	return buffer;
}

/* Locate a buffer with a particular name.  (Note that this uses the buffer
 * name, not the filename.)  Returns a pointer to the buffer, or NULL for
 * unknown buffers.
 */
BUFFER buffind(name)
	CHAR	*name;	/* name of the buffer to find */
{
	BUFFER buffer;

	/* scan through buffers, looking for a match */
	for (buffer = buffers; buffer && CHARcmp(name, o_bufname(buffer)); buffer = buffer->next)
	{
	}
	return buffer;
}

/* Read a text file or filter output into a specific place in the buffer */
BOOLEAN bufread(mark, rfile)
	MARK	mark;	/* where to insert the new next */
	char	*rfile;	/* file to read, or !cmd for filter */
{
	BUFFER	buf;		/* the buffer we're reading into */
	long	offset;		/* offset of mark before inserting text */
	long	origlines;	/* original number of lines in file */
	CHAR	chunk[4096];	/* I/O buffer */
	int	nread;		/* number of bytes in chunk[] */
	BOOLEAN	newbuf;		/* is this a new buffer? */

	/* initialize some vars */
	buf = markbuffer(mark);
	newbuf = (BOOLEAN)(o_bufchars(buf) == 0 && rfile[0] != '!' && (o_verbose || !o_internal(buf)));
	origlines = o_buflines(buf);

	/* open the file/filter */
	if (!ioopen(rfile, 'r', False, False, o_binary(markbuffer(mark))))
	{
		msg(MSG_ERROR, "[s]error opening $1", rfile);
		return False;
	}

	/* read the text */
	if (newbuf)
		msg(MSG_STATUS, "[s]reading $1", rfile);
	while ((nread = ioread(chunk, QTY(chunk))) > 0)
	{
		if (guipoll(False))
		{
			ioclose();
			return False;
		}
		offset = markoffset(mark);
		bufreplace(mark, mark, chunk, nread);
		marksetoffset(mark, offset + nread);
	}
	ioclose();
	if (newbuf)
		msg(bufmsgtype, "[sdd]read $1, $2 lines, $3 chars", rfile,
					o_buflines(buf), o_bufchars(buf));
	else if (!o_internal(buf))
		msg(bufmsgtype, "[d]read $1 lines", o_buflines(buf) - origlines);

	return True;
}

/* Create a buffer for a given file, and then load the file.  Return a pointer
 * to the buffer.  If the file can't be read for some reason, then complain and
 * leave the buffer empty, but still return the empty buffer.
 *
 * If the buffer already exists and contains text, then the "reload" option
 * can be used to force it to discard that text and reload the buffer; when
 * "reload" is False, it would leave the buffer unchanged instead.
 */
BUFFER bufload(bufname, filename, reload)
	CHAR	*bufname;	/* name of buffer, or NULL to derive from filename */
	char	*filename;	/* name of file to load into a buffer */
	BOOLEAN	reload;		/* load from file even if previously loaded? */
{
	BUFFER	buf;
	MARKBUF	top;
	MARKBUF	end;
	BUFFER	initbuf;	/* buffer containing the initialization script */
	int	i;


	/* Create a buffer, whose name defaults to the same as this file */
	buf = bufalloc(bufname ? bufname : toCHAR(filename), 0);

	/* Does the buffer already contain text? */
	if (o_bufchars(buf) > 0)
	{
		/* If we aren't supposed to reload, then just return the
		 * buffer as-is.
		 */
		if (!reload)
			return buf;

		/* Save the text as an "undo" version, and then delete it */
		if (windefault && markbuffer(windefault->cursor) == buf)
			bufwilldo(windefault->cursor);
		else
			bufwilldo(marktmp(top, buf, 0));
		bufreplace(marktmp(top, buf, 0), marktmp(end, buf, o_bufchars(buf)), (CHAR *)0, 0);
	}

	/* Set the buffer's options */
	optpreset(o_filename(buf), CHARkdup(filename), OPT_HIDE|OPT_LOCK|OPT_FREE);
	optpreset(o_internal(buf), (bufname ? True : False), OPT_HIDE);
	optpreset(o_edited(buf), True, OPT_HIDE);
	o_readonly(buf) = False;
	optpreset(o_newfile(buf), False, OPT_HIDE);
	switch (dirperm(filename))
	{
	  case DIR_INVALID:
	  case DIR_BADPATH:
	  case DIR_NOTFILE:
	  case DIR_UNREADABLE:
	  case DIR_READONLY:
		o_readonly(buf) = True;
		break;

	  case DIR_NEW:
		o_newfile(buf) = True;
		break;

	  case DIR_READWRITE:
		/* nothing needed */
		break;
	}

	/* Execute the "before read"  script, if it exists.  If the script
	 * fails, then don't load the newly-created buffer.
	 */
	if (!o_internal(buf))
	{
		initbuf = buffind(toCHAR(BEFOREREAD_BUF));
		if (initbuf)
		{
			/* make the buffer available to :set */
			bufoptions(buf);

			/* execute the script */
			if (experform(windefault, marktmp(top, initbuf, 0),
				marktmp(end, initbuf, o_bufchars(initbuf))) != RESULT_COMPLETE)
			{
				return buf;
			}
		}
	}

	/* read the file's contents into the buffer */
	if (o_newfile(buf))
	{
		msg(bufmsgtype, "[sdd]$1 [NEW FILE]", filename);
	}
	else if (!bufread(marktmp(top, buf, 0), filename))
	{
		o_edited(buf) = False;
		return buf;
	}

	/* set other options to describe the file */
	o_modified(buf) = False;
	optpreset(o_errlines(buf), o_buflines(buf), OPT_HIDE);

	/* Restore the marks to their previous offsets.  Otherwise any marks
	 * which refer to this buffer will be set to the end of the file.
	 * Restoring them isn't perfect, but it beats setting them all to EOF!
	 * (Note: New buffers won't have an "undo" version.)
	 */
	if (buf->undo)
	{
		for (i = 0; i < QTY(buf->undo->offset); i++)
		{
			/* ignore unset marks, or marks into other buffers*/
			if (buf->undo->offset[i] < 0)
				continue;
			assert(markbuffer(namedmark[i]) == buf);

			/* marks past the new end should be freed */
			if (buf->undo->offset[i] >= o_bufchars(buf))
			{
				markfree(namedmark[i]);
				continue;
			}

			/* other marks should have their offsets restored
			 * to what they were before the buffer was loaded
			 */
			marksetoffset(namedmark[i], buf->undo->offset[i]);
		}
	}

	/* execute the file initialization script, if it exists */
	if (!o_internal(buf))
	{
		initbuf = buffind(toCHAR(AFTERREAD_BUF));
		if (initbuf)
		{
			/* make the buffer available to :set */
			bufoptions(buf);

			/* Execute the script's contents. */
			(void)experform(windefault, marktmp(top, initbuf, 0),
				marktmp(end, initbuf, o_bufchars(initbuf)));
		}
	}

	return buf;
}


/* This function searches through a path for a file to load.  It then loads
 * that file into a buffer and returns the buffer.  If the file couldn't be
 * located, it returns NULL instead.  If a buffer already exists with the given
 * name, then it returns that buffer without attempting to load anything.
 */
BUFFER bufpath(path, filename, bufname)
	CHAR	*path;		/* path to search through */
	char	*filename;	/* file to search for */
	CHAR	*bufname;	/* name of buffer to store the file */
{
	char	*pathname;	/* full pathname of the loaded file */
	char	pathdup[256];	/* local copy of pathname */
	BUFFER	buf;

	/* if the buffer already exists, return it immediately */
	buf = buffind(bufname);
	if (buf)
	{
		return buf;
	}

	/* try to find the file */
	pathname = iopath(tochar8(path), filename, False);
	if (!pathname)
	{
		return (BUFFER)0;
	}

	/* we need a local copy of the pathname, because bufload() will also
	 * call iopath() to find the "elvis.brf" and "elvis.arf" files, and
	 * iopath() only has a single static buffer that it uses for returning
	 * the found pathname.  We don't want our pathname clobbered.
	 */
	strcpy(pathdup, pathname);

	/* load the file */
	buf = bufload(bufname, pathdup, True);
	return buf;
}


/* This function deletes the oldest undo versions of a given buffer */
static void freeundo(buffer)
	BUFFER	buffer;	/* buffer to be cleaned */
{
	struct undo_s *undo, *other, *tail;
	int	      i;

	checkundo("before freundo()");

	/* locate the most recent doomed version */
	i = o_undolevels(buffer);
	if (i < 1) i++;
	for (other = NULL, undo = buffer->undo;
	     i > 0 && undo;
	     i--, other = undo, undo = undo->next)
	{
	}

	/* if none are doomed, return now */
	if (!undo)
	{
		return;
	}

	/* Remove the most recent doomed version (and all following versions)
	 * from the linked list of undo versions.
	 */
	tail = other;
	if (tail)
	{
		tail->next = NULL;
	}
	else
	{
		buffer->undo = NULL;
	}

	/* delete each doomed version */
	for (; undo; undo = other)
	{
		other = undo->next;
		/* free the lowbuf and the (struct undo_s) structure */
		lowfree(undo->bufinfo);
#ifdef DEBUG_ALLOC
		removeundo(undo);
#endif
		safefree(undo);
	}
	checkundo("after freeundo");
}



/* Free a buffer which was created via bufalloc(). */
void buffree(buffer)
	BUFFER	buffer;	/* buffer to be destroyed */
{
	BUFFER	scan, lag;
	struct undo_s *undo;

	assert(buffer != bufdefopts);
	checkundo("before buffree");

	/* if any window is editing this buffer, then fail */
	if (wincount(buffer) > 0)
	{
		return;
	}

	/* transfer any marks to the dummy "bufdefopts" buffer */
	while (buffer->marks)
	{
		marksetoffset(buffer->marks, 0L);
		marksetbuffer(buffer->marks, bufdefopts);
	}

	/* free any undo/redo versions of this buffer */
	while (buffer->undo)
	{
		undo = buffer->undo;
		buffer->undo = undo->next;
		lowfree(undo->bufinfo);
#ifdef DEBUG_ALLOC
		removeundo(undo);
#endif
		safefree(undo);
	}
	while (buffer->redo)
	{
		undo = buffer->redo;
		buffer->redo = undo->next;
		lowfree(undo->bufinfo);
#ifdef DEBUG_ALLOC
		removeundo(undo);
#endif
		safefree(undo);
	}
	if (buffer->undolnptr)
	{
		lowfree(buffer->undolnptr->bufinfo);
#ifdef DEBUG_ALLOC
		removeundo(buffer->undolnptr);
#endif
		safefree(buffer->undolnptr);
		buffer->undolnptr = NULL;
	}
	assert(buffer->undo == NULL && buffer->redo == NULL);

	/* locate the buffer in the linked list */
	for (lag = NULL, scan = buffers; scan != buffer; lag = scan, scan = scan->next)
	{
		assert(scan->next);
	}

	/* remove it from the linked list */
	if (lag)
	{
		lag->next = scan->next;
	}
	else
	{
		buffers = scan->next;
	}

	/* free the values of any string options which have been set */
	optfree(QTY(bdesc), &buffer->filename);

	/* free any marks in this buffer */
	while (buffer->marks)
	{
		markfree(buffer->marks);
	}

	/* free the low-level block */
	lowfree(buffer->bufinfo);

	/* free the buffer struct itself */
	safefree(buffer);
	checkundo("after buffree");
}


/* Free a buffer, if possible without losing anything important.  Return
 * True if freed, False if retained.  If "force" is True, it tries harder.
 */
BOOLEAN bufunload(buf, force, save)
	BUFFER	buf;	/* buffer to be unloaded */
	BOOLEAN	force;	/* if True, try harder */
	BOOLEAN	save;	/* if True, maybe save even if noautowrite */
{
	MARKBUF	top, bottom;

	/* if "internal" then retain it for now */
	if (o_internal(buf))
	{
		return False;
	}

	/* if being used by some window, then retain it */
	if (wincount(buf) > 0)
	{
		return False;
	}

	/* If supposed to retain, then keep it unless "force" is True.
	 * If this is a temporary session, then no buffers will be retained,
	 * so we should fail in that situation too.
	 */
	if (o_retain(buf) && !force && !o_tempsession)
	{
		return False;
	}

	/* if not modified, then discard it */
	if (!o_modified(buf))
	{
		buffree(buf);
		return True;
	}

	/* if readonly, or no known filename then free it if "force" or
	 * retain it if not "force"
	 */
	if (o_readonly(buf) || o_filename(buf) == NULL)
	{
		if (force)
		{
			buffree(buf);
		}
		return force;
	}

	/* Try to save the buffer to a file */
	if (save && o_filename(buf)
	 && bufwrite(marktmp(top, buf, 0), marktmp(bottom, buf, o_bufchars(buf)), tochar8(o_filename(buf)), force))
	{
		free(buf);
		return True;
	}
	return False;
}


/* Return True if "buf" can be deleted without loosing data, or False if it
 * can't -- in which case it also emits a message describing why.  NOTE THAT
 * YOU ALSO NEED TO MAKE SURE THE BUFFER ISN'T BEING EDITED IN A WINDOW.
 *
 * This function has side-effects.  If the buffer has been modified, it may
 * try to write the buffer out to a file.
 */
BOOLEAN bufsave(buf, force, mustwr)
	BUFFER	buf;	/* the buffer to write */
	BOOLEAN	force;	/* passed to bufwrite() if writing is necessary */
	BOOLEAN	mustwr;	/* write to file even if buffer isn't modified */
{
	MARKBUF	top, bottom;

	/* Can never "save" or delete the internal buffers */
	if (o_internal(buf))
	{
		msg(MSG_ERROR, "[s]$1 is used internally by elvis", o_bufname(buf));
		return False;
	}

	/* If writing wasn't explicitly demanded, and isn't needed, then
	 * return True without doing anything else.
	 */
	if (!mustwr && !o_modified(buf))
	{
		return True;
	}

	/* We know that we need to write this buffer.  If it has no filename
	 * then we can't write it.
	 */
	if (!o_filename(buf))
	{
		msg(MSG_ERROR, "no file name");
		return False;
	}

	/* try to write the buffer out to its file */
	return bufwrite(marktmp(top, buf, 0),
			marktmp(bottom, buf, o_bufchars(buf)),
			tochar8(o_filename(buf)), force);
}



/* Write a buffer, or part of a buffer, out to a file.  Return True if
 * successful, or False if error.
 */
BOOLEAN bufwrite(from, to, wfile, force)
	MARK	from;	/* start of text to write */
	MARK	to;	/* end of text to write */
	char	*wfile;	/* output file, ">>file" to append, "!cmd" to filter */
	BOOLEAN	force;	/* write even if file already exists */
{
	BUFFER	buf = markbuffer(from);
	BUFFER	initbuf;	/* one of the file initialization buffers */
	MARKBUF	top;		/* the endpoints of initbuf */
	MARKBUF	next;		/* used for determining append location */
	CHAR	*cp;		/* used for scanning through file */
	BOOLEAN	append;		/* If True, we're appending */
	BOOLEAN	filter;		/* If True, we're writing to a filter */
	BOOLEAN	wholebuf;	/* if True, we're writing the whole buffer */
	BOOLEAN	samefile;	/* If True, we're writing the buffer to its original file */
	int	bytes;

	assert(from && to && wfile);
	assert(markbuffer(from) == markbuffer(to));
	assert(markoffset(from) >= 0);
	assert(markoffset(from) <= markoffset(to));
	assert(markoffset(to) <= o_bufchars(buf));

	/* Determine some characteristics of this write */
	append = (BOOLEAN)(wfile[0] == '>' && wfile[1] == '>');
	filter = (BOOLEAN)(wfile[0] == '!');
	wholebuf = (BOOLEAN)(markoffset(from) == 0 && markoffset(to) == o_bufchars(buf));
	samefile = (BOOLEAN)(o_filename(buf) && !strcmp(wfile, tochar8(o_filename(buf))));

	/* if we're appending, skip the initial ">>" */
	if (append)
	{
		for (wfile += 2; isspace(*wfile); wfile++)
		{
		}
	}

	/* If writing to the same file, as it is a readonly file, then fail
	 * unless we're forcing a write.
	 */
	if (!filter && wholebuf && samefile && o_readonly(buf) && !force)
	{
		msg(MSG_ERROR, "[s]$1 readonly" , wfile);
		return False;
	}

	/* If this is supposed to be a new file, or we're writing to
	 * a name other than what was originally loaded, and we aren't
	 * forcing a write, then make sure the file doesn't already
	 * exist.
	 */
	if ((o_newfile(buf) || !o_edited(buf) || !samefile || !wholebuf)
	  && !force
	  && !append
	  && !filter
	  && dirperm(wfile) != DIR_NEW)
	{
		msg(MSG_ERROR, "[s]$1 exists", wfile);
		return False;
	}

	/* If we're writing the whole file back over itself, then execute the
	 * "before write" script if it exists.  If this fails and "force" isn't
	 * true, then fail.
	 */
	if (wholebuf && samefile && !filter && !append)
	{
		initbuf = buffind(toCHAR(BEFOREWRITE_BUF));
		if (initbuf)
		{
			/* make the buffer be the default buffer */
			bufoptions(buf);

			/* execute the script */
			if (experform(windefault, marktmp(top, initbuf, 0),
				marktmp(next, initbuf, o_bufchars(initbuf))) != RESULT_COMPLETE
			    && !force)
			{
				return False;
			}
		}
	}

	/* Try to write the file */
	if (ioopen(wfile, append ? 'a' : 'w', False, True, o_binary(markbuffer(from))))
	{
		if (wholebuf && !filter)
		{
			msg(MSG_STATUS, "[s]writing $1", wfile);
		}
		next = *from;
		scanalloc(&cp, &next);
		assert(cp);
		bytes = 1;
		do
		{
			/* check for ^C */
			if (guipoll(False))
			{
				ioclose();
				scanfree(&cp);
				return False;
			}

			bytes = scanright(&cp);
			if (markoffset(&next) + bytes > markoffset(to))
			{
				bytes = (int)(markoffset(to) - markoffset(&next));
			}
			if (iowrite(cp, bytes) < bytes)
			{
				msg(MSG_ERROR, (wfile[0] == '!') ? "broken pipe" : "disk full");
				ioclose();
				scanfree(&cp);
				return False;
			}
			markaddoffset(&next, bytes);
			scanseek(&cp, &next);
		} while (cp != NULL && markoffset(&next) < markoffset(to));
		ioclose();
		scanfree(&cp);

		if (!filter)
		{
			if (append)
				msg(bufmsgtype, "[ds]appended $1 lines to $2",
					markline(to) - markline(from), wfile);
			else
				msg(bufmsgtype, "[sdd]wrote $1, $2 lines, $3 chars",
					wfile, markline(to) - markline(from),
					markoffset(to) - markoffset(from));
		}
	}
	else
	{
		/* We had an error, and already wrote the error message */
		return False;
	}

	/* Execute the "after write" script. */
	if (wholebuf && samefile && !filter && !append)
	{
		initbuf = buffind(toCHAR(AFTERWRITE_BUF));
		if (initbuf)
		{
			/* make the buffer be the default buffer */
			bufoptions(buf);

			/* execute the script */
			(void)experform(windefault, marktmp(top, initbuf, 0),
				marktmp(next, initbuf, o_bufchars(initbuf)));
		}
	}

	/* Writing the whole file has some side-effects on options */
	if (wholebuf && !append && !filter)
	{
		/* if it had no filename before, it has one now */
		if (!o_filename(buf) && wholebuf)
		{
			o_filename(buf) = CHARdup(toCHAR(wfile));
			optflags(o_filename(buf)) |= OPT_FREE;
			buftitle(buf, toCHAR(wfile));
		}

		/* buffer is no longer modified */
		o_modified(buf) = False;
		o_newfile(buf) = False;
		
		/* if the original file was overwritten, then reset the
		 * readonly flag because apparently the file isn't readonly.
		 */
		if (!CHARcmp(o_filename(buf), toCHAR(wfile)))
		{
			o_readonly(buf) = False;
			o_edited(buf) = True;
		}
	}

	/* success! */
	return True;
}


/* Make "buf" be the default buffer.  The default buffer is the one whose
 * options are available to the :set command.
 */
void bufoptions(buf)
	BUFFER	buf;	/* the buffer to become the new default buffer */
{
	/* if same as before, then do nothing */
	if (buf == bufdefault)
	{
		return;
	}

	/* if there is a previous buffer, then delete its options */
	if (bufdefault)
	{
		optdelete(&bufdefault->filename);
	}

	/* make this buffer be the default */
	bufdefault = buf;

	/* if bufdefault is not NULL, then insert its options */
	if (buf)
	{
		optinsert("buf", QTY(bdesc), bdesc, &buf->filename);
	}
}


/* This function changes the name of a buffer.  If the buffer happens to
 * be the main buffer of one or more windows, then it will also retitle
 * those windows.
 */
void buftitle(buffer, title)
	BUFFER	buffer;	/* the buffer whose name is to be changed */
	CHAR	*title;	/* the new name of the buffer */
{
	WINDOW	win;

	/* change the name */
	safefree(o_bufname(buffer));
	o_bufname(buffer) = CHARkdup(title);

	/* change the window titles, if the gui supports that */
	if (gui->retitle)
	{
		for (win = winofbuf((WINDOW)0, buffer);
		     win;
		     win = winofbuf(win, buffer))
		{
			(*gui->retitle)(win->gw, tochar8(o_bufname(buffer)));
		}
	}
}


/* Set the buffer's flag that will eventually cause an undo version of to
 * be saved.
 */
void bufwilldo(cursor)
	MARK	cursor;	/* where to put cursor if we return to this "undo" version */
{
	markbuffer(cursor)->willdo = True;
	markbuffer(cursor)->docursor = markoffset(cursor);
}


/* Save an "undo" version of the buffer that "cursor" points to */
static void bufdo(buf, wipe)
	BUFFER	buf;	/* buffer to make an "undo" version for */
	BOOLEAN	wipe;	/* if True, then delete all "redo" versions */
{
	struct undo_s	*undo;
	int		i;
	long		linenum;

	checkundo("before bufdo");

	/* never save an undo version of an internal buffer */
	if (o_internal(buf))
		return;

	/* allocate an undo structure */
	undo = (struct undo_s *)safealloc(1, sizeof *undo);
#ifdef DEBUG_ALLOC
	undo->link1 = undohead;
	undohead = undo;
	undo->link2 = NULL;
	if (undo->link1)
		undo->link1->link2 = undo;
	else
		undotail = undo;
	undo->buf = buf;
	undo->undoredo = 'u';
#endif

	/* fill it in */
	undo->changepos = buf->changepos = buf->docursor;
	undo->buflines = o_buflines(buf);
	undo->bufchars = o_bufchars(buf);
	for (i = 0; i < QTY(namedmark); i++)
	{
		if (namedmark[i] && markbuffer(namedmark[i]) == buf)
		{
			undo->offset[i] = markoffset(namedmark[i]);
		}
		else
		{
			undo->offset[i] = -1;
		}
	}
	undo->bufinfo = lowdup(buf->bufinfo);

	/* insert it into the buffer's "undo" list */
	undo->next = buf->undo;
	buf->undo = undo;

	checkundo("in bufdo, before changing undolnptr");

	/* If this is on a different line from previous change, then store this
	 * as the current "line-undo".  This is done by recursively calling
	 * bufdo() with the same arguments to create a second identical
	 * struct undo_s structure, and then moving the second one from the
	 * undo list to the undolnptr variable.
	 */
	(void)lowoffset(undo->bufinfo, undo->changepos,
		(COUNT *)0, (COUNT *)0, (LBLKNO *)0, &linenum);
	if (linenum != buf->undoline)
	{
		/* change undoline to the expected line NOW, so we don't get
		 * stuck in infinite recursion.
		 */
		buf->undoline = linenum;

		/* Call bufdo() again to make another copy of the undo version.
		 * Increment undolevels temporarily so we don't loose the
		 * oldest one yet.
		 */
		linenum = o_undolevels(buf);
		o_undolevels(buf)++;
		if (o_undolevels(buf) < 2) o_undolevels(buf) = 2;
		bufdo(buf, False);
		o_undolevels(buf) = linenum;

		/* free the old one, if any */
		if (buf->undolnptr)
		{
			lowfree(buf->undolnptr->bufinfo);
#ifdef DEBUG_ALLOC
			removeundo(buf->undolnptr);
#endif
			safefree(buf->undolnptr);
		}

		/* Use the second undo copy as the line-undo version */
		buf->undolnptr = buf->undo;
		buf->undo = buf->undo->next;
#ifdef DEBUG_ALLOC
		buf->undolnptr->undoredo = 'l';
#endif
		assert(buf->undo == undo);
	}

	checkundo("in bufdo, after changing undolnptr");

	/* discard the redo versions, if we're supposed to */
	if (wipe)
	{
		while (buf->redo && buf->redo != buf->undolnptr)
		{
			undo = buf->redo;
			buf->redo = undo->next;
			lowfree(undo->bufinfo);
#ifdef DEBUG_ALLOC
			removeundo(undo);
#endif
			safefree(undo);
		}

		checkundo("in bufdo, after wiping the redo versions");
	}

	/* discard the oldest version[s] from the undo list */
	freeundo(buf);

	/* make sure this is written to disk */
	if (!o_internal(buf) && eventcounter > 2)
		sessync();
}

/* Recall a previous "undo" version of a given buffer.  The "back" argument
 * should be positive to undo, negative to redo, or 0 to undo all changes
 * to the current line.  Returns the cursor offset if successful, or -1 if
 * the requested undo version doesn't exist.
 */
long bufundo(cursor, back)
	MARK	cursor;	/* buffer to be undone, plus cursor offset of current version */
	long	back;	/* number of versions back (negative to redo) */
{
	struct undo_s	*undo;
	struct undo_s	*tmp;
	long		i;
	BUFFER		buffer;
	MARKBUF		from, to;
	long		delta;
	long		origulev;

	checkundo("before bufundo");

	/* locate the desired undo version */
	buffer = markbuffer(cursor);
	if (back == 0)
	{
		/* line undo */
		undo = buffer->undolnptr;
	}
	else if (o_undolevels(buffer) == 0)
	{
		/* Can only oscillate between previous version and this one,
		 * but it has the advantage that <u> and <^R> both do exactly
		 * the same thing.
		 */
		if (buffer->redo)
			undo = buffer->redo, back = -1;
		else
			undo = buffer->undo, back = 1;
	}
	else if (back > 0)
	{
		/* undo */
		for (i = back, undo = buffer->undo; undo && i > 1; i--, undo = undo->next)
		{
		}
	}
	else
	{
		/* redo */
		for (i = -back, undo = buffer->redo; undo && i > 1; i--, undo = undo->next)
		{
		}
	}

	/* if the requested version doesn't exist, then fail */
	if (!undo)
	{
		return -1;
	}

	/* save the current version as either an undo version or a redo version,
	 * so we can revert to it.  Note that we increase the number of undo
	 * levels temporarily, so the oldest undo version won't be discarded
	 * yet.
	 */
	origulev = o_undolevels(buffer);
	o_undolevels(buffer)++;
	if (o_undolevels(buffer) < 2) o_undolevels(buffer) = 2;
	bufwilldo(marktmp(from, buffer, undo->changepos));
	bufdo(buffer, False);
	if (back > 0)
	{
		/* undoing: move from "undo" to "redo" */
		while (buffer->undo != undo)
		{
			assert(buffer->undo);
			tmp = buffer->undo;
			buffer->undo = tmp->next;
			tmp->next = buffer->redo;
			buffer->redo = tmp;
#ifdef DEBUG_ALLOC
			tmp->undoredo = 'r';
#endif
		}

		/* remove the selected version from the "undo" list */
		buffer->undo = undo->next;
	}
	else if (back < 0)
	{
		/* redoing: move from "redo" to "undo" */
		while (buffer->redo != undo)
		{
			assert(buffer->redo);
			tmp = buffer->redo;
			buffer->redo = tmp->next;
			tmp->next = buffer->undo;
			buffer->undo = tmp;
#ifdef DEBUG_ALLOC
			tmp->undoredo = 'u';
#endif
		}

		/* remove the selected version from the "redo" list */
		buffer->redo = undo->next;
	}
	else
	{
		/* line-undo: Remove the selected undo version from the
		 * undolnptr pointer.
		 */
		buffer->undolnptr = (struct undo_s *)0;
		buffer->undoline = 0;
	}

	/* replace the current version with the selected undo version */
	lowfree(buffer->bufinfo);
	buffer->bufinfo = undo->bufinfo;
	delta = undo->bufchars - o_bufchars(buffer);
	buffer->changepos = undo->changepos;
	o_buflines(buffer) = undo->buflines;
	o_bufchars(buffer) = undo->bufchars;
	buffer->changes++;
	o_modified(buffer) = True;

	/* Adjust the values of any marks.  Most marks can be fixed just by
	 * calling markadjust(), but the named marks' old values are stored in
	 * the undo buffer and we can recall them exactly.
	 */
	if (delta < 0)
	{
		markadjust(marktmp(from, buffer, undo->changepos),
			marktmp(to, buffer, undo->changepos - delta),
			delta);
	}
	else
	{
		markadjust(marktmp(from, buffer, undo->changepos), &from, delta);
	}
	for (i = 0; i < QTY(namedmark); i++)
	{
		if (undo->offset[i] >= 0 && (!namedmark[i] || markbuffer(namedmark[i]) == buffer))
		{
			if (namedmark[i])
				markfree(namedmark[i]);
			namedmark[i] = markalloc(buffer, undo->offset[i]);
		}
	}
#ifdef DISPLAY_MARKUP
	dmmuadjust(marktmp(from, buffer, 0), marktmp(to, buffer, o_bufchars(buffer)), 0);
#endif

	/* We can free the undo_s structure now. */
#ifdef DEBUG_ALLOC
	removeundo(undo);
#endif
	safefree(undo);

	/* Okay to free the oldest "undo" version now */
	o_undolevels(buffer) = origulev;
	freeundo(buffer);	/* Is this really necessary? */

	/* Return the offset of the last change, so the cursor can be moved
	 * there.  Never return a point past the end of the buffer, though.
	 */
	checkundo("after bufundo");
	if (o_bufchars(buffer) == 0)
		buffer->changepos = 0;
	else if (buffer->changepos >= o_bufchars(buffer))
		buffer->changepos = o_bufchars(buffer) - 1;
	return buffer->changepos;
}

/* This function replaces part of a buffer with new text.  In addition to
 * replacement, it can also be used to implement insertion (by having "from"
 * and "to" be identical) or deletion (by having "newlen" be zero).
 *
 * It uses markadjust() to automatically update marks.  If the buffer's
 * "willdo" flag is set, then it will automatically create an "undo" version
 * of the buffer before making the change.
 */
void bufreplace(from, to, newp, newlen)
	MARK	from;	/* starting position of old text */
	MARK	to;	/* ending position of old text */
	CHAR	*newp;	/* pointer to new text (in RAM) */
	long	newlen;	/* length of new text */
{
	long	chglines;
	long	chgchars;


	assert(markbuffer(from) == markbuffer(to) && newlen >= 0);

	/* if the destination's "willdo" flag is set, then save an "undo"
	 * version of it before doing the change
	 */
	if (markbuffer(from)->willdo)
	{
		bufdo(markbuffer(from), True);
		markbuffer(from)->willdo = False;
	}

	/* make the change to the buffer contents */
	if (markoffset(from) == markoffset(to))
	{
		/* maybe we aren't really changing anything? */
		if (newlen == 0)
		{
			return;
		}

		/* we're inserting */
		chglines = lowinsert(markbuffer(from)->bufinfo, markoffset(from), newp, newlen);
	}
	else if (newlen == 0)
	{
		/* we're deleting */
		chglines = lowdelete(markbuffer(from)->bufinfo, markoffset(from), markoffset(to));
	}
	else
	{
		/* we're replacing */
		chglines = lowreplace(markbuffer(from)->bufinfo, markoffset(from), markoffset(to), newp, newlen);
	}

	/* adjust the buffer totals */
	chgchars = newlen - (markoffset(to) - markoffset(from));
	o_buflines(markbuffer(from)) += chglines;
	o_bufchars(markbuffer(from)) += chgchars;
	o_modified(markbuffer(from)) = True;
	markbuffer(from)->changes++;

	/* adjust the marks */
#ifdef DISPLAY_MARKUP
	dmmuadjust(from, to, chgchars);
#endif
	markadjust(from, to, chgchars);
}

/* Copy part of one buffer into another.  "dst" is the destination (where
 * the pasted text will be inserted), and "from" and "to" describe the
 * portion of the source buffer to insert.
 *
 * This calls markadjust() to automatically adjust marks.  If the destination
 * buffer's "willdo" flag is set, it will save an "undo" version before
 * making the change.
 */
void bufpaste(dst, from, to)
	MARK	dst;	/* destination */
	MARK	from;	/* start of source */
	MARK	to;	/* end of source */
{
	long	chglines;
	long	chgchars;


	assert(markbuffer(from) == markbuffer(to));

	/* if the destination's "willdo" flag is set, then save an "undo"
	 * version of it before doing the paste
	 */
	if (markbuffer(dst)->willdo)
	{
		bufdo(markbuffer(dst), True);
		markbuffer(dst)->willdo = False;
	}

	/* make the change to the buffer's contents */
	chglines = lowpaste(markbuffer(dst)->bufinfo, markoffset(dst),
		markbuffer(from)->bufinfo, markoffset(from), markoffset(to));

	/* adjust the destination's counters */
	chgchars = markoffset(to) - markoffset(from);
	o_buflines(markbuffer(dst)) += chglines;
	o_bufchars(markbuffer(dst)) += chgchars;
	o_modified(markbuffer(dst)) = True;
	markbuffer(dst)->changes++;

	/* adjust marks */
	markadjust(dst, dst, chgchars);
}


/* Copy a section of some buffer into dynamically-allocated RAM, and append
 * a NUL to the end of the copy.  The calling function must call safefree()
 * on the returned memory.
 */
CHAR *bufmemory(from, to)
	MARK	from, to;	/* the section of the buffer to fetch */
{
	CHAR	*memory;	/* the allocated memory */
	CHAR	*scan, *build;	/* used for copying text from buffer to memory */
	long	i;

	assert(markbuffer(from) == markbuffer(to)
		&& markoffset(from) <= markoffset(to));

	/* allocate space for the copy */
	i = markoffset(to) - markoffset(from);
	memory = (CHAR *)safealloc((int)(i + 1), sizeof(CHAR));

	/* copy the text into it */
	for (scanalloc(&scan, from), build = memory; i > 0; scannext(&scan), i--)
	{
		assert(scan);
		*build++ = *scan;
	}
	scanfree(&scan);
	*build = '\0';

	return memory;
}
