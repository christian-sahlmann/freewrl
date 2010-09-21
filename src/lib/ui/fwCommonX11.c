/*
  $Id: fwCommonX11.c,v 1.6 2010/09/21 16:16:05 istakenv Exp $

  FreeWRL support library.
  X11 common functions.

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

#include <threads.h>


GLXContext GLcx;
long event_mask;
XEvent event;
Display *Xdpy;
int Xscreen;
Window Xroot_window;
Colormap colormap;
XVisualInfo *Xvi;
Window Xwin;
Window GLwin;
XSetWindowAttributes attr;
unsigned long mask = 0;
Atom WM_DELETE_WINDOW;

Cursor arrowc;
Cursor sensorc;
Cursor curcursor;

long event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask |
                    ButtonMotionMask | ButtonReleaseMask |
                    ExposureMask | StructureNotifyMask |
                    PointerMotionMask;

/**
 * X86 vmode : choose extended graphic mode
 */
#ifdef HAVE_XF86_VMODE

int oldx = 0, oldy = 0;
int vmode_nb_modes;
XF86VidModeModeInfo **vmode_modes = NULL;
int vmode_mode_selected = -1;

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
#endif /* HAVE_XF86_VMODE */

/**
 *   find_best_visual: use GLX to choose the X11 visual.
 */
XVisualInfo *find_best_visual()
{
	XVisualInfo *vi = NULL;
#define DEFAULT_COMPONENT_WEIGHT 5

	/*
	 * If FreeWRL is to be configurable one day,
	 * we will improve this visual query.
	 * One possibility: glXGetConfig.
	 */
	static int attribs[100] = {
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_RED_SIZE,    DEFAULT_COMPONENT_WEIGHT,
		GLX_GREEN_SIZE,  DEFAULT_COMPONENT_WEIGHT,
		GLX_BLUE_SIZE,   DEFAULT_COMPONENT_WEIGHT,
		GLX_ALPHA_SIZE,  DEFAULT_COMPONENT_WEIGHT,
		GLX_DEPTH_SIZE,  DEFAULT_COMPONENT_WEIGHT,
		None
	};

	if (shutterGlasses) {
		/* FIXME: handle stereo visual creation */
#ifdef STEREOCOMMAND
		system(STEREOCOMMAND);
#endif
	}

	if ((shutterGlasses) && (quadbuff_stereo_mode == 0)) {
		TRACE_MSG("Warning: No quadbuffer stereo visual found !");
		TRACE_MSG("On SGI IRIX systems read 'man setmon' or 'man xsetmon'\n");
	}

	quadbuff_stereo_mode = 0;

	vi = glXChooseVisual(Xdpy, Xscreen, attribs);
	return vi;
}

static int catch_XLIB(Display *disp, XErrorEvent *err)
{
	static int XLIB_errors = 0;
	static char error_msg[4096];

	XGetErrorText(disp, err->error_code, error_msg, sizeof(error_msg));

	ERROR_MSG("FreeWRL caught an XLib error !\n"
		  "   Display:    %s (%p)\n"
		  "   Error code: %d\n"
		  "   Error msg:  %s\n"
		  "   Request:    %d\n",
		  XDisplayName(NULL), disp, err->error_code, 
		  error_msg, err->request_code);
	
	XLIB_errors++;
	if (XLIB_errors > 20) {
		ERROR_MSG("FreeWRL - too many XLib errors (%d>20), exiting...\n", XLIB_errors);
		exit(0);
	}
	return 0;
}

int create_colormap()
{
	colormap = XCreateColormap(Xdpy, RootWindow(Xdpy, Xvi->screen),Xvi->visual, AllocNone);
	return TRUE;
}

/* void setMenuStatus(char *stat) */
/* { */
/* 	strncpy(myMenuStatus, stat, MAXSTAT); */
/* 	setMessageBar(); */
/* } */

/* void setMenuFps(float fps) */
/* { */
/* 	myFps = fps; */
/* 	setMessageBar(); */
/* } */

void resetGeometry()
{
#ifdef HAVE_XF86_VMODE
    int oldMode, i;

    if (fullscreen) {
	XF86VidModeGetAllModeLines(Xdpy, Xscreen, &vmode_nb_modes, &vmode_modes);
	oldMode = 0;
	
	for (i=0; i < vmode_nb_modes; i++) {
	    if ((vmode_modes[i]->hdisplay == oldx) && (vmode_modes[i]->vdisplay==oldy)) {
		oldMode = i;
		break;
	    }
	}
	
	XF86VidModeSwitchToMode(Xdpy, Xscreen, vmode_modes[oldMode]);
	XF86VidModeSetViewPort(Xdpy, Xscreen, 0, 0);
	XFlush(Xdpy);
    }
#endif /* HAVE_XF86_VMODE */
}

/*======== "VIRTUAL FUNCTIONS" ==============*/

/**
 *   open_display: setup up X11, choose visual, create colomap and query fullscreen capabilities.
 */
int open_display()
{
    char *display;

    fw_thread_dump();

    /* Display */
    XInitThreads();

    display = getenv("DISPLAY");
    Xdpy = XOpenDisplay(display);
    if (!Xdpy) {
	ERROR_MSG("can't open display %s.\n", display);
	return FALSE;
    }

    /* start up a XLib error handler to catch issues with FreeWRL. There
       should not be any issues, but, if there are, we'll most likely just
       throw our hands up, and continue */
    XSetErrorHandler(catch_XLIB); 

    Xscreen = DefaultScreen(Xdpy);
    Xroot_window = RootWindow(Xdpy,Xscreen);

    /* Visual */

    Xvi = find_best_visual();
    if(!Xvi) { 
	    ERROR_MSG("FreeWRL can not find an appropriate visual from GLX\n");
	    return FALSE;
    }

    /* Fullscreen */

    if (fullscreen) {
#ifdef HAVE_XF86_VMODE
	    int i;
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


    /* Color map */
    create_colormap();

    return TRUE;
}

/*=== create_main_window: in fwBareWindow.c or in fwMotifWindow.c */

/**
 *   create_GLcontext: create the main OpenGL context.
 *                     TODO: finish implementation for Mac and Windows.
 */
bool create_GLcontext()
{	
	int direct_rendering = TRUE;

	fw_thread_dump();

#if defined(TARGET_X11) || defined(TARGET_MOTIF)

	GLcx = glXCreateContext(Xdpy, Xvi, NULL, direct_rendering);
	if (!GLcx) {
		ERROR_MSG("can't create OpenGL context.\n");
		return FALSE;
	}
	if (glXIsDirect(Xdpy, GLcx)) {
		TRACE_MSG("glX: direct rendering enabled\n");
	}
#endif
	return TRUE;
}

/**
 *   bind_GLcontext: attache the OpenGL context to the main window.
 *                   TODO: finish implementation for Mac and Windows.
 */
bool bind_GLcontext()
{
	fw_thread_dump();

#if defined(TARGET_X11) || defined(TARGET_MOTIF)
	if (!Xwin) {
		ERROR_MSG("window not initialized, can't initialize OpenGL context.\n");
		return FALSE;
	}
	if (!glXMakeCurrent(Xdpy, GLwin, GLcx)) {
		ERROR_MSG("bind_GLcontext: can't set OpenGL context for this thread %d (glXMakeCurrent: %s).\n", fw_thread_id(), GL_ERROR_MSG);
		return FALSE;
	}
#endif

#if defined(TARGET_AQUA)
	return aglSetCurrentContext(aqglobalContext);
#endif

#if defined(TARGET_WIN32)
	return wglMakeCurrent(ghDC, ghRC);
#endif

	return TRUE;
}
