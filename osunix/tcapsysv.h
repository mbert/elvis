/* tcapsysv.h */

char id_tcapsysv[] = "$Id: tcapsysv.h,v 2.7 1996/04/28 18:18:27 steve Exp $";

#include <termio.h>
#include <signal.h>
#include <unistd.h>

/* HPUX does a "#define ttysize winsize".  Elvis doesn't like that. */
#undef ttysize

static struct termio	oldtermio;	/* original tty mode */
static struct termio	newtermio;	/* cbreak/noecho tty mode */

/* signal catching function */
void catchsig(signo)
	int	signo;
{
	caught |= (1<<signo);
}


/* remember the original tty state */
void ttyinit()
{
	/* get the old tty state */
	ioctl(0, TCGETA, &oldtermio);
}

/* switch back to the original tty state */
void ttynormal()
{
	ioctl(0, TCSETAW, &oldtermio);
}

/* switch to the tty state that elvis runs in */
void ttyraw(erasekey)
	char	*erasekey;	/* where to store the ERASE key */
{
	/* arrange for signals to be caught or ignored */
	signal(SIGHUP, catchsig);
	signal(SIGINT, catchsig);
#ifdef SIGWINCH
	signal(SIGWINCH, catchsig);
#endif
	signal(SIGQUIT, SIG_IGN);

	ospeed = (oldtermio.c_cflag & CBAUD);
	*erasekey = oldtermio.c_cc[VERASE];
	newtermio = oldtermio;
#  ifdef IXANY
	newtermio.c_iflag &= (IXON|IXOFF|IXANY|ISTRIP|IGNBRK);
#  else
	newtermio.c_iflag &= (IXON|IXOFF|ISTRIP|IGNBRK);
#  endif
	newtermio.c_oflag &= ~OPOST;
	newtermio.c_lflag &= ISIG;
	newtermio.c_cc[VINTR] = ELVCTRL('C'); /* always use ^C for interrupts */
	newtermio.c_cc[VMIN] = 1;
	newtermio.c_cc[VTIME] = 0;
#  ifdef VSWTCH
	newtermio.c_cc[VSWTCH] = 0;
#  endif
#  ifdef VSWTC
	newtermio.c_cc[VSWTC] = 0;
#  endif
#  ifdef VSUSP
#   if VSUSP < NCC
	newtermio.c_cc[VSUSP] = 0;
#   endif
#  endif
#  ifdef VDSUSP
	newtermio.c_cc[VSUSP] = 0;
#  endif
	ioctl(0, TCSETAW, &newtermio);
}

/* For System-V, we use VMIN/VTIME to implement the timeout.  For no
 * timeout, VMIN should be 1 and VTIME should be 0; for timeout, VMIN
 * should be 0 and VTIME should be the timeout value.
 */
int ttyread(buf, len, timeout)
	char	*buf;	/* where to store the input characters */
	int	len;	/* maximum number of characters to read */
	int	timeout;/* timeout (or 0 for none) */
{
	struct termio t, oldt;
	int	bytes;	/* number of bytes actually read */

	/* reset the "caught" variable */
	caught = 0;

	/* make sure the signal handler is still in place */
	signal(SIGHUP, catchsig);
	signal(SIGINT, catchsig);
#ifdef SIGWINCH
	signal(SIGWINCH, catchsig);
#endif

	/* arrange for timeout, and disable special keys */
	ioctl(0, TCGETA, &t);
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
	t.c_cc[VINTR] = t.c_cc[VQUIT] = 0;
#ifdef VSTART
	t.c_cc[VSTART] = t.c_cc[VSTOP] = 0;
#endif
	ioctl(0, TCSETA, &t);

	/* Perform the read. */
	bytes = read(0, buf, (unsigned)len);
	
	/* set the tty back to ordinary raw mode */
	ioctl(0, TCSETA, &oldt);

	/* return the number of bytes read */
	return bytes;
}
