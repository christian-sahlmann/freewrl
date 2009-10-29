/*
  $Id: display.h,v 1.35 2009/10/29 06:29:40 crc_canada Exp $

  FreeWRL support library.
  Display global definitions for all architectures.

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



#ifndef __LIBFREEWRL_DISPLAY_H__
#define __LIBFREEWRL_DISPLAY_H__


/* Main initialization function */
int display_initialize();
extern bool display_initialized;
#define IS_DISPLAY_INITIALIZED (display_initialized==TRUE)

/**
 * Sort of "virtual" functions
 *
 * TARGET_AQUA   : 
 * TARGET_X11    : ui/fwBareWindow.c
 * TARGET_MOTIF  : ui/fwMotifWindow.c
 * TARGET_WIN32  : ui/fwWindow32.c
 */
int open_display();
int create_main_window(int argc, char *argv[]);
bool create_GLcontext();
bool bind_GLcontext();
/* end of "virtual" functions */

bool initialize_GL();
bool initialize_rdr_caps();
void initialize_rdr_functions();
void rdr_caps_dump();
void setMessageBar(void);
void setMenuStatus(char *stat);
#define MAXSTAT 200
extern char myMenuStatus[MAXSTAT];
extern float myFps;

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

extern int shutterGlasses; /* shutter glasses, stereo enabled ? */
extern int quadbuff_stereo_mode; /* quad buffer enabled ? */

/* OpenGL renderer capabilities */

typedef struct {

	const char *renderer; /* replace GL_REN */
	const char *version;
	const char *vendor;
	const char *extensions;

	bool av_multitexture; /* Multi textures available ? */
	bool av_glsl_shaders; /* GLSL shaders available ? */ 
	bool av_npot_texture; /* Non power of 2 textures available ? */
	bool av_texture_rect; /* Rectangle textures available ? */
	bool av_occlusion_q;  /* Occlusion query available ? */
	
	int texture_units;
	int max_texture_size;
	
} s_renderer_capabilities_t;

extern s_renderer_capabilities_t rdr_caps;

/**
 * Specific platform : Mac
 */
#ifdef TARGET_AQUA

# include <OpenGL/OpenGL.h>
# include <OpenGL/CGLTypes.h>
# include <AGL/AGL.h>

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

# if defined(WANT_MULTI_OPENGL_THREADS)
/* multi-threaded OpenGL contexts - works on OS X, kind of ok on Linux, but
   blows plugins out of the water, because of the XLib threaded call in FrontEnd
   not working that well... */
#undef DO_MULTI_OPENGL_THREADS
# endif

#include "OpenGL/glu.h"

#endif /* defined TARGET_AQUA */

/**
 * Specific platform : Linux / UNIX
 */
#if defined(TARGET_X11) || defined(TARGET_MOTIF)

/**
 * X11 common: weither we use Motif or not
 */

# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/keysym.h>

# ifdef HAVE_LIBGLEW

# include <GL/glew.h> /* will include GL/gl.h, GL/glu.h and GL/glext.h */

# else

# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glext.h>

# endif

# include <GL/glx.h>

extern GLXContext GLcx;

extern XEvent event;
extern long event_mask;
extern Display *Xdpy;
extern int Xscreen;
extern Window Xroot_window;
extern XVisualInfo *Xvi;
extern Colormap colormap;
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

# ifdef HAVE_XF86_VMODE
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

void getMotifWindowedGLwin(Window *win);
# define GET_GLWIN getMotifWindowedGLwin(&GLwin)

# else /* defined(TARGET_MOTIF) */

/**
 * Only X11, no Motif
 */
# define GET_GLWIN getBareWindowedGLwin(&GLwin)

# endif /* defined(TARGET_MOTIF) */

#endif /* defined(TARGET_X11) || defined(TARGET_MOTIF) */

#ifdef TARGET_WIN32

/* Nothing special :P ... */

#endif /* TARGET_WIN32 */

/**
 * General : all systems
 */

extern GLenum _global_gl_err;
#define GL_ERROR_MSG gluErrorString(glGetError())
#define PRINT_GL_ERROR_IF_ANY(_where) do { if (global_print_opengl_errors) { \
                                              GLenum _global_gl_err = glGetError(); \
                                              if (_global_gl_err != GL_NO_ERROR) { \
                                                 char *_str = (char *) gluErrorString(_global_gl_err); \
                                                 fprintf(stderr, "GL error: %s, here: %s\n", _str, _where); \
                                              } \
                                           } \
                                      } while (0)

void resetGeometry();
void setScreenDim(int wi, int he);

/* debugging OpenGL calls  - allows us to keep track of what is happening */
#undef DEBUG_OPENGL_CALLS
#ifdef DEBUG_OPENGL_CALLS
	#define FW_GL_ENABLE(aaa) \
		{glEnable(aaa); \
		 printf ("glEnable %d at %s:%d\n",aaa,__FILE__,__LINE__);}
	#define FW_GL_DISABLE(aaa) \
		{glDisable(aaa); \
		 printf ("glDisable %d at %s:%d\n",aaa,__FILE__,__LINE__);}

	#define FW_GL_ENABLECLIENTSTATE(aaa) \
		{glEnableClientState(aaa); \
		 printf ("glEnableClientState %d at %s:%d\n",aaa,__FILE__,__LINE__);}
	#define FW_GL_DISABLECLIENTSTATE(aaa) \
		{glDisableClientState(aaa); \
		 printf ("glDisableClientState %d at %s:%d\n",aaa,__FILE__,__LINE__);}

	#define FW_GL_DRAWARRAYS(xxx,yyy,zzz) \
		{glDrawArrays(xxx,yyy,zzz); \
		printf ("glDrawArrays tat %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_TRANSLATE_F(xxx,yyy,zzz) \
		{glTranslatef(xxx,yyy,zzz); \
		printf ("glTranslatef\t%6.2f %6.2f %6.2f\tat %s:%d\n",xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_TRANSLATE_D(xxx,yyy,zzz) \
		{glTranslated(xxx,yyy,zzz); \
		printf ("glTranslated\t%6.2f %6.2f %6.2f\tat %s:%d\n",xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_ROTATE_F(aaa,xxx,yyy,zzz) \
		{glRotatef(aaa,xxx,yyy,zzz); \
		printf ("glRotatef\t%6.2f %6.2f %6.2f %6.2f\tat %s:%d\n",aaa,xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_ROTATE_D(aaa,xxx,yyy,zzz) \
		{glRotated(aaa,xxx,yyy,zzz); \
		printf ("glRotated\t%6.2f %6.2f %6.2f %6.2f\tat %s:%d\n",aaa,xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_SCALE_F(xxx,yyy,zzz) \
		{glScalef(xxx,yyy,zzz); \
		printf ("glScalef\t%6.2f %6.2f %6.2f\tat %s:%d\n",xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_SCALE_D(xxx,yyy,zzz) \
		{glScaled(xxx,yyy,zzz); \
		printf ("glScaled\t%6.2f %6.2f %6.2f\tat %s:%d\n",xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_LOAD_IDENTITY(aaa) {glLoadIdentity();printf ("glLoadIdentity\t at%s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_PUSH_MATRIX(aaa) {glPushMatrix();printf ("glPushMatrix\t at%s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_POP_MATRIX(aaa) {glPopMatrix();printf ("glPopMatrix\t at%s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_MATRIX_MODE(aaa) {glMatrixMode(aaa); \
		printf ("glMatrixMode("); \
		if (aaa==GL_TEXTURE) printf ("GL_TEXTURE"); \
		else if (aaa==GL_MODELVIEW) printf ("GL_MODELVIEW"); \
		else if (aaa==GL_PROJECTION) printf ("GL_PROJECTION"); \
		else printf ("unknown mode"); printf (")\tat %s:%d\n",__FILE__,__LINE__); }
	#define FW_GL_GETDOUBLEV(aaa,bbb) \
		{ fwGetDoublev(aaa,bbb); \
		printf ("fwGetDoublev at %s:%d\n",__FILE__,__LINE__);}

#else
	#define FW_GL_ENABLE(aaa) glEnable(aaa)
	#define FW_GL_DISABLE(aaa) glDisable(aaa); 
	#define FW_GL_ENABLECLIENTSTATE(aaa) glEnableClientState(aaa)
	#define FW_GL_DISABLECLIENTSTATE(aaa) glDisableClientState(aaa); 
	#define FW_GL_DRAWARRAYS(xxx,yyy,zzz) glDrawArrays(xxx,yyy,zzz)
	#define FW_GL_TRANSLATE_F(xxx,yyy,zzz) glTranslatef(xxx,yyy,zzz)
	#define FW_GL_TRANSLATE_D(xxx,yyy,zzz) glTranslated(xxx,yyy,zzz)
	#define FW_GL_ROTATE_D(aaa,xxx,yyy,zzz) glRotated(aaa,xxx,yyy,zzz)
	#define FW_GL_ROTATE_F(aaa,xxx,yyy,zzz) glRotatef(aaa,xxx,yyy,zzz)
	#define FW_GL_SCALE_F(xxx,yyy,zzz) glScalef(xxx,yyy,zzz)
	#define FW_GL_SCALE_D(xxx,yyy,zzz) glScaled(xxx,yyy,zzz)
	#define FW_GL_LOAD_IDENTITY glLoadIdentity
	#define FW_GL_MATRIX_MODE(aaa) glMatrixMode(aaa)
#if defined(_MSC_VER)
	#define FW_GL_PUSH_MATRIX(...) glPushMatrix()
	#define FW_GL_POP_MATRIX(...) glPopMatrix()
#else
	#define FW_GL_PUSH_MATRIX(aaa) glPushMatrix()
	#define FW_GL_POP_MATRIX(aaa) glPopMatrix()
#endif
	#define FW_GL_GETDOUBLEV(aaa,bbb) fwGetDoublev(aaa,bbb); 
#endif

#endif /* __LIBFREEWRL_DISPLAY_H__ */
