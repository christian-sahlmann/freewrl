/*
=INSERT_TEMPLATE_HERE=

$Id: Polyrep.c,v 1.41 2011/02/11 18:46:25 crc_canada Exp $

???

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
#include "../opengl/Frustum.h"
#include "../opengl/Material.h"
#include "../opengl/Textures.h"
#include "../opengl/OpenGL_Utils.h"
#include "../scenegraph/Component_Shape.h"
#include "../scenegraph/RenderFuncs.h"

#include "Polyrep.h"
#include "LinearAlgebra.h"
#include "Tess.h"


/* reset colors to defaults, if we have to */
static GLfloat diffuseColor[] = {0.3f, 0.3f, 0.8f, 1.0f};
static GLfloat ambientIntensity[] = {0.16f, 0.16f, 0.16f, 1.0f}; /*VRML diff*amb defaults */
static GLfloat specularColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
static GLfloat emissiveColor[] = {0.0f, 0.0f, 0.0f, 1.0f};


/* Polyrep rendering, node has a color field, which is an RGB field (not RGBA) and transparency is changing */
static void recalculateColorField(struct X3D_PolyRep *r) {
	int n;
	struct SFColorRGBA *newcolors;
	float *op, *np;

	/* first, make sure we do not do this over and over... */
	r->transparency = appearanceProperties.transparency;

	newcolors = MALLOC (struct SFColorRGBA *, sizeof (struct SFColorRGBA)*r->ntri*3);
	op = r->color;
	np = (float *)newcolors;

	for (n=0; n<r->ntri*3; n++) {
		*np = *op; np++; op++;  		/* R */
		*np = *op; np++; op++;  		/* G */
		*np = *op; np++; op++;  		/* B */
		*np = appearanceProperties.transparency; np++; op++;	/* A */
	}
	FREE_IF_NZ(r->color);
	r->color = (float *)newcolors;

	/* VBOs need this re-bound */
	if (global_use_VBOs) {
		if (r->VBO_buffers[COLOR_VBO] == 0) glGenBuffers(1,&r->VBO_buffers[COLOR_VBO]);
		FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,r->VBO_buffers[COLOR_VBO]);
		glBufferData(GL_ARRAY_BUFFER,r->ntri*sizeof(struct SFColorRGBA)*3,r->color, GL_STATIC_DRAW);
		FREE_IF_NZ(r->color);
	}
}

/* How many faces are in this IndexedFaceSet?			*/

int count_IFS_faces(int cin, struct Multi_Int32 *coordIndex) {
	/* lets see how many faces we have */
	int pointctr=0;
	int max_points_per_face = 0;
	int min_points_per_face = 99999;
	int i;
	int faces = 0;

	if (coordIndex == NULL) return 0;
	if (coordIndex->n == 0) return 0;

	for(i=0; i<cin; i++) {

		if((coordIndex->p[i] == -1) || (i==cin-1)) {
			if(coordIndex->p[i] != -1) {
				pointctr++;
			}

			faces++;
			if (pointctr > max_points_per_face)
				max_points_per_face = pointctr;
			if (pointctr < min_points_per_face)
				min_points_per_face = pointctr;
			pointctr = 0;
		} else pointctr++;
	}


	/*	
	printf ("this structure has %d faces\n",faces);
	printf ("	max points per face %d\n",max_points_per_face);
	printf ("	min points per face %d\n\n",min_points_per_face);
	*/
	
	if (faces < 1) {
		/* printf("an IndexedFaceSet with no faces found\n"); */
		return (0);
	}
	return faces;
}


/* Generate the normals for each face of an IndexedFaceSet	*/
/* create two datastructures:					*/
/* 	- face normals; given a face, tell me the normal	*/
/*	- point-face;   for each point, tell me the face(s)	*/

int IFS_face_normals (
	struct point_XYZ *facenormals,
	int *faceok,
	int *pointfaces,
	int faces,
	int npoints,
	int cin,
	struct SFVec3f *points,
	struct Multi_Int32 *coordIndex,
	int ccw) {

	int tmp_a = 0, this_face_finished;
	int i,checkpoint;
	int facectr;
	int pt_1, pt_2, pt_3;
	float AC, BC;
	struct SFVec3f *c1,*c2,*c3;
	float a[3]; float b[3];

	int retval = FALSE;

	float this_vl;
	struct point_XYZ thisfaceNorms;

	/* printf ("IFS_face_normals, faces %d\n",faces); */

	/*  Assume each face is ok for now*/
	for(i=0; i<faces; i++) {
		faceok[i] = TRUE;
	}

	/*  calculate normals for each face*/
	for(i=0; i<faces; i++) {
		/* lets decide which normal to choose here, in case of more than 1 triangle.
		   we choose the triangle with the greatest vector length hoping that it is
		   the least "degenerate" of them all */
		this_vl = 0.0f;
		facenormals[i].x = 0.0;
		facenormals[i].y = 0.0;
		facenormals[i].z = 1.0;


		if (tmp_a >= cin-2) {
			printf ("last face in Indexed Geometry has not enough vertexes\n");
			faceok[i] = FALSE;
		} else {
			/* does this face have at least 3 vertexes? */
			if ((coordIndex->p[tmp_a] == -1) ||
			    (coordIndex->p[tmp_a+1] == -1) ||
			    (coordIndex->p[tmp_a+2] == -1)) {
				printf ("IndexedFaceNormals: have a face with two or less vertexes\n");
				faceok[i] = FALSE;

				if (coordIndex->p[tmp_a] != -1) tmp_a++;
			} else {
				/* check to see that the coordIndex does not point to a
				   point that is outside the range of our point array */
				checkpoint = tmp_a;
				while (checkpoint < cin) {
					if (coordIndex->p[checkpoint] == -1) {
						checkpoint = cin; /*  stop the scan*/
					} else {
						/* printf ("verifying %d for face %d\n",coordIndex->p[checkpoint],i); */
						if ((coordIndex->p[checkpoint] < 0) ||
						    (coordIndex->p[checkpoint] >= npoints)) {
							printf ("Indexed Geometry face %d has a point out of range,",i);
							printf (" point is %d, should be between 0 and %d\n",
								coordIndex->p[checkpoint],npoints-1);
							faceok[i] = FALSE;
						}
						checkpoint++;
					}
				}
			}
		}

		/* face has passed checks so far... */
		if (faceok[i]) {
			/* printf ("face %d ok\n",i); */
			/* check for degenerate triangles -- we go through all triangles in a face to see which
			   triangle has the largest vector length */

			this_face_finished = FALSE;
			pt_1 = tmp_a;
			if (ccw) {
				/* printf ("IFS face normals CCW\n"); */
				pt_2 = tmp_a+1; pt_3 = tmp_a+2;
			} else {
				/* printf ("IFS face normals *NOT* CCW\n"); */
				pt_3 = tmp_a+1; pt_2 = tmp_a+2;
			}

			do {
				/* first three coords give us the normal */
				c1 = &(points[coordIndex->p[pt_1]]);
				c2 = &(points[coordIndex->p[pt_2]]);
				c3 = &(points[coordIndex->p[pt_3]]);

				a[0] = c2->c[0] - c1->c[0];
				a[1] = c2->c[1] - c1->c[1];
				a[2] = c2->c[2] - c1->c[2];
				b[0] = c3->c[0] - c1->c[0];
				b[1] = c3->c[1] - c1->c[1];
				b[2] = c3->c[2] - c1->c[2];

				/* printf ("a0 %f a1 %f a2 %f b0 %f b1 %f b2 %f\n", a[0],a[1],a[2],b[0],b[1],b[2]); */

				thisfaceNorms.x = a[1]*b[2] - b[1]*a[2];
				thisfaceNorms.y = -(a[0]*b[2] - b[0]*a[2]);
				thisfaceNorms.z = a[0]*b[1] - b[0]*a[1];

				/* printf ("vector length is %f\n",calc_vector_length (thisfaceNorms));  */

				/* is this vector length greater than a previous one? */
				if (calc_vector_length(thisfaceNorms) > this_vl) {
					/* printf ("for face, using points %d %d %d\n",pt_1, pt_2, pt_3);  */
					this_vl = calc_vector_length(thisfaceNorms);
					facenormals[i].x = thisfaceNorms.x;
					facenormals[i].y = thisfaceNorms.y;
					facenormals[i].z = thisfaceNorms.z;
				}

				/* lets skip along to next triangle in this face */

				AC=(c1->c[0]-c3->c[0])*(c1->c[1]-c3->c[1])*(c1->c[2]-c3->c[2]);
				BC=(c2->c[0]-c3->c[0])*(c2->c[1]-c3->c[1])*(c2->c[2]-c3->c[2]);
				/* printf ("AC %f ",AC); printf ("BC %f \n",BC); */

				/* we have 3 points, a, b, c */
				/* we also have 3 vectors, AB, AC, BC */
				/* find out which one looks the closest one to skip out */
				/* either we move both 2nd and 3rd points, or just the 3rd */

				if (ccw) {
					/* printf ("moving along IFS face normals CCW\n");  */
					if (fabs(AC) < fabs(BC)) { pt_2++; }
					pt_3++;
				} else {
					/* printf ("moving along IFS face normals *NOT* CCW\n"); */
					/* if (fabs(AC) < fabs(BC)) { pt_3++; } */
					pt_2++;
				}

				/* skip forward to the next couple of points - if possible */
				/* printf ("looking at %d, cin is %d\n",tmp_a, cin); */
				tmp_a ++;
				if ((tmp_a >= cin-2) || (coordIndex->p[tmp_a+2] == -1)) {
					this_face_finished = TRUE;  tmp_a +=2;
				}
			} while (!this_face_finished);

			if (APPROX(this_vl,0.0)) {
				/* printf ("face %d is degenerate\n",i); */
				faceok[i] = 0;
			} else {
				/* printf ("face %d is ok\n",i); */
				normalize_vector(&facenormals[i]);

			/*	
			printf ("vertices \t%f %f %f\n\t\t%f %f %f\n\t\t%f %f %f\n",
				c1->c[0],c1->c[1],c1->c[2],
				c2->c[0],c2->c[1],c2->c[2],
				c3->c[0],c3->c[1],c3->c[2]);
			printf ("normal %f %f %f\n\n",facenormals[i].x,
				facenormals[i].y,facenormals[i].z);
		
			*/
			}
			

		}

		/* skip forward to next ifs - we have the normal - but check for bad Points!*/
		if (i<faces-1) {
			if (tmp_a <= 0) {
				/* this is an error in the input file; lets try and continue */
				tmp_a = 1;
			} 

			if (tmp_a > 0) {
				while (((coordIndex->p[tmp_a-1]) != -1) && (tmp_a < cin-2)) {
					/* printf ("skipping past %d for face %d\n",coordIndex->p[tmp_a-1],i);*/
					tmp_a++;
				}
			}
		}
		/* printf ("for face %d, vec len is %f\n",i,this_vl); */
	}


	/* do we have any valid faces??? */
	for(i=0; i<faces; i++) {
		if (faceok[i] == TRUE) {
			retval = TRUE;
		}
	}
	if (!retval) return retval; /* nope, lets just drop out of here */
	

	/* now, go through each face, and make a point-face list
	   so that I can give it a point later, and I will know which face(s)
	   it belong to that point */
	/* printf ("\nnow generating point-face list\n");   */
	for (i=0; i<npoints; i++) { pointfaces[i*POINT_FACES]=0; }
	facectr=0;
	for(i=0; i<cin; i++) {
		tmp_a=coordIndex->p[i];
		/* printf ("pointfaces, coord %d coordIndex %d face %d\n",i,tmp_a,facectr); */
		if (tmp_a == -1) {
			facectr++;
		} else {
			if (faceok[facectr]) {
				tmp_a*=POINT_FACES;
				add_to_face (tmp_a,facectr,pointfaces);
			} else {
			/* 	printf ("skipping add_to_face for invalid face %d\n",facectr);*/
			}
		}
	}

	/*
	 printf ("\ncheck \n");
	for (i=0; i<npoints; i++) {
		int tmp_b;

		tmp_a = i*POINT_FACES;
		printf ("point %d is in %d faces, these are:\n ", i, pointfaces[tmp_a]);
		for (tmp_b=0; tmp_b<pointfaces[tmp_a]; tmp_b++) {
			printf ("%d ",pointfaces[tmp_a+tmp_b+1]);
		}
		printf ("\n");
	}
	*/

	return retval;
}



/* Tesselated faces MAY have the wrong normal calculated. re-calculate after tesselation	*/

void Extru_check_normal (
	struct point_XYZ *facenormals,
	int this_face,
	int direction,
	struct X3D_PolyRep  *rep_,
	int ccw) {

	/* only use this after tesselator as we get coord indexes from global var */
	struct SFVec3f *c1,*c2,*c3;
	float a[3]; float b[3];
	int zz1, zz2;

	if (ccw) {
		zz1 = 1;
		zz2 = 2;
	} else {
		zz1 = 2;
		zz2 = 1;
	}

	/* first three coords give us the normal */
 	c1 = (struct SFVec3f *) &rep_->actualCoord[3*global_IFS_Coords[0]];
 	c2 = (struct SFVec3f *) &rep_->actualCoord[3*global_IFS_Coords[zz1]];
 	c3 = (struct SFVec3f *) &rep_->actualCoord[3*global_IFS_Coords[zz2]];

	/*printf ("Extru_check_normal, coords %d %d %d\n",global_IFS_Coords[0],
		global_IFS_Coords[1],global_IFS_Coords[2]);
	printf ("Extru_check_normal vertices \t%f %f %f\n\t\t%f %f %f\n\t\t%f %f %f\n",
		c1->c[0],c1->c[1],c1->c[2],
		c2->c[0],c2->c[1],c2->c[2],
		c3->c[0],c3->c[1],c3->c[2]);
	*/

	a[0] = c2->c[0] - c1->c[0];
	a[1] = c2->c[1] - c1->c[1];
	a[2] = c2->c[2] - c1->c[2];
	b[0] = c3->c[0] - c1->c[0];
	b[1] = c3->c[1] - c1->c[1];
	b[2] = c3->c[2] - c1->c[2];

	facenormals[this_face].x = a[1]*b[2] - b[1]*a[2] * direction;
	facenormals[this_face].y = -(a[0]*b[2] - b[0]*a[2]) * direction;
	facenormals[this_face].z = a[0]*b[1] - b[0]*a[1] * direction;

	if (APPROX(calc_vector_length (facenormals[this_face]),0.0)) { 
		ConsoleMessage ("WARNING: FreeWRL got degenerate triangle; OpenGL tesselator should not give degenerate triangles back %f\n",
			fabs(calc_vector_length (facenormals[this_face])));
	}

	normalize_vector(&facenormals[this_face]);
	/* printf ("facenormal for %d is %f %f %f\n",this_face, facenormals[this_face].x,
			facenormals[this_face].y, facenormals[this_face].z); */
}

/* Tesselated faces MAY have the wrong normal calculated. re-calculate after tesselation	*/


void IFS_check_normal (
	struct point_XYZ *facenormals,
	int this_face,
	struct SFVec3f *points, int base,
	struct Multi_Int32 *coordIndex, int ccw) {

	struct SFVec3f *c1,*c2,*c3;
	float a[3]; float b[3];


	/* printf ("IFS_check_normal, base %d points %d %d %d\n",base,*/
	/* 	global_IFS_Coords[0],global_IFS_Coords[1],global_IFS_Coords[2]);*/
	/* printf ("normal was %f %f %f\n\n",facenormals[this_face].x,*/
	/* 	facenormals[this_face].y,facenormals[this_face].z);*/


	/* first three coords give us the normal */
	c1 = &(points[coordIndex->p[base+global_IFS_Coords[0]]]);
	if (ccw) {
		c2 = &(points[coordIndex->p[base+global_IFS_Coords[1]]]);
		c3 = &(points[coordIndex->p[base+global_IFS_Coords[2]]]);
	} else {
		c3 = &(points[coordIndex->p[base+global_IFS_Coords[1]]]);
		c2 = &(points[coordIndex->p[base+global_IFS_Coords[2]]]);
	}

	a[0] = c2->c[0] - c1->c[0];
	a[1] = c2->c[1] - c1->c[1];
	a[2] = c2->c[2] - c1->c[2];
	b[0] = c3->c[0] - c1->c[0];
	b[1] = c3->c[1] - c1->c[1];
	b[2] = c3->c[2] - c1->c[2];

	facenormals[this_face].x = a[1]*b[2] - b[1]*a[2];
	facenormals[this_face].y = -(a[0]*b[2] - b[0]*a[2]);
	facenormals[this_face].z = a[0]*b[1] - b[0]*a[1];

	/* printf ("vector length is %f\n",calc_vector_length (facenormals[this_face])); */

	if (APPROX(calc_vector_length (facenormals[this_face]),0.0)) {
		/* printf ("warning: Tesselated surface has invalid normal - if this is an IndexedFaceSet, check coordinates of ALL faces\n");*/
	} else {

		normalize_vector(&facenormals[this_face]);


		/* printf ("vertices \t%f %f %f\n\t\t%f %f %f\n\t\t%f %f %f\n",*/
		/* 	c1->c[0],c1->c[1],c1->c[2],*/
		/* 	c2->c[0],c2->c[1],c2->c[2],*/
		/* 	c3->c[0],c3->c[1],c3->c[2]);*/
		/* printf ("normal %f %f %f\n\n",facenormals[this_face].x,*/
		/* 	facenormals[this_face].y,facenormals[this_face].z);*/
	}

}


void add_to_face (
	int point,
	int face,
	int *pointfaces) {

	int count;
	if (pointfaces[point] < (POINT_FACES-1)) {
		/* room to add, but is it already there? */
		for (count = 1; count <= pointfaces[point]; count++) {
			if (pointfaces[point+count] == face) return;
		}
		/* ok, we have an empty slot, and face not already added */
		pointfaces[point]++;
		pointfaces[point+ pointfaces[point]] = face;
	}
}

/********************************************************************
 *
 * ElevationGrid Triangle
 *
 */
void Elev_Tri (
	int vertex_ind,
	int this_face,
	int A,
	int D,
	int E,
	int NONORMALS,
	struct X3D_PolyRep *this_Elev,
	struct point_XYZ *facenormals,
	int *pointfaces,
	int ccw) {

	struct SFVec3f *c1,*c2,*c3;
	float a[3]; float b[3];
	int tmp;

	/* printf ("Elev_Tri Triangle %d %d %d\n",A,D,E); */

	/* generate normals in a clockwise manner, reverse the triangle */
	if (!(ccw)) {
		tmp = D;
		D = E;
		E = tmp;
	}


	this_Elev->cindex[vertex_ind] = A;
	this_Elev->cindex[vertex_ind+1] = D;
	this_Elev->cindex[vertex_ind+2] = E;

	/*
	printf ("Elev_Tri, vertices for vertex_ind %d are:",vertex_ind);
               c1 = (struct SFVec3f *) &this_Elev->actualCoord[3*A];
               c2 = (struct SFVec3f *) &this_Elev->actualCoord[3*D];
               c3 = (struct SFVec3f *) &this_Elev->actualCoord[3*E];

	printf ("\n%f %f %f\n%f %f %f\n%f %f %f\n\n",
		c1->c[0], c1->c[1],c1->c[2],c2->c[0],c2->c[1],c2->c[2],
		c3->c[0],c3->c[1],c3->c[2]);
	*/


	if (NONORMALS) {
		/* calculate normal for this triangle */
                c1 = (struct SFVec3f *) &this_Elev->actualCoord[3*A];
                c2 = (struct SFVec3f *) &this_Elev->actualCoord[3*D];
                c3 = (struct SFVec3f *) &this_Elev->actualCoord[3*E];

		/*
		printf ("calc norms \n%f %f %f\n%f %f %f\n%f %f %f\n",
		c1->c[0], c1->c[1],c1->c[2],c2->c[0],c2->c[1],c2->c[2],
		c3->c[0],c3->c[1],c3->c[2]);
		*/

		a[0] = c2->c[0] - c1->c[0];
		a[1] = c2->c[1] - c1->c[1];
		a[2] = c2->c[2] - c1->c[2];
		b[0] = c3->c[0] - c1->c[0];
		b[1] = c3->c[1] - c1->c[1];
		b[2] = c3->c[2] - c1->c[2];

		facenormals[this_face].x = a[1]*b[2] - b[1]*a[2];
		facenormals[this_face].y = -(a[0]*b[2] - b[0]*a[2]);
		facenormals[this_face].z = a[0]*b[1] - b[0]*a[1];

		/*
		printf ("facenormals index %d is %f %f %f\n",this_face, facenormals[this_face].x,
				facenormals[this_face].y, facenormals[this_face].z);
		*/

		/* add this face to the faces for this point */
		add_to_face (A*POINT_FACES,this_face,pointfaces);
		add_to_face (D*POINT_FACES,this_face,pointfaces);
		add_to_face (E*POINT_FACES,this_face,pointfaces);
	}
}



/***********************************************************************8
 *
 * Extrusion Texture Mapping
 *
 ***********************************************************************/

void Extru_tex(
	int vertex_ind,
	int tci_ct,
	int A,
	int B,
	int C,
	int *tcindex,
	int ccw,
	int tcindexsize) {

	int j;

	/* bounds check */
	/* printf ("Extru_tex, tcindexsize %d, vertex_ind %d\n",tcindexsize, vertex_ind);  */
	if (vertex_ind+2 >= tcindexsize) {
		printf ("INTERNAL ERROR: Extru_tex, bounds check %d >= %d\n",vertex_ind+2,tcindexsize);
	}

	/* generate textures in a clockwise manner, reverse the triangle */
	if (!(ccw)) { j = B; B = C; C = j; }

	/* ok, we have to do textures; lets do the tcindexes and record min/max */
	tcindex[vertex_ind] = tci_ct+A;
	tcindex[vertex_ind+1] =tci_ct+B;
	tcindex[vertex_ind+2] =tci_ct+C;
}


/*********************************************************************
 *
 * S,T mappings for Extrusions on begin and end caps.
 *
 **********************************************************************/


void Extru_ST_map(
	int triind_start,
	int start,
	int end,
	float *Vals,
	int nsec,
	int *tcindex,
	int *cindex,
	float *GeneratedTexCoords,
	int tcoordsize) {

	int x;
	GLfloat minS = 9999.9f;
	GLfloat maxS = -9999.9f;
	GLfloat minT = 9999.9f;
	GLfloat maxT = -9999.9f;

	GLfloat Srange = 0.0f;
	GLfloat Trange = 0.0f;

	int Point_Zero;	/* the point that all cap tris start at. see comment below */

	/* printf ("Extru_ST, nsec %d\n",nsec); */

	/* find the base and range of S, T */
	for (x=0; x<nsec; x++) {
		 /* printf ("for textures, coord vals %f %f for sec %d\n", Vals[x*2+0], Vals[x*2+1],x); */
		if (Vals[x*2+0] < minS) minS = Vals[x*2+0];
		if (Vals[x*2+0] > maxS) maxS = Vals[x*2+0];
		if (Vals[x*2+1] < minT) minT = Vals[x*2+1];
		if (Vals[x*2+1] > maxT) maxT = Vals[x*2+1];
	}
	Srange = maxS -minS;
	Trange = maxT - minT;

	/* I hate divide by zeroes. :-) */
	if (APPROX(Srange, 0.0)) Srange = 0.001f;
	if (APPROX(Trange, 0.0)) Trange = 0.001f;

	/* printf ("minS %f Srange %f minT %f Trange %f\n",minS,Srange,minT,Trange); */

	/* Ok, we know the min vals of S and T; and the ranges. The way that end cap
	 * triangles are drawn is that we have one common point, the first point in
	 * each triangle. Use this as a base into the Vals index, to generate a S,T
	 * tex coord mapping for the [0,1] range
	 */

	for(x=start; x<end; x++) {
		int tci, ci;

		/*
		printf ("Extru_ST_Map: triangle has tex vertices:%d %d %d ",
			tcindex[triind_start*3],
			tcindex[triind_start*3+1] ,
			tcindex[triind_start*3+2]);
		printf ("Extru_ST_Map: coord vertices:%d %d %d\n",
			cindex[triind_start*3],
			cindex[triind_start*3+1] ,
			cindex[triind_start*3+2]);
		*/

		/* for first vertex */
		tci = tcindex[triind_start*3];
		ci = cindex[triind_start*3];
		Point_Zero = tci;

		if ((tci*3+2) >= tcoordsize) {
			printf ("INTERNAL ERROR: Extru_ST_map(1), index %d greater than %d \n",(tci*3+2),tcoordsize);
			return;
		}

		/* S value */
		GeneratedTexCoords[tci*3+0] = (Vals[(tci-Point_Zero)*2+0] - minS) / Srange ;

		/* not used by render_polyrep */
		GeneratedTexCoords[tci*3+1] = 0;

		/* T value */
		GeneratedTexCoords[tci*3+2] = (Vals[(tci-Point_Zero)*2+1] - minT) / Trange;


		/* for second vertex */
		tci = tcindex[triind_start*3+1];
		ci = cindex[triind_start*3+1];

		if ((tci*3+2) >= tcoordsize) {
			printf ("INTERNAL ERROR: Extru_ST_map(2), index %d greater than %d \n",(tci*3+2),tcoordsize);
			return;
		}

		/* S value */
		GeneratedTexCoords[tci*3+0] = (Vals[(tci-Point_Zero)*2+0] - minS) / Srange ;

		/* not used by render_polyrep */
		GeneratedTexCoords[tci*3+1] = 0;

		/* T value */
		GeneratedTexCoords[tci*3+2] = (Vals[(tci-Point_Zero)*2+1] - minT) / Trange;


		/* for third vertex */
		tci = tcindex[triind_start*3+2];
		ci = cindex[triind_start*3+2];

		if ((tci*3+2) >= tcoordsize) {
			printf ("INTERNAL ERROR: Extru_ST_map(3), index %d greater than %d \n",(tci*3+2),tcoordsize);
			return;
		}

		/* S value */
		GeneratedTexCoords[tci*3+0] = (Vals[(tci-Point_Zero)*2+0] - minS) / Srange ;

		/* not used by render_polyrep */
		GeneratedTexCoords[tci*3+1] = 0;

		/* T value */
		GeneratedTexCoords[tci*3+2] = (Vals[(tci-Point_Zero)*2+1] - minT) / Trange;

		triind_start++;
	}
}


void do_glNormal3fv(struct SFVec3f *dest, GLfloat *param) {
	struct point_XYZ myp;

	/* normalize all vectors; even if they are coded into a VRML file */

	myp.x = param[0]; myp.y = param[1]; myp.z = param[2];

	normalize_vector (&myp);

	dest->c[0] = (float) myp.x; dest->c[1] = (float) myp.y; dest->c[2] = (float) myp.z;
}





/*********************************************************************
 *
 * render_polyrep : render one of the internal polygonal representations
 * for some nodes
 *
 ********************************************************************/

void render_polyrep(void *node) {
	struct X3D_Virt *virt;
	struct X3D_Node *renderedNodePtr;
	struct X3D_PolyRep *pr;
	int hasc;

	renderedNodePtr = X3D_NODE(node);
	virt = virtTable[renderedNodePtr->_nodeType];
	pr = (struct X3D_PolyRep *)renderedNodePtr->_intern;

	#ifdef TEXVERBOSE
	printf ("\nrender_polyrep, _nodeType %s\n",stringNodeType(renderedNodePtr->_nodeType)); 
	printf ("ntri %d\n",r->ntri);
	#endif

	if (pr->ntri==0) {
		/* no triangles */
		return;
	}

	if (!pr->streamed) {
		printf ("render_polyrep, not streamed, returning\n");
		return;
	}

	/* save these values for streaming the texture coordinates later */
	global_tcin = pr->tcindex;
	global_tcin_count = pr->ntri*3;
	global_tcin_lastParent = node;

	/* we take the geometry here, and push it up the stream. */
        setExtent( renderedNodePtr->EXTENT_MAX_X, renderedNodePtr->EXTENT_MIN_X, renderedNodePtr->EXTENT_MAX_Y,
                renderedNodePtr->EXTENT_MIN_Y, renderedNodePtr->EXTENT_MAX_Z, renderedNodePtr->EXTENT_MIN_Z,
                renderedNodePtr);

	/*  clockwise or not?*/
	if (!pr->ccw) { FW_GL_FRONTFACE(GL_CW); }
	
	hasc = ((pr->VBO_buffers[COLOR_VBO]!=0) || pr->color) && (last_texture_type!=TEXTURE_NO_ALPHA);

	/* Do we have any colours? Are textures, if present, not RGB? */
	if(hasc){
		if (!pr->isRGBAcolorNode) 
			if (!APPROX(pr->transparency,appearanceProperties.transparency)) {
				recalculateColorField(pr);
			}
		
		LIGHTING_ON
		do_glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseColor);
	
		FW_GL_ENABLE(GL_COLOR_MATERIAL);

		#ifndef IPHONE
		FW_GL_COLOR_MATERIAL(GL_FRONT_AND_BACK, GL_DIFFUSE);
		#endif

		FW_GL_COLOR4FV(diffuseColor);
	
		do_glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientIntensity);
		do_glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor);
		do_glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissiveColor);
	}
	


	if (!global_use_VBOs) {
		/*  status bar, text do not have normals*/
		if (pr->normal) {
			FW_GL_NORMAL_POINTER(GL_FLOAT,0,(GLfloat *) pr->normal);
		} else FW_GL_DISABLECLIENTSTATE(GL_NORMAL_ARRAY); 
	
		/*  colours?*/
		if (hasc) {
			FW_GL_ENABLECLIENTSTATE(GL_COLOR_ARRAY);
			FW_GL_COLOR_POINTER(4,GL_FLOAT,0,pr->color);
		}

		/*  textures?*/
		if (pr->GeneratedTexCoords) {
			struct textureVertexInfo mtf = {pr->GeneratedTexCoords,2,GL_FLOAT,0,NULL};
			textureDraw_start(NULL,&mtf);
		} else {
			textureDraw_start(X3D_NODE(node), NULL);
		}
	
		/* do the array drawing; sides are simple 0-1-2,3-4-5,etc triangles */
		FW_GL_VERTEX_POINTER(3,GL_FLOAT,0,(GLfloat *) pr->actualCoord);
		FW_GL_DRAWELEMENTS(GL_TRIANGLES,pr->ntri*3,GL_UNSIGNED_INT, pr->cindex);

		/*  put things back to the way they were;*/
		if (!pr->normal) FW_GL_ENABLECLIENTSTATE(GL_NORMAL_ARRAY);
		if (hasc) {
			FW_GL_DISABLECLIENTSTATE(GL_COLOR_ARRAY);
			FW_GL_DISABLE(GL_COLOR_MATERIAL);
		}
	} else {
		/*  status bar, text do not have normals*/
		if (pr->VBO_buffers[NORMAL_VBO]!=0) {
			FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, pr->VBO_buffers[NORMAL_VBO]);
			FW_GL_NORMAL_POINTER(GL_FLOAT,0,0);
		} else FW_GL_DISABLECLIENTSTATE(GL_NORMAL_ARRAY); 
	
		/* colours? */
		if (hasc) {
			FW_GL_ENABLECLIENTSTATE(GL_COLOR_ARRAY);
			FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,pr->VBO_buffers[COLOR_VBO]);
			FW_GL_COLOR_POINTER(4,GL_FLOAT,0,0);
		}
		/*  textures?*/
		if (pr->VBO_buffers[TEXTURE_VBO] != 0) {
				struct textureVertexInfo mtf = {NULL,2,GL_FLOAT,0, NULL};
				FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,pr->VBO_buffers[TEXTURE_VBO]);
				textureDraw_start(NULL,&mtf);
		} else {
			textureDraw_start(X3D_NODE(node), NULL);
		}
		FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, pr->VBO_buffers[VERTEX_VBO]);
		FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER,pr->VBO_buffers[INDEX_VBO]);
		FW_GL_ENABLECLIENTSTATE(GL_VERTEX_ARRAY); // should already be enabled

		FW_GL_VERTEX_POINTER(3,GL_FLOAT,0,0);
		glDrawElements(GL_TRIANGLES,pr->ntri*3,GL_UNSIGNED_INT,0);

		/* turn VBOs off for now */
		FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, 0);
		FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER, 0);

		/*  put things back to the way they were;*/
		if (pr->VBO_buffers[NORMAL_VBO] == 0) FW_GL_ENABLECLIENTSTATE(GL_NORMAL_ARRAY);
		if (hasc) {
			FW_GL_DISABLECLIENTSTATE(GL_COLOR_ARRAY);
			FW_GL_DISABLE(GL_COLOR_MATERIAL);
		}
	}

	trisThisLoop += pr->ntri;

	textureDraw_end();
	if (!pr->ccw) FW_GL_FRONTFACE(GL_CCW);

	#ifdef TEXVERBOSE
	{
		int i;
		int *cin;
		float *cod;
		float *tcod;
		tcod = pr->GeneratedTexCoords;
		cod = pr->actualCoord;
		cin = pr->cindex;
		printf ("\n\nrender_polyrep:\n");
		for (i=0; i<pr->ntri*3; i++) {
			printf ("i %d cindex %d vertex %f %f %f",i,cin[i],
				cod[cin[i]*3+0],
				cod[cin[i]*3+1],
				cod[cin[i]*3+2]);

			if (tcod != 0) {
			printf (" tex %f %f",
				tcod[cin[i]*2+0],
				tcod[cin[i]*2+1]);
			}
			printf ("\n");
		}
	}
	#endif


}


/*********************************************************************
 *
 * render_ray_polyrep : get intersections of a ray with one of the
 * polygonal representations
 *
 * currently handled:
 *	rendray_Text 
 *	rendray_ElevationGrid  
 *	rendray_Extrusion 
 *	rendray_IndexedFaceSet  
 *	rendray_ElevationGrid 
 *	rendray_IndexedTriangleSet 
 *	rendray_IndexedTriangleFanSet 
 *	rendray_IndexedTriangleStripSet 
 *	rendray_TriangleSet 
 *	rendray_TriangleFanSet 
 *	rendray_TriangleStripSet 
 *	rendray_GeoElevationGrid 
 */

void render_ray_polyrep(void *node) {
	struct X3D_Virt *virt;
	struct X3D_Node *genericNodePtr;
	struct X3D_PolyRep *polyRep;
	int i;
	int pt;
	float *point[3];
	struct point_XYZ v1, v2, v3;
	struct point_XYZ ray;
	float pt1, pt2, pt3;
	struct point_XYZ hitpoint;
	float tmp1,tmp2;
	float v1len, v2len, v3len;
	float v12pt;

	/* is this structure still loading? */
	if (!node) return;

	ray.x = t_r2.x - t_r1.x;
	ray.y = t_r2.y - t_r1.y;
	ray.z = t_r2.z - t_r1.z;

	genericNodePtr = X3D_NODE(node);
	virt = virtTable[genericNodePtr->_nodeType];
	
	/* is this structure still loading? */
	if (!(genericNodePtr->_intern)) {
		/* printf ("render_ray_polyrep - no internal structure, returning\n"); */
		return;
	}

	polyRep = (struct X3D_PolyRep *)genericNodePtr->_intern;

	/*	
	printf("render_ray_polyrep %d '%s' (%d %d): %d\n",node,stringNodeType(genericNodePtr->_nodeType),
		genericNodePtr->_change, polyRep->_change, polyRep->ntri);
	*/

	

	for(i=0; i<polyRep->ntri; i++) {
		for(pt = 0; pt<3; pt++) {
			int ind = polyRep->cindex[i*3+pt];
			point[pt] = (polyRep->actualCoord+3*ind);
		}

		/*
		printf ("have points (%f %f %f) (%f %f %f) (%f %f %f)\n",
			point[0][0],point[0][1],point[0][2],
			point[1][0],point[1][1],point[1][2],
			point[2][0],point[2][1],point[2][2]);
		*/
		
		/* First we need to project our point to the surface */
		/* Poss. 1: */
		/* Solve s1xs2 dot ((1-r)r1 + r r2 - pt0)  ==  0 */
		/* I.e. calculate s1xs2 and ... */
		v1.x = point[1][0] - point[0][0];
		v1.y = point[1][1] - point[0][1];
		v1.z = point[1][2] - point[0][2];
		v2.x = point[2][0] - point[0][0];
		v2.y = point[2][1] - point[0][1];
		v2.z = point[2][2] - point[0][2];
		v1len = (float) sqrt(VECSQ(v1)); VECSCALE(v1, 1/v1len);
		v2len = (float) sqrt(VECSQ(v2)); VECSCALE(v2, 1/v2len);
		v12pt = (float) VECPT(v1,v2);

		/* this will get around a divide by zero further on JAS */
		if (fabs(v12pt-1.0) < 0.00001) continue;

		/* if we have a degenerate triangle, we can't compute a normal, so skip */

		if ((fabs(v1len) > 0.00001) && (fabs(v2len) > 0.00001)) {

			/* v3 is our normal to the surface */
			VECCP(v1,v2,v3);
			v3len = (float) sqrt(VECSQ(v3)); VECSCALE(v3, 1/v3len);

			pt1 = (float) VECPT(t_r1,v3);
			pt2 = (float) VECPT(t_r2,v3);
			pt3 = (float) (v3.x * point[0][0] + v3.y * point[0][1] + v3.z * point[0][2]);
			/* Now we have (1-r)pt1 + r pt2 - pt3 = 0
			 * r * (pt1 - pt2) = pt1 - pt3
			 */
			 tmp1 = pt1-pt2;
			 if(!APPROX(tmp1,0)) {
			 	float ra, rb;
				float k,l;
				struct point_XYZ p0h;

			 	tmp2 = (float) ((pt1-pt3) / (pt1-pt2));
				hitpoint.x = MRATX(tmp2);
				hitpoint.y = MRATY(tmp2);
				hitpoint.z = MRATZ(tmp2);
				/* Now we want to see if we are in the triangle */
				/* Projections to the two triangle sides */
				p0h.x = hitpoint.x - point[0][0];
				p0h.y = hitpoint.y - point[0][1];
				p0h.z = hitpoint.z - point[0][2];
				ra = (float) VECPT(v1, p0h);
				if(ra < 0.0f) {continue;}
				rb = (float) VECPT(v2, p0h);
				if(rb < 0.0f) {continue;}
				/* Now, the condition for the point to
				 * be inside
				 * (ka + lb = p)
				 * (k + l b.a = p.a)
				 * (k b.a + l = p.b)
				 * (k - (b.a)**2 k = p.a - (b.a)*p.b)
				 * k = (p.a - (b.a)*(p.b)) / (1-(b.a)**2)
				 */
				 k = (ra - v12pt * rb) / (1-v12pt*v12pt);
				 l = (rb - v12pt * ra) / (1-v12pt*v12pt);
				 k /= v1len; l /= v2len;
				 if(k+l > 1 || k < 0 || l < 0) {
				 	continue;
				 }
				 rayhit(((float)(tmp2)),
					((float)(hitpoint.x)),
					((float)(hitpoint.y)),
					((float)(hitpoint.z)),
				 	((float)(v3.x)),
					((float)(v3.y)),
					((float)(v3.z)),
					((float)-1),((float)-1), "polyrep");
			 }
		/*
		} else {
			printf ("render_ray_polyrep, skipping degenerate triangle\n");
		*/
		}
	}
}

/* make the internal polyrep structure - this will contain the actual RUNTIME parameters for OpenGL */
void compile_polyrep(void *innode, void *coord, void *color, void *normal, void *texCoord) {
	struct X3D_Virt *virt;
	struct X3D_Node *node;
	struct X3D_PolyRep *polyrep;

	node = X3D_NODE(innode);
	virt = virtTable[node->_nodeType];

	/* first time through; make the intern structure for this polyrep node */
	if(!node->_intern) {

		int i;

		node->_intern = MALLOC(struct X3D_PolyRep *, sizeof(struct X3D_PolyRep));

		polyrep = (struct X3D_PolyRep *)node->_intern;
		polyrep->ntri = -1;
		polyrep->cindex = 0; polyrep->actualCoord = 0; polyrep->colindex = 0; polyrep->color = 0;
		polyrep->norindex = 0; polyrep->normal = 0; polyrep->GeneratedTexCoords = 0;
		polyrep->tcindex = 0; 
		polyrep->tcoordtype = 0;
		polyrep->streamed = FALSE;

		/* for Collision, default texture generation */
		polyrep->minVals[0] =  999999.9f;
		polyrep->minVals[1] =  999999.9f;
		polyrep->minVals[2] =  999999.9f;
		polyrep->maxVals[0] =  -999999.9f;
		polyrep->maxVals[1] =  -999999.9f;
		polyrep->maxVals[2] =  -999999.9f;

		for (i=0; i<VBO_COUNT; i++) polyrep->VBO_buffers[i] = 0;
		if (global_use_VBOs) {
			/* printf ("generating buffers for node %p, type %s\n",p,stringNodeType(p->_nodeType)); */
			glGenBuffers(1,&polyrep->VBO_buffers[VERTEX_VBO]);
			glGenBuffers(1,&polyrep->VBO_buffers[INDEX_VBO]);

			/* printf ("they are %u %u %u %u\n",polyrep->VBO_buffers[0],polyrep->VBO_buffers[1],polyrep->VBO_buffers[2],polyrep->VBO_buffers[3]); */
		}


	}
	polyrep = (struct X3D_PolyRep *)node->_intern;
	/* if multithreading, tell the rendering loop that we are regenning this one */
	/* if singlethreading, this'll be set to TRUE before it is tested	     */
	polyrep->streamed = FALSE;

	FREE_IF_NZ(polyrep->cindex);
	FREE_IF_NZ(polyrep->actualCoord);
	FREE_IF_NZ(polyrep->GeneratedTexCoords);
	FREE_IF_NZ(polyrep->colindex);
	FREE_IF_NZ(polyrep->color);
	FREE_IF_NZ(polyrep->norindex);
	FREE_IF_NZ(polyrep->normal);
	FREE_IF_NZ(polyrep->tcindex);


	/* make the node by calling the correct method */
	virt->mkpolyrep(node);

	/* now, put the generic internal structure into OpenGL arrays for faster rendering */
	/* if there were errors, then rep->ntri should be 0 */
	if (polyrep->ntri != 0)
		stream_polyrep(node, coord, color, normal, texCoord);



	/* and, tell the rendering process that this shape is now compiled */
	polyrep->irep_change = node->_change;
}

