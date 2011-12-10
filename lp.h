/* lp.h */
/* Copyright 1995 by Steve Kirkendall */


typedef struct
{
    char *name;				/* printer type, e.g. "epson" */
    int  minorno;			/* value to pass to `before' */
    BOOLEAN spooled;			/* uses lpout spooler */
    void (*before) P_((int minorno, void (*draw)(_CHAR_ ch)));/* called before print job */
    void (*fontch) P_((_char_ font, _CHAR_ ch)); /* output a single char */
    void (*page) P_((int linesleft));	/* called at end of each page */
    void (*after) P_((int linesleft));	/* called at end of print job */
} LPTYPE;

BEGIN_EXTERNC
extern RESULT lp P_((WINDOW win, MARK top, MARK bottom, BOOLEAN force));
END_EXTERNC
extern LPTYPE lpepson, lppana, lpibm, lphp, lpdumb;
extern LPTYPE lpcr, lpbs;
extern LPTYPE lpps, lpps2;
#ifdef GUI_WIN32
extern LPTYPE lpwindows;
#endif
