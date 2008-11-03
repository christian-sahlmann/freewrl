/*******************************************************************
 *
 * FreeX3D support library
 *
 * public API - libFreeX3D.h
 *
 * $Id: old_stuff.h,v 1.1 2008/11/03 17:09:01 couannette Exp $
 *
 *******************************************************************/

#ifndef __LIBFREEX3D_MAIN_H__
#define __LIBFREEX3D_MAIN_H__

/**
 * Version embedded
 */
extern const char *libFreeX3D_get_version();


/**
 * Initialization
 */
void initFreewrl();
void closeFreewrl();

/**
 * General functions
 */
int ConsoleMessage(const char *fmt, ...);

/**
 * General variables
 */
char *BrowserFullPath;
int be_collision;
char *keypress_string;

/**
 * Plugin functions
 */

/**
 * Plugin variables
 */
int isBrowserPlugin;
int _fw_pipe;
int _fw_browser_plugin;
unsigned _fw_instance;

/**
 * Network functions
 */
int checkNetworkFile(char *fn);
void setFullPath(const char* file);
void makeAbsoluteFileName(char *filename, char *pspath,char *thisurl);

/**
 * Network variables
 */

/**
 * Display functions
 */
void resetGeometry();
void setLineWidth(float lwidth);

/**
 * Display variables
 */
int feHeight, feWidth;
int fullscreen;
float gl_linewidth;

/**
 * Threading functions
 */
#define STOP_DISPLAY_THREAD \
        if (DispThrd != NULL) { \
                quitThread = TRUE; \
                pthread_join(DispThrd,NULL); \
                DispThrd = NULL; \
        }

void displayThread();

/**
 * Threading variables
 */
pthread_t DispThrd;
int quitThread;

/**
 * EAI functions
 */
void create_EAI();
void shutdown_EAI(void);
void setEaiVerbose();

/**
 * EAI variables
 */
int wantEAI;
int EAIverbose;

/**
 * ALL INCLUDES FROM CURRENT FREEWRL/CFuncs : we must organize all that stuff ;) !
 *
 * 1. strip all #include
 *    except VrmlTypeList.h
 * 2. strip all #ifdef #ifndef #endif related to include armor
 * 3. strip all declarations that we know we handle with system.h
 * 4. strip VrmlTypeList.h declaration and recreate that file
 */

#include "Structs.h" /* generated code */

/*
  aquaInterface.h
  FreeWRL

  Created by Sarah Dumoulin on Mon Jan 19 2004.
  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
*/

void updateContext();
float getWidth();
float getHeight();
void  setAquaCursor(int ctype);
void setMenuButton_collision(int val);
void setMenuButton_texSize(int size);
void setMenuStatus(char* stat);
void setMenuButton_headlight(int val);
void setMenuFps(float fps);
int aquaSetConsoleMessage(char* str);
void setMenuButton_navModes(int type);
void createAutoReleasePool();
/*******************************************************************
 Copyright (C) 2003 John Stewart, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*********************************************************************/

/*****************************************

Bindable nodes - Background, TextureBackground, Fog, NavigationInfo, Viewpoint.

******************************************/

/* Bind stack */
#define MAX_STACK 20

extern GLint viewPort[];

/* Bindables, Viewpoint, NavigationInfo, Background, TextureBackground and Fog */
extern void * *fognodes;
extern void * *backgroundnodes;
extern void * *navnodes;
extern void * *viewpointnodes;
extern int totfognodes, totbacknodes, totnavnodes, totviewpointnodes;
extern int currboundvpno;

extern int viewpoint_tos;
extern int background_tos;
extern int fog_tos;
extern int navi_tos;

extern uintptr_t viewpoint_stack[];
extern uintptr_t navi_stack[];

void reset_upvector(void);
void set_naviinfo(struct X3D_NavigationInfo *node);
void send_bind_to(struct X3D_Node *node, int value);
void bind_node(struct X3D_Node *node, int *tos, uintptr_t *stack);
void render_Fog(struct X3D_Fog *node);
void render_NavigationInfo(struct X3D_NavigationInfo *node);
void render_Background(struct X3D_Background *node);
void render_TextureBackground(struct X3D_TextureBackground *node);

/* This is a common base class for FieldDeclarations on PROTOs and Scripts */

/* ************************************************************************** */
/* ************************************ FieldDecl *************************** */
/* ************************************************************************** */

/* Struct */
/* ****** */

struct FieldDecl
{
 indexT mode; /* field, exposedField, eventIn, eventOut */
 indexT type; /* field type */
 indexT name; /* field "name" (its lexer-index) */
};

/* Constructor and destructor */
/* ************************** */

struct FieldDecl* newFieldDecl(indexT, indexT, indexT);
#define deleteFieldDecl(me) \
 FREE_IF_NZ(me)

/* Copies */
#define fieldDecl_copy(me) \
 newFieldDecl((me)->mode, (me)->type, (me)->name)

/* Accessors */
/* ********* */

#define fieldDecl_getType(me) \
 ((me)->type)
#define fieldDecl_getAccessType(me) \
 ((me)->mode)
#define fieldDecl_getIndexName(me) \
 ((me)->name)
#define fieldDecl_getStringName(lex, me) \
 lexer_stringUser_fieldName(lex, fieldDecl_getIndexName(me), \
  fieldDecl_getAccessType(me))

/* Other members */
/* ************* */

/* Check if this is a given field */
#define fieldDecl_isField(me, nam, mod) \
 ((me)->name==(nam) && (me)->mode==(mod))

/*
 * Copyright (C) 2002 Nicolas Coderre CRC Canada
 * DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 * See the GNU Library General Public License (file COPYING in the distribution)
 * for conditions of use and redistribution.
 */

/* Collision detection results structure*/
struct sCollisionInfo {
    struct point_XYZ Offset;
    int Count;
    double Maximum2; /*squared. so we only need to root once */
};

typedef int prflags;
#define PR_DOUBLESIDED 0x01
#define PR_FRONTFACING 0x02 /* overrides effect of doublesided. */
#define PR_BACKFACING 0x04 /* overrides effect of doublesided, all normals are reversed. */
#define PR_NOSTEPING 0x08 /* gnores stepping. used internally */


/*uncomment this to enable the scene exporting functions */
/*#define DEBUG_SCENE_EXPORT*/

double
closest_point_of_segment_to_y_axis(struct point_XYZ p1,
								   struct point_XYZ p2);
double
closest_point_of_segment_to_origin(struct point_XYZ p1,
								   struct point_XYZ p2);

struct point_XYZ
closest_point_of_plane_to_origin(struct point_XYZ b,
								 struct point_XYZ n);

int
intersect_segment_with_line_on_yplane(struct point_XYZ* pk,
									  struct point_XYZ p1,
									  struct point_XYZ p2,
									  struct point_XYZ q1,
									  struct point_XYZ q2);

int
getk_intersect_line_with_ycylinder(double* k1,
								   double* k2,
								   double r,
								   struct point_XYZ pp1,
								   struct point_XYZ n);

int
project_on_cylindersurface(struct point_XYZ* res,
						   struct point_XYZ p,
						   struct point_XYZ n,
						   double r);

int
getk_intersect_line_with_sphere(double* k1,
								double* k2,
								double r,
								struct point_XYZ pp1,
								struct point_XYZ n);

int
project_on_spheresurface(struct point_XYZ* res,
						 struct point_XYZ p,
						 struct point_XYZ n,
						 double r);

struct point_XYZ
project_on_yplane(struct point_XYZ p1,
				  struct point_XYZ n,
				  double y);

struct point_XYZ
project_on_cylindersurface_plane(struct point_XYZ p,
								 struct point_XYZ n,
								 double r);

int
perpendicular_line_passing_inside_poly(struct point_XYZ a,
									   struct point_XYZ* p,
									   int num);

int
getk_intersect_segment_with_ycylinder(double* k1,
									  double* k2,
									  double r,
									  struct point_XYZ pp1,
									  struct point_XYZ pp2);

int
helper_poly_clip_cap(struct point_XYZ* clippedpoly,
					 int clippedpolynum,
					 const struct point_XYZ* p,
					 int num,
					 double r,
					 struct point_XYZ n,
					 double y,
					 int stepping);

struct point_XYZ
polyrep_disp_rec(double y1,
				 double y2,
				 double ystep,
				 double r,
				 struct X3D_PolyRep* pr,
				 struct point_XYZ* n,
				 struct point_XYZ dispsum,
				 prflags flags);

struct point_XYZ
planar_polyrep_disp_rec(double y1,
						double y2,
						double ystep,
						double r,
						struct X3D_PolyRep* pr,
						struct point_XYZ n,
						struct point_XYZ dispsum,
						prflags flags);

int
helper_line_clip_cap(struct point_XYZ* clippedpoly,
					 int clippedpolynum,
					 struct point_XYZ p1,
					 struct point_XYZ p2,
					 double r,
					 struct point_XYZ n,
					 double y,
					 int stepping);

/*accumulator function, for displacements. */
void accumulate_disp(struct sCollisionInfo* ci, struct point_XYZ add);

/*returns (1-k)p1 + k p2 */
struct point_XYZ weighted_sum(struct point_XYZ p1, struct point_XYZ p2, double k);

/*feed a poly, and stats of a cylinder, it returns the displacement in the direction of the
  normal of the poly that is needed for them not to intersect any more.*/
struct point_XYZ get_poly_normal_disp(double y1, double y2, double r, struct point_XYZ* p, int num, struct point_XYZ n);

/*feed a poly, and stats of a cylinder, it returns the vertical displacement that is needed for them not to intersect any more,
  if this displacement is less than the height of the cylinder (y2-y1).*/
struct point_XYZ get_poly_step_disp(double y1, double y2, double r, struct point_XYZ* p, int num, struct point_XYZ n);

/*feed a poly, and stats of a cylinder, it returns the displacement in the direction of the
  normal of the poly that is needed for them not to intersect any more, or vertically if contact point below ystep*/
struct point_XYZ get_poly_disp(double y1, double y2, double ystep, double r, struct point_XYZ* p, int num, struct point_XYZ n);

/*feed a poly, and radius of a sphere, it returns the displacement in the direction of the
  normal of the poly that is needed for them not to intersect any more.*/
struct point_XYZ get_poly_normal_disp_with_sphere(double r, struct point_XYZ* p, int num, struct point_XYZ n);
/*feed a poly, and radius of a sphere, it returns the minimum displacement and
  the direction that is needed for them not to intersect any more.*/
struct point_XYZ get_poly_min_disp_with_sphere(double r, struct point_XYZ* p, int num, struct point_XYZ n);

/*feed a line and a normal, and stats of a cylinder, it returns the displacement in the direction of the
  normal that is needed for them not to intersect any more.*/
struct point_XYZ get_line_normal_disp(double y1, double y2, double r, struct point_XYZ p1, struct point_XYZ p2, struct point_XYZ n);

/*feed a line and a normal, and stats of a cylinder, it returns the vertical displacement
  that is needed for them not to intersect any more.*/
struct point_XYZ get_line_step_disp(double y1, double y2, double r, struct point_XYZ p1, struct point_XYZ p2, struct point_XYZ n);

/*feed a line and a normal, and stats of a cylinder, it returns the displacement in the direction of the
  normal, or the vertical displacement(in case of stepping) that is needed for them not to intersect any more.*/
struct point_XYZ get_line_disp(double y1, double y2, double ystep, double r, struct point_XYZ p1, struct point_XYZ p2, struct point_XYZ n);

/*feed a point and a normal, and stats of a cylinder, it returns the displacement in the direction of the
  normal that is needed for them not to intersect any more.*/
struct point_XYZ get_point_normal_disp(double y1, double y2, double r, struct point_XYZ p1, struct point_XYZ n);

/*feed a point and a normal, and stats of a cylinder, it returns the displacement in the direction of the
  normal, or the vertical displacement(in case of stepping) that is needed for them not to intersect any more.*/
struct point_XYZ get_point_disp(double y1, double y2, double ystep, double r, struct point_XYZ p1, struct point_XYZ n);

/*feed a box (a corner, and the three vertice sides) and the stats of a cylinder, it returns the
  displacement of the box that is needed for them not to intersect any more, with optionnal stepping displacement */
struct point_XYZ box_disp(double y1, double y2, double ystep, double r,struct point_XYZ p0, struct point_XYZ i, struct point_XYZ j, struct point_XYZ k);

/*fast test to see if a box intersects a y-cylinder.
 * gives false positives */
int fast_ycylinder_box_intersect(double y1, double y2, double r,struct point_XYZ pcenter, double xs, double ys, double zs);


/*fast test to see if the min/max of a polyrep structure (IndexedFaceSet, eg)  intersects a y-cylinder.
 * gives false positives */
int fast_ycylinder_polyrep_intersect(double y1, double y2, double r,struct point_XYZ pcenter, double scale, struct X3D_PolyRep *pr);

/*fast test to see if a cone intersects a y-cylinder. */
/*gives false positives. */
int fast_ycylinder_cone_intersect(double y1, double y2, double r,struct point_XYZ pcenter, double halfheight, double baseradius);

/* fast test to see if a sphere intersects a y-cylinder.
   specify sphere center, and a point on it's surface
  gives false positives. */
int fast_ycylinder_sphere_intersect(double y1, double y2, double r,struct point_XYZ pcenter, struct point_XYZ psurface);


/*algorithm is approximative */
/*basically, it does collision with a triangle on a plane that passes through the origin.*/
struct point_XYZ cone_disp(double y1, double y2, double ydisp, double r, struct point_XYZ base, struct point_XYZ top, double baseradius);

/*algorithm is approximative */
/*basically, it does collision with a rectangle on a plane that passes through the origin.*/
struct point_XYZ cylinder_disp(double y1, double y2, double ydisp, double r, struct point_XYZ base, struct point_XYZ top, double baseradius);

/*uses sphere displacement, and a cylinder for stepping */
struct point_XYZ polyrep_disp(double y1, double y2, double ydisp, double r, struct X3D_PolyRep pr, GLdouble* mat, prflags flags);

/*displacement when the polyrep structure is all in the same plane
  if normal is zero, it will be calculated form the first triangle*/
struct point_XYZ planar_polyrep_disp(double y1, double y2, double ydisp, double r, struct X3D_PolyRep pr, GLdouble* mat, prflags flags, struct point_XYZ n);

struct point_XYZ elevationgrid_disp( double y1, double y2, double ydisp, double r, struct X3D_PolyRep pr,
			      int xdim, int zdim, double xs, double zs, GLdouble* mat, prflags flags);

/* functions VERY usefull for debugging purposes
   Use these inside FreeWRL to export a scene to
   the debugging programs. */
#ifdef DEBUG_SCENE_EXPORT
void printpolyrep(struct X3D_PolyRep pr, int npoints);

void printmatrix(GLdouble* mat);
#endif

void render_ComposedCubeMapTexture(struct X3D_ComposedCubeMapTexture *node);
void render_GeneratedCubeMapTexture(struct X3D_GeneratedCubeMapTexture *node);
void render_ImageCubeMapTexture(struct X3D_ImageCubeMapTexture *node);
/*******************************************************************
 Copyright (C) 2003 John Stewart, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*********************************************************************/

/* General header for VRML-parser (lexer/parser) */

/* Typedefs for VRML-types. */
typedef int	vrmlBoolT;
typedef struct SFColor	vrmlColorT;
typedef struct SFColorRGBA	vrmlColorRGBAT;
typedef float	vrmlFloatT;
typedef int32_t	vrmlInt32T;
typedef struct Multi_Int32	vrmlImageT;
typedef struct X3D_Node*	vrmlNodeT;
typedef struct SFRotation	vrmlRotationT;
typedef struct Uni_String*	vrmlStringT;
typedef double	vrmlTimeT;
typedef double	vrmlDoubleT;
typedef struct SFVec2f	vrmlVec2fT;
typedef struct SFVec2d	vrmlVec2dT;
typedef struct SFVec4f	vrmlVec4fT;
typedef struct SFVec4d	vrmlVec4dT;
typedef struct SFColor	vrmlVec3fT;
typedef struct SFVec3d  vrmlVec3dT;
typedef struct SFMatrix3f	vrmlMatrix3fT;
typedef struct SFMatrix3d vrmlMatrix3dT;
typedef struct SFMatrix4f	vrmlMatrix4fT;
typedef struct SFMatrix4d vrmlMatrix4dT;

/* This is an union to hold every vrml-type */
union anyVrml
{
 #define SF_TYPE(fttype, type, ttype) \
  vrml##ttype##T type;
 #define MF_TYPE(fttype, type, ttype) \
  struct Multi_##ttype type;
 #include "VrmlTypeList.h"
 #undef SF_TYPE
 #undef MF_TYPE
};

#define parseError(msg) \
 (ConsoleMessage("Parse error:  " msg "\n"), fprintf(stderr, msg "\n")) \

#define CPARSE_ERROR_CURID(str) \
		strcpy (fw_outline,str); \
		strcat (fw_outline,"expected colon in COMPONENT statement, found \""); \
		if (me->lexer->curID != ((void *)0)) strcat (fw_outline, me->lexer->curID); \
		else strcat (fw_outline, "(EOF)"); \
		strcat (fw_outline,"\" "); \
		ConsoleMessage(fw_outline); \
		fprintf (stderr,"%s\n",fw_outline);

/* tie assert in here to give better failure methodology */
#define ASSERT(cond) if(!(cond)){fw_assert(__FILE__,__LINE__);}
void fw_assert(char *,int);

/* VRML-parsing routines in C. */

/* for scanning and determining whether a character is part of a valid X3D name */
#define IS_ID_REST(c) \
 (c>0x20 && c!=0x22 && c!=0x23 && c!=0x27 && c!=0x2C && c!=0x2E && c!=0x3a && c!=0x5B && \
  c!=0x5C && c!=0x5D && c!=0x7B && c!=0x7D && c!=0x7F)
#define IS_ID_FIRST(c) \
 (IS_ID_REST(c) && (c<0x30 || c>0x39) && c!=0x2B && c!=0x2D)

BOOL cParse(void*, unsigned, const char*);

/* Destroy all data associated with the currently parsed world kept. */
#define destroyCParserData(me) \
 parser_destroyData(me)

/* Some accessor-methods */
struct X3D_Node* parser_getNodeFromName(const char*);
extern struct VRMLParser* globalParser;

/* tie assert in here to give better failure methodology */
#define ASSERT(cond) if(!(cond)){fw_assert(__FILE__,__LINE__);}
void fw_assert(char *,int);


/* 
Vector.h
General purpose containers - vector and stack (implemented on top of it)
*/

/* ************************************************************************** */
/* ******************************** Vector ********************************** */
/* ************************************************************************** */

/* This is the vector structure. */
struct Vector
{
 size_t	n;
 size_t	allocn;
 void*	data;
};

/* Constructor/destructor */
struct Vector* newVector_(size_t elSize, size_t initSize);
#define newVector(type, initSize) \
 newVector_(sizeof(type), initSize)

#ifdef DEBUG_MALLOC
	void deleteVector_(char *file, int line, size_t elSize, struct Vector*);
	#define deleteVector(type, me) deleteVector_(__FILE__,__LINE__,sizeof(type), me)
#else
	void deleteVector_(size_t elSize, struct Vector*);
	#define deleteVector(type, me) deleteVector_(sizeof(type), me)
#endif

/* Ensures there's at least one space free. */
void vector_ensureSpace_(size_t, struct Vector*);

/* Element retrieval. */
#define vector_get(type, me, ind) \
 ((type*)((struct Vector*)me)->data)[ind]

/* Size of vector */
#define vector_size(me) \
 (((struct Vector*)me)->n)

/* Back of a vector */
#define vector_back(type, me) \
 vector_get(type, me, vector_size(me)-1)

/* Is the vector empty? */
#define vector_empty(me) \
 (!vector_size(me))

/* Shrink the vector to minimum required space. */
void vector_shrink_(size_t, struct Vector*);
#define vector_shrink(type, me) \
 vector_shrink_(sizeof(type), me)

/* Push back operation. */
#define vector_pushBack(type, me, el) \
 { \
  vector_ensureSpace_(sizeof(type), me); \
  ASSERT(((struct Vector*)me)->n<((struct Vector*)me)->allocn); \
  vector_get(type, me, ((struct Vector*)me)->n)=el; \
  ++((struct Vector*)me)->n; \
 }

/* Pop back operation */
#define vector_popBack(type, me) \
 { \
  ASSERT(!vector_empty(me)); \
  --((struct Vector*)me)->n; \
 }
#define vector_popBackN(type, me, popn) \
 { \
  ASSERT(popn<=vector_size(me)); \
  ((struct Vector*)me)->n-=popn; \
 }

/* Release and get vector data. */
void* vector_releaseData_(size_t, struct Vector*);
#define vector_releaseData(type, me) \
 vector_releaseData_(sizeof(type), me)

/* ************************************************************************** */
/* ************************************ Stack ******************************* */
/* ************************************************************************** */

/* A stack is essentially a vector */
typedef struct Vector Stack;

/* Constructor and destructor */
#define newStack(type) \
 newVector(sizeof(type), 4)
#define deleteStack(type, me) \
 deleteVector(sizeof(type), me)

/* Push and pop */
#define stack_push(type, me, el) \
 vector_pushBack(type, me, el)
#define stack_pop(type, me) \
 vector_popBack(type, me)

/* Top of stack */
#define stack_top(type, me) \
 vector_get(type, me, vector_size(me)-1)

/* Is the stack empty? */
#define stack_empty(me) \
 vector_empty(me)

/* tie assert in here to give better failure methodology */
#define ASSERT(cond) if(!(cond)){fw_assert(__FILE__,__LINE__);}
void fw_assert(char *,int);




/* Lexer (input of terminal symbols) for CParse */

/* Tables of user-defined IDs:
 * userNodeNames (DEFs) is scoped with a simple stack, as every PROTO has its
   scope completely *different* from the rest of the world.
 * userNodeTypes (PROTO definitions) needs to be available up through the whole
   stack, values are stored in a vector, and the indices where each stack level
   ends are stored in a stack.
 * fields are not scoped and therefore stored in a simple vector.
 */

/* Undefined ID (for special "class", like builtIn and exposed) */
#ifdef ID_UNDEFINED
#undef ID_UNDEFINED
#endif
#define ID_UNDEFINED	((indexT)-1)

/* This is our lexer-object. */
struct VRMLLexer
{
 const char* nextIn;	/* Next input character. */
 const char* startOfStringPtr; /* beginning address of string, for FREE calls */
 char* curID;	/* Currently input but not lexed id. */
 BOOL isEof;	/* Error because of EOF? */
 Stack* userNodeNames;
 struct Vector* userNodeTypesVec;
 Stack* userNodeTypesStack;
 struct Vector* user_initializeOnly;
 struct Vector* user_inputOutput;
 struct Vector* user_inputOnly;
 struct Vector* user_outputOnly;
};

/* Constructor and destructor */
struct VRMLLexer* newLexer();
void deleteLexer(struct VRMLLexer*);

/* Other clean up. */
void lexer_destroyData(struct VRMLLexer* me);
void lexer_destroyIdStack(Stack*);

/* Count of elements to pop off the PROTO vector for scope-out */
#define lexer_getProtoPopCnt(me) \
 (vector_size(me->userNodeTypesVec)-stack_top(size_t, me->userNodeTypesStack))

/* Set input */
#define lexer_fromString(me, str) \
 { /* printf ("lexer_fromString, new string :%s:\n",str); */ \
	 (me)->isEof=(strlen(str)<=1); FREE_IF_NZ((me)->startOfStringPtr); (me)->startOfStringPtr=str; (me)->nextIn=str;}

/* Is EOF? */
#define lexer_eof(me) \
 ((me)->isEof && !(me)->curID)

/* indexT -> char* conversion */
#define lexer_stringUFieldName(me, index, type) \
 vector_get(char*, me->user_##type, index)
#define lexer_stringUser_initializeOnly(me, index) \
 lexer_stringUFieldName(me, index, initializeOnly)
#define lexer_stringUser_inputOutput(me, index) \
 lexer_stringUFieldName(me, index, inputOutput)
#define lexer_stringUser_inputOnly(me, index) \
 lexer_stringUFieldName(me, index, inputOnly)
#define lexer_stringUser_outputOnly(me, index) \
 lexer_stringUFieldName(me, index, outputOnly)
/* User field name -> char*, takes care of access mode */
const char* lexer_stringUser_fieldName(struct VRMLLexer* me, indexT name, indexT mode);

/* Skip whitespace and comments. */
void lexer_skip(struct VRMLLexer*);

/* Ensures that curID is set. */
BOOL lexer_setCurID(struct VRMLLexer*);

/* Some operations with IDs */
void lexer_scopeIn(struct VRMLLexer*);
void lexer_scopeOut(struct VRMLLexer*);
void lexer_scopeOut_PROTO(struct VRMLLexer*);
BOOL lexer_keyword(struct VRMLLexer*, indexT);
BOOL lexer_specialID(struct VRMLLexer*, indexT* retB, indexT* retU,
 const char**, const indexT, struct Vector*);
BOOL lexer_specialID_string(struct VRMLLexer*, indexT* retB, indexT* retU,
 const char**, const indexT, struct Vector*, const char*);
BOOL lexer_defineID(struct VRMLLexer*, indexT*, struct Vector*, BOOL);
#define lexer_defineNodeName(me, ret) \
 lexer_defineID(me, ret, stack_top(struct Vector*, me->userNodeNames), TRUE)
#define lexer_defineNodeType(me, ret) \
 lexer_defineID(me, ret, me->userNodeTypesVec, FALSE)
#define lexer_define_initializeOnly(me, ret) \
 lexer_defineID(me, ret, me->user_initializeOnly, TRUE)
#define lexer_define_inputOutput(me, ret) \
 lexer_defineID(me, ret, me->user_inputOutput, TRUE)
#define lexer_define_inputOnly(me, ret) \
 lexer_defineID(me, ret, me->user_inputOnly, TRUE)
#define lexer_define_outputOnly(me, ret) \
 lexer_defineID(me, ret, me->user_outputOnly, TRUE)
BOOL lexer_initializeOnly(struct VRMLLexer*, indexT*, indexT*, indexT*, indexT*);
BOOL lexer_event(struct VRMLLexer*, struct X3D_Node*,
 indexT*, indexT*, indexT*, indexT*, int routeToFrom);
#define lexer_inputOnly(me, node, a, b, c, d) \
 lexer_event(me, node, a, b, c, d, ROUTED_FIELD_EVENT_IN)
#define lexer_outputOnly(me, node, a, b, c, d) \
 lexer_event(me, node, a, b, c, d, ROUTED_FIELD_EVENT_OUT)
#define lexer_node(me, r1, r2) \
 lexer_specialID(me, r1, r2, NODES, NODES_COUNT, me->userNodeTypesVec)
#define lexer_nodeName(me, ret) \
 lexer_specialID(me, NULL, ret, NULL, 0, \
  stack_top(struct Vector*, me->userNodeNames))
#define lexer_protoFieldMode(me, r) \
 lexer_specialID(me, r, NULL, PROTOKEYWORDS, PROTOKEYWORDS_COUNT, NULL)
#define lexer_fieldType(me, r) \
 lexer_specialID(me, r, NULL, FIELDTYPES, FIELDTYPES_COUNT, NULL)
indexT lexer_string2id(const char*, const struct Vector*);
#define lexer_nodeName2id(me, str) \
 lexer_string2id(str, stack_top(struct Vector*, me->userNodeNames))

/* Input the basic literals */
BOOL lexer_int32(struct VRMLLexer*, vrmlInt32T*);
BOOL lexer_float(struct VRMLLexer*, vrmlFloatT*);
BOOL lexer_double(struct VRMLLexer*, vrmlDoubleT*);
BOOL lexer_string(struct VRMLLexer*, vrmlStringT*);

/* Checks for the five operators of VRML */
BOOL lexer_operator(struct VRMLLexer*, char);
#define lexer_point(me) \
 lexer_operator(me, '.')
#define lexer_openCurly(me) \
 lexer_operator(me, '{')
#define lexer_closeCurly(me) \
 lexer_operator(me, '}')
#define lexer_openSquare(me) \
 lexer_operator(me, '[')
#define lexer_closeSquare(me) \
 lexer_operator(me, ']')
#define lexer_colon(me) \
 lexer_operator(me,':')

/* recursively skip to the closing curly bracket */
void skipToEndOfOpenCurly(struct VRMLLexer *me, int level);

void concatAndGiveToLexer(struct VRMLLexer *me, char *str_a, char *str_b);

/* Parser (input of non-terminal symbols) for CParse */

struct ProtoDefinition;
struct ProtoFieldDecl;
struct Shader_Script;
struct OffsetPointer;


#define BLOCK_STATEMENT(LOCATION) \
   if(parser_routeStatement(me))  { \
	continue; \
   } \
 \
  if (parser_componentStatement(me)) { \
	continue; \
  } \
 \
  if (parser_exportStatement(me)) { \
	continue; \
  } \
 \
  if (parser_importStatement(me)) { \
	continue; \
  } \
 \
  if (parser_metaStatement(me)) { \
	continue; \
  } \
 \
  if (parser_profileStatement(me)) { \
	continue; \
  } 

/* This is our parser-object. */
struct VRMLParser
{
 struct VRMLLexer* lexer;	/* The lexer used. */
 /* Where to put the parsed nodes? */
 void* ptr;
 unsigned ofs;
 /* Currently parsing a PROTO? */
 struct ProtoDefinition* curPROTO;

 /* This is the DEF/USE memory. */
 Stack* DEFedNodes;

 /* This is for PROTOs -- not stacked, as explained in CParseLexer.h */
 struct Vector* PROTOs;
};

/* Functions parsing a type by its index */
extern BOOL (*PARSE_TYPE[])(struct VRMLParser*, void*);

/* Constructor and destructor */
struct VRMLParser* newParser(void*, unsigned);
struct VRMLParser* reuseParser(void*, unsigned);
void deleteParser(struct VRMLParser*);

/* Other clean up */
void parser_destroyData(struct VRMLParser*);

/* Scoping */
void parser_scopeIn(struct VRMLParser*);
void parser_scopeOut(struct VRMLParser*);

/* Sets parser's input */
#define parser_fromString(me, str) \
 lexer_fromString(me->lexer, str)

/* Parses MF* field values */
BOOL parser_mfboolValue(struct VRMLParser*, struct Multi_Bool*);
BOOL parser_mfcolorValue(struct VRMLParser*, struct Multi_Color*);
BOOL parser_mfcolorrgbaValue(struct VRMLParser*, struct Multi_ColorRGBA*);
BOOL parser_mffloatValue(struct VRMLParser*, struct Multi_Float*);
BOOL parser_mfint32Value(struct VRMLParser*, struct Multi_Int32*);
BOOL parser_mfnodeValue(struct VRMLParser*, struct Multi_Node*);
BOOL parser_mfrotationValue(struct VRMLParser*, struct Multi_Rotation*);
BOOL parser_mfstringValue(struct VRMLParser*, struct Multi_String*);
BOOL parser_mftimeValue(struct VRMLParser*, struct Multi_Time*);
BOOL parser_mfvec2fValue(struct VRMLParser*, struct Multi_Vec2f*);
BOOL parser_mfvec3fValue(struct VRMLParser*, struct Multi_Vec3f*);
BOOL parser_mfvec3dValue(struct VRMLParser*, struct Multi_Vec3d*);

/* Parses SF* field values */
BOOL parser_sfboolValue(struct VRMLParser*, vrmlBoolT*);
BOOL parser_sfcolorValue(struct VRMLParser*, vrmlColorT*);
BOOL parser_sfcolorrgbaValue(struct VRMLParser*, vrmlColorRGBAT*);
BOOL parser_sffloatValue_(struct VRMLParser*, vrmlFloatT*);
#define parser_sffloatValue(me, ret) \
 lexer_float(me->lexer, ret)
BOOL parser_sfimageValue(struct VRMLParser*, vrmlImageT*);
BOOL parser_sfint32Value_(struct VRMLParser*, vrmlInt32T*);
#define parser_sfint32Value(me, ret) \
 lexer_int32(me->lexer, ret)
BOOL parser_sfnodeValue(struct VRMLParser*, vrmlNodeT*);
BOOL parser_sfrotationValue(struct VRMLParser*, vrmlRotationT*);
BOOL parser_sfstringValue_(struct VRMLParser*, vrmlStringT*);
#define parser_sfstringValue(me, ret) \
 lexer_string(me->lexer, ret)
BOOL parser_sftimeValue(struct VRMLParser*, vrmlTimeT*);
BOOL parser_sfvec2fValue(struct VRMLParser*, vrmlVec2fT*);
BOOL parser_sfvec2dValue(struct VRMLParser*, vrmlVec2dT*);
BOOL parser_sfvec3dValue(struct VRMLParser*, vrmlVec3dT*);
#define parser_sfvec3fValue(me, ret) \
 parser_sfcolorValue(me, ret)
BOOL parser_sfvec4fValue(struct VRMLParser*, vrmlVec4fT*);
BOOL parser_sfvec4dValue(struct VRMLParser*, vrmlVec4dT*);
BOOL parser_sfmatrix3fValue(struct VRMLParser *, vrmlMatrix3fT*);
BOOL parser_sfmatrix3dValue(struct VRMLParser *, vrmlMatrix3dT*);
BOOL parser_sfmatrix4fValue(struct VRMLParser *, vrmlMatrix4fT*);
BOOL parser_sfmatrix4dValue(struct VRMLParser *, vrmlMatrix4dT*);


/* Parses nodes, fields and other statements. */
BOOL parser_routeStatement(struct VRMLParser*);
BOOL parser_componentStatement(struct VRMLParser*);
BOOL parser_exportStatement(struct VRMLParser*);
BOOL parser_importStatement(struct VRMLParser*);
BOOL parser_metaStatement(struct VRMLParser*);
BOOL parser_profileStatement(struct VRMLParser*);

BOOL parser_protoStatement(struct VRMLParser*);
BOOL parser_interfaceDeclaration(struct VRMLParser*,
 struct ProtoDefinition*, struct Shader_Script*);
BOOL parser_nodeStatement(struct VRMLParser*, vrmlNodeT*);
BOOL parser_node(struct VRMLParser*, vrmlNodeT*, indexT);
BOOL parser_field(struct VRMLParser*, struct X3D_Node*);
BOOL parser_fieldEvent(struct VRMLParser*, struct X3D_Node*);
BOOL parser_fieldEventAfterISPart(struct VRMLParser*, struct X3D_Node*,
 BOOL isIn, BOOL isOut, indexT, indexT);
BOOL parser_protoField(struct VRMLParser*, struct ProtoDefinition*, struct ProtoDefinition*);
BOOL parser_protoEvent(struct VRMLParser*, struct ProtoDefinition*, struct ProtoDefinition*);

/* Initializes node-specific fields */
void parser_specificInitNode(struct X3D_Node*, struct VRMLParser*);

/* Registers a ROUTE, in current PROTO or scene */
void parser_registerRoute(struct VRMLParser*,
 struct X3D_Node*, unsigned, struct X3D_Node*, unsigned, size_t, int);

/* Parses a field value of a certain type (literally or IS) */
BOOL parser_fieldValue(struct VRMLParser*, struct OffsetPointer*, indexT, indexT, BOOL, struct ProtoDefinition*, struct ProtoFieldDecl*);

/* Main parsing routine, parses the start symbol (vrmlScene) */
BOOL parser_vrmlScene(struct VRMLParser*);

BOOL parseType(struct VRMLParser* me, indexT type,   union anyVrml *defaultVal);


void replaceProtoField(struct VRMLLexer *me, struct ProtoDefinition *thisProto, char *thisID, char **outTextPtr, int *outSize);
/*
void getEquivPointer(struct OffsetPointer* origPointer, struct OffsetPointer* ret, struct X3D_Node* origProtoNode, struct X3D_Node* curProtoNode);
*/

/* CProto.h - this is the object representing a PROTO definition and being
 * capable of instantiating it.
 * 
 * We keep a vector of pointers to all that pointers which point to "inner
 * memory" and need therefore be updated when copying.  Such pointers include
 * field-destinations and parts of ROUTEs.  Those pointers are then simply
 * copied, their new positions put in the new vector, and afterwards are all
 * pointers there updated.
 */

struct PointerHash;
struct VRMLParser;

/* ************************************************************************** */
/* ******************************** OffsetPointer *************************** */
/* ************************************************************************** */

/* A pointer which is made up of the offset/node pair */
struct OffsetPointer
{
 struct X3D_Node* node;
 unsigned ofs;
};

/* Constructor/destructor */
struct OffsetPointer* newOffsetPointer(struct X3D_Node*, unsigned);
#define offsetPointer_copy(me) \
 newOffsetPointer((me)->node, (me)->ofs)
#define deleteOffsetPointer(me) \
 FREE_IF_NZ(me)

/* Dereference to simple pointer */
#define offsetPointer_deref(t, me) \
 ((t)(((char*)((me)->node))+(me)->ofs))

/* ************************************************************************** */
/* ************************** ProtoElementPointer *************************** */
/* ************************************************************************** */

/* A pointer which is made up of the offset/node pair */
struct ProtoElementPointer
{
	char *stringToken; 	/* pointer to a name, etc. NULL if one of the index_t fields is in use */
	indexT isNODE;		/* NODES index, if found, ID_UNDEFINED  otherwise */
	indexT isKEYWORD;	/* KEYWORDS index, if found, ID_UNDEFINED otherwise */ 
	indexT terminalSymbol;	/* ASCII value of ".", "{", "}", "[", "]", ":", ID_UNDEFINED otherwise */
	indexT fabricatedDef;	/* for making a unique DEF name */
};

/* Constructor/destructor */
struct ProtoElementPointer* newProtoElementPointer(void);

#define deleteProtoElementPointer(me) \
 {FREE_IF_NZ(me->stringToken); FREE_IF_NZ(me);}

struct ProtoElementPointer *copyProtoElementPointer(struct ProtoElementPointer *);

#define ASSIGN_UNIQUE_ID(me) \
	{me->fabricatedDef = nextFabricatedDef; nextFabricatedDef ++; }

#define FABRICATED_DEF_HEADER "fReEwEL_fAbricatio_dEF_" /* hopefully quite unique! */

/* ************************************************************************** */
/* ********************************* ProtoFieldDecl ************************* */
/* ************************************************************************** */

/* The object */
struct ProtoFieldDecl
{
 indexT mode; /* field, exposedField, eventIn, eventOut */
 indexT type; /* field type */
 indexT name; /* field "name" (its lexer-index) */
 char *fieldString; /* the field, in ascii form */
# ifdef OLDDEST
/* This is the list of desination pointers for this field */
 struct Vector* dests; 
# endif

 /* Only for exposedField or field */
 BOOL alreadySet; /* Has the value already been set? */
 union anyVrml defaultVal; /* Default value */
 /* Script fields */
 struct Vector* scriptDests;
};

/* Constructor and destructor */
struct ProtoFieldDecl* newProtoFieldDecl(indexT, indexT, indexT);
void deleteProtoFieldDecl(struct ProtoFieldDecl*);

/* Copies */
struct ProtoFieldDecl* protoFieldDecl_copy(struct VRMLLexer*, struct ProtoFieldDecl*);

/* Accessors */
#define protoFieldDecl_getType(me) \
 ((me)->type)
#define protoFieldDecl_getAccessType(me) \
 ((me)->mode)
#define protoFieldDecl_getIndexName(me) \
 ((me)->name)
#define protoFieldDecl_getStringName(lex, me) \
 lexer_stringUser_fieldName(lex, protoFieldDecl_getIndexName(me), \
  protoFieldDecl_getAccessType(me))
 
#ifdef OLDDEST
# define protoFieldDecl_getDestinationCount(me) \
 vector_size((me)->dests)
# define protoFieldDecl_getDestination(me, i) \
 vector_get(struct OffsetPointer*, (me)->dests, i)
#endif


#define protoFieldDecl_getDefaultValue(me) \
 ((me)->defaultVal)


/* Add a destination this field's value must be assigned to */
#ifdef OLDDEST
# define protoFieldDecl_addDestinationOptr(me, optr) \
 vector_pushBack(struct OffsetPointer*, me->dests, optr)
# define protoFieldDecl_addDestination(me, n, o) \
 protoFieldDecl_addDestinationOptr(me, newOffsetPointer(n, o))
#endif


/* Sets this field's value (copy to destinations) */
void protoFieldDecl_setValue(struct VRMLLexer*, struct ProtoFieldDecl*, union anyVrml*);

/* Build a ROUTE from/to this field */
void protoFieldDecl_routeTo(struct ProtoFieldDecl*,
 struct X3D_Node*, unsigned, int dir, struct VRMLParser*);
void protoFieldDecl_routeFrom(struct ProtoFieldDecl*,
 struct X3D_Node*, unsigned, int dir, struct VRMLParser*);

/* Finish this field - if value is not yet set, use default. */
#define protoFieldDecl_finish(lex, me) \
 if(((me)->mode==PKW_initializeOnly || (me)->mode==PKW_inputOutput) && \
  !(me)->alreadySet) \
  protoFieldDecl_setValue(lex, me, &(me)->defaultVal)

/* Add inner pointers' pointers to the vector */
void protoFieldDecl_addInnerPointersPointers(struct ProtoFieldDecl*,
 struct Vector*);

/* ************************************************************************** */
/* ******************************* ProtoRoute ******************************* */
/* ************************************************************************** */

/* A ROUTE defined inside a PROTO block. */
struct ProtoRoute
{
 struct X3D_Node* from;
 struct X3D_Node* to;
 int fromOfs;
 int toOfs;
 size_t len;
 int dir;
};

/* Constructor and destructor */
struct ProtoRoute* newProtoRoute(struct X3D_Node*, int, struct X3D_Node*, int,
 size_t, int);
#define protoRoute_copy(me) \
 newProtoRoute((me)->from, (me)->fromOfs, (me)->to, (me)->toOfs, \
 (me)->len, (me)->dir)
#define deleteProtoRoute(me) \
 FREE_IF_NZ(me)

/* Register this route */
#define protoRoute_register(me) \
 CRoutes_RegisterSimple((me)->from, (me)->fromOfs, (me)->to, (me)->toOfs, \
 (me)->len, (me)->dir)

/* Add this one's inner pointers to the vector */
#define protoRoute_addInnerPointersPointers(me, vec) \
 { \
  vector_pushBack(void**, vec, &(me)->from); \
  vector_pushBack(void**, vec, &(me)->to); \
 }

/* ************************************************************************** */
/* ****************************** ProtoDefinition *************************** */
/* ************************************************************************** */

/* The object */
struct ProtoDefinition
{
 indexT protoDefNumber;	/* unique sequence number */
 struct Vector* iface; /* The ProtoFieldDecls making up the interface */
 struct Vector* deconstructedProtoBody; /* PROTO body tokenized */
 int estimatedBodyLen; /* an estimate of the expanded proto body size, to give us an output string len */
};

/* Constructor and destructor */
struct ProtoDefinition* newProtoDefinition();
void deleteProtoDefinition(struct ProtoDefinition*);

/* Adds a field declaration to the interface */
#define protoDefinition_addIfaceField(me, field) \
 vector_pushBack(struct ProtoFieldDecl*, (me)->iface, field)

/* Get fields by indices */
#define protoDefinition_getFieldCount(me) \
 vector_size((me)->iface)
#define protoDefinition_getFieldByNum(me, i) \
 vector_get(struct ProtoFieldDecl*, (me)->iface, i)

/* Retrieves a field declaration of this PROTO */
struct ProtoFieldDecl* protoDefinition_getField(struct ProtoDefinition*, 
 indexT, indexT);

/* Copies a ProtoDefinition, so that we can afterwards fill in field values */
struct ProtoDefinition* protoDefinition_copy(struct VRMLLexer*, struct ProtoDefinition*);

/* Extracts the scene graph out of a ProtoDefinition */
struct X3D_Group* protoDefinition_extractScene(struct VRMLLexer* lex, struct ProtoDefinition*);

/* Does a recursively deep copy of a node-tree */
struct X3D_Node* protoDefinition_deepCopy(struct VRMLLexer*, struct X3D_Node*,
 struct ProtoDefinition*, struct PointerHash*);

/* ************************************************************************** */
/* ******************************* PointerHash ****************************** */
/* ************************************************************************** */

/* A hash table used to check whether a specific pointer has already been
 * copied.  Otherwise we can't keep things like multiple references to the same
 * node when copying. */

/* An entry */
struct PointerHashEntry
{
 struct X3D_Node* original;
 struct X3D_Node* copy;
};

/* The object */
struct PointerHash
{
 #define POINTER_HASH_SIZE	4321
 struct Vector* data[POINTER_HASH_SIZE];
};

struct PointerHash* newPointerHash();
void deletePointerHash(struct PointerHash*);

/* Query the hash */
struct X3D_Node* pointerHash_get(struct PointerHash*, struct X3D_Node*);

/* Add to the hash */
void pointerHash_add(struct PointerHash*, struct X3D_Node*, struct X3D_Node*);

/* JAS - make a copy of a script in a PROTO, and give it a new number */
void registerScriptInPROTO (struct X3D_Script *scr,struct ProtoDefinition* new);

/* This structure holds information about a nested proto reference. That is to say, when a we have an instantiation
   of a proto within another proto definition, and two user defined fields are linked to each other in an IS statement.
   PROTO Proto1 [
	field SFFloat myvalue 0.1
  ] {
	DEF MAT Material { shininess IS myvalue }	
  }
  PROTO Proto2 [
	field SFFloat secondvalue 0.5
  ] { 
	DEF MAT2 Proto1 { myvalue IS secondvalue } 
  {

  In this case, we need the dests for myvalue must be replicated for secondvalue, but the references to nodes must be for the nodes in the proto expansion of Proto1, 
  not references to nodes in Proto1 itself.  We accomplish this by copying the dests list for the proto field after the proto has been expanded.

  The NestedProtoFields structure is used when parsing a nested proto expansion in order to keep track of all instances of linked user defined fields so that the dests
  lists may be adjusted appropriately when parsing is finalised for the node. */
struct NestedProtoField 
{
   struct ProtoFieldDecl* origField;
   struct ProtoFieldDecl* localField;
};

void getEquivPointer(struct OffsetPointer* origPointer, struct OffsetPointer* ret, struct X3D_Node* origProtoNode, struct X3D_Node* curProtoNode);
void getProtoInvocationFields(struct VRMLParser *me, struct ProtoDefinition *thisProto);
struct ProtoFieldDecl* getProtoFieldDeclaration(struct VRMLLexer *me, struct ProtoDefinition *thisProto, char *thisID);
void tokenizeProtoBody(struct ProtoDefinition *, char *);
char *protoExpand (struct VRMLParser *me, indexT nodeTypeU, struct ProtoDefinition **thisProto);
BOOL resolveProtoNodeField(struct VRMLParser *me, struct ProtoDefinition *Proto, struct X3D_Node **Node);

/* Class to wrap a java script for CParser */

/* ************************************************************************** */
/* ************************ Methods used by X3D Parser  ********************* */
/* ************************************************************************** */
uintptr_t nextScriptHandle (void);
void zeroScriptHandles (void);
struct X3D_Script * protoScript_copy (struct X3D_Script *me);


/* ************************************************************************** */
/* ****************************** ScriptFieldDecl *************************** */
/* ************************************************************************** */

/* Struct */
/* ****** */

struct ScriptFieldDecl
{
 /* subclass of FieldDecl */
 struct FieldDecl* fieldDecl;

 /* Stringified */
 const char* name;
 const char* type;
 const char* ISname;

 /* For fields */
 union anyVrml value;
 BOOL valueSet;	/* Has the value been set? */
};

/* Structure that holds information regarding script fields that are targets in PROTO IS statements */
struct ScriptFieldInstanceInfo {
	struct ScriptFieldDecl* decl;
	struct Shader_Script* script;
};

/* Constructor and destructor */
/* ************************** */

struct ScriptFieldDecl* newScriptFieldDecl(struct VRMLLexer*, indexT, indexT, indexT);
struct ScriptFieldInstanceInfo* newScriptFieldInstanceInfo(struct ScriptFieldDecl*, struct Shader_Script*);
struct ScriptFieldDecl* scriptFieldDecl_copy(struct VRMLLexer*, struct ScriptFieldDecl*);
void deleteScriptFieldDecl(struct ScriptFieldDecl*);

/* Other members */
/* ************* */

/* Get "offset" data for routing */
int scriptFieldDecl_getRoutingOffset(struct ScriptFieldDecl*);

/* Set field value */
void scriptFieldDecl_setFieldValue(struct ScriptFieldDecl*, union anyVrml);

/* Do JS-init for given script handle */
void scriptFieldDecl_jsFieldInit(struct ScriptFieldDecl*, uintptr_t);

/* Forwards to inherited methods */
#define scriptFieldDecl_isField(me, nam, mod) \
 fieldDecl_isField((me)->fieldDecl, nam, mod)

/* ************************************************************************** */
/* ********************************** Script ******************************** */
/* ************************************************************************** */

/* Struct */
/* ****** */

struct Shader_Script
{
 struct X3D_Node *ShaderScriptNode; /* NODE_Script, NODE_ComposedShader, etc */
 uintptr_t num;	/* The script handle  if a script, -1 if a shader */
 BOOL loaded;	/* Has the code been loaded into this script? */
 struct Vector* fields;
};

/* Constructor and destructor */
/* ************************** */

struct Shader_Script* new_Shader_Script(struct X3D_Node *);
void deleteScript();

/* Other members */
/* ************* */

/* Initializes the script with its code */
BOOL script_initCode(struct Shader_Script*, const char*);
BOOL script_initCodeFromUri(struct Shader_Script*, const char*);
BOOL script_initCodeFromMFUri(struct Shader_Script*, const struct Multi_String*);

/* Add a new field */
void script_addField(struct Shader_Script*, struct ScriptFieldDecl*);

/* Get a field by name */
struct ScriptFieldDecl* script_getField(struct Shader_Script*, indexT ind, indexT mod);


void InitScriptField(int num, indexT kind, indexT type, char* field, union anyVrml value);
void SaveScriptField (int num, indexT kind, indexT type, char* field, union anyVrml value);
struct ScriptParamList {
        struct ScriptParamList *next;
        indexT kind;
        indexT type;
        char *field;
        union anyVrml value;
};

struct CRscriptStruct {
	/* type */
	int thisScriptType;

	/* Javascript parameters */
	int _initialized;			/* this script initialized yet? */
	uintptr_t	cx;			/* JSContext		*/
	uintptr_t	glob;			/* JSGlobals		*/
	uintptr_t	eventsProcessed; 	/* eventsProcessed() compiled function parameter*/
	char *scriptText;
	struct ScriptParamList *paramList;
};

/* headers for EAI and java CLASS invocation */

/* function prototypes */
void handle_Listener (void);
void EAI_Convert_mem_to_ASCII (int id, char *reptype, int type, char *memptr, char *buf);
unsigned int EAI_SendEvent (char *ptr);
void EAI_RNewW(char *bufptr);
void EAI_RW(char *bufptr);


#define MAXEAIHOSTNAME	255		/* length of hostname on command line */
#define EAIREADSIZE	8192		/* maximum we are allowed to read in from socket */
#define EAIBASESOCKET   9877		/* socket number to start at */


/* these are commands accepted from the EAI client */
#define GETNODE		'A'
#define UPDATEROUTING 	'B'
#define SENDCHILD 	'C'
#define SENDEVENT	'D'
#define GETVALUE	'E'
#define GETFIELDTYPE	'F'
#define	REGLISTENER	'G'
#define	ADDROUTE	'H'
#define REREADWRL	'I'
#define	DELETEROUTE	'J'
#define GETNAME		'K'
#define	GETVERSION	'L'
#define GETCURSPEED	'M'
#define GETFRAMERATE	'N'
#define	GETURL		'O'
#define	REPLACEWORLD	'P'
#define	LOADURL		'Q'
#define VIEWPOINT	'R'
#define CREATEVS	'S'
#define	CREATEVU	'T'
#define	STOPFREEWRL	'U'
#define UNREGLISTENER   'W'
#define GETRENDPROP	'X'
#define GETENCODING	'Y'
#define CREATENODE	'a'
#define CREATEPROTO	'b'
#define UPDNAMEDNODE	'c'
#define REMNAMEDNODE	'd'
#define GETPROTODECL 	'e'
#define UPDPROTODECL	'f'
#define REMPROTODECL	'g'
#define GETFIELDDEFS	'h'
#define GETNODEDEFNAME	'i'
#define GETROUTES	'j'
#define GETNODETYPE	'k'
#define MIDIINFO  	'l'
#define MIDICONTROL  	'm'


/* command string to get the rootNode - this is a special match... */
#define SYSTEMROOTNODE "_Sarah_this_is_the_FreeWRL_System_Root_Node"


/* Subtypes - types of data to get from EAI  - we don't use the ones defined in
   headers.h, because we want ASCII characters */

#define	EAI_SFBool		'b'
#define	EAI_SFColor		'c'
#define	EAI_SFFloat		'd'
#define	EAI_SFTime		'e'
#define	EAI_SFInt32		'f'
#define	EAI_SFString		'g'
#define	EAI_SFNode		'h'
#define	EAI_SFRotation		'i'
#define	EAI_SFVec2f		'j'
#define	EAI_SFImage		'k'
#define	EAI_MFColor		'l'
#define	EAI_MFFloat		'm'
#define	EAI_MFTime		'n'
#define	EAI_MFInt32		'o'
#define	EAI_MFString		'p'
#define	EAI_MFNode		'q'
#define	EAI_MFRotation		'r'
#define	EAI_MFVec2f		's'
#define EAI_MFVec3f		't'
#define EAI_SFVec3f		'u'
#define EAI_MFColorRGBA		'v'
#define EAI_SFColorRGBA		'w'
#define EAI_MFBool		'x'
#define EAI_FreeWRLPTR		'y'
#define EAI_MFVec3d		'A'
#define EAI_SFVec2d		'B'
#define EAI_SFVec3d		'C'
#define EAI_MFVec2d		'D'
#define EAI_SFVec4d		'E'
#define EAI_MFDouble		'F'
#define EAI_SFDouble		'G'
#define EAI_SFMatrix3f		'H'
#define EAI_MFMatrix3f		'I'
#define EAI_SFMatrix3d		'J'
#define EAI_MFMatrix3d		'K'
#define EAI_SFMatrix4f		'L'
#define EAI_MFMatrix4f		'M'
#define EAI_SFMatrix4d		'N'
#define EAI_MFMatrix4d		'O'
#define EAI_SFVec4f		'P'
#define EAI_MFVec4f		'Q'
#define EAI_MFVec4d		'R'



/* Function Prototype for plugins, Java Class Invocation */
int createUDPSocket();
int conEAIorCLASS(int socketincrement, int *sockfd, int *listenfd);
void EAI_send_string (char *str, int listenfd);
char *read_EAI_socket(char *bf, int *bfct, int *bfsz, int *listenfd);
extern int EAIlistenfd;
extern int EAIsockfd;
extern int EAIport;
extern int EAIwanted;
extern int EAIbufsize;
extern char *EAIbuffer;
extern int EAIbufcount;
extern char EAIListenerData[EAIREADSIZE];
extern char EAIListenerArea[40];

#define MIDI_CONTROLLER_UNUSED 4
#define MIDI_CONTROLLER_FADER 1
#define MIDI_CONTROLLER_KEYPRESS 2
#define MIDI_CONTROLLER_UNKNOWN 999
/*******************************************************************
 Copyright (C) 2003 John Stewart, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*********************************************************************/

/* see if an inputOnly "set_" field has changed */
#define IO_FLOAT -2335549.0

void *freewrlMalloc(int line, char *file, size_t sz);
void *freewrlRealloc (int line, char *file, void *ptr, size_t size);
void *freewrlStrdup (int line, char *file, char *str);
#define MALLOC(sz) FWMALLOC (__LINE__,__FILE__,sz)
#define FWMALLOC(l,f,sz) freewrlMalloc(l,f,sz)
#define REALLOC(a,b) freewrlRealloc(__LINE__,__FILE__,a,b)
#define STRDUP(a) freewrlStrdup(__LINE__,__FILE__,a)
#ifdef DEBUG_MALLOC
	/* free a malloc'd pointer */
	void freewrlFree(int line, char *file, void *a);
	#define FREE_IF_NZ(a) if(a) {freewrlFree(__LINE__,__FILE__,a); a = 0;}

#else 
	#define FREE_IF_NZ(a) if(a) {free(a); a = 0;}
#endif

#define UNLINK(fdd) {printf ("unlinking %s at %s:%d\n",fdd,__FILE__,__LINE__); unlink (fdd); }

/* children fields path optimizations */
#define CHILDREN_COUNT int nc = node->children.n;
#define RETURN_FROM_CHILD_IF_NOT_FOR_ME \
        /* any children at all? */ \
        if (nc==0) return;      \
        /* should we go down here? */ \
        /* printf ("Group, rb %x VF_B %x, rg  %x VF_G %x\n",render_blend, VF_Blend, render_geom, VF_Geom); */ \
        if (render_blend == VF_Blend) \
                if ((node->_renderFlags & VF_Blend) != VF_Blend) { \
                        return; \
                } \
        if (render_proximity == VF_Proximity) \
                if ((node->_renderFlags & VF_Proximity) != VF_Proximity)  { \
                        return; \
                } \




#undef DEBUG_JAVASCRIPT_PROPERTY
#ifdef DEBUG_JAVASCRIPT_PROPERTY
#define JS_GET_PROPERTY_STUB js_GetPropertyDebug
#define JS_SET_PROPERTY_STUB1 js_SetPropertyDebug1
#define JS_SET_PROPERTY_STUB2 js_SetPropertyDebug2 
#define JS_SET_PROPERTY_STUB3 js_SetPropertyDebug3 
#define JS_SET_PROPERTY_STUB4 js_SetPropertyDebug4 
#define JS_SET_PROPERTY_STUB5 js_SetPropertyDebug5 
#define JS_SET_PROPERTY_STUB6 js_SetPropertyDebug6 
#define JS_SET_PROPERTY_STUB7 js_SetPropertyDebug7 
#else
#define JS_GET_PROPERTY_STUB JS_PropertyStub
#define JS_SET_PROPERTY_STUB1 JS_PropertyStub
#define JS_SET_PROPERTY_STUB2 JS_PropertyStub
#define JS_SET_PROPERTY_STUB3 JS_PropertyStub
#define JS_SET_PROPERTY_STUB4 JS_PropertyStub
#define JS_SET_PROPERTY_STUB5 JS_PropertyStub
#define JS_SET_PROPERTY_STUB6 JS_PropertyStub
#define JS_SET_PROPERTY_STUB7 JS_PropertyStub
#endif

/* stop the display thread. Used (when this comment was made) by the OSX Safari plugin; keeps
most things around, just stops display thread, when the user exits a world. */
#define STOP_DISPLAY_THREAD \
        if (DispThrd != NULL) { \
                quitThread = TRUE; \
                pthread_join(DispThrd,NULL); \
                DispThrd = NULL; \
        }


typedef struct _CRnodeStruct {
        struct X3D_Node *routeToNode;
        unsigned int foffset;
} CRnodeStruct;

/* Size of static array */
#define ARR_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

/* Some stuff for routing */
#define FROM_SCRIPT 1
#define TO_SCRIPT 2
#define SCRIPT_TO_SCRIPT 3

/* Helper to get size of a struct's memer */
#define sizeof_member(str, var) \
 sizeof(((str*)NULL)->var)

/* C routes */
#define MAXJSVARIABLELENGTH 25	/* variable name length can be this long... */

struct CRStruct {
        struct X3D_Node*  routeFromNode;
        uintptr_t fnptr;
        unsigned int tonode_count;
        CRnodeStruct *tonodes;
        int     isActive;
        int     len;
        void    (*interpptr)(void *); /* pointer to an interpolator to run */
        int     direction_flag; /* if non-zero indicates script in/out,
                                                   proto in/out */
        int     extra;          /* used to pass a parameter (eg, 1 = addChildren..) */
};
struct CRjsnameStruct {
        int     	type;
        char    	name[MAXJSVARIABLELENGTH];
	uintptr_t 	eventInFunction;		/* compiled javascript function... if it is required */
};


extern struct CRjsnameStruct *JSparamnames;
extern struct CRStruct *CRoutes;
extern int jsnameindex;
extern int MAXJSparamNames;

extern char *BrowserName;
extern char *BrowserFullPath;

/* multi-threaded OpenGL contexts - works on OS X, kind of ok on Linux, but
   blows plugins out of the water, because of the XLib threaded call in FrontEnd
   not working that well... */
#ifdef AQUA
	#define DO_MULTI_OPENGL_THREADS
#endif

/* display the BoundingBoxen */
#undef  DISPLAYBOUNDINGBOX

/* rendering constants used in SceneGraph, etc. */
#define VF_Viewpoint 				0x0001
#define VF_Geom 				0x0002
#define VF_DirectionalLight			0x0004 
#define VF_Sensitive 				0x0008
#define VF_Blend 				0x0010
#define VF_Proximity 				0x0020
#define VF_Collision 				0x0040
#define VF_hasVisibleChildren 			0x0100
#define VF_otherLight				0x0800 

/* for z depth buffer calculations */
#define DEFAULT_NEARPLANE 0.1
#define xxDEFAULT_FARPLANE 21000.0
#define DEFAULT_FARPLANE 210.0
extern double geoHeightinZAxis;


/* compile simple nodes (eg, Cone, LineSet) into an internal format. Check out
   CompileC in VRMLRend.pm, and look for compile_* functions in code. General
   meshes are rendered using the PolyRep scheme, which also compiles into OpenGL 
   calls, using the PolyRep (and, stream_PolyRep) methodology */

int isShapeCompilerParsing(void);
void compile_polyrep(void *node, void *coord, void *color, void *normal, void *texCoord);
#define COMPILE_POLY_IF_REQUIRED(a,b,c,d) \
                if(!node->_intern || node->_change != ((struct X3D_PolyRep *)node->_intern)->irep_change) { \
                        compileNode ((void *)compile_polyrep, node, a,b,c,d); \
		} \
		if (!node->_intern) return;

#define COMPILE_IF_REQUIRED { struct X3D_Virt *v; \
	if (node->_ichange != node->_change) { \
		/* printf ("COMP %d %d\n",node->_ichange, node->_change); */ \
		v = *(struct X3D_Virt **)node; \
		if (v->compile) { \
			compileNode (v->compile, (void *)node, NULL, NULL, NULL, NULL); \
		} else {printf ("huh - have COMPIFREQD, but v->compile null for %s\n",stringNodeType(node->_nodeType));} \
		} \
		if (node->_ichange == 0) return; \
	}

/* same as COMPILE_IF_REQUIRED, but passes in node name */
#define COMPILE_IF_REQUIRED2(node) { struct X3D_Virt *v; \
	if (node->_ichange != node->_change) { \
	/* printf ("COMP %d %d\n",node->_ichange, node->_change); */ \
		v = *(struct X3D_Virt **)node; \
		if (v->compile) { \
			compileNode (v->compile, (void *)node, NULL, NULL, NULL, NULL); \
		} else {printf ("huh - have COMPIFREQD, but v->compile null for %s\n",stringNodeType(node->_nodeType));} \
		} \
		if (node->_ichange == 0) return; \
	}


/* convert a PROTO node (which will be a Group node) into a node. eg, for Materials  - this is a possible child
node for ANY node that takes something other than a Group */
#define POSSIBLE_PROTO_EXPANSION(inNode,outNode) \
	if (inNode == NULL) outNode = NULL; \
	else {if (X3D_NODE(inNode)->_nodeType == NODE_Group) { \
		if (X3D_GROUP(inNode)->children.n>0) { \
			outNode = X3D_GROUP(inNode)->children.p[0]; \
		} else outNode = NULL; \
	} else outNode = inNode; };


#define RENDER_MATERIAL_SUBNODES(which) \
	{ void *tmpN;   \
		POSSIBLE_PROTO_EXPANSION(which,tmpN) \
       		if(tmpN) { \
			render_node(tmpN); \
       		} else { \
			/* no material, so just colour the following shape */ \
	       		/* Spec says to disable lighting and set coloUr to 1,1,1 */ \
	       		LIGHTING_OFF  \
       			glColor3f(1,1,1); \
 \
			/* tell the rendering passes that this is just "normal" */ \
			last_texture_type = NOTEXTURE; \
			/* same with global_transparency */ \
			global_transparency=0.99999; \
		} \
	}

#define MARK_NODE_COMPILED node->_ichange = node->_change;
#define NODE_NEEDS_COMPILING (node->_ichange != node->_change)
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
if (!APPROX (node->setField.r[0],node->regField.r[0]) || \
        !APPROX(node->setField.r[1],node->regField.r[1]) || \
        !APPROX(node->setField.r[2],node->regField.r[2]) || \
        !APPROX(node->setField.r[3],node->regField.r[3]) ) { \
        /* now, is the setField at our default value??  if not, we just use the regField */ \
        if (APPROX(node->setField.r[0], IO_FLOAT) && APPROX(node->setField.r[1],IO_FLOAT) && APPROX(node->setField.r[2],IO_FLOAT) && APPROX(node->setField.r[3],IO_FLOAT)) { \
		/* printf ("just use SFRotation regField\n"); */ \
        } else { \
		/* printf ("use the setField SFRotation as the real poistion field\n");  */ \
        	memcpy (node->regField.r, node->setField.r, sizeof (struct SFRotation)); \
	} \
}




int find_key (int kin, float frac, float *keys);
void startOfLoopNodeUpdates(void);
void OcclusionCulling (void);
void OcclusionStartofEventLoop(void);
extern int HaveSensitive;
void zeroVisibilityFlag(void);
void setField_fromJavascript (struct X3D_Node *ptr, char *field, char *value);
unsigned int setField_FromEAI (char *ptr);
void setField_javascriptEventOut(struct X3D_Node  *tn,unsigned int tptr, int fieldType, unsigned len, int extraData, uintptr_t mycx);

extern char *GL_VEN;
extern char *GL_VER;
extern char *GL_REN;

/* do we have GL Occlusion Culling? */
#ifdef AQUA
	#define OCCLUSION
        #define VISIBILITYOCCLUSION
        #define SHAPEOCCLUSION
        #define glGenQueries(a,b) glGenQueriesARB(a,b)
        #define glDeleteQueries(a,b) glDeleteQueriesARB(a,b)
#else
	/* on Linux, test to see if we have this defined. */
	#ifdef HAVE_GL_QUERIES_ARB
		#define OCCLUSION
		#define VISIBILITYOCCLUSION
		#define SHAPEOCCLUSION
		#define glGenQueries(a,b) glGenQueriesARB(a,b)
		#define glDeleteQueries(a,b) glDeleteQueriesARB(a,b)
	#else 
		#undef OCCLUSION
		#undef VISIBILITYOCCLUSION
		#undef SHAPEOCCLUSION
		#define glGenQueries(a,b)
		#define glDeleteQueries(a,b)
	#endif
#endif

#define glIsQuery(a) glIsQueryARB(a)
#define glBeginQuery(a,b) glBeginQueryARB(a,b)
#define glEndQuery(a) glEndQueryARB(a)
#define glGetQueryiv(a,b,c) glGetQueryivARB(a,b,c)
#define glGetQueryObjectiv(a,b,c) glGetQueryObjectivARB(a,b,c)
#define glGetQueryObjectuiv(a,b,c) glGetQueryObjectuivARB(a,b,c)

extern GLuint OccQuerySize;
extern GLint OccResultsAvailable;
extern int OccFailed;
extern int *OccCheckCount;
extern GLuint *OccQueries;
extern void * *OccNodes;
int newOcclude(void);
void zeroOcclusion(void);
extern int QueryCount;
extern GLuint potentialOccluderCount;
extern void* *occluderNodePointer;

#ifdef OCCLUSION
#define OCCLUSIONTEST \
	/* a value of ZERO means that it HAS visible children - helps with initialization */ \
        if ((render_geom!=0) | (render_sensitive!=0)) { \
		/* printf ("OCCLUSIONTEST node %d fl %x\n",node, node->_renderFlags & VF_hasVisibleChildren); */ \
                if ((node->_renderFlags & VF_hasVisibleChildren) == 0) { \
                        /* printf ("WOW - we do NOT need to do this transform but doing it %x!\n",(node->_renderFlags)); \
 printf (" vp %d geom %d light %d sens %d blend %d prox %d col %d\n", \
         render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */ \
                        return; \
                } \
        } 
#else
#define OCCLUSIONTEST
#endif



#define BEGINOCCLUSIONQUERY \
	if (render_geom) { \
		if (potentialOccluderCount < OccQuerySize) { \
			if (node->__occludeCheckCount < 0) { \
				/* printf ("beginOcclusionQuery, query %u, node %s\n",potentialOccluderCount, stringNodeType(node->_nodeType)); */ \
				glBeginQuery(GL_SAMPLES_PASSED, OccQueries[potentialOccluderCount]); \
				occluderNodePointer[potentialOccluderCount] = (void *)node; \
			} \
		} \
	} 


#define ENDOCCLUSIONQUERY \
	if (render_geom) { \
		if (potentialOccluderCount < OccQuerySize) { \
			if (node->__occludeCheckCount < 0) { \
				/* printf ("glEndQuery node %u\n",node); */ \
				glEndQuery(GL_SAMPLES_PASSED); \
				potentialOccluderCount++; \
			} \
		} \
	} 

#define EXTENTTOBBOX
#define INITIALIZE_EXTENT        node->EXTENT_MAX_X = -10000.0; \
        node->EXTENT_MAX_Y = -10000.0; \
        node->EXTENT_MAX_Z = -10000.0; \
        node->EXTENT_MIN_X = 10000.0; \
        node->EXTENT_MIN_Y = 10000.0; \
        node->EXTENT_MIN_Z = 10000.0;

/********************************
	Verbosity
*********************************/
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

/* External Authoring Interface */
extern int eaiverbose;

/* number of tesselated coordinates allowed */
#define TESS_MAX_COORDS  500

#define offset_of(p_type,field) ((unsigned int)(&(((p_type)NULL)->field)-NULL))

#define UNUSED(v) ((void) v)
#define ISUSED(v) ((void) v)

#define BOOL_STRING(b) (b ? "TRUE" : "FALSE")

#ifdef M_PI
#define PI M_PI
#else
#define PI 3.141592653589793
#endif
/* return TRUE if numbers are very close */
#define APPROX(a,b) (fabs(a-b)<0.00000001)
/* defines for raycasting: */

#define NORMAL_VECTOR_LENGTH_TOLERANCE 0.00001
/* (test if the vector part of a rotation is normalized) */
#define IS_ROTATION_VEC_NOT_NORMAL(rot)        ( \
       fabs(1-sqrt(rot.r[0]*rot.r[0]+rot.r[1]*rot.r[1]+rot.r[2]*rot.r[2])) \
               >NORMAL_VECTOR_LENGTH_TOLERANCE \
)

/* from VRMLC.pm */
extern int displayOpenGLErrors;
extern int sound_from_audioclip;
extern int have_texture;
extern float last_transparency;
extern int global_lineProperties;
extern int global_fillProperties;
extern int fullscreen;
extern float gl_linewidth;
extern int soundWarned;
extern int cur_hits;
extern struct point_XYZ hyper_r1,hyper_r2;

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
/* trat: test if a ratio is reasonable */
#undef TRAT
#define TRAT(a) 1
#undef TRAT
#define TRAT(a) ((a) > 0 && ((a) < hpdist || hpdist < 0))

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

/* Used to determine in Group, etc, if a child is a DirectionalLight; do comparison with this */
void DirectionalLight_Rend(void *nod_);
#define DIRLIGHTCHILDREN(a) \
	if ((node->_renderFlags & VF_DirectionalLight)==VF_DirectionalLight)dirlightChildren(a);

#define DIRECTIONAL_LIGHT_OFF if ((node->_renderFlags & VF_DirectionalLight)==VF_DirectionalLight) { \
		lightState(savedlight+1,FALSE); curlight = savedlight; }
#define DIRECTIONAL_LIGHT_SAVE int savedlight = curlight;

void normalize_ifs_face (float *point_normal,
                         struct point_XYZ *facenormals,
                         int *pointfaces,
                        int mypoint,
                        int curpoly,
                        float creaseAngle);


void FW_rendertext(unsigned int numrows,struct Uni_String **ptr,char *directstring, unsigned int nl, double *length,
                double maxext, double spacing, double mysize, unsigned int fsparam,
                struct X3D_PolyRep *rp);


/* Triangulator extern defs - look in CFuncs/Tess.c */
extern struct X3D_PolyRep *global_tess_polyrep;
extern GLUtriangulatorObj *global_tessobj;
extern int global_IFS_Coords[];
extern int global_IFS_Coord_count;

/* do we have to do textures?? */
#define HAVETODOTEXTURES (texture_count != 0)

/* multitexture and single texture handling */
#define MAX_MULTITEXTURE 10

/* texture stuff - see code. Need array because of MultiTextures */
extern GLuint bound_textures[MAX_MULTITEXTURE];
extern int bound_texture_alphas[MAX_MULTITEXTURE];
extern GLint maxTexelUnits;
extern int texture_count; 
extern int     *global_tcin;
extern int     global_tcin_count; 
extern void 	*global_tcin_lastParent;

extern void textureDraw_start(struct X3D_IndexedFaceSet *texC, GLfloat *tex);
extern void textureDraw_end(void);

extern void * this_textureTransform;  /* do we have some kind of textureTransform? */

extern int isTextureLoaded(int texno);
extern int isTextureAlpha(int n);
extern int displayDepth;
extern int display_status;

extern int _fw_pipe, _fw_FD;
extern int _fw_browser_plugin;
#define RUNNINGASPLUGIN (isBrowserPlugin)

#define RUNNINGONAMD64 (sizeof(void *) == 8)

/* appearance does material depending on last texture depth */
#define NOTEXTURE 0
#define TEXTURE_NO_ALPHA 1
#define TEXTURE_ALPHA 2

extern int last_texture_type;
extern float global_transparency;

/* what is the max texture size as set by FreeWRL? */
extern GLint global_texSize;


/* Text node system fonts. On startup, freewrl checks to see where the fonts
 * are stored
 */
#define fp_name_len 256
extern char sys_fp[fp_name_len];


extern float AC_LastDuration[];

extern int SoundEngineStarted;

/* Material optimizations */
void do_shininess (float shininess);
void do_glMaterialfv (GLenum face, GLenum pname, GLfloat *param);

/* used to determine whether we have transparent materials. */
extern int have_transparency;


/* current time */
extern double TickTime;
extern double lastTime;

/* number of triangles this rendering loop */
extern int trisThisLoop;


/* Transform node optimizations */
int verify_rotate(GLfloat *params);
int verify_translate(GLfloat *params);
int verify_scale(GLfloat *params);

void mark_event (struct X3D_Node *from, unsigned int fromoffset);
void mark_event_check (struct X3D_Node *from, unsigned int fromoffset,char *fn, int line);

/* saved rayhit and hyperhit */
extern struct SFColor ray_save_posn, hyp_save_posn, hyp_save_norm;

/* set a node to be sensitive */
void setSensitive(struct X3D_Node *parent,struct X3D_Node *me);

/* bindable nodes */
extern GLint viewport[];
extern GLdouble fieldofview;
extern struct point_XYZ ViewerUpvector;
extern struct sNaviInfo naviinfo;
extern double defaultExamineDist;


/* Sending events back to Browser (eg, Anchor) */
extern int BrowserAction;
extern struct X3D_Anchor *AnchorsAnchor;
extern uintptr_t _fw_instance;
int checkIfX3DVRMLFile(char *fn);
void Anchor_ReplaceWorld (char *fn);
void EAI_Anchor_Response (int resp);
extern int wantEAI;
struct Uni_String *newASCIIString(char *str);
void verify_Uni_String(struct  Uni_String *unis, char *str);

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




void CRoutes_js_new (uintptr_t num,int scriptType);
extern int max_script_found;
extern int max_script_found_and_initialized;
void getMFNodetype (char *strp, struct Multi_Node *ch, struct X3D_Node *par, int ar);
void AddRemoveChildren (struct X3D_Node *parent, struct Multi_Node *tn, uintptr_t *nodelist, int len, int ar);

void update_node(struct X3D_Node *ptr);
void update_renderFlag(struct X3D_Node *ptr, int flag);

int JSparamIndex (char *name, char *type);

/* setting script eventIns from routing table or EAI */
void Set_one_MultiElementtype (uintptr_t tn, uintptr_t tptr, void *fn, unsigned len);
void set_one_ECMAtype (uintptr_t tonode, int toname, int dataType, void *Data, unsigned datalen);
void mark_script (uintptr_t num);


/* structure for rayhits */
struct currayhit {
	struct X3D_Node *node; /* What node hit at that distance? */
	GLdouble modelMatrix[16]; /* What the matrices were at that node */
	GLdouble projMatrix[16];
};



void JSMaxAlloc(void);
void cleanupDie(uintptr_t num, const char *msg);
void shutdown_EAI(void);
uintptr_t EAI_GetNode(const char *str);
unsigned int EAI_GetViewpoint(const char *str);
void EAI_killBindables (void);
void EAI_GetType (uintptr_t cNode,  char *ctmp, char *dtmp, uintptr_t *cNodePtr, uintptr_t *fieldOffset,
                        uintptr_t *dataLen, uintptr_t *typeString,  unsigned int *scripttype, int *accessType);


void setScriptECMAtype(uintptr_t);
void resetScriptTouchedFlag(int actualscript, int fptr);
int get_touched_flag(uintptr_t fptr, uintptr_t actualscript);
void getMultiElementtype(char *strp, struct Multi_Vec3f *tn, int eletype);
void setScriptMultiElementtype(uintptr_t);
void Parser_scanStringValueToMem(struct X3D_Node *ptr, int coffset, int ctype, char *value);
void Multimemcpy(void *tn, void *fn, int len);
void CRoutes_RegisterSimple(struct X3D_Node* from, int fromOfs,
 struct X3D_Node* to, int toOfs, int len, int dir);
void CRoutes_Register(int adrem,        struct X3D_Node *from,
                                 int fromoffset,
                                 unsigned int to_count,
                                 char *tonode_str,
                                 int length,
                                 void *intptr,
                                 int scrdir,
                                 int extra);
void CRoutes_free(void);
void propagate_events(void);
int getRoutesCount(void);
void getSpecificRoute (int routeNo, uintptr_t *fromNode, int *fromOffset, 
                uintptr_t *toNode, int *toOffset);
void sendScriptEventIn(uintptr_t num);
void getField_ToJavascript (int num, int fromoffset);
void add_first(struct X3D_Node * node);
void registerTexture(struct X3D_Node * node);
void registerMIDINode(struct X3D_Node *node);
int checkNode(struct X3D_Node *node, char *fn, int line);


void do_first(void);
void process_eventsProcessed(void);

void getEAI_MFStringtype (struct Multi_String *from, struct Multi_String *to);


extern struct CRscriptStruct *ScriptControl; /* Script invocation parameters */
extern uintptr_t *scr_act;    /* script active array - defined in CRoutes.c */
extern int *thisScriptType;    /* what kind of script this is - in CRoutes.c */
extern int JSMaxScript;  /* defined in JSscipts.c; maximum size of script arrays */
void JSCreateScriptContext(uintptr_t num); 
void JSInitializeScriptAndFields (uintptr_t num);
void SaveScriptText(uintptr_t num, char *text);


void update_status(char* msg);
void kill_status();

/* menubar stuff */
void frontendUpdateButtons(void); /* used only if we are not able to multi-thread OpenGL */
void setMenuButton_collision (int val) ;
void setMenuButton_headlight (int val) ;
void setMenuButton_navModes (int type) ;
void setConsoleMessage(char *stat) ;
void setMenuStatus(char *stat) ;
void setMenuFps (float fps) ;
void setMenuButton_texSize (int size);
extern int textures_take_priority;
void setTextures_take_priority (int x);
extern int useExperimentalParser;
void setUseCParser (int x);
extern int useShapeThreadIfPossible;
void setUseShapeThreadIfPossible(int x);

int convert_typetoInt (const char *type);	/* convert a string, eg "SFBOOL" to type, eg SFBOOL */

extern double BrowserFPS;
extern double BrowserSpeed;
void render_polyrep(void *node);

extern int CRoutesExtra;		/* let EAI see param of routing table - Listener data. */

/* types of scripts. */
#define NOSCRIPT 	0
#define JAVASCRIPT	1
#define SHADERSCRIPT	4

#ifdef die
#undef die
#endif


/* types to tell the Perl thread what to handle */
#define FROMSTRING 	1
#define	FROMURL		2
#define INLINE		3
#define ZEROBINDABLES   8   /* get rid of Perl datastructures */
#define FROMCREATENODE	13  /* create a node by just giving its node type */
#define FROMCREATEPROTO	14  /* create a node by just giving its node type */
#define UPDATEPROTOD	16  /* update a PROTO definition */
#define GETPROTOD	17  /* update a PROTO definition */



extern void *rootNode;
extern int isPerlParsing(void);
extern int isURLLoaded(void);	/* initial scene loaded? Robert Sim */
extern int isTextureParsing(void);
extern void loadInline(struct X3D_Inline *node);
extern void loadTextureNode(struct X3D_Node *node,  void *param);
extern void loadMovieTexture(struct X3D_MovieTexture *node,  void *param);
extern void loadMultiTexture(struct X3D_MultiTexture *node);
extern void loadBackgroundTextures (struct X3D_Background *node);
extern void loadTextureBackgroundTextures (struct X3D_TextureBackground *node);
extern GLfloat boxtex[], boxnorms[], BackgroundVert[];
extern GLfloat Backtex[], Backnorms[];

extern void new_tessellation(void);
extern void initializePerlThread(void);
extern void setGeometry (const char *optarg);
extern void setWantEAI(int flag);
extern void setPluginPipe(const char *optarg);
extern void setPluginFD(const char *optarg);
extern void setPluginInstance(const char *optarg);

/* shutter glasses, stereo view  from Mufti@rus */
extern void setShutter (void);
#ifndef AQUA
extern int shutterGlasses;
#endif
extern void setScreenDist (const char *optArg);
extern void setStereoParameter (const char *optArg);
extern void setEyeDist (const char *optArg);

extern int isPerlinitialized(void);

#ifdef HAVE_MOTIF
	#define ISDISPLAYINITIALIZED isMotifDisplayInitialized()
	#define GET_GLWIN getMotifWindowedGLwin (&GLwin);
	#define OPEN_TOOLKIT_MAINWINDOW openMotifMainWindow (argc, argv);
	#define CREATE_TOOLKIT_MAIN_WINDOW createMotifMainWindow();
	int isMotifDisplayInitialized(void);
	void getMotifWindowedGLwin (Window *);
	void openMotifMainWindow (int argc, char ** argv);
	void createMotifMainWindow(void);
#else

	#ifdef HAVE_GTK2
		#define ISDISPLAYINITIALIZED isGtkDisplayInitialized()
		#define GET_GLWIN getGtkWindowedGLwin (&GLwin);
		#define OPEN_TOOLKIT_MAINWINDOW openGtkMainWindow (argc, argv);
		#define CREATE_TOOLKIT_MAIN_WINDOW createGtkMainWindow();
		int isGtkDisplayInitialized(void);
		void getGtkWindowedGLwin (Window *);
		void openGtkMainWindow (int argc, char ** argv);
		void createGtkMainWindow(void);

	#else
		#define HAVE_NOTOOLKIT
		#define ISDISPLAYINITIALIZED TRUE
		#define GET_GLWIN getBareWindowedGLwin (&GLwin);
		#define OPEN_TOOLKIT_MAINWINDOW openBareMainWindow (argc, argv);
		#define CREATE_TOOLKIT_MAIN_WINDOW createBareMainWindow();
	#endif
#endif

extern char *getInputURL(void);
extern char *keypress_string;
extern char *lastReadFile; 		/* name last file read in */
extern int be_collision;		/* toggle collision detection - defined in VRMLC.pm */
extern int  lightingOn;			/* state of GL_LIGHTING */
extern int cullFace;			/* state of GL_CULL_FACE */
extern int colorMaterialEnabled;	/* state of GL_COLOR_MATERIAL */
extern double hpdist;			/* in VRMLC.pm */
extern struct point_XYZ hp;			/* in VRMLC.pm */
extern void *hypersensitive; 		/* in VRMLC.pm */
extern int hyperhit;			/* in VRMLC.pm */
extern struct point_XYZ r1, r2;		/* in VRMLC.pm */
extern struct sCollisionInfo CollisionInfo;
extern struct currayhit rayHit,rayph,rayHitHyper;
extern GLint smooth_normals;

extern void xs_init(void);

extern int navi_tos;
extern void initializeTextureThread(void);
extern int isTextureinitialized(void);
extern int fileExists(char *fname, char *firstBytes, int GetIt, int *isTemp);
extern int checkNetworkFile(char *fn);
extern void checkAndAllocMemTables(int *texture_num, int increment);
extern void   storeMPGFrameData(int latest_texture_number, int h_size, int v_size,
        int mt_repeatS, int mt_repeatT, char *Image);
void mpg_main(char *filename, int *x,int *y,int *depth,int *frameCount,void **ptr);
void makeAbsoluteFileName(char *filename, char *pspath,char *thisurl);
int getValidFileFromUrl (char *filename, char *path, struct Multi_String *inurl, char *firstBytes);
void removeFilenameFromPath (char *path);


void create_EAI(void);
int EAI_CreateVrml(const char *tp, const char *inputstring, uintptr_t *retarr, int retarrsize);
void EAI_Route(char cmnd, const char *tf);
void EAI_replaceWorld(const char *inputstring);

void render_hier(struct X3D_Node *p, int rwhat);
void handle_EAI(void);
void handle_aqua(const int mev, const unsigned int button, int x, int y);


extern int screenWidth, screenHeight;


#define overMark        23425
/* mimic X11 events in AQUA */
#ifdef AQUA
#define KeyPress        2
#define KeyRelease      3
#define ButtonPress     4
#define ButtonRelease   5
#define MotionNotify    6
#define MapNotify       19

#endif

/* SD AQUA FUNCTIONS */
#ifdef AQUA
extern int getOffset();
extern void initGL();
extern void setButDown(int button, int value);
extern void setCurXY(int x, int y);
extern void do_keyPress (char ch, int ev);
extern void setLastMouseEvent(int etype);
extern void initFreewrl();
extern void aqDisplayThread();
#endif
extern void setSnapSeq();
extern void setEAIport(int pnum);
extern void setTexSize(int num);
extern void setKeyString(const char *str);
extern void setNoCollision();
extern void setSnapGif();
extern void setLineWidth(float lwidth);
extern void closeFreewrl();
extern void setSeqFile(const char* file);
extern void setSnapFile(const char* file);
extern void setMaxImages(int max);
extern void setBrowserFullPath(const char *str);
extern void setSeqTemp(const char* file);
extern void setFullPath(const char *str);
extern void setInstance(uintptr_t instance);
extern void setScreenDim(int w, int h);

extern char *getLibVersion();
extern void doQuit(void);
extern void doBrowserAction ();


extern char *myPerlInstallDir;

/* for Extents and BoundingBoxen */
#define EXTENT_MAX_X _extent[0]
#define EXTENT_MIN_X _extent[1]
#define EXTENT_MAX_Y _extent[2]
#define EXTENT_MIN_Y _extent[3]
#define EXTENT_MAX_Z _extent[4]
#define EXTENT_MIN_Z _extent[5]
void setExtent (float maxx, float minx, float maxy, float miny, float maxz, float minz, struct X3D_Node *this_);

#define RECORD_DISTANCE if (render_geom) recordDistance (X3D_NODE(node));
void recordDistance(struct X3D_Node *nod);

void propagateExtent (struct X3D_Node *this_);

#ifdef DISPLAYBOUNDINGBOX
void BoundingBox(struct X3D_Node* node);
#define BOUNDINGBOX if (render_geom && (!render_blend)) BoundingBox (X3D_NODE(node));
#else
#define BOUNDINGBOX
#endif

void freewrlDie (const char *format);
char * readInputString(char *fn);
char * sanitizeInputString(char *instr);

extern double nearPlane, farPlane, screenRatio, calculatedNearPlane, calculatedFarPlane;

/* children stuff moved out of VRMLRend.pm and VRMLC.pm for v1.08 */

extern int render_sensitive,render_vp,render_light,render_proximity,curlight,verbose,render_blend,render_geom,render_collision;

extern void XEventStereo();


/* Java CLASS invocation */
int newJavaClass(int scriptInvocationNumber,char * nodestr,char *node);
int initJavaClass(int scriptno);

int SAI_IntRetCommand (char cmnd, const char *fn);
char * SAI_StrRetCommand (char cmnd, const char *fn);
char *EAI_GetTypeName (unsigned int uretval);
char* EAI_GetValue(unsigned int nodenum, const char *fieldname, const char *nodename);
void setCLASStype (uintptr_t num);
void sendCLASSEvent(uintptr_t fn, int scriptno, char *fieldName, int type, int len);
void processClassEvents(int scriptno, int startEntry, int endEntry);
char *processThisClassEvent (void *fn, int startEntry, int endEntry, char *buf);
void getCLASSMultNumType (char *buf, int bufSize,
	struct Multi_Vec3f *tn,
	struct X3D_Node *parent,
	int eletype, int addChild);

void fwGetDoublev (int ty, double *mat);
void fwMatrixMode (int mode);
void fwXformPush(void);
void fwXformPop(void);
void fwLoadIdentity (void);
void invalidateCurMat(void);
void doBrowserAction (void);
void add_parent(struct X3D_Node *node_, struct X3D_Node *parent_,char *file, int line);
void remove_parent(struct X3D_Node *child, struct X3D_Node *parent);
void EAI_readNewWorld(char *inputstring);
void make_indexedfaceset(struct X3D_IndexedFaceSet *this_);

void render_LoadSensor(struct X3D_LoadSensor *this);

void render_Text (struct X3D_Text * this_);
#define rendray_Text render_ray_polyrep
void make_Text (struct X3D_Text * this_);
void collide_Text (struct X3D_Text * this_);
void render_TextureCoordinateGenerator(struct X3D_TextureCoordinateGenerator *this);
void render_TextureCoordinate(struct X3D_TextureCoordinate *this);

/* Component Grouping */
void prep_Transform (struct X3D_Transform *this_);
void fin_Transform (struct X3D_Transform *this_);
void child_Transform (struct X3D_Transform *this_);
void prep_Group (struct X3D_Group *this_);
void child_Group (struct X3D_Group *this_);
void child_StaticGroup (struct X3D_StaticGroup *this_);
void child_Switch (struct X3D_Switch *this_);

void changed_Group (struct X3D_Group *this_);
void changed_Switch (struct X3D_Switch *this_);
void changed_StaticGroup (struct X3D_StaticGroup *this_);
void changed_Transform (struct X3D_Transform *this_);

/* Environmental Sensor nodes */
void proximity_ProximitySensor (struct X3D_ProximitySensor *this_);
void child_VisibilitySensor (struct X3D_VisibilitySensor *this_);

/* Navigation Component */
void prep_Billboard (struct X3D_Billboard *this_);
void proximity_Billboard (struct X3D_Billboard *this_);
void changed_Billboard (struct X3D_Billboard *this_);
void prep_Viewpoint(struct X3D_Viewpoint *node);
void child_Billboard (struct X3D_Billboard *this_);
void child_LOD (struct X3D_LOD *this_);
void proximity_LOD (struct X3D_LOD *this_);
void fin_Billboard (struct X3D_Billboard *this_);
void child_Collision (struct X3D_Collision *this_);
void changed_Collision (struct X3D_Collision *this_);


/* HAnim Component */
void prep_HAnimJoint (struct X3D_HAnimJoint *this);
void prep_HAnimSite (struct X3D_HAnimSite *this);

void child_HAnimHumanoid(struct X3D_HAnimHumanoid *this_); 
void child_HAnimJoint(struct X3D_HAnimJoint *this_); 
void child_HAnimSegment(struct X3D_HAnimSegment *this_); 
void child_HAnimSite(struct X3D_HAnimSite *this_); 

void render_HAnimHumanoid (struct X3D_HAnimHumanoid *node);
void render_HAnimJoint (struct X3D_HAnimJoint * node);

void fin_HAnimSite (struct X3D_HAnimSite *this_);
void fin_HAnimJoint (struct X3D_HAnimJoint *this_);

void changed_HAnimSite (struct X3D_HAnimSite *this_);



/* Sound Component */
void render_Sound (struct X3D_Sound *this_);
void render_AudioControl (struct X3D_AudioControl *this_);
void render_AudioClip (struct X3D_AudioClip *this_);

/* Texturing Component */
void render_PixelTexture (struct X3D_PixelTexture *this_);
void render_ImageTexture (struct X3D_ImageTexture *this_);
void render_MultiTexture (struct X3D_MultiTexture *this_);
void render_MovieTexture (struct X3D_MovieTexture *this_);

/* Shape Component */
void render_Appearance (struct X3D_Appearance *this_);
void render_FillProperties (struct X3D_FillProperties *this_);
void render_LineProperties (struct X3D_LineProperties *this_);
void render_Material (struct X3D_Material *this_);
void render_TwoSidedMaterial (struct X3D_TwoSidedMaterial *this_);
void render_Shape (struct X3D_Shape *this_);
void child_Shape (struct X3D_Shape *this_);
void child_Appearance (struct X3D_Appearance *this_);

/* Geometry3D nodes */
void render_Box (struct X3D_Box *this);
void compile_Box (struct X3D_Box *this);
void collide_Box (struct X3D_Box *this);
void render_Cone (struct X3D_Cone *this);
void compile_Cone (struct X3D_Cone *this);
void collide_Cone (struct X3D_Cone *this);
void render_Cylinder (struct X3D_Cylinder *this);
void compile_Cylinder (struct X3D_Cylinder *this);
void collide_Cylinder (struct X3D_Cylinder *this);
void render_ElevationGrid (struct X3D_ElevationGrid *this);
#define rendray_ElevationGrid  render_ray_polyrep
#define collide_ElevationGrid collide_IndexedFaceSet
void render_Extrusion (struct X3D_Extrusion *this);
void collide_Extrusion (struct X3D_Extrusion *this);
#define rendray_Extrusion render_ray_polyrep
void render_IndexedFaceSet (struct X3D_IndexedFaceSet *this);
void collide_IndexedFaceSet (struct X3D_IndexedFaceSet *this);
#define rendray_IndexedFaceSet render_ray_polyrep 
void render_IndexedFaceSet (struct X3D_IndexedFaceSet *this);
void render_Sphere (struct X3D_Sphere *this);
void compile_Sphere (struct X3D_Sphere *this);
void collide_Sphere (struct X3D_Sphere *this);
void make_Extrusion (struct X3D_Extrusion *this);
#define make_IndexedFaceSet make_indexedfaceset
#define make_ElevationGrid make_indexedfaceset
#define rendray_ElevationGrid render_ray_polyrep
void rendray_Box (struct X3D_Box *this_);
void rendray_Sphere (struct X3D_Sphere *this_);
void rendray_Cylinder (struct X3D_Cylinder *this_);
void rendray_Cone (struct X3D_Cone *this_);

/* Geometry2D nodes */
void render_Arc2D (struct X3D_Arc2D *this_);
void compile_Arc2D (struct X3D_Arc2D *this_);
void render_ArcClose2D (struct X3D_ArcClose2D *this_);
void compile_ArcClose2D (struct X3D_ArcClose2D *this_);
void render_Circle2D (struct X3D_Circle2D *this_);
void compile_Circle2D (struct X3D_Circle2D *this_);
void render_Disk2D (struct X3D_Disk2D *this_);
void compile_Disk2D (struct X3D_Disk2D *this_);
void render_Polyline2D (struct X3D_Polyline2D *this_);
void render_Polypoint2D (struct X3D_Polypoint2D *this_);
void render_Rectangle2D (struct X3D_Rectangle2D *this_);
void compile_Rectangle2D (struct X3D_Rectangle2D *this_);
void render_TriangleSet2D (struct X3D_TriangleSet2D *this_);
void compile_TriangleSet2D (struct X3D_TriangleSet2D *this_);
void collide_Disk2D (struct X3D_Disk2D *this_);
void collide_Rectangle2D (struct X3D_Rectangle2D *this_);
void collide_TriangleSet2D (struct X3D_TriangleSet2D *this_);

/* Component Rendering nodes */
#define rendray_IndexedTriangleSet render_ray_polyrep
#define rendray_IndexedTriangleFanSet render_ray_polyrep
#define rendray_IndexedTriangleStripSet render_ray_polyrep
#define rendray_TriangleSet render_ray_polyrep
#define rendray_TriangleFanSet render_ray_polyrep
#define rendray_TriangleStripSet render_ray_polyrep
void render_IndexedTriangleFanSet (struct X3D_IndexedTriangleFanSet *this_); 
void render_IndexedTriangleSet (struct X3D_IndexedTriangleSet *this_); 
void render_IndexedTriangleStripSet (struct X3D_IndexedTriangleStripSet *this_); 
void render_TriangleFanSet (struct X3D_TriangleFanSet *this_); 
void render_TriangleStripSet (struct X3D_TriangleStripSet *this_); 
void render_TriangleSet (struct X3D_TriangleSet *this_); 
void render_LineSet (struct X3D_LineSet *this_); 
void render_IndexedLineSet (struct X3D_IndexedLineSet *this_); 
void render_PointSet (struct X3D_PointSet *this_); 
#define collide_IndexedTriangleFanSet  collide_IndexedFaceSet
#define collide_IndexedTriangleSet  collide_IndexedFaceSet
#define collide_IndexedTriangleStripSet  collide_IndexedFaceSet
#define collide_TriangleFanSet  collide_IndexedFaceSet
#define collide_TriangleSet  collide_IndexedFaceSet
#define collide_TriangleStripSet  collide_IndexedFaceSet
#define make_IndexedTriangleFanSet  make_indexedfaceset
#define make_IndexedTriangleSet  make_indexedfaceset
#define make_IndexedTriangleStripSet  make_indexedfaceset
#define make_TriangleFanSet  make_indexedfaceset
#define make_TriangleSet  make_indexedfaceset
#define make_TriangleStripSet  make_indexedfaceset
void compile_LineSet (struct X3D_LineSet *this_); 
void compile_IndexedLineSet (struct X3D_IndexedLineSet *this_); 

/* Component Lighting Nodes */
void render_DirectionalLight (struct X3D_DirectionalLight *this_);
void prep_SpotLight (struct X3D_SpotLight *this_);
void prep_PointLight (struct X3D_PointLight *this_);

/* Geospatial nodes */
int checkX3DGeoElevationGridFields (struct X3D_ElevationGrid *node, float **points, int *npoints);
void render_GeoElevationGrid (struct X3D_GeoElevationGrid *this_);
void rendray_GeoElevationGrid (struct X3D_GeoElevationGrid *this_);
void collide_GeoElevationGrid (struct X3D_GeoElevationGrid *this_);
void make_GeoElevationGrid (struct X3D_GeoElevationGrid *this_);
void prep_GeoLocation (struct X3D_GeoLocation *this_);
void prep_GeoViewpoint (struct X3D_GeoViewpoint *this_);
void fin_GeoLocation (struct X3D_GeoLocation *this_);
void changed_GeoLocation (struct X3D_GeoLocation *this_);
void child_GeoLOD (struct X3D_GeoLOD *this_);
void proximity_GeoLOD (struct X3D_GeoLOD *this_);
void child_GeoLocation (struct X3D_GeoLocation *this_);
void compile_GeoCoordinate (struct X3D_GeoCoordinate * this);
void compile_GeoElevationGrid (struct X3D_GeoElevationGrid * this);
void compile_GeoLocation (struct X3D_GeoLocation * this);
void compile_GeoLOD (struct X3D_GeoLOD * this);
void compile_GeoMetadata (struct X3D_GeoMetadata * this);
void compile_GeoOrigin (struct X3D_GeoOrigin * this);
void compile_GeoPositionInterpolator (struct X3D_GeoPositionInterpolator * this);
void compile_GeoTouchSensor (struct X3D_GeoTouchSensor * this);
void compile_GeoViewpoint (struct X3D_GeoViewpoint * this);
void compile_GeoProximitySensor (struct X3D_GeoProximitySensor *this);
void compile_GeoTransform (struct X3D_GeoTransform * node);
void proximity_GeoProximitySensor (struct X3D_GeoProximitySensor *this);
void prep_GeoTransform (struct X3D_GeoTransform *);
void child_GeoTransform (struct X3D_GeoTransform *);
void fin_GeoTransform (struct X3D_GeoTransform *);
void changed_GeoTransform (struct X3D_GeoTransform *);


/* Networking Component */
void child_Anchor (struct X3D_Anchor *this_);
void child_Inline (struct X3D_Inline *this_);
void changed_Inline (struct X3D_Inline *this_);
void changed_Anchor (struct X3D_Anchor *this_);

/* KeyDevice Component */
void killKeySensorNodeList(void);
void addNodeToKeySensorList(struct X3D_Node* node);
int KeySensorNodePresent(void);
void sendKeyToKeySensor(const char key, int upDown);

/* Programmable Shaders Component */
void render_ComposedShader (struct X3D_ComposedShader *);
void render_PackagedShader (struct X3D_PackagedShader *);
void render_ProgramShader (struct X3D_ProgramShader *);
void compile_ComposedShader (struct X3D_ComposedShader *);
void compile_PackagedShader (struct X3D_PackagedShader *);
void compile_ProgramShader (struct X3D_ProgramShader *);
#define TURN_APPEARANCE_SHADER_OFF {extern GLuint globalCurrentShader; if (globalCurrentShader!=0) { globalCurrentShader = 0; glUseProgram(0);}}

void prep_MidiControl (struct X3D_MidiControl *node);
void do_MidiControl (void *node);
void MIDIRegisterMIDI(char *str);
/* ReWire device/controller  table */
struct ReWireDeviceStruct {
        struct X3D_MidiControl* node;   /* pointer to the node that controls this */
        int encodedDeviceName;          /* index into ReWireNamenames */
        int bus;                        /* which MIDI bus this is */
        int channel;                    /* which MIDI channel on this bus it is */
        int encodedControllerName;      /* index into ReWireNamenames */
        int controller;                 /* controller number */
        int cmin;                       /* minimum value for this controller */
        int cmax;                       /* maximum value for this controller */
        int ctype;                      /* controller type TYPE OF FADER control - not used currently */
};

/* ReWireName table */
struct ReWireNamenameStruct {
        char *name;
};

extern struct ReWireNamenameStruct *ReWireNamenames;
extern int ReWireNametableSize;
extern int MAXReWireNameNames;
extern struct ReWireDeviceStruct *ReWireDevices;
extern int ReWireDevicetableSize;
extern int MAXReWireDevices;


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


/* OpenGL state cache */
/* solid shapes, GL_CULL_FACE can be enabled if a shape is Solid */ 
#define CULL_FACE(v) /* printf ("nodeSolid %d cullFace %d GL_FALSE %d FALSE %d\n",v,cullFace,GL_FALSE,FALSE); */ \
		if (v != cullFace) {	\
			cullFace = v; \
			if (cullFace == 1) glEnable(GL_CULL_FACE);\
			else glDisable(GL_CULL_FACE);\
		}
#define DISABLE_CULL_FACE CULL_FACE(0)
#define ENABLE_CULL_FACE CULL_FACE(1)
#define CULL_FACE_INITIALIZE cullFace=0; glDisable(GL_CULL_FACE);

#define LIGHTING_ON if (!lightingOn) {lightingOn=TRUE;glEnable(GL_LIGHTING);}
#define LIGHTING_OFF if(lightingOn) {lightingOn=FALSE;glDisable(GL_LIGHTING);}
#define LIGHTING_INITIALIZE lightingOn=TRUE; glEnable(GL_LIGHTING);

#define COLOR_MATERIAL_ON if (colorMaterialEnabled == GL_FALSE) {colorMaterialEnabled=GL_TRUE;glEnable(GL_COLOR_MATERIAL);}
#define COLOR_MATERIAL_OFF if (colorMaterialEnabled == GL_TRUE) {colorMaterialEnabled=GL_FALSE;glDisable(GL_COLOR_MATERIAL);}
#define COLOR_MATERIAL_INITIALIZE colorMaterialEnabled = GL_FALSE; glDisable(GL_COLOR_MATERIAL);

void zeroAllBindables(void);
void Next_ViewPoint(void);
void Prev_ViewPoint(void);
void First_ViewPoint(void);
void Last_ViewPoint(void);

int freewrlSystem (const char *string);

int inputParse(unsigned type, char *inp, int bind, int returnifbusy,
                        void *ptr, unsigned ofs, int *complete,
                        int zeroBind);
void compileNode (void (*nodefn)(void *, void *, void *, void *, void *), void *node, void *a, void *b, void *c, void *d);
void destroyCParserData();

void getMovieTextureOpenGLFrames(int *highest, int *lowest,int myIndex);


int ConsoleMessage(const char *fmt, ...);

void closeConsoleMessage(void);
extern int consMsgCount;

void outOfMemory(const char *message);

void killErrantChildren(void);

void kill_routing(void);
void kill_bindables(void);
void kill_javascript(void);
void kill_oldWorld(int a, int b, int c);
void kill_clockEvents(void);
void kill_openGLTextures(void);
void kill_X3DDefs(void);
extern int currentFileVersion;

int findFieldInFIELDNAMES(const char *field);
int findFieldInFIELD(const char* field);
int findFieldInFIELDTYPES(const char *field);
int findFieldInX3DACCESSORS(const char *field);
int findFieldInEXPOSED_FIELD(const char* field);
int findFieldInEVENT_IN(const char* field);
int findFieldInEVENT_OUT(const char* field);
int findFieldInX3DSPECIALKEYWORDS(const char *field);
int findFieldInGEOSPATIAL(const char *field);

/* Values for fromTo */
#define ROUTED_FIELD_EVENT_OUT 0
#define ROUTED_FIELD_EVENT_IN  1
int findRoutedFieldInFIELDNAMES(struct X3D_Node *node, const char *field, int fromTo);
int findRoutedFieldInEXPOSED_FIELD(struct X3D_Node*, const char*, int);
int findRoutedFieldInEVENT_IN(struct X3D_Node*, const char*, int);
int findRoutedFieldInEVENT_OUT(struct X3D_Node*, const char*, int);
int findFieldInNODES(const char *node);
int findFieldInCOMPONENTS(const char *node);
int findFieldInPROFILESS(const char *node);
int findFieldInALLFIELDNAMES(const char *field);
void findFieldInOFFSETS(const int *nodeOffsetPtr, const int field, int *coffset, int *ctype, int *ckind);
char *findFIELDNAMESfromNodeOffset(struct X3D_Node *node, int offset);
int findFieldInKEYWORDS(const char *field);
int findFieldInPROTOKEYWORDS(const char *field);
int countCommas (char *instr);
void sortChildren (struct Multi_Node ch);
void dirlightChildren(struct Multi_Node ch);
void normalChildren(struct Multi_Node ch);
void checkParentLink (struct X3D_Node *node,struct X3D_Node *parent);

/* background colour */
void setglClearColor (float *val); 
void doglClearColor(void);
extern int cc_changed;
extern int forceBackgroundRecompile;

char *findPathToFreeWRLFile(char *lfn);

int mapFieldTypeToInernaltype (indexT kwIndex);
void finishEventLoop();
void resetEventLoop();

/* MIDI stuff... */
void ReWireRegisterMIDI (char *str);
void ReWireMIDIControl(char *str);


/* META data, component, profile  stuff */
void handleMetaDataStringString(struct Uni_String *val1,struct Uni_String *val2);
void handleProfile(int myp);
void handleComponent(int com, int lev);
void handleExport (char *node, char *as);
void handleImport (char *nodeName,char *nodeImport, char *as);
void handleVersion (float lev);


/* free memory */
void registerX3DNode(struct X3D_Node * node);

void doNotRegisterThisNodeForDestroy(struct X3D_Node * nodePtr);

struct Multi_Vec3f *getCoordinate (void *node, char *str);

#ifdef AQUA
Boolean notFinished();
void disposeContext();
void setPaneClipRect(int npx, int npy, WindowPtr fwWindow, int ct, int cb, int cr, int cl, int width, int height);
void createContext(CGrafPtr grafPtr);
void setIsPlugin();
void sendPluginFD(int fd);
void aquaPrintVersion();
void setPluginPath(char* path);
#endif
void setEaiVerbose();
void replaceWorldNeeded(char* str);

/* X3D C parser */
int X3DParse (struct X3D_Group *parent, char *inputstring);
void *createNewX3DNode (int nt);

/* this is set by OSX, or to FALSE if on Linux. */
#ifdef AQUA
extern Boolean isBrowserPlugin;
#else
extern int isBrowserPlugin;
#endif


/* threading stuff */
extern pthread_t DispThrd;
extern pthread_t PCthread;
extern pthread_t shapeThread;
extern pthread_t loadThread;

/* node binding */
extern void *setViewpointBindInRender;
extern void *setFogBindInRender;
extern void *setBackgroundBindInRender;
extern void *setNavigationBindInRender;


/* ProximitySensor and GeoProximitySensor are same "code" at this stage of the game */
#define PROXIMITYSENSOR(type,center,initializer1,initializer2) \
void proximity_##type (struct X3D_##type *node) { \
	/* Viewer pos = t_r2 */ \
	double cx,cy,cz; \
	double len; \
	struct point_XYZ dr1r2; \
	struct point_XYZ dr2r3; \
	struct point_XYZ nor1,nor2; \
	struct point_XYZ ins; \
	static const struct point_XYZ yvec = {0,0.05,0}; \
	static const struct point_XYZ zvec = {0,0,-0.05}; \
	static const struct point_XYZ zpvec = {0,0,0.05}; \
	static const struct point_XYZ orig = {0,0,0}; \
	struct point_XYZ t_zvec, t_yvec, t_orig; \
	GLdouble modelMatrix[16]; \
	GLdouble projMatrix[16]; \
 \
	if(!((node->enabled))) return; \
	initializer1 \
	initializer2 \
 \
	/* printf (" vp %d geom %d light %d sens %d blend %d prox %d col %d\n",*/ \
	/* render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision);*/ \
 \
	/* transforms viewers coordinate space into sensors coordinate space. \
	 * this gives the orientation of the viewer relative to the sensor. \
	 */ \
	fwGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix); \
	fwGetDoublev(GL_PROJECTION_MATRIX, projMatrix); \
	gluUnProject(orig.x,orig.y,orig.z,modelMatrix,projMatrix,viewport, \
		&t_orig.x,&t_orig.y,&t_orig.z); \
	gluUnProject(zvec.x,zvec.y,zvec.z,modelMatrix,projMatrix,viewport, \
		&t_zvec.x,&t_zvec.y,&t_zvec.z); \
	gluUnProject(yvec.x,yvec.y,yvec.z,modelMatrix,projMatrix,viewport, \
		&t_yvec.x,&t_yvec.y,&t_yvec.z); \
 \
 \
	/*printf ("\n"); \
	printf ("unprojected, t_orig (0,0,0) %lf %lf %lf\n",t_orig.x, t_orig.y, t_orig.z); \
	printf ("unprojected, t_yvec (0,0.05,0) %lf %lf %lf\n",t_yvec.x, t_yvec.y, t_yvec.z); \
	printf ("unprojected, t_zvec (0,0,-0.05) %lf %lf %lf\n",t_zvec.x, t_zvec.y, t_zvec.z); \
	*/ \
	cx = t_orig.x - ((node->center ).c[0]); \
	cy = t_orig.y - ((node->center ).c[1]); \
	cz = t_orig.z - ((node->center ).c[2]); \
 \
	if(((node->size).c[0]) == 0 || ((node->size).c[1]) == 0 || ((node->size).c[2]) == 0) return; \
 \
	if(fabs(cx) > ((node->size).c[0])/2 || \
	   fabs(cy) > ((node->size).c[1])/2 || \
	   fabs(cz) > ((node->size).c[2])/2) return; \
	/* printf ("within (Geo)ProximitySensor\n"); */ \
 \
	/* Ok, we now have to compute... */ \
	(node->__hit) /*cget*/ = 1; \
 \
	/* Position */ \
	((node->__t1).c[0]) = t_orig.x; \
	((node->__t1).c[1]) = t_orig.y; \
	((node->__t1).c[2]) = t_orig.z; \
 \
	VECDIFF(t_zvec,t_orig,dr1r2);  /* Z axis */ \
	VECDIFF(t_yvec,t_orig,dr2r3);  /* Y axis */ \
 \
	/* printf ("      dr1r2 %lf %lf %lf\n",dr1r2.x, dr1r2.y, dr1r2.z); \
	printf ("      dr2r3 %lf %lf %lf\n",dr2r3.x, dr2r3.y, dr2r3.z); \
	*/ \
 \
	len = sqrt(VECSQ(dr1r2)); VECSCALE(dr1r2,1/len); \
	len = sqrt(VECSQ(dr2r3)); VECSCALE(dr2r3,1/len); \
 \
	/* printf ("scaled dr1r2 %lf %lf %lf\n",dr1r2.x, dr1r2.y, dr1r2.z); \
	printf ("scaled dr2r3 %lf %lf %lf\n",dr2r3.x, dr2r3.y, dr2r3.z); \
	*/ \
 \
	/* \
	printf("PROX_INT: (%f %f %f) (%f %f %f) (%f %f %f)\n (%f %f %f) (%f %f %f)\n", \
		t_orig.x, t_orig.y, t_orig.z, \
		t_zvec.x, t_zvec.y, t_zvec.z, \
		t_yvec.x, t_yvec.y, t_yvec.z, \
		dr1r2.x, dr1r2.y, dr1r2.z, \
		dr2r3.x, dr2r3.y, dr2r3.z \
		); \
	*/ \
 \
	if(fabs(VECPT(dr1r2, dr2r3)) > 0.001) { \
		printf ("Sorry, can't handle unevenly scaled ProximitySensors yet :(" \
		  "dp: %f v: (%f %f %f) (%f %f %f)\n", VECPT(dr1r2, dr2r3), \
		  	dr1r2.x,dr1r2.y,dr1r2.z, \
		  	dr2r3.x,dr2r3.y,dr2r3.z \
			); \
		return; \
	} \
 \
 \
	if(APPROX(dr1r2.z,1.0)) { \
		/* rotation */ \
		((node->__t2).r[0]) = 0; \
		((node->__t2).r[1]) = 0; \
		((node->__t2).r[2]) = 1; \
		((node->__t2).r[3]) = atan2(-dr2r3.x,dr2r3.y); \
	} else if(APPROX(dr2r3.y,1.0)) { \
		/* rotation */ \
		((node->__t2).r[0]) = 0; \
		((node->__t2).r[1]) = 1; \
		((node->__t2).r[2]) = 0; \
		((node->__t2).r[3]) = atan2(dr1r2.x,dr1r2.z); \
	} else { \
		/* Get the normal vectors of the possible rotation planes */ \
		nor1 = dr1r2; \
		nor1.z -= 1.0; \
		nor2 = dr2r3; \
		nor2.y -= 1.0; \
 \
		/* Now, the intersection of the planes, obviously cp */ \
		VECCP(nor1,nor2,ins); \
 \
		len = sqrt(VECSQ(ins)); VECSCALE(ins,1/len); \
 \
		/* the angle */ \
		VECCP(dr1r2,ins, nor1); \
		VECCP(zpvec, ins, nor2); \
		len = sqrt(VECSQ(nor1)); VECSCALE(nor1,1/len); \
		len = sqrt(VECSQ(nor2)); VECSCALE(nor2,1/len); \
		VECCP(nor1,nor2,ins); \
 \
		((node->__t2).r[3]) = -atan2(sqrt(VECSQ(ins)), VECPT(nor1,nor2)); \
 \
		/* rotation  - should normalize sometime... */ \
		((node->__t2).r[0]) = ins.x; \
		((node->__t2).r[1]) = ins.y; \
		((node->__t2).r[2]) = ins.z; \
	} \
	/* \
	printf("NORS: (%f %f %f) (%f %f %f) (%f %f %f)\n", \
		nor1.x, nor1.y, nor1.z, \
		nor2.x, nor2.y, nor2.z, \
		ins.x, ins.y, ins.z \
	); \
	*/ \
} 

/*
 * Copyright (C) 1998 Tuomas J. Lukka, 2002 John Stewart, Ayla Khan CRC Canada
 * DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 * See the GNU Library General Public License
 * (file COPYING in the distribution) for conditions of use and
 * redistribution, EXCEPT on the files which belong under the
 * Mozilla public license.
 *
 */

typedef struct _BrowserNative {
	/* int magic; does this really do anything ??? */
	/* and, this really does nothing SV *sv_js; */
	int dummyEntry;
} BrowserNative;

typedef struct _SFNodeNative {
	int valueChanged;
	uintptr_t *handle;
	char *X3DString;
	int fieldsExpanded;
} SFNodeNative;

typedef struct _SFRotationNative {
	int valueChanged;
	struct SFRotation v;
} SFRotationNative;

typedef struct _SFVec2fNative {
	int valueChanged;
	struct SFVec2f v;
} SFVec2fNative;

typedef struct _SFVec3fNative {
	int valueChanged;
	struct SFColor v;
} SFVec3fNative;

typedef struct _SFVec3dNative {
	int valueChanged;
	struct SFVec3d v;
} SFVec3dNative;

typedef struct _SFImageNative {
	int valueChanged;
} SFImageNative;

typedef struct _SFColorNative {
	int valueChanged;
	struct SFColor v;
} SFColorNative;

typedef struct _SFColorRGBANative {
	int valueChanged;
	struct SFColorRGBA v;
} SFColorRGBANative;


#include <jsapi.h>

/*
 * Adds additional (touchable) property to instance of a native
 * type.
 */
extern JSBool
addGlobalECMANativeProperty(void *cx,
							void *glob,
							char *name);

extern JSBool
addGlobalAssignProperty(void *cx,
						void *glob,
						char *name,
						char *str);

extern JSBool
addSFNodeProperty(void *cx,
				  void *glob,
				  char *nodeName,
				  char *name,
				  char *str);

extern void *
SFNodeNativeNew(void);

extern JSBool
SFNodeNativeAssign(void *top, void *fromp);

extern void *
SFRotationNativeNew(void);

extern void
SFRotationNativeAssign(void *top, void *fromp);

extern void
SFRotationNativeSet(void *p, struct Uni_String *sv);

extern void *
SFVec3fNativeNew(void);

extern void
SFVec3fNativeAssign(void *top, void *fromp);

extern void
SFVec3fNativeSet(void *p, struct Uni_String *sv);

extern void *
SFVec2fNativeNew(void);

extern void
SFVec2fNativeAssign(void *top, void *fromp);

extern void
SFVec2fNativeSet(void *p, struct Uni_String *sv);

extern void *
SFImageNativeNew(void);

extern void
SFImageNativeAssign(void *top, void *fromp);

extern void
SFImageNativeSet(void *p, struct Uni_String *sv);

extern void *
SFColorNativeNew(void);

extern void
SFColorNativeAssign(void *top, void *fromp);

extern void
SFColorNativeSet(void *p, struct Uni_String *sv);

/*
 * Copyright (C) 1998 Tuomas J. Lukka, 2002 John Stewart, Ayla Khan CRC Canada
 * DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 * See the GNU Library General Public License
 * (file COPYING in the distribution) for conditions of use and
 * redistribution, EXCEPT on the files which belong under the
 * Mozilla public license.
 *
 */

#define CLEANUP_JAVASCRIPT(cx) \
	/* printf ("calling JS_GC at %s:%d cx %u thread %u\n",__FILE__,__LINE__,cx,pthread_self()); */ \
	JS_GC(cx);

#define LARGESTRING 2048
#define STRING 512
#define SMALLSTRING 128

#define FNAME_STUB "file"
#define LINENO_STUB 0

/* for keeping track of the ECMA values */
struct ECMAValueStruct {
	jsval	JS_address;
	JSContext *context;
	int	valueChanged;
	char 	*name;
};


extern struct ECMAValueStruct ECMAValues[];
extern int maxECMAVal;
int findInECMATable(JSContext *context, jsval toFind);
int findNameInECMATable(JSContext *context, char *toFind);
void resetNameInECMATable(JSContext *context, char *toFind);

/* We keep around the results of script routing, or just script running... */
extern jsval JSCreate_global_return_val;
extern jsval JSglobal_return_val;
extern uintptr_t *JSSFpointer;

int jsrrunScript(JSContext *_context, JSObject *_globalObj, char *script, jsval *rval);
int JS_DefineSFNodeSpecificProperties (JSContext *context, JSObject *object, struct X3D_Node * ptr);

#ifdef JAVASCRIPTVERBOSE
#define ACTUALRUNSCRIPT(a,b,c) ActualrunScript(a,b,c,__FILE__,__LINE__)
int ActualrunScript(uintptr_t num, char *script, jsval *rval, char *fn, int line);
#else
#define ACTUALRUNSCRIPT(a,b,c) ActualrunScript(a,b,c)
int ActualrunScript(uintptr_t num, char *script, jsval *rval);
#endif

int
JSrunScript(uintptr_t num,
			char *script,
			struct Uni_String *rstr,
			struct Uni_String *rnum);

int
JSaddGlobalAssignProperty(uintptr_t num,
						  char *name,
						  char *str);

int
JSaddSFNodeProperty(uintptr_t num,
					char *nodeName,
					char *name,
					char *str);

int
JSaddGlobalECMANativeProperty(uintptr_t num,
							  char *name);

void
reportWarningsOn(void);

void
reportWarningsOff(void);

void
errorReporter(JSContext *cx,
			  const char *message,
			  JSErrorReport *report);

int JSGetProperty(uintptr_t num, char *script, struct Uni_String *rstr);
void JSInit(uintptr_t num);

void X3D_ECMA_TO_JS(JSContext *cx, void *Data, unsigned datalen, int dataType, jsval *ret);
JSBool setSFNodeField (JSContext *context, JSObject *obj, jsval id, jsval *vp);

/*
 * Copyright (C) 1998 Tuomas J. Lukka, 2002 John Stewart, Ayla Khan CRC Canada
 * DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 * See the GNU Library General Public License
 * (file COPYING in the distribution) for conditions of use and
 * redistribution, EXCEPT on the files which belong under the
 * Mozilla public license.
 *
 */


#ifndef UNUSED
#define UNUSED(v) ((void) v)
#endif

extern char *BrowserName; /* defined in VRMLC.pm */
extern double BrowserFPS;				/* defined in VRMLC.pm */

#define BROWMAGIC 12345

JSBool VrmlBrowserInit(JSContext *context, JSObject *globalObj,	BrowserNative *brow );


JSBool
VrmlBrowserGetName(JSContext *cx,
				   JSObject *obj,
				   uintN argc,
				   jsval *argv,
				   jsval *rval);


JSBool
VrmlBrowserGetVersion(JSContext *cx,
					  JSObject *obj,
					  uintN argc,
					  jsval *argv,
					  jsval *rval);


JSBool
VrmlBrowserGetCurrentSpeed(JSContext *cx,
					   JSObject *obj,
					   uintN argc,
					   jsval *argv,
					   jsval *rval);


JSBool
VrmlBrowserGetCurrentFrameRate(JSContext *cx,
						   JSObject *obj,
						   uintN argc,
						   jsval *argv,
						   jsval *rval);


JSBool
VrmlBrowserGetWorldURL(JSContext *cx,
					   JSObject *obj,
					   uintN argc,
					   jsval *argv,
					   jsval *rval);


JSBool
VrmlBrowserReplaceWorld(JSContext *cx,
					JSObject *obj,
					uintN argc,
					jsval *argv,
					jsval *rval);


JSBool
VrmlBrowserLoadURL(JSContext *cx,
				   JSObject *obj,
				   uintN argc,
				   jsval *argv,
				   jsval *rval);


JSBool
VrmlBrowserSetDescription(JSContext *cx,
						  JSObject *obj,
						  uintN argc,
						  jsval *argv,
						  jsval *rval);


JSBool
VrmlBrowserCreateVrmlFromString(JSContext *cx,
						  JSObject *obj,
								uintN argc,
								jsval *argv,
								jsval *rval);


JSBool
VrmlBrowserCreateVrmlFromURL(JSContext *cx,
							 JSObject *obj,
							 uintN argc,
							 jsval *argv,
							 jsval *rval);


JSBool
VrmlBrowserAddRoute(JSContext *cx,
					JSObject *obj,
					uintN argc,
					jsval *argv,
					jsval *rval);


JSBool
VrmlBrowserPrint(JSContext *cx,
					   JSObject *obj,
					   uintN argc,
					   jsval *argv,
					   jsval *rval);

JSBool
VrmlBrowserDeleteRoute(JSContext *cx,
					   JSObject *obj,
					   uintN argc,
					   jsval *argv,
					   jsval *rval);

JSBool
VrmlBrowserGetMidiDeviceList(JSContext *cx,
					   JSObject *obj,
					   uintN argc,
					   jsval *argv,
					   jsval *rval);

JSBool
VrmlBrowserGetMidiDeviceInfo(JSContext *cx,
					   JSObject *obj,
					   uintN argc,
					   jsval *argv,
					   jsval *rval);



static JSClass Browser = {
	"Browser",
	JSCLASS_HAS_PRIVATE,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_EnumerateStub,
	JS_ResolveStub,
	JS_ConvertStub,
	JS_FinalizeStub
};

/*
 * Copyright (C) 1998 Tuomas J. Lukka, 2002, 2007  John Stewart, Ayla Khan CRC Canada
 * DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 * See the GNU Library General Public License
 * (file COPYING in the distribution) for conditions of use and
 * redistribution, EXCEPT on the files which belong under the
 * Mozilla public license.
 *
 * Complex VRML nodes as Javascript classes.
 *
 */

#ifndef UNUSED
#define UNUSED(v) ((void) v)
#endif

#define INIT_ARGC_NODE 1
#define INIT_ARGC 0

/* quick fix to get around some compiler warnings on 64 bit systems */
#define VERBOSE_OBJX (unsigned long)
#define VERBOSE_OBJ 

/* tie a node into the root. Currently not required, as we do a better job
of garbage collection */
#define ADD_ROOT(a,b) \
	/* printf ("adding root  cx %u pointer %u value %u\n",a,&b,b); \
        if (JS_AddRoot(a,&b) != JS_TRUE) { \
                printf ("JA_AddRoot failed at %s:%d\n",__FILE__,__LINE__); \
                return JS_FALSE; \
        } */

#define REMOVE_ROOT(a,b) \
	/* printf ("removing root %u\n",b); \
        JS_RemoveRoot(a,&b);  */


#define DEFINE_LENGTH(thislength,thisobject) \
	{jsval zimbo = INT_TO_JSVAL(thislength);\
	/* printf ("defining length to %d for %d %d\n",thislength,cx,obj);*/ \
	if (!JS_DefineProperty(cx, thisobject, "length", zimbo, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) { \
		printf( "JS_DefineProperty failed for \"length\" at %s:%d.\n",__FILE__,__LINE__); \
		printf ("myThread is %u\n",pthread_self()); \
		return JS_FALSE;\
	}}

#define DEFINE_MF_ECMA_HAS_CHANGED \
	{jsval zimbo = INT_TO_JSVAL(0); \
	/* printf ("defining property for MF_ECMA_HAS_CHANGED... %d %d ",cx,obj); */ \
	if (!JS_DefineProperty(cx, obj, "MF_ECMA_has_changed", zimbo, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) { \
		printf( "JS_DefineProperty failed for \"MF_ECMA_has_changed\" at %s:%d.\n",__FILE__,__LINE__); \
		printf ("myThread is %u\n",pthread_self()); \
		return JS_FALSE; \
	}}

#define SET_MF_ECMA_HAS_CHANGED { jsval myv; \
                        myv = INT_TO_JSVAL(1); \
			/* printf ("setting property for MF_ECMA_has_changed %d %d\n",cx,obj); */ \
                        if (!JS_SetProperty(cx, obj, "MF_ECMA_has_changed", &myv)) { \
                                printf( "JS_SetProperty failed for \"MF_ECMA_has_changed\" in doMFSetProperty.\n"); \
		printf ("myThread is %u\n",pthread_self()); \
                                return JS_FALSE; \
                        }}


#define SET_JS_TICKTIME { jsval zimbo; \
        zimbo = DOUBLE_TO_JSVAL(JS_NewDouble(cx, TickTime));  \
        if (!JS_DefineProperty(cx,obj, "__eventInTickTime", zimbo, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) {  \
                printf( "JS_DefineProperty failed for \"__eventInTickTime\" at %s:%d.\n",__FILE__,__LINE__); \
		printf ("myThread is %u\n",pthread_self()); \
                return; \
        }}

#define COMPILE_FUNCTION_IF_NEEDED(tnfield) \
	if (JSparamnames[tnfield].eventInFunction == 0) { \
		sprintf (scriptline,"%s(__eventIn_Value_%s,__eventInTickTime)", JSparamnames[tnfield].name,JSparamnames[tnfield].name); \
		/* printf ("compiling function %s\n",scriptline); */ \
		JSparamnames[tnfield].eventInFunction = (uintptr_t) JS_CompileScript( \
			cx, obj, scriptline, strlen(scriptline), "compile eventIn",1); \
	}
#define RUN_FUNCTION(tnfield) \
	{jsval zimbo; \
	if (!JS_ExecuteScript(cx, obj, (JSScript *) JSparamnames[tnfield].eventInFunction, &zimbo)) { \
		printf ("failed to set parameter for eventIn %s in FreeWRL code %s:%d\n",JSparamnames[tnfield].name,__FILE__,__LINE__); \
		printf ("myThread is %u\n",pthread_self()); \
	}} 


#define SET_LENGTH(cx,newMFObject,length) \
	{ jsval lenval; \
                lenval = INT_TO_JSVAL(length); \
                if (!JS_SetProperty(cx, newMFObject, "length", &lenval)) { \
                        printf( "JS_SetProperty failed for \"length\" at %s:%d\n",__FILE__,__LINE__); \
		printf ("myThread is %u\n",pthread_self()); \
                        return JS_FALSE; \
                }} 

#define SET_EVENTIN_VALUE(cx,obj,nameIndex,newObj) \
	{ char scriptline[100]; \
		sprintf (scriptline,"__eventIn_Value_%s", JSparamnames[nameIndex].name); \
        	if (!JS_DefineProperty(cx,obj, scriptline, OBJECT_TO_JSVAL(newObj), JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) {  \
        	        printf( "JS_DefineProperty failed for \"ECMA in\" at %s:%d.\n",__FILE__,__LINE__);  \
		printf ("myThread is %u\n",pthread_self()); \
        	        return JS_FALSE; \
        }	}
	

/*
 * The following VRML field types don't need JS classes:
 * (ECMAScript native datatypes, see JS.pm):
 *
 * * SFBool
 * * SFFloat
 * * SFInt32
 * * SFString
 * * SFTime
 *
 * VRML field types that are implemented here as Javascript classes
 * are:
 *
 * * SFColor, MFColor
 * * MFFloat
 * * SFImage -- not supported currently
 * * MFInt32
 * * SFNode (special case - must be supported perl (see JS.pm), MFNode
 * * SFRotation, MFRotation
 * * MFString
 * * MFTime
 * * SFVec2f, MFVec2f
 * * SFVec3f, MFVec3f
 * * SFVec3d
 *
 * These (single value) fields have struct types defined elsewhere
 * (see Structs.h) that are stored by Javascript classes as private data.
 *
 * Some of the computations for SFVec3f, SFRotation are now defined
 * elsewhere (see LinearAlgebra.h) to avoid duplication.
 */


/* helper functions */
void JS_MY_Finalize(JSContext *cx, JSObject *obj);

JSBool doMFToString(JSContext *cx, JSObject *obj, const char *className, jsval *rval); 
JSBool doMFAddProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp, char *name); 
JSBool doMFSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp, int type); 
JSBool getBrowser(JSContext *context, JSObject *obj, BrowserNative **brow); 
JSBool doMFStringUnquote(JSContext *cx, jsval *vp);


/* class functions */

JSBool
globalResolve(JSContext *cx,
			  JSObject *obj,
			  jsval id);

JSBool
loadVrmlClasses(JSContext *context,
				JSObject *globalObj);


JSBool
setECMANative(JSContext *cx,
			  JSObject *obj,
			  jsval id,
			  jsval *vp);


JSBool
getAssignProperty(JSContext *context,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);

JSBool
setAssignProperty(JSContext *context,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);



JSBool
SFColorGetHSV(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorSetHSV(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp); 

JSBool
SFColorSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

JSBool
SFColorRGBAGetHSV(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorRGBASetHSV(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorRGBAToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorRGBAAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorRGBAConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorRGBAGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp); 

JSBool
SFColorRGBASetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

JSBool
SFImageToString(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

JSBool
SFImageAssign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);


JSBool
SFImageConstr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
SFImageGetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
SFImageSetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);



JSBool
SFNodeToString(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
SFNodeAssign(JSContext *cx, JSObject *obj,
			 uintN argc,
			 jsval *argv,
			 jsval *rval);

JSBool
SFNodeConstr(JSContext *cx,
			 JSObject *obj,
			 uintN argc,
			 jsval *argv,
			 jsval *rval);

void SFNodeFinalize(JSContext *cx, JSObject *obj);

JSBool
SFNodeGetProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);

JSBool
SFNodeSetProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);



JSBool
SFRotationGetAxis(JSContext *cx,
				  JSObject *obj,
				  uintN argc,
				  jsval *argv,
				  jsval *rval);

/* not implemented */
JSBool
SFRotationInverse(JSContext *cx,
				  JSObject *obj,
				  uintN argc,
				  jsval *argv,
				  jsval *rval);

JSBool
SFRotationMultiply(JSContext *cx,
				   JSObject *obj,
				   uintN argc,
				   jsval *argv,
				   jsval *rval);
JSBool
SFRotationMultVec(JSContext *cx,
				  JSObject *obj,
				  uintN argc,
				  jsval *argv,
				  jsval *rval);

JSBool
SFRotationSetAxis(JSContext *cx,
				  JSObject *obj,
				  uintN argc,
				  jsval *argv,
				  jsval *rval);

JSBool
SFRotationSlerp(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

JSBool
SFRotationToString(JSContext *cx,
				   JSObject *obj,
				   uintN argc,
				   jsval *argv,
				   jsval *rval);

JSBool
SFRotationAssign(JSContext *cx,
				 JSObject *obj,
				 uintN argc,
				 jsval *argv,
				 jsval *rval);

JSBool
SFRotationConstr(JSContext *cx,
				 JSObject *obj,
				 uintN argc,
				 jsval *argv,
				 jsval *rval);

JSBool
SFRotationGetProperty(JSContext *cx,
					  JSObject *obj,
					  jsval id,
					  jsval *vp);

JSBool
SFRotationSetProperty(JSContext *cx,
					  JSObject *obj,
					  jsval id,
					  jsval *vp);



JSBool
SFVec2fAdd(JSContext *cx,
		   JSObject *obj,
		   uintN argc,
		   jsval *argv,
		   jsval *rval);

JSBool
SFVec2fDivide(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
SFVec2fDot(JSContext *cx,
		   JSObject *obj,
		   uintN argc,
		   jsval *argv,
		   jsval *rval);

JSBool
SFVec2fLength(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
SFVec2fMultiply(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

/* JSBool
SFVec2fNegate(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);
*/

JSBool
SFVec2fNormalize(JSContext *cx,
				 JSObject *obj,
				 uintN argc,
				 jsval *argv,
				 jsval *rval);

JSBool
SFVec2fSubtract(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

JSBool
SFVec2fToString(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

JSBool
SFVec2fAssign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
SFVec2fConstr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
SFVec2fGetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
SFVec2fSetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);



JSBool SFVec3fAdd(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fCross(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fDivide(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fDot(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fLength(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fMultiply(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fNegate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fNormalize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fSubtract(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp); 
JSBool SFVec3fSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);


JSBool SFVec3dAdd(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dCross(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dDivide(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dDot(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dLength(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dMultiply(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dNegate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dNormalize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dSubtract(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp); 
JSBool SFVec3dSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);



JSBool
MFColorToString(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
MFColorAssign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFColorConstr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFColorAddProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFColorGetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFColorSetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);



JSBool
MFFloatToString(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
MFFloatAssign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFFloatConstr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFFloatAddProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFFloatGetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFFloatSetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);



JSBool
MFInt32ToString(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
MFInt32Assign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFInt32Constr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFInt32AddProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFInt32GetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFInt32SetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);


JSBool
MFNodeToString(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
MFNodeAssign(JSContext *cx,
			 JSObject *obj,
			 uintN argc,
			 jsval *argv,
			 jsval *rval);

JSBool
MFNodeConstr(JSContext *cx,
			 JSObject *obj,
			 uintN argc,
			 jsval *argv,
			 jsval *rval);

JSBool
MFNodeAddProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);

JSBool
MFNodeGetProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);

JSBool
MFNodeSetProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);



JSBool
MFRotationToString(JSContext *cx,
				   JSObject *obj,
				   uintN argc,
				   jsval *argv,
				   jsval *rval);

JSBool
MFRotationAssign(JSContext *cx,
				 JSObject *obj,
				 uintN argc,
				 jsval *argv,
				 jsval *rval);

JSBool
MFRotationConstr(JSContext *cx,
				 JSObject *obj,
				 uintN argc,
				 jsval *argv,
				 jsval *rval);

JSBool
MFRotationGetProperty(JSContext *cx,
					  JSObject *obj,
					  jsval id,
					  jsval *vp);

JSBool
MFRotationSetProperty(JSContext *cx,
					  JSObject *obj,
					  jsval id,
					  jsval *vp);

JSBool
MFRotationAddProperty(JSContext *cx,
					  JSObject *obj,
					  jsval id,
					  jsval *vp);



JSBool
MFStringToString(JSContext *cx,
				 JSObject *obj,
				 uintN argc,
				 jsval *argv,
				 jsval *rval);

JSBool
MFStringAssign(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
MFStringConstr(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
MFStringGetProperty(JSContext *cx,
					JSObject *obj,
					jsval id,
					jsval *vp);

JSBool
MFStringSetProperty(JSContext *cx,
					JSObject *obj,
					jsval id,
					jsval *vp);


JSBool
MFStringAddProperty(JSContext *cx,
					JSObject *obj,
					jsval id,
					jsval *vp);


JSBool
MFTimeToString(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
MFTimeAssign(JSContext *cx,
			 JSObject *obj,
			 uintN argc,
			 jsval *argv,
			 jsval *rval);

JSBool
MFTimeConstr(JSContext *cx,
			 JSObject *obj,
			 uintN argc,
			 jsval *argv,
			 jsval *rval);

JSBool
MFTimeAddProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);

JSBool
MFTimeGetProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);

JSBool
MFTimeSetProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);



JSBool
MFVec2fToString(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

JSBool
MFVec2fAssign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFVec2fConstr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFVec2fAddProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFVec2fGetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFVec2fSetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);



JSBool
MFVec3fToString(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

JSBool
MFVec3fAssign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFVec3fConstr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFVec3fAddProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFVec3fGetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFVec3fSetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
VrmlMatrixToString(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

JSBool
VrmlMatrixAssign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);


JSBool VrmlMatrixsetTransform(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool VrmlMatrixgetTransform(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool VrmlMatrixinverse(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool VrmlMatrixtranspose(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool VrmlMatrixmultLeft(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool VrmlMatrixmultRight(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool VrmlMatrixmultVecMatrix(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool VrmlMatrixmultMatrixVec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
VrmlMatrixConstr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
VrmlMatrixAddProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
VrmlMatrixGetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
VrmlMatrixSetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);


extern JSClass SFColorClass;
extern JSPropertySpec (SFColorProperties)[];
extern JSFunctionSpec (SFColorFunctions)[];
extern JSClass SFColorRGBAClass;
extern JSPropertySpec (SFColorRGBAProperties)[];
extern JSFunctionSpec (SFColorRGBAFunctions)[];
extern JSClass SFImageClass;
extern JSPropertySpec (SFImageProperties)[];
extern JSFunctionSpec (SFImageFunctions)[];
extern JSClass SFNodeClass;
extern JSPropertySpec (SFNodeProperties)[];
extern JSFunctionSpec (SFNodeFunctions)[];
extern JSClass SFRotationClass;
extern JSPropertySpec (SFRotationProperties)[];
extern JSFunctionSpec (SFRotationFunctions)[];
extern JSClass SFVec2fClass;
extern JSPropertySpec (SFVec2fProperties)[];
extern JSFunctionSpec (SFVec2fFunctions)[];
extern JSClass SFVec3fClass;
extern JSPropertySpec (SFVec3fProperties)[];
extern JSFunctionSpec (SFVec3fFunctions)[];
extern JSClass SFVec3dClass;
extern JSPropertySpec (SFVec3dProperties)[];
extern JSFunctionSpec (SFVec3dFunctions)[];
extern JSClass MFColorClass;
extern JSFunctionSpec (MFColorFunctions)[];
extern JSClass MFFloatClass;
extern JSFunctionSpec (MFFloatFunctions)[];
extern JSClass MFInt32Class;
extern JSFunctionSpec (MFInt32Functions)[];
extern JSClass MFNodeClass;
extern JSFunctionSpec (MFNodeFunctions)[];
extern JSClass MFRotationClass;
extern JSFunctionSpec (MFRotationFunctions)[];
extern JSClass MFStringClass;
extern JSFunctionSpec (MFStringFunctions)[];
extern JSClass MFTimeClass;
extern JSPropertySpec (MFTimeProperties)[] ;
extern JSFunctionSpec (MFTimeFunctions)[];
extern JSClass MFVec2fClass;
extern JSFunctionSpec (MFVec2fFunctions)[];
extern JSClass MFVec3fClass;
extern JSFunctionSpec (MFVec3fFunctions)[];
extern JSClass VrmlMatrixClass;
extern JSFunctionSpec (VrmlMatrixFunctions)[];

#ifdef DEBUG_JAVASCRIPT_PROPERTY
JSBool js_GetPropertyDebug (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug1 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug2 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug3 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug4 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug5 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug6 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug7 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug8 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug9 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
#endif

/*
 * Copyright (C) 2002 Nicolas Coderre CRC Canada
 * Portions Copyright (C) 1998 Tuomas J. Lukka 1998 Bernhard Reiter 1999 John Stewart CRC Canada
 * DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 * See the GNU Library General Public License (file COPYING in the distribution)
 * for conditions of use and redistribution.
 */

/*Fast macros */

#define VECSQ(a) VECPT(a,a)
#define VECPT(a,b) ((a).x*(b).x + (a).y*(b).y + (a).z*(b).z)
#define VECDIFF(a,b,c) {(c).x = (a).x-(b).x;(c).y = (a).y-(b).y;(c).z = (a).z-(b).z;}
#define VECADD(a,b) {(a).x += (b).x; (a).y += (b).y; (a).z += (b).z;}
#define VEC_FROM_CDIFF(a,b,r) {(r).x = (a).c[0]-(b).c[0];(r).y = (a).c[1]-(b).c[1];(r).z = (a).c[2]-(b).c[2];}
#define VECCP(a,b,c) {(c).x = (a).y*(b).z-(b).y*(a).z; (c).y = -((a).x*(b).z-(b).x*(a).z); (c).z = (a).x*(b).y-(b).x*(a).y;}
#define VECSCALE(a,c) {(a).x *= c; (a).y *= c; (a).z *= c;}

/*special case ; used in Extrusion.GenPolyRep and ElevationGrid.GenPolyRep:
 *	Calc diff vec from 2 coordvecs which must be in the same field 	*/
#define VEC_FROM_COORDDIFF(f,a,g,b,v) {\
	(v).x= (f)[(a)*3]-(g)[(b)*3];	\
	(v).y= (f)[(a)*3+1]-(g)[(b)*3+1];	\
	(v).z= (f)[(a)*3+2]-(g)[(b)*3+2];	\
}

/* rotate a vector along one axis				*/
#define VECROTATE_X(c,angle) { \
	/*(c).x =  (c).x	*/ \
	  (c).y = 		  cos(angle) * (c).y 	- sin(angle) * (c).z; \
	  (c).z = 		  sin(angle) * (c).y 	+ cos(angle) * (c).z; \
	}
#define VECROTATE_Y(c,angle) { \
	  (c).x = cos(angle)*(c).x +			+ sin(angle) * (c).z; \
	/*(c).y = 				(c).y 	*/ \
	  (c).z = -sin(angle)*(c).x 			+ cos(angle) * (c).z; \
	}
#define VECROTATE_Z(c,angle) { \
	  (c).x = cos(angle)*(c).x - sin(angle) * (c).y;	\
	  (c).y = sin(angle)*(c).x + cos(angle) * (c).y; 	\
	/*(c).z = s						 (c).z; */ \
	}

#define MATRIX_ROTATION_X(angle,m) {\
	m[0][0]=1; m[0][1]=0; m[0][2]=0; \
	m[1][0]=0; m[1][1]=cos(angle); m[1][2]=- sin(angle); \
	m[2][0]=0; m[2][1]=sin(angle); m[2][2]=cos(angle); \
}
#define MATRIX_ROTATION_Y(angle,m) {\
	m[0][0]=cos(angle); m[0][1]=0; m[0][2]=sin(angle); \
	m[1][0]=0; m[1][1]=1; m[1][2]=0; \
	m[2][0]=-sin(angle); m[2][1]=0; m[2][2]=cos(angle); \
}
#define MATRIX_ROTATION_Z(angle,m) {\
	m[0][0]=cos(angle); m[0][1]=- sin(angle); m[0][2]=0; \
	m[1][0]=sin(angle); m[1][1]=cos(angle); m[1][2]=0; \
	m[2][0]=0; m[2][1]=0; m[2][2]=1; \
}

/* next matrix calculation comes from comp.graphics.algorithms FAQ	*/
/* the axis vector has to be normalized					*/
#define MATRIX_FROM_ROTATION(ro,m) { \
	struct { double x,y,z,w ; } __q; \
        double sinHalfTheta = sin(0.5*(ro.r[3]));\
        double xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;\
        __q.x = (ro.r[0])*sinHalfTheta;\
        __q.y = (ro.r[1])*sinHalfTheta;\
        __q.z = (ro.r[2])*sinHalfTheta;\
        __q.w = cos(0.5*(ro.r[3]));\
        xs = 2*__q.x;  ys = 2*__q.y;  zs = 2*__q.z;\
        wx = __q.w*xs; wy = __q.w*ys; wz = __q.w*zs;\
        xx = __q.x*xs; xy = __q.x*ys; xz = __q.x*zs;\
        yy = __q.y*ys; yz = __q.y*zs; zz = __q.z*zs;\
        m[0][0] = 1 - (yy + zz); m[0][1] = xy - wz;      m[0][2] = xz + wy;\
        m[1][0] = xy + wz;       m[1][1] = 1 - (xx + zz);m[1][2] = yz - wx;\
        m[2][0] = xz - wy;       m[2][1] = yz + wx;      m[2][2] = 1-(xx + yy);\
}

/* matrix multiplication */
#define VECMM(m,c) { \
	double ___x=(c).x,___y=(c).y,___z=(c).z; \
	(c).x= m[0][0]*___x + m[0][1]*___y + m[0][2]*___z; \
	(c).y= m[1][0]*___x + m[1][1]*___y + m[1][2]*___z; \
	(c).z= m[2][0]*___x + m[2][1]*___y + m[2][2]*___z; \
}


/* next define rotates vector c with rotation vector r and angle */
/*  after section 5.8 of the VRML`97 spec			 */

#define VECROTATE(rx,ry,rz,angle,nc) { \
	double ___x=(nc).x,___y=(nc).y,___z=(nc).z; \
	double ___c=cos(angle),  ___s=sin(angle), ___t=1-___c; \
	(nc).x=   (___t*((rx)*(rx))+___c)     *___x    \
	        + (___t*(rx)*(ry)  -___s*(rz))*___y    \
	        + (___t*(rx)*(rz)  +___s*(ry))*___z ;  \
	(nc).y=   (___t*(rx)*(ry)  +___s*(rz))*___x    \
	        + (___t*((ry)*(ry))+___c)     *___y    \
	        + (___t*(ry)*(rz)  -___s*(rx))*___z ;  \
	(nc).z=   (___t*(rx)*(rz)  -___s*(ry))*___x    \
	        + (___t*(ry)*(rz)  +___s*(rx))*___y    \
	        + (___t*((rz)*(rz))+___c)     *___z ;  \
	}


/*
#define VECROTATE(rx,ry,rz,angle,c) { \
	double ___c=cos(angle),  ___s=sin(angle), ___t=1-___c; \
	(c).x=   (___t*((rx)*(rx))+___c)     *(c).x    \
	       + (___t*(rx)*(ry)  +___s*(rz))*(c).y    \
	       + (___t*(rx)*(rz)  -___s*(ry))*(c).z ;  \
	(c).y=   (___t*(rx)*(ry)  -___s*(rz))*(c).x    \
	       + (___t*((ry)*(ry))+___c)     *(c).y    \
	       + (___t*(ry)*(rz)  +___s*(rx))*(c).z ;  \
	(c).z=   (___t*(rx)*(rz)  +___s*(ry))*(c).x    \
	       + (___t*(ry)*(rz)  -___s*(rx))*(c).y    \
	       + (___t*((rz)*(rz))+ ___c)    *(c).z ;  \
	}

*/
/* next define abbreviates VECROTATE with use of the SFRotation struct	*/
#define VECRROTATE(ro,c) VECROTATE((ro).r[0],(ro).r[1],(ro).r[2],(ro).r[3],c)


#define calc_vector_length(pt) veclength(pt)

float veclength( struct point_XYZ p );

/* returns vector length, too */
GLdouble vecnormal(struct point_XYZ*r, struct point_XYZ* v);

#define normalize_vector(pt) vecnormal(pt,pt)

float calc_angle_between_two_vectors(struct point_XYZ a, struct point_XYZ b);

double vecangle(struct point_XYZ* V1, struct point_XYZ* V2);


#define calc_vector_product(a,b,c) veccross(c,a,b);

void veccross(struct point_XYZ *c , struct point_XYZ a, struct point_XYZ b);


GLdouble det3x3(GLdouble* data);

struct point_XYZ* transform(struct point_XYZ* r, const struct point_XYZ* a, const GLdouble* b);
float* transformf(float* r, const float* a, const GLdouble* b);

/*only transforms using the rotation component.
  Usefull for transforming normals, and optimizing when you know there's no translation */
struct point_XYZ* transform3x3(struct point_XYZ* r, const struct point_XYZ* a, const GLdouble* b);

struct point_XYZ* vecscale(struct point_XYZ* r, struct point_XYZ* v, GLdouble s);

double vecdot(struct point_XYZ* a, struct point_XYZ* b);

#define calc_vector_scalar_product(a,b) vecdot(&(a),&(b))

double closest_point_of_segment_to_y_axis_segment(double y1, double y2, struct point_XYZ p1, struct point_XYZ p2);

struct point_XYZ* vecadd(struct point_XYZ* r, struct point_XYZ* v, struct point_XYZ* v2);

struct point_XYZ* vecdiff(struct point_XYZ* r, struct point_XYZ* v, struct point_XYZ* v2);

/*specify a direction "n", and you get two vectors i, and j, perpendicular to n and themselfs. */
void make_orthogonal_vector_space(struct point_XYZ* i, struct point_XYZ* j, struct point_XYZ n);

GLdouble* matinverse(GLdouble* res, GLdouble* m);

struct point_XYZ* polynormal(struct point_XYZ* r, struct point_XYZ* p1, struct point_XYZ* p2, struct point_XYZ* p3);
/*simple wrapper for now. optimize later */
struct point_XYZ* polynormalf(struct point_XYZ* r, float* p1, float* p2, float* p3);

GLdouble* matrotate(GLdouble* Result, double Theta, double x, double y, double z);

/*rotates dv back on iv*/
double matrotate2v(GLdouble* res, struct point_XYZ iv/*original*/, struct point_XYZ dv/*result*/);

GLdouble* mattranslate(GLdouble* r, double dx, double dy, double dz);

GLdouble* matmultiply(GLdouble* r, GLdouble* m , GLdouble* n);

/*
 * Copyright (c) 1995 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*
 * Portions of this software Copyright (c) 1995 Brown University.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement
 * is hereby granted, provided that the above copyright notice and the
 * following two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF BROWN
 * UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * BROWN UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
 * BASIS, AND BROWN UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */


/* Define Parsing error codes. */

#define SKIP_PICTURE (-10)
#define SKIP_TO_START_CODE (-1)
#define PARSE_OK 1

/* Set ring buffer size. */

#define RING_BUF_SIZE 5

/* Macros for picture code type. */

#define I_TYPE 1
#define P_TYPE 2
#define B_TYPE 3
#define D_TYPE 4

/* Start codes. */

#define SEQ_END_CODE 0x000001b7
#define SEQ_START_CODE 0x000001b3
#define GOP_START_CODE 0x000001b8
#define PICTURE_START_CODE 0x00000100
#define SLICE_MIN_START_CODE 0x00000101
#define SLICE_MAX_START_CODE 0x000001af
#define EXT_START_CODE 0x000001b5
#define USER_START_CODE 0x000001b2
#define SEQUENCE_ERROR_CODE 0x000001b4

/* Number of macroblocks to process in one call to mpeg_VidRsrc. */

#define MB_QUANTUM 100

/* Macros used with macroblock address decoding. */

#define MB_STUFFING 34
#define MB_ESCAPE 35

/* Lock flags for pict images. */

#define DISPLAY_LOCK 0x01
#define PAST_LOCK 0x02
#define FUTURE_LOCK 0x04

#define MONO_THRESHOLD 11

/* External declaration of row,col to zig zag conversion matrix. */

/* Brown - changed to const int because it is a help variable */
extern const int scan[][8];

/* Temporary definition of time stamp structure. */

typedef int TimeStamp;

/* Structure with reconstructed pixel values. */

typedef struct pict_image {
  unsigned char *luminance;              /* Luminance plane.   */
  unsigned char *Cr;                     /* Cr plane.          */
  unsigned char *Cb;                     /* Cb plane.          */
  unsigned char *display;                /* Display plane.     */
  int locked;                            /* Lock flag.         */
  TimeStamp show_time;                   /* Presentation time. */
} PictImage;

/* Group of pictures structure. */

typedef struct GoP {
  int drop_flag;                     /* Flag indicating dropped frame. */
  unsigned int tc_hours;                 /* Hour component of time code.   */
  unsigned int tc_minutes;               /* Minute component of time code. */
  unsigned int tc_seconds;               /* Second component of time code. */
  unsigned int tc_pictures;              /* Picture counter of time code.  */
  int closed_gop;                    /* Indicates no pred. vectors to
					    previous group of pictures.    */
  int broken_link;                   /* B frame unable to be decoded.  */
  char *ext_data;                        /* Extension data.                */
  char *user_data;                       /* User data.                     */
} GoP;

/* Picture structure. */

typedef struct pict {
  unsigned int temp_ref;                 /* Temporal reference.             */
  unsigned int code_type;                /* Frame type: P, B, I             */
  unsigned int vbv_delay;                /* Buffer delay.                   */
  int full_pel_forw_vector;          /* Forw. vectors specified in full
					    pixel values flag.              */
  unsigned int forw_r_size;              /* Used for vector decoding.       */
  unsigned int forw_f;                   /* Used for vector decoding.       */
  int full_pel_back_vector;          /* Back vectors specified in full
					    pixel values flag.              */
  unsigned int back_r_size;              /* Used in decoding.               */
  unsigned int back_f;                   /* Used in decoding.               */
  char *extra_info;                      /* Extra bit picture info.         */
  char *ext_data;                        /* Extension data.                 */
  char *user_data;                       /* User data.                      */
} Pict;

/* Slice structure. */

typedef struct slice {
  unsigned int vert_pos;                 /* Vertical position of slice. */
  unsigned int quant_scale;              /* Quantization scale.         */
  char *extra_info;                      /* Extra bit slice info.       */
} Slice;

/* Macroblock structure. */

typedef struct macroblock {
  int mb_address;                        /* Macroblock address.              */
  int past_mb_addr;                      /* Previous mblock address.         */
  int motion_h_forw_code;                /* Forw. horiz. motion vector code. */
  unsigned int motion_h_forw_r;          /* Used in decoding vectors.        */
  int motion_v_forw_code;                /* Forw. vert. motion vector code.  */
  unsigned int motion_v_forw_r;          /* Used in decdoinge vectors.       */
  int motion_h_back_code;                /* Back horiz. motion vector code.  */
  unsigned int motion_h_back_r;          /* Used in decoding vectors.        */
  int motion_v_back_code;                /* Back vert. motion vector code.   */
  unsigned int motion_v_back_r;          /* Used in decoding vectors.        */
  unsigned int cbp;                      /* Coded block pattern.             */
  int mb_intra;                      /* Intracoded mblock flag.          */
  int bpict_past_forw;               /* Past B frame forw. vector flag.  */
  int bpict_past_back;               /* Past B frame back vector flag.   */
  int past_intra_addr;                   /* Addr of last intracoded mblock.  */
  int recon_right_for_prev;              /* Past right forw. vector.         */
  int recon_down_for_prev;               /* Past down forw. vector.          */
  int recon_right_back_prev;             /* Past right back vector.          */
  int recon_down_back_prev;              /* Past down back vector.           */
} Macroblock;

/* Block structure. */

typedef struct block {
  short int dct_recon[8][8];             /* Reconstructed dct coeff matrix. */
  short int dct_dc_y_past;               /* Past lum. dc dct coefficient.   */
  short int dct_dc_cr_past;              /* Past cr dc dct coefficient.     */
  short int dct_dc_cb_past;              /* Past cb dc dct coefficient.     */
} Block;

/* Video stream structure. */

typedef struct vid_stream {
  unsigned int h_size;                         /* Horiz. size in pixels.     */
  unsigned int v_size;                         /* Vert. size in pixels.      */
  unsigned int mb_height;                      /* Vert. size in mblocks.     */
  unsigned int mb_width;                       /* Horiz. size in mblocks.    */
  unsigned char aspect_ratio;                  /* Code for aspect ratio.     */
  unsigned char picture_rate;                  /* Code for picture rate.     */
  unsigned int bit_rate;                       /* Bit rate.                  */
  unsigned int vbv_buffer_size;                /* Minimum buffer size.       */
  int const_param_flag;                    /* Contrained parameter flag. */
  unsigned char intra_quant_matrix[8][8];      /* Quantization matrix for
						  intracoded frames.         */
  unsigned char non_intra_quant_matrix[8][8];  /* Quanitization matrix for
						  non intracoded frames.     */
  char *ext_data;                              /* Extension data.            */
  char *user_data;                             /* User data.                 */
  GoP group;                                   /* Current group of pict.     */
  Pict picture;                                /* Current picture.           */
  Slice slice;                                 /* Current slice.             */
  Macroblock mblock;                           /* Current macroblock.        */
  Block block;                                 /* Current block.             */
  int state;                                   /* State of decoding.         */
  int bit_offset;                              /* Bit offset in stream.      */
  unsigned int *buffer;                        /* Pointer to next byte in
						  buffer.                    */
  int buf_length;                              /* Length of remaining buffer.*/
  unsigned int *buf_start;                     /* Pointer to buffer start.   */
/* Brown - beginning of added variables that used to be static or global */
  int max_buf_length;                          /* Max length of buffer.      */
  int film_has_ended;                          /* Boolean - film has ended   */
  int sys_layer;                               /* -1 uninitialized,
	                                           0 video layer,
						   1 syslayer                */
  unsigned int num_left;                       /* from ReadPacket - leftover */
  unsigned int leftover_bytes;                 /* from ReadPacket - leftover */
  int EOF_flag;                                /* stream is EOF              */
  FILE *input;                                 /* stream comes from here     */
  long seekValue;                              /* 0 no seeking
						  >0 do a seek,
						  <0 already has done seek   */
  int swap;                                /* from ReadFile              */
  int Parse_done;                          /* from read_sys              */
  int gAudioStreamID;
  int gVideoStreamID;
  int gReservedStreamID;
  int right_for,down_for;                      /* From ReconPMBlock, video.c */
  int right_half_for, down_half_for;
  unsigned int curBits;                        /* current bits               */
  int matched_depth;                           /* depth of displayed movie   */
  char *filename;                              /* Name of stream filename    */
  int ditherType;                              /* What type of dithering     */
  char *ditherFlags;                           /* flags for MB Ordered dither*/
  int totNumFrames;                            /* Total Number of Frames     */
  double realTimeStart;                        /* When did the movie start?  */
/* Brown - end of added variables */
  PictImage *past;                             /* Past predictive frame.     */
  PictImage *future;                           /* Future predictive frame.   */
  PictImage *current;                          /* Current frame.             */
  PictImage *ring[RING_BUF_SIZE];              /* Ring buffer of frames.     */
  /* x,y size of PPM output file */
  int ppm_width, ppm_height, ppm_modulus;
} mpeg_VidStream;


/* Declaration of global display pointer. */



/* Definition of Contant integer scale factor. */

#define CONST_BITS 13

/* Misc DCT definitions */
#define DCTSIZE		8	/* The basic DCT block is 8x8 samples */
#define DCTSIZE2	64	/* DCTSIZE squared; # of elements in a block */


typedef short DCTELEM;
typedef DCTELEM DCTBLOCK[DCTSIZE2];

#ifdef __STDC__
# define	P(s) s
#include <stdlib.h>	/* used by almost all modules */
#else
# define P(s) ()
#endif

/* util.c */
void correct_underflow P((mpeg_VidStream *vid_stream ));
int next_bits P((int num , unsigned int mask , mpeg_VidStream *vid_stream ));
char *get_ext_data P((mpeg_VidStream *vid_stream ));
int next_start_code P((mpeg_VidStream *vid_stream));
char *get_extra_bit_info P((mpeg_VidStream *vid_stream ));

/* video.c */
void init_stats P((void ));
void PrintAllStats P((mpeg_VidStream *vid_stream ));
double ReadSysClock P((void ));
void PrintTimeInfo P(( mpeg_VidStream *vid_stream ));
void InitCrop P((void ));
mpeg_VidStream *Newmpeg_VidStream P((unsigned int buffer_len ));
#ifndef NOCONTROLS
void Resetmpeg_VidStream P((mpeg_VidStream *vid ));
#endif
void Destroympeg_VidStream P((mpeg_VidStream *astream));
PictImage *NewPictImage P(( mpeg_VidStream *vid_stream ));
void DestroyPictImage P((PictImage *apictimage));
mpeg_VidStream *mpeg_VidRsrc P((TimeStamp time_stamp,mpeg_VidStream *vid_stream, int first ));
void SetBFlag P((int val ));
void SetPFlag P((int val ));

/* parseblock.c */
void ParseReconBlock P((int n, mpeg_VidStream *vid_stream ));
void ParseAwayBlock P((int n , mpeg_VidStream *vid_stream ));

/* motionvector.c */
void ComputeForwVector P((int *recon_right_for_ptr , int *recon_down_for_ptr , mpeg_VidStream *the_stream ));
void ComputeBackVector P((int *recon_right_back_ptr , int *recon_down_back_ptr, mpeg_VidStream *the_stream ));

/* decoders.c */
void mpeg_init_tables P((void ));
void decodeDCTDCSizeLum P((unsigned int *value ));
void decodeDCTDCSizeChrom P((unsigned int *value ));
void decodeDCTCoeffFirst P((unsigned int *run , int *level ));
void decodeDCTCoeffNext P((unsigned int *run , int *level ));

/* readfile.c */
void clear_data_stream P(( mpeg_VidStream *vid_stream));
int get_more_data P(( mpeg_VidStream *vid_stream ));
int pure_get_more_data P((unsigned int *buf_start , int max_length , int *length_ptr , unsigned int **buf_ptr, mpeg_VidStream *vid_stream ));
int read_sys P(( mpeg_VidStream *vid_stream, unsigned int start ));
int ReadStartCode P(( unsigned int *startCode, mpeg_VidStream *vid_stream ));

int ReadPackHeader P((
   double *systemClockTime,
   unsigned long *muxRate,
   mpeg_VidStream *vid_stream ));

int ReadSystemHeader P(( mpeg_VidStream *vid_stream ));

int find_start_code P(( FILE *input ));

int ReadPacket P(( unsigned char packetID, mpeg_VidStream *vid_stream ));

void ReadTimeStamp P((
   unsigned char *inputBuffer,
   unsigned char *hiBit,
   unsigned long *low4Bytes));

void ReadSTD P((
   unsigned char *inputBuffer,
   unsigned char *stdBufferScale,
   unsigned long *stdBufferSize));

void ReadRate P((
   unsigned char *inputBuffer,
   unsigned long *rate));

int MakeFloatClockTime P((
   unsigned char hiBit,
   unsigned long low4Bytes,
   double *floatClockTime));


#undef P

/* Status codes for bit stream i/o operations. */

#define NO_VID_STREAM (-1)
#define STREAM_UNDERFLOW (-2)
#define OK 1

/* Size increment of extension data buffers. */

#define EXT_BUF_SIZE 1024

/* External declarations for bitstream i/o operations. */
extern unsigned int bitMask[];
extern unsigned int nBitMask[];
extern unsigned int rBitMask[];
extern unsigned int bitTest[];

/* Macro for updating bit counter if analysis tool is on. */
#ifdef ANALYSIS
#define UPDATE_COUNT(numbits) bitCount += numbits
#else
#define UPDATE_COUNT(numbits)
#endif

#ifdef NO_SANITY_CHECKS
#define get_bits1(result)                                                 \
{                                                                         \
  UPDATE_COUNT(1);                                                        \
  result = ((vid_stream->curBits & 0x80000000) != 0);                     \
  vid_stream->curBits <<= 1;                                              \
  vid_stream->bit_offset++;                                               \
                                                                          \
  if (vid_stream->bit_offset & 0x20) {                                    \
    vid_stream->bit_offset = 0;                                           \
    vid_stream->buffer++;                                                 \
    vid_stream->curBits = *vid_stream->buffer;                            \
    vid_stream->buf_length--;                                             \
  }                                                                       \
}

#define get_bits2(result)                                                 \
{                                                                         \
  UPDATE_COUNT(2);                                                        \
  vid_stream->bit_offset += 2;                                            \
                                                                          \
  if (vid_stream->bit_offset & 0x20) {                                    \
    vid_stream->bit_offset -= 32;                                         \
    vid_stream->buffer++;                                                 \
    vid_stream->buf_length--;                                             \
    if (vid_stream->bit_offset) {                                         \
      vid_stream->curBits |=                                              \
	 (*vid_stream->buffer >> (2 - vid_stream->bit_offset));           \
    }                                                                     \
    result = ((vid_stream->curBits & 0xc0000000) >> 30);                  \
    vid_stream->curBits = *vid_stream->buffer << vid_stream->bit_offset;  \
  }                                                                       \
                                                                          \
  result = ((vid_stream->curBits & 0xc0000000) >> 30);                    \
  vid_stream->curBits <<= 2;                                              \
}

#define get_bitsX(num, mask, shift,  result)                              \
{                                                                         \
  UPDATE_COUNT(num);                                                      \
  vid_stream->bit_offset += num;                                          \
                                                                          \
  if (vid_stream->bit_offset & 0x20) {                                    \
    vid_stream->bit_offset -= 32;                                         \
    vid_stream->buffer++;                                                 \
    vid_stream->buf_length--;                                             \
    if (vid_stream->bit_offset) {                                         \
      vid_stream->curBits |= (*vid_stream->buffer >>                      \
      (num - vid_stream->bit_offset));                                    \
    }                                                                     \
    result = ((vid_stream->curBits & mask) >> shift);                     \
    vid_stream->curBits = *vid_stream->buffer << vid_stream->bit_offset;  \
  }                                                                       \
  else {                                                                  \
    result = ((vid_stream->curBits & mask) >> shift);                     \
    vid_stream->curBits <<= num;                                          \
  }                                                                       \
}
#else

#define get_bits1(result)                                                 \
{                                                                         \
  /* Check for underflow. */                                              \
                                                                          \
  if (vid_stream->buf_length < 2) {                                       \
    correct_underflow(vid_stream);                                        \
  }                                                                       \
  UPDATE_COUNT(1);                                                        \
  result = ((vid_stream->curBits & 0x80000000) != 0);                     \
  vid_stream->curBits <<= 1;                                              \
  vid_stream->bit_offset++;                                               \
                                                                          \
  if (vid_stream->bit_offset & 0x20) {                                    \
    vid_stream->bit_offset = 0;                                           \
    vid_stream->buffer++;                                                 \
    vid_stream->curBits = *vid_stream->buffer;                            \
    vid_stream->buf_length--;                                             \
  }                                                                       \
}

#define get_bits2(result)                                                 \
{                                                                         \
  /* Check for underflow. */                                              \
                                                                          \
  if (vid_stream->buf_length < 2) {                                       \
    correct_underflow(vid_stream);                                        \
  }                                                                       \
  UPDATE_COUNT(2);                                                        \
  vid_stream->bit_offset += 2;                                            \
                                                                          \
  if (vid_stream->bit_offset & 0x20) {                                    \
    vid_stream->bit_offset -= 32;                                         \
    vid_stream->buffer++;                                                 \
    vid_stream->buf_length--;                                             \
    if (vid_stream->bit_offset) {                                         \
      vid_stream->curBits |= (*vid_stream->buffer >>                      \
      (2 - vid_stream->bit_offset));                                      \
    }                                                                     \
    result = ((vid_stream->curBits & 0xc0000000) >> 30);                  \
    vid_stream->curBits = *vid_stream->buffer << vid_stream->bit_offset;  \
  }                                                                       \
                                                                          \
  result = ((vid_stream->curBits & 0xc0000000) >> 30);                    \
  vid_stream->curBits <<= 2;                                              \
}

#define get_bitsX(num, mask, shift,  result)                              \
{                                                                         \
  /* Check for underflow. */                                              \
                                                                          \
  if (vid_stream->buf_length < 2) {                                       \
    correct_underflow(vid_stream);                                        \
  }                                                                       \
  UPDATE_COUNT(num);                                                      \
  vid_stream->bit_offset += num;                                          \
                                                                          \
  if (vid_stream->bit_offset & 0x20) {                                    \
    vid_stream->bit_offset -= 32;                                         \
    vid_stream->buffer++;                                                 \
    vid_stream->buf_length--;                                             \
    if (vid_stream->bit_offset) {                                         \
      vid_stream->curBits |= (*vid_stream->buffer >>                      \
      (num - vid_stream->bit_offset));                                    \
    }                                                                     \
    result = ((vid_stream->curBits & mask) >> shift);                     \
    vid_stream->curBits = *vid_stream->buffer << vid_stream->bit_offset;  \
  }                                                                       \
  else {                                                                  \
   result = ((vid_stream->curBits & mask) >> shift);                      \
   vid_stream->curBits <<= num;                                           \
  }                                                                       \
}
#endif

#define get_bits3(result) get_bitsX(3,   0xe0000000, 29, result)
#define get_bits4(result) get_bitsX(4,   0xf0000000, 28, result)
#define get_bits5(result) get_bitsX(5,   0xf8000000, 27, result)
#define get_bits6(result) get_bitsX(6,   0xfc000000, 26, result)
#define get_bits7(result) get_bitsX(7,   0xfe000000, 25, result)
#define get_bits8(result) get_bitsX(8,   0xff000000, 24, result)
#define get_bits9(result) get_bitsX(9,   0xff800000, 23, result)
#define get_bits10(result) get_bitsX(10, 0xffc00000, 22, result)
#define get_bits11(result) get_bitsX(11, 0xffe00000, 21, result)
#define get_bits12(result) get_bitsX(12, 0xfff00000, 20, result)
#define get_bits14(result) get_bitsX(14, 0xfffc0000, 18, result)
#define get_bits16(result) get_bitsX(16, 0xffff0000, 16, result)
#define get_bits18(result) get_bitsX(18, 0xffffc000, 14, result)
#define get_bits32(result) get_bitsX(32, 0xffffffff,  0, result)

#define get_bitsn(num, result) get_bitsX((num), nBitMask[num], (32-(num)), result)

#ifdef NO_SANITY_CHECKS
#define show_bits32(result)                              		\
{                                                                       \
  if (vid_stream->bit_offset) {					        \
    result = vid_stream->curBits | (*(vid_stream->buffer+1) >>          \
	 (32 - vid_stream->bit_offset));                                \
  }                                                                     \
  else {                                                                \
    result = vid_stream->curBits;					\
  }                                                                     \
}

#define show_bitsX(num, mask, shift,  result)                           \
{                                                                       \
  int bO;                                                               \
  bO = vid_stream->bit_offset + num;                                    \
  if (bO > 32) {                                                        \
    bO -= 32;                                                           \
    result = ((vid_stream->curBits & mask) >> shift) |                  \
                (*(vid_stream->buffer+1) >> (shift + (num - bO)));      \
  }                                                                     \
  else {                                                                \
    result = ((vid_stream->curBits & mask) >> shift);                   \
  }                                                                     \
}

#else
#define show_bits32(result)                               		\
{                                                                       \
  /* Check for underflow. */                                            \
  if (vid_stream->buf_length < 2) {                                     \
    correct_underflow(vid_stream);                                      \
  }                                                                     \
  if (vid_stream->bit_offset) {						\
    result = vid_stream->curBits | (*(vid_stream->buffer+1) >>          \
    (32 - vid_stream->bit_offset));		                        \
  }                                                                     \
  else {                                                                \
    result = vid_stream->curBits;					\
  }                                                                     \
}

#define show_bitsX(num, mask, shift, result)                            \
{                                                                       \
  int bO;                                                               \
                                                                        \
  /* Check for underflow. */                                            \
  if (vid_stream->buf_length < 2) {                                     \
    correct_underflow(vid_stream);                                      \
  }                                                                     \
  bO = vid_stream->bit_offset + num;                                    \
  if (bO > 32) {                                                        \
    bO -= 32;                                                           \
    result = ((vid_stream->curBits & mask) >> shift) |                  \
                (*(vid_stream->buffer+1) >> (shift + (num - bO)));      \
  }                                                                     \
  else {                                                                \
    result = ((vid_stream->curBits & mask) >> shift);                   \
  }                                                                     \
}
#endif

#define show_bits1(result)  show_bitsX(1,  0x80000000, 31, result)
#define show_bits2(result)  show_bitsX(2,  0xc0000000, 30, result)
#define show_bits3(result)  show_bitsX(3,  0xe0000000, 29, result)
#define show_bits4(result)  show_bitsX(4,  0xf0000000, 28, result)
#define show_bits5(result)  show_bitsX(5,  0xf8000000, 27, result)
#define show_bits6(result)  show_bitsX(6,  0xfc000000, 26, result)
#define show_bits7(result)  show_bitsX(7,  0xfe000000, 25, result)
#define show_bits8(result)  show_bitsX(8,  0xff000000, 24, result)
#define show_bits9(result)  show_bitsX(9,  0xff800000, 23, result)
#define show_bits10(result) show_bitsX(10, 0xffc00000, 22, result)
#define show_bits11(result) show_bitsX(11, 0xffe00000, 21, result)
#define show_bits12(result) show_bitsX(12, 0xfff00000, 20, result)
#define show_bits13(result) show_bitsX(13, 0xfff80000, 19, result)
#define show_bits14(result) show_bitsX(14, 0xfffc0000, 18, result)
#define show_bits15(result) show_bitsX(15, 0xfffe0000, 17, result)
#define show_bits16(result) show_bitsX(16, 0xffff0000, 16, result)
#define show_bits17(result) show_bitsX(17, 0xffff8000, 15, result)
#define show_bits18(result) show_bitsX(18, 0xffffc000, 14, result)
#define show_bits19(result) show_bitsX(19, 0xffffe000, 13, result)
#define show_bits20(result) show_bitsX(20, 0xfffff000, 12, result)
#define show_bits21(result) show_bitsX(21, 0xfffff800, 11, result)
#define show_bits22(result) show_bitsX(22, 0xfffffc00, 10, result)
#define show_bits23(result) show_bitsX(23, 0xfffffe00,  9, result)
#define show_bits24(result) show_bitsX(24, 0xffffff00,  8, result)
#define show_bits25(result) show_bitsX(25, 0xffffff80,  7, result)
#define show_bits26(result) show_bitsX(26, 0xffffffc0,  6, result)
#define show_bits27(result) show_bitsX(27, 0xffffffe0,  5, result)
#define show_bits28(result) show_bitsX(28, 0xfffffff0,  4, result)
#define show_bits29(result) show_bitsX(29, 0xfffffff8,  3, result)
#define show_bits30(result) show_bitsX(30, 0xfffffffc,  2, result)
#define show_bits31(result) show_bitsX(31, 0xfffffffe,  1, result)

#define show_bitsn(num,result) show_bitsX((num), (0xffffffff << (32-(num))), (32-(num)), result)

#ifdef NO_SANITY_CHECKS
#define flush_bits32                                                  \
{                                                                     \
  UPDATE_COUNT(32);                                                   \
                                                                      \
  vid_stream->buffer++;                                               \
  vid_stream->buf_length--;                                           \
  vid_stream->curBits = *vid_stream->buffer  << vid_stream->bit_offset;\
}


#define flush_bits(num)                                               \
{                                                                     \
  vid_stream->bit_offset += num;                                      \
                                                                      \
  UPDATE_COUNT(num);                                                  \
                                                                      \
  if (vid_stream->bit_offset & 0x20) {                                \
    vid_stream->bit_offset -= 32;                                     \
    vid_stream->buffer++;                                             \
    vid_stream->buf_length--;                                         \
    vid_stream->curBits = *vid_stream->buffer  << vid_stream->bit_offset;\
  }                                                                   \
  else {                                                              \
    vid_stream->curBits <<= num;                                      \
  }                                                                   \
}
#else
#define flush_bits32                                                  \
{                                                                     \
  if (vid_stream  == NULL) {                                          \
    /* Deal with no vid stream here. */                               \
  }                                                                   \
                                                                      \
  if (vid_stream->buf_length < 2) {                                   \
    correct_underflow(vid_stream);                                    \
  }                                                                   \
                                                                      \
  UPDATE_COUNT(32);                                                   \
                                                                      \
  vid_stream->buffer++;                                               \
  vid_stream->buf_length--;                                           \
  vid_stream->curBits = *vid_stream->buffer  << vid_stream->bit_offset;\
}

#define flush_bits(num)                                               \
{                                                                     \
  if (vid_stream== NULL) {                                            \
    /* Deal with no vid stream here. */                               \
  }                                                                   \
                                                                      \
  if (vid_stream->buf_length < 2) {                                   \
    correct_underflow(vid_stream);                                    \
  }                                                                   \
                                                                      \
  UPDATE_COUNT(num);                                                  \
                                                                      \
  vid_stream->bit_offset += num;                                      \
                                                                      \
  if (vid_stream->bit_offset & 0x20) {                                \
    vid_stream->buf_length--;                                         \
    vid_stream->bit_offset -= 32;                                     \
    vid_stream->buffer++;                                             \
    vid_stream->curBits = *vid_stream->buffer << vid_stream->bit_offset;\
  }                                                                   \
  else {                                                              \
    vid_stream->curBits <<= num;                                      \
  }                                                                   \
}
#endif

#define UTIL2

extern int LUM_RANGE;
extern int CR_RANGE;
extern int CB_RANGE;


#define CB_BASE 1
#define CR_BASE (CB_BASE*CB_RANGE)
#define LUM_BASE (CR_BASE*CR_RANGE)

extern unsigned char pixel[256];
extern unsigned long wpixel[256];
extern int *lum_values;
extern int *cr_values;
extern int *cb_values;

#define Min(x,y) (((x) < (y)) ? (x) : (y))
#define Max(x,y) (((x) > (y)) ? (x) : (y))

#define GAMMA_CORRECTION(x) ((int)(pow((x) / 255.0, 1.0 / gammaCorrect) * 255.0))
#define CHROMA_CORRECTION256(x) ((x) >= 128 \
                        ? 128 + Min(127, (int)(((x) - 128.0) * chromaCorrect)) \
                        : 128 - Min(128, (int)((128.0 - (x)) * chromaCorrect)))
#define CHROMA_CORRECTION128(x) ((x) >= 0 \
                        ? Min(127,  (int)(((x) * chromaCorrect))) \
                        : Max(-128, (int)(((x) * chromaCorrect))))
#define CHROMA_CORRECTION256D(x) ((x) >= 128 \
                        ? 128.0 + Min(127.0, (((x) - 128.0) * chromaCorrect)) \
                        : 128.0 - Min(128.0, (((128.0 - (x)) * chromaCorrect))))
#define CHROMA_CORRECTION128D(x) ((x) >= 0 \
                        ? Min(127.0,  ((x) * chromaCorrect)) \
                        : Max(-128.0, ((x) * chromaCorrect)))



/* Code for unbound values in decoding tables */
#define ERROR (-1)
#define DCT_ERROR 63

#define MACRO_BLOCK_STUFFING 34
#define MACRO_BLOCK_ESCAPE 35

/* Two types of DCT Coefficients */
#define DCT_COEFF_FIRST 0
#define DCT_COEFF_NEXT 1

/* Special values for DCT Coefficients */
#define END_OF_BLOCK 62
#define ESCAPE 61

/* Structure for an entry in the decoding table of
 * macroblock_address_increment */
typedef struct {
  int value;       /* value for macroblock_address_increment */
  int num_bits;             /* length of the Huffman code */
} mb_addr_inc_entry;

/* Structure for an entry in the decoding table of macroblock_type */
typedef struct {
  unsigned int mb_quant;              /* macroblock_quant */
  unsigned int mb_motion_forward;     /* macroblock_motion_forward */
  unsigned int mb_motion_backward;    /* macroblock_motion_backward */
  unsigned int mb_pattern;            /* macroblock_pattern */
  unsigned int mb_intra;              /* macroblock_intra */
  int num_bits;                       /* length of the Huffman code */
} mb_type_entry;


/* Structures for an entry in the decoding table of coded_block_pattern */
typedef struct {
  unsigned int cbp;            /* coded_block_pattern */
  int num_bits;                /* length of the Huffman code */
} coded_block_pattern_entry;

/* External declaration of coded block pattern table. */

extern coded_block_pattern_entry coded_block_pattern[512];



/* Structure for an entry in the decoding table of motion vectors */
typedef struct {
  int code;              /* value for motion_horizontal_forward_code,
			  * motion_vertical_forward_code,
			  * motion_horizontal_backward_code, or
			  * motion_vertical_backward_code.
			  */
  int num_bits;          /* length of the Huffman code */
} motion_vectors_entry;


/* Decoding table for motion vectors */
extern motion_vectors_entry motion_vectors[2048];


/* Structure for an entry in the decoding table of dct_dc_size */
typedef struct {
  unsigned int value;    /* value of dct_dc_size (luminance or chrominance) */
  int num_bits;          /* length of the Huffman code */
} dct_dc_size_entry;

/* DCT coeff tables. */

#define RUN_MASK 0xfc00
#define LEVEL_MASK 0x03f0
#define NUM_MASK 0x000f
#define RUN_SHIFT 10
#define LEVEL_SHIFT 4

#define DecodeDCTDCSizeLum(macro_val)                    \
{                                                    \
  unsigned int index;	\
	\
  show_bits5(index);	\
  	\
  if (index < 31) {	\
  	macro_val = dct_dc_size_luminance[index].value;	\
  	flush_bits(dct_dc_size_luminance[index].num_bits);	\
  }	\
  else {	\
	show_bits9(index);	\
	index -= 0x1f0;	\
	macro_val = dct_dc_size_luminance1[index].value;	\
	flush_bits(dct_dc_size_luminance1[index].num_bits);	\
  }	\
}

#define DecodeDCTDCSizeChrom(macro_val)                      \
{                                                        \
  unsigned int index;	\
	\
  show_bits5(index);	\
  	\
  if (index < 31) {	\
  	macro_val = dct_dc_size_chrominance[index].value;	\
  	flush_bits(dct_dc_size_chrominance[index].num_bits);	\
  }	\
  else {	\
	show_bits10(index);	\
	index -= 0x3e0;	\
	macro_val = dct_dc_size_chrominance1[index].value;	\
	flush_bits(dct_dc_size_chrominance1[index].num_bits);	\
  }	\
}

#define DecodeDCTCoeff(dct_coeff_tbl, run, level)			\
{									\
  unsigned int temp, index;						\
  unsigned int value, next32bits, flushed;				\
									\
  show_bits32(next32bits);						\
									\
  /* show_bits8(index); */						\
  index = next32bits >> 24;						\
									\
  if (index > 3) {							\
    value = dct_coeff_tbl[index];					\
    run = value >> RUN_SHIFT;						\
    if (run != END_OF_BLOCK) {						\
      /* num_bits = (value & NUM_MASK) + 1; */				\
      /* flush_bits(num_bits); */					\
      if (run != ESCAPE) {						\
	 /* get_bits1(value); */					\
	 /* if (value) level = -level; */				\
	 flushed = (value & NUM_MASK) + 2;				\
         level = (value & LEVEL_MASK) >> LEVEL_SHIFT;			\
	 value = next32bits >> (32-flushed);				\
	 value &= 0x1;							\
	 if (value) level = -level;					\
	 /* next32bits &= ((~0) >> flushed);  last op before update */	\
       }								\
       else {    /* run == ESCAPE */					\
	 /* Get the next six into run, and next 8 into temp */		\
         /* get_bits14(temp); */					\
	 flushed = (value & NUM_MASK) + 1;				\
	 temp = next32bits >> (18-flushed);				\
	 /* Normally, we'd ad 14 to flushed, but I've saved a few	\
	  * instr by moving the add below */				\
	 temp &= 0x3fff;						\
	 run = temp >> 8;						\
	 temp &= 0xff;							\
	 if (temp == 0) {						\
            /* get_bits8(level); */					\
	    level = next32bits >> (10-flushed);				\
	    level &= 0xff;						\
	    flushed += 22;						\
 	    assert(level >= 128);					\
	 } else if (temp != 128) {					\
	    /* Grab sign bit */						\
	    flushed += 14;						\
	    level = ((int) (temp << 24)) >> 24;				\
	 } else {							\
            /* get_bits8(level); */					\
	    level = next32bits >> (10-flushed);				\
	    level &= 0xff;						\
	    flushed += 22;						\
	    level = level - 256;					\
	    assert(level <= -128 && level >= -255);			\
	 }								\
       }								\
       /* Update bitstream... */					\
       flush_bits(flushed);						\
       assert (flushed <= 32);						\
    }									\
  }									\
  else {								\
    switch (index) {                                                    \
    case 2: {   							\
      /* show_bits10(index); */						\
      index = next32bits >> 22;						\
      value = dct_coeff_tbl_2[index & 3];				\
      break;                                                            \
    }									\
    case 3: { 						                \
      /* show_bits10(index); */						\
      index = next32bits >> 22;						\
      value = dct_coeff_tbl_3[index & 3];				\
      break;                                                            \
    }									\
    case 1: {                                             		\
      /* show_bits12(index); */						\
      index = next32bits >> 20;						\
      value = dct_coeff_tbl_1[index & 15];				\
      break;                                                            \
    }									\
    default: { /* index == 0 */						\
      /* show_bits16(index); */						\
      index = next32bits >> 16;						\
      value = dct_coeff_tbl_0[index & 255];				\
    }}									\
    run = value >> RUN_SHIFT;						\
    level = (value & LEVEL_MASK) >> LEVEL_SHIFT;			\
									\
    /*									\
     * Fold these operations together to make it fast...		\
     */									\
    /* num_bits = (value & NUM_MASK) + 1; */				\
    /* flush_bits(num_bits); */						\
    /* get_bits1(value); */						\
    /* if (value) level = -level; */					\
									\
    flushed = (value & NUM_MASK) + 2;					\
    value = next32bits >> (32-flushed);					\
    value &= 0x1;							\
    if (value) level = -level;						\
									\
    /* Update bitstream ... */						\
    flush_bits(flushed);						\
    assert (flushed <= 32);						\
  }									\
}

#define DecodeDCTCoeffFirst(runval, levelval)         \
{                                                     \
  DecodeDCTCoeff(dct_coeff_first, runval, levelval);  \
}

#define DecodeDCTCoeffNext(runval, levelval)          \
{                                                     \
  DecodeDCTCoeff(dct_coeff_next, runval, levelval);   \
}

#define DecodeMBAddrInc(val)				\
{							\
    unsigned int index;					\
    show_bits11(index);					\
    val = mb_addr_inc[index].value;			\
    flush_bits(mb_addr_inc[index].num_bits);		\
}
#define DecodeMotionVectors(value)			\
{							\
  unsigned int index;					\
  show_bits11(index);					\
  value = motion_vectors[index].code;			\
  flush_bits(motion_vectors[index].num_bits);		\
}
#define DecodeMBTypeB(quant, motion_fwd, motion_bwd, pat, intra)	\
{									\
  unsigned int index;							\
									\
  show_bits6(index);							\
									\
  quant = mb_type_B[index].mb_quant;					\
  motion_fwd = mb_type_B[index].mb_motion_forward;			\
  motion_bwd = mb_type_B[index].mb_motion_backward;			\
  pat = mb_type_B[index].mb_pattern;					\
  intra = mb_type_B[index].mb_intra;					\
  flush_bits(mb_type_B[index].num_bits);				\
}
#define DecodeMBTypeI(quant, motion_fwd, motion_bwd, pat, intra)	\
{									\
  unsigned int index;							\
  static int quantTbl[4] = {ERROR, 1, 0, 0};				\
									\
  show_bits2(index);							\
									\
  motion_fwd = 0;							\
  motion_bwd = 0;							\
  pat = 0;								\
  intra = 1;								\
  quant = quantTbl[index];						\
  if (index) {								\
    flush_bits (1 + quant);						\
  }									\
}
#define DecodeMBTypeP(quant, motion_fwd, motion_bwd, pat, intra)	\
{									\
  unsigned int index;							\
									\
  show_bits6(index);							\
									\
  quant = mb_type_P[index].mb_quant;					\
  motion_fwd = mb_type_P[index].mb_motion_forward;			\
  motion_bwd = mb_type_P[index].mb_motion_backward;			\
  pat = mb_type_P[index].mb_pattern;					\
  intra = mb_type_P[index].mb_intra;					\
									\
  flush_bits(mb_type_P[index].num_bits);				\
}
#define DecodeCBP(coded_bp)						\
{									\
  unsigned int index;							\
									\
  show_bits9(index);							\
  coded_bp = coded_block_pattern[index].cbp;				\
  flush_bits(coded_block_pattern[index].num_bits);			\
}


void j_rev_dct_sparse (DCTBLOCK data, int pos);
void j_rev_dct (DCTBLOCK data);


/*******************************************************************
 Copyright (C) 2003 John Stewart, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*********************************************************************/

void start_textureTransform (void *textureNode, int ttnum);
void end_textureTransform (void *textureNode, int ttnum);

void
glpOpenGLInitialize(void);


void
BackEndClearBuffer(void);

void
BackEndLightsOff(void);

void lightState (GLint light, int state);

#ifndef AQUA
extern Display *Xdpy;
extern GLXContext GLcx;
extern XVisualInfo *Xvi;
extern Window Xwin;
extern Window GLwin;
extern void resetGeometry();
#endif
extern void glpOpenGLInitialize(void);

/*
 * FreeWRL plugin utilities header file.
 */

#ifndef _DEBUG
#define _DEBUG 0
#endif

#define SMALLSTRINGSIZE 64
#define STRINGSIZE 128
#define LARGESTRINGSIZE 256

#define PLUGIN_PORT 2009
#define PLUGIN_TIMEOUT_SEC 10
#define PLUGIN_TIMEOUT_NSEC 0

#define PLUGIN_RETRY 2
#define SLEEP_TIME 5

#define NO_ERROR 0
#define SOCKET_ERROR -1000
#define SIGNAL_ERROR -1001

#define UNUSED(v) ((void) v)

typedef struct _urlRequest {
    char url[FILENAME_MAX]; /* limit url length (defined in stdio.h) */
    void *instance;   /* NPP instance for plugin */
    unsigned int notifyCode; /* NPN_GetURLNotify, NPP_URLNotify */
} urlRequest;

const char* XEventToString(int type);
const char* XErrorToString(int error);

void URLencod (char *dest, const char *src, int maxlen);

/* what Browser are we running under? eg, netscape, mozilla, konqueror, etc */
#define MAXNETSCAPENAMELEN 256
extern char NetscapeName[MAXNETSCAPENAMELEN];

char *requestUrlfromPlugin(int sockDesc, uintptr_t plugin_instance, const char *url);
void  requestNewWindowfromPlugin( int sockDesc, uintptr_t plugin_instance, const char *url);
void requestPluginPrint(int sockDesc, const char* msg);
int receiveUrl(int sockDesc, urlRequest *request);

/*******************************************************************
 Copyright (C) 1998 Tuomas J. Lukka
 Copyright (C) 2002 John Stewart, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*********************************************************************/

/* transformed ray */
extern struct point_XYZ t_r1;
extern struct point_XYZ t_r2;
extern struct point_XYZ t_r3;

int
count_IFS_faces(int cin,
				struct X3D_IndexedFaceSet *this_IFS);

int 
IFS_face_normals(struct point_XYZ *facenormals,
				 int *faceok,
				 int *pointfaces,
				 int faces,
				 int npoints,
				 int cin,
				 struct SFColor *points,
				 struct X3D_IndexedFaceSet *this_IFS,
				 int ccw);

void
IFS_check_normal(struct point_XYZ *facenormals,
				 int this_face,
				 struct SFColor *points,
				 int base,
				 struct X3D_IndexedFaceSet *this_IFS,
				 int ccw);

void
add_to_face(int point,
			int face,
			int *pointfaces);

void
Elev_Tri(int vertex_ind,
		 int this_face,
		 int A,
		 int D,
		 int E,
		 int NONORMALS,
		 struct X3D_PolyRep *this_Elev,
		 struct point_XYZ *facenormals,
		 int *pointfaces,
		 int ccw);

void
Extru_tex(int vertex_ind,
		  int tci_ct,
		  int A,
		  int B,
		  int C,
		  int *tcindex,
		  int ccw,
		  int tcindexsize);

void Extru_ST_map(
        int triind_start,
        int start,
        int end,
        float *Vals,
        int nsec,
        int *tcindex,
        int *cindex,
        float *GeneratedTexCoords,
        int tcoordsize);

void
Extru_check_normal(struct point_XYZ *facenormals,
				   int this_face,
				   int dire,
				   struct X3D_PolyRep *rep_,
				   int ccw);

void
do_color_normal_reset(void);

void
do_glNormal3fv(struct SFColor *dest, GLfloat *param);

void stream_polyrep(void *node, void *coord, void *color, void *normal, void *texCoord);
void
render_ray_polyrep(void *node);

void compile_polyrep(void *node, void *coord, void *color, void *normal, void *texCoord);

/******************************************************************************
 Copyright (C) 1998 Tuomas J. Lukka, 2003 John Stewart, Ayla Khan, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*******************************************************************************/

#define DELTA 0.0001

/* definitions for mapping matrix in OpenGL format to standard math */
#define MAT00 mat[0]
#define MAT01 mat[1]
#define MAT02 mat[2]
#define MAT03 mat[3]
#define MAT10 mat[4]
#define MAT11 mat[5]
#define MAT12 mat[6]
#define MAT13 mat[7]
#define MAT20 mat[8]
#define MAT21 mat[9]
#define MAT22 mat[10]
#define MAT23 mat[11]
#define MAT30 mat[12]
#define MAT31 mat[13]
#define MAT32 mat[14]
#define MAT33 mat[15]

typedef struct quaternion {
	double w;
	double x;
	double y;
	double z;
} Quaternion;
void
matrix_to_quaternion (Quaternion *quat, double *mat) ;
void
quaternion_to_matrix (float *mat, Quaternion *quat) ;


void
vrmlrot_to_quaternion(Quaternion *quat,
					  const double x,
					  const double y,
					  const double z,
					  const double a);

void
quaternion_to_vrmlrot(const Quaternion *quat,
					  double *x,
					  double *y,
					  double *z,
					  double *a);

void
conjugate(Quaternion *quat);

void
inverse(Quaternion *ret,
		const Quaternion *quat);

double
norm(const Quaternion *quat);

void
normalize(Quaternion *quat);

void
add(Quaternion *ret,
	const Quaternion *q1,
	const Quaternion *q2);

void
multiply(Quaternion *ret,
		 const Quaternion *q1,
		 const Quaternion *q2);

void
scalar_multiply(Quaternion *quat,
				const double s);

void
rotation(struct point_XYZ *ret,
		 const Quaternion *quat,
		 const struct point_XYZ *v);

void
togl(Quaternion *quat);

void
set(Quaternion *ret,
	const Quaternion *quat);

void
slerp(Quaternion *ret,
	  const Quaternion *q1,
	  const Quaternion *q2,
	  const double t);

/*---------------------------------------------------------------------------

   rpng - simple PNG display program                              readpng.h

  ---------------------------------------------------------------------------

      Copyright (c) 1998-2000 Greg Roelofs.  All rights reserved.

      This software is provided "as is," without warranty of any kind,
      express or implied.  In no event shall the author or contributors
      be held liable for any damages arising in any way from the use of
      this software.

      Permission is granted to anyone to use this software for any purpose,
      including commercial applications, and to alter it and redistribute
      it freely, subject to the following restrictions:

      1. Redistributions of source code must retain the above copyright
         notice, disclaimer, and this list of conditions.
      2. Redistributions in binary form must reproduce the above copyright
         notice, disclaimer, and this list of conditions in the documenta-
         tion and/or other materials provided with the distribution.
      3. All advertising materials mentioning features or use of this
         software must display the following acknowledgment:

            This product includes software developed by Greg Roelofs
            and contributors for the book, "PNG: The Definitive Guide,"
            published by O'Reilly and Associates.

  ---------------------------------------------------------------------------*/

#ifndef MAX
#  define MAX(a,b)  ((a) > (b)? (a) : (b))
#  define MIN(a,b)  ((a) < (b)? (a) : (b))
#endif

#ifdef DEBUG
#  define Trace(x)  {fprintf x ; fflush(stderr); fflush(stdout);}
#else
#  define Trace(x)  ;
#endif

typedef unsigned char   uch;
typedef unsigned short  ush;
typedef unsigned long   ulg;


/* prototypes for public functions in readpng.c */

void readpng_version_info(void);

int readpng_init(FILE *infile, ulg *pWidth, ulg *pHeight);

int readpng_get_bgcolor(uch *bg_red, uch *bg_green, uch *bg_blue);

uch *readpng_get_image(double display_exponent, int *pChannels,
                       ulg *pRowbytes);

void readpng_cleanup(int free_image_data);
/*******************************************************************
 Copyright (C) 2003 John Stewart, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*********************************************************************/

#define ASLEN 500


double return_Duration(int indx);

void
do_active_inactive(int *act,
				   double *inittime,
				   double *startt,
				   double *stopt,
				   int loop,
				   double myDuration,
				   double speed);

int
find_key(int kin, float frac, float *keys);

void
do_OintScalar(void *node);

void
do_OintCoord(void *node);

void do_OintCoord2D(void *node);
void do_OintPos2D(void *node);
void do_PositionInterpolator(void *node);
void do_ColorInterpolator(void *node);
void do_GeoPositionInterpolator(void *node);

void
do_Oint4(void *node);

void do_CollisionTick(void *ptr);
void do_AudioTick(void *ptr);
void do_TimeSensorTick(void *ptr);
void do_ProximitySensorTick(void *ptr);
void do_GeoProximitySensorTick(void *ptr);
void do_MovieTextureTick(void *ptr);
void do_VisibilitySensorTick(void *ptr);

void do_Anchor( void *ptr, int typ, int but1, int over);
void do_TouchSensor( void *ptr, int typ, int but1, int over);
void do_GeoTouchSensor(void *ptr, int typ, int but1, int over);
void do_PlaneSensor(void *ptr, int typ, int but1, int over);
void do_CylinderSensor(void *ptr, int typ, int but1, int over);
void do_SphereSensor(void *ptr, int typ, int but1, int over);

extern int snapCount;
extern int maxSnapImages;          /* --maximg command line parameter              */
extern int snapGif;            /* --gif save as an animated GIF, not mpg       */
extern char *snapseqB;          /* --seqb - snap sequence base filename         */
extern char *snapsnapB;         /* --snapb -single snapshot files               */
extern char *seqtmp;            /* --seqtmp - directory for temp files          */
extern int snapsequence;	/* --seq - snapshot sequence, not single click	*/
extern int doSnapshot;		/* are we doing a snapshot?			*/
void setSnapshot();		/* set a snapshot going				*/
void Snapshot();
extern void abort();
/*******************************************************************
 Copyright (C) 2003 John Stewart, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*********************************************************************/

/* include file for sound engine client/server */


#define SNDMAXMSGSIZE 256

/* states of the sound engine */
#define SOUND_FAILED  2
#define SOUND_STARTED 1
#define SOUND_NEEDS_STARTING 3

#define MAXSOUNDS 50

typedef struct {
	long mtype;	/* message type */
	char	msg[SNDMAXMSGSIZE]; /* message data */
} FWSNDMSG;


void
Sound_toserver(char *message);

void
SoundEngineInit(void);

void
waitformessage(void);

void
SoundEngineDestroy(void);

int
SoundSourceRegistered(int num);

float
SoundSourceInit(int num,
				int loop,
				double pitch,
				double start_time,
				double stop_time,
				char *url);

void
SetAudioActive(int num, int stat);

/*******************************************************************
 Copyright (C) 1998 Tuomas J. Lukka
 Copyright (C) 2002 John Stewart, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*********************************************************************/

#define GET_THIS_TEXTURE thisTextureType = node->_nodeType; \
                                if (thisTextureType==NODE_ImageTexture){ \
                                it = (struct X3D_ImageTexture*) node; \
                                thisTexture = it->__textureTableIndex; \
                        } else if (thisTextureType==NODE_PixelTexture){ \
                                pt = (struct X3D_PixelTexture*) node; \
                                thisTexture = pt->__textureTableIndex; \
                        } else if (thisTextureType==NODE_MovieTexture){ \
                                mt = (struct X3D_MovieTexture*) node; \
                                thisTexture = mt->__textureTableIndex; \
                        } else { ConsoleMessage ("Invalid type for texture, %s\n",stringNodeType(thisTextureType)); return;}

/* for texIsloaded structure */
#define TEX_NOTLOADED       0
#define TEX_LOADING         1
#define TEX_NEEDSBINDING	2
#define TEX_LOADED          3
#define TEX_UNSQUASHED      4


/* older stuff - check if needed */

/* bind_texture stores the param table pointer for the texture here */

struct loadTexParams {
	/* data sent in to texture parsing thread */
	GLuint *texture_num;
	GLuint genned_texture;
	unsigned repeatS;
	unsigned repeatT;
	struct Uni_String *parenturl;
	unsigned type;
	struct Multi_String url;

	/* data returned from texture parsing thread */
	char *filename;
	int depth;
	int x;
	int y;
	int frames;		/* 1 unless video stream */
	unsigned char *texdata;
	GLint Src;
	GLint Trc;
	GLint Image;
};

struct multiTexParams {
	GLint texture_env_mode;
	GLint combine_rgb;
	GLint source0_rgb;
	GLint operand0_rgb;
	GLint source1_rgb;
	GLint operand1_rgb;
	GLint combine_alpha;
	GLint source0_alpha;
	GLint operand0_alpha;
	GLint source1_alpha;
	GLint operand1_alpha;
	GLfloat rgb_scale;
	GLfloat alpha_scale;
};


/* we keep track of which textures have been loaded, and which have not */

void bind_image(int type, struct Uni_String *parenturl, struct Multi_String url,
				GLuint *texture_num,
				int repeatS,
				int repeatT,
				void  *param);

/*******************************************************************
 Copyright (C) 2003 John Stewart, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*********************************************************************/

#define NONE 0
#define EXAMINE 1
#define WALK 2
#define EXFLY 3
#define FLY 4

#define VIEWER_STRING(type) ( \
	type == NONE ? "NONE" : ( \
	type == EXAMINE ? "EXAMINE" : ( \
	type == WALK ? "WALK" : ( \
	type == EXFLY ? "EXFLY" : ( \
	type == FLY ? "FLY" : "UNKNOWN")))))

#define PRESS "PRESS"
#define PRESS_LEN 5

#define DRAG "DRAG"
#define DRAG_LEN 4

#define RELEASE "RELEASE"
#define RELEASE_LEN 7

#define KEYS_HANDLED 12
/* my %actions = ( */
/* 	a => sub {$aadd[2] -= $_[0]}, */
/* 	z => sub {$aadd[2] += $_[0]}, */
/* 	j => sub {$aadd[0] -= $_[0]}, */
/* 	l => sub {$aadd[0] += $_[0]}, */
/* 	p => sub {$aadd[1] += $_[0]}, */
/* 	';' => sub {$aadd[1] -= $_[0]}, */

/* 	8 => sub {$radd[0] += $_[0]}, */
/* 	k => sub {$radd[0] -= $_[0]}, */
/* 	u => sub {$radd[1] -= $_[0]}, */
/* 	o => sub {$radd[1] += $_[0]}, */
/* 	7 => sub {$radd[2] -= $_[0]}, */
/* 	9 => sub {$radd[2] += $_[0]}, */
/* ); */
#define KEYMAP {{ 'a', 0 }, { 'z', 0 }, { 'j', 0 }, { 'l', 0 }, { 'p', 0 }, { ';', 0 }, { '8', 0 }, { 'k', 0 }, { 'u', 0 }, { 'o', 0 }, { '7', 0 }, { '9', 0 }}

#define COORD_SYS 3
#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

#define STRING_SIZE 256

#define IN_FILE "/tmp/inpdev"
#define IN_FILE_BYTES 100
#define INPUT_LEN 9
#define INPUT_LEN_Z 8
#define X_OFFSET 8
#define Y_OFFSET 17
#define Z_OFFSET 0
#define QUAT_W_OFFSET 26
#define QUAT_X_OFFSET 35
#define QUAT_Y_OFFSET 44
#define QUAT_Z_OFFSET 53


/* extern struct point_XYZ ViewerPosition; */
/* extern struct orient ViewerOrientation; */


typedef struct viewer_walk {
	double SX;
	double SY;
	double XD;
	double YD;
	double ZD;
	double RD;
} X3D_Viewer_Walk;


typedef struct viewer_examine {
	struct point_XYZ Origin;
	Quaternion OQuat;
	Quaternion SQuat;
	double ODist;
	double SY;
} X3D_Viewer_Examine;

typedef struct key {
	char key;
	unsigned int hit;
} Key;


/* Modeled after Descent(tm) ;) */
typedef struct viewer_fly {
	double Velocity[COORD_SYS];
	double AVelocity[COORD_SYS];
	Key Down[KEYS_HANDLED];
	Key WasDown[KEYS_HANDLED];
	double lasttime;
} X3D_Viewer_Fly;


typedef struct viewer {
	struct point_XYZ Pos;
	struct point_XYZ AntiPos;
	Quaternion Quat;
	Quaternion AntiQuat;
	int headlight;
	double speed;
	double Dist;
	double eyehalf;
	double eyehalfangle;
	unsigned int buffer;
	int oktypes[6];		/* boolean for types being acceptable. */
	X3D_Viewer_Walk *walk;
	X3D_Viewer_Examine *examine;
	X3D_Viewer_Fly *fly;

	struct X3D_GeoViewpoint *GeoSpatialNode; /* NULL, unless we are a GeoViewpoint */
} X3D_Viewer;


void
viewer_init(X3D_Viewer *viewer,
			int type);

void
print_viewer();

unsigned int
get_buffer();

void
set_buffer( const unsigned int buffer);

int
get_headlight();

void
toggle_headlight();

int
use_keys(void);

void
set_eyehalf( const double eyehalf,
			const double eyehalfangle);

void
set_viewer_type(const int type);

void
resolve_pos(void);
void getViewpointExamineDistance(void);

void
xy2qua(Quaternion *ret,
	   const double x,
	   const double y);

void
viewer_togl( double fieldofview);


void handle(const int mev, const unsigned int button, const float x, const float y);

void
handle_walk( const int mev,
			const unsigned int button,
			const float x,
			const float y);

void
handle_examine(const int mev,
			   const unsigned int button,
			   const float x,
			   const float y);

void
handle_key(const char key);

void
handle_keyrelease (const char key);

void
handle_tick();

void
handle_tick_walk();

void
handle_tick_exfly();

void
handle_tick_fly();

void
set_action(char *key);

void
set_stereo_offset(unsigned int buffer,
				  const double eyehalf,
				  const double eyehalfangle,
				  double fieldofview);

void
increment_pos( struct point_XYZ *vec);

void
bind_viewpoint(struct X3D_Viewpoint *node);

void
bind_geoviewpoint(struct X3D_GeoViewpoint *node);

void viewer_calculate_speed(void);

extern X3D_Viewer Viewer; /* in VRMLC.pm */

void viewer_default(void);

extern float eyedist;
extern float screendist;

void XEventStereo(void);

void getCurrentSpeed();

/* header file for the X3D parser, only items common between the X3DParser files should be here. */

#define PARSING_NODES 1
#define PARSING_SCRIPT 2
#define PARSING_PROTODECLARE  3
#define PARSING_PROTOINTERFACE  4
#define PARSING_PROTOBODY       5
#define PARSING_PROTOINSTANCE   6
#define PARSING_IS		7
#define PARSING_CONNECT		8

/* for our internal PROTO tables, and, for initializing the XML parser */
#define PROTOINSTANCE_MAX_LEVELS 30
#define PROTOINSTANCE_MAX_PARAMS 20

#define DECREMENT_PARENTINDEX \
        if (parentIndex > 0) parentIndex--; else ConsoleMessage ("X3DParser, line %d stack underflow",LINE);

#define INCREMENT_PARENTINDEX \
        if (parentIndex < (PARENTSTACKSIZE-2))  { \
                parentIndex++; \
                parentStack[parentIndex] = NULL; /* make sure we know the state of the new Top of Stack */ \
        } else ConsoleMessage ("X3DParser, line %d stack overflow",LINE);



int freewrl_XML_GetCurrentLineNumber();
#define LINE freewrl_XML_GetCurrentLineNumber()
#define TTY_SPACE {int tty; printf ("%3d ",parentIndex); for (tty = 0; tty < parentIndex; tty++) printf ("  ");}
extern int parserMode;

#define PARENTSTACKSIZE 256
extern int parentIndex;
extern struct X3D_Node *parentStack[PARENTSTACKSIZE];
extern char *scriptText;
extern int scriptTextMallocSize;


/* function protos */
void parseProtoDeclare (const char **atts);
void parseProtoInterface (const char **atts);
void parseProtoBody (const char **atts);
void registerX3DScriptField(int myScriptNumber,int type,int kind, int myFieldOffs, char *name, char *value);
void parseProtoInstance (const char **atts);
void parseProtoInstanceFields(const char *name, const char **atts);
void dumpProtoBody (const char *name, const char **atts);
void dumpCDATAtoProtoBody (char *str);
void endDumpProtoBody (const char *name);
void parseScriptProtoField(const char **atts);
int getFieldFromScript (char *fieldName, int scriptno, int *offs, int *type, int *accessType);
void expandProtoInstance(struct X3D_Group * myGroup);
void freeProtoMemory (void);
void kill_X3DProtoScripts(void);


#endif /* __LIBFREEX3D_MAIN_H__ */
