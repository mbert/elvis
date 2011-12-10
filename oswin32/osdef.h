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
 * This is the default printer, as report by ":set lpout?".
 */
#define OSLPOUT "prn"

/*=============================================================================
 * These are the default values of the ccprg and makeprg options
 */
#ifndef OSCCPRG
# define OSCCPRG	"cl /c ($1?$1:$2)"
#endif
#ifndef OSMAKEPRG
# define OSMAKEPRG	"nmake -f ($1?$1:basename($2)\".mak\")"
#endif

/*=============================================================================
 * This should be defined if there is an osinit() function.  When defined,
 * this function will be called after a GUI has been selected but before any
 * other initialization.  It is used mostly to initialize options.
 */
#define OSINIT

/*=============================================================================
 * This should be 1 if the argv[] array passed to main() needs to have
 * wildcards expanded in filenames.
 */
#define OSEXPANDARGS	1

/*=============================================================================
 * This is the delimiter in a search path string
 */
#define OSPATHDELIM	';'

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
