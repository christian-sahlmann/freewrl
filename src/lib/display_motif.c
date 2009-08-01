/*
=INSERT_TEMPLATE_HERE=

$Id: display_motif.c,v 1.9 2009/08/01 09:45:39 couannette Exp $

FreeWRL support library.
Display (Motif specific) initialization.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>


/* display part specific to X11/Motif */

XtAppContext Xtcx;


int create_main_window_motif()
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
#ifdef HAVE_XF86_VMODE
	XF86VidModeSetViewPort(Xdpy, Xscreen, 0, 0);
#endif
	XGrabPointer(Xdpy, Xwin, TRUE, 0, GrabModeAsync, GrabModeAsync, Xwin, None, CurrentTime);
	XGrabKeyboard(Xdpy, Xwin, TRUE, GrabModeAsync, GrabModeAsync, CurrentTime);
    } else {
	WM_DELETE_WINDOW = XInternAtom(Xdpy, "WM_DELETE_WINDOW", FALSE);
	XSetWMProtocols(Xdpy, Xwin, &WM_DELETE_WINDOW, 1);
    }
		
/*     XFlush(Xdpy); */

    XQueryPointer(Xdpy, Xwin, &root_ret, &child_ret, &root_x_ret, &root_y_ret,
		  &mouse_x, &mouse_y, &mask_ret);

    window_title = "FreeWRL";
    XStoreName(Xdpy, Xwin, window_title);
    XSetIconName(Xdpy, Xwin, window_title);
		
    XFlush(Xdpy);

    return TRUE;
}
