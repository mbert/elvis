/* need.c */
/* Copyright 1995 by Steve Kirkendall */

char id_need[] = "$Id: need.c,v 2.1 1996/07/02 19:34:50 steve Exp $";

#include "elvis.h"


#ifdef NEED_STRDUP
# if USE_PROTOTYPES
char *strdup(const char *str)
{
# else /* don't USE_PROTOTYPES */
char *strdup(str)
	char	*str;
{
# endif /* don't USE_PROTOTYPES */

	char	*ret;

	ret = (char *)safealloc(strlen(str) + 1, sizeof(char));
	strcpy(ret, str);
	return ret;
}
#endif /* NEED_STRDUP */
