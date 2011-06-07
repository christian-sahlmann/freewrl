/*
=INSERT_TEMPLATE_HERE=

$Id: Polyrep.h,v 1.14 2011/06/07 20:00:59 dug9 Exp $

Polyrep ???

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
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "LinearAlgebra.h"


/* transformed ray */
//extern struct point_XYZ t_r1;
//extern struct point_XYZ t_r2;
//extern struct point_XYZ t_r3;



int count_IFS_faces(int cin, struct Multi_Int32 *coordIndex);

int 
IFS_face_normals(struct point_XYZ *facenormals,
				 int *faceok,
				 int *pointfaces,
				 int faces,
				 int npoints,
				 int cin,
				 struct SFVec3f *points,
				 struct Multi_Int32 *coordIndex,
				 int ccw);

void
IFS_check_normal(struct point_XYZ *facenormals,
				 int this_face,
				 struct SFVec3f *points,
				 int base,
				 struct Multi_Int32 *coordIndex,
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
		  GLuint *tcindex,
		  int ccw,
		  int tcindexsize);

void Extru_ST_map(
        int triind_start,
        int start,
        int end,
        float *Vals,
        int nsec,
        GLuint *tcindex,
        GLuint *cindex,
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
do_glNormal3fv(struct SFVec3f *dest, GLfloat *param);

void stream_polyrep(void *node, void *coord, void *color, void *normal, void *texCoord);
void compile_polyrep(void *node, void *coord, void *color, void *normal, void *texCoord);

