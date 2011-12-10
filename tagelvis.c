/* tagelvis.c */

/* Elvis uses this file to scan a tags file, and built a list of the matching
 * tags, sorted by name and likelyhood that they're the intended tag.
 */

#include "elvis.h"


/* Each call to tetag() sets this option, to indicate whether the cursor
 * position should be saved.
 */
static BOOLEAN	newtag = True;

/* This option can be set via temodified to indicate that the current tag
 * couldn't be loaded in a previous attempt because some other buffer was
 * modified.  The current tag hasn't been rejected; the next tetag() call
 * should return the same tag and leave the history unchanged.
 */
static BOOLEAN	sametag = False;

/* Cause the next tetag() to return the same tag as the previous tetag() call */
void tesametag P_((void))
{
	assert(taglist != NULL);

	sametag = True;
}

/* Locate a tag.  Return the tag if it exists, or NULL if there is none.
 * Also, update the tag history for successful or unsuccessful searches.
 */
TAG *tetag(select)
	CHAR	*select;
{
 static	char	*tfilename;	/* pathname of the previous "tags" file */
	char	*tmp;
	CHAR	*scan;
	CHAR	*args[2];
	int	i;

	/* Decide whether the previous search was successful or not, by
	 * checking whether the current argument consists of only the tagname
	 * of the previous tag.
	 */
	if (taglist && !(select && *select && CHARcmp(select, toCHAR(taglist->TAGNAME))))
	{
		/* if supposed to return the same tag as last time, do that. */
		if (sametag && taglist != NULL)
		{
			sametag = False;
			return taglist;
		}

		/* failed search */
		tsadjust(taglist, '-');

		/* try the next one in the list.  If we hit the end of the
		 * list for this particular tags file, then try the next one
		 * from the path.
		 */
		tagdelete(False);
		if (!taglist && tfilename)
		{
			/* find the next tags file from tagpath */
			for (tmp = iopath(tochar8(o_tags), "tags", True);
			     tmp && strcmp(tmp, tfilename);
			     tmp = iopath(NULL, "tags", True))
			{
			}
			if (tmp)
				tmp = iopath(NULL, "tags", True);

			/* process the following tag files until we get tags */
			while (tmp && !taglist)
			{
				tsfile(tmp, o_taglength);
				tmp = iopath(NULL, "tags", True);
			}

			/* if we found tags, then remember the tags file name */
			if (tmp)
			{
				safefree(tfilename);
				tfilename = safedup(tmp);
			}
		}

		/* return the next matching tag, if any */
		newtag = False;
		goto Finish;
	}
	newtag = True;

	/* if no tag given on command line, then use o_previoustag */
	if (!select || !*select)
	{
		if (!o_previoustag)
		{
			msg(MSG_ERROR, "no previous tag");
			sametag = False;
			return NULL;
		}
		select = o_previoustag;
	}
	else
	{
		/* tag given... or maybe it is a selection expression.
		 * Wipe out the old value of previoustag */
		if (o_previoustag)
			safefree(o_previoustag);
		o_previoustag = NULL;

		/* Determine whether we're given a tag or a more complex
		 * restriction expression.
		 */
		for (scan = select; *scan && *scan != ':' && !isspace(*scan); scan++)
			if (*scan == '\\' && scan[1])
				scan++;

		/* If given a simple tag, then remember it as the value of
		 * o_previoustag.  Strip out any backslashes used as quotes.
		 *
		 * Note: If we find any matching tags, then the value set here
		 * will be overridden by the name of the found tag, so really
		 * the value we set here only matters if there is no matching
		 * tag.
		 */
		if (!*scan)
		{
			for (scan = select; *scan; scan++)
			{
				if (*scan == '\\' && scan[1])
					scan++;
				buildCHAR(&o_previoustag, *scan);
			}
		}
	}

	/* Previous tag search (if any) was apparently successful */
	if (taglist)
		tsadjust(taglist, '+');

	/* Delete all tags from previous search */
	tagdelete(True);

	/* using internal tag search, or external? */
	if (o_tagprgonce || o_tagprg)
	{
		/* external tag search */

		/* Wipe out the set of restrictions */
		tsreset();
		tmp = tochar8(calculate(toCHAR("file:+(filename)"),NULL, True));
		assert(tmp);
		tsparse(tmp);

		/* evaluate the tagprg string with $1 set to the args */
		args[0] = select;
		args[1] = NULL;
		if (o_tagprgonce)
		{
			scan = calculate(o_tagprgonce, args, True);
			if (optflags(o_tagprgonce) & OPT_FREE)
				safefree(o_tagprgonce);
			o_tagprgonce = NULL;
		}
		else
			scan = calculate(o_tagprg, args, True);
		if (!scan)
			goto Finish;

		/* add a "!" to the front of the command string, so tsfile
		 * will know it is a command and not a weird file name.
		 */
		tmp = (char *)safealloc(CHARlen(scan) + 2, sizeof(char));
		tmp[0] = '!';
		for (i = 1; *scan; i++, scan++)
			tmp[i] = *scan;

		/* read tags from the program */
		tsfile(tochar8(tmp), o_taglength);
		safefree(tmp);

		/* wipe out the tag file name */
		if (tfilename)
			safefree(tfilename);
		tfilename = NULL;
	}
	else
	{
		/* internal tag search */

		/* Build a new set of restrictions */
		tsreset();
		tsparse(tochar8(select));
		tmp = tochar8(calculate(toCHAR("file:+(filename)"),NULL, True));
		assert(tmp);
		tsparse(tmp);

		/* locate the first set of tags */
		for (tmp = iopath(tochar8(o_tags), "tags", True);
		     tmp && !taglist;
		     tmp = iopath(NULL, "tags", True))
		{
			tsfile(tmp, o_taglength);
		}

		/* remember the tag file name */
		if (tfilename)
			safefree(tfilename);
		tfilename = (tmp ? safedup(tmp) : NULL);
	}

Finish:
	/* return the first matching tag, if any */
	if (!taglist)
		msg(MSG_ERROR, newtag ? "no matching tags" : "no more matching tags");
	else if (!o_previoustag || CHARcmp(o_previoustag, toCHAR(taglist->TAGNAME)))
	{
		if (o_previoustag)
			safefree(o_previoustag);
		o_previoustag = CHARdup(toCHAR(taglist->TAGNAME));
	}
	sametag = False;
	return taglist;
}


/* Build a browser document for a given set of restrictions */
BUFFER tebrowse(all, select)
	BOOLEAN	all;		/* scan all tags files? (else only first) */
	CHAR	*select;	/* the restrictions, from command line */
{
	BUFFER	buf;		/* the buffer containing the new browser file */
	MARKBUF	from, to;	/* positions in the buffer */
	char	*proto;		/* name of tags file or prototype file */
	CHAR	*item;		/* format of a single item, from prototype */
	CHAR	*address;	/* the tagaddress of a tag, converted to HTML */
	CHAR	*url;		/* the URL of a tag */
	CHAR	*args[5];	/* tagname, tagfile, tagaddress, url, NULL */
	CHAR	*dflt = toCHAR("<ul>\n\n<li><a href=\"$4\">$1</a> $2, $3\n\n</ul>\n");
	CHAR	*cp;
	CHAR	prev, prev2;
	long	qty;
	CHAR	qtystr[20];
	TAG	*qtytag;
	char	*tmp;
	int	i;

	/* forget any old tag info */
	tagdelete(True);
	tsreset();

	/* default args are none */
	if (!select)
		select = toCHAR("");

	if (o_tagprg || o_tagprgonce)
	{
		/* external tag search */

		/* make an HTML copy of the string */
		cp = (CHAR *)safealloc(CHARlen(select) + 8, sizeof(CHAR));
		CHARcpy(cp, toCHAR("Browse "));
		CHARcat(cp, select);

		/* evaluate the tagprg string with $1 set to the args */
		args[0] = select;
		args[1] = NULL;
		select = cp;
		if (o_tagprgonce)
		{
			cp = calculate(o_tagprgonce, args, True);
			if (optflags(o_tagprgonce) & OPT_FREE)
				safefree(o_tagprgonce);
			o_tagprgonce = NULL;
		}
		else
			cp = calculate(o_tagprg, args, True);
		if (!cp)
			return NULL;

		/* add a "!" to the front of the command string, so tsfile
		 * will know it is a command and not a weird file name.
		 */
		tmp = (char *)safealloc(CHARlen(cp) + 2, sizeof(char));
		tmp[0] = '!';
		for (i = 1; *cp; i++, cp++)
			tmp[i] = *cp;

		/* read tags from the program */
		tsfile(tochar8(tmp), o_taglength);
		safefree(tmp);
	}
	else
	{
		/* internal tag search */

		/* parse the restrictions & make an HTML copy of the string */
		cp = (CHAR *)safealloc(CHARlen(select) + 8, sizeof(CHAR));
		CHARcpy(cp, toCHAR("Browse "));
		CHARcat(cp, select);
		tsparse(tochar8(select));
		select = cp;

		/* build the tags list */
		for (proto = iopath(tochar8(o_tags), "tags", True);
		     proto && (!taglist || all);
		     proto = iopath(NULL, "tags", True))
		{
			tsfile(proto, o_taglength);
		}
	}

	/* if no tags, then fail */
	if (!taglist)
	{
		return NULL;
	}

	/* count the tags */
	for (qty = 0L, qtytag = taglist; qtytag; qty++, qtytag = qtytag->next)
	{
	}
	sprintf(tochar8(qtystr), "%ld", qty);

	/* create a buffer to hold the browser document */
	buf = bufalloc(select, 0, False);
	o_bufdisplay(buf) = toCHAR("html");

	/* get the document format */
	proto = iopath(tochar8(o_elvispath), BROWSER_FILE, False);
	if (proto)
		(void)bufload(o_bufname(buf), proto, True);
	o_readonly(buf) = True;
	if (o_bufchars(buf) == 0L)
		bufreplace(marktmp(from, buf, 0L), &from, dflt, CHARlen(dflt));
	if (o_filename(buf))
	{
		if (optflags(o_filename(buf)) & OPT_FREE)
			safefree(o_filename(buf));
		o_filename(buf) = NULL;
	}

	/* Parse the document; i.e., locate the item section, copy it into RAM,
	 * and then delete it from the buffer.  The item section is delimited
	 * by blank lines.
	 */
	to = from;
	for (scanalloc(&cp, marktmp(from, buf, 0L)), prev = prev2 = '\0';
	     cp;
	     prev2 = prev, prev = *cp, scannext(&cp))
	{
		/* watch for multiple-newlines */
		if (prev2 == '\n' && prev == '\n' && *cp != '\n')
		{
			if (markoffset(&from) == 0L)
				from = *scanmark(&cp);
			else
				to = *scanmark(&cp);
		}

		/* watch for $1 or $2 in the header */
		if (markoffset(&from) == 0 && prev2 == '$' && (prev == '1' || prev == '2'))
		{
			to = *scanmark(&cp);
			scanfree(&cp);
			from = to;
			markaddoffset(&from, -2);
			switch (prev)
			{
			  case '1':
				args[0] = select;
				args[1] = NULL;
				cp = calculate(toCHAR("htmlsafe($1)"), args, False);
				break;

			  case '2':
				cp = qtystr;
				break;
			}
			bufreplace(&from, &to, cp, CHARlen(cp));
			markaddoffset(&from, CHARlen(cp));
			scanalloc(&cp, &from);
			marksetoffset(&from, 0);
			to = from;
		}
	}
	scanfree(&cp);
	if (markoffset(&to) == 0L)
	{
		msg(MSG_ERROR, "bad elvis.bdf");
		return NULL;
	}
	markaddoffset(&to, -1);
	item = bufmemory(&from, &to);
	markaddoffset(&from, -1);
	markaddoffset(&to, 1);
	bufreplace(&from, &to, NULL, 0L);

	/* for each tag in the list... */
	for ( ; taglist; tagdelete(False))
	{
		/* Convert the address to a plaintext line */
		if (isdigit(*taglist->TAGADDR))
		{
			/* line number -- make sure it isn't JUST a number! */
			address = NULL;
			buildstr(&address, "(line ");
			buildstr(&address, taglist->TAGADDR);
			buildCHAR(&address, ')');
		}
		else
		{
			/* strip /^ and $/, along with any backslashes */
			for (address = NULL, proto = taglist->TAGADDR + 2;
			     proto[2];
			     proto++)
			{
				if (*proto == '\\' && proto[1])
					proto++;
				buildCHAR(&address, *proto);
			}
			if (!address)
				address = (CHAR *)safealloc(1, sizeof(CHAR));
		}

		/* Generate an URL.  Note that the tagaddress is URL-encoded */
		url = NULL;
		buildstr(&url, taglist->TAGFILE);
		buildCHAR(&url, '?');
		for (cp = toCHAR(taglist->TAGADDR); *cp; cp++)
		{
			switch (*cp)
			{
			  case '\t':	buildstr(&url, "%09");	break;
			  case '+':	buildstr(&url, "%2B");	break;
			  case '"':	buildstr(&url, "%22");	break;
			  case '%':	buildstr(&url, "%25");	break;
			  case '<':	buildstr(&url, "%3C");	break;
			  case '>':	buildstr(&url, "%3E");	break;
			  case ' ':	buildCHAR(&url, '+');	break;
			  default:	buildCHAR(&url, *cp);
			}
		}

		/* evaluate the item line with this tag's values */
		args[0] = toCHAR(taglist->TAGNAME);
		args[1] = toCHAR(taglist->TAGFILE);
		args[2] = address;
		args[3] = url;
		args[4] = NULL;
		cp = calculate(item, args, True);
		if (!cp)
			cp = item; /* error -- but give user a clue */

		/* stuff the tag into the document */
		bufreplace(&from, &from, cp, CHARlen(cp));
		markaddoffset(&from, CHARlen(cp));

		/* free the temporary stuff */
		safefree(address);
		safefree(url);
	}

	/* turn off the "modified" flag. */
	o_modified(buf) = False;
	buf->docursor = 0L;

	/* return the buffer */
	return buf;
}

/* Save the current cursor position on the tag stack.  */
void tepush(win, label)
	WINDOW	win;	/* window where push should occur */
	CHAR	*label;	/* dynamically-allocated name of old position */
{
	int	i;

	if (o_tagstack
	 && newtag
	 && (o_filename(markbuffer(win->cursor))
		|| o_bufchars(markbuffer(win->cursor))))
	{
		/* The oldest tag will be lost.  If it had pointers to any
		 * dynamically allocated memory, then free that memory now.
		 */
		if (win->tagstack[TAGSTK - 1].prevtag)
			safefree(win->tagstack[TAGSTK - 1].prevtag);
		if (win->tagstack[TAGSTK - 1].origin)
			markfree(win->tagstack[TAGSTK - 1].origin);

		/* Shift the tag stack; top is always win->tagstack[0] */
		for (i = TAGSTK - 1; i > 0; i--)
		{
			win->tagstack[i] = win->tagstack[i - 1];
		}

		/* insert data into the top slot */
		win->tagstack[0].origin = markdup(win->cursor);
		win->tagstack[0].display = win->md->name;
		win->tagstack[0].prevtag = label;
	}
	else
	{
		safefree(label);
	}

	/* always leave newtag set to "true" */
	newtag = True;
}


#ifdef FEATURE_SHOWTAG
/* build a list of all top-level tags defined in this buffer */
void tebuilddef(buf)
	BUFFER	buf;
{
   /* for building the new tagdef array */
	TEDEF	*tagdef;	/* the new tagdef array */
	TEDEF	*bigger;	/* used while enlarging tagdef */
	int	allocated;	/* number of items allocated for tagdef */
	int	ntagdefs;	/* number of items in tagdef */
   /* for scanning the tags file... */
	CHAR	tagline[1000];	/* input buffer */
	BOOLEAN	allnext;	/* does tagline[] contain the whole next line?*/
	int	bytes;		/* number of bytes in tagline */
	CHAR	*src, *dst;	/* for manipulating tagline[] */
	TAG	*tag;		/* a tag parsed from tagline[] */
    /* for locating a tag defintion within this buffer */
	EXINFO	xinfb;		/* dummy ex command, for parsing tag address */
	BOOLEAN	wasmagic;	/* stores the normal value of o_magic */
	BOOLEAN wassaveregexp;	/* stores the normal value of o_saveregexp */
	BOOLEAN	wasmsghide;	/* stores the msghide() flag */
	CHAR	*cp;		/* for scanning the line */
	long	offset;		/* offset of the tag within this buffer */
	int	i;

	/* Destroy the old list, if any */
	tefreedef(buf);

	/* if "showtag" isn't set, then do nothing more */
	if (!o_showtag)
		return;

	/* if this buffer contains no file, then do nothing */
	if (!o_filename(buf))
		return;

	/* Scan the "tags" file.  Note that we're using the lower-level
	 * tag reading functions for a couple of reasons: speed of course,
	 * but also because we don't want to clobber any existing tag list.
	 * That's important because tebuilddef() will be called whenever
	 * :tag causes a file to be loaded, and we want to keep the remainder
	 * of that tag list in case we just loaded the wrong one.
	 */

	/* If there is no tags file, then do nothing.  This check is
	 * necessary because the ioopen() function displays an error
	 * message when the file it's trying to read doesn't exist.
	 */
	if (dirperm("tags") == DIR_NEW)
		return;

	/* open the file */
	if (!ioopen("tags", 'r', False, False, 't'))
		return;

	/* For each line from the tags file */
	ntagdefs = allocated = 0;
	tagdef = NULL;
	wasmsghide = msghide(True);
	bytes = ioread(tagline, QTY(tagline) - 1);
	while (bytes > 5) /* shortest possible legal tag line */
	{
		/* find the end of this line */
		for (src = tagline; src < &tagline[bytes] && *src != '\n'; src++)
		{
		}

		/* parse it */
		*src = '\0';
		tag = tagparse(tochar8(tagline));
		if (!tag)
			break;

		/* if for this file, and its definition isn't indented... */
		if (!strcmp(tag->TAGFILE, tochar8(o_filename(buf)))
		 && (isdigit(tag->TAGADDR[0]) || !isspace(tag->TAGADDR[2])))
		{
			/* if the tag has a "ln" attribute, start searching
			 * there -- saves *a lot* of time.
			 */
			memset((char *)&xinfb, 0, sizeof xinfb);
			(void)marktmp(xinfb.defaddr, buf, 0);
			for (i = 3; i < MAXATTR && tagattrname[i] && strcmp(tagattrname[i], "ln"); i++)
			{
			}
			if (i < MAXATTR && tag->attr[i] && (offset = atol(tag->attr[i])) > 1L)
				(void)marksetline(&xinfb.defaddr, offset - 1);

			/* locate the tag's definition within this buffer */
			scanstring(&cp, toCHAR(tag->TAGADDR));
			wasmagic = o_magic;
			o_magic = False;
			wassaveregexp = o_saveregexp;
			o_saveregexp = False;
			if (!exparseaddress(&cp, &xinfb))
			{
				scanfree(&cp);
				o_magic = wasmagic;
				o_saveregexp = wassaveregexp;
				goto NotFound;
			}
			scanfree(&cp);
			o_magic = wasmagic;
			o_saveregexp = wassaveregexp;
			offset = lowline(bufbufinfo(buf), xinfb.to);
			exfree(&xinfb);

			/* enlarge tagdef[] if necessary */
			if (ntagdefs + 1 > allocated)
			{
				allocated += 50;
				bigger = (TEDEF *)safealloc(allocated, sizeof(TEDEF));
				if (tagdef)
				{
					memcpy(bigger, tagdef, ntagdefs * sizeof(TEDEF));
					safefree(tagdef);
				}
				tagdef = bigger;
			}

			/* insert this tag into the array, sorted by offset */
			for (i = ntagdefs;
			     i > 0 && markoffset(tagdef[i - 1].where) > offset;
			     i--)
			{
					tagdef[i] = tagdef[i - 1];
			}
			tagdef[i].where = markalloc(buf, offset);
			tagdef[i].label = CHARdup(toCHAR(tag->TAGNAME));
			ntagdefs++;
		}
NotFound:

		/* delete this line from tagline[] */
		for (dst = tagline, src++, allnext = False; src < &tagline[bytes]; )
		{
			if (*src == '\n')
				allnext = True;
			*dst++ = *src++;
		}
		bytes = (int)(dst - tagline);

		/* if the next line is incomplete, read some more text
		 * from the tags file.
		 */
		if (!allnext)
		{
			bytes += ioread(dst, (int)QTY(tagline) - bytes - 1);
		}
	}
	(void)ioclose();
	msghide(wasmsghide);

	/* store the list */
	buf->tagdef = tagdef;
	buf->ntagdefs = ntagdefs;
	safeinspect();
}



/* free the tag definition info for a given buffer */
void tefreedef(buf)
	BUFFER	buf;
{
	int	i;

	safeinspect();
	if (buf->tagdef)
	{
		for (i = 0; i < buf->ntagdefs; i++)
		{
			markfree(buf->tagdef[i].where);
			safefree(buf->tagdef[i].label);
		}
		safefree((void *)buf->tagdef);
		buf->tagdef = NULL;
		buf->ntagdefs = 0;
	}
	safeinspect();
}



/* return the info about the tag that is defined at the cursor location. */
CHAR *telabel(cursor)
	MARK	cursor;
{
	int	i;
 static	CHAR	noinfo[1];
	TEDEF	*tagdef = markbuffer(cursor)->tagdef;

	/* if buffer has no tags, or the "showtag" option is off, then
	 * return no info
	 */
	if (!tagdef || !o_showtag)
		return noinfo;

	/* search for this MARK in the list */
	for (i = 0; i < markbuffer(cursor)->ntagdefs; i++)
	{
		if (markoffset(tagdef[i].where) > markoffset(cursor))
			break;
	}

	/* report what (if anything) we found */
	if (i == 0)
		return noinfo;
	else
		return tagdef[i - 1].label;
}
#endif /* defined(FEATURE_SHOWTAG) */

#ifdef FEATURE_COMPLETE
/* This function is used for completing a tag name.  It searches backward
 * from the provided cursor position to collect the characters of a partial
 * tag name, and then it looks for any known tags whose name matches that
 * partial name.  It returns a string containing any new characters that it
 * could match.
 */
CHAR *tagcomplete(win, m)
	WINDOW	win;	/* the window where multiple matches are listed */
	MARK	m;	/* the cursor position (end of partial name) */
{
	char	rest[300];
 static CHAR	retbuf[100];
	int	plen;		/* length of partial string */
	CHAR	*cp;
	int	mlen;
	TAG	*tag, *scan;
	long	oldtaglength;
	TAG	*oldtaglist;
	BOOLEAN	oldmsghide;
	BOOLEAN	oldexrefresh;
	CHAR	*oldprevioustag;
	DRAWSTATE olddrawstate;

	/* collect the characters of the partial name */
	rest[0] = '\0';
	for (scanalloc(&cp, m), plen = 0;
	     scanprev(&cp) && (isalnum(*cp) || *cp == '_') && plen < QTY(rest) - 1;
	     )
	{
		memmove(rest + 1, rest, QTY(rest) - 1);
		rest[0] = *cp;
		plen++;
	}
	scanfree(&cp);

	/* if no text, or a keyword, then just return a tab character */
	if (plen == 0 || dmskeyword(win, toCHAR(rest)))
	{
		retbuf[0] = '\t';
		retbuf[1] = '\0';
		return retbuf;
	}

	/* find the matching tags */
	oldtaglist = taglist;
	taglist = NULL;
	oldtaglength = o_taglength;
	oldmsghide = msghide(True);
	oldprevioustag = o_previoustag ? CHARdup(o_previoustag) : NULL;
	o_previoustag = NULL;
	o_taglength = plen;
	if (o_filename(markbuffer(m))
	 && strlen(rest) + 2 + CHARlen(o_filename(markbuffer(m))) < QTY(rest))
		sprintf(rest + strlen(rest), " file:%s", tochar8(o_filename(markbuffer(m))) );
	tag = tetag(toCHAR(rest));
	taglist = oldtaglist;
	msghide(oldmsghide);
	o_taglength = oldtaglength;
	if (o_previoustag)
		safefree(o_previoustag);
	o_previoustag = oldprevioustag;

	/* if no matches, then return a space */
	if (!tag)
	{
		CHARcpy(retbuf, toCHAR(" "));
		if (plen == 0)
			*retbuf = '\0';
		return retbuf;
	}

	/* eliminate duplicates */
	for (scan = tag; scan->next; )
	{
		if (!strcmp(scan->TAGNAME, scan->next->TAGNAME))
			scan->next = tagfree(scan->next);
		else
			scan = scan->next;
	}

	/* if only one match, then return the remainder of its name plus
	 * a space.
	 */
	if (!tag->next)
	{
		CHARcpy(retbuf, toCHAR(tag->TAGNAME + plen));
		CHARcat(retbuf, toCHAR(" "));
		while (tag)
			tag = tagfree(tag);
		return retbuf;
	}

	/* We have multiple matches.  Can we add any characters? */
	mlen = strlen(tag->TAGNAME);
	for (scan = tag->next; scan; scan = scan->next)
	{
		while (strncmp(tag->TAGNAME, scan->TAGNAME, mlen) != 0)
			mlen--;
	}

	/* If we can add some chars then do so */
	if (mlen > plen)
	{
		CHARncpy(retbuf, toCHAR(tag->TAGNAME + plen), mlen - plen);
		retbuf[mlen - plen] = '\0';
		while (tag)
			tag = tagfree(tag);
		return retbuf;
	}

	/* Else list all matches */
	plen = 0;
	olddrawstate = win->di->drawstate;
	for (scan = tag; scan; scan = scan->next)
	{
		mlen = strlen(scan->TAGNAME);
		if (plen + mlen + 1 >= o_columns(win))
		{
			drawextext(win, toCHAR("\n"), 1);
			plen = 0;
			olddrawstate = win->di->drawstate;
		}
		else if (plen > 0)
		{
			drawextext(win, blanks, 1);
			plen++;
		}
		drawextext(win, toCHAR(scan->TAGNAME), mlen);
		plen += mlen;
	}

	/* complete the last output line.  Note that we try to do this in
	 * a clever way which avoids prompting the user to "Hit <Enter> to
	 * continue" if the whole list fits on a single line.
	 */
	if (olddrawstate == DRAW_VISUAL)
	{
		oldexrefresh = o_exrefresh;
		o_exrefresh = True;
		drawextext(win, retbuf, 0);
		o_exrefresh = oldexrefresh;
		win->di->drawstate = DRAW_VMSG;
	}
	else
		drawextext(win, toCHAR("\n"), 1);

	/* we weren't able to extend the partial name at all */
	*retbuf = '\0';
	return retbuf;
}
#endif
