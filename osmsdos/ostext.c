/* osmsdos/ostext.c */

#include "elvis.h"
#include <sys/types.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <errno.h>

/* Microsoft has an annoying habit of adding underscores to the front of
 * conventional names.
 */
#ifndef O_WRONLY
# define O_WRONLY	_O_WRONLY
# define O_RDONLY	_O_RDONLY
# define O_RDWR		_O_RDWR
# define O_CREAT	_O_CREAT
# define O_EXCL		_O_EXCL
# define O_TRUNC	_O_TRUNC
# define O_APPEND	_O_APPEND
# define O_BINARY	_O_BINARY
# define O_TEXT		_O_TEXT
#endif

/* This is the fd of the open file */
static int	fd;


/* Open a text file for reading (if rwa is 'r') or create/overwrite
 * a file for writing (if rwa is 'w') or appending (if rwa is 'a').
 * When overwriting an existing file, the file's original permissions
 * should be preserved.  Returns 0 if successful, -1 if no permission,
 * -2 if not a regular file (e.g., a directory), or -3 for other errors.
 */
int txtopen(char *filename, _char_ rwa, ELVBOOL binary)
{
	int	flags;

	/* try to open the file */
	flags = (binary ? O_BINARY : O_TEXT);
	switch (rwa)
	{
	  case 'r': flags |= O_RDONLY;			break;
	  case 'w': flags |= O_WRONLY|O_CREAT|O_TRUNC;	break;
	  case 'a': flags |= O_WRONLY|O_APPEND;		break;
	}
	fd = open(filename, flags, 0666);

	/* return a code indicating the success or reason for failure */
	if (fd >= 0)
		return 0;
	else if (errno == EACCES)
		return -1;
	else if (errno == EISDIR)
		return -2;
	else
		return -3;
#if 0
	/* if supposed to append, then seek to the end of the file */
	if (rwa == 'a')
		lseek(fd, 0L, 2);
#endif
}

/* Close the file that was opened by txtopen(). */
void txtclose(void)
{
	close(fd);
}

/* Append text to a file which has been opened for writing.
 * Returns nbytes if successful, or 0 if the disk is full.
 * Should perform any necessary translations for converting
 * elvis' idea of text into the local OS's idea of text.
 */
int txtwrite(CHAR *buf, int nbytes)
{
	return write(fd, buf, nbytes);
}

/* Read the next chunk of text from a file.  nbytes is the maximum
 * number to read.  Returns the number of characters actually read
 * after any conversions such as CRLF->LF translation.
 */
int txtread(CHAR *buf, int nbytes)
{
	return read(fd, buf, nbytes);
}
