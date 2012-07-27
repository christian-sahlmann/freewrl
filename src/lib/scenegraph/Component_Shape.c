/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Shape.c,v 1.116 2012/07/27 15:40:03 crc_canada Exp $

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

#define NOTHING 0

typedef struct pComponent_Shape{

	struct matpropstruct appearanceProperties;

	/* pointer for a TextureTransform type of node */
	struct X3D_Node *  this_textureTransform;  /* do we have some kind of textureTransform? */

	/* for doing shader material properties */
	struct X3D_TwoSidedMaterial *material_twoSided;
	struct X3D_Material *material_oneSided;
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
		p->norm = -1;
		p->vert = -1;
		p->modView = -1;
		p->projMat = -1;
		p->normMat = -1;
	}
}

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

#define CHECK_COLOUR_FIELD(aaa) \
	case NODE_##aaa: { \
		struct X3D_##aaa *me = (struct X3D_##aaa *)realNode; \
		if (me->color == NULL) return NOTHING; /* do not add any properties here */\
		else return COLOUR_MATERIAL_SHADER; \
		break; \
	} 

#define CHECK_VRML1_COLOUR_FIELD(aaa) \
	case NODE_##aaa: { \
		struct X3D_##aaa *me = (struct X3D_##aaa *)realNode; \
		if (me->_color == NULL) return NOTHING; /* do not add any properties here */\
		else return COLOUR_MATERIAL_SHADER; \
		break; \
	} \

/* if this is a LineSet, PointSet, etc... */
static bool getIfLinePoints(struct X3D_Node *myGeom) {
	struct X3D_Node *realNode;

	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,myGeom,realNode);

	if (realNode == NULL) return NO_APPEARANCE_SHADER;
	switch (realNode->_nodeType) {
		case NODE_IndexedLineSet:
		case NODE_LineSet:
		case NODE_VRML1_IndexedLineSet:
		case NODE_PointSet:
		case NODE_VRML1_PointSet:
			return  true;

	}
	return false; // do not add any capabilites here.
}



/* Some shapes have Color nodes - if so, then we have other shaders */
static int getShapeColourShader (struct X3D_Node *myGeom) {
	struct X3D_Node *realNode;

	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,myGeom,realNode);

	if (realNode == NULL) return NOTHING;

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
	return NOTHING; /* do not add any capabilites here */
}

static int getAppearanceShader (struct X3D_Node *myApp) {
	struct X3D_Appearance *realAppearanceNode;
	struct X3D_Node *realMaterialNode;


	int retval = NOTHING;
    
	/* if there is no appearance node... */
	if (myApp == NULL) return retval;

	POSSIBLE_PROTO_EXPANSION(struct X3D_Appearance *, myApp,realAppearanceNode);
	if (realAppearanceNode->_nodeType != NODE_Appearance) return retval;
    
	if (realAppearanceNode->material != NULL) {
        POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, realAppearanceNode->material,realMaterialNode);
		        
		if (realMaterialNode->_nodeType == NODE_Material) {
			retval |= MATERIAL_APPEARANCE_SHADER;
		}
		if (realMaterialNode->_nodeType == NODE_TwoSidedMaterial) {
			retval |= TWO_MATERIAL_APPEARANCE_SHADER;
		}
	}
    
    
	if (realAppearanceNode->fillProperties != NULL) {
        	struct X3D_Node *fp;
        	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, realAppearanceNode->fillProperties,fp);
		if (fp->_nodeType != NODE_FillProperties) {
			ConsoleMessage("getAppearanceShader, fillProperties has a node type of %s",stringNodeType(fp->_nodeType));
		} else {
			retval |= FILL_PROPERTIES_SHADER;
		}
	}


	if (realAppearanceNode->texture != NULL) {
        //printf ("getAppearanceShader - rap node is %s\n",stringNodeType(realAppearanceNode->texture->_nodeType));
        struct X3D_Node *tex;
        
        POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, realAppearanceNode->texture,tex);
        if ((tex->_nodeType == NODE_ImageTexture) ||
            (tex->_nodeType == NODE_PixelTexture)){
			retval |= ONE_TEX_APPEARANCE_SHADER;
		} else if (tex->_nodeType == NODE_MultiTexture) {
            retval |= MULTI_TEX_APPEARANCE_SHADER;
        } else {
			ConsoleMessage ("getAppearanceShader, texture field %s not supported yet\n",
			stringNodeType(tex->_nodeType));
		}
	}
    
	#ifdef SHAPE_VERBOSE
	printf ("getAppearanceShader, returning %x\n",retval);
	#endif //SHAPE_VERBOSE

	return retval;
}


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

/* now works with our pushing matricies (norm, proj, modelview) but not for complete shader appearance replacement */
void render_FillProperties (struct X3D_FillProperties *node) {
	GLfloat hatchX;
	GLfloat hatchY;
	GLint algor;
	GLint hatched;
	GLint filled;
	int success;

    s_shader_capabilities_t *me = getAppearanceProperties()->currentShaderProperties;
	if (me==NULL) return;

	hatchX = 0.80f; hatchY = 0.80f;
	algor = node->hatchStyle; filled = node->filled; hatched = node->hatched;
	switch (node->hatchStyle) {
		case 1: hatchX = 1.0f; break; /* horizontal lines */
		case 2: hatchY = 1.0f; break; /* vertical lines */
		case 3: hatchY=1.0f; break; /* positive sloped lines */
		case 4: hatchY=1.0f; break; /* negative sloped lines */
		case 5: break; /* square pattern */
		case 6: hatchY = 1.0f; break; /* diamond pattern */

		default :{
			node->hatched = FALSE;
		}
	}

	GLUNIFORM2F(me->hatchPercent,hatchX, hatchY);
	GLUNIFORM1I(me->filledBool,filled);
	GLUNIFORM1I(me->hatchedBool,hatched);
	GLUNIFORM1I(me->algorithm,algor);
	GLUNIFORM4F(me->hatchColour,node->hatchColor.c[0], node->hatchColor.c[1], node->hatchColor.c[2],1.0f);

}


void render_LineProperties (struct X3D_LineProperties *node) {
	GLint	factor;
	GLushort pat;

	if (node->applied) {
		//ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

		if (node->linewidthScaleFactor > 1.0) {
			FW_GL_LINEWIDTH(node->linewidthScaleFactor);
            #ifndef GL_ES_VERSION_2_0
			FW_GL_POINTSIZE(node->linewidthScaleFactor);
            #endif
		}


		if (node->linetype > 1) {
			factor = 2;
			pat = 0xffff; /* can not support fancy line types - this is the default */
			switch (node->linetype) {
				case 2: pat = 0xff00; break; /* dashed */
				case 3: pat = 0x4040; break; /* dotted */
				case 4: pat = 0x04ff; break; /* dash dot */
				case 5: pat = 0x44fe; break; /* dash dot dot */
				case 6: pat = 0x0100; break; /* optional */
				case 7: pat = 0x0100; break; /* optional */
				case 10: pat = 0xaaaa; break; /* optional */
				case 11: pat = 0x0170; break; /* optional */
				case 12: pat = 0x0000; break; /* optional */
				case 13: pat = 0x0000; break; /* optional */
				default: {}
			}
			//FW_GL_LINE_STIPPLE(factor,pat);
			//FW_GL_ENABLE(GL_LINE_STIPPLE);
		}
	}
}

void child_Shape (struct X3D_Shape *node) {
	struct X3D_Node *tmpN;    
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

        /* enable the shader for this shape */
        enableGlobalShader (getMyShader(node->_shaderTableEntry));

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
    
    /* load the identity matrix for textures. This is necessary, as some nodes have TextureTransforms
        and some don't. So, if we have a TextureTransform, loadIdentity */
    
    if (p->this_textureTransform) {
        p->this_textureTransform = NULL;
        FW_GL_MATRIX_MODE(GL_TEXTURE);
        FW_GL_LOAD_IDENTITY();
        FW_GL_MATRIX_MODE(GL_MODELVIEW);
    }
    
    /* LineSet, PointSets, set the width back to the original. */
	{
		float gl_linewidth = tg->Mainloop.gl_linewidth;
		FW_GL_LINEWIDTH(gl_linewidth);
        #ifndef GL_ES_VERSION_2_0
		FW_GL_POINTSIZE(gl_linewidth);
        #endif
	}
    
	/* did the lack of an Appearance or Material node turn lighting off? */
	LIGHTING_ON;

	/* turn off face culling */
	DISABLE_CULL_FACE;
}

void compile_Shape (struct X3D_Shape *node) {
	int whichAppearanceShader = 0;
	int whichShapeColorShader = 0;
	bool isUnlitGeometry = false;
    int whichUnlitGeometry = 0;
    struct X3D_Node *tmpN = NULL;
    
    POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->geometry,tmpN);
	whichShapeColorShader = getShapeColourShader(tmpN);
    isUnlitGeometry = getIfLinePoints(tmpN);
    
    POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->appearance,tmpN);

    /* Lines, points - can get the emission colour from an appearance node */
    if (isUnlitGeometry) {
        int myAppShad =  getAppearanceShader(tmpN);
        
        /* Ok. We if whichShapeColorShader is non-zero, we have a Colour node.
           if it is zero, we either have an appearance node, or we have no-material. */
        ConsoleMessage ("unlit geometry, appearance shader is %d whichShapeColorShader is %d",myAppShad,whichShapeColorShader);
        if ((whichShapeColorShader != 0) || (myAppShad != 0)) {
            ConsoleMessage ("Unlit geometry, but with either a Color node, or an appearance node");
            
            /* which one first? */
            if (whichShapeColorShader != 0) whichUnlitGeometry = HAVE_LINEPOINTS_COLOR;
            else whichUnlitGeometry = HAVE_LINEPOINTS_APPEARANCE;
        }
    } else {
    	whichAppearanceShader = getAppearanceShader(tmpN);
    }
    
    /* in case we had no appearance, etc, we do the bland NO_APPEARANCE_SHADER */
    node->_shaderTableEntry= (whichShapeColorShader | whichAppearanceShader | whichUnlitGeometry);
    
    
	if (node->_shaderTableEntry == NOTHING) 
        node->_shaderTableEntry = NO_APPEARANCE_SHADER;

    printf ("compile_Shape, node->_shaderTableEntry is %x\n",node->_shaderTableEntry);
    
	MARK_NODE_COMPILED
}


void compile_TwoSidedMaterial (struct X3D_TwoSidedMaterial *node) {
	int i;
	float trans;

	/* verify that the numbers are within range */
	if (node->ambientIntensity < 0.0) node->ambientIntensity=0.0f;
	if (node->ambientIntensity > 1.0) node->ambientIntensity=1.0f;
	if (node->shininess < 0.0) node->shininess=0.0f;
	if (node->shininess > 1.0) node->shininess=1.0f;
	if (node->transparency < 0.0) node->transparency=MIN_NODE_TRANSPARENCY;
	if (node->transparency >= 1.0) node->transparency=MAX_NODE_TRANSPARENCY;

	if (node->backAmbientIntensity < 0.0) node->backAmbientIntensity=0.0f;
	if (node->backAmbientIntensity > 1.0) node->backAmbientIntensity=1.0f;
	if (node->backShininess < 0.0) node->backShininess=0.0f;
	if (node->backShininess > 1.0) node->backShininess=1.0f;
	if (node->backTransparency < 0.0) node->backTransparency=0.0f;
	if (node->backTransparency > 1.0) node->backTransparency=1.0f;

	for (i=0; i<3; i++) {
		if (node->diffuseColor.c[i] < 0.0) node->diffuseColor.c[i]=0.0f;
		if (node->diffuseColor.c[i] > 1.0) node->diffuseColor.c[i]=1.0f;
		if (node->emissiveColor.c[i] < 0.0) node->emissiveColor.c[i]=0.0f;
		if (node->emissiveColor.c[i] > 1.0) node->emissiveColor.c[i]=1.0f;
		if (node->specularColor.c[i] < 0.0) node->specularColor.c[i]=0.0f;
		if (node->specularColor.c[i] > 1.0) node->specularColor.c[i]=1.0f;

		if (node->backDiffuseColor.c[i] < 0.0) node->backDiffuseColor.c[i]=0.0f;
		if (node->backDiffuseColor.c[i] > 1.0) node->backDiffuseColor.c[i]=1.0f;
		if (node->backEmissiveColor.c[i] < 0.0) node->backEmissiveColor.c[i]=0.0f;
		if (node->backEmissiveColor.c[i] > 1.0) node->backEmissiveColor.c[i]=1.0f;
		if (node->backSpecularColor.c[i] < 0.0) node->backSpecularColor.c[i]=0.0f;
		if (node->backSpecularColor.c[i] > 1.0) node->backSpecularColor.c[i]=1.0f;
	}


	/* first, put in the transparency */
	trans = 1.0 - node->transparency;
        node->_verifiedFrontColor.p[3] = trans;
        node->_verifiedFrontColor.p[7] = trans;
        node->_verifiedFrontColor.p[11] = trans;
        node->_verifiedFrontColor.p[15] = trans;
	trans = 1.0 - node->backTransparency;
        node->_verifiedBackColor.p[3] = trans;
        node->_verifiedBackColor.p[7] = trans;
        node->_verifiedBackColor.p[11] = trans;
        node->_verifiedBackColor.p[15] = trans;
                

	/* DiffuseColor */
	memcpy((void *)(&node->_verifiedFrontColor.p[8]), node->diffuseColor.c, sizeof (float) * 3);

	/* Ambient  - diffuseFrontColor * ambientIntensity */
        for(i=0; i<4; i++) { node->_verifiedFrontColor.p[i+4] = node->_verifiedFrontColor.p[i+8] * node->ambientIntensity; }

	/* Specular */
	memcpy((void *)(&node->_verifiedFrontColor.p[12]), node->specularColor.c, sizeof (float) * 3);

	/* Emissive */
	memcpy((void *)(&node->_verifiedFrontColor.p[0]), node->emissiveColor.c, sizeof (float) * 3);

	/* Shininess */
        node->_verifiedFrontColor.p[16] = node->shininess * 128.0f;

#define MAX_SHIN 128.0f
#define MIN_SHIN 0.01f
        if ((node->_verifiedFrontColor.p[16] > MAX_SHIN) || (node->_verifiedFrontColor.p[16] < MIN_SHIN)) {
                if (node->_verifiedFrontColor.p[16]>MAX_SHIN){node->_verifiedFrontColor.p[16] = MAX_SHIN;}else{node->_verifiedFrontColor.p[16]=MIN_SHIN;}
        }
#undef MAX_SHIN
#undef MIN_SHIN

	if (node->separateBackColor) {

		/* DiffuseColor */
		memcpy((void *)(&node->_verifiedBackColor.p[8]), node->backDiffuseColor.c, sizeof (float) * 3);
	
		/* Ambient  - diffuseBackColor * ambientIntensity */
	        for(i=0; i<3; i++) { node->_verifiedBackColor.p[i+4] = node->_verifiedBackColor.p[i+8] * node->ambientIntensity; }
	
		/* Specular */
		memcpy((void *)(&node->_verifiedBackColor.p[12]), node->backSpecularColor.c, sizeof (float) * 3);
	
		/* Emissive */
		memcpy((void *)(&node->_verifiedBackColor.p[0]), node->backEmissiveColor.c, sizeof (float) * 3);
	
		/* Shininess */
	        node->_verifiedBackColor.p[16] = node->shininess * 128.0f;
	
#define MAX_SHIN 128.0f
#define MIN_SHIN 0.01f
	        if ((node->_verifiedBackColor.p[16] > MAX_SHIN) || (node->_verifiedBackColor.p[16] < MIN_SHIN)) {
	                if (node->_verifiedBackColor.p[16]>MAX_SHIN){node->_verifiedBackColor.p[16] = MAX_SHIN;}else{node->_verifiedBackColor.p[16]=MIN_SHIN;}
	        }
#undef MAX_SHIN
#undef MIN_SHIN

	} else {
		/* just copy the front materials to the back */
		memcpy(node->_verifiedBackColor.p, node->_verifiedFrontColor.p, sizeof (float) * 17);
	}



	MARK_NODE_COMPILED
}

void render_TwoSidedMaterial (struct X3D_TwoSidedMaterial *node) {
	
	COMPILE_IF_REQUIRED
	{
	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

	/* record this node for OpenGL-ES and OpenGL-3.1 operation */
	p->material_twoSided = node;
	}
}

