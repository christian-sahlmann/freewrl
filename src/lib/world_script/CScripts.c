/*
=INSERT_TEMPLATE_HERE=

$Id: CScripts.c,v 1.27 2009/10/05 15:07:24 crc_canada Exp $

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

#include "CScripts.h"
#include "JScript.h"
#include "jsUtils.h"
#include "jsNative.h"
#include "jsVRMLClasses.h"

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
 	ret->fieldDecl=newFieldDecl(mod, type, name, -1, FALSE);
 ASSERT(ret->fieldDecl);

 /* Stringify */
 ret->ASCIIname=fieldDecl_getStringName(me, ret->fieldDecl);
 ret->ASCIItype=FIELDTYPES[type];
 ret->ASCIIvalue=NULL; /* used only for XML PROTO ProtoInterface fields */

 /* Field's value not yet initialized! */
 ret->valueSet=(mod!=PKW_initializeOnly);
 /* value is set later on */

 #ifdef CPARSERVERBOSE
 printf ("newScriptFieldDecl, returning name %s, type %s, mode %s\n",ret->ASCIIname, ret->ASCIItype,PROTOKEYWORDS[ret->fieldDecl->mode]); 
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

	ret->ASCIIname = fieldDecl_getStringName(lex, ret->fieldDecl);
	ret->ASCIItype = me->ASCIItype;
	ret->ASCIIvalue = me->ASCIIvalue;
	
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
 me->value=v;
 me->valueSet=TRUE;
}

void scriptFieldDecl_setFieldASCIIValue(struct ScriptFieldDecl *me, const char *val)
{
 me->ASCIIvalue=val;
}


/* Get "offset" data for routing. Return an error if we are passed an invalid pointer. */
int scriptFieldDecl_getRoutingOffset(struct ScriptFieldDecl* me)
{
 if (me == NULL) {
	ConsoleMessage ("call to scriptFieldDecl_getRoutingOffset made with NULL input");
	return INT_ID_UNDEFINED;
 }
 return JSparamIndex(me->ASCIIname, me->ASCIItype);
}

/* Initialize JSField */
static void scriptFieldDecl_jsFieldInit(struct ScriptFieldDecl* me, uintptr_t num) {
	#ifdef CPARSERVERBOSE
	printf ("scriptFieldDecl_jsFieldInit mode %d\n",me->fieldDecl->mode);
	#endif

	ASSERT(me->valueSet);
 	SaveScriptField(num, me->fieldDecl->mode, me->fieldDecl->type, me->ASCIIname, me->value);
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
struct ScriptFieldDecl* script_getField_viaASCIIname (struct Shader_Script* me, const char *name)
{
 size_t i;
 for(i=0; i!=vector_size(me->fields); ++i)
 {
  struct ScriptFieldDecl* curField= vector_get(struct ScriptFieldDecl*, me->fields, i);
  if(strcmp(name,curField->ASCIIname) == NULL)
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
			printf ("script_addField, now have field %d of %d, ASCIIname %s:",i,vector_size(me->fields),curField->ASCIIname);
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
 char *filename = NULL;
 char *mypath = NULL;
 int rv;

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

 #ifdef CPARSERVERBOSE
 printf ("script_initCodeFromUri, uri is %s\n",uri); 
 #endif

#if defined(_MSC_VER)
#define PATH_MAX _MAX_PATH  /*32kb*/
#endif
 // Test if this could be a path name
 if (strlen(getInputURL) >= PATH_MAX) {
     return FALSE;
 }

 filename = (char *)MALLOC(PATH_MAX);

 /* get the current parent */
 mypath = STRDUP(getInputURL());

 /* and strip off the file name, leaving any path */
 removeFilenameFromPath (mypath);

 /* add the two together */
 makeAbsoluteFileName(filename,mypath,(char *)uri);

 /* and see if it exists. If it does, try running script_initCode() on it */
 rv = FALSE;
 if (fileExists(filename,NULL,TRUE)) {
	buffer = readInputString(filename);
	rv= (buffer != NULL);
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
