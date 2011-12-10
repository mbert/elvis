/* draw.h */
/* Copyright 1995 by Steve Kirkendall */


typedef struct
{
	long	lineoffset; /* which line this row is for */
	int	insrows;
	int	shiftright;
	int	inschars;
} DRAWROW;

typedef struct
{
	long	start;	/* offset from start of buffer to start of line */
	int	width;	/* width of the line */
	int	startrow;/* where it appears on the screen */
} DRAWLINE;

typedef enum
{
	DRAW_VISUAL,    /* visual, no message or status msg */
	DRAW_VMSG,      /* visual, non-status message */
	DRAW_OPENEDIT,  /* open, editing a line */
	DRAW_OPENOUTPUT /* open, after outputing a message */
} DRAWSTATE;

typedef enum
{
	DRAW_NORMAL,	/* optimizable, refreshable, cursor on screen */
	DRAW_CHANGED,	/* non-optimizable, refreshable, cursor on screen */
	DRAW_CENTER,	/* non-optimizable, refreshable, cursor in top half */
	DRAW_SCRATCH 	/* non-optimizable, non-refreshable, cursor on screen */
} DRAWLOGIC;


typedef struct
{
	DRAWSTATE drawstate;	/* drawing state */
	DRAWROW	 *newrow;	/* info about new rows */
	DRAWLINE *newline;	/* info about new lines */
	DRAWLINE *curline;	/* info about current lines */
	CHAR	 *newchar;	/* characters of new image */
	char	 *newfont;	/* fonts of new image */
	CHAR	 *curchar;	/* characters of current image */
	char	 *curfont;	/* fonts of current image */
	long	 *offsets;	/* buffer offsets of each individual cell */
	long	 topline;	/* offset of first line drawn */
	long	 bottomline;	/* offset of line after last drawn */
	BUFFER	 curbuf;	/* current buffer */
	long	 curnbytes;	/* size of buffer when current image drawn */
	long	 curchgs;	/* buffer's "changes" counter when image drawn */
	int	 rows, columns; /* dimensions of screen */
	int	 cursrow, curscol;/* position of cursor */
	int	 skipped;	/* number of columns skipped from first line */
	int	 nlines;	/* number of lines */
	DRAWLOGIC logic;	/* ignore current image? */
	BOOLEAN	 newmsg;	/* does msg row contain anything important? */
	MARK	 openline;	/* current line (open mode only) */
	CHAR	 *openimage;	/* image of current line, '\0'-terminated */
	long	 opencursor;	/* where cursor is within line */
	long	 opencnt;	/* width of line in openimage */
	int	 opencell;	/* tty simulator's cursor position */
} DRAWINFO;
