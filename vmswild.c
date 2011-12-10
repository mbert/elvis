/*
   This 'C' module may be included prior to the ``main'' programs on VMS in
   order to allow 'C' arguments to contain redirection symbols (<,>,>>) and
   VMS wild cards (*,% ...], [-).  By including this module, two programs
   redirect() and expand() are run prior to turning control over to
   your main() entry point.

/*
    redirect-- Gregg Townsend circa 1983,
    expand-- John Campbell circa 1987

   This code is public domain, others may use it freely. Credit, however, to
   Gregg Townsend (who wrote ``redirect()'') and John Campbell (who followed
   with ``expand()'') would be appreciated.  If someone writes the next
   logical successor ``pipe()'', please email a copy to
   ...!arizona!naucse!jdc (John Campbell) :-).
*/

#include <rms>   /* No easy way to tell if this has already been included. */
#ifndef ERANGE   /* Include only if missing. */
/* #include <stdlib>  causes lots of warnings. */
#endif
#include <stdio.h>  /* Stdio.h won't include itself twice. */

/* Expansion of wild cards is done using RMS. */
typedef struct NAMBLK { struct NAM nam;         /* VMS nam block structure */
                 char es[NAM$C_MAXRSS],         /* Extended string         */
                      rs[NAM$C_MAXRSS];         /* Resultant string        */
               };

#define ErrorExit 2

/* Allow the user to override _N_FARGS or _E_FLAG if they wish. */
#ifndef _N_FARGS
#define _N_FARGS 0
#endif
#ifndef _E_FLAG
#define _E_FLAG 2
#endif
/*
   Since the following will possibly be included in a single module, try
   hard to avoid name conflicts. (Just being static doesn't cut it if
   compiled in the same module.)
*/
#define redirect     _r_edirect
#define filearg      _f_ilearg
#define expand       _e_xpand
#define wild_found   _w_ild_found
#define wild_expand  _w_ild_expand

main(argc, argv, envp)
int argc;
char *argv[], *envp[];
{
   char **expand();

   redirect (&argc, argv, _N_FARGS);
   argv = expand (&argc, argv, _E_FLAG);

/* Make the user's main entry point this routine's entry point. */
#define main _user_main
   _user_main (argc, argv, envp);
}

/*
 * redirect(&argc,argv,nfargs) - redirect standard I/O
 *    int *argc         number of command arguments (from call to main)
 *    char *argv[]      command argument list (from call to main)
 *    int nfargs        number of filename arguments to process
 *
 * argc and argv will be adjusted by redirect.
 *
 * redirect processes a program's command argument list and handles redirection
 * of stdin, and stdout.  Any arguments which redirect I/O are removed from the
 * argument list, and argc is adjusted accordingly.  redirect would typically be
 * called as the first statement in the main program.
 *
 * Files are redirected based on syntax or position of command arguments.
 * Arguments of the following forms always redirect a file:
 *
 *    <file     redirects standard input to read the given file
 *    >file     redirects standard output to write to the given file
 *    >>file    redirects standard output to append to the given file
 *
 * It is often useful to allow alternate input and output files as the
 * first two command arguments without requiring the <file and >file
 * syntax.  If the nfargs argument to redirect is 2 or more then the
 * first two command arguments, if supplied, will be interpreted in this
 * manner:  the first argument replaces stdin and the second stdout.
 * A filename of "-" may be specified to occupy a position without
 * performing any redirection.
 *
 * If nfargs is 1, only the first argument will be considered and will
 * replace standard input if given.  Any arguments processed by setting
 * nfargs > 0 will be removed from the argument list, and again argc will
 * be adjusted.  Positional redirection follows syntax-specified
 * redirection and therefore overrides it.
 *
 */


redirect(argc,argv,nfargs)
int *argc, nfargs;
char *argv[];
{
   int i;

   i = 1;
   while (i < *argc)  {         /* for every command argument... */
      switch (argv[i][0])  {            /* check first character */
         case '<':                      /* <file redirects stdin */
            filearg(argc,argv,i,1,stdin,"r");
            break;
         case '>':                      /* >file or >>file redirects stdout */
            if (argv[i][1] == '>')
               filearg(argc,argv,i,2,stdout,"a");
            else
               filearg(argc,argv,i,1,stdout,"w");
            break;
         default:                       /* not recognized, go on to next arg */
            i++;
      }
   }
   if (nfargs >= 1 && *argc > 1)        /* if positional redirection & 1 arg */
      filearg(argc,argv,1,0,stdin,"r"); /* then redirect stdin */
   if (nfargs >= 2 && *argc > 1)        /* likewise for 2nd arg if wanted */
      filearg(argc,argv,1,0,stdout,"w");/* redirect stdout */
}



/* filearg(&argc,argv,n,i,fp,mode) - redirect and remove file argument
 *    int *argc         number of command arguments (from call to main)
 *    char *argv[]      command argument list (from call to main)
 *    int n             argv entry to use as file name and then delete
 *    int i             first character of file name to use (skip '<' etc.)
 *    FILE *fp          file pointer for file to reopen (typically stdin etc.)
 *    char mode[]       file access mode (see freopen spec)
 */

filearg(argc,argv,n,i,fp,mode)
int *argc, n, i;
char *argv[], mode[];
FILE *fp;
{
   if (strcmp(argv[n]+i,"-"))           /* alter file if arg not "-" */
      fp = freopen(argv[n]+i,mode,fp);
   if (fp == NULL)  {                   /* abort on error */
      fprintf(stderr,"%%can't open %s",argv[n]+i);
      exit(ErrorExit);
   }
   for ( ;  n < *argc;  n++)            /* move down following arguments */
      argv[n] = argv[n+1];
   *argc = *argc - 1;                   /* decrement argument count */
}

/* EXPAND code. */

/* Global prototype. */
char **expand (int *argc, const char *argv[], const int flag);
/*-
   ``expand()'' is a routine to expand wild-cards to file specifications.
   This routine is often used in conjunction with ``redirect()'' to provide
   both wild card expansion and standard file redirection prior to doing
   any real work in a 'C' program.

   Normal usage is to include the following line prior to using argc or
   argv in main():

     argv = expand (&argc, argv, 0);

   ``argc'' will be adjusted by ``expand()'', the return value from expand
   will replace ``argv''.

   ``expand()'' processes a program's command argument list and expands any
   wild cards into zero or more argv entries. Only arguments that posses VMS
   wild-cards are expanded. Wild cards searched for are ``*'', ``%'',
   ``...]'', and ``[-''. If the wild-card is found inside a single or double
   quote ("*" or '%') then they are not counted as wild-cards. Be aware that
   the expansion of a VMS wild card will match all VMS files, including
   directory files (".DIR;1").

   NOTE: The use of quotes in VMS requires thinking about how the CLI expands
   things before handing the argument line over to your program.  Do not
   expect "*" to avoid expansion, use """*""" instead.  Likewise, expression
   substitution precludes the use of (') to quote wild cards:
           $ A := HELLO
           $ ECHO 'a'   ! 'C' program that calls ``expand()''
           hello
   The easiest way to escape a wild-card may be "'*'".  The point is that
   ``expand()'' will only recognize quotes passed into main().

   ``expand()'' references the VMS runtime routines, you will need to
   link with the 'C' RTL whenever expand is used.

   Parameters:

         argc:  Pointer to the number of command arguments (from main),
                the contents of this parameter are modified.

         argv:  Pointer to the initial command argument list (from main),
                the contents are copied into a new array which is returned
                from this routine.

         flag:  Flag indicating how to expand wild-cards:
                   0 - Complete file name expansion
                   1 - only file name (no directory or version).
                   2 - directory info and file name (no version).
                   3 - file name and version info (no directory).
 -*/

/* Local prototypes. */
int wild_found (char *string);
char **wild_expand (const char *string, char **argv, int *argc,
                    int extra, int flag);
/*
   General note: removing the prototyping and const keywords should
   allow this code to compile with VMS 'C' compilers prior to version
   2.3-024.
*/


char **expand (int *argc, char *argv[], int flag)
/*
   Routine to expand all the arguments from main(argc,argv).  The
   return value is a pointer to a new (expanded) argv array.

   Parameters:

         argc:  Pointer to the number of command arguments (from main),
                the contents of this parameter are modified.

         argv:  Pointer to the initial command argument list (from main),
                the contents are copied into a new array which is returned
                from this routine.

         flag:  Flag indicating how to expand wild-card:
                   0 - Complete file name expansion
                   1 - only file name (no directory or version).
                   2 - directory info and file name (no version).
                   3 - file name and version info (no directory).
*/
{
   int i, nargc;
   char **nargv, **wild_expand();
   char *start, *end;

/* Get an initial amount of memory for the master nargv array. */
   if ((nargv = (char **)malloc ((*argc+1) * sizeof (char *))) == NULL) {
      fprintf (stderr, "Not enough memory to expand argument list\n");
      exit (ErrorExit);
   }

/*
   Fix the command string so that it only has the name and not the path of
   the function (more in line with what unix reports with argv[0]
*/
   start = argv[0];
   end = argv[0] + strlen (argv[0]);
   while (start < end) {
   /* Scan from the back for the first '.' and replace it with a '\0' */
      if (*end == '.') *end = '\0';
   /* And trim off path if it is found. */
      if (*end == ']') {
         end++;
         break;
      }
      --end;
   }
   nargv[0] = end;

/* Copy each argument, expanding those that have wild characters. */
   for (nargc = i = 1; i < *argc; i++) {
      if (wild_found(argv[i]))
         nargv = wild_expand(argv[i], nargv, &nargc, *argc-i, flag);
      else
         nargv[nargc++] = argv[i];
   }
   *argc = nargc;
   nargv[nargc] = NULL;  /* realloc always 0 fills, but... */

   return nargv;
}


static int wild_found (char *string)
/*
   Routine to search the given string for a VMS wild-card pattern.
   Returns 1 if "*", "%", "[-", or "...]" is found.  (This may not
   be all VMS wild-cards but it is enough for now--anyone that wants
   to recognize others can change this code.)

   Parameter:

      string: '\0' terminated character array.
*/
{
   int state = 0;

/*
   State of 0 is "rest" state.  State 1 on our way to [-, states 2-4
   on our way to ...], negative states indicate the two quotes (' -10,
   " -1).
*/
   for ( ;*string; string++) {
      switch (*string) {
      case '*':
      case '%':
         if (state >= 0)
            return 1;                    /* Unquoted % or * found. */
      break;
      case '[':
         if (state >= 0)
            state = 1;
      break;
      case ']':
         if (state == 4)
            return 1;                    /* Unquoted ...] found. */
         else if (state >= 0)
            state = 0;
      break;
      case '-':
         if (state == 1)
            return 1;                    /* Unquoted [- found. */
         else if (state >= 0)
            state = 0;
      break;
      case '.':
         if (state == 1 || state == 0)
            state = 2;                   /* First '.' */
         else if (state > 1 && state < 5)
            state++;                     /* ... == states 2, 3, 4 */
         else if (state >= 0)
            state = 0;
      break;
      case '\'':
         if (state <= -10)
            state += 10;           /* One ', possibly one " also */
         else if (state < 0)
            state -= 10;           /* 0 ', possibly one " */
         else
            state = -10;           /* No ' or " prior to this ' */
      break;
      case '"':
         if (state == -11)
            state = -10;           /* Both " and ' prior to this. */
         else if (state == -10)
            state = -11;           /* A ' prior to this. */
         else if (state == -1)
            state = 0;             /* A " prior to this. */
         else
            state = -1;            /* No ' or " prior to this " */
      break;
      }
   }
   return 0;
}


static char **wild_expand (const char *wild, char **argv,
                                        int *argc, int extra, int flag)
/*
   Routine to expand wild into new arguments appended to the end
   of argv[*argc].  This routine must realloc in order to make room
   for the individual arguments and malloc for enough space for each
   of the arguments.  The return value is a new **argv.

   Parameters:

         wild:  '\0' terminated string that needs to be expanded.

         argv:  initial starting address of the argv array.

         argc:  pointer to an integer that tells the current end of the
                argument list.

        extra:  The number of extra pointers that the returned argv
                must have available for future assignments.

         flag:  Flag indicating how to expand wild-card:
                  0 - Complete file name expansion
                  1 - only file name (no directory or version).
                  2 - directory info and file name (no version)
                  3 - file name and version info (no directory).
*/
{
   int more_to_go = 1, err, length, status, len_wild;
   char *namptr; /* , *strncpy();  Picky, but bothers other main programs */
   struct FAB fab_blk;
   struct NAMBLK nam_blk;

   len_wild = strlen(wild);

/* Initialize all the fab and nam fields needed for parse and search */

   fab_blk = cc$rms_fab;                   /* Initialize FAB structure */

   nam_blk.nam    = cc$rms_nam;            /* Initialize NAM structure */
   fab_blk.fab$l_dna = ".*";               /* Default file specif.     */
   fab_blk.fab$b_dns = 2;                  /* Length of default spec.  */
   fab_blk.fab$l_nam = &nam_blk.nam;       /* Set address of NAM in FAB*/
   nam_blk.nam.nam$b_ess = NAM$C_MAXRSS;   /* Set extended  string size*/
   nam_blk.nam.nam$l_esa = &nam_blk.es;    /* and address              */
   nam_blk.nam.nam$b_rss = NAM$C_MAXRSS;   /* Set resultant string size*/
   nam_blk.nam.nam$l_rsa = &nam_blk.rs;    /* and address              */
   nam_blk.nam.nam$l_rlf = NULL;           /* No related file address  */

   fab_blk.fab$l_fna = wild;           /* Address of file name string  */
   fab_blk.fab$b_fns = len_wild;       /* Length of file name string   */

/* Prepare to enter the search loop, parse fab. */
   err = SYS$PARSE (&fab_blk);

/* Catch the directory not found error and return no files found. */
   if (err != RMS$_NORMAL)
      exit(err);

   while (more_to_go) {
      err = SYS$SEARCH (&fab_blk);
      if (err == RMS$_NMF || err == RMS$_FNF)
         more_to_go = 0;               /* Done, no more files found */
      else if (err != RMS$_NORMAL)
         exit (err);
      else {
      /* Count that we now have this many arguments. */
         (*argc)++;

      /* Make sure there is room for a new pointer. */
         if ((argv = realloc (argv, (*argc + extra)*sizeof(char *))) == NULL) {
            fprintf (stderr, "Not enough memory to expand argument list\n");
            exit(ErrorExit);
         }

      /* Move the right name into the list. */
         switch (flag) {
         case 0:             /* Complete file name */
            length = nam_blk.nam.nam$b_rsl;
            namptr = &nam_blk.rs;
            break;
         case 1:            /* File name only (no directory or version). */
            length = nam_blk.nam.nam$b_name + nam_blk.nam.nam$b_type;
            namptr = nam_blk.nam.nam$l_name;
            break;
         case 2:            /* directory and file name (no version) */
            length = nam_blk.nam.nam$b_rsl - nam_blk.nam.nam$b_ver;
            namptr = &nam_blk.rs;
            break;
         case 3:            /* File name and version (no directory). */
            length = nam_blk.nam.nam$b_name +
                     nam_blk.nam.nam$b_type +
                     nam_blk.nam.nam$b_ver;
            namptr = nam_blk.nam.nam$l_name;
            break;
         default:
            fprintf (stderr, "illegal flag used in VMS expand call\n");
            exit (ErrorExit);
         }
      /* Copy the requested string into the argument array. */
         if ((argv[*argc-1] = malloc (length+1)) == NULL) {
            fprintf (stderr, "Not enough memory to expand argument list\n");
            exit (ErrorExit);
         }
         (void )strncpy (argv[*argc-1], namptr, length);
         argv[*argc-1][length] = '\0';
      }
   }
   return (argv);
}

/* Remove all the defines that might affect the user's code. */

#undef redirect
#undef filearg
#undef expand
#undef wild_found
#undef wild_expand

#ifdef __TST_ECHO     /* Example code using expand(). */

# ifndef __FILE
#include stdio
#endif

main(argc, argv)
int argc;
char *argv[];
/*
   This main program allows you to run experiments with ``expand()''.
   Try $ echo *.*, $ echo -f1 [-...]*.*, $ echo -f[0-3] *.*.
   Questions about using "%", "\", etc. may be answered by testing
   with this version of echo.

   To use this, of course, you need to link directly with expand--
   avoiding the substitution of main with _user_main above.
*/
{
    int i, flag=0;
    char **expand();

    for(i=1; i<argc; i++)
        printf("%s %c", argv[i], (i<argc-1) ? '  ':'\n');

    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'f')
       flag = atoi (&argv[1][2]);

    argv = expand (&argc, argv, flag);

    printf ("\n\n");
    for(i=1; i<argc; i++)
        printf("%s %c", argv[i], flag%2 == 0 ? '\n' : i%4 == 0 ? '\n':'\t');

}
#endif  /* __TST_ECHO */
