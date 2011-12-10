/* guivio.h */

/*
 * Interface for VIO (console) user interface for OS/2. Work by Lee Johnson
 * except a few fixes by herbert.
 *
 * herbert: 
 * - added macro stuff to avoid trouble when compiling with emx/gcc.
 * - addes "static" qualifiers for function prototypes where necessary.
 */

#if !defined(_GUIVIO_H)
#define _GUIVIO_H

/*
 * A VIO color index.  The four high-order bits contain the background
 * color, while the four low-order bits contain the foreground colour.
 * Unlike the ANSI console, 'bright' colours may be used for the
 * background, and all 256 combinations are valid (though 16 of them
 * lead to invisible text. :-)
 */
typedef unsigned char VCOLOR;

/* Structs of this type are used to remember the location and size of each 
 * window.  In the VIO interface, all windows must be as wide as the
 * screen, and the sum of all windows' heights must equal the screen size.
 */
typedef struct vwin_s
{
    struct vwin_s   *next;          /* some other window on this screen */
    int             height;         /* size of the window */
    int             pos;            /* position of the window */
    int             newheight;      /* height after screen is rearranged */
    int             newpos;         /* position after screen is rearranged */
    int             cursx, cursy;   /* logical cursor position */
    ELVCURSOR       shape;          /* logical cursor shape */

    /*
     * The following buffers hold the colour indices for the various
     * fonts.
    VCOLOR  vc_normal;
    VCOLOR  vc_fixed;
    VCOLOR  vc_bold;
    VCOLOR  vc_emph;
    VCOLOR  vc_italic;
    VCOLOR  vc_underline;
    VCOLOR  vc_hilite;
     */
} VWIN;

/*
 * Macro definitions:
 */
#define PM_RequestUpdate(pw) \
    { if (!(pw)->destroyed) (pw)->update_me = ElvTrue; }

/*
 * GUI function declarations.  (Not all of these are actually defined.)
 */
static int     vio_test P_((void));
static int     vio_init P_((int argc, char **argv));
static void    vio_loop P_((void));
static ELVBOOL vio_poll P_((ELVBOOL reset));
static void    vio_term P_((void));
static ELVBOOL vio_creategw P_((char *name, char *attributes));
static void    vio_destroygw P_((GUIWIN *gw, ELVBOOL force));
static ELVBOOL vio_focusgw P_((GUIWIN *gw));
static void    vio_reset P_((void));
static void    vio_flush P_((void));
static void    vio_moveto P_((GUIWIN *gw, int column, int row));
static void    vio_draw P_((GUIWIN *gw, long fg, long bg, 
                                int bits, CHAR *text, int len));
static ELVBOOL vio_shift P_((GUIWIN *gw, int qty, int rows));
static ELVBOOL vio_scroll P_((GUIWIN *gw, int qty, ELVBOOL notlast));
static ELVBOOL vio_clrtoeol P_((GUIWIN *gw));
static void    vio_beep P_((GUIWIN *gw));
static ELVBOOL vio_color P_((int fontcode, CHAR *name, 
                        ELVBOOL isfg, long *colorptr, unsigned char *rgb));
static ELVBOOL vio_tabcmd P_((GUIWIN *gw, _CHAR_ key2, long count));
static ELVBOOL vio_prgopen P_((char *command, ELVBOOL willwrite, ELVBOOL willread));
static int     vio_prgclose P_((void));

/* These functions aren't in the Elvis GUI--but they should be. :-) */

#endif /* _GUIVIO_H */
