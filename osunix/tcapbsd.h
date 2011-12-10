/* tcapbsd.h */

char id_tcapbsd[] = "$Id: tcapbsd.h,v 2.4 1996/04/03 21:43:19 steve Exp $";

#include <sys/types.h>
#include <sys/time.h>
#include <sgtty.h>
#include <signal.h>

static struct sgttyb	oldsgttyb;	/* original tty mode */
static struct sgttyb	newsgttyb;	/* cbreak/nl/noecho tty mode */
static int		oldint;		/* ^C or DEL, the "intr" character */
static int		oldstart;	/* ^Q the "start" or "xon" character */
static int		oldstop;	/* ^S the "stop" or "xoff character */
static int		oldswitch;	/* ^Z, the "suspend" character */
static int		olddswitch;	/* ^Y, the "delayed suspend" char */
static int		oldquote;	/* ^V, the "quote next char" char */
static int		oldflush;	/* ^O, the "flush output" char */

/* This function is used for catching signals */
static void catchsig(signo)
	int	signo;
{
	caught |= (1 << signo);
}

/* save the original tty state */
void ttyinit()
{
	/* get the old tty state */
	ioctl(0, TIOCGETP, &oldsgttyb);
}

/* switch to the tty state that elvis runs in */
void ttyraw(erasekey)
	char	*erasekey;	/* where to store the ERASE key */
{
	struct tchars	tbuf;
#  ifdef TIOCSLTC
	struct ltchars	ltbuf;
#  endif

	/* arrange for signals to be caught or ignored */
	signal(SIGHUP, catchsig);
	signal(SIGINT, catchsig);
	signal(SIGQUIT, SIG_IGN);
#ifdef SIGWINCH
	signal(SIGWINCH, catchsig);
#endif

	/* make ospeed and the erase key value be accessible outside this file */
	ospeed = oldsgttyb.sg_ospeed;
	*erasekey = oldsgttyb.sg_erase;

	/* switch to raw mode */
	newsgttyb = oldsgttyb;
	newsgttyb.sg_flags |= CBREAK;
	newsgttyb.sg_flags &= ~(CRMOD|ECHO|XTABS);
	ioctl(0, TIOCSETP, &newsgttyb);

	ioctl(0, TIOCGETC, (struct sgttyb *) &tbuf);
	oldint = tbuf.t_intrc;
	tbuf.t_intrc = ELVCTRL('C');	/* always use ^C for interrupts */
	ioctl(0, TIOCSETC, (struct sgttyb *) &tbuf);

#  ifdef TIOCSLTC
	ioctl(0, TIOCGLTC, &ltbuf);
	oldswitch = ltbuf.t_suspc;
	ltbuf.t_suspc = 0;		/* disable ^Z for elvis */
	olddswitch = ltbuf.t_dsuspc;
	ltbuf.t_dsuspc = 0;		/* disable ^Y for elvis */
	oldquote = ltbuf.t_lnextc;
	ltbuf.t_lnextc = 0;		/* disable ^V for elvis */
	oldflush = ltbuf.t_flushc;
	ltbuf.t_flushc = 0;		/* disable ^O for elvis */
	ioctl(0, TIOCSLTC, &ltbuf);
#  endif
}


/* switch back to the original tty state */
void ttynormal()
{
	struct tchars	tbuf;
#  ifdef TIOCSLTC
	struct ltchars	ltbuf;
#  endif

	ioctl(0, TIOCSETP, &oldsgttyb);

	ioctl(0, TIOCGETC, (struct sgttyb *) &tbuf);
	tbuf.t_intrc = oldint;
	tbuf.t_startc = oldstart;
	tbuf.t_stopc = oldstop;
	ioctl(0, TIOCSETC, (struct sgttyb *) &tbuf);

#  ifdef TIOCSLTC
	ioctl(0, TIOCGLTC, &ltbuf);
	ltbuf.t_suspc = oldswitch;
	ltbuf.t_dsuspc = olddswitch;
	ltbuf.t_lnextc = oldquote;
	ltbuf.t_flushc = oldflush;
	ioctl(0, TIOCSLTC, &ltbuf);
#  endif
}

/* Read from keyboard, with timeout.  For BSD, we use select() to wait for
 * characters to become available, and then do a read() to actually get the
 * characters.
 */
int ttyread(buf, len, timeout)
	char	*buf;	/* where to place the input characters */
	int	len;	/* maximum number of characters to read */
	int	timeout;/* timeout (0 for none) */
{
	fd_set	rd;	/* the file descriptors that we want to read from */
	static	tty;	/* 'y' if reading from tty, or 'n' if not a tty */
	int	i;
	struct timeval t;
	struct timeval *tp;


	/* reset the "caught" variable */
	caught = 0;

	/* do we know whether this is a tty or not? */
	if (!tty)
	{
		tty = (isatty(0) ? 'y' : 'n');
	}

	/* compute the timeout value */
	if (timeout)
	{
		t.tv_sec = timeout / 10;
		t.tv_usec = (timeout % 10) * 100000L;
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
		  case -1:
			/* probably SIGWINCH */
			return -1;
	
		  case 0:
			/* timeout or EOF */
			return 0;
	
		  default:
			/* characters available */
			return read(0, buf, len);
		}
	}
}
