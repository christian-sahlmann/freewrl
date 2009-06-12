/*
=INSERT_TEMPLATE_HERE=

$Id: Polyrep.h,v 1.5 2009/06/12 20:13:00 crc_canada Exp $

Polyrep ???

*/

#include <config.h>
/* JAS #include <system.h> */
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "LinearAlgebra.h"


/* transformed ray */
extern struct point_XYZ t_r1;
extern struct point_XYZ t_r2;
extern struct point_XYZ t_r3;



int count_IFS_faces(int cin, struct Multi_Int32 *coordIndex);

int 
IFS_face_normals(struct point_XYZ *facenormals,
				 int *faceok,
				 int *pointfaces,
				 int faces,
				 int npoints,
				 int cin,
				 struct SFColor *points,
				 struct Multi_Int32 *coordIndex,
				 int ccw);

void
IFS_check_normal(struct point_XYZ *facenormals,
				 int this_face,
				 struct SFColor *points,
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
		  int *tcindex,
		  int ccw,
		  int tcindexsize);

void Extru_ST_map(
        int triind_start,
        int start,
        int end,
        float *Vals,
        int nsec,
        int *tcindex,
        int *cindex,
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
do_glNormal3fv(struct SFColor *dest, GLfloat *param);

void stream_polyrep(void *node, void *coord, void *color, void *normal, void *texCoord);
void
render_ray_polyrep(void *node);

void compile_polyrep(void *node, void *coord, void *color, void *normal, void *texCoord);

