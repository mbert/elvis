#ifndef __DEBUG_H
#define __DEBUG_H

#ifndef NDEBUG

/*@printflike@*/ int /*@alt void@*/ 
debug_printf (const char *template, ...);
#define STRR(X) #X
#define STR(X) STRR(X)
#define DPRINTF(X) \
  (debug_printf("%s: %s ", __FILE__, STR(__LINE__)), debug_printf X)

#else
#define DPRINTF(x)
#endif /* NDEBUG*/

#endif /* __DEBUG_H */
