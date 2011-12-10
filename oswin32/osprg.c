/* oswin32/osprg.c */


#include <windows.h>
#include <wtypes.h>
#include <io.h>
#define CHAR ElvisCHAR
#include "elvis.h"
#ifdef FEATURE_RCSID
char id_osprg[] = "$Id: osprg.c,v 2.19 2003/10/17 17:41:23 steve Exp $";
#endif


#undef DEBUGGING


#define TMPDIR	(o_directory ? tochar8(o_directory) : ".")

static char	*command;	/* the command to run */
static ELVBOOL	filter;		/* redirecting both stdin and stdout? */
static char	tempfname[MAX_PATH];	/* name of temp file */
static HANDLE	writefd;	/* handle used for writing to program's stdin */
static HANDLE	readfd;		/* handle used for reading program's stdout */
static HANDLE	pid;		/* process ID of program */
static HANDLE	tid;		/* thread ID of program */
static SECURITY_ATTRIBUTES inherit = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
static STARTUPINFO start = {sizeof(STARTUPINFO)};
static UINT	unique;
static char	pipefname[MAX_PATH];	/* name of pipe's temp file */
static BOOL	supports_pipes;	/* can program write to a pipe? */
static BOOL	supports_detach;/* can program run without a console? */
static DWORD	status;		/* exit status of program */


static BOOL classifyprog(char *prog);
static BOOL runcmd(char *cmd, BOOL detached);
static BOOL MaybeCreatePipe(PHANDLE r, PHANDLE w);

#ifdef DEBUGGING
extern void ElvisError(char *where);
static FILE *tracelog;

/* Display an error message */
void ElvisError(where)
	char	*where;
{
	char	*what;
	DWORD	error = GetLastError();
	char	errornum[20];

	/* convert error code to text */
	switch (error & 0xffff)
	{
	  case ERROR_SUCCESS:			what = "SUCCESS";	break;
	  case ERROR_INVALID_FUNCTION:		what = "INVALID_FUNCTION";	break;
	  case ERROR_FILE_NOT_FOUND:		what = "FILE_NOT_FOUND";	break;
	  case ERROR_PATH_NOT_FOUND:		what = "PATH_NOT_FOUND";	break;
	  case ERROR_TOO_MANY_OPEN_FILES:	what = "TOO_MANY_OPEN_FILES";	break;
	  case ERROR_ACCESS_DENIED:		what = "ACCESS_DENIED";	break;
	  case ERROR_INVALID_HANDLE:		what = "INVALID_HANDLE";	break;
	  case ERROR_ARENA_TRASHED:		what = "ARENA_TRASHED";	break;
	  case ERROR_NOT_ENOUGH_MEMORY:		what = "NOT_ENOUGH_MEMORY";	break;
	  case ERROR_INVALID_BLOCK:		what = "INVALID_BLOCK";	break;
	  case ERROR_BAD_ENVIRONMENT:		what = "BAD_ENVIRONMENT";	break;
	  case ERROR_BAD_FORMAT:		what = "BAD_FORMAT";	break;
	  case ERROR_INVALID_ACCESS:		what = "INVALID_ACCESS";	break;
	  case ERROR_INVALID_DATA:		what = "INVALID_DATA";	break;
	  case ERROR_OUTOFMEMORY:		what = "OUTOFMEMORY";	break;
	  case ERROR_INVALID_DRIVE:		what = "INVALID_DRIVE";	break;
	  case ERROR_CURRENT_DIRECTORY:		what = "CURRENT_DIRECTORY";	break;
	  case ERROR_NOT_SAME_DEVICE:		what = "NOT_SAME_DEVICE";	break;
	  case ERROR_NO_MORE_FILES:		what = "NO_MORE_FILES";	break;
	  case ERROR_WRITE_PROTECT:		what = "WRITE_PROTECT";	break;
	  case ERROR_BAD_UNIT:			what = "BAD_UNIT";	break;
	  case ERROR_NOT_READY:			what = "NOT_READY";	break;
	  case ERROR_BAD_COMMAND:		what = "BAD_COMMAND";	break;
	  case ERROR_CRC:			what = "CRC";	break;
	  case ERROR_BAD_LENGTH:		what = "BAD_LENGTH";	break;
	  case ERROR_SEEK:			what = "SEEK";	break;
	  case ERROR_NOT_DOS_DISK:		what = "NOT_DOS_DISK";	break;
	  case ERROR_SECTOR_NOT_FOUND:		what = "SECTOR_NOT_FOUND";	break;
	  case ERROR_OUT_OF_PAPER:		what = "OUT_OF_PAPER";	break;
	  case ERROR_WRITE_FAULT:		what = "WRITE_FAULT";	break;
	  case ERROR_READ_FAULT:		what = "READ_FAULT";	break;
	  case ERROR_GEN_FAILURE:		what = "GEN_FAILURE";	break;
	  case ERROR_SHARING_VIOLATION:		what = "SHARING_VIOLATION";	break;
	  case ERROR_LOCK_VIOLATION:		what = "LOCK_VIOLATION";	break;
	  case ERROR_WRONG_DISK:		what = "WRONG_DISK";	break;
	  case ERROR_SHARING_BUFFER_EXCEEDED:	what = "SHARING_BUFFER_EXCEEDED";	break;
	  case ERROR_HANDLE_EOF:		what = "HANDLE_EOF";	break;
	  case ERROR_HANDLE_DISK_FULL:		what = "HANDLE_DISK_FULL";	break;
	  case ERROR_NOT_SUPPORTED:		what = "NOT_SUPPORTED";	break;
	  case ERROR_REM_NOT_LIST:		what = "REM_NOT_LIST";	break;
	  case ERROR_DUP_NAME:			what = "DUP_NAME";	break;
	  case ERROR_BAD_NETPATH:		what = "BAD_NETPATH";	break;
	  case ERROR_NETWORK_BUSY:		what = "NETWORK_BUSY";	break;
	  case ERROR_DEV_NOT_EXIST:		what = "DEV_NOT_EXIST";	break;
	  case ERROR_TOO_MANY_CMDS:		what = "TOO_MANY_CMDS";	break;
	  case ERROR_ADAP_HDW_ERR:		what = "ADAP_HDW_ERR";	break;
	  case ERROR_BAD_NET_RESP:		what = "BAD_NET_RESP";	break;
	  case ERROR_UNEXP_NET_ERR:		what = "UNEXP_NET_ERR";	break;
	  case ERROR_BAD_REM_ADAP:		what = "BAD_REM_ADAP";	break;
	  case ERROR_PRINTQ_FULL:		what = "PRINTQ_FULL";	break;
	  case ERROR_NO_SPOOL_SPACE:		what = "NO_SPOOL_SPACE";	break;
	  case ERROR_PRINT_CANCELLED:		what = "PRINT_CANCELLED";	break;
	  case ERROR_NETNAME_DELETED:		what = "NETNAME_DELETED";	break;
	  case ERROR_NETWORK_ACCESS_DENIED:	what = "NETWORK_ACCESS_DENIED";	break;
	  case ERROR_BAD_DEV_TYPE:		what = "BAD_DEV_TYPE";	break;
	  case ERROR_BAD_NET_NAME:		what = "BAD_NET_NAME";	break;
	  case ERROR_TOO_MANY_NAMES:		what = "TOO_MANY_NAMES";	break;
	  case ERROR_TOO_MANY_SESS:		what = "TOO_MANY_SESS";	break;
	  case ERROR_SHARING_PAUSED:		what = "SHARING_PAUSED";	break;
	  case ERROR_REQ_NOT_ACCEP:		what = "REQ_NOT_ACCEP";	break;
	  case ERROR_REDIR_PAUSED:		what = "REDIR_PAUSED";	break;
	  case ERROR_FILE_EXISTS:		what = "FILE_EXISTS";	break;
	  case ERROR_CANNOT_MAKE:		what = "CANNOT_MAKE";	break;
	  case ERROR_FAIL_I24:			what = "FAIL_I24";	break;
	  case ERROR_OUT_OF_STRUCTURES:		what = "OUT_OF_STRUCTURES";	break;
	  case ERROR_ALREADY_ASSIGNED:		what = "ALREADY_ASSIGNED";	break;
	  case ERROR_INVALID_PASSWORD:		what = "INVALID_PASSWORD";	break;
	  case ERROR_INVALID_PARAMETER:		what = "INVALID_PARAMETER";	break;
	  case ERROR_NET_WRITE_FAULT:		what = "NET_WRITE_FAULT";	break;
	  case ERROR_NO_PROC_SLOTS:		what = "NO_PROC_SLOTS";	break;
	  case ERROR_TOO_MANY_SEMAPHORES:	what = "TOO_MANY_SEMAPHORES";	break;
	  case ERROR_EXCL_SEM_ALREADY_OWNED:	what = "EXCL_SEM_ALREADY_OWNED";	break;
	  case ERROR_SEM_IS_SET:		what = "SEM_IS_SET";	break;
	  case ERROR_TOO_MANY_SEM_REQUESTS:	what = "TOO_MANY_SEM_REQUESTS";	break;
	  case ERROR_INVALID_AT_INTERRUPT_TIME:	what = "INVALID_AT_INTERRUPT_TIME";	break;
	  case ERROR_SEM_OWNER_DIED:		what = "SEM_OWNER_DIED";	break;
	  case ERROR_SEM_USER_LIMIT:		what = "SEM_USER_LIMIT";	break;
	  case ERROR_DISK_CHANGE:		what = "DISK_CHANGE";	break;
	  case ERROR_DRIVE_LOCKED:		what = "DRIVE_LOCKED";	break;
	  case ERROR_BROKEN_PIPE:		what = "BROKEN_PIPE";	break;
	  case ERROR_OPEN_FAILED:		what = "OPEN_FAILED";	break;
	  case ERROR_BUFFER_OVERFLOW:		what = "BUFFER_OVERFLOW";	break;
	  case ERROR_DISK_FULL:			what = "DISK_FULL";	break;
	  case ERROR_NO_MORE_SEARCH_HANDLES:	what = "NO_MORE_SEARCH_HANDLES";	break;
	  case ERROR_INVALID_TARGET_HANDLE:	what = "INVALID_TARGET_HANDLE";	break;
	  case ERROR_INVALID_CATEGORY:		what = "INVALID_CATEGORY";	break;
	  case ERROR_INVALID_VERIFY_SWITCH:	what = "INVALID_VERIFY_SWITCH";	break;
	  case ERROR_BAD_DRIVER_LEVEL:		what = "BAD_DRIVER_LEVEL";	break;
	  case ERROR_CALL_NOT_IMPLEMENTED:	what = "CALL_NOT_IMPLEMENTED";	break;
	  case ERROR_SEM_TIMEOUT:		what = "SEM_TIMEOUT";	break;
	  case ERROR_INSUFFICIENT_BUFFER:	what = "INSUFFICIENT_BUFFER";	break;
	  case ERROR_INVALID_NAME:		what = "INVALID_NAME";	break;
	  case ERROR_INVALID_LEVEL:		what = "INVALID_LEVEL";	break;
	  case ERROR_NO_VOLUME_LABEL:		what = "NO_VOLUME_LABEL";	break;
	  case ERROR_MOD_NOT_FOUND:		what = "MOD_NOT_FOUND";	break;
	  case ERROR_PROC_NOT_FOUND:		what = "PROC_NOT_FOUND";	break;
	  case ERROR_WAIT_NO_CHILDREN:		what = "WAIT_NO_CHILDREN";	break;
	  case ERROR_CHILD_NOT_COMPLETE:	what = "CHILD_NOT_COMPLETE";	break;
	  case ERROR_DIRECT_ACCESS_HANDLE:	what = "DIRECT_ACCESS_HANDLE";	break;
	  case ERROR_NEGATIVE_SEEK:		what = "NEGATIVE_SEEK";	break;
	  case ERROR_SEEK_ON_DEVICE:		what = "SEEK_ON_DEVICE";	break;
	  case ERROR_IS_JOIN_TARGET:		what = "IS_JOIN_TARGET";	break;
	  case ERROR_IS_JOINED:			what = "IS_JOINED";	break;
	  case ERROR_IS_SUBSTED:		what = "IS_SUBSTED";	break;
	  case ERROR_NOT_JOINED:		what = "NOT_JOINED";	break;
	  case ERROR_NOT_SUBSTED:		what = "NOT_SUBSTED";	break;
	  case ERROR_JOIN_TO_JOIN:		what = "JOIN_TO_JOIN";	break;
	  case ERROR_SUBST_TO_SUBST:		what = "SUBST_TO_SUBST";	break;
	  case ERROR_JOIN_TO_SUBST:		what = "JOIN_TO_SUBST";	break;
	  case ERROR_SUBST_TO_JOIN:		what = "SUBST_TO_JOIN";	break;
	  case ERROR_BUSY_DRIVE:		what = "BUSY_DRIVE";	break;
	  case ERROR_SAME_DRIVE:		what = "SAME_DRIVE";	break;
	  case ERROR_DIR_NOT_ROOT:		what = "DIR_NOT_ROOT";	break;
	  case ERROR_DIR_NOT_EMPTY:		what = "DIR_NOT_EMPTY";	break;
	  case ERROR_IS_SUBST_PATH:		what = "IS_SUBST_PATH";	break;
	  case ERROR_IS_JOIN_PATH:		what = "IS_JOIN_PATH";	break;
	  case ERROR_PATH_BUSY:			what = "PATH_BUSY";	break;
	  case ERROR_IS_SUBST_TARGET:		what = "IS_SUBST_TARGET";	break;
	  case ERROR_SYSTEM_TRACE:		what = "SYSTEM_TRACE";	break;
	  case ERROR_INVALID_EVENT_COUNT:	what = "INVALID_EVENT_COUNT";	break;
	  case ERROR_TOO_MANY_MUXWAITERS:	what = "TOO_MANY_MUXWAITERS";	break;
	  case ERROR_INVALID_LIST_FORMAT:	what = "INVALID_LIST_FORMAT";	break;
	  case ERROR_LABEL_TOO_LONG:		what = "LABEL_TOO_LONG";	break;
	  case ERROR_TOO_MANY_TCBS:		what = "TOO_MANY_TCBS";	break;
	  case ERROR_SIGNAL_REFUSED:		what = "SIGNAL_REFUSED";	break;
	  case ERROR_DISCARDED:			what = "DISCARDED";	break;
	  case ERROR_NOT_LOCKED:		what = "NOT_LOCKED";	break;
	  case ERROR_BAD_THREADID_ADDR:		what = "BAD_THREADID_ADDR";	break;
	  case ERROR_BAD_ARGUMENTS:		what = "BAD_ARGUMENTS";	break;
	  case ERROR_BAD_PATHNAME:		what = "BAD_PATHNAME";	break;
	  case ERROR_SIGNAL_PENDING:		what = "SIGNAL_PENDING";	break;
	  case ERROR_MAX_THRDS_REACHED:		what = "MAX_THRDS_REACHED";	break;
	  case ERROR_LOCK_FAILED:		what = "LOCK_FAILED";	break;
	  case ERROR_BUSY:			what = "BUSY";	break;
	  case ERROR_CANCEL_VIOLATION:		what = "CANCEL_VIOLATION";	break;
	  case ERROR_ATOMIC_LOCKS_NOT_SUPPORTED:what = "ATOMIC_LOCKS_NOT_SUPPORTED";	break;
	  case ERROR_INVALID_SEGMENT_NUMBER:	what = "INVALID_SEGMENT_NUMBER";	break;
	  case ERROR_INVALID_ORDINAL:		what = "INVALID_ORDINAL";	break;
	  case ERROR_ALREADY_EXISTS:		what = "ALREADY_EXISTS";	break;
	  case ERROR_INVALID_FLAG_NUMBER:	what = "INVALID_FLAG_NUMBER";	break;
	  case ERROR_SEM_NOT_FOUND:		what = "SEM_NOT_FOUND";	break;
	  case ERROR_INVALID_STARTING_CODESEG:	what = "INVALID_STARTING_CODESEG";	break;
	  case ERROR_INVALID_STACKSEG:		what = "INVALID_STACKSEG";	break;
	  case ERROR_INVALID_MODULETYPE:	what = "INVALID_MODULETYPE";	break;
	  case ERROR_INVALID_EXE_SIGNATURE:	what = "INVALID_EXE_SIGNATURE";	break;
	  case ERROR_EXE_MARKED_INVALID:	what = "EXE_MARKED_INVALID";	break;
	  case ERROR_BAD_EXE_FORMAT:		what = "BAD_EXE_FORMAT";	break;
	  case ERROR_ITERATED_DATA_EXCEEDS_64k:	what = "ITERATED_DATA_EXCEEDS_64k";	break;
	  case ERROR_INVALID_MINALLOCSIZE:	what = "INVALID_MINALLOCSIZE";	break;
	  case ERROR_DYNLINK_FROM_INVALID_RING:	what = "DYNLINK_FROM_INVALID_RING";	break;
	  case ERROR_IOPL_NOT_ENABLED:		what = "IOPL_NOT_ENABLED";	break;
	  case ERROR_INVALID_SEGDPL:		what = "INVALID_SEGDPL";	break;
	  case ERROR_AUTODATASEG_EXCEEDS_64k:	what = "AUTODATASEG_EXCEEDS_64k";	break;
	  case ERROR_RING2SEG_MUST_BE_MOVABLE:	what = "RING2SEG_MUST_BE_MOVABLE";	break;
	  case ERROR_RELOC_CHAIN_XEEDS_SEGLIM:	what = "RELOC_CHAIN_XEEDS_SEGLIM";	break;
	  case ERROR_INFLOOP_IN_RELOC_CHAIN:	what = "INFLOOP_IN_RELOC_CHAIN";	break;
	  case ERROR_ENVVAR_NOT_FOUND:		what = "ENVVAR_NOT_FOUND";	break;
	  case ERROR_NO_SIGNAL_SENT:		what = "NO_SIGNAL_SENT";	break;
	  case ERROR_FILENAME_EXCED_RANGE:	what = "FILENAME_EXCED_RANGE";	break;
	  case ERROR_RING2_STACK_IN_USE:	what = "RING2_STACK_IN_USE";	break;
	  case ERROR_META_EXPANSION_TOO_LONG:	what = "META_EXPANSION_TOO_LONG";	break;
	  case ERROR_INVALID_SIGNAL_NUMBER:	what = "INVALID_SIGNAL_NUMBER";	break;
	  case ERROR_THREAD_1_INACTIVE:		what = "THREAD_1_INACTIVE";	break;
	  case ERROR_LOCKED:			what = "LOCKED";	break;
	  case ERROR_TOO_MANY_MODULES:		what = "TOO_MANY_MODULES";	break;
	  case ERROR_NESTING_NOT_ALLOWED:	what = "NESTING_NOT_ALLOWED";	break;
	  case ERROR_BAD_PIPE:			what = "BAD_PIPE";	break;
	  case ERROR_PIPE_BUSY:			what = "PIPE_BUSY";	break;
	  case ERROR_NO_DATA:			what = "NO_DATA";	break;
	  case ERROR_PIPE_NOT_CONNECTED:	what = "PIPE_NOT_CONNECTED";	break;
	  case ERROR_MORE_DATA:			what = "MORE_DATA";	break;
	  case ERROR_VC_DISCONNECTED:		what = "VC_DISCONNECTED";	break;
	  case ERROR_INVALID_EA_NAME:		what = "INVALID_EA_NAME";	break;
	  case ERROR_EA_LIST_INCONSISTENT:	what = "EA_LIST_INCONSISTENT";	break;
	  case ERROR_NO_MORE_ITEMS:		what = "NO_MORE_ITEMS";	break;
	  case ERROR_CANNOT_COPY:		what = "CANNOT_COPY";	break;
	  case ERROR_DIRECTORY:			what = "DIRECTORY";	break;
	  case ERROR_EAS_DIDNT_FIT:		what = "EAS_DIDNT_FIT";	break;
	  case ERROR_EA_FILE_CORRUPT:		what = "EA_FILE_CORRUPT";	break;
	  case ERROR_EA_TABLE_FULL:		what = "EA_TABLE_FULL";	break;
	  case ERROR_INVALID_EA_HANDLE:		what = "INVALID_EA_HANDLE";	break;
	  case ERROR_EAS_NOT_SUPPORTED:		what = "EAS_NOT_SUPPORTED";	break;
	  case ERROR_NOT_OWNER:			what = "NOT_OWNER";	break;
	  case ERROR_TOO_MANY_POSTS:		what = "TOO_MANY_POSTS";	break;
	  case ERROR_PARTIAL_COPY:		what = "PARTIAL_COPY";	break;
	  case ERROR_MR_MID_NOT_FOUND:		what = "MR_MID_NOT_FOUND";	break;
	  case ERROR_INVALID_ADDRESS:		what = "INVALID_ADDRESS";	break;
	  case ERROR_ARITHMETIC_OVERFLOW:	what = "ARITHMETIC_OVERFLOW";	break;
	  case ERROR_PIPE_CONNECTED:		what = "PIPE_CONNECTED";	break;
	  case ERROR_PIPE_LISTENING:		what = "PIPE_LISTENING";	break;
	  case ERROR_EA_ACCESS_DENIED:		what = "EA_ACCESS_DENIED";	break;
	  case ERROR_OPERATION_ABORTED:		what = "OPERATION_ABORTED";	break;
	  case ERROR_IO_INCOMPLETE:		what = "IO_INCOMPLETE";	break;
	  case ERROR_IO_PENDING:		what = "IO_PENDING";	break;
	  case ERROR_NOACCESS:			what = "NOACCESS";	break;
	  case ERROR_SWAPERROR:			what = "SWAPERROR";	break;
	  case ERROR_STACK_OVERFLOW:		what = "STACK_OVERFLOW";	break;
	  case ERROR_INVALID_MESSAGE:		what = "INVALID_MESSAGE";	break;
	  case ERROR_CAN_NOT_COMPLETE:		what = "CAN_NOT_COMPLETE";	break;
	  case ERROR_INVALID_FLAGS:		what = "INVALID_FLAGS";	break;
	  case ERROR_UNRECOGNIZED_VOLUME:	what = "UNRECOGNIZED_VOLUME";	break;
	  case ERROR_FILE_INVALID:		what = "FILE_INVALID";	break;
	  case ERROR_FULLSCREEN_MODE:		what = "FULLSCREEN_MODE";	break;
	  case ERROR_NO_TOKEN:			what = "NO_TOKEN";	break;
	  case ERROR_BADDB:			what = "BADDB";	break;
	  case ERROR_BADKEY:			what = "BADKEY";	break;
	  case ERROR_CANTOPEN:			what = "CANTOPEN";	break;
	  case ERROR_CANTREAD:			what = "CANTREAD";	break;
	  case ERROR_CANTWRITE:			what = "CANTWRITE";	break;
	  case ERROR_REGISTRY_RECOVERED:	what = "REGISTRY_RECOVERED";	break;
	  case ERROR_REGISTRY_CORRUPT:		what = "REGISTRY_CORRUPT";	break;
	  case ERROR_REGISTRY_IO_FAILED:	what = "REGISTRY_IO_FAILED";	break;
	  case ERROR_NOT_REGISTRY_FILE:		what = "NOT_REGISTRY_FILE";	break;
	  case ERROR_KEY_DELETED:		what = "KEY_DELETED";	break;
	  case ERROR_NO_LOG_SPACE:		what = "NO_LOG_SPACE";	break;
	  case ERROR_KEY_HAS_CHILDREN:		what = "KEY_HAS_CHILDREN";	break;
	  case ERROR_CHILD_MUST_BE_VOLATILE:	what = "CHILD_MUST_BE_VOLATILE";	break;
	  case ERROR_NOTIFY_ENUM_DIR:		what = "NOTIFY_ENUM_DIR";	break;
	  case ERROR_DEPENDENT_SERVICES_RUNNING:what = "DEPENDENT_SERVICES_RUNNING";	break;
	  case ERROR_INVALID_SERVICE_CONTROL:	what = "INVALID_SERVICE_CONTROL";	break;
	  case ERROR_SERVICE_REQUEST_TIMEOUT:	what = "SERVICE_REQUEST_TIMEOUT";	break;
	  case ERROR_SERVICE_NO_THREAD:		what = "SERVICE_NO_THREAD";	break;
	  case ERROR_SERVICE_DATABASE_LOCKED:	what = "SERVICE_DATABASE_LOCKED";	break;
	  case ERROR_SERVICE_ALREADY_RUNNING:	what = "SERVICE_ALREADY_RUNNING";	break;
	  case ERROR_INVALID_SERVICE_ACCOUNT:	what = "INVALID_SERVICE_ACCOUNT";	break;
	  case ERROR_SERVICE_DISABLED:		what = "SERVICE_DISABLED";	break;
	  case ERROR_CIRCULAR_DEPENDENCY:	what = "CIRCULAR_DEPENDENCY";	break;
	  case ERROR_SERVICE_DOES_NOT_EXIST:	what = "SERVICE_DOES_NOT_EXIST";	break;
	  case ERROR_SERVICE_CANNOT_ACCEPT_CTRL:what = "SERVICE_CANNOT_ACCEPT_CTRL";	break;
	  case ERROR_SERVICE_NOT_ACTIVE:	what = "SERVICE_NOT_ACTIVE";	break;
	  case ERROR_FAILED_SERVICE_CONTROLLER_CONNECT:	what = "FAILED_SERVICE_CONTROLLER_CONNECT";	break;
	  case ERROR_EXCEPTION_IN_SERVICE:	what = "EXCEPTION_IN_SERVICE";	break;
	  case ERROR_DATABASE_DOES_NOT_EXIST:	what = "DATABASE_DOES_NOT_EXIST";	break;
	  case ERROR_SERVICE_SPECIFIC_ERROR:	what = "SERVICE_SPECIFIC_ERROR";	break;
	  case ERROR_PROCESS_ABORTED:		what = "PROCESS_ABORTED";	break;
	  case ERROR_SERVICE_DEPENDENCY_FAIL:	what = "SERVICE_DEPENDENCY_FAIL";	break;
	  case ERROR_SERVICE_LOGON_FAILED:	what = "SERVICE_LOGON_FAILED";	break;
	  case ERROR_SERVICE_START_HANG:	what = "SERVICE_START_HANG";	break;
	  case ERROR_INVALID_SERVICE_LOCK:	what = "INVALID_SERVICE_LOCK";	break;
	  case ERROR_SERVICE_MARKED_FOR_DELETE:	what = "SERVICE_MARKED_FOR_DELETE";	break;
	  case ERROR_SERVICE_EXISTS:		what = "SERVICE_EXISTS";	break;
	  case ERROR_ALREADY_RUNNING_LKG:	what = "ALREADY_RUNNING_LKG";	break;
	  case ERROR_SERVICE_DEPENDENCY_DELETED:what = "SERVICE_DEPENDENCY_DELETED";	break;
	  case ERROR_BOOT_ALREADY_ACCEPTED:	what = "BOOT_ALREADY_ACCEPTED";	break;
	  case ERROR_SERVICE_NEVER_STARTED:	what = "SERVICE_NEVER_STARTED";	break;
	  case ERROR_DUPLICATE_SERVICE_NAME:	what = "DUPLICATE_SERVICE_NAME";	break;
	  case ERROR_END_OF_MEDIA:		what = "END_OF_MEDIA";	break;
	  case ERROR_FILEMARK_DETECTED:		what = "FILEMARK_DETECTED";	break;
	  case ERROR_BEGINNING_OF_MEDIA:	what = "BEGINNING_OF_MEDIA";	break;
	  case ERROR_SETMARK_DETECTED:		what = "SETMARK_DETECTED";	break;
	  case ERROR_NO_DATA_DETECTED:		what = "NO_DATA_DETECTED";	break;
	  case ERROR_PARTITION_FAILURE:		what = "PARTITION_FAILURE";	break;
	  case ERROR_INVALID_BLOCK_LENGTH:	what = "INVALID_BLOCK_LENGTH";	break;
	  case ERROR_DEVICE_NOT_PARTITIONED:	what = "DEVICE_NOT_PARTITIONED";	break;
	  case ERROR_UNABLE_TO_LOCK_MEDIA:	what = "UNABLE_TO_LOCK_MEDIA";	break;
	  case ERROR_UNABLE_TO_UNLOAD_MEDIA:	what = "UNABLE_TO_UNLOAD_MEDIA";	break;
	  case ERROR_MEDIA_CHANGED:		what = "MEDIA_CHANGED";	break;
	  case ERROR_BUS_RESET:			what = "BUS_RESET";	break;
	  case ERROR_NO_MEDIA_IN_DRIVE:		what = "NO_MEDIA_IN_DRIVE";	break;
	  case ERROR_NO_UNICODE_TRANSLATION:	what = "NO_UNICODE_TRANSLATION";	break;
	  case ERROR_DLL_INIT_FAILED:		what = "DLL_INIT_FAILED";	break;
	  case ERROR_SHUTDOWN_IN_PROGRESS:	what = "SHUTDOWN_IN_PROGRESS";	break;
	  case ERROR_NO_SHUTDOWN_IN_PROGRESS:	what = "NO_SHUTDOWN_IN_PROGRESS";	break;
	  case ERROR_IO_DEVICE:			what = "IO_DEVICE";	break;
	  case ERROR_SERIAL_NO_DEVICE:		what = "SERIAL_NO_DEVICE";	break;
	  case ERROR_IRQ_BUSY:			what = "IRQ_BUSY";	break;
	  case ERROR_MORE_WRITES:		what = "MORE_WRITES";	break;
	  case ERROR_COUNTER_TIMEOUT:		what = "COUNTER_TIMEOUT";	break;
	  case ERROR_FLOPPY_ID_MARK_NOT_FOUND:	what = "FLOPPY_ID_MARK_NOT_FOUND";	break;
	  case ERROR_FLOPPY_WRONG_CYLINDER:	what = "FLOPPY_WRONG_CYLINDER";	break;
	  case ERROR_FLOPPY_UNKNOWN_ERROR:	what = "FLOPPY_UNKNOWN_ERROR";	break;
	  case ERROR_FLOPPY_BAD_REGISTERS:	what = "FLOPPY_BAD_REGISTERS";	break;
	  case ERROR_DISK_RECALIBRATE_FAILED:	what = "DISK_RECALIBRATE_FAILED";	break;
	  case ERROR_DISK_OPERATION_FAILED:	what = "DISK_OPERATION_FAILED";	break;
	  case ERROR_DISK_RESET_FAILED:		what = "DISK_RESET_FAILED";	break;
	  case ERROR_EOM_OVERFLOW:		what = "EOM_OVERFLOW";	break;
	  case ERROR_NOT_ENOUGH_SERVER_MEMORY:	what = "NOT_ENOUGH_SERVER_MEMORY";	break;
	  case ERROR_POSSIBLE_DEADLOCK:		what = "POSSIBLE_DEADLOCK";	break;
	  case ERROR_MAPPED_ALIGNMENT:		what = "MAPPED_ALIGNMENT";	break;
	  case ERROR_BAD_USERNAME:		what = "BAD_USERNAME";	break;
	  case ERROR_NOT_CONNECTED:		what = "NOT_CONNECTED";	break;
	  case ERROR_OPEN_FILES:		what = "OPEN_FILES";	break;
	  case ERROR_ACTIVE_CONNECTIONS:	what = "ACTIVE_CONNECTIONS";	break;
	  case ERROR_DEVICE_IN_USE:		what = "DEVICE_IN_USE";	break;
	  case ERROR_BAD_DEVICE:		what = "BAD_DEVICE";	break;
	  case ERROR_CONNECTION_UNAVAIL:	what = "CONNECTION_UNAVAIL";	break;
	  case ERROR_DEVICE_ALREADY_REMEMBERED:	what = "DEVICE_ALREADY_REMEMBERED";	break;
	  case ERROR_NO_NET_OR_BAD_PATH:	what = "NO_NET_OR_BAD_PATH";	break;
	  case ERROR_BAD_PROVIDER:		what = "BAD_PROVIDER";	break;
	  case ERROR_CANNOT_OPEN_PROFILE:	what = "CANNOT_OPEN_PROFILE";	break;
	  case ERROR_BAD_PROFILE:		what = "BAD_PROFILE";	break;
	  case ERROR_NOT_CONTAINER:		what = "NOT_CONTAINER";	break;
	  case ERROR_EXTENDED_ERROR:		what = "EXTENDED_ERROR";	break;
	  case ERROR_INVALID_GROUPNAME:		what = "INVALID_GROUPNAME";	break;
	  case ERROR_INVALID_COMPUTERNAME:	what = "INVALID_COMPUTERNAME";	break;
	  case ERROR_INVALID_EVENTNAME:		what = "INVALID_EVENTNAME";	break;
	  case ERROR_INVALID_DOMAINNAME:	what = "INVALID_DOMAINNAME";	break;
	  case ERROR_INVALID_SERVICENAME:	what = "INVALID_SERVICENAME";	break;
	  case ERROR_INVALID_NETNAME:		what = "INVALID_NETNAME";	break;
	  case ERROR_INVALID_SHARENAME:		what = "INVALID_SHARENAME";	break;
	  case ERROR_INVALID_PASSWORDNAME:	what = "INVALID_PASSWORDNAME";	break;
	  case ERROR_INVALID_MESSAGENAME:	what = "INVALID_MESSAGENAME";	break;
	  case ERROR_INVALID_MESSAGEDEST:	what = "INVALID_MESSAGEDEST";	break;
	  case ERROR_SESSION_CREDENTIAL_CONFLICT:	what = "SESSION_CREDENTIAL_CONFLICT";	break;
	  case ERROR_REMOTE_SESSION_LIMIT_EXCEEDED:	what = "REMOTE_SESSION_LIMIT_EXCEEDED";	break;
	  case ERROR_DUP_DOMAINNAME:		what = "DUP_DOMAINNAME";	break;
	  case ERROR_NO_NETWORK:		what = "NO_NETWORK";	break;
	  case ERROR_CANCELLED:			what = "CANCELLED";	break;
	  case ERROR_USER_MAPPED_FILE:		what = "USER_MAPPED_FILE";	break;
	  case ERROR_CONNECTION_REFUSED:	what = "CONNECTION_REFUSED";	break;
	  case ERROR_GRACEFUL_DISCONNECT:	what = "GRACEFUL_DISCONNECT";	break;
	  case ERROR_ADDRESS_ALREADY_ASSOCIATED:what = "ADDRESS_ALREADY_ASSOCIATED";	break;
	  case ERROR_ADDRESS_NOT_ASSOCIATED:	what = "ADDRESS_NOT_ASSOCIATED";	break;
	  case ERROR_CONNECTION_INVALID:	what = "CONNECTION_INVALID";	break;
	  case ERROR_CONNECTION_ACTIVE:		what = "CONNECTION_ACTIVE";	break;
	  case ERROR_NETWORK_UNREACHABLE:	what = "NETWORK_UNREACHABLE";	break;
	  case ERROR_HOST_UNREACHABLE:		what = "HOST_UNREACHABLE";	break;
	  case ERROR_PROTOCOL_UNREACHABLE:	what = "PROTOCOL_UNREACHABLE";	break;
	  case ERROR_PORT_UNREACHABLE:		what = "PORT_UNREACHABLE";	break;
	  case ERROR_REQUEST_ABORTED:		what = "REQUEST_ABORTED";	break;
	  case ERROR_CONNECTION_ABORTED:	what = "CONNECTION_ABORTED";	break;
	  case ERROR_RETRY:			what = "RETRY";	break;
	  case ERROR_CONNECTION_COUNT_LIMIT:	what = "CONNECTION_COUNT_LIMIT";	break;
	  case ERROR_LOGIN_TIME_RESTRICTION:	what = "LOGIN_TIME_RESTRICTION";	break;
	  case ERROR_LOGIN_WKSTA_RESTRICTION:	what = "LOGIN_WKSTA_RESTRICTION";	break;
	  case ERROR_INCORRECT_ADDRESS:		what = "INCORRECT_ADDRESS";	break;
	  case ERROR_ALREADY_REGISTERED:	what = "ALREADY_REGISTERED";	break;
	  case ERROR_SERVICE_NOT_FOUND:		what = "SERVICE_NOT_FOUND";	break;
	  case ERROR_NOT_ALL_ASSIGNED:		what = "NOT_ALL_ASSIGNED";	break;
	  case ERROR_SOME_NOT_MAPPED:		what = "SOME_NOT_MAPPED";	break;
	  case ERROR_NO_QUOTAS_FOR_ACCOUNT:	what = "NO_QUOTAS_FOR_ACCOUNT";	break;
	  case ERROR_LOCAL_USER_SESSION_KEY:	what = "LOCAL_USER_SESSION_KEY";	break;
	  case ERROR_NULL_LM_PASSWORD:		what = "NULL_LM_PASSWORD";	break;
	  case ERROR_UNKNOWN_REVISION:		what = "UNKNOWN_REVISION";	break;
	  case ERROR_REVISION_MISMATCH:		what = "REVISION_MISMATCH";	break;
	  case ERROR_INVALID_OWNER:		what = "INVALID_OWNER";	break;
	  case ERROR_INVALID_PRIMARY_GROUP:	what = "INVALID_PRIMARY_GROUP";	break;
	  case ERROR_NO_IMPERSONATION_TOKEN:	what = "NO_IMPERSONATION_TOKEN";	break;
	  case ERROR_CANT_DISABLE_MANDATORY:	what = "CANT_DISABLE_MANDATORY";	break;
	  case ERROR_NO_LOGON_SERVERS:		what = "NO_LOGON_SERVERS";	break;
	  case ERROR_NO_SUCH_LOGON_SESSION:	what = "NO_SUCH_LOGON_SESSION";	break;
	  case ERROR_NO_SUCH_PRIVILEGE:		what = "NO_SUCH_PRIVILEGE";	break;
	  case ERROR_PRIVILEGE_NOT_HELD:	what = "PRIVILEGE_NOT_HELD";	break;
	  case ERROR_INVALID_ACCOUNT_NAME:	what = "INVALID_ACCOUNT_NAME";	break;
	  case ERROR_USER_EXISTS:		what = "USER_EXISTS";	break;
	  case ERROR_NO_SUCH_USER:		what = "NO_SUCH_USER";	break;
	  case ERROR_GROUP_EXISTS:		what = "GROUP_EXISTS";	break;
	  case ERROR_NO_SUCH_GROUP:		what = "NO_SUCH_GROUP";	break;
	  case ERROR_MEMBER_IN_GROUP:		what = "MEMBER_IN_GROUP";	break;
	  case ERROR_MEMBER_NOT_IN_GROUP:	what = "MEMBER_NOT_IN_GROUP";	break;
	  case ERROR_LAST_ADMIN:		what = "LAST_ADMIN";	break;
	  case ERROR_WRONG_PASSWORD:		what = "WRONG_PASSWORD";	break;
	  case ERROR_ILL_FORMED_PASSWORD:	what = "ILL_FORMED_PASSWORD";	break;
	  case ERROR_PASSWORD_RESTRICTION:	what = "PASSWORD_RESTRICTION";	break;
	  case ERROR_LOGON_FAILURE:		what = "LOGON_FAILURE";	break;
	  case ERROR_ACCOUNT_RESTRICTION:	what = "ACCOUNT_RESTRICTION";	break;
	  case ERROR_INVALID_LOGON_HOURS:	what = "INVALID_LOGON_HOURS";	break;
	  case ERROR_INVALID_WORKSTATION:	what = "INVALID_WORKSTATION";	break;
	  case ERROR_PASSWORD_EXPIRED:		what = "PASSWORD_EXPIRED";	break;
	  case ERROR_ACCOUNT_DISABLED:		what = "ACCOUNT_DISABLED";	break;
	  case ERROR_NONE_MAPPED:		what = "NONE_MAPPED";	break;
	  case ERROR_TOO_MANY_LUIDS_REQUESTED:	what = "TOO_MANY_LUIDS_REQUESTED";	break;
	  case ERROR_LUIDS_EXHAUSTED:		what = "LUIDS_EXHAUSTED";	break;
	  case ERROR_INVALID_SUB_AUTHORITY:	what = "INVALID_SUB_AUTHORITY";	break;
	  case ERROR_INVALID_ACL:		what = "INVALID_ACL";	break;
	  case ERROR_INVALID_SID:		what = "INVALID_SID";	break;
	  case ERROR_INVALID_SECURITY_DESCR:	what = "INVALID_SECURITY_DESCR";	break;
	  case ERROR_BAD_INHERITANCE_ACL:	what = "BAD_INHERITANCE_ACL";	break;
	  case ERROR_SERVER_DISABLED:		what = "SERVER_DISABLED";	break;
	  case ERROR_SERVER_NOT_DISABLED:	what = "SERVER_NOT_DISABLED";	break;
	  case ERROR_INVALID_ID_AUTHORITY:	what = "INVALID_ID_AUTHORITY";	break;
	  case ERROR_ALLOTTED_SPACE_EXCEEDED:	what = "ALLOTTED_SPACE_EXCEEDED";	break;
	  case ERROR_INVALID_GROUP_ATTRIBUTES:	what = "INVALID_GROUP_ATTRIBUTES";	break;
	  case ERROR_BAD_IMPERSONATION_LEVEL:	what = "BAD_IMPERSONATION_LEVEL";	break;
	  case ERROR_CANT_OPEN_ANONYMOUS:	what = "CANT_OPEN_ANONYMOUS";	break;
	  case ERROR_BAD_VALIDATION_CLASS:	what = "BAD_VALIDATION_CLASS";	break;
	  case ERROR_BAD_TOKEN_TYPE:		what = "BAD_TOKEN_TYPE";	break;
	  case ERROR_NO_SECURITY_ON_OBJECT:	what = "NO_SECURITY_ON_OBJECT";	break;
	  case ERROR_CANT_ACCESS_DOMAIN_INFO:	what = "CANT_ACCESS_DOMAIN_INFO";	break;
	  case ERROR_INVALID_SERVER_STATE:	what = "INVALID_SERVER_STATE";	break;
	  case ERROR_INVALID_DOMAIN_STATE:	what = "INVALID_DOMAIN_STATE";	break;
	  case ERROR_INVALID_DOMAIN_ROLE:	what = "INVALID_DOMAIN_ROLE";	break;
	  case ERROR_NO_SUCH_DOMAIN:		what = "NO_SUCH_DOMAIN";	break;
	  case ERROR_DOMAIN_EXISTS:		what = "DOMAIN_EXISTS";	break;
	  case ERROR_DOMAIN_LIMIT_EXCEEDED:	what = "DOMAIN_LIMIT_EXCEEDED";	break;
	  case ERROR_INTERNAL_DB_CORRUPTION:	what = "INTERNAL_DB_CORRUPTION";	break;
	  case ERROR_INTERNAL_ERROR:		what = "INTERNAL_ERROR";	break;
	  case ERROR_GENERIC_NOT_MAPPED:	what = "GENERIC_NOT_MAPPED";	break;
	  case ERROR_BAD_DESCRIPTOR_FORMAT:	what = "BAD_DESCRIPTOR_FORMAT";	break;
	  case ERROR_NOT_LOGON_PROCESS:		what = "NOT_LOGON_PROCESS";	break;
	  case ERROR_LOGON_SESSION_EXISTS:	what = "LOGON_SESSION_EXISTS";	break;
	  case ERROR_NO_SUCH_PACKAGE:		what = "NO_SUCH_PACKAGE";	break;
	  case ERROR_BAD_LOGON_SESSION_STATE:	what = "BAD_LOGON_SESSION_STATE";	break;
	  case ERROR_LOGON_SESSION_COLLISION:	what = "LOGON_SESSION_COLLISION";	break;
	  case ERROR_INVALID_LOGON_TYPE:	what = "INVALID_LOGON_TYPE";	break;
	  case ERROR_CANNOT_IMPERSONATE:	what = "CANNOT_IMPERSONATE";	break;
	  case ERROR_RXACT_INVALID_STATE:	what = "RXACT_INVALID_STATE";	break;
	  case ERROR_RXACT_COMMIT_FAILURE:	what = "RXACT_COMMIT_FAILURE";	break;
	  case ERROR_SPECIAL_ACCOUNT:		what = "SPECIAL_ACCOUNT";	break;
	  case ERROR_SPECIAL_GROUP:		what = "SPECIAL_GROUP";	break;
	  case ERROR_SPECIAL_USER:		what = "SPECIAL_USER";	break;
	  case ERROR_MEMBERS_PRIMARY_GROUP:	what = "MEMBERS_PRIMARY_GROUP";	break;
	  case ERROR_TOKEN_ALREADY_IN_USE:	what = "TOKEN_ALREADY_IN_USE";	break;
	  case ERROR_NO_SUCH_ALIAS:		what = "NO_SUCH_ALIAS";	break;
	  case ERROR_MEMBER_NOT_IN_ALIAS:	what = "MEMBER_NOT_IN_ALIAS";	break;
	  case ERROR_MEMBER_IN_ALIAS:		what = "MEMBER_IN_ALIAS";	break;
	  case ERROR_ALIAS_EXISTS:		what = "ALIAS_EXISTS";	break;
	  case ERROR_LOGON_NOT_GRANTED:		what = "LOGON_NOT_GRANTED";	break;
	  case ERROR_TOO_MANY_SECRETS:		what = "TOO_MANY_SECRETS";	break;
	  case ERROR_SECRET_TOO_LONG:		what = "SECRET_TOO_LONG";	break;
	  case ERROR_INTERNAL_DB_ERROR:		what = "INTERNAL_DB_ERROR";	break;
	  case ERROR_TOO_MANY_CONTEXT_IDS:	what = "TOO_MANY_CONTEXT_IDS";	break;
	  case ERROR_LOGON_TYPE_NOT_GRANTED:	what = "LOGON_TYPE_NOT_GRANTED";	break;
	  case ERROR_NT_CROSS_ENCRYPTION_REQUIRED:	what = "NT_CROSS_ENCRYPTION_REQUIRED";	break;
	  case ERROR_NO_SUCH_MEMBER:		what = "NO_SUCH_MEMBER";	break;
	  case ERROR_INVALID_MEMBER:		what = "INVALID_MEMBER";	break;
	  case ERROR_TOO_MANY_SIDS:		what = "TOO_MANY_SIDS";	break;
	  case ERROR_LM_CROSS_ENCRYPTION_REQUIRED:	what = "LM_CROSS_ENCRYPTION_REQUIRED";	break;
	  case ERROR_NO_INHERITANCE:		what = "NO_INHERITANCE";	break;
	  case ERROR_FILE_CORRUPT:		what = "FILE_CORRUPT";	break;
	  case ERROR_DISK_CORRUPT:		what = "DISK_CORRUPT";	break;
	  case ERROR_NO_USER_SESSION_KEY:	what = "NO_USER_SESSION_KEY";	break;
	  case ERROR_INVALID_WINDOW_HANDLE:	what = "INVALID_WINDOW_HANDLE";	break;
	  case ERROR_INVALID_MENU_HANDLE:	what = "INVALID_MENU_HANDLE";	break;
	  case ERROR_INVALID_CURSOR_HANDLE:	what = "INVALID_CURSOR_HANDLE";	break;
	  case ERROR_INVALID_ACCEL_HANDLE:	what = "INVALID_ACCEL_HANDLE";	break;
	  case ERROR_INVALID_HOOK_HANDLE:	what = "INVALID_HOOK_HANDLE";	break;
	  case ERROR_INVALID_DWP_HANDLE:	what = "INVALID_DWP_HANDLE";	break;
	  case ERROR_TLW_WITH_WSCHILD:		what = "TLW_WITH_WSCHILD";	break;
	  case ERROR_CANNOT_FIND_WND_CLASS:	what = "CANNOT_FIND_WND_CLASS";	break;
	  case ERROR_WINDOW_OF_OTHER_THREAD:	what = "WINDOW_OF_OTHER_THREAD";	break;
	  case ERROR_HOTKEY_ALREADY_REGISTERED:	what = "HOTKEY_ALREADY_REGISTERED";	break;
	  case ERROR_CLASS_ALREADY_EXISTS:	what = "CLASS_ALREADY_EXISTS";	break;
	  case ERROR_CLASS_DOES_NOT_EXIST:	what = "CLASS_DOES_NOT_EXIST";	break;
	  case ERROR_CLASS_HAS_WINDOWS:		what = "CLASS_HAS_WINDOWS";	break;
	  case ERROR_INVALID_INDEX:		what = "INVALID_INDEX";	break;
	  case ERROR_INVALID_ICON_HANDLE:	what = "INVALID_ICON_HANDLE";	break;
	  case ERROR_PRIVATE_DIALOG_INDEX:	what = "PRIVATE_DIALOG_INDEX";	break;
	  case ERROR_LISTBOX_ID_NOT_FOUND:	what = "LISTBOX_ID_NOT_FOUND";	break;
	  case ERROR_NO_WILDCARD_CHARACTERS:	what = "NO_WILDCARD_CHARACTERS";	break;
	  case ERROR_CLIPBOARD_NOT_OPEN:	what = "CLIPBOARD_NOT_OPEN";	break;
	  case ERROR_HOTKEY_NOT_REGISTERED:	what = "HOTKEY_NOT_REGISTERED";	break;
	  case ERROR_WINDOW_NOT_DIALOG:		what = "WINDOW_NOT_DIALOG";	break;
	  case ERROR_CONTROL_ID_NOT_FOUND:	what = "CONTROL_ID_NOT_FOUND";	break;
	  case ERROR_INVALID_COMBOBOX_MESSAGE:	what = "INVALID_COMBOBOX_MESSAGE";	break;
	  case ERROR_WINDOW_NOT_COMBOBOX:	what = "WINDOW_NOT_COMBOBOX";	break;
	  case ERROR_INVALID_EDIT_HEIGHT:	what = "INVALID_EDIT_HEIGHT";	break;
	  case ERROR_DC_NOT_FOUND:		what = "DC_NOT_FOUND";	break;
	  case ERROR_INVALID_HOOK_FILTER:	what = "INVALID_HOOK_FILTER";	break;
	  case ERROR_INVALID_FILTER_PROC:	what = "INVALID_FILTER_PROC";	break;
	  case ERROR_HOOK_NEEDS_HMOD:		what = "HOOK_NEEDS_HMOD";	break;
	  case ERROR_GLOBAL_ONLY_HOOK:		what = "GLOBAL_ONLY_HOOK";	break;
	  case ERROR_JOURNAL_HOOK_SET:		what = "JOURNAL_HOOK_SET";	break;
	  case ERROR_HOOK_NOT_INSTALLED:	what = "HOOK_NOT_INSTALLED";	break;
	  case ERROR_INVALID_LB_MESSAGE:	what = "INVALID_LB_MESSAGE";	break;
	  case ERROR_SETCOUNT_ON_BAD_LB:	what = "SETCOUNT_ON_BAD_LB";	break;
	  case ERROR_LB_WITHOUT_TABSTOPS:	what = "LB_WITHOUT_TABSTOPS";	break;
	  case ERROR_DESTROY_OBJECT_OF_OTHER_THREAD:	what = "DESTROY_OBJECT_OF_OTHER_THREAD";	break;
	  case ERROR_CHILD_WINDOW_MENU:		what = "CHILD_WINDOW_MENU";	break;
	  case ERROR_NO_SYSTEM_MENU:		what = "NO_SYSTEM_MENU";	break;
	  case ERROR_INVALID_MSGBOX_STYLE:	what = "INVALID_MSGBOX_STYLE";	break;
	  case ERROR_INVALID_SPI_VALUE:		what = "INVALID_SPI_VALUE";	break;
	  case ERROR_SCREEN_ALREADY_LOCKED:	what = "SCREEN_ALREADY_LOCKED";	break;
	  case ERROR_HWNDS_HAVE_DIFF_PARENT:	what = "HWNDS_HAVE_DIFF_PARENT";	break;
	  case ERROR_NOT_CHILD_WINDOW:		what = "NOT_CHILD_WINDOW";	break;
	  case ERROR_INVALID_GW_COMMAND:	what = "INVALID_GW_COMMAND";	break;
	  case ERROR_INVALID_THREAD_ID:		what = "INVALID_THREAD_ID";	break;
	  case ERROR_NON_MDICHILD_WINDOW:	what = "NON_MDICHILD_WINDOW";	break;
	  case ERROR_POPUP_ALREADY_ACTIVE:	what = "POPUP_ALREADY_ACTIVE";	break;
	  case ERROR_NO_SCROLLBARS:		what = "NO_SCROLLBARS";	break;
	  case ERROR_INVALID_SCROLLBAR_RANGE:	what = "INVALID_SCROLLBAR_RANGE";	break;
	  case ERROR_INVALID_SHOWWIN_COMMAND:	what = "INVALID_SHOWWIN_COMMAND";	break;
	  case ERROR_EVENTLOG_FILE_CORRUPT:	what = "EVENTLOG_FILE_CORRUPT";	break;
	  case ERROR_EVENTLOG_CANT_START:	what = "EVENTLOG_CANT_START";	break;
	  case ERROR_LOG_FILE_FULL:		what = "LOG_FILE_FULL";	break;
	  case ERROR_EVENTLOG_FILE_CHANGED:	what = "EVENTLOG_FILE_CHANGED";	break;
	  case RPC_S_INVALID_STRING_BINDING:	what = "RPC_S_INVALID_STRING_BINDING";	break;
	  case RPC_S_WRONG_KIND_OF_BINDING:	what = "RPC_S_WRONG_KIND_OF_BINDING";	break;
	  case RPC_S_INVALID_BINDING:		what = "RPC_S_INVALID_BINDING";	break;
	  case RPC_S_PROTSEQ_NOT_SUPPORTED:	what = "RPC_S_PROTSEQ_NOT_SUPPORTED";	break;
	  case RPC_S_INVALID_RPC_PROTSEQ:	what = "RPC_S_INVALID_RPC_PROTSEQ";	break;
	  case RPC_S_INVALID_STRING_UUID:	what = "RPC_S_INVALID_STRING_UUID";	break;
	  case RPC_S_INVALID_ENDPOINT_FORMAT:	what = "RPC_S_INVALID_ENDPOINT_FORMAT";	break;
	  case RPC_S_INVALID_NET_ADDR:		what = "RPC_S_INVALID_NET_ADDR";	break;
	  case RPC_S_NO_ENDPOINT_FOUND:		what = "RPC_S_NO_ENDPOINT_FOUND";	break;
	  case RPC_S_INVALID_TIMEOUT:		what = "RPC_S_INVALID_TIMEOUT";	break;
	  case RPC_S_OBJECT_NOT_FOUND:		what = "RPC_S_OBJECT_NOT_FOUND";	break;
	  case RPC_S_ALREADY_REGISTERED:	what = "RPC_S_ALREADY_REGISTERED";	break;
	  case RPC_S_TYPE_ALREADY_REGISTERED:	what = "RPC_S_TYPE_ALREADY_REGISTERED";	break;
	  case RPC_S_ALREADY_LISTENING:		what = "RPC_S_ALREADY_LISTENING";	break;
	  case RPC_S_NO_PROTSEQS_REGISTERED:	what = "RPC_S_NO_PROTSEQS_REGISTERED";	break;
	  case RPC_S_NOT_LISTENING:		what = "RPC_S_NOT_LISTENING";	break;
	  case RPC_S_UNKNOWN_MGR_TYPE:		what = "RPC_S_UNKNOWN_MGR_TYPE";	break;
	  case RPC_S_UNKNOWN_IF:		what = "RPC_S_UNKNOWN_IF";	break;
	  case RPC_S_NO_BINDINGS:		what = "RPC_S_NO_BINDINGS";	break;
	  case RPC_S_NO_PROTSEQS:		what = "RPC_S_NO_PROTSEQS";	break;
	  case RPC_S_CANT_CREATE_ENDPOINT:	what = "RPC_S_CANT_CREATE_ENDPOINT";	break;
	  case RPC_S_OUT_OF_RESOURCES:		what = "RPC_S_OUT_OF_RESOURCES";	break;
	  case RPC_S_SERVER_UNAVAILABLE:	what = "RPC_S_SERVER_UNAVAILABLE";	break;
	  case RPC_S_SERVER_TOO_BUSY:		what = "RPC_S_SERVER_TOO_BUSY";	break;
	  case RPC_S_INVALID_NETWORK_OPTIONS:	what = "RPC_S_INVALID_NETWORK_OPTIONS";	break;
	  case RPC_S_NO_CALL_ACTIVE:		what = "RPC_S_NO_CALL_ACTIVE";	break;
	  case RPC_S_CALL_FAILED:		what = "RPC_S_CALL_FAILED";	break;
	  case RPC_S_CALL_FAILED_DNE:		what = "RPC_S_CALL_FAILED_DNE";	break;
	  case RPC_S_PROTOCOL_ERROR:		what = "RPC_S_PROTOCOL_ERROR";	break;
	  case RPC_S_UNSUPPORTED_TRANS_SYN:	what = "RPC_S_UNSUPPORTED_TRANS_SYN";	break;
	  case RPC_S_UNSUPPORTED_TYPE:		what = "RPC_S_UNSUPPORTED_TYPE";	break;
	  case RPC_S_INVALID_TAG:		what = "RPC_S_INVALID_TAG";	break;
	  case RPC_S_INVALID_BOUND:		what = "RPC_S_INVALID_BOUND";	break;
	  case RPC_S_NO_ENTRY_NAME:		what = "RPC_S_NO_ENTRY_NAME";	break;
	  case RPC_S_INVALID_NAME_SYNTAX:	what = "RPC_S_INVALID_NAME_SYNTAX";	break;
	  case RPC_S_UNSUPPORTED_NAME_SYNTAX:	what = "RPC_S_UNSUPPORTED_NAME_SYNTAX";	break;
	  case RPC_S_UUID_NO_ADDRESS:		what = "RPC_S_UUID_NO_ADDRESS";	break;
	  case RPC_S_DUPLICATE_ENDPOINT:	what = "RPC_S_DUPLICATE_ENDPOINT";	break;
	  case RPC_S_UNKNOWN_AUTHN_TYPE:	what = "RPC_S_UNKNOWN_AUTHN_TYPE";	break;
	  case RPC_S_MAX_CALLS_TOO_SMALL:	what = "RPC_S_MAX_CALLS_TOO_SMALL";	break;
	  case RPC_S_STRING_TOO_LONG:		what = "RPC_S_STRING_TOO_LONG";	break;
	  case RPC_S_PROTSEQ_NOT_FOUND:		what = "RPC_S_PROTSEQ_NOT_FOUND";	break;
	  case RPC_S_PROCNUM_OUT_OF_RANGE:	what = "RPC_S_PROCNUM_OUT_OF_RANGE";	break;
	  case RPC_S_BINDING_HAS_NO_AUTH:	what = "RPC_S_BINDING_HAS_NO_AUTH";	break;
	  case RPC_S_UNKNOWN_AUTHN_SERVICE:	what = "RPC_S_UNKNOWN_AUTHN_SERVICE";	break;
	  case RPC_S_UNKNOWN_AUTHN_LEVEL:	what = "RPC_S_UNKNOWN_AUTHN_LEVEL";	break;
	  case RPC_S_INVALID_AUTH_IDENTITY:	what = "RPC_S_INVALID_AUTH_IDENTITY";	break;
	  case RPC_S_UNKNOWN_AUTHZ_SERVICE:	what = "RPC_S_UNKNOWN_AUTHZ_SERVICE";	break;
	  case EPT_S_INVALID_ENTRY:		what = "EPT_S_INVALID_ENTRY";	break;
	  case EPT_S_CANT_PERFORM_OP:		what = "EPT_S_CANT_PERFORM_OP";	break;
	  case EPT_S_NOT_REGISTERED:		what = "EPT_S_NOT_REGISTERED";	break;
	  case RPC_S_NOTHING_TO_EXPORT:		what = "RPC_S_NOTHING_TO_EXPORT";	break;
	  case RPC_S_INCOMPLETE_NAME:		what = "RPC_S_INCOMPLETE_NAME";	break;
	  case RPC_S_INVALID_VERS_OPTION:	what = "RPC_S_INVALID_VERS_OPTION";	break;
	  case RPC_S_NO_MORE_MEMBERS:		what = "RPC_S_NO_MORE_MEMBERS";	break;
	  case RPC_S_NOT_ALL_OBJS_UNEXPORTED:	what = "RPC_S_NOT_ALL_OBJS_UNEXPORTED";	break;
	  case RPC_S_INTERFACE_NOT_FOUND:	what = "RPC_S_INTERFACE_NOT_FOUND";	break;
	  case RPC_S_ENTRY_ALREADY_EXISTS:	what = "RPC_S_ENTRY_ALREADY_EXISTS";	break;
	  case RPC_S_ENTRY_NOT_FOUND:		what = "RPC_S_ENTRY_NOT_FOUND";	break;
	  case RPC_S_NAME_SERVICE_UNAVAILABLE:	what = "RPC_S_NAME_SERVICE_UNAVAILABLE";	break;
	  case RPC_S_INVALID_NAF_ID:		what = "RPC_S_INVALID_NAF_ID";	break;
	  case RPC_S_CANNOT_SUPPORT:		what = "RPC_S_CANNOT_SUPPORT";	break;
	  case RPC_S_NO_CONTEXT_AVAILABLE:	what = "RPC_S_NO_CONTEXT_AVAILABLE";	break;
	  case RPC_S_INTERNAL_ERROR:		what = "RPC_S_INTERNAL_ERROR";	break;
	  case RPC_S_ZERO_DIVIDE:		what = "RPC_S_ZERO_DIVIDE";	break;
	  case RPC_S_ADDRESS_ERROR:		what = "RPC_S_ADDRESS_ERROR";	break;
	  case RPC_S_FP_DIV_ZERO:		what = "RPC_S_FP_DIV_ZERO";	break;
	  case RPC_S_FP_UNDERFLOW:		what = "RPC_S_FP_UNDERFLOW";	break;
	  case RPC_S_FP_OVERFLOW:		what = "RPC_S_FP_OVERFLOW";	break;
	  case RPC_X_NO_MORE_ENTRIES:		what = "RPC_X_NO_MORE_ENTRIES";	break;
	  case RPC_X_SS_CHAR_TRANS_OPEN_FAIL:	what = "RPC_X_SS_CHAR_TRANS_OPEN_FAIL";	break;
	  case RPC_X_SS_CHAR_TRANS_SHORT_FILE:	what = "RPC_X_SS_CHAR_TRANS_SHORT_FILE";	break;
	  case RPC_X_SS_IN_NULL_CONTEXT:	what = "RPC_X_SS_IN_NULL_CONTEXT";	break;
	  case RPC_X_SS_CONTEXT_DAMAGED:	what = "RPC_X_SS_CONTEXT_DAMAGED";	break;
	  case RPC_X_SS_HANDLES_MISMATCH:	what = "RPC_X_SS_HANDLES_MISMATCH";	break;
	  case RPC_X_SS_CANNOT_GET_CALL_HANDLE:	what = "RPC_X_SS_CANNOT_GET_CALL_HANDLE";	break;
	  case RPC_X_NULL_REF_POINTER:		what = "RPC_X_NULL_REF_POINTER";	break;
	  case RPC_X_ENUM_VALUE_OUT_OF_RANGE:	what = "RPC_X_ENUM_VALUE_OUT_OF_RANGE";	break;
	  case RPC_X_BYTE_COUNT_TOO_SMALL:	what = "RPC_X_BYTE_COUNT_TOO_SMALL";	break;
	  case RPC_X_BAD_STUB_DATA:		what = "RPC_X_BAD_STUB_DATA";	break;
	  case ERROR_INVALID_USER_BUFFER:	what = "INVALID_USER_BUFFER";	break;
	  case ERROR_UNRECOGNIZED_MEDIA:	what = "UNRECOGNIZED_MEDIA";	break;
	  case ERROR_NO_TRUST_LSA_SECRET:	what = "NO_TRUST_LSA_SECRET";	break;
	  case ERROR_NO_TRUST_SAM_ACCOUNT:	what = "NO_TRUST_SAM_ACCOUNT";	break;
	  case ERROR_TRUSTED_DOMAIN_FAILURE:	what = "TRUSTED_DOMAIN_FAILURE";	break;
	  case ERROR_TRUSTED_RELATIONSHIP_FAILURE:	what = "TRUSTED_RELATIONSHIP_FAILURE";	break;
	  case ERROR_TRUST_FAILURE:		what = "TRUST_FAILURE";	break;
	  case RPC_S_CALL_IN_PROGRESS:		what = "RPC_S_CALL_IN_PROGRESS";	break;
	  case ERROR_NETLOGON_NOT_STARTED:	what = "NETLOGON_NOT_STARTED";	break;
	  case ERROR_ACCOUNT_EXPIRED:		what = "ACCOUNT_EXPIRED";	break;
	  case ERROR_REDIRECTOR_HAS_OPEN_HANDLES:	what = "REDIRECTOR_HAS_OPEN_HANDLES";	break;
	  case ERROR_PRINTER_DRIVER_ALREADY_INSTALLED:	what = "PRINTER_DRIVER_ALREADY_INSTALLED";	break;
	  case ERROR_UNKNOWN_PORT:		what = "UNKNOWN_PORT";	break;
	  case ERROR_UNKNOWN_PRINTER_DRIVER:	what = "UNKNOWN_PRINTER_DRIVER";	break;
	  case ERROR_UNKNOWN_PRINTPROCESSOR:	what = "UNKNOWN_PRINTPROCESSOR";	break;
	  case ERROR_INVALID_SEPARATOR_FILE:	what = "INVALID_SEPARATOR_FILE";	break;
	  case ERROR_INVALID_PRIORITY:		what = "INVALID_PRIORITY";	break;
	  case ERROR_INVALID_PRINTER_NAME:	what = "INVALID_PRINTER_NAME";	break;
	  case ERROR_PRINTER_ALREADY_EXISTS:	what = "PRINTER_ALREADY_EXISTS";	break;
	  case ERROR_INVALID_PRINTER_COMMAND:	what = "INVALID_PRINTER_COMMAND";	break;
	  case ERROR_INVALID_DATATYPE:		what = "INVALID_DATATYPE";	break;
	  case ERROR_INVALID_ENVIRONMENT:	what = "INVALID_ENVIRONMENT";	break;
	  case RPC_S_NO_MORE_BINDINGS:		what = "RPC_S_NO_MORE_BINDINGS";	break;
	  case ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT:	what = "NOLOGON_INTERDOMAIN_TRUST_ACCOUNT";	break;
	  case ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT:	what = "NOLOGON_WORKSTATION_TRUST_ACCOUNT";	break;
	  case ERROR_NOLOGON_SERVER_TRUST_ACCOUNT:	what = "NOLOGON_SERVER_TRUST_ACCOUNT";	break;
	  case ERROR_DOMAIN_TRUST_INCONSISTENT:	what = "DOMAIN_TRUST_INCONSISTENT";	break;
	  case ERROR_SERVER_HAS_OPEN_HANDLES:	what = "SERVER_HAS_OPEN_HANDLES";	break;
	  case ERROR_RESOURCE_DATA_NOT_FOUND:	what = "RESOURCE_DATA_NOT_FOUND";	break;
	  case ERROR_RESOURCE_TYPE_NOT_FOUND:	what = "RESOURCE_TYPE_NOT_FOUND";	break;
	  case ERROR_RESOURCE_NAME_NOT_FOUND:	what = "RESOURCE_NAME_NOT_FOUND";	break;
	  case ERROR_RESOURCE_LANG_NOT_FOUND:	what = "RESOURCE_LANG_NOT_FOUND";	break;
	  case ERROR_NOT_ENOUGH_QUOTA:		what = "NOT_ENOUGH_QUOTA";	break;
	  case RPC_S_NO_INTERFACES:		what = "RPC_S_NO_INTERFACES";	break;
	  case RPC_S_CALL_CANCELLED:		what = "RPC_S_CALL_CANCELLED";	break;
	  case RPC_S_BINDING_INCOMPLETE:	what = "RPC_S_BINDING_INCOMPLETE";	break;
	  case RPC_S_COMM_FAILURE:		what = "RPC_S_COMM_FAILURE";	break;
	  case RPC_S_UNSUPPORTED_AUTHN_LEVEL:	what = "RPC_S_UNSUPPORTED_AUTHN_LEVEL";	break;
	  case RPC_S_NO_PRINC_NAME:		what = "RPC_S_NO_PRINC_NAME";	break;
	  case RPC_S_NOT_RPC_ERROR:		what = "RPC_S_NOT_RPC_ERROR";	break;
	  case RPC_S_UUID_LOCAL_ONLY:		what = "RPC_S_UUID_LOCAL_ONLY";	break;
	  case RPC_S_SEC_PKG_ERROR:		what = "RPC_S_SEC_PKG_ERROR";	break;
	  case RPC_S_NOT_CANCELLED:		what = "RPC_S_NOT_CANCELLED";	break;
	  case RPC_X_INVALID_ES_ACTION:		what = "RPC_X_INVALID_ES_ACTION";	break;
	  case RPC_X_WRONG_ES_VERSION:		what = "RPC_X_WRONG_ES_VERSION";	break;
	  case RPC_X_WRONG_STUB_VERSION:	what = "RPC_X_WRONG_STUB_VERSION";	break;
	  case RPC_S_GROUP_MEMBER_NOT_FOUND:	what = "RPC_S_GROUP_MEMBER_NOT_FOUND";	break;
	  case EPT_S_CANT_CREATE:		what = "EPT_S_CANT_CREATE";	break;
	  case RPC_S_INVALID_OBJECT:		what = "RPC_S_INVALID_OBJECT";	break;
	  case ERROR_INVALID_TIME:		what = "INVALID_TIME";	break;
	  case ERROR_INVALID_FORM_NAME:		what = "INVALID_FORM_NAME";	break;
	  case ERROR_INVALID_FORM_SIZE:		what = "INVALID_FORM_SIZE";	break;
	  case ERROR_ALREADY_WAITING:		what = "ALREADY_WAITING";	break;
	  case ERROR_PRINTER_DELETED:		what = "PRINTER_DELETED";	break;
	  case ERROR_INVALID_PRINTER_STATE:	what = "INVALID_PRINTER_STATE";	break;
	  case ERROR_PASSWORD_MUST_CHANGE:	what = "PASSWORD_MUST_CHANGE";	break;
	  case ERROR_DOMAIN_CONTROLLER_NOT_FOUND:	what = "DOMAIN_CONTROLLER_NOT_FOUND";	break;
	  case ERROR_ACCOUNT_LOCKED_OUT:	what = "ACCOUNT_LOCKED_OUT";	break;
	  case ERROR_NO_BROWSER_SERVERS_FOUND:	what = "NO_BROWSER_SERVERS_FOUND";	break;
	  case ERROR_INVALID_PIXEL_FORMAT:	what = "INVALID_PIXEL_FORMAT";	break;
	  case ERROR_BAD_DRIVER:		what = "BAD_DRIVER";	break;
	  case ERROR_INVALID_WINDOW_STYLE:	what = "INVALID_WINDOW_STYLE";	break;
	  case ERROR_METAFILE_NOT_SUPPORTED:	what = "METAFILE_NOT_SUPPORTED";	break;
	  case ERROR_TRANSFORM_NOT_SUPPORTED:	what = "TRANSFORM_NOT_SUPPORTED";	break;
	  case ERROR_CLIPPING_NOT_SUPPORTED:	what = "CLIPPING_NOT_SUPPORTED";	break;
	  case ERROR_UNKNOWN_PRINT_MONITOR:	what = "UNKNOWN_PRINT_MONITOR";	break;
	  case ERROR_PRINTER_DRIVER_IN_USE:	what = "PRINTER_DRIVER_IN_USE";	break;
	  case ERROR_SPOOL_FILE_NOT_FOUND:	what = "SPOOL_FILE_NOT_FOUND";	break;
	  case ERROR_SPL_NO_STARTDOC:		what = "SPL_NO_STARTDOC";	break;
	  case ERROR_SPL_NO_ADDJOB:		what = "SPL_NO_ADDJOB";	break;
	  case ERROR_PRINT_PROCESSOR_ALREADY_INSTALLED:	what = "PRINT_PROCESSOR_ALREADY_INSTALLED";	break;
	  case ERROR_PRINT_MONITOR_ALREADY_INSTALLED:	what = "PRINT_MONITOR_ALREADY_INSTALLED";	break;
	  case ERROR_WINS_INTERNAL:		what = "WINS_INTERNAL";	break;
	  case ERROR_CAN_NOT_DEL_LOCAL_WINS:	what = "CAN_NOT_DEL_LOCAL_WINS";	break;
	  case ERROR_STATIC_INIT:		what = "STATIC_INIT";	break;
	  case ERROR_INC_BACKUP:		what = "INC_BACKUP";	break;
	  case ERROR_FULL_BACKUP:		what = "FULL_BACKUP";	break;
	  case ERROR_REC_NON_EXISTENT:		what = "REC_NON_EXISTENT";	break;
	  case ERROR_RPL_NOT_ALLOWED:		what = "RPL_NOT_ALLOWED";	break;
	  default	:
		sprintf(errornum, "error #%ld", error & 0xffff);
		what = errornum;
	}

	/* display the message */
	msg(MSG_ERROR,"[ss]$1: $2", where, what);
}
# define TRACE(stmt)	stmt
#else /* not DEBUGGING */
# define ElvisError(where)
# define TRACE(stmt)
#endif

/* Returns ElvTrue if a program uses Win32 format, and false otherwise.
 * This is significant because only Win32 programs support real pipes.
 */
static BOOL classifyprog(prog)
	char	*prog;	/* a program to check, possibly followed by args */
{
	char	*freethis = NULL;
	char	*name;
	char	*cp;
	long	exetype;

	/* if command uses shell metacharacters, then also check shell.
	 * If pipes, then check to make sure other progs are win32 also.
	 */
	if ((strchr(prog, '<') != NULL || strchr(prog, '>') != NULL)
		&& (!o_shell || !classifyprog(tochar8(o_shell))))
		goto NotWin32;
	cp = strchr(prog, '|');
	if (cp && !classifyprog(cp + 1))
		goto NotWin32;

	/* Isolate the program name within a COPY of the command string.
	 * Note that the copy has 5 extra chars, so we can append ".exe" and
	 * a NUL character.
	 */
	while (elvspace(*prog))
		prog++;
	freethis = name = safealloc(strlen(prog) + 5, sizeof(char));
	strcpy(name, prog);
	for (cp = name; *cp && !elvspace(*cp) && *cp != '/'; cp++)
	{
	}
	*cp = '\0';
	if (!*name)
		goto NotWin32;

	/* all win32 programs end with ".exe" */
	if (strlen(name) < 4 || name[strlen(name) - 4] != '.')
		strcat(name, ".exe");

	/* look for the program in current dir, or in path */
	if (_access(name, 0) != 0)
	{
		cp = getenv("PATH");
		if (!cp)
			goto NotWin32;
		name = iopath(cp, name, ElvFalse);
		if (!name)
			name = tochar8(o_shell);
		if (!name)
			goto NotWin32;
	}
TRACE(fprintf(tracelog, "classifyprog(\"%s\"), name=\"%s\"\n", prog, name));

	/* check the file's type */
	if (GetBinaryType(name, &exetype))
	{
TRACE(fprintf(tracelog, "GetBinaryType() succeeded, exetype=%d\n", exetype);)
		if (exetype != SCS_32BIT_BINARY && exetype != SCS_WOW_BINARY)
		{
			supports_detach = FALSE;
		}
		if (exetype == SCS_32BIT_BINARY || exetype == SCS_POSIX_BINARY)
		{
			safefree(freethis);
TRACE(fprintf(tracelog, "...classifyprog()returning ElvTrue\n", prog, name);)
			return ElvTrue;
		}
	}
TRACE(else fprintf(tracelog, "GetBinaryType() failed\n");)

#ifndef GUI_WIN32
	/* Windows95 apparently doesn't support GetBinaryType, but it *does*
	 * support pipes.  Since GetBinaryType() failed, return ElvTrue.
	 */
	safefree(freethis);
	return ElvTrue;
#endif

NotWin32:
	if (freethis != NULL)
		safefree(freethis);
TRACE(fprintf(tracelog, "...classifyprog()returning ElvFalse\n", prog, name);)
	return ElvFalse;
}

/* This function either creates a pipe, or creates a temp file and opens
 * two file descriptors which access it like a pipe.  It is sensitive to the
 * value of the "supports_pipes" variable, so it won't use pipes with a program
 * which doesn't support them.
 */
static BOOL MaybeCreatePipe(PHANDLE r, PHANDLE w)
{
	/* use CreatePipe() if possible */
	if (supports_pipes && CreatePipe(r, w, &inherit, 0))
	{
TRACE(fprintf(tracelog, "MaybeCreatePipe() using pipe\n"));
		return TRUE;
	}

	/* create a temp file, and open a write fd to it. */
	do
	{
		if (unique == 0)
			unique = (UINT)GetCurrentProcessId();
		else if (unique == 0xfffe)
			unique = 1;
		else
			unique++;
		GetTempFileName(TMPDIR, "elv", unique, pipefname);
		*w = CreateFile(pipefname, GENERIC_READ|GENERIC_WRITE,
				FILE_SHARE_READ|FILE_SHARE_WRITE, &inherit,
				CREATE_NEW,
				FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,
				INVALID_HANDLE_VALUE);
	} while (*w == INVALID_HANDLE_VALUE
			&& GetLastError() == ERROR_FILE_EXISTS);
	if (*w == INVALID_HANDLE_VALUE)
	{
		ElvisError("CreateFile[1]");
		*pipefname = '\0';
		return FALSE;
	}

	/* also open a file handle for reading */
	if (!DuplicateHandle(GetCurrentProcess(), *w, GetCurrentProcess(), r, 0, TRUE, DUPLICATE_SAME_ACCESS))
	{
		ElvisError("DuplicateHandle[1]");
		CloseHandle(*w);
		*w = INVALID_HANDLE_VALUE;
		(void)DeleteFile(pipefname);
		*pipefname = '\0';
		return FALSE;
	}

	/* done! */
TRACE(fprintf(tracelog, "MaybeCreatePipe() using file \"%s\"\n", pipefname));
	return TRUE;
}

/* Run a command via the shell.  If the command contains "$1", then replace
 * $1 with the name of the temp file (if any).  Uses the "start" variable.
 * Returns TRUE if successful, or FALSE if error.
 *
 * Note: We try to avoid using the shell to start the command.  This is
 * partly for the sake of efficiency, but mostly it is because the shell
 * sometimes creates a console window when run from the MFC elvis and I'm
 * trying to avoid that.
 */
static BOOL runcmd(cmd, detached)
	char		*cmd;		/* the command to run */
	BOOL		detached;	/* doesn't use the console */
{
	ElvisCHAR	*scansh;
	char		*scan;
	ElvisCHAR	*cmdline;
	BOOL		result;
	PROCESS_INFORMATION proc;	/* info about started process */
	int		shlen;		/* length of shell command */

#ifdef GUI_WIN32
# define ElvisDETACHED	(detached && supports_detach ? DETACHED_PROCESS : 0)
#else
# define ElvisDETACHED	0
#endif

TRACE(fprintf(tracelog, "runcmd(\"%s\", %sdetached, ...)\n", cmd, detached?"":"not ");)
TRACE(fprintf(tracelog, "GetStdHandle(STD_INPUT_HANDLE) = 0x%lx\n", (long)GetStdHandle(STD_INPUT_HANDLE));)
TRACE(fprintf(tracelog, "GetStdHandle(STD_OUTPUT_HANDLE) = 0x%lx\n", (long)GetStdHandle(STD_OUTPUT_HANDLE));)
TRACE(fprintf(tracelog, "GetStdHandle(STD_ERROR_HANDLE) = 0x%lx\n", (long)GetStdHandle(STD_ERROR_HANDLE));)
TRACE(fprintf(tracelog, "start.hStdInput() = 0x%lx\n", (long)start.hStdInput);)
TRACE(fprintf(tracelog, "start.hStdOutput() = 0x%lx\n", (long)start.hStdOutput);)
TRACE(fprintf(tracelog, "start.hStdError() = 0x%lx\n", (long)start.hStdError);)
TRACE(fprintf(tracelog, "readfd = 0x%lx\n", (long)readfd);)
TRACE(fprintf(tracelog, "writefd = 0x%lx\n", (long)writefd);)

	/* build the command line */
	cmdline = NULL;
	shlen = 0;
	if (o_shell && *o_shell)
	{
		for (scansh = o_shell; *scansh; scansh++)
			buildCHAR(&cmdline, *scansh);
		for (scan = " /c "; *scan; scan++)
			buildCHAR(&cmdline, (ElvisCHAR)*scan);
		shlen = CHARlen(cmdline);
	}

	/* if any shell characters are used, then treat the shell command as
	 * part of the user's command line.  I.e., don't try to avoid using
	 * the shell.
	 */
	if (shlen > 0 && strpbrk(cmd, "<>|") != NULL)
	{
		shlen = 0;
	}

	/* append the user's command to the command line */
	for (; *cmd; cmd++)
	{
		if (cmd[0] == '$' && cmd[1] == '1')
		{
			for (scan = tempfname; *scan; scan++)
				buildCHAR(&cmdline, (ElvisCHAR)*scan);
			cmd++;
		}
		else
		{
			buildCHAR(&cmdline, (ElvisCHAR)*cmd);
		}
	}

#ifdef GUI_WIN32
	/* For the "win32" gui, any subprocesses will normally start out
	 * minimized, because we don't really care what's in its window.
	 * Win32 just does weird things with console-less programs, and I'm
	 * tired of fighting it.
	 */
	start.wShowWindow = SW_HIDE;
	start.dwFlags |= STARTF_USESHOWWINDOW;
#endif

	/* run the command.  First try to run it without a shell.  If
	 * that fails (probably because the command is built into the
	 * shell) then try the same command with the shell.
	 */
	result = (CreateProcess(NULL, tochar8(cmdline + shlen),
			NULL, NULL, TRUE,
			ElvisDETACHED | NORMAL_PRIORITY_CLASS,
			NULL, NULL, &start, &proc)
		|| (shlen > 0 && CreateProcess(NULL, tochar8(cmdline),
			NULL, NULL, TRUE,
			ElvisDETACHED | NORMAL_PRIORITY_CLASS,
			NULL, NULL, &start, &proc)));
TRACE(fprintf(tracelog, "CreateProcess(\"%s\") returned %s\n", cmdline, result?"TRUE":"FALSE");)

	/* remember info about the process */
	if (result)
	{
		pid = proc.hProcess;
		tid = proc.hThread;
	}

	/* free the command line */
	safefree(cmdline);

	/* return the result */
	return result;

#undef ElvisDETACHED
}


/* Declares which program we'll run, and what we'll be doing with it.
 * This function should return ElvTrue if successful.  If there is an error,
 * it should issue an error message via msg(), and return ElvFalse.
 *
 * For Win32, the behavior of this function depends on willwrite.
 * If willwrite, then the command is saved and a temporary file is
 * is created to store the data that will become the program's stdin,
 * and the function succeeds if the temp file was created successfully.
 * Else the program is forked (with stdout/stderr redirected to a pipe
 * if willread) and the function succedes if pipe() and fork()
 * succeed.
 */
ELVBOOL prgopen(cmd, willwrite, willread)
	char		*cmd;		/* command string */
	ELVBOOL	willwrite;	/* if ElvTrue, redirect command's stdin */
	ELVBOOL	willread;	/* if ElvTrue, redirect command's stdout */
{
	HANDLE	piper, pipew;	/* read and write ends of pipe */
	HANDLE	handle;		/* temporary variable */

#ifdef DEBUGGING
	if (!tracelog) tracelog = fopen("trace.log", "w");
	fprintf(tracelog, "\nprgopen(\"%s\"%s%s)\n", cmd, willwrite?", willwrite":"", willread?", willread":"");
#endif

	/* Determine whether this is a Win32 program.  If not, then we can't
	 * use pipes.  Also, check whether program can be run detached from a
	 * console.
	 */
	supports_detach = FALSE; /*!!!*/
	supports_pipes = classifyprog(cmd);
TRACE(fprintf(tracelog, "supports_detach=%s, supports_pipes=%s\n", supports_detach ? "TRUE" : "FALSE", supports_pipes ? "TRUE" : "FALSE");)

	/* Mark both fd's as being unused */
	writefd = readfd = pid = tid = INVALID_HANDLE_VALUE;
	filter = (ELVBOOL)(willwrite && willread);
	command = NULL;

	/* Next step depends on what I/O we expect to do with this program */
	if (willwrite)
	{
		/* Either we'll be filtering text through the external
		 * program, or we'll be writing to a program which can only
		 * read from a file.  Either way, the program's input will
		 * come from a temporary file.
		 */

		/* save the command; we'll need it in prggo()*/
		command = strdup(cmd);

		/* create a temporary file for feeding the program's stdin */
		do
		{
			if (unique == 0)
				unique = (UINT)GetCurrentProcessId();
			else if (unique == 0xfffe)
				unique = 1;
			else
				unique++;
			GetTempFileName(TMPDIR, "elv", unique, tempfname);
			writefd = CreateFile(tempfname, GENERIC_READ|GENERIC_WRITE,
					FILE_SHARE_READ, &inherit, CREATE_NEW,
					FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,
					INVALID_HANDLE_VALUE);
		} while (writefd == INVALID_HANDLE_VALUE
				&& GetLastError() == ERROR_FILE_EXISTS);
		if (writefd == INVALID_HANDLE_VALUE)
		{
			ElvisError("CreateFile[3]");
			*tempfname = '\0';
			free(command);
			return ElvFalse;
		}
TRACE(fprintf(tracelog, "%s(%d) tmpfile=0x%lx\n", __FILE__, __LINE__, (long)writefd);)
	}
	else if (willread) /* but not willwrite */
	{
		/* We're reading the program's stdout and leaving its stdin
		 * unredirected.  Use a pipe.
		 */

		/* create a pipe */
		if (!MaybeCreatePipe(&piper, &pipew))
		{
			return ElvFalse;
		}
TRACE(fprintf(tracelog, "%s(%d) piper=0x%lx, pipew=0x%lx\n", __FILE__, __LINE__, (long)piper, (long)pipew);)

		/* set the child's stdio, so stdout/stderr are redirected */
#ifdef GUI_WIN32
		start.hStdInput = pipew;
#else
		start.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
#endif
		start.hStdOutput = pipew;
		start.hStdError = pipew;

		/* make the read end of the pipe non-inheritable */
		if (!DuplicateHandle(GetCurrentProcess(), piper, GetCurrentProcess(), &handle, GENERIC_READ, FALSE, 0))
		{
			ElvisError("DuplicateHandle[2]");
			CloseHandle(piper);
			CloseHandle(pipew);
			if (*pipefname)
				(void)DeleteFile(pipefname);
			return ElvFalse;
		}
TRACE(fprintf(tracelog, "%s(%d) piper=0x%lx (was 0x%lx)\n", __FILE__, __LINE__, (long)handle, (long)piper);)
		CloseHandle(piper);
		readfd = piper = handle;

		/* fork off the external program */
		start.dwFlags = STARTF_USESTDHANDLES;
		if (!runcmd(cmd, TRUE))
		{
			ElvisError("runcmd[1]");
			CloseHandle(piper);
			CloseHandle(pipew);
			if (*pipefname)
				(void)DeleteFile(pipefname);
			return ElvFalse;
		}

		CloseHandle(pipew);
	}
	else /* no redirection */
	{
		/* fork off the external program */
		start.dwFlags = 0;
		if (!runcmd(cmd, FALSE))
		{
			ElvisError("runcmd[2]");
			return ElvFalse;
		}
	}

	/* if we get here, we must have succeeded */
	return ElvTrue;
}

/* Write the contents of buf to the program's stdin, and return nbytes
 * if successful, or -1 for error.  Note that this text should
 * be subjected to the same kind of transformations as textwrite().
 * In fact, it may use textwrite() internally.
 *
 * For Win32, this is simply a WriteFile() to the temp file or pipe.
 */
int prgwrite(buf, nbytes)
	ElvisCHAR	*buf;	/* buffer, contains text to be written */
	int		nbytes;	/* number of characters in buf */
{
	DWORD	written;/* number of characters actually written */

	assert(writefd != INVALID_HANDLE_VALUE);
	if (!WriteFile(writefd, buf, (DWORD)nbytes, &written, NULL))
		return -1;
TRACE(if ((DWORD)nbytes!=written)msg(MSG_INFO, "[dd]prgwrite(..., $1) returning $2", (long)nbytes, (long)written);)
	return written;
}

/* Marks the end of writing.  Returns ElvTrue if all is okay, or ElvFalse if
 * error.
 *
 * For Win32, the temp file is closed, and the program is forked.
 * Returns ElvTrue if the fork was successful, or ElvFalse if it failed.
 */
ELVBOOL prggo()
{
	HANDLE	piper, pipew;	/* read & write ends of a pipe */
	HANDLE	handle;		/* temporary variable */

TRACE(fprintf(tracelog, "prggo(), readfd=%x, writefd=%x\n", readfd, writefd));

	/* If we weren't writing, then there's nothing to be done here */
	if (writefd != INVALID_HANDLE_VALUE)
	{

		/* If we're using a temp file, then rewind it, and then
		 * fork the program with its stdin redirected to come from file.
		 */
		if (command)
		{
			/* rewind the temp file, and use it for stdin */
			start.hStdInput = writefd;
			(void)SetFilePointer(writefd, 0L, NULL, FILE_BEGIN);

			piper = pipew = INVALID_HANDLE_VALUE;
			if (filter)
			{
				/* make a pipe to use for reading stdout/stderr */
				if (!MaybeCreatePipe(&piper, &pipew))
				{
					return ElvFalse;
				}
	TRACE(fprintf(tracelog, "%s(%d) piper=0x%lx, pipew=0x%lx\n", __FILE__, __LINE__, (long)piper, (long)pipew);)

				/* make the read end be non-inheritable */
				if (!DuplicateHandle(GetCurrentProcess(), piper, GetCurrentProcess(), &handle, 0, FALSE, DUPLICATE_SAME_ACCESS))
				{
					ElvisError("DuplicateHandle[3]");
					CloseHandle(piper);
					CloseHandle(pipew);
					if (*pipefname)
						(void)DeleteFile(pipefname);
					return ElvFalse;
				}
	TRACE(fprintf(tracelog, "%s(%d) piper=0x%lx (was 0x%lx)\n", __FILE__, __LINE__, (long)handle, (long)piper);)
				CloseHandle(piper);
				readfd = piper = handle;

				/* use the pipe's write end for stdout/stderr */
				start.hStdOutput = start.hStdError = pipew;
			}
			else
			{
				/* don't redirect stdout/stderr */
				start.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
				start.hStdError = GetStdHandle(STD_ERROR_HANDLE);
			}

			/* start the process */
			start.dwFlags = STARTF_USESTDHANDLES;
			if (!runcmd(command, TRUE))
			{
				ElvisError("runcmd[3]");
				return ElvFalse;
			}

			/* close the write-end of the pipe */
			if (pipew != INVALID_HANDLE_VALUE)
				CloseHandle(pipew);

			/* don't need the command string any more. */
			free(command);
		}

		/* close the writefd */
		CloseHandle(writefd);
		writefd = INVALID_HANDLE_VALUE;
	}

	/* If we're using temp files to read stdout/stderr, then wait
	 * for the program to terminate, so we can be sure the temp
	 * file contains all the data it will ever contain, before the
	 * first prgread() call.  (If we're using a pipe, then we don't
	 * need to worry about this.)
	 */
TRACE(fprintf(tracelog, "pipefname=\"%s\", bytes=%ld\n", pipefname, *pipefname ? SetFilePointer(readfd, 0L, NULL, FILE_END) : 0L);)
	if (*pipefname)
	{
		(void)WaitForSingleObject(pid, INFINITE);
		GetExitCodeProcess(pid, &status);
		CloseHandle(pid);
		CloseHandle(tid);
		pid = tid = INVALID_HANDLE_VALUE;
		if (SetFilePointer(readfd, 0L, NULL, FILE_BEGIN) == 0xffffffff)
		{
			ElvisError("SetFilePointer");
		}
	}

	return ElvTrue;
}


/* Reads text from the program's stdout, and returns the number of
 * characters read.  At EOF, it returns 0.  Note that this text
 * should be subjected to the same kinds of transformations as
 * textread().
 *
 * For Win32, we use ReadFile() to read from the pipe, and then perform
 * newline conversion explicitly, since pipes are always binary.
 */
int prgread(buf, nbytes)
	ElvisCHAR	*buf;	/* buffer where text should be placed */
	int		nbytes;	/* maximum number of characters to read */
{
	DWORD	nread;
	int	i, j;

TRACE(fprintf(tracelog, "prgread(buf, %d), readfd=%d\n", nbytes, readfd); fflush(tracelog);)

	assert(readfd != INVALID_HANDLE_VALUE);
	if (!ReadFile(readfd, buf, (DWORD)nbytes, &nread, NULL))
	{
TRACE(fprintf(tracelog, "prgread() returning -1\n"); fflush(tracelog);)
		return -1;
	}

	/* Convert CR-LF to LF.  This will fail if a CR/LF pair is split
	 * across two prgread() calls.  (That's a bug!!!)
	 */
	for (i = j = 0; (DWORD)j < nread; j++)
	{
		if (buf[j] != '\r' || (DWORD)(j + 1) >= nread || buf[j + 1] != '\n')
			buf[i++] = buf[j];
	}
	nread = i;

TRACE(fprintf(tracelog, "prgread() returning %d\n", nread); fflush(tracelog);)
	return nread;
}

/* Clean up, and return the program's exit status.  The exit status
 * should be 0 normally.
 *
 * For Win32, this involves closing the pipe, calling WaitForSingleObject()
 * to get the program's exit status, and then deleting the temp file.
 */
int prgclose()
{
	/* close the readfd, if necessary */
	if (readfd != INVALID_HANDLE_VALUE)
	{
		CloseHandle(readfd);
		readfd = INVALID_HANDLE_VALUE;
	}

	/* close the writefd, if necessary */
	if (writefd != INVALID_HANDLE_VALUE)
	{
		CloseHandle(writefd);
		writefd = INVALID_HANDLE_VALUE;
	}

	/* wait for the program to die */
	if (pid != INVALID_HANDLE_VALUE)
	{
TRACE(fprintf(tracelog, "About to wait for program to die, pid=%d\n", pid); fflush(tracelog);)
		(void)WaitForSingleObject(pid, INFINITE);
		GetExitCodeProcess(pid, &status);
		CloseHandle(pid);
		CloseHandle(tid);
	}

	/* delete the temp files, if there were any */
	if (*tempfname)
	{
		(void)DeleteFile(tempfname);
		*tempfname = '\0';
	}
	if (*pipefname)
	{
		(void)DeleteFile(pipefname);
		*pipefname = '\0';
	}

	return status;
}
