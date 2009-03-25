#ifdef AQUA
#include <sys/types.h>
#include <stdint.h>
#include "../input/EAIheaders.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define STRLEN 256

typedef struct { int type; int value; } _intX3D_SFBool;
typedef struct { int type; float value; } _intX3D_SFFloat;
typedef struct { int type; double value; } _intX3D_SFTime;
typedef struct { int type; int value; } _intX3D_SFInt32;
typedef struct { int type; uintptr_t *adr; } _intX3D_SFNode;
typedef struct { int type; float r[4]; } _intX3D_SFRotation;
typedef struct { int type; float c[2]; } _intX3D_SFVec2f;
typedef struct { int type; double c[2]; } _intX3D_SFVec2d;
typedef struct { int type; float c[3]; } _intX3D_SFColor;
typedef struct { int type; float c[3]; } _intX3D_SFVec3f;
typedef struct { int type; double c[3]; } _intX3D_SFVec3d;
typedef struct { int type; float r[4]; } _intX3D_SFColorRGBA;
typedef struct { int type; int len; char *strptr;} _intX3D_SFString;
typedef struct { int type; int len; char *strptr;} _intX3D_SFImage;

typedef struct { int type; int n; _intX3D_SFColor *p; } _intX3D_MFColor;
typedef struct { int type; int n; _intX3D_SFColorRGBA *p; } _intX3D_MFColorRGBA;
typedef struct { int type; int n; _intX3D_SFFloat *p; } _intX3D_MFFloat;
typedef struct { int type; int n; _intX3D_SFTime *p; } _intX3D_MFTime;
typedef struct { int type; int n; _intX3D_SFRotation *p; } _intX3D_MFRotation;
typedef struct { int type; int n; _intX3D_SFVec3d *p; } _intX3D_MFVec3d;
typedef struct { int type; int n; _intX3D_SFVec2d *p; } _intX3D_MFVec2d;
typedef struct { int type; int n; _intX3D_SFVec3f *p; } _intX3D_MFVec3f;
typedef struct { int type; int n; _intX3D_SFVec2f *p; } _intX3D_MFVec2f;
typedef struct { int type; int n; _intX3D_SFBool *p; } _intX3D_MFBool;
typedef struct { int type; int n; _intX3D_SFInt32 *p; } _intX3D_MFInt32;
typedef struct { int type; int n; _intX3D_SFNode *p; } _intX3D_MFNode;
typedef struct { int type; int n; _intX3D_SFString *p; } _intX3D_MFString;
typedef struct { int type; int n; _intX3D_SFImage *p; } _intX3D_MFImage;

typedef union _X3DNode {
	int 			type;
	_intX3D_MFBool		X3D_MFBool;
	_intX3D_SFBool		X3D_SFBool;
	_intX3D_SFFloat		X3D_SFFloat;
	_intX3D_SFTime		X3D_SFTime;
	_intX3D_SFInt32		X3D_SFInt32;
	_intX3D_MFColor 	X3D_MFColor;
	_intX3D_MFColorRGBA	X3D_MFColorRGBA;
	_intX3D_SFString	X3D_SFString;
	_intX3D_SFNode		X3D_SFNode;
	_intX3D_SFRotation	X3D_SFRotation;
	_intX3D_SFVec2f		X3D_SFVec2f;
	_intX3D_SFVec2d		X3D_SFVec2d;
	_intX3D_SFColor		X3D_SFColor;
	_intX3D_SFColor		X3D_SFVec3f;
	_intX3D_SFVec3d		X3D_SFVec3d;
	_intX3D_SFColorRGBA	X3D_SFColorRGBA;
	_intX3D_MFFloat		X3D_MFFloat;
	_intX3D_MFTime		X3D_MFTime;
	_intX3D_MFInt32		X3D_MFInt32;
	_intX3D_MFString	X3D_MFString;
	_intX3D_MFNode		X3D_MFNode;
	_intX3D_MFRotation	X3D_MFRotation;
	_intX3D_MFVec2f		X3D_MFVec2f;
	_intX3D_MFVec3f		X3D_MFVec3f;
	_intX3D_MFImage		X3D_MFImage;
	_intX3D_MFVec3d		X3D_MFVec3d;
	
} X3DNode;

#ifndef TRUE
#define TRUE 1==1
#define FALSE 1!=1
#endif

#define REMOVE_EOT {char *lp; lp=strstr(ptr,"RE_EOT"); if (lp!=NULL) {lp--; *lp='\0';}};
#define SKIP_IF_GT_SPACE        while (*ptr > ' ') ptr++;
#define SKIP_CONTROLCHARS       while ((*ptr != '\0') && (*ptr <= ' ')) ptr++;


/* structures */


struct _intX3DEventIn {
	uintptr_t	nodeptr;
	int 		offset;
	int		datatype;
	int 		datasize;
	int		scripttype;
	char 		*field;
};

#define X3DEventIn struct _intX3DEventIn
#define X3DEventOut struct _intX3DEventIn
/* single value structures */


X3DNode *X3D_getNode (char *name);
X3DEventIn *X3D_getEventIn(X3DNode *node, char *name);
X3DEventOut *X3D_getEventOut(X3DNode *node, char *name);
void X3D_setValue (X3DEventIn *dest, X3DNode *node);
void X3D_addRoute (X3DEventOut *from, X3DEventIn *to);
void X3D_deleteRoute (X3DEventOut *from, X3DEventIn *to);


/* initialize, shutdown public methods */
void X3D_initialize(char *);
void X3D_shutdown();
void freewrlReadThread(void);
void freewrlSwigThread(void);

/* float public methods */
float X3D_getCurrentSpeed();
float X3D_getCurrentFrameRate();

/* null public methods */
void X3D_firstViewpoint();
void X3D_lastViewpoint();
void X3D_nextViewpoint();
void X3D_previousViewpoint();
void X3D_setDescription(char *newDescription);

/* string return val public methods */
char *X3D_getDescription();
char *X3D_getName();
char *X3D_getVersion();
char *X3D_getWorldURL();

/* MFNode public methods */
X3DNode *X3D_createVrmlFromString(char *str);
X3DNode *X3D_newSFVec3f (float a, float b, float c);
X3DNode *X3D_newSFColor (float a, float b, float c);
X3DNode *X3D_newSFVec2f (float a, float b);
X3DNode *X3D_newSFRotation (float a, float b,float c, float d);
X3DNode *X3D_newSFColorRGBA (float a, float b,float c, float d);
X3DNode *X3D_newSFVec3d (double a, double b,double c);
X3DNode *X3D_newSFVec2d (double a, double b);
X3DNode *X3D_newSFBool (int a);
X3DNode *X3D_newSFFloat (float a);
X3DNode *X3D_newSFTime (double a);
X3DNode *X3D_newSFInt32 (int a);
X3DNode *X3D_newSFString();
X3DNode *X3D_newSFNode();
X3DNode *X3D_newSFImage();
X3DNode *X3D_newMFColor();
X3DNode *X3D_newMFFloat();
X3DNode *X3D_newMFTime();
X3DNode *X3D_newMFInt32();
X3DNode *X3D_newMFString();
X3DNode *X3D_newMFNode();
X3DNode *X3D_newMFRotation();
X3DNode *X3D_newMFVec2f();
X3DNode *X3D_newMFVec3f();
X3DNode *X3D_newMFColorRGBA();
X3DNode *X3D_newMFBool();
X3DNode *X3D_newMFVec3d();
X3DNode *X3D_newMFVec2d();

extern int _X3D_queryno;
extern int _X3D_FreeWRL_FD;
extern int _X3D_FreeWRL_Swig_FD;
extern int isSwig;
int _X3D_countWords(char *ptr);
char *_X3D_make1StringCommand (char command, char *name);
char *_X3D_make2StringCommand (char command, char *str1, char *str2);
char *_X3D_Browser_SendEventType(uintptr_t *adr,char *name, char *evtype);
char *_X3D_makeShortCommand (char command);
void _X3D_sendEvent (char command, char *string);
void _handleFreeWRLcallback(char *command);

void X3D_error(char *msg);
char *fieldTypeName(char type);

char * _RegisterListener (X3DEventOut *node, int adin);
int X3DAdvise (X3DEventOut *node, void *fn);
void _handleReWireCallback(char *buf);
char mapFieldTypeToEAItype (int st);
int mapEAItypeToFieldType (char st);
void sendMIDITableToFreeWRL(char *buf);
void sendMIDIControlToFreeWRL(long relativeSamplePos, int bus, int channel, int controller, int value);
#else
* GeneratedCode.c  generated by VRMLC.pm. DO NOT MODIFY, MODIFY VRMLC.pm INSTEAD */
/* definitions to help scanning values in from a string */
#define SCANTONUMBER(value) while (isspace(*value) || (*value==',')) value++;
#define SCANTOSTRING(value) while (isspace(*value) || (*value==',')) value++;
#define OLDSCANTOSTRING(value) while ((*value==' ') || (*value==',')) value++;
#define ISSTARTNUMBER(value) (isdigit(*value) \
                || (*value == '+') || (*value == '-'))
#define SCANPASTFLOATNUMBER(value) while (isdigit(*value) \
                || (*value == '.') || \
                (*value == 'E') || (*value == 'e') || (*value == '+') || (*value == '-')) value++;
#define SCANPASTINTNUMBER(value) if (isdigit(*value) || (*value == '-') || (*value == '+')) value++; \
                while (isdigit(*value) || \
                (*value == 'x') || (*value == 'X') ||\
                ((*value >='a') && (*value <='f')) || \
                ((*value >='A') && (*value <='F')) || \
                (*value == '-') || (*value == '+')) value++;
#define FIELDTYPE_SFFloat       0
#define FIELDTYPE_MFFloat       1
#define FIELDTYPE_SFRotation    2
#define FIELDTYPE_MFRotation    3
#define FIELDTYPE_SFVec3f       4
#define FIELDTYPE_MFVec3f       5
#define FIELDTYPE_SFBool        6
#define FIELDTYPE_MFBool        7
#define FIELDTYPE_SFInt32       8
#define FIELDTYPE_MFInt32       9
#define FIELDTYPE_SFNode        10
#define FIELDTYPE_MFNode        11
#define FIELDTYPE_SFColor       12
#define FIELDTYPE_MFColor       13
#define FIELDTYPE_SFColorRGBA   14
#define FIELDTYPE_MFColorRGBA   15
#define FIELDTYPE_SFTime        16
#define FIELDTYPE_MFTime        17
#define FIELDTYPE_SFString      18
#define FIELDTYPE_MFString      19
#define FIELDTYPE_SFVec2f       20
#define FIELDTYPE_MFVec2f       21
#define FIELDTYPE_SFImage       22
#define FIELDTYPE_FreeWRLPTR    23
#define FIELDTYPE_SFVec3d       24
#define FIELDTYPE_MFVec3d       25
#define FIELDTYPE_SFDouble      26
#define FIELDTYPE_MFDouble      27
#define FIELDTYPE_SFMatrix3f    28
#define FIELDTYPE_MFMatrix3f    29
#define FIELDTYPE_SFMatrix3d    30
#define FIELDTYPE_MFMatrix3d    31
#define FIELDTYPE_SFMatrix4f    32
#define FIELDTYPE_MFMatrix4f    33
#define FIELDTYPE_SFMatrix4d    34
#define FIELDTYPE_MFMatrix4d    35
#define FIELDTYPE_SFVec2d       36
#define FIELDTYPE_MFVec2d       37
#define FIELDTYPE_SFVec4f       38
#define FIELDTYPE_MFVec4f       39
#define FIELDTYPE_SFVec4d       40
#define FIELDTYPE_MFVec4d       41

#endif
