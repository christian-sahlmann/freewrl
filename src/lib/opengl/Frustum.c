/*
=INSERT_TEMPLATE_HERE=

$Id: Frustum.c,v 1.8 2009/02/11 15:12:54 istakenv Exp $

???

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../scenegraph/quaternion.h"


#include "Textures.h"

/*********************************************************************
 * OLD - NOW USE Occlusion tests
 * Frustum calculations. Definitive work (at least IMHO) is thanks to
 * Steven Baker - look at http://sjbaker.org/steve/omniv/frustcull.html/
 *
 * Thanks Steve!
 *
 */

#undef OCCLUSIONVERBOSE

/* if we have a visible Shape node, how long should we wait until we try to determine
   if it is still visible? */
#define OCCWAIT 		20 

/* we have a visibility sensor, we want to really see when it becomes invis. */
#define OCCCHECKNEXTLOOP	1

/* we are invisible - don't let it go too long before we try to see visibility */
#define OCCCHECKSOON		4

/* how many samples of a Shape are needed before it becomes visible? If it is too
   small, don't worry about displaying it If this number is too large, "flashing" 
   will occur, as the shape is dropped, while still displaying (the number) of pixels
   on the screen */
#define OCCSHAPESAMPLESIZE	1	


/* Occlusion VisibilitySensor code */
GLuint *OccQueries = NULL;

/* newer occluder code */
GLuint potentialOccluderCount = 0;
void ** occluderNodePointer = NULL;

/* older occluder code */
static int maxOccludersFound = 0;
GLuint OccQuerySize=0;
static int QueryCount = 0;
static GLint queryCounterBits;
static int OccInitialized = FALSE;
int OccFailed = FALSE;
GLint OccResultsAvailable = FALSE;


#define PROP_EXTENT_CHECK \
		if (maxx > geomParent->EXTENT_MAX_X) {geomParent->EXTENT_MAX_X = maxx; touched = TRUE;} \
		if (minx < geomParent->EXTENT_MIN_X) {geomParent->EXTENT_MIN_X = minx; touched = TRUE;} \
		if (maxy > geomParent->EXTENT_MAX_Y) {geomParent->EXTENT_MAX_Y = maxy; touched = TRUE;} \
		if (miny < geomParent->EXTENT_MIN_Y) {geomParent->EXTENT_MIN_Y = miny; touched = TRUE;} \
		if (maxz > geomParent->EXTENT_MAX_Z) {geomParent->EXTENT_MAX_Z = maxz; touched = TRUE;} \
		if (minz < geomParent->EXTENT_MIN_Z) {geomParent->EXTENT_MIN_Z = minz; touched = TRUE;} 

#define FRUSTUM_TRANS(myNodeType)  \
	if (me->_nodeType == NODE_##myNodeType) { \
		/* have we actually done a propagateExtent on this one? Because we look at ALL points in a boundingBox, \
		   because of rotations, we HAVE to ensure that the default values are not there, otherwise we will \
		   take the "inside out" boundingBox as being correct! */ \
 \
			/* has this node actually been extented away from the default? */ \
 \
		if (!APPROX(me->EXTENT_MAX_X,-10000.0)) { \
			struct X3D_##myNodeType *node; \
			node = (struct X3D_##myNodeType *)me; \
			float my_rotation; \
			float my_scaleO; \
			float myscale; \
			Quaternion rq; \
			struct point_XYZ inxyz[8]; struct point_XYZ outxyz[8]; \
	 \
			/* make up a "cube" with vertexes being our bounding box */ \
			BBV(0,MAX_X,MAX_Y,MAX_Z); \
			BBV(1,MAX_X,MAX_Y,MIN_Z); \
			BBV(2,MAX_X,MIN_Y,MAX_Z); \
			BBV(3,MAX_X,MIN_Y,MIN_Z); \
			BBV(4,MIN_X,MAX_Y,MAX_Z); \
			BBV(5,MIN_X,MAX_Y,MIN_Z); \
			BBV(6,MIN_X,MIN_Y,MAX_Z); \
			BBV(7,MIN_X,MIN_Y,MIN_Z); \
	 \
	                /* 1: REVERSE CENTER */ \
	                if (node->__do_center) { \
	                        /* FW_GL_TRANSLATE_F(-node->center.c[0],-node->center.c[1],-node->center.c[2]); */ \
				add_translation(inxyz,-node->center.c[0],-node->center.c[1],-node->center.c[2],8); \
			} \
	 \
	                /* 2: REVERSE SCALE ORIENTATION */ \
	                if (node->__do_scaleO) { \
				my_scaleO = node->scaleOrientation.r[3]/3.1415926536*180; \
	                        /* FW_GL_ROTATE_F(-my_scaleO, node->scaleOrientation.r[0], node->scaleOrientation.r[1],node->scaleOrientation.r[2]); */ \
				vrmlrot_to_quaternion(&rq,node->scaleOrientation.r[0], node->scaleOrientation.r[1], node->scaleOrientation.r[2], -node->scaleOrientation.r[3]); \
				quaternion_multi_rotation(outxyz,&rq,inxyz,8); \
	 \
				/* copy these points back out */ \
				memcpy (inxyz,outxyz,8*sizeof(struct point_XYZ)); \
			} \
	 \
	                /* 3: SCALE */ \
	                if (node->__do_scale) { \
	                        /* FW_GL_SCALE_F(node->scale.c[0],node->scale.c[1],node->scale.c[2]); */ \
				multiply_in_scale(inxyz,node->scale.c[0],node->scale.c[1],node->scale.c[2],8); \
			} \
	 \
	                /* 4: SCALEORIENTATION */ \
	                if (node->__do_scaleO) { \
				vrmlrot_to_quaternion(&rq,node->scaleOrientation.r[0], node->scaleOrientation.r[1], node->scaleOrientation.r[2], -node->scaleOrientation.r[3]); \
				quaternion_multi_rotation(outxyz,&rq,inxyz,8); \
	 \
				/* copy these points back out */ \
				memcpy (inxyz,outxyz,8*sizeof(struct point_XYZ)); \
	                        /* FW_GL_ROTATE_F(my_scaleO, node->scaleOrientation.r[0], \
	                                node->scaleOrientation.r[1],node->scaleOrientation.r[2]); */ \
	                } \
	 \
	                /* 5: ROTATION */ \
	                if (node->__do_rotation) { \
	                        /* FW_GL_ROTATE_F(my_rotation, node->rotation.r[0],node->rotation.r[1],node->rotation.r[2]); */ \
				vrmlrot_to_quaternion(&rq,node->rotation.r[0], node->rotation.r[1], node->rotation.r[2], node->rotation.r[3]); \
				quaternion_multi_rotation(outxyz,&rq,inxyz,8); \
	 \
				/* copy these points back out */ \
				memcpy (inxyz,outxyz,8*sizeof(struct point_XYZ)); \
	                } \
	 \
	                /* 6: CENTER */ \
	                if (node->__do_center) { \
	                        /* FW_GL_TRANSLATE_F(node->center.c[0],node->center.c[1],node->center.c[2]); */ \
				add_translation(inxyz,node->center.c[0],node->center.c[1],node->center.c[2],8); \
			} \
\
	                /* 7: TRANSLATION */ \
	                if (node->__do_trans) { \
	                        /* FW_GL_TRANSLATE_F(node->translation.c[0],node->translation.c[1],node->translation.c[2]); */ \
				add_translation(inxyz,node->translation.c[0],node->translation.c[1],node->translation.c[2],8); \
			} \
	 \
	 \
			/* work changes into extent */ \
			for (i=0; i<8; i++) { \
				if (inxyz[i].x > maxx) maxx = inxyz[i].x; \
				if (inxyz[i].y > maxy) maxy = inxyz[i].y; \
				if (inxyz[i].z > maxz) maxz = inxyz[i].z; \
				if (inxyz[i].x < minx) minx = inxyz[i].x; \
				if (inxyz[i].y < miny) miny = inxyz[i].y; \
				if (inxyz[i].z < minz) minz = inxyz[i].z; \
			} \
		} \
	} 


#define FRUSTUM_GEOLOCATION  \
	if (me->_nodeType == NODE_GeoLocation) { \
		/* have we actually done a propagateExtent on this one? Because we look at ALL points in a boundingBox, \
		   because of rotations, we HAVE to ensure that the default values are not there, otherwise we will \
		   take the "inside out" boundingBox as being correct! */ \
 \
			/* has this node actually been extented away from the default? */ \
 \
		if (!APPROX(me->EXTENT_MAX_X,-10000.0)) { \
		struct X3D_GeoLocation *node; \
		node = (struct X3D_GeoLocation *)me; \
		float my_rotation; \
		float my_scaleO; \
		float myscale; \
		Quaternion rq; \
		struct point_XYZ inxyz[8]; struct point_XYZ outxyz[8]; \
	 \
		/* make up a "cube" with vertexes being our bounding box */ \
		BBV(0,MAX_X,MAX_Y,MAX_Z); \
		BBV(1,MAX_X,MAX_Y,MIN_Z); \
		BBV(2,MAX_X,MIN_Y,MAX_Z); \
		BBV(3,MAX_X,MIN_Y,MIN_Z); \
		BBV(4,MIN_X,MAX_Y,MAX_Z); \
		BBV(5,MIN_X,MAX_Y,MIN_Z); \
		BBV(6,MIN_X,MIN_Y,MAX_Z); \
		BBV(7,MIN_X,MIN_Y,MIN_Z); \
	 \
\
	        /* 5: ROTATION */ \
		vrmlrot_to_quaternion(&rq,node->__localOrient.c[0], node->__localOrient.c[1], node->__localOrient.c[2], node->__localOrient.c[3]); \
		quaternion_multi_rotation(outxyz,&rq,inxyz,8); \
	 \
		/* copy these points back out */ \
		memcpy (inxyz,outxyz,8*sizeof(struct point_XYZ)); \
	 \
                /* 7: TRANSLATION */ \
                /* FW_GL_TRANSLATE_F(node->translation.c[0],node->translation.c[1],node->translation.c[2]); */ \
		/*printf ("doing translation %f %f %f\n", node->__movedCoords.c[0],node->__movedCoords.c[1],node->__movedCoords.c[2]); */ \
		add_translation(inxyz,node->__movedCoords.c[0],node->__movedCoords.c[1],node->__movedCoords.c[2],8); \
 \
	 \
	 \
		/* work changes into extent */ \
		for (i=0; i<8; i++) { \
			if (inxyz[i].x > maxx) maxx = inxyz[i].x; \
			if (inxyz[i].y > maxy) maxy = inxyz[i].y; \
			if (inxyz[i].z > maxz) maxz = inxyz[i].z; \
			if (inxyz[i].x < minx) minx = inxyz[i].x; \
			if (inxyz[i].y < miny) miny = inxyz[i].y; \
			if (inxyz[i].z < minz) minz = inxyz[i].z; \
		} \
		} \
	} 

#define FRUSTUM_GEOTRANS  \
	if (me->_nodeType == NODE_GeoTransform) { \
		/* have we actually done a propagateExtent on this one? Because we look at ALL points in a boundingBox, \
		   because of rotations, we HAVE to ensure that the default values are not there, otherwise we will \
		   take the "inside out" boundingBox as being correct! */ \
 \
			/* has this node actually been extented away from the default? */ \
 \
		if (!APPROX(me->EXTENT_MAX_X,-10000.0)) { \
			struct X3D_GeoTransform *node; \
			node = (struct X3D_GeoTransform *)me; \
			float my_rotation; \
			float my_scaleO; \
			float myscale; \
			Quaternion rq; \
			struct point_XYZ inxyz[8]; struct point_XYZ outxyz[8]; \
	 \
			/* make up a "cube" with vertexes being our bounding box */ \
			BBV(0,MAX_X,MAX_Y,MAX_Z); \
			BBV(1,MAX_X,MAX_Y,MIN_Z); \
			BBV(2,MAX_X,MIN_Y,MAX_Z); \
			BBV(3,MAX_X,MIN_Y,MIN_Z); \
			BBV(4,MIN_X,MAX_Y,MAX_Z); \
			BBV(5,MIN_X,MAX_Y,MIN_Z); \
			BBV(6,MIN_X,MIN_Y,MAX_Z); \
			BBV(7,MIN_X,MIN_Y,MIN_Z); \
	 \
	                /* 1: REVERSE CENTER */ \
	                if (node->__do_center) { \
	                        /* FW_GL_TRANSLATE_F(-node->center.c[0],-node->center.c[1],-node->center.c[2]); */ \
				add_translation(inxyz,-node->geoCenter.c[0],-node->geoCenter.c[1],-node->geoCenter.c[2],8); \
			} \
	 \
	                /* 2: REVERSE SCALE ORIENTATION */ \
	                if (node->__do_scaleO) { \
				my_scaleO = node->scaleOrientation.r[3]/3.1415926536*180; \
	                        /* FW_GL_ROTATE_F(-my_scaleO, node->scaleOrientation.r[0], node->scaleOrientation.r[1],node->scaleOrientation.r[2]); */ \
				vrmlrot_to_quaternion(&rq,node->scaleOrientation.r[0], node->scaleOrientation.r[1], node->scaleOrientation.r[2], -node->scaleOrientation.r[3]); \
				quaternion_multi_rotation(outxyz,&rq,inxyz,8); \
	 \
				/* copy these points back out */ \
				memcpy (inxyz,outxyz,8*sizeof(struct point_XYZ)); \
			} \
	 \
	                /* 3: SCALE */ \
	                if (node->__do_scale) { \
	                        /* FW_GL_SCALE_F(node->scale.c[0],node->scale.c[1],node->scale.c[2]); */ \
				multiply_in_scale(inxyz,node->scale.c[0],node->scale.c[1],node->scale.c[2],8); \
			} \
	 \
	                /* 4: SCALEORIENTATION */ \
	                if (node->__do_scaleO) { \
				vrmlrot_to_quaternion(&rq,node->scaleOrientation.r[0], node->scaleOrientation.r[1], node->scaleOrientation.r[2], -node->scaleOrientation.r[3]); \
				quaternion_multi_rotation(outxyz,&rq,inxyz,8); \
	 \
				/* copy these points back out */ \
				memcpy (inxyz,outxyz,8*sizeof(struct point_XYZ)); \
	                        /* FW_GL_ROTATE_F(my_scaleO, node->scaleOrientation.r[0], \
	                                node->scaleOrientation.r[1],node->scaleOrientation.r[2]); */ \
	                } \
	 \
	                /* 5: ROTATION */ \
	                if (node->__do_rotation) { \
	                        /* FW_GL_ROTATE_F(my_rotation, node->rotation.r[0],node->rotation.r[1],node->rotation.r[2]); */ \
				vrmlrot_to_quaternion(&rq,node->rotation.r[0], node->rotation.r[1], node->rotation.r[2], node->rotation.r[3]); \
				quaternion_multi_rotation(outxyz,&rq,inxyz,8); \
	 \
				/* copy these points back out */ \
				memcpy (inxyz,outxyz,8*sizeof(struct point_XYZ)); \
	                } \
	 \
	                /* 6: CENTER */ \
	                if (node->__do_center) { \
	                        /* FW_GL_TRANSLATE_F(node->center.c[0],node->center.c[1],node->center.c[2]); */ \
				add_translation(inxyz,node->geoCenter.c[0],node->geoCenter.c[1],node->geoCenter.c[2],8); \
			} \
\
                	/*	FW_GL_TRANSLATE_D(node->__movedCoords.c[0], node->__movedCoords.c[1], node->__movedCoords.c[2]); */ \
                	/* my_rotation = node->__localOrient.c[3]/3.1415926536*180; */ \
                	/* GL_ROTATE_D(my_rotation, node->__localOrient.c[0],node->__localOrient.c[1],node->__localOrient.c[2]); */ \
			add_translation (inxyz,X3D_GEOTRANSFORM(node)->__movedCoords.c[0], X3D_GEOTRANSFORM(node)->__movedCoords.c[1], X3D_GEOTRANSFORM(node)->__movedCoords.c[2],8); \
\
				vrmlrot_to_quaternion(&rq,X3D_GEOTRANSFORM(node)->__localOrient.c[0], X3D_GEOTRANSFORM(node)->__localOrient.c[1], X3D_GEOTRANSFORM(node)->__localOrient.c[2], X3D_GEOTRANSFORM(node)->__localOrient.c[3]); \
				quaternion_multi_rotation(outxyz,&rq,inxyz,8); \
	 \
				/* copy these points back out */ \
				memcpy (inxyz,outxyz,8*sizeof(struct point_XYZ)); \
	 \
	 \
	                /* 7: TRANSLATION */ \
	                if (node->__do_trans) { \
	                        /* FW_GL_TRANSLATE_F(node->translation.c[0],node->translation.c[1],node->translation.c[2]); */ \
				add_translation(inxyz,node->translation.c[0],node->translation.c[1],node->translation.c[2],8); \
			} \
	 \
	 \
			/* work changes into extent */ \
			for (i=0; i<8; i++) { \
				if (inxyz[i].x > maxx) maxx = inxyz[i].x; \
				if (inxyz[i].y > maxy) maxy = inxyz[i].y; \
				if (inxyz[i].z > maxz) maxz = inxyz[i].z; \
				if (inxyz[i].x < minx) minx = inxyz[i].x; \
				if (inxyz[i].y < miny) miny = inxyz[i].y; \
				if (inxyz[i].z < minz) minz = inxyz[i].z; \
			} \
		} \
	} 




/* does this current node actually fit in the Switch rendering scheme? */
static int is_Switchchild_inrange(struct X3D_Switch *node, struct X3D_Node *me) {
        int wc = node->whichChoice;

        /* is this VRML, or X3D?? */
        if (node->__isX3D == 0) {
                if(wc >= 0 && wc < ((node->choice).n)) {
                        void *p = ((node->choice).p[wc]);
                        return (X3D_NODE(p)==me);
                }
        } else {
                if(wc >= 0 && wc < ((node->children).n)) {
                        void *p = ((node->children).p[wc]);
                        return (X3D_NODE(p)==me);
                }
        }
	return FALSE;
}

/* does this current node actually fit in the GeoLOD rendering scheme? */
static int is_GeoLODchild_inrange (struct X3D_GeoLOD* gpnode, struct X3D_Node *me) {
	/* is this node part of the active path for rendering? */
	int x,y;
	y = FALSE;

	for (x=0; x<gpnode->rootNode.n; x++) {
		/* printf ("comparing %u:%u %d of %d, types me %s rootNodeField: %s\n", 
			me, X3D_NODE(gpnode->rootNode.p[x]),
			x, gpnode->rootNode.n,
			stringNodeType (me->_nodeType),
			stringNodeType( X3D_NODE(gpnode->rootNode.p[x])->_nodeType)
			);
		*/

		if (me == X3D_NODE(gpnode->rootNode.p[x])) {
			y=TRUE;
			break;
		}
	}

/*
	if (y) printf ("GeoLOD, found child in rootNode "); else printf ("GeoLOD, child NOT part of ROOT ");
	if (X3D_GEOLOD(geomParent)->__inRange) printf ("INRANGE "); else printf ("NOT inrange ");
*/
	/* is this one actually being rendered? */
	return (y ^ gpnode->__inRange);
}


/* take the measurements of a geometry (eg, box), and save it. Note
 * that what is given is a Shape, the values get pushed up to the
 * Geometries grouping node parent. */


/* this is used for collision in transformChildren - don't bother going through
   children of a transform if there is nothing close... */

void setExtent(float maxx, float minx, float maxy, float miny, float maxz, float minz, struct X3D_Node *me) {
	int c,d;
	struct X3D_Node *shapeParent;
	struct X3D_Node *geomParent;
	double tminp, tmaxp;
	int touched;

	#ifdef FRUSTUMVERBOSE
	printf ("setExtent maxx %f minx %f maxy %f miny %f maxz %f minz %f me %u nt %s\n",
			maxx, minx, maxy, miny, maxz, minz, me, stringNodeType(me->_nodeType));
	#endif
	/* record this for ME for sorting purposes for sorting children fields */
	me->EXTENT_MAX_X = maxx; me->EXTENT_MIN_X = minx;
	me->EXTENT_MAX_Y = maxx; me->EXTENT_MIN_Y = miny;
	me->EXTENT_MAX_Z = maxx; me->EXTENT_MIN_Z = minz;

	for (c=0; c<(me->_nparents); c++) {
		shapeParent = X3D_NODE(me->_parents[c]);
	
		/* record this for ME for sorting purposes for sorting children fields */
		shapeParent->EXTENT_MAX_X = maxx; shapeParent->EXTENT_MIN_X = minx;
		shapeParent->EXTENT_MAX_Y = maxx; shapeParent->EXTENT_MIN_Y = miny;
		shapeParent->EXTENT_MAX_Z = maxx; shapeParent->EXTENT_MIN_Z = minz;
	
		#ifdef FRUSTUMVERBOSE
		if (shapeParent == NULL)
		printf ("parent %u of %u is %u, is null\n",c,me,me->_parents[c]); 
		else
		printf ("parent %u of %u is %u, type %s\n",c,me,me->_parents[c],stringNodeType(shapeParent->_nodeType)); 
		printf ("setExtent - Geometry has %d parents \n",shapeParent->_nparents);
		#endif

		for (d=0; d<(shapeParent->_nparents); d++) {
			geomParent = X3D_NODE(shapeParent->_parents[d]);

			#ifdef FRUSTUMVERBOSE
			printf ("setExtent in loop, parent %u of shape %s is %s\n",c,stringNodeType(shapeParent->_nodeType),
				stringNodeType(geomParent->_nodeType)); 
	
			/* is there a problem with this geomParent? */
			if (!checkNode(geomParent, __FILE__, __LINE__)) printf ("problem here with checkNode\n");
			#endif
	
	
			/* printf ("geomParent dist is %lf\n",geomParent->_dist); */
			/* note, maxz is positive, minz is negative, distance should be negative, so we take a negative distance,
				and subtract the "positive" z value to get the closest point, then take the negative distance,
				and subtract the "negative" z value to get the far distance */
	
			PROP_EXTENT_CHECK;
	
			#ifdef FRUSTUMVERBOSE
			printf ("setExtent - now parent %u has extent maxx %f minx %f maxy %f miny %f maxz %f minz %f\n",
					geomParent,
					geomParent->EXTENT_MAX_X ,
					geomParent->EXTENT_MIN_X ,
					geomParent->EXTENT_MAX_Y ,
					geomParent->EXTENT_MIN_Y ,
					geomParent->EXTENT_MAX_Z ,
					geomParent->EXTENT_MIN_Z);
			#endif 

		}
	}
}

void quaternion_multi_rotation(struct point_XYZ *ret, const Quaternion *quat, const struct point_XYZ * v, int count){
	int i;
	for (i=0; i<count; i++) {
		quaternion_rotation(ret, quat, v);
		ret++; v++;
	}
}


#define BBV(num,XX,YY,ZZ) \
			inxyz[num].x= (double) (me->EXTENT_##XX); \
			inxyz[num].y= (double) (me->EXTENT_##YY); \
			inxyz[num].z= (double) (me->EXTENT_##ZZ);

void add_translation (struct point_XYZ *arr,  float x, float y, float z, int count) {
	int i;
	for (i=0; i<count; i++) {
		arr->x += (double)x;
		arr->y += (double)y;
		arr->z += (double)z;
		arr++;
	}
}

void multiply_in_scale(struct point_XYZ *arr, float x, float y, float z, int count) {
	int i;
	for (i=0; i<count; i++) {
		arr->x *= (double)x;
		arr->y *= (double)y;
		arr->z *= (double)z;
		arr++;
	}
}


void printmatrix(GLdouble* mat) {
    int i;
    for(i = 0; i< 16; i++) {
	printf("mat[%d] = %4.3f%s",i,mat[i],i==3 ? "\n" : i==7? "\n" : i==11? "\n" : "");
    }
	printf ("\n");

}


/* for children nodes; set the parent grouping nodes extent - we expect the center
 * of the group to be passed in in the floats x,y,z */

void propagateExtent(struct X3D_Node *me) {
	float minx, miny, minz, maxx, maxy, maxz;
	int i;
	struct X3D_Node *geomParent;
	int touched;

	#ifdef FRUSTUMVERBOSE
	printf ("propextent Iam %s, myExtent (%4.2f %4.2f) (%4.2f %4.2f) (%4.2f %4.2f) me %u parents %d\n",
			stringNodeType(me->_nodeType),
			me->EXTENT_MAX_X, me->EXTENT_MIN_X,
			me->EXTENT_MAX_Y, me->EXTENT_MIN_Y,
			me->EXTENT_MAX_Z, me->EXTENT_MIN_Z,
			me, me->_nparents);
	#endif

	/* calculate the maximum of the current position, and add the previous extent */
	maxx = me->EXTENT_MAX_X; minx = me->EXTENT_MIN_X;
	maxy = me->EXTENT_MAX_Y; miny = me->EXTENT_MIN_Y;
	maxz = me->EXTENT_MAX_Z; minz = me->EXTENT_MIN_Z;

	/* is this a transform? Should we add in the translated position?? */
	FRUSTUM_TRANS(Transform);
	FRUSTUM_GEOTRANS;
	FRUSTUM_GEOLOCATION;
	FRUSTUM_TRANS(HAnimSite);
	FRUSTUM_TRANS(HAnimJoint);

	for (i=0; i<(me->_nparents); i++) {
		geomParent = X3D_NODE(me->_parents[i]);

		/* do we propagate for this parent? */
		touched = FALSE;

		/* switch nodes - only propagate extent back up if the node is "active" */
		switch (geomParent->_nodeType) {
			case NODE_GeoLOD: 
				if (is_GeoLODchild_inrange(X3D_GEOLOD(geomParent), me)) {
			                PROP_EXTENT_CHECK;
        			}
				break;
			case NODE_LOD: {
				/* works for both X3D and VRML syntax; compare with the "_selected" field */
				if (me == X3D_LODNODE(geomParent)->_selected) {
			                PROP_EXTENT_CHECK;
        			}
				break;
				}
			case NODE_Switch: {

				if (is_Switchchild_inrange(X3D_SWITCH(geomParent), me)) {
			                PROP_EXTENT_CHECK;
        			}
				break;
				}
			default: {
				PROP_EXTENT_CHECK;
			}

			#ifdef FRUSTUMVERBOSE
			printf ("after calcs me (%u %s) my parent %d is (%u %s) ext %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f\n",
				me, stringNodeType(me->_nodeType),i,geomParent, stringNodeType(geomParent->_nodeType),
				geomParent->EXTENT_MAX_X, geomParent->EXTENT_MIN_X,
				geomParent->EXTENT_MAX_Y, geomParent->EXTENT_MIN_Y,
				geomParent->EXTENT_MAX_Z, geomParent->EXTENT_MIN_Z);
			#endif
		}

		/* now, send these up the line, assuming this child makes the extent larger */
		if (touched) propagateExtent(geomParent); 
	}
}


void record_ZBufferDistance(struct X3D_Node *node) {
	GLdouble modelMatrix[16];
	int retval;
	int xcount, pointok;

	retval = 0;
	xcount=0;
	pointok=0;


	/* printf ("\nrecordDistance for node %u nodeType %s center %4.2f %4.2f %4.2f ",node, stringNodeType (node->_nodeType),
	node->EXTENT_MAX_X - node->EXTENT_MIN_X,
	node->EXTENT_MAX_Y - node->EXTENT_MIN_Y,
	node->EXTENT_MAX_Z - node->EXTENT_MIN_Z
	); 

if (APPROX(node->EXTENT_MAX_X,-10000.0)) printf ("EXTENT NOT INIT");

printf ("\n");
printf ("recordDistance, max,min %f:%f, %f:%f, %f:%f\n",
	node->EXTENT_MAX_X , node->EXTENT_MIN_X,
	node->EXTENT_MAX_Y , node->EXTENT_MIN_Y,
	node->EXTENT_MAX_Z , node->EXTENT_MIN_Z);

	*/

	fwGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

	/* 
	printf ("recordDistance, have modelMatrix %4.2f %4.2f %4.2f\n",modelMatrix[12],modelMatrix[13],modelMatrix[14]);
	*/

	node->_dist = modelMatrix[14];

}

/***************************************************************************/

void OcclusionStartofEventLoop() {
	int i;

	/* each time through the event loop, we count the occluders. Note, that if, say, a 
	   shape was USED 100 times, that would be 100 occlude queries, BUT ONE SHAPE, thus
	   there is not an implicit 1:1 mapping between shapes and occlude queries */

	potentialOccluderCount = 0;

	/* if the headers.h does not define OCCLUSION, skip all this */
	#ifndef OCCLUSION
	OccFailed = TRUE;
	#endif

	/* did we have a failure here ? */
	if (OccFailed) return;

	/* have we been through this yet? */
	if (OccInitialized == FALSE) {
		#ifdef OCCLUSIONVERBOSE
		printf ("initializing OcclusionCulling...\n");
		#endif

		/* do we have an environment variable for this? */
		if (getenv ("FREEWRL_NO_GL_ARB_OCCLUSION_QUERY")!= NULL) {
			printf ("FreeWRL: FREEWRL_NO_GL_ARB_OCCLUSION_QUERY set, turning off hardware Occlusion Culling\n");
			OccFailed = TRUE;
		} else {
	        	/* printf ("aqDisplayThread, extensions %s\n",glGetString(GL_EXTENSIONS));  */
	        	if (strstr((const char *)glGetString(GL_EXTENSIONS),"GL_ARB_occlusion_query") != 0) {
				#ifdef OCCLUSIONVERBOSE
	        	        printf ("OcclusionStartofEventLoop: have OcclusionQuery\n"); 
				#endif
	
				/* we make the OccQuerySize larger than the maximum number of occluders,
				   so we don't have to realloc too much */
				OccQuerySize = maxOccludersFound + 1000;

				occluderNodePointer = MALLOC (sizeof (void *) * OccQuerySize);
				OccQueries = MALLOC (sizeof(int) * OccQuerySize);
	                	glGenQueries(OccQuerySize,OccQueries);
				OccInitialized = TRUE;
				for (i=0; i<OccQuerySize; i++) {
					occluderNodePointer[i] = 0;
				}
				QueryCount = maxOccludersFound; /* for queries - we can do this number */
				#ifdef OCCLUSIONVERBOSE
				printf ("QueryCount now %d\n",QueryCount);
				#endif

        		} else {
				#ifdef OCCLUSIONVERBOSE
        	       		 printf ("OcclusionStartofEventLoop: DO NOT have OcclusionQuery\n"); 
				#endif

				/* we dont seem to have this extension here at runtime! */
				/* this happened, eg, on my Core4 AMD64 box with Mesa	*/
				OccFailed = TRUE;
				return;
			}
		}

	}

	/* did we find more shapes than before? */
	if (maxOccludersFound > QueryCount) {
        	if (maxOccludersFound > OccQuerySize) {
        	        /* printf ("have to regen queries\n"); */
			QueryCount = 0;

			/* possibly previous had zero occluders, lets just not bother deleting for zero */
			if (OccQuerySize > 0) {
				glDeleteQueries (OccQuerySize, OccQueries);
				glFlush();
			}

			OccQuerySize = maxOccludersFound + 1000;
			occluderNodePointer = REALLOC (occluderNodePointer,sizeof (void *) * OccQuerySize);
			OccQueries = REALLOC (OccQueries,sizeof (int) * OccQuerySize);
        	        glGenQueries(OccQuerySize,OccQueries);
			for (i=0; i<OccQuerySize; i++) {
				occluderNodePointer[i] = 0;
			}
		}
		QueryCount = maxOccludersFound; /* for queries - we can do this number */
		#ifdef OCCLUSIONVERBOSE
		printf ("QueryCount here is %d\n",QueryCount);
		#endif

       }


	#ifdef OCCLUSIONVERBOSE
        glGetQueryiv(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &queryCounterBits);
        printf ("queryCounterBits %d\n",queryCounterBits);
        #endif
}

void OcclusionCulling ()  {
#ifdef OCCLUSION
	int i;
	int maxcount;
	struct X3D_Shape *shapePtr;
	struct X3D_VisibilitySensor *visSenPtr;
	int checkCount;
	GLint samples;


#ifdef OCCLUSIONVERBOSE
	GLint query;
	glGetQueryiv(GL_SAMPLES_PASSED, GL_CURRENT_QUERY, &query);
	printf ("currentQuery is %d\n",query);
#endif

	
	/* Step 0. go through list of assigned nodes, and either:
		- if we have OcclusionQueries: REMOVE the VF_hasVisibleChildren flag;
		- else, set every node to VF_hasVisibleChildren */
	zeroVisibilityFlag();

	/* Step 1. did we have some problem with Occlusion ? */
	if (OccFailed) return;
	 
	/* Step 2. go through the list of "OccludeCount" nodes, and determine if they are visible. 
	   If they are not, then, we have to, at some point, make them visible, so that we can test again. */
	/* note that the potentialOccluderCount is only incremented if the __occludeCheckCount tells us
	   that it should be checked again - see the interplay between the eventLoop stuff in OpenGLUtils.c
 	   and the OCCLUSION* defines in headers.h - we DO NOT generate a query every time through the loop */
 
	#ifdef OCCLUSIONVERBOSE
	printf ("OcclusionCulling - potentialOccluderCount %d\n",potentialOccluderCount);
	#endif

	for (i=0; i<potentialOccluderCount; i++) {
		#ifdef OCCLUSIONVERBOSE
		printf ("checking node %d of %d\n",i, potentialOccluderCount);
		#endif

		checkCount = 0;

		/* get the check count field for this node - see if we did a check of this */
		shapePtr = X3D_SHAPE(occluderNodePointer[i]);
		if (shapePtr->_nodeType == NODE_Shape) {
			visSenPtr = NULL;
			checkCount = shapePtr->__occludeCheckCount;
		} else if (shapePtr->_nodeType == NODE_VisibilitySensor) {
			visSenPtr = X3D_VISIBILITYSENSOR(occluderNodePointer[i]);
			shapePtr = NULL;
			checkCount = visSenPtr->__occludeCheckCount;
		} else {
			printf ("OcclusionCulling on node type %s not allowed\n",stringNodeType(shapePtr->_nodeType));
			return;
		}

		#ifdef OCCLUSIONVERBOSE
		printf ("OcclusionCulling, for a %s (index %d) checkCount %d\n",stringNodeType(shapePtr->_nodeType),i,checkCount);
		#endif

		/* an Occlusion test will have been run on this one */

		glGetQueryObjectiv(OccQueries[i],GL_QUERY_RESULT_AVAILABLE,&OccResultsAvailable);
		glPrintError ("glGetQueryObjectiv::QUERY_RESULTS_AVAIL");

		#ifdef SLEEP_FOR_QUERY_RESULTS
		/* for now, lets loop to see when we get results */
		while (OccResultsAvailable == GL_FALSE) {
			printf ("waiting and looping for results\n");
			usleep(100);
			glGetQueryObjectiv(OccQueries[i],GL_QUERY_RESULT_AVAILABLE,&OccResultsAvailable);
			glPrintError ("glGetQueryObjectiv::QUERY_RESULTS_AVAIL");
		}
		#endif


		#ifdef OCCLUSIONVERBOSE
		if (OccResultsAvailable == GL_FALSE) printf ("results not ready for %d\n",i);
		#endif


		/* if we are NOT ready; we keep the count going, but we do NOT change the results of VisibilitySensors */
		if (OccResultsAvailable == GL_FALSE) samples = 10000;  
			
	        glGetQueryObjectiv (OccQueries[i], GL_QUERY_RESULT, &samples);
		glPrintError ("glGetQueryObjectiv::QUERY");
				
		#ifdef OCCLUSIONVERBOSE
		printf ("i %d checkc %d samples %d\n",i,checkCount,samples);
		#endif
	
		if (occluderNodePointer[i] != 0) {
		
			/* if this is a VisibilitySensor, record the samples */
			if (visSenPtr != NULL) {

				#ifdef OCCLUSIONVERBOSE
				printf ("OcclusionCulling, found VisibilitySensor at %d, fragments %d active %d\n",i,samples,checkCount);
				#endif

				/* if this is a DEF/USE, we might already have done this one, as we have same
				   node pointer used in other places. */
				if (checkCount != OCCCHECKNEXTLOOP) {
	
					if (samples > 0) {
						visSenPtr->__visible  = TRUE;
						visSenPtr->__occludeCheckCount = OCCCHECKNEXTLOOP; /* look for this EVERY time through */
						visSenPtr->__Samples = samples;
					} else {
						visSenPtr->__occludeCheckCount = OCCCHECKSOON; /* check again soon */
						visSenPtr->__visible =FALSE;
						visSenPtr->__Samples = 0;
					}
				 /* } else {
					printf ("shape, already have checkCount == OCCCHECKNEXTLOOP, not changing visibility params\n");
				*/
				
				}
			}
		
		
			/* is this is Shape? */
			else if (shapePtr != NULL) {
				#ifdef OCCLUSIONVERBOSE
				printf ("OcclusionCulling, found Shape %d, fragments %d active %d\n",i,samples,checkCount);
				#endif

				/* if this is a DEF/USE, we might already have done this one, as we have same
				   node pointer used in other places. */
				if (checkCount != OCCWAIT) {
	
					/* is this node visible? If so, tell the parents! */
					if (samples > OCCSHAPESAMPLESIZE) {
						/* printf ("Shape is VISIBLE\n"); */
						shapePtr->__visible = TRUE;
						shapePtr->__occludeCheckCount= OCCWAIT; /* wait a little while before checking again */
						shapePtr->__Samples = samples;
					} else {
						/* printf ("Shape is NOT VISIBLE\n"); */
						shapePtr->__visible=FALSE;
						shapePtr->__occludeCheckCount = OCCCHECKSOON; /* check again soon */
						shapePtr->__Samples = 0; 
					}
				/* } else {
					printf ("shape, already have checkCount == OCCWAIT, not changing visibility params\n");
				*/	
				}
			}
		}
	}
#endif
}

/* shut down the occlusion stuff */
void zeroOcclusion(void) {
	if (OccFailed) return;

	QueryCount = 0;
	glDeleteQueries (OccQuerySize, OccQueries);
	glFlush();
	
	OccQuerySize=0;
	maxOccludersFound = 0;
	OccInitialized = FALSE;
	FREE_IF_NZ(OccQueries);
	FREE_IF_NZ(occluderNodePointer);
}
