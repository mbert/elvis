/* optglob.h */
/* Copyright 1995 by Steve Kirkendall */


/* This function declares global options, and macros for accessing them */

extern OPTVAL optglob[];
extern OPTVAL lpval[];

#define o_blksize		optglob[0].value.number
#define o_blkhash		optglob[1].value.number
#define o_blkcache		optglob[2].value.number
#define o_blkgrow		optglob[3].value.number
#define o_blkfill		optglob[4].value.number
#define o_version		optglob[5].value.string
#define o_bitsperchar		optglob[6].value.number
#define o_gui			optglob[7].value.string
#define o_os			optglob[8].value.string
#define o_session		optglob[9].value.string
#define o_recovering		optglob[10].value.boolean
#define o_digraph		optglob[11].value.boolean
#define o_exrc			optglob[12].value.boolean
#define o_modeline		optglob[13].value.boolean
#define o_modelines		optglob[14].value.boolean
#define o_ignorecase		optglob[15].value.boolean
#define o_magic			optglob[16].value.boolean
#define o_novice		optglob[17].value.boolean
#define o_prompt		optglob[18].value.boolean
#define o_remap			optglob[19].value.boolean
#define o_report		optglob[20].value.number
#define o_shell			optglob[21].value.string
#define o_sync			optglob[22].value.boolean
#define o_taglength		optglob[23].value.number
#define o_tags			optglob[24].value.string
#define o_tagstack		optglob[25].value.boolean
#define o_autoprint		optglob[26].value.boolean
#define o_autowrite		optglob[27].value.boolean
#define o_autoselect		optglob[28].value.boolean
#define o_warn			optglob[29].value.boolean
#define o_window		optglob[30].value.number
#define o_wrapscan		optglob[31].value.boolean
#define o_writeany		optglob[32].value.boolean
#define o_defaultreadonly	optglob[33].value.boolean
#define o_initialstate		optglob[34].value.character
#define o_exitcode		optglob[35].value.number
#define o_keytime		optglob[36].value.number
#define o_usertime		optglob[37].value.number
#define o_safer			optglob[38].value.boolean
#define o_tempsession		optglob[39].value.boolean
#define o_newsession		optglob[40].value.boolean
#define o_exrefresh		optglob[41].value.boolean
#define o_home			optglob[42].value.string
#define o_elvispath		optglob[43].value.string
#define o_terse			optglob[44].value.boolean
#define o_previousfile		optglob[45].value.string
#define o_previousfileline	optglob[46].value.number
#define o_previouscommand	optglob[47].value.string
#define o_previoustag		optglob[48].value.string
#define o_nearscroll		optglob[49].value.number
#define o_optimize		optglob[50].value.boolean
#define o_edcompatible		optglob[51].value.boolean
#define o_pollfrequency		optglob[52].value.number
#define o_sentenceend		optglob[53].value.string
#define o_sentencequote		optglob[54].value.string
#define o_sentencegap		optglob[55].value.number
#define o_verbose		optglob[56].value.boolean
#define o_anyerror		optglob[57].value.boolean
#define o_directory		optglob[58].value.string
#define o_errorbells		optglob[59].value.boolean
#define o_warningbells		optglob[60].value.boolean
#define o_flash			optglob[61].value.boolean
#define o_program		optglob[62].value.string
#define o_backup		optglob[63].value.boolean
#define o_showmarkups		optglob[64].value.boolean
#define o_nonascii		optglob[65].value.character
#define o_beautify		optglob[66].value.boolean
#define o_mesg			optglob[67].value.boolean
#define o_sessionpath		optglob[68].value.string
#define o_maptrace		optglob[69].value.character
#define o_maplog		optglob[70].value.character
#define o_gdefault		optglob[71].value.boolean
#define QTY_GLOBAL_OPTS			72

#define o_lptype		lpval[0].value.string
#define o_lpcrlf		lpval[1].value.boolean
#define o_lpout			lpval[2].value.string
#define o_lpcolumns		lpval[3].value.number
#define o_lpwrap		lpval[4].value.boolean
#define o_lplines		lpval[5].value.number
#define o_lpconvert		lpval[6].value.boolean
#define o_lpformfeed		lpval[7].value.boolean
#define o_lppaper		lpval[8].value.string
#define QTY_LP_OPTS		      9

BEGIN_EXTERNC
extern void optglobinit P_((void));
extern void optprevfile P_((CHAR *name, long line));
END_EXTERNC
