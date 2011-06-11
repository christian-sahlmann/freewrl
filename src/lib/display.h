/*
  $Id: display.h,v 1.128 2011/06/11 01:29:59 couannette Exp $

  FreeWRL support library.

Purpose:
  Handle platform specific includes about windowing systems and OpenGL.
  Try to present a generic interface to the rest of FreeWRL library.

Data:

Functions:
  
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

#if 1 || defined(HAVE_FV_INLIB)
#define KEEP_FV_INLIB 1
#define KEEP_X11_INLIB 1
#else
#define KEEP_FV_INLIB 0
#define KEEP_X11_INLIB 0
#endif

#ifndef __LIBFREEWRL_DISPLAY_H__
#define __LIBFREEWRL_DISPLAY_H__

/**
 * Specific platform : Mac
 */
#ifdef AQUA

#ifdef IPHONE
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
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
#define ERROR 0
#endif
#endif /* TARGET_WIN32 */


#if !defined (_MSC_VER) && !defined (TARGET_AQUA) /* not aqua and not win32, ie linux */
#ifdef HAVE_GLEW_H
#include <GL/glew.h>
#else
#ifndef AQUA
#if !defined(_ANDROID)
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glx.h>
#else
/* ANDROID NDK */
//#include <GLES/gl.h>
//#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

typedef char GLchar;

#endif /*ANDROID_NDK*/
#endif
#endif
#endif



/* generic - OpenGL ES 2.0 does not have doubles */
#ifdef GL_ES_VERSION_2_0
	#define GLDOUBLE double
	#define DOUBLE_MAX fmax
	#define DOUBLE_MIN fmin
	#undef HAVE_GEOMETRY_SHADERS
#else
	#define GLDOUBLE GLdouble
	#define DOUBLE_MAX max
	#define DOUBLE_MIN min
#endif


/* face culling */
#ifdef GL_ES_VERSION_2_0
#define CULL_FACE(v) /* printf ("nodeSolid %d getAppearanceProperties()->cullFace %d GL_FALSE %d FALSE %d\n",v,getAppearanceProperties()->cullFace,GL_FALSE,FALSE); */ \
                if (v != getAppearanceProperties()->cullFace) {    \
                        getAppearanceProperties()->cullFace = v; \
                }
	#define CULL_FACE_INITIALIZE getAppearanceProperties()->cullFace=FALSE; 
#else
#define CULL_FACE(v) /* printf ("nodeSolid %d getAppearanceProperties()->cullFace %d GL_FALSE %d FALSE %d\n",v,getAppearanceProperties()->cullFace,GL_FALSE,FALSE); */ \
                if (v != getAppearanceProperties()->cullFace) {    \
                        getAppearanceProperties()->cullFace = v; \
                        if (getAppearanceProperties()->cullFace == TRUE) {FW_GL_ENABLE(GL_CULL_FACE);}\
                        else { FW_GL_DISABLE(GL_CULL_FACE);} \
                }
	#define CULL_FACE_INITIALIZE getAppearanceProperties()->cullFace=FALSE; FW_GL_DISABLE(GL_CULL_FACE);
#endif

#define DISABLE_CULL_FACE CULL_FACE(FALSE)
#define ENABLE_CULL_FACE CULL_FACE(TRUE)


#ifdef GL_ES_VERSION_2_0
#if !defined(PATH_MAX)
	#define PATH_MAX 5000
#endif

	/* as we now do our own matrix manipulation, we can change these; note that OpenGL-ES 2.0 does not
	   have these by default */
	#define GL_MODELVIEW                   0x1700
	#define GL_MODELVIEW_MATRIX            0x0BA6
	#define GL_PROJECTION                  0x1701
	#define GL_PROJECTION_MATRIX           0x0BA7
	#define GL_TEXTURE_MATRIX		0x0BA8

	/* same with material properties - we do our own, but need some constants, so... */
	#define GL_SHININESS                      0x1601
	#define GL_DIFFUSE                        0x1201
	#define GL_AMBIENT                        0x1200
	#define GL_SPECULAR                       0x1202
	#define GL_EMISSION                       0x1600
	#define GL_ENABLE_BIT				0x00002000
/*
	#define GL_LIGHTING                       0x0B50
*/
	#define GL_LIGHT_MODEL_COLOR_CONTROL		0x81F8
	#define GL_SEPARATE_SPECULAR_COLOR		0x81FA
	#define GL_LIGHT_MODEL_TWO_SIDE			0x0B52
	#define GL_LIGHT_MODEL_LOCAL_VIEWER		0x0B51
	#define GL_LIGHT_MODEL_AMBIENT			0x0B53
	#define GL_LIGHT0                         0x4000

	/* and, one buffer only, not stereo viewing */
	#define GL_BACK_LEFT			GL_BACK
	#define GL_BACK_RIGHT			GL_BACK
	#define GL_STEREO                         0x0C33

	/* we do not do occlusion queries yet; have to figure this one out */
	#define GL_QUERY_RESULT                   0x8866
	#define GL_QUERY_RESULT_AVAILABLE         0x8867
	#define GL_SAMPLES_PASSED                 0x8914

	/* and, we have shaders, but not OpenGL 2.0, so we just put these here */
	#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
	#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
	#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
	#define GL_RGBA8				0x8058
	#define GL_RGB8					0x8051
	#define GL_BGR                            0x80E0
	#define GL_RGB5					0x8050

#define GL_EDGE_FLAG_ARRAY			0x8079
#define GL_INDEX_ARRAY				0x8077
#define GL_FOG_COORD_ARRAY                GL_FOG_COORDINATE_ARRAY
#define GL_SECONDARY_COLOR_ARRAY          0x845E
	#define GL_LINE_STIPPLE                   0x0B24
	#define GL_VERTEX_ARRAY                   0x8074
	#define GL_NORMAL_ARRAY                   0x8075
	#define GL_TEXTURE_COORD_ARRAY            0x8078
	#define GL_COLOR_ARRAY                    0x8076
	#define GL_OBJECT_LINEAR                  0x2401
	#define GL_EYE_LINEAR                     0x2400
	#define GL_REFLECTION_MAP                 0x8512
	#define GL_SPHERE_MAP                     0x2402
	#define GL_NORMAL_MAP                     0x8511
	#define GL_S                              0x2000
	#define GL_TEXTURE_GEN_MODE               0x2500
	#define GL_T                              0x2001
	#define GL_TEXTURE_GEN_S                  0x0C60
	#define GL_TEXTURE_GEN_T                  0x0C61
	#define GL_TEXTURE_ENV                    0x2300
	#define GL_TEXTURE_ENV_MODE               0x2200
	#define GL_MODULATE                       0x2100
	#define GL_COMBINE                        0x8570
	#define GL_COMBINE_RGB                    0x8571
	#define GL_SOURCE0_RGB                    0x8580
	#define GL_OPERAND0_RGB                   0x8590
	#define GL_SOURCE1_RGB                    0x8581
	#define GL_OPERAND1_RGB                   0x8591
	#define GL_COMBINE_ALPHA                  0x8572
	#define GL_SOURCE0_ALPHA                  0x8588
	#define GL_OPERAND0_ALPHA                 0x8598
	#define GL_RGB_SCALE                      0x8573
	#define GL_ALPHA_SCALE                    0x0D1C
	#define GL_SOURCE1_ALPHA                  0x8589
	#define GL_OPERAND1_ALPHA                 0x8599
	#define GL_TEXTURE_GEN_S                  0x0C60
	#define GL_TEXTURE_GEN_T                  0x0C61
	#define GL_PREVIOUS                       0x8578
	#define GL_ADD                            0x0104
	#define GL_SUBTRACT                       0x84E7
	#define GL_DOT3_RGB                       0x86AE
	#define GL_ADD_SIGNED                     0x8574
	#define GL_CLAMP                          0x2900
	#define GL_CLAMP_TO_BORDER                0x812D
	#define GL_TEXTURE_WRAP_R                 0x8072
	#define GL_R                              0x2002
	#define GL_TEXTURE_GEN_R                  0x0C62
	#define GL_GENERATE_MIPMAP                0x8191
	#define GL_TEXTURE_PRIORITY               0x8066
	#define GL_TEXTURE_BORDER_COLOR           0x1004
	#define GL_TEXTURE_INTERNAL_FORMAT        0x1003
	#define GL_COMPRESSED_RGBA                0x84EE
	#define GL_TEXTURE_COMPRESSION_HINT       0x84EF
	#define GL_PROXY_TEXTURE_2D               0x8064
	#define GL_TEXTURE_WIDTH                  0x1000
	#define GL_TEXTURE_HEIGHT                 0x1001
	#define GL_POSITION                       0x1203
	#define GL_SPOT_DIRECTION                 0x1204
	#define GL_POSITION                       0x1203
	#define GL_CONSTANT_ATTENUATION           0x1207
	#define GL_LINEAR_ATTENUATION             0x1208
	#define GL_QUADRATIC_ATTENUATION          0x1209
	#define GL_SPOT_CUTOFF                    0x1206
	#define GL_SPOT_DIRECTION                 0x1204
	#define GL_POSITION                       0x1203
	#define GL_CONSTANT_ATTENUATION           0x1207
	#define GL_LINEAR_ATTENUATION             0x1208
	#define GL_QUADRATIC_ATTENUATION          0x1209
	#define GL_SPOT_EXPONENT                  0x1205
	#define GL_SPOT_CUTOFF                    0x1206

//#if !defined(_ANDROID)
	#define HAVE_SHADERS
//#endif
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
	#define USE_SHADER(aaa) glUseProgram(aaa)
	#define VERBOSE_USE_SHADER(aaa) {printf ("glUseShader %d\n",aaa); glUseProgram(aaa);}
	#define GET_SHADER_INFO glGetShaderiv
	#define LINK_STATUS GL_LINK_STATUS
	#define COMPILE_STATUS GL_COMPILE_STATUS
	#define GET_UNIFORM(aaa,bbb) glGetUniformLocation(aaa,bbb)
	#define GET_UNIFORM_BLOCK(aaa,bbb) glGetUniformBlockIndex(aaa,bbb)
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
	#define GLUNIFORMMATRIX4FV glUniformMatrix4fv
	#define GLUNIFORMMATRIX3FV glUniformMatrix3fv
#endif
/* Main initialization function */
/* int display_initialize(); */
//extern bool display_initialized;
#define IS_DISPLAY_INITIALIZED (gglobal()->display.display_initialized==TRUE)

/**
 * Sort of "virtual" functions
 *
 * TARGET_AQUA   : 
 * TARGET_X11    : ui/fwBareWindow.c
 * TARGET_MOTIF  : ui/fwMotifWindow.c
 * TARGET_WIN32  : ui/fwWindow32.c
 */

/* are we doing Vertex Buffer Objects? (VBOs) for OpenGL? */
#define VERTEX_VBO 0
#define NORMAL_VBO 1
#define TEXTURE_VBO 2
#define INDEX_VBO 3
#define COLOR_VBO 4
#define VBO_COUNT 5

void fv_setScreenDim(int wi, int he);

int fv_open_display();
int fv_display_initialize(void);
int fv_create_main_window(int argc, char *argv[]);
bool fv_create_GLcontext();
bool fv_bind_GLcontext();
/* end of "virtual" functions */

/* OLDCODE: bool fwl_initialize_GL(); is now in lib header */

/* OpenGL renderer capabilities */


typedef enum shader_type {
	/* Background shaders */
	backgroundSphereShader,
	backgroundTextureBoxShader,

	/* generic (not geometry Shader specific) shaders */
	noMaterialNoAppearanceShader,
	noTexOneMaterialShader,
	noTexTwoMaterialShader,
	oneTexOneMaterialShader,
	oneTexTwoMaterialShader,
	complexTexOneMaterialShader,
	complexTexTwoMaterialShader,

	/* Sphere Geometry Shaders */
	noMaterialNoAppearanceSphereShader,
	noTexOneMaterialSphereShader,
	noTexTwoMaterialSphereShader,
	oneTexOneMaterialSphereShader,
	oneTexTwoMaterialSphereShader,
	complexTexOneMaterialSphereShader,
	complexTexTwoMaterialSphereShader,

	/* Shape has Color node */
	/* noMaterialNoAppearanceColourShader, -same as backgroundSphereShader */
	noTexTwoMaterialColourShader,
	noTexOneMaterialColourShader,
	oneTexTwoMaterialColourShader,
	oneTexOneMaterialColourShader,

	/* final one, used for array sizing */
	max_enum_shader_type
} shader_type_t;

typedef struct {
	GLint compiledOK;
	GLuint myShaderProgram;

	GLint myMaterialAmbient;
	GLint myMaterialDiffuse;
	GLint myMaterialSpecular;
	GLint myMaterialShininess;
	GLint myMaterialEmission;

	GLint myMaterialBackAmbient;
	GLint myMaterialBackDiffuse;
	GLint myMaterialBackSpecular;
	GLint myMaterialBackShininess;
	GLint myMaterialBackEmission;

	GLint lightState;
        GLint lightAmbient;
        GLint lightDiffuse;
        GLint lightSpecular;
        GLint lightPosition;

	GLint ModelViewMatrix;
	GLint ProjectionMatrix;
	GLint NormalMatrix;
	GLint Vertices;
	GLint Normals;
	GLint Colours;
	GLint TexCoords;
	GLint Texture0;

	
	/* some geom shaders have particular uniforms, eg geom radius */
	GLint specialUniform1;
	GLint specialUniform2;
	GLint specialUniform3;
	GLint specialUniform4;
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

	s_shader_capabilities_t backgroundShaderArrays[max_enum_shader_type]; /* one element for each shader_type */
} s_renderer_capabilities_t;

extern s_renderer_capabilities_t rdr_caps;

bool initialize_rdr_caps();
void initialize_rdr_functions();
void rdr_caps_dump();


/**
 * Main window parameters
 */
//extern int win_height; /* window */
//extern int win_width;
//extern long int winToEmbedInto;
//extern int fullscreen;
//extern int view_height; /* viewport */
//extern int view_width;
//
//extern int screenWidth;
//extern int screenHeight;
//
//extern double screenRatio;
//
//extern char *window_title;
//
//extern int mouse_x;
//extern int mouse_y;
//
//extern int show_mouse;
//
//extern int xPos;
//extern int yPos;
//
//extern int displayDepth;
//
//extern int shutterGlasses; /* shutter glasses, stereo enabled ? */
//extern int quadbuff_stereo_mode; /* quad buffer enabled ? */

#ifdef TARGET_AQUA
#ifndef IPHONE

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

//extern GLenum _global_gl_err;

#if defined(_ANDROID)
#define PRINT_GL_ERROR_IF_ANY(_where) { \
                                              GLenum _global_gl_err = glGetError(); \
                                              while (_global_gl_err != GL_NO_ERROR) { \
						if (_global_gl_err == GL_INVALID_ENUM) {DROIDDEBUG ("GL_INVALID_ENUM"); } \
						else if (_global_gl_err == GL_INVALID_VALUE) {DROIDDEBUG("GL_INVALID_VALUE"); } \
						else if (_global_gl_err == GL_INVALID_OPERATION) {DROIDDEBUG("GL_INVALID_OPERATION"); } \
						else if (_global_gl_err == GL_OUT_OF_MEMORY) {DROIDDEBUG("GL_OUT_OF_MEMORY"); } \
						else DROIDDEBUG("unknown error"); \
                                                 DROIDDEBUG(" here: %s (%s:%d)\n", _where,__FILE__,__LINE__); \
                                                 _global_gl_err = glGetError(); \
                                              } \
                                           } 

#else
/* This used to be IPHONE only, but it is best if no code in the library depends on gluErrorString() */
#define PRINT_GL_ERROR_IF_ANY(_where) { \
                                              GLenum _global_gl_err = glGetError(); \
                                              while (_global_gl_err != GL_NO_ERROR) { \
						if (_global_gl_err == GL_INVALID_ENUM) {printf ("GL_INVALID_ENUM"); } \
						else if (_global_gl_err == GL_INVALID_VALUE) {printf ("GL_INVALID_VALUE"); } \
						else if (_global_gl_err == GL_INVALID_OPERATION) {printf ("GL_INVALID_OPERATION"); } \
						else if (_global_gl_err == GL_OUT_OF_MEMORY) {printf ("GL_OUT_OF_MEMORY"); } \
						else printf ("unknown error"); \
                                                 printf(" here: %s (%s:%d)\n", _where,__FILE__,__LINE__); \
                                                 _global_gl_err = glGetError(); \
                                              } \
                                           } 
#endif

#define GL_ERROR_MSG (\
	(glGetError() == GL_NO_ERROR)?"":\
		(glGetError() == GL_INVALID_ENUM)?"GL_INVALID_ENUM":\
		(glGetError() == GL_INVALID_VALUE)?"GL_INVALID_VALUE":\
		(glGetError() == GL_INVALID_OPERATION)?"GL_INVALID_OPERATION":\
		(glGetError() == GL_OUT_OF_MEMORY)?"GL_OUT_OF_MEMORY":\
		"unknown GL_ERROR")

void resetGeometry();
/* void setScreenDim(int wi, int he); */

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
	#define USE_SHADER(aaa) glUseProgram(aaa)
	#define VERBOSE_USE_SHADER(aaa) {printf ("glUseShader %d\n",aaa); glUseProgram(aaa);}
	#define GET_SHADER_INFO glGetShaderiv
	#define LINK_STATUS GL_LINK_STATUS
	#define COMPILE_STATUS GL_COMPILE_STATUS
	#define GET_UNIFORM(aaa,bbb) glGetUniformLocation(aaa,bbb)
	#define GET_UNIFORM_BLOCK(aaa,bbb) glGetUniformBlockIndex(aaa,bbb)
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
	#define GLUNIFORMMATRIX4FV glUniformMatrix4fv
	#define GLUNIFORMMATRIX3FV glUniformMatrix3fv

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
	#define USE_SHADER(aaa) glUseProgramObjectARB(aaa)
	#define CREATE_SHADER glCreateShaderObjectARB
	#define GET_SHADER_INFO glGetObjectParameterivARB
	#define LINK_STATUS GL_OBJECT_LINK_STATUS_ARB
	#define COMPILE_STATUS GL_OBJECT_COMPILE_STATUS_ARB
	#define GET_UNIFORM(aaa,bbb) glGetUniformLocationARB(aaa,bbb)
	#define GET_UNIFORM_BLOCK(aaa,bbb) glGetUniformBlockIndex(aaa,bbb)
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
	#define GLUNIFORMMATRIX4FV glUniformMatrix4fvARB
	#define GLUNIFORMMATRIX3FV glUniformMatrix3fvARB
#endif
#endif

/* OpenGL-2.x and OpenGL-3.x "desktop" systems calls */
	/****************************************************************/
	/* First - any platform specifics to do? 			*/
	/****************************************************************/

	#if defined(_MSC_VER)
		#define FW_GL_SWAPBUFFERS SwapBuffers(wglGetCurrentDC());
	#endif

	#if defined (TARGET_AQUA)
#ifdef OLDCODE
OLDCODE		#if !defined (FRONTEND_HANDLES_DISPLAY_THREAD) 
OLDCODE			#define FW_GL_SWAPBUFFERS { \
OLDCODE				CGLError err = FW_GL_CGLFLUSHDRAWABLE(myglobalContext); \
OLDCODE				if (err != kCGLNoError) printf ("FW_GL_CGLFLUSHDRAWABLE error %d\n",err); }
OLDCODE		#else
OLDCODE			#define FW_GL_SWAPBUFFERS /* do nothing */
OLDCODE		#endif /* FRONTEND_HANDLES_DISPLAY_THREAD */
#else
			#define FW_GL_SWAPBUFFERS /* do nothing */
#endif

	#endif

#if KEEP_X11_INLIB
	#if defined (TARGET_X11) || defined (TARGET_MOTIF)
		#define FW_GL_SWAPBUFFERS glXSwapBuffers(Xdpy,GLwin);
	#endif
#else
	#define FW_GL_SWAPBUFFERS /* nothing */
#endif
	
	#if defined( _ANDROID )
		#define FW_GL_SWAPBUFFERS /* nothing */
	#endif


	/****************************************************************/
	/* Second - things that might be specific to one platform;	*/
	/*	this is the "catch for other OS" here 			*/
	/****************************************************************/
        /* nothing here */

	/****************************************************************/
	/* Third - common across all platforms				*/
	/****************************************************************/


	/* GLU replacement - needs local matrix stacks, plus more code */
	#define FW_GLU_PERSPECTIVE(aaa,bbb,ccc,ddd) fw_gluPerspective(aaa,bbb,ccc,ddd)
	#define FW_GLU_UNPROJECT(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii) fw_gluUnProject(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii)
	#define FW_GLU_PROJECT(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii) fw_gluProject(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii)
	#define FW_GLU_PICK_MATRIX(aaa, bbb, ccc, ddd, eee) fw_gluPickMatrix(aaa, bbb, ccc, ddd, eee)

	/* GLU replacement -these still need doing */
	#define FW_GLU_DELETETESS(aaa) gluDeleteTess(aaa)
	#define FW_GLU_NEW_TESS gluNewTess
	#define FW_GLU_END_POLYGON(aaa) gluEndPolygon(aaa)
	#define FW_GLU_BEGIN_POLYGON(aaa) gluBeginPolygon(aaa)
	#define FW_GLU_TESS_VERTEX(aaa, bbb, ccc) gluTessVertex(aaa, bbb, ccc)
	#define FW_GLU_TESS_CALLBACK(aaa, bbb, ccc) gluTessCallback(aaa,bbb,ccc);
	#define FW_GLU_NEXT_CONTOUR(aaa, bbb) gluNextContour(aaa,bbb)
	#define FW_GLU_SCALE_IMAGE(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii) gluScaleImage(aaa, bbb, ccc, ddd, eee, fff, ggg, hhh, iii)


	#define FW_GL_GETDOUBLEV(aaa,bbb) fw_glGetDoublev(aaa,bbb);
	#define FW_GL_LOAD_IDENTITY fw_glLoadIdentity
	#define FW_GL_POP_MATRIX() fw_glPopMatrix()
	#define FW_GL_PUSH_MATRIX() fw_glPushMatrix()

	#define FW_GL_TRANSLATE_F(xxx,yyy,zzz) fw_glTranslatef(xxx,yyy,zzz)
	#define FW_GL_TRANSLATE_D(xxx,yyy,zzz) fw_glTranslated(xxx,yyy,zzz)
	#define FW_GL_ROTATE_F(aaa,xxx,yyy,zzz) fw_glRotatef(aaa,xxx,yyy,zzz)
	#define FW_GL_ROTATE_D(aaa,xxx,yyy,zzz) fw_glRotated(aaa,xxx,yyy,zzz)
	#define FW_GL_ROTATE_RADIANS(aaa,xxx,yyy,zzz) fw_glRotateRad(aaa,xxx,yyy,zzz)
	#define FW_GL_SCALE_F(xxx,yyy,zzz) fw_glScalef(xxx,yyy,zzz)
	#define FW_GL_SCALE_D(xxx,yyy,zzz) fw_glScaled(xxx,yyy,zzz)
        #define FW_GL_PUSH_ATTRIB(aaa) glPushAttrib(aaa); 
	#define FW_GL_POP_ATTRIB() glPopAttrib();
	#define FW_GL_MATRIX_MODE(aaa) fw_glMatrixMode(aaa)
	#define FW_GL_ORTHO(aaa,bbb,ccc,ddd,eee,fff) fw_Ortho(aaa,bbb,ccc,ddd,eee,fff);


	/* geometry rendering - varies on whether we are using appearance shaders, etc */
	#define FW_VERTEX_POINTER_TYPE 44354
	#define FW_NORMAL_POINTER_TYPE 5434
	#define FW_COLOR_POINTER_TYPE 12453
	#define FW_TEXCOORD_POINTER_TYPE 67655
	#define FW_GL_VERTEX_POINTER(aaa, bbb, ccc, ddd) {sendAttribToGPU(FW_VERTEX_POINTER_TYPE, aaa, bbb, GL_FALSE, ccc, ddd,__FILE__,__LINE__); }
	/* color buffer subject to draw-gray anaglyph before call */
	#define FW_GL_COLOR_POINTER(aaa, bbb, ccc, ddd) {sendAttribToGPU(FW_COLOR_POINTER_TYPE, aaa, bbb, GL_FALSE, ccc, ddd,__FILE__,__LINE__); }
	#define FW_GL_NORMAL_POINTER(aaa, bbb, ccc) {sendAttribToGPU(FW_NORMAL_POINTER_TYPE, 0, aaa, GL_FALSE, bbb, ccc,__FILE__,__LINE__); }
	#define FW_GL_TEXCOORD_POINTER(aaa, bbb, ccc, ddd) {sendAttribToGPU(FW_TEXCOORD_POINTER_TYPE, aaa, bbb, GL_FALSE, ccc, ddd,__FILE__,__LINE__); }
	#define FW_GL_ENABLECLIENTSTATE(aaa) { sendClientStateToGPU(TRUE,aaa); }
	#define FW_GL_DISABLECLIENTSTATE(aaa) { sendClientStateToGPU(FALSE,aaa); }
	#define FW_GL_DRAWARRAYS(xxx,yyy,zzz) { sendArraysToGPU(xxx,yyy,zzz); }
	#define FW_GL_BINDBUFFER(xxx,yyy) {sendBindBufferToGPU(xxx,yyy,__FILE__,__LINE__); }
	#define FW_GL_VERTEXATTRBPOINTER(a1,a2,a3,a4,a5,a6) {sendVertexAttribsToGPU(a1,a2,a3,a4,a5,a6);}
	#define FW_GL_DRAWELEMENTS(aaa,bbb,ccc,ddd) {sendElementsToGPU(aaa,bbb,ccc,ddd); }



	#define FW_GL_VIEWPORT(aaa,bbb,ccc,ddd) glViewport(aaa,bbb,ccc,ddd);
	#define FW_GL_CLEAR_COLOR(aaa,bbb,ccc,ddd) glClearColor(aaa,bbb,ccc,ddd);
	#define FW_GL_DEPTHMASK(aaa) glDepthMask(aaa);
	#define FW_GL_ENABLE(aaa) glEnable(aaa)
	#define FW_GL_DISABLE(aaa) glDisable(aaa) 
	#define FW_GL_ALPHAFUNC(aaa,bbb) glAlphaFunc(aaa,bbb); 
        #define FW_GL_SCISSOR(aaa,bbb,ccc,ddd) glScissor(aaa,bbb,ccc,ddd); 
	#define FW_GL_WINDOWPOS2I(aaa,bbb) glWindowPos2i(aaa,bbb);
	#define FW_GL_FLUSH glFlush
	#define FW_GL_RASTERPOS2I(aaa,bbb) glRasterPos2i(aaa,bbb); 
	#define FW_GL_PIXELZOOM(aaa,bbb) glPixelZoom(aaa,bbb);
        #define FW_GL_LIGHTMODELI(aaa,bbb) glLightModeli(aaa,bbb); 
        #define FW_GL_LIGHTMODELFV(aaa,bbb) glLightModelfv(aaa,bbb); 
	#define FW_GL_CLEAR_DEPTH(aaa) glClearDepth(aaa); 
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
	#define FW_GL_TEXENVI(aaa,bbb,ccc) glTexEnvi(aaa,bbb,ccc)
	#define FW_GL_TEXGENI(aaa,bbb,ccc) glTexGeni(aaa,bbb,ccc)
	#define FW_GL_BINDTEXTURE(aaa,bbb) glBindTexture(aaa,bbb)


	#define FW_GL_FOGFV(aaa, bbb) glFogfv(aaa, bbb)
	#define FW_GL_FOGF(aaa, bbb) glFogf(aaa, bbb)
	#define FW_GL_FOGI(aaa, bbb) glFogi(aaa, bbb)
	#define FW_GL_BEGIN_QUERY(aaa, bbb) glBeginQuery(aaa, bbb)
	#define FW_GL_END_QUERY(aaa) glEndQuery(aaa)
	#define FW_GL_LINE_STIPPLE(aaa, bbb) glLineStipple(aaa, bbb)
	#define FW_GL_VERTEX3D(aaa, bbb, ccc) glVertex3d(aaa, bbb, ccc)


	#define FW_GL_GET_TEX_LEVEL_PARAMETER_IV(aaa, bbb, ccc, ddd) glGetTexLevelParameteriv(aaa, bbb, ccc, ddd)
#ifdef IPHONE
	/* ES 2.0 - set the sampler */
	#define SET_TEXTURE_UNIT(aaa) { glActiveTexture(GL_TEXTURE0+aaa); glUniform1i(getAppearanceProperties()->currentShaderProperties->Texture0, aaa); }
#else
	#define SET_TEXTURE_UNIT(aaa) { glActiveTexture(GL_TEXTURE0+aaa); glClientActiveTexture(GL_TEXTURE0+aaa); }
#endif
	
	#define FW_GL_VERTEX3F(aaa, bbb, ccc) glVertex3f(aaa, bbb, ccc)
	#define FW_GL_GETSTRING(aaa) glGetString(aaa)
	#define FW_GL_DELETETEXTURES(aaa,bbb) glDeleteTextures(aaa,bbb);
	#define FW_GL_LOADMATRIXD(aaa) fw_glLoadMatrixd(aaa)
	#define FW_GL_GETINTEGERV(aaa,bbb) glGetIntegerv(aaa,bbb);
	#define FW_GL_GETFLOATV(aaa,bbb) glGetFloatv(aaa,bbb);

	#define FW_GL_MATERIALF(aaa, bbb, ccc) glMaterialf(aaa, bbb, ccc)
	#define FW_GL_COLOR_MATERIAL(aaa, bbb) glColorMaterial(aaa, bbb)
int usingAnaglyph2();
/* color functions subject to draw-gray anaglyph >>  */
void fwAnaglyphRemapf(float *r2, float *g2, float* b2, float r, float g, float b);
void fwAnaglyphremapRgbav(unsigned char *rgba,int y,int x);
void fwglMaterialfv(GLenum face, GLenum pname, const GLfloat *params);
void fwglColor3fv(float *rgb);
void fwglColor4f(float r,float g, float b, float a);
void fwglColor4fv(float *rgba);
void fwglColor3d(double r, double g, double b);
void fwglColor3f(float r, float g, float b);
	#define FW_GL_MATERIALFV(aaa, bbb, ccc) fwglMaterialfv(aaa, bbb, ccc)

	#define FW_GL_COLOR3F(aaa,bbb,ccc) fwglColor3f(aaa,bbb,ccc);
	#define FW_GL_COLOR4FV(aaa) fwglColor4fv(aaa);
	#define FW_GL_COLOR3D(aaa, bbb, ccc) fwglColor3d(aaa, bbb, ccc)
	#define FW_GL_COLOR3FV(aaa) fwglColor3fv(aaa);
	#define FW_GL_COLOR4F(aaa,bbb,ccc,ddd) fwglColor4f(aaa,bbb,ccc,ddd);
	#define FW_GL_COLOR4FV(aaa) fwglColor4fv(aaa);

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
	
	/*apr 6 2010 checkout win32 was missing the following macros */
	#define FW_GL_ACCUM(aaa,bbb) glAccum(aaa,bbb)
	#define FW_GL_DRAWBUFFER(aaa) glDrawBuffer(aaa)
	#define FW_GL_ENDLIST() glEndList()
	#define FW_GL_BITMAP(aaa,bbb,ccc,ddd,eee,fff,ggg) glBitmap(aaa,bbb,ccc,ddd,eee,fff,ggg)
	#define FW_GL_CALLLISTS(aaa,bbb,ccc) glCallLists(aaa,bbb,ccc)
	#define FW_GL_LISTBASE(aaa) glListBase(aaa)
	#define FW_GL_DRAWPIXELS(aaa,bbb,ccc,ddd,eee) glDrawPixels(aaa,bbb,ccc,ddd,eee)
	
#endif /* __LIBFREEWRL_DISPLAY_H__ */
