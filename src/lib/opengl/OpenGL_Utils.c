
/*
  $Id: OpenGL_Utils.c,v 1.199 2011/06/03 20:06:52 dug9 Exp $

  FreeWRL support library.
  OpenGL initialization and functions. Rendering functions.
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
#include <system_threads.h>

#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>
#include <list.h>
#include <io_files.h>
#include <resources.h>


#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../main/ProdCon.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CParse.h"
#include "../vrml_parser/CRoutes.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"
#include "../scenegraph/sounds.h"
#include "../scenegraph/LinearAlgebra.h"
#include "../scenegraph/Component_KeyDevice.h"	/* resolving implicit declarations */
#include "../input/EAIHeaders.h"		/* resolving implicit declarations */
#include "../input/InputFunctions.h"
#include "Frustum.h"
#include "../opengl/Material.h"
#include "../scenegraph/Component_Core.h"
#include "../scenegraph/Component_Networking.h"
#include "Textures.h"
#include "OpenGL_Utils.h"
#include "../scenegraph/RenderFuncs.h"
#include "../scenegraph/Component_Shape.h"

#include <float.h>

#include "../x3d_parser/Bindable.h"

#define USE_JS_EXPERIMENTAL_CODE 0
void kill_rendering(void);

/* Node Tracking */
#if USE_JS_EXPERIMENTAL_CODE
static void kill_X3DNodes(void);
#endif

static void createdMemoryTable();
static void increaseMemoryTable();
//static struct X3D_Node ** memoryTable = NULL;
//static int tableIndexSize = INT_ID_UNDEFINED;
//static int nextEntry = 0;
//static struct X3D_Node *forgottenNode;

#if USE_JS_EXPERIMENTAL_CODE
static void killNode (int index);
#endif

static void mesa_Frustum(GLDOUBLE left, GLDOUBLE right, GLDOUBLE bottom, GLDOUBLE top, GLDOUBLE nearZ, GLDOUBLE farZ, GLDOUBLE *m);

#undef DEBUG_FW_LOADMAT
#ifdef DEBUG_FW_LOADMAT
static void fw_glLoadMatrixd(GLDOUBLE *val,char *, int);
#define FW_GL_LOADMATRIX(aaa) fw_glLoadMatrixd(aaa,__FILE__,__LINE__);
#else
static void fw_glLoadMatrixd(GLDOUBLE *val);
#define FW_GL_LOADMATRIX(aaa) fw_glLoadMatrixd(aaa);
#endif

static void mesa_Ortho(GLDOUBLE left, GLDOUBLE right, GLDOUBLE bottom, GLDOUBLE top, GLDOUBLE nearZ, GLDOUBLE farZ, GLDOUBLE *m);
static void getShaderCommonInterfaces (s_shader_capabilities_t *me);

///* is this 24 bit depth? 16? 8?? Assume 24, unless set on opening */
//int displayDepth = 24;
//
////static float cc_red = 0.0f, cc_green = 0.0f, cc_blue = 0.0f, cc_alpha = 1.0f;
//int cc_changed = FALSE;

//static pthread_mutex_t  memtablelock = PTHREAD_MUTEX_INITIALIZER;
#define LOCK_MEMORYTABLE 		pthread_mutex_lock(&p->memtablelock);
#define UNLOCK_MEMORYTABLE		pthread_mutex_unlock(&p->memtablelock);
/*
#define LOCK_MEMORYTABLE 		{printf ("LOCK_MEMORYTABLE at %s:%d\n",__FILE__,__LINE__); pthread_mutex_lock(&memtablelock);}
#define UNLOCK_MEMORYTABLE		{printf ("UNLOCK_MEMORYTABLE at %s:%d\n",__FILE__,__LINE__); pthread_mutex_unlock(&memtablelock);}
*/

/* OpenGL perform matrix state here */
#define MAX_LARGE_MATRIX_STACK 32	/* depth of stacks */
#define MAX_SMALL_MATRIX_STACK 2	/* depth of stacks */
#define MATRIX_SIZE 16		/* 4 x 4 matrix */
typedef GLDOUBLE MATRIX4[MATRIX_SIZE];



typedef struct pOpenGL_Utils{
	struct X3D_Node ** memoryTable;// = NULL;
	int tableIndexSize;// = INT_ID_UNDEFINED;
	int nextEntry;// = 0;
	struct X3D_Node *forgottenNode;
	float cc_red, cc_green, cc_blue, cc_alpha;
	pthread_mutex_t  memtablelock;// = PTHREAD_MUTEX_INITIALIZER;
	MATRIX4 FW_ModelView[MAX_LARGE_MATRIX_STACK];
	MATRIX4 FW_ProjectionView[MAX_SMALL_MATRIX_STACK];
	MATRIX4 FW_TextureView[MAX_SMALL_MATRIX_STACK];
	 
	int modelviewTOS;// = 0;
	int projectionviewTOS;// = 0;
	int textureviewTOS;// = 0;

	int whichMode;// = GL_MODELVIEW;
	GLDOUBLE *currentMatrix;// = FW_ModelView[0];

}* ppOpenGL_Utils;
void *OpenGL_Utils_constructor(){
	void *v = malloc(sizeof(struct pOpenGL_Utils));
	memset(v,0,sizeof(struct pOpenGL_Utils));
	return v;
}
void OpenGL_Utils_init(struct tOpenGL_Utils *t)
{
	//public
	/* is this 24 bit depth? 16? 8?? Assume 24, unless set on opening */
	t->displayDepth = 24;

	//static float cc_red = 0.0f, cc_green = 0.0f, cc_blue = 0.0f, cc_alpha = 1.0f;
	t->cc_changed = FALSE;

	//private
	t->prv = OpenGL_Utils_constructor();
	{
		ppOpenGL_Utils p = (ppOpenGL_Utils)t->prv;
		p->memoryTable = NULL;
		p->tableIndexSize = INT_ID_UNDEFINED;
		p->nextEntry = 0;
		//p->forgottenNode;
		p->cc_red = 0.0f;
		p->cc_green = 0.0f;
		p->cc_blue = 0.0f;
		p->cc_alpha = 1.0f;
		//p->memtablelock = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_init(&(p->memtablelock), NULL);
		p->FW_ModelView[MAX_LARGE_MATRIX_STACK];
		p->FW_ProjectionView[MAX_SMALL_MATRIX_STACK];
		p->FW_TextureView[MAX_SMALL_MATRIX_STACK];
		 
		p->modelviewTOS = 0;
		p->projectionviewTOS = 0;
		p->textureviewTOS = 0;

		p->whichMode = GL_MODELVIEW;
		p->currentMatrix = p->FW_ModelView[0];

	}
}


#define TURN_OFF_SHOULDSORTCHILDREN node->_renderFlags = node->_renderFlags & (0xFFFF^ VF_shouldSortChildren);
/******************************************************************/
/* textureTransforms of all kinds */

/* change the clear colour, selected from the GUI, but do the command in the
   OpenGL thread */

void fwl_set_glClearColor (float red , float green , float blue , float alpha) {
	ppOpenGL_Utils p;
	ttglobal tg = gglobal();
	p = (ppOpenGL_Utils)tg->OpenGL_Utils.prv;
	p->cc_red = red; p->cc_green = green ; p->cc_blue = blue ; p->cc_alpha = alpha ;
	tg->OpenGL_Utils.cc_changed = TRUE;
}

void setglClearColor (float *val) {
	ppOpenGL_Utils p;
	ttglobal tg = gglobal();
	p = (ppOpenGL_Utils)tg->OpenGL_Utils.prv;
	p->cc_red = *val; val++;
	p->cc_green = *val; val++;
	p->cc_blue = *val;
#ifdef AQUA
	val++;
	p->cc_alpha = *val;
#endif
	tg->OpenGL_Utils.cc_changed = TRUE;
}        


/**************************************************************************************

		Determine near far clip planes.

We have 2 choices; normal geometry, or we have a Geospatial sphere.

If we have normal geometry (normal Viewpoint, or GeoViewpoint with GC coordinates)
then, we take our AABB (axis alligned bounding box), rotate the 8 Vertices, and
find the min/max Z distance, and just use this. 

This works very well for examine objects, or when we are within a virtual world.
----

If we are Geospatializing around the earth, so we have GeoSpatial and have UTM or GD
coordinates, lets do some optimizations here.

First optimization, we know our height above the origin, and we most certainly are not
going to see things past the origin, so we assume far plane is height above the origin.

Second, we know our AABB contains the Geospatial sphere, and it ALSO contains the highest
mountain peak, so we just go and find the value representing the highest peak. Our
near plane is thus farPlane - highestPeak. 

**************************************************************************************/

#if USE_JS_EXPERIMENTAL_CODE
/* read a file, put it into memory. */
static char * readInputString(char *fn) {
        char *buffer;
        FILE *infile;
        size_t justread;

	#define MAXREADSIZE 4000

        /* ok, now, really read this one. */
        infile = fopen(fn,"r");

        if (infile == NULL){
                ConsoleMessage("problem reading file '%s' \n",fn);
                return NULL;
        }


        buffer =MALLOC(char *, MAXREADSIZE * sizeof (char));
        justread = fread (buffer,1,MAXREADSIZE,infile);
	if (justread >= MAXREADSIZE) {
		ConsoleMessage ("Shader too large for buffer\n");
		return NULL;
	}

	fclose (infile);

	buffer[justread] = '\0';
        return (buffer);
}
#endif
#undef MAXREADSIZE


#ifdef SHADERS_2011
static void shaderErrorLog(GLuint myShader, char *which) {
        #if defined  (GL_VERSION_2_0) || defined (GL_ES_VERSION_2_0)
#define MAX_INFO_LOG_SIZE 512
                GLchar infoLog[MAX_INFO_LOG_SIZE];
                glGetShaderInfoLog(myShader, MAX_INFO_LOG_SIZE, NULL, infoLog);
                ConsoleMessage ("problem with %s shader: %s",which, infoLog);
        #else
                ConsoleMessage ("Problem compiling shader");
        #endif
}

static char *backgroundSphereShaderFragment = 
" varying vec4 v_color; void main () {gl_FragColor = v_color;}";

static char *backgroundSphereShaderVertex = "attribute	vec4 fw_Color; " \
			"attribute	vec4 fw_Vertex;" \
			"uniform		mat4 fw_ModelViewMatrix;" \
			"uniform		mat4 fw_ProjectionMatrix;" \
			"varying	vec4 v_color;" \
			"void main(void) {" \
			"	gl_Position = fw_ProjectionMatrix * fw_ModelViewMatrix * fw_Vertex;" \
			"	v_color = fw_Color; " \
			"	/* v_color = vec4(1., 0., 0., 1.); v_color.a = 1.; */" \
			"}";

static char *backgroundTextureBoxShaderFragment =
       				" varying vec2 v_texC; uniform sampler2D fw_Texture0; " \
				" void main () {gl_FragColor = texture2D(fw_Texture0, v_texC);}";
static char *backgroundTextureBoxShaderVertex =
			"attribute	vec2 fw_TexCoords; " \
			"attribute	vec4 fw_Vertex;" \
			"uniform	mat4 fw_ModelViewMatrix;" \
			"uniform	mat4 fw_ProjectionMatrix;" \
			"varying	vec2 v_texC;" \
			"void main(void) {" \
			"	vec4 pos = fw_ModelViewMatrix * fw_Vertex;" \
			"	gl_Position = fw_ProjectionMatrix * fw_ModelViewMatrix * fw_Vertex;" \
			"	v_texC =fw_TexCoords;" \
			"}";

static char *oneTexFragmentShader = " \
	varying vec2 v_texC; \
	uniform sampler2D fw_Texture0; \
	void main () { \
		gl_FragColor = texture2D(fw_Texture0, v_texC); \
	}";


/* Vertex Shaders */

static char *noTexVertexShaderForSphereGeomShader = " \
	attribute      vec4 fw_Vertex; \
	uniform        mat4 fw_ModelViewMatrix; \
	uniform        mat4 fw_ProjectionMatrix; \
	void main(void) { \
		/* just pass these through - the geom shader will calculate */ \
		/* and transform as it creates new vertices */ \
		gl_Position = fw_Vertex; \
	}";

static char *oneTexVertexShaderForSphereGeomShader = " \
	attribute      vec4 fw_Vertex; \
	uniform        mat4 fw_ModelViewMatrix; \
	uniform        mat4 fw_ProjectionMatrix; \
        \
        attribute vec2 fw_TexCoords; \
        varying        vec2 v_texCIn; /* goes to v_texCin[3] */ \
        \
	void main(void) { \
		/* just pass these through - the geom shader will calculate */ \
		/* and transform as it creates new vertices */ \
		gl_Position = fw_Vertex; \
               v_texCIn =fw_TexCoords; \
	}";

/* vertex shader - phong lighting */
static char *phongSimpleVertexShader = " \
	varying vec3 Norm; \
	varying vec4 Pos; \
	attribute      vec4 fw_Vertex; \
	attribute      vec3 fw_Normal; \
	uniform        mat4 fw_ModelViewMatrix; \
	uniform        mat4 fw_ProjectionMatrix; \
	uniform        mat3 fw_NormalMatrix; \
	void main(void) { \
	       Norm = normalize(fw_NormalMatrix * fw_Normal); \
	       Pos = fw_ModelViewMatrix * fw_Vertex;  \
	       gl_Position = fw_ProjectionMatrix * fw_ModelViewMatrix * fw_Vertex; \
	}";





static char *oneTexVertexShader = " \
        /* varying vec3 Norm; */ \
        /* varying vec4 Pos; */ \
        attribute      vec4 fw_Vertex; \
        attribute      vec3 fw_Normal; \
        uniform        mat4 fw_ModelViewMatrix; \
        uniform        mat4 fw_ProjectionMatrix; \
        uniform        mat3 fw_NormalMatrix; \
        \
        attribute vec2 fw_TexCoords; \
        varying        vec2 v_texC; \
        \
        void main(void) { \
               /* Norm = normalize(fw_NormalMatrix * fw_Normal); */ \
               /* Pos = fw_ModelViewMatrix * fw_Vertex;  */ \
               gl_Position = fw_ProjectionMatrix * fw_ModelViewMatrix * fw_Vertex; \
               v_texC =fw_TexCoords; \
        }";



/* Geometry Shaders */

static const char *noTexSphereGeomShader = " \
	\
	int numLayers;\
	\
	varying float LightIntensity;\
	varying vec3 Norm; \
	varying vec4 Pos; \
	uniform		mat4 fw_ModelViewMatrix; \
	uniform		mat4 fw_ProjectionMatrix; \
	uniform		mat3 fw_NormalMatrix; \
	uniform		float sphereRadius; \
	\
	vec3 V0, V01, V02;\
	\
	void ProduceVertex( float s, float t ) {\
		const vec3 lightPos = vec3( 0., 10., 0. );\
		\
		vec3 v = V0 + s*V01 + t*V02;\
		v = normalize(v);\
		vec3 n = v;\
		Norm = normalize( fw_NormalMatrix * n ); \
		\
		vec4 ECposition = fw_ModelViewMatrix * vec4( (sphereRadius*v), 1. );\
		\
		gl_Position = fw_ProjectionMatrix * ECposition;\
		Pos = ECposition; \
		EmitVertex();\
	}\
	 \
	void main() { \
		V01 = ( gl_PositionIn[1] - gl_PositionIn[0] ).xyz;\
		V02 = ( gl_PositionIn[2] - gl_PositionIn[0] ).xyz;\
		V0  =   gl_PositionIn[0].xyz;\
		\
		/* trying random scaling to perform LOD */ \
		vec4 distFromViewer = fw_ProjectionMatrix * fw_ModelViewMatrix * vec4 (0., 0., 0., 1.); \
		float dist = distFromViewer.z * sphereRadius; \
		if (dist < 20) { numLayers=6; } else { \
			if (dist < 70) {numLayers=4;} else { \
				if (dist < 120) {numLayers=2;} else {numLayers=1;}; \
			} \
		} \
		float dt = 1. / float( numLayers );\
		\
		float t_top = 1.;\
		\
		for( int it = 0; it < numLayers; it++ ) {\
			float t_bot = t_top - dt;\
			float smax_top = 1. - t_top;\
			float smax_bot = 1. - t_bot;\
			\
			int nums = it + 1;\
			float ds_top = smax_top / float( nums - 1 );\
			float ds_bot = smax_bot / float( nums );\
			\
			float s_top = 0.;\
			float s_bot = 0.;\
			\
			for( int is = 0; is < nums; is++ ) {\
				ProduceVertex( s_bot, t_bot );\
				ProduceVertex( s_top, t_top );\
				s_top += ds_top;\
				s_bot += ds_bot;\
			}\
			\
			ProduceVertex( s_bot, t_bot );\
			EndPrimitive();\
			\
			t_top = t_bot;\
			t_bot -= dt;\
		} \
	 }";

static const char *noAppearanceSphereGeomShader = " \
	\
	int numLayers;\
	\
	varying float LightIntensity;\
	/* varying vec3 Norm; */ \
	/* varying vec4 Pos; */ \
	uniform		mat4 fw_ModelViewMatrix; \
	uniform		mat4 fw_ProjectionMatrix; \
	uniform		mat3 fw_NormalMatrix; \
	uniform		float sphereRadius; \
	\
	vec3 V0, V01, V02;\
	\
	void ProduceVertex( float s, float t ) {\
		const vec3 lightPos = vec3( 0., 10., 0. );\
		\
		vec3 v = V0 + s*V01 + t*V02;\
		v = normalize(v);\
		vec3 n = v;\
		/* Norm = normalize( fw_NormalMatrix * n ); */ \
		\
		vec4 ECposition = fw_ModelViewMatrix * vec4( (sphereRadius*v), 1. );\
		\
		gl_Position = fw_ProjectionMatrix * ECposition;\
		/* Pos = ECposition; */ \
		EmitVertex();\
	}\
	 \
	void main() { \
		V01 = ( gl_PositionIn[1] - gl_PositionIn[0] ).xyz;\
		V02 = ( gl_PositionIn[2] - gl_PositionIn[0] ).xyz;\
		V0  =   gl_PositionIn[0].xyz;\
		\
		/* trying random scaling to perform LOD */ \
		vec4 distFromViewer = fw_ProjectionMatrix * fw_ModelViewMatrix * vec4 (0., 0., 0., 1.); \
		float dist = distFromViewer.z * sphereRadius; \
		if (dist < 20) { numLayers=6; } else { \
			if (dist < 70) {numLayers=4;} else { \
				if (dist < 120) {numLayers=2;} else {numLayers=1;}; \
			} \
		} \
		float dt = 1. / float( numLayers );\
		\
		float t_top = 1.;\
		\
		for( int it = 0; it < numLayers; it++ ) {\
			float t_bot = t_top - dt;\
			float smax_top = 1. - t_top;\
			float smax_bot = 1. - t_bot;\
			\
			int nums = it + 1;\
			float ds_top = smax_top / float( nums - 1 );\
			float ds_bot = smax_bot / float( nums );\
			\
			float s_top = 0.;\
			float s_bot = 0.;\
			\
			for( int is = 0; is < nums; is++ ) {\
				ProduceVertex( s_bot, t_bot );\
				ProduceVertex( s_top, t_top );\
				s_top += ds_top;\
				s_bot += ds_bot;\
			}\
			\
			ProduceVertex( s_bot, t_bot );\
			EndPrimitive();\
			\
			t_top = t_bot;\
			t_bot -= dt;\
		} \
	 }";

/* simple texture shader for Sphere Geometry Shader */
static const char *oneTexSphereGeomShader = " \
	\
	int numLayers;\
	\
	varying float LightIntensity;\
	/* varying vec3 Norm; */ \
	/* varying vec4 Pos; */ \
	uniform		mat4 fw_ModelViewMatrix; \
	uniform		mat4 fw_ProjectionMatrix; \
	uniform		mat3 fw_NormalMatrix; \
	uniform		float sphereRadius; \
	varying in vec2 v_texCIn[3]; /* texture coordinates in */ \
	varying out vec2 v_texC;  /* texture coordinates out */ \
	\
	vec3 V0, V01, V02;\
	vec2 T0, T01, T02; \
	\
	void ProduceVertex( float s, float t ) {\
		const vec3 lightPos = vec3( 0., 10., 0. );\
		\
		vec3 v = V0 + s*V01 + t*V02;\
		v = normalize(v);\
		vec3 n = v;\
		/* Norm = normalize( fw_NormalMatrix * n ); */	\
		\
		vec4 ECposition = fw_ModelViewMatrix * vec4( (sphereRadius*v), 1. );\
		\
		gl_Position = fw_ProjectionMatrix * ECposition;\
		/* Pos = ECposition; */ \
		v_texC = T0; \
		EmitVertex();\
	}\
	 \
	void main() { \
		V01 = ( gl_PositionIn[1] - gl_PositionIn[0] ).xyz;\
		V02 = ( gl_PositionIn[2] - gl_PositionIn[0] ).xyz;\
		V0  =   gl_PositionIn[0].xyz;\
		T0 = v_texCIn[0]; \
		T01 = v_texCIn[1]; \
		T02 = v_texCIn[2];  \
		\
		/* trying random scaling to perform LOD */ \
		vec4 distFromViewer = fw_ModelViewMatrix * vec4 (0., 0., 0., 1.); \
		distFromViewer = fw_ProjectionMatrix * distFromViewer; \
		float dist = distFromViewer.z * sphereRadius; \
		if (dist < 20) { numLayers=6; } else { \
			if (dist < 70) {numLayers=4;} else { \
				if (dist < 120) {numLayers=2;} else {numLayers=1;}; \
			} \
		} \
		float dt = 1. / float( numLayers );\
		\
		float t_top = 1.;\
		\
		for( int it = 0; it < numLayers; it++ ) {\
			float t_bot = t_top - dt;\
			float smax_top = 1. - t_top;\
			float smax_bot = 1. - t_bot;\
			\
			int nums = it + 1;\
			float ds_top = smax_top / float( nums - 1 );\
			float ds_bot = smax_bot / float( nums );\
			\
			float s_top = 0.;\
			float s_bot = 0.;\
			\
			for( int is = 0; is < nums; is++ ) {\
				ProduceVertex( s_bot, t_bot );\
				ProduceVertex( s_top, t_top );\
				s_top += ds_top;\
				s_bot += ds_bot;\
			}\
			\
			ProduceVertex( s_bot, t_bot );\
			EndPrimitive();\
			\
			t_top = t_bot;\
			t_bot -= dt;\
		} \
	 }";
/* Fragment Shaders */



static const char *phongFragmentShader =  " \
\
struct fw_MaterialParameters { \
                vec4 emission; \
                vec4 ambient; \
                vec4 diffuse; \
                vec4 specular; \
                float shininess; \
        }; \
 \
uniform fw_MaterialParameters fw_FrontMaterial; \
uniform fw_MaterialParameters fw_BackMaterial; \
\
varying vec3 Norm;  \
varying vec4 Pos;  \
\
uniform int lightState[8]; \
uniform float light_linAtten[8]; \
uniform float light_constAtten[8]; \
uniform float light_quadAtten[8]; \
uniform float lightSpotCut[8]; \
uniform float lightSpotExp[8]; \
uniform vec4 lightAmbient[8]; \
uniform vec4 lightDiffuse[8]; \
uniform vec4 lightPosition[8]; \
uniform vec4 lightSpecular[8]; \
\
/* use ADSLightModel here \
 the ADS colour is returned from the function. \
*/  \
vec3 ADSLightModel(in vec3 myNormal, in vec4 myPosition) { \
	int i; \
	vec4 diffuse = vec4(0., 0., 0., 0.); \
	vec4 ambient = vec4(0., 0., 0., 0.); \
	vec4 specular = vec4(0., 0., 0., 1.); \
 \
	vec3 norm = normalize (myNormal); \
	vec3 viewv = -normalize(myPosition.xyz); \
	vec4 emissive = fw_FrontMaterial.emission; \
\
	/* apply the lights to this material */ \
	for (i=0; i<8; i++) { \
		if (lightState[i] == 1) { \
		vec4 myLightDiffuse = lightDiffuse[i]; \
		vec4 myLightAmbient = lightAmbient[i]; \
		vec4 myLightSpecular = lightSpecular[i]; \
		vec4 myLightPosition = lightPosition[i]; \
  \
		/* normal, light, view, and light reflection vectors */ \
		vec3 lightv = normalize(myLightPosition.xyz-myPosition.xyz); \
		vec3 refl = reflect (-lightv, norm); \
 \
		/* diffuse light computation */ \
		diffuse += max (0.0, dot(lightv, norm))*fw_FrontMaterial.diffuse*myLightDiffuse; \
 \
		/* ambient light computation */ \
		ambient += fw_FrontMaterial.ambient*myLightAmbient; \
 \
		/* Specular light computation */ \
		if (dot(lightv, viewv) > 0.0) { \
			specular += pow(max(0.0, dot(viewv, refl)), \
				fw_FrontMaterial.shininess)*fw_FrontMaterial.specular*myLightSpecular; \
		} \
		} \
	} \
	return clamp(vec3(ambient+diffuse+specular+emissive), 0.0, 1.0); \
} \
void main () { \
	gl_FragColor = vec4(ADSLightModel(Norm,Pos),1.); \
} \
 " ;




static const char *phongTwoSidedFragmentShader =  " \
\
struct fw_MaterialParameters { \
                vec4 emission; \
                vec4 ambient; \
                vec4 diffuse; \
                vec4 specular; \
                float shininess; \
        }; \
 \
uniform fw_MaterialParameters fw_FrontMaterial; \
uniform fw_MaterialParameters fw_BackMaterial; \
\
varying vec3 Norm;  \
varying vec4 Pos;  \
\
uniform int lightState[8]; \
uniform float light_linAtten[8]; \
uniform float light_constAtten[8]; \
uniform float light_quadAtten[8]; \
uniform float lightSpotCut[8]; \
uniform float lightSpotExp[8]; \
uniform vec4 lightAmbient[8]; \
uniform vec4 lightDiffuse[8]; \
uniform vec4 lightPosition[8]; \
uniform vec4 lightSpecular[8]; \
\
/* use ADSLightModel here \
 the ADS colour is returned from the function. \
*/  \
vec3 ADSLightModel(in vec3 myNormal, in vec4 myPosition) { \
	int i; \
	vec4 diffuse = vec4(0., 0., 0., 0.); \
	vec4 ambient = vec4(0., 0., 0., 0.); \
	vec4 specular = vec4(0., 0., 0., 1.); \
	vec3 viewv = -normalize(myPosition.xyz); \
	vec3 norm = normalize (myNormal); \
	vec4 emissive; \
\
	bool backFacing = (dot(norm,viewv) < 0.0); \
 \
	/* back Facing materials - flip the normal */ \
	if (backFacing) { \
		norm = -norm; \
		emissive = fw_BackMaterial.emission;	\
	} else { \
		emissive = fw_FrontMaterial.emission;	\
	} \
\
	/* apply the lights to this material */ \
	for (i=0; i<8; i++) { \
		if (lightState[i] == 1) { \
		vec4 myLightDiffuse = lightDiffuse[i]; \
		vec4 myLightAmbient = lightAmbient[i]; \
		vec4 myLightSpecular = lightSpecular[i]; \
		vec4 myLightPosition = lightPosition[i]; \
  \
		/* normal, light, view, and light reflection vectors */ \
		vec3 lightv = normalize(myLightPosition.xyz-myPosition.xyz); \
		vec3 refl = reflect (-lightv, norm); \
 \
		if (backFacing) { \
			/* diffuse light computation */ \
			diffuse += max (0.0, dot(lightv, norm))*fw_BackMaterial.diffuse*myLightDiffuse; \
 \
			/* ambient light computation */ \
			ambient += fw_BackMaterial.ambient*myLightAmbient; \
 \
			/* Specular light computation */ \
			if (dot(lightv, viewv) > 0.0) { \
				specular += pow(max(0.0, dot(viewv, refl)), \
					fw_FrontMaterial.shininess)*fw_BackMaterial.specular*myLightSpecular; \
			} \
		} else { \
 \
			/* diffuse light computation */ \
			diffuse += max (0.0, dot(lightv, norm))*fw_FrontMaterial.diffuse*myLightDiffuse; \
 \
			/* ambient light computation */ \
			ambient += fw_FrontMaterial.ambient*myLightAmbient; \
 \
			/* Specular light computation */ \
			if (dot(lightv, viewv) > 0.0) { \
				specular += pow(max(0.0, dot(viewv, refl)), \
					fw_FrontMaterial.shininess)*fw_FrontMaterial.specular*myLightSpecular; \
			} \
		} \
		} \
	} \
	return clamp(vec3(ambient+diffuse+specular+emissive), 0.0, 1.0); \
} \
void main () { \
	gl_FragColor = vec4(ADSLightModel(Norm,Pos),1.); \
} \
 " ;

static char *noAppearanceNoMaterialFragmentShader =
" void main () {gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);}";

static char *noAppearanceNoMaterialVertexShader = " \
	attribute      vec4 fw_Vertex; \
	uniform        mat4 fw_ModelViewMatrix; \
	uniform        mat4 fw_ProjectionMatrix; \
	void main(void) { \
	       gl_Position = fw_ProjectionMatrix * fw_ModelViewMatrix * fw_Vertex; \
	}";


/****************************************************************************************/
/*											*/
/* Shaders where we have a specific Color node (check out IndexedFaceSets). The Color   */
/* replaces the diffuse component of the colour of the appearance calculations.  	*/
/*											*/
/* Basically, these shaders are copies of the above, with the node colour sent in	*/
/*											*/
/****************************************************************************************/

/* noMaterialNoAppearance - just use the backgroundSphereShader */

/* this is the same as the phongFragmentShader, but replace the diffuseColor with the v_color from the shape */
static const char *phongFragmentColourShader =  " \
\
struct fw_MaterialParameters { \
                vec4 emission; \
                vec4 ambient; \
                vec4 diffuse; \
                vec4 specular; \
                float shininess; \
        }; \
 \
uniform fw_MaterialParameters fw_FrontMaterial; \
uniform fw_MaterialParameters fw_BackMaterial; \
\
varying vec3 Norm;  \
varying vec4 Pos;  \
varying vec4 v_color; \
\
uniform int lightState[8]; \
uniform float light_linAtten[8]; \
uniform float light_constAtten[8]; \
uniform float light_quadAtten[8]; \
uniform float lightSpotCut[8]; \
uniform float lightSpotExp[8]; \
uniform vec4 lightAmbient[8]; \
uniform vec4 lightDiffuse[8]; \
uniform vec4 lightPosition[8]; \
uniform vec4 lightSpecular[8]; \
\
/* use ADSLightModel here \
 the ADS colour is returned from the function. \
*/  \
vec3 ADSLightModel(in vec3 myNormal, in vec4 myPosition) { \
	int i; \
	vec4 diffuse = vec4(0., 0., 0., 0.); \
	vec4 ambient = vec4(0., 0., 0., 0.); \
	vec4 specular = vec4(0., 0., 0., 1.); \
 \
	vec3 norm = normalize (myNormal); \
	vec3 viewv = -normalize(myPosition.xyz); \
	vec4 emissive = fw_FrontMaterial.emission; \
\
	/* apply the lights to this material */ \
	for (i=0; i<8; i++) { \
		if (lightState[i] == 1) { \
		vec4 myLightDiffuse = lightDiffuse[i]; \
		vec4 myLightAmbient = lightAmbient[i]; \
		vec4 myLightSpecular = lightSpecular[i]; \
		vec4 myLightPosition = lightPosition[i]; \
  \
		/* normal, light, view, and light reflection vectors */ \
		vec3 lightv = normalize(myLightPosition.xyz-myPosition.xyz); \
		vec3 refl = reflect (-lightv, norm); \
 \
		/* diffuse light computation */ \
		/* NOTE THE SUBSTITUTION OF v_color HERE */ \
		diffuse += max (0.0, dot(lightv, norm))*v_color*myLightDiffuse; \
 \
		/* ambient light computation */ \
		ambient += fw_FrontMaterial.ambient*myLightAmbient; \
 \
		/* Specular light computation */ \
		if (dot(lightv, viewv) > 0.0) { \
			specular += pow(max(0.0, dot(viewv, refl)), \
				fw_FrontMaterial.shininess)*fw_FrontMaterial.specular*myLightSpecular; \
		} \
		} \
	} \
	return clamp(vec3(ambient+diffuse+specular+emissive), 0.0, 1.0); \
} \
void main () { \
	gl_FragColor = vec4(ADSLightModel(Norm,Pos),1.); \
} \
 " ;

/* this is the same as the phongTwoSidedFragmentShader, but replace the diffuseColor with the v_color from the shape */
static const char *phongTwoSidedFragmentColourShader =  " \
\
struct fw_MaterialParameters { \
                vec4 emission; \
                vec4 ambient; \
                vec4 diffuse; \
                vec4 specular; \
                float shininess; \
        }; \
 \
uniform fw_MaterialParameters fw_FrontMaterial; \
uniform fw_MaterialParameters fw_BackMaterial; \
\
varying vec3 Norm;  \
varying vec4 Pos;  \
varying vec4 v_color; \
\
uniform int lightState[8]; \
uniform float light_linAtten[8]; \
uniform float light_constAtten[8]; \
uniform float light_quadAtten[8]; \
uniform float lightSpotCut[8]; \
uniform float lightSpotExp[8]; \
uniform vec4 lightAmbient[8]; \
uniform vec4 lightDiffuse[8]; \
uniform vec4 lightPosition[8]; \
uniform vec4 lightSpecular[8]; \
\
/* use ADSLightModel here \
 the ADS colour is returned from the function. \
*/  \
vec3 ADSLightModel(in vec3 myNormal, in vec4 myPosition) { \
	int i; \
	vec4 diffuse = vec4(0., 0., 0., 0.); \
	vec4 ambient = vec4(0., 0., 0., 0.); \
	vec4 specular = vec4(0., 0., 0., 1.); \
	vec3 viewv = -normalize(myPosition.xyz); \
	vec3 norm = normalize (myNormal); \
	vec4 emissive; \
\
	bool backFacing = (dot(norm,viewv) < 0.0); \
 \
	/* back Facing materials - flip the normal */ \
	if (backFacing) { \
		norm = -norm; \
		emissive = fw_BackMaterial.emission;	\
	} else { \
		emissive = fw_FrontMaterial.emission;	\
	} \
\
	/* apply the lights to this material */ \
	for (i=0; i<8; i++) { \
		if (lightState[i] == 1) { \
		vec4 myLightDiffuse = lightDiffuse[i]; \
		vec4 myLightAmbient = lightAmbient[i]; \
		vec4 myLightSpecular = lightSpecular[i]; \
		vec4 myLightPosition = lightPosition[i]; \
  \
		/* normal, light, view, and light reflection vectors */ \
		vec3 lightv = normalize(myLightPosition.xyz-myPosition.xyz); \
		vec3 refl = reflect (-lightv, norm); \
 \
		if (backFacing) { \
			/* diffuse light computation */ \
			/* Colour material - SUBSTITUTE v_color HERE */ \
			diffuse += max (0.0, dot(lightv, norm))*v_color*myLightDiffuse; \
 \
			/* ambient light computation */ \
			ambient += fw_BackMaterial.ambient*myLightAmbient; \
 \
			/* Specular light computation */ \
			if (dot(lightv, viewv) > 0.0) { \
				specular += pow(max(0.0, dot(viewv, refl)), \
					fw_FrontMaterial.shininess)*fw_BackMaterial.specular*myLightSpecular; \
			} \
		} else { \
 \
			/* diffuse light computation */ \
			/* Colour material - SUBSTITUTE v_color HERE */ \
			diffuse += max (0.0, dot(lightv, norm))*v_color*myLightDiffuse; \
 \
			/* ambient light computation */ \
			ambient += fw_FrontMaterial.ambient*myLightAmbient; \
 \
			/* Specular light computation */ \
			if (dot(lightv, viewv) > 0.0) { \
				specular += pow(max(0.0, dot(viewv, refl)), \
					fw_FrontMaterial.shininess)*fw_FrontMaterial.specular*myLightSpecular; \
			} \
		} \
		} \
	} \
	return clamp(vec3(ambient+diffuse+specular+emissive), 0.0, 1.0); \
} \
void main () { \
	gl_FragColor = vec4(ADSLightModel(Norm,Pos),1.); \
} \
 " ;

static char *phongSimpleVertexColourShader = " \
	varying vec3 Norm; \
	varying vec4 Pos; \
        varying        vec4 v_color; \
	attribute      vec4 fw_Vertex; \
	attribute      vec3 fw_Normal; \
	attribute      vec4 fw_Color; \
	uniform        mat4 fw_ModelViewMatrix; \
	uniform        mat4 fw_ProjectionMatrix; \
	uniform        mat3 fw_NormalMatrix; \
	void main(void) { \
	       Norm = normalize(fw_NormalMatrix * fw_Normal); \
	       Pos = fw_ModelViewMatrix * fw_Vertex;  \
	       gl_Position = fw_ProjectionMatrix * fw_ModelViewMatrix * fw_Vertex; \
		v_color = fw_Color; \
	}";







/****************************************************************************************/

static int getGenericShaderSource (char **vertexSource, char **fragmentSource, char **geometrySource, shader_type_t whichOne) {
	/* initialize */
	*vertexSource = NULL;
	*fragmentSource = NULL;
	*geometrySource = NULL;


	switch (whichOne) {
		/* Background, this is the simple shader for doing the sky/ground colours */
		case backgroundSphereShader: {
			*fragmentSource = backgroundSphereShaderFragment;
			*vertexSource  = backgroundSphereShaderVertex;
			break;
		}


		/* background, this is the simple shader for adding textures */
		case backgroundTextureBoxShader: {
			*fragmentSource = backgroundTextureBoxShaderFragment;
			*vertexSource = backgroundTextureBoxShaderVertex;
			break;
		}

		/* generic polyreps */
		case noTexOneMaterialShader: {
			*fragmentSource = phongFragmentShader;
                	*vertexSource = phongSimpleVertexShader;
			break;
		}
		case noMaterialNoAppearanceShader: {
			*fragmentSource = noAppearanceNoMaterialFragmentShader;
			*vertexSource = noAppearanceNoMaterialVertexShader;
			break;
		}
		case noTexTwoMaterialShader : {
			*fragmentSource = phongTwoSidedFragmentShader;
                	*vertexSource = phongSimpleVertexShader;
			break;
		}

		case oneTexOneMaterialShader: {
			*fragmentSource = oneTexFragmentShader;
                	*vertexSource = oneTexVertexShader;
			break;
		}

		/* nodes with Color node - from spec, 11.4.2 Color:
			"Color nodes are only used to specify multiple colours for a 
			single geometric shape, such as colours for the faces or vertices 
			of an IndexedFaceSet. A Material node is used to specify the overall 
			material parameters of lit geometry. If both a Material node and a 
			Color node are specified for a geometric shape, the colours shall 
			replace the diffuse component of the material."
		*/

		case noTexOneMaterialColourShader: {
			*fragmentSource = phongFragmentColourShader;
                	*vertexSource = phongSimpleVertexColourShader;
			break;
		}

		case noTexTwoMaterialColourShader: {
			*fragmentSource = phongTwoSidedFragmentColourShader;
                	*vertexSource = phongSimpleVertexColourShader;
			break;
		}


		/* still to be written */
		case oneTexTwoMaterialColourShader:
		case oneTexOneMaterialColourShader:


		case oneTexTwoMaterialShader:
		case oneTexTwoMaterialSphereShader: {
			printf ("warning! shader still to be written at %s:%d\n",__FILE__,__LINE__);
			return FALSE;
			break;
		}




		/* Sphere Geometry shader */
		case noMaterialNoAppearanceSphereShader: {
			*fragmentSource = noAppearanceNoMaterialFragmentShader;
                	*vertexSource = noTexVertexShaderForSphereGeomShader;
			*geometrySource = noAppearanceSphereGeomShader;
			break;
		}

		case noTexOneMaterialSphereShader: {
			*fragmentSource = phongFragmentShader;
                	*vertexSource = noTexVertexShaderForSphereGeomShader;
			*geometrySource = noTexSphereGeomShader;
			break;
		}


        	case noTexTwoMaterialSphereShader: {
			*fragmentSource = phongTwoSidedFragmentShader;
                	*vertexSource = noTexVertexShaderForSphereGeomShader;
			*geometrySource = noTexSphereGeomShader;
			break;
		}
		case oneTexOneMaterialSphereShader: {
			*fragmentSource = oneTexFragmentShader;
                	*vertexSource = oneTexVertexShaderForSphereGeomShader;
			*geometrySource = oneTexSphereGeomShader;
			break;
		}

		/* still to be written */
		case complexTexOneMaterialShader:
		case complexTexTwoMaterialShader:
		case complexTexOneMaterialSphereShader:
		case complexTexTwoMaterialSphereShader: {
			printf ("warning! shader still to be written at %s:%d\n",__FILE__,__LINE__);
			return FALSE;
			break;
		}


		default: {
			printf ("getShaderSource, do not handle %d properly yet\n",whichOne);
			return FALSE;
		}
	}

	return TRUE;
}

/* find these names in our shaders */
static void getShaderCommonInterfaces (s_shader_capabilities_t *me) {
	GLuint myProg = me->myShaderProgram;

	#ifdef DEBUG
	{
	GLsizei count;
	GLuint shaders[10];
	GLint  xxx[10];
	int i;
	GLchar sl[3000];

	/*
	printf ("getShaderCommonInterfaces, I am program %d\n",myProg);

	if (glIsProgram(myProg)) printf ("getShaderCommonInterfaces, %d is a program\n",myProg); else printf ("hmmm - it is not a program!\n");
	glGetAttachedShaders(myProg,10,&count,shaders);
	printf ("got %d attached shaders, they are: \n",count);
	for (i=0; i<count; i++) {
		GLsizei len;

		printf ("%d\n",shaders[i]);
		glGetShaderSource(shaders[i],3000,&len,sl);
		printf ("len %d\n",len);
		printf ("sl: %s\n",sl);
	}
	glGetProgramiv(myProg,GL_INFO_LOG_LENGTH, xxx); printf ("GL_INFO_LOG_LENGTH_STATUS %d\n",xxx[0]);
	glGetProgramiv(myProg,GL_LINK_STATUS, xxx); printf ("GL_LINK_STATUS %d\n",xxx[0]);
	glGetProgramiv(myProg,GL_VALIDATE_STATUS, xxx); printf ("GL_VALIDATE_STATUS %d\n",xxx[0]);
	glGetProgramiv(myProg,GL_ACTIVE_ATTRIBUTES, xxx); printf ("GL_ACTIVE_ATTRIBUTES %d\n",xxx[0]);
	glGetProgramiv(myProg,GL_ACTIVE_UNIFORMS, xxx); printf ("GL_ACTIVE_UNIFORMS %d\n",xxx[0]);
*/
	glGetProgramiv(myProg,GL_INFO_LOG_LENGTH, xxx);
	if (xxx[0] != 0) {
		#define MAX_INFO_LOG_SIZE 512
                GLchar infoLog[MAX_INFO_LOG_SIZE];
                glGetProgramInfoLog(myProg, MAX_INFO_LOG_SIZE, NULL, infoLog);
		printf ("log: %s\n",infoLog);


	}
	}
	#endif /* DEBUG */


	me->myMaterialEmission = GET_UNIFORM(myProg,"fw_FrontMaterial.emission");
	me->myMaterialDiffuse = GET_UNIFORM(myProg,"fw_FrontMaterial.diffuse");
	me->myMaterialShininess = GET_UNIFORM(myProg,"fw_FrontMaterial.shininess");
	me->myMaterialAmbient = GET_UNIFORM(myProg,"fw_FrontMaterial.ambient");
	me->myMaterialSpecular = GET_UNIFORM(myProg,"fw_FrontMaterial.specular");

	me->myMaterialBackEmission = GET_UNIFORM(myProg,"fw_BackMaterial.emission");
	me->myMaterialBackDiffuse = GET_UNIFORM(myProg,"fw_BackMaterial.diffuse");
	me->myMaterialBackShininess = GET_UNIFORM(myProg,"fw_BackMaterial.shininess");
	me->myMaterialBackAmbient = GET_UNIFORM(myProg,"fw_BackMaterial.ambient");
	me->myMaterialBackSpecular = GET_UNIFORM(myProg,"fw_BackMaterial.specular");

        me->lightState = GET_UNIFORM(myProg,"lightState");
        me->lightAmbient = GET_UNIFORM(myProg,"lightAmbient");
        me->lightDiffuse = GET_UNIFORM(myProg,"lightDiffuse");
        me->lightSpecular = GET_UNIFORM(myProg,"lightSpecular");
        me->lightPosition = GET_UNIFORM(myProg,"lightPosition");


	me->ModelViewMatrix = GET_UNIFORM(myProg,"fw_ModelViewMatrix");
	me->ProjectionMatrix = GET_UNIFORM(myProg,"fw_ProjectionMatrix");
	me->NormalMatrix = GET_UNIFORM(myProg,"fw_NormalMatrix");
	me->Vertices = GET_ATTRIB(myProg,"fw_Vertex");
	me->Normals = GET_ATTRIB(myProg,"fw_Normal");
	me->Colours = GET_ATTRIB(myProg,"fw_Color");

	me->TexCoords = GET_ATTRIB(myProg,"fw_TexCoords");
	me->Texture0 = GET_UNIFORM(myProg,"fw_Texture0");

	/* for Sphere geometry shader */
	me->specialUniform1 = GET_UNIFORM(myProg,"sphereRadius");

	#ifdef DEBUG
	printf ("shader uniforms: vertex %d normal %d modelview %d projection %d\n",
		me->Vertices, me->Normals, me->ModelViewMatrix, me->ProjectionMatrix); 
	#endif
}


static void printCompilingShader() {
/*
printf ("compiling shader: ");
switch (whichOne) {
case backgroundSphereShader: printf ("backgroundSphereShader\n"); break;
case backgroundTextureBoxShader: printf ("backgroundTextureBoxShader\n"); break;
case noTexOneMaterialShader: printf ("noTexOneMaterialShader\n"); break;
case noMaterialNoAppearanceShader: printf ("noMaterialNoAppearanceShader\n"); break;
case noTexTwoMaterialShader: printf ("noTexTwoMaterialShader\n"); break;
case noMaterialNoAppearanceSphereShader: printf ("noMaterialNoAppearanceSphereShader\n"); break;
case noTexOneMaterialSphereShader: printf ("noTexOneMaterialSphereShader\n"); break;
case noTexTwoMaterialSphereShader: printf ("noTexTwoMaterialSphereShader\n"); break;
case oneTexOneMaterialShader: printf ("oneTexOneMaterialShader\n"); break;
case oneTexTwoMaterialShader: printf ("oneTexTwoMaterialShader\n"); break;
case oneTexOneMaterialSphereShader: printf ("oneTexOneMaterialSphereShader\n"); break;
case oneTexTwoMaterialSphereShader: printf ("oneTexTwoMaterialSphereShader\n"); break;
case complexTexOneMaterialShader: printf ("complexTexOneMaterialShader\n"); break;
case complexTexTwoMaterialShader: printf ("complexTexTwoMaterialShader\n"); break;
case complexTexOneMaterialSphereShader: printf ("complexTexOneMaterialSphereShader\n"); break;
case complexTexTwoMaterialSphereShader: printf ("complexTexTwoMaterialSphereShader\n"); break;
case noTexTwoMaterialColourShader: printf ("noTexTwoMaterialColourShader\n"); break;
case noTexOneMaterialColourShader: printf ("noTexOneMaterialColourShader\n"); break;
case oneTexTwoMaterialColourShader: printf ("oneTexTwoMaterialColourShader\n"); break;
case oneTexOneMaterialColourShader: printf ("oneTexOneMaterialColourShader\n"); break;
*/


}


/* read in shaders and place the resulting shader program in the "whichShader" field if success. */
static void getGenericShader(shader_type_t whichOne) {
	GLint success;
	GLuint myVertexShader = 0;
	GLuint myFragmentShader= 0;
	GLuint myGeometryShader=0;
	GLuint myProg = 0;
	s_shader_capabilities_t *myShader;
	char *vertexSource[2];
	char  *fragmentSource[2];
	char *geometrySource[2];

	/* pointerize this */
	myShader = &gglobal()->display..backgroundShaderArrays[whichOne];
	myProg = glCreateProgram(); /* CREATE_PROGRAM */
	(*myShader).myShaderProgram = myProg;

	/* assume the worst... */
	(*myShader).compiledOK = FALSE;
	
	/* we put the sources in 2 formats, allows for differing GL/GLES prefixes */
	if (!getGenericShaderSource (&vertexSource[1], &fragmentSource[1], &geometrySource[1], whichOne)) {
		return;
	}

	#ifdef GL_ES_VERSION_2_0
	vertexSource[0] = "";
	fragmentSource[0] = " \
		precision lowp float;\n \
	";
	geometrySource[0] = "";


	#else

	vertexSource[0] = "";
	fragmentSource[0] = "";
	geometrySource[0] = " \
		#version 120\n \
		#extension GL_EXT_gpu_shader4: enable\n \
		#extension GL_EXT_geometry_shader4: enable\n \
	";
	#endif


	/* geometryShader */
	if (geometrySource[1] != NULL) {

#ifndef GL_ES_VERSION_2_0

		myGeometryShader = CREATE_SHADER(GL_GEOMETRY_SHADER_EXT);
		SHADER_SOURCE(myGeometryShader, 2, (const GLchar **) &geometrySource, NULL);
		COMPILE_SHADER(myGeometryShader);
		GET_SHADER_INFO(myGeometryShader, COMPILE_STATUS, &success);
		if (!success) {
			shaderErrorLog(myGeometryShader,"GEOMETRY");
		} else {
			GLint n;

	                glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &n);
        	        /* printf ("compile_geomshader, geom shader max vertices at start %d\n",n); */
                	glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT, &n);
                	/* printf ("compile_geomshader, geom total output components at start %d\n",n); */

                	glProgramParameteriEXT (myProg,GL_GEOMETRY_INPUT_TYPE_EXT,GL_TRIANGLES);
                	glProgramParameteriEXT (myProg, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
                	glProgramParameteriEXT (myProg, GL_GEOMETRY_VERTICES_OUT_EXT, 1024);
			ATTACH_SHADER(myProg, myGeometryShader);
		}
#else
printf ("HMMM - GL_ES_VERSION_2_0 and Geometry shader\n");
#endif
	}


	myVertexShader = CREATE_SHADER (VERTEX_SHADER);
	SHADER_SOURCE(myVertexShader, 2, (const GLchar **) vertexSource, NULL);
	COMPILE_SHADER(myVertexShader);
	GET_SHADER_INFO(myVertexShader, COMPILE_STATUS, &success);
	if (!success) {
		shaderErrorLog(myVertexShader,"VERTEX");
	} else {

		ATTACH_SHADER(myProg, myVertexShader);
	}


	/* get Fragment shader */
	myFragmentShader = CREATE_SHADER (FRAGMENT_SHADER);
	SHADER_SOURCE(myFragmentShader, 2, (const GLchar **) fragmentSource, NULL);
	COMPILE_SHADER(myFragmentShader);
	GET_SHADER_INFO(myFragmentShader, COMPILE_STATUS, &success);
	if (!success) {
		shaderErrorLog(myFragmentShader,"FRAGMENT");
	} else {
		ATTACH_SHADER(myProg, myFragmentShader);
	}


	LINK_SHADER(myProg);
	glGetProgramiv(myProg,GL_LINK_STATUS, &success); 
	(*myShader).compiledOK = (success == GL_TRUE);

	getShaderCommonInterfaces(myShader);
}

#endif /* ifdef SHADERS_2011 */

static void handle_GeoLODRange(struct X3D_GeoLOD *node) {
	int oldInRange, handled;
	GLDOUBLE cx,cy,cz;
	handled = 0;
	/* find the length of the line between the moved center and our current viewer position */
	cx = Viewer.currentPosInModel.x - node->__movedCoords.c[0];
	cy = Viewer.currentPosInModel.y - node->__movedCoords.c[1];
	cz = Viewer.currentPosInModel.z - node->__movedCoords.c[2];

	/* printf ("geoLOD, distance between me and center is %lf\n", sqrt (cx*cx + cy*cy + cz*cz)); */

	/* try to see if we are closer than the range */
	oldInRange = node->__inRange;
	if((cx*cx+cy*cy+cz*cz) > (node->range*X3D_GEOLOD(node)->range)) {
		node->__inRange = FALSE;
	} else {
		node->__inRange = TRUE;
	}

	
	if (oldInRange != node->__inRange) {

		#ifdef VERBOSE
		if (node->__inRange) printf ("TRUE:  "); else printf ("FALSE: ");
		printf ("range changed; level %d, comparing %lf:%lf:%lf and range %lf node %u\n",
			node->__level, cx,cy,cz, node->range, node);
		#endif

		/* initialize the "children" field, if required */
		if (node->children.p == NULL) node->children.p=MALLOC(void *,sizeof(void *) * 4);

		if (node->__inRange == TRUE) { //dug9 FALSE) {
			#ifdef VERBOSE
			printf ("GeoLOD %u level %d, inRange set to FALSE, range %lf\n",node, node->__level, node->range);
			#endif		
			node->level_changed = 1;
			node->children.p[0] = node->__child1Node; 
			node->children.p[1] = node->__child2Node; 
			node->children.p[2] = node->__child3Node; 
			node->children.p[3] = node->__child4Node; 
			node->children.n = 4;
		} else {
			#ifdef VERBOSE
			printf ("GeoLOD %u level %d, inRange set to TRUE range %lf\n",node, node->__level, node->range);
			#endif
			node->level_changed = 0;
			node->children.n = 0;
			if( node->__rootUrl )
			{
				node->children.p[0] = node->__rootUrl;
				node->children.n = 1;
			}
			else if( node->rootNode.p && node->rootNode.p[0] )
			{
				node->children.p[0] = node->rootNode.p[0]; 
				node->children.n = 1;
			}
		}
		MARK_EVENT(X3D_NODE(node), offsetof (struct X3D_GeoLOD, level_changed));
		MARK_EVENT(X3D_NODE(node), offsetof (struct X3D_GeoLOD, children));
		oldInRange = X3D_GEOLOD(node)->__inRange;

		/* lets work out extent here */
		INITIALIZE_EXTENT;
		/* printf ("geolod range changed, initialized extent, czyzsq %4.2f rangesw %4.2f from %4.2f %4.2f %4.2f\n",
cx*cx+cy*cy+cz*cz,node->range*node->range,cx,cy,cz); */
		update_node(X3D_NODE(node));
	}
}

/* draw a simple bounding box around an object */
void drawBBOX(struct X3D_Node *node) {
#ifndef GL_ES_VERSION_2_0
/* debugging */	FW_GL_COLOR3F((float)1.0,(float)0.6,(float)0.6);
/* debugging */
/* debugging */	/* left group */
/* debugging */	glBegin(GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MIN_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MIN_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MIN_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MAX_Y, node->EXTENT_MIN_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MAX_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MAX_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MIN_Y, node->EXTENT_MAX_Z);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MAX_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	/* right group */
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MIN_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MIN_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MIN_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MAX_Y, node->EXTENT_MIN_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MAX_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MAX_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MIN_Y, node->EXTENT_MAX_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MAX_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	/* joiners */
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MIN_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MIN_Y, node->EXTENT_MIN_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MIN_Y, node->EXTENT_MAX_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MIN_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MAX_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MAX_Y, node->EXTENT_MIN_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MAX_Y, node->EXTENT_MAX_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MAX_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
#endif

}

static void calculateNearFarplanes(struct X3D_Node *vpnode) {
	struct point_XYZ bboxPoints[8];
	GLDOUBLE cfp = DBL_MIN;
	GLDOUBLE cnp = DBL_MAX;
	GLDOUBLE MM[16];

	int ci;

	#ifdef VERBOSE
	printf ("have a bound viewpoint... lets calculate our near/far planes from it \n");
	printf ("we are currently at %4.2f %4.2f %4.2f\n",Viewer.currentPosInModel.x, Viewer.currentPosInModel.y, Viewer.currentPosInModel.z);
	#endif


	/* verify parameters here */
	if ((vpnode->_nodeType != NODE_Viewpoint) && 
		(vpnode->_nodeType != NODE_OrthoViewpoint) &&
		(vpnode->_nodeType != NODE_GeoViewpoint)) {
		printf ("can not do this node type yet %s, for cpf\n",stringNodeType(vpnode->_nodeType));
		Viewer.nearPlane = DEFAULT_NEARPLANE;
		Viewer.farPlane = DEFAULT_FARPLANE;
		Viewer.backgroundPlane = DEFAULT_BACKGROUNDPLANE;
		return;
	}	

	if (rootNode == NULL) {
		return; /* nothing to display yet */
	}

	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, MM);

		#ifdef VERBOSE
		printf ("rootNode extents x: %4.2f %4.2f  y:%4.2f %4.2f z: %4.2f %4.2f\n",
				rootNode->EXTENT_MAX_X, rootNode->EXTENT_MIN_X,
				rootNode->EXTENT_MAX_Y, rootNode->EXTENT_MIN_Y,
				rootNode->EXTENT_MAX_Z, rootNode->EXTENT_MIN_Z);
		#endif
		/* make up 8 vertices for our bounding box, and place them within our view */
		moveAndRotateThisPoint(&bboxPoints[0], rootNode->EXTENT_MIN_X, rootNode->EXTENT_MIN_Y, rootNode->EXTENT_MIN_Z,MM);
		moveAndRotateThisPoint(&bboxPoints[1], rootNode->EXTENT_MIN_X, rootNode->EXTENT_MIN_Y, rootNode->EXTENT_MAX_Z,MM);
		moveAndRotateThisPoint(&bboxPoints[2], rootNode->EXTENT_MIN_X, rootNode->EXTENT_MAX_Y, rootNode->EXTENT_MIN_Z,MM);
		moveAndRotateThisPoint(&bboxPoints[3], rootNode->EXTENT_MIN_X, rootNode->EXTENT_MAX_Y, rootNode->EXTENT_MAX_Z,MM);
		moveAndRotateThisPoint(&bboxPoints[4], rootNode->EXTENT_MAX_X, rootNode->EXTENT_MIN_Y, rootNode->EXTENT_MIN_Z,MM);
		moveAndRotateThisPoint(&bboxPoints[5], rootNode->EXTENT_MAX_X, rootNode->EXTENT_MIN_Y, rootNode->EXTENT_MAX_Z,MM);
		moveAndRotateThisPoint(&bboxPoints[6], rootNode->EXTENT_MAX_X, rootNode->EXTENT_MAX_Y, rootNode->EXTENT_MIN_Z,MM);
		moveAndRotateThisPoint(&bboxPoints[7], rootNode->EXTENT_MAX_X, rootNode->EXTENT_MAX_Y, rootNode->EXTENT_MAX_Z,MM);
	
		for (ci=0; ci<8; ci++) {
			#ifdef VERBOSE
			printf ("moved bbox node %d is %4.2f %4.2f %4.2f\n",ci,bboxPoints[ci].x, bboxPoints[ci].y, bboxPoints[ci].z);
			#endif
	
			if (-(bboxPoints[ci].z) > cfp) cfp = -(bboxPoints[ci].z);
			if (-(bboxPoints[ci].z) < cnp) cnp = -(bboxPoints[ci].z);
		}

	/* lets bound check here, both must be positive, and farPlane more than DEFAULT_NEARPLANE */
	/* because we may be navigating towards the shapes, we give the nearPlane a bit of room, otherwise
	   we might miss part of the geometry that comes closest to us */
	cnp = cnp/2.0;
	if (cnp<DEFAULT_NEARPLANE) cnp = DEFAULT_NEARPLANE;

	if (cfp<1.0) cfp = 1.0;	
	/* if we are moving, or if we have something with zero depth, floating point calculation errors could
	   give us a geometry that is at (or, over) the far plane. Eg, tests/49.wrl, where we have Text nodes,
	   can give us this issue; so lets give us a bit of leeway here, too */
	cfp *= 1.25;


	#ifdef VERBOSE
	printf ("cnp %lf cfp before leaving room for Background %lf\n",cnp,cfp);
	#endif

	/* lets use these values; leave room for a Background or TextureBackground node here */
	Viewer.nearPlane = cnp; 
	/* backgroundPlane goes between the farthest geometry, and the farPlane */
	if (background_stack[0]!= 0) {
		Viewer.farPlane = cfp * 10.0;
		Viewer.backgroundPlane = cfp*5.0;
	} else {
		Viewer.farPlane = cfp;
		Viewer.backgroundPlane = cfp; /* just set it to something */
	}
}

void doglClearColor() {
	ppOpenGL_Utils p;
	ttglobal tg = gglobal();
	p = (ppOpenGL_Utils)tg->OpenGL_Utils.prv;
	FW_GL_CLEAR_COLOR(p->cc_red, p->cc_green, p->cc_blue, p->cc_alpha);
	tg->OpenGL_Utils.cc_changed = FALSE;
}

/* did we have a TextureTransform in the Appearance node? */
void start_textureTransform (struct X3D_Node *textureNode, int ttnum) {

	/* stuff common to all textureTransforms - gets undone at end_textureTransform */
	FW_GL_MATRIX_MODE(GL_TEXTURE);
       	/* done in RenderTextures now FW_GL_ENABLE(GL_TEXTURE_2D); */
	FW_GL_LOAD_IDENTITY();

	/* is this a simple TextureTransform? */
	if (textureNode->_nodeType == NODE_TextureTransform) {
		struct X3D_TextureTransform  *ttt = (struct X3D_TextureTransform *) textureNode;
		/*  Render transformations according to spec.*/
        	FW_GL_TRANSLATE_F(-((ttt->center).c[0]),-((ttt->center).c[1]), 0);		/*  5*/
        	FW_GL_SCALE_F(((ttt->scale).c[0]),((ttt->scale).c[1]),1);			/*  4*/
        	FW_GL_ROTATE_RADIANS(ttt->rotation,0,0,1);					/*  3*/
        	FW_GL_TRANSLATE_F(((ttt->center).c[0]),((ttt->center).c[1]), 0);		/*  2*/
        	FW_GL_TRANSLATE_F(((ttt->translation).c[0]), ((ttt->translation).c[1]), 0);	/*  1*/

	/* is this a MultiTextureTransform? */
	} else  if (textureNode->_nodeType == NODE_MultiTextureTransform) {
		struct X3D_MultiTextureTransform *mtt = (struct X3D_MultiTextureTransform *) textureNode;
		if (ttnum < mtt->textureTransform.n) {
			struct X3D_TextureTransform *ttt = (struct X3D_TextureTransform *) mtt->textureTransform.p[ttnum];
			/* is this a simple TextureTransform? */
			if (ttt->_nodeType == NODE_TextureTransform) {
				/*  Render transformations according to spec.*/
        			FW_GL_TRANSLATE_F(-((ttt->center).c[0]),-((ttt->center).c[1]), 0);		/*  5*/
        			FW_GL_SCALE_F(((ttt->scale).c[0]),((ttt->scale).c[1]),1);			/*  4*/
        			FW_GL_ROTATE_RADIANS(ttt->rotation,0,0,1);					/*  3*/
        			FW_GL_TRANSLATE_F(((ttt->center).c[0]),((ttt->center).c[1]), 0);		/*  2*/
        			FW_GL_TRANSLATE_F(((ttt->translation).c[0]), ((ttt->translation).c[1]), 0);	/*  1*/
			} else {
				printf ("MultiTextureTransform expected a textureTransform for texture %d, got %d\n",
					ttnum, ttt->_nodeType);
			}
		} else {
			printf ("not enough textures in MultiTextureTransform....\n");
		}

	} else if (textureNode->_nodeType == NODE_VRML1_Texture2Transform) {
		struct X3D_VRML1_Texture2Transform  *ttt = (struct X3D_VRML1_Texture2Transform *) textureNode;
		/*  Render transformations according to spec.*/
        	FW_GL_TRANSLATE_F(-((ttt->center).c[0]),-((ttt->center).c[1]), 0);		/*  5*/
        	FW_GL_SCALE_F(((ttt->scaleFactor).c[0]),((ttt->scaleFactor).c[1]),1);			/*  4*/
        	FW_GL_ROTATE_F(ttt->rotation,0,0,1);						/*  3*/
        	FW_GL_TRANSLATE_F(((ttt->center).c[0]),((ttt->center).c[1]), 0);		/*  2*/
        	FW_GL_TRANSLATE_F(((ttt->translation).c[0]), ((ttt->translation).c[1]), 0);	/*  1*/
	} else {
		printf ("expected a textureTransform node, got %d\n",textureNode->_nodeType);
	}
	FW_GL_MATRIX_MODE(GL_MODELVIEW);
}

/* did we have a TextureTransform in the Appearance node? */
void end_textureTransform (void) {
	FW_GL_MATRIX_MODE(GL_TEXTURE);
	FW_GL_LOAD_IDENTITY();
	FW_GL_MATRIX_MODE(GL_MODELVIEW);
}


/**
 *   fwl_initializa_GL: initialize GLEW (->rdr caps) and OpenGL initial state
 */
bool fwl_initialize_GL()
{
	char blankTexture[] = {0x40, 0x40, 0x40, 0xFF};
	ppOpenGL_Utils p;
	ttglobal tg = gglobal();
	p = (ppOpenGL_Utils)tg->OpenGL_Utils.prv;

#ifdef OLDCODE
OLDCODE#if !defined (FRONTEND_HANDLES_DISPLAY_THREAD)
OLDCODE	#if defined (TARGET_AQUA) 
OLDCODE		extern CGLContextObj myglobalContext;
OLDCODE                CGLSetCurrentContext(myglobalContext);
OLDCODE	#endif /* TARGET_AQUA */
OLDCODE#endif /* FRONTEND_HANDLES_DISPLAY_THREAD */
#endif // OLDCODE

	PRINT_GL_ERROR_IF_ANY("fwl_initialize_GL start 1");
	initialize_rdr_caps();

	PRINT_GL_ERROR_IF_ANY("fwl_initialize_GL start 2");
	initialize_rdr_functions();

	PRINT_GL_ERROR_IF_ANY("fwl_initialize_GL start 3");

	/* lets make sure everything is sync'd up */

#if KEEP_FV_INLIB
#if defined(TARGET_X11) || defined(TARGET_MOTIF)
	XFlush(Xdpy);
#endif
#endif /* KEEP_FV_INLIB */


	#ifdef SHADERS_2011

	/* for rendering Background and TextureBackground nodes */
	getGenericShader(backgroundSphereShader);
	getGenericShader(backgroundTextureBoxShader);


	/* for rendering Polyreps, and other shapes without Geometry and other special shaders */
        getGenericShader(noMaterialNoAppearanceShader);
        getGenericShader(noTexOneMaterialShader);
	getGenericShader(noTexTwoMaterialShader);
	getGenericShader(oneTexOneMaterialShader);
	getGenericShader(oneTexTwoMaterialShader);
	getGenericShader(complexTexOneMaterialShader);
	getGenericShader(complexTexTwoMaterialShader);


	/* Color node present in shape */
	/* note, noMaterialNoAppearanceColourShader is the same as the Background Sphere shader -we use that one */
	getGenericShader(noTexTwoMaterialColourShader);
	getGenericShader(noTexOneMaterialColourShader);
	getGenericShader(oneTexTwoMaterialColourShader);
	getGenericShader(oneTexOneMaterialColourShader);



	/* nodes with Geometry Shaders */
	#ifdef HAVE_GEOMETRY_SHADERS
        getGenericShader(noMaterialNoAppearanceSphereShader);
        getGenericShader(noTexOneMaterialSphereShader);
	getGenericShader(noTexTwoMaterialSphereShader);
	getGenericShader(oneTexOneMaterialSphereShader);
	getGenericShader(oneTexTwoMaterialSphereShader);
	getGenericShader(complexTexOneMaterialSphereShader);
	getGenericShader(complexTexTwoMaterialSphereShader);
	#endif /* HAVE_GEOMETRY_SHADERS */

	#endif /* SHADERS_2011 */

    
	PRINT_GL_ERROR_IF_ANY("fwl_initialize_GL start 4");
    
	/* Set up the OpenGL state. This'll get overwritten later... */
	#ifndef SHADERS_2011
	#if defined( _ANDROID )
	glClearDepthf(1.0);
	#else // _ANDROID
	FW_GL_CLEAR_DEPTH(1.0);
	#endif // _ANDROID	
	#endif
    
	PRINT_GL_ERROR_IF_ANY("fwl_initialize_GL start 5");
    

	FW_GL_MATRIX_MODE(GL_PROJECTION);
	FW_GL_LOAD_IDENTITY();
	FW_GL_MATRIX_MODE(GL_MODELVIEW);
    
	PRINT_GL_ERROR_IF_ANY("fwl_initialize_GL start 6");
    

	/* Configure OpenGL for our uses. */
        FW_GL_ENABLECLIENTSTATE(GL_VERTEX_ARRAY);
        FW_GL_ENABLECLIENTSTATE(GL_NORMAL_ARRAY);
	FW_GL_CLEAR_COLOR(p->cc_red, p->cc_green, p->cc_blue, p->cc_alpha);

	PRINT_GL_ERROR_IF_ANY("fwl_initialize_GL start 7");
    
	
	#ifndef SHADERS_2011
#if !(defined(_ANDROID))
	FW_GL_SHADEMODEL(GL_SMOOTH);
	FW_GL_HINT(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	FW_GL_ENABLE (GL_RESCALE_NORMAL);
#endif
	#endif
    
	PRINT_GL_ERROR_IF_ANY("fwl_initialize_GL start 8");
    

	FW_GL_DEPTHFUNC(GL_LEQUAL);
	FW_GL_ENABLE(GL_DEPTH_TEST);
    
	PRINT_GL_ERROR_IF_ANY("fwl_initialize_GL start 9");
    
	
	#ifndef GL_ES_VERSION_2_0
	{
		float gl_linewidth = gglobal()->Mainloop.gl_linewidth;
		FW_GL_LINEWIDTH(gl_linewidth);
		FW_GL_POINTSIZE(gl_linewidth);
	}
	#endif
    
	PRINT_GL_ERROR_IF_ANY("fwl_initialize_GL start a");
    


	/*
     * JAS - ALPHA testing for textures - right now we just use 0/1 alpha
     * JAS   channel for textures - true alpha blending can come when we sort
     * JAS   nodes.
	 */

	FW_GL_ENABLE(GL_BLEND);
	FW_GL_BLENDFUNC(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	//FW_GL_BLENDFUNC(GL_ONE, GL_ONE); wierd colours
	//FW_GL_BLENDFUNC(GL_SRC, GL_ONE_MINUS_SRC);

	//if this is enabled, VisibilitySensors must have an alpha of greater than 0.0
	//FW_GL_ENABLE(GL_ALPHA_TEST);
	//FW_GL_ALPHAFUNC(GL_GREATER, 0); 

	FW_GL_CLEAR(GL_COLOR_BUFFER_BIT);
    
	PRINT_GL_ERROR_IF_ANY("fwl_initialize_GL start b");
    

	#ifndef SHADERS_2011
	FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
#if !(defined(ANDROID))
	FW_GL_ENABLE(GL_NORMALIZE);
#endif
	#endif

	/* for textured appearance add specular highlights as a separate secondary color
	   redbook p.270, p.455 and http://www.gamedev.net/reference/programming/features/oglch9excerpt/

	   if we don't have texture we can disable this (less computation)...
	   but putting this here is already a saving ;)...
	*/
    
	PRINT_GL_ERROR_IF_ANY("fwl_initialize_GL start c0");
    
	/* keep track of light states; initial turn all lights off except for headlight */
	initializeLightTables();
    
	PRINT_GL_ERROR_IF_ANY("fwl_initialize_GL start c1");
    

	/* ensure state of GL_CULL_FACE */
	CULL_FACE_INITIALIZE;

	FW_GL_PIXELSTOREI(GL_UNPACK_ALIGNMENT,1);
	FW_GL_PIXELSTOREI(GL_PACK_ALIGNMENT,1);
    
	PRINT_GL_ERROR_IF_ANY("fwl_initialize_GL start c");
    

	do_shininess(GL_FRONT_AND_BACK,(float) 0.2);
	{
	//extern GLuint defaultBlankTexture;

        /* create an empty texture, defaultBlankTexture, to be used when a texture is loading, or if it fails */
        FW_GL_GENTEXTURES (1,&tg->Textures.defaultBlankTexture);
        FW_GL_BINDTEXTURE (GL_TEXTURE_2D, tg->Textures.defaultBlankTexture);
        FW_GL_TEXPARAMETERI( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        FW_GL_TEXPARAMETERI( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        FW_GL_TEXIMAGE2D(GL_TEXTURE_2D, 0, GL_RGBA,  1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blankTexture);
        
        PRINT_GL_ERROR_IF_ANY("fwl_initialize_GL start d");
        
	}

	return TRUE;
}

void BackEndClearBuffer(int which) {
	if(which == 2) {
		FW_GL_CLEAR(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	} else { 
		if(which==1) {
			FW_GL_CLEAR(GL_DEPTH_BUFFER_BIT);
		}
	}
}

/* turn off all non-headlight lights; will turn them on if required. */
void BackEndLightsOff() {
	int i;
	for (i=0; i<7; i++) {
		lightState(i, FALSE);
	}
}


///* OpenGL perform matrix state here */
//#define MAX_LARGE_MATRIX_STACK 32	/* depth of stacks */
//#define MAX_SMALL_MATRIX_STACK 2	/* depth of stacks */
//#define MATRIX_SIZE 16		/* 4 x 4 matrix */
//typedef GLDOUBLE MATRIX4[MATRIX_SIZE];
//
//static MATRIX4 FW_ModelView[MAX_LARGE_MATRIX_STACK];
//static MATRIX4 FW_ProjectionView[MAX_SMALL_MATRIX_STACK];
//static MATRIX4 FW_TextureView[MAX_SMALL_MATRIX_STACK];
// 
//static int modelviewTOS = 0;
//static int projectionviewTOS = 0;
//static int textureviewTOS = 0;
//
//static int whichMode = GL_MODELVIEW;
//static GLDOUBLE *currentMatrix = FW_ModelView[0];


void fw_glMatrixMode(GLint mode) {
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	p->whichMode = mode;
	
	#ifdef VERBOSE
	printf ("fw_glMatrixMode, projTOS %d, modTOS %d\n",p->projectionviewTOS,p->modelviewTOS);

	switch (p->whichMode) {
		case GL_PROJECTION: printf ("glMatrixMode(GL_PROJECTION)\n"); break;
		case GL_MODELVIEW: printf ("glMatrixMode(GL_MODELVIEW)\n"); break;
		case GL_TEXTURE: printf ("glMatrixMode(GL_TEXTURE)\n"); break;
	}
	#endif
	
	#ifndef GL_ES_VERSION_2_0
	glMatrixMode(mode); /* JAS - tell OpenGL what the current matrix mode is */
	#endif

	switch (p->whichMode) {
		case GL_PROJECTION: p->currentMatrix = (GLDOUBLE *) &p->FW_ProjectionView[p->projectionviewTOS]; break;
		case GL_MODELVIEW: p->currentMatrix = (GLDOUBLE *) &p->FW_ModelView[p->modelviewTOS]; break;
		case GL_TEXTURE: p->currentMatrix = (GLDOUBLE *) &p->FW_TextureView[p->textureviewTOS]; break;
		default: printf ("invalid mode sent in it is %d, expected one of %d %d %d\n",p->whichMode, GL_PROJECTION,GL_MODELVIEW,GL_TEXTURE);
	}

}

void fw_glLoadIdentity(void) {
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	loadIdentityMatrix(p->currentMatrix);
	FW_GL_LOADMATRIX(p->currentMatrix); 
}

#define PUSHMAT(a,b,c,d) case a: b++; if (b>=c) {b=c-1; printf ("stack overflow, whichmode %d\n",p->whichMode); } \
		memcpy ((void *)d[b], (void *)d[b-1],sizeof(GLDOUBLE)*16); p->currentMatrix = d[b]; break;

void fw_glPushMatrix(void) {
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	switch (p->whichMode) {
		PUSHMAT (GL_PROJECTION,p->projectionviewTOS,MAX_SMALL_MATRIX_STACK,p->FW_ProjectionView)
		PUSHMAT (GL_MODELVIEW,p->modelviewTOS,MAX_LARGE_MATRIX_STACK,p->FW_ModelView)
		PUSHMAT (GL_TEXTURE,p->textureviewTOS,MAX_SMALL_MATRIX_STACK,p->FW_TextureView)
		default :printf ("wrong mode in popMatrix\n");
	}
	/* if (p->whichMode == GL_PROJECTION) { printf ("	fw_glPushMatrix tos now %d\n",p->projectionviewTOS); }  */

 	FW_GL_LOADMATRIX(p->currentMatrix); 
#undef PUSHMAT
}

#define POPMAT(a,b,c) case a: b--; if (b<0) {b=0;printf ("popMatrix, stack underflow, whichMode %d\n",p->whichMode);} p->currentMatrix = c[b]; break;

void fw_glPopMatrix(void) {
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	switch (p->whichMode) {
		POPMAT(GL_PROJECTION,p->projectionviewTOS,p->FW_ProjectionView)
		POPMAT(GL_MODELVIEW,p->modelviewTOS,p->FW_ModelView)
		POPMAT (GL_TEXTURE,p->textureviewTOS,p->FW_TextureView)
		default :printf ("wrong mode in popMatrix\n");
	}
	/* if (whichMode == GL_PROJECTION) { printf ("	fw_glPopMatrix tos now %d\n",p->projectionviewTOS); } */

 	FW_GL_LOADMATRIX(p->currentMatrix); 
}
#undef POPMAT


void fw_glTranslated(GLDOUBLE x, GLDOUBLE y, GLDOUBLE z) {
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	//printf ("fw_glTranslated %lf %lf %lf\n",x,y,z);
	//printf ("translated, currentMatrix %p\n",p->currentMatrix);

	p->currentMatrix[12] = p->currentMatrix[0] * x + p->currentMatrix[4] * y + p->currentMatrix[8]  * z + p->currentMatrix[12];
	p->currentMatrix[13] = p->currentMatrix[1] * x + p->currentMatrix[5] * y + p->currentMatrix[9]  * z + p->currentMatrix[13];
	p->currentMatrix[14] = p->currentMatrix[2] * x + p->currentMatrix[6] * y + p->currentMatrix[10] * z + p->currentMatrix[14];
	p->currentMatrix[15] = p->currentMatrix[3] * x + p->currentMatrix[7] * y + p->currentMatrix[11] * z + p->currentMatrix[15];

 	FW_GL_LOADMATRIX(p->currentMatrix); 
}

void fw_glTranslatef(float x, float y, float z) {
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	//printf ("fw_glTranslatef %f %f %f\n",x,y,z);
	p->currentMatrix[12] = p->currentMatrix[0] * x + p->currentMatrix[4] * y + p->currentMatrix[8]  * z + p->currentMatrix[12];
	p->currentMatrix[13] = p->currentMatrix[1] * x + p->currentMatrix[5] * y + p->currentMatrix[9]  * z + p->currentMatrix[13];
	p->currentMatrix[14] = p->currentMatrix[2] * x + p->currentMatrix[6] * y + p->currentMatrix[10] * z + p->currentMatrix[14];
	p->currentMatrix[15] = p->currentMatrix[3] * x + p->currentMatrix[7] * y + p->currentMatrix[11] * z + p->currentMatrix[15];

	FW_GL_LOADMATRIX(p->currentMatrix); 
}

/* perform rotation, assuming that the angle is in radians. */
void fw_glRotateRad (GLDOUBLE angle, GLDOUBLE x, GLDOUBLE y, GLDOUBLE z) {
	MATRIX4 myMat;
	GLDOUBLE mag;
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	//printf ("fw_glRotateRad %lf %lf %lf %lf modTOS %d projTOS %d\n",angle,x,y,z,p->modelviewTOS,p->projectionviewTOS);
	//printmatrix2(p->currentMatrix,"in rad");
	loadIdentityMatrix (myMat);

	/* FIXME - any way we can ensure that the angles are normalized? */
	mag =  x*x + y*y + z*z; 

	/* bounds check - the axis is invalid. */
	if (APPROX(mag,0.00)) {
		return;
	}

	/* bounds check - angle is zero, no rotation happening here */
	if (APPROX(angle,0.0)) {
		return;
	}

	if (!APPROX(mag,1.0)) {
		struct point_XYZ in; struct point_XYZ out;
		in.x = x; in.y = y, in.z = z;
		vecnormal(&out,&in);
		x = out.x; y = out.y; z = out.z;
	}
	//printf ("rad, normalized axis %lf %lf %lf\n",x,y,z);


	matrotate(myMat,angle,x,y,z); 

	//printmatrix2 (myMat, "rotation matrix");
	matmultiply(p->currentMatrix,myMat,p->currentMatrix); 

	//printmatrix2 (p->currentMatrix,"currentMatrix after rotate");

	FW_GL_LOADMATRIX(p->currentMatrix); 
}

/* perform the rotation, assuming that the angle is in degrees */
void fw_glRotated (GLDOUBLE angle, GLDOUBLE x, GLDOUBLE y, GLDOUBLE z) {
	MATRIX4 myMat;
	GLDOUBLE mag;
	GLDOUBLE radAng;
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	/* convert angle from degrees to radians */
	/* FIXME - must try and make up a rotate-radians call to get rid of these incessant conversions */
	radAng = angle * 3.1415926536/ 180.0;

	loadIdentityMatrix (myMat);

	/* FIXME - any way we can ensure that the angles are normalized? */
	mag =  x*x + y*y + z*z; 

	/* bounds check - the axis is invalid. */
	if (APPROX(mag,0.00)) {
		return;
	}

	/* bounds check - angle is zero, no rotation happening here */
	if (APPROX(angle,0.0)) {
		return;
	}

	if (!APPROX(mag,1.0)) {
		struct point_XYZ in; struct point_XYZ out;
		in.x = x; in.y = y, in.z = z;
		vecnormal(&out,&in);
		x = out.x; y = out.y; z = out.z;
	}


	/* are the axis close to zero? */
	if (mag < 0.001) {
		return;
	}
	matrotate(myMat,radAng,x,y,z); 
	matmultiply(p->currentMatrix,p->currentMatrix,myMat); 

	FW_GL_LOADMATRIX(p->currentMatrix); 
}

void fw_glRotatef (float a, float x, float y, float z) {
	fw_glRotated((GLDOUBLE)a, (GLDOUBLE)x, (GLDOUBLE)y, (GLDOUBLE)z);
}

void fw_glScaled (GLDOUBLE x, GLDOUBLE y, GLDOUBLE z) {
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

//	printf ("glScaled(%5.4lf %5.4lf %5.4lf)\n",x,y,z);

	p->currentMatrix[0] *= x;   p->currentMatrix[4] *= y;   p->currentMatrix[8]  *= z;
	p->currentMatrix[1] *= x;   p->currentMatrix[5] *= y;   p->currentMatrix[9]  *= z;
	p->currentMatrix[2] *= x;   p->currentMatrix[6] *= y;   p->currentMatrix[10] *= z;
	p->currentMatrix[3] *= x;   p->currentMatrix[7] *= y;   p->currentMatrix[11] *= z;

	FW_GL_LOADMATRIX(p->currentMatrix); 
}

void fw_glScalef (float x, float y, float z) {
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

//      printf ("glScalef(%5.4f %5.4f %5.4f)\n",x,y,z);

        p->currentMatrix[0] *= x;   p->currentMatrix[4] *= y;   p->currentMatrix[8]  *= z;
        p->currentMatrix[1] *= x;   p->currentMatrix[5] *= y;   p->currentMatrix[9]  *= z;
        p->currentMatrix[2] *= x;   p->currentMatrix[6] *= y;   p->currentMatrix[10] *= z;
        p->currentMatrix[3] *= x;   p->currentMatrix[7] *= y;   p->currentMatrix[11] *= z;

        FW_GL_LOADMATRIX(p->currentMatrix);
}


void fw_glGetDoublev (int ty, GLDOUBLE *mat) {
	GLDOUBLE *dp;
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

/*
	switch (ty) {
		case GL_PROJECTION_MATRIX: printf ("getDoublev(GL_PROJECTION_MATRIX)\n"); break;
		case GL_MODELVIEW_MATRIX: printf ("getDoublev(GL_MODELVIEW_MATRIX)\n"); break;
		case GL_TEXTURE_MATRIX: printf ("getDoublev(GL_TEXTURE_MATRIX)\n"); break;
	}
*/

	switch (ty) {
		case GL_PROJECTION_MATRIX: dp = p->FW_ProjectionView[p->projectionviewTOS]; break;
		case GL_MODELVIEW_MATRIX: dp = p->FW_ModelView[p->modelviewTOS]; break;
#ifndef GL_ES_VERSION_2_0
		case GL_TEXTURE_MATRIX: dp = p->FW_TextureView[p->textureviewTOS]; break;
#endif
		default: { 
			loadIdentityMatrix(mat); 
		printf ("invalid mode sent in it is %d, expected one of %d %d %d\n",ty,GL_PROJECTION_MATRIX,GL_MODELVIEW_MATRIX,GL_TEXTURE_MATRIX);
			return;}
	}
	memcpy((void *)mat, (void *) dp, sizeof (GLDOUBLE) * MATRIX_SIZE);
}


/* for Sarah's front end - should be removed sometime... */
void kill_rendering() {
/* printf ("kill_rendering called...\n"); */
}


/* if we have a ReplaceWorld style command, we have to remove the old world. */
/* NOTE: There are 2 kinds of of replaceworld commands - sometimes we have a URL
   (eg, from an Anchor) and sometimes we have a list of nodes (from a Javascript
   replaceWorld, for instance). The URL ones can really replaceWorld, the node
   ones, really, it's just replace the rootNode children, as WE DO NOT KNOW
   what the user has programmed, and what nodes are (re) used in the Scene Graph */

void kill_oldWorld(int kill_EAI, int kill_JavaScript, char *file, int line) {
	int i;
	#ifndef AQUA
        char mystring[20];
	#endif

#ifdef VERBOSE
	printf ("kill 1 myThread %u displayThread %u\n",pthread_self(), gglobal()->threads.DispThrd);
#ifdef _MSC_VER
	if (pthread_self().p != gglobal()->threads.DispThrd.p ) {
#else
	if (pthread_self() != gglobal()->threads.DispThrd) {
#endif
		ConsoleMessage ("kill_oldWorld must run in the displayThread called at %s:%d\n",file,line);
		return;
	}
#endif

	/* get rid of sensor events */
	resetSensorEvents();


	/* make the root_res equal NULL - this throws away all old resource info */
	/*
		if (gglobal()->resources.root_res != NULL) {
			printf ("root_res %p has the following...\n",gglobal()->resources.root_res);
			resource_dump(gglobal()->resources.root_res);
		}else {printf ("root_res is null, no need to dump\n");}
	*/

	gglobal()->resources.root_res = NULL;

	/* mark all rootNode children for Dispose */
	for (i=0; i<rootNode->children.n; i++) {
		markForDispose(rootNode->children.p[i], TRUE);
	}

	/* stop rendering */
	rootNode->children.n = 0;

	/* close the Console Message system, if required. */
	closeConsoleMessage();

	/* occlusion testing - zero total count, but keep MALLOC'd memory around */
	zeroOcclusion();

	/* clock events - stop them from ticking */
	kill_clockEvents();


	/* kill DEFS, handles */
	EAI_killBindables();
	kill_bindables();
	killKeySensorNodeList();


	/* stop routing */
	kill_routing();

	/* tell the statusbar that it needs to reinitialize */
	kill_status();

	/* free textures */
/*
	kill_openGLTextures();
*/
	
	/* free scripts */
	#ifdef HAVE_JAVASCRIPT
	kill_javascript();
	#endif


	/* free EAI */
	if (kill_EAI) {
	       	shutdown_EAI();
	}

	#ifndef AQUA
		sprintf (mystring, "QUIT");
		Sound_toserver(mystring);
	#endif


	/* reset any VRML Parser data */
	if (globalParser != NULL) {
		parser_destroyData(globalParser);
		globalParser = NULL;
	}


	/* tell statusbar that we have none */
	viewer_default();
	setMenuStatus("NONE");
}

/* for verifying that a memory pointer exists */
int checkNode(struct X3D_Node *node, char *fn, int line) {
	int tc;
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	if (node == NULL) {
		printf ("checkNode, node is NULL at %s %d\n",fn,line);
		return FALSE;
	}

	if (node == p->forgottenNode) return TRUE;


	LOCK_MEMORYTABLE
	for (tc = 0; tc< p->nextEntry; tc++)
		if (p->memoryTable[tc] == node) {
			if (node->referenceCount > 0) {
			UNLOCK_MEMORYTABLE
			return TRUE;
		}
	}


	printf ("checkNode: did not find %p in memory table at i%s %d\n",node,fn,line);

	UNLOCK_MEMORYTABLE
	return FALSE;
}


/*keep track of node created*/
void registerX3DNode(struct X3D_Node * tmp){
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	LOCK_MEMORYTABLE
	/* printf("nextEntry=%d	",nextEntry); printf("tableIndexSize=%d \n",tableIndexSize); */
	/*is table to small give us some leeway in threads */
	if (p->nextEntry >= (p->tableIndexSize-10)){
		/*is table exist*/	
		if (p->tableIndexSize <= INT_ID_UNDEFINED){
			createdMemoryTable();		
		} else {
			increaseMemoryTable();
		}
	}
	/*adding node in table*/	
	/* printf ("registerX3DNode, adding %x at %d\n",tmp,nextEntry); */
	p->memoryTable[p->nextEntry] = tmp;
	p->nextEntry+=1;
	UNLOCK_MEMORYTABLE
}

/*We don't register the first node created for reload reason*/
void doNotRegisterThisNodeForDestroy(struct X3D_Node * nodePtr){
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	LOCK_MEMORYTABLE
	if(nodePtr==(p->memoryTable[p->nextEntry-1])){
		p->nextEntry-=1;
		p->forgottenNode = nodePtr;
	}	
	UNLOCK_MEMORYTABLE
}

/*creating node table*/
void createdMemoryTable(){
	int count;
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	p->tableIndexSize=50;
	p->memoryTable = MALLOC(struct X3D_Node **, p->tableIndexSize * sizeof(struct X3D_Node*));

	/* initialize this to a known state */
	for (count=0; count < p->tableIndexSize; count++) {
		p->memoryTable[count] = NULL;
	}
}

/*making table bigger*/
void increaseMemoryTable(){
	int count;
	int oldhigh;
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	oldhigh = p->tableIndexSize;

	
	p->tableIndexSize*=2;
	p->memoryTable = REALLOC (p->memoryTable, p->tableIndexSize * sizeof(struct X3D_Node*) );

	/* initialize this to a known state */
	for (count=oldhigh; count < p->tableIndexSize; count++) {
		p->memoryTable[count] = NULL;
	}
	/* printf("increasing memory table=%d\n",tableIndexSize); */
}


/* sort children - use bubble sort with early exit flag */
/* we use this for z buffer rendering; drawing scene in correct z-buffer order */
/* we ONLY sort if:
	1) the node has changed - this is the "needsCompiling" field;
	2) the number of children has changed - again, the "needsCompiling" flag should be set,
		but we enforce it;
	3) the first pass shows that nodes are out of order 
*/

static void sortChildren (int line, struct Multi_Node *ch, struct Multi_Node *sortedCh, int needsCompiling, int sortForDistance) {
	int i,j;
	int nc;
	int noswitch;
	struct X3D_Node *a, *b, *c;

	/* simple, inefficient bubble sort */
	/* this is a fast sort when nodes are already sorted;
	   may wish to go and "QuickSort" or so on, when nodes
	   move around a lot. (Bubblesort is bad when nodes
	   have to be totally reversed) */

	nc = ch->n;

	/* printf ("sortChildren line %d nc %d ",line,nc);
		if (sortForDistance) printf ("sortForDistance ");
		if (needsCompiling) printf ("needsCompiling ");
		printf ("\n");
	*/

	/* has this changed size? */
	if (ch->n != sortedCh->n) {
		FREE_IF_NZ(sortedCh->p);
		sortedCh->p = MALLOC (void *, sizeof (void *) * ch->n);
		needsCompiling = TRUE; /* force this change; should be
			set anyway */
	}

	/* copy the nodes over; we will sort the sorted list */

	if (needsCompiling) {
		memcpy (sortedCh->p,ch->p,sizeof (void *) * nc);
		sortedCh->n = nc;
	}

	#ifdef VERBOSE
	printf ("sortChildren start, %d, chptr %u\n",nc,ch);
	#endif

	/* do we care about rendering order? */
	if (!sortForDistance) return;
	if (nc < 2) return;

	for(i=0; i<nc; i++) {
		noswitch = TRUE;
		for (j=(nc-1); j>i; j--) {
			/* printf ("comparing %d %d\n",i,j); */
			a = X3D_NODE(sortedCh->p[j-1]);
			b = X3D_NODE(sortedCh->p[j]);

			/* check to see if a child is NULL - if so, skip it */
			if (a && b) {
				if (a->_dist > b->_dist) {
					/* printf ("sortChildren at %lf, have to switch %d %d dists %lf %lf\n",TickTime(),i,j, 
a->_dist, b->_dist); */ 
					c = a;
					sortedCh->p[j-1] = b;
					sortedCh->p[j] = c;
					noswitch = FALSE;

				}
			}	
		}
		/* did we have a clean run? */
		if (noswitch) {
			break;
		}
	}
	
	#ifdef VERBOSE
	printf ("sortChildren returning.\n");
	for(i=0; i<nc; i++) {
		b = sortedCh->p[i];
		if (b)
			printf ("child %d %u %f %s",i,b,b->_dist,stringNodeType(b->_nodeType));
		else
			printf ("no child %d", i);
		b = ch->p[i];
		printf (" unsorted %u\n",b);
	}
	#endif
}

/* zero the Visibility flag in all nodes */
void zeroVisibilityFlag(void) {
	struct X3D_Node* node;
	int i;
	int ocnum;
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	ocnum=-1;

	
	/* do not bother doing this if the inputThread is active. */
	if (fwl_isinputThreadParsing()) return;
 	LOCK_MEMORYTABLE

	/* do we have GL_ARB_occlusion_query, or are we still parsing Textures? */
	if ((gglobal()->Frustum.OccFailed) || fwl_isTextureParsing()) {
		/* if we have textures still loading, display all the nodes, so that the textures actually
		   get put into OpenGL-land. If we are not texture parsing... */
		/* no, we do not have GL_ARB_occlusion_query, just tell every node that it has visible children 
		   and hope that, sometime, the user gets a good computer graphics card */
		for (i=0; i<p->nextEntry; i++){		
			node = p->memoryTable[i];	
			if (node != NULL) {
				node->_renderFlags = node->_renderFlags | VF_hasVisibleChildren;
			}
		}	
	} else {
		/* we do... lets zero the hasVisibleChildren flag */
		for (i=0; i<p->nextEntry; i++){		
			node = p->memoryTable[i];		
			if (node != NULL) {
		
			#ifdef OCCLUSIONVERBOSE
			if (((node->_renderFlags) & VF_hasVisibleChildren) != 0) {
			printf ("%lf, zeroVisibility - %d is a %s, flags %x\n",TickTime(), i,stringNodeType(node->_nodeType), (node->_renderFlags) & VF_hasVisibleChildren); 
			}
			#endif

			node->_renderFlags = node->_renderFlags & (0xFFFF^VF_hasVisibleChildren);
			}
	
		}		
	}

	UNLOCK_MEMORYTABLE
}

/* go through the linear list of nodes, and do "special things" for special nodes, like
   Sensitive nodes, Viewpoint nodes, ... */

#define CMD(TTT,YYY) \
	/* printf ("nt %s change %d ichange %d\n",stringNodeType(X3D_NODE(YYY)->_nodeType),X3D_NODE(YYY)->_change, X3D_NODE(YYY)->_ichange); */ \
	if (NODE_NEEDS_COMPILING) compile_Metadata##TTT((struct X3D_Metadata##TTT *) YYY)

#define BEGIN_NODE(thistype) case NODE_##thistype:
#define END_NODE break;

#define SIBLING_SENSITIVE(thistype) \
			/* make Sensitive */ \
			if (((struct X3D_##thistype *)node)->enabled) { \
				nParents = ((struct X3D_##thistype *)node)->_nparents; \
				pp = (((struct X3D_##thistype *)node)->_parents); \
			}  

#define ANCHOR_SENSITIVE(thistype) \
			/* make THIS Sensitive - most nodes make the parents sensitive, Anchors have children...*/ \
			anchorPtr = (struct X3D_Anchor *)node;

#ifdef VIEWPOINT
#undef VIEWPOINT /* defined for the EAI,SAI, does not concern us uere */
#endif
#define VIEWPOINT(thistype) \
			setBindPtr = (uintptr_t *) ((uintptr_t)(node) + offsetof (struct X3D_##thistype, set_bind)); \
			if (*setBindPtr == 100) {setBindPtr = NULL; }/* already done */ 

#define CHILDREN_NODE(thistype) \
			addChildren = NULL; removeChildren = NULL; \
			offsetOfChildrenPtr = offsetof (struct X3D_##thistype, children); \
			if (((struct X3D_##thistype *)node)->addChildren.n > 0) { \
				addChildren = &((struct X3D_##thistype *)node)->addChildren; \
				childrenPtr = &((struct X3D_##thistype *)node)->children; \
			} \
			if (((struct X3D_##thistype *)node)->removeChildren.n > 0) { \
				removeChildren = &((struct X3D_##thistype *)node)->removeChildren; \
				childrenPtr = &((struct X3D_##thistype *)node)->children; \
			} 

#define CHILDREN_SWITCH_NODE(thistype) \
			addChildren = NULL; removeChildren = NULL; \
			offsetOfChildrenPtr = offsetof (struct X3D_##thistype, choice); \
			if (((struct X3D_##thistype *)node)->addChildren.n > 0) { \
				addChildren = &((struct X3D_##thistype *)node)->addChildren; \
				childrenPtr = &((struct X3D_##thistype *)node)->choice; \
			} \
			if (((struct X3D_##thistype *)node)->removeChildren.n > 0) { \
				removeChildren = &((struct X3D_##thistype *)node)->removeChildren; \
				childrenPtr = &((struct X3D_##thistype *)node)->choice; \
			} 

#define CHILDREN_LOD_NODE \
			addChildren = NULL; removeChildren = NULL; \
			offsetOfChildrenPtr = offsetof (struct X3D_LOD, children); \
			if (X3D_LODNODE(node)->addChildren.n > 0) { \
				addChildren = &X3D_LODNODE(node)->addChildren; \
				if (X3D_LODNODE(node)->__isX3D == 0) childrenPtr = &X3D_LODNODE(node)->level; \
				else childrenPtr = &X3D_LODNODE(node)->children; \
			} \
			if (X3D_LODNODE(node)->removeChildren.n > 0) { \
				removeChildren = &X3D_LODNODE(node)->removeChildren; \
				if (X3D_LODNODE(node)->__isX3D == 0) childrenPtr = &X3D_LODNODE(node)->level; \
				else childrenPtr = &X3D_LODNODE(node)->children; \
			}

#define EVIN_AND_FIELD_SAME(thisfield, thistype) \
			if ((((struct X3D_##thistype *)node)->set_##thisfield.n) > 0) { \
				((struct X3D_##thistype *)node)->thisfield.n = 0; \
				FREE_IF_NZ (((struct X3D_##thistype *)node)->thisfield.p); \
				((struct X3D_##thistype *)node)->thisfield.p = ((struct X3D_##thistype *)node)->set_##thisfield.p; \
				((struct X3D_##thistype *)node)->thisfield.n = ((struct X3D_##thistype *)node)->set_##thisfield.n; \
				((struct X3D_##thistype *)node)->set_##thisfield.n = 0; \
				((struct X3D_##thistype *)node)->set_##thisfield.p = NULL; \
			} 

/* just tell the parent (a grouping node) that there is a locally scoped light as a child */
/* do NOT send this up the scenegraph! */
#define LOCAL_LIGHT_PARENT_FLAG \
{ int i; \
	for (i = 0; i < node->_nparents; i++) { \
		struct X3D_Node *n = X3D_NODE(node->_parents[i]); \
		if( n != 0 ) n->_renderFlags = n->_renderFlags | VF_localLight; \
	} \
}



#define  CHECK_MATERIAL_TRANSPARENCY \
	if (((struct X3D_Material *)node)->transparency > 0.0001) { \
		/* printf ("node %d MATERIAL HAS TRANSPARENCY of %f \n", node, ((struct X3D_Material *)node)->transparency); */ \
		update_renderFlag(X3D_NODE(node),VF_Blend | VF_shouldSortChildren);\
		have_transparency = TRUE; \
	}
 
#define CHECK_IMAGETEXTURE_TRANSPARENCY \
	if (isTextureAlpha(((struct X3D_ImageTexture *)node)->__textureTableIndex)) { \
		/* printf ("node %d IMAGETEXTURE HAS TRANSPARENCY\n", node); */ \
		update_renderFlag(X3D_NODE(node),VF_Blend | VF_shouldSortChildren);\
		have_transparency = TRUE; \
	}

#define CHECK_PIXELTEXTURE_TRANSPARENCY \
	if (isTextureAlpha(((struct X3D_PixelTexture *)node)->__textureTableIndex)) { \
		/* printf ("node %d PIXELTEXTURE HAS TRANSPARENCY\n", node); */ \
		update_renderFlag(X3D_NODE(node),VF_Blend | VF_shouldSortChildren);\
		have_transparency = TRUE; \
	}

#define CHECK_MOVIETEXTURE_TRANSPARENCY \
	if (isTextureAlpha(((struct X3D_MovieTexture *)node)->__textureTableIndex)) { \
		/* printf ("node %d MOVIETEXTURE HAS TRANSPARENCY\n", node); */ \
		update_renderFlag(X3D_NODE(node),VF_Blend | VF_shouldSortChildren);\
		have_transparency = TRUE; \
	}


void startOfLoopNodeUpdates(void) {
	struct X3D_Node* node;
	struct X3D_Anchor* anchorPtr;
	void **pp;
	int nParents;
	int i,j;
	uintptr_t *setBindPtr;

	struct Multi_Node *addChildren;
	struct Multi_Node *removeChildren;
	struct Multi_Node *childrenPtr;
	size_t offsetOfChildrenPtr;

	/* process one inline per loop; do it outside of the lock/unlock memory table */
	struct Vector *loadInlines;
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	if (rootNode == NULL) return; /* nothing to do, and we have not really started yet */

	/* initialization */
	addChildren = NULL;
	removeChildren = NULL;
	childrenPtr = NULL;
	pp = NULL;
	loadInlines = NULL;
	offsetOfChildrenPtr = 0;

	/* assume that we do not have any sensitive nodes at all... */
	gglobal()->Mainloop.HaveSensitive = FALSE;
	have_transparency = FALSE;


	/* do not bother doing this if the inputparsing thread is active */
	if (fwl_isinputThreadParsing()) return;

	LOCK_MEMORYTABLE

	/* go through the node table, and zero any bits of interest */
	for (i=0; i<p->nextEntry; i++){		
		node = p->memoryTable[i];	
		if (node != NULL) {
			/* printf ("%d ref %d\n",i,node->referenceCount); */
			if (node->referenceCount == 0) {
				/* killNode(i); */
			} else {
				/* turn OFF these flags */
				node->_renderFlags = node->_renderFlags & (0xFFFF^VF_Sensitive);
				node->_renderFlags = node->_renderFlags & (0xFFFF^VF_Viewpoint);
				node->_renderFlags = node->_renderFlags & (0xFFFF^VF_localLight);
				node->_renderFlags = node->_renderFlags & (0xFFFF^VF_globalLight);
				node->_renderFlags = node->_renderFlags & (0xFFFF^VF_Blend);
			}
		}
	}
	/* turn OFF these flags */
	rootNode->_renderFlags = rootNode->_renderFlags & (0xFFFF^VF_Sensitive);
	rootNode->_renderFlags = rootNode->_renderFlags & (0xFFFF^VF_Viewpoint);
	rootNode->_renderFlags = rootNode->_renderFlags & (0xFFFF^VF_localLight);
	rootNode->_renderFlags = rootNode->_renderFlags & (0xFFFF^VF_globalLight);
	rootNode->_renderFlags = rootNode->_renderFlags & (0xFFFF^VF_Blend);

	/* sort the rootNode, if it is Not NULL */
	if (rootNode != NULL) {
		sortChildren (__LINE__,&rootNode->children, &rootNode->_sortedChildren,ROOTNODE_NEEDS_COMPILING,rootNode->_renderFlags & VF_shouldSortChildren);
		rootNode->_renderFlags=rootNode->_renderFlags & (0xFFFF^VF_shouldSortChildren);
	}

	/* go through the list of nodes, and "work" on any that need work */
	nParents = 0;
	setBindPtr = NULL;
	anchorPtr = NULL;

	for (i=0; i<p->nextEntry; i++){		
		node = p->memoryTable[i];	
		if (node != NULL) 
		if (node->referenceCount > 0) {
			switch (node->_nodeType) {
				BEGIN_NODE(Shape)
					/* send along a "look at me" flag if we are visible, or we should look again */
					if ((X3D_SHAPE(node)->__occludeCheckCount <=0) ||
							(X3D_SHAPE(node)->__visible)) {
						update_renderFlag (X3D_NODE(node),VF_hasVisibleChildren);
						/* printf ("shape occludecounter, pushing visiblechildren flags\n");  */

					}
					X3D_SHAPE(node)->__occludeCheckCount--;
					/* printf ("shape occludecounter %d\n",X3D_SHAPE(node)->__occludeCheckCount); */
				END_NODE

				/* Lights. DirectionalLights are "scope relative", PointLights and
				   SpotLights are transformed */

				BEGIN_NODE(DirectionalLight)
					if (X3D_DIRECTIONALLIGHT(node)->on) {
						if (X3D_DIRECTIONALLIGHT(node)->global) 
							update_renderFlag(node,VF_globalLight);
						else
							LOCAL_LIGHT_PARENT_FLAG
					}
				END_NODE
				BEGIN_NODE(SpotLight)
					if (X3D_SPOTLIGHT(node)->on) {
						if (X3D_SPOTLIGHT(node)->global) 
							update_renderFlag(node,VF_globalLight);
						else
							LOCAL_LIGHT_PARENT_FLAG
					}
				END_NODE
				BEGIN_NODE(PointLight)
					if (X3D_POINTLIGHT(node)->on) {
						if (X3D_POINTLIGHT(node)->global) 
							update_renderFlag(node,VF_globalLight);
						else
							LOCAL_LIGHT_PARENT_FLAG
					}
				END_NODE


				/* some nodes, like Extrusions, have "set_" fields same as normal internal fields,
				   eg, "set_spine" and "spine". Here we just copy the fields over, and remove the
				   "set_" fields. This works for MF* fields ONLY */
				BEGIN_NODE(IndexedLineSet)
					EVIN_AND_FIELD_SAME(colorIndex,IndexedLineSet)
					EVIN_AND_FIELD_SAME(coordIndex,IndexedLineSet)
				END_NODE

				BEGIN_NODE(IndexedTriangleFanSet)
					EVIN_AND_FIELD_SAME(index,IndexedTriangleFanSet)
				END_NODE
				BEGIN_NODE(IndexedTriangleSet)
					EVIN_AND_FIELD_SAME(index,IndexedTriangleSet)
				END_NODE
				BEGIN_NODE(IndexedTriangleStripSet)
					EVIN_AND_FIELD_SAME(index,IndexedTriangleStripSet)
				END_NODE
				BEGIN_NODE(GeoElevationGrid)
					EVIN_AND_FIELD_SAME(height,GeoElevationGrid)
				END_NODE
				BEGIN_NODE(ElevationGrid)
					EVIN_AND_FIELD_SAME(height,ElevationGrid)
				END_NODE
				BEGIN_NODE(Extrusion)
					EVIN_AND_FIELD_SAME(crossSection,Extrusion)
					EVIN_AND_FIELD_SAME(orientation,Extrusion)
					EVIN_AND_FIELD_SAME(scale,Extrusion)
					EVIN_AND_FIELD_SAME(spine,Extrusion)
				END_NODE
				BEGIN_NODE(IndexedFaceSet)
					EVIN_AND_FIELD_SAME(colorIndex,IndexedFaceSet)
					EVIN_AND_FIELD_SAME(coordIndex,IndexedFaceSet)
					EVIN_AND_FIELD_SAME(normalIndex,IndexedFaceSet)
					EVIN_AND_FIELD_SAME(texCoordIndex,IndexedFaceSet)
				END_NODE
/* GeoViewpoint works differently than other nodes - see compile_GeoViewpoint for manipulation of these fields
				BEGIN_NODE(GeoViewpoint)
					EVIN_AND_FIELD_SAME(orientation,GeoViewpoint) 
					EVIN_AND_FIELD_SAME(position,GeoViewpoint)
				END_NODE
*/
	
				/* get ready to mark these nodes as Mouse Sensitive */
				BEGIN_NODE(PlaneSensor) SIBLING_SENSITIVE(PlaneSensor) END_NODE
				BEGIN_NODE(SphereSensor) SIBLING_SENSITIVE(SphereSensor) END_NODE
				BEGIN_NODE(CylinderSensor) SIBLING_SENSITIVE(CylinderSensor) END_NODE
				BEGIN_NODE(TouchSensor) SIBLING_SENSITIVE(TouchSensor) END_NODE
				BEGIN_NODE(GeoTouchSensor) SIBLING_SENSITIVE(GeoTouchSensor) END_NODE
	
				/* Anchor is Mouse Sensitive, AND has Children nodes */
				BEGIN_NODE(Anchor)
					sortChildren (__LINE__,&X3D_ANCHOR(node)->children,&X3D_ANCHOR(node)->_sortedChildren,NODE_NEEDS_COMPILING,node->_renderFlags & VF_shouldSortChildren);
					TURN_OFF_SHOULDSORTCHILDREN
					propagateExtent(X3D_NODE(node));
					ANCHOR_SENSITIVE(Anchor)
					CHILDREN_NODE(Anchor)
				END_NODE
				
				/* maybe this is the current Viewpoint? */
				BEGIN_NODE(Viewpoint) VIEWPOINT(Viewpoint) END_NODE
				BEGIN_NODE(OrthoViewpoint) VIEWPOINT(OrthoViewpoint) END_NODE
				BEGIN_NODE(GeoViewpoint) VIEWPOINT(GeoViewpoint) END_NODE
	
				BEGIN_NODE(NavigationInfo) 
					render_NavigationInfo ((struct X3D_NavigationInfo *)node);
				END_NODE

				BEGIN_NODE(StaticGroup)
					/* we should probably not do this, but... */
					sortChildren (__LINE__,&X3D_STATICGROUP(node)->children,&X3D_STATICGROUP(node)->_sortedChildren,NODE_NEEDS_COMPILING,node->_renderFlags & VF_shouldSortChildren);
					TURN_OFF_SHOULDSORTCHILDREN
					propagateExtent(X3D_NODE(node));
				END_NODE


				/* does this one possibly have add/removeChildren? */
				BEGIN_NODE(Group) 
/*
printf ("Group %p, set_children.n %d children.n %d addChildren.n %d removeChildren.n %d\n",
node,
X3D_GROUP(node)->children.n,
X3D_GROUP(node)->addChildren.n,
X3D_GROUP(node)->removeChildren.n);
*/
					sortChildren (__LINE__,&X3D_GROUP(node)->children,&X3D_GROUP(node)->_sortedChildren,NODE_NEEDS_COMPILING,node->_renderFlags & VF_shouldSortChildren);
					TURN_OFF_SHOULDSORTCHILDREN

					propagateExtent(X3D_NODE(node));
					CHILDREN_NODE(Group) 
				END_NODE

#ifdef DJTRACK_PICKSENSORS
				/* DJTRACK_PICKSENSORS */
				BEGIN_NODE(PickableGroup) 
					sortChildren (__LINE__,&X3D_PICKABLEGROUP(node)->children,&X3D_PICKABLEGROUP(node)->_sortedChildren,NODE_NEEDS_COMPILING,node->_renderFlags & VF_shouldSortChildren);
					TURN_OFF_SHOULDSORTCHILDREN

					propagateExtent(X3D_NODE(node));
					CHILDREN_NODE(PickableGroup) 
				END_NODE
				/* PointPickSensor needs its own flag sent up the chain */
				BEGIN_NODE (PointPickSensor)
                			if (X3D_POINTPICKSENSOR(node)->enabled) update_renderFlag(node,VF_PickingSensor);
				END_NODE

#endif

				BEGIN_NODE(Inline) 
					if (X3D_INLINE(node)->__loadstatus != INLINE_STABLE) {
						/* schedule this after we have unlocked the memory table */
						if (loadInlines == NULL) {
							loadInlines = newVector(struct X3D_Inline*, 16);
						}
						vector_pushBack(struct X3D_Inline *, loadInlines, X3D_INLINE(node));
					}
					sortChildren (__LINE__,&X3D_INLINE(node)->__children,&X3D_INLINE(node)->_sortedChildren,NODE_NEEDS_COMPILING,node->_renderFlags & VF_shouldSortChildren);
					TURN_OFF_SHOULDSORTCHILDREN
					propagateExtent(X3D_NODE(node));
				END_NODE

				BEGIN_NODE(Transform) 
					sortChildren (__LINE__,&X3D_TRANSFORM(node)->children,&X3D_TRANSFORM(node)->_sortedChildren,NODE_NEEDS_COMPILING,node->_renderFlags & VF_shouldSortChildren);
					TURN_OFF_SHOULDSORTCHILDREN
					propagateExtent(X3D_NODE(node));
					CHILDREN_NODE(Transform) 
				END_NODE

				BEGIN_NODE(NurbsGroup) 
					CHILDREN_NODE(NurbsGroup) 
				END_NODE

				BEGIN_NODE(Contour2D) 
					CHILDREN_NODE(Contour2D) 
				END_NODE

				BEGIN_NODE(HAnimSite) 
					CHILDREN_NODE(HAnimSite) 
				END_NODE

				BEGIN_NODE(HAnimSegment) 
					CHILDREN_NODE(HAnimSegment) 
				END_NODE

				BEGIN_NODE(HAnimJoint) 
					CHILDREN_NODE(HAnimJoint) 
				END_NODE

				BEGIN_NODE(Billboard) 
					sortChildren (__LINE__,&X3D_BILLBOARD(node)->children,&X3D_BILLBOARD(node)->_sortedChildren,NODE_NEEDS_COMPILING,node->_renderFlags & VF_shouldSortChildren);
					TURN_OFF_SHOULDSORTCHILDREN
					propagateExtent(X3D_NODE(node));
					CHILDREN_NODE(Billboard) 
                			update_renderFlag(node,VF_Proximity);
				END_NODE

				BEGIN_NODE(Collision) 
					sortChildren (__LINE__,&X3D_COLLISION(node)->children,&X3D_COLLISION(node)->_sortedChildren,NODE_NEEDS_COMPILING,node->_renderFlags & VF_shouldSortChildren);
					TURN_OFF_SHOULDSORTCHILDREN
					propagateExtent(X3D_NODE(node));
					CHILDREN_NODE(Collision) 
				END_NODE

				BEGIN_NODE(Switch) 
					propagateExtent(X3D_NODE(node));
					CHILDREN_SWITCH_NODE(Switch) 
				END_NODE

				BEGIN_NODE(LOD) 
					propagateExtent(X3D_NODE(node));
					CHILDREN_LOD_NODE 
                			update_renderFlag(node,VF_Proximity);
				END_NODE

				/* Material - transparency of materials */
				BEGIN_NODE(Material) CHECK_MATERIAL_TRANSPARENCY END_NODE

				/* Textures - check transparency  */
				BEGIN_NODE(ImageTexture) CHECK_IMAGETEXTURE_TRANSPARENCY END_NODE
				BEGIN_NODE(PixelTexture) CHECK_PIXELTEXTURE_TRANSPARENCY END_NODE
				BEGIN_NODE(MovieTexture) CHECK_MOVIETEXTURE_TRANSPARENCY END_NODE


				/* Backgrounds, Fog */
				BEGIN_NODE(Background)
					if (X3D_BACKGROUND(node)->isBound) update_renderFlag (X3D_NODE(node),VF_hasVisibleChildren);
				END_NODE

				BEGIN_NODE(TextureBackground)
					if (X3D_TEXTUREBACKGROUND(node)->isBound) update_renderFlag (X3D_NODE(node),VF_hasVisibleChildren);
				END_NODE

				BEGIN_NODE(Fog)
					if (X3D_FOG(node)->isBound) update_renderFlag (X3D_NODE(node),VF_hasVisibleChildren);
				END_NODE


				/* VisibilitySensor needs its own flag sent up the chain */
				BEGIN_NODE (VisibilitySensor)
					/* send along a "look at me" flag if we are visible, or we should look again */
					if ((X3D_VISIBILITYSENSOR(node)->__occludeCheckCount <=0) ||
							(X3D_VISIBILITYSENSOR(node)->__visible)) {
						update_renderFlag (X3D_NODE(node),VF_hasVisibleChildren);
						/* printf ("vis occludecounter, pushing visiblechildren flags\n"); */

					}
					X3D_VISIBILITYSENSOR(node)->__occludeCheckCount--;
					/* VisibilitySensors have a transparent bounding box we have to render */

                			update_renderFlag(node,VF_Blend & VF_shouldSortChildren);
				END_NODE

				/* ProximitySensor needs its own flag sent up the chain */
				BEGIN_NODE (ProximitySensor)
                			if (X3D_PROXIMITYSENSOR(node)->enabled) update_renderFlag(node,VF_Proximity);
				END_NODE

				/* GeoProximitySensor needs its own flag sent up the chain */
				BEGIN_NODE (GeoProximitySensor)
                			if (X3D_GEOPROXIMITYSENSOR(node)->enabled) update_renderFlag(node,VF_Proximity);
				END_NODE

				/* GeoLOD needs its own flag sent up the chain, and it has to push extent up, too */
				BEGIN_NODE (GeoLOD)
					if (!(NODE_NEEDS_COMPILING)) {
						handle_GeoLODRange(X3D_GEOLOD(node));
					}
                			/* update_renderFlag(node,VF_Proximity); */
					propagateExtent(X3D_NODE(node));
				END_NODE

				BEGIN_NODE (GeoTransform)
					sortChildren (__LINE__,&X3D_GEOTRANSFORM(node)->children,&X3D_GEOTRANSFORM(node)->_sortedChildren,NODE_NEEDS_COMPILING,node->_renderFlags & VF_shouldSortChildren);
					TURN_OFF_SHOULDSORTCHILDREN
					propagateExtent(X3D_NODE(node));
					CHILDREN_NODE(GeoTransform) 
				END_NODE

				BEGIN_NODE (GeoLocation)
					sortChildren (__LINE__,&X3D_GEOLOCATION(node)->children,&X3D_GEOLOCATION(node)->_sortedChildren,NODE_NEEDS_COMPILING,node->_renderFlags & VF_shouldSortChildren);
					TURN_OFF_SHOULDSORTCHILDREN
					propagateExtent(X3D_NODE(node));
					CHILDREN_NODE(GeoLocation) 
				END_NODE

				BEGIN_NODE(MetadataSFBool) CMD(SFBool,node); END_NODE
				BEGIN_NODE(MetadataSFFloat) CMD(SFFloat,node); END_NODE
				BEGIN_NODE(MetadataMFFloat) CMD(MFFloat,node); END_NODE
				BEGIN_NODE(MetadataSFRotation) CMD(SFRotation,node); END_NODE
				BEGIN_NODE(MetadataMFRotation) CMD(MFRotation,node); END_NODE
				BEGIN_NODE(MetadataSFVec3f) CMD(SFVec3f,node); END_NODE
				BEGIN_NODE(MetadataMFVec3f) CMD(MFVec3f,node); END_NODE
				BEGIN_NODE(MetadataMFBool) CMD(MFBool,node); END_NODE
				BEGIN_NODE(MetadataSFInt32) CMD(SFInt32,node); END_NODE
				BEGIN_NODE(MetadataMFInt32) CMD(MFInt32,node); END_NODE
				BEGIN_NODE(MetadataSFNode) CMD(SFNode,node); END_NODE
				BEGIN_NODE(MetadataMFNode) CMD(MFNode,node); END_NODE
				BEGIN_NODE(MetadataSFColor) CMD(SFColor,node); END_NODE
				BEGIN_NODE(MetadataMFColor) CMD(MFColor,node); END_NODE
				BEGIN_NODE(MetadataSFColorRGBA) CMD(SFColorRGBA,node); END_NODE
				BEGIN_NODE(MetadataMFColorRGBA) CMD(MFColorRGBA,node); END_NODE
				BEGIN_NODE(MetadataSFTime) CMD(SFTime,node); END_NODE
				BEGIN_NODE(MetadataMFTime) CMD(MFTime,node); END_NODE
				BEGIN_NODE(MetadataSFString) CMD(SFString,node); END_NODE
				BEGIN_NODE(MetadataMFString) CMD(MFString,node); END_NODE
				BEGIN_NODE(MetadataSFVec2f) CMD(SFVec2f,node); END_NODE
				BEGIN_NODE(MetadataMFVec2f) CMD(MFVec2f,node); END_NODE
				BEGIN_NODE(MetadataSFImage) CMD(SFImage,node); END_NODE
				BEGIN_NODE(MetadataSFVec3d) CMD(SFVec3d,node); END_NODE
				BEGIN_NODE(MetadataMFVec3d) CMD(MFVec3d,node); END_NODE
				BEGIN_NODE(MetadataSFDouble) CMD(SFDouble,node); END_NODE
				BEGIN_NODE(MetadataMFDouble) CMD(MFDouble,node); END_NODE
				BEGIN_NODE(MetadataSFMatrix3f) CMD(SFMatrix3f,node); END_NODE
				BEGIN_NODE(MetadataMFMatrix3f) CMD(MFMatrix3f,node); END_NODE
				BEGIN_NODE(MetadataSFMatrix3d) CMD(SFMatrix3d,node); END_NODE
				BEGIN_NODE(MetadataMFMatrix3d) CMD(MFMatrix3d,node); END_NODE
				BEGIN_NODE(MetadataSFMatrix4f) CMD(SFMatrix4f,node); END_NODE
				BEGIN_NODE(MetadataMFMatrix4f) CMD(MFMatrix4f,node); END_NODE
				BEGIN_NODE(MetadataSFMatrix4d) CMD(SFMatrix4d,node); END_NODE
				BEGIN_NODE(MetadataMFMatrix4d) CMD(MFMatrix4d,node); END_NODE
				BEGIN_NODE(MetadataSFVec2d) CMD(SFVec2d,node); END_NODE
				BEGIN_NODE(MetadataMFVec2d) CMD(MFVec2d,node); END_NODE
				BEGIN_NODE(MetadataSFVec4f) CMD(SFVec4f,node); END_NODE
				BEGIN_NODE(MetadataMFVec4f) CMD(MFVec4f,node); END_NODE
				BEGIN_NODE(MetadataSFVec4d) CMD(SFVec4d,node); END_NODE
				BEGIN_NODE(MetadataMFVec4d) CMD(MFVec4d,node); END_NODE

				/* VRML1 Separator node; we do a bare bones implementation; always assume there are 
					lights, geometry, and viewpoints here. */
				BEGIN_NODE(VRML1_Separator) 
					sortChildren (__LINE__,&VRML1_SEPARATOR(node)->VRML1children,&VRML1_SEPARATOR(node)->_sortedChildren,NODE_NEEDS_COMPILING,node->_renderFlags & VF_shouldSortChildren);
					TURN_OFF_SHOULDSORTCHILDREN
					propagateExtent(X3D_NODE(node));
					update_renderFlag(X3D_NODE(node),VF_localLight|VF_Viewpoint|VF_Geom|VF_hasVisibleChildren);
				END_NODE
			}
		}

		/* now, act on this node  for Sensitive nodes. here we tell the PARENTS that they are sensitive */
		if (nParents != 0) {
			for (j=0; j<nParents; j++) {
				struct X3D_Node *n = X3D_NODE(pp[j]);
				n->_renderFlags = n->_renderFlags  | VF_Sensitive;
			}

			/* tell mainloop that we have to do a sensitive pass now */
			gglobal()->Mainloop.HaveSensitive = TRUE;
			nParents = 0;
		}

		/* Anchor nodes are slightly different than sibling-sensitive nodes */
		if (anchorPtr != NULL) {
			anchorPtr->_renderFlags = anchorPtr->_renderFlags  | VF_Sensitive;

			/* tell mainloop that we have to do a sensitive pass now */
			gglobal()->Mainloop.HaveSensitive = TRUE;
			anchorPtr = NULL;
		}

		/* do BINDING of Viewpoint Nodes */
		if (setBindPtr != NULL) {
			/* check the set_bind eventin to see if it is TRUE or FALSE */
			if (*setBindPtr < 100) {
				/* up_vector is reset after a bind */
				//if (*setBindPtr==1) reset_upvector();
				bind_node ((void *)node, &viewpoint_tos,&viewpoint_stack[0]);

				//dug9 added July 24, 2009: when you bind, it should set the 
				//avatar to the newly bound viewpoint pose and forget any 
				// cumulative avatar navigation from the last viewpoint parent

				if (node->_nodeType==NODE_Viewpoint) {
					bind_Viewpoint((struct X3D_Viewpoint *)node); 
				} else if (node->_nodeType==NODE_OrthoViewpoint) {
					bind_OrthoViewpoint((struct X3D_OrthoViewpoint *) node);
				} else {
					bind_GeoViewpoint((struct X3D_GeoViewpoint *) node);
				}
			}
			setBindPtr = NULL;
		}

		/* this node possibly has to do add/remove children? */
		if (childrenPtr != NULL) {
			if (addChildren != NULL) {
				AddRemoveChildren(node,childrenPtr,(struct X3D_Node * *) addChildren->p,addChildren->n,1,__FILE__,__LINE__);
				addChildren->n=0;
			}
			if (removeChildren != NULL) {
				AddRemoveChildren(node,childrenPtr,(struct X3D_Node * *) removeChildren->p,removeChildren->n,2,__FILE__,__LINE__);
				removeChildren->n=0;
			}
			/* printf ("OpenGL, marking children changed\n"); */
			MARK_EVENT(node,offsetOfChildrenPtr);
			childrenPtr = NULL;
		}
	}



	UNLOCK_MEMORYTABLE

	/* do we have Inlines to load here, outside of the memorytable lock? */
	if (loadInlines != NULL) {
		indexT ind;

		for (ind=0; ind<vector_size(loadInlines); ind++) {
			struct X3D_Inline *node;
			node=vector_get(struct X3D_Inline*, loadInlines,ind);
			load_Inline (node);
		}
		deleteVector (struct X3D_Inline*, loadInlines);
	}

	/* now, we can go and tell the grouping nodes which ones are the lucky ones that contain the current Viewpoint node */
	if (viewpoint_stack[viewpoint_tos] != 0) {
		update_renderFlag(X3D_NODE(viewpoint_stack[viewpoint_tos]), VF_Viewpoint);
		calculateNearFarplanes(X3D_NODE(viewpoint_stack[viewpoint_tos]));
	} else {
		/* keep these at the defaults, if no viewpoint is present. */
		Viewer.nearPlane = DEFAULT_NEARPLANE;
		Viewer.farPlane = DEFAULT_FARPLANE;
		Viewer.backgroundPlane = DEFAULT_BACKGROUNDPLANE;
	}
}

void markForDispose(struct X3D_Node *node, int recursive){
	struct Multi_Node* MNode;

	#if USE_JS_EXPERIMENTAL_CODE
	struct X3D_Node sfnode;
	#endif

	int *fieldOffsetsPtr;
	char * fieldPtr;

	if (node==NULL) return;

	 
/*
	printf ("\nmarkingForDispose %u (%s) currently at %d\n",node,
		stringNodeType(node->_nodeType),node->referenceCount);
*/
	
	if (node->referenceCount > 0) node->referenceCount --;

	if (recursive) {

	/* cast a "const int" to an "int" */
	fieldOffsetsPtr = (int*) NODE_OFFSETS[node->_nodeType];
	/*go thru all field*/				
	while (*fieldOffsetsPtr != -1) {
		fieldPtr = offsetPointer_deref(char *, node,*(fieldOffsetsPtr+1));
		#ifdef VERBOSE
		printf ("looking at field %s type %s\n",FIELDNAMES[*fieldOffsetsPtr],FIELDTYPES[*(fieldOffsetsPtr+2)]); 
		#endif

		/* some fields we skip, as the pointers are duplicated, and we CAN NOT free both */
		if (*fieldOffsetsPtr == FIELDNAMES_setValue) 
			break; /* can be a duplicate SF/MFNode pointer */
	
		if (*fieldOffsetsPtr == FIELDNAMES_valueChanged) 
			break; /* can be a duplicate SF/MFNode pointer */
	
		if (*fieldOffsetsPtr == FIELDNAMES___lastParent) 
			break; /* can be a duplicate SFNode pointer - field only in NODE_TextureCoordinate */
	
		if (*fieldOffsetsPtr == FIELDNAMES__selected) 
			break; /* can be a duplicate SFNode pointer - field only in NODE_LOD and NODE_GeoLOD */

		if (*fieldOffsetsPtr == FIELDNAMES___oldChildren) 
			break; /* can be a duplicate SFNode pointer - field only in NODE_LOD and NODE_GeoLOD */

		if (*fieldOffsetsPtr == FIELDNAMES___oldKeyPtr) 
			break; /* used for seeing if interpolator values change */

		if (*fieldOffsetsPtr == FIELDNAMES___oldKeyValuePtr) 
			break; /* used for seeing if interpolator values change */

		/* GeoLOD nodes, the children field exports either the rootNode, or the list of child nodes */
		if (node->_nodeType == NODE_GeoLOD) {
			if (*fieldOffsetsPtr == FIELDNAMES_children) break;
		}
	
		/* nope, not a special field, lets just get rid of it as best we can */
		switch(*(fieldOffsetsPtr+2)){
			case FIELDTYPE_MFNode: {
				int i;
				struct X3D_Node *tp;
				MNode=(struct Multi_Node *)fieldPtr;
		/* printf (" ... field MFNode, %s type %s\n",FIELDNAMES[*fieldOffsetsPtr],FIELDTYPES[*(fieldOffsetsPtr+2)]); */

				for (i=0; i<MNode->n; i++) {
					tp = MNode->p[i];
					 
					if (tp!=NULL)
						markForDispose(tp,TRUE);
				}
				MNode->n=0;
				break;
				}	
#ifdef wrwewetwetwe
			case FIELDTYPE_SFNode: {
				struct X3D_Node *SNode;

				SNode = (struct X3D_Node *)*fieldPtr;
printf ("SFNode, field is %u...\n",SNode);
if (SNode != NULL)
printf ("SFNode, .... and it is of type %s\n",stringNodeType(SNode->_nodeType));

		printf (" ... field SFNode, %s type %s\n",FIELDNAMES[*fieldOffsetsPtr],FIELDTYPES[*(fieldOffsetsPtr+2)]); 
				printf ("marking this SFnode for dispose, %u\n",SNode); 
				markForDispose(SNode, TRUE);
				break;
				

			}	
#endif
			default:; /* do nothing - field not malloc'd */
		}
		fieldOffsetsPtr+=5;	
	}


	}
}


#if USE_JS_EXPERIMENTAL_CODE
/*delete node created*/
static void killNode (int index) {
	int j=0;
	int *fieldOffsetsPtr;
	char * fieldPtr;
	struct X3D_Node* structptr;
	struct Multi_Float* MFloat;
	struct Multi_Rotation* MRotation;
	struct Multi_Vec3f* MVec3f;
	struct Multi_Bool* Mbool;
	struct Multi_Int32* MInt32;
	struct Multi_Node* MNode;
	struct Multi_Color* MColor;
	struct Multi_ColorRGBA* MColorRGBA;
	struct Multi_Time* MTime;
	struct Multi_String* MString;
	struct Multi_Vec2f* MVec2f;
	uintptr_t * VPtr;
	struct Uni_String *MyS;

	structptr = memoryTable[index];		

	#ifdef VERBOSE
	printf("Node pointer	= %u entry %d of %d ",structptr,i,nextEntry);
	printf (" number of parents %d ", structptr->_nparents);
	printf("Node Type	= %s\n",stringNodeType(structptr->_nodeType));  
	#endif

	/* kill any parents that may exist. */
	FREE_IF_NZ (structptr->_parents);

	fieldOffsetsPtr = NODE_OFFSETS[structptr->_nodeType];
	/*go thru all field*/				
	while (*fieldOffsetsPtr != -1) {
		fieldPtr = offsetPointer_deref(char *, structptr,*(fieldOffsetsPtr+1));
		#ifdef VERBOSE
		printf ("looking at field %s type %s\n",FIELDNAMES[*fieldOffsetsPtr],FIELDTYPES[*(fieldOffsetsPtr+2)]); 
		#endif

		/* some fields we skip, as the pointers are duplicated, and we CAN NOT free both */
		if (*fieldOffsetsPtr == FIELDNAMES_setValue) 
			break; /* can be a duplicate SF/MFNode pointer */
	
		if (*fieldOffsetsPtr == FIELDNAMES_valueChanged) 
			break; /* can be a duplicate SF/MFNode pointer */
	

		if (*fieldOffsetsPtr == FIELDNAMES___oldmetadata) 
			break; /* can be a duplicate SFNode pointer */
	
		if (*fieldOffsetsPtr == FIELDNAMES___lastParent) 
			break; /* can be a duplicate SFNode pointer - field only in NODE_TextureCoordinate */
	
		if (*fieldOffsetsPtr == FIELDNAMES__selected) 
			break; /* can be a duplicate SFNode pointer - field only in NODE_LOD and NODE_GeoLOD */

		if (*fieldOffsetsPtr == FIELDNAMES___oldChildren) 
			break; /* can be a duplicate SFNode pointer - field only in NODE_LOD and NODE_GeoLOD */

		if (*fieldOffsetsPtr == FIELDNAMES___oldMFString) 
			break; 

		if (*fieldOffsetsPtr == FIELDNAMES___scriptObj) 
			break; 

		if (*fieldOffsetsPtr == FIELDNAMES___oldSFString) 
			break; 

		if (*fieldOffsetsPtr == FIELDNAMES___oldKeyPtr) 
			break; /* used for seeing if interpolator values change */

		if (*fieldOffsetsPtr == FIELDNAMES___oldKeyValuePtr) 
			break; /* used for seeing if interpolator values change */

		if (*fieldOffsetsPtr == FIELDNAMES___shaderIDS) {
			struct X3D_ComposedShader *cps = (struct X3D_ComposedShader *) structptr;
			if ((cps->_nodeType == NODE_ComposedShader) || (cps->_nodeType == NODE_ProgramShader)) {
#ifdef GL_VERSION_2_0
				if (cps->__shaderIDS.p != NULL) {
					DELETE_PROGRAM((GLuint) cps->__shaderIDS.p[0]);
					FREE_IF_NZ(cps->__shaderIDS.p);
					cps->__shaderIDS.n=0;
				}
#endif

			} else {
				ConsoleMessage ("error destroying shaderIDS on kill");
			}
		}

		/* GeoLOD nodes, the children field exports either the rootNode, or the list of child nodes */
		if (structptr->_nodeType == NODE_GeoLOD) {
			if (*fieldOffsetsPtr == FIELDNAMES_children) break;
		}
	
		/* nope, not a special field, lets just get rid of it as best we can */
		switch(*(fieldOffsetsPtr+2)){
			case FIELDTYPE_MFFloat:
				MFloat=(struct Multi_Float *)fieldPtr;
				MFloat->n=0;
				FREE_IF_NZ(MFloat->p);
				break;
			case FIELDTYPE_MFRotation:
				MRotation=(struct Multi_Rotation *)fieldPtr;
				MRotation->n=0;
				FREE_IF_NZ(MRotation->p);
				break;
			case FIELDTYPE_MFVec3f:
				MVec3f=(struct Multi_Vec3f *)fieldPtr;
				MVec3f->n=0;
				FREE_IF_NZ(MVec3f->p);
				break;
			case FIELDTYPE_MFBool:
				Mbool=(struct Multi_Bool *)fieldPtr;
				Mbool->n=0;
				FREE_IF_NZ(Mbool->p);
				break;
			case FIELDTYPE_MFInt32:
				MInt32=(struct Multi_Int32 *)fieldPtr;
				MInt32->n=0;
				FREE_IF_NZ(MInt32->p);
				break;
			case FIELDTYPE_MFNode:
				MNode=(struct Multi_Node *)fieldPtr;
				#ifdef VERBOSE
				/* verify node structure. Each child should point back to me. */
				{
					int i;
					struct X3D_Node *tp;
					for (i=0; i<MNode->n; i++) {
						tp = MNode->p[i];
						printf ("	MNode field has child %u\n",tp);
						if (tp!=NULL)
						printf ("	ct %s\n",stringNodeType(tp->_nodeType));
					}
				}	
				#endif
				MNode->n=0;
				FREE_IF_NZ(MNode->p);
				break;

			case FIELDTYPE_MFColor:
				MColor=(struct Multi_Color *)fieldPtr;
				MColor->n=0;
				FREE_IF_NZ(MColor->p);
				break;
			case FIELDTYPE_MFColorRGBA:
				MColorRGBA=(struct Multi_ColorRGBA *)fieldPtr;
				MColorRGBA->n=0;
				FREE_IF_NZ(MColorRGBA->p);
				break;
			case FIELDTYPE_MFTime:
				MTime=(struct Multi_Time *)fieldPtr;
				MTime->n=0;
				FREE_IF_NZ(MTime->p);
				break;
			case FIELDTYPE_MFString: 
				MString=(struct Multi_String *)fieldPtr;
				{
				struct Uni_String* ustr;
				for (j=0; j<MString->n; j++) {
					ustr=MString->p[j];
					if (ustr != NULL) {
					ustr->len=0;
					ustr->touched=0;
					FREE_IF_NZ(ustr->strptr);
					}
				}
				MString->n=0;
				FREE_IF_NZ(MString->p);
				}
				break;
			case FIELDTYPE_MFVec2f:
				MVec2f=(struct Multi_Vec2f *)fieldPtr;
				MVec2f->n=0;
				FREE_IF_NZ(MVec2f->p);
				break;
			case FIELDTYPE_FreeWRLPTR:
				VPtr = (uintptr_t *) fieldPtr;
				VPtr = (uintptr_t *) (*VPtr);
				FREE_IF_NZ(VPtr);
				break;
			case FIELDTYPE_SFString:
				VPtr = (uintptr_t *) fieldPtr;
				MyS = (struct Uni_String *) *VPtr;
				MyS->len = 0;
				FREE_IF_NZ(MyS->strptr);
				FREE_IF_NZ(MyS);
				break;
				
			default:; /* do nothing - field not malloc'd */
		}
		fieldOffsetsPtr+=5;	
	}
	FREE_IF_NZ(memoryTable[index]);
	memoryTable[index]=NULL;
}
#endif


#ifdef DEBUG_FW_LOADMAT
	static void fw_glLoadMatrixd(GLDOUBLE *val,char *where, int line) {
	{int i;
	 for (i=0; i<16; i++) {
		if (val[i] > 2000.0) printf ("FW_GL_LOADMATRIX, val %d %lf at %s:%d\n",i,val[i],where,line);
		if (val[i] < -2000.0) printf ("FW_GL_LOADMATRIX, val %d %lf at %s:%d\n",i,val[i],where,line);
	}
	}
#else
	static void fw_glLoadMatrixd(GLDOUBLE *val) {
#endif

	/* printf ("loading matrix...\n"); */
	#ifndef GL_ES_VERSION_2_0
	glLoadMatrixd(val);
	#endif
}

void sendMatriciesToShader(s_shader_capabilities_t *me) {
	float spval[16];
	int i;
	float *sp; 
	GLDOUBLE *dp;
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	/* ModelView first */
	dp = p->FW_ModelView[p->modelviewTOS];
	sp = spval;

	/* convert GLDOUBLE to float */
	for (i=0; i<16; i++) {
		*sp = (float) *dp; 	
		sp ++; dp ++;
	}
	GLUNIFORMMATRIX4FV(me->ModelViewMatrix,1,GL_FALSE,spval);

	/* ProjectionMatrix */
	sp = spval;
	dp = p->FW_ProjectionView[p->projectionviewTOS];

	/* convert GLDOUBLE to float */
	for (i=0; i<16; i++) {
		*sp = (float) *dp; 	
		sp ++; dp ++;
	}
	GLUNIFORMMATRIX4FV(me->ProjectionMatrix,1,GL_FALSE,spval);

	/* send in the NormalMatrix */
	/* Uniform mat3  gl_NormalMatrix;  transpose of the inverse of the upper
                               		  leftmost 3x3 of gl_ModelViewMatrix */
	if (me->NormalMatrix != -1) {
		GLDOUBLE inverseMV[16];
		GLDOUBLE transInverseMV[16];
		GLDOUBLE MV[16];
		float normMat[9];
		dp = p->FW_ModelView[p->modelviewTOS];
		memcpy(MV,dp,sizeof(GLDOUBLE)*16);

		matinverse (inverseMV,MV);
		mattranspose(transInverseMV,inverseMV);
		/* get the 3x3 normal matrix from this guy */
		normMat[0] = (float) transInverseMV[0];
		normMat[1] = (float) transInverseMV[1];
		normMat[2] = (float) transInverseMV[2];
		
		normMat[3] = (float) transInverseMV[4];
		normMat[4] = (float) transInverseMV[5];
		normMat[5] = (float) transInverseMV[6];
		
		normMat[6] = (float) transInverseMV[8];
		normMat[7] = (float) transInverseMV[9];
		normMat[8] = (float) transInverseMV[10];

/* 
printf ("NormalMatrix: \n \t%4.3f %4.3f %4.3f\n \t%4.3f %4.3f %4.3f\n \t%4.3f %4.3f %4.3f\n",
normMat[0],normMat[1],normMat[2],
normMat[3],normMat[4],normMat[5],
normMat[6],normMat[7],normMat[8]);
*/

		GLUNIFORMMATRIX3FV(me->NormalMatrix,1,GL_FALSE,normMat);
	}

}
void sendMaterialsToShader(s_shader_capabilities_t *me) {
	/* go through all of the Uniforms for this shader */

#define SEND_VEC4(myMat,myVal) \
	if (me->myMat != -1) { GLUNIFORM4FV(me->myMat,1,myVal);}

#define SEND_FLOAT(myMat,myVal) \
	if (me->myMat != -1) { GLUNIFORM1F(me->myMat,myVal);}

/* eventually do this with code blocks in glsl */
	SEND_VEC4(myMaterialAmbient,appearanceProperties.fw_FrontMaterial.ambient);
	SEND_VEC4(myMaterialDiffuse,appearanceProperties.fw_FrontMaterial.diffuse);
	SEND_VEC4(myMaterialSpecular,appearanceProperties.fw_FrontMaterial.specular);
	SEND_VEC4(myMaterialEmission,appearanceProperties.fw_FrontMaterial.emission);
	SEND_FLOAT(myMaterialShininess,appearanceProperties.fw_FrontMaterial.shininess);

	SEND_VEC4(myMaterialBackAmbient,appearanceProperties.fw_BackMaterial.ambient);
	SEND_VEC4(myMaterialBackDiffuse,appearanceProperties.fw_BackMaterial.diffuse);
	SEND_VEC4(myMaterialBackSpecular,appearanceProperties.fw_BackMaterial.specular);
	SEND_VEC4(myMaterialBackEmission,appearanceProperties.fw_BackMaterial.emission);
	SEND_FLOAT(myMaterialBackShininess,appearanceProperties.fw_BackMaterial.shininess);

	if (me->lightState != -1) sendLightInfo(me);
}

static void __gluMultMatrixVecd(const GLDOUBLE matrix[16], const GLDOUBLE in[4],
                      GLDOUBLE out[4])
{
    int i;

    for (i=0; i<4; i++) {
        out[i] =
            in[0] * matrix[0*4+i] +
            in[1] * matrix[1*4+i] +
            in[2] * matrix[2*4+i] +
            in[3] * matrix[3*4+i];
    }
}


void fw_gluProject
(GLDOUBLE objx, GLDOUBLE objy, GLDOUBLE objz, 
	      const GLDOUBLE modelMatrix[16], 
	      const GLDOUBLE projMatrix[16],
              const GLint viewport[4],
	      GLDOUBLE *winx, GLDOUBLE *winy, GLDOUBLE *winz)
{
    GLDOUBLE in[4];
    GLDOUBLE out[4];

    in[0]=objx;
    in[1]=objy;
    in[2]=objz;
    in[3]=1.0;
    __gluMultMatrixVecd(modelMatrix, in, out);
    __gluMultMatrixVecd(projMatrix, out, in);
    if (in[3] == 0.0) return;
    in[0] /= in[3];
    in[1] /= in[3];
    in[2] /= in[3];
    /* Map x, y and z to range 0-1 */
    in[0] = in[0] * 0.5 + 0.5;
    in[1] = in[1] * 0.5 + 0.5;
    in[2] = in[2] * 0.5 + 0.5;

    /* Map x,y to viewport */
    in[0] = in[0] * viewport[2] + viewport[0];
    in[1] = in[1] * viewport[3] + viewport[1];

    *winx=in[0];
    *winy=in[1];
    *winz=in[2];
}

static void __gluMultMatricesd(const GLDOUBLE a[16], const GLDOUBLE b[16],
                                GLDOUBLE r[16])
{
    int i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            r[i*4+j] =
                a[i*4+0]*b[0*4+j] +
                a[i*4+1]*b[1*4+j] +
                a[i*4+2]*b[2*4+j] +
                a[i*4+3]*b[3*4+j];
        }
    }
}


/*
** Invert 4x4 matrix.
** Contributed by David Moore (See Mesa bug #6748)
*/
static int __gluInvertMatrixd(const GLDOUBLE m[16], GLDOUBLE invOut[16])
{
    GLDOUBLE inv[16], det;
    int i;

    inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
             + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
             - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
             + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
             - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
             - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
             + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
             - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
             + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
             + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
    inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
             - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
    inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
             + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
             - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
    inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
             - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
    inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
             + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
             - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
    inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
             + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

    det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if (det == 0)
        return GL_FALSE;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return GL_TRUE;
}



void fw_gluUnProject(GLDOUBLE winx, GLDOUBLE winy, GLDOUBLE winz,
		const GLDOUBLE modelMatrix[16], 
		const GLDOUBLE projMatrix[16],
                const GLint viewport[4],
	        GLDOUBLE *objx, GLDOUBLE *objy, GLDOUBLE *objz)
{
    GLDOUBLE finalMatrix[16];
    GLDOUBLE in[4];
    GLDOUBLE out[4];

    __gluMultMatricesd(modelMatrix, projMatrix, finalMatrix);
    if (!__gluInvertMatrixd(finalMatrix, finalMatrix)) return;

    in[0]=winx;
    in[1]=winy;
    in[2]=winz;
    in[3]=1.0;

    /* Map x and y from window coordinates */
    in[0] = (in[0] - viewport[0]) / viewport[2];
    in[1] = (in[1] - viewport[1]) / viewport[3];

    /* Map to range -1 to 1 */
    in[0] = in[0] * 2 - 1;
    in[1] = in[1] * 2 - 1;
    in[2] = in[2] * 2 - 1;

    __gluMultMatrixVecd(finalMatrix, in, out);
    if (out[3] == 0.0) return;
    out[0] /= out[3];
    out[1] /= out[3];
    out[2] /= out[3];
    *objx = out[0];
    *objy = out[1];
    *objz = out[2];
}


void fw_Ortho (GLDOUBLE left, GLDOUBLE right, GLDOUBLE bottom, GLDOUBLE top, GLDOUBLE nearZ, GLDOUBLE farZ) {
	GLDOUBLE *dp;
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;

	/* do the glOrtho on the top of the stack, and send that along */
	dp = p->FW_ProjectionView[p->projectionviewTOS];

	/* do some bounds checking here */
	if (right <= left) right = left+1.0;   /* resolve divide by zero possibility */
	if (top <= bottom) top= bottom+1.0;    /* resolve divide by zero possibility */
	if (farZ <= nearZ) farZ= nearZ + 2.0;  /* resolve divide by zero possibility */

	/* {int i; for (i=0; i<16;i++) { printf ("ModView before  %d: %4.3f \n",i,dp[i]); } } */
	mesa_Ortho(left,right,bottom,top,nearZ,farZ,dp);

	 /* {int i; for (i=0; i<16;i++) { printf ("ModView after   %d: %4.3f \n",i,dp[i]); } } */

	FW_GL_LOADMATRIX(dp);
}

void printmatrix2(GLDOUBLE* mat,char* description ) {
    int i,j;
    printf("mat %s {\n",description);
    for(i = 0; i< 4; i++) {
		printf("mat [%2d-%2d] = ",i*4,(i*4)+3);
		for(j=0;j<4;j++) 
			printf(" %f ",mat[(i*4)+j]);
			//printf("mat[%d] = %f%s;\n",i,mat[i],i==12 ? " +disp.x" : i==13? " +disp.y" : i==14? " +disp.z" : "");
		printf("\n");
    }
    printf("}\n");

}


/* gluPerspective replacement */
void fw_gluPerspective(GLDOUBLE fovy, GLDOUBLE aspect, GLDOUBLE zNear, GLDOUBLE zFar) {
	GLDOUBLE xmin, xmax, ymin, ymax;

	GLDOUBLE *dp;
	GLDOUBLE ndp[16];
	GLDOUBLE ndp2[16];
	ppOpenGL_Utils p = (ppOpenGL_Utils)gglobal()->OpenGL_Utils.prv;



	ymax = zNear * tan(fovy * M_PI / 360.0);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;

	/* do the glFrsutum on the top of the stack, and send that along */
	FW_GL_MATRIX_MODE(GL_PROJECTION);
	//FW_GL_LOAD_IDENTITY();
	dp = p->FW_ProjectionView[p->projectionviewTOS];

	mesa_Frustum(xmin, xmax, ymin, ymax, zNear, zFar, ndp);
	mattranspose(ndp2,ndp);

	//printmatrix2(ndp,"ndp");
	//printmatrix2(ndp2,"ndp2 = transpose(ndp)");
	//JAS printmatrix2(dp,"dp");

	matmultiply(ndp,ndp2,dp);

	//printmatrix2(ndp,"ndp = ndp2*dp");

	/* method = 1; */
	#define TRY_PERSPECTIVE_METHOD_1
	#ifdef TRY_PERSPECTIVE_METHOD_1
	  	FW_GL_LOADMATRIX(ndp);
		/* put the matrix back on our matrix stack */
		memcpy (p->FW_ProjectionView[p->projectionviewTOS],ndp,16*sizeof (GLDOUBLE));
	#endif


	#ifdef TRY_PERSPECTIVE_METHOD_2
/* testing... */
{
	GLDOUBLE m[16];
    GLDOUBLE sine, cotangent, deltaZ;
    GLDOUBLE radians = fovy / 2.0 * M_PI / 180.0;

    deltaZ = zFar - zNear;
    sine = sin(radians);
    if ((deltaZ == 0) || (sine == 0) || (aspect == 0)) {
        return;
    }
    cotangent = cos(radians) / sine;

	loadIdentityMatrix(m); //(&m);
    //__gluMakeIdentityd(&m[0][0]);
    m[0*4+0] = cotangent / aspect;
    m[1*4+1] = cotangent;
    m[2*4+2] = -(zFar + zNear) / deltaZ;
    m[2*4+3] = -1;
    m[3*4+2] = -2 * zNear * zFar / deltaZ;
    m[3*4+3] = 0;
	matmultiply(m,m,dp);
	if(method==2)
	  FW_GL_LOADMATRIX(m);

    //glMultMatrixd(&m[0][0]);
}
	#endif


	#ifdef TRY_PERSPECTIVE_METHOD_3
	{
	GLDOUBLE yyy[16];
//printf ("fw_gluPerspective, have...\n");

	if(method==3)
	  gluPerspective(fovy,aspect,zNear,zFar);
	FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX,yyy);
	//JAS printmatrix2(dp,"dp orig");
	//JAS printmatrix2(ndp,"ndp myPup");
	//JAS printmatrix2(yyy,"yyy gluP");
	//JAS printmatrix2(m,"m mesa");
	//for (i=0; i<16;i++) {printf ("%d orig: %5.2lf myPup: %5.2lf gluP: %5.2lf mesa %5.2lf\n",i,dp[i],
	//	ndp[i],yyy[i],m[i]); 
	//}
	}
	#endif

}



/* gluPickMatrix replacement */
void fw_gluPickMatrix(GLDOUBLE xx, GLDOUBLE yy, GLDOUBLE width, GLDOUBLE height, GLint *vp) {
	#ifdef VERBOSE
	printf ("PickMat %lf %lf %lf %lf %d %d %d %d\n",xx,yy,width,height,vp[0], vp[1],vp[2],vp[3]);
	#endif

	if ((width < 0.0) || (height < 0.0)) return;
	/* Translate and scale the picked region to the entire window */
	FW_GL_TRANSLATE_D((vp[2] - 2.0 * (xx - vp[0])) / width, (vp[3] - 2.0 * (yy - vp[1])) / height, 0.0);
	FW_GL_SCALE_D(vp[2] / width, vp[3] / height, 1.0);

}


/* glFrustum replacement - taken from the MESA source;

 * matrix.c
 *
 * Some useful matrix functions.
 *
 * Brian Paul
 * 10 Feb 2004
 */

/**
 * Build a glFrustum matrix.
 */

static void
mesa_Frustum(GLDOUBLE left, GLDOUBLE right, GLDOUBLE bottom, GLDOUBLE top, GLDOUBLE nearZ, GLDOUBLE farZ, GLDOUBLE *m)
{
 /* http://www.songho.ca/opengl/gl_projectionmatrix.html shows derivation*/
   GLDOUBLE x = (2.0*nearZ) / (right-left);
   GLDOUBLE y = (2.0*nearZ) / (top-bottom);
   GLDOUBLE a = (right+left) / (right-left);
   GLDOUBLE b = (top+bottom) / (top-bottom);
   GLDOUBLE c = -(farZ+nearZ) / ( farZ-nearZ);
   GLDOUBLE d = -(2.0F*farZ*nearZ) / (farZ-nearZ);

	/* printf ("mesa_Frustum (%lf, %lf, %lf, %lf, %lf, %lf)\n",left,right,bottom,top,nearZ, farZ); */
#define M(row,col)  m[col*4+row]
	m[0] = x;
	m[1] = 0.0;
	m[2] = a;
	m[3] = 0.0;

	m[4] = 0.0;
	m[5] = y;
	m[6] = b;
	m[7] = 0.0;

	m[8] = 0.0;
	m[9] = 0.0;
	m[10] = c;
	m[11] = d;

	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = -1.0;
	m[15] = 0.0;
/*
	
   M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
   M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
   M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
   M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M
*/
}

/**
 * Build a glOrtho marix.
 */
static void
mesa_Ortho(GLDOUBLE left, GLDOUBLE right, GLDOUBLE bottom, GLDOUBLE top, GLDOUBLE nearZ, GLDOUBLE farZ, GLDOUBLE *m)
{
#define M(row,col)  m[col*4+row]
   M(0,0) = 2.0F / (right-left);
   M(0,1) = 0.0F;
   M(0,2) = 0.0F;
   M(0,3) = -(right+left) / (right-left);

   M(1,0) = 0.0F;
   M(1,1) = 2.0F / (top-bottom);
   M(1,2) = 0.0F;
   M(1,3) = -(top+bottom) / (top-bottom);

   M(2,0) = 0.0F;
   M(2,1) = 0.0F;
   M(2,2) = -2.0F / (farZ-nearZ);
   M(2,3) = -(farZ+nearZ) / (farZ-nearZ);

   M(3,0) = 0.0F;
   M(3,1) = 0.0F;
   M(3,2) = 0.0F;
   M(3,3) = 1.0F;
#undef M
}
