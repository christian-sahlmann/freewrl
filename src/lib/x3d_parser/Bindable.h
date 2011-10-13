/*
=INSERT_TEMPLATE_HERE=

$Id: Bindable.h,v 1.11 2011/10/13 16:14:58 crc_canada Exp $

Bindable nodes - Background, TextureBackground, Fog, NavigationInfo, Viewpoint.

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


#ifndef __FREEWRL_BINDABLE_H__
#define __FREEWRL_BINDABLE_H__


/* Bind stack */

void
reset_upvector(void);

void
set_naviinfo(struct X3D_NavigationInfo *node);

void
send_bind_to(struct X3D_Node *node, int value);

void
bind_node(struct X3D_Node *node, struct Vector *stack);

void
render_Fog(struct X3D_Fog *node);

void
render_NavigationInfo(struct X3D_NavigationInfo *node);

void render_Background(struct X3D_Background *node);
void render_TextureBackground(struct X3D_TextureBackground *node);

void set_naviWidthHeightStep(double wid, double hei, double step) ;

#endif /* __FREEWRL_BINDABLE_H__ */
