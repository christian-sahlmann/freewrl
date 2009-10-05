/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Shape.c,v 1.19 2009/10/05 15:07:23 crc_canada Exp $

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
#include "Component_ProgrammableShaders.h"
#include "Component_Shape.h"


static int     linePropertySet;  /* line properties -width, etc                  */

struct matpropstruct appearanceProperties;

/* this is for the FillProperties node */
static GLuint fillpropCurrentShader = 0;

/* pointer for a TextureTransform type of node */
struct X3D_Node *  this_textureTransform;  /* do we have some kind of textureTransform? */

 
#define SET_SHADER_SELECTED_FALSE(x3dNode) \
	switch (X3D_NODE(x3dNode)->_nodeType) { \
		case NODE_ComposedShader: \
			X3D_COMPOSEDSHADER(x3dNode)->isSelected = FALSE; \
			break; \
		case NODE_ProgramShader: \
			X3D_PROGRAMSHADER(x3dNode)->isSelected = FALSE; \
			break; \
		case NODE_PackagedShader: \
			X3D_PROGRAMSHADER(x3dNode)->isSelected = FALSE; \
			break; \
		default: { \
			/* this is NOT a shader; should we say something, or just \
			   ignore? Lets ignore, for now */ \
		} \
	}

#define SET_FOUND_GOOD_SHADER(x3dNode) \
	switch (X3D_NODE(x3dNode)->_nodeType) { \
		case NODE_ComposedShader: \
			foundGoodShader = X3D_COMPOSEDSHADER(x3dNode)->isValid; \
			X3D_COMPOSEDSHADER(x3dNode)->isSelected = foundGoodShader; \
			break; \
		case NODE_ProgramShader: \
			foundGoodShader = X3D_PROGRAMSHADER(x3dNode)->isValid; \
			X3D_PROGRAMSHADER(x3dNode)->isSelected = foundGoodShader; \
			break; \
		case NODE_PackagedShader: \
			foundGoodShader = X3D_PROGRAMSHADER(x3dNode)->isValid; \
			X3D_PACKAGEDSHADER(x3dNode)->isSelected = foundGoodShader; \
			break; \
		default: { \
			/* this is NOT a shader; should we say something, or just \
			   ignore? Lets ignore, for now */ \
		} \
	}

void render_LineProperties (struct X3D_LineProperties *node) {
	GLint	factor;
	GLushort pat;

	if (node->applied) {
		linePropertySet=TRUE;
		if (node->linewidthScaleFactor > 1.0) {
			glLineWidth(node->linewidthScaleFactor);
			glPointSize(node->linewidthScaleFactor);
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
			glLineStipple (factor,pat);
			glEnable(GL_LINE_STIPPLE);
		}
	}
}


/* FillProperties code. Use a Shader to do the filling. */

#ifdef GL_VERSION_2_0
	#define HAVE_SHADERS
	#define VERTEX_SHADER GL_VERTEX_SHADER
	#define FRAGMENT_SHADER GL_FRAGMENT_SHADER
	#define SHADER_SOURCE glShaderSource
	#define COMPILE_SHADER glCompileShader
	#define CREATE_PROGRAM glCreateProgram();
	#define ATTACH_SHADER glAttachShader
	#define LINK_SHADER glLinkProgram
	#define USE_SHADER glUseProgram
	#define CREATE_SHADER glCreateShader
	#define GET_UNIFORM(aaa,bbb) glGetUniformLocation(aaa,bbb)
	#define GLUNIFORM2F glUniform2f
	#define GLUNIFORM1I glUniform1i
	#define GLUNIFORM4F glUniform4f
#else
#ifdef GL_VERSION_1_5
	#define HAVE_SHADERS
	#define VERTEX_SHADER GL_VERTEX_SHADER_ARB
	#define FRAGMENT_SHADER GL_FRAGMENT_SHADER_ARB
	#define SHADER_SOURCE glShaderSourceARB
	#define COMPILE_SHADER glCompileShaderARB
	#define CREATE_PROGRAM glCreateProgramObjectARB();
	#define ATTACH_SHADER glAttachObjectARB
	#define LINK_SHADER glLinkProgramARB
	#define USE_SHADER  glUseProgramObjectARB
	#define CREATE_SHADER glCreateShaderObjectARB
	#define GET_UNIFORM(aaa,bbb) glGetUniformLocationARB(aaa,bbb)
	#define GLUNIFORM2F GLUNIFORM2FARB
	#define GLUNIFORM1I glUniform1iARB
	#define GLUNIFORM4F glUniform4fARB
#endif
#endif

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
			    vec3 ecPosition = vec3(gl_ModelViewMatrix * gl_Vertex); \n\
			    vec3 tnorm      = normalize(gl_NormalMatrix * gl_Normal); \n\
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


	hatchX = 0.80; hatchY = 0.80;
	algor = node->hatchStyle; filled = node->filled; hatched = node->hatched;
	switch (node->hatchStyle) {
		case 1: hatchX = 1.0; break; /* horizontal lines */
		case 2: hatchY = 1.0; break; /* vertical lines */
		case 3: hatchY=1.0; break; /* positive sloped lines */
		case 4: hatchY=1.0; break; /* negative sloped lines */
		case 5: break; /* square pattern */
		case 6: hatchY = 1.0; break; /* diamond pattern */

		default :{
			node->hatched = FALSE;
		}
	}
	GLUNIFORM2F(hatchPercent,hatchX, hatchY);
	GLUNIFORM1I(filledBool,filled);
	GLUNIFORM1I(hatchedBool,hatched);
	GLUNIFORM1I(algorithm,algor);
	GLUNIFORM4F(hatchColour,node->hatchColor.c[0], node->hatchColor.c[1], node->hatchColor.c[2],1.0);
}




#define DO_MAT(diffusec,emissc,shinc,ambc,specc,transc) \
		\
	/* set the diffuseColor; we will reset this later if the		\
	   texture depth is 3 (RGB texture) */		\
		\
	for (i=0; i<3;i++){ dcol[i] = node->diffusec.c[i]; }		\
		\
	/* set the transparency here for the material */		\
	trans = 1.0 - node->transc;		\
		\
	if (trans<0.0) trans = 0.0;		\
	if (trans>=0.999999) trans = 0.9999999;		\
	appearanceProperties.transparency = trans;		\
		\
	dcol[3] = trans;		\
	scol[3] = trans;		\
	ecol[3] = trans;		\
		\
	/* the diffuseColor might change, depending on the texture depth - that we do not have yet */		\
	do_glMaterialfv(whichFace, GL_DIFFUSE, dcol);		\
		\
	/* do the ambientIntensity; this will allow lights with ambientIntensity to		\
	   illuminate it as per the spec. Note that lights have the ambientIntensity		\
	   set to 0.0 by default; this should make ambientIntensity lighting be zero		\
	   via OpenGL lighting equations. */		\
	amb = node->ambc;		\
		\
 		for(i=0; i<3; i++) { dcol[i] *= amb; } 		\
	do_glMaterialfv(whichFace, GL_AMBIENT, dcol);		\
		\
	for (i=0; i<3;i++){ scol[i] = node->specc.c[i]; }		\
	do_glMaterialfv(whichFace, GL_SPECULAR, scol);		\
		\
	for (i=0; i<3;i++){ ecol[i] = node->emissc.c[i]; }		\
	do_glMaterialfv(whichFace, GL_EMISSION, ecol);		\
		\
	do_shininess(whichFace,node->shinc);





void render_TwoSidedMaterial (struct X3D_TwoSidedMaterial *node) {
	int i;
	float dcol[4];
	float ecol[4];
	float scol[4];
	float amb;
	float trans= 1.0;

	GLenum whichFace;

	/* first, do back */
	if (node->separateBackColor) {
		whichFace = GL_BACK;
		DO_MAT(backDiffuseColor,backEmissiveColor,backShininess,backAmbientIntensity,backSpecularColor,backTransparency)
		whichFace = GL_FRONT;
	} else {
		whichFace=GL_FRONT_AND_BACK;
	}
	DO_MAT(diffuseColor,emissiveColor,shininess,ambientIntensity,specularColor,transparency)
	
	/* remember this one */
	appearanceProperties.transparency = trans;
}


void compile_Material (struct X3D_Material *node) {

#ifdef wrwerwer

                ambientIntensity => [SFFloat, 0.2, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                diffuseColor => [SFColor, [0.8, 0.8, 0.8], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                emissiveColor => [SFColor, [0, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                metadata => [SFNode, NULL, inputOutput, "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                shininess => [SFFloat, 0.2, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                specularColor => [SFColor, [0, 0, 0], inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                transparency => [SFFloat, 0, inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
                __ambientIntensity => [SFFloat, 0.2, inputOutput, 0],
                __diffuseColor => [SFColor, [0.8, 0.8, 0.8], inputOutput, 0],
                __emissiveColor => [SFColor, [0, 0, 0], inputOutput, 0],
                __shininess => [SFFloat, 0.2, inputOutput, 0],
                __specularColor => [SFColor, [0, 0, 0], inputOutput, 0],
                __transparency => [SFFloat, 0, inputOutput, 0],

#endif
	MARK_NODE_COMPILED
}


void render_Material (struct X3D_Material *node) {
	int i;
	float dcol[4];
	float ecol[4];
	float scol[4];
	float amb;
	float trans=1.0;

	#define whichFace GL_FRONT_AND_BACK

	COMPILE_IF_REQUIRED

	DO_MAT(diffuseColor,emissiveColor,shininess,ambientIntensity,specularColor,transparency)
	
	/* remember this one */
	appearanceProperties.transparency = trans;
}


void child_Shape (struct X3D_Shape *node) {
	void *tmpN;

	if(!(node->geometry)) { return; }


	RECORD_DISTANCE

	if((render_collision) || (render_sensitive)) {
		/* only need to forward the call to the child */
		POSSIBLE_PROTO_EXPANSION(node->geometry,tmpN);
		render_node(tmpN);
		return;
	}

	/* set up Appearance Properties here */
	/* reset textureTransform pointer */
	this_textureTransform = NULL;
	linePropertySet=FALSE;
	appearanceProperties.transparency = 0.0;


	/* JAS - if not collision, and render_geom is not set, no need to go further */
	/* printf ("render_Shape vp %d geom %d light %d sens %d blend %d prox %d col %d\n",*/
	/* render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision);*/

	/* a texture and a transparency flag... */
	texture_count = 0; /* will be >=1 if textures found */

	/* assume that lighting is enabled. Absence of Material or Appearance
	   node will turn lighting off; in this case, at the end of Shape, we
	   have to turn lighting back on again. */
	LIGHTING_ON;

	/* if we have a very few samples, it means that:
		- Occlusion culling is working on this system (default is -1)
		- this node is very small in the scene;
		- if it is 0, it means that we are trying this shape for 
		  Occlusion Culling.
	*/

	if (!OccFailed && (node->__Samples <=4)) {
		/* draw this as a subdued grey */
		glColor3f(0.3,0.3,0.3);

		/* dont do any textures, or anything */
		last_texture_type = NOTEXTURE;
	} else {
		/* is there an associated appearance node? */
		RENDER_MATERIAL_SUBNODES(node->appearance);
	}

	/* now, are we rendering blended nodes or normal nodes?*/
	if (render_blend == (node->_renderFlags & VF_Blend)) {

#ifdef SHAPEOCCLUSION
		BEGINOCCLUSIONQUERY;
#endif
		POSSIBLE_PROTO_EXPANSION(node->geometry,tmpN);
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
		glDisable (GL_LINE_STIPPLE);
		glLineWidth(1.0);
		glPointSize(1.0);
	}

	/* any shader turned on? if so, turn it off */
	TURN_APPEARANCE_SHADER_OFF;
}


void child_Appearance (struct X3D_Appearance *node) {
	void *tmpN;
	struct X3D_Node *localShaderNode;
	
	/* initialization */
	last_texture_type = NOTEXTURE;
	localShaderNode = NULL;
	
	/* printf ("in Appearance, this %d, nodeType %d\n",node, node->_nodeType);
	   printf (" vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	   render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */
	
	/* shaders here/supported?? */
	if (node->shaders.n !=0) {
		int count;
		int foundGoodShader = FALSE;
		
		for (count=0; count<node->shaders.n; count++) {
			POSSIBLE_PROTO_EXPANSION(node->shaders.p[count], tmpN);
			
			/* have we found a valid shader yet? */
			if (foundGoodShader) {
				/* printf ("skipping shader %d of %d\n",count, node->shaders.n); */
				/* yes, just tell other shaders that they are not selected */
				SET_SHADER_SELECTED_FALSE(tmpN);
			} else {
				/* render this node; if it is valid, then we call this one the selected one */
				localShaderNode = tmpN;
				SET_FOUND_GOOD_SHADER(localShaderNode);
			}
		}
	}
	
	/* Render the material node... */
	RENDER_MATERIAL_SUBNODES(node->material);
	
	if (node->fillProperties) {
		POSSIBLE_PROTO_EXPANSION(node->fillProperties,tmpN);
		render_node(tmpN);
	}
	
	/* set line widths - if we have line a lineProperties node */
	if (node->lineProperties) {
		POSSIBLE_PROTO_EXPANSION(node->lineProperties,tmpN);
		render_node(tmpN);
	}
	
	if(node->texture) {
		/* we have to do a glPush, then restore, later */
		/* glPushAttrib(GL_ENABLE_BIT); */
		
		/* is there a TextureTransform? if no texture, fugutaboutit */
		POSSIBLE_PROTO_EXPANSION(node->textureTransform,this_textureTransform);
		
		/* now, render the texture */
		POSSIBLE_PROTO_EXPANSION(node->texture,tmpN);

		render_node(tmpN);
	}
	/* shaders here/supported?? */
	if (localShaderNode != NULL) {
		DEBUG_SHADER("running shader (%s) %d of %d\n",
			     stringNodeType(X3D_NODE(localShaderNode)->_nodeType),count, node->shaders.n);
		render_node(localShaderNode);
	}
}
