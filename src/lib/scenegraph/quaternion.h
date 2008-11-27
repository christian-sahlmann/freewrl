/*
=INSERT_TEMPLATE_HERE=

$Id: quaternion.h,v 1.2 2008/11/27 00:27:18 couannette Exp $

Quaternion ???

*/

#ifndef __FREEX3D_QUATERNION_H__
#define __FREEX3D_QUATERNION_H__


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


#endif /* __FREEX3D_QUATERNION_H__ */
