/*
=INSERT_TEMPLATE_HERE=

$Id: Component_EnvironSensor.c,v 1.15 2010/09/21 20:00:25 crc_canada Exp $

X3D Environmental Sensors Component

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

#include "LinearAlgebra.h"
#include "Component_Geospatial.h"
#include "../opengl/Frustum.h"
#include "../opengl/OpenGL_Utils.h"
#include "../scenegraph/RenderFuncs.h"


/* can we do a VisibiltySensor? Only if we have OpenGL support for OcclusionCulling */
int candoVisibility = TRUE;

static void rendVisibilityBox (struct X3D_VisibilitySensor *node);

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

		if (render_blend) { 
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

static void rendVisibilityBox (struct X3D_VisibilitySensor *node) {
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


	/* printf ("VISIBILITY BOXc vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
         render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	if NODE_NEEDS_COMPILING {
		/*  have to regen the shape*/
		MARK_NODE_COMPILED

		/*  MALLOC memory (if possible)*/
		if (!node->__points) node->__points = MALLOC (sizeof(struct SFColor)*(36));

		/*  now, create points; 4 points per face.*/
		pt = (float *) node->__points;

		#define PTF0 *pt++ = cx+x; *pt++ = cy+y; *pt++ = cz+z;
		#define PTF1 *pt++ = cx-x; *pt++ = cy+y; *pt++ = cz+z;
		#define PTF2 *pt++ = cx-x; *pt++ = cy-y; *pt++ = cz+z;
		#define PTF3 *pt++ = cx+x; *pt++ = cy-y; *pt++ = cz+z;
		#define PTR0 *pt++ = cx+x; *pt++ = cy+y; *pt++ = cz-z;
		#define PTR1 *pt++ = cx-x; *pt++ = cy+y; *pt++ = cz-z;
		#define PTR2 *pt++ = cx-x; *pt++ = cy-y; *pt++ = cz-z;
		#define PTR3 *pt++ = cx+x; *pt++ = cy-y; *pt++ = cz-z;


		PTF0 PTF1 PTF2  PTF0 PTF2 PTF3 /* front */
		PTR2 PTR1 PTR0  PTR3 PTR2 PTR0 /* back  */
		PTF0 PTR0 PTR1  PTF0 PTR1 PTF1 /* top   */
		PTF3 PTF2 PTR2  PTF3 PTR2 PTR3 /* bottom */
		PTF0 PTF3 PTR3 	PTF0 PTR3 PTR0 /* right */
		PTF1 PTR1 PTR2  PTF1 PTR2 PTF2 /* left */

		/* finished, and have good data */
	}

	FW_GL_DEPTHMASK(FALSE);
	/* note the ALPHA of zero - totally transparent */
	FW_GL_COLOR4F(0.0f, 1.0f, 0.0f, 0.0f);

	/*  Draw it; assume VERTEX and NORMALS already defined.*/
	FW_GL_VERTEX_POINTER(3,GL_FLOAT,0,(GLfloat *)node->__points);
	FW_GL_NORMAL_POINTER(GL_FLOAT,0,boxnorms);

	/* do the array drawing; sides are simple 0-1-2-3, 4-5-6-7, etc quads */
	FW_GL_DRAWARRAYS (GL_TRIANGLES, 0, 36);
	FW_GL_DEPTHMASK(TRUE);
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
