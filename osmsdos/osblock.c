/* osmsdos/osblock.c */

#include <sys/types.h>
#include <dos.h>
#include <io.h>
#include <errno.h>
#include <fcntl.h>
#include "elvis.h"
#ifndef DEFAULT_SESSION
# define DEFAULT_SESSION "ELVIS%d.SES"
#endif

/* Microsoft seems to think that the goal of ANSI C's namespace pollution
 * conventions is to make the language incompatible with existing C code.
 */
#ifndef O_RDWR
# define O_RDWR		_O_RDWR
# define O_RDONLY	_O_RDONLY
# define O_WRONLY	_O_WRONLY
# define O_CREAT	_O_CREAT
# define O_EXCL		_O_EXCL
# define O_BINARY	_O_BINARY
#endif

static int fd = -1; /* file descriptor of the session file */

/* This function creates a new block file, and returns True if successful,
 * or False if failed because the file was already busy.
 */
BOOLEAN blkopen(BOOLEAN force, BLK *buf)
{
	char	sesname[100];
	char	*sespath;
	int	len, i;

	/* If no session file was explicitly requested, use the default */
	if (!o_session)
	{
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
			msg(MSG_FATAL, "set \\$SESSIONPATH to a writable directory");
		}
		close(i);
		remove(sesname);

		/* create a unique session file in that directory */
		for (i = 1; i <= 999; i++)
		{
			sprintf(sesname + len, DEFAULT_SESSION, i);
			if (o_recovering)
			{
				if (access(sesname, 0) == 0)
					break;
			}
			else
			{
				if (access(sesname, 0) != 0)
					break;
			}	

			/* If the user wants to cancel, then fail */
			if (chosengui->poll && (*chosengui->poll)(False))
				return False;
		}
		if (i > 999)
		{
			msg(MSG_FATAL, "could not find default session file");
		}
		optpreset(o_session, CHARdup(toCHAR(sesname)), OPT_LOCK|OPT_FREE);
		o_tempsession = (BOOLEAN)!o_recovering;
	}

	/* Try to open the session file */
	fd = open(tochar8(o_session), O_RDWR|O_BINARY);
	if (fd < 0)
	{
		if (errno == ENOENT)
		{
			fd = open(o_session, O_RDWR|O_CREAT|O_EXCL|O_BINARY, 0600);
			if (fd >= 0)
			{
				if (write(fd, (char *)buf, (unsigned)o_blksize) < o_blksize)
				{
					close(fd);
					remove((char *)o_session);
					fd = -1;
					errno = ENOENT;
				}
				else
				{
					lseek(fd, 0L, 0);
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
		o_tempsession = False;
	}

	/* Read the first block & mark the session file as being "in use".
	 * If already marked as "in use" and !force, then fail.
	 */
	/* lockf(fd, LOCK, sizeof buf->super); */
	if (read(fd, buf, sizeof buf->super) != sizeof buf->super)
	{
		msg(MSG_FATAL, "blkopen's read failed");
	}
	if (buf->super.inuse && !force)
	{
		return False;
	}
	buf->super.inuse = 1;
	lseek(fd, 0L, 0);
	(void)write(fd, buf, sizeof buf->super);

	/* done! */
	return True;
}


/* This function closes the session file, given its handle */
void blkclose(BLK *buf)
{
	blkread(buf, 0);
	buf->super.inuse = 0L;
	blkwrite(buf, 0);
	close(fd);
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
	lseek(fd, (long)blkno * (long)o_blksize, 0);
	if (write(fd, buf, (unsigned)o_blksize) != o_blksize)
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
	lseek(fd, (long)blkno * o_blksize, 0);
	if (read(fd, buf, (unsigned)o_blksize) != o_blksize)
	{
		msg(MSG_FATAL, "blkread failed");
	}
}

/* Force changes out to disk. */
void blksync P_((void))
{
	close(fd);
	fd = open(tochar8(o_session), O_RDWR|O_BINARY);
}
