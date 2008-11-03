/*******************************************************************
 *
 * FreeX3D support library
 *
 * internal header - display.h
 *
 * Library internal display declarations: X11/Motif or OSX/Aqua
 *
 * $Id: display.h,v 1.4 2008/11/03 14:14:12 couannette Exp $
 *
 *******************************************************************/

#ifndef __LIBFREEX3D_DISPLAY_H__
#define __LIBFREEX3D_DISPLAY_H__

/**
 * All declarations here are internal to the library.
 */

#ifdef AQUA /* Mac stuff */

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

#else /* X-Window display/window stuff, future would be to support Windows */

# include <X11/X.h>
# include <X11/Xlib.h>
# include <X11/keysym.h>
# include <X11/cursorfont.h>

# ifdef XF86V4
#  include <X11/extensions/xf86vmode.h>
# endif

# include <X11/Intrinsic.h>

# include <GL/gl.h>
# include <GL/glx.h>
# include <GL/glu.h>
# include <GL/glext.h>

Display *Xdpy;
GLXContext GLcx;
XVisualInfo *Xvi;
Window Xwin;
Window GLwin;

#endif

/**
 * OpenGL / Window initialization
 */
extern void GL_create();
extern void Window_create();

/**
 * Windowing event handlers
 */
extern void doQuit();
extern void do_keyPress(const char kp, int type);

extern void updateContext();
extern float getWidth();
extern float getHeight();
extern void  setAquaCursor(int ctype);
extern void setMenuButton_collision(int val);
extern void setMenuButton_texSize(int size);
extern void setMenuStatus(char* stat);
extern void setMenuButton_headlight(int val);
extern void setMenuFps(float fps);
extern int aquaSetConsoleMessage(char* str);
extern void setMenuButton_navModes(int type);
extern void createAutoReleasePool();

extern void XEventStereo(void);

extern void openMainWindow (void);
extern void glpOpenGLInitialize(void);
extern void EventLoop(void);
extern void resetGeometry(void);



#endif /* __LIBFREEX3D_DISPLAY_H__ */
