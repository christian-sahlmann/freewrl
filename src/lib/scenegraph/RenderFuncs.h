/*
=INSERT_TEMPLATE_HERE=

$Id: RenderFuncs.h,v 1.18 2011/01/04 15:43:32 crc_canada Exp $

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


#ifndef __FREEWRL_SCENEGRAPH_RENDERFUNCS_H__
#define __FREEWRL_SCENEGRAPH_RENDERFUNCS_H__

void chooseBackgroundShader (shader_type_t);
void setCurrentShader(s_shader_capabilities_t *);

void turnGlobalShaderOff(void);

#ifdef GL_VERSION_2_0
	#define TURN_GLOBAL_SHADER_OFF \
		turnGlobalShaderOff()
	#define TURN_FILLPROPERTIES_SHADER_OFF \
		{if (fillpropCurrentShader!=0) { glUseProgram(0);}}
#else
	#ifdef GL_VERSION_1_5
		#define TURN_GLOBAL_SHADER_OFF \
		turnGlobalShaderOff()
		#define TURN_FILLPROPERTIES_SHADER_OFF \
			{if (fillpropCurrentShader!=0) { fillpropCurrentShader = 0; glUseProgramObjectARB(0);}}
	#else
		#define TURN_GLOBAL_SHADER_OFF
		#define TURN_FILLPROPERTIES_SHADER_OFF
	#endif
#endif

/* trat: test if a ratio is reasonable */
#define TRAT(a) ((a) > 0 && ((a) < hitPointDist || hitPointDist < 0))

/* structure for rayhits */
struct currayhit {
	struct X3D_Node *hitNode; /* What node hit at that distance? */
	GLDOUBLE modelMatrix[16]; /* What the matrices were at that node */
	GLDOUBLE projMatrix[16];
};

extern struct currayhit rayHit,rayph,rayHitHyper;
extern double hitPointDist;                   /* in VRMLC.pm */
extern struct point_XYZ hp;                     /* in VRMLC.pm */
extern void *hypersensitive;            /* in VRMLC.pm */
extern int hyperhit;                    /* in VRMLC.pm */
extern struct point_XYZ r1, r2;         /* in VRMLC.pm */


/* function protos */
int nextlight(void);
void render_node(struct X3D_Node *node);

extern int BrowserAction;
extern struct X3D_Anchor *AnchorsAnchor;
extern char *OSX_replace_world_from_console;

extern GLint lightOnOff[];


void lightState(GLint light, int status);
void saveLightState(int *ls);
void restoreLightState(int *ls);
void fwglLightfv (int light, int pname, GLfloat *params);
void fwglLightf (int light, int pname, GLfloat param);
void initializeLightTables(void);
void chooseAppearanceShader(shader_type_t requestedShader, struct X3D_Material *material_oneSided, struct X3D_TwoSidedMaterial *material_twoSided);
void sendAttribToGPU(int myType, int mySize, int  xtype, int normalized, int stride, float *pointer);
void sendClientStateToGPU(int enable, int cap);
void sendArraysToGPU (int mode, int first, int count);
void sendElementsToGPU (int mode, int count, int type, int *indices);
void render_hier(struct X3D_Group *p, int rwhat);

#endif /* __FREEWRL_SCENEGRAPH_RENDERFUNCS_H__ */
