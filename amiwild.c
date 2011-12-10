/* amiwild.c */

/*-
 *	Mike Rieser 				Dale Rahn
 *	2410 Happy Hollow Rd. Apt D-10		540 Vine St.
 *	West Lafayette, IN 47906 		West Lafayette, IN 47906
 *	riesermc@mentor.cc.purdue.edu		rahn@sage.cc.purdue.edu
 */

/* The following macros were defined in "vi.h". I'm undefining them so they
 * don't conflict with the versions in exec/io.h.
 */
#ifdef CMD_READ
#undef CMD_READ
#undef CMD_WRITE
#undef CMD_STOP
#endif

/* #include <string.h> */
#include <stdio.h>
#include <stddef.h>
#include <exec/memory.h>
#include <dos/dosextens.h>
#include <clib/dos_protos.h>

/* Some needed prototypes from clib/exec_protos.h */
extern APTR  AllocMem(unsigned long byteSize, unsigned long requirements);
extern void  FreeMem(APTR memoryBlock, unsigned long byteSize);
extern struct Library *OpenLibrary(UBYTE * libName, unsigned long version);
extern void  CloseLibrary(struct Library *library);

#if AZTEC_C
#include <pragmas/exec_lib.h>
#include <pragmas/dos_lib.h>
#else
#include <pragmas/exec.h>
#include <pragmas/dos.h>
#endif

#define	DOS_LIBRARY	((UBYTE *) "dos.library")

#ifdef AZTEC_C
/* Some needed prototypes from string.h and stdlib.h */
extern char *strdup(char *);
extern char *strpbrk(char *, char *);
extern void *realloc(void *, size_t);
extern void  free(void *);
#endif

/* Dynamic Stack Routines by Mike Rieser */
void         push(void *object);
void        *pop(void);

#define STACK_SIZE	20

static struct stack
{
    void       **top, **bottom;
} stack = { (void **) 0, (void **) 0 };

/* Functions */

/*-
 * Only push() a pointer to the object to be stacked!
 *
 * The first call to push() allocates the stack's memory,
 * and a push() to a full stack increases its size.
 *
 * WARNING:  Modification to an object after a push()
 * will effect the stacked value!
 */
void push(void   *object)
{
    size_t       stack_size = stack.top - stack.bottom;

    if (0 == stack_size % STACK_SIZE)
    {
	stack.bottom = (void **) realloc(stack.bottom, sizeof(stack.top)
					 * (stack_size + STACK_SIZE));
	if ((void **) 0 == stack.bottom)
	{
	    free(stack.bottom);
	    puts("Memory exhausted.");
	    clean_exit(10);
	}
	stack.top = stack.bottom + stack_size;
    }
    *stack.top++ = object;		/* increment the top of the stack */
    return;
}


/*-
 * pop() returns a pointer to the top object on the stack.
 *
 * pop() on the last elment frees the stack's memory.
 *
 * pop() on an empty stack is permitted and returns 0.
 *
 * NOTE: As long as you aren't trying to save NULL pointers,
 * you can use pop() to tell when the stack is empty.
 */
void *pop(void)
{
    void        *object;

    if (!stack.bottom)
	return (void *) 0;

    object = *--stack.top;

    if (stack.top == stack.bottom)
    {
	free(stack.bottom);
	stack.top = stack.bottom = (void **) 0;
    }
    return object;
}


/*
 * isOldDos - this function checks if the dos version is pre 2.x.
 */
int isOldDOS()
{
    static BOOL  OldDOS = -1;

    switch (OldDOS)
    {
    case 0:
	break;
    case 1:
	break;
    default:
	{
	    struct Library *DosBase;

	    if (DosBase = OpenLibrary(DOS_LIBRARY, 37L))
	    {
		CloseLibrary(DosBase);
		OldDOS = 0;
	    } else
	    {
		OldDOS = 1;
	    }
	}
    }

    return OldDOS;
}


/*
 * matchwild - pushes filenames which match the given pattern.
 * it also returns a count of the matches found.
*/
int matchwild(char *pattern)
{
    static char *special = "#?*%([|";	/* )] */
    struct AnchorPath *APath;
    int          matches = 0;
    LONG         error;

    /* Check if correct OS */
    if (isOldDOS())
	return;

    /* Check if pattern is special */
    if (!(strpbrk(pattern, special)))
	return;

    APath = AllocMem(sizeof(struct AnchorPath) + BLKSIZE, MEMF_CLEAR);

    if (!(APath))
	return;

    APath->ap_Strlen = BLKSIZE;
    APath->ap_BreakBits = SIGBREAKF_CTRL_C;

    if ((error = MatchFirst((UBYTE *) pattern, APath)) == 0)
    {
	do
	{
	    ++matches;
	    push(strdup((char *) APath->ap_Buf));
	}
	while ((error = MatchNext(APath)) == 0);
    }
    MatchEnd(APath);

    if (error != ERROR_NO_MORE_ENTRIES)
    {
	PrintFault(error, NULL);
    }
    FreeMem(APath, sizeof(struct AnchorPath) + BLKSIZE);

    return matches;
}


/*
 * expand -- returns new char **argv to replace previous one. It also adjusts
 * argc.
 * 
 * NOTE: The calling function really needs to free each element.
 */
char **expand(int  *argc, char **argv)
{
    int          i;
    static char *special = "#?*%([|";	/* )] */

    for (i = 0; i < *argc; ++i)
    {
	if (strpbrk(argv[i], special))
	{
	    matchwild(argv[i]);		/* expands the wildcard pattern */
	} else
	{
	    push(strdup(argv[i]));	/* Make sure nobody frees memory twice */
	}
    }

    *argc = stack.top - stack.bottom;
    return stack.bottom;
}


/*
 * This is something I wish I didn't have to participate in.
 *
 * wildcard - returns filename arguments in one string 
 * separated by spaces.
 */
char *wildcard(char *names)
{
    int          i, count;
    char        *next, *pc, *buf;

    buf = strdup(names);
    if (0 == (count = matchwild(names)))
    {
	strcpy(names, buf);
	free(buf);
	return names;
    }
    free(buf);

    buf = tmpblk.c;
    for (i = 0; i < count; ++i)
    {
	next = pc = pop();
        for (--next; *++next; )
            if (*next == ' ')
                *next = SPACEHOLDER;
	buf += sprintf(buf, "%s ", pc);
	free(pc);
    }

    return tmpblk.c;
}


#ifndef AZTEC_C

/*
 * strdup -- copies a string into a safe place.
 */
char *strdup(char *str)
{
    char        *dup = (char *) malloc(strlen(str) + 1);

    return (dup) ? strcpy(dup, str) : (char *) 0;	/* returns dup */
}

#endif


/*
 * Replace main by one that will expand the arg list.
 */
void main(int argc, char **argv)
{
    char       **nargv;

    if (argc == 0)
    	if (Output() == 0)
	    exit(2);	/* ran from WorkBench with no window */

    nargv = expand(&argc, argv);

    (void) _user_main(argc, nargv);
    (void) clean_exit(0);
}


clean_exit(int val)
{
    void        *pc;

    while (pc = pop())
	free(pc);
    exit(val);
}

#define main	_user_main
#define exit	clean_exit

/* The following macros were defined in <exec/io.h>. I'm undefining them so they
 * don't conflict with the versions in vi.h. If they're used in main.c after
 * this point, you need to redefine the way amiwild.c works.
 */
#ifdef CMD_READ
#undef CMD_READ
#endif

#ifdef CMD_WRITE
#undef CMD_WRITE
#endif

#ifdef CMD_STOP
#undef CMD_STOP
#endif
