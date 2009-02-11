/*
=INSERT_TEMPLATE_HERE=

$Id: Component_HAnim.c,v 1.6 2009/02/11 15:12:55 istakenv Exp $

X3D H-Anim Component

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h" /* point_XYZ */
#include "../main/headers.h"

/* #include "OpenFW_GL_Utils.h" */


/* last HAnimHumanoid skinCoord and skinNormals */
void *HANimSkinCoord = 0;
void *HAnimSkinNormal = 0;

void prep_HAnimJoint (struct X3D_HAnimJoint *node) {
/*
	GLfloat my_rotation;
	GLfloat my_scaleO=0;
*/
return;
#ifdef HANIMHANIM
        /* rendering the viewpoint means doing the inverse transformations in reverse order (while poping stack),
         * so we do nothing here in that case -ncoder */

	/* printf ("render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",*/
	/* render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision);*/

	if(!render_vp) {
		FW_GL_PUSH_MATRIX();

		/* might we have had a change to a previously ignored value? */
		if (node->_change != node->_dlchange) {
			/* printf ("re-rendering for %d\n",node);*/
			node->__do_center = verify_translate ((GLfloat *)node->center.c);
			node->__do_trans = verify_translate ((GLfloat *)node->translation.c);
			node->__do_scale = verify_scale ((GLfloat *)node->scale.c);
			node->__do_rotation = verify_rotate ((GLfloat *)node->rotation.r);
			node->__do_scaleO = verify_rotate ((GLfloat *)node->scaleOrientation.r);
			node->_dlchange = node->_change;
		}



		/* TRANSLATION */
		if (node->__do_trans)
			FW_GL_TRANSLATE_F(node->translation.c[0],node->translation.c[1],node->translation.c[2]);

		/* CENTER */
		if (node->__do_center)
			FW_GL_TRANSLATE_F(node->center.c[0],node->center.c[1],node->center.c[2]);

		/* ROTATION */
		if (node->__do_rotation) {
			my_rotation = node->rotation.r[3]/3.1415926536*180;
			FW_GL_ROTATE_F(my_rotation,
				node->rotation.r[0],node->rotation.r[1],node->rotation.r[2]);
		}

		/* SCALEORIENTATION */
		if (node->__do_scaleO) {
			my_scaleO = node->scaleOrientation.r[3]/3.1415926536*180;
			FW_GL_ROTATE_F(my_scaleO, node->scaleOrientation.r[0],
				node->scaleOrientation.r[1],node->scaleOrientation.r[2]);
		}


		/* SCALE */
		if (node->__do_scale)
			FW_GL_SCALE_F(node->scale.c[0],node->scale.c[1],node->scale.c[2]);

		/* REVERSE SCALE ORIENTATION */
		if (node->__do_scaleO)
			FW_GL_ROTATE_F(-my_scaleO, node->scaleOrientation.r[0],
				node->scaleOrientation.r[1],node->scaleOrientation.r[2]);

		/* REVERSE CENTER */
		if (node->__do_center)
			FW_GL_TRANSLATE_F(-node->center.c[0],-node->center.c[1],-node->center.c[2]);

		RECORD_DISTANCE
        }
#endif
}


void prep_HAnimSite (struct X3D_HAnimSite *node) {

	/*
	GLfloat my_rotation;
	GLfloat my_scaleO=0;
	*/
return;
#ifdef HANIMHANIM

        /* rendering the viewpoint means doing the inverse transformations in reverse order (while poping stack),
         * so we do nothing here in that case -ncoder */

	/* printf ("render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",*/
	/* render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision);*/

	if(!render_vp) {
		FW_GL_PUSH_MATRIX();

		/* might we have had a change to a previously ignored value? */
		if (node->_change != node->_dlchange) {
			/* printf ("re-rendering for %d\n",node);*/
			node->__do_center = verify_translate ((GLfloat *)node->center.c);
			node->__do_trans = verify_translate ((GLfloat *)node->translation.c);
			node->__do_scale = verify_scale ((GLfloat *)node->scale.c);
			node->__do_rotation = verify_rotate ((GLfloat *)node->rotation.r);
			node->__do_scaleO = verify_rotate ((GLfloat *)node->scaleOrientation.r);
			node->_dlchange = node->_change;
		}



		/* TRANSLATION */
		if (node->__do_trans)
			FW_GL_TRANSLATE_F(node->translation.c[0],node->translation.c[1],node->translation.c[2]);

		/* CENTER */
		if (node->__do_center)
			FW_GL_TRANSLATE_F(node->center.c[0],node->center.c[1],node->center.c[2]);

		/* ROTATION */
		if (node->__do_rotation) {
			my_rotation = node->rotation.r[3]/3.1415926536*180;
			FW_GL_ROTATE_F(my_rotation,
				node->rotation.r[0],node->rotation.r[1],node->rotation.r[2]);
		}

		/* SCALEORIENTATION */
		if (node->__do_scaleO) {
			my_scaleO = node->scaleOrientation.r[3]/3.1415926536*180;
			FW_GL_ROTATE_F(my_scaleO, node->scaleOrientation.r[0],
				node->scaleOrientation.r[1],node->scaleOrientation.r[2]);
		}


		/* SCALE */
		if (node->__do_scale)
			FW_GL_SCALE_F(node->scale.c[0],node->scale.c[1],node->scale.c[2]);

		/* REVERSE SCALE ORIENTATION */
		if (node->__do_scaleO)
			FW_GL_ROTATE_F(-my_scaleO, node->scaleOrientation.r[0],
				node->scaleOrientation.r[1],node->scaleOrientation.r[2]);

		/* REVERSE CENTER */
		if (node->__do_center)
			FW_GL_TRANSLATE_F(-node->center.c[0],-node->center.c[1],-node->center.c[2]);
		
		RECORD_DISTANCE
        }
#endif
}

void render_HAnimHumanoid (struct X3D_HAnimHumanoid *node) {
	/* save the skinCoords and skinNormals for use in following HAnimJoints */
	POSSIBLE_PROTO_EXPANSION(node->skinCoord,HANimSkinCoord)
	POSSIBLE_PROTO_EXPANSION(node->skinNormal,HAnimSkinNormal)
	/* printf ("rendering HAnimHumanoid\n"); */
}

void render_HAnimJoint (struct X3D_HAnimJoint * node) {
return;
	/* printf ("rendering HAnimJoint %d\n",node); */

}

void child_HAnimHumanoid(struct X3D_HAnimHumanoid *node) {
	int nc;
	DIRECTIONAL_LIGHT_SAVE

	/* any segments at all? */
/*
printf ("hanimHumanoid, segment coutns %d %d %d %d %d %d\n",
		node->joints.n,
		node->segments.n,
		node->sites.n,
		node->skeleton.n,
		node->skin.n,
		node->viewpoints.n);
*/

	nc = node->joints.n + node->segments.n + node->viewpoints.n + node->sites.n +
		node->skeleton.n + node->skin.n;

	RETURN_FROM_CHILD_IF_NOT_FOR_ME 

	/* Lets do segments first */
	/* do we have to sort this node? */
	if ((node->segments.n > 1)  && !render_blend) sortChildren(node->segments);
	/* now, just render the non-directionalLight segments */
	normalChildren(node->segments);


	/* Lets do joints second */
	/* do we have to sort this node? */
	if ((node->joints.n > 1)  && !render_blend) sortChildren(node->joints);
	/* now, just render the non-directionalLight joints */
	normalChildren(node->joints);


	/* Lets do sites third */
	/* do we have to sort this node? */
	if ((node->sites.n > 1)  && !render_blend) sortChildren(node->sites);
	/* do we have a DirectionalLight for a child? */
	DIRLIGHTCHILDREN(node->sites);
	/* now, just render the non-directionalLight sites */
	normalChildren(node->sites);

	/* Lets do skeleton fourth */
	/* do we have to sort this node? */
	if ((node->skeleton.n > 1)  && !render_blend) sortChildren(node->skeleton);
	/* now, just render the non-directionalLight skeleton */
	normalChildren(node->skeleton);

	/* Lets do skin fifth */
	/* do we have to sort this node? */
	if ((node->skin.n > 1)  && !render_blend) sortChildren(node->skin);
	/* do we have a DirectionalLight for a child? */
	DIRLIGHTCHILDREN(node->skin);
	/* now, just render the non-directionalLight skin */
	normalChildren(node->skin);


	/* Lets do viewpoints last */
	normalChildren(node->segments);

	/* did we have that directionalLight? */
	DIRECTIONAL_LIGHT_OFF
}


void child_HAnimJoint(struct X3D_HAnimJoint *node) {
	CHILDREN_COUNT
return;
#ifdef HANIMHANIM
	/* any children at all? */
	if (nc==0) return;

	/* should we go down here? */
	RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* do we have to sort this node? */
	if ((nc > 1)  && !render_blend) sortChildren(node->children);

	/* just render the non-directionalLight children */
	normalChildren(node->children);

#endif
}

void child_HAnimSegment(struct X3D_HAnimSegment *node) {
	CHILDREN_COUNT
return;
#ifdef HANIMHANIM


note to implementer: have to POSSIBLE_PROTO_EXPANSION(node->coord, tmpN)

	/* any children at all? */
	if (nc==0) return;

	/* should we go down here? */
	RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* do we have to sort this node? Only if not a proto - only first node has visible children. */
	if ((nc > 1)  && !render_blend) sortChildren(node->children);

	/* now, just render the non-directionalLight children */
	normalChildren(node->children);
#endif
}


void child_HAnimSite(struct X3D_HAnimSite *node) {
	CHILDREN_COUNT
	DIRECTIONAL_LIGHT_SAVE
return;
#ifdef HANIMHANIM
	RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* do we have to sort this node? */
	if ((nc > 1)  && !render_blend) sortChildren(node->children);

	/* do we have a DirectionalLight for a child? */
	DIRLIGHTCHILDREN(node->children);

	/* now, just render the non-directionalLight children */
	normalChildren(node->children);

	DIRECTIONAL_LIGHT_OFF
#endif
}

void fin_HAnimSite (struct X3D_HAnimSite * node) {
        if(!render_vp) {
            FW_GL_POP_MATRIX();
        } else {
           /*Rendering the viewpoint only means finding it, and calculating the reverse WorldView matrix.*/
            if((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) {
                FW_GL_TRANSLATE_F(((node->center).c[0]),((node->center).c[1]),((node->center).c[2])
                );
                FW_GL_ROTATE_F(((node->scaleOrientation).r[3])/3.1415926536*180,((node->scaleOrientation).r[0]),((node->scaleOrientation).r[1]),((node->scaleOrientation).r[2])
                );
                FW_GL_SCALE_F(1.0/(((node->scale).c[0])),1.0/(((node->scale).c[1])),1.0/(((node->scale).c[2]))
                );
                FW_GL_ROTATE_F(-(((node->scaleOrientation).r[3])/3.1415926536*180),((node->scaleOrientation).r[0]),((node->scaleOrientation).r[1]),((node->scaleOrientation).r[2])
                );
                FW_GL_ROTATE_F(-(((node->rotation).r[3]))/3.1415926536*180,((node->rotation).r[0]),((node->rotation).r[1]),((node->rotation).r[2])
                );
                FW_GL_TRANSLATE_F(-(((node->center).c[0])),-(((node->center).c[1])),-(((node->center).c[2]))
                );
                FW_GL_TRANSLATE_F(-(((node->translation).c[0])),-(((node->translation).c[1])),-(((node->translation).c[2]))
                );
            }
        }
}

void fin_HAnimJoint (struct X3D_HAnimJoint * node) {
        if(!render_vp) {
            FW_GL_POP_MATRIX();
        } else {
           /*Rendering the viewpoint only means finding it, and calculating the reverse WorldView matrix.*/
            if((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) {
                FW_GL_TRANSLATE_F(((node->center).c[0]),((node->center).c[1]),((node->center).c[2])
                );
                FW_GL_ROTATE_F(((node->scaleOrientation).r[3])/3.1415926536*180,((node->scaleOrientation).r[0]),((node->scaleOrientation).r[1]),((node->scaleOrientation).r[2])
                );
                FW_GL_SCALE_F(1.0/(((node->scale).c[0])),1.0/(((node->scale).c[1])),1.0/(((node->scale).c[2]))
                );
                FW_GL_ROTATE_F(-(((node->scaleOrientation).r[3])/3.1415926536*180),((node->scaleOrientation).r[0]),((node->scaleOrientation).r[1]),((node->scaleOrientation).r[2])
                );
                FW_GL_ROTATE_F(-(((node->rotation).r[3]))/3.1415926536*180,((node->rotation).r[0]),((node->rotation).r[1]),((node->rotation).r[2])
                );
                FW_GL_TRANSLATE_F(-(((node->center).c[0])),-(((node->center).c[1])),-(((node->center).c[2]))
                );
                FW_GL_TRANSLATE_F(-(((node->translation).c[0])),-(((node->translation).c[1])),-(((node->translation).c[2]))
                );
            }
        }
}

void changed_HAnimSite (struct X3D_HAnimSite *node) {
	MARK_SFNODE_INOUT_EVENT(node->metadata, node->__oldmetadata, offsetof (struct X3D_HAnimSite, metadata))
	INITIALIZE_EXTENT
}

