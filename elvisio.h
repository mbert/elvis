/* elvisio.h */
/* Copyright 1995 by Steve Kirkendall */


BEGIN_EXTERNC
#ifdef DEBUG_ALLOC
extern BOOLEAN	_ioopen P_((char *file, int line, char *name, _char_ rwa, BOOLEAN prgsafe, BOOLEAN force, _char_ eol));
# define ioopen(n,r,p,f,e)	_ioopen(__FILE__, __LINE__, n,r,p,f,e)
#else
extern BOOLEAN	ioopen P_((char *name, _char_ rwa, BOOLEAN prgsafe, BOOLEAN force, _char_ eol));
#endif
extern int	iowrite P_((CHAR *iobuf, int len));
extern int	ioread P_((CHAR *iobuf, int len));
extern BOOLEAN	ioclose P_((void));
extern char	*iopath P_((char *path, char *filename, BOOLEAN usefile));
extern char	*iofilename P_((char *partial, _char_ endchar));
extern char	*ioeol P_((char *filename));

char *urllocal P_((char *url));
DIRPERM urlperm P_((char *url));
#if defined(PROTOCOL_HTTP) || defined(PROTOCOL_FTP)
BOOLEAN urlremote P_((char *url));
BOOLEAN urlopen P_((char *url, BOOLEAN force, _char_ rwa));
void urlbytes P_((long totbytes));
int urlread P_((CHAR *buf, int bytes));
int urlwrite P_((CHAR *buf, int bytes));
void urlclose P_((void));

# ifdef PROTOCOL_HTTP
BOOLEAN httpopen P_((char *site_port, char *resource));
int httpread P_((CHAR *buf, int nbytes));
void httpclose P_((void));
# endif

# ifdef PROTOCOL_FTP
extern DIRPERM ftpperms;
BOOLEAN ftpopen P_((char *site_port, char *resource, BOOLEAN force, _char_ rwa));
int ftpread P_((CHAR *buf, int nbytes));
int ftpwrite P_((CHAR *buf, int nbytes));
void ftpclose P_((void));
# endif

#endif /* PROTOCOL_HTTP || PROTOCOL_FTP */
END_EXTERNC
