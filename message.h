/* msg.h */
/* Copyright 1995 by Steve Kirkendall */




/* This data type is used to denote the importance and class of a message. */
typedef enum
{
	MSG_STATUS,	/* e.g., "Reading file..." */
	MSG_INFO,	/* e.g., "Read file, 20 lines, 187 characters" */
	MSG_WARNING,	/* e.g., "More files to edit" */
	MSG_ERROR,	/* e.g., "Unknown command" */
	MSG_FATAL	/* e.g., "Error writing to session file" */
} MSGIMP;


/* This function is used to display messages.  It translates terse messages
 * into verbose message via the "elvis messages" buffer, and then calls the
 * GUI's msg() function with the expanded text.
 */
BEGIN_EXTERNC
extern void msg P_((MSGIMP imp, char *terse, ...));

/* This function flushes queued message to the screen */
extern void msgflush P_((void));
END_EXTERNC
