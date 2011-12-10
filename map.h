/* map.h */
/* Copyright 1995 by Steve Kirkendall */


/* The current state of the keystroke mapping queue */
typedef enum
{
	MAP_CLEAR,	/* not in the middle of any map */
	MAP_USER,	/* at least 1 unresolved user map */
	MAP_KEY		/* at least 1 unresolved key map, but no user maps */
} MAPSTATE;

/* This data type is a bitmap of the following flags.  It is used to describe
 * when a map takes effect, and how it is interpretted.
 */
typedef unsigned int MAPFLAGS;
#define MAP_INPUT	0x0001	/* map when in input mode */
#define MAP_COMMAND	0x0002	/* map when in visual command mode */
#define MAP_OPEN	0x0004	/* map when in "open" mode */
#define MAP_ASCMD	0x0100	/* always execute map as visual commands */
#define MAP_ALL		0x0107	/* all of the above */
#define MAP_ALL_VISUAL	0x0103	/* all except MAP_OPEN */
#define MAP_ABBR	0x0200	/* this is an abbr, not a map */
#define MAP_DISABLE	0x0400	/* disable all maps for next keystroke */
#define MAP_BREAK	0x0800	/* switch from "run" to "step" trace mode */

BEGIN_EXTERNC
extern void	mapinsert P_((CHAR *rawin, int rawlen, CHAR *cooked, int cooklen, CHAR *label, MAPFLAGS flags));
extern BOOLEAN	mapdelete P_((CHAR *rawin, int rawlen, MAPFLAGS flags, BOOLEAN del, BOOLEAN brk));
extern MAPSTATE	mapdo P_((CHAR *keys, int nkeys));
extern void	mapunget P_((CHAR *keys, int nkeys, BOOLEAN remap));
extern CHAR	*maplist P_((MAPFLAGS flags, int *reflen));
extern RESULT	maplearn P_((_CHAR_ buf, BOOLEAN starting));
extern CHAR	maplrnchar P_((_CHAR_ dflt));
extern void	mapalert P_((void));
extern CHAR	*mapabbr P_((CHAR *bkwd, long *oldptr, long *newptr, BOOLEAN exline));
#ifndef NO_EXRC
extern void	mapsave P_((BUFFER buf));
#endif
END_EXTERNC
