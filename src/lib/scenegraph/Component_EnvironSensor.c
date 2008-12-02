/*
=INSERT_TEMPLATE_HERE=

$Id: Component_EnvironSensor.c,v 1.2 2008/11/27 00:27:18 couannette Exp $

X3D Environmental Sensors Component

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"


/* can we do a VisibiltySensor? Only if we have OpenGL support for OcclusionCulling */
int candoVisibility = TRUE;

void rendVisibilityBox (struct X3D_VisibilitySensor *node);

PROXIMITYSENSOR(ProximitySensor,center,,);


/* VisibilitySensors - mimic what Shape does to display the box. */


void child_VisibilitySensor (struct X3D_VisibilitySensor *node) {
		
	/* if not enabled, do nothing */
	if (!node) return;
	if (!node->enabled) return;

		if (!candoVisibility) return;

		/* first time through, if we have a visibility sensor, but do not have the OpenGL ability to
		   use it, we print up a console message */
		if (OccFailed) {
			candoVisibility = FALSE;
			ConsoleMessage("VisibilitySensor: OpenGL on this machine does not support GL_ARB_occlusion_query");
			return;
		}

		RECORD_DISTANCE

		if (render_blend == VF_Blend) { 
                        #ifdef VISIBILITYOCCLUSION

			BEGINOCCLUSIONQUERY
			LIGHTING_OFF
			DISABLE_CULL_FACE 

			rendVisibilityBox(node);
			
			ENABLE_CULL_FACE
			LIGHTING_ON
			
			ENDOCCLUSIONQUERY
                        #endif
		}

}

void rendVisibilityBox (struct X3D_VisibilitySensor *node) {
	extern GLfloat boxnorms[];		/*  in CFuncs/statics.c*/
	float *pt;
	float x = ((node->size).c[0])/2;
	float y = ((node->size).c[1])/2;
	float z = ((node->size).c[2])/2;
	float cx = node->center.c[0];
	float cy = node->center.c[1];
	float cz = node->center.c[2];

	/* test for <0 of sides */
	if ((x < 0) || (y < 0) || (z < 0)) return;

	/* for BoundingBox calculations */
	setExtent(cx+x, cx-x, cx+y, cx-y, cx+z, cx-z,X3D_NODE(node));


	if NODE_NEEDS_COMPILING {
		/*  have to regen the shape*/

		MARK_NODE_COMPILED

		/*  MALLOC memory (if possible)*/
		if (!node->__points) node->__points = MALLOC (sizeof(struct SFColor)*(24));

		/*  now, create points; 4 points per face.*/
		pt = (float *) node->__points;
		/*  front*/
		*pt++ = cx+x; *pt++ = cy+y; *pt++ = cz+z; *pt++ = cx-x; *pt++ = cy+y; *pt++ = cz+z;
		*pt++ = cx-x; *pt++ = cy-y; *pt++ = cz+z; *pt++ = cx+x; *pt++ = cy-y; *pt++ = cz+z;
		/*  back*/
		*pt++ = cx+x; *pt++ = cy-y; *pt++ = cz-z; *pt++ = cx-x; *pt++ = cy-y; *pt++ = cz-z;
		*pt++ = cx-x; *pt++ = cy+y; *pt++ = cz-z; *pt++ = cx+x; *pt++ = cy+y; *pt++ = cz-z;
		/*  top*/
		*pt++ = cx-x; *pt++ = cy+y; *pt++ = cz+z; *pt++ = cx+x; *pt++ = cy+y; *pt++ = cz+z;
		*pt++ = cx+x; *pt++ = cy+y; *pt++ = cz-z; *pt++ = cx-x; *pt++ = cy+y; *pt++ = cz-z;
		/*  down*/
		*pt++ = cx-x; *pt++ = cy-y; *pt++ = cz-z; *pt++ = cx+x; *pt++ = cy-y; *pt++ = cz-z;
		*pt++ = cx+x; *pt++ = cy-y; *pt++ = cz+z; *pt++ = cx-x; *pt++ = cy-y; *pt++ = cz+z;
		/*  right*/
		*pt++ = cx+x; *pt++ = cy-y; *pt++ = cz+z; *pt++ = cx+x; *pt++ = cy-y; *pt++ = cz-z;
		*pt++ = cx+x; *pt++ = cy+y; *pt++ = cz-z; *pt++ = cx+x; *pt++ = cy+y; *pt++ = cz+z;
		/*  left*/
		*pt++ = cx-x; *pt++ = cy-y; *pt++ = cz+z; *pt++ = cx-x; *pt++ = cy+y; *pt++ = cz+z;
		*pt++ = cx-x; *pt++ = cy+y; *pt++ = cz-z; *pt++ = cx-x; *pt++ = cy-y; *pt++ = cz-z;
	}

	/* note the ALPHA of zero - totally transparent */
	glColor4f(0.0, 1.0, 0.0, 0.0);

	/*  Draw it; assume VERTEX and NORMALS already defined.*/
	glVertexPointer (3,GL_FLOAT,0,(GLfloat *)node->__points);
	glNormalPointer (GL_FLOAT,0,boxnorms);

	/* do the array drawing; sides are simple 0-1-2-3, 4-5-6-7, etc quads */
	glDrawArrays (GL_QUADS, 0, 24);
}


void do_VisibilitySensorTick (void *ptr) {
	struct X3D_VisibilitySensor *node = (struct X3D_VisibilitySensor *) ptr;

	/* if not enabled, do nothing */
	if (!node) return;
	if (node->__oldEnabled != node->enabled) {
		node->__oldEnabled = node->enabled;
		MARK_EVENT(X3D_NODE(node),offsetof (struct X3D_VisibilitySensor, enabled));
	}
	if (!node->enabled) return;
	/* are we enabled? */

	#ifdef SEVERBOSE
	printf ("do_VisibilitySensorTick, samples %d\n",node->__samples);
	#endif
	
	if (node->__Samples > 0) {
		/* we are here... */
                if (!node->isActive) {
                        #ifdef SEVERBOSE
                        printf ("visibilitysensor - now active\n");
                        #endif

                        node->isActive = 1;
                        node->enterTime = TickTime;
                        MARK_EVENT (ptr, offsetof(struct X3D_VisibilitySensor, isActive));
                        MARK_EVENT (ptr, offsetof(struct X3D_VisibilitySensor, enterTime));

                }
	} else {
		/* we are here... */
		if (node->isActive) {
                        #ifdef SEVERBOSE
                        printf ("visibilitysensor - going inactive\n");
                        #endif

                        node->isActive = 0;
                        node->exitTime = TickTime;
                        MARK_EVENT (ptr, offsetof(struct X3D_VisibilitySensor, isActive));
                        MARK_EVENT (ptr, offsetof(struct X3D_VisibilitySensor, exitTime));
		}
	}
}