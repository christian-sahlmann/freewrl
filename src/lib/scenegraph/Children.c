/*
=INSERT_TEMPLATE_HERE=

$Id: Children.c,v 1.2 2008/11/27 00:27:18 couannette Exp $

Render the children of nodes.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h" /* point_XYZ */
#include "quaternion.h"

#include "Collision.h"


/* sort children - use bubble sort with early exit flag */
void sortChildren (struct Multi_Node ch) {
	int i,j;
	int nc;
	int noswitch;
	struct X3D_Node *a, *b, *c;

	/* simple, inefficient bubble sort */
	/* this is a fast sort when nodes are already sorted;
	   may wish to go and "QuickSort" or so on, when nodes
	   move around a lot. (Bubblesort is bad when nodes
	   have to be totally reversed) */

	nc = ch.n;

	for(i=0; i<nc; i++) {
		noswitch = TRUE;
		for (j=(nc-1); j>i; j--) {
			/* printf ("comparing %d %d\n",i,j); */
			a = X3D_NODE(ch.p[j-1]);
			b = X3D_NODE(ch.p[j]);

			/* check to see if a child is NULL - if so, skip it */
			if ((a != NULL) && (b != NULL)) {
				if (a->_dist > b->_dist) {
					/* printf ("have to switch %d %d\n",i,j); */
					c = a;
					ch.p[j-1] = b;
					ch.p[j] = c;
					noswitch = FALSE;
				}
			}	
		}
		/* did we have a clean run? */
		if (noswitch) {
			break;
		}
	}
	/*
	printf ("sortChild returning.\n");
	for(i=0; i<nc; i++) {
		b = ch.p[i];
		printf ("child %d %d %f %s\n",i,b,b->_dist,stringNodeType(b->_nodeType));
	}
	*/
}

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
