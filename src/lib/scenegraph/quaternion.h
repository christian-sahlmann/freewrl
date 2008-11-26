/******************************************************************************
 Copyright (C) 1998 Tuomas J. Lukka, 2003 John Stewart, Ayla Khan, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*******************************************************************************/

/*
 * $Id: quaternion.h,v 1.1 2008/11/26 11:24:14 couannette Exp $
 *
 */

#ifndef __QUATERNION_H__
#define __QUATERNION_H__


#include "headers.h"

#ifndef AQUA
#include <GL/gl.h>
#else
#include <gl.h>
#endif
#include <math.h>

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

#endif /* __QUATERNION_H__ */
