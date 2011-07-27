/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Shape.c,v 1.95 2011/07/27 23:42:31 crc_canada Exp $

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



typedef struct pComponent_Shape{

	int     linePropertySet;  /* line properties -width, etc                  */

	struct matpropstruct appearanceProperties;

	/* this is for the FillProperties node */
	GLuint fillpropCurrentShader;// = 0;

	/* pointer for a TextureTransform type of node */
	struct X3D_Node *  this_textureTransform;  /* do we have some kind of textureTransform? */

	/* for doing shader material properties */
	struct X3D_TwoSidedMaterial *material_twoSided;
	struct X3D_Material *material_oneSided;
	int fpshaderloaded;// = FALSE; 
	GLint hatchColour;
	GLint hatchPercent;
	GLint filledBool;
	GLint hatchedBool;
	GLint algorithm;
	GLint norm;
	GLint vert;
	GLint modView;
	GLint projMat;
	GLint normMat;

}* ppComponent_Shape;
void *Component_Shape_constructor(){
	void *v = malloc(sizeof(struct pComponent_Shape));
	memset(v,0,sizeof(struct pComponent_Shape));
	return v;
}
void Component_Shape_init(struct tComponent_Shape *t){
	//public
	//private
	t->prv = Component_Shape_constructor();
	{
		ppComponent_Shape p = (ppComponent_Shape)t->prv;
		//p->linePropertySet;  /* line properties -width, etc                  */

		//p->appearanceProperties;

		/* this is for the FillProperties node */
		p->fillpropCurrentShader = 0;

		/* pointer for a TextureTransform type of node */
		//p->this_textureTransform;  /* do we have some kind of textureTransform? */

		/* for doing shader material properties */
		//p->material_twoSided;
		//p->material_oneSided;
		p->fpshaderloaded = FALSE; 
		p->hatchColour = -1;
		p->hatchPercent = -1;
		p->filledBool= -1;
		p->hatchedBool = -1;
		p->algorithm = -1;
		p->norm = -1;
		p->vert = -1;
		p->modView = -1;
		p->projMat = -1;
		p->normMat = -1;
	}
}
//ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

//getters
struct matpropstruct *getAppearanceProperties(){
	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

	return &p->appearanceProperties;
}

struct X3D_Node *getThis_textureTransform(){
	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
	return p->this_textureTransform;
}

void render_LineProperties (struct X3D_LineProperties *node) {
	GLint	factor;
	GLushort pat;

	if (node->applied) {
		ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

		p->linePropertySet=TRUE;
		if (node->linewidthScaleFactor > 1.0) {
			FW_GL_LINEWIDTH(node->linewidthScaleFactor);
            #ifndef GL_ES_VERSION_2_0
			FW_GL_POINTSIZE(node->linewidthScaleFactor);
            #endif
		}


		if (node->linetype > 1) {
#ifndef GL_ES_VERSION_2_0
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
#else
            ConsoleMessage ("OpenGL-ES - no line stipple");
#endif
		}
	}
}


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


/* now works with our pushing matricies (norm, proj, modelview) but not for complete shader appearance replacement */
void render_FillProperties (struct X3D_FillProperties *node) {
	GLfloat hatchX;
	GLfloat hatchY;
	GLint algor;
	GLint hatched;
	GLint filled;
	int success;

	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

	if (!p->fpshaderloaded) {
		const char *vs = "\
			/* \n\
			  Shader source from \n\
			  \"Introduction to the OpenGL Shading Language\" \n\
			  presentation by Randi Rost, 3DLabs (GLSLOverview2005.pdf) \n\
			*/ \n\
			 \n\
uniform                mat4 fw_ModelViewMatrix; \n\
uniform                mat4 fw_ProjectionMatrix; \n\
uniform mat3	fw_NormalMatrix; \n\
			uniform vec3 LightPosition; \n\
			uniform bool filled; \n\
			const float SpecularContribution = 0.3; \n\
			const float DiffuseContribution = 1.0 - SpecularContribution; \n\
			varying float LightIntensity; \n\
			varying vec2 MCposition; \n\
			void main(void) \n\
			{ \n\
               gl_Position = fw_ProjectionMatrix * fw_ModelViewMatrix * gl_Vertex; \n\
			    vec3 ecPosition = vec3(fw_ModelViewMatrix * gl_Vertex); \n\
			    vec3 tnorm      = normalize(fw_NormalMatrix * gl_Normal); \n\
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
			    /* old - JAS gl_Position     = ftransform(); */ \n\
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
	
                COMPILE_SHADER(v);
                GET_SHADER_INFO(v, COMPILE_STATUS, &success);
                if (!success) {
                        shaderErrorLog(v,"GEOMETRY");
                }

		COMPILE_SHADER(f);
                GET_SHADER_INFO(f, COMPILE_STATUS, &success);
                if (!success) {
                        shaderErrorLog(f,"GEOMETRY");
                }

		#ifdef FILLVERBOSE
			printf ("creating program and attaching\n");
		#endif

		p->fillpropCurrentShader = CREATE_PROGRAM;
		
		ATTACH_SHADER(p->fillpropCurrentShader,v);
		ATTACH_SHADER(p->fillpropCurrentShader,f);
	
		#ifdef FILLVERBOSE
			printf ("linking program\n");
		#endif


		LINK_SHADER(p->fillpropCurrentShader);

		#ifdef FILLVERBOSE
			printf ("getting shader vars\n");
		#endif

		p->hatchColour = GET_UNIFORM(p->fillpropCurrentShader,"HatchColour");
		p->hatchPercent = GET_UNIFORM(p->fillpropCurrentShader,"HatchPct");
		p->filledBool = GET_UNIFORM(p->fillpropCurrentShader,"filled");
		p->hatchedBool = GET_UNIFORM(p->fillpropCurrentShader,"hatched");
		p->algorithm = GET_UNIFORM(p->fillpropCurrentShader,"algorithm");
		p->modView = GET_UNIFORM(p->fillpropCurrentShader, "fw_ModelViewMatrix");
		p->projMat = GET_UNIFORM(p->fillpropCurrentShader, "fw_ProjectionMatrix");
		p->normMat = GET_UNIFORM(p->fillpropCurrentShader, "fw_NormalMatrix");

		#ifdef FILLVERBOSE
			printf ("hatchColour %d hatchPercent %d filledbool %d hatchedbool %d algor %d\n",p->hatchColour,p->hatchPercent,p->filledBool,p->hatchedBool,p->algorithm);
			printf ("norm %d vert %d mod %d, proj %d norm %d\n",p->norm, p->vert, p->modView, p->projMat, p->normMat);
		#endif


		p->fpshaderloaded = TRUE;
	}
	USE_SHADER(p->fillpropCurrentShader);


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
	GLUNIFORM2F(p->hatchPercent,hatchX, hatchY);
	GLUNIFORM1I(p->filledBool,filled);
	GLUNIFORM1I(p->hatchedBool,hatched);
	GLUNIFORM1I(p->algorithm,algor);
	GLUNIFORM4F(p->hatchColour,node->hatchColor.c[0], node->hatchColor.c[1], node->hatchColor.c[2],1.0f);

	/* now for the transform, normal and modelview matricies */
	sendExplicitMatriciesToShader (p->modView, p->projMat, p->normMat);


}

void compile_TwoSidedMaterial (struct X3D_TwoSidedMaterial *node) {
	int i;
	float trans;

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


	/* first, put in the transparency */
	trans = node->transparency;
        node->_verifiedFrontColor.p[3] = trans;
        node->_verifiedFrontColor.p[7] = trans;
        node->_verifiedFrontColor.p[11] = trans;
        node->_verifiedFrontColor.p[15] = trans;
	trans = node->backTransparency;
        node->_verifiedBackColor.p[3] = trans;
        node->_verifiedBackColor.p[7] = trans;
        node->_verifiedBackColor.p[11] = trans;
        node->_verifiedBackColor.p[15] = trans;
                

	/* DiffuseColor */
	memcpy((void *)(&node->_verifiedFrontColor.p[8]), node->diffuseColor.c, sizeof (float) * 3);

	/* Ambient  - diffuseFrontColor * ambientIntensity */
        for(i=0; i<3; i++) { node->_verifiedFrontColor.p[i+4] = node->_verifiedFrontColor.p[i+8] * node->ambientIntensity; }

	/* Specular */
	memcpy((void *)(&node->_verifiedFrontColor.p[12]), node->specularColor.c, sizeof (float) * 3);

	/* Emissive */
	memcpy((void *)(&node->_verifiedFrontColor.p[0]), node->emissiveColor.c, sizeof (float) * 3);

	/* Shininess */
        node->_verifiedFrontColor.p[16] = node->shininess * 128.0f;

#define MAX_SHIN 128.0f
#define MIN_SHIN 0.01f
        if ((node->_verifiedFrontColor.p[16] > MAX_SHIN) || (node->_verifiedFrontColor.p[16] < MIN_SHIN)) {
                if (node->_verifiedFrontColor.p[16]>MAX_SHIN){node->_verifiedFrontColor.p[16] = MAX_SHIN;}else{node->_verifiedFrontColor.p[16]=MIN_SHIN;}
        }
#undef MAX_SHIN
#undef MIN_SHIN

	if (node->separateBackColor) {

		/* DiffuseColor */
		memcpy((void *)(&node->_verifiedBackColor.p[8]), node->backDiffuseColor.c, sizeof (float) * 3);
	
		/* Ambient  - diffuseBackColor * ambientIntensity */
	        for(i=0; i<3; i++) { node->_verifiedBackColor.p[i+4] = node->_verifiedBackColor.p[i+8] * node->ambientIntensity; }
	
		/* Specular */
		memcpy((void *)(&node->_verifiedBackColor.p[12]), node->backSpecularColor.c, sizeof (float) * 3);
	
		/* Emissive */
		memcpy((void *)(&node->_verifiedBackColor.p[0]), node->backEmissiveColor.c, sizeof (float) * 3);
	
		/* Shininess */
	        node->_verifiedBackColor.p[16] = node->shininess * 128.0f;
	
#define MAX_SHIN 128.0f
#define MIN_SHIN 0.01f
	        if ((node->_verifiedBackColor.p[16] > MAX_SHIN) || (node->_verifiedBackColor.p[16] < MIN_SHIN)) {
	                if (node->_verifiedBackColor.p[16]>MAX_SHIN){node->_verifiedBackColor.p[16] = MAX_SHIN;}else{node->_verifiedBackColor.p[16]=MIN_SHIN;}
	        }
#undef MAX_SHIN
#undef MIN_SHIN

	} else {
		/* just copy the front materials to the back */
		memcpy(node->_verifiedBackColor.p, node->_verifiedFrontColor.p, sizeof (float) * 17);
	}



	MARK_NODE_COMPILED
}

void render_TwoSidedMaterial (struct X3D_TwoSidedMaterial *node) {
	
	COMPILE_IF_REQUIRED
	{
	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

	/* record this node for OpenGL-ES and OpenGL-3.1 operation */
	p->material_twoSided = node;
	}
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
	/* Remember, VRML/X3D transparency 0.0 = solid; OpenGL 1.0 = solid, so we reverse it... */
        trans = 1.0f - node->transparency;
                
	/* we now keep verified params in a structure that maps to Shaders well...
		struct gl_MaterialParameters {
        	        vec4 emission;
        	        vec4 ambient;
        	        vec4 diffuse;
        	        vec4 specular;
        	        float shininess;
        	};
	  which is stored in the _verifiedColor[17] array here.
	  emission [0]..[3];
	  ambient [4]..[7];
	  diffuse [8]..[11];
	  specular [12]..[15];
	  shininess [16]
	  
*/
	/* first, put in the transparency */
        node->_verifiedColor.p[3] = trans;
        node->_verifiedColor.p[7] = trans;
        node->_verifiedColor.p[11] = trans;
        node->_verifiedColor.p[15] = trans;
                
	/* DiffuseColor */
	memcpy((void *)(&node->_verifiedColor.p[8]), node->diffuseColor.c, sizeof (float) * 3);

	/* Ambient  - diffuseColor * ambientIntensity */
        for(i=0; i<3; i++) { node->_verifiedColor.p[i+4] = node->_verifiedColor.p[i+8] * node->ambientIntensity; }

	/* Specular */
	memcpy((void *)(&node->_verifiedColor.p[12]), node->specularColor.c, sizeof (float) * 3);

	/* Emissive */
	memcpy((void *)(&node->_verifiedColor.p[0]), node->emissiveColor.c, sizeof (float) * 3);

	/* Shininess */
        node->_verifiedColor.p[16] = node->shininess * 128.0f;

#define MAX_SHIN 128.0f
#define MIN_SHIN 0.01f
        if ((node->_verifiedColor.p[16] > MAX_SHIN) || (node->_verifiedColor.p[16] < MIN_SHIN)) {
                if (node->_verifiedColor.p[16]>MAX_SHIN){node->_verifiedColor.p[16] = MAX_SHIN;}else{node->_verifiedColor.p[16]=MIN_SHIN;}
        }
#undef MAX_SHIN
#undef MIN_SHIN


	MARK_NODE_COMPILED
}


void render_Material (struct X3D_Material *node) {
	COMPILE_IF_REQUIRED
	{
	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

	/* record this node for OpenGL-ES and OpenGL-3.1 operation */
	p->material_oneSided = node;
	}
}


#ifdef SHADERS_2011

/* Least significant hex digit - appearance */
#define NO_APPEARANCE_SHADER 0x0000
#define MATERIAL_APPEARANCE_SHADER 0x0001
#define TWO_MATERIAL_APPEARANCE_SHADER 0x0002
#define ONE_TEX_APPEARANCE_SHADER 0x004

/* second least significant hex digit - PolyRep colour present */
#define NO_COLOUR_SHADER 0x0000
#define HAVE_COLOUR_SHADER 0x00010

/* third least significant hex digit - do shape geom in shader */
#define NO_GEOM_SHADER 0x0000
#define SPHERE_GEOM_SHADER 0x0100

/* fourth least significant hex digit - lines, points */
#define NO_LINES_POINTS 0x0000
#define HAVE_LINEPOINTS 0x1000

#define CHECK_COLOUR_FIELD(aaa) \
	case NODE_##aaa: { \
		struct X3D_##aaa *me = (struct X3D_##aaa *)realNode; \
		if (me->color == NULL) return NO_COLOUR_SHADER; \
		else return HAVE_COLOUR_SHADER; \
		break; \
	} 

#define CHECK_VRML1_COLOUR_FIELD(aaa) \
	case NODE_##aaa: { \
		struct X3D_##aaa *me = (struct X3D_##aaa *)realNode; \
		if (me->_color == NULL) return NO_COLOUR_SHADER; \
		else return HAVE_COLOUR_SHADER; \
		break; \
	} \

/* if this is a LineSet, PointSet, etc... */
static int getIfLinePoints(struct X3D_Node *myGeom) {
	struct X3D_Node *realNode;

	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,myGeom,realNode);

	if (realNode == NULL) return NO_GEOM_SHADER;
	switch (realNode->_nodeType) {
		case NODE_IndexedLineSet:
		case NODE_LineSet:
		case NODE_VRML1_IndexedLineSet:
		case NODE_PointSet:
		case NODE_VRML1_PointSet:
			return  HAVE_LINEPOINTS;

	}
	return NO_LINES_POINTS;
}



/* Some shapes have Color nodes - if so, then we have other shaders */
static int getShapeColourShader (struct X3D_Node *myGeom) {
	struct X3D_Node *realNode;

	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,myGeom,realNode);

	if (realNode == NULL) return NO_GEOM_SHADER;

	/* go through each node type that can have a Color node, and if it is not NULL
	   we know we have a Color node */

	switch (realNode->_nodeType) {
		CHECK_COLOUR_FIELD(IndexedFaceSet);
		CHECK_COLOUR_FIELD(IndexedLineSet);
		CHECK_COLOUR_FIELD(IndexedTriangleFanSet);
		CHECK_COLOUR_FIELD(IndexedTriangleSet);
		CHECK_COLOUR_FIELD(IndexedTriangleStripSet);
		CHECK_COLOUR_FIELD(LineSet);
		CHECK_COLOUR_FIELD(PointSet);
		CHECK_COLOUR_FIELD(TriangleFanSet);
		CHECK_COLOUR_FIELD(TriangleStripSet);
		CHECK_COLOUR_FIELD(TriangleSet);
		CHECK_COLOUR_FIELD(ElevationGrid);
		CHECK_COLOUR_FIELD(GeoElevationGrid);
		CHECK_VRML1_COLOUR_FIELD(VRML1_IndexedFaceSet);
	}

	/* if we are down here, we KNOW we do not have a color field */
	return NO_COLOUR_SHADER;
}

/* find info on the geometry of this shape */
static int getGeometryShader (struct X3D_Node *myGeom) {

	#ifdef HAVE_GEOMETRY_SHADERS
	struct X3D_Node *realNode;
    
	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,myGeom,realNode);

	if (realNode == NULL) return NO_GEOM_SHADER;

	if (realNode->_nodeType == NODE_Sphere) return SPHERE_GEOM_SHADER;
	#endif /* HAVE_GEOMETRY_SHADERS */

	return NO_GEOM_SHADER;
}

static int getAppearanceShader (struct X3D_Node *myApp) {
	struct X3D_Appearance *realAppearanceNode;
	struct X3D_Node *realMaterialNode;
	struct X3D_Node *realTextureNode;

	int retval = NO_APPEARANCE_SHADER;

	/* if there is no appearance node... */
	if (myApp == NULL) return retval;

	POSSIBLE_PROTO_EXPANSION(struct X3D_Appearance *,myApp, realAppearanceNode);
	if (realAppearanceNode->_nodeType != NODE_Appearance) return retval;

	if (realAppearanceNode->material != NULL) {
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,realAppearanceNode->material, realMaterialNode);
		if (realMaterialNode->_nodeType == NODE_Material) {
			retval |= MATERIAL_APPEARANCE_SHADER;
		}
		if (realMaterialNode->_nodeType == NODE_TwoSidedMaterial) {
			retval |= TWO_MATERIAL_APPEARANCE_SHADER;
		}
	}

	if (realAppearanceNode->texture != NULL) {
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,realAppearanceNode->texture, realTextureNode);
		if (realTextureNode->_nodeType == NODE_ImageTexture) {
			retval |= ONE_TEX_APPEARANCE_SHADER;
		} else {
			printf ("getAppearanceShader, texture field %s not supported yet\n",
			stringNodeType(realTextureNode->_nodeType));
		}
	}


	return retval;
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

#endif /* SHADERS_2011 */

/* this should be vectorized, and made global in the rdr_caps, but for now... */
//s_shader_capabilities_t globalShaders[10];


void compile_Shape (struct X3D_Shape *node) {
#ifdef SHADERS_2011

	int whichGeometryShader = -1;
	int whichAppearanceShader = -1;
	int whichShapeColorShader = -1;
	int isUnlitGeometry = -1;

	whichGeometryShader = getGeometryShader(node->geometry);
	whichShapeColorShader = getShapeColourShader(node->geometry);
	whichAppearanceShader = getAppearanceShader(node->appearance);
	isUnlitGeometry = getIfLinePoints(node->geometry);



	/* choose the shader. Note that we just "or" the results together */

	switch (whichGeometryShader | whichAppearanceShader | whichShapeColorShader | isUnlitGeometry) {

	/* SECTION 1 - Spheres CAN NOT have colour nodes, we can ignore the ColourShader field */
	case SPHERE_GEOM_SHADER | NO_APPEARANCE_SHADER:
		node->_shaderTableEntry = noMaterialNoAppearanceSphereShader;
		break;

	case SPHERE_GEOM_SHADER | MATERIAL_APPEARANCE_SHADER:
		node->_shaderTableEntry = noTexOneMaterialSphereShader;
		break;

	case SPHERE_GEOM_SHADER | TWO_MATERIAL_APPEARANCE_SHADER:
		node->_shaderTableEntry = noTexTwoMaterialSphereShader;
		break;

	case SPHERE_GEOM_SHADER | ONE_TEX_APPEARANCE_SHADER:
	case SPHERE_GEOM_SHADER | ONE_TEX_APPEARANCE_SHADER | MATERIAL_APPEARANCE_SHADER:
		node->_shaderTableEntry = oneTexOneMaterialSphereShader;
		break;	



	/* SECTION 2 - No Geom Shaders; No Color node in Shape */
	case NO_GEOM_SHADER | NO_APPEARANCE_SHADER| NO_COLOUR_SHADER:
		node->_shaderTableEntry = noMaterialNoAppearanceShader;
		break;

	case NO_GEOM_SHADER | TWO_MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER:
		node->_shaderTableEntry = noTexTwoMaterialShader;
		break;

	case NO_GEOM_SHADER | MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER:
		node->_shaderTableEntry = noTexOneMaterialShader;
		break;	

	case NO_GEOM_SHADER | ONE_TEX_APPEARANCE_SHADER | TWO_MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER:
		node->_shaderTableEntry = oneTexTwoMaterialShader;
		break;

	case NO_GEOM_SHADER | ONE_TEX_APPEARANCE_SHADER| NO_COLOUR_SHADER:
	case NO_GEOM_SHADER | ONE_TEX_APPEARANCE_SHADER | MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER:
		node->_shaderTableEntry = oneTexOneMaterialShader;
		break;	


	

	/* SECTION 3 - No Geom Shaders, HAVE Color node in shape */

	/* if we have a LineSet, PointSet, etc, and there is a Color node in it, choose this one! */	
	case NO_GEOM_SHADER | NO_APPEARANCE_SHADER| HAVE_COLOUR_SHADER | HAVE_LINEPOINTS:
	case NO_GEOM_SHADER | MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER | HAVE_LINEPOINTS:
	case NO_GEOM_SHADER | TWO_MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER | HAVE_LINEPOINTS:
	case NO_GEOM_SHADER | ONE_TEX_APPEARANCE_SHADER | TWO_MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER | HAVE_LINEPOINTS:
	case NO_GEOM_SHADER | ONE_TEX_APPEARANCE_SHADER| HAVE_COLOUR_SHADER | HAVE_LINEPOINTS:
	case NO_GEOM_SHADER | ONE_TEX_APPEARANCE_SHADER | MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER | HAVE_LINEPOINTS:
		node->_shaderTableEntry = linePointColorNodeShader;
		break;

	
	/* if we have a LineSet, PointSet, etc, and there is NOT a Color node in it, choose this one! */	
	case NO_GEOM_SHADER | NO_APPEARANCE_SHADER| NO_COLOUR_SHADER | HAVE_LINEPOINTS:
	case NO_GEOM_SHADER | MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER | HAVE_LINEPOINTS:
	case NO_GEOM_SHADER | TWO_MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER | HAVE_LINEPOINTS:
	case NO_GEOM_SHADER | ONE_TEX_APPEARANCE_SHADER | TWO_MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER | HAVE_LINEPOINTS:
	case NO_GEOM_SHADER | ONE_TEX_APPEARANCE_SHADER| NO_COLOUR_SHADER | HAVE_LINEPOINTS:
	case NO_GEOM_SHADER | ONE_TEX_APPEARANCE_SHADER | MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER | HAVE_LINEPOINTS:
		node->_shaderTableEntry = linePointNoColorNodeShader;
		break;

	case NO_GEOM_SHADER | NO_APPEARANCE_SHADER| HAVE_COLOUR_SHADER:
		node->_shaderTableEntry = backgroundSphereShader;
		break;

	case NO_GEOM_SHADER | TWO_MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER:
		node->_shaderTableEntry = noTexTwoMaterialColourShader;
		break;

	case NO_GEOM_SHADER | MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER:
		node->_shaderTableEntry = noTexOneMaterialColourShader;
		break;	

	case NO_GEOM_SHADER | ONE_TEX_APPEARANCE_SHADER | TWO_MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER:
		node->_shaderTableEntry = oneTexTwoMaterialColourShader;
		break;

	case NO_GEOM_SHADER | ONE_TEX_APPEARANCE_SHADER| HAVE_COLOUR_SHADER:
	case NO_GEOM_SHADER | ONE_TEX_APPEARANCE_SHADER | MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER:
		node->_shaderTableEntry = oneTexOneMaterialColourShader;
		break;	
	

	default: node->_shaderTableEntry = noMaterialNoAppearanceShader;
	}
/*
printf ("forcing to use noMaterialNoAppearanceShader\n");
node->_shaderTableEntry = noMaterialNoAppearanceShader;
*/

/* debug */

    
printf ("shape using shader: ");
switch (node->_shaderTableEntry) {
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
case linePointColorNodeShader: printf ("linePointColorNodeShader\n"); break;
case linePointNoColorNodeShader: printf ("linePointNoColorNodeShader\n"); break;
default: {printf ("no ascii equiv\n");}
}



	#endif /* SHADERS_2011 */


	MARK_NODE_COMPILED
}

#ifdef SHADERS_2011
void child_Shape (struct X3D_Shape *node) {
	struct X3D_Node *tmpN;
	// JAS int i;
	// JAS float dcol[4];
	// JAS float ecol[4];
	// JAS float scol[4];
	// JAS float amb;
    
	ppComponent_Shape p;
        ttglobal tg = gglobal();



	COMPILE_IF_REQUIRED

	/* JAS - if not collision, and render_geom is not set, no need to go further */
	/* printf ("child_Shape vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	 render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	if(!(node->geometry)) { return; }

	RECORD_DISTANCE

	if((renderstate()->render_collision) || (renderstate()->render_sensitive)) {
		/* only need to forward the call to the child */
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,node->geometry,tmpN);
		render_node(tmpN);
		return;
	}

	if (node->_shaderTableEntry == -1) {
		return;
	}
        p = (ppComponent_Shape)tg->Component_Shape.prv;
	{
		struct fw_MaterialParameters defaultMaterials = {
					{0.0f, 0.0f, 0.0f, 1.0f}, /* emissiveColor */
					{0.0f, 0.0f, 0.0f, 1.0f}, /* ambientIntensity */
					{0.8f, 0.8f, 0.8f, 1.0f}, /* diffuseColor */
					{0.0f, 0.0f, 0.0f, 1.0f}, /* specularColor */
					10.0f};                   /* shininess */

		/* copy the material stuff in preparation for copying all to the shader */
		memcpy (&p->appearanceProperties.fw_FrontMaterial, &defaultMaterials, sizeof (struct fw_MaterialParameters));
		memcpy (&p->appearanceProperties.fw_BackMaterial, &defaultMaterials, sizeof (struct fw_MaterialParameters));

	}
	/* enable the shader for this shape */
        enableGlobalShader (node->_shaderTableEntry);

	/* now, are we rendering blended nodes or normal nodes?*/
	if (renderstate()->render_blend == (node->_renderFlags & VF_Blend)) {

		RENDER_MATERIAL_SUBNODES(node->appearance);

		if (p->material_oneSided != NULL) {

			memcpy (&p->appearanceProperties.fw_FrontMaterial, p->material_oneSided->_verifiedColor.p, sizeof (struct fw_MaterialParameters));
			memcpy (&p->appearanceProperties.fw_BackMaterial, p->material_oneSided->_verifiedColor.p, sizeof (struct fw_MaterialParameters));
			/* copy the emissive colour over for lines and points */
			memcpy(p->appearanceProperties.emissionColour,&(p->material_oneSided->_verifiedColor.p[0]), 3*sizeof(float));

		} else if (p->material_twoSided != NULL) {
			memcpy (&p->appearanceProperties.fw_FrontMaterial, p->material_twoSided->_verifiedFrontColor.p, sizeof (struct fw_MaterialParameters));
			memcpy (&p->appearanceProperties.fw_BackMaterial, p->material_twoSided->_verifiedBackColor.p, sizeof (struct fw_MaterialParameters));
			/* copy the emissive colour over for lines and points */
			memcpy(p->appearanceProperties.emissionColour,&(p->material_twoSided->_verifiedFrontColor.p[0]), 3*sizeof(float));
		} else {
			/* no materials selected.... */
		}

		/* send along lighting, material, other visible properties */
		sendMaterialsToShader(p->appearanceProperties.currentShaderProperties);

		#ifdef SHAPEOCCLUSION
		beginOcclusionQuery((struct X3D_VisibilitySensor*)node,renderstate()->render_geom); //BEGINOCCLUSIONQUERY;
		#endif

		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->geometry,tmpN);
		render_node(tmpN);

		#ifdef SHAPEOCCLUSION
		endOcclusionQuery((struct X3D_VisibilitySensor*)node,renderstate()->render_geom); //ENDOCCLUSIONQUERY;
		#endif
	}

	/* any shader turned on? if so, turn it off */
	TURN_GLOBAL_SHADER_OFF;
	p->material_twoSided = NULL;
	p->material_oneSided = NULL;

    
	/* did the lack of an Appearance or Material node turn lighting off? */
	LIGHTING_ON;

	/* turn off face culling */
	DISABLE_CULL_FACE;

}

#else /* ifdef SHADERS_2011 */

void child_Shape (struct X3D_Shape *node) {
	struct X3D_Node *tmpN;
	int i;
	float dcol[4];
	float ecol[4];
	float scol[4];
	float amb;
	ppComponent_Shape p;

	ttglobal tg = gglobal();

	COMPILE_IF_REQUIRED

	/* JAS - if not collision, and render_geom is not set, no need to go further */
	/* printf ("child_Shape vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	 render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	if(!(node->geometry)) { return; }

	RECORD_DISTANCE

	if((renderstate()->render_collision) || (renderstate()->render_sensitive)) {
		/* only need to forward the call to the child */
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,node->geometry,tmpN);
		render_node(tmpN);
		return;
	}

	/*
	if we want to see the bounding box of this shape:
	drawBBOX(X3D_NODE(node));
	*/
	p = (ppComponent_Shape)tg->Component_Shape.prv;

	/* set up Appearance Properties here */
	p->this_textureTransform = NULL;
	p->linePropertySet=FALSE;
	p->appearanceProperties.transparency = MAX_NODE_TRANSPARENCY;  /* 1 == totally solid, 0 = totally transparent */  
	p->appearanceProperties.cubeFace = 0; /* assume no CubeMapTexture */
	p->material_twoSided = NULL;
	p->material_oneSided = NULL;

	/* a texture and a transparency flag... */
	tg->RenderFuncs.textureStackTop = 0; /* will be >=1 if textures found */
	/* assume that lighting is enabled. Absence of Material or Appearance
	   node will turn lighting off; in this case, at the end of Shape, we
	   have to turn lighting back on again. */
	LIGHTING_ON;

	/* is there an associated appearance node? */
	RENDER_MATERIAL_SUBNODES(node->appearance);

	/* do the appearance here */
#ifdef SHAPEVERBOSE
	printf ("child_Shape, material_oneSided %u, tg->RenderFuncs.textureStackTop %d\n",material_oneSided,textureStackTop);
	{int i; for (i=0; i<tg->RenderFuncs.textureStackTop; i++) {
		printf ("boundTextureStack[%d] is texture %d\n",i,boundTextureStack[i]);
		if (tg->RenderTextures.textureParameterStack[i] == NULL) {
			printf ("textureParameterStack empty\n");
		} else {
			struct multiTexParams *tps = (struct multiTexParams *)tg->RenderTextures.textureParameterStack[i];
			printf ("	texture_env_mode	 %d\n",tps->texture_env_mode);
			printf ("	combine_rgb		 %d\n",tps->combine_rgb);
			printf ("	source0_rgb		 %d\n",tps->source0_rgb);
			printf ("	operand0_rgb		 %d\n",tps->operand0_rgb);
			printf ("	source1_rgb		 %d\n",tps->source1_rgb);
			printf ("	operand1_rgb		 %d\n",tps->operand1_rgb);
			printf ("	combine_alpha		 %d\n",tps->combine_alpha);
			printf ("	source0_alpha		 %d\n",tps->source0_alpha);
			printf ("	operand0_alpha		 %d\n",tps->operand0_alpha);
			printf ("	source1_alpha		 %d\n",tps->source1_alpha);
			printf ("	operand1_alpha		 %d\n",tps->operand1_alpha);
			printf ("	rgb_scale		 %d\n",tps->rgb_scale);
			printf ("	alpha_scale		 %d\n",tps->alpha_scale);

			//printf ("	texture_env_mode	 %d\n",textureParameterStack[i]->texture_env_mode);
			//printf ("	combine_rgb		 %d\n",textureParameterStack[i]->combine_rgb);
			//printf ("	source0_rgb		 %d\n",textureParameterStack[i]->source0_rgb);
			//printf ("	operand0_rgb		 %d\n",textureParameterStack[i]->operand0_rgb);
			//printf ("	source1_rgb		 %d\n",textureParameterStack[i]->source1_rgb);
			//printf ("	operand1_rgb		 %d\n",textureParameterStack[i]->operand1_rgb);
			//printf ("	combine_alpha		 %d\n",textureParameterStack[i]->combine_alpha);
			//printf ("	source0_alpha		 %d\n",textureParameterStack[i]->source0_alpha);
			//printf ("	operand0_alpha		 %d\n",textureParameterStack[i]->operand0_alpha);
			//printf ("	source1_alpha		 %d\n",textureParameterStack[i]->source1_alpha);
			//printf ("	operand1_alpha		 %d\n",textureParameterStack[i]->operand1_alpha);
			//printf ("	rgb_scale		 %d\n",textureParameterStack[i]->rgb_scale);
			//printf ("	alpha_scale		 %d\n",textureParameterStack[i]->alpha_scale);

		}
	}
	}
#endif

	/* if we do NOT have a shader node, do the appearance nodes */
        if (p->appearanceProperties.currentShader == 0) {
			if (p->material_oneSided != NULL) {
				/* we have a normal material node */
				p->appearanceProperties.transparency = 1.0f - p->material_oneSided->transparency; /* 1 == solid, 0 = totally transparent */ 

	/* we now keep verified params in a structure that maps to Shaders well...
		struct gl_MaterialParameters {
        	        vec4 emission;
        	        vec4 ambient;
        	        vec4 diffuse;
        	        vec4 specular;
        	        float shininess;
        	};
	  which is stored in the _verifiedColor[17] array here.
	  emission [0]..[3];
	  ambient [4]..[7];
	  diffuse [8]..[11];
	  specular [12]..[15];
	  shininess [16]
	  
*/
				FW_GL_MATERIALFV(GL_FRONT_AND_BACK, GL_DIFFUSE, &(p->material_oneSided->_verifiedColor.p[8])); 
				FW_GL_MATERIALFV(GL_FRONT_AND_BACK, GL_AMBIENT, &(p->material_oneSided->_verifiedColor.p[4]));
				FW_GL_MATERIALFV(GL_FRONT_AND_BACK, GL_SPECULAR, &(p->material_oneSided->_verifiedColor.p[12]));
				FW_GL_MATERIALFV(GL_FRONT_AND_BACK, GL_EMISSION, &(p->material_oneSided->_verifiedColor.p[0]));
				FW_GL_MATERIALF(GL_FRONT_AND_BACK, GL_SHININESS,p->material_oneSided->_verifiedColor.p[16]);

				/* copy the emissive colour over for lines and points */
				memcpy(p->appearanceProperties.emissionColour,&(p->material_oneSided->_verifiedColor.p[0]), 3*sizeof(float));
			} else if (p->material_twoSided != NULL) {
				GLenum whichFace;
				float trans;
		
				/* we have a two sided material here */
				/* first, do back */
				if (p->material_twoSided->separateBackColor) {
					whichFace = GL_BACK;
					DO_MAT(p->material_twoSided,backDiffuseColor,backEmissiveColor,backShininess,backAmbientIntensity,backSpecularColor,backTransparency)
					whichFace = GL_FRONT;
				} else {
					whichFace=GL_FRONT_AND_BACK;
				}
				DO_MAT(p->material_twoSided,diffuseColor,emissiveColor,shininess,ambientIntensity,specularColor,transparency)


				/* Material twosided - emissiveColour for points, lines, etc - lets just set this up; we should remove
				   the DO_MAT calls above and do a compile-time verification of colours. */
				p->appearanceProperties.emissionColour[0] = 0.8f;
				p->appearanceProperties.emissionColour[1] = 0.8f;
				p->appearanceProperties.emissionColour[2] = 0.8f;
			} else {
	
				/* no material, so just colour the following shape */ 
				/* Spec says to disable lighting and set coloUr to 1,1,1 */ 
				LIGHTING_OFF  
				FW_GL_COLOR3F(1.0f,1.0f,1.0f); 
		 
				/* tell the rendering passes that this is just "normal" */ 
				tg->RenderFuncs.last_texture_type = NOTEXTURE; 
				/* same with materialProperties.transparency */ 
				p->appearanceProperties.transparency=MAX_NODE_TRANSPARENCY; 
			}
	}

	/* now, are we rendering blended nodes or normal nodes?*/
	if (renderstate()->render_blend == (node->_renderFlags & VF_Blend)) {

		#ifdef SHAPEOCCLUSION
		beginOcclusionQuery((struct X3D_VisibilitySensor*)node,renderstate()->render_geom); //BEGINOCCLUSIONQUERY;
		#endif
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->geometry,tmpN);
		render_node(tmpN);

		#ifdef SHAPEOCCLUSION
		endOcclusionQuery((struct X3D_VisibilitySensor*)node,renderstate()->render_geom); //ENDOCCLUSIONQUERY;
		#endif
	}

	/* did the lack of an Appearance or Material node turn lighting off? */
	LIGHTING_ON;

	/* any FillProperties? */
	TURN_FILLPROPERTIES_SHADER_OFF;


	if (p->linePropertySet) {
		FW_GL_DISABLE (GL_LINE_STIPPLE);
		FW_GL_LINEWIDTH(1.0f);
        #ifndef GL_ES_VERSION_2_0
		FW_GL_POINTSIZE(1.0f);
        #endif
	}

	/* were we cubemapping? */
	if (p->appearanceProperties.cubeFace !=0) {
		FW_GL_DISABLE(GL_TEXTURE_CUBE_MAP);
		FW_GL_DISABLE(GL_TEXTURE_GEN_S);
		FW_GL_DISABLE(GL_TEXTURE_GEN_T);
		FW_GL_DISABLE(GL_TEXTURE_GEN_R);
		p->appearanceProperties.cubeFace=0;
	}


	/* any shader turned on? if so, turn it off */
	TURN_GLOBAL_SHADER_OFF;

	/* turn off face culling */
	DISABLE_CULL_FACE;
}

#endif /* SHADERS_2011 */


void child_Appearance (struct X3D_Appearance *node) {
	struct X3D_Node *tmpN;
	
	/* initialization */
	gglobal()->RenderFuncs.last_texture_type = NOTEXTURE;
	
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
		ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

		/* is there a TextureTransform? if no texture, fugutaboutit */
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->textureTransform,p->this_textureTransform);
		
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
