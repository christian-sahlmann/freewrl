/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Lighting.c,v 1.6 2009/05/11 21:11:59 crc_canada Exp $

X3D Lighting Component

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "RenderFuncs.h"
#include "../opengl/OpenGL_Utils.h"

/* pointLights are done before the rendering of geometry */
void prep_DirectionalLight (struct X3D_DirectionalLight *node) {

	if (!render_light) return;
	/* this will be a global light here... */
	render_DirectionalLight(node);
}

void render_DirectionalLight (struct X3D_DirectionalLight *node) {
	/* NOTE: This is called by the Group Children code
	 * at the correct point (in the beginning of the rendering
	 * of the children. We just turn the light on right now.
	 */

	if(node->on) {
		int light = nextlight();
		if(light >= 0) {
			float vec[4];
			/* glEnable(light); */
			lightState(light-GL_LIGHT0,TRUE);
			vec[0] = -((node->direction).c[0]);
			vec[1] = -((node->direction).c[1]);
			vec[2] = -((node->direction).c[2]);
			vec[3] = 0;
			glLightfv(light, GL_POSITION, vec);
			vec[0] = ((node->color).c[0]) * (node->intensity);
			vec[1] = ((node->color).c[1]) * (node->intensity);
			vec[2] = ((node->color).c[2]) * (node->intensity);
			vec[3] = 1;
			glLightfv(light, GL_DIFFUSE, vec);
			glLightfv(light, GL_SPECULAR, vec);

			/* Aubrey Jaffer */
			vec[0] = ((node->color).c[0]) * (node->ambientIntensity);
			vec[1] = ((node->color).c[1]) * (node->ambientIntensity);
			vec[2] = ((node->color).c[2]) * (node->ambientIntensity);

			glLightfv(light, GL_AMBIENT, vec);
		}
	}
}



/* pointLights are done before the rendering of geometry */
void prep_PointLight (struct X3D_PointLight *node) {

	if (!render_light) return;
	/* this will be a global light here... */
	render_PointLight(node);
}

void render_PointLight (struct X3D_PointLight *node) {
                if(node->on) {
                        int light = nextlight();
                        if(light >= 0) {
                                float vec[4];
				lightState(light-GL_LIGHT0,TRUE);
                                vec[0] = ((node->direction).c[0]);
                                vec[1] = ((node->direction).c[1]);
                                vec[2] = ((node->direction).c[2]);
                                vec[3] = 1;
                                glLightfv(light, GL_SPOT_DIRECTION, vec);
                                vec[0] = ((node->location).c[0]);
                                vec[1] = ((node->location).c[1]);
                                vec[2] = ((node->location).c[2]);
                                vec[3] = 1;
                                glLightfv(light, GL_POSITION, vec);

                                glLightf(light, GL_CONSTANT_ATTENUATION,
                                        ((node->attenuation).c[0]));
                                glLightf(light, GL_LINEAR_ATTENUATION,
                                        ((node->attenuation).c[1]));
                                glLightf(light, GL_QUADRATIC_ATTENUATION,
                                        ((node->attenuation).c[2]));


                                vec[0] = ((node->color).c[0]) * (node->intensity);
                                vec[1] = ((node->color).c[1]) * (node->intensity);
                                vec[2] = ((node->color).c[2]) * (node->intensity);
                                vec[3] = 1;
                                glLightfv(light, GL_DIFFUSE, vec);
                                glLightfv(light, GL_SPECULAR, vec);

                                /* Aubrey Jaffer */
                                vec[0] = ((node->color).c[0]) * (node->ambientIntensity);
                                vec[1] = ((node->color).c[1]) * (node->ambientIntensity);
                                vec[2] = ((node->color).c[2]) * (node->ambientIntensity);
                                glLightfv(light, GL_AMBIENT, vec);

                                /* XXX */
                                glLightf(light, GL_SPOT_CUTOFF, 180);
                        }
                }
}


/* SpotLights are done before the rendering of geometry */
void prep_SpotLight (struct X3D_SpotLight *node) {
	float ft;
	if (!render_light) return;
	render_SpotLight(node);
}

void render_SpotLight(struct X3D_SpotLight *node) {
	float ft;
                if(node->on) {
                        int light = nextlight();
                        if(light >= 0) {
                                float vec[4];
                                lightState(light-GL_LIGHT0,TRUE);
               
                                vec[0] = ((node->direction).c[0]);
                                vec[1] = ((node->direction).c[1]);
                                vec[2] = ((node->direction).c[2]);
                                vec[3] = 1;
                                glLightfv(light, GL_SPOT_DIRECTION, vec);
                                vec[0] = ((node->location).c[0]);
                                vec[1] = ((node->location).c[1]);
                                vec[2] = ((node->location).c[2]);
                                vec[3] = 1;
                                glLightfv(light, GL_POSITION, vec);
        
                                glLightf(light, GL_CONSTANT_ATTENUATION,
                                        ((node->attenuation).c[0]));
                                glLightf(light, GL_LINEAR_ATTENUATION,
                                        ((node->attenuation).c[1]));
                                glLightf(light, GL_QUADRATIC_ATTENUATION,
                                        ((node->attenuation).c[2]));
        
        
                                vec[0] = ((node->color).c[0]) * (node->intensity);
                                vec[1] = ((node->color).c[1]) * (node->intensity);
                                vec[2] = ((node->color).c[2]) * (node->intensity);
                                vec[3] = 1; 
                                glLightfv(light, GL_DIFFUSE, vec);
                                glLightfv(light, GL_SPECULAR, vec);

                                /* Aubrey Jaffer */
                                vec[0] = ((node->color).c[0]) * (node->ambientIntensity);
                                vec[1] = ((node->color).c[1]) * (node->ambientIntensity);
                                vec[2] = ((node->color).c[2]) * (node->ambientIntensity);

                                glLightfv(light, GL_AMBIENT, vec);

				ft = 0.5/(node->beamWidth +0.1);
				if (ft>128.0) ft=128.0;
				if (ft<0.0) ft=0.0;
                                glLightf(light, GL_SPOT_EXPONENT,ft);

				ft = node->cutOffAngle /3.1415926536*180;
				if (ft>90.0) ft=90.0;
				if (ft<0.0) ft=0.0;
                                glLightf(light, GL_SPOT_CUTOFF, ft);
                        }
                }
        }
