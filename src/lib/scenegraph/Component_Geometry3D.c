/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Geometry3D.c,v 1.3 2009/02/11 15:12:54 istakenv Exp $

X3D Geometry 3D Component

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h" /* point_XYZ */
#include "../main/headers.h"

#include "Collision.h"
#include "Polyrep.h"
#include "LinearAlgebra.h"


/*******************************************************************************/

/*  have to regen the shape*/
void compile_Box (struct X3D_Box *node) {
	float *pt;
	void *ptr;
	float x = ((node->size).c[0])/2;
	float y = ((node->size).c[1])/2;
	float z = ((node->size).c[2])/2;

	MARK_NODE_COMPILED

	/*  MALLOC memory (if possible)*/
	if (!node->__points) ptr = MALLOC (sizeof(struct SFColor)*(24));
	else ptr = node->__points;

	/*  now, create points; 4 points per face.*/
	pt = (float *) ptr;
	/*  front*/
	*pt++ =  x; *pt++ =  y; *pt++ =  z; *pt++ = -x; *pt++ =  y; *pt++ =  z;
	*pt++ = -x; *pt++ = -y; *pt++ =  z; *pt++ =  x; *pt++ = -y; *pt++ =  z;
	/*  back*/
	*pt++ =  x; *pt++ = -y; *pt++ = -z; *pt++ = -x; *pt++ = -y; *pt++ = -z;
	*pt++ = -x; *pt++ =  y; *pt++ = -z; *pt++ =  x; *pt++ =  y; *pt++ = -z;
	/*  top*/
	*pt++ = -x; *pt++ =  y; *pt++ =  z; *pt++ =  x; *pt++ =  y; *pt++ =  z;
	*pt++ =  x; *pt++ =  y; *pt++ = -z; *pt++ = -x; *pt++ =  y; *pt++ = -z;
	/*  down*/
	*pt++ = -x; *pt++ = -y; *pt++ = -z; *pt++ =  x; *pt++ = -y; *pt++ = -z;
	*pt++ =  x; *pt++ = -y; *pt++ =  z; *pt++ = -x; *pt++ = -y; *pt++ =  z;
	/*  right*/
	*pt++ =  x; *pt++ = -y; *pt++ =  z; *pt++ =  x; *pt++ = -y; *pt++ = -z;
	*pt++ =  x; *pt++ =  y; *pt++ = -z; *pt++ =  x; *pt++ =  y; *pt++ =  z;
	/*  left*/
	*pt++ = -x; *pt++ = -y; *pt++ =  z; *pt++ = -x; *pt++ =  y; *pt++ =  z;
	*pt++ = -x; *pt++ =  y; *pt++ = -z; *pt++ = -x; *pt++ = -y; *pt++ = -z;

	/* finished, and have good data */
	node->__points = ptr;
}

void render_Box (struct X3D_Box *node) {
	extern GLfloat boxtex[];		/*  in CFuncs/statics.c*/
	extern GLfloat boxnorms[];		/*  in CFuncs/statics.c*/
	float x = ((node->size).c[0])/2;
	float y = ((node->size).c[1])/2;
	float z = ((node->size).c[2])/2;

	/* test for <0 of sides */
	if ((x < 0) || (y < 0) || (z < 0)) return;

	COMPILE_IF_REQUIRED
	if (!node->__points) return; /* still compiling */

	/* for BoundingBox calculations */
	setExtent(x,-x,y,-y,z,-z,X3D_NODE(node));

	CULL_FACE(node->solid)

	/*  Draw it; assume VERTEX and NORMALS already defined.*/
	textureDraw_start(NULL,boxtex);
	glVertexPointer (3,GL_FLOAT,0,(GLfloat *)node->__points);
	glNormalPointer (GL_FLOAT,0,boxnorms);

	/* do the array drawing; sides are simple 0-1-2-3, 4-5-6-7, etc quads */
	glDrawArrays (GL_QUADS, 0, 24);
	textureDraw_end();
	trisThisLoop += 24;
}


/*******************************************************************************/

void compile_Cylinder (struct X3D_Cylinder * node) {
	#define CYLDIV 20
	float h = (node->height)/2;
	float r = node->radius;
	int i = 0;
	struct SFColor *pt;
	float a1, a2;
	void *tmpptr;

	/*  have to regen the shape*/
	MARK_NODE_COMPILED


	/* use node->__points as compile completed flag for threading */

	/*  MALLOC memory (if possible)*/
	if (!node->__points) tmpptr = MALLOC(sizeof(struct SFColor)*2*(CYLDIV+4));
	else tmpptr = node->__points;

	if (!node->__normals) node->__normals = MALLOC(sizeof(struct SFColor)*2*(CYLDIV+1));

	/*  now, create the vertices; this is a quad, so each face = 4 points*/
	pt = (struct SFColor *) tmpptr;
	for (i=0; i<CYLDIV; i++) {
		a1 = PI*2*i/(float)CYLDIV;
		a2 = PI*2*(i+1)/(float)CYLDIV;
		pt[i*2+0].c[0] = r*sin(a1);
		pt[i*2+0].c[1] = (float) h;
		pt[i*2+0].c[2] = r*cos(a1);
		pt[i*2+1].c[0] = r*sin(a1);
		pt[i*2+1].c[1] = (float) -h;
		pt[i*2+1].c[2] = r*cos(a1);
	}

	/*  wrap the points around*/
	memcpy (&pt[CYLDIV*2].c[0],&pt[0].c[0],sizeof(struct SFColor)*2);

	/*  center points of top and bottom*/
	pt[CYLDIV*2+2].c[0] = 0.0; pt[CYLDIV*2+2].c[1] = (float) h; pt[CYLDIV*2+2].c[2] = 0.0;
	pt[CYLDIV*2+3].c[0] = 0.0; pt[CYLDIV*2+3].c[1] = (float)-h; pt[CYLDIV*2+3].c[2] = 0.0;
	node->__points = tmpptr;
}

void render_Cylinder (struct X3D_Cylinder * node) {
	extern GLfloat cylnorms[];		/*  in CFuncs/statics.c*/
	extern unsigned char cyltopindx[];	/*  in CFuncs/statics.c*/
	extern unsigned char cylbotindx[];	/*  in CFuncs/statics.c*/
	extern GLfloat cylendtex[];		/*  in CFuncs/statics.c*/
	extern GLfloat cylsidetex[];		/*  in CFuncs/statics.c*/
	float h = (node->height)/2;
	float r = node->radius;

	if ((h < 0) || (r < 0)) {return;}

	/* for BoundingBox calculations */
	setExtent(r,-r,h,-h,r,-r,X3D_NODE(node));

	COMPILE_IF_REQUIRED
	if (!node->__points) return;

	CULL_FACE(node->solid)

	/*  Display the shape*/
	glVertexPointer (3,GL_FLOAT,0,(GLfloat *)node->__points);

	if (node->side) {
		glNormalPointer (GL_FLOAT,0,cylnorms);
		textureDraw_start(NULL,cylsidetex);

		/* do the array drawing; sides are simple 0-1-2,3-4-5,etc triangles */
		glDrawArrays (GL_QUAD_STRIP, 0, (CYLDIV+1)*2);
		trisThisLoop += (CYLDIV+1)*2*2; /* 2 triangles per quad strip */
	}
	if(node->bottom) {
		textureDraw_start(NULL,cylendtex);
		glDisableClientState (GL_NORMAL_ARRAY);
		glNormal3f(0.0,-1.0,0.0);
		glDrawElements (GL_TRIANGLE_FAN, CYLDIV+2 ,GL_UNSIGNED_BYTE,cylbotindx);
		glEnableClientState(GL_NORMAL_ARRAY);
		trisThisLoop += CYLDIV+2;
	}

	if (node->top) {
		textureDraw_start(NULL,cylendtex);
		glDisableClientState (GL_NORMAL_ARRAY);
		glNormal3f(0.0,1.0,0.0);
		glDrawElements (GL_TRIANGLE_FAN, CYLDIV+2 ,GL_UNSIGNED_BYTE,cyltopindx);
		glEnableClientState(GL_NORMAL_ARRAY);
		trisThisLoop += CYLDIV+2;
	}
	textureDraw_end();

}



/*******************************************************************************/

void compile_Cone (struct X3D_Cone *node) {
	/*  DO NOT change this define, unless you want to recalculate statics below....*/
	#define  CONEDIV 20

	float h = (node->height)/2;
	float r = node->bottomRadius;
	float angle;
	int i;
	struct SFColor *pt;			/*  bottom points*/
	struct SFColor *spt;			/*  side points*/
	struct SFColor *norm;			/*  side normals*/
	void *ptr;

	/*  have to regen the shape*/
	MARK_NODE_COMPILED

	/*  MALLOC memory (if possible)*/
	if (!node->__botpoints) node->__botpoints = MALLOC (sizeof(struct SFColor)*(CONEDIV+3));
	if (!node->__sidepoints) node->__sidepoints = MALLOC (sizeof(struct SFColor)*3*(CONEDIV+1));

	/* use normals for compiled flag for threading */

	if (!node->__normals) ptr = MALLOC (sizeof(struct SFColor)*3*(CONEDIV+1));
	else ptr = node->__normals;
	
	/*  generate the vertexes for the triangles; top point first. (note: top point no longer used)*/
	pt = (struct SFColor *)node->__botpoints;
	pt[0].c[0] = 0.0; pt[0].c[1] = (float) h; pt[0].c[2] = 0.0;
	for (i=1; i<=CONEDIV; i++) {
		pt[i].c[0] = r*sin(PI*2*i/(float)CONEDIV);
		pt[i].c[1] = (float) -h;
		pt[i].c[2] = r*cos(PI*2*i/(float)CONEDIV);
	}
	/*  and throw another point that is centre of bottom*/
	pt[CONEDIV+1].c[0] = 0.0; pt[CONEDIV+1].c[1] = (float) -h; pt[CONEDIV+1].c[2] = 0.0;

	/*  and, for the bottom, [CONEDIV] = [CONEDIV+2]; but different texture coords, so...*/
	memcpy (&pt[CONEDIV+2].c[0],&pt[CONEDIV].c[0],sizeof (struct SFColor));

	/*  side triangles. Make 3 seperate points per triangle... makes glDrawArrays with normals*/
	/*  easier to handle.*/
	/*  rearrange bottom points into this array; top, bottom, left.*/
	spt = (struct SFColor *)node->__sidepoints;
	for (i=0; i<CONEDIV; i++) {
		/*  top point*/
		spt[i*3].c[0] = 0.0; spt[i*3].c[1] = (float) h; spt[i*3].c[2] = 0.0;
		/*  left point*/
		memcpy (&spt[i*3+1].c[0],&pt[i+1].c[0],sizeof (struct SFColor));
		/* right point*/
		memcpy (&spt[i*3+2].c[0],&pt[i+2].c[0],sizeof (struct SFColor));
	}

	/*  wrap bottom point around once again... ie, final right point = initial left point*/
	memcpy (&spt[(CONEDIV-1)*3+2].c[0],&pt[1].c[0],sizeof (struct SFColor));

	/*  Side Normals - note, normals for faces doubled - see MALLOC above*/
	/*  this gives us normals half way between faces. 1 = face 1, 3 = face2, 5 = face 3...*/
	norm = (struct SFColor *)ptr;
	for (i=0; i<=CONEDIV; i++) {
		/*  top point*/
		angle = PI * 2 * (i+0.5) / (float) (CONEDIV);
		norm[i*3+0].c[0] = sin(angle); norm[i*3+0].c[1] = (float)h/r; norm[i*3+0].c[2] = cos(angle);
		/* left point*/
		angle = PI * 2 * (i+0) / (float) (CONEDIV);
		norm[i*3+1].c[0] = sin(angle); norm[i*3+1].c[1] = (float)h/r; norm[i*3+1].c[2] = cos(angle);
		/*  right point*/
		angle = PI * 2 * (i+1) / (float) (CONEDIV);
		norm[i*3+2].c[0] = sin(angle); norm[i*3+2].c[1] = (float)h/r; norm[i*3+2].c[2] = cos(angle);
	}

	/* ok, finished compiling, finish */
	node->__normals = ptr;
}

void render_Cone (struct X3D_Cone *node) {
	extern unsigned char tribotindx[];	/*  in CFuncs/statics.c*/
	extern float tribottex[];		/*  in CFuncs/statics.c*/
	extern float trisidtex[];		/*  in CFuncs/statics.c*/
	/*  DO NOT change this define, unless you want to recalculate statics below....*/
	#define  CONEDIV 20

	float h = (node->height)/2;
	float r = node->bottomRadius;

	if ((h < 0) || (r < 0)) {return;}
	COMPILE_IF_REQUIRED

	/* use normals for threaded compile completed flag */
	if (!node->__normals) return;

	/* for BoundingBox calculations */
	setExtent(r,-r,h,-h,r,-r,X3D_NODE(node));


	CULL_FACE(node->solid)

	/*  OK - we have vertex data, so lets just render it.*/
	/*  Always assume GL_VERTEX_ARRAY and GL_NORMAL_ARRAY are enabled.*/

	if(node->bottom) {
		glDisableClientState (GL_NORMAL_ARRAY);
		glVertexPointer (3,GL_FLOAT,0,(GLfloat *)node->__botpoints);
		textureDraw_start(NULL,tribottex);
		glNormal3f(0.0,-1.0,0.0);
		glDrawElements (GL_TRIANGLE_FAN, CONEDIV+2, GL_UNSIGNED_BYTE,tribotindx);
		glEnableClientState(GL_NORMAL_ARRAY);
		trisThisLoop += CONEDIV+2;
	}

	if(node->side) {
		glVertexPointer (3,GL_FLOAT,0,(GLfloat *)node->__sidepoints);
		glNormalPointer (GL_FLOAT,0,(GLfloat *)node->__normals);
		textureDraw_start(NULL,trisidtex);

		/* do the array drawing; sides are simple 0-1-2,3-4-5,etc triangles */
		glDrawArrays (GL_TRIANGLES, 0, 60);
		trisThisLoop += 60;
	}
	textureDraw_end();
}



/*******************************************************************************/


void compile_Sphere (struct X3D_Sphere *node) {
	#define INIT_TRIG1(div) t_aa = sin(PI/(div)); t_aa *= 2*t_aa; t_ab = -sin(2*PI/(div));
	#define START_TRIG1 t_sa = 0; t_ca = -1;
	#define UP_TRIG1 t_sa1 = t_sa; t_sa -= t_sa*t_aa - t_ca * t_ab; t_ca -= t_ca * t_aa + t_sa1 * t_ab;
	#define SIN1 t_sa
	#define COS1 t_ca
	#define INIT_TRIG2(div) t2_aa = sin(PI/(div)); t2_aa *= 2*t2_aa; t2_ab = -sin(2*PI/(div));
	#define START_TRIG2 t2_sa = -1; t2_ca = 0;
	#define UP_TRIG2 t2_sa1 = t2_sa; t2_sa -= t2_sa*t2_aa - t2_ca * t2_ab; t2_ca -= t2_ca * t2_aa + t2_sa1 * t2_ab;
	#define SIN2 t2_sa
	#define COS2 t2_ca

	/*  make the divisions 20; dont change this, because statics.c values*/
	/*  will then need recaculating.*/
	#define SPHDIV 20

	int count;
	float rad = node->radius;
	void *ptr;

	int v; int h;
	float t_aa, t_ab, t_sa, t_ca, t_sa1;
	float t2_aa, t2_ab, t2_sa, t2_ca, t2_sa1;
	struct SFColor *pts;

	/*  have to regen the shape*/
	MARK_NODE_COMPILED

	/*  MALLOC memory (if possible)*/
	/*  2 vertexes per points. (+1, to loop around and close structure)*/
	if (!node->__points) ptr = MALLOC (sizeof(struct SFColor) * SPHDIV * (SPHDIV+1) * 2);
	else ptr = node->__points;

	pts = (struct SFColor *) ptr;
	count = 0;

	INIT_TRIG1(SPHDIV)
	INIT_TRIG2(SPHDIV)

	START_TRIG1
	for(v=0; v<SPHDIV; v++) {
		float vsin1 = SIN1;
		float vcos1 = COS1, vsin2,vcos2;
		UP_TRIG1
		vsin2 = SIN1;
		vcos2 = COS1;
		START_TRIG2
		for(h=0; h<=SPHDIV; h++) {
			float hsin1 = SIN2;
			float hcos1 = COS2;
			UP_TRIG2
			pts[count].c[0] = rad * vsin2 * hcos1;
			pts[count].c[1] = rad * vcos2;
			pts[count].c[2] = rad * vsin2 * hsin1;
			count++;
			pts[count].c[0] = rad * vsin1 * hcos1;
			pts[count].c[1] = rad * vcos1;
			pts[count].c[2] = rad * vsin1 * hsin1;
			count++;
		}
	}

	/* finished - for threading */
	node->__points = ptr;
}


void render_Sphere (struct X3D_Sphere *node) {
	/*  make the divisions 20; dont change this, because statics.c values*/
	/*  will then need recaculating.*/
	#define SPHDIV 20
	extern GLfloat spherenorms[];		/*  side normals*/
	extern float spheretex[];		/*  in CFuncs/statics.c*/

	float rad = node->radius;
	int count;

	if (rad<=0.0) { return;}

	/* for BoundingBox calculations */
	setExtent(rad,-rad,rad,-rad,rad,-rad,X3D_NODE(node));

	COMPILE_IF_REQUIRED

	CULL_FACE(node->solid)

	/*  Display the shape*/
	textureDraw_start(NULL,spheretex);
	glVertexPointer (3,GL_FLOAT,0,(GLfloat *)node->__points);
	glNormalPointer (GL_FLOAT,0,spherenorms);

	/* do the array drawing; sides are simple 0-1-2,3-4-5,etc triangles */
	/* for (count = 0; count < SPHDIV; count ++) { */
	for (count = 0; count < SPHDIV/2; count ++) { 
		glDrawArrays (GL_QUAD_STRIP, count*(SPHDIV+1)*2, (SPHDIV+1)*2);
		trisThisLoop += (SPHDIV+1) * 4;
	}
	textureDraw_end();
}

void render_IndexedFaceSet (struct X3D_IndexedFaceSet *node) {
		COMPILE_POLY_IF_REQUIRED (node->coord, node->color, node->normal, node->texCoord)
		CULL_FACE(node->solid)
		render_polyrep(node);
}

void render_ElevationGrid (struct X3D_ElevationGrid *node) {
		COMPILE_POLY_IF_REQUIRED (NULL, node->color, node->normal, node->texCoord)
		CULL_FACE(node->solid)
		render_polyrep(node);
}

void render_Extrusion (struct X3D_Extrusion *node) {
		COMPILE_POLY_IF_REQUIRED (NULL,NULL,NULL,NULL)
		CULL_FACE(node->solid)
		render_polyrep(node);
}


void collide_IndexedFaceSet (struct X3D_IndexedFaceSet *node ){
	       GLdouble awidth = naviinfo.width; /*avatar width*/
	       GLdouble atop = naviinfo.width; /*top of avatar (relative to eyepoint)*/
	       GLdouble abottom = -naviinfo.height; /*bottom of avatar (relative to eyepoint)*/
	       GLdouble astep = -naviinfo.height+naviinfo.step;
	       GLdouble modelMatrix[16];
	       GLdouble upvecmat[16];

	       GLdouble scale; /* FIXME: won''t work for non-uniform scales. */
	       struct point_XYZ t_orig = {0,0,0};

	       struct point_XYZ tupv = {0,1,0};
	       struct point_XYZ delta = {0,0,0};

	       struct X3D_PolyRep pr;
	       prflags flags = 0;
	       int change = 0;

	        struct X3D_Coordinate *xc;

		/* JAS - first pass, intern is probably zero */
		if (((struct X3D_PolyRep *)node->_intern) == 0) return;

		/* JAS - no triangles in this text structure */
		if ((((struct X3D_PolyRep *)node->_intern)->ntri) == 0) return;


	       /*save changed state.*/
	       if(node->_intern) change = ((struct X3D_PolyRep *)node->_intern)->irep_change;
		COMPILE_POLY_IF_REQUIRED (NULL, NULL, NULL, NULL)


	       if(node->_intern) ((struct X3D_PolyRep *)node->_intern)->irep_change = change;
	       /*restore changes state, invalidates mk_polyrep work done, so it can be done
	         correclty in the RENDER pass */

	       if(!node->solid) {
		   flags = flags | PR_DOUBLESIDED;
	       }

	       pr = *((struct X3D_PolyRep*)node->_intern);

		/* IndexedFaceSets are "different", in that the user specifies points, among
		   other things.  The rendering pass takes these external points, and streams
		   them to make rendering much faster on hardware accel. We have to check to
		   see whether we have got here before the first rendering of a possibly new
		   IndexedFaceSet */
		if (!pr.actualCoord) {
			struct Multi_Vec3f* tmp;
			tmp = getCoordinate(node->coord,"Collision");
			pr.actualCoord = (float *) tmp->p;
		}

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

		/* lets try and see if we are close - this gives false positives, but it does
		   weed out fairly distant objects */
		if(!fast_ycylinder_polyrep_intersect(abottom,atop,awidth,t_orig,scale, &pr)) return;

		/* printf ("trying larger radius \n"); */
	       /* delta = polyrep_disp(abottom,atop,astep,awidth*2,pr,modelMatrix,flags); */
	       delta = polyrep_disp(abottom,atop,astep,awidth,pr,modelMatrix,flags);




	/*	printf ("collide_IFS, node %u, delta %f %f %f\n",node,delta.x, delta.y, delta.z); */

	       vecscale(&delta,&delta,-1);
	       transform3x3(&delta,&delta,upvecmat);
		/* printf ("collide_IFS after transform, node %u, delta %f %f %f\n",node,delta.x, delta.y, delta.z); */

	       accumulate_disp(&CollisionInfo,delta);
		/* printf ("collide_IFS, colinfo %lf %lf %lf, count %d Maximum2 %lf\n",CollisionInfo.Offset.x,
			CollisionInfo.Offset.y, CollisionInfo.Offset.z,CollisionInfo.Count, CollisionInfo.Maximum2);  */

		#ifdef COLLISIONVERBOSE
	       if((fabs(delta.x) != 0. || fabs(delta.y) != 0. || fabs(delta.z) != 0.))  {
/*		   printmatrix(modelMatrix);*/
		   fprintf(stderr,"COLLISION_IFS: (%f %f %f) (%f %f %f)\n",
			  t_orig.x, t_orig.y, t_orig.z,
			  delta.x, delta.y, delta.z
			  );

	       }
		#endif
}


void collide_Sphere (struct X3D_Sphere *node) {
	       struct point_XYZ t_orig; /*transformed origin*/
	       struct point_XYZ p_orig; /*projected transformed origin */
	       struct point_XYZ n_orig; /*normal(unit length) transformed origin */
	       GLdouble modelMatrix[16];
	       GLdouble upvecmat[16];
	       GLdouble dist2;
	       struct point_XYZ delta = {0,0,0};
	       GLdouble radius;

	       /*easy access, naviinfo.step unused for sphere collisions */
	       GLdouble awidth = naviinfo.width; /*avatar width*/
	       GLdouble atop = naviinfo.width; /*top of avatar (relative to eyepoint)*/
	       GLdouble abottom = -naviinfo.height; /*bottom of avatar (relative to eyepoint)*/

	       struct point_XYZ tupv = {0,1,0};

		/* are we initialized yet? */
		if (node->__points==0) {
			return;
		}

	       /* get the transformed position of the Sphere, and the scale-corrected radius. */
	       fwGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

	       transform3x3(&tupv,&tupv,modelMatrix);
	       matrotate2v(upvecmat,ViewerUpvector,tupv);
	       matmultiply(modelMatrix,upvecmat,modelMatrix);
	       matinverse(upvecmat,upvecmat);

	       t_orig.x = modelMatrix[12];
	       t_orig.y = modelMatrix[13];
	       t_orig.z = modelMatrix[14];
	       radius = pow(det3x3(modelMatrix),1./3.) * node->radius;

	       /* squared distance to center of sphere (on the y plane)*/
	       dist2 = t_orig.x * t_orig.x + t_orig.z * t_orig.z;

	       /* easy tests. clip as if sphere was a box */
	       /*clip with cylinder */
	       if(dist2 - (radius + awidth) * (radius +awidth) > 0) {
		   return;
	       }
	       /*clip with bottom plane */
	       if(t_orig.y + radius < abottom) {
		   return;
	       }
	       /*clip with top plane */
	       if(t_orig.y-radius > atop) {
		   return;
	       }

	       /* project onto (y x t_orig) plane */
	       p_orig.x = sqrt(dist2);
	       p_orig.y = t_orig.y;
	       p_orig.z = 0;
	       /* we need this to unproject rapidly */
	       /* n_orig is t_orig.y projected on the y plane, then normalized. */
	       n_orig.x = t_orig.x;
	       n_orig.y = 0.0;
	       n_orig.z = t_orig.z;
	       VECSCALE(n_orig,1.0/p_orig.x); /*equivalent to vecnormal(n_orig);, but faster */

	       /* 5 cases : sphere is over, over side, side, under and side, under (relative to y axis) */
	       /* these 5 cases correspond to the 5 vornoi regions of the cylinder */
	       if(p_orig.y > atop) {

		   if(p_orig.x < awidth) {
			#ifdef RENDERVERBOSE
		       printf(" /* over, we push down. */ \n");
			#endif

		       delta.y = (p_orig.y - radius) - (atop);
		   } else {
		       struct point_XYZ d2s;
		       GLdouble ratio;
		       #ifdef RENDERVERBOSE
			printf(" /* over side */ \n");
			#endif

		       /* distance vector from corner to center of sphere*/
		       d2s.x = p_orig.x - awidth;
		       d2s.y = p_orig.y - (atop);
		       d2s.z = 0;

		       ratio = 1- radius/sqrt(d2s.x * d2s.x + d2s.y * d2s.y);

		       if(ratio >= 0) {
			   /* no collision */
			   return;
		       }

		       /* distance vector from corner to surface of sphere, (do the math) */
		       VECSCALE(d2s, ratio );

		       /* unproject, this is the fastest way */
		       delta.y = d2s.y;
		       delta.x = d2s.x* n_orig.x;
		       delta.z = d2s.x* n_orig.z;
		   }
	       } else if(p_orig.y < abottom) {
		   if(p_orig.x < awidth) {
			#ifdef RENDERVERBOSE
		       printf(" /* under, we push up. */ \n");
			#endif

		       delta.y = (p_orig.y + radius) -abottom;
		   } else {
		       struct point_XYZ d2s;
		       GLdouble ratio;
			#ifdef RENDERVERBOSE
		       printf(" /* under side */ \n");
			#endif

		       /* distance vector from corner to center of sphere*/
		       d2s.x = p_orig.x - awidth;
		       d2s.y = p_orig.y - abottom;
		       d2s.z = 0;

		       ratio = 1- radius/sqrt(d2s.x * d2s.x + d2s.y * d2s.y);

		       if(ratio >= 0) {
			   /* no collision */
			   return;
		       }

		       /* distance vector from corner to surface of sphere, (do the math) */
		       VECSCALE(d2s, ratio );

		       /* unproject, this is the fastest way */
		       delta.y = d2s.y;
		       delta.x = d2s.x* n_orig.x;
		       delta.z = d2s.x* n_orig.z;
		   }

	       } else {
		   #ifdef RENDERVERBOSE
			printf(" /* side */ \n");
		   #endif

		   /* push to side */
		   delta.x = ((p_orig.x - radius)- awidth) * n_orig.x;
		   delta.z = ((p_orig.x - radius)- awidth) * n_orig.z;
	       }


	       transform3x3(&delta,&delta,upvecmat);
	       accumulate_disp(&CollisionInfo,delta);

		#ifdef COLLISIONVERBOSE
	       if((delta.x != 0. || delta.y != 0. || delta.z != 0.))
	           printf("COLLISION_SPH: (%f %f %f) (%f %f %f) (px=%f nx=%f nz=%f)\n",
			  t_orig.x, t_orig.y, t_orig.z,
			  delta.x, delta.y, delta.z,
			  p_orig.x, n_orig.x, n_orig.z
			  );
		#endif
}

void collide_Box (struct X3D_Box *node) {
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


                iv.x = ((node->size).c[0]);
                jv.y = ((node->size).c[1]);
                kv.z = ((node->size).c[2]);
                ov.x = -((node->size).c[0])/2; ov.y = -((node->size).c[1])/2; ov.z = -((node->size).c[2])/2;


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
               if(!fast_ycylinder_box_intersect(abottom,atop,awidth,t_orig,scale*((node->size).c[0]),scale*((node->size).c[1]),scale*((node->size).c[2]))) return;

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


void collide_Cone (struct X3D_Cone *node) {

	       /*easy access, naviinfo.step unused for sphere collisions */
	       GLdouble awidth = naviinfo.width; /*avatar width*/
	       GLdouble atop = naviinfo.width; /*top of avatar (relative to eyepoint)*/
	       GLdouble abottom = -naviinfo.height; /*bottom of avatar (relative to eyepoint)*/
	       GLdouble astep = -naviinfo.height+naviinfo.step;


                float h = (node->height) /2;
                float r = (node->bottomRadius) ;

	       GLdouble modelMatrix[16];
	       GLdouble upvecmat[16];
	       struct point_XYZ iv = {0,0,0};
	       struct point_XYZ jv = {0,0,0};
	       GLdouble scale; /* FIXME: won''t work for non-uniform scales. */
	       struct point_XYZ t_orig = {0,0,0};

	       struct point_XYZ delta;
	       struct point_XYZ tupv = {0,1,0};

	       iv.y = h; jv.y = -h;

	       /* get the transformed position of the Sphere, and the scale-corrected radius. */
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

	       if(!fast_ycylinder_cone_intersect(abottom,atop,awidth,t_orig,scale*h,scale*r)) return;

	       /* get transformed box edges and position */
	       transform(&iv,&iv,modelMatrix);
	       transform(&jv,&jv,modelMatrix);

	       delta = cone_disp(abottom,atop,astep,awidth,jv,iv,scale*r);

	       vecscale(&delta,&delta,-1);
	       transform3x3(&delta,&delta,upvecmat);

	       accumulate_disp(&CollisionInfo,delta);

		#ifdef COLLISIONVERBOSE
	       if((fabs(delta.x) != 0. || fabs(delta.y) != 0. || fabs(delta.z) != 0.))
	           printf("COLLISION_CON: (%f %f %f) (%f %f %f)\n",
			  iv.x, iv.y, iv.z,
			  delta.x, delta.y, delta.z
			  );
	       if((fabs(delta.x != 0.) || fabs(delta.y != 0.) || fabs(delta.z) != 0.))
	           printf("iv=(%f %f %f) jv=(%f %f %f) bR=%f\n",
			  iv.x, iv.y, iv.z,
			  jv.x, jv.y, jv.z,
			  scale*r
			  );
		#endif
}

void collide_Cylinder (struct X3D_Cylinder *node) {
	       /*easy access, naviinfo.step unused for sphere collisions */
	       GLdouble awidth = naviinfo.width; /*avatar width*/
	       GLdouble atop = naviinfo.width; /*top of avatar (relative to eyepoint)*/
	       GLdouble abottom = -naviinfo.height; /*bottom of avatar (relative to eyepoint)*/
	       GLdouble astep = -naviinfo.height+naviinfo.step;

                float h = (node->height)/2;
                float r = (node->radius);


	       GLdouble modelMatrix[16];
	       GLdouble upvecmat[16];
	       struct point_XYZ iv = {0,0,0};
	       struct point_XYZ jv = {0,0,0};
	       GLdouble scale; /* FIXME: won''t work for non-uniform scales. */
	       struct point_XYZ t_orig = {0,0,0};

	       struct point_XYZ tupv = {0,1,0};
	       struct point_XYZ delta;


		iv.y = h;
		jv.y = -h;

	       /* get the transformed position of the Sphere, and the scale-corrected radius. */
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
	       if(!fast_ycylinder_cone_intersect(abottom,atop,awidth,t_orig,scale*h,scale*r)) return;



	       /* get transformed box edges and position */
	       transform(&iv,&iv,modelMatrix);
	       transform(&jv,&jv,modelMatrix);


	       delta = cylinder_disp(abottom,atop,astep,awidth,jv,iv,scale*r);

	       vecscale(&delta,&delta,-1);
	       transform3x3(&delta,&delta,upvecmat);

	       accumulate_disp(&CollisionInfo,delta);

		#ifdef COLLISIONVERBOSE
	       if((fabs(delta.x) != 0. || fabs(delta.y) != 0. || fabs(delta.z) != 0.))
	           printf("COLLISION_CYL: (%f %f %f) (%f %f %f)\n",
			  iv.x, iv.y, iv.z,
			  delta.x, delta.y, delta.z
			  );
	       if((fabs(delta.x != 0.) || fabs(delta.y != 0.) || fabs(delta.z) != 0.))
	           printf("iv=(%f %f %f) jv=(%f %f %f) bR=%f\n",
			  iv.x, iv.y, iv.z,
			  jv.x, jv.y, jv.z,
			  scale*r
			  );
		#endif
}

void collide_Extrusion (struct X3D_Extrusion *node) {
	       GLdouble awidth = naviinfo.width; /*avatar width*/
	       GLdouble atop = naviinfo.width; /*top of avatar (relative to eyepoint)*/
	       GLdouble abottom = -naviinfo.height; /*bottom of avatar (relative to eyepoint)*/
	       GLdouble astep = -naviinfo.height+naviinfo.step;
	       GLdouble modelMatrix[16];
	       GLdouble upvecmat[16];

	       GLdouble scale; /* FIXME: won''t work for non-uniform scales. */
	       struct point_XYZ t_orig = {0,0,0};

	       struct point_XYZ tupv = {0,1,0};
	       struct point_XYZ delta = {0,0,0};

	       struct X3D_PolyRep pr;
	       prflags flags = 0;
	       int change = 0;

		/* JAS - first pass, intern is probably zero */
		if (((struct X3D_PolyRep *)node->_intern) == 0) return;

		/* JAS - no triangles in this text structure */
		if ((((struct X3D_PolyRep *)node->_intern)->ntri) == 0) return;


	       /*save changed state.*/
	       if(node->_intern) change = ((struct X3D_PolyRep *)node->_intern)->irep_change;
                COMPILE_POLY_IF_REQUIRED(NULL, NULL, NULL, NULL)
 	       if(node->_intern) ((struct X3D_PolyRep *)node->_intern)->irep_change = change;
	       /*restore changes state, invalidates compile_polyrep work done, so it can be done
	         correclty in the RENDER pass */

	       if(!node->solid) {
		   flags = flags | PR_DOUBLESIDED;
	       }
/*	       printf("_PolyRep = %d\n",node->_intern);*/
	       pr = *((struct X3D_PolyRep*)node->_intern);
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
/*	       if(!fast_ycylinder_cone_intersect(abottom,atop,awidth,t_orig,scale*h,scale*r)) return;*/

/*	       printf("ntri=%d\n",pr.ntri);
	       for(i = 0; i < pr.ntri; i++) {
		   printf("cindex[%d]=%d\n",i,pr.cindex[i]);
	       }*/
	       delta = polyrep_disp(abottom,atop,astep,awidth,pr,modelMatrix,flags);

	       vecscale(&delta,&delta,-1);
	       transform3x3(&delta,&delta,upvecmat);

	       accumulate_disp(&CollisionInfo,delta);

		#ifdef COLLISIONVERBOSE
	       if((fabs(delta.x) != 0. || fabs(delta.y) != 0. || fabs(delta.z) != 0.))  {
/*		   printmatrix(modelMatrix);*/
		   fprintf(stderr,"COLLISION_EXT: (%f %f %f) (%f %f %f)\n",
			  t_orig.x, t_orig.y, t_orig.z,
			  delta.x, delta.y, delta.z
			  );
	       }
		#endif
}



void rendray_Sphere (struct X3D_Sphere *node) {
        float r = (node->radius) /*cget*/;
        /* Center is at zero. t_r1 to t_r2 and t_r1 to zero are the vecs */
        float tr1sq = VECSQ(t_r1);
        /* float tr2sq = VECSQ(t_r2); */
        /* float tr1tr2 = VECPT(t_r1,t_r2); */
        struct point_XYZ dr2r1;
        float dlen;
        float a,b,c,disc;

        VECDIFF(t_r2,t_r1,dr2r1);
        dlen = VECSQ(dr2r1);

        a = dlen; /* tr1sq - 2*tr1tr2 + tr2sq; */
        b = 2*(VECPT(dr2r1, t_r1));
        c = tr1sq - r*r;

        disc = b*b - 4*a*c; /* The discriminant */

        if(disc > 0) { /* HITS */
                float q ;
                float sol1 ;
                float sol2 ;
                float cx,cy,cz;
                q = sqrt(disc);
                /* q = (-b+(b>0)?q:-q)/2; */
                sol1 = (-b+q)/(2*a);
                sol2 = (-b-q)/(2*a);
                /*
                printf("SPHSOL0: (%f %f %f) (%f %f %f)\n",
                        t_r1.x, t_r1.y, t_r1.z, t_r2.x, t_r2.y, t_r2.z);
                printf("SPHSOL: (%f %f %f) (%f) (%f %f) (%f) (%f %f)\n",
                        tr1sq, tr2sq, tr1tr2, a, b, c, und, sol1, sol2);
                */ 
                cx = MRATX(sol1);
                cy = MRATY(sol1);
                cz = MRATZ(sol1);
                rayhit(sol1, cx,cy,cz, cx/r,cy/r,cz/r, -1,-1, "sphere 0");
                cx = MRATX(sol2);
                cy = MRATY(sol2);
                cz = MRATZ(sol2);
                rayhit(sol2, cx,cy,cz, cx/r,cy/r,cz/r, -1,-1, "sphere 1");
        }

}


void rendray_Box (struct X3D_Box *node) {
	float x = ((node->size).c[0])/2;
	float y = ((node->size).c[1])/2;
	float z = ((node->size).c[2])/2;
	/* 1. x=const-plane faces? */
	if(!XEQ) {
		float xrat0 = XRAT(x);
		float xrat1 = XRAT(-x);
		#ifdef RENDERVERBOSE 
		printf("!XEQ: %f %f\n",xrat0,xrat1);
		#endif

		if(TRAT(xrat0)) {
			float cy = MRATY(xrat0);
			#ifdef RENDERVERBOSE 
			printf("TRok: %f\n",cy);
			#endif

			if(cy >= -y && cy < y) {
				float cz = MRATZ(xrat0);
				#ifdef RENDERVERBOSE 
				printf("cyok: %f\n",cz);
				#endif

				if(cz >= -z && cz < z) {
					#ifdef RENDERVERBOSE 
					printf("czok:\n");
					#endif

					rayhit(xrat0, x,cy,cz, 1,0,0, -1,-1, "cube x0");
				}
			}
		}
		if(TRAT(xrat1)) {
			float cy = MRATY(xrat1);
			if(cy >= -y && cy < y) {
				float cz = MRATZ(xrat1);
				if(cz >= -z && cz < z) {
					rayhit(xrat1, -x,cy,cz, -1,0,0, -1,-1, "cube x1");
				}
			}
		}
	}
	if(!YEQ) {
		float yrat0 = YRAT(y);
		float yrat1 = YRAT(-y);
		if(TRAT(yrat0)) {
			float cx = MRATX(yrat0);
			if(cx >= -x && cx < x) {
				float cz = MRATZ(yrat0);
				if(cz >= -z && cz < z) {
					rayhit(yrat0, cx,y,cz, 0,1,0, -1,-1, "cube y0");
				}
			}
		}
		if(TRAT(yrat1)) {
			float cx = MRATX(yrat1);
			if(cx >= -x && cx < x) {
				float cz = MRATZ(yrat1);
				if(cz >= -z && cz < z) {
					rayhit(yrat1, cx,-y,cz, 0,-1,0, -1,-1, "cube y1");
				}
			}
		}
	}
	if(!ZEQ) {
		float zrat0 = ZRAT(z);
		float zrat1 = ZRAT(-z);
		if(TRAT(zrat0)) {
			float cx = MRATX(zrat0);
			if(cx >= -x && cx < x) {
				float cy = MRATY(zrat0);
				if(cy >= -y && cy < y) {
					rayhit(zrat0, cx,cy,z, 0,0,1, -1,-1,"cube z0");
				}
			}
		}
		if(TRAT(zrat1)) {
			float cx = MRATX(zrat1);
			if(cx >= -x && cx < x) {
				float cy = MRATY(zrat1);
				if(cy >= -y && cy < y) {
					rayhit(zrat1, cx,cy,-z, 0,0,-1,  -1,-1,"cube z1");
				}
			}
		}
	}
}


void rendray_Cylinder (struct X3D_Cylinder *node) {
        float h = (node->height) /*cget*//2; /* pos and neg dir. */
        float r = (node->radius) /*cget*/;
        float y = h;
        /* Caps */
        if(!YEQ) {
                float yrat0 = YRAT(y);
                float yrat1 = YRAT(-y);
                if(TRAT(yrat0)) {
                        float cx = MRATX(yrat0);
                        float cz = MRATZ(yrat0);
                        if(r*r > cx*cx+cz*cz) {
                                rayhit(yrat0, cx,y,cz, 0,1,0, -1,-1, "cylcap 0");
                        }
                }
                if(TRAT(yrat1)) {
                        float cx = MRATX(yrat1);
                        float cz = MRATZ(yrat1);
                        if(r*r > cx*cx+cz*cz) {
                                rayhit(yrat1, cx,-y,cz, 0,-1,0, -1,-1, "cylcap 1");
                        }
                }
        }
        /* Body -- do same as for sphere, except no y axis in distance */
        if((!XEQ) && (!ZEQ)) {
                float dx = t_r2.x-t_r1.x; float dz = t_r2.z-t_r1.z;
                float a = dx*dx + dz*dz;
                float b = 2*(dx * t_r1.x + dz * t_r1.z);
                float c = t_r1.x * t_r1.x + t_r1.z * t_r1.z - r*r;
                float und;
                b /= a; c /= a;
                und = b*b - 4*c;
                if(und > 0) { /* HITS the infinite cylinder */
                        float sol1 = (-b+sqrt(und))/2;
                        float sol2 = (-b-sqrt(und))/2;
                        float cy,cx,cz;
                        cy = MRATY(sol1);
                        if(cy > -h && cy < h) {
                                cx = MRATX(sol1);
                                cz = MRATZ(sol1);
                                rayhit(sol1, cx,cy,cz, cx/r,0,cz/r, -1,-1, "cylside 1");
                        }
                        cy = MRATY(sol2);
                        if(cy > -h && cy < h) {
                                cx = MRATX(sol2);
                                cz = MRATZ(sol2);
                                rayhit(sol2, cx,cy,cz, cx/r,0,cz/r, -1,-1, "cylside 2");
                        }
                }
        }
}

void rendray_Cone (struct X3D_Cone *node) {
	float h = (node->height) /*cget*//2; /* pos and neg dir. */
	float y = h;
	float r = (node->bottomRadius) /*cget*/;
	float dx = t_r2.x-t_r1.x; float dz = t_r2.z-t_r1.z;
	float dy = t_r2.y-t_r1.y;
	float a = dx*dx + dz*dz - (r*r*dy*dy/(2*h*2*h));
	float b = 2*(dx*t_r1.x + dz*t_r1.z) +
		2*r*r*dy/(2*h)*(0.5-t_r1.y/(2*h));
	float tmp = (0.5-t_r1.y/(2*h));
	float c = t_r1.x * t_r1.x + t_r1.z * t_r1.z
		- r*r*tmp*tmp;
	float und;
	b /= a; c /= a;
	und = b*b - 4*c;
	/*
	printf("CONSOL0: (%f %f %f) (%f %f %f)\n",
		t_r1.x, t_r1.y, t_r1.z, t_r2.x, t_r2.y, t_r2.z);
	printf("CONSOL: (%f %f %f) (%f) (%f %f) (%f)\n",
		dx, dy, dz, a, b, c, und);
	*/
	if(und > 0) { /* HITS the infinite cylinder */
		float sol1 = (-b+sqrt(und))/2;
		float sol2 = (-b-sqrt(und))/2;
		float cy,cx,cz;
		float cy0;
		cy = MRATY(sol1);
		if(cy > -h && cy < h) {
			cx = MRATX(sol1);
			cz = MRATZ(sol1);
			/* XXX Normal */
			rayhit(sol1, cx,cy,cz, cx/r,0,cz/r, -1,-1, "conside 1");
		}
		cy0 = cy;
		cy = MRATY(sol2);
		if(cy > -h && cy < h) {
			cx = MRATX(sol2);
			cz = MRATZ(sol2);
			rayhit(sol2, cx,cy,cz, cx/r,0,cz/r, -1,-1, "conside 2");
		}
		/*
		printf("CONSOLV: (%f %f) (%f %f)\n", sol1, sol2,cy0,cy);
		*/
	}
	if(!YEQ) {
		float yrat0 = YRAT(-y);
		if(TRAT(yrat0)) {
			float cx = MRATX(yrat0);
			float cz = MRATZ(yrat0);
			if(r*r > cx*cx + cz*cz) {
				rayhit(yrat0, cx, -y, cz, 0, -1, 0, -1, -1, "conbot");
			}
		}
	}
}
