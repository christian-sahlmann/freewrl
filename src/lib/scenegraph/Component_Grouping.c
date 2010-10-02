/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Grouping.c,v 1.41 2010/10/02 06:42:25 davejoubert Exp $

X3D Grouping Component

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
#include "../vrml_parser/CRoutes.h"
#include "../main/headers.h"

#include "../opengl/OpenGL_Utils.h"
#include "../opengl/Frustum.h"
#include "../opengl/Material.h"

#include "LinearAlgebra.h"
#include "Children.h"

void compile_Transform (struct X3D_Transform *node) { 
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_Transform, metadata))
	INITIALIZE_EXTENT;

	/* printf ("changed Transform for node %u\n",node); */
	node->__do_center = verify_translate ((GLfloat *)node->center.c);
	node->__do_trans = verify_translate ((GLfloat *)node->translation.c);
	node->__do_scale = verify_scale ((GLfloat *)node->scale.c);
	node->__do_rotation = verify_rotate ((GLfloat *)node->rotation.c);
	node->__do_scaleO = verify_rotate ((GLfloat *)node->scaleOrientation.c);

	node->__do_anything = (node->__do_center ||
			node->__do_trans ||
			node->__do_scale ||
			node->__do_rotation ||
			node->__do_scaleO);

	MARK_NODE_COMPILED
}

/* we compile the Group so that children are not continuously sorted */
void compile_Group(struct X3D_Group *node) {
	MARK_NODE_COMPILED
}

/* prep_Group - we need this so that distance (and, thus, distance sorting) works for Groups */
void prep_Group (struct X3D_Group *node) {
	COMPILE_IF_REQUIRED
	RECORD_DISTANCE

/*
printf ("prepGroup %p (root %p), flags %x children %d ",node,rootNode,node->_renderFlags,node->children.n);
if ((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) printf ("VF_Viewpoint ");
if ((node->_renderFlags & VF_Geom) == VF_Geom) printf ("VF_Geom ");
if ((node->_renderFlags & VF_localLight) == VF_localLight) printf ("VF_localLight ");
if ((node->_renderFlags & VF_Sensitive) == VF_Sensitive) printf ("VF_Sensitive ");
if ((node->_renderFlags & VF_Blend) == VF_Blend) printf ("VF_Blend ");
if ((node->_renderFlags & VF_Proximity) == VF_Proximity) printf ("VF_Proximity ");
if ((node->_renderFlags & VF_Collision) == VF_Collision) printf ("VF_Collision ");
if ((node->_renderFlags & VF_globalLight) == VF_globalLight) printf ("VF_globalLight ");
if ((node->_renderFlags & VF_hasVisibleChildren) == VF_hasVisibleChildren) printf ("VF_hasVisibleChildren ");
if ((node->_renderFlags & VF_shouldSortChildren) == VF_shouldSortChildren) printf ("VF_shouldSortChildren ");
if ((node->_renderFlags & VF_inPickableGroup) == VF_inPickableGroup) printf ("VF_inPickableGroup ");
printf ("\n");
*/


}
/* DJTRACK_PICKSENSORS - modelled on prep_Group above */
/* prep_PickableGroup - we need this so that distance (and, thus, distance sorting) works for PickableGroups */
void prep_PickableGroup (struct X3D_Group *node) {
	/* printf("%s:%d prep_PickableGroup\n",__FILE__,__LINE__); */
	RECORD_DISTANCE
/*
printf ("prep_PickableGroup %p (root %p), flags %x children %d ",node,rootNode,node->_renderFlags,node->children.n);
if ((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) printf ("VF_Viewpoint ");
if ((node->_renderFlags & VF_Geom) == VF_Geom) printf ("VF_Geom ");
if ((node->_renderFlags & VF_localLight) == VF_localLight) printf ("VF_localLight ");
if ((node->_renderFlags & VF_Sensitive) == VF_Sensitive) printf ("VF_Sensitive ");
if ((node->_renderFlags & VF_Blend) == VF_Blend) printf ("VF_Blend ");
if ((node->_renderFlags & VF_Proximity) == VF_Proximity) printf ("VF_Proximity ");
if ((node->_renderFlags & VF_Collision) == VF_Collision) printf ("VF_Collision ");
if ((node->_renderFlags & VF_globalLight) == VF_globalLight) printf ("VF_globalLight ");
if ((node->_renderFlags & VF_hasVisibleChildren) == VF_hasVisibleChildren) printf ("VF_hasVisibleChildren ");
if ((node->_renderFlags & VF_shouldSortChildren) == VF_shouldSortChildren) printf ("VF_shouldSortChildren ");
if ((node->_renderFlags & VF_inPickableGroup) == VF_inPickableGroup) printf ("VF_inPickableGroup ");
printf ("\n");
*/
}

/* do transforms, calculate the distance */
void prep_Transform (struct X3D_Transform *node) {

	COMPILE_IF_REQUIRED

        /* rendering the viewpoint means doing the inverse transformations in reverse order (while poping stack),
         * so we do nothing here in that case -ncoder */

	/* printf ("prep_Transform, render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	 render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	/* do we have any geometry visible, and are we doing anything with geometry? */
	OCCLUSIONTEST

	if(!render_vp) {
		/* do we actually have any thing to rotate/translate/scale?? */
		if (node->__do_anything) {

/* do we want to do a glPushMatrix and glPopMatrix call? It is best to do so although the code
to NOT do it is here for performance testing reasons, to see if many pushes/pops is fast/slow */

#define DO_PUSH
#ifdef DO_PUSH
		FW_GL_PUSH_MATRIX();
#endif
		/* TRANSLATION */
		if (node->__do_trans)
			FW_GL_TRANSLATE_F(node->translation.c[0],node->translation.c[1],node->translation.c[2]);

		/* CENTER */
		if (node->__do_center)
			FW_GL_TRANSLATE_F(node->center.c[0],node->center.c[1],node->center.c[2]);

		/* ROTATION */
		if (node->__do_rotation) {
			FW_GL_ROTATE_RADIANS(node->rotation.c[3], node->rotation.c[0],node->rotation.c[1],node->rotation.c[2]);
		}

		/* SCALEORIENTATION */
		if (node->__do_scaleO) {
			FW_GL_ROTATE_RADIANS(node->scaleOrientation.c[3], node->scaleOrientation.c[0], node->scaleOrientation.c[1],node->scaleOrientation.c[2]);
		}


		/* SCALE */
		if (node->__do_scale)
			FW_GL_SCALE_F(node->scale.c[0],node->scale.c[1],node->scale.c[2]);

		/* REVERSE SCALE ORIENTATION */
		if (node->__do_scaleO)
			FW_GL_ROTATE_RADIANS(-node->scaleOrientation.c[3], node->scaleOrientation.c[0], node->scaleOrientation.c[1],node->scaleOrientation.c[2]);

		/* REVERSE CENTER */
		if (node->__do_center)
			FW_GL_TRANSLATE_F(-node->center.c[0],-node->center.c[1],-node->center.c[2]);
		} 

		RECORD_DISTANCE
        }
}


void fin_Transform (struct X3D_Transform *node) {
	OCCLUSIONTEST

        if(!render_vp) {
            if (node->__do_anything) {
#ifdef DO_PUSH
		FW_GL_POP_MATRIX();
#else
		/* 7 REVERSE CENTER */
		if (node->__do_center) {
			FW_GL_TRANSLATE_F(node->center.c[0],node->center.c[1],node->center.c[2]);
		} 

		/* 6 REVERSE SCALE ORIENTATION */
		if (node->__do_scaleO) {
			FW_GL_ROTATE_RADIANS(-node->scaleOrientation.c[3], node->scaleOrientation.c[0],
				node->scaleOrientation.c[1],node->scaleOrientation.c[2]);
		}

		/* 5 SCALE */
		if (node->__do_scale)
			FW_GL_SCALE_F(1.0/node->scale.c[0],1.0/node->scale.c[1],1.0/node->scale.c[2]);

		/* 4 SCALEORIENTATION */
		if (node->__do_scaleO) {
			FW_GL_ROTATE_RADIANS(node->scaleOrientation.c[3], node->scaleOrientation.c[0],
				node->scaleOrientation.c[1],node->scaleOrientation.c[2]);
		}

		/* 3 ROTATION */
		if (node->__do_rotation) {
			FW_GL_ROTATE_RADIANS(-node->rotation.c[3],
				node->rotation.c[0],node->rotation.c[1],node->rotation.c[2]);
		}

		/* 2 CENTER */
		if (node->__do_center)
			FW_GL_TRANSLATE_F(-node->center.c[0],-node->center.c[1],-node->center.c[2]);

		/* 1 TRANSLATION */
		if (node->__do_trans)
			FW_GL_TRANSLATE_F(-node->translation.c[0],-node->translation.c[1],-node->translation.c[2]);


#endif

	    }
        } else {
           /*Rendering the viewpoint only means finding it, and calculating the reverse WorldView matrix.*/
            if((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) {
                FW_GL_TRANSLATE_F(((node->center).c[0]),((node->center).c[1]),((node->center).c[2])
                );
                FW_GL_ROTATE_RADIANS(((node->scaleOrientation).c[3]),((node->scaleOrientation).c[0]),((node->scaleOrientation).c[1]),((node->scaleOrientation).c[2])
                );
                FW_GL_SCALE_F((float)1.0/(((node->scale).c[0])),(float)1.0/(((node->scale).c[1])),(float)1.0/(((node->scale).c[2]))
                );
                FW_GL_ROTATE_RADIANS(-(((node->scaleOrientation).c[3])),((node->scaleOrientation).c[0]),((node->scaleOrientation).c[1]),((node->scaleOrientation).c[2])
                );
                FW_GL_ROTATE_RADIANS(-(((node->rotation).c[3])),((node->rotation).c[0]),((node->rotation).c[1]),((node->rotation).c[2])
                );
                FW_GL_TRANSLATE_F(-(((node->center).c[0])),-(((node->center).c[1])),-(((node->center).c[2]))
                );
                FW_GL_TRANSLATE_F(-(((node->translation).c[0])),-(((node->translation).c[1])),-(((node->translation).c[2]))
                );
            }
        }
} 

void child_Switch (struct X3D_Switch *node) {
	/* exceedingly simple - render only one child */
	int wc = node->whichChoice;

	/* is this VRML, or X3D?? */
	if (node->__isX3D) {
		if(wc >= 0 && wc < ((node->children).n)) {
			void *p = ((node->children).p[wc]);
			render_node(p);
		}
	} else {
		if(wc >= 0 && wc < ((node->choice).n)) {
			void *p = ((node->choice).p[wc]);
			render_node(p);
		}
	}
}


void child_StaticGroup (struct X3D_StaticGroup *node) {
	CHILDREN_COUNT
	LOCAL_LIGHT_SAVE

	RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* did this change? */
	if NODE_NEEDS_COMPILING {
		ConsoleMessage ("StaticGroup changed");
		MARK_NODE_COMPILED;
	}

	/* do we have a local light for a child? */
	LOCAL_LIGHT_CHILDREN(node->_sortedChildren);

	/* now, just render the non-directionalLight children */
	normalChildren(node->_sortedChildren);

	LOCAL_LIGHT_OFF
}

void child_Group (struct X3D_Group *node) {
	CHILDREN_COUNT
	LOCAL_LIGHT_SAVE
/*
printf ("chldGroup %p (root %p), flags %x children %d ",node,rootNode,node->_renderFlags,node->children.n);
if ((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) printf ("VF_Viewpoint ");
if ((node->_renderFlags & VF_Geom) == VF_Geom) printf ("VF_Geom ");
if ((node->_renderFlags & VF_localLight) == VF_localLight) printf ("VF_localLight ");
if ((node->_renderFlags & VF_Sensitive) == VF_Sensitive) printf ("VF_Sensitive ");
if ((node->_renderFlags & VF_Blend) == VF_Blend) printf ("VF_Blend ");
if ((node->_renderFlags & VF_Proximity) == VF_Proximity) printf ("VF_Proximity ");
if ((node->_renderFlags & VF_Collision) == VF_Collision) printf ("VF_Collision ");
if ((node->_renderFlags & VF_globalLight) == VF_globalLight) printf ("VF_globalLight ");
if ((node->_renderFlags & VF_hasVisibleChildren) == VF_hasVisibleChildren) printf ("VF_hasVisibleChildren ");
if ((node->_renderFlags & VF_shouldSortChildren) == VF_shouldSortChildren) printf ("VF_shouldSortChildren ");
if ((node->_renderFlags & VF_inPickableGroup) == VF_inPickableGroup) printf ("VF_inPickableGroup ");
printf ("\n");
*/

	RETURN_FROM_CHILD_IF_NOT_FOR_ME

/*
	 {
		int x;
		struct X3D_Node *xx;

		printf ("child_Group, this %u rf %x isProto %d\n",node,node->_renderFlags, node->FreeWRL__protoDef);
        printf ("	..., render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
         render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); 
		for (x=0; x<nc; x++) {
			xx = X3D_NODE(node->_sortedChildren.p[x]);
			printf ("	ch %u type %s dist %f\n",node->_sortedChildren.p[x],stringNodeType(xx->_nodeType),xx->_dist);
		}
	}
*/



		
	/* do we have a DirectionalLight for a child? */
	LOCAL_LIGHT_CHILDREN(node->_sortedChildren);

	/* printf ("chld_Group, for %u, protodef %d and FreeWRL_PROTOInterfaceNodes.n %d\n",
		node, node->FreeWRL__protoDef, node->FreeWRL_PROTOInterfaceNodes.n); */
	/* now, just render the non-directionalLight children */
	if ((node->FreeWRL__protoDef!=INT_ID_UNDEFINED) && render_geom) {
		(node->children).n = 1;
		normalChildren(node->children);
		(node->children).n = nc;
	} else {
		normalChildren(node->_sortedChildren);
	}

	LOCAL_LIGHT_OFF
}

/* DJTRACK_PICKSENSORS - modelled on child_Group above */
void child_PickableGroup (struct X3D_Group *node) {
	CHILDREN_COUNT
	LOCAL_LIGHT_SAVE
/*
printf ("chldGroup %p (root %p), flags %x children %d ",node,rootNode,node->_renderFlags,node->children.n);
if ((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) printf ("VF_Viewpoint ");
if ((node->_renderFlags & VF_Geom) == VF_Geom) printf ("VF_Geom ");
if ((node->_renderFlags & VF_localLight) == VF_localLight) printf ("VF_localLight ");
if ((node->_renderFlags & VF_Sensitive) == VF_Sensitive) printf ("VF_Sensitive ");
if ((node->_renderFlags & VF_Blend) == VF_Blend) printf ("VF_Blend ");
if ((node->_renderFlags & VF_Proximity) == VF_Proximity) printf ("VF_Proximity ");
if ((node->_renderFlags & VF_Collision) == VF_Collision) printf ("VF_Collision ");
if ((node->_renderFlags & VF_globalLight) == VF_globalLight) printf ("VF_globalLight ");
if ((node->_renderFlags & VF_hasVisibleChildren) == VF_hasVisibleChildren) printf ("VF_hasVisibleChildren ");
if ((node->_renderFlags & VF_shouldSortChildren) == VF_shouldSortChildren) printf ("VF_shouldSortChildren ");
if ((node->_renderFlags & VF_inPickableGroup) == VF_inPickableGroup) printf ("VF_inPickableGroup ");
printf ("\n");
*/
	RETURN_FROM_CHILD_IF_NOT_FOR_ME
	/* printf("%s:%d child_PickableGroup\n",__FILE__,__LINE__); */

	 if (1==0) {
		int x;
		struct X3D_Node *xx;

		printf ("child_PickableGroup, this %p rf %x isProto %d\n",node,node->_renderFlags, node->FreeWRL__protoDef);
		printf ("	..., render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
			render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); 

		for (x=0; x<nc; x++) {
			xx = X3D_NODE(node->_sortedChildren.p[x]);
			printf ("	ch %p type %s dist %f\n",node->_sortedChildren.p[x],stringNodeType(xx->_nodeType),xx->_dist);
		}
	}

	/* do we have a DirectionalLight for a child? */
	LOCAL_LIGHT_CHILDREN(node->_sortedChildren);

	/* printf ("chld_PickableGroup, for %u, protodef %d and FreeWRL_PROTOInterfaceNodes.n %d\n",
		node, node->FreeWRL__protoDef, node->FreeWRL_PROTOInterfaceNodes.n); */
	/* now, just render the non-directionalLight children */
	if ((node->FreeWRL__protoDef!=INT_ID_UNDEFINED) && render_geom) {
		(node->children).n = 1;
		normalChildren(node->children);
		(node->children).n = nc;
	} else {
		normalChildren(node->_sortedChildren);
	}

	LOCAL_LIGHT_OFF
}


void child_Transform (struct X3D_Transform *node) {
	LOCAL_LIGHT_SAVE
	CHILDREN_COUNT
	OCCLUSIONTEST

	RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* any children at all? */
	if (nc==0) return;

	 /* {
		int x;
		struct X3D_Node *xx;

		printf ("child_Transform, this %u rf %x \n",node,node->_renderFlags);
        printf ("      ..., render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
         render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); 
		for (x=0; x<nc; x++) {
			xx = X3D_NODE(node->_sortedChildren.p[x]);
			printf ("	ch %u type %s dist %f\n",node->_sortedChildren.p[x],stringNodeType(xx->_nodeType),xx->_dist);
		}
	} 
*/

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
	       FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix);

	      // transform3x3(&tupv,&tupv,modelMatrix);
	      // matrotate2v(upvecmat,ViewerUpvector,tupv);
	      // matmultiply(modelMatrix,upvecmat,modelMatrix);
	       /* matinverse(upvecmat,upvecmat); */
			matmultiply(modelMatrix,FallInfo.avatar2collision,modelMatrix); 

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
	/* do we have a local light for a child? */
	LOCAL_LIGHT_CHILDREN(node->_sortedChildren);

	/* now, just render the non-directionalLight children */

	/* printf ("Transform %d, flags %d, render_sensitive %d\n",
			node,node->_renderFlags,render_sensitive); */

	#ifdef CHILDVERBOSE
		printf ("transform - doing normalChildren\n");
	#endif

	normalChildren(node->_sortedChildren);

	#ifdef CHILDVERBOSE
		printf ("transform - done normalChildren\n");
	#endif

	LOCAL_LIGHT_OFF
}
