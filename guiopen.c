/* guiopen.c */
/* Copyright 1995 by Steve Kirkendall */

char id_guiopen[] = "$Id: guiopen.c,v 2.19 1996/09/26 18:43:37 steve Exp $";

#include "elvis.h"
#ifdef GUI_OPEN

/* This file contains a user interface which internally resembles guitcap.c,
 * but which doesn't depend on termcap and can therefore be used with terminals
 * of an unknown type.
 */


#if USE_PROTOTYPES
static BOOLEAN creategw(char *name, char *attributes);
static int init(int argc, char **argv);
static int test(void);
static void beep(GUIWIN *gw);
static void destroygw(GUIWIN *gw, BOOLEAN force);
static void endtty(void);
static void loop(void);
static void quitloop(void);
static BOOLEAN quitmsg(GUIWIN *gw, MSGIMP imp, CHAR *text, int len);
static void starttty(void);
static void term(void);
static void textline(GUIWIN *gw, CHAR *text, int len);
static BOOLEAN oprgopen(char *command, BOOLEAN willread, BOOLEAN willwrite);
static int oprgclose(void);
#endif


static char ttyerasekey;	/* taken from the ioctl structure */

static void starttty()
{
	/* change the terminal mode to cbreak/noecho */
	ttyinit();
	ttyraw(&ttyerasekey);
}

static void endtty()
{
	/* change the terminal mode back the way it was */
	ttynormal();

	/* leave the cursor on a fresh line */
	ttywrite("\n", 1);
}

/*----------------------------------------------------------------------------*/
/* start of GUI functions */

GUIWIN *current;
BOOLEAN	anycmd;

/* return True if available. */
static int test P_((void))
{
	return 1;
}


/* initialize the interface. */
static int init(argc, argv)
	int	argc;	/* number of command-line arguments */
	char	**argv;	/* values of command-line arguments */
{
	int	i;

	/* initialize the termcap stuff */
	starttty();

	/* set anycmd if there is a "-c cmd" or "+cmd" argument. */
	for (i = 1; i < argc && !anycmd; i++)
	{
		anycmd = (BOOLEAN)(argv[i][0] == '+' ||
				  (argv[i][0] == '-' && argv[i][1] == 'c'));
	}
	return argc;
}


/* Repeatedly get events (keystrokes), and call elvis' event functions */
static void loop P_((void))
{
	char	buf[10];
	int	len;
	MAPSTATE mst = MAP_CLEAR;

	while (current)
	{
		/* redraw the window(s) */
		(void)eventdraw(current);

		/* read events */
		do
		{
			len = ttyread(buf, sizeof buf, (mst==MAP_CLEAR ? 0 : 2));
		} while (len < 1);

		/* process keystroke data */
		mst = eventkeys(current, toCHAR(buf), len);
	}
}


/* This is the loop function for the "quit" user interface.  It immediately
 * deletes the only window, and then exits.
 */
static void quitloop P_((void))
{
	if (!anycmd)
		msg(MSG_INFO, "session=(session)");
	if (current)
		(void)eventdestroy(current);
}


/* shut down the termcap interface */
static void term P_((void))
{
	endtty();
}


/* This function creates a window */
static BOOLEAN creategw(name, attributes)
	char	*name;		/* name of new window's buffer */
	char	*attributes;	/* other parameters, if any */
{
	/* We can only have one window.  If we already made it, fail */
	if (current)
	{
		return False;
	}

	/* make elvis do its own initialization */
	current = (GUIWIN *)1;
	if (!eventcreate(current, NULL, name, 24, 80))
	{
		/* elvis can't make it -- fail */
		return False;
	}

	return True;
}


/* This function deletes a window */
static void destroygw(gw, force)
	GUIWIN	*gw;	/* window to be destroyed */
	BOOLEAN	force;	/* if True, try harder */
{
	assert(gw == current);

	/* simulate a "destroy" event */
	eventdestroy(gw);
	current = NULL;
}

/* This function rings the bell */
static void beep(gw)
	GUIWIN	*gw;	/* window that generated a beep request */
{
	ttywrite("\007", 1);
}


/* This function outputs a mixture of text and control characters.  The only
 * possible control characters will be '\b', '\n', and '\r'.
 */
static void textline(gw, text, len)
	GUIWIN	*gw;	/* where the text is to be output */
	CHAR	*text;	/* the text to be output */
	int	len;	/* length of text */
{
	ttywrite(tochar8(text), len);
}


/* This function is used by the "quit" gui to hide all messages except errors */
static BOOLEAN quitmsg(gw, imp, text, len)
	GUIWIN	*gw;	/* window which generated the message */
	MSGIMP	imp;	/* message importance */
	CHAR	*text;	/* the message itself */
	int	len;	/* length of the message */
{
	return (BOOLEAN)(imp != MSG_ERROR && imp != MSG_FATAL);
}


static BOOLEAN oprgopen(command, willwrite, willread)
	char	*command;
	BOOLEAN	willwrite;
	BOOLEAN	willread;
{
	/* switch the tty to normal mode */
	ttynormal();
	
	/* use the OS prgopen() function */
	return prgopen(command, willwrite, willread);
}


static int oprgclose P_((void))
{
	int	ret;
	
	/* use the OS perclose() function */
	ret = prgclose();

	/* switch the tty back to raw mode */
	ttyraw(&ttyerasekey);
	
	return ret;
}


/* structs of this type are used to describe each available GUI */
GUI guiopen =
{
	"open",		/* name */
	"Generic interface with limited capabilities",
	False,	 	/* exonly */
	True,	 	/* newblank */
	False,	 	/* minimizeclr */
	True,		/* scrolllast */
	False,		/* shiftrows */
	3,		/* movecost */
	0,		/* opts */
	NULL,		/* optdescs */
	test,
	init,
	NULL,		/* usage */
	loop,
	NULL,	 	/* poll */
	term,
	creategw,
	destroygw,
	NULL,		/* focusgw */
	NULL,		/* retitle */
	NULL,		/* reset */
	NULL,		/* flush */
	NULL,		/* moveto */
	NULL,		/* draw */
	NULL,		/* shift */
	NULL,		/* scroll */
	NULL,		/* clrtoeol */
	textline,
	beep,
	NULL,		/* msg */
	NULL,		/* scrollbar */
	NULL,		/* status */
	NULL,		/* keylabel */
	NULL,		/* clipopen */
	NULL,		/* clipwrite */
	NULL,		/* clipread */
	NULL,		/* clipclose */
	NULL,		/* color */
	NULL,	 	/* guicmd */
	NULL,		/* tabcmd */
	NULL,		/* save */
	NULL,		/* wildcard */
	oprgopen,	/* prgopen */
	oprgclose	/* prgclose */
};


/* structs of this type are used to describe each available GUI */
GUI guiquit =
{
	"quit",		/* name */
	"Quits immediately after processing \"-c cmd\"",
	False,	 	/* exonly */
	True,	 	/* newblank */
	False,	 	/* minimizeclr */
	True,		/* scrolllast */
	False,		/* shiftrows */
	3,		/* movecost */
	0,		/* opts */
	NULL,		/* optdescs */
	test,
	init,
	NULL,		/* usage */
	quitloop,
	ttypoll,	/* poll */
	term,
	creategw,
	destroygw,
	NULL,		/* focusgw */
	NULL,		/* retitle */
	NULL,		/* reset */
	NULL,		/* flush */
	NULL,		/* moveto */
	NULL,		/* draw */
	NULL,		/* shift */
	NULL,		/* scroll */
	NULL,		/* clrtoeol */
	textline,
	beep,
	quitmsg,
	NULL,		/* scrollbar */
	NULL,		/* status */
	NULL,		/* keylabel */
	NULL,		/* clipopen */
	NULL,		/* clipwrite */
	NULL,		/* clipread */
	NULL,		/* clipclose */
	NULL,		/* color */
	NULL,	 	/* guicmd */
	NULL,		/* tabcmd */
	NULL,		/* save */
	NULL,		/* wildcard */
	oprgopen,	/* prgopen */
	oprgclose	/* prgclose */
};
#endif
