/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Shape.c,v 1.110 2012/07/11 19:10:54 crc_canada Exp $

X3D Shape Component

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
#include "../opengl/Frustum.h"
#include "../opengl/Material.h"
#include "../opengl/Textures.h"
#include "../opengl/OpenGL_Utils.h"
#include "Component_ProgrammableShaders.h"
#include "Component_Shape.h"
#include "RenderFuncs.h"


typedef struct pComponent_Shape{

	int     linePropertySet;  /* line properties -width, etc                  */

	struct matpropstruct appearanceProperties;

	/* this is for the FillProperties node */
	GLuint fillpropCurrentShader;// = 0;

	/* pointer for a TextureTransform type of node */
	struct X3D_Node *  this_textureTransform;  /* do we have some kind of textureTransform? */

	/* for doing shader material properties */
	struct X3D_TwoSidedMaterial *material_twoSided;
	struct X3D_Material *material_oneSided;
	int fpshaderloaded;// = FALSE; 
	GLint hatchColour;
	GLint hatchPercent;
	GLint filledBool;
	GLint hatchedBool;
	GLint algorithm;
	GLint norm;
	GLint vert;
	GLint modView;
	GLint projMat;
	GLint normMat;

}* ppComponent_Shape;
void *Component_Shape_constructor(){
	void *v = malloc(sizeof(struct pComponent_Shape));
	memset(v,0,sizeof(struct pComponent_Shape));
	return v;
}
void Component_Shape_init(struct tComponent_Shape *t){
	//public
	//private
	t->prv = Component_Shape_constructor();
	{
		ppComponent_Shape p = (ppComponent_Shape)t->prv;
		//p->linePropertySet;  /* line properties -width, etc                  */

		//p->appearanceProperties;

		/* this is for the FillProperties node */
		p->fillpropCurrentShader = 0;

		/* pointer for a TextureTransform type of node */
		//p->this_textureTransform;  /* do we have some kind of textureTransform? */

		/* for doing shader material properties */
		//p->material_twoSided;
		//p->material_oneSided;
		p->fpshaderloaded = FALSE; 
		p->hatchColour = -1;
		p->hatchPercent = -1;
		p->filledBool= -1;
		p->hatchedBool = -1;
		p->algorithm = -1;
		p->norm = -1;
		p->vert = -1;
		p->modView = -1;
		p->projMat = -1;
		p->normMat = -1;
	}
}
//ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

//getters
struct matpropstruct *getAppearanceProperties(){
	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

	return &p->appearanceProperties;
}


struct X3D_Node *getThis_textureTransform(){
    ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
	return p->this_textureTransform;
}

void child_Appearance (struct X3D_Appearance *node) {
	struct X3D_Node *tmpN;
	
	/* initialization */
	gglobal()->RenderFuncs.last_texture_type = NOTEXTURE;
	
	/* printf ("in Appearance, this %d, nodeType %d\n",node, node->_nodeType);
	   printf (" vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	   render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */
	
	/* Render the material node... */
	RENDER_MATERIAL_SUBNODES(node->material);
	
	if (node->fillProperties) {
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->fillProperties,tmpN);
		render_node(tmpN);
	}
	
	/* set line widths - if we have line a lineProperties node */
	if (node->lineProperties) {
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->lineProperties,tmpN);
		render_node(tmpN);
	}
	
	if(node->texture) {
		/* we have to do a glPush, then restore, later */
		/* glPushAttrib(GL_ENABLE_BIT); */
		ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

		/* is there a TextureTransform? if no texture, fugutaboutit */
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->textureTransform,p->this_textureTransform);
		
		/* now, render the texture */
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->texture,tmpN);

		render_node(tmpN);
	}

	/* shaders here/supported?? */
	if (node->shaders.n !=0) {
		int count;
		int foundGoodShader = FALSE;
		
		for (count=0; count<node->shaders.n; count++) {
			POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->shaders.p[count], tmpN);
			
			/* have we found a valid shader yet? */
			if (foundGoodShader) {
				/* printf ("skipping shader %d of %d\n",count, node->shaders.n); */
				/* yes, just tell other shaders that they are not selected */
				SET_SHADER_SELECTED_FALSE(tmpN);
			} else {
				/* render this node; if it is valid, then we call this one the selected one */
				SET_FOUND_GOOD_SHADER(tmpN);
				DEBUG_SHADER("running shader (%s) %d of %d\n",
			    	 stringNodeType(X3D_NODE(tmpN)->_nodeType),count, node->shaders.n);
				render_node(tmpN);
			}
		}
	}
}


void render_Material (struct X3D_Material *node) {
	COMPILE_IF_REQUIRED
	{
	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

	/* record this node for OpenGL-ES and OpenGL-3.1 operation */
	p->material_oneSided = node;
	}
}

#ifdef OLDCODE
OLDCODE #ifdef SHADERS_2011
#endif //OLDCODE

/* bounds check the material node fields */
void compile_Material (struct X3D_Material *node) {
	int i;
	float trans;


	/* verify that the numbers are within range */
	if (node->ambientIntensity < 0.0f) node->ambientIntensity=0.0f;
	if (node->ambientIntensity > 1.0f) node->ambientIntensity=1.0f;
	if (node->shininess < 0.0f) node->shininess=0.0f;
	if (node->shininess > 1.0f) node->shininess=1.0f;
	if (node->transparency < 0.0f) node->transparency=MIN_NODE_TRANSPARENCY;
	if (node->transparency >= 1.0f) node->transparency=MAX_NODE_TRANSPARENCY;

	for (i=0; i<3; i++) {
		if (node->diffuseColor.c[i] < 0.0f) node->diffuseColor.c[i]=0.0f;
		if (node->diffuseColor.c[i] > 1.0f) node->diffuseColor.c[i]=1.0f;
		if (node->emissiveColor.c[i] < 0.0f) node->emissiveColor.c[i]=0.0f;
		if (node->emissiveColor.c[i] > 1.0f) node->emissiveColor.c[i]=1.0f;
		if (node->specularColor.c[i] < 0.0f) node->specularColor.c[i]=0.0f;
		if (node->specularColor.c[i] > 1.0f) node->specularColor.c[i]=1.0f;
	}

        /* set the transparency here for the material */
	/* Remember, VRML/X3D transparency 0.0 = solid; OpenGL 1.0 = solid, so we reverse it... */
        trans = 1.0f - node->transparency;
                
	/* we now keep verified params in a structure that maps to Shaders well...
		struct gl_MaterialParameters {
        	        vec4 emission;
        	        vec4 ambient;
        	        vec4 diffuse;
        	        vec4 specular;
        	        float shininess;
        	};
	  which is stored in the _verifiedColor[17] array here.
	  emission [0]..[3];
	  ambient [4]..[7];
	  diffuse [8]..[11];
	  specular [12]..[15];
	  shininess [16]
	  
*/
	/* first, put in the transparency */
        node->_verifiedColor.p[3] = trans;
        node->_verifiedColor.p[7] = trans;
        node->_verifiedColor.p[11] = trans;
        node->_verifiedColor.p[15] = trans;
                
	/* DiffuseColor */
	memcpy((void *)(&node->_verifiedColor.p[8]), node->diffuseColor.c, sizeof (float) * 3);

	/* Ambient  - diffuseColor * ambientIntensity */
        for(i=0; i<3; i++) { node->_verifiedColor.p[i+4] = node->_verifiedColor.p[i+8] * node->ambientIntensity; }

	/* Specular */
	memcpy((void *)(&node->_verifiedColor.p[12]), node->specularColor.c, sizeof (float) * 3);

	/* Emissive */
	memcpy((void *)(&node->_verifiedColor.p[0]), node->emissiveColor.c, sizeof (float) * 3);

	/* Shininess */
        node->_verifiedColor.p[16] = node->shininess * 128.0f;

#define MAX_SHIN 128.0f
#define MIN_SHIN 0.01f
        if ((node->_verifiedColor.p[16] > MAX_SHIN) || (node->_verifiedColor.p[16] < MIN_SHIN)) {
                if (node->_verifiedColor.p[16]>MAX_SHIN){node->_verifiedColor.p[16] = MAX_SHIN;}else{node->_verifiedColor.p[16]=MIN_SHIN;}
        }
#undef MAX_SHIN
#undef MIN_SHIN


	MARK_NODE_COMPILED
}

/* Least significant hex digit - appearance */
#define NO_APPEARANCE_SHADER 0x0000
#define MATERIAL_APPEARANCE_SHADER 0x0001
#define TWO_MATERIAL_APPEARANCE_SHADER 0x0002
#define ONE_TEX_APPEARANCE_SHADER 0x0004
#define MULTI_TEX_APPEARANCE_SHADER 0x0008

/* second least significant hex digit - PolyRep colour present */
#define NO_COLOUR_SHADER 0x0000
#define HAVE_COLOUR_SHADER 0x00010

/* fourth least significant hex digit - lines, points */
#define NO_LINES_POINTS 0x0000
#define HAVE_LINEPOINTS 0x1000


#define CHECK_COLOUR_FIELD(aaa) \
	case NODE_##aaa: { \
		struct X3D_##aaa *me = (struct X3D_##aaa *)realNode; \
		if (me->color == NULL) return NO_COLOUR_SHADER; \
		else return HAVE_COLOUR_SHADER; \
		break; \
	} 

#define CHECK_VRML1_COLOUR_FIELD(aaa) \
	case NODE_##aaa: { \
		struct X3D_##aaa *me = (struct X3D_##aaa *)realNode; \
		if (me->_color == NULL) return NO_COLOUR_SHADER; \
		else return HAVE_COLOUR_SHADER; \
		break; \
	} \

/* if this is a LineSet, PointSet, etc... */
static int getIfLinePoints(struct X3D_Node *myGeom) {
	struct X3D_Node *realNode;

	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,myGeom,realNode);

	if (realNode == NULL) return NO_APPEARANCE_SHADER;
	switch (realNode->_nodeType) {
		case NODE_IndexedLineSet:
		case NODE_LineSet:
		case NODE_VRML1_IndexedLineSet:
		case NODE_PointSet:
		case NODE_VRML1_PointSet:
			return  HAVE_LINEPOINTS;

	}
	return NO_LINES_POINTS;
}



/* Some shapes have Color nodes - if so, then we have other shaders */
static int getShapeColourShader (struct X3D_Node *myGeom) {
	struct X3D_Node *realNode;

	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,myGeom,realNode);

	if (realNode == NULL) return NO_APPEARANCE_SHADER;

	/* go through each node type that can have a Color node, and if it is not NULL
	   we know we have a Color node */

	switch (realNode->_nodeType) {
		CHECK_COLOUR_FIELD(IndexedFaceSet);
		CHECK_COLOUR_FIELD(IndexedLineSet);
		CHECK_COLOUR_FIELD(IndexedTriangleFanSet);
		CHECK_COLOUR_FIELD(IndexedTriangleSet);
		CHECK_COLOUR_FIELD(IndexedTriangleStripSet);
		CHECK_COLOUR_FIELD(LineSet);
		CHECK_COLOUR_FIELD(PointSet);
		CHECK_COLOUR_FIELD(TriangleFanSet);
		CHECK_COLOUR_FIELD(TriangleStripSet);
		CHECK_COLOUR_FIELD(TriangleSet);
		CHECK_COLOUR_FIELD(ElevationGrid);
		CHECK_COLOUR_FIELD(GeoElevationGrid);
		CHECK_VRML1_COLOUR_FIELD(VRML1_IndexedFaceSet);
	}

	/* if we are down here, we KNOW we do not have a color field */
	return NO_COLOUR_SHADER;
}

static int getAppearanceShader (struct X3D_Node *myApp) {
	struct X3D_Appearance *realAppearanceNode;
	struct X3D_Node *realMaterialNode;


	int retval = NO_APPEARANCE_SHADER;

	/* if there is no appearance node... */
	if (myApp == NULL) return retval;

	realAppearanceNode = ((struct X3D_Appearance *)myApp);
	if (realAppearanceNode->_nodeType != NODE_Appearance) return retval;

	if (realAppearanceNode->material != NULL) {
		realMaterialNode = ((struct X3D_Node *)realAppearanceNode->material);
		if (realMaterialNode->_nodeType == NODE_Material) {
			retval |= MATERIAL_APPEARANCE_SHADER;
		}
		if (realMaterialNode->_nodeType == NODE_TwoSidedMaterial) {
			retval |= TWO_MATERIAL_APPEARANCE_SHADER;
		}
	}
    
    

	if (realAppearanceNode->texture != NULL) {
        //printf ("getAppearanceShader - rap node is %s\n",stringNodeType(realAppearanceNode->texture->_nodeType));
        if ((realAppearanceNode->texture->_nodeType == NODE_ImageTexture) ||
            (realAppearanceNode->texture->_nodeType == NODE_PixelTexture)){
			retval |= ONE_TEX_APPEARANCE_SHADER;
		} else if (realAppearanceNode->texture->_nodeType == NODE_MultiTexture) {
            retval |= MULTI_TEX_APPEARANCE_SHADER;
        } else {
			printf ("getAppearanceShader, texture field %s not supported yet\n",
			stringNodeType(realAppearanceNode->texture->_nodeType));
		}
	}
    //printf ("getAppearanceShader, returning %x\n",retval);

	return retval;
}


#ifdef SHAPE_VERBOSE
static void printChoosingShader(shader_type_t whichOne) {
    
    ConsoleMessage ("choose shader: ");
    switch (whichOne) {
        case backgroundSphereShader: ConsoleMessage ("backgroundSphereShader\n"); break;
        case backgroundTextureBoxShader: ConsoleMessage ("backgroundTextureBoxShader\n"); break;
        case noTexOneMaterialShader: ConsoleMessage ("noTexOneMaterialShader\n"); break;
        case noMaterialNoAppearanceShader: ConsoleMessage ("noMaterialNoAppearanceShader\n"); break;
        case noTexTwoMaterialShader: ConsoleMessage ("noTexTwoMaterialShader\n"); break;
        case oneTexOneMaterialShader: ConsoleMessage ("oneTexOneMaterialShader\n"); break;
        case oneTexTwoMaterialShader: ConsoleMessage ("oneTexTwoMaterialShader\n"); break;
        case complexTexOneMaterialShader: ConsoleMessage ("complexTexOneMaterialShader\n"); break;
        case complexTexTwoMaterialShader: ConsoleMessage ("complexTexTwoMaterialShader\n"); break;
        case noTexTwoMaterialColourShader: ConsoleMessage ("noTexTwoMaterialColourShader\n"); break;
        case noTexOneMaterialColourShader: ConsoleMessage ("noTexOneMaterialColourShader\n"); break;
        case oneTexTwoMaterialColourShader: ConsoleMessage ("oneTexTwoMaterialColourShader\n"); break;
        case oneTexOneMaterialColourShader: ConsoleMessage ("oneTexOneMaterialColourShader\n"); break;
        case multiTexShader: ConsoleMessage ("multiTexShader\n"); break;
        case linePointColorNodeShader: ConsoleMessage ("linePointColorNodeShader\n"); break;
        case linePointNoColorNodeShader: ConsoleMessage ("linePointNoColorNodeShader\n"); break;
        default : ConsoleMessage ("shader node unidentified");
    }
    
}
#endif //SHAPE_VERBOSE

/* find info on the appearance of this Shape and create a shader */
/* 
	The possible sequence of a properly constructed appearance field is:

	Shape.appearance -> Appearance
	
	Appearance.fillProperties 	-> FillProperties
	Appearance.lineProperties 	-> LineProperties
	Appearance.material 		-> Material
					-> TwoSidedMaterial	
	Appearance.shaders		-> ComposedShader
	Appearance.shaders		-> PackagedShader
	Appearance.shaders		-> ProgramShader

	Appearance.texture		-> Texture
	Appearance.texture		-> MultiTexture
	Appearance.textureTransform	->

*/

void render_FillProperties (struct X3D_FillProperties *node) {
ConsoleMessage("should not be calling render_FillProperties");
}

void render_LineProperties (struct X3D_LineProperties *node) {
	ConsoleMessage ("should not be calling render_LineProperties");
}


void child_Shape (struct X3D_Shape *node) {
	struct X3D_Node *tmpN;
	// JAS int i;
	// JAS float dcol[4];
	// JAS float ecol[4];
	// JAS float scol[4];
	// JAS float amb;
    
	ppComponent_Shape p;
        ttglobal tg = gglobal();



	COMPILE_IF_REQUIRED

	/* JAS - if not collision, and render_geom is not set, no need to go further */
	/* printf ("child_Shape vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	 render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	if(!(node->geometry)) { return; }

	RECORD_DISTANCE

	if((renderstate()->render_collision) || (renderstate()->render_sensitive)) {
		/* only need to forward the call to the child */
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,node->geometry,tmpN);
		render_node(tmpN);
		return;
	}

	if (node->_shaderTableEntry == -1) {
		return;
	}
        p = (ppComponent_Shape)tg->Component_Shape.prv;
	{
		struct fw_MaterialParameters defaultMaterials = {
					{0.0f, 0.0f, 0.0f, 1.0f}, /* emissiveColor */
					{0.0f, 0.0f, 0.0f, 1.0f}, /* ambientIntensity */
					{0.8f, 0.8f, 0.8f, 1.0f}, /* diffuseColor */
					{0.0f, 0.0f, 0.0f, 1.0f}, /* specularColor */
					10.0f};                   /* shininess */

		/* copy the material stuff in preparation for copying all to the shader */
		memcpy (&p->appearanceProperties.fw_FrontMaterial, &defaultMaterials, sizeof (struct fw_MaterialParameters));
		memcpy (&p->appearanceProperties.fw_BackMaterial, &defaultMaterials, sizeof (struct fw_MaterialParameters));

	}

	// now done in textureDraw_end  tg->RenderFuncs.textureStackTop = 0; /* will be >=1 if textures found */

	/* enable the shader for this shape */
        enableGlobalShader (node->_shaderTableEntry);

	/* now, are we rendering blended nodes or normal nodes?*/
	if (renderstate()->render_blend == (node->_renderFlags & VF_Blend)) {

		RENDER_MATERIAL_SUBNODES(node->appearance);

		if (p->material_oneSided != NULL) {

			memcpy (&p->appearanceProperties.fw_FrontMaterial, p->material_oneSided->_verifiedColor.p, sizeof (struct fw_MaterialParameters));
			memcpy (&p->appearanceProperties.fw_BackMaterial, p->material_oneSided->_verifiedColor.p, sizeof (struct fw_MaterialParameters));
			/* copy the emissive colour over for lines and points */
			memcpy(p->appearanceProperties.emissionColour,&(p->material_oneSided->_verifiedColor.p[0]), 3*sizeof(float));

		} else if (p->material_twoSided != NULL) {
			memcpy (&p->appearanceProperties.fw_FrontMaterial, p->material_twoSided->_verifiedFrontColor.p, sizeof (struct fw_MaterialParameters));
			memcpy (&p->appearanceProperties.fw_BackMaterial, p->material_twoSided->_verifiedBackColor.p, sizeof (struct fw_MaterialParameters));
			/* copy the emissive colour over for lines and points */
			memcpy(p->appearanceProperties.emissionColour,&(p->material_twoSided->_verifiedFrontColor.p[0]), 3*sizeof(float));
		} else {
			/* no materials selected.... */
		}

		/* send along lighting, material, other visible properties */
		sendMaterialsToShader(p->appearanceProperties.currentShaderProperties);

		#ifdef SHAPEOCCLUSION
		beginOcclusionQuery((struct X3D_VisibilitySensor*)node,renderstate()->render_geom); //BEGINOCCLUSIONQUERY;
		#endif

		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->geometry,tmpN);
		render_node(tmpN);

		#ifdef SHAPEOCCLUSION
		endOcclusionQuery((struct X3D_VisibilitySensor*)node,renderstate()->render_geom); //ENDOCCLUSIONQUERY;
		#endif
	}

	/* any shader turned on? if so, turn it off */
	TURN_GLOBAL_SHADER_OFF;
	p->material_twoSided = NULL;
	p->material_oneSided = NULL;

    
	/* did the lack of an Appearance or Material node turn lighting off? */
	LIGHTING_ON;

	/* turn off face culling */
	DISABLE_CULL_FACE;

}

void compile_Shape (struct X3D_Shape *node) {
	int whichAppearanceShader = -1;
	int whichShapeColorShader = -1;
	int isUnlitGeometry = -1;
    
	whichShapeColorShader = getShapeColourShader(node->geometry);
	whichAppearanceShader = getAppearanceShader(node->appearance);
	isUnlitGeometry = getIfLinePoints(node->geometry);



	/* choose the shader. Note that we just "or" the results together */

    /* right now, multiTextures go through one heck of a big unoptimized shader */
    if (whichAppearanceShader == (MULTI_TEX_APPEARANCE_SHADER | MATERIAL_APPEARANCE_SHADER)) {
        node->_shaderTableEntry = multiTexShader;
        
    } else {
	switch (whichAppearanceShader | whichShapeColorShader | isUnlitGeometry) {
            
	case  NO_APPEARANCE_SHADER| NO_COLOUR_SHADER:
		node->_shaderTableEntry = noMaterialNoAppearanceShader;
		break;

	case  TWO_MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER:
		node->_shaderTableEntry = noTexTwoMaterialShader;
		break;

	case MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER:
		node->_shaderTableEntry = noTexOneMaterialShader;
		break;	

	case ONE_TEX_APPEARANCE_SHADER | TWO_MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER:
		node->_shaderTableEntry = oneTexTwoMaterialShader;
		break;

	case ONE_TEX_APPEARANCE_SHADER| NO_COLOUR_SHADER:
	case ONE_TEX_APPEARANCE_SHADER | MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER:
		node->_shaderTableEntry = oneTexOneMaterialShader;
		break;	


	

	/* SECTION 2 -  HAVE Color node in shape */

	/* if we have a LineSet, PointSet, etc, and there is a Color node in it, choose this one! */	
	case NO_APPEARANCE_SHADER| HAVE_COLOUR_SHADER | HAVE_LINEPOINTS:
	case MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER | HAVE_LINEPOINTS:
	case TWO_MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER | HAVE_LINEPOINTS:
	case ONE_TEX_APPEARANCE_SHADER | TWO_MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER | HAVE_LINEPOINTS:
	case ONE_TEX_APPEARANCE_SHADER| HAVE_COLOUR_SHADER | HAVE_LINEPOINTS:
	case ONE_TEX_APPEARANCE_SHADER | MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER | HAVE_LINEPOINTS:
		node->_shaderTableEntry = linePointColorNodeShader;
		break;

	
	/* if we have a LineSet, PointSet, etc, and there is NOT a Color node in it, choose this one! */	
	case NO_APPEARANCE_SHADER| NO_COLOUR_SHADER | HAVE_LINEPOINTS:
	case MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER | HAVE_LINEPOINTS:
	case TWO_MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER | HAVE_LINEPOINTS:
	case ONE_TEX_APPEARANCE_SHADER | TWO_MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER | HAVE_LINEPOINTS:
	case ONE_TEX_APPEARANCE_SHADER| NO_COLOUR_SHADER | HAVE_LINEPOINTS:
	case ONE_TEX_APPEARANCE_SHADER | MATERIAL_APPEARANCE_SHADER| NO_COLOUR_SHADER | HAVE_LINEPOINTS:
		node->_shaderTableEntry = linePointNoColorNodeShader;
		break;

	case NO_APPEARANCE_SHADER| HAVE_COLOUR_SHADER:
		node->_shaderTableEntry = backgroundSphereShader;
		break;

	case TWO_MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER:
		node->_shaderTableEntry = noTexTwoMaterialColourShader;
		break;

	case MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER:
		node->_shaderTableEntry = noTexOneMaterialColourShader;
		break;	

	case ONE_TEX_APPEARANCE_SHADER | TWO_MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER:
		node->_shaderTableEntry = oneTexTwoMaterialColourShader;
		break;

	case ONE_TEX_APPEARANCE_SHADER| HAVE_COLOUR_SHADER:
	case ONE_TEX_APPEARANCE_SHADER | MATERIAL_APPEARANCE_SHADER| HAVE_COLOUR_SHADER:
		node->_shaderTableEntry = oneTexOneMaterialColourShader;
		break;	
	

	default: node->_shaderTableEntry = noMaterialNoAppearanceShader;
    }
    }

    
	#ifdef SHAPE_VERBOSE
	printChoosingShader(node->_shaderTableEntry);
	#endif //SHAPE_VERBOSE

	MARK_NODE_COMPILED
}

void compile_TwoSidedMaterial (struct X3D_Appearance *node) {
ConsoleMessage ("compile_TwoSidedMaterial called\n");
}

void render_TwoSidedMaterial (struct X3D_Appearance *node) {
ConsoleMessage ("render_TwoSidedMaterial called\n");
}

#ifdef OLDCODE
OLDCODE#else //SHADERS_2011
OLDCODE
OLDCODEstatic void shaderErrorLog(GLuint myShader, char *which) {
OLDCODE        #if defined  (GL_VERSION_2_0) || defined (GL_ES_VERSION_2_0)
OLDCODE#define MAX_INFO_LOG_SIZE 512
OLDCODE                GLchar infoLog[MAX_INFO_LOG_SIZE];
OLDCODE                glGetShaderInfoLog(myShader, MAX_INFO_LOG_SIZE, NULL, infoLog);
OLDCODE                ConsoleMessage ("problem with %s shader: %s",which, infoLog);
OLDCODE        #else
OLDCODE                ConsoleMessage ("Problem compiling shader");
OLDCODE        #endif
OLDCODE}
OLDCODE
OLDCODE
OLDCODE/* now works with our pushing matricies (norm, proj, modelview) but not for complete shader appearance replacement */
OLDCODEvoid render_FillProperties (struct X3D_FillProperties *node) {
OLDCODE	GLfloat hatchX;
OLDCODE	GLfloat hatchY;
OLDCODE	GLint algor;
OLDCODE	GLint hatched;
OLDCODE	GLint filled;
OLDCODE	int success;
OLDCODE
OLDCODE	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
OLDCODE
OLDCODE	if (!p->fpshaderloaded) {
OLDCODE		const char *vs = "\
OLDCODE			/* \n\
OLDCODE			  Shader source from \n\
OLDCODE			  \"Introduction to the OpenGL Shading Language\" \n\
OLDCODE			  presentation by Randi Rost, 3DLabs (GLSLOverview2005.pdf) \n\
OLDCODE			*/ \n\
OLDCODE			 \n\
OLDCODEuniform                mat4 fw_ModelViewMatrix; \n\
OLDCODEuniform                mat4 fw_ProjectionMatrix; \n\
OLDCODEuniform mat3	fw_NormalMatrix; \n\
OLDCODE			uniform vec3 LightPosition; \n\
OLDCODE			uniform bool filled; \n\
OLDCODE			const float SpecularContribution = 0.3; \n\
OLDCODE			const float DiffuseContribution = 1.0 - SpecularContribution; \n\
OLDCODE			varying float LightIntensity; \n\
OLDCODE			varying vec2 MCposition; \n\
OLDCODE			void main(void) \n\
OLDCODE			{ \n\
OLDCODE               gl_Position = fw_ProjectionMatrix * fw_ModelViewMatrix * gl_Vertex; \n\
OLDCODE			    vec3 ecPosition = vec3(fw_ModelViewMatrix * gl_Vertex); \n\
OLDCODE			    vec3 tnorm      = normalize(fw_NormalMatrix * gl_Normal); \n\
OLDCODE			    vec3 lightVec   = normalize(LightPosition - ecPosition); \n\
OLDCODE			    vec3 reflectVec = reflect(-lightVec, tnorm); \n\
OLDCODE			    vec3 viewVec    = normalize(-ecPosition); \n\
OLDCODE			    float diffuse   = max(dot(lightVec, tnorm), 0.0); \n\
OLDCODE			    float spec      = 0.0; \n\
OLDCODE			    if (diffuse > 0.0) \n\
OLDCODE			    { \n\
OLDCODE			        spec = max(dot(reflectVec, viewVec), 0.0); \n\
OLDCODE			        spec = pow(spec, 16.0); \n\
OLDCODE			    } \n\
OLDCODE			    LightIntensity = DiffuseContribution * diffuse + \n\
OLDCODE			                       SpecularContribution * spec; \n\
OLDCODE			    MCposition      = gl_Vertex.xy; \n\
OLDCODE			    /* old - JAS gl_Position     = ftransform(); */ \n\
OLDCODE			    // Get the vertex colour\n\
OLDCODE			    if (filled) gl_FrontColor = gl_FrontMaterial.diffuse;\n\
OLDCODE			    else gl_FrontColor = vec4(0.0, 0.0, 0.0, 0.0); // make transparent \n\
OLDCODE			} \n\
OLDCODE		";
OLDCODE
OLDCODE		const char *fs = "\
OLDCODE			/*  \n\
OLDCODE			  Shader source from  \n\
OLDCODE			  \"Introduction to the OpenGL Shading Language\"  \n\
OLDCODE			  presentation by Randi Rost, 3DLabs (GLSLOverview2005.pdf)  \n\
OLDCODE			*/  \n\
OLDCODE			  \n\
OLDCODE			// HatchSize - x and y - larger = less hatches on shape \n\
OLDCODE			const vec2  HatchSize= vec2(0.15, 0.15);  \n\
OLDCODE			  \n\
OLDCODE			uniform vec4 HatchColour;     //= (0.85,0.86,0.84);  \n\
OLDCODE			uniform bool hatched;\n\
OLDCODE			uniform bool filled; \n\
OLDCODE			uniform vec2  HatchPct;               //= (0.90, 0.85);  \n\
OLDCODE			uniform int algorithm;	\n\
OLDCODE			varying vec2  MCposition;  \n\
OLDCODE			varying float LightIntensity;  \n\
OLDCODE			  \n\
OLDCODE			void main(void)  \n\
OLDCODE			{  \n\
OLDCODE			    vec4 color;  \n\
OLDCODE			    vec2 position, useBrick;  \n\
OLDCODE			    vec4 fragCol = gl_Color; \n\
OLDCODE			  \n\
OLDCODE			    position = MCposition / HatchSize;  \n\
OLDCODE			  \n\
OLDCODE			    if (algorithm == 0) {// bricking \n\
OLDCODE			    	if (fract(position.y * 0.5) > 0.5)  \n\
OLDCODE			            position.x += 0.5;  \n\
OLDCODE			   } \n\
OLDCODE			  \n\
OLDCODE			    // algorithm 1, 2 = no futzing required here \n\
OLDCODE			    if (algorithm == 3) {// positive diagonals \n\
OLDCODE				    vec2 curpos = position; \n\
OLDCODE			            position.x -= curpos.y;  \n\
OLDCODE			   } \n\
OLDCODE			  \n\
OLDCODE			    if (algorithm == 4) {// negative diagonals \n\
OLDCODE				    vec2 curpos = position; \n\
OLDCODE			            position.x += curpos.y;  \n\
OLDCODE			   } \n\
OLDCODE			  \n\
OLDCODE			    if (algorithm == 6) {// diagonal crosshatch \n\
OLDCODE				vec2 curpos = position; \n\
OLDCODE				if (fract(position.y) > 0.5)  { \n\
OLDCODE        			    if (fract(position.x) < 0.5) position.x += curpos.y; \n\
OLDCODE        			    else position.x -= curpos.y; \n\
OLDCODE				} else { \n\
OLDCODE        			    if (fract(position.x) > 0.5) position.x += curpos.y; \n\
OLDCODE        			    else position.x -= curpos.y; \n\
OLDCODE				} \n\
OLDCODE			   } \n\
OLDCODE			  \n\
OLDCODE			    position = fract(position);  \n\
OLDCODE			  \n\
OLDCODE			    useBrick = step(position, HatchPct);  \n\
OLDCODE			  \n\
OLDCODE			    if (hatched) color = mix(HatchColour, fragCol, useBrick.x * useBrick.y);  \n\
OLDCODE			    else color = fragCol; \n\
OLDCODE			  \n\
OLDCODE			    color *= LightIntensity;  \n\
OLDCODE			    gl_FragColor = color;  \n\
OLDCODE				if (filled) gl_FragColor.a = 1.0; //JAS \n\
OLDCODE			}  \n\
OLDCODE		";
OLDCODE
OLDCODE		GLuint v;
OLDCODE		GLuint f;
OLDCODE	
OLDCODE		#ifdef FILLVERBOSE
OLDCODE			printf ("creating shaders\n");
OLDCODE		#endif
OLDCODE
OLDCODE
OLDCODE		v = CREATE_SHADER(GL_VERTEX_SHADER);
OLDCODE		f = CREATE_SHADER(GL_FRAGMENT_SHADER);	
OLDCODE	
OLDCODE		#ifdef FILLVERBOSE
OLDCODE			printf ("assigning shader source\n");
OLDCODE		#endif
OLDCODE
OLDCODE
OLDCODE		SHADER_SOURCE(v, 1, &vs,NULL);
OLDCODE		SHADER_SOURCE(f, 1, &fs,NULL);
OLDCODE	
OLDCODE		#ifdef FILLVERBOSE
OLDCODE			printf ("compiling shaders\n");
OLDCODE		#endif
OLDCODE
OLDCODE
OLDCODE		COMPILE_SHADER(v);
OLDCODE	
OLDCODE                COMPILE_SHADER(v);
OLDCODE                GET_SHADER_INFO(v, COMPILE_STATUS, &success);
OLDCODE                if (!success) {
OLDCODE                        shaderErrorLog(v,"GEOMETRY");
OLDCODE                }
OLDCODE
OLDCODE		COMPILE_SHADER(f);
OLDCODE                GET_SHADER_INFO(f, COMPILE_STATUS, &success);
OLDCODE                if (!success) {
OLDCODE                        shaderErrorLog(f,"GEOMETRY");
OLDCODE                }
OLDCODE
OLDCODE		#ifdef FILLVERBOSE
OLDCODE			printf ("creating program and attaching\n");
OLDCODE		#endif
OLDCODE
OLDCODE		p->fillpropCurrentShader = CREATE_PROGRAM;
OLDCODE		
OLDCODE		ATTACH_SHADER(p->fillpropCurrentShader,v);
OLDCODE		ATTACH_SHADER(p->fillpropCurrentShader,f);
OLDCODE	
OLDCODE		#ifdef FILLVERBOSE
OLDCODE			printf ("linking program\n");
OLDCODE		#endif
OLDCODE
OLDCODE
OLDCODE		LINK_SHADER(p->fillpropCurrentShader);
OLDCODE
OLDCODE		#ifdef FILLVERBOSE
OLDCODE			printf ("getting shader vars\n");
OLDCODE		#endif
OLDCODE
OLDCODE		p->hatchColour = GET_UNIFORM(p->fillpropCurrentShader,"HatchColour");
OLDCODE		p->hatchPercent = GET_UNIFORM(p->fillpropCurrentShader,"HatchPct");
OLDCODE		p->filledBool = GET_UNIFORM(p->fillpropCurrentShader,"filled");
OLDCODE		p->hatchedBool = GET_UNIFORM(p->fillpropCurrentShader,"hatched");
OLDCODE		p->algorithm = GET_UNIFORM(p->fillpropCurrentShader,"algorithm");
OLDCODE		p->modView = GET_UNIFORM(p->fillpropCurrentShader, "fw_ModelViewMatrix");
OLDCODE		p->projMat = GET_UNIFORM(p->fillpropCurrentShader, "fw_ProjectionMatrix");
OLDCODE		p->normMat = GET_UNIFORM(p->fillpropCurrentShader, "fw_NormalMatrix");
OLDCODE
OLDCODE		#ifdef FILLVERBOSE
OLDCODE			printf ("hatchColour %d hatchPercent %d filledbool %d hatchedbool %d algor %d\n",p->hatchColour,p->hatchPercent,p->filledBool,p->hatchedBool,p->algorithm);
OLDCODE			printf ("norm %d vert %d mod %d, proj %d norm %d\n",p->norm, p->vert, p->modView, p->projMat, p->normMat);
OLDCODE		#endif
OLDCODE
OLDCODE
OLDCODE		p->fpshaderloaded = TRUE;
OLDCODE	}
OLDCODE	USE_SHADER(p->fillpropCurrentShader);
OLDCODE
OLDCODE
OLDCODE	hatchX = 0.80f; hatchY = 0.80f;
OLDCODE	algor = node->hatchStyle; filled = node->filled; hatched = node->hatched;
OLDCODE	switch (node->hatchStyle) {
OLDCODE		case 1: hatchX = 1.0f; break; /* horizontal lines */
OLDCODE		case 2: hatchY = 1.0f; break; /* vertical lines */
OLDCODE		case 3: hatchY=1.0f; break; /* positive sloped lines */
OLDCODE		case 4: hatchY=1.0f; break; /* negative sloped lines */
OLDCODE		case 5: break; /* square pattern */
OLDCODE		case 6: hatchY = 1.0f; break; /* diamond pattern */
OLDCODE
OLDCODE		default :{
OLDCODE			node->hatched = FALSE;
OLDCODE		}
OLDCODE	}
OLDCODE	GLUNIFORM2F(p->hatchPercent,hatchX, hatchY);
OLDCODE	GLUNIFORM1I(p->filledBool,filled);
OLDCODE	GLUNIFORM1I(p->hatchedBool,hatched);
OLDCODE	GLUNIFORM1I(p->algorithm,algor);
OLDCODE	GLUNIFORM4F(p->hatchColour,node->hatchColor.c[0], node->hatchColor.c[1], node->hatchColor.c[2],1.0f);
OLDCODE
OLDCODE	/* now for the transform, normal and modelview matricies */
OLDCODE	sendExplicitMatriciesToShader (p->modView, p->projMat, p->normMat);
OLDCODE
OLDCODE
OLDCODE}
OLDCODEvoid render_LineProperties (struct X3D_LineProperties *node) {
OLDCODE	GLint	factor;
OLDCODE	GLushort pat;
OLDCODE
OLDCODE	if (node->applied) {
OLDCODE		ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
OLDCODE
OLDCODE		p->linePropertySet=TRUE;
OLDCODE		if (node->linewidthScaleFactor > 1.0) {
OLDCODE			FW_GL_LINEWIDTH(node->linewidthScaleFactor);
OLDCODE			FW_GL_POINTSIZE(node->linewidthScaleFactor);
OLDCODE		}
OLDCODE
OLDCODE
OLDCODE		if (node->linetype > 1) {
OLDCODE			factor = 2;
OLDCODE			pat = 0xffff; /* can not support fancy line types - this is the default */
OLDCODE			switch (node->linetype) {
OLDCODE				case 2: pat = 0xff00; break; /* dashed */
OLDCODE				case 3: pat = 0x4040; break; /* dotted */
OLDCODE				case 4: pat = 0x04ff; break; /* dash dot */
OLDCODE				case 5: pat = 0x44fe; break; /* dash dot dot */
OLDCODE				case 6: pat = 0x0100; break; /* optional */
OLDCODE				case 7: pat = 0x0100; break; /* optional */
OLDCODE				case 10: pat = 0xaaaa; break; /* optional */
OLDCODE				case 11: pat = 0x0170; break; /* optional */
OLDCODE				case 12: pat = 0x0000; break; /* optional */
OLDCODE				case 13: pat = 0x0000; break; /* optional */
OLDCODE				default: {}
OLDCODE			}
OLDCODE			FW_GL_LINE_STIPPLE(factor,pat);
OLDCODE			FW_GL_ENABLE(GL_LINE_STIPPLE);
OLDCODE		}
OLDCODE	}
OLDCODE}
OLDCODE
OLDCODE
OLDCODE
OLDCODEvoid compile_TwoSidedMaterial (struct X3D_TwoSidedMaterial *node) {
OLDCODE	int i;
OLDCODE	float trans;
OLDCODE
OLDCODE	/* verify that the numbers are within range */
OLDCODE	if (node->ambientIntensity < 0.0) node->ambientIntensity=0.0f;
OLDCODE	if (node->ambientIntensity > 1.0) node->ambientIntensity=1.0f;
OLDCODE	if (node->shininess < 0.0) node->shininess=0.0f;
OLDCODE	if (node->shininess > 1.0) node->shininess=1.0f;
OLDCODE	if (node->transparency < 0.0) node->transparency=MIN_NODE_TRANSPARENCY;
OLDCODE	if (node->transparency >= 1.0) node->transparency=MAX_NODE_TRANSPARENCY;
OLDCODE
OLDCODE	if (node->backAmbientIntensity < 0.0) node->backAmbientIntensity=0.0f;
OLDCODE	if (node->backAmbientIntensity > 1.0) node->backAmbientIntensity=1.0f;
OLDCODE	if (node->backShininess < 0.0) node->backShininess=0.0f;
OLDCODE	if (node->backShininess > 1.0) node->backShininess=1.0f;
OLDCODE	if (node->backTransparency < 0.0) node->backTransparency=0.0f;
OLDCODE	if (node->backTransparency > 1.0) node->backTransparency=1.0f;
OLDCODE
OLDCODE	for (i=0; i<3; i++) {
OLDCODE		if (node->diffuseColor.c[i] < 0.0) node->diffuseColor.c[i]=0.0f;
OLDCODE		if (node->diffuseColor.c[i] > 1.0) node->diffuseColor.c[i]=1.0f;
OLDCODE		if (node->emissiveColor.c[i] < 0.0) node->emissiveColor.c[i]=0.0f;
OLDCODE		if (node->emissiveColor.c[i] > 1.0) node->emissiveColor.c[i]=1.0f;
OLDCODE		if (node->specularColor.c[i] < 0.0) node->specularColor.c[i]=0.0f;
OLDCODE		if (node->specularColor.c[i] > 1.0) node->specularColor.c[i]=1.0f;
OLDCODE
OLDCODE		if (node->backDiffuseColor.c[i] < 0.0) node->backDiffuseColor.c[i]=0.0f;
OLDCODE		if (node->backDiffuseColor.c[i] > 1.0) node->backDiffuseColor.c[i]=1.0f;
OLDCODE		if (node->backEmissiveColor.c[i] < 0.0) node->backEmissiveColor.c[i]=0.0f;
OLDCODE		if (node->backEmissiveColor.c[i] > 1.0) node->backEmissiveColor.c[i]=1.0f;
OLDCODE		if (node->backSpecularColor.c[i] < 0.0) node->backSpecularColor.c[i]=0.0f;
OLDCODE		if (node->backSpecularColor.c[i] > 1.0) node->backSpecularColor.c[i]=1.0f;
OLDCODE	}
OLDCODE
OLDCODE
OLDCODE	/* first, put in the transparency */
OLDCODE	trans = node->transparency;
OLDCODE        node->_verifiedFrontColor.p[3] = trans;
OLDCODE        node->_verifiedFrontColor.p[7] = trans;
OLDCODE        node->_verifiedFrontColor.p[11] = trans;
OLDCODE        node->_verifiedFrontColor.p[15] = trans;
OLDCODE	trans = node->backTransparency;
OLDCODE        node->_verifiedBackColor.p[3] = trans;
OLDCODE        node->_verifiedBackColor.p[7] = trans;
OLDCODE        node->_verifiedBackColor.p[11] = trans;
OLDCODE        node->_verifiedBackColor.p[15] = trans;
OLDCODE                
OLDCODE
OLDCODE	/* DiffuseColor */
OLDCODE	memcpy((void *)(&node->_verifiedFrontColor.p[8]), node->diffuseColor.c, sizeof (float) * 3);
OLDCODE
OLDCODE	/* Ambient  - diffuseFrontColor * ambientIntensity */
OLDCODE        for(i=0; i<3; i++) { node->_verifiedFrontColor.p[i+4] = node->_verifiedFrontColor.p[i+8] * node->ambientIntensity; }
OLDCODE
OLDCODE	/* Specular */
OLDCODE	memcpy((void *)(&node->_verifiedFrontColor.p[12]), node->specularColor.c, sizeof (float) * 3);
OLDCODE
OLDCODE	/* Emissive */
OLDCODE	memcpy((void *)(&node->_verifiedFrontColor.p[0]), node->emissiveColor.c, sizeof (float) * 3);
OLDCODE
OLDCODE	/* Shininess */
OLDCODE        node->_verifiedFrontColor.p[16] = node->shininess * 128.0f;
OLDCODE
OLDCODE#define MAX_SHIN 128.0f
OLDCODE#define MIN_SHIN 0.01f
OLDCODE        if ((node->_verifiedFrontColor.p[16] > MAX_SHIN) || (node->_verifiedFrontColor.p[16] < MIN_SHIN)) {
OLDCODE                if (node->_verifiedFrontColor.p[16]>MAX_SHIN){node->_verifiedFrontColor.p[16] = MAX_SHIN;}else{node->_verifiedFrontColor.p[16]=MIN_SHIN;}
OLDCODE        }
OLDCODE#undef MAX_SHIN
OLDCODE#undef MIN_SHIN
OLDCODE
OLDCODE	if (node->separateBackColor) {
OLDCODE
OLDCODE		/* DiffuseColor */
OLDCODE		memcpy((void *)(&node->_verifiedBackColor.p[8]), node->backDiffuseColor.c, sizeof (float) * 3);
OLDCODE	
OLDCODE		/* Ambient  - diffuseBackColor * ambientIntensity */
OLDCODE	        for(i=0; i<3; i++) { node->_verifiedBackColor.p[i+4] = node->_verifiedBackColor.p[i+8] * node->ambientIntensity; }
OLDCODE	
OLDCODE		/* Specular */
OLDCODE		memcpy((void *)(&node->_verifiedBackColor.p[12]), node->backSpecularColor.c, sizeof (float) * 3);
OLDCODE	
OLDCODE		/* Emissive */
OLDCODE		memcpy((void *)(&node->_verifiedBackColor.p[0]), node->backEmissiveColor.c, sizeof (float) * 3);
OLDCODE	
OLDCODE		/* Shininess */
OLDCODE	        node->_verifiedBackColor.p[16] = node->shininess * 128.0f;
OLDCODE	
OLDCODE#define MAX_SHIN 128.0f
OLDCODE#define MIN_SHIN 0.01f
OLDCODE	        if ((node->_verifiedBackColor.p[16] > MAX_SHIN) || (node->_verifiedBackColor.p[16] < MIN_SHIN)) {
OLDCODE	                if (node->_verifiedBackColor.p[16]>MAX_SHIN){node->_verifiedBackColor.p[16] = MAX_SHIN;}else{node->_verifiedBackColor.p[16]=MIN_SHIN;}
OLDCODE	        }
OLDCODE#undef MAX_SHIN
OLDCODE#undef MIN_SHIN
OLDCODE
OLDCODE	} else {
OLDCODE		/* just copy the front materials to the back */
OLDCODE		memcpy(node->_verifiedBackColor.p, node->_verifiedFrontColor.p, sizeof (float) * 17);
OLDCODE	}
OLDCODE
OLDCODE
OLDCODE
OLDCODE	MARK_NODE_COMPILED
OLDCODE}
OLDCODE
OLDCODEvoid render_TwoSidedMaterial (struct X3D_TwoSidedMaterial *node) {
OLDCODE	
OLDCODE	COMPILE_IF_REQUIRED
OLDCODE	{
OLDCODE	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
OLDCODE
OLDCODE	/* record this node for OpenGL-ES and OpenGL-3.1 operation */
OLDCODE	p->material_twoSided = node;
OLDCODE	}
OLDCODE}
OLDCODE
OLDCODE
OLDCODE/* bounds check the material node fields */
OLDCODEvoid compile_Material (struct X3D_Material *node) {
OLDCODE	int i;
OLDCODE	float trans;
OLDCODE
OLDCODE
OLDCODE	/* verify that the numbers are within range */
OLDCODE	if (node->ambientIntensity < 0.0f) node->ambientIntensity=0.0f;
OLDCODE	if (node->ambientIntensity > 1.0f) node->ambientIntensity=1.0f;
OLDCODE	if (node->shininess < 0.0f) node->shininess=0.0f;
OLDCODE	if (node->shininess > 1.0f) node->shininess=1.0f;
OLDCODE	if (node->transparency < 0.0f) node->transparency=MIN_NODE_TRANSPARENCY;
OLDCODE	if (node->transparency >= 1.0f) node->transparency=MAX_NODE_TRANSPARENCY;
OLDCODE
OLDCODE	for (i=0; i<3; i++) {
OLDCODE		if (node->diffuseColor.c[i] < 0.0f) node->diffuseColor.c[i]=0.0f;
OLDCODE		if (node->diffuseColor.c[i] > 1.0f) node->diffuseColor.c[i]=1.0f;
OLDCODE		if (node->emissiveColor.c[i] < 0.0f) node->emissiveColor.c[i]=0.0f;
OLDCODE		if (node->emissiveColor.c[i] > 1.0f) node->emissiveColor.c[i]=1.0f;
OLDCODE		if (node->specularColor.c[i] < 0.0f) node->specularColor.c[i]=0.0f;
OLDCODE		if (node->specularColor.c[i] > 1.0f) node->specularColor.c[i]=1.0f;
OLDCODE	}
OLDCODE
OLDCODE        /* set the transparency here for the material */
OLDCODE	/* Remember, VRML/X3D transparency 0.0 = solid; OpenGL 1.0 = solid, so we reverse it... */
OLDCODE        trans = 1.0f - node->transparency;
OLDCODE                
OLDCODE	/* we now keep verified params in a structure that maps to Shaders well...
OLDCODE		struct gl_MaterialParameters {
OLDCODE        	        vec4 emission;
OLDCODE        	        vec4 ambient;
OLDCODE        	        vec4 diffuse;
OLDCODE        	        vec4 specular;
OLDCODE        	        float shininess;
OLDCODE        	};
OLDCODE	  which is stored in the _verifiedColor[17] array here.
OLDCODE	  emission [0]..[3];
OLDCODE	  ambient [4]..[7];
OLDCODE	  diffuse [8]..[11];
OLDCODE	  specular [12]..[15];
OLDCODE	  shininess [16]
OLDCODE	  
OLDCODE*/
OLDCODE	/* first, put in the transparency */
OLDCODE        node->_verifiedColor.p[3] = trans;
OLDCODE        node->_verifiedColor.p[7] = trans;
OLDCODE        node->_verifiedColor.p[11] = trans;
OLDCODE        node->_verifiedColor.p[15] = trans;
OLDCODE                
OLDCODE	/* DiffuseColor */
OLDCODE	memcpy((void *)(&node->_verifiedColor.p[8]), node->diffuseColor.c, sizeof (float) * 3);
OLDCODE
OLDCODE	/* Ambient  - diffuseColor * ambientIntensity */
OLDCODE        for(i=0; i<3; i++) { node->_verifiedColor.p[i+4] = node->_verifiedColor.p[i+8] * node->ambientIntensity; }
OLDCODE
OLDCODE	/* Specular */
OLDCODE	memcpy((void *)(&node->_verifiedColor.p[12]), node->specularColor.c, sizeof (float) * 3);
OLDCODE
OLDCODE	/* Emissive */
OLDCODE	memcpy((void *)(&node->_verifiedColor.p[0]), node->emissiveColor.c, sizeof (float) * 3);
OLDCODE
OLDCODE	/* Shininess */
OLDCODE        node->_verifiedColor.p[16] = node->shininess * 128.0f;
OLDCODE
OLDCODE#define MAX_SHIN 128.0f
OLDCODE#define MIN_SHIN 0.01f
OLDCODE        if ((node->_verifiedColor.p[16] > MAX_SHIN) || (node->_verifiedColor.p[16] < MIN_SHIN)) {
OLDCODE                if (node->_verifiedColor.p[16]>MAX_SHIN){node->_verifiedColor.p[16] = MAX_SHIN;}else{node->_verifiedColor.p[16]=MIN_SHIN;}
OLDCODE        }
OLDCODE#undef MAX_SHIN
OLDCODE#undef MIN_SHIN
OLDCODE
OLDCODE
OLDCODE	MARK_NODE_COMPILED
OLDCODE}
OLDCODE
OLDCODE
OLDCODE/* find info on the appearance of this Shape and create a shader */
OLDCODE
OLDCODEvoid compile_Shape (struct X3D_Shape *node) {
OLDCODE	MARK_NODE_COMPILED
OLDCODE}
OLDCODE
OLDCODEvoid child_Shape (struct X3D_Shape *node) {
OLDCODE	struct X3D_Node *tmpN;
OLDCODE	int i;
OLDCODE	float dcol[4];
OLDCODE	float ecol[4];
OLDCODE	float scol[4];
OLDCODE	float amb;
OLDCODE	ppComponent_Shape p;
OLDCODE
OLDCODE	ttglobal tg = gglobal();
OLDCODE
OLDCODE	COMPILE_IF_REQUIRED
OLDCODE
OLDCODE	/* JAS - if not collision, and render_geom is not set, no need to go further */
OLDCODE	/* printf ("child_Shape vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
OLDCODE	 render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */
OLDCODE
OLDCODE	if(!(node->geometry)) { return; }
OLDCODE
OLDCODE	RECORD_DISTANCE
OLDCODE
OLDCODE	if((renderstate()->render_collision) || (renderstate()->render_sensitive)) {
OLDCODE		/* only need to forward the call to the child */
OLDCODE		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,node->geometry,tmpN);
OLDCODE		render_node(tmpN);
OLDCODE		return;
OLDCODE	}
OLDCODE
OLDCODE	/*
OLDCODE	if we want to see the bounding box of this shape:
OLDCODE	drawBBOX(X3D_NODE(node));
OLDCODE	*/
OLDCODE	p = (ppComponent_Shape)tg->Component_Shape.prv;
OLDCODE
OLDCODE	/* set up Appearance Properties here */
OLDCODE	p->this_textureTransform = NULL;
OLDCODE	p->linePropertySet=FALSE;
OLDCODE	p->appearanceProperties.transparency = MAX_NODE_TRANSPARENCY;  /* 1 == totally solid, 0 = totally transparent */  
OLDCODE	p->appearanceProperties.cubeFace = 0; /* assume no CubeMapTexture */
OLDCODE	p->material_twoSided = NULL;
OLDCODE	p->material_oneSided = NULL;
OLDCODE
OLDCODE	/* a texture and a transparency flag... */
OLDCODE
OLDCODE
OLDCODE	tg->RenderFuncs.textureStackTop = 0; /* will be >=1 if textures found */
OLDCODE	/* assume that lighting is enabled. Absence of Material or Appearance
OLDCODE	   node will turn lighting off; in this case, at the end of Shape, we
OLDCODE	   have to turn lighting back on again. */
OLDCODE	LIGHTING_ON;
OLDCODE
OLDCODE	/* is there an associated appearance node? */
OLDCODE	RENDER_MATERIAL_SUBNODES(node->appearance);
OLDCODE
OLDCODE	/* do the appearance here */
OLDCODE#ifdef SHAPE_VERBOSE
OLDCODE	printf ("child_Shape, material_oneSided %u, tg->RenderFuncs.textureStackTop %d\n",material_oneSided,textureStackTop);
OLDCODE	{int i; for (i=0; i<tg->RenderFuncs.textureStackTop; i++) {
OLDCODE		printf ("boundTextureStack[%d] is texture %d\n",i,boundTextureStack[i]);
OLDCODE		if (tg->RenderTextures.textureParameterStack[i] == NULL) {
OLDCODE			printf ("textureParameterStack empty\n");
OLDCODE		} else {
OLDCODE			struct multiTexParams *tps = (struct multiTexParams *)tg->RenderTextures.textureParameterStack[i];
OLDCODE			printf ("	texture_env_mode	 %d\n",tps->texture_env_mode);
OLDCODE			printf ("	combine_rgb		 %d\n",tps->combine_rgb);
OLDCODE			printf ("	source0_rgb		 %d\n",tps->source0_rgb);
OLDCODE			printf ("	operand0_rgb		 %d\n",tps->operand0_rgb);
OLDCODE			printf ("	source1_rgb		 %d\n",tps->source1_rgb);
OLDCODE			printf ("	operand1_rgb		 %d\n",tps->operand1_rgb);
OLDCODE			printf ("	combine_alpha		 %d\n",tps->combine_alpha);
OLDCODE			printf ("	source0_alpha		 %d\n",tps->source0_alpha);
OLDCODE			printf ("	operand0_alpha		 %d\n",tps->operand0_alpha);
OLDCODE			printf ("	source1_alpha		 %d\n",tps->source1_alpha);
OLDCODE			printf ("	operand1_alpha		 %d\n",tps->operand1_alpha);
OLDCODE			printf ("	rgb_scale		 %d\n",tps->rgb_scale);
OLDCODE			printf ("	alpha_scale		 %d\n",tps->alpha_scale);
OLDCODE
OLDCODE			//printf ("	texture_env_mode	 %d\n",textureParameterStack[i]->texture_env_mode);
OLDCODE			//printf ("	combine_rgb		 %d\n",textureParameterStack[i]->combine_rgb);
OLDCODE			//printf ("	source0_rgb		 %d\n",textureParameterStack[i]->source0_rgb);
OLDCODE			//printf ("	operand0_rgb		 %d\n",textureParameterStack[i]->operand0_rgb);
OLDCODE			//printf ("	source1_rgb		 %d\n",textureParameterStack[i]->source1_rgb);
OLDCODE			//printf ("	operand1_rgb		 %d\n",textureParameterStack[i]->operand1_rgb);
OLDCODE			//printf ("	combine_alpha		 %d\n",textureParameterStack[i]->combine_alpha);
OLDCODE			//printf ("	source0_alpha		 %d\n",textureParameterStack[i]->source0_alpha);
OLDCODE			//printf ("	operand0_alpha		 %d\n",textureParameterStack[i]->operand0_alpha);
OLDCODE			//printf ("	source1_alpha		 %d\n",textureParameterStack[i]->source1_alpha);
OLDCODE			//printf ("	operand1_alpha		 %d\n",textureParameterStack[i]->operand1_alpha);
OLDCODE			//printf ("	rgb_scale		 %d\n",textureParameterStack[i]->rgb_scale);
OLDCODE			//printf ("	alpha_scale		 %d\n",textureParameterStack[i]->alpha_scale);
OLDCODE
OLDCODE		}
OLDCODE	}
OLDCODE	}
OLDCODE#endif
OLDCODE	/* if we do NOT have a shader node, do the appearance nodes */
OLDCODE        if (p->appearanceProperties.currentShader == 0) {
OLDCODE			if (p->material_oneSided != NULL) {
OLDCODE				/* we have a normal material node */
OLDCODE				p->appearanceProperties.transparency = 1.0f - p->material_oneSided->transparency; /* 1 == solid, 0 = totally transparent */ 
OLDCODE
OLDCODE	/* we now keep verified params in a structure that maps to Shaders well...
OLDCODE		struct gl_MaterialParameters {
OLDCODE        	        vec4 emission;
OLDCODE        	        vec4 ambient;
OLDCODE        	        vec4 diffuse;
OLDCODE        	        vec4 specular;
OLDCODE        	        float shininess;
OLDCODE        	};
OLDCODE	  which is stored in the _verifiedColor[17] array here.
OLDCODE	  emission [0]..[3];
OLDCODE	  ambient [4]..[7];
OLDCODE	  diffuse [8]..[11];
OLDCODE	  specular [12]..[15];
OLDCODE	  shininess [16]
OLDCODE	  
OLDCODE*/
OLDCODE				FW_GL_MATERIALFV(GL_FRONT_AND_BACK, GL_DIFFUSE, &(p->material_oneSided->_verifiedColor.p[8])); 
OLDCODE				FW_GL_MATERIALFV(GL_FRONT_AND_BACK, GL_AMBIENT, &(p->material_oneSided->_verifiedColor.p[4]));
OLDCODE				FW_GL_MATERIALFV(GL_FRONT_AND_BACK, GL_SPECULAR, &(p->material_oneSided->_verifiedColor.p[12]));
OLDCODE				FW_GL_MATERIALFV(GL_FRONT_AND_BACK, GL_EMISSION, &(p->material_oneSided->_verifiedColor.p[0]));
OLDCODE				FW_GL_MATERIALF(GL_FRONT_AND_BACK, GL_SHININESS,p->material_oneSided->_verifiedColor.p[16]);
OLDCODE
OLDCODE				/* copy the emissive colour over for lines and points */
OLDCODE				memcpy(p->appearanceProperties.emissionColour,&(p->material_oneSided->_verifiedColor.p[0]), 3*sizeof(float));
OLDCODE			} else if (p->material_twoSided != NULL) {
OLDCODE				GLenum whichFace;
OLDCODE				float trans;
OLDCODE		
OLDCODE				/* we have a two sided material here */
OLDCODE				/* first, do back */
OLDCODE				if (p->material_twoSided->separateBackColor) {
OLDCODE					whichFace = GL_BACK;
OLDCODE					DO_MAT(p->material_twoSided,backDiffuseColor,backEmissiveColor,backShininess,backAmbientIntensity,backSpecularColor,backTransparency)
OLDCODE					whichFace = GL_FRONT;
OLDCODE				} else {
OLDCODE					whichFace=GL_FRONT_AND_BACK;
OLDCODE				}
OLDCODE				DO_MAT(p->material_twoSided,diffuseColor,emissiveColor,shininess,ambientIntensity,specularColor,transparency)
OLDCODE
OLDCODE
OLDCODE				/* Material twosided - emissiveColour for points, lines, etc - lets just set this up; we should remove
OLDCODE				   the DO_MAT calls above and do a compile-time verification of colours. */
OLDCODE				p->appearanceProperties.emissionColour[0] = 0.8f;
OLDCODE				p->appearanceProperties.emissionColour[1] = 0.8f;
OLDCODE				p->appearanceProperties.emissionColour[2] = 0.8f;
OLDCODE			} else {
OLDCODE	
OLDCODE				/* no material, so just colour the following shape */ 
OLDCODE				/* Spec says to disable lighting and set coloUr to 1,1,1 */ 
OLDCODE				LIGHTING_OFF  
OLDCODE				FW_GL_COLOR3F(1.0f,1.0f,1.0f); 
OLDCODE		 
OLDCODE				/* tell the rendering passes that this is just "normal" */ 
OLDCODE				tg->RenderFuncs.last_texture_type = NOTEXTURE; 
OLDCODE				/* same with materialProperties.transparency */ 
OLDCODE				p->appearanceProperties.transparency=MAX_NODE_TRANSPARENCY; 
OLDCODE			}
OLDCODE	}
OLDCODE
OLDCODE	/* now, are we rendering blended nodes or normal nodes?*/
OLDCODE	if (renderstate()->render_blend == (node->_renderFlags & VF_Blend)) {
OLDCODE
OLDCODE		#ifdef SHAPEOCCLUSION
OLDCODE		beginOcclusionQuery((struct X3D_VisibilitySensor*)node,renderstate()->render_geom); //BEGINOCCLUSIONQUERY;
OLDCODE		#endif
OLDCODE		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->geometry,tmpN);
OLDCODE		render_node(tmpN);
OLDCODE
OLDCODE		#ifdef SHAPEOCCLUSION
OLDCODE		endOcclusionQuery((struct X3D_VisibilitySensor*)node,renderstate()->render_geom); //ENDOCCLUSIONQUERY;
OLDCODE		#endif
OLDCODE	}
OLDCODE
OLDCODE	/* did the lack of an Appearance or Material node turn lighting off? */
OLDCODE	LIGHTING_ON;
OLDCODE
OLDCODE	/* any FillProperties? */
OLDCODE	TURN_FILLPROPERTIES_SHADER_OFF;
OLDCODE
OLDCODE
OLDCODE	if (p->linePropertySet) {
OLDCODE		FW_GL_DISABLE (GL_LINE_STIPPLE);
OLDCODE		FW_GL_LINEWIDTH(1.0f);
OLDCODE        #ifndef GL_ES_VERSION_2_0
OLDCODE		FW_GL_POINTSIZE(1.0f);
OLDCODE        #endif
OLDCODE	}
OLDCODE
OLDCODE	/* were we cubemapping? */
OLDCODE	if (p->appearanceProperties.cubeFace !=0) {
OLDCODE		FW_GL_DISABLE(GL_TEXTURE_CUBE_MAP);
OLDCODE		FW_GL_DISABLE(GL_TEXTURE_GEN_S);
OLDCODE		FW_GL_DISABLE(GL_TEXTURE_GEN_T);
OLDCODE		FW_GL_DISABLE(GL_TEXTURE_GEN_R);
OLDCODE		p->appearanceProperties.cubeFace=0;
OLDCODE	}
OLDCODE
OLDCODE
OLDCODE	/* any shader turned on? if so, turn it off */
OLDCODE	TURN_GLOBAL_SHADER_OFF;
OLDCODE
OLDCODE	/* turn off face culling */
OLDCODE	DISABLE_CULL_FACE;
OLDCODE}
OLDCODE
OLDCODE
OLDCODE#endif //SHADERS_2011
#endif //OLDCODE

