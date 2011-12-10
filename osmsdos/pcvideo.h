/* pcvideo.h */

/* This file contains functions for a BIOS video interface.  These functions
 * were originally written by Guntram Blohm and Martin Patzel, for elvis 1.x
 * in the "pc.c" file.  Most of them were moved to this file without any
 * modifications, but some of the color functions were modified by Steve
 * Kirkendall.
 * 
 * Original Author:
 *	Guntram Blohm
 *	Buchenstrasse 19
 *	7904 Erbach, West Germany
 *	Tel. ++49-7305-6997
 *	sorry - no regular network connection
 *
 * The functions are:
 *	v_up()			Move the cursor up one line
 *	v_cb()			Make the cursor big
 *	v_cs()			Make the cursor small
 *	v_ce()			Clear to end-of-line
 *	v_cl()			Clear screen and move cursor to home
 *	v_cd()			Clear to end-of-screen
 *	v_al(n)			Insert n rows
 *	v_dl(n)			Delete n rows
 *	v_sr()			Reverse scroll
 *	v_move(x,y)		Move cursor to column x,y
 *	v_attr(mask, bits)	Set the character attribute byte
 *	v_put(ch)		Write a single character to display
 *	v_cols()		Return the number of column on the display
 *	v_rows()		Return the number of rows on the display
 */

#ifdef __TURBOC__
# include <conio.h>
# include <dos.h>
#else
# ifndef REGS
#  ifndef GO32
#   define REGS	_REGS
#  endif
# endif
#endif

BEGIN_EXTERNC
#if USE_PROTOTYPES
static void v_up(void);
static void v_cb(void);
static void v_cs(void);
static void v_ce(void);
static void v_cl(void);
static void v_cd(void);
static void v_al(int n);
static void v_dl(int n);
static void v_sr(void);
static void v_move(int x, int y);
static void v_attr(int mask, int bits);
static int v_put(int ch);
static int v_cols(void);
static int v_rows(void);
#endif

static void video P_((int, int *, int *));
END_EXTERNC

static ELVBOOL vmono;	/* is this a monochrome screen? */
static int vmode = 0x07;/* attribute byte */
static int vcols;	/* columns per row */
static int vrows;	/* rows on screen */

#if 0
/* cursor up: determine current position, decrement row, set position */
void v_up()
{
	int dx;

	video(0x300,(int *)0,&dx);
	dx -= 0x100;
	video(0x200,(int *)0,&dx);
}
#endif

/* cursor big: set begin scan to end scan - 4 */
void v_cb()
{
	int cx;

	video(0x300, &cx, (int *)0);
	cx = ((cx & 0xff) | (((cx & 0xff) - 4) << 8));
	video(0x100, &cx, (int *)0);
}

/* cursor small: set begin scan to end scan - 1 */
void v_cs()
{
	int cx;

	video(0x300, &cx, (int *)0);
	cx = ((cx & 0xff) | (((cx & 0xff) - 1) << 8));
	video(0x100, &cx, (int *)0);
}

/* clear to end: get cursor position and emit the aproppriate number
 * of spaces, without moving cursor.
 */
void v_ce()
{
	int cx, dx;

	video(0x300, (int *)0, &dx);
	cx = vcols - (dx & 0xff);
	video(0x920, &cx, (int *)0);
}

/* clear screen: clear all and set cursor home */
void v_cl()
{
	int cx = 0, dx = ((vrows - 1) << 8) + vcols - 1;

	video(0x0600, &cx, &dx);
	dx = 0;
	video(0x0200, &cx, &dx);
}

#if 0
/* clear to bottom: get position, clear to eol, clear next line to end */
void v_cd()
{
	int cx, dx, dxtmp;

	video(0x0300, (int *)0, &dx);
	dxtmp = (dx & 0xff00) | (vcols - 1);
	cx = dx;
	video(0x0600, &cx, &dxtmp);
	cx = (dx & 0xff00) + 0x100;
	dx = ((vrows-1)<<8) + vcols - 1;
	video(0x600, &cx, &dx);
}
#endif

/* add line: scroll rest of screen down */
void v_al(n)
	int	n;
{
	int cx,dx;

	video(0x0300, (int *)0, &dx);
	cx = (dx & 0xff00);
	dx = ((vrows - 1) << 8) + vcols - 1;
	video(0x700 + n, &cx, &dx);
}

/* delete line: scroll rest up */
void v_dl(n)
	int	n;
{
	int cx, dx;

	video(0x0300, (int *)0, &dx);
	cx = (dx & 0xff00)/*+0x100*/;
	dx = ((vrows - 1) << 8) + vcols - 1;
	video(0x600 + n, &cx, &dx);
}

/* set cursor */
void v_move(x,y)
	int x, y;
{
	int dx = (y<<8)+x;

	video(0x200, (int *)0, &dx);
}

/* set the attribute byte */
void v_attr(mask, bits)
	int mask, bits;
{
	if (vmono)
	{
		bits >>= 8;
		mask >>= 8;
	}
	vmode = ((vmode & ~mask) ^ bits);
}

/* put character: set attribute first, then execute char.
 * Also remember if current line has changed.
 */
int v_put(ch)
	int ch;
{
	int cx = 1;

	ch &= 0xff;
	if (ch >= ' ')
		video(0x900 | ch, &cx, (int *)0);
	return ch;
}

/* determine number of screen columns. Also set vmono according
 * to monochrome/color screen.
 */
int v_cols()
{
	union REGS regs;

	regs.h.ah = 0x0f;
	int86(0x10, &regs, &regs);
	vmono = (ELVBOOL)(regs.h.al == 7);
	return (vcols = regs.h.ah);	/* yes, ASSIGNMENT! */
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

#ifndef GO32
	/* screen size less then 4K? then we have 25 lines only */

	if (*(int far *)(0x0040004cl)<=4096)
		return (vrows = 25);	/* yes, ASSIGNMENT! */

	/* VEGA vga uses the bios byte at 0x40:0x84 for # of rows.
	 * Use that byte, if it makes sense together with memory size.
	 */

	if ((((*(unsigned char far *)(0x0040004aL) * 2 *
		(*(unsigned char far *)(0x00400084L)+1))+0xfff)&(~0xfff)) ==
		*(unsigned int far *)(0x0040004cL))
			return (vrows = *(unsigned char far *)(0x00400084L)+1);
#endif

	/* uh oh. Emit '\n's until screen starts scrolling. */

	v_move(oldline = 0, 0);
	for (;;)
	{
		video(0xe0a, (int *)0, (int *)0);
		video(0x300, (int *)0, &line);
		line >>= 8;
		if (oldline == line)
			return (vrows = line+1);
		oldline = line;	
	}
}

/* the REAL bios interface -- used internally only. */
static void video(ax, cx, dx)
	int ax, *cx, *dx;
{
	union REGS regs;

	regs.x.ax = ax;
	if ((ax&0xff00) == 0x600 || (ax&0xff00) == 0x700)
		regs.h.bh = vmode;
	else
	{
		regs.h.bh = 0;
		regs.h.bl = vmode;
	}
	if (cx) regs.x.cx = *cx;
	if (dx) regs.x.dx = *dx;
	int86(0x10, &regs, &regs);
	if (dx) *dx = regs.x.dx;
	if (cx) *cx = regs.x.cx;
}
