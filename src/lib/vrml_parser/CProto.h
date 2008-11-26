/* CProto.h - this is the object representing a PROTO definition and being
 * capable of instantiating it.
 * 
 * We keep a vector of pointers to all that pointers which point to "inner
 * memory" and need therefore be updated when copying.  Such pointers include
 * field-destinations and parts of ROUTEs.  Those pointers are then simply
 * copied, their new positions put in the new vector, and afterwards are all
 * pointers there updated.
 */

#ifndef CPROTO_H
#define CPROTO_H

#include "headers.h"

#include "CParseGeneral.h"
#include "Vector.h"
#include "CScripts.h"

struct PointerHash;
struct VRMLParser;

/* ************************************************************************** */
/* ******************************** OffsetPointer *************************** */
/* ************************************************************************** */

/* A pointer which is made up of the offset/node pair */
struct OffsetPointer
{
 struct X3D_Node* node;
 unsigned ofs;
};

/* Constructor/destructor */
struct OffsetPointer* newOffsetPointer(struct X3D_Node*, unsigned);
#define offsetPointer_copy(me) \
 newOffsetPointer((me)->node, (me)->ofs)
#define deleteOffsetPointer(me) \
 FREE_IF_NZ(me)

/* Dereference to simple pointer */
#define offsetPointer_deref(t, me) \
 ((t)(((char*)((me)->node))+(me)->ofs))

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
struct ProtoElementPointer* newProtoElementPointer(void);

#define deleteProtoElementPointer(me) \
 {FREE_IF_NZ(me->stringToken); FREE_IF_NZ(me);}

struct ProtoElementPointer *copyProtoElementPointer(struct ProtoElementPointer *);

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
 
#ifdef OLDDEST
#define protoFieldDecl_getDestinationCount(me) \
 vector_size((me)->dests)
#define protoFieldDecl_getDestination(me, i) \
 vector_get(struct OffsetPointer*, (me)->dests, i)
#endif


#define protoFieldDecl_getDefaultValue(me) \
 ((me)->defaultVal)


/* Add a destination this field's value must be assigned to */
#ifdef OLDDEST

#define protoFieldDecl_addDestinationOptr(me, optr) \
 vector_pushBack(struct OffsetPointer*, me->dests, optr)
#define protoFieldDecl_addDestination(me, n, o) \
 protoFieldDecl_addDestinationOptr(me, newOffsetPointer(n, o))
#endif


/* Sets this field's value (copy to destinations) */
void protoFieldDecl_setValue(struct VRMLLexer*, struct ProtoFieldDecl*, union anyVrml*);

/* Build a ROUTE from/to this field */
void protoFieldDecl_routeTo(struct ProtoFieldDecl*,
 struct X3D_Node*, unsigned, int dir, struct VRMLParser*);
void protoFieldDecl_routeFrom(struct ProtoFieldDecl*,
 struct X3D_Node*, unsigned, int dir, struct VRMLParser*);

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
struct ProtoDefinition* protoDefinition_copy(struct VRMLLexer*, struct ProtoDefinition*);

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

void getEquivPointer(struct OffsetPointer* origPointer, struct OffsetPointer* ret, struct X3D_Node* origProtoNode, struct X3D_Node* curProtoNode);
void getProtoInvocationFields(struct VRMLParser *me, struct ProtoDefinition *thisProto);
struct ProtoFieldDecl* getProtoFieldDeclaration(struct VRMLLexer *me, struct ProtoDefinition *thisProto, char *thisID);
void tokenizeProtoBody(struct ProtoDefinition *, char *);
char *protoExpand (struct VRMLParser *me, indexT nodeTypeU, struct ProtoDefinition **thisProto);
BOOL resolveProtoNodeField(struct VRMLParser *me, struct ProtoDefinition *Proto, struct X3D_Node **Node);

#endif /* Once-check */
