/* options.h */
/* Copyright 1995 by Steve Kirkendall */


/* This file contains extern declarations for the option maniplulation
 * functions.
 */

/* These values are ORed together in the "flags" field of an option value
 * to inform elvis how to treat it.  Initially, all flags should be 0.
 * Elvis maintains the "set" and "show" flags itself; the "save" flag is'
 * maintained by each option's store() function.
 */
#define OPT_SET		0x01	/* value has been changed */
#define OPT_SHOW	0x02	/* value should be shown */
#define OPT_HIDE	0x04	/* value should be hidden even if ":set all" */
#define OPT_LOCK	0x08	/* value can never be changed */
#define OPT_UNSAFE	0x10	/* value can't be changed if "safer" is set */
#define OPT_FREE	0x20	/* call safefree() on the value before freeing/changing it */
#define OPT_REDRAW	0x40	/* changing this option forces a redraw */

/* Instances of this structure are used to store values of variables.  Values
 * are stored apart from their descriptions because for some options the
 * values will vary for each window or buffer, but their descriptions won't.
 */
typedef struct
{
	union
	{
		long	number;		/* for numeric options */
		BOOLEAN	boolean;	/* for boolean options */
		char	character;	/* for "one of" options */
		CHAR	*string;	/* for string options */
		void	*pointer;	/* for any other type of option */
	} value;
	short flags;		/* flags for the option */
} OPTVAL;

/* Instances of this structure are used to describe options. */
typedef struct optdesc_s
{
   char	*longname, *shortname;
   CHAR *(*asstring) P_((struct optdesc_s *opt, OPTVAL *val));
   int	(*isvalid) P_((struct optdesc_s *opt, OPTVAL *val, CHAR *newval));
   char *limit;
   int	(*store) P_((struct optdesc_s *opt, OPTVAL *val, CHAR *newval));
} OPTDESC;


/* This macro returns the flags field of an optval, when given the lvalue
 * of the value field.  For example, optflags(o_blksize) accesses the flags
 * of the blksize option.
 */
#define optflags(o)		(((OPTVAL *)&(o))->flags)


/* This macro sets the value & flags of an option.  For example, the macro
 * optpreset(o_session, "session.elv", OPT_HIDE) sets the value of the
 * "session" option "session.elv", and turns on its OPT_HIDE flag.
 */
#define optpreset(o, v, f)	((o) = (v), optflags(o) |= (f))


/* These functions are used to say which particular options are relevent at
 * the moment.
 */
BEGIN_EXTERNC
extern void optinsert P_((char *domain, int nopts, OPTDESC desc[], OPTVAL val[]));
extern void optdelete P_((OPTVAL val[]));
extern void optfree P_((int nopts, OPTVAL *val));
extern int optisnumber P_((OPTDESC *desc, OPTVAL *val, CHAR *newval));
extern int optisstring P_((OPTDESC *desc, OPTVAL *val, CHAR *newval));
extern int optisoneof P_((OPTDESC *desc, OPTVAL *val, CHAR *newval));
extern CHAR *optnstring P_((OPTDESC *desc, OPTVAL *val));
extern CHAR *optsstring P_((OPTDESC *desc, OPTVAL *val));
extern CHAR *opt1string P_((OPTDESC *desc, OPTVAL *val));
extern BOOLEAN optset P_((BOOLEAN bang, CHAR *ARGS, CHAR *OUTBUF, size_t outsize));
extern CHAR *optgetstr P_((CHAR *name));
extern BOOLEAN optputstr P_((CHAR *name, CHAR *value));
extern CHAR *optname P_((CHAR *name));
#if defined (GUI_WIN32)
extern int optiswinsize (OPTDESC *desc, OPTVAL *val, CHAR *newval);
#endif
END_EXTERNC
