/* unix/osdir.c */

/* This file contains functions that deal with filenames.  The structure of
 * this file is a little unusual, because some of the support programs also
 * use functions from this file.  To minimize the size of those support
 * programs, this file may be compiled with JUST_DIRFIRST or JUST_DIRPATH
 * defined to exclude the unused functions.  If neither of those names is
 * defined then the whole file is compiled.
 */

char id_osdir[] = "$Id: osdir.c,v 2.16 1996/09/20 23:56:25 steve Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#ifndef S_ISREG
# define S_ISREG(mode)	(((mode) & 0070000) == 0)
#endif
#if !defined(JUST_DIRFIRST) && !defined(JUST_DIRPATH)
# include "elvis.h"
#endif

#ifndef JUST_DIRPATH
# if USE_PROTOTYPES
static BOOLEAN wildcmp(char *fname, char *wild);
# endif
#endif

#ifndef NAME_MAX
# ifdef MAXNAMLEN
#  define NAME_MAX MAXNAMLEN
# else
#  define NAME_MAX 1024
# endif
#endif

#ifndef JUST_DIRPATH

/* This is the wildcard expression used by dirfirst() and dirnext() */
static char *wildfile;

/* This is the name of the directory being scanned by dirfirst() and dirnext() */
static char *wilddir;

/* This is the directory stream used by dirfirst() and dirnext() */
static DIR *dirfp;

/* This variable is True if the wildcard expression only needs to match the
 * beginning of the name, or False if it must match the entire filename.
 */
static BOOLEAN partial;

/* This recursive function checks a filename against a wildcard expression.
 * Returns True for match, False for mismatch.
 */
static BOOLEAN wildcmp(fname, wild)
	char	*fname;	/* an actual filename */
	char	*wild;	/* a wildcard expression */
{
	int	i, match;

  TailRecursion:
	switch (*wild)
	{
	  case '\\':
		/* character after \ must match exactly, except \0 and / */
		if (*fname == '/' || !*fname || *fname != wild[1])
		{
			return False;
		}
		fname++;
		wild += 2;
		goto TailRecursion;

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
			if (*fname == wild[i])
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
		for (i = strlen(fname);
		     i >= 0 && !wildcmp(fname + i, wild + 1);
		     i--)
		{
		}
		return (BOOLEAN)(i >= 0);

	  case '\0':
		return ((*fname && !partial) ? False : True);

	  default:
		if (*fname != *wild)
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
char *dirfirst(wildexpr, ispartial)
	char	*wildexpr;	/* a wildcard expression to search for */
	BOOLEAN	ispartial;	/* is this just the front part of a name? */
{
	char	*found;

	/* If no wildcard characters, just return the expression */
	if (!diriswild(wildexpr) && !ispartial)
	{
		dirfp = (DIR *)0;
		return wildexpr;
	}

	/* If a previous dirfirst()/dirnext() was left unfinished, then
	 * abandon it now.
	 */
	if (dirfp)
	{
		closedir(dirfp);
	}

	/* Open the directory for scanning.  If can't open, then return
	 * the wildexpr unchanged.
	 */
	wilddir = dirdir(wildexpr);
	dirfp = opendir(wilddir);
	if (!dirfp)
	{
		return wildexpr;
	}

	/* Use dirnext to do most of the dirty work */
	wildfile = dirfile(wildexpr);
	partial = ispartial;
	found = dirnext();
	return (found ? found : wildexpr);
}

/* Return the next filename (in a static buffer) that matches the
 * wildexpr of the previous dirfirst(), or NULL if no more files match.
 */
char *dirnext()
{
	struct dirent *dent;

	/* if no directory is open, then fail immediately */
	if (!dirfp)
	{
		return (char *)0;
	}

	/* loop until we find a matching entry, or end of directory.
	 * Note that we're careful about not allowing the ? and * wildcards
	 * match a . at the start of a filename; those files are supposed to
	 * be hidden.
	 */
	while ((dent = readdir(dirfp)) != NULL
	    && ((dent->d_name[0] == '.' && (wildfile[0] != '.' || wildfile[0] != '['))
		|| !wildcmp(dent->d_name, wildfile)))
	{
	}

	/* if no entries matched, return NULL */
	if (!dent)
	{
		closedir(dirfp);
		dirfp = NULL;
		return NULL;
	}

	/* combine the found name with the wilddir */
	return dirpath(wilddir, dent->d_name);
}

#ifndef JUST_DIRFIRST

/* Return True if wildexpr contains any wildcards; else False */
BOOLEAN diriswild(wildexpr)
	char	*wildexpr;	/* either a filename or a wildcard expression */
{
#if 0
	char	*quote = '\0';

	while (*wildexpr)
	{
		if (!quote && strchr("?*[", *wildexpr))
		{
			return True;
		}
		else if (quote == '\\' || *wildexpr == quote)
		{
			quote = '\0';
		}
		else if (!quote && strchr("\"'`\\", *wildexpr))
		{
			quote = *wildexpr;
		}
	}
#else
	if (strpbrk(wildexpr, "?*[\\"))
	{
		return True;
	}
#endif
	return False;
}


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
DIRPERM dirperm(filename)
	char	*filename;	/* name of file to check */
{
	struct stat st;

	if (stat(filename, &st) < 0)
	{
		if (errno == ENOENT)
			return DIR_NEW;
		else
			return DIR_BADPATH;
	}
	if (!S_ISREG(st.st_mode))
	{
		return DIR_NOTFILE;
	}
	else if (access(filename, 4) < 0)
	{
		return DIR_UNREADABLE;
	}
	else if (access(filename, 2) < 0)
	{
		return DIR_READONLY;
	}
	else
	{
		return DIR_READWRITE;
	}
}

/* return the file part of a pathname.  This particular implementation doesn't
 * use an internal buffer; it simply returns a pointer to the filename at the
 * end of the pathname.
 */
char *dirfile(pathname)
	char	*pathname;
{
	char	*slash;

	slash = strrchr(pathname, '/');
	if (slash)
	{
		return slash + 1;
	}
	else
	{
		return pathname;
	}
}

#endif /* !JUST_DIRFIRST */
#endif /* !JUST_DIRPATH */

/* return the directory part of a pathname */
char *dirdir(pathname)
	char	*pathname;
{
	static char dir[NAME_MAX + 1];
	char	*slash;

	strncpy(dir, pathname, sizeof dir);
	slash = strrchr(dir, '/');
	if (slash == dir)
	{
		return "/";
	}
	else if (slash)
	{
		*slash = '\0';
		return dir;
	}
	else
	{
		return ".";
	}
}

#ifndef JUST_DIRFIRST

/* combine a directory name and a filename, yielding a pathname. */
char *dirpath(dir, file)
	char	*dir;	/* directory name */
	char	*file;	/* filename */
{
	static char	path[NAME_MAX + 1];
	int		len;

	if (*file == '/' || !strcmp(dir, "."))
	{
		strcpy(path, file);
	}
	else
	{
		len = strlen(dir);
		if (len > 0 && dir[len - 1] == '/')
			len--;
		sprintf(path, "%.*s/%s", len, dir, file);
	}
	return path;
}

#ifndef JUST_DIRPATH

/* return the pathname of the current working directory */
char *dircwd()
{
	static char	cwd[NAME_MAX + 1];

	if (getcwd(cwd, sizeof cwd))
	{
		return cwd;
	}
	else
	{
		return ".";
	}
}

/* change the current directory, and return True if successful (else False) */
BOOLEAN dirchdir(pathname)
	char	*pathname;	/* new directory */
{
	return (BOOLEAN)(chdir(pathname) >= 0);
}

#endif /* !JUST_DIRPATH */
#endif /* !JUST_DIRFIRST */
