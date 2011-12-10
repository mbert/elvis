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


static struct termios        oldtermio;        /* original tty mode */
static struct termios        newtermio;        /* cbreak/noecho tty mode */

static int ttykbd = 0;
static int ttyscr = 1;

/* this function is used to catch signals */
static void 
catchsig(int signo)
{
  caught = (1 << signo);
}

/* get the original tty state */
void 
ttyinit(void)
{
  /* get the old tty state */
  tcgetattr(0, &oldtermio);

#ifdef FEATURE_STDIN
  /* maybe use an alternate input */
  if (stdin_not_kbd)
    ttykbd = 2;
#endif

  /* maybe use an alternate output */
  if (!isatty(1))
    ttyscr = 2;
}

/* switch to the tty state that elvis runs in */
void ttyraw(char *erasekey)  /* where to store the ERASE char */
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
  tcsetattr(ttykbd, TCSADRAIN, &newtermio);
}

/* switch back to the original tty state */
void 
ttynormal(void)
{
  tcsetattr(ttykbd, TCSADRAIN, &oldtermio);
}

/* Read from keyboard with timeout.  For POSIX, we use VMIN/VTIME to implement
 * the timeout.  For no timeout, VMIN should be 1 and VTIME should be 0; for
 * timeout, VMIN should be 0 and VTIME should be the timeout value.
 */
int ttyread(char *buf,     /* where to place the read characters */
            int len,       /* maximum number of characters to read */
            int timeout)   /* timeout (0 for none) */
{
  struct termios t, oldt;
  int bytes;        /* number of bytes actually read */

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
  tcsetattr(ttykbd, TCSANOW, &t);

  /* Perform the read. */
  bytes = read(ttykbd, buf, (unsigned)len);

  /* revert to previous mode */
  tcsetattr(ttykbd, TCSANOW, &oldt);

  /* return the number of bytes read */
  return bytes;
}

/* write characters out to the screen */
void 
ttywrite (char *buf,     /* buffer, holds characters to be written */
         int len)        /* number of characters in buf */
{
  write(ttyscr, buf, (size_t)len);
}

/* determine the terminal type */
char *
ttytermtype (void)
{
  char        *type;

  type = getenv("TERM");
  if (!type)
    {
      type = TTY_DEFAULT;
    }
  return type;
}


/* This function gets the window size. */
ELVBOOL 
ttysize(int *linesptr,       /* where to store the number of rows */
        int *colsptr)        /* where to store the number of columns */
{
  int coli[2];
  _scrsize (coli);
  *colsptr = coli[0];
  *linesptr = coli[1];
  return ElvTrue;
}

/* Check for signs of boredom from user, so we can abort a time-consuming
 * operation.  Here we check to see if SIGINT has been caught recently.
 */
ELVBOOL 
ttypoll(ELVBOOL reset)
{
  return (caught & (1L << SIGINT)) ? ElvTrue : ElvFalse;
}


#else /* not emx */

void 
ttyinit (void)
{
}

void 
ttyraw (char *erasekey)
{
}

void 
ttynormal (void)
{
}

int 
ttyread (char *buf, int len, int timeout)
{
}

void 
ttywrite (char *buf, int len)
{
}

char *
ttytermtype (void)
{
  char        *type;

  type = getenv ("TERM");
  if (!type)
    {
      type = TTY_DEFAULT;
    }
  return type;
}


/* This function gets the window size. */
ELVBOOL 
ttysize (int *linesptr, int *colsptr)
{
  /* no special way to detect screen size */
  return ElvFalse;
}
#endif /* emx */
#endif /* GUI_TERMCAP || GUI_OPEN */
