/*
  $Id: display.h,v 1.71 2010/03/22 19:16:01 crc_canada Exp $

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


/**
 * Specific platform : Mac
 */
#ifdef AQUA

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

#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLTypes.h>

#include <AGL/AGL.h> 
#endif /* defined IPHONE */
#endif /* defined TARGET_AQUA */

#ifdef _MSC_VER /* TARGET_WIN32 */
#ifndef AQUA

/* Nothing special :P ... */
#include <GL/glew.h>
#define SENSOR_CURSOR sensor_cursor32();
#define ARROW_CURSOR arrow_cursor32();
#define ERROR 0
#endif
#endif /* TARGET_WIN32 */


#if !defined (_MSC_VER) && !defined (TARGET_AQUA) /* not aqua and not win32, ie linux */
#ifdef HAVE_GLEW_H
#include <GL/glew.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glx.h>
#endif
#endif

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
	GLuint myShaderProgram;
	GLint myMaterialAmbient;
	GLint myMaterialDiffuse;
	GLint myMaterialSpecular;
	GLint myMaterialShininess;
	GLint myMaterialEmission;
	GLint lightState;
	GLint lightAmbient;
	GLint lightDiffuse;
	GLint lightSpecular;
	GLint lightPosition;
} s_shader_capabilities_t;

typedef struct {

	const char *renderer; /* replace GL_REN */
	const char *version;
	const char *vendor;
	const char *extensions;
	float versionf;
	bool have_GL_VERSION_1_1;
	bool have_GL_VERSION_1_2;
	bool have_GL_VERSION_1_3;
	bool have_GL_VERSION_1_4;
	bool have_GL_VERSION_1_5;
	bool have_GL_VERSION_2_0;
	bool have_GL_VERSION_2_1;
	bool have_GL_VERSION_3_0;

	bool av_multitexture; /* Multi textures available ? */
	bool av_glsl_shaders; /* GLSL shaders available ? */ 
	bool av_npot_texture; /* Non power of 2 textures available ? */
	bool av_texture_rect; /* Rectangle textures available ? */
	bool av_occlusion_q;  /* Occlusion query available ? */
	
	int texture_units;
	int max_texture_size;
	float anisotropicDegree;

	/* for general Appearance Shaders */
	bool haveGenericAppearanceShader;  /* do immediate mode or shader? */
	s_shader_capabilities_t shaderArrays[10]; /* one element for each shader_type */
} s_renderer_capabilities_t;

typedef enum shader_type {
	noAppearanceNoMaterialShader,
	noLightNoTextureAppearanceShader,
	genericHeadlightNoTextureAppearanceShader,
	multiLightNoTextureAppearanceShader,
	headlightOneTextureAppearanceShader,
	headlightMultiTextureAppearanceShader,
	multiLightMultiTextureAppearanceShader
} shader_type_t;


extern s_renderer_capabilities_t rdr_caps;

#ifdef TARGET_AQUA
#ifndef IPHONE
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

extern int PaneClipct;
extern int PaneClipcb;
extern int PaneClipcr;
extern int PaneClipcl;
extern int PaneClipwidth;
extern int PaneClipheight;
extern int PaneClipChanged;

# if defined(WANT_MULTI_OPENGL_THREADS)
/* multi-threaded OpenGL contexts - works on OS X, kind of ok on Linux, but
   blows plugins out of the water, because of the XLib threaded call in FrontEnd
   not working that well... */
#undef DO_MULTI_OPENGL_THREADS
#endif

#include "OpenGL/glu.h"
#endif
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

/**
 * General : all systems
 */

extern GLenum _global_gl_err;
#ifndef IPHONE
#define PRINT_GL_ERROR_IF_ANY(_where) if (global_print_opengl_errors) { \
                                              GLenum _global_gl_err = glGetError(); \
                                              while (_global_gl_err != GL_NO_ERROR) { \
                                                 char *_str = (char *) gluErrorString(_global_gl_err); \
                                                 fprintf(stderr, "GL error: %s, here: %s\n", _str, _where); \
                                                 _global_gl_err = glGetError(); \
                                              } \
                                           } 
#endif

void resetGeometry();
void setScreenDim(int wi, int he);

/* GLSL variables */
/* Versions 1.5 and above have shaders */
#ifdef GL_VERSION_2_0
	#define HAVE_SHADERS
	#define VERTEX_SHADER GL_VERTEX_SHADER
	#define FRAGMENT_SHADER GL_FRAGMENT_SHADER
	#define SHADER_SOURCE glShaderSource
	#define COMPILE_SHADER glCompileShader
	#define CREATE_PROGRAM glCreateProgram();
	#define CREATE_SHADER glCreateShader
	#define ATTACH_SHADER glAttachShader
	#define LINK_SHADER glLinkProgram
	#define DELETE_SHADER glDeleteShader
	#define DELETE_PROGRAM glDeleteProgram
	#define USE_SHADER glUseProgram
	#define GET_SHADER_INFO glGetShaderiv
	#define LINK_STATUS GL_LINK_STATUS
	#define COMPILE_STATUS GL_COMPILE_STATUS
	#define GET_UNIFORM(aaa,bbb) glGetUniformLocation(aaa,bbb)
	#define GET_ATTRIB(aaa,bbb) glGetAttribLocation(aaa,bbb)
	#define GLUNIFORM1I glUniform1i
	#define GLUNIFORM1F glUniform1f
	#define GLUNIFORM2F glUniform2f
	#define GLUNIFORM3F glUniform3f
	#define GLUNIFORM4F glUniform4f
	#define GLUNIFORM1IV glUniform1iv
	#define GLUNIFORM1FV glUniform1fv
	#define GLUNIFORM2FV glUniform2fv
	#define GLUNIFORM3FV glUniform3fv
	#define GLUNIFORM4FV glUniform4fv
#else
#ifdef GL_VERSION_1_5
	#define HAVE_SHADERS
	#define VERTEX_SHADER GL_VERTEX_SHADER_ARB
	#define FRAGMENT_SHADER GL_FRAGMENT_SHADER_ARB
	#define SHADER_SOURCE glShaderSourceARB
	#define COMPILE_SHADER glCompileShaderARB
	#define CREATE_PROGRAM glCreateProgramObjectARB();
	#define CREATE_SHADER glCreateShaderARB
	#define ATTACH_SHADER glAttachObjectARB
	#define LINK_SHADER glLinkProgramARB
	#define DELETE_SHADER glDeleteShaderARB
	#define DELETE_PROGRAM glDeleteProgramARB
	#define USE_SHADER  glUseProgramObjectARB
	#define CREATE_SHADER glCreateShaderObjectARB
	#define GET_SHADER_INFO glGetObjectParameterivARB
	#define LINK_STATUS GL_OBJECT_LINK_STATUS_ARB
	#define COMPILE_STATUS GL_OBJECT_COMPILE_STATUS_ARB
	#define GET_UNIFORM(aaa,bbb) glGetUniformLocationARB(aaa,bbb)
	#define GET_ATTRIB(aaa,bbb) glGetAttribLocationARB(aaa,bbb)
	#define GLUNIFORM1F glUniform1fARB
	#define GLUNIFORM1I glUniform1iARB
	#define GLUNIFORM2F glUniform2fARB
	#define GLUNIFORM3F glUniform3fARB
	#define GLUNIFORM4F glUniform4fARB
	#define GLUNIFORM1IV glUniform1ivARB
	#define GLUNIFORM1FV glUniform1fvARB
	#define GLUNIFORM2FV glUniform2fvARB
	#define GLUNIFORM3FV glUniform3fvARB
	#define GLUNIFORM4FV glUniform4fvARB
#endif
#endif

/* OpenGL-2.x and OpenGL-3.x "desktop" systems calls */
#ifndef IPHONE
	/****************************************************************/
	/* First - any platform specifics to do? 			*/
	/****************************************************************/

	#if defined(_MSC_VER)
		#define FW_GL_PUSH_MATRIX(...) fw_glPushMatrix()
		#define FW_GL_POP_MATRIX(...) fw_glPopMatrix()
		#define FW_GL_SWAPBUFFERS SwapBuffers(wglGetCurrentDC());
	#endif

	#if defined (TARGET_AQUA)
		#define FW_GL_SWAPBUFFERS { \
			CGLError err = FW_GL_CGLFLUSHDRAWABLE(myglobalContext); \
			if (err != kCGLNoError) printf ("FW_GL_CGLFLUSHDRAWABLE error %d\n",err); }

	#endif

	#if defined (TARGET_X11) || defined (TARGET_MOTIF)
		#define FW_GL_SWAPBUFFERS glXSwapBuffers(Xdpy,GLwin);
	#endif


	/****************************************************************/
	/* Second - things that might be specific to one platform;	*/
	/*	this is the "catch for other OS" here 			*/
	/****************************************************************/

	#if !defined (FW_GL_PUSH_MATRIX)
		#define FW_GL_PUSH_MATRIX(aaa) fw_glPushMatrix()
		#define FW_GL_POP_MATRIX(aaa) fw_glPopMatrix()
	#endif 

	/****************************************************************/
	/* Third - common across all platforms				*/
	/****************************************************************/

	#define FW_GLU_DELETETESS(aaa) gluDeleteTess(aaa)
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
		DEBUG_MSG("fw_glRotatef\t%6.2f %6.2f %6.2f %6.2f\tat %s:%d\n",aaa,xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_ROTATE_D(aaa,xxx,yyy,zzz) \
		{fw_glRotated(aaa,xxx,yyy,zzz); \
		DEBUG_MSG("fw_glRotated\t%6.2f %6.2f %6.2f %6.2f\tat %s:%d\n",aaa,xxx,yyy,zzz,__FILE__,__LINE__);}
	#define FW_GL_ROTATE_RADIANS(aaa,xxx,yyy,zzz) fw_glRotateRad(aaa,xxx,yyy,zzz)
	#define FW_GL_SCALE_F(xxx,yyy,zzz) fw_glScalef(xxx,yyy,zzz)
	#define FW_GL_SCALE_D(xxx,yyy,zzz) fw_glScaled(xxx,yyy,zzz)
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
	#define FW_GL_LIGHTFV(aaa,bbb,ccc) fwglLightfv(aaa,bbb,ccc);
	#define FW_GL_LIGHTF(aaa,bbb,ccc) fwglLightf(aaa,bbb,ccc);
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
	#define FW_GL_TEXCOORD_POINTER(aaa,bbb,ccc,ddd) glTexCoordPointer(aaa,bbb,ccc,ddd)
	#define FW_GL_BINDTEXTURE(aaa,bbb) glBindTexture(aaa,bbb)


	#define GLDOUBLE GLdouble
	#define FW_GL_FOGFV(aaa, bbb) glFogfv(aaa, bbb)
	#define FW_GL_FOGF(aaa, bbb) glFogf(aaa, bbb)
	#define FW_GL_FOGI(aaa, bbb) glFogi(aaa, bbb)
	#define FW_GLU_NEW_TESS gluNewTess
	#define FW_GLU_END_POLYGON(aaa) gluEndPolygon(aaa)
	#define FW_GLU_BEGIN_POLYGON(aaa) gluBeginPolygon(aaa)
	#define FW_GLU_TESS_VERTEX(aaa, bbb, ccc) gluTessVertex(aaa, bbb, ccc)
	#define FW_GLU_TESS_CALLBACK(aaa, bbb, ccc) gluTessCallback(aaa,bbb,ccc);
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
	
	#define FW_GL_VERTEX3F(aaa, bbb, ccc) glVertex3f(aaa, bbb, ccc)
	#define FW_GL_DRAWELEMENTS(aaa,bbb,ccc,ddd) glDrawElements(aaa,bbb,ccc,ddd)
	#define FW_GL_GETSTRING(aaa) glGetString(aaa)
	#define FW_GL_COLOR_POINTER(aaa, bbb,ccc,ddd) glColorPointer(aaa, bbb,ccc,ddd)
	#define FW_GL_DELETETEXTURES(aaa,bbb) glDeleteTextures(aaa,bbb);
	#define FW_GL_COLOR3FV(aaa) glColor3fv(aaa);
	#define FW_GL_LOADMATRIXD(aaa) glLoadMatrixd(aaa)
	#define FW_GL_GETINTEGERV(aaa,bbb) glGetIntegerv(aaa,bbb);
	#define FW_GL_GETFLOATV(aaa,bbb) glGetFloatv(aaa,bbb);
	#define FW_GL_COLOR4F(aaa,bbb,ccc,ddd) glColor4f(aaa,bbb,ccc,ddd);
	#define FW_GL_COLOR4FV(aaa) glColor4fv(aaa);
	#define FW_GL_FRONTFACE(aaa) glFrontFace(aaa);
	#define FW_GL_GENLISTS(aaa) glGenLists(aaa)
	#define FW_GL_GENTEXTURES(aaa,bbb) glGenTextures(aaa,bbb)
	#define FW_GL_GETBOOLEANV(aaa,bbb) glGetBooleanv(aaa,bbb)
	#define FW_GL_NEWLIST(aaa,bbb) glNewList(aaa,bbb)
	#define FW_GL_NORMAL3F(aaa,bbb,ccc) glNormal3f(aaa,bbb,ccc)

	#define FW_GL_READPIXELS(aaa,bbb,ccc,ddd,eee,fff,ggg) glReadPixels(aaa,bbb,ccc,ddd,eee,fff,ggg) 
	#define FW_GL_TEXIMAGE2D(aaa,bbb,ccc,ddd,eee,fff,ggg,hhh,iii) glTexImage2D(aaa,bbb,ccc,ddd,eee,fff,ggg,hhh,iii)
	#define FW_GL_TEXPARAMETERF(aaa,bbb,ccc) glTexParameterf(aaa,bbb,ccc)
	#define FW_GL_TEXPARAMETERI(aaa,bbb,ccc) glTexParameteri(aaa,bbb,ccc)
	#define FW_GL_TEXPARAMETERFV(aaa,bbb,ccc) glTexParameterfv(aaa,bbb,ccc)
        #define FW_GL_GETQUERYOBJECTIV(aaa,bbb,ccc) glGetQueryObjectiv(aaa,bbb,ccc)
	#define FW_GL_GENQUERIES(aaa,bbb) glGenQueries(aaa,bbb)
	#define FW_GL_DELETE_QUERIES(aaa,bbb) glDeleteQueries(aaa,bbb)
	




#endif /* NDEF IPHONE */




#ifdef IPHONE

/* JAS - Sarah made these up to allow for compiling under OpenGL-ES 2.0. 
	
	Notes: 	- The constants NEED to be removed, and official ones from the framework used.
		- function calls; many of the nulled functions exist in OpenGL-ES 2.0; others will
		  need to be replicated.

	Check out: http://www.khronos.org/opengles/sdk/docs/man/

*/
	
	/* quirks */
	#define VERTEX_SHADER GL_VERTEX_SHADER
	#define FRAGMENT_SHADER GL_FRAGMENT_SHADER
	#define PATH_MAX 5000

	/****************************************************************/
	/*	CONSTANTS not available in OpenGL-ES 2.0  		*/
	/****************************************************************/

	#define COMPILE_STATUS GL_COMPILE_STATUS 
	#define CREATE_PROGRAM 0;
	#define FL_CLAMP_TO_BORDER 0
	#define GLDOUBLE GLfloat
	#define GLU_BEGIN                            100100
	#define GLU_EDGE_FLAG                        100104
	#define GLU_END                              100102
	#define GLU_ERROR                            100103
	#define GLU_TESS_COMBINE                     100105
	#define GLU_TESS_COMBINE_DATA                100111
	#define GLU_TESS_VERTEX                      100101
	#define GLU_UNKNOWN                          100124
	#define GLU_VERTEX                           100101
	#define GL_ADD                            0x0104
	#define GL_ADD_SIGNED                     0x8574
	#define GL_ALPHA_SCALE                    0x0D1C
	#define GL_AMBIENT                        0x1200
	#define GL_BACK_LEFT                      0x0402
	#define GL_BACK_RIGHT                     0x0403
	#define GL_CLAMP                          0x2900
	#define GL_CLAMP_TO_BORDER                0x812D
	#define GL_COLOR_MATERIAL                 0x0B57
	#define GL_COMBINE                        0x8570
	#define GL_COMBINE_ALPHA                  0x8572
	#define GL_COMBINE_RGB                    0x8571
	#define GL_COMPILE_STATUS                 0x8B81
	#define GL_COMPRESSED_RGBA                0x84EE
	#define GL_CONSTANT_ATTENUATION           0x1207
	#define GL_DIFFUSE                        0x1201
	#define GL_DOT3_RGB                       0x86AE
	#define GL_EMISSION                       0x1600
	#define GL_EXP                            0x0800
	#define GL_EYE_LINEAR                     0x2400
	#define GL_FOG                            0x0B60
	#define GL_FOG_COLOR                      0x0B66
	#define GL_FOG_DENSITY                    0x0B62
	#define GL_FOG_END                        0x0B64
	#define GL_FOG_MODE                       0x0B65
	#define GL_FOG_START                      0x0B63
	#define GL_GENERATE_MIPMAP                0x8191
	#define GL_LIGHT0                         0x4000
	#define GL_LIGHT1                         0x4001
	#define GL_LIGHT2                         0x4002
	#define GL_LIGHT3                         0x4003
	#define GL_LIGHT4                         0x4004
	#define GL_LIGHT5                         0x4005
	#define GL_LIGHT6                         0x4006
	#define GL_LIGHTING                       0x0B50
	#define GL_LINE_STIPPLE                   0x0B24
	#define GL_LINEAR_ATTENUATION             0x1208
	#define GL_MAX_TEXTURE_UNITS              0x84E2
	#define GL_MODELVIEW                      0x1700
	#define GL_MODELVIEW_MATRIX               0x0BA6
	#define GL_MODULATE                       0x2100
	#define GL_NORMAL_MAP                     0x8511
	#define GL_NORMALIZE                      0x0BA1
	#define GL_OBJECT_LINEAR                  0x2401
	#define GL_OPERAND0_ALPHA                 0x8598
	#define GL_OPERAND0_RGB                   0x8590
	#define GL_OPERAND1_ALPHA                 0x8599
	#define GL_OPERAND1_RGB                   0x8591
	#define GL_PERSPECTIVE_CORRECTION_HINT    0x0C50
	#define GL_POSITION                       0x1203
	#define GL_PREVIOUS                       0x8578
	#define GL_PROJECTION                     0x1701
	#define GL_PROJECTION_MATRIX              0x0BA7
	#define GL_PROXY_TEXTURE_2D               0x8064
	#define GL_QUADRATIC_ATTENUATION          0x1209
	#define GL_QUERY_RESULT                   0x8866
	#define GL_QUERY_RESULT_AVAILABLE         0x8867
	#define GL_R                              0x2002
	#define GL_REFLECTION_MAP                 0x8512
	#define GL_RESCALE_NORMAL                 0x803A
	#define GL_RGBA8                          0x8058
	#define GL_RGB_SCALE                      0x8573
	#define GL_S                              0x2000
	#define GL_SAMPLES_PASSED                 0x8914
	#define GL_SHININESS                      0x1601
	#define GL_SMOOTH                         0x1D01
	#define GL_SOURCE0_ALPHA                  0x8588
	#define GL_SOURCE0_RGB                    0x8580
	#define GL_SOURCE1_ALPHA                  0x8589
	#define GL_SOURCE1_RGB                    0x8581
	#define GL_SPECULAR                       0x1202
	#define GL_SPHERE_MAP                     0x2402
	#define GL_SPOT_CUTOFF                    0x1206
	#define GL_SPOT_DIRECTION                 0x1204
	#define GL_SPOT_EXPONENT                  0x1205
	#define GL_STEREO                         0x0C33
	#define GL_SUBTRACT                       0x84E7
	#define GL_T                              0x2001
	#define GL_TEXTURE_BORDER_COLOR           0x1004
	#define GL_TEXTURE_COMPRESSION_HINT       0x84EF
	#define GL_TEXTURE_ENV                    0x2300
	#define GL_TEXTURE_ENV_MODE               0x2200
	#define GL_TEXTURE_GEN_MODE               0x2500
	#define GL_TEXTURE_GEN_R                  0x0C62
	#define GL_TEXTURE_GEN_S                  0x0C60
	#define GL_TEXTURE_GEN_T                  0x0C61
	#define GL_TEXTURE_HEIGHT                 0x1001
	#define GL_TEXTURE_INTERNAL_FORMAT        0x1003
	#define GL_TEXTURE_PRIORITY               0x8066
	#define GL_TEXTURE_WIDTH                  0x1000
	#define GL_TEXTURE_WRAP_R                 0x8072
	#define GL_QUADS                          0x0007
	#define GL_QUAD_STRIP                     0x0008

	/****************************************************************/
	/*	FUNCTIONS not available in OpenGL-ES 2.0  		*/
	/****************************************************************/

	/****************************************************************/
	/*	FUNCTIONS available in OpenGL-ES 2.0	  		*/
	/* see http://www.khronos.org/opengles/headers/2_0/GL.h		*/
	/****************************************************************/

	#define CREATE_SHADER glCreateShader
	#define ATTACH_SHADER glAttachShader
	#define COMPILE_SHADER glCompileShader
	#define DELETE_SHADER glDeleteShader
	#define DELETE_PROGRAM glDeleteProgram
	#define GET_ATTRIB(aaa,bbb) glGetAttribLocation(aaa,bbb)
	#define GET_UNIFORM(aaa,bbb) glGetUniformLocation(aaa,bbb)
	#define GET_ATTRIB(aaa,bbb) glGetAttribLocation(aaa,bbb)
	#define USE_SHADER glUseProgram
	#define GET_SHADER_INFO glGetShaderiv
	#define GLUNIFORM1I glUniform1i
	#define GLUNIFORM1F glUniform1f
	#define GLUNIFORM2F glUniform2f
	#define GLUNIFORM3F glUniform3f
	#define GLUNIFORM4F glUniform4f
	#define GLUNIFORM1IV glUniform1iv
	#define GLUNIFORM1FV glUniform1fv
	#define GLUNIFORM2FV glUniform2fv
	#define GLUNIFORM3FV glUniform3fv
	#define GLUNIFORM4FV glUniform4fv
	#define FW_GL_FRONTFACE(aaa) glFrontFace(aaa);
	#define FW_GL_DRAWARRAYS(xxx,yyy,zzz) glDrawArrays(xxx,yyy,zzz)
	#define FW_GL_DEPTHMASK(aaa) glDepthMask(aaa);
	#define FW_GL_ENABLE(aaa) glEnable(aaa)
	#define FW_GL_DISABLE(aaa) glDisable(aaa); 
	#define FW_GL_POINTSIZE(aaa) glPointSize(aaa); 
	#define FW_GL_LINEWIDTH(aaa) glLineWidth(aaa);
	#define FW_GL_CLEAR(zzz) glClear(zzz); 
	#define FW_GL_DRAWELEMENTS(aaa,bbb,ccc,ddd) glDrawElements(aaa,bbb,ccc,ddd)
	#define FW_GL_HINT(aaa,bbb) glHint(aaa,bbb); 
	#define FW_GL_GENTEXTURES(aaa,bbb) glGenTextures(aaa,bbb)
	#define FW_GL_GETBOOLEANV(aaa,bbb) glGetBooleanv(aaa,bbb)



	/****************************************************************/
	#define FW_GLU_DELETETESS(aaa)
	#define FW_GL_GETDOUBLEV(aaa,bbb)  printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_LOAD_IDENTITY(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_MATRIX_MODE(aaa)  printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define GL_ERROR_MSG  printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define PRINT_GL_ERROR_IF_ANY(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_PIXELSTOREI(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_SHADEMODEL(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_DEPTHFUNC(zzz) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_LIGHTFV(aaa,bbb,ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_LIGHTF(aaa,bbb,ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_BLENDFUNC(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_FRUSTUM(aaa,bbb,ccc,ddd,eee,fff) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_CLEAR_DEPTH(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_LIGHTMODELI(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_LIGHTMODELFV(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_PIXELZOOM(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_COLOR3F(aaa,bbb,ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_SCISSOR(aaa,bbb,ccc,ddd) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_ALPHAFUNC(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_ENABLECLIENTSTATE(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_DISABLECLIENTSTATE(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_TRANSLATE_F(xxx,yyy,zzz) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_TRANSLATE_D(xxx,yyy,zzz) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_ROTATE_F(aaa,xxx,yyy,zzz) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_ROTATE_D(aaa,xxx,yyy,zzz) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_ROTATE_RADIANS(aaa,xxx,yyy,zzz) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_SCALE_F(xxx,yyy,zzz) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_SCALE_D(xxx,yyy,zzz) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_LOAD_IDENTITY(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_PUSH_MATRIX(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_POP_MATRIX(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_MATRIX_MODE(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_GETDOUBLEV(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_PUSH_ATTRIB(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_POP_ATTRIB(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_WINDOWPOS2I(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_FLUSH() printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_ORTHO(aaa,bbb,ccc,ddd,eee,fff) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_COLOR4FV(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_RASTERPOS2I(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_CLEAR_COLOR(aaa,bbb,ccc,ddd) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_CGLFLUSHDRAWABLE(aaa) GL_FALSE printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_VIEWPORT(aaa,bbb,ccc,ddd) printf("WRONG"); printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GLU_PERSPECTIVE(aaa,bbb,ccc,ddd) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GLU_PICK_MATRIX(aaa, bbb, ccc, ddd, eee) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_MATERIALF(aaa, bbb, ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_MATERIALFV(aaa, bbb, ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_COLOR_MATERIAL(aaa, bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_COLOR4FV(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_COLOR3F(aaa, bbb, ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_COLOR3D(aaa, bbb, ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_BEGIN(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_VERTEX3D(aaa, bbb, ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_END() printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_VERTEX_POINTER(aaa, bbb, ccc, ddd) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_NORMAL_POINTER(aaa, bbb, ccc) printf("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GLU_SCALE_IMAGE(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_GET_TEX_LEVEL_PARAMETER_IV(aaa, bbb, ccc, ddd) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GLU_UNPROJECT(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GLU_PROJECT(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_LINE_STIPPLE(aaa, bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define SHADER_SOURCE(aaa, bbb, ccc, ddd) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define LINK_SHADER(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_BEGIN_QUERY(aaa, bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_END_QUERY(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GLU_TESS_VERTEX(aaa, bbb, ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GLU_TESS_CALLBACK(aaa, bbb, ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GLU_NEXT_CONTOUR(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GLU_BEGIN_POLYGON(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GLU_END_POLYGON(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GLU_NEW_TESS() 0
	#define FW_GL_FOGFV(aaa, bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_FOGF(aaa, bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_FOGI(aaa, bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define SET_TEXTURE_UNIT(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_TEXENVI(aaa,bbb,ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_TEXGENI(aaa,bbb,ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_TEXCOORD_POINTER(aaa,bbb,ccc,ddd)  printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_BINDTEXTURE(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_SWAPBUFFERS printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)


	#define FW_GL_COLOR_POINTER(aaa, bbb,ccc,ddd) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_DELETETEXTURES(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_DRAWBUFFER printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_ENDLIST() printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_GETFLOATV(aaa,bbb)  printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_GETINTEGERV(aaa,bbb)  printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_GETSTRING(aaa) 0 
	#define FW_GL_LISTBASE(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_LOADMATRIX(aaa)  printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_VERTEX3F(aaa, bbb, ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_COLOR_POINTER(aaa, bbb,ccc,ddd) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_DELETETEXTURES(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_COLOR3FV(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_LOADMATRIXD(aaa) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_COLOR4F(aaa,bbb,ccc,ddd) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_GENLISTS(aaa) 0
	#define FW_GL_NEWLIST(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_NORMAL3F(aaa,bbb,ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_READPIXELS(aaa,bbb,ccc,ddd,eee,fff,ggg) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_TEXIMAGE2D(aaa,bbb,ccc,ddd,eee,fff,ggg,hhh,iii) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_TEXPARAMETERF(aaa,bbb,ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_TEXPARAMETERI(aaa,bbb,ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_TEXPARAMETERFV(aaa,bbb,ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_GETQUERYOBJECTIV(aaa,bbb,ccc) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_GENQUERIES(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)
	#define FW_GL_DELETE_QUERIES(aaa,bbb) printf ("subbed openglES call at %s:%d \n",__FILE__,__LINE__)

	

#endif /* ifdef IPHONE */

#endif /* __LIBFREEWRL_DISPLAY_H__ */
