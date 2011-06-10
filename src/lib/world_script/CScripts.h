/*
=INSERT_TEMPLATE_HERE=

$Id: CScripts.h,v 1.23 2011/06/10 00:27:17 dug9 Exp $

Class to wrap a java script for CParser

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


#ifndef __FREEWRL_CSCRIPTS_H__
#define __FREEWRL_CSCRIPTS_H__

/* ************************************************************************** */
/* ************************ Java function name to JSScript mapping  ********* */
/* ************************************************************************** */
#define MAXJSVARIABLELENGTH 25  /* variable name length can be this long... */
/* Some stuff for routing */
#define FROM_SCRIPT 1
#define TO_SCRIPT 2
#define SCRIPT_TO_SCRIPT 3

struct CRjsnameStruct {
        int     	type;
        char    	name[MAXJSVARIABLELENGTH];
	JSScript *	eventInFunction;		/* compiled javascript function... if it is required */
};


/* ************************************************************************** */
/* ************************ Generic defines/function protos   *************** */
/* ************************************************************************** */
extern struct CRjsnameStruct *JSparamnames;
extern int jsnameindex;
extern int MAXJSparamNames;

#define JS_GET_PROPERTY_STUB JS_PropertyStub
/* #define JS_GET_PROPERTY_STUB js_GetPropertyDebug */

#define JS_SET_PROPERTY_STUB1 js_SetPropertyDebug1

/* #define JS_SET_PROPERTY_STUB2 js_SetPropertyDebug2  */
#if JS_VERSION < 185
# define JS_SET_PROPERTY_STUB2 JS_PropertyStub
#else
# define JS_SET_PROPERTY_STUB2 JS_StrictPropertyStub
#endif

#define JS_SET_PROPERTY_STUB3 js_SetPropertyDebug3 
#define JS_SET_PROPERTY_STUB4 js_SetPropertyDebug4 
#define JS_SET_PROPERTY_STUB5 js_SetPropertyDebug5 
#define JS_SET_PROPERTY_STUB6 js_SetPropertyDebug6 
#define JS_SET_PROPERTY_STUB7 js_SetPropertyDebug7 
#define JS_SET_PROPERTY_STUB8 js_SetPropertyDebug8 
#define JS_SET_PROPERTY_CHECK js_SetPropertyCheck

/* ************************************************************************** */
/* ************************ Methods used by X3D Parser  ********************* */
/* ************************************************************************** */
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

 /* Stringified value, if required by a parser. */
 char* ASCIIvalue; 

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
struct VRMLLexer;
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
 int num;	/* The script handle  if a script, -1 if a shader */
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
BOOL script_initCodeFromMFUri(struct Shader_Script*, const struct Multi_String*);
char **shader_initCodeFromMFUri(const struct Multi_String* s);

/* Add a new field */
void script_addField(struct Shader_Script*, struct ScriptFieldDecl*);

/* Get a field by name */
struct ScriptFieldDecl* script_getField(struct Shader_Script*, indexT ind, indexT mod);
struct ScriptFieldDecl* script_getField_viaCharName (struct Shader_Script* me, const char *name);


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
	JSContext *	cx;			/* JSContext		*/
	JSObject *	glob;			/* JSGlobals		*/
	JSScript *eventsProcessed; 	/* eventsProcessed() compiled function parameter*/
	char *scriptText;
	struct ScriptParamList *paramList;
	int 		scriptOK;		/* set to TRUE if the script loads ok */
};
//extern struct CRscriptStruct *ScriptControl;
struct CRscriptStruct *getScriptControl();
void setScriptControl(struct CRscriptStruct *ScriptControl);

/* function protos */

struct ScriptFieldInstanceInfo* scriptFieldInstanceInfo_copy(struct ScriptFieldInstanceInfo*);
void scriptFieldDecl_setFieldASCIIValue(struct ScriptFieldDecl *me, const char *val);

int JSparamIndex (const char *name, const char *type);

/* setting script eventIns from routing table or EAI */
void Set_one_MultiElementtype (int tn, int tptr, void *fn, unsigned len);
void mark_script (int num);



#endif /* __FREEWRL_CSCRIPTS_H__ */
