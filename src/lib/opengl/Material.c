/*
=INSERT_TEMPLATE_HERE=

$Id: Material.c,v 1.30 2012/07/31 15:19:39 crc_canada Exp $

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

#ifdef OLDCODE
OLDCODE/* bounds check and do the shininess calculations */
OLDCODEvoid do_shininess (GLenum face, float shininess) {
OLDCODE	/* which should it be? From the spec:
OLDCODE		"Lower shininess values produce soft glows, while higher values result in sharper, smaller highlights."
OLDCODE	so, we either do 1.0-shininess * 128, or we do shininess * 128... */
OLDCODE
OLDCODE	/* shininess = (1.0 - shininess) * 128.0; */
OLDCODE	shininess *= 128.0f;
OLDCODE
OLDCODE	/* printf ("do_shininess, %f for face %d, GL_FRONT %d, BACK %d F&B %d\n",shininess, face, GL_FRONT,GL_BACK,GL_FRONT_AND_BACK);  */
OLDCODE
OLDCODE#define MAX_SHIN 128.0f
OLDCODE#define MIN_SHIN 0.01f
OLDCODE	if ((shininess > MAX_SHIN) || (shininess < MIN_SHIN)) {
OLDCODE		if (shininess>MAX_SHIN){shininess = MAX_SHIN;}else{shininess=MIN_SHIN;}
OLDCODE	}
OLDCODE
OLDCODE	FW_GL_MATERIALF(face, GL_SHININESS, (float)shininess);
OLDCODE}
#endif //OLDCODE


void fwAnaglyphRemapf(float *r2, float *g2, float* b2, float r, float g, float b)
{
	float gray = .299F*r + .587F*g + .114F*b;
	*r2 = *g2 = *b2 = gray;
}
void fwAnaglyphremapRgbav(unsigned char *rgba,int y,int x)
{
	int i,j;
	/* convert bitmap to grayscale Q. is there a way to use a shader program to do this faster?
	   Q. if all -win32,osx,linux- marked/flagged images when they are originally grayscale, could we use the 
	   flag here to skip the conversion to gray?
	*/
	for(j=0;j<y;j++)
	{	
		for(i=0;i<x;i++)
		{
			int igray;
			unsigned char *rgb = &rgba[(j*x + i)*4]; /* assume RGBA */
			igray = rgb[0]*76 + rgb[1]*150 + rgb[2]*29; //255 = 76 + 150 + 29
			igray = igray >> 8; 
			rgb[0] = rgb[1] = rgb[2] = (unsigned char) igray;
		}
	}
}
//void fwAnaglyphRemapc(unsigned char *r2, unsigned char *g2, unsigned char *b2, unsigned char r, unsigned char g, unsigned char b)
//{
//	//gray = rgb[0]*.299 + rgb[1]*.587 + rgb[2]*.114;
//	int igray = r*76 + g*150 + b*29; //255 = 76 + 150 + 29
//	igray = igray >> 8; 
//	*r2 = *g2 = *b2 = (unsigned char) igray;
//}

#ifdef OLDCODE
OLDCODEvoid fwglMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
OLDCODE{
OLDCODE	if(usingAnaglyph2())
OLDCODE		switch(pname)
OLDCODE		{
OLDCODE			case GL_DIFFUSE:
OLDCODE			case GL_AMBIENT:
OLDCODE			case GL_SPECULAR:
OLDCODE			case GL_EMISSION:
OLDCODE				{
OLDCODE					float gray, p2[4];
OLDCODE					gray = .299F*params[0] + .587F*params[1] + .114F*params[2];
OLDCODE					p2[0] = p2[1] = p2[2] = gray;
OLDCODE					p2[3] = params[3];
OLDCODE					//fwColorRemapf(&p2[0],&p2[1],&p2[2],params[0],params[1],params[2]);
OLDCODE					glMaterialfv(face,pname,p2); 
OLDCODE				}
OLDCODE				break;
OLDCODE			default:
OLDCODE				glMaterialfv(face,pname,params);
OLDCODE		}
OLDCODE	else
OLDCODE		glMaterialfv(face,pname,params);
OLDCODE}
OLDCODE
OLDCODEvoid fwglColor3fv(float *color,char *wh, int li)
OLDCODE{
OLDCODEConsoleMessage("fwglColor3f, at %s:%d",wh,li);
OLDCODE
OLDCODE	if(usingAnaglyph2())
OLDCODE	{
OLDCODE		float gray, ccc[3];
OLDCODE		memcpy(ccc,color,3*sizeof(float));
OLDCODE		gray = ccc[0]*.299F + ccc[1]*.587F + ccc[2]*.144F;
OLDCODE		ccc[0] = ccc[1] = ccc[2] = gray;
OLDCODE		glColor3fv(ccc);
OLDCODE	}
OLDCODE	else
OLDCODE		glColor3fv(color);
OLDCODE
OLDCODE}
OLDCODEvoid fwglColor4fv(float *rgba)
OLDCODE{
OLDCODE	if(usingAnaglyph2())
OLDCODE	{
OLDCODE		float gray, ccc[4];
OLDCODE		memcpy(ccc,rgba,4*sizeof(float));
OLDCODE		gray = ccc[0]*.299F + ccc[1]*.587F + ccc[2]*.144F;
OLDCODE		ccc[0] = ccc[1] = ccc[2] = gray;
OLDCODE		glColor4fv(ccc);
OLDCODE	}
OLDCODE	else
OLDCODE		glColor4fv(rgba);
OLDCODE}
OLDCODE
OLDCODEvoid fwglColor3d(double r, double g, double b)
OLDCODE{
OLDCODE	if(usingAnaglyph2())
OLDCODE	{
OLDCODE		double gray, ccc[3];
OLDCODE		ccc[0] = r;
OLDCODE		ccc[1] = g;
OLDCODE		ccc[2] = b;
OLDCODE		gray = ccc[0]*.299 + ccc[1]*.587 + ccc[2]*.144;
OLDCODE		ccc[0] = ccc[1] = ccc[2] = gray;
OLDCODE		glColor3dv(ccc);
OLDCODE	}
OLDCODE	else
OLDCODE		glColor3d(r,g,b);
OLDCODE}
OLDCODE
OLDCODEvoid fwglColor3f(float r, float g, float b,char *wh, int li)
OLDCODE{
OLDCODE	float ccc[3];
OLDCODE	ccc[0] = r;
OLDCODE	ccc[1] = g;
OLDCODE	ccc[2] = b;
OLDCODE	fwglColor3fv(ccc,wh,li);
OLDCODE}
OLDCODEvoid fwglColor4f(float r, float g, float b, float a)
OLDCODE{
OLDCODE	float ccc[4];
OLDCODE	ccc[0] = r;
OLDCODE	ccc[1] = g;
OLDCODE	ccc[2] = b;
OLDCODE	ccc[3] = a;
OLDCODE	fwglColor4fv(ccc);
OLDCODE}
OLDCODE
OLDCODE
OLDCODEvoid do_glMaterialfv (GLenum face, GLenum pname, GLfloat *param) {
OLDCODE
OLDCODE	/* for IndexedLineSet, etc, we keep the last emissiveColor around */
OLDCODE	FW_GL_MATERIALFV(face,pname,param);
OLDCODE}
OLDCODE
#endif //OLDCODE


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

