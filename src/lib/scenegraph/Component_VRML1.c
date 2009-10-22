/*
=INSERT_TEMPLATE_HERE=

$Id: Component_VRML1.c,v 1.12 2009/10/22 16:58:49 crc_canada Exp $

X3D VRML1 Component

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




/* Here we render VRML1 nodes.

The VRML1 input string has been converted into X3D Classic encoding, and we have finished parsing.

Here we render the nodes. (well, most of them anyway!) We try and use as much of the X3D coding as
possible, so that future performance improvements will help these nodes, too.

Notes: 

- you can NOT route to these nodes. That is fine, the VRML1 spec does not support routing...

- you CAN interchange VRML1 nodes with VRML2; but if you write VRML1 nodes in a VRML2 program, you
  have to preface them with "VRML1_"...

- There were some "funnies" - Separator is, I think, ok - but I was not quite sure what HINTS flowed
  around Separators; this code might be in error.

- IndexedFaceSet - takes DiffuseColor if required as a Color node.

- Lines, takes emissiveColor from the Material node, if you specify an index for line segments.

- Separators do NOT do node sorting, scene graph pruning, whatever. It looks to me like these
  facilities might confuse rendering.

Have fun! John A. Stewart, June 2009

*/


#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"

#include "../opengl/OpenGL_Utils.h"
#include "../opengl/Material.h"
#include "../scenegraph/RenderFuncs.h"
#include "../scenegraph/Vector.h"
#include "../scenegraph/Component_Shape.h"


#include "LinearAlgebra.h"

#define VRML1CHILDREN_COUNT int nc = node->VRML1children.n;

#define DIVS 18

#ifndef ID_UNDEFINED
#define ID_UNDEFINED	((indexT)-1)
#endif

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
	appearanceProperties.transparency = trans;

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
	appearanceProperties.transparency = trans;

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

	/* did we have a textureTransform? */
	if (cSLD->t2tNode!=NULL) end_textureTransform();

	if (cSLD->t2tNode) FW_GL_DISABLE(GL_TEXTURE_2D);
} 

void child_VRML1_Separator (struct X3D_VRML1_Separator *node) { 
	LOCAL_LIGHT_SAVE

	/* ensure that textures start off at the first texture unit */
	texture_count = 0;

	/* printf ("vhild_sep %u, vp %d geom %d light %d sens %d blend %d prox %d col %d\n",node,
	render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision);  */

/*	RETURN_FROM_CHILD_IF_NOT_FOR_ME */

	/* do we have a local light for a child? */
	LOCAL_LIGHT_CHILDREN(node->VRML1children);

	/* now, just render the non-directionalLight children */
	normalChildren(node->VRML1children);

	LOCAL_LIGHT_OFF
}


void render_VRML1_Cone (struct X3D_VRML1_Cone *node) {
	if (node->_ILS==NULL) {
		struct X3D_Cone *sp = createNewX3DNode(NODE_Cone);
		node->_ILS = sp;

		sp->side = FALSE;
		sp->bottom = FALSE;
		if (!strcmp(node->parts->strptr,"BOTTOM")) {
			sp->bottom = TRUE;
		}
		if (!strcmp(node->parts->strptr,"SIDES")) {
			sp->side = TRUE;
		}
		if (!strcmp(node->parts->strptr,"ALL")) {
			sp->side = TRUE;
			sp->bottom = TRUE;
		}

		sp->bottomRadius = node->bottomRadius;
		sp->height = node->height;
	}
	render_node(node->_ILS);

}

void render_VRML1_Cube (struct X3D_VRML1_Cube *node) {
	if (node->_ILS==NULL) {
		struct X3D_Box *sp = createNewX3DNode(NODE_Box);
		node->_ILS = sp;
		sp->size.c[0] = node->width;
		sp->size.c[1] = node->height;
		sp->size.c[2] = node->depth;
	}
	render_node(node->_ILS);
}

void render_VRML1_Sphere (struct X3D_VRML1_Sphere *node){
	if (node->_ILS==NULL) {
		struct X3D_Sphere *sp = createNewX3DNode(NODE_Sphere);
		node->_ILS = sp;
		sp->radius = node->radius;
	}
	render_node(node->_ILS);
}

void render_VRML1_Cylinder (struct X3D_VRML1_Cylinder *node) {
	if (node->_ILS==NULL) {
		struct X3D_Cylinder *sp = createNewX3DNode(NODE_Cylinder);
		node->_ILS = sp;

		sp->side = FALSE;
		sp->bottom = FALSE;
		sp->top = FALSE;
		if (!strcmp(node->parts->strptr,"BOTTOM")) {
			sp->bottom = TRUE;
		}
		if (!strcmp(node->parts->strptr,"TOP")) {
			sp->top = TRUE;
		}
		if (!strcmp(node->parts->strptr,"SIDES")) {
			sp->side = TRUE;
		}
		if (!strcmp(node->parts->strptr,"ALL")) {
			sp->side = TRUE;
			sp->bottom = TRUE;
			sp->top = TRUE;
		}
		sp->radius = node->radius;
		sp->height = node->height;
	}
	render_node(node->_ILS);
	
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




void render_VRML1_Rotation (struct X3D_VRML1_Rotation *node) {
	glRotatef(node->rotation.c[3]/3.1415926536*180, node->rotation.c[0], node->rotation.c[1], node->rotation.c[2]);
}

void render_VRML1_DirectionalLight (struct X3D_VRML1_DirectionalLight *node) {
	if(node->on) {
		int light = nextlight();
		if(light >= 0) {
			float vec[4];
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
	if (!node->_initialized) {
		node->_wrapS = findFieldInVRML1Modifier(node->wrapS->strptr);
		node->_wrapT = findFieldInVRML1Modifier(node->wrapT->strptr);
		node->_initialized = TRUE;
	}
	/* save this node pointer */
	if (cSLD!=NULL) cSLD->t2Node = node;
	/* printf ("tex2, parent %s, me %s\n",node->__parenturl->strptr, node->filename.p[0]->strptr); */
        loadTextureNode(X3D_NODE(node),NULL);
        texture_count=1; /* not multitexture - should have saved to bound_textures[0] */
}

void render_VRML1_Texture2Transform (struct X3D_VRML1_Texture2Transform  *node) {
	/* save this node pointer */
	if (cSLD!=NULL) cSLD->t2tNode = node;
	/* call this, level is zero */
	start_textureTransform (X3D_NODE(node),0);
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
        struct SFColor *points=NULL; int npoints=0;
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



/********* IndexedFaceSet ******************************************************************/
/* Copy the data from sibling nodes into hidden fields of this VRML1 IndexedFaceSet, so that
   the FreeWRL PolyRep routines work properly */

static void copyPointersToVRML1IndexedFaceSet(struct X3D_VRML1_IndexedFaceSet *node) {
	/* yes, we do have to remake this node */
		
	/* we need to fill in the hidden SFFloats, SFBools here */
	if (cSLD->shNode != NULL) {
		node->_solid = cSLD->shNode->_typeValue==VRML1MOD_SOLID;
		node->_ccw =  cSLD->shNode->_vertValue==VRML1MOD_COUNTERCLOCKWISE;
		/* are we not sure? if not sure, make shape not SOLID so both sides of tris are drawn */
		if (cSLD->shNode->_vertValue==VRML1MOD_UNKNOWN_ORDERING) {
			node->_solid = FALSE;
		}
		
		node->_convex = cSLD->shNode->_faceValue == VRML1MOD_CONVEX;
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

	/* do we have a scoped Material node, with more than 1 diffuseColor? */
	if (cSLD->matNode!= NULL) {
		/* if we have more than 1 diffuseColor, we should treat them as a color node */
		if (cSLD->matNode->diffuseColor.n > 1) {
			struct X3D_Color *nc;
			if (node->_color==NULL) { 
				node->_color = createNewX3DNode(NODE_Color);
				ADD_PARENT(node->_color,X3D_NODE(node));
			}
			nc = node->_color;
			FREE_IF_NZ(nc->color.p);
			nc->color.p = MALLOC(sizeof (struct SFColor) * cSLD->matNode->diffuseColor.n);
			memcpy(nc->color.p, cSLD->matNode->diffuseColor.p, sizeof (struct SFColor) * cSLD->matNode->diffuseColor.n);
			nc->color.n = cSLD->matNode->diffuseColor.n;
		}
	} else node->_color=NULL;
		
	/* do we have a scoped Coordinate3 node? */
	if (cSLD->c3Node!= NULL) {
		struct X3D_Coordinate *nc;
		if (node->_coord==NULL) {
			node->_coord = createNewX3DNode(NODE_Coordinate);
			ADD_PARENT(node->_coord,X3D_NODE(node));
		}
		nc = node->_coord;
		FREE_IF_NZ(nc->point.p);
		nc->point.p = MALLOC(sizeof (struct SFColor) * cSLD->c3Node->point.n);
		memcpy(nc->point.p, cSLD->c3Node->point.p, sizeof (struct SFColor) * cSLD->c3Node->point.n);

		nc->point.n = cSLD->c3Node->point.n;
	} else node->_coord=NULL;
		
	/* do we have a Normal node? */
	if (cSLD->nNode!= NULL) {
		struct X3D_Normal *nc;
		if (node->_normal==NULL) {
			node->_normal = createNewX3DNode(NODE_Normal);
			ADD_PARENT(node->_normal,X3D_NODE(node));
		}
		nc = node->_normal;
		FREE_IF_NZ(nc->vector.p);
		nc->vector.p = MALLOC(sizeof (struct SFColor) * cSLD->nNode->vector.n);
		memcpy(nc->vector.p, cSLD->nNode->vector.p, sizeof (struct SFColor) * cSLD->nNode->vector.n);

		nc->vector.n = cSLD->nNode->vector.n;
	} else node->_normal=NULL;
		
	/* do we have a TextureCoordinate2 node? */
	if (cSLD->tc2Node!= NULL) {
		struct X3D_TextureCoordinate *nc;
		if (node->_texCoord==NULL) {
			node->_texCoord = createNewX3DNode(NODE_Normal);
			ADD_PARENT(node->_texCoord,X3D_NODE(node));
		}
		nc = node->_texCoord;
		FREE_IF_NZ(nc->point.p);
		nc->point.p = MALLOC(sizeof (struct SFVec2f) * cSLD->tc2Node->point.n);
		memcpy(nc->point.p, cSLD->tc2Node->point.p, sizeof (struct SFVec2f) * cSLD->tc2Node->point.n);

		nc->point.n = cSLD->tc2Node->point.n;
	} else node->_texCoord=NULL;
}


void render_VRML1_IndexedFaceSet (struct X3D_VRML1_IndexedFaceSet *node) {

	/* if we need to remake this IndexedFaceSet */
	/* this is like the COMPILE_POLY_IF_REQUIRED(a,b,c,d) macro, with additional steps */

	if(!node->_intern || node->_change != ((struct X3D_PolyRep *)node->_intern)->irep_change) { 
		
		copyPointersToVRML1IndexedFaceSet(node);
		compileNode ((void *)compile_polyrep, node, 
			node->_coord, node->_color, node->_normal, node->_texCoord);
	} 
	/* something went wrong... */
	if (!node->_intern) return;

	CULL_FACE(node->_solid)
	render_polyrep(node);
}


/********* IndexedLineSet ******************************************************************/

/* X3D uses emissiveColor for its line, so we will take the emissiveColor from the Material node
and use it here. This might be wrong for VRML1... */

static void copyPointersToVRML1IndexedLineSet(struct X3D_VRML1_IndexedLineSet *node) {
	/* yes, we do have to remake this node */

	struct X3D_IndexedLineSet *ILS = createNewX3DNode(NODE_IndexedLineSet);

	node->_ILS = ILS; 

	if (cSLD->mbNode!=NULL) ILS->colorPerVertex = cSLD->mbNode->_Value==VRML1MOD_PER_VERTEX;
	else ILS->colorPerVertex = FALSE;

	/* do we have a scoped Material node, with more than 1 emissiveColor? */
	if (cSLD->matNode!= NULL) {
		/* if we have more than 1 emissiveColor, we should treat them as a color node */
		if (cSLD->matNode->emissiveColor.n > 1) {
			struct X3D_Color *nc;
			if (ILS->color==NULL) { 
				ILS->color = createNewX3DNode(NODE_Color);
				ADD_PARENT(ILS->color,X3D_NODE(node));
			}
			nc = ILS->color;
			FREE_IF_NZ(nc->color.p);
			nc->color.p = MALLOC(sizeof (struct SFColor) * cSLD->matNode->emissiveColor.n);
			memcpy(nc->color.p, cSLD->matNode->emissiveColor.p, sizeof (struct SFColor) * cSLD->matNode->emissiveColor.n);
			nc->color.n = cSLD->matNode->emissiveColor.n;
		}
	}
		
	/* do we have a scoped Coordinate3 node? */
	if (cSLD->c3Node!= NULL) {
		struct X3D_Coordinate *nc;
		if (ILS->coord==NULL) {
			ILS->coord = createNewX3DNode(NODE_Coordinate);
			ADD_PARENT(ILS->coord,X3D_NODE(node));
		}
		nc = ILS->coord;
		FREE_IF_NZ(nc->point.p);
		nc->point.p = MALLOC(sizeof (struct SFColor) * cSLD->c3Node->point.n);
		memcpy(nc->point.p, cSLD->c3Node->point.p, sizeof (struct SFColor) * cSLD->c3Node->point.n);

		nc->point.n = cSLD->c3Node->point.n;
	}

	/* lets copy over the coordIndex and colorIndex fields */
	if (node->coordIndex.n>0) {
		ILS->coordIndex.p = MALLOC(node->coordIndex.n*sizeof(int));
		memcpy (ILS->coordIndex.p,node->coordIndex.p,node->coordIndex.n*sizeof(int));
		ILS->coordIndex.n = node->coordIndex.n;
	}
	if (node->materialIndex.n>0) {
		ILS->colorIndex.p = MALLOC(node->materialIndex.n*sizeof(int));
		memcpy (ILS->colorIndex.p,node->materialIndex.p,node->materialIndex.n*sizeof(int));
		ILS->colorIndex.n = node->materialIndex.n;
	}
}

void compile_IndexedLineSet(struct X3D_IndexedLineSet *);
void render_IndexedLineSet(struct X3D_IndexedLineSet *);
void render_VRML1_IndexedLineSet (struct X3D_VRML1_IndexedLineSet *node) {
	/* call an X3D IndexedLineSet here */
	if (node->_ILS == NULL) {
		copyPointersToVRML1IndexedLineSet(node);
		compile_IndexedLineSet((struct X3D_IndexedLineSet *)(node->_ILS));

	}
	LIGHTING_ON /* use emissiveColors, so turn lighting on */
	render_IndexedLineSet((struct X3D_IndexedLineSet *)(node->_ILS));
}



void render_VRML1_AsciiText (struct X3D_VRML1_AsciiText *this) {}
void render_VRML1_MatrixTransform (struct X3D_VRML1_MatrixTransform  *node) {}
void render_VRML1_Switch (struct X3D_VRML1_Switch  *node) {}
void render_VRML1_WWWAnchor (struct X3D_VRML1_WWWAnchor  *node) {}
void render_VRML1_LOD (struct X3D_VRML1_LOD  *node) {}
void render_VRML1_OrthographicCamera (struct X3D_VRML1_OrthographicCamera  *node) {}
void render_VRML1_PerspectiveCamera (struct X3D_VRML1_PerspectiveCamera  *node) {}
void render_VRML1_WWWInline (struct X3D_VRML1_WWWInline  *node) {}
