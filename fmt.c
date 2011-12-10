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

#if USE_PROTOTYPES
void usage(void);
void putword(int shortlines);
void fmt(FILE *in);
int main(int argc, char **argv);
#endif

int	width = 72;	/* the desired line width */
int	shortlines = 0;	/* keep short lines, instead of joining them */
int	isblankln;	/* is the current output line blank? */
int	indent;		/* width of the indentation */
char	ind[512];	/* indentation text */
char	word[1024];	/* word buffer */

/* This function displays a usage message and quits */
void usage P_((void))
{
	fprintf(stderr, "Usage: fmt [-w width|-width] [-s] [files]...\n");
	fprintf(stderr, "    -w width   make lines roughly \"width\" columns wide\n");
	fprintf(stderr, "    -s         split long lines, but don't join short lines\n");
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
			/* no, so write a newline and the indentation */
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
	     (ch = getc(in)) != EOF;
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

				/* output a blank line */
				putchar('\n');
			}

			/* continue with next input line... */
			indent = -1;
			i = 0;
			inword = FALSE;
			continue;
		}

		/* if we're expecting indentation now... */
		if (indent < 0)
		{
			/* if this is part of the indentation... */
			if (iswhite(ch))
			{
				/* remember it */
				ind[i++] = ch;
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
				ungetc(ch, in);
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
#if OSEXPANDARGS
	char	*name;
#endif

	/* detect special GNU flags */
	if (argc >= 2)
	{
		if (!strcmp(argv[1], "-v")
		 || !strcmp(argv[1], "-version")
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
			exit(0);
		}
	}

        while (argc > 1 && argv[1][0] == '-')
        {
        	switch (argv[1][1])
        	{
        		case 'w':
        		{
                                /* -w width */
        			width = (argc > 2 ? atoi(argv[2]) : -1);
                                if (width <= 0)
                                {
                                	usage();
                                }
                                argc-=2;
                                argv+=2;
                                break;
        		}
        		case 's':
        		{
                                /* -s */
        			shortlines = 1;
        			--argc;
        			++argv;
        			break;
        		}
        		default:
                        {
				/* -width */
				width = atoi(argv[1] + 1);
				if (width <= 0)
				{
					usage();
				}
				argc--;
				argv++;
			}
		}
	}

	/* for now, assume there are no errors */
	error = 0;

	/* if no filenames given, then process stdin */
	if (argc == 1)
	{
		fmt(stdin);
	}
	else /* one or more filenames given */
	{
		for (i = 1; i < argc; i++)
		{
#if OSEXPANDARGS
			for (name = dirfirst(argv[i], False); name; name = dirnext())
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
			in = fopen(argv[i], "r");
			if (!in)
			{
				perror(argv[i]);
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
