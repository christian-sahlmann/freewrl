/*
=INSERT_TEMPLATE_HERE=

$Id: Material.c,v 1.4 2009/02/25 15:03:17 crc_canada Exp $

Only do material settings that "matter" and bounds check all values.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

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

	glMaterialf (face, GL_SHININESS, (float)shininess);
}

void do_glMaterialfv (GLenum face, GLenum pname, GLfloat *param) {
	int i;

	for (i=0; i<4; i++) {
		if ((param[i] < 0.0) || (param[i] >1.0)) {
			/* printf ("do_glMaterialfv, pname %d idx %d val %f out of range\n",
					pname,i,param[i]); */
			if (param[i]>1.0) {param[i]=1.0;} else {param[i]=0.0;}
			/* JAS return; bounds check error found, break out */
		}
	}

	/* for IndexedLineSet, etc, we keep the last emissiveColor around */
	if (pname == GL_EMISSION)
		for (i=0; i<4; i++) {
			last_emission[i] = param[i];
		}
			

	/* compare default values with new */
if (pname != GL_DIFFUSE)
	glMaterialfv (face,pname,param);
else {
	glMaterialfv (face,pname,param);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial (face,pname);
	glColor4fv(param);
}

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

