/*
=INSERT_TEMPLATE_HERE=

$Id: Component_ProgrammableShaders.c,v 1.53 2011/06/02 19:50:43 dug9 Exp $

X3D Programmable Shaders Component

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



/* Mapping X3D types to shaders

Notes:
	If the shader variable is not defined AND used in the shader, you will get an error on initialization
	via X3D. Even if it is defined as a Uniform variable, if it does not exist in the shader, it will not 
	be able to be initialized, and thus the shader interface will return an error.

X3D type			GLSL type		Initialize	Route In	Route Out
-------------------------------------------------------------------------------------------------
FIELDTYPE_SFFloat		GL_FLOAT		YES		YES
FIELDTYPE_MFFloat		GL_FLOAT		YES		YES
FIELDTYPE_SFRotation		GL_FLOAT_VEC4		YES		YES
FIELDTYPE_MFRotation		GL_FLOAT_VEC4		YES		YES
FIELDTYPE_SFVec3f		GL_FLOAT_VEC3		YES		YES
FIELDTYPE_MFVec3f		GL_FLOAT_VEC3		YES		YES
FIELDTYPE_SFBool		GL_BOOL			YES		YES
FIELDTYPE_MFBool		GL_BOOL			YES		YES
FIELDTYPE_SFInt32		GL_INT			YES		YES
FIELDTYPE_MFInt32		GL_INT			YES		YES
FIELDTYPE_SFNode		
FIELDTYPE_MFNode		--
FIELDTYPE_SFColor		GL_FLOAT_VEC3		YES		YES
FIELDTYPE_MFColor		GL_FLOAT_VEC3		YES		YES
FIELDTYPE_SFColorRGBA		GL_FLOAT_VEC4		YES		YES
FIELDTYPE_MFColorRGBA		GL_FLOAT_VEC4		YES		YES
FIELDTYPE_SFTime		GL_FLOAT		YES(float)	YES(float)
FIELDTYPE_MFTime	
FIELDTYPE_SFString		--
FIELDTYPE_MFString		--
FIELDTYPE_SFVec2f		GL_FLOAT_VEC2		YES		YES
FIELDTYPE_MFVec2f		GL_FLOAT_VEC2		YES		YES
FIELDTYPE_SFImage	
FIELDTYPE_FreeWRLPTR		--
FIELDTYPE_SFVec3d		GL_FLOAT_VEC3		YES(float)	YES(float)
FIELDTYPE_MFVec3d	
FIELDTYPE_SFDouble		GL_FLOAT		YES(float)	YES(float)
FIELDTYPE_MFDouble	
FIELDTYPE_SFMatrix3f	
FIELDTYPE_MFMatrix3f	
FIELDTYPE_SFMatrix3d	
FIELDTYPE_MFMatrix3d	
FIELDTYPE_SFMatrix4f	
FIELDTYPE_MFMatrix4f	
FIELDTYPE_SFMatrix4d	
FIELDTYPE_MFMatrix4d	
FIELDTYPE_SFVec2d		GL_FLOAT_2		YES(float)	YES(float)
FIELDTYPE_MFVec2d	
FIELDTYPE_SFVec4f		GL_FLOAT_VEC4		YES		YES
FIELDTYPE_MFVec4f				
FIELDTYPE_SFVec4d		GL_FLOAT_VEC4		YES(float)	YES(float)
FIELDTYPE_MFVec4d	
*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../vrml_parser/CRoutes.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../lib/scenegraph/Vector.h"

#include "../world_script/JScript.h"
#include "../world_script/CScripts.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../opengl/OpenGL_Utils.h"
#include "../scenegraph/Component_Shape.h"
#include "../opengl/Textures.h"
#include "Component_ProgrammableShaders.h"

static void sendInitialFieldsToShader(struct X3D_Node *);

#define MAX_INFO_LOG_SIZE 512
/* we do support older versions of shaders; but not all info logs are printed if we
   have OpenGL prior to 2.0 */

#define SUPPORT_GLSL_ONLY \
	if (strcmp(node->language->strptr,"GLSL")) { \
		ConsoleMessage ("Shaders: support only GLSL shading language, got :%s:, skipping...",node->language->strptr); \
		node->isValid = FALSE; \
	}

#define RUN_IF_VALID \
		if (node->isValid) { \
			if (node->__shaderIDS.n != 0) { \
				appearanceProperties.currentShader = (GLuint) node->__shaderIDS.p[0]; \
				USE_SHADER(appearanceProperties.currentShader); \
				if (!node->__initialized) sendInitialFieldsToShader(X3D_NODE(node)); \
			} \
		}

#define CHECK_SHADERS \
	if (!gglobal()->display.rdr_caps.av_glsl_shaders) { \
		if (node->isValid) ConsoleMessage ("have an X3D program with shaders, but no shader support on this computer"); \
		node->isValid = FALSE; \
		return; \
	}

static void shaderErrorLog(GLuint myShader) {
	#ifdef GL_VERSION_2_0 
		GLchar infoLog[MAX_INFO_LOG_SIZE]; 
		glGetShaderInfoLog(myShader, MAX_INFO_LOG_SIZE, NULL, infoLog); 
		ConsoleMessage ("problem with VERTEX shader: %s",infoLog); 
	#else 
		ConsoleMessage ("Problem compiling shader"); 
	#endif 
}


/* common code to compile and link shaders */
#define COMPILE_IF_VALID(subField)  \
	if (node->isValid) { \
		GLint success; \
		GLuint myVertexShader = 0; \
		GLuint myFragmentShader= 0; \
		 \
 \
		if (haveVertShaderText) { \
			myVertexShader = CREATE_SHADER (VERTEX_SHADER);	 \
			SHADER_SOURCE(myVertexShader, node->subField.n, (const GLchar **) vertShaderSource, NULL); \
			COMPILE_SHADER(myVertexShader); \
			GET_SHADER_INFO(myVertexShader, COMPILE_STATUS, &success); \
			if (!success) { \
				shaderErrorLog(myVertexShader); \
				node->isValid = FALSE; \
			} else { \
				ATTACH_SHADER(myProgram, myVertexShader); \
			} \
		} \
		 \
		if (haveFragShaderText) {	 \
			myFragmentShader = CREATE_SHADER (FRAGMENT_SHADER);	 \
			SHADER_SOURCE(myFragmentShader, node->subField.n,(const GLchar **)  fragShaderSource, NULL); \
			COMPILE_SHADER(myFragmentShader); \
 \
			GET_SHADER_INFO(myFragmentShader, COMPILE_STATUS, &success); \
			if (!success) { \
				shaderErrorLog(myFragmentShader); \
				node->isValid = FALSE; \
			} else { \
				ATTACH_SHADER(myProgram, myFragmentShader); \
			} \
		} \
	}

#define COMPILE_SHADER_PARTS(myNodeType, myField) \
		for (i=0; i<node->myField.n; i++) { \
			struct X3D_##myNodeType *prog; \
			prog = (struct X3D_##myNodeType *) node->myField.p[i]; \
			vertShaderSource[i] = ""; \
			fragShaderSource[i] = ""; \
 \
			if (prog!=NULL) { \
				if (prog->_nodeType == NODE_##myNodeType) { \
					/* compile this program */ \
 \
					if (!((strcmp (prog->type->strptr,"VERTEX")) && (strcmp(prog->type->strptr,"FRAGMENT")))) { \
						char *myText = NULL; /* pointer to text to send in */ \
						char **cptr; /* returned caracter pointer pointer */ \
						 \
						  cptr = shader_initCodeFromMFUri(&prog->url); \
						  if (cptr == NULL) { \
							ConsoleMessage ("error reading url for :%s:",stringNodeType(NODE_##myNodeType)); \
							myText = ""; \
						  } else { myText = *cptr; \
\
						/* assign this text to VERTEX or FRAGMENT buffers */ \
						if (!strcmp(prog->type->strptr,"VERTEX")) { \
							vertShaderSource[i] = STRDUP(myText); \
							haveVertShaderText = TRUE; \
						} else { \
							fragShaderSource[i] = STRDUP(myText); \
							haveFragShaderText = TRUE; \
						} \
						/* printf ("Shader text for type %s is  %s\n",prog->type->strptr,myText); */ \
						FREE_IF_NZ(*cptr); }\
					} else { \
						ConsoleMessage ("%s, invalid Type, got \"%s\"",stringNodeType(NODE_##myNodeType), prog->type->strptr); \
						node->isValid = FALSE; \
					} \
				} else { \
					ConsoleMessage ("Shader, expected \"%s\", got \"%s\"",stringNodeType(NODE_##myNodeType), stringNodeType(prog->_nodeType)); \
					node->isValid = FALSE; \
				} \
			} \
		} 

#ifdef GL_VERSION_2_0
	#define LINK_IF_VALID \
		if (node->isValid) { \
			GLint success; \
			/* link the shader programs together */ \
			LINK_SHADER(myProgram); \
			glGetProgramiv(myProgram, GL_LINK_STATUS, &success); \
			if (!success) { \
				GLchar infoLog[MAX_INFO_LOG_SIZE]; \
				glGetProgramInfoLog(myProgram, MAX_INFO_LOG_SIZE, NULL, infoLog); \
				printf ("problem with Shader Program link: %s\n",infoLog); \
				node->isValid = FALSE; \
			} \
			/* does the program get a thumbs up? */	 \
			glValidateProgram (myProgram); \
			glGetProgramiv(myProgram, GL_VALIDATE_STATUS, &success); \
			if (!success) { \
				GLchar infoLog[MAX_INFO_LOG_SIZE]; \
				glGetProgramInfoLog(myProgram, MAX_INFO_LOG_SIZE, NULL, infoLog); \
				printf ("problem with Shader Program Validate: %s\n",infoLog); \
				node->isValid = FALSE; \
			} \
			if (node->__shaderIDS.n == 0) { \
				node->__shaderIDS.n = 1; \
				node->__shaderIDS.p = MALLOC(GLint *, sizeof (GLint)); \
				node->__shaderIDS.p[0] = (int)myProgram; \
			} \
		}
#else
	#define LINK_IF_VALID \
		if (node->isValid) { \
			/* link the shader programs together */ \
			LINK_SHADER(myProgram); \
			if (node->__shaderIDS.n == 0) { \
				node->__shaderIDS.n = 1; \
				node->__shaderIDS.p = MALLOC(GLint *, sizeof (GLint)); \
				node->__shaderIDS.p[0] = (int)myProgram; \
			} \
		}
#endif

/* do type checking of shader and field variables when initializing interface */
static int shader_checkType(struct FieldDecl * myField,
		GLuint myShader, GLint myVar) {
	int retval;

	/* check the type, if we are OpenGL 2.0 or above */

	#ifdef GL_VERSION_2_0
	GLsizei len;
	GLint size;
	GLenum type;
	GLchar ch[100];		

	retval = FALSE;
	ch[0] = '\0';
	
	glGetActiveUniform(myShader,myVar,90,&len,&size,&type,ch);

	/* verify that the X3D fieldType matches the Shader type */
	switch (fieldDecl_getType(myField)) {
		case FIELDTYPE_SFFloat: 	retval = type == GL_FLOAT; break;
		case FIELDTYPE_MFFloat: 	retval = type == GL_FLOAT; break;
		case FIELDTYPE_SFRotation: 	retval = type == GL_FLOAT_VEC4; break;
		case FIELDTYPE_MFRotation: 	retval = type == GL_FLOAT_VEC4; break;
		case FIELDTYPE_SFVec3f: 	retval = type == GL_FLOAT_VEC3; break;
		case FIELDTYPE_MFVec3f: 	retval = type == GL_FLOAT_VEC3; break;
		case FIELDTYPE_MFInt32:
		case FIELDTYPE_SFInt32:		retval = type == GL_INT; break;
		case FIELDTYPE_MFBool: 		
		case FIELDTYPE_SFBool: 		retval = type == GL_BOOL; break;
		case FIELDTYPE_SFNode: 		break;
		case FIELDTYPE_MFNode: 		break;
		case FIELDTYPE_SFColor: 	retval = type == GL_FLOAT_VEC3; break;
		case FIELDTYPE_MFColor: 	retval = type == GL_FLOAT_VEC3; break;
		case FIELDTYPE_SFColorRGBA: 	retval = type == GL_FLOAT_VEC4; break;
		case FIELDTYPE_MFColorRGBA: 	retval = type == GL_FLOAT_VEC4; break;
		case FIELDTYPE_SFTime: 		retval = type == GL_FLOAT; break;
		case FIELDTYPE_MFTime: 		break;
		case FIELDTYPE_SFString: 	break;
		case FIELDTYPE_MFString: 	break;
		case FIELDTYPE_SFVec2f: 	retval = type ==  GL_FLOAT_VEC2; break;
		case FIELDTYPE_MFVec2f: 	retval = type ==  GL_FLOAT_VEC2; break;
		case FIELDTYPE_SFImage: 	break;
		case FIELDTYPE_FreeWRLPTR: 	break;
		case FIELDTYPE_SFVec3d: 	retval = type == GL_FLOAT_VEC3; break;
		case FIELDTYPE_MFVec3d: 	break;
		case FIELDTYPE_SFDouble: 	retval = type ==  GL_FLOAT; break;
		case FIELDTYPE_MFDouble:	break;
		case FIELDTYPE_SFMatrix3f:	break;
		case FIELDTYPE_MFMatrix3f:	break;
		case FIELDTYPE_SFMatrix3d:	break;
		case FIELDTYPE_MFMatrix3d:	break;
		case FIELDTYPE_SFMatrix4f:	break;
		case FIELDTYPE_MFMatrix4f:	break;
		case FIELDTYPE_SFMatrix4d:	break;
		case FIELDTYPE_MFMatrix4d: 	break;
		case FIELDTYPE_SFVec2d: 	retval = type == GL_FLOAT_VEC2; break;
		case FIELDTYPE_MFVec2d: 	break;
		case FIELDTYPE_SFVec4f: 	retval = type == GL_FLOAT_VEC4; break;
		case FIELDTYPE_MFVec4f: 	break;
		case FIELDTYPE_SFVec4d: 	retval = type == GL_FLOAT_VEC4; break;
		case FIELDTYPE_MFVec4d: 	break;
	}

	/* did we have an error? */

	if (!retval) {
		ConsoleMessage ("Shader type check: X3D type and shader type not compatible for variable :%s:",ch);
#ifdef VERBOSE
	printf ("shaderCheck mode %d (%s) type %d (%s) name %d\n",fieldDecl_getAccessType(myField),
			stringPROTOKeywordType(fieldDecl_getAccessType(myField)), 
			fieldDecl_getType(myField), stringFieldtypeType(fieldDecl_getType(myField)),
			fieldDecl_getIndexName(myField));

	printf ("len %d size %d type %d ch %s\n",len,size,type,ch);
	switch (type) {

	case GL_FLOAT: printf ("GL_FLOAT\n"); break;
	case GL_FLOAT_VEC2: printf ("GL_FLOAT_VEC2\n"); break;
	case GL_FLOAT_VEC3: printf ("GL_FLOAT_VEC3\n"); break;
	case GL_FLOAT_VEC4: printf ("GL_FLOAT_VEC4\n"); break;
	case GL_INT: printf ("GL_INT\n"); break;
	case GL_INT_VEC2: printf ("GL_INT_VEC2\n"); break;
	case GL_INT_VEC3: printf ("GL_INT_VEC3\n"); break;
	case GL_INT_VEC4: printf ("GL_INT_VEC4\n"); break;
	case GL_BOOL: printf ("GL_BOOL\n"); break;
	case GL_BOOL_VEC2: printf ("GL_BOOL_VEC2\n"); break;
	case GL_BOOL_VEC3: printf ("GL_BOOL_VEC3\n"); break;
	case GL_BOOL_VEC4: printf ("GL_BOOL_VEC4\n"); break;
	case GL_FLOAT_MAT2: printf ("GL_FLOAT_MAT2\n"); break;
	case GL_FLOAT_MAT3: printf ("GL_FLOAT_MAT3\n"); break;
	case GL_FLOAT_MAT4: printf ("GL_FLOAT_MAT4\n"); break;
/*
	case GL_FLOAT_MAT2x3: printf ("GL_FLOAT_MAT2x3\n"); break;
	case GL_FLOAT_MAT2x4: printf ("GL_FLOAT_MAT2x4\n"); break;
	case GL_FLOAT_MAT3x2: printf ("GL_FLOAT_MAT3x2\n"); break;
	case GL_FLOAT_MAT3x4: printf ("GL_FLOAT_MAT3x4\n"); break;
	case GL_FLOAT_MAT4x2: printf ("GL_FLOAT_MAT4x2\n"); break;
	case GL_FLOAT_MAT4x3: printf ("GL_FLOAT_MAT4x3\n"); break;
*/
	case GL_SAMPLER_1D: printf ("GL_SAMPLER_1D\n"); break;
	case GL_SAMPLER_2D: printf ("GL_SAMPLER_2D\n"); break;
	case GL_SAMPLER_3D: printf ("GL_SAMPLER_3D\n"); break;
	case GL_SAMPLER_CUBE: printf ("GL_SAMPLER_CUBE\n"); break;
	case GL_SAMPLER_1D_SHADOW: printf ("GL_SAMPLER_1D_SHADOW\n"); break;
	case GL_SAMPLER_2D_SHADOW: printf ("GL_SAMPLER_2D_SHADOW\n"); break;
default :{printf ("not decoded yet, probably a matrix type\n");}
	}
#endif
	}
#endif 
	return retval;
}
#undef VERBOSE


/* fieldDecl_getshaderVariableID(myf), fieldDecl_getValue(myf)); */
static void sendValueToShader(struct ScriptFieldDecl* myField) {
	GLint shaderVariable = fieldDecl_getshaderVariableID(myField->fieldDecl);

	#ifdef SHADERVERBOSE
	printf ("sendValueToShader... ft %s\n",stringFieldtypeType(fieldDecl_getType(myField->fieldDecl)));
	printf ("shaderVariableID is %d\n",shaderVariable);
	#endif

	/* either not defined in the shader, OR not used in the shader so it is stripped by glsl compiler */
	if (shaderVariable == INT_ID_UNDEFINED) return;

	switch (fieldDecl_getType(myField->fieldDecl)) {

#define SF_FLOATS_TO_SHADER(ttt,ty1,ty2) \
		case FIELDTYPE_SF##ty1: \
			GLUNIFORM##ttt##FV(shaderVariable, 1, myField->value.sf##ty2.c); \
		break; 

#define SF_DOUBLES_TO_SHADER(ttt,ty1,ty2) \
		case FIELDTYPE_SF##ty1: {float val[4]; int i; \
			for (i=0; i<ttt; i++) { val[i] = (float) (myField->value.sf##ty2.c[i]); } \
				GLUNIFORM##ttt##FV(shaderVariable, 1, val); \
		break; }

#define SF_FLOAT_TO_SHADER(ty1,ty2) \
		case FIELDTYPE_SF##ty1: \
			GLUNIFORM1F(shaderVariable, myField->value.sf##ty2); \
		break; 

#define SF_DOUBLE_TO_SHADER(ty1,ty2) \
		case FIELDTYPE_SF##ty1: {float val = myField->value.sf##ty2; \
			GLUNIFORM1F(shaderVariable, val); \
		break; }

#define MF_FLOATS_TO_SHADER(ttt,ty1,ty2) \
		case FIELDTYPE_MF##ty1: \
			GLUNIFORM##ttt##FV(shaderVariable, myField->value.mf##ty2.n, (float *)myField->value.mf##ty2.p); \
		break; 

#define SF_INTS_TO_SHADER(ty1,ty2) \
		case FIELDTYPE_SF##ty1: \
			GLUNIFORM1I(shaderVariable, myField->value.sf##ty2);  \
			break;

#define MF_INTS_TO_SHADER(ty1,ty2) \
		case FIELDTYPE_MF##ty1: \
			GLUNIFORM1IV(shaderVariable, (GLsizei) myField->value.mf##ty2.n, (const GLint *)myField->value.mf##ty2.p); \
			break;


		SF_FLOAT_TO_SHADER(Float,float)
		SF_DOUBLE_TO_SHADER(Double,float)
		SF_DOUBLE_TO_SHADER(Time,float)

		SF_FLOATS_TO_SHADER(2,Vec2f,vec2f)
		SF_FLOATS_TO_SHADER(3,Vec3f,vec3f)
		SF_FLOATS_TO_SHADER(3,Color,color)
		SF_FLOATS_TO_SHADER(4,ColorRGBA,colorrgba)
		SF_FLOATS_TO_SHADER(4,Rotation,rotation)
		SF_FLOATS_TO_SHADER(4,Vec4f,vec4f)
		SF_DOUBLES_TO_SHADER(2,Vec2d, vec2d)
		SF_DOUBLES_TO_SHADER(3,Vec3d, vec3d)
		SF_DOUBLES_TO_SHADER(4,Vec4d, vec4d)

		MF_FLOATS_TO_SHADER(1,Float,float)
		MF_FLOATS_TO_SHADER(2,Vec2f,vec2f)
		MF_FLOATS_TO_SHADER(3,Color,color)
		MF_FLOATS_TO_SHADER(3,Vec3f,vec3f)
		MF_FLOATS_TO_SHADER(4,ColorRGBA,colorrgba)
		MF_FLOATS_TO_SHADER(4,Rotation,rotation)

		SF_INTS_TO_SHADER(Bool,bool)
		SF_INTS_TO_SHADER(Int32,int32)
		MF_INTS_TO_SHADER(Bool,bool)
		MF_INTS_TO_SHADER(Int32,int32)


		//MF_FLOATS_TO_SHADER(4,Vec4f,vec4f)

		//SF_FLOAT_TO_SHADER(9,Matrix3f, matrix3f)
		//SF_FLOAT_TO_SHADER(16,Matrix4f, matrix4f)

		case FIELDTYPE_SFNode:
		case FIELDTYPE_SFImage:
		case FIELDTYPE_FreeWRLPTR:
		case FIELDTYPE_SFString:
		case FIELDTYPE_SFMatrix3d:
		case FIELDTYPE_SFMatrix4d:


		case FIELDTYPE_MFNode:
		case FIELDTYPE_MFTime:
		case FIELDTYPE_MFString:
		case FIELDTYPE_MFDouble:
		case FIELDTYPE_MFVec3d:
		case FIELDTYPE_MFMatrix3f:
		case FIELDTYPE_MFMatrix3d:
		case FIELDTYPE_MFMatrix4f:
		case FIELDTYPE_MFMatrix4d:
		case FIELDTYPE_MFVec2d:
		case FIELDTYPE_MFVec4d:
		default : printf ("can not send a shader variable of %s yet\n",stringFieldtypeType(fieldDecl_getType(myField->fieldDecl)));
	}
}

/* Routing - sending a value to a shader - we get passed the routing table entry number */
void getField_ToShader(int num) {
	struct Shader_Script *myObj;
	unsigned int to_counter;
	int fromFieldID;
	size_t i;
	GLfloat* sourceData;
	GLuint currentShader;	


	/* go through each destination for this node */
	for (to_counter = 0; to_counter < CRoutes[num].tonode_count; to_counter++) {
		CRnodeStruct *to_ptr = NULL;

		to_ptr = &(CRoutes[num].tonodes[to_counter]);
		fromFieldID = to_ptr->foffset;
		/* printf ("getField_ToShader, num %d, foffset %d\n",num,fromFieldID);  */

		switch (to_ptr->routeToNode->_nodeType) {
		case NODE_ComposedShader:
			myObj = (struct Shader_Script *)(X3D_COMPOSEDSHADER(to_ptr->routeToNode)->__shaderObj);
			break;
		case NODE_PackagedShader:
			myObj = (struct Shader_Script *)(X3D_PACKAGEDSHADER(to_ptr->routeToNode)->__shaderObj);
			break;
		case NODE_ShaderProgram: 
			myObj = (struct Shader_Script *)(X3D_SHADERPROGRAM(to_ptr->routeToNode)->__shaderObj);
			break;
		default: {
			ConsoleMessage ("getField_ToShader, unhandled type??");
			return;
			}

		}
		/* we have the struct Shader_Script; go through the fields and find the correct one */

/*
	printf ("Shader_Script has node of %u ",myObj->ShaderScriptNode);
	printf ("of type %s ",stringNodeType(myObj->ShaderScriptNode->_nodeType));
	printf ("and has %d as a vector size ",vector_size(myObj->fields));
	if (myObj->loaded) printf ("locked and loaded "); else printf ("needs loading, I guess ");
	printf ("\n");
*/

	/* is there any fields? */
	if (myObj == NULL) return;

	/* this script should be loaded... if not, wait until it is */
	if (!myObj->loaded) {
		/* ConsoleMessage ("ShaderProgram should be loaded, hmmm"); */
		return;
	}

	/* initialize this one */
	currentShader = 0;

	switch (to_ptr->routeToNode->_nodeType) {
	case NODE_ComposedShader:
		currentShader = (GLuint) X3D_COMPOSEDSHADER(to_ptr->routeToNode)->__shaderIDS.p[0];
		break;
	case NODE_PackagedShader:
		ConsoleMessage ("do not know how to route to a PackagedShader yet");
		return;
		break;
	case NODE_ShaderProgram: 
		/* printf ("ShaderProgram- the parent ProgramShader holds the ids\n"); */
		if (X3D_SHADERPROGRAM(to_ptr->routeToNode)->_nparents > 0) {
			/* assume only one parent here? */
			currentShader = (GLuint) X3D_PROGRAMSHADER(
				X3D_SHADERPROGRAM(to_ptr->routeToNode)->_parents[0])->__shaderIDS.p[0];
		} else {
			printf ("no parents for routed ShaderProgram\n");
		}

		break;
	}


	/* printf ("going through fields.... have %d fields\n",vector_size(myObj->fields)); */
	for(i=0; i!=vector_size(myObj->fields); ++i) {
		GLint shaderVariable;
		struct ScriptFieldDecl* curField;
		struct FieldDecl * myf;

		/* initialization */
		curField = vector_get(struct ScriptFieldDecl*, myObj->fields, i);
		myf = curField->fieldDecl;
		shaderVariable = fieldDecl_getshaderVariableID(myf);

		
		
		/*printf ("curField %d name %d type %d ",i,
			fieldDecl_getIndexName(myf), fieldDecl_getType(myf));
		printf ("fieldDecl mode %d (%s) type %d (%s) name %d\n",
				fieldDecl_getAccessType(myf),
			stringPROTOKeywordType(fieldDecl_getAccessType(myf)), 
				fieldDecl_getType(myf), stringFieldtypeType(fieldDecl_getType(myf)),
				fieldDecl_getIndexName(myf));
		printf ("comparing fromFieldID %d and name %d\n",fromFieldID, fieldDecl_getIndexName(myf));
			printf ("	types %d, %d\n",JSparamnames[fromFieldID].type,fieldDecl_getType(myf));
			printf ("	shader ascii name is %s\n",fieldDecl_getShaderScriptName(curField->fieldDecl));
		*/
		



		if (fromFieldID == fieldDecl_getShaderScriptIndex(myf)) {
			/*
			printf ("	field match, %d==%d\n",fromFieldID, fieldDecl_getIndexName(myf));
			printf ("	types %d, %d\n",JSparamnames[fromFieldID].type,fieldDecl_getType(myf));
			printf ("	shaderVariableID is %d\n",fieldDecl_getShaderVariableID(myf));
			*/
		

		/* ok, here we have the Shader_Script, the field offset, and the entry */

		sourceData = offsetPointer_deref(GLfloat *,CRoutes[num].routeFromNode, CRoutes[num].fnptr);
		
		/* turn the selected shader program ON */
		USE_SHADER(currentShader);

#define ROUTE_SF_FLOAT_TO_SHADER(ty1) \
                case FIELDTYPE_SF##ty1: \
                        GLUNIFORM1F(shaderVariable, *((float*)sourceData)); \
                break;

#define ROUTE_SF_DOUBLE_TO_SHADER(ty1) \
                case FIELDTYPE_SF##ty1: {float val = (float) *((double *)sourceData); \
                        GLUNIFORM1F(shaderVariable, val); \
                break; }

#define ROUTE_SF_INTS_TO_SHADER(ty1) \
                case FIELDTYPE_SF##ty1: \
                        GLUNIFORM1I(shaderVariable, *((int*)sourceData));  \
                        break;

#define ROUTE_SF_FLOATS_TO_SHADER(ttt,ty1) \
                case FIELDTYPE_SF##ty1: \
                        GLUNIFORM##ttt##FV(shaderVariable, 1, (float*)sourceData); \
                break;

#define ROUTE_SF_DOUBLES_TO_SHADER(ttt,ty1) \
                case FIELDTYPE_SF##ty1: {float val[4]; int i; double *fp = (double*)sourceData; \
                        for (i=0; i<ttt; i++) { val[i] = (float) (*fp); fp++; } \
                                GLUNIFORM##ttt##FV(shaderVariable, 1, val); \
                break; }

#define ROUTE_MF_FLOATS_TO_SHADER(ttt,ty1) \
                case FIELDTYPE_MF##ty1: { struct Multi_##ty1 *sd = (struct Multi_##ty1*) sourceData; \
                        GLUNIFORM##ttt##FV(shaderVariable, sd->n, (const GLfloat *)sd->p); \
                break; }

#define ROUTE_MF_INTS_TO_SHADER(ttt,ty1) \
                case FIELDTYPE_MF##ty1: { struct Multi_##ty1 *sd = (struct Multi_##ty1*) sourceData; \
                        GLUNIFORM##ttt##IV(shaderVariable, sd->n, (const GLint *)sd->p); \
                break; }



		/* send in the correct parameters */
		switch (JSparamnames[fromFieldID].type) {
                ROUTE_SF_FLOAT_TO_SHADER(Float)
                ROUTE_SF_DOUBLE_TO_SHADER(Double)
                ROUTE_SF_DOUBLE_TO_SHADER(Time)
		ROUTE_SF_INTS_TO_SHADER(Bool)
		ROUTE_SF_INTS_TO_SHADER(Int32)

                ROUTE_SF_FLOATS_TO_SHADER(2,Vec2f)
                ROUTE_SF_FLOATS_TO_SHADER(3,Vec3f)
                ROUTE_SF_FLOATS_TO_SHADER(3,Color)
                ROUTE_SF_FLOATS_TO_SHADER(4,ColorRGBA)
                ROUTE_SF_FLOATS_TO_SHADER(4,Rotation)
                ROUTE_SF_FLOATS_TO_SHADER(4,Vec4f)
                ROUTE_SF_DOUBLES_TO_SHADER(2,Vec2d)
                ROUTE_SF_DOUBLES_TO_SHADER(3,Vec3d)
                ROUTE_SF_DOUBLES_TO_SHADER(4,Vec4d)

                ROUTE_MF_FLOATS_TO_SHADER(1,Float)
                ROUTE_MF_FLOATS_TO_SHADER(2,Vec2f)
                ROUTE_MF_FLOATS_TO_SHADER(3,Color)
                ROUTE_MF_FLOATS_TO_SHADER(3,Vec3f)
                ROUTE_MF_FLOATS_TO_SHADER(4,ColorRGBA)
                ROUTE_MF_FLOATS_TO_SHADER(4,Rotation)

                ROUTE_MF_INTS_TO_SHADER(1,Bool)
                ROUTE_MF_INTS_TO_SHADER(1,Int32)



			case FIELDTYPE_SFNode:
			case FIELDTYPE_MFNode:
			case FIELDTYPE_MFTime:
			case FIELDTYPE_SFString:
			case FIELDTYPE_MFString:
			case FIELDTYPE_SFImage:
			case FIELDTYPE_FreeWRLPTR:
			case FIELDTYPE_MFVec3d:
			case FIELDTYPE_MFDouble:
			case FIELDTYPE_SFMatrix3f:
			case FIELDTYPE_MFMatrix3f:
			case FIELDTYPE_SFMatrix3d:
			case FIELDTYPE_MFMatrix3d:
			case FIELDTYPE_SFMatrix4f:
			case FIELDTYPE_MFMatrix4f:
			case FIELDTYPE_SFMatrix4d:
			case FIELDTYPE_MFMatrix4d:
			case FIELDTYPE_MFVec2d:
			case FIELDTYPE_MFVec4d:
				ConsoleMessage ("shader field type %s not routable yet",stringFieldtypeType(JSparamnames[fromFieldID].type));
				break;
			default: {
				ConsoleMessage ("shader field type %s not routable yet",stringFieldtypeType(JSparamnames[fromFieldID].type));
			}


			}

			/* turn the shader OFF */
			USE_SHADER(0);
		}
	}
	}
}

/* send fields to a shader; expect either a ShaderProgram, or a ComposedShader */
static void send_fieldToShader (GLuint myShader, struct X3D_Node *node) {
	size_t i;

	struct Shader_Script* me = NULL;
	if (node->_nodeType==NODE_ShaderProgram) me = (struct Shader_Script *) X3D_SHADERPROGRAM(node)->__shaderObj;
	else if (node->_nodeType == NODE_ComposedShader) me = (struct Shader_Script *) X3D_COMPOSEDSHADER(node)->__shaderObj;
	else {
		printf ("send_fieldToShader, expected a ShaderProgram or ComposedShader, got %s\n",
			stringNodeType(node->_nodeType));
		return;
	}

	#ifdef SHADERVERBOSE
	printf ("Shader_Script has node of %u ",me->ShaderScriptNode);
	printf ("of type %s ",stringNodeType(me->ShaderScriptNode->_nodeType));
	printf ("and has %d as a vector size ",vector_size(me->fields));
	if (me->loaded) printf ("locked and loaded "); else printf ("needs loading, I guess ");
	printf ("\n");
	#endif

	/* lets look for, and tie in, textures */
	/* we make X3D_Texture0 = 0; X3D_Texture1=1, etc */
	for (i=0; i<MAX_MULTITEXTURE; i++) {
		char myShaderTextureName[200];
		GLint myVar;

		sprintf (myShaderTextureName,"X3D_Texture%d",(int) i);
		myVar = GET_UNIFORM(myShader,myShaderTextureName);
		if (myVar != INT_ID_UNDEFINED) {
			/* printf ("for texture %s, we got %d\n", myShaderTextureName,myVar); */
			GLUNIFORM1I(myVar,(int) i);
		}
	}

	/* is there any fields? */
	if (me == NULL) return;

	/* this script should NOT be loaded... */
	if (me->loaded) ConsoleMessage ("ShaderProgram is flagged as being loaded, hmmm");


	for(i=0; i!=vector_size(me->fields); ++i) {
		GLint myVar;
		struct ScriptFieldDecl* curField;
		struct FieldDecl * myf;

		/* initialization */
		myVar = -1;
		curField = vector_get(struct ScriptFieldDecl*, me->fields, i);
		myf = curField->fieldDecl;

		#ifdef SHADERVERBOSE
		DEBUG_SHADER("curField %d contains :%s: ",i,curField->ASCIIvalue);
		DEBUG_SHADER("fieldDecl mode %d (%s) type %d (%s) name %d\n",myf->PKWmode, 
			stringPROTOKeywordType(myf->PKWmode), myf->fieldType, stringFieldtypeType(myf->fieldType),myf->lexerNameIndex);
		#endif

		/* ask the shader for its handle for this variable */

		/* try Uniform  */
		myVar = GET_UNIFORM(myShader,fieldDecl_getShaderScriptName(curField->fieldDecl));
		if (myVar == INT_ID_UNDEFINED) {
			if (GET_ATTRIB(myShader,fieldDecl_getShaderScriptName(curField->fieldDecl)) != INT_ID_UNDEFINED)
			ConsoleMessage ("Shader variable :%s: is declared as an attribute; we can not do much with this",fieldDecl_getShaderScriptName(curField->fieldDecl));
			else
			ConsoleMessage ("Shader variable :%s: is either not declared or not used in the shader program",fieldDecl_getShaderScriptName(curField->fieldDecl));
		}

		#ifdef SHADERVERBOSE
		DEBUG_SHADER("trying to get ID for :%s:\n",curField->ASCIIvalue);
		#endif

		/* do the types of the field variable, and the shader variable match? */
		shader_checkType(myf,myShader,myVar);

			
		/* save the variable object for this variable */
		fieldDecl_setshaderVariableID(myf,myVar);

		if ((fieldDecl_getAccessType(myf)==PKW_initializeOnly) || (fieldDecl_getAccessType(myf)==PKW_inputOutput)) {
			#ifdef SHADERVERBOSE
			DEBUG_SHADER("initializing Shader %d containing :%s:\n",myShader,curField->ASCIIvalue);
			#endif

			sendValueToShader(curField);
		}

	}

	/* done the loading of this shader part */
	me->loaded = TRUE;
}


/* on load of shader, send along any initial field values in the X3D file to the Shader */
/* we can have shaders like:

ComposedShader { 
	field SFFloat blubber 0.5
	field SFVec3f decis 0.96 0.44 0.22 
	language "GLSL" parts [ 
		ShaderPart { type "VERTEX" url "toon.vs" } 
		ShaderPart { type "FRAGMENT" url "toon.fs" } 
	] }

ProgramShader { 
	language  "GLSL" programs   [ 
		ShaderProgram { 
			field SFFloat blubber 0.5
			type "VERTEX" url "toon.vs" } 
		ShaderProgram { 
			field SFVec3f decis 0.96 0.44 0.22 
			type "FRAGMENT" url "toon.fs" } 
	] }

Note the differing location of the fields...

*/


static void sendInitialFieldsToShader(struct X3D_Node * node) {
	int i;
	GLuint myShader;

	switch (node->_nodeType) {
		case NODE_ProgramShader: {
			/* anything to do here? */ 
			if ((X3D_PROGRAMSHADER(node)->__shaderIDS.n) >=1) {
				myShader = X3D_PROGRAMSHADER(node)->__shaderIDS.p[0];
				for (i=0; i<X3D_PROGRAMSHADER(node)->programs.n; i++) {
					#ifdef SHADERVERBOSE
					printf ("ProgramShader, activate %d isSelected %d isValid %d TRUE %d FALSE %d\n",
						X3D_PROGRAMSHADER(node)->activate,X3D_PROGRAMSHADER(node)->isSelected,
						X3D_PROGRAMSHADER(node)->isValid, TRUE, FALSE);
					printf ("runningShader %d, myShader %d\n",appearanceProperties.currentShader, X3D_PROGRAMSHADER(node)->__shaderIDS.p[0]);
					#endif

					struct X3D_ShaderProgram *part = X3D_SHADERPROGRAM(X3D_PROGRAMSHADER(node)->programs.p[i]);

					#ifdef SHADERVERBOSE
					printf ("sendInitial, have part %d\n",part);
					#endif

					send_fieldToShader(myShader, X3D_NODE(part));
				}
			}
			X3D_PROGRAMSHADER(node)->__initialized = TRUE;
			break;
		}


		case NODE_ComposedShader: {
			/* anything to do here? */ 
			if ((X3D_COMPOSEDSHADER(node)->__shaderIDS.n) >=1) {
				myShader = X3D_COMPOSEDSHADER(node)->__shaderIDS.p[0];
				send_fieldToShader(myShader, X3D_NODE(node));
			}
			X3D_COMPOSEDSHADER(node)->__initialized = TRUE;
			break;
		}
	}
}

/*********************************************************************/

void compile_ComposedShader (struct X3D_ComposedShader *node) {
	DEBUG_SHADER("called compile_ComposedShader(%p)\n",(void *)node);
	printf("called compile_ComposedShader(%p)\n",(void *)node);
	#ifdef HAVE_SHADERS
	{
		/* an array of text pointers, should contain shader source */
		GLchar **vertShaderSource;
		GLchar **fragShaderSource;
		int i;
		GLuint myProgram;
		/* do we have anything to compile? */
		int haveVertShaderText; 
		int haveFragShaderText; 

		/* can we do shaders at runtime? */
		/* NOTE - need to do this first because no shaders = no CREATE_PROGRAM */
		CHECK_SHADERS

		/* initialization */
		haveVertShaderText = FALSE;
		haveFragShaderText = FALSE;
		myProgram = CREATE_PROGRAM;

		vertShaderSource = MALLOC(GLchar **, sizeof(GLchar*) * node->parts.n); 
		fragShaderSource = MALLOC(GLchar **, sizeof(GLchar*) * node->parts.n);
	
		/* set this up... set it to FALSE if there are problems */
		node->isValid = TRUE;
	
		/* we support only GLSL here */
		SUPPORT_GLSL_ONLY
		
		/* ok so far, go through the parts */
		COMPILE_SHADER_PARTS(ShaderPart,parts)
	
		/* compile shader, if things are ok to here */
		COMPILE_IF_VALID(parts)
		LINK_IF_VALID

		MARK_NODE_COMPILED
	}
	#endif	
}
void compile_ProgramShader (struct X3D_ProgramShader *node) {
	#ifdef HAVE_SHADERS
		/* an array of text pointers, should contain shader source */
		GLchar **vertShaderSource;
		GLchar **fragShaderSource;
		int i;
		GLuint myProgram;
		/* do we have anything to compile? */
		int haveVertShaderText; 
		int haveFragShaderText; 

		/* can we do shaders at runtime? */
		/* NOTE - need to do this first because no shaders = no CREATE_PROGRAM */
		CHECK_SHADERS

		/* initialization */
		haveVertShaderText = FALSE;
		haveFragShaderText = FALSE;
		myProgram = CREATE_PROGRAM;
	
		vertShaderSource = MALLOC(GLchar **, sizeof(GLchar*) * node->programs.n); 
		fragShaderSource = MALLOC(GLchar **, sizeof(GLchar*) * node->programs.n);
	
		/* set this up... set it to FALSE if there are problems */
		node->isValid = TRUE;
	
		/* we support only GLSL here */
		SUPPORT_GLSL_ONLY
			
		/* ok so far, go through the programs */
		COMPILE_SHADER_PARTS(ShaderProgram,programs)
	
		/* compile shader, if things are ok to here */
		COMPILE_IF_VALID(programs)
		LINK_IF_VALID

		MARK_NODE_COMPILED
	#endif	
}
void compile_PackagedShader (struct X3D_PackagedShader *node) {
	#ifdef HAVE_SHADERS
		ConsoleMessage ("found PackagedShader, do not support this structure, as we support only GLSL");
		node->isValid = FALSE;
		MARK_NODE_COMPILED
	#endif
}


/*****************************************************************/
void render_ComposedShader (struct X3D_ComposedShader *node) {
	#ifdef HAVE_SHADERS
		DEBUG_RENDER("render_ComposedShader: calling COMPILE_IF_REQUIRED\n");
		COMPILE_IF_REQUIRED
		DEBUG_RENDER("render_ComposedShader: calling RUN_IF_VALID\n");
		RUN_IF_VALID
	#endif
}
void render_PackagedShader (struct X3D_PackagedShader *node) {
	#ifdef HAVE_SHADERS
		COMPILE_IF_REQUIRED
	#endif
}
void render_ProgramShader (struct X3D_ProgramShader *node) {
	#ifdef HAVE_SHADERS
		COMPILE_IF_REQUIRED
		RUN_IF_VALID
	#endif
}
