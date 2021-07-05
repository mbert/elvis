/*
** WinElvis.c  --  MS-Windows port of elvis.
**
** Copyright 1996, Serge Pirotte.
*/

#define CHAR    Char
#include "elvis.h" 
#undef CHAR
#undef ELVBOOL 

#if defined (GUI_WIN32)

#define _WIN32_WINNT    0x400
#include <windows.h>
#include <commctrl.h>
#include "winelvis.h"
#include "elvisres.h"
#include "wintools.h"
#if _MSC_VER <= 900
#if defined (USE_CTL3D)
#include <ctl3d.h>
#endif
#endif

extern void main (int argc, char **argv);

HINSTANCE       hInst;
HCURSOR         hLeftArrow;
HCURSOR         hRightArrow;
HCURSOR         selectedCursor;

#if _MSC_VER <= 900
#if defined (USE_CTL3D)
HANDLE      ctl3d_handle;
#endif
#endif

static int init_application (HINSTANCE hInst);
static int init_instance (HINSTANCE hInst);
static void exit_instance (HINSTANCE hInst);
static LRESULT APIENTRY FrameWndProc (HWND hwnd, UINT msg,
                                      WPARAM wParam, LPARAM lParam);
static LRESULT APIENTRY ClientWndProc (HWND hwnd, UINT msg,
                                       WPARAM wParam, LPARAM lParam);

/* ---[ WinMain ]------------------------------------------------------ */

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow)

{
	hInst = hInstance;

	/* register the window classes */
	if (!hPrevInstance)
		if (!init_application (hInstance))
			return FALSE;

	/* instance specific initialization */
	if (!init_instance (hInstance))
		return FALSE;

	/* parse the command line */
	parse_cmd_line (lpCmdLine);

	/* call elvis */
	main (cmd_line_argc, cmd_line_argv);

	/* instance specific cleanup */
	exit_instance (hInstance);

	return 0;
}

/* ---[ init_application ]--------------------------------------------- */

static int init_application (HINSTANCE hInst)

{
	WNDCLASS    wndclass;

	/* find common controls library */
	InitCommonControls ();

	/* get cursors */
	hLeftArrow = LoadCursor (NULL, IDC_ARROW);
	hRightArrow = LoadCursor (hInst, MAKEINTRESOURCE (IDC_RIGHTARROW));
	selectedCursor = hLeftArrow;

	/* register frame window */
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = (WNDPROC)FrameWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInst;
	wndclass.hIcon = LoadIcon (hInst, MAKEINTRESOURCE (IDI_ELVIS));
	wndclass.hCursor = NULL;
	wndclass.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
	wndclass.lpszMenuName = MAKEINTRESOURCE (IDM_ELVIS);
	wndclass.lpszClassName = "ElvisFrameWnd";

	if (!RegisterClass (&wndclass))
		return FALSE;

	/* register client window */
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc = (WNDPROC)ClientWndProc;
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "ElvisClientWnd";

	if (!RegisterClass (&wndclass))
		return FALSE;

	/* indicate success */
	return TRUE;
}

/* ---[ init_instance ]------------------------------------------------ */

static int init_instance (HINSTANCE hInstance)

{
#if _MSC_VER < 900
#if defined (USE_CTL3D)
	/* try to register to the 3D controls library */
	if ((ctl3d_handle = LoadLibrary ("CTL3D32.DLL")) != NULL) {
		Ctl3dRegister (hInstance);
		Ctl3dAutoSubclass (hInstance);
	}
#endif
#endif

	return TRUE;
}

/* ---[ exit_instance ]------------------------------------------------ */

static void exit_instance (HINSTANCE hInstance)

{
#if _MSC_VER < 900
#if defined (USE_CTL3D)
	/* try to unregister to the 3D controls library */
	if (ctl3d_handle != NULL) {
		Ctl3dUnregister (hInstance);
		FreeLibrary (ctl3d_handle);
	}
#endif
#endif
}

/* ---[ FrameWndProc ]------------------------------------------------- */

LRESULT APIENTRY FrameWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)

{
	LRESULT    rc = 1;

	switch (msg) {
		case WM_COMMAND:
			rc = gwcmd (gw_find_frame (hwnd), wParam);
			break;
		case WM_CLOSE:
			rc = gwcmd (gw_find_frame (hwnd), (UINT)IDM_FILE_EXIT);
			break;
		case WM_GETMINMAXINFO:
			rc = gwframe_WM_GETMINMAXINFO (gw_find_frame (hwnd), wParam, lParam);
			break;
		case WM_INITMENU:
			rc = gwframe_WM_INITMENU (gw_find_frame (hwnd), wParam, lParam);
			break;
		case WM_MENUSELECT:
			rc = gwframe_WM_MENUSELECT (gw_find_frame (hwnd), wParam, lParam);
			break;
#if _MSC_VER > 900
		case WM_NOTIFY:
			rc = gwframe_WM_NOTIFY (gw_find_frame (hwnd), wParam, lParam);
			break;
#endif
		case WM_PAINT:
			rc = gwframe_WM_PAINT (gw_find_frame (hwnd), wParam, lParam);
			break;
		case WM_SETFOCUS:
			rc = gwframe_WM_SETFOCUS (gw_find_frame (hwnd), wParam, lParam);
			break;
		case WM_SIZE:
			rc = gwframe_WM_SIZE (gw_find_frame (hwnd), wParam, lParam);
			break;
	}

	return (rc == 0) ? 0 : DefWindowProc (hwnd, msg, wParam, lParam);
}

/* ---[ ClientWndProc ]------------------------------------------------ */

LRESULT APIENTRY ClientWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)

{
	LRESULT            rc = 1;

	switch (msg) {
		case WM_CHAR:
			rc = gwclient_WM_CHAR (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_DROPFILES:
			rc = gwclient_WM_DROPFILES (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_ERASEBKGND:
			rc = gwclient_WM_ERASEBKGND (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_KEYDOWN:
			rc = gwclient_WM_KEYDOWN (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_KEYUP:
			rc = gwclient_WM_KEYUP (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_KILLFOCUS:
			rc = gwclient_WM_KILLFOCUS (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_LBUTTONDBLCLK:
			rc = gwclient_WM_LBUTTONDBLCLK (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_LBUTTONDOWN:
			rc = gwclient_WM_LBUTTONDOWN (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_LBUTTONUP:
			rc = gwclient_WM_LBUTTONUP (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_MOUSEMOVE:
			rc = gwclient_WM_MOUSEMOVE (gw_find_client (hwnd), wParam, lParam);
			break;
#if defined (WM_MOUSEWHEEL)
	  case WM_MOUSEWHEEL:
		  rc = gwclient_WM_MOUSEWHEEL (gw_find_client (hwnd), wParam, lParam);
		  break;
#endif
		case WM_PAINT:
			rc = gwclient_WM_PAINT (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_RBUTTONDBLCLK:
			rc = gwclient_WM_RBUTTONDBLCLK (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_RBUTTONDOWN:
			rc = gwclient_WM_RBUTTONDOWN (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_RBUTTONUP:
			rc = gwclient_WM_RBUTTONUP (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_SETFOCUS:
			rc = gwclient_WM_SETFOCUS (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_SIZE:
			rc = gwclient_WM_SIZE (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_SYSKEYDOWN:
			rc = gwclient_WM_SYSKEYDOWN (gw_find_client (hwnd), wParam, lParam);
			break;
		case WM_VSCROLL:
			rc = gwclient_WM_VSCROLL (gw_find_client (hwnd), wParam, lParam);
			break;
	}

	return (rc == 0) ? 0 : DefWindowProc (hwnd, msg, wParam, lParam);
}

#endif
/* ex:se ts=4 sw=4: */
