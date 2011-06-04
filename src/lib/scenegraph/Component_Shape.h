/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Shape.h,v 1.12 2011/06/04 17:33:47 dug9 Exp $

Proximity sensor macro.

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



#ifndef __FREEWRL_SCENEGRAPH_SHAPE_H__
#define __FREEWRL_SCENEGRAPH_SHAPE_H__


/*
struct gl_LightSourceParameters {
	vec4 ambient; 
	vec4 diffuse; 
	vec4 specular; 
	vec4 position; 
	vec4 halfVector; 
	vec3 spotDirection; 
	float spotExponent; 
	float spotCutoff; // (range: [0.0,90.0], 180.0)
	float spotCosCutoff; // (range: [1.0,0.0],-1.0)
	float constantAttenuation; 
	float linearAttenuation; 
	float quadraticAttenuation;	
};
	
uniform gl_LightSourceParameters gl_LightSource[gl_MaxLights];
	
struct gl_LightModelParameters {
	vec4 ambient; 
};
	
uniform gl_LightModelParameters gl_LightModel;

struct gl_MaterialParameters {
		vec4 emission;   
		vec4 ambient;    
		vec4 diffuse;    
		vec4 specular;   
		float shininess; 
	};
	
uniform gl_MaterialParameters gl_FrontMaterial;
uniform gl_MaterialParameters gl_BackMaterial;
*/


struct fw_MaterialParameters {
		float emission[4];   
		float ambient[4];    
		float diffuse[4];    
		float specular[4];   
		float shininess; 
	};

struct matpropstruct {
	/* material properties for current shape */
	struct fw_MaterialParameters fw_FrontMaterial;
	struct fw_MaterialParameters fw_BackMaterial;

	/* which shader is active; 0 = no shader active */
	GLint currentShader; 
	s_shader_capabilities_t *currentShaderProperties;

	float	transparency;
	GLfloat	emissionColour[3];
	GLint   cubeFace;		/* for cubemapping, if 0, not cube mapping */
	int 	cullFace;		/* is this single-sided or two-sided? */
};

//extern struct matpropstruct appearanceProperties;
struct matpropstruct* getAppearanceProperties();

#define MIN_NODE_TRANSPARENCY 0.0f
#define MAX_NODE_TRANSPARENCY 0.99f  /* if 1.0, then occlusion culling will cause flashing */



#define DO_MAT(myNode, diffusec,emissc,shinc,ambc,specc,transc) \
		\
	/* set the diffuseColor; we will reset this later if the		\
	   texture depth is 3 (RGB texture) */		\
		\
	for (i=0; i<3;i++){ dcol[i] = myNode->diffusec.c[i]; }		\
		\
	/* set the transparency here for the material */		\
	trans = 1.0f - myNode->transc;		\
		\
	if (trans<0.0f) trans = 0.0f;		\
	if (trans>=0.999999f) trans = 0.9999999f;		\
	getAppearanceProperties()->transparency = trans;		\
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
	amb = myNode->ambc;		\
		\
 		for(i=0; i<3; i++) { dcol[i] *= amb; } 		\
	do_glMaterialfv(whichFace, GL_AMBIENT, dcol);		\
		\
	for (i=0; i<3;i++){ scol[i] = myNode->specc.c[i]; }		\
	do_glMaterialfv(whichFace, GL_SPECULAR, scol);		\
		\
	for (i=0; i<3;i++){ ecol[i] = myNode->emissc.c[i]; }		\
	do_glMaterialfv(whichFace, GL_EMISSION, ecol);		\
		\
	do_shininess(whichFace,myNode->shinc);


#define RENDER_MATERIAL_SUBNODES(which) \
	{ struct X3D_Node *tmpN;   \
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, which,tmpN) \
       		if(tmpN) { \
			render_node(tmpN); \
		} \
	}


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
#endif /* __FREEWRL_SCENEGRAPH_SHAPE_H__ */
