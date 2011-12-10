/* tcaphelp.c */

char id_tcaphelp[] = "$Id: tcaphelp.c,v 2.13 1996/09/10 14:59:03 steve Exp $";

#include "elvis.h"
#if defined(GUI_TERMCAP) || defined(GUI_OPEN)
# include <unistd.h>
# ifdef NEED_WINSIZE
#  include <sys/types.h>
#  include <sys/stream.h>
#  include <sys/ptem.h>
# endif

#ifdef linux
# include <sys/ioctl.h>
#endif
#if defined(ultrix) || defined(__osf__)
extern int ioctl P_((int d, int request, void *argp));
#endif

/* This file includes one of three three versions of low-level tty control
 * functions: one for POSIX, one for BSD, and one for SysV.  If _POSIX_SOURCE
 * is defined, then the POSIX versions are used; else if bsd is defined, then
 * the BSD versions are used; else SYSV is used.
 *
 * The version-specific functions in the included file are:
 *	ttyinit()		- remember the initial serial line configuration
 *	ttyraw()		- switch to the mode that elvis runs it
 *	ttynormal()		- switch back to the mode saved by ttyinit()
 *	ttyread(buf,len,timeout)- read characters, possibly with timeout
 *
 * The generic UNIXish functions in this file:
 *	ttywrite(buf,len)	- write characters
 *	ttytermtype()		- return the name of the terminal type
 *	ttysize()		- determine the terminal size
 */

/* This variable is used by all versions to indicate which signals have
 * been caught.
 */
static long caught;

/* include the version-specific functions */
#ifdef USE_SGTTY
# include "tcapbsd.h"
#else
# ifdef USE_TERMIO
#  include "tcapsysv.h"
# else
#  include "tcaposix.h" /* this is the preferred version */
# endif
#endif

/* write characters out to the screen */
void ttywrite(buf, len)
	char	*buf;	/* buffer, holds characters to be written */
	int	len;	/* number of characters in buf */
{
	write(1, buf, (size_t)len);
}

/* determine the terminal type */
char *ttytermtype()
{
	char	*type;

	type = getenv("TERM");
	if (!type)
	{
		type = TTY_DEFAULT;
	}
	return type;
}


/* This function gets the window size. */
BOOLEAN ttysize(linesptr, colsptr)
	int	*linesptr;	/* where to store the number of rows */
	int	*colsptr;	/* where to store the number of columns */
{
#ifdef TIOCGWINSZ
	struct winsize size;

	/* try using the TIOCGWINSZ call, if defined */
	if (ioctl(0, TIOCGWINSZ, &size) >= 0)
	{
		*linesptr = size.ws_row;
		*colsptr = size.ws_col;
		return True;
	}
#endif

	/* no special way to detect screen size */
	return False;
}

/* Check for signs of boredom from user, so we can abort a time-consuming
 * operation.  Here we check to see if SIGINT has been caught recently.
 */
BOOLEAN ttypoll(reset)
	BOOLEAN reset;
{
	return (caught & (1L << SIGINT)) ? True : False;
}

#ifdef SIGSTOP
/* Send SIGSTOP if the shell supports it, and return */
RESULT ttystop()
{
	/* the Bourne shell can't handle ^Z, but BASH can */
	if (!CHARcmp(o_shell, toCHAR("/bin/sh")) &&
		!getenv("BASH_VERSION") && !getenv("BASH"))
	{
		return RESULT_MORE;
	}
	else
	{
		/* save user buffers, if necessary */
		eventsuspend();

		/* switch the tty out of raw mode */
		ttysuspend();

		/* stop and wait for a SIGCONT */
		kill(0, SIGSTOP);

		/* switch back to raw mode */
		ttyresume(True);

		return RESULT_COMPLETE;
	}
}
#endif

#endif /* GUI_TERMCAP */
