/*******************************************************************
 *
 * FreeX3D support library
 *
 * internal header - display.h
 *
 * Library internal display declarations: X11/Motif or OSX/Aqua
 *
 * $Id: display.h,v 1.8 2008/12/02 17:41:38 couannette Exp $
 *
 *******************************************************************/

#ifndef __LIBFREEX3D_DISPLAY_H__
#define __LIBFREEX3D_DISPLAY_H__

/**
 * Main window parameters
 */
int win_height; /* window */
int win_width;
int fullscreen;
int view_height; /* viewport */
int view_width;

char *window_title;

int mouse_x;
int mouse_y;

int show_mouse;

/**
 * Specific platform : Mac
 */
#if defined TARGET_AQUA

# include <OpenGL.h>
# include <glu.h>
# include <CGLTypes.h>
# include <AGL/AGL.h>

CGLContextObj myglobalContext;
AGLContext aqglobalContext;

# if WANT_MULTI_OPENGL_THREADS
/* multi-threaded OpenGL contexts - works on OS X, kind of ok on Linux, but
   blows plugins out of the water, because of the XLib threaded call in FrontEnd
   not working that well... */
#  define DO_MULTI_OPENGL_THREADS
# endif

#endif /* defined TARGET_AQUA */

/**
 * Specific platform : Linux / UNIX
 */
#if defined(TARGET_X11) || defined(TARGET_MOTIF)

# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/keysym.h>
# include <X11/extensions/xf86vmode.h>

Display *Xdpy;
int Xscreen;
Window Xroot_window;
XVisualInfo *Xvi;
Window Xwin;
Window GLwin;
XSetWindowAttributes attr;
unsigned long mask;
Atom WM_DELETE_WINDOW;

# if HAVE_XF86_VMODE

int vmode_nb_modes;
XF86VidModeModeInfo **vmode_modes;
int vmode_mode_selected;

# endif /* HAVE_XF86_VMODE */

# if defined(TARGET_MOTIF)

# include <X11/Intrinsic.h>
# include <Xm/Xm.h>

XtAppContext Xtcx;

int create_main_window_motif();

# else

int create_main_window_x11();

# endif /* defined(TARGET_MOTIF) */

# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glx.h>

GLXContext GLcx;

#endif /* defined(TARGET_X11) || defined(TARGET_MOTIF) */

/**
 * OpenGL / Window initialization
 */
int display_initialize();
int open_display();
int create_main_window();
int create_GL_context();
int initialize_gl_context();
int initialize_viewport();

#define GL_ERROR_MSG gluErrorString(glGetError())


#endif /* __LIBFREEX3D_DISPLAY_H__ */
