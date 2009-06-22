/*
=INSERT_TEMPLATE_HERE=

$Id: Component_VRML1.c,v 1.4 2009/06/22 19:40:41 crc_canada Exp $

X3D VRML1 Component

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"

#include "../opengl/OpenGL_Utils.h"
#include "../scenegraph/RenderFuncs.h"
#include "../scenegraph/Vector.h"


#include "LinearAlgebra.h"

#define VRML1CHILDREN_COUNT int nc = node->VRML1children.n;

#define DIVS 18

#ifndef ID_UNDEFINED
#define ID_UNDEFINED	((indexT)-1)
#endif

#ifdef M_PI
#define PI M_PI
#else
#define PI 3.141592653589793
#endif

/* Faster trig macros (thanks for Robin Williams) */

#define DECL_TRIG1 float t_aa, t_ab, t_sa, t_ca;
#define INIT_TRIG1(div) t_aa = sin(PI/(div)); t_aa *= 2*t_aa; t_ab = sin(2*PI/(div));
#define START_TRIG1 t_sa = 0; t_ca = 1;
#define UP_TRIG1 {float t_sa1 = t_sa; t_sa -= t_sa*t_aa - t_ca * t_ab; t_ca -= t_ca * t_aa + t_sa1 * t_ab;}
#define SIN1 t_sa
#define COS1 t_ca
#define DECL_TRIG2 float t2_aa, t2_ab;
#define INIT_TRIG2(div) t2_aa = sin(PI/(div)); t2_aa *= 2*t2_aa; t2_ab = sin(2*PI/(div));

#define TC(a,b) glTexCoord2f(a,b)

extern int findFieldInVRML1Modifier(const char*);	/* defined in convert1to2.c */

struct currentSLDPointer {
	struct X3D_VRML1_Material *matNode;
	struct X3D_VRML1_Coordinate3  *c3Node;
	struct X3D_VRML1_FontStyle  *fsNode;
	struct X3D_VRML1_MaterialBinding  *mbNode;
	struct X3D_VRML1_NormalBinding  *nbNode;
	struct X3D_VRML1_Normal  *nNode;
	struct X3D_VRML1_Texture2  *t2Node;
	struct X3D_VRML1_Texture2Transform  *t2tNode;
	struct X3D_VRML1_TextureCoordinate2  *tc2Node;
	struct X3D_VRML1_ShapeHints  *shNode;
};

static struct Vector *separatorVector = NULL;
static indexT separatorLevel = ID_UNDEFINED;
static struct currentSLDPointer *cSLD = NULL;

static struct currentSLDPointer *new_cSLD(void) {
	struct currentSLDPointer *retval = MALLOC (sizeof (struct currentSLDPointer));
	return retval;
}


#define MAX_STACK_LEVELS 32
#define GET_cSLD \
	/* bounds check this vector array - if it is off, it is a user input problem */ \
	if (separatorLevel <0) separatorLevel=0; \
	if (separatorLevel >=MAX_STACK_LEVELS) separatorLevel=MAX_STACK_LEVELS; \
	cSLD = vector_get(struct currentSLDPointer*, separatorVector, separatorLevel);

#define CLEAN_cSLD \
	{ \
	cSLD->matNode = NULL; \
	cSLD->c3Node = NULL; \
	cSLD->t2Node = NULL; \
        cSLD->fsNode = NULL; \
        cSLD->nbNode = NULL; \
        cSLD->nNode = NULL; \
        cSLD->mbNode = NULL; \
        cSLD->t2tNode = NULL; \
        cSLD->tc2Node = NULL; \
        cSLD->shNode = NULL; \
	}

/* if the user wants to change materials for IndexedLineSets, IndexedFaceSets or PointSets */
/* this is just like render_VRML1_Material, except that it allows different indexes */
static void renderSpecificMaterial (int ind) {
	int i;
	float dcol[4];
	float ecol[4];
	float scol[4];
	float trans=1.0;
	struct X3D_VRML1_Material *node;

	if (cSLD == NULL) return;
	if (cSLD->matNode == NULL) return;
	node = cSLD->matNode;

	#define whichFace GL_FRONT_AND_BACK

	/* set the transparency here for the material */
	if(node->transparency.n>ind)
	trans = 1.0 - node->transparency.p[ind];

	if (trans<0.0) trans = 0.0;
	if (trans>=0.999999) trans = 0.9999999;
	global_transparency = trans;

	dcol[3] = trans;
	scol[3] = trans;
	ecol[3] = trans;

	if (node->diffuseColor.n>ind)  {
		for (i=0; i<3;i++){ dcol[i] = node->diffuseColor.p[ind].c[i]; }		
	} else {
		for (i=0; i<3;i++){ dcol[i] = 0.8; }		
	}
	do_glMaterialfv(whichFace, GL_DIFFUSE, dcol);

	if (node->ambientColor.n>ind)  {
		for (i=0; i<3;i++){ dcol[i] *= node->ambientColor.p[ind].c[i]; }		
	} else {
		for (i=0; i<3;i++){ dcol[i] *= 0.2; }		
	}
	do_glMaterialfv(whichFace, GL_AMBIENT, dcol);

	if (node->specularColor.n>ind)  {
		for (i=0; i<3;i++){ scol[i] = node->specularColor.p[ind].c[i]; }		
	} else {
		for (i=0; i<3;i++){ scol[i] = 0.0; }		
	}
	do_glMaterialfv(whichFace, GL_SPECULAR, scol);
		\
	if (node->emissiveColor.n>ind)  {
		for (i=0; i<3;i++){ ecol[i] = node->emissiveColor.p[ind].c[i]; }		
	} else {
		for (i=0; i<3;i++){ ecol[i] = 0.0; }		
	}

	do_glMaterialfv(whichFace, GL_EMISSION, ecol);
	
	if (node->shininess.n>ind)
	do_shininess(whichFace,node->shininess.p[ind]);
}

/* do transforms, calculate the distance */
void prep_VRML1_Separator (struct X3D_VRML1_Separator *node) {
	/* printf ("prepSep %u\n",node); */

	/* lets push on to the stack... */
	separatorLevel++;
	/* do we need to create a new vector stack? */
	if (separatorVector == NULL) {
		indexT i;
		/* printf ("creating new vector list\n"); */
		separatorVector = newVector (struct currentSLDPointer*, MAX_STACK_LEVELS);
		ASSERT(separatorVector);
		for (i=0; i<MAX_STACK_LEVELS; i++) {
			cSLD = new_cSLD();
			vector_pushBack(struct currentSLDPointer*, separatorVector, cSLD);

		} 
	}
	/* printf ("prep - getting level %d\n",separatorLevel); */
	GET_cSLD;
	CLEAN_cSLD;
	FW_GL_PUSH_MATRIX();
}


void fin_VRML1_Separator (struct X3D_VRML1_Separator *node) {
	/* printf ("finSep %u\n",node); */
	FW_GL_POP_MATRIX();
	separatorLevel--;
	if (separatorLevel == ID_UNDEFINED) {
		/* printf ("CLEAN UP SEP STACK\n"); */
	} else {
		/* printf ("getting seplevel %d\n",separatorLevel); */
		GET_cSLD;
	}

	/* any simple uses for material, re-do material node */
	if (cSLD->matNode != NULL) {
		render_VRML1_Material(cSLD->matNode);
	}
} 

void child_VRML1_Separator (struct X3D_VRML1_Separator *node) { 
	LOCAL_LIGHT_SAVE

	/* printf ("vhild_sep %u, vp %d geom %d light %d sens %d blend %d prox %d col %d\n",node,
	render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision);   */

	/* we do not do collisions */
	if (render_collision) return;

/*	RETURN_FROM_CHILD_IF_NOT_FOR_ME */

	/* do we have a local light for a child? */
	LOCAL_LIGHT_CHILDREN(node->VRML1children);

	/* now, just render the non-directionalLight children */
	normalChildren(node->VRML1children);

	LOCAL_LIGHT_OFF
}


void render_VRML1_Cone (struct X3D_VRML1_Cone *node) {

	int div = DIVS;
	float df = div;
	float h = (node->height)/2;
	float r = (node->bottomRadius); 
	int i;
	int doSide = FALSE;
	int doBottom = FALSE;
	
	DECL_TRIG1
		
	if (!strcmp(node->parts->strptr,"BOTTOM")) {
		doBottom = TRUE;
	}
	if (!strcmp(node->parts->strptr,"SIDES")) {
		doSide = TRUE;
	}
	if (!strcmp(node->parts->strptr,"ALL")) {
		doSide = TRUE;
		doBottom = TRUE;
	}
	

	if(h <= 0 && r <= 0) {return;}
	INIT_TRIG1(div)
	if(doBottom) {
		glBegin(GL_POLYGON);
		glNormal3f(0,-1,0);
		START_TRIG1
		for(i=div-1; i>=0; i--) {
			TC(0.5+0.5*-SIN1,0.5+0.5*COS1);
			glVertex3f(r*-SIN1,-h,r*COS1);
			UP_TRIG1
		}
		glEnd();
	}
	if(doSide) {
		double ml = sqrt(h*h + r * r);
		double mlh = h / ml;
		double mlr = r / ml;
		glBegin(GL_TRIANGLES);
		START_TRIG1
		for(i=0; i<div; i++) {
			float lsin = SIN1;
			float lcos = COS1;
			UP_TRIG1;
			glNormal3f(mlh*lsin,mlr,-mlh*lcos);
			TC((i+0.5)/df,0);
			glVertex3f(0,h,0);
			glNormal3f(mlh*SIN1,mlr,-mlh*COS1);
			TC((i+1)/df,1);
			glVertex3f(r*SIN1,-h,-r*COS1);
			glNormal3f(mlh*lsin,mlr,-mlh*lcos);
			TC(i/df,1);
			glVertex3f(r*lsin,-h,-r*lcos);
		}
		glEnd();
	}
}

void render_VRML1_Cube (struct X3D_VRML1_Cube *node) {
	 float x = (node->width)/2;
	 float y = (node->height)/2;
	 float z = (node->depth)/2;
	 
	glPushAttrib(GL_LIGHTING);
	glShadeModel(GL_FLAT);
		glBegin(GL_QUADS);
		glNormal3f(0,0,1);
		TC(1,1);
		glVertex3f(x,y,z);
		TC(0,1);
		glVertex3f(-x,y,z);
		TC(0,0);
		glVertex3f(-x,-y,z);
		TC(1,0);
		glVertex3f(x,-y,z);

		glNormal3f(0,0,-1);
		TC(1,0);
		glVertex3f(x,-y,-z);
		TC(0,0);
		glVertex3f(-x,-y,-z);
		TC(0,1);
		glVertex3f(-x,y,-z);
		TC(1,1);
		glVertex3f(x,y,-z);

		glNormal3f(0,1,0);
		TC(1,1);
		glVertex3f(x,y,z);
		TC(1,0);
		glVertex3f(x,y,-z);
		TC(0,0);
		glVertex3f(-x,y,-z);
		TC(0,1);
		glVertex3f(-x,y,z);

		glNormal3f(0,-1,0);
		TC(0,1);
		glVertex3f(-x,-y,z);
		TC(0,0);
		glVertex3f(-x,-y,-z);
		TC(1,0);
		glVertex3f(x,-y,-z);
		TC(1,1);
		glVertex3f(x,-y,z);

		glNormal3f(1,0,0);
		TC(1,1);
		glVertex3f(x,y,z);
		TC(0,1);
		glVertex3f(x,-y,z);
		TC(0,0);
		glVertex3f(x,-y,-z);
		TC(1,0);
		glVertex3f(x,y,-z);

		glNormal3f(-1,0,0);
		TC(1,0);
		glVertex3f(-x,y,-z);
		TC(0,0);
		glVertex3f(-x,-y,-z);
		TC(0,1);
		glVertex3f(-x,-y,z);
		TC(1,1);
		glVertex3f(-x,y,z);
		glEnd();
	glPopAttrib();
}

void render_VRML1_Sphere (struct X3D_VRML1_Sphere *node){
	int vdiv = DIVS;
	int hdiv = DIVS;
	float vf = DIVS;
	float hf = DIVS;
	int v; int h;
	float va1,va2,van,ha1,ha2,han;
	DECL_TRIG1
	DECL_TRIG2 
	INIT_TRIG1(vdiv) 
	INIT_TRIG2(hdiv)
		
	glPushMatrix();
	glScalef(node->radius, node->radius, node->radius);
	START_TRIG1
	glBegin(GL_QUAD_STRIP);
	for(v=0; v<vdiv; v++) {
		va1 = v * 3.15 / vdiv;
		va2 = (v+1) * 3.15 / vdiv;
		van = (v+0.5) * 3.15 / vdiv;
		for(h=0; h<=hdiv; h++) {
			ha1 = h * 6.29 / hdiv;
			ha2 = (h+1) * 6.29 / hdiv;
			han = (h+0.5) * 6.29 / hdiv;

			glNormal3f(sin(va1) * cos(ha1), sin(va1) * sin(ha1), cos(va1));
			TC(v/vf,h/hf);
			glVertex3f(sin(va1) * cos(ha1), sin(va1) * sin(ha1), cos(va1));

			glNormal3f(sin(va2) * cos(ha1), sin(va2) * sin(ha1), cos(va2));
			TC((v+1)/vf,h/hf);
			glVertex3f(sin(va2) * cos(ha1), sin(va2) * sin(ha1), cos(va2));
		}
	}
	glEnd();
	glPopMatrix();
}

void render_VRML1_Cylinder (struct X3D_VRML1_Cylinder *node) {
	int div = DIVS;
	float df = div;
	float h = (node->height)/2;
	float r = (node->radius);
	DECL_TRIG1
	int i;
	int doTop = FALSE;
	int doBottom = FALSE;
	int doSide = FALSE;
		
	if (!strcmp(node->parts->strptr,"BOTTOM")) {
		doBottom = TRUE;
	}
	if (!strcmp(node->parts->strptr,"TOP")) {
		doTop = TRUE;
	}
	if (!strcmp(node->parts->strptr,"SIDES")) {
		doSide = TRUE;
	}
	if (!strcmp(node->parts->strptr,"ALL")) {
		doSide = TRUE;
		doBottom = TRUE;
		doTop = TRUE;
	}
	

	INIT_TRIG1(div)
	if(doBottom) {
		glBegin(GL_POLYGON);
		glNormal3f(0,1,0);
		START_TRIG1
		for(i=0; i<div; i++) {
			TC(0.5+0.5*SIN1,0.5+0.5*SIN1);
			glVertex3f(r*SIN1,h,r*COS1);
			UP_TRIG1
		}
		glEnd();
	} 
	if(doTop) {
		glBegin(GL_POLYGON);
		glNormal3f(0,-1,0);
		START_TRIG1
		for(i=div-1; i>=0; i--) {
			TC(0.5+0.5*-SIN1,0.5+0.5*COS1);
			glVertex3f(-r*SIN1,-h,r*COS1);
			UP_TRIG1
		}
		glEnd();
	}
	if(doSide) {
		glBegin(GL_QUADS);
		START_TRIG1
		for(i=0; i<div; i++) {
			float lsin = SIN1;
			float lcos = COS1;
			UP_TRIG1;
			glNormal3f(lsin,0,lcos);
			TC(i/df,0);
			glVertex3f(r*lsin,-h,r*lcos);
			glNormal3f(SIN1,0,COS1);
			TC((i+1)/df,0);
			glVertex3f(r*SIN1,-h,r*COS1);
			TC((i+1)/df,1);
			glVertex3f(r*SIN1,h,r*COS1);
			glNormal3f(lsin,0,lcos);
			TC(i/df,1);
			glVertex3f(r*lsin,h,r*lcos);
		}
		glEnd();
	}
}

void render_VRML1_Scale (struct X3D_VRML1_Scale *node) {
	glScalef(node->scaleFactor.c[0], node->scaleFactor.c[1], node->scaleFactor.c[2]);
}

void render_VRML1_Transform (struct X3D_VRML1_Transform *node) {
	/* from spec: 
Transform {
    translation T1
    rotation R1
    scaleFactor S
    scaleOrientation R2
    center T2
  }
is equivalent to the sequence:

Translation { translation T1 }
Translation { translation T2 }
Rotation { rotation R1 }
Rotation { rotation R2 }
Scale { scaleFactor S }
Rotation { rotation -R2 }
Translation { translation -T2 }
*/

	
	glTranslatef(node->translation.c[0], node->translation.c[1], node->translation.c[2]);
	glTranslatef(node->center.c[0], node->center.c[1], node->center.c[2]);
	glRotatef(node->rotation.c[3]/3.1415926536*180, node->rotation.c[0], node->rotation.c[1], node->rotation.c[2]);
	glRotatef(node->scaleOrientation.c[3]/3.1415926536*180, node->scaleOrientation.c[0], node->scaleOrientation.c[1], node->scaleOrientation.c[2]);
	glScalef(node->scaleFactor.c[0], node->scaleFactor.c[1], node->scaleFactor.c[2]);
	glRotatef(-node->rotation.c[3]/3.1415926536*180, node->rotation.c[0], node->rotation.c[1], node->rotation.c[2]);
	glTranslatef(-node->center.c[0], -node->center.c[1], -node->center.c[2]);
}

void render_VRML1_Translation (struct X3D_VRML1_Translation *node) {
	glTranslatef(node->translation.c[0], node->translation.c[1], node->translation.c[2]);
}



void render_VRML1_Material (struct X3D_VRML1_Material *node) {
	int i;
	float dcol[4];
	float ecol[4];
	float scol[4];
	float trans=1.0;

	#define whichFace GL_FRONT_AND_BACK

	/* save this node pointer */
	if (cSLD!=NULL) cSLD->matNode = node;


	/* set the transparency here for the material */
	if(node->transparency.n>0)
	trans = 1.0 - node->transparency.p[0];

	if (trans<0.0) trans = 0.0;
	if (trans>=0.999999) trans = 0.9999999;
	global_transparency = trans;

	dcol[3] = trans;
	scol[3] = trans;
	ecol[3] = trans;

	if (node->diffuseColor.n>0)  {
		for (i=0; i<3;i++){ dcol[i] = node->diffuseColor.p[0].c[i]; }		
	} else {
		for (i=0; i<3;i++){ dcol[i] = 0.8; }		
	}
	do_glMaterialfv(whichFace, GL_DIFFUSE, dcol);

	/* do the ambientIntensity; this will allow lights with ambientIntensity to
	   illuminate it as per the spec. Note that lights have the ambientIntensity
	   set to 0.0 by default; this should make ambientIntensity lighting be zero
	   via OpenGL lighting equations. */
	if (node->ambientColor.n>0)  {
		for (i=0; i<3;i++){ dcol[i] *= node->ambientColor.p[0].c[i]; }		
	} else {
		for (i=0; i<3;i++){ dcol[i] *= 0.2; }		
	}
	do_glMaterialfv(whichFace, GL_AMBIENT, dcol);

	if (node->specularColor.n>0)  {
		for (i=0; i<3;i++){ scol[i] = node->specularColor.p[0].c[i]; }		
	} else {
		for (i=0; i<3;i++){ scol[i] = 0.0; }		
	}
	do_glMaterialfv(whichFace, GL_SPECULAR, scol);
		\
	if (node->emissiveColor.n>0)  {
		for (i=0; i<3;i++){ ecol[i] = node->emissiveColor.p[0].c[i]; }		
	} else {
		for (i=0; i<3;i++){ ecol[i] = 0.0; }		
	}

	do_glMaterialfv(whichFace, GL_EMISSION, ecol);
	
	if (node->shininess.n>0)
	do_shininess(whichFace,node->shininess.p[0]);
}

void render_VRML1_Rotation (struct X3D_VRML1_Rotation *node) {
	glRotatef(node->rotation.c[3]/3.1415926536*180, node->rotation.c[0], node->rotation.c[1], node->rotation.c[2]);
}

void render_VRML1_DirectionalLight (struct X3D_VRML1_DirectionalLight *node) {
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

			vec[0] = ((node->color).c[0]) * 0.2;
			vec[1] = ((node->color).c[1]) * 0.2;
			vec[2] = ((node->color).c[2]) * 0.2;

			glLightfv(light, GL_AMBIENT, vec);
		}
	}


}

void render_VRML1_PointLight (struct X3D_VRML1_PointLight *node) {
	if(node->on) {
		int light = nextlight();
		if(light >= 0) {
			float vec[4];
			lightState(light-GL_LIGHT0,TRUE);
			#ifdef VRML2
			vec[0] = ((node->direction).c[0]);
			vec[1] = ((node->direction).c[1]);
			vec[2] = ((node->direction).c[2]);
			vec[3] = 1;
			glLightfv(light, GL_SPOT_DIRECTION, vec);
			#endif

			vec[0] = ((node->location).c[0]);
			vec[1] = ((node->location).c[1]);
			vec[2] = ((node->location).c[2]);
			vec[3] = 1;
			glLightfv(light, GL_POSITION, vec);

			glLightf(light, GL_CONSTANT_ATTENUATION, 1.0);
			glLightf(light, GL_LINEAR_ATTENUATION, 0.0);
			glLightf(light, GL_QUADRATIC_ATTENUATION, 0.0);

			vec[0] = ((node->color).c[0]) * (node->intensity);
			vec[1] = ((node->color).c[1]) * (node->intensity);
			vec[2] = ((node->color).c[2]) * (node->intensity);
			vec[3] = 1;
			glLightfv(light, GL_DIFFUSE, vec);
			glLightfv(light, GL_SPECULAR, vec);

			vec[0] = ((node->color).c[0]) * 0.2;
			vec[1] = ((node->color).c[1]) * 0.2;
			vec[2] = ((node->color).c[2]) * 0.2;
			glLightfv(light, GL_AMBIENT, vec);

			/* XXX */
			glLightf(light, GL_SPOT_CUTOFF, 180);
		}
	}


}

void render_VRML1_SpotLight (struct X3D_VRML1_SpotLight *node) {
	if(node->on) {
		int light = nextlight();
		if(light >= 0) {
			float vec[4];
			float ft;
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

			glLightf(light, GL_CONSTANT_ATTENUATION,1.0);
			glLightf(light, GL_LINEAR_ATTENUATION,0.0);
			glLightf(light, GL_QUADRATIC_ATTENUATION,0.0);

			vec[0] = ((node->color).c[0]) * (node->intensity);
			vec[1] = ((node->color).c[1]) * (node->intensity);
			vec[2] = ((node->color).c[2]) * (node->intensity);
			vec[3] = 1;
			glLightfv(light, GL_DIFFUSE, vec);
			glLightfv(light, GL_SPECULAR, vec);

			vec[0] = ((node->color).c[0]) * 0.2;
			vec[1] = ((node->color).c[1]) * 0.2;
			vec[2] = ((node->color).c[2]) * 0.2;

			glLightfv(light, GL_AMBIENT, vec);

			ft = 0.5/(1.570796 +0.1); /* 1.570796 = default beamWidth in X3D */
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

void render_VRML1_Coordinate3 (struct X3D_VRML1_Coordinate3  *node) {
	/* save this node pointer */
	if (cSLD!=NULL) cSLD->c3Node = node;
}

void render_VRML1_FontStyle (struct X3D_VRML1_FontStyle  *node) {
	/* save this node pointer */
	if (cSLD!=NULL) cSLD->fsNode = node;
}

void render_VRML1_MaterialBinding (struct X3D_VRML1_MaterialBinding  *node) {
	if (!node->_initialized) {
		node->_Value = findFieldInVRML1Modifier(node->value->strptr);
		node->_initialized = TRUE;
	}

	/* save this node pointer */
	if (cSLD!=NULL) cSLD->mbNode = node;
}

void render_VRML1_Normal (struct X3D_VRML1_Normal  *node) {
	/* save this node pointer */
	if (cSLD!=NULL) cSLD->nNode = node;
}

void render_VRML1_NormalBinding (struct X3D_VRML1_NormalBinding  *node) {
	if (!node->_initialized) {
		node->_Value = findFieldInVRML1Modifier(node->value->strptr);
		node->_initialized = TRUE;
	}
	/* save this node pointer */
	if (cSLD!=NULL) cSLD->nbNode = node;
}

void render_VRML1_Texture2 (struct X3D_VRML1_Texture2  *node) {
	/* save this node pointer */
	if (cSLD!=NULL) cSLD->t2Node = node;
}

void render_VRML1_Texture2Transform (struct X3D_VRML1_Texture2Transform  *node) {
	/* save this node pointer */
	if (cSLD!=NULL) cSLD->t2tNode = node;
}

void render_VRML1_TextureCoordinate2 (struct X3D_VRML1_TextureCoordinate2  *node) {
	/* save this node pointer */
	if (cSLD!=NULL) cSLD->tc2Node = node;
}

void render_VRML1_ShapeHints (struct X3D_VRML1_ShapeHints  *node) {
	if (!node->_initialized) {
		node->_vertValue = findFieldInVRML1Modifier(node->vertexOrdering->strptr);
		node->_typeValue = findFieldInVRML1Modifier(node->shapeType->strptr);
		node->_faceValue = findFieldInVRML1Modifier(node->faceType->strptr);
		node->_initialized = TRUE;
	}
	/* save this node pointer */
	if (cSLD!=NULL) cSLD->shNode = node;
}

void render_VRML1_PointSet (struct X3D_VRML1_PointSet *this) {
        int i;
        struct SFColor *points; int npoints=0;
	int renderMatOver = FALSE;
	glPointSize (2);

        if(cSLD->c3Node) {
		points =  cSLD->c3Node->point.p;
		npoints = cSLD->c3Node->point.n;
	}
		
	if (cSLD->mbNode != NULL) {
		renderMatOver = !((cSLD->mbNode->_Value==VRML1MOD_OVERALL) || (cSLD->mbNode->_Value==VRML1MOD_DEFAULT));
	}

	/* do we have enough points? */
	if (npoints < (this->startIndex - this->numPoints)) { 
		printf ("PointSet Error, npoints %d, startIndex %d, numPoints %d, not enough...\n",
				npoints, this->startIndex, this->numPoints);
		this->numPoints = -1;
	}

        glBegin(GL_POINTS);
        for(i=this->startIndex; i<this->numPoints; i++) {
		if (renderMatOver) renderSpecificMaterial (i);
                glVertex3f(
                        points[i].c[0],
                        points[i].c[1],
                        points[i].c[2]
                );
        }
        glEnd();
}

void render_VRML1_IndexedFaceSet (struct X3D_VRML1_IndexedFaceSet *node) {

	/* if we need to remake this IndexedFaceSet */
	/* this is like the COMPILE_POLY_IF_REQUIRED(a,b,c,d) macro, with additional steps */

	if(!node->_intern || node->_change != ((struct X3D_PolyRep *)node->_intern)->irep_change) { 
		/* yes, we do have to remake this node */
		
		/* we need to fill in the hidden SFFloats, SFBools here */
		if (cSLD->shNode != NULL) {

			node->_ccw =  cSLD->shNode->_vertValue!=VRML1MOD_CLOCKWISE;
			node->_convex = cSLD->shNode->_faceValue == VRML1MOD_CONVEX;
			node->_solid = cSLD->shNode->_typeValue==VRML1MOD_SOLID;
			node->_creaseAngle = cSLD->shNode->creaseAngle;
		} else {
			node->_ccw = FALSE;
			node->_convex = TRUE;
			node->_solid = FALSE;
			node->_creaseAngle = 0.5;
		}

		if (cSLD->mbNode!=NULL) node->_cpv = cSLD->mbNode->_Value==VRML1MOD_PER_VERTEX;
		else node->_cpv = FALSE;

		if (cSLD->nbNode!=NULL) node->_npv = cSLD->nbNode->_Value==VRML1MOD_PER_VERTEX;
		else node->_npv = FALSE;

		if (cSLD->matNode!= NULL) {
			/* if we have more than 1 diffuseColor, we should treat them as a color node */
			if (cSLD->matNode->diffuseColor.n > 1) {
			struct X3D_Color *nc;
			if (node->_color==NULL) { 
				node->_color = createNewX3DNode(NODE_Color);
				ADD_PARENT(node->_color,node);
			}
			nc = node->_color;
			FREE_IF_NZ(nc->color.p);
			nc->color.p = MALLOC(sizeof (struct SFColor) * cSLD->matNode->diffuseColor.n);
			memcpy(nc->color.p, cSLD->matNode->diffuseColor.p, sizeof (struct SFColor) * cSLD->matNode->diffuseColor.n);
			nc->color.n = cSLD->matNode->diffuseColor.n;
/*printf ("created and copied %d colours\n",nc->color.n);
			{
			int i;
			for (i=0; i<nc->color.n; i++) {
			printf ("%d: %f %f %f\n",i,nc->color.p[i].c[0], nc->color.p[i].c[1],nc->color.p[i].c[2]);
			}
			}
*/
			}
		} else node->_color=NULL;
		
		if (cSLD->c3Node!= NULL) {
			struct X3D_Coordinate *nc;
			if (node->_coord==NULL) {
				node->_coord = createNewX3DNode(NODE_Coordinate);
				ADD_PARENT(node->_coord,node);
			}
			nc = node->_coord;
			FREE_IF_NZ(nc->point.p);
			nc->point.p = MALLOC(sizeof (struct SFColor) * cSLD->c3Node->point.n);
			memcpy(nc->point.p, cSLD->c3Node->point.p, sizeof (struct SFColor) * cSLD->c3Node->point.n);

			nc->point.n = cSLD->c3Node->point.n;
/*
printf ("created and copied %d coordinates\n",nc->point.n);
			{
			int i;
			for (i=0; i<cSLD->c3Node->point.n; i++) {
			printf ("%d: %f %f %f\n",i,nc->point.p[i].c[0], nc->point.p[i].c[1],nc->point.p[i].c[2]);
			}}
*/
		} else node->_coord=NULL;
		
		if (cSLD->nNode!= NULL) {
			if (node->_normal==NULL) node->_normal = createNewX3DNode(NODE_Normal);
			node->_normal=cSLD->nNode;
printf ("normal node prob\n");
		} else node->_normal=NULL;
		
		if (cSLD->tc2Node!= NULL) {
			if (node->_texCoord==NULL) node->_texCoord = createNewX3DNode(NODE_TextureCoordinate);
			node->_texCoord=cSLD->tc2Node;
printf ("texture coord node prob\n");
		} else node->_texCoord=NULL;

		compileNode ((void *)compile_polyrep, node, 
			node->_coord, node->_color, node->_normal, node->_texCoord);
	} 
	/* something went wrong... */
	if (!node->_intern) return;

	CULL_FACE(node->_solid)
	render_polyrep(node);
}

void render_VRML1_IndexedLineSet (struct X3D_VRML1_IndexedLineSet *node) {
        int cin = node->coordIndex.n;
	int ind;
	int i;
	int colorI = 0;

        struct SFColor *points; int npoints=0;
	int renderMatOver = VRML1MOD_OVERALL;
	int doMat = 0;

	glLineWidth(2);

        if(cSLD->c3Node) {
		points =  cSLD->c3Node->point.p;
		npoints = cSLD->c3Node->point.n;
	}
		
	if (cSLD->mbNode != NULL) {
		renderMatOver = cSLD->mbNode->_Value;
	}
	
	glBegin(GL_LINE_STRIP);
	for(i=0; i<cin; i++) {
		if (i<node->coordIndex.n) ind = node->coordIndex.p[i];
		else {
			printf ("IndexedLineSet, index> avail coords\n");
			node->coordIndex.n = 0;
			glEnd();
			return;
		}
		if(ind==-1) {
			glEnd();
#ifdef xx
			plno++;
			if(ncolors && !cpv) {
				c = plno;
				if((!colin && plno < ncolors) ||
				   (colin && plno < colin)) {
					if(colin) {
						c = ((this_->colorIndex).p[c]);
					}
					glColor3f(colors[c].c[0],
						  colors[c].c[1],
						  colors[c].c[2]);
				}
			}
#endif
			glBegin(GL_LINE_STRIP);
		} else {
#ifdef xx
			if(ncolors && cpv) {
				c = i;
				if(colin) {
					c = ((this_->colorIndex).p[c]);
				}
				glColor3f(colors[c].c[0],
					  colors[c].c[1],
					  colors[c].c[2]);
			}
			/* printf("Line: vertex %f %f %f\n",
				points[ind].c[0],
				points[ind].c[1],
				points[ind].c[2]
			);
			*/
#endif
			glVertex3f(
				points[ind].c[0],
				points[ind].c[1],
				points[ind].c[2]
			);
		}
	}
	glEnd();
}



void render_VRML1_AsciiText (struct X3D_VRML1_AsciiText *this) {}
void render_VRML1_MatrixTransform (struct X3D_VRML1_MatrixTransform  *node) {}
void render_VRML1_Switch (struct X3D_VRML1_Switch  *node) {}
void render_VRML1_WWWAnchor (struct X3D_VRML1_WWWAnchor  *node) {}
void render_VRML1_LOD (struct X3D_VRML1_LOD  *node) {}
void render_VRML1_OrthographicCamera (struct X3D_VRML1_OrthographicCamera  *node) {}
void render_VRML1_PerspectiveCamera (struct X3D_VRML1_PerspectiveCamera  *node) {}
void render_VRML1_WWWInline (struct X3D_VRML1_WWWInline  *node) {}
