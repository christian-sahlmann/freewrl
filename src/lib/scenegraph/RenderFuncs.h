/*
=INSERT_TEMPLATE_HERE=

$Id: RenderFuncs.h,v 1.7 2010/01/27 21:18:52 crc_canada Exp $

Proximity sensor macro.

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


#ifndef __FREEWRL_SCENEGRAPH_RENDERFUNCS_H__
#define __FREEWRL_SCENEGRAPH_RENDERFUNCS_H__

/* for X3D_Node */
#include "../vrml_parser/Structs.h"

/* function protos */
int nextlight(void);
void render_node(struct X3D_Node *node);

extern int BrowserAction;
extern struct X3D_Anchor *AnchorsAnchor;
extern char *OSX_replace_world_from_console;


void lightState(GLint light, int status);
void saveLightState(int *ls);
void restoreLightState(int *ls);
void fwglLightfv (int light, int pname, GLfloat *params);
void fwglLightf (int light, int pname, GLfloat param);
void initializeLightTables(void);
void propagateLightingInfo(void);


#endif /* __FREEWRL_SCENEGRAPH_RENDERFUNCS_H__ */
