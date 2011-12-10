/* url.c */

#include "elvis.h"

static char	url_protocol[10];
static char	url_site_port[50];
static char	*url_resource;

#if USE_PROTOTYPES
static BOOLEAN parseurl(char *url);
static char *findproxy(BOOLEAN never_direct);
#endif

/* Parse an URL, and return True if successful. */
static BOOLEAN parseurl(url)
	char	*url;
{
	int	i;

	/* Copy the protocol into url_protocol[], converting to lowercase */
	for (i = 0; i < QTY(url_protocol) - 1 && isalpha(*url); i++, url++)
	{
		url_protocol[i] = tolower(*url);
	}
	url_protocol[i] = '\0';

	/* We expect a colon after the protocol.  If we don't have that, then
	 * fail.  Also fail if the protocol is less than two characters long,
	 * so we don't mistake the "C:" drive letter for a protocol.
	 */
	if (*url++ != ':' || i < 2)
	{
		return False;
	}

	/* Check for a host.  Not all protocols require one. */
	url_site_port[0] = '\0';
	if (url[0] == '/' && url[1] == '/')
	{
		url += 2;
		for (i = 0; i < QTY(url_site_port) - 1 && *url && *url != '/'; i++, url++)
		{
			url_site_port[i] = tolower(*url);
		}
		url_site_port[i] = '\0';
	}

	/* The rest of the URL is assumed to be a resource name.  If it is a
	 * null string, then assume it should be "/".
	 */
	url_resource = *url ? url : "/";

	/* some debugging code */
	if (o_verbose >= 4)
		msg(MSG_INFO, "[sss]protocol=$1, site_port=$2, resource=$3",
			url_protocol, url_site_port, url_resource);

	return True;
}

/* If the URL refers to a local file, this returns the name of the
 * file.  This handles "file:" protocol and names which have no
 * protocol; for all others it returns NULL.  It is also smart enough
 * to realize that "C:" is a drive letter rather than a protocol.
 */
char *urllocal(url)
	char	*url;
{
	if (!parseurl(url))
		return url;
	else if (!strcmp(url_protocol, "file"))
		return url_resource;
	return NULL;
}

/* Check the permissions on a URL. */
DIRPERM urlperm(url)
	char	*url;
{
#if defined(PROTOCOL_HTTP) || defined(PROTOCOL_FTP)
	/* Verify that this is a remote URL, other than mailto:.  This also
	 * has the side-effect of parsing the URL.  For local URLs try using
	 * the dirperm() function.
	 */
	if (!urlremote(url))
		return dirperm(url);

	/* If the protocol is anything other than "ftp" then assume the URL
	 * exists and is readonly.
	 */
	if (strcmp(url_protocol, "ftp"))
		return DIR_READONLY;

	/* Else use FTP to inspect the file (not easy!) and then return
	 * whatever permissions it found.
	 */
	(void)ftpopen(url_site_port, url_resource, False, 'p');
	return ftpperms;

#else /* no network protocols */

	return dirperm(url);
#endif
}



#if defined(PROTOCOL_HTTP) || defined(PROTOCOL_FTP)

/* Return True if the url uses a protocol other than "mailto:" or
 * "file:".
 */
BOOLEAN urlremote(url)
	char	*url;
{
	/* Parse the url.  If parsing fails, then it isn't remote.  Also,
	 * check for a few specific protocols which aren't supported.
	 */
	if (!parseurl(url)
	 || !strcmp(url_protocol, "mailto")
	 || !strcmp(url_protocol, "file"))
		return False;
	return True;
}

static long	totalbytes;
static long	expectbytes;
static long	oldpollfreq;
#ifdef PROTOCOL_FTP
static BOOLEAN	useftp;
#endif

/* Scan through the elvis.net file for this URL's domain.  Return the proxy
 * site's name if found, or NULL if the URL should be accessed directly.
 * The proxy name will be in a dynamically-allocated buffer, which must be
 * freed by the calling function.
 *
 * This assumes that parseurl() was recently called on the URL, so the site
 * name can be found in url_site_port[].
 */
static char *findproxy(never_direct)
	BOOLEAN	never_direct;	/* protocol isn't supported directly by elvis */
{
	CHAR	*word, *proxy;	/* words from file */
	int	sitelen, len;	/* lengths of site name and current word */
	FILE	*fp;		/* stream used for reading from file */
	int	ch;		/* character from file */
	BOOLEAN	expect_proxy;	/* is next word expected to be proxy name? */
	char	*tmp;

	/* compute the length of the site name, excluding the port number */
	tmp = strchr(url_site_port, ':');
	if (tmp)
		sitelen = (int)(tmp - url_site_port);
	else
		sitelen = strlen(url_site_port);

	/* find the "elvis.net" file (actually the buffer containing it) */
	tmp = iopath(tochar8(o_elvispath), NET_FILE, False);
	if (!tmp)
		return NULL;

	/* scan for the domain in the "elvis.net" file */
	fp = fopen(tmp, "r");
	proxy = word = NULL;
	expect_proxy = False;
	while ((ch = getc(fp)) != EOF)
	{
		/* If '#' is encountered, then skip ahead to the next newline.
		 * Later code will then see the newline and hence treat the
		 * entire comment like whitespace.
		 */
		if (ch == '#')
		{
			do
			{
				if ((ch = getc(fp)) == EOF)
					goto End;
			} while (ch != '\n');
		}

		/* non-whitespace just adds the character to the current word */
		if (!isspace(ch))
		{
			buildCHAR(&word, ch);
			continue;
		}

		/* if we have no word (multiple spaces) then continue */
		if (!word)
			continue;

		/* if this word is supposed to be a proxy name, then use it */
		if (expect_proxy)
		{
			expect_proxy = False;
			assert(!proxy);
			proxy = word;
			word = NULL;
			continue;
		}

		/* Handle the "direct" keyword */
		if (!strcmp(tochar8(word), "direct"))
		{
			/* forget the current proxy */
			if (proxy && !never_direct)
			{
				safefree(proxy);
				proxy = NULL;
			}
			goto NextWord;
		}

		/* handle the "proxy" keyword */
		if (!strcmp(tochar8(word), "proxy"))
		{
			if (proxy)
			{
				safefree(proxy);
				proxy = NULL;
			}
			expect_proxy = True;
			goto NextWord;
		}

		/* Then it must be a domain name.  Is the the URL's? */
		len = CHARlen(word);
		if ((len < sitelen && *word == '.' && !strncmp(&url_site_port[len - sitelen], tochar8(word), len))
		 || (len == sitelen && !strncmp(url_site_port, tochar8(word), len)))
		{
			/* Found it! Stop looking! */
			break;
		}

		/* otherwise we can forget the current word */
NextWord:
		safefree(word);
		word = NULL;
	}
	fclose(fp);

	/* if there was a last word, free it */
End:
	if (word)
		safefree(word);

	/* return the proxy (which may be NULL) */
	return tochar8(proxy);
}


/* Open a remote URL for reading or writing.  Returns True if successful.  If
 * error, it gives an error message and returns False.
 */
BOOLEAN urlopen(url, force, rwa)
	char	*url;
	BOOLEAN	force;
	_char_	rwa;
{
	char	*proxy;
	BOOLEAN	unsupported;
	BOOLEAN	reading;
	BOOLEAN	retval;

	/* reset the expectbytes/totalbytes values to an impossible value */
	expectbytes = -1L;
	totalbytes = 0;

	/* Verify that this is a remote URL, other than mailto:.  This also
	 * has the side-effect of parsing the URL.
	 */
	if (!urlremote(url))
	{
		msg(MSG_ERROR, "[s]$1 not a remote url", url);
		return False;
	}

	/* set "unsupported" if this isn't a built-in protocol */
	unsupported = (BOOLEAN)!(
#ifdef PROTOCOL_FTP
		!strcmp(url_protocol, "ftp") ||
#endif
		!strcmp(url_protocol, "http"));

	/* If a proxy is specified in "elvis.net", then use it.  Otherwise
	 * we'll access the site/port directly.
	 */
	proxy = findproxy(unsupported);
	if (proxy)
	{
		/* Use the proxy.  Assume it uses the HTTP protocol */
		strcpy(url_protocol, "http");
		strcpy(url_site_port, proxy);
		url_resource = url;

		/* Free the proxy name, but don't zero the proxy pointer.
		 * Later, we can still compare proxt against NULL, to detect
		 * whether we're using a proxy.
		 */
		safefree(proxy);
	}
	else
	{
		/* We'll access it directly... unless we don't know how! */
		if (unsupported)
		{
			msg(MSG_ERROR, "[s]unsupported protocol $1", url_protocol);
			return False;
		}
	}

	/* Will we be reading or writing? */
	reading = (BOOLEAN)(rwa == 'r');

	/* arrange for frequent polling during the network operation */
	oldpollfreq = o_pollfrequency;
	o_pollfrequency = 1L;
	(void)guipoll(True);

	/* At this point, we know we have a supported protocol.  Which one? */
#ifdef PROTOCOL_FTP
	useftp = (BOOLEAN)!strcmp(url_protocol, "ftp");
	if (useftp)
		retval = ftpopen(url_site_port, url_resource, force, rwa);
	else
#endif
	{
		if (!reading)
		{
			msg(MSG_ERROR, "[s]can only READ from http $1", proxy ? "proxy" : "server");
			retval = False;
		}
		else
			retval = httpopen(url_site_port, url_resource);
	}

	/* restore pollfrequency */
	o_pollfrequency = oldpollfreq;
	return retval;
}

/* During a "write" operation, this value will be displayed to the
 * user as the total number of bytes to be transferred.  You should
 * call urlbytes() after calling urlopen() with 'w' or 'a' as the
 * rwa parameter.  Alternatively, you may omit calling this function
 * in which case elvis simply won't display the total number of bytes
 * to the user.  When rwa is 'r', this function does nothing.
 *
 * Is is also harmless to call urlbytes() after other flavors of ioopen()
 * for writing; i.e., you can safely call urlbytes() after any ioopen() which
 * has an rwa argument of anything except 'r'.  The httpopen() and ftpopen()
 * functions call urlbytes() for 'r' operations.
 */
void urlbytes(totbytes)
	long	totbytes;	/* number of bytes we expect to write */
{
	expectbytes = totbytes;
}

/* Read data from an URL.  Returns the number of bytes read (0 at EOF);
 * if error or user-abort, it given an error message and returns -1.
 */
int urlread(buf, bytes)
	CHAR	*buf;
	int	bytes;
{
#ifdef PROTOCOL_FTP
	if (useftp)
		bytes = ftpread(buf, bytes);
	else
#endif
		bytes = httpread(buf, bytes);
	if (bytes > 0)
	{
		totalbytes += bytes;
		if (windefault)
		{
			if (totalbytes <= expectbytes)
				msg(MSG_STATUS, "[dd]receiving... $1 of $2 bytes", totalbytes, expectbytes);
			else
				msg(MSG_STATUS, "[d]receiving... $1 bytes", totalbytes);
		}
	}
	return bytes;
}

/* Write data to an URL.  Returns the number of bytes written;
 * if error or user-abort, it given an error message and returns -1.
 */
int urlwrite(buf, bytes)
	CHAR	*buf;
	int	bytes;
{
#ifdef PROTOCOL_FTP
	if (useftp)
		bytes = ftpwrite(buf, bytes);
	/* there is no httpwrite() */

	if (bytes > 0)
	{
		totalbytes += bytes;
		if (windefault)
		{
			if (totalbytes <= expectbytes)
				msg(MSG_STATUS, "[dd]sending... $1 of $2 bytes", totalbytes, expectbytes);
			else
				msg(MSG_STATUS, "[d]sending... $1 bytes", totalbytes);
		}
	}
#endif
	return bytes;
}

/* Close an URL. */
void urlclose P_((void))
{
#ifdef PROTOCOL_FTP
	if (useftp)
		ftpclose();
	else
#endif
		httpclose();

	/* restore the old poll frequency */
	o_pollfrequency = oldpollfreq;
}

#endif /* PROTOCOL_HTTP || PROTOCOL_FTP */
