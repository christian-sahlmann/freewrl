/*
=INSERT_TEMPLATE_HERE=

$Id: NormalCalcs.c,v 1.9 2012/08/15 15:00:29 crc_canada Exp $

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

#include "LinearAlgebra.h"


void fwnorprint (float *norm) {
		printf ("normals %f %f %f\n",norm[0],norm[1],norm[2]);
}

void normalize_ifs_face (float *point_normal,
			 struct point_XYZ *facenormals,
			 int *pointfaces,
			int mypoint,
			int curpoly,
			float creaseAngle) {

	/* IndexedFaceSet (and possibly sometime, others)
	   normal generator

	  Passed in:
		point_normal	- where to put the calculated normal
		facenormals	- normals of each face of a polygon
		pointfaces	- each point - which face(s) is it part of
		mypoint		- which point are we looking at
		curpoly		- which poly (face) we are working on
		creaseAngle	- creaseAngle of polygon
	*/
	int tmp_a;
	int tmp_b;
	int facecount;
	float zz;
	struct point_XYZ temp;
    bool foundInOtherFaces = false;

	point_normal[0] = 0.0f; point_normal[1] = 0.0f; point_normal[2] = 0.0f;

    	//printf ("\nstart normalize_ifs_face\n");
	//printf ("my normal is %f %f %f\n", facenormals[curpoly].x,facenormals[curpoly].y,facenormals[curpoly].z);

	/* short cut for a point in only 1 face */
	if (pointfaces[mypoint*POINT_FACES] == 1) {
		point_normal[0]=(float) facenormals[curpoly].x;
		point_normal[1]=(float) facenormals[curpoly].y;
		point_normal[2]=(float) facenormals[curpoly].z;
        	//printf ("normalize_ifs_face: quick return normalized vector is %f %f %f\n",point_normal[0], point_normal[1], point_normal[2]);
		return;
	}

	/* ok, calculate normal */
	facecount = 0;
	for (tmp_b=0; tmp_b<pointfaces[mypoint*POINT_FACES]; tmp_b++) {
		tmp_a = pointfaces[mypoint*POINT_FACES+tmp_b+1];
		 //printf ("comparing myface %d to %d\n",curpoly,tmp_a); 

		if (curpoly == tmp_a) {
			zz = 0.0f;
		} else {
			zz = calc_angle_between_two_vectors(facenormals[curpoly],facenormals[tmp_a] );
		}
		 //printf ("angle between faces is %f, creaseAngle is %f\n",zz,creaseAngle);


		if (zz <= creaseAngle) {
			//printf ("count this one in; adding %f %f %f\n",facenormals[tmp_a].x,facenormals[tmp_a].y,facenormals[tmp_a].z);
            		foundInOtherFaces = true;
			point_normal[0] += (float) facenormals[tmp_a].x;
			point_normal[1] += (float) facenormals[tmp_a].y;
			point_normal[2] += (float) facenormals[tmp_a].z;
		}
	}
    
    // do we have to average this one, or should we just return our original normal?
    if (foundInOtherFaces) {
        temp.x = point_normal[0]; temp.y=point_normal[1]; temp.z=point_normal[2];
        normalize_vector(&temp);
        point_normal[0]=(float) temp.x; point_normal[1]=(float) temp.y; point_normal[2]=(float) temp.z;
    } else {
        //printf ("false alarm - just copy original over");
        point_normal[0]=(float) facenormals[curpoly].x;
		point_normal[1]=(float) facenormals[curpoly].y;
		point_normal[2]=(float) facenormals[curpoly].z;
    }

	//printf ("normalize_ifs_face: normalized vector is %f %f %f\n",point_normal[0], point_normal[1], point_normal[2]);
}
