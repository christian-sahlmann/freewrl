/*
  =INSERT_TEMPLATE_HERE=

  $Id: fwBareWindow.c,v 1.8 2009/10/05 15:07:24 crc_canada Exp $

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

char    *wintitle =  "FreeWRL VRML/X3D Browser";
XTextProperty windowName;
Window Pwin;
static Colormap Cmap;
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
