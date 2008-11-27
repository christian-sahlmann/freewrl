/*******************************************************************
 Copyright (C) 2006 John Stewart, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*********************************************************************/

#include <headers.h>
#include <vrmlconf.h>
#include <unistd.h>
#include "OpenGL_Utils.h"
#include "Viewer.h"

 #include <stdlib.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <math.h>

 #include <X11/keysym.h>
#ifndef AQUA
 #include <GL/gl.h>
 #include <GL/glu.h>
 #include <GL/glx.h>
#else
#include <OpenGL.h>
#endif
#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>

#ifdef HAVE_NOTOOLKIT
extern long event_mask;
extern int xPos;
extern int yPos;

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

#ifndef AQUA
void getBareWindowedGLwin (Window *win) {
		GLwin = Xwin;
}


void openBareMainWindow (argc, argv) {
        /* get a connection */
        Xdpy = XOpenDisplay(0);
        if (!Xdpy) { fprintf(stderr, "No display!\n");exit(-1);}
}

void createBareMainWindow () {
	/* create a color map */
	Cmap = XCreateColormap(Xdpy, RootWindow(Xdpy, Xvi->screen),Xvi->visual, AllocNone);
        /* create a window */
        Swa.colormap = Cmap;
        Swa.border_pixel = 0;
        Swa.event_mask = event_mask;


	Pwin = RootWindow(Xdpy, Xvi->screen);
	Xwin = XCreateWindow(Xdpy, Pwin,
		xPos, yPos, feWidth, feHeight, 0, Xvi->depth, InputOutput,
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
/* 	if (!RUNNINGONAMD64) { */
		if (!RUNNINGASPLUGIN) {
                        /* just map us to the display */
                        XMapWindow(Xdpy, Xwin);
                        XSetInputFocus(Xdpy, Pwin, RevertToParent, CurrentTime);
		}
/* 	} */
}
#endif
#endif
