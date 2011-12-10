/* unix/osblock.c */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#ifndef S_ISDIR
# define S_ISDIR(mode)	(((mode & 0170000) == 0040000)
#endif
#include "elvis.h"
#ifndef DEFAULT_SESSION
# define DEFAULT_SESSION "%s/elvis%d.ses"
#endif
#ifndef F_OK
# define F_OK	0
#endif
#ifndef W_OK
# define W_OK	2
#endif

char id_osblock[] = "$Id: osblock.c,v 2.17 1996/10/01 19:48:23 steve Exp $";

static int fd = -1; /* file descriptor of the session file */

/* This function creates a new block file, and returns True if successful,
 * or False if failed because the file was already busy.
 */
BOOLEAN blkopen(force, buf)
	BOOLEAN	force;		/* if True, open even if "in use" flag set */
	BLK	*buf;		/* buffer, holds SUPER block */
{
 static char	dfltname[100];
	char	dir[80];
	struct stat st;
	int	i, j;

	/* If no session file was explicitly requested, try successive
	 * defaults until we find an existing file (if we're trying to
	 * recover) or a non-existent file (if we're not trying to recover).
	 */
	if (!o_session)
	{
		/* search through sessionpath for a writable directory */
		if (!o_sessionpath)
			o_sessionpath = toCHAR("~:.");
		for (i = 0, *dir = '\0'; o_sessionpath[i] && !*dir; )
		{
			/* copy next name from o_sessionpath to dfltname */
			j = 0;
			if (o_sessionpath[i] == '~' && !isalnum(o_sessionpath[i + 1]))
			{
				strcpy(dir, tochar8(o_home));
				j = strlen(dir);
				i++;
			}
			while (o_sessionpath[i] && o_sessionpath[i] != ':')
			{
				dir[j++] = o_sessionpath[i++];
			}
			dir[j] = '\0';
			if (j == 0)
				strcpy(dir, ".");
			if (o_sessionpath[i] == ':')
				i++;

			/* If not writable directory, forget it */
			if (stat(dir, &st) != 0
			 || !S_ISDIR(st.st_mode)
			 || !(st.st_uid == geteuid()
			 	? (st.st_mode & S_IRWXU) == S_IRWXU
			 	: st.st_gid == getegid()
			 		? (st.st_mode & S_IRWXG) == S_IRWXG
			 		: (st.st_mode & S_IRWXO) == S_IRWXO))
			{
				*dir = '\0';
			}
		}
		if (!*dir)
		{
			msg(MSG_FATAL, "set \\$SESSIONPATH to a writable directory");
		}
		optpreset(o_directory, CHARdup(toCHAR(dir)), OPT_FREE);

		/* choose the name of a session file */
		i = 1;
		do
		{
			sprintf(dfltname, DEFAULT_SESSION, dir, i++);

			/* if the file exists and is writable by this user,
			 * and we aren't recovering, then print a warning
			 * so the user know he should recover eventually.
			 */
			if (!o_recovering && access(dfltname, W_OK) == 0)
			{
				msg(MSG_WARNING, "[s]skipping old session file $1", dfltname);
			}

			/* if user wants to cancel, then fail */
			if (chosengui->poll && (*chosengui->poll)(False))
			{
				return False;
			}
		} while (access(dfltname, F_OK) != (o_recovering ? 0 : -1));
		o_session = toCHAR(dfltname);
		o_tempsession = True;
	}

	/* Try to open the session file */
	fd = open(tochar8(o_session), O_RDWR);
	if (fd >= 0)
	{
		/* we're opening an existing session -- definitely not temporary */
		o_tempsession = False;
	}
	else
	{
		if (errno == ENOENT)
		{
			fd = open(tochar8(o_session), O_RDWR|O_CREAT|O_EXCL, 0600);
			if (fd >= 0)
			{
				o_newsession = True;
				if (write(fd, (char *)buf, (unsigned)o_blksize) < o_blksize)
				{
					close(fd);
					unlink(tochar8(o_session));
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
			msg(MSG_FATAL, "no such session");
		}
	}

	/* if elvis runs other programs, they shouldn't inherit this fd */
	fcntl(fd, F_SETFL, 1);

	/* Read the first block & mark the session file as being "in use".
	 * If already marked as "in use" and !force, then fail.
	 */
	/* lockf(fd, LOCK, sizeof buf->super); */
	if (read(fd, (char *)buf, sizeof buf->super) != sizeof buf->super)
	{
		msg(MSG_FATAL, "blkopen's read failed");
	}
	if (buf->super.inuse && !force)
	{
		/* lockf(fd, ULOCK, o_blksize); */
		return False;
	}
	buf->super.inuse = getpid();
	lseek(fd, 0L, 0);
	(void)write(fd, (char *)buf, sizeof buf->super);
	/* lockf(fd, ULOCK, o_blksize); */

	/* done! */
	return True;
}


/* This function closes the session file, given its handle */
void blkclose(buf)
	BLK	*buf;	/* buffer, holds superblock */
{
	blkread(buf, 0);
	buf->super.inuse = 0L;
	blkwrite(buf, 0);
	close(fd);
	fd = -1;
	if (o_tempsession)
	{
		unlink(tochar8(o_session));
	}
}

/* Write the contents of buf into record # blkno, for the block file
 * identified by blkhandle.  Blocks are numbered starting at 0.  The
 * requested block may be past the end of the file, in which case
 * this function is expected to extend the file.
 */
void blkwrite(buf, blkno)
	BLK	*buf;	/* buffer, holds contents of block */
	_BLKNO_	blkno;	/* where to write it */
{
	/* write the block */
	lseek(fd, (off_t)blkno * (off_t)o_blksize, 0);
	if (write(fd, (char *)buf, (size_t)o_blksize) != o_blksize)
	{
		msg(MSG_FATAL, "blkwrite failed");
	}
}

/* Read the contends of record # blkno into buf, for the block file
 * identified by blkhandle.  The request block will always exist;
 * it will never be beyond the end of the file.
 */
void blkread(buf, blkno)
	BLK	*buf;	/* buffer, where buffer should be read into */
	_BLKNO_	blkno;	/* where to read from */
{
	/* read the block */
	lseek(fd, (off_t)blkno * o_blksize, 0);
	if (read(fd, (char *)buf, (size_t)o_blksize) != o_blksize)
	{
		msg(MSG_FATAL, "blkread failed");
	}
}

/* Force changes out to disk.  Ideally we would only force the session file's
 * blocks out to the disk, but UNIX doesn't offer a way to do that, so we
 * force them all out.  Major bummer.
 */
void blksync P_((void))
{
	sync();
}
