/* elvdump.c */
/* Copyright 1995 by Steve Kirkendall */


/* This file contains a replacement for elvis' main() function.  The resulting
 * program will test each component of elvis.
 */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_elvdump[] = "$Id: elvdump.c,v 1.11 2003/10/17 17:41:23 steve Exp $";
#endif

#ifdef FEATURE_COMPLETE
# error You must #undef FEATURE_COMPLETE in config.h to compile this program
#endif

extern void dump(char *bufname, ELVBOOL useronly);

/* support for elvis' ctype macros */
#ifdef ELVCT_DIGIT
CHAR elvct_class[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	ELVCT_DIGIT,ELVCT_DIGIT,ELVCT_DIGIT,ELVCT_DIGIT,ELVCT_DIGIT,
	ELVCT_DIGIT,ELVCT_DIGIT,ELVCT_DIGIT,ELVCT_DIGIT,ELVCT_DIGIT,
	0,0,0,0,0,0,0,
	ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,
	ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,
	ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,
	ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,
	ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,
	ELVCT_UPPER,
	0,0,0,0,0,0,
	ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,
	ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,
	ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,
	ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,
	ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,
	ELVCT_LOWER
};
#endif

static GUI nogui;
GUI *chosengui = &nogui;
GUI *gui = &nogui;

ELVBOOL guipoll(ELVBOOL reset)
{
	return ElvFalse;
}

void msg(MSGIMP imp, char *format, ...)
{
	fprintf(stderr, "%s\n", format);
}

CHAR *msgtranslate(char *msg)
{
	return toCHAR(msg);
}

ELVBOOL calcnumber(CHAR *arg)
{
	return ElvFalse;
}

ELVBOOL calctrue(CHAR *arg)
{
	return ElvFalse;
}

WINDOW windefault;
WINDOW winofbuf(WINDOW window, BUFFER buffer)
{
	return windefault;
}

void bufreplace(MARK from, MARK to, CHAR *newp, long newlen)
{
}

void dump(char *bufname, ELVBOOL useronly)
{
	BLK	*super;
	BLK	*bufinfo;
	BLK	*blklist;
	BLK	*chars;
	BLKNO	blkno, next;
	int	i, j;

	sesopen(ElvTrue);
	seslock(0, ElvFalse, SES_SUPER);
	super = sesblk(0);
	for (i = 0; i < SES_MAXSUPER; i++)
	{
		/* skip empty buffer slots */
		if (!super->super.buf[i])
		{
			continue;
		}

		/* read the bufinfo block */
		sesalloc(super->super.buf[i], SES_BUFINFO);
		seslock(super->super.buf[i], ElvFalse, SES_BUFINFO);
		bufinfo = sesblk(super->super.buf[i]);

		/* skip if "-u" and this isn't a user buffer */
		if (useronly && !strncmp(bufinfo->bufinfo.name, "Elvis ", 6))
		{
			sesunlock(super->super.buf[i], ElvFalse);
			continue;
		}

		/* output buffer name, unless we're supposed to list contents */
		if (!bufname)
			printf("%6d  bufinfo, bufname=\"%s\", changes=%ld\n",
				super->super.buf[i],
				bufinfo->bufinfo.name,
				bufinfo->bufinfo.changes);

		/* if "-u" and we don't have a buffer name, then we're done */
		if (useronly && !bufname)
		{
			sesunlock(super->super.buf[i], ElvFalse);
			continue;
		}

		/* for each blklist block... */
		for (blkno = bufinfo->bufinfo.first; blkno; blkno = next)
		{
			if (!bufname) printf("%6d      blklist for #%d\n", blkno, super->super.buf[i]);

			/* read the blklist block */
			sesalloc(blkno, SES_BLKLIST);
			seslock(blkno, ElvFalse, SES_BLKLIST);
			blklist = sesblk(blkno);

			/* for each chars block... */
			for (j = 0; j < SES_MAXBLKLIST && blklist->blklist.blk[j].blkno; j++)
			{
				if (!bufname) printf("%6d          chars #%d[%d], chars=%d, lines=%d\n",
					blklist->blklist.blk[j].blkno,
					blkno,
					j,
					blklist->blklist.blk[j].nchars,
					blklist->blklist.blk[j].nlines);
				if (bufname && !strcmp(bufname, bufinfo->bufinfo.name))
				{
					/* read the chars block & output its contents */
					sesalloc(blklist->blklist.blk[j].blkno, SES_CHARS);
					seslock(blklist->blklist.blk[j].blkno, ElvFalse, SES_CHARS);
					chars = sesblk(blklist->blklist.blk[j].blkno);
					fwrite(chars->chars.chars, blklist->blklist.blk[j].nchars, sizeof(CHAR), stdout);
					sesunlock(blklist->blklist.blk[j].blkno, ElvFalse);
				}
			}

			/* release the blklist block */
			next = blklist->blklist.next;
			sesunlock(blkno, ElvFalse);
		}

		/* release the bufinfo block */
		sesunlock(super->super.buf[i], ElvFalse);
	}
	sesunlock(0, ElvFalse);
	sesclose();
}

int main(int argc, char **argv)
{
	ELVBOOL useronly = ElvFalse;

	optglobinit();
	if (argc >= 2 && !strcmp(argv[1], "-u"))
	{
		useronly = ElvTrue;
		argv++;
		argc--;
	}
	if (argc == 1)
		dump(NULL, useronly);
	else if (argc == 2 && argv[1][0] != '-')
	{
		o_session = toCHAR(argv[1]);
		dump(NULL, useronly);
	}
	else if (argc == 3 && argv[1][0] != '-')
	{
		o_session = toCHAR(argv[1]);
		dump(argv[2], useronly);
	}
	else
	{
		printf("Usage: elvdump [-u] [session [buffer]]\n");
		printf("When invoked with a session file name and a buffer name, elvdump\n");
		printf("will write the text of all versions of that buffer to stdout.\n");
		printf("When invoked with just a session file name, it will describe\n");
		printf("the block usage for each buffer in the file (or, with -h, only\n");
		printf("the user buffers).  When invoked with no arguments, it will\n");
		printf("open an anonymous session file and describe its block usage.\n");

		exit(1);
	}
	exit(0);
}
