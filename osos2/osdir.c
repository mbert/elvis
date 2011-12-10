/* osos2/osdir.c */

/* This file contains functions that deal with filenames.  The structure of
 * this file is a little unusual, because some of the support programs also
 * use functions from this file.  To minimize the size of those support
 * programs, this file may be compiled with JUST_DIRFIRST or JUST_DIRPATH
 * defined to exclude the unused functions.  If neither of those names is
 * defined then the whole file is compiled.
 *
 * Ported by Lee Johnson, few changes (e.g. emx/gcc compatibility) by 
 * Martin "Herbert" Dietze.
 */

char id_osdir[] = "$Id: osdir.c,v 2.9 1996/03/11 01:31:22 steve Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef __IBMC__ /* Herbert: not needed for emx/gcc */
# include <direct.h>
#endif
#include <io.h>
#include <string.h>
#if !defined(JUST_DIRFIRST) && !defined(JUST_DIRPATH)
# include "elvis.h"
#endif

#ifdef JUST_DIRFIRST
/* will do for ctags, 
 * -- avoids needing stuff from digraph.c */
# undef tolower
# define tolower(c) (c | 0x20)
# undef toupper
# define toupper(c) (c & ~0x20)
# undef isalpha
# define isalpha(c)	((unsigned)(((c) & 0xffdf) - 'A') < 26)
#endif

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#ifdef __EMX__
# define USE_OS2_TOOLKIT_HEADERS
#endif
#include <os2.h>
#ifdef CHAR
# undef CHAR
#endif

#ifndef JUST_DIRPATH
# if USE_PROTOTYPES
static BOOLEAN wildcmp (char *fname, char *wild);
static BOOLEAN match_file (FILEFINDBUF3 *fb);
# endif
static void fix_slashes (char *path);
#endif

#define char_eq(c1, c2)	(tolower(c1) == tolower(c2))

/* Herbert:
 * the following two are needed by ref.exe, too...
 */
static void
fix_slashes (char *path)
{
	char *cp;

	for (cp = path; *cp != '\0'; cp++)
	{
		if (*cp == OSALTPSEP)
			*cp = OSPATHSEP;
	}
}


/* return the directory part of a pathname */
char *dirdir (pathname)
	char	*pathname;
{
	static char dir[CCHMAXPATH];
	char	*slash;

	strncpy (dir, pathname, sizeof dir);
	for (slash = &dir[strlen(dir)]; 
		--slash >= dir && *slash != '/' && *slash != '\\';)
	{
	}

	/* Convert slashes to backslashes. */
/*	fix_slashes (dir);
	slash = strrchr (dir, OSPATHSEP);*/

	if (slash == dir)
	{
		dir[1] = '\0';
	}
	else if (isalpha (dir[0]) && dir[1] == ':' && slash == &dir[2])
	{
		dir[3] = '\0';
	}
	else if (slash != NULL)
	{
		*slash = '\0';
	}
	else if (isalpha (dir[0]) && dir[1] == ':')
	{
		dir[2] = '\0';
	}
	else
	{
		strcpy (dir, ".");
	}

	return dir;
}

/* combine a directory name and a filename, yielding a pathname. */
char *dirpath (dir, file)
	char	*dir;	/* directory name */
	char	*file;	/* filename */
{
	static char	path[CCHMAXPATH];

	/* Convert slashes to backslashes. */
	fix_slashes (dir);
	fix_slashes (file);

	if (strcmp (dir, ".") == 0
		|| dir[0] == '\0'
	 	|| (isalpha (file[0]) && file[1] == ':')
	 	|| (!(isalpha (dir[0]) && dir[1] == ':') && file[0] == OSPATHSEP))
	{
		/* no dir, or file has drive letter, or file is absolute within
		 * drive but dir doesn't specify drive.
		 */
		strcpy (path, file);
	}
	else if (isalpha (dir[0]) && dir[1] == ':' && file[0] == OSPATHSEP)
	{
		/* dir has drive letter, and file is absolute within drive */
		sprintf (path, "%.2s%s", dir, file);
	}
	else if (!dir[0] || dir[strlen(dir) - 1] != OSPATHSEP)
	{
		/* dir ends without \, and file is relative to dir */
		sprintf (path, "%s\\%s", dir, file);
	}
	else
	{
		/* dir ends with \, and file is relative to dir */
		sprintf (path, "%s%s", dir, file);
	}
	return path;
}

#ifndef JUST_DIRPATH

/* This is the wildcard expression used by dirfirst() and dirnext() */
static char *wildfile;

/* This is the name of the directory being scanned by dirfirst() and dirnext() */
static char *wilddir;

/* This is the directory handle used by dirfirst() and dirnext() */
static HDIR hdir = NULLHANDLE;

/* This variable is True if the wildcard expression only needs to match the
 * beginning of the name, or False if it must match the entire filename.
 */
static BOOLEAN partial;

/* This recursive function checks a filename against a wildcard expression.
 * Returns True for match, False for mismatch.
 */
static BOOLEAN wildcmp (fname, wild)
	char	*fname;	/* an actual filename */
	char	*wild;	/* a wildcard expression */
{
	int	i, match;

  TailRecursion:
	switch (*wild)
	{
	  case '?':
		/* match any single character except \0 and / */
		if (*fname == '/' || !*fname)
		{
			return False;
		}
		fname++;
		wild++;
		goto TailRecursion;

	  case '[':
		/* if no matching ], then compare [ as literal character;
		 * else next char in fname must be in brackets
		 */
		match = 0;
		for (i = 1; wild[i] && (i == 1 || wild[i] != ']'); i++)
		{
			if (char_eq (*fname, wild[i]))
			{
				match = i;
			}
		}
		if (wild[i] != ']' ? *fname != '[' : match == 0)
		{
			return False;
		}
		fname++;
		wild += i + 1;
		goto TailRecursion;

	  case '*':
		/* The * should match as much text as possible.  Start by
		 * trying to make it match all of the name, and if that doesn't
		 * work then back off until it does match or no match is
		 * possible.
		 */
		for (i = strlen (fname);
		     i >= 0 && !wildcmp (fname + i, wild + 1);
		     i--)
		{
		}
		return (BOOLEAN)(i >= 0);

	  case '\0':
		return ((*fname && !partial) ? False : True);

	  default:
		if (!char_eq (*fname, *wild))
		{
			return False;
		}
		fname++;
		wild++;
		goto TailRecursion;
	}
	/*NOTREACHED*/
}

/* Return the first filename (in a static buffer) that matches wildexpr,
 * or wildexpr itself if none matches.  If wildexpr contains no wildcards,
 * then just return wildexpr without checking for files.
 */
char *dirfirst (wildexpr, ispartial)
	char	*wildexpr;	/* a wildcard expression to search for */
	BOOLEAN	ispartial;	/* is this just the front part of a name? */
{
	char search_dir[CCHMAXPATH + 4];
	FILEFINDBUF3 fb;
	ULONG entries;
	APIRET rc;
	char *found;

	/* If no wildcard characters, just return the expression */
	if (!diriswild (wildexpr) && !ispartial)
	{
		hdir = NULLHANDLE;
		return wildexpr;
	}

	/* If a previous dirfirst()/dirnext() was left unfinished, then
	 * abandon it now.
	 */
	if (hdir != NULLHANDLE)
		DosFindClose (hdir);

	/* Open the directory for scanning.  If can't open, then return
	 * the wildexpr unchanged.
	 */
	wilddir = dirdir (wildexpr);
	strcpy (search_dir, wilddir);
    fix_slashes (search_dir);
    found = &search_dir[strlen (search_dir)];

    if ((found - search_dir < CCHMAXPATH-2) && (*(found-1) != OSPATHSEP)) {
		found[0] = OSPATHSEP;
		found[1] = '\0';
    }
	strcat (search_dir, "*.*");
	hdir = HDIR_CREATE;
	entries = 1;

	rc = DosFindFirst (search_dir, &hdir,
			          FILE_DIRECTORY | FILE_READONLY,
			          &fb, sizeof (fb), &entries, FIL_STANDARD);
	if (rc != NO_ERROR)
	{
		hdir = NULLHANDLE;
		return wildexpr;
	}

	/* Use dirnext to do most of the dirty work */
	wildfile = dirfile (wildexpr);
	partial = ispartial;
	if (match_file (&fb)) 
		found = dirpath (wilddir, fb.achName);
	else  
		found = dirnext ();
	return (found ? found : wildexpr);
}

/* Return the next filename (in a static buffer) that matches the
 * wildexpr of the previous dirfirst(), or NULL if no more files match.
 */
char *dirnext ()
{
	FILEFINDBUF3 fb;
	ULONG entries;
	APIRET rc;

	/* if no directory is open, then fail immediately */
	if (hdir == NULLHANDLE)
		return NULL;

	/* loop until we find a matching entry */
	do
	{
		entries = 1;
		rc = DosFindNext (hdir, &fb, sizeof(fb), &entries);
		if (rc != NO_ERROR || entries != 1)
		{
			/* if no entries matched, return NULL */
			DosFindClose (hdir);
			hdir = NULLHANDLE;
			return NULL;
		}
	}
	while (!match_file (&fb));

	/* combine the found name with the wilddir */
	return dirpath (wilddir, fb.achName);
}

static BOOLEAN
match_file (FILEFINDBUF3 *fb)
{
	if (strcmp (fb->achName, ".") == 0 || strcmp (fb->achName, "..") == 0)
		return False;
	else 
	    return wildcmp (fb->achName, wildfile);
}

/* return the file part of a pathname.  This particular implementation doesn't
 * use an internal buffer; it simply returns a pointer to the filename at the
 * end of the pathname.
 */
char *dirfile (pathname)
	char	*pathname;
{
	char	*slash;

	/* Convert slashes to backslashes. */
	fix_slashes (pathname);

	if ((slash = strrchr (pathname, OSPATHSEP)) == NULL)
	{
		slash = strrchr (pathname, ':');
	}

	return slash != NULL ? slash + 1 : pathname;
}

#endif /* !JUST_DIRPATH */


/* Return True if wildexpr contains any wildcards; else False */
BOOLEAN diriswild (wildexpr)
	char	*wildexpr;	/* either a filename or a wildcard expression */
{
#if 0
	char	*quote = '\0';

	while (*wildexpr)
	{
		if (!quote && strchr ("?*[", *wildexpr))
		{
			return True;
		}
		else if (quote == '\\' || *wildexpr == quote)
		{
			quote = '\0';
		}
		else if (!quote && strchr ("\"'`\\", *wildexpr))
		{
			quote = *wildexpr;
		}
	}
#else
	if (strpbrk (wildexpr, "?*["))
	{
		return True;
	}
#endif
	return False;
}


#ifndef JUST_DIRFIRST


/* Check a the type & permissions of a file.  Return one of the
 * following to describe the file's type & permissions:
 *    DIR_INVALID    malformed filename (can't happen with UNIX)
 *    DIR_BADPATH    unable to check file
 *    DIR_NOTFILE    directory or other non-file
 *    DIR_NEW        file doesn't exist yet
 *    DIR_UNREADABLE file exists but is unreadable
 *    DIR_READONLY   file is readable but not writable
 *    DIR_READWRITE  file is readable and writable.
 */
DIRPERM dirperm (filename)
	char	*filename;	/* name of file to check */
{
	struct stat st;
    int i;

    /* check for a protocol */
    for (i = 0; isalpha (filename[i]); i++) 
        /* nix */;
    /* endfor */
    if (i < 2 || filename[i] != ':')
        i = 0;

    /* skip past "file:" protocol; assume all others are readonly */
    if (!strncmp (filename, "file:", 5))
        filename += i + 1;
    else if (!strncmp (filename, "ftp:", 4))
        return DIR_READWRITE;
    else if (i > 0)
        return DIR_READONLY;


	if (stat (filename, &st) < 0)
	{
		if (errno == ENOENT)
			return DIR_NEW;
		else
			return DIR_BADPATH;
	}
	if ((st.st_mode & S_IFREG) == 0)
	{
		return DIR_NOTFILE;
	}
	else if ((st.st_mode & S_IREAD) == 0)
	{
		return DIR_UNREADABLE;
	}
	else if ((st.st_mode & S_IWRITE) == 0)
	{
		return DIR_READONLY;
	}
	else
	{
		return DIR_READWRITE;
	}
}

#ifndef JUST_DIRPATH

/* return the pathname of the current working directory */
char *dircwd ()
{
	static char	cwd[CCHMAXPATH];
	ULONG drive, drive_map;
	ULONG length;

	DosQueryCurrentDisk (&drive, &drive_map);
	cwd[0] = drive + 'A' - 1;
	cwd[1] = ':';
	cwd[2] = OSPATHSEP;

	length = sizeof (cwd) - 3;
	if (DosQueryCurrentDir (drive, cwd + 3, &length) == NO_ERROR)
	{
		fix_slashes (cwd);
		return cwd;
	}
	else
	{
		return ".";
	}
}

/* change the current directory, and return True if successful (else False) */
BOOLEAN dirchdir (pathname)
	char	*pathname;	/* new directory */
{
	if (strlen (pathname) >= 2
		&& isalpha (pathname[0])
		&& pathname[1] == ':')
	{
		if (DosSetDefaultDisk (tolower (pathname[0]) - 'a' + 1) != NO_ERROR)
		{
			return False;
		}
		pathname += 2;
	}

	if (pathname[0] != '\0')
		return (BOOLEAN)(DosSetCurrentDir (pathname) == NO_ERROR);
	else
		return True;
}

/* OS-dependent initialization function.  Here we initialize the o_elvispath
 * option to include the executables directory among the possible library
 * directories.
 */
void osinit (argv0)
	char	*argv0;
{
	char	*tmp;
	static	char	path[CCHMAXPATH];

	/* if argv0 isn't a pathname, then try to locate "elvis.exe" in
	 * the execution path.  We need this when figuring out the default
	 * value for the "elvispath" option and maybe the "home" option.
	 */
	if (!strchr (argv0, '\\') && access ("elvis.exe", 0) != 0)
	{
		tmp = getenv ("PATH");
		if (tmp != NULL)
		{
#ifndef ELVDUMP
			tmp = iopath (tmp, "elvis.exe", False);
			if (tmp != NULL)
			{
				argv0 = tmp;
			}
#endif
		}
	}

	/* Set the "home" option.  This is set (by order of preference) to
	 * one of the following:
	 *	- the value of the HOME environment variable, if set
	 *	- the pathname of the elvis.exe program, if known
	 *	- C:\
	 *
	 * NOTE: The portable code in "optglob.c" will override this value
	 * if the HOME environment variable is set.
	 */
	if (strchr (argv0, '\\'))
	{
		/* program name has a path.  Use it as home */
		optpreset (o_home, CHARdup (toCHAR (dirdir (argv0))), OPT_FREE);
	}
	else
	{
		/* last resort -- just use C:\ */
		o_home = toCHAR ("C:\\");
	}

	/* Set the "elvispath" option.  We want this to include ~\elvislib,
	 * plus the directory where ELVIS.EXE resides and the lib subdirectory
	 * under that directory.
	 *
	 * NOTE: The portable code in "optglob.c" will override this value
	 * if the ELVISPATH environment variable is set.
	 */
	tmp = dirdir (argv0);
	sprintf (path, "~\\elvislib;%s;%s", tmp, dirpath (tmp, "lib"));
	o_elvispath = toCHAR (path);
}

#endif /* !JUST_DIRPATH */
#endif /* !JUST_DIRFIRST */
