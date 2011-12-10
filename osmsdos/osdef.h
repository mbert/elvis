/* osmsdos/osdef.h */

#define PORTEDBY "DOS port by S.K., from code by Guntram Blohm & Martin Patzel"

/*=============================================================================
 * This is the name of the OS, as reported by ":set os?"
 */
#ifndef OSNAME
# define OSNAME	"msdos"
#endif

/*=============================================================================
 * This is the default shell, as reported by ":set shell?"
 */
#ifndef OSSHELL
# define OSSHELL "C:\\COMMAND.COM"
#endif
#ifndef OSSHELLENV
# define OSSHELLENV "COMSPEC"
#endif

/*=============================================================================
 * This is the default printer, as report by ":set lpout?".
 * NOTE: I'd like to use "!print $1 >nul" but the PRINT.EXE program doesn't
 * copy its files; since elvis deletes the temp file, the resident portion
 * of PRINT.EXE looses the file almost immediately after starting to print it.
 */
#ifndef OSLPOUT
# define OSLPOUT "prn"
#endif

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
 * This should be True if the argv[] array passed to main() needs to have
 * wildcards expanded in filenames.
 */
#define OSEXPANDARGS	1

/*=============================================================================
 * This is the delimiter in a search path string
 */
#define OSPATHDELIM	';'

/*=============================================================================
 * This is the default value of the "elvispath" option.  Normally this value
 * is never used because a custom value is created by OSINIT, but the compiler
 * needs this anyway.
 */
#define OSLIBPATH	"~\\lib;~"

/*=============================================================================
 * This is the default terminal type, used by the "termcap" GUI whenever the
 * TERM environment variable is unset.
 */
#define TTY_DEFAULT	"pcbios"

/*=============================================================================
 * This is just to make sure that prototypes are used
 */
#define USE_PROTOTYPES	1
