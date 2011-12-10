/* opsys.h */
/* Copyright 1995 by Steve Kirkendall */



typedef enum
{
	DIR_INVALID,	/* malformed filename (can't happen with UNIX) */
	DIR_BADPATH,	/* unable to check file */
	DIR_NOTFILE,	/* directory or other non-file */
	DIR_NEW,	/* file doesn't exist yet */
	DIR_UNREADABLE,	/* file exists but is unreadable */
	DIR_READONLY,	/* file is readable but not writable */
	DIR_READWRITE	/* file is readable and writable. */
} DIRPERM;

BEGIN_EXTERNC
extern BOOLEAN	blkopen P_((BOOLEAN force, BLK *buf));
extern void	blkclose P_((BLK *buf));
extern void	blkwrite P_((BLK *buf, _BLKNO_ blkno));
extern void	blkread P_((BLK *buf, _BLKNO_ blkno));
extern void	blksync P_((void));

extern char	*dirfirst P_((char *wildexpr, BOOLEAN ispartial));
extern char	*dirnext P_((void));
extern BOOLEAN	diriswild P_((char *wildexpr));
extern DIRPERM	dirperm P_((char *filename));
extern char	*dirdir P_((char *pathname));
extern char	*dirfile P_((char *pathname));
extern char	*dirpath P_((char *dir, char *file));
extern char	*dircwd P_((void));
extern BOOLEAN	dirchdir P_((char *pathname));

extern BOOLEAN	prgopen P_((char *command, BOOLEAN willwrite, BOOLEAN willread));
extern int	prgwrite P_((CHAR *buf, int nbytes));
extern BOOLEAN	prggo P_((void));
extern int	prgread P_((CHAR *buf, int nbytes));
extern int	prgclose P_((void));

extern int	txtopen P_((char *filename, _char_ rwa, BOOLEAN binary));
extern void	txtclose P_((void));
extern int	txtwrite P_((CHAR *buf, int nbytes));
extern int	txtread P_((CHAR *buf, int nbytes));

#ifdef OSINIT
extern void	osinit P_((char *argv0));
#endif
END_EXTERNC
