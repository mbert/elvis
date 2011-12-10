/* amiga.c */

/*-
 *	Mike Rieser 				Dale Rahn
 *	2410 Happy Hollow Rd. Apt D-10		540 Vine St.
 *	West Lafayette, IN 47906 		West Lafayette, IN 47906
 *	riesermc@mentor.cc.purdue.edu		rahn@sage.cc.purdue.edu
 */

#include <string.h>
#include <fcntl.h>
#include <exec/memory.h>
#include <dos/dostags.h>	/* for system() and rpipe() */
#include <dos/var.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#if AZTEC_C
#include <pragmas/exec_lib.h>
#include <pragmas/dos_lib.h>
#else
#include <pragmas/exec.h>
#include <pragmas/dos.h>
#endif

#include "config.h"

static struct TagItem systags[]=
{
    {SYS_Input, 0L},
    {SYS_Output, 0L},
    {SYS_UserShell, TRUE},
    {SYS_Asynch, TRUE},
    {TAG_DONE}
};

#define ENVSIZE 1028
#define FILE_SEP ' '

#ifdef LATTICE
/* INDENT OFF */
int  CXBRK(void) { return 0; }		/* Disable Lattice CTRL/C handling */
int  chkabort(void) { return 0; }
#elif AZTEC_C
long  Chk_Abort(void) { return 0; }	/* Disable Aztec CTRL/C handling */
/* INDENT ON */
#endif


/*
 * getpid() - This is only used to make tmpfiles.
 */
int
getpid()
{
    return (int) FindTask(0L);
}


/*
 * rpipe() - gets a `cmd' to run and a `file descriptor' to use as its stdin.
 */
int
rpipe(UBYTE * cmd, int fd)
{
    BPTR         fdFH = 0,	/* FileHandle of passed file descriptor */
                 outPH = 0,	/* Pipe (File) Handle for child to write to. */
                 inPH = 0,	/* Pipe (File) Handle for child to read from. */
                 lock = 0;
    int          fdr = 0;
    char         pname[32], *pc;
    extern char  o_shell[];

    if (isOldDOS())
	return -1;

    /*-
     * Sorry, I'm playing with an AZTEC internal here:
     * _devtab[fd].fd is Aztec's FileHandle for the file descriptor `fd'.
     * 
     * HINT: For your compiler, look in your compiler's fcntl.h.
     */

    switch (fd)
    {
    case 0:
	inPH = Open((UBYTE *) "*", MODE_READWRITE);
	break;
    default:

#ifdef	AZTEC_C
	fdFH = _devtab[fd].fd;		/* Grab FileHandle from fd */
#elif	_DCC
	fdFH = fdtofh(fd);		/* DCC does it right! */
#else
	return -1;			/* Sorry, can't help you. */
#endif

	/*
	 * Get a FileHandle to use for the child's stdin.
	 * The only reason we Dup is because we'll run the child ASynch,
	 * and it will close its Input and Output on exit.
	 */
	lock = DupLockFromFH(fdFH);
	if (!lock)
	    return -1;
	inPH = OpenFromLock(lock);
    }

    if (!inPH)
    {
	if (lock)
	    UnLock(lock);
	return -1;
    }

    /*
     * Get a pipe to use for the child's stdout, which we will read from.
     */
    strcpy(pname, "PIPE:ElvisXXX.XXX");
    pc = mktemp(pname);			/* Get a unique PIPE: */
    if (!*pc)
    {
    	Close(inPH);
	return -1;			/* Failure. */
    }

    /*
     * Get a FileHandle to use for the child's stdout.
     */
    if ((BPTR) 0 == (outPH = Open((UBYTE *) pc, MODE_NEWFILE)))
    {
	Close(inPH);
	return -1;			/* Failure. */
    }

    /* Get a file descriptor to return to the calling function */
    if ((fdr = open(pc, O_RDONLY)) < 0)
    {
	Close(inPH);
	Close(outPH);
	return -1;			/* Failure. */
    }

    /* exec the cmd */
    systags[0].ti_Data = inPH;		/* Input FileHandle for child */
    systags[1].ti_Data = outPH;		/* Output FileHandle for child */
    systags[2].ti_Data = (long) o_shell;/* which shell to use */

    if (System((UBYTE *) cmd, systags))
    {
	close(fdr);
	return -1;			/* Failure. */
    }

    return fdr;				/* Success! */
}


/*
 * This is supposed to wait till the child process is done. Unfortunately, I'm
 * not sure how to do that.
 */
int
rpclose(int fd)
{
    close(fd);
    return 0;
}


int
sleep(unsigned seconds)
{
    if (seconds) 
	 Delay(seconds * TICKS_PER_SECOND);
    return 0;
}


int
system(UBYTE * command)
{
    return (isOldDOS()) ? -1 : System((UBYTE *) command, TAG_DONE);
}


#ifdef AZTEC_C

/*
 * Aztec's library getenv() doesn't allow for Environment variables larger than
 * 256 bytes.  It also doesn't check the local environment, which would make
 * elvis running on an AUX: port more useful.
 */
char        *
getenv(char *var)
{
    static char *buf;

    buf = (char *) malloc(sizeof(*buf) * (ENVSIZE + 1));
    if ((char *) 0 == buf)
    {
	return 0;
    }
    if (isOldDOS())
    {
	int          bytes;
	BPTR         fh;

	strcpy(buf, "env:");
	strcat(buf, var);
	fh = Open((UBYTE *) buf, MODE_OLDFILE);
	if ((BPTR) 0 == fh)
	{
	    _free_(buf);
	    return (char *) 0;		/* return null for not defined */
	}
	bytes = Read(fh, (UBYTE *) buf, ENVSIZE);
	Close(fh);
	if (bytes == -1)
	{
	    _free_(buf);
	    return (char *) 0;		/* return null for not defined */
	}
	buf[bytes] = '\000';
    } else if (-1 == GetVar((UBYTE *) var, (UBYTE *) buf, ENVSIZE, GVF_BINARY_VAR))
    {					/* no varible defined, free memory */
	_free_(buf);
	return (char *) 0;		/* return null for not defined */
    }
    return (char *) buf;
}

#endif
