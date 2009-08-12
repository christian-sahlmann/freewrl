/* 
=INSERT_TEMPLATE_HERE=

$Id: CParseParser.h,v 1.14 2009/08/12 22:15:04 crc_canada Exp $

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

/* Parses SF* field values */
#define parser_sffloatValue(me, ret) \
 lexer_float(me->lexer, ret)
#define parser_sfint32Value(me, ret) \
 lexer_int32(me->lexer, ret)
#define parser_sfstringValue(me, ret) \
 lexer_string(me->lexer, ret)
#define lexer_sfstringValue(me, ret) \
 lexer_string(me, ret)

/* Initializes node-specific fields */
void parser_specificInitNode(struct X3D_Node*, struct VRMLParser*);

/* Registers a ROUTE, in current PROTO or scene */
void parser_registerRoute(struct VRMLParser*,
 struct X3D_Node*, int, struct X3D_Node*, int, size_t);

BOOL parseType(struct VRMLParser* me, indexT type,   union anyVrml *defaultVal);


void replaceProtoField(struct VRMLLexer *me, struct ProtoDefinition *thisProto, char *thisID, char **outTextPtr, int *outSize);

void cParseErrorCurID(struct VRMLParser *me, char *str);
void cParseErrorFieldString(struct VRMLParser *me, char *str1, const char *str2);

#define CPARSE_ERROR_CURID(str) cParseErrorCurID(me, str);
#define CPARSE_ERROR_FIELDSTRING(str1,str2) cParseErrorFieldString(me, str1, str2);

/* Main parsing routine, parses the start symbol (vrmlScene) */
BOOL parser_vrmlScene(struct VRMLParser*);


#endif /* __FREEWRL_CPARSE_PARSER_H__ */
