/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Geometry2D.c,v 1.7 2009/07/22 19:30:03 crc_canada Exp $

X3D Geometry2D  Component

*/

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

#define SEGMENTS_PER_CIRCLE 36
#define PIE 10
#define CHORD 20
#define NONE 30

void *createLines (float start, float end, float radius, int closed, int *size);

/***********************************************************************************/

void compile_Arc2D (struct X3D_Arc2D *node) {
       /*  have to regen the shape*/
	void *tmpptr_a, *tmpptr_b;
	int tmpint;

	MARK_NODE_COMPILED
	
	tmpint = 0;
	tmpptr_a = createLines (node->startAngle, node->endAngle, node->radius, NONE, &tmpint);

	/* perform the switch - worry about threading here without locking */
	node->__numPoints = 0;		/* tell us that it has zero points */
	tmpptr_b = node->__points;	/* old set of points, for freeing later */
	node->__points = tmpptr_a;	/* new points */
	node->__numPoints = tmpint;
	FREE_IF_NZ (tmpptr_b);
	/* switch completed */
	
}

void render_Arc2D (struct X3D_Arc2D *node) {
	COMPILE_IF_REQUIRED
	if (node->__numPoints>0) {	
	        LIGHTING_OFF
	        DISABLE_CULL_FACE
		glColor3f (1.0, 1.0, 1.0);

		glDisableClientState (GL_NORMAL_ARRAY);
		glVertexPointer (2,GL_FLOAT,0,(GLfloat *)node->__points);
        	FW_GL_DRAWARRAYS (GL_LINE_STRIP, 0, node->__numPoints);
		glEnableClientState (GL_NORMAL_ARRAY);
		trisThisLoop += node->__numPoints;
	}
}

/***********************************************************************************/

void compile_ArcClose2D (struct X3D_ArcClose2D *node) {
	int xx;
	char *ct;
	void *tmpptr_a, *tmpptr_b;
	int tmpint;

        /*  have to regen the shape*/
	MARK_NODE_COMPILED
		
	ct = node->closureType->strptr;
	xx = node->closureType->len;
	tmpint = 0;
	tmpptr_a = NULL;

	if (strcmp(ct,"PIE") == 0) {
		tmpptr_a = createLines (node->startAngle,
			node->endAngle, node->radius, PIE, &tmpint);
	} else if (strcmp(ct,"CHORD") == 0) {
		tmpptr_a = createLines (node->startAngle,
			node->endAngle, node->radius, CHORD, &tmpint);
	} else {
		printf ("ArcClose2D, closureType %s invalid\n",node->closureType->strptr);
	}

	/* perform the switch - worry about threading here without locking */
	node->__numPoints = 0;		/* tell us that it has zero points */
	tmpptr_b = node->__points;	/* old set of points, for freeing later */
	node->__points = tmpptr_a;	/* new points */
	node->__numPoints = tmpint;
	FREE_IF_NZ (tmpptr_b);
	/* switch completed */
}


void render_ArcClose2D (struct X3D_ArcClose2D *node) {
	COMPILE_IF_REQUIRED
	if (node->__numPoints>0) {	
        	LIGHTING_OFF
        	DISABLE_CULL_FACE
		glColor3f (1.0, 1.0, 1.0);

		glDisableClientState (GL_NORMAL_ARRAY);
		glVertexPointer (2,GL_FLOAT,0,(GLfloat *)node->__points);
        	FW_GL_DRAWARRAYS (GL_LINE_STRIP, 0, node->__numPoints);
		glEnableClientState (GL_NORMAL_ARRAY);
		trisThisLoop += node->__numPoints;
	}
}

/***********************************************************************************/

void compile_Circle2D (struct X3D_Circle2D *node) {
	void *tmpptr_a, *tmpptr_b;
	int tmpint;

       /*  have to regen the shape*/
	MARK_NODE_COMPILED
		
	tmpptr_a = createLines (0.0, 0.0, node->radius, NONE, &tmpint);

	/* perform the switch - worry about threading here without locking */
	node->__numPoints = 0;		/* tell us that it has zero points */
	tmpptr_b = node->__points;	/* old set of points, for freeing later */
	node->__points = tmpptr_a;	/* new points */
	node->__numPoints = tmpint;
	FREE_IF_NZ (tmpptr_b);
	/* switch completed */
}

void render_Circle2D (struct X3D_Circle2D *node) {
	COMPILE_IF_REQUIRED
	if (node->__numPoints>0) {	
	        LIGHTING_OFF
	        DISABLE_CULL_FACE
		glColor3f (1.0, 1.0, 1.0);

		glDisableClientState (GL_NORMAL_ARRAY);
		glVertexPointer (2,GL_FLOAT,0,(GLfloat *)node->__points);
        	FW_GL_DRAWARRAYS (GL_LINE_STRIP, 0, node->__numPoints);
		glEnableClientState (GL_NORMAL_ARRAY);
		trisThisLoop += node->__numPoints;
	}
}

/***********************************************************************************/


void render_Polyline2D (struct X3D_Polyline2D *node){
	if (node->lineSegments.n>0) {
	        LIGHTING_OFF
	        DISABLE_CULL_FACE
		glColor3f (1.0, 1.0, 1.0);

		glDisableClientState (GL_NORMAL_ARRAY);
		glVertexPointer (2,GL_FLOAT,0,(GLfloat *)node->lineSegments.p);
        	FW_GL_DRAWARRAYS (GL_LINE_STRIP, 0, node->lineSegments.n);
		glEnableClientState (GL_NORMAL_ARRAY);
		trisThisLoop += node->lineSegments.n;
	}
}

/***********************************************************************************/


void render_Polypoint2D (struct X3D_Polypoint2D *node){
	if (node->point.n>0) {
	        LIGHTING_OFF
	        DISABLE_CULL_FACE
		glColor3f (1.0, 1.0, 1.0);

		glDisableClientState (GL_NORMAL_ARRAY);
		glVertexPointer (2,GL_FLOAT,0,(GLfloat *)node->point.p);
        	FW_GL_DRAWARRAYS (GL_POINTS, 0, node->point.n);
		glEnableClientState (GL_NORMAL_ARRAY);
		trisThisLoop += node->point.n;
	}
}

/***********************************************************************************/

void compile_Disk2D (struct X3D_Disk2D *node){
        /*  have to regen the shape*/
	GLfloat *fp;
	GLfloat *tp;
	GLfloat *sfp;
	GLfloat *stp;
	GLfloat *ofp;
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
		fp = sfp = MALLOC (sizeof(GLfloat) * 2 * (tmpint));
		tp = stp = MALLOC (sizeof(GLfloat) * 2 * (tmpint));

		/* initial TriangleFan point */
		*fp = 0.0; fp++; *fp = 0.0; fp++;
		*tp = 0.5; tp++; *tp = 0.5; tp++;
		id = 2.0;

		for (i=SEGMENTS_PER_CIRCLE; i >= 0; i--) {
			*fp = node->outerRadius * sinf((PI * 2.0 * (float)i)/((float)SEGMENTS_PER_CIRCLE));	fp++;
			*fp = node->outerRadius * cosf((PI * 2.0 * (float)i)/((float)SEGMENTS_PER_CIRCLE));	fp++;
			*tp = 0.5 + (sinf((PI * 2.0 * (float)i)/((float)SEGMENTS_PER_CIRCLE))/id);	tp++;
			*tp = 0.5 + (cosf((PI * 2.0 * (float)i)/((float)SEGMENTS_PER_CIRCLE))/id);	tp++;
		}
	} else {
		tmpint = (SEGMENTS_PER_CIRCLE+1) * 2;
		fp = sfp = MALLOC (sizeof(GLfloat) * 2 * tmpint);
		tp = stp = MALLOC (sizeof(GLfloat) * 2 * tmpint);


		/* texture scaling params */
		od = 2.0;
		id = node->outerRadius * 2.0 / node->innerRadius;

		for (i=SEGMENTS_PER_CIRCLE; i >= 0; i--) {
			*fp = node->innerRadius * sinf((PI * 2.0 * (float)i)/((float)SEGMENTS_PER_CIRCLE));	fp++;
			*fp = node->innerRadius * cosf((PI * 2.0 * (float)i)/((float)SEGMENTS_PER_CIRCLE));	fp++;
			*fp = node->outerRadius * sinf((PI * 2.0 * (float)i)/((float)SEGMENTS_PER_CIRCLE));	fp++;
			*fp = node->outerRadius * cosf((PI * 2.0 * (float)i)/((float)SEGMENTS_PER_CIRCLE));	fp++;
			*tp = 0.5 + (sinf((PI * 2.0 * (float)i)/((float)SEGMENTS_PER_CIRCLE))/id);	tp++;
			*tp = 0.5 + (cosf((PI * 2.0 * (float)i)/((float)SEGMENTS_PER_CIRCLE))/id);	tp++;
			*tp = 0.5 + (sinf((PI * 2.0 * (float)i)/((float)SEGMENTS_PER_CIRCLE))/od);	tp++;
			*tp = 0.5 + (cosf((PI * 2.0 * (float)i)/((float)SEGMENTS_PER_CIRCLE))/od);	tp++;
		}
	}


	/* compiling done, set up for rendering. thread safe */
	node->__numPoints = 0;
	ofp = node->__points;
	otp = node->__texCoords;
	node->__points = sfp;
	node->__texCoords = stp;
	node->__simpleDisk = simpleDisc;
	node->__numPoints = tmpint;
	FREE_IF_NZ (ofp);
	FREE_IF_NZ (otp);
}

void render_Disk2D (struct X3D_Disk2D *node){
	COMPILE_IF_REQUIRED
	if (node->__numPoints>0) {	
		CULL_FACE(node->solid)

		textureDraw_start(NULL,(GLfloat *)node->__texCoords);
		glVertexPointer (2,GL_FLOAT,0,(GLfloat *)node->__points);
		glDisableClientState (GL_NORMAL_ARRAY);
		glNormal3f (0.0, 0.0, 1.0);

		/* do the array drawing; sides are simple 0-1-2-3, 4-5-6-7, etc quads */
		if (node->__simpleDisk) {FW_GL_DRAWARRAYS (GL_TRIANGLE_FAN, 0, node->__numPoints);}
		else 			{FW_GL_DRAWARRAYS (GL_QUAD_STRIP, 0, node->__numPoints); }

		textureDraw_end();
		glEnableClientState (GL_NORMAL_ARRAY);
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
	node->__texCoords = fp = MALLOC (sizeof (GLfloat) * tmpint * 2);

	/* find min/max values for X and Y axes */
	minY = minX = 99999999.0;
	maxY = maxX = -9999999.0;
	for (i=0; i<tmpint; i++) {
		if (node->vertices.p[i].c[0] < minX) minX = node->vertices.p[i].c[0];
		if (node->vertices.p[i].c[1] < minY) minY = node->vertices.p[i].c[1];
		if (node->vertices.p[i].c[0] > maxX) maxX = node->vertices.p[i].c[0];
		if (node->vertices.p[i].c[1] > maxY) maxY = node->vertices.p[i].c[1];
	}
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
		CULL_FACE(node->solid)

		textureDraw_start(NULL,(GLfloat *)node->__texCoords);
		glVertexPointer (2,GL_FLOAT,0,(GLfloat *)node->vertices.p);
		glDisableClientState (GL_NORMAL_ARRAY);
		glNormal3f (0.0, 0.0, 1.0);

		FW_GL_DRAWARRAYS (GL_TRIANGLES, 0, node->vertices.n);

		textureDraw_end();
		glEnableClientState (GL_NORMAL_ARRAY);
		trisThisLoop += node->vertices.n;
	}
}


/***********************************************************************************/

/* this code is remarkably like Box, but with a zero z axis. */
void compile_Rectangle2D (struct X3D_Rectangle2D *node) {
	float *pt;
	void *xx;
	float x = ((node->size).c[0])/2;
	float y = ((node->size).c[1])/2;
	MARK_NODE_COMPILED

	xx = NULL;

	/* once MALLOC'd, this never changes size, so is "threadsafe" */
	/*  MALLOC memory (if possible)*/
	if (!node->__points) xx = MALLOC (sizeof(struct SFColor)*(4));

	/*  now, create points; 4 points per face.*/
	pt = (float *) xx;
	/*  front*/
	*pt++ =  x; *pt++ =  y; *pt++ =  0.0; *pt++ = -x; *pt++ =  y; *pt++ =  0.0;
	*pt++ = -x; *pt++ = -y; *pt++ =  0.0; *pt++ =  x; *pt++ = -y; *pt++ =  0.0;
	if (!node->__points) node->__points =xx;
}

void render_Rectangle2D (struct X3D_Rectangle2D *node) {
	extern GLfloat boxtex[];		/*  in CFuncs/statics.c*/
	float x = ((node->size).c[0])/2;
	float y = ((node->size).c[1])/2;

	/* test for <0 of sides */
	if ((x < 0) || (y < 0)) return;
	COMPILE_IF_REQUIRED
	
	/* is it compiled yet? (threading) */
	if (!node->__points) return;

	/* for BoundingBox calculations */
	setExtent(x,-x,y,-y,0.0,0.0,X3D_NODE(node));

	CULL_FACE(node->solid)

	/*  Draw it; assume VERTEX and NORMALS already defined.*/
	textureDraw_start(NULL,boxtex);
	glVertexPointer (3,GL_FLOAT,0,(GLfloat *)node->__points);
	glDisableClientState (GL_NORMAL_ARRAY);
	glNormal3f (0.0, 0.0, 1.0);

	/* do the array drawing; sides are simple 0-1-2-3, 4-5-6-7, etc quads */
	FW_GL_DRAWARRAYS (GL_QUADS, 0, 4);
	textureDraw_end();
	glEnableClientState (GL_NORMAL_ARRAY);
	trisThisLoop += 2;
}



/***********************************************************************************/

void *createLines (float start, float end, float radius, int closed, int *size) {
	int i;
	int isCircle;
	int numPoints;
	GLfloat tmp;
	GLfloat *points;
	GLfloat *fp;
	int arcpoints;

	*size = 0;

	/* is this a circle? */
	isCircle =  APPROX(start,end);

	/* bounds check, and sort values */
	if ((start < PI*2.0) || (start > PI*2.0)) start = 0;
	if ((end < PI*2.0) || (end > PI*2.0)) end = PI/2;
	if (radius<0.0) radius = 1.0;

	if (end > start) {
		tmp = start;
		start = end;
		end = tmp;
	}
		

	if (isCircle) {
		numPoints = SEGMENTS_PER_CIRCLE;
		closed = NONE; /* this is a circle, CHORD, PIE dont mean anything now */
	} else {
		numPoints = ((float) SEGMENTS_PER_CIRCLE * (start-end)/(PI*2.0));
		if (numPoints>SEGMENTS_PER_CIRCLE) numPoints=SEGMENTS_PER_CIRCLE;
	}

	/* we always have to draw the line - we have a line strip, and we calculate
	   the beginning points; we have also to calculate the ending point. */
	numPoints++;
	arcpoints = numPoints;

	/* closure type */
	if (closed == CHORD) numPoints++;
	if (closed == PIE) numPoints+=2;

	points = MALLOC (sizeof(float)*numPoints*2);
	fp = points;

	for (i=0; i<arcpoints; i++) {
		*fp = -radius * sinf((PI * 2.0 * (float)i)/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
		*fp = radius * cosf((PI * 2.0 * (float)i)/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
	}

	/* do we have to draw any pies, cords, etc, etc? */
	if (closed == CHORD) {
		/* loop back to origin */
		*fp = -radius * sinf(0.0/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
		*fp = radius * cosf(0.0/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
	} else if (closed == PIE) {
		/* go to origin */
		*fp = 0.0; fp++; *fp=0.0; fp++; 
		*fp = -radius * sinf(0.0/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
		*fp = radius * cosf(0.0/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
	}

		
	*size = numPoints;
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
	       GLdouble awidth = naviinfo.width; /*avatar width*/
	       GLdouble atop = naviinfo.width; /*top of avatar (relative to eyepoint)*/
	       GLdouble abottom = -naviinfo.height; /*bottom of avatar (relative to eyepoint)*/
	       GLdouble astep = -naviinfo.height+naviinfo.step;

	       GLdouble modelMatrix[16];
	       GLdouble upvecmat[16];
	       struct point_XYZ iv = {0,0,0};
	       struct point_XYZ jv = {0,0,0};
	       struct point_XYZ kv = {0,0,0};
	       struct point_XYZ ov = {0,0,0};

	       struct point_XYZ t_orig = {0,0,0};
	       GLdouble scale; /* FIXME: won''t work for non-uniform scales. */

	       struct point_XYZ delta;
	       struct point_XYZ tupv = {0,1,0};

		iv.x = node->size.c[0];
		jv.y = node->size.c[1]; 
		kv.z = 0.0;
		ov.x = -((node->size).c[0])/2; ov.y = -((node->size).c[1])/2; ov.z = 0.0;

	       /* get the transformed position of the Box, and the scale-corrected radius. */
	       fwGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

	       transform3x3(&tupv,&tupv,modelMatrix);
	       matrotate2v(upvecmat,ViewerUpvector,tupv);
	       matmultiply(modelMatrix,upvecmat,modelMatrix);
	       matinverse(upvecmat,upvecmat);

	       /* values for rapid test */
	       t_orig.x = modelMatrix[12];
	       t_orig.y = modelMatrix[13];
	       t_orig.z = modelMatrix[14];
	       scale = pow(det3x3(modelMatrix),1./3.);
	       if(!fast_ycylinder_box_intersect(abottom,atop,awidth,t_orig,scale*node->size.c[0],
			scale*node->size.c[1],0.0)) return;

	       /* get transformed box edges and position */
	       transform(&ov,&ov,modelMatrix);
	       transform3x3(&iv,&iv,modelMatrix);
	       transform3x3(&jv,&jv,modelMatrix);
	       transform3x3(&kv,&kv,modelMatrix);


	       delta = box_disp(abottom,atop,astep,awidth,ov,iv,jv,kv);

	       vecscale(&delta,&delta,-1);
	       transform3x3(&delta,&delta,upvecmat);

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
