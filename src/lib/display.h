/*
=INSERT_TEMPLATE_HERE=

$Id: display.h,v 1.14 2009/01/03 01:15:07 couannette Exp $

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

extern int screenWidth;
extern int screenHeight;

extern double screenRatio;

extern char *window_title;

extern int mouse_x;
extern int mouse_y;

extern int show_mouse;

extern int xPos;
extern int yPos;

extern int displayDepth;

/**
 * Specific platform : Mac
 */
#if defined TARGET_AQUA

# include <OpenGL/OpenGL.h>
# include <OpenGL/CGLTypes.h>
# include <AGL/AGL.h>
# include <OpenGL/glu.h>

extern CGLContextObj myglobalContext;
extern AGLContext aqglobalContext;

extern GLboolean cErr;

extern GDHandle gGDevice;

extern int ccurse;
extern int ocurse;

#define SCURSE 1
#define ACURSE 0

#define SENSOR_CURSOR ccurse = SCURSE
#define ARROW_CURSOR  ccurse = ACURSE

/* for handling Safari window changes at the top of the display event loop */
extern int PaneClipnpx;
extern int PaneClipnpy;
extern WindowPtr PaneClipfwWindow;
extern int PaneClipct;
extern int PaneClipcb;
extern int PaneClipcr;
extern int PaneClipcl;
extern int PaneClipwidth;
extern int PaneClipheight;
extern int PaneClipChanged;

void eventLoopsetPaneClipRect(int npx, int npy, WindowPtr fwWindow, int ct, int cb, int cr, int cl, int width, int height);

int create_main_window_aqua(); /* mb */

# if defined(WANT_MULTI_OPENGL_THREADS)
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

# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/keysym.h>

# include <GL/glx.h>

extern GLXContext GLcx;

extern XEvent event;
extern long event_mask;
extern Display *Xdpy;
extern int Xscreen;
extern Window Xroot_window;
extern XVisualInfo *Xvi;
extern Window Xwin;
extern Window GLwin;
extern XSetWindowAttributes attr;
extern unsigned long mask;
extern Atom WM_DELETE_WINDOW;

extern Cursor arrowc;
extern Cursor sensorc;

# define SENSOR_CURSOR cursor = sensorc
# define ARROW_CURSOR  cursor = arrowc

extern Cursor curcursor;

void handle_Xevents(XEvent event);

# if HAVE_XF86_VMODE
#  include <X11/extensions/xf86vmode.h>
extern int vmode_nb_modes;
extern XF86VidModeModeInfo **vmode_modes;
extern int vmode_mode_selected;
# endif /* HAVE_XF86_VMODE */

# if defined(TARGET_MOTIF)

/**
 * Motif
 */
# include <X11/Intrinsic.h>
# include <Xm/Xm.h>

extern XtAppContext Xtcx;

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

void resetGeometry();
void setScreenDim(int wi, int he);


#endif /* __LIBFREEX3D_DISPLAY_H__ */
