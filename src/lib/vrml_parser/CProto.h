/*
=INSERT_TEMPLATE_HERE=

$Id: CProto.h,v 1.12 2009/10/05 15:07:24 crc_canada Exp $

CProto.h - this is the object representing a PROTO definition and being
capable of instantiating it.
 
We keep a vector of pointers to all that pointers which point to "inner
memory" and need therefore be updated when copying.  Such pointers include
field-destinations and parts of ROUTEs.  Those pointers are then simply
copied, their new positions put in the new vector, and afterwards are all
pointers there updated.

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


#ifndef __FREEWRL_CPROTO_H__
#define __FREEWRL_CPROTO_H__


struct PointerHash;
struct VRMLParser;

/* ************************************************************************** */
/* ************************** ProtoElementPointer *************************** */
/* ************************************************************************** */

/* A pointer which is made up of the offset/node pair */
struct ProtoElementPointer
{
	char *stringToken; 	/* pointer to a name, etc. NULL if one of the index_t fields is in use */
	indexT isNODE;		/* NODES index, if found, ID_UNDEFINED  otherwise */
	indexT isKEYWORD;	/* KEYWORDS index, if found, ID_UNDEFINED otherwise */ 
	indexT terminalSymbol;	/* ASCII value of ".", "{", "}", "[", "]", ":", ID_UNDEFINED otherwise */
	indexT fabricatedDef;	/* for making a unique DEF name */
};

/* Constructor/destructor */
#define deleteProtoElementPointer(me) \
 {FREE_IF_NZ(me->stringToken); FREE_IF_NZ(me);}

#define ASSIGN_UNIQUE_ID(me) \
	{me->fabricatedDef = nextFabricatedDef; nextFabricatedDef ++; }

#define FABRICATED_DEF_HEADER "fReEwEL_fAbricatio_dEF_" /* hopefully quite unique! */

/* ************************************************************************** */
/* ********************************* ProtoFieldDecl ************************* */
/* ************************************************************************** */

/* The object */
struct ProtoFieldDecl
{
 indexT mode; /* field, exposedField, eventIn, eventOut */
 indexT type; /* field type */
 indexT name; /* field "name" (its lexer-index) */
 char *fieldString; /* the field, in ascii form */
 #ifdef OLDDEST
/* This is the list of desination pointers for this field */
 struct Vector* dests; 
#endif

 /* Only for exposedField or field */
 BOOL alreadySet; /* Has the value already been set? */
 union anyVrml defaultVal; /* Default value */
 /* Script fields */
 struct Vector* scriptDests;
};

/* Constructor and destructor */
struct ProtoFieldDecl* newProtoFieldDecl(indexT, indexT, indexT);
void deleteProtoFieldDecl(struct ProtoFieldDecl*);

/* Copies */
struct ProtoFieldDecl* protoFieldDecl_copy(struct VRMLLexer*, struct ProtoFieldDecl*);

/* Accessors */
#define protoFieldDecl_getType(me) \
 ((me)->type)
#define protoFieldDecl_getAccessType(me) \
 ((me)->mode)
#define protoFieldDecl_getIndexName(me) \
 ((me)->name)
#define protoFieldDecl_getStringName(lex, me) \
 lexer_stringUser_fieldName(lex, protoFieldDecl_getIndexName(me), \
  protoFieldDecl_getAccessType(me))
 

#define protoFieldDecl_getDefaultValue(me) \
 ((me)->defaultVal)


/* Sets this field's value (copy to destinations) */
void protoFieldDecl_setValue(struct VRMLLexer*, struct ProtoFieldDecl*, union anyVrml*);

#ifdef OLDCODE
/* Build a ROUTE from/to this field */
void protoFieldDecl_routeTo(struct ProtoFieldDecl*,
 struct X3D_Node*, unsigned, int dir, struct VRMLParser*);
void protoFieldDecl_routeFrom(struct ProtoFieldDecl*,
 struct X3D_Node*, unsigned, int dir, struct VRMLParser*);
#endif

/* Finish this field - if value is not yet set, use default. */
#define protoFieldDecl_finish(lex, me) \
 if(((me)->mode==PKW_initializeOnly || (me)->mode==PKW_inputOutput) && \
  !(me)->alreadySet) \
  protoFieldDecl_setValue(lex, me, &(me)->defaultVal)

/* Add inner pointers' pointers to the vector */
void protoFieldDecl_addInnerPointersPointers(struct ProtoFieldDecl*,
 struct Vector*);

/* ************************************************************************** */
/* ******************************* ProtoRoute ******************************* */
/* ************************************************************************** */

/* A ROUTE defined inside a PROTO block. */
struct ProtoRoute
{
 struct X3D_Node* from;
 struct X3D_Node* to;
 int fromOfs;
 int toOfs;
 size_t len;
 int dir;
};

/* Constructor and destructor */
struct ProtoRoute* newProtoRoute(struct X3D_Node*, int, struct X3D_Node*, int,
 size_t, int);
#define protoRoute_copy(me) \
 newProtoRoute((me)->from, (me)->fromOfs, (me)->to, (me)->toOfs, \
 (me)->len, (me)->dir)
#define deleteProtoRoute(me) \
 FREE_IF_NZ(me)

/* Register this route */
#define protoRoute_register(me) \
 CRoutes_RegisterSimple((me)->from, (me)->fromOfs, (me)->to, (me)->toOfs, \
 (me)->len, (me)->dir)

/* Add this one's inner pointers to the vector */
#define protoRoute_addInnerPointersPointers(me, vec) \
 { \
  vector_pushBack(void**, vec, &(me)->from); \
  vector_pushBack(void**, vec, &(me)->to); \
 }

/* ************************************************************************** */
/* ****************************** ProtoDefinition *************************** */
/* ************************************************************************** */

/* The object */
struct ProtoDefinition
{
 indexT protoDefNumber;	/* unique sequence number */
 struct Vector* iface; /* The ProtoFieldDecls making up the interface */
 struct Vector* deconstructedProtoBody; /* PROTO body tokenized */
 int estimatedBodyLen; /* an estimate of the expanded proto body size, to give us an output string len */
 char *protoName;      /* proto name as a string - used in EAI calls */
 int isCopy;		/* is this the original or a copy? the original keeps the deconstructedProtoBody */
};

/* Constructor and destructor */
struct ProtoDefinition* newProtoDefinition();
void deleteProtoDefinition(struct ProtoDefinition*);

/* Adds a field declaration to the interface */
#define protoDefinition_addIfaceField(me, field) \
 vector_pushBack(struct ProtoFieldDecl*, (me)->iface, field)

/* Get fields by indices */
#define protoDefinition_getFieldCount(me) \
 vector_size((me)->iface)
#define protoDefinition_getFieldByNum(me, i) \
 vector_get(struct ProtoFieldDecl*, (me)->iface, i)

/* Retrieves a field declaration of this PROTO */
struct ProtoFieldDecl* protoDefinition_getField(struct ProtoDefinition*, 
 indexT, indexT);

/* Copies a ProtoDefinition, so that we can afterwards fill in field values */
/* struct ProtoDefinition* protoDefinition_copy(struct VRMLLexer* lex, struct ProtoDefinition* me); */

/* Extracts the scene graph out of a ProtoDefinition */
struct X3D_Group* protoDefinition_extractScene(struct VRMLLexer* lex, struct ProtoDefinition*);

/* Does a recursively deep copy of a node-tree */
struct X3D_Node* protoDefinition_deepCopy(struct VRMLLexer*, struct X3D_Node*,
 struct ProtoDefinition*, struct PointerHash*);

/* ************************************************************************** */
/* ******************************* PointerHash ****************************** */
/* ************************************************************************** */

/* A hash table used to check whether a specific pointer has already been
 * copied.  Otherwise we can't keep things like multiple references to the same
 * node when copying. */

/* An entry */
struct PointerHashEntry
{
 struct X3D_Node* original;
 struct X3D_Node* copy;
};

/* The object */
struct PointerHash
{
 #define POINTER_HASH_SIZE	4321
 struct Vector* data[POINTER_HASH_SIZE];
};

struct PointerHash* newPointerHash();
void deletePointerHash(struct PointerHash*);

/* Query the hash */
struct X3D_Node* pointerHash_get(struct PointerHash*, struct X3D_Node*);

/* Add to the hash */
void pointerHash_add(struct PointerHash*, struct X3D_Node*, struct X3D_Node*);

/* JAS - make a copy of a script in a PROTO, and give it a new number */
void registerScriptInPROTO (struct X3D_Script *scr,struct ProtoDefinition* new);

/* This structure holds information about a nested proto reference. That is to say, when a we have an instantiation
   of a proto within another proto definition, and two user defined fields are linked to each other in an IS statement.
   PROTO Proto1 [
	field SFFloat myvalue 0.1
  ] {
	DEF MAT Material { shininess IS myvalue }	
  }
  PROTO Proto2 [
	field SFFloat secondvalue 0.5
  ] { 
	DEF MAT2 Proto1 { myvalue IS secondvalue } 
  {

  In this case, we need the dests for myvalue must be replicated for secondvalue, but the references to nodes must be for the nodes in the proto expansion of Proto1, 
  not references to nodes in Proto1 itself.  We accomplish this by copying the dests list for the proto field after the proto has been expanded.

  The NestedProtoFields structure is used when parsing a nested proto expansion in order to keep track of all instances of linked user defined fields so that the dests
  lists may be adjusted appropriately when parsing is finalised for the node. */
struct NestedProtoField 
{
   struct ProtoFieldDecl* origField;
   struct ProtoFieldDecl* localField;
};

void getProtoInvocationFields(struct VRMLParser *me, struct ProtoDefinition *thisProto);
struct ProtoFieldDecl* getProtoFieldDeclaration(struct VRMLLexer *me, struct ProtoDefinition *thisProto, char *thisID);
void tokenizeProtoBody(struct ProtoDefinition *, char *);
char *protoExpand (struct VRMLParser *me, indexT nodeTypeU, struct ProtoDefinition **thisProto, int *protoSize);
BOOL resolveProtoNodeField(struct VRMLParser *me, struct ProtoDefinition *Proto, char * thisField, struct X3D_Node **Node);

int newProtoDefinitionPointer (struct ProtoDefinition *vrmlnpd, int xmlpd); 
struct ProtoDefinition *getVRMLprotoDefinition (struct X3D_Group *me);
void kill_ProtoDefinitionTable (void);
int getXMLprotoDefinition (struct X3D_Group *me);


#endif /* __FREEWRL_CPROTO_H__ */
