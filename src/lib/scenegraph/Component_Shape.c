/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Shape.c,v 1.112 2012/07/17 22:29:35 crc_canada Exp $

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

/* now works with our pushing matricies (norm, proj, modelview) but not for complete shader appearance replacement */
void render_FillProperties (struct X3D_FillProperties *node) {
#ifdef OLDERCODE
	GLfloat hatchX;
	GLfloat hatchY;
	GLint algor;
	GLint hatched;
	GLint filled;
	int success;

	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

	if (!p->fpshaderloaded) {
		const char *vs = "\
			/* \n\
			  Shader source from \n\
			  \"Introduction to the OpenGL Shading Language\" \n\
			  presentation by Randi Rost, 3DLabs (GLSLOverview2005.pdf) \n\
			*/ \n\
			 \n\
uniform                mat4 fw_ModelViewMatrix; \n\
uniform                mat4 fw_ProjectionMatrix; \n\
uniform mat3	fw_NormalMatrix; \n\
			uniform vec3 LightPosition; \n\
			uniform bool filled; \n\
			const float SpecularContribution = 0.3; \n\
			const float DiffuseContribution = 1.0 - SpecularContribution; \n\
			varying float LightIntensity; \n\
			varying vec2 MCposition; \n\
			void main(void) \n\
			{ \n\
               gl_Position = fw_ProjectionMatrix * fw_ModelViewMatrix * gl_Vertex; \n\
			    vec3 ecPosition = vec3(fw_ModelViewMatrix * gl_Vertex); \n\
			    vec3 tnorm      = normalize(fw_NormalMatrix * gl_Normal); \n\
			    vec3 lightVec   = normalize(LightPosition - ecPosition); \n\
			    vec3 reflectVec = reflect(-lightVec, tnorm); \n\
			    vec3 viewVec    = normalize(-ecPosition); \n\
			    float diffuse   = max(dot(lightVec, tnorm), 0.0); \n\
			    float spec      = 0.0; \n\
			    if (diffuse > 0.0) \n\
			    { \n\
			        spec = max(dot(reflectVec, viewVec), 0.0); \n\
			        spec = pow(spec, 16.0); \n\
			    } \n\
			    LightIntensity = DiffuseContribution * diffuse + \n\
			                       SpecularContribution * spec; \n\
			    MCposition      = gl_Vertex.xy; \n\
			    /* old - JAS gl_Position     = ftransform(); */ \n\
			    // Get the vertex colour\n\
			    if (filled) gl_FrontColor = gl_FrontMaterial.diffuse;\n\
			    else gl_FrontColor = vec4(0.0, 0.0, 0.0, 0.0); // make transparent \n\
			} \n\
		";

		const char *fs = "\
			/*  \n\
			  Shader source from  \n\
			  \"Introduction to the OpenGL Shading Language\"  \n\
			  presentation by Randi Rost, 3DLabs (GLSLOverview2005.pdf)  \n\
			*/  \n\
			  \n\
			// HatchSize - x and y - larger = less hatches on shape \n\
			const vec2  HatchSize= vec2(0.15, 0.15);  \n\
			  \n\
			uniform vec4 HatchColour;     //= (0.85,0.86,0.84);  \n\
			uniform bool hatched;\n\
			uniform bool filled; \n\
			uniform vec2  HatchPct;               //= (0.90, 0.85);  \n\
			uniform int algorithm;	\n\
			varying vec2  MCposition;  \n\
			varying float LightIntensity;  \n\
			  \n\
			void main(void)  \n\
			{  \n\
			    vec4 color;  \n\
			    vec2 position, useBrick;  \n\
			    vec4 fragCol = gl_Color; \n\
			  \n\
			    position = MCposition / HatchSize;  \n\
			  \n\
			    if (algorithm == 0) {// bricking \n\
			    	if (fract(position.y * 0.5) > 0.5)  \n\
			            position.x += 0.5;  \n\
			   } \n\
			  \n\
			    // algorithm 1, 2 = no futzing required here \n\
			    if (algorithm == 3) {// positive diagonals \n\
				    vec2 curpos = position; \n\
			            position.x -= curpos.y;  \n\
			   } \n\
			  \n\
			    if (algorithm == 4) {// negative diagonals \n\
				    vec2 curpos = position; \n\
			            position.x += curpos.y;  \n\
			   } \n\
			  \n\
			    if (algorithm == 6) {// diagonal crosshatch \n\
				vec2 curpos = position; \n\
				if (fract(position.y) > 0.5)  { \n\
        			    if (fract(position.x) < 0.5) position.x += curpos.y; \n\
        			    else position.x -= curpos.y; \n\
				} else { \n\
        			    if (fract(position.x) > 0.5) position.x += curpos.y; \n\
        			    else position.x -= curpos.y; \n\
				} \n\
			   } \n\
			  \n\
			    position = fract(position);  \n\
			  \n\
			    useBrick = step(position, HatchPct);  \n\
			  \n\
			    if (hatched) color = mix(HatchColour, fragCol, useBrick.x * useBrick.y);  \n\
			    else color = fragCol; \n\
			  \n\
			    color *= LightIntensity;  \n\
			    gl_FragColor = color;  \n\
				if (filled) gl_FragColor.a = 1.0; //JAS \n\
			}  \n\
		";

		GLuint v;
		GLuint f;
	
		#ifdef FILLVERBOSE
			printf ("creating shaders\n");
		#endif


		v = CREATE_SHADER(GL_VERTEX_SHADER);
		f = CREATE_SHADER(GL_FRAGMENT_SHADER);	
	
		#ifdef FILLVERBOSE
			printf ("assigning shader source\n");
		#endif


		SHADER_SOURCE(v, 1, &vs,NULL);
		SHADER_SOURCE(f, 1, &fs,NULL);
	
		#ifdef FILLVERBOSE
			printf ("compiling shaders\n");
		#endif


		COMPILE_SHADER(v);
	
                COMPILE_SHADER(v);
                GET_SHADER_INFO(v, COMPILE_STATUS, &success);
                if (!success) {
                        shaderErrorLog(v,"GEOMETRY");
                }

		COMPILE_SHADER(f);
                GET_SHADER_INFO(f, COMPILE_STATUS, &success);
                if (!success) {
                        shaderErrorLog(f,"GEOMETRY");
                }

		#ifdef FILLVERBOSE
			printf ("creating program and attaching\n");
		#endif

		p->fillpropCurrentShader = CREATE_PROGRAM;
		
		ATTACH_SHADER(p->fillpropCurrentShader,v);
		ATTACH_SHADER(p->fillpropCurrentShader,f);
	
		#ifdef FILLVERBOSE
			printf ("linking program\n");
		#endif


		LINK_SHADER(p->fillpropCurrentShader);

		#ifdef FILLVERBOSE
			printf ("getting shader vars\n");
		#endif

		p->hatchColour = GET_UNIFORM(p->fillpropCurrentShader,"HatchColour");
		p->hatchPercent = GET_UNIFORM(p->fillpropCurrentShader,"HatchPct");
		p->filledBool = GET_UNIFORM(p->fillpropCurrentShader,"filled");
		p->hatchedBool = GET_UNIFORM(p->fillpropCurrentShader,"hatched");
		p->algorithm = GET_UNIFORM(p->fillpropCurrentShader,"algorithm");
		p->modView = GET_UNIFORM(p->fillpropCurrentShader, "fw_ModelViewMatrix");
		p->projMat = GET_UNIFORM(p->fillpropCurrentShader, "fw_ProjectionMatrix");
		p->normMat = GET_UNIFORM(p->fillpropCurrentShader, "fw_NormalMatrix");

		#ifdef FILLVERBOSE
			printf ("hatchColour %d hatchPercent %d filledbool %d hatchedbool %d algor %d\n",p->hatchColour,p->hatchPercent,p->filledBool,p->hatchedBool,p->algorithm);
			printf ("norm %d vert %d mod %d, proj %d norm %d\n",p->norm, p->vert, p->modView, p->projMat, p->normMat);
		#endif


		p->fpshaderloaded = TRUE;
	}
	USE_SHADER(p->fillpropCurrentShader);


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
	GLUNIFORM2F(p->hatchPercent,hatchX, hatchY);
	GLUNIFORM1I(p->filledBool,filled);
	GLUNIFORM1I(p->hatchedBool,hatched);
	GLUNIFORM1I(p->algorithm,algor);
	GLUNIFORM4F(p->hatchColour,node->hatchColor.c[0], node->hatchColor.c[1], node->hatchColor.c[2],1.0f);

	/* now for the transform, normal and modelview matricies */
	sendExplicitMatriciesToShader (p->modView, p->projMat, p->normMat);
#endif //OLDERCODE
}


void render_LineProperties (struct X3D_LineProperties *node) {
	GLint	factor;
	GLushort pat;

	if (node->applied) {
		ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

		p->linePropertySet=TRUE;
		if (node->linewidthScaleFactor > 1.0) {
			FW_GL_LINEWIDTH(node->linewidthScaleFactor);
			FW_GL_POINTSIZE(node->linewidthScaleFactor);
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
    struct X3D_Node *tmpN = NULL;
    
    POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->geometry,tmpN);
	whichShapeColorShader = getShapeColourShader(tmpN);
    isUnlitGeometry = getIfLinePoints(tmpN);
    
    POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->appearance,tmpN);
	whichAppearanceShader = getAppearanceShader(tmpN);
    
	



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


