/* osos2/osdef.h */

#define PORTEDBY \
 "OS/2 port by Lee Johnson (lee_johnson@sympatico.ca) and Martin\n\"Herbert\" Dietze (herbert@the-little-red-haired-girl.org)."

/*=============================================================================
 * This is the name of the OS, as reported by ":set os?"
 */
#ifndef OSNAME
# define OSNAME	"os2"
#endif

/*=============================================================================
 * This is the default shell, as reported by ":set shell?"
 */
#ifndef OSSHELL
# define OSSHELL "cmd.exe"
#endif

/*=============================================================================
 * This is the default printer, as report by ":set lpout?".
 */
#define OSLPOUT "prn"

/*=============================================================================
 * This is the default printing setup, as report by ":set lptype?".
 */
#define OSLPTYPE "cr"

/*=============================================================================
 * This should be defined if there is an osinit() function.  When defined,
 * this function will be called after a GUI has been selected but before any
 * other initialization.  It is used mostly to initialize options.
 */
#define OSINIT

/*=============================================================================
 * Path separator characters.  If you prefer to use '/' instead of '\',
 * just swap the definitions of PATH_SEP and ALT_SEP.  OS/2 will accept
 * either character in a pathname in file system calls.
 */
#define OSPATHSEP   '\\'
#define OSALTPSEP   '/'


/*=============================================================================
 * OSFILENAMERULES should be a bitwise-OR of ELVFNR flags, indicating how
 * file names passed to elvis should be interpreted.  Usually this will be
 * "(ELVFNR_TILDE|ELVFNR_DOLLAR|ELVFNR_WILDCARD)" to do the kind of processing
 * that the shell would do on Unix, or "(ELVFNR)0" on Unix systems.
 *
 * OSEXPANDARGS should be 1 if the args ever need expansion, or 0 if they
 * never do.
 */
#define OSFILENAMERULES	(ELVFNR_TILDE|ELVFNR_DOLLAR|ELVFNR_WILDCARD)
#define OSEXPANDARGS	1

/*=============================================================================
 * This is the delimiter in a search path string
 */
#define OSPATHDELIM	';'

/*=============================================================================
 * This is used as the directory delimiter inside a file name.  For UNIX, this
 * is traditionally a '/' character.  Most other OSes use a '\\' character.
 */
#define OSDIRDELIM	'\\'

/*=============================================================================
 * OS/2 uses a case insignificant file system.
 */
#define FILES_IGNORE_CASE 1

/*=============================================================================
 * This is a list of directories where elvis might store its session file.
 */
#define OSSESSIONPATH (getenv ("TEMP") == NULL? \
                (getenv ("TMP") == NULL?        \
                (getenv ("HOME") == NULL?       \
                    ".": getenv ("HOME")): getenv ("TMP")): getenv ("TEMP"))

/*=============================================================================
 * This is the default path that elvis searches through when looking for its
 * support files. This is *never* used, it gets overridden by code in osinit()
 * on program startup. Call it `default for a default' :-)
 */
#define OSLIBPATH	"e:\\usr\\lib\\elvis"

/*=============================================================================
 * This is the default terminal type, used by the "termcap" GUI whenever the
 * TERM environment variable is unset.
 */
#define TTY_DEFAULT	"console"

/*=============================================================================
 * These are the default values of the ccprg and makeprg options
 */
#define OSCCPRG	"gcc -c ($1?$1:$2)"
#define OSMAKEPRG	"make"

/*=============================================================================
 * Configure compiler options.
 */
#ifdef __IBMC__
# if __IBMC__ >= 200	/* IBM C Set ++ version 2.0 or higher */
#  define USE_PROTOTYPES	1
#  define P_(args)	args
# endif
#endif
