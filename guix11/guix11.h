/* guix11.h */

#include <sys/types.h>
#include <sys/time.h>
#ifdef NEED_SELECT_H
# include <sys/select.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#ifdef NEED_XOS_H
# include <X11/Xos.h>
#endif
#ifndef NO_XLOCALE
# include <X11/Intrinsic.h>
# if (XtSpecificationRelease < 6)
#  define NO_XLOCALE
# else
#  include <X11/Xlocale.h>
# endif
#endif	/* NO_XLOCALE */

typedef struct x11win_s X11WIN;

#include "xmisc.h"
#include "xscroll.h"
#include "xtool.h"
#include "xtext.h"
#include "xstatus.h"
#include "xevent.h"
#include "xclip.h"
#include "xdialog.h"

struct x11win_s
{
	struct x11win_s	*next;	   /* pointer to some other window */
	Window		win;	   /* top-level X window */
	X_SCROLLBAR	sb;	   /* scrollbar info */
	X_TOOLBAR	tb;	   /* toolbar info */
	X_TEXTAREA	ta;	   /* text area info */
	X_STATUSBAR	st;	   /* status bar info */
	GC		gc;	   /* graphic context for this window */
	unsigned long	fg, bg;	   /* current foreground & background */
	BOOLEAN		grexpose;  /* are graphic exposures allow now? */
	BOOLEAN		ismapped;  /* is window visible? */
	char		*title;	   /* name of the window */
	ELVISSTATE	state;	   /* command state of window */
	int		x, y;	   /* position of window */
	unsigned int	w, h;	   /* overall size of the window */
	BOOLEAN		nowarp;	   /* don't warp pointer into this window */
#ifndef NO_XLOCALE
	XIC		ic;	   /* input context, for composing chars */
#endif
};

extern Display		*x_display;	/* X11 display */
extern int		x_screen;	/* screen number */
extern X11WIN		*x_winlist;	/* list of windows */
extern int		x_depth;	/* bits per pixel */
extern Colormap		x_colormap;	/* colormap shared by elvis windows */
extern BOOLEAN		x_ownselection;	/* does elvis own the X11 selection? */
extern X11WIN		*x_hasfocus;	/* window with kbd. focus, or NULL */
extern unsigned long	x_black;	/* black pixel color */
extern unsigned long	x_white;	/* white pixel color */
extern BOOLEAN		x_mono;		/* is this a monochrome display? */
extern Time		x_now;		/* timestamp of recent event */
extern Atom		x_elvis_cutbuffer;/* id for cut/paste buffer */
extern Atom		x_wm_protocols;	/* value for WM_PROTOCOLS atom */
extern Atom		x_wm_delete_window;/* value for WM_DELETE_WINDOW atom */
extern Atom		x_elvis_server;	/* value for ELVIS_SERVER atom */
extern Atom		x_resource_manager;/* value for MANAGER_RESOURCES atom */
extern Pixmap		x_gray;			/* gray background for mono */
extern Pixmap		x_elvis_icon;		/* elvis' window icon */
extern Pixmap		x_elvis_pin_icon;	/* elvis icon with pushpin */
extern unsigned		x_elvis_icon_width;	/* width of x_elvis_icon */
extern unsigned		x_elvis_icon_height;	/* height of x_elvis_icon */
extern X_LOADEDFONT	*x_defaultnormal;/* normal font */
extern X_LOADEDFONT	*x_defaultbold;	 /* bold font, or NULL to fake it */
extern X_LOADEDFONT	*x_defaultitalic;/* italic font, or NULL to fake it */
extern X_LOADEDFONT	*x_loadedcontrol;/* toolbar font */

extern void x_reconfig P_((X11WIN *xw, unsigned columns, unsigned rows));

extern struct x_optvals_s
{
	OPTVAL	normalfont, boldfont, italicfont, controlfont, toolbar,
		scrollbarwidth, scrollbartime, scrollbarleft, scrollbar,
		statusbar, dblclicktime, blinktime, xrows, xcolumns,
		firstx, firsty, icon, iconic, stopshell, autoiconify,
		altkey, stagger, warpback, warpto, focusnew, textcursor,
		underline, outlinemono, borderwidth, xrootwidth, xrootheight,
		submit, cancel, help;
} x_optvals;
#define o_normalfont	 x_optvals.normalfont.value.string
#define o_boldfont	 x_optvals.boldfont.value.string
#define o_italicfont	 x_optvals.italicfont.value.string
#define o_controlfont	 x_optvals.controlfont.value.string
#define o_toolbar	 x_optvals.toolbar.value.boolean
#define o_scrollbarwidth x_optvals.scrollbarwidth.value.number
#define o_scrollbartime	 x_optvals.scrollbartime.value.number
#define o_scrollbarleft	 x_optvals.scrollbarleft.value.boolean
#define o_scrollbar	 x_optvals.scrollbar.value.boolean
#define o_statusbar	 x_optvals.statusbar.value.character
#define o_dblclicktime	 x_optvals.dblclicktime.value.number
#define o_blinktime	 x_optvals.blinktime.value.number
#define o_xrows		 x_optvals.xrows.value.number
#define o_xcolumns	 x_optvals.xcolumns.value.number
#define o_firstx	 x_optvals.firstx.value.number
#define o_firsty	 x_optvals.firsty.value.number
#define o_icon		 x_optvals.icon.value.boolean
#define o_iconic	 x_optvals.iconic.value.boolean
#define o_stopshell	 x_optvals.stopshell.value.string
#define o_autoiconify	 x_optvals.autoiconify.value.boolean
#define o_altkey	 x_optvals.altkey.value.character
#define o_stagger	 x_optvals.stagger.value.number
#define o_warpback	 x_optvals.warpback.value.boolean
#define o_warpto	 x_optvals.warpto.value.character
#define o_focusnew	 x_optvals.focusnew.value.boolean
#define o_textcursor	 x_optvals.textcursor.value.character
#define o_underline	 x_optvals.underline.value.boolean
#define o_outlinemono	 x_optvals.outlinemono.value.number
#define o_borderwidth	 x_optvals.borderwidth.value.number
#define o_xrootwidth	 x_optvals.xrootwidth.value.number
#define o_xrootheight	 x_optvals.xrootheight.value.number
#define o_submit	 x_optvals.submit.value.string
#define o_cancel	 x_optvals.cancel.value.string
#define o_help		 x_optvals.help.value.string

/* The following store names of colors */
extern CHAR x_background[50];	
extern CHAR x_foreground[50];
extern CHAR x_boldcolor[50];
extern CHAR x_emphcolor[50];
extern CHAR x_fixedcolor[50];
extern CHAR x_italiccolor[50];
extern CHAR x_underlinecolor[50];
extern CHAR x_cursorcolor[50];
extern CHAR x_owncolor[50];
extern CHAR x_toolbg[50];
extern CHAR x_toolfg[50];
extern CHAR x_scrollbarbg[50];
extern CHAR x_scrollbarfg[50];
