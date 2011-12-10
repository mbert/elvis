/* prsvdos.c */

/* This file contains the DOS-specific parts of the "elvprsv" program. */

#include <stdio.h>

/* This function returns the login name of the owner of a file */
char *ownername(filename)
	char	*filename;	/* name of a file */
{
	return "user";
}


/* This function sends a mail message to a given user, saying that a file
 * has been preserved.
 */
void mail(user, file, when, tmp)
	char	*user;	/* name of user who should receive the mail */
	char	*file;	/* name of original text file that was preserved */
	char	*when;	/* description of why the file was preserved */
	int	tmp;	/* NULL normally; else name of temp file if user should run elvprsv -R */
{
	char	cmd[80];/* buffer used for constructing a "mail" command */
	FILE	*m;	/* stream used for giving text to the "mail" program */
	char	*base;	/* basename of the file */

	/* separate the directory name from the basename. */
	for (base = file + strlen(file); --base > file && *base != SLASH; )
	{
	}
	if (*base == SLASH)
	{
		*base++ = '\0';
	}

	/* for anonymous buffers, pretend the name was "foo" */
	if (!strcmp(base, "*"))
	{
		base = "foo";
	}

	/* Tell the user that the file was preserved */
	printf("A version of your file \"%s%c%s\"\n", file, SLASH, base);
	printf("was preserved when %s.\n", when);
	printf("To recover this file, do the following:\n");
	printf("\n");
	printf("     C:\\> cd %s\n", file);
	if (tmp)
	{
		printf("     %s> elvprsv -R %s\n", file, tmp);
	}
	else
	{
		printf("     %s> elvrec %s\n", file, base);
	}
	printf("\n");
}
