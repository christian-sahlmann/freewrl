/*
=INSERT_TEMPLATE_HERE=

$Id: fieldGet.c,v 1.20 2009/08/06 21:24:03 couannette Exp $

Javascript C language binding.

*/

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
#include "../input/EAIheaders.h"
#include "../input/EAIHelpers.h"
#include "../input/SensInterps.h"
#include "../x3d_parser/Bindable.h"

#include "CScripts.h"
#include "jsUtils.h"
#include "jsNative.h"
#include "jsVRMLClasses.h"
#include "fieldSet.h"
#include "fieldGet.h"

void set_one_ECMAtype (uintptr_t tonode, int toname, int dataType, void *Data, unsigned datalen);
/********************************************************************

getField_ToJavascript.

this sends events to scripts that have eventIns defined.

********************************************************************/
void getField_ToJavascript (int num, int fromoffset) {
	int ignored;

	#ifdef SETFIELDVERBOSE 
		printf ("CRoutes, sending ScriptEventIn from offset %d type %d num=%ld\n",
			fromoffset,JSparamnames[fromoffset].type,num);
	#endif

	/* set the parameter */
	/* see comments in gatherScriptEventOuts to see exact formats */

	switch (JSparamnames[fromoffset].type) {
	case FIELDTYPE_SFBool:
	case FIELDTYPE_SFFloat:
	case FIELDTYPE_SFTime:
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

void set_one_ECMAtype (uintptr_t tonode, int toname, int dataType, void *Data, unsigned datalen) {

	char scriptline[100];
	jsval newval;
	JSContext *cx;
	JSObject *obj;

	#ifdef SETFIELDVERBOSE
	printf ("set_one_ECMAtype, to %d namepointer %d, fieldname %s, datatype %d length %d\n",
		tonode,toname,JSparamnames[toname].name,dataType,datalen); 
	#endif

	/* get context and global object for this script */
	cx = (JSContext *) ScriptControl[tonode].cx;
	obj = (JSObject *)ScriptControl[tonode].glob;

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
        case FIELDTYPE_SFInt32:
        case FIELDTYPE_SFString:
*/

void setScriptECMAtype (uintptr_t num) {
	uintptr_t fn, tn;
	int tptr;
	int len;
	unsigned int to_counter;
	CRnodeStruct *to_ptr = NULL;

	fn = (uintptr_t)(CRoutes[num].routeFromNode) + (uintptr_t)(CRoutes[num].fnptr);
	len = CRoutes[num].len;

	for (to_counter = 0; to_counter < CRoutes[num].tonode_count; to_counter++) {
                struct Shader_Script *myObj;

		to_ptr = &(CRoutes[num].tonodes[to_counter]);
                myObj = X3D_SCRIPT(to_ptr->routeToNode)->__scriptObj;
		/* printf ("setScriptECMAtype, myScriptNumber is %d\n",myObj->num); */
		tn = (uintptr_t) to_ptr->routeToNode;
		tptr = to_ptr->foffset;
		set_one_ECMAtype (myObj->num, tptr, JSparamnames[tptr].type, (void *)fn,len);
	}
}


/* use Javascript to send in one element of an MF. datalen is in number of elements in type. */
int set_one_MFElementType(uintptr_t tonode, int toname, int dataType, void *Data, unsigned datalen) {
	JSContext *cx;
	JSObject *obj;
	int elementlen;
	int x;
	char scriptline[20000];
	float *fp, *fp_in=(float *)Data;
	int *ip;

	/* for PixelTextures we have: */
	struct X3D_PixelTexture *mePix;
	struct Multi_Int32 image;

	/* for MFStrings we have: */
	char *chptr;
	struct Uni_String  **uniptr;

	/* get context and global object for this script */
	cx = (JSContext *) ScriptControl[tonode].cx;
	obj = (JSObject *)ScriptControl[tonode].glob;

	/* set the TickTime (possibly again) for this context */
	SET_JS_TICKTIME(FALSE)

	/* make up the name */
	switch (dataType) {
		case FIELDTYPE_MFRotation: {	
			JSObject *newMFObject;
			JSObject *newSFObject;
			SFRotationNative 	*SFRPptr;

			/* create a new MFRotation object... */
			newMFObject = JS_ConstructObject(cx, &MFRotationClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = sizeof (float);
			for (x=0; x<datalen; x++) {
				/* create a new SFRotation object */
				newSFObject = JS_ConstructObject(cx,&SFRotationClass,NULL, newMFObject);
				if ((SFRPptr = (SFRotationNative *)JS_GetPrivate(cx, newSFObject)) == NULL) {
					printf ("failure in getting SF class at %s:%d\n",__FILE__,__LINE__);
					return FALSE;
				}

				/* fill the private pointer area */
				fp = (float *)fp_in; SFRPptr->v.c[0] = *fp; fp_in += elementlen;
				fp = (float *)fp_in; SFRPptr->v.c[1] = *fp; fp_in += elementlen;
				fp = (float *)fp_in; SFRPptr->v.c[2] = *fp; fp_in += elementlen;
				fp = (float *)fp_in; SFRPptr->v.c[3] = *fp; fp_in += elementlen;

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

			/* create a new MFVec3f object... */
			newMFObject = JS_ConstructObject(cx, &MFVec3fClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = sizeof (float);
			for (x=0; x<datalen; x++) {
				/* create a new SFVec3f object */
				newSFObject = JS_ConstructObject(cx,&SFVec3fClass,NULL, newMFObject);
				if ((SFRPptr = (SFVec3fNative *)JS_GetPrivate(cx, newSFObject)) == NULL) {
					printf ("failure in getting SF class at %s:%d\n",__FILE__,__LINE__);
					return FALSE;
				}

				/* fill the private pointer area */
				fp = (float *)fp_in; SFRPptr->v.c[0] = *fp; fp_in += elementlen;
				fp = (float *)fp_in; SFRPptr->v.c[1] = *fp; fp_in += elementlen;
				fp = (float *)fp_in; SFRPptr->v.c[2] = *fp; fp_in += elementlen;

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

			/* create a new MFColor object... */
			newMFObject = JS_ConstructObject(cx, &MFColorClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = sizeof (float);
			for (x=0; x<datalen; x++) {
				/* create a new SFColor object */
				newSFObject = JS_ConstructObject(cx,&SFColorClass,NULL, newMFObject);
				if ((SFRPptr = (SFColorNative *)JS_GetPrivate(cx, newSFObject)) == NULL) {
					printf ("failure in getting SF class at %s:%d\n",__FILE__,__LINE__);
					return FALSE;
				}

				/* fill the private pointer area */
				fp = (float *)fp_in; SFRPptr->v.c[0] = *fp; fp_in += elementlen;
				fp = (float *)fp_in; SFRPptr->v.c[1] = *fp; fp_in += elementlen;
				fp = (float *)fp_in; SFRPptr->v.c[2] = *fp; fp_in += elementlen;

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

			/* create a new MFVec2f object... */
			newMFObject = JS_ConstructObject(cx, &MFVec2fClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = sizeof (float);
			for (x=0; x<datalen; x++) {
				/* create a new SFVec2f object */
				newSFObject = JS_ConstructObject(cx,&SFVec2fClass,NULL, newMFObject);
				if ((SFRPptr = (SFVec2fNative *)JS_GetPrivate(cx, newSFObject)) == NULL) {
					printf ("failure in getting SF class at %s:%d\n",__FILE__,__LINE__);
					return FALSE;
				}

				/* fill the private pointer area */
				fp = (float *)fp_in; SFRPptr->v.c[0] = *fp; fp_in += elementlen;
				fp = (float *)fp_in; SFRPptr->v.c[1] = *fp; fp_in += elementlen;

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
			/* create a new MFFloat object... */
			newMFObject = JS_ConstructObject(cx, &MFFloatClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = sizeof (float);
			for (x=0; x<datalen; x++) {
				/* create a new SFFloat object */
				
				fp = (float *)fp_in; 
				newjsval = DOUBLE_TO_JSVAL(JS_NewDouble(cx,(double)*fp));
				fp_in += elementlen;

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
			/* create a new MFTime object... */
			newMFObject = JS_ConstructObject(cx, &MFTimeClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = sizeof (float);
			for (x=0; x<datalen; x++) {
				/* create a new SFTime object */
				
				fp = (float *)fp_in; 
				newjsval = DOUBLE_TO_JSVAL(JS_NewDouble(cx,(double)*fp));
				fp_in += elementlen;

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
			/* create a new MFInt32 object... */
			newMFObject = JS_ConstructObject(cx, &MFInt32Class, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = sizeof (float);
			for (x=0; x<datalen; x++) {
				/* create a new SFInt32 object */
				
				ip = (int *)fp_in; 
				newjsval = INT_TO_JSVAL(ip);
				fp_in += elementlen;

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
			/* create a new MFString object... */
			newMFObject = JS_ConstructObject(cx, &MFStringClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* Data points to a Uni_String */
			uniptr = (struct Uni_String **) fp_in;

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
			/* create a new MFNode object... */
			newMFObject = JS_ConstructObject(cx, &MFNodeClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			elementlen = sizeof (float);
			for (x=0; x<datalen; x++) {
				ip = (int *)fp_in; 
				newjsval = INT_TO_JSVAL(ip);
				fp_in += elementlen;

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
			/* create a new MFNode object... */
			newMFObject = JS_ConstructObject(cx, &SFImageClass, NULL ,JS_GetParent(cx, obj));
			ADD_ROOT (cx, newMFObject)

			/* define the "length" property for this object */ 
			DEFINE_LENGTH(cx,newMFObject)

			/* fill in private pointer area */
			mePix = (struct X3D_PixelTexture *) fp_in;
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



int setMFElementtype (uintptr_t num) {
	void * fn;
	uintptr_t fptr;
	int len;
	unsigned int to_counter;
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
	pptr = (char *)fn + fptr;

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


#ifdef OLDCODE
void set_EAI_MFElementtype (int num, int offset, unsigned char *pptr, int len) {

    int tn, tptr;
    char scriptline[2000];
    char sline[100];
    jsval retval;
    int x;
    int elementlen;
    float *fp;
    int *ip;
    double *dp;

    JSContext *cx;
    JSObject *obj;

    #ifdef SETFIELDVERBOSE 
	printf("------------BEGIN set_EAI_MFElementtype ---------------\n");
    #endif


    tn   = num;
    tptr = offset;

    #ifdef SETFIELDVERBOSE 
	printf ("got a script event! index %d\n",num);
	printf ("\tto %#x toptr %#x\n",tn,tptr);
	printf ("\tdata length %d\n",len);
	printf ("and, sending it to %s\n",JSparamnames[tptr].name);
    #endif

    /* get context and global object for this script */
    cx = (JSContext *) ScriptControl[tn].cx;
    obj = (JSObject *)ScriptControl[tn].glob;
    SET_JS_TICKTIME()

    /* make up the name */
    sprintf (scriptline,"%s(",JSparamnames[tptr].name);
    switch (JSparamnames[tptr].type) {
      case FIELDTYPE_MFVec3f: {
	  strcat (scriptline, "new MFVec3f(");
	  elementlen = sizeof (float) * 3;
	  for (x=0; x<(len/elementlen); x++) {
	      fp = (float *)pptr;
	      sprintf (sline,"%f %f %f",*fp,
		       *(fp+elementlen),
		       *(fp+(elementlen*2)));
	      if (x < ((len/elementlen)-1)) {
		  strcat(sline,",");
	      }
	      pptr += elementlen;
	      strcat (scriptline,sline);
	  }
	  break;
      }
      case FIELDTYPE_MFColor: {
	  strcat (scriptline, "new MFColor(");
	  elementlen = sizeof (float) * 3;
	  for (x=0; x<(len/elementlen); x++) {
	      fp = (float *)pptr;
	      sprintf (sline,"%f %f %f",*fp,
		       *(fp+elementlen),
		       *(fp+(elementlen*2)));
	      if (x < ((len/elementlen)-1)) {
		  strcat(sline,",");
	      }
	      pptr += elementlen;
	      strcat (scriptline,sline);
	  }
	  break;
      }
      case FIELDTYPE_MFFloat: {
	  strcat (scriptline, "new MFFloat(");
	  elementlen = sizeof (float);
	  for (x=0; x<(len/elementlen); x++) {
	      fp = (float *)pptr;
	      sprintf (sline,"%f",*fp);
	      if (x < ((len/elementlen)-1)) {
		  strcat(sline,",");
	      }
	      pptr += elementlen;
	      strcat (scriptline,sline);
	  }

	  break;
      }
      case FIELDTYPE_MFTime:  {
	  strcat (scriptline, "new MFTime(");
	  elementlen = sizeof (double);
	  for (x=0; x<(len/elementlen); x++) {
	      dp = (double *)pptr;
	      sprintf (sline,"%lf",*dp);
	      if (x < ((len/elementlen)-1)) {
		  strcat(sline,",");
	      }
	      pptr += elementlen;
	      strcat (scriptline,sline);
	  }
	  break;
      }
      case FIELDTYPE_MFInt32: {
	  strcat (scriptline, "new MFInt32(");
	  elementlen = sizeof (int);
	  for (x=0; x<(len/elementlen); x++) {
	      ip = (int *)pptr;
	      sprintf (sline,"%d",*ip);
	      if (x < ((len/elementlen)-1)) {
		  strcat(sline,",");
	      }
	      pptr += elementlen;
	      strcat (scriptline,sline);
	  }
	  break;
      }
      case FIELDTYPE_MFString:{
	  strcat (scriptline, "new MFString(");
	  elementlen = sizeof (float);
	  printf ("ScriptAssign, MFString probably broken\n");
	  for (x=0; x<(len/elementlen); x++) {
	      fp = (float *)pptr;
	      sprintf (sline,"%f",*fp);
	      if (x < ((len/elementlen)-1)) {
		  strcat(sline,",");
	      }
	      pptr += elementlen;
	      strcat (scriptline,sline);
	  }
	  break;
      }
      case FIELDTYPE_MFNode:  {
	  strcat (scriptline, "new MFNode(");
	  elementlen = sizeof (int);
	  for (x=0; x<(len/elementlen); x++) {
	      ip = (int *)pptr;
	      sprintf (sline,"%u",*ip);
	      if (x < ((len/elementlen)-1)) {
		  strcat(sline,",");
	      }
	      pptr += elementlen;
	      strcat (scriptline,sline);
	  }
	  break;
      }
      case FIELDTYPE_MFRotation: {	strcat (scriptline, "new MFRotation(");
      elementlen = sizeof (float)*4;
      for (x=0; x<(len/elementlen); x++) {
	  fp = (float *)pptr;
	  sprintf (sline,"%f %f %f %f",*fp,
		   *(fp+elementlen),
		   *(fp+(elementlen*2)),
		   *(fp+(elementlen*3)));
	  sprintf (sline,"%f",*fp);
	  if (x < ((len/elementlen)-1)) {
	      strcat(sline,",");
	  }
	  pptr += elementlen;
	  strcat (scriptline,sline);
      }
      break;
      }
      default: {
	  printf ("setMFElement, SHOULD NOT DISPLAY THIS\n");
	  strcat (scriptline,"(");
      }
    }

    /* convert these values to a jsval type */
    strcat (scriptline,"))");

    #ifdef SETFIELDVERBOSE 
	printf("ScriptLine: %s\n",scriptline);
    #endif

    if (!ACTUALRUNSCRIPT(tn,scriptline,&retval))
      printf ("AR failed in setxx\n");

    #ifdef SETFIELDVERBOSE 
	printf("------------END set_EAI_MFElementtype ---------------\n");
    #endif
}
#endif /* OLDCODE */

/****************************************************************/
/* sets a SFVec3f and SFColor and SFVec3d 			*/
/* and SFRotation and SFVec2fin a script 			*/
/*								*/
/* all *Native types have the same structure of the struct -	*/
/* we are just looking for the pointer, thus we can handle	*/
/* multi types here 						*/
/* sets a SFVec3f and SFColor in a script 			*/
/****************************************************************/

/* really do the individual set; used by script routing and EAI sending to a script */
void set_one_MultiElementType (uintptr_t tonode, uintptr_t tnfield, void *Data, unsigned dataLen ) {
	char scriptline[100];
	jsval retval;
	SFVec3fNative *_privPtr;

	JSContext *cx;
	JSObject *obj, *_sfvec3fObj;

	/* get context and global object for this script */
	cx = (JSContext *) ScriptControl[tonode].cx;
	obj = (JSObject *)ScriptControl[tonode].glob;

	/* set the time for this script */
	SET_JS_TICKTIME()

	/* get the variable name to hold the incoming value */
	sprintf (scriptline,"__eventIn_Value_%s", JSparamnames[tnfield].name);
	#ifdef SETFIELDVERBOSE 
	printf ("set_one_MultiElementType: script %d line %s\n",tonode, scriptline);
	#endif

	if (!JS_GetProperty(cx,obj,scriptline,&retval))
		printf ("JS_GetProperty failed in set_one_MultiElementType.\n");

	if (!JSVAL_IS_OBJECT(retval))
		printf ("set_one_MultiElementType - not an object\n");

	_sfvec3fObj = JSVAL_TO_OBJECT(retval);

	if ((_privPtr = (SFVec3fNative *)JS_GetPrivate(cx, _sfvec3fObj)) == NULL)
		printf("JS_GetPrivate failed set_one_MultiElementType.\n");

	/* copy over the data from the VRML side into the script variable. */
	memcpy ((void *) &_privPtr->v,Data, dataLen);

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

void setScriptMultiElementtype (uintptr_t num)
{
	uintptr_t tptr, fptr;
	unsigned int len;
	unsigned int to_counter;

	CRnodeStruct *fn, *to_ptr = NULL;

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
		cx = (JSContext *) ScriptControl[myObj->num].cx;
		obj = (JSObject *)ScriptControl[myObj->num].glob;

		fn += fptr;

		set_one_MultiElementType (myObj->num, tptr, (void*)fn, len);
	}
}

/* convert a number in memory to a printable type. Used to send back EVents, or replies to
   the Java client program. */

void EAI_Convert_mem_to_ASCII (int id, char *reptype, int type, char *memptr, char *buf) {

	double dval;
	float fl[4];
	float dl[4];
	float *fp;
	int *ip;
	int ival;
	unsigned int uval;
	int row;			/* MF* counter */
	struct Multi_String *MSptr;	/* MFString pointer */
	struct Multi_Node *MNptr;	/* MFNode pointer */
	struct Multi_Color *MCptr;	/* MFColor pointer */
	char *ptr;			/* used for building up return string */
	struct Uni_String *svptr;
	unsigned char *retSFString;

	int numPerRow;			/* 1, 2, 3 or 4 floats per row of this MF? */
	int i;

	/* used because of endian problems... */
	int *intptr;
	intptr = (int *) memptr;

	switch (type) {
		case FIELDTYPE_SFBool: 	{
			if (eaiverbose) { 
			printf ("EAI_SFBOOL - value %d; TRUE %d false %d\n",*intptr,TRUE,FALSE);
			}

			if (*intptr == 1) sprintf (buf,"%s\n%f\n%d\nTRUE",reptype,TickTime,id);
			else sprintf (buf,"%s\n%f\n%d\nFALSE",reptype,TickTime,id);
			break;
		}

		case FIELDTYPE_SFTime:	{
			if (eaiverbose) { 
			printf ("EAI_SFTIME\n");
			}
			memcpy(&dval,memptr,sizeof(double));
			sprintf (buf, "%s\n%f\n%d\n%lf",reptype,TickTime,id,dval);
			break;
		}

		case FIELDTYPE_SFInt32:	{
			if (eaiverbose) { 
			printf ("EAI_SFINT32\n");
			}
			memcpy(&ival,memptr,sizeof(int));
			sprintf (buf, "%s\n%f\n%d\n%d",reptype,TickTime,id,ival);
			break;
		}

		case FIELDTYPE_SFNode:	{
			if (eaiverbose) { 
			printf ("EAI_SFNODE\n");
			}
			memcpy(&uval,memptr,sizeof(unsigned int));
			sprintf (buf, "%s\n%f\n%d\n%u",reptype,TickTime,id,registerEAINodeForAccess(X3D_NODE(uval)));
			break;
		}

		case FIELDTYPE_SFFloat:	{
			if (eaiverbose) { 
			printf ("EAI_SFFLOAT\n");
			}

			memcpy(fl,memptr,sizeof(float));
			sprintf (buf, "%s\n%f\n%d\n%f",reptype,TickTime,id,fl[0]);
			break;
		}

		case FIELDTYPE_SFVec3f:
		case FIELDTYPE_SFColor:	{
			if (eaiverbose) { 
			printf ("EAI_SFCOLOR or EAI_SFVEC3F\n");
			}
			memcpy(fl,memptr,sizeof(float)*3);
			sprintf (buf, "%s\n%f\n%d\n%f %f %f",reptype,TickTime,id,fl[0],fl[1],fl[2]);
			break;
		}

		case FIELDTYPE_SFVec3d:	{
			if (eaiverbose) { 
			printf ("EAI_SFVEC3D\n");
			}
			memcpy(dl,memptr,sizeof(double)*3);
			sprintf (buf, "%s\n%f\n%d\n%lf %lf %lf",reptype,TickTime,id,dl[0],dl[1],dl[2]);
			break;
		}

		case FIELDTYPE_SFVec2f:	{
			if (eaiverbose) { 
			printf ("EAI_SFVEC2F\n");
			}
			memcpy(fl,memptr,sizeof(float)*2);
			sprintf (buf, "%s\n%f\n%d\n%f %f",reptype,TickTime,id,fl[0],fl[1]);
			break;
		}

		case FIELDTYPE_SFColorRGBA:
		case FIELDTYPE_SFRotation:	{
			if (eaiverbose) { 
			printf ("EAI_SFROTATION\n");
			}

			memcpy(fl,memptr,sizeof(float)*4);
			sprintf (buf, "%s\n%f\n%d\n%f %f %f %f",reptype,TickTime,id,fl[0],fl[1],fl[2],fl[3]);
			break;
		}

		case FIELDTYPE_SFImage:
		case FIELDTYPE_SFString:	{
			uintptr_t *xx;

			if (eaiverbose) { 
			printf ("EAI_SFSTRING\n");
			}

			/* get the pointer to the string, do this in a couple of steps... */
			svptr = (struct Uni_String *)memptr;
			xx= (uintptr_t *) memptr;
			svptr = (struct Uni_String *) *xx;

			retSFString = (unsigned char *)svptr->strptr; 
			sprintf (buf, "%s\n%f\n%d\n\"%s\"",reptype,TickTime,id,retSFString);
			break;
		}

		case FIELDTYPE_MFString:	{
			if (eaiverbose) { 
			printf ("EAI_MFSTRING\n");
			}

			/* make the Multi_String pointer */
			MSptr = (struct Multi_String *) memptr;

			/* printf ("EAI_MFString, there are %d strings\n",(*MSptr).n);*/
			sprintf (buf, "%s\n%f\n%d\n",reptype,TickTime,id);
			ptr = buf + strlen(buf);

			for (row=0; row<(*MSptr).n; row++) {
        	        	/* printf ("String %d is %s\n",row,(*MSptr).p[row]->strptr);*/
				if (strlen ((*MSptr).p[row]->strptr) == 0) {
					sprintf (ptr, "\"\" "); /* encode junk for Java side.*/
				} else {
					sprintf (ptr, "\"%s\" ",(*MSptr).p[row]->strptr);
				}
				/* printf ("buf now is %s\n",buf);*/
				ptr = buf + strlen (buf);
			}

			break;
		}

		case FIELDTYPE_MFNode: 	{
			MNptr = (struct Multi_Node *) memptr;

			if (eaiverbose) { 
			printf ("EAI_Convert_mem_to_ASCII: EAI_MFNode, there are %d nodes at %p\n",(*MNptr).n,memptr);
			}

			sprintf (buf, "%s\n%f\n%d\n",reptype,TickTime,id);
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
				printf ("EAI_MFColor, there are %d nodes at %p\n",(*MCptr).n,memptr);
			}

			sprintf (buf, "%s\n%f\n%d\n%d \n",reptype,TickTime,id,(*MCptr).n);
			ptr = buf + strlen(buf);

			ip = (int *) (*MCptr).p;
			for (row=0; row<(*MCptr).n; row++) {
				sprintf (ptr, "%d \n",*ip); 
				ip++;
				/* printf ("line %d is %s\n",row,ptr);  */
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
				printf ("EAI_MFColor, there are %d nodes at %p\n",(*MCptr).n,memptr);
			}

			sprintf (buf, "%s\n%f\n%d\n%d \n",reptype,TickTime,id,(*MCptr).n);
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
				/* printf ("line %d is %s\n",row,ptr); */
				ptr = buf + strlen (buf);
			}

			break;
		}
		default: {
			printf ("EAI, type %s not handled yet\n",stringFieldtypeType (type));
		}


/*XXX	case EAI_MFTIME:	{handleptr = &handleEAI_MFTIME_Listener;break;}*/
	}
}

