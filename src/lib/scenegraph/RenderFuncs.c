/*
=INSERT_TEMPLATE_HERE=

$Id: RenderFuncs.c,v 1.19.2.1 2009/07/08 21:55:04 couannette Exp $

Scenegraph rendering.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <pthread.h> /* this is needed here, for some reason JAS */
#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"

#include "Polyrep.h"
#include "Collision.h"
#include "../scenegraph/quaternion.h"
#include "Viewer.h"
#include "LinearAlgebra.h"
#include "../input/SensInterps.h"

#include "RenderFuncs.h"

/* Rearrange to take advantage of headlight when off */
static int curlight = 0;
int nlightcodes = 7;
int lightcode[7] = {
	GL_LIGHT0,
	GL_LIGHT1,
	GL_LIGHT2,
	GL_LIGHT3,
	GL_LIGHT4,
	GL_LIGHT5,
	GL_LIGHT6,
};
int nextlight() {
	if(curlight == nlightcodes) { return -1; }
	return lightcode[curlight++];
}




/* material node usage depends on texture depth; if rgb (depth1) we blend color field
   and diffusecolor with texture, else, we dont bother with material colors */

int last_texture_type = NOTEXTURE;

/* Sounds can come from AudioClip nodes, or from MovieTexture nodes. Different
   structures on these */
int sound_from_audioclip = 0;

/* and, we allow a maximum of so many pixels per texture */
/* if this is zero, first time a texture call is made, this is set to the OpenGL implementations max */
GLint global_texSize = 0;
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

int be_collision = 0;	/* do collision detection? */

/* texture stuff - see code. Need array because of MultiTextures */
GLuint bound_textures[MAX_MULTITEXTURE];
int bound_texture_alphas[MAX_MULTITEXTURE];
int texture_count;

int	have_transparency=FALSE;/* did any Shape have transparent material? */
void *	this_textureTransform;  /* do we have some kind of textureTransform? */
int	lightingOn;		/* do we need to restore lighting in Shape? */
int	cullFace;		/* is GL_CULL_FACE enabled or disabled?		*/

int     shutterGlasses = 0; 	/* stereo shutter glasses */


GLint smooth_normals = TRUE; /* do normal generation? */

int cur_hits=0;

/* Collision detection results */
struct sCollisionInfo CollisionInfo = { {0,0,0} , 0, 0. };

/* dimentions of viewer, and "up" vector (for collision detection) */
struct sNaviInfo naviinfo = {0.25, 1.6, 0.75};

/* for alignment of collision cylinder, and gravity (later). */
struct point_XYZ ViewerUpvector = {0,0,0};

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
struct X3D_Anchor *AnchorsAnchor;


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

/* stored FreeWRL version, pointers to initialize data */
#ifdef DO_MULTI_OPENGL_THREADS
pthread_t shapeThread = -1;
#endif

void *rootNode=NULL;	/* scene graph root node */
void *empty_group=0;

/*******************************************************************************/

/* Sub, rather than big macro... */
void rayhit(float rat, float cx,float cy,float cz, float nx,float ny,float nz,
float tx,float ty, char *descr)  {
	GLdouble modelMatrix[16];
	GLdouble projMatrix[16];

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
	fwGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	fwGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
	gluProject(cx,cy,cz, modelMatrix, projMatrix, viewport, &hp.x, &hp.y, &hp.z);
	hpdist = rat;
	rayHit=rayph;
	rayHitHyper=rayph;
	#ifdef RENDERVERBOSE 
		printf ("Rayhit, hp.x y z: - %f %f %f rat %f hpdist %f\n",hp.x,hp.y,hp.z, rat, hpdist);
	#endif
}


/* Call this when modelview and projection modified */
void upd_ray() {
	GLdouble modelMatrix[16];
	GLdouble projMatrix[16];
	fwGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	fwGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
	gluUnProject(r1.x,r1.y,r1.z,modelMatrix,projMatrix,viewport,
		&t_r1.x,&t_r1.y,&t_r1.z);
	gluUnProject(r2.x,r2.y,r2.z,modelMatrix,projMatrix,viewport,
		&t_r2.x,&t_r2.y,&t_r2.z);
	gluUnProject(r3.x,r3.y,r3.z,modelMatrix,projMatrix,viewport,
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

void render_node(struct X3D_Node *node) {
	struct X3D_Virt *v;
	int srg = 0;
	int sch = 0;
	struct currayhit srh;
	GLint glerror = GL_NONE;
	char* stage = "";

	X3D_NODE_CHECK(node);

	#ifdef RENDERVERBOSE
		renderLevel ++;
	#endif

	if(!node) {
		#ifdef RENDERVERBOSE
		printf ("%d no node, quick return\n",renderLevel); renderLevel--;
		#endif
		return;
	}

	v = *(struct X3D_Virt **)node;
	#ifdef RENDERVERBOSE 
	    printf("%d =========================================NODE RENDERED===================================================\n",renderLevel);
	printf ("%d node %u (%s) , v %u renderFlags %x ",renderLevel, node,stringNodeType(node->_nodeType),v,node->_renderFlags);
	    printf("PREP: %d REND: %d CH: %d FIN: %d RAY: %d HYP: %d\n",v, v->prep, v->rend, v->children, v->fin,
		   v->rendray, hypersensitive);
            printf ("%d state: vp %d geom %d light %d sens %d blend %d prox %d col %d ", renderLevel, 
         	render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); 
	    printf ("change %d ichange %d changed %d\n",node->_change, node->_ichange,v->changed);
	#endif


	/* call the "changed_" function */
	if(NODE_NEEDS_COMPILING  && (v->changed != NULL)) {
	    #ifdef RENDERVERBOSE 
		printf ("rs 1 pch %d pich %d vch %d\n",node->_change,node->_ichange,v->changed);
	    #endif
	    v->changed(node);
	    MARK_NODE_COMPILED

	    if (displayOpenGLErrors) 
	  	  if(glerror == GL_NONE && ((glerror = glGetError()) != GL_NONE) ) stage = "change";
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
	    #ifdef RENDERVERBOSE 
		printf ("rs 2\n");
	    #endif

	    v->prep(node);
	    if(render_sensitive && !hypersensitive) {
		upd_ray();
	      }
	    if (displayOpenGLErrors) if(glerror == GL_NONE && ((glerror = glGetError()) != GL_NONE) ) stage = "prep";
	  }

	if(render_proximity && v->proximity) {
	    #ifdef RENDERVERBOSE 
		printf ("rs 2a\n");
	    #endif
	    v->proximity(node);
	    if (displayOpenGLErrors) if(glerror == GL_NONE && ((glerror = glGetError()) != GL_NONE) ) stage = "render_proximity";
	}

	if(render_collision && v->collision) {
	    #ifdef RENDERVERBOSE 
		printf ("rs 2b\n");
	    #endif

	    v->collision(node);
	    #ifdef RENDERVERBOSE 
	    #endif
	
	    if (displayOpenGLErrors) if(glerror == GL_NONE && ((glerror = glGetError()) != GL_NONE) ) stage = "render_collision";
	}



	if(render_geom && !render_sensitive && v->rend) {
	    #ifdef RENDERVERBOSE 
		printf ("rs 3\n");
	    #endif

	    v->rend(node);
	    if (displayOpenGLErrors) if(glerror == GL_NONE && ((glerror = glGetError()) != GL_NONE) ) stage = "render_geom";
	  }
	 
	if(render_sensitive && (node->_renderFlags & VF_Sensitive)) {
	    #ifdef RENDERVERBOSE 
		printf ("rs 5\n");
	    #endif

	    srg = render_geom;
	    render_geom = 1;
	    #ifdef RENDERVERBOSE 
		printf("CH1 %d: %d\n",node, cur_hits, node->_hit);
	    #endif

	    sch = cur_hits;
	    cur_hits = 0;
	    /* HP */
	      srh = rayph;
	    rayph.node = node;
	    fwGetDoublev(GL_MODELVIEW_MATRIX, rayph.modelMatrix);
	    fwGetDoublev(GL_PROJECTION_MATRIX, rayph.projMatrix);
	    if (displayOpenGLErrors) if(glerror == GL_NONE && ((glerror = glGetError()) != GL_NONE) ) stage = "render_sensitive";

	  }
	if(render_geom && render_sensitive && !hypersensitive && v->rendray) {
	    #ifdef RENDERVERBOSE 
		printf ("rs 6\n");
	    #endif

	    v->rendray(node);
	    if (displayOpenGLErrors) if(glerror == GL_NONE && ((glerror = glGetError()) != GL_NONE) ) stage = "rs 6";
	  }


        if((render_sensitive) && (hypersensitive == node)) {
            #ifdef RENDERVERBOSE 
		printf ("rs 7\n");
	    #endif

            hyper_r1 = t_r1;
            hyper_r2 = t_r2;
            hyperhit = 1;
        }
        if(v->children) { 
	#ifdef RENDERVERBOSE 
		printf ("rs 8 - has valid child node pointer\n");
	    #endif

            v->children(node);
	    if (displayOpenGLErrors) if(glerror == GL_NONE && ((glerror = glGetError()) != GL_NONE) ) stage = "children";
        }

	if(render_sensitive && (node->_renderFlags & VF_Sensitive)) {
	    #ifdef RENDERVERBOSE 
		printf ("rs 9\n");
	    #endif

	    render_geom = srg;
	    cur_hits = sch;
	    #ifdef RENDERVERBOSE 
		printf("CH3: %d %d\n",cur_hits, node->_hit);
	    #endif

	    /* HP */
	      rayph = srh;
	  }
	if(v->fin) {
	    #ifdef RENDERVERBOSE 
		printf ("rs A\n");
	    #endif

	    v->fin(node);

	    if(render_sensitive && v == &virt_Transform)
	      {
		upd_ray();
	      }
	    if (displayOpenGLErrors) if(glerror != GL_NONE && ((glerror = glGetError()) != GL_NONE) ) stage = "fin";
	  }
	#ifdef RENDERVERBOSE 
		printf("%d (end render_node)\n",renderLevel);
		renderLevel--;
	#endif

	if (displayOpenGLErrors) if(glerror != GL_NONE)
	  {
	    printf("============== GLERROR : %s in stage %s =============\n",gluErrorString(glerror),stage);
	    printf("Render_node_v %d (%s) PREP: %d REND: %d CH: %d FIN: %d RAY: %d HYP: %d\n",v,
		   stringNodeType(node->_nodeType),
		   v->prep,
		   v->rend,
		   v->children,
		   v->fin,
		   v->rendray,
		   hypersensitive);
	    printf("Render_state geom %d light %d sens %d\n",
		   render_geom,
		   render_light,
		   render_sensitive);
	    printf ("pchange %d pichange %d vchanged %d\n",node->_change, node->_ichange,v->changed);
	    printf("==============\n");
	  }
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
	int count;

	if(!node) return;

	#ifdef CHILDVERBOSE
	printf ("add_parent; adding node %u (%s) to parent %u (%s) at %s:%d\n",node, stringNodeType(node->_nodeType), 
			parent, stringNodeType(parent->_nodeType),file,line);
	#endif

	/* does this already exist? it is OK to have this added more than once */
	#ifdef checkforduplicateparentsinaddparent
	for (count=0; count<node->_nparents; count++) {
		if (node->_parents[count] == parent) {
			#ifdef CHILDVERBOSE
			printf ("add_parent; parent already exists in this node\n");
			#endif
			return;
		}
	}
	#endif
 
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
	GLdouble modelMatrix[16];
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
	curlight = 0;
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
#ifdef WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
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


	/*get viewpoint result, only for upvector*/
	if (render_vp &&
		ViewerUpvector.x == 0 &&
		ViewerUpvector.y == 0 &&
		ViewerUpvector.z == 0) {

		/* store up vector for gravity and collision detection */
		/* naviinfo.reset_upvec is set to 1 after a viewpoint change */
		fwGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
		matinverse(modelMatrix,modelMatrix);
		transform3x3(&ViewerUpvector,&upvec,modelMatrix);

		#ifdef RENDERVERBOSE 
		printf("ViewerUpvector = (%f,%f,%f)\n", ViewerUpvector);
		#endif

	}
}

/* shutter glasses, stereo view  from Mufti@rus */
/* handle setting shutter from parameters */
void setShutter (void) {
        shutterGlasses = 1;
}

/******************************************************************************
 *
 * shape compiler "thread"
 *
 ******************************************************************************/
#ifdef DO_MULTI_OPENGL_THREADS

/* threading variables for loading shapes in threads */
static pthread_mutex_t shapeMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t shapeCond   = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t shapeGenMutex = PTHREAD_MUTEX_INITIALIZER;
#define SLOCK           pthread_mutex_lock(&shapeMutex);
#define SUNLOCK         pthread_mutex_unlock(&shapeMutex);
#define S_LOCK_SIGNAL   pthread_cond_signal(&shapeCond);
#define S_LOCK_WAIT     pthread_cond_wait(&shapeCond,&shapeMutex);
/* lock the reallocs of data structures */
#define REGENLOCK       pthread_mutex_lock(&shapeGenMutex);
#define REGENUNLOCK     pthread_mutex_unlock(&shapeGenMutex);

/* are we currently active? */
int shapeCompiling = FALSE;

int CompileThreadInitialized = FALSE;
static void (*shapemethodptr)(void *, void *, void *, void *, void *); 	/* method used to compile this node 	*/
static void *shapenodeptr;		/* node pointer of node data		*/
static void *shapecoord;		/* Polrep shape coord node		*/
static void *shapecolor;		/* Polyrep shape color node		*/
static void *shapenormal;		/* Polyrep shape normal node		*/
static void *shapetexCoord;		/* Polyrep shape tex coord node		*/

void _shapeCompileThread () {
	/* we wait forever for the data signal to be sent */
	for (;;) {
		SLOCK
		CompileThreadInitialized = TRUE;
		S_LOCK_WAIT
		shapeCompiling = TRUE;
		/* printf ("shapethread compiling\n"); */

		/* so, lets do the compile */
		shapemethodptr(shapenodeptr, shapecoord, shapecolor, shapenormal, shapetexCoord);

		shapeCompiling = FALSE;
		SUNLOCK
	}
}


void initializeShapeCompileThread() {
	int iret;

	if (shapeThread == NULL) {
        	iret = pthread_create(&shapeThread, NULL, (void *(*)(void *))&_shapeCompileThread, NULL);
	}
}

#endif

int isShapeCompilerParsing() {
	#ifdef DO_MULTI_OPENGL_THREADS
	return shapeCompiling;
	#else
	return FALSE;
	#endif
}

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

	#ifdef DO_MULTI_OPENGL_THREADS

	/* do we want to use a seperate thread for compiling shapes, or THIS thread? */
	if (useShapeThreadIfPossible) {
		if (!shapeCompiling) {
			if (!CompileThreadInitialized) return; /* still starting up */
	
	
			/* lock for exclusive thread access */
	        	SLOCK
	
			/* copy our params over */
			shapemethodptr = nodefn;
			shapenodeptr = node;
			shapecoord = coord;
			shapecolor = color;
			shapenormal = normal;
			shapetexCoord = texCoord;
			/* signal to the shape compiler thread that there is data here */
			S_LOCK_SIGNAL
	        	SUNLOCK
			
		}
		sched_yield();
	} else {
		/* ok, we do not want to use the shape compile thread, just do it */
		nodefn(node, coord, color, normal, texCoord);
	}

	#else
	/* ok, we cant do a shape compile thread, just do it */
	nodefn(node, coord, color, normal, texCoord);
	#endif
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
	} else if (strcmp("NormalInterpolator",x)==0) { return (void *)do_OintCoord;
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
	offsetptr = (int *)NODE_OFFSETS[node->_nodeType];

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
		COMPILE_IF_REQUIRED
		gxc = X3D_GEOCOORD(node);
		return &(gxc->__movedCoords);
	} else {
		ConsoleMessage ("%s - coord expected but got %s\n", stringNodeType(xc->_nodeType));
	}
}


/************************************************************************************************/
/*												*/
/*	MetadataMF and MetadataSF nodes								*/
/*												*/
/************************************************************************************************/

#define META_IS_INITIALIZED (node->_ichange != 0)

/* anything changed for this PROTO interface datatype? */
#define CMD_I32(type) void compile_MetadataSF##type (struct X3D_MetadataSF##type *node) { \
	if META_IS_INITIALIZED { \
	if (node->value != node->setValue) { \
		node->value = node->setValue; \
		node->valueChanged = node->setValue; \
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataSF##type, valueChanged)); \
	} \
	} else { \
		/* initialize fields */ \
		node->valueChanged = node->value; node->setValue = node->value; \
	} \
	MARK_NODE_COMPILED \
}

#define CMD_FL(type) void compile_MetadataSF##type (struct X3D_MetadataSF##type *node) { \
	if META_IS_INITIALIZED { \
	if (!APPROX(node->value,node->setValue)) { \
		node->value = node->setValue; \
		node->valueChanged = node->setValue; \
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataSF##type, valueChanged)); \
	} \
	} else { \
		/* initialize fields */ \
		node->valueChanged = node->value; node->setValue = node->value; \
	} \
	MARK_NODE_COMPILED \
}

#define CMD_MFL(type,elelength) void compile_MetadataSF##type (struct X3D_MetadataSF##type *node) { \
	int count; \
	if META_IS_INITIALIZED { \
	for (count=0; count < elelength; count++) { \
		if (!APPROX(node->value.c[count],node->setValue.c[count])) { \
			memcpy (&node->value, &node->setValue, sizeof node->value.c[0]* elelength); \
			memcpy (&node->valueChanged, &node->setValue, sizeof node->value.c[0] * elelength); \
			MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataSF##type, valueChanged)); \
			return; \
		} \
	} \
	} else { \
		/* initialize fields */ \
		memcpy (&node->setValue, &node->value, sizeof node->value.c[0]* elelength); \
		memcpy (&node->valueChanged, &node->value, sizeof node->value.c[0] * elelength); \
	} \
	MARK_NODE_COMPILED \
}



/* compare element counts, and pointer values */
/* NOTE - VALUES CAN NOT BE DESTROYED BY THE KILL PROCESSES, AS THESE ARE JUST COPIES OF POINTERS */
#define CMD_MULTI(type,elelength,dataSize) void compile_MetadataMF##type (struct X3D_MetadataMF##type *node) { \
	int count; int changed = FALSE; \
	if META_IS_INITIALIZED { \
	if (node->value.n != node->setValue.n) changed = TRUE; else { \
		/* yes, these two array must have the same index counts... */ \
		for (count=0; count<node->setValue.n; count++) { \
			int count2; for (count2=0; count2<elelength; count2++) { if (!APPROX(node->value.p[count].c[count2], node->setValue.p[count].c[count2])) changed = TRUE; break; }\
		if (changed) break; } \
	} \
	\
	if (changed) { \
                        /* printf ("MSFL, change hit, freeing pointers %x and %x\n", node->value.p, node->valueChanged.p); */ \
			FREE_IF_NZ (node->value.p); \
			FREE_IF_NZ(node->valueChanged.p); \
			node->value.p = MALLOC(dataSize * node->setValue.n * elelength); \
			node->valueChanged.p = MALLOC(dataSize * node->setValue.n * elelength); \
			memcpy(node->value.p, node->setValue.p, dataSize * node->setValue.n * elelength); \
			memcpy(node->valueChanged.p, node->setValue.p, dataSize * node->setValue.n * elelength); \
                        node->value.n = node->setValue.n; \
                        node->valueChanged.n = node->setValue.n; \
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataMF##type, valueChanged)); \
	} \
	} else { \
		/* the "value" will hold everything we need */ \
		/* initialize it, but do not bother doing any routing on it */ \
		if ((node->setValue.n != 0) || (node->setValue.p != NULL) || (node->valueChanged.n != 0) || (node->valueChanged.p != NULL)) { printf ("PROTO header - initialization set and changed, but not zero??\n");  \
                node->setValue.n = 0; FREE_IF_NZ(node->setValue.p);  \
                node->valueChanged.n = 0; FREE_IF_NZ(node->valueChanged.p); } \
	} \
	MARK_NODE_COMPILED \
}

#define CMD_MSFI32(type,dataSize) void compile_MetadataMF##type (struct X3D_MetadataMF##type *node) { \
        /* printf ("MSFI32:, node %x\n",node); \
        printf ("MSFI32:, nt %s change %d ichange %d\n",stringNodeType(node->_nodeType),node->_change, node->_ichange); */ \
        if META_IS_INITIALIZED { \
                int count; int changed = FALSE; \
                /* printf ("MSFI32:, so this is initialized; value %d setValue count%d\n",node->value.n,node->setValue.n); */ \
/* { int count; char *cptr = (char *)&(node->setValue); for (count = 0; count < 8; count ++) { printf ("%u: %x ",count, *cptr); cptr ++; } \
 printf ("\n"); \
cptr = (char *)&(node->value); for (count = 0; count < 8; count ++) { printf ("%u: %x ",count, *cptr); cptr ++; } \
 printf ("\n"); \
} */\
                if (node->value.n != node->setValue.n) changed = TRUE; \
                else { \
		    /* same count, but something caused this to be called; go through each element */ \
                    for (count=0; count<node->setValue.n; count++) { \
                          /* printf ("MSFI32, comparing ele %d %x %x\n",count, node->value.p[count], node->setValue.p[count]); */ \
                          if (node->value.p[count] != node->setValue.p[count]) {changed = TRUE; break; } \
                    } \
                } \
 \
                if (changed) { \
                        /* printf ("MSFI32, change hit, freeing pointers %x and %x\n", node->value.p, node->valueChanged.p); */ \
			FREE_IF_NZ (node->value.p); \
			FREE_IF_NZ(node->valueChanged.p); \
			node->value.p = MALLOC(dataSize * node->setValue.n); \
			node->valueChanged.p = MALLOC(dataSize * node->setValue.n); \
			memcpy(node->value.p, node->setValue.p, dataSize * node->setValue.n); \
			memcpy(node->valueChanged.p, node->setValue.p, dataSize * node->setValue.n); \
                        node->value.n = node->setValue.n; \
                        node->valueChanged.n = node->setValue.n; \
                        MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataMF##type, valueChanged)); \
                } \
        } else { \
                /* the "value" will hold everything we need */ \
                /* initialize it, but do not bother doing any routing on it */ \
		/* printf ("MSFI32: initializing\n"); */ \
		if ((node->setValue.n != 0) || (node->setValue.p != NULL) || (node->valueChanged.n != 0) || (node->valueChanged.p != NULL)) { printf ("PROTO header - initialization set and changed, but not zero??\n");  \
                node->setValue.n = 0; FREE_IF_NZ(node->setValue.p);  \
                node->valueChanged.n = 0; FREE_IF_NZ(node->valueChanged.p); } \
		/* printf ("MSFI32 - leaving the setValue and ValueChanged pointers to %x %x\n",node->setValue.p, node->valueChanged.p);*/ \
        } \
        MARK_NODE_COMPILED \
        /* printf ("MSFI32: DONE; value %d, value_changed.n %d\n", node->value.n,node->valueChanged.n);  */ \
} 







/* compare element counts, then individual elements, if the counts are the same */
/* NOTE - VALUES CAN NOT BE DESTROYED BY THE KILL PROCESSES, AS THESE ARE JUST COPIES OF POINTERS */
#define CMD_MSFL(type,dataSize) void compile_MetadataMF##type (struct X3D_MetadataMF##type *node) { \
	int count; int changed = FALSE; \
	if META_IS_INITIALIZED { \
	if (node->value.n != node->setValue.n) changed = TRUE; else { \
		/* yes, these two array must have the same index counts... */ \
		for (count=0; count<node->setValue.n; count++) if (!APPROX(node->value.p[count], node->setValue.p[count])) { changed = TRUE; break; }}\
	\
	if (changed) { \
                        /* printf ("MSFL, change hit, freeing pointers %x and %x\n", node->value.p, node->valueChanged.p); */ \
			FREE_IF_NZ (node->value.p); \
			FREE_IF_NZ(node->valueChanged.p); \
			node->value.p = MALLOC( dataSize * node->setValue.n); \
			node->valueChanged.p = MALLOC(dataSize * node->setValue.n); \
			memcpy(node->value.p, node->setValue.p, dataSize * node->setValue.n); \
			memcpy(node->valueChanged.p, node->setValue.p, dataSize * node->setValue.n); \
                        node->value.n = node->setValue.n; \
                        node->valueChanged.n = node->setValue.n; \
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataMF##type, valueChanged)); \
	} \
	} else { \
		/* the "value" will hold everything we need */ \
		/* initialize it, but do not bother doing any routing on it */ \
		if ((node->setValue.n != 0) || (node->setValue.p != NULL) || (node->valueChanged.n != 0) || (node->valueChanged.p != NULL)) { printf ("PROTO header - initialization set and changed, but not zero??\n");  \
                node->setValue.n = 0; FREE_IF_NZ(node->setValue.p);  \
                node->valueChanged.n = 0; FREE_IF_NZ(node->valueChanged.p); } \
	} \
	MARK_NODE_COMPILED \
}


CMD_FL(Float)
CMD_FL(Time)
CMD_FL(Double)
CMD_I32(Bool)
CMD_I32(Int32)
CMD_I32(Node)

CMD_MFL(Vec2f,2)
CMD_MFL(Vec3f,3)
CMD_MFL(Vec4f,4)
CMD_MFL(Vec2d,2)
CMD_MFL(Vec3d,3)
CMD_MFL(Vec4d,4)
CMD_MFL(Rotation,4)
CMD_MFL(Color,3)
CMD_MFL(ColorRGBA,4)
CMD_MFL(Matrix3f,9)
CMD_MFL(Matrix3d,9)
CMD_MFL(Matrix4f,16)
CMD_MFL(Matrix4d,16)

CMD_MULTI(Rotation,4,sizeof (float))
CMD_MULTI(Vec2f,2,sizeof (float))
CMD_MULTI(Vec3f,3,sizeof (float))
CMD_MULTI(Vec4f,4,sizeof (float))
CMD_MULTI(Vec2d,2,sizeof (double))
CMD_MULTI(Vec3d,3,sizeof (double))
CMD_MULTI(Vec4d,4,sizeof (double))
CMD_MULTI(Color,3,sizeof (float))
CMD_MULTI(ColorRGBA,4,sizeof (float))
CMD_MULTI(Matrix3f,9,sizeof (float))
CMD_MULTI(Matrix4f,16,sizeof (float))
CMD_MULTI(Matrix3d,9,sizeof (double))
CMD_MULTI(Matrix4d,16,sizeof (double))

CMD_MSFI32(Bool, sizeof(int))
CMD_MSFI32(Int32,sizeof (int))
CMD_MSFI32(Node,sizeof (void *))
CMD_MSFL(Time,sizeof (double))
CMD_MSFL(Float,sizeof (float))
CMD_MSFL(Double,sizeof (double))
CMD_MSFI32(String,sizeof (void *))


void compile_MetadataSFImage (struct X3D_MetadataSFImage *node){ printf ("make compile_Metadata %s\n",stringNodeType(node->_nodeType));}
/*
struct Uni_String {
        int len;
        char * strptr;
        int touched;
};
*/

void compile_MetadataSFString (struct X3D_MetadataSFString *node){ 
	int count; int changed = FALSE; 

	if META_IS_INITIALIZED { 
	if (node->value->len != node->setValue->len) changed = TRUE; else { 
		for (count=0; count<node->setValue->len; count++) 
			if (node->value->strptr[count] != node->setValue->strptr[count]) changed = TRUE; }
	
	if (changed) { 
		node->value->len = node->setValue->len; node->value->strptr = node->setValue->strptr; 
		node->valueChanged->len = node->setValue->len; node->valueChanged->strptr = node->setValue->strptr; 
		node->value->touched = TRUE; node->valueChanged->touched = TRUE;
		MARK_EVENT (X3D_NODE(node), offsetof (struct X3D_MetadataSFString, valueChanged)); 
	} 
	} else {
		/* initialize this one */
		node->valueChanged->len = node->value->len;
		node->valueChanged->touched = node->value->touched;
		node->valueChanged->strptr = node->value->strptr;
		node->setValue->len = node->value->len;
		node->setValue->touched = node->value->touched;
		node->setValue->strptr = node->value->strptr;
	}
	MARK_NODE_COMPILED
}

