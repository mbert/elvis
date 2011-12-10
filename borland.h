/* borland.h
 * This file will turn off the use of pre-compiled headers for a
 * single source file when "included" before other include files
 * and compiling with borland c. (Has no effect with turbo c.)
 * (All this to work around a stupid bug that results in an
 * executable not getting access to argc, argv, & envp. Needed 
 * only in a module with a 'main' function.)
 * This needs to be in a separate file, because not everyone
 * recognizes pragma directives.
 */
 
#pragma hdrstop
#pragma warn -pro
#pragma warn -use
#pragma warn -eff
#pragma warn -par
#pragma warn -pia
#pragma warn -ccc
