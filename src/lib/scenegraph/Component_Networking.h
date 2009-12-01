/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Networking.h,v 1.4 2009/12/01 21:34:51 crc_canada Exp $

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


#ifndef __FREEWRL_SCENEGRAPH_NETWORKING_H__
#define __FREEWRL_SCENEGRAPH_NETWORKING_H__

/* Inline status */
#define INLINE_INITIAL_STATE 0
#define INLINE_FETCHING_RESOURCE 1
#define INLINE_PARSING 2
#define INLINE_STABLE 10

/* function protos */
void registerReWireNode(struct X3D_Node *node);
void load_Inline (struct X3D_Inline *node);


#endif /* __FREEWRL_SCENEGRAPH_NETWORKING_H__ */
