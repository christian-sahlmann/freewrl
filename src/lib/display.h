/*
  $Id: display.h,v 1.46 2010/01/21 19:26:27 crc_canada Exp $

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
	float anisotropicDegree;
	
} s_renderer_capabilities_t;

extern s_renderer_capabilities_t rdr_caps;

/**
 * Specific platform : Mac
 */
#ifdef TARGET_AQUA

#ifdef IPHONE
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
extern int ccurse;
extern int ocurse;
#define SCURSE 1
#define ACURSE 0

#define SENSOR_CURSOR ccurse = SCURSE
#define ARROW_CURSOR  ccurse = ACURSE
#else

# include <OpenGL/OpenGL.h>
# include <OpenGL/CGLTypes.h>

# include <AGL/AGL.h> 
#ifdef OLDCODE
OLDCODEextern GLboolean cErr;
OLDCODEextern GDHandle gGDevice;
OLDCODEextern AGLContext aqglobalContext;
#endif

extern CGLContextObj myglobalContext;


extern int ccurse;
extern int ocurse;

#define SCURSE 1
#define ACURSE 0

#define SENSOR_CURSOR ccurse = SCURSE
#define ARROW_CURSOR  ccurse = ACURSE

/* for handling Safari window changes at the top of the display event loop */
extern int PaneClipnpx;
extern int PaneClipnpy;

#ifdef OLDCODE
OLDCODE extern WindowPtr PaneClipfwWindow;
#endif

extern int PaneClipct;
extern int PaneClipcb;
extern int PaneClipcr;
extern int PaneClipcl;
extern int PaneClipwidth;
extern int PaneClipheight;
extern int PaneClipChanged;

#ifdef OLDCODE
OLDCODEvoid eventLoopsetPaneClipRect(int npx, int npy, WindowPtr fwWindow, int ct, int cb, int cr, int cl, int width, int height);
#endif

# if defined(WANT_MULTI_OPENGL_THREADS)
/* multi-threaded OpenGL contexts - works on OS X, kind of ok on Linux, but
   blows plugins out of the water, because of the XLib threaded call in FrontEnd
   not working that well... */
#undef DO_MULTI_OPENGL_THREADS
#endif

#include "OpenGL/glu.h"

#endif /* defined IPHONE */
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
#include <GL/glew.h>
#define SENSOR_CURSOR sensor_cursor32();
#define ARROW_CURSOR arrow_cursor32();
#define ERROR 0

#endif /* TARGET_WIN32 */

/**
 * General : all systems
 */

extern GLenum _global_gl_err;
#ifndef IPHONE
#define GL_ERROR_MSG gluErrorString(glGetError())
#define PRINT_GL_ERROR_IF_ANY(_where) do { if (global_print_opengl_errors) { \
                                              GLenum _global_gl_err = glGetError(); \
                                              if (_global_gl_err != GL_NO_ERROR) { \
                                                 char *_str = (char *) gluErrorString(_global_gl_err); \
                                                 fprintf(stderr, "GL error: %s, here: %s\n", _str, _where); \
                                              } \
                                           } \
                                      } while (0)
#endif

void resetGeometry();
void setScreenDim(int wi, int he);

/* debugging OpenGL calls  - allows us to keep track of what is happening */
#undef DEBUG_OPENGL_CALLS
#ifdef DEBUG_OPENGL_CALLS
	#define FW_GL_ORTHO(aaa,bbb,ccc,ddd,eee,fff) \
		{glOrtho(aaa,bbb,ccc,ddd,eee,fff); \
		printf ("glOrtho at %s:%d\n",__FILE__,__LINE__);}

	#define FW_GL_LINEWIDTH(aaa) \
		{glLineWidth(aaa); \
		printf ("glLineWdith at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_DEPTHMASK(aaa) \
		{glDepthMask(aaa);
		printf ("glDepthMask at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_COLOR3F(aaa,bbb,ccc) \
		{glColor3f(aaa,bbb,ccc);\
		printf ("glColor3f at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_COLOR4FV(aaa) \
		{glColor3fv(aaa);\
		printf ("glColor4fv at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_RASTERPOS2I(aaa,bbb) \
		{glRasterPos2i(aaa,bbb); \
		printf ("glRasterPos2i at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_SCISSOR(aaa,bbb,ccc,ddd) \
		{glScissor(aaa,bbb,ccc,ddd); \
		printf ("glScissor at %s:%d\n",__FILE__,__LINE__);}
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
		{fw_glTranslatef(xxx,yyy,zzz); \
		printf ("fw_glTranslatef\t%6.2f %6.2f %6.2f\tat %s:%d\n",xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_TRANSLATE_D(xxx,yyy,zzz) \
		{fw_glTranslated(xxx,yyy,zzz); \
		printf ("fw_glTranslated\t%6.2f %6.2f %6.2f\tat %s:%d\n",xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_ROTATE_F(aaa,xxx,yyy,zzz) \
		{fw_glRotatef(aaa,xxx,yyy,zzz); \
		printf ("fw_glRotatef\t%6.2f %6.2f %6.2f %6.2f\tat %s:%d\n",aaa,xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_ROTATE_D(aaa,xxx,yyy,zzz) \
		{fw_glRotated(aaa,xxx,yyy,zzz); \
		printf ("fw_glRotated\t%6.2f %6.2f %6.2f %6.2f\tat %s:%d\n",aaa,xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_ROTATE_RADIANS(aaa,xxx,yyy,zzz) \
		{fw_glRotateRad(aaa,xxx,yyy,zzz); \
		printf ("fw_glRotateRadians\t%6.2f %6.2f %6.2f %6.2f\tat %s:%d\n",aaa,xxx,yyy,zzz,__FILE__,__LINE__);}
	
	#define FW_GL_SCALE_F(xxx,yyy,zzz) \
		{fw_glScalef(xxx,yyy,zzz); \
		printf ("fw_glScalef\t%6.2f %6.2f %6.2f\tat %s:%d\n",xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_SCALE_D(xxx,yyy,zzz) \
		{fw_glScaled(xxx,yyy,zzz); \
		printf ("fw_glScaled\t%6.2f %6.2f %6.2f\tat %s:%d\n",xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_LOAD_IDENTITY(aaa) {fw_glLoadIdentity();printf ("fw_glLoadIdentity\t at%s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_PUSH_MATRIX(aaa) {fw_glPushMatrix();printf ("fw_glPushMatrix\t at%s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_POP_MATRIX(aaa) {fw_glPopMatrix();printf ("fw_glPopMatrix\t at%s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_ALPHAFUNC(aaa,bbb){glAlphaFunc(aaa,bbb); printf ("glAlphaFunc at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_MATRIX_MODE(aaa) {fw_glMatrixMode(aaa); \
		printf ("fw_glMatrixMode("); \
		if (aaa==GL_TEXTURE) printf ("GL_TEXTURE"); \
		else if (aaa==GL_MODELVIEW) printf ("GL_MODELVIEW"); \
		else if (aaa==GL_PROJECTION) printf ("GL_PROJECTION"); \
		else printf ("unknown mode"); printf (")\tat %s:%d\n",__FILE__,__LINE__); }
	#define FW_GL_GETDOUBLEV(aaa,bbb) \
		{ fw_glGetDoublev(aaa,bbb); \
		printf ("fw_glGetDoublev at %s:%d\n",__FILE__,__LINE__);}

	#define FW_GL_PUSH_ATTRIB(aaa) \
		{ glPushAttrib(aaa); \
		printf ("glPushAttrib at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_POP_ATTRIB(aaa) \
		{ glPopAttrib(aaa); \
		printf ("glPopAttrib at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_WINDOWPOS2I(aaa,bbb) \
		{ glWindowPos2i(aaa,bbb); \
		printf ("glWindowPos2i at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_FLUSH() \
		{glFlush(); \
		printf ("glFlush at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_PIXELZOOM(aaa,bbb) \
		{glPixelZoom(aaa,bbb); \
		glPixelZoom(aaa,bbb);}
	#define FW_GL_CLEAR_COLOR(aaa,bbb,ccc,ddd) \
		{ glClearColor(aaa,bbb,ccc,ddd);
		printf ("glClearColor at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_CLEAR_DEPTH(aaa) \
		{glClearDepth(aaa); \
		printf ("glClearDepth at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_LIGHTMODELFV(aaa,bbb) \
		{glLightModelfv(aaa,bbb); \
		printf ("glLightModelfv at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_LIGHTMODELI(aaa,bbb) \
		{glLightModeli(aaa,bbb); \
		printf ("glLightModeli at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_FRUSTUM(aaa,bbb,ccc,ddd,eee,fff) \
		{glFrustum(aaa,bbb,ccc,ddd,eee,fff); \
		printf ("glFrustum at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_BLENDFUNC(aaa,bbb) \
		{glBlendFunc(aaa,bbb); \
		printf ("glBlendFunc at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_LIGHTFV(aaa,bbb,ccc) \
		{glLightfv(aaa,bbb,ccc);\
		printf ("glLightfv at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_HINT(aaa,bbb) \
		{glHint(aaa,bbb); \
		printf ("glHint at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_CLEAR(aaa) \
		{glClear(aaa); \
		printf ("glClear at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_DEPTHFUNC(aaa) \
		{glDepthFunc(aaa); \
		printf ("glDepthFunc at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_SHADEMODEL(aaa) \
		{glShadeModel(aaa); \
		printf ("glShadeModel at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_PIXELSTOREI(aaa,bbb) \
		{glPixelStorei(aaa,bbb); \
		printf ("glPixelNstorei at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_POINTSIZE(aaa) \
		{glPointSize(aaa); \
		printf ("glPointSize at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_CGLFLUSHDRAWABLE(aaa) \
		CGLFlushDrawable(aaa); \
		printf ("CGLFLushDrawable at %s:%d\n",__FILE__,__LINE__);
	#define FW_GL_GETDOUBLEV(aaa,bbb) \
		{glGetDoublev(aaa,bbb);
		printf ("glGetDoublev at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GL_VIEWPORT(aaa,bbb,ccc,ddd) \
		{glViewport(aaa,bbb,ccc,ddd);
		printf ("glViewpoirt at %s:%d\n",__FILE__,__LINE__);}
	#define FW_GLU_PERSPECTIVE(aaa,bbb,ccc,ddd) \
		gluPerspective(aaa,bbb,ccc,ddd); \
		printf ("gluPerspective at %s:%d\n",__FILE__,__LINE__);}
	#define SET_TEXTURE_UNIT(aaa) { printf ("setActiveTexture to %d at %s:%d\n",aaa,__FILE__,__LINE__); \
			glActiveTexture(GL_TEXTURE0+aaa); glClientActiveTexture(GL_TEXTURE0+aaa); }

#else /* DEBUG_OPENGL_CALLS */
#ifndef IPHONE
	#define FW_GL_GETDOUBLEV(aaa,bbb) glGetDoublev(aaa,bbb);
	#define FW_GL_LOAD_IDENTITY fw_glLoadIdentity
	#define FW_GL_MATRIX_MODE(aaa) fw_glMatrixMode(aaa)
	#define FW_GL_VIEWPORT(aaa,bbb,ccc,ddd) glViewport(aaa,bbb,ccc,ddd);
	#define FW_GL_CLEAR_COLOR(aaa,bbb,ccc,ddd) glClearColor(aaa,bbb,ccc,ddd);
	#define FW_GL_COLOR3F(aaa,bbb,ccc) glColor3f(aaa,bbb,ccc);
	#define FW_GL_COLOR4FV(aaa) glColor4fv(aaa);
	#define FW_GL_DEPTHMASK(aaa) glDepthMask(aaa);
	#define FW_GL_ENABLE(aaa) glEnable(aaa)
	#define FW_GL_DISABLE(aaa) glDisable(aaa); 
	#define FW_GL_ENABLECLIENTSTATE(aaa) glEnableClientState(aaa)
	#define FW_GL_DISABLECLIENTSTATE(aaa) glDisableClientState(aaa); 
	#define FW_GL_DRAWARRAYS(xxx,yyy,zzz) glDrawArrays(xxx,yyy,zzz)
	#define FW_GL_TRANSLATE_F(xxx,yyy,zzz) fw_glTranslatef(xxx,yyy,zzz)
	#define FW_GL_TRANSLATE_D(xxx,yyy,zzz) fw_glTranslated(xxx,yyy,zzz)

	#define FW_GL_ROTATE_F(aaa,xxx,yyy,zzz) \
		{fw_glRotatef(aaa,xxx,yyy,zzz); \
		printf ("fw_glRotatef\t%6.2f %6.2f %6.2f %6.2f\tat %s:%d\n",aaa,xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_ROTATE_D(aaa,xxx,yyy,zzz) \
		{fw_glRotated(aaa,xxx,yyy,zzz); \
		printf ("fw_glRotated\t%6.2f %6.2f %6.2f %6.2f\tat %s:%d\n",aaa,xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_ROTATE_RADIANS(aaa,xxx,yyy,zzz) fw_glRotateRad(aaa,xxx,yyy,zzz)
/*
	#define FW_GL_ROTATE_D(aaa,xxx,yyy,zzz) fw_glRotated(aaa,xxx,yyy,zzz)
	#define FW_GL_ROTATE_F(aaa,xxx,yyy,zzz) fw_glRotatef(aaa,xxx,yyy,zzz)
*/
	#define FW_GL_SCALE_F(xxx,yyy,zzz) fw_glScalef(xxx,yyy,zzz)
	#define FW_GL_SCALE_D(xxx,yyy,zzz) fw_glScaled(xxx,yyy,zzz)
#if defined(_MSC_VER)
	#define FW_GL_PUSH_MATRIX(...) fw_glPushMatrix()
	#define FW_GL_POP_MATRIX(...) fw_glPopMatrix()
#else /* _MSC_VER */
	#define FW_GL_PUSH_MATRIX(aaa) fw_glPushMatrix()
	#define FW_GL_POP_MATRIX(aaa) fw_glPopMatrix()

#endif /* _MSC_VER */
	#define FW_GL_ALPHAFUNC(aaa,bbb) glAlphaFunc(aaa,bbb); 
        #define FW_GL_SCISSOR(aaa,bbb,ccc,ddd) glScissor(aaa,bbb,ccc,ddd); 
        #define FW_GL_PUSH_ATTRIB(aaa) glPushAttrib(aaa); 
        #define FW_GL_POP_ATTRIB(aaa) glPopAttrib(aaa); 
	#define FW_GL_WINDOWPOS2I(aaa,bbb) glWindowPos2i(aaa,bbb);
	#define FW_GL_FLUSH glFlush
	#define FW_GL_ORTHO(aaa,bbb,ccc,ddd,eee,fff) glOrtho(aaa,bbb,ccc,ddd,eee,fff); 
	#define FW_GL_RASTERPOS2I(aaa,bbb) glRasterPos2i(aaa,bbb); 
	#define FW_GL_PIXELZOOM(aaa,bbb) glPixelZoom(aaa,bbb);
        #define FW_GL_LIGHTMODELI(aaa,bbb) glLightModeli(aaa,bbb); 
        #define FW_GL_LIGHTMODELFV(aaa,bbb) glLightModelfv(aaa,bbb); 
	#define FW_GL_CLEAR_DEPTH(aaa) glClearDepth(aaa); 
	#define FW_GL_FRUSTUM(aaa,bbb,ccc,ddd,eee,fff) glFrustum(aaa,bbb,ccc,ddd,eee,fff);
	#define FW_GL_BLENDFUNC(aaa,bbb) glBlendFunc(aaa,bbb);
	#define FW_GL_LIGHTFV(aaa,bbb,ccc) glLightfv(aaa,bbb,ccc);
	#define FW_GL_HINT(aaa,bbb) glHint(aaa,bbb); 
	#define FW_GL_CLEAR(zzz) glClear(zzz); 
	#define FW_GL_DEPTHFUNC(zzz) glDepthFunc(zzz); 
	#define FW_GL_SHADEMODEL(aaa) glShadeModel(aaa); 
	#define FW_GL_LINEWIDTH(aaa) glLineWidth(aaa); 
	#define FW_GL_POINTSIZE(aaa) glPointSize(aaa); 
	#define FW_GL_PIXELSTOREI(aaa,bbb) glPixelStorei(aaa,bbb);
	#define FW_GL_CGLFLUSHDRAWABLE(aaa) CGLFlushDrawable(aaa)
	#define FW_GLU_PERSPECTIVE(aaa,bbb,ccc,ddd) gluPerspective(aaa,bbb,ccc,ddd)
	#define FW_GLU_PICK_MATRIX(aaa, bbb, ccc, ddd, eee) gluPickMatrix(aaa, bbb, ccc, ddd, eee)
	#define FW_GL_TEXENVI(aaa,bbb,ccc) glTexEnvi(aaa,bbb,ccc)
	#define FW_GL_TEXGENI(aaa,bbb,ccc) glTexGeni(aaa,bbb,ccc)
	#define FW_GL_TEXCOORDPOINTER(aaa,bbb,ccc,ddd) glTexCoordPointer(aaa,bbb,ccc,ddd)
	#define FW_GL_BINDTEXTURE(aaa,bbb) glBindTexture(aaa,bbb)
#endif /* NDEF IPHONE */

#endif /* DEBUG_OPENGL_CALLS */

// Use macros to enable compilation under OpenGL ES 2.0 
#ifdef IPHONE
#define GL_STEREO 0
#define GL_FOG_COLOR 0
#define GL_FOG_DENSITY 0
#define GL_FOG_END 0
#define GL_FOG_MODE 0
#define GL_SMOOTH 0
#define GL_FOG_START 0
#define GL_EXP 0
#define GLU_BEGIN 0
#define GLU_EDGE_FLAG 0
#define GLU_VERTEX 0
#define GLU_TESS_VERTEX 0
#define GLU_ERROR 0
#define GLU_END 0
#define GLU_TESS_COMBINE_DATA 0
#define GLU_TESS_COMBINE 0
#define GL_LIGHT1 0
#define GL_LIGHT2 0
#define GL_LIGHT3 0
#define GL_LIGHT4 0
#define GL_LIGHT5 0
#define GL_LIGHT6 0
#define GLU_UNKNOWN 0
#define GL_SAMPLES_PASSED 0
#define GL_LIGHT0 0
#define GL_POSITION 0
#define GL_SPECULAR 0
#define GL_AMBIENT 0
#define GL_SPOT_DIRECTION 0
#define GL_POSITION 0
#define GL_CONSTANT_ATTENUATION 0
#define GL_LINEAR_ATTENUATION 0
#define GL_QUADRATIC_ATTENUATION 0
#define GL_SPOT_CUTOFF 0
#define GL_SPOT_EXPONENT 0
#define GL_RGBA8 0
#define GL_R 0
#define PATH_MAX 0
#define GL_PREVIOUS 0
#define GL_ADD 0
#define GL_SUBTRACT 0
#define GL_DOT3_RGB 0
#define GL_ADD_SIGNED 0
#define GL_CLAMP 0
#define FL_CLAMP_TO_BORDER 0
#define GL_TEXTURE_WRAP_R 0
#define GL_GENERATE_MIPMAP 0
#define GL_TEXTURE_PRIORITY 0
#define GL_TEXTURE_BORDER_COLOR 0
#define GL_TEXTURE_INTERNAL_FORMAT 0
#define GL_COMPRESSED_RGBA 0
#define GL_TEXTURE_COMPRESSION_HINT 0
#define GL_CLAMP_TO_BORDER 0
#define GL_PROXY_TEXTURE_2D 0
#define GL_TEXTURE_WIDTH 0
#define GL_TEXTURE_HEIGHT 0
#define GL_OBJECT_LINEAR 20
#define GL_EYE_LINEAR 210
#define GL_REFLECTION_MAP 220
#define GL_SPHERE_MAP 230
#define GL_NORMAL_MAP 240
#define GL_S 250
#define GL_TEXTURE_GEN_MODE 260
#define GL_T 270
#define GL_TEXTURE_ENV 280
#define GL_TEXTURE_ENV_MODE 290
#define GL_MODULATE 30
#define GL_COMBINE 310
#define GL_COMBINE_RGB 1
#define GL_SOURCE0_RGB 2
#define GL_OPERAND0_RGB 3
#define GL_SOURCE1_RGB 4
#define GL_OPERAND1_RGB 5
#define GL_COMBINE_ALPHA 7
#define GL_SOURCE0_ALPHA 6
#define GL_OPERAND0_ALPHA 8
#define GL_SOURCE1_ALPHA 9
#define GL_OPERAND1_ALPHA 10
#define GL_RGB_SCALE 11
#define GL_ALPHA_SCALE 120
#define GL_PROJECTION 130
#define GL_MODELVIEW 140
#define GL_PROJECTION_MATRIX 150
#define GL_SHININESS 160
#define GL_EMISSION 170
#define GL_DIFFUSE 180
#define GL_BACK_LEFT 190
#define GL_BACK_RIGHT 0
#define GL_BACK 0
#define GL_QUERY_RESULT_AVAILABLE 0
#define GL_QUERY_RESULT 0
#define GLDOUBLE GLfloat
#define GL_MAX_TEXTURE_UNITS 16   // Made this up for now
#define GL_MODELVIEW_MATRIX 0	 // Doesn't exist in 2.0, did exist in 1.1
#define FW_GL_GETDOUBLEV(aaa,bbb) 
#define FW_GL_LOAD_IDENTITY(aaa)
#define FW_GL_MATRIX_MODE(aaa) 
#define GL_ERROR_MSG 
#define PRINT_GL_ERROR_IF_ANY(aaa)
#define FW_GL_PIXELSTOREI(aaa,bbb)
#define FW_GL_LINEWIDTH(aaa)
#define FW_GL_POINTSIZE(aaa)
#define FW_GL_SHADEMODEL(aaa)
#define FW_GL_CLEAR(zzz)
#define FW_GL_DEPTHFUNC(zzz)
#define FW_GL_HINT(aaa,bbb)
#define FW_GL_LIGHTFV(aaa,bbb,ccc)
#define FW_GL_BLENDFUNC(aaa,bbb)
#define FW_GL_FRUSTUM(aaa,bbb,ccc,ddd,eee,fff)
#define FW_GL_CLEAR_DEPTH(aaa)
#define FW_GL_LIGHTMODELI(aaa,bbb)
#define FW_GL_LIGHTMODELFV(aaa,bbb)
#define FW_GL_PIXELZOOM(aaa,bbb)
#define FW_GL_COLOR3F(aaa,bbb,ccc)
#define FW_GL_SCISSOR(aaa,bbb,ccc,ddd)
#define FW_GL_ALPHAFUNC(aaa,bbb)
#define FW_GL_ENABLE(aaa)
#define FW_GL_DISABLE(aaa)
#define FW_GL_ENABLECLIENTSTATE(aaa)
#define FW_GL_DISABLECLIENTSTATE(aaa)
#define FW_GL_DRAWARRAYS(xxx,yyy,zzz)
#define FW_GL_TRANSLATE_F(xxx,yyy,zzz)
#define FW_GL_TRANSLATE_D(xxx,yyy,zzz)
#define FW_GL_ROTATE_F(aaa,xxx,yyy,zzz)
#define FW_GL_ROTATE_D(aaa,xxx,yyy,zzz)
#define FW_GL_ROTATE_RADIANS(aaa,xxx,yyy,zzz)
#define FW_GL_SCALE_F(xxx,yyy,zzz)
#define FW_GL_SCALE_D(xxx,yyy,zzz)
#define FW_GL_LOAD_IDENTITY(aaa)
#define FW_GL_PUSH_MATRIX(aaa)
#define FW_GL_POP_MATRIX(aaa)
#define FW_GL_MATRIX_MODE(aaa)
#define FW_GL_GETDOUBLEV(aaa,bbb)
#define FW_GL_PUSH_ATTRIB(aaa)
#define FW_GL_POP_ATTRIB(aaa)
#define FW_GL_WINDOWPOS2I(aaa,bbb)
#define FW_GL_FLUSH()
#define FW_GL_DEPTHMASK(aaa)
#define FW_GL_ORTHO(aaa,bbb,ccc,ddd,eee,fff)
#define FW_GL_COLOR4FV(aaa)
#define FW_GL_RASTERPOS2I(aaa,bbb)
#define FW_GL_CLEAR_COLOR(aaa,bbb,ccc,ddd)
#define FW_GL_CGLFLUSHDRAWABLE(aaa) GL_FALSE
#define FW_GL_VIEWPORT(aaa,bbb,ccc,ddd)
#define FW_GLU_PERSPECTIVE(aaa,bbb,ccc,ddd)
#define FW_GLU_PICK_MATRIX(aaa, bbb, ccc, ddd, eee)
#define FW_GL_MATERIALF(aaa, bbb, ccc)
#define FW_GL_MATERIALFV(aaa, bbb, ccc)
#define FW_GL_COLOR_MATERIAL(aaa, bbb)
#define FW_GL_COLOR4FV(aaa)
#define FW_GL_COLOR3F(aaa, bbb, ccc)
#define FW_GL_COLOR3D(aaa, bbb, ccc)
#define FW_GL_BEGIN(aaa)
#define FW_GL_VERTEX3D(aaa, bbb, ccc)
#define FW_GL_END()
#define FW_GL_VERTEX_POINTER(aaa, bbb, ccc, ddd)
#define FW_GL_NORMAL_POINTER(aaa, bbb, ccc)
#define FW_GLU_SCALE_IMAGE(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii)
#define FW_GL_GET_TEX_LEVEL_PARAMETER_IV(aaa, bbb, ccc, ddd)
#define FW_GLU_UNPROJECT(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii)
#define FW_GLU_PROJECT(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii)
#define FW_GL_POINTSIZE(aaa)
#define FW_GL_LINE_STIPPLE(aaa, bbb)
#define CREATE_SHADER(aaa) 0 
#define SHADER_SOURCE(aaa, bbb, ccc, ddd)
#define COMPILE_SHADER(aaa)
#define ATTACH_SHADER(aaa, bbb)
#define LINK_SHADER(aaa)
#define GET_UNIFORM(aaa, bbb) 0
#define USE_SHADER(aaa)
#define GLUNIFORM2F(aaa,bbb,ccc)
#define GLUNIFORM1I(aaa,bbb)
#define GLUNIFORM4F(aaa,bbb,ccc,ddd,eee)
#define CREATE_PROGRAM 0
#define FW_GL_BEGIN_QUERY(aaa, bbb)
#define FW_GL_END_QUERY(aaa)
#define FW_GLU_TESS_VERTEX(aaa, bbb, ccc)
#define FW_GLU_NEXT_CONTOUR(aaa,bbb)
#define FW_GLU_BEGIN_POLYGON(aaa)
#define FW_GLU_END_POLYGON(aaa)
#define FW_GLU_NEW_TESS() 0
#define FW_GL_FOGFV(aaa, bbb)
#define FW_GL_FOGF(aaa, bbb)
#define FW_GL_FOGI(aaa, bbb)
#define FW_GL_SHADE_MODEL(aaa)
#define SET_TEXTURE_UNIT(aaa)
#define FW_GL_TEXENVI(aaa,bbb,ccc)
#define FW_GL_TEXGENI(aaa,bbb,ccc)
#define FW_GL_TEXCOORDPOINTER(aaa,bbb,ccc,ddd) 
#define FW_GL_BINDTEXTURE(aaa,bbb)


#else  /* ifndef IPHONE */

#define GLDOUBLE GLdouble
#define FW_GL_SHADE_MODEL(aaa) glShadeModel(aaa)
#define FW_GL_FOGFV(aaa, bbb) glFogfv(aaa, bbb)
#define FW_GL_FOGF(aaa, bbb) glFogf(aaa, bbb)
#define FW_GL_FOGI(aaa, bbb) glFogi(aaa, bbb)
#define FW_GLU_NEW_TESS gluNewTess
#define FW_GLU_END_POLYGON(aaa) gluEndPolygon(aaa)
#define FW_GLU_BEGIN_POLYGON(aaa) gluBeginPolygon(aaa)
#define FW_GLU_TESS_VERTEX(aaa, bbb, ccc) gluTessVertex(aaa, bbb, ccc)
#define FW_GLU_NEXT_CONTOUR(aaa, bbb) gluNextContour(aaa,bbb)
#define FW_GL_BEGIN_QUERY(aaa, bbb) glBeginQuery(aaa, bbb)
#define FW_GL_END_QUERY(aaa) glEndQuery(aaa)
#define FW_GL_LINE_STIPPLE(aaa, bbb) glLineStipple(aaa, bbb)
#define FW_GLU_UNPROJECT(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii) gluUnProject(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii)
#define FW_GLU_PROJECT(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii) gluProject(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii)
#define FW_GL_END() glEnd()
#define FW_GL_VERTEX3D(aaa, bbb, ccc) glVertex3d(aaa, bbb, ccc)
#define FW_GL_VERTEX_POINTER(aaa, bbb, ccc, ddd) glVertexPointer(aaa, bbb, ccc, ddd)
#define FW_GL_NORMAL_POINTER(aaa, bbb, ccc) glNormalPointer(aaa, bbb, ccc)
#define FW_GL_BEGIN(aaa) glBegin(aaa)
#define FW_GL_MATERIALF(aaa, bbb, ccc) glMaterialf(aaa, bbb, ccc)
#define FW_GL_MATERIALFV(aaa, bbb, ccc) glMaterialfv(aaa, bbb, ccc)
#define FW_GL_COLOR_MATERIAL(aaa, bbb) glColorMaterial(aaa, bbb)
#define FW_GL_COLOR3D(aaa, bbb, ccc) glColor3d(aaa, bbb, ccc)
#define FW_GLU_SCALE_IMAGE(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii) gluScaleImage(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii)
#define FW_GL_GET_TEX_LEVEL_PARAMETER_IV(aaa, bbb, ccc, ddd) glGetTexLevelParameteriv(aaa, bbb, ccc, ddd)
#define SET_TEXTURE_UNIT(aaa) { glActiveTexture(GL_TEXTURE0+aaa); glClientActiveTexture(GL_TEXTURE0+aaa); }
#define FW_GL_TEXENVI(aaa,bbb,ccc) glTexEnvi(aaa,bbb,ccc)
#define FW_GL_TEXGENI(aaa,bbb,ccc) glTexGeni(aaa,bbb,ccc)
#define FW_GL_TEXCOORDPOINTER(aaa,bbb,ccc,ddd) glTexCoordPointer(aaa,bbb,ccc,ddd)
#define FW_GL_BINDTEXTURE(aaa,bbb) glBindTexture(aaa,bbb)
#endif /* ifdef IPHONE */

#endif /* __LIBFREEWRL_DISPLAY_H__ */
