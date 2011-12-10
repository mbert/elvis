/* regexp.h */

#define NSUBEXP  10	/* max # of subexpressions, plus 1 for whole expr */

typedef struct regexp {
	long	startp[NSUBEXP];/* start of text matching a subexpr */
	long	endp[NSUBEXP];	/* end of a text matching a subexpr */
	long	leavep;		/* offset of text matching \= */
	long	nextlinep;	/* offset of start of following line */
	BUFFER	buffer;		/* buffer that the above offsets refer to */
	int	minlen;		/* length of shortest possible match */
	CHAR	first;		/* first character, if known; else \0 */
	BOOLEAN	bol;		/* must start at beginning of line? */
	CHAR	program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

BEGIN_EXTERNC
extern regexp	*regcomp P_((CHAR *retext, MARK cursor));
extern int	regexec P_((regexp *re, MARK from, BOOLEAN bol));
extern CHAR	*regtilde P_((CHAR *newp));
extern CHAR	*regsub P_((regexp *re, CHAR *newp, BOOLEAN doit));
extern void	regerror P_((char *errmsg));
END_EXTERNC

#ifndef REG
# define REG /* as nothing */
#endif
