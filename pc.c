/* pc.c */

/* Author:
 *	Guntram Blohm
 *	Buchenstrasse 19
 *	7904 Erbach, West Germany
 *	Tel. ++49-7305-6997
 *	sorry - no regular network connection
 */

/* This file implements the ibm pc bios interface. See IBM documentation
 * for details.
 * If TERM is set upon invocation of elvis, this code is ignored completely,
 * and the standard termcap functions are used, thus, even not-so-close
 * compatibles can run elvis. For close compatibles however, bios output
 * is much faster (and permits reverse scrolling, adding and deleting lines,
 * and much more ansi.sys isn't capable of). GB.
 */

/* The routines for setting raw mode were written by Dan Kegel.  They were
 * taken [with slight modifications] from the "raw.c" file that he distributes
 * with his NANSI.SYS console driver.  Email: dank@moc.jpl.nasa.gov
 */

#include "config.h"
#include "vi.h"

#if MSDOS

#include <dos.h>

#if TURBOC
#include <conio.h>
#endif

static void video P_((int, int *, int *));

/* vmode contains the screen attribute index and is set by attrset.*/

int vmode;

/* The following array contains attribute definitions for
 * color/monochrome attributes. Screen selects one of the sets.
 * Maybe i'll put them into elvis options one day.
 */

static int screen;
static char attr[2][8] =
{
    /* :se:  :so:  :VB:  quit  :ul:  :as:  popup visible */
    {  0x1f, 0x1d, 0x1e, 0x07, 0x1a, 0x1c, 0x2f, 0x3f}, /* color */
    {  0x07, 0x70, 0x0f, 0x07, 0x01, 0x0f, 0x70, 0x70}, /* mono */
};

/*
 * bios interface functions for elvis - pc version
 */

int biosquit()
{
	int	cx = 1;

	vmode = A_QUIT;
	v_ce();
	return TRUE;
}


/* This function changes the table of attribute bytes used during BIOS output.
 */
int bioscolor(mode, attrbyte)
	int	mode;	/* e.g. A_NORMAL */
	int	attrbyte;/* color code, as a PC attribute byte */
{
	attr[0][mode] = attrbyte;
	return 0;
}

/* IOCTL GETBITS/SETBITS bits. */
#define DEVICE		0x80
#define RAW		0x20

/* IOCTL operations */
#define GETBITS		0
#define SETBITS		1
#define GETINSTATUS	6

/* DOS function numbers. */
#define BREAKCHECK	0x33
#define IOCTL		0x44

/* A nice way to call the DOS IOCTL function */
static int
elvis_ioctl(int handle, int mode, unsigned setvalue)
{
	union REGS regs;

	regs.h.ah = IOCTL;
	regs.h.al = (char) mode;
	regs.x.bx = handle;
	regs.h.dl = (char) setvalue;
	regs.h.dh = 0;			/* Zero out dh */
	intdos(&regs, &regs);
	return (regs.x.dx);
}


/*--------------------------------------------------------------------------
 Call this routine to set or clear RAW mode for the device associated with
 the given file.
 Example: raw_set(1, TRUE);
--------------------------------------------------------------------------*/
void raw_set(fd, rawstate)
	int fd;
	int rawstate;
{
	int bits;
	bits = elvis_ioctl(fd, GETBITS, 0);
	if (DEVICE & bits) {
		if (rawstate)
			bits |= RAW;
		else
			bits &= ~RAW;
		(void) elvis_ioctl(fd, SETBITS, bits);
	}
}

/* A nice way to call the DOS BREAKCHECK function */
static int breakctl(int mode, int setvalue)
{
	union REGS regs;

	regs.h.ah = BREAKCHECK;
	regs.h.al = (char) mode;
	regs.h.dl = (char) setvalue;
	intdos(&regs, &regs);
	return (regs.x.dx & 0xff);
}

/*--------------------------------------------------------------------------
 Call this routine to determine whether DOS is checking for break (Control-C)
 before it executes any DOS function call.
 Return value is FALSE if it only checks before console I/O function calls,
 TRUE if it checks before any function call.
--------------------------------------------------------------------------*/
int break_get(void)
{
	return ( 0 != breakctl(GETBITS, 0));
}

/*--------------------------------------------------------------------------
 Call this routine with TRUE to tell DOS to check for break (Control-C)
 before it executes any DOS function call.
 Call this routine with FALSE to tell DOS to only check for break before
 it executes console I/O function calls.
--------------------------------------------------------------------------*/
void break_set(check)
	int check;
{
	(void) breakctl(SETBITS, check);
}

/*--------------------------------------------------------------------------
 One routine to set (or clear) raw mode on stdin and stdout,
 clear (or restore) break checking, and turn off input buffering on stdin.
 This is the most common configuration; under MS-DOS, since setting raw mode
 on stdout sometimes sets it on stdin, it's best to set it on both & be done
 with it.
--------------------------------------------------------------------------*/
void raw_set_stdio(rawstate)
	int rawstate;	/* TRUE -> set raw mode; FALSE -> clear raw mode */
{
	static int was_break_checking = 0;

	raw_set(0, rawstate);
	raw_set(1, rawstate);
	if (rawstate) {
		was_break_checking = break_get();
		break_set(0);
	} else {
		break_set(was_break_checking);
	}
}






/* cursor up: determine current position, decrement row, set position */

void v_up()
{
	int dx;
	video(0x300,(int *)0,&dx);
	dx-=0x100;
	video(0x200,(int *)0,&dx);
}

#ifndef NO_CURSORSHAPE
/* cursor big: set begin scan to end scan - 4 */
void v_cb()
{
	int cx;
	video(0x300, &cx, (int *)0);
	cx=((cx&0xff)|(((cx&0xff)-4)<<8));
	video(0x100, &cx, (int *)0);
}

/* cursor small: set begin scan to end scan - 1 */
void v_cs()
{
	int cx;
	video(0x300, &cx, (int *)0);
	cx=((cx&0xff)|(((cx&0xff)-1)<<8));
	video(0x100, &cx, (int *)0);
}
#endif

/* clear to end: get cursor position and emit the aproppriate number
 * of spaces, without moving cursor.
 */
 
void v_ce()
{
	int cx, dx;
	video(0x300,(int *)0,&dx);
	cx=COLS-(dx&0xff);
	video(0x920,&cx,(int *)0);
}

/* clear screen: clear all and set cursor home */

void v_cl()
{
	int cx=0, dx=((LINES-1)<<8)+COLS-1;
	video(0x0600,&cx,&dx);
	dx=0;
	video(0x0200,&cx,&dx);
}

/* clear to bottom: get position, clear to eol, clear next line to end */

void v_cd()
{
	int cx, dx, dxtmp;
	video(0x0300,(int *)0,&dx);
	dxtmp=(dx&0xff00)|(COLS-1);
	cx=dx;
	video(0x0600,&cx,&dxtmp);
	cx=(dx&0xff00)+0x100;
	dx=((LINES-1)<<8)+COLS-1;
	video(0x600,&cx,&dx);
}

/* add line: scroll rest of screen down */

void v_al()
{
	int cx,dx;
	video(0x0300,(int *)0,&dx);
	cx=(dx&0xff00);
	dx=((LINES-1)<<8)+COLS-1;
	video(0x701,&cx,&dx);
}

/* delete line: scroll rest up */

void v_dl()
{
	int cx,dx;
	video(0x0300,(int *)0,&dx);
	cx=(dx&0xff00)/*+0x100*/;
	dx=((LINES-1)<<8)+COLS-1;
	video(0x601,&cx,&dx);
}

/* scroll reverse: scroll whole screen */

void v_sr()
{
	int cx=0, dx=((LINES-1)<<8)+COLS-1;
	video(0x0701,&cx,&dx);
}

/* set cursor */

void v_move(x,y)
	int x, y;
{
	int dx=(y<<8)+x;
	video(0x200,(int *)0,&dx);
}

/* put character: set attribute first, then execute char.
 * Also remember if current line has changed.
 */

int v_put(ch)
	int ch;
{
	int cx=1;
	ch&=0xff;
	if (ch>=' ')
		video(0x900|ch,&cx,(int *)0);
	video(0xe00|ch,(int *)0, (int *)0);
	if (ch=='\n')
	{	exwrote = TRUE;
		video(0xe0d, (int *)0, (int *)0);
	}
	return ch;
}

/* determine number of screen columns. Also set attrset according
 * to monochrome/color screen.
 */

int v_cols()
{
	union REGS regs;
	regs.h.ah=0x0f;
	int86(0x10, &regs, &regs);
	if (regs.h.al==7)			/* monochrome mode ? */
		screen=1;
	else
		screen=0;
	return regs.h.ah;
}

/* Getting the number of rows is hard. Most screens support 25 only,
 * EGA/VGA also support 43/50 lines, and some OEM's even more.
 * Unfortunately, there is no standard bios variable for the number
 * of lines, and the bios screen memory size is always rounded up
 * to 0x1000. So, we'll really have to cheat.
 * When using the screen memory size, keep in mind that each character
 * byte has an associated attribute byte.
 *
 * uses:	word at 40:4c contains	memory size
 *		byte at 40:84 		# of rows-1 (sometimes)
 *		byte at	40:4a		# of columns
 */

int v_rows()
{
	int line, oldline;

	/* screen size less then 4K? then we have 25 lines only */

	if (*(int far *)(0x0040004cl)<=4096)
		return 25;

	/* VEGA vga uses the bios byte at 0x40:0x84 for # of rows.
	 * Use that byte, if it makes sense together with memory size.
	 */

	if ((((*(unsigned char far *)(0x0040004aL)*2*
		(*(unsigned char far *)(0x00400084L)+1))+0xfff)&(~0xfff))==
		*(unsigned int far *)(0x0040004cL))
			return *(unsigned char far *)(0x00400084L)+1;

	/* uh oh. Emit '\n's until screen starts scrolling. */

	v_move(oldline=0, 0);
	for (;;)
	{
		video(0xe0a,(int *)0,(int *)0);
		video(0x300,(int *)0,&line);
		line>>=8;
		if (oldline==line)
			return line+1;
		oldline=line;	
	}
}

/* the REAL bios interface -- used internally only. */

static void video(ax, cx, dx)
	int ax, *cx, *dx;
{
	union REGS regs;

	regs.x.ax=ax;
	if ((ax&0xff00)==0x600 || (ax&0xff00)==0x700)
		regs.h.bh=attr[screen][vmode];
	else
	{
		regs.h.bh=0;
		regs.h.bl=attr[screen][vmode];
	}
	if (cx) regs.x.cx=*cx;
	if (dx) regs.x.dx=*dx;
	int86(0x10, &regs, &regs);
	if (dx) *dx=regs.x.dx;
	if (cx) *cx=regs.x.cx;
}

/* The following function determines which character is used for
 * commandline-options by command.com. This system call is undocumented
 * and valid for versions < 4.00 only.
 */
 
int switchar()
{
	union REGS regs;
	regs.x.ax=0x3700;
	int86(0x21, &regs, &regs);
	return regs.h.dl;
}


/* this function returns the DOS time, as a 32-bit long int representing
 * hundredths of a second since midnight.  Some systems may be limited to
 * a resolution of whole seconds, but the values will still represent
 * hundredths.
 */
static long dostime P_((void))
{
	union REGS	regs;

	regs.h.ah = 0x2c;	/* MS-DOS "get time" service */
	intdos(&regs, &regs);
	return (((regs.h.ch * 60L) + regs.h.cl) * 60L + regs.h.dh) * 100L + regs.h.dl;
}


/*ARGSUSED*/
/* This function implements a raw read from the keyboard, with timeout. */
int ttyread(buf, len, time)
	char	*buf;	/* where to store the keystrokes */
	int	len;	/* maximum number of characters to get -- ignored */
	int	time;	/* maximum time to wait, in 1/9th second increments */
{
	long	stop;

	/* are we going to timeout? */
	if (time != 0)
	{
		/* compute the time when we'll give up */
		stop = dostime() + time * 10L;

		/* wait for either keystroke or timeout */
		while (!kbhit())
		{
			if (dostime() > stop)
			{
				/* we couldn't read any characters
				 * before timeout
				 */
				return 0;
			}
		}
	}

	/* get a keystroke */
	buf[0] = getch();
	if (buf[0] == 0) /* function key? */
	{
		buf[0] = '#';
		buf[1] = getch();
		return 2;
	}
	else
	{
		return 1;
	}
}

#if !TURBOC 
/* Turboc provides sleep, declared as void sleep(unsigned seconds) */
int sleep(seconds)
	unsigned seconds;
{
	long	stop;

	stop = dostime() + 100L * seconds;
	while (dostime() < stop)
	{
	}
	return 0;
}
#endif
#endif
