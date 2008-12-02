/* header file for the X3D parser, only items common between the X3DParser files should be here. */

#include "expat.h"

#define PARSING_NODES 1
#define PARSING_SCRIPT 2
#define PARSING_PROTODECLARE  3
#define PARSING_PROTOINTERFACE  4
#define PARSING_PROTOBODY       5
#define PARSING_PROTOINSTANCE   6
#define PARSING_IS		7
#define PARSING_CONNECT		8

/* for our internal PROTO tables, and, for initializing the XML parser */
#define PROTOINSTANCE_MAX_LEVELS 30
#define PROTOINSTANCE_MAX_PARAMS 20

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
extern int parserMode;

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
void registerX3DScriptField(int myScriptNumber,int type,int kind, int myFieldOffs, char *name, char *value);
void parseProtoInstance (const char **atts);
void parseProtoInstanceFields(const char *name, const char **atts);
void dumpProtoBody (const char *name, const char **atts);
void dumpCDATAtoProtoBody (char *str);
void endDumpProtoBody (const char *name);
void parseScriptProtoField(const char **atts);
int getFieldFromScript (char *fieldName, int scriptno, int *offs, int *type, int *accessType);
void expandProtoInstance(struct X3D_Group * myGroup);
void freeProtoMemory (void);
void kill_X3DProtoScripts(void);