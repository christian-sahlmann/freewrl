/*
=INSERT_TEMPLATE_HERE=

$Id: display_x11.c,v 1.7 2008/12/05 13:20:52 couannette Exp $

FreeX3D support library.
Display (X11) initialization.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>


/**
 * X11 initialization functions. Available to Motif as well.
 */

/**
 * public variables
 */
Display *Xdpy;
int Xscreen;
Window Xroot_window;
XVisualInfo *Xvi;
Window Xwin;
Window GLwin;
XSetWindowAttributes attr;
unsigned long mask = 0;
Atom WM_DELETE_WINDOW;

/**
 * local variables
 */
#if HAVE_XF86_VMODE

int vmode_nb_modes;
XF86VidModeModeInfo **vmode_modes = NULL;
int vmode_mode_selected = -1;

#endif


/**
 * quick sort comparison function to sort X modes
 */
static int mode_cmp(const void *pa,const void *pb)
{
    XF86VidModeModeInfo *a = *(XF86VidModeModeInfo**)pa;
    XF86VidModeModeInfo *b = *(XF86VidModeModeInfo**)pb;
    if(a->hdisplay > b->hdisplay) return -1;
    return b->vdisplay - a->vdisplay;
}

#if HAVE_XF86_VMODE
void switch_to_mode(int i)
{
    if ((!vmode_modes) || (i<0)) {
	ERROR_MSG("switch_to_mode: no valid mode available.\n");
	return;
    }

    vmode_mode_selected = i;

    win_width = vmode_modes[i]->hdisplay;
    win_height = vmode_modes[i]->vdisplay;
    TRACE_MSG("switch_to_mode: mode selected: %d (%d,%d).\n", 
	  vmode_mode_selected, win_width, win_height);
    XF86VidModeSwitchToMode(Xdpy, Xscreen, vmode_modes[i]);
    XF86VidModeSetViewPort(Xdpy, Xscreen, 0, 0);
}
#endif

/**
 * Initialize X-Window
 */
int open_display()
{
    char *display;
    int i;

    display = getenv("DISPLAY");
    Xdpy = XOpenDisplay(display);
    if (!Xdpy) {
	ERROR_MSG("can't open display %s.\n", display);
	return FALSE;
    }

    Xscreen = DefaultScreen(Xdpy);
    Xroot_window = RootWindow(Xdpy,Xscreen);

    if (fullscreen) {
#if HAVE_XF86_VMODE
	if (vmode_modes == NULL) {
	    if (XF86VidModeGetAllModeLines(Xdpy, Xscreen, &vmode_nb_modes, &vmode_modes) == 0) {
		ERROR_MSG("can`t get mode lines through XF86VidModeGetAllModeLines.\n");
		return FALSE;
	    }
	    qsort(vmode_modes, vmode_nb_modes, sizeof(XF86VidModeModeInfo*), mode_cmp);
	}
	for (i = 0; i < vmode_nb_modes; i++) {
	    if (vmode_modes[i]->hdisplay <= win_width && vmode_modes[i]->vdisplay <= win_height) {
		switch_to_mode(i);
		break;
	    }
	}
#endif
    }
    return TRUE;
}

int create_main_window_x11()
{
    Window root_ret;
    Window child_ret;
    int root_x_ret;
    int root_y_ret;
    unsigned int mask_ret;

    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(Xdpy, Xroot_window, Xvi->visual, AllocNone);

    attr.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | LeaveWindowMask | MapNotify | ButtonPressMask | ButtonReleaseMask | FocusChangeMask;

    if (fullscreen) {
	mask = CWBackPixel | CWColormap | CWOverrideRedirect | CWSaveUnder | CWBackingStore | CWEventMask;
	attr.override_redirect = true;
	attr.backing_store = NotUseful;
	attr.save_under = false;
    } else {
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
    }
		
    Xwin = XCreateWindow(Xdpy, Xroot_window, 0, 0, win_width, win_height,
			 0, Xvi->depth, InputOutput, Xvi->visual, mask, &attr);
    XMapWindow(Xdpy, Xwin);
		
    if (fullscreen) {
	XMoveWindow(Xdpy, Xwin, 0, 0);
	XRaiseWindow(Xdpy, Xwin);
	XFlush(Xdpy);
	XF86VidModeSetViewPort(Xdpy, Xscreen, 0, 0);
	XGrabPointer(Xdpy, Xwin, TRUE, 0, GrabModeAsync, GrabModeAsync, Xwin, None, CurrentTime);
	XGrabKeyboard(Xdpy, Xwin, TRUE, GrabModeAsync, GrabModeAsync, CurrentTime);
    } else {
	WM_DELETE_WINDOW = XInternAtom(Xdpy, "WM_DELETE_WINDOW", FALSE);
	XSetWMProtocols(Xdpy, Xwin, &WM_DELETE_WINDOW, 1);
    }
		
/*     XFlush(Xdpy); */

    XQueryPointer(Xdpy, Xwin, &root_ret, &child_ret, &root_x_ret, &root_y_ret,
		  &mouse_x, &mouse_y, &mask_ret);

    window_title = "FreeX3D";
    XStoreName(Xdpy, Xwin, window_title);
    XSetIconName(Xdpy, Xwin, window_title);
		
    XFlush(Xdpy);

    return TRUE;
}

/**
 * Create OpenGL context
 */
int create_GL_context()
{

#define DEFAULT_COMPONENT_WEIGHT 5

    static int attribs[] = {
	GLX_RGBA,
	GLX_DOUBLEBUFFER,
	GLX_RED_SIZE,    DEFAULT_COMPONENT_WEIGHT,
	GLX_GREEN_SIZE,  DEFAULT_COMPONENT_WEIGHT,
	GLX_BLUE_SIZE,   DEFAULT_COMPONENT_WEIGHT,
	GLX_ALPHA_SIZE,  DEFAULT_COMPONENT_WEIGHT,
	GLX_DEPTH_SIZE,  DEFAULT_COMPONENT_WEIGHT,
	None
    };

    Xvi = glXChooseVisual(Xdpy, Xscreen, attribs);
    if (!Xvi) {
	ERROR_MSG("can't choose appropriate visual (component weight=%d).\n", 
	      DEFAULT_COMPONENT_WEIGHT);
	return FALSE;
    }

    GLcx = glXCreateContext(Xdpy, Xvi, GLcx, TRUE);
    if (!GLcx) {
	ERROR_MSG("can't create OpenGL context.\n");
	return FALSE;
    }

    return TRUE;
}

int initialize_gl_context()
{
    if (!Xwin) {
	ERROR_MSG("window not initialized, can't initialize OpenGL context.\n");
	return FALSE;
    }
    if (!glXMakeCurrent(Xdpy, Xwin, GLcx)) {
	ERROR_MSG("can't initialize OpenGL context (glXMakeCurrent: %s).\n", GL_ERROR_MSG);
	return FALSE;
    }
    return TRUE;
}
