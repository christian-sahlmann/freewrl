/*
=INSERT_TEMPLATE_HERE=

$Id: CProto.c,v 1.50 2010/11/24 20:12:12 crc_canada Exp $

CProto ???

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
#include <stdio.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/JScript.h"
#include "../world_script/CScripts.h"
#include "../world_script/fieldSet.h"
#include "../input/EAIHelpers.h"	/* for newASCIIString() */
#include "CParseParser.h"
#include "CParseLexer.h"
#include "CProto.h"



#define DJ_KEEP_COMPILER_WARNING 0

#define PROTO_CAT(newString) { char *pt = (char *)newString; int len=0; int wlen = 0;\
		while ((*pt)) {len++; pt++;}; \
		wlen = (int) fwrite (newString,len,1,pexfile); \
		curstringlen += len; } 

#define STARTPROTOGROUP "Group{FreeWRL__protoDef %d FreeWRL_PROTOInterfaceNodes ["
#define PROTOGROUPNUMBER "] #PROTOPARAMS\n  children[ #PROTOGROUP\n"
#define ENDPROTOGROUP "}#END PROTOGROUP\n"

#define APPEND_SPACE PROTO_CAT (" ");
#define APPEND_THISID PROTO_CAT (thisID);
#define APPEND_NODE PROTO_CAT (stringNodeType(ele->isNODE));
#define APPEND_KEYWORD PROTO_CAT (stringKeywordType(ele->isKEYWORD));
#define APPEND_STRINGTOKEN PROTO_CAT ( ele->stringToken);
#define APPEND_TERMINALSYMBOL \
			{ char chars[3]; \
			chars[0] = (char) ele->terminalSymbol;\
			chars[1] = '\0';\
			PROTO_CAT (chars);\
			if (chars[0]=='}') { \
				PROTO_CAT(" #PROTO EXPANSION of ") PROTO_CAT((*thisProto)->protoName); \
				PROTO_CAT("\n"); }  \
			} 

#if DJ_KEEP_COMPILER_WARNING
#define APPEND_ENDPROTOGROUP PROTO_CAT (ENDPROTOGROUP);
#endif
#define SOMETHING_IN_ISVALUE (strlen(newTl) > 0) 
#define APPEND_ISVALUE PROTO_CAT (newTl);
#define APPEND_STARTPROTOGROUP_1 \
	sprintf(thisID,STARTPROTOGROUP,newProtoDefinitionPointer(*thisProto,INT_ID_UNDEFINED)); \
	PROTO_CAT(thisID);


#define APPEND_STARTPROTOGROUP_2 \
	PROTO_CAT(PROTOGROUPNUMBER)

#if DJ_KEEP_COMPILER_WARNING
#define APPEND_IF_NOT_OUTPUTONLY \
{ int coffset, ctype, ckind, field; \
	printf ("checking to see if node %s has an outputOnly field %s\n",stringNodeType(lastNode->isNODE), ele->stringToken); \
	findFieldInOFFSETS(lastNode->isNODE, findFieldInFIELDNAMES(ele->stringToken), &coffset, &ctype, &ckind); \
	printf ("found coffset %d ctype %d ckind %d\n",coffset, ctype, ckind); \
	if (ckind != KW_outputOnly) { printf ("APPENDING\n"); APPEND_STRINGTOKEN } else printf ("NOT APPENDING\n"); \
}
#endif

#define APPEND_EDITED_STRINGTOKEN \
	/* if this is an outputOnly field, dont bother printing it. It SHOULD be followed by an \
	   KW_IS, but the IS value from the PROTO definition will be blank */ \
	if (lastKeyword != NULL) { \
		/* is this a KW_IS, or another keyword that needs a PROTO expansion specific ID? */ \
		if (lastKeyword->isKEYWORD != KW_IS) { \
			sprintf (thisID," %s%d_",FABRICATED_DEF_HEADER,(int) ((*thisProto)->protoDefNumber)); \
			APPEND_THISID \
			APPEND_STRINGTOKEN \
		} \
		lastKeyword = NULL; \
	} else {APPEND_STRINGTOKEN}

static int dumpProtoFieldDeclarationNodes(struct VRMLLexer *me, struct ProtoDefinition *thisProto, FILE *pexfile);
static struct ProtoFieldDecl* protoFieldDecl_copy(struct VRMLLexer* lex, struct ProtoFieldDecl* me);
static struct X3D_Node* pointerHash_get(struct PointerHash* me, struct X3D_Node* o);
static void deletePointerHash(struct PointerHash* me);
static struct PointerHash* newPointerHash();
static void pointerHash_add(struct PointerHash* me, struct X3D_Node* o, struct X3D_Node* c);

/* internal sequence number for protos */
static  indexT latest_protoDefNumber =1;
static  indexT nextFabricatedDef=1;

/* ************************************************************************** */
/* ************************** ProtoElementPointer *************************** */
/* ************************************************************************** */

/* Constructor/destructor */
static struct ProtoElementPointer* newProtoElementPointer(void) {
	struct ProtoElementPointer *ret=MALLOC(sizeof(struct ProtoElementPointer));
	ASSERT (ret);

	ret->stringToken = NULL;
	ret->isNODE = ID_UNDEFINED;
	ret->isKEYWORD = ID_UNDEFINED;
	ret->terminalSymbol = ID_UNDEFINED;	
	ret->fabricatedDef = ID_UNDEFINED;
	return ret;
}


/* ************************************************************************** */
/* ******************************* ProtoFieldDecl *************************** */
/* ************************************************************************** */

/* Constructor and destructor */
/* ************************** */

/* Without default value (event) */
struct ProtoFieldDecl* newProtoFieldDecl(indexT mode, indexT type, indexT name)
{
 struct ProtoFieldDecl* ret=MALLOC(sizeof(struct ProtoFieldDecl));
  /* printf("creating ProtoFieldDecl %p\n", ret);  */
 ret->mode=mode;
 ret->type=type;
 ret->name=name;
 ret->alreadySet=FALSE;
 ret->fieldString = NULL;

 ret->scriptDests=newVector(struct ScriptFieldInstanceInfo*, 4);
 ASSERT(ret->scriptDests);
 return ret;
}

void deleteProtoFieldDecl(struct ProtoFieldDecl* me)
{
 FREE_IF_NZ(me);
}


/* setValue is at the end, because we need deep-copying there */
/* copy is at the end, too, because defaultVal needs to be deep-copied. */

/* ************************************************************************** */
/* ******************************** ProtoDefinition ************************* */
/* ************************************************************************** */

/* Constructor and destructor */
/* ************************** */

struct ProtoDefinition* newProtoDefinition()
{
 /* Attention!  protoDefinition_copy also sets up data from raw MALLOC!  Don't
  * forget to mimic any changes to this method there, too!
  */

 struct ProtoDefinition* ret=MALLOC(sizeof(struct ProtoDefinition));
 ASSERT(ret);

 /* printf("creating new ProtoDefinition %u\n", ret);  */

 ret->iface=newVector(struct ProtoFieldDecl*, 4);
 ASSERT(ret->iface);

 /* proto bodies are tokenized to help IS and routing to/from PROTOS */
/*
 ret->deconstructedProtoBody=newVector(struct ProtoElementPointer*, 128);
 ASSERT(ret->deconstructedProtoBody);
*/
 ret->deconstructedProtoBody = NULL;



 ret->protoDefNumber = latest_protoDefNumber++;
 ret->estimatedBodyLen = 0;
 ret->protoName = NULL;
 ret->isCopy = FALSE;

 return ret;
}

/* Other members */
/* ************* */

/* Retrieve a field by its "name" */
struct ProtoFieldDecl* protoDefinition_getField(struct ProtoDefinition* me,
 indexT ind, indexT mode)
{
 /* TODO:  O(log(n)) by sorting */
 size_t i;
 /* printf ("protoDefinition_getField; sizeof iface %d\n",vector_size(me->iface));  */
 if (!me) return NULL; /* error here, can not go through fields */
 for(i=0; i!=vector_size(me->iface); ++i)
 {
  struct ProtoFieldDecl* f=vector_get(struct ProtoFieldDecl*, me->iface, i);
  if(f->name==ind && f->mode==mode) {
   /* printf ("protoDefinition_getField, comparing %d %d and %d %d\n", f->name, ind, f->mode, mode); */
   return f;
  }
 }

 return NULL;
}

/* Copies the PROTO */
static struct ProtoDefinition* protoDefinition_copy(struct VRMLLexer* lex, struct ProtoDefinition* me)
{
	struct ProtoDefinition* ret=MALLOC(sizeof(struct ProtoDefinition));
	size_t i;

	ASSERT(ret);

	/* printf("protoDefinition_copy: copying %u to %u\n", me, ret);  */
	/* Copy interface */
	ret->iface=newVector(struct ProtoFieldDecl*, vector_size(me->iface));
	ASSERT(ret->iface);
	for(i=0; i!=vector_size(me->iface); ++i)
		vector_pushBack(struct ProtoFieldDecl*, ret->iface,
	protoFieldDecl_copy(lex, vector_get(struct ProtoFieldDecl*, me->iface, i)));

	/* reference the deconsctructed PROTO body */
	ret->deconstructedProtoBody = me->deconstructedProtoBody;
	ret->isCopy = TRUE;
	
 	ret->estimatedBodyLen = me->estimatedBodyLen;
	ret->protoDefNumber = latest_protoDefNumber++;
	if (me->protoName) ret->protoName = STRDUP(me->protoName);

	return ret;
}


/* Deep copying */
/* ************ */

/* Deepcopies sf */
#define DEEPCOPY_sfbool(l, v, i, h) v
#define DEEPCOPY_sfcolor(l,v, i, h) v
#define DEEPCOPY_sfcolorrgba(l,v, i, h) v
#define DEEPCOPY_sffloat(l,v, i, h) v
#define DEEPCOPY_sfint32(l,v, i, h) v
#define DEEPCOPY_sfnode(l,v, i, h) protoDefinition_deepCopy(l,v, i, h)
#define DEEPCOPY_sfrotation(l,v, i, h) v
#define DEEPCOPY_sfstring(l,v, i, h) deepcopy_sfstring(l, v)
#define DEEPCOPY_sftime(l,v, i, h) v
#define DEEPCOPY_sfvec2f(l,v, i, h) v
#define DEEPCOPY_sfvec2d(l,v, i, h) v
#define DEEPCOPY_mfvec2d(l,v, i, h) v
#define DEEPCOPY_sfvec3f(l,v, i, h) v
#define DEEPCOPY_sfvec4f(l,v, i, h) v
#define DEEPCOPY_sfvec3d(l,v, i, h) v
#define DEEPCOPY_sfvec4d(l,v, i, h) v

#if DJ_KEEP_COMPILER_WARNING
#define DEEPCOPY_sfvec4d(l,v, i, h) v
#endif

#define DEEPCOPY_mfvec4f(l,v, i, h) v
#define DEEPCOPY_mfvec4d(l,v, i, h) v
#define DEEPCOPY_sfimage(l, v, i, h) v
#define DEEPCOPY_sfdouble(l, v, i, h) v
#define DEEPCOPY_sfmatrix3f(l, v, i, h) v
#define DEEPCOPY_sfmatrix4f(l, v, i, h) v
#define DEEPCOPY_mfmatrix3f(l, v, i, h) v
#define DEEPCOPY_mfmatrix4f(l, v, i, h) v
#define DEEPCOPY_sfmatrix3d(l, v, i, h) v
#define DEEPCOPY_sfmatrix4d(l, v, i, h) v
#define DEEPCOPY_mfmatrix3d(l, v, i, h) v
#define DEEPCOPY_mfmatrix4d(l, v, i, h) v

static vrmlStringT deepcopy_sfstring(struct VRMLLexer* lex, vrmlStringT str)
{
	if (str)  return newASCIIString (str->strptr);

	/* should this return a blank string, or null? leave null for now */
		/* return newASCIIString(""); */
	return NULL;
}

/* Deepcopies a mf* */
#define DEEPCOPY_MFVALUE(lex, type, stype) \
 static struct Multi_##stype DEEPCOPY_mf##type(struct VRMLLexer* lex, struct Multi_##stype src, \
  struct ProtoDefinition* new, struct PointerHash* hash) \
 { \
  struct Multi_##stype dest; \
	/* printf ("DEEPCOPY_MFVALUE, src %u, dest count %d\n",src,src.n); */ \
  dest.n=src.n; \
  dest.p=MALLOC(sizeof(src.p[0])*src.n); \
  /* int i; for(i=0; i!=src.n; ++i) \
{ printf ("copying MF %d of %d\n",i,src.n); \
   dest.p[i]=DEEPCOPY_sf##type(lex, src.p[i], new, hash); \
} */ \
  return dest; \
 }
DEEPCOPY_MFVALUE(lex, bool, Bool)
DEEPCOPY_MFVALUE(lex, color, Color)
DEEPCOPY_MFVALUE(lex, colorrgba, ColorRGBA)
DEEPCOPY_MFVALUE(lex, float, Float)
DEEPCOPY_MFVALUE(lex, int32, Int32)
DEEPCOPY_MFVALUE(lex, node, Node)
DEEPCOPY_MFVALUE(lex, rotation, Rotation)
DEEPCOPY_MFVALUE(lex, string, String)
DEEPCOPY_MFVALUE(lex, time, Time)
DEEPCOPY_MFVALUE(lex, double, Double)
DEEPCOPY_MFVALUE(lex, vec2f, Vec2f)
DEEPCOPY_MFVALUE(lex, vec3f, Vec3f)
DEEPCOPY_MFVALUE(lex, vec3d, Vec3d)

/* ************************************************************************** */

/* Nodes; may be used to update the interface-pointers, too. */
static struct X3D_Node* protoDefinition_deepCopy(struct VRMLLexer* lex, struct X3D_Node* node,
 struct ProtoDefinition* new, struct PointerHash* hash)
{
 struct X3D_Node* ret;
 BOOL myHash=(!hash);


 printf("doing a deepcopy of proto with root node %p\n", node);

 /* If we get nothing, what can we return? */
 if(!node) return NULL;

 /* Check if we've already copied this node */
 if(hash)
 {
  ret=pointerHash_get(hash, node);
  if(ret)
   return ret;
 }

 if(!hash)
  hash=newPointerHash();

 /* Create it */
 ret=createNewX3DNode(node->_nodeType);
 /* printf ("CProto deepcopy, created a node %u of type %s\n",ret, stringNodeType(ret->_nodeType)); */

 /* Copy the fields using the NodeFields.h file */
 switch(node->_nodeType)
 {

  #define BEGIN_NODE(n) \
   case NODE_##n: \
   { \
    struct X3D_##n* node2=(struct X3D_##n*)node; \
    struct X3D_##n* ret2=(struct X3D_##n*)ret; \
	UNUSED(node2); UNUSED(ret2); 

  #define END_NODE(n) \
    break; \
   }


  /* Copying of fields depending on type */

  #define FIELD(n, field, type, var, realType) \
   ret2->var=DEEPCOPY_##type(lex, node2->var, new, hash); \
   UNUSED(ret2);

  #define EVENT_IN(n, f, t, v, realType)
  #define EVENT_OUT(n, f, t, v, realType)
  #define EXPOSED_FIELD(n, field, type, var, realType) \
   FIELD(n, field, type, var, realType)

  #include "NodeFields.h"

  #undef BEGIN_NODE
  #undef END_NODE
  #undef EVENT_IN
  #undef EVENT_OUT
  #undef EXOSED_FIELD
  #undef FIELD

  default:
   fprintf(stderr, "Nodetype %s unsupported for deep-copy...\n", stringNodeType(node->_nodeType));
   break;

 }
 if (node->_nodeType == NODE_Script) {
  /* If this is a script node, create a new context for the script */
	struct Shader_Script* old_script;
	struct Shader_Script* new_script;
	struct X3D_Script* ret2 = X3D_SCRIPT(ret);
	struct X3D_Script* node2 = X3D_SCRIPT(node);
	int i, j, k;

 	old_script = node2->__scriptObj; 
	ret2->__scriptObj = new_Shader_Script(node);

	new_script = ret2->__scriptObj;

	/* Init the code for the new script */
	script_initCodeFromMFUri(X3D_SCRIPT(ret2)->__scriptObj, &X3D_SCRIPT(ret2)->url);

	/* Add in the fields defined for the old script into the new script */
	for (i = 0; i !=  vector_size(old_script->fields); ++i) {
		struct ScriptFieldDecl* sfield = vector_get(struct ScriptFieldDecl*, old_script->fields, i);
		struct ScriptFieldDecl* newfield = scriptFieldDecl_copy(lex, sfield);
		if (fieldDecl_getAccessType(sfield->fieldDecl) == PKW_initializeOnly) {
			scriptFieldDecl_setFieldValue(newfield, sfield->value);
		}
		script_addField(new_script, newfield);

		/* Update the pointers in the proto expansion's field interface list for each field interface that has script destinations 
		   so that the script destinations point to the new script */
        	for (k=0; k != vector_size(new->iface); ++k) {
        	        struct ProtoFieldDecl* newdecl = vector_get(struct ProtoFieldDecl*, new->iface, k);
        	        for (j=0; j!= vector_size(newdecl->scriptDests); j++) {
        	                struct ScriptFieldInstanceInfo* sfieldinfo = vector_get(struct ScriptFieldInstanceInfo*, newdecl->scriptDests, j);
        	                if (sfieldinfo->script == old_script) {
        	                        sfieldinfo->script = new_script;
        	                }
				if (sfieldinfo->decl == sfield) {
					sfieldinfo->decl = newfield;
				}
        	        }
        	}
	}

  }

 if(myHash)
  deletePointerHash(hash);

 /* Add pointer pair to hash */
 if(!myHash)
  pointerHash_add(hash, node, ret);

 return ret;
}

/* ************************************************************************** */


/* Copies a fieldDeclaration */
static struct ProtoFieldDecl* protoFieldDecl_copy(struct VRMLLexer* lex, struct ProtoFieldDecl* me)
{
 struct ProtoFieldDecl* ret=newProtoFieldDecl(me->mode, me->type, me->name);
 size_t i;
 ret->alreadySet=FALSE;

	#ifdef CPROTOVERBOSE
	printf ("\nstart of protoFieldDecl_copy\n");
	#endif

 /* copy over the fieldString */
	#ifdef CPROTOVERBOSE
  printf ("protoFieldDecl_copy: copying field string for field... %s\n",me->fieldString); 
	#endif
 if (me->fieldString != NULL) ret->fieldString = STRDUP(me->fieldString);

 ret->mode=me->mode;
 ret->type=me->type;
 ret->name=me->name;
	#ifdef CPROTOVERBOSE
 printf ("copied mode %s type %s and name %d\n",stringPROTOKeywordType(ret->mode)
	, stringFieldtypeType(ret->type), ret->name);
 printf ("protoFieldDecl_copy, copied fieldString for proto field\n"); 
	#endif

  /* Copy scriptfield dests */
  for (i=0; i!=vector_size(me->scriptDests); ++i) {
   	struct ScriptFieldInstanceInfo* temp;
   	struct ScriptFieldInstanceInfo* temp2;
	vector_pushBack(struct ScriptFieldInstanceInfo*, ret->scriptDests, scriptFieldInstanceInfo_copy(vector_get(struct ScriptFieldInstanceInfo*, me->scriptDests, i)));
   	temp = vector_get(struct ScriptFieldInstanceInfo*, me->scriptDests, i);
   	temp2 = vector_get(struct ScriptFieldInstanceInfo*, ret->scriptDests, i);
  }

 /* Copy default value */


 /* nodes that are of type mode==PKW_initializeOnly || mode==PKW_inputOutput 
	are copied, nodes that are just inputOnly or outputOnly are ignored */

	#ifdef CPROTOVERBOSE
	printf ("protoFieldDecl_copy, copying type %s\n",stringFieldtypeType(me->type));
	#endif

	if (me->type==PKW_initializeOnly || me->type==PKW_inputOutput) {
 		switch(me->type)
 		{
 		 #define SF_TYPE(fttype, type, ttype) \
   			case FIELDTYPE_##fttype: \
    			ret->defaultVal.type=DEEPCOPY_##type(lex, me->defaultVal.type, NULL, NULL); \
    			break;
  			#define MF_TYPE(fttype, type, ttype) \
   				SF_TYPE(fttype, type, ttype)
  			#include "VrmlTypeList.h"
  			#undef SF_TYPE
  			#undef MF_TYPE

  		default:
   		parseError("Unsupported type in defaultValue!");
 		}
	} else {
	#ifdef CPROTOVERBOSE
	printf ("protoFieldDecl_copy, ignoring this field\n");
	#endif
	}

	#ifdef CPROTOVERBOSE
	printf ("finished protoFieldDecl_copy\n");
	#endif
 return ret;
}

/* ************************************************************************** */
/* ******************************* PointerHash ****************************** */
/* ************************************************************************** */

/* Constructor and destructor */
/* ************************** */

static struct PointerHash* newPointerHash()
{
 struct PointerHash* ret=MALLOC(sizeof(struct PointerHash));
 size_t i;
 ASSERT(ret);

 for(i=0; i!=POINTER_HASH_SIZE; ++i)
  ret->data[i]=NULL;

 return ret;
}

static void deletePointerHash(struct PointerHash* me)
{
 size_t i;
 for(i=0; i!=POINTER_HASH_SIZE; ++i)
  if(me->data[i])
   deleteVector(struct PointerHashEntry, me->data[i]);
 FREE_IF_NZ (me);
}

/* Query the hash */
static struct X3D_Node* pointerHash_get(struct PointerHash* me, struct X3D_Node* o)
{
 size_t pos=((unsigned long)o)%POINTER_HASH_SIZE;
 size_t i;

 if(!me->data[pos])
  return NULL;

 for(i=0; i!=vector_size(me->data[pos]); ++i)
 {
  struct PointerHashEntry* entry=
   &vector_get(struct PointerHashEntry, me->data[pos], i);
  if(entry->original==o)
   return entry->copy;
 }

 return NULL;
}

/* Add to the hash */
static void pointerHash_add(struct PointerHash* me,
 struct X3D_Node* o, struct X3D_Node* c)
{
 size_t pos=((unsigned long)o)%POINTER_HASH_SIZE;
 struct PointerHashEntry entry;

 ASSERT(!pointerHash_get(me, o));

 if(!me->data[pos])
  me->data[pos]=newVector(struct PointerHashEntry, 4);
 
 entry.original=o;
 entry.copy=c;

 vector_pushBack(struct PointerHashEntry, me->data[pos], entry);
}


/* for backtracking to see if this field expansion has a proto expansion in it */
/* I know, I know - going backwards through input, but what can happen is that:


#VRML V2.0 utf8
PROTO StandardAppearance [ ] { Appearance { material Material { } } } #end of proto standardAppearance
PROTO Test [ field MFNode children [ ] ] { Transform { children IS children } } #end of protoTest

See the PROTO expansion in the "children" field of the Test proto invocation in the following:
Test { children Shape { appearance StandardAppearance { } geometry Sphere { } } } #end of test expansion 

the "children" field expansion gets:
Shape { appearance Group{FreeWRL__protoDef 5980080 children[ #PROTOGROUP
Appearance {material Material {}}]}#END PROTOGROUP
 geometry Sphere { } }

And, the whole thing becomes:
Group{FreeWRL__protoDef 5975760 children[ #PROTOGROUP
Transform {children  Shape { appearance Group{FreeWRL__protoDef 5980080 children[ #PROTOGROUP
Appearance {material Material {}}]}#END PROTOGROUP
 geometry Sphere { } } }]}#END PROTOGROUP

*/

/* go through a proto invocation, and get the invocation fields (if any) */
static void getProtoInvocationFields(struct VRMLParser *me, struct ProtoDefinition *thisProto) {
	char *copyPointer;
	char *initCP;
	char tmp;
	char *inputCopy = NULL;

	struct ProtoFieldDecl* pdecl = NULL;
	union anyVrml thisVal;
	#ifdef CPROTOVERBOSE
	printf ("start of getProtoInvocationFields, lexer %u nextIn :%s:\n",me->lexer, me->lexer->nextIn);
	#endif

	while (!lexer_closeCurly(me->lexer)) {

		lexer_setCurID(me->lexer);
		#ifdef CPROTOVERBOSE
		printf ("getProtoInvocationFields, id is %s\n",me->lexer->curID);
		#endif

		if (me->lexer->curID != NULL) {
			#ifdef CPROTOVERBOSE
			printf ("getProtoInvocationFields, curID != NULL\n");
			#endif

			pdecl = getProtoFieldDeclaration(me->lexer, thisProto,me->lexer->curID);
			if (pdecl != NULL) {
				#ifdef CPROTOVERBOSE
				printf ("getProtoInvocationFields, we have pdecl original as %s\n",pdecl->fieldString);
				#endif

	
				/* get a link to the beginning of this field */
				FREE_IF_NZ(me->lexer->curID);
				copyPointer = (char *) me->lexer->nextIn;
				initCP = (char *) me->lexer->startOfStringPtr[me->lexer->lexerInputLevel];

				#ifdef CPROTOVERBOSE
				printf ("getProtoInvocationField, going to parse type of %d mode (%s), curID %s, starting at :%s:\n",pdecl->type, stringPROTOKeywordType(pdecl->mode),
						me->lexer->curID, me->lexer->nextIn);
				#endif

				if (pdecl->mode != PKW_outputOnly) {

					lexer_setCurID(me->lexer);
					#ifdef CPROTOVERBOSE
					printf ("getProtoInvocationField, checking if this is an IS :%s:\n",me->lexer->curID);
					#endif

					if(me->curPROTO && lexer_keyword(me->lexer, KW_IS)) {
						struct ProtoFieldDecl* pexp;

						#ifdef CPROTOVERBOSE
						printf ("getProtoInvocationField, FOUND IS on protoInvocationFields; have to replace \n");
						#endif


						FREE_IF_NZ(me->lexer->curID);
						lexer_setCurID(me->lexer);
						#ifdef CPROTOVERBOSE
						printf ("getProtoInvocationField, going to try and find :%s: in curPROTO\n",me->lexer->curID);
						#endif

						pexp = getProtoFieldDeclaration(me->lexer,me->curPROTO,me->lexer->curID);
						if (pexp != NULL) {
							#ifdef CPROTOVERBOSE
							printf ("getProtoInvocationField, attaching :%s: to :%s:\n",pexp->fieldString, me->lexer->nextIn);
							#endif

							concatAndGiveToLexer(me->lexer, pexp->fieldString, me->lexer->nextIn);
						}
						FREE_IF_NZ(me->lexer->curID);
					}

					/* printf ("getPRotoInvocationFields, before parseType, nextIn %s\n",me->lexer->nextIn); */
					if (!parseType(me, pdecl->type, &thisVal)) {
						#ifdef CPROTOVERBOSE
						printf ("getProtoInvocationField, parsing error on field value in proto expansion\n");
						#endif

					} else {
						#ifdef CPROTOVERBOSE
						printf ("getProtoInvocationField, parsed field; nextin was :%s:, now :%s:\n",copyPointer, me->lexer->nextIn);
						#endif

					}
	
					/* was this possibly a proto expansion? */

					if (initCP != me->lexer->startOfStringPtr[me->lexer->lexerInputLevel]) {
					printf ("getProtoInvocationField, initCP %p startOfStringPtr %p\n",initCP, me->lexer->startOfStringPtr[me->lexer->lexerInputLevel]); 
printf ("PROTO HEADER - possible proto expansion in header?? \n");
					}

					/* copy over the new value */
					initCP = (char *) (me->lexer->nextIn);
					tmp = *initCP; *initCP = '\0';
					FREE_IF_NZ(pdecl->fieldString); 
					pdecl->fieldString = MALLOC (3 + strlen(copyPointer));
					pdecl->fieldString[0] = '\0';
					strcat (pdecl->fieldString, " ");
					strcat (pdecl->fieldString,copyPointer);
					*initCP = tmp;
					#ifdef CPROTOVERBOSE
					printf ("getProtoInvocationFields, just copied :%s: remainder :%s:\n",pdecl->fieldString,me->lexer->nextIn);
					#endif
				} else {
					#ifdef CPROTOVERBOSE
					printf ("getProtoInvocationField, skipped parsing this type of %s\n",stringPROTOKeywordType(pdecl->type));
					#endif

				}
			} else {
				CPARSE_ERROR_CURID("PROTO field not found in PROTO");
				return;
			}
		} else {
			/* hmmm - an error; lets not loop here forever */
			CPARSE_ERROR_CURID ("expected an identifier in PROTO header...");
			return;
		}

		#ifdef CPROTOVERBOSE
		printf ("after pt, curID %s\n",me->lexer->curID);
		#endif

		lexer_skip (me->lexer);
		FREE_IF_NZ (inputCopy);
	}

	#ifdef CPROTOVERBOSE
	printf ("end of getProtoInvocationFields\n");
	#endif
}

/* what we do is to ensure that we have a "destination" we create a node containing the VALUE of each field of the 
   PROTO expansion, and we put it in a "special place" within the PROTO Group expansion */

static int dumpProtoFieldDeclarationNodes(struct VRMLLexer *lex, struct ProtoDefinition *thisProto, FILE *pexfile) {
	struct ProtoFieldDecl* pdecl = NULL;
	
	int max;
	int i;
	int writtenlen = 0;

	max = protoDefinition_getFieldCount(thisProto);

	/* printf ("fieldCount %d\n",protoDefinition_getFieldCount(thisProto)); */

	for (i=0; i<max; i++) {
		pdecl = protoDefinition_getFieldByNum(thisProto,i);

		/* printf ("getIndexName %s ",protoFieldDecl_getStringName(lex,pdecl));
		printf (" mode %d type %s name %d fieldString %s\n", 
			pdecl->mode, stringFieldtypeType(pdecl->type), pdecl->name, pdecl->fieldString); 
		*/
		

		/* fields that are PKW_initializeOnly CAN NOT have routing, so we do not need to take
		   up valuable memory creating a pocket for the routing values */
		if (pdecl->mode != PKW_initializeOnly) {
		writtenlen += fprintf (pexfile, "\tDEF PROTO_%p_%s Metadata%s {\n",
			thisProto,
			protoFieldDecl_getStringName(lex,pdecl),
			stringFieldtypeType(pdecl->type)); 
		if (pdecl->fieldString != NULL) 
			writtenlen += fprintf (pexfile, "\t\tvalue %s\n",pdecl->fieldString); 
		writtenlen += fprintf (pexfile, "\t}\n"); 
		}
	}
	return writtenlen;
}


/* find the proto field declare for a particular proto */
struct ProtoFieldDecl* getProtoFieldDeclaration(struct VRMLLexer *me, struct ProtoDefinition *thisProto, char *thisID)
{
	indexT retUO;
	const char** userArr;
	size_t userCnt;
	struct ProtoFieldDecl* ret = NULL;

	#ifdef CPROTOVERBOSE
	printf ("getProtoFieldDeclaration, for field :%s:\n",thisID);
	#endif

	/* go through all 4 vectors; it is possible that:
		PROTOA {field XXX...}
		PROTOB {inputOutput XXX} 
	  we have to go through until we find a match, in this example, XXX
	  could be found in a couple of fieldType arrays.
	*/

	/* start off with the initializeOnly fieldType */
	userArr=&vector_get(const char*, me->user_initializeOnly, 0);
	userCnt=vector_size(me->user_initializeOnly);
	retUO=findFieldInARR(thisID, userArr, userCnt);
	if (retUO != ID_UNDEFINED) {
		/* we found the string in the parser fieldType, lets see if it is here... */
		ret=protoDefinition_getField(thisProto,retUO,PKW_initializeOnly);
		/* FOUND IT in here.... */
		if (ret != NULL) return ret;
	}

	/* look in the inputOutput fields... */
	userArr=&vector_get(const char*, me->user_inputOutput, 0);
	userCnt=vector_size(me->user_inputOutput);
	retUO=findFieldInARR(thisID, userArr, userCnt);
	if (retUO != ID_UNDEFINED) {
		/* we found the string in the parser fieldType, lets see if it is here... */
		ret=protoDefinition_getField(thisProto,retUO,PKW_inputOutput);
		/* FOUND IT in here.... */
		if (ret != NULL) return ret;
	}

	/* possibly we will get lucky with the inputOnly fields?? */
	userArr=&vector_get(const char*, me->user_inputOnly, 0);
	userCnt=vector_size(me->user_inputOnly);
	retUO=findFieldInARR(thisID, userArr, userCnt);
	if (retUO != ID_UNDEFINED) {
		/* we found the string in the parser fieldType, lets see if it is here... */
		ret=protoDefinition_getField(thisProto,retUO,PKW_inputOnly);
		/* FOUND IT in here.... */
		if (ret != NULL) return ret;
	}

	/* boy, lets hope that this is an outputOnly field... */
	userArr=&vector_get(const char*, me->user_outputOnly, 0);
	userCnt=vector_size(me->user_outputOnly);
	retUO=findFieldInARR(thisID, userArr, userCnt);
	if (retUO != ID_UNDEFINED) { 
		ret=protoDefinition_getField(thisProto,retUO,PKW_outputOnly);
		/* FOUND IT in here.... */
		if (ret != NULL) return ret;
	}

	/* return an error, and handle this later */
	return NULL;
}


/* take an ascii string apart, and tokenize this so that we can extract
	IS fields, and route to/from properly */
void tokenizeProtoBody(struct ProtoDefinition *me, char *pb) {
	struct VRMLLexer *lex;
	vrmlInt32T tmp32;
	vrmlFloatT tmpfloat;
	vrmlStringT tmpstring;
	struct ProtoElementPointer* ele;
	struct ProtoElementPointer* tE;
	struct ProtoElementPointer* tD;
	int toPush;
	int index; 	/* needs to go negative, so can not do indexT */
	indexT ct = 0;

	/* remove spaces at start of string, to help to see if string is empty */
	while ((*pb != '\0') && (*pb <= ' ')) pb++;

	/* record this body length to help us with MALLOCing when expanding PROTO */
	me->estimatedBodyLen = (int) strlen(pb) * 2;

	lex = newLexer();
	lexer_fromString(lex,pb);

	/* make up deconstructedProtoBody here */
 	me->deconstructedProtoBody=newVector(struct ProtoElementPointer*, 128);
	ASSERT(me->deconstructedProtoBody);


	while (lex->isEof == FALSE) {

		ele = newProtoElementPointer();
		toPush = TRUE; /* only put this new element on Vector if it is successful */

		if (lexer_setCurID(lex)) {
			if ((ele->isKEYWORD = findFieldInKEYWORDS(lex->curID)) == ID_UNDEFINED)  {
				if ((ele->isNODE = findFieldInNODES(lex->curID)) == ID_UNDEFINED)  {
					ele->stringToken = lex->curID;
					lex->curID = NULL;
				}
			}
			FREE_IF_NZ(lex->curID);

			/* ok, if this is an IS, go back to the controlling NODE, and ENSURE that
			   it has a DEF to enable it to accept external ROUTE requests */
			if (ele->isKEYWORD == KW_IS) {
				/* printf ("tokenizeProtoBody, found IS at element %d\n",index); */

				index  = (int) vector_size(me->deconstructedProtoBody) -1; /* remember, 0->(x-1), not 1->x */
				/* printf ("rewinding, starting at %d\n",index); */
				/* printf ("tokenizeProtoBody, found an IS, lets go back... we currently have %d\n",index); */
				if (index>0) {
					do {
                				tE = vector_get(struct ProtoElementPointer*, me->deconstructedProtoBody, (indexT)index);
                				ASSERT(tE);
						index--;
					} while ((index>=0) && (tE->isNODE == ID_UNDEFINED));

					/* ok, we should be at the NODE for this IS */

					/* printf ("after rewinding, we are at node %d type %d\n",index,tE->isNODE); */

					if (tE->isNODE != ID_UNDEFINED) {
						/* printf ("node for IS is an %s\n",stringNodeType(tE->isNODE)); */
						tD = NULL;
						index--; 		/* back up to where the DEF should be */
						if (index>=0) {
							tD = vector_get(struct ProtoElementPointer*, me->deconstructedProtoBody, (indexT)index);
							/* printf ("backed up, have NO %d KW %d te %d str %s\n",
								tD->isNODE, tD->isKEYWORD, tD->terminalSymbol, tD->stringToken); */

							ASSERT (tD);
							if (tD->isKEYWORD != KW_DEF) {
								tD = NULL; /* not a DEF, we have to make a special note of this */
							}

						}

						/* if we DID NOT find a DEF back in the stack, tell this NODE to put a DEF in */
						if (tD == NULL) {
							/* printf ("did not find DEF, assign unique ID to this node\n"); */
							ASSIGN_UNIQUE_ID(tE)
						}
					}
				}
			}
				
		} else if (lexer_point(lex)) { ele->terminalSymbol = (indexT) '.';
		} else if (lexer_openCurly(lex)) { ele->terminalSymbol = (indexT) '{';
		} else if (lexer_closeCurly(lex)) { ele->terminalSymbol = (indexT) '}';
		} else if (lexer_openSquare(lex)) { ele->terminalSymbol = (indexT) '[';
		} else if (lexer_closeSquare(lex)) { ele->terminalSymbol = (indexT) ']';
		} else if (lexer_colon(lex)) { ele->terminalSymbol = (indexT) ':';

		} else if (lexer_string(lex,&tmpstring)) { 
			/* must put the double quotes back on */
			ele->stringToken = MALLOC (tmpstring->len + 3);
			sprintf (ele->stringToken, "\"%s\"",tmpstring->strptr);
		} else {
			/* printf ("probably a number, scan along until it is done. :%s:\n",lex->nextIn); */
			if ((*lex->nextIn == '-') || ((*lex->nextIn >= '0') && (*lex->nextIn <= '9'))) {
				char *cur;
				int ignore;
				char *endone; char *endtwo;
				int length;

				/* see which of float, int32 gobbles up more of the string */
				cur = (char *) lex->nextIn;

				ignore = lexer_float(lex,&tmpfloat);
				endone = (char *)lex->nextIn;

				/* put the next in pointer back to the beginning of the number */
				lex->nextIn = cur;


				ignore = lexer_int32(lex,&tmp32);
				endtwo = (char *)lex->nextIn;

				/* put the next in pointer back to the beginning of the number */
				lex->nextIn = cur;

				/* which one puts us further into the proto text? */
				if (endone > endtwo) {
					lex->nextIn = endone;
				} else {
					lex->nextIn = endtwo;
				}

				/* how many bytes do we have here? */
				length = (int) (lex->nextIn-cur);
				if ((length <0) || (length > 50)) {
					ConsoleMessage ("Internal error parsing proto - complain bitterly");
					length = 0;
				}

				ele->stringToken = MALLOC (length+1);
				ASSERT (ele->stringToken);

				memcpy(ele->stringToken,cur,(size_t)length);
				ele->stringToken[length] = '\0';
			} else {
				if (*lex->nextIn != '\0') ConsoleMessage ("lexer_setCurID failed on char :%d:\n",*lex->nextIn);
				lex->nextIn++;
				toPush = FALSE;
			}
		}


		/* printf ("newprotoele %d, NODE %d KW %d ts %d st %s\n",ct, ele->isNODE, ele->isKEYWORD, ele->terminalSymbol, ele->stringToken); */
		ct ++;

		/* push this element on the vector for the PROTO */
		if (toPush) vector_pushBack(struct ProtoElementPointer*, me->deconstructedProtoBody, ele);
	}
	deleteLexer(lex);



	/* check the deconstructedProtoBody */
	/* if the user types in "DEF Material Material {}" the first "Material" is NOT a node, but a stringToken... */

	{
        indexT i;
        indexT protoElementCount;
        struct ProtoElementPointer* ele;
        struct ProtoElementPointer* tempEle;

        /* go through each part of this deconstructedProtoBody, and see what needs doing... */
        protoElementCount = vector_size(me->deconstructedProtoBody);
        i = 0;
        while (i < protoElementCount) {
                /* get the current element */
                ele = vector_get(struct ProtoElementPointer*, me->deconstructedProtoBody, i);

		/* is this a DEF, USE or IS? */
		/* sanity check following node */
		switch (ele->isKEYWORD) {
			case KW_DEF:
			case KW_USE:
			case KW_IS: 
				
			if (i<(protoElementCount-1)) {

               			tempEle = vector_get(struct ProtoElementPointer*, me->deconstructedProtoBody, i+1);
				if (tempEle->stringToken == NULL) {
					/* did this one get read in as a NODE? */
					if (tempEle->isNODE != ID_UNDEFINED) {
						/* yes, make this one a stringToken... */
						tempEle->stringToken = STRDUP(stringNodeType(tempEle->isNODE));
						/* ... and make sure it is NOT a node! */
						tempEle->isNODE = ID_UNDEFINED;
					}
				}
			/* printf ("protoCheck, at %d, keyword %s, stringToken %s\n",i,stringKeywordType(ele->isKEYWORD), tempEle->stringToken); */
			}

			break;
			default: {};
		}

		i++;
	}
	}
}

/* possibly create some ROUTES to reflect changes for internal variables - this is the MetadataSF and MetadataMF special nodes routing */
static int addProtoUpdateRoute (struct VRMLLexer *me, FILE *routefile, char *fieldName, char *protoNameInHeader, char *thisID, struct ProtoDefinition *thisProto, indexT indx) {
	struct ProtoFieldDecl *myPF;
	struct ProtoElementPointer* tE;
	char *defName = "";
	int retcount = 0;
	int index = (int) indx; /* cast to Integer */

	#ifdef CPROTOVERBOSE
	printf ("addProtoUpdateRoute, fieldName :%s: protoNameInHeader :%s: thisID :%s:\n",fieldName, protoNameInHeader,thisID);
	#endif

	/* bounds check - if the deconstructedProtoBody == null, just return */
	if (thisProto->deconstructedProtoBody == NULL) return 0;


	/* is this one a DEFed node, whereby we need to keep the DEF name as part of this? */
	do {
		tE = vector_get(struct ProtoElementPointer*, thisProto->deconstructedProtoBody, (indexT)index);
		ASSERT(tE);
		index--;
	} while ((index >= 0) && (tE->isNODE == ID_UNDEFINED));
	#ifdef CPROTOVERBOSE
	printf ("addProto, looking at element %d\n",index);
	printf (" NODE:\t%s\n",stringNodeType(tE->isNODE));
	#endif

	/* printf ("addProtoUpdateRoute at element %d...\n",index); */
	if (index>0) {
		tE = vector_get(struct ProtoElementPointer*, thisProto->deconstructedProtoBody, (indexT) (index-1));
		/* printf(" got TE, isKeyword %d\n",tE->isKEYWORD); */

               if (tE->isKEYWORD == KW_DEF) {
			#ifdef CPROTOVERBOSE
                        printf (" KW:\t%s\n",stringKeywordType(tE->isKEYWORD));
			#endif

			/* lets look for the DEF name */
			tE = vector_get(struct ProtoElementPointer*, thisProto->deconstructedProtoBody, (indexT) index);
               		if (tE->stringToken != NULL) {
				#ifdef CPROTOVERBOSE
                	        printf (" string\t:%s:",tE->stringToken);
				#endif

				defName = tE->stringToken;
                	}
                }
	}


	myPF = getProtoFieldDeclaration (me, thisProto, protoNameInHeader);
	if (myPF == NULL) return 0;

	/* is this an initializeOnly field, so no possible routing updates? */
	if (myPF->mode == PKW_initializeOnly) return 0;

	/* is this one accepting routes into the PROTO? */
	if ((myPF->mode == PKW_inputOutput) || (myPF->mode == PKW_inputOnly)) {
		retcount += fprintf (routefile,"ROUTE PROTO_%p_%s.valueChanged TO %s%s.%s #Meta route, inputOutput or inputOnly\n",
			thisProto,protoNameInHeader,thisID,defName,fieldName);
	}

	/* is this one accepting routes from the PROTO to the outside world? */
	if ((myPF->mode == PKW_inputOutput) || (myPF->mode == PKW_outputOnly)) {
		retcount += fprintf (routefile, "ROUTE %s%s.%s TO PROTO_%p_%s.setValue #Meta route, inputOutput or outputOnly\n",
			thisID,defName,fieldName, thisProto,protoNameInHeader);
	}

	return retcount;
}


/* make an expansion of the PROTO, and return it to the main caller */
/* ok, now, this is kind of complicated.

A proto of:
PROTO FullRound [
        field SFTime cycleInterval 6
        eventOut SFRotation value_changed
        field SFVec3f axis 1 0 0
] {
        DEF TIMER TimeSensor
          {cycleInterval IS cycleInterval
           startTime 0 stopTime -1 enabled TRUE loop TRUE}
        DEF ORI ScalarInterpolator {
                key [ 0 0.25 0.5 0.75 1]
                keyValue [
                        0
                        1.570796
                        3.141593
                        4.71238898
                        6.2831853
                ]
        }
        DEF SCRI Script {
                eventIn SFFloat set_fraction
                eventOut SFRotation value_changed IS value_changed
                field SFVec3f axis IS axis
                url "javascript:
                   function set_fraction(val,time) {
                        value_changed = new SFRotation(
                         axis.x,axis.y,axis.z,val
                        );
                   }
                "
        }
        ROUTE TIMER.fraction_changed TO ORI.set_fraction
        ROUTE ORI.value_changed TO SCRI.set_fraction
}


gets expanded to the following. 

NOTE:	- the PROTOInterfaceNodes section;
	- routing to/from these nodes at the end.
	- DEF names become uniqie;
	- local routing (see last 2 routes above) get redefined to use local names;
	- any routing TO/FROM the proto goes to the nodes in the FreeWRL_PROTOInterfaceNodes;
	  the routes to/from these nodes do the route multiplication; eg, one route to the
	  PROTO can get expanded into many IS references.


Group{
	FreeWRL__protoDef 11446464 FreeWRL_PROTOInterfaceNodes [	
		DEF PROTO_11446464_value_changed MetadataSFRotation { }
] #PROTOPARAMS

  children[ #PROTOGROUP
DEF  fReEwEL_fAbricatio_dEF_11_TIMER TimeSensor {cycleInterval     40 startTime 0 stopTime -1 enabled TRUE loop TRUE } #PROTO EXPANSION of FullRound
DEF  fReEwEL_fAbricatio_dEF_11_ORI ScalarInterpolator {key [0 0.250000 0.500000 0.750000 1 ]keyValue [0 1.570796 3.141593 4.712389 6.283185 ]} #PROTO EXPANSION of FullRound
DEF  fReEwEL_fAbricatio_dEF_11_SCRI Script {i
	eventIn SFFloat set_fraction 
	eventOut SFRotation value_changed  
	field SFVec3f axis   0 0 1 url "javascript:
		   function set_fraction(val,time) {
		   	value_changed = new SFRotation(
			 axis.x,axis.y,axis.z,val
			);
		   }
		" } #PROTO EXPANSION of FullRound
ROUTE  fReEwEL_fAbricatio_dEF_11_TIMER .fraction_changed TO  fReEwEL_fAbricatio_dEF_11_ORI .set_fraction 
ROUTE  fReEwEL_fAbricatio_dEF_11_ORI .value_changed TO  fReEwEL_fAbricatio_dEF_11_SCRI .set_fraction ] #End of PROTO Expansion children field

ROUTE  fReEwEL_fAbricatio_dEF_11_SCRI.value_changed TO PROTO_11446464_value_changed.setValue #Meta route, inputOutput or outputOnly
}#END PROTOGROUP

*/



char *protoExpand (struct VRMLParser *me, indexT nodeTypeU, struct ProtoDefinition **thisProto, int *protoSize) {
	char *newProtoText;
	char tempname[1000];
	char tempRoutename[1000];
	char thisID[1000];
	indexT i;
	indexT protoElementCount;
	struct ProtoElementPointer* ele;
	struct ProtoElementPointer* tempEle;
	
	struct ProtoElementPointer* lastKeyword = NULL;
	struct ProtoElementPointer* lastNode = NULL;

	FILE *pexfile;
	FILE *routefile;
	int curstringlen = 0;

	int routeSize = 0;

	#define	OPEN_PROTO_EXPAND_FILE_WRITE { \
		sprintf (tempname, "%s",tempnam("/tmp","freewrl_tmp")); \
		sprintf (tempRoutename, "%s",tempnam("/tmp","freewrl_tmp")); \
		pexfile = fopen (tempname,"w"); \
		routefile = fopen (tempRoutename,"w"); \
		if ((pexfile==NULL) ||(routefile==NULL)) {ConsoleMessage ("error opening temp file!"); return "";}}

	#define	OPEN_PROTO_EXPAND_FILE_READ { \
		/* file name already in the "tempname" file */ \
		pexfile = fopen (tempname,"r"); \
		routefile = fopen (tempRoutename,"r"); \
		if ((pexfile==NULL) ||(routefile==NULL)) {ConsoleMessage ("error opening temp file!"); return "";}}


	#define	CLOSE_PROTO_EXPAND_FILE {fclose (pexfile); fclose (routefile);}

	#define UNLINK_PROTO_EXPAND_FILE {unlink (tempname); unlink (tempRoutename);}

	#ifdef CPROTOVERBOSE
	printf ("start of protoExpand me %u me->lexer %u\n",me, me->lexer);
	#endif

	/* protoSize is zero, unless we are successful */
	*protoSize = 0;

	*thisProto = protoDefinition_copy(me->lexer, vector_get(struct ProtoDefinition*, me->PROTOs, nodeTypeU));
	#ifdef CPROTOVERBOSE
	printf ("expanding proto, "); 
	printf ("thisProto %u, me->curPROTO %u\n",(*thisProto),me->curPROTO);
	#endif

	OPEN_PROTO_EXPAND_FILE_WRITE

	APPEND_STARTPROTOGROUP_1

	/* copy the proto fields, so that we have either the defined field, or the field at invocation */

	#ifdef CPROTOVERBOSE
	printf ("getProtoInvocationFields being called\n");
	#endif
	getProtoInvocationFields(me,(*thisProto));

	#ifdef CPROTOVERBOSE
	printf ("dumpProtoFieldDeclarationNodes being called\n");
	#endif
	curstringlen += dumpProtoFieldDeclarationNodes(me->lexer, *thisProto, pexfile);

	#ifdef CPROTOVERBOSE
	printf ("APPEND_STARTGROUP_2 being called\n");
	#endif
	APPEND_STARTPROTOGROUP_2

	/* go through each part of this deconstructedProtoBody, and see what needs doing... */
	if ((*thisProto)->deconstructedProtoBody == NULL) return ""; /* nothing to do! */

	protoElementCount = vector_size((*thisProto)->deconstructedProtoBody);

	#ifdef CPROTOVERBOSE
		printf ("protoExpand - protoEpementCount %d\n",protoElementCount);
	#endif
	

	i = 0;
	while (i < protoElementCount) {
		/* get the current element */
		ele = vector_get(struct ProtoElementPointer*, (*thisProto)->deconstructedProtoBody, i);

		ASSERT(ele);
		#ifdef XXX
		PROTO_CAT ( "# at A\n");
		#endif

		#ifdef CPROTOVERBOSE
		printf ("PROTO - ele %d of %d; ",i, protoElementCount);
		if (ele->isNODE != -1) {
			printf (" NODE:\t%s",stringNodeType(ele->isNODE));
		}

		if (ele->isKEYWORD != -1) {
			printf (" KW:\t%s",stringKeywordType(ele->isKEYWORD));
		}

		if (ele->stringToken != NULL) {
			printf (" string\t:%s:",ele->stringToken);
		}

		if (ele->terminalSymbol != -1) {
			printf (" ts\t:%c",ele->terminalSymbol);
		}

		if (ele->fabricatedDef != -1) {
			printf (" def\t%d",ele->fabricatedDef);
		}
		printf ("\n");
		#endif

		/* this is a NODE, eg, "SphereSensor". If we need this DEFined because it contains an IS, then make the DEF */
		if (ele->isNODE != ID_UNDEFINED) {
			/* keep this entry around, to see if there is an outputOnly ISd field */
			lastNode = ele; 

			/* possibly this is a synthetic DEF for possible external IS routing */
			if (ele->fabricatedDef != ID_UNDEFINED) {
			#ifdef XXX
			PROTO_CAT ( "# at B\n");
			#endif
				PROTO_CAT ("DEF ");
				sprintf (thisID, "%s%d_",FABRICATED_DEF_HEADER,(int)(ele->fabricatedDef));
				APPEND_THISID
				APPEND_SPACE
			}
			#ifdef XXX
			PROTO_CAT ( "# at C\n");
			#endif
			APPEND_NODE
			APPEND_SPACE

		/* Maybe this is a KEYWORD, like "USE" or "DEFINE". if this is an "IS" DO NOT print it out */
		} else if (ele->isKEYWORD != ID_UNDEFINED) {
			if ((ele->isKEYWORD == KW_DEF) ||
			    (ele->isKEYWORD == KW_USE) ||
			    (ele->isKEYWORD == KW_IS) ||
			    (ele->isKEYWORD == KW_ROUTE) ||
			    (ele->isKEYWORD == KW_TO))
				lastKeyword = ele;

			if (ele->isKEYWORD != KW_IS) { 
				#ifdef XXX
				PROTO_CAT ( "# at D\n");
				#endif
				APPEND_KEYWORD APPEND_SPACE 
			}

		/* Hmmm - maybe this is a "{" or something like that */
		} else if (ele->terminalSymbol != ID_UNDEFINED) {
			#ifdef XXX
			PROTO_CAT ( "# at E\n");
			#endif

			APPEND_TERMINALSYMBOL

		/* nope, this is a fieldname, DEF name, or string, or something equivalent */
		} else if (ele->stringToken != NULL) {
			/* ok we have a name. Is the NEXT token an IS? If not, just continue */
			/* printf ("we have a normal stringToken - :%s:\n",ele->stringToken); */

			if (i<(protoElementCount-2)) {
				tempEle = vector_get(struct ProtoElementPointer*, (*thisProto)->deconstructedProtoBody, i+1);
				if ((tempEle != NULL) && (tempEle->isKEYWORD == KW_IS)) {
					size_t tl =100;
					char *newTl = MALLOC(tl);
					newTl[0] = '\0';

					/* printf ("next element is an IS \n"); 
					printf ("and, thisID is :%s:\n",thisID); */
					tempEle = vector_get(struct ProtoElementPointer*, (*thisProto)->deconstructedProtoBody, i+2);
					/* printf ("ok, so IS of :%s: is :%s:\n",ele->stringToken, tempEle->stringToken); */
					routeSize += addProtoUpdateRoute(me->lexer, routefile, ele->stringToken, tempEle->stringToken, thisID, *thisProto,i);

					replaceProtoField(me->lexer, *thisProto, tempEle->stringToken,&newTl,&tl);
					/* printf ("IS replacement is len %d, str :%s:\n",strlen(newTl), newTl);  */

					/* is there actually a value for this field?? */
					if SOMETHING_IN_ISVALUE {
						#ifdef XXX
						PROTO_CAT ( "# at F\n");
						#endif

						APPEND_STRINGTOKEN
						APPEND_SPACE
						APPEND_ISVALUE

					} else if (lastNode != NULL) {
						if (lastNode->isNODE == NODE_Script) {
						#ifdef XXX
						PROTO_CAT ( "# at G\n");
						#endif

						/* Script nodes NEED the fieldname, even if it is blank, so... */
						APPEND_STRINGTOKEN
						APPEND_SPACE
						}
					}

					i+=2; /* skip the IS and the field */
					FREE_IF_NZ(newTl);
				} else { 
					#ifdef XXX
					PROTO_CAT ( "# at H\n");
					#endif

					APPEND_EDITED_STRINGTOKEN
				}

			} else { 
				#ifdef XXX
				PROTO_CAT ( "# at I\n");
				#endif
				APPEND_EDITED_STRINGTOKEN
			}

			APPEND_SPACE
		} else {
			/* this is a blank proto... */
			/* ConsoleMessage ("PROTO EXPANSION, vector element %d, can not expand\n",i); */
		}

		/* go to the next token */
		i++;
	}

	/* finish things off, and close the file */
	PROTO_CAT ("] #End of PROTO Expansion children field\n");
	CLOSE_PROTO_EXPAND_FILE

	/* read in the expanded PROTO text, and return it. */

	newProtoText = MALLOC(sizeof (char) * (curstringlen + routeSize + strlen(ENDPROTOGROUP) + 10));
	newProtoText[0] = '\0';

        OPEN_PROTO_EXPAND_FILE_READ;

	{ int i;
		i=(int) fread(newProtoText,sizeof(char),curstringlen,pexfile);
		/* printf ("just read in %d, should be %d\n",i,curstringlen); */
		/* i should == curstringlen, btw */
		if ((i>0) && (i<=curstringlen)) newProtoText[i] = '\0';
	}
	{ int i;
		i=(int) fread(&newProtoText[curstringlen],sizeof(char),routeSize,routefile);
		/* printf ("just read in %d, should be %d\n",i,routeSize); */
		/* i should == routeSize, btw */
		if ((i>0) && (i<=routeSize)) newProtoText[i+curstringlen] = '\0';
	}

	strcat (newProtoText,ENDPROTOGROUP);

	

	CLOSE_PROTO_EXPAND_FILE;
	UNLINK_PROTO_EXPAND_FILE;


	#ifdef CPROTOVERBOSE
	printf ("so, newProtoText \n%s\n",newProtoText);
	#endif

	*protoSize = curstringlen + routeSize + (int) strlen(ENDPROTOGROUP);
	newProtoText[*protoSize] = '\0';

	return newProtoText;
}

/* for resolving ROUTEs to/from PROTOS... */
/* this is a PROTO; go through and find the node, and fill in the correct curID so that parsing can
   continue.  */

BOOL resolveProtoNodeField(struct VRMLParser *me, struct ProtoDefinition *Proto, char *fieldName, struct X3D_Node **Node) {
	indexT ret;
	char thisID[2000];

	#ifdef CPROTOVERBOSE
	printf ("resolveProtoNodeField, looking for field %s\n",me->lexer->curID);
	#endif

	/* make up the def name requested; this will be a unique name */
	sprintf (thisID,"PROTO_%p_%s",Proto,fieldName);

	
	/* ok, now we have a DEF name, lets look it up. */
	#ifdef CPROTOVERBOSE
	printf ("resolveProtoNodeField looking up :%s:\n",thisID);
	#endif

	lexer_specialID_string(me->lexer, NULL, &ret, NULL, 
		0, stack_top(struct Vector*, me->lexer->userNodeNames), thisID);
	#ifdef CPROTOVERBOSE
	printf ("resolveProtoNodeField after lookup, ret %d\n",ret);
	#endif

	if (ret != ID_UNDEFINED) {

       		/* If we're USEing it, it has to already be defined. */
        	ASSERT(ret!=ID_UNDEFINED);

        	/* It also has to be in the DEFedNodes stack */
        	ASSERT(me->DEFedNodes && !stack_empty(me->DEFedNodes) &&
        		ret<vector_size(stack_top(struct Vector*, me->DEFedNodes)));

        	/* Get a pointer to the X3D_Node structure for this DEFed node and return it in ret */
        	*Node = vector_get(struct X3D_Node*, stack_top(struct Vector*, me->DEFedNodes), ret);
		#ifdef CPROTOVERBOSE
		printf ("RETURNING; so, now the node of the DEF is %u, type %s\n",*Node, stringNodeType((*Node)->_nodeType));
		#endif
	} else {
		/* no luck here */
		#ifdef CPROTOVERBOSE
		printf  ("could not find encompassing DEF for PROTO field %s of node %s\n", me->lexer->curID,thisID);
		#endif
		ConsoleMessage ("could not find encompassing DEF for PROTO field %s", me->lexer->curID);
	}

	return (ret != ID_UNDEFINED);
}


/****************************************************************************

Utility functions for holding on to proto definitions. Note that in both VRML
classic and XML encoded, the protoDefinitions are kept around. This used to be
a pointer stuffed into an int; now it is a reference to here.

If you look, a group has a definition in it of:

FreeWRL__protoDef => [SFInt32, INT_ID_UNDEFINED,

so if it has a "FreeWRL__protoDef" that is != INT_ID_UNDEFINED, it is a valid
index here.

******************************************************************************/

static struct Vector *protoDefVec = NULL;
struct protoInsert {
		struct ProtoDefinition *vrmlProtoDef;
		int xmlProtoDef;
};

int newProtoDefinitionPointer (struct ProtoDefinition *vrmlpd, int xmlpd) {
	struct protoInsert *npd = MALLOC(sizeof (struct protoInsert));

	npd->vrmlProtoDef = vrmlpd;
	npd->xmlProtoDef = xmlpd;


	if (protoDefVec == NULL) {
		protoDefVec = newVector(struct protoInsert *, 16);
	}
	vector_pushBack (struct protoInsert*,protoDefVec, npd);	

	/* printf ("newProtoDefinitionPointer, have vrmlpd %u and xmlpd %d\n",vrmlpd,xmlpd);
	printf ("newProtoDefinitionPointer, returning %d\n",(int)(vector_size(protoDefVec)-1)); */

	return (int) (vector_size(protoDefVec)-1);
}

struct ProtoDefinition *getVRMLprotoDefinition (struct X3D_Group *me) {
	struct protoInsert *npd;
	int mpd;

	mpd = me->FreeWRL__protoDef;

	/* printf ("getVRMLprotoDefinition, looking for %d\n",mpd); */
	if (mpd == INT_ID_UNDEFINED) return NULL;
	if (mpd >= vector_size(protoDefVec)) {
		printf ("internal error, can not get proto def %d, out of bounds; vector size %d\n",mpd,vector_size(protoDefVec));
		return NULL;
	}
	/* printf ("getProtoDefinition, mpd %d, returning %u\n",mpd, vector_get(struct ProtoDefinition *,protoDefVec,mpd)); */
	npd = vector_get(struct protoInsert*,protoDefVec,mpd);
	return npd->vrmlProtoDef;
}
