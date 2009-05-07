/*
=INSERT_TEMPLATE_HERE=

$Id: Children.c,v 1.8 2009/05/07 20:03:20 crc_canada Exp $

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


/* this grouping node has a DirectionalLight for a child, render this first */
void dirlightChildren(struct Multi_Node ch) {
	int i;

	/* glPushAttrib(GL_LIGHTING_BIT|GL_ENABLE_BIT); */
	for(i=0; i<ch.n; i++) {
		struct X3D_Node *p = X3D_NODE(ch.p[i]);
		if (p != NULL) {
			if (p->_nodeType == NODE_DirectionalLight) 
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
			if (p->_nodeType != NODE_DirectionalLight) {
			/*printf ("normalchildren, (%d of %d) child %d\n",i,ch.n,p);   */
			/*			if ((p->PIV) > 0) */
				render_node(p);
			}
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
