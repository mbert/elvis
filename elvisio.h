/* elvisio.h */
/* Copyright 1995 by Steve Kirkendall */


BEGIN_EXTERNC
extern BOOLEAN	ioopen P_((char *name, _char_ rwa, BOOLEAN prgsafe, BOOLEAN force, BOOLEAN binary));
extern int	iowrite P_((CHAR *iobuf, int len));
extern int	ioread P_((CHAR *iobuf, int len));
extern BOOLEAN	ioclose P_((void));
extern char	*iopath P_((char *path, char *filename, BOOLEAN usefile));
extern char	*iofilename P_((char *partial, _char_ endchar));
END_EXTERNC
