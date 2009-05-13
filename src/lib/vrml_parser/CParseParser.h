/* 
=INSERT_TEMPLATE_HERE=

$Id: CParseParser.h,v 1.8 2009/05/13 13:53:56 crc_canada Exp $

Parser (input of non-terminal symbols) for CParse

*/

#ifndef __FREEWRL_CPARSE_PARSER_H__
#define __FREEWRL_CPARSE_PARSER_H__

void resetParseSuccessfullyFlag(void);
int parsedSuccessfully(void);

#ifdef REWIRE
#define BOOL int
#include "../../libeai/EAI_C.h"
#endif

struct ProtoDefinition;
struct ProtoFieldDecl;
struct Shader_Script;
struct OffsetPointer;


#define BLOCK_STATEMENT(LOCATION) \
   if(parser_routeStatement(me))  { \
	continue; \
   } \
 \
  if (parser_componentStatement(me)) { \
	continue; \
  } \
 \
  if (parser_exportStatement(me)) { \
	continue; \
  } \
 \
  if (parser_importStatement(me)) { \
	continue; \
  } \
 \
  if (parser_metaStatement(me)) { \
	continue; \
  } \
 \
  if (parser_profileStatement(me)) { \
	continue; \
  } 

/* This is our parser-object. */
struct VRMLParser
{
 struct VRMLLexer* lexer;	/* The lexer used. */
 /* Where to put the parsed nodes? */
 void* ptr;
 unsigned ofs;
 /* Currently parsing a PROTO? */
 struct ProtoDefinition* curPROTO;

 /* This is the DEF/USE memory. */
 Stack* DEFedNodes;

 /* This is for PROTOs -- not stacked, as explained in CParseLexer.h */
 struct Vector* PROTOs;

	/* which format some field strings will be in - XML and "classic" VRML are different */
	int parsingX3DfromXML;
};

/* Functions parsing a type by its index */
extern BOOL (*PARSE_TYPE[])(struct VRMLParser*, void*);

/* Constructor and destructor */
struct VRMLParser* newParser(void*, unsigned, int isX3DFormat);
struct VRMLParser* reuseParser(void*, unsigned);
void deleteParser(struct VRMLParser*);

/* Other clean up */
void parser_destroyData(struct VRMLParser*);

/* Scoping */
void parser_scopeIn(struct VRMLParser*);
void parser_scopeOut(struct VRMLParser*);

/* Sets parser's input */
#define parser_fromString(me, str) \
 lexer_fromString(me->lexer, str)

/* Parses MF* field values */
BOOL parser_mfboolValue(struct VRMLParser*, struct Multi_Bool*);
BOOL parser_mfcolorValue(struct VRMLParser*, struct Multi_Color*);
BOOL parser_mfcolorrgbaValue(struct VRMLParser*, struct Multi_ColorRGBA*);
BOOL parser_mffloatValue(struct VRMLParser*, struct Multi_Float*);
BOOL parser_mfint32Value(struct VRMLParser*, struct Multi_Int32*);
BOOL parser_mfnodeValue(struct VRMLParser*, struct Multi_Node*);
BOOL parser_mfrotationValue(struct VRMLParser*, struct Multi_Rotation*);
BOOL parser_mfstringValue(struct VRMLParser*, struct Multi_String*);
BOOL parser_mftimeValue(struct VRMLParser*, struct Multi_Time*);
BOOL parser_mfvec2fValue(struct VRMLParser*, struct Multi_Vec2f*);
BOOL parser_mfvec3fValue(struct VRMLParser*, struct Multi_Vec3f*);
BOOL parser_mfvec3dValue(struct VRMLParser*, struct Multi_Vec3d*);

/* Parses SF* field values */
BOOL parser_sfboolValue(struct VRMLParser*, vrmlBoolT*);
BOOL parser_sfcolorValue(struct VRMLParser*, vrmlColorT*);
BOOL parser_sfcolorrgbaValue(struct VRMLParser*, vrmlColorRGBAT*);
BOOL parser_sffloatValue_(struct VRMLParser*, vrmlFloatT*);
#define parser_sffloatValue(me, ret) \
 lexer_float(me->lexer, ret)
BOOL parser_sfimageValue(struct VRMLParser*, vrmlImageT*);
BOOL parser_sfint32Value_(struct VRMLParser*, vrmlInt32T*);
#define parser_sfint32Value(me, ret) \
 lexer_int32(me->lexer, ret)
BOOL parser_sfnodeValue(struct VRMLParser*, vrmlNodeT*);
BOOL parser_sfrotationValue(struct VRMLParser*, vrmlRotationT*);
BOOL parser_sfstringValue_(struct VRMLParser*, vrmlStringT*);
#define parser_sfstringValue(me, ret) \
 lexer_string(me->lexer, ret)
#define lexer_sfstringValue(me, ret) \
 lexer_string(me, ret)
BOOL parser_sftimeValue(struct VRMLParser*, vrmlTimeT*);
BOOL parser_sfvec2fValue(struct VRMLParser*, vrmlVec2fT*);
BOOL parser_sfvec2dValue(struct VRMLParser*, vrmlVec2dT*);
BOOL parser_sfvec3dValue(struct VRMLParser*, vrmlVec3dT*);
#define parser_sfvec3fValue(me, ret) \
 parser_sfcolorValue(me, ret)
BOOL parser_sfvec4fValue(struct VRMLParser*, vrmlVec4fT*);
BOOL parser_sfvec4dValue(struct VRMLParser*, vrmlVec4dT*);
BOOL parser_sfmatrix3fValue(struct VRMLParser *, vrmlMatrix3fT*);
BOOL parser_sfmatrix3dValue(struct VRMLParser *, vrmlMatrix3dT*);
BOOL parser_sfmatrix4fValue(struct VRMLParser *, vrmlMatrix4fT*);
BOOL parser_sfmatrix4dValue(struct VRMLParser *, vrmlMatrix4dT*);


/* Parses nodes, fields and other statements. */
BOOL parser_routeStatement(struct VRMLParser*);
BOOL parser_componentStatement(struct VRMLParser*);
BOOL parser_exportStatement(struct VRMLParser*);
BOOL parser_importStatement(struct VRMLParser*);
BOOL parser_metaStatement(struct VRMLParser*);
BOOL parser_profileStatement(struct VRMLParser*);

BOOL parser_protoStatement(struct VRMLParser*);
BOOL parser_interfaceDeclaration(struct VRMLParser*,
 struct ProtoDefinition*, struct Shader_Script*);
BOOL parser_nodeStatement(struct VRMLParser*, vrmlNodeT*);
BOOL parser_node(struct VRMLParser*, vrmlNodeT*, indexT);
BOOL parser_field(struct VRMLParser*, struct X3D_Node*);
/* JAS BOOL parser_protoEvent(struct VRMLParser*, struct ProtoDefinition*, struct ProtoDefinition*); */

/* Initializes node-specific fields */
void parser_specificInitNode(struct X3D_Node*, struct VRMLParser*);

/* Registers a ROUTE, in current PROTO or scene */
void parser_registerRoute(struct VRMLParser*,
 struct X3D_Node*, unsigned, struct X3D_Node*, unsigned, size_t, int);

/* Parses a field value of a certain type (literally or IS) */
BOOL parser_fieldValue(struct VRMLParser* me, struct X3D_Node *node, int offs,
                       indexT type, indexT origFieldE, BOOL protoExpansion, struct ProtoDefinition* pdef, struct ProtoFieldDecl* origField);


/* Main parsing routine, parses the start symbol (vrmlScene) */
BOOL parser_vrmlScene(struct VRMLParser*);

BOOL parseType(struct VRMLParser* me, indexT type,   union anyVrml *defaultVal);


void replaceProtoField(struct VRMLLexer *me, struct ProtoDefinition *thisProto, char *thisID, char **outTextPtr, int *outSize);

void cParseErrorCurID(struct VRMLParser *me, char *str);
void cParseErrorFieldString(struct VRMLParser *me, char *str1, const char *str2);

#define CPARSE_ERROR_CURID(str) cParseErrorCurID(me, str);
#define CPARSE_ERROR_FIELDSTRING(str1,str2) cParseErrorFieldString(me, str1, str2);



#endif /* __FREEWRL_CPARSE_PARSER_H__ */
