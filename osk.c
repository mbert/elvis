/* osk.c */

/* ------------------------------------------------------------------- *
 |
 | OS9Lib:  stat(), fstat()
 |
 |
 |     Copyright (c) 1988 by Wolfgang Ocker, Puchheim,
 |                           Ulli Dessauer, Germering and
 |                           Reimer Mellin, Muenchen
 |                           (W-Germany)
 |
 |  This  programm can  be  copied and  distributed freely  for any
 |  non-commercial  purposes.   It can only  be  incorporated  into
 |  commercial software with the written permission of the authors.
 |
 |  If you should modify this program, the authors would appreciate
 |  a notice about the changes. Please send a (context) diff or the
 |  complete source to:
 |
 |  address:     Wolfgang Ocker
 |               Lochhauserstrasse 35a
 |               D-8039 Puchheim
 |               West Germany
 |
 |  e-mail:      weo@altger.UUCP, ud@altger.UUCP, ram@altger.UUCP
 |               pyramid!tmpmbx!recco!weo
 |               pyramid!tmpmbx!nitmar!ud
 |               pyramid!tmpmbx!ramsys!ram
 |
 * ----------------------------------------------------------------- */

#ifdef OSK

#define PATCHLEVEL 1

#include <module.h>
#include <sgstat.h>
#include <sg_codes.h>
#include <direct.h>
#ifndef ELVPRSV
#include <stdio.h>
#include <errno.h>
#include <modes.h>
#include <signal.h>
#include "config.h"
#endif
#include "osk.h"

#define TIME(secs) (((secs << 8) / 10) | 0x80000000)

/*
 * f s t a t
 */
int fstat(fd, buff)
  int		 fd;
  struct stat *buff;
{
  struct fildes ftmp;
  struct tm	 ttmp;
  struct _sgr   fopt;

  if (_gs_gfd(fd, &ftmp, 16) < 0) /* 16 insteat of sizeof(struct fildes)   */
	return(-1);				   /* used due to a bug in stupid os9net */

  if (_gs_opt(fd, &fopt) < 0)
	return(-1);

  ttmp.tm_year  = (int) ftmp.fd_date[0];
  ttmp.tm_mon   = (int) ftmp.fd_date[1] - 1;
  ttmp.tm_mday  = (int) ftmp.fd_date[2];	
  ttmp.tm_hour  = (int) ftmp.fd_date[3];
  ttmp.tm_min   = (int) ftmp.fd_date[4];
  ttmp.tm_sec   = 0;
  ttmp.tm_isdst = -1;

  buff->st_atime = buff->st_mtime = mktime(&ttmp);

  ttmp.tm_year  = (int) ftmp.fd_dcr[0];
  ttmp.tm_mon   = (int) ftmp.fd_dcr[1] - 1;
  ttmp.tm_mday  = (int) ftmp.fd_dcr[2];	
  ttmp.tm_hour  = ttmp.tm_min = ttmp.tm_sec = 0;
  ttmp.tm_isdst = -1;
  
  buff->st_ctime = mktime(&ttmp);

  memcpy(&(buff->st_size), ftmp.fd_fsize, sizeof(long));  /* misalignment! */
  buff->st_uid   = ftmp.fd_own[1];
  buff->st_gid   = ftmp.fd_own[0];
  buff->st_mode  = ftmp.fd_att;
  buff->st_nlink = ftmp.fd_link;

  buff->st_ino   = fopt._sgr_fdpsn;
  buff->st_dev   = fopt._sgr_dvt;

  return(0);
}

/*
 * s t a t
 */	
int stat(filename, buff)
  char		*filename;
  struct stat *buff;
{
  register int i, ret;

  if ((i = open(filename, S_IREAD)) < 0)
	if ((i = open(filename, S_IFDIR | S_IREAD)) < 0)
	  return(-1);

  ret = fstat(i, buff);
  close(i);

  return(ret);
}

/*
	unix library functions mist in OSK
	Author: Peter Reinig
*/


typedef (*procref)();
#define MAX_SIGNAL 10

extern exit();

static int (*sig_table[MAX_SIGNAL])();
static int _sig_install = 0;

sig_handler(sig)
int sig;
{
	if ((int) sig_table[sig] > MAX_SIGNAL)
		sig_table[sig](sig);
}

procref signal(sig,func)
int sig;
int (*func)();
{
	int i, (*sav)();

	if (!_sig_install) {
		for (i=0; i < MAX_SIGNAL; i++)
			sig_table[i] = exit;
		_sig_install = 1;
		intercept(sig_handler);
	}	
	sav = sig_table[sig];
	switch ((int) func) {
		case SIG_DFL : sig_table[sig] = exit;
					   break;
		case SIG_IGN : sig_table[sig] = 0;
					   break;
		default	  : sig_table[sig] = func;
					   break;
	}
	return sav;
}

perror(str)
char *str;
{
	static int path = 0;
	if (!path && (path = open("/dd/sys/Errmsg", S_IREAD)) == -1) {
		fprintf(stderr,"Can\'t open error message file\n");
		path = 0;
	}
	if (str && *str) {
		fprintf(stderr,"%s: ",str);
		fflush(stderr);
	}
	prerr(path,(short) errno);
}

isatty(fd)
int fd;
{
	struct sgbuf buffer;
	char type;

	_gs_opt(fd,&buffer);
	type = buffer.sg_class;
	if (type == DT_SCF)
		return 1;
	else
		return 0;
}

static struct passwd pw;
static char line[128];

struct passwd *getpwuid(uid)
int uid;
{
	FILE   *fp;
	register char *p, *q;

	if ((fp = fopen(PASSWD, "r")) == NULL)
		return (struct passwd *) NULL;
	while (fgets(line, sizeof(line), fp)) {
		p = q = line;
		while (*p && *p != ',') p++;
		if (!*p)
			continue;
		*p = '\0';
		pw.pw_name = q;
		q = ++p;
		while (*p && *p != ',') p++;
		if (!*p)
			continue;
		*p = '\0';
		pw.pw_passwd = q;
		q = ++p;
		while (*p && *p != '.') p++;
		if (!*p)
			continue;
		*p = '\0';
		pw.pw_gid = atoi(q);
		q = ++p;
		while (*p && *p != ',') p++;
		if (!*p)
			continue;
		*p = '\0';
		pw.pw_uid = atoi(q);
		q = ++p;
		if (uid != pw.pw_uid)
			continue;
		while (*p && *p != ',') p++;
		if (!*p)
			return (struct passwd *) NULL;
		*p = '\0';
		pw.pw_prio = atoi(q);
		q = ++p;
		while (*p && *p != ',') p++;
		if (!*p)
			return (struct passwd *) NULL;
		*p = '\0';
		pw.pw_xdir = q;
		q = ++p;
		while (*p && *p != ',') p++;
		if (!*p)
			return (struct passwd *) NULL;
		*p = '\0';
		pw.pw_dir = q;
		p++;
		if (!*p)
			return (struct passwd *) NULL;
		pw.pw_shell = p;
		while (*p++) ;
		*(--p) = '\0';
		return &pw;
	}
	return (struct passwd *) NULL;
}

/* This function is used to catch the alarm signal */
static int dummy()
{
}

/* This function implements read-with-timeout from the keyboard.*/
int ttyread(buf, len, time)
	char	*buf;	/* where to store the gotten characters */
	int	len;	/* maximum number of characters to read */
	int	time;	/* maximum time to allow for reading characters */
{
	REG int	i;
	int	alrmid;

	/* are some characters available in the type-ahead buffer? */
	if ((i = _gs_rdy(0)) > 0)
	{
		/* some characters are available -- read them immediately */
		len = read(0, buf, i < len ? i : len);
	}
	else if (!time) /* reading with no timeout? */
	{
		/* do a blocking read, with no timeout */
		do
			len = read(0, buf, 1);
		while (len < 0);
	}
	else
	{
		/* set an alarm and then do a blocking read */
		signal(SIGQUIT, dummy);
		alrmid = alm_set(SIGQUIT, TIME(time));
		len =  read(0, buf, 1);
		alm_delete(alrmid);
	}
	return len;
}

/* The code of getcwd, popen and pclose is taken from blarslib from Bob Larson */

/* Internet:	blarson@usc.edu */
/* StG:			blarson@zog */
/* Compuserve:	75126.723@compuserve.com */

char *getcwd(p, n)
char *p;
int n;
{
	register char *cp;
	register struct dirent *dp;
	register int l, olddot = 0, i, d, dot, dotdot;
	struct dirent db[8];
	char buf[1024];

	if(p==NULL) {
		p = (char *)malloc((unsigned)n);
		if(p==NULL) return NULL;
	}
	cp = &buf[1024-1];
	*cp = '\0';
	for(;;) {
		if((d = open(".", S_IREAD | S_IFDIR)) < 0) {
			if(*cp) chdir(cp+1);
			return NULL;
		}
		if((i = read(d, (char *)db, sizeof(db))) == 0) {
			if(*cp) chdir(cp+1);
			close(d);
			return NULL;
		}
		dotdot = db[0].dir_addr;
		dot = db[1].dir_addr;
		if(olddot) {
			i -= 2 * sizeof(struct dirent);
			dp = &db[2];
			for(;;) {
				if(i <= 0) {
					if((i = read(d, (char *)db, sizeof(db))) == 0) {
					if(*cp) chdir(cp+1);
					close(d);
					return NULL;
				}
			dp = &db[0];
			}
			if(olddot == dp->dir_addr) {
				l = strlen(dp->dir_name);
				/* last character has parity bit set... */
				*--cp = dp->dir_name[--l] & 0x7f;
				while(l) *--cp = dp->dir_name[--l];
				*--cp = '/';
				break;
			}
			i -= sizeof(struct dirent);
			dp++;
			}
		}
		if(dot==dotdot) {
			if(*cp) chdir(cp+1);
			*p = '/';
			if(_gs_devn(d, p+1) < 0) {
				close(d);
				return NULL;
			}
			close(d);
			if(n < (strlen(p) + strlen(cp))) return NULL;
			strcat(p, cp);
			return p;
		}
		close(d);
		if(chdir("..") != 0) {
			if(*cp) chdir(cp+1);
			return NULL;
		}
		olddot = dot;
	}
}

extern char *environ;
extern int os9forkc();

static int proc[_NFILE];

/* This version of popen is derived from Robert B. Larson library blarslib *
/* and was modified by Peter Reinig to meet the needs of elvis */

FILE *popen(command, mode)
char *command;
char *mode;
{
	int pipe;

	if (pipe = osk_popen(command, mode, 0, 1))
		return (fdopen(pipe, mode));
	else
		return ((FILE*) NULL);
}

mod_exec *mp = -1;

int osk_popen(command, mode, in, as_popen)
char *command;
char *mode;
int in, as_popen;
{
	int temp, fd, stdinp;
	int pipe, pid;
	char *argv[4];
	register char *cp;
	static char namebuffer[128];
	static char module[128];

	if(mode[1]!='\0' || (*mode!='r' && *mode!='w'))
		return 0;
	fd = (*mode=='r');
	if((temp = dup(fd)) <= 0)
		return 0;
	if((pipe = creat("/pipe", S_IREAD | S_IWRITE)) < 0) {
		close(temp);
		return 0;
	}
	close(fd);
	dup(pipe);
	if (in != 0) {
		stdinp = dup(0);
		close(0);
		dup(in);
		close(in);
	}
	argv[0] = "shell";
	argv[1] = "ex";
	argv[2] = command;
	argv[3] = (char *)NULL;
	strcpy(module, command);
	cp = module;
	while (*cp && *cp != ' ' && *cp != '\t') cp++;
	*cp = '\0';
	mp = (mod_exec *)modloadp(module, (MT_PROGRAM << 8) + ML_ANY, namebuffer);
	if((mp == (mod_exec*)-1)
		|| ((pid = os9exec(os9forkc, argv[0], argv, environ, 0, 0, 3)) < 0)) {
		if (mp > 0)
			munlink(mp);
		mp = (mod_exec *) -1;
		close(fd);
		close(pipe);
		dup(temp);
		close(temp);
		if (in != 0) {
			close(0);
			dup(stdinp);
		}
		return 0;
	}
	if (as_popen)
		proc[pipe] = pid;
	close(fd);
	dup(temp);
	close(temp);
	if (in != 0) {
		close(0);
		dup(stdinp);
	}
	return pipe;
}

int pclose(pipe)
FILE *pipe;
{
	int p, stat, w;

	if((p = proc[fileno(pipe)]) <= 0) return -1;
	proc[fileno(pipe)] = 0;
	fflush(pipe);
	fclose(pipe);
	while((w = wait(&stat)) != -1 && w != p) ;
	if (mp > 0)
		munlink(mp);
	mp = (mod_exec *) -1;
	return w == -1 ? -1 : stat;
}
#endif /* OSK */
