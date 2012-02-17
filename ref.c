/* ref.c */

/* This program locates a tag, and displays its context.  A handy reference. */

#include "elvis.h"

/* This is the default path that is searched for tags */
#if OSK
# define DEFTAGPATH ".:/dd/defs:/dd/defs/sys:/dd/usr/src/lib:../lib:/dd/usr/lib"
#else
# if ANY_UNIX
#  define DEFTAGPATH ".:/usr/include:/usr/include/sys:/usr/src/lib:../lib:/usr/local/lib"
# else
#  if MSDOS || TOS || OS2
#   define DEFTAGPATH ".;C:\\include;C:\\include\\sys;C:\\lib;..\\lib"
#  else
#   if AMIGA
#    define DEFTAGPATH ".;Include:;Include:sys"
#   else /* any other OS */
#    define DEFTAGPATH "."
#   endif
#  endif
# endif
#endif

/* maximum length of a path list */
#define MAXPATH	2048

/* compile some of the osdir functions into this module */
#define JUST_DIRPATH
#include "osdir.c"

/* This data type is used for classifying lines */
typedef enum
{
	LC_BLANK,	/* an empty line */
	LC_COMMENT,	/* a comment, or continuation of a comment */
	LC_PARTIAL,	/* partial definition -- no semicolon, or in brackets */
	LC_COMPLETE	/* anything else */
} LINECLS;


#if USE_PROTOTYPES
static void usage(char *argv0);
static char *elv_getline(FILE *fp);
static void store(char *line, char **list);
static LINECLS classify(char *line, LINECLS prev);
static void lookup(TAG *tag);
static void add_to_path(char *file);
int main(int argc, char **argv);
#endif /* USE_PROTOTYPES */


/* support for elvis' ctype macros */
#ifdef ELVCT_DIGIT
CHAR elvct_class[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	ELVCT_DIGIT,ELVCT_DIGIT,ELVCT_DIGIT,ELVCT_DIGIT,ELVCT_DIGIT,
	ELVCT_DIGIT,ELVCT_DIGIT,ELVCT_DIGIT,ELVCT_DIGIT,ELVCT_DIGIT,
	0,0,0,0,0,0,0,
	ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,
	ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,
	ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,
	ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,
	ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,ELVCT_UPPER,
	ELVCT_UPPER,
	0,0,0,0,0,0,
	ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,
	ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,
	ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,
	ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,
	ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,ELVCT_LOWER,
	ELVCT_LOWER
};
#endif


/* These reflect the command-line options */
static int output_tag_info;
static int output_verbose_info;
static int output_html_browser;
static int output_all_matches;
static int omit_comment_lines;
static int omit_other_lines;
static int search_all_files;
static char tag_path[MAXPATH] = DEFTAGPATH;
static long tag_length;


/* These store lines which preceed the tag definition. */
static char	*comments[20];	/* comment lines before tag */
static int	ncomments;	/* number of comment lines */
static char	*members[20];	/* partial definitions before tag */
static int	nmembers;	/* number of partial definition lines */

static void usage(argv0)
	char *argv0;	/* name of program */
{
	fprintf(stderr, "Usage: %s [options] [restrictions]...\n", argv0);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "   -t             Output tag info, instead of the function header\n");
	fprintf(stderr, "   -v             Output verbose tag info, instead of the function header\n");
	fprintf(stderr, "   -h             Output HTML browser info, instead of the function header\n");
	fprintf(stderr, "   -c             Don't output introductory comments\n");
	fprintf(stderr, "   -d             Don't output other lines of the definition\n");
	fprintf(stderr, "   -a             List all matches (else just the most likely one)\n");
	fprintf(stderr, "   -s             Search all tags files (else stop after first with matches)\n");
	fprintf(stderr, "   -p tagpath     List of directories or tags files to search\n");
	fprintf(stderr, "   -l taglength   Only check the first 'taglength' characters of tag names\n");
	fprintf(stderr, "Restrictions:\n");
	fprintf(stderr, "   tag            A tag to search for, short for tagname:tag\n");
	fprintf(stderr, "   attrib:value   An optional attribute (global tags permitted)\n");
	fprintf(stderr, "   attrib:=value  A mandatory attribute (global tags rejected)\n");
	fprintf(stderr, "   attrib:/value  An optional attribute, but require as substring of tagaddr\n");
	fprintf(stderr, "   attrib:+value  List tags with given attribute first (more likely)\n");
	fprintf(stderr, "   attrib:-value  List tags with given attribute last (less likely)\n");
	fprintf(stderr, "A single attribute can be given multiple acceptable values.  The easiest way\n");
	fprintf(stderr, "to specify them is \"attrib:value,value,value\".  This works for any of the\n");
	fprintf(stderr, "restriction operators.  A global tag is one which has no value for a given\n");
	fprintf(stderr, "attribute; e.g., for the \"class\" attribute, a tag with no class is global.\n");
	exit(0);
}


/* display a fatal error message from safe.c */
#if USE_PROTOTYPES
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


/* some custom versions of elvis text I/O functions */
static FILE *iofp;
#ifdef DEBUG_ALLOC
ELVBOOL _ioopen(file, line, name, rwa, prgsafe, force, eol)
	char	*file;
	int	line;
#else
ELVBOOL ioopen(name, rwa, prgsafe, force, eol)
#endif
	char	*name;	/* name of file to open */
	_char_	rwa;	/* ignored; 'r'=read, 'w'=write, 'a'=append */
	ELVBOOL	prgsafe;/* ignored; safe to use "!prg" as file name? */
	ELVBOOL	force;	/* ignored; okay to overwrite existing files? */
	_char_	eol;	/* ignored; open in binary mode? */
{
	iofp = fopen(name, "r");
	return (ELVBOOL)(iofp != NULL);
}
int ioread(iobuf, len)
	CHAR	*iobuf;	/* Input buffer */
	int	len;	/* maximum number of CHARs to read into iobuf */
{
	return fread(iobuf, sizeof(CHAR), len, iofp);
}
ELVBOOL ioclose()
{
	fclose(iofp);
	return ElvTrue;
}


/* This function reads a single line, and replaces the terminating newline with
 * a '\0' byte.  The string will be in a static buffer.  Returns NULL at EOF.
 */
static char *elv_getline(fp)
	FILE	*fp;
{
	int	ch;
 static char	buf[1024];
 	int	len;

	/* initialize ch just to avoid a bogus compiler warning */
	ch = 0;

	for (len = 0; len < QTY(buf) - 1 && (ch = getc(fp)) != EOF && ch != '\n'; )
	{
		buf[len++] = ch;
	}
	if (ch == EOF)
		return NULL;
	buf[len] = '\0';
	if (len >= 1 && buf[len - 1] == '\r')
		buf[len - 1] = '\0';
	return buf;
}


/* Store a line in a list, or clobber the list. */
static void store(line, list)
	char	*line;	/* the text to store, or NULL to clobber */
	char	**list;	/* either comments[] or members[] */
{
	int	qty;	/* size of the array */
	int	*nptr;	/* pointer to number of items already in the list */
	int	i;	/* in case we need to scroll the list */


	/* get the list specifics */
	if (list == comments)
		qty = QTY(comments), nptr = &ncomments;
	else
		qty = QTY(members), nptr = &nmembers;

	/* if supposed to clobber, then clobber */
	if (!line)
	{
		for (i = 0; i < *nptr; i++)
		{
			safefree(list[i]);
		}
		*nptr = 0;
		return;
	}

	/* if list is full, then scroll it */
	if (*nptr == qty)
	{
		safefree(list[0]);
		for (i = 1; i < *nptr; i++)
		{
			list[i - 1] = list[i];
		}
		(*nptr)--;
	}

	/* add the new line to the list */
	list[(*nptr)++] = safedup(line);
}


/* This function classifies a line */
static LINECLS classify(line, prev)
	char	*line;	/* the line to classify */
	LINECLS prev;	/* classification of previous line */
{
	char	*front;	/* line, after skipping indentation */

	/* find the front of the line */
	for (front = line; *front == ' ' || *front == '\t'; front++)
	{
	}

	/* blank line? */
	if (!*front)
		return LC_BLANK;

	/* is it the start of a comment? */
	if ((front[0] == '/' && (front[1] == '*' || front[1] == '/'))
	 || (front[0] == '(' && front[1] == '*')
	 || (front[0] == '-' && front[1] == '-'))
	{
		return LC_COMMENT;
	}

	/* is it a continuation of a comment? */
	if (prev == LC_COMMENT
	 && !(front[0] == '#' || elvalnum(*front)))
	{
		return LC_COMMENT;
	}

	/* is it a partial declaration? */
	if (*front == '#'
		? front[strlen(front) - 1] == '\\'
		: (strchr(front, ';') == NULL
			|| (prev == LC_PARTIAL && front != line)))
	{
		return LC_PARTIAL;
	}

	/* anything else is considered to be a complete declaration */
	return LC_COMPLETE;
}

/* display the source lines which define a given tag */
static void lookup(tag)
	TAG	*tag;	/* the tag to be displayed */
{
	char	*line;	/* current line from file */
	long	lnum;	/* current line number */
	char	*l, *t;	/* for scanning chars in line and tag->TAGADDR*/
	FILE	*fp;	/* source file */
	long	taglnum;/* line number of number tag address, or 0 */
	char	*tagline;/* text form of regexp tag address */
	LINECLS	lc;	/* line classification */
	int	len;
	int	i;

	/* some initializations just to avoid bogus compiler warnings */
	tagline = NULL;
	len = 0;

	/* open the file, or the "refs" file if the source file is unreadable */
	fp = fopen(tag->TAGFILE, "r");
	if (!fp)
	{
		fp = fopen(dirpath(dirdir(tag->TAGFILE), "refs"), "r");
		if (!fp)
		{
			/* can't open anything -- give error for source file */
			(void)fopen(tag->TAGFILE, "r");
			perror(tag->TAGFILE);
			exit(1);
		}
	}

	/* initially we have no stored lines */
	store(NULL, comments);
	store(NULL, members);

	/* Convert tag address */
	taglnum = atol(tag->TAGADDR);
	if (!taglnum)
	{
		/* NOTE: We alter the regexp string!  This is okay since "ref"
		 * only searches for each tag once.
		 */
		tagline = tag->TAGADDR;
		for (l = tagline, t = tag->TAGADDR + 2; t[2]; )
		{
			if (*t == '\\')
				t++;
			*l++ = *t++;
		}
		len = (int)(l - tagline);
		if (*t == '$')
			len++;
		*l = '\0';

		/* If only supposed to output the source line (no comments or
		 * other definition lines), and the tagaddress contains the
		 * source line, then just output that.
		 */
		if (omit_comment_lines && omit_other_lines && *t == '$')
		{
			puts(tagline);
			return;
		}
	}

	/* for each line... */
	for (lnum = 1, lc = LC_COMPLETE; (line = elv_getline(fp)) != NULL; lnum++)
	{
		/* is this the tag definition? */
		if (taglnum > 0 ? taglnum == lnum : !strncmp(tagline, line, len))
		{
			/* output the tag location */
			if (!omit_comment_lines)
				printf("\"%s\", %s, line %ld:\n", tag->TAGNAME, tag->TAGFILE, lnum);

			/* output any introductory comments */
			if (!omit_comment_lines)
				for (i = 0; i < ncomments; i++)
					puts(comments[i]);

			/* output any partial definition lines */
			if (!omit_other_lines)
				for (i = 0; i < nmembers; i++)
					puts(members[i]);

			/* output this line */
			puts(line);

			/* output any following argument lines, unless the
			 * line ends with a semicolon.
			 */
			if (!omit_other_lines && line[strlen(line) - 1] != ';')
			{
				if (strchr(line, '(') != NULL)
				{
					while ((line = elv_getline(fp)) != NULL
					    && *line
					    && ((*line != '#' && *line != '{')
						|| line[strlen(line) - 1] == '\\'))
					{
						puts(line);
					}
				}
				else if ((lc = classify(line, lc)) == LC_PARTIAL)
				{
					while ((line = elv_getline(fp)) != NULL
					    && (lc = classify(line, lc)) == LC_PARTIAL)
					{
						puts(line);
					}
					if (lc == LC_COMPLETE)
						puts(line);
				}
			}

			/* done! */
			goto Succeed;
		}

		/* classify this line */
		lc = classify(line, lc);

		/* process each line, to adjust the stored lines */
		switch (lc)
		{
		  case LC_COMMENT:
			store(line, comments);
			store(NULL, members);
			break;

		  case LC_PARTIAL:
			/* leave comments unchanged, but... */
			store(line, members);
			break;

		  default:
			store(NULL, comments);
			store(NULL, members);
			break;
		}
	}

/* Fail:*/
	/* complain: not found */
	fprintf(stderr, "%s: not found in %s\n", tag->TAGNAME, tag->TAGFILE);

Succeed:
	/* close the file */
	fclose(fp);
}

/* Add the directory portion of a file name to the path */
static void add_to_path(file)
	char	*file;
{
	int	len;

	len = strlen(tag_path);
	tag_path[len++] = OSPATHDELIM;
	strcpy(tag_path + len, dirdir(file));
}

/* The main function */
int main(argc, argv)
	int	argc;
	char	**argv;
{
	int	i, j;
	char	*dir, *file, *scan;
	TAG	*tag;
	char	kindf[10];

	/* check for some standard arguments */
	if (argc > 1)
	{
		if (!strcmp(argv[1], "-help")	/* old GNU */
		 || !strcmp(argv[1], "--help")	/* new GNU */
		 || !strcmp(argv[1], "/?")	/* DOS */
		 || !strcmp(argv[1], "-?"))	/* common */
		{
			usage(argv[0]);
		}
		if (!strcmp(argv[1], "-version")  /* old GNU */
		 || !strcmp(argv[1], "--version"))/* new GNU */
		{
			printf("ref (elvis) %s\n", VERSION);
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
#ifdef PORTEDBY
			puts(PORTEDBY);
#endif
			exit(0);
		}
	}

	/* check the environment for TAGPATH */
	scan = getenv("TAGPATH");
	if (scan)
		strcpy(tag_path, scan);

	/* parse the options */
	for (i = 1; i < argc && argv[i][0] == '-'; i++)
	{
		for (j = 1; argv[i][j]; j++)
		{
			switch (argv[i][j])
			{
			  case 't':
				output_tag_info = 1;
				break;

			  case 'v':
				output_verbose_info = 1;
				break;

			  case 'h':
			  	output_html_browser = 1;
			  	break;

			  case 'c':
			  	omit_comment_lines = 1;
			  	break;

			  case 'd':
			  	omit_other_lines = 1;
			  	break;

			  case 'a':
				output_all_matches = 1;
				break;

			  case 's':
			  	search_all_files = 1;
			  	break;

			  case 'p':
			  	if (argv[i][j + 1])
			  		strcpy(tag_path, &argv[i][j + 1]);
			  	else if (i + 1 < argc)
					strcpy(tag_path, argv[++i]);
			  	else
			  		usage(argv[0]);
				j = strlen(argv[i]) - 1; /* skip to next argv */
				break;

			  case 'l':
			  	if (argv[i][j + 1])
			  		tag_length = atol(&argv[i][j + 1]);
			  	else if (i + 1 < argc)
			  		tag_length = atol(argv[++i]);
			  	else
			  		usage(argv[0]);
			  	if (tag_length < 0)
			  		usage(argv[0]);
				j = strlen(argv[i]) - 1; /* skip to next argv */
				break;

			  default:
			  	usage(argv[0]);
			}
		}
	}

	/* -h implies -a; nobody would want to browse a single tag */
	if (output_html_browser)
		output_all_matches = 1;
	if (output_html_browser + output_tag_info + output_verbose_info > 1)
	{
		fprintf(stderr, "%s: can't mix -t, -v, and -h\n", argv[0]);
		exit(1);
	}

	/* parse any restrictions */
	tsreset();
	strcpy(kindf, "kind:+f");
	tsparse(kindf);
	j = i;
	for ( ; i < argc; i++)
	{
		/* a little extra work for "file:" -- add its directory name
		 * to the tag path.
		 */
		if (!strncmp(argv[i], "file:", 5))
		{
			if (strchr("+-=/", argv[i][5]))
				add_to_path(&argv[i][6]);
			else
				add_to_path(&argv[i][5]);
		}

		tsparse(argv[i]);
	}

	/* As a last resort, add elvis' data directory path to the tag_path */
	i = strlen(tag_path);
	tag_path[i++] = OSPATHDELIM;
	scan = getenv("ELVISPATH");
	if (!scan)
		scan = OSLIBPATH;
	strcpy(tag_path + i, scan);

	/* for each element of the tag path... */
	for (dir = tag_path; *dir && (search_all_files || !taglist); dir = scan)
	{
		/* find the end of this directory name */
		for (scan = dir; *scan && *scan != OSPATHDELIM; scan++)
		{
		}
		if (*scan)
			*scan++ = '\0';

		/* first check to see if there is a tags file there */
		file = dirpath(*dir ? dir : ".", "tags");
		if (ioopen(file, 'r', ElvFalse, ElvFalse, 't'))
		{
			/* yes, scan the tags file */
			ioclose();
			tsfile(file, tag_length);
		}
		else
		{
			/* no, perhaps this tag element is a file? */
			tsfile(dir, tag_length);
		}
	}

	/* if nothing found, then complain */
	if (!taglist)
	{
		fprintf(stderr, "%s: tag not found\n", argv[0]);
		exit(1);
	}

	/* output HTML header, if appropriate */
	if (output_html_browser)
	{
		printf("<html><head>\n");
		printf("<title>Tag Browser</title>\n");
		printf("</head><body>\n");
		printf("<h1>Tag Browser</h1>\n");
		printf("<table border=2 cellspacing=0>\n");
	}

	/* output the information */
	for (tag = taglist; tag; tag = tag->next)
	{
		/* skip tags whose name starts with "!_".  They're pseudo-tags
		 * added to describe the tags file, not any of the user's
		 * source files.  The user doesn't care about them.
		 */
		if (!strncmp(tag->TAGNAME, "!_", 2))
			continue;

		/* output the tag */
		if (output_tag_info)
		{
			printf("%s\t%s\t%s\n", tag->TAGNAME, tag->TAGFILE, tag->TAGADDR);
		}
		else if (output_verbose_info)
		{
			for (i = 0; i < MAXATTR; i++)
			{
				if (tag->attr[i])
				{
					printf("%10s:%s\n",
						tagattrname[i], tag->attr[i]);
				}
			}
			printf("     match:%ld\n", tag->match);
			if (tag->next && output_all_matches)
				putchar('\n');
		}
		else if (output_html_browser)
		{
			printf("<tr>\n");
			printf("<th><a href=\"%s?", tag->TAGFILE);
			for (scan = tag->TAGADDR; *scan; scan++)
			{
				switch (*scan)
				{
				  case '\t':
				  case '+':
				  case '"':
				  case '%':
					printf("%%%02X", *scan);
					break;

				  case ' ':
					putchar('+');
					break;

				  default:
					putchar(*scan);
				}
			}
			printf("\">%s</a></th>\n", tag->TAGNAME);
			printf("<td>%s</td>\n", tag->TAGFILE);
			printf("<td>");
			if (elvdigit(*tag->TAGADDR))
			{
				printf("<em>line %s</em>", tag->TAGADDR);
			}
			else
			{
				for (scan = tag->TAGADDR + 2; scan[2]; scan++)
				{
					switch (*scan)
					{
					  case '&':  printf("&amp;");	break;
					  case '<':  printf("&lt;");	break;
					  case '>':  printf("&gt");	break;
					  case '"':  printf("&quot;");	break;
					  case '\\': scan++; /* and fall thru */
					  default:   putchar(*scan);
					}
				}
			}
			printf("</td>\n</tr>\n");
		}
		else
		{
			lookup(tag);
			if (tag->next && output_all_matches && !omit_comment_lines)
				puts("-------------------------------------------------------------------------------");
		}

		/* if supposed to stop after first, then stop */
		if (!output_all_matches)
			break;
	}

	/* output the HTML trailer, if appropriate */
	if (output_html_browser)
	{
		printf("</table>\n");
		printf("</body></html>");
	}

	/* done! */
	exit(0);
	return 0;	/* <- to silence a compiler warning */
}
