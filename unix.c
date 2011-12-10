/* unix.c */

/* Author:
 *	Steve Kirkendall
 *	1500 SW Park #326
 *	Portland OR, 97201
 *	kirkenda@cs.pdx.edu
 */


/* This file contains the unix-specific versions the ttyread() functions.
 * There are actually three versions of ttyread() defined here, because
 * BSD, SysV, and V7 all need quite different implementations.
 */

#include "config.h"
#if ANY_UNIX || OS2
# include "vi.h"

# if BSD && !TERMIOS
/* For BSD, we use select() to wait for characters to become available,
 * and then do a read() to actually get the characters.  We also try to
 * handle SIGWINCH -- if the signal arrives during the select() call, then
 * we adjust the o_columns and o_lines variables, and fake a control-L.
 */
#  include <sys/types.h>
#  include <sys/time.h>
int ttyread(buf, len, time)
	char	*buf;	/* where to store the gotten characters */
	int	len;	/* maximum number of characters to read */
	int	time;	/* maximum time to allow for reading */
{
	fd_set	rd;	/* the file descriptors that we want to read from */
	static	tty;	/* 'y' if reading from tty, or 'n' if not a tty */
	int	i;
	struct timeval t;
	struct timeval *tp;


	/* do we know whether this is a tty or not? */
	if (!tty)
	{
		tty = (isatty(0) ? 'y' : 'n');
	}

	/* compute the timeout value */
	if (time)
	{
		t.tv_sec = time / 10;
		t.tv_usec = (time % 10) * 100000L;
		tp = &t;
	}
	else
	{
		tp = (struct timeval *)0;
	}

	/* loop until we get characters or a definite EOF */
	for (;;)
	{
		if (tty == 'y')
		{
			/* wait until timeout or characters are available */
			FD_ZERO(&rd);
			FD_SET(0, &rd);
			i = select(1, &rd, (fd_set *)0, (fd_set *)0, tp);
		}
		else
		{
			/* if reading from a file or pipe, never timeout!
			 * (This also affects the way that EOF is detected)
			 */
			i = 1;
		}
	
		/* react accordingly... */
		switch (i)
		{
		  case -1:	/* assume we got an EINTR because of SIGWINCH */
			if (*o_lines != LINES || *o_columns != COLS)
			{
#ifndef CRUNCH
				*o_nearscroll = 
#endif
				*o_lines = LINES;
				*o_columns = COLS;
				if (mode != MODE_EX)
				{
					/* pretend the user hit ^L */
					*buf = ctrl('L');
					return 1;
				}
			}
			break;
	
		  case 0:	/* timeout */
			return 0;
	
		  default:	/* characters available */
			return read(0, buf, len);
		}
	}
}
# else

# if UNIXV || COH_386 || OS2 || TERMIOS
/* For System-V, we use VMIN/VTIME to implement the timeout.  For no timeout,
 * VMIN should be 1 and VTIME should be 0; for timeout, VMIN should be 0 and
 * VTIME should be the timeout value.
 */
#  if TERMIOS
#   include <termios.h>
#  else
#   include <termio.h>
#  endif
int ttyread(buf, len, time)
	char	*buf;	/* where to store the gotten characters */
	int	len;	/* maximum number of characters to read */
	int	time;	/* maximum time to allow for reading */
{
#   if TERMIOS
	struct termios tio;
#   else
	struct termio tio;
#   endif
	int	bytes;	/* number of bytes actually read */

	/* arrange for timeout */
#   if TERMIOS
	tcgetattr(0, &tio);
#   else
	ioctl(0, TCGETA, &tio);
#   endif
	if (time)
	{
		tio.c_cc[VMIN] = 0;
		tio.c_cc[VTIME] = time;
	}
	else
	{
		tio.c_cc[VMIN] = 1;
		tio.c_cc[VTIME] = 0;
	}
#   if TERMIOS
	tcsetattr(0, TCSANOW, &tio);
#   else
	ioctl(0, TCSETA, &tio);
#   endif

	/* Perform the read.  Loop if EINTR error happens */
	while ((bytes = read(0, buf, (unsigned)len)) < 0)
	{
		/* probably EINTR error because a SIGWINCH was received */
		if (*o_lines != LINES || *o_columns != COLS)
		{
			*o_lines = LINES;
			*o_columns = COLS;
			if (mode != MODE_EX)
			{
				/* pretend the user hit ^L */
				*buf = ctrl('L');
				return 1;
			}
		}
	}

#if OS2
	{
		int cnt;
		for (cnt = 0; cnt < bytes; cnt++)
		{
			/* extended key, scan code follows */
			if (buf[cnt] == 0)
			{
				buf[cnt] = '#';
			}
		}
	}
#endif

	/* return the number of bytes read */
	return bytes;

	/* NOTE: The terminal may be left in a timeout-mode after this function
	 * returns.  This shouldn't be a problem since Elvis *NEVER* tries to
	 * read from the keyboard except through this function.
	 */
}

# else /* any other version of UNIX, assume it is V7 compatible */

/* For V7 UNIX (including Minix) we set an alarm() before doing a blocking
 * read(), and assume that the SIGALRM signal will cause the read() function
 * to give up.
 */

#include <setjmp.h>

static jmp_buf env;

/*ARGSUSED*/
SIGTYPE dummy(signo)
	int	signo;
{
	longjmp(env, 1);
}
int ttyread(buf, len, time)
	char	*buf;	/* where to store the gotten characters */
	int	len;	/* maximum number of characters to read */
	int	time;	/* maximum time to allow for reading */
{
	/* arrange for timeout */
	signal(SIGALRM, dummy);
	alarm(time);

	/* perform the blocking read */
	if (setjmp(env) == 0)
	{
		len = read(0, buf, (unsigned)len);
	}
	else /* I guess we timed out */
	{
		len = 0;
	}

	/* cancel the alarm */
	signal(SIGALRM, dummy); /* <-- to work around a bug in Minix */
	alarm(0);

	/* return the number of bytes read */
	if (len < 0)
		len = 0;
	return len;
}

# endif /* M_SYSV */
# endif /* !BSD */

#endif /* ANY_UNIX */
