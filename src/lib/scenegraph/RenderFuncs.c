/*
  $Id: RenderFuncs.c,v 1.112 2011/06/07 20:00:59 dug9 Exp $

  FreeWRL support library.
  Scenegraph rendering.

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
#include "../opengl/Textures.h"
#include "../scenegraph/Component_ProgrammableShaders.h"

#include "Polyrep.h"
#include "Collision.h"
#include "../scenegraph/quaternion.h"
#include "Viewer.h"
#include "LinearAlgebra.h"
#include "../input/SensInterps.h"
#include "system_threads.h"
#include "threads.h"

#include "../opengl/OpenGL_Utils.h"
#include "../scenegraph/Component_Shape.h"
#include "RenderFuncs.h"

typedef float shaderVec4[4];

///* which arrays are enabled, and defaults for each array */
//static int shaderNormalArray = TRUE;
//static int shaderVertexArray = TRUE;
//static int shaderColourArray = FALSE;
//static int shaderTextureArray = FALSE;
//
//static float light_linAtten[8];
//static float light_constAtten[8];
//static float light_quadAtten[8];
//static float light_spotCut[8];
//static float light_spotExp[8];
//static shaderVec4 light_amb[8];
//static shaderVec4 light_dif[8];
//static shaderVec4 light_pos[8];
//static shaderVec4 light_spec[8];
//static shaderVec4 light_spot[8];
//
///* Rearrange to take advantage of headlight when off */
//static int nextFreeLight = 0;
//
///* lights status. Light 7 is the headlight */
//static GLint lightOnOff[8];
//
///* should we send light changes along? */
//static bool lightStatusDirty = FALSE;
//static bool lightParamsDirty = FALSE;
//


typedef struct pRenderFuncs{
#ifdef RENDERVERBOSE
	int renderLevel;// = 0;
#endif
	/* which arrays are enabled, and defaults for each array */
	int shaderNormalArray;// = TRUE;
	int shaderVertexArray;// = TRUE;
	int shaderColourArray;// = FALSE;
	int shaderTextureArray;// = FALSE;

	float light_linAtten[8];
	float light_constAtten[8];
	float light_quadAtten[8];
	float light_spotCut[8];
	float light_spotExp[8];
	shaderVec4 light_amb[8];
	shaderVec4 light_dif[8];
	shaderVec4 light_pos[8];
	shaderVec4 light_spec[8];
	shaderVec4 light_spot[8];

	/* Rearrange to take advantage of headlight when off */
	int nextFreeLight;// = 0;

	/* lights status. Light 7 is the headlight */
	GLint lightOnOff[8];

	/* should we send light changes along? */
	bool lightStatusDirty;// = FALSE;
	bool lightParamsDirty;// = FALSE;
	int cur_hits;//=0;
	void *empty_group;//=0;
	//struct point_XYZ ht1, ht2; not used
	struct point_XYZ hyper_r1,hyper_r2; /* Transformed ray for the hypersensitive node */
	struct currayhit rayph;
	struct X3D_Group *rootNode;//=NULL;	/* scene graph root node */
	struct X3D_Anchor *AnchorsAnchor;// = NULL;
	struct currayhit rayHit,rayHitHyper;

}* ppRenderFuncs;
void *RenderFuncs_constructor(){
	void *v = malloc(sizeof(struct pRenderFuncs));
	memset(v,0,sizeof(struct pRenderFuncs));
	return v;
}
void RenderFuncs_init(struct tRenderFuncs *t){
	//public
	t->OSX_replace_world_from_console = NULL;
	t->BrowserAction = FALSE;
	//	t->hitPointDist; /* distance in ray: 0 = r1, 1 = r2, 2 = 2*r2-r1... */
	///* used to save rayhit and hyperhit for later use by C functions */
	//t->hyp_save_posn;
	//t->hyp_save_norm;t->ray_save_posn;
	t->hypersensitive = 0;
	t->hyperhit = 0;
	//private
	t->prv = RenderFuncs_constructor();
	{
		ppRenderFuncs p = (ppRenderFuncs)t->prv;
#ifdef RENDERVERBOSE
		p->renderLevel = 0;
#endif
		/* which arrays are enabled, and defaults for each array */
		p->shaderNormalArray = TRUE;
		p->shaderVertexArray = TRUE;
		p->shaderColourArray = FALSE;
		p->shaderTextureArray = FALSE;


		/* Rearrange to take advantage of headlight when off */
		p->nextFreeLight = 0;


		/* should we send light changes along? */
		p->lightStatusDirty = FALSE;
		p->lightParamsDirty = FALSE;
		p->cur_hits=0;
		p->empty_group=0;
		p->rootNode=NULL;	/* scene graph root node */
		p->AnchorsAnchor = NULL;
		t->rayHit = (void *)&p->rayHit;
		t->rayHitHyper = (void *)&p->rayHitHyper;
	}
}
//	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;




/* we assume max 8 lights. The max light is the Headlight, so we go through 0-6 for Lights */
int nextlight() {
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;
	int rv = p->nextFreeLight;
	if(p->nextFreeLight == 7) { return -1; }
	p->nextFreeLight ++;
	return rv;
}


/* keep track of lighting */
void lightState(GLint light, int status) {
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;

    PRINT_GL_ERROR_IF_ANY("start lightState");
    
    
	if (light<0) return; /* nextlight will return -1 if too many lights */
	if (p->lightOnOff[light] != status) {
		if (status) {
			/* printf ("light %d on\n",light); */
            
#ifndef GL_ES_VERSION_2_0
			FW_GL_ENABLE(GL_LIGHT0+light);
#endif /* GL_ES_VERSION_2_0 */
            
			p->lightStatusDirty = TRUE;
		} else {
			/* printf ("light %d off\n",light);  */
            
#ifndef GL_ES_VERSION_2_0
			FW_GL_DISABLE(GL_LIGHT0+light);
#endif /* GL_ES_VERSION_2_0 */
            
			p->lightStatusDirty = TRUE;
		}
		p->lightOnOff[light]=status;
	}
    PRINT_GL_ERROR_IF_ANY("end lightState");
}

/* for local lights, we keep track of what is on and off */
void saveLightState(int *ls) {
	int i;
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;
	for (i=0; i<7; i++) ls[i] = p->lightOnOff[i];
} 

void restoreLightState(int *ls) {
	int i;
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;
	for (i=0; i<7; i++) {
		if (ls[i] != p->lightOnOff[i]) {
			lightState(i,ls[i]);
		}
	}
}

void fwglLightfv (int light, int pname, GLfloat *params) {
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;
	/* printf ("fwglLightfv %d %d %f %f %f %f\n",light,pname,params[0], params[1],params[2],params[3]);  */
	#ifndef GL_ES_VERSION_2_0
		glLightfv(GL_LIGHT0+light,pname,params);
	#endif

	switch (pname) {
		case GL_AMBIENT:
			memcpy ((void *)p->light_amb[light],(void *)params,sizeof(shaderVec4));
			break;
		case GL_DIFFUSE:
			memcpy ((void *)p->light_dif[light],(void *)params,sizeof(shaderVec4));
			break;
		case GL_POSITION:
			memcpy ((void *)p->light_pos[light],(void *)params,sizeof(shaderVec4));
			break;
		case GL_SPECULAR:
			memcpy ((void *)p->light_spec[light],(void *)params,sizeof(shaderVec4));
			break;
		case GL_SPOT_DIRECTION:
			memcpy ((void *)p->light_spot[light],(void *)params,sizeof(shaderVec4));
			break;
		default: {printf ("help, unknown fwgllightfv param %d\n",pname);}
	}
	p->lightParamsDirty=TRUE;
}

void fwglLightf (int light, int pname, GLfloat param) {
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;
	//printf ("glLightf %d %d %f\n",light,pname,param);
	#ifndef GL_ES_VERSION_2_0

		glLightf(GL_LIGHT0+light,pname,param);
	#else
		printf ("skipping glLightf in fwglLightf\n");
	#endif

	switch (pname) {
		case GL_CONSTANT_ATTENUATION:
			p->light_constAtten[light] = param;
			break;
		case GL_LINEAR_ATTENUATION:
			p->light_linAtten[light] = param;
			break;
		case GL_QUADRATIC_ATTENUATION:
			p->light_quadAtten[light] = param;
			break;
		case GL_SPOT_CUTOFF:
			p->light_spotCut[light] = param;
			break;
		case GL_SPOT_EXPONENT:
			p->light_spotExp[light] = param;
			break;
		default: {printf ("help, unknown fwgllightfv param %d\n",pname);}
	}
	p->lightParamsDirty = TRUE;
}

/* send light info into Shader. if OSX gets glGetUniformBlockIndex calls, we can do this with 1 call */
void sendLightInfo (s_shader_capabilities_t *me) {
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;
		/* for debugging: */
/*
			int i;
			printf ("sendMAt - sending in lightState ");
			for (i=0; i<8; i++) printf ("%d:%d ",i,p->lightOnOff[i]); printf ("\n");
*/
		

		/* if one of these are equal to -1, we had an error in the shaders... */
		GLUNIFORM1IV(me->lightState,8,p->lightOnOff);
		GLUNIFORM4FV(me->lightAmbient,8,(float *)p->light_amb);
		GLUNIFORM4FV(me->lightDiffuse,8,(float *)p->light_dif);
		GLUNIFORM4FV(me->lightPosition,8,(float *)p->light_pos);
		GLUNIFORM4FV(me->lightSpecular,8,(float *)p->light_spec);
	}

/* finished rendering thisshape. */
void turnGlobalShaderOff(void) {
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;
	if (getAppearanceProperties()->currentShader != 0) {
		getAppearanceProperties()->currentShader = 0;
		getAppearanceProperties()->currentShaderProperties = NULL;
		USE_SHADER(0);
	}

	/* set array booleans back to defaults */
	p->shaderNormalArray = TRUE;
	p->shaderVertexArray = TRUE;
	p->shaderColourArray = FALSE;
	p->shaderTextureArray = FALSE;
}



/* choose and turn on a shader for this geometry */
void enableGlobalShader(shader_type_t requestedShader) {

	getAppearanceProperties()->currentShaderProperties = &(gglobal()->display.rdr_caps.backgroundShaderArrays[requestedShader]);
	getAppearanceProperties()->currentShader = getAppearanceProperties()->currentShaderProperties->myShaderProgram;
	USE_SHADER(getAppearanceProperties()->currentShader);

	/* send in the current position and modelview matricies */
	sendMatriciesToShader(getAppearanceProperties()->currentShaderProperties); 

}


/* send in vertices, normals, etc, etc... to either a shader or via older opengl methods */
void sendAttribToGPU(int myType, int dataSize, int dataType, int normalized, int stride, float *pointer, char *file, int line){

#ifdef RENDERVERBOSE
printf ("sendAttribToGPU, getAppearanceProperties()->currentShaderProperties %p\n",getAppearanceProperties()->currentShaderProperties);
printf ("myType %d, dataSize %d, dataType %d, stride %d\n",myType,dataSize,dataType,stride);
	if (getAppearanceProperties()->currentShaderProperties != NULL) {
		switch (myType) {
			case FW_NORMAL_POINTER_TYPE:
				printf ("glVertexAttribPointer  Normals %d at %s:%d\n",getAppearanceProperties()->currentShaderProperties->Normals,file,line);
				break;
			case FW_VERTEX_POINTER_TYPE:
				printf ("glVertexAttribPointer  Vertexs %d at %s:%d\n",getAppearanceProperties()->currentShaderProperties->Vertices,file,line);
				break;
			case FW_COLOR_POINTER_TYPE:
				printf ("glVertexAttribPointer  Colours %d at %s:%d\n",getAppearanceProperties()->currentShaderProperties->Colours,file,line);
				break;
			case FW_TEXCOORD_POINTER_TYPE:
				printf ("glVertexAttribPointer  TexCoords %d at %s:%d\n",getAppearanceProperties()->currentShaderProperties->TexCoords,file,line);
				break;

			default : {printf ("sendAttribToGPU, unknown type in shader\n");}
		}
	}
#endif

	if (getAppearanceProperties()->currentShaderProperties != NULL) {
		switch (myType) {
			case FW_NORMAL_POINTER_TYPE:
			if (getAppearanceProperties()->currentShaderProperties->Normals != -1) {
				glEnableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->Normals);
				glVertexAttribPointer(getAppearanceProperties()->currentShaderProperties->Normals, 3, dataType, normalized, stride, pointer);
			}
				break;
			case FW_VERTEX_POINTER_TYPE:
			if (getAppearanceProperties()->currentShaderProperties->Vertices != -1) {
				glEnableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->Vertices);
				glVertexAttribPointer(getAppearanceProperties()->currentShaderProperties->Vertices, dataSize, dataType, normalized, stride, pointer);
			}
				break;
			case FW_COLOR_POINTER_TYPE:
			if (getAppearanceProperties()->currentShaderProperties->Colours != -1) {
				glEnableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->Colours);
				glVertexAttribPointer(getAppearanceProperties()->currentShaderProperties->Colours, dataSize, dataType, normalized, stride, pointer);
			}
				break;
			case FW_TEXCOORD_POINTER_TYPE:
			if (getAppearanceProperties()->currentShaderProperties->TexCoords != -1) {
				glEnableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->TexCoords);
				glVertexAttribPointer(getAppearanceProperties()->currentShaderProperties->TexCoords, dataSize, dataType, normalized, stride, pointer);
			}
				break;

			default : {printf ("sendAttribToGPU, unknown type in shader\n");}
		}

	/* not shaders; older style of rendering */
	} else {
		#ifndef GL_ES_VERSION_2_0
		switch (myType) {
			case FW_VERTEX_POINTER_TYPE:
				glVertexPointer(dataSize, dataType, stride, pointer); 
				break;
			case FW_NORMAL_POINTER_TYPE:
				glNormalPointer(dataType,stride,pointer);
				break;
			case FW_COLOR_POINTER_TYPE:
				glColorPointer(dataSize, dataType, stride, pointer); 
				break;
			case FW_TEXCOORD_POINTER_TYPE:
				glTexCoordPointer(dataSize, dataType, stride, pointer);
				break;
			default : {printf ("sendAttribToGPU, unknown type in shader\n");}
		}
		#else
		printf ("not shaders for pointers\n");
		#endif


	}
}

void sendClientStateToGPU(int enable, int cap) {
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;
	if (getAppearanceProperties()->currentShaderProperties != NULL) {
		switch (cap) {
			case GL_NORMAL_ARRAY:
				p->shaderNormalArray = enable;
				break;
			case GL_VERTEX_ARRAY:
				p->shaderVertexArray = enable;
				break;
			case GL_COLOR_ARRAY:
				p->shaderColourArray = enable;
				break;
			case GL_TEXTURE_COORD_ARRAY:
				p->shaderTextureArray = enable;
				break;

			default : {printf ("sendAttribToGPU, unknown type in shader\n");}
		}
#ifdef RENDERVERBOSE
printf ("sendClientStateToGPU: getAppearanceProperties()->currentShaderProperties %p \n",getAppearanceProperties()->currentShaderProperties);
if (p->shaderNormalArray) printf ("enabling Normal\n"); else printf ("disabling Normal\n");
if (p->shaderVertexArray) printf ("enabling Vertex\n"); else printf ("disabling Vertex\n");
if (p->shaderColourArray) printf ("enabling Colour\n"); else printf ("disabling Colour\n");
if (p->shaderTextureArray) printf ("enabling Texture\n"); else printf ("disabling Texture\n");
#endif

	} else {
		#ifndef GL_ES_VERSION_2_0
			if (enable) glEnableClientState(cap);
			else glDisableClientState(cap);
		#else
			printf ("sendClientStateToGPU - currentShaderProperties not set\n");
		#endif
	}
}

void sendBindBufferToGPU (GLenum target, GLuint buffer, char *file, int line) {

	
/*
	if (target == GL_ARRAY_BUFFER_BINDING) printf ("glBindBuffer, GL_ARRAY_BUFFER_BINDING %d at %s:%d\n",buffer,file,line);
	else if (target == GL_ARRAY_BUFFER) printf ("glBindBuffer, GL_ARRAY_BUFFER %d at %s:%d\n",buffer,file,line);
	else if (target == GL_ELEMENT_ARRAY_BUFFER) printf ("glBindBuffer, GL_ELEMENT_ARRAY_BUFFER %d at %s:%d\n",buffer,file,line);
	else printf ("glBindBuffer, %d %d at %s:%d\n",target,buffer,file,line);
	
*/

	glBindBuffer(target,buffer);
}


void sendArraysToGPU (int mode, int first, int count) {
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;
	#ifdef RENDERVERBOSE
	printf ("sendArraysToGPU; getAppearanceProperties()->currentShaderProperties %p (true %d) normal %d vertex %d colour %d texture %d\n",
	getAppearanceProperties()->currentShaderProperties,TRUE, p->shaderNormalArray,p->shaderVertexArray,p->shaderColourArray,p->shaderTextureArray);
	if (p->shaderNormalArray) printf ("glEnableVertexAttribArray Normal\n"); else printf ("glDisableVertexAttribArray Normal\n");
	if (p->shaderVertexArray) printf ("glEnableVertexAttribArray Vertex\n"); else printf ("glDisableVertexAttribArray Vertex\n");
	if (p->shaderColourArray) printf ("glEnableVertexAttribArray Colour\n"); else printf ("glDisableVertexAttribArray Colour\n");
	if (p->shaderTextureArray) printf ("glEnableVertexAttribArray Texture\n"); else printf ("glDisableVertexAttribArray Texture\n");
	printf ("calling glDrawArrays, mode %d, first %d, count %d\n",mode,first,count);
	#endif


	if (getAppearanceProperties()->currentShaderProperties != NULL) {

		/* if we had a shader compile problem, do not draw */
		if (!(getAppearanceProperties()->currentShaderProperties->compiledOK)) {
			#ifdef RENDERVERBOSE
			printf ("shader compile error\n");
			#endif
			return;
		}


	if (getAppearanceProperties()->currentShaderProperties->Normals != -1) {
			if (p->shaderNormalArray) glEnableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->Normals);
			else glDisableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->Normals);
	}

	if (getAppearanceProperties()->currentShaderProperties->Vertices != -1) {
			if (p->shaderVertexArray) glEnableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->Vertices);
			else glDisableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->Vertices);
	}

	if (getAppearanceProperties()->currentShaderProperties->Colours != -1) {
			if (p->shaderColourArray) glEnableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->Colours);
			else glDisableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->Colours);
	}

	if (getAppearanceProperties()->currentShaderProperties->TexCoords != -1) {
			if (p->shaderTextureArray) glEnableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->TexCoords);
			else glDisableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->TexCoords);
	} 

	}
	glDrawArrays(mode,first,count);
}

void sendElementsToGPU (int mode, int count, int type, int *indices) {
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;
	#ifdef RENDERVERBOSE
	printf ("sendElementsToGPU start\n"); 
	#endif
    
	if (getAppearanceProperties()->currentShaderProperties != NULL) {

		/* if we had a shader compile problem, do not draw */
		if (!(getAppearanceProperties()->currentShaderProperties->compiledOK)) {
			#ifdef RENDERVERBOSE
			printf ("shader compile error\n");
			#endif
			return;
		}

	#ifdef RENDERVERBOSE
        	printf ("we have Normals %d Vertices %d Colours %d TexCoords %d \n",
                getAppearanceProperties()->currentShaderProperties->Normals,
                getAppearanceProperties()->currentShaderProperties->Vertices,
                getAppearanceProperties()->currentShaderProperties->Colours,
                getAppearanceProperties()->currentShaderProperties->TexCoords);
		if (p->shaderNormalArray) printf ("p->shaderNormalArray TRUE\n"); else printf ("p->shaderNormalArray FALSE\n");        
	        if (p->shaderVertexArray) printf ("shaderVertexArray TRUE\n"); else printf ("shaderVertexArray FALSE\n");
	        if (p->shaderColourArray) printf ("shaderColourArray TRUE\n"); else printf ("shaderColourArray FALSE\n");
		if (p->shaderTextureArray) printf ("shaderTextureArray TRUE\n"); else printf ("shaderTextureArray FALSE\n");               
	#endif
        
        
		if (getAppearanceProperties()->currentShaderProperties->Normals != -1) {
			if (p->shaderNormalArray) glEnableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->Normals);
			else glDisableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->Normals);
		}

		if (getAppearanceProperties()->currentShaderProperties->Vertices != -1) {
			if (p->shaderVertexArray) glEnableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->Vertices);
			else glDisableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->Vertices);
		}

		if (getAppearanceProperties()->currentShaderProperties->Colours != -1) {
			if (p->shaderColourArray) glEnableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->Colours);
			else glDisableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->Colours);
		}

		if (getAppearanceProperties()->currentShaderProperties->TexCoords != -1) {
			if (p->shaderTextureArray) glEnableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->TexCoords);
			else glDisableVertexAttribArray(getAppearanceProperties()->currentShaderProperties->TexCoords);
		}

	}

	glDrawElements(mode,count,type,indices);

	#ifdef RENDERVERBOSE
	printf ("sendElementsToGPU finish\n"); 
	#endif
}


void initializeLightTables() {
	int i;
        float pos[] = { 0.0f, 0.0f, 1.0f, 0.0f };
        float dif[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        float shin[] = { 0.6f, 0.6f, 0.6f, 1.0f };
        float As[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;

      PRINT_GL_ERROR_IF_ANY("start of initializeightTables");

	for(i=0; i<8; i++) {
                p->lightOnOff[i] = 9999;
                lightState(i,FALSE);
            
        	FW_GL_LIGHTFV(i, GL_POSITION, pos);
        	FW_GL_LIGHTFV(i, GL_AMBIENT, As);
        	FW_GL_LIGHTFV(i, GL_DIFFUSE, dif);
        	FW_GL_LIGHTFV(i, GL_SPECULAR, shin);
            

            
            PRINT_GL_ERROR_IF_ANY("initizlizeLight2");
        }
        lightState(HEADLIGHT_LIGHT, TRUE);

	#ifndef GL_ES_VERSION_2_0
        FW_GL_LIGHTMODELI(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
        FW_GL_LIGHTMODELI(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        FW_GL_LIGHTMODELI(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
        FW_GL_LIGHTMODELI(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        FW_GL_LIGHTMODELFV(GL_LIGHT_MODEL_AMBIENT,As);
	#else
	printf ("skipping light setups\n");
	#endif

    LIGHTING_INITIALIZE
	

    PRINT_GL_ERROR_IF_ANY("end initializeLightTables");
}



int render_vp; /*set up the inverse viewmatrix of the viewpoint.*/
int render_geom;
int render_light;
int render_sensitive;
int render_blend;
int render_proximity;
int render_collision;
int render_other;
#ifdef DJTRACK_PICKSENSORS
int render_picksensors;
int render_pickables;
int	render_allAsPickables;
#endif


/* material node usage depends on texture depth; if rgb (depth1) we blend color field
   and diffusecolor with texture, else, we dont bother with material colors */
int last_texture_type = NOTEXTURE;

/* texture stuff - see code. Need array because of MultiTextures */
GLuint boundTextureStack[MAX_MULTITEXTURE];
int textureStackTop;

int	have_transparency=FALSE;/* did any Shape have transparent material? */
int	lightingOn;		/* do we need to restore lighting in Shape? */

//int cur_hits=0;


///* dimentions of viewer, and "up" vector (for collision detection) */
//struct sNaviInfo naviinfo = {0.25, 1.6, 0.75};

/* for alignment of collision cylinder, and gravity (later). */
//struct point_XYZ ViewerUpvector = {0,0,0};

//X3D_Viewer Viewer; /* has to be defined somewhere, so it found itself stuck here */

//true statics:
GLint viewport[4] = {-1,-1,2,2};  //doesn't change, used in glu unprojects
/* These two points define a ray in window coordinates */
struct point_XYZ r1 = {0,0,-1},r2 = {0,0,0},r3 = {0,1,0};


//struct point_XYZ t_r1,t_r2,t_r3; /* transformed ray */
//void *hypersensitive = 0; 
//int hyperhit = 0;
//struct point_XYZ hyper_r1,hyper_r2; /* Transformed ray for the hypersensitive node */


/* These three points define 1. hitpoint 2., 3. two different tangents
 * of the surface at hitpoint (to get transformation correctly */

/* All in window coordinates */
//struct point_XYZ hp;
//static struct point_XYZ ht1, ht2;
//double hitPointDist; /* distance in ray: 0 = r1, 1 = r2, 2 = 2*r2-r1... */
/* used to save rayhit and hyperhit for later use by C functions */
//struct SFColor hyp_save_posn, hyp_save_norm, ray_save_posn;

/* Any action for the Browser to do? */
//int BrowserAction = FALSE;
//struct X3D_Anchor *_AnchorsAnchor = NULL;
struct X3D_Anchor *AnchorsAnchor()
{
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;
	return p->AnchorsAnchor;
}
void setAnchorsAnchor(struct X3D_Anchor* anchor)
{
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;
	p->AnchorsAnchor = anchor;
}

//char *OSX_replace_world_from_console = NULL;


//static struct currayhit rayph;
//struct currayhit rayHit,rayHitHyper;
/* used to test new hits */



//struct X3D_Group *_rootNode=NULL;	/* scene graph root node */
struct X3D_Group *rootNode()
{
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;	
	return p->rootNode;
}
void setRootNode(struct X3D_Group *rn)
{
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;
	p->rootNode = rn;
}
//void *empty_group=0;

/*******************************************************************************/

/* Sub, rather than big macro... */
void rayhit(float rat, float cx,float cy,float cz, float nx,float ny,float nz,
	    float tx,float ty, char *descr)  {
	GLDOUBLE modelMatrix[16];
	GLDOUBLE projMatrix[16];
	ppRenderFuncs p;
	ttglobal tg = gglobal();
	p = (ppRenderFuncs)tg->RenderFuncs.prv;

	/* Real rat-testing */
#ifdef RENDERVERBOSE
	printf("RAY HIT %s! %f (%f %f %f) (%f %f %f)\n\tR: (%f %f %f) (%f %f %f)\n",
	       descr, rat,cx,cy,cz,nx,ny,nz,
	       t_r1.x, t_r1.y, t_r1.z,
	       t_r2.x, t_r2.y, t_r2.z
		);
#endif

	if(rat<0 || (rat>tg->RenderFuncs.hitPointDist && tg->RenderFuncs.hitPointDist >= 0)) {
		return;
	}
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix);
	FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, projMatrix);
	FW_GLU_PROJECT(cx,cy,cz, modelMatrix, projMatrix, viewport, &tg->RenderFuncs.hp.x, &tg->RenderFuncs.hp.y, &tg->RenderFuncs.hp.z);
	tg->RenderFuncs.hitPointDist = rat;
	p->rayHit=p->rayph;
	p->rayHitHyper=p->rayph;
#ifdef RENDERVERBOSE 
	printf ("Rayhit, hp.x y z: - %f %f %f rat %f hitPointDist %f\n",hp.x,hp.y,hp.z, rat, tg->RenderFuncs.hitPointDist);
#endif
}


/* Call this when modelview and projection modified */
void upd_ray() {
	struct point_XYZ t_r1,t_r2,t_r3;
	GLDOUBLE modelMatrix[16];
	GLDOUBLE projMatrix[16];
	ttglobal tg = gglobal();
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix);
	FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, projMatrix);
/*

{int i; printf ("\n"); 
printf ("upd_ray, pm %p\n",projMatrix);
for (i=0; i<16; i++) printf ("%4.3lf ",modelMatrix[i]); printf ("\n"); 
for (i=0; i<16; i++) printf ("%4.3lf ",projMatrix[i]); printf ("\n"); 
} 
*/


	FW_GLU_UNPROJECT(r1.x,r1.y,r1.z,modelMatrix,projMatrix,viewport,
		     &t_r1.x,&t_r1.y,&t_r1.z);
	FW_GLU_UNPROJECT(r2.x,r2.y,r2.z,modelMatrix,projMatrix,viewport,
		     &t_r2.x,&t_r2.y,&t_r2.z);
	FW_GLU_UNPROJECT(r3.x,r3.y,r3.z,modelMatrix,projMatrix,viewport,
		     &t_r3.x,&t_r3.y,&t_r3.z);

	VECCOPY(tg->RenderFuncs.t_r1,t_r1);
	VECCOPY(tg->RenderFuncs.t_r2,t_r2);
	VECCOPY(tg->RenderFuncs.t_r3,t_r3);

/*
	printf("Upd_ray: (%f %f %f)->(%f %f %f) == (%f %f %f)->(%f %f %f)\n",
	r1.x,r1.y,r1.z,r2.x,r2.y,r2.z,
	t_r1.x,t_r1.y,t_r1.z,t_r2.x,t_r2.y,t_r2.z);
*/

}


/* if a node changes, void the display lists */
/* Courtesy of Jochen Hoenicke */

void update_node(struct X3D_Node *node) {
	int i;
	ppRenderFuncs p = (ppRenderFuncs)gglobal()->RenderFuncs.prv;

#ifdef VERBOSE
	printf ("update_node for %d %s nparents %d renderflags %x\n",node, stringNodeType(node->_nodeType),node->_nparents, node->_renderFlags); 
	if (node->_nparents == 0) {
		if (node == rootNode) printf ("...no parents, this IS the rootNode\n"); 
		else printf ("...no parents, this IS NOT the rootNode\n");
	}
	for (i = 0; i < node->_nparents; i++) {
		struct X3D_Node *n = X3D_NODE(node->_parents[i]);
		if( n != 0 ) {
			printf ("	parent %u is %s\n",n,stringNodeType(n->_nodeType));
		} else {
			printf ("	parent %d is NULL\n",i);
		}
	}
#endif

	node->_change ++;
	for (i = 0; i < node->_nparents; i++) {
		struct X3D_Node *n = X3D_NODE(node->_parents[i]);
		if(n == node) {
			fprintf(stderr, "Error: self-referential node structure! (node:'%s')\n", stringNodeType(node->_nodeType));
			node->_parents[i] = p->empty_group;
		} else if( n != 0 ) {
			update_node(n);
		}
	}
}

/*********************************************************************
 *********************************************************************
 *
 * render_node : call the correct virtual functions to render the node
 * depending on what we are doing right now.
 */

//#ifdef RENDERVERBOSE
//static int renderLevel = 0;
//#endif

#define PRINT_NODE(_node, _v)  do {					\
		if (gglobal()->internalc.global_print_opengl_errors && (gglobal()->display._global_gl_err != GL_NO_ERROR)) { \
			printf("Render_node_v %p (%s) PREP: %p REND: %p CH: %p FIN: %p RAY: %p HYP: %p\n",_v, \
			       stringNodeType(_node->_nodeType),	\
			       _v->prep,				\
			       _v->rend,				\
			       _v->children,			\
			       _v->fin,				\
			       _v->rendray,			\
			       gglobal()->RenderFuncs.hypersensitive);			\
			printf("Render_state geom %d light %d sens %d\n", \
			       render_geom,				\
			       render_light,				\
			       render_sensitive);			\
			printf("pchange %d pichange %d \n", _node->_change, _node->_ichange); \
		}							\
	} while (0)


void render_node(struct X3D_Node *node) {
	struct X3D_Virt *virt;

	int srg = 0;
	int sch = 0;
	struct currayhit srh;
	ppRenderFuncs p;
	ttglobal tg = gglobal();
	p = (ppRenderFuncs)tg->RenderFuncs.prv;

	X3D_NODE_CHECK(node);

#ifdef RENDERVERBOSE
	renderLevel ++;
#endif

	if(!node) {
#ifdef RENDERVERBOSE
		DEBUG_RENDER("%d no node, quick return\n", renderLevel);
		renderLevel--;
#endif
		return;
	}

	virt = virtTable[node->_nodeType];

#ifdef RENDERVERBOSE 
	//printf("%d =========================================NODE RENDERED===================================================\n",renderLevel);
	{
		int i;
		for(i=0;i<renderLevel;i++) printf(" ");
	}
	printf("%d node %u (%s) , v %u renderFlags %x ",renderLevel, node,stringNodeType(node->_nodeType),virt,node->_renderFlags);

	if ((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) printf (" VF_Viewpoint");
	if ((node->_renderFlags & VF_Geom )== VF_Geom) printf (" VF_Geom");
	if ((node->_renderFlags & VF_localLight )== VF_localLight) printf (" VF_localLight");
	if ((node->_renderFlags & VF_Sensitive) == VF_Sensitive) printf (" VF_Sensitive");
	if ((node->_renderFlags & VF_Blend) == VF_Blend) printf (" VF_Blend");
	if ((node->_renderFlags & VF_Proximity) == VF_Proximity) printf (" VF_Proximity");
	if ((node->_renderFlags & VF_Collision) == VF_Collision) printf (" VF_Collision");
	if ((node->_renderFlags & VF_globalLight) == VF_globalLight) printf (" VF_globalLight");
	if ((node->_renderFlags & VF_hasVisibleChildren) == VF_hasVisibleChildren) printf (" VF_hasVisibleChildren");
	if ((node->_renderFlags & VF_shouldSortChildren) == VF_shouldSortChildren) printf (" VF_shouldSortChildren");
	if ((node->_renderFlags & VF_Other) == VF_Other) printf (" VF_Other");
	/*
	if ((node->_renderFlags & VF_inPickableGroup == VF_inPickableGroup) printf (" VF_inPickableGroup");
	if ((node->_renderFlags & VF_PickingSensor == VF_PickingSensor) printf (" VF_PickingSensor");
	*/
	printf ("\n");

	//printf("PREP: %d REND: %d CH: %d FIN: %d RAY: %d HYP: %d\n",virt->prep, virt->rend, virt->children, virt->fin,
	//       virt->rendray, hypersensitive);
	//printf("%d state: vp %d geom %d light %d sens %d blend %d prox %d col %d ", renderLevel, 
 //        	render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); 
	//printf("change %d ichange %d \n",node->_change, node->_ichange);
#endif

        /* if we are doing Viewpoints, and we don't have a Viewpoint, don't bother doing anything here */ 
        if (render_vp == VF_Viewpoint) { 
                if ((node->_renderFlags & VF_Viewpoint) != VF_Viewpoint) { 
#ifdef RENDERVERBOSE
                        printf ("doing Viewpoint, but this  node is not for us - just returning\n"); 
			renderLevel--;
#endif
                        return; 
                } 
        }

	/* are we working through global PointLights, DirectionalLights or SpotLights, but none exist from here on down? */
        if (render_light == VF_globalLight) { 
                if ((node->_renderFlags & VF_globalLight) != VF_globalLight) { 
#ifdef RENDERVERBOSE
                        printf ("doing globalLight, but this  node is not for us - just returning\n"); 
			renderLevel--;
#endif
                        return; 
                }
        }

	if(virt->prep) {
		DEBUG_RENDER("rs 2\n");
		virt->prep(node);
		if(render_sensitive && !tg->RenderFuncs.hypersensitive) {
			upd_ray();
		}
		PRINT_GL_ERROR_IF_ANY("prep"); PRINT_NODE(node,virt);
	}
	if(render_proximity && virt->proximity) {
		DEBUG_RENDER("rs 2a\n");
		virt->proximity(node);
		PRINT_GL_ERROR_IF_ANY("render_proximity"); PRINT_NODE(node,virt);
	}
	
	if(render_collision && virt->collision) {
		DEBUG_RENDER("rs 2b\n");
		virt->collision(node);
		PRINT_GL_ERROR_IF_ANY("render_collision"); PRINT_NODE(node,virt);
	}

	if(render_geom && !render_sensitive && virt->rend) {
		DEBUG_RENDER("rs 3\n");
		virt->rend(node);
		PRINT_GL_ERROR_IF_ANY("render_geom"); PRINT_NODE(node,virt);
	}

	if(render_other && virt->other )
	{
#if DJTRACK_PICKSENSORS
		DEBUG_RENDER("rs 4a\n");
		virt->other(node); //other() is responsible for push_renderingState(VF_inPickableGroup);
		PRINT_GL_ERROR_IF_ANY("render_other"); PRINT_NODE(node,virt);
#endif
	} //other

	if(render_sensitive && (node->_renderFlags & VF_Sensitive)) {
		DEBUG_RENDER("rs 5\n");
		srg = render_geom;
		render_geom = 1;
		DEBUG_RENDER("CH1 %d: %d\n",node, p->cur_hits, node->_hit);
		sch = p->cur_hits;
		p->cur_hits = 0;
		/* HP */
		srh = p->rayph;
		p->rayph.hitNode = node;
		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, p->rayph.modelMatrix);
		FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, p->rayph.projMatrix);
		PRINT_GL_ERROR_IF_ANY("render_sensitive"); PRINT_NODE(node,virt);
	}

	if(render_geom && render_sensitive && !tg->RenderFuncs.hypersensitive && virt->rendray) {
		DEBUG_RENDER("rs 6\n");
		virt->rendray(node);
		PRINT_GL_ERROR_IF_ANY("rs 6"); PRINT_NODE(node,virt);
	}

    if((render_sensitive) && (tg->RenderFuncs.hypersensitive == node)) {
		DEBUG_RENDER("rs 7\n");
		p->hyper_r1 = tg->RenderFuncs.t_r1;
		p->hyper_r2 = tg->RenderFuncs.t_r2;
		tg->RenderFuncs.hyperhit = 1;
    }

	/* start recursive section */
    if(virt->children) { 
		DEBUG_RENDER("rs 8 - has valid child node pointer\n");
		virt->children(node);
		PRINT_GL_ERROR_IF_ANY("children"); PRINT_NODE(node,virt);
    }
	/* end recursive section */

	if(render_other && virt->other)
	{
#if DJTRACK_PICKSENSORS
		//pop_renderingState(VF_inPickableGroup);
#endif
	}

	if(render_sensitive && (node->_renderFlags & VF_Sensitive)) {
		DEBUG_RENDER("rs 9\n");
		render_geom = srg;
		p->cur_hits = sch;
		DEBUG_RENDER("CH3: %d %d\n",p->cur_hits, node->_hit);
		/* HP */
		p->rayph = srh;
	}

	if(virt->fin) {
		DEBUG_RENDER("rs A\n");
		virt->fin(node);
		if(render_sensitive && virt == &virt_Transform) {
			upd_ray();
		}
		PRINT_GL_ERROR_IF_ANY("fin"); PRINT_NODE(node,virt);
	}

#ifdef RENDERVERBOSE 
	{
		int i;
		for(i=0;i<renderLevel;i++)printf(" ");
	}
	printf("%d (end render_node)\n",renderLevel);
	renderLevel--;
#endif
}


/*
 * The following code handles keeping track of the parents of a given
 * node. This enables us to traverse the scene on C level for optimizations.
 *
 * We use this array code because VRML nodes usually don't have
 * hundreds of children and don't usually shuffle them too much.
 */

void add_parent(struct X3D_Node *node, struct X3D_Node *parent, char *file, int line) {
	int oldparcount;

	if(!node) return;

#ifdef CHILDVERBOSE
	printf ("add_parent; adding node %u ,to parent %u at %s:%d\n",node,  parent,file,line);
	printf ("add_parent; adding node %x ,to parent %x (hex) at %s:%d\n",node,  parent,file,line);
	printf ("add_parent; adding node %p ,to parent %p (ptr) at %s:%d\n",node,  parent,file,line);


	printf ("add_parent; adding node %u (%s) to parent %u (%s) at %s:%d\n",node, stringNodeType(node->_nodeType), 
		parent, stringNodeType(parent->_nodeType),file,line);
#endif

	parent->_renderFlags = parent->_renderFlags | node->_renderFlags;

	oldparcount = node->_nparents;
	if((oldparcount+1) > node->_nparalloc) {
		node->_nparents = 0; /* for possible threading issues */
		node->_nparalloc += 10;
		if (node->_parents == NULL)  {
			node->_parents = MALLOC(void **, sizeof(node->_parents[0])* node->_nparalloc) ;
		} else {
			node->_parents = (void **)REALLOC(node->_parents, sizeof(node->_parents[0])*
							  node->_nparalloc) ;
		}
	}
	node->_parents[oldparcount] = parent;
	node->_nparents = oldparcount+1;

	/* tie in sensitive nodes */
	setSensitive (parent, node);
}

void remove_parent(struct X3D_Node *child, struct X3D_Node *parent) {
	int i;
	int pi;

	if(!child) return;
	if(!parent) return;
	
#ifdef CHILDVERBOSE
	printf ("remove_parent, parent %u (%s) , child %u (%s)\n",parent, stringNodeType(parent->_nodeType),
		child, stringNodeType(child->_nodeType));
#endif

	/* find the index of this parent in this child. */
	pi = -1;

	for(i=0; i<child->_nparents; i++) {
		/* printf ("comparing %u and %u\n",child->_parents[i], parent); */
		if(child->_parents[i] == parent) {
			pi = i;
			break;
		}
	}

	if (pi < 0) return; /* child does not have this parent - removed already?? anyway... */

	/* The order of parents does not matter. Instead of moving the whole
	 * block of data after the current position, we simply swap the one to
	 * delete at the end and do a vector pop_back (decrease nparents, which
	 * has already happened).
	 */

#ifdef CHILDVERBOSE
	printf ("remove_parent, pi %d, index at end %d\n",pi, child->_nparents-1);
#endif

	child->_parents[pi]=child->_parents[child->_nparents-1];
	child->_nparents--;

#ifdef CHILDVERBOSE
	if (child->_nparents == -1) {
		printf ("remove_parent, THIS NODE HAS NO PARENTS...\n");
	}
#endif
}

void
render_hier(struct X3D_Group *g, int rwhat) {
	/// not needed now - see below struct point_XYZ upvec = {0,1,0};
	/// not needed now - see below GLDOUBLE modelMatrix[16];

#ifdef render_pre_profile
	/*  profile */
	double xx,yy,zz,aa,bb,cc,dd,ee,ff;
	struct timeval mytime;
	struct timezone tz; /* unused see man gettimeofday */
#endif
	ppRenderFuncs p;
	ttglobal tg = gglobal();
	p = (ppRenderFuncs)tg->RenderFuncs.prv;


	render_vp = rwhat & VF_Viewpoint;
	render_geom =  rwhat & VF_Geom;
	render_light = rwhat & VF_globalLight;
	render_sensitive = rwhat & VF_Sensitive;
	render_blend = rwhat & VF_Blend;
	render_proximity = rwhat & VF_Proximity;
	render_collision = rwhat & VF_Collision;
	render_other = rwhat & VF_Other;
#ifdef DJTRACK_PICKSENSORS
	render_picksensors = rwhat & VF_PickingSensor;
	render_pickables = rwhat & VF_inPickableGroup;
#endif
	p->nextFreeLight = 0;
	tg->RenderFuncs.hitPointDist = -1;


#ifdef render_pre_profile
	if (render_geom) {
		gettimeofday (&mytime,&tz);
		aa = (double)mytime.tv_sec+(double)mytime.tv_usec/1000000.0;
	}
#endif

	/* printf ("render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	   render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision);  */

	if (!g) {
		/* we have no geometry yet, sleep for a tiny bit */
		usleep(1000);
		return;
	}

#ifdef RENDERVERBOSE
	printf("Render_hier node=%d what=%d\n", g, rwhat);
#endif

#ifdef render_pre_profile
	if (render_geom) {
		gettimeofday (&mytime,&tz);
		bb = (double)mytime.tv_sec+(double)mytime.tv_usec/1000000.0;
	}
#endif

	if (render_sensitive) {
		upd_ray();
	}

#ifdef render_pre_profile
	if (render_geom) {
		gettimeofday (&mytime,&tz);
		cc = (double)mytime.tv_sec+(double)mytime.tv_usec/1000000.0;
	}
#endif

	render_node(X3D_NODE(g));

#ifdef render_pre_profile
	if (render_geom) {
		gettimeofday (&mytime,&tz);
		dd = (double)mytime.tv_sec+(double)mytime.tv_usec/1000000.0;
		printf ("render_geom status %f ray %f geom %f\n",bb-aa, cc-bb, dd-cc);
	}
#endif
}


/******************************************************************************
 *
 * shape compiler "thread"
 *
 ******************************************************************************/

void compileNode (void (*nodefn)(void *, void *, void *, void *, void *), void *node, void *Icoord, void *Icolor, void *Inormal, void *ItexCoord) {
	void *coord; void *color; void *normal; void *texCoord;

	/* are any of these SFNodes PROTOS? If so, get the underlying real node, as PROTOS are handled like Groups. */
	POSSIBLE_PROTO_EXPANSION(void *, Icoord,coord)
		POSSIBLE_PROTO_EXPANSION(void *, Icolor,color)
		POSSIBLE_PROTO_EXPANSION(void *, Inormal,normal)
		POSSIBLE_PROTO_EXPANSION(void *, ItexCoord,texCoord)

	nodefn(node, coord, color, normal, texCoord);
}


/* for CRoutes, we need to have a function pointer to an interpolator to run, if we
   route TO an interpolator */
void *returnInterpolatorPointer (const char *x) {
	if (strcmp("OrientationInterpolator",x)==0) { return (void *)do_Oint4;
	} else if (strcmp("CoordinateInterpolator2D",x)==0) { return (void *)do_OintCoord2D;
	} else if (strcmp("PositionInterpolator2D",x)==0) { return (void *)do_OintPos2D;
	} else if (strcmp("ScalarInterpolator",x)==0) { return (void *)do_OintScalar;
	} else if (strcmp("ColorInterpolator",x)==0) { return (void *)do_ColorInterpolator;
	} else if (strcmp("PositionInterpolator",x)==0) { return (void *)do_PositionInterpolator;
	} else if (strcmp("CoordinateInterpolator",x)==0) { return (void *)do_OintCoord;
	} else if (strcmp("NormalInterpolator",x)==0) { return (void *)do_OintNormal;
	} else if (strcmp("GeoPositionInterpolator",x)==0) { return (void *)do_GeoPositionInterpolator;
	} else if (strcmp("BooleanFilter",x)==0) { return (void *)do_BooleanFilter;
	} else if (strcmp("BooleanSequencer",x)==0) { return (void *)do_BooleanSequencer;
	} else if (strcmp("BooleanToggle",x)==0) { return (void *)do_BooleanToggle;
	} else if (strcmp("BooleanTrigger",x)==0) { return (void *)do_BooleanTrigger;
	} else if (strcmp("IntegerTrigger",x)==0) { return (void *)do_IntegerTrigger;
	} else if (strcmp("IntegerSequencer",x)==0) { return (void *)do_IntegerSequencer;
	} else if (strcmp("TimeTrigger",x)==0) { return (void *)do_TimeTrigger;
	
	} else {
		return 0;
	}
}




void checkParentLink (struct X3D_Node *node,struct X3D_Node *parent) {
        int n;

	int *offsetptr;
	char *memptr;
	struct Multi_Node *mfn;
	uintptr_t *voidptr;

        if (node == NULL) return;

	/* printf ("checkParentLink for node %u parent %u type %s\n",node,parent,stringNodeType(node->_nodeType)); */
 
        if (parent != NULL) ADD_PARENT(node, parent);

	if ((node->_nodeType<0) || (node->_nodeType>NODES_COUNT)) {
		ConsoleMessage ("checkParentLink - %d not a valid nodeType",node->_nodeType);
		return;
	}

	/* find all the fields of this node */
	offsetptr = (int *) NODE_OFFSETS[node->_nodeType];

	/* FIELDNAMES_bboxCenter, offsetof (struct X3D_Group, bboxCenter),  FIELDTYPE_SFVec3f, KW_field, */
	while (*offsetptr >= 0) {

		/* 
		   printf ("	field %s",FIELDNAMES[offsetptr[0]]); 
		   printf ("	offset %d",offsetptr[1]);
		   printf ("	type %s",FIELDTYPES[offsetptr[2]]);
		   printf ("	kind %s\n",KEYWORDS[offsetptr[3]]);
		*/

		/* worry about SFNodes and MFNodes */
		if ((offsetptr[2] == FIELDTYPE_SFNode) || (offsetptr[2] == FIELDTYPE_MFNode)) {
			if ((offsetptr[3] == KW_initializeOnly) || (offsetptr[3] == KW_inputOutput)) {

				/* create a pointer to the actual field */
				memptr = (char *) node;
				memptr += offsetptr[1];

				if (offsetptr[2] == FIELDTYPE_SFNode) {
					/* get the field as a POINTER VALUE, not just a pointer... */
					voidptr = (uintptr_t *) memptr;
					voidptr = (uintptr_t *) *voidptr;

					/* is there a node here? */
					if (voidptr != NULL) {
						checkParentLink(X3D_NODE(voidptr),node);
					}
				} else {
					mfn = (struct Multi_Node*) memptr;
					/* printf ("MFNode has %d children\n",mfn->n); */
					for (n=0; n<mfn->n; n++) {
				                checkParentLink(mfn->p[n],node);
					}
				}
			}

		}
		offsetptr+=5;
	}
}

#define X3D_COORD(node) ((struct X3D_Coordinate*)node)
#define X3D_GEOCOORD(node) ((struct X3D_GeoCoordinate*)node)

/* get a coordinate array - (SFVec3f) from either a NODE_Coordinate or NODE_GeoCoordinate */
struct Multi_Vec3f *getCoordinate (struct X3D_Node *innode, char *str) {
	struct X3D_Coordinate * xc;
	struct X3D_GeoCoordinate *gxc;
	struct X3D_Node *node;

	POSSIBLE_PROTO_EXPANSION (struct X3D_Node *, innode,node)

		xc = X3D_COORD(node);
	/* printf ("getCoordinate, have a %s\n",stringNodeType(xc->_nodeType)); */

	if (xc->_nodeType == NODE_Coordinate) {
		return &(xc->point);
	} else if (xc->_nodeType == NODE_GeoCoordinate) {
		COMPILE_IF_REQUIRED_RETURN_NULL_ON_ERROR;
		gxc = X3D_GEOCOORD(node);
		return &(gxc->__movedCoords);
	} else {
		ConsoleMessage ("%s - coord expected but got %s\n", stringNodeType(xc->_nodeType));
	}
	return NULL;
}


