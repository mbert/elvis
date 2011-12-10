/*
   VMSIO.C -- replacements for lseek(), read() and close() to allow
              arbitrary byte seeks with non-stream-lf files.
             
   (The original version (file routines) were written to port the
    unix ``less'' program.  This version helps with the elvis port.
    It may be useful elsewhere to port other utilities.)

   Written by John Campbell  CAMPBELL@NAUVAX.bitnet.   Use as you
   wish, but leave me some credit for killing myself on this when
   I was sick one weekend, ok?

   Also, added vms_rename as a weak replacment for link().  4/2/91
   ...and because there was a routine named delete, created a vms_delete.

   Sigh, Steve wanted pipe stuff, so vms_rpipe(), vms_pread() and
   vms_pclose() was born.                                   8/2/91

   Moved the tty i/o routines into this module as well.  vms_open_tty()
   and vms_ttyread()  (ttread)                              8/2/91
*/

/*
Entry points:

FILE I/O
vms_close (fd)
long vms_lseek (fd, offset, direction)
int vms_read (fd, buf, len)
int vms_rename (from, to)
vms_delete (file)

PIPE I/O
int vms_rpipe (cmd, fd, input_file)
int vms_pread (pfile, buffer, size)
int vms_rpclose(pfile)

TERMINAL I/O
vms_open_tty()
vms_ttyread(buf, len, time)
*/
static char *version = "VMSIO, version 1.0";

#include <stdio.h>
#include <errno.h>
#include <perror.h>
#define BUFSIZE 4096

/* Data and buffers used to implement vms_lseek() and vms_read() */
   static struct {
      int type, cur_loc, size, lastbin, maxbin, offset, eob;
   } fdints[_NFILE] =
      {{0,0,0,0,0,0,-1},{0,0,0,0,0,0,-1},{0,0,0,0,0,0,-1},{0,0,0,0,0,0,-1},
       {0,0,0,0,0,0,-1},{0,0,0,0,0,0,-1},{0,0,0,0,0,0,-1},{0,0,0,0,0,0,-1},
       {0,0,0,0,0,0,-1},{0,0,0,0,0,0,-1},{0,0,0,0,0,0,-1},{0,0,0,0,0,0,-1},
       {0,0,0,0,0,0,-1},{0,0,0,0,0,0,-1},{0,0,0,0,0,0,-1},{0,0,0,0,0,0,-1},
       {0,0,0,0,0,0,-1},{0,0,0,0,0,0,-1},{0,0,0,0,0,0,-1},{0,0,0,0,0,0,-1}};
   static char **fdbufs[_NFILE] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
typedef struct {
        int loc, bstart, bend;
} seeks;
   static seeks *fdseeks[_NFILE] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/* Intended use for fdseeks:  fdseeks[fd][i] for i'th triple */

#define G_cur_loc  fdints[fd].cur_loc
#define G_size     fdints[fd].size
#define G_eof_seen fdints[fd].eof_seen
#define G_offset   fdints[fd].offset
#define G_eob      fdints[fd].eob


vms_close (fd)
int fd;
{
   if (fd >= 0) {
   /* Reset fdints[fd] and free any buffers we were using. */
      fdints[fd].type    = 0;
      fdints[fd].cur_loc = 0;
      fdints[fd].size    = 0;
      fdints[fd].lastbin = 0;
      fdints[fd].maxbin  = 0;
      fdints[fd].offset  = 0;
      fdints[fd].eob     = -1;

      if (fdseeks[fd])
         fdseeks[fd] = free (fdseeks[fd]);
      if (fdbufs[fd])
         fdbufs[fd]  = free (fdbufs[fd]);
   }
   return (close(fd));
}

long vms_lseek (fd, offset, direction)
int fd, offset, direction;
{
   int tmp;

   if (fd > _NFILE) {
      fprintf (stderr,"Too many files for vms_lseek\n");
      exit(2);
   }
   if (fd < 0) {
      return lseek (fd, offset, direction);
   }
   if (fdints[fd].type == 0)
      _init(fd);
   if (fdints[fd].type == 1)
      return lseek (fd, offset, direction);

/* Convert the possibly relative `offset' to an absolute offset. */
   if (direction == 1) {
      offset = G_cur_loc + offset + G_offset;
   }
   else if (direction == 2) {
      if (G_eob == -1)
         _refill (fd, -1);   /* Figure out the correct eob byte count */
      offset = G_eob + offset;
   }
   if (offset < 0)
      return -1;

/* Limitation of this implementation--only seeks to end of file. */
   if (G_eob > -1 && offset > G_eob)
      offset = G_eob;

   if (G_offset > offset || offset >= G_offset + G_size) {
      if (_refill (fd, offset) == -1)
         return -1;
   }

/* Byte we want is now in our current buffer. */
   G_cur_loc = offset - G_offset;

/* Sanity check. */
   if (G_cur_loc < 0 || G_cur_loc > BUFSIZE)
      _error ("G_cur_loc error in vms_lseek\n", 1);

   return G_cur_loc + G_offset;   /* Byte location in current buffer */
}


int vms_read (fd, buf, len)
int fd, len;
char *buf;
{
/* Buffer the read as the unix system would do. */
   int  fill = 0, n, num = 0,tmp;
   char *buffer;

   if (fd > _NFILE) {
      fprintf (stderr,"Too many files for vms_read\n");
      exit(2);
   }

   if (fdints[fd].type == 0)
      _init(fd);
   if (fdints[fd].type == 1)
      return read (fd, buf, len);

   buffer = fdbufs[fd];

   if (G_eob != -1 && G_cur_loc + G_offset >= G_eob)
      return 0;   /* EOF */

/* Move any buffered data into place. */
   while (len && G_cur_loc < G_size)  {
      *buf++ = buffer[G_cur_loc++];
      --len;
      ++num;
   }
/* Refill as many buffers as necessary to put len bytes into buf. */
   do {
   /* Don't refill when asked to read beyond the end of the file. */
      if (G_cur_loc == G_size && (G_eob == -1 || G_offset + G_size < G_eob)) {
         if (_refill (fd, G_offset + G_size) == -1) return -1;
      }
      while (len && G_cur_loc < G_size)  {
         *buf++ = buffer[G_cur_loc++];
         --len;
         ++num;
      }
   } while (len && (G_eob == -1 || G_offset + G_size < G_eob));
/* Sanity check. */
   if (G_cur_loc < 0 || G_cur_loc > BUFSIZE)
      _error ("G_cur_loc bad in vms_read", 1);

   return num;
}

/*
   Fill from the appropriate buffer.  Note that offset is a true byte
   offset and needs to be converted to fdseeks[fd][i].loc before seeking.
   This technique may, in fact, generalize to other machines.
*/
static int _refill (fd, offset)
int fd, offset;
{
   int n, seekbin, tmp;
   char *buffer = fdbufs[fd];

/* See if the offset is in our current buffer (eob offset is a pain). */
   if (offset > 0 && G_eob > -1 && offset >= G_eob)
      offset = G_eob - 1;

   if (offset > 0 && G_offset <= offset && offset < G_offset + G_size)
      return G_offset;

   seekbin = fdints[fd].lastbin;

/* Now see if the offset is known (already been read once). */
   if (offset >= 0 && offset < fdseeks[fd][seekbin].bstart) {
   /* Yes!  We can seek to a known point to pick up this offset. */
      while (seekbin > 0) {
         if (offset >= fdseeks[fd][seekbin].bstart)
           break;
         --seekbin;
      }
   }
/* Position ourselves for the read (even if we are already there?) */
/* We can avoid this lseek if G_offset == [seekbin-1].bstart */
   if (lseek (fd, fdseeks[fd][seekbin].loc, 0) == -1)
      _error ("bad seek in _refill");

/* Ok, fill this buffer. */
more:
   G_cur_loc = 0;
   G_size = 0;
   G_offset = fdseeks[fd][seekbin].bstart;

/*
   Don't want to be left with a partial record.  Choices are to shrink
   down the read size or to keep seeking so we can backup to just before
   n == read size.  Which seems faster?  Which puts an arbitrary limit
   on the record size?
*/
   /* Some lines longer than 512 may be truncated. */
      while (G_size < BUFSIZE - 512 &&
                (n = read (fd, &buffer[G_size], BUFSIZE - G_size)) > 0) {
#ifdef OTHERMETHOD
         if (n == BUFSIZE - G_size)
            break;   /* Don't count last line (it may be a partial line) */
#endif
         G_size += n;
      }
      if (n == 0) {
         if (G_size == 0)
            ++G_size;
         G_eob = G_offset + G_size;
         offset = G_eob;
      }
      else if (n == -1) {
         _error ("vms_read (_refill) error", 0);
         return -1;
      }

/* Update the seek array.  Finish current bin and start next bin. */
   if (G_size == 0)
      ++G_size;
   tmp = G_offset + G_size - 1;
   if (seekbin < fdints[fd].lastbin && fdseeks[fd][seekbin].bend != tmp) {
      fprintf (stderr, "Consistency failure in _refill\n");
      exit(2);
   }
   fdseeks[fd][seekbin].bend = tmp;
   if (G_eob == -1 || G_offset + G_size < G_eob) {
   /* Set up the next seek bin */
      if (++seekbin > fdints[fd].maxbin) {
      /* Make more room. */
         fdints[fd].maxbin = 2*fdints[fd].maxbin;
         fdseeks[fd] = (void *)realloc (fdseeks[fd], fdints[fd].maxbin);
      }

   /* Make sure that lastbin is up to date. */
      if (seekbin > fdints[fd].lastbin) fdints[fd].lastbin = seekbin;

      if ((tmp = lseek(fd,0,1)) == -1)
         _error ("Seek error in vms_read (refill)");
      if (seekbin < fdints[fd].lastbin && fdseeks[fd][seekbin].loc != tmp) {
         fprintf (stderr, "Consistency failure in _refill\n");
         exit(2);
      }
      fdseeks[fd][seekbin].loc = tmp;

      if (seekbin >= fdints[fd].lastbin) {
         fdseeks[fd][seekbin].bstart = G_offset + G_size;
         fdseeks[fd][seekbin].bend = 0;
      }
      else {
      /* Consistency check */
         tmp = G_offset + G_size;
         if (fdseeks[fd][seekbin].bstart != tmp) {
            fprintf (stderr, "Consistency failure in _refill\n");
            exit(1);
         }
      }
   }
/* Well, it's either a big do-while loop or this goto... */
   if (offset == -1 ||
     (offset >= G_offset + G_size && (offset < G_eob || G_eob == -1)))
      goto more;

   return G_offset;
}

#include <types.h>
#include <stat.h>
/*
   Note: may want to store a pointer to the statbuf in place of type.
   For one thing, this would give you the maximum record size.

   Ok, there are some problems when we don't control open and close.
   For instance, switching fd's in midstream can throw us.  We could
   store a statbuf pointer to detect this and it may turn out to be
   necessary later.  (statbuf has the fid for the file in it.)
*/
static _init(fd)
int fd;
{
/* Check the file type; allocate a buffer. */
   struct stat statbuf;
   int tmp;

   if (fstat(fd, &statbuf) < 0)
      _error ("Can't stat\n", 1);
/*
   Some possible file types (from fab.h):

   rat: FTN - 0      rfm: UDF   0  undefined
        CR  - 1           FIX   1
        PRN - 2           VAR   2  default type
        BLK - 3           VFC   3
                          STM   4
                          STMLF 5
                          STMCR 6
Also, st_size == 0 seems to indicate a non-file device (mailbox, terminal).
*/
   if (statbuf.st_fab_rfm == 5 || statbuf.st_fab_rfm == 0
                                        || statbuf.st_size == 0)
      fdints[fd].type = 1;  /* Use 'C' rtl routines directly */
   else
      fdints[fd].type = 2;  /* Use these routines. */

/* Get a buffer for our buffered I/O */
   fdbufs[fd] = malloc (BUFSIZE);

/* Get an array to use for seeking (initial guess). */
   tmp = (statbuf.st_size + BUFSIZE)/BUFSIZE * sizeof(seeks) + sizeof(seeks);
   fdseeks[fd] = (void *)malloc (tmp);
   fdints[fd].maxbin = tmp/sizeof(seeks);

/* Fill in the 0'th (first) seeks triplet. */
   fdseeks[fd][0].loc    = 0;
   fdseeks[fd][0].bstart = 0;
   fdseeks[fd][0].bend   = 0;
}

#include <descrip.h>
static _error (string, leave)
char *string;
int leave;
{
   char errstr[81];
   $DESCRIPTOR(errdesc,errstr);
   short int length;

   perror (string);
   if (errno == EVMSERR) {
      if (SYS$GETMSG(vaxc$errno, &length, &errdesc, 1, 0) == 1) {
          errstr[length] = '\0';
          fprintf (stderr, "%s\n", errstr);
      }
   }
   if (leave)
      exit (44);  /* SS_ABORT */
}
#include <ssdef>
#include <iodef>
#include <rms>
#include <fibdef>
#include <atrdef>

/* Simply need a trick to hide a global called 'delete' elsewhere. */
vms_delete (file)
char *file;
{
   delete (file);
}

/* Pipe routines modified from Chris Janton's (chj) VMS Icon port. */
/*
   Here we fudge to help the "elvis" program implement rpipe.  The
   routine essentially does an popen using fd as stdin--except that
   few VMS utilities use the 'C' library.  So we pass in the standard
   input file name and use it if fd is non-zero.
*/
#include <dvidef>
#include <file>

typedef struct _descr {
   int length;
   char *ptr;
} descriptor;

typedef struct _pipe {
   long pid;                    /* process id of child */
   long status;                 /* exit status of child */
   long flags;                  /* LIB$SPAWN flags */
   int ichan;                 /* MBX channel number */
   int ochan;
   int efn;
   unsigned running : 1;        /* 1 if child is running */
} Pipe;

Pipe _pipes[_NFILE];            /* one for every open file */

#define NOWAIT          1
#define NOCLISYM        2
#define NOLOGNAM        4
#define NOKEYPAD        8
#define NOTIFY          16
#define NOCONTROL       32
#define SFLAGS  (NOWAIT|NOKEYPAD|NOCONTROL)

int vms_rpipe (cmd, fd, input_file)
char *cmd, *input_file;
int fd;
{
   int pfile;                   /* the Pfile */
   Pipe *pd;                    /* _pipe database */
   descriptor inmbxname;        /* name of input mailbox */
   descriptor outmbxname;          /* name of mailbox */
   char inmname[65];
   char outmname[65];              /* mailbox name string */
   int ochan;                    /* mailbox channel number */
   int ichan;                  /* Input mailbox channel number */
   int status;                  /* system service status */
   int efn;
   struct {
      short len;
      short code;
      char *address;
      char *retlen;
      int last;
   } itmlst;

   if (!cmd || cmd[0] == '\0')
      return (-1);
   LIB$GET_EF(&efn);
   if (efn == -1)
      return (-1);

   /* create and open the input mailbox */
   status = SYS$CREMBX(0, &ichan, 0, 0, 0, 0, 0);
   if (!(status & 1)) {
      LIB$FREE_EF(&efn);
      return (-1);
   }
   itmlst.last = inmbxname.length = 0;
   itmlst.address = inmbxname.ptr = inmname;
   itmlst.retlen = &inmbxname.length;
   itmlst.code = DVI$_DEVNAM;
   itmlst.len = 64;
   status = SYS$GETDVIW(0, ichan, 0, &itmlst, 0, 0, 0, 0);
   if (!(status & 1)) {
      LIB$FREE_EF(&efn);
      return (-1);
   }
   inmname[inmbxname.length] = '\0';

   /* create and open the output mailbox */
   status = SYS$CREMBX(0, &ochan, 0, 0, 0, 0, 0);
   if (!(status & 1)) {
      LIB$FREE_EF(&efn);
      return (-1);
   }
   itmlst.last = outmbxname.length = 0;
   itmlst.address = outmbxname.ptr = outmname;
   itmlst.retlen = &outmbxname.length;
   status = SYS$GETDVIW(0, ochan, 0, &itmlst, 0, 0, 0, 0);
   if (!(status & 1)) {
      LIB$FREE_EF(&efn);
      return (-1);
   }
   outmname[outmbxname.length] = '\0';
   pfile = open(outmname, O_RDONLY);
   if (pfile < 0) {
      LIB$FREE_EF(&efn);
      SYS$DASSGN(ichan);
      SYS$DASSGN(ochan);
      return (-1);
   }
   /* Save file information now */
   pd = &_pipes[pfile]; /* get Pipe pointer */
   pd->pid = pd->status = pd->running = 0;
   pd->flags = SFLAGS;
   pd->ichan = ichan;
   pd->ochan = ochan;
   pd->efn = efn;

   /* Initiate the command by writing down the input mailbox (SYS$INPUT). */
   if (fd > 0) {
      char *pre_command[132+12];
      strcpy (pre_command, "DEFINE/USER SYS$INPUT ");
      strcat (pre_command, input_file);
      status = sys$qiow(0, ichan, IO$_WRITEVBLK | IO$M_NOW, 0, 0, 0,
                        pre_command, strlen(pre_command), 0, 0, 0, 0);
      if (!(status & 1)) {
         LIB$FREE_EF(&efn);
         SYS$DASSGN(ichan);
         SYS$DASSGN(ochan);
         return (-1);
      }
   }
   status = sys$qiow(0, ichan, IO$_WRITEVBLK | IO$M_NOW,
                                0, 0, 0, cmd, strlen(cmd), 0, 0, 0, 0);
   if (!(status & 1)) {
      LIB$FREE_EF(&efn);
      SYS$DASSGN(ichan);
      SYS$DASSGN(ochan);
      return (-1);
   }
   status = sys$qiow(0, ichan, IO$_WRITEOF | IO$M_NOW, 0, 0, 0,
                                                          0, 0, 0, 0, 0, 0);
   if (!(status & 1)) {
      LIB$FREE_EF(&efn);
      SYS$DASSGN(ichan);
      SYS$DASSGN(ochan);
      return (-1);
   }

   status = LIB$SPAWN(0,
      &inmbxname,                          /* input file */
      &outmbxname,                         /* output file */
      &pd->flags, 0, &pd->pid, &pd->status, &pd->efn, 0, 0, 0, 0);
   if (!(status & 1)) {
      LIB$FREE_EF(&efn);
      SYS$DASSGN(ichan);
      SYS$DASSGN(ochan);
      return (-1);
   } else {
      pd->running = 1;
   }
   return (pfile);
}

int vms_pread (pfile, buffer, size)
int pfile, size;
char *buffer;
/* Be compatible when we read data in (handle newlines). */
{
   Pipe *pd;
   int status, request;
   struct {
      short  status,
             count;
      int       :16;
   } iosb;

   pd = pfile >= 0 ? &_pipes[pfile] : 0;
   if (pd == NULL) return -1;

/*
   This is sort of nasty.  The default mailbox size is 256 maxmsg
   and 1056 bufquo if your sysgen parameters are standard.  Asking
   for more on the CREMBX command (in rpipe) might be a bad idea as
   that could cause an "exceeded quota" error.  Since we only return
   -1 on error, there's no hope that the poor user would ever know
   what went wrong.
*/
   request = size > 256 ? 256 : size - 1;
   status = sys$qiow(0, pd->ochan, IO$_READVBLK, &iosb, 0, 0,
                                      buffer, request, 0, 0, 0, 0);
   if (!(status & 1)) return -1;
   if (iosb.status == SS$_ENDOFFILE)
      return 0;
   buffer[iosb.count] = '\n';
   return iosb.count+1;
}

/*
 * Taken from pclose - close a pipe
 * Last modified 2-Apr-86/chj
 *
 */
int vms_rpclose(pfile)
int pfile;
{
   Pipe *pd;
   int status;
   int fstatus;

   pd = pfile >= 0 ? &_pipes[pfile] : 0;
   if (pd == NULL)
      return (-1);
   fstatus = close(pfile);
   SYS$DASSGN(pd->ichan);
   SYS$DASSGN(pd->ochan);
   LIB$FREE_EF(&pd->efn);
   pd->running = 0;
   return (fstatus);
}

/* Terminal routines. */

static int tty;
static int VMS_term_chan;
int VMS_read_raw;   /* Set by curses.c in elvis. */

#define VMSCheck(a) {int _s; if (~(_s = (a)) & 1) vms_sys$exit (_s);}

vms_open_tty()
{
   int c;

   $DESCRIPTOR(_terminal,"SYS$INPUT");
   if (VMS_term_chan == 0) {
      VMSCheck(SYS$ASSIGN (&_terminal, &VMS_term_chan, 0, 0));
   }
/* Was 2 -- jdc */
   tty = 0;

/* Could reset the scrolling region, setup LINES, COLS, etc. */

   read(tty,&c,0);   /* Flush the tty buffer. */
}

/*
 * Get a character from the keyboard.
 */
/*ARGSUSED*/
vms_ttyread(buf, len, time)
        char *buf;
        int len;
        int time;
{
        char c;
        int result, terminat[2] = {0,0};
   struct {
      short int status;              /* I/O completion status */
      short int bcount;              /* byte transfer count   */
      int dev_dep_data;              /* device dependant data */
   } iosb;                  /* This is a QIO I/O Status Block */


   if (VMS_read_raw) {
/*      read(tty,&c,0);  */ /* Flush the tty buffer. (needed?) */
      if (time == 0) {
         VMSCheck(sys$qiow(0, VMS_term_chan,
           IO$_READVBLK | IO$M_NOECHO | IO$M_NOFILTR | IO$M_TRMNOECHO,
                 &iosb, 0, 0, buf, 1, 0, terminat, 0, 0));
      }
      else {
         VMSCheck(sys$qiow(0, VMS_term_chan,
        IO$_READVBLK | IO$M_NOECHO | IO$M_NOFILTR | IO$M_TRMNOECHO | IO$M_TIMED,
                 &iosb, 0, 0, buf, 1, time, terminat, 0, 0));
      }
      if (iosb.status == SS$_ENDOFFILE)
         vms_sys$exit(SS$_ABORT);
      else if (iosb.status == SS$_TIMEOUT)
         return 0; /* Timeout */
      return 1;  /* iosb.bcount; */
   }
   else {
/* I don't believe we ever use ttyread to read in non-raw mode... */
#undef read
      return read (0, buf, len);
   }
}

vms_sys$exit(val)
{
   sys$exit(val);   /* Debugger entry point. */
}
