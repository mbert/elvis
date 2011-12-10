/* gui2.h */
/* Copyright 1995 by Steve Kirkendall */



#define guiscrollbar(w,t,b,n)	if (gui->scrollbar != NULL) \
					(*scrollbar)((w)->gw, t, b, n); else (void)0
#define guiflush()		if (gui->flush) (*gui->flush)()

BEGIN_EXTERNC
extern void	guimoveto P_((WINDOW win, int column, int row));
extern void	guidraw P_((WINDOW win, _char_ font, CHAR *text, int len));
extern BOOLEAN	guishift P_((WINDOW win, int qty, int rows));
extern BOOLEAN	guiscroll P_((WINDOW win, int qty, BOOLEAN notlast));
extern void	guiclrtoeol P_((WINDOW win));
extern void	guireset P_((void));
extern BOOLEAN	guipoll P_((BOOLEAN reset));
extern void	guibeep P_((WINDOW win));
END_EXTERNC

extern GUI	*gui;

#ifdef GUI_X11
extern GUI	guix11;
#endif

#ifdef GUI_CURSES
extern GUI	guicurses;
#endif

#ifdef GUI_BIOS
extern GUI	guibios;
#endif

#ifdef GUI_TERMCAP
extern GUI	guitermcap;
#endif

#ifdef GUI_OPEN
extern GUI	guiopen;
extern GUI	guiquit;
#endif

#ifdef GUI_WIN32
extern GUI	guiwin32;
#endif
