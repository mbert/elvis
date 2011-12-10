/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */
#define NSUBEXP  10

typedef struct regexp {
	char	*startp[NSUBEXP];/* start of text matching a subexpr */
	char	*endp[NSUBEXP];	/* end of a text matching a subexpr */
	char	*leavep;	/* pointer to text matching \= */
	int	minlen;		/* length of shortest possible match */
	char	first;		/* first character, if known; else \0 */
	char	bol;		/* boolean: must start at beginning of line? */
	char	program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

extern regexp *regcomp P_((char *));
extern int regexec P_((regexp *, char *, int));
extern void regsub P_((regexp *, char *, char *));
extern void regerror P_((char *));
