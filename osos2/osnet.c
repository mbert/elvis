/* osos2/osnet.c */

/* This is basically identical to the win32 version, adapted for OS/2 by
 * Herbert */

/* This function contains functions for accessing the internet.  These are
 * all protocol-independent, but (to some degree) OS-dependent.  Specifically,
 * the following functions are defined:
 *
 *   sockbuf_t *netconnect(char *site_port, unsigned int defport);
 *	Open a socket connection to a specific port on a specific site.
 *	site_port is a host name or numbers-and-dots address, optionally
 *	followed by a port number.  defport is the port number to use if
 *	site_port contains no port number.
 *
 *   void netdisconnect(sockbuf_t *sb);
 *	Close a socket connection which was created via netconnect()
 *
 *   BOOLEAN netread(sockbuf_t *sb);
 *	Read bytes from sb's socket.  Returns True if successful (even if
 *	no data has been read, at the end of input), or False if error or
 *	user-abort (after giving an error message).
 *
 *   char *netgetline(sockbuf_t *sb);
 *	Fetch the next line of text from a socket, and return it.
 *	Returns NULL if error or user-abort.
 *
 *   char *netbuffer(sockbuf_t *sb);
 *	Return a pointer to the next byte in sb's read-buffer.
 *	(This is actually a macro, defined in osnet.h.)
 *
 *   int netbytes(sockbuf_t *sb);
 *	Return the number of bytes in sb's read-buffer.
 *	(This is actually a macro, defined in osnet.h.)
 *
 *   void netconsume(sockbuf_t *sb, int len);
 *	Consume len bytes from sb's read-buffer.
 *	(This is actually a macro, defined in osnet.h)
 *
 *   BOOLEAN netwrite(sockbuf_t *sb, char *text, int bytes);
 *	Write data to sb's socket.  Returns True iff successful.
 *
 *   BOOLEAN netputline(sockbuf_t *sb, char *text);
 *	Write a line to sb's socket.  text is a NUL-terminated line;
 *	netlinewrite() adds a CR-LF pair itself.
 *
 *   char *netself(void)
 *	Return the name of the local system.
 */

#include "elvis.h"
#if defined(PROTOCOL_HTTP) || defined(PROTOCOL_FTP)
# ifdef __IBMC__
#  define BSD_SELECT
#  include <types.h>
# endif
# ifdef __EMX__
#  include <sys/types.h>
#  include <sys/time.h>
# endif
# include <sys/socket.h>
# include <netdb.h>
# ifdef __EMX__
#  include <io.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
# endif
# include <sys/select.h>
# ifdef __IBMC__
#  define close(s)	    soclose(s)
#  define read(s,p,n)	recv(s, p, n, 0)
#  define write(s,p,n)	send(s, p, n, 0)
# endif

# if USE_PROTOTYPES
static BOOLEAN site2addr (char *site, struct in_addr *address);
# endif


#ifndef __EMX__
static BOOLEAN initialized;
#endif

static int inet_aton (char *site, struct in_addr *addr)
{
	int a0,a1,a2,a3;

	if (sscanf (site, "%d.%d.%d.%d", &a0, &a1, &a2, &a3) != 4)
		return 0;
	((char *)&addr->s_addr)[0] = a0;
	((char *)&addr->s_addr)[1] = a1;
	((char *)&addr->s_addr)[2] = a2;
	((char *)&addr->s_addr)[3] = a3;
	return 1;
}

/* Find the address of a site.  The site can be given as either a domain name,
 * or a "numbers and dots" name.  If successful, the address is stuffed into
 * address struct, and True is returned; otherwise False is returned.
 */
static BOOLEAN site2addr(site, address)
	char		*site;
	struct in_addr	*address;
{
	struct hostent		*siteinfo;
	static char		prevsite[100];
	static struct in_addr	prevaddr;


	/* if the site name starts with a digit, then assume it is in the
	 * "numbers and dots" format.
	 */
	if (isdigit(site[0]))
	{
		/* convert to binary address */
		if (!inet_aton(site, address))
			goto Error;
	}
	else
	{
		/* if same name as last time, then just use the same address */
		if (strlen(site) < QTY(prevsite) && !strcmp(prevsite, site))
		{
			*address = prevaddr;
			return True;
		}

		/* look up the name */
		msg(MSG_STATUS, "[s]looking up site $1", site); /* !!!*/
		siteinfo = gethostbyname(site);
		if (!siteinfo)
		{
			goto Error;
		}

		/* use the primary address */
		memcpy(address, siteinfo->h_addr_list[0], siteinfo->h_length);

		/* save the info about this site */
		strncpy(prevsite, site, sizeof prevsite);
		prevaddr = *address;
	}

	return True;

Error:
	msg(MSG_ERROR, "[s]unknown site $1", site);
	return False;
}

/* Open a connection to a given site and port.
 * Returns a sockbuf_t pointer if successful, or NULL for errors (in which
 * case it also gives an error message).
 */
sockbuf_t *netconnect (site_port, defport)
	char		*site_port;	/* name of remote system, with optional port number */
	unsigned int	defport;
{
	struct in_addr	serverAddress;
	struct sockaddr_in destPort;
	char		*tmp;
	char		buf[150];
	sockbuf_t	*sb;

#ifndef __EMX__
	/* If first time, then initialize OS/2 sockets */
	if (!initialized)
	{
	    (void)sock_init ();
		initialized = True;
	}
#endif

	/* if site_port contains a port number, then separate it from the site
	 * name, and use the given port instead of the default port.
	 */
	strcpy (buf, site_port);
	tmp = strchr (buf, ':');
	if (tmp)
	{
		*tmp++ = '\0';
		defport = atoi (tmp);
	}

	/* look up the address of the server */
	if (!site2addr (buf, &serverAddress))
		return NULL;

	/* The remainder of this function connects to the server */
	msg (MSG_STATUS, "[s]connecting to $1", buf);

	/* create a socket, and a sockbuf_t to buffer it */
	sb = safealloc(1, sizeof(sockbuf_t));
	sb->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (sb->fd < 0)
	{
		safefree(sb);
		return NULL;
	}

	/* connect the socket to the remote machine */
	destPort.sin_family = AF_INET;
	destPort.sin_port = htons((unsigned short)defport);
	destPort.sin_addr = serverAddress;
	if (connect(sb->fd, (struct sockaddr *)&destPort, sizeof(destPort)))
	{
		netdisconnect(sb);
		return NULL;
	}

	return sb;
}


void netdisconnect (sb)
	sockbuf_t	*sb;	/* buffered socket to be freed */
{
	close(sb->fd);
	safefree(sb);
}


/* Read as much data as possible from a socket, with a timeout.  Returns True
 * if successful, or False if there was an error or the user aborted; in the
 * latter cases, an error message is given.
 */
BOOLEAN netread (sockbuf_t *sb)
{
	int		i;
	int		gotfds;
	fd_set		rfds;
	struct timeval	timeout;

	/* Wait for data, checking periodically for a user abort. */
	do
	{
		/* try to read for up to 2 seconds */
		FD_ZERO (&rfds);
		FD_SET (sb->fd, &rfds);
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		gotfds = select (sb->fd + 1, &rfds, NULL, NULL, &timeout);
		if (gotfds < 0 || guipoll(False))
		{
			return False;
		}
	} while (gotfds == 0);

	/* remove any consumed text from the buffer */
	if (sb->left > 0 && sb->right > sb->left)
		/* regions may overlap -- it is guaranteed to work */
		memmove(sb->buf, &sb->buf[sb->left], sb->right - sb->left);
	sb->right -= sb->left;
	sb->left = 0;

	/* Read as much data as is available */
	i = read(sb->fd, &sb->buf[sb->right], sizeof sb->buf - sb->right);
	if (i < 0)
		i = read(sb->fd, &sb->buf[sb->right], sizeof sb->buf - sb->right);
	if (i < 0)
	{
		msg(MSG_ERROR, "error reading from socket");
		return False;
	}
	sb->right += i;

	/* return the number of bytes available */
	return True;
}


/* Read a line from a socket.  Return the line if successful, NULL if error. */
char *netgetline(sockbuf_t *sb)
{
	int	i;
	char	*ret;

	/* loop until something interesting happens */
	for (;;)
	{
		/* see if we have a whole line now */
		for (ret = netbuffer(sb), i = sb->left; i < sb->right; i++)
		{
			if (sb->buf[i] == '\n')
			{
				/* found end-of-line!  Mark it & return. */
				sb->buf[i] = '\0';
				if (i >= 1 && sb->buf[i - 1] == '\r')
					sb->buf[i - 1] = '\0';
				sb->left = i + 1;
				return ret;
			}
		}

		/* read some more data */
		i = netbytes(sb);
		if (!netread(sb) || i == netbytes(sb))
			return NULL;
	}
	/*NOTREACHED*/
}


/* Send data bytes through a socket.  Returns True if successful, or False if
 * error (after giving an error message).
 */
BOOLEAN netwrite(sb, data, len)
	sockbuf_t	*sb;
	char		*data;
	int		len;
{
	if (write(sb->fd, data, len) == len)
		return True;
	msg(MSG_ERROR, "transmission failed");
	return False;
}


/* Send a line through a socket.  This intended to be used for sending commands
 * to an FTP or HTTP server.  The line should be a normal NUL-terminated string
 * with no newline; this function appends a CRLF.  Returns True if successful,
 * or False if error (after giving an error message).
 */
BOOLEAN netputline(sb, command, arg1, arg2)
	sockbuf_t	*sb;		/* stream to write to */
	char		*command;	/* command name */
	char		*arg1, *arg2;	/* arguments, may be NULL if not used */
{
	int	len;
	char	*buf;

	/* combine the command and arg into one string */
	if (arg2)
	{
		len = strlen(command) + strlen(arg1) + strlen(arg2) + 5;
		buf = safealloc(len, sizeof(char));
		sprintf(buf, "%s %s %s\r\n", command, arg1, arg2);
	}
	else if (arg1)
	{
		len = strlen(command) + strlen(arg1) + 4;
		buf = safealloc(len, sizeof(char));
		sprintf(buf, "%s %s\r\n", command, arg1);
	}
	else
	{
		len = strlen(command) + 3;
		buf = safealloc(len, sizeof(char));
		sprintf(buf, "%s\r\n", command);
	}
	len--; /* <-- so the NUL terminator isn't sent */

	/* send the command to the server */
	if (write(sb->fd, buf, len) != len && *command)
	{
		msg(MSG_ERROR, "could not send request to server");
		safefree(buf);
		return False;
	}
	safefree(buf);
	return True;
}


char *netself P_((void))
{
	static char	name[50];

	if (gethostname(name, sizeof(name)))
		strcpy(name, "graceland");
	return name;
}

#endif /* PROTOCOL_HTTP || PROTOCOL_FTP */
