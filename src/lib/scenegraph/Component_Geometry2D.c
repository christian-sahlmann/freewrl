/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Geometry2D.c,v 1.32 2011/05/28 17:11:42 dug9 Exp $

X3D Geometry2D  Component

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

#include "Collision.h"
#include "LinearAlgebra.h"
#include "../opengl/Frustum.h"
#include "../opengl/Material.h"
#include "../opengl/Textures.h"
#include "Component_Geometry3D.h"
#include "../opengl/OpenGL_Utils.h"

#include "Component_Shape.h"
#include "../scenegraph/RenderFuncs.h"

#include <float.h>
#if defined(_MSC_VER) && _MSC_VER < 1500
#define cosf cos
#define sinf sin
#endif
#define SEGMENTS_PER_CIRCLE 36
#define PIE 10
#define CHORD 20
#define NONE 30

static void *createLines (float start, float end, float radius, int closed, int *size,float *_extent);

#define COMPILE_AND_GET_BOUNDS(myType,myField) \
void compile_##myType (struct X3D_##myType *node){ \
	float myminx = FLT_MAX; \
	float mymaxx = FLT_MIN; \
	float myminy = FLT_MAX; \
	float mymaxy = FLT_MIN; \
	int count; \
 \
	if (node->myField.n<=0) { \
		node->EXTENT_MIN_X = 0.0f; \
		node->EXTENT_MAX_X = 0.0f; \
		node->EXTENT_MIN_Y = 0.0f; \
		node->EXTENT_MAX_Y = 0.0f; \
	} else { \
		for (count = 0; count < node->myField.n; count++) { \
			if (node->myField.p[count].c[0] > mymaxx) mymaxx = node->myField.p[count].c[0]; \
			if (node->myField.p[count].c[0] < myminx) myminx = node->myField.p[count].c[0]; \
			if (node->myField.p[count].c[1] > mymaxy) mymaxy = node->myField.p[count].c[1]; \
			if (node->myField.p[count].c[1] < myminy) myminy = node->myField.p[count].c[1]; \
		} \
		node->EXTENT_MAX_X = mymaxx; \
		node->EXTENT_MIN_X = myminx; \
		node->EXTENT_MAX_Y = mymaxy; \
		node->EXTENT_MIN_Y = myminy; \
	} \
 \
	MARK_NODE_COMPILED \
}
/***********************************************************************************/

void compile_Arc2D (struct X3D_Arc2D *node) {
       /*  have to regen the shape*/
	struct SFVec2f *tmpptr_a, *tmpptr_b;
	int tmpint;

	MARK_NODE_COMPILED
	
	tmpint = 0;
	tmpptr_a = createLines (node->startAngle, node->endAngle, node->radius, NONE, &tmpint, node->_extent);

	/* perform the switch - worry about threading here without locking */
	node->__numPoints = 0;		/* tell us that it has zero points */
	tmpptr_b = node->__points.p;	/* old set of points, for freeing later */
	node->__points.p = tmpptr_a;	/* new points */
	node->__numPoints = tmpint;
	FREE_IF_NZ (tmpptr_b);
	/* switch completed */
	
}

void render_Arc2D (struct X3D_Arc2D *node) {
	DEFAULT_COLOUR_POINTER
	COMPILE_IF_REQUIRED
	if (node->__numPoints>0) {	
		/* for BoundingBox calculations */
		setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, 
			node->EXTENT_MAX_Y, node->EXTENT_MIN_Y, 0.0f,0.0f,X3D_NODE(node));

		GET_COLOUR_POINTER
	        LIGHTING_OFF
	        DISABLE_CULL_FACE
		DO_COLOUR_POINTER

		FW_GL_DISABLECLIENTSTATE (GL_NORMAL_ARRAY);
		FW_GL_VERTEX_POINTER (2,GL_FLOAT,0,(GLfloat *)node->__points.p);
        	FW_GL_DRAWARRAYS (GL_LINE_STRIP, 0, node->__numPoints);
		FW_GL_ENABLECLIENTSTATE (GL_NORMAL_ARRAY);
		trisThisLoop += node->__numPoints;
	}
}

/***********************************************************************************/

void compile_ArcClose2D (struct X3D_ArcClose2D *node) {
	int xx;
	char *ct;
	struct SFVec2f *tmpptr_a, *tmpptr_b;
	int tmpint;

        /*  have to regen the shape*/
	MARK_NODE_COMPILED
		
	ct = node->closureType->strptr;
	xx = node->closureType->len;
	tmpint = 0;
	tmpptr_a = NULL;

	if (strcmp(ct,"PIE") == 0) {
		tmpptr_a = createLines (node->startAngle,
			node->endAngle, node->radius, PIE, &tmpint,node->_extent);
	} else if (strcmp(ct,"CHORD") == 0) {
		tmpptr_a = createLines (node->startAngle,
			node->endAngle, node->radius, CHORD, &tmpint,node->_extent);
	} else {
		printf ("ArcClose2D, closureType %s invalid\n",node->closureType->strptr);
	}

	/* perform the switch - worry about threading here without locking */
	node->__numPoints = 0;		/* tell us that it has zero points */
	tmpptr_b = node->__points.p;	/* old set of points, for freeing later */
	node->__points.p = tmpptr_a;	/* new points */
	node->__numPoints = tmpint;
	FREE_IF_NZ (tmpptr_b);
	/* switch completed */
}


void render_ArcClose2D (struct X3D_ArcClose2D *node) {
	DEFAULT_COLOUR_POINTER
	COMPILE_IF_REQUIRED
	if (node->__numPoints>0) {	
		/* for BoundingBox calculations */
		setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, 
			node->EXTENT_MAX_Y, node->EXTENT_MIN_Y, 0.0f,0.0f,X3D_NODE(node));

		GET_COLOUR_POINTER
	        LIGHTING_OFF
	        DISABLE_CULL_FACE
		DO_COLOUR_POINTER

		FW_GL_DISABLECLIENTSTATE (GL_NORMAL_ARRAY);
		FW_GL_VERTEX_POINTER (2,GL_FLOAT,0,(GLfloat *)node->__points.p);
        	FW_GL_DRAWARRAYS (GL_LINE_STRIP, 0, node->__numPoints);
		FW_GL_ENABLECLIENTSTATE (GL_NORMAL_ARRAY);
		trisThisLoop += node->__numPoints;
	}
}

/***********************************************************************************/

void compile_Circle2D (struct X3D_Circle2D *node) {
	struct SFVec2f *tmpptr_a, *tmpptr_b;
	int tmpint;

       /*  have to regen the shape*/
	MARK_NODE_COMPILED
		
	tmpptr_a = createLines (0.0f, 0.0f, node->radius, NONE, &tmpint,node->_extent);

	/* perform the switch - worry about threading here without locking */
	node->__numPoints = 0;		/* tell us that it has zero points */
	tmpptr_b = node->__points.p;	/* old set of points, for freeing later */
	node->__points.p = tmpptr_a;	/* new points */
	node->__numPoints = tmpint;
	FREE_IF_NZ (tmpptr_b);
	/* switch completed */
}

void render_Circle2D (struct X3D_Circle2D *node) {
	DEFAULT_COLOUR_POINTER
	COMPILE_IF_REQUIRED
	if (node->__numPoints>0) {	
		/* for BoundingBox calculations */
		setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, 
			node->EXTENT_MAX_Y, node->EXTENT_MIN_Y, 0.0f,0.0f,X3D_NODE(node));

		GET_COLOUR_POINTER
	        LIGHTING_OFF
	        DISABLE_CULL_FACE
		DO_COLOUR_POINTER

		
		FW_GL_DISABLECLIENTSTATE (GL_NORMAL_ARRAY);
		FW_GL_VERTEX_POINTER (2,GL_FLOAT,0,(GLfloat *)node->__points.p);
        	FW_GL_DRAWARRAYS (GL_LINE_STRIP, 0, node->__numPoints);
		FW_GL_ENABLECLIENTSTATE (GL_NORMAL_ARRAY);
		trisThisLoop += node->__numPoints;
	}
}

/***********************************************************************************/


COMPILE_AND_GET_BOUNDS(Polyline2D,lineSegments)

void render_Polyline2D (struct X3D_Polyline2D *node){
	DEFAULT_COLOUR_POINTER

	COMPILE_IF_REQUIRED
	if (node->lineSegments.n>0) {
		/* for BoundingBox calculations */
		setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, 
			node->EXTENT_MAX_Y, node->EXTENT_MIN_Y, 0.0f,0.0f,X3D_NODE(node));

		GET_COLOUR_POINTER
	        LIGHTING_OFF
	        DISABLE_CULL_FACE
		DO_COLOUR_POINTER

		FW_GL_DISABLECLIENTSTATE (GL_NORMAL_ARRAY);
		FW_GL_VERTEX_POINTER (2,GL_FLOAT,0,(GLfloat *)node->lineSegments.p);
        	FW_GL_DRAWARRAYS (GL_LINE_STRIP, 0, node->lineSegments.n);
		FW_GL_ENABLECLIENTSTATE (GL_NORMAL_ARRAY);
		trisThisLoop += node->lineSegments.n;
	}
}

/***********************************************************************************/

COMPILE_AND_GET_BOUNDS(Polypoint2D,point)

void render_Polypoint2D (struct X3D_Polypoint2D *node){
	DEFAULT_COLOUR_POINTER

	COMPILE_IF_REQUIRED
	if (node->point.n>0) {
		/* for BoundingBox calculations */
		setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, 
			node->EXTENT_MAX_Y, node->EXTENT_MIN_Y, 0.0f,0.0f,X3D_NODE(node));

		GET_COLOUR_POINTER
	        LIGHTING_OFF
	        DISABLE_CULL_FACE
		DO_COLOUR_POINTER



		FW_GL_DISABLECLIENTSTATE (GL_NORMAL_ARRAY);
		FW_GL_VERTEX_POINTER (2,GL_FLOAT,0,(GLfloat *)node->point.p);
        	FW_GL_DRAWARRAYS (GL_POINTS, 0, node->point.n);
		FW_GL_ENABLECLIENTSTATE (GL_NORMAL_ARRAY);
		trisThisLoop += node->point.n;
	}
}

/***********************************************************************************/

void compile_Disk2D (struct X3D_Disk2D *node){
        /*  have to regen the shape*/
	struct SFVec2f *fp;
	GLfloat *tp;
	struct SFVec2f *sfp;
	GLfloat *stp;
	struct SFVec2f *ofp;
	GLfloat *otp;
	int i;
	GLfloat id;
	GLfloat od;
	int tmpint;
	int simpleDisc;

	MARK_NODE_COMPILED


	/* bounds checking */
	if (node->innerRadius<0) {node->__numPoints = 0; return;}
	if (node->outerRadius<0) {node->__numPoints = 0; return;}

	/* is this a simple disc ? */
	if ((APPROX (node->innerRadius, 0.0)) || 
		(APPROX(node->innerRadius,node->outerRadius))) simpleDisc = TRUE;
	else simpleDisc = FALSE;

	/* is this a simple disk, or one with an inner circle cut out? */
	if (simpleDisc) {
		tmpint = SEGMENTS_PER_CIRCLE+2;
		fp = sfp = MALLOC (struct SFVec2f *, sizeof(struct SFVec2f) * (tmpint));
		tp = stp = MALLOC (GLfloat *, sizeof(GLfloat) * 2 * (tmpint));

		/* initial TriangleFan point */
		(*fp).c[0] = 0.0f; (*fp).c[1] = 0.0f; fp++;
		*tp = 0.5f; tp++; *tp = 0.5f; tp++;
		id = 2.0f;

		for (i=SEGMENTS_PER_CIRCLE; i >= 0; i--) {
			(*fp).c[0] = node->outerRadius * sinf((PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));
			(*fp).c[1] = node->outerRadius * cosf((PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));	
			fp++;
			*tp = 0.5f + (sinf((PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE))/id);	tp++;
			*tp = 0.5f + (cosf((PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE))/id);	tp++;
		}
	} else {
		tmpint = (SEGMENTS_PER_CIRCLE+1) * 2;
		fp = sfp = MALLOC (struct SFVec2f *, sizeof(struct SFVec2f) * 2 * tmpint);
		tp = stp = MALLOC (GLfloat *, sizeof(GLfloat) * 2 * tmpint);


		/* texture scaling params */
		od = 2.0f;
		id = node->outerRadius * 2.0f / node->innerRadius;

		for (i=SEGMENTS_PER_CIRCLE; i >= 0; i--) {
			(*fp).c[0] = node->innerRadius * (float) sinf((PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));
			(*fp).c[1] = node->innerRadius * (float) cosf((PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));	
			fp++;
			(*fp).c[0] = node->outerRadius * (float) sinf((PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));
			(*fp).c[1] = node->outerRadius * (float) cosf((PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));	
			fp++;
			*tp = 0.5f + ((float)sinf((PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE))/id);	tp++;
			*tp = 0.5f + ((float)cosf((PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE))/id);	tp++;
			*tp = 0.5f + ((float)sinf((PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE))/od);	tp++;
			*tp = 0.5f + ((float)cosf((PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE))/od);	tp++;
		}
	}


	/* compiling done, set up for rendering. thread safe */
	node->__numPoints = 0;
	ofp = node->__points.p;
	otp = node->__texCoords;
	node->__points.p = sfp;
	node->__texCoords = stp;
	node->__simpleDisk = simpleDisc;
	node->__numPoints = tmpint;
	FREE_IF_NZ (ofp);
	FREE_IF_NZ (otp);

	/* we can set the extents here... */
	node->EXTENT_MAX_X = node->outerRadius;
	node->EXTENT_MIN_X = -node->outerRadius;
	node->EXTENT_MAX_Y = node->outerRadius;
	node->EXTENT_MIN_Y = -node->outerRadius;
}

void render_Disk2D (struct X3D_Disk2D *node){
	COMPILE_IF_REQUIRED
	if (node->__numPoints>0) {	
		struct textureVertexInfo mtf = {(GLfloat *)node->__texCoords,2,GL_FLOAT,0,NULL};
		/* for BoundingBox calculations */
		setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, 
			node->EXTENT_MAX_Y, node->EXTENT_MIN_Y, 0.0f,0.0f,X3D_NODE(node));

		CULL_FACE(node->solid)

		textureDraw_start(NULL,&mtf);
		FW_GL_VERTEX_POINTER (2,GL_FLOAT,0,(GLfloat *)node->__points.p);
		FW_GL_DISABLECLIENTSTATE (GL_NORMAL_ARRAY);
#if !(defined(IPHONE) || defined(_ANDROID))
		FW_GL_NORMAL3F (0.0f, 0.0f, 1.0f);
#endif

		/* do the array drawing; sides are simple 0-1-2-3, 4-5-6-7, etc quads */
		if (node->__simpleDisk) {FW_GL_DRAWARRAYS (GL_TRIANGLE_FAN, 0, node->__numPoints);}
#if !defined(IPHONE) && !defined(_ANDROID)
		else 			{FW_GL_DRAWARRAYS (GL_QUAD_STRIP, 0, node->__numPoints); }
#endif

		textureDraw_end();
		FW_GL_ENABLECLIENTSTATE (GL_NORMAL_ARRAY);
		trisThisLoop += node->__numPoints;
	}
}

/***********************************************************************************/

void compile_TriangleSet2D (struct X3D_TriangleSet2D *node){
        /*  have to regen the shape*/
	GLfloat maxX, minX;
	GLfloat maxY, minY;
	GLfloat Ssize, Tsize;
	int i;
	GLfloat *fp;
	int tmpint;

	MARK_NODE_COMPILED

	/* do we have vertex counts in sets of 3? */
	if ((node->vertices.n %3) != 0) {
		printf ("TriangleSet2D, have incorrect vertex count, %d\n",node->vertices.n);
		node->vertices.n -= node->vertices.n % 3;
	}

	/* save this, and tell renderer that this has 0 vertices (threading stuff) */
	tmpint = node->vertices.n;
	node->vertices.n = 0;

	/* ok, now if renderer renders (threading) it'll see zero, so we are safe */
	FREE_IF_NZ (node->__texCoords);
	node->__texCoords = fp = MALLOC (GLfloat *, sizeof (GLfloat) * tmpint * 2);

	/* find min/max values for X and Y axes */
	minY = minX = FLT_MAX;
	maxY = maxX = FLT_MIN;
	for (i=0; i<tmpint; i++) {
		if (node->vertices.p[i].c[0] < minX) minX = node->vertices.p[i].c[0];
		if (node->vertices.p[i].c[1] < minY) minY = node->vertices.p[i].c[1];
		if (node->vertices.p[i].c[0] > maxX) maxX = node->vertices.p[i].c[0];
		if (node->vertices.p[i].c[1] > maxY) maxY = node->vertices.p[i].c[1];
	}

	/* save these numbers for extents */
	node->EXTENT_MAX_X = maxX;
	node->EXTENT_MIN_X = minX;
	node->EXTENT_MAX_Y = maxY;
	node->EXTENT_MIN_Y = minY;

	/* printf ("minX %f maxX %f minY %f maxY %f\n",minX, maxX, minY, maxY); */
	Ssize = maxX - minX;
	Tsize = maxY - minY;
	/* printf ("ssize %f tsize %f\n",Ssize, Tsize); */

	for (i=0; i<tmpint; i++) {
		*fp = (node->vertices.p[i].c[0] - minX) / Ssize; fp++;
		*fp = (node->vertices.p[i].c[1] - minY) / Tsize; fp++;
	}

	/* restore, so we know how many tris there are */
	node->vertices.n = tmpint;
}

void render_TriangleSet2D (struct X3D_TriangleSet2D *node){
	COMPILE_IF_REQUIRED
	if (node->vertices.n>0) {	
		struct textureVertexInfo mtf = {(GLfloat *)node->__texCoords,2,GL_FLOAT,0,NULL};
		/* for BoundingBox calculations */
		setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, 
			node->EXTENT_MAX_Y, node->EXTENT_MIN_Y, 0.0f,0.0f,X3D_NODE(node));

		CULL_FACE(node->solid)

		textureDraw_start(NULL,&mtf);
		FW_GL_VERTEX_POINTER (2,GL_FLOAT,0,(GLfloat *)node->vertices.p);
		FW_GL_DISABLECLIENTSTATE (GL_NORMAL_ARRAY);
#if defined(IPHONE) || defined(_ANDROID)
printf ("render_TriangleSet2D, this should be VBOd\n");
#else
		FW_GL_NORMAL3F (0.0f, 0.0f, 1.0f);
#endif

		FW_GL_DRAWARRAYS (GL_TRIANGLES, 0, node->vertices.n);

		textureDraw_end();
		FW_GL_ENABLECLIENTSTATE (GL_NORMAL_ARRAY);
		trisThisLoop += node->vertices.n;
	}
}


/***********************************************************************************/


/* this code is remarkably like Box, but with a zero z axis. */
void compile_Rectangle2D (struct X3D_Rectangle2D *node) {
	float *pt;
	struct SFVec3f *ptr;
	float x = ((node->size).c[0])/2;
	float y = ((node->size).c[1])/2;

	MARK_NODE_COMPILED

	/*  MALLOC memory (if possible)*/
	if (!node->__points.p) ptr = MALLOC (struct SFVec3f *,sizeof(struct SFVec3f)*(6));
	else ptr = node->__points.p;

	/*  now, create points; 6 points per face.*/
	pt = (float *) ptr;
	#define PTF0 *pt++ =  x; *pt++ =  y; *pt++ =  0.0f;
	#define PTF1 *pt++ = -x; *pt++ =  y; *pt++ =  0.0f;
	#define PTF2 *pt++ = -x; *pt++ = -y; *pt++ =  0.0f;
	#define PTF3 *pt++ =  x; *pt++ = -y; *pt++ =  0.0f;

	PTF0 PTF1 PTF2  PTF0 PTF2 PTF3 /* front */
	/* finished, and have good data */
	node->__points.p = (struct SFVec3f*) ptr;

	#undef PTF0
	#undef PTF1
	#undef PTF2
	#undef PTF3
}


void render_Rectangle2D (struct X3D_Rectangle2D *node) {
	extern GLfloat boxtex[];		/*  in CFuncs/statics.c*/
	extern GLfloat boxnorms[];		/*  in CFuncs/statics.c*/
	struct textureVertexInfo mtf = {boxtex,2,GL_FLOAT,0,NULL};
	
	float x = ((node->size).c[0])/2;
	float y = ((node->size).c[1])/2;

	/* test for <0 of sides */
	if ((x < 0) || (y < 0)) return;

	COMPILE_IF_REQUIRED
	if (!node->__points.p) return; /* still compiling */

	/* for BoundingBox calculations */
	setExtent(x,-x,y,-y,0.0f,0.0f,X3D_NODE(node));

	CULL_FACE(node->solid)

	/*  Draw it; assume VERTEX and NORMALS already defined.*/
	textureDraw_start(NULL,&mtf);
	FW_GL_VERTEX_POINTER (3,GL_FLOAT,0,(GLfloat *)node->__points.p);
	FW_GL_NORMAL_POINTER (GL_FLOAT,0,boxnorms);

	/* do the array drawing; sides are simple 0-1-2-3, 4-5-6-7, etc quads */
	FW_GL_DRAWARRAYS (GL_TRIANGLES, 0, 6);
	textureDraw_end();
	trisThisLoop += 2;
}

/***********************************************************************************/

static void *createLines (float start, float end, float radius, int closed, int *size, float *_extent) {
	int i;
	int isCircle;
	int numPoints;
	GLfloat tmp;
	GLfloat *points;
	GLfloat *fp;
	int arcpoints;

        float myminx = FLT_MAX;
        float mymaxx = FLT_MIN;
        float myminy = FLT_MAX;
        float mymaxy = FLT_MIN;

	*size = 0;

	/* is this a circle? */
	isCircle =  APPROX(start,end);

	/* bounds check, and sort values */
	if ((start < PI*2.0) || (start > PI*2.0)) start = 0.0f;
	if ((end < PI*2.0) || (end > PI*2.0)) end = (float) (PI/2.0);
	if (radius<0.0) radius = 1.0f;

	if (end > start) {
		tmp = start;
		start = end;
		end = tmp;
	}
		

	if (isCircle) {
		numPoints = SEGMENTS_PER_CIRCLE;
		closed = NONE; /* this is a circle, CHORD, PIE dont mean anything now */
	} else {
		numPoints = (int) ((float)(SEGMENTS_PER_CIRCLE * (start-end))/(PI*2.0f));
		if (numPoints>SEGMENTS_PER_CIRCLE) numPoints=SEGMENTS_PER_CIRCLE;
	}

	/* we always have to draw the line - we have a line strip, and we calculate
	   the beginning points; we have also to calculate the ending point. */
	numPoints++;
	arcpoints = numPoints;

	/* closure type */
	if (closed == CHORD) numPoints++;
	if (closed == PIE) numPoints+=2;

	points = MALLOC (float *, sizeof(float)*numPoints*2);
	fp = points;

	for (i=0; i<arcpoints; i++) {
		*fp = -radius * sinf((PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
		*fp = radius * cosf((PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
	}

	/* do we have to draw any pies, cords, etc, etc? */
	if (closed == CHORD) {
		/* loop back to origin */
		*fp = -radius * sinf(0.0f/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
		*fp = radius * cosf(0.0f/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
	} else if (closed == PIE) {
		/* go to origin */
		*fp = 0.0f; fp++; *fp=0.0f; fp++; 
		*fp = -radius * sinf(0.0f/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
		*fp = radius * cosf(0.0f/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
	}

		
	/* find extents */
	*size = numPoints;
	if (numPoints==0) {
                EXTENT_MAX_X = 0.0f;
                EXTENT_MIN_X = 0.0f;
                EXTENT_MAX_Y = 0.0f;
                EXTENT_MIN_Y = 0.0f;
        } else { 
		/* find min/max for setExtent for these points */
		fp = points;
		for (i=0; i<numPoints; i++) {
			/* do X first */
                        if (*fp > mymaxx) mymaxx = *fp;
                        if (*fp < myminx) myminx = *fp;
			fp++;
			/* do Y second */
                        if (*fp > mymaxy) mymaxy = *fp;
                        if (*fp < myminy) myminy = *fp;
			fp++;
		}
		EXTENT_MIN_X = myminx;
		EXTENT_MAX_X = mymaxx;
		EXTENT_MIN_Y = myminy;
		EXTENT_MAX_Y = mymaxy;
	}

	return (void *)points;
}




void collide_TriangleSet2D (struct X3D_TriangleSet2D *node) {
	UNUSED (node);
}

void collide_Disk2D (struct X3D_Disk2D *node) {
	UNUSED (node);
}

void collide_Rectangle2D (struct X3D_Rectangle2D *node) {
		/* Modified Box code. */

	       /*easy access, naviinfo.step unused for sphere collisions */
	       GLDOUBLE awidth = naviinfo.width; /*avatar width*/
	       GLDOUBLE atop = naviinfo.width; /*top of avatar (relative to eyepoint)*/
	       GLDOUBLE abottom = -naviinfo.height; /*bottom of avatar (relative to eyepoint)*/
	       GLDOUBLE astep = -naviinfo.height+naviinfo.step;

	       GLDOUBLE modelMatrix[16];
	       //GLDOUBLE upvecmat[16];
	       struct point_XYZ iv = {0,0,0};
	       struct point_XYZ jv = {0,0,0};
	       struct point_XYZ kv = {0,0,0};
	       struct point_XYZ ov = {0,0,0};

	       struct point_XYZ delta;

		iv.x = node->size.c[0];
		jv.y = node->size.c[1]; 
		kv.z = 0.0;
		ov.x = -((node->size).c[0])/2; ov.y = -((node->size).c[1])/2; ov.z = 0.0;

	       /* get the transformed position of the Box, and the scale-corrected radius. */
	       FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix);

			matmultiply(modelMatrix,FallInfo.avatar2collision,modelMatrix); 

		   {
			   /*  minimum bounding box MBB test in avatar/collision space */
				GLDOUBLE shapeMBBmin[3], shapeMBBmax[3];
				int i;
				for(i=0;i<3;i++)
				{
					shapeMBBmin[i] = DOUBLE_MIN(-(node->size).c[i]*.5,(node->size).c[i]*.5);
					shapeMBBmax[i] = DOUBLE_MAX(-(node->size).c[i]*.5,(node->size).c[i]*.5);
				}
				if(!avatarCollisionVolumeIntersectMBB(modelMatrix, shapeMBBmin, shapeMBBmax))return;
		   }
	       /* get transformed box edges and position */
	       transform(&ov,&ov,modelMatrix);
	       transform3x3(&iv,&iv,modelMatrix);
	       transform3x3(&jv,&jv,modelMatrix);
	       transform3x3(&kv,&kv,modelMatrix);

	       delta = box_disp(abottom,atop,astep,awidth,ov,iv,jv,kv);

	       vecscale(&delta,&delta,-1);

	       accumulate_disp(&CollisionInfo,delta);


		#ifdef COLLISIONVERBOSE
	       if((fabs(delta.x) != 0. || fabs(delta.y) != 0. || fabs(delta.z) != 0.))
	           printf("COLLISION_BOX: (%f %f %f) (%f %f %f)\n",
			  ov.x, ov.y, ov.z,
			  delta.x, delta.y, delta.z
			  );
	       if((fabs(delta.x != 0.) || fabs(delta.y != 0.) || fabs(delta.z) != 0.))
	           printf("iv=(%f %f %f) jv=(%f %f %f) kv=(%f %f %f)\n",
			  iv.x, iv.y, iv.z,
			  jv.x, jv.y, jv.z,
			  kv.x, kv.y, kv.z
			  );
		#endif
}
