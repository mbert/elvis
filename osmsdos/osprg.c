/* osmsdos/osprg.c */

#include "elvis.h"
#include <sys/types.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>

/* Use "#if 1" for a plain old system() call, or "#if 0" to use the fancy
 * version which swaps elvis out to EMS/XMS memory or disk while the child
 * process runs.
 */
#if 0
# define swapsystem system
#else
# include "doexec.h"

static int swapsystem(char *cmd)
{
	char	*prog, *args;
	int	retcode;

	for (; elvspace(*cmd); cmd++)
	{
	}
	prog = safedup(cmd);
	for (args = prog; *args && !elvspace(*args); args++)
	{
	}
	if (*args)
		*args++ = '\0';
	retcode = do_exec(prog, args, USE_ALL|HIDE_FILE, 0xFFFF, NULL);
	if (retcode == RC_NOFILE)
	{
		safefree(prog);
		prog = safealloc(strlen(cmd) + 4, sizeof(char));
		strcpy(prog, "/c ");
		strcat(prog, cmd);
		retcode = do_exec(tochar8(o_shell), prog,
					USE_ALL|HIDE_FILE, 0xFFFF, NULL);
	}
	safefree(prog);
	return retcode;
}
#endif


/* Microsoft has an annoying habit of adding underscores to the front of
 * conventional names.
 */
#ifndef O_WRONLY
# define O_WRONLY	_O_WRONLY
# define O_RDONLY	_O_RDONLY
# define O_RDWR		_O_RDWR
# define O_CREAT	_O_CREAT
# define O_EXCL		_O_EXCL
# define O_TRUNC	_O_TRUNC
#endif

static char	*cmd;		/* the command to run */
static char	tempread[100];	/* name of temp file for prog's stdout */
static char	tempwrite[100];	/* name of temp file for prog's stdout */
static int	fd;		/* file descriptor for reading or writing */
static int	status;		/* exit status of the program */

/* Declares which program we'll run, and what we'll be doing with it.
 * This function should return ElvTrue if successful.  If there is an error,
 * it should issue an error message via msg(), and return ElvFalse.
 */
ELVBOOL prgopen(char *command, ELVBOOL willwrite, ELVBOOL willread)
{
	/* remember the command we're supposed to run */
	cmd = command;

	/* if we're going to be reading, then choose a tempread name now */
	if (willread)
	{
		strcpy(tempread, o_directory ? dirpath(tochar8(o_directory), "rdXXXXXX") : "rdXXXXXX");
		mktemp(tempread);

		/* open it and close it, just to make sure it exists */
		fd = open(tempread, O_WRONLY|O_CREAT|O_EXCL, 0600);
		if (fd < 0)
		{
			msg(MSG_ERROR, "[s]can't create temp file $1", tempread);
			return ElvFalse;
		}
		close(fd);
	}
	else
		tempread[0] = '\0';

	/* if we'll be writing, then choose a tempwrite name and open it */
	if (willwrite)
	{
		strcpy(tempwrite, o_directory ? dirpath(tochar8(o_directory), "wrXXXXXX") : "wrXXXXXX");
		mktemp(tempwrite);
		fd = open(tempwrite, O_WRONLY|O_CREAT|O_EXCL, 0600);
		if (fd < 0)
		{
			msg(MSG_ERROR, "[s]can't create temp file $1", tempwrite);
			if (tempread[0])
				remove(tempread);
			return ElvFalse;
		}
	}
	else
		tempwrite[0] = '\0';

	return ElvTrue;
}

/* Write the contents of buf to the program's stdin, and return nbytes
 * if successful, or -1 for error.  Note that this text should
 * be subjected to the same kind of transformations as textwrite().
 * In fact, it may use textwrite() internally.
 *
 * For DOS, this is simply a write() to the temp file.
 */
int prgwrite(CHAR *buf, int nbytes)
{
	return write(fd, buf, nbytes);
}

/* Marks the end of writing.  Returns ElvTrue if all is okay, or ElvFalse if
 * error.
 */
ELVBOOL prggo(void)
{
	int	old0;	/* elvis' stdin */
	int	old1;	/* elvis' stdout */
	int	old2;	/* elvis' stderr */
	CHAR	*arg[3];/* arguments when evaluating command string */

	/* are we supposed to redirect stdin? */
	if (tempwrite[0])
	{
		/* close the file we've been writing to */
		close(fd);

		/* save the old stdin as another fd so we can switch back later */
		old0 = dup(0);
		assert(old0 > 2);

		/* open the temp file as stdin */
		close(0);
#ifdef NDEBUG
		(void)open(tempwrite, O_RDONLY);
#else
		assert(open(tempwrite, O_RDONLY) == 0);
#endif
	}

	/* are we supposed to redirect stdout/stderr ? */
	if (tempread[0])
	{
		/* save the old stdout and stderr as other fds */
		old1 = dup(1);
		old2 = dup(2);
		assert(old1 > 2 && old2 > 2);

		/* open the temp file as stdout/stderr */
		close(1);
		close(2);
#ifdef NDEBUG
		(void)open(tempread, O_WRONLY);
		dup(1);
#else
		assert(open(tempread, O_WRONLY) == 1);
		assert(dup(1) == 2);
#endif
	}

#if 0
	/* if redirecting anything, then evaluate the command string like a
	 * message, substituting the input and/or output files for $1 and $2
	 */
	if ((tempwrite[0] || tempread[0]) && CHARchr(cmd, '$'))
	{
		arg[0] = toCHAR(tempwrite);
		arg[1] = toCHAR(tempread);
		arg[2] = NULL;
		cmd = tochar8(calculate(toCHAR(cmd), arg, CALC_MSG));
		if (!cmd)
		{
			if (tempwrite[0])
				remove(tempwrite);
			if (tempread[0])
				remove(tempread);
			return ElvFalse;
		}
	}
#endif

	/* run the program */
	status = swapsystem(cmd);

	/* if we redirected stdin, undo it now */
	if (tempwrite[0])
	{
		/* undo the redirection */
		close(0);
#ifdef NDEBUG
		(void)dup(old0);
#else
		assert(dup(old0) == 0);
#endif
		close(old0);

		/* delete the temp file */
		remove(tempwrite);
	}

	/* if we redirected stdout/stderr, undo it now and open the temp file */
	if (tempread[0])
	{
		/* undo the redirection */
		close(1);
		close(2);
#ifdef NDEBUG
		(void)dup(old1);
		(void)dup(old2);
#else
		assert(dup(old1) == 1);
		assert(dup(old2) == 2);
#endif
		close(old1);
		close(old2);

		/* open the temp file */
		fd = open(tempread, O_RDONLY);
		assert(fd > 0);
	}

	return ElvTrue;
}


/* Reads text from the program's stdout, and returns the number of
 * characters read.  At EOF, it returns 0.  Note that this text
 * should be subjected to the same kinds of transformations as
 * textread().
 */
int prgread(CHAR *buf, int nbytes)
{
	return read(fd, buf, nbytes);
}

/* Clean up, and return the program's exit status.  The exit status
 * should be 0 normally.
 */
int prgclose(void)
{
	/* if we were reading stdout, then close & delete that temp file */
	if (tempread[0])
	{
		close(fd);
		remove(tempread);
	}

	/* return the program's exit status */
	return status;
}
