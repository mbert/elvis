/* xstatus.c */

/* Copyright 1997 by Steve Kirkendall */

char id_xstatus[] = "$Id: xstatus.c,v 2.7 1997/12/23 19:25:46 steve Exp $";

#include "elvis.h"
#ifdef GUI_X11
# include "guix11.h"

#define ST_BEVEL	1	/* bevel width for panes */
#define ST_ADJACENT	2	/* gap for adjacent panes */

#if USE_PROTOTYPES
static int draw1pane(X11WIN *xw, int pos, char *text, unsigned int *wideptr);
#endif


static int	statusheight;	/* height of a pane, including bevel */
static int	statusbase;	/* vertical offset of label within pane */

/* This function draws a single pane at the right-hand edge of the status bar
 * The width of the pane is computed automatically, and the pane's horizontal
 * offset is returned.  The intent is that you'll first set "pos" to the width
 * of the toolbar, and then iterate through all panes, setting "pos" to the
 * value returned by this function.  Note that you'll need to clear the
 * statusbar window before doing this.
 */
static int draw1pane(xw, pos, text, wideptr)
	X11WIN		*xw;	/* a top-level app window */
	int		pos;	/* horizontal offset of the previous pane */
	char		*text;	/* the pane's text */
	unsigned int	*wideptr;/* widest this pane has ever been */
{
	XCharStruct	size;
	unsigned int	width;	/* width of pane */
	unsigned int	textx;	/* where to draw text */
	int		dummy;


	/* Compute the width of the pane's text */
	XTextExtents(x_loadedcontrol->fontinfo, text, strlen(text),
		&dummy, &dummy, &dummy, &size);
	textx = ST_BEVEL + 1 - size.lbearing;
		/* Note: size.lbearing is non-positive number,
		 * so it actually increases to the scan->textx
		 * and scan->width values.
		 */
	width = size.rbearing - size.lbearing + 2 * ST_BEVEL + 2 + ST_ADJACENT;
	if (width > *wideptr)
		*wideptr = width;
	else
		width = *wideptr;

	/* if the label is narrower than the pane, then divide the excess
	 * in half, so the label will be centered.
	 */
	textx += (*wideptr - size.rbearing + size.lbearing - 2 * ST_BEVEL - 2 - ST_ADJACENT) / 2;

	/* draw the pane's bevel and face */
	pos -= width;
	x_drawbevel(xw, xw->st.win, pos, 2, width - ST_ADJACENT, statusheight, 'b', -ST_BEVEL);

	/* draw the pane's label */
	XSetForeground(x_display, xw->gc, xw->st.fglabel);
	XSetFont(x_display, xw->gc, x_loadedcontrol->fontinfo->fid);
	XDrawString(x_display, xw->st.win, xw->gc,
		pos + textx, 2 + statusbase, text, strlen(text));
	xw->fg = xw->st.fglabel;

	/* return the position */
	return pos;
}

/* This function predicts the height of a statusbar window of a given width.  */
void x_st_predict(xw, w, h)
	X11WIN	 *xw;
	unsigned w, h;	/* width of the statusbar (height is ignored) */
{
	/* allocate the colors */
	xw->st.bg = x_loadcolor(x_scrollbarbg, x_black);
	xw->st.fgface = x_loadcolor(x_toolbg, x_white);
	xw->st.fglabel = x_loadcolor(x_toolfg, x_black);

	/* if "set nostatusbar", then statusbar height is always 0 */
	if (!o_statusbar)
	{
		xw->st.h = 0;
		xw->st.w = w;
		xw->st.win = None;
		return;
	}

	/* compute some info about the font */
	statusheight = x_loadedcontrol->height + 2 * ST_BEVEL;
	statusbase = x_loadedcontrol->fontinfo->max_bounds.ascent + ST_BEVEL;

	/* compute the window size */
	xw->st.w = w;
	xw->st.h = statusheight + 3;
}


void x_st_create(xw, x, y)
	X11WIN	 *xw;	/* top-level application window */
	int	 x, y;	/* position of the statusbar within the top-level window */
{
	if (xw->st.h == 0)
		return;

	/* create the window */
	xw->st.win = XCreateSimpleWindow(x_display, xw->win,
		x, y, xw->st.w, xw->st.h, 0, xw->st.fglabel, xw->st.bg);
	if (x_mono || xw->st.fglabel == xw->st.bg)
		XSetWindowBackgroundPixmap(x_display, xw->st.win, x_gray);
	XSelectInput(x_display, xw->st.win, ExposureMask);

	/* remember its size and position */
	xw->st.x = x;
	xw->st.y = y;

	/* set "mode" to a safe value */
	xw->st.mode = "   ";
	xw->st.modepos = xw->st.learnpos = xw->st.rulerpos = 0;
}

/* Destroy the statusbar for a window */
void x_st_destroy(xw)
	X11WIN	*xw;	/* top-level application window to loose the statusbar */
{
	/* free the colors */
	x_unloadcolor(xw->st.bg);
	x_unloadcolor(xw->st.fgface);
	x_unloadcolor(xw->st.fglabel);

	/* destroy the window */
	if (xw->st.win != None)
		XDestroyWindow(x_display, xw->st.win);
	xw->st.win = None;
}


/* Change all fields of the status bar */
void x_st_status(xw, info, line, column, learn, mode)
	X11WIN	*xw;	/* window whose status should change */
	CHAR	*info;	/* info field, or NULL to blank it out */
	long	line;	/* line number */
	long	column;	/* column number */
	_char_	learn;	/* learn letter, or '*' if modified, or ',' otherwise */
	char	*mode;	/* name of command mode, e.g., "Command" or "Input" */
{
	int	lclear;/* left edge of area to clear */
	int	rclear;/* right edge of area to clear */
	char	tmp[24];

	/* If we have new text, and it is different from old text, use it */
	lclear = (int)xw->st.rulerpos;
	if (!info)
		info = toCHAR("");
	if (strncmp(xw->st.info, tochar8(info), sizeof xw->st.info - 1))
	{
		strncpy(xw->st.info, tochar8(info), sizeof(xw->st.info) - 1);
		xw->st.info[sizeof xw->st.info - 1] = '\0';
		lclear = 0;
	}

	/* Figure out which panes we need to erase (and redraw) */
	if (xw->st.mode != mode)
		rclear = xw->st.w;
	else if (xw->st.learn != learn)
		rclear = xw->st.modepos;
	else if (xw->st.line != line || xw->st.column != column)
		rclear = xw->st.learnpos;
	else
		rclear = xw->st.rulerpos;

	/* If we don't have to clear anything, then we don't draw anything
	 * either.  We're done!  Also true if there is no status window.
	 */
	if (lclear == rclear || xw->st.h == 0)
		return;

	/* clear the required part of the statusbar */
	XClearArea(x_display, xw->st.win, lclear, 0, (unsigned int)(rclear - lclear), 0, False);

	/* if necessary, redraw the info */
	if (lclear == 0 && xw->st.info[0])
	{
		XSetForeground(x_display, xw->gc, xw->st.fglabel);
		XSetFont(x_display, xw->gc, x_loadedcontrol->fontinfo->fid);
		x_drawstring(x_display, xw->st.win, xw->gc,
			0, 2 + statusbase, xw->st.info, strlen(xw->st.info));
		xw->fg = xw->st.fglabel;
	}

	/* redraw the panes that have changed */
	if (rclear > xw->st.modepos)
		xw->st.modepos = draw1pane(xw, (int)xw->st.w + 1, mode, &xw->st.modewidth);
	if (rclear > xw->st.learnpos)
	{
		if (learn == '*')
			strcpy(tmp, "Mod");
		else
		{
			strcpy(tmp, "   ");
			if (learn != ',')
				tmp[1] = learn;
		}
		xw->st.learnpos = draw1pane(xw, xw->st.modepos, tmp, &xw->st.learnwidth);
	}
	if (rclear > xw->st.rulerpos)
	{
		sprintf(tmp, "%ld,%-ld", line, column);
		xw->st.rulerpos = draw1pane(xw, xw->st.learnpos, tmp, &xw->st.rulerwidth);
	}

	/* draw a black line across the top of the status bar -- looks pretty */
	XSetForeground(x_display, xw->gc, x_black);
	XDrawLine(x_display, xw->st.win, xw->gc, 0, 0, xw->st.w - 1, 0);
	xw->fg = x_black;

	/* remember the current information (info already stored) */
	xw->st.mode = mode;
	xw->st.learn = learn;
	xw->st.line = line;
	xw->st.column = column;
}

/* Change only the info field of the status bar */
void x_st_info(xw, info)
	X11WIN	*xw;
	CHAR	*info;	/* info text, or NULL to revert to deftext */
{
	x_st_status(xw, info, xw->st.line, xw->st.column, xw->st.learn, xw->st.mode);
}

/* Redraw the statusbar from scratch */
void x_st_event(xw, event)
	X11WIN	*xw;
	XEvent	*event;
{
	char	tmp[80];
	char	*mode;

	switch (event->type)
	{
	  case Expose:
		/* force right edge of redrawn area to include rightmost pane */
		mode = xw->st.mode;
		xw->st.mode = NULL;

		/* force left edge of redrawn area to include text on left */
		strcpy(tmp, xw->st.info);
		xw->st.info[0]++;

		/* now draw it */
		x_st_status(xw, toCHAR(tmp), xw->st.line, xw->st.column, xw->st.learn, mode);
		break;
	}
}


void x_st_recolor(xw, font)
	X11WIN	*xw;	/* window to be recolored */
	_char_	font;	/* indicates which colors were changed */
{
	XEvent	event;

	/* Currently it uses the scrollbar background color for the toolbar
	 * itself, and the tool colors for the fields.
	 */
	switch (font)
	{
	  case 't':
		x_unloadcolor(xw->st.fglabel);
		xw->st.fglabel = x_loadcolor(x_toolfg, x_black);
		xw->st.fgface = x_loadcolor(x_toolbg, x_white);
		break;

	  case 's':
		x_unloadcolor(xw->st.bg);
		xw->st.bg = x_loadcolor(x_scrollbarbg, x_black);

		/* if there is a toolbar window, change its background */
		if (xw->st.h != 0)
			XSetWindowBackground(x_display, xw->st.win, xw->st.bg);
		break;

	  default:
		return;
	}

	/* redraw the toolbar */
	event.type = Expose;
	x_st_event(xw, &event);
}
#endif
