/* oswin32/osblock.c */

#include <sys/types.h>
#include <dos.h>
#include <io.h>
#include <errno.h>
#include <fcntl.h>
#include "elvis.h"
#ifndef DEFAULT_SESSION
# define DEFAULT_SESSION "elvis%d.ses"
#endif


static int fd = -1; /* file descriptor of the session file */

/* This function creates a new block file, and returns ElvTrue if successful,
 * or ElvFalse if failed because the file was already busy.
 */
ELVBOOL blkopen(ELVBOOL force, BLK *buf)
{
	char	sesname[256];
	char	*sespath;
	int	len;
	int	i;

	/* search through SESSIONPATH for a writable directory */
	sespath = getenv("SESSIONPATH");
	if (!sespath)
		sespath = getenv("TMP");
	if (!sespath)
		sespath = "C:\\TMP;~;";
	do
	{
		for (len = 0; *sespath && *sespath != ';'; sespath++)
		{
			if (len == 0 && *sespath == '~')
			{
				strcpy(sesname, tochar8(o_home));
				len = strlen(sesname);
			}
			else
			{
				sesname[len++] = *sespath;
			}
		}
		if (len == 0)
		{
			strcpy(sesname, ".");
			len = 1;
		}
		if (sesname[len - 1] != '\\')
			sesname[len++] = '\\';
		strcpy(sesname + len, "elvis.tmp");
	} while ((i = open(sesname, O_RDWR|O_CREAT, 0666)) < 0 && *sespath++);
	if (i < 0)
	{
		msg(MSG_FATAL, "set SESSIONPATH to a writable directory");
	}
	close(i);
	remove(sesname);

	/* If no session file was explicitly requested, use the default */
	i = 1;
	if (!o_session)
	{
		/* protect against trying a ridiculous number of names */
		if (i >= 1000)
		{
			sesname[len] = '\0';
			msg(MSG_FATAL, o_recovering
				? "[s]no session file found in $1"
				: "[s]too many session files in $1", sesname);
		}

		if (o_recovering)
			do
			{
				sprintf(sesname + len, DEFAULT_SESSION, i++);

				/* If the user wants to cancel, then fail */
				if (chosengui->poll && (*chosengui->poll)(ElvFalse))
					return ElvFalse;
			} while (_access(sesname, 6) == -1 && errno == ENOENT);
		else
			do
			{
				sprintf(sesname + len, DEFAULT_SESSION, i++);

				/* If the user wants to cancel, then fail */
				if (chosengui->poll && (*chosengui->poll)(ElvFalse))
					return ElvFalse;
			} while (_access(sesname, 0) == 0);
		optpreset(o_session, CHARdup(toCHAR(sesname)), OPT_LOCK|OPT_FREE);
		o_tempsession = !o_recovering;
	}

	/* Try to open the session file */
	fd = _open(tochar8(o_session), _O_RDWR|_O_BINARY);
	if (fd < 0)
	{
		if (errno == ENOENT)
		{
			fd = _open(tochar8(o_session), _O_RDWR|_O_CREAT|_O_EXCL|_O_BINARY, 0600);
			if (fd >= 0)
			{
				if (_write(fd, (char *)buf, (unsigned)o_blksize) < o_blksize)
				{
					_close(fd);
					remove(tochar8(o_session));
					fd = -1;
					errno = ENOENT;
				}
				else
				{
					_lseek(fd, 0L, 0);
				}
			}
		}
		if (fd < 0)
		{
			msg(MSG_FATAL, "[Sd]$1: no such session, errno=$2", o_session, (long)errno);
		}
	}
	else
	{
		/* if the session existed before elvis, it'll exist after */
		o_tempsession = ElvFalse;
	}

	/* Read the first block & mark the session file as being "in use".
	 * If already marked as "in use" and !force, then fail.
	 */
	/* lockf(fd, LOCK, sizeof buf->super); */
	if (_read(fd, buf, sizeof buf->super) != sizeof buf->super)
	{
		msg(MSG_FATAL, "blkopen's read failed");
	}
	if (buf->super.inuse && !force)
	{
		return ElvFalse;
	}
	buf->super.inuse = 1;
	_lseek(fd, 0L, 0);
	(void)_write(fd, buf, sizeof buf->super);

	/* done! */
	return ElvTrue;
}


/* This function closes the session file, given its handle */
void blkclose(BLK *buf)
{
	blkread(buf, 0);
	buf->super.inuse = 0L;
	blkwrite(buf, 0);
	_close(fd);
	fd = -1;
	if (o_tempsession)
		remove(tochar8(o_session));
}

/* Write the contents of buf into record # blkno, for the block file
 * identified by blkhandle.  Blocks are numbered starting at 0.  The
 * requested block may be past the end of the file, in which case
 * this function is expected to extend the file.
 */
void blkwrite(BLK *buf, _BLKNO_ blkno)
{
	/* write the block */
	_lseek(fd, blkno * o_blksize, 0);
	if (_write(fd, buf, (unsigned)o_blksize) != o_blksize)
	{
		msg(MSG_FATAL, "blkwrite failed");
	}
}

/* Read the contends of record # blkno into buf, for the block file
 * identified by blkhandle.  The request block will always exist;
 * it will never be beyond the end of the file.
 */
void blkread(BLK *buf, _BLKNO_ blkno)
{
	/* read the block */
	_lseek(fd, blkno * o_blksize, 0);
	if (_read(fd, buf, (unsigned)o_blksize) != o_blksize)
	{
		msg(MSG_FATAL, "blkread failed");
	}
}

/* Force changes out to disk */
void blksync P_((void))
{
    int     dupfd;

    if ((dupfd = _dup (fd)) != -1)
        _close (dupfd);
}
