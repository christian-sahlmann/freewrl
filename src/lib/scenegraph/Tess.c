/*
=INSERT_TEMPLATE_HERE=

$Id: Tess.c,v 1.14 2010/03/13 02:10:32 sdumoulin Exp $

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


/* JAS */
#ifdef AQUA
typedef GLvoid (*_GLUfuncptr)(GLvoid);
#endif
#if defined(_MSC_VER)
typedef  void (__stdcall *_GLUfuncptr)();
#endif

/* WIN32 p.411 openGL programmers guide - windows needs CALLBACK, unix not */
#ifndef CALLBACK 
#define CALLBACK
#endif


/*********************************************************************
 * General tessellation functions
 *
 * to use the tessellation function, you have to
 * let global_tess_polyrep point towards a structure.
 * global_tess_polyrep->ntri is the first index number which will
 * be filled by the routines (which is the number of triangles
 * already represented in global_tess_polyrep)
 * global_tess_polyrep->cindex and global_tess_polyrep->coords have
 * to point towards enough memory.
 * (And you have to give gluTessVertex a third argument, in which
 * the new coords are written, it has to be a vector of
 * GLDOUBLE s with enough space)
 * After calling gluTessEndPolygon() these vector will be filled.
 * global_tess_polyrep->ntri will contain the absolute
 * number of triangles in global_tess_polyrep after tessellation.
 */

#ifndef IPHONE
GLUtriangulatorObj *global_tessobj;
#else
int global_tessobj;
#endif
struct X3D_PolyRep *global_tess_polyrep=NULL;
int global_IFS_Coords[TESS_MAX_COORDS];
int global_IFS_Coord_count=0;

/* and now all the callback functions, which will be called
	by OpenGL automatically, if the Polygon is specified	*/

void CALLBACK FW_tess_begin(GLenum e) {
               /*printf(" FW_tess_begin   e = %s\n", (e == GL_TRIANGLES ? "GL_TRIANGLES" : "UNKNOWN")); */
		/* we only should get GL_TRIANGLES as type, because
		we defined  the edge_flag callback		*/
		/* check, if the structure is there		*/
	if(e!=GL_TRIANGLES)
		freewrlDie("Something went wrong while tessellating!");
}

void CALLBACK FW_tess_end(void) {
	/*printf("FW_tess_end: Tesselation done.\n"); */
	/* nothing to do	*/
}

void CALLBACK FW_tess_edgeflag(GLenum flag) {
	/*printf("FW_tess_edgeflag: An edge was done (flag = %d).\n", flag); */
	/* nothing to do, this function has to be registered
	so that only GL_TRIANGLES are used	*/
}

void CALLBACK FW_IFS_tess_vertex(void *p) {
	int *dp=(int*)p;

	if (global_IFS_Coord_count == TESS_MAX_COORDS) {
		/* printf ("FW_IFS_tess_vertex, too many coordinates in this face, change TESS_MAX_COORDS\n"); */
		/*
		global_IFS_Coord_count++;
		global_IFS_Coords[global_IFS_Coord_count] =
			global_IFS_Coords[global_IFS_Coord_count-1];
		*/
	} else {
		/*printf ("FW_IFS_tess_vertex, global_ifs_coord count %d, pointer %d\n",global_IFS_Coord_count,*dp);*/
		global_IFS_Coords[global_IFS_Coord_count++] = *dp;
	}

}

void CALLBACK FW_tess_error(GLenum e) {
	/* Prints out tesselation errors. Older versions of at least MESA would
	 give errors, so for now at least, lets just ignore them.
	 printf("FW_tess_error %d: >%s<\n",e,GL_ERROR_MSG); */
}



void CALLBACK FW_tess_combine_data (GLDOUBLE c[3], GLfloat *d[4], GLfloat w[4], void **out,void *polygondata) {
	GLDOUBLE *nv = (GLDOUBLE *) MALLOC(sizeof(GLDOUBLE)*3);
	/* printf("FW_tess_combine data\n"); 
	 printf("combine c:%lf %lf %lf\ndw: %f %f %f %f\n\n",
		c[0],c[1],c[2],w[0],w[1],w[2],w[3]); 
	printf ("vertex 0 %lf %lf %lf, 1 %lf %lf %lf, 2 %lf %lf %lf, 3 %lf %lf %lf\n",
		*d[0]->x,*d[0]->y,*d[0]->z,
		*d[1]->x,*d[1]->y,*d[1]->z,
		*d[2]->x,*d[2]->y,*d[2]->z,
		*d[3]->x,*d[3]->y,*d[3]->z); 

	printf ("d %d %d %d %d\n",d[0],d[1],d[2],d[3]);
	printf ("d %f %f %f %f\n",*d[0],*d[1],*d[2],*d[3]);
	printf ("new coord %d\n",nv);
	*/
	nv[0] = c[0];
	nv[1] = c[1];
	nv[2] = c[2];
	*out = nv;
}


/* Some tesselators will give back garbage. Lets try and remove it */
/* Text handles errors better itself, so this is just used for Extrusions and IndexedFaceSets */
void verify_global_IFS_Coords(int max) {
	int count;

	for (count = 0; count < global_IFS_Coord_count; count++) {
		/*printf ("verifying count %d; val is %d, max %d\n",
				count,global_IFS_Coords[count],max); */
		if ((global_IFS_Coords[count] < 0) ||
			(global_IFS_Coords[count] >= max)) {

			if (count == 0) {
				global_IFS_Coords[count] = 0;
			} else {
				global_IFS_Coords[count] = global_IFS_Coords[count-1];
			}

		}
	}
}

void CALLBACK FW_tess_combine (GLDOUBLE c[3], void *d[4], GLfloat w[4], void **out) {
	GLDOUBLE *nv = (GLDOUBLE *) MALLOC(sizeof(GLDOUBLE)*3);
	/*printf("FW_tess_combine c:%lf %lf %lf\ndw: %f %f %f %f\n\n",
		c[0],c[1],c[2],w[0],w[1],w[2],w[3]); */
	nv[0] = c[0];
	nv[1] = c[1];
	nv[2] = c[2];
	*out = nv;
}


/* next function has to be called once, after an OpenGL context is made
	and before tessellation is started			*/

void new_tessellation(void) {
	global_tessobj=FW_GLU_NEW_TESS();
	if(!global_tessobj)
		freewrlDie("Got no memory for Tessellation Object!");

	/* register the CallBackfunctions				*/
	FW_GLU_TESS_CALLBACK(global_tessobj,GLU_BEGIN,(_GLUfuncptr)FW_tess_begin);
	FW_GLU_TESS_CALLBACK(global_tessobj,GLU_EDGE_FLAG,(_GLUfuncptr)FW_tess_edgeflag);
	FW_GLU_TESS_CALLBACK(global_tessobj,GLU_VERTEX,(_GLUfuncptr)FW_IFS_tess_vertex);
	FW_GLU_TESS_CALLBACK(global_tessobj,GLU_TESS_VERTEX,(_GLUfuncptr)FW_IFS_tess_vertex);
	FW_GLU_TESS_CALLBACK(global_tessobj,GLU_ERROR,(_GLUfuncptr)FW_tess_error);
	FW_GLU_TESS_CALLBACK(global_tessobj,GLU_END,(_GLUfuncptr)FW_tess_end);
	FW_GLU_TESS_CALLBACK(global_tessobj, GLU_TESS_COMBINE_DATA,(_GLUfuncptr)FW_tess_combine_data);
	FW_GLU_TESS_CALLBACK(global_tessobj, GLU_TESS_COMBINE,(_GLUfuncptr)FW_tess_combine);

	    /* Unused right now.
	    FW_GLU_TESS_CALLBACK(triang, GLU_TESS_BEGIN, FW_GLU_TESS_BEGIN);
	    FW_GLU_TESS_CALLBACK(triang, GLU_TESS_BEGIN_DATA,FW_GLU_TESS_BEGIN_DATA);
	    FW_GLU_TESS_CALLBACK(triang, GLU_TESS_EDGE_FLAG,FW_GLU_TESS_EDGE_FLAG);
	    FW_GLU_TESS_CALLBACK(triang, GLU_TESS_EDGE_FLAG_DATA,FW_GLU_TESS_EDGE_FLAG_DATA);
	    FW_GLU_TESS_CALLBACK(triang, GLU_TESS_VERTEX,FW_GLU_TESS_VERTEX);
	    FW_GLU_TESS_CALLBACK(triang, GLU_TESS_VERTEX_DATA,FW_GLU_TESS_VERTEX_DATA);
	    FW_GLU_TESS_CALLBACK(triang, GLU_TESS_END,FW_GLU_TESS_END);
	    FW_GLU_TESS_CALLBACK(triang, GLU_TESS_END_DATA,FW_GLU_TESS_END_DATA);
	    FW_GLU_TESS_CALLBACK(triang, tess_combine_DATA,FW_tess_combine_DATA);
	    FW_GLU_TESS_CALLBACK(triang, GLU_TESS_ERROR,FW_GLU_TESS_ERROR);
	    FW_GLU_TESS_CALLBACK(triang, GLU_TESS_ERROR_DATA,FW_GLU_TESS_ERROR_DATA);
	    */
}

/* next function should be called once at the end, but where?	*/
void destruct_tessellation(void) {
	FW_GLU_DELETETESS(global_tessobj);
	printf("Tessellation Object deleted!\n");
}


