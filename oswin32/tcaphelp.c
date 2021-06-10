/* oswin32/tcaphelp.c */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_tcaphelp[] = "$Id: tcaphelp.c,v 2.35 2004/01/30 00:55:08 steve Exp $";
#endif
#if defined(GUI_TERMCAP) || defined(GUI_OPEN)
# define CHAR CHAR_nt
# include <windows.h>
# include <fcntl.h>
# include <io.h>
# include <signal.h>
# undef CHAR
# define SMART_LINE_WRAP 1

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


/* This variable is defined in guitcap.c */
extern long ttycaught;

static void catchsig(int signo);
static void retitle(GUIWIN *gw, char *name);
static ELVBOOL clipopen(ELVBOOL forwrite);
static int clipwrite(CHAR *text, int len);
static int clipread(CHAR *text, int len);
static void clipclose(void);
static void switchcsbi(HANDLE which, COORD *curpos);
static int optisttyrows(OPTDESC *opt, OPTVAL *val, CHAR *newval);
static int optisttycols(OPTDESC *opt, OPTVAL *val, CHAR *newval);
static void ttyresize(int rows, int cols);

static ELVBOOL	useconsole = ElvTrue;
static HANDLE	inConsole, outConsole;	/* the DOS console buffer */
static HANDLE	myConsole;		/* elvis' console buffer */
static HANDLE	console;		/* the current console buffer */
static CONSOLE_SCREEN_BUFFER_INFO consinfo;/* info about the console */
static DWORD	inMode, outMode;	/* I/O modes of DOS console buffer */
static int	prevWidth, prevHeight;	/* size of DOS console buffer */
static ELVBOOL	resized;		/* has screen size changed lately? */
static WORD	origattr;		/* DOS console attributes */
static WORD	myattr;			/* elvis' console attributes */
static WORD	attr;	/* attribute byte for writing subsequent text */
static int	iniWidth, iniHeight;	/* init size of DOS console buffer */
static int	curWidth, curHeight;	/* curr size of DOS console buffer */

/* These store info about the GUI's clip buffer */
static HGLOBAL  clip_hGlob = NULL;
static char     *clip_data = 0;
static int      clip_len = 0;
static int      clip_offset = 0;

/* The window's original title is stored here */
static char	orig_title[256];

/* The codepage option stores the code page */
static OPTVAL win32val[1];
#define o_codepage	win32val[0].value.number
static int setcp(OPTDESC *opt, OPTVAL *val, CHAR *newval)
{
	UINT	newcp = atol(newval);
	if (!newcp && newval[0] != '0')
	{
		msg(MSG_ERROR, "[s]$1 requires a numeric value", opt->longname);
		return -1;
	}
	if (!SetConsoleOutputCP(newcp) || !SetConsoleCP(newcp))
	{
		msg(MSG_ERROR, "[sd]invalid $1 $2", opt->longname, (long)newcp);
		return -1;
	}
	o_codepage = newcp;
	return 1;
}
static OPTDESC win32desc[] =
{
	{"codepage", "cpg",	optnstring,	setcp}
};


static int optisttyrows (opt, val, newval)
	OPTDESC		*opt;
	OPTVAL		*val;
	CHAR		*newval;
{
	int	lines;
	int	cols;
	long	value = atol ((char *)newval);

	if (value < 2)
		return -1;
	else if (value == val->value.number)
		return 0;
	switchcsbi(myConsole, NULL);
	ttyresize(value, -1);
	ttysize(&lines, &cols);
	o_ttyrows = lines;
	o_ttycolumns = cols;
	if (windefault)
		resized = ElvTrue;

	return 1;
}

static int optisttycols (opt, val, newval)
	OPTDESC		*opt;
	OPTVAL		*val;
	CHAR		*newval;
{
	int	lines;
	int	cols;
	long	value = atol ((char *)newval);

	if (value < 30)
		return -1;
	else if (value == val->value.number)
		return 0;
	switchcsbi(myConsole, NULL);
	ttyresize(-1, value);
	ttysize(&lines, &cols);
	o_ttyrows = lines;
	o_ttycolumns = cols;
	if (windefault)
		resized = ElvTrue;

	return 1;
}
/* This function catches signals, especially SIGINT */
static void catchsig(signo)
	int	signo;
{
	ttycaught |= (1 << signo);
}


/* remember the original terminal settings */
void ttyinit()
{
	SECURITY_ATTRIBUTES	sec;
	COORD			size;
	COORD			home;
	DWORD			dummy;
	OPTDESC			*desc;

	/* get handles of the console */
	inConsole = GetStdHandle(STD_INPUT_HANDLE);
	assert(inConsole != INVALID_HANDLE_VALUE);
	outConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	assert(outConsole != INVALID_HANDLE_VALUE);

	/* remember the original modes of those handles */
	GetConsoleMode(inConsole, &inMode);
	GetConsoleMode(outConsole, &outMode);

	/* create a new console buffer, the same size as the window */
	GetConsoleScreenBufferInfo(outConsole, &consinfo);
	sec.nLength = sizeof(sec);
	sec.lpSecurityDescriptor = NULL;
	sec.bInheritHandle = FALSE;
	myConsole = CreateConsoleScreenBuffer(
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			&sec,
			CONSOLE_TEXTMODE_BUFFER,
			NULL);
	assert(myConsole != INVALID_HANDLE_VALUE);

	/* set the codepage option */
	o_codepage = GetConsoleOutputCP();
	SetConsoleCP((UINT)o_codepage);
	optinsert("win32", QTY(win32val), win32desc, win32val);

	/* make the ttyrows and ttycolumns global options use win32-specific
	 * functions for changing their values.
	 */
	if (optgetstr("ttyrows", &desc))
		desc->isvalid = optisttyrows;
	if (optgetstr("ttycolumns", &desc))
		desc->isvalid = optisttycols;

	/* change the new buffer's size to match the current buffer's window */
	iniWidth = curWidth = prevWidth = size.X = consinfo.srWindow.Right - consinfo.srWindow.Left + 1;
	iniHeight = curHeight = prevHeight = size.Y = consinfo.srWindow.Bottom - consinfo.srWindow.Top + 1;
	SetConsoleScreenBufferSize(myConsole, size);

	/* make the default colors of the new console buffer be the same as the
	 * old console buffer.
	 */
	attr = myattr = origattr = consinfo.wAttributes;
	home.X = home.Y = 0;
	FillConsoleOutputAttribute(myConsole, myattr, size.X * size.Y, home, &dummy);
	FillConsoleOutputCharacter(myConsole, ' ', size.X * size.Y, home, &dummy);

	/* remember the window's original title */
	GetConsoleTitle (orig_title, sizeof orig_title);

	/* enable the retitle and clip operations */
	guitermcap.retitle = retitle;
	guitermcap.clipopen = clipopen;
	guitermcap.clipwrite = clipwrite;
	guitermcap.clipread = clipread;
	guitermcap.clipclose = clipclose;
}


/* switch to the tty state that elvis runs in */
void ttyraw(erasekey)
	char	*erasekey;	/* where to store the ERASE key */
{
	DWORD	newmode;

	if (useconsole)
	{
		/* recheck the console handles, just in case! */
		inConsole = GetStdHandle(STD_INPUT_HANDLE);
		assert(inConsole != INVALID_HANDLE_VALUE);
		outConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		assert(outConsole != INVALID_HANDLE_VALUE);

		/* switch to "raw" mode, and allow the window to be resized */
#if 0
		newmode = ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT | ENABLE_WRAP_AT_EOL_OUTPUT;
		SetConsoleMode(inConsole, newmode);
		SetConsoleMode(outConsole, newmode|ENABLE_PROCESSED_OUTPUT);
		SetConsoleMode(myConsole, newmode);
#else
		newmode = ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT;
		SetConsoleMode(inConsole, newmode);
		SetConsoleMode(outConsole, newmode);
		SetConsoleMode(myConsole, newmode);
#endif
	}
	else
	{
		/* switch to binary mode */
		_setmode(1, _O_BINARY);
	}

	/* trap ^C signals */
	signal(SIGINT, catchsig);
}


/* switch back to the original tty state */
void ttynormal()
{
	if (useconsole)
	{
		/* switch back to the original mode */
		switchcsbi(outConsole, NULL);
		SetConsoleMode(inConsole, inMode);
		SetConsoleMode(outConsole, outMode);
	}
	else
	{
		/* switch to text mode */
		_setmode(1, _O_TEXT);
	}
}


/* This function switches between screen buffers. */
static void switchcsbi(HANDLE which, COORD * curpos)
{
	int	savedHeight;
	int	savedWidth;
	CONSOLE_SCREEN_BUFFER_INFO localconsinfo;

#if 1
	/* if no change, then do nothing */
	if (which == console)
		return;
#endif
	/* resize to original size if switching to original console */
	if (which == outConsole &&
		(iniHeight != curHeight || iniWidth != curWidth))
	{
		/* resize, buf save current size in case of subshell */
		savedHeight = curHeight;
		savedWidth = curWidth;
		ttyresize (iniHeight, iniWidth);
		curHeight = savedHeight;
		curWidth = savedWidth;
	}

	/* switch to the requested console */
	SetConsoleActiveScreenBuffer(which);
	console = which;

	/* if switching to non-elvis screen, then restore the title */
	if (console == outConsole)
	{
		/* set the title */
		SetConsoleTitle(orig_title);
	}
	else
	{
		/* resize if switching to private console */
		if (iniHeight != curHeight || iniWidth != curWidth)
		{
			ttyresize (curHeight, curWidth);
			ttysize (&savedHeight, &savedWidth);
		}
		attr = myattr;
	}

	if (curpos != NULL)
	{
		GetConsoleScreenBufferInfo(console, &localconsinfo);
		*curpos = localconsinfo.dwCursorPosition;
	}
}


/* Read from keyboard, with timeout.  Since Win32 apparently can't read with
 * timeout, this function simulates a timeout immediately if timeouts are
 * allowed within 0.5 seconds, and waits forever for a keystroke if timeouts
 * are either disabled, or are allowed in more that 0.5 seconds.  Also,
 * if the console is resized it returns -1 to simulate a SIGWINCH signal.
 * After a mouse event, it returns -2 so that windows can be redrawn.
 */
int ttyread(buf, len, timeout)
	char	*buf;	/* where to place the input characters */
	int	len;	/* maximum number of characters to read */
	int	timeout;/* timeout (0 for none) */
{
	INPUT_RECORD event;/* buffer, holds an input record */
	DWORD	mode;	/* Console mode */
	DWORD	nevents;/* number of events read into event */
	int	e;	/* for counting through event */
	int	got;	/* character counter, for keystrokes */
static	DWORD	prevmb;	/* previous mouse button state */
	DWORD	press;	/* bitmap of new button presses */
	GUIWIN	*gw;	/* window where mouse event happened */
static	GUIWIN	*selgw;	/* window where selection is taking place */
	int	y, x;	/* coordinates of mouse within "gw" window */
static	ELVBOOL	justpressed;/* between a press and the start of a drag */
static	int	prevy, prevx;/* cell where originally pressed */
static	ELVBOOL	justdbl;/* between double-click & bogus single-click */

	/* reset the "ttycaught" variable */
	signal(SIGINT, catchsig);
	ttycaught = 0;

	/* if timeouts are allowed within 0.5 seconds, then return 0
	 * immediately unless there's already a keystroke waiting.
	 */
	if (timeout > 0
	 && timeout <= 5
	 && (!useconsole || (PeekConsoleInput(inConsole, &event, 1, &nevents)
			     && nevents == 0)))
	{
		return 0;
	}

	/* if resized since last read, then return -1 to simulate the value
	 * that UNIX's read() call would return after a SIGWINCH.
	 */
	if (resized)
	{
		resized = ElvFalse;
		return -1;
	}

	/* if not using console, then just read from stdin */
	if (!useconsole)
	{
		return fread(buf, len, sizeof(char), stdin);
	}

	/* initialize the mouse variables */
	gw = NULL;

	/* disallow SIGINT while in loop */
	GetConsoleMode(inConsole, &mode);
	SetConsoleMode(inConsole, mode & ~ENABLE_PROCESSED_INPUT);
	signal(SIGINT, catchsig); /* just in case of bad timing! */

	/* get at least one event.  The only events we pay attention to are
	 * window events (resizing), keyboard events, and mouse events.
	 */
	got = nevents = 0;
	do
	{
		if (!ReadConsoleInput(inConsole, &event, 1, &nevents))
		{
			/* How could ReadConsoleInput() fail? */
			return -1;
		}

		/* process the event */
		switch (event.EventType)
		{
		  case KEY_EVENT:
			if (event.Event.KeyEvent.bKeyDown)
			{
				for (x = event.Event.KeyEvent.wRepeatCount;
				     x > 0 && got + 2 < len;
				     x--)
				{
					if (event.Event.KeyEvent.uChar.AsciiChar == '\t'
					 && (event.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED) != 0)
					{
						buf[got++] = '#';
						buf[got++] = '\t';
					}
					else if (event.Event.KeyEvent.uChar.AsciiChar)
					{
						/* plain text character*/
						buf[got++] = event.Event.KeyEvent.uChar.AsciiChar;
					}
					else if (3 == (char)event.Event.KeyEvent.wVirtualScanCode)
					{
						/* Ctrl-2 should be a NUL character */
						buf[got++] = '\0';
					}
					else if (7 == (char)event.Event.KeyEvent.wVirtualScanCode)
					{
						/* Ctrl-6 should be a ^^ character */
						buf[got++] = ELVCTRL('^');
					}
					else if (event.Event.KeyEvent.wVirtualScanCode >= ' '
					      && !strchr("EF68*:", (char)event.Event.KeyEvent.wVirtualScanCode))
					{
						buf[got++] = '#';
						buf[got++] = (char)event.Event.KeyEvent.wVirtualScanCode;
					}
				}
			}
			break;

		  case MOUSE_EVENT:
			/* Ignore if no buttons pressed.  Also, ignore anything
			 * after a double-click until button is released.
			 */
			if (justdbl)
			{
				prevmb = event.Event.MouseEvent.dwButtonState;
				if (prevmb == 0)
					justdbl = ElvFalse;
				break;
			}
			else if (event.Event.MouseEvent.dwButtonState == 0
				&& prevmb == 0)
			{
				break;
			}

			/* Figure out which window the event occurred in */
			gw = ttywindow(event.Event.MouseEvent.dwMousePosition.Y,
				       event.Event.MouseEvent.dwMousePosition.X,
				       &y, &x);
			if (!gw || (selgw && selgw != gw))
				break;
			press = (event.Event.MouseEvent.dwButtonState & ~prevmb);
			prevmb = event.Event.MouseEvent.dwButtonState;

			/* Make the window become the current window */
			(*gui->focusgw)(gw);
			eventfocus(gw, ElvTrue);

			/* process the event */
			if (event.Event.MouseEvent.dwEventFlags & DOUBLE_CLICK)
			{
				/* DOUBLE CLICK */
				if (event.Event.MouseEvent.dwButtonState & 1)
					eventclick(gw, y, x, CLICK_TAG);
				else
					eventclick(gw, y, x, CLICK_UNTAG);
				SetConsoleMode(inConsole, mode);
				selgw = NULL;
				justdbl = ElvTrue;
				return -2;
			}
			if ((event.Event.MouseEvent.dwEventFlags & MOUSE_MOVED) != 0 && justpressed && (x != prevx || y != prevy))
			{
				/* starting a draw-through */
				switch (event.Event.MouseEvent.dwButtonState)
				{
				  case 1:
					eventclick(gw, prevy, prevx, CLICK_SELCHAR);
					break;

				  case 2:
					eventclick(gw, prevy, prevx, CLICK_SELLINE);
					break;

				  default:
					eventclick(gw, prevy, prevx, CLICK_SELRECT);
				}
				justpressed = ElvFalse;
			}
			else if (press != 0)
			{
				/* button was pressed */
				if (press & 1)
					eventclick(gw, y, x, CLICK_CANCEL);
				justpressed = ElvTrue;
				selgw = gw;
			}
			else if (prevmb == 0)
			{
				/* last button released */
				selgw = NULL;
			}

			/* move the cursor to follow the mouse */
			eventclick(gw, y, x, CLICK_MOVE);
			prevx = x;
			prevy = y;
			SetConsoleMode(inConsole, mode);
			return -2;

		  case WINDOW_BUFFER_SIZE_EVENT:
			/* Return -1 to simulate the EINTR error that a UNIX
			 * read() function would have after SIGWINCH arrived.
			 * Discard any preceding keystokes.
			 */
			SetConsoleMode(inConsole, mode);
			return -1;
		}

		/* one event finished */
		nevents--;

	} while (got == 0 || (got < len - 2 &&
		(nevents > 0 ||
			(PeekConsoleInput(inConsole, &event, 1, &e) && e > 0))));
	SetConsoleMode(inConsole, mode);
	return got;
}


/* write characters out to the screen */
void ttywrite(buf, len)
	char		*buf;	/* buffer, holds characters to be written */
	int		len;	/* number of characters in buf */
{
 static int	   arg[9];	/* arguments to escape sequence */
 static int	   argno = -1;	/* -1 normally, else index of escape sequence argument */
 static COORD	   coord;	/* coordinates of cursor position */
 static ELVBOOL	   bgset = ElvFalse;/* has the background color been set? */
 static ELVBOOL	   boldset = ElvFalse;/* is the foreground brightness set? */
 static ELVBOOL	   soset = ElvFalse;/* is standout set? */
	SMALL_RECT rect;	/* source for inserting/deleting/scrolling */
	COORD	   dest;	/* destination for inserting/deletingi/scrolling */
	CHAR_INFO  ci;		/* fill info */
	DWORD	   qty;		/* number of characters to fill */
	CONSOLE_CURSOR_INFO cci;/* cursor shape */
	int	   i, j;
	long	   dummy;

	/* if not using a WIN32 console, then just write the characters to stdout */
	if (!useconsole || gui != &guitermcap)
	{
		_write(1, buf, len);
		return;
	}

	/* handle each character separately */
	for (i = 0; i < len; i++)
	{
		if (buf[i] == '\033')
		{
			/* start an escape sequence */
			for (j = 0; j < QTY(arg); j++)
				arg[j] = 0;
			argno = 0;
		}
		else if (console == outConsole && argno == -1)
		{
			/* most characters written literally to shell console */
			for (j = 1; i + j < len && buf[i + j] != '\033'; j++)
			{
			}
			SetConsoleCursorPosition(console, coord);
			WriteConsole(console, &buf[i], j, &dummy, NULL);
			i += j - 1; /* "- 1" because of "i++" at top of loop */
		}
		else if (buf[i] == '\007')
		{
			/* ring the bell */
			MessageBeep(MB_OK);
		}
		else if (buf[i] == '\b')
		{
			if (coord.X > 0)
				coord.X--;
			else
			{
				coord.X = consinfo.dwSize.X - 1;
				if (coord.Y > 0)
					coord.Y--;
			}
		}
		else if (buf[i] == '\n')
		{
			if (coord.Y + 1 < consinfo.dwSize.Y)
			{
				/* move cursor down on line */
				coord.Y++;
			}
			else
			{
				/* scroll screen upward one line */
				rect.Top = 1;
				rect.Left = 0;
				rect.Bottom = consinfo.dwSize.Y;
				rect.Right = consinfo.dwSize.X;
				dest.X = 0;
				dest.Y = 0;
				ci.Char.AsciiChar = ' ';
				ci.Attributes = attr;
				ScrollConsoleScreenBuffer(console, &rect, NULL, dest, &ci);
			}
		}
		else if (buf[i] == '\r')
		{
			coord.X = 0;
		}
		else if ((unsigned)buf[i] < ' ')
		{
			/* other control characters have no effect */
		}
		else if (argno < 0)
		{
			/* normal character */

			/* count consecutive normal characters */
			for (j = 1; i + j < len && buf[i + j] >= ' '; j++)
			{
			}

			/* write the normal characters all at once. */
#if 0
			SetConsoleCursorPosition(console, coord);
			if (console == myConsole)
				SetConsoleTextAttribute(console, myattr);
			WriteConsole(console, &buf[i], j, &dummy, NULL);
#else
			if (console == myConsole)
				FillConsoleOutputAttribute(myConsole, myattr,
					j, coord, &dummy);
			WriteConsoleOutputCharacter(console, &buf[i],
				j, coord, &dummy);
#endif

			/* move "i" past all but the last character.  The "i++"
			 * in the for() loop will take care of that last one.
			 */
			i += j - 1;

			/* figure out where the cursor belongs after that. */
#ifdef SMART_LINE_WRAP
			while (coord.X + j > consinfo.dwSize.X)
			{
				coord.Y++;
				j -= consinfo.dwSize.X;
			}
			coord.X += j;
			if (coord.X == consinfo.dwSize.X)
				coord.X--;
#else /* dumb line wrap */
			while (coord.X + j >= consinfo.dwSize.X)
			{
				coord.Y++;
				j -= consinfo.dwSize.X;
			}
			coord.X += j;
#endif

			/* check for scrolling */
			j = coord.Y - consinfo.dwSize.Y + 1;
			if (j > 0)
			{
				/* scroll screen upward "j" lines */
				rect.Top = j;
				rect.Left = 0;
				rect.Bottom = consinfo.dwSize.Y;
				rect.Right = consinfo.dwSize.X;
				dest.X = 0;
				dest.Y = 0;
				ci.Char.AsciiChar = ' ';
				ci.Attributes = attr;
				ScrollConsoleScreenBuffer(console, &rect, NULL, dest, &ci);

				/* this leaves the cursor on the last row */
				coord.Y = consinfo.dwSize.Y - 1;
			}
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

			  case 'A':
				j = (arg[0] ? arg[0] : 1);
				if (coord.Y >= j)
					coord.Y -= j;
				argno = -1;
				break;

			  case 'B':
				j = (arg[0] ? arg[0] : 1);
				if (coord.Y + j < consinfo.dwSize.Y)
					coord.Y += j;
				argno = -1;
				break;

			  case 'C':
				j = (arg[0] ? arg[0] : 1);
				if (coord.X + j < consinfo.dwSize.X)
					coord.X += j;
				argno = -1;
				break;

			  case 'D':
				j = (arg[0] ? arg[0] : 1);
				if (coord.X >= j)
					coord.X -= j;
				argno = -1;
				break;

			  case 'H':
				/* move the cursor */
				coord.X = (arg[1] ? arg[1] - 1 : 0);
				coord.Y = (arg[0] ? arg[0] - 1 : 0);
				argno = -1;
				break;

			  case 'J':
				/* clear the screen */
				qty = consinfo.dwSize.X - coord.X +
					consinfo.dwSize.X * (consinfo.dwSize.Y - 1 - coord.Y);
				if (qty > 0)
				{
					FillConsoleOutputCharacter(console, ' ', qty, coord, &dummy);
					FillConsoleOutputAttribute(console, attr, qty, coord, &dummy);
				}
				argno = -1;
				break;

			  case 'K':
				/* clear to end-of-line */
				qty = consinfo.dwSize.X - coord.X;
				if (qty > 0)
				{
					FillConsoleOutputCharacter(console, ' ', qty, coord, &dummy);
					if (console == myConsole)
					{
						FillConsoleOutputAttribute(console, attr, qty, coord, &dummy);
					}
				}
				argno = -1;
				break;

			  case 'L':
				/* insert j lines */
				j = arg[0] ? arg[0] : 1;
				rect.Top = coord.Y;
				rect.Left = 0;
				rect.Bottom = consinfo.dwSize.Y - j;
				rect.Right = consinfo.dwSize.X;
				dest.X = 0;
				dest.Y = coord.Y + j;
				ci.Char.AsciiChar = ' ';
				ci.Attributes = attr;
				ScrollConsoleScreenBuffer(console, &rect, NULL, dest, &ci);
				argno = -1;
				break;

			  case 'M':
				/* delete j lines */
				j = arg[0] ? arg[0] : 1;
				rect.Top = coord.Y + j;
				rect.Left = 0;
				rect.Bottom = consinfo.dwSize.Y;
				rect.Right = consinfo.dwSize.X;
				dest.X = 0;
				dest.Y = coord.Y;
				ci.Char.AsciiChar = ' ';
				ci.Attributes = attr;
				ScrollConsoleScreenBuffer(console, &rect, NULL, dest, &ci);
				argno = -1;
				break;

			  case 'h':
				if (arg[0] == 1)
				{
					switchcsbi(myConsole, &coord);
				}
				else if (arg[0] == 12)
				{
					cci.dwSize = 50;
					cci.bVisible = ElvTrue;
					SetConsoleCursorInfo(myConsole, &cci);
				}
				argno = -1;
				break;
				
			  case 'l':
				if (arg[0] == 1)
				{
					switchcsbi(outConsole, &coord);
				}
				else if (arg[0] == 12)
				{
					cci.dwSize = 20;
					cci.bVisible = ElvTrue;
					SetConsoleCursorInfo(myConsole, &cci);
				}
				argno = -1;
				break;

			  case 'm':
				for (j = 0; j <= argno; j++)
				{
					switch (arg[j])
					{
					  case 0:
						myattr = origattr;
						bgset = ElvFalse;
						boldset = ElvFalse;
						soset = ElvFalse;
						break;

					  case 1:
						if (myattr == origattr
						 && (myattr & FOREGROUND_INTENSITY) != 0)
							myattr |= FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
						else
							myattr |= FOREGROUND_INTENSITY;
						boldset = ElvTrue;
						break;

					  case 4:
						if (!bgset)
							myattr ^= BACKGROUND_RED;
						break;

					  case 7:
						if (!soset)
						{
							myattr ^= FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE
								| BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE;
							if (!boldset)
								myattr &= ~FOREGROUND_INTENSITY;
							soset = ElvTrue;
						}
						break;

					  case 30: 
					  case 31:
					  case 32:
					  case 33:
					  case 34:
					  case 35:
					  case 36:
					  case 37:
						myattr &= ~(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
						if (!boldset)
							myattr &= ~FOREGROUND_INTENSITY;
						if ((arg[j] - 30) & 1)
							myattr |= FOREGROUND_RED;
						if ((arg[j] - 30) & 2)
							myattr |= FOREGROUND_GREEN;
						if ((arg[j] - 30) & 4)
							myattr |= FOREGROUND_BLUE;
						if (soset)
							myattr ^= (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
						break;

					  case 39:
						myattr &= ~(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
						myattr |= origattr & (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
						boldset = (myattr & FOREGROUND_INTENSITY) != 0;
						break;

					  case 40:
					  case 41:
					  case 42:
					  case 43:
					  case 44:
					  case 45:
					  case 46:
					  case 47:
						myattr &= ~(BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE);
						if ((arg[j] - 40) & 1)
							myattr |= BACKGROUND_RED;
						if ((arg[j] - 40) & 2)
							myattr |= BACKGROUND_GREEN;
						if ((arg[j] - 40) & 4)
							myattr |= BACKGROUND_BLUE;
						if (soset)
							myattr ^= (BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE);
						bgset = ElvTrue;
						break;

					  case 49:
						myattr &= ~(BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE);
						myattr |= origattr & (BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE);
						break;

					}
				}

				/* note that we only set the attribute of
				 * myConsole
				 */
				if (console == myConsole)
				{
					attr = myattr;
				}
				argno = -1;
				break;

			  default:
				/* does it look like the end of an unrecognized command? */
				if ((buf[i] & 0x40) != 0)
				{
					argno = -1;
				}
			}
		}
	}
	SetConsoleCursorPosition(console, coord);
}


/* determine the terminal type */
char *ttytermtype()
{
	char	*type;

	type = getenv("TERM");
	if (!type)
		type = TTY_DEFAULT;

	/* are we using the console? */
	useconsole = !strcmp(type, "console") || !strcmp(type, "cygwin");
	if (useconsole)
	{
		inConsole = GetStdHandle(STD_INPUT_HANDLE);
		if (inConsole == INVALID_HANDLE_VALUE)
		{
			AllocConsole();
			inConsole = GetStdHandle(STD_INPUT_HANDLE);
		}
		outConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		assert(inConsole != INVALID_HANDLE_VALUE);
	}
	return type;
}


/* This function gets the window size. */
ELVBOOL ttysize(linesptr, colsptr)
	int	*linesptr;	/* where to store the number of rows */
	int	*colsptr;	/* where to store the number of columns */
{
	SMALL_RECT size;
	CONSOLE_SCREEN_BUFFER_INFO localconsinfo;

	if (!useconsole)
		return ElvFalse;

	/* Get the console buffer size */
	if (!GetConsoleScreenBufferInfo(myConsole, &localconsinfo))
		return ElvFalse;
#if 0
	prevHeight = *linesptr = localconsinfo.dwSize.Y;
	prevWidth = *colsptr = localconsinfo.dwSize.X;
#else
	*linesptr = localconsinfo.dwSize.Y;
	*colsptr = localconsinfo.dwSize.X;
#endif

	/* make the window as large as the console buffer */
	size.Top = 0;
	size.Left = 0;
	size.Bottom = *linesptr - 1;
	size.Right = *colsptr - 1;
	(void)SetConsoleWindowInfo(myConsole, TRUE, &size);
	return ElvTrue;
}


/* Check for signs of boredom from user, so we can abort a time-consuming
 * operation.  Here we check to see if SIGINT has been caught recently.
 * Returns ElvTrue to abort an operation, or ElvFalse to continue it.
 */
ELVBOOL ttypoll(reset)
	ELVBOOL reset;
{
	return (ELVBOOL)((ttycaught & (1 << SIGINT)) != 0);
}

/* This function resizes the terminal */
static void ttyresize(rows, cols)
	int	rows;
	int	cols;
{
	SMALL_RECT	rect;
	COORD		coord;
	BOOL		b;
	CONSOLE_SCREEN_BUFFER_INFO localconsinfo;

	if (!useconsole)
		return;

#if 0
	if (console != myConsole)
		SetConsoleActiveScreenBuffer(myConsole);
#endif

	coord = GetLargestConsoleWindowSize(myConsole);
	GetConsoleScreenBufferInfo(myConsole, &localconsinfo);
	rect.Left = 0;
	rect.Top = 0;
	if (rows > 0)
		rect.Bottom = min(coord.Y, rows) - 1;
	else
		rect.Bottom = localconsinfo.srWindow.Bottom - localconsinfo.srWindow.Top;
	if (cols > 0)
		rect.Right = min(coord.X, cols) - 1;
	else
		rect.Right = localconsinfo.srWindow.Right - localconsinfo.srWindow.Left;
	coord.X = rect.Right + 1;
	coord.Y = rect.Bottom + 1;
	b = SetConsoleWindowInfo(myConsole, TRUE, &rect);
	b = SetConsoleScreenBufferSize(myConsole, coord);
	if (curHeight != coord.Y)
		curHeight = coord.Y;
	if (curWidth != coord.X)
		curWidth = coord.X;
}


/* This function sets the new console title */
static void retitle(GUIWIN *gw, char *name)
{
	char	title[_MAX_PATH + 20];

	sprintf(title, "Elvis - [%s]", name);
	SetConsoleTitle(title);
}


/* This function opens the clipboard */
static ELVBOOL clipopen(ELVBOOL forwrite)
{
	BUFFER	buf = cutbuffer ('>', ElvFalse);

	/* check if something to do */
	if (!forwrite &&
	    !IsClipboardFormatAvailable (CF_TEXT) &&
	    !IsClipboardFormatAvailable (CF_OEMTEXT))
		return ElvFalse;

	if (forwrite && buf == NULL)
		return ElvFalse;

	/* open the clipboard */
	if (!OpenClipboard (GetActiveWindow ()))
		return ElvFalse;

	/* allocate memory if writing */
	if (forwrite) {
		clip_len = o_bufchars (buf) + o_buflines(buf) + 1;
		clip_hGlob = GlobalAlloc (GMEM_MOVEABLE | GMEM_DDESHARE,
						(DWORD)clip_len + 1);
		if (clip_hGlob == NULL)
			return ElvFalse;

		clip_data = (char *)GlobalLock (clip_hGlob);
		clip_offset = 0;
		EmptyClipboard ();
	}

	/* indicate success */
	return ElvTrue;
}

/* This function writes to the clipboard */
static int clipwrite(CHAR *text, int len)
{
	register char	*p = clip_data + clip_offset;
	register int	numchars = len;

	/* fill the allocated memory block */
	while (numchars-- > 0) {
		if (*text == '\n') {
			*p++ = '\r';
			clip_offset++;
		}
		*p++ = *text++;
		clip_offset++;
	}

	return len;
}

/* This function reads from the clipboard */
static int clipread(CHAR *text, int len)
{
	register char	*p;
	register int	numchars = 0;

	/* first time, rerieve memory block */
	if (clip_hGlob == NULL) {
		if ((clip_hGlob = GetClipboardData (CF_TEXT)) == NULL && 
			(clip_hGlob = GetClipboardData (CF_OEMTEXT)) == NULL)
			return 0;

		clip_data = (char *)GlobalLock (clip_hGlob);
		clip_len = strlen (clip_data) - 1;
		clip_offset = 0;
	}

	/* fill caller's data */
	for (p = clip_data + clip_offset; *p != '\0'; p++) {
		if (numchars == len)
			break;
		if (*p != '\r') {
			*text++ = *p;
			numchars++;
		}
		clip_offset++;
	}

	/* unlock global memory when done */
	if (numchars == 0) {
		GlobalUnlock (clip_hGlob);
		clip_hGlob = 0;
	}

	return numchars;
}

/* This function closes the clipboard */
static void clipclose(void)
{
	/* write to clipboard if writing */
	if (clip_hGlob != NULL) {
		clip_data[clip_offset] = '\0'; /* !!! */
		GlobalUnlock (clip_hGlob);
		SetClipboardData (CF_TEXT, clip_hGlob);
		clip_hGlob = NULL;
	}

	/* close the clipboard */
	CloseClipboard ();
}
#endif /* GUI_TERMCAP */
