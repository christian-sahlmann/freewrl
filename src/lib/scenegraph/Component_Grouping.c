/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Grouping.c,v 1.2 2008/11/27 00:27:18 couannette Exp $

X3D Grouping Component

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h" /* point_XYZ */
#include "../main/headers.h"

#include "../opengl/OpenGL_Utils.h"


void changed_Transform (struct X3D_Transform *node) { 
	INITIALIZE_EXTENT 
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_Transform, metadata))
	/* printf ("changed Transform for node %u\n",node); */
	node->__do_center = verify_translate ((GLfloat *)node->center.c);
	node->__do_trans = verify_translate ((GLfloat *)node->translation.c);
	node->__do_scale = verify_scale ((GLfloat *)node->scale.c);
	node->__do_rotation = verify_rotate ((GLfloat *)node->rotation.r);
	node->__do_scaleO = verify_rotate ((GLfloat *)node->scaleOrientation.r);
}

/* prep_Group - we need this so that distance (and, thus, distance sorting) works for Groups */
void prep_Group (struct X3D_Group *node) {
	RECORD_DISTANCE
}

/* do transforms, calculate the distance */
void prep_Transform (struct X3D_Transform *node) {
	GLfloat my_rotation;
	GLfloat my_scaleO=0;

        /* rendering the viewpoint means doing the inverse transformations in reverse order (while poping stack),
         * so we do nothing here in that case -ncoder */

	/* printf ("prep_Transform, render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	 render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	/* do we have any geometry visible, and are we doing anything with geometry? */
	OCCLUSIONTEST

	if(!render_vp) {
		fwXformPush();

		/* TRANSLATION */
		if (node->__do_trans)
			glTranslatef(node->translation.c[0],node->translation.c[1],node->translation.c[2]);

		/* CENTER */
		if (node->__do_center)
			glTranslatef(node->center.c[0],node->center.c[1],node->center.c[2]);

		/* ROTATION */
		if (node->__do_rotation) {
			my_rotation = node->rotation.r[3]/3.1415926536*180;
			glRotatef(my_rotation,
				node->rotation.r[0],node->rotation.r[1],node->rotation.r[2]);
		}

		/* SCALEORIENTATION */
		if (node->__do_scaleO) {
			my_scaleO = node->scaleOrientation.r[3]/3.1415926536*180;
			glRotatef(my_scaleO, node->scaleOrientation.r[0],
				node->scaleOrientation.r[1],node->scaleOrientation.r[2]);
		}


		/* SCALE */
		if (node->__do_scale)
			glScalef(node->scale.c[0],node->scale.c[1],node->scale.c[2]);

		/* REVERSE SCALE ORIENTATION */
		if (node->__do_scaleO)
			glRotatef(-my_scaleO, node->scaleOrientation.r[0],
				node->scaleOrientation.r[1],node->scaleOrientation.r[2]);

		/* REVERSE CENTER */
		if (node->__do_center)
			glTranslatef(-node->center.c[0],-node->center.c[1],-node->center.c[2]);

		RECORD_DISTANCE
        }
}


void fin_Transform (struct X3D_Transform *node) {
	OCCLUSIONTEST

        if(!render_vp) {
            fwXformPop();
        } else {
           /*Rendering the viewpoint only means finding it, and calculating the reverse WorldView matrix.*/
            if((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) {
                glTranslatef(((node->center).c[0]),((node->center).c[1]),((node->center).c[2])
                );
                glRotatef(((node->scaleOrientation).r[3])/3.1415926536*180,((node->scaleOrientation).r[0]),((node->scaleOrientation).r[1]),((node->scaleOrientation).r[2])
                );
                glScalef(1.0/(((node->scale).c[0])),1.0/(((node->scale).c[1])),1.0/(((node->scale).c[2]))
                );
                glRotatef(-(((node->scaleOrientation).r[3])/3.1415926536*180),((node->scaleOrientation).r[0]),((node->scaleOrientation).r[1]),((node->scaleOrientation).r[2])
                );
                glRotatef(-(((node->rotation).r[3]))/3.1415926536*180,((node->rotation).r[0]),((node->rotation).r[1]),((node->rotation).r[2])
                );
                glTranslatef(-(((node->center).c[0])),-(((node->center).c[1])),-(((node->center).c[2]))
                );
                glTranslatef(-(((node->translation).c[0])),-(((node->translation).c[1])),-(((node->translation).c[2]))
                );
            }
        }
} 

void child_Switch (struct X3D_Switch *node) {
	/* exceedingly simple - render only one child */
	int wc = node->whichChoice;

	/* is this VRML, or X3D?? */
	if (node->__isX3D == 0) {
		if(wc >= 0 && wc < ((node->choice).n)) {
			void *p = ((node->choice).p[wc]);
			render_node(p);
		}
	} else {
		if(wc >= 0 && wc < ((node->children).n)) {
			void *p = ((node->children).p[wc]);
			render_node(p);
		}
	}
}


void child_StaticGroup (struct X3D_StaticGroup *node) {
	CHILDREN_COUNT
	DIRECTIONAL_LIGHT_SAVE
	int createlist = FALSE;

	RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* did this change? */
	if NODE_NEEDS_COMPILING {
		ConsoleMessage ("StaticGroup changed");
		MARK_NODE_COMPILED;
	}

	/* do we have to sort this node? Only if not a proto - only first node has visible children. */
	if ((nc > 1)  && !render_blend) sortChildren(node->children);

	/* do we have a DirectionalLight for a child? */
	DIRLIGHTCHILDREN(node->children);

	/* now, just render the non-directionalLight children */
	normalChildren(node->children);

	BOUNDINGBOX
	DIRECTIONAL_LIGHT_OFF
}

#ifdef NONWORKINGCODE

it is hard to sort lights, transparency, etc for a static group when using display lists.

So, we just render as a normal group.

void Old child_StaticGroup (struct X3D_StaticGroup *node) {
	CHILDREN_COUNT
	DIRECTIONAL_LIGHT_SAVE
	int createlist = FALSE;

	RETURN_FROM_CHILD_IF_NOT_FOR_ME

	if (render_geom) {
		if (render_blend==VF_Blend) {
			if (node->__transparency < 0) {
				/* printf ("creating transparency display list %d\n",node->__transparency); */
				node->__transparency  = glGenLists(1);
				createlist = TRUE;
				glNewList(node->__transparency,GL_COMPILE_AND_EXECUTE);
			} else {
				/* printf ("calling transparency list\n"); */
				glCallList (node->__transparency);
				return;
			}

		} else {
			if  (node->__solid <0 ) {
				/* printf ("creating solid display list\n"); */
				node->__solid  = glGenLists(1);
				createlist = TRUE;
				glNewList(node->__solid,GL_COMPILE_AND_EXECUTE);
			} else {
				/* printf ("calling solid list\n"); */
				glCallList (node->__solid);
				return;
			}
		}
	}


	if (render_blend == VF_Blend)
		if ((node->_renderFlags & VF_Blend) != VF_Blend) {
			if (createlist) glEndList();
			return;
		}

	/* do we have to sort this node? Only if not a proto - only first node has visible children. */
	if ((nc > 1)  && !render_blend) sortChildren(node->children);

	/* do we have a DirectionalLight for a child? */
	DIRLIGHTCHILDREN(node->children);

	/* now, just render the non-directionalLight children */
	normalChildren(node->children);

	BOUNDINGBOX
	if (createlist) glEndList();
	DIRECTIONAL_LIGHT_OFF
}

#endif


void child_Group (struct X3D_Group *node) {
	CHILDREN_COUNT
	DIRECTIONAL_LIGHT_SAVE

	RETURN_FROM_CHILD_IF_NOT_FOR_ME


	/* {
		int x;
		struct X3D_Node *xx;

		printf ("child_Group, this %d isProto %p\n",node,node->FreeWRL__protoDef);
		for (x=0; x<nc; x++) {
			xx = X3D_NODE(node->children.p[x]);
			printf ("	ch %d type %s dist %f\n",node->children.p[x],stringNodeType(xx->_nodeType),xx->_dist);
		}
	} */


		

	/* do we have to sort this node? Only if not a proto - only first node has visible children. */
	if ((!node->FreeWRL__protoDef) && (nc > 1)  && !render_blend) sortChildren(node->children);

	/* do we have a DirectionalLight for a child? */
	DIRLIGHTCHILDREN(node->children);

	/* now, just render the non-directionalLight children */
	if (node->FreeWRL__protoDef && render_geom) {
		(node->children).n = 1;
		normalChildren(node->children);
		(node->children).n = nc;
	} else {
		normalChildren(node->children);
	}

	BOUNDINGBOX
	DIRECTIONAL_LIGHT_OFF
}


void child_Transform (struct X3D_Transform *node) {
	CHILDREN_COUNT
	OCCLUSIONTEST

	DIRECTIONAL_LIGHT_SAVE
	RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* any children at all? */
	if (nc==0) return;

	/* {
		int x;
		struct X3D_Node *xx;

		printf ("child_Transform, this %d \n",node);
		for (x=0; x<nc; x++) {
			xx = X3D_NODE(node->children.p[x]);
			printf ("	ch %d type %s dist %f\n",node->children.p[x],stringNodeType(xx->_nodeType),xx->_dist);
		}
	} */

	/* Check to see if we have to check for collisions for this transform. */
#ifdef COLLISIONTRANSFORM
	if (render_collision) {
		iv.x = node->EXTENT_MAX_X/2.0;
		jv.y = node->EXTENT_MAX_Y/2.0;
		kv.z = node->EXTENT_MAX_Z/2.0;
		ov.x = -(node->EXTENT_MAX_X); 
		ov.y = -(node->EXTENT_MAX_Y); 
		ov.z = -(node->EXTENT_MAX_Z);

	       /* get the transformed position of the Box, and the scale-corrected radius. */
	       fwGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

	       transform3x3(&tupv,&tupv,modelMatrix);
	       matrotate2v(upvecmat,ViewerUpvector,tupv);
	       matmultiply(modelMatrix,upvecmat,modelMatrix);
	       /* matinverse(upvecmat,upvecmat); */

	       /* values for rapid test */
	       t_orig.x = modelMatrix[12];
	       t_orig.y = modelMatrix[13];
	       t_orig.z = modelMatrix[14];
		/* printf ("TB this %d, extent %4.3f %4.3f %4.3f pos %4.3f %4.3f %4.3f\n", 
			node,node->EXTENT_MAX_X,node->EXTENT_MAX_Y,EXTENT_MAX_Z,
			t_orig.x,t_orig.y,t_orig.z); */
	       scale = pow(det3x3(modelMatrix),1./3.);
	       if(!fast_ycylinder_box_intersect(abottom,atop,awidth,t_orig,
			scale*node->EXTENT_MAX_X*2,
			scale*node->EXTENT_MAX_Y*2,
			scale*node->EXTENT_MAX_Z*2)) {
			/* printf ("TB this %d returning fast\n",node); */
			return;
		/* } else {
			printf ("TB really look at %d\n",node); */
		}
	}
#endif

#ifdef XXBOUNDINGBOX
	if (node->PIV > 0) {
#endif
	/* do we have to sort this node? */
	if ((nc > 1 && !render_blend)) sortChildren(node->children);

	/* do we have a DirectionalLight for a child? */
	DIRLIGHTCHILDREN(node->children);

	/* now, just render the non-directionalLight children */

	/* printf ("Transform %d, flags %d, render_sensitive %d\n",
			node,node->_renderFlags,render_sensitive); */

	#ifdef CHILDVERBOSE
		printf ("transform - doing normalChildren\n");
	#endif

	normalChildren(node->children);

	#ifdef CHILDVERBOSE
		printf ("transform - done normalChildren\n");
	#endif
#ifdef XXBOUNDINGBOX
	}
#endif

	BOUNDINGBOX
	DIRECTIONAL_LIGHT_OFF
}

void changed_StaticGroup (struct X3D_StaticGroup *node) { 
	INITIALIZE_EXTENT 
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_StaticGroup, metadata))
}


void changed_Group (struct X3D_Group *node) { 
	INITIALIZE_EXTENT 
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_Group, metadata))
}

void changed_Switch (struct X3D_Switch *node) { 
	INITIALIZE_EXTENT 
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_Switch, metadata))
}


