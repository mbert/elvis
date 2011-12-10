/* misc.c */
/* Copyright 1995 by Steve Kirkendall */

char id_misc[] = "$Id: misc.c,v 2.6 1996/05/30 17:39:04 steve Exp $";

#include "elvis.h"



/* This is used as a zero-length string */
CHAR	empty[1];



/* This is used when we need a bunch of blanks */
CHAR	blanks[] = "                                                                                ";



/* These store the args list.  The "arglist" variable points to a dynamically
 * allocated array of (char *) pointers.  Each element of the array ppoints to
 * a dynamically allocated string, except that the last one is NULL.  The
 * "argnext" variable stores the index of the next (not current!) element
 * to be edited.
 */
char	**arglist;	/* array of strings (dynamically allocated) */
int	argnext;	/* index into arglist[] of next arg */



/* This function appends a single character to a dynamically-allocated
 * string.  A NUL character is always appended after the last character,
 * but this function also supports NUL characters in the middle of the
 * string.
 *
 * Only one string can be under construction at a time.  To start a string,
 * Call this function with a pointer to a (CHAR *) variable which is NULL.
 * To append to that string, call this function with a pointer to the same
 * (CHAR *) variable.
 *
 * This function updates the value of the (CHAR *) variable whenever it
 * reallocates memory.  It returns the number of characters added so far,
 * excluding the terminal NUL.
 */
int buildCHAR(refstr, ch)
	CHAR	**refstr;	/* pointer to variable which points to string */
	_CHAR_	ch;		/* character to append to that string */
{
	static int	len;	/* length of the string so far */
	CHAR		*newp;	/* new memory for the same string */
#define GRANULARITY	32	/* minimum number of chars to allocate */

	/* if the string pointer is currently NULL, then start a new string */
	if (!*refstr)
	{
		len = 0;
		*refstr = (CHAR *)safealloc(GRANULARITY, sizeof(CHAR));
	}

	/* if the string is expanding beyond the current allocated memory,
	 * then allocate some new memory and copy the string into it.
	 */
	if ((len + 1) % GRANULARITY == 0)
	{
		newp = safealloc(len + 1 + GRANULARITY, sizeof(CHAR));
		memcpy(newp, *refstr, len * sizeof(CHAR));
		safefree(*refstr);
		*refstr = newp;
	}

	/* append the new character, and a NUL character */
	(*refstr)[len++] = ch;
	(*refstr)[len] = '\0';
	return len;
}


/* This function finds the endpoints of the word at a given point.  Upon
 * return, the offset of the argument MARK will have been changed to the
 * character after the end of the word, and this function will return a
 * static temporary MARK which points to the start of the word.  Exception:
 * If the argument MARK isn't on a word, this function leaves it unchanged
 * and returns NULL.
 */
MARK wordatcursor(cursor)
	MARK	cursor;	/* some point in the word */
{
 static	MARKBUF	retmark;/* the return value */
	CHAR	*p;

	/* If "cursor" is NULL, fail */
	if (!cursor)
	{
		return NULL;
	}

	/* If "cursor" isn't on a letter, digit, or underscore, then fail */
	scanalloc(&p, cursor);
	if (!p || (!isalnum(*p) && *p != '_'))
	{
		scanfree(&p);
		return NULL;
	}

	/* search back to the start of the word */
	retmark = *cursor;
	do
	{
		scanprev(&p);
		markaddoffset(&retmark, -1);
	} while (p && (isalnum(*p) || *p == '_'));
	markaddoffset(&retmark, 1);

	/* search forward to the end of the word */
	scanseek(&p, cursor);
	do
	{
		scannext(&p);
	} while (p && (isalnum(*p) || *p == '_'));
	marksetoffset(cursor, markoffset(scanmark(&p)));

	/* clean up & return the front of the word */
	scanfree(&p);
	return &retmark;
}
