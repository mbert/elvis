/* regexp.c */

/* This file contains the code that compiles regular expressions and executes
 * them.  It supports the same syntax and features as vi's regular expression
 * code.  Specifically, the meta characters are:
 *	^	matches the beginning of a line
 *	$	matches the end of a line
 *	\<	matches the beginning of a word
 *	\>	matches the end of a word
 *	.	matches any single character
 *	[]	matches any character in a character class
 *	\@	matches the word at the cursor, if any
 *	\=	if searching, leaves the cursor here
 *	\(	delimits the start of a subexpression
 *	\)	delimits the end of a subexpression
 *	*	repeats the preceding 0 or more times 
 *	\+	repeats the preceding 1 or more times
 *	\?	repeats the preceding 0 or 1 times
 *	\{m\}	repeats the preceding m times
 *	\{m,\}	repeats the preceding m or more times
 *	\{m,n\}	repeats the preceding between m and n times
 * NOTE: You cannot follow a \) with a closure operator such as *
 *
 * The physical structure of a compiled regexp is as follows:
 *	- First, there is a one-byte value that says how many character classes
 *	  are used in this regular expression
 *	- Next, each character class is stored as a bitmap that is 256 bits
 *	  (32 bytes) long.
 *	- A mixture of literal characters and compiled meta characters follows.
 *	  This begins with M_BEGIN(0) and ends with M_END(0).  All meta chars
 *	  are stored as a \n followed by a one-byte code, so they take up two
 *	  bytes apiece.  Literal characters take up one byte apiece.  \n can't
 *	  be used as a literal character.
 */

#include <setjmp.h>
#include "elvis.h"


#if USE_PROTOTYPES
static BOOLEAN handlenamedclass(CHAR **textp, REG CHAR *bmap);
static CHAR *makeclass(REG CHAR *text, REG CHAR *bmap);
static int gettoken(CHAR **sptr, regexp *re);
static unsigned calcsize(CHAR *text, MARK cursor);
static int match1(regexp *re, REG _CHAR_ ch, REG int token);
static int match(regexp *re, MARK str, REG CHAR *prog, CHAR **here, BOOLEAN bol);
#endif


static CHAR	*previous;	/* the previous regexp, used when null regexp is given */


/* These are used to classify or recognize meta-characters */
#define META		'\0'
#define BASE_META(m)	((m) - 256)
#define INT_META(c)	((c) + 256)
#define IS_META(m)	((m) >= 256)
#define IS_CLASS(m)	((m) >= M_CLASS(0) && (m) <= M_CLASS(9))
#define IS_START(m)	((m) >= M_START(0) && (m) <= M_START(9))
#define IS_END(m)	((m) >= M_END(0) && (m) <= M_END(9))
#define IS_CLOSURE(m)	((m) >= M_SPLAT && (m) <= M_RANGE)
#define ADD_META(s,m)	(*(s)++ = META, *(s)++ = BASE_META(m))
#define GET_META(s)	(*(s) == META ? INT_META(*++(s)) : *s)

/* These are the internal codes used for each type of meta-character */
#define M_BEGLINE	256		/* internal code for ^ */
#define M_ENDLINE	257		/* internal code for $ */
#define M_BEGWORD	258		/* internal code for \< */
#define M_ENDWORD	259		/* internal code for \> */
#define M_ANY		260		/* internal code for . */
#define M_LEAVECURSOR	261		/* internal code for \= */
#define M_ATCURSOR	262		/* internal code for \@ */
#define M_SPLAT		263		/* internal code for * */
#define M_PLUS		264		/* internal code for \+ */
#define M_QMARK		265		/* internal code for \? */
#define M_RANGE		266		/* internal code for \{ */
#define M_NUL		267		/* internal code for a NUL character */
#define M_CLASS(n)	(268+(n))	/* internal code for [] */
#define M_START(n)	(278+(n))	/* internal code for \( */
#define M_END(n)	(288+(n))	/* internal code for \) */

#define CLASS_DIGIT	1
#define CLASS_UPPER	2
#define CLASS_LOWER	4
#define CLASS_ALPHA	(CLASS_UPPER|CLASS_LOWER)
#define CLASS_ALNUM	(CLASS_UPPER|CLASS_LOWER|CLASS_DIGIT)

/* These are used during compilation */
static int	class_cnt;	/* used to assign class IDs */
static int	start_cnt;	/* used to assign start IDs */
static int	end_stk[NSUBEXP];/* used to assign end IDs */
static int	end_sp;
static CHAR	*retext;	/* points to the text being compiled */

/* error-handling stuff */
jmp_buf	errorhandler;
#define FAIL(why)	regerror(why); longjmp(errorhandler, 1)



/* This function copies a regular expression into a dynamically allocated
 * string.  The (CHAR*) that refp refers to will be incremented to point to
 * the character after the closing delimiter, or to the newline or NUL if
 * there is no closing delimiter.  It may also be set to NULL if it hits the
 * end of the string/buffer being scanned.
 *
 * The calling function is responsible for calling safefree() on the string
 * when it is no longer needed.
 */
CHAR *regbuild(delim, refp)
	_CHAR_	delim;	/* the delimiter */
	CHAR	**refp;	/* reference to a CHAR* already used with scanalloc() */
{
	CHAR	*retext;
	BOOLEAN	inclass;
	int	phase;	/* context within a bracketted character class */
			/* 0: initially & after '-' of range: '-',']' literal */
			/* 1: after end of range: '-' literal, ']' ends */
			/* 2: normal: '-' denotes range, ']' ends */
			/* 3: in named class w/ extra brackets: ']' ends name */

	for (retext = NULL, inclass = False, phase = 0;
	     *refp && **refp && **refp != '\n' && (inclass || **refp != delim);
	     )
	{
		/* if not in a class, then '[' starts a class */
		if (!inclass && **refp == '[' && o_magic)
		{
			inclass = True;
			buildCHAR(&retext, '[');
			if (scannext(refp) && **refp == '^')
			{
				buildCHAR(&retext, '^');
				scannext(refp);
			}
			phase = 0;
			continue;
		}

		/* if not in a class, then backslashes need special treatment */
		if (!inclass && **refp == '\\')
		{
			/* move to next character, if any */
			if (!scannext(refp) || !**refp || **refp == '\n')
				continue;

			/* if the backslashed character is a delimiter, then
			 * add the delimiter without the backslash; else add
			 * both the backslash and the character.
			 */
			if (**refp != delim)
				buildCHAR(&retext, '\\');
			buildCHAR(&retext, **refp);
			scannext(refp);
			continue;
		}

		/* otherwise add the character to the retext */
		buildCHAR(&retext, **refp);

		/* if in class, we need to update phase */
		if (inclass)
		{
			if (**refp == '[' && scannext(refp))
			{
				if (**refp == ':')
					phase = 3; /* next will be class name */
				else if (phase == 0)
					phase = 1; /* next will be after range*/
				else
					phase = 2; /* next will be normal */
				continue;
			}
			else if (**refp == ']' && phase == 3)
				phase = 1; /* next will be char after range */
			else if (**refp == ']' && phase != 0)
				inclass = False; /* END OF CHARACTER CLASS */
			else if (**refp == '-' && phase == 2)
				phase = 0; /* next will be end of range */
			else if (phase == 0)
				phase = 1; /* next will be char after range */
			else if (phase != 3)
				phase = 2; /* next will be normal character */
		}

		/* advance to next character */
		scannext(refp);
	}

	/* if no characters, then return a dynamically allocated "" string */
	if (!retext)
		retext = (CHAR *)safealloc(1, sizeof(CHAR));

	/* if we hit a closing delimiter, then move past it */
	if (*refp && **refp == delim)
		scannext(refp);

	/* return the dynamic string */
	return retext;
}


/* This is a utility function for makeclass. It is factored out as makeclas
 * was becoming unwieldy. Returns True if a named class; False otherwise.
 * Should possibly detect errors in such constructs as [...[:fred:]...]
 */
static BOOLEAN handlenamedclass(textp, bmap)
	  CHAR        **textp;        /* character list (updates *textp) */
      REG CHAR        *bmap;          /* bitmap of selected characters */
{
	static struct clastran {
		char	cname[sizeof "xdigit:]"];
		size_t	nsize;
		int	include;
		enum {NONE=0, INVERT=1, ASCII=2, PRINT=4, BLANK=8} flags;
	} tran[] = {
	    { "alnum:]",  sizeof "alnum:]"  - 1, ELVCT_ALNUM, NONE        },
	    { "alpha:]",  sizeof "alpha:]"  - 1, ELVCT_ALPHA, NONE        },
	    { "ascii:]",  sizeof "ascii:]"  - 1, 0          , ASCII       },
	    { "blank:]",  sizeof "blank:]"  - 1, 0          , BLANK       },
	    { "cntrl:]",  sizeof "cntrl:]"  - 1, ELVCT_CNTRL, NONE        },
	    { "digit:]",  sizeof "digit:]"  - 1, ELVCT_DIGIT, NONE        },
	    { "graph:]",  sizeof "graph:]"  - 1, ELVCT_GRAPH, PRINT       },
	    { "lower:]",  sizeof "lower:]"  - 1, ELVCT_LOWER, NONE        },
	    { "print:]",  sizeof "print:]"  - 1, ELVCT_CNTRL, INVERT|PRINT},
	    { "punct:]",  sizeof "punct:]"  - 1, ELVCT_PUNCT, NONE        },
	    { "space:]",  sizeof "space:]"  - 1, ELVCT_SPACE, NONE        },
	    { "upper:]",  sizeof "upper:]"  - 1, ELVCT_UPPER, NONE        },
	    { "xdigit:]", sizeof "xdigit:]" - 1, ELVCT_XDIGIT,NONE        },
	};

	struct clastran *sp;

	REG CHAR        *text = *textp;
	REG int         i, incl;

	/* if obviously not a named class, then return False */
	if (text[0] != '[' || text[1] != ':')
		return False;

	/* search for the class in tran[] */
	for (sp = tran; sp < tran + QTY(tran); sp++)
	{
		if (CHARncmp(text+2, toCHAR(sp->cname), sp->nsize) == 0)
		{
			/* add the named class to the bitmap */
			for (i = 0; bmap && i < 256; i++)
			{
				/* begin by checking the ctype macros */
				incl = sp->include ? (sp->include & elvct_class[i]) : 1;
				/* invert if necessary */
				if (sp->flags & INVERT)
					incl = !incl;

				/* the BLANK, ASCII, and PRINT flags eliminate
				 * some characters.
				 */
				if (((sp->flags & BLANK) && i != '\t' && i != ' ')
				 || ((sp->flags & ASCII) && i >= 128))
					incl = 0;
				if (sp->flags & PRINT)
				{
					switch (o_nonascii)
					{
					  case 'a':	break;
					  case 'n':
					  	if (i >= 128)
					  		incl = 0;
					  	break;

					  case 'm':
					  case 's':
						if (i >= 128 && i < 160)
							incl = 0;
						break;
					}
				}
				if (incl)
				{
					bmap[i >> 3] |= 1 << (i & 7);
				}
			}

			/* move past this named class */
			*textp += sp->nsize + 2;
			return True;
		}
	}

	/* if we get here, then it was an unknown named class */
	FAIL("unknown named character class");
	/*NOTREACHED*/
}

/* This function builds a bitmap for a particular class.  "text" points
 * to the start of the class string, and "bmap" is a pointer to memory
 * which can be used to store the bitmap of the class.  If "bmap" is NULL,
 * then the class will be parsed but bitmap will not be generated.
 */
static CHAR *makeclass(text, bmap)
	REG CHAR	*text;	/* character list */
	REG CHAR	*bmap;	/* bitmap of selected characters */
{
	REG int		i;
	int		complement = 0;
	BOOLEAN		first;


	/* zero the bitmap */
	for (i = 0; bmap && i < 32; i++)
	{
		bmap[i] = 0;
	}

	/* see if we're going to complement this class */
	if (*text == '^')
	{
		text++;
		complement = 1;
	}

	/* add in the characters */
	for (first = True; *text && (first || *text != ']'); first = False)
	{
		/* is this a span of characters? */
		if (text[1] == '-' && text[2])
		{
			/* spans can't be backwards */
			if (text[0] > text[2])
			{
				FAIL("backwards span in []");
			}

			/* add each character in the span to the bitmap */
			for (i = text[0]; bmap && (unsigned)i <= text[2]; i++)
			{
				bmap[i >> 3] |= (1 << (i & 7));
			}

			/* move past this span */
			text += 3;
		}
		else if (!handlenamedclass(&text, bmap))
		{
			/* add this single character to the span */
			i = *text++;
			if (bmap)
			{
				bmap[i >> 3] |= (1 << (i & 7));
			}
		}
	}

	/* make sure the closing ] isn't missing */
	if (*text++ != ']')
	{
		FAIL("] missing");
	}

	/* if we're supposed to complement this class, then do so */
	if (complement && bmap)
	{
		for (i = 0; i < 32; i++)
		{
			bmap[i] = ~bmap[i];
		}
	}

	return text;
}




/* This function gets the next character or meta character from a string.
 * The pointer is incremented by 1, or by 2 for \-quoted characters.  For [],
 * a bitmap is generated via makeclass() (if re is given), and the
 * character-class text is skipped.  "sptr" is a pointer to the pointer
 * which is used for scanning the text source of the regular expression,
 * and "re" is a pointer to a buffer which will be used to store the
 * compiled regular expression, or NULL if it hasn't been allocated yet.
 */
static int gettoken(sptr, re)
	CHAR	**sptr;	/* pointer to the text scanning pointer */
	regexp	*re;	/* pointer to the regexp being built, or NULL */
{
	int	c;
	CHAR	*subexpr;

	c = **sptr;
	if (!c)
	{
		return c;
	}
	++*sptr;
	if (c == '\\')
	{
		c = **sptr;
		++*sptr;
		switch (c)
		{
		  case '<':
			return M_BEGWORD;

		  case '>':
			return M_ENDWORD;

		  case '(':
			if (start_cnt >= NSUBEXP)
			{
				FAIL("too many \\(s");
			}
			end_stk[end_sp++] = start_cnt;
			return M_START(start_cnt++);

		  case ')':
			if (end_sp <= 0)
			{
				FAIL("mismatched \\)");
			}
			return M_END(end_stk[--end_sp]);

		  case '*':
			return (o_magic ? c : M_SPLAT);

		  case '.':
			return (o_magic ? c : M_ANY);

		  case '+':
			return M_PLUS;

		  case '?':
			return M_QMARK;

		  case '=':
			return M_LEAVECURSOR;

		  case '@':
			return M_ATCURSOR;

		  case '{':
			return M_RANGE;

		  case '0':
			return M_NUL;

		  case 'a':
			return '\007';	/* BEL */

		  case 'b':
			return '\b';	/* BS */

		  case 'e':
			return '\033';	/* ESC */

		  case 'f':
			return '\f';	/* FF */

		  case 'n':
			FAIL("\\n doesn't work in regexp");

		  case 'r':
			return '\r';	/* CR */

		  case 't':
			return '\t';	/* TAB */

		  case 'd':
			subexpr = toCHAR("[[:digit:]]");
			return gettoken(&subexpr, re);

		  case 'D':
			subexpr = toCHAR("[^[:digit:]]");
			return gettoken(&subexpr, re);

		  case 'p':
			subexpr = toCHAR("[[:print:]]");
			return gettoken(&subexpr, re);

		  case 'P':
			subexpr = toCHAR("[^[:print:]]");
			return gettoken(&subexpr, re);

		  case 's':
			subexpr = toCHAR("[[:space:]]");
			return gettoken(&subexpr, re);

		  case 'S':
			subexpr = toCHAR("[^[:space:]]");
			return gettoken(&subexpr, re);

		  case 'w':
			subexpr = toCHAR("[[:alnum:]_]");
			return gettoken(&subexpr, re);

		  case 'W':
			subexpr = toCHAR("[^[:alnum:]_]");
			return gettoken(&subexpr, re);

		  default:
			return c;
		}
	}
	else if (o_magic)
	{
		switch (c)
		{
		  case '^':
			if (*sptr == retext + 1)
			{
				return M_BEGLINE;
			}
			return c;

		  case '$':
			if (!**sptr)
			{
				return M_ENDLINE;
			}
			return c;

		  case '.':
			return M_ANY;

		  case '*':
			return M_SPLAT;

		  case '[':
			/* make sure we don't have too many classes */
			if (class_cnt >= 10)
			{
				FAIL("too many []s");
			}

			/* process the character list for this class */
			if (re)
			{
				/* generate the bitmap for this class */
				*sptr = makeclass(*sptr, re->program + 1 + 32 * class_cnt);
			}
			else
			{
				/* skip to end of the class */
				*sptr = makeclass(*sptr, (CHAR *)0);
			}
			return M_CLASS(class_cnt++);

		  default:
			return c;
		}
	}
	else	/* unquoted nomagic */
	{
		switch (c)
		{
		  case '^':
			if (*sptr == retext + 1)
			{
				return M_BEGLINE;
			}
			return c;

		  case '$':
			if (!**sptr)
			{
				return M_ENDLINE;
			}
			return c;

		  default:
			return c;
		}
	}
	/*NOTREACHED*/
}




/* This function calculates the number of bytes that will be needed for a
 * compiled regexp.  Its argument is the uncompiled version.  It is not clever
 * about catching syntax errors; that is done in a later pass.  "text" is
 * a pointer to the source text of the regular expression.
 */
static unsigned calcsize(text, cursor)
	CHAR	*text;	/* source code of the regexp */
	MARK	cursor;	/* cursor position, to support "\@" */
{
	unsigned	size;
	int		token;
	MARKBUF		tmpb;
	MARK		tmp;

	retext = text;
	class_cnt = 0;
	start_cnt = 1;
	end_sp = 0;
	size = 5;
	while ((token = gettoken(&text, (regexp *)0)) != 0)
	{
		if (IS_CLASS(token))
		{
			size += 34;
		}

		else if (token == M_RANGE)
		{
			size += 4;
			while ((token = gettoken(&text, (regexp *)0)) != 0
			    && token != '}')
			{
			}
			if (!token)
			{
				return size;
			}
		}
		else if (token == M_ATCURSOR)
		{
			tmpb = *cursor;
			tmp = wordatcursor(&tmpb);
			if (tmp)
			{
				assert(markoffset(&tmpb) > markoffset(tmp));
				size += markoffset(&tmpb) - markoffset(tmp);
			}
		}
		else if (IS_META(token))
		{
			size += 2;
		}
		else
		{
			size++;
		}
	}

	return size;
}



/* This function compiles a regexp.  "exp" is the source text of the regular
 * expression.
 */
regexp *regcomp(exp, cursor)
	CHAR	*exp;	/* source code of the regular expression */
	MARK	cursor;	/* cursor position, to support "\@" */
{
	int		needfirst;
	unsigned	size;
	int		token;
	int		peek;
	CHAR		*scan, *build;
	regexp		*re;
	int		from;
	int		to;
	int		digit;
#ifdef DEBUG
	int		calced;
#endif
	MARK		tmp;


	/* prepare for error handling */
	re = (regexp *)0;
	if (setjmp(errorhandler))
	{
		if (re)
		{
			safefree(re);
		}
		return (regexp *)0;
	}

	/* if an empty regexp string was given, use the previous one */
	if (*exp == 0)
	{
		if (!previous)
		{
			FAIL("no previous regexp");
		}
		exp = previous;
	}
	else if (o_saveregexp) /* non-empty regexp given, so remember it */
	{
		if (previous)
			safefree(previous);
		previous = (CHAR *)safekept((int)(CHARlen(exp) + 1), sizeof(CHAR));
		if (previous)
			CHARcpy(previous, exp);
	}

	/* allocate memory */
	class_cnt = 0;
	start_cnt = 1;
	end_sp = 0;
	retext = exp;
#ifdef DEBUG
	calced = calcsize(exp, cursor);
	size = calced + sizeof(regexp);
#else
	size = calcsize(exp, cursor) + sizeof(regexp) + 10; /* !!! 10 bytes for slop */
#endif
#ifdef lint
	re = (regexp *)0;
#else
	re = (regexp *)safekept((int)size, sizeof(CHAR));
#endif
	if (!re)
	{
		FAIL("not enough memory for this regexp");
	}

	/* compile it */
	build = &re->program[1 + 32 * class_cnt];
	re->program[0] = class_cnt;
	for (token = 0; token < NSUBEXP; token++)
	{
		re->startp[token] = re->endp[token] = -1;
	}
	re->leavep = -1;
	re->first = 0;
	re->bol = False;
	re->minlen = 0;
	needfirst = 1;
	class_cnt = 0;
	start_cnt = 1;
	end_sp = 0;
	retext = exp;
	for (token = M_START(0), peek = gettoken(&exp, re);
	     token;
	     token = peek, peek = gettoken(&exp, re))
	{
		/* special processing for the closure operator */
		if (IS_CLOSURE(peek))
		{
			/* detect misuse of closure operator */
			if (IS_START(token))
			{
				FAIL("closure operator follows nothing");
			}
			else if (IS_META(token) && token != M_ANY && token != M_NUL && !IS_CLASS(token))
			{
				FAIL("closure operators can only follow a normal character or . or []");
			}

			/* if \{ \} then read the range */
			if (peek == M_RANGE)
			{
				from = 0;
				for (digit = gettoken(&exp, re);
				     !IS_META(digit) && isdigit(digit);
				     digit = gettoken(&exp, re))
				{
					from = from * 10 + digit - '0';
				}
				/*{*/
				if (digit == '}')
				{
					to = from;
				}
				else if (digit == ',')
				{
					to = 0;
					for (digit = gettoken(&exp, re);
					     !IS_META(digit) && isdigit(digit);
					     digit = gettoken(&exp, re))
					{
						to = to * 10 + digit - '0';
					}
					if (to == 0)
					{
						to = 255;
					}
				}
				/*{*/
				if (digit != '}')
				{
					FAIL("bad characters after \\{"); /*}*/
				}
				else if (to < from || to == 0 || from >= 255)
				{
					FAIL("invalid range for \\{ \\}");
				}
				re->minlen += from;
			}
			else if (peek != M_SPLAT)
			{
				re->minlen++;
			}

			/* it is okay -- make it prefix instead of postfix */
			ADD_META(build, peek);
			if (peek == M_RANGE)
			{
				*build++ = from;
				*build++ = (to < 255 ? to : 255);
			}
			

			/* take care of "needfirst" - is this the first char? */
			if (needfirst && peek == M_PLUS && !IS_META(token))
			{
				re->first = token;
			}
			needfirst = 0;

			/* we used "peek" -- need to refill it */
			peek = gettoken(&exp, re);
			if (IS_CLOSURE(peek))
			{
				FAIL("* or \\+ or \\? doubled up");
			}
		}
		else if (!IS_META(token))
		{
			/* normal char is NOT argument of closure */
			if (needfirst)
			{
				re->first = token;
				needfirst = 0;
			}
			re->minlen++;
		}
		else if (token == M_ANY || IS_CLASS(token) || token == M_NUL)
		{
			/* . or [] is NOT argument of closure */
			needfirst = 0;
			re->minlen++;
		}

		/* the "token" character is not closure -- process it normally */
		if (token == M_BEGLINE)
		{
			/* set the BOL flag instead of storing M_BEGLINE */
			re->bol = True;
		}
		else if (token == M_ATCURSOR)
		{
			tmp = wordatcursor(cursor);
			if (!tmp)
			{
				FAIL("cursor not on word");
			}
			for (scanalloc(&scan, tmp);
			     scan && markoffset(scanmark(&scan)) < markoffset(cursor);
			     scannext(&scan))
			{
				*build++ = *scan;
			}
			scanfree(&scan);
		}
		else if (IS_META(token))
		{
			ADD_META(build, token);
		}
		else
		{
			*build++ = token;
		}
	}

	/* end it with a \) which MUST MATCH the opening \( */
	ADD_META(build, M_END(0));
	if (end_sp > 0)
	{
		FAIL("not enough \\)s");
	}

#ifdef FEATURE_LITRE
	/* Detect whether this is a literal regexp.  Literal regexps contain
	 * no metacharacters except M_BEGIN(0) and M_END(0).
	 */
	for (scan = &re->program[2 + 32 * re->program[0]]; *scan != META; scan++)
	{
	}
	if (GET_META(scan) == M_END(0))
		re->literal = True;
	else
		re->literal = False;
#endif

#ifdef DEBUG
	if ((int)(build - re->program) != calced)
	{
		msg(MSG_WARN, "[dd]regcomp error: calced=$1, actual=$2", calced, (int)(build - re->program));
		getkey(0);
	}
#endif

	return re;
}


/* allocate a new copy of a regular expression */
regexp *regdup(re)
	regexp	*re;
{
	CHAR	*p;
	int	i;
	regexp	*newp;

	/* count the size of the regular expression's program */
	for (p = &re->program[1 + 32 * re->program[0]];
	     GET_META(p) != M_END(0);
	     p++)
	{
	}
	i = (int)(p - re->program);

	/* allocate memory */
	i = sizeof(regexp) + i * sizeof re->program[0];
	newp = safealloc(1, i);
	memcpy(newp, re, i);
	return newp;
}


/*---------------------------------------------------------------------------*/


/* This function checks for a match between a character and a token which is
 * known to represent a single character.  It returns 0 if they match, or
 * 1 if they don't.  "re" is a pointer to the compiled regular expression,
 * "ch" is the next character or '\n' at the end of the line, and "token"
 * is the particular part of the regular expression which this character
 * is supposed to match.
 */
static int match1(re, ch, token)
	regexp		*re;	/* regular expression being matched */
	REG _CHAR_	ch;	/* character from searched text */
	REG int		token;	/* token from regular expression */
{
	if (ch == '\n')
	{
		/* the end of a line can't match any regexp of width 1 */
		return 1;
	}
	if (token == M_ANY)
	{
		return 0;
	}
	else if (token == M_NUL)
	{
		if (ch == '\0')
			return 0;
	}
	else if (IS_CLASS(token))
	{
		if (re->program[1 + 32 * (token - M_CLASS(0)) + (ch >> 3)] & (1 << (ch & 7)))
			return 0;
	}
	else if ((_char_)ch == token || (o_ignorecase && tolower((_char_)ch) == tolower(token)))
	{
		return 0;
	}
	return 1;
}



/* This function checks characters up to and including the next closure, at
 * which point it does a recursive call to check the rest of it.  This function
 * returns 0 if everything matches, or 1 if something doesn't match.
 */
static int match(re, str, prog, here, bol)
	regexp		*re;	/* the regular expression being matched */
	MARK		str;	/* string to be compared against regexp */
	REG CHAR	*prog;	/* pointer into body of compiled regexp */
	CHAR		**here;	/* pointer into the "str" string */
	BOOLEAN		bol;	/* if True, "str" is the start of a line */
{
	REG int		token;	/* the token pointed to by prog */
	REG long	nmatched;/* counter, used during closure matching */ 
	REG int		closure;/* the token denoting the type of closure */
	long		from;	/* minimum number of matches in closure */
	long		to;	/* maximum number of matches in closure */
	CHAR		*there;	/* temporary scan, starts equal to "here" */
	CHAR		ch;	/* character from the buffer being searched */

	/* use a local copy of the scanning pointer */
	scandup(&there, here);

	/* compare a single character or metacharacter */
	for (token = GET_META(prog); !IS_CLOSURE(token); prog++, token = GET_META(prog))
	{
		/* if we hit the end of the buffer, fail */
		if (!there)
		{
			scanfree(&there);
			return 1;
		}

		switch (token)
		{
		/*case M_BEGLINE: can't happen; re->bol is used instead */
		  case M_ENDLINE:
			if (!there || *there != '\n')
			{
				scanfree(&there);
				return 1;
			}
			break;

		  case M_BEGWORD:
			if (!bol || (there && markoffset(scanmark(&there))) != markoffset(str))
			{
				scanprev(&there);
				ch = *there;
				scannext(&there);
				if (ch == '_' || isalnum(ch))
				{
					scanfree(&there);
					return 1;
				}
			}
			break;

		  case M_ENDWORD:
			ch = *there;
			if (ch == '_' || isalnum(ch))
			{
				scanfree(&there);
				return 1;
			}
			break;

		  case M_LEAVECURSOR:
			re->leavep = markoffset(scanmark(&there));
			break;

		  case M_START(0):
		  case M_START(1):
		  case M_START(2):
		  case M_START(3):
		  case M_START(4):
		  case M_START(5):
		  case M_START(6):
		  case M_START(7):
		  case M_START(8):
		  case M_START(9):
			re->startp[token - M_START(0)] = markoffset(scanmark(&there));
			break;

		  case M_END(0):
		  case M_END(1):
		  case M_END(2):
		  case M_END(3):
		  case M_END(4):
		  case M_END(5):
		  case M_END(6):
		  case M_END(7):
		  case M_END(8):
		  case M_END(9):
			re->endp[token - M_END(0)] = markoffset(scanmark(&there));
			if (token == M_END(0))
			{
				scanfree(&there);
				return 0;
			}
			break;

		  default: /* literal, M_CLASS(n), M_ANY, or M_NUL */
		  	assert(there != NULL);
			if (match1(re, *there, token) != 0)
			{
				scanfree(&there);
				return 1;
			}
			scannext(&there);
		}
	}

	/* C L O S U R E */

	/* step 1: see what we have to match against, and move "prog" to point
	 * to the remainder of the compiled regexp.
	 */
	closure = token;
	prog++;
	switch (closure)
	{
	  case M_SPLAT:
		from = 0;
		to = INFINITY;
		break;

	  case M_PLUS:
		from = 1;
		to = INFINITY;
		break;

	  case M_QMARK:
		from = 0;
		to = 1;
		break;

	  case M_RANGE:
		from = *prog++;
		to = *prog++;
		if (to == 255)
		{
			to = INFINITY;
		}
		break;
	}
	token = GET_META(prog);
	prog++;

	/* step 2: see how many times we can match that token against the string */
	for (nmatched = 0;
	     nmatched < to
		&& there
		&& match1(re, *there, token) == 0;
	     nmatched++, scannext(&there))
	{
	}

	/* step 3: try to match the remainder, and back off if it doesn't */
	if (!there)
	{
		scanfree(&there);
		return 1;
	}
	while (nmatched >= from && match(re, str, prog, &there, False) != 0)
	{
		/* back off */
		nmatched--;
		scanprev(&there);
		if (!there)
		{
			scanfree(&there);
			return 1;
		}
	}
	scanfree(&there);

	/* so how did it work out? */
	if (nmatched >= from)
	{
		return 0;
	}
	return 1;
}



/* This function searches through a string for text that matches a regexp.
 * "re" is the compiled recular expression, "str" is the string to compare
 * against "re", and "bol" is a flag indicating whether "str" points to the
 * start of a line or not.  Returns 1 for match, or 0 for mismatch.
 */
int regexec(re, str, bol)
	regexp	*re;	/* a compiled regular expression */
	MARK	str;	/* a string to compare against the regexp */
	BOOLEAN	bol;	/* if True, "str" is the beginning of a string */
{
	CHAR	*prog;	/* the entry point of re->program */
	int	len;	/* length of the string */
	CHAR	*here;	/* pointer used for scanning text */
#ifdef FEATURE_LITRE
	int	right;	/* contiguous characters to right of str */
	MARKBUF	m;
#endif


	/* find the remaining length of this line */
	scanalloc(&prog, str);
#ifdef FEATURE_LITRE
	right = scanright(&prog);
	for (len = 0; len < right && prog[len] != '\n'; len++)
	{
	}
	if (len >= right)
	{
		scanseek(&prog, marktmp(m, markbuffer(str), markoffset(str) + len));
		for (; prog && *prog != '\n'; scannext(&prog), len++)
		{
		}
		re->nextlinep = (prog
			? markoffset(scanmark(&prog)) + 1
			: o_bufchars(markbuffer(str)));
	}
	else
		re->nextlinep = markoffset(str) + len + 1;
#else
	for (len = 0; prog && *prog != '\n'; scannext(&prog), len++)
	{
	}
	re->nextlinep = (prog
		? markoffset(scanmark(&prog)) + 1
		: o_bufchars(markbuffer(str)));
#endif

	scanfree(&prog);

	/* if must start at the beginning of a line, and this isn't, then fail */
	if (re->bol && !bol)
	{
		return 0;
	}

	/* NOTE: If we ever support alternation (the \| metacharacter) then
	 * we'll need to reset startp[] and endp[] to -1L.
	 */
	re->leavep = -1;

	/* find the first token of the compiled regular expression */
	prog = re->program + 1 + 32 * re->program[0];

	/* search for the regexp in the string */
	scanalloc(&here, str);
#ifdef FEATURE_LITRE
	if (re->literal && re->bol && !o_ignorecase && right >= re->minlen)
	{
		/* must match exactly, right here, and we know we have enough
		 * of this line for the entire match to be in contiguous memory.
		 */
		prog += 2;
		if (CHARncmp(prog, here, re->minlen))
		{
			/* didn't match */
			scanfree(&here);
			return 0;
		}

		/* hey, it did match!  Remember the endpoints */
		re->startp[0] = re->leavep = markoffset(str);
		re->endp[0] = re->startp[0] + re->minlen;
		re->buffer = markbuffer(str);
		scanfree(&here);
		return 1;
	}
	else if (re->literal && !o_ignorecase && right >= len)
	{
		/* The regexp must match exactly, anywhere before the end
		 * of this line.  We know the entire line is in contiguous
		 * memory, so we can use CHARncmp() to check for the string,
		 * and we can use here++ instead of scannext(&here) to
		 * increment the pointer in the for() statement.
		 */
		prog += 2;
		for (; len >= re->minlen; len--, here++)
		{
			if (*prog == *here && !CHARncmp(prog, here, re->minlen))
			{
				/* Found a match!  Remember the endpoints */
				re->startp[0] = re->leavep = markoffset(scanmark(&here));
				re->endp[0] = re->startp[0] + re->minlen;
				re->buffer = markbuffer(str);
				scanfree(&here);
				return 1;
			}
		}

		/* didn't match */
		scanfree(&here);
		return 0;
	}
	else 
#endif /* FEATURE_LITRE */
	if (re->bol)
	{
		/* must occur at BOL */
		if ((re->first
			&& match1(re, scanchar(str), re->first))/* wrong first letter? */
		 || len < re->minlen				/* not long enough? */
		 || match(re, str, prog, &here, bol))		/* doesn't match? */
		{
			scanfree(&here);
			return 0;				/* THEN FAIL! */
		}
	}
	else if (!o_ignorecase)
	{
		/* can occur anywhere in the line, noignorecase */
		for (;
		     here && ((re->first && re->first != *here)
			|| match(re, str, prog, &here, bol));
		     len--, bol = False)
		{
			scannext(&here);
			if (!here || len <= re->minlen)
			{
				scanfree(&here);
				return 0;
			}
		}
	}
	else
	{
		/* can occur anywhere in the line, ignorecase */
		for (;
		     here && ((re->first && match1(re, *here, (int)re->first))
			|| match(re, str, prog, &here, bol));
		     len--, bol = False)
		{
			scannext(&here);
			if (!here || len <= re->minlen)
			{
				scanfree(&here);
				return 0;
			}
		}
	}
	scanfree(&here);

	/* if we didn't fail, then we must have succeeded */
	re->buffer = markbuffer(str);
	if (re->leavep == -1)
	{
		re->leavep = re->startp[0];
	}
	return 1;
}


void regerror(str)
	char	*str;	/* an error message */
{
	msg(MSG_ERROR, str);
}
