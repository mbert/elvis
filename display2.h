/* display2.h */
/* Copyright 1995 by Steve Kirkendall */


struct dispmode_s
{
	char	*name;
	char	*desc;
	BOOLEAN	canopt;
	BOOLEAN	wordwrap;
	int	nwinopts;
	OPTDESC	*winoptd;
	int	nglobopts;
	OPTDESC	*globoptd;
	OPTVAL	*globoptv;
	DMINFO	*(*init) P_((WINDOW win));
	void	(*term) P_((DMINFO *info));
	long	(*mark2col) P_((WINDOW w, MARK mark, BOOLEAN cmd));
	MARK	(*move) P_((WINDOW w, MARK from, long linedelta, long column, BOOLEAN cmd));
	MARK	(*wordmove) P_((MARK from, long count, BOOLEAN backward, BOOLEAN whitespace));
	MARK	(*setup) P_((MARK top, long cursor, MARK bottom, DMINFO *info));
	MARK	(*image) P_((WINDOW w, MARK line, DMINFO *info,
			void (*draw)(CHAR *p, long qty, _char_ font, long offset)));
	void	(*header) P_((WINDOW w, int pagenum, DMINFO *info,
			void (*draw)(CHAR *p, long qty, _char_ font, long offset)));
	void	(*indent) P_((WINDOW w, MARK line, long linedelta));
	CHAR	*(*tagatcursor) P_((WINDOW win, MARK cursor));
	MARK	(*tagload) P_((CHAR *tagname, MARK from));
	MARK	(*tagnext) P_((MARK cursor));
};

extern DISPMODE	dmnormal;
#ifdef DISPLAY_HEX
extern DISPMODE	dmhex;
#endif
#ifdef DISPLAY_MARKUP
extern DISPMODE	dmhtml;
extern DISPMODE	dmman;
#endif
#ifdef DISPLAY_SYNTAX
extern DISPMODE dmsyntax;
#endif
extern DISPMODE	*allmodes[];

BEGIN_EXTERNC
extern void	displist P_((WINDOW win));
extern BOOLEAN	dispset P_((WINDOW win, char *newmode));
extern void	dispinit P_((BOOLEAN before));
extern void	dispoptions P_((DISPMODE *mode, DMINFO *info));
extern MARK	dispmove P_((WINDOW win, long linedelta, long wantcol));
extern long	dispmark2col P_((WINDOW win));
extern void	dispindent P_((WINDOW w, MARK line, long linedelta));

#ifdef DISPLAY_MARKUP
extern void	dmmuadjust P_((MARK from, MARK to, long delta));
#endif
#ifdef DISPLAY_SYNTAX
extern BOOLEAN	dmsknown P_((char *filename));
#endif
END_EXTERNC
