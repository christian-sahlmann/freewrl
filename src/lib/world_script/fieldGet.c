/*
=INSERT_TEMPLATE_HERE=

$Id: fieldGet.c,v 1.43 2011/05/16 17:47:08 crc_canada Exp $

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
#include <system_threads.h>
#include <system_js.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../vrml_parser/CRoutes.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../main/Snapshot.h"
#include "../scenegraph/Collision.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"
#include "../input/EAIHeaders.h"
#include "../input/EAIHelpers.h"	/* resolving implicit declarations */
#include "../input/SensInterps.h"
#include "../x3d_parser/Bindable.h"

#include "JScript.h"
#include "CScripts.h"
#include "jsUtils.h"
#include "jsNative.h"
#include "jsVRMLClasses.h"
#include "fieldSet.h"
#include "fieldGet.h"

#ifdef HAVE_JAVASCRIPT
/********************************************************************

getField_ToJavascript.

this sends events to scripts that have eventIns defined.

********************************************************************/

void getField_ToJavascript (int num, int fromoffset) {
	int ignored;

	#ifdef SETFIELDVERBOSE 
		printf ("getField_ToJavascript, from offset %d type %d num=%d\n",
			fromoffset,JSparamnames[fromoffset].type,num);
	#endif

	/* set the parameter */
	/* see comments in gatherScriptEventOuts to see exact formats */

	switch (JSparamnames[fromoffset].type) {
	case FIELDTYPE_SFBool:
	case FIELDTYPE_SFFloat:
	case FIELDTYPE_SFTime:
	case FIELDTYPE_SFDouble:
	case FIELDTYPE_SFInt32:
	case FIELDTYPE_SFString:
		setScriptECMAtype(num);
		break;
	case FIELDTYPE_SFColor:
	case FIELDTYPE_SFNode:
	case FIELDTYPE_SFVec2f:
	case FIELDTYPE_SFVec3f:
	case FIELDTYPE_SFVec3d:
	case FIELDTYPE_SFRotation:
		setScriptMultiElementtype(num);
		break;
	case FIELDTYPE_MFColor:
	case FIELDTYPE_MFVec3f:
	case FIELDTYPE_MFVec3d:
	case FIELDTYPE_MFVec2f:
	case FIELDTYPE_MFFloat:
	case FIELDTYPE_MFTime:
	case FIELDTYPE_MFInt32:
	case FIELDTYPE_MFString:
	case FIELDTYPE_MFNode:
	case FIELDTYPE_MFRotation:
	case FIELDTYPE_SFImage:
		ignored = setMFElementtype(num);
		break;
	default : {
		printf("WARNING: sendScriptEventIn type %s not handled yet\n",
			FIELDTYPES[JSparamnames[fromoffset].type]);
		}
	}
}


/******************************************************************************/

void set_one_ECMAtype (int tonode, int toname, int dataType, void *Data, int datalen) {
	char scriptline[100];
	jsval newval;
	JSContext *cx;
	JSObject *obj;

	#ifdef SETFIELDVERBOSE
	printf ("set_one_ECMAtype, to %d namepointer %d, fieldname %s, datatype %d length %d\n",
		tonode,toname,JSparamnames[toname].name,dataType,datalen); 
	#endif

	/* get context and global object for this script */
	cx =  ScriptControl[tonode].cx;
	obj = ScriptControl[tonode].glob;

	/* set the time for this script */
	SET_JS_TICKTIME()

	X3D_ECMA_TO_JS(cx, Data, datalen, dataType, &newval);

	/* get the variable name to hold the incoming value */
	sprintf (scriptline,"__eventIn_Value_%s", JSparamnames[toname].name);

	#ifdef SETFIELDVERBOSE
	printf ("set_one_ECMAtype, calling JS_DefineProperty on name %s obj %u, setting setECMANative, 0 \n",scriptline,obj);
	#endif

        if (!JS_DefineProperty(cx,obj, scriptline, newval, JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_PERMANENT)) {  
                printf( "JS_DefineProperty failed for \"ECMA in\" at %s:%d.\n",__FILE__,__LINE__); 
                return; 
        }

	/* is the function compiled yet? */
	COMPILE_FUNCTION_IF_NEEDED(toname)

	/* and run the function */
	RUN_FUNCTION (toname)
}

/*  setScriptECMAtype called by getField_ToJavascript for 
        case FIELDTYPE_SFBool:
        case FIELDTYPE_SFFloat:
        case FIELDTYPE_SFTime:
        case FIELDTYPE_SFDouble:
        case FIELDTYPE_SFInt32:
        case FIELDTYPE_SFString:
*/

void setScriptECMAtype (int num) {
	void *fn;
	int tptr;
	int len;
	int to_counter;
	CRnodeStruct *to_ptr = NULL;

	fn = offsetPointer_deref(void *, CRoutes[num].routeFromNode, CRoutes[num].fnptr);
	len = CRoutes[num].len;

	for (to_counter = 0; to_counter < CRoutes[num].tonode_count; to_counter++) {
                struct Shader_Script *myObj;

		to_ptr = &(CRoutes[num].tonodes[to_counter]);
                myObj = X3D_SCRIPT(to_ptr->routeToNode)->__scriptObj;
		/* printf ("setScriptECMAtype, myScriptNumber is %d\n",myObj->num); */
		tptr = to_ptr->foffset;
		set_one_ECMAtype (myObj->num, tptr, JSparamnames[tptr].type, fn,len);
	}
}


/* use Javascript to send in one element of an MF. datalen is in number of elements in type. */
int set_one_MFElementType(int tonode, int toname, int dataType, void *Data, int datalen) {
	JSContext *cx;
	JSObject *obj;
	int elementlen;
	int x;
	char scriptline[20000];
	//float *fp, *fp_in=(float *)Data;
	//int *ip;

	/* for PixelTextures we have: */
	struct X3D_PixelTexture *mePix;
	struct Multi_Int32 image;

	/* for MFStrings we have: */
	char *chptr;
	struct Uni_String  **uniptr;

	/* get context and global object for this script */
	cx =  ScriptControl[tonode].cx;
	obj = ScriptControl[tonode].glob;

	/* set the TickTime (possibly again) for this context */
	SET_JS_TICKTIME_RV(FALSE)

	/* make up the name */
	switch (dataType) {
		case FIELDTYPE_MFRotation: {	
			JSObject *newMFObject;
			JSObject *newSFObject;
			SFRotationNative 	*SFRPptr;
			float *fp, *fp_in=(float *)Data;

			/* create a new MFRotation object... */
			newMFObject = JS_ConstructObject(cx, &MFRotationClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = (int) sizeof (float);
			for (x=0; x<datalen; x++) {
				/* create a new SFRotation object */
				newSFObject = JS_ConstructObject(cx,&SFRotationClass,NULL, newMFObject);
				if ((SFRPptr = (SFRotationNative *)JS_GetPrivate(cx, newSFObject)) == NULL) {
					printf ("failure in getting SF class at %s:%d\n",__FILE__,__LINE__);
					return FALSE;
				}

				/* fill the private pointer area */
				fp = (float *)fp_in; SFRPptr->v.c[0] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
				fp = (float *)fp_in; SFRPptr->v.c[1] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
				fp = (float *)fp_in; SFRPptr->v.c[2] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
				fp = (float *)fp_in; SFRPptr->v.c[3] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);

				/* put this object into the MF class */
				if (!JS_DefineElement(cx, newMFObject, (jsint) x, OBJECT_TO_JSVAL(newSFObject),
					JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
						printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
				}
			}

			/* set the length of this MF */
			SET_LENGTH (cx,newMFObject,datalen)

			/* set the global variable with this new MF object */
			SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

			/* run the function */
			COMPILE_FUNCTION_IF_NEEDED(toname)
			RUN_FUNCTION(toname)
			break;
		}

		case FIELDTYPE_MFVec3f: {	
			JSObject *newMFObject;
			JSObject *newSFObject;
			SFVec3fNative 	*SFRPptr;
			float *fp, *fp_in=(float *)Data;

			/* create a new MFVec3f object... */
			newMFObject = JS_ConstructObject(cx, &MFVec3fClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = (int) sizeof (float);
			for (x=0; x<datalen; x++) {
				/* create a new SFVec3f object */
				newSFObject = JS_ConstructObject(cx,&SFVec3fClass,NULL, newMFObject);
				if ((SFRPptr = (SFVec3fNative *)JS_GetPrivate(cx, newSFObject)) == NULL) {
					printf ("failure in getting SF class at %s:%d\n",__FILE__,__LINE__);
					return FALSE;
				}

				/* fill the private pointer area */
				fp = (float *)fp_in; SFRPptr->v.c[0] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
				fp = (float *)fp_in; SFRPptr->v.c[1] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
				fp = (float *)fp_in; SFRPptr->v.c[2] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);

				/* put this object into the MF class */
				if (!JS_DefineElement(cx, newMFObject, (jsint) x, OBJECT_TO_JSVAL(newSFObject),
					JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
						printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
				}
			}

			/* set the length of this MF */
			SET_LENGTH (cx,newMFObject,datalen)

			/* set the global variable with this new MF object */
			SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

			/* run the function */
			COMPILE_FUNCTION_IF_NEEDED(toname)
			RUN_FUNCTION(toname)
			break;
		}

		case FIELDTYPE_MFColor: {	
			JSObject *newMFObject;
			JSObject *newSFObject;
			SFColorNative 	*SFRPptr;
			float *fp, *fp_in=(float *)Data;

			/* create a new MFColor object... */
			newMFObject = JS_ConstructObject(cx, &MFColorClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = (int) sizeof (float);
			for (x=0; x<datalen; x++) {
				/* create a new SFColor object */
				newSFObject = JS_ConstructObject(cx,&SFColorClass,NULL, newMFObject);
				if ((SFRPptr = (SFColorNative *)JS_GetPrivate(cx, newSFObject)) == NULL) {
					printf ("failure in getting SF class at %s:%d\n",__FILE__,__LINE__);
					return FALSE;
				}

				/* fill the private pointer area */
				fp = (float *)fp_in; SFRPptr->v.c[0] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
				fp = (float *)fp_in; SFRPptr->v.c[1] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
				fp = (float *)fp_in; SFRPptr->v.c[2] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);

				/* put this object into the MF class */
				if (!JS_DefineElement(cx, newMFObject, (jsint) x, OBJECT_TO_JSVAL(newSFObject),
					JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
						printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
				}
			}

			/* set the length of this MF */
			SET_LENGTH (cx,newMFObject,datalen)

			/* set the global variable with this new MF object */
			SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

			/* run the function */
			COMPILE_FUNCTION_IF_NEEDED(toname)
			RUN_FUNCTION(toname)
			break;
		} 

		case FIELDTYPE_MFVec2f: {	
			JSObject *newMFObject;
			JSObject *newSFObject;
			SFVec2fNative 	*SFRPptr;
			float *fp, *fp_in=(float *)Data;

			/* create a new MFVec2f object... */
			newMFObject = JS_ConstructObject(cx, &MFVec2fClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = (int) sizeof (float);
			for (x=0; x<datalen; x++) {
				/* create a new SFVec2f object */
				newSFObject = JS_ConstructObject(cx,&SFVec2fClass,NULL, newMFObject);
				if ((SFRPptr = (SFVec2fNative *)JS_GetPrivate(cx, newSFObject)) == NULL) {
					printf ("failure in getting SF class at %s:%d\n",__FILE__,__LINE__);
					return FALSE;
				}

				/* fill the private pointer area */
				fp = (float *)fp_in; SFRPptr->v.c[0] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);
				fp = (float *)fp_in; SFRPptr->v.c[1] = *fp; fp_in = offsetPointer_deref(float *,fp_in,elementlen);

				/* put this object into the MF class */
				if (!JS_DefineElement(cx, newMFObject, (jsint) x, OBJECT_TO_JSVAL(newSFObject),
					JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
						printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
				}
			}

			/* set the length of this MF */
			SET_LENGTH (cx,newMFObject,datalen)

			/* set the global variable with this new MF object */
			SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

			/* run the function */
			COMPILE_FUNCTION_IF_NEEDED(toname)
			RUN_FUNCTION(toname)
			break;
		}


		case FIELDTYPE_MFFloat: {	
			JSObject *newMFObject;
			jsval newjsval;
			float *fp, *fp_in=(float *)Data;
			/* create a new MFFloat object... */
			newMFObject = JS_ConstructObject(cx, &MFFloatClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = (int) sizeof (float);
			for (x=0; x<datalen; x++) {
				/* create a new SFFloat object */
				
				fp = (float *)fp_in; 
				JS_NewNumberValue(cx,(double)*fp,&newjsval);
				fp_in = offsetPointer_deref(float *,fp_in,elementlen);

				/* put this object into the MF class */
				if (!JS_DefineElement(cx, newMFObject, (jsint) x, OBJECT_TO_JSVAL(newjsval),
					JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
						printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
				}
			}

			/* set the length of this MF */
			SET_LENGTH (cx,newMFObject,datalen)

			/* set the global variable with this new MF object */
			SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

			/* run the function */
			COMPILE_FUNCTION_IF_NEEDED(toname)
			RUN_FUNCTION(toname)
			break;
		}
		case FIELDTYPE_MFTime: {	
			JSObject *newMFObject;
			jsval newjsval;
			double *dp, *dp_in=(double *)Data;

			/* create a new MFTime object... */
			newMFObject = JS_ConstructObject(cx, &MFTimeClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = (int) sizeof (double);
			for (x=0; x<datalen; x++) {
				/* create a new SFTime object */
				
				dp = (double *)dp_in; 
				JS_NewNumberValue(cx,(double)*dp,&newjsval);
				dp_in = offsetPointer_deref(double *,dp_in,elementlen);

				/* put this object into the MF class */
				if (!JS_DefineElement(cx, newMFObject, (jsint) x, OBJECT_TO_JSVAL(newjsval),
					JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
						printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
				}
			}

			/* set the length of this MF */
			SET_LENGTH (cx,newMFObject,datalen)

			/* set the global variable with this new MF object */
			SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

			/* run the function */
			COMPILE_FUNCTION_IF_NEEDED(toname)
			RUN_FUNCTION(toname)
			break;
		}
		case FIELDTYPE_MFInt32: {	
			JSObject *newMFObject;
			jsval newjsval;
			int *ip, *ip_in=(int *)Data;

			/* create a new MFInt32 object... */
			newMFObject = JS_ConstructObject(cx, &MFInt32Class, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = (int) sizeof (float);
			for (x=0; x<datalen; x++) {
				/* create a new SFInt32 object */
				
				ip = (int *)ip_in; 
				newjsval = INT_TO_JSVAL(ip);
				ip_in = offsetPointer_deref(int *,ip_in,elementlen);

				/* put this object into the MF class */
				if (!JS_DefineElement(cx, newMFObject, (jsint) x, newjsval,
					JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
						printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
				}
			}

			/* set the length of this MF */
			SET_LENGTH (cx,newMFObject,datalen)

			/* set the global variable with this new MF object */
			SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

			/* run the function */
			COMPILE_FUNCTION_IF_NEEDED(toname)
			RUN_FUNCTION(toname)
			break;
		}
		case FIELDTYPE_MFString: {	
			JSObject *newMFObject;
			jsval newjsval;
			struct Uni_String * *ip_in=(struct Uni_String **)Data;

			/* create a new MFString object... */
			newMFObject = JS_ConstructObject(cx, &MFStringClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* Data points to a Uni_String */
			uniptr = (struct Uni_String **) ip_in;

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			for (x=0; x<datalen; x++) {
				/* create a new SFString object */
				
				chptr = uniptr[x]->strptr;
				newjsval = STRING_TO_JSVAL( JS_NewStringCopyZ(cx,chptr));

				/* put this object into the MF class */
				if (!JS_DefineElement(cx, newMFObject, (jsint) x, newjsval,
					JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
						printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
				}
			}

			/* set the length of this MF */
			SET_LENGTH (cx,newMFObject,datalen)

			/* set the global variable with this new MF object */
			SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

			/* run the function */
			COMPILE_FUNCTION_IF_NEEDED(toname)
			RUN_FUNCTION(toname)
			break;
		}
		case FIELDTYPE_MFNode: {	
			JSObject *newMFObject;
			jsval newjsval;
			double *ip, *ip_in=(double *)Data;
			/* create a new MFNode object... */
			newMFObject = JS_ConstructObject(cx, &MFNodeClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = (int) sizeof (void *);
			for (x=0; x<datalen; x++) {
				ip = ip_in; 
				newjsval = INT_TO_JSVAL(ip);
				ip_in = offsetPointer_deref(double *,ip_in,elementlen);

				/* put this object into the MF class */
				if (!JS_DefineElement(cx, newMFObject, (jsint) x, newjsval,
					JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
						printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
				}
			}

			/* set the length of this MF */
			SET_LENGTH (cx,newMFObject,datalen)

			/* set the global variable with this new MF object */
			SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

			/* run the function */
			COMPILE_FUNCTION_IF_NEEDED(toname)
			RUN_FUNCTION(toname)
			break;
		}

		case FIELDTYPE_SFImage:	{
			JSObject *newMFObject;
			jsval newjsval;
			double *ip, *ip_in=(double *)Data;

			/* create a new MFNode object... */
			newMFObject = JS_ConstructObject(cx, &SFImageClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			mePix = (struct X3D_PixelTexture *) ip_in;
			if (mePix->_nodeType == NODE_PixelTexture) {
				image = mePix->image;
				if (image.n > 2) {
					datalen = image.n;
					/* copy over the image */
					for (x=0; x<datalen; x++) {
						newjsval = INT_TO_JSVAL(image.p[x]);
						/* put this object into the MF class */
						if (!JS_DefineElement(cx, newMFObject, (jsint) x, newjsval,
						JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
							printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
						}
					}
				} else {
					datalen = 3;
					for (x=0; x<datalen; x++) {
						newjsval = INT_TO_JSVAL(0);
						/* put this object into the MF class */
						if (!JS_DefineElement(cx, newMFObject, (jsint) x, newjsval,
						JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
							printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
						}
					}
				}

			} else {
				/* make up a "zero" Pixeltexture */
					datalen = 3;
					for (x=0; x<datalen; x++) {
						newjsval = INT_TO_JSVAL(0);
						/* put this object into the MF class */
						if (!JS_DefineElement(cx, newMFObject, (jsint) x, newjsval,
						JS_GET_PROPERTY_STUB, JS_SET_PROPERTY_STUB3, JSPROP_ENUMERATE)) {
							printf("failure in inserting SF class at %s:%d\n",__FILE__,__LINE__);
						}
					}
			}

			/* set the length of this MF */
			SET_LENGTH (cx,newMFObject,datalen)

			/* set the global variable with this new MF object */
			SET_EVENTIN_VALUE (cx,obj,toname,newMFObject)

			/* run the function */
			COMPILE_FUNCTION_IF_NEEDED(toname)
			RUN_FUNCTION(toname)

			break;
			} 

		default: {
				printf ("setMFElement, SHOULD NOT DISPLAY THIS\n");
				strcat (scriptline,"(");
			}
	}
	return TRUE;
}


/* setMFElementtype called by getField_ToJavascript for
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
*/



int setMFElementtype (int num) {
	void * fn;
	int fptr;
	int len;
	int to_counter;
	CRnodeStruct *to_ptr = NULL;
	char *pptr;
	struct Multi_Node *mfp;


	#ifdef SETFIELDVERBOSE 
		printf("------------BEGIN setMFElementtype ---------------\n");
	#endif


	fn = (void *)CRoutes[num].routeFromNode;
	fptr = CRoutes[num].fnptr;
	
	/* we can do arithmetic on character pointers; so we have to cast void *
	   to char * here */
	pptr = offsetPointer_deref (char *, fn, fptr);

	len = CRoutes[num].len;

	/* is this from a MFElementType? positive lengths in routing table  == easy memcpy types */
	if (len <= 0) {
		mfp = (struct Multi_Node *) pptr;

		/* check Multimemcpy for C to C routing for this type */
		/* get the number of elements */
		len = mfp->n;  
		pptr = (char *) mfp->p; /* pptr is a char * just for math stuff */
		#ifdef SETFIELDVERBOSE 
		printf ("setMFElementtype, len now %d, from %d\n",len,fn);
		#endif
	} else {
		/* SFImages will have a length of greater than zero */
		/* printf ("setMFElementtype, length is greater than 0 (%d), how can this be?\n",len); */
	}

	/* go through all the nodes that this script sends to for this entry in the CRoutes table */
	for (to_counter = 0; to_counter < CRoutes[num].tonode_count; to_counter++) {
                struct Shader_Script *myObj;

		to_ptr = &(CRoutes[num].tonodes[to_counter]);
                myObj = X3D_SCRIPT(to_ptr->routeToNode)->__scriptObj;

		#ifdef SETFIELDVERBOSE 
			printf ("got a script event! index %d type %d\n",
					num, CRoutes[num].direction_flag);
/*
			printf ("\tfrom %#x from ptr %#x\n\tto %#x toptr %#x\n",fn,fptr,tn,to_ptr->foffset);
			printf ("\tfrom %d from ptr %d\n\tto %d toptr %d\n",fn,fptr,tn,to_ptr->foffset);
*/
			printf ("\tdata length %d\n",len);
			printf ("and, sending it to %s as type %d\n",JSparamnames[to_ptr->foffset].name,
					JSparamnames[to_ptr->foffset].type);
		#endif

		set_one_MFElementType(myObj->num, to_ptr->foffset, JSparamnames[to_ptr->foffset].type, (void *)pptr,len);
	}


	#ifdef SETFIELDVERBOSE 
		printf("------------END setMFElementtype ---------------\n");
	#endif
	return FALSE; /* return value never checked; #defines expect a return value */
}


/****************************************************************/
/* sets a SFVec3f and SFColor and SFVec3d 			*/
/* and SFRotation and SFVec2fin a script 			*/
/*								*/
/* all *Native types have the same structure of the struct -	*/
/* we are just looking for the pointer, thus we can handle	*/
/* multi types here 						*/
/* sets a SFVec3f and SFColor in a script 			*/
/****************************************************************/

/* get a pointer to the internal data for this object, or return NULL on error */
void **getInternalDataPointerForJavascriptObject(JSContext *cx, JSObject *obj, int tnfield) {
	char scriptline[100];
	void *_privPtr;
	JSObject *sfObj;
	jsval retval;

	/* get the variable name to hold the incoming value */
	sprintf (scriptline,"__eventIn_Value_%s", JSparamnames[tnfield].name);
	#ifdef SETFIELDVERBOSE 
	printf ("getInternalDataPointerForJavascriptObject: line %s\n",scriptline);
	#endif

	if (!JS_GetProperty(cx,obj,scriptline,&retval))
		printf ("JS_GetProperty failed in set_one_MultiElementType.\n");

	if (!JSVAL_IS_OBJECT(retval))
		printf ("set_one_MultiElementType - not an object\n");

	sfObj = JSVAL_TO_OBJECT(retval);

	if ((_privPtr = JS_GetPrivate(cx, sfObj)) == NULL)
		printf("JS_GetPrivate failed set_one_MultiElementType.\n");

	if (_privPtr == NULL) return NULL;

	/* what kind of class of object is this? */

	/* we look at EVERY kind of native class found in "jsNative.h" even
	   if it may not be ever used here */

        if (JS_InstanceOf(cx, sfObj, &SFVec3fClass, NULL)) { 
		SFVec3fNative *me = (SFVec3fNative *)_privPtr;
		return (void **) &me->v;
		
        } else if (JS_InstanceOf(cx, sfObj, &SFVec3dClass, NULL)) { 
		SFVec3dNative *me = (SFVec3dNative *)_privPtr;
		return (void **) &me->v;
		
        } else if (JS_InstanceOf(cx, sfObj, &SFRotationClass, NULL)) { 
		SFRotationNative *me = (SFRotationNative *)_privPtr;
		return (void **) &me->v;
		
        } else if (JS_InstanceOf(cx, sfObj, &SFVec2fClass, NULL)) { 
		SFVec2fNative *me = (SFVec2fNative *)_privPtr;
		return (void **) &me->v;
		
        } else if (JS_InstanceOf(cx, sfObj, &SFColorClass, NULL)) { 
		SFColorNative *me = (SFColorNative *)_privPtr;
		return (void **) &me->v;
		
        } else if (JS_InstanceOf(cx, sfObj, &SFColorRGBAClass, NULL)) { 
		SFColorRGBANative *me = (SFColorRGBANative *)_privPtr;
		return (void **) &me->v;
		
        } else if (JS_InstanceOf(cx, sfObj, &SFVec4fClass, NULL)) { 
		SFVec4fNative *me = (SFVec4fNative *)_privPtr;
		return (void **) &me->v;
		
        } else if (JS_InstanceOf(cx, sfObj, &SFVec4dClass, NULL)) { 
		SFVec4dNative *me = (SFVec4dNative *)_privPtr;
		return (void **) &me->v;
		
        } else if (JS_InstanceOf(cx, sfObj, &SFNodeClass, NULL)) { 
		SFNodeNative *me = (SFNodeNative *)_privPtr;
		//JAS return (void **) &me->v;
		
        } else if (JS_InstanceOf(cx, sfObj, &SFImageClass, NULL)) { 
		SFImageNative *me = (SFImageNative *)_privPtr;
		//JAS return (void **) &me->v;
		
        }

	ConsoleMessage ("getInternalDataPointerForJavascriptObject malfunction");

	return NULL;
}



/* really do the individual set; used by script routing and EAI sending to a script */
void set_one_MultiElementType (int tonode, int tnfield, void *Data, int dataLen ) {
	char scriptline[100];
	JSContext *cx;
	JSObject *obj;
	void **pp;

	/* get context and global object for this script */
	cx =  ScriptControl[tonode].cx;
	obj = ScriptControl[tonode].glob;

	/* set the time for this script */
	SET_JS_TICKTIME()

	/* copy over the data from the VRML side into the script variable. */
	pp = getInternalDataPointerForJavascriptObject(cx,obj,tnfield);

	if (pp != NULL) {
		memcpy (pp,Data, dataLen);
		/* printf ("set_one_MultiElementType, dataLen %d, sizeof(double) %d\n",dataLen, sizeof(double));
		printf ("and, sending the data to pointer %p\n",pp); */
	}

	/* is the function compiled yet? */
	COMPILE_FUNCTION_IF_NEEDED(tnfield)

	/* and run the function */
	#ifdef SETFIELDVERBOSE
	printf ("set_one_MultiElementType: running script %s\n",scriptline);
	#endif

	RUN_FUNCTION (tnfield)
}


/* setScriptMultiElementtype called by getField_ToJavascript for 
        case FIELDTYPE_SFColor:
        case FIELDTYPE_SFNode:
        case FIELDTYPE_SFVec2f:
        case FIELDTYPE_SFVec3f:
        case FIELDTYPE_SFRotation:
*/

void setScriptMultiElementtype (int num)
{
	int tptr, fptr;
	int len;
	int to_counter;

	CRnodeStruct *to_ptr = NULL;

	void *fn;

	JSContext *cx;
	JSObject *obj;

	fn = (void *)CRoutes[num].routeFromNode;
	fptr = CRoutes[num].fnptr;
	if (CRoutes[num].len == ROUTING_SFNODE) len = returnElementLength(FIELDTYPE_SFNode);
	else if (CRoutes[num].len < 0) {
		ConsoleMessage ("setScriptMultiElementtype - len of %d unhandled\n",CRoutes[num].len);
		return;
	} else {
		len = CRoutes[num].len;
	}

	for (to_counter = 0; to_counter < CRoutes[num].tonode_count; to_counter++) {
                struct Shader_Script *myObj;

                to_ptr = &(CRoutes[num].tonodes[to_counter]);
                myObj = X3D_SCRIPT(to_ptr->routeToNode)->__scriptObj;

		/* the to_node should be a script number; it will be a small integer */
		tptr = to_ptr->foffset;

		#ifdef SETFIELDVERBOSE 
			printf ("got a script event! index %d type %d\n",
					num, CRoutes[num].direction_flag);
			printf ("\tfrom %#x from ptr %#x\n\tto %#x toptr %#x\n",fn,fptr,myObj->num,tptr);
			printf ("\tdata length %d\n",len);
			printf ("setScriptMultiElementtype here script number  %d tptr %d len %d\n",myObj->num, tptr,len);
		#endif

		/* get context and global object for this script */
		cx =  ScriptControl[myObj->num].cx;
		obj = ScriptControl[myObj->num].glob;

		fn = offsetPointer_deref(void*,fn,fptr); /*fn += fptr;*/

		set_one_MultiElementType (myObj->num, tptr, fn, len);
	}
}


#endif /* HAVE_JAVASCRIPT */

/* convert a number in memory to a printable type. Used to send back EVents, or replies to
   the SAI/EAI client program. */

void EAI_Convert_mem_to_ASCII (int id, char *reptype, int type, char *memptr, char *buf) {

	char utilBuf[EAIREADSIZE];
	int errcount;
	memset(utilBuf,'\0',sizeof(utilBuf));

	errcount = UtilEAI_Convert_mem_to_ASCII (type,memptr, utilBuf);
	if (0 == errcount) {
		sprintf (buf,"%s\n%f\n%d\n%s",reptype,TickTime,id, utilBuf);
	} else {
		sprintf (buf,"%s\n%f\n%d\n%s",reptype,TickTime,id, "indeterminate....");
	}
}

/* Utility routine to convert a value in memory to a printable type. */

int UtilEAI_Convert_mem_to_ASCII (int type, char *memptr, char *buf) { /* Returns errcount */

	double dval;
	float fl[4];
	double dl[4];
	float *fp;
	int *ip;
	int ival;
	struct X3D_Node *uval;
	int row;			/* MF* counter */
	struct Multi_String *MSptr;	/* MFString pointer */
	struct Multi_Node *MNptr;	/* MFNode pointer */
	struct Multi_Color *MCptr;	/* MFColor pointer */
	char *ptr;			/* used for building up return string */
	struct Uni_String *svptr;
	char *retSFString;

	int numPerRow;			/* 1, 2, 3 or 4 floats per row of this MF? */
	int i, errcount;

	/* used because of endian problems... */
	int *intptr;
	intptr = (int *) memptr;

/* printf("%s,%d UtilEAI_Convert_mem_to_ASCII (type=%d , memptr=%p intptr=%p ....)\n",__FILE__,__LINE__,type,memptr,intptr); */

	errcount=0;
	switch (type) {
		case FIELDTYPE_SFBool: 	{
			if (eaiverbose) { 
			printf ("UtilEAI_Convert_mem_to_ASCII: EAI_SFBOOL - value %d; TRUE %d false %d\n",*intptr,TRUE,FALSE);
			}

			if (*intptr == 1) sprintf (buf,"TRUE");
			else sprintf (buf,"FALSE");
			break;
		}

		case FIELDTYPE_SFDouble:
		case FIELDTYPE_SFTime:	{
			if (eaiverbose) { 
			printf ("UtilEAI_Convert_mem_to_ASCII: EAI_SFTIME\n");
			}
			memcpy(&dval,memptr,sizeof(double));
			sprintf (buf, "%lf",dval);
			break;
		}

		case FIELDTYPE_SFInt32:	{
			if (eaiverbose) { 
			printf ("UtilEAI_Convert_mem_to_ASCII: EAI_SFINT32\n");
			}
			memcpy(&ival,memptr,sizeof(int));
			sprintf (buf, "%d",ival);
			break;
		}

		case FIELDTYPE_SFNode:	{
			if (eaiverbose) { 
			printf ("UtilEAI_Convert_mem_to_ASCII: EAI_SFNODE\n");
			}
			memcpy((void *)&uval,(void *)memptr,sizeof(void *));
			sprintf (buf, "%u",registerEAINodeForAccess(X3D_NODE(uval)));
			break;
		}

		case FIELDTYPE_SFFloat:	{
			if (eaiverbose) { 
			printf ("UtilEAI_Convert_mem_to_ASCII: EAI_SFFLOAT\n");
			}

			memcpy(fl,memptr,sizeof(float));
			sprintf (buf, "%f",fl[0]);
			break;
		}

		case FIELDTYPE_SFVec3f:
		case FIELDTYPE_SFColor:	{
			if (eaiverbose) { 
			printf ("UtilEAI_Convert_mem_to_ASCII: EAI_SFCOLOR or EAI_SFVEC3F\n");
			}
			memcpy(fl,memptr,sizeof(float)*3);
			sprintf (buf, "%f %f %f",fl[0],fl[1],fl[2]);
			break;
		}

		case FIELDTYPE_SFVec3d:	{
			if (eaiverbose) { 
			printf ("UtilEAI_Convert_mem_to_ASCII: EAI_SFVEC3D\n");
			}
			memcpy(dl,memptr,sizeof(double)*3);
			sprintf (buf, "%lf %lf %lf",dl[0],dl[1],dl[2]);
			break;
		}

		case FIELDTYPE_SFVec2f:	{
			if (eaiverbose) { 
			printf ("UtilEAI_Convert_mem_to_ASCII: EAI_SFVEC2F\n");
			}
			memcpy(fl,memptr,sizeof(float)*2);
			sprintf (buf, "%f %f",fl[0],fl[1]);
			break;
		}

		case FIELDTYPE_SFColorRGBA:
		case FIELDTYPE_SFRotation:	{
			if (eaiverbose) { 
			printf ("UtilEAI_Convert_mem_to_ASCII: EAI_SFROTATION\n");
			}

			memcpy(fl,memptr,sizeof(float)*4);
			sprintf (buf, "%f %f %f %f",fl[0],fl[1],fl[2],fl[3]);
			break;
		}

		case FIELDTYPE_SFImage:
		case FIELDTYPE_SFString:	{
			uintptr_t *xx;

			if (eaiverbose) { 
			printf ("UtilEAI_Convert_mem_to_ASCII: EAI_SFSTRING\n");
			}

			/* get the pointer to the string, do this in a couple of steps... */
			svptr = (struct Uni_String *)memptr;
			xx= (uintptr_t *) memptr;
			svptr = (struct Uni_String *) *xx;

			retSFString = (char *)svptr->strptr; 
			sprintf (buf, "\"%s\"",retSFString);
			break;
		}

		case FIELDTYPE_MFString:	{
			if (eaiverbose) { 
			printf ("UtilEAI_Convert_mem_to_ASCII: EAI_MFSTRING\n");
			}

			/* make the Multi_String pointer */
			MSptr = (struct Multi_String *) memptr;

			/* printf ("UtilEAI_Convert_mem_to_ASCII: EAI_MFString, there are %d strings\n",(*MSptr).n);*/
			ptr = buf + strlen(buf);

			for (row=0; row<(*MSptr).n; row++) {
        	        	/* printf ("UtilEAI_Convert_mem_to_ASCII: String %d is %s\n",row,(*MSptr).p[row]->strptr); */
				if (strlen ((*MSptr).p[row]->strptr) == 0) {
					sprintf (ptr, "\"\" "); /* encode junk for Java side.*/
				} else {
					sprintf (ptr, "\"%s\" ",(*MSptr).p[row]->strptr);
				}
				/* printf ("UtilEAI_Convert_mem_to_ASCII: buf now is %s\n",buf); */
				ptr = buf + strlen (buf);
			}

			break;
		}

		case FIELDTYPE_MFNode: 	{
			MNptr = (struct Multi_Node *) memptr;

			if (eaiverbose) { 
			printf ("UtilEAI_Convert_mem_to_ASCII: EAI_MFNode, there are %d nodes at %p\n",(*MNptr).n,memptr);
			}

			ptr = buf + strlen(buf);

			for (row=0; row<(*MNptr).n; row++) {
				sprintf (ptr, "%d ",registerEAINodeForAccess(X3D_NODE((*MNptr).p[row])));
				ptr = buf + strlen (buf);
			}
			break;
		}

		case FIELDTYPE_MFInt32: {
			MCptr = (struct Multi_Color *) memptr;
			if (eaiverbose) { 
				printf ("UtilEAI_Convert_mem_to_ASCII: EAI_MFColor, there are %d nodes at %p\n",(*MCptr).n,memptr);
			}

			sprintf (buf, "%d \n",(*MCptr).n);
			ptr = buf + strlen(buf);

			ip = (int *) (*MCptr).p;
			for (row=0; row<(*MCptr).n; row++) {
				sprintf (ptr, "%d \n",*ip); 
				ip++;
				/* printf ("UtilEAI_Convert_mem_to_ASCII: line %d is ",row,ptr);  */
				ptr = buf + strlen (buf);
			}

			break;
		}

		case FIELDTYPE_MFFloat:
		case FIELDTYPE_MFVec2f:
		case FIELDTYPE_MFVec3f:
		case FIELDTYPE_MFRotation:
		case FIELDTYPE_MFColorRGBA:
		case FIELDTYPE_MFColor: {
			numPerRow=3;
			if (type==FIELDTYPE_MFFloat) {numPerRow=1;}
			else if (type==FIELDTYPE_MFVec2f) {numPerRow=2;}
			else if (type==FIELDTYPE_MFRotation) {numPerRow=4;}
			else if (type==FIELDTYPE_MFColorRGBA) {numPerRow=4;}

			MCptr = (struct Multi_Color *) memptr;
			if (eaiverbose) { 
				printf ("UtilEAI_Convert_mem_to_ASCII: EAI_MFColor, there are %d nodes at %p\n",(*MCptr).n,memptr);
			}

			sprintf (buf, "%d \n",(*MCptr).n);
			ptr = buf + strlen(buf);


			fp = (float *) (*MCptr).p;
			for (row=0; row<(*MCptr).n; row++) {
				for (i=0; i<numPerRow; i++) {
					fl[i] = *fp; fp++;
				}
				switch (numPerRow) {
					case 1:
						sprintf (ptr, "%f \n",fl[0]); break;
					case 2:
						sprintf (ptr, "%f %f \n",fl[0],fl[1]); break;
					case 3:
						sprintf (ptr, "%f %f %f \n",fl[0],fl[1],fl[2]); break;
					case 4:
						sprintf (ptr, "%f %f %f %f \n",fl[0],fl[1],fl[2],fl[3]); break;
				}
				/* printf ("UtilEAI_Convert_mem_to_ASCII: line %d is ",row,ptr); */
				ptr = buf + strlen (buf);
			}

			break;
		}
		default: {
			errcount++;
			printf ("UtilEAI_Convert_mem_to_ASCII: EAI, type %d (%s) not handled yet\n",type,stringFieldtypeType (type));
		}


/*XXX	case EAI_MFTIME:	{handleptr = &handleEAI_MFTIME_Listener;break;}*/
	}
	return errcount ;
}

