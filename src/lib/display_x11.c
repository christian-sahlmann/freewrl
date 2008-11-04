/*******************************************************************
 *
 * FreeX3D support library
 *
 * display_x11.c
 *
 * $Id: display_x11.c,v 1.3 2008/11/04 00:40:34 couannette Exp $
 *
 *******************************************************************/

#include "config.h"
#include "system.h"
#include "display.h"
#include "internal.h"

/* display part specific to bare X11 */

XSetWindowAttributes attr;
unsigned long mask = 0;
int num_modes = 0;
XF86VidModeModeInfo **modes = NULL;
static Atom WM_DELETE_WINDOW;

Window root_ret;
Window child_ret;
int root_x_ret;
int root_y_ret;
unsigned int mask_ret;

/**
 * quick sort comparison function to sort X modes
 */
static int mode_cmp(const void *pa,const void *pb) {
    XF86VidModeModeInfo *a = *(XF86VidModeModeInfo**)pa;
    XF86VidModeModeInfo *b = *(XF86VidModeModeInfo**)pb;
    if(a->hdisplay > b->hdisplay) return -1;
    return b->vdisplay - a->vdisplay;
}

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
	//ERROR
	return FALSE;
    }

    Xscreen = DefaultScreen(Xdpy);
    Xroot_window = RootWindow(Xdpy,Xscreen);

    if (fullscreen) {
	if (modes == NULL) {
	    if (XF86VidModeGetAllModeLines(Xdpy, Xscreen, &num_modes, &modes) == 0)
		//ERROR("can`t get mode lines");
		qsort(modes, num_modes, sizeof(XF86VidModeModeInfo*), mode_cmp);
	}
	for (i = 0; i < num_modes; i++) {
	    if (modes[i]->hdisplay <= win_width && modes[i]->vdisplay <= win_height) {
		win_width = modes[i]->hdisplay;
		win_height = modes[i]->vdisplay;
		XF86VidModeSwitchToMode(Xdpy, Xscreen, modes[i]);
		XF86VidModeSetViewPort(Xdpy, Xscreen, 0, 0);
		break;
	    }
	}
    }
    return TRUE;
}

int create_main_window()
{
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
		
    XFlush(Xdpy);

    XQueryPointer(Xdpy, Xwin, &root_ret, &child_ret, &root_x_ret, &root_y_ret,
		  &mouse_x, &mouse_y, &mask_ret);

    window_title = "FreeX3D";
    XStoreName(Xdpy, Xwin, window_title);
    XSetIconName(Xdpy, Xwin, window_title);

    if (!glXMakeCurrent(Xdpy, Xwin, GLcx)) {
	//ERROR("glXMakeCurrent() failed");
	return FALSE;
    }

    glViewport(0, 0, win_width, win_height);
		
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
	//ERROR
	return FALSE;
    }

    GLcx = glXCreateContext(Xdpy, Xvi, GLcx, TRUE);
    if (!GLcx) {
	//ERROR("glXCreateContext() failed");
	return FALSE;
    }

    return TRUE;
}

