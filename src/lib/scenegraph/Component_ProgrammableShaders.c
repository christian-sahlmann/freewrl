/*
=INSERT_TEMPLATE_HERE=

$Id: Component_ProgrammableShaders.c,v 1.3 2009/02/11 15:12:55 istakenv Exp $

X3D Programmable Shaders Component

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h" /* point_XYZ */
#include "../main/headers.h"


/* which shader is running?? */
GLuint globalCurrentShader = 0;

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
						char *myText = NULL; \
						char filename[1000]; \
						 \
 \
						if (getValidFileFromUrl (filename, prog->__parenturl->strptr, &prog->url, NULL, &removeIt) ) { \
							myText = readInputString(filename); \
							if (removeIt) UNLINK(filename); \
						} else { \
							ConsoleMessage ("error reading url for :%s:",stringNodeType(NODE_##myNodeType)); \
							myText = ""; \
						} \
						/* assign this text to VERTEX or FRAGMENT buffers */ \
						if (!strcmp(prog->type->strptr,"VERTEX")) { \
							vertShaderSource[i] = myText; \
							haveVertShaderText = TRUE; \
						} else { \
							fragShaderSource[i] = myText; \
							haveFragShaderText = TRUE; \
						} \
						/* printf ("Shader text %s\n",myText); */ \
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
		int removeIt = FALSE;

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
		int removeIt = FALSE;
	
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
