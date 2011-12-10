/* ls.c */

/* I got tired of the limitations of Microsoft's "dir" command, so I wrote
 * this little subset of the Unix "ls" command.  It lists directories in the
 * "ls -CF" format.
 */

#include "elvis.h"
#define JUST_DIRFIRST
#include "osdir.c"
#ifndef FALSE
# define FALSE	0
# define TRUE	1
#endif

typedef struct dent_s
{
	struct dent_s	*next;
	int		width;
	char		name[1];
} dent_t;

#include "ctypetbl.h"


static int isdir P_((char *name));
static dent_t *collect P_((char *pattern, dent_t *list));
static void output P_((char *dir, dent_t *list));


void msg P_((MSGIMP imp, char *text, ...))
{
}

/* This function returns TRUE if a given name is a directory */
static int isdir(name)
	char	*name;
{
	struct _stat statb;

	if (_stat(name, &statb) >= 0 &&
	    (statb.st_mode & _S_IFMT) != _S_IFREG)
		return TRUE;
	else
		return FALSE;
}


/* This function collects the names which match a wildcard pattern, and sorts
 * them.
 */
static dent_t *collect(pattern, list)
	char	*pattern;
	dent_t	*list;
{
	char	*name;
	dent_t	*new, *scan, *lag;

	/* for each name... */
	for (name = dirfirst(pattern, FALSE); name; name = dirnext())
	{
		/* allocate an entry for it */
		new = (dent_t *)safealloc(1, sizeof(dent_t) + strlen(name) + 1);
		strcpy(new->name, name);
		if (isdir(name))
			strcat(new->name, "\\");
		new->width = strlen(new->name) + 2;

		/* insert it into the list */
		for (scan = list, lag = NULL;
		     scan && strcmp(scan->name, new->name) < 0;
		     lag = scan, scan = scan->next)
		{
		}
		new->next = scan;
		if (lag)
			lag->next = new;
		else
			list = new;
	}

	return list;
}



/* output a list in columns */
static void output(dir, list)
	char	*dir;
	dent_t	*list;
{
	int	items, i, col;
	int	dirwidth;
	int	columns = 10;
	int	totwidth;
	int	percol;
	int	colwidth[10];
	dent_t	*colptr[10];
	dent_t	*scan;

	/* if nothing to list, then do nothing */
	if (!list)
		return;

	/* if directory name, then output it */
	if (dir)
	{
		printf("\n%s:\n", dir);
		dirwidth = strlen(dir) + 1;
	}
	else
		dirwidth = 0;

	/* count the items */
	for (items = 0, scan = list; scan; items++, scan = scan->next)
	{
	}

	/* number of columns can't exceed number of items */
	if (columns > items)
		columns = items;

	/* Format using as many columns as possible */
	columns++;
	do
	{
		columns--;

		/* find the top of each coluumn, and column widths */
		percol = (items + columns - 1) / columns;
		for (col = -1, i = 0, scan = list; scan; i++, scan = scan->next)
		{
			if (i % percol == 0)
			{
				col++;
				colptr[col] = scan;
				colwidth[col] = scan->width;
			}
			else if (colwidth[col] < scan->width)
			{
				colwidth[col] = scan->width;
			}
		}

		/* find the total width */
		for (col = totwidth = 0; col < columns; col++)
		{
			colwidth[col] -= dirwidth;
			totwidth += colwidth[col];
		}

	} while (columns > 0 && totwidth >= 80);

	/* Note: The "items" and "percol" values are correct, but "columns"
	 * may be larger than it should be.  E.g., if 17 items can fit on
	 * 8 columns, then you would need 3 items per column.  But at 3 items
	 * per column you only need 6 columns.  So recompute the number of
	 * columns from "item" and "percol" now.
	 */
	columns = (items + percol - 1) / percol;

	/* output the columns, one row at a time */
	while (percol-- > 0)
	{
		for (col = 0; col < columns; col++)
		{
			if (!colptr[col])
				continue;
			if (col + 1 < columns && colptr[col + 1])
				printf("%-*s", colwidth[col], &colptr[col]->name[dirwidth]);
			else
				printf("%s\n", &colptr[col]->name[dirwidth]);
			colptr[col] = colptr[col]->next;
		}
	}
}



/* free a list */
static void freelist(list)
	dent_t	*list;
{
	dent_t	*doomed;

	while (list)
	{
		doomed = list;
		list = list->next;
		safefree(doomed);
	}
}



void main(argc, argv)
	int	argc;
	char	**argv;
{
	int	i;
	dent_t	*list;

	/* if no args, then do "ls ." */
	if (argc == 1)
	{
		list = collect("*", NULL);
		output(NULL, list);
		freelist(list);
	}

	/* for each wildcard pattern... */
	for (i = 1, list = NULL; i < argc; i++)
	{
		/* add its matches to the list */
		if (!isdir(argv[i]))
		{
			list = collect(argv[i], list);
		}
	}
	if (list)
	{
		output(NULL, list);
		freelist(list);
	}

	/* for each directory... */
	for (i = 1; i < argc; i++)
	{
		/* list the directory */
		list = collect(dirpath(argv[i], "*"), NULL);
		if (list && strcmp(list->name, dirpath(argv[i], "*")))
		{
			output(argv[i], list);
		}
		freelist(list);
	}

	exit(0);
}
