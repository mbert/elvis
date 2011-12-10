/*
** wintools.h  --  various windows tools.
**
** Copyright 1996, Serge Pirotte.
*/

#if !defined (WINTOOLS_H)
#define WINTOOLS_H

extern int         cmd_line_argc;
extern char        **cmd_line_argv;

extern void parse_cmd_line (char *cmdp);
extern void center_window (HWND parent_window, HWND this_window);

#endif

