/*
=INSERT_TEMPLATE_HERE=

$Id: Material.c,v 1.26 2011/06/30 15:13:21 crc_canada Exp $

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
GLfloat default_diffuse[]  = {0.8f,0.8f,0.8f,1.0f};
GLfloat default_ambient[]  = {0.2f,0.2f,0.2f,1.0f};
GLfloat default_specular[] = {0.0f,0.0f,0.0f,1.0f};
GLfloat default_emission[] = {0.0f,0.0f,0.0f,1.0f};

/* bounds check and do the shininess calculations */
void do_shininess (GLenum face, float shininess) {
	/* which should it be? From the spec:
		"Lower shininess values produce soft glows, while higher values result in sharper, smaller highlights."
	so, we either do 1.0-shininess * 128, or we do shininess * 128... */

	/* shininess = (1.0 - shininess) * 128.0; */
	shininess *= 128.0f;

	/* printf ("do_shininess, %f for face %d, GL_FRONT %d, BACK %d F&B %d\n",shininess, face, GL_FRONT,GL_BACK,GL_FRONT_AND_BACK);  */

#define MAX_SHIN 128.0f
#define MIN_SHIN 0.01f
	if ((shininess > MAX_SHIN) || (shininess < MIN_SHIN)) {
		if (shininess>MAX_SHIN){shininess = MAX_SHIN;}else{shininess=MIN_SHIN;}
	}

	FW_GL_MATERIALF(face, GL_SHININESS, (float)shininess);
}


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

void fwglMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
	if(usingAnaglyph2())
		switch(pname)
		{
			case GL_DIFFUSE:
			case GL_AMBIENT:
			case GL_SPECULAR:
			case GL_EMISSION:
				{
					float gray, p2[4];
					gray = .299F*params[0] + .587F*params[1] + .114F*params[2];
					p2[0] = p2[1] = p2[2] = gray;
					p2[3] = params[3];
					//fwColorRemapf(&p2[0],&p2[1],&p2[2],params[0],params[1],params[2]);
					glMaterialfv(face,pname,p2); 
				}
				break;
			default:
				glMaterialfv(face,pname,params);
		}
	else
		glMaterialfv(face,pname,params);
}

void fwglColor3fv(float *color)
{
	if(usingAnaglyph2())
	{
		float gray, ccc[3];
		memcpy(ccc,color,3*sizeof(float));
		gray = ccc[0]*.299F + ccc[1]*.587F + ccc[2]*.144F;
		ccc[0] = ccc[1] = ccc[2] = gray;
		glColor3fv(ccc);
	}
	else
		glColor3fv(color);

}
void fwglColor4fv(float *rgba)
{
	if(usingAnaglyph2())
	{
		float gray, ccc[4];
		memcpy(ccc,rgba,4*sizeof(float));
		gray = ccc[0]*.299F + ccc[1]*.587F + ccc[2]*.144F;
		ccc[0] = ccc[1] = ccc[2] = gray;
		glColor4fv(ccc);
	}
	else
		glColor4fv(rgba);
}

void fwglColor3d(double r, double g, double b)
{
	if(usingAnaglyph2())
	{
		double gray, ccc[3];
		ccc[0] = r;
		ccc[1] = g;
		ccc[2] = b;
		gray = ccc[0]*.299 + ccc[1]*.587 + ccc[2]*.144;
		ccc[0] = ccc[1] = ccc[2] = gray;
		glColor3dv(ccc);
	}
	else
		glColor3d(r,g,b);
}

void fwglColor3f(float r, float g, float b)
{
	float ccc[3];
	ccc[0] = r;
	ccc[1] = g;
	ccc[2] = b;
	fwglColor3fv(ccc);
}
void fwglColor4f(float r, float g, float b, float a)
{
	float ccc[4];
	ccc[0] = r;
	ccc[1] = g;
	ccc[2] = b;
	ccc[3] = a;
	fwglColor4fv(ccc);
}


void do_glMaterialfv (GLenum face, GLenum pname, GLfloat *param) {

	/* for IndexedLineSet, etc, we keep the last emissiveColor around */
	FW_GL_MATERIALFV(face,pname,param);
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

#include "Component_Shape.h"

/* for OpenGL ES, we mimic the old glColor stuff from fixed functionality */
#ifdef GL_ES_VERSION_2_0
void glColor3d (double r, double g, double b) {
    float cols[3];
    cols[0] = (float)r;
    cols[1] = (float)g;
    cols[2] = (float)b;
    
//printf ("... active shader %d, for ",getAppearanceProperties()->currentShader);
//printf ("glColor3d %lf %lf %lf\n",r,g,b);
    
    if (getAppearanceProperties()->currentShaderProperties != NULL) {
        if (getAppearanceProperties()->currentShaderProperties->myMaterialColour != -1) {
            GLUNIFORM3FV(getAppearanceProperties()->currentShaderProperties->myMaterialColour,
                         1,cols);
        } else {
            ConsoleMessage ("glColor3d called; no shader property to send to");
        }
    }

}

void glColor3dv (double *dcols) {
    float cols[3];
    cols[0] = (float)dcols[0];
    cols[1] = (float)dcols[1];
    cols[2] = (float)dcols[2];

//printf ("... active shader %d, for ",getAppearanceProperties()->currentShader);
//printf ("glColor3dv %lf %lf %lf\n",cols[0],cols[1],cols[2]);
    
    if (getAppearanceProperties()->currentShaderProperties != NULL) {
        if (getAppearanceProperties()->currentShaderProperties->myMaterialColour != -1) {
            GLUNIFORM3FV(getAppearanceProperties()->currentShaderProperties->myMaterialColour,
                         1,cols);
        } else {
            ConsoleMessage ("glColor3dv called; no shader property to send to");
        }
    }

}

void glColor3fv (float *cols) {
//printf ("... active shader %d, for ",getAppearanceProperties()->currentShader);
//printf ("glColor3fv %f %f %f\n",cols[0],cols[1],cols[2]);
    if (getAppearanceProperties()->currentShaderProperties != NULL) {
    if (getAppearanceProperties()->currentShaderProperties->myMaterialColour != -1) {
     GLUNIFORM3FV(getAppearanceProperties()->currentShaderProperties->myMaterialColour,
                  1,cols);
    } else {
        ConsoleMessage ("glColor3fv called; no shader property to send to");
    }
    }
    
}

void glColor4fv (float *cols) {
    
    // ignore the alpha
    
//printf ("... active shader %d, for ",getAppearanceProperties()->currentShader);
//printf ("glColor4fv %lf %lf %lf %lf\n",cols[0],cols[1],cols[2],cols[3]);
    if (getAppearanceProperties()->currentShaderProperties != NULL) {
        if (getAppearanceProperties()->currentShaderProperties->myMaterialColour != -1) {
            GLUNIFORM3FV(getAppearanceProperties()->currentShaderProperties->myMaterialColour,
                         1,cols);
    } else {
        ConsoleMessage ("glColor4fv called; no shader property to send to");

    }
    }
}

void glColorMaterial (GLenum face, GLenum mode) {
printf ("... active shader %d, for ",getAppearanceProperties()->currentShader);
printf ("glColorMaterial %x, %d\n",face,mode);
}

void glMaterialf (GLenum face, GLenum pname, float param) {
printf ("... active shader %d, for ",getAppearanceProperties()->currentShader);
printf ("glMaterialf, face %d pname %d (GL_SHININESS == %d), param %f\n",
	face,pname,GL_SHININESS,param);
}

void glMaterialfv (GLenum face, GLenum pname, float *param) {
printf ("... active shader %d, for ",getAppearanceProperties()->currentShader);
printf ("glMaterialfv, face %d pname %d (GL_SHININESS == %d), param %f\n",
	face,pname,GL_SHININESS,*param);
}
#endif

