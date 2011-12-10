/* xtool.h */

#define MAXTOOLS	50

typedef struct
{
	Window		win;		/* toolbar subwindow */
	int		x, y;		/* position of toolbar within window */
	unsigned int	w, h;		/* overall size of the toolbar */
	unsigned long	bg;		/* background color of toolbar */
	unsigned long	face;		/* color of button faces */
	unsigned long	fglabel;	/* color of button labels */
	struct
	{
		int	x, y;		/* position of a button within toolbar*/
		int	bevel;		/* bevel height of a button */
	}		state[MAXTOOLS];/* info about each button */
} X_TOOLBAR;

CHAR *x_tb_dump P_((char *label));
void x_tb_predict P_((X11WIN *xw, unsigned w, unsigned h));
void x_tb_create P_((X11WIN *xw, int x, int y));
void x_tb_destroy P_((X11WIN *xw));
void x_tb_draw P_((X11WIN *xw, BOOLEAN fromscratch));
void x_tb_event P_((X11WIN *xw, XEvent *event));
void x_tb_config P_((BOOLEAN gap, char *label, _char_ op, char *value));
void x_tb_recolor P_((X11WIN *xw, _char_ font));
