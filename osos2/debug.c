#include <config.h>
#include <stdio.h>
#include <stdarg.h>
#include "debug.h"

/**
 * Routine to print messages to stdout.
 *
 * This routine behaves just like {\em printf ()}.
 *
 */
/*@printflike@*/ int /*@alt void@*/
debug_printf (const char *template, ...)
   /*@globals stderr@*/
   /*@modifies fileSystem, *stderr@*/
{
  va_list ap;
  int result;

  va_start (ap, template);
  result = vfprintf (stderr, template, ap);
  va_end (ap);
  return result;
}
