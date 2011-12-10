/* elvgdb.c */

/* Copyright 2004 by Steve Kirkendall.  This program may be freely redistributed
 * under the terms of the Perl "Clarified Artistic License".
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include "elvis.h"

/* This typedef is used for storing the attributes of an annotation.  The
 * first three bits indicate the highlighting color for subsequent text,
 * and every other attribute is simply a one-bit boolean flag.
 */
typedef enum {
	EG_BLACK = 0,		/* black foreground */
	EG_RED = 1,		/* red foreground */
	EG_GREEN = 2,		/* green foreground */
	EG_BROWN = 3,		/* brown/yellow foreground */
	EG_BLUE = 4,		/* blue foreground */
	EG_MAGENTA = 5,		/* magenta foreground */
	EG_CYAN = 6,		/* cyan foreground */
	EG_WHITE = 7,		/* white foreground */
	EG_COLOR = 8,		/* change the color */
	EG_BOLD = 16,		/* turn on the bold/bright bit */
	EG_UNDERLINE = 32,	/* turn on the underline bit */
	EG_HIGHLIGHT = 64,	/* don't turn off colors or other attributes */

	EG_FROMGDB = 256,	/* start a :fromgdb command */
	EG_TOELVIS = 512,	/* end a :fromgdb command, and send it */
	EG_ARGS = 1024,		/* append this annotation's args to command */
	EG_TEXT = 2048,		/* append the following text to command */
	EG_TABLE = 4096,	/* save name of table */
	EG_RECORD = 8192,	/* send previous record, start a new one */
	EG_FLUSH = 16384,	/* flush output after each character */
	EG_TRACE = 32768,	/* display the annotation's name */
	EG_SERVER = 65536,	/* hide output if "server" command */
	EG_LIMIT = 131072	/* guard against multiple messages */
} attrib_t;
#define EG_YELLOW (EG_BROWN|EG_BOLD)
#define EG_GRAY (EG_BLACK|EG_BOLD)
#define EG_COLORMASK (EG_WHITE)

static struct
{
	char	*name;	/* name of an attribute */
	attrib_t attrib;/* bits that it should set */
} attribname[] =
{
	{"black",	EG_BLACK|EG_COLOR},
	{"red",		EG_RED|EG_COLOR},
	{"green",	EG_GREEN|EG_COLOR},
	{"brown",	EG_BROWN|EG_COLOR},
	{"blue",	EG_BLUE|EG_COLOR},
	{"magenta",	EG_MAGENTA|EG_COLOR},
	{"cyan",	EG_CYAN|EG_COLOR},
	{"white",	EG_WHITE|EG_COLOR},
	{"yellow",	EG_BROWN|EG_BOLD|EG_COLOR},
	{"gray",	EG_BLACK|EG_BOLD|EG_COLOR},
	{"bold",	EG_BOLD},
	{"underline",	EG_UNDERLINE},
	{"highlight",	EG_HIGHLIGHT},
	{"fromgdb",	EG_FROMGDB},
	{"toelvis",	EG_TOELVIS},
	{"args",	EG_ARGS},
	{"text",	EG_TEXT},
	{"table",	EG_TABLE},
	{"record",	EG_RECORD},
	{"flush",	EG_FLUSH},
	{"trace",	EG_TRACE},
	{"server",	EG_SERVER},
	{"limit",	EG_LIMIT}
};


typedef struct annot_s
{
	struct annot_s	*next;	/* some other annotation */
	attrib_t	attrib;	/* attributes of this annotation */
	int		sent;	/* Boolean: have we sent one already? */
	char		name[1];/* storage space for name of annotation */
} annot_t;

/* Forward declarations for functions */
#ifndef P_
# if __stdc__
#  define P_(X) X
# else
#  define P_(X) ()
# endif
#endif
static void openx P_((void));
static annot_t *findannot P_((char *name));
static char *searchpath P_((char *pathlist, char *filename));
static void loadannot P_((char *filename));
static FILE *opengdb P_((int argc, char **argv));
static int closegdb P_((FILE *fromgdb));
static void highlight P_((attrib_t attrib));
static attrib_t doannot P_((Window elviswin, char *name, char *args));
static attrib_t annotation P_((FILE *fromgdb, Window elviswin));
static void echoch P_((attrib_t attrib, int ch));
static int rungdb P_((int argc, char **argv));
static Window findgdb P_((void));
static int tellgdb P_((int argc, char **argv));
extern int main P_((int argc, char **argv));

static annot_t *annotlist;		/* list of annotations */

static attrib_t defattrib = (attrib_t)0;	/* default attributes */
static attrib_t traceattrib = EG_HIGHLIGHT;	/* trace highlight attributes */

/* This stores the :fromgdb command that will be sent to elvis */
static char toelvis[1000];
static char *buildtoelvis;
static char tablename[100];
static long secret;

/* This is used to handle "server" suppression.  If hideserver=NULL, then
 * output as normal.  Else if pointing to a non-'\0' character, then compare
 * the next output char with *hideserver, and increment *hideserver if match
 * or set it to NULL if mismatch.  Else (when pointing to a '\0') suppress
 * output.
 */
static char *hideserver;

/* These store info about the X server */
static Display *display;	/* display */
static Window	root;		/* root window of default screen */
static Atom	elvis_server;	/* window attribute tag for finding elvis */
static Atom	elvgdb_server;	/* window attribute tag for finding elvgdb */
static Atom	targets;	/* the TARGETS atom */


/* This does most of the work needed for opening a connection to the X server */
static void openx()
{
	char	name[100];

	/* open the display */
	display = XOpenDisplay("");
	if (!display)
	{
		fprintf(stderr, "elvgdb: could not access the X server\n");
		exit(2);
	}

	/* find the root window of the default screen */
	root = RootWindow(display, DefaultScreen(display));

	/* convert attribute names to atoms */
	targets = XInternAtom(display, "TARGETS", True);
	sprintf(name, "ELVIS_SERVER_%d@", geteuid());
	gethostname(name + strlen(name), sizeof name - strlen(name));
	elvis_server = XInternAtom(display, name, True);
	sprintf(name, "ELVGDB_SERVER_%d@", geteuid());
	gethostname(name + strlen(name), sizeof name - strlen(name));
	elvgdb_server = XInternAtom(display, name, False);
}

/* locate an annotation's info.  If not found, then return NULL */
static annot_t *findannot(name)
	char	*name;	/* name of the annotation to find */
{
	annot_t	*scan;

	/* search for it */
	for (scan = annotlist;
	     scan && strcmp(scan->name, name);
	     scan = scan->next);

	return scan;
}

/* search through a path for a given file */
static char *searchpath(pathlist, filename)
	char	*pathlist;	/* colon-delimited list of directories */
	char	*filename;	/* name to search for */
{
	static char fullname[300];
	char	*home, *build;

	/* get the value of $HOME */
	home = getenv("HOME");

	/* for each directory in the pathlist... */
	do
	{
		/* if empty, then use current directory */
		if (*pathlist == ':' || *pathlist == '\0')
		{
			strcpy(fullname, filename);
		}
		else /* use the given directory name */
		{
			/* if starts with "~/" then use $HOME/ */
			build = fullname;
			if (*pathlist == '~' && !isalnum(pathlist[1]) && home)
			{
				strcpy(fullname, home);
				build = fullname + strlen(fullname);
				pathlist++;
			}

			/* copy the directory name */
			while (*pathlist != ':' && *pathlist)
			{
				*build++ = *pathlist++;
			}

			/* append a slash unless the dir name already has one */
			if (build != fullname && build[-1] != '/')
				*build++ = '/';

			/* append the filename */
			strcpy(build, filename);
		}

		/* if the file exists here, then return the name */
		if (access(fullname, F_OK) >= 0)
			return fullname;
	} while (*pathlist++ == ':');

	/* not found */
	return NULL;
}


/* load annotation attributes */
static void loadannot(filename)
	char	*filename;	/* name of file containing annotation info */
{
	FILE	*fp;	/* used for scanning the annotation info */
	char	buf[1000];/* a line from the file */
	char	*name;	/* pointer to an annotation name in buf */
	attrib_t attrib;/* the attribute flags */
	int	lnum;	/* line number */
	annot_t	*annot;	/* an annotation struct */
	annot_t	*all;	/* the "all" pseudo-annotation */
	char	*str;
	int	i;

	/* open the file */
	fp = fopen(filename, "r");
	if (!fp)
		return;

	/* for each line... */
	lnum = 0;
	while (fgets(buf, sizeof buf, fp))
	{
		lnum++;

		/* skip leading whitespace */
		for (name = buf; isspace(*name); name++)
		{
		}

		/* skip if empty line or it starts with "#" */
		if (*name == '\0' || *name == '#')
			continue;

		/* look for the ':' delimiting annotations from attributes */
		str = strchr(name, ':');
		if (!str)
		{
			fprintf(stderr, "elvgdb: %s line %d has no ':'\n",
				filename, lnum);
			continue;
		}
		*str++ = '\0';
		if (!*name)
		{
			fprintf(stderr, "elvgdb: %s line %d has no annotations\n",
				filename, lnum);
			continue;
		}

		/* scan the attribute string... */
		for (attrib = (attrib_t)0, name = NULL; *str; str++)
		{
			/* did we reach the end of a name? */
			if (!isalnum(*str) && name)
			{
				/* terminate the name */
				*str = '\0';

				/* see if we recognize it */
				for (i = 0; i < QTY(attribname) && strcmp(attribname[i].name, name); i++)
				{
				}
				if (i < QTY(attribname))
					attrib |= attribname[i].attrib;
				else
				{
					fprintf(stderr, "elvgdb: unknown attribute \"%s\" in file \"%s\" line %d\n", name, filename, lnum);
				}

				/* reset for next */
				name = NULL;
			}

			/* are we starting a new attribute name? */
			if (!name && isalnum(*str))
				name = str;
		}
		/* NOTE: Lucky for us, fgets() keeps the newline at the end of
		 * an input line.  This means that the last attribute name is
		 * always followed by a newline, if nothing else, so we don't
		 * need to worry about the case of dangling attribute name.
		 * By the time we reach this point, name=NULL.
		 */

		/* Scan the annotation string, and add these attributes to
		 * each annotation.
		 */
		str = buf - 1;
		name = NULL;
		do
		{
			/* next character */
			str++;

			/* end of a name? */
			if (name && !isalnum(*str) && *str != '-')
			{
				/* find the annotation, or add it */
				i = *str;
				*str = '\0';
				annot = findannot(name);
				if (!annot)
				{
					annot = (annot_t *)malloc(sizeof(annot_t) + strlen(name));
					strcpy(annot->name, name);
					annot->attrib = (attrib_t)0;
					annot->next = annotlist;
					annotlist = annot;
				}
				*str = (char)i;

				/* merge new attributes with any previous */
				if (attrib & EG_COLOR)
					annot->attrib &= ~(EG_COLORMASK|EG_COLOR);
				annot->attrib |= attrib;

				/* reset for next name */
				name = NULL;
			}

			/* start of a name? */
			if (!name && isalnum(*str))
				name = str;

		} while (*str);
	}

	/* close the file*/
	fclose(fp);

	/* if there's an "all" annotation, merge its attributes */
	all = findannot("elvgdb-all");
	if (all)
	{
		for (annot = annotlist; annot; annot = annot->next)
		{
			/* skip the pseudo-annotations */
			if (!strncmp(annot->name, "elvgdb-", 7))
				continue;

			/* merge the attributes */
			if (annot->attrib & EG_COLOR)
				annot->attrib |= all->attrib & ~EG_COLORMASK;
			else
				annot->attrib |= all->attrib;
		}
	}

	/* if there's a "default" annotation, then remember its attributes */
	annot = findannot("elvgdb-default");
	if (annot)
		defattrib = annot->attrib;

	/* if there's a "trace" annotation, then remember its attributes */
	annot = findannot("elvgdb-trace");
	if (annot)
		traceattrib = annot->attrib;
}

/* Fork gdb, with its stdout coming back to elvgdb through a pipe */
static FILE *opengdb(argc, argv)
	int	argc;	/* number of args */
	char	**argv;	/* values of the args */
{
	int	r0w1[2];	/* the pipe fd's */
	char	**newargv;	/* tweaked copy of args */
	int	i;

	/* create a pipe */
	if (pipe(r0w1) < 0)
		return NULL;

	/* fork gdb */
	switch (fork())
	{
	  case -1: /* error */
		close(r0w1[0]);
		close(r0w1[1]);
		return NULL;

	  case 0: /* child */
		/* close the read end of the pipe */
		close(r0w1[0]);

		/* redirect stdout/stderr to go to the write end of the pipe */
		close(1);
		dup(r0w1[1]);
		close(2);
		dup(r0w1[1]);
		close(r0w1[1]);

		/* create a copy of argv with "--annotate=2" inserted */
		newargv = (char **)malloc((argc + 2) * sizeof(char *));
		if (!newargv)
			return NULL;
		newargv[0] = "gdb";
		newargv[1] = "--annotate=2";
		for (i = 1; i < argc; i++)
			newargv[i + 1] = argv[i];
		newargv[argc + 1] = NULL;

		/* execute gdb */
		execvp("gdb", newargv);
		exit(1);
		/*NOTREACHED*/

	  default: /* parent */
		/* close the write end of the pipe */
		close(r0w1[1]);

		/* use buffering for the read end */
		return fdopen(r0w1[0], "r");
	}
}

/* close the pipe created by opengdb, and return gdb's exit code */
static int closegdb(fromgdb)
	FILE	*fromgdb;	/* pipe that was opened by opengdb */
{
	int	status;

	/* close the socket */
	fclose(fromgdb);

	/* get gdb's exit code */
	wait(&status);
	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	else
		return 1;
}


/* alter the highlighting for a given annotation attribute */
static void highlight(attrib)
	attrib_t	attrib;	/* attributes, including highlight info */
{
	static int highlighted;

	/* maybe end previous attributes */
	if (highlighted && !(attrib & EG_HIGHLIGHT))
	{
		printf("\033[m");
		highlighted = 0;
	}

	/* maybe start new attributes */
	if (attrib & (EG_COLOR|EG_BOLD|EG_UNDERLINE))
	{
		if (attrib & EG_COLOR)
			printf("\033[3%d%s%sm", attrib & EG_COLORMASK,
				attrib & EG_BOLD ? ";1" : "",
				attrib & EG_UNDERLINE ? ";4": "");
		else if (attrib & EG_BOLD)
			printf("\033[1%sm", attrib & EG_UNDERLINE ? ";4" : "");
		else
			printf("\033[4m");
		highlighted = 1;
	}
}

static attrib_t doannot(elviswin, name, args)
	Window	elviswin;/* where to send elvis commands */
	char	*name;	/* name of the annotation */
	char	*args;	/* any other text in the annotation itself */
{
	annot_t	*annot;
	attrib_t attrib;
	int	i;

	/* look up this annotation's attributes */
	annot = findannot(name);
	attrib = annot ? annot->attrib : defattrib;

	/* if supposed to guard against multiple messages, and this has been
	 * seen before, then don't start a message this time.
	 */
	if (attrib & EG_LIMIT)
	{
		if (annot->sent)
			attrib &= ~EG_FROMGDB;
		else
			annot->sent = 1;
	}

	/* maybe trace this annotation */
	if (attrib & EG_TRACE)
	{
		highlight(traceattrib);
		printf("[%s]", name);
	}

	/* maybe remember this annotation name for table records */
	if (attrib & EG_TABLE)
	{
		/* maybe trace */
		if (traceattrib & EG_TABLE)
		{
			highlight(traceattrib);
			printf("(TABLE-BEGIN)");
		}

		strcpy(tablename, name);
	}

	/* maybe start a new :fromgdb command */
	if (attrib & EG_FROMGDB)
	{
		/* maybe trace it */
		if (traceattrib & EG_FROMGDB)
		{
			highlight(traceattrib);
			printf("(fromgdb %s)", tablename);
		}

		if (secret)
			sprintf(toelvis, "\"%ld\"fromgdb %s", secret, name);
		else
			sprintf(toelvis, "fromgdb %s", name);
	}

	/* can't use "args" and "text" unless toelvis is filling up */
	if (!*toelvis)
		attrib &= ~(EG_ARGS | EG_TEXT);

	/* maybe append args to :toelvis command */
	if ((attrib & EG_ARGS) && *args)
	{
		strcat(toelvis, " ");
		strcat(toelvis, args);
	}

	/* maybe send :fromgdb command to elvis */
	if ((attrib & (EG_TOELVIS|EG_RECORD)) && *toelvis)
	{
		/* maybe trace the messages */
		if (traceattrib & EG_TOELVIS)
		{
			highlight(traceattrib);
			printf("<%s>", toelvis);
		}
			
		/* add a newline to mark the end of the command */
		strcat(toelvis, "\n");

		/* send the command to elvis */
		XChangeProperty(display, elviswin, elvis_server,
				XA_STRING, 8, PropModeAppend,
				(unsigned char *)toelvis, strlen(toelvis));

		/* clobber buf */
		*toelvis = '\0';
	}

	/* maybe start a new record */
	if ((attrib & EG_RECORD) && *tablename)
	{
		/* maybe trace it */
		if (traceattrib & EG_FROMGDB)
		{
			highlight(traceattrib);
			printf("(fromgdb %s)", tablename);
		}

		if (secret)
			sprintf(toelvis, "\"%ld\"fromgdb %s",secret, tablename);
		else
			sprintf(toelvis, "fromgdb %s", tablename);
	}


	/* if this is tablename with "-end", then clobber the tablename */
	i = strlen(tablename);
	if (!strncmp(name, tablename, i) && !strcmp(name + i, "-end"))
	{
		/* maybe trace */
		if (traceattrib & EG_TABLE)
		{
			highlight(traceattrib);
			printf("(TABLE-END)");
		}

		*tablename = '\0';
	}

	/* if this annotation has "server" attribute, and elvgdb-trace doesn't,
	 * then start watching for a "server " prefix on the command line.
	 */
	if ((attrib & EG_SERVER) && !(traceattrib & EG_SERVER))
		hideserver = "server ";

	/* we'll want a space before args of "text" */
	if ((attrib & EG_TEXT) && *toelvis)
	{
		buildtoelvis = toelvis + strlen(toelvis);
		if (buildtoelvis[-1] != ' ')
			*buildtoelvis++ = ' ';
	}

	/* update attributes for this annotation */
	highlight(attrib);

	/* maybe flush stuff too */
	if (attrib & EG_FLUSH)
	{
		/* flush commands to elvis */
		XFlush(display);

		/* reset the "sent" flags for any annotations */
		for (annot = annotlist; annot; annot = annot->next)
			annot->sent = 0;
	}

	/* The EG_FLUSH, EG_TEXT, and EG_SERVER attributes all affect the
	 * handling of text between annotations.
	 */
	return attrib;
}

/* Read & process an annotation line.  We assume the <newline><Ctrl-Z><Ctrl-Z>
 * sequence has already been read.  Return a boolean value indicating whether
 * text should be flushed for subsequent chars.
 */
static attrib_t annotation(fromgdb, elviswin)
	FILE	*fromgdb;	/* pipe that was opened by opengdb */
	Window	elviswin;	/* window of elvis server */
{
	char	name[100];
	char	args[1000];
	int	ch;
	unsigned i;

	/* read the name */
	for (i = 0;
	     i < sizeof name - 1 && (ch = getc(fromgdb)) != EOF && !isspace(ch);
	     i++)
		name[i] = ch;
	name[i] = '\0';

	/* if args, then read them too */
	if (ch != '\n' && ch != EOF)
	{
		for (i = 0;
		     i < sizeof args - 1 && (ch = getc(fromgdb)) != EOF
					 && ch != '\n';
		     i++)
			args[i] = ch;
		args[i] = '\0';
	}
	else
		args[0] = '\0';

	/* convert "field n" to "fieldn" */
	if (!strcmp(name, "field") && isdigit(args[0]))
	{
		strcat(name, args);
		*args = '\0';
	}

	/* if not end of line yet, then keep reading */
	while  (ch != EOF && ch != '\n')
		ch = getc(fromgdb);

	/* handle it */
	return doannot(elviswin, name, args);
}

/* write a character to stdout */
static void echoch(attrib, ch)
	attrib_t attrib;/* attributes, especially EG_FLUSH */
	int	ch;	/* character to write */
{
	/* detect "server" command prefix */
	if (hideserver != NULL && *hideserver != '\0')
	{
		if (ch == *hideserver)
		{
			hideserver++;
			if (*hideserver == '\0')
			{
				fputs("\b\b\b\b\b\b      \b\b\b\b\b\b", stdout);
			}
		}
		else
		{
			hideserver = NULL;
		}
	}

	/* echo the char to the user */
	if (hideserver == NULL || *hideserver != '\0')
	{
		putchar(ch);

		/* flush immediately, if prompting from user */
		if ((attrib & EG_FLUSH) || ch == '\n')
			fflush(stdout);
	}

	/* maybe add to :fromgdb command */
	if ((attrib & EG_TEXT) && buildtoelvis < &toelvis[sizeof toelvis + 2])
	{
		if (isgraph(ch))
			*buildtoelvis++ = ch;
		else if (buildtoelvis[-1] != ' ')
			*buildtoelvis++ = ' ';
		*buildtoelvis = '\0';
	}
}

/* run gdb, and watch for annotations in its output */
static int rungdb(argc, argv)
	int	argc;	/* number of args */
	char	**argv;	/* values of the args */
{
	FILE	*fromgdb;
	attrib_t attrib = (attrib_t)0;
	int	ch;
	char	*str;
	Atom		type;
	unsigned long	ul, dummy;
	int		format;
	unsigned char	*data;
	Window		elviswin;

	/* open a connection to the X server */
	openx();

	/* find the elvis server's window */
	XGetWindowProperty(display, root, elvis_server, 0L, 1L, False,
		XA_WINDOW, &type, &format, &ul, &dummy, &data);
	if (ul != 1 || type != XA_WINDOW || format != 32)
	{
		fprintf(stderr, "elvgdb: warning: can't find elvis\n");
		elviswin = None;
	}
	else
	{
		elviswin = *(Window *)data;
		XFree(data);
	}

	/* store elvgdb's xterm windowid as the elvgdb server attribute */
	str = getenv("WINDOWID");
	if (!str)
	{
		fprintf(stderr, "elvgdb: warning: -t only possible when run in an xterm (with $WINDOWID set)\n");
	}
	else
	{
		ul = strtol(str, NULL, 0);
		XChangeProperty(display, root, elvgdb_server, XA_WINDOW, 32,
			PropModeReplace, (unsigned char *)&ul, 1);
		XFlush(display);
	}

	/* did elvis share its secret code for bypassing security? */
	str = getenv("secret");
	if (str)
		secret = strtol(str, NULL, 0);

	/* load annotation attributes */
	str = getenv("ELVISPATH");
	if (!str)
		str = OSLIBPATH;
	str = searchpath(str, "elvgdb.ini");
	if (!str)
		fprintf(stderr, "elvgdb: Warning: could not find \"elvgdb.ini\" in $ELVISPATH\n");
	else
		loadannot(str);

	/* simulate an "elvgdb-init" annotation before gdb starts */
	doannot(elviswin, "elvgdb-init", "");

	/* if couldn't open gdb then fail */
	fromgdb = opengdb(argc, argv);
	if (!fromgdb)
	{
		doannot(elviswin, "elvgdb-term", "");
		perror("elvgdb");
		return 2;
	}

	/* ignore termios signals */
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	/* process gdb's output */
	while ((ch = getc(fromgdb)) != EOF)
	{
		/* watch for annotation lines */
		if (ch == '\n')	
		{
			if ((ch = getc(fromgdb)) == ELVCTRL('Z'))
			{
				if ((ch = getc(fromgdb)) == ELVCTRL('Z'))
				{
					attrib = annotation(fromgdb, elviswin);
					if (attrib & EG_FLUSH)
						fflush(stdout);
				}
				else
				{
					echoch(attrib, '\n'); 
					echoch(attrib, ELVCTRL('Z'));
					ungetc(ch, fromgdb);
				}
			}
			else
			{
				echoch(attrib, '\n'); 
				ungetc(ch, fromgdb);
			}
		}
		else
		{
			echoch(attrib, ch);
		}
	}

	/* get the exit status */
	format = closegdb(fromgdb);

	/* simulate a "elvgdb-term" annotation after gdb terminates */
	doannot(elviswin, "elvgdb-term", "");

	/* remove the ELVGDB_SERVER attribute from the root window */
	XDeleteProperty(display, root, elvgdb_server);
	XFlush(display);

	/* close the pipe, return gdb's exit code */
	return format;
}

static Window findgdb()
{
	Atom		type;
	unsigned long	ul, dummy;
	int		format;
	unsigned char	*data;
	Window		elvgdbwin, wroot, wparent, *wchild;
	unsigned int	nchild, scan, largest;
	unsigned long	scanpixels, largestpixels;
	XWindowAttributes wattr;

	/* find elvgdb's xterm  */
	XGetWindowProperty(display, root, elvgdb_server, 0L, 1L, False,
		XA_WINDOW, &type, &format, &ul, &dummy, &data);
	if (ul != 1 || type != XA_WINDOW || format != 32)
	{
		fprintf(stderr, "elvgdb: warning: can't find elvgdb\n");
		return None;
	}
	elvgdbwin = *(Window *)data;
	XFree(data);

	/* We actually need to paste into the vt100 widget within that window.
	 * Fetch the information about its children.
	 */
	if (!XQueryTree(display, elvgdbwin, &wroot, &wparent, &wchild, &nchild))
	{
		fprintf(stderr, "elvgdb: the elvgdb xterm seems weird -- no widgets\n");
		return None;
	}

	/* scan for the largest child window */
	for (largestpixels = 0, scan = 0; scan < nchild; scan++)
	{
		/* get info on this window */
		if (!XGetWindowAttributes(display, wchild[scan], &wattr))
			continue;	

		scanpixels = wattr.width * wattr.height;
		if (largestpixels == 0 || scanpixels > largestpixels)
		{
			largest = scan;
			largestpixels = scanpixels;
		}
	}
	if (largestpixels == 0)
	{
		fprintf(stderr, "elvgdb: the elvgdb xterm seems weird -- widgets are untouchable\n");
		wroot = None;
	}
	else
		wroot = wchild[largest];
	XFree(wchild);
	return wroot;
}

static int tellgdb(argc, argv)
	int	argc;	/* number of args */
	char	**argv;	/* values of the args */
{
	Window		gdbvt100, win;
	XEvent		event, notify;
	int		i, len;
	char		*buf;

	/* count the combined arg length */
	len = 1;
	i = 2;
	if (argv[1][2] != '\0')
		len += strlen(&argv[1][2]);
	if (len == 1 && i < argc)
		len = 0;
	for (; i < argc; i++)
		len += strlen(argv[i]) + 1;
	if (len == 1)
	{
		fprintf(stderr, "-t requires text\n");
		return 2;
	}

	/* combine the args, with spaces between them and a CR at end. */
	buf = (char *)malloc(len + 1);
	i = 2;
	buf[0] = '\0';
	if (argv[1][2] != '\0')
	{
		strcpy(buf, &argv[1][2]);
	}
	for (; i < argc; i++)
	{
		if (*buf)
			strcat(buf, " ");
		strcat(buf, argv[i]);
	}
	strcat(buf, "\r");
	len = strlen(buf);

	/* open a connect to the X server */
	openx();

	gdbvt100 = findgdb();
	if (gdbvt100 == None)
	{
		/* error message already given */
		return 2;
	}

	/* Create a window but don't map it.  We need a window that we can
	 * use to receive SelectionRequest events on.
	 */
	win = XCreateSimpleWindow(display, root, 0, 0, 1, 1, 0, 0L, 0L);

	/* claim the selection */
	XSetSelectionOwner(display, XA_PRIMARY, win, CurrentTime);
	if (XGetSelectionOwner(display, XA_PRIMARY) != win)
	{
		fprintf(stderr, "Could not claim X selection\n");
		return 2;
	}

	/* simulate a middle-click on elvgdb's xterm */
	memset(&event, 0, sizeof event);
	event.xbutton.type = ButtonPress;
	event.xbutton.display = display;
	event.xbutton.window = gdbvt100;
	event.xbutton.root = root;
	event.xbutton.subwindow = None;
	event.xbutton.state = Button2Mask|ShiftMask;
	event.xbutton.button = Button2;
	event.xbutton.same_screen = True;
	XSendEvent(display, gdbvt100, True, ButtonPressMask, &event);
	event.xbutton.type = ButtonRelease;
	event.xbutton.state = ShiftMask;
	XSendEvent(display, gdbvt100, True, ButtonReleaseMask, &event);

	/* wait for an event -- expect SelectionRequest */
	XNextEvent(display, &event);
	if (event.type != SelectionRequest)
	{
		fprintf(stderr, "Got event.type=%d instead of SelectionRequest\n", event.type);
		return 2;
	}

	/* fill in most fields of the SelectionNotify event */
	memset(&notify, 0, sizeof notify);
	notify.type = SelectionNotify;
	notify.xselection.requestor= event.xselectionrequest.requestor;
	notify.xselection.selection= event.xselectionrequest.selection;
	notify.xselection.target = event.xselectionrequest.target;
	notify.xselection.time = event.xselectionrequest.time;
	notify.xselection.property = event.xselectionrequest.property;

	/* store the selection's value into the property */
	XChangeProperty(display,
		event.xselectionrequest.requestor,
		event.xselectionrequest.property,
		event.xselectionrequest.target,
		8, PropModeReplace,
		(unsigned char *)buf, len);

	/* notify the requestor */
	XSendEvent(display, notify.xselection.requestor, False, 0L, &notify);
	XFlush(display);

	return 0;
}

int main(argc, argv)
	int	argc;	/* number of args */
	char	**argv;	/* values of the args */
{
	int	exitcode;

	if (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "?")))
	{
		printf("Usage: elvgdb [gdbargs]         run gdb, and communicate with elvis\n");
		printf("   or: elvgdb -t commandline    paste commandline into elvgdb's xterm\n");
		exitcode = 0;
	}
	else if (argc == 2 && (!strcmp(argv[1], "--version") || !strcmp(argv[1], "-v")))
	{
		printf("elvgdb (elvis) %s\n", VERSION);
		exitcode = 0;
	}
	else if (argc >= 2 && !strncmp(argv[1], "-t", 2))
	{
		exitcode = tellgdb(argc, argv);
	}
	else
	{
		exitcode = rungdb(argc, argv);
	}

	return exitcode;
}
