/*
=INSERT_TEMPLATE_HERE=

$Id: Component_ProgrammableShaders.c,v 1.6 2009/05/15 19:42:22 crc_canada Exp $

X3D Programmable Shaders Component

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../lib/scenegraph/Vector.h"

#include "../world_script/CScripts.h"
#include "../vrml_parser/CFieldDecls.h"


/* which shader is running?? */
GLuint globalCurrentShader = 0;
static void sendInitialFieldsToShader(struct X3D_Node *);

#define MAX_INFO_LOG_SIZE 512
/* we do support older versions of shaders; but not all info logs are printed if we
   have OpenGL prior to 2.0 */


/* Versions 1.5 and above have shaders */
#ifdef GL_VERSION_2_0
	#define HAVE_SHADERS
	#define VERTEX_SHADER GL_VERTEX_SHADER
	#define FRAGMENT_SHADER GL_FRAGMENT_SHADER
	#define SHADER_SOURCE glShaderSource
	#define COMPILE_SHADER glCompileShader
	#define CREATE_PROGRAM glCreateProgram();
	#define ATTACH_SHADER glAttachShader
	#define LINK_SHADER glLinkProgram
	#define USE_SHADER glUseProgram
	#define CREATE_SHADER glCreateShader
	#define GET_SHADER_INFO glGetShaderiv
	#define LINK_STATUS GL_LINK_STATUS
	#define COMPILE_STATUS GL_COMPILE_STATUS
	#define GET_UNIFORM(aaa,bbb) glGetUniformLocation(aaa,bbb)
	#define GET_ATTRIB(aaa,bbb) glGetAttribLocation(aaa,bbb)
	#define GLUNIFORM1FV glUniform1fv
	#define GLUNIFORM2FV glUniform2fv
	#define GLUNIFORM3FV glUniform3fv
	#define GLUNIFORM4FV glUniform4fv
	#define GLATTRIB1FV glVertexAttrib1fv
	#define GLATTRIB2FV glVertexAttrib2fv
	#define GLATTRIB3FV glVertexAttrib3fv
	#define GLATTRIB4FV glVertexAttrib4fv
#else
#ifdef GL_VERSION_1_5
	#define HAVE_SHADERS
	#define VERTEX_SHADER GL_VERTEX_SHADER_ARB
	#define FRAGMENT_SHADER GL_FRAGMENT_SHADER_ARB
	#define SHADER_SOURCE glShaderSourceARB
	#define COMPILE_SHADER glCompileShaderARB
	#define CREATE_PROGRAM glCreateProgramObjectARB();
	#define ATTACH_SHADER glAttachObjectARB
	#define LINK_SHADER glLinkProgramARB
	#define USE_SHADER  glUseProgramObjectARB
	#define CREATE_SHADER glCreateShaderObjectARB
	#define GET_SHADER_INFO glGetObjectParameterivARB
	#define LINK_STATUS GL_OBJECT_LINK_STATUS_ARB
	#define COMPILE_STATUS GL_OBJECT_COMPILE_STATUS_ARB
	#define GET_UNIFORM(aaa,bbb) glGetUniformLocationARB(aaa,bbb)
	#define GET_ATTRIB(aaa,bbb) glGetAttribLocationARB(aaa,bbb)
	#define GLUNIFORM1FV glUniform1fARB
	#define GLUNIFORM2FV glUniform2fARB
	#define GLUNIFORM3FV glUniform3fARB
	#define GLUNIFORM4FV glUniform4fARB
	#define GLATTRIB1FV glVertexAttrib1fARB
	#define GLATTRIB2FV glVertexAttrib2fARB
	#define GLATTRIB3FV glVertexAttrib3fARB
	#define GLATTRIB4FV glVertexAttrib4fARB
#endif
#endif

#define SUPPORT_GLSL_ONLY \
	if (strcmp(node->language->strptr,"GLSL")) { \
		ConsoleMessage ("Shaders: support only GLSL shading language, got :%s:, skipping...",node->language->strptr); \
		node->isValid = FALSE; \
	}

#define RUN_IF_VALID \
		if (node->isValid) { \
			if (node->__shaderIDS.n != 0) { \
				globalCurrentShader = (GLuint) node->__shaderIDS.p[0]; \
				USE_SHADER(globalCurrentShader); \
				if (!node->__initialized) sendInitialFieldsToShader(X3D_NODE(node)); \
			} \
		}

#define CHECK_SHADERS \
	if (!shadersChecked) checkShaders(); \
	if (!haveShaders) { \
		ConsoleMessage ("have an X3D program with shaders, but no shader support on this computer"); \
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
						FREE_IF_NZ(*cptr); }\
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

#ifdef OPENGL_VERSION_2_0
	#define LINK_IF_VALID \
		if (node->isValid) { \
			/* link the shader programs together */ \
			LINK_SHADER(myProgram); \
			glGetProgramiv(myProgram, GL_LINK_STATUS, &success); \
			if (!success) { \
				GLchar infoLog[MAX_INFO_LOG_SIZE]; \
				glGetProgramInfoLog(myFragmentShader, MAX_INFO_LOG_SIZE, NULL, infoLog); \
				printf ("problem with Shader Program link: %s\n",infoLog); \
				node->isValid = FALSE; \
			} \
			/* does the program get a thumbs up? */	 \
			glValidateProgram (myProgram); \
			glGetProgramiv(myProgram, GL_VALIDATE_STATUS, &success); \
			if (!success) { \
				GLchar infoLog[MAX_INFO_LOG_SIZE]; \
				glGetProgramInfoLog(myFragmentShader, MAX_INFO_LOG_SIZE, NULL, infoLog); \
				printf ("problem with Shader Program Validate: %s\n",infoLog); \
				node->isValid = FALSE; \
			} \
			if (node->__shaderIDS.n == 0) { \
				node->__shaderIDS.n = 1; \
				node->__shaderIDS.p = MALLOC(sizeof (GLuint)); \
				node->__shaderIDS.p[0] = (void *)myProgram; \
			} \
		}
#else
	#define LINK_IF_VALID \
		if (node->isValid) { \
			/* link the shader programs together */ \
			LINK_SHADER(myProgram); \
			if (node->__shaderIDS.n == 0) { \
				node->__shaderIDS.n = 1; \
				node->__shaderIDS.p = MALLOC(sizeof (GLuint)); \
				node->__shaderIDS.p[0] = (void *)myProgram; \
			} \
		}
#endif


/* fieldDecl_getshaderVariableID(myf), fieldDecl_getValue(myf)); */
static void sendValueToShader(struct ScriptFieldDecl* myField) {
	GLint shaderVariable = fieldDecl_getshaderVariableID(myField->fieldDecl);
	int isUniform = fieldDecl_isshaderVariableUniform(myField->fieldDecl);

	#ifdef SHADERVERBOSE
	printf ("sendValueToShader...");
	printf ("shaderVariableID is %d\n",shaderVariable);
	#endif

	/* either not defined in the shader, OR not used in the shader so it is stripped by glsl compiler */
	if (shaderVariable == INT_ID_UNDEFINED) return;

	switch (fieldDecl_getType(myField->fieldDecl)) {

		case FIELDTYPE_SFVec3f:
			if (isUniform)
				GLUNIFORM3FV(shaderVariable, 1, myField->value.sfvec3f.c);
			else
				GLATTRIB3FV(shaderVariable, myField->value.sfvec3f.c);
		break;

		case FIELDTYPE_SFFloat:
		case FIELDTYPE_MFFloat:
		case FIELDTYPE_SFRotation:
		case FIELDTYPE_MFRotation:
		case FIELDTYPE_MFVec3f:
		case FIELDTYPE_SFBool:
		case FIELDTYPE_MFBool:
		case FIELDTYPE_SFInt32:
		case FIELDTYPE_MFInt32:
		case FIELDTYPE_SFNode:
		case FIELDTYPE_MFNode:
		case FIELDTYPE_SFColor:
		case FIELDTYPE_MFColor:
		case FIELDTYPE_SFColorRGBA:
		case FIELDTYPE_MFColorRGBA:
		case FIELDTYPE_SFTime:
		case FIELDTYPE_MFTime:
		case FIELDTYPE_SFString:
		case FIELDTYPE_MFString:
		case FIELDTYPE_SFVec2f:
		case FIELDTYPE_MFVec2f:
		case FIELDTYPE_SFImage:
		case FIELDTYPE_FreeWRLPTR:
		case FIELDTYPE_SFVec3d:
		case FIELDTYPE_MFVec3d:
		case FIELDTYPE_SFDouble:
		case FIELDTYPE_MFDouble:
		case FIELDTYPE_SFMatrix3f:
		case FIELDTYPE_MFMatrix3f:
		case FIELDTYPE_SFMatrix3d:
		case FIELDTYPE_MFMatrix3d:
		case FIELDTYPE_SFMatrix4f:
		case FIELDTYPE_MFMatrix4f:
		case FIELDTYPE_SFMatrix4d:
		case FIELDTYPE_MFMatrix4d:
		case FIELDTYPE_SFVec2d:
		case FIELDTYPE_MFVec2d:
		case FIELDTYPE_SFVec4f:
		case FIELDTYPE_MFVec4f:
		case FIELDTYPE_SFVec4d:
		case FIELDTYPE_MFVec4d:
		default : printf ("can not send a shader variable of %s yet\n",stringFieldtypeType(fieldDecl_getType(myField->fieldDecl)));
	}
}

/* this is specific for a ShaderProgram */
static void send_ProgramShaderPart (GLuint myShader, struct X3D_ShaderProgram *node) {
	size_t i;

	struct Shader_Script* me = NULL;
	me = (struct Shader_Script *) node->__shaderObj;

	#ifdef SHADERVERBOSE
	printf ("send_ProgramShaderPart, type %s\n",node->type->strptr);
	printf ("Shader_Script has node of %u ",me->ShaderScriptNode);
	printf ("of type %s ",stringNodeType(me->ShaderScriptNode->_nodeType));
	printf ("and has %d as a vector size ",vector_size(me->fields));
	if (me->loaded) printf ("locked and loaded "); else printf ("needs loading, I guess ");
	printf ("script handle %d\n",me->num);
	#endif

	/* this script should NOT be loaded... */
	if (me->loaded) ConsoleMessage ("ShaderProgram is flagged as being loaded, hmmm");


	for(i=0; i!=vector_size(me->fields); ++i) {
		GLint myVar = -1;

		struct ScriptFieldDecl* curField=vector_get(struct ScriptFieldDecl*, me->fields, i);
		struct FieldDecl * myf = curField->fieldDecl;

		#ifdef SHADERVERBOSE
		printf ("curField %d name %s type %s ",i,curField->name,curField->type);
		printf ("fieldDecl mode %d (%s) type %d (%s) name %d\n",myf->mode, 
			stringPROTOKeywordType(myf->mode), myf->type, stringFieldtypeType(myf->type),myf->name);
		#endif

		/* ask the shader for its handle for this variable */

		/* try Uniform variables first */
		myVar = GET_UNIFORM(myShader,curField->name);
		if (myVar == INT_ID_UNDEFINED) {
			myVar = GET_ATTRIB(myShader,curField->name);
			fieldDecl_setshaderVariableUniform(myf,FALSE);
		} else {
			fieldDecl_setshaderVariableUniform(myf,TRUE);
		}

		#ifdef SHADERVERBOSE
		printf ("trying to get ID for :%s:\n",curField->name);
		#endif

		fieldDecl_setshaderVariableID(myf,myVar);

		if ((myf->mode==PKW_initializeOnly) || (myf->mode==PKW_inputOutput)) {
			#ifdef SHADERVERBOSE
			printf ("initializing Shader %d variable %s, type %s\n",myShader, curField->name,stringFieldtypeType(myf->type)); */
			#endif

			sendValueToShader(curField);
		}

	}

	/* done the loading of this shader part */
	me->loaded = TRUE;
}

static void sendInitialFieldsToShader(struct X3D_Node * node) {
	struct Shader_Script* me = NULL;
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
					printf ("runningShader %d, myShader %d\n",globalCurrentShader, X3D_PROGRAMSHADER(node)->__shaderIDS.p[0]);
					#endif

					struct X3D_ShaderProgram *part = X3D_PROGRAMSHADER(node)->programs.p[i];

					#ifdef SHADERVERBOSE
					printf ("sendInitial, have part %d\n",part);
					#endif

					send_ProgramShaderPart(myShader, part);
				}
			}
			X3D_PROGRAMSHADER(node)->__initialized = TRUE;

			break;
		}


		case NODE_ComposedShader: me = (struct Shader_Script *) (X3D_COMPOSEDSHADER(node)->__shaderObj); 
			printf ("have to somehow fixup composed shader\n"); break;
	}
}


/* do we support shaders on runtime? */
static int shadersChecked = FALSE;
static int haveShaders = FALSE;

/* can we handle shaders at runtime? We may have had a binary distro move onto an older machine */
static void checkShaders (void) {
	char *glExtensions;

        glExtensions = (char *)glGetString(GL_EXTENSIONS);
        if ((strstr (glExtensions, "GL_ARB_fragment_shader")!=0)  &&
                (strstr (glExtensions,"GL_ARB_vertex_shader")!=0)) {
			haveShaders = TRUE;
	}
	shadersChecked = TRUE;
}


/*********************************************************************/

void compile_ComposedShader (struct X3D_ComposedShader *node) {
	#ifdef HAVE_SHADERS
		/* an array of text pointers, should contain shader source */
		GLchar **vertShaderSource;
		GLchar **fragShaderSource;
		int i;
		GLuint myProgram = CREATE_PROGRAM;
		/* do we have anything to compile? */
		int haveVertShaderText = FALSE; 
		int haveFragShaderText = FALSE; 

		/* can we do shaders at runtime? */
		CHECK_SHADERS
		vertShaderSource = MALLOC(sizeof(GLchar*) * node->parts.n); 
		fragShaderSource = MALLOC(sizeof(GLchar*) * node->parts.n);
	
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
	#endif	
}
void compile_ProgramShader (struct X3D_ProgramShader *node) {
	#ifdef HAVE_SHADERS
		/* an array of text pointers, should contain shader source */
		GLchar **vertShaderSource;
		GLchar **fragShaderSource;
		int i;
		GLuint myProgram = CREATE_PROGRAM;
		/* do we have anything to compile? */
		int haveVertShaderText = FALSE; 
		int haveFragShaderText = FALSE; 
	
		CHECK_SHADERS
		vertShaderSource = MALLOC(sizeof(GLchar*) * node->programs.n); 
		fragShaderSource = MALLOC(sizeof(GLchar*) * node->programs.n);
	
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
		COMPILE_IF_REQUIRED
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
