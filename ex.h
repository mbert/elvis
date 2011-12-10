/* ex.h */
/* Copyright 1995 by Steve Kirkendall */

/* This is a list of all possible print flag combinations */
typedef enum
{
	PF_NONE,	/* don't print */
	PF_PRINT,	/* print the line */
	PF_NUMBER,	/* print line number, then print line */
	PF_LIST,	/* list the line (making control characters visible) */
	PF_NUMLIST	/* print line number, then list line */
} PFLAG;
 
typedef enum
{
	EX_ABBR, EX_ALL, EX_APPEND, EX_ARGS, EX_AT,
	EX_BANG, EX_BREAK, EX_BUFFER,
	EX_CALC, EX_CC, EX_CD, EX_CHANGE, EX_CLOSE, EX_COLOR, EX_COMMENT,
		EX_COPY,
	EX_DELETE, EX_DIGRAPH, EX_DISPLAY,
	EX_ECHO, EX_EDIT, EX_ELSE, EX_EQUAL, EX_ERRLIST, EX_EVAL,
	EX_FILE,
	EX_GOTO, EX_GLOBAL, EX_GUI,
	EX_HELP,
	EX_IF, EX_INSERT,
	EX_LAST, EX_LET, EX_LIST, EX_LPR,
	EX_MAKE, EX_MAP, EX_MARK, EX_MKEXRC, EX_MOVE,
	EX_NEXT, EX_NORMAL, EX_NUMBER,
	EX_OPEN,
	EX_POP, EX_PRESERVE, EX_PREVIOUS, EX_PRINT, EX_PUT,
	EX_QALL, EX_QUIT,
	EX_READ, EX_REDO, EX_REWIND,
	EX_SALL, EX_SAFER, EX_SET, EX_SHELL, EX_SHIFTL, EX_SHIFTR, EX_SLAST,
		EX_SNEW, EX_SNEXT, EX_SOURCE, EX_SPLIT, EX_SPREVIOUS,
		EX_SREWIND, EX_STAG, EX_STACK, EX_STOP, EX_SUBAGAIN,
		EX_SUBSTITUTE, EX_SUSPEND,
	EX_TAG, EX_THEN,
	EX_UNABBR, EX_UNBREAK, EX_UNDO, EX_UNMAP,
	EX_VERSION, EX_VGLOBAL, EX_VISUAL,
	EX_WINDOW, EX_WNEXT, EX_WQUIT, EX_WRITE,
	EX_XIT,
	EX_YANK,
	EX_Z
} EXCMD;

/* This structure is used to store a parsed ex command */
typedef struct
{
	WINDOW	window;		/* window where line was entered */
	MARKBUF	defaddr;	/* default address (includes buffer spec) */
	long	from, to;	/* range line numbers */
	long	fromoffset;	/* exact offset of an addressed point */
	MARK	fromaddr;	/* start of the "from" line */
	MARK	toaddr;		/* end of "to" line (start of following line) */
	BOOLEAN	anyaddr;	/* True if any addresses given, False if none */
	char	*cmdname;	/* name of command (for diagnostics) */
	EXCMD	command;	/* code for command name, e.g. EX_PRINT */
	int	cmdidx;		/* index into internal array of cmd attributes */
	int	multi;		/* number of times cmd name was stuttered */
	BOOLEAN	bang;		/* True if '!' appended to command name */
	MARK	destaddr;	/* end of destination line */
	regexp	*re;		/* regular expression */
	CHAR	*lhs;		/* single-word argument or "+lineno" string */
	CHAR	*rhs;		/* multi-word argument, or command line */
	char	**file;		/* array of file names */
	int	nfiles;		/* size of "file" array */
	CHAR	cutbuf;		/* cut buffer name */
	long	count;		/* count argument, or plus value */
	PFLAG	pflag;		/* print flag, causes output of some lines */
	long	delta;		/* print offset */
	BOOLEAN	global;		/* executed as part of :global command? */
	BOOLEAN	undo;		/* save an "undo" version before first change? */
	MARK	newcurs;	/* where cursor should be left (NULL to not move) */
} EXINFO;

BEGIN_EXTERNC
extern BOOLEAN	exparseaddress P_((CHAR **refp, EXINFO *xinf));
extern RESULT	experform P_((WINDOW win, MARK from, MARK to));
extern RESULT	exstring P_((WINDOW win, CHAR *str));
extern CHAR	*exname P_((CHAR *name));
extern RESULT	exenter P_((WINDOW win));
extern long	exprintlines P_((WINDOW win, MARK line, long qty, PFLAG pflag));
extern void	exfree P_((EXINFO *xinf));
extern BOOLEAN	exaddfilearg P_((char ***file, int *nfiles, char *filename, BOOLEAN wild));

extern RESULT	ex_all P_((EXINFO *xinf));
extern RESULT	ex_append P_((EXINFO *xinf));
extern RESULT	ex_args P_((EXINFO *xinf));
extern RESULT	ex_at P_((EXINFO *xinf));
extern RESULT	ex_bang P_((EXINFO *xinf));
extern RESULT	ex_buffer P_((EXINFO *xinf));
extern RESULT	ex_cd P_((EXINFO *xinf));
extern RESULT	ex_color P_((EXINFO *xinf));
extern RESULT	ex_comment P_((EXINFO *xinf));
extern RESULT	ex_delete P_((EXINFO *xinf));
extern RESULT	ex_digraph P_((EXINFO *xinf));
extern RESULT	ex_display P_((EXINFO *xinf));
extern RESULT	ex_edit P_((EXINFO *xinf));
extern RESULT	ex_errlist P_((EXINFO *xinf));
extern RESULT	ex_file P_((EXINFO *xinf));
extern RESULT	ex_global P_((EXINFO *xinf));
extern RESULT	ex_gui P_((EXINFO *xinf));
extern RESULT	ex_help P_((EXINFO *xinf));
extern RESULT	ex_if P_((EXINFO *xinf));
extern RESULT	ex_join P_((EXINFO *xinf));
extern RESULT	ex_lpr P_((EXINFO *xinf));
extern RESULT	ex_make P_((EXINFO *xinf));
extern RESULT	ex_map P_((EXINFO *xinf));
extern RESULT	ex_mark P_((EXINFO *xinf));
extern RESULT	ex_mkexrc P_((EXINFO *xinf));
extern RESULT	ex_move P_((EXINFO *xinf));
extern RESULT	ex_next P_((EXINFO *xinf));
extern RESULT	ex_pop P_((EXINFO *xinf));
extern RESULT	ex_print P_((EXINFO *xinf));
extern RESULT	ex_put P_((EXINFO *xinf));
extern RESULT	ex_qall P_((EXINFO *xinf));
extern RESULT	ex_read P_((EXINFO *xinf));
extern RESULT	ex_sall P_((EXINFO *xinf));
extern RESULT	ex_set P_((EXINFO *xinf));
extern RESULT	ex_shift P_((EXINFO *xinf));
extern RESULT	ex_source P_((EXINFO *xinf));
extern RESULT	ex_stack P_((EXINFO *xinf));
extern RESULT	ex_substitute P_((EXINFO *xinf));
extern RESULT	ex_suspend P_((EXINFO *xinf));
extern RESULT	ex_tag P_((EXINFO *xinf));
extern RESULT	ex_then P_((EXINFO *xinf));
extern RESULT	ex_undo P_((EXINFO *xinf));
extern RESULT	ex_version P_((EXINFO *xinf));
extern RESULT	ex_split P_((EXINFO *xinf));
extern RESULT	ex_window P_((EXINFO *xinf));
extern RESULT	ex_write P_((EXINFO *xinf));
extern RESULT	ex_xit P_((EXINFO *xinf));
extern RESULT	ex_z P_((EXINFO *xinf));

extern void	colorsave P_((BUFFER custom));
END_EXTERNC
