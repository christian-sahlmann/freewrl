
/*
  $Id: OpenGL_Utils.c,v 1.145 2010/09/10 23:01:31 dug9 Exp $

  FreeWRL support library.
  OpenGL initialization and functions. Rendering functions.
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
#include <system_threads.h>
#include "../threads.h"		/* for isinputThreadParsing() */
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>
#include <list.h>
#include <io_files.h>
#include <resources.h>


#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../main/ProdCon.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CParse.h"
#include "../vrml_parser/CRoutes.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"
#include "../scenegraph/sounds.h"
#include "../scenegraph/LinearAlgebra.h"
#include "../scenegraph/Component_KeyDevice.h"	/* resolving implicit declarations */
#include "../input/EAIHeaders.h"		/* resolving implicit declarations */
#include "../input/InputFunctions.h"
#include "Frustum.h"
#include "../opengl/Material.h"
#include "../scenegraph/Component_Core.h"
#include "../scenegraph/Component_Networking.h"
#include "../scenegraph/RenderFuncs.h"
#include "Textures.h"
#include "OpenGL_Utils.h"

#include <float.h>

#include "../x3d_parser/Bindable.h"

#define USE_JS_EXPERIMENTAL_CODE 0
void kill_rendering(void);

/* Node Tracking */
#if USE_JS_EXPERIMENTAL_CODE
static void kill_X3DNodes(void);
#endif

static void createdMemoryTable();
static void increaseMemoryTable();
static struct X3D_Node ** memoryTable = NULL;
static int tableIndexSize = INT_ID_UNDEFINED;
static int nextEntry = 0;
static struct X3D_Node *forgottenNode;

#if USE_JS_EXPERIMENTAL_CODE
static void killNode (int index);
#endif

static void mesa_Frustum(double left, double right, double bottom, double top, double nearZ, double farZ, double *m);
static void fw_glLoadMatrixd(double *val);
static void mesa_Ortho(double left, double right, double bottom, double top, double nearZ, double farZ, double *m);

/* is this 24 bit depth? 16? 8?? Assume 24, unless set on opening */
int displayDepth = 24;

static float cc_red = 0.0f, cc_green = 0.0f, cc_blue = 0.0f, cc_alpha = 1.0f;
int cc_changed = FALSE;

static pthread_mutex_t  memtablelock = PTHREAD_MUTEX_INITIALIZER;
#define LOCK_MEMORYTABLE 		pthread_mutex_lock(&memtablelock);
#define UNLOCK_MEMORYTABLE		pthread_mutex_unlock(&memtablelock);
/*
#define LOCK_MEMORYTABLE 		{printf ("LOCK_MEMORYTABLE at %s:%d\n",__FILE__,__LINE__); pthread_mutex_lock(&memtablelock);}
#define UNLOCK_MEMORYTABLE		{printf ("UNLOCK_MEMORYTABLE at %s:%d\n",__FILE__,__LINE__); pthread_mutex_unlock(&memtablelock);}
*/


/******************************************************************/
/* textureTransforms of all kinds */

/* change the clear colour, selected from the GUI, but do the command in the
   OpenGL thread */

void setglClearColor (float *val) {
	cc_red = *val; val++;
	cc_green = *val; val++;
	cc_blue = *val;
#ifdef AQUA
	val++;
	cc_alpha = *val;
#endif
	cc_changed = TRUE;
}        


/**************************************************************************************

		Determine near far clip planes.

We have 2 choices; normal geometry, or we have a Geospatial sphere.

If we have normal geometry (normal Viewpoint, or GeoViewpoint with GC coordinates)
then, we take our AABB (axis alligned bounding box), rotate the 8 Vertices, and
find the min/max Z distance, and just use this. 

This works very well for examine objects, or when we are within a virtual world.
----

If we are Geospatializing around the earth, so we have GeoSpatial and have UTM or GD
coordinates, lets do some optimizations here.

First optimization, we know our height above the origin, and we most certainly are not
going to see things past the origin, so we assume far plane is height above the origin.

Second, we know our AABB contains the Geospatial sphere, and it ALSO contains the highest
mountain peak, so we just go and find the value representing the highest peak. Our
near plane is thus farPlane - highestPeak. 

**************************************************************************************/

#if USE_JS_EXPERIMENTAL_CODE
/* read a file, put it into memory. */
static char * readInputString(char *fn) {
        char *buffer;
        FILE *infile;
        size_t justread;

	#define MAXREADSIZE 4000

        /* ok, now, really read this one. */
        infile = fopen(fn,"r");

        if (infile == NULL){
                ConsoleMessage("problem reading file '%s' \n",fn);
                return NULL;
        }


        buffer =(char *)MALLOC(MAXREADSIZE * sizeof (char));
        justread = fread (buffer,1,MAXREADSIZE,infile);
	if (justread >= MAXREADSIZE) {
		ConsoleMessage ("Shader too large for buffer\n");
		return NULL;
	}

	fclose (infile);

	buffer[justread] = '\0';
        return (buffer);
}
#endif
#undef MAXREADSIZE

#ifndef USE_OLD_OPENGL
static void shaderErrorLog(GLuint myShader) {
        #ifdef GL_VERSION_2_0
#define MAX_INFO_LOG_SIZE 512
                GLchar infoLog[MAX_INFO_LOG_SIZE];
                glGetShaderInfoLog(myShader, MAX_INFO_LOG_SIZE, NULL, infoLog);
                ConsoleMessage ("problem with VERTEX shader: %s",infoLog);
        #else
                ConsoleMessage ("Problem compiling shader");
        #endif
}
#endif

#ifndef USE_OLD_OPENGL
/* read in shaders and place the resulting shader program in the "whichShader" field if success. */
static void getAppearanceShader(s_shader_capabilities_t *myShader, char *pathToShaders) {
	char *inTextPointer;
	GLint success;
	GLuint myVertexShader = 0;
	GLuint myFragmentShader= 0;
	GLuint myProg = 0;

#ifdef READ_SHADER_FROM_FILE
	char *inTextFile;

	if (strlen(pathToShaders) > 1000) return;  /* bounds error */
	inTextFile = MALLOC(2000);

	/* get Vertex shader */
	strcpy (inTextFile,pathToShaders);
	strcat (inTextFile,".vs");

	printf ("getAppearanceShader, path %s\n",inTextFile); 
	inTextPointer = readInputString(inTextFile);
	if (inTextPointer==NULL) return;
	
#else
	inTextPointer = 
		"varying vec4 colour;" \
		"varying vec4 spec;" \
		"uniform 	vec4 myMaterialAmbient;" \
		"uniform 	vec4 myMaterialDiffuse;" \
		"uniform 	vec4 myMaterialSpecular;" \
		"uniform 	float myMaterialShininess;" \
		"uniform 	vec4 myMaterialEmission;" \
		"uniform		mat4 fw_ModelViewMatrix;" \
		"uniform		mat4 fw_ProjectionMatrix;" \
		"attribute	vec4 fw_Vertex;" \
		"attribute	vec3 fw_Normal; " \
		" " \
		"uniform int lightState;" \
		"vec3 ADSLightModel(in vec3 myNormal, in vec4 myPosition) {" \
		"	vec4 myLightAmbient = vec4(0., 0., 0., 1);" \
		"	vec4 myLightDiffuse = vec4(1., 1., 1., 1.);" \
		"	vec4 myLightPosition = vec4 (0., 0., 1., 0.);" \
		"	vec3 norm = normalize (myNormal);" \
		"	vec3 lightv = normalize(myLightPosition.xyz-myPosition.xyz);" \
		"	vec3 viewv = -normalize(myPosition.xyz);" \
		"	vec3 refl = reflect (-lightv, norm);" \
		"	vec4 diffuse = max (0.0, dot(lightv, norm))*myMaterialDiffuse*myLightDiffuse;" \
		"	vec4 ambient = myMaterialAmbient*myLightAmbient;" \
		"	return clamp(vec3(ambient+diffuse+myMaterialEmission), 0.0, 1.0);" \
		"}" \
		"vec4 specularCalculation(in vec3 myNormal, in vec4 myPosition) {" \
		"	vec4 myLightSpecular = vec4(0.6, 0.6, 0.6, 1.0);" \
		"	vec4 myLightPosition = vec4 (0., 0., 1., 0.);" \
		"	vec3 norm = normalize (myNormal);" \
		"	vec3 lightv = normalize(myLightPosition.xyz-myPosition.xyz);" \
		"	vec3 viewv = -normalize(myPosition.xyz);" \
		"	vec3 refl = reflect (-lightv, norm);" \
		"	vec4 specular = vec4 (0.0, 0.0, 0.0, 1.0);" \
		"	if (dot(lightv, viewv) > 0.0) {" \
		"		specular = pow(max(0.0, dot(viewv, refl))," \
		"			myMaterialShininess)*myMaterialSpecular*myLightSpecular;" \
		"	}" \
		"	return clamp(vec4(specular), 0.0, 1.0);" \
		"}" \
		"void main(void) {" \
		"	vec3 transNorm = vec3(fw_ModelViewMatrix * vec4(fw_Normal,0.0)); " \
		"	/* transNorm = normalize(transNorm); */" \
		"	vec4 pos = fw_ModelViewMatrix * fw_Vertex;" \
		"	colour = vec4(ADSLightModel(transNorm, pos),1);" \
		"	spec = specularCalculation(transNorm,pos);" \
		"	gl_Position = fw_ProjectionMatrix * fw_ModelViewMatrix * fw_Vertex;" \
		"}" \
		"";

#endif
	(*myShader).myShaderProgram = CREATE_PROGRAM;
	myProg = (*myShader).myShaderProgram;

	myVertexShader = CREATE_SHADER (VERTEX_SHADER);
	SHADER_SOURCE(myVertexShader, 1, (const GLchar **) &inTextPointer, NULL);
	COMPILE_SHADER(myVertexShader);
	GET_SHADER_INFO(myVertexShader, COMPILE_STATUS, &success);
	if (!success) {
		shaderErrorLog(myVertexShader);
	} else {

		ATTACH_SHADER(myProg, myVertexShader);
	}

#ifdef READ_SHADER_FROM_FILE
	FREE_IF_NZ(inTextPointer);
#endif


#ifdef READ_SHADER_FROM_FILE
	/* get Fragment shader */
	strcpy (inTextFile,pathToShaders);
	strcat (inTextFile,".fs");

	/* printf ("getAppearanceShader, path %s\n",inTextFile); */
	inTextPointer = readInputString(inTextFile);
	if (inTextPointer==NULL) return;
#else

	inTextPointer = "varying vec4 colour; varying vec4 spec; void main () { gl_FragColor = clamp(colour+spec,0.,1.);}";

#endif

	myFragmentShader = CREATE_SHADER (FRAGMENT_SHADER);
	SHADER_SOURCE(myFragmentShader, 1, (const GLchar **) &inTextPointer, NULL);
	COMPILE_SHADER(myFragmentShader);
	GET_SHADER_INFO(myFragmentShader, COMPILE_STATUS, &success);
	if (!success) {
		shaderErrorLog(myFragmentShader);
	} else {
		ATTACH_SHADER(myProg, myFragmentShader);
	}
#ifdef READ_SHADER_FROM_FILE
	FREE_IF_NZ(inTextPointer);
	FREE_IF_NZ(inTextFile);
#endif

	LINK_SHADER(myProg);

	(*myShader).myMaterialAmbient = GET_UNIFORM(myProg,"myMaterialAmbient");
	(*myShader).myMaterialDiffuse = GET_UNIFORM(myProg,"myMaterialDiffuse");
	(*myShader).myMaterialSpecular = GET_UNIFORM(myProg,"myMaterialSpecular");
	(*myShader).myMaterialShininess = GET_UNIFORM(myProg,"myMaterialShininess");
	(*myShader).myMaterialEmission = GET_UNIFORM(myProg,"myMaterialEmission");
	(*myShader).lightState = GET_UNIFORM(myProg,"lightState");
	(*myShader).lightAmbient = GET_UNIFORM(myProg,"lightAmbient");
	(*myShader).lightDiffuse = GET_UNIFORM(myProg,"lightDiffuse");
	(*myShader).lightSpecular = GET_UNIFORM(myProg,"lightSpecular");
	(*myShader).lightPosition = GET_UNIFORM(myProg,"lightPosition");

	(*myShader).ModelViewMatrix = GET_UNIFORM(myProg,"fw_ModelViewMatrix");
	(*myShader).ProjectionMatrix = GET_UNIFORM(myProg,"fw_ProjectionMatrix");
	(*myShader).Vertices = GET_ATTRIB(myProg,"fw_Vertex");
	(*myShader).Normals = GET_ATTRIB(myProg,"fw_Normal");
printf ("shader uniforms: vertex %d normal %d modelview %d projection %d\n",
(*myShader).Vertices,
(*myShader).Normals,
(*myShader).ModelViewMatrix,
(*myShader).ProjectionMatrix);

}
#endif


static void handle_GeoLODRange(struct X3D_GeoLOD *node) {
	int oldInRange;
	double cx,cy,cz;

	/* find the length of the line between the moved center and our current viewer position */
	cx = Viewer.currentPosInModel.x - node->__movedCoords.c[0];
	cy = Viewer.currentPosInModel.y - node->__movedCoords.c[1];
	cz = Viewer.currentPosInModel.z - node->__movedCoords.c[2];

	/* printf ("geoLOD, distance between me and center is %lf\n", sqrt (cx*cx + cy*cy + cz*cz)); */

	/* try to see if we are closer than the range */
	oldInRange = node->__inRange;
	if((cx*cx+cy*cy+cz*cz) > (node->range*X3D_GEOLOD(node)->range)) {
		node->__inRange = FALSE;
	} else {
		node->__inRange = TRUE;
	}

	
	if (oldInRange != node->__inRange) {

		#ifdef VERBOSE
		if (node->__inRange) printf ("TRUE:  "); else printf ("FALSE: ");
		printf ("range changed; level %d, comparing %lf:%lf:%lf and range %lf node %u\n",
			node->__level, cx,cy,cz, node->range, node);
		#endif

		/* initialize the "children" field, if required */
		if (node->children.p == NULL) node->children.p=MALLOC(sizeof(void *) * 4);

		if (node->__inRange == FALSE) {
			#ifdef VERBOSE
			printf ("GeoLOD %u level %d, inRange set to FALSE, range %lf\n",node, node->__level, node->range);
			#endif

			node->level_changed = 1;
			node->children.p[0] = node->__child1Node; 
			node->children.p[1] = node->__child2Node; 
			node->children.p[2] = node->__child3Node; 
			node->children.p[3] = node->__child4Node; 
			node->children.n = 4;
		} else {
			#ifdef VERBOSE
			printf ("GeoLOD %u level %d, inRange set to TRUE range %lf\n",node, node->__level, node->range);
			#endif
			node->level_changed = 0;
			node->children.p[0] = node->rootNode.p[0]; node->children.n = 1;
		}
		MARK_EVENT(X3D_NODE(node), offsetof (struct X3D_GeoLOD, level_changed));
		MARK_EVENT(X3D_NODE(node), offsetof (struct X3D_GeoLOD, children));
		oldInRange = X3D_GEOLOD(node)->__inRange;

		/* lets work out extent here */
		INITIALIZE_EXTENT;
		/* printf ("geolod range changed, initialized extent, czyzsq %4.2f rangesw %4.2f from %4.2f %4.2f %4.2f\n",
cx*cx+cy*cy+cz*cz,node->range*node->range,cx,cy,cz); */
		update_node(X3D_NODE(node));
	}
}

/* draw a simple bounding box around an object */
void drawBBOX(struct X3D_Node *node) {
#ifndef IPHONE
/* debugging */	FW_GL_COLOR3F((float)1.0,(float)0.6,(float)0.6);
/* debugging */
/* debugging */	/* left group */
/* debugging */	glBegin(GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MIN_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MIN_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MIN_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MAX_Y, node->EXTENT_MIN_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MAX_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MAX_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MIN_Y, node->EXTENT_MAX_Z);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MAX_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	/* right group */
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MIN_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MIN_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MIN_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MAX_Y, node->EXTENT_MIN_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MAX_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MAX_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MIN_Y, node->EXTENT_MAX_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MAX_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	/* joiners */
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MIN_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MIN_Y, node->EXTENT_MIN_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MIN_Y, node->EXTENT_MAX_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MIN_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MAX_Y, node->EXTENT_MIN_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MAX_Y, node->EXTENT_MIN_Z);
/* debugging */	glEnd();
/* debugging */	
/* debugging */	glBegin (GL_LINES);
/* debugging */	glVertex3d(node->EXTENT_MIN_X, node->EXTENT_MAX_Y, node->EXTENT_MAX_Z);
/* debugging */	glVertex3d(node->EXTENT_MAX_X, node->EXTENT_MAX_Y, node->EXTENT_MAX_Z);
/* debugging */	glEnd();
#endif

}

static void calculateNearFarplanes(struct X3D_Node *vpnode) {
	struct point_XYZ bboxPoints[8];
	double cfp = DBL_MIN;
	double cnp = DBL_MAX;
	GLDOUBLE MM[16];

	int ci;

	#ifdef VERBOSE
	printf ("have a bound viewpoint... lets calculate our near/far planes from it \n");
	printf ("we are currently at %4.2f %4.2f %4.2f\n",Viewer.currentPosInModel.x, Viewer.currentPosInModel.y, Viewer.currentPosInModel.z);
	#endif


	/* verify parameters here */
	if ((vpnode->_nodeType != NODE_Viewpoint) && 
		(vpnode->_nodeType != NODE_OrthoViewpoint) &&
		(vpnode->_nodeType != NODE_GeoViewpoint)) {
		printf ("can not do this node type yet %s, for cpf\n",stringNodeType(vpnode->_nodeType));
		nearPlane = DEFAULT_NEARPLANE;
		farPlane = DEFAULT_FARPLANE;
		backgroundPlane = DEFAULT_BACKGROUNDPLANE;
		return;
	}	

	if (rootNode == NULL) {
		return; /* nothing to display yet */
	}

	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, MM);

		#ifdef VERBOSE
		printf ("rootNode extents x: %4.2f %4.2f  y:%4.2f %4.2f z: %4.2f %4.2f\n",
				rootNode->EXTENT_MAX_X, rootNode->EXTENT_MIN_X,
				rootNode->EXTENT_MAX_Y, rootNode->EXTENT_MIN_Y,
				rootNode->EXTENT_MAX_Z, rootNode->EXTENT_MIN_Z);
		#endif
		/* make up 8 vertices for our bounding box, and place them within our view */
		moveAndRotateThisPoint(&bboxPoints[0], rootNode->EXTENT_MIN_X, rootNode->EXTENT_MIN_Y, rootNode->EXTENT_MIN_Z,MM);
		moveAndRotateThisPoint(&bboxPoints[1], rootNode->EXTENT_MIN_X, rootNode->EXTENT_MIN_Y, rootNode->EXTENT_MAX_Z,MM);
		moveAndRotateThisPoint(&bboxPoints[2], rootNode->EXTENT_MIN_X, rootNode->EXTENT_MAX_Y, rootNode->EXTENT_MIN_Z,MM);
		moveAndRotateThisPoint(&bboxPoints[3], rootNode->EXTENT_MIN_X, rootNode->EXTENT_MAX_Y, rootNode->EXTENT_MAX_Z,MM);
		moveAndRotateThisPoint(&bboxPoints[4], rootNode->EXTENT_MAX_X, rootNode->EXTENT_MIN_Y, rootNode->EXTENT_MIN_Z,MM);
		moveAndRotateThisPoint(&bboxPoints[5], rootNode->EXTENT_MAX_X, rootNode->EXTENT_MIN_Y, rootNode->EXTENT_MAX_Z,MM);
		moveAndRotateThisPoint(&bboxPoints[6], rootNode->EXTENT_MAX_X, rootNode->EXTENT_MAX_Y, rootNode->EXTENT_MIN_Z,MM);
		moveAndRotateThisPoint(&bboxPoints[7], rootNode->EXTENT_MAX_X, rootNode->EXTENT_MAX_Y, rootNode->EXTENT_MAX_Z,MM);
	
		for (ci=0; ci<8; ci++) {
			#ifdef VERBOSE
			printf ("moved bbox node %d is %4.2f %4.2f %4.2f\n",ci,bboxPoints[ci].x, bboxPoints[ci].y, bboxPoints[ci].z);
			#endif
	
			if (-(bboxPoints[ci].z) > cfp) cfp = -(bboxPoints[ci].z);
			if (-(bboxPoints[ci].z) < cnp) cnp = -(bboxPoints[ci].z);
		}

	/* lets bound check here, both must be positive, and farPlane more than DEFAULT_NEARPLANE */
	/* because we may be navigating towards the shapes, we give the nearPlane a bit of room, otherwise
	   we might miss part of the geometry that comes closest to us */
	cnp = cnp/2.0;
	if (cnp<DEFAULT_NEARPLANE) cnp = DEFAULT_NEARPLANE;

	if (cfp<1.0) cfp = 1.0;	
	/* if we are moving, or if we have something with zero depth, floating point calculation errors could
	   give us a geometry that is at (or, over) the far plane. Eg, tests/49.wrl, where we have Text nodes,
	   can give us this issue; so lets give us a bit of leeway here, too */
	cfp *= 1.25;


	#ifdef VERBOSE
	printf ("cnp %lf cfp before leaving room for Background %lf\n",cnp,cfp);
	#endif

	/* lets use these values; leave room for a Background or TextureBackground node here */
	nearPlane = cnp; 
	/* backgroundPlane goes between the farthest geometry, and the farPlane */
	if (background_stack[0]!= 0) {
		farPlane = cfp * 10.0;
		backgroundPlane = cfp*5.0;
	} else {
		farPlane = cfp;
		backgroundPlane = cfp; /* just set it to something */
	}
}

void doglClearColor() {
	FW_GL_CLEAR_COLOR(cc_red, cc_green, cc_blue, cc_alpha);
	cc_changed = FALSE;
}

/* did we have a TextureTransform in the Appearance node? */
void start_textureTransform (struct X3D_Node *textureNode, int ttnum) {

	/* stuff common to all textureTransforms - gets undone at end_textureTransform */
	FW_GL_MATRIX_MODE(GL_TEXTURE);
       	/* done in RenderTextures now FW_GL_ENABLE(GL_TEXTURE_2D); */
	FW_GL_LOAD_IDENTITY();

	/* is this a simple TextureTransform? */
	if (textureNode->_nodeType == NODE_TextureTransform) {
		struct X3D_TextureTransform  *ttt = (struct X3D_TextureTransform *) textureNode;
		/*  Render transformations according to spec.*/
        	FW_GL_TRANSLATE_F(-((ttt->center).c[0]),-((ttt->center).c[1]), 0);		/*  5*/
        	FW_GL_SCALE_F(((ttt->scale).c[0]),((ttt->scale).c[1]),1);			/*  4*/
        	FW_GL_ROTATE_RADIANS(ttt->rotation,0,0,1);					/*  3*/
        	FW_GL_TRANSLATE_F(((ttt->center).c[0]),((ttt->center).c[1]), 0);		/*  2*/
        	FW_GL_TRANSLATE_F(((ttt->translation).c[0]), ((ttt->translation).c[1]), 0);	/*  1*/

	/* is this a MultiTextureTransform? */
	} else  if (textureNode->_nodeType == NODE_MultiTextureTransform) {
		struct X3D_MultiTextureTransform *mtt = (struct X3D_MultiTextureTransform *) textureNode;
		if (ttnum < mtt->textureTransform.n) {
			struct X3D_TextureTransform *ttt = (struct X3D_TextureTransform *) mtt->textureTransform.p[ttnum];
			/* is this a simple TextureTransform? */
			if (ttt->_nodeType == NODE_TextureTransform) {
				/*  Render transformations according to spec.*/
        			FW_GL_TRANSLATE_F(-((ttt->center).c[0]),-((ttt->center).c[1]), 0);		/*  5*/
        			FW_GL_SCALE_F(((ttt->scale).c[0]),((ttt->scale).c[1]),1);			/*  4*/
        			FW_GL_ROTATE_RADIANS(ttt->rotation,0,0,1);					/*  3*/
        			FW_GL_TRANSLATE_F(((ttt->center).c[0]),((ttt->center).c[1]), 0);		/*  2*/
        			FW_GL_TRANSLATE_F(((ttt->translation).c[0]), ((ttt->translation).c[1]), 0);	/*  1*/
			} else {
				printf ("MultiTextureTransform expected a textureTransform for texture %d, got %d\n",
					ttnum, ttt->_nodeType);
			}
		} else {
			printf ("not enough textures in MultiTextureTransform....\n");
		}

	} else if (textureNode->_nodeType == NODE_VRML1_Texture2Transform) {
		struct X3D_VRML1_Texture2Transform  *ttt = (struct X3D_VRML1_Texture2Transform *) textureNode;
		/*  Render transformations according to spec.*/
        	FW_GL_TRANSLATE_F(-((ttt->center).c[0]),-((ttt->center).c[1]), 0);		/*  5*/
        	FW_GL_SCALE_F(((ttt->scaleFactor).c[0]),((ttt->scaleFactor).c[1]),1);			/*  4*/
        	FW_GL_ROTATE_F(ttt->rotation,0,0,1);						/*  3*/
        	FW_GL_TRANSLATE_F(((ttt->center).c[0]),((ttt->center).c[1]), 0);		/*  2*/
        	FW_GL_TRANSLATE_F(((ttt->translation).c[0]), ((ttt->translation).c[1]), 0);	/*  1*/
	} else {
		printf ("expected a textureTransform node, got %d\n",textureNode->_nodeType);
	}
	FW_GL_MATRIX_MODE(GL_MODELVIEW);
}

/* did we have a TextureTransform in the Appearance node? */
void end_textureTransform (void) {
	FW_GL_MATRIX_MODE(GL_TEXTURE);
	FW_GL_LOAD_IDENTITY();
	FW_GL_MATRIX_MODE(GL_MODELVIEW);
}


/**
 *   initializa_GL: initialize GLEW (->rdr caps) and OpenGL initial state
 */
bool initialize_GL()
{
#if defined (TARGET_AQUA) && !defined(IPHONE)
		/* printf("initialize gl, myglobalcontext is %u", myglobalContext); */
                CGLSetCurrentContext(myglobalContext);
#endif

	initialize_rdr_caps();

	initialize_rdr_functions();

	PRINT_GL_ERROR_IF_ANY("initialize_GL start");

	/* lets make sure everything is sync'd up */
#if defined(TARGET_X11) || defined(TARGET_MOTIF)
	XFlush(Xdpy);
#endif


#ifndef USE_OLD_OPENGL
	/* are we using shaders for Appearance, etc? (OpenGL-ES) */
	if (global_use_shaders_when_possible) {
/*
	we choose the shader based on the following, so get all the shaders loaded 

	noAppearanceNoMaterialShader:			no lighting, no appearance.
        noLightNoTextureAppearanceShader:		no lights, no textures, only emissive lights.
        genericHeadlightNoTextureAppearanceShader:	headlight on, no textures.
        multiLightNoTextureAppearanceShader:		more than headlight on; no textures.
        headlightOneTextureAppearanceShader:		headlight, no othere lights, one texture.
        headlightMultiTextureAppearanceShader:		headlight, no other light, multi-texture.
        multiLightMultiTextureAppearanceShader:		everything including kitchen sink.
*/

		getAppearanceShader(&rdr_caps.shaderArrays[genericHeadlightNoTextureAppearanceShader], "/Users/johns/Desktop/shaderReplacement/genericHeadlightNoTextureAppearanceShader");
/* do these later

		getAppearanceShader(&rdr_caps.shaderArrays[noLightNoTextureAppearanceShader], "/Users/johns/Desktop/shaderReplacement/noLightNoTextureAppearanceShader");
		getAppearanceShader(&rdr_caps.shaderArrays[noAppearanceNoMaterialShader], "/Users/johns/Desktop/shaderReplacement/noAppearanceNoMaterialShader");
		getAppearanceShader(&rdr_caps.shaderArrays[multiLightNoTextureAppearanceShader], "/Users/johns/Desktop/shaderReplacement/multiLightNoTextureAppearanceShader");
		getAppearanceShader(&rdr_caps.shaderArrays[headlightOneTextureAppearanceShader], "/Users/johns/Desktop/shaderReplacement/headlightOneTextureAppearanceShader");
		getAppearanceShader(&rdr_caps.shaderArrays[headlightMultiTextureAppearanceShader], "/Users/johns/Desktop/shaderReplacement/headlightMultiTextureAppearanceShader");
		getAppearanceShader(&rdr_caps.shaderArrays[multiLightMultiTextureAppearanceShader], "/Users/johns/Desktop/shaderReplacement/multiLightMultiTextureAppearanceShader");
*/


		/* tell the child_Shape routine to use these shaders, if there is not a user-specified shader */
		rdr_caps.haveGenericAppearanceShader = TRUE;
	} else {
		/* put non-shader stuff here eventually */
	}
#endif /* USE_OLD_OPENGL */

	/* Set up the OpenGL state. This'll get overwritten later... */
	FW_GL_CLEAR_DEPTH(1.0);
	FW_GL_CLEAR_COLOR((float)0.0, (float)0.0, (float)1.0, (float)0.0);
	FW_GL_MATRIX_MODE(GL_PROJECTION);
	FW_GL_LOAD_IDENTITY();
	FW_GL_MATRIX_MODE(GL_MODELVIEW);

	/* Configure OpenGL for our uses. */
        FW_GL_ENABLECLIENTSTATE(GL_VERTEX_ARRAY);
        FW_GL_ENABLECLIENTSTATE(GL_NORMAL_ARRAY);
	FW_GL_CLEAR_COLOR(cc_red, cc_green, cc_blue, cc_alpha);
	FW_GL_SHADEMODEL(GL_SMOOTH);
	FW_GL_DEPTHFUNC(GL_LEQUAL);
	//FW_GL_DEPTHFUNC(GL_LESS);
	FW_GL_ENABLE(GL_DEPTH_TEST);
	FW_GL_LINEWIDTH(gl_linewidth);
	FW_GL_POINTSIZE(gl_linewidth);

	FW_GL_HINT(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	FW_GL_ENABLE (GL_RESCALE_NORMAL);

	/*
     * JAS - ALPHA testing for textures - right now we just use 0/1 alpha
     * JAS   channel for textures - true alpha blending can come when we sort
     * JAS   nodes.
	 */

	FW_GL_ENABLE(GL_BLEND);
	FW_GL_BLENDFUNC(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	//FW_GL_BLENDFUNC(GL_ONE, GL_ONE); wierd colours
	//FW_GL_BLENDFUNC(GL_SRC, GL_ONE_MINUS_SRC);

	//if this is enabled, VisibilitySensors must have an alpha of greater than 0.0
	//FW_GL_ENABLE(GL_ALPHA_TEST);
	//FW_GL_ALPHAFUNC(GL_GREATER, 0); 

	FW_GL_CLEAR(GL_COLOR_BUFFER_BIT);

	FW_GL_TEXENVI(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);


	FW_GL_ENABLE(GL_NORMALIZE);

	/* for textured appearance add specular highlights as a separate secondary color
	   redbook p.270, p.455 and http://www.gamedev.net/reference/programming/features/oglch9excerpt/

	   if we don't have texture we can disable this (less computation)...
	   but putting this here is already a saving ;)...
	*/

	/* keep track of light states; initial turn all lights off except for headlight */
	initializeLightTables();

	/* ensure state of GL_CULL_FACE */
	CULL_FACE_INITIALIZE;

	FW_GL_PIXELSTOREI(GL_UNPACK_ALIGNMENT,1);
	FW_GL_PIXELSTOREI(GL_PACK_ALIGNMENT,1);

	do_shininess(GL_FRONT_AND_BACK,(float) 0.2);

	return TRUE;
}

void BackEndClearBuffer(int which) {
	if(which == 2) {
		FW_GL_CLEAR(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	} else { 
		if(which==1) {
			FW_GL_CLEAR(GL_DEPTH_BUFFER_BIT);
		}
	}
}

/* turn off all non-headlight lights; will turn them on if required. */
void BackEndLightsOff() {
	int i;
	for (i=0; i<7; i++) {
		lightState(i, FALSE);
	}
}


#ifndef USE_OLD_OPENGL
/* OpenGL perform matrix state here */
#define MAX_LARGE_MATRIX_STACK 32	/* depth of stacks */
#define MAX_SMALL_MATRIX_STACK 2	/* depth of stacks */
#define MATRIX_SIZE 16		/* 4 x 4 matrix */
typedef double MATRIX4[MATRIX_SIZE];

static MATRIX4 FW_ModelView[MAX_LARGE_MATRIX_STACK];
static MATRIX4 FW_ProjectionView[MAX_SMALL_MATRIX_STACK];
static MATRIX4 FW_TextureView[MAX_SMALL_MATRIX_STACK];
 
static int modelviewTOS = 0;
static int projectionviewTOS = 0;
static int textureviewTOS = 0;

static int whichMode = GL_MODELVIEW;
static double *currentMatrix = FW_ModelView;


void fw_glMatrixMode(GLint mode) {
	whichMode = mode;
	/*
	switch (whichMode) {
		case GL_PROJECTION: printf ("glMatrixMode(GL_PROJECTION)\n"); break;
		case GL_MODELVIEW: printf ("glMatrixMode(GL_MODELVIEW)\n"); break;
		case GL_TEXTURE: printf ("glMatrixMode(GL_TEXTURE)\n"); break;
	}
	*/

	/* printf ("fw_glMatrixMode %d\n",mode); */
	switch (whichMode) {
		case GL_PROJECTION: currentMatrix = &FW_ProjectionView[projectionviewTOS]; break;
		case GL_MODELVIEW: currentMatrix = &FW_ModelView[modelviewTOS]; break;
		case GL_TEXTURE: currentMatrix = &FW_TextureView[textureviewTOS]; break;
		default: printf ("invalid mode sent in it is %d, expected one of %d %d %d\n",whichMode, GL_PROJECTION,GL_MODELVIEW,GL_TEXTURE);
	}

	glMatrixMode(mode); /* JAS */
}

void fw_glLoadIdentity(void) {
	/* printf ("glLoadIdentity()\n"); */

	loadIdentityMatrix(currentMatrix);
	if (!global_use_shaders_when_possible)
 		fw_glLoadMatrixd(currentMatrix); 
}

#define PUSHMAT(a,b,c,d) case a: b++; if (b>=c) {b=c-1; printf ("stack overflow, whichmode %d\n",whichMode); } \
		memcpy ((void *)d[b], (void *)d[b-1],sizeof(double)*16); currentMatrix = d[b]; break;

void fw_glPushMatrix(void) {
	/* glPushMatrix(); */
	switch (whichMode) {
		PUSHMAT (GL_PROJECTION,projectionviewTOS,MAX_SMALL_MATRIX_STACK,FW_ProjectionView)
		PUSHMAT (GL_MODELVIEW,modelviewTOS,MAX_LARGE_MATRIX_STACK,FW_ModelView)
		PUSHMAT (GL_TEXTURE,textureviewTOS,MAX_SMALL_MATRIX_STACK,FW_TextureView)
		default :printf ("wrong mode in popMatrix\n");
	}
	/* if (whichMode == GL_PROJECTION) { printf ("	fw_glPushMatrix tos now %d\n",projectionviewTOS); }  */

	if (!global_use_shaders_when_possible)
 		fw_glLoadMatrixd(currentMatrix); 
#undef PUSHMAT
}

#define POPMAT(a,b,c) case a: b--; if (b<0) {b=0;printf ("popMatrix, stack underflow, whichMode %d\n",whichMode);} currentMatrix = c[b]; break;

void fw_glPopMatrix(void) {
	/* glPopMatrix();  */
	switch (whichMode) {
		POPMAT(GL_PROJECTION,projectionviewTOS,FW_ProjectionView)
		POPMAT(GL_MODELVIEW,modelviewTOS,FW_ModelView)
		POPMAT (GL_TEXTURE,textureviewTOS,FW_TextureView)
		default :printf ("wrong mode in popMatrix\n");
	}
	/* if (whichMode == GL_PROJECTION) { printf ("	fw_glPopMatrix tos now %d\n",projectionviewTOS); } */

	if (!global_use_shaders_when_possible)
 		fw_glLoadMatrixd(currentMatrix); 
}
#undef POPMAT


void fw_glTranslated(double x, double y, double z) {
	//printf ("fw_glTranslated %lf %lf %lf\n",x,y,z);
	//printf ("translated, currentMatrix %p\n",currentMatrix);

	currentMatrix[12] = currentMatrix[0] * x + currentMatrix[4] * y + currentMatrix[8]  * z + currentMatrix[12];
	currentMatrix[13] = currentMatrix[1] * x + currentMatrix[5] * y + currentMatrix[9]  * z + currentMatrix[13];
	currentMatrix[14] = currentMatrix[2] * x + currentMatrix[6] * y + currentMatrix[10] * z + currentMatrix[14];
	currentMatrix[15] = currentMatrix[3] * x + currentMatrix[7] * y + currentMatrix[11] * z + currentMatrix[15];

	if (!global_use_shaders_when_possible)
 		fw_glLoadMatrixd(currentMatrix); 
}

void fw_glTranslatef(float x, float y, float z) {
	//printf ("fw_glTranslatef %f %f %f\n",x,y,z);
	currentMatrix[12] = currentMatrix[0] * x + currentMatrix[4] * y + currentMatrix[8]  * z + currentMatrix[12];
	currentMatrix[13] = currentMatrix[1] * x + currentMatrix[5] * y + currentMatrix[9]  * z + currentMatrix[13];
	currentMatrix[14] = currentMatrix[2] * x + currentMatrix[6] * y + currentMatrix[10] * z + currentMatrix[14];
	currentMatrix[15] = currentMatrix[3] * x + currentMatrix[7] * y + currentMatrix[11] * z + currentMatrix[15];

	if (!global_use_shaders_when_possible)
 		fw_glLoadMatrixd(currentMatrix); 
}

/* perform rotation, assuming that the angle is in radians. */
void fw_glRotateRad (double angle, double x, double y, double z) {
	MATRIX4 myMat;
	double mag;

	loadIdentityMatrix (myMat);

	/* FIXME - any way we can ensure that the angles are normalized? */
	mag =  x*x + y*y + z*z; 

	/* bounds check - the axis is invalid. */
	if (APPROX(mag,0.00)) {
		return;
	}

	/* bounds check - angle is zero, no rotation happening here */
	if (APPROX(angle,0.0)) {
		return;
	}

	if (!APPROX(mag,1.0)) {
		struct point_XYZ in; struct point_XYZ out;
		in.x = x; in.y = y, in.z = z;
		vecnormal(&out,&in);
		x = out.x; y = out.y; z = out.z;
	}


	matrotate(myMat,angle,x,y,z); 
	matmultiply(currentMatrix,currentMatrix,myMat); 
	if (!global_use_shaders_when_possible)
 		fw_glLoadMatrixd(currentMatrix); 
}

/* perform the rotation, assuming that the angle is in degrees */
void fw_glRotated (double angle, double x, double y, double z) {
	MATRIX4 myMat;
	double mag;
	double radAng;

	/* convert angle from degrees to radians */
	/* FIXME - must try and make up a rotate-radians call to get rid of these incessant conversions */
	radAng = angle * 3.1415926536/ 180.0;

	loadIdentityMatrix (myMat);

	/* FIXME - any way we can ensure that the angles are normalized? */
	mag =  x*x + y*y + z*z; 

	/* bounds check - the axis is invalid. */
	if (APPROX(mag,0.00)) {
		return;
	}

	/* bounds check - angle is zero, no rotation happening here */
	if (APPROX(angle,0.0)) {
		return;
	}

	if (!APPROX(mag,1.0)) {
		struct point_XYZ in; struct point_XYZ out;
		in.x = x; in.y = y, in.z = z;
		vecnormal(&out,&in);
		x = out.x; y = out.y; z = out.z;
	}


	/* are the axis close to zero? */
	if (mag < 0.001) {
		return;
	}
	matrotate(myMat,radAng,x,y,z); 
	matmultiply(currentMatrix,currentMatrix,myMat); 
	if (!global_use_shaders_when_possible)
 		fw_glLoadMatrixd(currentMatrix); 
}

void fw_glRotatef (float a, float x, float y, float z) {
	fw_glRotated((double)a, (double)x, (double)y, (double)z);
}

void fw_glScaled (double x, double y, double z) {

//	printf ("glScaled(%5.4lf %5.4lf %5.4lf)\n",x,y,z);

	currentMatrix[0] *= x;   currentMatrix[4] *= y;   currentMatrix[8]  *= z;
	currentMatrix[1] *= x;   currentMatrix[5] *= y;   currentMatrix[9]  *= z;
	currentMatrix[2] *= x;   currentMatrix[6] *= y;   currentMatrix[10] *= z;
	currentMatrix[3] *= x;   currentMatrix[7] *= y;   currentMatrix[11] *= z;
	if (!global_use_shaders_when_possible)
 		fw_glLoadMatrixd(currentMatrix); 
}

void fw_glScalef (float x, float y, float z) {

//	printf ("glScalef(%5.4f %5.4f %5.4f)\n",x,y,z);

	currentMatrix[0] *= x;   currentMatrix[4] *= y;   currentMatrix[8]  *= z;
	currentMatrix[1] *= x;   currentMatrix[5] *= y;   currentMatrix[9]  *= z;
	currentMatrix[2] *= x;   currentMatrix[6] *= y;   currentMatrix[10] *= z;
	currentMatrix[3] *= x;   currentMatrix[7] *= y;   currentMatrix[11] *= z;
	if (!global_use_shaders_when_possible)
 		fw_glLoadMatrixd(currentMatrix); 
}

void fw_glGetDoublev (int ty, double *mat, char *fn, int ln) {
	double *dp;
/*
	switch (ty) {
		case GL_PROJECTION_MATRIX: printf ("getDoublev(GL_PROJECTION_MATRIX)\n"); break;
		case GL_MODELVIEW_MATRIX: printf ("getDoublev(GL_MODELVIEW_MATRIX)\n"); break;
		case GL_TEXTURE_MATRIX: printf ("getDoublev(GL_TEXTURE_MATRIX)\n"); break;
	}
*/

	switch (ty) {
		case GL_PROJECTION_MATRIX: dp = FW_ProjectionView[projectionviewTOS]; break;
		case GL_MODELVIEW_MATRIX: dp = FW_ModelView[modelviewTOS]; break;
#ifndef IPHONE
		case GL_TEXTURE_MATRIX: dp = FW_TextureView[textureviewTOS]; break;
#endif
		default: { 
			loadIdentityMatrix(mat); 
#ifndef IPHONE
		printf ("invalid mode sent in it is %d, expected one of %d %d %d\n",ty,GL_PROJECTION_MATRIX,GL_MODELVIEW_MATRIX,GL_TEXTURE_MATRIX);
#endif
			return;}
	}
	memcpy((void *)mat, (void *) dp, sizeof (double) * MATRIX_SIZE);

/*
{ int i;
	for (i=0; i<16; i++) {
		printf ("%4.3lf ",dp[i]);

	}
	printf ("at %s:%d\n",fn,ln);
}
*/
	
}

#endif /* #define USE_OLD_OPENGL */

/* for Sarah's front end - should be removed sometime... */
void kill_rendering() {
/* printf ("kill_rendering called...\n"); */
}


/* if we have a ReplaceWorld style command, we have to remove the old world. */
/* NOTE: There are 2 kinds of of replaceworld commands - sometimes we have a URL
   (eg, from an Anchor) and sometimes we have a list of nodes (from a Javascript
   replaceWorld, for instance). The URL ones can really replaceWorld, the node
   ones, really, it's just replace the rootNode children, as WE DO NOT KNOW
   what the user has programmed, and what nodes are (re) used in the Scene Graph */

void kill_oldWorld(int kill_EAI, int kill_JavaScript, char *file, int line) {
	int i;
	#ifndef AQUA
        char mystring[20];
	#endif

#ifdef VERBOSE
	printf ("kill 1 myThread %u displayThread %u\n",pthread_self(), DispThrd);
#ifdef _MSC_VER
	if (pthread_self().p != DispThrd.p ) {
#else
	if (pthread_self() != DispThrd) {
#endif
		ConsoleMessage ("kill_oldWorld must run in the displayThread called at %s:%d\n",file,line);
		return;
	}
#endif

	/* get rid of sensor events */
	resetSensorEvents();


	/* make the root_res equal NULL - this throws away all old resource info */
	/*
		if (root_res != NULL) {
			printf ("root_res %p has the following...\n",root_res);
			resource_dump(root_res);
		}else {printf ("root_res is null, no need to dump\n");}
	*/

	root_res = NULL;

	/* mark all rootNode children for Dispose */
	for (i=0; i<rootNode->children.n; i++) {
		markForDispose(rootNode->children.p[i], TRUE);
	}

	/* stop rendering */
	rootNode->children.n = 0;

	/* close the Console Message system, if required. */
	closeConsoleMessage();

	/* occlusion testing - zero total count, but keep MALLOC'd memory around */
	zeroOcclusion();

	/* clock events - stop them from ticking */
	kill_clockEvents();


	/* kill DEFS, handles */
	EAI_killBindables();
	kill_bindables();
	killKeySensorNodeList();


	/* stop routing */
	kill_routing();

	/* tell the statusbar that it needs to reinitialize */
	kill_status();

	/* free textures */
/*
	kill_openGLTextures();
*/
	
	/* free scripts */
	kill_javascript();


	/* free EAI */
	if (kill_EAI) {
	       	shutdown_EAI();
	}

	#ifndef AQUA
		sprintf (mystring, "QUIT");
		Sound_toserver(mystring);
	#endif


	/* reset any VRML Parser data */
	if (globalParser != NULL) {
		parser_destroyData(globalParser);
		globalParser = NULL;
	}


	/* tell statusbar that we have none */
	viewer_default();
	setMenuStatus("NONE");
}

/* for verifying that a memory pointer exists */
int checkNode(struct X3D_Node *node, char *fn, int line) {
	int tc;

	if (node == NULL) {
		printf ("checkNode, node is NULL at %s %d\n",fn,line);
		return FALSE;
	}

	if (node == forgottenNode) return TRUE;


	LOCK_MEMORYTABLE
	for (tc = 0; tc< nextEntry; tc++)
		if (memoryTable[tc] == node) {
			if (node->referenceCount > 0) {
			UNLOCK_MEMORYTABLE
			return TRUE;
		}
	}


	printf ("checkNode: did not find %p in memory table at i%s %d\n",node,fn,line);

	UNLOCK_MEMORYTABLE
	return FALSE;
}


/*keep track of node created*/
void registerX3DNode(struct X3D_Node * tmp){	
	LOCK_MEMORYTABLE
	/* printf("nextEntry=%d	",nextEntry); printf("tableIndexSize=%d \n",tableIndexSize); */
	/*is table to small give us some leeway in threads */
	if (nextEntry >= (tableIndexSize-10)){
		/*is table exist*/	
		if (tableIndexSize <= INT_ID_UNDEFINED){
			createdMemoryTable();		
		} else {
			increaseMemoryTable();
		}
	}
	/*adding node in table*/	
	/* printf ("registerX3DNode, adding %x at %d\n",tmp,nextEntry); */
	memoryTable[nextEntry] = tmp;
	nextEntry+=1;
	UNLOCK_MEMORYTABLE
}

/*We don't register the first node created for reload reason*/
void doNotRegisterThisNodeForDestroy(struct X3D_Node * nodePtr){
	LOCK_MEMORYTABLE
	if(nodePtr==(memoryTable[nextEntry-1])){
		nextEntry-=1;
		forgottenNode = nodePtr;
	}	
	UNLOCK_MEMORYTABLE
}

/*creating node table*/
void createdMemoryTable(){
	int count;

	tableIndexSize=50;
	memoryTable = MALLOC(tableIndexSize * sizeof(struct X3D_Node*));

	/* initialize this to a known state */
	for (count=0; count < tableIndexSize; count++) {
		memoryTable[count] = NULL;
	}
}

/*making table bigger*/
void increaseMemoryTable(){
	int count;
	int oldhigh;

	oldhigh = tableIndexSize;

	
	tableIndexSize*=2;
	memoryTable = REALLOC (memoryTable, tableIndexSize * sizeof(struct X3D_Node*) );

	/* initialize this to a known state */
	for (count=oldhigh; count < tableIndexSize; count++) {
		memoryTable[count] = NULL;
	}
	/* printf("increasing memory table=%d\n",tableIndexSize); */
}


/* sort children - use bubble sort with early exit flag */
/* we use this for z buffer rendering; drawing scene in correct z-buffer order */
/* we ONLY sort if:
	1) the node has changed - this is the "needsCompiling" field;
	2) the number of children has changed - again, the "needsCompiling" flag should be set,
		but we enforce it;
	3) the first pass shows that nodes are out of order 
*/

static void sortChildren (struct Multi_Node *ch, struct Multi_Node *sortedCh, int needsCompiling) {
	int i,j;
	int nc;
	int noswitch;
	struct X3D_Node *a, *b, *c;

	/* simple, inefficient bubble sort */
	/* this is a fast sort when nodes are already sorted;
	   may wish to go and "QuickSort" or so on, when nodes
	   move around a lot. (Bubblesort is bad when nodes
	   have to be totally reversed) */

	nc = ch->n;

	/* has this changed size? */
	if (ch->n != sortedCh->n) {
		FREE_IF_NZ(sortedCh->p);
		sortedCh->p = MALLOC (sizeof (void *) * ch->n);
		needsCompiling = TRUE; /* force this change; should be
			set anyway */
	}

	/* copy the nodes over; we will sort the sorted list */

	if (needsCompiling) {
		memcpy (sortedCh->p,ch->p,sizeof (void *) * nc);
		sortedCh->n = nc;
	}

	#ifdef VERBOSE
	printf ("sortChildren start, %d, chptr %u\n",nc,ch);
	#endif

	if (nc < 2) return;

	for(i=0; i<nc; i++) {
		noswitch = TRUE;
		for (j=(nc-1); j>i; j--) {
			/* printf ("comparing %d %d\n",i,j); */
			a = X3D_NODE(sortedCh->p[j-1]);
			b = X3D_NODE(sortedCh->p[j]);

			/* check to see if a child is NULL - if so, skip it */
			if (a && b) {
				if (a->_dist > b->_dist) {
					/* printf ("sortChildren at %lf, have to switch %d %d\n",TickTime,i,j);  */
					c = a;
					sortedCh->p[j-1] = b;
					sortedCh->p[j] = c;
					noswitch = FALSE;
				}
			}	
		}
		/* did we have a clean run? */
		if (noswitch) {
			break;
		}
	}
	
	#ifdef VERBOSE
	printf ("sortChildren returning.\n");
	for(i=0; i<nc; i++) {
		b = sortedCh->p[i];
		if (b)
			printf ("child %d %u %f %s",i,b,b->_dist,stringNodeType(b->_nodeType));
		else
			printf ("no child %d", i);
		b = ch->p[i];
		printf (" unsorted %u\n",b);
	}
	#endif
}

/* zero the Visibility flag in all nodes */
void zeroVisibilityFlag(void) {
	struct X3D_Node* node;
	int i;
	int ocnum;

	ocnum=-1;

	
	/* do not bother doing this if the inputThread is active. */
	if (isinputThreadParsing()) return;
 	LOCK_MEMORYTABLE

	/* do we have GL_ARB_occlusion_query, or are we still parsing Textures? */
	if ((OccFailed) || isTextureParsing()) {
		/* if we have textures still loading, display all the nodes, so that the textures actually
		   get put into OpenGL-land. If we are not texture parsing... */
		/* no, we do not have GL_ARB_occlusion_query, just tell every node that it has visible children 
		   and hope that, sometime, the user gets a good computer graphics card */
		for (i=0; i<nextEntry; i++){		
			node = memoryTable[i];	
			if (node != NULL) {
				node->_renderFlags = node->_renderFlags | VF_hasVisibleChildren;
			}
		}	
	} else {
		/* we do... lets zero the hasVisibleChildren flag */
		for (i=0; i<nextEntry; i++){		
			node = memoryTable[i];		
			if (node != NULL) {
		
			#ifdef OCCLUSIONVERBOSE
			if (((node->_renderFlags) & VF_hasVisibleChildren) != 0) {
			printf ("zeroVisibility - %d is a %s, flags %x\n",i,stringNodeType(node->_nodeType), (node->_renderFlags) & VF_hasVisibleChildren); 
			}
			#endif

			node->_renderFlags = node->_renderFlags & (0xFFFF^VF_hasVisibleChildren);
			}
	
		}		
	}
	UNLOCK_MEMORYTABLE
}

/* go through the linear list of nodes, and do "special things" for special nodes, like
   Sensitive nodes, Viewpoint nodes, ... */

#define CMD(TTT,YYY) \
	/* printf ("nt %s change %d ichange %d\n",stringNodeType(X3D_NODE(YYY)->_nodeType),X3D_NODE(YYY)->_change, X3D_NODE(YYY)->_ichange); */ \
	if (NODE_NEEDS_COMPILING) compile_Metadata##TTT((struct X3D_Metadata##TTT *) YYY)

#define BEGIN_NODE(thistype) case NODE_##thistype:
#define END_NODE break;

#define SIBLING_SENSITIVE(thistype) \
			/* make Sensitive */ \
			if (((struct X3D_##thistype *)node)->enabled) { \
				nParents = ((struct X3D_##thistype *)node)->_nparents; \
				pp = (((struct X3D_##thistype *)node)->_parents); \
			}  

#define ANCHOR_SENSITIVE(thistype) \
			/* make THIS Sensitive - most nodes make the parents sensitive, Anchors have children...*/ \
			anchorPtr = (struct X3D_Anchor *)node;
#ifdef xxx
			nParents = ((struct X3D_Anchor *)node)->children.n; pp = ((struct X3D_Anchor *)node)->children.p; 
#endif

#ifdef VIEWPOINT
#undef VIEWPOINT /* defined for the EAI,SAI, does not concern us uere */
#endif
#define VIEWPOINT(thistype) \
			setBindPtr = (uintptr_t *) ((uintptr_t)(node) + offsetof (struct X3D_##thistype, set_bind));

#define CHILDREN_NODE(thistype) \
			addChildren = NULL; removeChildren = NULL; \
			offsetOfChildrenPtr = offsetof (struct X3D_##thistype, children); \
			if (((struct X3D_##thistype *)node)->addChildren.n > 0) { \
				addChildren = &((struct X3D_##thistype *)node)->addChildren; \
				childrenPtr = &((struct X3D_##thistype *)node)->children; \
			} \
			if (((struct X3D_##thistype *)node)->removeChildren.n > 0) { \
				removeChildren = &((struct X3D_##thistype *)node)->removeChildren; \
				childrenPtr = &((struct X3D_##thistype *)node)->children; \
			} 

#define CHILDREN_SWITCH_NODE(thistype) \
			addChildren = NULL; removeChildren = NULL; \
			offsetOfChildrenPtr = offsetof (struct X3D_##thistype, choice); \
			if (((struct X3D_##thistype *)node)->addChildren.n > 0) { \
				addChildren = &((struct X3D_##thistype *)node)->addChildren; \
				childrenPtr = &((struct X3D_##thistype *)node)->choice; \
			} \
			if (((struct X3D_##thistype *)node)->removeChildren.n > 0) { \
				removeChildren = &((struct X3D_##thistype *)node)->removeChildren; \
				childrenPtr = &((struct X3D_##thistype *)node)->choice; \
			} 

#define CHILDREN_LOD_NODE \
			addChildren = NULL; removeChildren = NULL; \
			offsetOfChildrenPtr = offsetof (struct X3D_LOD, children); \
			if (X3D_LODNODE(node)->addChildren.n > 0) { \
				addChildren = &X3D_LODNODE(node)->addChildren; \
				if (X3D_LODNODE(node)->__isX3D == 0) childrenPtr = &X3D_LODNODE(node)->level; \
				else childrenPtr = &X3D_LODNODE(node)->children; \
			} \
			if (X3D_LODNODE(node)->removeChildren.n > 0) { \
				removeChildren = &X3D_LODNODE(node)->removeChildren; \
				if (X3D_LODNODE(node)->__isX3D == 0) childrenPtr = &X3D_LODNODE(node)->level; \
				else childrenPtr = &X3D_LODNODE(node)->children; \
			}

#define EVIN_AND_FIELD_SAME(thisfield, thistype) \
			if ((((struct X3D_##thistype *)node)->set_##thisfield.n) > 0) { \
				((struct X3D_##thistype *)node)->thisfield.n = 0; \
				FREE_IF_NZ (((struct X3D_##thistype *)node)->thisfield.p); \
				((struct X3D_##thistype *)node)->thisfield.p = ((struct X3D_##thistype *)node)->set_##thisfield.p; \
				((struct X3D_##thistype *)node)->thisfield.n = ((struct X3D_##thistype *)node)->set_##thisfield.n; \
				((struct X3D_##thistype *)node)->set_##thisfield.n = 0; \
				((struct X3D_##thistype *)node)->set_##thisfield.p = NULL; \
			} 

/* just tell the parent (a grouping node) that there is a locally scoped light as a child */
/* do NOT send this up the scenegraph! */
#define LOCAL_LIGHT_PARENT_FLAG \
{ int i; \
	for (i = 0; i < node->_nparents; i++) { \
		struct X3D_Node *n = X3D_NODE(node->_parents[i]); \
		if( n != 0 ) n->_renderFlags = n->_renderFlags | VF_localLight; \
	} \
}



#define  CHECK_MATERIAL_TRANSPARENCY \
	if (((struct X3D_Material *)node)->transparency > 0.0001) { \
		/* printf ("node %d MATERIAL HAS TRANSPARENCY of %f \n", node, ((struct X3D_Material *)node)->transparency); */ \
		update_renderFlag(X3D_NODE(node),VF_Blend);\
		have_transparency = TRUE; \
	}
 
#define CHECK_IMAGETEXTURE_TRANSPARENCY \
	if (isTextureAlpha(((struct X3D_ImageTexture *)node)->__textureTableIndex)) { \
		/* printf ("node %d IMAGETEXTURE HAS TRANSPARENCY\n", node); */ \
		update_renderFlag(X3D_NODE(node),VF_Blend);\
		have_transparency = TRUE; \
	}

#define CHECK_PIXELTEXTURE_TRANSPARENCY \
	if (isTextureAlpha(((struct X3D_PixelTexture *)node)->__textureTableIndex)) { \
		/* printf ("node %d PIXELTEXTURE HAS TRANSPARENCY\n", node); */ \
		update_renderFlag(X3D_NODE(node),VF_Blend);\
		have_transparency = TRUE; \
	}

#define CHECK_MOVIETEXTURE_TRANSPARENCY \
	if (isTextureAlpha(((struct X3D_MovieTexture *)node)->__textureTableIndex)) { \
		/* printf ("node %d MOVIETEXTURE HAS TRANSPARENCY\n", node); */ \
		update_renderFlag(X3D_NODE(node),VF_Blend);\
		have_transparency = TRUE; \
	}


void startOfLoopNodeUpdates(void) {
	struct X3D_Node* node;
	struct X3D_Anchor* anchorPtr;
	void **pp;
	int nParents;
	int i,j;
	uintptr_t *setBindPtr;

	struct Multi_Node *addChildren;
	struct Multi_Node *removeChildren;
	struct Multi_Node *childrenPtr;
	size_t offsetOfChildrenPtr;

	/* process one inline per loop; do it outside of the lock/unlock memory table */
	struct Vector *loadInlines;

	/* initialization */
	addChildren = NULL;
	removeChildren = NULL;
	childrenPtr = NULL;
	pp = NULL;
	loadInlines = NULL;
	offsetOfChildrenPtr = 0;

	/* assume that we do not have any sensitive nodes at all... */
	HaveSensitive = FALSE;
	have_transparency = FALSE;


	/* do not bother doing this if the inputparsing thread is active */
	if (isinputThreadParsing()) return;

	LOCK_MEMORYTABLE

	/* go through the node table, and zero any bits of interest */
	for (i=0; i<nextEntry; i++){		
		node = memoryTable[i];	
		if (node != NULL) {
			/* printf ("%d ref %d\n",i,node->referenceCount); */
			if (node->referenceCount == 0) {
				/* killNode(i); */
			} else {
				/* turn OFF these flags */
				node->_renderFlags = node->_renderFlags & (0xFFFF^VF_Sensitive);
				node->_renderFlags = node->_renderFlags & (0xFFFF^VF_Viewpoint);
				node->_renderFlags = node->_renderFlags & (0xFFFF^VF_localLight);
				node->_renderFlags = node->_renderFlags & (0xFFFF^VF_globalLight);
				node->_renderFlags = node->_renderFlags & (0xFFFF^VF_Blend);
			}
		}
	}

	/* sort the rootNode, if it is Not NULL */
	if (rootNode != NULL) {
		sortChildren (&rootNode->children, &rootNode->_sortedChildren,ROOTNODE_NEEDS_COMPILING);
	}

	/* go through the list of nodes, and "work" on any that need work */
	nParents = 0;
	setBindPtr = NULL;
	anchorPtr = NULL;

	for (i=0; i<nextEntry; i++){		
		node = memoryTable[i];	
		if (node != NULL) 
		if (node->referenceCount > 0) {
			switch (node->_nodeType) {
				BEGIN_NODE(Shape)
					/* send along a "look at me" flag if we are visible, or we should look again */
					if ((X3D_SHAPE(node)->__occludeCheckCount <=0) ||
							(X3D_SHAPE(node)->__visible)) {
						update_renderFlag (X3D_NODE(node),VF_hasVisibleChildren);
						/* printf ("shape occludecounter, pushing visiblechildren flags\n");  */

					}
					X3D_SHAPE(node)->__occludeCheckCount--;
					/* printf ("shape occludecounter %d\n",X3D_SHAPE(node)->__occludeCheckCount); */
				END_NODE

				/* Lights. DirectionalLights are "scope relative", PointLights and
				   SpotLights are transformed */

				BEGIN_NODE(DirectionalLight)
					if (X3D_DIRECTIONALLIGHT(node)->on) {
						if (X3D_DIRECTIONALLIGHT(node)->global) 
							update_renderFlag(node,VF_globalLight);
						else
							LOCAL_LIGHT_PARENT_FLAG
					}
				END_NODE
				BEGIN_NODE(SpotLight)
					if (X3D_SPOTLIGHT(node)->on) {
						if (X3D_SPOTLIGHT(node)->global) 
							update_renderFlag(node,VF_globalLight);
						else
							LOCAL_LIGHT_PARENT_FLAG
					}
				END_NODE
				BEGIN_NODE(PointLight)
					if (X3D_POINTLIGHT(node)->on) {
						if (X3D_POINTLIGHT(node)->global) 
							update_renderFlag(node,VF_globalLight);
						else
							LOCAL_LIGHT_PARENT_FLAG
					}
				END_NODE


				/* some nodes, like Extrusions, have "set_" fields same as normal internal fields,
				   eg, "set_spine" and "spine". Here we just copy the fields over, and remove the
				   "set_" fields. This works for MF* fields ONLY */
				BEGIN_NODE(IndexedLineSet)
					EVIN_AND_FIELD_SAME(colorIndex,IndexedLineSet)
					EVIN_AND_FIELD_SAME(coordIndex,IndexedLineSet)
				END_NODE

				BEGIN_NODE(IndexedTriangleFanSet)
					EVIN_AND_FIELD_SAME(index,IndexedTriangleFanSet)
				END_NODE
				BEGIN_NODE(IndexedTriangleSet)
					EVIN_AND_FIELD_SAME(index,IndexedTriangleSet)
				END_NODE
				BEGIN_NODE(IndexedTriangleStripSet)
					EVIN_AND_FIELD_SAME(index,IndexedTriangleStripSet)
				END_NODE
				BEGIN_NODE(GeoElevationGrid)
					EVIN_AND_FIELD_SAME(height,GeoElevationGrid)
				END_NODE
				BEGIN_NODE(ElevationGrid)
					EVIN_AND_FIELD_SAME(height,ElevationGrid)
				END_NODE
				BEGIN_NODE(Extrusion)
					EVIN_AND_FIELD_SAME(crossSection,Extrusion)
					EVIN_AND_FIELD_SAME(orientation,Extrusion)
					EVIN_AND_FIELD_SAME(scale,Extrusion)
					EVIN_AND_FIELD_SAME(spine,Extrusion)
				END_NODE
				BEGIN_NODE(IndexedFaceSet)
					EVIN_AND_FIELD_SAME(colorIndex,IndexedFaceSet)
					EVIN_AND_FIELD_SAME(coordIndex,IndexedFaceSet)
					EVIN_AND_FIELD_SAME(normalIndex,IndexedFaceSet)
					EVIN_AND_FIELD_SAME(texCoordIndex,IndexedFaceSet)
				END_NODE
/* GeoViewpoint works differently than other nodes - see compile_GeoViewpoint for manipulation of these fields
				BEGIN_NODE(GeoViewpoint)
					EVIN_AND_FIELD_SAME(orientation,GeoViewpoint) 
					EVIN_AND_FIELD_SAME(position,GeoViewpoint)
				END_NODE
*/
	
				/* get ready to mark these nodes as Mouse Sensitive */
				BEGIN_NODE(PlaneSensor) SIBLING_SENSITIVE(PlaneSensor) END_NODE
				BEGIN_NODE(SphereSensor) SIBLING_SENSITIVE(SphereSensor) END_NODE
				BEGIN_NODE(CylinderSensor) SIBLING_SENSITIVE(CylinderSensor) END_NODE
				BEGIN_NODE(TouchSensor) SIBLING_SENSITIVE(TouchSensor) END_NODE
				BEGIN_NODE(GeoTouchSensor) SIBLING_SENSITIVE(GeoTouchSensor) END_NODE
	
				/* Anchor is Mouse Sensitive, AND has Children nodes */
				BEGIN_NODE(Anchor)
					sortChildren(&X3D_ANCHOR(node)->children,&X3D_ANCHOR(node)->_sortedChildren,NODE_NEEDS_COMPILING);
					propagateExtent(X3D_NODE(node));
					ANCHOR_SENSITIVE(Anchor)
					CHILDREN_NODE(Anchor)
				END_NODE
				
				/* maybe this is the current Viewpoint? */
				BEGIN_NODE(Viewpoint) VIEWPOINT(Viewpoint) END_NODE
				BEGIN_NODE(OrthoViewpoint) VIEWPOINT(OrthoViewpoint) END_NODE
				BEGIN_NODE(GeoViewpoint) VIEWPOINT(GeoViewpoint) END_NODE
	
				BEGIN_NODE(NavigationInfo) 
					render_NavigationInfo ((struct X3D_NavigationInfo *)node);
				END_NODE

				BEGIN_NODE(StaticGroup)
					/* we should probably not do this, but... */
					sortChildren(&X3D_STATICGROUP(node)->children,&X3D_STATICGROUP(node)->_sortedChildren,NODE_NEEDS_COMPILING);
					propagateExtent(X3D_NODE(node));
				END_NODE


				/* does this one possibly have add/removeChildren? */
				BEGIN_NODE(Group) 
					sortChildren(&X3D_GROUP(node)->children,&X3D_GROUP(node)->_sortedChildren,NODE_NEEDS_COMPILING);

					propagateExtent(X3D_NODE(node));
					CHILDREN_NODE(Group) 
				END_NODE

				/* DJTRACK_PICKSENSORS */
				BEGIN_NODE(PickableGroup) 
					sortChildren(&X3D_PICKABLEGROUP(node)->children,&X3D_PICKABLEGROUP(node)->_sortedChildren,NODE_NEEDS_COMPILING);

					propagateExtent(X3D_NODE(node));
					CHILDREN_NODE(PickableGroup) 
				END_NODE

				BEGIN_NODE(Inline) 
					if (X3D_INLINE(node)->__loadstatus != INLINE_STABLE) {
						/* schedule this after we have unlocked the memory table */
						if (loadInlines == NULL) {
							loadInlines = newVector(struct X3D_Inline*, 16);
						}
						vector_pushBack(struct X3D_Inline *, loadInlines, X3D_INLINE(node));
					}
					sortChildren (&X3D_INLINE(node)->__children,&X3D_INLINE(node)->_sortedChildren,NODE_NEEDS_COMPILING);
					propagateExtent(X3D_NODE(node));
				END_NODE

				BEGIN_NODE(Transform) 
					sortChildren(&X3D_TRANSFORM(node)->children,&X3D_TRANSFORM(node)->_sortedChildren,NODE_NEEDS_COMPILING);
					propagateExtent(X3D_NODE(node));
					CHILDREN_NODE(Transform) 
				END_NODE

				BEGIN_NODE(NurbsGroup) 
					CHILDREN_NODE(NurbsGroup) 
				END_NODE

				BEGIN_NODE(Contour2D) 
					CHILDREN_NODE(Contour2D) 
				END_NODE

				BEGIN_NODE(HAnimSite) 
					CHILDREN_NODE(HAnimSite) 
				END_NODE

				BEGIN_NODE(HAnimSegment) 
					CHILDREN_NODE(HAnimSegment) 
				END_NODE

				BEGIN_NODE(HAnimJoint) 
					CHILDREN_NODE(HAnimJoint) 
				END_NODE

				BEGIN_NODE(Billboard) 
					sortChildren (&X3D_BILLBOARD(node)->children,&X3D_BILLBOARD(node)->_sortedChildren,NODE_NEEDS_COMPILING);
					propagateExtent(X3D_NODE(node));
					CHILDREN_NODE(Billboard) 
                			update_renderFlag(node,VF_Proximity);
				END_NODE

				BEGIN_NODE(Collision) 
					sortChildren (&X3D_COLLISION(node)->children,&X3D_COLLISION(node)->_sortedChildren,NODE_NEEDS_COMPILING);
					propagateExtent(X3D_NODE(node));
					CHILDREN_NODE(Collision) 
				END_NODE

				BEGIN_NODE(Switch) 
					propagateExtent(X3D_NODE(node));
					CHILDREN_SWITCH_NODE(Switch) 
				END_NODE

				BEGIN_NODE(LOD) 
					propagateExtent(X3D_NODE(node));
					CHILDREN_LOD_NODE 
                			update_renderFlag(node,VF_Proximity);
				END_NODE

				/* Material - transparency of materials */
				BEGIN_NODE(Material) CHECK_MATERIAL_TRANSPARENCY END_NODE

				/* Textures - check transparency  */
				BEGIN_NODE(ImageTexture) CHECK_IMAGETEXTURE_TRANSPARENCY END_NODE
				BEGIN_NODE(PixelTexture) CHECK_PIXELTEXTURE_TRANSPARENCY END_NODE
				BEGIN_NODE(MovieTexture) CHECK_MOVIETEXTURE_TRANSPARENCY END_NODE


				/* Backgrounds, Fog */
				BEGIN_NODE(Background)
					if (X3D_BACKGROUND(node)->isBound) update_renderFlag (X3D_NODE(node),VF_hasVisibleChildren);
				END_NODE

				BEGIN_NODE(TextureBackground)
					if (X3D_TEXTUREBACKGROUND(node)->isBound) update_renderFlag (X3D_NODE(node),VF_hasVisibleChildren);
				END_NODE

				BEGIN_NODE(Fog)
					if (X3D_FOG(node)->isBound) update_renderFlag (X3D_NODE(node),VF_hasVisibleChildren);
				END_NODE


				/* VisibilitySensor needs its own flag sent up the chain */
				BEGIN_NODE (VisibilitySensor)
					/* send along a "look at me" flag if we are visible, or we should look again */
					if ((X3D_VISIBILITYSENSOR(node)->__occludeCheckCount <=0) ||
							(X3D_VISIBILITYSENSOR(node)->__visible)) {
						update_renderFlag (X3D_NODE(node),VF_hasVisibleChildren);
						/* printf ("vis occludecounter, pushing visiblechildren flags\n"); */

					}
					X3D_VISIBILITYSENSOR(node)->__occludeCheckCount--;
					/* VisibilitySensors have a transparent bounding box we have to render */

                			update_renderFlag(node,VF_Blend);
                			//update_renderFlag(node,VF_Sensitive);
					//HaveSensitive=TRUE;

				END_NODE

				/* ProximitySensor needs its own flag sent up the chain */
				BEGIN_NODE (ProximitySensor)
                			if (X3D_PROXIMITYSENSOR(node)->enabled) update_renderFlag(node,VF_Proximity);
				END_NODE

				/* GeoProximitySensor needs its own flag sent up the chain */
				BEGIN_NODE (GeoProximitySensor)
                			if (X3D_GEOPROXIMITYSENSOR(node)->enabled) update_renderFlag(node,VF_Proximity);
				END_NODE

				/* GeoLOD needs its own flag sent up the chain, and it has to push extent up, too */
				BEGIN_NODE (GeoLOD)
					if (!(NODE_NEEDS_COMPILING)) {
						handle_GeoLODRange(X3D_GEOLOD(node));
					}
                			/* update_renderFlag(node,VF_Proximity); */
					propagateExtent(X3D_NODE(node));
				END_NODE

				BEGIN_NODE (GeoTransform)
					sortChildren(&X3D_GEOTRANSFORM(node)->children,&X3D_GEOTRANSFORM(node)->_sortedChildren,NODE_NEEDS_COMPILING);
					propagateExtent(X3D_NODE(node));
					CHILDREN_NODE(GeoTransform) 
				END_NODE

				BEGIN_NODE (GeoLocation)
					sortChildren(&X3D_GEOLOCATION(node)->children,&X3D_GEOLOCATION(node)->_sortedChildren,NODE_NEEDS_COMPILING);
					propagateExtent(X3D_NODE(node));
					CHILDREN_NODE(GeoLocation) 
				END_NODE

				BEGIN_NODE(MetadataSFBool) CMD(SFBool,node); END_NODE
				BEGIN_NODE(MetadataSFFloat) CMD(SFFloat,node); END_NODE
				BEGIN_NODE(MetadataMFFloat) CMD(MFFloat,node); END_NODE
				BEGIN_NODE(MetadataSFRotation) CMD(SFRotation,node); END_NODE
				BEGIN_NODE(MetadataMFRotation) CMD(MFRotation,node); END_NODE
				BEGIN_NODE(MetadataSFVec3f) CMD(SFVec3f,node); END_NODE
				BEGIN_NODE(MetadataMFVec3f) CMD(MFVec3f,node); END_NODE
				BEGIN_NODE(MetadataMFBool) CMD(MFBool,node); END_NODE
				BEGIN_NODE(MetadataSFInt32) CMD(SFInt32,node); END_NODE
				BEGIN_NODE(MetadataMFInt32) CMD(MFInt32,node); END_NODE
				BEGIN_NODE(MetadataSFNode) CMD(SFNode,node); END_NODE
				BEGIN_NODE(MetadataMFNode) CMD(MFNode,node); END_NODE
				BEGIN_NODE(MetadataSFColor) CMD(SFColor,node); END_NODE
				BEGIN_NODE(MetadataMFColor) CMD(MFColor,node); END_NODE
				BEGIN_NODE(MetadataSFColorRGBA) CMD(SFColorRGBA,node); END_NODE
				BEGIN_NODE(MetadataMFColorRGBA) CMD(MFColorRGBA,node); END_NODE
				BEGIN_NODE(MetadataSFTime) CMD(SFTime,node); END_NODE
				BEGIN_NODE(MetadataMFTime) CMD(MFTime,node); END_NODE
				BEGIN_NODE(MetadataSFString) CMD(SFString,node); END_NODE
				BEGIN_NODE(MetadataMFString) CMD(MFString,node); END_NODE
				BEGIN_NODE(MetadataSFVec2f) CMD(SFVec2f,node); END_NODE
				BEGIN_NODE(MetadataMFVec2f) CMD(MFVec2f,node); END_NODE
				BEGIN_NODE(MetadataSFImage) CMD(SFImage,node); END_NODE
				BEGIN_NODE(MetadataSFVec3d) CMD(SFVec3d,node); END_NODE
				BEGIN_NODE(MetadataMFVec3d) CMD(MFVec3d,node); END_NODE
				BEGIN_NODE(MetadataSFDouble) CMD(SFDouble,node); END_NODE
				BEGIN_NODE(MetadataMFDouble) CMD(MFDouble,node); END_NODE
				BEGIN_NODE(MetadataSFMatrix3f) CMD(SFMatrix3f,node); END_NODE
				BEGIN_NODE(MetadataMFMatrix3f) CMD(MFMatrix3f,node); END_NODE
				BEGIN_NODE(MetadataSFMatrix3d) CMD(SFMatrix3d,node); END_NODE
				BEGIN_NODE(MetadataMFMatrix3d) CMD(MFMatrix3d,node); END_NODE
				BEGIN_NODE(MetadataSFMatrix4f) CMD(SFMatrix4f,node); END_NODE
				BEGIN_NODE(MetadataMFMatrix4f) CMD(MFMatrix4f,node); END_NODE
				BEGIN_NODE(MetadataSFMatrix4d) CMD(SFMatrix4d,node); END_NODE
				BEGIN_NODE(MetadataMFMatrix4d) CMD(MFMatrix4d,node); END_NODE
				BEGIN_NODE(MetadataSFVec2d) CMD(SFVec2d,node); END_NODE
				BEGIN_NODE(MetadataMFVec2d) CMD(MFVec2d,node); END_NODE
				BEGIN_NODE(MetadataSFVec4f) CMD(SFVec4f,node); END_NODE
				BEGIN_NODE(MetadataMFVec4f) CMD(MFVec4f,node); END_NODE
				BEGIN_NODE(MetadataSFVec4d) CMD(SFVec4d,node); END_NODE
				BEGIN_NODE(MetadataMFVec4d) CMD(MFVec4d,node); END_NODE

				/* VRML1 Separator node; we do a bare bones implementation; always assume there are 
					lights, geometry, and viewpoints here. */
				BEGIN_NODE(VRML1_Separator) 
					sortChildren(&VRML1_SEPARATOR(node)->VRML1children,&VRML1_SEPARATOR(node)->_sortedChildren,NODE_NEEDS_COMPILING);
					propagateExtent(X3D_NODE(node));
					update_renderFlag(X3D_NODE(node),VF_localLight|VF_Viewpoint|VF_Geom|VF_hasVisibleChildren);
				END_NODE
			}
		}

		/* now, act on this node  for Sensitive nodes. here we tell the PARENTS that they are sensitive */
		if (nParents != 0) {
			for (j=0; j<nParents; j++) {
				struct X3D_Node *n = X3D_NODE(pp[j]);
				n->_renderFlags = n->_renderFlags  | VF_Sensitive;
			}

			/* tell mainloop that we have to do a sensitive pass now */
			HaveSensitive = TRUE;
			nParents = 0;
		}

		/* Anchor nodes are slightly different than sibling-sensitive nodes */
		if (anchorPtr != NULL) {
			anchorPtr->_renderFlags = anchorPtr->_renderFlags  | VF_Sensitive;

			/* tell mainloop that we have to do a sensitive pass now */
			HaveSensitive = TRUE;
			anchorPtr = NULL;
		}

		/* do BINDING of Viewpoint Nodes */
		if (setBindPtr != NULL) {
			/* check the set_bind eventin to see if it is TRUE or FALSE */
			if (*setBindPtr < 100) {
				/* printf ("Found a vp to modify %d\n",node); */
				/* up_vector is reset after a bind */
				//if (*setBindPtr==1) reset_upvector();
				bind_node ((void *)node, &viewpoint_tos,&viewpoint_stack[0]);

				//dug9 added July 24, 2009: when you bind, it should set the 
				//avatar to the newly bound viewpoint pose and forget any 
				// cumulative avatar navigation from the last viewpoint parent

				if (node->_nodeType==NODE_Viewpoint) {
					bind_Viewpoint((struct X3D_Viewpoint *)node); 
				} else if (node->_nodeType==NODE_OrthoViewpoint) {
					bind_OrthoViewpoint((struct X3D_OrthoViewpoint *) node);
				} else {
					bind_GeoViewpoint((struct X3D_GeoViewpoint *) node);
				}
			}
			setBindPtr = NULL;
		}

		/* this node possibly has to do add/remove children? */
		if (childrenPtr != NULL) {
			if (addChildren != NULL) {
				AddRemoveChildren(node,childrenPtr,(struct X3D_Node * *) addChildren->p,addChildren->n,1,__FILE__,__LINE__);
				addChildren->n=0;
			}
			if (removeChildren != NULL) {
				AddRemoveChildren(node,childrenPtr,(struct X3D_Node * *) removeChildren->p,removeChildren->n,2,__FILE__,__LINE__);
				removeChildren->n=0;
			}
			/* printf ("OpenGL, marking children changed\n"); */
			MARK_EVENT(node,offsetOfChildrenPtr);
			childrenPtr = NULL;
		}
	}



	UNLOCK_MEMORYTABLE

	/* do we have Inlines to load here, outside of the memorytable lock? */
	if (loadInlines != NULL) {
		indexT ind;

		for (ind=0; ind<vector_size(loadInlines); ind++) {
			struct X3D_Inline *node;
			node=vector_get(struct X3D_Inline*, loadInlines,ind);
			load_Inline (node);
		}
		deleteVector (struct X3D_Inline*, loadInlines);
	}

	/* now, we can go and tell the grouping nodes which ones are the lucky ones that contain the current Viewpoint node */
	if (viewpoint_stack[viewpoint_tos] != 0) {
		update_renderFlag(X3D_NODE(viewpoint_stack[viewpoint_tos]), VF_Viewpoint);
		calculateNearFarplanes(X3D_NODE(viewpoint_stack[viewpoint_tos]));
	} else {
		/* keep these at the defaults, if no viewpoint is present. */
		nearPlane = DEFAULT_NEARPLANE;
		farPlane = DEFAULT_FARPLANE;
		backgroundPlane = DEFAULT_BACKGROUNDPLANE;
	}
}

void markForDispose(struct X3D_Node *node, int recursive){
	struct Multi_Node* MNode;

	#if USE_JS_EXPERIMENTAL_CODE
	struct X3D_Node sfnode;
	#endif

	int *fieldOffsetsPtr;
	char * fieldPtr;

	if (node==NULL) return;

	 
/*
	printf ("\nmarkingForDispose %u (%s) currently at %d\n",node,
		stringNodeType(node->_nodeType),node->referenceCount);
*/
	
	if (node->referenceCount > 0) node->referenceCount --;

	if (recursive) {

	fieldOffsetsPtr = NODE_OFFSETS[node->_nodeType];
	/*go thru all field*/				
	while (*fieldOffsetsPtr != -1) {
		fieldPtr = offsetPointer_deref(char *, node,*(fieldOffsetsPtr+1));
		#ifdef VERBOSE
		printf ("looking at field %s type %s\n",FIELDNAMES[*fieldOffsetsPtr],FIELDTYPES[*(fieldOffsetsPtr+2)]); 
		#endif

		/* some fields we skip, as the pointers are duplicated, and we CAN NOT free both */
		if (*fieldOffsetsPtr == FIELDNAMES_setValue) 
			break; /* can be a duplicate SF/MFNode pointer */
	
		if (*fieldOffsetsPtr == FIELDNAMES_valueChanged) 
			break; /* can be a duplicate SF/MFNode pointer */
	

		if (*fieldOffsetsPtr == FIELDNAMES___oldmetadata) 
			break; /* can be a duplicate SFNode pointer */
	
		if (*fieldOffsetsPtr == FIELDNAMES___lastParent) 
			break; /* can be a duplicate SFNode pointer - field only in NODE_TextureCoordinate */
	
		if (*fieldOffsetsPtr == FIELDNAMES__selected) 
			break; /* can be a duplicate SFNode pointer - field only in NODE_LOD and NODE_GeoLOD */

		if (*fieldOffsetsPtr == FIELDNAMES___oldChildren) 
			break; /* can be a duplicate SFNode pointer - field only in NODE_LOD and NODE_GeoLOD */

		if (*fieldOffsetsPtr == FIELDNAMES___oldKeyPtr) 
			break; /* used for seeing if interpolator values change */

		if (*fieldOffsetsPtr == FIELDNAMES___oldKeyValuePtr) 
			break; /* used for seeing if interpolator values change */

		/* GeoLOD nodes, the children field exports either the rootNode, or the list of child nodes */
		if (node->_nodeType == NODE_GeoLOD) {
			if (*fieldOffsetsPtr == FIELDNAMES_children) break;
		}
	
		/* nope, not a special field, lets just get rid of it as best we can */
		switch(*(fieldOffsetsPtr+2)){
			case FIELDTYPE_MFNode: {
				int i;
				struct X3D_Node *tp;
				MNode=(struct Multi_Node *)fieldPtr;
		/* printf (" ... field MFNode, %s type %s\n",FIELDNAMES[*fieldOffsetsPtr],FIELDTYPES[*(fieldOffsetsPtr+2)]); */

				for (i=0; i<MNode->n; i++) {
					tp = MNode->p[i];
					 
					if (tp!=NULL)
						markForDispose(tp,TRUE);
				}
				MNode->n=0;
				break;
				}	
#ifdef wrwewetwetwe
			case FIELDTYPE_SFNode: {
				struct X3D_Node *SNode;

				SNode = (struct X3D_Node *)*fieldPtr;
printf ("SFNode, field is %u...\n",SNode);
if (SNode != NULL)
printf ("SFNode, .... and it is of type %s\n",stringNodeType(SNode->_nodeType));

		printf (" ... field SFNode, %s type %s\n",FIELDNAMES[*fieldOffsetsPtr],FIELDTYPES[*(fieldOffsetsPtr+2)]); 
				printf ("marking this SFnode for dispose, %u\n",SNode); 
				markForDispose(SNode, TRUE);
				break;
				

			}	
#endif
			default:; /* do nothing - field not malloc'd */
		}
		fieldOffsetsPtr+=5;	
	}


	}
}


#if USE_JS_EXPERIMENTAL_CODE
/*delete node created*/
static void killNode (int index) {
	int j=0;
	int *fieldOffsetsPtr;
	char * fieldPtr;
	struct X3D_Node* structptr;
	struct Multi_Float* MFloat;
	struct Multi_Rotation* MRotation;
	struct Multi_Vec3f* MVec3f;
	struct Multi_Bool* Mbool;
	struct Multi_Int32* MInt32;
	struct Multi_Node* MNode;
	struct Multi_Color* MColor;
	struct Multi_ColorRGBA* MColorRGBA;
	struct Multi_Time* MTime;
	struct Multi_String* MString;
	struct Multi_Vec2f* MVec2f;
	uintptr_t * VPtr;
	struct Uni_String *MyS;

	structptr = memoryTable[index];		

	#ifdef VERBOSE
	printf("Node pointer	= %u entry %d of %d ",structptr,i,nextEntry);
	printf (" number of parents %d ", structptr->_nparents);
	printf("Node Type	= %s\n",stringNodeType(structptr->_nodeType));  
	#endif

	/* kill any parents that may exist. */
	FREE_IF_NZ (structptr->_parents);

	fieldOffsetsPtr = NODE_OFFSETS[structptr->_nodeType];
	/*go thru all field*/				
	while (*fieldOffsetsPtr != -1) {
		fieldPtr = offsetPointer_deref(char *, structptr,*(fieldOffsetsPtr+1));
		#ifdef VERBOSE
		printf ("looking at field %s type %s\n",FIELDNAMES[*fieldOffsetsPtr],FIELDTYPES[*(fieldOffsetsPtr+2)]); 
		#endif

		/* some fields we skip, as the pointers are duplicated, and we CAN NOT free both */
		if (*fieldOffsetsPtr == FIELDNAMES_setValue) 
			break; /* can be a duplicate SF/MFNode pointer */
	
		if (*fieldOffsetsPtr == FIELDNAMES_valueChanged) 
			break; /* can be a duplicate SF/MFNode pointer */
	

		if (*fieldOffsetsPtr == FIELDNAMES___oldmetadata) 
			break; /* can be a duplicate SFNode pointer */
	
		if (*fieldOffsetsPtr == FIELDNAMES___lastParent) 
			break; /* can be a duplicate SFNode pointer - field only in NODE_TextureCoordinate */
	
		if (*fieldOffsetsPtr == FIELDNAMES__selected) 
			break; /* can be a duplicate SFNode pointer - field only in NODE_LOD and NODE_GeoLOD */

		if (*fieldOffsetsPtr == FIELDNAMES___oldChildren) 
			break; /* can be a duplicate SFNode pointer - field only in NODE_LOD and NODE_GeoLOD */

		if (*fieldOffsetsPtr == FIELDNAMES___oldMFString) 
			break; 

		if (*fieldOffsetsPtr == FIELDNAMES___scriptObj) 
			break; 

		if (*fieldOffsetsPtr == FIELDNAMES___oldSFString) 
			break; 

		if (*fieldOffsetsPtr == FIELDNAMES___oldKeyPtr) 
			break; /* used for seeing if interpolator values change */

		if (*fieldOffsetsPtr == FIELDNAMES___oldKeyValuePtr) 
			break; /* used for seeing if interpolator values change */

		if (*fieldOffsetsPtr == FIELDNAMES___shaderIDS) {
			struct X3D_ComposedShader *cps = (struct X3D_ComposedShader *) structptr;
			if ((cps->_nodeType == NODE_ComposedShader) || (cps->_nodeType == NODE_ProgramShader)) {
#ifdef GL_VERSION_2_0
				if (cps->__shaderIDS.p != NULL) {
					DELETE_PROGRAM((GLuint) cps->__shaderIDS.p[0]);
					FREE_IF_NZ(cps->__shaderIDS.p);
					cps->__shaderIDS.n=0;
				}
#endif

			} else {
				ConsoleMessage ("error destroying shaderIDS on kill");
			}
		}

		/* GeoLOD nodes, the children field exports either the rootNode, or the list of child nodes */
		if (structptr->_nodeType == NODE_GeoLOD) {
			if (*fieldOffsetsPtr == FIELDNAMES_children) break;
		}
	
		/* nope, not a special field, lets just get rid of it as best we can */
		switch(*(fieldOffsetsPtr+2)){
			case FIELDTYPE_MFFloat:
				MFloat=(struct Multi_Float *)fieldPtr;
				MFloat->n=0;
				FREE_IF_NZ(MFloat->p);
				break;
			case FIELDTYPE_MFRotation:
				MRotation=(struct Multi_Rotation *)fieldPtr;
				MRotation->n=0;
				FREE_IF_NZ(MRotation->p);
				break;
			case FIELDTYPE_MFVec3f:
				MVec3f=(struct Multi_Vec3f *)fieldPtr;
				MVec3f->n=0;
				FREE_IF_NZ(MVec3f->p);
				break;
			case FIELDTYPE_MFBool:
				Mbool=(struct Multi_Bool *)fieldPtr;
				Mbool->n=0;
				FREE_IF_NZ(Mbool->p);
				break;
			case FIELDTYPE_MFInt32:
				MInt32=(struct Multi_Int32 *)fieldPtr;
				MInt32->n=0;
				FREE_IF_NZ(MInt32->p);
				break;
			case FIELDTYPE_MFNode:
				MNode=(struct Multi_Node *)fieldPtr;
				#ifdef VERBOSE
				/* verify node structure. Each child should point back to me. */
				{
					int i;
					struct X3D_Node *tp;
					for (i=0; i<MNode->n; i++) {
						tp = MNode->p[i];
						printf ("	MNode field has child %u\n",tp);
						if (tp!=NULL)
						printf ("	ct %s\n",stringNodeType(tp->_nodeType));
					}
				}	
				#endif
				MNode->n=0;
				FREE_IF_NZ(MNode->p);
				break;

			case FIELDTYPE_MFColor:
				MColor=(struct Multi_Color *)fieldPtr;
				MColor->n=0;
				FREE_IF_NZ(MColor->p);
				break;
			case FIELDTYPE_MFColorRGBA:
				MColorRGBA=(struct Multi_ColorRGBA *)fieldPtr;
				MColorRGBA->n=0;
				FREE_IF_NZ(MColorRGBA->p);
				break;
			case FIELDTYPE_MFTime:
				MTime=(struct Multi_Time *)fieldPtr;
				MTime->n=0;
				FREE_IF_NZ(MTime->p);
				break;
			case FIELDTYPE_MFString: 
				MString=(struct Multi_String *)fieldPtr;
				{
				struct Uni_String* ustr;
				for (j=0; j<MString->n; j++) {
					ustr=MString->p[j];
					if (ustr != NULL) {
					ustr->len=0;
					ustr->touched=0;
					FREE_IF_NZ(ustr->strptr);
					}
				}
				MString->n=0;
				FREE_IF_NZ(MString->p);
				}
				break;
			case FIELDTYPE_MFVec2f:
				MVec2f=(struct Multi_Vec2f *)fieldPtr;
				MVec2f->n=0;
				FREE_IF_NZ(MVec2f->p);
				break;
			case FIELDTYPE_FreeWRLPTR:
				VPtr = (uintptr_t *) fieldPtr;
				VPtr = (uintptr_t *) (*VPtr);
				FREE_IF_NZ(VPtr);
				break;
			case FIELDTYPE_SFString:
				VPtr = (uintptr_t *) fieldPtr;
				MyS = (struct Uni_String *) *VPtr;
				MyS->len = 0;
				FREE_IF_NZ(MyS->strptr);
				FREE_IF_NZ(MyS);
				break;
				
			default:; /* do nothing - field not malloc'd */
		}
		fieldOffsetsPtr+=5;	
	}
	FREE_IF_NZ(memoryTable[index]);
	memoryTable[index]=NULL;
}
#endif

#ifndef USE_OLD_OPENGL
#ifdef IPHONE
/* OpenGL-ES specifics for Materials and Vertices */
void fw_iphone_enableClientState(GLenum aaa)
{ printf  ("called fw_iphone_enableClientState\n");}

void fw_iphone_disableClientState(GLenum aaa)
{ printf  ("called fw_iphone_disableClientState\n");}

void fw_iphone_vertexPointer(GLint aaa,GLenum bbb,GLsizei ccc,const GLvoid *ddd)
{ printf  ("called fw_iphone_somethingPointer\n");}

void fw_iphone_normalPointer(GLenum aaa,GLsizei bbb, const GLvoid *ccc)
{ printf  ("called fw_iphone_somethingPointer\n");}

void fw_iphone_texcoordPointer(GLint aaa, GLenum bbb ,GLsizei ccc,const GLvoid *ddd)
{ printf  ("called fw_iphone_somethingPointer\n");}

void fw_iphone_colorPointer(GLint aaa, GLenum bbb,GLsizei ccc,const GLvoid *ddd)
{ printf  ("called fw_iphone_somethingPointer\n");}
#endif

#endif /* USE_OLD_OPENGL */

static void fw_glLoadMatrixd(double *val) {
	/* printf ("loading matrix...\n"); */
	glLoadMatrixd(val);
}

#ifndef USE_OLD_OPENGL

void sendMatriciesToShader(GLint MM,GLint PM) {
	float spval[16];
	int i;
	float *sp; 
	double *dp;

	// you can use this for testing something at the [0,0,0] position
	//float mvm[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -10, 1};
	//float pvm[] = {1.74427, 0, 0, 0, 0, 2.41421, 0, 0, 0, 0, -1.00001, -1, -0, -0, -0.200001, -0};

	/* ModelView first */
	dp = FW_ModelView[modelviewTOS];
	sp = spval;

	for (i=0; i<16; i++) {
		*sp = (float) *dp; 	
		sp ++; dp ++;
	}

	//for (i=0; i<16;i++) { printf ("ModelView %d: %4.3f %4.3f\n",i,spval[i],mvm[i]); }
	//glUniformMatrix4fv(MM,1,GL_FALSE,mvm);

	glUniformMatrix4fv(MM,1,GL_FALSE,spval);

	/* ProjectionMatrix */
	sp = spval;
	dp = FW_ProjectionView[projectionviewTOS];

	for (i=0; i<16; i++) {
		*sp = (float) *dp; 	
		sp ++; dp ++;
	}
	//for (i=0; i<16;i++) { printf ("Projection %d: %4.3f %4.3f\n",i,spval[i],pvm[i]); }
	//glUniformMatrix4fv(PM,1,GL_FALSE,pvm);

	glUniformMatrix4fv(PM,1,GL_FALSE,spval);
}

#endif /* USE_OLD_OPENGL */


static void __gluMultMatrixVecd(const GLdouble matrix[16], const GLdouble in[4],
                      GLdouble out[4])
{
    int i;

    for (i=0; i<4; i++) {
        out[i] =
            in[0] * matrix[0*4+i] +
            in[1] * matrix[1*4+i] +
            in[2] * matrix[2*4+i] +
            in[3] * matrix[3*4+i];
    }
}


void fw_gluProject
(GLdouble objx, GLdouble objy, GLdouble objz, 
	      const GLdouble modelMatrix[16], 
	      const GLdouble projMatrix[16],
              const GLint viewport[4],
	      GLdouble *winx, GLdouble *winy, GLdouble *winz)
{
    double in[4];
    double out[4];

    in[0]=objx;
    in[1]=objy;
    in[2]=objz;
    in[3]=1.0;
    __gluMultMatrixVecd(modelMatrix, in, out);
    __gluMultMatrixVecd(projMatrix, out, in);
    if (in[3] == 0.0) return;
    in[0] /= in[3];
    in[1] /= in[3];
    in[2] /= in[3];
    /* Map x, y and z to range 0-1 */
    in[0] = in[0] * 0.5 + 0.5;
    in[1] = in[1] * 0.5 + 0.5;
    in[2] = in[2] * 0.5 + 0.5;

    /* Map x,y to viewport */
    in[0] = in[0] * viewport[2] + viewport[0];
    in[1] = in[1] * viewport[3] + viewport[1];

    *winx=in[0];
    *winy=in[1];
    *winz=in[2];
}

static void __gluMultMatricesd(const GLdouble a[16], const GLdouble b[16],
                                GLdouble r[16])
{
    int i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            r[i*4+j] =
                a[i*4+0]*b[0*4+j] +
                a[i*4+1]*b[1*4+j] +
                a[i*4+2]*b[2*4+j] +
                a[i*4+3]*b[3*4+j];
        }
    }
}


/*
** Invert 4x4 matrix.
** Contributed by David Moore (See Mesa bug #6748)
*/
static int __gluInvertMatrixd(const GLdouble m[16], GLdouble invOut[16])
{
    double inv[16], det;
    int i;

    inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
             + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
             - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
             + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
             - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
             - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
             + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
             - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
             + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
             + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
    inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
             - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
    inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
             + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
             - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
    inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
             - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
    inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
             + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
             - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
    inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
             + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

    det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if (det == 0)
        return GL_FALSE;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return GL_TRUE;
}



void fw_gluUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
		const GLdouble modelMatrix[16], 
		const GLdouble projMatrix[16],
                const GLint viewport[4],
	        GLdouble *objx, GLdouble *objy, GLdouble *objz)
{
    double finalMatrix[16];
    double in[4];
    double out[4];

    __gluMultMatricesd(modelMatrix, projMatrix, finalMatrix);
    if (!__gluInvertMatrixd(finalMatrix, finalMatrix)) return;

    in[0]=winx;
    in[1]=winy;
    in[2]=winz;
    in[3]=1.0;

    /* Map x and y from window coordinates */
    in[0] = (in[0] - viewport[0]) / viewport[2];
    in[1] = (in[1] - viewport[1]) / viewport[3];

    /* Map to range -1 to 1 */
    in[0] = in[0] * 2 - 1;
    in[1] = in[1] * 2 - 1;
    in[2] = in[2] * 2 - 1;

    __gluMultMatrixVecd(finalMatrix, in, out);
    if (out[3] == 0.0) return;
    out[0] /= out[3];
    out[1] /= out[3];
    out[2] /= out[3];
    *objx = out[0];
    *objy = out[1];
    *objz = out[2];
}


void fw_Ortho (double left, double right, double bottom, double top, double nearZ, double farZ) {
	#ifdef USE_OLD_OPENGL
	GLDOUBLE dp[16];
	#else
	GLDOUBLE *dp;
	#endif

	/* do the glOrtho on the top of the stack, and send that along */
	#ifdef USE_OLD_OPENGL
	FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX,dp);
	#else
	dp = FW_ProjectionView[projectionviewTOS];
	#endif

	/* {int i; for (i=0; i<16;i++) { printf ("ModView before  %d: %4.3f \n",i,dp[i]); } */
	mesa_Ortho(left,right,bottom,top,nearZ,farZ,dp);

	/* {int i; for (i=0; i<16;i++) { printf ("ModView after   %d: %4.3f \n",i,dp[i]); } */

	fw_glLoadMatrixd(dp);
}

void printmatrix2(GLDOUBLE* mat,char* description ) {
    int i,j;
    printf("mat %s {\n",description);
    for(i = 0; i< 4; i++) {
		printf("mat [%2d-%2d] = ",i*4,(i*4)+3);
		for(j=0;j<4;j++) 
			printf(" %f ",mat[(i*4)+j]);
			//printf("mat[%d] = %f%s;\n",i,mat[i],i==12 ? " +disp.x" : i==13? " +disp.y" : i==14? " +disp.z" : "");
		printf("\n");
    }
    printf("}\n");

}


/* gluPerspective replacement */
void fw_gluPerspective(GLDOUBLE fovy, GLDOUBLE aspect, GLDOUBLE zNear, GLDOUBLE zFar) {
	GLDOUBLE xmin, xmax, ymin, ymax;

	#ifdef USE_OLD_OPENGL
	GLDOUBLE dp[16];
	#else
	GLDOUBLE *dp;
	#endif
	GLDOUBLE ndp[16];
	GLDOUBLE ndp2[16];

int i, method;
double xxx[16];
double yyy[16];
    GLdouble m[16];


	ymax = zNear * tan(fovy * M_PI / 360.0);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;

	/* do the glFrsutum on the top of the stack, and send that along */
	FW_GL_MATRIX_MODE(GL_PROJECTION);
	//FW_GL_LOAD_IDENTITY();
	#ifdef USE_OLD_OPENGL
	FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX,dp);
	#else
	dp = FW_ProjectionView[projectionviewTOS];
	#endif

	mesa_Frustum(xmin, xmax, ymin, ymax, zNear, zFar, ndp);
	mattranspose(ndp2,ndp);
	//printmatrix2(ndp,"ndp");
	//printmatrix2(ndp2,"ndp2 = transpose(ndp)");
	printmatrix2(dp,"dp");
	matmultiply(ndp,ndp2,dp);
	//printmatrix2(ndp,"ndp = ndp2*dp");
	method = 1;
	if(method==1)
	  FW_GL_LOADMATRIXD(ndp);
	//FW_GL_MATRIX_MODE(GL_PROJECTION);
	//FW_GL_LOADMATRIXD(ndp2);


/* testing... */
{
    double sine, cotangent, deltaZ;
    double radians = fovy / 2.0 * M_PI / 180.0;

    deltaZ = zFar - zNear;
    sine = sin(radians);
    if ((deltaZ == 0) || (sine == 0) || (aspect == 0)) {
        return;
    }
    cotangent = cos(radians) / sine;

	loadIdentityMatrix(m); //(&m);
    //__gluMakeIdentityd(&m[0][0]);
    m[0*4+0] = cotangent / aspect;
    m[1*4+1] = cotangent;
    m[2*4+2] = -(zFar + zNear) / deltaZ;
    m[2*4+3] = -1;
    m[3*4+2] = -2 * zNear * zFar / deltaZ;
    m[3*4+3] = 0;
	matmultiply(m,m,dp);
	if(method==2)
	  FW_GL_LOADMATRIXD(m);

    //glMultMatrixd(&m[0][0]);
}

//printf ("fw_gluPerspective, have...\n");

	if(method==3)
	  gluPerspective(fovy,aspect,zNear,zFar);
	FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX,yyy);
	printmatrix2(dp,"dp orig");
	printmatrix2(ndp,"ndp myPup");
	printmatrix2(yyy,"yyy gluP");
	printmatrix2(m,"m mesa");
	//for (i=0; i<16;i++) {printf ("%d orig: %5.2lf myPup: %5.2lf gluP: %5.2lf mesa %5.2lf\n",i,dp[i],
	//	ndp[i],yyy[i],m[i]); 
	//}

}



/* gluPickMatrix replacement */
void fw_gluPickMatrix(GLDOUBLE xx, GLDOUBLE yy, GLDOUBLE width, GLDOUBLE height, GLint *vp) {
printf ("PickMat %lf %lf %lf %lf %d %d %d %d\n",xx,yy,width,height,vp[0], vp[1],vp[2],vp[3]);

	//return;
	if ((width < 0.0) || (height < 0.0)) return;
	/* Translate and scale the picked region to the entire window */
	FW_GL_TRANSLATE_D((vp[2] - 2.0 * (xx - vp[0])) / width, (vp[3] - 2.0 * (yy - vp[1])) / height, 0.0);
	FW_GL_SCALE_D(vp[2] / width, vp[3] / height, 1.0);

}


#ifndef USE_OLD_OPENGL
/* glFrustum replacement - taken from the MESA source;

 * matrix.c
 *
 * Some useful matrix functions.
 *
 * Brian Paul
 * 10 Feb 2004
 */

/**
 * Build a glFrustum matrix.
 */

#endif

static void
mesa_Frustum(double left, double right, double bottom, double top, double nearZ, double farZ, double *m)
{
 /* http://www.songho.ca/opengl/gl_projectionmatrix.html shows derivation*/
   double x = (2.0*nearZ) / (right-left);
   double y = (2.0*nearZ) / (top-bottom);
   double a = (right+left) / (right-left);
   double b = (top+bottom) / (top-bottom);
   double c = -(farZ+nearZ) / ( farZ-nearZ);
   double d = -(2.0F*farZ*nearZ) / (farZ-nearZ);

	/* printf ("mesa_Frustum (%lf, %lf, %lf, %lf, %lf, %lf)\n",left,right,bottom,top,nearZ, farZ); */
#define M(row,col)  m[col*4+row]
	m[0] = x;
	m[1] = 0.0;
	m[2] = a;
	m[3] = 0.0;

	m[4] = 0.0;
	m[5] = y;
	m[6] = b;
	m[7] = 0.0;

	m[8] = 0.0;
	m[9] = 0.0;
	m[10] = c;
	m[11] = d;

	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = -1.0;
	m[15] = 0.0;
/*
	
   M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
   M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
   M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
   M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M
*/
}

/**
 * Build a glOrtho marix.
 */
static void
mesa_Ortho(double left, double right, double bottom, double top, double nearZ, double farZ, double *m)
{
#define M(row,col)  m[col*4+row]
   M(0,0) = 2.0F / (right-left);
   M(0,1) = 0.0F;
   M(0,2) = 0.0F;
   M(0,3) = -(right+left) / (right-left);

   M(1,0) = 0.0F;
   M(1,1) = 2.0F / (top-bottom);
   M(1,2) = 0.0F;
   M(1,3) = -(top+bottom) / (top-bottom);

   M(2,0) = 0.0F;
   M(2,1) = 0.0F;
   M(2,2) = -2.0F / (farZ-nearZ);
   M(2,3) = -(farZ+nearZ) / (farZ-nearZ);

   M(3,0) = 0.0F;
   M(3,1) = 0.0F;
   M(3,2) = 0.0F;
   M(3,3) = 1.0F;
#undef M
}
