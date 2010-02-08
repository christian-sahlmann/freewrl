/*
=INSERT_TEMPLATE_HERE=

$Id: Children.c,v 1.15 2010/02/08 19:51:01 crc_canada Exp $

Render the children of nodes.

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

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h" 
#include "quaternion.h"

#include "Children.h"
#include "Collision.h"
#include "RenderFuncs.h"
#include "../opengl/Frustum.h"


#define VF_localLight				0x0004

/* this grouping node has a local light for a child, render this first */
void localLightChildren(struct Multi_Node ch) {
	int i;
	for(i=0; i<ch.n; i++) {
		struct X3D_Node *p = X3D_NODE(ch.p[i]);
		if (p != NULL) {
			if ((p->_nodeType == NODE_DirectionalLight) ||
				(p->_nodeType == NODE_PointLight) ||
				(p->_nodeType == NODE_SpotLight))
				render_node(p);
		}
	}
}

/* render all children, except the directionalight ones */
void normalChildren(struct Multi_Node ch) {
	int i;

	for(i=0; i<ch.n; i++) {
		struct X3D_Node *p = X3D_NODE(ch.p[i]);
		if (p != NULL) {
			/* printf ("child %d of %d is a %s\n",i,ch.n,stringNodeType(p->_nodeType)); */
			/* as long as this is not a local light... if it is, it will be handled by
			   the localLightChildren function, above */
			if (p->_nodeType == NODE_DirectionalLight) {
				if (X3D_DIRECTIONALLIGHT(p)->global == TRUE) render_node(p);
			} else if (p->_nodeType == NODE_SpotLight) {
				if (X3D_SPOTLIGHT(p)->global == TRUE) render_node(p);
			} else if (p->_nodeType == NODE_PointLight) {
				if (X3D_POINTLIGHT(p)->global == TRUE) render_node(p);
			} else render_node(p);
		}
	}
}

/* propagate flags up the scene graph */
/* used to tell the rendering pass that, there is/used to be nodes
 * of interest down the branch. Eg, Transparent nodes - no sense going
 * through it all when rendering only for nodes. */

void update_renderFlag (struct X3D_Node *p, int flag) {
	int i;

	/* send notification up the chain */
	
	/* printf ("start of update_RenderFlag for %d (%s) flag %x parents %d\n",p, stringNodeType(p->_nodeType),
			flag, p->_nparents); */
	

	p->_renderFlags = p->_renderFlags | flag;

	for (i = 0; i < p->_nparents; i++) {
		/* printf ("node %d has %d for a parent\n",p,p->_parents[i]);  */
		switch (X3D_NODE(p->_parents[i])->_nodeType) {

			case NODE_Switch:
				if (is_Switchchild_inrange(X3D_SWITCH(p->_parents[i]),p)) {
					/* printf ("switch, this is the chosen node\n"); */
					update_renderFlag(p->_parents[i],flag);
				}
				break;

			case NODE_LOD:
				/* works for both X3D and VRML syntax; compare with the "_selected" field */
				if (p == X3D_LODNODE(p->_parents[i])->_selected) {
					update_renderFlag(p->_parents[i],flag);
				}
				break;

			case NODE_GeoLOD:
				if (is_GeoLODchild_inrange(X3D_GEOLOD(p->_parents[i]),p)) {
					/* printf ("switch, this is the chosen node\n"); */
					update_renderFlag(p->_parents[i],flag);
				}
				break;

			default:

				update_renderFlag(p->_parents[i],flag);
		}
	}
	/* printf ("finished update_RenderFlag for %d\n",p); */
}
