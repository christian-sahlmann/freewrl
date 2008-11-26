/* Class to wrap a java script for CParser */

#ifndef CSCRIPTS_H
#define CSCRIPTS_H

#include "headers.h"
#include "Vector.h"

#include "CFieldDecls.h"
#include "CParseGeneral.h"


/* ************************************************************************** */
/* ************************ Methods used by X3D Parser  ********************* */
/* ************************************************************************** */
uintptr_t nextScriptHandle (void);
void zeroScriptHandles (void);
struct X3D_Script * protoScript_copy (struct X3D_Script *me);


/* ************************************************************************** */
/* ****************************** ScriptFieldDecl *************************** */
/* ************************************************************************** */

/* Struct */
/* ****** */

struct ScriptFieldDecl
{
 /* subclass of FieldDecl */
 struct FieldDecl* fieldDecl;

 /* Stringified */
 const char* name;
 const char* type;
 const char* ISname;

 /* For fields */
 union anyVrml value;
 BOOL valueSet;	/* Has the value been set? */
};

/* Structure that holds information regarding script fields that are targets in PROTO IS statements */
struct ScriptFieldInstanceInfo {
	struct ScriptFieldDecl* decl;
	struct Shader_Script* script;
};

/* Constructor and destructor */
/* ************************** */

struct ScriptFieldDecl* newScriptFieldDecl(struct VRMLLexer*, indexT, indexT, indexT);
struct ScriptFieldInstanceInfo* newScriptFieldInstanceInfo(struct ScriptFieldDecl*, struct Shader_Script*);
struct ScriptFieldDecl* scriptFieldDecl_copy(struct VRMLLexer*, struct ScriptFieldDecl*);
void deleteScriptFieldDecl(struct ScriptFieldDecl*);

/* Other members */
/* ************* */

/* Get "offset" data for routing */
int scriptFieldDecl_getRoutingOffset(struct ScriptFieldDecl*);

/* Set field value */
void scriptFieldDecl_setFieldValue(struct ScriptFieldDecl*, union anyVrml);

/* Do JS-init for given script handle */
void scriptFieldDecl_jsFieldInit(struct ScriptFieldDecl*, uintptr_t);

/* Forwards to inherited methods */
#define scriptFieldDecl_isField(me, nam, mod) \
 fieldDecl_isField((me)->fieldDecl, nam, mod)

/* ************************************************************************** */
/* ********************************** Script ******************************** */
/* ************************************************************************** */

/* Struct */
/* ****** */

struct Shader_Script
{
 struct X3D_Node *ShaderScriptNode; /* NODE_Script, NODE_ComposedShader, etc */
 uintptr_t num;	/* The script handle  if a script, -1 if a shader */
 BOOL loaded;	/* Has the code been loaded into this script? */
 struct Vector* fields;
};

/* Constructor and destructor */
/* ************************** */

struct Shader_Script* new_Shader_Script(struct X3D_Node *);
void deleteScript();

/* Other members */
/* ************* */

/* Initializes the script with its code */
BOOL script_initCode(struct Shader_Script*, const char*);
BOOL script_initCodeFromUri(struct Shader_Script*, const char*);
BOOL script_initCodeFromMFUri(struct Shader_Script*, const struct Multi_String*);

/* Add a new field */
void script_addField(struct Shader_Script*, struct ScriptFieldDecl*);

/* Get a field by name */
struct ScriptFieldDecl* script_getField(struct Shader_Script*, indexT ind, indexT mod);


void InitScriptField(int num, indexT kind, indexT type, char* field, union anyVrml value);
void SaveScriptField (int num, indexT kind, indexT type, char* field, union anyVrml value);
struct ScriptParamList {
        struct ScriptParamList *next;
        indexT kind;
        indexT type;
        char *field;
        union anyVrml value;
};

struct CRscriptStruct {
	/* type */
	int thisScriptType;

	/* Javascript parameters */
	int _initialized;			/* this script initialized yet? */
	uintptr_t	cx;			/* JSContext		*/
	uintptr_t	glob;			/* JSGlobals		*/
	uintptr_t	eventsProcessed; 	/* eventsProcessed() compiled function parameter*/
	char *scriptText;
	struct ScriptParamList *paramList;
};
#endif /* Once-check */
