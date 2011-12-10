/* dmsyntax.c */
/* Copyright 1995 by Steve Kirkendall */

char id_dmsyntax[] = "$Id: dmsyntax.c,v 2.24 1996/09/21 01:21:36 steve Exp $";

#include "elvis.h"
#ifdef DISPLAY_SYNTAX

#if USE_PROTOTYPES
static CHAR *iskeyword(CHAR *word);
static void addkeyword(CHAR *word, _char_ font, BOOLEAN doesregexp, BOOLEAN doesregsub);
static CHAR **fetchline(void);
static DMINFO *init(WINDOW win);
static void term(DMINFO *info);
static MARK setup(MARK top, long cursor, MARK bottom, DMINFO *info);
static MARK image(WINDOW w, MARK line, DMINFO *info, void (*draw)(CHAR *p, long qty, _char_ font, long offset));
static CHAR *tagatcursor(WINDOW win, MARK cursor);
static MARK tagload(CHAR *tagname, MARK from);
#endif

/* These are the font names, used in the option descriptions below */
char fontnames[] = "normal bold emphasized italic underlined fixed";

/* These are the descriptions and values of some global options */
static OPTDESC globdesc[] =
{
	{"commentfont","cfont",	opt1string,	optisoneof,	fontnames },
	{"stringfont","sfont",	opt1string,	optisoneof,	fontnames },
	{"keywordfont","kfont",	opt1string,	optisoneof,	fontnames },
	{"functionfont","ffont",opt1string,	optisoneof,	fontnames },
	{"variablefont","vfont",opt1string,	optisoneof,	fontnames },
	{"prepfont", "pfont",	opt1string,	optisoneof,	fontnames },
	{"otherfont", "ofont",	opt1string,	optisoneof,	fontnames },
	{"includepath", "inc",	optsstring,	optisstring }
};
static OPTVAL globval[QTY(globdesc)];
#define o_commentfont	globval[0].value.character
#define o_stringfont	globval[1].value.character
#define o_keywordfont	globval[2].value.character
#define o_functionfont	globval[3].value.character
#define o_variablefont	globval[4].value.character
#define o_prepfont	globval[5].value.character
#define o_otherfont	globval[6].value.character
#define o_includepath	globval[7].value.string

/* This data type is used to denote a token type.  Values of this type will
 * be used as indicies into the cfont[] array, below, to determine which
 * font each language element should use.  The last symbol in the list must
 * be PUNCT, because the declaration of cfont[] depends on this.
 */
typedef enum
{
	COMMENT, COMMENT2, STRING, CHARACTER, REGEXP, REGSUB, KEYWORD,
	FIRSTPUNCT, SECONDPUNCT, FUNCTION, VARIABLE, PREP, PREPWORD, PREPQUOTE,
	OTHER, PUNCT
} TOKENTYPE;

/* This structure stores information about the current language's syntax.
 * Each window has its own copy of this, so different windows which happen
 * to be in "syntax" mode can each display different languages.
 */
typedef struct
{
	/* info about the current parsing state */
	TOKENTYPE token;	/* used during parsing */

	/* info from the "elvis.syn" file */
	CHAR	**keyword[256];	/* hash table of keyword names */
	CHAR	function;	/* character used for function calls */
	CHAR	strbegin;	/* string start-quote character */
	CHAR	strend;		/* string end-quote character */
	CHAR	charbegin;	/* character quote character */
	CHAR	charend;	/* character quote character */
	CHAR	preprocessor;	/* first character of preprocessor directives */
	CHAR	pqbegin;	/* preprocessor start-quote character */
	CHAR	pqend;		/* preprocessor start-quote character */
	CHAR	comment[2];	/* start of one-line comment */
	CHAR	combegin[2];	/* start of multi-line comment */
	CHAR	comend[2];	/* end of multi-line comment */
	BOOLEAN	allcaps;	/* uppercase words are "other" */
	BOOLEAN	initialcaps;	/* mixed case words starting with upper are "other" */
	BOOLEAN	mixedcaps;	/* mixed case words starting with lower are "other" */
	BOOLEAN	finalt;		/* words ending with "_t" are "other" */
	BOOLEAN	initialpunct;	/* words starting with punctuation */
	BOOLEAN	ignorecase;	/* keywords may be uppercase */
	char	wordbits[256];	/* which chars can appear in a word */
} SINFO;
#define STARTWORD		0x01	/* can start a word */
#define INWORD			0x02	/* can occur within a word */
#define PUNCTWORD		0x04	/* may be first punct of 2-punct word */
#define DELIMREGEXP		0x08	/* can delimit regular expression */
#define USEREGEXP		0x10	/* occurs before possible regexp */
#define USEREGSUB		0x20	/* occurs before possible regsub */
#define isstartword(si,c)	((si)->wordbits[(CHAR)(c)] & STARTWORD)
#define isinword(si,c)		((si)->wordbits[(CHAR)(c)] & INWORD)
#define ispunctword(si,c)	((si)->wordbits[(CHAR)(c)] & PUNCTWORD)
#define isregexp(si,c)		((si)->wordbits[(CHAR)(c)] & DELIMREGEXP)
#define isbeforeregexp(si,c)	((si)->wordbits[(CHAR)(c)] & USEREGEXP)
#define isbeforeregsub(si,c)	((si)->wordbits[(CHAR)(c)] & USEREGSUB)
#define wordbeforeregexp(w)	((w)[-2] & USEREGEXP)
#define wordbeforeregsub(w)	((w)[-2] & USEREGSUB)
#define wordfont(w)		((char)((w)[-1]))



/* This array stores the fonts to be used with the TOKENTYPES, above.  It is
 * initialized by the setup() function each time the screen is redrawn,
 * to reflect the values of the "font" options above.
 */
static char cfont[PUNCT + 1];
static SINFO *sinfo;

/* This macro computes a hash value for a word, which is used for looking
 * the word up in the SINFO.keyword[] table.  The word is known to be at least
 * one character long and terminated with a '\0', so word[1] is guaranteed to
 * be valid and consistent.
 */
#define KWHASH(word)	(((word)[0] & 0x1f) ^ (((word)[1] & 0x03) << 5))


/* This function returns a pointer to the hashed keyword description if the
 * given word is in fact a keyword.  Otherwise it returns NULL.
 */
static CHAR *iskeyword(word)
	CHAR	*word;	/* pointer to word */
{
	int	hash;
	int	i, j;

	/* compute a hash value for this word */
	hash = KWHASH(word);

	/* if no keywords have that hash value, then return False right away */
	if (!sinfo->keyword[hash])
		return NULL;

	/* try to find the word in the list of keywords */
	if (sinfo->ignorecase)
	{
		/* not case sensitive */
		for (i = 0; sinfo->keyword[hash][i]; i++)
		{
			for (j = 0; toupper(sinfo->keyword[hash][i][j]) == toupper(word[j]); j++)
			{
				if (!sinfo->keyword[hash][i][j])
					return sinfo->keyword[hash][i];
			}
		}
	}
	else
	{
		/* case sensitive */
		for (i = 0; sinfo->keyword[hash][i]; i++)
		{
			if (!CHARcmp(sinfo->keyword[hash][i], word))
			{
				return sinfo->keyword[hash][i];
			}
		}
	}

	/* we didn't find it in the keyword list */
	return NULL;
}

static void addkeyword(word, font, doesregexp, doesregsub)
	CHAR	*word;		/* a keyword */
	_char_	font;		/* font, or '\0' for default */
	BOOLEAN	doesregexp;	/* can keyword be followed by a regexp? */
	BOOLEAN	doesregsub;	/* can keyword be followed by regexp+regsub? */
{
	int	hash;		/* hash value of word */
	CHAR	*keyword;	/* entry describing the current keyword */
	CHAR	**hashed;	/* new list for a hash table slot */
	int	i;

	/* see if the keyword is already in the list */
	keyword = iskeyword(word);
	if (!keyword)
	{
		/* no, we need to add it... */
		hash = KWHASH(word);
		if (sinfo->keyword[hash])
		{
			/* allocate a new, larger copy of the hash array */
			for (i = 0; sinfo->keyword[hash][i]; i++)
			{
			}
			hashed = safealloc(i + 2, sizeof(CHAR *));
			for (i = 0; sinfo->keyword[hash][i]; i++)
			{
				hashed[i] = sinfo->keyword[hash][i];
			}
			safefree(sinfo->keyword[hash]);
			sinfo->keyword[hash] = hashed;
		}
		else
		{
			/* create the first hash list */
			sinfo->keyword[hash] = safealloc(2, sizeof(CHAR *));
			i = 0;
		}

		/* append the new keyword to the list */
		keyword = safealloc(CHARlen(word) + 3, sizeof(CHAR));
		*keyword++ = '\0'; /* no special bits yet */
		*keyword++ = '\0'; /* no special font yet */
		CHARcpy(keyword, word);
		sinfo->keyword[hash][i] = keyword;
		sinfo->keyword[hash][i + 1] = NULL;
	}

	/* set the word's attributes */
	if (font)
		keyword[-1] = font;
	if (doesregexp)
		keyword[-2] |= USEREGEXP;
	if (doesregsub)
		keyword[-2] |= USEREGEXP | USEREGSUB;

	/* if not a normal word, then mark the character for extra checking */
	if (!isstartword(sinfo, keyword[0]) ||
		(keyword[1] && !isinword(sinfo, keyword[1])))
	{
		sinfo->wordbits[keyword[0]] |= PUNCTWORD;
	}
}


/* read a line from a file via ioread(), and parse it into words */
static CHAR **fetchline()
{
	static CHAR	line[500];
	static CHAR	*word[50];
	CHAR		ch;
	int		w, l;
	BOOLEAN		inword;
	int		nread;

	do
	{
		for (w = l = 0, inword = False;
		     (nread = ioread(&ch, 1)) == 1 && ch != '\n';
		     )
		{
			if (isspace(ch))
			{
				if (inword)
				{
					line[l++] = '\0';
					inword = False;
				}
			}
			else
			{
				if (!inword)
				{
					word[w++] = &line[l];
					inword = True;
				}
				line[l++] = ch;
			}
		}
		if (inword)
			line[l] = '\0';
		word[w] = NULL;
	} while (nread == 1 && (!word[0] || *word[0] == '#'));
	return nread==1 ? word : NULL;
}

/* This function checks whether a given file name's extension is listed in
 * the "lib/elvis.syn" file.  Returns True if known, or False otherwise.
 */
BOOLEAN dmsknown(filename)
	char	*filename;
{
	CHAR	**values;
	char	*synname;
	int	len;
	int	i, j;

	values = NULL;
	synname = iopath(tochar8(o_elvispath), SYNTAX_FILE, False);
	if (synname && ioopen(synname, 'r', False, False, False))
	{
		/* locate an "extension" line that ends like filename */
		len = strlen(filename);
		while ((values = fetchline()) != NULL)
		{
			if (CHARcmp(values[0], toCHAR("extension")))
				continue;
			for (i = 1;
			     values[i] &&
				((j = CHARlen(values[i])) > len ||
					CHARcmp(values[i], toCHAR(filename + len - j)));
			     i++)
			{
			}
			if (values[i])
				break;
		}
		ioclose();
	}
	return (BOOLEAN)(values != NULL);
}

/* start the mode, and allocate dminfo */
static DMINFO *init(win)
	WINDOW	win;
{
	char	*pathname, *str;
	CHAR	*cp, **values;
	int	i, j;

	/* if this is the first-ever time a window has been initialized to
	 * this mode, then we have some extra work to do...
	 */
	if (!dmsyntax.mark2col)
	{
		/* Inherit some functions from normal mode. */
		dmsyntax.mark2col = dmnormal.mark2col;
		dmsyntax.move = dmnormal.move;
		dmsyntax.wordmove = dmnormal.wordmove;
		dmsyntax.indent = dmnormal.indent; /* !!! really a good idea? */
		dmsyntax.tagnext = dmnormal.tagnext;

		/* initialize the mode's global options */
		optpreset(o_commentfont, 'i', OPT_REDRAW);
		optpreset(o_stringfont, 'f', OPT_REDRAW);
		optpreset(o_keywordfont, 'b', OPT_REDRAW);
		optpreset(o_functionfont, 'n', OPT_REDRAW);
		optpreset(o_variablefont, 'n', OPT_REDRAW);
		optpreset(o_prepfont, 'e', OPT_REDRAW);
		optpreset(o_otherfont, 'b', OPT_REDRAW);
		str = getenv("INCLUDE");
#ifdef OSINCLUDEPATH
		if (!str)
			str = OSINCLUDEPATH;
#endif
		o_includepath = toCHAR(str);

		/* if no real window, then we're done! */
		if (!win)
			return NULL;
	}

	/* allocate a SINFO structure for this window */
	sinfo = (SINFO *)safealloc(1, sizeof(SINFO));

	/* initialize the wordbits[] array to allow letters and digits */
	for (i = 0; i < QTY(sinfo->wordbits); i++)
	{
		sinfo->wordbits[i] = (isalnum(i) ? (STARTWORD|INWORD) : 0);
	}

	/* locate the "elvis.syn" file */
	pathname = iopath(tochar8(o_elvispath), SYNTAX_FILE, False);
	if (pathname && ioopen(pathname, 'r', False, False, False))
	{
		cp = CHARchr(o_display(win), ' ');
		if (cp)
		{
			/* locate a "language" line containing the name */
			cp++;
			while ((values = fetchline()) != NULL)
			{
				if (CHARcmp(values[0], toCHAR("language")))
					continue;
				for (i = 1; values[i] && CHARcmp(values[i], cp); i++)
				{
				}
				if (values[i])
					break;
			}
		}
		else if (o_filename(markbuffer(win->cursor)))
		{
			/* locate an "extension" line that ends like filename */
			cp = o_filename(markbuffer(win->cursor));
			j = CHARlen(cp);
			while ((values = fetchline()) != NULL)
			{
				if (CHARcmp(values[0], toCHAR("extension")))
					continue;
				for (i = 1; values[i] && (CHARlen(values[i]) > (unsigned)j || CHARcmp(values[i], cp + j - CHARlen(values[i]))); i++)
				{
				}
				if (values[i])
					break;
			}
		}
		else
		{
			values = NULL;
		}

		/* if we found it, then read the attributes */
		if (values)
		{
			while ((values = fetchline()) != NULL
				&& CHARcmp(values[0], toCHAR("language")))
			{
				str = tochar8(values[0]);
				if (!strcmp(str, "keyword"))
				{
					for (i = 1; values[i]; i++)
					{
						addkeyword(values[i], '\0', False, False);
					}
				}
				else if (!strcmp(str, "font") && values[1])
				{
					str = tochar8(values[1]);
					if (!strcmp(str, "normal")
					 || !strcmp(str, "n")
					 || !strcmp(str, "fixed")
					 || !strcmp(str, "f")
					 || !strcmp(str, "bold")
					 || !strcmp(str, "b")
					 || !strcmp(str, "emphasized")
					 || !strcmp(str, "e")
					 || !strcmp(str, "italic")
					 || !strcmp(str, "i")
					 || !strcmp(str, "underlined")
					 || !strcmp(str, "u"))
					{
						for (i = 2; values[i]; i++)
						{
							addkeyword(values[i], *str, False, False);
						}
					}
					/* else invalid font */
				}
				else if (!strcmp(str, "comment"))
				{
					for (i = 1; values[i]; i++)
					{
						if (values[i + 1])
						{
							CHARncpy(sinfo->combegin, values[i], 2);
							CHARncpy(sinfo->comend, values[++i], 2);
						}
						else
						{
							CHARncpy(sinfo->comment, values[1], 2);
						}
					}
				}
				else if (!strcmp(str, "string"))
				{
					if (values[1] && values[2])
					{
						sinfo->strbegin = *values[1];
						sinfo->strend = *values[2];
					}
					else if (values[1] && values[1][1])
					{
						sinfo->strbegin = values[1][0];
						sinfo->strend = values[1][1];
					}
					else if (values[1])
					{
						sinfo->strbegin =
						sinfo->strend = *values[1];
					}
				}
				else if (!strcmp(str, "character"))
				{
					if (values[1] && values[2])
					{
						sinfo->charbegin = *values[1];
						sinfo->charend = *values[2];
					}
					else if (values[1] && values[1][1])
					{
						sinfo->charbegin = values[1][0];
						sinfo->charend = values[1][1];
					}
					else if (values[1])
					{
						sinfo->charbegin =
						sinfo->charend = *values[1];
					}
				}
				else if (!strcmp(str, "regexp"))
				{
					for (i = 1; values[i]; i++)
					{
						for (j = 0; values[i][j]; j++)
						{
							sinfo->wordbits[values[i][j]] |= DELIMREGEXP;
						}
					}
				}
				else if (!strcmp(str, "useregexp"))
				{
					for (i = 1; values[i]; i++)
					{
						cp = iskeyword(values[i]);
						if (cp)
						{
							cp[-2] |= USEREGEXP;
						}
						else if (isalpha(values[i][0]))
						{
							addkeyword(values[i], '\0', True, False);
						}
						else /* character list */
						{
							for (j = 0; values[i][j]; j++)
							{
								sinfo->wordbits[values[i][j]] |= USEREGEXP;
							}
						}
					}
				}
				else if (!strcmp(str, "useregsub"))
				{
					for (i = 1; values[i]; i++)
					{
						cp = iskeyword(values[i]);
						if (cp)
						{
							cp[-2] |= USEREGEXP | USEREGSUB;
						}
						else if (isalpha(values[i][0]))
						{
							addkeyword(values[i], '\0', True, True);
						}
						else /* character list */
						{
							for (j = 0; values[i][j]; j++)
							{
								sinfo->wordbits[values[i][j]] |= USEREGEXP | USEREGSUB;
							}
						}
					}
				}
				else if (!strcmp(str, "preprocessor"))
				{
					if (values[1])
						sinfo->preprocessor = *values[1];
				}
				else if (!strcmp(str, "prepquote"))
				{
					if (values[1] && values[2])
					{
						sinfo->pqbegin = *values[1];
						sinfo->pqend = *values[2];
					}
					else if (values[1] && values[1][1])
					{
						sinfo->pqbegin = values[1][0];
						sinfo->pqend = values[1][1];
					}
					else if (values[1])
					{
						sinfo->pqbegin =
						sinfo->pqend = *values[1];
					}
				}
				else if (!strcmp(str, "function"))
				{
					if (values[1])
						sinfo->function = *values[1];
				}
				else if (!strcmp(str, "other"))
				{
					for (i = 1; values[i]; i++)
					{
						str = tochar8(values[i]);
						if (!strcmp(str, "allcaps"))
							sinfo->allcaps = (BOOLEAN)!sinfo->allcaps;
						else if (!strcmp(str, "initialcaps"))
							sinfo->initialcaps = (BOOLEAN)!sinfo->initialcaps;
						else if (!strcmp(str, "mixedcaps"))
							sinfo->mixedcaps = (BOOLEAN)!sinfo->mixedcaps;
						else if (!strcmp(str, "final_t"))
							sinfo->finalt = (BOOLEAN)!sinfo->finalt;
						else if (!strcmp(str, "initialpunct"))
							sinfo->initialpunct = (BOOLEAN)!sinfo->initialpunct;
						/* else unknown type */
					}
				}
				else if (!strcmp(str, "startword"))
				{
					for (i = 1; values[i]; i++)
					{
						for (j = 0; values[i][j]; j++)
						{
							sinfo->wordbits[values[i][j]] |= STARTWORD;
						}
					}
				}
				else if (!strcmp(str, "inword"))
				{
					for (i = 1; values[i]; i++)
					{
						for (j = 0; values[i][j]; j++)
						{
							sinfo->wordbits[values[i][j]] |= INWORD;
						}
					}
				}
				else if (!strcmp(str, "ignorecase"))
				{
					if (values[1])
						sinfo->ignorecase = calctrue(values[1]);
					else
						sinfo->ignorecase = True;
				}
				/* else unknown attribute */
			}

			/* close the "elvis.syn" file */
			(void)ioclose();
		}
	}

	/* return the window's SINFO structure */
	return (DMINFO *)sinfo;
}


/* end the mode, and free the modeinfo */
static void term(info)
	DMINFO	*info;	/* window-specific information about mode */
{
	int	hash, i;
	SINFO	*si = (SINFO *)info;

	for (hash = 0; hash < 256; hash++)
	{
		/* skip empty hash lists */
		if (!si->keyword[hash]) continue;

		/* free all words in this hash list, and then free the list */
		for (i = 0; si->keyword[hash][i]; i++)
		{
			safefree(si->keyword[hash][i] - 2);
				/* "- 2" because we sneak in two extra chars
				 * to store attributes of the keyword.
				 */
		}
		safefree(si->keyword[hash]);
	}
	safefree(info);
}


/* Choose a line to appear at the top of the screen, and return its mark.
 * Also, initialize the info for the next line.
 */
static MARK setup(top, cursor, bottom, info)
	MARK	top;	/* where the image drawing began last time */
	long	cursor;	/* cursor's offset into buffer */
	MARK	bottom;	/* where the image drawing ended last time */
	DMINFO	*info;	/* window-specific information about mode */
{
	MARK	newtop;
	BOOLEAN	oddquotes;
	CHAR	*cp;
	CHAR	following;
	BOOLEAN	knowstr, knowcom;

	sinfo = (SINFO *)info;

	/* copy the values of the "font" options into the cfont[] array */
	cfont[COMMENT] = cfont[COMMENT2] = o_commentfont;
	cfont[STRING] = cfont[CHARACTER] = cfont[REGEXP] = cfont[REGSUB] = cfont[PREPQUOTE] = o_stringfont;
	cfont[KEYWORD] = cfont[FIRSTPUNCT] = cfont[SECONDPUNCT] = o_keywordfont;
	cfont[FUNCTION] = o_functionfont;
	cfont[VARIABLE] = o_variablefont;
	cfont[PREP] = cfont[PREPWORD] = o_prepfont;
	cfont[OTHER] = o_otherfont;
	cfont[PUNCT] = 'n';

	/* use the normal mode's setup function to choose the screen top */
	newtop = (*dmnormal.setup)(top, cursor, bottom, info);
	if (!newtop || markoffset(newtop) >= o_bufchars(markbuffer(newtop)))
		return newtop;

	/* The top line could be a continuation of a COMMENT or STRING.
	 * (Other tokens can't span a newline, so we can ignore them.)
	 * Scan backward for clues about comments or strings.
	 *
	 * This isn't perfect.  To do the job perfectly, we'd need to start
	 * at the top of the buffer, and scan *forward* to the top of the
	 * screen, but that could take far too long.
	 */
	following = *scanalloc(&cp, newtop);
	oddquotes = False;
	knowstr = (BOOLEAN)(sinfo->strbegin == '\0' || (cp && *cp == sinfo->strbegin));
	knowcom = (BOOLEAN)!sinfo->combegin[0];
	sinfo->token = PUNCT;
	for (; scanprev(&cp) && (!knowstr || !knowcom); following = *cp)
	{
		if (sinfo->strend && *cp != '\\' && following == sinfo->strend && !knowstr)
		{
			/* a " which isn't preceded by a \ toggles the quote state */
			oddquotes = (BOOLEAN)!oddquotes;
		}
		else if (sinfo->strend && *cp != '\\' && following == '\n')
		{
			/* strings can't span a newline unless preceded by a backslash */
			knowstr = True;
		}
		else if (sinfo->combegin[0]
		      && *cp == sinfo->combegin[0]
		      && (!sinfo->combegin[1] || following == sinfo->combegin[1]))
		{
			/* We'll assume that slash-asterisk always starts a
			 * comment (i.e., that it never occurs inside a string).
			 * However, some C++ programmers like to begin comments
			 * with slash-slash and a bunch of asterisks; we need
			 * to watch out for that.
			 */
			knowstr = knowcom = True;
			if (*cp != sinfo->comment[1] || !scanprev(&cp) || *cp != sinfo->comment[0])
			{
				sinfo->token = COMMENT;
				break;
			}
		}
		else if (sinfo->comend[0]
		      && *cp == sinfo->comend[0]
		      && (!sinfo->comend[1] || following == sinfo->comend[1]))
		{
			/* We'll assume that asterisk-slash always ends a comment.
			 * (I.e., that it never occurs inside a string.)
			 */
			knowstr = knowcom = True;
		}
		else if (sinfo->comment[0]
		      && *cp == sinfo->comment[0]
		      && (!sinfo->comment[1] || following == sinfo->comment[1]))
		{
			/* We'll assume that slash-slash always indicates a single-
			 * line comment.  (I.e., that it never occurs in a string or
			 * slash-asterisk type comment.
			 */
			knowstr = knowcom = True;
			oddquotes = False;
		}
	}
	scanfree(&cp);

	/* If it isn't a comment, then it might be a string... check oddquotes */
	if (sinfo->token == PUNCT && oddquotes)
	{
		*(TOKENTYPE *)info = STRING;
	}

	return newtop;
}

/* generate the image of a line, and return the mark of the next line */
static MARK image(w, line, info, draw)
	WINDOW	w;		/* window where drawing will take place */
	MARK	line;		/* line to be drawn */
	DMINFO	*info;		/* window-specific information amount mode */
	void	(*draw)P_((CHAR *p, long qty, _char_ font, long offset));
				/* function for drawing a single character */
{
	int	col;
	CHAR	*cp;
	CHAR	tmpchar;
	CHAR	regexpdelim;	/* delimiter for current regexp */
	long	offset;
	static MARKBUF tmp;
	CHAR	undec[40];	/* characters of undecided font */
	CHAR	*up;		/* pointer used for scanning chars */
	CHAR	prev, prev2;	/* the preceding two characters */
	BOOLEAN	quote;		/* True after a backslash in STRING or CHARACTER */
	int	upper, lower;	/* counts letters of each case */
	BOOLEAN	indent;
	CHAR	*kp;		/* pointer to a keyword */
	BOOLEAN	expectregexp;	/* allow a regular expression to start next */
	BOOLEAN	expectregsub;	/* allow substitution text to follow regexp */
	BOOLEAN	expectprepq;	/* allow preprocessor quotes */
	int	i;

	/* initially, we'll assume we continue the font of the previous line */
	sinfo = (SINFO *)info;
	quote = False;
	indent = True;
	expectregexp = (BOOLEAN)(sinfo->token == PUNCT);
	expectregsub = expectprepq = False;

	/* this is just to silence a compiler warning */
	regexpdelim = '/';

	/* for each character in the line... */
	for (prev = ' ', col = 0, offset = markoffset(line), scanalloc(&cp, line);
	     cp && *cp != '\n' && (*cp != '\f' || markoffset(w->cursor) < o_bufchars(markbuffer(w->cursor)));
	     prev = *cp, offset++, scannext(&cp))
	{
		/* some characters are handled specially */
		if (*cp == '\t' && !o_list(w))
		{
			/* tab ends any symbol */
			if (sinfo->token == KEYWORD || sinfo->token == FUNCTION
			   || sinfo->token == VARIABLE || sinfo->token == OTHER
			   || sinfo->token == PREPWORD)
			{
				sinfo->token = PUNCT;
			}

			/* display the tab character as a bunch of spaces */
			tmpchar = ' ';
			i = o_tabstop(markbuffer(w->cursor));
			i -= col % i;
			(*draw)(&tmpchar, -i, indent ? 'n' : cfont[sinfo->token], offset);
			col += i;
		}
		else if (*cp < ' ' || *cp == 127)
		{
			/* any control character ends any symbol */
			if (sinfo->token == KEYWORD || sinfo->token == FUNCTION
			   || sinfo->token == VARIABLE || sinfo->token == OTHER
			   || sinfo->token == PREPWORD)
			{
				sinfo->token = PUNCT;
			}

			/* also ends indentation */
			indent = False;

			/* control characters */
			tmpchar = '^';
			(*draw)(&tmpchar, -1, cfont[sinfo->token], offset);
			tmpchar = *cp ^ 0x40;
			(*draw)(&tmpchar, -1, cfont[sinfo->token], offset);
			col += 2;
		}
		else if (sinfo->preprocessor && indent
			&& *cp == sinfo->preprocessor && sinfo->token == PUNCT)
		{
			/* output the '#' in prepfont */
			indent = False;
			sinfo->token = PREP;
			(*draw)(cp, 1, cfont[PREP], offset);
			col++;
		}
		else /* normal printable character */
		{
			/* ending a keyword/function/variable? */
			if (!isinword(sinfo, *cp)
				&& (sinfo->token == FUNCTION
					|| sinfo->token == VARIABLE
					|| sinfo->token == KEYWORD
					|| sinfo->token == OTHER))
			{
				sinfo->token = PUNCT;
			}

			/* starting a keyword/function/variable? */
			if (sinfo->token == PUNCT && isstartword(sinfo, *cp))
			{
				/* this isn't regexp */
				expectregexp = expectregsub = False;

				/* store first letter of possible keyword */
				lower = upper = 0;
				undec[0] = *cp;
				if (islower(*cp))
					lower++;
				else if (isupper(*cp))
					upper++;

				/* collect more letters of possible keyword */
				for (i = 1, prev2 = prev = '\0',
					scanalloc(&up, marktmp(tmp, markbuffer(line), offset + 1));
				     i < QTY(undec) - 1 && up && isinword(sinfo, *up);
				     prev2 = prev, prev = *up, i++, scannext(&up))
				{
					undec[i] = *up;
					if (islower(*up))
						lower++;
					else if (isupper(*up))
						upper++;
				}
				undec[i] = '\0';

				/* did we find a keyword? */
				kp = iskeyword(undec);
				if (kp)
				{
					sinfo->token = KEYWORD;
					cfont[KEYWORD] = wordfont(kp);
					if (!cfont[KEYWORD])
						cfont[KEYWORD] = o_keywordfont;
					expectregexp = (BOOLEAN)wordbeforeregexp(kp);
					expectregsub = (BOOLEAN)wordbeforeregsub(kp);
				}
				else /* must be function, variable, or other */
				{
					/* continue on to the end of the word */
					for (;
					     up && isinword(sinfo, *up);
					     prev2 = prev, prev = *up, scannext(&up))
					{
						if (islower(*up))
							lower++;
						else if (isupper(*up))
							upper++;
					}

					/* skip any following whitespace */
					for (; up && *up == ' '; scannext(&up))
					{
					}

					/* is the word followed by a '(' ? */
					if (up && sinfo->function && *up == sinfo->function)
					{
						sinfo->token = FUNCTION;
					}
					else if (sinfo->finalt && prev2 == '_' && prev == 't')
					{
						sinfo->token = OTHER;
					}
					else if (sinfo->initialpunct && !isalnum(undec[0]))
					{
						sinfo->token = OTHER;
					}
					else if (sinfo->allcaps && upper >= 2 && lower == 0)
					{
						sinfo->token = OTHER;
					}
					else if (sinfo->initialcaps && lower > 0 && isupper(undec[0]))
					{
						sinfo->token = OTHER;
					}
					else if (sinfo->mixedcaps && lower > 0 && upper > 0)
					{
						sinfo->token = OTHER;
					}
					else if (i > 1 || isalnum(*cp))
					{
						sinfo->token = VARIABLE;
					}
					/* else leave it set to PUNCT so we can
					 * recognize two-punctuation keywords.
					 */
				}
				scanfree(&up);
			}
			else if (sinfo->token == PREP && !isspace(*cp))
			{
				sinfo->token = PREPWORD;
			}
			else if (sinfo->token == PREPWORD && !isalnum(*cp))
			{
				sinfo->token = PUNCT;
				expectprepq = True;
			}

			/* start of preprocessor quote? */
			if (sinfo->token == PUNCT && expectprepq && *cp == sinfo->pqbegin)
			{
				sinfo->token = PREPQUOTE;
				expectprepq = False;
			}

			/* start of a two-punctuation keyword? */
			if (sinfo->token == PUNCT && ispunctword(sinfo, *cp))
			{
				/* maybe... check the other punct character */
				undec[0] = *cp;
				undec[1] = '\0';
				kp = iskeyword(undec);
				if (!kp)
				{
					tmp = *scanmark(&cp);
					markaddoffset(&tmp, 1);
					undec[1] = scanchar(&tmp);
					undec[2] = '\0';
					kp = iskeyword(undec);
				}
				if (kp)
				{
					sinfo->token = kp[1] ? FIRSTPUNCT : SECONDPUNCT;
					cfont[FIRSTPUNCT] = cfont[SECONDPUNCT] = wordfont(kp);
					if (!cfont[FIRSTPUNCT])
						cfont[FIRSTPUNCT] = cfont[SECONDPUNCT] = o_keywordfont;
					expectregexp = (BOOLEAN)wordbeforeregexp(kp);
					expectregsub = (BOOLEAN)wordbeforeregsub(kp);
				}
			}

			/* start of a string? */
			if (sinfo->strbegin && sinfo->token == PUNCT && *cp == sinfo->strbegin)
			{
				sinfo->token = STRING;
				expectregexp = expectregsub = False;

				/* make sure the initial quote character
				 * isn't going to be mistaken for the
				 * terminating quote character.
				 */
				quote = True;
			}

			/* start of a character literal? */
			if (sinfo->charbegin && sinfo->token == PUNCT && *cp == sinfo->charbegin)
			{
				sinfo->token = CHARACTER;
				expectregexp = expectregsub = False;

				/* make sure the initial quote character
				 * isn't going to be mistaken for the
				 * terminating quote character.
				 */
				quote = True;
			}

			/* start of a C comment? */
			if (sinfo->token == PUNCT
				&& sinfo->combegin[0]
				&& *cp == sinfo->combegin[0]
				&& (!sinfo->combegin[1]
					|| scanchar(marktmp(tmp, markbuffer(line), offset + 1)) == sinfo->combegin[1]))
			{
				sinfo->token = COMMENT;
				expectregexp = expectregsub = False;
			}

			/* start of a one-line comment? */
			if (sinfo->token == PUNCT
				&& sinfo->comment[0]
				&& *cp == sinfo->comment[0]
				&& (indent || !expectregexp)
				&& (!sinfo->comment[1]
					|| scanchar(marktmp(tmp, markbuffer(line), offset + 1)) == sinfo->comment[1]))
			{
				sinfo->token = COMMENT2;
				expectregexp = expectregsub = False;
			}

			/* start of a regular expression? */
			if (sinfo->token == PUNCT && expectregexp && isregexp(sinfo, *cp))
			{
				sinfo->token = REGEXP;
				expectregexp = False;
				regexpdelim = *cp;

				/* make sure the initial quote character
				 * isn't going to be mistaken for the
				 * terminating quote character.
				 */
				quote = True;
			}

			/* would a regexp be allowed after this char? */
			if (sinfo->token == PUNCT && !isspace(*cp))
			{
				expectregexp = (BOOLEAN)isbeforeregexp(sinfo, *cp);
				expectregsub = (BOOLEAN)isbeforeregsub(sinfo, *cp);
			}

			/* any non-whitespace ends indent and disabled prepquote */
			if (!isspace(*cp))
			{
				indent = expectprepq = False;
			}

			/* draw the character */
			(*draw)(cp, 1, indent ? 'n' : cfont[sinfo->token], offset);
			col++;

			/* end of a string? */
			if (sinfo->token == STRING && *cp == sinfo->strend && !quote)
			{
				sinfo->token = PUNCT;
			}

			/* end of a character? */
			if (sinfo->token == CHARACTER && *cp == sinfo->charend && !quote)
			{
				sinfo->token = PUNCT;
			}

			/* end of a comment? */
			if (sinfo->token == COMMENT
			 && (sinfo->comend[1]
				? (prev == sinfo->comend[0] && *cp == sinfo->comend[1])
				: *cp == sinfo->comend[0]))
			{
				sinfo->token = PUNCT;
			}

			/* end of a regexp or substitution text? */
			if ((sinfo->token == REGEXP || sinfo->token == REGSUB)
				&& *cp == regexpdelim && !quote)
			{
				if (expectregsub)
				{
					sinfo->token = REGSUB;
					quote = True;
					expectregsub = False;
				}
				else
				{
					sinfo->token = PUNCT;
				}
			}

			/* middle/end of a two-punctuation character keyword? */
			if (sinfo->token == FIRSTPUNCT)
			{
				sinfo->token = SECONDPUNCT;
			}
			else if (sinfo->token == SECONDPUNCT)
			{
				sinfo->token = PUNCT;
			}

			/* end of a prepquote? */
			if (sinfo->token == PREPQUOTE && *cp == sinfo->pqend)
			{
				sinfo->token = PUNCT;
			}

			/* in a STRING, CHARACTER, REGEXP, or REGSUB constant,
			 * backslash is used to quote the following character.
			 */
			if ((sinfo->token==STRING || sinfo->token==CHARACTER
			 || sinfo->token == REGEXP || sinfo->token == REGSUB)
				&& *cp == '\\' && !quote)
			{
				quote = True;
			}
			else
			{
				quote = False;
			}
		}
	}

	/* end the line */
	if (o_list(w) && (!cp || *cp == '\n'))
	{
		tmpchar = '$';
		(*draw)(&tmpchar, -1, 'n', -1);
	}
	tmpchar = (cp ? *cp : '\n');
	(*draw)(&tmpchar, 1, 'n', offset);
	if (cp)
	{
		offset++;
	}
	else
	{
		offset = o_bufchars(markbuffer(w->cursor));
	}

	/* Strings can span a newline if the newline is preceded by a
	 * backslash.  Old-style C comments can span a newline.  Everything
	 * else ends here.
	 */
	if ((sinfo->token != STRING || prev != '\\') && sinfo->token != COMMENT)
	{
		sinfo->token = PUNCT;
	}

	/* clean up & return the MARK of the next line */
	scanfree(&cp);
	return marktmp(tmp, markbuffer(w->cursor), offset);
}


/* This function considers the possibility that the cursor may be on a quoted
 * filename.  If so, it returns the name, with the quotes.  Otherwise it calls
 * dmnormal.tagatcursor() for the traditional tags.
 *
 * The return value is a dynamically-allocated string; the calling function
 * is responsible for freeing it when it is no longer required.
 */
static CHAR *tagatcursor(win, cursor)
	WINDOW win;	/* window, used for finding mode-dependent info */
	MARK cursor;	/* where the desired tag name can be found */
{
	CHAR	*ret;	/* return value */
	CHAR	*cp;	/* used for scanning */

	/* initialization */
	sinfo = (SINFO *)win->mi;
	ret = NULL;

	/* search backward for first quote or whitespace */
	for (scanalloc(&cp, cursor);
	     cp && !isspace(*cp) && *cp != sinfo->pqbegin && *cp != sinfo->strbegin;
	     scanprev(&cp))
	{
	}

	/* did we find some kind of quote? */
	if (cp && *cp == sinfo->strbegin)
	{
		/* string - search forward for closing quote, collecting chars
		 * along the way.  Beware of whitespace.
		 */
		do
		{
			buildCHAR(&ret, *cp);
		} while (scannext(&cp) && !isspace(*cp) && *cp != sinfo->strend);
		if (cp && *cp == sinfo->strend)
		{
			buildCHAR(&ret, *cp);
			scanfree(&cp);
			return ret;
		}
	}
	else if (cp && *cp == sinfo->pqbegin)
	{
		/* prepquote - search forward for closing quote, collecting
		 * chars along the way.  Beware of whitespace.
		 */
		do
		{
			buildCHAR(&ret, *cp);
		} while (scannext(&cp) && !isspace(*cp) && *cp != sinfo->pqend);
		if (cp && *cp == sinfo->pqend)
		{
			buildCHAR(&ret, *cp);
			scanfree(&cp);
			return ret;
		}
	}

	/* cleanup */
	scanfree(&cp);
	if (ret)
		safefree(ret);

	/* use dmnormal's tagatcursor() function */
	return (*dmnormal.tagatcursor)(win, cursor);
}


/* This function checks the tagname.  If it is a quoted filename, then it
 * attempts to load the file.  Otherwise it calls dmnormal.tagload() to try
 * and load a normal tag.
 */
static MARK tagload(tagname, from)
	CHAR *tagname;	/* name of tag to move to */
	MARK from;	/* where we're coming from */
{
	char	*name;
	char	*tmp;
	unsigned len;
	DIRPERM	perms;
	BUFFER	buf;
 static	MARKBUF	retb;

	/* is it a quoted filename? */
	len = CHARlen(tagname);
	if (len >= 3 && (*tagname == '"' || (sinfo && sinfo->pqbegin && *tagname == sinfo->pqbegin)))
	{
		/* make an unquoted, 8-bit version of the name */
		name = strdup(tochar8(tagname + 1));
		if ((*tagname == '"' && tagname[len - 1] == '"')
		 || (sinfo && sinfo->pqbegin && *tagname == sinfo->pqbegin && tagname[len - 1] == sinfo->pqend))
		{
			name[len - 2] = '\0';
		}

		/* if plain old quote character, then look for it first in
		 * the current directory.
		 */
		if (*tagname == '"')
		{
			perms = dirperm(name);
			if (perms == DIR_READONLY || perms == DIR_READWRITE)
			{
				buf = bufload(NULL, name, False);
				assert(buf != NULL);
				safefree(name);
				return marktmp(retb, buf, buf->changepos);
			}
		}

		/* search through the include path */
		if (o_includepath)
		{
			tmp = iopath(tochar8(o_includepath), name, False);
			if (tmp)
			{
				buf = bufload(NULL, tmp, False);
				assert(buf != NULL);
				safefree(name);
				return marktmp(retb, buf, buf->changepos);
			}
		}

		/* Failed!  Complain, clean up, and return NULL */
		msg(MSG_ERROR, "[s]header file $1 not found", name);
		safefree(name);
		return NULL;
	}

	/* use dmnormal's tagload() function */
	return (*dmnormal.tagload)(tagname, from);
}


DISPMODE dmsyntax =
{
	"syntax",
	"generic syntax coloring",
	True,	/* can optimize */
	True,	/* can use normal wordwrap */
	0,	/* no window options */
	NULL,
	QTY(globdesc),
	globdesc,
	globval,
	init,
	term,
	NULL,	/* init() sets this to be identical to dmnormal's mark2col() */
	NULL,	/* init() sets this to be identical to dmnormal's move() */
	NULL,	/* init() sets this to be identical to dmnormal's moveword() */
	setup,
	image,
	NULL,	/* doesn't need a header */
	NULL,	/* init() sets this to be identical to dmnormal's indent() */
	tagatcursor,
	tagload,
	NULL	/* init() sets this to be identical to dmnormal's tagnext() */
};
#endif /* DISPLAY_SYNTAX */
