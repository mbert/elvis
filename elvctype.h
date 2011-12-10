/* elvctype.h */

#ifndef isupper
# define ELVCT_UPPER	0x01
# define ELVCT_LOWER	0x02
# define ELVCT_DIGIT	0x04
# define ELVCT_XDIGIT	0x08
# define ELVCT_SPACE	0x10
# define ELVCT_PUNCT	0x20
# define ELVCT_CNTRL	0x40

# define ELVCT_ALPHA	(ELVCT_UPPER|ELVCT_LOWER)
# define ELVCT_ALNUM	(ELVCT_ALPHA|ELVCT_DIGIT)
# define ELVCT_PRINT	(ELVCT_ALNUM|ELVCT_PUNCT)

# define isupper(c)	((elvct_class[(CHAR)(c)] & ELVCT_UPPER) != 0)
# define islower(c)	((elvct_class[(CHAR)(c)] & ELVCT_LOWER) != 0)
# define isdigit(c)	((elvct_class[(CHAR)(c)] & ELVCT_DIGIT) != 0)
# define isxdigit(c)	((elvct_class[(CHAR)(c)] & ELVCT_XDIGIT) != 0)
# define isspace(c)	((elvct_class[(CHAR)(c)] & ELVCT_SPACE) != 0)
# define ispunct(c)	((elvct_class[(CHAR)(c)] & ELVCT_PUNCT) != 0)
# define isprint(c)	((elvct_class[(CHAR)(c)] & ELVCT_PRINT) != 0)
# define iscntrl(c)	((elvct_class[(CHAR)(c)] & ELVCT_CNTRL) != 0)
# define isalpha(c)	((elvct_class[(CHAR)(c)] & ELVCT_ALPHA) != 0)
# define isalnum(c)	((elvct_class[(CHAR)(c)] & ELVCT_ALNUM) != 0)
# define isprint(c)	((elvct_class[(CHAR)(c)] & ELVCT_PRINT) != 0)

# define setupper(c)	(elvct_class[(CHAR)(c)] |= ELVCT_UPPER)
# define setlower(c)	(elvct_class[(CHAR)(c)] |= ELVCT_LOWER)
# define setpunct(c)	(elvct_class[(CHAR)(c)] |= ELVCT_PUNCT)

# define clrupper(c)	(elvct_class[(CHAR)(c)] &= ~ELVCT_UPPER)
# define clrlower(c)	(elvct_class[(CHAR)(c)] &= ~ELVCT_LOWER)
# define clrpunct(c)	(elvct_class[(CHAR)(c)] &= ~ELVCT_PUNCT)

# define toupper(c)	elvct_upper[(CHAR)(c)]
# define tolower(c)	elvct_lower[(CHAR)(c)]

extern CHAR elvct_upper[256];
extern CHAR elvct_lower[256];
extern CHAR elvct_class[256];

#endif /* ndef isupper */
