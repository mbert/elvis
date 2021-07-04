/* oswin32/osdir.c */

/* This file contains functions which deal with filenames.  The structure of
 * this file is a little unusual because some of the support programs also
 * use functions from this file.  To minimize the size of those support
 * programs, this file may be compiled with JUST_DIRFIRST or JUST_DIRPATH
 * defined to exclude the unused functions.  If neither of those names is
 * defined, then the whole file is compiled.
 */

#if !defined(JUST_DIRFIRST) && !defined(JUST_DIRPATH)
# include "elvis.h"
#endif
#define CHAR winCHAR
#include <windows.h> /* for registry and find functions */
#undef CHAR
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef MAX_PATH
# define MAX_PATH (_MAX_FNAME * 3)
#endif

#ifndef NEW_CYGWIN
# define NEW_CYGWIN 1	/* 1=parse output of mount command, 0=read registry */
#endif

#if defined(JUST_DIRFIRST) || defined(JUST_DIRPATH)
# undef elvalpha
# define elvalpha(c)	((unsigned)(((c) & 0xffdf) - 'A') < 26)
#endif /* JUST_DIRFIRST */

#ifndef JUST_DIRPATH

/* This recursive function checks a filename against a wildcard expression.
 * Returns ElvTrue for match, ElvFalse for mismatch.
 *
 * For Windows this is case-insensitive and supports the *, ?, and [] wildcards.
 * However, the contents of [] are case-sensitive.
 */
ELVBOOL dirwildcmp(fname, wild)
	char	*fname;	/* an actual filename */
	char	*wild;	/* a wildcard expression */
{
	int	i;
	int	match, negate;
	int	c1, c2;

  TailRecursion:
	switch (*wild)
	{
	  case '/':
	  case '\\':
		/* / and \ match each other */
		if (*fname != '/' && *fname != '\\')
			return ElvFalse;
		fname++;
		wild++;
		goto TailRecursion;

	  case '?':
		/* match any single character except \0 and / */
		if (*fname == '/' || !*fname)
		{
			return ElvFalse;
		}
		fname++;
		wild++;
		goto TailRecursion;

	  case '[':
		/* if no matching ], then compare [ as literal character;
		 * else next char in fname must be in brackets
		 */

		/* if it starts with '^' we'll want to negate the result */
		match = negate = ElvFalse;
		i = 1;
		if (wild[i] == '^')
		{
			negate = ElvTrue;
			i++;
		}

		/* Compare this fname char to each bracketed char.  This is
		 * a little tricky because 1) the ']' char can be included in
		 * the brackets in unambiguous contexts, 2) we need to support
		 * ranges of characters, and 3) there might not be a closing ']'
		 */
		for (; wild[i] && (i == 1 || wild[i] != ']'); i++)
		{
			if (*fname == wild[i])
			{
				match = ElvTrue;
			}
			if (wild[i + 1] == '-' && wild[i + 2])
			{
				if (*fname > wild[i] && *fname <= wild[i + 2])
					match = ElvTrue;
				i += 2;
			}
		}

		/* If no ']' then this fname character must be '[' (i.e., treat
		 * the '[' like a literal character).  However, if there is a
		 * ']' then use the result of the bracket matching if not in
		 * the brackets (or not out if "negate" is set) then fail.
		 */
		if (wild[i] != ']' ? *fname != '[' : match == negate)
		{
			return ElvFalse;
		}

		/* It matched!  Advance fname and wild, then loop */
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
		     i >= 0 && !dirwildcmp(fname + i, wild + 1);
		     i--)
		{
		}
		return (ELVBOOL)(i >= 0);

	  case '\0':
		return ((*fname) ? ElvFalse : ElvTrue);

	  default:
		c1 = *fname++;
		if (c1 >= 'A' && c1 <= 'Z')
			c1 += 'a' - 'A';
		c2 = *wild++;
		if (c2 >= 'A' && c2 <= 'Z')
			c2 += 'a' - 'A';
		if (c1 != c2)
			return ElvFalse;
		goto TailRecursion;
	}
	/*NOTREACHED*/
}


/* These are CygWin mount points */
static struct mtab_s
{
	char	native[MAX_PATH];	/* Windows name for a directory */
	char	unix[MAX_PATH];		/* Unix name for a directory */
	int	unixlen;		/* length of unix[] */
} mtab[10];
static int nmtabs;	/* number of used entries in mtab[] */

/* These are used for communication between dirfirst() and dirnext() */
static char			finddir[MAX_PATH];
static char			findwild[MAX_PATH];
static char			findbracket[MAX_PATH];
static char			*findbase;
static char			found[MAX_PATH];
static WIN32_FIND_DATA	FileData;
static HANDLE			hSearch;

/* adjust a file path via the Cygwin mount table */
char *dirnormalize(char *path)
{
	static char	winp[MAX_PATH + 1];
	int		i;

	/* convert slashes to backslashes */
	for (i = 0; path[i]; i++)
		if (path[i] == '/')
			winp[i] = '\\';
		else
			winp[i] = path[i];
	winp[i] = '\0';

	/* check it against Cygwin's mount table */
	for (i = 0; i < nmtabs; i++)
		if (!strncmp(winp, mtab[i].unix, mtab[i].unixlen)
		 && (!winp[mtab[i].unixlen]
		    || (winp[mtab[i].unixlen] == '\\' && mtab[i].unixlen > 1)))
			break;
	if (i < nmtabs)
	{
		strcpy(winp, mtab[i].native);
		strcat(winp, path + mtab[i].unixlen);
	}
	return winp;
}

/* Return the first filename (in a static buffer) that matches
 * wildexpr, or wildexpr if none matches.  If wildexpr has no contains
 * no wildcards, then just return wildexpr without checking for files.
 */
char *dirfirst(char *wildexpr, ELVBOOL ispartial)
{
	char *src, *dst, *peek;

	/* remember the directory name */
	strcpy(finddir, dirdir(wildexpr));

	/* Copy the wildexpr into findbracket[].  If it is meant to be a
	 * partial expression then append "*" to it.
	 */
	strcpy(findbracket, dirnormalize(wildexpr));
	if (ispartial)
		strcat(findbracket, "*");

	/* convert any bracket wildcards into ? wildcards */
	for (src = findbracket, dst = findwild; *src; )
	{
		if (*src == '[')
		{
			/* find the end of the bracket list.  This is tricky
			 * because ']' is allowed after the initial '[' or '[^'
			 * and at the end of a '-' range.
			 */
			peek = src + 1;
			if (*peek == '^')
				peek++;
			if (*peek == ']')
				peek++;
			for (; *peek && *peek != ']'; peek++)
				if (peek[1] == '-' && peek[2])
					peek += 2;

			/* if no end, then '[' is literal.  Else use '?' for
			 * the entire bracket expression.
			 */
			if (!*peek)
			{
				*dst++ = '[';
				src++;
			}
			else
			{
				*dst++ = '?';
				src = peek + 1;
			}
		}
		else
			*dst++ = *src++;
	}
	*dst = '\0';

	/* if the wildexpr (now findbracket) contains a directory name, then
	 * we want to pass the whole directory name to FindFirstFile, but only
	 * the last part of it to dirwildcmp
	 */
	findbase = strrchr(findbracket, '\\');
	if (findbase)
		findbase++;
	else
		findbase = findbracket;

	/* if no match, then return the original wildexpr unchanged */
	FileData.cFileName[0] = '\0';
	hSearch = FindFirstFile(findwild, &FileData);
	while (hSearch != INVALID_HANDLE_VALUE && (*FileData.cFileName == '.' || !dirwildcmp(FileData.cFileName, findbase)))
	{
		if (FindNextFile(hSearch, &FileData) == FALSE)
		{
			FindClose(hSearch);
			hSearch = INVALID_HANDLE_VALUE;
		}
	}
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		found[0] = '\0';
		return wildexpr;
	}

	/* combine the directory name with the found file's name */
	strcpy(found, dirpath(finddir, FileData.cFileName));

	return found;
}

/* Return the next filename (in a static buffer) that matches the
 * wildexpr of the previous dirfirst(), or NULL if no more files match.
 */
char *dirnext(void)
{
	/* if previous call returned NULL, then return NULL again. */
	if (!found[0])
		return NULL;

	/* if there is no match, then return NULL */
	do
	{
		if (FindNextFile(hSearch, &FileData) == FALSE)
		{
			found[0] = '\0';
			FindClose(hSearch);
			return NULL;
		}
	} while (*FileData.cFileName == '.' || !dirwildcmp(FileData.cFileName, findbase));

	/* combine the directory name with the found file's name */
	strcpy(found, dirpath(finddir, FileData.cFileName));

	return found;
}

#ifndef JUST_DIRFIRST

/* Return ElvTrue if wildexpr contains any wildcards; else ElvFalse */
ELVBOOL diriswild(char *wildexpr)
{
	if (nmtabs > 0 || strchr(wildexpr, '*') || strchr(wildexpr, '?'))
	{
		return ElvTrue;
	}
	return ElvFalse;
}


/* Check a the type & permissions of a file.  Return one of the
 * following to describe the file's type & permissions:
 *    DIR_INVALID    malformed filename (can't happen with UNIX)
 *    DIR_BADPATH    unable to check file
 *    DIR_NOTFILE    file exists but is neither normal nor a directory
 *    DIR_DIRECTORY  file is a directory
 *    DIR_NEW        file doesn't exist yet
 *    DIR_UNREADABLE file exists but is unreadable
 *    DIR_READONLY   file is readable but not writable
 *    DIR_READWRITE  file is readable and writable.
 */
DIRPERM dirperm(char *filename)
{
	struct _stat statb;
	int	i;

	/* check for a protocol */
	for (i = 0; elvalpha(filename[i]); i++)
	{
	}
	if (i < 2 || filename[i] != ':')
		i = 0;

	/* skip past "file:" protocol; assume all others are readonly */
	if (!strncmp(filename, "file:", 5))
		filename += i + 1;
	else if (!strncmp(filename, "ftp:", 4))
		return DIR_READWRITE;
	else if (i > 0)
		return DIR_READONLY;

	if (_stat(dirnormalize(filename), &statb) < 0)
	{
		if (errno == ENOENT)
			return DIR_NEW;
		else
			return DIR_BADPATH;
	}
	if ((statb.st_mode & _S_IFMT) != _S_IFREG)
		return DIR_DIRECTORY;
	if (!strcmp(filename, "nul")
	 || !strcmp(filename, "prn")
	 || !strncmp(filename, "lpt", 3))
		return DIR_NOTFILE;
	if ((statb.st_mode & _S_IWRITE) == 0)
		return DIR_READONLY;
	return DIR_READWRITE;
}
#endif /* !JUST_DIRFIRST */
#endif /* !JUST_DIRPATH */

/* return the directory part of a pathname */
char *dirdir(char *pathname)
{
 static char	ret[MAX_PATH];
	char	*slash;

	/* break the name at the last backslash */
	strcpy(ret, pathname);
	for (slash = &ret[strlen(ret)];
	     --slash >= ret && *slash != '/' && *slash != '\\';
	     )
	{
	}
	if (slash == ret)
	{
		ret[1] = '\0';
	}
	else if (elvalpha(ret[0]) && ret[1] == ':' && slash == &ret[2])
	{
		ret[3] = '\0';
	}
	else if (slash >= ret)
	{
		*slash = '\0';
	}
	else if (elvalpha(ret[0]) && ret[1] == ':')
	{
		ret[2] = '\0';
	}
	else
	{
		strcpy(ret, ".");
	}
	return ret;
}

#ifndef JUST_DIRPATH
/* return the file part of a pathname.  This particular implementation doesn't
 * use an internal buffer; it simply returns a pointer to the filename at the
 * end of the pathname.
 */
char *dirfile(char *pathname)
{
	char	*backslash;
	char	*slash;

	backslash = strrchr(pathname, '\\');
	slash = strchr(pathname, '/');
	if (!slash || (backslash && backslash > slash))
		slash = backslash;
	if (!slash)
	{
		slash = strrchr(pathname, ':');
	}
	if (!slash)
	{
		slash = pathname - 1;
	}
	return slash + 1;
}
#endif /* !JUST_DIRPATH */


/* combine a directory name and a filename, yielding a pathname. */
char *dirpath(char *dir, char *file)
{
	static char	path[MAX_PATH + 1];
	char		*build = path;

	/* if dir and file are both "" then simply return a backslash */
	if (!*dir && !*file)
		return "\\";

	/* choose a drive letter, from either dir or file.  If file is used,
	 * then ignore dir completely.
	 */
	if (elvalpha(file[0]) && file[1] == ':')
	{
		*build++ = *file++;
		*build++ = *file++;
		dir = ".";
	}
	else if (elvalpha(dir[0]) && dir[1] == ':')
	{
		*build++ = *dir++;
		*build++ = *dir++;
	}

	/* if file has absolute path, or dir is ".", then ignore dir.  Else
	 * add dir to the path now, deleting the terminal backslash (if any).
	 */
	if (file[0] != '/' && file[0] != '\\' && strcmp(dir, "."))
	{
		while (*dir)
		{
			if (*dir == '/')
				*build++ = '\\';
			else
				*build++ = *dir;
			dir++;
		}
	}

	/* add a backslash, if necessary. */
	if (build != path && build[-1] != ':' && build[-1] != '\\' && *file != '/' && *file != '\\')
		*build++ = '\\';

	/* add the filename */
	while (*file)
	{
		if (*file == '/')
			*build++ = '\\', file++;
		else
			*build++ = *file++;
	}
	*build = '\0';

	return path;
}

#ifndef JUST_DIRFIRST

/* Return a file's timestamp, or current time if filename is NULL or "".
 * The normal return format is "YYYY-MM-DDThh:mm:ss", but for invalid files
 * it returns "".
 */
char *dirtime(filename)
	char	*filename;
{
	static char	str[20];
	SYSTEMTIME	when;
	FILETIME	localftime;
	WIN32_FIND_DATA	info;
	HANDLE		handle;

	/* For NULL or "", get current time */
	if (!filename || !*filename)
	{
		GetLocalTime(&when);
	}
	else /* get timstamp of a file */
	{
		/* Get file info.  If that fails, return "" */
		handle = FindFirstFile(filename, &info);
		if (handle == INVALID_HANDLE_VALUE)
			return "";
		FindClose(handle);

		/* Convert timestamp to local time */
		if (!FileTimeToLocalFileTime(&info.ftLastWriteTime , &localftime)
		 || !FileTimeToSystemTime(&localftime, &when))
			return "";
	}

	/* Convert to string, and return it */
	sprintf(str, "%04d-%02d-%02dT%02d:%02d:%02d",
		when.wYear, when.wMonth, when.wDay,
		when.wHour, when.wMinute, when.wSecond);
	return str;
}

#ifndef JUST_DIRPATH

/* return the pathname of the current working directory */
char *dircwd(void)
{
	static char	cwd[MAX_PATH + 1];

	if (_getcwd(cwd, sizeof cwd))
	{
		return cwd;
	}
	else
	{
		return ".";
	}
}


/* Change working directory and return ElvTrue if successful */
ELVBOOL dirchdir(pathname)
	char	*pathname;	/* new directory name */
{
	return (ELVBOOL)(_chdir(pathname) >= 0);
}


/* OS-dependent initialization function.  Here we initialize the o_elvispath
 * option to include the executables directory among the possible library
 * directories.
 */
void osinit(argv0)
	char	*argv0;
{
	char	*tmp;
 static	char	path[260];
	char    modulep[MAX_PATH];
	int	i, j;
	struct mtab_s swapper;
#if NEW_CYGWIN
	ELVBOOL oldhide;
	CHAR	ch, line[200];
#else
	DWORD	dw, dw2;
	FILETIME when;
	char	name[200];
	char	value[200];
	HKEY	hCygnus, hMounts, hKey;
#endif

       /* if argv0 isn't a pathname, then try the Windows API call
	* GetModuleFileName to retrieve the pathname of the executable file 
	* associated with the current process. If that fails, try to locate 
	* "elvis.exe" in the execution path.  We need this when figuring 
	* out the default value for the "elvispath" option and maybe the 
	* "home" option.
	 */
	if (!strchr(argv0, '\\') && _access("elvis.exe", 0) != 0)
	{
	       if(GetModuleFileName(NULL, modulep, sizeof(modulep))) 
		{
		       argv0 = modulep;
		} 
		else 
		{
			tmp = getenv("PATH");
			if (tmp)
			{
				tmp = iopath(tmp, "elvis.exe", ElvFalse);
				if (tmp)
				{
					argv0 = tmp;
				}
			}
		}
	}

	/* Set the "home" option.  This is set (by order of preference) to
	 * one of the following:
	 *	- the value of the HOME environment variable, if set
	 *	- the values of HOMEDRIVE and HOMEPATH, if set
	 *	- the pathname of the elvis.exe program, if known
	 *	- C:\
	 *
	 * NOTE: The portable code in "optglob.c" will override this value
	 * if the HOME environment variable is set.
	 */
	if (getenv("HOMEDRIVE") && getenv("HOMEPATH"))
	{
		/* derive from HOMEDRIVE and HOMEPATH */
		optpreset(o_home, (CHAR *)safealloc(strlen(getenv("HOMEPATH")) + 3, sizeof(CHAR)), OPT_FREE);
		sprintf(tochar8(o_home), "%.2s%s", getenv("HOMEDRIVE"), getenv("HOMEPATH"));
	}
	else if (strchr(argv0, '\\'))
	{
		/* program name has a path.  Use it as home */
		optpreset(o_home, CHARdup(toCHAR(dirdir(argv0))), OPT_FREE);
	}
	else
	{
		/* last resort -- just use C:\ */
		o_home = toCHAR("C:\\");
	}

	/* Set the "elvispath" option.  We want this to include ~\dotelvis,
	 * plus the "data" and "doc" subdirectories under the directory where
	 * ELVIS.EXE resides
	 *
	 * NOTE: The portable code in "optglob.c" will override this value
	 * if the ELVISPATH environment variable is set.
	 */
	tmp = dirdir(argv0);
	sprintf(path, "~\\dotelvis;%s\\data;%s\\doc", tmp, tmp);
	o_elvispath = toCHAR(path);

	/* Read the mounts for the latest Cygwin version */
	nmtabs = 0;
#if NEW_CYGWIN

	oldhide = msghide(TRUE);
	if (prgopen("mount", FALSE, TRUE))
	{
		for (i = 0; prgread(&ch, 1) == 1 && nmtabs < QTY(mtab); )
		{
			if (ch == '\n')
			{
				line[i] = '\0';
				for (i = 0; line[i] && strncmp(&line[i], " on /", 5); i++)
				{
				}
				if (line[i])
				{
					line[i] = '\0';
					for (j = i + 4; line[j] && strncmp(&line[j], " type ", 6); j++)
					{
					}
					if (line[j])
					{
						line[j] = '\0';
						strcpy(mtab[nmtabs].native, line);
						strcpy(mtab[nmtabs].unix, line + i + 4);
						mtab[nmtabs].unixlen = strlen(mtab[nmtabs].unix);
						nmtabs++;
					}
				}
				
				/* prepare for next line */
				i = 0;
			}
			else if (ch >= ' ' && i < QTY(line) - 1)
			{
				line[i++] = ch;
			}
		}
		prgclose();
	}
	(void)msghide(oldhide);

#else /* not NEW_CYGWIN */

	hCygnus = hMounts = hKey = 0;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Cygnus Solutions\\CYGWIN.DLL setup", 0L, KEY_READ|KEY_EXECUTE, &hCygnus) == ERROR_SUCCESS)
	{
		/* there may be multiple versions.  Use the most recent one */
		for (i = 0, dw = sizeof name, *value = '\0';
		     RegEnumKeyEx(hCygnus, i, name, &dw, NULL, NULL, NULL, &when) == ERROR_SUCCESS;
		     i++, dw = sizeof name)
		{
			if (strcmp(*name=='b' ? name+1 : name,
				   *value=='b' ? value+1 : value) > 0)
				strcpy(value, name);
		}
		strcpy(name, value);
		strcat(name, "\\mounts");
		if (RegOpenKeyEx(hCygnus, name, 0L, KEY_READ|KEY_EXECUTE, &hMounts) == ERROR_SUCCESS)
		{
			for (i = 0, dw = sizeof name;
			     nmtabs < QTY(mtab) && RegEnumKeyEx(hMounts, i, name, &dw, NULL, NULL, NULL, &when) == ERROR_SUCCESS;
			     i++, dw = sizeof name)
			{
				/* print the mount table entry */
				if (RegOpenKeyEx(hMounts, name, 0L, KEY_READ|KEY_EXECUTE, &hKey) == ERROR_SUCCESS)
				{
					dw = sizeof mtab[nmtabs].native;
					if (RegQueryValueEx(hKey, "native", NULL, &dw2, mtab[nmtabs].native, &dw) != ERROR_SUCCESS
					 || dw2 != REG_SZ
					 || !(dw = sizeof mtab[nmtabs].unix) /* yes, ASSIGNMENT! */
					 || RegQueryValueEx(hKey, "unix", NULL, &dw2, mtab[nmtabs].unix, &dw) != ERROR_SUCCESS
					 || dw2 != REG_SZ)
						msg(MSG_WARNING, "Cygwin mount table contains malformed entries");
					else
						nmtabs++;
					RegCloseKey(hKey);
				}
			}
			RegCloseKey(hMounts);
		}
		RegCloseKey(hCygnus);
	}
#endif

	/* convert forward slashes in mtab[] to backslashes */
	for (i = 0; i < nmtabs; i++)
	{
		while ((tmp = strchr(mtab[i].native, '/')) != NULL)
			*tmp = '\\';
		while ((tmp = strchr(mtab[i].unix, '/')) != NULL)
			*tmp = '\\';
		mtab[i].unixlen = strlen(mtab[i].unix);
	}

	/* sort mtab[] by the length of the unix name (longest first) */
	for (i = 0; i < nmtabs - 1; i++)
		for (j = i + 1; j < nmtabs; j++)
			if (mtab[i].unixlen < mtab[j].unixlen)
			{
				swapper = mtab[i];
				mtab[i] = mtab[j];
				mtab[j] = swapper;
			}
}

#endif /* !JUST_DIRPATH */
#endif /* !JUST_DIRFIRST */
