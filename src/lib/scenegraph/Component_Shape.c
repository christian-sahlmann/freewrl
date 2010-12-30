/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Shape.c,v 1.61 2010/12/30 20:45:08 crc_canada Exp $

X3D Shape Component

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
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../opengl/Frustum.h"
#include "../opengl/Material.h"
#include "../opengl/Textures.h"
#include "../opengl/OpenGL_Utils.h"
#include "Component_ProgrammableShaders.h"
#include "Component_Shape.h"
#include "RenderFuncs.h"


static int     linePropertySet;  /* line properties -width, etc                  */

struct matpropstruct appearanceProperties;

/* this is for the FillProperties node */
static GLuint fillpropCurrentShader = 0;

/* pointer for a TextureTransform type of node */
struct X3D_Node *  this_textureTransform;  /* do we have some kind of textureTransform? */


/* for doing shader material properties */
static struct X3D_TwoSidedMaterial *material_twoSided;
static struct X3D_Material *material_oneSided;
 
void render_LineProperties (struct X3D_LineProperties *node) {
	GLint	factor;
	GLushort pat;

	if (node->applied) {
		linePropertySet=TRUE;
		if (node->linewidthScaleFactor > 1.0) {
			FW_GL_LINEWIDTH(node->linewidthScaleFactor);
			FW_GL_POINTSIZE(node->linewidthScaleFactor);
		}
			
		if (node->linetype > 0) {
			factor = 2;
			pat = 0xffff; /* can not support fancy line types - this is the default */
			switch (node->linetype) {
				case 2: pat = 0xff00; break; /* dashed */
				case 3: pat = 0x4040; break; /* dotted */
				case 4: pat = 0x04ff; break; /* dash dot */
				case 5: pat = 0x44fe; break; /* dash dot dot */
				case 6: pat = 0x0100; break; /* optional */
				case 7: pat = 0x0100; break; /* optional */
				case 10: pat = 0xaaaa; break; /* optional */
				case 11: pat = 0x0170; break; /* optional */
				case 12: pat = 0x0000; break; /* optional */
				case 13: pat = 0x0000; break; /* optional */
				default: {}
			}
			FW_GL_LINE_STIPPLE(factor,pat);
			FW_GL_ENABLE(GL_LINE_STIPPLE);
		}
	}
}


static int fpshaderloaded = FALSE; 
static GLint hatchColour;
static GLint hatchPercent;
static GLint filledBool;
static GLint hatchedBool;
static GLint algorithm;

void render_FillProperties (struct X3D_FillProperties *node) {
	GLfloat hatchX;
	GLfloat hatchY;
	GLint algor;
	GLint hatched;
	GLint filled;

	if (!fpshaderloaded) {
		const char *vs = "\
			/* \n\
			  Shader source from \n\
			  \"Introduction to the OpenGL Shading Language\" \n\
			  presentation by Randi Rost, 3DLabs (GLSLOverview2005.pdf) \n\
			*/ \n\
			 \n\
			uniform vec3 LightPosition; \n\
			uniform bool filled; \n\
			const float SpecularContribution = 0.3; \n\
			const float DiffuseContribution = 1.0 - SpecularContribution; \n\
			varying float LightIntensity; \n\
			varying vec2 MCposition; \n\
			void main(void) \n\
			{ \n\
			    vec3 ecPosition = vec3(fw_ModelViewMatrix * fw_Vertex); \n\
			    vec3 tnorm      = normalize(gl_NormalMatrix * fw_Normal); \n\
			    vec3 lightVec   = normalize(LightPosition - ecPosition); \n\
			    vec3 reflectVec = reflect(-lightVec, tnorm); \n\
			    vec3 viewVec    = normalize(-ecPosition); \n\
			    float diffuse   = max(dot(lightVec, tnorm), 0.0); \n\
			    float spec      = 0.0; \n\
			    if (diffuse > 0.0) \n\
			    { \n\
			        spec = max(dot(reflectVec, viewVec), 0.0); \n\
			        spec = pow(spec, 16.0); \n\
			    } \n\
			    LightIntensity = DiffuseContribution * diffuse + \n\
			                       SpecularContribution * spec; \n\
			    MCposition      = gl_Vertex.xy; \n\
			    gl_Position     = ftransform(); \n\
			    // Get the vertex colour\n\
			    if (filled) gl_FrontColor = gl_FrontMaterial.diffuse;\n\
			    else gl_FrontColor = vec4(0.0, 0.0, 0.0, 0.0); // make transparent \n\
			} \n\
		";

		const char *fs = "\
			/*  \n\
			  Shader source from  \n\
			  \"Introduction to the OpenGL Shading Language\"  \n\
			  presentation by Randi Rost, 3DLabs (GLSLOverview2005.pdf)  \n\
			*/  \n\
			  \n\
			// HatchSize - x and y - larger = less hatches on shape \n\
			const vec2  HatchSize= vec2(0.15, 0.15);  \n\
			  \n\
			uniform vec4 HatchColour;     //= (0.85,0.86,0.84);  \n\
			uniform bool hatched;\n\
			uniform bool filled; \n\
			uniform vec2  HatchPct;               //= (0.90, 0.85);  \n\
			uniform int algorithm;	\n\
			varying vec2  MCposition;  \n\
			varying float LightIntensity;  \n\
			  \n\
			void main(void)  \n\
			{  \n\
			    vec4 color;  \n\
			    vec2 position, useBrick;  \n\
			    vec4 fragCol = gl_Color; \n\
			  \n\
			    position = MCposition / HatchSize;  \n\
			  \n\
			    if (algorithm == 0) {// bricking \n\
			    	if (fract(position.y * 0.5) > 0.5)  \n\
			            position.x += 0.5;  \n\
			   } \n\
			  \n\
			    // algorithm 1, 2 = no futzing required here \n\
			    if (algorithm == 3) {// positive diagonals \n\
				    vec2 curpos = position; \n\
			            position.x -= curpos.y;  \n\
			   } \n\
			  \n\
			    if (algorithm == 4) {// negative diagonals \n\
				    vec2 curpos = position; \n\
			            position.x += curpos.y;  \n\
			   } \n\
			  \n\
			    if (algorithm == 6) {// diagonal crosshatch \n\
				vec2 curpos = position; \n\
				if (fract(position.y) > 0.5)  { \n\
        			    if (fract(position.x) < 0.5) position.x += curpos.y; \n\
        			    else position.x -= curpos.y; \n\
				} else { \n\
        			    if (fract(position.x) > 0.5) position.x += curpos.y; \n\
        			    else position.x -= curpos.y; \n\
				} \n\
			   } \n\
			  \n\
			    position = fract(position);  \n\
			  \n\
			    useBrick = step(position, HatchPct);  \n\
			  \n\
			    if (hatched) color = mix(HatchColour, fragCol, useBrick.x * useBrick.y);  \n\
			    else color = fragCol; \n\
			  \n\
			    color *= LightIntensity;  \n\
			    gl_FragColor = color;  \n\
				if (filled) gl_FragColor.a = 1.0; //JAS \n\
			}  \n\
		";

		GLuint v;
		GLuint f;
	
		#ifdef FILLVERBOSE
			printf ("creating shaders\n");
		#endif


		v = CREATE_SHADER(GL_VERTEX_SHADER);
		f = CREATE_SHADER(GL_FRAGMENT_SHADER);	
	
		#ifdef FILLVERBOSE
			printf ("assigning shader source\n");
		#endif


		SHADER_SOURCE(v, 1, &vs,NULL);
		SHADER_SOURCE(f, 1, &fs,NULL);
	
		#ifdef FILLVERBOSE
			printf ("compiling shaders\n");
		#endif


		COMPILE_SHADER(v);
		COMPILE_SHADER(f);
	

		#ifdef FILLVERBOSE
			printf ("creating program and attaching\n");
		#endif

		fillpropCurrentShader = CREATE_PROGRAM;
		
		ATTACH_SHADER(fillpropCurrentShader,v);
		ATTACH_SHADER(fillpropCurrentShader,f);
	
		#ifdef FILLVERBOSE
			printf ("linking program\n");
		#endif


		LINK_SHADER(fillpropCurrentShader);

		#ifdef FILLVERBOSE
			printf ("getting shader vars\n");
		#endif

		hatchColour = GET_UNIFORM(fillpropCurrentShader,"HatchColour");
		hatchPercent = GET_UNIFORM(fillpropCurrentShader,"HatchPct");
		filledBool = GET_UNIFORM(fillpropCurrentShader,"filled");
		hatchedBool = GET_UNIFORM(fillpropCurrentShader,"hatched");
		algorithm = GET_UNIFORM(fillpropCurrentShader,"algorithm");
		#ifdef FILLVERBOSE
			printf ("hatchColour %d hatchPercent %d filledbool %d hatchedbool %d algor %d\n",hatchColour,hatchPercent,filledBool,hatchedBool,algor);
		#endif


		fpshaderloaded = TRUE;
	}
	USE_SHADER(fillpropCurrentShader);


	hatchX = 0.80f; hatchY = 0.80f;
	algor = node->hatchStyle; filled = node->filled; hatched = node->hatched;
	switch (node->hatchStyle) {
		case 1: hatchX = 1.0f; break; /* horizontal lines */
		case 2: hatchY = 1.0f; break; /* vertical lines */
		case 3: hatchY=1.0f; break; /* positive sloped lines */
		case 4: hatchY=1.0f; break; /* negative sloped lines */
		case 5: break; /* square pattern */
		case 6: hatchY = 1.0f; break; /* diamond pattern */

		default :{
			node->hatched = FALSE;
		}
	}
	GLUNIFORM2F(hatchPercent,hatchX, hatchY);
	GLUNIFORM1I(filledBool,filled);
	GLUNIFORM1I(hatchedBool,hatched);
	GLUNIFORM1I(algorithm,algor);
	GLUNIFORM4F(hatchColour,node->hatchColor.c[0], node->hatchColor.c[1], node->hatchColor.c[2],1.0f);
}

void compile_TwoSidedMaterial (struct X3D_TwoSidedMaterial *node) {
	int i;

	/* verify that the numbers are within range */
	if (node->ambientIntensity < 0.0) node->ambientIntensity=0.0f;
	if (node->ambientIntensity > 1.0) node->ambientIntensity=1.0f;
	if (node->shininess < 0.0) node->shininess=0.0f;
	if (node->shininess > 1.0) node->shininess=1.0f;
	if (node->transparency < 0.0) node->transparency=MIN_NODE_TRANSPARENCY;
	if (node->transparency >= 1.0) node->transparency=MAX_NODE_TRANSPARENCY;

	if (node->backAmbientIntensity < 0.0) node->backAmbientIntensity=0.0f;
	if (node->backAmbientIntensity > 1.0) node->backAmbientIntensity=1.0f;
	if (node->backShininess < 0.0) node->backShininess=0.0f;
	if (node->backShininess > 1.0) node->backShininess=1.0f;
	if (node->backTransparency < 0.0) node->backTransparency=0.0f;
	if (node->backTransparency > 1.0) node->backTransparency=1.0f;

	for (i=0; i<3; i++) {
		if (node->diffuseColor.c[i] < 0.0) node->diffuseColor.c[i]=0.0f;
		if (node->diffuseColor.c[i] > 1.0) node->diffuseColor.c[i]=1.0f;
		if (node->emissiveColor.c[i] < 0.0) node->emissiveColor.c[i]=0.0f;
		if (node->emissiveColor.c[i] > 1.0) node->emissiveColor.c[i]=1.0f;
		if (node->specularColor.c[i] < 0.0) node->specularColor.c[i]=0.0f;
		if (node->specularColor.c[i] > 1.0) node->specularColor.c[i]=1.0f;

		if (node->backDiffuseColor.c[i] < 0.0) node->backDiffuseColor.c[i]=0.0f;
		if (node->backDiffuseColor.c[i] > 1.0) node->backDiffuseColor.c[i]=1.0f;
		if (node->backEmissiveColor.c[i] < 0.0) node->backEmissiveColor.c[i]=0.0f;
		if (node->backEmissiveColor.c[i] > 1.0) node->backEmissiveColor.c[i]=1.0f;
		if (node->backSpecularColor.c[i] < 0.0) node->backSpecularColor.c[i]=0.0f;
		if (node->backSpecularColor.c[i] > 1.0) node->backSpecularColor.c[i]=1.0f;
	}


	MARK_NODE_COMPILED
}

void render_TwoSidedMaterial (struct X3D_TwoSidedMaterial *node) {
	
	COMPILE_IF_REQUIRED

	/* record this node for OpenGL-ES and OpenGL-3.1 operation */
	material_twoSided = node;
}


/* bounds check the material node fields */
void compile_Material (struct X3D_Material *node) {
	int i;
	float trans;


	/* verify that the numbers are within range */
	if (node->ambientIntensity < 0.0f) node->ambientIntensity=0.0f;
	if (node->ambientIntensity > 1.0f) node->ambientIntensity=1.0f;
	if (node->shininess < 0.0f) node->shininess=0.0f;
	if (node->shininess > 1.0f) node->shininess=1.0f;
	if (node->transparency < 0.0f) node->transparency=MIN_NODE_TRANSPARENCY;
	if (node->transparency >= 1.0f) node->transparency=MAX_NODE_TRANSPARENCY;

	for (i=0; i<3; i++) {
		if (node->diffuseColor.c[i] < 0.0f) node->diffuseColor.c[i]=0.0f;
		if (node->diffuseColor.c[i] > 1.0f) node->diffuseColor.c[i]=1.0f;
		if (node->emissiveColor.c[i] < 0.0f) node->emissiveColor.c[i]=0.0f;
		if (node->emissiveColor.c[i] > 1.0f) node->emissiveColor.c[i]=1.0f;
		if (node->specularColor.c[i] < 0.0f) node->specularColor.c[i]=0.0f;
		if (node->specularColor.c[i] > 1.0f) node->specularColor.c[i]=1.0f;
	}

        /* set the transparency here for the material */
        trans = 1.0f - node->transparency;
                
        node->_dcol.c[3] = trans;
        node->_amb.c[3] = trans;
        node->_scol.c[3] = trans;
        node->_ecol.c[3] = trans;
                
	memcpy((void *)node->_dcol.c, node->diffuseColor.c, sizeof (float) * 3);
        /* for (i=0; i<3;i++){ node->_dcol[i] = node->diffuseColor.c[i]; } */

        for(i=0; i<3; i++) { node->_amb.c[i] = node->_dcol.c[i] * node->ambientIntensity; }

	/* for (i=0; i<3;i++){ node->_scol[i] = node->specularColor.c[i]; } */
	memcpy((void *)node->_scol.c, node->specularColor.c, sizeof (float) * 3);

	/* for (i=0; i<3;i++){ node->_ecol[i] = node->emissiveColor.c[i]; } */
	memcpy((void *)node->_ecol.c, node->emissiveColor.c, sizeof (float) * 3);

        node->_shin = node->shininess * 128.0f;

#define MAX_SHIN 128.0f
#define MIN_SHIN 0.01f
        if ((node->_shin > MAX_SHIN) || (node->_shin < MIN_SHIN)) {
                if (node->_shin>MAX_SHIN){node->_shin = MAX_SHIN;}else{node->_shin=MIN_SHIN;}
        }
#undef MAX_SHIN
#undef MIN_SHIN


	MARK_NODE_COMPILED
}


void render_Material (struct X3D_Material *node) {
	COMPILE_IF_REQUIRED

	/* record this node for OpenGL-ES and OpenGL-3.1 operation */
	material_oneSided = node;
}


#ifdef SHADERS_2011

/* Vertex Shaders */

static char *vertexShaderForSphereGeomShader = " \
varying vec3 Norm; \
varying vec4 Pos; \
attribute      vec4 fw_Vertex; \
attribute      vec3 fw_Normal; \
uniform        mat4 fw_ModelViewMatrix; \
uniform        mat4 fw_ProjectionMatrix; \
void main(void) { \
       Norm = vec3(fw_ModelViewMatrix * vec4(fw_Normal,0.0)); \
       Pos = fw_ModelViewMatrix * fw_Vertex;  \
  gl_Position = fw_Vertex; \
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
       /* Norm = normalize(gl_NormalMatrix * fw_Normal); */\
       Pos = fw_ModelViewMatrix * fw_Vertex;  \
       gl_Position = fw_ProjectionMatrix * fw_ModelViewMatrix * fw_Vertex; \
}";



/* Geometry Shaders */

static const char *passThroughGeomShader = " \
#version 120 \n \
#extension GL_EXT_geometry_shader4 : enable \n \
\n \ 
void main() { \n \
  for(int i = 0; i < gl_VerticesIn; ++i) { \n \
    gl_FrontColor = gl_FrontColorIn[i]; \n \
    gl_Position = gl_PositionIn[i]; \n \
    EmitVertex(); \n \
  } \n \
} \n \
";

static const char *sphereGeomShader = 
"#version 120\n " \
"#extension GL_EXT_gpu_shader4: enable\n " \
"#extension GL_EXT_geometry_shader4: enable\n " \
"\n " \
"uniform int Level;\n " \
"\n " \
"varying float LightIntensity;\n " \
"varying vec3 Norm; \n " \
"varying vec4 Pos; /* is this the same as gl_Position??  */ \n " \
"uniform		mat4 fw_ModelViewMatrix;" \
"uniform		mat4 fw_ProjectionMatrix;" \
"varying	vec3 fw_Normal; " \
"\n " \
"vec3 V0, V01, V02;\n " \
"\n " \
"void\n " \
"ProduceVertex( float s, float t )\n " \
"{\n " \
"	const vec3 lightPos = vec3( 0., 10., 0. );\n " \
"\n " \
"	vec3 v = V0 + s*V01 + t*V02;\n " \
"	v = normalize(v);\n " \
"	vec3 n = v;\n " \
"	/* calculate a gl_NormalMatrix replacement */ \n " \
"      /* gl_NormalMatrix is the inverse transpose of the modelview matrix, but as every matrix here needs to be transposed, we end up with {MODELVIEW_MATRIX, INVERSE}. */ \n " \
"	/* no inverse here... mat4 myNormalMatrix = transpose(inverse(fw_ModelViewMatrix)); */ \n " \
"	mat3 myNormalMatrix = transpose(mat3x3(fw_ModelViewMatrix)); \n " \
"	fw_Normal = normalize( myNormalMatrix * n );	\n " \
"\n " \
"	vec4 ECposition = fw_ModelViewMatrix * vec4( (1.0*v), 1. );\n " \
"	LightIntensity  = dot( normalize(lightPos - ECposition.xyz), fw_Normal );\n " \
"	LightIntensity = abs( LightIntensity );\n " \
"	LightIntensity *= 1.5;\n " \
"\n " \
"	gl_Position = fw_ProjectionMatrix * ECposition;\n " \
"	Pos = fw_ProjectionMatrix * ECposition;\n " \
"	EmitVertex();\n " \
"}\n " \
" \n " \
"\n " \
"\n " \
"void\n " \
"main()\n " \
"{ \n " \
"	V01 = ( gl_PositionIn[1] - gl_PositionIn[0] ).xyz;\n " \
"	V02 = ( gl_PositionIn[2] - gl_PositionIn[0] ).xyz;\n " \
"	V0  =   gl_PositionIn[0].xyz;\n " \
"\n " \
"	int numLayers = 4;  \n " \
"\n " \
"	float dt = 1. / float( numLayers );\n " \
"\n " \
"	float t_top = 1.;\n " \
"\n " \
"	for( int it = 0; it < numLayers; it++ )\n " \
"	{\n " \
"		float t_bot = t_top - dt;\n " \
"		float smax_top = 1. - t_top;\n " \
"		float smax_bot = 1. - t_bot;\n " \
"\n " \
"		int nums = it + 1;\n " \
"		float ds_top = smax_top / float( nums - 1 );\n " \
"		float ds_bot = smax_bot / float( nums );\n " \
"\n " \
"		float s_top = 0.;\n " \
"		float s_bot = 0.;\n " \
"\n " \
"		for( int is = 0; is < nums; is++ )\n " \
"		{\n " \
"			ProduceVertex( s_bot, t_bot );\n " \
"			ProduceVertex( s_top, t_top );\n " \
"			s_top += ds_top;\n " \
"			s_bot += ds_bot;\n " \
"		}\n " \
"\n " \
"		ProduceVertex( s_bot, t_bot );\n " \
"		EndPrimitive();\n " \
"\n " \
"		t_top = t_bot;\n " \
"		t_bot -= dt;\n " \
"	} \n " \
" }\n ";



/* Fragment Shaders */
static char *FS = 
"varying float LightIntensity; " \
"uniform vec4 Color; " \
"void main() { " \
"        gl_FragColor = vec4( 0.5, 0.5, 1.0, 1. ); " \
"        /* gl_FragColor = vec4( LightIntensity*Color.rgb, 1. ); */ " \
"        /* gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0); */ " \
"}";

static const char *phongFragmentShader = 
"#version 120 \n " \
"#extension GL_EXT_gpu_shader4: enable \n " \
" \n " \
"varying vec3 Norm; \n " \
"varying vec4 Pos; \n " \
" \n " \
"// use ADSLightModel here \n " \
" \n " \
"// Assumed context: \n " \
"// uniform variables gl_Lightsource[i] and gl_FrontMaterial are  \n " \
"// stubbed with constant values below. These would probably be \n " \
"// in parameters for the function if used in an application. \n " \
"// \n " \
"// variables myNormal and myPosition are passed in; in a vertex \n " \
"// shader these would be computed and used directly, while in a  \n " \
"// fragment shader these would be set as varying variables by \n " \
"// the associated vertex shader \n " \
"// \n " \
"// the ADS colour is returned from the function. \n " \
" \n " \
"vec3 ADSLightModel(in vec3 myNormal, in vec4 myPosition) { \n " \
"	// the group of variables below would normallye taken from gl_Lightsource[i] components \n " \
"	vec4 myLightDiffuse = vec4(1., 1., 1., 1.); \n " \
"	vec4 myLightAmbient = vec4(0.2, 0.2, 0.2, 0.2); \n " \
"	vec4 myLightSpecular = vec4(1., 1., 1., 1.); \n " \
"	vec4 myLightPosition = vec4 (1., 0.5, 0., 1.); \n " \
" \n " \
"	// the group of variables below would normally be taken from \n " \
"	// gl_FrontMaterial components \n " \
"	vec4 myMaterialDiffuse = vec4(1., .5, 0., 1.); \n " \
"	vec4 myMaterialAmbient = vec4(1., .5, 0., 1.); \n " \
"	vec4 myMaterialSpecular = vec4(0.6, 0.6, 0.6, 1.); \n " \
"	float myMaterialShininess = 80.; \n " \
" \n " \
"	// normal, light, view, and light reflection vectors \n " \
"	vec3 norm = normalize (myNormal); \n " \
"	vec3 lightv = normalize(myLightPosition.xyz-myPosition.xyz); \n " \
"	vec3 viewv = -normalize(myPosition.xyz); \n " \
"	vec3 refl = reflect (-lightv, norm); \n " \
" \n " \
"	// diffuse light computation \n " \
"	vec4 diffuse = max (0.0, dot(lightv, norm))*myMaterialDiffuse*myLightDiffuse; \n " \
" \n " \
"	// ambient light computation \n " \
"	vec4 ambient = myMaterialAmbient*myLightAmbient; \n " \
" \n " \
"	// Specular light computation \n " \
"	vec4 specular = vec4 (0.0, 0.0, 0.0, 1.0); \n " \
"	if (dot(lightv, viewv) > 0.0) { \n " \
"		specular = pow(max(0.0, dot(viewv, refl)), \n " \
"			myMaterialShininess)*myMaterialSpecular*myLightSpecular; \n " \
"	} \n " \
"	return clamp(vec3(ambient+diffuse+specular), 0.0, 1.0); \n " \
"} \n " \
"void main () { \n " \
"	gl_FragColor = vec4(ADSLightModel(Norm,Pos),1.); \n " \
"} \n " ;





static const char *XphongFragmentShader = 
"/*  phong fragment shader  */" \
" " \
"varying vec3 Norm;" \
"varying vec4 Pos;" \
"" \
"/*  use ADSLightModel here */" \
"" \
"/*  Assumed context: */" \
"/*  uniform variables gl_LightSource[i] and gl_FrontMaterial are  */" \
"/*  stubbed with constant values below. These would probably be */" \
"/*  in parameters for the function if used in an application. */" \
"/*  */" \
"/*  variables myNormal and myPosition are passed in; in a vertex */" \
"/*  shader these would be computed and used directly, while in a  */" \
"/*  fragment shader these would be set as varying variables by */" \
"/*  the associated vertex shader */" \
"/*  */" \
"/*  the ADS colour is returned from the function. */" \
"" \
"vec3 ADSLightModel(in vec3 myNormal, in vec4 myPosition) {" \
"	int i;" \
"" \
"/* 	 the group of variables below would normally be taken from */" \
"/* 	 gl_FrontMaterial components */" \
"	vec4 myMaterialDiffuse = vec4(1., .5, 0., 1.);" \
"	vec4 myMaterialAmbient = vec4(1., .5, 0., 1.);" \
"	vec4 myMaterialSpecular = vec4(0.6, 0.6, 0.6, 1.);" \
"	float myMaterialShininess = 80.;" \
"" \
"	vec3 norm = normalize (myNormal);" \
"	vec3 viewv = -normalize(myPosition.xyz);" \
"	vec4 ambient = vec4(0., 0., 0., 0.);" \
"	vec4 specular = vec4 (0.0, 0.0, 0.0, 1.0);" \
"	vec4 diffuse = vec4(0., 0., 0., 1.);" \
"" \
"	for (i=0; i<8; i++) {" \
"/* 		 the group of variables below would normallye taken from gl_LightSource[i] components */" \
"		vec4 myLightDiffuse = gl_LightSource[i].diffuse;" \
"		vec4 myLightAmbient = gl_LightSource[i].ambient;" \
"		vec4 myLightSpecular = gl_LightSource[i].specular;" \
"		vec4 myLightPosition = gl_LightSource[i].position;" \
"" \
"/* 		 normal, light, view, and light reflection vectors */" \
"		vec3 lightv = normalize(myLightPosition.xyz-myPosition.xyz);" \
"		vec3 refl = reflect (-lightv, norm);" \
"" \
"/* 		 diffuse light computation */" \
"		diffuse += max (0.0, dot(lightv, norm))*myMaterialDiffuse*myLightDiffuse;" \
"" \
"/* 		 ambient light computation */" \
"		ambient += myMaterialAmbient*myLightAmbient;" \
"" \
"/* 		 Specular light computation */" \
"		if (dot(lightv, viewv) > 0.0) {" \
"			specular += pow(max(0.0, dot(viewv, refl))," \
"				myMaterialShininess)*myMaterialSpecular*myLightSpecular;" \
"		}" \
"	}" \
"" \
"	return clamp(vec3(ambient+diffuse+specular), 0.0, 1.0);" \
"}" \
"void main () {" \
"	gl_FragColor = vec4(ADSLightModel(Norm,Pos),1.); " \
"}";


static void shaderErrorLog(GLuint myShader, char *type) {
        #ifdef GL_VERSION_2_0
#define MAX_INFO_LOG_SIZE 512
                GLchar infoLog[MAX_INFO_LOG_SIZE];
                glGetShaderInfoLog(myShader, MAX_INFO_LOG_SIZE, NULL, infoLog);
		if (strlen(infoLog) > 0)
                	ConsoleMessage ("problem with %s shader: %s",type, infoLog);
        #else
                ConsoleMessage ("Problem compiling shader");
        #endif
}


/* find info on the geometry of this shape */
static void getGeometryShader (struct X3D_Node *myGeom, GLuint *myShad) {
	struct X3D_Node *realNode;
	GLint success;

	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,myGeom,realNode);

	*myShad = 0; /* default condition */

	if (realNode == NULL) return;

	/* which shapes have an associated Geometry shader? */
	if (realNode->_nodeType == NODE_Sphere) {
		*myShad = CREATE_SHADER(GL_GEOMETRY_SHADER_EXT);
		printf ("using geom shader sphereGeomShader\n");
		SHADER_SOURCE (*myShad, 1,&sphereGeomShader,NULL);
	}

	if (*myShad != 0) {	
		COMPILE_SHADER(*myShad);
        	GET_SHADER_INFO(*myShad, COMPILE_STATUS, &success);
        	if (!success) {
			shaderErrorLog(*myShad,"GEOMETRY");
        	}
	}
}

/* find info on the appearance of this Shape and create a shader */
/* 
	The possible sequence of a properly constructed appearance field is:

	Shape.appearance -> Appearance
	
	Appearance.fillProperties 	-> FillProperties
	Appearance.lineProperties 	-> LineProperties
	Appearance.material 		-> Material
					-> TwoSidedMaterial	
	Appearance.shaders		-> ComposedShader
	Appearance.shaders		-> PackagedShader
	Appearance.shaders		-> ProgramShader

	Appearance.texture		-> Texture
	Appearance.texture		-> MultiTexture
	Appearance.textureTransform	->

*/


static void getAppearanceShaders (struct X3D_Node * myApp, GLuint *myVert, GLuint *myFrag, int haveGeomShader) {	
	struct X3D_Node *fillPNode;
	struct X3D_Node *linePNode;
	struct X3D_Appearance *realNode;
	struct X3D_Node *materialNode;
	struct X3D_Node *shaderNode;
	struct X3D_Node *textureNode;
	struct X3D_Node *textureTransformNode;
	GLint success;

	/* resolve PROTO, if this is a PROTO... */
	POSSIBLE_PROTO_EXPANSION(struct X3D_Appearance *,myApp,realNode);
	/* printf ("getAppearanceShader, node type %s\n",stringNodeType(realNode->_nodeType)); */

	if (realNode == NULL) return; /* no Appearance - that is ok */

	/* this should be an appearance... */
	if (realNode->_nodeType != NODE_Appearance) {
		ConsoleMessage ("Appearance node expected in Shape appearance field, got :%s:\n",
			stringNodeType(realNode->_nodeType));
		return;
	}

	/* get fields, ensure that we do not get confused by PROTO */
	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,realNode->fillProperties,fillPNode);
	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,realNode->lineProperties,linePNode);
	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,realNode->material,materialNode);
	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,realNode->texture,textureNode);
	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,realNode->textureTransform,textureTransformNode);

	/* warning messages, at least here until we get it all implemented */
	if (fillPNode != NULL) ConsoleMessage ("Shaders_2011: fillProperties not yet supported\n");
	if (linePNode != NULL) ConsoleMessage ("Shaders_2011: lineProperties not yet supported\n");
	if (materialNode != NULL) ConsoleMessage ("Shaders_2011: material field not yet supported\n");
	if (textureNode != NULL) ConsoleMessage ("Shaders_2011: texture field not yet supported\n");
	if (textureTransformNode != NULL) ConsoleMessage ("Shaders_2011: textureTransform field not yet supported\n");

	/* printf ("getAppearance, fill %p line %p material %p texture %p tt %p\n",
		fillPNode, linePNode, materialNode, textureNode, textureTransformNode); */
	
	*myVert = CREATE_SHADER(GL_VERTEX_SHADER);
	*myFrag = CREATE_SHADER(GL_FRAGMENT_SHADER);

	/* vertex shader */
	if (haveGeomShader) {
		printf ("using vertex shader vertexShaderForSphereGeomShader\n");
		SHADER_SOURCE(*myVert, 1,&vertexShaderForSphereGeomShader,NULL);
	} else {
		printf ("using vertex shader phongSimpleVertexShader\n");
		SHADER_SOURCE(*myVert, 1,&phongSimpleVertexShader,NULL);
	}
	COMPILE_SHADER(*myVert);
        GET_SHADER_INFO(*myVert, COMPILE_STATUS, &success);
        if (!success) {
		shaderErrorLog(*myVert,"VERTEX");
        }

	/* fragment shader */
printf ("using fragment shader phongFragmentShader\n");
	SHADER_SOURCE(*myFrag, 1,&phongFragmentShader,NULL);
	COMPILE_SHADER(*myFrag);
        GET_SHADER_INFO(*myFrag, COMPILE_STATUS, &success);
        if (!success) {
		shaderErrorLog(*myFrag,"FRAGMENT");
        }
}

#endif /* SHADERS_2011 */

/* this should be vectorized, and made global in the rdr_caps, but for now... */
s_shader_capabilities_t globalShaders[10];


void compile_Shape (struct X3D_Shape *node) {
	s_shader_capabilities_t *mySTE;
#ifdef SHADERS_2011
	GLuint geomShad;
	GLuint fragShad;
	GLuint vertShad;
	GLint success;

	/* do we need to find a new shader table entry for this sucker? */
	if (node->_shaderTableEntry == -1) {
		printf ("compile_Shape, assuming ste of 0\n");
		node->_shaderTableEntry = 0;
	}
	
	fragShad = 0; geomShad = 0; vertShad = 0;

	mySTE = &globalShaders[node->_shaderTableEntry];

	mySTE->myShaderProgram = glCreateProgram();

	getGeometryShader(node->geometry,&geomShad);
	getAppearanceShaders(node->appearance,&vertShad, &fragShad, (geomShad != 0));

	if (geomShad != 0) {
		printf ("compile_Shape, have geom shader\n");
		glProgramParameteriEXT (mySTE->myShaderProgram,GL_GEOMETRY_INPUT_TYPE_EXT,GL_TRIANGLES);
		glProgramParameteriEXT (mySTE->myShaderProgram, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
		glProgramParameteriEXT (mySTE->myShaderProgram, GL_GEOMETRY_VERTICES_OUT_EXT, 1024);
		ATTACH_SHADER(mySTE->myShaderProgram,geomShad);
	}


	ATTACH_SHADER(mySTE->myShaderProgram,fragShad);
	ATTACH_SHADER(mySTE->myShaderProgram,vertShad);

        LINK_SHADER(mySTE->myShaderProgram);

	#ifdef GL_VERSION_2_0
	#define MAX_INFO_LOG_SIZE 512
	glGetProgramiv(mySTE->myShaderProgram, GL_LINK_STATUS, &success); 
	if (!success) { 
		GLchar infoLog[MAX_INFO_LOG_SIZE]; 
		glGetProgramInfoLog(mySTE->myShaderProgram, MAX_INFO_LOG_SIZE, NULL, infoLog); 
		ConsoleMessage ("problem with Shader Program link: %s\n",infoLog); 
	} 
	#undef MAX_INFO_LOG_SIZE
	#endif

	getShaderCommonInterfaces (mySTE);
#endif

	MARK_NODE_COMPILED
}


void child_Shape (struct X3D_Shape *node) {
	struct X3D_Node *tmpN;
	int i;
	float dcol[4];
	float ecol[4];
	float scol[4];
	float amb;

	COMPILE_IF_REQUIRED

	/* JAS - if not collision, and render_geom is not set, no need to go further */
	/* printf ("child_Shape vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	 render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	if(!(node->geometry)) { return; }

	RECORD_DISTANCE

	if((render_collision) || (render_sensitive)) {
		/* only need to forward the call to the child */
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,node->geometry,tmpN);
		render_node(tmpN);
		return;
	}

	/*
	if we want to see the bounding box of this shape:
	drawBBOX(X3D_NODE(node));
	*/

	#ifdef SHADERS_2011
	if (node->_shaderTableEntry != -1) {
        	setCurrentShader (&globalShaders[node->_shaderTableEntry]);
	}
	#endif

	/* set up Appearance Properties here */
	this_textureTransform = NULL;
	linePropertySet=FALSE;
	appearanceProperties.transparency = MAX_NODE_TRANSPARENCY;  /* 1 == totally solid, 0 = totally transparent */  
	appearanceProperties.cubeFace = 0; /* assume no CubeMapTexture */
	material_twoSided = NULL;
	material_oneSided = NULL;

	/* a texture and a transparency flag... */
	textureStackTop = 0; /* will be >=1 if textures found */
	/* assume that lighting is enabled. Absence of Material or Appearance
	   node will turn lighting off; in this case, at the end of Shape, we
	   have to turn lighting back on again. */
	LIGHTING_ON;

	/* is there an associated appearance node? */
	RENDER_MATERIAL_SUBNODES(node->appearance);

	/* do the appearance here */
#ifdef SHAPEVERBOSE
	printf ("child_Shape, material_oneSided %u, textureStackTop %d\n",material_oneSided,textureStackTop);
	{int i; for (i=0; i<textureStackTop; i++) {
		printf ("boundTextureStack[%d] is texture %d\n",i,boundTextureStack[i]);
		if (textureParameterStack[i] == NULL) {
			printf ("textureParameterStack empty\n");
		} else {
			printf ("	texture_env_mode	 %d\n",textureParameterStack[i]->texture_env_mode);
			printf ("	combine_rgb		 %d\n",textureParameterStack[i]->combine_rgb);
			printf ("	source0_rgb		 %d\n",textureParameterStack[i]->source0_rgb);
			printf ("	operand0_rgb		 %d\n",textureParameterStack[i]->operand0_rgb);
			printf ("	source1_rgb		 %d\n",textureParameterStack[i]->source1_rgb);
			printf ("	operand1_rgb		 %d\n",textureParameterStack[i]->operand1_rgb);
			printf ("	combine_alpha		 %d\n",textureParameterStack[i]->combine_alpha);
			printf ("	source0_alpha		 %d\n",textureParameterStack[i]->source0_alpha);
			printf ("	operand0_alpha		 %d\n",textureParameterStack[i]->operand0_alpha);
			printf ("	source1_alpha		 %d\n",textureParameterStack[i]->source1_alpha);
			printf ("	operand1_alpha		 %d\n",textureParameterStack[i]->operand1_alpha);
			printf ("	rgb_scale		 %d\n",textureParameterStack[i]->rgb_scale);
			printf ("	alpha_scale		 %d\n",textureParameterStack[i]->alpha_scale);
		}
	}
	}
#endif

	/* if we do NOT have a shader node, do the appearance nodes */
        if (globalCurrentShader == 0) {
			if (material_oneSided != NULL) {
				/* we have a normal material node */
				appearanceProperties.transparency = 1.0f - material_oneSided->transparency; /* 1 == solid, 0 = totally transparent */ 

				FW_GL_MATERIALFV(GL_FRONT_AND_BACK, GL_DIFFUSE, material_oneSided->_dcol.c); 
				FW_GL_MATERIALFV(GL_FRONT_AND_BACK, GL_AMBIENT, material_oneSided->_amb.c);
				FW_GL_MATERIALFV(GL_FRONT_AND_BACK, GL_SPECULAR, material_oneSided->_scol.c);
				FW_GL_MATERIALFV(GL_FRONT_AND_BACK, GL_EMISSION, material_oneSided->_ecol.c);
				FW_GL_MATERIALF(GL_FRONT_AND_BACK, GL_SHININESS,material_oneSided->_shin);

				/* copy the emissive colour over for lines and points */
				memcpy(appearanceProperties.emissionColour,material_oneSided->_ecol.c, 3*sizeof(float));
			} else if (material_twoSided != NULL) {
				GLenum whichFace;
				float trans;
		
				/* we have a two sided material here */
				/* first, do back */
				if (material_twoSided->separateBackColor) {
					whichFace = GL_BACK;
					DO_MAT(material_twoSided,backDiffuseColor,backEmissiveColor,backShininess,backAmbientIntensity,backSpecularColor,backTransparency)
					whichFace = GL_FRONT;
				} else {
					whichFace=GL_FRONT_AND_BACK;
				}
				DO_MAT(material_twoSided,diffuseColor,emissiveColor,shininess,ambientIntensity,specularColor,transparency)


				/* Material twosided - emissiveColour for points, lines, etc - lets just set this up; we should remove
				   the DO_MAT calls above and do a compile-time verification of colours. */
				appearanceProperties.emissionColour[0] = 0.8f;
				appearanceProperties.emissionColour[1] = 0.8f;
				appearanceProperties.emissionColour[2] = 0.8f;
			} else {
	
				/* no material, so just colour the following shape */ 
				/* Spec says to disable lighting and set coloUr to 1,1,1 */ 
				LIGHTING_OFF  
				FW_GL_COLOR3F(1.0f,1.0f,1.0f); 
		 
				/* tell the rendering passes that this is just "normal" */ 
				last_texture_type = NOTEXTURE; 
				/* same with materialProperties.transparency */ 
				appearanceProperties.transparency=MAX_NODE_TRANSPARENCY; 
			}

	}


	/* now, are we rendering blended nodes or normal nodes?*/
	if (render_blend == (node->_renderFlags & VF_Blend)) {

#ifdef SHAPEOCCLUSION
		BEGINOCCLUSIONQUERY;
#endif
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->geometry,tmpN);
		render_node(tmpN);

#ifdef SHAPEOCCLUSION
		ENDOCCLUSIONQUERY;
#endif
	}

	/* did the lack of an Appearance or Material node turn lighting off? */
	LIGHTING_ON;

	/* any FillProperties? */
	TURN_FILLPROPERTIES_SHADER_OFF;

	if (linePropertySet) {
		FW_GL_DISABLE (GL_LINE_STIPPLE);
		FW_GL_LINEWIDTH(1.0f);
		FW_GL_POINTSIZE(1.0f);
	}

	/* were we cubemapping? */
	if (appearanceProperties.cubeFace !=0) {
		FW_GL_DISABLE(GL_TEXTURE_CUBE_MAP);
		FW_GL_DISABLE(GL_TEXTURE_GEN_S);
		FW_GL_DISABLE(GL_TEXTURE_GEN_T);
		FW_GL_DISABLE(GL_TEXTURE_GEN_R);
		appearanceProperties.cubeFace=0;
	}



	/* any shader turned on? if so, turn it off */
	TURN_GLOBAL_SHADER_OFF;

	/* turn off face culling */
	DISABLE_CULL_FACE;
}


void child_Appearance (struct X3D_Appearance *node) {
	struct X3D_Node *tmpN;
	
	/* initialization */
	last_texture_type = NOTEXTURE;
	
	/* printf ("in Appearance, this %d, nodeType %d\n",node, node->_nodeType);
	   printf (" vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	   render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */
	
	/* Render the material node... */
	RENDER_MATERIAL_SUBNODES(node->material);
	
	if (node->fillProperties) {
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->fillProperties,tmpN);
		render_node(tmpN);
	}
	
	/* set line widths - if we have line a lineProperties node */
	if (node->lineProperties) {
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->lineProperties,tmpN);
		render_node(tmpN);
	}
	
	if(node->texture) {
		/* we have to do a glPush, then restore, later */
		/* glPushAttrib(GL_ENABLE_BIT); */
		
		/* is there a TextureTransform? if no texture, fugutaboutit */
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->textureTransform,this_textureTransform);
		
		/* now, render the texture */
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->texture,tmpN);

		render_node(tmpN);
	}

	/* shaders here/supported?? */
	if (node->shaders.n !=0) {
		int count;
		int foundGoodShader = FALSE;
		
		for (count=0; count<node->shaders.n; count++) {
			POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->shaders.p[count], tmpN);
			
			/* have we found a valid shader yet? */
			if (foundGoodShader) {
				/* printf ("skipping shader %d of %d\n",count, node->shaders.n); */
				/* yes, just tell other shaders that they are not selected */
				SET_SHADER_SELECTED_FALSE(tmpN);
			} else {
				/* render this node; if it is valid, then we call this one the selected one */
				SET_FOUND_GOOD_SHADER(tmpN);
				DEBUG_SHADER("running shader (%s) %d of %d\n",
			    	 stringNodeType(X3D_NODE(tmpN)->_nodeType),count, node->shaders.n);
				render_node(tmpN);
			}
		}
	}
	
}
