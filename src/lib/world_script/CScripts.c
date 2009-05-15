/*
=INSERT_TEMPLATE_HERE=

$Id: CScripts.c,v 1.10 2009/05/15 19:42:22 crc_canada Exp $

???

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>


#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../vrml_parser/CParseLexer.h"
#include "../main/Snapshot.h"
#include "../scenegraph/Vector.h"
#include "../scenegraph/Collision.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"
#include "../input/SensInterps.h"
#include "../x3d_parser/Bindable.h"
/* #include "../input/EAIheaders.h" */

#include "CScripts.h"
#include "JScript.h"
#include "jsUtils.h"
#include "jsNative.h"
#include "jsVRMLClasses.h"


#undef CPARSERVERBOSE

/* JavaScript-"protocols" */
const char* JS_PROTOCOLS[]={
 "shader",
 "javascript",
 "ecmascript",
 "vrmlscript"};

/* ************************************************************************** */
/* ****************************** ScriptFieldDecl *************************** */
/* ************************************************************************** */

/* Constructor and destructor */
/* ************************** */

struct ScriptFieldDecl* newScriptFieldDecl(struct VRMLLexer* me, indexT mod, indexT type, indexT name)
{
 struct ScriptFieldDecl* ret=MALLOC(sizeof(struct ScriptFieldDecl));

 ASSERT(ret);

 ASSERT(mod!=PKW_inputOutput);

	/* shaderID will get set when shader is activiated */
 ret->fieldDecl=newFieldDecl(mod, type, name, -1, FALSE);
 ASSERT(ret->fieldDecl);

 /* Stringify */
 ret->name=fieldDecl_getStringName(me, ret->fieldDecl);
 ret->type=FIELDTYPES[type];

 /* Field's value not yet initialized! */
 ret->valueSet=(mod!=PKW_initializeOnly);
 /* value is set later on */

 #ifdef CPARSERVERBOSE
 printf ("newScriptFieldDecl, returning name %s, type %s, mode %s\n",ret->name, ret->type,PROTOKEYWORDS[ret->fieldDecl->mode]); 
 #endif

 return ret;
}

/* Create a new ScriptFieldInstanceInfo structure to hold information about script fields that are destinations for IS statements in PROTOs */
struct ScriptFieldInstanceInfo* newScriptFieldInstanceInfo(struct ScriptFieldDecl* dec, struct Shader_Script* script) {
	struct ScriptFieldInstanceInfo* ret = MALLOC(sizeof(struct ScriptFieldInstanceInfo));
	
	ASSERT(ret);

	ret->decl = dec;
	ret->script = script;

	#ifdef CPARSERVERBOSE
	printf("creating new scriptfieldinstanceinfo with decl %p script %p\n", dec, script); 
	#endif

	return(ret);
}

/* Copy a ScriptFieldInstanceInfo structure to a new structure */
struct ScriptFieldInstanceInfo* scriptFieldInstanceInfo_copy(struct ScriptFieldInstanceInfo* me) {
	struct ScriptFieldInstanceInfo* ret = MALLOC(sizeof(struct ScriptFieldInstanceInfo));

	#ifdef CPARSERVERBOSE
	printf("copying instanceinfo %p (%p %p) to %p\n", me, me->decl, me->script, ret);
	#endif

	
	ASSERT(ret);

	ret->decl = me->decl;
	ret->script = me->script;

	return ret;
}

struct ScriptFieldDecl* scriptFieldDecl_copy(struct VRMLLexer* lex, struct ScriptFieldDecl* me) 
{
	struct ScriptFieldDecl* ret = MALLOC(sizeof (struct ScriptFieldDecl));
	ASSERT(ret);

	#ifdef CPARSERVERBOSE
	printf("copying script field decl %p to %p\n", me, ret);
	#endif


	ret->fieldDecl = fieldDecl_copy(me->fieldDecl);
	ASSERT(ret->fieldDecl);	

	ret->name = fieldDecl_getStringName(lex, ret->fieldDecl);
	ret->type = me->type;
	
	ret->valueSet=((ret->fieldDecl->mode)!=PKW_initializeOnly);
	return ret;
}

void deleteScriptFieldDecl(struct ScriptFieldDecl* me)
{
 deleteFieldDecl(me->fieldDecl);
 FREE_IF_NZ (me);
}

/* Other members */
/* ************* */

/* Sets script field value */
void scriptFieldDecl_setFieldValue(struct ScriptFieldDecl* me, union anyVrml v)
{
 ASSERT(me->fieldDecl->mode==PKW_initializeOnly); 
 /* ASSERT(!me->valueSet); */


 me->value=v;

 me->valueSet=TRUE;
}

/* Get "offset" data for routing */
int scriptFieldDecl_getRoutingOffset(struct ScriptFieldDecl* me)
{
 return JSparamIndex((char *)me->name, (char *)me->type);
}

/* Initialize JSField */
void scriptFieldDecl_jsFieldInit(struct ScriptFieldDecl* me, uintptr_t num) {
	#ifdef CPARSERVERBOSE
	printf ("scriptFieldDecl_jsFieldInit mode %d\n",me->fieldDecl->mode);
	#endif

	ASSERT(me->valueSet);
 	SaveScriptField(num, me->fieldDecl->mode, me->fieldDecl->type, me->name, me->value);
}

/* ************************************************************************** */
/* ********************************** Script ******************************** */
/* ************************************************************************** */

/* Constructor and destructor */
/* ************************** */

/* Next handle to be assinged */
static uintptr_t handleCnt=0;

uintptr_t nextScriptHandle (void) {uintptr_t retval; retval = handleCnt; handleCnt++; return retval;}

/* copy a Script node in a proto. */
struct X3D_Script * protoScript_copy (struct X3D_Script *me) {
	struct X3D_Script* ret;

	ret = createNewX3DNode(NODE_Script);
	ret->__parenturl = me->__parenturl;
	ret->directOutput = me->directOutput;
	ret->mustEvaluate = me->mustEvaluate;
	ret->url = me->url;
	ret->__scriptObj = me->__scriptObj;

	/* script handle gets updated in registerScriptInPROTO */
	/* ((struct Script *) (ret->__scriptObj))->num = nextScriptHandle(); */

	return ret;
	
}

/* on a reload, zero script counts */
void zeroScriptHandles (void) {handleCnt = 0;}

/* this can be a script, or a shader, take your pick */
struct Shader_Script* new_Shader_Script(struct X3D_Node *node) {
 	struct Shader_Script* ret=MALLOC(sizeof(struct Shader_Script));

 	ASSERT(ret);

	ret->loaded=FALSE;
	ret->fields=newVector(struct ScriptFieldDecl*, 4);
	ret->ShaderScriptNode = node; 	/* pointer back to the node that this is associated with */

	/* printf ("new_Shader_Script, node %s\n",stringNodeType(node->_nodeType)); */
	if (node->_nodeType == NODE_Script) {
	 	ret->num=nextScriptHandle();
 		#ifdef CPARSERVERBOSE
			printf("newScript: created new script with num %d\n", ret->num);
		#endif

		JSInit(ret->num);
	} else {
		ret->num = -1;
	}

	/* printf ("new_Shader_Script - num %d\n",ret->num); */

	return ret;
}

void deleteScript(struct Shader_Script* me)
{
 size_t i;
 for(i=0; i!=vector_size(me->fields); ++i)
  deleteScriptFieldDecl(vector_get(struct ScriptFieldDecl*, me->fields, i));
 deleteVector(struct ScriptFieldDecl*, me->fields);
 
 FREE_IF_NZ (me);
 /* FIXME:  JS-handle is not freed! */
}

/* Other members */
/* ************* */

struct ScriptFieldDecl* script_getField(struct Shader_Script* me, indexT n, indexT mod)
{
 size_t i;
 for(i=0; i!=vector_size(me->fields); ++i)
 {
  struct ScriptFieldDecl* curField=
   vector_get(struct ScriptFieldDecl*, me->fields, i);
  if(scriptFieldDecl_isField(curField, n, mod))
   return curField;
 }

 return NULL;
}

void script_addField(struct Shader_Script* me, struct ScriptFieldDecl* field)
{
 #ifdef CPARSERVERBOSE
 printf ("script_addField: adding field %p to script %d (pointer %p)\n",field,me->num,me); 
 #endif 

 vector_pushBack(struct ScriptFieldDecl*, me->fields, field);
 /* only do this for scripts */
 if (me->ShaderScriptNode->_nodeType==NODE_Script) scriptFieldDecl_jsFieldInit(field, me->num);
}

/* save the script code, as found in the VRML/X3D URL for this script */
BOOL script_initCode(struct Shader_Script* me, const char* code)
{
 	ASSERT(!me->loaded);

	SaveScriptText (me->num, (char *)code);
 	me->loaded=TRUE;
 	return TRUE;
}

/* get the script from this SFString. First checks to see if the string
   contains the script; if not, it goes and tries to see if the SFString 
   contains a file that (hopefully) contains the script */

static char *buffer = NULL;
static BOOL script_initCodeFromUri(struct Shader_Script* me, const char* uri)
{
 size_t i;
 char *filename = NULL;
 char *mypath = NULL;
 int rv;
 int removeIt = FALSE;

 /* strip off whitespace at the beginning JAS */
 while ((*uri<= ' ') && (*uri>0)) uri++;

 /* Try javascript protocol */
 for(i=0; i!=ARR_SIZE(JS_PROTOCOLS); ++i)
 {
  const char* u=uri;
  const char* v=JS_PROTOCOLS[i];

  while(*u && *v && *u==*v)
  {
   ++u;
   ++v;
  }

  /* Is this a simple "javascript:" "ecmascript:" or "vrmlscript:" uri? JAS*/
  if(!*v && *u==':') {
   	if (me != NULL) {
		return script_initCode(me, u+1); /* a script */
	} else {
		buffer = STRDUP(u+1);
		return TRUE; /* a shader, program text will be in the "buffer" */
	}
   }
 }

 /* Not a valid script in this SFString. Lets see if this
    is this a possible file that we have to get? */

 #ifdef CPARSERVERBOSE
 printf ("script_initCodeFromUri, uri is %s\n",uri); 
 #endif

 filename = (char *)MALLOC(1000);

 /* get the current parent */
 mypath = STRDUP(getInputURL());

 /* and strip off the file name, leaving any path */
 removeFilenameFromPath (mypath);

 /* add the two together */
 makeAbsoluteFileName(filename,mypath,(char *)uri);

 /* and see if it exists. If it does, try running script_initCode() on it */
 rv = FALSE;
 if (fileExists(filename,NULL,TRUE,&removeIt)) {
	buffer = readInputString(filename);
	rv= (buffer != NULL);
 	if (removeIt) UNLINK (filename);
	if (me!=NULL) rv = script_initCode(me,buffer);
 }

 FREE_IF_NZ (filename);
 FREE_IF_NZ (mypath);

 return rv;
}


/* initialize a script from a url. Expects valid input */
BOOL script_initCodeFromMFUri(struct Shader_Script* me, const struct Multi_String* s) {
	size_t i;

	for(i=0; i!=s->n; ++i) {
		FREE_IF_NZ(buffer);
		if(script_initCodeFromUri(me, s->p[i]->strptr)) {
			FREE_IF_NZ(buffer);
   			return TRUE;
		}
	}

	/* failure... */
	FREE_IF_NZ(buffer);
 	return FALSE;
}

/* initialize a shader from a url. returns pointer to pointer to text, if found, null otherwise */
/* pointer is freed (if not NULL) in the Shaders code */
char **shader_initCodeFromMFUri(const struct Multi_String* s) {
	size_t i;

	for(i=0; i!=s->n; ++i) {
		FREE_IF_NZ(buffer);
		if(script_initCodeFromUri(NULL, s->p[i]->strptr)) {
   			return &buffer;
		}
	}

	/* failure... */
	FREE_IF_NZ(buffer);
 	return NULL;
}
