/* main.c */
/* Copyright 1995 by Steve Kirkendall */

char id_main[] = "$Id: main.c,v 2.29 1996/09/21 02:12:31 steve Exp $";

#include "elvis.h"

#if USE_PROTOTYPES
static void usage(char *hint);
static void guiusage(void);
static int parseflags(int argc, char **argv);
static int choosegui(int argc, char **argv);
static void doexrc(void);
static void buildargs(int argc, char **argv);
static void startfirst(void);
static void init(int argc, char **argv);
void mainfirstcmd(WINDOW win);
void term(void);
void main(int argc, char **argv);
#endif


/* This array contains pointers to all known GUIs.  Ideally, they should be
 * sorted so that fancy/rarely-available GUIs appear near the front of the
 * list, and basic/universal GUIs appear near the end.
 */
static GUI *allguis[] =
{
#ifdef GUI_WIN32
	&guiwin32
#endif

#ifdef GUI_X11
	&guix11,
#endif

#ifdef GUI_TERMCAP
	&guitermcap,
#endif

#ifdef GUI_CURSES
	&guicurses,
#endif

#ifdef GUI_BIOS
	&guibios,
#endif

#ifdef GUI_OPEN
	&guiopen,
	&guiquit
#endif
};

/* These flags are set according to command-line flags */
static char	*initialcommand;
static char	*initialtag;
static BOOLEAN	initialall;
GUI	*chosengui;

/* Give a usage message, and then exit */
static void usage(hint)
	char	*hint;	/* an error message */
{
	int	i;
	char	guinames[80];

	msg(MSG_INFO, "Usage: elvis [flags] [files]...");
	msg(MSG_INFO, "Flags: -V          Verbose -- give more status information");
	msg(MSG_INFO, "       -a          Create a separate window for each file");
	msg(MSG_INFO, "       -r          Restart a session after a crash");
	msg(MSG_INFO, "       -R          Mark new buffers as \"readonly\"");
	msg(MSG_INFO, "       -e          Start in ex mode instead of vi mode");
	msg(MSG_INFO, "       -i          Start in input mode instead of vi mode");
	msg(MSG_INFO, "       -s          Set the \"safer\" option, for security");
	msg(MSG_INFO, "       -w lines    Set scroll amount to \"lines\"");
	msg(MSG_INFO, "       -f session  Use \"session\" as the session file");
	msg(MSG_INFO, "       -G gui      Use the \"gui\" user interface \\(see below\\)");
	msg(MSG_INFO, "       -c command  Execute \"command\" after loading first file");
	msg(MSG_INFO, "       -t tag      Perform a tag search");
	msg(MSG_INFO, "       -b blksize  Use blocks of size \"blksize\"");
	msg(MSG_INFO, "       +command    Archaic form of \"-c command\" flag");
	guinames[0] = '\0';
	for (i = 0; i < QTY(allguis); i++)
	{
		/* space between names */
		if (i > 0)
			strcat(guinames, " ");

		/* concatenate the gui names into a list */
		switch ((*allguis[i]->test)())
		{
		  case 0:
			sprintf(guinames + strlen(guinames), "(%s)", allguis[i]->name);
			break;

		  case 2:
			sprintf(guinames + strlen(guinames), "%s?", allguis[i]->name);
			break;

		  default:
			strcat(guinames, allguis[i]->name);
		}
	}
	msg(MSG_INFO, "[s]User interfaces: $1", guinames);
	msg(MSG_INFO, "For more information about user interfaces, give the command 'elvis -G?'");
	msg(MSG_INFO, "[s]Report bugs to $1", "kirkenda@cs.pdx.edu");
	if (hint)
	{
		msg(MSG_INFO, hint);
	}
	if (chosengui)
		(*chosengui->term)();
	exit(0);
}

static void guiusage P_((void))
{
	int	i;
	BOOLEAN	found;
	char	*msgfmt;

	msg(MSG_INFO, "user interfaces:");
	for (i = 0, found = False; i < QTY(allguis); i++)
	{
		switch ((*allguis[i]->test)())
		{
		  case 0:
			msgfmt = "[ss]   -G ($1<<12) $2 \\(UNAVAILABLE\\)";
			break;

		  case 2:
			msgfmt = "[ss]   -G ($1<<12) $2 \\(MAYBE\\)";
			break;

		  default:
			if (found)
				msgfmt = "[ss]   -G ($1<<12) $2";
			else
				msgfmt = "[ss]   -G ($1<<12) $2 \\(DEFAULT\\)";
			found = True;
		}
		msg(MSG_INFO, msgfmt, allguis[i]->name, allguis[i]->desc);
		if (allguis[i]->usage)
			(*allguis[i]->usage)();
	}
	if (chosengui)
		(*chosengui->term)();
	exit(0);
}

/* parse command-line flags.  Leave global variables set to reflect the
 * flags discovered.  Upon return, any arguments handled here should be
 * deleted from argv[]; the return value is the new argc value.
 */
static int parseflags(argc, argv)
	int	argc;	/* number of command-line arguments */
	char	**argv;	/* values of command-line arguments */
{
	int	i, j, del;
	long	size = 0;

	/* copy argv[0] into an option so we can access it in "elvis.ini" */
	o_program = toCHAR(argv[0]);

	/* for each argument... */
	for (i = 1; i < argc; i++)
	{
		/* for now, assume we'll be deleting 0 arguements */
		del = 0;

		/* check for some special flags */
		if (!strcmp(argv[i], "-version")
		 || !strcmp(argv[i], "--version")
		 || !strcmp(argv[i], "-v"))
		{
			msg(MSG_INFO, "[s]elvis $1", VERSION);
#ifdef COPY1
			msg(MSG_INFO, "[s]$1", COPY1);
#endif
#ifdef COPY2
			msg(MSG_INFO, "[s]$1", COPY2);
#endif
#ifdef COPY3
			msg(MSG_INFO, "[s]$1", COPY3);
#endif
#ifdef COPY4
			msg(MSG_INFO, "[s]$1", COPY4);
#endif
#ifdef PORTEDBY
			msg(MSG_INFO, "[s]$1", PORTEDBY);
#endif
			if (chosengui)
				(*chosengui->term)();
			exit(0);
		}
		else if (!strcmp(argv[i], "-help")
		      || !strcmp(argv[i], "--help")
		      || !strcmp(argv[i], "-?"))
		{
			usage(NULL);
		}

		/* recognize any normal flags */
		if (argv[i][0] == '-')
		{
			for (j = 1; j != 0 && argv[i][j]; j++)
			{
				switch (argv[i][j])
				{
				  case 'a':
					initialall = True;
					del = 1;
					break;

				  case 'r':
					o_recovering = True;
					del = 1;
					break;

				  case 'R':
					o_defaultreadonly = True;
					del = 1;
					break;

				  case 'V':
				  	o_verbose = True;
				  	del = 1;
				  	break;

				  case 'f':
					if (argv[i][j + 1])
					{
						o_session = toCHAR(&argv[i][j + 1]);
						del = 1;
					}
					else if (i + 1 < argc)
					{
						o_session = toCHAR(argv[i + 1]);
						del = 2;
						i++;
					}
					else
					{
						usage("-s requires the name of a session file");
					}
					j = -1; /* so we stop processing this arg */
					break;

				  case 'G':
					usage("only one \"-G gui\" is allowed");
					break;

				  case 'c':
					if (argv[i][j + 1])
					{
						initialcommand = &argv[i][j + 1];
						del = 1;
					}
					else if (i + 1 < argc)
					{
						initialcommand = argv[i + 1];
						del = 2;
						i++;
					}
					else
					{
						usage("-c requires an initial command");
					}
					j = -1; /* so we stop processing this arg */
					break;

				  case 't':
					if (argv[i][j + 1])
					{
						initialtag = &argv[i][j + 1];
						del = 1;
					}
					else if (i + 1 < argc)
					{
						initialtag = argv[i + 1];
						del = 2;
						i++;
					}
					else
					{
						usage("-t requires a tag name");
					}
					j = -1; /* so we stop processing this arg */
					break;

				  case 'b':
					if (argv[i][j + 1])
					{
						size = atol(&argv[i][j + 1]);
						del = 1;
					}
					else if (i + 1 < argc)
					{
						size = atol(argv[i + 1]);
						del = 2;
						i++;
					}
					else
					{
						usage("-b requires a block size for the session file");
					}
					if (size < 256 || size > 8192)
					{
						usage("bad blksize given for -b");
					}
					j = -1; /* so we stop processing this arg */
					o_blksize = size;
					break;

				  case 'e':
				  case 'i':
					o_initialstate = argv[i][j];
					del = 1;
					break;

				  case 's':
					o_safer = True;
					del = 1;
					break;

				  case 'w':
					if (argv[i][j + 1])
					{
						size = atol(&argv[i][j + 1]);
						del = 1;
					}
					else if (i + 1 < argc)
					{
						size = atol(argv[i + 1]);
						del = 2;
						i++;
					}
					else
					{
						usage("-w requires a window size");
					}
					j = -1; /* so we stop processing this arg */
					break;

				  default:
					if (del)
					{
						usage(NULL);
					}
					j = -1;
				}
			}
		}
		else if (argv[i][0] == '+')
		{
			if (argv[i][1])
			{
				initialcommand = &argv[i][1];
			}
			else
			{
				initialcommand = "$";
			}
			del = 1;
		}

		/* delete arguments, if we're supposed to */
		if (del > 0)
		{
			for (j = i + 1 - del; j < argc - del; j++)
			{
				argv[j] = argv[j + del];
			}
			i -= del;
			argc -= del;
		}
	}

	return argc;
}

/* Choose a GUI.  If one was specified via "-G gui", then the o_gui option
 * will be set already, and we just need to verify it.  Otherwise, we need
 * to test each GUI until we find one which tests as being available or
 * maybe available.  Call the GUI's init() function, and leave the global
 * "gui" pointer pointing to the chosen GUI.
 *
 * Like parseflags(), this function should delete any arguments from argv that
 * it uses, and return the new argc value.
 */
static int choosegui(argc, argv)
	int	argc;	/* number of command-line arguments */
	char	**argv;	/* values of command-line arguments */
{
	int	i, j, val;

	/* Initialize "j" just to silence a compiler warning */
	j = 0;

	/* search for "-G gui" in the command line */
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-' && argv[i][1] == 'G')
		{
			/* remember the gui name */
			if (argv[i][2])
				o_gui = toCHAR(&argv[i][2]), j = i + 1;
			else if (i + 1 < argc)
				o_gui = toCHAR(argv[i + 1]), j = i + 2;
			else
				usage("-G requires a user interface name");

			/* delete the "-G" and its argument */
			while (j < argc)
			{
				argv[i++] = argv[j++];
			}
			argv[i] = NULL;
			argc = i;

			/* stop looking for "-G" */
			break;
		}
	}

	/* find the specified GUI, or the first available if none specified */
	if (o_gui)
	{
		/* find the named GUI */
		for (i = 0; i < QTY(allguis) && strcmp(allguis[i]->name, tochar8(o_gui)); i++)
		{
		}
		if (i >= QTY(allguis))
		{
			if (CHARcmp(o_gui, toCHAR("?")))
			{
				msg(MSG_ERROR, "[s]invalid gui $1", o_gui);
			}
			guiusage();
		}
	}
	else
	{
		/* Find the first GUI that is definitely available.  While
		 * searching, also remember the first one that *MIGHT* be
		 * available, just in case we don't find any that are definitely
		 * available.
		 */
		for (i = 0, j = -1; i < QTY(allguis) && (val = (*allguis[i]->test)()) != 1; i++)
		{
			if (val == 2 && j == -1)
			{
				j = i;
			}
		}

		/* If we didn't find a definite one, use the maybe one */
		if (i >= QTY(allguis))
		{
			if (j >= 0)
			{
				i = j;
			}
			else
			{
				msg(MSG_ERROR, "no gui available", o_gui);
				exit(0);
			}
		}
	}

	/* Call the GUI's init() function */
	argc = (*allguis[i]->init)(argc, argv);
	if (argc <= 0)
	{
		exit(0);
	}

	/* Remember the chosen GUI, but don't set the official "gui" pointer
	 * yet because we want any initialization error message to go to
	 * stderr instead of a window.
	 */
	chosengui = allguis[i];
	return argc;
}

/* execute the initialization buffer (which may create some windows) */
static void doexrc()
{
	BUFFER	buf;	/* the buffer itself */
	MARKBUF	top;	/* top of the buffer */
	MARKBUF	bottom;	/* bottom of the buffer */

	/* If a buffer named "Elvis initialization" exists */
	buf = bufpath(o_elvispath, INIT_FILE, toCHAR(INIT_BUF));
	if (buf)
	{
		/* Temporarily make all display-mode options available */
		dispinit(True);

		/* Execute its contents. */
		(void)experform((WINDOW)0, marktmp(top, buf, 0),
			marktmp(bottom, buf, o_bufchars(buf)));

		/* After this, only the current display-mode's options should
		 * be available.
		 */
		dispinit(False);
	}
}

/* make the "elvis args" buffer contain file names from command line */
static void buildargs(argc, argv)
	int	argc;	/* number of command-line arguments */
	char	**argv;	/* values of command-line arguments */
{
	int	i;

	/* skip "--", if given */
	argc--;
	argv++;
	if (argc > 0 && !strcmp(argv[0], "--"))
	{
		argc--;
		argv++;
	}

	/* Copy each argument to the args list.  For some operating systems,
	 * we'll be expanding wildcards in each argument as we do this.
	 */
	for (argnext = i = 0; i < argc; i++)
	{
		(void)exaddfilearg(&arglist, &argnext, argv[i], OSEXPANDARGS);
	}

	/* if nothing read was added, then still allocate something */
	if (!arglist)
		arglist = (char **)safealloc(1, sizeof(char *));

	/* reset the "next" pointer */
	argnext = 0;
}

/* start first file */
static void startfirst()
{
	BUFFER	buf;

	/* If "Elvis args" is empty */
	if (!arglist[0])
	{
		/* Create an anonymous buffer and a window for it */
		buf = bufalloc(NULL, 0);
		assert(buf);
		(void)(*gui->creategw)(tochar8(o_bufname(buf)), "");
	}
	else
	{
		/* Load buffers and make windows for either just the first
		 * argument, or as many of the arguments as possible.
		 */
		assert(argnext == 0);
		do
		{
			/* Create a buffer & window the first file */
			buf = bufload((CHAR *)0, arglist[argnext++], True);
			assert(buf);
		} while ((*gui->creategw)(tochar8(o_bufname(buf)), "")
					&& initialall && arglist[argnext]);
	}
}

/* perform "-c cmd" or "-t tag" */
void mainfirstcmd(win)
	WINDOW	win;	/* the first window */
{
	CHAR	tagcmd[100];

	/* If "-c cmd" was given */
	if (initialcommand)
	{
		/* Execute the command */
		exstring(win, toCHAR(initialcommand));
	}

	/* If "-t tag" was given */
	if (initialtag)
	{
		/* Compose a ":tag" command */
		CHARcpy(tagcmd, toCHAR("tag "));
		CHARcat(tagcmd, toCHAR(initialtag));

		/* Execute the command */
		exstring(win, tagcmd);
	}
}

static void init(argc, argv)
	int	argc;	/* number of command-line arguments */
	char	**argv;	/* values of command-line arguments */
{
	BUFFER	buf;
	char	*lc;		/* locale name */
	char	lcfile[100];	/* combination of locale name and file name */

#ifdef OSINIT
	/* initialize the OS */
	osinit(argv[0]);
#endif

	/* initialize options */
	optglobinit();

	/* Choose a GUI, and call its init() function */
	argc = choosegui(argc, argv);

	/* Parse command-line flags */
	argc = parseflags(argc, argv);

	/* set up the default options for windows */
	wininit();

	/* Create or restart the session file */
	bufinit();

	/* Set the "gui" pointer to the chosen gui.  After this point, all
	 * error/warning/info messages will written to the window instead
	 * of stderr.
	 */
	gui = chosengui;
	o_gui = toCHAR(gui->name);

	/* Create the "Elvis ex history" buffer, and tweak its options */
	buf = bufalloc(toCHAR(EX_BUF), (BLKNO)0);
	o_inputtab(buf) = 'f';
	o_internal(buf) = True;

	/* Create the "Elvis error list" buffer, and tweak its options */
	buf = bufalloc(toCHAR(ERRLIST_BUF), (BLKNO)0);
	o_internal(buf) = True;
	o_undolevels(buf) = 0;

	/* Execute the initialization buffer (which may create some windows) */
	doexrc();

	/* Load the verbose messages, plus a few others */
	if (((lc = getenv("LC_ALL")) != NULL && *lc)
	 || ((lc = getenv("LC_MESSAGES")) != NULL && *lc)
	 || ((lc = getenv("LANG")) != NULL && *lc))
	{
		/* Try to find "elvis.msg" in a locale-dependent subdirectory.
		 * If you can't find it there, then look for the standard one.
		 */
		strcpy(lcfile, dirpath(lc, MSG_FILE));
		buf = bufpath(o_elvispath, lcfile, toCHAR(MSG_BUF));
		if (!buf || o_bufchars(buf) == 0)
		{
			(void)bufpath(o_elvispath, MSG_FILE, toCHAR(MSG_BUF));
		}
	}
	else
	{
		/* just use the standard versbose messages */
		(void)bufpath(o_elvispath, MSG_FILE, toCHAR(MSG_BUF));
	}
	(void)bufpath(o_elvispath, BEFOREREAD_FILE, toCHAR(BEFOREREAD_BUF));
	(void)bufpath(o_elvispath, AFTERREAD_FILE, toCHAR(AFTERREAD_BUF));
	(void)bufpath(o_elvispath, BEFOREWRITE_FILE, toCHAR(BEFOREWRITE_BUF));
	(void)bufpath(o_elvispath, AFTERWRITE_FILE, toCHAR(AFTERWRITE_BUF));

	/* Store the filename arguments in a list */
	buildargs(argc, argv);

	/* start the first file (i.e., make sure we have at least 1 window) */
	startfirst();
}

/* Terminate elvis.  By the time this function is called, all windows have
 * been closed, and hence any buffers that can be discarded have been.
 */
void term()
{
#ifdef DEBUG_ALLOC
	int	i;
#endif

	/* Call the GUI's term() function */
	gui->term();
	gui = NULL;

	/* Flush any final messages */
	msgflush();

	/* Close the session */
	sesclose();

#ifdef DEBUG_ALLOC
	/* Free the args list so it isn't reported as a memory leak */
	for (i = 0; arglist[i]; i++)
		safefree(arglist[i]);
	safefree(arglist);
#endif

	/* Check for any memory leaks */
	safeterm();
}

void main(argc, argv)
	int	argc;	/* number of command-line arguments */
	char	**argv;	/* values of the command-line arguments */
{
	init(argc, argv);
	(*gui->loop)();
	term();
#if !defined (GUI_WIN32)
	exit((int)o_exitcode);
#endif
}
