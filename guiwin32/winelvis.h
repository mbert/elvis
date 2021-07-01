/*
** winelvis.h  --  MS-Windows support for elvis.
**
** Copyright 1996, Serge Pirotte.
*/

#if !defined (WINELVIS_H)
#define WINELVIS_H

struct vkey {
   char        *label;
   char        *cooked;
   MAPFLAGS    flags;
   char        *rawin;
};

struct gwopts {
    OPTVAL          scrollbar;
    OPTVAL          toolbar;
    OPTVAL          statusbar;
    OPTVAL          menubar;
    OPTVAL          font;
    OPTVAL	    propfont;
    OPTVAL	    titleformat;
    OPTVAL	    scrollbgimage;
    OPTVAL	    iconimage;
};

typedef struct GUI_WINDOW {
    struct GUI_WINDOW   *nextp;
    HWND                frameHWnd;
    HWND                clientHWnd;
    HWND                toolbarHWnd;
    HWND                statusbarHWnd;
    HMENU               menuHndl;
    COLORREF		bg;
    long                scrolled;
    int                 active;
    int                 currow;
    int                 curcol;
    int                 numrows;
    int                 numcols;
    int                 xcsize;
    int                 ycsize;
    int                 xsize;
    int                 ysize;
    int                 font_size_array[256];
    long                thumbsize;
    long                scrollsize;
    ELVCURSOR           cursor_type;
    int                 caret;
    HFONT               fonts[8];
    struct gwopts       options;
} GUI_WINDOW;

typedef struct
{
	char				*name;
	unsigned char		rgb[3];
} GUI_COLORTBL;

#define o_scrollbar(p)       (p)->options.scrollbar.value.boolean
#define o_toolbar(p)         (p)->options.toolbar.value.boolean
#define o_statusbar(p)       (p)->options.statusbar.value.boolean
#define o_menubar(p)         (p)->options.menubar.value.boolean
#define o_font(p)            (p)->options.font.value.string
#define o_propfont(p)        (p)->options.propfont.value.string
#define o_titleformat(p)     (p)->options.titleformat.value.string
#define o_scrollbgimage(p)   (p)->options.scrollbgimage.value.boolean

#define NUM_OPTIONS     8
#define DEFAULT_FONT    "Courier new*10"
#define DEFAULT_PROPFONT "Times*10"
#define DEFAULT_TITLEFORMAT "$1"

extern GUI_WINDOW       gw_def_win;
extern HINSTANCE        hInst;
extern PRINTDLG         gwpdlg;
extern int              gw_printing_ok;
extern HCURSOR          hLeftArrow;
extern HCURSOR          hRightArrow;
extern HCURSOR          selectedCursor;
extern GUI_COLORTBL	    colortbl[];
extern HICON            gwcustomicon;

/* ---[ function prototypes ]------------------------------------------ */

extern LRESULT gwcmd (GUI_WINDOW *gwp, WPARAM wParam);

extern BOOL CALLBACK DlgAbout (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern BOOL CALLBACK DlgGoto (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern BOOL CALLBACK DlgSearch (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern BOOL CALLBACK DlgSearch (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern BOOL CALLBACK DlgReplace (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern BOOL CALLBACK DlgRun (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern BOOL CALLBACK DlgTags (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern BOOL CALLBACK DlgOptGui (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern BOOL CALLBACK DlgOptBuffer (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern BOOL CALLBACK DlgOptGlobal (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern BOOL CALLBACK DlgOptWindow (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern BOOL CALLBACK DlgOptUser (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern void opt_parse_font (unsigned char *option, LOGFONT *plf);
extern void opt_parse_attr (unsigned char *option, LOGFONT *plf);
extern int optstoresb (OPTDESC *opt, OPTVAL *val, Char *newval);
extern int optstoretb (OPTDESC *opt, OPTVAL *val, Char *newval);
extern int optstorestb (OPTDESC *opt, OPTVAL *val, Char *newval);
extern int optstoremnu (OPTDESC *opt, OPTVAL *val, Char *newval);
extern int optisfont (OPTDESC *opt, OPTVAL *val, Char *newval);
extern int optisicon (OPTDESC *opt, OPTVAL *val, Char *newval);
extern int optstoreattr (OPTDESC *opt, OPTVAL *val, Char *newval);

extern void gw_disable_printing (GUI_WINDOW *gwp);
extern void gw_get_default_printer (void);
extern void gw_set_default_printer (char *pDevice, char *pDriver,
                                    char *pOutput);

extern LRESULT gwframe_WM_GETMINMAXINFO (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwframe_WM_INITMENU (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwframe_WM_MENUSELECT (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwframe_WM_NOTIFY (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwframe_WM_PAINT (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwframe_WM_SIZE (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwframe_WM_SETFOCUS (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_CHAR (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_DROPFILES (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_ERASEBKGND (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_KEYDOWN (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_KEYUP (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_KILLFOCUS (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_LBUTTONDBLCLK (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_LBUTTONDOWN (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_LBUTTONUP (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_MOUSEMOVE (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_MOUSEWHEEL (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_PAINT (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_RBUTTONDBLCLK (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_RBUTTONDOWN (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_RBUTTONUP (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_SETFOCUS (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_SIZE (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_SYSKEYDOWN (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);
extern LRESULT gwclient_WM_VSCROLL (GUI_WINDOW *gwp, WPARAM wParam, LPARAM lParam);

extern void gw_create_status_bar (GUI_WINDOW *gwp);
extern void gw_destroy_status_bar (GUI_WINDOW *gwp);
extern int gw_size_status_bar (GUI_WINDOW *gwp, RECT *parentRect, int maximized);
extern void gw_upd_status_bar (GUI_WINDOW *gwp, Char *cmd, long row, long col, char learn, char *mode);
extern void gw_upd_status_bar_ind (GUI_WINDOW *gwp, int num_state, int cap_state);
extern void gw_status_bar_text (GUI_WINDOW *gwp, char *str);

extern void gw_create_toolbar (GUI_WINDOW *gwp);
extern void gw_destroy_toolbar (GUI_WINDOW *gwp);
extern int gw_size_toolbar (GUI_WINDOW *gwp, RECT *rect);
extern void gw_toolbar_tooltip (GUI_WINDOW *gwp, LPARAM lParam);

extern GUI_WINDOW *gw_find_client (HWND hWnd);
extern GUI_WINDOW *gw_find_frame (HWND hWnd);
extern void gw_get_win_size (GUI_WINDOW *gwp);
extern int gw_set_win_size (GUI_WINDOW *gwp, int check);
extern void gw_set_fonts (GUI_WINDOW *gwp);
extern void gw_set_cursor (GUI_WINDOW *gwp, BOOLEAN chgshape);
extern void gw_del_fonts (GUI_WINDOW *gwp);
extern void gw_redraw_win (GUI_WINDOW *gwp);

# ifdef FEATURE_IMAGE
extern HBITMAP normalimage, idleimage;

extern HBITMAP gw_load_xpm(char *filename, long tint, long *average, HBITMAP *mask);
extern void gw_unload_xpm(HBITMAP bitmap);
extern void gw_erase_rect(HDC hdc, RECT *rect, HBITMAP bitmap, long scrolled);
#endif

#endif

