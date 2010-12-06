/*
=INSERT_TEMPLATE_HERE=

$Id: jsUtils.c,v 1.27 2010/12/06 18:39:10 davejoubert Exp $

A substantial amount of code has been adapted from js/src/js.c,
which is the sample application included with the javascript engine.

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
#include "fieldSet.h"

extern void dump_scene (FILE *fp, int level, struct X3D_Node* node); // in GeneratedCode.c
extern char *parser_getNameFromNode(struct X3D_Node *ptr) ; /* vi +/dump_scene src/lib/scenegraph/GeneratedCode.c */

/********************** Javascript to X3D Scenegraph ****************************/


/* a SF value has changed in an MF; eg, xx[10] = new SFVec3f(...); */
/* These values were created below; new SF values to this MF are not accessed here, but in 
   the MFVec3fSetProperty (or equivalent). Note that we do NOT use something like JS_SetElement
   on these because it would be a recursive call; thus we set the private data */


static int insetSFStr = FALSE;
static JSBool reportWarnings = JS_TRUE;

static JSBool setSF_in_MF (JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
	int num;
	jsval pf;
	jsval nf;
	JSObject *me;
	JSObject *par;
	jsval ele; 

	/* when we save the value, we will be called again, so we make sure that we
	   know if we are being called from within, or from without */
	if (insetSFStr) { 
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("setSF_in_MF: already caught this value; this is our JS_SetElement call\n"); 
		#endif
		return JS_TRUE;
	}

	/* ok, we are really called to replace an existing SFNode MF value assignment */
	insetSFStr = TRUE; 

	if (JSVAL_IS_INT(id)) {
		if (!JS_ValueToInt32(cx,id,&num)) {
			printf ("setSF_in_MF: error converting to number...\n");
			return JS_FALSE;
		}
		/* get a pointer to the object at the index in the parent */ 
		if (!JS_GetElement(cx, obj, num, &ele)) { 
			printf ("error getting child %d in setSF_in_MF\n",num); 
			return JS_FALSE; 
		} 
		/* THIS is the touching that will cause us to be called recursively,
		   which is why insetSFStr is TRUE right here */
		if (!JS_SetElement(cx,obj,num,vp)) { 
			printf ("can not set element %d in MFString\n",num); 
			return JS_FALSE; 
		} 
	} else {
		printf ("expect an integer id in setSF_in_MF\n");
		return JS_FALSE;
	}


	/* copy this value out to the X3D scene graph */
	me = obj;
	par = JS_GetParent(cx, me);
	while (par != NULL) {
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("for obj %u: ",me);
			printJSNodeType(cx,me);
		printf ("... parent %u\n",par);
			printJSNodeType(cx,par);
		#endif

		if (JS_InstanceOf (cx, par, &SFNodeClass, NULL)) {
			#ifdef JSVRMLCLASSESVERBOSE
			printf (" the parent IS AN SFNODE - it is %u\n",par);
			#endif


			if (!JS_GetProperty(cx, obj, "_parentField", &pf)) {
				printf ("doMFSetProperty, can not get parent field from this object\n");
				return JS_FALSE;
			}

			nf = OBJECT_TO_JSVAL(me);

			#ifdef JSVRMLCLASSESVERBOSE
			printf ("parentField is %u \"%s\"\n", pf, JS_GetStringBytes(JSVAL_TO_STRING(pf)));
			#endif

			if (!setSFNodeField (cx, par, pf, &nf)) {
				printf ("could not set field of SFNode\n");
			}

		}
		me = par;
		par = JS_GetParent(cx, me);
	}
	insetSFStr = FALSE;
	return JS_TRUE;
}

/* take an ECMA value in the X3D Scenegraph, and return a jsval with it in */
/* This is FAST as w deal just with pointers */
static void JS_ECMA_TO_X3D(JSContext *cx, void *Data, unsigned datalen, int dataType, jsval *newval) {
	float fl;
	double dl;
	int il;

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("calling JS_ECMA_TO_X3D on type %s\n",FIELDTYPES[dataType]);
	#endif

	switch (dataType) {

		case FIELDTYPE_SFFloat:	{
			if (!JS_ValueToNumber(cx,*newval,&dl)) {
				printf ("problems converting Javascript val to number\n");
				return;
			}
			fl = (float) dl;
			memcpy (Data, (void *) &fl, datalen);
			break;
		}
		case FIELDTYPE_SFTime:	{
			if (!JS_ValueToNumber(cx,*newval,&dl)) {
				printf ("problems converting Javascript val to number\n");
				return;
			}
			memcpy (Data, (void *) &dl, datalen);
			break;
		}
		case FIELDTYPE_SFBool: {
			il = JSVAL_TO_BOOLEAN (*newval);
			memcpy (Data, (void *) &il, datalen);
			break;
		}

		case FIELDTYPE_SFInt32: 	{ 
			il = JSVAL_TO_INT (*newval);
			memcpy (Data, (void *) &il, datalen);
			break;
		}

		case FIELDTYPE_SFString: {
			struct Uni_String *oldS;
        		JSString *_idStr;
        		char *_id_c;

        		_idStr = JS_ValueToString(cx, *newval);
        		_id_c = JS_GetStringBytes(_idStr);

			oldS = (struct Uni_String *) *((uintptr_t *)Data);

			#ifdef JSVRMLCLASSESVERBOSE
			printf ("JS_ECMA_TO_X3D, replacing \"%s\" with \"%s\" \n", oldS->strptr, _id_c);
			#endif

			/* replace the C string if it needs to be replaced. */
			verify_Uni_String (oldS,_id_c);
			break;
		}
		default: {	printf("WARNING: SHOULD NOT BE HERE in JS_ECMA_TO_X3D! %d\n",dataType); }
	}
}


/* take a Javascript  ECMA value and put it in the X3D Scenegraph. */
static void JS_SF_TO_X3D(JSContext *cx, void *Data, unsigned datalen, int dataType, jsval *newval) {
        SFColorNative *Cptr;
	SFVec3fNative *V3ptr;
	SFVec3dNative *V3dptr;
	SFVec2fNative *V2ptr;
	SFRotationNative *VRptr;
	SFNodeNative *VNptr;

	void *VPtr;

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("calling JS_SF_TO_X3D on type %s\n",FIELDTYPES[dataType]);
	#endif

	/* get a pointer to the internal private data */
	if ((VPtr = JS_GetPrivate(cx, JSVAL_TO_OBJECT(*newval))) == NULL) {
		printf( "JS_GetPrivate failed in JS_SF_TO_X3D.\n");
		return;
	}

	/* copy over the data from the X3D node to this new Javascript object */
	switch (dataType) {
                case FIELDTYPE_SFColor:
			Cptr = (SFColorNative *)VPtr;
			memcpy (Data, (void *)((Cptr->v).c), datalen);
			break;
                case FIELDTYPE_SFVec3d:
			V3dptr = (SFVec3dNative *)VPtr;
			memcpy (Data, (void *)((V3dptr->v).c), datalen);
			break;
                case FIELDTYPE_SFVec3f:
			V3ptr = (SFVec3fNative *)VPtr;
			memcpy (Data, (void *)((V3ptr->v).c), datalen);
			break;
                case FIELDTYPE_SFVec2f:
			V2ptr = (SFVec2fNative *)VPtr;
			memcpy (Data, (void *)((V2ptr->v).c), datalen);
			break;
                case FIELDTYPE_SFRotation:
			VRptr = (SFRotationNative *)VPtr;
			memcpy (Data,(void *)((VRptr->v).c), datalen);
			break;
                case FIELDTYPE_SFNode:
			VNptr = (SFNodeNative *)VPtr;
			memcpy (Data, (void *)(VNptr->handle), datalen);
			break;

		default: {	printf("WARNING: SHOULD NOT BE HERE! %d\n",dataType); }
	}
}


/* make an MF type from the X3D node. This can be fairly slow... */
static void JS_MF_TO_X3D(JSContext *cx, JSObject * obj, void *Data, int dataType, jsval *newval) {

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("calling JS_MF_TO_X3D on type %s\n",FIELDTYPES[dataType]);
	printf ("JS_MF_TO_X3D, we have object %u, newval %u\n",obj,*newval);
	printf ("JS_MF_TO_X3D, obj is an:\n");
	if (JSVAL_IS_OBJECT(OBJECT_TO_JSVAL(obj))) { printf ("JS_MF_TO_X3D - obj is an OBJECT\n"); }
	if (JSVAL_IS_PRIMITIVE(OBJECT_TO_JSVAL(obj))) { printf ("JS_MF_TO_X3D - obj is an PRIMITIVE\n"); }
	printf ("JS_MF_TO_X3D, obj is a "); printJSNodeType(cx,obj);
	printf ("JS_MF_TO_X3D, vp is an:\n");
	if (JSVAL_IS_OBJECT(*newval)) { printf ("JS_MF_TO_X3D - vp is an OBJECT\n"); }
	if (JSVAL_IS_PRIMITIVE(*newval)) { printf ("JS_MF_TO_X3D - vp is an PRIMITIVE\n"); }
	printf ("JS_MF_TO_X3D, vp is a "); printJSNodeType(cx,JSVAL_TO_OBJECT(*newval));
	#endif

	JSglobal_return_val = *newval;
	getJSMultiNumType (cx, (struct Multi_Vec3f*) Data, convertToSFType(dataType));

}

/********************** X3D Scenegraph to Javascript ****************************/

/* take an ECMA value in the X3D Scenegraph, and return a jsval with it in */
/* This is FAST as w deal just with pointers */
void X3D_ECMA_TO_JS(JSContext *cx, void *Data, int datalen, int dataType, jsval *newval) {
	float fl;
	double dl;
	int il;

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("calling X3D_ECMA_TO_JS on type %s\n",FIELDTYPES[dataType]);
	#endif

	switch (dataType) {

		case FIELDTYPE_SFFloat:	{
			memcpy ((void *) &fl, Data, datalen);
			*newval = DOUBLE_TO_JSVAL(JS_NewDouble(cx,(double)fl));
			break;
		}
		case FIELDTYPE_SFTime:	{
			memcpy ((void *) &dl, Data, datalen);
			*newval = DOUBLE_TO_JSVAL(JS_NewDouble(cx,dl));
			break;
		}
		case FIELDTYPE_SFBool:
		case FIELDTYPE_SFInt32: 	{ 
			memcpy ((void *) &il,Data, datalen);
			*newval = INT_TO_JSVAL(il);
			break;
		}

		case FIELDTYPE_SFString: {
			struct Uni_String *ms;

			/* datalen will be ROUTING_SFSTRING here; or at least should be! We
			   copy over the data, which is a UniString pointer, and use the pointer
			   value here */
			memcpy((void *) &ms,Data, sizeof(void *));
			*newval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,ms->strptr));
			break;
		}
		default: {	printf("WARNING: SHOULD NOT BE HERE in X3D_ECMA_TO_JS! %d\n",dataType); }
	}
}

/* take an ECMA value in the X3D Scenegraph, and return a jsval with it in */
/* this is not so fast; we call a script to make a default type, then we fill it in */
static void X3D_SF_TO_JS(JSContext *cx, JSObject *obj, void *Data, unsigned datalen, int dataType, jsval *newval) {
        SFColorNative *Cptr;
	SFVec3fNative *V3ptr;
	SFVec3dNative *V3dptr;
	SFVec2fNative *V2ptr;
	SFRotationNative *VRptr;
	SFNodeNative *VNptr;

	void *VPtr;
	jsval rval;
	char *script = NULL;

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("calling X3D_SF_TO_JS on type %s, newval %u\n",FIELDTYPES[dataType],*newval);
	#endif

	if (!JSVAL_IS_OBJECT(*newval)) {
		/* find a script to create the correct object */
		switch (dataType) {
        	        case FIELDTYPE_SFVec3f: script = "new SFVec3f()"; break;
        	        case FIELDTYPE_SFVec3d: script = "new SFVec3d()"; break;
        	        case FIELDTYPE_SFColor: script = "new SFColor()"; break;
        	        case FIELDTYPE_SFNode: script = "new SFNode()"; break;
        	        case FIELDTYPE_SFVec2f: script = "new SFVec2f()"; break;
        	        case FIELDTYPE_SFRotation: script = "new SFRotation()"; break;
			default: printf ("invalid type in X3D_SF_TO_JS\n"); return;
		}

		/* create the object */


		#ifdef JSVRMLCLASSESVERBOSE
		printf ("X3D_SF_TO_JS, have to run script to make new object: \"%s\"\n",script);
		#endif

		if (!JS_EvaluateScript(cx, obj, script, (int) strlen(script), FNAME_STUB, LINENO_STUB, &rval)) {
			printf ("error creating the new object in X3D_SF_TO_JS, script :%s:\n",script);
			return;
		}

		/* this is the return pointer, lets save it right now */
		*newval = rval;

		#ifdef JSVRMLCLASSESVERBOSE
		printf ("X3D_SF_TO_JS, so, newval now is %u\n",*newval);
		#endif

	}
	/* get a pointer to the internal private data */
	if ((VPtr = JS_GetPrivate(cx, JSVAL_TO_OBJECT(*newval))) == NULL) {
		printf( "JS_GetPrivate failed in X3D_SF_TO_JS.\n");
		return;
	}


	/* copy over the data from the X3D node to this new Javascript object */
	switch (dataType) {
                case FIELDTYPE_SFColor:
			Cptr = (SFColorNative *)VPtr;
			memcpy ((void *)((Cptr->v).c), Data, datalen);
        		Cptr->valueChanged = 1;
			break;
                case FIELDTYPE_SFVec3f:
			V3ptr = (SFVec3fNative *)VPtr;
			memcpy ((void *)((V3ptr->v).c), Data, datalen);
        		V3ptr->valueChanged = 1;
			break;
                case FIELDTYPE_SFVec3d:
			V3dptr = (SFVec3dNative *)VPtr;
			memcpy ((void *)((V3dptr->v).c), Data, datalen);
        		V3dptr->valueChanged = 1;
			break;
                case FIELDTYPE_SFVec2f:
			V2ptr = (SFVec2fNative *)VPtr;
			memcpy ((void *)((V2ptr->v).c), Data, datalen);
        		V2ptr->valueChanged = 1;
			break;
                case FIELDTYPE_SFRotation:
			VRptr = (SFRotationNative *)VPtr;
			memcpy ((void *)((VRptr->v).c), Data, datalen);
        		VRptr->valueChanged = 1;
			break;
                case FIELDTYPE_SFNode:
			VNptr = (SFNodeNative *)VPtr;
			memcpy ((void *)(&(VNptr->handle)), Data, datalen);
        		VNptr->valueChanged = 1;
			break;

		default: {	printf("WARNING: SHOULD NOT BE HERE! %d\n",dataType); }
	}
}

/* make an MF type from the X3D node. This can be fairly slow... */
static void X3D_MF_TO_JS(JSContext *cx, JSObject *obj, void *Data, int dataType, jsval *newval, char *fieldName) {
	int i;
	jsval rval;
	char *script = NULL;
	struct Multi_Int32 *MIptr;
	struct Multi_Float *MFptr;
	struct Multi_Time *MTptr;
	jsval fieldNameAsJSVAL;


	/* so, obj should be an __SFNode_proto, and newval should be a __MFString_proto (or whatever) */

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("calling X3D_MF_TO_JS on type %s\n",FIELDTYPES[dataType]);
	printf ("X3D_MF_TO_JS, we have object %u, newval %u\n",obj,*newval);
	printf ("X3D_MF_TO_JS, obj is an:\n");
	if (JSVAL_IS_OBJECT(OBJECT_TO_JSVAL(obj))) { printf ("X3D_MF_TO_JS - obj is an OBJECT\n"); 
		printf ("X3D_MF_TO_JS, obj is a "); printJSNodeType(cx,obj);
	}
	if (JSVAL_IS_PRIMITIVE(OBJECT_TO_JSVAL(obj))) { printf ("X3D_MF_TO_JS - obj is an PRIMITIVE\n"); }
	printf ("X3D_MF_TO_JS, vp is an:\n");
	if (JSVAL_IS_OBJECT(*newval)) { printf ("X3D_MF_TO_JS - newval is an OBJECT\n"); 
		printf ("X3D_MF_TO_JS, newval is a "); printJSNodeType(cx,JSVAL_TO_OBJECT(*newval));
	}
	if (JSVAL_IS_PRIMITIVE(*newval)) { printf ("X3D_MF_TO_JS - newval is an PRIMITIVE\n"); }
	#endif


#ifdef JSVRMLCLASSESVERBOSE
	printf ("X3D_MF_TO_JS - is this already expanded? \n");
	{
        SFNodeNative *VNptr;

        /* get a pointer to the internal private data */
        if ((VNptr = JS_GetPrivate(cx, obj)) == NULL) {
                printf( "JS_GetPrivate failed in X3D_MF_TO_JS.\n");
                return;
        }
	if (VNptr->fieldsExpanded) printf ("FIELDS EXPANDED\n");
	else printf ("FIELDS NOT EXPANDED\n");
	}
#endif


	if (!JSVAL_IS_OBJECT(*newval)) {
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("X3D_MF_TO_JS - have to create empty MF type \n");
		#endif

		/* find a script to create the correct object */
		switch (dataType) {
			case FIELDTYPE_MFString: script = "new MFString()"; break;
			case FIELDTYPE_MFFloat: script = "new MFFloat()"; break;
			case FIELDTYPE_MFTime: script = "new MFTime()"; break;
			case FIELDTYPE_MFInt32: script = "new MFInt32()"; break;
			case FIELDTYPE_SFImage: script = "new SFImage()"; break;
        	        case FIELDTYPE_MFVec3f: script = "new MFVec3f()"; break;
        	        case FIELDTYPE_MFColor: script = "new MFColor()"; break;
        	        case FIELDTYPE_MFNode: script = "new MFNode()"; break;
        	        case FIELDTYPE_MFVec2f: script = "new MFVec2f()"; break;
        	        case FIELDTYPE_MFRotation: script = "new MFRotation()"; break;
			default: printf ("invalid type in X3D_MF_TO_JS\n"); return;
		}

		if (!JS_EvaluateScript(cx, obj, script, (int) strlen(script), FNAME_STUB, LINENO_STUB, &rval)) {
			printf ("error creating the new object in X3D_MF_TO_JS\n");
			return;
		}

		/* this is the return pointer, lets save it right now */
		*newval = rval;
	}

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("setting parent for %u to %u\n", *newval, obj);
	#endif

	/* ok - if we are setting an MF* field by a thing like myField[10] = new String(); the
	   set method does not really get called. So, we go up the parental chain until we get
	   either no parent, or a SFNode. If we get a SFNode, we call the "save this" function
	   so that the X3D scene graph gets the updated array value. To make a long story short,
	   here's the call to set the parent for the above. */
	if (!JS_SetParent (cx, (JSObject *)*newval, obj)) {
		printf ("X3D_MF_TO_JS - can not set parent!\n");
	} 

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("telling %u that it is a child \"%s\" of parent %u\n",*newval, fieldName, obj);
	#endif

	fieldNameAsJSVAL = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,fieldName));

        if (!JS_DefineProperty(cx, JSVAL_TO_OBJECT(*newval), "_parentField", fieldNameAsJSVAL, 
			JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB5, JSPROP_READONLY)) {
                printf("JS_DefineProperty failed for \"%s\" in X3D_MF_TO_JS.\n", fieldName);
                return;
       	}


	#ifdef JSVRMLCLASSESVERBOSE
	printf ("X3D_MF_TO_JS - object is %u, copying over data\n",*newval);
	#endif


	/* copy over the data from the X3D node to this new Javascript object */
	switch (dataType) {
		case FIELDTYPE_MFInt32:
			MIptr = (struct Multi_Int32*) Data;
			for (i=0; i<MIptr->n; i++) {
                		if (!JS_DefineElement(cx, (JSObject *)*newval, (jsint) i, INT_TO_JSVAL(MIptr->p[i]),
                        		  JS_GET_PROPERTY_STUB, setSF_in_MF, JSPROP_ENUMERATE)) {
                        		printf( "JS_DefineElement failed for arg %u in MFInt32Constr.\n", i);
                        		return;
                		}
			}
			break;
		case FIELDTYPE_MFFloat:
			MFptr = (struct Multi_Float*) Data;
			for (i=0; i<MFptr->n; i++) {
                		if (!JS_DefineElement(cx, (JSObject *)*newval, (jsint) i, INT_TO_JSVAL(MFptr->p[i]),
                        		  JS_GET_PROPERTY_STUB, setSF_in_MF, JSPROP_ENUMERATE)) {
                        		printf( "JS_DefineElement failed for arg %u in MFFloatConstr.\n", i);
                        		return;
                		}
			}
			break;
		case FIELDTYPE_MFTime:
			MTptr = (struct Multi_Time*) Data;
			for (i=0; i<MTptr->n; i++) {
                		if (!JS_DefineElement(cx, (JSObject *)*newval, (jsint) i, INT_TO_JSVAL(MTptr->p[i]),
                        		  JS_GET_PROPERTY_STUB, setSF_in_MF, JSPROP_ENUMERATE)) {
                        		printf( "JS_DefineElement failed for arg %u in MFTimeConstr.\n", i);
                        		return;
                		}
			}
			break;
                case FIELDTYPE_MFColor:
		case FIELDTYPE_MFVec3f: {
			struct Multi_Vec3f* MCptr;
			char newline[100];
			jsval xf;

			MCptr = (struct Multi_Vec3f *) Data;
			for (i=0; i<MCptr->n; i++) {
				if (dataType == FIELDTYPE_MFColor) 
					sprintf (newline,"new SFColor(%f, %f, %f)", MCptr->p[i].c[0], MCptr->p[i].c[1], MCptr->p[i].c[2]);	
				else
					sprintf (newline,"new SFColor(%f, %f, %f)", MCptr->p[i].c[0], MCptr->p[i].c[1], MCptr->p[i].c[2]);	
				if (!JS_EvaluateScript(cx, (JSObject *)*newval, newline, (int) strlen(newline), FNAME_STUB, LINENO_STUB, &xf)) {
					printf ("error creating the new object in X3D_MF_TO_JS string :%s:\n",newline);
					return;
				}
                		if (!JS_DefineElement(cx, (JSObject *)*newval, (jsint) i, xf,
                        		  JS_GET_PROPERTY_STUB, setSF_in_MF, JSPROP_ENUMERATE)) {
                        		printf( "JS_DefineElement failed for arg %u .\n", i);
                        		return;
                		}
			}
		} break;

		case FIELDTYPE_MFVec2f: {
			struct Multi_Vec2f* MCptr;
			char newline[100];
			jsval xf;

			MCptr = (struct Multi_Vec2f *) Data;
			for (i=0; i<MCptr->n; i++) {
				sprintf (newline,"new SFVec2f(%f, %f)", MCptr->p[i].c[0], MCptr->p[i].c[1]);	
				if (!JS_EvaluateScript(cx, (JSObject *)*newval, newline, (int) strlen(newline), FNAME_STUB, LINENO_STUB, &xf)) {
					printf ("error creating the new object in X3D_MF_TO_JS string :%s:\n",newline);
					return;
				}
                		if (!JS_DefineElement(cx, (JSObject *)*newval, (jsint) i, xf,
                        		  JS_GET_PROPERTY_STUB, setSF_in_MF, JSPROP_ENUMERATE)) {
                        		printf( "JS_DefineElement failed for arg %u .\n", i);
                        		return;
                		}
			}
		} break;
		case FIELDTYPE_MFRotation: {
			struct Multi_Rotation* MCptr;
			char newline[100];
			jsval xf;

			MCptr = (struct Multi_Rotation*) Data;
			for (i=0; i<MCptr->n; i++) {
				sprintf (newline,"new SFRotation(%f, %f, %f, %f)", MCptr->p[i].c[0], MCptr->p[i].c[1], MCptr->p[i].c[2], MCptr->p[i].c[3]);	
				if (!JS_EvaluateScript(cx, (JSObject *)*newval, newline, (int) strlen(newline), FNAME_STUB, LINENO_STUB, &xf)) {
					printf ("error creating the new object in X3D_MF_TO_JS string :%s:\n",newline);
					return;
				}
                		if (!JS_DefineElement(cx, (JSObject *)*newval, (jsint) i, xf,
                        		  JS_GET_PROPERTY_STUB, setSF_in_MF, JSPROP_ENUMERATE)) {
                        		printf( "JS_DefineElement failed for arg %u .\n", i);
                        		return;
                		}
			}
		} break;

		case FIELDTYPE_MFNode: {
			struct Multi_Node* MCptr;
			char newline[100];
			jsval xf;

			MCptr = (struct Multi_Node *) Data;

			for (i=0; i<MCptr->n; i++) {
				/* purge out null nodes */
				if (MCptr->p[i] != NULL) {
				sprintf (newline,"new SFNode(\"%p\")", MCptr->p[i]);	

				if (!JS_EvaluateScript(cx, (JSObject *)*newval, newline, (int) strlen(newline), FNAME_STUB, LINENO_STUB, &xf)) {
					printf ("error creating the new object in X3D_MF_TO_JS string :%s:\n",newline);
					return;
				}
                		if (!JS_DefineElement(cx, (JSObject *)*newval, (jsint) i, xf,
                        		  JS_GET_PROPERTY_STUB, setSF_in_MF, JSPROP_ENUMERATE)) {
                        		printf( "JS_DefineElement failed for arg %u .\n", i);
                        		return;
                		}
				} else {
					/* printf ("X3DMF, ignoring NULL node here \n"); */
				}
			}
		} break;


		case FIELDTYPE_MFString: {
			struct Multi_String* MCptr;
			char newline[100];
			jsval xf;

			MCptr = (struct Multi_String *) Data;
			for (i=0; i<MCptr->n; i++) {
				#ifdef JSVRMLCLASSESVERBOSE
				printf ("X3D_MF_TO_JS, working on %d of %d, p %u\n",i, MCptr->n, MCptr->p[i]);
				#endif

				if (((struct Uni_String *)MCptr->p[i])->strptr != NULL)
					sprintf (newline,"new String('%s')", ((struct Uni_String *)MCptr->p[i])->strptr);	
				else sprintf (newline,"new String('(NULL)')");	

				#ifdef JSVRMLCLASSESVERBOSE
				printf ("X3D_MF_TO_JS, we have a new script to evaluate: \"%s\"\n",newline);
				#endif

				if (!JS_EvaluateScript(cx, (JSObject *)*newval, newline, (int) strlen(newline), FNAME_STUB, LINENO_STUB, &xf)) {
					printf ("error creating the new object in X3D_MF_TO_JS string :%s:\n",newline);
					return;
				}
                		if (!JS_DefineElement(cx, (JSObject *)*newval, (jsint) i, xf,
                        		  JS_GET_PROPERTY_STUB, setSF_in_MF, JSPROP_ENUMERATE)) {
                        		printf( "JS_DefineElement failed for arg %u .\n", i);
                        		return;
                		}
			}
		} break;

		case FIELDTYPE_SFImage: {
			struct Multi_Int32* MCptr;
			char newline[10000];
			jsval xf;

			/* look at the PixelTexture internals, an image is just a bunch of Int32s */
			MCptr = (struct Multi_Int32 *) Data;
			sprintf (newline, "new SFImage(");

			for (i=0; i<MCptr->n; i++) {
				char sl[20];
				sprintf (sl,"0x%x ", MCptr->p[i]);	
				strcat (newline,sl); 

				if (i != ((MCptr->n)-1)) strcat (newline,",");
				if (i == 2) strcat (newline, " new MFInt32(");

			}
			strcat (newline, "))");

			if (!JS_EvaluateScript(cx, (JSObject *)*newval, newline, (int) strlen(newline), FNAME_STUB, LINENO_STUB, &xf)) {
				printf ("error creating the new object in X3D_MF_TO_JS string :%s:\n",newline);
				return;
			}
			*newval = xf; /* save this version */
		} break;
		default: {	printf("WARNING: SHOULD NOT BE HERE! %d\n",dataType); }
	}

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("returning from X3D_MF_TO_JS\n");
	#endif
}

void
reportWarningsOn() { reportWarnings = JS_TRUE; }


void
reportWarningsOff() { reportWarnings = JS_FALSE; }


void
errorReporter(JSContext *context, const char *message, JSErrorReport *report)
{
	char *errorReport = 0;
	int len = 0, charPtrSize = (int) sizeof(char *);

    if (!report) {
        fprintf(stderr, "%s\n", message);
        return;
    }

    /* Conditionally ignore reported warnings. */
    if (JSREPORT_IS_WARNING(report->flags) && !reportWarnings) {
		return;
	}

	len = (int) ((strlen(report->filename) + 1) + (strlen(message) + 1));

	errorReport = (char *) JS_malloc(context, (len + STRING) * charPtrSize);
	if (!errorReport) {
		return;
	}


    if (JSREPORT_IS_WARNING(report->flags)) {
		sprintf(errorReport,
				"%swarning in %s at line %u:\n\t%s\n",
				JSREPORT_IS_STRICT(report->flags) ? "strict " : "",
				report->filename ? report->filename : "",
				report->lineno ? report->lineno : 0,
				message ? message : "No message.");
	} else {
		sprintf(errorReport,
				"error in %s at line %u:\n\t%s\n",
				report->filename ? report->filename : "",
				report->lineno ? report->lineno : 0,
				message ? message : "No message.");
	}

	fprintf(stderr, "Javascript -- %s", errorReport);

	JS_free(context, errorReport);
}


/* SFNode - find the fieldOffset pointer for this field within this node */
static int *getFOP (struct X3D_Node *node, const char *str) {
	int *fieldOffsetsPtr;


	if (node != NULL) {
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("...getFOP... it is a %s\n",stringNodeType(node->_nodeType));
		#endif

                fieldOffsetsPtr = (int *) NODE_OFFSETS[node->_nodeType];
                /*go thru all field*/
		/* what we have is a list of 4 numbers, representing:
        		FIELDNAMES__parentResource, offsetof (struct X3D_Anchor, __parenturl),  FIELDTYPE_SFString, KW_initializeOnly,
		*/

                while (*fieldOffsetsPtr != -1) {
			#ifdef JSVRMLCLASSESVERBOSE
                        printf ("getFOP, looking at field %s type %s to match %s\n",FIELDNAMES[*fieldOffsetsPtr],FIELDTYPES[*(fieldOffsetsPtr+2)],str); 
			#endif

			/* skip any fieldNames starting with an underscore, as these are "internal" ones */
			/* There is in fact nothing in this function that actually enforces this, which is good!! */
			if (strcmp(str,FIELDNAMES[*fieldOffsetsPtr]) == 0) {
				#ifdef JSVRMLCLASSESVERBOSE
				printf ("getFOP, found entry for %s, it is %u (%p)\n",str,fieldOffsetsPtr,fieldOffsetsPtr);
				#endif
				return fieldOffsetsPtr;
			}

			fieldOffsetsPtr += 5;
		}

		/* failed to find field?? */
	} else {
		printf ("getFOP, passed in X3D node was NULL!\n");
	}
	return NULL;
}


/* getter for SFNode accesses */
static JSBool getSFNodeField (JSContext *context, JSObject *obj, jsval id, jsval *vp) {
	JSString *_idStr;
	char *_id_c;
        SFNodeNative *ptr;
	int *fieldOffsetsPtr;
	struct X3D_Node *node;

	_idStr = JS_ValueToString(context, id);
	_id_c = JS_GetStringBytes(_idStr);
	
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("\ngetSFNodeField called on name %s object %u\n",_id_c, obj);
	#endif

        if ((ptr = (SFNodeNative *)JS_GetPrivate(context, obj)) == NULL) {
                printf( "JS_GetPrivate failed in getSFNodeField.\n");
                return JS_FALSE;
        }
	node = X3D_NODE(ptr->handle);

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("getSFNodeField, got node %u for field %s object %u\n",node,_id_c, obj);
	printf ("getSFNodeField, got node %p for field %s object %p\n",node,_id_c, obj);
	#endif

	if (node == NULL) {
		printf ("getSFNodeField, can not set field \"%s\", NODE is NULL!\n",_id_c);
		return JS_FALSE;
	}

	/* get the table entry giving the type, offset, etc. of this field in this node */
	fieldOffsetsPtr = getFOP(ptr->handle,_id_c);
	if (fieldOffsetsPtr == NULL) {
		return JS_FALSE;
	}
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("ptr=%p _id_c=%s node=%p node->_nodeType=%d stringNodeType=%s\n",ptr,_id_c,node,node->_nodeType,stringNodeType(node->_nodeType)) ;
	printf ("getSFNodeField, fieldOffsetsPtr is %d for node %u, field %s\n",fieldOffsetsPtr, ptr->handle, _id_c);
	#endif


	/* now, we have an X3D node, offset, type, etc. Just get the value from memory, and move
	   it to javascript. Kind of like: void getField_ToJavascript (int num, int fromoffset) 
	   for Routing. */

	/* fieldOffsetsPtr points to a 4-number line like:
		FIELDNAMES__parentResource, offsetof (struct X3D_Anchor, __parenturl),  FIELDTYPE_SFString, KW_initializeOnly */
        switch (*(fieldOffsetsPtr+2)) {
		case FIELDTYPE_SFBool:
		case FIELDTYPE_SFFloat:
		case FIELDTYPE_SFTime:
		case FIELDTYPE_SFInt32:
		case FIELDTYPE_SFString:
			X3D_ECMA_TO_JS(context, offsetPointer_deref (void *, node, *(fieldOffsetsPtr+1)),
				returnElementLength(*(fieldOffsetsPtr+2)), *(fieldOffsetsPtr+2), vp);
			break;
		case FIELDTYPE_SFColor:
		case FIELDTYPE_SFNode:
		case FIELDTYPE_SFVec2f:
		case FIELDTYPE_SFVec3f:
		case FIELDTYPE_SFVec3d:
		case FIELDTYPE_SFRotation:
			X3D_SF_TO_JS(context, obj, offsetPointer_deref (void *, node, *(fieldOffsetsPtr+1)),
				returnElementLength(*(fieldOffsetsPtr+2)) * returnElementRowSize(*(fieldOffsetsPtr+2)) , *(fieldOffsetsPtr+2), vp);
			break;
		case FIELDTYPE_MFColor:
		case FIELDTYPE_MFVec3f:
		case FIELDTYPE_MFVec2f:
		case FIELDTYPE_MFFloat:
		case FIELDTYPE_MFTime:
		case FIELDTYPE_MFInt32:
		case FIELDTYPE_MFString:
		case FIELDTYPE_MFNode:
		case FIELDTYPE_MFRotation:
		case FIELDTYPE_SFImage:
			X3D_MF_TO_JS(context, obj, offsetPointer_deref (void *, node, *(fieldOffsetsPtr+1)), *(fieldOffsetsPtr+2), vp, 
				(char *)FIELDNAMES[*(fieldOffsetsPtr+0)]);
			break;
		default: printf ("unhandled type FIELDTYPE_ %d in getSFNodeField\n", *(fieldOffsetsPtr+2)) ;
		return JS_FALSE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("end of getSFNodeField\n");
	#endif

	return JS_TRUE;
}

/* setter for SFNode accesses */
JSBool setSFNodeField (JSContext *context, JSObject *obj, jsval id, jsval *vp) {
	char *_id_c;
        SFNodeNative *ptr;
	int *fieldOffsetsPtr;
	struct X3D_Node *node;

	/* get the id field... */

	_id_c = JS_GetStringBytes(JSVAL_TO_STRING(id));
	
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("\nsetSFNodeField called on name %s object %u, jsval %u\n",_id_c, obj, *vp);
	#endif

	/* get the private data. This will contain a pointer into the FreeWRL scenegraph */
        if ((ptr = (SFNodeNative *)JS_GetPrivate(context, obj)) == NULL) {
                printf( "JS_GetPrivate failed in setSFNodeField.\n");
                return JS_FALSE;
        }

	/* get the X3D Scenegraph node pointer from this Javascript SFNode node */
	node = (struct X3D_Node *) ptr->handle;

	if (node == NULL) {
		printf ("setSFNodeField, can not set field \"%s\", NODE is NULL!\n",_id_c);
		return JS_FALSE;
	}

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("so then, we have a node of type %s\n",stringNodeType(node->_nodeType));
	#endif

	/* get the table entry giving the type, offset, etc. of this field in this node */
	fieldOffsetsPtr = getFOP(ptr->handle,_id_c);
	if (fieldOffsetsPtr == NULL) {
		return JS_FALSE;
	}

	/* now, we have an X3D node, offset, type, etc. Just get the value from Javascript, and move
	   it to the X3D Scenegraph. */

	/* fieldOffsetsPtr points to a 4-number line like:
		FIELDNAMES__parentResource, offsetof (struct X3D_Anchor, __parenturl),  FIELDTYPE_SFString, KW_initializeOnly */
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("and a field type of %s\n",FIELDTYPES[*(fieldOffsetsPtr+2)]);
	#endif

        switch (*(fieldOffsetsPtr+2)) {
		case FIELDTYPE_SFBool:
		case FIELDTYPE_SFFloat:
		case FIELDTYPE_SFTime:
		case FIELDTYPE_SFInt32:
		case FIELDTYPE_SFString:
			JS_ECMA_TO_X3D(context, ((void *)( ((unsigned char *) node) + *(fieldOffsetsPtr+1))),
				returnElementLength(*(fieldOffsetsPtr+2)), *(fieldOffsetsPtr+2), vp);
			break;
		case FIELDTYPE_SFColor:
		case FIELDTYPE_SFNode:
		case FIELDTYPE_SFVec2f:
		case FIELDTYPE_SFVec3f:
		case FIELDTYPE_SFVec3d:
		case FIELDTYPE_SFRotation:
			JS_SF_TO_X3D(context, ((void *)( ((unsigned char *) node) + *(fieldOffsetsPtr+1))),
				returnElementLength(*(fieldOffsetsPtr+2)) * returnElementRowSize(*(fieldOffsetsPtr+2)) , *(fieldOffsetsPtr+2), vp);
			break;
		case FIELDTYPE_MFColor:
		case FIELDTYPE_MFVec3f:
		case FIELDTYPE_MFVec2f:
		case FIELDTYPE_MFFloat:
		case FIELDTYPE_MFTime:
		case FIELDTYPE_MFInt32:
		case FIELDTYPE_MFString:
		case FIELDTYPE_MFNode:
		case FIELDTYPE_MFRotation:
		case FIELDTYPE_SFImage:
			JS_MF_TO_X3D(context, obj, ((void *)( ((unsigned char *) node) + *(fieldOffsetsPtr+1))), *(fieldOffsetsPtr+2), vp);
			break;
		default: printf ("unhandled type in setSFNodeField\n");
		return JS_FALSE;
	}

	/* tell the X3D Scenegraph that something has changed in Kansas */
	update_node(node);

	#ifdef JSVRMLCLASSESVERBOSE
	printf ("end of setSFNodeField\n");
	#endif


	return JS_TRUE;
}

/* for SFNodes, go through and insure that all properties are defined for the specific node type */
int JS_DefineSFNodeSpecificProperties (JSContext *context, JSObject *object, struct X3D_Node * ptr) {
	int *fieldOffsetsPtr;
	jsval rval = INT_TO_JSVAL(0);
	uintN attrs = JSPROP_PERMANENT 
		| JSPROP_ENUMERATE 
#ifdef JSPROP_EXPORTED
		| JSPROP_EXPORTED 
#endif
	/*	| JSPROP_INDEX */
		;
	char *name;
	SFNodeNative *nodeNative;
	#ifdef JSVRMLCLASSESVERBOSE
	char *nodeName;
	struct X3D_Node *confirmNode;
	#endif

	#ifdef JSVRMLCLASSESVERBOSE
        nodeName = parser_getNameFromNode(ptr) ; /* vi +/dump_scene src/lib/scenegraph/GeneratedCode.c */
        if (nodeName == NULL) {
		printf ("\nStart of JS_DefineSFNodeSpecificProperties for '---' ... working on node %u object %u (%p,%p)\n",ptr,object,ptr,object);
        } else {
		printf ("\nStart of JS_DefineSFNodeSpecificProperties for '%s' ... working on node %u object %u (%p,%p)\n",nodeName,ptr,object,ptr,object);
		confirmNode = parser_getNodeFromName(nodeName);
        	if (confirmNode == NULL) {
			printf("RoundTrip failed : ptr (%p) -> nodeName (%s) -----\n",ptr,nodeName) ;
		} else {
			printf("RoundTrip OK : ptr (%p) -> nodeName (%s) -> confirmNode (%p)\n",ptr,nodeName,confirmNode) ;
		}
        }
	#endif 

	if (ptr != NULL) {
		#ifdef JSVRMLCLASSESVERBOSE
		printf ("...JS_DefineSFNodeSpecificProperties... it is a %s\n",stringNodeType(ptr->_nodeType));
		#endif

		/* have we already done this for this node? We really do not want to do this again */
		if ((nodeNative = (SFNodeNative *)JS_GetPrivate(context,object)) == NULL) {
			printf ("JS_DefineSFNodeSpecificProperties, can not get private for a SFNode!\n");
			return JS_FALSE;
		}
		if (nodeNative->fieldsExpanded) {
			#ifdef JSVRMLCLASSESVERBOSE
			printf ("JS_DefineSFNodeSpecificProperties, already done for node\n");
			#endif

			return JS_TRUE;
		}

                fieldOffsetsPtr = (int *) NODE_OFFSETS[ptr->_nodeType];
                /*go thru all field*/
		/* what we have is a list of 4 numbers, representing:
        		FIELDNAMES__parentResource, offsetof (struct X3D_Anchor, __parenturl),  FIELDTYPE_SFString, KW_initializeOnly,
		*/

                while (*fieldOffsetsPtr != -1) {
                        /* fieldPtr=(char*)structptr+(*(fieldOffsetsPtr+1)); */
			#ifdef JSVRMLCLASSESVERBOSE
                        printf ("looking at field %s type %s\n",FIELDNAMES[*fieldOffsetsPtr],FIELDTYPES[*(fieldOffsetsPtr+2)]); 
			#endif
			
			/* skip any fieldNames starting with an underscore, as these are "internal" ones */
			if (FIELDNAMES[*fieldOffsetsPtr][0] != '_') {
				name = (char *)FIELDNAMES[*fieldOffsetsPtr];
				rval = INT_TO_JSVAL(*fieldOffsetsPtr);

				/* is this an initializeOnly property? */
				/* lets not do this, ok? 
				if ((*(fieldOffsetsPtr+3)) == KW_initializeOnly) attrs |= JSPROP_READONLY;
				*/

				#ifdef JSVRMLCLASSESVERBOSE
				printf ("calling JS_DefineProperty on (context=%p, object=%p, name=%s, rval=%p), setting getSFNodeField, setSFNodeField\n",context,object,name,rval);
				#endif

        			if (!JS_DefineProperty(context, object,  name, rval, getSFNodeField, setSFNodeField, attrs)) {
        			        printf("JS_DefineProperty failed for \"%s\" in JS_DefineSFNodeSpecificProperties.\n", name);
        			        return JS_FALSE;
        			}
			}
			fieldOffsetsPtr += 5;
		}

		/* set a flag indicating that we have been here already */
		nodeNative->fieldsExpanded = TRUE;
	}
	#ifdef JSVRMLCLASSESVERBOSE
	printf ("JS_DefineSFNodeSpecificProperties, returning TRUE\n");
	#endif

	return TRUE;
}


/********************************************************************************************/
/* new addition April 2009. It was noted that the following code would not send an event to
   FreeWRL:
#VRML V2.0 utf8
      DEF DisplayScript Script {
        eventOut MFString display_string

        url [ "javascript:
          function eventsProcessed () {
		display_string[7] = ' ';
          }
        "]
      }


Shape {geometry DEF Display Text {}}
    ROUTE DisplayScript.display_string TO Display.set_string

(it would if the assignment was display_string = new MFString(...) )

But, this property check gets called on the equals. Lets figure out how to indicate that the
holding object needs to route to FreeWRL... */


#define SET_TOUCHED_TYPE_MF_A(thisMFtype,thisSFtype) \
	else if (JS_InstanceOf (cx, obj, &thisMFtype##Class, NULL)) {\
		jsval mainElement;\
		thisSFtype##Native *ptr; \
\
		if (!JS_GetElement(cx, obj, num, &mainElement)) { \
			printf ("JS_GetElement failed for %d in get_valueChanged_flag\n",num); \
			return JS_FALSE; \
		} \
\
		if ((ptr = (thisSFtype##Native *)JS_GetPrivate(cx, (JSObject *)mainElement)) == NULL) {\
			printf( "JS_GetPrivate failed in assignCheck.\n"); \
			return JS_FALSE; \
		} else { \
			/* printf ("got private for MFVec3f, doing it...\n"); */ \
			ptr->valueChanged++; \
		} \
		return JS_TRUE; \
        }



JSBool js_SetPropertyCheck (JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
	int num=0;

#ifdef VERBOSE
	char *_id_c = "(no value in string)";
	/* get the id field... */
	if (JSVAL_IS_STRING(id)) { 
		_id_c = JS_GetStringBytes(JSVAL_TO_STRING(id)); 
        	/* printf ("hmmm...js_SetPropertyCheck called on string \"%s\" object %u, jsval %u\n",_id_c, obj, *vp); */
	} else if (JSVAL_IS_DOUBLE(id)) {
		_id_c = JS_GetStringBytes(JSVAL_TO_STRING(id)); 
        	printf ("\n...js_SetPropertyCheck called on double %s object %u, jsval %u\n",_id_c, obj, *vp);
	} else if (JSVAL_IS_INT(id)) {
		num = JSVAL_TO_INT(id);
        	printf ("\n...js_SetPropertyCheck called on number %d object %u, jsval %u\n",num, obj, *vp); 
	} else {
        	printf ("hmmm...js_SetPropertyCheck called on unknown type of object %u, jsval %u\n", obj, *vp);
	}
#endif

	/* lets worry about the MFs containing ECMAs here - MFFloat MFInt32 MFTime MFString MFBool */

	if (JS_InstanceOf (cx, obj, &MFStringClass, NULL)) {
		SET_MF_ECMA_HAS_CHANGED;
		return JS_TRUE;
	}
	else if (JS_InstanceOf (cx, obj, &MFFloatClass, NULL)) {
		SET_MF_ECMA_HAS_CHANGED;
		return JS_TRUE;
	}
	else if (JS_InstanceOf (cx, obj, &MFInt32Class, NULL)) {
		SET_MF_ECMA_HAS_CHANGED;
		return JS_TRUE;
	}

#ifdef NEWCLASSES
	else if (JS_InstanceOf (cx, obj, &MFBoolClass, NULL)) {
		SET_MF_ECMA_HAS_CHANGED;
		return JS_TRUE;
	}
#endif

        SET_TOUCHED_TYPE_MF_A(MFRotation,SFRotation)
        SET_TOUCHED_TYPE_MF_A(MFNode,SFNode)
        SET_TOUCHED_TYPE_MF_A(MFVec2f,SFVec2f)
        SET_TOUCHED_TYPE_MF_A(MFVec3f,SFVec3f)
        /* SET_TOUCHED_TYPE_MF_A(MFImage,SFImage)  */
        SET_TOUCHED_TYPE_MF_A(MFColor,SFColor)
        /* SET_TOUCHED_TYPE_MF_A(MFColorRGBA,SFColorRGBA) */


        #ifdef JSVRMLCLASSESVERBOSE
	printf ("this is a class of "); printJSNodeType (cx,obj);
        #endif
	
	return JS_TRUE;

}

/****************************************************************************/

JSBool js_GetPropertyDebug (JSContext *context, JSObject *obj, jsval id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE 
	char *_id_c = "(no value in string)";
	int num;
	/* get the id field... */

	if (JSVAL_IS_STRING(id)) { 
		_id_c = JS_GetStringBytes(JSVAL_TO_STRING(id)); 
        	printf ("\n...js_GetPropertyDebug called on string \"%s\" object %u, jsval %lu\n",_id_c, (unsigned int) obj, *vp);
	} else if (JSVAL_IS_INT(id)) {
		num = JSVAL_TO_INT(id);
        	printf ("\n...js_GetPropertyDebug called on number %d object %u, jsval %lu\n",num, (unsigned int) obj, *vp);
	} else {
        	printf ("\n...js_GetPropertyDebug called on unknown type of object %u, jsval %lu\n", (unsigned int) obj, *vp);
	}
	#endif
	return JS_TRUE;
}

JSBool js_SetPropertyDebug (JSContext *context, JSObject *obj, jsval id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE 
	char *_id_c = "(no value in string)";
	int num;

	/* get the id field... */
	if (JSVAL_IS_STRING(id)) { 
		_id_c = JS_GetStringBytes(JSVAL_TO_STRING(id)); 
        	printf ("\n...js_SetPropertyDebug called on string \"%s\" object %u, jsval %lu\n",_id_c, (unsigned int) obj, *vp);
	} else if (JSVAL_IS_INT(id)) {
		num = JSVAL_TO_INT(id);
        	printf ("\n...js_SetPropertyDebug called on number %d object %u, jsval %lu\n",num, (unsigned int) obj, *vp);
	} else {
        	printf ("\n...js_SetPropertyDebug called on unknown type of object %u, jsval %lu\n", (unsigned int) obj, *vp);
	}
	#endif
	return JS_TRUE;
}
JSBool js_SetPropertyDebug1 (JSContext *context, JSObject *obj, jsval id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE 
	char *_id_c = "(no value in string)";
	int num;

	/* get the id field... */
	if (JSVAL_IS_STRING(id)) { 
		_id_c = JS_GetStringBytes(JSVAL_TO_STRING(id)); 
        	printf ("\n...js_SetPropertyDebug1 called on string \"%s\" object %u, jsval %lu\n",_id_c, (unsigned int) obj, *vp);
	} else if (JSVAL_IS_INT(id)) {
		num = JSVAL_TO_INT(id);
        	printf ("\n...js_SetPropertyDebug1 called on number %d object %u, jsval %lu\n",num, (unsigned int) obj, *vp);
	} else {
        	printf ("\n...js_SetPropertyDebug1 called on unknown type of object %u, jsval %lu\n", (unsigned int) obj, *vp);
	}
	#endif
	return JS_TRUE;
}
JSBool js_SetPropertyDebug2 (JSContext *context, JSObject *obj, jsval id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE 
	char *_id_c = "(no value in string)";
	int num;

	/* get the id field... */
	if (JSVAL_IS_STRING(id)) { 
		_id_c = JS_GetStringBytes(JSVAL_TO_STRING(id)); 
        	printf ("...js_SetPropertyDebug2 called on string \"%s\" object %u, jsval %lu\n",_id_c, (unsigned int) obj, *vp);
	} else if (JSVAL_IS_INT(id)) {
		num = JSVAL_TO_INT(id);
        	printf ("...js_SetPropertyDebug2 called on number %d object %u, jsval %lu\n",num, (unsigned int) obj, *vp);
	} else {
        	printf ("...js_SetPropertyDebug2 called on unknown type of object %u, jsval %lu\n", (unsigned int) obj, *vp);
	}
	#endif
	return JS_TRUE;
}
JSBool js_SetPropertyDebug3 (JSContext *context, JSObject *obj, jsval id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE 
	char *_id_c = "(no value in string)";
	int num;

	/* get the id field... */
	if (JSVAL_IS_STRING(id)) { 
		_id_c = JS_GetStringBytes(JSVAL_TO_STRING(id)); 
        	printf ("\n...js_SetPropertyDebug3 called on string \"%s\" object %u, jsval %lu\n",_id_c, (unsigned int) obj, *vp);
	} else if (JSVAL_IS_INT(id)) {
		num = JSVAL_TO_INT(id);
        	printf ("\n...js_SetPropertyDebug3 called on number %d object %u, jsval %lu\n",num, (unsigned int) obj, *vp);
	} else {
        	printf ("\n...js_SetPropertyDebug3 called on unknown type of object %u, jsval %lu\n", (unsigned int) obj, *vp);
	}
	#endif
	return JS_TRUE;
}
JSBool js_SetPropertyDebug4 (JSContext *context, JSObject *obj, jsval id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE 
	char *_id_c = "(no value in string)";
	int num;

	/* get the id field... */
	if (JSVAL_IS_STRING(id)) { 
		_id_c = JS_GetStringBytes(JSVAL_TO_STRING(id)); 
        	printf ("\n...js_SetPropertyDebug4 called on string \"%s\" object %u, jsval %lu\n",_id_c, (unsigned int) obj, *vp);
	} else if (JSVAL_IS_INT(id)) {
		num = JSVAL_TO_INT(id);
        	printf ("\n...js_SetPropertyDebug4 called on number %d object %u, jsval %lu\n",num, (unsigned int) obj, *vp);
	} else {
        	printf ("\n...js_SetPropertyDebug4 called on unknown type of object %u, jsval %lu\n", (unsigned int) obj, *vp);
	}
	#endif
	return JS_TRUE;
}
JSBool js_SetPropertyDebug5 (JSContext *context, JSObject *obj, jsval id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE 
	char *_id_c = "(no value in string)";
	int num;

	/* get the id field... */
	if (JSVAL_IS_STRING(id)) { 
		_id_c = JS_GetStringBytes(JSVAL_TO_STRING(id)); 
        	printf ("\n...js_SetPropertyDebug5 called on string \"%s\" object %u, jsval %lu\n",_id_c, (unsigned int) obj, *vp);
	} else if (JSVAL_IS_INT(id)) {
		num = JSVAL_TO_INT(id);
        	printf ("\n...js_SetPropertyDebug5 called on number %d object %u, jsval %lu\n",num, (unsigned int) obj, *vp);
	} else {
        	printf ("\n...js_SetPropertyDebug5 called on unknown type of object %u, jsval %lu\n", (unsigned int) obj, *vp);
	}
	#endif
	return JS_TRUE;
}
JSBool js_SetPropertyDebug6 (JSContext *context, JSObject *obj, jsval id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE 
	char *_id_c = "(no value in string)";
	int num;

	/* get the id field... */
	if (JSVAL_IS_STRING(id)) { 
		_id_c = JS_GetStringBytes(JSVAL_TO_STRING(id)); 
        	printf ("\n...js_SetPropertyDebug6 called on string \"%s\" object %u, jsval %lu\n",_id_c, (unsigned int) obj, *vp);
	} else if (JSVAL_IS_INT(id)) {
		num = JSVAL_TO_INT(id);
        	printf ("\n...js_SetPropertyDebug6 called on number %d object %u, jsval %lu\n",num, (unsigned int) obj, *vp);
	} else {
        	printf ("\n...js_SetPropertyDebug6 called on unknown type of object %u, jsval %lu\n", (unsigned int) obj, *vp);
	}
	#endif
	return JS_TRUE;
}
JSBool js_SetPropertyDebug7 (JSContext *context, JSObject *obj, jsval id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE 
	char *_id_c = "(no value in string)";
	int num;

	/* get the id field... */
	if (JSVAL_IS_STRING(id)) { 
		_id_c = JS_GetStringBytes(JSVAL_TO_STRING(id)); 
        	printf ("\n...js_SetPropertyDebug7 called on string \"%s\" object %u, jsval %lu\n",_id_c, (unsigned int) obj, *vp);
	} else if (JSVAL_IS_INT(id)) {
		num = JSVAL_TO_INT(id);
        	printf ("\n...js_SetPropertyDebug7 called on number %d object %u, jsval %lu\n",num, (unsigned int) obj, *vp);
	} else {
        	printf ("\n...js_SetPropertyDebug7 called on unknown type of object %u, jsval %lu\n", (unsigned int) obj, *vp);
	}
	#endif
	return JS_TRUE;
}
JSBool js_SetPropertyDebug8 (JSContext *context, JSObject *obj, jsval id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE 
	char *_id_c = "(no value in string)";
	int num;

	/* get the id field... */
	if (JSVAL_IS_STRING(id)) { 
		_id_c = JS_GetStringBytes(JSVAL_TO_STRING(id)); 
        	printf ("\n...js_SetPropertyDebug8 called on string \"%s\" object %u, jsval %lu\n",_id_c, (unsigned int) obj, *vp);
	} else if (JSVAL_IS_INT(id)) {
		num = JSVAL_TO_INT(id);
        	printf ("\n...js_SetPropertyDebug8 called on number %d object %u, jsval %lu\n",num, (unsigned int) obj, *vp);
	} else {
        	printf ("\n...js_SetPropertyDebug8 called on unknown type of object %u, jsval %lu\n", (unsigned int) obj, *vp);
	}
	#endif
	return JS_TRUE;
}
JSBool js_SetPropertyDebug9 (JSContext *context, JSObject *obj, jsval id, jsval *vp) {
	#ifdef JSVRMLCLASSESVERBOSE 
	char *_id_c = "(no value in string)";
	int num;

	/* get the id field... */
	if (JSVAL_IS_STRING(id)) { 
		_id_c = JS_GetStringBytes(JSVAL_TO_STRING(id)); 
        	printf ("\n...js_SetPropertyDebug9 called on string \"%s\" object %u, jsval %lu\n",_id_c, (unsigned int) obj, *vp);
	} else if (JSVAL_IS_INT(id)) {
		num = JSVAL_TO_INT(id);
        	printf ("\n...js_SetPropertyDebug9 called on number %d object %u, jsval %lu\n",num, (unsigned int) obj, *vp);
	} else {
        	printf ("\n...js_SetPropertyDebug9 called on unknown type of object %u, jsval %lu\n", (unsigned int) obj, *vp);
	}
	#endif
	return JS_TRUE;
}
