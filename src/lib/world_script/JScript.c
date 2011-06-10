/*
=INSERT_TEMPLATE_HERE=

$Id: JScript.c,v 1.39 2011/06/10 13:31:42 crc_canada Exp $

Javascript C language binding.

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

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../vrml_parser/CRoutes.h"
#include "../main/Snapshot.h"
#include "../scenegraph/Collision.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"
#include "../input/EAIHelpers.h"
#include "../input/SensInterps.h"
#include "../x3d_parser/Bindable.h"

#include "JScript.h"
#include "CScripts.h"
#include "jsUtils.h"
#include "jsNative.h"
#include "jsVRMLClasses.h"
#include "jsVRMLBrowser.h"


#ifdef HAVE_JAVASCRIPT

//int JSMaxScript = 0;
///* Script name/type table */
//struct CRjsnameStruct *JSparamnames = NULL;

//int jsnameindex = -1;
//int MAXJSparamNames = 0;
//static JSRuntime *runtime = NULL;
static JSClass staticGlobalClass = {
	"global",
	0,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_EnumerateStub,
	globalResolve,
	JS_ConvertStub,
	JS_FinalizeStub
};


#endif // HAVE_JAVASCRIPT


typedef struct pJScript{
	/* Script name/type table */
	struct CRjsnameStruct *JSparamnames;// = NULL;
	int JSMaxScript;// = 0;

#ifdef HAVE_JAVASCRIPT
	JSRuntime *runtime;// = NULL;
	JSClass globalClass;
#endif // HAVE_JAVASCRIPT

}* ppJScript;


void *JScript_constructor(){
	void *v = malloc(sizeof(struct pJScript));
	memset(v,0,sizeof(struct pJScript));
	return v;
}
void JScript_init(struct tJScript *t){
	//public
	t->jsnameindex = -1;
	t->MAXJSparamNames = 0;

	//private
	t->prv = JScript_constructor();
	{
		ppJScript p = (ppJScript)t->prv;
		/* Script name/type table */
		p->JSparamnames = NULL;
#ifdef HAVE_JAVASCRIPT
		p->JSMaxScript = 0;
		p->runtime = NULL;
		memcpy(&p->globalClass,&staticGlobalClass,sizeof(staticGlobalClass));
#endif // HAVE_JAVASCRIPT

	}
}
//	ppJScript p = (ppJScript)gglobal()->JScript.prv;

struct CRjsnameStruct *getJSparamnames()
{
	ppJScript p = (ppJScript)gglobal()->JScript.prv;
	return p->JSparamnames;
}
void setJSparamnames(struct CRjsnameStruct *JSparamnames)
{
	ppJScript p = (ppJScript)gglobal()->JScript.prv;
	p->JSparamnames = JSparamnames;
}

/* Save the text, so that when the script is initialized in the fwl_RenderSceneUpdateScene thread, it will be there */
void SaveScriptText(int num, const char *text) {
	ttglobal tg = gglobal();
	ppJScript p = (ppJScript)tg->JScript.prv;
	struct CRscriptStruct *ScriptControl = getScriptControl();

	/* printf ("SaveScriptText, num %d, thread %u saving :%s:\n",num, pthread_self(),text); */
	if (num >= p->JSMaxScript)  {
		ConsoleMessage ("SaveScriptText: warning, script %d initialization out of order",num);
		return;
	}
	FREE_IF_NZ(ScriptControl[num].scriptText);
	ScriptControl[num].scriptText = STRDUP(text);

	if (((int)num) > tg->CRoutes.max_script_found) tg->CRoutes.max_script_found = num;
	/* printf ("SaveScriptText, for script %d scriptText %s\n",text);
	printf ("SaveScriptText, max_script_found now %d\n",max_script_found); */
}

#ifdef HAVE_JAVASCRIPT

/* MAX_RUNTIME_BYTES controls when garbage collection takes place. */
/* #define MAX_RUNTIME_BYTES 0x1000000 */
#define MAX_RUNTIME_BYTES 0x4000000L
/* #define MAX_RUNTIME_BYTES 0xC00000L */

#define STACK_CHUNK_SIZE 8192

static int JSaddGlobalECMANativeProperty(int num, const char *name);
static int JSaddGlobalAssignProperty(int num, const char *name, const char *str);
static void InitScriptField(int num, indexT kind, indexT type, const char* field, union anyVrml value);

/*
 * Global JS variables (from Brendan Eichs short embedding tutorial):
 *
 * JSRuntime       - 1 runtime per process
 * JSContext       - 1 CONTEXT per thread
 * global JSObject - 1 global object per CONTEXT
 *
 * struct JSClass {
 *     char *name;
 *     uint32 flags;
 * Mandatory non-null function pointer members:
 *     JSPropertyOp addProperty;
 *     JSPropertyOp delProperty;
 *     JSPropertyOp getProperty;
 *     JSPropertyOp setProperty;
 *     JSEnumerateOp enumerate;
 *     JSResolveOp resolve;
 *     JSConvertOp convert;
 *     JSFinalizeOp finalize;
 * Optionally non-null members start here:
 *     JSGetObjectOps getObjectOps;
 *     JSCheckAccessOp checkAccess;
 *     JSNative call;
 *     JSNative construct;
 *     JSXDRObjectOp xdrObject;
 *     JSHasInstanceOp hasInstance;
 *     prword spare[2];
 * };
 *
 * global JSClass  - populated by stubs
 *
 */

static char *DefaultScriptMethods = "function initialize() {}; " \
			" function shutdown() {}; " \
			" function eventsProcessed() {}; " \
			" TRUE=true; FALSE=false; " \
			" function print(x) {Browser.print(x)}; " \
			" function println(x) {Browser.println(x)}; " \
			" function getName() {return Browser.getName()}; "\
			" function getVersion() {return Browser.getVersion()}; "\
			" function getCurrentSpeed() {return Browser.getCurrentSpeed()}; "\
			" function getCurrentFrameRate() {return Browser.getCurrentFrameRate()}; "\
			" function getWorldURL() {return Browser.getWorldURL()}; "\
			" function replaceWorld(x) {Browser.replaceWorld(x)}; "\
			" function loadURL(x,y) {Browser.loadURL(x,y)}; "\
			" function setDescription(x) {Browser.setDescription(x)}; "\
			" function createVrmlFromString(x) {Browser.createVrmlFromString(x)}; "\
			" function createVrmlFromURL(x,y,z) {Browser.createVrmlFromURL(x,y,z)}; "\
			" function createX3DFromString(x) {Browser.createX3DFromString(x)}; "\
			" function createX3DFromURL(x,y,z) {Browser.createX3DFromURL(x,y,z)}; "\
			" function addRoute(a,b,c,d) {Browser.addRoute(a,b,c,d)}; "\
			" function deleteRoute(a,b,c,d) {Browser.deleteRoute(a,b,c,d)}; "
			"";

//static JSRuntime *runtime = NULL;
//static JSClass globalClass = {
//	"global",
//	0,
//	JS_PropertyStub,
//	JS_PropertyStub,
//	JS_PropertyStub,
//	JS_PropertyStub,
//	JS_EnumerateStub,
//	globalResolve,
//	JS_ConvertStub,
//	JS_FinalizeStub
//};


/* housekeeping routines */
void kill_javascript(void) {
	int i;
	ttglobal tg = gglobal();
	ppJScript p = (ppJScript)tg->JScript.prv;
	struct CRscriptStruct *ScriptControl = getScriptControl();

	/* printf ("calling kill_javascript()\n"); */
	zeroScriptHandles();
	if (p->runtime != NULL) {
		for (i=0; i<=tg->CRoutes.max_script_found_and_initialized; i++) {
			/* printf ("kill_javascript, looking at %d\n",i); */
			if (ScriptControl[i].cx != 0) {
				/* printf ("kill_javascript, context is %p\n",ScriptControl[i].cx); */
				JS_DestroyContextMaybeGC(ScriptControl[i].cx);
			}
		}

		JS_DestroyRuntime(p->runtime);
		p->runtime = NULL;
	}
	p->JSMaxScript = 0;
	tg->CRoutes.max_script_found = -1;
	tg->CRoutes.max_script_found_and_initialized = -1;
	FREE_IF_NZ (ScriptControl);
	FREE_IF_NZ(tg->CRoutes.scr_act);

	/* Script name/type table */
	FREE_IF_NZ(p->JSparamnames);
	tg->JScript.jsnameindex = -1;
	tg->JScript.MAXJSparamNames = 0;

}

void cleanupDie(int num, const char *msg) {
	kill_javascript();
	freewrlDie(msg);
}

void JSMaxAlloc() {
	/* perform some REALLOCs on JavaScript database stuff for interfacing */
	int count;
	ttglobal tg = gglobal();
	ppJScript p = (ppJScript)tg->JScript.prv;
	/* printf ("start of JSMaxAlloc, JSMaxScript %d\n",JSMaxScript); */
	struct CRscriptStruct *ScriptControl = getScriptControl();

	p->JSMaxScript += 10;
	setScriptControl( (struct CRscriptStruct*)REALLOC (ScriptControl, sizeof (*ScriptControl) * p->JSMaxScript));
	ScriptControl = getScriptControl();
	tg->CRoutes.scr_act = (int *)REALLOC (tg->CRoutes.scr_act, sizeof (*tg->CRoutes.scr_act) * p->JSMaxScript);

	/* mark these scripts inactive */
	for (count=p->JSMaxScript-10; count<p->JSMaxScript; count++) {
		tg->CRoutes.scr_act[count]= FALSE;
		ScriptControl[count].thisScriptType = NOSCRIPT;
		ScriptControl[count].eventsProcessed = NULL;
		ScriptControl[count].cx = 0;
		ScriptControl[count].glob = 0;
		ScriptControl[count]._initialized = FALSE;
		ScriptControl[count].scriptOK = FALSE;
		ScriptControl[count].scriptText = NULL;
		ScriptControl[count].paramList = NULL;
	}
}

/* set up table entry for this new script */
void JSInit(int num) {
	ppJScript p = (ppJScript)gglobal()->JScript.prv;
	#ifdef JAVASCRIPTVERBOSE 
	printf("JSinit: script %d\n",num);
	#endif

	/* more scripts than we can handle right now? */
	if (num >= p->JSMaxScript)  {
		JSMaxAlloc();
	}
}

void JSInitializeScriptAndFields (int num) {
        struct ScriptParamList *thisEntry;
        struct ScriptParamList *nextEntry;
	jsval rval;
	ppJScript p = (ppJScript)gglobal()->JScript.prv;
	struct CRscriptStruct *ScriptControl = getScriptControl();

	/* printf ("JSInitializeScriptAndFields script %d, thread %u\n",num,pthread_self());   */
	/* run through paramList, and run the script */
	/* printf ("JSInitializeScriptAndFields, running through params and main script\n");  */
	if (num >= p->JSMaxScript)  {
		ConsoleMessage ("JSInitializeScriptAndFields: warning, script %d initialization out of order",num);
		return;
	}
	/* run through fields in order of entry in the X3D file */
        thisEntry = ScriptControl[num].paramList;
        while (thisEntry != NULL) {
		/* printf ("script field is %s\n",thisEntry->field);  */
		InitScriptField(num, thisEntry->kind, thisEntry->type, thisEntry->field, thisEntry->value);

		/* get the next block; free the current name, current block, and make current = next */
		nextEntry = thisEntry->next;
		FREE_IF_NZ (thisEntry->field);
		FREE_IF_NZ (thisEntry);
		thisEntry = nextEntry;
	}
	
	/* we have freed each element, set list to NULL in case anyone else comes along */
	ScriptControl[num].paramList = NULL;

	if (!ACTUALRUNSCRIPT(num, ScriptControl[num].scriptText, &rval)) {
		ConsoleMessage ("JSInitializeScriptAndFields, script failure");
		ScriptControl[num].scriptOK = FALSE;
		ScriptControl[num]._initialized = TRUE;
		return;
	}
	FREE_IF_NZ(ScriptControl[num].scriptText);
	ScriptControl[num]._initialized = TRUE;
	ScriptControl[num].scriptOK = TRUE;

}

/* create the script context for this script. This is called from the thread
   that handles script calling in the fwl_RenderSceneUpdateScene */
void JSCreateScriptContext(int num) {
	jsval rval;
	JSContext *_context; 	/* these are set here */
	JSObject *_globalObj; 	/* these are set here */
	BrowserNative *br; 	/* these are set here */
	ppJScript p = (ppJScript)gglobal()->JScript.prv;
	struct CRscriptStruct *ScriptControl = getScriptControl();

	/* is this the first time through? */
	if (p->runtime == NULL) {
		p->runtime = JS_NewRuntime(MAX_RUNTIME_BYTES);
		if (!p->runtime) freewrlDie("JS_NewRuntime failed");

		#ifdef JAVASCRIPTVERBOSE 
		printf("\tJS runtime created,\n");
		#endif
	}


	_context = JS_NewContext(p->runtime, STACK_CHUNK_SIZE);
	if (!_context) freewrlDie("JS_NewContext failed");

	#ifdef JAVASCRIPTVERBOSE 
	printf("\tJS context created,\n");
	#endif


	_globalObj = JS_NewObject(_context, &p->globalClass, NULL, NULL);
	if (!_globalObj) freewrlDie("JS_NewObject failed");

	#ifdef JAVASCRIPTVERBOSE 
	printf("\tJS global object created,\n");
	#endif


	/* gets JS standard classes */
	if (!JS_InitStandardClasses(_context, _globalObj))
		freewrlDie("JS_InitStandardClasses failed");

	#ifdef JAVASCRIPTVERBOSE 
	printf("\tJS standard classes initialized,\n");
	#endif

	#ifdef JAVASCRIPTVERBOSE 
	 	reportWarningsOn();
	#endif

	JS_SetErrorReporter(_context, errorReporter);

	#ifdef JAVASCRIPTVERBOSE 
	printf("\tJS errror reporter set,\n");
	#endif

	br = (BrowserNative *) JS_malloc(_context, sizeof(BrowserNative));

	/* for this script, here are the necessary data areas */
	ScriptControl[num].cx =  _context;
	ScriptControl[num].glob =  _globalObj;


	if (!loadVrmlClasses(_context, _globalObj))
		freewrlDie("loadVrmlClasses failed");


	#ifdef JAVASCRIPTVERBOSE 
	printf("\tVRML classes loaded,\n");
	#endif

	if (!VrmlBrowserInit(_context, _globalObj, br))
		freewrlDie("VrmlBrowserInit failed");

	#ifdef JAVASCRIPTVERBOSE 
	printf("\tVRML Browser interface loaded,\n");
	#endif

	if (!ACTUALRUNSCRIPT(num,DefaultScriptMethods,&rval))
		cleanupDie(num,"runScript failed in VRML::newJS DefaultScriptMethods");

	/* send this data over to the routing table functions. */
	CRoutes_js_new (num, JAVASCRIPT);

	#ifdef JAVASCRIPTVERBOSE 
	printf("\tVRML browser initialized, thread %u\n",pthread_self());
	#endif
}


/* run the script from within C */
#ifdef JAVASCRIPTVERBOSE
int ActualrunScript(int num, char *script, jsval *rval, char *fn, int line) {
#else
int ActualrunScript(int num, char *script, jsval *rval) {
#endif
	int len;
	JSContext *_context;
	JSObject *_globalObj;
	struct CRscriptStruct *ScriptControl = getScriptControl();


	/* get context and global object for this script */
	_context = ScriptControl[num].cx;
	_globalObj = ScriptControl[num].glob;

	#ifdef JAVASCRIPTVERBOSE
		printf("ActualrunScript script called at %s:%d  num: %d cx %x \"%s\", \n", 
			fn, line, num, _context, script);
	#endif
	CLEANUP_JAVASCRIPT(_context)

	len = (int) strlen(script);
	if (!JS_EvaluateScript(_context, _globalObj, script, len, FNAME_STUB, LINENO_STUB, rval)) {
		printf ("ActualrunScript - JS_EvaluateScript failed for %s", script);
		printf ("\n");
		ConsoleMessage ("ActualrunScript - JS_EvaluateScript failed for %s", script);

		return JS_FALSE;
	 }

	#ifdef JAVASCRIPTVERBOSE 
	printf ("runscript passed\n");
	#endif

	return JS_TRUE;
}

/* run the script from within Javascript  */
int jsrrunScript(JSContext *_context, JSObject *_globalObj, char *script, jsval *rval) {

	int len;

	#ifdef JAVASCRIPTVERBOSE
		printf("jsrrunScript script cx %x \"%s\", \n",
			   _context, script);
	#endif

	len = (int) strlen(script);
	if (!JS_EvaluateScript(_context, _globalObj, script, len,
						   FNAME_STUB, LINENO_STUB, rval)) {
		ConsoleMessage ("jsrunScript - JS_EvaluateScript failed for %s", script);
		return JS_FALSE;
	 }

	#ifdef JAVASCRIPTVERBOSE 
	printf ("runscript passed\n");
	#endif

	return JS_TRUE;
}

/* FROM VRMLC.pm */
void *SFNodeNativeNew()
{
	SFNodeNative *ptr;
	ptr = MALLOC(SFNodeNative *, sizeof(*ptr));

	/* printf ("SFNodeNativeNew; string len %d handle_len %d\n",vrmlstring_len,handle_len);*/

	ptr->handle = 0;
	ptr->valueChanged = 0;
	ptr->X3DString = NULL;
	ptr->fieldsExpanded = FALSE;
	return ptr;
}

/* assign this internally to the Javascript engine environment */
int SFNodeNativeAssign(void *top, void *fromp)
{
	SFNodeNative *to = (SFNodeNative *)top;
	SFNodeNative *from = (SFNodeNative *)fromp;

	/* indicate that this was touched; and copy contents over */
	to->valueChanged++;

	if (from != NULL) {
		to->handle = from->handle;
		to->X3DString = STRDUP(from->X3DString);

		#ifdef JAVASCRIPTVERBOSE
		printf ("SFNodeNativeAssign, copied %d to %d, handle %d, string %s\n", from, to, to->handle, to->X3DString);
		#endif
	} else {
		to->handle = 0;
		to->X3DString = STRDUP("from a NULL assignment");
	}

	return JS_TRUE;
}

void *SFColorRGBANativeNew()
{
	SFColorRGBANative *ptr;
	ptr = MALLOC(SFColorRGBANative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFColorRGBANativeAssign(void *top, void *fromp)
{
	SFColorRGBANative *to = (SFColorRGBANative *)top;
	SFColorRGBANative *from = (SFColorRGBANative *)fromp;
	to->valueChanged ++;
	(to->v) = (from->v);
}

void *SFColorNativeNew()
{
	SFColorNative *ptr;
	ptr = MALLOC(SFColorNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFColorNativeAssign(void *top, void *fromp)
{
	SFColorNative *to = (SFColorNative *)top;
	SFColorNative *from = (SFColorNative *)fromp;
	to->valueChanged++;
	(to->v) = (from->v);
}

void *SFImageNativeNew()
{
	SFImageNative *ptr;
	ptr =MALLOC(SFImageNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFImageNativeAssign(void *top, void *fromp)
{
	SFImageNative *to = (SFImageNative *)top;
	/* SFImageNative *from = fromp; */
	UNUSED(fromp);

	to->valueChanged++;
/* 	(to->v) = (from->v); */
}

void *SFRotationNativeNew()
{
	SFRotationNative *ptr;
	ptr = MALLOC(SFRotationNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFRotationNativeAssign(void *top, void *fromp)
{
	SFRotationNative *to = (SFRotationNative *)top;
	SFRotationNative *from = (SFRotationNative *)fromp;
	to->valueChanged++;
	(to->v) = (from->v);
}

void *SFVec2fNativeNew()
{
	SFVec2fNative *ptr;
	ptr = MALLOC(SFVec2fNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFVec2fNativeAssign(void *top, void *fromp)
{
	SFVec2fNative *to = (SFVec2fNative *)top;
	SFVec2fNative *from = (SFVec2fNative *)fromp;
	to->valueChanged++;
	(to->v) = (from->v);
}

void *SFVec3fNativeNew() {
	SFVec3fNative *ptr;
	ptr = MALLOC(SFVec3fNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFVec3fNativeAssign(void *top, void *fromp) {
	SFVec3fNative *to = (SFVec3fNative *)top;
	SFVec3fNative *from = (SFVec3fNative *)fromp;
	to->valueChanged++;
	(to->v) = (from->v);
}

void *SFVec3dNativeNew() {
	SFVec3dNative *ptr;
	ptr = MALLOC(SFVec3dNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFVec3dNativeAssign(void *top, void *fromp) {
	SFVec3dNative *to = (SFVec3dNative *)top;
	SFVec3dNative *from = (SFVec3dNative *)fromp;
	to->valueChanged++;
	(to->v) = (from->v);
}

void *SFVec4fNativeNew() {
	SFVec4fNative *ptr;
	ptr = MALLOC(SFVec4fNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFVec4fNativeAssign(void *top, void *fromp) {
	SFVec4fNative *to = (SFVec4fNative *)top;
	SFVec4fNative *from = (SFVec4fNative *)fromp;
	to->valueChanged++;
	(to->v) = (from->v);
}

void *SFVec4dNativeNew() {
	SFVec4dNative *ptr;
	ptr = MALLOC(SFVec4dNative *, sizeof(*ptr));
	ptr->valueChanged = 0;
	return ptr;
}

void SFVec4dNativeAssign(void *top, void *fromp) {
	SFVec4dNative *to = (SFVec4dNative *)top;
	SFVec4dNative *from = (SFVec4dNative *)fromp;
	to->valueChanged++;
	(to->v) = (from->v);
}


/* A new version of InitScriptField which takes "nicer" arguments; currently a
 * simple and restricted wrapper, but it could replace it soon? */
/* Parameters:
	num:		Script number. Starts at 0. 
	kind:		One of PKW_initializeOnly PKW_outputOnly PKW_inputOutput PKW_inputOnly
	type:		One of the FIELDTYPE_ defines, eg, FIELDTYPE_MFFloat
	field:		the field name as found in the VRML/X3D file. eg "set_myField"
		
*/

/* save this field from the parser; initialize it when the fwl_RenderSceneUpdateScene wants to initialize it */
void SaveScriptField (int num, indexT kind, indexT type, const char* field, union anyVrml value) {
	struct ScriptParamList **nextInsert;
	struct ScriptParamList *newEntry;
	struct CRscriptStruct *ScriptControl = getScriptControl();
	ppJScript p = (ppJScript)gglobal()->JScript.prv;

	if (num >= p->JSMaxScript)  {
		ConsoleMessage ("JSSaveScriptText: warning, script %d initialization out of order",num);
		return;
	}

	/* generate a new ScriptParamList entry */
	/* note that this is a linked list, and we put things on at the end. The END MUST
	   have NULL termination */
	nextInsert = &(ScriptControl[num].paramList);
	while (*nextInsert != NULL) {
		nextInsert = &(*nextInsert)->next;
	}

	/* create a new entry and link it in */
	newEntry = MALLOC (struct ScriptParamList *, sizeof (struct ScriptParamList));
	*nextInsert = newEntry;
	
	/* initialize the new entry */
	newEntry->next = NULL;
	newEntry->kind = kind;
	newEntry->type = type;
	newEntry->field = STRDUP(field);
	newEntry->value = value;
}

static char* re_strcat(char *_Dest, char *_Source, int *destLen, int *destDim)
{
	/* strcats, but first checks strlen on source and destination
	   and reallocs if necessary - good when you are doing a lot of strcatting of un-pre-known elements
	   (Q. is there something for this already?)
	   _Dest, _Source - as with strcat(_Dest,_Source)
	   destLen - current cumulative strlen(_Dest)
	   destDim - current malloc/realloc dimension
	   Usage example:
		dstdim = (rows+1)*(elements*15) + 100; //a guess
		dstlen = 0;
		smallfield = MALLOC (char *, dstdim+1); //rows+1)*(elements*15) + 100);
		smallfield[0] = '\0';
		...
		for(;;)
		{
			...
			smallfield = re_strcat(smallfield, "new ",&dstlen,&dstdim);
		...
		FREE_IF_NZ(smallfield)
	*/
	int srclen = (int) strlen(_Source);
	*destLen = *destLen + srclen;
	if(*destLen > *destDim -1)
	{
		*destDim = *destDim + srclen + 1 + 100;
		_Dest = realloc(_Dest,*destDim);
	}
	_Dest = strcat(_Dest,_Source);
	return _Dest;
}
/* the fwl_RenderSceneUpdateScene is initializing this field now */
static void InitScriptField(int num, indexT kind, indexT type, const char* field, union anyVrml value) {
	jsval rval;
	char *smallfield = NULL;
	char mynewname[400];
	char *thisValue;
	int rows, elements;
	char *sftype = NULL;

	int haveMulti;
	int MFhasECMAtype;
	int rowCount, eleCount;

	int tlen;
	float *FloatPtr;
	struct X3D_Node **VoidPtr;
	int *IntPtr;
	double *DoublePtr;
	struct Uni_String **SVPtr;

	float defaultFloat[] = {0.0f,0.0f,0.0f,0.0f};
	int defaultInt[] = {0,0,0,0};
	double defaultDouble[] = {0.0, 0.0, 0.0, 0.0};
	struct Uni_String *sptr[1];
	struct X3D_Node *defaultVoid[] = {NULL,NULL};
	struct CRscriptStruct *ScriptControl = getScriptControl();

	#ifdef JAVASCRIPTVERBOSE
	printf ("calling InitScriptField from thread %u\n",pthread_self());
	printf ("\nInitScriptField, num %d, kind %s type %s field %s value %d\n", num,PROTOKEYWORDS[kind],FIELDTYPES[type],field,value);
	#endif

        if ((kind != PKW_inputOnly) && (kind != PKW_outputOnly) && (kind != PKW_initializeOnly) && (kind != PKW_inputOutput)) {
                ConsoleMessage ("InitScriptField: invalid kind for script: %d\n",kind);
                return;
        }

        if (type >= FIELDTYPES_COUNT) {
                ConsoleMessage ("InitScriptField: invalid type for script: %d\n",type);
                return;
        }

	/* first, make a new name up */
	if (kind == PKW_inputOnly) {
		sprintf (mynewname,"__eventIn_Value_%s",field);
	} else strcpy(mynewname,field);

	/* ok, lets handle the types here */
	switch (type) {
		/* ECMA types */
		case FIELDTYPE_SFBool:
		case FIELDTYPE_SFFloat:
		case FIELDTYPE_SFTime:
		case FIELDTYPE_SFDouble:
		case FIELDTYPE_SFInt32:
		case FIELDTYPE_SFString: {
			/* do not care about eventIns */
			if (kind != PKW_inputOnly)  {
				JSaddGlobalECMANativeProperty(num, field);
				if (kind == PKW_initializeOnly) {
					if  (type == FIELDTYPE_SFString) {
						tlen = (int) strlen(value.sfstring->strptr) + 20;
					} else {
						tlen = (int) strlen(field) + 400; /* long, in the case of doubles */
					}
					smallfield = MALLOC (char *, tlen);
					smallfield[0] = '\0';

					switch (type) {
						case FIELDTYPE_SFFloat: sprintf (smallfield,"%s=%f\n",field,value.sffloat);break;
						case FIELDTYPE_SFTime: sprintf (smallfield,"%s=%f\n",field,value.sftime);break;
						case FIELDTYPE_SFDouble: sprintf (smallfield,"%s=%f\n",field,value.sftime);break;
						case FIELDTYPE_SFInt32: sprintf (smallfield,"%s=%d\n",field,value.sfint32); break;
						case FIELDTYPE_SFBool: 
							if (value.sfbool == 1) sprintf (smallfield,"%s=true",field);
							else sprintf (smallfield,"%s=false",field);
							break;
						case FIELDTYPE_SFString:  
							sprintf (smallfield,"%s=\"%s\"\n",field,value.sfstring->strptr); break;
					}

					if (!ACTUALRUNSCRIPT(num,smallfield,&rval))
						printf ("huh??? Field initialization script failed %s\n",smallfield);
				}
			}
			break;
		}
		/* non ECMA types */
		default: {
			/* get an appropriate pointer - we either point to the initialization value
			   in the script header, or we point to some data here that are default values */
			
			/* does this MF type have an ECMA type as a single element? */
			switch (type) {
				case FIELDTYPE_MFString:
				case FIELDTYPE_MFTime:
				case FIELDTYPE_MFBool:
				case FIELDTYPE_MFInt32:
				case FIELDTYPE_MFFloat: 
				JSaddGlobalECMANativeProperty(num, field);
					MFhasECMAtype = TRUE;
					break;
				default: {
					MFhasECMAtype = FALSE;
				}
			}

			elements=0;
			IntPtr = NULL;
			FloatPtr = NULL;
			DoublePtr = NULL;
			SVPtr = NULL;
			VoidPtr = NULL;
			if (kind == PKW_initializeOnly) {
				switch (type) {
					case FIELDTYPE_SFImage:
						VoidPtr = (struct X3D_Node **) (&(value.sfimage)); elements = 1;
						break;
					case FIELDTYPE_SFNode:
						VoidPtr = (struct X3D_Node **) (&(value.sfnode)); elements = 1;
						break;
					case FIELDTYPE_MFColor:
						FloatPtr = (float *) value.mfcolor.p; elements = value.mfcolor.n;
						break;
					case FIELDTYPE_MFColorRGBA:
						FloatPtr = (float *) value.mfcolorrgba.p; elements = value.mfcolorrgba.n;
						break;
					case FIELDTYPE_MFVec2f:
						FloatPtr = (float *) value.mfvec2f.p; elements = value.mfvec2f.n;
						break;
					case FIELDTYPE_MFVec3f:
						FloatPtr = (float *) value.mfvec3f.p; elements = value.mfvec3f.n;
						break;
					case FIELDTYPE_MFRotation: 
						FloatPtr = (float *) value.mfrotation.p; elements = value.mfrotation.n;
						break;
					case FIELDTYPE_SFVec2f:
						FloatPtr = (float *) value.sfvec2f.c; elements = 1;
						break;
					case FIELDTYPE_SFColor:
						FloatPtr = value.sfcolor.c; elements = 1;
						break;
					case FIELDTYPE_SFColorRGBA:
						FloatPtr = value.sfcolorrgba.c; elements = 1;
						break;
					case FIELDTYPE_SFRotation:
						FloatPtr = value.sfrotation.c; elements = 1;
						break;
					case FIELDTYPE_SFVec3f: 
						FloatPtr = value.sfvec3f.c; elements =1;
						break;
					case FIELDTYPE_SFVec3d: 
						DoublePtr = value.sfvec3d.c; elements =1;
						break;
					case FIELDTYPE_MFString:
						SVPtr = value.mfstring.p; elements = value.mfstring.n;
						break;
					case FIELDTYPE_MFTime:
						DoublePtr = value.mftime.p; elements = value.mftime.n;
						break;
					case FIELDTYPE_MFBool:
						IntPtr = value.mfbool.p; elements = value.mfbool.n;
						break;
					case FIELDTYPE_MFInt32:
						IntPtr = value.mfint32.p; elements = value.mfint32.n;
						break;
					case FIELDTYPE_MFNode:
						VoidPtr = (struct X3D_Node **)(value.mfnode.p); elements = value.mfnode.n;
						break;
					case FIELDTYPE_MFFloat: 
						FloatPtr = value.mffloat.p; elements = value.mffloat.n;
						break;
					case FIELDTYPE_SFVec4f:
						FloatPtr = value.sfvec4f.c; elements = 1;
						break;
					case FIELDTYPE_SFVec4d:
						DoublePtr = value.sfvec4d.c; elements = 1;
						break;

					default: {
						printf ("unhandled type, in InitScriptField %d\n",type);
						return;
					}
				}

			} else {
				/* make up a default pointer */
				elements = 1;
				switch (type) {
					/* Void types */
					case FIELDTYPE_SFNode:
					case FIELDTYPE_MFNode:
						VoidPtr = (struct X3D_Node **) &defaultVoid;
						break;

					/* Float types */
					case FIELDTYPE_MFColor:
					case FIELDTYPE_MFColorRGBA:
					case FIELDTYPE_MFVec2f:
					case FIELDTYPE_MFVec3f:
					case FIELDTYPE_MFRotation: 
					case FIELDTYPE_SFVec2f:
					case FIELDTYPE_SFColor:
					case FIELDTYPE_SFColorRGBA:
					case FIELDTYPE_SFRotation:
					case FIELDTYPE_SFVec3f: 
					case FIELDTYPE_SFVec4f: 
					case FIELDTYPE_MFFloat: 
						FloatPtr = defaultFloat;
						break;

					/* Int types */
					case FIELDTYPE_MFBool:
					case FIELDTYPE_MFInt32:
						IntPtr = defaultInt;
						break;

					/* String types */
					case FIELDTYPE_SFString:
					case FIELDTYPE_MFString:
						sptr[0] = newASCIIString("");
						SVPtr = sptr;
						break;

					/* SFImage */
					case FIELDTYPE_SFImage:
						IntPtr = defaultInt;
						break;

					/* Double types */
					case FIELDTYPE_SFVec2d:
					case FIELDTYPE_SFVec3d:
					case FIELDTYPE_MFTime:
					case FIELDTYPE_SFTime:
					case FIELDTYPE_SFDouble:
					case FIELDTYPE_SFVec4d:
						DoublePtr = defaultDouble;
						break;
						
					default: {
						printf ("unhandled type, in InitScriptField part 2 %d\n",type);
						return;
					}
				}

			}

			rows = returnElementRowSize (type);

			#ifdef JAVASCRIPTVERBOSE
			printf ("in fieldSet, we have ElementRowSize %d and individual elements %d\n",rows,elements);
			#endif

			/* make this at least as large as required, then add some more on to the end... */
			/*
			Old Approach 
					step1: compute using guestimate formulas
					step2: malloc
					step3: loop through strcat() and hope no overrun 
				Problem: heap corruption from array overrun - the guestimate has been bad 
				    a few times in 2010 with MFVec2fs and MFStrings with 42 and 47 elements, strings of varying length
				example for MFVec2f
				'new MFVec2f(new SFVec2f(1234.678910,1234.678910),...)'
				each SF 2 numbers each 10 digits plus new type(,), 15 chars  =35.
				3 x 15 = 45 (or (rows+1)x(elements*15)+100) 
				old formula falls short:
					old formula: smallfield = MALLOC (rows*((elements*15) + 100));
					example 47 SFVec2fs
					actual bytes: 47 x 35 bytes = 1645 + 13 for the MF = 1658
					old formula  2 x ((47*15)+100) = 1610   //thats 48 bytes short and I bomb out
					better formula  3 x (47*15) + 100 = 2215
			New Approach (July 28, 2010)
					step1: compute using guestimate formulas
					step2: malloc
					step3: loop through and realloc before strcat() if short
			*/
			{
				int dstlen, dstdim, tdim;
				tdim = 200;
				thisValue = MALLOC(char *, tdim+1);
				dstdim = (rows+1)*(elements*15) + 100; /* a guess */
				dstlen = 0;
				smallfield = MALLOC (char *, dstdim+1); //rows+1)*(elements*15) + 100);
				/* what is the equivalent SF for this MF?? */
				if (type != convertToSFType(type)) haveMulti = TRUE;
				 else haveMulti = FALSE;
				
				/* the sftype is the SF form of either the MF or SF */
				sftype = STRDUP((char *)FIELDTYPES[convertToSFType(type)]);

				/* SFStrings are Strings */
				if (strncmp(sftype,"SFString",8)==0) strcpy (sftype,"String");


				/* start the string */
				smallfield[0] = '\0';

				/* is this an MF variable, with SFs in it? */
				if (haveMulti) {
					smallfield = re_strcat(smallfield, "new ",&dstlen,&dstdim);
					smallfield = re_strcat(smallfield, (char *)FIELDTYPES[type],&dstlen,&dstdim);
					smallfield = re_strcat(smallfield, "(",&dstlen,&dstdim);
				}

				/* loop through, and put values in */
				for (eleCount=0; eleCount<elements; eleCount++) {
					/* ECMA native types can just be passed in... */
					if (!MFhasECMAtype) {
						smallfield = re_strcat(smallfield, "new ",&dstlen,&dstdim);
						smallfield = re_strcat(smallfield, sftype,&dstlen,&dstdim);
						smallfield = re_strcat(smallfield, "(",&dstlen,&dstdim);
					}

					/* go through the SF type; SFints will have 1; SFVec3f's will have 3, etc */
					for (rowCount=0; rowCount<rows; rowCount++ ) {
						if (IntPtr != NULL) {
							sprintf (thisValue,"%d",*IntPtr); IntPtr++;
						} else if (FloatPtr != NULL) {
							sprintf (thisValue,"%f",*FloatPtr); FloatPtr++;
						} else if (DoublePtr != NULL) {
							sprintf (thisValue,"%f",*DoublePtr); DoublePtr++;
						} else if (SVPtr != NULL) {
							sptr[0] = *SVPtr; SVPtr++;
							if(strlen(sptr[0]->strptr)+2 > tdim-1)
							{	
								tdim = (int) strlen(sptr[0]->strptr) + 1 + 100;
								thisValue = realloc(thisValue,tdim);
							}
							sprintf (thisValue,"\"%s\"",sptr[0]->strptr);
						} else { /* must be a Void */
							/* printf ("sending in a VoidPtr, it is %p\n",VoidPtr[0]);
							if (VoidPtr[0] != NULL) {printf ("it is a %s type\n",stringNodeType(X3D_NODE(VoidPtr[0])->_nodeType));} */
							sprintf (thisValue,"\"%p\"", VoidPtr[0]); VoidPtr++;
						}
						smallfield = re_strcat(smallfield, thisValue,&dstlen,&dstdim);
						if (rowCount < (rows-1)) smallfield = re_strcat(smallfield,",",&dstlen,&dstdim);
					}

					if (!MFhasECMAtype) smallfield = re_strcat(smallfield, ")",&dstlen,&dstdim);
					if (eleCount < (elements-1)) smallfield = re_strcat(smallfield,",",&dstlen,&dstdim);

				}


				if (haveMulti) {
					smallfield = re_strcat(smallfield,")",&dstlen,&dstdim);
				}
				/* printf("dstlen=%d dstdim=%d\n",dstlen,dstdim); */
				FREE_IF_NZ (thisValue);
			}
			/* Warp factor 5, Dr Sulu... */
			#ifdef JAVASCRIPTVERBOSE 
			printf ("JScript, for non-ECMA newname %s, sending :%s:\n",mynewname,smallfield); 
			#endif

			JSaddGlobalAssignProperty (num,mynewname,smallfield);
		}
	}

	/* Fields can generate an event, so we allow the touched flag to remain set. eventOuts have just
	   been initialized, and as such, should not send events, until after they really have been set.
	*/
	if (kind == PKW_outputOnly) {
		int fptr;
		int touched;

		/* get the number representing this type */
		fptr = JSparamIndex (field, FIELDTYPES[type]);

		/* set up global variables so that we can reset the touched flag */
		touched = get_valueChanged_flag (fptr, num);

		/* and, reset the touched flag, knowing that we have the variables set properly */
		resetScriptTouchedFlag(num, fptr); 
	}

	CLEANUP_JAVASCRIPT(ScriptControl[num].cx)

	FREE_IF_NZ (smallfield);
	FREE_IF_NZ (sftype);

	#ifdef JAVASCRIPTVERBOSE
	printf ("finished InitScriptField\n");
	#endif
}

static int JSaddGlobalECMANativeProperty(int num, const char *name) {
	JSContext *_context;
	JSObject *_globalObj;
	jsval rval = INT_TO_JSVAL(0);
	struct CRscriptStruct *ScriptControl = getScriptControl();

	/* get context and global object for this script */
	_context =  ScriptControl[num].cx;
	_globalObj = ScriptControl[num].glob;

	#ifdef  JAVASCRIPTVERBOSE
		printf("addGlobalECMANativeProperty: name \"%s\"\n", name);
	#endif

	if (!JS_DefineProperty(_context, _globalObj, name, rval, NULL, setECMANative, 0 | JSPROP_PERMANENT)) {
		printf("JS_DefineProperty failed for \"%s\" in addGlobalECMANativeProperty.\n", name);
		return JS_FALSE;
	}

	return JS_TRUE;
}

static int JSaddGlobalAssignProperty(int num, const char *name, const char *str) {
	jsval _rval = INT_TO_JSVAL(0);
	JSContext *_context;
	JSObject *_globalObj;
	struct CRscriptStruct *ScriptControl = getScriptControl();

	/* get context and global object for this script */
	_context =  ScriptControl[num].cx;
	_globalObj = ScriptControl[num].glob;

	#ifdef JAVASCRIPTVERBOSE 
		printf("addGlobalAssignProperty: cx: %d obj %d name \"%s\", evaluate script \"%s\"\n",
			   _context, _globalObj, name, str);
	#endif

	if (!JS_EvaluateScript(_context, _globalObj, str, (int) strlen(str), FNAME_STUB, LINENO_STUB, &_rval)) {
		ConsoleMessage ("JSaddGlobalAssignProperty - JS_EvaluateScript failed for %s", str);
		return JS_FALSE;
	}
	if (!JS_DefineProperty(_context, _globalObj, name, _rval, getAssignProperty, setAssignProperty, 0 | JSPROP_PERMANENT)) {
		printf("JS_DefineProperty failed for \"%s\" in addGlobalAssignProperty.\n", name);
		return JS_FALSE;
	}
	return JS_TRUE;
}

#endif /* HAVE_JAVASCRIPT */
