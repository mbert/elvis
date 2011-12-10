/* ctags.c */

char id_ctags[] = "$Id: ctags.c,v 2.23 1999/02/26 21:31:41 steve Exp $";

/* This is a reimplementation of the ctags(1) program.  It supports ANSI C,
 * and has heaps o' flags.  It is meant to be distributed with elvis.
 *
 * 30.04.1995	Michael Beck & Dirk Verworner (beck@informatik.hu-berlin.de)
 *		added support for inline definitions and
 *		function pointers in parameter declaration 
 */

#include "elvis.h"
#if OSEXPANDARGS
# define JUST_DIRFIRST
# include "osdir.c"
#endif

#ifndef FALSE
# define FALSE	0
# define TRUE	1
#endif
#ifndef TAGS
# define TAGS	"tags"
#endif
#ifndef REFS
# define REFS	"refs"
#endif
#ifndef BLKSIZE
# define BLKSIZE 512
#endif

/* #define EOF -1 */
#define DELETED	  0
#define BODY	  1
#define ARGS	  2
#define COMMA	  3
#define SEMICOLON 4
#define CLASSSEP  5
#define TYPEDEF   6
#define CLASS	  7
#define STRUCT	  8
#define UNION	  9
#define KSTATIC	  10
#define EXTERN	  11
#define NAME	  12
#define INLINE	  13

extern void	file_open P_((char *));
extern int	file_getc P_((void));
extern void	file_ungetc P_((int));
extern void	file_copyline P_((long, FILE *, char *));
extern void	cpp_open P_((char *));
extern void	cpp_echo P_((int));
extern int	cpp_getc P_((void));
extern void	cpp_ungetc P_((int));
extern int	lex_gettoken P_((void));
extern void	maketag P_((int, char *, long, long, int, char *));
extern void	ctags P_((char *));
extern void	usage P_((void));
extern void	main P_((int, char **));


#if defined (GUI_WIN32)
extern void set_current_file (char *file_name);
extern void set_current_tags (int num_tags);
extern void set_total_tags (int num_tags);

static int  num_tags = 0;
static int  total_tags = 0;
#endif

/* support for elvis' ctype macros */
#include "ctypetbl.h"

/* -------------------------------------------------------------------------- */
/* Some global variables */

/* The following boolean variables are set according to command line flags */
char	*del_word;	/* -Dword  ignore word -- good for parameter macro */
int	backward;	/* -B  regexp patterns search backwards */
int	use_numbers;	/* -N  use line numbers instead of regexp patterns */
int	incl_static;	/* -s  include static tags */
int	incl_extern;	/* -e  include extern tags */
int	incl_types;	/* -t  include typedefs and structs */
int	incl_vars;	/* -v  include variables */
int	incl_inline;	/* -i  include inline's */
int	make_parse;	/* -p  write parsed tokens to stdout */
int	make_xtbl;	/* -x  write cross-reference table to stdout */
int	make_tags = 1;	/*     generate a "tags" file (implied by lack of -x) */
int	make_refs;	/* -r  generate a "refs" file */
int	append_files;	/* -a  append to "tags" [and "refs"] files */
int	add_hints;	/* -h  include extra fields that give elvis hints */
int	add_ln;		/* -l  include line number in hints */

/* The following are used for outputting to the "tags" and "refs" files */
FILE	*tags;		/* used for writing to the "tags" file */
FILE	*refs;		/* used for writing to the "refs" file */

/* The following are either empty or store hints */
char	hint_class[200];/* class name, for tags preceeded by a name and :: */

/* -------------------------------------------------------------------------- */
/* display a fatal error message from safe.c */
#ifdef USE_PROTOTYPES
void msg(MSGIMP type, char *msg, ...)
#else
void msg(type, msg)
	MSGIMP	type;
	char	*msg;
#endif
{
	fprintf(stderr, "%s\n", msg);
	abort();
}

/* -------------------------------------------------------------------------- */
/* These are used for reading a source file.  It keeps track of line numbers */
char	*file_name;	/* name of the current file */
FILE	*file_fp;	/* stream used for reading the file */
long	file_lnum;	/* line number in the current file */
long	file_seek;	/* fseek() offset to the start of current line */
int	file_afternl;	/* boolean: was previous character a newline? */
int	file_prevch;	/* a single character that was ungotten */
int	file_header;	/* boolean: is the current file a header file? */

/* This function opens a file, and resets the line counter.  If it fails, it
 * it will display an error message and leave the file_fp set to NULL.
 */
void file_open(name)
	char	*name;	/* name of file to be opened */
{
	/* if another file was already open, then close it */
	if (file_fp)
	{
		fclose(file_fp);
	}

	/* try to open the file for reading.  The file must be opened in
	 * "binary" mode because otherwise fseek() would misbehave under DOS.
	 */
	file_fp = fopen(name, "rb");
	if (!file_fp)
	{
		perror(name);
	}

	/* reset the name & line number */
	file_name = name;
	file_lnum = 0L;
	file_seek = 0L;
	file_afternl = TRUE;

	/* determine whether this is a header file */
	file_header = FALSE;
	name += strlen(name) - 2;
	if (name >= file_name && name[0] == '.' && (name[1] == 'h' || name[1] == 'H'))
	{
		file_header = TRUE;
	}
}

/* This function reads a single character from the stream.  If the *previous*
 * character was a newline, then it also increments file_lnum and sets
 * file_offset.
 */
int file_getc()
{
	int	ch;

	/* if there is an ungotten character, then return it.  Don't do any
	 * other processing on it, though, because we already did that the
	 * first time it was read.
	 */
	if (file_prevch)
	{
		ch = file_prevch;
		file_prevch = 0;
		return ch;
	}

	/* if previous character was a newline, then we're starting a line */
	if (file_afternl && file_fp)
	{
		file_afternl = FALSE;
		file_seek = ftell(file_fp);
		file_lnum++;
	}

	/* Get a character.  If no file is open, then return EOF */
	ch = (file_fp ? getc(file_fp) : EOF);

	/* if it is a newline, then remember that fact */
	if (ch == '\n')
	{
		file_afternl = TRUE;
	}

	/* return the character */
	return ch;
}

/* This function ungets a character from the current source file */
void file_ungetc(ch)
	int	ch;	/* character to be ungotten */
{
	file_prevch = ch;
}

/* This function copies the current line out some other fp.  It has no effect
 * on the file_getc() function.  During copying, any '\' characters are doubled
 * and a leading '^' or trailing '$' is also quoted.  The '\n' character is not
 * copied.  If the '\n' is preceded by a '\r', then the '\r' isn't copied.
 *
 * This is meant to be used when generating a tag line.
 */
void file_copyline(seek, fp, buf)
	long	seek;	/* where the lines starts in the source file */
	FILE	*fp;	/* the output stream to copy it to if buf==NULL */
	char	*buf;	/* line buffer, or NULL to write to fp */
{
	long	oldseek;/* where the file's pointer was before we messed it up */
	char	ch;	/* a single character from the file */
	char	next;	/* the next character from this file */

	/* go to the start of the line */
	oldseek = ftell(file_fp);
	fseek(file_fp, seek, 0);

	/* write everything up to, but not including, the newline */
	for (ch = getc(file_fp); ch != '\n' && ch != EOF; ch = next)
	{
		/* preread the next character from this file */
		next = getc(file_fp);

		/* if character is '\', or a terminal '$', then quote it */
		if (buf && (ch == '\\'
			   || ch == (backward ? '?' : '/')
			   || (ch == '$' && next == '\n')))
		{
			*buf++ = '\\';
		}

		/* copy the character, unless it is a terminal '\r' */
		if (ch != '\r' || next != '\n')
		{
			if (buf)
				*buf++ = ch;
			else
				putc(ch, fp);
		}
	}

	/* mark the end of the line */
	if (buf)
		*buf = '\0';

	/* seek back to the old position */
	fseek(file_fp, oldseek, 0);
}

/* -------------------------------------------------------------------------- */
/* This section handles preprocessor directives.  It strips out all of the
 * directives, and may emit a tag for #define directives.
 */

int	cpp_afternl;	/* boolean: look for '#' character? */
int	cpp_prevch;	/* an ungotten character, if any */
int	cpp_refsok;	/* boolean: can we echo characters out to "refs"? */
int	cpp_refsnl;	/* boolean: dup next \n in "refs"? */

/* This function opens the file & resets variables */
void cpp_open(name)
	char	*name;	/* name of source file to be opened */
{
	/* use the lower-level file_open function to open the file */
	file_open(name);

	/* reset variables */
	cpp_afternl = TRUE;
	cpp_refsok = TRUE;
}

/* This function copies a character from the source file to the "refs" file */
void cpp_echo(ch)
	int	ch; /* the character to copy */
{
	static	wasnl;

	/* echo non-EOF chars, unless not making "ref", or echo turned off */
	if (ch != EOF && make_refs && cpp_refsok && !file_header)
	{
		/* try to avoid blank lines */
		if (ch == '\n')
		{
			/* hack: double \n at end of declarations, to
			   help `ref' find the right starting point...
			*/
			if (cpp_refsnl)
			{
				putc('\n', refs);
				cpp_refsnl = FALSE;
			}
			if (wasnl)
			{
				return;
			}
			wasnl = TRUE;
		}
		else
		{
			wasnl = FALSE;
		}

		/* add the character */
		putc(ch, refs);
	}
}

/* This function returns the next character which isn't part of a directive */
int cpp_getc()
{
	static
	int	ch;	/* the next input character */
	char	*scan;
	char	name[50];/* name of a macro */
	int	len;	 /* length of macro name */

	/* if we have an ungotten character, then return it */
	if (cpp_prevch)
	{
		ch = cpp_prevch;
		cpp_prevch = 0;
		return ch;
	}

	/* Get a character from the file.  Return it if not special '#' */
	ch = file_getc();
	if (ch == '\n')
	{
		cpp_afternl = TRUE;
		cpp_echo(ch);
		return ch;
	}
	else if (ch != '#' || !cpp_afternl)
	{
		/* normal character.  Any non-whitespace should turn off afternl */
		if (ch != ' ' && ch != '\t')
		{
			cpp_afternl = FALSE;
		}
		cpp_echo(ch);
		return ch;
	}

	/* Yikes!  We found a directive */

	/* see whether this is a #define line */
	scan = " define ";
	while (*scan)
	{
		if (*scan == ' ')
		{
			/* space character matches any whitespace */
			do
			{
				ch = file_getc();
			} while (ch == ' ' || ch == '\t');
			file_ungetc(ch);
		}
		else
		{
			/* other characters should match exactly */
			ch = file_getc();
			if (ch != *scan)
			{
				file_ungetc(ch);
				break;
			}
		}
		scan++;
	}

	/* is this a #define line?  and should we generate a tag for it? */
	if (!*scan)
	{
		/* collect chars of the tag name */
		for (ch = file_getc(), len = 0;
		     isalnum(ch) || ch == '_';
		     ch = file_getc())
		{
			if (len < sizeof(name) - 2)
			{
				name[len++] = ch;
			}
		}
		name[len] = '\0';

		/* maybe output parsing info */
		if (make_parse)
		{
			printf("#define %s\n", name);
		}

		/* output a tag line */
		maketag(file_header ? 0 : KSTATIC, name, file_lnum, file_seek, TRUE, "d");
	}

	/* skip to the end of the directive -- a newline that isn't preceded
	 * by a '\' character.
	 */
	while (ch != EOF && ch != '\n')
	{
		if (ch == '\\')
		{
			ch = file_getc();
		}
		ch = file_getc();
	}

	/* return the newline that we found at the end of the directive */
	cpp_echo(ch);
	return ch;
}

/* This puts a character back into the input queue for the source file */
void cpp_ungetc(ch)
	int	ch;	/* a character to be ungotten */
{
	cpp_prevch = ch;
}


/* -------------------------------------------------------------------------- */
/* This is the lexical analyser.  It gets characters from the preprocessor,
 * and gives tokens to the parser.  Some special codes are...
 *   (deleted)  / *...* / (comments)
 *   (deleted)	/ /...\n  (comments)
 *   (deleted)	(*	(parens used in complex declaration)
 *   (deleted)	[...]	(array subscript, when ... contains no ])
 *   (deleted)	struct	(intro to structure declaration)
 *   (deleted)	:words{	(usually ":public baseclass {" in a class declaration
 *   BODY	{...}	('{' can occur anywhere, '}' only at BOL if ... has '{')
 *   ARGS	(...{	(args of function, not extern or forward)
 *   ARGS	(...);	(args of an extern/forward function declaration)
 *   COMMA	,	(separate declarations that have same scope)
 *   SEMICOLON	;	(separate declarations that have different scope)
 *   SEMICOLON  =...;	(initializer)
 *   CLASSSEP	::	(class separator)
 *   TYPEDEF	typedef	(the "typedef" keyword)
 *   KSTATIC	static	(the "static" keyword)
 *   KSTATIC	private	(the "static" keyword)
 *   KSTATIC	PRIVATE	(the "static" keyword)
 *   NAME	[a-z]+	(really any valid name that isn't reserved word)
 */

char	lex_name[BLKSIZE];	/* the name of a "NAME" token */
long	lex_seek;		/* start of line that contains lex_name */
long	lex_lnum;		/* line number of lex_name occurrence */

int lex_gettoken()
{
	char	name[BLKSIZE];	/* stores a temporary word */
	int	ch;		/* a character from the preprocessor */
	int	next;		/* the next character */
	int	token;		/* the token that we'll return */
	int	par;		/* '()' counter */
	int	hasname;	/* have we seen an arg name lately? */
	int	i;

	/* loop until we get a token that isn't "DELETED" */
	do
	{
		/* get the next character */
		ch = cpp_getc();

		/* process the character */
		switch (ch)
		{
		  case ',':
			token = COMMA;
			break;

		  case ';':
			token = SEMICOLON;
			break;

		  case '/':
			/* get the next character */
			ch = cpp_getc();
			switch (ch)
			{
			  case '*':	/* start of C comment */
				ch = cpp_getc();
				next = cpp_getc();
				while (next != EOF && (ch != '*' || next != '/'))
				{
					ch = next;
					next = cpp_getc();
				}
				break;

			  case '/':	/* start of a C++ comment */
				do
				{
					ch = cpp_getc();
				} while (ch != '\n' && ch != EOF);
				break;

			  default:	/* some other slash */
				cpp_ungetc(ch);
			}
			token = DELETED;
			break;

		  case '(':
			ch = cpp_getc();
			if (ch == '*')
			{
				/* In a declaration such as (*name)(), we
				 * want to delete the (* but show the name.
				 */
				token = DELETED;
			}
			else
			{
				/* After the name of a function declaration or
				 * definition, we want to parse the args as a
				 * single token.  New-style args are easy to
				 * parse, but old-style is not.  Begin by
				 * parsing matching parentheses.
				 */
				for (par = 1; par > 0; ch = cpp_getc())
				{
					switch (ch)
					{
					  case '(':  par++;	break;
					  case ')':  par--;	break;
					  case EOF:  par = 0;	break;
					}
				}

				/* now we may have old-style arguments, or
				 * a semicolon or a '{'.  We want to stop
				 * before the first semicolon which isn't
				 * preceded by a word, or before any '{'
				 */
				for (hasname = FALSE;
				     ch!=EOF && ch!='{' && (ch!=';' || hasname);
				     ch = cpp_getc())
				{
					if (isalpha(ch) || ch == '_')
						hasname = TRUE;
				}
				if (ch != EOF)
				{
					cpp_ungetc(ch);
				}
				token = ARGS;
			}
			break;

		  case '{':/*}*/
			/* don't send the next characters to "refs" */
			cpp_refsok = FALSE;

			/* skip ahead to closing '}', or to embedded '{' */
			do
			{
				ch = cpp_getc();
			} while (ch != '{' && ch != '}' && ch != EOF);

			/* if has embedded '{', then skip to '}' in column 1 */
			if (ch == '{') /*}*/
			{
				ch = cpp_getc();
				next = cpp_getc();
				while (ch != EOF && (ch != '\n' || next != '}'))/*{*/
				{
					ch = next;
					next = cpp_getc();
				}
			}

			/* resume "refs" processing */
			cpp_refsok = TRUE;
			cpp_echo('}');

			token = BODY;
			cpp_refsnl = TRUE;
			break;

		  case '[':
			/* skip to matching ']' */
			do
			{
				ch = cpp_getc();
			} while (ch != ']' && ch != EOF);
			token = DELETED;
			break;

		  case '=':
		  	/* skip to next ';' */
			do
			{
				ch = cpp_getc();

				/* leave array initializers out of "refs" */
				if (ch == '{')
				{
					cpp_refsok = FALSE;
				}
			} while (ch != ';' && ch != EOF);

			/* resume echoing to "refs" */
			if (!cpp_refsok)
			{
				cpp_refsok = TRUE;
				cpp_echo('}');
				cpp_echo(';');
			}
			token = SEMICOLON;
			break;

		  case ':':
		  	/* check for a second ':' */
		  	ch = cpp_getc();
		  	if (ch == ':')
		  	{
		  		token = CLASSSEP;
		  	}
		  	else
		  	{
		  		/* probably ": public baseclass {" in a class
		  		 * declaration.
		  		 */
		  		while (isspace(ch) || isalnum(ch) || ch == '_')
		  		{
		  			ch = cpp_getc();
		  		}
				cpp_ungetc(ch);
				token = DELETED;
		  	}
		  	break;

		  case EOF:
			token = EOF;
			break;

		  default:
			/* is this the start of a name/keyword? */
			if (isalpha(ch) || ch == '_')
			{
				/* collect the whole word */
				name[0] = ch;
				for (i = 1, ch = cpp_getc();
				     i < BLKSIZE - 1 && (isalnum(ch) || ch == '_');
				     i++, ch = cpp_getc())
				{
					name[i] = ch;
				}
				name[i] = '\0';
				cpp_ungetc(ch);

				/* is it a reserved word? */
				if (!strcmp(name, "typedef"))
				{
					token = TYPEDEF;
					lex_seek = -1L;
				}
				else if (!strcmp(name, "class"))
				{
					token = CLASS;
					lex_seek = -1L;
				}
				else if (!strcmp(name, "struct"))
				{
					token = STRUCT;
					lex_seek = -1L;
				}
				else if (!strcmp(name, "union"))
				{
					token = UNION;
					lex_seek = -1L;
				}
				else if (!strcmp(name, "static")
				      || !strcmp(name, "private")
				      || !strcmp(name, "protected")
				      || !strcmp(name, "PRIVATE"))
				{
					token = KSTATIC;
					/* ch already contains ungotten next
					 * char.  If ':' then delete it.
					 */
					if (ch == ':')
						(void)cpp_getc();
					lex_seek = -1L;
				}
				else if (!strcmp(name, "extern")
				      || !strcmp(name, "EXTERN")
				      || !strcmp(name, "FORWARD")
				      || !strcmp(name, "virtual"))
				{
					token = EXTERN;
					lex_seek = -1L;
				}
				else if (!strcmp(name, "inline")
				      || !strcmp(name, "__inline")
				      || !strcmp(name, "__inline__"))
				{
					token = INLINE;
					lex_seek = -1L;
				}
				else if (!strcmp(name, "public")
				      || !strcmp(name, "P_")
				      || !strcmp(name, "__P")
				      || (del_word && !strcmp(name, del_word)))
				{
					token = DELETED;
					lex_seek = -1L;
				}
				else if (!strcmp(name, "operator"))
				{
					token = NAME;
					ch = cpp_getc();
					while (strchr("[]<>=!%^&*+-?/:,|~", ch))
					{
						name[i++] = ch;
						ch = cpp_getc();
					}
					cpp_ungetc(ch);

					name[i] = '\0';
					strcpy(lex_name, name);
					lex_seek = file_seek;
					lex_lnum = file_lnum;
				}
				else
				{
					token = NAME;
					strcpy(lex_name, name);
					lex_seek = file_seek;
					lex_lnum = file_lnum;
				}
			}
			else /* not part of a name/keyword */
			{
				token = DELETED;
			}

		} /* end switch(ch) */

	} while (token == DELETED);

	return token;
}

/* -------------------------------------------------------------------------- */
/* This is the parser.  It locates tag candidates, and then decides whether to
 * generate a tag for them.
 */

/* This function generates a tag for the object in lex_name, whose tag line is
 * located at a given seek offset.
 */
void maketag(scope, name, lnum, seek, number, kind)
	int	scope;	/* 0 if global, or KSTATIC if static */
	char	*name;	/* name of the tag */
	long	lnum;	/* line number of the tag */
	long	seek;	/* the seek offset of the line */
	int	number;	/* 1 to give line number, or 0 to give regexp */
	char	*kind;	/* token type of the tag: "f" for function, etc. */
{
	TAG	tag;	/* structure for storing tag info */
	char	buf[200];
	char	lnbuf[20];


	/* if tag has a scope that we don't care about, ignore it */
	if ((scope == EXTERN && !incl_extern)
	 || (scope == KSTATIC && !incl_static))
	{
		return;
	}

	if (make_tags)
	{
#if defined (GUI_WIN32)
		set_current_tags (++num_tags);
		set_total_tags (++total_tags);
#endif

		/* store the basic attributes */
		memset(&tag, 0, sizeof tag);
		tag.TAGNAME = name;
		tag.TAGFILE = file_name;
		if (number)
		{
			sprintf(buf, "%ld", lnum);
		}
		else
		{
			strcpy(buf, backward ? "?^" : "/^");
			file_copyline(seek, NULL, buf + 2);
			strcat(buf, backward ? "$?" : "$/");
		}
		tag.TAGADDR = buf;

		/* add any relevent hints */
		if (add_hints)
		{
			if (scope == KSTATIC)
				tagattr(&tag, "file", "");
			if (hint_class[0])
				tagattr(&tag, "class", hint_class);
			if (kind)
				tagattr(&tag, "kind", kind);
			if (add_ln)
			{
				sprintf(lnbuf, "%ld", lnum);
				tagattr(&tag, "ln", lnbuf);
			}
		}

		/* store the tag */
		tagadd(tagdup(&tag));
	}

	if (make_xtbl)
	{
		printf("%-15.15s%6ld %-16.16s ", name, lnum, file_name);
		file_copyline(seek, stdout, NULL);
		putchar('\n');
	}
}


/* This function parses a source file, adding any tags that it finds */
void ctags(name)
	char	*name;	/* the name of a source file to be checked */
{
	int	prev;	/* the previous token from the source file */
	int	token;	/* the current token from the source file */
	int	scope;	/* normally 0, but could be a TYPEDEF or KSTATIC token */
	int	gotname;/* boolean: does lex_name contain a tag candidate? */
	long	tagseek;/* start of line that contains lex_name */

#if defined (GUI_WIN32)
    set_current_file (name);
    set_current_tags (0);
    num_tags = 0;
#endif

	/* open the file */
	cpp_open(name);

	/* reset */
	scope = 0;
	gotname = FALSE;
	token = SEMICOLON;
	tagseek = 0L;

	/* parse until the end of the file */
	while (prev = token, (token = lex_gettoken()) != EOF)
	{
		if (make_parse)
		{
			switch (token)
			{
			  case  BODY:	   printf("{}\n");		break;
			  case  ARGS:	   printf("() ");		break;
			  case  COMMA:	   printf(",\n");		break;
			  case  SEMICOLON: printf(";\n");		break;
			  case	CLASSSEP:  printf("::");		break;
			  case  TYPEDEF:   printf("typedef ");		break;
			  case	CLASS:	   printf("class ");		break;
			  case	STRUCT:	   printf("struct ");		break;
			  case	UNION:	   printf("union ");		break;
			  case  KSTATIC:   printf("static ");		break;
			  case  EXTERN:	   printf("extern ");		break;
			  case  NAME:	   printf("\"%s\" ", lex_name);	break;
			  case  INLINE:	   printf("inline ");		break;
			}
		}

		/* the class name is clobbered after most tokens */
		if (prev != NAME && prev != CLASSSEP && prev != ARGS)
		{
			*hint_class = '\0';
		}

		/* scope keyword? */
		if (token == TYPEDEF || token == KSTATIC
		 || token == EXTERN || token == CLASS
		 || token == STRUCT || token == UNION)
		{
			scope = token;
			gotname = FALSE;
			continue;
		}

		/* inline ? */
		if (incl_inline && token == INLINE)
		{
			if (!scope || (scope == EXTERN)) scope = token;
			gotname = FALSE;
			continue;
		}

		/* name of a possible tag candidate? */
		if (token == NAME)
		{
			tagseek = file_seek;
			gotname = TRUE;
			continue;
		}

		/* if NAME CLASSSEP, then NAME is a possible class name */
		if (gotname && token == CLASSSEP)
		{
			strcpy(hint_class, lex_name);
			gotname = False;
			continue;
		}

		/* if NAME BODY, without ARGS, then NAME is a struct tag or a
		 * class name
		 */
		if (gotname && token == BODY && prev != ARGS)
		{
			gotname = FALSE;
			
			/* ignore if in typedef -- better name is coming soon */
			if (scope == TYPEDEF)
			{
				continue;
			}

			/* generate a tag, if -t and maybe -s */
			if (incl_types)
			{
				maketag(file_header ? 0 : KSTATIC,
					lex_name, lex_lnum, tagseek,
					use_numbers,
					scope==CLASS ? "c" :
					scope==STRUCT ? "s" : "u");
			}
		}

		/* If NAME ARGS BODY, then NAME is a function */
		if (gotname && prev == ARGS && token == BODY)
		{
			gotname = FALSE;
			
			/* generate a tag, maybe checking -s */
			maketag(scope, lex_name, lex_lnum, tagseek, use_numbers, "f");
		}

		/* If NAME SEMICOLON or NAME COMMA, then NAME is var/typedef.
		 * Note that NAME ARGS SEMICOLON is an extern function
		 * declaration, even if the word "extern" was left off, so we
		 * need to guard against that possibility.
		 */
		if (gotname && (token == SEMICOLON || token == COMMA))
		{
			gotname = FALSE;

			if (prev == ARGS)
			{
				maketag(EXTERN, lex_name, lex_lnum, tagseek, use_numbers, "x");
			}
			/* generate a tag, if -v/-t and maybe -s */
			else if (scope==TYPEDEF || scope==STRUCT || scope==UNION ? incl_types : incl_vars)
			{
				/* a TYPEDEF outside of a header is KSTATIC */
				if (scope == TYPEDEF && !file_header)
				{
					maketag(KSTATIC, lex_name, lex_lnum, tagseek, use_numbers, "t");
				}
				else /* use whatever scope was declared */
				{
					maketag(scope, lex_name, lex_lnum, tagseek, use_numbers, scope==TYPEDEF || scope==STRUCT || scope==UNION ? "t" : "v");
				}
			}
		}

		/* reset after a semicolon or ARGS BODY pair */
		if (token == SEMICOLON || (prev == ARGS && token == BODY))
		{
			scope = 0;
			gotname = FALSE;
		}
	}

	/* The source file will be automatically closed */
}

/* -------------------------------------------------------------------------- */

void usage()
{
	fprintf(stderr, "usage: ctags [flags] filenames...\n");
	fprintf(stderr, "\t-Dword  Ignore \"word\" -- handy for parameter macro name\n");
	fprintf(stderr, "\t-F      Use /regexp/ (default)\n");
	fprintf(stderr, "\t-B      Use ?regexp? instead of /regexp/\n");
	fprintf(stderr, "\t-N      Use line numbers instead of /regexp/\n");
	fprintf(stderr, "\t-s      Include static tags\n");
	fprintf(stderr, "\t-e      Include extern tags\n");
	fprintf(stderr, "\t-i      Include inline definitions\n");
	fprintf(stderr, "\t-t      Include typedefs\n");
	fprintf(stderr, "\t-v      Include variable declarations\n");
	fprintf(stderr, "\t-h      Add hints to help elvis distinguish between overloaded tags\n");
	fprintf(stderr, "\t-l      Add a \"ln\" line number hint (implies -h)\n");
	fprintf(stderr, "\t-p      Write parse info to stdout (for debugging ctags)\n");
	fprintf(stderr, "\t-x      Write cross-reference table to stdout; skip \"tags\"\n");
	fprintf(stderr, "\t-r      Write a \"refs\" file, in addition to \"tags\"\n");
	fprintf(stderr, "\t-a      Append to \"tags\", instead of overwriting\n");
	fprintf(stderr, "If no flags are given, ctags assumes it should use -l -i -s -t -v\n");
	fprintf(stderr, "Report bugs to kirkenda@cs.pdx.edu\n");
	exit(2);
}



void main(argc, argv)
	int	argc;
	char	**argv;
{
	int	i, j;
#if OSEXPANDARGS
	char	*name;
#endif


#if defined (GUI_WIN32)
    set_current_file ("");
    set_current_tags (0);
    set_total_tags (0);
#endif

	/* detect special GNU flags */
	if (argc >= 2)
	{
		if (!strcmp(argv[1], "-version")
		 || !strcmp(argv[1], "--version"))
		{
			printf("ctags (elvis) %s\n", VERSION);
#ifdef COPY1
			puts(COPY1);
#endif
#ifdef COPY2
			puts(COPY2);
#endif
#ifdef COPY3
			puts(COPY3);
#endif
#ifdef COPY4
			puts(COPY4);
#endif
#ifdef COPY5
			puts(COPY5);
#endif
#ifdef GUI_WIN32
# ifdef PORTEDBY
			puts(PORTEDBY);
# endif
#endif
			exit(0);
		}
		else if (!strcmp(argv[1], "/?"))
		{
			usage();
		}
	}

	/* parse the option flags */
	for (i = 1; i < argc && argv[i][0] == '-'; i++)
	{
		for (j = 1; argv[i][j]; j++)
		{
			switch (argv[i][j])
			{
			  case 'D':
				if (argv[i][j + 1])
					del_word = &argv[i][j + 1];
				else if (i + 1 < argc)
					del_word = argv[++i];
				j = strlen(argv[i]) - 1;/* to exit inner loop */
				break;
			  case 'F':	backward = FALSE;		break;
			  case 'B':	backward = TRUE;		break;
			  case 'N':	use_numbers = TRUE;		break;
			  case 's':	incl_static = TRUE;		break;
			  case 'e':	incl_extern = TRUE;		break;
			  case 'i':	incl_inline = TRUE;		break;
			  case 't':	incl_types = TRUE;		break;
			  case 'v':	incl_vars = TRUE;		break;
			  case 'r':	make_refs = TRUE;		break;
			  case 'p':	make_parse = TRUE;		break;
			  case 'x':	make_xtbl=TRUE,make_tags=FALSE;	break;
			  case 'a':	append_files = TRUE;		break;
			  case 'h':	add_hints = TRUE;		break;
			  case 'l':	add_hints = add_ln = TRUE;	break;
			  default:	usage();
			}
		}
	}

	/* There should always be at least one source file named in args */
	if (i == argc)
	{
		usage();
	}

	/* If no flags given on command line, then use big defaults */
	if (i == 1)
		add_ln = add_hints = incl_static = incl_types =
					incl_vars = incl_inline = TRUE;

	/* open the "tags" and maybe "refs" files */
	if (make_tags)
	{
		tags = fopen(TAGS, append_files ? "a" : "w");
		if (!tags)
		{
			perror(TAGS);
			exit(3);
		}
	}
	if (make_refs)
	{
		refs = fopen(REFS, append_files ? "a" : "w");
		if (!refs)
		{
			perror(REFS);
			exit(4);
		}
	}

	/* parse each source file */
	for (; i < argc; i++)
	{
#if OSEXPANDARGS
		for (name = dirfirst(argv[i], False); name; name = dirnext())
		{
			ctags(name);
		}
#else
		ctags(argv[i]);
#endif
	}

	/* add the extra format args */
	fprintf(tags, "!_TAG_FILE_FORMAT\t%d\t/supported features/\n", add_hints ? 2 : 1);
	fprintf(tags, "!_TAG_FILE_SORTED\t1\t/0=unsorted, 1=sorted/\n");
	fprintf(tags, "!_TAG_PROGRAM_AUTHOR\tSteve Kirkendall\t/kirkenda@cs.pdx.edu/\n");
	fprintf(tags, "!_TAG_PROGRAM_NAME\tElvis Ctags\t//\n");
	fprintf(tags, "!_TAG_PROGRAM_URL\tftp://ftp.cs.pdx.edu/pub/elvis/README.html\t/official site/\n");
	fprintf(tags, "!_TAG_PROGRAM_VERSION\t%s\t//\n", VERSION);

	/* write the tags */
	while (taglist)
	{
		fprintf(tags, "%s\t%s\t%s",
			taglist->TAGNAME, taglist->TAGFILE, taglist->TAGADDR);
		for (i = 3, j = -1; i < MAXATTR; i++)
		{
			if (taglist->attr[i])
			{
				if (j == -1)
					fprintf(tags, ";\"");
				if (strcmp(tagattrname[i], "kind"))
					fprintf(tags, "\t%s:", tagattrname[i]);
				else
					putc('\t', tags);
				for (j = 0; taglist->attr[i][j]; j++)
				{
					switch (taglist->attr[i][j])
					{
					  case '\\':
						putc('\\', tags);
						putc('\\', tags);
						break;

					  case '\n':
						putc('\\', tags);
						putc('n', tags);
						break;

					  case '\r':
						putc('\\', tags);
						putc('r', tags);
						break;

					  case '\t':
						putc('\\', tags);
						putc('t', tags);
						break;

					  default:
						putc(taglist->attr[i][j], tags);
					}
				}
			}
		}
		putc('\n', tags);
		tagdelete(False);
	}

	/* close "tags" and maybe "refs" */
	if (make_tags)
	{
		fclose(tags);
	}
	if (make_refs)
	{
		fclose(refs);
	}

	exit(0);
	/*NOTREACHED*/
}
