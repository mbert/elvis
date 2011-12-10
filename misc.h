/* misc.h */
/* Copyright 1995 by Steve Kirkendall */

extern CHAR	empty[];
extern CHAR	blanks[];
extern char	**arglist;
extern int	argnext;
BEGIN_EXTERNC
extern int	buildCHAR P_((CHAR **refstr, _CHAR_ ch));
extern int	buildstr P_((CHAR **refstr, char *add));
extern MARK	wordatcursor P_((MARK cursor));
extern CHAR	*addquotes P_((CHAR *chars, CHAR *str));
extern CHAR	*removequotes P_((CHAR *chars, CHAR *str));
END_EXTERNC
