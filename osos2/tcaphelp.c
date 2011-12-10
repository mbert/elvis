/* osos2/tcaphelp.c */

/* This file contains code from the osunix/tcaphelp.c and osunix/tcaposix.h
 * files necessary if Termcap support (emx ony) is desired.  Else nothing
 * usable is implemented yet.  A minimal Termcap support could be of interest
 * for the Open interface that is currently only available in the emx version.
 */

#include "elvis.h"
#if defined(GUI_TERMCAP)  || defined(GUI_OPEN) 

# ifdef __EMX__

#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

/* This variable is used by all versions to indicate which signals have
 * been caught.
 */
static long caught;


static struct termios	oldtermio;	/* original tty mode */
static struct termios	newtermio;	/* cbreak/noecho tty mode */


/* this function is used to catch signals */
static void catchsig(signo)
	int	signo;
{
	caught = (1 << signo);
}

/* get the original tty state */
void ttyinit()
{
	/* get the old tty state */
	tcgetattr(0, &oldtermio);
}

/* switch to the tty state that elvis runs in */
void ttyraw(erasekey)
	char	*erasekey;	/* where to store the ERASE char */
{
	signal(SIGINT, catchsig);
	signal(SIGQUIT, SIG_IGN);

	/* switch to raw mode */
	ospeed = cfgetospeed(&oldtermio);
	*erasekey = oldtermio.c_cc[VERASE];
	newtermio = oldtermio;
	newtermio.c_iflag &= (IXON|IXOFF|ISTRIP|IGNBRK);
	newtermio.c_oflag &= ~OPOST;
	newtermio.c_lflag &= ISIG;
	newtermio.c_cc[VINTR] = ELVCTRL('C'); /* always use ^C for interrupts */
#ifdef NDEBUG
	newtermio.c_cc[VQUIT] = 0;
#endif
	newtermio.c_cc[VMIN] = 1;
	newtermio.c_cc[VTIME] = 0;
	newtermio.c_cc[VSUSP] = 0;
	tcsetattr(0, TCSADRAIN, &newtermio);
}

/* switch back to the original tty state */
void ttynormal()
{
	tcsetattr(0, TCSADRAIN, &oldtermio);
}

/* Read from keyboard with timeout.  For POSIX, we use VMIN/VTIME to implement
 * the timeout.  For no timeout, VMIN should be 1 and VTIME should be 0; for
 * timeout, VMIN should be 0 and VTIME should be the timeout value.
 */
int ttyread(buf, len, timeout)
	char	*buf;	/* where to place the read characters */
	int	len;	/* maximum number of characters to read */
	int	timeout;/* timeout (0 for none) */
{
	struct termios t, oldt;
	int	bytes;	/* number of bytes actually read */

	/* clear the "caught" variable */
	caught = 0;

	/* make sure the signal catcher hasn't been reset */
	signal(SIGINT, catchsig);

	/* arrange for timeout, and disable control chars */
	tcgetattr(0, &t);
	oldt = t;
	if (timeout)
	{
		t.c_cc[VMIN] = 0;
		t.c_cc[VTIME] = timeout;
	}
	else
	{
		t.c_cc[VMIN] = 1;
		t.c_cc[VTIME] = 0;
	}
	t.c_cc[VINTR] = t.c_cc[VQUIT] = t.c_cc[VSTART] = t.c_cc[VSTOP] = 0;
	tcsetattr(0, TCSANOW, &t);

	/* Perform the read. */
	bytes = read(0, buf, (unsigned)len);

	/* revert to previous mode */
	tcsetattr(0, TCSANOW, &oldt);

	/* return the number of bytes read */
	return bytes;
}

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
    int coli[2];
    _scrsize (coli);
    *colsptr = coli[0];
    *linesptr = coli[1];
	return True;
}

/* Check for signs of boredom from user, so we can abort a time-consuming
 * operation.  Here we check to see if SIGINT has been caught recently.
 */
BOOLEAN ttypoll(reset)
	BOOLEAN reset;
{
	return (caught & (1L << SIGINT)) ? True : False;
}


#else /* not emx */

void ttyinit (void)
{
}

void ttyraw (char *erasekey)
{
}

void ttynormal (void)
{
}

int ttyread (char *buf, int len, int timeout)
{
}

void ttywrite (char *buf, int len)
{
}

char *ttytermtype (void)
{
	char	*type;

	type = getenv ("TERM");
	if (!type)
	{
		type = TTY_DEFAULT;
	}
	return type;
}


/* This function gets the window size. */
BOOLEAN ttysize (int *linesptr, int *colsptr)
{
	/* no special way to detect screen size */
	return False;
}
#endif /* emx */
#endif /* GUI_TERMCAP || GUI_OPEN */
