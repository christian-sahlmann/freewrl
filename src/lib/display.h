/*
=INSERT_TEMPLATE_HERE=

$Id: display.h,v 1.11 2008/12/10 18:46:54 crc_canada Exp $

FreeX3D support library.
Internal header: display (X11/Motif or OSX/Aqua) dependencies.

*/

#ifndef __LIBFREEX3D_DISPLAY_H__
#define __LIBFREEX3D_DISPLAY_H__

/**
 * Main window parameters
 */
extern int win_height; /* window */
extern int win_width;
extern int fullscreen;
extern int view_height; /* viewport */
extern int view_width;

extern char *window_title;

extern int mouse_x;
extern int mouse_y;

extern int show_mouse;

/**
 * Specific platform : Mac
 */
#if defined TARGET_AQUA

# include <OpenGL.h>
# include <glu.h>
# include <CGLTypes.h>
# include <AGL/AGL.h>

extern CGLContextObj myglobalContext;
extern AGLContext aqglobalContext;

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

/**
 * X11 common: weither we use Motif or not
 */

# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glx.h>

GLXContext GLcx;

# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/keysym.h>

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
#  include <X11/extensions/xf86vmode.h>
int vmode_nb_modes;
XF86VidModeModeInfo **vmode_modes;
int vmode_mode_selected;
# endif /* HAVE_XF86_VMODE */

# if defined(TARGET_MOTIF)

/**
 * Motif
 */
# include <X11/Intrinsic.h>
# include <Xm/Xm.h>

XtAppContext Xtcx;

int create_main_window_motif(); /* mb */

int isMotifDisplayInitialized();
void getMotifWindowedGLwin(Window *win);
void openMotifMainWindow(int argc, char ** argv);
void createMotifMainWindow();
# define ISDISPLAYINITIALIZED isMotifDisplayInitialized()
# define GET_GLWIN getMotifWindowedGLwin(&GLwin)
# define OPEN_TOOLKIT_MAINWINDOW openMotifMainWindow(argc, argv)
# define CREATE_TOOLKIT_MAIN_WINDOW createMotifMainWindow()

# else /* defined(TARGET_MOTIF) */

/**
 * Only X11, no Motif
 */
int create_main_window_x11(); /* mb */

# define HAVE_NOTOOLKIT
# define ISDISPLAYINITIALIZED TRUE
# define GET_GLWIN getBareWindowedGLwin(&GLwin)
# define OPEN_TOOLKIT_MAINWINDOW openBareMainWindow (argc, argv);
# define CREATE_TOOLKIT_MAIN_WINDOW createBareMainWindow();

# endif /* defined(TARGET_MOTIF) */

#endif /* defined(TARGET_X11) || defined(TARGET_MOTIF) */

/**
 * General : all systems
 */

#define GL_ERROR_MSG gluErrorString(glGetError())

int display_initialize(); /* mb */
int open_display(); /* mb */
int create_main_window(); /* mb */
int create_GL_context(); /* mb */
int initialize_gl_context(); /* mb */
int initialize_viewport(); /* mb */


#endif /* __LIBFREEX3D_DISPLAY_H__ */
