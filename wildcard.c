/* wildcard.c */

/* Author:
 *	Guntram Blohm
 *	Buchenstrasse 19
 *	7904 Erbach, West Germany
 *	Tel. ++49-7305-6997
 *	sorry - no regular network connection
 */

/* this program implements wildcard expansion for elvis/dos. It works
 * like UNIX echo, but uses the dos wildcard conventions
 * (*.* matches all files, * matches files without extension only,
 * filespecs may contain drive letters, wildcards not allowed in directory
 * names).
 *
 * It is also #included into ctags.c and elvprsv.c; in this case,
 * we don't want a main function here.
 */

#include <stdio.h>
#ifdef __STDC__
# include <stdlib.h>
#endif
#ifndef	WILDCARD_NO_MAIN
# include "config.h"
#endif
#ifdef	__TURBOC__
# include <dir.h>
#endif

void	expand P_((char *));
void	addfile P_((char *));

/* We include ctype.c here (instead of including just ctype.h and linking
 * with ctype.o) because on some systems ctype.o will have been compiled in
 * "large model" and the wildcard program is to be compiled in "small model" 
 * You can't mix models.  By including ctype.c here, we can avoid linking
 * with ctype.o.
 *
 * HOWEVER, if WILDCARD_NO_MAIN is defined then wildcard.c is itself being
 * included in another .c file, which has either already included ctypes.c or
 * will be linked with ctypes.o, so in this instance we include just ctypos.h.
 */
#ifdef WILDCARD_NO_MAIN
# include "ctype.h"
#else
# include "ctype.c"
#endif

#ifdef	M_I86
# define findfirst(a,b,c)	_dos_findfirst(a,c,b)
# define findnext		_dos_findnext
# define ffblk			find_t
# define ff_name		name
# include <dos.h>
#endif

/* Atari TOS, MWC */
#ifdef M68000
# include <stat.h>
# include <osbind.h>
# define findfirst(a,b,c)	(Fsetdta(b), (Fsfirst(a,c)))
# define findnext(x)		(Fsnext())
# define ff_name		d_fname
#endif

/* Atari TOS, GNU-C */
#ifdef __m68k__
# include <stat.h>
# include <osbind.h>
# define findfirst(a,b,c)	(Fsetdta(b), (Fsfirst(a,c)))
# define findnext(x)		(Fsnext())
# define ff_name		dta_name
# define DMABUFFER 		struct _dta
#endif

/* OS/2, emx+gcc 0.8x */
#if OS2
# define size_t xxx_size_t
# include <sys/emx.h>
# undef size_t
# define findfirst(a,b,c)	__findfirst(a,c,b)
# define findnext		__findnext
# define ffblk			_find
# define ff_name		name
#endif

#define	MAXFILES	1000

#if !defined(__STDC__) && !defined(__TURBOC__)
extern char *calloc();
#endif

char *files[MAXFILES];
int nfiles;

#ifndef	WILDCARD_NO_MAIN

main(argc, argv)
	char **argv;
{
	int i;

	/* GRR:  for OS/2 and emx+gcc, could use _wildcard() built-in */

	_ct_init("");
	for (i=1; i<argc; i++)
		expand(argv[i]);
	if (nfiles)
		printf("%s", files[0]);
	for (i=1; i<nfiles; i++)
	{
		printf(" %s", files[i]);
	}
	putchar('\n');
	return 0;
}

#else
char **wildexpand(argc, argv)
	int *argc;
	char **argv;
{
	int i;
	
	/* GRR:  for OS/2 and emx+gcc, could just use _wildcard() built-in */

	for (i=0; i<*argc; i++)
		expand(argv[i]);
	*argc=nfiles;
	return files;
}	
#endif

#ifdef __TURBOC__
int pstrcmp(const void *a, const void *b)
#else
int pstrcmp(a, b)
	char **a, **b;
#endif
{
	return strcmp(*(char **)a, *(char **)b);
}

void expand(name)
	char *name;
{
	char *filespec;
	int wildcard=0;
#if defined(M68000) || defined(__m68k__)
	DMABUFFER findbuf;
#else
	struct ffblk findbuf;
#endif
	int err;
	char buf[80];
	int lastn;

	strcpy(buf, name);
	for (filespec=buf; *filespec; filespec++)
		;

	while (--filespec>=buf)
	{	if (*filespec=='?' || *filespec=='*')
			wildcard=1;
		if (*filespec=='/' || *filespec=='\\' || *filespec==':')
			break;
	}
	if (!wildcard)
		addfile(buf);
	else
	{
		lastn=nfiles;
		filespec++;
		if ((err=findfirst(buf, &findbuf, 0))!=0)
			addfile(buf);
		while (!err)
		{
			strcpy(filespec, findbuf.ff_name);
			addfile(buf);
			err=findnext(&findbuf);
		}
		if (lastn!=nfiles)
			qsort(files+lastn, nfiles-lastn, sizeof(char *), pstrcmp);
	}
}

#if MINT
extern int __mint;
#endif
void addfile(buf)
	char *buf;
{
	char *p;

#if MINT
	/* there are filesystems on MiNT that are case sensitive... and for
	 * the vanilla GEMDOS fs MiNT already does this conversion itself.
	 */
	if (!__mint)
#endif
#if !OS2
	for (p=buf; *p; p++)
		*p=tolower(*p);
#endif
	/* convert spaces to something else so not confused with delimiter */
	for (p=buf; *p; p++)
		if (*p == ' ')
			*p = SPACEHOLDER;

	if (nfiles<MAXFILES && (files[nfiles]=calloc(strlen(buf)+1, 1))!=0)
		strcpy(files[nfiles++], buf);
}
