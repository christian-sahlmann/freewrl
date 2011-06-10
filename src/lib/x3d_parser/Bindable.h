/*
=INSERT_TEMPLATE_HERE=

$Id: Bindable.h,v 1.10 2011/06/10 22:28:33 dug9 Exp $

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
#define MAX_STACK 20

//extern GLint viewPort[];

/* Bindables, Viewpoint, NavigationInfo, Background, TextureBackground and Fog */
//extern void * *fognodes;
//extern void * *backgroundnodes;
//extern void * *navnodes;
//extern void * *viewpointnodes;
//extern int totfognodes, totbacknodes, totnavnodes;//, totviewpointnodes;
//extern int currboundvpno;

//extern int viewpoint_tos;
//extern int background_tos;
//extern int fog_tos;
//extern int navi_tos;
//
//extern uintptr_t viewpoint_stack[];
//extern uintptr_t background_stack[];
//extern uintptr_t navi_stack[];

void
reset_upvector(void);

void
set_naviinfo(struct X3D_NavigationInfo *node);

void
send_bind_to(struct X3D_Node *node, int value);

void
bind_node(struct X3D_Node *node, int *tos, uintptr_t *stack);

void
render_Fog(struct X3D_Fog *node);

void
render_NavigationInfo(struct X3D_NavigationInfo *node);

void render_Background(struct X3D_Background *node);
void render_TextureBackground(struct X3D_TextureBackground *node);

void set_naviWidthHeightStep(double wid, double hei, double step) ;

#endif /* __FREEWRL_BINDABLE_H__ */
