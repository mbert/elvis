/* io.c */
/* Copyright 1995 by Steve Kirkendall */

char id_io[] = "$Id: io.c,v 2.21 1996/08/27 00:59:40 steve Exp $";

#include "elvis.h"

/* This file contains some generic I/O functions.  They can read/write to
 * either a file or a filter.  They perform efficient character-at-a-time
 * semantics by buffering the I/O requests.
 */

static BOOLEAN reading;	/* True if file/program open for reading */
static BOOLEAN writing;	/* True if file/program open for writing */
static BOOLEAN forfile;	/* True if I/O to file; False for program */
static BOOLEAN usestdio;/* True if using stdin/stdout; False otherwise */
static BOOLEAN beautify;/* True if we're supposed to strip control chars */

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
BOOLEAN	ioopen(name, rwa, prgsafe, force, binary)
	char	*name;	/* name of file, or "!program" */
	_char_	rwa;	/* 'r' to read, 'w' to write, or 'a' to append */
	BOOLEAN	prgsafe;/* If True, allow "!program"; else refuse */
	BOOLEAN	force;	/* if True, allow files to be clobbered */
	BOOLEAN	binary;	/* If True, use binary I/O; else use text I/O */
{
	DIRPERM perms;

	assert(!reading && !writing);

	/* are we going to beautify this text? */
	beautify = (BOOLEAN)(o_beautify && !binary);

	/* Is this stdin/stdout? */
	if (!name || !*name)
	{
		usestdio = True;
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
			return False;
		}

		/* will we be reading or writing? */
		forfile = usestdio = False;
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
			return False;

		  default:
			msg(MSG_ERROR, "can't append to filter");
			return False;
		}
	}
	else /* I/O to file */
	{
		/* try to open the file */
		forfile = True;
		perms = dirperm(name);
		switch (rwa)
		{
		  case 'r':
			if ((perms == DIR_READONLY || perms == DIR_READWRITE)
				&& txtopen(name, 'r', binary) == 0)
			{
				reading = True;
				return True;
			}
			break;

		  case 'w':
			if ((perms == DIR_NEW || perms == DIR_NOTFILE ||
				(perms == DIR_READWRITE && (force || o_writeany)))
			     && txtopen(name, 'w', binary) == 0)
			{
				writing = True;
				return True;
			}
			break;

		  default:
			if (((perms == DIR_NEW && force) || perms == DIR_READWRITE)
			     && txtopen(name, 'a', binary) == 0)
			{
				writing = True;
				return True;
			}
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
		return False;
	}
	/*NOTREACHED*/
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
	assert(writing);

	/* write to the file/program */
	if (usestdio)
	{
		return fwrite(iobuf, sizeof(CHAR), (size_t)len, stdout);
	}
	else if (forfile)
	{
		return txtwrite(iobuf, len);
	}
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

	/* read from the file/program */
	if (usestdio)
	{
		nread = fread(iobuf, sizeof(CHAR), (size_t)len, stdin);
	}
	else if (forfile)
	{
		nread = txtread(iobuf, len);
	}
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

	/* don't really close stdin/stdout; just reset variables */
	if (usestdio)
	{
		usestdio = reading = writing = False;
		return True;
	}

	/* completing I/O for a file is easy */
	if (forfile)
	{
		txtclose();
		reading = writing = False;
		return True;
	}

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
	int		matchlen;	/* # of leading identical characters */
	int		nmatches;	/* number of matches */
	char		*fname;		/* name of a matching file */
	CHAR		slash[1];	/* this system's directory separator */
	int		col;		/* width of directory listing */

	/* Find the directory separator character */
	slash[0] = dirpath("a", "b")[1];

	/* replace ~ with home directory name */
	if (partial[0] == '~' && partial[1] == (char)slash[0])
	{
		strcpy(homed, tochar8(o_home));
		strcat(homed, &partial[1]);
		partial = homed;
	}

	/* count the matching filenames */
	for (nmatches = matchlen = 0, fname = dirfirst(partial, True);
	     fname;
	     nmatches++, fname = dirnext())
	{
		if (nmatches == 0)
		{
			strcpy(match, fname);
			matchlen = strlen(match);
		}
		else
		{
			while (matchlen > 0 && strncmp(match, fname, (size_t)matchlen))
			{
				matchlen--;
			}
		}
	}

	/* so what did we come up with? */
	if (nmatches == 1)
	{
		/* unique match -- append a tab or slash */
		match[matchlen] = (dirperm(match)==DIR_NOTFILE) ? *slash : endchar;
		match[matchlen + 1] = '\0';
		return match;
	}
	else if (matchlen > 0)
	{
		/* can we add any chars to the partial name? */
		if ((unsigned)matchlen <= strlen(partial) && !strncmp(partial, match, matchlen))
		{
			/* No - list all matches */
			for (fname = dirfirst(partial, True), col = 0;
			     fname;
			     fname = dirnext())
			{
				/* space between names */
				if (col + strlen(fname) + 2 >= (unsigned)o_columns(windefault))
				{
					drawextext(windefault, toCHAR("\n"), 1);
					col = 0;
				}
				else if (col > 0)
				{
					drawextext(windefault, toCHAR(" "), 1);
					col++;
				}

				/* show the name */
				drawextext(windefault, toCHAR(fname), strlen(fname));
				col += strlen(fname);

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
