/*
  $Id: fwBareWindow.c,v 1.9 2009/10/26 10:52:22 couannette Exp $

  Create X11 window. Manage events.

*/

/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2009 CRC Canada. (http://www.crc.gc.ca)

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FreeWRL/FreeX3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FreeWRL/FreeX3D.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/



#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include <X11/keysym.h>
#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>

#include "fwBareWindow.h"

XTextProperty windowName;
Window Pwin;
static XSetWindowAttributes Swa;

/* we have no buttons; just make these functions do nothing */
void frontendUpdateButtons() {}
void setMenuButton_collision (int val) {}
void setMenuButton_headlight (int val) {}
void setMenuButton_navModes (int type) {}
void setMenuButton_texSize (int size) {}
void setMessageBar() {}


void getBareWindowedGLwin (Window *win)
{
    GLwin = Xwin;
}

void openBareMainWindow (int argc, char **argv)
{
    /* get a connection */
    Xdpy = XOpenDisplay(0);
    if (!Xdpy) { fprintf(stderr, "No display!\n");exit(-1);}
}

#if 0
void createBareMainWindow ()
{
    /* create a color map */
    Cmap = XCreateColormap(Xdpy, RootWindow(Xdpy, Xvi->screen),Xvi->visual, AllocNone);
    /* create a window */
    Swa.colormap = Cmap;
    Swa.border_pixel = 0;
    Swa.event_mask = event_mask;


    Pwin = RootWindow(Xdpy, Xvi->screen);
    Xwin = XCreateWindow(Xdpy, Pwin,
                         xPos, yPos, win_width, win_height, 0, Xvi->depth, InputOutput,
                         Xvi->visual, CWBorderPixel | CWColormap | CWEventMask, &Swa);

    /* create window and icon name */
    if (XStringListToTextProperty(&wintitle, 1, &windowName) == 0){
        fprintf(stderr,
                "XStringListToTextProperty failed for %s, windowName in glpcOpenWindow.\n",
                wintitle);
    }
    XSetWMName(Xdpy, Xwin, &windowName);
    XSetWMIconName(Xdpy, Xwin, &windowName);
        
    /* are we running without Motif, and as a plugin? */
/*      if (!RUNNINGONAMD64) { */
    if (!RUNNINGASPLUGIN) {
        /* just map us to the display */
        XMapWindow(Xdpy, Xwin);
        XSetInputFocus(Xdpy, Pwin, RevertToParent, CurrentTime);
    }
/*      } */
}
#endif

int create_main_window(int argc, char *argv[])
{
    Window root_ret;
    Window child_ret;
    int root_x_ret;
    int root_y_ret;
    unsigned int mask_ret;

    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = colormap;

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

    GLwin = Xwin;
		
    XFlush(Xdpy);

    return TRUE;
}
