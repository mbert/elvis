/* fmt.c */

/* Usage: fmt [-s] [-w width] [-width] [files]...
 *
 * Fmt rearrages text in order to make each line have roughly the
 * same width.  Indentation and word spacing is preserved.
 *
 * The default width is 72 characters, but you can override that via -width,
 * which is the older BSD way, or -w width, which is more common these days.
 * If you don't want short lines to be joined, -s prevents fmt from its
 * default of doing so.
 *
 * If no files are given on the command line, then fmt reads stdin.
 */

#include "config.h"
#if HAS_STDLIB
# include <stdlib.h>
#endif
#include <stdio.h>
#include "elvis.h"
#if OSEXPANDARGS
# define JUST_DIRFIRST
# include "osdir.c"
#endif

#ifndef TRUE
# define TRUE	1
# define FALSE	0
#endif

#ifndef P_
# define P_(args)	()
#endif

#define iswhite(c)	((c) == ' ' || (c) == '\t')
#define isindent(c)	indchars[(c) & 0xff]

#include "ctypetbl.h"

#if USE_PROTOTYPES
void usage(void);
void putword(int shortlines);
int fetchc(FILE *in);
void fmt(FILE *in);
int main(int argc, char **argv);
#endif

int	width = 72;	/* the desired line width */
int	shortlines = 0;	/* keep short lines, instead of joining them */
int	iscrownln;	/* is current output line first of paragraph? */
int	isblankln;	/* is current output line blank? (nothing but indent) */
int	iswhiteln;	/* is current INPUT line white? (nothing but whitespc)*/
int	crownmargin;	/* beware of paragraphs with a crown margin */
int	indent;		/* width of the indentation */
int	indchars[256];	/* character to allow in indentation */
char	ind[512];	/* indentation text */
char	word[1024];	/* word buffer */
char	ungotten[1024];	/* characters from the crown line*/
int	nungotten;	/* number of chars in ungotten[] */
int	nregotten;	/* number of chars used from ungotten[] */
int	singleungotten;	/* -1, or a character in range 0-255 */

/* This function displays a usage message and quits */
void usage()
{
	fprintf(stderr, "Usage: fmt [-w width|-width] [-s] [-i chars] [files]...\n");
	fprintf(stderr, "    -w width   make lines roughly \"width\" columns wide\n");
	fprintf(stderr, "    -s         split long lines, but don't join short lines\n");
	fprintf(stderr, "    -c         crown margin -- be smarter about quirky first line\n");
	fprintf(stderr, "    -i chars   allow chars in indentation, in addition to space and tab\n");
	fprintf(stderr, "    -C         for C/C++ comments, like \"-c -i'/*'\"\n");
	fprintf(stderr, "    -M         for email messages with > quoting, like \"-i'>'\"\n");
	fprintf(stderr, "Report bugs to kirkenda@cs.pdx.edu");
	exit(2);
}



/* This function outputs a single word.  It takes care of spacing and the
 * newlines within a paragraph.
 */
void putword(shortlines)
	int		shortlines;	/* keep short lines (don't join) */
{
	int		i;		/* index into word[], or whatever */
	int		ww;		/* width of the word */
	int		sw;		/* width of spacing after word */
	static int	psw;		/* space width of previous word */
	static int	tab;		/* the width of text already written */


	/* separate the word and its spacing */
	for (ww = 0; word[ww] && !iswhite(word[ww]); ww++)
	{
	}
	sw = strlen(word) - ww;
	word[ww] = '\0';

	/* if no spacing (that is, the word was at the end of the line) then
	 * assume 1 space unless the last char of the word was punctuation
	 */
	if (sw == 0)
	{
		sw = 1;
		if (word[ww - 1] == '.' || word[ww - 1] == '?' || word[ww - 1] == '!')
			sw = 2;
	}

	/* if this is the first word on the line... */
	if (isblankln)
	{
		/* output the indentation first */
		fputs(ind, stdout);
		tab = indent;
	}
	else /* text has already been written to this output line */
	{
		/* will the word fit on this line? */
		if (psw + ww + tab <= width)
		{
			/* yes - so write the previous word's spacing */
			for (i = 0; i < psw; i++)
			{
				putchar(' ');
			}
			tab += psw;
		}
		else
		{
			putchar('\n');
			fputs(ind, stdout);
			tab = indent;
		}
	}

	/* write the word itself */
	fputs(word, stdout);
	if (shortlines)
	{
		putchar('\n');
		tab = 0;
		psw = 0;
		isblankln = TRUE;
	}
	else
	{
		tab += ww;

		/* remember this word's spacing */
		psw = sw;

		/* this output line isn't blank anymore. */
		isblankln = FALSE;
	}
}


/* This is like getc(), except that fetchc() handles singleungotten and
 * ungotten[nungotten].
 */
int fetchc(in)
	FILE	*in;	/* the input stream */
{
	int	c;

	if (singleungotten != -1)
	{
		c = singleungotten;
		singleungotten = -1;
	}
	else if (nregotten < nungotten)
		c = ungotten[nregotten++];
	else
		c = getc(in);
	return c;
}

/* This function reformats text. */
void fmt(in)
	FILE	*in;		/* the input stream */
{
	int	ch;		/* character from input stream */
	int	prevch;		/* the previous character in the loop */
	int	i;		/* index into ind[] or word[] */
	int	inword;		/* boolean: are we between indent & newline? */


	/* for each character in the stream... */
	for (indent = -1, isblankln = TRUE, inword = FALSE, i = 0, prevch = '\n';
	     (ch = fetchc(in)) != EOF;
	     prevch = ch)
	{
		/* is this the end of a line? */
		if (ch == '\n')
		{
			/* if end of last word in the input line */
			if (inword)
			{
				/* if it really is a word */
				if (i > 0)
				{
					/* output it */
					word[i] = '\0';
					putword(shortlines);
				}
			}
			else /* blank line in input */
			{
				/* finish the previous paragraph */
				if (!isblankln)
				{
					putchar('\n');
					isblankln = TRUE;
				}

				/* output an indentation-only line */
				ind[i] = '\0';
				if (iswhiteln)
					putchar('\n');
				else
					puts(ind);

				iscrownln = 1;
			}

			/* continue with next input line... */
			iswhiteln = 1;
			indent = -1;
			i = 0;
			inword = FALSE;
			continue;
		}

		/* if we're expecting indentation now... */
		if (indent < 0)
		{
			/* if this is part of the indentation... */
			if (isindent(ch))
			{
				/* remember it */
				ind[i++] = ch;
				if (!iswhite(ch))
					iswhiteln = 0;
			}
			else /* end of indentation */
			{
				/* mark the end of the indentation string */
				ind[i] = '\0';

				/* calculate the width of the indentation */
				for (i = indent = 0; ind[i]; i++)
				{
					if (ind[i] == '\t')
						indent = (indent | 7) + 1;
					else
						indent++;
				}

				/* reset the word index */
				i = 0;

				/* reprocess that last character */
				singleungotten = ch & 0xff;
			}

			/* continue in the for-loop */
			continue;
		}

		/* if we get here, we're either in a word or in the space
		 * after a word.
		 */
		inword = TRUE;

		/* is this the start of a new word? */
		if (!iswhite(ch) && iswhite(prevch))
		{
			/* yes!  output the previous word */
			word[i] = '\0';
			putword(0);

			/* if this was the first word on the crown line, then
			 * scan forward for the crown of the next line.  Save
			 * the scanned text so we can format it in the usual
			 * way after we know what the indentation looks like.
			 */
			if (crownmargin && iscrownln)
			{
				nungotten = nregotten = i = 0;
				if (singleungotten == -1)
					singleungotten = ch;
				for (ch = singleungotten, singleungotten = -1;
				     ch != EOF && (iscrownln || isindent(ch));
				     ch = getc(in), nungotten++)
				{
					ungotten[nungotten] = ch;
					if (ch == '\n')
					{
						iscrownln = 0;
					}
					else if (!iscrownln)
						ind[i++] = ch;
				}
				ind[i] = '\0';
				if (ch != EOF)
					ungotten[nungotten++] = ch;

				/* calculate the width of the indentation */
				for (i = indent = 0; ind[i]; i++)
				{
					if (ind[i] == '\t')
						indent = (indent | 7) + 1;
					else
						indent++;
				}

				/* remember that we've already scanned ahead */
				iscrownln = 0;

				/* go back to the first char of ungotten text */
				ch = fetchc(in);
			}
			
			/* reset `i' to the start of the word[] buffer */
			i = 0;
		}
		word[i++] = ch;
	}

	/* if necessary, write a final newline */
	if (!isblankln)
	{
		putchar('\n');
		isblankln = TRUE;
	}
}





int main(argc, argv)
	int	argc;
	char	**argv;
{
	FILE	*in;	/* an input stream */
	int	error;	/* if non-zero, then an error occurred */
	int	i;
	int	arg;
	char	*name;

	/* detect special GNU flags */
	if (argc >= 2)
	{
		if (!strcmp(argv[1], "-version")
		 || !strcmp(argv[1], "--version"))
		{
			printf("fmt (elvis) %s\n", VERSION);
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
		else if (!strcmp(argv[1], "/?"))
		{
			usage();
		}
	}

	/* initialize indchars[] table, etc. */
	indchars[' '] = indchars['\t'] = 1;
	singleungotten = -1;

	/* parse the options */
	for (arg = 1; arg < argc && argv[arg][0] == '-'; arg++)
	{
		for (i = 1; argv[arg][i]; i++)
		{
			switch (argv[arg][i])
			{
			  case 'w':
				/* -w width */
				width = -1;
				if (argv[arg][i + 1])
					width = atoi(argv[arg] + i + 1);
				else if (arg + 1 < argc)
					width = atoi(argv[++arg]);
				if (width <= 0)
					usage();
				i = strlen(argv[arg]) - 1;
				break;

			  case 's':
				/* -s */
				shortlines = 1;
				break;

			  case 'i':
				/* -i chars */
				name = NULL;
				if (argv[arg][i + 1])
					name = argv[arg] + i + 1;
				else if (arg + 1 < argc)
					name = argv[++arg];
				if (!name)
					usage();
				while (*name)
					indchars[*name++ & 0xff] = 1;
				i = strlen(argv[arg]) - 1;
				break;
				
			  case 'c':
				/* -c */
				crownmargin = 1;
				break;

			  case 'C':
				/* -C, like "-c -i'/ *'" */
				crownmargin = 1;
				indchars['/'] = indchars['*'] = 1;
				break;

			  case 'M':
				/* -M, like "-i'>'" */
				crownmargin = 1;
				indchars['>'] = 1;
				break;

			  default:
				/* -width */
				width = atoi(argv[arg] + i + 1);
				if (width <= 0)
					usage();
				i = strlen(argv[arg] - 1);
			}
		}
	}

	/* for now, assume there are no errors */
	error = 0;

	/* if no filenames given, then process stdin */
	if (arg == argc)
	{
		fmt(stdin);
	}
	else /* one or more filenames given */
	{
		for (; arg < argc; arg++)
		{
#if OSEXPANDARGS
			for (name = dirfirst(argv[arg], ElvFalse); name; name = dirnext())
			{
				in = fopen(name, "r");
				if (!in)
				{
					perror(name);
					error = 3;
				}
				else
				{
					fmt(in);
					fclose(in);
				}
			}
#else
			in = fopen(argv[arg], "r");
			if (!in)
			{
				perror(argv[arg]);
				error = 3;
			}
			else
			{
				fmt(in);
				fclose(in);
			}
#endif
		}
	}

	/* exit, possibly indicating an error */
	return error;
}
