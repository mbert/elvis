/* alias.c */

/* Author:
 *		Peter Reinig
 *		Universitaet Kaiserslautern
 *		Postfach 3049
 *		7650 Kaiserslautern
 *		W. Germany
 *		reinig@physik.uni-kl.de
 *
 * adapted for IBM's OS/2 compiler by herbert
 * tweaked for DOS/Win32/OS2 portability by Steve Kirkendall
 */

/* This tiny program executes elvis with the flags that are appropriate
 * for a given command name.  This program is used only on systems that
 * don't allow UNIX-style file links.
 *
 * The benefit of this program is: instead of having 5 copies of elvis
 * on your disk, you only need one copy of elvis and 4 copies of this
 * little program.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>
#include <io.h>
#include "osdef.h"

/* This is the name of the real elvis program, with the directory name or
 * the ".exe" extension.
 */
#define ELVIS	"elvis"

/* These are alternative names.  You can use these with -DARGV0=... */
#define VI	"vi"
#define EX	"ex"
#define VIEW	"view"
#define INPUT	"input"

/* if the shell doesn't set argv[0] to the program name, then argv[0] will
 * be set to this value.  Usually this value is set by compiling elvis with
 * -DARGV0=EX or something like that, so this is just the default.
 */
#ifndef ARGV0
# define ARGV0	VI
#endif

/* Win32 seems to have developed an inability to exec() correctly.  The shell
 * thinks the program has exited when in fact it has merely exec'ed some
 * other function.  This messes up the ex/vi/view aliases for elvis.  To work
 * around it, we SPAWN elvis and wait for it to return before the alias exits.
 * This wastes a little memory, but at least it works.
 */
#if _MSC_VER >= 900 /* 32-bit compiler for Windows */
# define execvp(p,a)	(i = _spawnvp(_P_WAIT, p,a)); if (i >= 0) exit(i);
#endif



main(argc, argv)
	int	argc;
	char	*argv[];
{
	int	i, j;
	int	letter;
	char	**argblk;
	char	fullelvis[256];
	char	*scan;

	/* if argv[0] isn't set, then set it now */
	if (!argv[0])
		argv[0] = ARGV0;

	/* allocate enough space for a copy of the argument list, plus a
	 * terminating NULL, plus maybe an added flag.
	 */
	argblk = (char **) malloc((argc + 2) * sizeof(char *));
	if (!argblk)
	{
		perror(argv[0]);
		exit(2);
	}

	/* find the last letter in the invocation name of this program */
	/* we almost certainly must bypass ".EXE" or ".TTP" from argv[0] */
	i = strlen(argv[0]);
	if (i > 4 && argv[0][i - 4] == '.')
		i -= 4;
	letter = argv[0][i - 1];

	/* find the name of elvis.  First try looking for ELVIS.EXE in the
	 * same directory as the VI.EXE file.  If that doesn't work, then
	 * just call it ELVIS and hope that it can be found somewhere in the
	 * execution path.
	 */
	strcpy(fullelvis, argv[0]);
	scan = strrchr(fullelvis, '\\');
	if (scan)
	{
		for (scan++, j = 0; ELVIS[j]; )
			*scan++ = ELVIS[j++];
		while (i >= 0 && argv[0][i])
			*scan++ = argv[0][i++];
		*scan = '\0';
		if (access(fullelvis, 0) != 0)
			strcpy(fullelvis, ELVIS);
	}
	else /* no path information in argv[0], try to load via PATH */
		strcpy(fullelvis, ELVIS);

	/* copy argv to argblk, possibly inserting a flag such as "-R" */
	argblk[0] = fullelvis;
	i = j = 1;
	switch (letter)
	{
	  case 'w':			/* "view" */
	  case 'W':
		argblk[i++] = "-R";
		break;

	  case 'x':			/* "ex" */
	  case 'X':
		argblk[i++] = "-e";
		break;

	  case 't':			/* "input" */
	  case 'T':
		argblk[i++] = "-i";
		break;
	}
	while (j < argc)
	{
		argblk[i++] = argv[j++];
	}
	argblk[i] = (char *)0;

	/* execute the real ELVIS program */
	(void)execvp(argblk[0], argblk);
	perror(ELVIS);
}
