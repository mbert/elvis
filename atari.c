/* atari.c */

/* Author:
 *	Guntram Blohm
 *	Buchenstrasse 19
 *	7904 Erbach, West Germany
 *	Tel. ++49-7305-6997
 *	sorry - no regular network connection
 */

/*
 * This file contains the 'standard' functions which are not supported
 * by Atari/Mark Williams, and some other TOS-only requirements.
 */
 
#include "config.h"
#include "vi.h"

#if TOS
#include <osbind.h>

/* vi uses mode==0 only ... */
int access(file, mode)
	char *file;
{
	int fd=Fopen(file, 0);
	if (fd<0)
		return -1;
	Fclose(fd);
	return 0;
}

char *mktemp(template)
	char *template;
{
	return template;
}

#ifndef __GNUC__
char *getcwd(buf, size)
	char *buf;
{
	if (size < 2 + 64)
		return (char *)0;
	buf[0] = Dgetdrv() + 'A';
	buf[1] = ':';
	Dgetpath(buf + 2, 0);
	return buf;
}
#endif

/* read -- text mode, compress \r\n to \n
 * warning: might fail when maxlen==1 and at eol
 */

int tread(fd, buf, maxlen)
	int fd;
	char *buf;
	int maxlen;
{
	int i, j, nread=read(fd, buf, (unsigned)maxlen);

	if (nread && buf[nread-1]=='\r')
	{	nread--;
		lseek(fd, -1l, 1);
	}
	for (i=j=0; j<nread; i++,j++)
	{	if (buf[j]=='\r' && buf[j+1]=='\n')
			j++;
		buf[i]=buf[j];
	}
	return i;
}

int twrite(fd, buf, maxlen)
	int fd;
	char *buf;
	int maxlen;
{
	int i, j, nwritten=0, hadnl=0;
	char writbuf[BLKSIZE];

	for (i=j=0; j<maxlen; )
	{
		if ((writbuf[i++]=buf[j++])=='\n')
		{	writbuf[i-1]='\r';
			if (i<BLKSIZE)
				writbuf[i++]='\n';
			else
				hadnl=1;
		}
		if (i==BLKSIZE)
		{
			write(fd, writbuf, (unsigned)i);
			i=0;
		}
		if (hadnl)
		{
			writbuf[i++]='\n';
			hadnl=0;
		}
	}
	if (i)
		write(fd, writbuf, (unsigned)i);
	return j;
}


/* The "timer" variable is used as a shadow of the system's timer.  Since the
 * system's timer can only be accessed in Supervisor mode, we are forced to
 * do a Supexec(gettime) to copy the system's timer in to the User-mode "timer"
 * variable.
 */
static int timer;
static gettime()
{
	timer = *(long *)(0x4ba);
}

/* This function implements a read-with-timeout from the keyboard. */
/*ARGSUSED*/
int ttyread(buf, len, time)
	char	*buf;	/* where to store the gotten characters */
	int	len;	/* maximum number of characters to get -- ignored */
	int	time;	/* maximum time to allow for reading */
{
	int	pos=0;
	long	l;
	long	endtime;

	/* compute the ending time, in increments of 1/200th seconds */
	Supexec(gettime);
	endtime = time * 20 + timer;

	/* wait until time runs out, or we get a keystroke */
	while (!pos && (!time || timer < endtime))
	{
		if (Bconstat(2))
		{
			l = Bconin(2);
			buf[pos] = l;
			if (buf[pos++] == '\0')
			{
				buf[pos - 1] = '#';
				buf[pos++] = l >> 16;
			}
		}
		Supexec(gettime);
	}
	return pos;
}

/* This function writes characters to the screen */
ttywrite(buf, len)
	char *buf;
	int len;
{
	while (len--)
		Bconout(2, *buf++);
}
#endif
