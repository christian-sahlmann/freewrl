/*
=INSERT_TEMPLATE_HERE=

$Id: Collision.c,v 1.16 2010/12/07 18:27:50 crc_canada Exp $

Render the children of nodes.

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

#include "LinearAlgebra.h"
#include "Collision.h"

#define DJ_KEEP_COMPILER_WARNING 0

//static int dug9debug = 0;
//int	setdug9debug()
//{ dug9debug = 1; return 0;}
//int cleardug9debug()
//{ dug9debug = 0; return 0;}

#define swap(x,y) {double k = x; x = y; y = k; }
#define FLOAT_TOLERANCE 0.00000001
#if DJ_KEEP_COMPILER_WARNING
#define MAX_POLYREP_DISP_RECURSION_COUNT 10
#endif
#define STEPUP_MAXINCLINE 0.9

#ifdef DEBUGPTS
#define DEBUGPTSPRINT(x,y,z) printf(x,y,z)
#else
#define DEBUGPTSPRINT(x,y,z) {}
#endif

/*usefull pretty much everywhere*/
static const struct point_XYZ zero = {0,0,0};
static struct point_XYZ* clippedPoly1 = NULL;
static int clippedPoly1Size = 0; /* number of struct point_XYZ* 's in the clippedPoly data area */
static struct point_XYZ* clippedPoly2 = NULL;
static int clippedPoly2Size = 0; /* number of struct point_XYZ* 's in the clippedPoly data area */
static struct point_XYZ* clippedPoly3 = NULL;
static int clippedPoly3Size = 0; /* number of struct point_XYZ* 's in the clippedPoly data area */
static struct point_XYZ* clippedPoly4 = NULL;
static int clippedPoly4Size = 0; /* number of struct point_XYZ* 's in the clippedPoly data area */
static struct point_XYZ* clippedPoly5 = NULL;
static int clippedPoly5Size = 0; /* number of struct point_XYZ* 's in the clippedPoly data area */


/* JAS - make return val global, not local for polyrep-disp */
struct point_XYZ res ={0,0,0};

/*a constructor */
#define make_pt(p,xc,yc,zc) { p.x = (xc); p.y = (yc); p.z = (zc); }

int overlapMBBs(GLDOUBLE *MBBmin1, GLDOUBLE *MBBmax1, GLDOUBLE *MBBmin2, GLDOUBLE* MBBmax2)
{
	/* test for overlap between two axes aligned minimum bounding boxes MBBs in same space. 
	   returns true if they overlap
	   rule: must overlap in all 3 dimensions in order to intersect
	   dimension your MBB: double MBBmin[3], MBBmax[3];
	*/
	int i, overlap;
	overlap = 1;
	for(i=0;i<3;i++)
	{
		overlap = overlap && !(MBBmin1[i] > MBBmax2[i] || MBBmax1[i] < MBBmin2[i]);
	}
	return overlap;
}



/*accumulator function, for displacements. */
void accumulate_disp(struct sCollisionInfo* ci, struct point_XYZ add) {
    double len2 = vecdot(&add,&add);
    ci->Count++;
    VECADD(ci->Offset,add);
    if(len2 > ci->Maximum2)
	ci->Maximum2 = len2;
}

double closest_point_of_segment_to_y_axis(struct point_XYZ p1, struct point_XYZ p2) {
    /*the equation */
    double x12 = (p1.x - p2.x);
    double z12 = (p1.z - p2.z);
    double q = ( x12*x12 + z12*z12 );

    /* double i = ((q == 0.) ? 0.5 : (p1.x * x12 + p1.z * z12) / q); */
    double i = ((APPROX(q, 0)) ? 0.5 : (p1.x * x12 + p1.z * z12) / q);

    /* struct point_XYZ result; */

     /*clamp result to constraints */
    if(i < 0) i = 0.;
    if(i > 1) i = 1.;

    return i;

}


double closest_point_of_segment_to_origin(struct point_XYZ p1, struct point_XYZ p2) {
    /*the equation (guessed from above)*/
    double x12 = (p1.x - p2.x);
    double y12 = (p1.y - p2.y);
    double z12 = (p1.z - p2.z);
    double q = ( x12*x12 + y12*y12 + z12*z12 );

    /* double i = ((q == 0.) ? 0.5 : (p1.x * x12 + p1.y * y12 + p1.z * z12) / q); */
    double i = ((APPROX(q, 0)) ? 0.5 : (p1.x * x12 + p1.y * y12 + p1.z * z12) / q);

    /* struct point_XYZ result; */

     /*clamp result to constraints */
    if(i < 0) i = 0.;
    if(i > 1) i = 1.;

    return i;

}

/*n must be normal */
struct point_XYZ closest_point_of_plane_to_origin(struct point_XYZ b, struct point_XYZ n) {
    /*the equation*/
    double k = b.x*n.x + b.y*n.y + b.z*n.z;

    vecscale(&n,&n,k);

    return n;
}


/* [p1,p2[ is segment,  q1,q2 defines line */
/* ignores y coord. eg intersection is done on projection of segment and line on the y plane */
/* nowtice point p2 is NOT included, (for simplification elsewhere) */
int intersect_segment_with_line_on_yplane(struct point_XYZ* pk, struct point_XYZ p1, struct point_XYZ p2, struct point_XYZ q1, struct point_XYZ q2) {
    double k,l,quotient;

    /* p2 becomes offset */
    VECDIFF(p2,p1,p2);
    /* q2 becomes offset */
    VECDIFF(q2,q1,q2);

    /* if(q2.x == 0 && q2.z == 0.) */
    if(APPROX(q2.x, 0) && APPROX(q2.z, 0)) {
	/* degenerate case.
	it fits our objective to simply specify a random line. */
	q2.x = 1;
	q2.y = 0;
	q2.z = 0;
	}

    quotient = ((-p2.z)*q2.x + p2.x*q2.z);
    /* if(quotient == 0.) return 0; */
    if(APPROX(quotient, 0)) return 0;

    k = (p1.z*q2.x - q1.z*q2.x - p1.x*q2.z + q1.x*q2.z)/quotient;
    l = (p1.z*p2.x - p1.x*p2.z + p2.z*q1.x - p2.x*q1.z)/quotient;

    if((k >= 0.) && (k < 1.)) {
	vecscale(pk,&p2,k);
	VECADD(*pk,p1);
	return 1;
    } else
	return 0;

}

/*finds the intersection of the line pp1 + k n with a cylinder on the y axis.
  returns the 0,1 or 2 values.
 */
int getk_intersect_line_with_ycylinder(double* k1, double* k2, double r, struct point_XYZ pp1, struct point_XYZ n) {
    double b,a,sqrdelta,delta;
    /* int res = 0; */

    /*solves (pp1+ k n) . (pp1 + k n) = r^2 , ignoring y values.*/
    a = 2*(n.x*n.x + n.z*n.z);
    b = -2*(pp1.x*n.x + pp1.z*n.z);
    delta = (4*((pp1.x*n.x + pp1.z*n.z)*(pp1.x*n.x + pp1.z*n.z)) -
	     4*((n.x*n.x + n.z*n.z))*((pp1.x*pp1.x + pp1.z*pp1.z - r*r)));
    if(delta < 0.) return 0;
    sqrdelta = sqrt(delta);

    *k1 = (b+sqrdelta)/a;
    /* if(sqrdelta == 0.) return 1; */
    if(APPROX(sqrdelta, 0)) return 1;

    *k2 = (b-sqrdelta)/a;
    return 2;
}



/*projects a point on the surface of the cylinder, in the inverse direction of n.
  returns TRUE if exists.
   */
int project_on_cylindersurface(struct point_XYZ* res, struct point_XYZ p, struct point_XYZ n,double r) {
    double k1,k2;
    vecscale(&n,&n,-1.0);
    switch(getk_intersect_line_with_ycylinder(&k1,&k2,r,p,n)) {
    case 0:
	return 0;
    case 1:
    case 2:
	vecscale(res,&n,k1);
	VECADD(*res,p);
	return 1;
    }
    return 0;
}

/*finds the intersection of the line pp1 + k n with a sphere.
  returns the 0,1 or 2 values.
 */
int getk_intersect_line_with_sphere(double* k1, double* k2, double r, struct point_XYZ pp1, struct point_XYZ n) {
    double b,a,sqrdelta,delta;
    /* int res = 0; */

    /*solves (pp1+ k n) . (pp1 + k n) = r^2 */
    a = 2*(n.x*n.x + n.y*n.y + n.z*n.z);
    b = -2*(pp1.x*n.x + pp1.y*n.y + pp1.z*n.z);
    delta = (4*((pp1.x*n.x + pp1.y*n.y + pp1.z*n.z)*(pp1.x*n.x + pp1.y*n.y + pp1.z*n.z)) -
	     4*((n.x*n.x + n.y*n.y + n.z*n.z))*((pp1.x*pp1.x + pp1.y*pp1.y + pp1.z*pp1.z - r*r)));
    if(delta < 0.) return 0;
    sqrdelta = sqrt(delta);

    *k1 = (b+sqrdelta)/a;
    /* if(sqrdelta == 0.) return 1; */
    if(APPROX(sqrdelta, 0)) return 1;

    *k2 = (b-sqrdelta)/a;
    return 2;
}

/*projects a point on the surface of the sphere, in the inverse direction of n.
  returns TRUE if exists.
   */
int project_on_spheresurface(struct point_XYZ* res, struct point_XYZ p, struct point_XYZ n,double r) {
    double k1,k2;
    vecscale(&n,&n,-1.0);
    switch(getk_intersect_line_with_sphere(&k1,&k2,r,p,n)) {
    case 0:
	return 0;
    case 1:
    case 2:
	vecscale(res,&n,k1);
	VECADD(*res,p);
	return 1;
    }
    return 0;
}


/*projects a point on the y="y" plane, in the direction of n. *
  n probably needs to be normal. */
struct point_XYZ project_on_yplane(struct point_XYZ p1, struct point_XYZ n,double y) {
    struct point_XYZ ret;
    make_pt(ret,p1.x - (n.x*(p1.y-y))/n.y,y,(p1.z - (n.z*(p1.y-y))/n.y));
    return ret;
}

/*projects a point on the plane tangent to the surface of the cylinder at point -kn (the prolonged normal)
  , in the inverse direction of n.
  n probably needs to be normal. */
struct point_XYZ project_on_cylindersurface_plane(struct point_XYZ p, struct point_XYZ n,double r) {
    struct point_XYZ pp;
    struct point_XYZ ret;
    vecscale(&n,&n,-1.0);
    pp = n;
    pp.y = 0;
    vecnormal(&pp,&pp);
    vecscale(&pp,&pp,r);

    ret.x = p.x + (n.x*((n.x*((pp.x - p.x)) + n.z*((pp.z - p.z)))))/(n.x*n.x + n.z*n.z);
    ret.y = p.y + (n.y*((n.x*((pp.x - p.x)) + n.z*((pp.z - p.z)))))/(n.x*n.x + n.z*n.z);
    ret.z = p.z + (n.z*((n.x*((pp.x - p.x)) + n.z*((pp.z - p.z)))))/(n.x*n.x + n.z*n.z);

    return ret;
}

/*makes half-plane starting at point, perpendicular to plane (eg: passing through origin)
  if this plane cuts through polygon edges an odd number of time, we are inside polygon*/
/* works for line passing through origin, polygon plane must not pass through origin. */
int perpendicular_line_passing_inside_poly(struct point_XYZ a,struct point_XYZ* p, int num) {
    struct point_XYZ n;  /*half-plane will be defined as: */
    struct point_XYZ i;  /* p(x,y) = xn + yi, with i >= 0 */
    struct point_XYZ j;  /*  j is half-plane normal */
    int f,sectcount = 0;
    struct point_XYZ epsilon; /* computationnal trick to handle points directly on plane. displace them. */
    /* if(vecnormal(&n,&a) == 0) */
    if(APPROX(vecnormal(&n,&a), 0)) {
	/* happens when polygon plane passes through origin */
	return 0;
    }
    make_orthogonal_vector_space(&i,&j,n);

    vecscale(&epsilon,&j,FLOAT_TOLERANCE); /*make our epsilon*/

    for(f = 0; f < num; f++) {
	/*segment points relative to point a */
	struct point_XYZ p1,p2;
	double p1j,p2j;
	VECDIFF(p[f],a,p1);
	VECDIFF(p[(f+1)%num],a,p2);
	/* while((p1j = vecdot(&p1,&j)) == 0.) VECADD(p1,epsilon); */
	while(APPROX((p1j = vecdot(&p1,&j)), 0)) VECADD(p1,epsilon);

	/* while((p2j = vecdot(&p2,&j)) == 0.) VECADD(p2,epsilon); */
	while(APPROX((p2j = vecdot(&p2,&j)), 0)) VECADD(p2,epsilon);

	/*see if segment crosses plane*/
	if(p1j * p2j <= 0 /*if signs differ*/) {
	    double k;
	    struct point_XYZ p0;
	    /* solves (k p1 + (1 - k)p2).j  = 0 */
	    /* k = (p1j-p2j != 0) ? (p1j/ (p1j - p2j)) : 0.; */
	    k = (! APPROX(p1j-p2j, 0)) ? (p1j/ (p1j - p2j)) : 0.;

	    /*see if point on segment that is on the plane (p0), is also on the half-plane */
	    p0 = weighted_sum(p1, p2, k);
	    if(vecdot(&p0,&i) >= 0)
		sectcount++;
	}
    }

    return sectcount % 2;

}


/*finds the intersection of the segment(pp1,pp2) with a cylinder on the y axis.
  returns the 0,1 or 2 values in the range [0..1]
 */
int getk_intersect_segment_with_ycylinder(double* k1, double* k2, double r, struct point_XYZ pp1, struct point_XYZ pp2) {
    double b,a,sqrdelta,delta;
    int res = 0;

    /* pp2 becomes offset */
    VECDIFF(pp2,pp1,pp2);

    /*solves (pp1+ k pp2) . (pp1 + k pp2) = r^2 */
    a = 2*(pp2.x*pp2.x + pp2.z*pp2.z);
    b = -2*(pp1.x*pp2.x + pp1.z*pp2.z);
    delta = (4*((pp1.x*pp2.x + pp1.z*pp2.z)*(pp1.x*pp2.x + pp1.z*pp2.z)) -
	     4*((pp2.x*pp2.x + pp2.z*pp2.z))*((pp1.x*pp1.x + pp1.z*pp1.z - r*r)));
    if(delta < 0.) return 0;
    sqrdelta = sqrt(delta);
    /* keep the ks that are in the segment */
    *k1 = (b+sqrdelta)/a;
    *k2 = (b-sqrdelta)/a;

    if(*k1 >= 0. && *k1 <= 1.) res++;
    if(*k2 >= 0. && *k2 <= 1.) res++;
    if(res == 1 && (*k1 < 0. || *k1 > 1.)) swap(*k1,*k2);
/*    if(res == 2 && sqrdelta == 0.) res = 1; */

    return res;
}

/*returns (1-k)p1 + k p2 */
struct point_XYZ weighted_sum(struct point_XYZ p1, struct point_XYZ p2, double k) {
    struct point_XYZ ret;
    make_pt(ret,
	    p1.x*(1-k)+p2.x*k,
	    p1.y*(1-k)+p2.y*k,
	    p1.z*(1-k)+p2.z*k);

	/* printf ("weighted sum, from %lf %lf %lf, %lf %lf %lf, return %lf %lf %lf\n",
	   p1.x,p1.y,p1.z,p2.x,p2.y,p2.z,ret.x,ret.y,ret.z); */
    return ret;
}


/*used by get_poly_normal_disp to clip the polygon on the cylinder caps, called twice*/
/*used by get_poly_step_disp to clip the polygon in the cylinder, by bypassing projection
  Code reuse please.
 */
int helper_poly_clip_cap(struct point_XYZ* clippedpoly, int clippedpolynum, const struct point_XYZ* p, int num, double r, struct point_XYZ n, double y, int stepping)
{
    struct point_XYZ* ppoly;
    int allin = 1;
    int i;

    if(!stepping) {
		ppoly = MALLOC(struct point_XYZ*, sizeof(struct point_XYZ) * num);

		/*sqush poly on cylinder cap plane.*/
		for(i= 0; i < num; i++) {
			ppoly[i] = project_on_yplane(p[i],n,y);
		}
    } else
		ppoly = (struct point_XYZ*)p; /*const cast*/

    /*find points of poly hitting cylinder cap*/
    for(i= 0; i < num; i++) {
		if(ppoly[i].x*ppoly[i].x + ppoly[i].z*ppoly[i].z > r*r) {
			allin = 0;
		} else {
			DEBUGPTSPRINT("intersect_point_cap(%f)= %d\n",y,clippedpolynum);
			clippedpoly[clippedpolynum++] = ppoly[i];
		}
    }

    if(!allin) {
		int numdessect = 0;
		struct point_XYZ dessect[2];
		double k1,k2;
		int nsect;

		/*find intersections of poly with cylinder cap edge*/
		for(i=0; i <num; i++) {
			nsect = getk_intersect_segment_with_ycylinder(&k1,&k2,r,ppoly[i],ppoly[(i+1)%num]);
			switch(nsect) {
				case 2:
				if(fabs(k1-k2) < FLOAT_TOLERANCE) /* segment touches edge of circle. we want to ignore this. */
					break;
				DEBUGPTSPRINT("intersect_segment_cap(%f)_2= %d\n",y,clippedpolynum);
				clippedpoly[clippedpolynum++] = weighted_sum(ppoly[i],ppoly[(i+1)%num],k2);
				case 1:
				DEBUGPTSPRINT("intersect_segment_cap(%f)_1= %d\n",y,clippedpolynum);
				clippedpoly[clippedpolynum++] = weighted_sum(ppoly[i],ppoly[(i+1)%num],k1);
				case 0: break;
			}
			/*find points of poly intersecting descending line on poly*/
			if((numdessect != 2) && intersect_segment_with_line_on_yplane(&dessect[numdessect],ppoly[i],ppoly[(i+1)%num],n,zero)) {
				numdessect++;
			}
		}
		/*find intersections of descending segment too.
		  these will point out maximum and minimum in cylinder cap edge that is inside triangle */
		if(numdessect == 2) {
			nsect = getk_intersect_segment_with_ycylinder(&k1,&k2,r,dessect[0],dessect[1]);
			switch(nsect) {
				case 2:
				if(fabs(k1-k2) < FLOAT_TOLERANCE) /* segment touches edge of circle. we want to ignore this. */
					break;
				DEBUGPTSPRINT("intersect_descending_segment_cap(%f)_2= %d\n",y,clippedpolynum);
				clippedpoly[clippedpolynum++] = weighted_sum(dessect[0],dessect[1],k2);
				case 1:
				DEBUGPTSPRINT("intersect_descending_segment_cap(%f)_1= %d\n",y,clippedpolynum);
				clippedpoly[clippedpolynum++] = weighted_sum(dessect[0],dessect[1],k1);
				case 0: break;
			}
		}
    }

    if(!stepping) {FREE_IF_NZ (ppoly);}

    return clippedpolynum;
}


/* yes, global. for speed optimizations. */

double get_poly_mindisp;

/*feed a poly, and stats of a cylinder, it returns the displacement in the direction of the
  normal of the poly that is needed for them not to intersect any more.*/
struct point_XYZ get_poly_normal_disp(double y1, double y2, double r, struct point_XYZ* p, int num, struct point_XYZ n) 
{
    int i;
    double polydisp;
    struct point_XYZ result;

    int clippedPoly1num = 0;

    get_poly_mindisp = 1E90;

#ifdef DEBUGFACEMASK
    printf("facemask = %d, debugsurface = %d\n",facemask,debugsurface);
    if((facemask & (1 <<debugsurface++)) ) return zero;
#endif

    /*allocate data */
    if ((num*5+4)>clippedPoly1Size) 
	{
        clippedPoly1 = (struct point_XYZ*) REALLOC(clippedPoly1,sizeof(struct point_XYZ) * (num*5+4));
        clippedPoly1Size = num*5+4;
    }

    /*if normal not specified, calculate it */
    /* if(n.x == 0 && n.y == 0 && n.z == 0) */
    if(APPROX(n.x, 0) && APPROX(n.y, 0) && APPROX(n.z, 0)) 
	{
		polynormal(&n,&p[0],&p[1],&p[2]);
    }

    for(i = 0; i < num; i++) 
	{
		if(project_on_cylindersurface(&clippedPoly1[clippedPoly1num],weighted_sum(p[i],p[(i+1)%num],closest_point_of_segment_to_y_axis(p[i],p[(i+1)%num])),n,r) &&
		   clippedPoly1[clippedPoly1num].y < y2 &&
		   clippedPoly1[clippedPoly1num].y > y1 ) 
		{

			DEBUGPTSPRINT("intersect_closestpolypoints_on_surface[%d]= %d\n",i,clippedPoly1num);
			clippedPoly1num++;
		}
    }

    /* clip polygon on top and bottom cap */
    /* if(n.y!= 0.) */
    if(! APPROX(n.y, 0)) 
	{
		clippedPoly1num = helper_poly_clip_cap(clippedPoly1, clippedPoly1num, p, num, r, n, y1, 0 /*stepping false*/);
		clippedPoly1num = helper_poly_clip_cap(clippedPoly1, clippedPoly1num, p, num, r, n, y2, 0 /*stepping false*/);
    }

    /*find intersections of poly with cylinder side*/
    /* if(n.y != 1. && n.y != -1.) { */ /*n.y == +/-1 makes n.x == n.z == 0, wich does div's by 0, besides making no sense at all. */
    if(! APPROX(n.y, 1) && ! APPROX(n.y, -1)) 
	{ /*n.y == +/-1 makes n.x == n.z == 0, wich does div's by 0, besides making no sense at all. */
		int numdessect3d = 0;
		struct point_XYZ dessect3d[2];
		double k1,k2;
		int nsect;

		for(i=0; i <num; i++) 
		{
			/*find points of poly intersecting descending line on poly, (non-projected)*/
			if((numdessect3d != 2) && intersect_segment_with_line_on_yplane(&dessect3d[numdessect3d],p[i],p[(i+1)%num],n,zero)) 
			{
				numdessect3d++;
			}
		}
		if(numdessect3d == 2) 
		{
			dessect3d[0] = project_on_cylindersurface_plane(dessect3d[0],n,r);
			dessect3d[1] = project_on_cylindersurface_plane(dessect3d[1],n,r);

			/*only do/correct points if dessect3d line is somewhere inside the cylinder */
			if((dessect3d[0].y <= y2 || dessect3d[1].y <= y2) && (dessect3d[0].y >= y1 || dessect3d[1].y >= y1)) 
			{
				if(dessect3d[0].y > y2) dessect3d[0].y = y2;
				if(dessect3d[0].y < y1) dessect3d[0].y = y1;
				if(dessect3d[1].y > y2) dessect3d[1].y = y2;
				if(dessect3d[1].y < y1) dessect3d[1].y = y1;

				DEBUGPTSPRINT("project_on_cylindersurface_plane(%d)= %d\n",1,clippedPoly1num);
				clippedPoly1[clippedPoly1num++] = dessect3d[0];
				DEBUGPTSPRINT("project_on_cylindersurface_plane(%d)= %d\n",2,clippedPoly1num);
				clippedPoly1[clippedPoly1num++] = dessect3d[1];
			}
		}
		{ /*find intersections on cylinder of polygon points projected on surface */
			struct point_XYZ sect;
			for(i = 0; i < num; i++) 
			{
				nsect = getk_intersect_line_with_ycylinder(&k1, &k2, r, p[i], n);
				if(nsect == 0) continue;

				/*sect = p[i] + k2 n*/
				vecscale(&sect,&n,k2);
				VECADD(sect,p[i]);

				if(sect.y > y1 && sect.y < y2) 
				{
					DEBUGPTSPRINT("intersect_polypoints_on_surface[%d]= %d\n",i,clippedPoly1num);
					clippedPoly1[clippedPoly1num++] = sect;
				}
			}
		}
    }
#ifdef DEBUGPTS
    for(i=0; i < clippedPoly1num; i++) {
	debugpts.push_back(clippedPoly1[i]);
    }
#endif
    /*here we find mimimum displacement possible */
    polydisp = vecdot(&p[0],&n);

    /*calculate farthest point from the "n" plane passing through the origin */
    for(i = 0; i < clippedPoly1num; i++) 
	{
		double disp = vecdot(&clippedPoly1[i],&n) - polydisp;
		if(disp < get_poly_mindisp) 
		{
			get_poly_mindisp = disp;
		}
    }
    if(get_poly_mindisp <= 0.) 
	{
	    vecscale(&result,&n,get_poly_mindisp);
    } else
		result = zero;

    /*free alloc'd data */
    return result;
}
/*feed a poly, and stats of a cylinder, it returns the vertical displacement that is needed for them not to intersect any more,
  if this displacement is less than the height of the cylinder (y2-y1).*/

struct point_XYZ get_poly_step_disp(double y1, double y2, double r, struct point_XYZ* p, int num, struct point_XYZ n) {
    int i;
    /* int allin = 1; */
    double dmax = -1E99;
    double pmax = -1E99;
    struct point_XYZ result;
    int clippedPoly2num = 0;

    get_poly_mindisp = 1E90;

#ifdef DEBUGFACEMASK
    printf("facemask = %d, debugsurface = %d\n",facemask,debugsurface);
    if((facemask & (1 <<debugsurface++)) ) return zero;
#endif

    /*if normal not specified, calculate it */
    /* if(n.x == 0 && n.y == 0 && n.z == 0) */
    if(APPROX(n.x, 0) && APPROX(n.y, 0) && APPROX(n.z, 0)) {
		polynormal(&n,&p[0],&p[1],&p[2]);
    }


	/*get highest point (not nessesarily inside)*/
	for(i = 0; i < num; i++) {
		if(p[i].y > pmax)
			pmax = p[i].y;
	}
	if(((pmax > y2 || n.y < 0) && n.y < STEPUP_MAXINCLINE)) /*to high to step on and to steep to step on or facing downward*/
		return zero;


	/*allocate data */
	if ((num*3+4)>clippedPoly2Size) {
		clippedPoly2 = (struct point_XYZ*) REALLOC(clippedPoly2,sizeof(struct point_XYZ) * (num*3+4));
		clippedPoly2Size = num*3+4;
	}
	clippedPoly2num = helper_poly_clip_cap(clippedPoly2, clippedPoly2num, p, num, r, n, y1, 1 /*stepping true*/ );

#ifdef DEBUGPTS
	for(i=0; i < clippedPoly2num; i++) {
	debugpts.push_back(clippedPoly2[i]);
	}
#endif

	/*get maximum*/
	for(i = 0; i < clippedPoly2num; i++) {
		if(clippedPoly2[i].y > dmax)
			dmax = clippedPoly2[i].y;
	}

    /*diplace only if displacement completely clears polygon*/
    if(dmax > y2)
		return zero;

    get_poly_mindisp = y1-dmax;

    if(dmax > y1) {
		result.x = 0;
		result.y = get_poly_mindisp;
		result.z = 0;
		return result;
    } else
		return zero;
}

/*feed a poly, and stats of a cylinder, it returns the displacement in the direction of the
  normal of the poly that is needed for them not to intersect any more, or vertically if contact point below ystep*/
struct point_XYZ get_poly_disp(double y1, double y2, double ystep, double r, struct point_XYZ* p, int num, struct point_XYZ n) {
    struct point_XYZ result;
	result.x = result.y = result.z = 0.0;
    result = get_poly_step_disp(y1,ystep,r,p,num,n); /*dug9 - competes with climb for the knee-down part*/
    /* if(result.y != 0.) */
    if(! APPROX(result.y, 0)) {
		return result;
    } else {
		return get_poly_normal_disp(y1,y2,r,p,num,n); /*dug9 - for walking should be used instead of sphere: just do this */
	}
}
void accumulateFallingClimbing(double y1, double y2, double ystep, struct point_XYZ *p, int num, struct point_XYZ nused, double *tmin, double *tmax);

int pointOnPlaneInsidePoly(struct point_XYZ D,struct point_XYZ *p, int num, struct point_XYZ* n )
{
	int i,j,inside;

	struct point_XYZ a, b,c,last; //,d,e;

	inside = 1;
	for(i=0;i<num;i++)
	{
		j = (i+1)%num; //i==(num-1)? 0 : i + 1;
		vecdiff(&a,&D,&p[i]);
		vecdiff(&b,&p[j],&p[i]);
		veccross(&c,a,b);
		if( i > 0 )
			inside = inside && vecdot(&last,&c) >= 0.0;
		last = c;
	}
	return inside;
}

int intersectLineSegmentWithPoly(struct point_XYZ s0,struct point_XYZ s1,double r, struct point_XYZ *p,int num,struct point_XYZ n,double *dr)
{
	/* returns 1 if a line segment intersects a convex planar polygon, else 0
	   if 1, returns dr the fraction of D=(s1-s0) where the intersection point is
	   so to get the intersection point from dr, go (s1-s0)*dr + s0 in your calling code
	   tested use: send in s1 normalized (length 1), s0=0,0,0 and r the length you want the segment
	   see p.391 of Graphics Gems I (a book)
	   line segment:
	      s0 - starting point of ray
		  s1 - direction vector of ray unit length
		  r - scalar length of ray
	   polygon:
	      p[num] - points
		  n - normal (precomputed)
	   return: 
	      0 - no intersection 
		  1 - intersection within poly and within line segment
		  dr - intersection point expressed as fraction of (s1-s0) in range 0 to r
	*/
	   
	int hit = 0;
	double d,t, ndotD;
	struct point_XYZ D;
	/* step1 intersect infinite line with infinite plane */
	d = -vecdot(&n,&p[0]); /* compute plane constant */
	VECDIFF(s1,s0,D); /* compute ray direction vector from line segment start and end points */
	/* vecnormal(&D,&D); D should already be normalized - just sin & cos values */
	ndotD = vecdot(&n,&D);
	*dr = 0.0; /* displacement inward from r */
	if(APPROX(ndotD,0.0) )
	{
		/* infinite plane and infinite line are parallel - no intersection */
		*dr = 0.0;
		return hit;
	}
	t = - ( (d + vecdot(&n,&s0)) / ndotD ); /*magic plane-line intersection equation - t is parametric value of intersection point on line */
	/* step2 test intersection in line segment */
	if( t < 0.0 || t > r )
	{
		/* intersection is outside of line segment */
		return hit;
	}

	/* step3 test intersection in poly */
	vecscale(&D,&D,t);
	VECADD(D,s0);
	hit = pointOnPlaneInsidePoly(D,p,num,&n);
	if( hit ) *dr = t;
	return hit;
}

/*feed a poly, and stats of a cylinder, it returns the displacement in the radial direction of the
  avatar that is needed for them not to intersect any more */
struct point_XYZ get_poly_radialSample_disp(double y1, double y2, double ystep, double r,struct point_XYZ* p, int num, struct point_XYZ n, double *tmin, double *tmax)
{ 
	/* uses a statistical sampler method - 8 radial rays at the ystep, avatar origin Y=0, and y2, levels, each ray segment r long 
	   It's a sampler because it will miss small things. But it is supposed to catch major things like walls.
	*/
	int i,j,hit;
	double level[3],dr,dmax,eighth,theta, avmin[3], avmax[3];
	struct point_XYZ s0,s1,result = zero;
	level[0] = ystep;
	level[1] = 0.0;
	level[2] = y2;
	s0.x = s0.y = s0.z = 0.0; /*avatar axis*/
	dmax = 0.0;
	eighth = M_PI * 2.0 / 8.0;

	for(i=0;i<3;i++)
	{
		avmin[i] = -r;
		avmax[i] =  r;
	}
	avmin[1] = ystep; avmax[1] = y2;
	if(!overlapMBBs(avmin,avmax,tmin,tmax)) return result;


	for(i=0;i<3;i++)
	{
		s0.y = level[i];
		s1.y = level[i];
		theta = 0.0;
		for(j=0;j<8;j++)
		{
			theta += eighth;
			s1.x = cos(theta);
			s1.z = sin(theta);
			/* quick check of overlap */
			avmin[0] = min(s0.x,s1.x);
			avmin[1] = min(s0.y,s1.y);
			avmin[2] = min(s0.z,s1.z);
			avmax[0] = max(s0.x,s1.x);
			avmax[1] = max(s0.y,s1.y);
			avmax[2] = max(s0.z,s1.z);
			if( overlapMBBs(avmin,avmax,tmin,tmax) )
			{
				hit = intersectLineSegmentWithPoly(s0,s1,r,p,num,n,&dr);
				if(hit)
				{
					if( (dr > FLOAT_TOLERANCE) && (dr > dmax) )
					{
						dmax = r-dr;
						vecscale(&result,&s1,r-dr);
						result.y = 0.0;
						//printf(">");
					}
				}
			}
		}
	}
	/* process hits to find optimal direction and magnitude */
	return result;
}

int get_poly_penetration_disp( double r,struct point_XYZ* p, int num, struct point_XYZ n, double *tmin, double *tmax, struct point_XYZ *result, double *rmax)
{
	/*	checks for wall penetration and computes a correction
	    checks between the convex poly you pass in, 
		and LastPos-Pos vector saved in FallInfo struct in render_collision()
		r - avatar radius - will be added onto a penetration correction so avatar is repositioned off the wall
		p[num] - convex planar poly to test 
		n - poly normal precomputed
		tmin[3],tmax[3] - poly MBB precomputed
		returns: 
			zero (0,0,0) if no intersection else 
			result - the displacement vector needed to move the avatar back
		all coordinates in collision space 
	*/
	int hit = 0; 
	double dr; 
	struct point_XYZ s0=zero;
	*result = zero;
	*rmax = 0.0;

	/*	from FallInfo struct:
		penvec - unit vector in direction from avatar (0,0,0) toward the last avatar position on the last loop (or more precisely, last time checked)
		penRadius - distance between avatar(0,0,0) and last avatar position 0+ - infinity 
		penMin[3],penMax[3] - pre-computed MBB of penvec
		all coordinates in collision space
	*/
	if( overlapMBBs(FallInfo.penMin,FallInfo.penMax,tmin,tmax) )
	{
		/* penvec length 1.0 normalized. penRadius can be 0 to 1000+ - how far did you travel/navigate on one loop */
		hit = intersectLineSegmentWithPoly(s0,FallInfo.penvec,FallInfo.penRadius,p,num,n,&dr);
		if(hit)
		{
			vecscale(result,&FallInfo.penvec,(dr + r));
			*rmax = dr;
		}
	}
	return hit; 
}

//struct point_XYZ get_poly_disp_2(double y1, double y2, double ystep, double r, struct point_XYZ* p, int num, struct point_XYZ n) {
struct point_XYZ get_poly_disp_2(struct point_XYZ* p, int num, struct point_XYZ n) {
	/* 
		generalized logic for all planar facet geometry and avatar collision volumes 
		If you can break your geometry into planar facets, then call this for each facet. 
		We will know what to do here based on global structs.

		 The avatar collision volume - 
		 A.for flying/examining:
		    1. sphere.

		 B.For walking:
		   1. cylinder (body) from  (-Height + stepSize) to (+avatarWidth)
		   2. line - climb/fall (legs,feet) from (-FallInfo.FallHeight to 0)
		   3. ray to last avatar position for wall penetration
		   Order of tests (so can exit early to save unnecessary computations):
		   a) ray b) cylinder c) line
		   If you have a wall penetration ray hit, you don't need to check for cylinder collision.
		   If you have a cylinder hit, you don't need to test climb/fall.
		   if you have a climb, you can skip the fall.
		   otherwise test for fall.

		   fast tests using MBB in the caller elliminate some volumes by setting check variables to true if MBB overlap
		   checkPenetration - ray for wall penetration should be tested
		   checkCylinder - sphere or cyclinder should be tested
		   checkFall - climb and fall should be tested
	*/

    struct point_XYZ result;
	int hit,i;
	double tmin[3],tmax[3]; /* MBB for facet */
	GLDOUBLE awidth = naviinfo.width; /*avatar width*/
	GLDOUBLE atop = naviinfo.width; /*top of avatar (relative to eyepoint)*/
	GLDOUBLE abottom = -naviinfo.height; /*bottom of avatar (relative to eyepoint)*/
	GLDOUBLE astep = -naviinfo.height+naviinfo.step;
	
	result = zero;
	get_poly_mindisp = 0.0;
	if(FallInfo.walking)
	{
		tmin[0] = tmax[0] = p[0].x;
		tmin[1] = tmax[1] = p[0].y;
		tmin[2] = tmax[2] = p[0].z;
		for(i=1;i<num;i++)
		{
			tmin[0] = min(tmin[0],p[i].x);
			tmin[1] = min(tmin[1],p[i].y);
			tmin[2] = min(tmin[2],p[i].z);
			tmax[0] = max(tmax[0],p[i].x);
			tmax[1] = max(tmax[1],p[i].y);
			tmax[2] = max(tmax[2],p[i].z);
		}

		/* walking */
		hit = 0;
		if(FallInfo.checkPenetration)
		{
			double rmax;
			struct point_XYZ presult;
			hit = get_poly_penetration_disp(awidth,p,num,n,tmin,tmax,&presult,&rmax);
			if(hit) 
			{
				FallInfo.isPenetrate = 1;
				if(rmax > FallInfo.pendisp)
				{
					FallInfo.pencorrection = presult;
					FallInfo.pendisp = rmax;
				}
			}
		}
		if(FallInfo.checkCylinder && !hit)
		{
			/* the poly normal is jerky with wild displacements. I need a cylinder collision like the sphere. */
			/* get_poly_normal_disp(y1,y2,r,p,num,n); */
			/* 0=sphere 1=normal_cylinder 2=disp_ 3=sampler. Jan 15, 2010 Sampler is the best for walking. Sphere works but ducks */
			switch(FallInfo.walkColliderMethod)
			{
				case 0: result = get_poly_min_disp_with_sphere(awidth, p, num, n); break;
				case 1: result = get_poly_normal_disp(abottom,atop,awidth,p,num,n);break; /*y1-ystep,y2,r,p,num,n); dug9 - for walking should be used instead of sphere: just do this */
				case 2:	result = get_poly_disp(abottom,atop,astep,awidth,p,num,n); break;/*y1-ystep,y2,r,p,num,n); dug9 - for walking should be used instead of sphere: just do this */
				case 3: result = get_poly_radialSample_disp(abottom,atop,astep,awidth,p,num,n,tmin,tmax); break; //statistical method
				case 4: result = get_poly_step_disp(abottom,atop,awidth,p,num,n); break; //<<< doesn't make sense I've got a cylinder going from foot to ystep
			}

			hit = !(APPROX(result.x, 0) && APPROX(result.y, 0) && APPROX(result.z, 0));
			if(hit) 
				FallInfo.canFall = 0; /* rule: don't fall or climb if we are colliding */
		}
		if(FallInfo.checkFall && !hit)
		{
			accumulateFallingClimbing(abottom,atop,astep,p,num,n,tmin,tmax); //y1, y2, p, num, n);

		}
	}
	else
	{
		/* fly, examine */
		result = get_poly_min_disp_with_sphere(awidth, p, num, n);
	}
	get_poly_mindisp = vecdot(&result,&result);
	return result;
}


/*feed a poly, and radius of a sphere, it returns the displacement in the direction of the
  normal of the poly that is needed for them not to intersect any more.*/
struct point_XYZ get_poly_normal_disp_with_sphere(double r, struct point_XYZ* p, int num, struct point_XYZ n) {
    int i;
    double polydisp;
    struct point_XYZ result;

    //double get_poly_mindisp;
    int clippedPoly3num = 0;

    get_poly_mindisp = 1E90;

    /*allocate data */
    if ((num+1)>clippedPoly3Size) {
    	clippedPoly3 = (struct point_XYZ*) REALLOC(clippedPoly3,sizeof(struct point_XYZ) * (num+1));
	clippedPoly3Size = num+1;
    }

    /*if normal not specified, calculate it */
    /* if(n.x == 0 && n.y == 0 && n.z == 0) */
    if (APPROX(n.x, 0) && APPROX(n.y, 0) && APPROX(n.z, 0)) {
		polynormal(&n,&p[0],&p[1],&p[2]);
    }

    for(i = 0; i < num; i++) {
	if( project_on_spheresurface(&clippedPoly3[clippedPoly3num],weighted_sum(p[i],p[(i+1)%num],closest_point_of_segment_to_origin(p[i],p[(i+1)%num])),n,r) )
	{
	    DEBUGPTSPRINT("intersect_closestpolypoints_on_surface[%d]= %d\n",i,clippedPoly3num);
	    clippedPoly3num++;
	}
    }

    /*find closest point of polygon plane*/
    clippedPoly3[clippedPoly3num] = closest_point_of_plane_to_origin(p[0],n);

    /*keep if inside*/
    if(perpendicular_line_passing_inside_poly(clippedPoly3[clippedPoly3num],p, num)) {
	/*good, project it on surface*/

	vecscale(&clippedPoly3[clippedPoly3num],&clippedPoly3[clippedPoly3num],r/veclength(clippedPoly3[clippedPoly3num]));

	DEBUGPTSPRINT("perpendicular_line_passing_inside_poly[%d]= %d\n",0,clippedPoly3num);
	clippedPoly3num++;
    }


#ifdef DEBUGPTS
    for(i=0; i < clippedPoly3num; i++) {
	debugpts.push_back(clippedPoly3[i]);
    }
#endif

    /*here we find mimimum displacement possible */
    polydisp = vecdot(&p[0],&n);

    /*calculate farthest point from the "n" plane passing through the origin */
    for(i = 0; i < clippedPoly3num; i++) {
	double disp = vecdot(&clippedPoly3[i],&n) - polydisp;
	if(disp < get_poly_mindisp) {
	    get_poly_mindisp = disp;
	}
    }
    if(get_poly_mindisp <= 0.) {
	vecscale(&result,&n,get_poly_mindisp);
    } else
	result = zero;

    return result;
}


/*feed a poly, and radius of a sphere, it returns the minimum displacement and
  the direction  that is needed for them not to intersect any more.
*/
struct point_XYZ get_poly_min_disp_with_sphere(double r, struct point_XYZ* p, int num, struct point_XYZ n) {
    int i,j;
    /* double polydisp; */
    struct point_XYZ result;
	double tmin[3],tmax[3],rmin[3],rmax[3],q[3];
    double get_poly_mindisp;
    int clippedPoly4num = 0;
    get_poly_mindisp = 1E90;
	
	/* cheap MBB test */
	//double tmin[3],tmax[3],rmin[3],rmax[3],q[3];
	memcpy(tmin,&p[0],3*sizeof(double));
	memcpy(tmax,&p[3],3*sizeof(double));
	for(i=0;i<num;i++)
	{
		memcpy(q,&p[i],3*sizeof(double));
		for(j=0;j<3;j++)
		{
			tmin[j] = min(tmin[j],q[j]);
			tmax[j] = max(tmax[j],q[j]);
		}
	}
	for(i=0;i<3;i++)
	{
		rmin[i] = -r;
		rmax[i] = r;
	}
	if( !overlapMBBs(rmin,rmax,tmin,tmax) )
	{
		return zero;
	}
	/* end cheap MBB test */

#ifdef DEBUGFACEMASK
    if(facemask != debugsurface++)
	return zero;
#endif
    /*allocate data */
    if ((num+1)>clippedPoly4Size) {
    	clippedPoly4 = (struct point_XYZ*) REALLOC(clippedPoly4,sizeof(struct point_XYZ) * (num + 1));
		clippedPoly4Size = num+1;
    }

    /*if normal not specified, calculate it */
    /* if(n.x == 0 && n.y == 0 && n.z == 0) */
    if(APPROX(n.x, 0) && APPROX(n.y, 0) && APPROX(n.z, 0)) {
		polynormal(&n,&p[0],&p[1],&p[2]);
    }

    for(i = 0; i < num; i++) {
		DEBUGPTSPRINT("intersect_closestpolypoints_on_surface[%d]= %d\n",i,clippedPoly4num);
		clippedPoly4[clippedPoly4num++] = weighted_sum(p[i],p[(i+1)%num],closest_point_of_segment_to_origin(p[i],p[(i+1)%num]));
    }

    /*find closest point of polygon plane*/
    clippedPoly4[clippedPoly4num] = closest_point_of_plane_to_origin(p[0],n);

    /*keep if inside*/
    if(perpendicular_line_passing_inside_poly(clippedPoly4[clippedPoly4num],p, num)) {
		DEBUGPTSPRINT("perpendicular_line_passing_inside_poly[%d]= %d\n",0,clippedPoly4num);
		clippedPoly4num++;
	}


#ifdef DEBUGPTS
    for(i=0; i < clippedPoly4num; i++) {
	debugpts.push_back(clippedPoly4[i]);
    }
#endif

    /*here we find mimimum displacement possible */

    /*calculate the closest point to origin */
    for(i = 0; i < clippedPoly4num; i++) 
	{
		/* printf ("get_poly_min_disp_with_sphere, checking against %d %f %f %f",i,clippedPoly4[i].x, 
		clippedPoly4[i].y,clippedPoly4[i].z); */

		double disp = vecdot(&clippedPoly4[i],&clippedPoly4[i]);

		/* printf (" disp %lf, get_poly_mindisp %lf\n",disp,get_poly_mindisp); */

		if(disp < get_poly_mindisp) 
		{
			get_poly_mindisp = disp;
			result = clippedPoly4[i];
		}
    }
    if(get_poly_mindisp <= r*r) 
	{
		/*  scale result to length of missing distance. */
		double rl;
		rl = veclength(result);
		/* printf ("get_poly_min_disp_with_sphere, comparing %f and %f veclen %lf result %f %f %f\n",get_poly_mindisp, r*r, rl, result.x,result.y,result.z); */
		/* if(rl != 0.) */
		if(! APPROX(rl, 0)) 
		{
			/* printf ("approx rl, 0... scaling by %lf, %lf - %lf / %lf\n",(r-sqrt(get_poly_mindisp)) / rl,
				r, sqrt(get_poly_mindisp), rl); */
			vecscale(&result,&result,(r-sqrt(get_poly_mindisp)) / rl);
		} 
		else
		{
			result = zero;
		}
    }
    else
	{
		result = zero;
	}
    return result;
}


/*used by get_line_normal_disp to clip the polygon on the cylinder caps, called twice*/
int helper_line_clip_cap(struct point_XYZ* clippedpoly, int clippedpolynum, struct point_XYZ p1, struct point_XYZ p2, double r, struct point_XYZ n, double y, int stepping)
{
    struct point_XYZ ppoly[2];
    int allin = 1;
    int i;

    if(!stepping) {
	/*sqush poly on cylinder cap plane.*/
	ppoly[0] = project_on_yplane(p1,n,y);
	ppoly[1] = project_on_yplane(p2,n,y);
    } else {
	ppoly[0] = p1;
	ppoly[1] = p2;
    }

    /*find points of poly hitting cylinder cap*/
    for(i= 0; i < 2; i++) {
	if(ppoly[i].x*ppoly[i].x + ppoly[i].z*ppoly[i].z > r*r) {
	    allin = 0;
	} else {
	    clippedpoly[clippedpolynum++] = ppoly[i];
	}
    }

    if(!allin) {
	/* int numdessect = 0; */
	struct point_XYZ dessect;
	double k1,k2;
	int nsect;


	/*find intersections of line with cylinder cap edge*/
	nsect = getk_intersect_segment_with_ycylinder(&k1,&k2,r,ppoly[0],ppoly[1]);
	switch(nsect) {
	case 2:
	    if(fabs(k1-k2) < FLOAT_TOLERANCE) /* segment touches edge of circle. we want to ignore this. */
		break;
	    clippedpoly[clippedpolynum++] = weighted_sum(ppoly[0],ppoly[1],k2);
	case 1:
	    clippedpoly[clippedpolynum++] = weighted_sum(ppoly[0],ppoly[1],k1);
	case 0: break;
	}
	/*find intersections of descending segment too.
	  these will point out maximum and minimum in cylinder cap edge that is inside triangle */
	if(intersect_segment_with_line_on_yplane(&dessect,ppoly[0],ppoly[1],n,zero)) {
	    if(dessect.x*dessect.x + dessect.z*dessect.z < r*r) {

		clippedpoly[clippedpolynum++] = dessect;
	    }
	}
    }

    return clippedpolynum;
}

/*feed a line and a normal, and stats of a cylinder, it returns the displacement in the direction of the
  normal that is needed for them not to intersect any more.*/
struct point_XYZ get_line_normal_disp(double y1, double y2, double r, struct point_XYZ p1, struct point_XYZ p2, struct point_XYZ n) {
    int i;
    double mindisp = 0;
    double polydisp;
    struct point_XYZ p[2];
    int num = 2;
    struct point_XYZ result;

    struct point_XYZ clippedpoly[14];
    int clippedpolynum = 0;

    p[0] = p1;
    p[1] = p2;

    /* clip line on top and bottom cap */
    /* if(n.y!= 0.) */
    if(! APPROX(n.y, 0)) {
		clippedpolynum = helper_line_clip_cap(clippedpoly, clippedpolynum, p1, p2, r, n, y1, 0);
		clippedpolynum = helper_line_clip_cap(clippedpoly, clippedpolynum, p1, p2, r, n, y2, 0);
    }

    /*find intersections of line with cylinder side*/
    /* if(n.y != 1. && n.y != -1.) */ /*n.y == +/-1 makes n.x == n.z == 0, wich does div's by 0, besides making no sense at all. */
    if(! APPROX(n.y, 1) && ! APPROX(n.y, -1)) { /*n.y == +/-1 makes n.x == n.z == 0, wich does div's by 0, besides making no sense at all. */

	struct point_XYZ dessect3d;
	double k1,k2;
	int nsect;


	/*find points of poly intersecting descending line on poly, (non-projected)*/
	if(intersect_segment_with_line_on_yplane(&dessect3d,p[0],p[1],n,zero)) {
	    dessect3d = project_on_cylindersurface_plane(dessect3d,n,r);

	    if(dessect3d.y < y2 &&
	       dessect3d.y > y1)
		clippedpoly[clippedpolynum++] = dessect3d;

	}
	{ /*find intersections on cylinder of polygon points projected on surface */
	    struct point_XYZ sect[2];
	    for(i = 0; i < num; i++) {
		nsect = getk_intersect_line_with_ycylinder(&k1, &k2, r, p[i], n);
		if(nsect == 0) continue;

		/*sect = p[i] + k2 n*/
		vecscale(&sect[i],&n,k2);
		VECADD(sect[i],p[i]);

		if(sect[i].y > y1 && sect[i].y < y2) {
		    clippedpoly[clippedpolynum++] = sect[i];
		}
	    }
	    /*case where vertical line passes through cylinder, but no edges are inside */
	    /* if( (n.y == 0.) && ( */
	    if( (APPROX(n.y, 0)) && (
	       (sect[0].y <= y1 && sect[1].y >= y2) ||
	       (sect[1].y <= y1 && sect[0].y >= y2) )) {
		sect[0].y = (y1+y2)/2;
		    clippedpoly[clippedpolynum++] = sect[0];
	    }
	}

    }

#ifdef DEBUGPTS
    for(i=0; i < clippedpolynum; i++) {
	debugpts.push_back(clippedpoly[i]);
    }
#endif

    /*here we find mimimum displacement possible */
    polydisp = vecdot(&p[0],&n);

    /*calculate farthest point from the "n" plane passing through the origin */
    for(i = 0; i < clippedpolynum; i++) {
	double disp = vecdot(&clippedpoly[i],&n) - polydisp;
	if(disp < mindisp) mindisp = disp;
    }
    vecscale(&result,&n,mindisp);

    return result;
}

/*feed a line and a normal, and stats of a cylinder, it returns the vertical displacement
  that is needed for them not to intersect any more.*/
struct point_XYZ get_line_step_disp(double y1, double y2, double r, struct point_XYZ p1, struct point_XYZ p2, struct point_XYZ n) {
    int i;
    /* int allin = 1; */
    double dmax = -1E99;
    struct point_XYZ result;

    int clippedPoly5num = 0;

    get_poly_mindisp = 1E90;

#ifdef DEBUGFACEMASK
    printf("facemask = %d, debugsurface = %d\n",facemask,debugsurface);
    if((facemask & (1 <<debugsurface++)) ) return zero;
#endif

    if((p1.y > y2 || p2.y > y2 || n.y < 0) && n.y < STEPUP_MAXINCLINE) /*to high to step on and to steep to step on or facing downwards*/
	return zero;

    /*allocate data */
    if ((10)>clippedPoly5Size) {
        clippedPoly5 = (struct point_XYZ*) REALLOC(clippedPoly5,sizeof(struct point_XYZ) * (10));
        clippedPoly5Size = 10;
    }


    clippedPoly5num = helper_line_clip_cap(clippedPoly5, clippedPoly5num, p1, p2, r, n, y1,1 );

#ifdef DEBUGPTS
    for(i=0; i < clippedPoly5num; i++) {
	debugpts.push_back(clippedPoly5[i]);
    }
#endif

    /*get maximum*/
    for(i = 0; i < clippedPoly5num; i++) {
	if(clippedPoly5[i].y > dmax)
	    dmax = clippedPoly5[i].y;
    }

    /*diplace only if displacement completely clears line*/
    if(dmax > y2)
	return zero;

    get_poly_mindisp = y1-dmax;

    if(dmax > y1) {
	result.x = 0;
	result.y = get_poly_mindisp;
	result.z = 0;
	return result;
    } else
	return zero;

}

/*feed a line and a normal, and stats of a cylinder, it returns the displacement in the direction of the
  normal, or the vertical displacement(in case of stepping) that is needed for them not to intersect any more.*/
struct point_XYZ get_line_disp(double y1, double y2, double ystep, double r, struct point_XYZ p1, struct point_XYZ p2, struct point_XYZ n) {
    struct point_XYZ result;
    result = get_line_step_disp(y1,ystep,r,p1,p2,n);
    /* if(result.y != 0.) */
    if (! APPROX(result.y, 0)) {
		return result;
    } else {
		return get_line_normal_disp(y1,y2,r,p1,p2,n);
	}
}

/*feed a point and a normal, and stats of a cylinder, it returns the displacement in the direction of the
  normal that is needed for them not to intersect any more.*/
struct point_XYZ get_point_normal_disp(double y1, double y2, double r, struct point_XYZ p1, struct point_XYZ n) {
    return get_point_disp(y1,y2,y1,r,p1,n);
}

/*feed a point and a normal, and stats of a cylinder, it returns the displacement in the direction of the
  normal, or the vertical displacement(in case of stepping) that is needed for them not to intersect any more.*/
struct point_XYZ get_point_disp(double y1, double y2, double ystep, double r, struct point_XYZ p1, struct point_XYZ n) {
    double y;
    struct point_XYZ result = {0,0,0};
    struct point_XYZ cp;

    /*check if stepup.*/
    if((p1.y <= ystep && p1.y > y1 && p1.x*p1.x + p1.z*p1.z < r*r) && (n.y > STEPUP_MAXINCLINE) /*to steep to step on*/) {
	result.y = y1-p1.y;
	return result;
    }

    /*select relevant cap*/
    y = (n.y < 0.) ? y2 : y1;

    /*check if intersect cap*/
    /* if(n.y != 0) */
    if (! APPROX(n.y, 0)) {
	cp = project_on_yplane(p1,n,y);
	if(cp.x*cp.x + cp.z*cp.z < r*r) {
	    VECDIFF(cp,p1,result);
	    return result;
	}
    }

    /*find intersections of point with cylinder side*/
    /* if(n.y != 1. && n.y != -1.) */ /*n.y == +/-1 makes n.x == n.z == 0, wich does div's by 0, besides making no sense at all. */
    if (! APPROX(n.y,  1) && ! APPROX(n.y, -1)) { /*n.y == +/-1 makes n.x == n.z == 0, wich does div's by 0, besides making no sense at all. */
	int nsect;
	double k1,k2;
	/*find pos of the point projected on surface of cylinder*/
	nsect = getk_intersect_line_with_ycylinder(&k1, &k2, r, p1, n);
	if(nsect != 0) {
	    /*sect = p1 + k2 n*/
	    if(k2 >= 0) return zero; /* wrong direction. we are out. */
	    vecscale(&result,&n,k2);
	    cp = result;
	    VECADD(cp,p1);

	    if(cp.y > y1 && cp.y < y2) {
		return result;
	    }
	}

    }

    return zero;
}


/*feed a box (a corner, and the three vertice sides) and the stats of a cylinder, it returns the
  displacement of the box that is needed for them not to intersect any more, with optionnal stepping displacement */
struct point_XYZ box_disp(double y1, double y2, double ystep, double r,struct point_XYZ p0, struct point_XYZ i, struct point_XYZ j, struct point_XYZ k) {
    struct point_XYZ p[8];
    struct point_XYZ n[6];
    struct point_XYZ maxdispv = {0,0,0};
	double maxdisp = 0;
    //struct point_XYZ middle;
    /*draw this up, you will understand: */
    static const int faces[6][4] = {
	{1,7,2,0},
	{2,6,3,0},
	{3,5,1,0},
	{5,3,6,4},
	{7,1,5,4},
	{6,2,7,4}
    };
    int ci;

    for(ci = 0; ci < 8; ci++) p[ci] = p0;

    /*compute points of box*/
    VECADD(p[1],i);
    VECADD(p[2],j);
    VECADD(p[3],k);
    VECADD(p[4],i); VECADD(p[4],j); VECADD(p[4],k); /*p[4]= i+j+k */
    VECADD(p[5],k); VECADD(p[5],i); /*p[6]= k+i */
    VECADD(p[6],j); VECADD(p[6],k); /*p[5]= j+k */
    VECADD(p[7],i); VECADD(p[7],j); /*p[7]= i+j */

    /*compute normals, in case of perfectly orthogonal box, a shortcut exists*/
    veccross(&n[0],j,i);
    veccross(&n[1],k,j);
    veccross(&n[2],i,k);
    vecnormal(&n[0],&n[0]);
    vecnormal(&n[1],&n[1]);
    vecnormal(&n[2],&n[2]);
    vecscale(&n[3],&n[0],-1.);
    vecscale(&n[4],&n[1],-1.);
    vecscale(&n[5],&n[2],-1.);

    /*what it says : middle of box */
    //middle = weighted_sum(p[0],p[4],.5);

    for(ci = 0; ci < 6; ci++) {
	/*only clip faces "facing" origin */
	//if(vecdot(&n[ci],&middle) < 0.) {
	{
	    struct point_XYZ pts[5];
	    struct point_XYZ dispv;
	    double disp;
	    pts[0] = p[faces[ci][0]];
	    pts[1] = p[faces[ci][1]];
	    pts[2] = p[faces[ci][2]];
	    pts[3] = p[faces[ci][3]];
	    pts[4] = p[faces[ci][0]]; /* dug9 - for test split into 2 triangles for sphere test - no help with sphere*/
	    //dispv = get_poly_disp(y1,y2,ystep,r,pts,4,n[ci]);
		dispv = get_poly_disp_2(pts,4,n[ci]);
	    disp = vecdot(&dispv,&dispv);

		/*keep result only if:
		  displacement is positive
		  displacement is smaller than minimum displacement up to date 
		*/ 
		if( (disp > FLOAT_TOLERANCE) && (disp > maxdisp) ){
			maxdisp = disp;
			maxdispv = dispv;
	    }
    }
	}
    return maxdispv;

}

/*
 * fast test to see if a box intersects a y-cylinder,
 * gives false positives.
 */
int fast_ycylinder_box_intersect(double y1, double y2, double r,struct point_XYZ pcenter, double xs, double ys, double zs) {
    double y = pcenter.y < 0 ? y1 : y2;

    double lefteq = sqrt(y*y + r*r) + sqrt(xs*xs + ys*ys + zs*zs);
    return lefteq*lefteq > vecdot(&pcenter,&pcenter);
}
void transformMBB(GLDOUBLE *rMBBmin, GLDOUBLE *rMBBmax, GLDOUBLE *matTransform, GLDOUBLE* inMBBmin, GLDOUBLE* inMBBmax)
{
	/* transform axes aligned minimum bounding box MBB via octo box - will expand as necessary to cover original volume */
	struct point_XYZ abox[8];
	int i,j,k,m;
	GLDOUBLE p[3],rx,ry,rz;

	/* generate an 8 corner box in shape space to represent the shape collision volume */
	m = 0;
	for(i=0;i<2;i++)
	{
		rx = i==0? inMBBmin[0] : inMBBmax[0];
		for(j=0;j<2;j++)
		{
			ry = j==0? inMBBmin[1] : inMBBmax[1];
			for(k=0;k<2;k++)
			{
				rz = k==0? inMBBmin[2] : inMBBmax[2];
				abox[m].x = rx;
				abox[m].y = ry;
				abox[m].z = rz;
				m++;
			}
		}
	}

	/* transform the corners of the octo box  */
	for(m=0;m<8;m++)
       transform(&abox[m],&abox[m],matTransform);

	/*find the MBB of the transformed octo box */
	memcpy(rMBBmin,&abox[0],3*sizeof(GLDOUBLE)); //sizeof(struct point_XYZ)); 
	memcpy(rMBBmax,&abox[0],3*sizeof(GLDOUBLE));
	for(m=1;m<8;m++)
	{
		memcpy(p,&abox[m],3*sizeof(GLDOUBLE));
		for(i=0;i<3;i++)
		{
			rMBBmin[i] = min(rMBBmin[i],p[i]);
			rMBBmax[i] = max(rMBBmax[i],p[i]);
		}
	}
}
int fast_ycylinder_MBB_intersect_shapeSpace(double y1, double y2, double r, GLDOUBLE *collision2shape, GLDOUBLE *shapeMBBmin, GLDOUBLE *shapeMBBmax )
{ //foot,atop,awidth, Collision2Shape, node->size.c))
	/* goal: transform avatar collision volume from avatar space to raw shape space, get its enclosing 
	  axes-aligned minimum bounding box MBB in shape space, and test against raw shape MBB 
	  returns 1 if they intersect
	  Issue: caller must invert modelMatrix
	*/
	int i;
	GLDOUBLE avmin[3], avmax[3];

	/* generate mins and maxes for avatar cylinder in avatar/collision space to represent the avatar collision volume */
	for(i=0;i<3;i++)
	{
		avmin[i] = -r;
		avmax[i] =  r;
	}
	avmin[1] = y1; avmax[1] = y2;
	transformMBB(avmin,avmax,collision2shape,avmin,avmax);
	return overlapMBBs(shapeMBBmin,shapeMBBmax,avmin,avmax);
}
int fast_ycylinder_MBB_intersect_collisionSpace(double y1, double y2, double r, GLDOUBLE *shape2collision, GLDOUBLE *shapeMBBmin, GLDOUBLE *shapeMBBmax ) 
{
	/* goal: transform shape MBB to avatar/collision space, get its MBB there, and return 1 if the shape MBB intersects 
	  the avatar cylinderical collision volume MBB 
	 Issue: you'll likely have more false positives with the avatar-space MBB intersection test (versus shape-space)
	 shapes in shape space are statistically more often cuboid and axes-aligned - think of wall-like objects.
	 As a result a MBB in shape space is relatively smaller than the MBB recomputed after transforming to 
	 avatar space. (Same could be said in theory for the avatar cylinder going the other way, but when I think of wall
	 objects I'd rather be doing it in shape space. 
	*/
	int i;
	GLDOUBLE smin[3], smax[3], avmin[3], avmax[3];

	/* generate mins and maxes for avatar cylinder in avatar space to represent the avatar collision volume */
	for(i=0;i<3;i++)
	{
		avmin[i] = -r;
		avmax[i] =  r;
	}
	avmin[1] = y1; avmax[1] = y2;

	transformMBB(smin,smax,shape2collision,shapeMBBmin,shapeMBBmax);
	return overlapMBBs(avmin,avmax,smin,smax);
}
int fast_sphere_MBB_intersect_collisionSpace(double r, GLDOUBLE *shape2collision, GLDOUBLE *shapeMBBmin, GLDOUBLE *shapeMBBmax ) 
{
	/* goal: transform shape MBB to avatar/collision space, get its MBB there, and return 1 if the shape MBB intersects 
	  the avatar spherical collision volume MBB 
	*/
	int i;
	GLDOUBLE smin[3], smax[3], avmin[3], avmax[3];

	/* generate mins and maxes for avatar cylinder in avatar space to represent the avatar collision volume */
	for(i=0;i<3;i++)
	{
		avmin[i] = -r;
		avmax[i] =  r;
	}
	transformMBB(smin,smax,shape2collision,shapeMBBmin,shapeMBBmax);
	return overlapMBBs(avmin,avmax,smin,smax);
}
int fast_sphere_MBB_intersect_shapeSpace(double r, GLDOUBLE *collision2shape, GLDOUBLE *shapeMBBmin, GLDOUBLE *shapeMBBmax ) 
{
	/* goal: transform avatar/collision MBB to shape space, get its MBB there, and return 1 if the shape MBB intersects 
	  the avatar spherical collision volume MBB 
	*/
	int i;
	GLDOUBLE avmin[3], avmax[3];

	/* generate mins and maxes for avatar sphere in shape space to represent the avatar collision volume */
	for(i=0;i<3;i++)
	{
		avmin[i] = -r;
		avmax[i] =  r;
	}
	transformMBB(avmin,avmax,collision2shape,avmin,avmax);
	return overlapMBBs(avmin,avmax,shapeMBBmin,shapeMBBmax);
}

/*
 * fast test to see if a box intersects a y-cylinder,
 * gives false positives.

 * this code is adapted from the fast_ycylinder_cone_intersect code, below.
 */
int fast_ycylinder_polyrep_intersect(double y1, double y2, double AVr,struct point_XYZ pcenter, double scale, struct X3D_PolyRep *pr) {
	double AVy = pcenter.y < 0 ? y1 : y2;
	double rx, rz, myr;
	double myh;
	double lefteq;

	
	/*
	printf ("fast_ycylinder_polyrep, y %lf, r %lf, scale %lf, pcenter %lf %lf %lf\n",AVy,AVr,scale,pcenter.x,pcenter.y,pcenter.z);
	printf ("we have min/max for x and z: %lf, %lf and %lf %lf\n",pr->minVals[0],pr->maxVals[0], pr->minVals[2], pr->maxVals[2]);
	*/
	

	/* find the largest radius - use this for the cylinder radius */
	rx = (pr->maxVals[0]-pr->minVals[0])/2.0;
	myh = (pr->maxVals[1]-pr->minVals[1])/2.0*scale;
	rz = (pr->maxVals[2]-pr->minVals[2])/2.0;
	myr = sqrt (rx*rx + rz*rz)*scale;

	
	/* printf ("chose radius %lf from %lf, %lf passed in r is %lf\n",myr, rx,rz,AVr); */
	

	/* simplify - we know that (A + B)(A + B) = AA + AB + BA + BB */
	lefteq = sqrt(AVy*AVy + AVr*AVr) + sqrt(myh*myh + myr*myr); 

	/* if (lefteq*lefteq > vecdot(&pcenter,&pcenter)) printf ("returning TRUE\n"); else printf ("returing FALSE\n");  */
	

	return lefteq*lefteq > vecdot(&pcenter,&pcenter);
}
int fast_ycylinder_polyrep_intersect2(double y1, double y2, double AVr,struct point_XYZ pcenter, double scale, double *minVals, double *maxVals) {
	double AVy = pcenter.y < 0 ? y1 : y2;
	double rx, rz, myr;
	double myh;
	double lefteq;

	
	/*
	printf ("fast_ycylinder_polyrep, y %lf, r %lf, scale %lf, pcenter %lf %lf %lf\n",AVy,AVr,scale,pcenter.x,pcenter.y,pcenter.z);
	printf ("we have min/max for x and z: %lf, %lf and %lf %lf\n",pr->minVals[0],pr->maxVals[0], pr->minVals[2], pr->maxVals[2]);
	*/
	

	/* find the largest radius - use this for the cylinder radius */
	rx = (maxVals[0]-minVals[0])/2.0;
	myh = (maxVals[1]-minVals[1])/2.0*scale;
	rz = (maxVals[2]-minVals[2])/2.0;
	myr = sqrt (rx*rx + rz*rz)*scale;

	
	/* printf ("chose radius %lf from %lf, %lf passed in r is %lf\n",myr, rx,rz,AVr); */
	

	/* simplify - we know that (A + B)(A + B) = AA + AB + BA + BB */
	lefteq = sqrt(AVy*AVy + AVr*AVr) + sqrt(myh*myh + myr*myr); 

	/* if (lefteq*lefteq > vecdot(&pcenter,&pcenter)) printf ("returning TRUE\n"); else printf ("returing FALSE\n");  */
	

	return lefteq*lefteq > vecdot(&pcenter,&pcenter);
}

/*fast test to see if a cone intersects a y-cylinder. */
/*gives false positives. */
int fast_ycylinder_cone_intersect(double y1, double y2, double r,struct point_XYZ pcenter, double halfheight, double baseradius) {
    double y = pcenter.y < 0 ? y1 : y2;

    double lefteq = sqrt(y*y + r*r) + sqrt(halfheight*halfheight + baseradius*baseradius);
    return lefteq*lefteq > vecdot(&pcenter,&pcenter);
}

/* fast test to see if a sphere intersects a y-cylinder.
   specify sphere center, and a point on it's surface */
/*gives false positives. */
int fast_ycylinder_sphere_intersect(double y1, double y2, double r,struct point_XYZ pcenter, struct point_XYZ psurface) {
    double y = pcenter.y < 0 ? y1 : y2;
    double lefteq;

    VECDIFF(pcenter,psurface,psurface);

    lefteq = sqrt(y*y + r*r) + sqrt(psurface.x*psurface.x + psurface.y*psurface.y + psurface.z*psurface.z);
    return lefteq*lefteq > vecdot(&pcenter,&pcenter);
}



/*algorithm is approximative */
/*basically, it does collision with a triangle on a plane that passes through the origin.*/
struct point_XYZ cone_disp(double y1, double y2, double ystep, double r, struct point_XYZ base, struct point_XYZ top, double baseradius) {

    struct point_XYZ i; /* cone axis vector*/
    double h; /* height of cone*/
    struct point_XYZ tmp;
    struct point_XYZ bn; /* direction from cone to cylinder*/
    struct point_XYZ side; /* side of base in direction of origin*/
    struct point_XYZ normalbase; /* collision normal of base (points downwards)*/
    struct point_XYZ normalside; /* collision normal of side (points outside)*/
    struct point_XYZ normaltop; /* collision normal of top (points up)*/
    struct point_XYZ bn_normal; /* bn, normalized;*/
    struct point_XYZ mindispv= {0,0,0};
    double mindisp = 1E99;

    /*find closest point of cone base to origin. */

    vecscale(&bn,&base,-1.0);

    VECDIFF(top,base,i);
    vecscale(&tmp,&i,- vecdot(&i,&bn)/vecdot(&i,&i));
    VECADD(bn,tmp);
    /* if(vecnormal(&bn,&bn) == 0.) */
    if (APPROX(vecnormal(&bn,&bn), 0)) {
	/* origin is aligned with cone axis */
	/* must use different algorithm to find side */
	struct point_XYZ tmpn = i;
	struct point_XYZ tmpj;
	vecnormal(&tmpn,&tmpn);
	make_orthogonal_vector_space(&bn,&tmpj,tmpn);
	bn_normal = bn;
    }
    vecscale(&side,&bn,baseradius);
    VECADD(side,base);

    /* find normals ;*/
    h = vecnormal(&i,&i);
    normaltop = i;
    vecscale(&normalbase,&normaltop,-1.0);
    vecscale(&i,&i,-baseradius);
    vecscale(&normalside,&bn,-h);
    VECADD(normalside,i);
    vecnormal(&normalside,&normalside);
    vecscale(&normalside,&normalside,-1.0);

    {
	/*get minimal displacement*/
	struct point_XYZ dispv;
	double disp;

	if( vecdot(&normalside,&top) < 0. ) {
	    dispv = get_line_disp(y1,y2,ystep,r,top,side,normalside);
	    disp = vecdot(&dispv,&dispv);
	    if(disp < mindisp)
		mindispv = dispv, mindisp = disp;
	}

	if( vecdot(&normalbase,&base) < 0. ) {
	    dispv = get_line_disp(y1,y2,ystep,r,base,side,normalbase);
	    disp = vecdot(&dispv,&dispv);
	    if(disp < mindisp)
		mindispv = dispv, mindisp = disp;
	}

	if( vecdot(&normaltop,&top) < 0. ) {
	    dispv = get_point_disp(y1,y2,ystep,r,top,normaltop);
	    disp = vecdot(&dispv,&dispv);
	    /*i don't like "disp !=0." there should be a different condition for
	     * non applicability.*/
	    /* if(disp != 0. && disp < mindisp)  */
	    if(! APPROX(disp, 0) && disp < mindisp)
		mindispv = dispv, mindisp = disp;
	}
    }

    return mindispv;
}


/*algorithm is approximative */
/*basically, it does collision with a rectangle on a plane that passes through the origin.*/
struct point_XYZ cylinder_disp(double y1, double y2, double ystep, double r, struct point_XYZ base, struct point_XYZ top, double baseradius) {

    struct point_XYZ i; /* cone axis vector*/
    double h; /* height of cone*/
    struct point_XYZ tmp;
    struct point_XYZ bn; /* direction from cone to cylinder*/
    struct point_XYZ sidetop; /* side of top in direction of origin*/
    struct point_XYZ sidebase; /* side of base in direction of origin*/
    struct point_XYZ normalbase; /* collision normal of base (points downwards)*/
    struct point_XYZ normalside; /* collision normal of side (points outside)*/
    struct point_XYZ normaltop; /* collision normal of top (points upwards)*/
    struct point_XYZ mindispv= {0,0,0};
    double mindisp = 1E99;

    /*find closest point of cone base to origin. */

    vecscale(&bn,&base,-1.0);

    VECDIFF(top,base,i);
    vecscale(&tmp,&i,- vecdot(&i,&bn)/vecdot(&i,&i));
    VECADD(bn,tmp);
    /* if(vecnormal(&bn,&bn) == 0.) */
    if (APPROX(vecnormal(&bn,&bn), 0)) {
	/* origin is aligned with cone axis */
	/* must use different algorithm to find side */
	struct point_XYZ tmpn = i;
	struct point_XYZ tmpj;
	vecnormal(&tmpn,&tmpn);
	make_orthogonal_vector_space(&bn,&tmpj,tmpn);
    }
    vecscale(&sidebase,&bn,baseradius);
    sidetop = top; 
    VECADD(sidetop,sidebase);
    VECADD(sidebase,base);

    /* find normals ;*/
    h = vecnormal(&i,&i);
    normaltop = i;
    vecscale(&normalbase,&normaltop,-1.0);
    normalside = bn;

    {
	/*get minimal displacement*/
	struct point_XYZ dispv;
	double disp;

	if( vecdot(&normalside,&sidetop) < 0. ) {
	    dispv = get_line_disp(y1,y2,ystep,r,sidetop,sidebase,normalside);
	    disp = vecdot(&dispv,&dispv);
	    if(disp < mindisp)
		mindispv = dispv, mindisp = disp;
	}

	if( vecdot(&normalbase,&base) < 0. ) {
	    dispv = get_line_disp(y1,y2,ystep,r,base,sidebase,normalbase);
	    disp = vecdot(&dispv,&dispv);
	    if(disp < mindisp)
		mindispv = dispv, mindisp = disp;
	}

	if( vecdot(&normaltop,&top) < 0. ) {
	    dispv = get_line_disp(y1,y2,ystep,r,top,sidetop,normaltop);
	    disp = vecdot(&dispv,&dispv);
	    if( disp < mindisp)
		mindispv = dispv, mindisp = disp;
	}
    }

    return mindispv;
}
int intersectionHeightOfVerticalLineWithSurfaceElement(double* height, struct point_XYZ* p, int num, struct point_XYZ* n, double *tmin, double *tmax )
{

	/* 	Intersects a Y vertical infinite line passing through origin with a convex polygon  
	    convex polygon - like a triangle or quad or pentagon. Not like a star or general polygon like font glyph outline 
			etc which could be concave like a U
		Input: 
		p[num] - polygon vertices 
		n - polygon normal
		returns:
		0 if intersection fails due to either vertical polygon face or intesection point outside polygon
		height - if inside, this will be the Y height relative to the origin of the intersection point
	*/
	/* step 1 compute the infinite plane through the points p. Use the normal N passed in. */
	int overlap;
	struct point_XYZ D; 
	double dd, ndotD; 
	overlap = tmax[0] >= 0.0 && tmin[0] <= 0.0 && tmax[2] >= 0.0 && tmin[2] <= 0.0;
	if(!overlap) return 0;
	/* end cheap MBR test */
	dd = -vecdot(&p[0],n);
	/* the Origin is 0,0,0 and N*O is 0 */
	/* D is the ray direction - our zenith vector */
	D.x = 0.0;
	D.y = 1.0;
	D.z = 0.0;
	ndotD = vecdot(n,&D);
	/* slope check - if near vertical should we skip it?. */
	if( fabs(ndotD) < .1 )
	{
		return 0; /* vertical wall */
	}
	*height = - dd/ndotD;
	/* step 2 determine if inside the triangle */
	/* in theory if the cross products (D*height - p[i]) x (p[i+1] - p[i]) 
	   point in the same general direction, it's inside (if one alternates, its outside)*/
	D.y = *height;
	return pointOnPlaneInsidePoly(D,p,num,n);
}

void accumulateFallingClimbing(double y1, double y2, double ystep, struct point_XYZ *p, int num, struct point_XYZ nused, double *tmin, double *tmax)
{
	if(FallInfo.walking) 
	{
		/* Goal: Falling - if we're floating above the terrain we'll have no regular collision
		   so we need special test to see if we should fall. If climbing we nullify any fall.
	    */
		double hh;
		if( intersectionHeightOfVerticalLineWithSurfaceElement(&hh,&p[0],num,&nused, tmin, tmax))
		{
			/* terrain below avatar at 0,0,0? */
			double hhbelowy1 = hh - y1;
			if( hh < 0.0 )
			{
				/* falling */
				if( hh < y1 && hh > -FallInfo.fallHeight) 
				{
					/* FALLING */
					if(FallInfo.hits ==0)
						FallInfo.hfall = hhbelowy1; //hh - y1;
					else
						if(hhbelowy1 > FallInfo.hfall) FallInfo.hfall = hhbelowy1; //hh - y1;
					FallInfo.hits++;
					FallInfo.isFall = 1;
				}else
				/* regular below / nadir collision - below avatar center but above avatar's feet which are at 0.0 - avatar.height*/
				if( hh >= y1  ) /* && hh <= (y1-ystep) ) //no step height implementation */
				{
					/* CLIMBING. handled elsewhere for displacements, except annihilates any fall*/
					FallInfo.canFall = 0;

					if( FallInfo.isClimb == 0 )
						FallInfo.hclimb = hhbelowy1; //hh - y1;
					else
					    FallInfo.hclimb = max(FallInfo.hclimb,hhbelowy1);
					FallInfo.isClimb = 1;
				}
				/* no stepheight implementation here
				else
				if(hh > (y1-ystep) && hh < 0.0 )
				{
					// blocked by a high step, no climb 
					printf("S");
				}
				*/
					

			}else{
				/* regular above / zenith collision */
				if( hh > 0.0 && hh < y2 )
				{
					/* HEAD-BUMP handled elsewhere */
					if(0) printf("c");
				}
				else
					if(0) printf("B"); /* head is CLEAR of ceiling point */
			}
		}
	}
}

/*used by polyrep_disp 
  - coming in - all coords are already transformed into collision space
  - sets up avatar collision volume: a cyclinder for walking/stepping, a sphere for fly/examine
  - tests triangle by triangle for a collision
  - accumulates displacements from collisions
y1,y2,ystep,r (usually abottom,atop,astep,width from naviinfo height,step,width for the avatar) are in global scale coordinates
	pr.actualCoord[pr->ntri] 
	n[pr->ntri] - one normal for each triangle
dispsum.xyz - output - sum of collision displacement vectors - a mean will be computed by caller
flags - doublesided, front/back facing hints, no-stepping (?)
*/
//struct point_XYZ polyrep_disp_rec(double y1, double y2, double ystep, double r, struct X3D_PolyRep* pr, struct point_XYZ* n,  struct point_XYZ dispsum, prflags flags) {
struct point_XYZ polyrep_disp_rec2(struct X3D_PolyRep* pr, struct point_XYZ* n,  struct point_XYZ dispsum, prflags flags) {
    struct point_XYZ p[3];
    double maxdisp = 0;
    struct point_XYZ maxdispv = {0,0,0};
    double disp;
    struct point_XYZ dispv;
    int i;
    int frontfacing;
    int ccw;

    ccw = pr->ccw;
    for(i = 0; i < pr->ntri; i++) 
	{
		p[0].x = pr->actualCoord[pr->cindex[i*3]*3]    +dispsum.x;
		p[0].y = pr->actualCoord[pr->cindex[i*3]*3+1]  +dispsum.y;
		p[0].z = pr->actualCoord[pr->cindex[i*3]*3+2]  +dispsum.z;

		if (ccw) frontfacing = (vecdot(&n[i],&p[0]) < 0);	/*if normal facing avatar. avatar is at 0,0,0. If vector P going opposite direction to N, P*N will be negative */
		else frontfacing = (vecdot(&n[i],&p[0]) >= 0);		/*if ccw facing avatar */

		/* printf ("polyrep_disp_rec, frontfacing %d BACKFACING %d FRONTFACING %d DOUBLESIDED %d\n",
			frontfacing, flags & PR_BACKFACING, flags & PR_FRONTFACING, flags & PR_DOUBLESIDED); */
		/* use if either:
		   -frontfacing and not in doubleside mode;
		   -if in doubleside mode:
			   use if either:
			   -PR_FRONTFACING or PR_BACKFACING not yet specified
			   -fontfacing and PR_FRONTFACING specified
			   -not frontfacing and PR_BACKFACING specified 
		*/
		if(    (frontfacing && !(flags & PR_DOUBLESIDED) )
			|| ( (flags & PR_DOUBLESIDED)  && !(flags & (PR_FRONTFACING | PR_BACKFACING) )  )
			|| (frontfacing && (flags & PR_FRONTFACING))
			|| (!frontfacing && (flags & PR_BACKFACING))  ) 
		{

			struct point_XYZ nused;
			p[1].x = pr->actualCoord[pr->cindex[i*3+1]*3]    +dispsum.x;
			p[1].y = pr->actualCoord[pr->cindex[i*3+1]*3+1]  +dispsum.y;
			p[1].z = pr->actualCoord[pr->cindex[i*3+1]*3+2]  +dispsum.z;
			p[2].x = pr->actualCoord[pr->cindex[i*3+2]*3]    +dispsum.x;
			p[2].y = pr->actualCoord[pr->cindex[i*3+2]*3+1]  +dispsum.y;
			p[2].z = pr->actualCoord[pr->cindex[i*3+2]*3+2]  +dispsum.z;

			if(frontfacing) 
			{
				nused = n[i];
			} else { /*can only be here in DoubleSided mode*/
				/*reverse polygon orientation, and do calculations*/
				vecscale(&nused,&n[i],-1.0);
			}
			/* ready to start testing the triangle against our collision volume (sphere/cylinder/line) tests.
			 The avatar collision volume - 
			 A.for flying/examining:
			    1. sphere.

			 B.For walking:
			   1. sphere (head)  <<<<< problem I do not like this displacement logic when walking
					  because you'll end up floating over something that's just below avatar 0,0,0 
					  which for walking is wrong - you should collide with the cylinder going from top to bottom. 
					  Recommendation: either always do both sphere and cylinder or
			          just do #2 cylinder abottom to atop for head and body.
			   2. cylinder (body)
			   3. line - climb/fall (legs,feet)
			   Order of tests (so can exit early to save unnecessary computations):
			   sphere > cylinder > climb > fall
			   if you have a sphere hit you don't need to test the rest.
			   If you have a cylinder hit, you don't need to test climb/fall.
			   if you have a climb, you can skip the fall.
			   otherwise test for fall.

			   fast tests using MBB in the caller elliminate some volumes
			   docollision - sphere and cyclinder should be tested
			   dofall - climb and fall should be tested

			   Total logic for walking:
			   collision = docollision
			   if(docollision)
				do sphere
				if not sphere 
					do cylinder
					if not cylinder
						collision = 0
			   if( dofall && !collision )
				do line
				if not climb
					do fall
		   */
			dispv = get_poly_disp_2(p, 3, nused); //get_poly_disp_2(y1,y2,ystep,r, p, 3, nused);
			disp = vecdot(&dispv,&dispv);

	#ifdef DEBUGPTS
			if(dispv.x != 0 || dispv.y != 0 || dispv.z != 0)
			printf("polyd: (%f,%f,%f) |%f|\n",dispv.x,dispv.y,dispv.z,disp);
	#endif


			/*keep result only if:
			  displacement is positive
			  displacement is smaller than minimum displacement up to date 
			 */ 
			if( (disp > FLOAT_TOLERANCE) && (disp > maxdisp) ) 
			{
				maxdisp = disp;
				maxdispv = dispv;
				/* printf ("polyrep_disp_rec, maxdisp now %f, dispv %f %f %f\n",maxdisp,dispv.x, dispv.y, dispv.z); */
			}
		}
    }

	VECADD(dispsum,maxdispv);
	return dispsum;
}


static float* prd_newc_floats = NULL;
static int prd_newc_floats_size = 0;
static struct point_XYZ* prd_normals = NULL;
static int prd_normals_size = 0;

/*uses sphere displacement, and a cylinder for stepping 
 y1, y2, ystep, r -  (usually abottom, atop, astep, awidth) are from naviiinfo avatar height, step, width 
 pr - will be transformed by mat from raw shape coordinates into collision space: 
      Fly/Examine: avatar space
	  Walk: Bound-Viewpoint-Vertical-aligned Avatar-centric BVVA space.
 flags - 
*/
//struct point_XYZ polyrep_disp(double y1, double y2, double ystep, double r, struct X3D_PolyRep pr, GLDOUBLE* mat, prflags flags) {
struct point_XYZ polyrep_disp2(struct X3D_PolyRep pr, GLDOUBLE* mat, prflags flags) {
    int i;
    int maxc;


    res.x=0.0; res.y=0.0; res.z=0.0;
    maxc = 0; /*  highest cindex, used to point into prd_newc_floats structure.*/

    for(i = 0; i < pr.ntri*3; i++) {
	if (pr.cindex[i] > maxc) {maxc = pr.cindex[i];}
    }

    /*transform all points from raw shape to viewer(fly) or BVAAC(walk) space */
    if (maxc>prd_newc_floats_size) {
		prd_newc_floats = REALLOC(prd_newc_floats,maxc*9*sizeof(float));
		prd_newc_floats_size = maxc;
    }


    for(i = 0; i < pr.ntri*3; i++) {
	transformf(&prd_newc_floats[pr.cindex[i]*3],&pr.actualCoord[pr.cindex[i]*3],mat);
   }
   pr.actualCoord = prd_newc_floats; /*remember, coords are only replaced in our local copy of PolyRep */

 
    /*pre-calculate face normals */
    if (pr.ntri>prd_normals_size) {
		prd_normals = REALLOC(prd_normals,pr.ntri*sizeof(struct point_XYZ));
		prd_normals_size = pr.ntri;
    }

    for(i = 0; i < pr.ntri; i++) {
	polynormalf(&prd_normals[i],&pr.actualCoord[pr.cindex[i*3]*3],&pr.actualCoord[pr.cindex[i*3+1]*3],&pr.actualCoord[pr.cindex[i*3+2]*3]);
    }

	res = polyrep_disp_rec2(&pr,prd_normals,res,flags); //polyrep_disp_rec(y1,y2,ystep,r,&pr,prd_normals,res,flags);

    pr.actualCoord = 0;
    return res;
}


/*Optimized polyrep_disp for planar polyreps.
  Used for text.
  planar_polyrep_disp computes the normal using the first polygon, if no normal is specified (if it is zero).
  JAS - Normal is always specified now. (see VRMLRend.pm for invocation)
*/
struct point_XYZ planar_polyrep_disp_rec(double y1, double y2, double ystep, double r, struct X3D_PolyRep* pr, struct point_XYZ n, struct point_XYZ dispsum, prflags flags) {
    struct point_XYZ p[3];
    double lmaxdisp = 0;
    struct point_XYZ maxdispv = {0,0,0};
    double disp;
    struct point_XYZ dispv;
    /* static int recursion_count = 0; */
    int i;
    int frontfacing;

    p[0].x = pr->actualCoord[pr->cindex[0]*3]    +dispsum.x;
    p[0].y = pr->actualCoord[pr->cindex[0]*3+1]  +dispsum.y;
    p[0].z = pr->actualCoord[pr->cindex[0]*3+2]  +dispsum.z;

    frontfacing = (vecdot(&n,&p[0]) < 0);	/*if normal facing avatar */

    if(!frontfacing && !(flags & PR_DOUBLESIDED)) return dispsum;

    if(!frontfacing) vecscale(&n,&n,-1.0);

    for(i = 0; i < pr->ntri; i++) {
	p[0].x = pr->actualCoord[pr->cindex[i*3]*3]    +dispsum.x;
	p[0].y = pr->actualCoord[pr->cindex[i*3]*3+1]  +dispsum.y;
	p[0].z = pr->actualCoord[pr->cindex[i*3]*3+2]  +dispsum.z;
	p[1].x = pr->actualCoord[pr->cindex[i*3+1]*3]    +dispsum.x;
	p[1].y = pr->actualCoord[pr->cindex[i*3+1]*3+1]  +dispsum.y;
	p[1].z = pr->actualCoord[pr->cindex[i*3+1]*3+2]  +dispsum.z;
	p[2].x = pr->actualCoord[pr->cindex[i*3+2]*3]    +dispsum.x;
	p[2].y = pr->actualCoord[pr->cindex[i*3+2]*3+1]  +dispsum.y;
	p[2].z = pr->actualCoord[pr->cindex[i*3+2]*3+2]  +dispsum.z;

	//dispv = get_poly_disp(y1,y2,ystep, r, p, 3, n);
	dispv = get_poly_disp_2(p, 3, n);
	disp = -get_poly_mindisp; /*global variable. was calculated inside poly_normal_disp already. */

#ifdef DEBUGPTS
	printf("polyd: (%f,%f,%f) |%f|\n",dispv.x,dispv.y,dispv.z,disp);
#endif

	/*keep result only if:
	      displacement is positive
	      displacement is bigger than maximum displacement up to date
	*/
	if((disp > FLOAT_TOLERANCE) && (disp > lmaxdisp)) {
	    lmaxdisp = disp;
	    maxdispv = dispv;
	}

    }
    VECADD(dispsum,maxdispv);
    return dispsum;

}


struct point_XYZ planar_polyrep_disp(double y1, double y2, double ystep, double r, struct X3D_PolyRep pr, GLDOUBLE* mat, prflags flags, struct point_XYZ n) {
    int i;
    int maxc;


    res.x=0.0; res.y=0.0; res.z=0.0;
    maxc = 0; /*  highest cindex, used to point into newc structure.*/

    for(i = 0; i < pr.ntri*3; i++) {
	if (pr.cindex[i] > maxc) {maxc = pr.cindex[i];}
    }

    /*transform all points to viewer space */
    if (maxc>prd_newc_floats_size) {
		prd_newc_floats = REALLOC(prd_newc_floats,maxc*9*sizeof(float));
		prd_newc_floats_size = maxc;
    }

    for(i = 0; i < pr.ntri*3; i++) {
	transformf(&prd_newc_floats[pr.cindex[i]*3],&pr.actualCoord[pr.cindex[i]*3],mat);
    }
    pr.actualCoord = prd_newc_floats; /*remember, coords are only replaced in our local copy of PolyRep */

    /*if normal not speced, calculate it */
    /* if(n.x == 0 && n.y == 0 && n.z == 0.) */
    if(APPROX(n.x, 0) && APPROX(n.y, 0) && APPROX(n.z, 0)) {
	polynormalf(&n,&pr.actualCoord[pr.cindex[0]*3],&pr.actualCoord[pr.cindex[1]*3],&pr.actualCoord[pr.cindex[2]*3]);
    }

    res = planar_polyrep_disp_rec(y1,y2,ystep,r,&pr,n,res,flags);

    return res;
}






struct point_XYZ elevationgrid_disp( double y1, double y2, double ystep, double r, struct X3D_PolyRep pr,
			      int xdim, int zdim, double xs, double zs, GLDOUBLE* mat, prflags flags) {
    struct point_XYZ orig;
    int x1,x2,z1,z2; /*integer index bounds to elevation grid tests.*/
    double maxr = sqrt((y2-y1)*(y2-y1) + r*r); /*maximum radius of cylinder */
    struct point_XYZ dispf = {0,0,0};
    struct point_XYZ dispb = {0,0,0};
    double scale; /* inverse scale factor.*/
    GLDOUBLE invmat[16]; /* inverse transformation matrix*/
    double maxd2f = 0; /* maximum distance of polygon displacements, frontfacing (squared)*/
    double maxd2b = 0; /* maximum distance of polygon displacements, backfacing (squared)*/
    int dispcountf = 0; /* number of polygon displacements*/
    int dispcountb = 0; /* number of polygon displacements*/
    int x,z;
    struct point_XYZ tris[6]; /* two triangles*/
    float* newc; /* transformed coordinates.*/
    int frontfacing;

    /*essentially do an inverse transform of cylinder origin, and size*/
    /*FIXME: does not work right with non-unifrom scale*/
    matinverse(invmat,mat);
    orig.x = invmat[12];
    orig.y = invmat[13];
    orig.z = invmat[14];
    scale = 1/pow(det3x3(mat),1./3.);

    x1 = (int) ((orig.x - scale*maxr) / xs);
    x2 = (int) ((orig.x + scale*maxr) / xs) +1;
    z1 = (int) ((orig.z - scale*maxr) / zs);
    z2 = (int) ((orig.z + scale*maxr) / zs) +1;
    if(x1 < 0) x1 = 0;
    if(x2 >= xdim) x2 = xdim-1;
    if(x1 >= x2) return zero; /*  outside*/
    if(z1 < 0) z1 = 0;
    if(z2 >= zdim) z2 = zdim-1;
    if(z1 >= z2) return zero; /*  outside*/
    /* printf ("coll, xdim %d, zdim %d\n",xdim, zdim);*/

    if(!pr.cindex || !pr.actualCoord)
	printf("ZERO PTR! WE ARE DOOMED!\n");

    newc = MALLOC(float *, xdim*zdim*3*sizeof(float)); /* big chunk will be uninitialized.*/
    /*  transform points that will be used.*/
    for(z = z1; z <= z2; z++)
	for(x = x1; x <= x2; x++) {
	    transformf(&newc[(x+xdim*z)*3],&pr.actualCoord[(x+xdim*z)*3],mat);
	}
    pr.actualCoord = newc;

    /* changed to go to (z2-1) and (x2-1) from z2 and x2 - Apr 04 - JAS */
    for(z = z1; z < (z2-1); z++)
	for(x = x1; x < (x2-1); x++) {
	    int i;
	    struct point_XYZ pd;
		/* printf ("z %d z1 %d z2 %d x %d x1 %d x2 %d\n",z,z1,z2,x,x1,x2);*/
		/* printf ("ntri %d\n",pr.ntri);*/

		/* printf ("x %d z %d; x+(xdim-1)*z = %d\n",x,z,x+(xdim-1)*z);*/

	    for(i = 0; i < 3; i++) {
		    /*
		    printf ("i %d cindex %d\n",
				    (2*(x+(xdim-1)*z)+0)*3+i,
				    pr.cindex[(2*(x+(xdim-1)*z)+0)*3+i]);

		printf ("using coords %d %d %d %d %d %d\n",
		3*(2*(x+(xdim-1)*z)+0)+i + 0,
		3*(2*(x+(xdim-1)*z)+0)+i + 1,
		3*(2*(x+(xdim-1)*z)+0)+i + 2,
		3*(2*(x+(xdim-1)*z)+1)+i + 0,
		3*(2*(x+(xdim-1)*z)+1)+i + 1,
		3*(2*(x+(xdim-1)*z)+1)+i + 2);
		*/

		tris[i].x = pr.actualCoord[3*(2*(x+(xdim-1)*z)+0)+i + 0];
		tris[i].y = pr.actualCoord[3*(2*(x+(xdim-1)*z)+0)+i + 1];
		tris[i].z = pr.actualCoord[3*(2*(x+(xdim-1)*z)+0)+i + 2];
		tris[3+i].x = pr.actualCoord[3*(2*(x+(xdim-1)*z)+1)+i + 0];
		tris[3+i].y = pr.actualCoord[3*(2*(x+(xdim-1)*z)+1)+i + 1];
		tris[3+i].z = pr.actualCoord[3*(2*(x+(xdim-1)*z)+1)+i + 2];
	    }

	    for(i = 0; i < 2; i++) { /* repeat for both triangles*/
		struct point_XYZ normal;
		polynormal(&normal,&tris[0+i*3],&tris[1+i*3],&tris[2+i*3]);
		frontfacing = (vecdot(&normal,&tris[0+i*3]) < 0);	/*if normal facing avatar */

		if((flags & PR_DOUBLESIDED) || frontfacing) {
		    if(!frontfacing) vecscale(&normal,&normal,-1.0);

		    /* pd = get_poly_disp(y1,y2,ystep,r, tris+(i*3), 3, normal); */
		    pd = get_poly_disp_2(tris+(i*3), 3, normal);
		    /* if(pd.x != 0. || pd.y != 0. || pd.z != 0.) */
		    if(! APPROX(pd.x, 0) || ! APPROX(pd.y, 0) || ! APPROX(pd.z, 0)) {
			double l2;
			if(frontfacing) {
			    dispcountf++;
			    VECADD(dispf,pd);
			    if((l2 = vecdot(&pd,&pd)) > maxd2f)
				maxd2f = l2;
			} else {
			    dispcountb++;
			    VECADD(dispb,pd);
			    if((l2 = vecdot(&pd,&pd)) > maxd2b)
				maxd2b = l2;
			}

		    }
		}

	    }


	}

    FREE_IF_NZ (newc);

    /*check wether we should frontface, or backface.
     */
    frontfacing = (dispcountf > dispcountb);
    if(dispcountf == dispcountb)
	frontfacing = (maxd2f < maxd2b);

    if(frontfacing) {
	if(dispcountf == 0)
	    return zero;
	/* if(vecnormal(&dispf,&dispf) == 0.)  */
	if(APPROX(vecnormal(&dispf,&dispf), 0))
	    return zero;
	vecscale(&dispf,&dispf,sqrt(maxd2f));

	return dispf;
    } else {
	if(dispcountb == 0)
	    return zero;
	/* if(vecnormal(&dispb,&dispb) == 0.)  */
	if(APPROX(vecnormal(&dispb,&dispb), 0))
	    return zero;
	vecscale(&dispb,&dispb,sqrt(maxd2b));

	return dispb;
    }

}






#ifdef DEBUG_SCENE_EXPORT
void printpolyrep(struct X3D_PolyRep pr) {
    int i;
    int npoints = 0;
    printf("X3D_PolyRep makepolyrep() {\n");
    printf(" int cindext[%d] = {",pr.ntri*3);
    for(i=0; i < pr.ntri*3-1; i++) {
	printf("%d,",pr.cindex[i]);
	if(pr.cindex[i] > npoints)
	    npoints = pr.cindex[i];
    }
    printf("%d};\n",pr.cindex[i]);
    if(pr.cindex[i] > npoints)
	npoints = pr.cindex[i];

    printf(" float coordt[%d] = {",npoints*3);
    for(i=0; i < npoints*3-1; i++)
	printf("%f,",pr.actualCoord[i]);
    printf("%f};\n",pr.actualCoord[i]);

    printf("static int cindex[%d];\nstatic float coord[%d];\n",pr.ntri*3,npoints*3);
    printf("X3D_PolyRep pr = {0,%d,%d,cindex,coord,NULL,NULL,NULL,NULL,NULL,NULL};\n",pr.ntri,pr.alloc_tri);
    printf("memcpy(cindex,cindext,sizeof(cindex));\n");
    printf("memcpy(coord,coordt,sizeof(coord));\n");
    printf("return pr; }\n");

};

void printmatrix(GLDOUBLE* mat) {
    int i;
    printf("void getmatrix(GLDOUBLE* mat, struct point_XYZ disp) {\n");
    for(i = 0; i< 16; i++) {
	printf("mat[%d] = %f%s;\n",i,mat[i],i==12 ? " +disp.x" : i==13? " +disp.y" : i==14? " +disp.z" : "");
    }
    printf("}\n");

}
#endif

