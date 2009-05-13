/*
=INSERT_TEMPLATE_HERE=

$Id: jsVRMLClasses.h,v 1.9 2009/05/13 20:30:49 crc_canada Exp $

Complex VRML nodes as Javascript classes.

*/

#ifndef __FREEWRL_JS_VRML_CLASSES_H__
#define __FREEWRL_JS_VRML_CLASSES_H__


#ifndef UNUSED
#define UNUSED(v) ((void) v)
#endif

#define INIT_ARGC_NODE 1
#define INIT_ARGC 0

/* quick fix to get around some compiler warnings on 64 bit systems */
#define VERBOSE_OBJX (unsigned long)
#define VERBOSE_OBJ (unsigned int)

/* tie a node into the root. Currently not required, as we do a better job
of garbage collection */
#define ADD_ROOT(a,b) \
	/* printf ("adding root  cx %u pointer %u value %u\n",a,&b,b); \
        if (JS_AddRoot(a,&b) != JS_TRUE) { \
                printf ("JA_AddRoot failed at %s:%d\n",__FILE__,__LINE__); \
                return JS_FALSE; \
        } */

#define REMOVE_ROOT(a,b) \
	/* printf ("removing root %u\n",b); \
        JS_RemoveRoot(a,&b);  */


#define DEFINE_LENGTH(thislength,thisobject) \
	{jsval zimbo = INT_TO_JSVAL(thislength);\
	/* printf ("defining length to %d for %d %d\n",thislength,cx,obj);*/ \
	if (!JS_DefineProperty(cx, thisobject, "length", zimbo, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) { \
		printf( "JS_DefineProperty failed for \"length\" at %s:%d.\n",__FILE__,__LINE__); \
		/* printf ("myThread is %u\n",pthread_self()); */ \
		return JS_FALSE;\
	}}

#define DEFINE_MF_ECMA_HAS_CHANGED \
	{jsval zimbo = INT_TO_JSVAL(0); \
	/* printf ("defining property for MF_ECMA_HAS_CHANGED... %d %d ",cx,obj);  */ \
	if (!JS_DefineProperty(cx, obj, "MF_ECMA_has_changed", zimbo, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) { \
		printf( "JS_DefineProperty failed for \"MF_ECMA_has_changed\" at %s:%d.\n",__FILE__,__LINE__); \
		/* printf ("myThread is %u\n",pthread_self()); */ \
		return JS_FALSE; \
	}}

#define SET_MF_ECMA_HAS_CHANGED { jsval myv; \
                        myv = INT_TO_JSVAL(1); \
			 /* printf ("setting property for MF_ECMA_has_changed %d %d\n",cx,obj); */ \
                        if (!JS_SetProperty(cx, obj, "MF_ECMA_has_changed", &myv)) { \
                                printf( "JS_SetProperty failed for \"MF_ECMA_has_changed\" in doMFSetProperty.\n"); \
                                return JS_FALSE; \
                        }}


#define SET_JS_TICKTIME(possibleRetVal) { jsval zimbo; \
        zimbo = DOUBLE_TO_JSVAL(JS_NewDouble(cx, TickTime));  \
        if (!JS_DefineProperty(cx,obj, "__eventInTickTime", zimbo, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) {  \
                printf( "JS_DefineProperty failed for \"__eventInTickTime\" at %s:%d.\n",__FILE__,__LINE__); \
                return possibleRetVal; \
        }}

#define COMPILE_FUNCTION_IF_NEEDED(tnfield) \
	if (JSparamnames[tnfield].eventInFunction == 0) { \
		sprintf (scriptline,"%s(__eventIn_Value_%s,__eventInTickTime)", JSparamnames[tnfield].name,JSparamnames[tnfield].name); \
		/* printf ("compiling function %s\n",scriptline); */ \
		JSparamnames[tnfield].eventInFunction = (uintptr_t) JS_CompileScript( \
			cx, obj, scriptline, strlen(scriptline), "compile eventIn",1); \
	}
#define RUN_FUNCTION(tnfield) \
	{jsval zimbo; \
	if (!JS_ExecuteScript(cx, obj, (JSScript *) JSparamnames[tnfield].eventInFunction, &zimbo)) { \
		printf ("failed to set parameter for eventIn %s in FreeWRL code %s:%d\n",JSparamnames[tnfield].name,__FILE__,__LINE__); \
		/* printf ("myThread is %u\n",pthread_self());*/ \
	}} 


#define SET_LENGTH(cx,newMFObject,length) \
	{ jsval lenval; \
                lenval = INT_TO_JSVAL(length); \
                if (!JS_SetProperty(cx, newMFObject, "length", &lenval)) { \
                        printf( "JS_SetProperty failed for \"length\" at %s:%d\n",__FILE__,__LINE__); \
		/* printf ("myThread is %u\n",pthread_self()); */ \
                        return JS_FALSE; \
                }} 

#define SET_EVENTIN_VALUE(cx,obj,nameIndex,newObj) \
	{ char scriptline[100]; \
		sprintf (scriptline,"__eventIn_Value_%s", JSparamnames[nameIndex].name); \
        	if (!JS_DefineProperty(cx,obj, scriptline, OBJECT_TO_JSVAL(newObj), JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) {  \
        	        printf( "JS_DefineProperty failed for \"ECMA in\" at %s:%d.\n",__FILE__,__LINE__);  \
		/* printf ("myThread is %u\n",pthread_self()); */ \
        	        return JS_FALSE; \
        }	}
	

/*
 * The following VRML field types don't need JS classes:
 * (ECMAScript native datatypes, see JS.pm):
 *
 * * SFBool
 * * SFFloat
 * * SFInt32
 * * SFString
 * * SFTime
 *
 * VRML field types that are implemented here as Javascript classes
 * are:
 *
 * * SFColor, MFColor
 * * MFFloat
 * * SFImage -- not supported currently
 * * MFInt32
 * * SFNode (special case - must be supported perl (see JS.pm), MFNode
 * * SFRotation, MFRotation
 * * MFString
 * * MFTime
 * * SFVec2f, MFVec2f
 * * SFVec3f, MFVec3f
 * * SFVec3d
 *
 * These (single value) fields have struct types defined elsewhere
 * (see Structs.h) that are stored by Javascript classes as private data.
 *
 * Some of the computations for SFVec3f, SFRotation are now defined
 * elsewhere (see LinearAlgebra.h) to avoid duplication.
 */


/* helper functions */
void JS_MY_Finalize(JSContext *cx, JSObject *obj);

JSBool doMFToString(JSContext *cx, JSObject *obj, const char *className, jsval *rval); 
JSBool doMFAddProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp, char *name); 
JSBool doMFSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp, int type); 
JSBool getBrowser(JSContext *context, JSObject *obj, BrowserNative **brow); 
JSBool doMFStringUnquote(JSContext *cx, jsval *vp);


/* class functions */

JSBool
globalResolve(JSContext *cx,
			  JSObject *obj,
			  jsval id);

JSBool
loadVrmlClasses(JSContext *context,
				JSObject *globalObj);


JSBool
setECMANative(JSContext *cx,
			  JSObject *obj,
			  jsval id,
			  jsval *vp);


JSBool
getAssignProperty(JSContext *context,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);

JSBool
setAssignProperty(JSContext *context,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);



JSBool
SFColorGetHSV(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorSetHSV(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp); 

JSBool
SFColorSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

JSBool
SFColorRGBAGetHSV(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorRGBASetHSV(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorRGBAToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorRGBAAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorRGBAConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
SFColorRGBAGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp); 

JSBool
SFColorRGBASetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

JSBool
SFImageToString(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

JSBool
SFImageAssign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);


JSBool
SFImageConstr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
SFImageGetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
SFImageSetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);



JSBool
SFNodeToString(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
SFNodeAssign(JSContext *cx, JSObject *obj,
			 uintN argc,
			 jsval *argv,
			 jsval *rval);

JSBool
SFNodeConstr(JSContext *cx,
			 JSObject *obj,
			 uintN argc,
			 jsval *argv,
			 jsval *rval);

void SFNodeFinalize(JSContext *cx, JSObject *obj);

JSBool
SFNodeGetProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);

JSBool
SFNodeSetProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);



JSBool
SFRotationGetAxis(JSContext *cx,
				  JSObject *obj,
				  uintN argc,
				  jsval *argv,
				  jsval *rval);

/* not implemented */
JSBool
SFRotationInverse(JSContext *cx,
				  JSObject *obj,
				  uintN argc,
				  jsval *argv,
				  jsval *rval);

JSBool
SFRotationMultiply(JSContext *cx,
				   JSObject *obj,
				   uintN argc,
				   jsval *argv,
				   jsval *rval);
JSBool
SFRotationMultVec(JSContext *cx,
				  JSObject *obj,
				  uintN argc,
				  jsval *argv,
				  jsval *rval);

JSBool
SFRotationSetAxis(JSContext *cx,
				  JSObject *obj,
				  uintN argc,
				  jsval *argv,
				  jsval *rval);

JSBool
SFRotationSlerp(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

JSBool
SFRotationToString(JSContext *cx,
				   JSObject *obj,
				   uintN argc,
				   jsval *argv,
				   jsval *rval);

JSBool
SFRotationAssign(JSContext *cx,
				 JSObject *obj,
				 uintN argc,
				 jsval *argv,
				 jsval *rval);

JSBool
SFRotationConstr(JSContext *cx,
				 JSObject *obj,
				 uintN argc,
				 jsval *argv,
				 jsval *rval);

JSBool
SFRotationGetProperty(JSContext *cx,
					  JSObject *obj,
					  jsval id,
					  jsval *vp);

JSBool
SFRotationSetProperty(JSContext *cx,
					  JSObject *obj,
					  jsval id,
					  jsval *vp);



JSBool
SFVec2fAdd(JSContext *cx,
		   JSObject *obj,
		   uintN argc,
		   jsval *argv,
		   jsval *rval);

JSBool
SFVec2fDivide(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
SFVec2fDot(JSContext *cx,
		   JSObject *obj,
		   uintN argc,
		   jsval *argv,
		   jsval *rval);

JSBool
SFVec2fLength(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
SFVec2fMultiply(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

/* JSBool
SFVec2fNegate(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);
*/

JSBool
SFVec2fNormalize(JSContext *cx,
				 JSObject *obj,
				 uintN argc,
				 jsval *argv,
				 jsval *rval);

JSBool
SFVec2fSubtract(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

JSBool
SFVec2fToString(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

JSBool
SFVec2fAssign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
SFVec2fConstr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
SFVec2fGetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
SFVec2fSetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);



JSBool SFVec3fAdd(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fCross(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fDivide(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fDot(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fLength(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fMultiply(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fNegate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fNormalize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fSubtract(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3fGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp); 
JSBool SFVec3fSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);


JSBool SFVec3dAdd(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dCross(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dDivide(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dDot(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dLength(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dMultiply(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dNegate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dNormalize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dSubtract(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec3dGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp); 
JSBool SFVec3dSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);


JSBool SFVec4fGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp); 
JSBool SFVec4fSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
JSBool SFVec4fToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec4fAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec4fConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec4dGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp); 
JSBool SFVec4dSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
JSBool SFVec4dToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec4dAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec4dConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
MFColorToString(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
MFColorAssign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFColorConstr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFColorAddProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFColorGetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFColorSetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);



JSBool
MFFloatToString(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
MFFloatAssign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFFloatConstr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFFloatAddProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFFloatGetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFFloatSetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);



JSBool
MFInt32ToString(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
MFInt32Assign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFInt32Constr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFInt32AddProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFInt32GetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFInt32SetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);


JSBool
MFNodeToString(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
MFNodeAssign(JSContext *cx,
			 JSObject *obj,
			 uintN argc,
			 jsval *argv,
			 jsval *rval);

JSBool
MFNodeConstr(JSContext *cx,
			 JSObject *obj,
			 uintN argc,
			 jsval *argv,
			 jsval *rval);

JSBool
MFNodeAddProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);

JSBool
MFNodeGetProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);

JSBool
MFNodeSetProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);



JSBool
MFRotationToString(JSContext *cx,
				   JSObject *obj,
				   uintN argc,
				   jsval *argv,
				   jsval *rval);

JSBool
MFRotationAssign(JSContext *cx,
				 JSObject *obj,
				 uintN argc,
				 jsval *argv,
				 jsval *rval);

JSBool
MFRotationConstr(JSContext *cx,
				 JSObject *obj,
				 uintN argc,
				 jsval *argv,
				 jsval *rval);

JSBool
MFRotationGetProperty(JSContext *cx,
					  JSObject *obj,
					  jsval id,
					  jsval *vp);

JSBool
MFRotationSetProperty(JSContext *cx,
					  JSObject *obj,
					  jsval id,
					  jsval *vp);

JSBool
MFRotationAddProperty(JSContext *cx,
					  JSObject *obj,
					  jsval id,
					  jsval *vp);



JSBool
MFStringToString(JSContext *cx,
				 JSObject *obj,
				 uintN argc,
				 jsval *argv,
				 jsval *rval);

JSBool
MFStringAssign(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
MFStringConstr(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
MFStringGetProperty(JSContext *cx,
					JSObject *obj,
					jsval id,
					jsval *vp);

JSBool
MFStringSetProperty(JSContext *cx,
					JSObject *obj,
					jsval id,
					jsval *vp);


JSBool
MFStringAddProperty(JSContext *cx,
					JSObject *obj,
					jsval id,
					jsval *vp);

JSBool MFStringDeleteProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) ;
JSBool MFStringEnumerateProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) ;
JSBool MFStringResolveProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) ;
JSBool MFStringConvertProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) ;
       


JSBool
MFTimeToString(JSContext *cx,
			   JSObject *obj,
			   uintN argc,
			   jsval *argv,
			   jsval *rval);

JSBool
MFTimeAssign(JSContext *cx,
			 JSObject *obj,
			 uintN argc,
			 jsval *argv,
			 jsval *rval);

JSBool
MFTimeConstr(JSContext *cx,
			 JSObject *obj,
			 uintN argc,
			 jsval *argv,
			 jsval *rval);

JSBool
MFTimeAddProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);

JSBool
MFTimeGetProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);

JSBool
MFTimeSetProperty(JSContext *cx,
				  JSObject *obj,
				  jsval id,
				  jsval *vp);



JSBool
MFVec2fToString(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

JSBool
MFVec2fAssign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFVec2fConstr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFVec2fAddProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFVec2fGetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFVec2fSetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);



JSBool
MFVec3fToString(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

JSBool
MFVec3fAssign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFVec3fConstr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
MFVec3fAddProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFVec3fGetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
MFVec3fSetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
VrmlMatrixToString(JSContext *cx,
				JSObject *obj,
				uintN argc,
				jsval *argv,
				jsval *rval);

JSBool
VrmlMatrixAssign(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);


JSBool VrmlMatrixsetTransform(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool VrmlMatrixgetTransform(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool VrmlMatrixinverse(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool VrmlMatrixtranspose(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool VrmlMatrixmultLeft(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool VrmlMatrixmultRight(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool VrmlMatrixmultVecMatrix(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool VrmlMatrixmultMatrixVec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
VrmlMatrixConstr(JSContext *cx,
			  JSObject *obj,
			  uintN argc,
			  jsval *argv,
			  jsval *rval);

JSBool
VrmlMatrixAddProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
VrmlMatrixGetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool
VrmlMatrixSetProperty(JSContext *cx,
				   JSObject *obj,
				   jsval id,
				   jsval *vp);

JSBool _standardMFAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval, JSClass *myClass, int type);
JSBool _standardMFGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp, char *makeNewElement, int type);
void printJSNodeType (JSContext *context, JSObject *myobj);

extern JSClass SFColorClass;
extern JSPropertySpec (SFColorProperties)[];
extern JSFunctionSpec (SFColorFunctions)[];
extern JSClass SFColorRGBAClass;
extern JSPropertySpec (SFColorRGBAProperties)[];
extern JSFunctionSpec (SFColorRGBAFunctions)[];
extern JSClass SFImageClass;
extern JSPropertySpec (SFImageProperties)[];
extern JSFunctionSpec (SFImageFunctions)[];
extern JSClass SFNodeClass;
extern JSPropertySpec (SFNodeProperties)[];
extern JSFunctionSpec (SFNodeFunctions)[];
extern JSClass SFRotationClass;
extern JSPropertySpec (SFRotationProperties)[];
extern JSFunctionSpec (SFRotationFunctions)[];
extern JSClass SFVec2fClass;
extern JSPropertySpec (SFVec2fProperties)[];
extern JSFunctionSpec (SFVec2fFunctions)[];
extern JSClass SFVec3fClass;
extern JSPropertySpec (SFVec3fProperties)[];
extern JSFunctionSpec (SFVec3fFunctions)[];
extern JSClass SFVec3dClass;
extern JSPropertySpec (SFVec3dProperties)[];
extern JSFunctionSpec (SFVec3dFunctions)[];


extern JSClass SFVec4fClass;
extern JSPropertySpec (SFVec4fProperties)[];
extern JSFunctionSpec (SFVec4fFunctions)[];
extern JSClass SFVec4dClass;
extern JSPropertySpec (SFVec4dProperties)[];
extern JSFunctionSpec (SFVec4dFunctions)[];

extern JSClass MFColorClass;
extern JSFunctionSpec (MFColorFunctions)[];
extern JSClass MFFloatClass;
extern JSFunctionSpec (MFFloatFunctions)[];
extern JSClass MFInt32Class;
extern JSFunctionSpec (MFInt32Functions)[];
extern JSClass MFNodeClass;
extern JSFunctionSpec (MFNodeFunctions)[];
extern JSClass MFRotationClass;
extern JSFunctionSpec (MFRotationFunctions)[];
extern JSClass MFStringClass;
extern JSFunctionSpec (MFStringFunctions)[];
extern JSClass MFTimeClass;
extern JSPropertySpec (MFTimeProperties)[] ;
extern JSFunctionSpec (MFTimeFunctions)[];
extern JSClass MFVec2fClass;
extern JSFunctionSpec (MFVec2fFunctions)[];
extern JSClass MFVec3fClass;
extern JSFunctionSpec (MFVec3fFunctions)[];
extern JSClass VrmlMatrixClass;
extern JSFunctionSpec (VrmlMatrixFunctions)[];

JSBool js_SetPropertyCheck (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_GetPropertyDebug (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug1 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug2 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug3 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug4 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug5 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug6 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug7 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug8 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug9 (JSContext *context, JSObject *obj, jsval id, jsval *vp);


#endif /*  JS_VRML_CLASSES */
