/* amitty.c */

/*-
 *	Mike Rieser 				Dale Rahn
 *	2410 Happy Hollow Rd. Apt D-10		540 Vine St.
 *	West Lafayette, IN 47906 		West Lafayette, IN 47906
 *	riesermc@mentor.cc.purdue.edu		rahn@sage.cc.purdue.edu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <devices/conunit.h>
#include <dos/dos.h>
#include <clib/macros.h>
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

/*-
 * Defines for amigatty.
 *
 * Note: Amiga <CSI> Control Sequence Introducer is either:
 * 0x9b or <ESC>[ == 0x1b 0x5b.
 *
 * The Amiga always responds with 0x9b.
 *
 * For you octal fans: 0x9b == \233, 0x1b == \033.
 */
#define CSI		'\233'
#define TIME_FACTOR	(100000)/* convert 1/10 sec to microsecs */

/* Amiga Console Control Sequences */
#define ENABLE_SCROLL	((UBYTE *) "\233>1h")	/* Amiga default */
#define DISABLE_SCROLL	((UBYTE *) "\233>1l")
#define AUTOWRAP_ON	((UBYTE *) "\233?7h")	/* Amiga default */
#define AUTOWRAP_OFF	((UBYTE *) "\233?7l")
#define CURSOR_ON	((UBYTE *) "\233 p")	/* Amiga default */
#define CURSOR_OFF	((UBYTE *) "\2330 p")

#define RAW_EVENTS_ON	((UBYTE *) "\23312{")	/* Set Window Resize Reports */
#define RAW_EVENTS_OFF	((UBYTE *) "\23312}")	/* Reset Window Resize Reports */

/* take out for compiling with elvis */

#ifndef ctrl
#define	ctrl(ch)		((ch)&037)
#endif

/* Variables */
static BPTR  fh = 0, inputFH, outputFH, oldinputFH, oldoutputFH;
static UBYTE title[]= "RAW:0/0/999/999/Amiga Elvis 1.5/SIMPLE";
static int   amigaterm = 0;

/* Function prototypes for amitty.c */
void         amiopenwin(char *termtype);
void         amiclosewin(void);
void         ttysetup(void);
void         ttyshutdown(void);
int          ttywrite(char *buf, int len);
int          ttyread(char *buf, int len, int time);
int          CheckforSpecial(char *buf, long len);
LONG         setRawCon(LONG toggle);
LONG         sendpkt(struct MsgPort *pid, LONG action, LONG args[], LONG nargs);

/*
 * amiopenwin - opens a window if we don't already have one.
 */
void
amiopenwin(char *termtype)
{
    if (!IsInteractive(Input()))
    {
	/* open our own window in RAW mode */
	if (isOldDOS() || (BPTR) 0 == (fh = Open(title, MODE_READWRITE)))
	{
	    PutStr((UBYTE *) "Couldn't open RAW: window");
	    clean_exit(2);
	} else
	{
	    oldinputFH = SelectInput(fh);
	    oldoutputFH = SelectOutput(fh);
	}
    }
    inputFH = Input();
    outputFH = Output();

    if (!strcmp(termtype, TERMTYPE))
    {
	amigaterm = 1;
	Write(outputFH, AUTOWRAP_OFF, sizeof(AUTOWRAP_OFF));
    }
    return;
}


/*
 * amiclosewin - closes a window if we opened one.
 */
void
amiclosewin()
{
    if (amigaterm)
    {
	Write(outputFH, AUTOWRAP_ON, sizeof(AUTOWRAP_ON));	/* Amiga default */
    }
    if (fh)
    {					/* Close down the window */
	SelectInput(oldinputFH);
	SelectOutput(oldoutputFH);
	Close(fh);
    }
    return;
}


/*
 * ttysetup - console initalization routine for Amiga Computers.
 * 
 * Sets raw mode and enables resize notifications.
 */
void
ttysetup()
{
    if (isOldDOS())
    {
	setRawCon(DOSTRUE);
    } else
    {
	SetMode(inputFH, 1);		/* Enter RAW mode */
    }

    if (amigaterm)
    {
	Write(outputFH, RAW_EVENTS_ON, sizeof(RAW_EVENTS_ON));
    }
    return;
}


/*
 * ttyshutdown - console shutdown routine for Amiga Computers.
 * 
 * Resets raw mode and disables resize notifications.
 */
void
ttyshutdown()
{
    if (amigaterm)
    {
	Write(outputFH, RAW_EVENTS_OFF, sizeof(RAW_EVENTS_OFF));
    }
    if (isOldDOS())
    {
	setRawCon(DOSFALSE);
    } else
    {
	SetMode(inputFH, 0);		/* Leave RAW mode */
    }

    return;
}


/*
 * ttywrite - amiga version of ttywrite.
 * 
 * This version makes small writes to the console as suggested in the RKM.
 * Also turns off the cursor to speed output to the screen.
 */
int
ttywrite(buf, len)
    char        *buf;
    int          len;
{
    int		    cursor_off;
    register int    bytes;
    register UBYTE *pc = (UBYTE *) buf;

    /* See if turning off the cursor is worthwhile */
    if (cursor_off = amigaterm && len > 2 * sizeof(CURSOR_OFF))
    {
	Write(outputFH, CURSOR_OFF, sizeof(CURSOR_OFF));	/* Turn Cursor OFF */
    }

    /* The console.device doesn't like large writes */
    for (bytes = 0; len; pc += bytes, len -= bytes)
    {
	bytes = Write(outputFH, pc, MIN((LONG) len, 256L));
    }

    if (cursor_off)
    {
	Write(outputFH, CURSOR_ON, sizeof(CURSOR_ON));	/* Turn Cursor ON */
    }
    return pc - buf;
}


/*
 * ttyread - amiga version of ttyread.
 */
int
ttyread(buf, len, time)
    char        *buf;		/* where to store the gotten characters */
    int          len;		/* maximum number of characters to read */
    int          time;		/* maximum time in 1/10 sec for reading */
{
    LONG         bytes = 0;	/* number of bytes actually read */

    if (!time || WaitForChar(inputFH, time * TIME_FACTOR))
    {					/* Read() if time == 0 or chars waiting */
	bytes = Read(inputFH, (UBYTE *) buf, (LONG) len);
	bytes = CheckforSpecial(buf, bytes);
    }
    return bytes;			/* the number of bytes read in buf */
}


/*
 * CheckforSpecial - crude parser for raw console events.
 */
int
CheckforSpecial(buf, len)
    char        *buf;
    long         len;
{
    int          isnewsize = 0;
    char        *pb, *peor, *pend;

    pb = buf;
    pend = &buf[len];
    do
    {
	if (CSI != *pb)
	    continue;

	if (WaitForChar(inputFH, 1))
	{
	    pend += Read(inputFH, pend, 72);
	}
	if (peor = strchr((char *) pb, '|'))	/* Window Resize Event */
	{				/* bug == <CSI> seq <CSI> event '|' */
	    *pb = ctrl('L');		/* force redraw */
	    isnewsize = 1;
	    memmove(pb + 1, peor + 1, pend - peor);
	    pend -= peor - pb;
	}
    }
    while (*pb++);

    if (isnewsize)
	getsize(0);

    return pend - buf;
}


/* INDENT OFF */
/* sendpkt code - A. Finkel, P. Lindsay, C. Scheppner  CBM */

LONG setRawCon(toggle)
LONG toggle;     /* DOSTRUE (-1L)  or  DOSFALSE (0L) */
   {
   struct MsgPort *conid;
   struct Process *me;
   LONG myargs[8] ,nargs, res1;

   me = (struct Process *) FindTask(NULL);
   conid = (struct MsgPort *) me->pr_ConsoleTask;

   myargs[0]= toggle;
   nargs = 1;
   res1 = (LONG)sendpkt(conid,ACTION_SCREEN_MODE,myargs,nargs);
   return(res1);
   }



LONG sendpkt(pid,action,args,nargs)
struct MsgPort *pid;  /* process indentifier ... (handlers message port ) */
LONG action,          /* packet type ... (what you want handler to do )   */
     args[],          /* a pointer to a argument list */
     nargs;           /* number of arguments in list  */
   {
   struct MsgPort        *replyport;
   struct StandardPacket *packet;
 
   LONG  count, *pargs, res1;

   replyport = (struct MsgPort *) CreatePort(NULL,0);
   if(!replyport) return((LONG)NULL);

   packet = (struct StandardPacket *) 
      AllocMem((long)sizeof(struct StandardPacket),MEMF_PUBLIC|MEMF_CLEAR);
   if(!packet) 
      {
      DeletePort(replyport);
      return((LONG)NULL);
      }

   packet->sp_Msg.mn_Node.ln_Name = (char *)&(packet->sp_Pkt);
   packet->sp_Pkt.dp_Link         = &(packet->sp_Msg);
   packet->sp_Pkt.dp_Port         = replyport;
   packet->sp_Pkt.dp_Type         = action;

   /* copy the args into the packet */
   pargs = &(packet->sp_Pkt.dp_Arg1);       /* address of first argument */
   for(count=0;count < nargs;count++) 
      pargs[count]=args[count];
 
   PutMsg(pid,(struct Message *)packet); /* send packet */

   WaitPort(replyport);
   GetMsg(replyport); 

   res1 = packet->sp_Pkt.dp_Res1;

   FreeMem(packet,(long)sizeof(struct StandardPacket));
   DeletePort(replyport); 

   return(res1);
   }
 
