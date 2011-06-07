/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Lighting.c,v 1.17 2011/06/07 21:44:18 dug9 Exp $

X3D Lighting Component

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
#include "../main/headers.h"
#include "../opengl/OpenGL_Utils.h"
#include "RenderFuncs.h"
#include "../opengl/OpenGL_Utils.h"

#define RETURN_IF_LIGHT_STATE_NOT_US \
		if (renderstate()->render_light== VF_globalLight) { \
			if (!node->global) return;\
			/* printf ("and this is a global light\n"); */\
		} else if (node->global) return; \
		/* else printf ("and this is a local light\n"); */




void render_DirectionalLight (struct X3D_DirectionalLight *node) {

	/* if we are doing global lighting, is this one for us? */
	RETURN_IF_LIGHT_STATE_NOT_US

	if(node->on) {
		int light = nextlight();
		if(light >= 0) {
			float vec[4];
			lightState(light,TRUE);
			vec[0] = -((node->direction).c[0]);
			vec[1] = -((node->direction).c[1]);
			vec[2] = -((node->direction).c[2]);
			vec[3] = 0.0f; /* 0.0 = DirectionalLight */
			FW_GL_LIGHTFV(light, GL_POSITION, vec);
			vec[0] = ((node->color).c[0]) * (node->intensity);
			vec[1] = ((node->color).c[1]) * (node->intensity);
			vec[2] = ((node->color).c[2]) * (node->intensity);
			vec[3] = 1;
			FW_GL_LIGHTFV(light, GL_DIFFUSE, vec);
			FW_GL_LIGHTFV(light, GL_SPECULAR, vec);

			/* Aubrey Jaffer */
			vec[0] = ((node->color).c[0]) * (node->ambientIntensity);
			vec[1] = ((node->color).c[1]) * (node->ambientIntensity);
			vec[2] = ((node->color).c[2]) * (node->ambientIntensity);

			FW_GL_LIGHTFV(light, GL_AMBIENT, vec);
		}
	}
}

/* global lights  are done before the rendering of geometry */
void prep_DirectionalLight (struct X3D_DirectionalLight *node) {
	if (!renderstate()->render_light) return;
	render_DirectionalLight(node);
}



void render_PointLight (struct X3D_PointLight *node) {

	/* if we are doing global lighting, is this one for us? */
	RETURN_IF_LIGHT_STATE_NOT_US

	if(node->on) {
		int light = nextlight();
		if(light >= 0) {
			float vec[4];
			lightState(light,TRUE);
			vec[0] = 0.0f; vec[1] = 0.0f; vec[2] = -1.0f; vec[3] = 1.0f;

			FW_GL_LIGHTFV(light, GL_SPOT_DIRECTION, vec);
			vec[0] = ((node->location).c[0]);
			vec[1] = ((node->location).c[1]);
			vec[2] = ((node->location).c[2]);
			vec[3] = 1;
			FW_GL_LIGHTFV(light, GL_POSITION, vec);

			FW_GL_LIGHTF(light, GL_CONSTANT_ATTENUATION,
				((node->attenuation).c[0]));
			FW_GL_LIGHTF(light, GL_LINEAR_ATTENUATION,
				((node->attenuation).c[1]));
			FW_GL_LIGHTF(light, GL_QUADRATIC_ATTENUATION,
				((node->attenuation).c[2]));


			vec[0] = ((node->color).c[0]) * (node->intensity);
			vec[1] = ((node->color).c[1]) * (node->intensity);
			vec[2] = ((node->color).c[2]) * (node->intensity);
			vec[3] = 1;
			FW_GL_LIGHTFV(light, GL_DIFFUSE, vec);
			FW_GL_LIGHTFV(light, GL_SPECULAR, vec);

			/* Aubrey Jaffer */
			vec[0] = ((node->color).c[0]) * (node->ambientIntensity);
			vec[1] = ((node->color).c[1]) * (node->ambientIntensity);
			vec[2] = ((node->color).c[2]) * (node->ambientIntensity);
			FW_GL_LIGHTFV(light, GL_AMBIENT, vec);

			/* XXX */
			FW_GL_LIGHTF(light, GL_SPOT_CUTOFF, 180);
		}
	}
}

/* pointLights are done before the rendering of geometry */
void prep_PointLight (struct X3D_PointLight *node) {

	if (!renderstate()->render_light) return;
	/* this will be a global light here... */
	render_PointLight(node);
}



void render_SpotLight(struct X3D_SpotLight *node) {
	float ft;

	/* if we are doing global lighting, is this one for us? */
	RETURN_IF_LIGHT_STATE_NOT_US

	if(node->on) {
		int light = nextlight();
		if(light >= 0) {
			float vec[4];
			lightState(light,TRUE);
	       
			vec[0] = ((node->direction).c[0]);
			vec[1] = ((node->direction).c[1]);
			vec[2] = ((node->direction).c[2]);
			vec[3] = 1;
			FW_GL_LIGHTFV(light, GL_SPOT_DIRECTION, vec);
			vec[0] = ((node->location).c[0]);
			vec[1] = ((node->location).c[1]);
			vec[2] = ((node->location).c[2]);
			vec[3] = 1;
			FW_GL_LIGHTFV(light, GL_POSITION, vec);
	
			FW_GL_LIGHTF(light, GL_CONSTANT_ATTENUATION,
					((node->attenuation).c[0]));
			FW_GL_LIGHTF(light, GL_LINEAR_ATTENUATION,
					((node->attenuation).c[1]));
			FW_GL_LIGHTF(light, GL_QUADRATIC_ATTENUATION,
					((node->attenuation).c[2]));
	
	
			vec[0] = ((node->color).c[0]) * (node->intensity);
			vec[1] = ((node->color).c[1]) * (node->intensity);
			vec[2] = ((node->color).c[2]) * (node->intensity);
			vec[3] = 1; 
			FW_GL_LIGHTFV(light, GL_DIFFUSE, vec);
			FW_GL_LIGHTFV(light, GL_SPECULAR, vec);

			/* Aubrey Jaffer */
			vec[0] = ((node->color).c[0]) * (node->ambientIntensity);
			vec[1] = ((node->color).c[1]) * (node->ambientIntensity);
			vec[2] = ((node->color).c[2]) * (node->ambientIntensity);

			FW_GL_LIGHTFV(light, GL_AMBIENT, vec);

			ft = 0.5f/(node->beamWidth +0.1f);
			if (ft>128.0) ft=128.0f;
			if (ft<0.0) ft=0.0f;
			FW_GL_LIGHTF(light, GL_SPOT_EXPONENT,ft);

			ft = node->cutOffAngle /3.1415926536f*180.0f;
			if (ft>90.0) ft=90.0f;
			if (ft<0.0) ft=0.0f;
			FW_GL_LIGHTF(light, GL_SPOT_CUTOFF, ft);
		}
	}
}
/* SpotLights are done before the rendering of geometry */
void prep_SpotLight (struct X3D_SpotLight *node) {
	if (!renderstate()->render_light) return;
	render_SpotLight(node);
}
