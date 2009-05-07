/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Navigation.c,v 1.15 2009/05/07 20:03:20 crc_canada Exp $

X3D Navigation Component

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"

#include "../x3d_parser/Bindable.h"
#include "LinearAlgebra.h"
#include "Collision.h"
#include "quaternion.h"
#include "Viewer.h"


extern struct sCollisionInfo OldCollisionInfo;

void prep_Viewpoint (struct X3D_Viewpoint *node) {
	double a1;

	if (!render_vp) return;

	/*  printf ("RVP, node %d ib %d sb %d gepvp\n",node,node->isBound,node->set_bind);
	 printf ("VP stack %d tos %d\n",viewpoint_tos, viewpoint_stack[viewpoint_tos]); */

	 

	/* check the set_bind eventin to see if it is TRUE or FALSE */
	/* code to perform binding is now in set_viewpoint. */

	/* we will never get here unless we are told that we are active by the scene graph; actually
	   doing this test can screw us up, so DO NOT do this test!
			if(!node->isBound) return;
	*/
	
	/* printf ("Component_Nav, found VP is %d, (%s)\n",node,node->description->strptr); */
	

	/* perform Viewpoint translations */
	FW_GL_ROTATE_D(-node->orientation.c[3]/PI*180.0,node->orientation.c[0],node->orientation.c[1],
		node->orientation.c[2]);
	FW_GL_TRANSLATE_D(-node->position.c[0],-node->position.c[1],-node->position.c[2]);

	/* now, lets work on the Viewpoint fieldOfView */
	glGetIntegerv(GL_VIEWPORT, viewPort);
	if(viewPort[2] > viewPort[3]) {
		a1=0;
		fieldofview = node->fieldOfView/3.1415926536*180;
	} else {
		a1 = node->fieldOfView;
		a1 = atan2(sin(a1),viewPort[2]/((float)viewPort[3]) * cos(a1));
		fieldofview = a1/3.1415926536*180;
	}
	/* printf ("render_Viewpoint, bound to %d, fieldOfView %f \n",node,node->fieldOfView); */
}

/******************************************************************************************/

void proximity_Billboard (struct X3D_Billboard *node) {
	/* printf ("prox_billboard, do nothing\n"); */
}

void prep_Billboard (struct X3D_Billboard *node) {
	struct point_XYZ vpos, ax, cp, cp2, arcp;
	static const struct point_XYZ orig = {0.0, 0.0, 0.0};
	static const struct point_XYZ zvec = {0.0, 0.0, 1.0};
	struct orient_XYZA viewer_orient;
	GLdouble mod[16];
	GLdouble proj[16];
	int align;
	double len, len2, angle;
	int sign;

	RECORD_DISTANCE

	ax.x = node->axisOfRotation.c[0];
	ax.y = node->axisOfRotation.c[1];
	ax.z = node->axisOfRotation.c[2];
	align = (APPROX(VECSQ(ax),0));

	quaternion_to_vrmlrot(&(Viewer.Quat),
		&(viewer_orient.x), &(viewer_orient.y),
		&(viewer_orient.z), &(viewer_orient.a));

	FW_GL_PUSH_MATRIX();

	fwGetDoublev(GL_MODELVIEW_MATRIX, mod);
	fwGetDoublev(GL_PROJECTION_MATRIX, proj);
	gluUnProject(orig.x, orig.y, orig.z, mod, proj,
		viewport, &vpos.x, &vpos.y, &vpos.z);

	len = VECSQ(vpos);
	if (APPROX(len, 0)) { return; }
	VECSCALE(vpos, 1/sqrt(len));

	if (align) {
		ax.x = viewer_orient.x;
		ax.y = viewer_orient.y;
		ax.z = viewer_orient.z;
	}

	VECCP(ax, zvec, arcp);
	len = VECSQ(arcp);
	if (APPROX(len, 0)) { return; }

	len = VECSQ(ax);
	if (APPROX(len, 0)) { return; }
	VECSCALE(ax, 1/sqrt(len));

	VECCP(vpos, ax, cp); /* cp is now 90deg to both vector and axis */
	len = sqrt(VECSQ(cp));
	if (APPROX(len, 0)) {
		FW_GL_ROTATE_F(-viewer_orient.a/3.1415926536*180, ax.x, ax.y, ax.z);
		return;
	}
	VECSCALE(cp, 1/len);

	/* Now, find out angle between this and z axis */
	VECCP(cp, zvec, cp2);

	len2 = VECPT(cp, zvec); /* cos(angle) */
	len = sqrt(VECSQ(cp2)); /* this is abs(sin(angle)) */

	/* Now we need to find the sign first */
	if (VECPT(cp, arcp) > 0) { sign = -1; } else { sign = 1; }
	angle = atan2(len2, sign*len);

	FW_GL_ROTATE_F(angle/3.1415926536*180, ax.x, ax.y, ax.z);
	invalidateCurMat();  /* force a glGetMatrix from the system */
}

void fin_Billboard (struct X3D_Billboard *node) {
	UNUSED(node);
	FW_GL_POP_MATRIX();
	invalidateCurMat();
}


void  child_Billboard (struct X3D_Billboard *node) {
	int nc = (node->children).n;

	DIRECTIONAL_LIGHT_SAVE


	/* any children at all? */
	if (nc==0) return;

	#ifdef CHILDVERBOSE
	printf("RENDER BILLBOARD START %d (%d)\n",node, nc);
	#endif

	/* do we have to sort this node? */
/* 	if ((nc > 1 && !render_blend)) sortChildren(node->children); */

	/* do we have a DirectionalLight for a child? */
	DIRLIGHTCHILDREN(node->children);

	/* now, just render the non-directionalLight children */
	normalChildren(node->children);

	if (render_geom && (!render_blend)) {
		EXTENTTOBBOX
	}

	#ifdef CHILDVERBOSE
	printf("RENDER BILLBOARD END %d\n",node);
	#endif

	DIRECTIONAL_LIGHT_OFF
}


void changed_Billboard (struct X3D_Billboard *node) {
                int i;
                int nc = ((node->children).n);

		INITIALIZE_EXTENT
}
/******************************************************************************************/


void render_NavigationInfo (struct X3D_NavigationInfo *node) {
	/* check the set_bind eventin to see if it is TRUE or FALSE */
	if (node->set_bind < 100) {
		if (node->set_bind == 1) set_naviinfo(node);

		bind_node (X3D_NODE(node), &navi_tos,&navi_stack[0]);
	}
	if(!node->isBound) return;
}



void child_Collision (struct X3D_Collision *node) {
	int nc = (node->children).n;
	int i;
	void *tmpN;

	if(render_collision) {
		/* test against the collide field (vrml) enabled (x3d) and that we actually have a proxy field */
		if((node->collide) && (node->enabled) && !(node->proxy)) {
			struct sCollisionInfo OldCollisionInfo = CollisionInfo;
			for(i=0; i<nc; i++) {
				void *p = ((node->children).p[i]);
				#ifdef CHILDVERBOSE
				printf("RENDER COLLISION %d CHILD %d\n",node, p);
				#endif
				render_node(p);
			}
			if((!APPROX(CollisionInfo.Offset.x,
					OldCollisionInfo.Offset.x)) ||
			   (!APPROX(CollisionInfo.Offset.y,
				   OldCollisionInfo.Offset.y)) ||
			   (!APPROX(CollisionInfo.Offset.z,
				    OldCollisionInfo.Offset.z))) {
			/* old code was:
			if(CollisionInfo.Offset.x != OldCollisionInfo.Offset.x ||
			   CollisionInfo.Offset.y != OldCollisionInfo.Offset.y ||
			   CollisionInfo.Offset.z != OldCollisionInfo.Offset.z) { */
				/*collision occured
				 * bit 0 gives collision, bit 1 gives change */
				node->__hit = (node->__hit & 1) ? 1 : 3;
			} else
				node->__hit = (node->__hit & 1) ? 2 : 0;

		}
        	if(node->proxy) {
			POSSIBLE_PROTO_EXPANSION(node->proxy,tmpN)
                       	render_node(tmpN);
		}

	} else { /*standard group behaviour*/
		DIRECTIONAL_LIGHT_SAVE

		#ifdef CHILDVERBOSE
		printf("RENDER COLLISIONCHILD START %d (%d)\n",node, nc);
		#endif
		/* do we have to sort this node? */
/* 		if ((nc > 1 && !render_blend)) sortChildren(node->children); */

		/* do we have a DirectionalLight for a child? */
		DIRLIGHTCHILDREN(node->children);

		/* now, just render the non-directionalLight children */
		normalChildren(node->children);

		#ifdef CHILDVERBOSE
		printf("RENDER COLLISIONCHILD END %d\n",node);
		#endif
		DIRECTIONAL_LIGHT_OFF
	}
}

void changed_LOD (struct X3D_LOD *node) {
}

/* LOD changes between X3D and VRML - level and children fields are "equivalent" */
void child_LOD (struct X3D_LOD *node) {
        render_node(node->_selected);
}


/* calculate the LOD distance */
void proximity_LOD (struct X3D_LOD *node) {
        GLdouble mod[16];
        GLdouble proj[16];
        struct point_XYZ vec;
        double dist;
        int nran = (node->range).n;
        int nnod = (node->level).n;
        int xnod = (node->children).n;
        int i;

	/* no range, display the first node, if it exists */
        if(!nran) {
		if (node->__isX3D)  {
			if (nnod > 0) node->_selected = (node->children).p[0];
			else node->_selected = NULL;
		} else {
			if (xnod > 0) node->_selected = (node->level).p[0];
			else node->_selected = NULL;
		}
                return;
        }

        /* calculate which one to display */
        fwGetDoublev(GL_MODELVIEW_MATRIX, mod);
        /* printf ("LOD, mat %f %f %f\n",mod[12],mod[13],mod[14]); */
        fwGetDoublev(GL_PROJECTION_MATRIX, proj);
        gluUnProject(0,0,0,mod,proj,viewport, &vec.x,&vec.y,&vec.z);
        vec.x -= (node->center).c[0];
        vec.y -= (node->center).c[1];
        vec.z -= (node->center).c[2];

        dist = sqrt(VECSQ(vec));
        i = 0;

        while (i<nran) {
                       if(dist < ((node->range).p[i])) { break; }
                       i++;
        }

	/* is this VRML or X3D? */
	if (node->__isX3D) {
		if (xnod > 0) {
			/* X3D "children" field */
        	       	if(i >= xnod) i = xnod-1;
        		       	node->_selected = (node->children).p[i];
				/* printf ("selecting X3D nod %d \n",i); */
		} else node->_selected = NULL;
		
	} else {
		if (nnod > 0) {
			/* VRML "range" field */
               		if(i >= nnod) i = nnod-1;
               		node->_selected = (node->level).p[i];
			/* printf ("selecting vrml nod\n"); */
		} else { node->_selected = NULL; }
	}
}



void changed_Inline (struct X3D_Inline *node) {
		INITIALIZE_EXTENT
}


void changed_Collision (struct X3D_Collision *node) {
		INITIALIZE_EXTENT
}
