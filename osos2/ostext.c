/* osos2/ostext.c */

/*
 * Ported by Lee Johnson, fixes and emx/gcc compatibility by 
 * Martin "Herbert" Dietze.
 *
 * $Log: ostext.c,v $
 * Revision 1.6  2003/10/23 23:35:45  steve
 * Herbert's latest changes.
 *
 * Revision 1.5  2003/10/17 17:41:23  steve
 * Renamed the BOOLEAN data type to ELVBOOL to avoid name clashes with
 *   types defined other headers.
 *
 * Revision 1.4  2001/10/23 01:37:09  steve
 * Sometweaks of FEATURE_XXXX names
 *
 * Revision 1.3  2001/10/22 18:23:14  steve
 * Added FEATURE_RCSID compile-time option
 *
 * Revision 1.2  2001/04/20 00:00:37  steve
 * Some bug fixes, and uglification of the source code.
 *
 * Revision 1.2  2000/06/04 10:26:55  HERBERT
 * Some formatting and CVS Logging.
 *
 *
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <errno.h>
#if defined __EMX__ || defined __WATCOMC__
# define EACCESS EPERM
#endif
#include "elvis.h"
#ifdef FEATURE_RCSID
char id_ostext[] = "$Id: ostext.c,v 1.6 2003/10/23 23:35:45 steve Exp $";
#endif


/* This is the filedescriptor of the file being read */
static int fd;

/* Open a text file for reading (if rwa is 'r') or create/overwrite
 * a file for writing (if rwa is 'w') or appending (if rwa is 'a').
 * When overwriting an existing file, the file's original permissions
 * should be preserved.  Returns 0 if successful, -1 if no permission,
 * -2 if not a regular file (e.g., a directory), or -3 for other errors.
 */
int 
txtopen (char *filename, /* name of file */
         _char_ rwa,     /* 'r'=read, 'w'=write, 'a'=append */
        ELVBOOL  binary) /* open as binary file */
{
  assert (rwa == 'r' || rwa == 'w' || rwa == 'a');

  /* herbert:
   * binary mode handling looks ugly. Seems setting binary mode 
   * with creat () only works via setmode ().
   */
  switch (rwa)
    {
    case 'r': fd = open (filename, O_RDONLY|(binary? O_BINARY: 0));
      break;
    case 'w': fd = creat (filename, S_IREAD|S_IWRITE);        
      if (binary)
        {
          setmode (fd, O_BINARY);                        
        }
      break; 
    case 'a': fd = open (filename, O_WRONLY|O_APPEND|binary? O_BINARY: 0);        
      break;
    }
  if (fd < 0)
    {
      if (errno == EACCESS)
        {
          return -1;
        }
      else
        {
          return -3;
        }
    }
  return 0;
}

/* Close the file that was opened by txtopen(). */
void 
txtclose (void)
{
  close(fd);
}

/* Append text to a file which has been opened for writing.
 * Returns nbytes if successful, or 0 if the disk is full.
 * Should perform any necessary translations for converting
 * elvis' idea of text into the local OS's idea of text.
 */
int 
txtwrite (CHAR *buf,    /* buffer, holds text to be written */
         int nbytes)    /* number of characters to bewritten */
{
  return write (fd, buf, nbytes);
}

/* Read the next chunk of text from a file.  nbytes is the maximum
 * number to read.  Returns the number of characters actually read
 * after any conversions such as CRLF->LF translation.
 */
int 
txtread (CHAR  *buf,        /* buffer where text should be read into */
         int nbytes)        /* maximum number of bytes to read */
{
  return read (fd, buf, nbytes);
}
