/* optglob.c */
/* Copyright 1995 by Steve Kirkendall */

char id_optglob[] = "$Id: optglob.c,v 2.43 1996/10/01 19:46:55 steve Exp $";

/* This file contains gobal options for the portable parts of elvis. */

#include "elvis.h"

/* descriptions of the global options */
static OPTDESC ogdesc[] =
{
	{"blksize", "bsz",	optnstring,	optisnumber,	"256:8192" },
	{"blkhash", "hash",	optnstring,	optisnumber,	"1:500" },
	{"blkcache", "cache",	optnstring,	optisnumber,	"5:200" },
	{"blkgrow", "bgr",	optnstring,	optisnumber,	"1:32" },
	{"blkfill", "bfill",	optnstring,	optisnumber	},
	{"version", "ver",	optsstring,	optisstring	},
	{"bitsperchar", "bits",	optnstring,	optisnumber	},
	{"gui", "gui",		optsstring,	optisstring	},
	{"os", "os",		optsstring,	optisstring	},
	{"session", "ses",	optsstring,	optisstring	},
	{"recovering", "rflag", NULL,		NULL		},
	{"digraph", "dig",	NULL,		NULL		},
	{"exrc", "ex",		NULL,		NULL		},
	{"modeline","ml",	NULL,		NULL		},
	{"modelines","mls",	optnstring,	optisnumber,	"1:100"},
	{"ignorecase", "ic",	NULL,		NULL		},
	{"magic", "ma",		NULL,		NULL		},
	{"novice", "novice",	NULL,		NULL		},
	{"prompt", "prompt",	NULL,		NULL		},
	{"remap", "remap",	NULL,		NULL		},
	{"report", "report",	optnstring,	optisnumber	},
	{"shell", "sh",		optsstring,	optisstring	},
	{"sync", "sync",	NULL,		NULL		},
	{"taglength", "tl",	optnstring,	optisnumber	},
	{"tags", "tagpath",	optsstring,	optisstring	},
	{"tagstack", "tsk",	NULL,		NULL		},
	{"autoprint", "ap",	NULL,		NULL		},
	{"autowrite", "aw",	NULL,		NULL		},
	{"autoselect", "as",	NULL,		NULL,		},
	{"warn", "warn",	NULL,		NULL		},
	{"window", "wi",	optnstring,	optisnumber	},
	{"wrapscan", "ws",	NULL,		NULL		},
	{"writeany", "wa",	NULL,		NULL		},
	{"defaultreadonly","dro",NULL,		NULL		},
	{"initialstate", "is",	opt1string,	optisoneof,	"input replace vi ex"},
	{"exitcode", "exit",	optnstring,	optisnumber,	"0:255"},
	{"keytime", "kt",	optnstring,	optisnumber,	"0:20"},
	{"usertime", "ut",	optnstring,	optisnumber,	"0:20"},
	{"safer", "trapunsafe",	NULL,		NULL		},
	{"tempsession", "temp",	NULL,		NULL		},
	{"newsession", "temp",	NULL,		NULL		},
	{"exrefresh", "er",	NULL,		NULL		},
	{"home", "home",	optsstring,	optisstring	},
	{"elvispath", "epath",	optsstring,	optisstring	},
	{"terse", "te",		NULL,		NULL		},
	{"previousfile", "#",	optsstring,	optisstring	},
	{"previousfileline","@",optnstring,	optisnumber	},
	{"previouscommand", "!",optsstring,	optisstring	},
	{"previoustag", "ptag",	optsstring,	optisstring	},
	{"nearscroll", "ns",	optnstring,	optisnumber,	"0:100"},
	{"optimize", "op",	NULL,		NULL		},
	{"edcompatible", "ed",	NULL,		NULL		},
	{"pollfrequency", "pf",	optnstring,	optisnumber,	"1:1000"},
	{"sentenceend", "se",	optsstring,	optisstring	},
	{"sentencequote", "sq",	optsstring,	optisstring	},
	{"sentencegap",	"sg",	optnstring,	optisnumber,	"0:3"},
	{"verbose", "-v",	NULL,		NULL		},
	{"anyerror", "ae",	NULL,		NULL		},
	{"directory", "dir",	optsstring,	optisstring	},
	{"errorbells", "eb",	NULL,		NULL		},
	{"warningbells", "wb",	NULL,		NULL		},
	{"flash", "vbell",	NULL,		NULL		},
	{"program", "argv0",	optsstring,	optisstring	},
	{"backup", "bk",	NULL,		NULL		},
	{"showmarkups", "smu",	NULL,		NULL		},
	{"nonascii", "asc",	opt1string,	optisoneof,	"all most none strip"},
	{"beautify", "bf",	NULL,		NULL		},
	{"mesg", "mesg",	NULL,		NULL		},
	{"sessionpath", "spath",optsstring,	optisstring	},
	{"maptrace", "mt",	opt1string,	optisoneof,	"off run step"},
	{"maplog", "mlog",	opt1string,	optisoneof,	"off reset append"},
	{"gdefault", "gd",	NULL,		NULL		}
};


/* where the values are stored */
OPTVAL optglob[QTY_GLOBAL_OPTS];


/* printer options */
static OPTDESC lpdesc[] =
{
	{"lptype", "lpt",	optsstring,	optisstring	},
	{"lpcrlf", "lpc",	NULL,		NULL 		},
	{"lpout", "lpo",	optsstring,	optisstring	},
	{"lpcolumns", "lpcols",	optnstring,	optisnumber,	"0:300"},
	{"lpwrap", "lpw",	NULL,		NULL		},
	{"lplines", "lprows",	optnstring,	optisnumber,	"0:100"},
	{"lpconvert", "lpcvt",	NULL,		NULL		},
	{"lpformfeed", "lpff",	NULL,		NULL		},
	{"lppaper", "lpp",	optsstring,	optisstring	}
};

/* where the values are stored */
OPTVAL lpval[QTY_LP_OPTS];


#ifndef NO_USERVARS
/* descriptions of the user options */
static OPTDESC userdesc[] =
{
	{"a", "a",		optsstring,	optisstring	},
	{"b", "b",		optsstring,	optisstring	},
	{"c", "c",		optsstring,	optisstring	},
	{"d", "d",		optsstring,	optisstring	},
	{"e", "e",		optsstring,	optisstring	},
	{"f", "f",		optsstring,	optisstring	},
	{"g", "g",		optsstring,	optisstring	},
	{"h", "h",		optsstring,	optisstring	},
	{"i", "i",		optsstring,	optisstring	},
	{"j", "j",		optsstring,	optisstring	},
	{"k", "k",		optsstring,	optisstring	},
	{"l", "l",		optsstring,	optisstring	},
	{"m", "m",		optsstring,	optisstring	},
	{"n", "n",		optsstring,	optisstring	},
	{"o", "o",		optsstring,	optisstring	},
	{"p", "p",		optsstring,	optisstring	},
	{"q", "q",		optsstring,	optisstring	},
	{"r", "r",		optsstring,	optisstring	},
	{"s", "s",		optsstring,	optisstring	},
	{"t", "t",		optsstring,	optisstring	},
	{"u", "u",		optsstring,	optisstring	},
	{"v", "v",		optsstring,	optisstring	},
	{"w", "w",		optsstring,	optisstring	},
	{"x", "x",		optsstring,	optisstring	},
	{"y", "y",		optsstring,	optisstring	},
	{"z", "z",		optsstring,	optisstring	},
};

/* where the values are stored */
static OPTVAL optuser[QTY(userdesc)];
#endif /* not NO_USERVARS */


/* This function inializes the global options.  This can't be done in the
 * usual way, because the values are declared as a union, and C doesn't allow
 * unions to be initialized.
 */
void optglobinit()
{
	int	i;
	char	*envval;

	assert(QTY(ogdesc) == QTY_GLOBAL_OPTS);
	assert(QTY(lpdesc) == QTY_LP_OPTS);

	/* set each option to a reasonable default */
	optpreset(o_blksize, BLKSIZE, OPT_LOCK|OPT_HIDE);
	optpreset(o_blkhash, BLKHASH, OPT_HIDE);
	optpreset(o_blkcache, BLKCACHE, OPT_HIDE);
	optpreset(o_blkgrow, BLKGROW, OPT_HIDE);
	optflags(o_blkfill) = OPT_LOCK|OPT_HIDE;
	optpreset(o_version, toCHAR(VERSION), OPT_LOCK|OPT_HIDE);
	optpreset(o_bitsperchar, 8 * sizeof(CHAR), OPT_LOCK|OPT_HIDE);
	optpreset(o_os, toCHAR(OSNAME), OPT_LOCK|OPT_HIDE);
	optflags(o_gui) = OPT_LOCK|OPT_HIDE;
	optflags(o_session) = OPT_LOCK|OPT_HIDE;
	optflags(o_recovering) = OPT_HIDE;
	o_magic = True;
	o_prompt = True;
	o_autoprint = True;
	o_remap = True;
	o_report = 5;
	o_modelines = 5;
#ifdef OSSHELLENV
	optpreset(o_shell, toCHAR(getenv(OSSHELLENV)), OPT_HIDE);
#else
	optpreset(o_shell, toCHAR(getenv("SHELL")), OPT_HIDE);
#endif
	if (!o_shell)
		o_shell = toCHAR(OSSHELL);
	o_tagstack = True;
	o_warn = True;
	o_window = 12;
	o_wrapscan = True;
	optpreset(o_initialstate,'v', OPT_HIDE); /* vi */
	o_keytime = 3;
	o_usertime = 15;
	optflags(o_exitcode) = OPT_HIDE;
	optflags(o_safer) = OPT_HIDE;
	optflags(o_tempsession) = OPT_HIDE;
	optflags(o_newsession) = OPT_HIDE;
	optpreset(o_nearscroll, 10, OPT_HIDE);
	optflags(o_previousfile) = OPT_HIDE|OPT_LOCK;
	optflags(o_previousfileline) = OPT_HIDE|OPT_LOCK;
	optflags(o_previouscommand) = OPT_HIDE|OPT_LOCK;
	optflags(o_previoustag) = OPT_HIDE|OPT_LOCK;
	o_optimize = True;
	optpreset(o_pollfrequency, 20, OPT_HIDE);
	optflags(o_sentenceend) = OPT_HIDE;
	optflags(o_sentencequote) = OPT_HIDE;
	optpreset(o_sentencegap, 2, OPT_HIDE);
	if (!o_directory)
		o_directory = toCHAR(getenv("TMP"));
#ifdef OSDIRECTORY
	if (!o_directory)
		o_directory = toCHAR(OSDIRECTORY);
#endif
	o_errorbells = True;
	optpreset(o_nonascii, 'm', OPT_HIDE); /* most */
	optflags(o_digraph) = OPT_HIDE;
	optpreset(o_sync, False, OPT_HIDE);
	optflags(o_autoselect) = OPT_HIDE;
	optflags(o_defaultreadonly) = OPT_HIDE;
	optflags(o_exrefresh) = OPT_HIDE;
	optflags(o_verbose) = OPT_HIDE;
	optflags(o_anyerror) = OPT_HIDE;
	optflags(o_program) = OPT_HIDE;
	optpreset(o_mesg, True, OPT_HIDE);
	optpreset(o_maptrace, 'o', OPT_HIDE); /* off */
	optpreset(o_maplog, 'o', OPT_HIDE); /* off */

	envval = getenv("HOME");
	if (envval)
	{
		if (optflags(o_home) & OPT_FREE)
			safefree(o_home);
		o_home = toCHAR(envval);
	}
	else if (!o_home)
	{
		o_home = toCHAR(".");
	}
	optflags(o_home) |= OPT_HIDE;

	o_tags = toCHAR(getenv("TAGPATH"));
	if (!o_tags)
	{
		o_tags = toCHAR("tags");
	}

	/* Generate the default elvispath value. */
	envval = getenv("ELVISPATH");
	if (envval)
	{
		if (optflags(o_elvispath) & OPT_FREE)
			safefree(o_elvispath);
		o_elvispath = toCHAR(envval);
	}
	else if (!o_elvispath)
	{
		o_elvispath = toCHAR(OSLIBPATH);
	}
	optflags(o_elvispath) |= OPT_HIDE;

	/* Generate the default sessionpath value. */
	envval = getenv("SESSIONPATH");
	if (envval)
	{
		if (optflags(o_sessionpath) & OPT_FREE)
			safefree(o_sessionpath);
		o_sessionpath = toCHAR(envval);
	}
	else if (!o_sessionpath)
	{
#ifdef OSSESSIONPATH
		o_sessionpath = toCHAR(OSSESSIONPATH);
#else
		o_sessionpath = toCHAR("~:.");
#endif
	}
	optflags(o_sessionpath) |= (OPT_HIDE | OPT_LOCK);

	/* initialize the printing options */
	optpreset(o_lptype, toCHAR("dumb"), OPT_HIDE);
	optpreset(o_lpcrlf, False, OPT_HIDE);
	optpreset(o_lpout, toCHAR(OSLPOUT), OPT_HIDE);
	optpreset(o_lpcolumns, 80, OPT_HIDE);
	optpreset(o_lpwrap, True, OPT_HIDE);
	optpreset(o_lplines, 60, OPT_HIDE);
	optpreset(o_lpconvert, False, OPT_HIDE);
	optpreset(o_lpformfeed, False, OPT_HIDE);
	optpreset(o_lppaper, toCHAR("letter"), OPT_HIDE);

	/* inform the options code about these options */
	optinsert("global", QTY(ogdesc), ogdesc, optglob);
	optinsert("lp", QTY(lpdesc), lpdesc, lpval);

#ifndef NO_USERVARS
	/* initialize user variables */
	for (i = 0; i < QTY(userdesc); i++)
	{
		optflags(optuser[i]) = OPT_HIDE;
	}
	optinsert("user", QTY(userdesc), userdesc, optuser);
#endif
}


/* This function sets the "previousfile" and "previousfileline" options. */
void optprevfile(filename, line)
	CHAR	*filename;	/* new value for "previousfile" */
	long	line;		/* new value for "previousfileline" */
{
	/* if no new filename, then don't clobber the old one */
	if (!filename)
		return;

	/* if there was an old previousfile, then free its storage space */
	if (o_previousfile)
		safefree(o_previousfile);

	/* store the new filename and line number */
	o_previousfile = CHARdup(filename);
	o_previousfileline = line;
}
