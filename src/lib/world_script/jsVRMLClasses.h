/*
=INSERT_TEMPLATE_HERE=

$Id: jsVRMLClasses.h,v 1.26 2012/04/21 21:21:40 dug9 Exp $

Complex VRML nodes as Javascript classes.

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


#ifndef __FREEWRL_JS_VRML_CLASSES_H__
#define __FREEWRL_JS_VRML_CLASSES_H__
//#ifdef HAVE_JAVASCRIPT
//# include <jsversion.h> //re-included for IDE readability below, should be included by jsapi.h elsewhere
//#endif

#ifndef UNUSED
#define UNUSED(v) ((void) v)
#endif

#define INIT_ARGC_NODE 1
#define INIT_ARGC 0

/* tie a node into the root. Currently not required, as we do a better job
of garbage collection 
... NOTE! JS_AddRoot and JS_RemoveRoot is DEPRECATED as of JS_VERSION 185*/
#define ADD_ROOT(a,b) \
	/* printf ("adding root  cx %u pointer %u value %u\n",a,&b,b); \
        if (JS_AddRoot(a,&b) != JS_TRUE) { \
                printf ("JA_AddRoot failed at %s:%d\n",__FILE__,__LINE__); \
                return JS_FALSE; \
        } */

#define REMOVE_ROOT(a,b) \
	/* printf ("removing root %u\n",b); \
        JS_RemoveRoot(a,&b);  */

//#define MF_LENGTH_FIELD "mf_len"
#define MF_LENGTH_FIELD "length"

#define DEFINE_LENGTH(this_context,this_object,this_length) \
	{jsval zimbo = INT_TO_JSVAL(this_length);\
	/* printf ("defining length to %d for %d %d\n",this_length,this_context,this_object);*/ \
	if (!JS_DefineProperty(this_context, this_object, MF_LENGTH_FIELD, zimbo, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) { \
		printf( "JS_DefineProperty failed for \"%s\" at %s:%d.\n",MF_LENGTH_FIELD,__FILE__,__LINE__); \
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


#define SET_JS_TICKTIME_RV(possibleRetVal) { jsval zimbo; \
        JS_NewNumberValue(cx, TickTime(), &zimbo);  \
        if (!JS_DefineProperty(cx,obj, "__eventInTickTime", zimbo, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) {  \
                printf( "JS_DefineProperty failed for \"__eventInTickTime\" at %s:%d.\n",__FILE__,__LINE__); \
                return possibleRetVal; \
        }}

#define SET_JS_TICKTIME() { jsval zimbo; \
        JS_NewNumberValue(cx, TickTime(), &zimbo);  \
        if (!JS_DefineProperty(cx,obj, "__eventInTickTime", zimbo, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB2, JSPROP_PERMANENT)) {  \
                printf( "JS_DefineProperty failed for \"__eventInTickTime\" at %s:%d.\n",__FILE__,__LINE__); \
                return; \
        }}

#if JS_VERSION < 185
#define COMPILE_FUNCTION_IF_NEEDED(tnfield) \
	if (JSparamnames[tnfield].eventInFunction == NULL) { \
		sprintf (scriptline,"%s(__eventIn_Value_%s,__eventInTickTime)", JSparamnames[tnfield].name,JSparamnames[tnfield].name); \
		/* printf ("compiling function %s\n",scriptline); */ \
		JSparamnames[tnfield].eventInFunction = (void *) JS_CompileScript( \
			cx, obj, scriptline, strlen(scriptline), "compile eventIn",1); \
	}
#else
#define COMPILE_FUNCTION_IF_NEEDED(tnfield) \
	if (JSparamnames[tnfield].eventInFunction == NULL) { \
		sprintf (scriptline,"%s(__eventIn_Value_%s,__eventInTickTime)", JSparamnames[tnfield].name,JSparamnames[tnfield].name); \
		/* printf ("compiling function %s\n",scriptline); */ \
		JSparamnames[tnfield].eventInFunction = JS_CompileScript( \
			cx, obj, scriptline, strlen(scriptline), "compile eventIn",1); \
		if (!JS_AddObjectRoot(cx,&(JSparamnames[tnfield].eventInFunction))) { \
			printf( "JS_AddObjectRoot failed for compilation of script \"%s\" at %s:%d.\n",scriptline,__FILE__,__LINE__); \
			return; \
		} \
	}
#endif
#define RUN_FUNCTION(tnfield) \
	{jsval zimbo; \
	if (!JS_ExecuteScript(cx, obj, JSparamnames[tnfield].eventInFunction, &zimbo)) { \
		printf ("failed to set parameter for eventIn %s in FreeWRL code %s:%d\n",JSparamnames[tnfield].name,__FILE__,__LINE__); \
		/* printf ("myThread is %u\n",pthread_self());*/ \
	}} 


#define SET_LENGTH(cx,newMFObject,length) \
	{ jsval lenval; \
                lenval = INT_TO_JSVAL(length); \
                if (!JS_SetProperty(cx, newMFObject,  MF_LENGTH_FIELD, &lenval)) { \
                        printf( "JS_SetProperty failed for \"%s\" at %s:%d\n", MF_LENGTH_FIELD,__FILE__,__LINE__); \
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
#if JS_VERSION < 185
JSBool doMFAddProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp, char *name); 
JSBool doMFSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp, int type); 
#else
JSBool doMFAddProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp, char *name); 
JSBool doMFSetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp, int type); 
#endif
JSBool getBrowser(JSContext *context, JSObject *obj, BrowserNative **brow); 
JSBool doMFStringUnquote(JSContext *cx, jsval *vp);


/* class functions */

JSBool
globalResolve(JSContext *cx,
			  JSObject *obj,
#if JS_VERSION < 185
			  jsval id);
#else
			  jsid id);
#endif

JSBool
loadVrmlClasses(JSContext *context,
				JSObject *globalObj);


JSBool
setECMANative(JSContext *cx,
			  JSObject *obj,
#if JS_VERSION < 185
			  jsval id,
#else
			  jsid id,
			  JSBool strict,
#endif
			  jsval *vp);


JSBool
getAssignProperty(JSContext *context,
				  JSObject *obj,
#if JS_VERSION < 185
				  jsval id,
#else
				  jsid id,
#endif
				  jsval *vp);

JSBool
setAssignProperty(JSContext *context,
				  JSObject *obj,
#if JS_VERSION < 185
				  jsval id,
#else
				  jsid id,
				  JSBool strict,
#endif
				  jsval *vp);



#if JS_VERSION < 185
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
#else
JSBool SFColorGetHSV(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorSetHSV(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorConstr(JSContext *cx, uintN argc, jsval *vp);
#endif

#if JS_VERSION < 185
JSBool SFColorGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp); 
JSBool SFColorSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
#else
JSBool SFColorGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp); 
JSBool SFColorSetProperty(JSContext *cx, JSObject *obj, jsid id, JSBool strict, jsval *vp);
#endif

#if JS_VERSION < 185
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
#else
JSBool SFColorRGBAGetHSV(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorRGBASetHSV(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorRGBAToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorRGBAAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFColorRGBAConstr(JSContext *cx, uintN argc, jsval *vp);
#endif

#if JS_VERSION < 185
JSBool SFColorRGBAGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp); 
JSBool SFColorRGBASetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
#else
JSBool SFColorRGBAGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp); 
JSBool SFColorRGBASetProperty(JSContext *cx, JSObject *obj, jsid id, JSBool strict, jsval *vp);
#endif

#if JS_VERSION < 185
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
#else
JSBool SFImageToString(JSContext *cx, uintN argc,jsval *vp);
JSBool SFImageAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFImageConstr(JSContext *cx, uintN argc, jsval *vp);
#endif

JSBool
SFImageGetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
#endif
				   jsval *vp);

JSBool
SFImageSetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
				   JSBool strict,
#endif
				   jsval *vp);



#if JS_VERSION < 185
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
#else
JSBool SFNodeToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFNodeAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFNodeConstr(JSContext *cx, uintN argc, jsval *vp);
#endif

void SFNodeFinalize(JSContext *cx, JSObject *obj);

JSBool
SFNodeGetProperty(JSContext *cx,
				  JSObject *obj,
#if JS_VERSION < 185
				  jsval id,
#else
				  jsid id,
#endif
				  jsval *vp);

JSBool
SFNodeSetProperty(JSContext *cx,
				  JSObject *obj,
#if JS_VERSION < 185
				  jsval id,
#else
				  jsid id,
				  JSBool strict,
#endif
				  jsval *vp);


#if JS_VERSION < 185
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

#else
JSBool SFRotationGetAxis(JSContext *cx, uintN argc, jsval *vp);
JSBool SFRotationInverse(JSContext *cx, uintN argc, jsval *vp); /* not implemented */
JSBool SFRotationMultiply(JSContext *cx, uintN argc, jsval *vp);
JSBool SFRotationMultVec(JSContext *cx, uintN argc, jsval *vp);
JSBool SFRotationSetAxis(JSContext *cx, uintN argc, jsval *vp);
JSBool SFRotationSlerp(JSContext *cx, uintN argc, jsval *vp);
JSBool SFRotationToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFRotationAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFRotationConstr(JSContext *cx, uintN argc, jsval *vp);
#endif


JSBool
SFRotationGetProperty(JSContext *cx,
					  JSObject *obj,
#if JS_VERSION < 185
					  jsval id,
#else
					  jsid id,
#endif
					  jsval *vp);

JSBool
SFRotationSetProperty(JSContext *cx,
					  JSObject *obj,
#if JS_VERSION < 185
					  jsval id,
#else
					  jsid id,
					  JSBool strict,
#endif
					  jsval *vp);


#if JS_VERSION < 185
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
#else
JSBool SFVec2fAdd(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fDivide(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fDot(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fLength(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fMultiply(JSContext *cx, uintN argc, jsval *vp);
/* JSBool SFVec2fNegate(JSContext *cx, uintN argc, jsval *vp); */
JSBool SFVec2fNormalize(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fSubtract(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec2fConstr(JSContext *cx, uintN argc, jsval *vp);
#endif

JSBool
SFVec2fGetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
#endif
				   jsval *vp);

JSBool
SFVec2fSetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
				   JSBool strict,
#endif
				   jsval *vp);


#if JS_VERSION < 185
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
#else
JSBool SFVec3fAdd(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fCross(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fDivide(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fDot(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fLength(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fMultiply(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fNegate(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fNormalize(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fSubtract(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3fGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp); 
JSBool SFVec3fSetProperty(JSContext *cx, JSObject *obj, jsid id, JSBool strict, jsval *vp);
#endif

#if JS_VERSION < 185
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
#else
JSBool SFVec3dAdd(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dCross(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dDivide(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dDot(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dLength(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dMultiply(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dNegate(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dNormalize(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dSubtract(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec3dGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp); 
JSBool SFVec3dSetProperty(JSContext *cx, JSObject *obj, jsid id, JSBool strict, jsval *vp);
#endif


#if JS_VERSION < 185
JSBool SFVec4fToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec4fAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec4fConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec4fGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp); 
JSBool SFVec4fSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
JSBool SFVec4dToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec4dAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec4dConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool SFVec4dGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp); 
JSBool SFVec4dSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
#else
JSBool SFVec4fToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec4fAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec4fConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec4fGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp); 
JSBool SFVec4fSetProperty(JSContext *cx, JSObject *obj, jsid id, JSBool strict, jsval *vp);
JSBool SFVec4dToString(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec4dAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec4dConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool SFVec4dGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp); 
JSBool SFVec4dSetProperty(JSContext *cx, JSObject *obj, jsid id, JSBool strict, jsval *vp);
#endif

#if JS_VERSION < 185
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
#else
JSBool MFColorToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFColorAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFColorConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFColorConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
#endif

JSBool
MFColorAddProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
#endif
				   jsval *vp);

JSBool
MFColorGetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
#endif
				   jsval *vp);

JSBool
MFColorSetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
				   JSBool strict,
#endif
				   jsval *vp);



#if JS_VERSION < 185
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
#else
JSBool MFFloatToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFFloatAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFFloatConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFFloatConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
#endif

JSBool
MFFloatAddProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
#endif
				   jsval *vp);

JSBool
MFFloatGetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
#endif
				   jsval *vp);

JSBool
MFFloatSetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
                                   JSBool strict,
#endif
				   jsval *vp);



#if JS_VERSION < 185
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
#else
JSBool MFInt32ToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFInt32Assign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFInt32Constr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFInt32ConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
#endif

JSBool
MFInt32AddProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
#endif
				   jsval *vp);

JSBool
MFInt32GetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
#endif
				   jsval *vp);

JSBool
MFInt32SetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsid id,
#else
				   jsid id,
                                   JSBool strict,
#endif
				   jsval *vp);


#if JS_VERSION < 185
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
#else
JSBool MFNodeToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFNodeAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFNodeConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFNodeConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
#endif

JSBool
MFNodeAddProperty(JSContext *cx,
				  JSObject *obj,
#if JS_VERSION < 185
				  jsval id,
#else
				  jsid id,
#endif
				  jsval *vp);

JSBool
MFNodeGetProperty(JSContext *cx,
				  JSObject *obj,
#if JS_VERSION < 185
				  jsval id,
#else
				  jsid id,
#endif
				  jsval *vp);

JSBool
MFNodeSetProperty(JSContext *cx,
				  JSObject *obj,
#if JS_VERSION < 185
				  jsval id,
#else
				  jsid id,
                                  JSBool strict,
#endif
				  jsval *vp);



#if JS_VERSION < 185
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
#else
JSBool MFRotationToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFRotationAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFRotationConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFRotationConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
#endif

JSBool
MFRotationGetProperty(JSContext *cx,
					  JSObject *obj,
#if JS_VERSION < 185
					  jsval id,
#else
					  jsid id,
#endif
					  jsval *vp);

JSBool
MFRotationSetProperty(JSContext *cx,
					  JSObject *obj,
#if JS_VERSION < 185
					  jsval id,
#else
					  jsid id,
                                          JSBool strict,
#endif
					  jsval *vp);

JSBool
MFRotationAddProperty(JSContext *cx,
					  JSObject *obj,
#if JS_VERSION < 185
					  jsval id,
#else
					  jsid id,
#endif
					  jsval *vp);


#if JS_VERSION < 185
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
#else
JSBool MFStringToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFStringAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFStringConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFStringConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
#endif

JSBool
MFStringGetProperty(JSContext *cx,
					JSObject *obj,
#if JS_VERSION < 185
					jsval id,
#else
					jsid id,
#endif
					jsval *vp);

JSBool
MFStringSetProperty(JSContext *cx,
					JSObject *obj,
#if JS_VERSION < 185
					jsval id,
#else
					jsid id,
                                        JSBool strict,
#endif
					jsval *vp);


JSBool
MFStringAddProperty(JSContext *cx,
					JSObject *obj,
#if JS_VERSION < 185
					jsval id,
#else
					jsid id,
#endif
					jsval *vp);

JSBool MFStringEnumerateProperty(JSContext *cx, JSObject *obj) ;
#if JS_VERSION < 185
JSBool MFStringDeleteProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) ;
JSBool MFStringResolveProperty(JSContext *cx, JSObject *obj, jsval id) ;
#else
JSBool MFStringDeleteProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) ;
JSBool MFStringResolveProperty(JSContext *cx, JSObject *obj, jsid id) ;
#endif
JSBool MFStringConvertProperty(JSContext *cx, JSObject *obj, JSType type, jsval *vp) ;
       


#if JS_VERSION < 185
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
#else
JSBool MFTimeToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFTimeAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFTimeConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFTimeConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
#endif

JSBool
MFTimeAddProperty(JSContext *cx,
				  JSObject *obj,
#if JS_VERSION < 185
				  jsval id,
#else
				  jsid id,
#endif
				  jsval *vp);

JSBool
MFTimeGetProperty(JSContext *cx,
				  JSObject *obj,
#if JS_VERSION < 185
				  jsval id,
#else
				  jsid id,
#endif
				  jsval *vp);

JSBool
MFTimeSetProperty(JSContext *cx,
				  JSObject *obj,
#if JS_VERSION < 185
				  jsval id,
#else
				  jsid id,
                                  JSBool strict,
#endif
				  jsval *vp);



#if JS_VERSION < 185
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
#else
JSBool MFVec2fToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFVec2fAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFVec2fConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFVec2fConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
#endif

JSBool
MFVec2fAddProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
#endif
				   jsval *vp);

JSBool
MFVec2fGetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
#endif
				   jsval *vp);

JSBool
MFVec2fSetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
                                   JSBool strict,
#endif
				   jsval *vp);



#if JS_VERSION < 185
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
#else
JSBool MFVec3fToString(JSContext *cx, uintN argc, jsval *vp);
JSBool MFVec3fAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool MFVec3fConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool MFVec3fConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
#endif

JSBool
MFVec3fAddProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
#endif
				   jsval *vp);

JSBool
MFVec3fGetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
#endif
				   jsval *vp);

JSBool
MFVec3fSetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
                                   JSBool strict,
#endif
				   jsval *vp);

#if JS_VERSION < 185
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
#else
JSBool VrmlMatrixToString(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixAssign(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixsetTransform(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixgetTransform(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixinverse(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixtranspose(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixmultLeft(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixmultRight(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixmultVecMatrix(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixmultMatrixVec(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixConstr(JSContext *cx, uintN argc, jsval *vp);
JSBool VrmlMatrixConstrInternals(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
#endif

JSBool
VrmlMatrixAddProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
#endif
				   jsval *vp);

JSBool
VrmlMatrixGetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
#endif
				   jsval *vp);

JSBool
VrmlMatrixSetProperty(JSContext *cx,
				   JSObject *obj,
#if JS_VERSION < 185
				   jsval id,
#else
				   jsid id,
				   JSBool strict,
#endif
				   jsval *vp);

JSBool _standardMFAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval, JSClass *myClass, int type);
#if JS_VERSION < 185
JSBool _standardMFGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp, char *makeNewElement, int type);
#else
JSBool _standardMFGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp, char *makeNewElement, int type);
#endif
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
extern JSClass MFBoolClass;
extern JSFunctionSpec (MFBoolFunctions)[];
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

#if JS_VERSION < 185
JSBool js_GetPropertyDebug (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyCheck (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug1 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug2 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug3 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug4 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug5 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug6 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug7 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug8 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
JSBool js_SetPropertyDebug9 (JSContext *context, JSObject *obj, jsval id, jsval *vp);
#else
JSBool js_GetPropertyDebug (JSContext *context, JSObject *obj, jsid id, jsval *vp);
JSBool js_SetPropertyCheck (JSContext *context, JSObject *obj, jsid id, JSBool strict, jsval *vp);
JSBool js_SetPropertyDebug1 (JSContext *context, JSObject *obj, jsid id, JSBool strict, jsval *vp);
JSBool js_SetPropertyDebug2 (JSContext *context, JSObject *obj, jsid id, JSBool strict, jsval *vp);
JSBool js_SetPropertyDebug3 (JSContext *context, JSObject *obj, jsid id, JSBool strict, jsval *vp);
JSBool js_SetPropertyDebug4 (JSContext *context, JSObject *obj, jsid id, JSBool strict, jsval *vp);
JSBool js_SetPropertyDebug5 (JSContext *context, JSObject *obj, jsid id, JSBool strict, jsval *vp);
JSBool js_SetPropertyDebug6 (JSContext *context, JSObject *obj, jsid id, JSBool strict, jsval *vp);
JSBool js_SetPropertyDebug7 (JSContext *context, JSObject *obj, jsid id, JSBool strict, jsval *vp);
JSBool js_SetPropertyDebug8 (JSContext *context, JSObject *obj, jsid id, JSBool strict, jsval *vp);
JSBool js_SetPropertyDebug9 (JSContext *context, JSObject *obj, jsid id, JSBool strict, jsval *vp);
#endif

#endif /*  JS_VRML_CLASSES */
