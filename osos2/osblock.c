/* osos2/osblock.c */

/*
 * Session file handling routines for OS/2.  We use the OS/2 Control
 * Program API (DosOpen(), DosWrite(), etc.) because the standard
 * open(), read(), and write() calls don't afford enough control over
 * file sharing and other attributes.  Some things are actually easier
 * this way, anyhow.  :-)
 *
 * $Log: osblock.c,v $
 * Revision 1.6  2003/10/23 23:35:45  steve
 * Herbert's latest changes.
 *
 * Revision 1.5  2003/10/17 17:41:23  steve
 * Renamed the BOOLEAN data type to ELVBOOL to avoid name clashes with
 *   types defined other headers.
 *
 * Revision 1.4  2002/07/30 17:02:46  steve
 * OS/2 changes from Herbert.
 *
 * Revision 1.3  2002/07/09 18:19:22  steve
 * Protect against trying a ridiculous number of possible session file names.
 *
 * Revision 1.2  2001/04/20 00:00:37  steve
 * Some bug fixes, and uglification of the source code.
 *
 * Revision 1.2  2000/06/04 10:26:53  HERBERT
 * Some formatting and CVS Logging.
 *
 *
 */
#include <process.h>
#include "elvis.h"

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#ifdef __EMX__
#define CHAR OS2CHAR
#endif
#include <os2.h>
#ifdef CHAR
# undef CHAR
#endif

#ifndef DEFAULT_SESSION
# define DEFAULT_SESSION "%sELV%05d.SES"
#endif


static HFILE fd = NULLHANDLE;  /* file handle of the session file */

static char sessionDir[128];
static char *sessionDirPtr = NULL;
#ifdef FEATURE_RAM
static BLK **blklist;
static int nblks;
#endif

/* This function creates a new block file, and returns ElvTrue if successful,
 * or ElvFalse if failed because the file was already busy.
 */
ELVBOOL 
blkopen (ELVBOOL force,    /* if ElvTrue, open even if "in use" flag set */
         BLK *buf)         /* buffer, holds SUPER block */
{
  static char  dfltname[256];
  int  i;
  APIRET rc;
  ULONG open_flags;
  ULONG open_mode;
  ULONG action;
  ULONG actual;

#ifdef FEATURE_RAM
  if (o_session && !CHARcmp(o_session, toCHAR("ram")))
  {
    nblks = 1024;
    blklist = (BLK **)calloc(nblks, sizeof(BLK *));
    blklist[0] = (BLK *)malloc(o_blksize);
    memcpy(blklist[0], buf, o_blksize);
    return ElvTrue;
  }
#endif

    /* This is a little bit dirty :-)  We try to find a directory for
     * our session files from the list o_sessionpath.  It will be the
     * first writable directory from the list we can find.  The dirty
     * part is we don't want to do this more than once.
     */
    if (sessionDirPtr == NULL) {
        char pathlist[128];
        char *ptr = pathlist;
        unsigned last;
        FILESTATUS fileInfo = {{0}};
        sprintf (sessionDir, "%c", OSPATHDELIM);

    /* search through sessionpath for a writable directory */
    if (!o_sessionpath)
      o_sessionpath = toCHAR(".");
        /* endif */

        /* Go through the directory list in o_sessionpath and use
         * the first writable one for the session file. */
        pathlist[sizeof pathlist - 1u] = '\0';
        strncpy (pathlist, o_sessionpath, sizeof pathlist - 1u);
        if ((ptr = strtok (pathlist, sessionDir)) != NULL) {
            do {
                last = strlen (ptr);
                if (ptr[last-1] == OSPATHSEP)
                    ptr[last-1] = '\0';
                /* endif */
                if (DosQueryPathInfo ((PSZ)ptr, FIL_STANDARD, &fileInfo, 
                        (ULONG)sizeof fileInfo) == NO_ERROR 
                        && (fileInfo.attrFile & 0x01) == 0
                        && (fileInfo.attrFile & 0x10) != 0)
                    break;
                /* endif */
            } while ((ptr = strtok (NULL, sessionDir)) != NULL);
        }/* if */

        /* Found a directory for the session file?  If not, just use
         * the current one else use the one found. */
        if (ptr == NULL)
            sprintf (sessionDir, "%c", '.');
        else {
            strncpy (sessionDir, ptr, sizeof sessionDir - 1u);
            sessionDir[sizeof sessionDir - 1u] = '\0';
        } /* if */

        /* Append a path delimiter and terminate string.  Be sure
         * not to write outside the buffer boundaries... */
        if ((last = strlen (sessionDir)) < sizeof sessionDir - 1u){
            sessionDir[last] = OSPATHSEP;
            sessionDir[last+1] = '\0';
        }/* if */
    } /* if */

  /* If elvis runs other programs, prevent them from inheriting
   * the session file's descriptor.  Also prevent write access
   * to the session file by other processes while this process
   * is using it.
   */
  open_mode = OPEN_FLAGS_NOINHERIT 
        | OPEN_SHARE_DENYWRITE
        | OPEN_ACCESS_READWRITE;

  /* If no session file was explicitly requested, try successive
   * defaults until we find an existing file (if we're trying to
   * recover) or a non-existent file (if we're not trying to recover).
   */
  if (!o_session) {
    i = 1;

    o_session = toCHAR (dfltname);
    o_tempsession = ElvTrue;
    do
    {
      /* protect against trying a ridiculous number of files */
      if (i >= 1000) {
	msg(MSG_FATAL, o_recovering
			? "[s]no session file found in $1"
			: "[s]too many session files in $1", sessionDir);
      }
      sprintf (dfltname, DEFAULT_SESSION, sessionDir, i++);
      open_flags = o_recovering
        ? OPEN_ACTION_FAIL_IF_NEW   | OPEN_ACTION_OPEN_IF_EXISTS
        : OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_FAIL_IF_EXISTS;

      rc = DosOpen (tochar8 (o_session),
             &fd, 
             &action,
             0L,
             FILE_NORMAL,
             open_flags,
             open_mode,
             NULL);

    } while (rc == ERROR_OPEN_FAILED);
  } else {
    /* Try to open the session file */
    open_flags = 
      OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW;

    rc = DosOpen (tochar8(o_session),
           &fd, 
           &action,
           0L,
           FILE_NORMAL,
           open_flags,
           open_mode,
           NULL);
  } /* if */

  /* Error checking. */
  switch (rc)
  {
  case NO_ERROR:
    break;

  case ERROR_SHARING_VIOLATION:
    msg(MSG_FATAL, "session file busy");

  default:
    msg(MSG_FATAL, "no such session");
  } /* if */

  if (action == FILE_EXISTED) {
    /* we're opening an existing session -- definitely not temporary */
    o_tempsession = ElvFalse;
  } else {
    o_newsession = ElvTrue;
    rc = DosWrite (fd, buf, BLKSIZE, &actual);
    if (rc != NO_ERROR || actual < BLKSIZE) {
      DosClose (fd);
      DosDelete (tochar8(o_session));
      fd = NULLHANDLE;
      msg (MSG_FATAL, "no such session");
    } else {
      (void)DosSetFilePtr (fd, 0L, FILE_BEGIN, &actual);
    } /* if */
  } /* if */

  /* Read the first block & mark the session file as being "in use".
   * If already marked as "in use" and !force, then fail.
   */
  rc = DosRead (fd, buf, sizeof buf->super, &actual);
  if (rc != NO_ERROR || actual != sizeof buf->super) {
    msg (MSG_FATAL, "blkopen's read failed");
  } /* if */
  if (buf->super.inuse && !force) {
    return ElvFalse;
  } /* if */
  buf->super.inuse = getpid();

  (void)DosSetFilePtr (fd, 0L, FILE_BEGIN, &actual);
  (void)DosWrite (fd, buf, sizeof buf->super, &actual);

  /* done! */
  return ElvTrue;
}

/* This function closes the session file, given its handle */
void 
blkclose (BLK  *buf)  /* buffer, holds superblock */
{
  blkread (buf, 0);
  buf->super.inuse = 0L;
  blkwrite (buf, 0);
  (void)DosClose (fd);
  fd = NULLHANDLE;
  if (o_tempsession) {
    (void)DosDelete (tochar8(o_session));
  } /* if */
}

/* Write the contents of buf into record # blkno, for the block file
 * identified by blkhandle.  Blocks are numbered starting at 0.  The
 * requested block may be past the end of the file, in which case
 * this function is expected to extend the file.
 */
void 
blkwrite (BLK    *buf,    /* buffer, holds contents of block */
          _BLKNO_  blkno) /* where to write it */
{
  LONG offset;
  ULONG actual;

#ifdef FEATURE_RAM
  /* store it in RAM */
  if (nblks > 0)
  {
    if (blkno >= nblks)
    {
      blklist = (BLK **)realloc(blklist,
            (nblks + 1024) * sizeof(BLK *));
      memset(&blklist[nblks], 0, 1024 * sizeof(BLK *));
      nblks += 1024;
    }
    if (!blklist[blkno])
      blklist[blkno] = malloc(o_blksize);
    memcpy(blklist[blkno], buf, o_blksize);
    return;
  }
#endif

  /* write the block */
  offset = (LONG)blkno * (LONG)o_blksize;
  if (DosSetFilePtr (fd, offset, FILE_BEGIN, &actual) != NO_ERROR
    || DosWrite (fd, buf, o_blksize, &actual) != NO_ERROR
    || actual != o_blksize)
  {
    msg (MSG_FATAL, "blkwrite(%d) failed", blkno);
  } /* if */
}

/* Read the contends of record # blkno into buf, for the block file
 * identified by blkhandle.  The request block will always exist;
 * it will never be beyond the end of the file.
 */
void 
blkread (BLK *buf,    /* buffer, where buffer should be read into */
         _BLKNO_  blkno)  /* where to read from */
{
  LONG offset;
  ULONG actual;

#ifdef FEATURE_RAM
  if (nblks > 0)
  {
    memcpy(buf, blklist[blkno], o_blksize);
    return;
  }
#endif

  /* read the block */
  offset = (LONG)blkno * (LONG)o_blksize;
  if (DosSetFilePtr (fd, offset, FILE_BEGIN, &actual) != NO_ERROR
    || DosRead (fd, buf, o_blksize, &actual) != NO_ERROR
    || actual != o_blksize)
  {
    msg (MSG_FATAL, "blkread failed");
  } /* if */
}

/* Force changes out to disk. */
void 
blksync P_((void))
{
#ifdef FEATURE_RAM
  if (nblks > 0)
    return;
#endif

  (void)DosResetBuffer (fd);
}
