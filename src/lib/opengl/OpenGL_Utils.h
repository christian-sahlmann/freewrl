/*
=INSERT_TEMPLATE_HERE=

$Id: OpenGL_Utils.h,v 1.15 2009/12/09 22:19:11 crc_canada Exp $

Screen snapshot.

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


#ifndef __FREEWRL_OPENGL_UTILS_H__
#define __FREEWRL_OPENGL_UTILS_H__


void start_textureTransform (struct X3D_Node *textureNode, int ttnum);
void end_textureTransform (void);
void markForDispose(struct X3D_Node *node, int recursive);

void
BackEndClearBuffer(int);

void
BackEndLightsOff(void);

void lightState (GLint light, int state);

void drawBBOX(struct X3D_Node *node);


#endif /* __FREEWRL_OPENGL_UTILS_H__ */
