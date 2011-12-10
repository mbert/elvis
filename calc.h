/* calc.h */
/* Copyright 1995 by Steve Kirkendall */

BEGIN_EXTERNC
extern BOOLEAN calcnumber P_((CHAR *value));
extern BOOLEAN calcbase10 P_((CHAR *value));
extern BOOLEAN calctrue P_((CHAR *value));
extern CHAR *calculate P_((CHAR *expr, CHAR **arg, BOOLEAN asmsg));
extern BOOLEAN calcsel P_((MARK from, MARK to));
END_EXTERNC
