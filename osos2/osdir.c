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
 *
 * $Log: osdir.c,v $
 * Revision 1.15  2003/10/23 23:35:45  steve
 * Herbert's latest changes.
 *
 * Revision 1.13  2003/10/12 18:36:18  steve
 * Changed elvis' ctype macro names to start with "elv"
 *
 * Revision 1.12  2003/10/07 15:44:07  steve
 * Tweaks from Herbert since 2.2i-beta.
 * Changed version number to 2.2
 *
 * Revision 1.10  2003/09/20 16:48:07  steve
 * Added OS-dependent dirwildcmp() function to perform wildcard comparisons.
 * Modified :autocmd to use dirwildcmp().
 *
 * Revision 1.9  2003/08/28 18:15:23  steve
 * Split the "lib" directory into separate "data" and "doc" directories.
 *
 * Revision 1.8  2001/10/30 15:49:58  steve
 * From Herbert: make dirtime() available to the ref program.
 *
 * Revision 1.7  2001/10/23 01:37:09  steve
 * Sometweaks of FEATURE_XXXX names
 *
 * Revision 1.6  2001/10/22 18:23:14  steve
 * Added FEATURE_RCSID compile-time option
 *
 * Revision 1.5  2001/04/20 00:00:37  steve
 * Some bug fixes, and uglification of the source code.
 *
 * Revision 1.2  2000/06/04 10:26:39  HERBERT
 * Fixed a bug in dirdir() which made it impossible to handle file specs like
 * "c:*.bat". Also some formatting and CVS Logging.
 *
 *
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#ifdef __IBMC__ /* Herbert: not needed for emx/gcc */
# include <direct.h>
# include <sys/stat.h>
#endif
#include <io.h>
#include <string.h>
#if !defined(JUST_DIRFIRST) && !defined(JUST_DIRPATH)
# include "elvis.h"
# ifdef FEATURE_RCSID
char id_osdir[] = "$Id: osdir.c,v 1.15 2003/10/23 23:35:45 steve Exp $";
# endif
#endif

#ifdef JUST_DIRFIRST
/* will do for ctags, 
 * -- avoids needing stuff from digraph.c */
# undef elvtolower
# define elvtolower(c) (c | 0x20)
# undef elvalpha
# define elvalpha(c)  ((unsigned)(((c) & 0xffdf) - 'A') < 26)
#endif

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#ifdef __EMX__
/*# define USE_OS2_TOOLKIT_HEADERS*/
#define CHAR OS2CHAR
#endif
#include <os2.h>
#ifdef CHAR
# undef CHAR
#endif

#ifndef JUST_DIRPATH
# if USE_PROTOTYPES
static ELVBOOL match_file (FILEFINDBUF3 *fb);
# endif
static void fix_slashes (char *path);
#endif

#define char_eq(c1, c2)  (elvtolower(c1) == elvtolower(c2))

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
        {
          *cp = OSPATHSEP;
        }
    }
}


/* return the directory part of a pathname */
char *
dirdir (char *pathname)
{
  static char dir[CCHMAXPATH];
  char  *slash;

  strncpy (dir, pathname, sizeof dir);
  for (slash = &dir[strlen(dir)]; 
       --slash >= dir && *slash != '/' && *slash != '\\';)
    {
    }

  if (slash == dir)
    {
      dir[1] = '\0';
    }
  else if (elvalpha (dir[0]) && dir[1] == ':' && slash == &dir[2])
    {
      dir[3] = '\0';
    }
  else if (slash >= dir)
    {
      *slash = '\0';
    }
  else if (elvalpha (dir[0]) && dir[1] == ':')
    {
      /* append a dot as this is the current directory
       * on a different drive... */
      dir[2] = '.';
      dir[3] = '\0';
    }
  else
    {
      strcpy (dir, ".");
    }

  return dir;
}

/* combine a directory name and a filename, yielding a pathname. */
char *
dirpath (char *dir, /* directory name */
         char *file)/* filename */
{
  static char  path[CCHMAXPATH];

  /* Convert slashes to backslashes. */
  fix_slashes (dir);
  fix_slashes (file);

  if (strcmp (dir, ".") == 0
      || dir[0] == '\0'
      || (elvalpha (file[0]) && file[1] == ':')
      || (!(elvalpha (dir[0]) && dir[1] == ':') && file[0] == OSPATHSEP))
    {
      /* no dir, or file has drive letter, or file is absolute within
       * drive but dir doesn't specify drive.
       */
      strcpy (path, file);
    }
  else if (elvalpha (dir[0]) && dir[1] == ':' && file[0] == OSPATHSEP)
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

/* This variable is ElvTrue if the wildcard expression only needs to match the
 * beginning of the name, or ElvFalse if it must match the entire filename.
 */
static ELVBOOL partial;

/* This recursive function checks a filename against a wildcard expression.
 * Returns ElvTrue for match, ElvFalse for mismatch.
 *
 * For OS/2 this is case-insensitive and supports the *, ?, and [] wildcards.
 * The characters inside [] are case-sensitive, however.
 */
ELVBOOL dirwildcmp(fname, wild)
  char	*fname;	/* an actual filename */
  char	*wild;	/* a wildcard expression */
{
  int	i;
  ELVBOOL	match, negate;

  TailRecursion:
  switch (*wild)
    {
      case '\\':
        /* character after \ must match exactly, except \0 and / */
        if (*fname == '/' || !*fname || *fname != wild[1])
          {
            return ElvFalse;
          }
        fname++;
        wild += 2;
        goto TailRecursion;

      case '?':
        /* match any single character except \0 and / */
        if (*fname == '/' || *fname == '\\' || !*fname)
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
        return ((*fname && !partial) ? ElvFalse : ElvTrue);

      default:
        if (!char_eq (*fname, *wild))
          {
            return ElvFalse;
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
char *
dirfirst (char  *wildexpr,  /* a wildcard expression to search for */
          ELVBOOL  ispartial)  /* is this just the front part of a name? */
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
    {
      DosFindClose (hdir);
    }

  /* Open the directory for scanning.  If can't open, then return
   * the wildexpr unchanged.
   */
  wilddir = dirdir (wildexpr);
  strcpy (search_dir, wilddir);
  fix_slashes (search_dir);
  found = &search_dir[strlen (search_dir)];

  if ((found - search_dir < CCHMAXPATH-2) && (*(found-1) != OSPATHSEP)) 
    {
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
    {
      found = dirpath (wilddir, fb.achName);
    }
  else  
    {
      found = dirnext ();
    }
  return (found ? found : wildexpr);
}

/* Return the next filename (in a static buffer) that matches the
 * wildexpr of the previous dirfirst(), or NULL if no more files match.
 */
char *
dirnext (void)
{
  FILEFINDBUF3 fb;
  ULONG entries;
  APIRET rc;

  /* if no directory is open, then fail immediately */
  if (hdir == NULLHANDLE)
    {
      return NULL;
    }

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

static ELVBOOL
match_file (FILEFINDBUF3 *fb)
{
  if (strcmp (fb->achName, ".") == 0 || strcmp (fb->achName, "..") == 0)
    {
      return ElvFalse;
    }
  else 
    {
      return dirwildcmp (fb->achName, wildfile);
    }
}

/* return the file part of a pathname.  This particular implementation doesn't
 * use an internal buffer; it simply returns a pointer to the filename at the
 * end of the pathname.
 */
char *
dirfile (char  *pathname)
{
  char  *slash;

  /* Convert slashes to backslashes. */
  fix_slashes (pathname);

  if ((slash = strrchr (pathname, OSPATHSEP)) == NULL)
    {
      slash = strrchr (pathname, ':');
    }

  return slash != NULL ? slash + 1 : pathname;
}

#endif /* !JUST_DIRPATH */


/* Return ElvTrue if wildexpr contains any wildcards; else ElvFalse */
ELVBOOL 
diriswild (char  *wildexpr)  /* either a filename or a wildcard expression */
{
  if (strpbrk (wildexpr, "?*["))
    {
      return ElvTrue;
    }
  return ElvFalse;
}


#ifndef JUST_DIRFIRST


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
DIRPERM 
dirperm (char  *filename)  /* name of file to check */
{
  struct stat st;
  int i;

  /* check for a protocol */
  for (i = 0; elvalpha (filename[i]); i++) 
    {
    }
  if (i < 2 || filename[i] != ':')
    {
      i = 0;
    }

  /* skip past "file:" protocol; assume all others are readonly */
  if (!strncmp (filename, "file:", 5))
    {
      filename += i + 1;
    }
  else if (!strncmp (filename, "ftp:", 4))
    {
      return DIR_READWRITE;
    }
  else if (i > 0)
    {
      return DIR_READONLY;
    }


  if (stat (filename, &st) < 0)
    {
      if (errno == ENOENT)
        {
          return DIR_NEW;
        }
      else
        {
          return DIR_BADPATH;
        }
    }
  if ((st.st_mode & S_IFDIR) != 0)
    {
      return DIR_DIRECTORY;
    }
  else if ((st.st_mode & S_IFREG) == 0)
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

/* Return the timestamp of a file, or the current time if no file is specified.
 * If an invalid file is specified, return "".
 * Herbert: 2001/10/26 moved outside JUST_DIRFIRST as needed by ref.exe
 */
char *
dirtime(char  *filename)  /* filename to check */
{
  static char  str[20];/* "YYYY-MM-DDThh:mm:ss\0" */
  time_t    when;  /* the date/time */
  struct stat  st;  /* holds info from timestamp */
  struct tm  *tp;  /* time, broken down */

  /* Choose a time to return (if any) */
  if (!filename || !*filename)
    {
      time(&when);
    }
  else if (stat(filename, &st) == 0)
    {
      when = (st.st_mtime > st.st_ctime) ? st.st_mtime : st.st_ctime;
    }
  else
    {
      return "";
    }

  /* Convert it to a string */
  tp = localtime(&when);
  sprintf(str, "%04d-%02d-%02dT%02d:%02d:%02d",
          tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday,
          tp->tm_hour, tp->tm_min, tp->tm_sec);

  /* return it */
  return str;
}

#ifndef JUST_DIRPATH

/* return the pathname of the current working directory */
char *
dircwd (void)
{
  static char  cwd[CCHMAXPATH];
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

/* change the current directory, and return ElvTrue if successful (else ElvFalse) */
ELVBOOL 
dirchdir (char  *pathname)  /* new directory */
{
  if (strlen (pathname) >= 2
      && elvalpha (pathname[0])
      && pathname[1] == ':')
    {
      if (DosSetDefaultDisk (elvtolower (pathname[0]) - 'a' + 1) != NO_ERROR)
        {
          return ElvFalse;
        }
      pathname += 2;
    }

  if (pathname[0] != '\0')
    {
      return (ELVBOOL)(DosSetCurrentDir (pathname) == NO_ERROR);
    }
  else
    {
      return ElvTrue;
    }
}

/* OS-dependent initialization function.  Here we initialize the o_elvispath
 * option to include the executables directory among the possible library
 * directories.
 */
void 
osinit (char  *argv0)
{
  char  *tmp;
  static  char  path[CCHMAXPATH];

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
          tmp = iopath (tmp, "elvis.exe", ElvFalse);
          if (tmp != NULL)
            {
              argv0 = tmp;
            }
#endif
        }
    }

  /* Set the "home" option.  This is set (by order of preference) to
   * one of the following:
   *  - the value of the HOME environment variable, if set
   *  - the pathname of the elvis.exe program, if known
   *  - C:\
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

  /* Set the "elvispath" option.  We want this to include ~\dotelvis,
   * plus the "data" and "doc" subdirectories under the directory where
   * ELVIS.EXE resides.
   *
   * NOTE: The portable code in "optglob.c" will override this value
   * if the ELVISPATH environment variable is set.
   */
  tmp = dirdir (argv0);
  sprintf (path, "~\\dotelvis;%s\\data;%s\\doc", tmp, tmp);
  o_elvispath = toCHAR (path);
}

#endif /* !JUST_DIRPATH */
#endif /* !JUST_DIRFIRST */
