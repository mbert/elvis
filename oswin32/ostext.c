/* oswin32/ostext.c */

#include "elvis.h"
#include <sys/types.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <errno.h>

/* the following is defined in oswin32\osdir.c */
extern char *dirnormalize(char *path);

#ifdef GUI_WIN32
#ifndef O_BINARY
#define O_BINARY	_O_BINARY
#endif
#ifndef O_TEXT
#define O_TEXT		_O_TEXT
#endif
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
	  case 'r': flags |= _O_RDONLY;				break;
	  case 'w': flags |= _O_WRONLY|_O_CREAT|_O_TRUNC;	break;
	  case 'a': flags |= _O_WRONLY|_O_APPEND;		break;
	}
	fd = _open(dirnormalize(filename), flags, 0666);

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
		_lseek(fd, 0L, 2);
#endif
}

/* Close the file that was opened by txtopen(). */
void txtclose(void)
{
	_close(fd);
}

/* Append text to a file which has been opened for writing.
 * Returns nbytes if successful, or 0 if the disk is full.
 * Should perform any necessary translations for converting
 * elvis' idea of text into the local OS's idea of text.
 */
int txtwrite(CHAR *buf, int nbytes)
{
	return _write(fd, buf, nbytes);
}

/* Read the next chunk of text from a file.  nbytes is the maximum
 * number to read.  Returns the number of characters actually read
 * after any conversions such as CRLF->LF translation.
 */
int txtread(CHAR *buf, int nbytes)
{
	return _read(fd, buf, nbytes);
}
