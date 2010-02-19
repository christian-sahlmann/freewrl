/*
  $Id: RenderFuncs.c,v 1.44 2010/02/19 14:42:21 crc_canada Exp $

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

#include "RenderFuncs.h"

typedef float shaderVec4[4];

static float light_linAtten[8];
static float light_constAtten[8];
static float light_quadAtten[8];
static float light_spotCut[8];
static float light_spotExp[8];
static shaderVec4 light_amb[8];
static shaderVec4 light_dif[8];
static shaderVec4 light_pos[8];
static shaderVec4 light_spec[8];
static shaderVec4 light_spot[8];

/* Rearrange to take advantage of headlight when off */
static int nextFreeLight = 0;

/* lights status. Light 7 is the headlight */
static int lights[8];

/* which shader is currently selected (general appearance shaders) */
static s_shader_capabilities_t *currentShaderStruct = NULL;

/* should we send light changes along? */
static bool lightStatusDirty = FALSE;
static bool lightParamsDirty = FALSE;

/* we assume max 8 lights. The max light is the Headlight, so we go through 0-6 for Lights */
int nextlight() {
	int rv = nextFreeLight;
	if(nextFreeLight == 7) { return -1; }
	nextFreeLight ++;
	return rv;
}


/* keep track of lighting */
void lightState(GLint light, int status) {
	if (light<0) return; /* nextlight will return -1 if too many lights */
	if (lights[light] != status) {
		if (status) {
			/* printf ("light %d on\n",light);  */
			FW_GL_ENABLE(GL_LIGHT0+light);
			lightStatusDirty = TRUE;
		} else {
			/* printf ("light %d off\n",light);  */
			FW_GL_DISABLE(GL_LIGHT0+light);
			lightStatusDirty = TRUE;
		}
		lights[light]=status;
	}
}

/* for local lights, we keep track of what is on and off */
void saveLightState(int *ls) {
	int i;
	for (i=0; i<7; i++) ls[i] = lights[i];
} 

void restoreLightState(int *ls) {
	int i;
	for (i=0; i<7; i++) {
		if (ls[i] != lights[i]) {
			lightState(i,ls[i]);
		}
	}
}

void fwglLightfv (int light, int pname, GLfloat *params) {
	/* printf ("glLightfv %d %d %f %f %f %f\n",light,pname,params[0], params[1],params[2],params[3]);  */
	glLightfv(GL_LIGHT0+light,pname,params);
	switch (pname) {
		case GL_AMBIENT:
			memcpy ((void *)light_amb[light],(void *)params,sizeof(shaderVec4));
			break;
		case GL_DIFFUSE:
			memcpy ((void *)light_dif[light],(void *)params,sizeof(shaderVec4));
			break;
		case GL_POSITION:
			memcpy ((void *)light_pos[light],(void *)params,sizeof(shaderVec4));
			break;
		case GL_SPECULAR:
			memcpy ((void *)light_spec[light],(void *)params,sizeof(shaderVec4));
			break;
		case GL_SPOT_DIRECTION:
			memcpy ((void *)light_spot[light],(void *)params,sizeof(shaderVec4));
			break;
		default: {printf ("help, unknown fwgllightfv param %d\n",pname);}
	}
	lightParamsDirty=TRUE;
}

void fwglLightf (int light, int pname, GLfloat param) {
	/* printf ("glLightf %d %d %f\n",light,pname,param);  */
	glLightf(GL_LIGHT0+light,pname,param);
	switch (pname) {
		case GL_CONSTANT_ATTENUATION:
			light_constAtten[light] = param;
			break;
		case GL_LINEAR_ATTENUATION:
			light_linAtten[light] = param;
			break;
		case GL_QUADRATIC_ATTENUATION:
			light_quadAtten[light] = param;
			break;
		case GL_SPOT_CUTOFF:
			light_spotCut[light] = param;
			break;
		case GL_SPOT_EXPONENT:
			light_spotExp[light] = param;
			break;
		default: {printf ("help, unknown fwgllightfv param %d\n",pname);}
	}
	lightParamsDirty = TRUE;
}

void chooseAppearanceShader(struct X3D_Material *material_oneSided, struct X3D_TwoSidedMaterial *material_twoSided) {
	shader_type_t whichShader;

	/* no appearance - just grey lighting */
	whichShader = noAppearanceNoMaterialShader;

	if (material_oneSided != NULL) {
		/* is the headlight on? */
		if (lights[HEADLIGHT_LIGHT]) {
			whichShader = genericHeadlightNoTextureAppearanceShader;
		} else {
			whichShader = noLightNoTextureAppearanceShader;
		}
	} else if (material_twoSided != NULL) {
	}
	
	/* are we no headlight, noAppearanceNoMaterialShader? */


	currentShaderStruct = &(rdr_caps.shaderArrays[whichShader]);
	globalCurrentShader = currentShaderStruct->myShaderProgram;
	USE_SHADER(globalCurrentShader);

	switch (whichShader) {
		case noAppearanceNoMaterialShader:
			break;
		case noLightNoTextureAppearanceShader:
			/* send up material selection to shader */
			glUniform4fv(currentShaderStruct->myMaterialEmission,1,material_oneSided->_ecol.c);
			break;
		case genericHeadlightNoTextureAppearanceShader:
			/* send up material selection to shader */

			glUniform4fv(currentShaderStruct->myMaterialAmbient,1,material_oneSided->_amb.c);
			glUniform4fv(currentShaderStruct->myMaterialDiffuse,1,material_oneSided->_dcol.c);
			glUniform4fv(currentShaderStruct->myMaterialSpecular,1,material_oneSided->_scol.c);
			glUniform1f(currentShaderStruct->myMaterialShininess,material_oneSided->_shin);
			glUniform4fv(currentShaderStruct->myMaterialEmission,1,material_oneSided->_ecol.c);
			break;
		default: {}; /* invalid shader here... */
	}

	lightParamsDirty = FALSE;
	lightStatusDirty = FALSE;
}


void initializeLightTables() {
        float pos[] = { 0.0, 0.0, 1.0, 0.0 };
        float dif[] = { 1.0, 1.0, 1.0, 1.0 };
        float shin[] = { 0.6, 0.6, 0.6, 1.0 };
        float As[] = { 0.0, 0.0, 0.0, 1.0 };

	int i;
        for (i=0; i<8; i++) {
                lights[i] = 9999;
                lightState(i,FALSE);

        	FW_GL_LIGHTFV(i, GL_POSITION, pos);
        	FW_GL_LIGHTFV(i, GL_AMBIENT, As);
        	FW_GL_LIGHTFV(i, GL_DIFFUSE, dif);
        	FW_GL_LIGHTFV(i, GL_SPECULAR, shin);
        }
        lightState(HEADLIGHT_LIGHT, TRUE);

        FW_GL_LIGHTMODELI(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
        FW_GL_LIGHTMODELI(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        FW_GL_LIGHTMODELI(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
        FW_GL_LIGHTMODELI(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        FW_GL_LIGHTMODELFV(GL_LIGHT_MODEL_AMBIENT,As);

	LIGHTING_INITIALIZE

}


/* material node usage depends on texture depth; if rgb (depth1) we blend color field
   and diffusecolor with texture, else, we dont bother with material colors */

int last_texture_type = NOTEXTURE;

/* Sounds can come from AudioClip nodes, or from MovieTexture nodes. Different
   structures on these */
int sound_from_audioclip = 0;

int textures_take_priority = TRUE;
#ifdef DO_MULTI_OPENGL_THREADS
int useShapeThreadIfPossible = TRUE;
#endif

/* for printing warnings about Sound node problems - only print once per invocation */
int soundWarned = FALSE;

int render_vp; /*set up the inverse viewmatrix of the viewpoint.*/
int render_geom;
int render_light;
int render_sensitive;
int render_blend;
int render_proximity;
int render_collision;

/* texture stuff - see code. Need array because of MultiTextures */
GLuint boundTextureStack[MAX_MULTITEXTURE];
int textureStackTop;

int	have_transparency=FALSE;/* did any Shape have transparent material? */
int	lightingOn;		/* do we need to restore lighting in Shape? */
int	cullFace;		/* is GL_CULL_FACE enabled or disabled?		*/

GLint smooth_normals = TRUE; /* do normal generation? */

int cur_hits=0;

/* Collision detection results */
struct sCollisionInfo CollisionInfo = { {0,0,0} , 0, 0. };
struct sFallInfo FallInfo; /* = {100.0,1.0,0.0,0.0, 0,1,0,0}; /* too many to initialize here */

/* dimentions of viewer, and "up" vector (for collision detection) */
struct sNaviInfo naviinfo = {0.25, 1.6, 0.75};

/* for alignment of collision cylinder, and gravity (later). */
//struct point_XYZ ViewerUpvector = {0,0,0};

X3D_Viewer Viewer; /* has to be defined somewhere, so it found itself stuck here */


/* These two points define a ray in window coordinates */

struct point_XYZ r1 = {0,0,-1},r2 = {0,0,0},r3 = {0,1,0};
struct point_XYZ t_r1,t_r2,t_r3; /* transformed ray */
void *hypersensitive = 0; int hyperhit = 0;
struct point_XYZ hyper_r1,hyper_r2; /* Transformed ray for the hypersensitive node */

GLint viewport[4] = {-1,-1,2,2};

/* These three points define 1. hitpoint 2., 3. two different tangents
 * of the surface at hitpoint (to get transformation correctly */

/* All in window coordinates */

struct point_XYZ hp, ht1, ht2;
double hpdist; /* distance in ray: 0 = r1, 1 = r2, 2 = 2*r2-r1... */

/* used to save rayhit and hyperhit for later use by C functions */
struct SFColor hyp_save_posn, hyp_save_norm, ray_save_posn;

/* Any action for the Browser to do? */
int BrowserAction = FALSE;
struct X3D_Anchor *AnchorsAnchor = NULL;
char *OSX_replace_world_from_console = NULL;


struct currayhit rayHit,rayph,rayHitHyper;
/* used to test new hits */

/* this is used to return the duration of an audioclip to the perl
   side of things. works, but need to figure out all
   references, etc. to bypass this fudge JAS */
float AC_LastDuration[50]  = {-1.0,-1.0,-1.0,-1.0,-1.0,
			      -1.0,-1.0,-1.0,-1.0,-1.0,
			      -1.0,-1.0,-1.0,-1.0,-1.0,
			      -1.0,-1.0,-1.0,-1.0,-1.0,
			      -1.0,-1.0,-1.0,-1.0,-1.0,
			      -1.0,-1.0,-1.0,-1.0,-1.0,
			      -1.0,-1.0,-1.0,-1.0,-1.0,
			      -1.0,-1.0,-1.0,-1.0,-1.0,
			      -1.0,-1.0,-1.0,-1.0,-1.0,
			      -1.0,-1.0,-1.0,-1.0,-1.0} ;

/* is the sound engine started yet? */
int SoundEngineStarted = FALSE;

void *rootNode=NULL;	/* scene graph root node */
void *empty_group=0;

/*******************************************************************************/

/* Sub, rather than big macro... */
void rayhit(float rat, float cx,float cy,float cz, float nx,float ny,float nz,
	    float tx,float ty, char *descr)  {
	GLDOUBLE modelMatrix[16];
	GLDOUBLE projMatrix[16];

	/* Real rat-testing */
#ifdef RENDERVERBOSE
	printf("RAY HIT %s! %f (%f %f %f) (%f %f %f)\n\tR: (%f %f %f) (%f %f %f)\n",
	       descr, rat,cx,cy,cz,nx,ny,nz,
	       t_r1.x, t_r1.y, t_r1.z,
	       t_r2.x, t_r2.y, t_r2.z
		);
#endif

	if(rat<0 || (rat>hpdist && hpdist >= 0)) {
		return;
	}
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix);
	FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, projMatrix);
	FW_GLU_PROJECT(cx,cy,cz, modelMatrix, projMatrix, viewport, &hp.x, &hp.y, &hp.z);
	hpdist = rat;
	rayHit=rayph;
	rayHitHyper=rayph;
#ifdef RENDERVERBOSE 
	printf ("Rayhit, hp.x y z: - %f %f %f rat %f hpdist %f\n",hp.x,hp.y,hp.z, rat, hpdist);
#endif
}


/* Call this when modelview and projection modified */
void upd_ray() {
	GLDOUBLE modelMatrix[16];
	GLDOUBLE projMatrix[16];
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix);
	FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, projMatrix);
	FW_GLU_UNPROJECT(r1.x,r1.y,r1.z,modelMatrix,projMatrix,viewport,
		     &t_r1.x,&t_r1.y,&t_r1.z);
	FW_GLU_UNPROJECT(r2.x,r2.y,r2.z,modelMatrix,projMatrix,viewport,
		     &t_r2.x,&t_r2.y,&t_r2.z);
	FW_GLU_UNPROJECT(r3.x,r3.y,r3.z,modelMatrix,projMatrix,viewport,
		     &t_r3.x,&t_r3.y,&t_r3.z);
/*	printf("Upd_ray: (%f %f %f)->(%f %f %f) == (%f %f %f)->(%f %f %f)\n",
	r1.x,r1.y,r1.z,r2.x,r2.y,r2.z,
	t_r1.x,t_r1.y,t_r1.z,t_r2.x,t_r2.y,t_r2.z);
*/
}


/* if a node changes, void the display lists */
/* Courtesy of Jochen Hoenicke */

void update_node(struct X3D_Node *node) {
	int i;

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
			node->_parents[i] = empty_group;
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

#ifdef RENDERVERBOSE
static int renderLevel = 0;
#endif

#define PRINT_NODE(_node, _v)  do {					\
		if (global_print_opengl_errors && (_global_gl_err != GL_NO_ERROR)) { \
			printf("Render_node_v %d (%s) PREP: %d REND: %d CH: %d FIN: %d RAY: %d HYP: %d\n",(int) _v, \
			       stringNodeType(_node->_nodeType),	\
			       (int) _v->prep,				\
			       (int) _v->rend,				\
			       (int) _v->children,			\
			       (int) _v->fin,				\
			       (int) _v->rendray,			\
			       (int) hypersensitive);			\
			printf("Render_state geom %d light %d sens %d\n", \
			       render_geom,				\
			       render_light,				\
			       render_sensitive);			\
			printf("pchange %d pichange %d vchanged %d\n", _node->_change, _node->_ichange,(int) _v->changed); \
		}							\
	} while (0)


void render_node(struct X3D_Node *node) {
	struct X3D_Virt *v;
	int srg = 0;
	int sch = 0;
	struct currayhit srh;

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

	v = *(struct X3D_Virt **)node;
#ifdef RENDERVERBOSE 
	printf("%d =========================================NODE RENDERED===================================================\n",renderLevel);
	printf("%d node %u (%s) , v %u renderFlags %x ",renderLevel, node,stringNodeType(node->_nodeType),v,node->_renderFlags);
	printf("PREP: %d REND: %d CH: %d FIN: %d RAY: %d HYP: %d\n",v, v->prep, v->rend, v->children, v->fin,
	       v->rendray, hypersensitive);
	printf("%d state: vp %d geom %d light %d sens %d blend %d prox %d col %d ", renderLevel, 
         	render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); 
	printf("change %d ichange %d changed %d\n",node->_change, node->_ichange,v->changed);
#endif


	/* call the "changed_" function */
	if(NODE_NEEDS_COMPILING  && (v->changed != NULL)) {
		DEBUG_RENDER("rs 1 pch %d pich %d vch %d\n",node->_change,node->_ichange,v->changed);
		v->changed(node);
		MARK_NODE_COMPILED;
		PRINT_GL_ERROR_IF_ANY("change"); PRINT_NODE(node,v);
	}

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

	if(v->prep) {
		DEBUG_RENDER("rs 2\n");
		v->prep(node);
		if(render_sensitive && !hypersensitive) {
			upd_ray();
		}
		PRINT_GL_ERROR_IF_ANY("prep"); PRINT_NODE(node,v);
	}

	if(render_proximity && v->proximity) {
		DEBUG_RENDER("rs 2a\n");
		v->proximity(node);
		PRINT_GL_ERROR_IF_ANY("render_proximity"); PRINT_NODE(node,v);
	}

	if(render_collision && v->collision) {
		DEBUG_RENDER("rs 2b\n");
		v->collision(node);
		PRINT_GL_ERROR_IF_ANY("render_collision"); PRINT_NODE(node,v);
	}

	if(render_geom && !render_sensitive && v->rend) {
		DEBUG_RENDER("rs 3\n");
		v->rend(node);
		PRINT_GL_ERROR_IF_ANY("render_geom"); PRINT_NODE(node,v);
	}
	 
	if(render_sensitive && (node->_renderFlags & VF_Sensitive)) {
		DEBUG_RENDER("rs 5\n");
		srg = render_geom;
		render_geom = 1;
		DEBUG_RENDER("CH1 %d: %d\n",node, cur_hits, node->_hit);
		sch = cur_hits;
		cur_hits = 0;
		/* HP */
		srh = rayph;
		rayph.node = node;
		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, rayph.modelMatrix);
		FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, rayph.projMatrix);
		PRINT_GL_ERROR_IF_ANY("render_sensitive"); PRINT_NODE(node,v);
	}

	if(render_geom && render_sensitive && !hypersensitive && v->rendray) {
		DEBUG_RENDER("rs 6\n");
		v->rendray(node);
		PRINT_GL_ERROR_IF_ANY("rs 6"); PRINT_NODE(node,v);
	}

        if((render_sensitive) && (hypersensitive == node)) {
		DEBUG_RENDER("rs 7\n");
		hyper_r1 = t_r1;
		hyper_r2 = t_r2;
		hyperhit = 1;
        }

        if(v->children) { 
		DEBUG_RENDER("rs 8 - has valid child node pointer\n");
		v->children(node);
		PRINT_GL_ERROR_IF_ANY("children"); PRINT_NODE(node,v);
        }

	if(render_sensitive && (node->_renderFlags & VF_Sensitive)) {
		DEBUG_RENDER("rs 9\n");
		render_geom = srg;
		cur_hits = sch;
		DEBUG_RENDER("CH3: %d %d\n",cur_hits, node->_hit);
		/* HP */
		rayph = srh;
	}

	if(v->fin) {
		DEBUG_RENDER("rs A\n");
		v->fin(node);
		if(render_sensitive && v == &virt_Transform) {
			upd_ray();
		}
		PRINT_GL_ERROR_IF_ANY("fin"); PRINT_NODE(node,v);
	}

#ifdef RENDERVERBOSE 
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

#define CHILDVERBOSE
#ifdef CHILDVERBOSE
	printf ("add_parent; adding node %u ,to parent %u at %s:%d\n",node,  parent,file,line);
	printf ("add_parent; adding node %x ,to parent %x (hex) at %s:%d\n",node,  parent,file,line);
	printf ("add_parent; adding node %p ,to parent %p (ptr) at %s:%d\n",node,  parent,file,line);


	printf ("add_parent; adding node %u (%s) to parent %u (%s) at %s:%d\n",node, stringNodeType(node->_nodeType), 
		parent, stringNodeType(parent->_nodeType),file,line);
#endif
#undef CHILDVERBOSE

	parent->_renderFlags = parent->_renderFlags | node->_renderFlags;

	oldparcount = node->_nparents;
	if((oldparcount+1) > node->_nparalloc) {
		node->_nparents = 0; /* for possible threading issues */
		node->_nparalloc += 10;
		if (node->_parents == NULL)  {
			node->_parents = (void **)MALLOC(sizeof(node->_parents[0])* node->_nparalloc) ;
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
render_hier(struct X3D_Node *p, int rwhat) {
	struct point_XYZ upvec = {0,1,0};
	GLDOUBLE modelMatrix[16];
#define XXXrender_pre_profile
#ifdef render_pre_profile
	/*  profile */
	double xx,yy,zz,aa,bb,cc,dd,ee,ff;
	struct timeval mytime;
	struct timezone tz; /* unused see man gettimeofday */
#endif


	render_vp = rwhat & VF_Viewpoint;
	render_geom =  rwhat & VF_Geom;
	render_light = rwhat & VF_globalLight;
	render_sensitive = rwhat & VF_Sensitive;
	render_blend = rwhat & VF_Blend;
	render_proximity = rwhat & VF_Proximity;
	render_collision = rwhat & VF_Collision;
	nextFreeLight = 0;
	hpdist = -1;


#ifdef render_pre_profile
	if (render_geom) {
		gettimeofday (&mytime,&tz);
		aa = (double)mytime.tv_sec+(double)mytime.tv_usec/1000000.0;
	}
#endif

	/* printf ("render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	   render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision);  */

	if (!p) {
		/* we have no geometry yet, sleep for a tiny bit */
		usleep(1000);
		return;
	}

#ifdef RENDERVERBOSE
	printf("Render_hier node=%d what=%d\n", p, rwhat);
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

	render_node(p);

#ifdef render_pre_profile
	if (render_geom) {
		gettimeofday (&mytime,&tz);
		dd = (double)mytime.tv_sec+(double)mytime.tv_usec/1000000.0;
		printf ("render_geom status %f ray %f geom %f\n",bb-aa, cc-bb, dd-cc);
	}
#endif


	/*get viewpoint result, only for upvector Jan16,2010 not needed now - gravity recomputed in mainloop.c render_collisions on each frame*/
//	if (render_vp &&
//	    ViewerUpvector.x == 0 &&
//	    ViewerUpvector.y == 0 &&
//	    ViewerUpvector.z == 0) {
//
//		/* store up vector for gravity and collision detection for component_grouping/compnent_geospatial #ifdef COLLISIONTRANSFORM
//			see also mainloop.c render_collisions() which computes gravity matrix for regular collision and terrain following */
//		/* naviinfo.reset_upvec is set to 1 after a viewpoint change */
//		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix);
//		matinverse(modelMatrix,modelMatrix);
//		transform3x3(&ViewerUpvector,&upvec,modelMatrix); 
//		printf("ViewerUpvector = (%f,%f,%f)\n", ViewerUpvector);
//		
//#ifdef RENDERVERBOSE 
//		printf("ViewerUpvector = (%f,%f,%f)\n", ViewerUpvector);
//#endif
//
//	}
}


/******************************************************************************
 *
 * shape compiler "thread"
 *
 ******************************************************************************/

void compileNode (void (*nodefn)(void *, void *, void *, void *, void *), void *node, void *Icoord, void *Icolor, void *Inormal, void *ItexCoord) {
	void *coord; void *color; void *normal; void *texCoord;
	/* check to see if textures are being parsed right now */

	/* give textures priority over node compiling */
	if (textures_take_priority) {
		if (isTextureParsing()==TRUE) {
			/* printf ("compileNode, textures parsing, returning\n"); */
			return;
		}
	}

	/* are any of these SFNodes PROTOS? If so, get the underlying real node, as PROTOS are handled like Groups. */
	POSSIBLE_PROTO_EXPANSION(Icoord,coord)
		POSSIBLE_PROTO_EXPANSION(Icolor,color)
		POSSIBLE_PROTO_EXPANSION(Inormal,normal)
		POSSIBLE_PROTO_EXPANSION(ItexCoord,texCoord)

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
	} else if (strcmp("MidiControl",x)==0) { return (void *)do_MidiControl;
	
	} else {
		return 0;
	}
}




void checkParentLink (struct X3D_Node *node,struct X3D_Node *parent) {
        int n;

	size_t *offsetptr;
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
	offsetptr = (size_t *)NODE_OFFSETS[node->_nodeType];

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
struct Multi_Vec3f *getCoordinate (void *innode, char *str) {
	struct X3D_Coordinate * xc;
	struct X3D_GeoCoordinate *gxc;
	struct X3D_Node *node;

	POSSIBLE_PROTO_EXPANSION (innode,node)

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


