/* xmisc.h */

typedef struct x_loadedfont_s
{
	struct x_loadedfont_s *next;	/* next font in linked list */
	int		links;		/* number of windows using this font */
	XFontStruct	*fontinfo;	/* X font structure */
	char		*name;		/* name of the font */
	int		height;		/* height of font (ascent + descent) */
} X_LOADEDFONT;

typedef struct x_loadedcolor_s
{
	struct x_loadedcolor_s *next;	/* next color in linked list */
	int		links;		/* number of uses for this color */
	unsigned long	pixel;		/* the color code */
	CHAR		*name;		/* name of the color */
} X_LOADEDCOLOR;

X_LOADEDFONT *x_loadfont P_((char *name));
void x_unloadfont P_((X_LOADEDFONT *font));
unsigned long x_loadcolor P_((CHAR *name, unsigned long def));
void x_unloadcolor P_((unsigned long pixel));
void x_drawbevel P_((X11WIN *xw, Window win, int x, int y, unsigned w, unsigned h, _char_ dir, int height));
void x_drawstring P_((Display *display, Window win, GC gc, int x, int y, char *str, int len));
