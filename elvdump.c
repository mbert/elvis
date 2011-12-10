/* elvdump.c */
/* Copyright 1995 by Steve Kirkendall */

char id_elvdump[] = "$Id: elvdump.c,v 1.5 1996/01/19 23:16:16 steve Exp $";

/* This file contains a replacement for elvis' main() function.  The resulting
 * program will test each component of elvis.
 */

#include "elvis.h"
extern void dump(char *bufname, BOOLEAN useronly);

void msg(MSGIMP imp, char *format, ...)
{
	fprintf(stderr, "%s\n", format);
}

BOOLEAN calcnumber(CHAR *arg)
{
	return False;
}

BOOLEAN calctrue(CHAR *arg)
{
	return False;
}

WINDOW windefault;
WINDOW winofbuf(WINDOW window, BUFFER buffer)
{
	return windefault;
}

void dump(char *bufname, BOOLEAN useronly)
{
	BLK	*super;
	BLK	*bufinfo;
	BLK	*blklist;
	BLK	*chars;
	BLKNO	blkno, next;
	int	i, j;

	sesopen(True);
	seslock(0, False, SES_SUPER);
	super = sesblk(0);
	for (i = 0; i < SES_MAXSUPER; i++)
	{
		/* skip empty buffer slots */
		if (!super->super.buf[i])
		{
			continue;
		}

		/* read the bufinfo block */
		sesalloc(super->super.buf[i]);
		seslock(super->super.buf[i], False, SES_BUFINFO);
		bufinfo = sesblk(super->super.buf[i]);

		/* skip if "-u" and this isn't a user buffer */
		if (useronly && !strncmp(bufinfo->bufinfo.name, "Elvis ", 6))
		{
			sesunlock(super->super.buf[i], False);
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
			sesunlock(super->super.buf[i], False);
			continue;
		}

		/* for each blklist block... */
		for (blkno = bufinfo->bufinfo.first; blkno; blkno = next)
		{
			if (!bufname) printf("%6d      blklist for #%d\n", blkno, super->super.buf[i]);

			/* read the blklist block */
			sesalloc(blkno);
			seslock(blkno, False, SES_BLKLIST);
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
					sesalloc(blklist->blklist.blk[j].blkno);
					seslock(blklist->blklist.blk[j].blkno, False, SES_CHARS);
					chars = sesblk(blklist->blklist.blk[j].blkno);
					fwrite(chars->chars.chars, blklist->blklist.blk[j].nchars, sizeof(CHAR), stdout);
					sesunlock(blklist->blklist.blk[j].blkno, False);
				}
			}

			/* release the blklist block */
			next = blklist->blklist.next;
			sesunlock(blkno, False);
		}

		/* release the bufinfo block */
		sesunlock(super->super.buf[i], False);
	}
	sesunlock(0, False);
	sesclose();
}

int main(int argc, char **argv)
{
	BOOLEAN useronly = False;

	optglobinit();
	if (argc >= 2 && !strcmp(argv[1], "-u"))
	{
		useronly = True;
		argv++;
		argc--;
	}
	if (argc == 1)
		dump(NULL, useronly);
	else if (argc == 2)
	{
		o_session = argv[1];
		dump(NULL, useronly);
	}
	else if (argc == 3)
	{
		o_session = argv[1];
		dump(argv[2], useronly);
	}
	else
	{
		fprintf(stderr, "usage: elvdump [-u] [session [buffer]]\n");
		exit(1);
	}
	exit(0);
}
