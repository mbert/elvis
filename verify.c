/* verify.c */
/* Copyright 1995 by Steve Kirkendall */

char id_verify[] = "$Id: verify.c,v 1.5 1996/06/28 03:39:18 steve Exp $";

/* This file contains a replacement for elvis' main() function.  The resulting
 * program will test each component of elvis.
 */

#include <stdio.h>
#include "elvis.h"

static BLK blkbuf;

#ifdef USE_PROTOTYPES
extern int main(int argc, char **argv);
static void init(void);
extern BOOLEAN buffer(void);
extern BOOLEAN lowbuf(void);
extern BOOLEAN session(void);
extern BOOLEAN options(void);
extern BOOLEAN ostext(void);
extern BOOLEAN osblock(void);
extern BOOLEAN osdir(void);
extern BOOLEAN sesmacro(void);
extern WINDOW winofbuf(WINDOW win, BUFFER buf);
extern WINDOW winalloc(GUIWIN *gw, OPTVAL *gvals, BUFFER buf, long rows, long columns);
extern void msg(MSGIMP imp, char *terse, ...);
extern void drawchar(CHAR *cp, long qty, _char_ font, long offset);
extern void draw1ch(CHAR c, _char_ font, long offset);
extern char *perm2str(DIRPERM perm);
extern char *bool2str(BOOLEAN bool);
#endif /* USE_PROTOTYPES */

/*****************************************************************************
 * Start of stub functions
 */

char *bool2str(BOOLEAN bool)
{
	return (bool ? "TRUE" : "FALSE");
}

char *perm2str(DIRPERM perm)
{
	switch (perm)
	{
	  case DIR_INVALID:	return "DIR_INVALID";
	  case DIR_BADPATH:	return "DIR_BADPATH";
	  case DIR_NOTFILE:	return "DIR_NOTFILE";
	  case DIR_NEW:		return "DIR_NEW";
	  case DIR_UNREADABLE:	return "DIR_UNREADABLE";
	  case DIR_READONLY:	return "DIR_READONLY";
	  default:		return "DIR_READWRITE";
	}
}

void draw1ch(CHAR c, _char_ font, long offset)
{
	static char prevfont = 'n';

	if (prevfont != font)
	{
		switch (font)
		{
		  case 'b':	fputs("\033[1m", stdout);	break;
		  case 'u':	fputs("\033[4m", stdout);	break;
		  default:	fputs("\033[m", stdout);	break;
		}
		prevfont = font;
	}
	putchar(c);
}

void drawchar(CHAR *cp, long qty, _char_ font, long offset)
{
	long	delta = 1;
	if (qty < 0)
	{
		qty = -qty;
		delta = 0;
	}
	while(--qty >= 0)
	{
		draw1ch(*cp, font, offset);
		cp += delta;
		offset += delta;
	}
}

void msg(MSGIMP imp, char *terse, ...)
{
	fputs(terse, stderr);
	if (imp == MSG_FATAL)
	{
		exit(2);
	}
}

void drawextext(WINDOW win, CHAR *text, int len){}
int wincount(BUFFER buf){ return 0; }
WINDOW windefault;
WINDOW windows;
WINDOW winalloc(GUIWIN *gw, OPTVAL *gvals, BUFFER buf, long rows, long columns)
{
	WINDOW	newp;

	/* allocate a window, and initialize it */
	newp = (WINDOW)safealloc(1, sizeof *newp);
	newp->next = windows;
	windows = newp;
	newp->gw = gw;
	newp->guivals = gvals;
	newp->cursor = markalloc(buf, 0);
	newp->wantcol = 0;

	/* initialize the options */
	optpreset(o_columns(newp), columns, OPT_SET|OPT_LOCK);
	optpreset(o_lines(newp), rows, OPT_SET|OPT_LOCK);
	optflags(o_display(newp)) = OPT_LOCK;
	o_scroll(newp) = rows / 2;
	o_ruler(newp) = o_showmode(newp) = True;

	/* no text is selected, initially */
	newp->seltop = newp->selbottom = NULL;

	return newp;
}

WINDOW winofbuf(WINDOW win, BUFFER buf)
{
	/* if "win" is NULL, then start searching at first window */
	if (!win)
	{
		win = windows;
	}
	else
	{
		win = win->next;
	}

	/* search for "buf", or NULL */
	while (win && buf && markbuffer(win->cursor) != buf)
	{
		win = win->next;
	}
	return win;
}


RESULT experform(WINDOW win, MARK top, MARK bottom)
{
	return RESULT_ERROR;
}
BOOLEAN exparseaddress(CHAR **refp, EXINFO *xinf)
{
	return False;
}
void exfree(EXINFO *xinf)
{
}
RESULT _viperform(WINDOW win)
{
	return RESULT_ERROR;
}

/*
 * End of stubs
 *****************************************************************************
 * Start of tests
 */

BOOLEAN sesmacro(void)
{ 
	BOOLEAN passed = True;

	if ((char *)&blkbuf.super.buf[SES_MAXSUPER] > &blkbuf.sizetester[o_blksize])
	{
		printf("\tSES_MAXSUPER = %ld\n", SES_MAXSUPER);
		passed = False;
	}
	if ((char *)&blkbuf.bufinfo.name[SES_MAXBUFINFO] > &blkbuf.sizetester[o_blksize])
	{
		printf("\tSES_MAXBUFINFO = %ld\n", SES_MAXBUFINFO);
		passed = False;
	}
	if ((char *)&blkbuf.blklist.blk[SES_MAXBLKLIST] > &blkbuf.sizetester[o_blksize])
	{
		printf("\tSES_MAXBLKLIST = %ld\n", SES_MAXBLKLIST);
		passed = False;
	}
	if ((char *)&blkbuf.chars.chars[SES_MAXCHARS] > &blkbuf.sizetester[o_blksize])
	{
		printf("\tSES_MAXCHARS = %ld\n", SES_MAXCHARS);
		passed = False;
	}

	return passed;
}

BOOLEAN osdir(void)
{
	BOOLEAN	passed = True;
	char	*tmp;

	tmp = dirpath("directory", "file");
	printf("\tdirpath(\"directory\", \"file\") = \"%s\"\n", tmp);
	printf("\tdirdir(\"%s\") = \"%s\"\n", tmp, dirdir(tmp));
	if (strcmp(dirdir(tmp), "directory"))
		passed = False;
	printf("\tdirfile(\"%s\") = \"%s\"\n", tmp, dirfile(tmp));
	if (strcmp(dirfile(tmp), "file"))
		passed = False;

	/* wildcard expansion */
	printf("\tdiriswild(\"*.c\") = %s\n", bool2str(diriswild("*.c")));
	tmp = dirfirst("*.c", False);
	printf("\tdirfirst(\"*.c\") = \"%s\"\n", tmp);
	while ((tmp = dirnext()) != NULL)
	{
		printf("\tdirnext() = \"%s\"\n", tmp);
	}
	printf("\tdirnext() = NULL\n");

	/* current working directory */
	printf("\tdircwd() = \"%s\"\n", dircwd());

	/* permissions */
	printf("\tdirperm(\"verify.c\") = %s\n", perm2str(dirperm("verify.c")));
	if (dirperm("verify.c") != DIR_READWRITE)
		passed = False;
	printf("\tdirperm(\"foo\") = %s\n", perm2str(dirperm("foo")));
	if (dirperm("foo") != DIR_NEW)
		passed = False;
	tmp = dircwd();
	printf("\tdirperm(\"%s\") = %s\n", tmp, perm2str(dirperm(tmp)));
	if (dirperm(tmp) != DIR_NOTFILE)
		passed = False;

	return passed;
}

BOOLEAN osblock(void)
{
	BOOLEAN	passed = True;

	blkbuf.super.magic = SESSION_MAGIC;
	blkbuf.super.blksize = sizeof(BLK);
	printf("\tblkopen(False, &blkbuf) = %s\n",
		bool2str(passed &= blkopen(False, &blkbuf)));
	blkclose(&blkbuf);
	printf("\tblkclose()\n");
	printf("\tblkopen(False, &blkbuf) = %s\n",
		bool2str(passed &= blkopen(False, &blkbuf)));
	memset(&blkbuf, 0, o_blksize);
	blkread(&blkbuf, 0);
	printf("\tblkbuf.super.magic = 0x%lx (expect %lx)\n",
		blkbuf.super.magic, SESSION_MAGIC);
	if (blkbuf.super.magic != SESSION_MAGIC)
		passed = False;
	printf("\tblkbuf.super.blksize = %d (expect %d)\n",
		blkbuf.super.blksize, sizeof(BLK));
	if (blkbuf.super.blksize != sizeof(BLK))
		passed = False;
	printf("\tblkbuf.super.inuse = %ld (expect non-0)\n",
		blkbuf.super.inuse);
	if (!blkbuf.super.inuse)
		passed = False;
	blkclose(&blkbuf);

	return passed;
}

BOOLEAN ostext(void)
{
	static CHAR	written[] = "line 1\n" "line 2\n" "line 3\n";
	CHAR		readback[2 * sizeof written];

	/* if "verify.txt" already exists, don't clobber it! */
	if (dirperm("verify.txt") != DIR_NEW)
	{
		printf("\t\"verify.txt\" already exists\n");
		return False;
	}

	/* test writing */
	if (txtopen("verify.txt", 'w', False))
	{
		printf("\ttxtopen(\"verify.txt\", 'w') failed\n");
		return False;
	}
	if (txtwrite(written, QTY(written) - 1) <= 0)
	{
		printf("\ttxtwrite() failed\n");
		txtclose();
		return False;
	}
	txtclose();

	/* readback, and compare */
	if (txtopen("verify.txt", 'r', False))
	{
		printf("\ttxtopen(\"verify.txt\", 'r') failed\n");
		return False;
	}
	if (txtread(readback, QTY(readback)) != QTY(written) - 1)
	{
		txtclose();
		printf("\ttxtread() returned short count\n");
		return False;
	}
	txtclose();
	if (memcmp(written, readback, (QTY(written) - 1) * sizeof(CHAR)))
	{
		printf("\tread text doesn't match written text.\n");
		return False;
	}

	/* test appending */
	if (txtopen("verify.txt", 'a', False))
	{
		printf("\ttxtopen(\"verify.txt\", 'a') failed\n");
		return False;
	}
	if (txtwrite(written, QTY(written) - 1) <= 0)
	{
		printf("\ttxtwrite() failed\n");
		txtclose();
		return False;
	}
	txtclose();

	/* readback, and compare */
	if (txtopen("verify.txt", 'r', False))
	{
		printf("\ttxtopen(\"verify.txt\", 'r') failed\n");
		return False;
	}
	if (txtread(readback, QTY(readback)) != 2 * (QTY(written) - 1))
	{
		txtclose();
		printf("\ttxtread() returned short count, after appending\n");
		return False;
	}
	txtclose();
	if (memcmp(written, readback, (QTY(written) - 1) * sizeof(CHAR))
	 || memcmp(written, readback + QTY(written) - 1, (QTY(written) - 1) * sizeof(CHAR)))
	{
		printf("\tread text doesn't match written text, after appending\n");
		return False;
	}

	remove("verify.txt");
	return True;
}

BOOLEAN options(void)
{
	BOOLEAN	passed = True;
	CHAR	outbuf[100];
	char	argbuf[20];

	/* boolean testing */
#if 0
	if (!optset(False, strcpy(argbuf, "warn"), outbuf, sizeof(outbuf))
	 || !o_warn
	 || !optset(False, strcpy(argbuf, "warn?"), outbuf, sizeof(outbuf))
	 || strcmp(outbuf, "warn\n")
	 || !optset(False, strcpy(argbuf, "nowarn"), outbuf, sizeof(outbuf))
	 || o_warn
	 || !optset(False, strcpy(argbuf, "warn?"), outbuf, sizeof(outbuf))
	 || strcmp(outbuf, "nowarn\n")
	 || (optflags(o_warn) & OPT_SET) == 0)
	{
		printf("\ttrouble with boolean options\n", argbuf, outbuf);
		passed = False;
	}
#else
	assert(optset(False, strcpy(argbuf, "warn"), outbuf, sizeof(outbuf)));
	assert(o_warn);
	assert(optset(False, strcpy(argbuf, "warn?"), outbuf, sizeof(outbuf)));
	assert(!strcmp(outbuf, "warn\n"));
	assert(optset(False, strcpy(argbuf, "nowarn"), outbuf, sizeof(outbuf)));
	assert(!o_warn);
	assert(optset(False, strcpy(argbuf, "warn?"), outbuf, sizeof(outbuf)));
	assert(!strcmp(outbuf, "nowarn\n"));
	assert((optflags(o_warn) & OPT_SET) != 0);
#endif

	/* string testing */
	if (!optset(False, (CHAR *)strcpy(argbuf, "sh=csh"), outbuf, sizeof(outbuf))
	 || !o_shell
	 || strcmp(tochar8(o_shell), "csh")
	 || !optset(False, (CHAR *)strcpy(argbuf, "shell"), outbuf, sizeof(outbuf))
	 || !strcmp(tochar8(outbuf), "shell=sh\n")
	 || (optflags(o_shell) & OPT_SET) == 0)
	{
		printf("\ttrouble with string options\n");
		passed = False;
	}

	/* number testing */
	if (!optset(False, (CHAR *)strcpy(argbuf, "window=17"), outbuf, sizeof(outbuf))
	 || o_window != 17
	 || !optset(False, (CHAR *)strcpy(argbuf, "window"), outbuf, sizeof(outbuf))
	 || strcmp(tochar8(outbuf), "window=17\n")
	 || (optflags(o_window) & OPT_SET) == 0)
	{
		printf("\ttrouble with number options\n");
		passed = False;
	}

	return passed;
}

BOOLEAN session(void)
{
	BOOLEAN	passed = True;
	BLK	*super;
	BLKNO	blkno1, blkno2;

	/* open the session file */
	sesopen(False);

	/* see if we can read the superblock */
	blkno1 = seslock(0, True, SES_SUPER);
	if (blkno1 != 0)
	{
		printf("\tseslock(0, ...) returned %d (expect 0)\n", blkno1);
		passed = False;
	}
	super = sesblk(blkno1);
	if (super->super.magic != SESSION_MAGIC)
	{
		printf("\tbad magic\n");
		passed = False;
	}
	sesunlock(0, True);

	/* allocate another block */
	blkno1 = sesalloc(0);
	if (blkno1 != 1)
	{
		printf("\tbad allocation of first block -- got %d (expect 1)\n", blkno1);
		passed = False;
	}
	blkno1 = seslock(blkno1, True, SES_CHARS);
	if (blkno1 != 1)
	{
		printf("\tcopy-on-write when not necessary -- got %d (expect 1)\n", blkno1);
		passed = False;
	}
	sesunlock(blkno1, False);
	blkno2 = sesalloc(blkno1);
	if (blkno2 != 1)
	{
		printf("\tsesalloc of block 1 failed -- got %d (expect 1)\n", blkno2);
		passed = False;
	}
	blkno2 = seslock(blkno2, True, SES_CHARS);
	if (blkno2 != 2)
	{
		printf("\tcopy-on-write when necessary -- got %d (expect 2)\n", blkno2);
		passed = False;
	}
	sesunlock(blkno2, False);

	sesclose();
	remove("verify.elv");
	return passed;
}


BOOLEAN lowbuf(void)
{
	BOOLEAN	passed = True;
	BLKNO	first, second, third, duped;
	char	text[20];
	long	off;
	int	i;

	sesopen(False);

	/* create/destroy/dup test */
	first = lowalloc("first");
	if (first != 1) passed = False;
	second = lowalloc("second");
	if (second != 2) passed = False;
	third = lowalloc("third");
	if (third != 3) passed = False;
	lowfree(second);
	duped = lowdup(first);
	if (duped != 2) passed = False;

	/* insertion test */
	for (i = 0, off = 0; i < 10000; i++)
	{
		sprintf(text, "line A%d\n", i);
		if (lowinsert(third, off, (CHAR *)text, strlen(text)) != 1) passed = False;
		off += strlen(text);
		sessync();
	}
	for (i = 0; i < 300; i++)
	{
		sprintf(text, "line B%d\n", i);
		if (lowinsert(third, 0, (CHAR *)text, strlen(text)) != 1) passed = False;
		off += strlen(text);
		sessync();
	}
	for (i = 0; i < 300; i++)
	{
		sprintf(text, "line C%d\n", i);
		if (lowinsert(third, o_blkfill, (CHAR *)text, strlen(text)) != 1) passed = False;
		off += strlen(text);
		sessync();
	}

	/* paste test */
	printf("lowpaste(first, 0, third, 0, %ld) = %ld\n", o_blksize * 2,
		lowpaste(first, 0, third, 0, o_blksize * 2));

	/* deletion test */
	printf("lowdelete(third, %ld, %ld) = %ld\n", o_blksize / 2, o_blksize * 5 / 2,
		lowdelete(third, o_blksize / 2, o_blksize * 5 / 2));

	sesclose();
	return passed;
}

BOOLEAN buffer(void)
{
	BUFFER	buf;
	WINDOW	win;
	BOOLEAN	passed = True;
	MARKBUF	mark;
	int	i;

	o_session = toCHAR("verify.ses");
#if 1
	bufinit();
#else
	sesopen(False);
#endif
	buf = bufalloc(toCHAR("buffer"), 0);
	(void)marktmp(mark, buf, 0);
	for (i = 0; i < 200; i++)
	{
		bufreplace(&mark, &mark, toCHAR("This is a test\n"), 15);
	}
	if (o_buflines(buf) != 200)
	{
		printf("o_buflines(buf) = %ld (expect 200)\n", o_buflines(buf));
		passed = False;
	}
	if (o_bufchars(buf) != 3000)
	{
		printf("o_bufchars(buf) = %ld (expect 3000)\n", o_bufchars(buf));
		passed = False;
	}
	win = winalloc(NULL, NULL, buf, 24, 80);
	while (markoffset(&mark) < o_bufchars(buf))
	{
		dmhex.image(win, &mark, (ELVISSTATE)0, drawchar);
	}
	sesclose();
	return passed;
}

/*
 * End of tests
 *****************************************************************************
 * Start of control functions
 */

struct
{
	BOOLEAN (*func)(void);	/* the testing function - TRUE if passed */
	char	*label;		/* label of the test */
	char	*desc;		/* description of the test */
} test[] =
{
	{ sesmacro,	"sesmacro",	"Macros in session.h"		},
	{ osdir,	"osdir",	"OS-dependent directory I/O"	},
	{ osblock,	"osblock",	"OS-dependent session file I/O"	},
	{ ostext,	"ostext",	"OS-dependent text file I/O"	},
	{ options,	"options",	"Options"			},
	{ session,	"session",	"low-level session file I/O"	},
	{ lowbuf,	"lowbuf",	"low-level buffer management"	},
	{ buffer,	"buffer",	"high-level buffer management"	},

	{ (BOOLEAN (*)(void))0 }
};


static void init(void)
{
	setbuf(stdout, NULL);
#ifdef OSINIT
	osinit();
#endif
	optglobinit();
}

int main(int argc, char **argv)
{
	int	i, j;
	BOOLEAN	passed;

	/* initialization */
	init();

	/* for each test... */
	for (i = 0; test[i].func; i++)
	{
		/* skip if not in explicit list */
		if (argc > 1)
		{
			for (j = 1; j < argc && strcmp(argv[j], test[i].label); j++)
			{
			}
			if (j == argc)
			{
				continue;
			}
		}

		/* perform the test */
		passed = (*test[i].func)();

		/* report the results */
		fprintf(stderr, "%s %-10s %s\n", passed ? "PASSED" : "FAILED", test[i].label, test[i].desc);
	}

	exit(0);
	/*NOTREACHED*/
}
