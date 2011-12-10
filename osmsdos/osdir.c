/* osmsdos/osdir.c */

/* This file contains functions for dealing with filenames.  This file's
 * structure is a little unusual because in addition to elvis itself, some
 * of the support programs also use a few of the functions defined here.
 * For the sake of efficiency with respect to those support programs,
 * this file can be compiled with JUST_DIRFIRST or JUST_DIRPATH defined
 * in order to exclude the functions they don't need.  If neither of those
 * names is defined, then the whole file is compiled.
 */

#if !defined(JUST_DIRFIRST) && !defined(JUST_DIRPATH)
# include "elvis.h"
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <dos.h>
#include <errno.h>
#ifdef  __TURBOC__
# include <dir.h>
extern unsigned _stklen = 16384U;
#endif
#ifdef M_I86
# include <direct.h>
# define findfirst(a,b,c)       _dos_findfirst(a,c,b)
# define findnext               _dos_findnext
# define ffblk                  find_t
# define ff_name                name
#endif

/* Microsoft has an annoying habit of putting underscores in front of
 * conventional names.
 */
#ifndef S_IFMT
# define S_IFMT		_S_IFMT
# define S_IFREG	_S_IFREG
# define S_IWRITE	_S_IWRITE
# define find_t		_find_t
# define stat		_stat
#endif
#ifndef A_RDONLY
# define A_RDONLY	_A_RDONLY
# define A_SUBDIR	_A_SUBDIR
#endif

#ifndef NAME_MAX
# ifdef MAXNAMLEN
#  define NAME_MAX MAXNAMLEN
# else
#  define NAME_MAX 255
# endif
#endif

#if !defined(JUST_DIRPATH)
/* These are used for communication between dirfirst() and dirnext() */
static char finddir[NAME_MAX];
static char findwild[NAME_MAX];
static char found[NAME_MAX];
static struct ffblk ff;

/* Return the first filename (in a static buffer) that matches
 * wildexpr, or wildexpr if none matches.  If wildexpr has no contains
 * no wildcards, then just return wildexpr without checking for files.
 */
char *dirfirst(char *wildexpr, BOOLEAN ispartial)
{
	/* remember the directory name */
	strcpy(finddir, dirdir(wildexpr));

	/* Copy the wildexpr into fildwild[].  If it is meant to be a partial
	 * name, then append "*" to it.
	 */
	strcpy(findwild, wildexpr);
	if (ispartial)
		strcat(findwild, strchr(findwild, '.') ? "*" : "*.*");

	/* if no match, then return the original wildexpr unchanged */
	if (findfirst(findwild, &ff, A_RDONLY|A_SUBDIR) != 0)
	{
		found[0] = '\0';
		return wildexpr;
	}

	/* combine the directory name with the found file's name */
	strcpy(found, dirpath(finddir, ff.ff_name));

	/* convert the filename to lowercase */
	for (wildexpr = found + strlen(found) - 1;
	     wildexpr >= found && *wildexpr != '\\' && *wildexpr != ':';
	     wildexpr--)
	{
		*wildexpr = tolower(*wildexpr);
	}
	return found;
}

/* Return the next filename (in a static buffer) that matches the
 * wildexpr of the previous dirfirst(), or NULL if no more files match.
 */
char *dirnext(void)
{
	char	*scan;

	/* if previous call returned NULL, then return NULL again. */
	if (!found[0])
		return NULL;

	/* if there is no match, then return NULL */
	if (findnext(&ff) != 0)
	{
		found[0] = '\0';
		return NULL;
	}

	/* combine the directory name with the found file's name */
	strcpy(found, dirpath(finddir, ff.ff_name));

	/* convert the filename to lowercase */
	for (scan = found + strlen(found) - 1;
	     scan >= found && *scan != '\\' && *scan != ':';
	     scan--)
	{
		*scan = tolower(*scan);
	}
	return found;
}
#endif /* !JUST_DIRPATH */

#if !defined(JUST_DIRFIRST) && !defined(JUST_DIRPATH)
/* Return True if wildexpr contains any wildcards; else False */
BOOLEAN diriswild(char *wildexpr)
{
	if (strchr(wildexpr, '*') || strchr(wildexpr, '?'))
	{
		return True;
	}
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
DIRPERM dirperm(char *filename)
{
	struct stat statb;

	if (stat(filename, &statb) < 0)
	{
		if (errno == ENOENT)
			return DIR_NEW;
		else
			return DIR_BADPATH;
	}
	if ((statb.st_mode & S_IFMT) != S_IFREG
	 || !strcmp(filename, "nul")
	 || !strcmp(filename, "prn"))
		return DIR_NOTFILE;
	if ((statb.st_mode & S_IWRITE) == 0)
		return DIR_READONLY;
	return DIR_READWRITE;
}
#endif /* !JUST_DIRFIRST && !JUST_DIRPATH */

/* return the directory part of a pathname */
char *dirdir(char *pathname)
{
 static char	ret[260];
	char	*slash;

	strcpy(ret, pathname);
	slash = strrchr(ret, '\\');
	if (slash == ret)
	{
		ret[1] = '\0';
	}
	else if (isalpha(ret[0]) && ret[1] == ':' && slash == &ret[2])
	{
		ret[3] = '\0';
	}
	else if (slash)
	{
		*slash = '\0';
	}
	else if (isalpha(ret[0]) && ret[1] == ':')
	{
		ret[2] = '\0';
	}
	else
	{
		strcpy(ret, ".");
	}
	return ret;
}

#if !defined(JUST_DIRFIRST) && !defined(JUST_DIRPATH)
/* return the file part of a pathname.  This particular implementation doesn't
 * use an internal buffer; it simply returns a pointer to the filename at the
 * end of the pathname.
 */
char *dirfile(char *pathname)
{
	char	*slash;

	slash = strrchr(pathname, '\\');
	if (!slash && isalpha(*pathname) && pathname[1] == ':')
	{
		slash = &pathname[1];
	}
	return slash ? slash + 1 : pathname;
}
#endif /* !JUST_DIRFIRST && !JUST_DIRPATH */

/* combine a directory name and a filename, yielding a pathname. */
char *dirpath(char *dir, char *file)
{
	static char	path[NAME_MAX + 1];

	if (!strcmp(dir, ".")
	 || !dir[0]
	 || (isalpha(file[0]) && file[1] == ':')
	 || !(isalpha(dir[0]) && dir[1] == ':') && file[0] == '\\')
	{
		/* no dir, or file has drive letter, or file is absolute within
		 * drive but dir doesn't specify drive.
		 */
		strcpy(path, file);
	}
	else if (isalpha(dir[0]) && dir[1] == ':'
				&& (!dir[2] || file[0] == '\\'))
	{
		/* dir has drive letter, and either dir has no directory name
		 * or file is absolute within drive.
		 */
		sprintf(path, "%.2s%s", dir, file);
	}
	else if (!dir[0] || dir[strlen(dir) - 1] != '\\')
	{
		/* dir ends without \, and file is relative to dir */
		sprintf(path, "%s\\%s", dir, file);
	}
	else
	{
		/* dir ends with \, and file is relative to dir */
		sprintf(path, "%s%s", dir, file);
	}
	return path;
}

#if !defined(JUST_DIRFIRST) && !defined(JUST_DIRPATH)
/* return the pathname of the current working directory */
char *dircwd(void)
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


/* Change working directory and return True if successful */
BOOLEAN dirchdir(pathname)
	char	*pathname;	/* new directory name */
{
	return (BOOLEAN)(chdir(pathname) >= 0);
}


/* OS-dependent initialization function.  Here we initialize the o_elvispath
 * option to include the executables directory among the possible library
 * directories.
 */
void osinit(argv0)
	char	*argv0;
{
 static char	path[260];
	char	*tmp;

	/* Set the "home" option to the directory which contains ELVIS.EXE.
	 * Later, the portable code in "optglob.c" will override this value
	 * if the HOME environment variable is set.
	 */
	optpreset(o_home, CHARdup(toCHAR(dirdir(argv0))), OPT_FREE);

	/* Set the "elvispath" option.  The value here should include
	 * ~/ELVISLIB, the directory where ELVIS.EXE resides, and the LIB
	 * subdirectory under that directory.  Later, the portable code in
	 * "optglob.c" will override this value if the ELVISPATH environment
	 * variable is set.
	 */
	tmp = dirdir(argv0);
	sprintf(path, "~\\elvislib;%s;%s", tmp, dirpath(tmp, "lib"));
	o_elvispath = toCHAR(path);
}

#endif /* !JUST_DIRFIRST && !JUST_DIRPATH */
