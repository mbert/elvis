/* alias.c */

/* Author:
 *		Peter Reinig
 *		Universitaet Kaiserslautern
 *		Postfach 3049
 *		7650 Kaiserslautern
 *		W. Germany
 *		reinig@physik.uni-kl.de
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
#define ELVIS	"elvis"

main(argc, argv)
	int	argc;
	char	*argv[];
{
	int	pid, i, j;
	int	letter;
	char	**argblk;
	char	fullelvis[256];
	char	*scan;

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
