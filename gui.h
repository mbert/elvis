/* gui.h */
/* Copyright 1995 by Steve Kirkendall */


/* generic pointer to GUI-specific window information */
typedef void GUIWIN;

/* structs of this type are used to describe each available GUI */
typedef struct gui_s
{
	char	*name;
	char	*desc;
	BOOLEAN	exonly;
	BOOLEAN	newblank;
	BOOLEAN	minimizeclr;
	BOOLEAN	scrolllast;
	BOOLEAN	shiftrows;
	int	movecost;
	int	nopts;
	OPTDESC	*optdescs;
	int	(*test) P_((void));
	int	(*init) P_((int argc, char **argv));
	void	(*usage) P_((void));
	void	(*loop) P_((void));
	BOOLEAN	(*poll) P_((BOOLEAN reset));
	void	(*term) P_((void));
	BOOLEAN	(*creategw) P_((char *name, char *attributes));
	void	(*destroygw) P_((GUIWIN *gw, BOOLEAN force));
	BOOLEAN	(*focusgw) P_((GUIWIN *gw));
	void	(*retitle) P_((GUIWIN *gw, char *name));
	void	(*reset) P_((void));
	void	(*flush) P_((void));
	void	(*moveto) P_((GUIWIN *gw, int column, int row));
	void	(*draw) P_((GUIWIN *gw, _char_ font, CHAR *text, int len));
	BOOLEAN	(*shift) P_((GUIWIN *gw, int qty, int rows));
	BOOLEAN	(*scroll) P_((GUIWIN *gw, int qty, BOOLEAN notlast));
	BOOLEAN	(*clrtoeol) P_((GUIWIN *gw));
	void	(*textline) P_((GUIWIN *gw, CHAR *text, int len));
	void	(*beep) P_((GUIWIN *gw));
	BOOLEAN	(*msg) P_((GUIWIN *gw, MSGIMP imp, CHAR *text, int len));
	void	(*scrollbar) P_((GUIWIN *gw, long top, long bottom, long nlines));
	void	(*status) P_((GUIWIN *gw, CHAR *cmd, long line, long column, _CHAR_ learn, char *mode));
	int	(*keylabel) P_((CHAR *given, int givenlen, CHAR **label, CHAR **raw));
	BOOLEAN	(*clipopen) P_((BOOLEAN forwrite));
	int	(*clipwrite) P_((CHAR *text, int len));
	int	(*clipread) P_((CHAR *text, int len));
	void	(*clipclose) P_((void));
	BOOLEAN	(*color) P_((GUIWIN *gw, _char_ font, CHAR *fg, CHAR *bg));
	BOOLEAN	(*guicmd) P_((GUIWIN *gw, char *extra));
	BOOLEAN	(*tabcmd) P_((GUIWIN *gw, _CHAR_ key2, long count));
	void	(*save) P_((BUFFER buf, GUIWIN *gw));
	int	(*wildcard) P_((char *names, char *buf, int bufsize, BOOLEAN single));
	BOOLEAN	(*prgopen) P_((char *command, BOOLEAN willwrite, BOOLEAN willread));
	int	(*prgclose) P_((void));
	RESULT	(*stop) P_((BOOLEAN alwaysfork));
} GUI;



#if defined(GUI_TERMCAP) || defined(GUI_OPEN)
/* The "termcap" and "open" user interfaces use the following OS-dependent
 * functions.  These functions must be defined in "osXXXX/tcaphelp.c" if
 * you're going to use "termcap" or "open".
 */
BEGIN_EXTERNC
extern void	ttyinit P_((void));
extern void	ttyraw P_((char *erasekey));
extern void	ttynormal P_((void));
extern void	ttysuspend P_((void));
extern void	ttyresume P_((BOOLEAN sendstr));
extern int	ttyread P_((char *buf, int len, int timeout));
extern void	ttywrite P_((char *buf, int len));
extern char	*ttytermtype P_((void));
extern BOOLEAN	ttysize P_((int *linesptr, int *colsptr));
extern BOOLEAN	ttypoll P_((BOOLEAN reset));
extern RESULT	ttystop P_((void));
extern GUIWIN	*ttywindow P_((int ttyrow, int ttycol, int *winrow, int *wincol));
END_EXTERNC

# ifdef NEED_SPEED_T
#  include <termcap.h>
# else
extern char	PC;
extern short	ospeed;		/* might be "speed_t" instead of "short" */
BEGIN_EXTERNC
extern int	tgetent P_((char *, char *));
extern int	tgetflag P_((char *));
extern int	tgetnum P_((char *));
extern char	*tgoto P_((char *, int, int));
extern char	*tgetstr P_((char*, char**));
extern void	tputs P_((char *, int, int (*)(int)));
END_EXTERNC
# endif /* !_POSIX_SOURCE */

#endif /* defined(GUI_TERMCAP) */
