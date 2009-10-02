/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Shape.h,v 1.1 2009/10/02 21:34:53 crc_canada Exp $

Proximity sensor macro.

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



#ifndef __FREEWRL_SCENEGRAPH_SHAPE_H__
#define __FREEWRL_SCENEGRAPH_SHAPE_H__

struct matpropstruct {
	float transparency;
};

extern struct matpropstruct appearanceProperties;

#define RENDER_MATERIAL_SUBNODES(which) \
	{ void *tmpN;   \
		POSSIBLE_PROTO_EXPANSION(which,tmpN) \
       		if(tmpN) { \
			render_node(tmpN); \
       		} else { \
			/* no material, so just colour the following shape */ \
	       		/* Spec says to disable lighting and set coloUr to 1,1,1 */ \
	       		LIGHTING_OFF  \
       			glColor3f(1,1,1); \
 \
			/* tell the rendering passes that this is just "normal" */ \
			last_texture_type = NOTEXTURE; \
			/* same with materialProperties.transparency */ \
			appearanceProperties.transparency=0.99999; \
		} \
	}

#endif /* __FREEWRL_SCENEGRAPH_SHAPE_H__ */
