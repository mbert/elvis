/* io.c */
/* Copyright 1995 by Steve Kirkendall */

char id_io[] = "$Id: io.c,v 2.43 1998/11/25 01:27:09 steve Exp $";

#include "elvis.h"

#if USE_PROTOTYPES
extern char *parseurl(char *url);
#endif

/* This file contains some generic I/O functions.  They can read/write to
 * either a file or a filter.  They perform efficient character-at-a-time
 * semantics by buffering the I/O requests.
 */


#ifdef FEATURE_COMPLETE
/* This is a list of characters which may have special meaning with filenames */
static CHAR dangerous[] = {' ', '#', '%', '*', '?', '[', '{', '\0'};
#endif

static BOOLEAN	reading;  /* True if file/program open for reading */
static BOOLEAN	writing;  /* True if file/program open for writing */
static BOOLEAN	forfile;  /* True if I/O to file; False for program */
static BOOLEAN	forstdio; /* True if using stdin/stdout; False otherwise */
static BOOLEAN	beautify; /* True if we're supposed to strip control chars */
static char	convert;  /* One of {unix, dos, mac} else no conversion */
static BOOLEAN	cvtcr;	  /* did last DOS read end with a CR, before LF? */
static CHAR	tinybuf[100];
static int	tinyqty;
static int	tinyused;

#if defined(PROTOCOL_HTTP) || defined(PROTOCOL_FTP)
static BOOLEAN	forurl;   /* True if I/O via HTTP; false otherwise */
#endif

#ifdef DEBUG_ALLOC
static char	*openfile; /* name of source file which opened current file */
static int	openline; /* line number within source file */
#endif

/* This function opens a file or program for either input or output.
 * It returns True if successful.  If there are any errors, it issues an
 * error message and returns False.
 *
 * "name" is the name of the file to open.  If the name begins with a '!'
 * then the rest of the name is interpretted as a program to be executed
 * instead of a program name.
 *
 * "rwa" can be 'r' to open the file/program for reading, 'w' to open the
 * file/program for writing, or 'a' to open a file for appending.  'a' can't
 * be used with programs.
 *
 * "prgsafe" is only significant if the "safer" option is set.  When "safer"
 * is set and "prgsafe" is False, this function will fail if "name" refers
 * to a program.
 *
 * "force" can cause a file to be overwritten even if "name" and "oldname"
 * don't match.
 */
#ifdef DEBUG_ALLOC
BOOLEAN	_ioopen(file, line, name, rwa, prgsafe, force, eol)
	char	*file;	/* name of caller's source file */
	int	line;	/* line number within caller's source file */
#else
BOOLEAN	ioopen(name, rwa, prgsafe, force, eol)
#endif
	char	*name;	/* name of file, or "!program" */
	_char_	rwa;	/* one of {read, write, append} */
	BOOLEAN	prgsafe;/* If True, allow "!program"; else refuse */
	BOOLEAN	force;	/* if True, allow files to be clobbered */
	_char_	eol;	/* one of {unix, dos, mac, text, binary} */
{
	DIRPERM perms;

#ifdef DEBUG_ALLOC
	if (openfile)
		msg(MSG_FATAL, "[sdsd]$1,$2: file still open from $3,$4", file, line, openfile, openline);
	openfile = file;
	openline = line;
#endif
	assert(!reading && !writing);

	/* are we going to beautify this text? */
	beautify = (BOOLEAN)(o_beautify && eol != 'b');

	/* if conversion is equivelent to binary, then use binary */
	if ((eol == 'm' && '\n' == 015)
	 || (eol == 'u' && '\n' == 012))
	{
		eol = 'b';
	}

	/* nothing in the tiny buffer */
	tinyqty = tinyused = 0;

	/* Is this stdin/stdout? */
	if (!name || !*name)
	{
		forstdio = True;
		reading = (BOOLEAN)(rwa == 'r');
		writing = (BOOLEAN)!reading;
		return True;
	}
  	/* Is this a program? */
	else if (*name == '!')
	{
		/* check safety */
		if (o_safer && !prgsafe)
		{
			msg(MSG_ERROR, "unsafe filter");
#ifdef DEBUG_ALLOC
			openfile = NULL;
#endif
			return False;
		}

		/* will we be reading or writing? */
		forfile = forstdio = False;
		switch (rwa)
		{
		  case 'r':
			/* Open the program for reading, using the GUI-
			 * dependent version of prgopen() if there is one.
			 * Then call prggo() right away, since we aren't going
			 * to be calling prgwrite().
			 */
		  	if ((gui->prgopen
		  		? (*gui->prgopen)(name + 1, False, True)
				: prgopen(name + 1, False, True))
			    && prggo())
			{
				/* success! */
				reading = True;
				return True;
			}
			else
			{
				msg(MSG_ERROR, "[s]can't run $1", name + 1);
			}
#ifdef DEBUG_ALLOC
			openfile = NULL;
#endif
			return False;

		  case 'w':
			/* Open the program.  Note that if there is no GUI-
			 * dependent version of prgopen(), then we'll need to
			 * explicitly read the program's output and display it
			 * in the window, so we call the generic prgopen()
			 * for both writing and reading.
			 */
		  	if (gui->prgopen
		  		? (*gui->prgopen)(name + 1, True, False)
				: prgopen(name + 1, True, True))
		  	{
				writing = True;
				return True;
			}
			msg(MSG_ERROR, "[s]can't run $1", name + 1);
#ifdef DEBUG_ALLOC
			openfile = NULL;
#endif
			return False;

		  default:
			msg(MSG_ERROR, "can't append to filter");
#ifdef DEBUG_ALLOC
			openfile = NULL;
#endif
			return False;
		}
	}

#if defined(PROTOCOL_HTTP) || defined(PROTOCOL_FTP)
	/* is it a remote URL? */
	if (urlremote(name))
	{
		/* try to open the resource via the network */
		if (!urlopen(name, force, rwa))
		{
			/* error message already given */
#ifdef DEBUG_ALLOC
			openfile = NULL;
#endif
			return False;
		}

		/* remember that we're using an URL */
		forurl = True;
		forstdio = forfile = False;
		reading = (BOOLEAN)(rwa == 'r');
		writing = (BOOLEAN)!reading;
		return True;
	}
#endif

	/* Anything else must be a plain old file */
	name = urllocal(name);
	if (!name)
	{
		msg(MSG_ERROR, "unsupported protocol");
#ifdef DEBUG_ALLOC
			openfile = NULL;
#endif
		return False;
	}

	/* try to open the file */
	forstdio = False;
	forfile = True;
	convert = eol;
	cvtcr = False;
	perms = urlperm(name);
	switch (rwa)
	{
	  case 'r':
		if ((perms == DIR_READONLY || perms == DIR_READWRITE)
			&& txtopen(name, 'r', (BOOLEAN)(convert != 't')) == 0)
		{
			reading = True;
			return True;
		}
		break;

	  case 'a':
		if (perms == DIR_READWRITE
		     && txtopen(name, 'a', (BOOLEAN)(convert != 't')) == 0)
		{
			writing = True;
			return True;
		}
		/* else fall through to the 'w' case */

	  case 'w':
		if ((perms == DIR_NEW || perms == DIR_NOTFILE ||
			(perms == DIR_READWRITE && (force || o_writeany)))
		     && txtopen(name, 'w', (BOOLEAN)(convert != 't')) == 0)
		{
			writing = True;
			return True;
		}
		break;

	}

	/* If we get here, we failed.  "perms" gives clue as to why */
	switch (perms)
	{
	  case DIR_INVALID:	msg(MSG_ERROR, "[s]malformed file name $1", name);	break;
	  case DIR_BADPATH:	msg(MSG_ERROR, "[s]bad path $1", name);			break;
	  case DIR_NOTFILE:	msg(MSG_ERROR, "[s]$1 is not a file", name);		break;
	  case DIR_NEW:		msg(MSG_ERROR, "[s]$1 doesn't exist", name);		break;
	  case DIR_UNREADABLE:	msg(MSG_ERROR, "[s]$1 unreadable", name);		break;
	  case DIR_READONLY:	msg(MSG_ERROR, "[s]$1 unwritable", name);		break;
	  case DIR_READWRITE:	msg(MSG_ERROR, "[s]$1 exists", name);			break;
	}
#ifdef DEBUG_ALLOC
	openfile = NULL;
#endif
	return False;
}

/* This function writes the contents of a given I/O buffer.  "iobuf" points
 * to the buffer, and "len" is the number of CHARs in that buffer.  This
 * function returns the number of CHARs actually written; values less than
 * "len" indicate trouble.
 */
int iowrite(iobuf, len)
	CHAR	*iobuf;	/* RAM buffer containing text */
	int	len;	/* number of CHARs in iobuf */
{
	int	base, okay;
 static	CHAR	crlf[2] = {015, 012};
	assert(writing);

	/* write to the file/program */
	if (forstdio)
	{
		return fwrite(iobuf, sizeof(CHAR), (size_t)len, stdout);
	}
	else if (forfile)
	{
		switch (convert)
		{
		  case 'u':
		  case 'd':
		  case 'm':
			for (base = okay = 0; base + okay < len; okay++)
			{
				if (iobuf[base + okay] == '\n')
				{
					if (okay > 0)
						txtwrite(&iobuf[base], okay);
					switch (convert)
					{
					  case 'u': txtwrite(crlf+1, 1); break;
					  case 'd': txtwrite(crlf, 2);	 break;
					  case 'm': txtwrite(crlf, 1);	 break;
					}
					base += okay + 1;
					okay = -1;
				}
			}
			if (okay > 0)
				txtwrite(&iobuf[base], okay);
			return base + okay;

		  default:
			return txtwrite(iobuf, len);
		}
	}
#if defined(PROTOCOL_HTTP) || defined(PROTOCOL_FTP)
	else if (forurl)
	{
		return urlwrite(iobuf, len);
	}
#endif
	else
	{
		return prgwrite(iobuf, len);
	}
}

/* This function fills a given I/O buffer with text read from the file/program.
 * "iobuf" points to the buffer, and "len" is the maximum number of CHARs that
 * it can hold.  This function returns the number of CHARs actually read, or
 * 0 at the end of the file.  If the "beautify" option is True, it strips
 * control characters.
 */
int ioread(iobuf, len)
	CHAR	*iobuf;	/* RAM buffer to receive input text */
	int	len;	/* maximum number of bytes to read */
{
	int	nread;	/* number of CHARs read */
	int	i, j;

	assert(reading);

	/* if trying to read a suitably tiny amount, then use the tinybuf
	 * instead of the user's buffer, so we can reduce syscalls.
	 */
	if (iobuf != tinybuf && len < QTY(tinybuf) && tinyused >= tinyqty)
	{
		tinyqty = ioread(tinybuf, QTY(tinybuf));
		tinyused = 0;
	}

	/* maybe we can fetch from the tiny buffer? */
	if (tinyqty > tinyused)
	{
		if (len > tinyqty - tinyused)
			len = tinyqty - tinyused;
		memcpy(iobuf, tinybuf + tinyused, len);
		tinyused += len;
		return len;
	}

	/* read from the file/program */
	if (forstdio)
	{
		nread = fread(iobuf, sizeof(CHAR), (size_t)len, stdin);
	}
	else if (forfile)
	{
		if (cvtcr)
		{
			iobuf[0] = 015;
			nread = 1 + txtread(iobuf + 1, len - 1);
			cvtcr = False;
		}
		else
			nread = txtread(iobuf, len);

		/* convert, if necessary */
		switch (convert)
		{
		  case 'u':
		  	if ('\n' == 012)
		  		break;
			for (i = 0; i < nread; i++)
			{
				if (iobuf[nread] == 012)
					iobuf[nread] = '\n';
			}
			break;

		  case 'd':
			for (i = j = 0; i < nread; i++, j++)
			{
				if (iobuf[i] == 015 && nread > 1)
				{
					if (i + 1 >= nread)
						cvtcr = True, j--;
					else if (iobuf[i + 1] == 012)
						iobuf[j] = '\n', i++;
					else
						iobuf[j] = iobuf[i];
				}
				else
					iobuf[j] = iobuf[i];
			}
			nread = j;
			break;

		  case 'm':
			for (i = 0; i < nread; i++)
			{
				if (iobuf[i] == 015)
					iobuf[i] = '\n';
			}
			break;
		}
	}
#if defined(PROTOCOL_HTTP) || defined(PROTOCOL_FTP)
	else if (forurl)
	{
		nread = urlread(iobuf, len);
	}
#endif
	else
	{
		nread = prgread(iobuf, len);
	}

	/* maybe strip control characters */
	if (beautify && nread > 0)
	{
		for (i = j = 0; i < nread; i++)
		{
			if (iobuf[i] >= ' ' || iobuf[i] == '\t' || iobuf[i] == '\n' || iobuf[i] == '\f')
			{
				iobuf[j++] = iobuf[i];
			}
		}
		nread = j;
	}

	/* return number of bytes read */
	return nread;
}

/* Close a file that was opened via ioopen() */
BOOLEAN	ioclose()
{
	CHAR	*rdbuf;
	int	nbytes;
	BOOLEAN	origrefresh;

	assert(reading || writing);
#ifdef DEBUG_ALLOC
	openfile = NULL;
#endif

	/* don't really close stdin/stdout; just reset variables */
	if (forstdio)
	{
		forstdio = reading = writing = False;
		return True;
	}

	/* completing I/O for a file is easy */
	if (forfile)
	{
		txtclose();
		reading = writing = False;
		return True;
	}

#if defined(PROTOCOL_HTTP) || defined(PROTOCOL_FTP)
	/* completing for HTTP is easy */
	if (forurl)
	{
		urlclose();
		reading = writing = forurl = False;
		return True;
	}
#endif

	/* Writing to a program; we need to call prggo() now.  Also, if there
	 * is no gui->prgopen() function then we need to explicitly copy the
	 * program's output to the current window, so the user can see error
	 * messages and other results.
	 */
	if (writing && prggo() && !gui->prgopen && windefault)
	{
		drawopencomplete(windefault);
		origrefresh = o_exrefresh;
		o_exrefresh = True;
		rdbuf = (CHAR *)safealloc(1024, sizeof(CHAR));
		while ((nbytes = prgread(rdbuf, 1024)) > 0)
		{
			drawextext(windefault, rdbuf, nbytes);
		}
		safefree(rdbuf);
		o_exrefresh = origrefresh;
	}

	/* if we wrote to a program with an explicit prgopen() command, then
	 * there may have been output that we don't know about.  Force the
	 * window into DRAW_OPENOUTPUT mode so we have to hit <Enter> to
	 * continue.
	 */
	if (writing && gui->prgopen && windefault)
	{
		drawopencomplete(windefault);
	}

	/* wait for the program to exit.  Succeed only if its exit code is 0 */
	reading = writing = False;
	return (BOOLEAN)((gui->prgclose ? (*gui->prgclose)() : prgclose()) == 0);
}



/* This function parses a "path" string into directory names, and checks each
 * directory until finds a readable file named "filename".  If the "usefile"
 * flag is True, then if an element of the path happens to be a file name
 * instead of a directory name, then it will also use that as the file name.
 *
 * If NULL is passed instead of a "path" string, then this function will
 * continue the previous path search instead of starting a new one.
 *
 * Returns the full pathname of the first readable file that it finds, or
 * NULL if it reaches the end of the path without finding one.
 */
char *iopath(path, filename, usefile)
	char	    *path;	/* list of directories to search */
	char	    *filename;	/* name of file to look for in each directory */
	BOOLEAN	    usefile;	/* allow path to contain filenames */
{
	static char *next;	/* the directory after this one */
	static char name[256];	/* full pathname, possibly of a readable file */
	int	    i;		/* used for building name */
	DIRPERM	    perms;	/* permissions of the file */

	/* if path was named, start from there; else continue previous search */
	if (path)
	{
		next = path;
	}

	/* repeat until we find a readable file... */
	do
	{
		/* if no place left to try, then quit */
		if (!next)
		{
			return (char *)0;
		}

		/* if next path element starts with ~, then use HOME instead */
		if (next[0] == '~' && !isalnum(next[1]))
		{
			/* copy HOME into name buffer */
			strcpy(name, tochar8(o_home));
			i = strlen(name);
			next++;
		}
		/* else if path element starts with "./" then use the current
		 * file's directory.  (unless there is no current file)
		 */
		else if (next[0] == '.'
		     && (next[1] == '/' || next[1] == '\\')
		     && bufdefault
		     && o_filename(bufdefault))
		{
			/* copy file's directory into name buffer */
			strcpy(name, dirdir(tochar8(o_filename(bufdefault))));
			i = strlen(name);
			next++;
		}
		else
		{
			i = 0;
		}

		/* copy characters up to next delimiter or end */
		while (*next && *next != OSPATHDELIM)
		{
			name[i++] = *next++;
		}
		name[i] = '\0';

		/* if this was the end of the path, then there is no "next" */
		if (!*next)
		{
			next = (char *)0;
		}
		else
		{
			/* skip delimiter */
			next++;
		}

		/* If files are allowed and this is a readable file, use it.
		 * Otherwise we'll need to append the filename and try that.
		 */
		if (!usefile ||
		   ((perms = dirperm(name)) != DIR_READONLY && perms != DIR_READWRITE))
		{
			/* append the filename */
			path = dirpath(*name ? name : ".", filename);
			strcpy(name, path);
			perms = dirperm(name);
		}
	} while (perms != DIR_READONLY && perms != DIR_READWRITE);

	/* return the found name */
	return name;
}

#ifdef FEATURE_COMPLETE

# if FILES_IGNORE_CASE
/* Compare two strings for equality, ignoring case differences.  Note that
 * unlike strcmp(), this function does *NOT* indicate which string comes first
 * if they don't match.
 */
static int ustrncmp(s1, s2, len)
	char	*s1, *s2;
	int	len;
{
	while (--len >= 0 && (*s1 || *s2))
	{
		if (toupper(*s1) != toupper(*s2))
			return 1;
		s1++;
		s2++;
	}
	return 0;
}
# endif /* FILES_IGNORE_CASE */

/* This function implements filename completion.  You pass it a partial
 * filename and it uses dirfirst()/dirnext() to extend the name.  If you've
 * given enough to uniquely identify a file, then it will also append a
 * tab after the filename.
 */
char *iofilename(partial, endchar)
	char	*partial;	/* a partial filenam to expand */
	_char_	endchar;	/* char to append if match found */
{
	char		homed[256];	/* partial, with "~" replaced by home */
	static char	match[256];	/* the matching text */
	unsigned	matchlen;	/* # of leading identical characters */
	int		nmatches;	/* number of matches */
	char		*fname;		/* name of a matching file */
	char		*bname;		/* basename (fname without path) */
	CHAR		slash[1];	/* this system's directory separator */
	CHAR		*str;		/* name with/without quotes */
	int		col;		/* width of directory listing */

	/* Find the directory separator character */
	slash[0] = dirpath("a", "b")[1];

	/* remove quotes (backslashes) from before certain characters */
	str = removequotes(dangerous, toCHAR(partial));

	/* replace ~ with home directory name, ~+ with current directory name,
	 * ~- with previous directory name, or (for Unix only) ~user with the
	 * home directory of the named user.
	 */
#if ANY_UNIX
	if (str[0] == '~' && isalpha(str[1]))
	{
		expanduserhome(tochar8(str), homed);
	}
	else
#endif /*ANY_UNIX*/
	if (str[0] == '~' && str[1] == (char)slash[0])
	{
		strcpy(homed, dirpath(tochar8(o_home), tochar8(str + 2)));
	}
	else if (str[0] == '~' && str[1] == '+' && str[2] == (char)slash[0])
	{
		strcpy(homed, dirpath(dircwd(), tochar8(str + 3)));
	}
	else if (str[0] == '~' && str[1] == '-' && str[2] == (char)slash[0])
	{
		strcpy(homed, dirpath(tochar8(o_previousdir), tochar8(str + 3)));
	}
	else
	{
		strcpy(homed, tochar8(str));
	}
	partial = homed;
	safefree(str);

	/* count the matching filenames */
	for (nmatches = matchlen = 0, fname = dirfirst(partial, True);
	     fname;
	     nmatches++, fname = dirnext())
	{
		/* skip if binary.  Keep directories, though */
		if (!o_completebinary		  /* we want to skip binaries */
		 && dirperm(fname) != DIR_NOTFILE /* but not directories */
		 && *ioeol(fname) == 'b')	  /* and this is a binary */
		{
			nmatches--;
			continue;
		}

		if (nmatches == 0)
		{
			strcpy(match, fname);
			matchlen = strlen(match);
		}
		else
		{
#if FILES_IGNORE_CASE
			while (matchlen > 0 && ustrncmp(match, fname, (size_t)matchlen))
				matchlen--;
#else
			while (matchlen > 0 && strncmp(match, fname, (size_t)matchlen))
				matchlen--;
#endif
		}
	}

	/* Filename completion should never reduce the partial file name!
	 * We need to guard against this, because if the filesystem is
	 * case-insensitive (as with Windows, but not Unix), then the list
	 * of matching file names could contain both upper- and lowercase
	 * names, which have *zero* matching characters.
	 */
	if (matchlen < strlen(partial))
		matchlen = strlen(partial);

	/* so what did we come up with? */
	if (nmatches == 1)
	{
		/* unique match... */

		/* decide whether to append a slash or tab */
		if (dirperm(match) == DIR_NOTFILE)
			endchar = (char)*slash;
		
		/* quote the dangerous chars */
		match[matchlen] = '\0';
		str = addquotes(dangerous, toCHAR(match));
		strncpy(match, tochar8(str), QTY(match) - 1);
		match[QTY(match) - 1] = '\0';
		matchlen = strlen(match);

		/* append a tab or slash, and return it */
		match[matchlen] = endchar;
		match[matchlen + 1] = '\0';
		return match;
	}
	else if (nmatches > 0 && matchlen > 0)
	{
		/* multiple matches... */

		/* can we add any chars to the partial name? */
		if ((unsigned)matchlen <= strlen(partial) && !strncmp(partial, match, matchlen))
		{
			/* No - list all matches */
			for (fname = dirfirst(partial, True), col = 0;
			     fname;
			     fname = dirnext())
			{
				/* skip binary files */
				if (!o_completebinary
				 && dirperm(fname) != DIR_NOTFILE
				 && *ioeol(fname) == 'b')
					continue;

				/* space between names */
				bname = dirfile(fname);
				if (col + strlen(bname) + 2 >= (unsigned)o_columns(windefault))
				{
					drawextext(windefault, toCHAR("\n"), 1);
					col = 0;
				}
				else if (col > 0)
				{
					drawextext(windefault, toCHAR(" "), 1);
					col++;
				}

				/* show the name, without its path */
				drawextext(windefault, toCHAR(bname), strlen(bname));
				col += strlen(bname);

				/* if directory, then append a slash */
				if (dirperm(fname) == DIR_NOTFILE)
				{
					drawextext(windefault, slash, 1);
					col++;
				}
			}
			if (col > 0)
			{
				drawextext(windefault, toCHAR("\n"), 1);
			}
		}

		/* Either way - return the common part of all matches */
		match[matchlen] = '\0';
		return match;
	}
	else
	{
		/* no match, or matches have nothing in common */
		return (char *)0;
	}
}
#endif /* FEATURE_COMPLETE */


/* This function tries to guess what type of newline a given file uses.
 * Returns "binary" if there is a NUL in the first few bytes.
 *         "unix"   if there is a solitary LF character.
 *         "dos"    if the first CR is followed by a LF.
 *         "mac"    if the first CR is followed by anything but LF.
 *         "text"   otherwise; e.g., for new or empty files.
 */
char *ioeol(filename)
	char	*filename;	/* name of file to check */
{
	int	nbytes;	/* number of bytes read from file */
	int	i;

#ifdef PROTOCOL_HTTP
	/* check for a protocol */
	for (i = 0; isalpha(filename[i]); i++)
	{
	}
	if (i < 2 || filename[i] != ':')
		i = 0;

	/* assume all protocols are binary except "file:" */
	if (!strncmp(filename, "file:", 5))
		filename += i + 1;
	else if (i > 0)
		return "binary";
#endif

	/* fill tinybuf with bytes from the beginning of the file */
	nbytes = 0;
	if (txtopen(filename, 'r', True) == 0)
	{
		nbytes = txtread(tinybuf, QTY(tinybuf));
		txtclose();
	}

	/* look for a NUL */
	for (i = 0; i < nbytes; i++)
	{
		if (tinybuf[i] == 0)
			return "binary";
	}

	/* look for an LF that isn't preceeded by a CR */
	for (i = 0; i < nbytes - 1; i++)
	{
		if (tinybuf[i] == 012 /* linefeed */
		 && (i == 0 || tinybuf[i - 1] != 015)) /* carriage return */
			return "unix";
	}

	/* look for the CR -- is it followed by a LF? */
	for (i = 0; i < nbytes - 1; i++)
	{
		if (tinybuf[i] == 015) /* carriage return */
		{
			if (tinybuf[i + 1] == 012) /* linefeed */
				return "dos";
			else
				return "mac";
		}
	}

	/* if all else fails, assume "text" */
	return "text";
}
