/* oswin32/tcaphelp.c */

char id_tcaphelp[] = "$Id: tcaphelp.c,v 2.18 1996/09/18 20:37:17 steve Exp $";

#include "elvis.h"
#if defined(GUI_TERMCAP) || defined(GUI_OPEN)
# define CHAR CHAR_nt
# define BOOLEAN BOOLEAN_nt
# include <windows.h>
# include <fcntl.h>
# include <io.h>
# include <signal.h>
# undef CHAR
# undef BOOLEAN
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

/* This variable is used to indicate that the BIOS interface is being used */
static BOOLEAN	useconsole = True;
static HANDLE	inConsole, outConsole, myConsole, console;
static CONSOLE_SCREEN_BUFFER_INFO consinfo;
static DWORD	inMode, outMode;
static int	prevWidth, prevHeight;
static BOOLEAN	resized;
static WORD	origattr;
static WORD	attr;	/* attribute byte for writing subsequent text */

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

	/* change the new buffer's size to match the current buffer's window */
	prevWidth = size.X = consinfo.srWindow.Right - consinfo.srWindow.Left + 1;
	prevHeight = size.Y = consinfo.srWindow.Bottom - consinfo.srWindow.Top + 1;
	SetConsoleScreenBufferSize(myConsole, size);

	/* make the default colors of the new console buffer be the same as the
	 * old console buffer.
	 */
	attr = origattr = consinfo.wAttributes;
	SetConsoleTextAttribute(myConsole, attr);
	home.X = home.Y = 0;
	FillConsoleOutputAttribute(myConsole, attr, size.X * size.Y, home, &dummy);
	FillConsoleOutputCharacter(myConsole, ' ', size.X * size.Y, home, &dummy);
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
		newmode = ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT | ENABLE_WRAP_AT_EOL_OUTPUT;
		SetConsoleMode(inConsole, newmode);
		SetConsoleMode(outConsole, newmode|ENABLE_PROCESSED_OUTPUT);
		SetConsoleMode(myConsole, newmode);
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
static void switchcsbi(HANDLE which)
{
	/* if no change, then do nothing */
	if (which == console)
		return;

	/* switch to the requested console */
	SetConsoleActiveScreenBuffer(which);
	console = which;
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
	INPUT_RECORD event[5];/* buffer, holds an input record */
	DWORD	mode;	/* Console mode */
	DWORD	nevents;/* number of events read into event[] */
	int	e;	/* for counting through event[] */
	int	got;	/* character counter, for keystrokes */
static	DWORD	prevmb;	/* previous mouse button state */
	DWORD	press;	/* bitmap of new button presses */
	GUIWIN	*gw;	/* window where mouse event happened */
static	GUIWIN	*selgw;	/* window where selection is taking place */
	int	y, x;	/* coordinates of mouse within "gw" window */
static	BOOLEAN	justpressed;/* between a press and the start of a drag */
static	int	prevy, prevx;/* cell where originally pressed */
static	BOOLEAN	justdbl;/* between double-click & bogus single-click */

	/* reset the "ttycaught" variable */
	signal(SIGINT, catchsig);
	ttycaught = 0;

	/* if timeouts are allowed within 0.5 seconds, then return 0
	 * immediately unless there's already a keystroke waiting.
	 */
	if (timeout > 0
	 && timeout <= 5
	 && (!useconsole || (PeekConsoleInput(inConsole, event, 1, &nevents)
			     && nevents == 0)))
	{
		return 0;
	}

	/* if resized since last read, then return -1 to simulate the value
	 * that UNIX's read() call would return after a SIGWINCH.
	 */
	if (resized)
	{
		resized = False;
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
		/* remove the event from the input queue */
		if (nevents > 0)
		{
			for (e = 0; e < nevents; e++)
				event[e] = event[e + 1];
		}
		else if (!ReadConsoleInput(inConsole, event, QTY(event), &nevents))
		{
			/* How could ReadConsoleInput() fail? */
			return -1;
		}

		/* process the event */
		switch (event[0].EventType)
		{
		  case KEY_EVENT:
			if (event[0].Event.KeyEvent.bKeyDown)
			{
				for (x = event[0].Event.KeyEvent.wRepeatCount;
				     x > 0 && got + 2 < QTY(buf);
				     x--)
				{
					if (event[0].Event.KeyEvent.uChar.AsciiChar)
					{
						buf[got++] = event[0].Event.KeyEvent.uChar.AsciiChar;
					}
					else if (3 == (char)event[0].Event.KeyEvent.wVirtualScanCode)
					{
						/* Ctrl-2 should be a NUL character */
						buf[got++] = '\0';
					}
					else if (7 == (char)event[0].Event.KeyEvent.wVirtualScanCode)
					{
						/* Ctrl-6 should be a ^^ character */
						buf[got++] = ELVCTRL('^');
					}
					else if (!strchr("68*:\x1d", (char)event[0].Event.KeyEvent.wVirtualScanCode))
					{
						buf[got++] = '#';
						buf[got++] = (char)event[0].Event.KeyEvent.wVirtualScanCode;
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
				prevmb = event[0].Event.MouseEvent.dwButtonState;
				if (prevmb == 0)
					justdbl = False;
				break;
			}
			else if (event[0].Event.MouseEvent.dwButtonState == 0
				&& prevmb == 0)
			{
				break;
			}

			/* Figure out which window the event occurred in */
			gw = ttywindow(event[0].Event.MouseEvent.dwMousePosition.Y,
				       event[0].Event.MouseEvent.dwMousePosition.X,
				       &y, &x);
			if (!gw || (selgw && selgw != gw))
				break;
			press = (event[0].Event.MouseEvent.dwButtonState & ~prevmb);
			prevmb = event[0].Event.MouseEvent.dwButtonState;

			/* Make the window become the current window */
			(*gui->focusgw)(gw);
			eventfocus(gw);

			/* process the event */
			if (event[0].Event.MouseEvent.dwEventFlags & DOUBLE_CLICK)
			{
				/* DOUBLE CLICK */
				if (event[0].Event.MouseEvent.dwButtonState & 1)
					eventclick(gw, y, x, CLICK_TAG);
				else
					eventclick(gw, y, x, CLICK_UNTAG);
				SetConsoleMode(inConsole, mode);
				selgw = NULL;
				justdbl = True;
				return -2;
			}
			if ((event[0].Event.MouseEvent.dwEventFlags & MOUSE_MOVED) != 0 && justpressed && (x != prevx || y != prevy))
			{
				/* starting a draw-through */
				switch (event[0].Event.MouseEvent.dwButtonState)
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
				justpressed = False;
			}
			else if (press != 0)
			{
				/* button was pressed */
				if (press & 1)
					eventclick(gw, y, x, CLICK_CANCEL);
				justpressed = True;
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
			(PeekConsoleInput(inConsole, event, 1, &e) && e > 0))));
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
 static BOOLEAN	   bgset = False;/* has the background color been set? */
 static BOOLEAN	   boldset = False;/* has the foreground brightness been set? */
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
			for (j = 1; j < len && buf[i + j] != '\033'; j++)
			{
			}
			WriteConsole(console, &buf[i], j, &dummy, NULL);
			i += j - 1; /* "- 1" because of "i++" at top of loop */
		}
		else if (buf[i] == '\007')
		{
			/* write the bell character */
			WriteConsole(console, &buf[i], 1, &dummy, NULL);
		}
		else if (buf[i] == '\b')
		{
			coord.X--;
			if (coord.X < 0)
			{
				coord.X += consinfo.dwSize.X;
				if (coord.Y > 0)
					coord.Y--;
			}
			SetConsoleCursorPosition(console, coord);
		}
		else if (buf[i] == '\n')
		{
			if (coord.Y + 1 < consinfo.dwSize.Y)
			{
				/* move cursor down on line */
				coord.Y++;
				SetConsoleCursorPosition(console, coord);
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
			SetConsoleCursorPosition(console, coord);
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

			/* write the normal characters all at once. Note
			 * that the attribute is set on myConsole, regardless
			 * of which console buffer is currently active.
			 */
			SetConsoleCursorPosition(console, coord);
			SetConsoleTextAttribute(myConsole, attr);
			WriteConsole(console, &buf[i], j, &dummy, NULL);

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
				ScrollConsoleScreenBuffer(outConsole, &rect, NULL, dest, &ci);

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
				SetConsoleCursorPosition(console, coord);
				argno = -1;
				break;

			  case 'B':
				j = (arg[0] ? arg[0] : 1);
				if (coord.Y + j < consinfo.dwSize.Y)
					coord.Y += j;
				SetConsoleCursorPosition(console, coord);
				argno = -1;
				break;

			  case 'C':
				j = (arg[0] ? arg[0] : 1);
				if (coord.X + j < consinfo.dwSize.X)
					coord.X += j;
				SetConsoleCursorPosition(console, coord);
				argno = -1;
				break;

			  case 'D':
				j = (arg[0] ? arg[0] : 1);
				if (coord.X >= j)
					coord.X -= j;
				SetConsoleCursorPosition(console, coord);
				argno = -1;
				break;

			  case 'H':
				/* move the cursor */
				coord.X = (arg[1] ? arg[1] - 1 : 0);
				coord.Y = (arg[0] ? arg[0] - 1 : 0);
				SetConsoleCursorPosition(console, coord);
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
					switchcsbi(myConsole);
				}
				else if (arg[0] == 12)
				{
					cci.dwSize = 50;
					cci.bVisible = True;
					SetConsoleCursorInfo(myConsole, &cci);
				}
				argno = -1;
				break;
				
			  case 'l':
				if (arg[0] == 1)
				{
					switchcsbi(outConsole);
				}
				else if (arg[0] == 12)
				{
					cci.dwSize = 20;
					cci.bVisible = True;
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
						attr = origattr;
						bgset = False;
						boldset = False;
						break;

					  case 1:
						if (attr == origattr
						 && (attr & FOREGROUND_INTENSITY) != 0)
							attr |= FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
						else
							attr |= FOREGROUND_INTENSITY;
						boldset = True;
						break;

					  case 4:
						if (!bgset)
							attr ^= BACKGROUND_RED;
						break;

					  case 7:
						attr ^= FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE
							| BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE;
						if (!boldset)
							attr &= ~FOREGROUND_INTENSITY;
						break;

					  case 30: 
					  case 31:
					  case 32:
					  case 33:
					  case 34:
					  case 35:
					  case 36:
					  case 37:
						attr &= ~(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
						if (!boldset)
							attr &= ~FOREGROUND_INTENSITY;
						if ((arg[j] - 30) & 1)
							attr |= FOREGROUND_RED;
						if ((arg[j] - 30) & 2)
							attr |= FOREGROUND_GREEN;
						if ((arg[j] - 30) & 4)
							attr |= FOREGROUND_BLUE;
						break;

					  case 40:
					  case 41:
					  case 42:
					  case 43:
					  case 44:
					  case 45:
					  case 46:
					  case 47:
						attr &= ~(BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE);
						if ((arg[j] - 40) & 1)
							attr |= BACKGROUND_RED;
						if ((arg[j] - 40) & 2)
							attr |= BACKGROUND_GREEN;
						if ((arg[j] - 40) & 4)
							attr |= BACKGROUND_BLUE;
						bgset = True;
						break;
					}
				}

				/* note that we always set the attribute of
				 * myConsole, regardless of which console buffer
				 * is currently active.
				 */
				SetConsoleTextAttribute(myConsole, attr);
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

}


/* determine the terminal type */
char *ttytermtype()
{
	char	*type;

	type = getenv("TERM");
	if (!type)
		type = TTY_DEFAULT;

	/* are we using the console? */
	useconsole = !strcmp(type, "console");
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
BOOLEAN ttysize(linesptr, colsptr)
	int	*linesptr;	/* where to store the number of rows */
	int	*colsptr;	/* where to store the number of columns */
{
	SMALL_RECT size;

	if (!useconsole)
		return False;

	/* Get the console buffer size */
	if (!GetConsoleScreenBufferInfo(myConsole, &consinfo))
		return False;
	prevHeight = *linesptr = consinfo.dwSize.Y;
	prevWidth = *colsptr = consinfo.dwSize.X;

	/* make the window as large as the console buffer */
	size.Top = 0;
	size.Left = 0;
	size.Bottom = *linesptr - 1;
	size.Right = *colsptr - 1;
	(void)SetConsoleWindowInfo(myConsole, TRUE, &size);
	return True;
}


/* Check for signs of boredom from user, so we can abort a time-consuming
 * operation.  Here we check to see if SIGINT has been caught recently.
 * Returns True to abort an operation, or False to continue it.
 */
BOOLEAN ttypoll(reset)
	BOOLEAN reset;
{
	return (BOOLEAN)((ttycaught & (1 << SIGINT)) != 0);
}
#endif /* GUI_TERMCAP */
