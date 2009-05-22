/*
=INSERT_TEMPLATE_HERE=

$Id: Children.c,v 1.11 2009/05/22 16:18:40 crc_canada Exp $

Render the children of nodes.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h" 
#include "quaternion.h"

#include "Collision.h"
#include "RenderFuncs.h"


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
			flag, p->_nparents);  */
	
	

	p->_renderFlags = p->_renderFlags | flag;

	for (i = 0; i < p->_nparents; i++) {
		/* printf ("node %d has %d for a parent\n",p,p->_parents[i]); */
		update_renderFlag(p->_parents[i],flag);
	}
	/* printf ("finished update_RenderFlag for %d\n",p);  */
}
