/*
=INSERT_TEMPLATE_HERE=

$Id: Material.c,v 1.12 2010/01/25 16:27:07 crc_canada Exp $

Only do material settings that "matter" and bounds check all values.

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
#include "Material.h"

#include <libFreeWRL.h>

/* #include "LinearAlgebra.h" */


/* default material properties */
GLfloat default_diffuse[]  = {0.8,0.8,0.8,1.0};
GLfloat default_ambient[]  = {0.2,0.2,0.2,1.0};
GLfloat default_specular[] = {0.0,0.0,0.0,1.0};
GLfloat default_emission[] = {0.0,0.0,0.0,1.0};

GLfloat last_emission[4];

/* bounds check and do the shininess calculations */
void do_shininess (GLenum face, float shininess) {
	/* which should it be? From the spec:
		"Lower shininess values produce soft glows, while higher values result in sharper, smaller highlights."
	so, we either do 1.0-shininess * 128, or we do shininess * 128... */

	/* shininess = (1.0 - shininess) * 128.0; */
	shininess *= 128.0;

	/* printf ("do_shininess, %f for face %d, GL_FRONT %d, BACK %d F&B %d\n",shininess, face, GL_FRONT,GL_BACK,GL_FRONT_AND_BACK);  */

#define MAX_SHIN 128.0
#define MIN_SHIN 0.01
	if ((shininess > MAX_SHIN) || (shininess < MIN_SHIN)) {
		if (shininess>MAX_SHIN){shininess = MAX_SHIN;}else{shininess=MIN_SHIN;}
	}

	FW_GL_MATERIALF(face, GL_SHININESS, (float)shininess);
}

void do_glMaterialfv (GLenum face, GLenum pname, GLfloat *param) {
	int i;

	/* checking should now be done in the compile_ functions */
#ifdef OLDCODE
	for (i=0; i<4; i++) {
		if ((param[i] < 0.0) || (param[i] >1.0)) {
			/* printf ("do_glMaterialfv, pname %d idx %d val %f out of range\n",
					pname,i,param[i]); */
			if (param[i]>1.0) {param[i]=1.0;} else {param[i]=0.0;}
			/* JAS return; bounds check error found, break out */
		}
	}
#endif

	/* for IndexedLineSet, etc, we keep the last emissiveColor around */
	if (pname == GL_EMISSION)
		for (i=0; i<4; i++) {
			last_emission[i] = param[i];
		}
			

	/* compare default values with new */
	FW_GL_MATERIALFV(face,pname,param);
#ifdef TRACK_GL_COLORMATERIAL
	if (pname == GL_DIFFUSE)
	{
		FW_GL_ENABLE(GL_COLOR_MATERIAL);
		FW_GL_COLOR_MATERIAL(face,pname);
		FW_GL_COLOR4FV(param);
	}
#endif

}



int verify_rotate(GLfloat *params) {
	/* angle very close to zero? */
	if (fabs(params[3]) < 0.001) return FALSE;
	return TRUE;
}

int verify_translate(GLfloat *params) {
	/* no translation? */

	if ((fabs(params[0]) < 0.001) &&
		(fabs(params[1]) < 0.001) &&
		(fabs(params[2]) < 0.001))  return FALSE;
	return TRUE;
}


int verify_scale(GLfloat *params) {
	/* no translation? */

	if ((fabs(params[0]-1.0) < 0.001) &&
		(fabs(params[1]-1.0) < 0.001) &&
		(fabs(params[2]-1.0) < 0.001))  return FALSE;

	return TRUE;
}

