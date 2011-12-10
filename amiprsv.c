/* amiprsv.c */

/*-
 *	Mike Rieser 				Dale Rahn
 *	2410 Happy Hollow Rd. Apt D-10		540 Vine St.
 *	West Lafayette, IN 47906 		West Lafayette, IN 47906
 *	riesermc@mentor.cc.purdue.edu		rahn@sage.cc.purdue.edu
 */

/* This file contains the AmigaDOS-specific parts of the "elvprsv" program. */

#include <stdio.h>

/* This function returns the login name of the owner of a file */
char        *
ownername(filename)
    char        *filename;	/* name of a file */
{
    return ("Amigan");
}


/*
 * This function sends a mail message to a given user, saying that a file has
 * been preserved.
 */
void 
mail(user, file, when)
    char        *user;		/* name of user who should receive the mail */
    char        *file;		/* name of original text file that was
				 * preserved */
    char        *when;		/* description of why the file was preserved */
{
    char         cmd[80];	/* buffer used for constructing a "mail"
				 * command */
    FILE        *m;		/* stream used for giving text to the "mail"
				 * program */
    char        *base;		/* basename of the file */

    /* separate the directory name from the basename. */
    for (base = file + strlen(file); --base > file && *base != SLASH;)
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
    m = fopen("CON:0/50/600/150/Elvis/CLOSE/WAIT", "w");
    if ((FILE *) 0 == m)
	m = stdout;

    /* Tell the user that the file was preserved */
    fprintf(m, "A version of your file \"%s%c%s\"\n", file, SLASH, base);
    fprintf(m, "was preserved when %s.\n", when);
    fprintf(m, "To recover this file, do the following:\n");
    fprintf(m, "\n");
    fprintf(m, "     cd %s\n", file);
    fprintf(m, "     elvrec %s\n", base);
    fprintf(m, "\n");

    fclose(m);
}
