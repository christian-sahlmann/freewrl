/*******************************************************************
 *
 * FreeX3D support library
 *
 * internal header - display.h
 *
 * Library display declarations: X11/Motif or OSX/Aqua
 *
 * $Id: display.h,v 1.2 2008/10/31 19:30:36 couannette Exp $
 *
 *******************************************************************/

#ifndef __LIBFREEX3D_DISPLAY_H__
#define __LIBFREEX3D_DISPLAY_H__


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

int fullscreen;

/**
 * OpenGL / Window initialization
 */
void GL_create();
void Window_create();

/**
 * Windowing event handlers
 */
void doQuit();
void do_keyPress(const char kp, int type);

void updateContext();
float getWidth();
float getHeight();
void  setAquaCursor(int ctype);
void setMenuButton_collision(int val);
void setMenuButton_texSize(int size);
void setMenuStatus(char* stat);
void setMenuButton_headlight(int val);
void setMenuFps(float fps);
int aquaSetConsoleMessage(char* str);
void setMenuButton_navModes(int type);
void createAutoReleasePool();


#endif /* __LIBFREEX3D_DISPLAY_H__ */
