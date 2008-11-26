/* $Id: LinearAlgebra.c,v 1.1 2008/11/26 11:24:14 couannette Exp $
 *
 * Copyright (C) 2002 Nicolas Coderre CRC Canada
 * Copyright (C) 2003 John Stewart CRC Canada
 * Portions Copyright (C) 1998 Tuomas J. Lukka 1998 Bernhard Reiter 1999 John Stewart CRC Canada
 * DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 * See the GNU Library General Public License (file COPYING in the distribution)
 * for conditions of use and redistribution.
 */
#include "headers.h"
#include "LinearAlgebra.h"


#include <stdio.h>
#include <memory.h>
/* Altenate implemetations available, should merge them eventually */

void veccross(struct point_XYZ *c, struct point_XYZ a, struct point_XYZ b)
{
    c->x = a.y * b.z - a.z * b.y;
    c->y = a.z * b.x - a.x * b.z;
    c->z = a.x * b.y - a.y * b.x;
}

float veclength( struct point_XYZ p )
{
    return sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
}


double vecangle(struct point_XYZ* V1, struct point_XYZ* V2) {
    return acos((V1->x*V2->x + V1->y*V2->y +V1->z*V2->z) /
		sqrt( (V1->x*V1->x + V1->y*V1->y + V1->z*V1->z)*(V2->x*V2->x + V2->y*V2->y + V2->z*V2->z) )  );
};


float calc_angle_between_two_vectors(struct point_XYZ a, struct point_XYZ b)
{
    float length_a, length_b, scalar, temp;
    scalar = calc_vector_scalar_product(a,b);
    length_a = calc_vector_length(a);
    length_b = calc_vector_length(b);

    /* printf("scalar: %f  length_a: %f  length_b: %f \n", scalar, length_a, length_b);*/

    /* if (scalar == 0) */
    if (APPROX(scalar, 0)) {
		return M_PI/2;
    }

    if ( (length_a <= 0)  || (length_b <= 0)){
	printf("Divide by 0 in calc_angle_between_two_vectors():  No can do! \n");
	return 0;
    }

    temp = scalar /(length_a * length_b);
    /*  printf("temp: %f", temp);*/

    /*acos() appears to be unable to handle 1 and -1  */
    /* fixed to handle border case where temp <=-1.0 for 0.39 JAS */
    if ((temp >= 1) || (temp <= -1)){
	if (temp < 0.0) return 3.141526;
	return 0.0;
    }
    return acos(temp);
}

/* returns vector length, too */
GLdouble vecnormal(struct point_XYZ*r, struct point_XYZ* v)
{
    GLdouble ret = sqrt(vecdot(v,v));
    /* if(ret == 0.) return 0.; */
    if (APPROX(ret, 0)) return 0.;
    vecscale(r,v,1./ret);
    return ret;
}


/*will add functions here as needed. */

GLdouble det3x3(GLdouble* data)
{
    return -data[1]*data[10]*data[4] +data[0]*data[10]*data[5] -data[2]*data[5]*data[8] +data[1]*data[6]*data[8] +data[2]*data[4]*data[9] -data[0]*data[6]*data[9];
}

struct point_XYZ* transform(struct point_XYZ* r, const struct point_XYZ* a, const GLdouble* b)
{
    struct point_XYZ tmp; /* JAS*/

    if(r != a) { /*protect from self-assignments */
	r->x = b[0]*a->x +b[4]*a->y +b[8]*a->z +b[12];
	r->y = b[1]*a->x +b[5]*a->y +b[9]*a->z +b[13];
	r->z = b[2]*a->x +b[6]*a->y +b[10]*a->z +b[14];
    } else {
	/* JAS was  - struct point_XYZ tmp = {a->x,a->y,a->z};*/
	tmp.x = a->x; tmp.y = a->y; tmp.z = a->z;
	r->x = b[0]*tmp.x +b[4]*tmp.y +b[8]*tmp.z +b[12];
	r->y = b[1]*tmp.x +b[5]*tmp.y +b[9]*tmp.z +b[13];
	r->z = b[2]*tmp.x +b[6]*tmp.y +b[10]*tmp.z +b[14];
    }
    return r;
}

float* transformf(float* r, const float* a, const GLdouble* b)
{
    float tmp[3];  /* JAS*/

    if(r != a) { /*protect from self-assignments */
	r[0] = b[0]*a[0] +b[4]*a[1] +b[8]*a[2] +b[12];
	r[1] = b[1]*a[0] +b[5]*a[1] +b[9]*a[2] +b[13];
	r[2] = b[2]*a[0] +b[6]*a[1] +b[10]*a[2] +b[14];
    } else {
	tmp[0] =a[0]; tmp[1] = a[1]; tmp[2] = a[2]; /* JAS*/
	r[0] = b[0]*tmp[0] +b[4]*tmp[1] +b[8]*tmp[2] +b[12];
	r[1] = b[1]*tmp[0] +b[5]*tmp[1] +b[9]*tmp[2] +b[13];
	r[2] = b[2]*tmp[0] +b[6]*tmp[1] +b[10]*tmp[2] +b[14];
    }
    return r;
}
/*transform point, but ignores translation.*/
struct point_XYZ* transform3x3(struct point_XYZ* r, const struct point_XYZ* a, const GLdouble* b)
{
    struct point_XYZ tmp;

    if(r != a) { /*protect from self-assignments */
	r->x = b[0]*a->x +b[4]*a->y +b[8]*a->z;
	r->y = b[1]*a->x +b[5]*a->y +b[9]*a->z;
	r->z = b[2]*a->x +b[6]*a->y +b[10]*a->z;
    } else {
	/* JAS struct point_XYZ tmp = {a->x,a->y,a->z};*/
	tmp.x = a->x; tmp.y = a->y; tmp.z = a->z;
	r->x = b[0]*tmp.x +b[4]*tmp.y +b[8]*tmp.z;
	r->y = b[1]*tmp.x +b[5]*tmp.y +b[9]*tmp.z;
	r->z = b[2]*tmp.x +b[6]*tmp.y +b[10]*tmp.z;
    }
    return r;
}

struct point_XYZ* vecscale(struct point_XYZ* r, struct point_XYZ* v, GLdouble s)
{
    r->x = v->x * s;
    r->y = v->y * s;
    r->z = v->z * s;
    return r;
}

double vecdot(struct point_XYZ* a, struct point_XYZ* b)
{
    return (a->x*b->x) + (a->y*b->y) + (a->z*b->z);
}


/* returns 0 if p1 is closest, 1 if p2 is closest, and a fraction if the closest point is in between */
/* To get the closest point, use pclose = retval*p1 + (1-retval)p2; */
/* y1 must be smaller than y2 */
/*double closest_point_of_segment_to_y_axis_segment(double y1, double y2, struct point_XYZ p1, struct point_XYZ p2) {
  double imin = (y1- p1.y) / (p2.y - p1.y);
  double imax = (y2- p1.y) / (p2.y - p1.y);

  double x21 = (p2.x - p1.x);
  double z21 = (p2.z - p1.z);
  double i = (p2.x * x21 + p2.z * z21) /
  ( x21*x21 + z21*z21 );
  return max(min(i,imax),imin);

  }*/

double closest_point_of_segment_to_y_axis_segment(double y1, double y2, struct point_XYZ p1, struct point_XYZ p2) {
    /*cylinder constraints (to be between y1 and y2) */
    double imin = (y1- p1.y) / (p2.y - p1.y);
    double imax = (y2- p1.y) / (p2.y - p1.y);

    /*the equation */
    double x12 = (p1.x - p2.x);
    double z12 = (p1.z - p2.z);
    double q = ( x12*x12 + z12*z12 );

    /* double i = ((q == 0.) ? 0 : (p1.x * x12 + p1.z * z12) / q); */
    double i = ((APPROX(q, 0)) ? 0 : (p1.x * x12 + p1.z * z12) / q);

    printf("imin=%f, imax=%f => ",imin,imax);

    if(imin > imax) {
	double tmp = imax;
	imax = imin;
	imin = tmp;
    }

    /*clamp constraints to segment*/
    if(imin < 0) imin = 0;
    if(imax > 1) imax = 1;

    printf("imin=%f, imax=%f\n",imin,imax);

    /*clamp result to constraints */
    if(i < imin) i = imin;
    if(i > imax) i = imax;
    return i;

}

struct point_XYZ* vecadd(struct point_XYZ* r, struct point_XYZ* v, struct point_XYZ* v2)
{
    r->x = v->x + v2->x;
    r->y = v->y + v2->y;
    r->z = v->z + v2->z;
    return r;
}

struct point_XYZ* vecdiff(struct point_XYZ* r, struct point_XYZ* v, struct point_XYZ* v2)
{
    r->x = v->x - v2->x;
    r->y = v->y - v2->y;
    r->z = v->z - v2->z;
    return r;
}

/*i,j,n will form an orthogonal vector space */
void make_orthogonal_vector_space(struct point_XYZ* i, struct point_XYZ* j, struct point_XYZ n) {
    /* optimal axis finding algorithm. the solution isn't unique.*/
    /*  each of these three calculations doesn't work (or works poorly)*/
    /*  in certain distinct cases. (gives zero vectors when two axes are 0)*/
    /*  selecting the calculations according to smallest axis avoids this problem.*/
    /*  (the two remaining axis are thus far from zero, if n is normal)*/
    if(fabs(n.x) <= fabs(n.y) && fabs(n.x) <= fabs(n.z)) { /* x smallest*/
	i->x = 0;
	i->y = n.z;
	i->z = -n.y;
	vecnormal(i,i);
	j->x = n.y*n.y + n.z*n.z;
	j->y = (-n.x)*n.y;
	j->z = (-n.x)*n.z;
    } else if(fabs(n.y) <= fabs(n.x) && fabs(n.y) <= fabs(n.z)) { /* y smallest*/
	i->x = -n.z;
	i->y = 0;
	i->z = n.x;
	vecnormal(i,i);
	j->x = (-n.x)*n.y;
	j->y = n.x*n.x + n.z*n.z;
	j->z = (-n.y)*n.z;
    } else { /* z smallest*/
	i->x = n.y;
	i->y = -n.x;
	i->z = 0;
	vecnormal(i,i);
	j->x = (-n.x)*n.z;
	j->y = (-n.y)*n.z;
	j->z =  n.x*n.x + n.y*n.y;
    }
}


GLdouble* matinverse(GLdouble* res, GLdouble* m)
{
    double Deta;
    GLdouble mcpy[16];

    if(res == m) {
	memcpy(mcpy,m,sizeof(GLdouble)*16);
	m = mcpy;
    }

    Deta = det3x3(m);

    res[0] = (-m[9]*m[6] +m[5]*m[10])/Deta;
    res[4] = (+m[8]*m[6] -m[4]*m[10])/Deta;
    res[8] = (-m[8]*m[5] +m[4]*m[9])/Deta;
    res[12] = ( m[12]*m[9]*m[6] -m[8]*m[13]*m[6] -m[12]*m[5]*m[10] +m[4]*m[13]*m[10] +m[8]*m[5]*m[14] -m[4]*m[9]*m[14])/Deta;

    res[1] = (+m[9]*m[2] -m[1]*m[10])/Deta;
    res[5] = (-m[8]*m[2] +m[0]*m[10])/Deta;
    res[9] = (+m[8]*m[1] -m[0]*m[9])/Deta;
    res[13] = (-m[12]*m[9]*m[2] +m[8]*m[13]*m[2] +m[12]*m[1]*m[10] -m[0]*m[13]*m[10] -m[8]*m[1]*m[14] +m[0]*m[9]*m[14])/Deta;

    res[2] = (-m[5]*m[2] +m[1]*m[6])/Deta;
    res[6] = (+m[4]*m[2] -m[0]*m[6])/Deta;
    res[10] = (-m[4]*m[1] +m[0]*m[5])/Deta;
    res[14] = ( m[12]*m[5]*m[2] -m[4]*m[13]*m[2] -m[12]*m[1]*m[6] +m[0]*m[13]*m[6] +m[4]*m[1]*m[14] -m[0]*m[5]*m[14])/Deta;

    res[3] = (+m[5]*m[2]*m[11] -m[1]*m[6]*m[11])/Deta;
    res[7] = (-m[4]*m[2]*m[11] +m[0]*m[6]*m[11])/Deta;
    res[11] = (+m[4]*m[1]*m[11] -m[0]*m[5]*m[11])/Deta;
    res[15] = (-m[8]*m[5]*m[2] +m[4]*m[9]*m[2] +m[8]*m[1]*m[6] -m[0]*m[9]*m[6] -m[4]*m[1]*m[10] +m[0]*m[5]*m[10])/Deta;

    return res;
}

struct point_XYZ* polynormal(struct point_XYZ* r, struct point_XYZ* p1, struct point_XYZ* p2, struct point_XYZ* p3) {
    struct point_XYZ v1;
    struct point_XYZ v2;
    VECDIFF(*p2,*p1,v1);
    VECDIFF(*p3,*p1,v2);
    veccross(r,v1,v2);
    vecnormal(r,r);
    return r;
}

/*simple wrapper for now. optimize later */
struct point_XYZ* polynormalf(struct point_XYZ* r, float* p1, float* p2, float* p3) {
    struct point_XYZ pp[3];
    pp[0].x = p1[0];
    pp[0].y = p1[1];
    pp[0].z = p1[2];
    pp[1].x = p2[0];
    pp[1].y = p2[1];
    pp[1].z = p2[2];
    pp[2].x = p3[0];
    pp[2].y = p3[1];
    pp[2].z = p3[2];
    return polynormal(r,pp+0,pp+1,pp+2);
}


GLdouble* matrotate(GLdouble* Result, double Theta, double x, double y, double z)
{
    GLdouble CosTheta = cos(Theta);
    GLdouble SinTheta = sin(Theta);

    Result[0] = x*x + (y*y+z*z)*CosTheta;
    Result[1] = x*y - x*y*CosTheta + z*SinTheta;
    Result[2] = x*z - x*z*CosTheta - y*SinTheta;
    Result[4] = x*y - x*y*CosTheta - z*SinTheta;
    Result[5] = y*y + (x*x+z*z)*CosTheta;
    Result[6] = z*y - z*y*CosTheta + x*SinTheta;
    Result[8] = z*x - z*x*CosTheta + y*SinTheta;
    Result[9] = z*y - z*y*CosTheta - x*SinTheta;
    Result[10]= z*z + (x*x+y*y)*CosTheta;
    Result[3] = 0;
    Result[7] = 0;
    Result[11] = 0;
    Result[12] = 0;
    Result[13] = 0;
    Result[14] = 0;
    Result[15] = 1;

    return Result;
}

GLdouble* mattranslate(GLdouble* r, double dx, double dy, double dz)
{
    r[0] = r[5] = r[10] = r[15] = 1;
    r[1] = r[2] = r[3] = r[4] =
	r[6] = r[7] = r[8] = r[9] =
	r[11] = 0;
    r[12] = dx;
    r[13] = dy;
    r[14] = dz;
    return r;
}

GLdouble* matmultiply(GLdouble* r, GLdouble* m , GLdouble* n)
{
    GLdouble tm[16],tn[16];
    /* prevent self-multiplication problems.*/
    if(r == m) {
	memcpy(tm,m,sizeof(GLdouble)*16);
	m = tm;
    }
    if(r == n) {
	memcpy(tn,n,sizeof(GLdouble)*16);
	n = tn;
    }
    r[0] = m[0]*n[0]+m[4]*n[1]+m[8]*n[2];
    r[4] = m[0]*n[4]+m[4]*n[5]+m[8]*n[6];
    r[8] = m[0]*n[8]+m[4]*n[9]+m[8]*n[10];
    r[12]= m[0]*n[12]+m[4]*n[13]+m[8]*n[14]+m[12];

    r[1] = m[1]*n[0]+m[5]*n[1]+m[9]*n[2];
    r[5] = m[1]*n[4]+m[5]*n[5]+m[9]*n[6];
    r[9] = m[1]*n[8]+m[5]*n[9]+m[9]*n[10];
    r[13]= m[1]*n[12]+m[5]*n[13]+m[9]*n[14]+m[13];

    r[2] = m[2]*n[0]+m[6]*n[1]+m[10]*n[2];
    r[6] = m[2]*n[4]+m[6]*n[5]+m[10]*n[6];
    r[10]= m[2]*n[8]+m[6]*n[9]+m[10]*n[10];
    r[14]= m[2]*n[12]+m[6]*n[13]+m[10]*n[14]+m[14];

    return r;
}

/*puts dv back on iv*/
double matrotate2v(GLdouble* res, struct point_XYZ iv/*original*/, struct point_XYZ dv/*result*/) {
    struct point_XYZ cv;
    double cvl,a;

    vecnormal(&dv,&dv);
    vecnormal(&iv,&iv);

    veccross(&cv,dv,iv); /*the axis of rotation*/
    cvl = vecnormal(&cv,&cv);
    /* if(cvl == 0) { */
    if(APPROX(cvl, 0)) {
		cv.z = 1;
	}

    a = atan2(cvl,vecdot(&dv,&iv)); /*the angle*/

    matrotate(res,a,cv.x,cv.y,cv.z);
    return a;
}




#ifdef COMMENT
/*fast crossproduct using references, that checks for auto-assignments */
GLdouble* veccross(GLdouble* r, GLdouble* v1, GLdouble* v2)
{
    /*check against self-assignment. */
    if(r != v1) {
	if(r != v2) {
	    r[0] = v1[1]*v2[2] - v1[2]*v2[1];
	    r[1] = v1[2]*v2[0] - v1[0]*v2[2];
	    r[2] = v1[0]*v2[1] - v1[1]*v2[0];
	} else { /* r == v2 */
	    GLdouble v2c[3] = {v2[0],v2[1],v2[2]};
	    r[0] = v1[1]*v2c[2] - v1[2]*v2c[1];
	    r[1] = v1[2]*v2c[0] - v1[0]*v2c[2];
	    r[2] = v1[0]*v2c[1] - v1[1]*v2c[0];
	}
    } else { /* r == v1 */
	GLdouble v1c[3] = {v1[0],v1[1],v1[2]};
	r[0] = v1c[1]*v2[2] - v1c[2]*v2[1];
	r[1] = v1c[2]*v2[0] - v1c[0]*v2[2];
	r[2] = v1c[0]*v2[1] - v1c[1]*v2[0];
    }
    return r;
}



#endif
