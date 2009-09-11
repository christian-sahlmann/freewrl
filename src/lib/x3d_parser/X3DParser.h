/*
=INSERT_TEMPLATE_HERE=

$Id: X3DParser.h,v 1.11 2009/09/11 19:13:10 crc_canada Exp $

X3D parser functions.

*/

#ifndef __FREEWRL_X3D_PARSER_H__
#define __FREEWRL_X3D_PARSER_H__

/* attributes get put into this structure */
struct nameValuePairs {
        char *fieldName;
        char *fieldValue;
};


/* header file for the X3D parser, only items common between the X3DParser files should be here. */

/* FIXME: This should be avoided -- including "expat.h" */
/*#define X3DPARSERVERBOSE 1*/
#define PARSING_NODES 1
#define PARSING_SCRIPT 2
#define PARSING_PROTODECLARE  3
#define PARSING_PROTOINTERFACE  4
#define PARSING_PROTOBODY       5
#define PARSING_PROTOINSTANCE   6
#define PARSING_IS		7
#define PARSING_CONNECT		8

/* for our internal PROTO tables, and, for initializing the XML parser */
#define PROTOINSTANCE_MAX_LEVELS 50
#define PROTOINSTANCE_MAX_PARAMS 20

#define FREEWRL_SPECIFIC "FrEEWrL_pRotto"

#define DECREMENT_PARENTINDEX \
        if (parentIndex > 0) parentIndex--; else ConsoleMessage ("X3DParser, line %d stack underflow",LINE);

#define INCREMENT_PARENTINDEX \
        if (parentIndex < (PARENTSTACKSIZE-2))  { \
                parentIndex++; \
                parentStack[parentIndex] = NULL; /* make sure we know the state of the new Top of Stack */ \
        } else ConsoleMessage ("X3DParser, line %d stack overflow",LINE);



int freewrl_XML_GetCurrentLineNumber();
#define LINE freewrl_XML_GetCurrentLineNumber()
#define TTY_SPACE {int tty; printf ("%3d ",parentIndex); for (tty = 0; tty < parentIndex; tty++) printf ("  ");}
extern int getParserMode(void);
extern void debugsetParserMode(int,char*, int);
#define setParserMode(xxx) debugsetParserMode(xxx,__FILE__,__LINE__)

#define PARENTSTACKSIZE 256
extern int parentIndex;
extern struct X3D_Node *parentStack[PARENTSTACKSIZE];
extern char *CDATA_Text;
extern int CDATA_Text_curlen;


/* function protos */
struct X3D_Node *DEFNameIndex (const char *name, struct X3D_Node* node, int force);

void parseProtoDeclare (const char **atts);
void parseProtoInterface (const char **atts);
void parseProtoBody (const char **atts);
void parseProtoInstance (const char **atts);
void parseProtoInstanceFields(const char *name, const char **atts);
void dumpProtoBody (const char *name, const char **atts);
void dumpCDATAtoProtoBody (char *str);
void parseScriptProtoField(struct VRMLLexer *, const char **atts);
int getFieldFromScript (struct VRMLLexer* myLexer, char *fieldName, struct Shader_Script *, int *offs, int *type, int *accessType);
void expandProtoInstance(struct VRMLLexer *, struct X3D_Group * myGroup);
void freeProtoMemory (void);
void kill_X3DProtoScripts(void);
void linkNodeIn(char *, int);
void parseConnect(struct VRMLLexer * myLexer, const char **atts, struct Vector *tos);
void endConnect();
struct X3D_Node *X3DParser_getNodeFromName(const char *name);
int getRoutingInfo (struct VRMLLexer *myLexer, struct X3D_Node *node, int *offs, int* type, int *accessType, char *name, int routeTo);
char *X3DParser_getNameFromNode(struct X3D_Node* myNode);

#endif /*  __FREEWRL_X3D_PARSER_H__ */
