/* unix/ostext.c */

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "elvis.h"

char id_ostext[] = "$Id: ostext.c,v 2.4 1995/11/22 01:08:41 steve Exp $";

/* This is the filedescriptor of the file being read */
static int fd;

/* Open a text file for reading (if rwa is 'r') or create/overwrite
 * a file for writing (if rwa is 'w') or appending (if rwa is 'a').
 * When overwriting an existing file, the file's original permissions
 * should be preserved.  Returns 0 if successful, -1 if no permission,
 * -2 if not a regular file (e.g., a directory), or -3 for other errors.
 */
int txtopen(filename, rwa, binary)
	char	*filename;	/* name of file */
	_char_	rwa;		/* 'r'=read, 'w'=write, 'a'=append */
	BOOLEAN	binary;		/* (ignored) */
{
	assert(rwa == 'r' || rwa == 'w' || rwa == 'a');

	switch (rwa)
	{
	  case 'r': fd = open(filename, O_RDONLY);		break;
	  case 'w': fd = creat(filename, 0666);			break;
	  case 'a': fd = open(filename, O_WRONLY|O_APPEND);	break;
	}
	if (fd < 0)
	{
		if (errno == EPERM || EACCES)
			return -1;
		else if (errno == EISDIR)
			return -2;
		else
			return -3;
	}
	return 0;
}

/* Close the file that was opened by txtopen(). */
void txtclose()
{
	close(fd);
}

/* Append text to a file which has been opened for writing.
 * Returns nbytes if successful, or 0 if the disk is full.
 * Should perform any necessary translations for converting
 * elvis' idea of text into the local OS's idea of text.
 */
int txtwrite(buf, nbytes)
	CHAR	*buf;	/* buffer, holds text to be written */
	int	nbytes;	/* number of characters to bewritten */
{
	return write(fd, buf, (size_t)nbytes);
}

/* Read the next chunk of text from a file.  nbytes is the maximum
 * number to read.  Returns the number of characters actually read
 * after any conversions such as CRLF->LF translation.
 */
int txtread(buf, nbytes)
	CHAR	*buf;	/* buffer where text should be read into */
	int	nbytes;	/* maximum number of bytes to read */
{
	return read(fd, buf, (size_t)nbytes);
}
