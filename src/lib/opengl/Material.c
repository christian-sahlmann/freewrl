/*
=INSERT_TEMPLATE_HERE=

$Id: Material.c,v 1.31 2012/08/05 20:52:25 dug9 Exp $

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
static GLfloat default_diffuse[]  = {0.8f,0.8f,0.8f,1.0f};
static GLfloat default_ambient[]  = {0.2f,0.2f,0.2f,1.0f};
static GLfloat default_specular[] = {0.0f,0.0f,0.0f,1.0f};
static GLfloat default_emission[] = {0.0f,0.0f,0.0f,1.0f};


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


#ifdef OLDCODE
OLDCODE#include "../scenegraph/Component_Shape.h"
OLDCODE
OLDCODE/* for OpenGL ES, we mimic the old glColor stuff from fixed functionality */
OLDCODE#ifdef GL_ES_VERSION_2_0
OLDCODEvoid glColor3d (double r, double g, double b) {
OLDCODE    float cols[3];
OLDCODE    cols[0] = (float)r;
OLDCODE    cols[1] = (float)g;
OLDCODE    cols[2] = (float)b;
OLDCODE    
OLDCODE//printf ("... active shader %d, for ",getAppearanceProperties()->currentShader);
OLDCODE//printf ("glColor3d %lf %lf %lf\n",r,g,b);
OLDCODE    s_shader_capabilities_t *me = getAppearanceProperties()->currentShaderProperties;
OLDCODE    
OLDCODE    if (me != NULL) {
OLDCODE        if (me->myMaterialColour != -1) {
OLDCODE            GLUNIFORM3FV(me->myMaterialColour,1,cols);
OLDCODE        } else {
OLDCODE            ConsoleMessage ("glColor3d called; no shader property to send to");
OLDCODE        }
OLDCODE    }
OLDCODE
OLDCODE}
OLDCODE
OLDCODEvoid glColor3dv (double *dcols) {
OLDCODE    float cols[3];
OLDCODE    cols[0] = (float)dcols[0];
OLDCODE    cols[1] = (float)dcols[1];
OLDCODE    cols[2] = (float)dcols[2];
OLDCODE
OLDCODE//printf ("... active shader %d, for ",getAppearanceProperties()->currentShader);
OLDCODE//printf ("glColor3dv %lf %lf %lf\n",cols[0],cols[1],cols[2]);
OLDCODE    
OLDCODE    s_shader_capabilities_t *me = getAppearanceProperties()->currentShaderProperties;
OLDCODE    
OLDCODE    if (me != NULL) {
OLDCODE        if (me->myMaterialColour != -1) {
OLDCODE            GLUNIFORM3FV(me->myMaterialColour,1,cols);
OLDCODE        } else {
OLDCODE            ConsoleMessage ("glColor3dv called; no shader property to send to");
OLDCODE        }
OLDCODE    }
OLDCODE
OLDCODE}
OLDCODE
OLDCODEvoid glColor3fv (float *cols) {
OLDCODE//printf ("... active shader %d, for ",getAppearanceProperties()->currentShader);
OLDCODE//printf ("glColor3fv %f %f %f\n",cols[0],cols[1],cols[2]);
OLDCODE    s_shader_capabilities_t *me = getAppearanceProperties()->currentShaderProperties;
OLDCODE    
OLDCODE    if (me != NULL) {
OLDCODE    if (me->myMaterialColour != -1) {
OLDCODE     GLUNIFORM3FV(me->myMaterialColour,1,cols);
OLDCODE    } else {
OLDCODE        ConsoleMessage ("glColor3fv called; no shader property to send to");
OLDCODE    }
OLDCODE    }
OLDCODE    
OLDCODE}
OLDCODE
OLDCODEvoid glColor4fv (float *cols) {
OLDCODE    
OLDCODE    // ignore the alpha
OLDCODE    
OLDCODE//printf ("... active shader %d, for ",getAppearanceProperties()->currentShader);
OLDCODE//printf ("glColor4fv %lf %lf %lf %lf\n",cols[0],cols[1],cols[2],cols[3]);
OLDCODE    s_shader_capabilities_t *me = getAppearanceProperties()->currentShaderProperties;
OLDCODE    
OLDCODE    if (me != NULL) {
OLDCODE        if (me->myMaterialColour != -1) {
OLDCODE            GLUNIFORM3FV(me->myMaterialColour,
OLDCODE                         1,cols);
OLDCODE    } else {
OLDCODE        ConsoleMessage ("glColor4fv called; no shader property to send to");
OLDCODE
OLDCODE    }
OLDCODE    }
OLDCODE}
OLDCODE
OLDCODEvoid glColorMaterial (GLenum face, GLenum mode) {
OLDCODE#ifndef GLES2
OLDCODEprintf ("... active shader %d, for ",getAppearanceProperties()->currentShaderProperties->myShaderProgram);
OLDCODEprintf ("glColorMaterial %x, %d\n",face,mode);
OLDCODE#endif
OLDCODE}
OLDCODE
OLDCODEvoid glMaterialf (GLenum face, GLenum pname, float param) {
OLDCODE#ifndef GLES2
OLDCODEprintf ("... active shader %d, for ",getAppearanceProperties()->currentShaderProperties->myShaderProgram);
OLDCODEprintf ("glMaterialf, face %d pname %d (GL_SHININESS == %d), param %f\n",
OLDCODE	face,pname,GL_SHININESS,param);
OLDCODE#endif
OLDCODE}
OLDCODE
OLDCODEvoid glMaterialfv (GLenum face, GLenum pname, float *param) {
OLDCODE#ifndef GLES2
OLDCODEprintf ("... active shader %d, for ",getAppearanceProperties()->currentShaderProperties->myShaderProgram);
OLDCODEprintf ("glMaterialfv, face %d pname %d (GL_SHININESS == %d), param %f\n",
OLDCODE	face,pname,GL_SHININESS,*param);
OLDCODE#endif
OLDCODE}
OLDCODE#endif
#endif //OLDCODE

