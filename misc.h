/* misc.h */
/* Copyright 1995 by Steve Kirkendall */

extern CHAR	empty[];
extern CHAR	blanks[];
extern char	**arglist;
extern int	argnext;
BEGIN_EXTERNC
extern int	buildCHAR P_((CHAR **refstr, _CHAR_ ch));
extern MARK	wordatcursor P_((MARK cursor));
END_EXTERNC
