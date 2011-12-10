/* cut.h */
/* Copyright 1995 by Steve Kirkendall */


/* Number of CHARs at start of buffer used internally for storing type */
#define CUT_TYPELEN	10

BEGIN_EXTERNC
extern BUFFER cutbuffer P_((_CHAR_ cbname, BOOLEAN create));
extern void cutyank P_((_CHAR_ cbname, MARK from, MARK to, _CHAR_ type, BOOLEAN del));
extern MARK cutput P_((_CHAR_ cbname, WINDOW win, MARK at, BOOLEAN after, BOOLEAN cretend, BOOLEAN lretend));
extern CHAR *cutmemory P_((_CHAR_ cbname));
END_EXTERNC
