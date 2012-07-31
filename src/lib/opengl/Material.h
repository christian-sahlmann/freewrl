/*
=INSERT_TEMPLATE_HERE=

$Id: Material.h,v 1.9 2012/07/31 15:19:39 crc_canada Exp $

Global includes.

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


#ifndef __FREEWRL_MATERIAL_H__
#define __FREEWRL_MATERIAL_H__


#ifdef OLDCODE
OLDCODE#define DEFAULT_COLOUR_POINTER  \
OLDCODE        GLfloat defColor[] = {1.0f, 1.0f, 1.0f};\
OLDCODE        GLfloat *thisColor;
OLDCODE
OLDCODE#define GET_COLOUR_POINTER \
OLDCODE		/* is there an emissiveColor here??? */ \
OLDCODE		if (gglobal()->RenderFuncs.lightingOn) { \
OLDCODE			thisColor = getAppearanceProperties()->emissionColour; \
OLDCODE		} else { \
OLDCODE			thisColor = defColor; \
OLDCODE		}
OLDCODE
OLDCODE#define DO_COLOUR_POINTER                FW_GL_COLOR3FV (thisColor);
#endif //OLDCODE


//OLDCODE void do_shininess (GLenum face, float shininess);
//OLDCODE void do_glMaterialfv (GLenum face, GLenum pname, GLfloat *param);
int verify_rotate(GLfloat *params);
int verify_translate(GLfloat *params);
int verify_scale(GLfloat *params);

#endif /* __FREEWRL_MATERIAL_H__ */
