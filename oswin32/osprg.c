/* oswin32/osprg.c */

char id_osprg[] = "$Id: osprg.c,v 2.13 1999/03/03 18:41:04 steve Exp $";

#include <windows.h>
#include <wtypes.h>
#include <io.h>
#define CHAR ElvisCHAR
#define BOOLEAN ElvisBOOLEAN
#include "elvis.h"

/* NOTE: Previous versions of this program attempted to use pipes.  But
 * there are some undocumented problems with pipes, and I/O redirection
 * in general, so I decided to "dumb it down" to the basics.  It now
 * creates batch file to run the command.
 */

#define TMPDIR	(o_directory ? tochar8(o_directory) : ".")

static char	namestdin[MAX_PATH];/* name of stdin temp file */
static char	namestdout[MAX_PATH];/* name of stdout/stderr file */
static char	namebatch[MAX_PATH];/* name of batch file */
static DWORD	status;		/* exit status of program */
static FILE	*fp;
static int	unique;
static SECURITY_ATTRIBUTES inherit = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
static STARTUPINFO start = {sizeof(STARTUPINFO)};


static BOOLEAN runbatch(void)
{
	HANDLE	pid, tid;
	CHAR	*cmdline;
	PROCESS_INFORMATION proc;
	BOOLEAN	result;


	/* build the shell invocation command, to run the batch file */
	cmdline = NULL;
	if (o_shell && *o_shell)
		buildstr(&cmdline, tochar8(o_shell));
	else
		buildstr(&cmdline, "command.com");
	buildstr(&cmdline, " /c ");
	buildstr(&cmdline, namebatch);

	memset(&proc, 0, sizeof proc);

#ifdef GUI_WIN32
	/* For the "win32" gui, any subprocesses will normally start out
	 * minimized, because we don't really care what's in its window.
	 * Win32 just does weird things with console-less programs, and I'm
	 * tired of fighting it.
	 */
	start.wShowWindow = SW_HIDE;
	start.dwFlags = STARTF_USESHOWWINDOW;
#endif

	/* run the command.  First try to run it without a shell.  If
	 * that fails (probably because the command is built into the
	 * shell) then try the same command with the shell.
	 */
	result = CreateProcess(NULL, tochar8(cmdline),
			NULL, NULL, TRUE,
#ifdef GUI_WIN32
			DETACHED_PROCESS | NORMAL_PRIORITY_CLASS,
#else
			NORMAL_PRIORITY_CLASS,
#endif
			NULL, NULL, &start, &proc);

	/* remember info about the process */
	if (result)
	{
		pid = proc.hProcess;
		tid = proc.hThread;
		(void)WaitForSingleObject(pid, INFINITE);
		GetExitCodeProcess(pid, &status);
		CloseHandle(pid);
		CloseHandle(tid);
	}

	/* free the command line */
	safefree(cmdline);

	/* return the result */
	return result;
}


/* Declares which program we'll run, and what we'll be doing with it.
 * This function should return True if successful.  If there is an error,
 * it should issue an error message via msg(), and return False.
 *
 * For Win32, this creates a batch file, to be run by prggo().
 */
ElvisBOOLEAN prgopen(cmd, willwrite, willread)
	char		*cmd;		/* command string */
	ElvisBOOLEAN	willwrite;	/* if True, redirect command's stdin */
	ElvisBOOLEAN	willread;	/* if True, redirect command's stdout */
{
	HANDLE	bfd;

	/* choose a name for the batch file */
	do
	{
		if (unique == 0)
			unique = (UINT)GetCurrentProcessId();
		else if (unique == 0xfffe)
			unique = 1;
		else
			unique++;
		sprintf(namebatch, "%s\\elv_%04x.bat", TMPDIR, unique);
		bfd = CreateFile(namebatch, GENERIC_WRITE, 0, &inherit,
				CREATE_NEW, FILE_FLAG_SEQUENTIAL_SCAN,
				INVALID_HANDLE_VALUE);
	} while (bfd == INVALID_HANDLE_VALUE
			&& GetLastError() == ERROR_FILE_EXISTS);

	/* Derive stdin and stdout names from the batch name */
	if (willwrite)
		sprintf(namestdin, "%s\\elv_%04x.in", TMPDIR, unique);
	else
		*namestdin = '\0';
	if (willread)
		sprintf(namestdout, "%s\\elv_%04x.out", TMPDIR, unique);
	else
		*namestdout = '\0';

	/* create the batch file */
	WriteFile(bfd, "@echo off\r\n", (DWORD)11, &unique, NULL);
	if (willwrite)
	{
		WriteFile(bfd, "<\"", (DWORD)2, &unique, NULL);
		unique = strlen(namestdin);
		WriteFile(bfd, namestdin, (DWORD)unique, &unique, NULL);
		WriteFile(bfd, "\" ", (DWORD)2, &unique, NULL);
	}
	unique = strlen(cmd);
	WriteFile(bfd, cmd, (DWORD)unique, &unique, NULL);
	if (willread)
	{
		WriteFile(bfd, " >\"", (DWORD)3, &unique, NULL);
		unique = strlen(namestdout);
		WriteFile(bfd, namestdout, (DWORD)unique, &unique, NULL);
		WriteFile(bfd, "\"", (DWORD)1, &unique, NULL);
	}
	WriteFile(bfd, "\r\n", (DWORD)2, &unique, NULL);
	CloseHandle(bfd);

	/* Are we going to write to stdin? */
	if (willwrite)
	{
		/* Yes -- create the stdin file */
		fp = fopen(namestdin, "w");
		if (!fp)
		{
			remove(namebatch);
			return False;
		}
		return True;
	}
	else
	{
		/* No -- run the batch now */
		if (runbatch())
			return True;

		/* if we get here, we couldn't run the batch file */
		DeleteFile(namebatch);
		return False;
	}
}

/* Write the contents of buf to the program's stdin, and return nbytes
 * if successful, or -1 for error.  Note that this text should
 * be subjected to the same kind of transformations as textwrite().
 * In fact, it may use textwrite() internally.
 *
 * For Win32, this is simply a fwrite() call.
 */
int prgwrite(buf, nbytes)
	ElvisCHAR	*buf;	/* buffer, contains text to be written */
	int		nbytes;	/* number of characters in buf */
{
	assert(*namestdin);
	return fwrite(buf, sizeof(char), nbytes, fp);
}

/* Marks the end of writing.  Returns True if all is okay, or False if
 * error.
 *
 * For Win32, this closes the stdin temp file if necessary, runs the batch,
 * and opens the
 * Returns True if the fork was successful, or False if it failed.
 */
ElvisBOOLEAN prggo()
{
	HANDLE	piper, pipew;	/* read & write ends of a pipe */
	HANDLE	handle;		/* temporary variable */

	/* if stdin was redirected, then close stdin file and run the batch */
	if (*namestdin)
	{
		/* close the stdin handle */
		fclose(fp);
		
		/* run the batch program */
		if (!runbatch())
		{
			DeleteFile(namebatch);
			remove(namestdin);
			return False;
		}
	}

	/* if we're supposed to read output, then open stdout temp file */
	fp = fopen(namestdout, "r");
	if (!fp)
	{
		DeleteFile(namebatch);
		return False;
	}

	return True;
}


/* Reads text from the program's stdout, and returns the number of
 * characters read.  At EOF, it returns 0.  Note that this text
 * should be subjected to the same kinds of transformations as
 * textread().
 *
 * For Win32, we use fread() to read from a temp file.
 */
int prgread(buf, nbytes)
	ElvisCHAR	*buf;	/* buffer where text should be placed */
	int		nbytes;	/* maximum number of characters to read */
{
	assert(*namestdout);
	return fread(buf, sizeof(char), nbytes, fp);
}

/* Clean up, and return the program's exit status.  The exit status
 * should be 0 normally.
 *
 * For Win32, this involves closing the pipe, calling WaitForSingleObject()
 * to get the program's exit status, and then deleting the temp file.
 */
int prgclose()
{
	/* close the handle used for reading from stdout */
	if (*namestdout)
		fclose(fp);

	/* delete the temp files, if there were any */
	DeleteFile(namebatch);
	if (*namestdin)
		remove(namestdin);
	if (*namestdout)
		remove(namestdout);

	return status;
}
