/*
=INSERT_TEMPLATE_HERE=

$Id: Material.h,v 1.2 2009/10/01 19:35:36 crc_canada Exp $

Global includes.

*/

/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2009 CRC Canada. (http://www.crc.gc.ca)

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FreeWRL/FreeX3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FreeWRL/FreeX3D.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/


#ifndef __FREEWRL_MATERIAL_H__
#define __FREEWRL_MATERIAL_H__


#define DEFAULT_COLOUR_POINTER  \
        GLfloat defColor[] = {1.0, 1.0, 1.0};\
        GLfloat *thisColor;

#define GET_COLOUR_POINTER \
		/* is there an emissiveColor here??? */ \
		if (lightingOn) { \
			thisColor = last_emission; \
		} else { \
			thisColor = defColor; \
		}

#define DO_COLOUR_POINTER                glColor3fv (thisColor);


extern GLfloat last_emission[4];


void do_shininess (GLenum face, float shininess);
void do_glMaterialfv (GLenum face, GLenum pname, GLfloat *param);
int verify_rotate(GLfloat *params);
int verify_translate(GLfloat *params);
int verify_scale(GLfloat *params);

#endif /* __FREEWRL_MATERIAL_H__ */
