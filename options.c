/* options.c */
/* Copyright 1995 by Steve Kirkendall */

char id_options[] = "$Id: options.c,v 2.31 1996/08/09 20:13:45 steve Exp $";

/* This file contains functions which manipulate options.
 *
 * If compiled with -DTRY, it will include a main() function which can be
 * used to test these functions.
 */

#include "elvis.h"

#ifndef OPT_MAXCOLS
# define OPT_MAXCOLS 7
#endif


/* This data type is used to record a collection of options that were added
 * via a call to optinsert().
 */
typedef struct domain_s
{
	struct domain_s	*next;	/* next domain in a linked list */
	char		*name;	/* domain name */
	int		nopts;	/* number of options in this domain */
	OPTDESC		*desc;	/* descriptions */
	OPTVAL		*val;	/* option values */
} OPTDOMAIN;

/* This data type is used to collect the names & values of options which are
 * supposed to be output.
 */
typedef struct optout_s
{
	struct optout_s *next;	/* another option to be output */
	OPTDOMAIN	*dom;	/* domain of option to be output */
	int		idx;	/* index of option to be output */
	int		width;	/* width of name+value */
} OPTOUT;


#if USE_PROTOTYPES
static BOOLEAN optshow(char *name);
static void optoutput(BOOLEAN domain, BOOLEAN all, BOOLEAN set, CHAR *outbuf, size_t outsize);
#endif


/* head of the list of current option domains */
static OPTDOMAIN	*head;

/* Check a number's validity.  If valid & different, then set val and return
 * 1; if valid & same, then return 0; else give error message and return -1.
 */
int optisnumber(desc, val, newval)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
	CHAR	*newval;/* value the option should have (as a string) */
{
	long	min, max, value;

	/* convert value to binary */
	if (!calcnumber(newval))
	{
		msg(MSG_ERROR, "[s]$1 requires a numeric value", desc->longname);
		return -1;
	}
	value = CHAR2long(newval);

	/* compare against range string */
	if (desc->limit)
	{
		sscanf(desc->limit, "%ld:%ld", &min, &max);
		if (value < min || value > max)
		{
			msg(MSG_ERROR, "[sdd]$1 must be between $2 and $3", desc->longname, min, max);
			return -1;
		}
	}

	/* same value as before? */
	if (val->value.number == value)
	{
		return 0;
	}

	/* store the value */
	val->value.number = value;
	return 1;
}


/* Check a strings validity (all are valid) and set the option.  Return 1
 * if different, or 0 if same.
 */
int optisstring(desc, val, newval)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
	CHAR	*newval;/* value the option should have (as a string) */
{
	/* if value is the same, do nothing */
	if (val->value.string && !CHARcmp(val->value.string, newval))
	{
		return 0;
	}

	/* free the old string, if necessary */
	if (val->value.string && (val->flags & OPT_FREE))
	{
		safefree(val->value.string);
	}

	/* store a copy of the new string */
	val->value.string = CHARkdup(newval);
	val->flags |= OPT_FREE;
	return 1;
}

/* Check a string's validity against a space-delimited list of legal values.
 * If valid & different, then set val to the string's first character, and
 * return 1; if valid & same, then return 0; else give error message and
 * return -1.  Note that each acceptable valid string must begin with a
 * unique character for this to work.
 */
int optisoneof(desc, val, newval)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
	CHAR	*newval;/* value the option should have (as a string) */
{
	int	len;
	char	*scan;

	assert(desc->limit != NULL);

	/* compute the length of newval */
	len = CHARlen(newval);
	if (len <= 0)
	{
		goto NoMatch;
	}

	/* compare against each legal value */
	for (scan = desc->limit; scan - 1 != NULL; scan = strchr(scan, ' ') + 1)
	{
		/* does it match this value? */
		if (!CHARncmp(newval, toCHAR(scan), (size_t)len))
		{
			/* yes! Either save it & return 1, or just return 0 */
			if ((char)*newval != val->value.character)
			{
				val->value.character = (char)*newval;
				return 1;
			}
			return 0;
		}
	}

	/* no match.  Give an error message and exit */
NoMatch:
	msg(MSG_ERROR, "[ss]$1 must be one of {$2}", desc->longname, desc->limit);
	return -1;
}

/* convert a "number" value to a string */
CHAR *optnstring(desc, val)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
{
	static char	buf[30];

	/* convert the value to a string */
	sprintf(buf, "%ld", val->value.number);
	return toCHAR(buf);
}

/* convert a "string" value to a string.  For NULL strings, return "". */
CHAR *optsstring(desc, val)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
{
	if (val->value.string)
		return val->value.string;
	else
		return toCHAR("");
}

/* convert a "one of" value to a string */
CHAR *opt1string(desc, val)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
{
	static CHAR	buf[30], *build;
	char		*scan;

	/* locate the current value in the limit string */
	scan = desc->limit;
	assert(scan != NULL);
	while (*scan != val->value.character)
	{
		scan = strchr(scan, ' ');
		assert(scan != NULL);
		scan++;
	}

	/* copy the value to buf, and terminate it with a NUL */
	for (build = buf; *scan && *scan != ' '; )
	{
		*build++ = *scan++;
	}
	*build = '\0';

	return buf;
}

/* Delete the options whose values are stored starting at val. */
void optdelete(val)
	OPTVAL	val[];	/* array of values to delete */
{
	OPTDOMAIN	*scan, *lag;

	assert(head != (OPTDOMAIN *)0);

	/* locate the domain in the list */
	for (scan = head, lag = (OPTDOMAIN *)0;
	     scan->val != val;
	     lag = scan, scan = scan->next)
	{
		assert(scan->next != (OPTDOMAIN *)0);
	}

	/* remove the domain from the list */
	if (lag)
	{
		lag->next = scan->next;
	}
	else
	{
		head = scan->next;
	}

	/* free the domain structure */
	safefree(scan);
}

/* Add options to the list known to :set.  desc is an array of
 * option descriptions, as described below.  val is a parallel
 * array where the option values are stored.  nopts is the number
 * of options being added.
 *
 * Descriptions and values are stored separately to support the
 * situation multiple items such as buffers must have their own
 * values for the same options.
 */
void optinsert(domain, nopts, desc, val)
	char	*domain;	/* name of this set of options */
	int	nopts;		/* number of options being inserted */
	OPTDESC	desc[];		/* descriptions of options */
	OPTVAL	val[];		/* values of options */
{
	OPTDOMAIN *newp;

	/* create a new domain structure */
	newp = (OPTDOMAIN *)safekept(1, sizeof(OPTDOMAIN));
	assert(newp != (OPTDOMAIN *)0);
	newp->name = domain,
	newp->nopts = nopts;
	newp->desc = desc;
	newp->val = val;

	/* insert it at the start of the list, so its values take precedence
	 * over any other options that may have been declared with the same
	 * name.
	 */
	newp->next = head;
	head = newp;
}


/* This function calls safefree() on any values which have the OPT_FREE
 * flag set.  This is handy when you intend to free a struct which contains
 * some option values.
 */
void optfree(nopts, vals)
	int	nopts;	/* number of options */
	OPTVAL	*vals;	/* values of options */
{
	int	i;

	for (i = 0; i < nopts; i++)
	{
		if (vals[i].flags & OPT_FREE)
		{
			safefree(vals[i].value.string);
		}
	}
}


/* This function sets the "show" flag for a given option.  Returns True if
 * successful, or False if the option doesn't exist.
 */
static BOOLEAN optshow(name)
	char	*name;	/* name of an option that should be shown */
{
	OPTDOMAIN *dom;
	BOOLEAN	  ret = False;
	int	  i;

	/* for each domain of options... */
	for (dom = head; dom; dom = dom->next)
	{
		/* for each option in that domain... */
		for (i = 0; i < dom->nopts; i++)
		{
			/* if this is the one we're looking for... */
			if (!strcmp(dom->desc[i].longname, name)
			 || !strcmp(dom->desc[i].shortname, name)
			 || !strcmp(dom->name, name))
			{
				dom->val[i].flags |= OPT_SHOW;
				ret = True;
			}
		}
	}

	/* complain if unknown */
	if (!ret)
	{
		msg(MSG_ERROR, "[s]bad option name $1", name);
	}

	return ret;
}

/* This function collects option names & values, sorts them, puts them into
 * columns, and outputs them.  Parameters are:
 *	domain	- include domain names as part of option name?
 *	all	- output all options?
 *	set	- output all options which have been set?
 *		  (If neither "all" nor "set" then only options which were
 *		   touched by a previous optshow() are output.)
 */
static void optoutput(domain, all, set, outbuf, outsize)
	BOOLEAN	  domain;	/* if True, include domain names */
	BOOLEAN	  all;		/* if True, output all non-hidden options */
	BOOLEAN	  set;		/* if True, output all changed options */
	CHAR	  *outbuf;	/* where to place the values */
	size_t	  outsize;	/* size of outbuf */
{
	OPTOUT	  *out;		/* list of options to be output */
	OPTOUT	  *scan, *lag;	/* used for scanning through "out" list */
	OPTOUT	  *newp;	/* a new OPTOUT to be inserted into list */
	OPTDOMAIN *dom;		/* used for scanning through domains */
	int	  i, j, k;	/* used for scanning through opts in a domain */
	int	  cmp;		/* results of comparison */
	int	  nshown;	/* number of options to show */
	int	  maxwidth;	/* width of widest item */
	int	  ncols, nrows;	/* number of columns, and items per column */
	struct
	{
		OPTOUT	*opt;	/* first option in a column */
		int	width;	/* width of the column */
	}	  colinfo[OPT_MAXCOLS];

	/* start with an empty list */
	out = (OPTOUT *)0;
	nshown = 0;
	maxwidth = 0;
	*outbuf = '\0';

	/* For each domain... */
	for (dom = head; dom; dom = dom->next)
	{
		/* For each option in the domain... */
		for (i = 0; i < dom->nopts; dom->val[i++].flags &= ~OPT_SHOW)
		{
			/* Skip if we aren't supposed to output this option */
			if (all ? ((dom->val[i].flags & OPT_HIDE) != 0 && !set)
				: !(dom->val[i].flags & (set ? OPT_SET : OPT_SHOW)))
			{
				continue;
			}

			/* See where this should be inserted into the output
			 * list.  Beware of duplicates; keep only first of each.
			 */
			for (scan = out, lag = (OPTOUT *)0, cmp = 1;
			     scan && (cmp = strcmp(scan->dom->desc[scan->idx].longname, dom->desc[i].longname)) < 0;
			     lag = scan, scan = scan->next)
			{
			}
			if (cmp == 0)
			{
				continue;
			}

			/* create a new OPTOUT structure for this option */
			newp = (OPTOUT *)safealloc(1, sizeof(OPTOUT));
			newp->dom = dom;
			newp->idx = i;
			newp->width = strlen(dom->desc[i].longname);
			if (domain) /* including domain name? */
			{
				newp->width += strlen(dom->name) + 1;
			}
			if (dom->desc[i].isvalid) /* non-boolean? */
			{
				newp->width += 1;
				if (dom->desc[i].asstring)
					newp->width += CHARlen((*dom->desc[i].asstring)(&dom->desc[i], &dom->val[i]));
			}
			else if (!dom->val[i].value.boolean)
			{
				newp->width += 2;
			}

			/* is this the widest value so far? */
			if (newp->width > maxwidth)
			{
				maxwidth = newp->width;
			}

			/* insert the new OPTOUT into the list */
			if (lag)
			{
				newp->next = lag->next;
				lag->next = newp;
			}
			else
			{
				newp->next = out;
				out = newp;
			}
			nshown++;
		}
	}

	/* if nothing to show, then exit */
	if (nshown == 0)
	{
		return;
	}

	/* try to use as many columns as possible */
	for (ncols = (nshown > OPT_MAXCOLS ? OPT_MAXCOLS : nshown); ; ncols--)
	{
		/* how many options would go in each column? */
		nrows = (nshown + ncols - 1) / ncols;

		/* figure out the width of each column */
		for (scan = out, i = 0; i < ncols; i++)
		{
			colinfo[i].opt = scan;
			colinfo[i].width = 0;
			for (j = 0; j < nrows && scan; j++, scan = scan->next)
			{
				/* if this is the widest so far, widen col */
				if (scan->width > colinfo[i].width)
				{
					colinfo[i].width = scan->width;
				}
			}
			colinfo[i].width += 2;
		}

		/* if the total width is narrow enough, then use it */
		for (j = -2, i = 0; i < ncols; i++)
		{
			j += colinfo[i].width;
		}
		if (ncols == 1 || j < (windefault ? o_columns(windefault) : 80) - 1)
		{
			break;
		}
	}

	/* if the list is too large to fit in the output buffer, then just
	 * return an empty string.
	 */
	if ((size_t)j * (size_t)nrows >= outsize)
	{
		/* free the list */
		for (scan = out; scan; scan = lag)
		{
			lag = scan->next;
			safefree(scan);
		}

		CHARncpy(outbuf, toCHAR("too big!\n"), outsize);
		return;
	}

	/* show 'em */
	for (i = 0; i < nrows; i++)
	{
		for (j = 0; j < ncols && colinfo[j].opt; j++)
		{
			/* include the domain name, if we're supposed to */
			scan = colinfo[j].opt;
			if (domain)
			{
				(void)CHARcat(outbuf, scan->dom->name);
				(void)CHARcat(outbuf, ".");
			}

			/* booleans are special... */
			if (!scan->dom->desc[scan->idx].isvalid)
			{
				if (!scan->dom->val[scan->idx].value.boolean)
				{
					(void)CHARcat(outbuf, "no");
				}
				(void)CHARcat(outbuf, scan->dom->desc[scan->idx].longname);
			}
			else
			{
				(void)CHARcat(outbuf, scan->dom->desc[scan->idx].longname);
				(void)CHARcat(outbuf, toCHAR("="));
				(void)CHARcat(outbuf, (*scan->dom->desc[scan->idx].asstring)(&scan->dom->desc[scan->idx], &scan->dom->val[scan->idx]));
			}

			/* pad to max width, except at end of column */
			if (j + 1 == ncols || !colinfo[j + 1].opt)
			{
				(void)CHARcat(outbuf, "\n");
			}
			else
			{
				for (k = colinfo[j].width - scan->width; k > 0; k--)
				{
					(void)CHARcat(outbuf, " ");
				}
			}

			/* next time, use the next option */
			colinfo[j].opt = colinfo[j].opt->next;
		}
	}

	/* free the list */
	for (scan = out; scan; scan = lag)
	{
		lag = scan->next;
		safefree(scan);
	}
}


/* This function returns the value of an option, as a nul-terminated string.
 * For booleans, it returns "true" or "false".  For invalid option names, it
 * returns a NULL pointer.
 */
CHAR *optgetstr(name)
	CHAR	*name;	/* NUL-terminated name */
{
	OPTDOMAIN *dom;	/* used for scanning through domains */
	int	  i;	/* used for scanning through opts in a domain */

	/* For each domain... */
	for (dom = head; dom; dom = dom->next)
	{
		/* For each option in the domain... */
		for (i = 0; i < dom->nopts; i++)
		{
			/* skip options with the wrong name */
			if (strcmp(dom->desc[i].longname, tochar8(name))
			 && strcmp(dom->desc[i].shortname, tochar8(name)))
			{
				continue;
			}

			/* convert it */
			if (dom->desc[i].isvalid) /* non-boolean? */
			{
				if (dom->desc[i].asstring)
				{
					return (CHAR *)(*dom->desc[i].asstring)(&dom->desc[i], &dom->val[i]);
				}
				return (CHAR *)"";
			}
			else if (dom->val[i].value.boolean)
			{
				return (CHAR *)"true";
			}
			else
			{
				return (CHAR *)"false";
			}
		}
	}

	/* if we get here, then we didn't find the option */
	return (CHAR *)0;
}

/* This function assigns a new value to an option.  For booleans, it expects
 * "true" or "false".  For invalid option names or inappropriate values it
 * outputs an error message and returns False.  If the value is NULL it
 * returns False without issueing an error message, on the assumption that
 * whatever caused the NULL pointer already issued a message.
 */
BOOLEAN optputstr(name, value)
	CHAR	*name;	/* NUL-terminated name */
	CHAR	*value;	/* NUL-terminated value */
{
	OPTDOMAIN *dom;	/* used for scanning through domains */
	int	  i;	/* used for scanning through opts in a domain */
	BOOLEAN	  ret;	/* return code */

	/* For each domain... */
	for (dom = head; dom; dom = dom->next)
	{
		/* For each option in the domain... */
		for (i = 0; i < dom->nopts; i++)
		{
			/* skip options with the wrong name */
			if (strcmp(dom->desc[i].longname, tochar8(name))
			 && strcmp(dom->desc[i].shortname, tochar8(name)))
			{
				continue;
			}

			/* if the option is locked, then fail */
			if (dom->val[i].flags & OPT_LOCK)
			{
				msg(MSG_ERROR, "[S]$1 is locked", name);
				return False;
			}

			/* convert it */
			ret = True;
			if (dom->desc[i].isvalid) /* non-boolean? */
			{
				/* if the value is valid & different and we need to 
				 * call a store function, then call it.
				 */
				if ((*dom->desc[i].isvalid)(&dom->desc[i], &dom->val[i], value) == 1
				 && dom->desc[i].store)
				{
					ret = (BOOLEAN)((*dom->desc[i].store)(&dom->desc[i], &dom->val[i], value) >= 0);
				}
			}
			else
			{
				dom->val[i].value.boolean = calctrue(value);
			}

			/* set the "set" flag */
			dom->val[i].flags |= OPT_SET;

			/* if the "redraw" flag is set, then force redraw */
			if ((dom->val[i].flags & OPT_REDRAW) != 0 && windefault)
			{
				windefault->di->logic = DRAW_CHANGED;
			}

			return ret;
		}
	}

	/* if we get here, then we didn't find the option */
	msg(MSG_ERROR, "[S]bad option name $1", name);
	return False;
}


/* This function parses the arguments to a ":set" command.  Returns True if
 * successful.  For errors, it issues an error message via msg() and returns
 * False.  If any options are to be output, their values will be stored in
 * a null-terminated string in outbuf.
 */
BOOLEAN optset(bang, args, outbuf, outsize)
	BOOLEAN	  bang;		/* if True, any options displayed will include domain */
	CHAR	  *args;	/* arguments of ":set" command */
	CHAR	  *outbuf;	/* buffer for storing output string */
	size_t	  outsize;	/* size of outbuf */
{
	CHAR	  *name;	/* name of option in args */
	CHAR	  *value;	/* value of the variable */
	CHAR	  *scan;	/* used for moving through strings */
	CHAR	  *build;	/* used for copying chars from "scan" */
	CHAR	  *prefix;	/* pointer to "neg" or "no" at front of a boolean */
	BOOLEAN	  quote;	/* boolean: inside '"' quotes? */
	OPTDOMAIN *dom;		/* used for scanning through domains list */
	BOOLEAN	  ret;		/* return code */
	WINDOW	  w;
	BOOLEAN	  b;
        int       i;

	/* be optimistic.  Begin by assuming this will succeed. */
	ret = True;

	/* initialize "prefix" just to avoid a compiler warning */
	prefix = NULL;

	/* if no arguments, list values of any set values */
	if (!*args)
	{
		optoutput(bang, False, True, outbuf, outsize);
		return True;
	}

	/* if "all", list values of all options */
	if (!CHARcmp(args, toCHAR("all")))
	{
		optoutput(bang, True, False, outbuf, outsize);
		return True;
	}
	if (!CHARcmp(args, toCHAR("everything")))
	{
		optoutput(bang, True, True, outbuf, outsize);
		return True;
	}

	/* for each assignment... */
	for (name = args; *name; name = scan)
	{
		/* skip whitespace */
		while (*name == ' ' || *name == '\t')
		{
			name++;
		}

		/* after the name, find the value (if any) */
		for (scan = name; isalnum(*scan); scan++)
		{
		}
		if (*scan == '=')
		{
			*scan++ = '\0';
			value = build = scan;
			for (quote = False; *scan && (quote || !isspace(*scan)); scan++)
			{
				if (*scan == '"')
				{
					quote = (BOOLEAN)!quote;
				}
				else if (*scan == '\\' && scan[1] && !isalnum(scan[1]))
				{
					*build++ = *++scan;
				}
				else
				{
					*build++ = *scan;
				}
			}
			if (*scan)
				scan++;
			*build = '\0';
		}
		else if (*scan == '?')
		{
			/* mark the option for showing */
			*scan++ = '\0';
			ret &= optshow(tochar8(name));
			continue;
		}
		else /* no "=" or "?" */
		{
			if (*scan)
			{
				*scan++ = '\0';
			}
			value = NULL;
			prefix = name;
			if (!CHARcmp(name, toCHAR("novice"))
			 || !CHARcmp(name, toCHAR("nonascii")))
				/* don't check for a "no" prefix */;
			else if (prefix[0] == 'n' && prefix[1] == 'o')
				name += 2;
			else if (prefix[0] == 'n' && prefix[1] == 'e' && prefix[2] == 'g')
				name += 3;
		}

		/* find the option */
		for (dom = head; dom; dom = dom->next)
		{
			/* check each option in this domain */
			for (i = 0; i < dom->nopts; i++)
			{
				if (!CHARcmp(name, toCHAR(dom->desc[i].longname))
				 || !CHARcmp(name, toCHAR(dom->desc[i].shortname)))
				{
					goto BreakBreak;
				}
			}
		}
BreakBreak:

		/* if not found, complain */
		if (!dom)
		{
			msg(MSG_ERROR, "[S]bad option name $1", name);
			ret = False;
			continue;
		}

		/* if non-boolean & we got no value, then assume '?' */
		if (dom->desc[i].isvalid && !value)
		{
			if (prefix == name)
			{
				optshow(tochar8(name));
			}
			else
			{
				msg(MSG_ERROR, "[S]$1 is not a boolean option", name);
				ret = False;
			}
			continue;
		}

		/* if option is locked, then complain */
		if (dom->val[i].flags & OPT_LOCK)
		{
			msg(MSG_ERROR, "[S]$1 is locked", name);
			name = scan;
			ret = False;
			continue;
		}

		/* if boolean & we got a value, then complain */
		if (!dom->desc[i].isvalid && value)
		{
			msg(MSG_ERROR, "[S]$1 is a boolean option", name);
			name = scan;
			ret = False;
			continue;
		}

		/* if boolean, set it */
		if (!dom->desc[i].isvalid)
		{
			/* set the value */
			if (prefix == name)
				b = True;
			else if (prefix[0] == 'n' && prefix[1] == 'o')
				b = False;
			else /* "neg" */
				b = (BOOLEAN)!dom->val[i].value.boolean;

			/* if there's a store function, then call it */
			if (dom->desc[i].store)
				ret &= (BOOLEAN)((*dom->desc[i].store)(&dom->desc[i], &dom->val[i], toCHAR(b ? "true" : "false")) >= 0);
			else
				dom->val[i].value.boolean = b;
		}
		else /* non-boolean with a value */
		{
			/* if the value is valid & different and we need to 
			 * call a store function, then call it.
			 */
			if ((*dom->desc[i].isvalid)(&dom->desc[i], &dom->val[i], value) == 1
			 && dom->desc[i].store)
			{
				ret &= (BOOLEAN)((*dom->desc[i].store)(&dom->desc[i], &dom->val[i], value) >= 0);
			}
		}

		/* set the "set" flag */
		if (!bang)
		{
			dom->val[i].flags |= OPT_SET;
		}

		/* If the "redraw" flag is set, then force redrawing (or at
		 * least regeneration) of all windows.
		 */
		if (dom->val[i].flags & OPT_REDRAW)
		{
			for (w = winofbuf(NULL, NULL); w; w = winofbuf(w, NULL))
			{
				if (w->di->logic == DRAW_NORMAL)
					w->di->logic = DRAW_CHANGED;
			}
		}
	}

	/* show any options which we're supposed to show */
	optoutput(bang, False, False, outbuf, outsize);
	return ret;
}


/* This function returns the full name of an option, given a possibly-
 * abbreviated string.  If the string is not the name of an option, it
 * returns NULL.
 */
CHAR *optname(name)
	CHAR	*name;
{
	OPTDOMAIN *dom;
	int	  i;

	/* for each domain of options... */
	for (dom = head; dom; dom = dom->next)
	{
		/* for each option in that domain... */
		for (i = 0; i < dom->nopts; i++)
		{
			/* if this is the one we're looking for... */
			if (!strcmp(dom->desc[i].longname, tochar8(name))
			 || !strcmp(dom->desc[i].shortname, tochar8(name)))
			{
				/* return the long name */
				return toCHAR(dom->desc[i].longname);
			}
		}
	}

	return NULL;
}


/* This function saves the values of some options.  It only does this for
 * options whose values have been changed, and which are in the "global",
 * "buf", "win", "syntax", or "lp" domains.
 */
void optsave(custom)
	BUFFER	custom;	/* where to stuff the "set" commands */
{
	MARKBUF	m;
	OPTDOMAIN *dom;
	int	  i, j;
	CHAR	  *str;
	char	  *tmp;

	/* for each domain of options... */
	for (dom = head; dom; dom = dom->next)
	{
		/* ignore if not "global", "buf", "win", "lp", or "syntax" */
		if (strcmp(dom->name, "global") && strcmp(dom->name, "buf")
			&& strcmp(dom->name, "win") && strcmp(dom->name, "lp")
			&& strcmp(dom->name, "syntax"))
		{
			continue;
		}

		/* for each option in that domain... */
		for (i = 0; i < dom->nopts; i++)
		{
			/* if its value has been set... */
			if ((dom->val[i].flags & (OPT_SET|OPT_LOCK|OPT_UNSAFE)) == OPT_SET)
			{
				/* then add it to the custom buffer */
				if (dom->desc[i].asstring)
				{
					str = (*dom->desc[i].asstring)(&dom->desc[i], &dom->val[i]);
					tmp = safealloc(7 + strlen(dom->desc[i].longname) + 2 * CHARlen(str), sizeof(char));
					strcpy(tmp, "set ");
					strcat(tmp, dom->desc[i].longname);
					strcat(tmp, "=");
					for (j = strlen(tmp); *str; )
					{
						if (*str == ' ' || *str == '\t' || *str == '|' || *str == '\\')
						{
							tmp[j++] = '\\';
						}
						tmp[j++] = (char)*str++;
					}
					tmp[j++] = '\n';
					tmp[j] = '\0';
				}
				else
				{
					tmp = safealloc(8 + strlen(dom->desc[i].longname), sizeof(char));
					sprintf(tmp, "set %s%s\n",
						dom->val[i].value.boolean ? "" : "no",
						dom->desc[i].longname);
				}
				bufreplace(marktmp(m, custom, o_bufchars(custom)), &m, toCHAR(tmp), (long)strlen(tmp));
				safefree((void *)tmp);
			}
		}
	}
}
