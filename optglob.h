/* optglob.h */
/* Copyright 1995 by Steve Kirkendall */


/* This function declares global options, and macros for accessing them */

extern OPTVAL optglob[];
#ifdef FEATURE_LPR
extern OPTVAL lpval[];
#endif

#define o_blksize		optglob[0].value.number
#define o_blkhash		optglob[1].value.number
#define o_blkcache		optglob[2].value.number
#define o_blkgrow		optglob[3].value.number
#define o_blkfill		optglob[4].value.number
#define o_blkhit		optglob[5].value.number
#define o_blkmiss		optglob[6].value.number
#define o_blkwrite		optglob[7].value.number
#define o_version		optglob[8].value.string
#define o_bitsperchar		optglob[9].value.number
#define o_gui			optglob[10].value.string
#define o_os			optglob[11].value.string
#define o_session		optglob[12].value.string
#define o_recovering		optglob[13].value.boolean
#define o_digraph		optglob[14].value.boolean
#define o_exrc			optglob[15].value.boolean
#define o_modeline		optglob[16].value.boolean
#define o_modelines		optglob[17].value.number
#define o_ignorecase		optglob[18].value.boolean
#define o_magic			optglob[19].value.boolean
#define o_novice		optglob[20].value.boolean
#define o_prompt		optglob[21].value.boolean
#define o_remap			optglob[22].value.boolean
#define o_report		optglob[23].value.number
#define o_shell			optglob[24].value.string
#define o_sync			optglob[25].value.boolean
#define o_taglength		optglob[26].value.number
#define o_tags			optglob[27].value.string
#define o_tagstack		optglob[28].value.boolean
#define o_tagprg		optglob[29].value.string
#define o_autoprint		optglob[30].value.boolean
#define o_autowrite		optglob[31].value.boolean
#define o_autoselect		optglob[32].value.boolean
#define o_warn			optglob[33].value.boolean
#define o_window		optglob[34].value.number
#define o_wrapscan		optglob[35].value.boolean
#define o_writeany		optglob[36].value.boolean
#define o_defaultreadonly	optglob[37].value.boolean
#define o_initialstate		optglob[38].value.character
#define o_exitcode		optglob[39].value.number
#define o_keytime		optglob[40].value.number
#define o_usertime		optglob[41].value.number
#define o_safer			optglob[42].value.boolean
#define o_tempsession		optglob[43].value.boolean
#define o_newsession		optglob[44].value.boolean
#define o_exrefresh		optglob[45].value.boolean
#define o_home			optglob[46].value.string
#define o_elvispath		optglob[47].value.string
#define o_terse			optglob[48].value.boolean
#define o_previousdir		optglob[49].value.string
#define o_previousfile		optglob[50].value.string
#define o_previousfileline	optglob[51].value.number
#define o_previouscommand	optglob[52].value.string
#define o_previoustag		optglob[53].value.string
#define o_nearscroll		optglob[54].value.number
#define o_optimize		optglob[55].value.boolean
#define o_edcompatible		optglob[56].value.boolean
#define o_pollfrequency		optglob[57].value.number
#define o_sentenceend		optglob[58].value.string
#define o_sentencequote		optglob[59].value.string
#define o_sentencegap		optglob[60].value.number
#define o_verbose		optglob[61].value.number
#define o_anyerror		optglob[62].value.boolean
#define o_directory		optglob[63].value.string
#define o_errorbells		optglob[64].value.boolean
#define o_warningbells		optglob[65].value.boolean
#define o_flash			optglob[66].value.boolean
#define o_program		optglob[67].value.string
#define o_backup		optglob[68].value.boolean
#define o_showmarkups		optglob[69].value.boolean
#define o_nonascii		optglob[70].value.character
#define o_beautify		optglob[71].value.boolean
#define o_mesg			optglob[72].value.boolean
#define o_sessionpath		optglob[73].value.string
#define o_maptrace		optglob[74].value.character
#define o_maplog		optglob[75].value.character
#define o_gdefault		optglob[76].value.boolean
#define o_matchchar		optglob[77].value.string
#define o_showname		optglob[78].value.string
#define o_writeeol		optglob[79].value.character
#define o_showtag		optglob[80].value.boolean
#define o_saveregexp		optglob[81].value.boolean
#define o_true			optglob[82].value.string
#define o_false			optglob[83].value.string
#define o_animation		optglob[84].value.number
#define o_completebinary	optglob[85].value.boolean
#define o_optionwidth		optglob[86].value.number
#define o_smarttab		optglob[87].value.boolean
/* For backward compatibility with older releases of elvis : */
#define o_more    		optglob[88].value.boolean
#define o_timeout 		optglob[89].value.boolean
#define o_hardtabs		optglob[90].value.number
#define o_redraw		optglob[91].value.boolean
#define QTY_GLOBAL_OPTS			92

#ifdef FEATURE_LPR
# define o_lptype		lpval[0].value.string
# define o_lpcrlf		lpval[1].value.boolean
# define o_lpout		lpval[2].value.string
# define o_lpcolumns		lpval[3].value.number
# define o_lpwrap		lpval[4].value.boolean
# define o_lplines		lpval[5].value.number
# define o_lpconvert		lpval[6].value.boolean
# define o_lpformfeed		lpval[7].value.boolean
# define o_lppaper		lpval[8].value.string
# define o_lpnumber		lpval[9].value.boolean
# define o_lpheader		lpval[10].value.boolean
# define o_lpcolor		lpval[11].value.boolean
# define QTY_LP_OPTS		      12
#endif

BEGIN_EXTERNC
extern void optglobinit P_((void));
extern void optprevfile P_((CHAR *name, long line));
END_EXTERNC
