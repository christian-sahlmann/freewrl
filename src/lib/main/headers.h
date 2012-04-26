/*
=INSERT_TEMPLATE_HERE=

$Id: headers.h,v 1.174 2012/04/26 16:36:23 crc_canada Exp $

Global includes.

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



#ifndef __FREEWRL_HEADERS_H__
#define __FREEWRL_HEADERS_H__

/* NO INCLUDE IN INCLUDE: this prevent us from doing real
   include separation :)...
*/
   
/* for lightState() */
/* #include "../opengl/OpenGL_Utils.h" */

/* for localLightChildren */
/* #include "../scenegraph/Children.h" */

/**
 * in utils.c
 */

#define int_t intptr_t

extern char *BrowserName;
const char* freewrl_get_browser_program();

void Multi_String_print(struct Multi_String *url);

/* see if an inputOnly "set_" field has changed */
#define IO_FLOAT -2335549.0f

/* specification versions, for close adherence to requested spec levels */
#define SPEC_VRML 0x01
#define SPEC_X3D30 0x02
#define SPEC_X3D31 0x04
#define SPEC_X3D32 0x08
#define SPEC_X3D33 0x10
#define SPEC_X3D34 0x20
#define SPEC_VRML1 0x01 /* same as SPEC_VRML */




/* children fields path optimizations */
#define CHILDREN_COUNT int nc = node->_sortedChildren.n;
#define RETURN_FROM_CHILD_IF_NOT_FOR_ME \
        /* any children at all? */ \
        if (nc==0) return;      \
        /* should we go down here? */ \
        /* printf ("Group, rb %x VF_B %x, rg  %x VF_G %x\n",render_blend, VF_Blend, render_geom, VF_Geom); */ \
        if (renderstate()->render_blend == VF_Blend) \
                if ((node->_renderFlags & VF_Blend) != VF_Blend) { \
                        return; \
                } \
        if (renderstate()->render_proximity == VF_Proximity) \
                if ((node->_renderFlags & VF_Proximity) != VF_Proximity)  { \
                        return; \
                } \
        if (renderstate()->render_light == VF_globalLight) \
                if ((node->_renderFlags & VF_globalLight) != VF_globalLight)  { \
                        return; \
                } \

/* which GL_LIGHT is the headlight? */
#define HEADLIGHT_LIGHT 7


#define INT_ID_UNDEFINED -1

/* Size of static array */
#define ARR_SIZE(arr) (int)(sizeof(arr)/sizeof((arr)[0]))

extern char *BrowserName;
extern char *BrowserFullPath;

/* To allow BOOL for boolean values */
#define BOOL	int

/* rendering constants used in SceneGraph, etc. */
#define VF_Viewpoint 				0x0001
#define VF_Geom 				0x0002
#define VF_localLight				0x0004 
#define VF_Sensitive 				0x0008
#define VF_Blend 				0x0010
#define VF_Proximity 				0x0020
#define VF_Collision 				0x0040
#define VF_globalLight				0x0080 
#define VF_hasVisibleChildren 			0x0100
#define VF_shouldSortChildren			0x0200
#define VF_Other					0x0400
#ifdef DJTRACK_PICKSENSORS
#define VF_inPickableGroup			0x0800 /* DJTRACK_PICKSENSORS */
#define VF_PickingSensor			0x1000 /* DJTRACK_PICKSENSORS */
#endif

/* for z depth buffer calculations */
#define DEFAULT_NEARPLANE 0.1
#define DEFAULT_FARPLANE 21000.0
#define DEFAULT_BACKGROUNDPLANE 18000.0 /* approx 80% of DEFAULT_FARPLANE */
extern double geoHeightinZAxis;


/* Routing - complex types */
#define ROUTING_SFNODE          -23
#define ROUTING_MFNODE          -10
#define ROUTING_SFIMAGE         -12
#define ROUTING_MFSTRING        -13
#define ROUTING_MFFLOAT        -14
#define ROUTING_MFROTATION      -15
#define ROUTING_MFINT32         -16
#define ROUTING_MFCOLOR         -17
#define ROUTING_MFVEC2F         -18
#define ROUTING_MFVEC3F         -19
#define ROUTING_MFVEC3D         -20
#define ROUTING_MFDOUBLE        -21
#define ROUTING_SFSTRING        -22
#define ROUTING_MFMATRIX4F	-30
#define ROUTING_MFMATRIX4D	-31
#define ROUTING_MFVEC2D		-32
#define ROUTING_MFVEC4F		-33
#define ROUTING_MFVEC4D		-34
#define ROUTING_MFMATRIX3F	-35
#define ROUTING_MFMATRIX3D	-36



/* compile simple nodes (eg, Cone, LineSet) into an internal format. Check out
   CompileC in VRMLRend.pm, and look for compile_* functions in code. General
   meshes are rendered using the PolyRep scheme, which also compiles into OpenGL 
   calls, using the PolyRep (and, stream_PolyRep) methodology */

void compile_polyrep(void *node, void *coord, void *color, void *normal, void *texCoord);

#define NODE_CHANGE_INIT_VAL 153	/* node->_change is set to this when created */
#define COMPILE_POLY_IF_REQUIRED(a,b,c,d) \
                if(!node->_intern || node->_change != ((struct X3D_PolyRep *)node->_intern)->irep_change) { \
                        compileNode ((void *)compile_polyrep, node, a,b,c,d); \
		} \
		if (!node->_intern) return;

#define COMPILE_IF_REQUIRED { struct X3D_Virt *v; \
	if (node->_ichange != node->_change) { \
		v = virtTable[node->_nodeType]; \
		if (v->compile) { \
			compileNode (v->compile, (void *)node, NULL, NULL, NULL, NULL); \
		} else {printf ("huh - have COMPIFREQD, but v->compile null for %s at %s:%d\n",stringNodeType(node->_nodeType),__FILE__,__LINE__);} \
		} \
		if (node->_ichange == 0) return; \
	}

#define COMPILE_IF_REQUIRED_RETURN_NULL_ON_ERROR { struct X3D_Virt *v; \
	if (node->_ichange != node->_change) { \
		v = virtTable[node->_nodeType]; \
		if (v->compile) { \
			compileNode (v->compile, (void *)node, NULL, NULL, NULL, NULL); \
		} else {printf ("huh - have COMPIFREQD, but v->compile null for %s at %s:%d\n",stringNodeType(node->_nodeType),__FILE__,__LINE__);} \
		} \
		if (node->_ichange == 0) return NULL; \
	}

#define COMPILE_TCNODE { \
    struct X3D_Virt *v; \
    if (myTCnode->_ichange != myTCnode->_change) { \
        v = virtTable[myTCnode->_nodeType]; \
        if (v->compile) { \
            compileNode (v->compile, (void *)myTCnode, NULL, NULL, NULL, NULL); \
            myTCnode->_ichange = myTCnode->_change; \
        } else {printf ("huh - have COMPIFREQD, but v->compile null for %s at %s:%d\n",stringNodeType(myTCnode->_nodeType),__FILE__,__LINE__);} \
    } \
    if (myTCnode->_ichange == 0) return; \
}



/* convert a PROTO node (which will be a Group node) into a node. eg, for Materials  - this is a possible child
node for ANY node that takes something other than a Group */

#define offsetPointer_deref(t, me, offs) ((t)(((char*)(me))+offs))

#define POSSIBLE_PROTO_EXPANSION(type,inNode,outNode) \
	if (inNode == NULL) outNode = NULL; \
	else {if (X3D_NODE(inNode)->_nodeType == NODE_Group) { \
		if (X3D_GROUP(inNode)->children.n>0) { \
			outNode = (type)(X3D_GROUP(inNode)->children.p[0]); \
		} else outNode = NULL; \
	} else outNode = (type)inNode; };


#define MARK_NODE_COMPILED node->_ichange = node->_change;
#define NODE_NEEDS_COMPILING (node->_ichange != node->_change)
#define ROOTNODE_NEEDS_COMPILING (rootNode()->_ichange != rootNode()->_change)
/* end of compile simple nodes code */


#define MARK_SFFLOAT_INOUT_EVENT(good,save,offset) \
        if (!APPROX(good,save)) { \
                MARK_EVENT(X3D_NODE(node), offset);\
                save = good; \
        }

#define MARK_SFTIME_INOUT_EVENT(good,save,offset) \
        if (!APPROX(good,save)) { \
                MARK_EVENT(X3D_NODE(node), offset);\
                save = good; \
        }

#define MARK_SFINT32_INOUT_EVENT(good,save,offset) \
        if (good!=save) { \
                MARK_EVENT(X3D_NODE(node), offset);\
                save = good; \
        }

#define MARK_SFBOOL_INOUT_EVENT(good,save,offset) \
        if (good!=save) { \
                MARK_EVENT(X3D_NODE(node), offset);\
                save = good; \
        }

#define MARK_SFSTRING_INOUT_EVENT(good,save,offset) \
        if (good->strptr!=save->strptr) { \
                MARK_EVENT(X3D_NODE(node), offset);\
                save->strptr = good->strptr; \
        }

#define MARK_MFSTRING_INOUT_EVENT(good,save,offset) \
	/* assumes that the good pointer has been updated */ \
	if (good.p != save.p) { \
                MARK_EVENT(X3D_NODE(node), offset);\
		save.n = good.n; \
		save.p = good.p; \
        }

#define MARK_SFVEC3F_INOUT_EVENT(good,save,offset) \
        if ((!APPROX(good.c[0],save.c[0])) || (!APPROX(good.c[1],save.c[1])) || (!APPROX(good.c[2],save.c[2]))) { \
                MARK_EVENT(X3D_NODE(node), offset);\
                memcpy (&save.c, &good.c, sizeof (struct SFColor));\
        }

#define MARK_SFVEC3D_INOUT_EVENT(good,save,offset) \
        if ((!APPROX(good.c[0],save.c[0])) || (!APPROX(good.c[1],save.c[1])) || (!APPROX(good.c[2],save.c[2]))) { \
                MARK_EVENT(X3D_NODE(node), offset);\
                memcpy (&save.c, &good.c, sizeof (struct SFVec3d));\
        }


/* make sure that the SFNODE "save" field is not deleted when scenegraph is removed (dup pointer problem) */
#define MARK_SFNODE_INOUT_EVENT(good,save,offset) \
        if (good != save) { \
                MARK_EVENT(X3D_NODE(node), offset);\
                save = good; \
        }


#define MARK_MFNODE_INOUT_EVENT(good,save,offset) \
	/* assumes that the good pointer has been updated */ \
	if (good.p != save.p) { \
                MARK_EVENT(X3D_NODE(node), offset);\
		save.n = good.n; \
		save.p = good.p; \
        }

/* for deciding on using set_ SF fields, with nodes with explicit "set_" fields...  note that MF fields are handled by
the EVIN_AND_FIELD_SAME MACRO */

#define USE_SET_SFVEC3D_IF_CHANGED(setField,regField) \
if (!APPROX (node->setField.c[0],node->regField.c[0]) || \
        !APPROX(node->setField.c[1],node->regField.c[1]) || \
        !APPROX(node->setField.c[2],node->regField.c[2]) ) { \
        /* now, is the setField at our default value??  if not, we just use the regField */ \
        if (APPROX(node->setField.c[0], IO_FLOAT) && APPROX(node->setField.c[1],IO_FLOAT) && APPROX(node->setField.c[2],IO_FLOAT)) { \
		/* printf ("just use regField\n"); */ \
        } else { \
		 /* printf ("use the setField as the real poistion field\n"); */ \
        	memcpy (node->regField.c, node->setField.c, sizeof (struct SFVec3d)); \
	} \
}

#define USE_SET_SFROTATION_IF_CHANGED(setField,regField) \
if (!APPROX (node->setField.c[0],node->regField.c[0]) || \
        !APPROX(node->setField.c[1],node->regField.c[1]) || \
        !APPROX(node->setField.c[2],node->regField.c[2]) || \
        !APPROX(node->setField.c[3],node->regField.c[3]) ) { \
        /* now, is the setField at our default value??  if not, we just use the regField */ \
        if (APPROX(node->setField.c[0], IO_FLOAT) && APPROX(node->setField.c[1],IO_FLOAT) && APPROX(node->setField.c[2],IO_FLOAT) && APPROX(node->setField.c[3],IO_FLOAT)) { \
		/* printf ("just use SFRotation regField\n"); */ \
        } else { \
		/* printf ("use the setField SFRotation as the real poistion field\n");  */ \
        	memcpy (node->regField.c, node->setField.c, sizeof (struct SFRotation)); \
	} \
}




int find_key (int kin, float frac, float *keys);
void startOfLoopNodeUpdates(void);
//extern int HaveSensitive;
void zeroVisibilityFlag(void);
void setField_fromJavascript (struct X3D_Node *ptr, char *field, char *value, int isXML);
unsigned int setField_FromEAI (char *ptr);

#define EXTENTTOBBOX
#define INITIALIZE_EXTENT        { node->EXTENT_MAX_X = (float) -10000.0; \
        node->EXTENT_MAX_Y = (float) -10000.0; \
        node->EXTENT_MAX_Z = (float) -10000.0; \
        node->EXTENT_MIN_X = (float) 10000.0; \
        node->EXTENT_MIN_Y = (float) 10000.0; \
        node->EXTENT_MIN_Z = (float) 10000.0; }

/********************************
	Verbosity
*********************************/
#if defined(DEBUG) || defined(FW_DEBUG)
	/* define verbosity that should be forced-on when debug is set */


#else
#undef SETFIELDVERBOSE
/* Parsing & Lexing */
#undef CPARSERVERBOSE 

/* Java Class invocation */
#undef JSVRMLCLASSESVERBOSE 

/* child node parsing */
#undef CHILDVERBOSE

/* routing */
#undef CRVERBOSE

/* Javascript */
#undef JSVERBOSE

/* sensitive events */
#undef SEVERBOSE

/* Text nodes */
#undef TEXTVERBOSE

/* Texture processing */
#undef TEXVERBOSE

/* streaming from VRML to OpenGL internals. */
#undef STREAM_POLY_VERBOSE

/* collision */
#undef COLLISIONVERBOSE

/* Capabilities of x3dv and x3d */
#undef CAPABILITIESVERBOSE

#endif /* end of ifdef DEBUG */

/* number of tesselated coordinates allowed */
#define TESS_MAX_COORDS  200000

#define offset_of(p_type,field) ((unsigned int)(&(((p_type)NULL)->field)-NULL))

#define UNUSED(v) ((void) v)
#define ISUSED(v) ((void) v)

#define PI 3.141592653589793f

/* return TRUE if numbers are very close */
#define APPROX(a,b) (fabs((a)-(b))<0.00000001)
/* defines for raycasting: */

#define NORMAL_VECTOR_LENGTH_TOLERANCE 0.00001
/* (test if the vector part of a rotation is normalized) */
#define IS_ROTATION_VEC_NOT_NORMAL(rot)        ( \
       fabs(1-sqrt(rot.c[0]*rot.c[0]+rot.c[1]*rot.c[1]+rot.c[2]*rot.c[2])) \
               >NORMAL_VECTOR_LENGTH_TOLERANCE \
)

/* from VRMLC.pm */
//extern int sound_from_audioclip;
extern int global_lineProperties;
extern int global_fillProperties;
//extern float gl_linewidth;
//extern int soundWarned;
//extern int cur_hits;
//extern struct point_XYZ hyper_r1,hyper_r2;

extern struct X3D_Text *lastTextNode;

/* defines for raycasting: */
#define XEQ (APPROX(t_r1.x,t_r2.x))
#define YEQ (APPROX(t_r1.y,t_r2.y))
#define ZEQ (APPROX(t_r1.z,t_r2.z))
/* xrat(a) = ratio to reach coordinate a on axis x */
#define XRAT(a) (((a)-t_r1.x)/(t_r2.x-t_r1.x))
#define YRAT(a) (((a)-t_r1.y)/(t_r2.y-t_r1.y))
#define ZRAT(a) (((a)-t_r1.z)/(t_r2.z-t_r1.z))
/* mratx(r) = x-coordinate gotten by multiplying by given ratio */
#define MRATX(a) (t_r1.x + (a)*(t_r2.x-t_r1.x))
#define MRATY(a) (t_r1.y + (a)*(t_r2.y-t_r1.y))
#define MRATZ(a) (t_r1.z + (a)*(t_r2.z-t_r1.z))

/* POLYREP stuff */
#define POINT_FACES	16 /* give me a point, and it is in up to xx faces */

/* Function Prototypes */

void render_node(struct X3D_Node *node);

void rayhit(float rat, float cx,float cy,float cz, float nx,float ny,float nz,
float tx,float ty, char *descr) ;

void fwnorprint (float *norm);


/* not defined anywhere: */
/* void Extru_init_tex_cap_vals(); */


/* from the PNG examples */
unsigned char  *readpng_get_image(double display_exponent, int *pChannels,
		                       unsigned long *pRowbytes);

/* Used to determine in Group, etc, if a child is a local light; do comparison with this */
void LocalLight_Rend(void *nod_);
void saveLightState(int *);
void restoreLightState(int *);
#define LOCAL_LIGHT_SAVE int savedlight[8];
#define LOCAL_LIGHT_CHILDREN(a) \
	if ((node->_renderFlags & VF_localLight)==VF_localLight){saveLightState(savedlight); localLightChildren(a);}

#define LOCAL_LIGHT_OFF if ((node->_renderFlags & VF_localLight)==VF_localLight) { \
		restoreLightState(savedlight); }

void normalize_ifs_face (float *point_normal,
                         struct point_XYZ *facenormals,
                         int *pointfaces,
                        int mypoint,
                        int curpoly,
                        float creaseAngle);



#define RUNNINGONAMD64 (sizeof(void *) == 8)

/* Text node system fonts. On startup, freewrl checks to see where the fonts
 * are stored
 */
//extern float AC_LastDuration[];

//extern int SoundEngineStarted;

/* used to determine whether we have transparent materials. */
//extern int have_transparency;


/* current time */
//extern double TickTime;
//extern double lastTime;
double TickTime();
double lastTime();

/* number of triangles this rendering loop */
//extern int trisThisLoop;

/* saved rayhit and hyperhit */
//extern struct SFColor ray_save_posn, hyp_save_posn, hyp_save_norm;

/* set a node to be sensitive */
void setSensitive(struct X3D_Node *parent,struct X3D_Node *me);

/* bindable nodes */
extern GLint viewport[]; //true static
//extern struct sNaviInfo naviinfo;


extern double defaultExamineDist;


/* Sending events back to Browser (eg, Anchor) */
extern int wantEAI;

void *returnInterpolatorPointer (const char *x);

/* SAI code node interface return values  The meanings of
   these numbers can be found in the SAI java code */
#define X3DBoundedObject			1
#define X3DBounded2DObject 		 	2
#define X3DURLObject 			 	3
#define X3DAppearanceNode 			10
#define X3DAppearanceChildNode 			11
#define X3DMaterialNode 			12
#define X3DTextureNode 				13
#define X3DTexture2DNode 			14
#define X3DTexture3DNode 			15
#define X3DTextureTransformNode  		16
#define X3DTextureTransform2DNode 		17
#define X3DGeometryNode 			18
#define X3DTextNode 				19
#define X3DParametricGeometryNode 		20
#define X3DGeometricPropertyNode 		21
#define X3DColorNode				22
#define X3DCoordinateNode 			23
#define X3DNormalNode 				24
#define X3DTextureCoordinateNode 		25
#define X3DFontStyleNode 			26
#define X3DProtoInstance 			27
#define X3DChildNode 				28
#define X3DBindableNode 			29
#define X3DBackgroundNode 			30
#define X3DGroupingNode 			31
#define X3DShapeNode 				32
#define X3DInterpolatorNode 			33
#define X3DLightNode 				34
#define X3DScriptNode 				35
#define X3DSensorNode 				36
#define X3DEnvironmentalSensorNode 		37
#define X3DKeyDeviceSensorNode 			38
#define X3DNetworkSensorNode 			39
#define X3DPointingDeviceSensorNode 		40
#define X3DDragSensorNode 			41
#define X3DTouchSensorNode 			42
#define X3DSequencerNode  			43
#define X3DTimeDependentNode 			44
#define X3DSoundSourceNode 			45
#define X3DTriggerNode 				46
#define X3DInfoNode 				47
#define X3DShaderNode				48
#define X3DVertexAttributeNode			49
#define X3DProgrammableShaderObject		50
#define X3DUrlObject				51
#define X3DEnvironmentTextureNode 52
#define X3DSFNode				53 /* this is an "X3DNode" in the spec, but it conflicts with an internal def. */



void AddRemoveChildren (struct X3D_Node *parent, struct Multi_Node *tn, struct X3D_Node * *nodelist, int len, int ar, char * where, int lin);

void update_node(struct X3D_Node *ptr);
void update_renderFlag(struct X3D_Node *ptr, int flag);

void JSMaxAlloc(void);


int get_touched_flag(uintptr_t fptr, uintptr_t actualscript);
void getMultiElementtype(char *strp, struct Multi_Vec3f *tn, int eletype);
void CRoutes_RemoveSimple(struct X3D_Node* from, int fromOfs,
 struct X3D_Node* to, int toOfs, int len);
void CRoutes_RegisterSimple(struct X3D_Node* from, int fromOfs,
 struct X3D_Node* to, int toOfs, int len);
void CRoutes_Register(int adrem,        struct X3D_Node *from,
                                 int fromoffset,
				 struct X3D_Node *to,
				 int toOffset,
                                 int length,
                                 void *intptr,
                                 int scrdir,
                                 int extra);
void CRoutes_free(void);
void propagate_events(void);
int getRoutesCount(void);
void getField_ToJavascript (int num, int fromoffset);
void add_first(struct X3D_Node * node);
void registerTexture(struct X3D_Node * node);
void registerMIDINode(struct X3D_Node *node);
int checkNode(struct X3D_Node *node, char *fn, int line);


void do_first(void);
void process_eventsProcessed(void);

#ifdef DJTRACK_PICKSENSORS
/* DJTRACK_PICKSENSORS */
void add_picksensor(struct X3D_Node * node);
void rewind_picksensors();
void advance_picksensors();
void activate_picksensors();
void deactivate_picksensors();
int active_picksensors();
int more_picksensors();
struct X3D_Node* get_picksensor();
#endif

void getEAI_MFStringtype (struct Multi_String *from, struct Multi_String *to);


void update_status(char* msg);
void kill_status();

/* menubar stuff */
void frontendUpdateButtons(void); /* used only if we are not able to multi-thread OpenGL */
/* IS -- moved these into libFreeWRL.h as front-ends call some of them directly
void setMenuButton_collision (int val) ;
void setMenuButton_headlight (int val) ;
void setMenuButton_navModes (int type) ;
void setMenuButton_texSize (int size); */
void setConsoleMessage(char *stat) ;
void fwl_reload(void);

int convert_typetoInt (const char *type);	/* convert a string, eg "SFBOOL" to type, eg SFBOOL */

//extern double BrowserFPS;
//extern double BrowserSpeed;
void render_polyrep(void *node);

//extern int CRoutesExtra;		/* let EAI see param of routing table - Listener data. */

/* types of scripts. */
#define NOSCRIPT 	0
#define JAVASCRIPT	1
#define SHADERSCRIPT	4

/* printf is defined by perl; causes segfault in threaded freewrl */
#ifdef printf
#undef printf
#endif
#ifdef die
#undef die
#endif

//extern struct X3D_Group *rootNode;
struct X3D_Group *rootNode();
void setRootNode(struct X3D_Group *rn);

extern int isPerlParsing(void);
/* extern int isURLLoaded(void);	/\* initial scene loaded? Robert Sim *\/ */
extern int isTextureParsing(void);
extern void loadMovieTexture(struct X3D_MovieTexture *node,  void *param);
extern void loadMultiTexture(struct X3D_MultiTexture *node);
extern void loadBackgroundTextures (struct X3D_Background *node);
extern void loadTextureBackgroundTextures (struct X3D_TextureBackground *node);
extern GLfloat boxtex[], boxnorms[], BackgroundVert[];
extern GLfloat Backnorms[];

extern void new_tessellation(void);
extern void initializePerlThread(void);
//extern void setWantEAI(int flag);
extern void setPluginPipe(const char *optarg);
extern void setPluginFD(const char *optarg);
extern void setPluginInstance(const char *optarg);

extern int isPerlinitialized(void);

extern char *getInputURL(void);
extern char *lastReadFile; 		/* name last file read in */
struct sCollisionInfo* CollisionInfo();
struct sFallInfo* FallInfo();

extern void xs_init(void);

//extern int navi_tos;
extern void checkAndAllocMemTables(int *texture_num, int increment);
extern void   storeMPGFrameData(int latest_texture_number, int h_size, int v_size,
        int mt_repeatS, int mt_repeatT, char *Image);
void mpg_main(char *filename, int *x,int *y,int *depth,int *frameCount,void **ptr);
void removeFilenameFromPath (char *path);

int EAI_CreateVrml(const char *tp, const char *inputstring, struct X3D_Group *node);
void EAI_Route(char cmnd, const char *tf);

void handle_EAI(void);
void handle_MIDIEAI(void);
void handle_aqua(const int mev, const unsigned int button, int x, int y);

#define overMark        23425

/* mimic X11 events in AQUA and/or WIN32 ; FIXME: establish a cleaner interface for this */
#define KeyChar         1

#if defined(AQUA) || defined(_MSC_VER) || defined(_ANDROID)
#ifndef _MIMIC_X11_SCREEN_BUTTONS
	#define _MIMIC_X11_SCREEN_BUTTONS
		#define KeyPress        2
		#define KeyRelease      3
		#define ButtonPress     4
		#define ButtonRelease   5
		#define MotionNotify    6
		#define MapNotify       19
	#endif
#endif

/* extern void setSnapSeq(); */
extern void setEAIport(int pnum);
/* extern void setKeyString(const char *str); */
extern void setNoCollision();
/* extern void setSeqFile(const char* file); */
/* extern void setMaxImages(int max); */
extern void setBrowserFullPath(const char *str);
extern void setInstance(uintptr_t instance);

extern const char *getLibVersion();
extern int doBrowserAction ();


extern char *myPerlInstallDir;

/* for Extents and BoundingBoxen */
#define EXTENT_MAX_X _extent[0]
#define EXTENT_MIN_X _extent[1]
#define EXTENT_MAX_Y _extent[2]
#define EXTENT_MIN_Y _extent[3]
#define EXTENT_MAX_Z _extent[4]
#define EXTENT_MIN_Z _extent[5]

void freewrlDie(const char *format);

/* children stuff moved out of VRMLRend.pm and VRMLC.pm for v1.08 */

//extern int render_sensitive,render_vp,render_light,render_proximity,render_other,verbose,render_blend,render_geom,render_collision;
typedef struct trenderstate{
int render_sensitive,render_vp,render_light,render_proximity,render_other,verbose,render_blend,render_geom,render_collision;
#ifdef DJTRACK_PICKSENSORS
int render_picksensors;
int render_pickables;
#endif
}* ttrenderstate;
//extern struct trenderstate renderstate;
ttrenderstate renderstate();

int SAI_IntRetCommand (char cmnd, const char *fn);
char * SAI_StrRetCommand (char cmnd, const char *fn);
char *EAI_GetTypeName (unsigned int uretval);
char* EAI_GetValue(unsigned int nodenum, const char *fieldname, const char *nodename);

void add_parent(struct X3D_Node *node_, struct X3D_Node *parent_,char *file, int line);
void remove_parent(struct X3D_Node *child, struct X3D_Node *parent);
void EAI_readNewWorld(char *inputstring);



void make_genericfaceset(struct X3D_IndexedFaceSet *this_);
#define rendray_Text render_ray_polyrep
#define rendray_VRML1_IndexedFaceSet render_ray_polyrep
#define make_VRML1_IndexedFaceSet make_genericfaceset
#define collide_VRML1_IndexedFaceSet collide_genericfaceset
#define rendray_ElevationGrid  render_ray_polyrep
#define collide_ElevationGrid collide_genericfaceset
#define rendray_Extrusion render_ray_polyrep
#define rendray_IndexedFaceSet render_ray_polyrep 
#define make_IndexedFaceSet make_genericfaceset
#define make_ElevationGrid make_genericfaceset
#define rendray_ElevationGrid render_ray_polyrep

/* Component Rendering nodes */
#define rendray_IndexedTriangleSet render_ray_polyrep
#define rendray_IndexedTriangleFanSet render_ray_polyrep
#define rendray_IndexedTriangleStripSet render_ray_polyrep
#define rendray_TriangleSet render_ray_polyrep
#define rendray_TriangleFanSet render_ray_polyrep
#define rendray_TriangleStripSet render_ray_polyrep
#define collide_IndexedFaceSet collide_genericfaceset
#define collide_IndexedTriangleFanSet  collide_genericfaceset
#define collide_IndexedTriangleSet  collide_genericfaceset
#define collide_IndexedTriangleStripSet  collide_genericfaceset
#define collide_TriangleFanSet  collide_genericfaceset
#define collide_TriangleSet  collide_genericfaceset
#define collide_TriangleStripSet  collide_genericfaceset
#define make_IndexedTriangleFanSet  make_genericfaceset
#define make_IndexedTriangleSet  make_genericfaceset
#define make_IndexedTriangleStripSet  make_genericfaceset
#define make_TriangleFanSet  make_genericfaceset
#define make_TriangleSet  make_genericfaceset
#define make_TriangleStripSet  make_genericfaceset
#define rendray_GeoElevationGrid  render_ray_polyrep
#define collide_GeoElevationGrid collide_genericfaceset
#define make_GeoElevationGrid make_genericfaceset


#ifdef OLDCODE
OLDCODE /* MIDI stuff... */
OLDCODE void ReWireRegisterMIDI (char *str);
OLDCODE void ReWireMIDIControl(char *str);

OLDCODEvoid MIDIRegisterMIDI(char *str);
OLDCODE/* ReWire device/controller  table */
OLDCODEstruct ReWireDeviceStruct {
OLDCODE        struct X3D_MidiControl* node;   /* pointer to the node that controls this */
OLDCODE        int encodedDeviceName;          /* index into ReWireNamenames */
OLDCODE        int bus;                        /* which MIDI bus this is */
OLDCODE        int channel;                    /* which MIDI channel on this bus it is */
OLDCODE        int encodedControllerName;      /* index into ReWireNamenames */
OLDCODE        int controller;                 /* controller number */
OLDCODE        int cmin;                       /* minimum value for this controller */
OLDCODE        int cmax;                       /* maximum value for this controller */
OLDCODE        int ctype;                      /* controller type TYPE OF FADER control - not used currently */
OLDCODE};
OLDCODE
OLDCODE/* ReWireName table */
OLDCODEstruct ReWireNamenameStruct {
OLDCODE        char *name;
OLDCODE};
#endif // OLDCODE


/* Event Utilities Component */
void do_BooleanFilter (void *node);
void do_BooleanSequencer (void *node);
void do_BooleanToggle (void *node);
void do_BooleanTrigger (void *node);
void do_IntegerSequencer (void *node);
void do_IntegerTrigger (void *node);
void do_TimeTrigger (void *node);


#define ADD_PARENT(a,b) add_parent(a,b,__FILE__,__LINE__)
#define NODE_ADD_PARENT(a) ADD_PARENT(a,X3D_NODE(ptr))
#define NODE_REMOVE_PARENT(a) ADD_PARENT(a,X3D_NODE(ptr))


#ifdef GL_ES_VERSION_2_0
	#define LIGHTING_ON if (!gglobal()->RenderFuncs.lightingOn) {gglobal()->RenderFuncs.lightingOn=TRUE;} 
	#define LIGHTING_OFF if(gglobal()->RenderFuncs.lightingOn) {gglobal()->RenderFuncs.lightingOn=FALSE;}

	#define LIGHTING_INITIALIZE gglobal()->RenderFuncs.lightingOn=TRUE;

#else 
	#define LIGHTING_ON if (!gglobal()->RenderFuncs.lightingOn) {gglobal()->RenderFuncs.lightingOn=TRUE;FW_GL_ENABLE(GL_LIGHTING);}
	#define LIGHTING_OFF if(gglobal()->RenderFuncs.lightingOn) {gglobal()->RenderFuncs.lightingOn=FALSE;FW_GL_DISABLE(GL_LIGHTING);}
	#define LIGHTING_INITIALIZE gglobal()->RenderFuncs.lightingOn=TRUE; FW_GL_ENABLE(GL_LIGHTING);
#endif /* GL_ES_VERSION_2_0 */

void zeroAllBindables(void);

int inputParse(unsigned type, char *inp, int bind, int returnifbusy,
                        void *ptr, unsigned ofs, int *complete,
                        int zeroBind);
void compileNode (void (*nodefn)(void *, void *, void *, void *, void *), void *node, void *a, void *b, void *c, void *d);
void destroyCParserData();
//extern struct VRMLParser* savedParser;

void getMovieTextureOpenGLFrames(int *highest, int *lowest,int myIndex);
int ConsoleMessage(const char *fmt, ...);
/* >>> statusbar hud change */
#define NEW_CONSOLEMESSAGE_VERSION 1
#ifdef NEW_CONSOLEMESSAGE_VERSION
int BrowserPrintConsoleMessage(const char *fmt, ...);
//extern int Console_writeToCRT;
//extern int Console_writeToFile;
//extern int Console_writeToHud;
#endif
/* <<< statusbar hud changes */
void closeConsoleMessage(void);
//extern int consMsgCount;

void outOfMemory(const char *message);

void killErrantChildren(void);

void kill_routing(void);
void kill_bindables(void);
void kill_javascript(void);
void kill_oldWorld(int kill_EAI, int kill_JavaScript, char *file, int line);
void kill_clockEvents(void);
void kill_openGLTextures(void);
void kill_X3DDefs(void);
//extern int currentFileVersion;

int countCommas (char *instr);
void dirlightChildren(struct Multi_Node ch);
void normalChildren(struct Multi_Node ch);
void checkParentLink (struct X3D_Node *node,struct X3D_Node *parent);

/* background colour */
void setglClearColor (float *val); 
void doglClearColor(void);
extern int cc_changed;

/* int mapFieldTypeToInernaltype (indexT kwIndex); msvc has problem perhaps with typedef typdef unsigned int indexT*/
int mapFieldTypeToInernaltype (unsigned int kwIndex);
void resetSensorEvents();


/* META data, component, profile  stuff */
void handleMetaDataStringString(struct Uni_String *val1,struct Uni_String *val2);
void handleProfile(int myp);
void handleComponent(int com, int lev);
void handleExport (char *node, char *as);
void handleImport (char *nodeName,char *nodeImport, char *as);
void handleVersion (const char *versionString);


/* free memory */
void registerX3DNode(struct X3D_Node * node);

void doNotRegisterThisNodeForDestroy(struct X3D_Node * nodePtr);

struct Multi_Vec3f *getCoordinate (struct X3D_Node *node, char *str);

void replaceWorldNeeded(char* str);

/* X3D C parser */
int X3DParse(struct X3D_Group *parent, const char *inputstring);
void *createNewX3DNode (int nt);

char* convert1To2(const char *inp);

//updateStatusBar(void);

#endif /* __FREEWRL_HEADERS_H__ */
