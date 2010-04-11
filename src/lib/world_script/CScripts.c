/*
=INSERT_TEMPLATE_HERE=

$Id: CScripts.c,v 1.41 2010/04/11 18:01:31 crc_canada Exp $

???

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
#include <list.h>
#include <io_files.h>

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
#include "../input/InputFunctions.h"
#include "../x3d_parser/Bindable.h"
#include "../opengl/Textures.h"

#include "JScript.h"
#include "CScripts.h"
#include "jsUtils.h"
#include "jsNative.h"
#include "jsVRMLClasses.h"

#include <resources.h>
#include <limits.h>


/* JavaScript-"protocols" */
const char* JS_PROTOCOLS[]={
 "shader",
 "javascript",
 "ecmascript",
 "vrmlscript",
 "data:text/plain"};

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
 	ret->fieldDecl=newFieldDecl(mod, type, name, 
		JSparamIndex(lexer_stringUser_fieldName(me,name,mod),FIELDTYPES[type])
		, -1);
 ASSERT(ret->fieldDecl);

 /* Stringify */
 ret->ASCIIvalue=NULL; /* used only for XML PROTO ProtoInterface fields */
 
 /* printf ("newScript, asciiType %s,\n",stringFieldtypeType(
		fieldDecl_getType(ret->fieldDecl)));
 printf ("newScriptFieldDecl, name :%s:, getIndexName %d, ShaderScriptIndex %d\n", 
	fieldDecl_getShaderScriptName(ret->fieldDecl),
	fieldDecl_getIndexName(ret->fieldDecl), fieldDecl_getShaderScriptIndex(ret->fieldDecl)); */

 /* Field's value not yet initialized! */
 ret->valueSet=(mod!=PKW_initializeOnly);
 /* value is set later on */

 #ifdef CPARSERVERBOSE
 printf ("newScriptFieldDecl, returning name %s, type %s, mode %s\n",fieldDecl_getShaderScriptName(ret->fieldDecl), 
		stringFieldtypeType( fieldDecl_getType(ret->fieldDecl))
,PROTOKEYWORDS[ret->fieldDecl->mode]); 
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

	ret->ASCIIvalue = me->ASCIIvalue;
	
	ret->valueSet=(fieldDecl_getAccessType(ret->fieldDecl)!=PKW_initializeOnly);
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
 ASSERT(me->fieldDecl->PKWmode==PKW_initializeOnly); 
 me->value=v;
 me->valueSet=TRUE;
}

void scriptFieldDecl_setFieldASCIIValue(struct ScriptFieldDecl *me, const char *val)
{
 me->ASCIIvalue=(char *)val;
}


/* Get "offset" data for routing. Return an error if we are passed an invalid pointer. */
/* this is the field used for Scripts and Shaders; each number identifies a name AND data
   type; eg, 0="Var1","SFInt32", 1="Var1","MFFloat" while the lexerNameIndex would be the
   same */

int scriptFieldDecl_getRoutingOffset(struct ScriptFieldDecl* me)
{
 if (me == NULL) {
	ConsoleMessage ("call to scriptFieldDecl_getRoutingOffset made with NULL input");
	return INT_ID_UNDEFINED;
 }
 return fieldDecl_getShaderScriptIndex(me->fieldDecl);
}



/* Initialize JSField */
static void scriptFieldDecl_jsFieldInit(struct ScriptFieldDecl* me, int num) {
	#ifdef CPARSERVERBOSE
	printf ("scriptFieldDecl_jsFieldInit mode %d\n",me->fieldDecl->mode);
	#endif

	ASSERT(me->valueSet);
 	SaveScriptField(num, fieldDecl_getAccessType(me->fieldDecl), 
		fieldDecl_getType(me->fieldDecl), fieldDecl_getShaderScriptName(me->fieldDecl), me->value);
}

/* ************************************************************************** */
/* ********************************** Script ******************************** */
/* ************************************************************************** */

/* Constructor and destructor */
/* ************************** */

/* Next handle to be assinged */
static int handleCnt=0;

static int nextScriptHandle (void) {int retval; retval = handleCnt; handleCnt++; return retval;}

/* copy a Script node in a proto. */
struct X3D_Script * protoScript_copy (struct X3D_Script *me) {
	struct X3D_Script* ret;

	ret = createNewX3DNode(NODE_Script);
	ret->_parentResource = me->_parentResource;
	ret->directOutput = me->directOutput;
	ret->mustEvaluate = me->mustEvaluate;
	ret->url = me->url;
	ret->__scriptObj = me->__scriptObj;
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
	ret->num = -1;

	/* X3D XML protos do not have a node defined when parsed, Shaders and Scripts do */
	if (node!=NULL) {
		/* printf ("new_Shader_Script, node %s\n",stringNodeType(node->_nodeType)); */
		if (node->_nodeType == NODE_Script) {
		 	ret->num=nextScriptHandle();
 			#ifdef CPARSERVERBOSE
				printf("newScript: created new script nodePtr %u with num %d\n", node, ret->num);
			#endif

			JSInit(ret->num);
		}
	}

	/* printf ("new_Shader_Script - num %d, Shader_Script is %u\n",ret->num,ret); */

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

/* look for the field, via the ASCII name. Slower than script_getField, though... */
struct ScriptFieldDecl* script_getField_viaCharName (struct Shader_Script* me, const char *name)
{
 size_t i;
 for(i=0; i!=vector_size(me->fields); ++i)
 {
  struct ScriptFieldDecl* curField= vector_get(struct ScriptFieldDecl*, me->fields, i);
  if(strcmp(name,fieldDecl_getShaderScriptName(curField->fieldDecl)) == 0)
   return curField;
 }

 return NULL;
}

struct ScriptFieldDecl* script_getField(struct Shader_Script* me, indexT n, indexT mod)
{
 size_t i;
 for(i=0; i!=vector_size(me->fields); ++i)
 {
  struct ScriptFieldDecl* curField= vector_get(struct ScriptFieldDecl*, me->fields, i);
  if(scriptFieldDecl_isField(curField, n, mod))
   return curField;
 }

 return NULL;
}

void script_addField(struct Shader_Script* me, struct ScriptFieldDecl* field)
{

 #ifdef CPARSERVERBOSE
 printf ("script_addField: adding field %u to script %d (pointer %u)\n",field,me->num,me); 
 #endif 

 vector_pushBack(struct ScriptFieldDecl*, me->fields, field);

#ifdef CPARSERVERBOSE
	{
		size_t i;
		for(i=0; i!=vector_size(me->fields); ++i) {
			struct ScriptFieldDecl* curField=
				vector_get(struct ScriptFieldDecl*, me->fields, i);
			printf ("script_addField, now have field %d of %d, ASCIIname %s:",i,vector_size(me->fields),fieldDecl_getShaderScriptName(curField->fieldDecl));
			printf ("\n");
		}

	}
#endif


 /* only do this for scripts */
 if (me->ShaderScriptNode != NULL) {
   if (me->ShaderScriptNode->_nodeType==NODE_Script) scriptFieldDecl_jsFieldInit(field, me->num);
 }
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
 int rv;
 resource_item_t *res;

  rv = FALSE; /* initialize it */

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

  /* Is this a "data:text/plain," uri? JAS*/
  if((!*v && *u==',') || (!*v && *u==':')) {
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

 DEBUG_CPARSER("script_initCodeFromUri, uri is %s\n", uri); 

 res = resource_create_single(uri);
 resource_identify(root_res, res);
 if (res->type != rest_invalid) {
	 if (resource_fetch(res)) {
		 if (resource_load(res)) {
			 s_list_t *l;
			 openned_file_t *of;

			 l = res->openned_files;
			 of = ml_elem(l);

			/* ok - Scripts get initialized; shaders get the buffer returned */
			if (me==NULL) { /* a Shader */
			 	buffer = STRDUP(of->text);
			 	/* JAS printf("**** Shader:\n%s\n", buffer); 
				printf ("*** Shader: doing the quick return here\n"); */
				return TRUE;
			} else {
				/* a Script */
			 	/* printf("**** Script:\n%s\n", buffer); */
			 	rv = script_initCode(me, of->text);
			}
		 }
	 }
 }
 
 
 if (res->status == ress_loaded && rv) {
	 /* ok - we are replacing EXTERNPROTO with PROTO */
	 res->status = ress_parsed;
	 res->complete = TRUE;
	 return TRUE;
 } else {
	 /* failure, FIXME: remove res from root_res... */
/* 		resource_destroy(res); */
 }

 return FALSE;
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

printf ("shader - initCodeFromMFUri, failure...\n");
	/* failure... */
	FREE_IF_NZ(buffer);
 	return NULL;
}
