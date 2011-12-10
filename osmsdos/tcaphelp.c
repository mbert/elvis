/* tcaphelp.c */


/* This file includes low-level tty control functions used by the termcap
 * user interface.  These are:
 *	ttyinit()		- remember the initial serial line configuration
 *	ttyraw()		- switch to the mode that elvis runs it
 *	ttynormal()		- switch back to the mode saved by ttyinit()
 *	ttyread(buf,len,timeout)- read characters, possibly with timeout
 *	ttywrite(buf,len)	- write characters
 *	ttytermtype()		- return the name of the terminal type
 *	ttysize()		- determine the terminal size
 *
 * Also, it contains a small terminal emulator to be used when TERM=bios.
 */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_tcaphelp[] = "$Id: tcaphelp.c,v 2.32 2003/10/17 17:41:23 steve Exp $";
#endif
#ifdef GUI_TERMCAP
# include <fcntl.h>
# include <dos.h>
# include <io.h>
# include <conio.h>
# include <signal.h>
# include "pcvideo.h"

# ifdef FEATURE_MOUSE
#  include "mouse.c"	/* Yes, "mouse.c" not "mouse.h" */
#  ifndef DBLCLICK
#   define DBLCLICK	40	/* double-click time, in units of 0.01 second */
#  endif
# endif

# ifndef O_TEXT
#  define O_TEXT	_O_TEXT
#  define O_BINARY	_O_BINARY
# endif

typedef enum {EvTimeout, EvSignal, EvKey, EvMouse} dosevent_t;

#if USE_PROTOTYPES
static long dostime(void);
static void catchsig(int signo);
static void yieldslice(void);
static dosevent_t getevent(int timeout, char str[3], short *x, short *y);
#endif

/* This is defined in guitcap.c */
extern long	ttycaught;

/* These variables remember the configuration of the console upon startup */
static int	origraw;	/* was it in raw mode? */
static int	origbrk;	/* was it looking for ^C? */
static int	origmode;	/* was it in O_TEXT mode, or O_BINARY? */

/* This variable is used to indicate that the BIOS interface is being used */
static ELVBOOL	usebios;

/* Width  & height of video display */
static int	pccols, pcrows;

/* IOCTL GETRAW/SETRAW bits. */
#define DEVICE		0x80
#define RAW		0x20

/* IOCTL operations */
#define GETRAW		0x4400
#define SETRAW		0x4401
#define GETBRK		0x3300
#define SETBRK		0x3301


/* A nice way to call the DOS IOCTL or  function */
static int elvioctl(int handle, int mode, unsigned setvalue)
{
	union REGS regs;

	regs.x.ax = mode;
	regs.x.bx = handle;
	regs.h.dl = (char) setvalue;
	regs.h.dh = 0;			/* Zero out dh */
	intdos(&regs, &regs);
	return (regs.x.dx);
}


/* This function catches signals, especially SIGINT */
static void catchsig(signo)
	int	signo;
{
	ttycaught |= (1 << signo);
}



void ttyinit()
{
	/* remember the original state */
	origraw = elvioctl(1, GETRAW, 0);
	origbrk = elvioctl(1, GETBRK, 0);
	origmode = setmode(1, O_TEXT);
}


/* switch to the tty state that elvis runs in */
void ttyraw(erasekey)
	char	*erasekey;	/* where to store the ERASE key */
{
#ifdef FEATURE_MOUSE
	PTRSHAPE ps;
#endif

	if (!usebios)
	{
		setmode(1, O_BINARY);
		if (origraw & DEVICE)
			(void)elvioctl(1, SETRAW, origraw | RAW);
	}
	(void)elvioctl(1, SETBRK, 0);
	signal(SIGINT, catchsig);

#ifdef FEATURE_MOUSE
	MouseInit();
	ps.t.chScreen = 255;
	ps.t.atScreen = 0x70;
	ps.t.chCursor = 0;
	ps.t.atCursor = 0x70;
	SetPtrShape(&ps);
	SetPtrVis(HIDE);
#endif
}


/* switch back to the original tty state */
void ttynormal()
{
	(void)elvioctl(1, SETBRK, origbrk);
	setmode(1, origmode);
	if (usebios)
	{
		v_attr((int)0xffff, (int)0x0707);
		v_put('\r');
		v_put('\n');
		v_ce();
	}
	else
	{
		if (origraw & DEVICE)
			(void)elvioctl(1, SETRAW, origraw);
		write(1, "\r\n", 2);
	}
}


/* this function returns the DOS time, as a 32-bit long int representing
 * hundredths of a second since midnight.  Some systems may be limited to
 * a resolution of whole seconds, but the values will still represent
 * hundredths.
 */
static long dostime P_((void))
{
        union REGS      regs;

        regs.h.ah = 0x2c;       /* MS-DOS "get time" service */
        intdos(&regs, &regs);
        return (((regs.h.ch * 60L) + regs.h.cl) * 60L + regs.h.dh) * 100L + regs.h.dl;
}


/* yield DOS timeslice, if running in a virtual machine */
static void yieldslice(void)
{
	union REGS	regs;
	struct SREGS	sregs;

	regs.x.ax = 0x352F;   /* get interrupt vector */
	int86x(0x0021, &regs, &regs, &sregs); /* Get DOS vector for 0x2F */

	if ((regs.x.ax == 0) && (sregs.es == 0)) 
	   return;

	regs.x.ax = 0x1680;   /* Release Current VM Time-Slice */
	int86(0x002f, &regs, &regs);

	return;
}


/* wait for a keyboard event, mouse event, signal, or timeout. */
static dosevent_t getevent(int timeout, char str[3], short *x, short *y)
{
	long	stop;
#ifdef FEATURE_MOUSE
	EVENT	mevent;	/* new state of mouse */
 static	EVENT	mstate;	/* previous state of mouse */
 static ELVBOOL	middle;	/* simulating middle button? */
 static	long	dbltime;/* time of last click, for detecting double-click */
 static char	dblbtn;	/* button of last click, for detecting double-click */
#endif

	stop = dostime() + timeout * 10L;

#ifdef FEATURE_MOUSE
	/* the mouse should be visible while we wait for an event */
	SetPtrVis(SHOW);
#endif

	/* wait for keystroke, timeout, or signal */
	do
	{
		/* signal? */
		if (ttycaught)
		{
#ifdef FEATURE_MOUSE
			SetPtrVis(HIDE);
#endif
			return EvSignal;
		}

		/* keystroke? */
		if (kbhit())
		{
			str[0] = getch();
			if (!str[0])
			{
				str[0] = '#';
				str[1] = getch();
				*x = 2;
			}
			else
			{
				*x = 1;
			}
#ifdef FEATURE_MOUSE
			SetPtrVis(HIDE);
#endif
			return EvKey;
		}

#ifdef FEATURE_MOUSE
		if (GetMouseEvent(&mevent) && (mevent.fsBtn || mstate.fsBtn))
		{
			/* which button changed? */
			switch (mevent.fsBtn == mstate.fsBtn
					? mstate.fsBtn :
					mevent.fsBtn ^ mstate.fsBtn)
			{
			  case LEFT_DOWN:
			  	str[0] = 'L';
			  	break;

			  case RIGHT_DOWN:
			  	str[0] = 'R';
			  	break;

			  default:
			  	str[0] = 'M';
			  	middle = ElvTrue;
			  	break;
			}
			if (!middle && mevent.fsBtn != LEFT_DOWN && mevent.fsBtn != RIGHT_DOWN)
			{
				middle = ElvTrue;
				str[0] = 'M';
			}

			/* pressed or released? */
			if (mevent.fsBtn == mstate.fsBtn)
				str[1] = 'm';
			else if (!mevent.fsBtn)
				str[1] = 'r';
			else
			{
				/* click or double-click? */
				if (str[0] == dblbtn && dostime() < dbltime + DBLCLICK)
				{
					/* double-click */
					str[1] = 'd';
				}
				else
				{
					/* single click */
					str[1] = 'p';

					/* to help detect dblclk next time... */
					if (str[0] != 'M')
					{
						dblbtn = str[0];
						dbltime = dostime();
					}
				}
			}

			/* other stuff */
			*x = mevent.x - 1;	/* leftmost should be 0 */
			*y = mevent.y - 1;	/* top row should be 0 */
			mstate = mevent;

			SetPtrVis(HIDE);
			return EvMouse;
		}
#endif

		/* nothing happening in this VM - give other VMs a shot */
		yieldslice();
	} while (timeout == 0 || dostime() < stop);

#ifdef FEATURE_MOUSE
	SetPtrVis(HIDE);
#endif
	return EvTimeout;
}



/* Read from keyboard, with timeout.  For DOS, we poll the keyboard in a
 * tight loop until we have a keystroke or the system's clock advances past
 * our timeout time.  If we don't time out, then we loop until there are no
 * more characters, or the buffer is about full.
 *
 * Returns -2 if mouse event occured; it also calls eventclick() here.
 * Returns -1 if screen size may have changed, or other signal received.
 * Returns 0 if timeout occurred without any other events.
 * Returns number of characters received otherwise.
 */
int ttyread(buf, len, timeout)
	char	*buf;	/* where to place the input characters */
	int	len;	/* maximum number of characters to read */
	int	timeout;/* timeout (0 for none) */
{
	short	x, y;	/* mouse position, for mouse events */
	int	got;
#ifdef FEATURE_MOUSE
	GUIWIN	*gw;	/* window where mouse event occurred */
 static GUIWIN	*selgw;	/* window where a selection is taking place, or NULL */
 	int	wx, wy;	/* current mouse position within the window */
 static int	px, py;	/* earlier clicked position within the window */
 static char	pe;	/* previous event */
	dosevent_t event;/* a mouse or keyboard event */

	/* mouse should be visible while waiting for event */
	SetPtrVis(SHOW);
#endif

	signal(SIGINT, catchsig);

	/* reset the "ttycaught" variable */
	ttycaught = 0;


        /* get at least one keystroke */
	got = 0;
	while (got + 1 < len &&
		(event = getevent(timeout, &buf[got], &x, &y)) != EvTimeout)
	{
		switch (event)
		{
		  case EvSignal:
			return -1;

		  case EvKey:
			got += x;
			break;

#ifdef FEATURE_MOUSE
		  case EvMouse:
			/* which window was clicked? */
			gw = ttywindow((int)y, (int)x, &wy, &wx);
			if (!gw || (selgw && selgw != gw))
				break;

			/* make it become the current window */
			(*gui->focusgw)(gw);
			eventfocus(gw, ElvTrue);

			/* handle the event */
			switch (buf[got + 1])
			{
			  case 'd':	/* double-click */
				if (buf[got] == 'L')
					eventclick(gw, wy, wx, CLICK_TAG);
				else
					eventclick(gw, wy, wx, CLICK_UNTAG);
				selgw = NULL;
				pe = 'd';
				return -2;

			  case 'm':	/* move, with a button pressed */
				if (pe != 'm')
				{
					switch (buf[got])
					{
					  case 'L':
						eventclick(gw, py, px, CLICK_SELCHAR);
						break;

					  case 'M':
						eventclick(gw, py, px, CLICK_SELRECT);
						break;

					  case 'R':
						eventclick(gw, py, px, CLICK_SELLINE);
						break;
					}
				}
				eventclick(gw, wy, wx, CLICK_MOVE);
				pe = 'm';
				return -2;

			  case 'p':	/* button pressed */
				if (buf[got] == 'L')
					eventclick(gw, wy, wx, CLICK_CANCEL);
				eventclick(gw, wy, wx, CLICK_MOVE);
				py = wy, px = wx;
				selgw = gw;
				pe = 'p';
				return -2;

			  case 'r':	/* button released */
			  	selgw = NULL;
				pe = 'r';
			}
			break;
#endif
		}

		/* After the first pass, we want to use the shorted possible
		 * timeout.  Since 0 means "never expire", we set timeout to
		 * -1 meaning "give up immediately".  It'll still make one
		 * pass through the event checking loop, though.
		 */
		timeout = -1;
	}

	/* timed out -- possibly after reading some characters */
	return got;
}


/* write characters out to the screen */
void ttywrite(buf, len)
	char	*buf;	/* buffer, holds characters to be written */
	int	len;	/* number of characters in buf */
{
static  int	arg[5];		/* arguments to an escape sequence */
static	int	argno = -1;	/* # of args in arg[], or -1 is not in Esc */
static	int	x, y;		/* cursor position */
static	ELVBOOL	rvid;		/* in reverse-video mode? */
static	ELVBOOL	uvid;		/* in false-underline mode? */
static	ELVBOOL colored;	/* explicit colors set? (disables uvid) */
	int	i, j;

	/* if not using the BIOS, then just write the characters to stdout */
	if (!usebios)
	{
		write(1, buf, len);
		return;
	}

	/* handle each character separately */
	for (i = 0; i < len; i++)
	{
		if (buf[i] == '\007')
		{
			v_put('\007');
		}
		else if (buf[i] == '\r')
		{
			x = 0;
			v_move(x, y);
		}
		else if (buf[i] == '\n')
		{
			if (y < pcrows - 1)
			{
				y++;
				v_move(x, y);
			}
			else
			{
				v_put('\n');
			}
		}
		else if (buf[i] == '\b')
		{
			x--;
			if (x < 0)
			{
				if (y > 0)
				{
					x += pccols;
					y--;
				}
				else
					x = 0;
			}
			v_move(x, y);	
		}
		else if (buf[i] == '\033')
		{
			/* start an escape sequence */
			for (j = 0; j < QTY(arg); j++)
				arg[j] = 0;
			argno = 0;
		}
		else if (argno < 0)
		{
			/* normal character */
			v_put(buf[i]);
			x++;
			if (x >= pccols)
			{
				x = 0;
				y++;
			}
			v_move(x, y);
		}
		else
		{
			/* in an escape sequence... */
			switch (buf[i])
			{
			  case '[':
			  case '?':
				/* ignored */
				break;

			  case ';':
				/* advance to next argument */
				argno++;
				break;

			  case '0':
			  case '1':
			  case '2':
			  case '3':
			  case '4':
			  case '5':
			  case '6':
			  case '7':
			  case '8':
			  case '9':
				/* incorporate this digit into current arg */
				arg[argno] = arg[argno] * 10 + buf[i] - '0';
				break;

			  case 'H':
				/* move the cursor */
				x = (arg[1] ? arg[1] - 1 : 0);
				y = (arg[0] ? arg[0] - 1 : 0);
				v_move(x, y);

				argno = -1;
				break;

			  case 'J':
				/* clear the screen */
				v_cl();
				argno = -1;
				break;

			  case 'K':
				/* clear to end-of-line */
				v_ce();
				argno = -1;
				break;

			  case 'L':
				/* insert n lines */
				v_al(arg[0] ? arg[0] : 1);
				argno = -1;
				break;

			  case 'M':
				/* delete n lines */
				v_dl(arg[0] ? arg[0] : 1);
				argno = -1;
				break;

			  case 'h':
				if (arg[0] == 12)
					v_cb(); /* cursor big */
				argno = -1;
				break;
				
			  case 'l':
				if (arg[0] == 12)
					v_cs(); /* cursor small */
				argno = -1;
				break;

			  case 'm':
				for (j = 0; j <= argno; j++)
				{
					switch (arg[j])
					{
					  case 0:
						/* Resetting attributes is a
						 * little tricky for color.  We
						 * want to force blink/bright
						 * off but leave the colors...
						 * Unless we were in standout
						 * mode, in which case we want
						 * to flip colors.
						 */
						if (rvid)
						{
							v_attr(0x0000, 0x0077);
							rvid = ElvFalse;
						}
						if (uvid && !colored)
						{
							v_attr(0x0000, 0x0040);
							uvid = ElvFalse;
						}
						colored = ElvFalse;
						v_attr(0xff88, 0x0700);
						break;

					  case 1:
					  	v_attr(0x0808, 0x0808);
					  	break;

					  case 4:
						if (colored)
						{
							v_attr(0x7700, 0x0100);
						}
						else
						{
							uvid = (ELVBOOL)!uvid;
							v_attr(0x7700, 0x0140);
						}
					  	break;

					  case 5: 
					  	v_attr(0x8080, 0x8080);
					  	break;

					  case 7:
						/* Setting standout mode is a
						 * little tricky for mono,
						 * because we need to force
						 * underlining off.
						 */
						rvid = (ELVBOOL)!rvid;
						v_attr(0x7700, rvid ? 0x7077 : 0x0777);
						break;

					  case 30: v_attr(0x0007, 0x0000);break;
					  case 31: v_attr(0x0007, 0x0004);break;
					  case 32: v_attr(0x0007, 0x0002);break;
					  case 33: v_attr(0x0007, 0x0006);break;
					  case 34: v_attr(0x0007, 0x0001);break;
					  case 35: v_attr(0x0007, 0x0005);break;
					  case 36: v_attr(0x0007, 0x0003);break;
					  case 37: v_attr(0x0007, 0x0007);break;
					  case 40: v_attr(0x0070, 0x0000);
						   colored = ElvTrue;	  break;
					  case 41: v_attr(0x0070, 0x0040);
						   colored = ElvTrue;	  break;
					  case 42: v_attr(0x0070, 0x0020);
						   colored = ElvTrue;	  break;
					  case 43: v_attr(0x0070, 0x0060);
						   colored = ElvTrue;	  break;
					  case 44: v_attr(0x0070, 0x0010);
						   colored = ElvTrue;	  break;
					  case 45: v_attr(0x0070, 0x0050);
						   colored = ElvTrue;	  break;
					  case 46: v_attr(0x0070, 0x0030);
						   colored = ElvTrue;	  break;
					  case 47: v_attr(0x0070, 0x0070);
						   colored = uvid;	  break;
					}
				}
				argno = -1;
				break;

			  default:
				/* does it look like the end of a command? */
				if ((buf[i] & 0x40) != 0)
				{
					argno = -1;
				}
			}
		}
	}
}


/* determine the terminal type */
char *ttytermtype()
{
	char	*type;

	type = getenv("TERM");
	if (!type)
		type = TTY_DEFAULT;
	if (!strcmp(type, "ansi"))
		type = "dosansi";
	usebios = (ELVBOOL)!strcmp(type, "pcbios");
	return type;
}


/* This function gets the window size. */
ELVBOOL ttysize(linesptr, colsptr)
	int	*linesptr;	/* where to store the number of rows */
	int	*colsptr;	/* where to store the number of columns */
{
	pccols = *colsptr = v_cols();
	pcrows = *linesptr = v_rows();
	return ElvTrue;
}


/* Check for signs of boredom from user, so we can abort a time-consuming
 * operation.  Here we check to see if SIGINT has been caught recently.
 */
ELVBOOL ttypoll(reset)
	ELVBOOL	reset;
{
	(void)kbhit();
	return (ELVBOOL)(ttycaught != 0);
}
#endif /* GUI_TERMCAP */
