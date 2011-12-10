/* oswin32/osdef.h */

/*=============================================================================
 * This is the name of the OS, as reported by ":set os?"
 */
#ifndef OSNAME
# define OSNAME	"win32"
#endif

/*=============================================================================
 * This is the default shell, as reported by ":set shell?"
 */
#ifndef OSSHELL
# define OSSHELL "cmd"
#endif
#ifndef OSSHELLENV
# define OSSHELLENV "COMSPEC"
#endif

/*=============================================================================
 * OSLPOUT is the default printer port, as reported by ":set lpout?".
 * OSLPTYPE is the default printer type, as reported by ":set lptype?".
 * Note that the "windows" lptype doesn't use lpout, but we set lpout anyway
 * to provide a reasonable default in case the user changes lptype.
 */
#define OSLPOUT "prn"
#ifdef GUI_WIN32
# define OSLPTYPE "windows"
#endif

/*=============================================================================
 * These are the default values of the ccprg and makeprg options
 */
#ifndef OSCCPRG
# define OSCCPRG	"cl /c ($1?$1:$2)"
#endif
#ifndef OSMAKEPRG
# define OSMAKEPRG	"make"
#endif

/*=============================================================================
 * This should be defined if there is an osinit() function.  When defined,
 * this function will be called after a GUI has been selected but before any
 * other initialization.  It is used mostly to initialize options.
 */
#define OSINIT

/*=============================================================================
 * OSFILENAMERULES should be a bitwise-OR of ELVFNR flags, indicating how
 * file names passed to elvis should be interpreted.  Usually this will be
 * "(ELVFNR_TILDE|ELVFNR_DOLLAR|ELVFNR_WILDCARD)" to do the kind of processing
 * that the shell would do on Unix, or "(ELVFNR)0" on Unix systems.
 *
 * OSEXPANDARGS should be 1 if the args ever need expansion, or 0 if they
 * never do.
 */
#define OSFILENAMERULES	(getenv("SHLVL") ? (ELVFNR)0 : (ELVFNR_TILDE|ELVFNR_DOLLAR|ELVFNR_WILDCARD))
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
 * This is the default path that elvis searches through when looking for its
 * support files.
 */
#define OSLIBPATH	"~;~\\lib"

/*=============================================================================
 * This is the default terminal type, used by the "termcap" GUI whenever the
 * TERM environment variable is unset.
 */
#define TTY_DEFAULT	"console"

/*=============================================================================
 * Use Prototypes, even though __STDC__ is undefined
 */
#define USE_PROTOTYPES	1

/*=============================================================================
 * This determines whether filename completion should ignore case differences.
 */
#define FILES_IGNORE_CASE 1
