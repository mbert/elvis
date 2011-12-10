/* osos2/osprg.c */

/*
 * Stuff for starting external programs like compilers or shell commands. 
 * 
 * Ported by Lee Johnson, small changes (e.g. emx/gcc compatibility) by 
 * Martin "Herbert" Dietze.
 *
 * $Log: osprg.c,v $
 * Revision 1.6  2003/10/23 23:35:45  steve
 * Herbert's latest changes.
 *
 * Revision 1.5  2003/10/17 17:41:23  steve
 * Renamed the BOOLEAN data type to ELVBOOL to avoid name clashes with
 *   types defined other headers.
 *
 * Revision 1.4  2001/10/23 01:37:09  steve
 * Sometweaks of FEATURE_XXXX names
 *
 * Revision 1.3  2001/10/22 18:23:14  steve
 * Added FEATURE_RCSID compile-time option
 *
 * Revision 1.2  2001/04/20 00:00:37  steve
 * Some bug fixes, and uglification of the source code.
 *
 * Revision 1.2  2000/06/04 10:26:54  HERBERT
 * Some formatting and CVS Logging.
 *
 *
 */

#include <errno.h>
#ifdef __EMX__
# include <sys/types.h>
#endif
#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <string.h>
#include <stdlib.h>
#include "elvis.h"
#ifdef FEATURE_RCSID
char id_osprg[] = "$Id: osprg.c,v 1.6 2003/10/23 23:35:45 steve Exp $";
#endif

#define INCL_DOSQUEUES
#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#ifdef __EMX__
#define CHAR OS2CHAR
#endif
#include <os2.h>
#ifdef CHAR
# undef CHAR
#endif


#define TMPDIR  (o_directory ? tochar8(o_directory) : "\\TMP")

static char  *command;    /* the command to run */
static char  tempfname[CCHMAXPATH];  /* name of temp file */
static int  writefd = -1;  /* fd used for writing to program's stdin */
static int  readfd = -1;  /* fd used for reading program's stdout */
static int  pid = -1;    /* process ID of program */


/* Define a convenient grab bag to save standard I/O handles in. */
typedef HFILE SAVE_IO[3];

/* Selection flags for SaveStdIOHandles(): */
#define SAVE_STDIN    0x01
#define SAVE_STDOUT    0x02
#define SAVE_STDERR    0x04


/*
 * Restore I/O handles saved by SaveStdIOHandles().
 */
static void
RestoreStdIOHandles (SAVE_IO save)
{
  HFILE fd;
  int i;

  /*
   * Restore any saved I/O handles in 'save'.  If the filesystem
   * calls fail, there's no easy way to recover.
   */
  for (i = 0; i < 3; i++)
    {
      if (save[i] != ~0)
        {
          fd = i;
          DosDupHandle (save[i], &fd);
          DosClose (save[i]);
          save[i] = ~0;
        }
    }
}


/*
 * Save standard I/O handles prior to redirection processing.  'select'
 * is a flag mask that specifies which of the standard handles are
 * marked for safekeeping.
 */
static ELVBOOL
SaveStdIOHandles (SAVE_IO save, int select)
{
  /* Establish default values for error recovery. */
  save[0] = save[1] = save[2] = ~0;

    /*
     * Make copies of the selected standard I/O handles and mark the
     * copies so that child processes will not inherit them.
     */
  if (select & SAVE_STDIN)
    {
      if (DosDupHandle (0, &save[0]) != NO_ERROR)
        {
          goto Error;
        }
      if (DosSetFHState (save[0], OPEN_FLAGS_NOINHERIT) != NO_ERROR)
        {
          goto Error;
        }
    }

  if (select & SAVE_STDOUT)
    {
      if (DosDupHandle (1, &save[1]) != NO_ERROR)
        {
          goto Error;
        }
      if (DosSetFHState (save[1], OPEN_FLAGS_NOINHERIT) != NO_ERROR)
        {
          goto Error;
        }
    }

  if (select & SAVE_STDERR)
    {
      if (DosDupHandle (2, &save[2]) != NO_ERROR)
        {
          goto Error;
        }
      if (DosSetFHState (save[2], OPEN_FLAGS_NOINHERIT) != NO_ERROR)
        {
          goto Error;
        }
    }

  /* Success. */
  return ElvTrue;

Error:
  /* Restore any handles we have managed to save. */
  RestoreStdIOHandles (save);
  msg (MSG_ERROR, "can't save stdio handle");
  return ElvFalse;
}


static int
RunCommand (char *command)
{
  char *buffer;
  char *shell;
  int pid;

  /* Locate the command shell. */
  shell = o_shell ? tochar8 (o_shell) : getenv ("COMSPEC");
  if (shell == NULL)
    {
      shell = "cmd.exe";
    }

  if ((buffer = malloc (strlen (command) + 5)) != NULL) 
    {
    /*
     * Herbert:
     * Find the name of the shell. I assume cmd.exe and 4os2.exe to
     * understand the "/c" switch and use "-c" for all the others.
     * The quotes around the command *seem* to work for Unix-like
     * shells, they are meant to pass the command to the shell as
     * one string.
     */
      char *org_sep = strrchr (shell, OSPATHSEP);
      char *alt_sep = strrchr (shell, OSALTPSEP);
      char *base = org_sep < alt_sep ? alt_sep : org_sep;
      base = base == NULL? shell: base+1;

      if ( stricmp (base, "cmd.exe") == 0 
           || stricmp (base, "4os2.exe") == 0) 
        {
          sprintf (buffer, "/c %s", command);
        }
      else 
        {
          sprintf (buffer, "-c \"%s\"", command);
        }

      pid = spawnlp (P_NOWAIT, shell, shell, buffer, NULL);
      free (buffer);
      return pid;
    } 
  else 
    {
      return -1;
    }
}


/*
 * Declares which program we'll run, and what we'll be doing with it.
 * This function should return ElvTrue if successful.  If there is an error,
 * it should issue an error message via msg(), and return ElvFalse.
 *
 * For UNIX, the behavior of this function depends on willwrite.
 * If willwrite, then the command is saved and a temporary file is
 * is created to store the data that will become the program's stdin,
 * and the function succeeds if the temp file was created successfully.
 * Else the program is forked (with stdout/stderr redirected to a pipe
 * if willread) and the function succedes if pipe() and fork()
 * succeed.
 */
ELVBOOL 
prgopen (char  *cmd,    /* command string */
         ELVBOOL  willwrite,  /* if ElvTrue, redirect command's stdin */
         ELVBOOL  willread)  /* if ElvTrue, redirect command's stdout */
{
  HFILE r_pipe, w_pipe;  /* two ends of a pipe */
  SAVE_IO save_io;
  HFILE fd;
  APIRET rc;

  /* Mark both fd's as being unused */
  writefd = readfd = -1;

  /* Next step depends on what I/O we expect to do with this program */
  if (willwrite && willread)
    {
      /* save the command */
      command = strdup (cmd);

      /* create a temporary file for feeding the program's stdin */
      sprintf (tempfname, "%s/elvis%d.tmp", TMPDIR, (int)getpid ());
      writefd = open (tempfname, O_WRONLY|O_CREAT|O_EXCL, S_IREAD|S_IWRITE);
      if (writefd < 0)
        {
          msg (MSG_ERROR, "can't make temporary file");
          free (command);
          return ElvFalse;
        }
    }
  else if (willwrite || willread) /* but not both */
    {
      /* Create a pipe. */
      rc = DosCreatePipe (&r_pipe, &w_pipe, 4096);
      if (rc != NO_ERROR)
        {
          msg(MSG_ERROR, "can't create pipe");
          goto Error;
        }

      /*
       * Redirect standard file handles for the CHILD PROCESS.
       */

      if (willwrite)
        {
          /* Save the standard input handle. */
          if (!SaveStdIOHandles (save_io, SAVE_STDIN))
            goto Error;

          /* Get standard input from the read end of the pipe. */
          fd = 0;
          if (DosDupHandle (r_pipe, &fd) != NO_ERROR)
            goto DupError;

          /*
           * Prevent the child process from inheriting the write end
           * of the pipe.  This will ensure that the pipe closes
           * cleanly when the parent process (elvis) is done with it.
           */
          if (DosSetFHState (w_pipe, OPEN_FLAGS_NOINHERIT) != NO_ERROR)
            {
              msg (MSG_ERROR, "can't set file inheritance");
              goto Error;
            }
        }
      else
        {
          /* Save the standard output and error handles. */
          if (!SaveStdIOHandles (save_io, SAVE_STDOUT | SAVE_STDERR))
            {
              goto Error;
            }

          /* Send standard output to the write end of the pipe. */
          fd = 1;
          if (DosDupHandle (w_pipe, &fd) != NO_ERROR)
            {
              goto DupError;
            }

          /* Send error output to the write end of the pipe. */
          fd = 2;
          if (DosDupHandle (w_pipe, &fd) != NO_ERROR)
            {
              goto DupError;
            }
        }

      /* Launch the command. */
      if ((pid = RunCommand (cmd)) < 0)
        {
          msg (MSG_ERROR, "can't spawn");
          goto Error;
        }

      if (willwrite)
        {
          /*
           * Close the read end of the pipe and remember the fd 
           * of the write end.
           */
          DosClose (r_pipe);
          r_pipe = NULLHANDLE;
          writefd = w_pipe;
        }
      else
        {
          /*
           * Close the write end of the pipe and remember the fd 
           * of the read end.
           */
          DosClose (w_pipe);
          w_pipe = NULLHANDLE;
          readfd = r_pipe;
        }

      /* Restore standard file handles for the PARENT PROCESS. */
      RestoreStdIOHandles (save_io);
    }
  else /* no redirection */
    {
      /* Launch the command. */
      if ((pid = RunCommand (cmd)) < 0)
        {
          msg(MSG_ERROR, "can't spawn");
          goto Error;
        }
    }

  /* if we get here, we must have succeeded */
  return ElvTrue;

DupError:
  msg(MSG_ERROR, "can't dup file handle");

Error:
  /* Restore standard I/O handles if necessary. */
  RestoreStdIOHandles (save_io);
  return ElvFalse;
}


/*
 * Write the contents of buf to the program's stdin, and return nbytes
 * if successful, or -1 for error.  Note that this text should
 * be subjected to the same kind of transformations as textwrite().
 * In fact, it may use textwrite() internally.
 *
 * For UNIX, this is simply a write() to the temp file or pipe.
 */
int 
prgwrite (CHAR  *buf,  /* buffer, contains text to be written */
          int  nbytes)  /* number of characters in buf */
{
  assert (writefd >= 0);
  return write (writefd, buf, (size_t)nbytes);
}


/*
 * Marks the end of writing.  Returns ElvTrue if all is okay, or ElvFalse if
 * error.
 *
 * For UNIX, the temp file is closed, and the program is forked.
 * (Since this function is only called when willwrite, the program
 * wasn't forked when prgopen() was called.)  Returns ElvTrue if the
 * fork was successful, or ElvFalse if it failed.
 */
ELVBOOL 
prggo (void)
{
  HFILE r_pipe, w_pipe;  /* two ends of a pipe */
  SAVE_IO save_io;
  HFILE fd, tmp_fd;
  APIRET rc;

  /* If we weren't writing, then there's nothing to be done here */
  if (writefd < 0)
    {
      return ElvTrue;
    }

  /*
   * If we're using a temp file, close it for writing, then spawn
   * the program with its stdin redirected to come from the file.
   */
  if (command)
    {
      /* Close the temp file for writing. */
      close (writefd);
      writefd = -1;

      /* Make a pipe to use for reading stdout/stderr. */
      rc = DosCreatePipe (&r_pipe, &w_pipe, 4096);
      if (rc != NO_ERROR)
        {
          msg (MSG_ERROR, "can't create pipe");
          goto Error;
        }

      /* Save all of the standard I/O handles. */
      if (!SaveStdIOHandles (save_io, 
                             SAVE_STDIN | SAVE_STDOUT | SAVE_STDERR))
        {
          goto Error;
        }

      /*
       * Redirect standard file handles for the CHILD PROCESS.
       */

      /* Get stdin from the temporary file. */
      tmp_fd = open (tempfname, O_RDONLY);
      fd = 0;
      if (DosDupHandle (tmp_fd, &fd) != NO_ERROR)
        {
          goto DupError;
        }
      close(tmp_fd);

      /* Connect the write end of the pipe to stdout/stderr. */
      fd = 1;
      if (DosDupHandle (w_pipe, &fd) != NO_ERROR)
        {
          goto DupError;
        }
      fd = 2;
      if (DosDupHandle (w_pipe, &fd) != NO_ERROR)
        {
          goto DupError;
        }

      /*
       * Prevent the child process from inheriting the read end
       * of the pipe.  This will ensure that the pipe closes
       * cleanly when the parent process (elvis) is done with it.
       */
      if (DosSetFHState (r_pipe, OPEN_FLAGS_NOINHERIT) != NO_ERROR)
        {
          msg (MSG_ERROR, "can't set file inheritance");
          goto Error;
        }

      /* Launch the command. */
      if ((pid = RunCommand (command)) < 0)
        {
          msg (MSG_ERROR, "can't spawn");
          goto Error;
        }

      /* Restore standard file handles for the PARENT PROCESS. */
      RestoreStdIOHandles (save_io);

      /*
       * Close the write end of the pipe; the read end becomes 
       * 'readfd'.
       */
      DosClose (w_pipe);
      w_pipe = NULLHANDLE;
      readfd = r_pipe;

      /* We don't need the command string any more. */
      free (command);
    }
  else /* writing but not reading */
    {
      /* close the writefd */
      close (writefd);
      writefd = -1;
    }

  return ElvTrue;

DupError:
  msg (MSG_ERROR, "can't dup file handle");

Error:
  /* Restore standard I/O handles if necessary. */
  RestoreStdIOHandles (save_io);
  return ElvFalse;
}


/*
 * Reads text from the program's stdout, and returns the number of
 * characters read.  At EOF, it returns 0.  Note that this text
 * should be subjected to the same kinds of transformations as
 * textread().
 *
 * For UNIX, this is simply a read() from the pipe.
 */
int 
prgread (CHAR  *buf,  /* buffer where text should be placed */
         int  nbytes)  /* maximum number of characters to read */
{
  assert (readfd >= 0);
  return read (readfd, buf, (size_t)nbytes);
}


/*
 * Clean up, and return the program's exit status.  The exit status
 * should be 0 normally.
 *
 * For UNIX, this involves closing the pipe, calling wait() to get the
 * program's exit status, and then deleting the temp file.
 */
int 
prgclose (void)
{
  int  status;

  /* close the readfd, if necessary */
  if (readfd >= 0)
    {
      close (readfd);
      readfd = -1;
    }

  /* close the writefd, if necessary */
  if (writefd >= 0)
    {
      close (writefd);
      writefd = -1;
    }

  /* wait for the program to die */
  if (pid >= 0)
    {
      while (wait (&status) != pid)
        {
        }
    }

  /* delete the temp file, if there was one */
  if (*tempfname)
    {
      unlink (tempfname);
      *tempfname = '\0';
    }

  return status;
}

