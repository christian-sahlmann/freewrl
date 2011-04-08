/*
=INSERT_TEMPLATE_HERE=

$Id: jsVRML_MFClasses.c,v 1.26 2011/04/08 19:20:50 istakenv Exp $

???

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
#include <system_threads.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../main/Snapshot.h"
#include "../scenegraph/Collision.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"
#include "../input/SensInterps.h"
#include "../x3d_parser/Bindable.h"
#include "../scenegraph/LinearAlgebra.h"

#include "JScript.h"
#include "CScripts.h"
#include "jsUtils.h"
#include "jsNative.h"
#include "jsVRMLClasses.h"
#include "JScript.h"

#ifdef HAVE_JAVASCRIPT

/********************************************************/
/*							*/
/* Third part - MF classes				*/
/*							*/
/********************************************************/

/* remove any private data from this datatype, and let the garbage collector handle the object */

void
JS_MY_Finalize(JSContext *cx, JSObject *obj)
{
	void *ptr;
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("finalizing %x\n",obj);
	printJSNodeType(cx,obj);
	#endif

	REMOVE_ROOT(cx,obj)

	if ((ptr = (void *)JS_GetPrivate(cx, obj)) != NULL) {
		FREE_IF_NZ(ptr);
	#ifdef JSVRMLCLASSESVERBOSE
	} else {
		printf ("Finalize - no private data!\n");
	#endif
	}
}

JSBool
MFColorToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	UNUSED(argc);
	UNUSED(argv);
	return doMFToString(cx, obj, "MFColor", rval);
}

JSBool
MFColorAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	return _standardMFAssign (cx, obj, argc, argv, rval, &MFColorClass,FIELDTYPE_SFColor);
}

JSBool
MFColorConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	JSObject *_obj;
	unsigned int i;
	
	ADD_ROOT(cx,obj)
	DEFINE_LENGTH(argc,obj)

	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFColorConstr: obj = %p, %u args\n",
			   obj, argc);
	#endif

	for (i = 0; i < argc; i++) {
		if (!JS_ValueToObject(cx, argv[i], &_obj)) {
			printf(
					"JS_ValueToObject failed in MFColorConstr.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,_obj,NULL,__FUNCTION__,SFColorClass)

		if (!JS_DefineElement(cx, obj, (jsint) i, argv[i], JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
			printf( "JS_DefineElement failed for arg %u in MFColorConstr.\n", i);
			return JS_FALSE;
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFColorAddProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return doMFAddProperty(cx, obj, id, vp,"MFColorAddProperty");
}

JSBool
MFColorGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return _standardMFGetProperty(cx, obj, id, vp,
			"_FreeWRL_Internal = new SFColor()", FIELDTYPE_MFColor);
}

JSBool
MFColorSetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFColor);
}

JSBool
MFFloatToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	UNUSED(argc);
	UNUSED(argv);
	return doMFToString(cx, obj, "MFFloat", rval);
}

JSBool
MFFloatAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	SET_MF_ECMA_HAS_CHANGED

	return _standardMFAssign (cx, obj, argc, argv, rval, &MFFloatClass,FIELDTYPE_SFFloat);
}

JSBool
MFFloatConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	jsdouble _d;
	unsigned int i;

	ADD_ROOT(cx,obj)
	DEFINE_LENGTH (argc,obj)
	DEFINE_MF_ECMA_HAS_CHANGED

	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFFloatConstr: obj = %p, %u args\n", obj, argc);
	#endif
	for (i = 0; i < argc; i++) {
		if (!JS_ValueToNumber(cx, argv[i], &_d)) {
			printf( "JS_ValueToNumber failed in MFFloatConstr.\n");
			return JS_FALSE;
		}

		if (!JS_DefineElement(cx, obj, (jsint) i, argv[i], JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
			printf( "JS_DefineElement failed for arg %u in MFFloatConstr.\n", i);
			return JS_FALSE;
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFFloatAddProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return doMFAddProperty(cx, obj, id, vp,"MFFloatAddProperty");
}

JSBool
MFFloatGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return _standardMFGetProperty(cx, obj, id, vp,
			"_FreeWRL_Internal = 0.0", FIELDTYPE_MFFloat);
}

JSBool
MFFloatSetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFFloat);
}


JSBool
MFInt32ToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	UNUSED(argc);
	UNUSED(argv);
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFInt32ToString\n");
	#endif

	return doMFToString(cx, obj, "MFInt32", rval);
}

JSBool
MFInt32Assign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFInt32Assign\n");
	#endif

	SET_MF_ECMA_HAS_CHANGED

	return _standardMFAssign (cx, obj, argc, argv, rval, &MFInt32Class,FIELDTYPE_SFInt32);
}


JSBool
MFInt32Constr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	int32 _i;
	unsigned int i;
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFInt32Constr\n");
	#endif

	ADD_ROOT(cx,obj)
        DEFINE_LENGTH(argc,obj)
        DEFINE_MF_ECMA_HAS_CHANGED
	
	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFInt32Constr: obj = %p, %u args\n", obj, argc);
	#endif

	/* any values here that we should add in? */
	for (i = 0; i < argc; i++) {
		if (!JS_ValueToInt32(cx, argv[i], &_i)) {
			printf( "JS_ValueToBoolean failed in MFInt32Constr.\n");
			return JS_FALSE;
		}
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("value at %d is %d\n",i,_i);
		#endif

		if (!JS_DefineElement(cx, obj, (jsint) i, argv[i], JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
			printf( "JS_DefineElement failed for arg %u in MFInt32Constr.\n", i);
			return JS_FALSE;
		}
	}

	*rval = OBJECT_TO_JSVAL(obj);

	return JS_TRUE;
}

JSBool
MFInt32AddProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFInt32AddProperty\n");
	#endif

	return doMFAddProperty(cx, obj, id, vp,"MFInt32AddProperty");
}

JSBool
MFInt32GetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFInt32GetProperty\n");
	#endif

	return _standardMFGetProperty(cx, obj, id, vp,
			"_FreeWRL_Internal = 0", FIELDTYPE_MFInt32);
}

JSBool
MFInt32SetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFInt32SetProperty\n");
	#endif

	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFInt32);
}


JSBool
MFNodeToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	UNUSED(argc);
	UNUSED(argv);
	
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFNODETOSTRING, obj %d\n",obj);
	#endif
	return doMFToString(cx, obj, "MFNode", rval);
}

JSBool
MFNodeAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFNODEASSIGN, obj %d\n",obj);
	#endif

	return _standardMFAssign (cx, obj, argc, argv, rval, &MFNodeClass,FIELDTYPE_SFNode);
}

JSBool
MFNodeConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	JSObject *_obj;
	unsigned int i;

	ADD_ROOT(cx,obj)
	DEFINE_LENGTH(argc,obj)

	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFNodeConstr: obj = %p, %u args\n", obj, argc);
	#endif

	for (i = 0; i < argc; i++) {
		if (JSVAL_IS_OBJECT(argv[i])) {

			if (!JS_ValueToObject(cx, argv[i], &_obj)) {
				printf( "JS_ValueToObject failed in MFNodeConstr.\n");
				return JS_FALSE;
			}

			CHECK_CLASS(cx,_obj,argv,__FUNCTION__,SFNodeClass)

			if (!JS_DefineElement(cx, obj, (jsint) i, argv[i], JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed for arg %d in MFNodeConstr.\n", i);
				return JS_FALSE;
			}
		} else {
			/* if a NULL is passed in, eg, we have a script with an MFNode eventOut, and
			   nothing sets it, we have a NULL here. Lets just ignore it */
			/* hmmm - this is not an object - lets see... */
			#ifdef JSVRMLCLASSESVERBOSE
			if (JSVAL_IS_NULL(argv[i])) { printf ("MFNodeConstr - its a NULL\n");}
			if (JSVAL_IS_INT(argv[i])) { printf ("MFNodeConstr - its a INT\n");}
			if (JSVAL_IS_STRING(argv[i])) { printf ("MFNodeConstr - its a STRING\n");}
			#endif
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFNodeAddProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("startof MFNODEADDPROPERTY\n");
	#endif
	return doMFAddProperty(cx, obj, id, vp,"MFNodeAddProperty");
}

JSBool
MFNodeGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("start of MFNODEGETPROPERTY obj %d\n");
	#endif
	return _standardMFGetProperty(cx, obj, id, vp,
			"_FreeWRL_Internal = 0",
			FIELDTYPE_MFNode);
}

JSBool
MFNodeSetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	/* printf ("start of MFNODESETPROPERTY obj %d\n",obj); */
	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFNode);
}


JSBool
MFTimeAddProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return doMFAddProperty(cx, obj, id, vp,"MFTimeAddProperty");
}

JSBool
MFTimeGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return _standardMFGetProperty(cx, obj, id, vp,
			 "_FreeWRL_Internal = 0.0",
			FIELDTYPE_MFTime);
}

JSBool
MFTimeSetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFTime);
}

JSBool
MFTimeToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	UNUSED(argc);
	UNUSED(argv);
	return doMFToString(cx, obj, "MFTime", rval);
}

JSBool
MFTimeConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	jsdouble _d;
	unsigned int i;

	ADD_ROOT(cx,obj)
	DEFINE_LENGTH(argc,obj)
	DEFINE_MF_ECMA_HAS_CHANGED

	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFTimeConstr: obj = %p, %u args\n", obj, argc);
	#endif
	for (i = 0; i < argc; i++) {
		if (!JS_ValueToNumber(cx, argv[i], &_d)) {
			printf(
					"JS_ValueToNumber failed in MFTimeConstr.\n");
			return JS_FALSE;
		}

		if (!JS_DefineElement(cx, obj, (jsint) i, argv[i], JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
			printf( "JS_DefineElement failed for arg %u in MFTimeConstr.\n", i);
			return JS_FALSE;
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFTimeAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	SET_MF_ECMA_HAS_CHANGED

	return _standardMFAssign (cx, obj, argc, argv, rval, &MFTimeClass,FIELDTYPE_SFTime);
}



JSBool
MFVec2fAddProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return doMFAddProperty(cx, obj, id, vp,"MFVec2fAddProperty");
}

JSBool
MFVec2fGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return _standardMFGetProperty(cx, obj, id, vp,
			 "_FreeWRL_Internal = new SFVec2f()",FIELDTYPE_MFVec2f);
}

JSBool
MFVec2fSetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFVec2f);
}

JSBool
MFVec2fToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	UNUSED(argc);
	UNUSED(argv);
	return doMFToString(cx, obj, "MFVec2f", rval);
}

JSBool
MFVec2fConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	JSObject *_obj;
	unsigned int i;

	ADD_ROOT(cx,obj)
	DEFINE_LENGTH(argc,obj)

	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFVec2fConstr: obj = %p, %u args\n", obj, argc);
	#endif

	for (i = 0; i < argc; i++) {
		if (!JS_ValueToObject(cx, argv[i], &_obj)) {
			printf( "JS_ValueToObject failed in MFVec2fConstr.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,_obj,NULL,__FUNCTION__,SFVec2fClass)

		if (!JS_DefineElement(cx, obj, (jsint) i, argv[i], JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
			printf( "JS_DefineElement failed for arg %d in MFVec2fConstr.\n", i);
			return JS_FALSE;
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFVec2fAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	return _standardMFAssign (cx, obj, argc, argv, rval, &MFVec2fClass,FIELDTYPE_SFVec2f);
}

/* MFVec3f */
JSBool
MFVec3fAddProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return doMFAddProperty(cx, obj, id, vp,"MFVec3fAddProperty");
}

JSBool
MFVec3fGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return _standardMFGetProperty(cx, obj, id, vp,
			 "_FreeWRL_Internal = new SFVec3f()",FIELDTYPE_MFVec3f);
}

JSBool
MFVec3fSetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFVec3f);
}

JSBool
MFVec3fToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	UNUSED(argc);
	UNUSED(argv);
	/* printf ("CALLED MFVec3fToString\n");*/
	return doMFToString(cx, obj, "MFVec3f", rval);
}

JSBool
MFVec3fConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	JSObject *_obj;
	unsigned int i;

	ADD_ROOT(cx,obj)
	DEFINE_LENGTH(argc,obj)

	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFVec3fConstr: obj = %p, %u args\n", obj, argc);
	#endif	
	for (i = 0; i < argc; i++) {
		if (!JS_ValueToObject(cx, argv[i], &_obj)) {
			printf( "JS_ValueToObject failed in MFVec3fConstr.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,_obj,NULL,__FUNCTION__,SFVec3fClass)

		if (!JS_DefineElement(cx, obj, (jsint) i, argv[i], JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
			printf( "JS_DefineElement failed for arg %d in MFVec3fConstr.\n", i);
			return JS_FALSE;
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFVec3fAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	return _standardMFAssign (cx, obj, argc, argv, rval, &MFVec3fClass,FIELDTYPE_SFVec3f);
}

/* VrmlMatrix */

static void _setmatrix (JSContext *cx, JSObject *obj, double *matrix) {
	jsval val;
	int i;
	for (i=0; i<16; i++) {

		if (JS_NewNumberValue(cx, matrix[i],&val) == JS_FALSE) {
			printf ("problem creating id matrix\n");
			return;
		}

		if (!JS_SetElement(cx, obj, (jsint) i, &val)) {
			printf( "JS_DefineElement failed for arg %u in VrmlMatrixSetTransform.\n", i);
			return;
		}
	}
}

/* get the matrix values into a double array */
static void _getmatrix (JSContext *cx, JSObject *obj, double *fl) {
	int32 _length;
	jsval _length_val;
	jsval val;
	int i;
	double d;

	if (!JS_GetProperty(cx, obj, "length", &_length_val)) {
		printf( "JS_GetProperty failed for \"length\" in _getmatrix.\n");
		_length = 0;
	} else {
		_length = JSVAL_TO_INT(_length_val);
	}

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("_getmatrix, length %d\n",_length);
	#endif


	if (_length>16) _length = 16;

	for (i = 0; i < _length; i++) {
		if (!JS_GetElement(cx, obj, (jsint) i, &val)) {
			printf( "failed in get of copyElements index %d.\n", i);
			fl[i] = 0.0;
		} else {
			if (!JS_ValueToNumber(cx, val, &d)) {
				printf ("this is not a mumber!\n");
				fl[i]=0.0;
			} else fl[i]=d;
		}
	}

	/* in case our matrix was short for some reason */
	for (i=_length; i < 16; i++) {
		fl[i]=0.0;
	}
}


JSBool
VrmlMatrixToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	UNUSED(argc);
	UNUSED(argv);

	return doMFToString(cx, obj, "MFFloat", rval);
}

/* get rows; used for scale and rot in getTransform */
void _get4f(double *ret, double *mat, int row) {
	if (row == 0) {ret[0]=MAT00;ret[1]=MAT01;ret[2]=MAT02;ret[3]=MAT03;}
	if (row == 1) {ret[0]=MAT10;ret[1]=MAT11;ret[2]=MAT12;ret[3]=MAT13;}
	if (row == 2) {ret[0]=MAT20;ret[1]=MAT21;ret[2]=MAT22;ret[3]=MAT23;}
}

/* set rows; used for scale and rot in getTransform */
void _set4f(double len, double *mat, int row) {
	if (row == 0) {MAT00=MAT00/len;MAT01=MAT01/len;MAT02=MAT02/len;MAT03=MAT03/len;}
	if (row == 1) {MAT10=MAT10/len;MAT11=MAT11/len;MAT12=MAT12/len;MAT13=MAT13/len;}
	if (row == 2) {MAT20=MAT20/len;MAT21=MAT21/len;MAT22=MAT22/len;MAT23=MAT23/len;}
}

JSBool
VrmlMatrixgetTransform(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	int i;
    	JSObject *transObj = NULL;
	JSObject *rotObj = NULL;
	JSObject *scaleObj = NULL;
	SFRotationNative *Rptr;
	SFVec3fNative *Vptr;

    	Quaternion quat;
    	double matrix[16];
    	double qu[4];
	double r0[4], r1[4], r2[4];
	double l0,l1,l2;

	/* some intermediate calculations */
	_getmatrix(cx,obj,matrix);
	/* get each row */
	_get4f(r0,matrix,0);
	_get4f(r1,matrix,1);
	_get4f(r2,matrix,2);
	/* get the length of each row */
	l0 = sqrt(r0[0]*r0[0] + r0[1]*r0[1] + r0[2]*r0[2] +r0[3]*r0[3]);
	l1 = sqrt(r1[0]*r1[0] + r1[1]*r1[1] + r1[2]*r1[2] +r1[3]*r1[3]);
	l2 = sqrt(r2[0]*r2[0] + r2[1]*r2[1] + r2[2]*r2[2] +r2[3]*r2[3]);

	if (argc == 1) {
		if (!JS_ConvertArguments(cx, argc, argv, "o", &transObj)) {
			printf ("getTransform, invalid parameters\n");
			return JS_FALSE;
		}
	}
	if (argc == 2) {
		if (!JS_ConvertArguments(cx, argc, argv, "o o", &transObj, &rotObj)) {
			printf ("getTransform, invalid parameters\n");
			return JS_FALSE;
		}
	}
	if (argc == 3) {
		if (!JS_ConvertArguments(cx, argc, argv, "o o o",
					&transObj,&rotObj,&scaleObj)) {
			printf ("getTransform, invalid parameters\n");
			return JS_FALSE;
		}
	}

	/* translation */
	if (transObj!=NULL) {
		CHECK_CLASS(cx,transObj,NULL,__FUNCTION__,SFVec3fClass)

		if ((Vptr = (SFVec3fNative *)JS_GetPrivate(cx, transObj)) == NULL) {
			printf( "JS_GetPrivate failed.\n");
			return JS_FALSE;
		}
		(Vptr->v).c[0] = (float) matrix[12];
		(Vptr->v).c[1] = (float) matrix[13];
		(Vptr->v).c[2] = (float) matrix[14];
		Vptr->valueChanged++;
	}

	/* rotation */
	if (rotObj!=NULL) {

		CHECK_CLASS(cx,rotObj,NULL,__FUNCTION__,SFRotationClass)

		if ((Rptr = (SFRotationNative*)JS_GetPrivate(cx, rotObj)) == NULL) {
			printf( "JS_GetPrivate failed.\n");
			return JS_FALSE;
		}

		/* apply length to each row */
		_set4f(l0, matrix, 0);
		_set4f(l1, matrix, 1);
		_set4f(l2, matrix, 2);

		/* convert the matrix to a quaternion */
		matrix_to_quaternion (&quat, matrix);
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("quaternion %f %f %f %f\n",quat.x,quat.y,quat.z,quat.w);
		#endif

		/* convert the quaternion to a VRML rotation */
		quaternion_to_vrmlrot(&quat, &qu[0],&qu[1],&qu[2],&qu[3]);

		/* now copy the values over */
		for (i=0; i<4; i++) (Rptr->v).c[i] = (float) qu[i];
		Rptr->valueChanged = 1;
	}

	/* scale */
	if (scaleObj != NULL) {
		CHECK_CLASS(cx,scaleObj,NULL,__FUNCTION__,SFVec3fClass)

		if ((Vptr = (SFVec3fNative*)JS_GetPrivate(cx, scaleObj)) == NULL) {
			printf( "JS_GetPrivate failed.\n");
			return JS_FALSE;
		}
		(Vptr->v).c[0] = (float) l0;
		(Vptr->v).c[1] = (float) l1;
		(Vptr->v).c[2] = (float) l2;
		Vptr->valueChanged = 1;
	}

	*rval = JSVAL_VOID;
	return JS_TRUE;
}


/* Sets the VrmlMatrix to the passed values. Any of the rightmost parameters may be omitted. 
   The method has 0 to 5 parameters. For example, specifying 0 parameters results in an 
   identity matrix while specifying 1 parameter results in a translation and specifying 2 
   parameters results in a translation and a rotation. Any unspecified parameter is set to 
   its default as specified for the Transform node. */

JSBool
VrmlMatrixsetTransform(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    	JSObject *transObj = NULL;
	JSObject *rotObj = NULL;
	JSObject *scaleObj = NULL;
	JSObject *scaleOObj = NULL;
	JSObject *centerObj = NULL;

    	double matrix[16];

	int error = FALSE;

#undef TESTING
#ifdef TESTING
	GLDOUBLE xxmat[16];
	FW_GL_MATRIX_MODE(GL_MODELVIEW);
	FW_GL_PUSH_MATRIX();
	FW_GL_LOAD_IDENTITY();
#endif


	/* set the identity for this matrix. We work on this matrix, then assign it to the variable */
	loadIdentityMatrix(matrix);

	/* first, is this a VrmlMatrix object? The chances of this failing are slim to none... */
	if (!JS_InstanceOf(cx, obj, &VrmlMatrixClass, NULL)) {
		error = TRUE;
	} else {
		if (argc == 1) {
			error = !JS_ConvertArguments(cx, argc, argv, "o", &transObj); 
		}
		if (argc == 2) {
			error = !JS_ConvertArguments(cx, argc, argv, "o o", &transObj,
				&rotObj);
		}
		if (argc == 3) {
			error = !JS_ConvertArguments(cx, argc, argv, "o o o",
				&transObj,&rotObj,&scaleObj);
		}
		if (argc == 4) {
			error = !JS_ConvertArguments(cx, argc, argv, "o o o o",
				&transObj,&rotObj,&scaleObj,&scaleOObj);
		}
		if (argc == 5) {
			error = !JS_ConvertArguments(cx, argc, argv, "o o o o o",
				&transObj,&rotObj,&scaleObj,&scaleOObj,&centerObj);
		}
		if (argc > 5) { error = TRUE; }
	}

	if (error) {
		ConsoleMessage ("setTransform: error in parameters");
		return JS_FALSE;
	}

	/* verify that we have the correct objects here */
	if (transObj != NULL) 
		error = !JS_InstanceOf(cx, transObj, &SFVec3fClass, NULL);
	if (!error && (rotObj != NULL)) 
		error = !JS_InstanceOf(cx, rotObj, &SFRotationClass, NULL);
	if (!error && (scaleObj != NULL)) 
		error = !JS_InstanceOf(cx, scaleObj, &SFVec3fClass, NULL);
	if (!error && (scaleOObj != NULL)) 
		error = !JS_InstanceOf(cx, scaleOObj, &SFRotationClass, NULL);
	if (!error && centerObj != NULL) 
		error = !JS_InstanceOf(cx, centerObj, &SFVec3fClass, NULL);

	if (error) {
		ConsoleMessage ("setTransform: at least one parameter incorrect type");
		return JS_FALSE;
	}

	/* apply Transform, if requested */
	if (transObj) {
		SFVec3fNative * Vptr;
		Vptr = (SFVec3fNative *)JS_GetPrivate(cx, transObj);
		error = (Vptr == NULL);
	
		if (!error) {
                	matrix[12]=Vptr->v.c[0];
                	matrix[13]=Vptr->v.c[1];
                	matrix[14]=Vptr->v.c[2];
		}
	}

	if (!error && (rotObj != NULL)) {
		SFRotationNative * Rptr;
                Rptr = (SFRotationNative *)JS_GetPrivate(cx, rotObj);
		error = (Rptr == NULL);
	
		if (!error) {
			Quaternion quat;
			vrmlrot_to_quaternion(&quat, Rptr->v.c[0], Rptr->v.c[1], Rptr->v.c[2], Rptr->v.c[3]);
			/* printf ("from rotation %f %f %f %f\n",Rptr->v.c[0], Rptr->v.c[1], Rptr->v.c[2], Rptr->v.c[3]);
			printf ("quaternion is %f %f %f %f\n",quat.x,quat.y,quat.x, quat.w); */
			quaternion_to_matrix (matrix, &quat);
		}
	}

	if (!error && (scaleObj != NULL)) {
		SFVec3fNative * Vptr;
                Vptr = (SFVec3fNative *)JS_GetPrivate(cx, scaleObj);
		error = (Vptr == NULL);

		if (!error) {
			struct point_XYZ myScale;

			COPY_SFVEC3F_TO_POINT_XYZ (myScale,Vptr->v.c);
			scale_to_matrix(matrix, &myScale);
		}

	}

	/* place the new values into the vrmlMatrix array */
	_setmatrix (cx, obj, matrix);

#ifdef TESTING
       printf ("calculated Matrix: \n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n",
                matrix[0],  matrix[4],  matrix[ 8],  matrix[12],
                matrix[1],  matrix[5],  matrix[ 9],  matrix[13],
                matrix[2],  matrix[6],  matrix[10],  matrix[14],
                matrix[3],  matrix[7],  matrix[11],  matrix[15]);
	glGetDoublev(GL_MODELVIEW,xxmat);
       printf ("modelview Matrix: \n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n",
                xxmat[0],  xxmat[4],  xxmat[ 8],  xxmat[12],
                xxmat[1],  xxmat[5],  xxmat[ 9],  xxmat[13],
                xxmat[2],  xxmat[6],  xxmat[10],  xxmat[14],
                xxmat[3],  xxmat[7],  xxmat[11],  xxmat[15]);
	FW_GL_POP_MATRIX();
#endif
	
	return JS_TRUE;
}


JSBool
VrmlMatrixinverse(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	double src[16];
	double dest[16];
	JSObject *retObj;
	UNUSED (argv);

	if (argc != 0) {
		printf ("VrmlMatrix, expect 0 parameters\n");
		return JS_FALSE;
	}
	_getmatrix (cx, obj,src);
	matinverse (dest,src);

        retObj = JS_ConstructObject(cx,&VrmlMatrixClass,NULL, NULL);

        _setmatrix(cx,retObj,dest);
        *rval = OBJECT_TO_JSVAL(retObj);
	return JS_TRUE;
}


JSBool
VrmlMatrixtranspose(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	double src[16];
	double dest[16];
	JSObject *retObj;
	UNUSED (argv);

	if (argc != 0) {
		printf ("VrmlMatrix, expect 0 parameters\n");
		return JS_FALSE;
	}
	_getmatrix (cx, obj,src);
	mattranspose (dest,src);

        retObj = JS_ConstructObject(cx,&VrmlMatrixClass,NULL, NULL);

        _setmatrix(cx,retObj,dest);
        *rval = OBJECT_TO_JSVAL(retObj);
	return JS_TRUE;
}



JSBool
VrmlMatrixmultLeft(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
        JSObject *transObj = NULL;
	JSObject *retObj = NULL;

        double matrix1[16];
        double matrix2[16];
        int error = FALSE;

	if (argc == 1) {
		error = !JS_ConvertArguments(cx, argc, argv, "o", &transObj);
	} else error = TRUE;

	if (!error) if (!JS_InstanceOf(cx, transObj, &VrmlMatrixClass, NULL)) { error = TRUE;}	

	if (error) {
		ConsoleMessage ("VrmlMatrixMultLeft, error in params");
		return JS_FALSE;
	}

	/* fill in the 2 matricies, multiply them, then return it */
	_getmatrix(cx,obj,matrix1);
	_getmatrix(cx,transObj,matrix2);
	matmultiply(matrix1,matrix1,matrix2);

	retObj = JS_ConstructObject(cx,&VrmlMatrixClass,NULL, NULL);

	/*
       printf ("multLeft calculated Matrix: \n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n",
                matrix1[0],  matrix1[4],  matrix1[ 8],  matrix1[12],
                matrix1[1],  matrix1[5],  matrix1[ 9],  matrix1[13],
                matrix1[2],  matrix1[6],  matrix1[10],  matrix1[14],
                matrix1[3],  matrix1[7],  matrix1[11],  matrix1[15]);
	*/
	_setmatrix(cx,retObj,matrix1);
	*rval = OBJECT_TO_JSVAL(retObj);

	return JS_TRUE;
}

JSBool
VrmlMatrixmultRight(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
        JSObject *transObj = NULL;
	JSObject *retObj = NULL;

        double matrix1[16];
        double matrix2[16];
        int error = FALSE;

	if (argc == 1) {
		error = !JS_ConvertArguments(cx, argc, argv, "o", &transObj);
	} else error = TRUE;

	if (!error) if (!JS_InstanceOf(cx, transObj, &VrmlMatrixClass, NULL)) { error = TRUE;}	

	if (error) {
		ConsoleMessage ("VrmlMatrixMultRight, error in params");
		return JS_FALSE;
	}

	/* fill in the 2 matricies, multiply them, then return it */
	_getmatrix(cx,obj,matrix1);
	_getmatrix(cx,transObj,matrix2);
	matmultiply(matrix1,matrix2,matrix1);

	retObj = JS_ConstructObject(cx,&VrmlMatrixClass,NULL, NULL);

	/*
       printf ("multRight calculated Matrix: \n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n\t%5.2f %5.2f %5.2f %5.2f\n",
                matrix1[0],  matrix1[4],  matrix1[ 8],  matrix1[12],
                matrix1[1],  matrix1[5],  matrix1[ 9],  matrix1[13],
                matrix1[2],  matrix1[6],  matrix1[10],  matrix1[14],
                matrix1[3],  matrix1[7],  matrix1[11],  matrix1[15]);
	*/
	_setmatrix(cx,retObj,matrix1);
	*rval = OBJECT_TO_JSVAL(retObj);

	return JS_TRUE;
}


JSBool
VrmlMatrixmultVecMatrix(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
        JSObject *transObj = NULL;
	JSObject *retObj = NULL;
	SFVec3fNative *Vptr;

        double matrix1[16];
        int error = FALSE;
	struct point_XYZ inp, outp;

	if (argc == 1) {
		error = !JS_ConvertArguments(cx, argc, argv, "o", &transObj);
	} else error = TRUE;

	if (!error) if (!JS_InstanceOf(cx, transObj, &SFVec3fClass, NULL)) { error = TRUE;}	

	if ((Vptr = (SFVec3fNative *)JS_GetPrivate(cx, transObj)) == NULL) {
		error = TRUE;
	}

	if (error) {
		ConsoleMessage ("VrmlMatrixMultVec, error in params");
		return JS_FALSE;
	}

	COPY_SFVEC3F_TO_POINT_XYZ(inp,Vptr->v.c);

	/* fill in the 2 matricies, multiply them, then return it */
	_getmatrix(cx,obj,matrix1);

	/* is this the one we have to transpose? */
	/* mattranspose (matrix1, matrix1); */
	
	matrotate2v(matrix1, inp, outp);

	retObj = JS_ConstructObject(cx,&SFVec3fClass,NULL, NULL);
	if ((Vptr = (SFVec3fNative *)JS_GetPrivate(cx, retObj)) == NULL) {
		printf ("error in new VrmlMatrix\n");
		return JS_FALSE;
	}

	COPY_POINT_XYZ_TO_SFVEC3F(Vptr->v.c,outp);
	*rval = OBJECT_TO_JSVAL(retObj);

	return JS_TRUE;
}


JSBool
VrmlMatrixmultMatrixVec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
        JSObject *transObj = NULL;
	JSObject *retObj = NULL;
	SFVec3fNative *Vptr;

        double matrix1[16];
        int error = FALSE;
	struct point_XYZ inp, outp;

	if (argc == 1) {
		error = !JS_ConvertArguments(cx, argc, argv, "o", &transObj);
	} else error = TRUE;

	if (!error) if (!JS_InstanceOf(cx, transObj, &SFVec3fClass, NULL)) { error = TRUE;}	

	if ((Vptr = (SFVec3fNative *)JS_GetPrivate(cx, transObj)) == NULL) {
		error = TRUE;
	}

	if (error) {
		ConsoleMessage ("VrmlMatrixMultVec, error in params");
		return JS_FALSE;
	}

	COPY_SFVEC3F_TO_POINT_XYZ(inp,Vptr->v.c);

	/* fill in the 2 matricies, multiply them, then return it */
	_getmatrix(cx,obj,matrix1);

	/* is this the one we have to transpose? */
	mattranspose (matrix1, matrix1);
	
	matrotate2v(matrix1, inp, outp);

	retObj = JS_ConstructObject(cx,&SFVec3fClass,NULL, NULL);
	if ((Vptr = (SFVec3fNative *)JS_GetPrivate(cx, retObj)) == NULL) {
		printf ("error in new VrmlMatrix\n");
		return JS_FALSE;
	}

	COPY_POINT_XYZ_TO_SFVEC3F(Vptr->v.c,outp);
	*rval = OBJECT_TO_JSVAL(retObj);

	return JS_TRUE;
}


JSBool
VrmlMatrixAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	return _standardMFAssign (cx, obj, argc, argv, rval, &VrmlMatrixClass,FIELDTYPE_FreeWRLPTR/*does not matter*/);
}

JSBool
VrmlMatrixConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	jsdouble _d;
	unsigned int i;

	ADD_ROOT(cx,obj)

	if ((argc != 16) && (argc != 0)) {
		printf ("VrmlMatrixConstr - require either 16 or no values\n");
		return JS_FALSE;
	}

	DEFINE_LENGTH(16,obj)

	if (argc == 16) {
		for (i = 0; i < 16; i++) {
			if (!JS_ValueToNumber(cx, argv[i], &_d)) {
				printf(
					"JS_ValueToNumber failed in VrmlMatrixConstr.\n");
				return JS_FALSE;
			}

			if (!JS_DefineElement(cx, obj, (jsint) i, argv[i], JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed for arg %u in VrmlMatrixConstr.\n", i);
				return JS_FALSE;
			}
		}
	} else {
		/* make the identity matrix */
		double matrix[16];
		loadIdentityMatrix(matrix);
		_setmatrix (cx, obj, matrix);
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
VrmlMatrixAddProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return doMFAddProperty(cx, obj, id, vp,"VrmlMatrixAddProperty");
}

JSBool
VrmlMatrixGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp)
{
	int32 _length, _index;
    jsval _length_val;

    if (!JS_GetProperty(cx, obj, "length", &_length_val)) {
		printf( "JS_GetProperty failed for \"length\" in VrmlMatrixGetProperty.\n");
        return JS_FALSE;
	}
	_length = JSVAL_TO_INT(_length_val);

/*
                if (JSVAL_IS_STRING(id)==TRUE) {
                printf("        is a common string :%s:\n",
                        JS_GetStringBytes(JS_ValueToString(cx, id)));
                }
                if (JSVAL_IS_OBJECT(id)==TRUE) {
                        printf ("       parameter is an object\n");
                }
                if (JSVAL_IS_PRIMITIVE(id)==TRUE) {
                        printf ("       parameter is a primitive\n");
                }
                if (JSVAL_IS_NULL(id)) { printf ("      - its a NULL\n");}
                if (JSVAL_IS_INT(id)) { printf ("       - its a INT %d\n",JSVAL_TO_INT(id));}
*/




	if (JSVAL_IS_INT(id)) {
		_index = JSVAL_TO_INT(id);

		if (_index >= _length) {
			JS_NewNumberValue(cx,0.0,vp);
			if (!JS_DefineElement(cx, obj, (jsint) _index, *vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed in VrmlMatrixGetProperty.\n");
				return JS_FALSE;
			}
		} else {
			if (!JS_LookupElement(cx, obj, _index, vp)) {
				printf(
						"JS_LookupElement failed in VrmlMatrixGetProperty.\n");
				return JS_FALSE;
			}
			if (*vp == JSVAL_VOID) {
				printf( "VrmlMatrixGetProperty: obj = %p, jsval = %d does not exist!\n",
					   obj, (int) _index);
				return JS_FALSE;
			}
		}
	} else if (JSVAL_IS_OBJECT(id)) {
	}

	return JS_TRUE;
}

JSBool
VrmlMatrixSetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return doMFSetProperty(cx, obj, id, vp,1000); /* do not have a FIELDTYPE for this */
}

/* MFRotation */
JSBool
MFRotationAddProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return doMFAddProperty(cx, obj, id, vp,"MFRotationAddProperty");
}

JSBool
MFRotationGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return _standardMFGetProperty(cx, obj, id, vp,
			 "_FreeWRL_Internal = new SFRotation()",FIELDTYPE_MFRotation);
}

JSBool
MFRotationSetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {
	return doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFRotation);
}

JSBool
MFRotationToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	UNUSED(argc);
	UNUSED(argv);
	return doMFToString(cx, obj, "MFRotation", rval);
}

JSBool
MFRotationConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	JSObject *_obj;
	unsigned int i;

	ADD_ROOT(cx,obj)
	DEFINE_LENGTH(argc,obj)

	if (!argv) {
		return JS_TRUE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
		printf("MFRotationConstr: obj = %p, %u args\n", obj, argc);
	#endif
	for (i = 0; i < argc; i++) {
		if (!JS_ValueToObject(cx, argv[i], &_obj)) {
			printf(
					"JS_ValueToObject failed in MFRotationConstr.\n");
			return JS_FALSE;
		}

		CHECK_CLASS(cx,_obj,NULL,__FUNCTION__,SFRotationClass)

		if (!JS_DefineElement(cx, obj, (jsint) i, argv[i], JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
			printf( "JS_DefineElement failed for arg %d in MFRotationConstr.\n", i);
			return JS_FALSE;
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

JSBool
MFRotationAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	return _standardMFAssign (cx, obj, argc, argv, rval, &MFRotationClass,FIELDTYPE_SFRotation);
}

/* MFStrings */
JSBool
MFStringAddProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp)
{
	#ifdef JSVRMLCLASSESVERBOSE
	printf("MFStringAddProperty: vp = %p\n", obj);
                if (JSVAL_IS_STRING(*vp)==TRUE) {
		printf("	is a common string :%s:\n",
                        JS_GetStringBytes(JS_ValueToString(cx, *vp)));
                }
                if (JSVAL_IS_OBJECT(*vp)==TRUE) {
                        printf ("       parameter is an object\n");
                }
                if (JSVAL_IS_PRIMITIVE(*vp)==TRUE) {
                        printf ("       parameter is a primitive\n");
                }
		if (JSVAL_IS_NULL(*vp)) { printf ("	- its a NULL\n");}
		if (JSVAL_IS_INT(*vp)) { printf ("	- its a INT %d\n",JSVAL_TO_INT(*vp));}

		printf("MFStringAddProperty: id = %p\n", obj);
                if (JSVAL_IS_STRING(id)==TRUE) {
		printf("	is a common string :%s:\n",
                        JS_GetStringBytes(JS_ValueToString(cx, id)));
                }
                if (JSVAL_IS_OBJECT(id)==TRUE) {
                        printf ("       parameter is an object\n");
                }
                if (JSVAL_IS_PRIMITIVE(id)==TRUE) {
                        printf ("       parameter is a primitive\n");
                }
		if (JSVAL_IS_NULL(id)) { printf ("	- its a NULL\n");}
		if (JSVAL_IS_INT(id)) { printf ("	- its a INT %d\n",JSVAL_TO_INT(id));}

	#endif


	/* unquote parts of vp string if necessary */
	if (JSVAL_IS_STRING(*vp)) {
		if (!doMFStringUnquote(cx, vp)) {
			printf(
				"doMFStringUnquote failed in MFStringAddProperty.\n");
			return JS_FALSE;
		}
	}
	return doMFAddProperty(cx, obj, id, vp,"MFStringAddProperty");
}


JSBool
MFStringGetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp)
{
	JSString *_str;
	int32 _length, _index;
    jsval _length_val;

	#ifdef JSVRMLCLASSESVERBOSE
	printf("MFStringGetProperty: obj = %p\n", obj);
	#endif

    if (!JS_GetProperty(cx, obj, "length", &_length_val)) {
		printf( "JS_GetProperty failed for \"length\" in MFStringGetProperty.\n");
        return JS_FALSE;
	}
	_length = JSVAL_TO_INT(_length_val);

	if (JSVAL_IS_INT(id)) {
		_index = JSVAL_TO_INT(id);

		if (_index >= _length) {
			_str = JS_NewStringCopyZ(cx, "");
			*vp = STRING_TO_JSVAL(_str);
			if (!JS_DefineElement(cx, obj, (jsint) _index, *vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
				printf( "JS_DefineElement failed in MFStringGetProperty.\n");
				return JS_FALSE;
			}
		} else {
			if (!JS_LookupElement(cx, obj, _index, vp)) {
				printf( "JS_LookupElement failed in MFStringGetProperty.\n");
				return JS_FALSE;
			}
			if (*vp == JSVAL_VOID) {
				/* jut make up new strings, as above */
				/* printf ("MFStringGetProperty, element %d is JSVAL_VOID, making up string for it\n",_index); */
				_str = JS_NewStringCopyZ(cx, "NULL");
				*vp = STRING_TO_JSVAL(_str);
				if (!JS_DefineElement(cx, obj, (jsint) _index, *vp, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
					printf( "JS_DefineElement failed in MFStringGetProperty.\n");
					return JS_FALSE;
				}
			}
		}
	}

	return JS_TRUE;
}

JSBool
MFStringSetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp)
{
	JSBool rv;

	#ifdef JSVRMLCLASSESVERBOSE
	printf("MFStringSetProperty: obj = %p id %d jsval %u\n", obj, id, *vp);

printf ("MFStringSetProperty, setting vp of type...\n");
		if (JSVAL_IS_OBJECT(*vp)) { printf ("	- MFStringSetProperty, vp is a OBJECT\n");}
		if (JSVAL_IS_PRIMITIVE(*vp)) { printf ("	- MFStringSetProperty, vp is a PRIMITIVE\n");}
		if (JSVAL_IS_NULL(*vp)) { printf ("	- MFStringSetProperty, vp is a NULL\n");}
		if (JSVAL_IS_STRING(*vp)) { printf ("	- MFStringSetProperty, vp is a STRING\n");}
		if (JSVAL_IS_INT(*vp)) { printf ("	- MFStringSetProperty, vp is a INT %d\n",JSVAL_TO_INT(*vp));}

	#endif


	/* unquote parts of vp string if necessary */
	if (JSVAL_IS_STRING(*vp)) {
		if (!doMFStringUnquote(cx, vp)) {
			printf(
				"doMFStringUnquote failed in MFStringSetProperty.\n");
			return JS_FALSE;
		}
	}
	rv = doMFSetProperty(cx, obj, id, vp,FIELDTYPE_MFString);
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("returning from MFStringSetProperty\n");
	#endif

	return rv;

}

JSBool
MFStringToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	UNUSED(argc);
	UNUSED(argv);
	#ifdef JSVRMLCLASSESVERBOSE
	printf("MFStringToString: obj = %p, %u args\n", obj, argc);
	#endif


	return doMFToString(cx, obj, "MFString", rval);
}


JSBool
MFStringConstr(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	unsigned int i;


	#ifdef JSVRMLCLASSESVERBOSE
	JSString *_str;
	printf("MFStringConstr: cx %u, obj %u args %d rval %d parent %d... ", cx, obj, argc, rval, JS_GetParent(cx, obj));
	#endif

	ADD_ROOT(cx,obj)
	DEFINE_LENGTH(argc,obj)
	DEFINE_MF_ECMA_HAS_CHANGED

	if (!argv) {
		return JS_TRUE;
	}

	for (i = 0; i < argc; i++) {
		#ifdef JSVRMLCLASSESVERBOSE
	  	printf ("argv %d is a ...",i);

		if (JSVAL_IS_STRING(argv[i])==TRUE) {
        	        printf (" Common String, is");
			_str = JS_ValueToString(cx, argv[i]);
			printf (JS_GetStringBytes(_str));
			printf ("..");
		
	        }                                          
		if (JSVAL_IS_OBJECT(argv[i])==TRUE) {   
	                printf (" is an object");
	        }                       
		if (JSVAL_IS_PRIMITIVE(argv[i])==TRUE) {
        	        printf (" is a primitive");
        	}

		if ((_str = JS_ValueToString(cx, argv[i])) == NULL) {
			printf( "JS_ValueToString failed in MFStringConstr.");
			return JS_FALSE;
		}
		printf ("\n");
		#endif

	
		if (!JS_DefineElement(cx, obj, (jsint) i, argv[i], JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_CHECK, JSPROP_ENUMERATE)) {
			printf( "JS_DefineElement failed for arg %d in MFStringConstr.\n", i);
			return JS_FALSE;
		}
	}
	*rval = OBJECT_TO_JSVAL(obj);

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("finished MFStringConstr\n");
	#endif

	return JS_TRUE;
}

JSBool
MFStringAssign(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	#ifdef JSVRMLCLASSESVERBOSE
	printf("MFStringAssign: obj = %p args %d... ", obj, argc);
	#endif
	SET_MF_ECMA_HAS_CHANGED

	return _standardMFAssign (cx, obj, argc, argv, rval, &MFStringClass,FIELDTYPE_SFString);
}

/* testing.. */
JSBool MFStringDeleteProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp) { 
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("MFStringDeleteProperty\n"); 
	#endif
	return JS_TRUE;
}
JSBool MFStringEnumerateProperty(JSContext *cx, JSObject *obj) { 
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("MFStringEnumerateProperty\n"); 
	#endif
	return JS_TRUE;
}

JSBool MFStringResolveProperty(JSContext *cx, JSObject *obj, jsid id) { 
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("MFStringResolveProperty\n"); 
	#endif
	return JS_TRUE;
}
JSBool MFStringConvertProperty(JSContext *cx, JSObject *obj, JSType type, jsval *vp) { 
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("MFStringConvertProperty\n"); 
	#endif
	return JS_TRUE;
}

#endif /* HAVE_JAVASCRIPT */
