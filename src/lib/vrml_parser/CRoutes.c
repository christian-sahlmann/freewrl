/*
=INSERT_TEMPLATE_HERE=

$Id: CRoutes.c,v 1.72 2010/11/24 20:12:12 crc_canada Exp $

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
#include "CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/JScript.h"
#include "../world_script/CScripts.h"
#include "../world_script/fieldSet.h"
#include "CParseParser.h"
#include "CParseLexer.h"
#include "../world_script/jsUtils.h"
#include "../world_script/jsNative.h"
#include "../input/SensInterps.h"
#include "../scenegraph/Component_ProgrammableShaders.h"
#include "../input/EAIHeaders.h"
#include "../input/EAIHelpers.h"		/* for verify_Uni_String */

#include "CRoutes.h"

/* static void Multimemcpy (struct X3D_Node *toNode, struct X3D_Node *fromNode, void *tn, void *fn, size_t multitype); */
static void sendScriptEventIn(int num);
static struct X3D_Node *returnSpecificTypeNode(int requestedType, int *offsetOfsetValue, int *offsetOfvalueChanged);

/* we count times through the scenegraph; helps to break routing loops */
static int thisIntTimeStamp = 1;

/* defines for getting touched flags and exact Javascript pointers */

/****************************** ECMA types ******************************************/
/* where we have a Native structure to go along with it */
#define GETJSPTR_TYPE_A(thistype) \
			 case FIELDTYPE_##thistype:  {  \
				thistype##Native *ptr; \
				/* printf ("getting private data in GETJSPTR for %p \n",JSglobal_return_val); */ \
        			if ((ptr = (thistype##Native *)JS_GetPrivate(cx, (JSObject *)JSglobal_return_val)) == NULL) { \
                			printf( "JS_GetPrivate failed in get_valueChanged_flag\n"); \
                			return JS_FALSE; \
				} \
				/* if (ptr->valueChanged > 0) printf ("private is %d valueChanged %d\n",ptr,ptr->valueChanged); */ \
				JSSFpointer = (void *)ptr; /* save this for quick extraction of values */ \
				touched = ptr->valueChanged; \
				break; \
			} 

#define RESET_TOUCHED_TYPE_A(thistype) \
                case FIELDTYPE_##thistype: { \
                        ((thistype##Native *)JSSFpointer)->valueChanged = 0; \
                        break; \
                }       

#define GETJSPTR_TYPE_MF_A(thisMFtype,thisSFtype) \
	case FIELDTYPE_##thisMFtype: { \
		thisSFtype##Native *ptr; \
		jsval mainElement; \
		int len; \
		int i; \
		if (!JS_GetProperty(cx, (JSObject *)JSglobal_return_val, "length", &mainElement)) { \
			printf ("JS_GetProperty failed for \"length\" in get_valueChanged_flag\n"); \
			return FALSE; \
		} \
		len = JSVAL_TO_INT(mainElement); \
		/* go through each element of the main array. */ \
		for (i = 0; i < len; i++) { \
			if (!JS_GetElement(cx, (JSObject *)JSglobal_return_val, i, &mainElement)) { \
				printf ("JS_GetElement failed for %d in get_valueChanged_flag\n",i); \
				return FALSE; \
			} \
			if ((ptr = (thisSFtype##Native *)JS_GetPrivate(cx, (JSObject *)mainElement)) == NULL) { \
				printf( "JS_GetPrivate failed for obj in setField_javascriptEventOut.\n"); \
				return FALSE; \
			} \
			if (ptr->valueChanged > 0) touched = TRUE; /* did this element change? */ \
			/* printf ("touched flag for element %d is %d\n",i,ptr->touched); */ \
		} \
		break; \
	} 

#define RESET_TOUCHED_TYPE_MF_A(thisMFtype,thisSFtype) \
	case FIELDTYPE_##thisMFtype: { \
		thisSFtype##Native *ptr; \
		jsval mainElement; \
		int len; \
		int i; \
		JSContext *cx; \
		cx = ScriptControl[actualscript].cx; \
		if (!JS_GetProperty(cx, (JSObject *)JSglobal_return_val, "length", &mainElement)) { \
			printf ("JS_GetProperty failed for \"length\" in get_valueChanged_flag\n"); \
			break; \
		} \
		len = JSVAL_TO_INT(mainElement); \
		/* go through each element of the main array. */ \
		for (i = 0; i < len; i++) { \
			if (!JS_GetElement(cx, (JSObject *)JSglobal_return_val, i, &mainElement)) { \
				printf ("JS_GetElement failed for %d in get_valueChanged_flag\n",i); \
				break; \
			} \
			if ((ptr = (thisSFtype##Native *)JS_GetPrivate(cx, (JSObject *)mainElement)) == NULL) { \
				printf( "JS_GetPrivate failed for obj in setField_javascriptEventOut.\n"); \
				break; \
			} \
			ptr->valueChanged = 0; \
		} \
		break; \
	} 

/****************************** ECMA types ******************************************/

/* "Bool" might be already declared - we DO NOT want it to be declared as an "int" */
#define savedBool Bool
#ifdef Bool
#undef Bool
#endif

#define GET_ECMA_TOUCHED(thistype) \
	case FIELDTYPE_SF##thistype: {	\
				touched = findNameInECMATable( ScriptControl[actualscript].cx,fullname);\
				break;\
			}

#define GET_ECMA_MF_TOUCHED(thistype) \
	case FIELDTYPE_MF##thistype: {\
		jsval mainElement; \
		/* printf ("GET_ECMA_MF_TOUCHED called on %d\n",JSglobal_return_val);  */ \
		if (!JS_GetProperty(cx, (JSObject *)JSglobal_return_val, "MF_ECMA_has_changed", &mainElement)) { \
			printf ("JS_GetProperty failed for \"MF_ECMA_HAS_changed\" in get_valueChanged_flag\n"); \
		} /* else printf ("GET_ECMA_MF_TOUCHED MF_ECMA_has_changed is %d for %d %d\n",JSVAL_TO_INT(mainElement),cx,JSglobal_return_val); */  \
		touched = JSVAL_TO_INT(mainElement);\
		break; \
	}

#define RESET_ECMA_MF_TOUCHED(thistype) \
	case FIELDTYPE_##thistype: {\
		jsval myv = INT_TO_JSVAL(0); \
		/* printf ("RESET_ECMA_MF_TOUCHED called on %d ",JSglobal_return_val); */ \
        	if (!JS_SetProperty( ScriptControl[actualscript].cx, (JSObject *)JSglobal_return_val, "MF_ECMA_has_changed", &myv)) { \
        		printf( "JS_SetProperty failed for \"MF_ECMA_has_changed\" in RESET_ECMA_MF_TOUCHED.\n"); \
        	}\
                /* if (!JS_GetProperty( ScriptControl[actualscript].cx, (JSObject *)JSglobal_return_val, "MF_ECMA_has_changed", &mainElement)) { \
                        printf ("JS_GetProperty failed for \"MF_ECMA_HAS_changed\" in get_valueChanged_flag\n"); \
		} \
                printf ("and MF_ECMA_has_changed is %d\n",JSVAL_TO_INT(mainElement)); */\
	break; \
	}

#define RESET_TOUCHED_TYPE_ECMA(thistype) \
			case FIELDTYPE_##thistype: { \
				resetNameInECMATable( ScriptControl[actualscript].cx,JSparamnames[fptr].name); \
				break; \
			}
/* in case Bool was defined above, restore the value */
#define Bool savedBool


void setMFElementtype (int num);

/*****************************************
C Routing Methodology:

Different nodes produce eventins/eventouts...

	EventOuts only:
		MovieTexture
		AudioClip
		TimeSensor
		TouchSensor
		PlaneSensor
		SphereSensor
		CylinderSensor
		VisibilitySensor
		ProximitySensor
		GeoProximitySensor

	EventIn/EventOuts:
		ScalarInterpolator
		OrientationInterpolator
		ColorInterpolator
		PositionInterpolator
		GeoPositionInterpolator
		NormalInterpolator
		CoordinateInterpolator
		Fog
		Background
		Viewpoint
		NavigationInfo
		Collision

	EventIns only:
		Almost everything else...


	Nodes with ClockTicks:
		MovieTexture, AudioClip, TimeSensor,
		ProximitySensor, Collision, ...?

	Nodes that have the EventsProcessed method:
		ScalarInterpolator, OrientationInterpolator,
		ColorInterpolator, PositionInterpolator,
		NormalInterpolator,  (should be all the interpolators)
		.... ??




	--------------------------------------------------------------------------
	C Routes are stored in a table with the following entries:
		Fromnode 	- the node that created an event address
		actual ptr	- pointer to the exact field within the address
		Tonode		- destination node address
		actual ptr	- pointer to the exact field within the address
		active		- True of False for each iteration
		length		- data field length
		interpptr	- pointer to an interpolator node, if this is one



	SCRIPTS handled like this:

		1) a call is made to        CRoutes_js_new (num,cx,glob,brow);
		   with the script number (0 on up), script context, script globals,
		   and browser data.

		2) Initialize called;


		3) scripts that have eventIns have the values copied over and
		   sent to the script by the routine "sendScriptEventIn".

		4) scripts that have eventOuts have the eventOut values copied over
		   and acted upon by the routine "gatherScriptEventOuts".


******************************************/
/* Routing table */
struct CRStruct *CRoutes;
static int CRoutes_Initiated = FALSE;
int CRoutes_Count;
int CRoutes_MAX;

/* Structure table */
struct CRscriptStruct *ScriptControl = 0; 	/* global objects and contexts for each script */
int *scr_act = 0;				/* this script has been sent an eventIn */
int max_script_found = -1;			/* the maximum script number found */
int max_script_found_and_initialized = -1;	/* the maximum script number found */

/* EAI needs the extra parameter, so we put it globally when a RegisteredListener is clicked. */
int CRoutesExtra = 0;

/* global return value for getting the value of a variable within Javascript */
jsval JSglobal_return_val;
void *JSSFpointer;

/* ClockTick structure for processing all of the initevents - eg, TimeSensors */
struct FirstStruct {
	void *	tonode;
	void (*interpptr)(void *);
};

/* ClockTick structure and counter */
struct FirstStruct *ClockEvents = NULL;
int num_ClockEvents = 0;


/* We buffer route registrations, JUST in case a registration comes from executing a route; eg,
from within a Javascript function invocation createVrmlFromURL call that was invoked by a routing
call */

struct CR_RegStruct {
		int adrem;
		struct X3D_Node *from;
		int fromoffset;
		struct X3D_Node *to;
		int toOfs;
		int fieldType;
		void *intptr;
		int scrdir;
		int extra; };

static struct Vector* routesToRegister = NULL;


/* if we get mark_events sent, before routing is established, save them and use them
   as soon as routing is here */
#define POSSIBLEINITIALROUTES 1000
static int initialEventBeforeRoutesCount = 0;
static int preRouteTableSize = 0;
struct initialRouteStruct {
	struct X3D_Node *from;
	size_t totalptr;
};
static struct initialRouteStruct *preEvents = NULL;
pthread_mutex_t  preRouteLock = PTHREAD_MUTEX_INITIALIZER;
#define LOCK_PREROUTETABLE                pthread_mutex_lock(&preRouteLock);
#define UNLOCK_PREROUTETABLE              pthread_mutex_unlock(&preRouteLock);

pthread_mutex_t  insertRouteLock = PTHREAD_MUTEX_INITIALIZER;
#define MUTEX_LOCK_ROUTING_UPDATES                pthread_mutex_lock(&insertRouteLock);
#define MUTEX_FREE_LOCK_ROUTING_UPDATES		pthread_mutex_unlock(&insertRouteLock);


/* a Script (JavaScript or CLASS) has given us an event, tell the system of this */
/* tell this node now needs to redraw  - but only if it is not a script to
   script route - see CRoutes_Register here, and check for the MALLOC in that code.
   You should see that the offset is zero, while in real nodes, the offset of user
   accessible fields is NEVER zero - check out CFuncs/Structs.h and look at any of
   the node types, eg, X3D_IndexedFaceSet  the first offset is for X3D_Virt :=)
*/

void markScriptResults(struct X3D_Node * tn, int tptr, int route, void * tonode) {
	if (tptr != 0) {
		#ifdef CRVERBOSE
		printf ("markScriptResults: can update this node %p %d\n",tn,tptr); 
		#endif
		update_node(tn);
	#ifdef CRVERBOSE
	} else {
		printf ("markScriptResults: skipping this node %p %d flag %d\n",tn,tptr,CRoutes[route].direction_flag); 
	#endif
	}

	MARK_EVENT (CRoutes[route].routeFromNode,CRoutes[route].fnptr);

	/* run an interpolator, if one is attached. */
	if (CRoutes[route].interpptr != 0) {
		/* this is an interpolator, call it */
		CRoutesExtra = CRoutes[route].extra; /* in case the interp requires it... */
		#ifdef CRVERBOSE 
		printf ("script propagate_events. index %d is an interpolator\n",route);
		#endif
		CRoutes[route].interpptr(tonode);
	}
}


/********************************************************************************/
/*									    	*/
/* get_valueChanged_flag - see if this variable (can be a sub-field; see tests   	*/
/* 8.wrl for the DEF PI PositionInterpolator). return true if variable is   	*/
/* touched, and pointer to touched value is in global variable              	*/
/* JSglobal_return_val, AND possibly:						*/
/*	void *JSSFpointer for SF non-ECMA nodes.				*/
/* 										*/
/* the way touched, and, the actual values work is as follows:			*/
/*										*/
/* keep track of the name in a table, and set valueChanged flag.		*/
/* look around the function setECMANative to see how this is done.		*/
/* FIELDTYPE_SFInt32								*/
/* FIELDTYPE_SFBool								*/
/* FIELDTYPE_SFFloat								*/
/* FIELDTYPE_SFTime								*/
/* FIELDTYPE_SFString								*/
/*										*/
/* check the "touched" flag for non-zero in the private area:			*/
/* FIELDTYPE_SFRotation								*/
/* FIELDTYPE_SFNode								*/
/* FIELDTYPE_SFVec2f								*/
/* FIELDTYPE_SFVec3f								*/
/* FIELDTYPE_SFImage								*/
/* FIELDTYPE_SFColor								*/
/* FIELDTYPE_SFColorRGBA							*/
/*										*/
/* go through all elements, and find if at least one SF has been touched:	*/
/* FIELDTYPE_MFRotation								*/
/* FIELDTYPE_MFNode								*/
/* FIELDTYPE_MFVec2f								*/
/* FIELDTYPE_MFVec3f								*/
/* FIELDTYPE_MFColor								*/
/* FIELDTYPE_MFColorRGBA							*/


/* has a flag called "MF_ECMA_has_changed" that is used here 			*/
/* FIELDTYPE_MFFloat	*/
/* FIELDTYPE_MFBool	*/
/* FIELDTYPE_MFInt32	*/
/* FIELDTYPE_MFTime	*/
/* FIELDTYPE_MFString	*/
/*                                                                          */
/****************************************************************************/

int get_valueChanged_flag (int fptr, int actualscript) {
	JSContext *cx;
	JSObject *interpobj;
	char *fullname;
	int touched;

	touched = FALSE;
	interpobj = ScriptControl[actualscript].glob;
	cx =  ScriptControl[actualscript].cx;
	fullname = JSparamnames[fptr].name;

	#ifdef CRVERBOSE
	printf ("\ngetting property for fullname %s, cx %p, interpobj %d script %d, fptr %d (%s:%s)\n",
		fullname,cx,interpobj,actualscript, fptr,
		JSparamnames[fptr].name, FIELDTYPES[JSparamnames[fptr].type]);
	#endif

	if (!JS_GetProperty(cx,  interpobj ,fullname,&JSglobal_return_val)) {
               	printf ("cant get property for %s\n",fullname);
		return FALSE;
        } else {
		#ifdef CRVERBOSE
		printf ("so, property is %d (%p)\n",JSglobal_return_val,JSglobal_return_val);
		printf("get_valueChanged_flag: node type: %s name %s\n",FIELDTYPES[JSparamnames[fptr].type],JSparamnames[fptr].name);
		#endif

		switch (JSparamnames[fptr].type) {
			GETJSPTR_TYPE_A(SFRotation)
			GETJSPTR_TYPE_A(SFNode)
			GETJSPTR_TYPE_A(SFVec2f)
			/* GETJSPTR_TYPE_A(SFVec2d) */
			GETJSPTR_TYPE_A(SFVec3f)
			GETJSPTR_TYPE_A(SFVec3d)
			GETJSPTR_TYPE_A(SFVec4f)
			GETJSPTR_TYPE_A(SFVec4d)
			GETJSPTR_TYPE_A(SFImage)
			GETJSPTR_TYPE_A(SFColor)
			GETJSPTR_TYPE_A(SFColorRGBA)

			GETJSPTR_TYPE_MF_A(MFRotation,SFRotation)
			GETJSPTR_TYPE_MF_A(MFNode,SFNode)
			GETJSPTR_TYPE_MF_A(MFVec2f,SFVec2f)
			GETJSPTR_TYPE_MF_A(MFVec3f,SFVec3f)
			GETJSPTR_TYPE_MF_A(MFVec4f,SFVec4f)
			GETJSPTR_TYPE_MF_A(MFVec4d,SFVec4d)
			/* GETJSPTR_TYPE_MF_A(MFImage,SFImage)  */
			GETJSPTR_TYPE_MF_A(MFColor,SFColor)
			GETJSPTR_TYPE_MF_A(MFColorRGBA,SFColorRGBA)
			
			GET_ECMA_MF_TOUCHED(Int32)
			GET_ECMA_MF_TOUCHED(Bool)
			GET_ECMA_MF_TOUCHED(Time)
			GET_ECMA_MF_TOUCHED(Float)
			GET_ECMA_MF_TOUCHED(String)

			GET_ECMA_TOUCHED(Int32) 
			GET_ECMA_TOUCHED(Bool) 
			GET_ECMA_TOUCHED(Float)
			GET_ECMA_TOUCHED(Time)
			GET_ECMA_TOUCHED(String)
			
			default: {printf ("not handled yet in get_valueChanged_flag %s\n",FIELDTYPES[JSparamnames[fptr].type]);
			}
		}
	}

#ifdef CHECKER
	if (JSparamnames[fptr].type == FIELDTYPE_MFString) {
		int len; int i;
                jsval mainElement; 
                int len; 

		unsigned CRCCheck = 0;
                cx = ScriptControl[actualscript].cx; 
                if (!JS_GetProperty(cx, JSglobal_return_val, "length", &mainElement)) { 
                        printf ("JS_GetProperty failed for length_flag\n"); 
                } 
                len = JSVAL_TO_INT(mainElement); 
                /* go through each element of the main array. */ 
                for (i = 0; i < len; i++) { 
                        if (!JS_GetElement(cx, JSglobal_return_val, i, &mainElement)) { 
                                printf ("JS_GetElement failed for %d in get_valueChanged_flag\n",i); 
                                break; 
                        } 
		CRCCheck += (unsigned) mainElement;

/*
                if (JSVAL_IS_OBJECT(mainElement)) printf ("sc, element %d is an OBJECT\n",i);
                if (JSVAL_IS_STRING(mainElement)) printf ("sc, element %d is an STRING\n",i);
                if (JSVAL_IS_NUMBER(mainElement)) printf ("sc, element %d is an NUMBER\n",i);
                if (JSVAL_IS_DOUBLE(mainElement)) printf ("sc, element %d is an DOUBLE\n",i);
                if (JSVAL_IS_INT(mainElement)) printf ("sc, element %d is an INT\n",i);
*/

                } 
		printf ("CRCcheck %u\n",CRCCheck);
	}
#endif



	return touched;
}

/****************************************************************/
/* Add or Remove a series of children				*/
/*								*/
/* pass in a pointer to a node, (see Structs.h for defn)	*/
/* a pointer to the actual field in that node,			*/
/*	a list of node pointers, in memory,			*/
/*	the length of this list, (ptr size, not bytes)		*/
/*	and a flag for add (1), remove (2) or replace (0) 	*/
/*								*/
/****************************************************************/


void AddRemoveChildren (
		struct X3D_Node *parent,
		struct Multi_Node *tn,
		struct X3D_Node * *nodelist,
		int len,
		int ar,
		char *file,
		int line) {
	int oldlen;
	void *newmal;
	struct X3D_Node * *remchild;
	struct X3D_Node * *remptr;
	struct X3D_Node * *tmpptr;
	int done;

	int counter, c2;
	#ifdef CRVERBOSE
	
	printf ("\n start of AddRemoveChildren; parent is a %s at %p\n",stringNodeType(parent->_nodeType),parent);
	printf ("AddRemove Children parent %p tn %p, len %d ar %d\n",parent,tn,len,ar);
	printf ("called at %s:%d\n",file,line);
	#endif

	/* if no elements, just return */
	if (len <=0) return;
	if ((parent==0) || (tn == 0)) {
		printf ("Freewrl: AddRemoveChildren, parent and/or field NULL\n");
		return;
	}

	oldlen = tn->n;
	#ifdef CRVERBOSE
	printf ("AddRemoveChildren, len %d, oldlen %d ar %d\n",len, oldlen, ar);
	#endif

	/* to do a "set_children", we remove the children, then do an add */
	if (ar == 0) {
		#ifdef CRVERBOSE
		printf ("we have to perform a \"set_children\" on this field\n");
		# endif

		/* make it so that we have 0 children */
		tn->n=0; 

		/* go through the children, and tell them that they are no longer wanted here */
		for (counter=0; counter < oldlen; counter ++) remove_parent(tn->p[counter],parent);

		/* now, totally free the old children array */
		if (oldlen > 0) {FREE_IF_NZ(tn->p);}

		/* now, make this into an addChildren */
		oldlen = 0;
		ar = 1;

	}


	if (ar == 1) {
		/* addChildren - now we know how many SFNodes are in this MFNode, lets MALLOC and add */

		/* first, set children to 0, in case render thread comes through here */
		tn->n = 0;

		newmal = MALLOC ((oldlen+len)*sizeof(struct X3D_Node *));

		/* copy the old stuff over */
		if (oldlen > 0) memcpy (newmal,tn->p,oldlen*sizeof(void *));

		/* set up the C structures for this new MFNode addition */
		FREE_IF_NZ (tn->p);
		tn->n = oldlen;
		tn->p = newmal;

		/* copy the new stuff over - note, tmpptr changes what it points to */
		tmpptr  = offsetPointer_deref(struct X3D_Node * *,newmal, sizeof(struct X3D_Node *) * oldlen);

		/* tell each node in the nodelist that it has a new parent */
		for (counter = 0; counter < len; counter++) {
			#ifdef CRVERBOSE
			printf ("AddRemove, count %d of %d, node %p parent %p\n",counter, len,nodelist[counter],parent);
			#endif
			if (nodelist[counter] != NULL) {
				*tmpptr = nodelist[counter];
				tmpptr ++;
				tn->n++;
				ADD_PARENT((void *)nodelist[counter],(void *)parent);
			} else {
				/* gosh, we are asking to add a NULL node pointer, lets just skip it... */
				printf ("AddRemoveChildren, Add, but new node is null; ignoring...\n");
			}
		}
	} else {
		int finalLength;
		int num_removed;

		/* this is a removeChildren */

		/* go through the original array, and "zero" out children that match one of
		   the parameters */

		num_removed = 0;
		remchild = nodelist;
		/* printf ("removing, len %d, tn->n %d\n",len,tn->n);  */
		for (c2 = 0; c2 < len; c2++) {
			remptr = (struct X3D_Node * *) tn->p;
			done = FALSE;

			for (counter = 0; counter < tn->n; counter ++) {
				#ifdef CRVERBOSE
				printf ("remove, comparing %p with %p\n",*remptr, *remchild); 
				#endif
				if ((*remptr == *remchild) && (!done)) {
					#ifdef CRVERBOSE
					printf ("Found it! removing this child from this parent\n");
					#endif

					remove_parent(X3D_NODE(*remchild),parent);
					*remptr = NULL;  /* "0" can not be a valid memory address */
					num_removed ++;
					done = TRUE; /* remove this child ONLY ONCE - in case it has been added
							more than once. */
				}
				remptr ++;
			}
			remchild ++;
		}


		finalLength = oldlen - num_removed;
		#ifdef CRVERBOSE
		printf ("final length is %d, we have %d in original array\n", finalLength, tn->n);
		remptr = (struct X3D_Node * *) tn->p;
		printf ("so, the original array, with zeroed elements is: \n");
		for (counter = 0; counter < tn->n; counter ++) {
			printf ("count %d of %d is %p\n",counter,tn->n, *remptr); 
			remptr ++;
		}
		#endif


		if (num_removed > 0) {
			if (finalLength > 0) {
				newmal = MALLOC (finalLength*sizeof(struct X3D_Node * *));
				bzero (newmal, (size_t)(finalLength*sizeof(struct X3D_Node * *)));
				tmpptr = (struct X3D_Node * *) newmal;
				remptr = (struct X3D_Node * *) tn->p;

				/* go through and copy over anything that is not zero */
				for (counter = 0; counter < tn->n; counter ++) {
					/* printf ("count %d is %p\n",counter, *remptr); */
					if (*remptr != NULL) {
						*tmpptr = *remptr;
						/* printf ("now, tmpptr is %p\n",*tmpptr);  */
						tmpptr ++;
					}
					remptr ++;
				}
				/* printf ("done loops, now make data active \n"); */

				/* now, do the move of data */
				tn->n = 0;
				FREE_IF_NZ (tn->p);
				tn->p = newmal;
				tn->n = finalLength;
			} else {
				tn->n = 0;
				FREE_IF_NZ(tn->p);
			}

			#ifdef CRVERBOSE
			printf ("so, we have a final array length of %d\n",tn->n);
			for (counter =0; counter <tn->n; counter ++) {
				printf ("    element %d is %p\n",counter,tn->p[counter]);
			}
			#endif

		}

	}

	update_node(parent);
}



/* These events must be run first during the event loop, as they start an event cascade.
   Regsister them with add_first, then call them during the event loop with do_first.    */

void kill_clockEvents() { 
	/* printf ("killing clckevents - was %d\n",num_ClockEvents); */
	num_ClockEvents = 0;
}

void add_first(struct X3D_Node * node) {
	void (*myp)(void *);
	int clocktype;
	int count;
	
	if (node == 0) {
		printf ("error in add_first; somehow the node datastructure is zero \n");
		return;
	}

	clocktype = node->_nodeType;
	/* printf ("add_first for %s\n",stringNodeType(clocktype)); */

	if (NODE_TimeSensor == clocktype) { myp =  do_TimeSensorTick;
	} else if (NODE_ProximitySensor == clocktype) { myp = do_ProximitySensorTick;
	} else if (NODE_Collision == clocktype) { myp = do_CollisionTick;
	} else if (NODE_MovieTexture == clocktype) { myp = do_MovieTextureTick;
	} else if (NODE_AudioClip == clocktype) { myp = do_AudioTick;
	} else if (NODE_VisibilitySensor == clocktype) { myp = do_VisibilitySensorTick;
	} else if (NODE_MovieTexture == clocktype) { myp = do_MovieTextureTick;
	} else if (NODE_GeoProximitySensor == clocktype) { myp = do_GeoProximitySensorTick;

	} else {
		/* printf ("this is not a type we need to add_first for %s\n",stringNodeType(clocktype)); */
		return;
	}

	ClockEvents = (struct FirstStruct *)REALLOC(ClockEvents,sizeof (struct FirstStruct) * (num_ClockEvents+1));
	if (ClockEvents == 0) {
		printf ("can not allocate memory for add_first call\n");
		num_ClockEvents = 0;
	}

	/* does this event exist? */
	for (count=0; count <num_ClockEvents; count ++) {
		if (ClockEvents[count].tonode == node) {
			/* printf ("add_first, already have %d\n",node); */
			return;
		}	
	}


	/* now, put the function pointer and data pointer into the structure entry */
	ClockEvents[num_ClockEvents].interpptr = myp;
	ClockEvents[num_ClockEvents].tonode = node;

	num_ClockEvents++;
}



/*******************************************************************

CRoutes_js_new;

Register a new script for future routing

********************************************************************/

void CRoutes_js_new (int num, int scriptType) {
	/* record whether this is a javascript, class invocation, ... */
	ScriptControl[num].thisScriptType = scriptType;

	/* compare with a intptr_t, because we need to compare to -1 */
	if (num > max_script_found) max_script_found = num;
}


/********************************************************************

JSparamIndex.

stores ascii names with types (see code for type equivalences).

********************************************************************/

int JSparamIndex (const char *name, const char *type) {
	size_t len;
	int ty;
	int ctr;

	#ifdef CRVERBOSE
	printf ("start of JSparamIndex, name %s, type %s\n",name,type);
	printf ("start of JSparamIndex, lengths name %d, type %d\n",
			strlen(name),strlen(type)); 
	#endif

	ty = findFieldInFIELDTYPES(type);

	#ifdef CRVERBOSE
	printf ("JSparamIndex, type %d, %s\n",ty,type); 
	#endif

	len = strlen(name);

	/* is this a duplicate name and type? types have to be same,
	   name lengths have to be the same, and the strings have to be the same.
	*/
	for (ctr=0; ctr<=jsnameindex; ctr++) {
		if (ty==JSparamnames[ctr].type) {
			if ((strlen(JSparamnames[ctr].name) == len) &&
				(strncmp(name,JSparamnames[ctr].name,len)==0)) {
				#ifdef CRVERBOSE
				printf ("JSparamIndex, duplicate, returning %d\n",ctr);
				#endif

				return ctr;
			}
		}
	}

	/* nope, not duplicate */

	jsnameindex ++;

	/* ok, we got a name and a type */
	if (jsnameindex >= MAXJSparamNames) {
		/* oooh! not enough room at the table */
		MAXJSparamNames += 100; /* arbitrary number */
		JSparamnames = (struct CRjsnameStruct*)REALLOC (JSparamnames, sizeof(*JSparamnames) * MAXJSparamNames);
	}

	if (len > MAXJSVARIABLELENGTH-2) len = MAXJSVARIABLELENGTH-2;	/* concatenate names to this length */
	strncpy (JSparamnames[jsnameindex].name,name,len);
	JSparamnames[jsnameindex].name[len] = 0; /* make sure terminated */
	JSparamnames[jsnameindex].type = ty;
	JSparamnames[jsnameindex].eventInFunction = NULL;
	#ifdef CRVERBOSE
	printf ("JSparamIndex, returning %d\n",jsnameindex); 
	#endif

	return jsnameindex;
}

/********************************************************************

Register a route, but with fewer and more expressive parameters than
CRoutes_Register.  Currently a wrapper around that other function.

********************************************************************/

void CRoutes_RegisterSimple(
	struct X3D_Node* from, int fromOfs,
	struct X3D_Node* to, int toOfs,
	int type)  {
	/* printf ("CRoutes_RegisterSimple, registering a route of %s\n",stringFieldtypeType(type)); */

 	/* 10+1+3+1=15:  Number <5000000000, :, number <999, \0 */
 	void* interpolatorPointer;
 	int extraData = 0;
	int dir = 0;
	
	/* get direction flags here */
	switch (from->_nodeType) {
		case NODE_Script:
		case NODE_ComposedShader:
		case NODE_PackagedShader:
		case NODE_ShaderProgram: 
			dir  = dir | FROM_SCRIPT; break;
		default: {}
	}
	switch (to->_nodeType) {
		case NODE_Script:
		case NODE_ComposedShader:
		case NODE_PackagedShader:
		case NODE_ShaderProgram: 
			dir  = dir | TO_SCRIPT; break;
		default: {}
	}

	/* check to ensure that we are not doing with a StaticGroup here */
	if (dir!=SCRIPT_TO_SCRIPT && dir!=TO_SCRIPT) {
		/* printf ("we are NOT sending to a script, checking for StaticGroup\n"); */
		if (to->_nodeType == NODE_StaticGroup) {
			ConsoleMessage ("ROUTE to a StaticGroup not allowed");
			return;
		}
	}
	/* check to ensure that we are not doing with a StaticGroup here */
	if (dir!=SCRIPT_TO_SCRIPT && dir!=FROM_SCRIPT) {
		/* printf ("we are NOT sending from a script, checking for StaticGroup\n"); */
		if (from->_nodeType == NODE_StaticGroup) {
			ConsoleMessage ("ROUTE from a StaticGroup not allowed");
			return;
		}
	}

	/* When routing to a script, to is not a node pointer! */
	if(dir!=SCRIPT_TO_SCRIPT && dir!=TO_SCRIPT)
		interpolatorPointer=returnInterpolatorPointer(stringNodeType(to->_nodeType));
	else
		interpolatorPointer=NULL;
	CRoutes_Register(1, from, fromOfs, to,toOfs, type, interpolatorPointer, dir, extraData);
}
 

/********************************************************************

Remove a route, but with fewer and more expressive parameters than
CRoutes_Register.  Currently a wrapper around that other function.

********************************************************************/

void CRoutes_RemoveSimple(
	struct X3D_Node* from, int fromOfs,
	struct X3D_Node* to, int toOfs,
	int type) {

 	/* 10+1+3+1=15:  Number <5000000000, :, number <999, \0 */
 	void* interpolatorPointer;
 	int extraData = 0;

  	interpolatorPointer=returnInterpolatorPointer(stringNodeType(to->_nodeType));

 	CRoutes_Register(0, from, fromOfs, to, toOfs, type, 
  		interpolatorPointer, 0, extraData);
}

/********************************************************************

CRoutes_Register.

Register a route in the routing table.

********************************************************************/

	
void CRoutes_Register(
		int adrem,
		struct X3D_Node *from,
		int fromoffset,
		struct X3D_Node *to,
		int toOfs,
		int type,
		void *intptr,
		int scrdir,
		int extra) {

	struct CR_RegStruct *newEntry;


/* Script to Script - we actually put a small node in, and route to/from this node so routing is a 2 step process */
	if (scrdir == SCRIPT_TO_SCRIPT) {
		struct X3D_Node *chptr;
		int set, changed;

		/* initialize stuff for compile checks */
		set = 0; changed = 0;

		chptr = returnSpecificTypeNode(type, &set, &changed);
		CRoutes_Register (adrem, from, fromoffset,chptr,set, type, 0, FROM_SCRIPT, extra);
		CRoutes_Register (adrem, chptr, changed, to, toOfs, type, 0, TO_SCRIPT, extra);
		return;
	}

	MUTEX_LOCK_ROUTING_UPDATES

	if (routesToRegister == NULL) {
		routesToRegister = newVector(struct CR_RegStruct *, 16);
	}


	newEntry = MALLOC(sizeof (struct CR_RegStruct));
	newEntry->adrem = adrem;
	newEntry->from = from;
	newEntry->fromoffset = fromoffset;
	newEntry->to = to;
	newEntry->toOfs = toOfs;
	newEntry->fieldType = type;
	newEntry->intptr = intptr;
	newEntry->scrdir = scrdir;
	newEntry->extra = extra;
	vector_pushBack(struct CR_RegStruct *, routesToRegister, newEntry);

	MUTEX_FREE_LOCK_ROUTING_UPDATES

}

static void actually_do_CRoutes_Register() {
	int insert_here, shifter;
	CRnodeStruct *to_ptr = NULL;
	size_t toof;		/* used to help determine duplicate routes */
	struct X3D_Node *toN;
	indexT ind;

	if (routesToRegister == NULL) return; /* should never get here, but... */

#ifdef CRVERBOSE
	printf ("actually_do_CRoutes_Register, vector size %d\n",vector_size(routesToRegister));
#endif

	for (ind=0; ind<vector_size(routesToRegister); ind++ ) {
		struct CR_RegStruct *newEntry;

		newEntry = vector_get(struct CR_RegStruct *, routesToRegister, ind);

#ifdef CRVERBOSE  
		printf ("CRoutes_Register adrem %d from %u ",newEntry->adrem, newEntry->from);
		if (newEntry->from > JSMaxScript) printf ("(%s) ",stringNodeType(X3D_NODE(newEntry->from->_nodeType)));

		printf ("off %u to %u intptr %p\n",
				newEntry->fromoffset, newEntry->to, newEntry->intptr);
		printf ("CRoutes_Register, CRoutes_Count is %d\n",CRoutes_Count);
#endif

		/* first time through, create minimum and maximum for insertion sorts */
		if (!CRoutes_Initiated) {
			/* allocate the CRoutes structure */
			CRoutes_MAX = 25; /* arbitrary number; max 25 routes to start off with */
			CRoutes = (struct CRStruct *)MALLOC (sizeof (*CRoutes) * CRoutes_MAX);
	
			CRoutes[0].routeFromNode = X3D_NODE(0);
			CRoutes[0].fnptr = 0;
			CRoutes[0].tonode_count = 0;
			CRoutes[0].tonodes = NULL;
			CRoutes[0].isActive = FALSE;
			CRoutes[0].interpptr = 0;
			CRoutes[0].intTimeStamp = 0;
			CRoutes[1].routeFromNode = X3D_NODE(-1);
			CRoutes[1].fnptr = 0x8FFFFFFF;
			CRoutes[1].tonode_count = 0;
			CRoutes[1].tonodes = NULL;
			CRoutes[1].isActive = FALSE;
			CRoutes[1].interpptr = 0;
			CRoutes[1].intTimeStamp = 0;
			CRoutes_Count = 2;
			CRoutes_Initiated = TRUE;
		}
	
		insert_here = 1;
	
		/* go through the routing list, finding where to put it */
		while (newEntry->from > CRoutes[insert_here].routeFromNode) {
			#ifdef CRVERBOSE 
				printf ("comparing %u to %u\n",newEntry->from, CRoutes[insert_here].routeFromNode);
			#endif
			insert_here++;
		}
	
		/* hmmm - do we have a route from this node already? If so, go
		   through and put the offsets in order */
		while ((newEntry->from == CRoutes[insert_here].routeFromNode) &&
			(newEntry->fromoffset > CRoutes[insert_here].fnptr)) {
			#ifdef CRVERBOSE 
				printf ("same routeFromNode, different offset\n");
			#endif
			insert_here++;
		}
	
	
		/* Quick check to verify that we don't have a duplicate route here
		   OR to delete a route... */
	
		#ifdef CRVERBOSE
		printf ("ok, CRoutes_Register - is this a duplicate? comparing from (%d %d), fnptr (%d %d) intptr (%d %d) and tonodes %d\n",
			CRoutes[insert_here].routeFromNode, newEntry->from,
			CRoutes[insert_here].fnptr, newEntry->fromoffset,
			CRoutes[insert_here].interpptr, newEntry->intptr,
			CRoutes[insert_here].tonodes);
		#endif
	
		if ((CRoutes[insert_here].routeFromNode==newEntry->from) &&
			(CRoutes[insert_here].fnptr==newEntry->fromoffset) &&
			(CRoutes[insert_here].interpptr==newEntry->intptr) &&
			(CRoutes[insert_here].tonodes!=0)) {
	
			/* possible duplicate route */
			toN = newEntry->to; 
			toof = newEntry->toOfs;

			if ((toN == (CRoutes[insert_here].tonodes)->routeToNode) &&
				(toof == (CRoutes[insert_here].tonodes)->foffset)) {
				/* this IS a duplicate, now, what to do? */
	
				#ifdef CRVERBOSE
				printf ("duplicate route; maybe this is a remove? \n");
				#endif
	
				/* is this an add? */
				if (newEntry->adrem == 1) {
					#ifdef CRVERBOSE
						printf ("definite duplicate, returning\n");
					#endif
					continue; //return;
				} else {
					/* this is a remove */
	
					for (shifter = insert_here; shifter < CRoutes_Count; shifter++) {
					#ifdef CRVERBOSE 
						printf ("copying from %d to %d\n",shifter, shifter-1);
					#endif
						memcpy ((void *)&CRoutes[shifter],
							(void *)&CRoutes[shifter+1],
							sizeof (struct CRStruct));
					}
					CRoutes_Count --;
					#ifdef CRVERBOSE 
						printf ("routing table now %d\n",CRoutes_Count);
						for (shifter = 0; shifter < CRoutes_Count; shifter ++) {
							printf ("%d: %u %u %u\n",shifter, CRoutes[shifter].routeFromNode, CRoutes[shifter].fnptr,
								CRoutes[shifter].interpptr);
						}
					#endif
	
					/* return; */
				}
			}
		}
	
		/* this is an Add; removes should be handled above. */
		if (newEntry->adrem == 1)  {
			#ifdef CRVERBOSE 
				printf ("CRoutes, inserting at %d\n",insert_here);
			#endif
		
			/* create the space for this entry. */
			for (shifter = CRoutes_Count; shifter > insert_here; shifter--) {
				memcpy ((void *)&CRoutes[shifter], (void *)&CRoutes[shifter-1],sizeof(struct CRStruct));
				#ifdef CRVERBOSE 
					printf ("Copying from index %d to index %d\n",shifter, shifter-1);
				#endif
			}
		
		
			/* and put it in */
			CRoutes[insert_here].routeFromNode = newEntry->from;
			CRoutes[insert_here].fnptr = newEntry->fromoffset;
			CRoutes[insert_here].isActive = FALSE;
			CRoutes[insert_here].tonode_count = 0;
			CRoutes[insert_here].tonodes = NULL;
			CRoutes[insert_here].len = returnRoutingElementLength(newEntry->fieldType);
			CRoutes[insert_here].interpptr = (void (*)(void*))newEntry->intptr;
			CRoutes[insert_here].direction_flag = newEntry->scrdir;
			CRoutes[insert_here].extra = newEntry->extra;
			CRoutes[insert_here].intTimeStamp = 0;
		
			if ((CRoutes[insert_here].tonodes =
				 (CRnodeStruct *) MALLOC(sizeof(CRnodeStruct))) == NULL) {
				fprintf(stderr, "CRoutes_Register: calloc failed to allocate memory.\n");
			} else {
				CRoutes[insert_here].tonode_count = 1;
				/* printf ("inserting route, to %u, offset %d\n",newEntry->to, newEntry->toOfs); */
		
				to_ptr = &(CRoutes[insert_here].tonodes[0]);
				to_ptr->routeToNode = newEntry->to;
				to_ptr->foffset = newEntry->toOfs;
			}
		
			/* record that we have one more route, with upper limit checking... */
			if (CRoutes_Count >= (CRoutes_MAX-2)) {
				/* printf("WARNING: expanding routing table\n");  */
				CRoutes_MAX += 50; /* arbitrary expansion number */
				CRoutes =(struct CRStruct *) REALLOC (CRoutes, sizeof (*CRoutes) * CRoutes_MAX);
			}
		
			CRoutes_Count ++;
		
	#ifdef CRVERBOSE 
				printf ("routing table now %d\n",CRoutes_Count);
				for (shifter = 0; shifter < CRoutes_Count; shifter ++) {
					printf ("%d: from: %p offset: %u Interpolator %p direction %d, len %d extra %d : ",shifter,
						CRoutes[shifter].routeFromNode, CRoutes[shifter].fnptr,
						CRoutes[shifter].interpptr, CRoutes[shifter].direction_flag, CRoutes[shifter].len, CRoutes[shifter].extra);
					for (insert_here = 0; insert_here < CRoutes[shifter].tonode_count; insert_here++) {
						printf (" to: %p %u",CRoutes[shifter].tonodes[insert_here].routeToNode,
									CRoutes[shifter].tonodes[insert_here].foffset);
					}
					printf ("\n");
				}
	#endif
		FREE_IF_NZ(newEntry);
		}
	}
	FREE_IF_NZ(routesToRegister);

}

#ifdef DEBUG_VALIDNODE
/* only if DEBUG_VALIDNODE is defined; helps us find memory/routing problems */
void mark_event_check (struct X3D_Node *from, int totalptr, char *fn, int line) {
	printf ("mark_event_check: at %s:%d\n",fn,line);
	if (X3D_NODE_CHECK(from)) {
		#ifdef CRVERBOSE
		printf ("mark_event_check, routing from a %s\n",stringNodeType(from->_nodeType));
		#endif
	} else {
		printf ("mark_event_check, not a real node %d\n",from);
	}
	mark_event(from,totalptr);
	printf ("mark_event_check: finished at %s:%d\n",fn,line); 
}
#endif
	
/********************************************************************

mark_event - something has generated an eventOut; record the node
data structure pointer, and the offset. Mark all relevant entries
in the routing table that this node/offset triggered an event.

********************************************************************/

void mark_event (struct X3D_Node *from, int totalptr) {
	int findit;

#ifdef WIN32
	if(from == 0) return;
	/*if(totalptr == 0) return; */
#endif
	X3D_NODE_CHECK(from);

	/* maybe this MARK_EVENT is coming in during initial node startup, before routing is registered? */
	if (!CRoutes_Initiated) {
		LOCK_PREROUTETABLE
		/* printf ("routes not registered yet; lets save this one for a bit...\n"); */
		if (initialEventBeforeRoutesCount >= preRouteTableSize) {
			preRouteTableSize += POSSIBLEINITIALROUTES;
			preEvents=REALLOC (preEvents,
				sizeof (struct initialRouteStruct) * preRouteTableSize);
		}
		preEvents[initialEventBeforeRoutesCount].from = from;
		preEvents[initialEventBeforeRoutesCount].totalptr = totalptr;
		initialEventBeforeRoutesCount++;
		UNLOCK_PREROUTETABLE

		return;  /* no routes registered yet */
	}

	findit = 1;

	#ifdef CRVERBOSE 
		printf ("\nmark_event, from %s (%u) fromoffset %u\n", stringNodeType(from->_nodeType),from, totalptr);
	#endif

	/* events in the routing table are sorted by routeFromNode. Find
	   out if we have at least one route from this node */
	while (from > CRoutes[findit].routeFromNode) {
		#ifdef CRVERBOSE
		printf ("mark_event, skipping past %x %x, index %d\n",from, CRoutes[findit].routeFromNode, findit);
		#endif
		findit ++;
	}

	/* while we have an eventOut from this NODE/OFFSET, mark it as
	   active. If no event from this NODE/OFFSET, ignore it */
	while ((from == CRoutes[findit].routeFromNode) &&
		(totalptr != CRoutes[findit].fnptr)) findit ++;

	/* did we find the exact entry? */
	#ifdef CRVERBOSE 
 		printf ("ep, (%#x %#x) (%#x %#x) at %d \n",
			from,CRoutes[findit].routeFromNode, totalptr,
			CRoutes[findit].fnptr,findit); 
	#endif

	/* if we did, signal it to the CEvents loop  - maybe more than one ROUTE,
	   eg, a time sensor goes to multiple interpolators */
	while ((from == CRoutes[findit].routeFromNode) &&
		(totalptr == CRoutes[findit].fnptr)) {
		#ifdef CRVERBOSE
			printf ("found event at %d\n",findit);
		#endif
		if (CRoutes[findit].intTimeStamp!=thisIntTimeStamp) {
			CRoutes[findit].isActive=TRUE;
			CRoutes[findit].intTimeStamp=thisIntTimeStamp;
		}

#ifdef CRVERBOSE
		else printf ("routing loop broken, findit %d\n",findit);
#endif

		findit ++;
	}
	#ifdef CRVERBOSE
		printf ("done mark_event\n");
	#endif
}

/********************************************************************

mark_script - indicate that this script has had an eventIn
zero_scripts - reset all script indicators

********************************************************************/
void mark_script (int num) {

	#ifdef CRVERBOSE 
		printf ("mark_script - script %d has been invoked\n",num);
	#endif
	scr_act[num]= TRUE;
}


/********************************************************************

gatherScriptEventOuts - at least one script has been triggered; get the
eventOuts for this script

********************************************************************/

static void gatherScriptEventOuts(void) {
	int route;
	size_t fptr;
	size_t tptr;
	size_t len;
 	struct X3D_Node* tn;
	struct X3D_Node* fn;

	int fromalready=FALSE;	 /* we have already got the from value string */
	int touched_flag=FALSE;
	unsigned int to_counter;
	CRnodeStruct *to_ptr = NULL;

	/* go through all routes, looking for this script as an eventOut */

	/* do we have any routes yet? - we can gather events before any routes are made */
	if (!CRoutes_Initiated) return;

	/* go from beginning to end in the routing table */
	route=1;
	while (route < (CRoutes_Count-1)) {
		#ifdef CRVERBOSE
		printf ("gather, routing %d is %s\n",route,
			stringNodeType(X3D_NODE(CRoutes[route].routeFromNode)->_nodeType));
		#endif

	if (X3D_NODE(CRoutes[route].routeFromNode)->_nodeType == NODE_Script) {
		struct X3D_Script *mys = X3D_SCRIPT(CRoutes[route].routeFromNode);
		struct Shader_Script *sp = (struct Shader_Script *) mys->__scriptObj;
		int actualscript = sp->num;

		/* printf ("gatherEvents, found a script at element %d, it is script number %d and node %u\n",
			route, actualscript,mys);  */
		/* this script initialized yet? We make sure that on initialization that the Parse Thread
		   does the initialization, once it is finished parsing. */
		if (!ScriptControl[actualscript]._initialized) {
			/* printf ("waiting for initializing script %d at %s:%d\n",actualscript, __FILE__,__LINE__); */
			return;
		}

		if (actualscript > max_script_found_and_initialized) {
			/* printf ("gatherScriptEventOut, waiting for script %d to become initialized\n"); */
			return;
		}

		if (!ScriptControl[actualscript].scriptOK) {
			/* printf ("gatherScriptEventOuts - script initialized but not OK\n"); */
			return;
		}
		
		/* is this the same from node/field as before? */
		if ((CRoutes[route].routeFromNode == CRoutes[route-1].routeFromNode) &&
			(CRoutes[route].fnptr == CRoutes[route-1].fnptr) &&
			(route > 1)) {
			fromalready=TRUE;
		} else {
			/* printf ("different from, have to get value\n"); */
			fromalready=FALSE;
		}

		fptr = CRoutes[route].fnptr;
		fn = CRoutes[route].routeFromNode;
		len = CRoutes[route].len;

		#ifdef CRVERBOSE
			printf ("\ngatherSentEvents, script %d from %s type %d len %d\n",actualscript, JSparamnames[fptr].name,
				JSparamnames[fptr].type, len);
		#endif

		/* now, set the actual properties - switch as documented above */
		if (!fromalready) {
			#ifdef CRVERBOSE 
				printf ("Not found yet, getting touched flag fptr %d script %d \n",fptr,actualscript);
			#endif
			touched_flag = get_valueChanged_flag((int)fptr,actualscript);
		}

		if (touched_flag!= 0) {
			/* get some easy to use pointers */
			for (to_counter = 0; to_counter < CRoutes[route].tonode_count; to_counter++) {
				to_ptr = &(CRoutes[route].tonodes[to_counter]);
				tn = to_ptr->routeToNode;
				tptr = to_ptr->foffset;

				#ifdef CRVERBOSE 
					printf ("%s script %d VALUE CHANGED! copy value and update %p\n",JSparamnames[fptr].name,actualscript,tn);
				#endif

				/* eventOuts go to VRML data structures */
				setField_javascriptEventOut(tn,(unsigned int) tptr,JSparamnames[fptr].type, (int) len, 
					CRoutes[route].extra, ScriptControl[actualscript].cx);

				/* tell this node now needs to redraw */
				markScriptResults(tn, (int) tptr, route, to_ptr->routeToNode);

				#ifdef CRVERBOSE 
					printf ("%s script %d has successfully updated  %u\n",JSparamnames[fptr].name,actualscript,tn);
				#endif

			}
		}

		/* unset the touched flag */
		resetScriptTouchedFlag ((int) actualscript, (int) fptr);

		/* REMOVE_ROOT(ScriptControl[actualscript].cx,global_return_val); */
	}
	route ++;
	}

	#ifdef CRVERBOSE 
		printf ("%f finished  gatherScriptEventOuts loop\n",TickTime);
	#endif
}


/* we have a Script/Shader at routing table element %d, send events to it */
static void sendScriptEventIn(int num) {
	int to_counter;
	CRnodeStruct *to_ptr = NULL;

	#ifdef CRVERBOSE
	  printf("----BEGIN-------\nsendScriptEventIn, num %d direction %d\n",num,
		CRoutes[num].direction_flag);
	#endif

	/* script value: 1: this is a from script route
			 2: this is a to script route
			 (3 = SCRIPT_TO_SCRIPT - this gets changed in to a FROM and a TO;
			 check for SCRIPT_TO_SCRIPT in this file */

	if (CRoutes[num].direction_flag == TO_SCRIPT) {
		for (to_counter = 0; to_counter < CRoutes[num].tonode_count; to_counter++) {
			struct Shader_Script *myObj;
			to_ptr = &(CRoutes[num].tonodes[to_counter]);
			if (to_ptr->routeToNode->_nodeType == NODE_Script) {
				/* this script initialized yet? We make sure that on initialization that the Parse Thread
				   does the initialization, once it is finished parsing. */

				/* get the value from the VRML structure, in order to propagate it to a script */
				myObj = X3D_SCRIPT(to_ptr->routeToNode)->__scriptObj;

				#ifdef CRVERBOSE
				printf ("myScriptNumber is %d\n",myObj->num);
				#endif


				/* is the script ok and initialized? */
				if ((!ScriptControl[myObj->num]._initialized) || (!ScriptControl[myObj->num].scriptOK)) {
					/* printf ("waiting for initializing script %d at %s:%d\n",(uintptr_t)to_ptr->routeToNode, __FILE__,__LINE__); */
					return;
				}

				/* mark that this script has been active SCRIPTS ARE INTEGER NUMBERS */
				mark_script(myObj->num);
				getField_ToJavascript((int)num,to_ptr->foffset);
			} else {
				getField_ToShader((int)num);
			}
		}
	} else {
		#ifdef CRVERBOSE 
			printf ("not a TO_SCRIPT value, ignoring this entry\n");
		#endif
	}
	#ifdef CRVERBOSE 
		printf("-----END-----\n");
	#endif

}

/********************************************************************

propagate_events.

Go through the event table, until the table is "active free". Some
nodes have eventins/eventouts - have to do the table multiple times
in this case.

********************************************************************/

#ifdef CRVERBOSE
char * BOOL_STRING(int inp) {if (inp)return "true "; else return "false ";}
#endif

void propagate_events() {
	int havinterp;
	int counter;
	int to_counter;
	CRnodeStruct *to_ptr = NULL;

		#ifdef CRVERBOSE
		printf ("\npropagate_events start\n");
		#endif

	/* increment the "timestamp" for this entry */
	thisIntTimeStamp ++; 

	do {
		havinterp=FALSE; /* assume no interpolators triggered */

		for (counter = 1; counter < CRoutes_Count-1; counter++) {
			for (to_counter = 0; to_counter < CRoutes[counter].tonode_count; to_counter++) {
				to_ptr = &(CRoutes[counter].tonodes[to_counter]);
				if (to_ptr == NULL) {
					printf("WARNING: tonode at %u is NULL in propagate_events.\n",
							to_counter);
					continue;
				}

				#ifdef CRVERBOSE
					printf("propagate_events: counter %d to_counter %u act %s from %u off %u to %u off %u oint %u dir %d\n",
						   counter, to_counter, BOOL_STRING(CRoutes[counter].isActive),
						   CRoutes[counter].routeFromNode, CRoutes[counter].fnptr,
						   to_ptr->routeToNode, to_ptr->foffset, CRoutes[counter].interpptr,
							CRoutes[counter].direction_flag);
				#endif

				if (CRoutes[counter].isActive == TRUE) {
					/* first thing, set this to FALSE */
					CRoutes[counter].isActive = FALSE;
						#ifdef CRVERBOSE
						printf("event %p %u len %d sent something", CRoutes[counter].routeFromNode, CRoutes[counter].fnptr,CRoutes[counter].len);
						if (CRoutes[counter].fnptr < 20) printf (" (script param: %s)",JSparamnames[CRoutes[counter].fnptr].name);
						else {
							printf (" (nodeType %s)",stringNodeType(X3D_NODE(CRoutes[counter].routeFromNode)->_nodeType));
						}
						printf ("\n");
						#endif
					/* to get routing to/from exposedFields, lets
					 * mark this to/offset as an event */
					MARK_EVENT (to_ptr->routeToNode, to_ptr->foffset);
					if (CRoutes[counter].direction_flag != 0) {
						/* scripts are a bit complex, so break this out */
						sendScriptEventIn(counter);
						havinterp = TRUE;
					} else {
						/* copy the value over */
						if (CRoutes[counter].len > 0) {
						/* simple, fixed length copy */
							memcpy( offsetPointer_deref(void *,to_ptr->routeToNode ,to_ptr->foffset),
								offsetPointer_deref(void *,CRoutes[counter].routeFromNode , CRoutes[counter].fnptr),
								(unsigned)CRoutes[counter].len);
						} else {
							/* this is a Multi*node, do a specialized copy. eg, Tiny3D EAI test will
							   trigger this */
							#ifdef CRVERBOSE
							printf ("in croutes, mmc len is %d\n",CRoutes[counter].len);
							#endif

							Multimemcpy (
								X3D_NODE(to_ptr->routeToNode),
								X3D_NODE(CRoutes[counter].routeFromNode),
								offsetPointer_deref(void *, to_ptr->routeToNode, to_ptr->foffset),
								offsetPointer_deref(void *, CRoutes[counter].routeFromNode, 
									CRoutes[counter].fnptr), CRoutes[counter].len);
						}

						/* is this an interpolator? if so call the code to do it */
						if (CRoutes[counter].interpptr != 0) {
							/* this is an interpolator, call it */
							havinterp = TRUE;
								#ifdef CRVERBOSE
								printf("propagate_events: index %d is an interpolator\n",
									   counter);
								#endif

							/* copy over this "extra" data, EAI "advise" calls need this */
							CRoutesExtra = CRoutes[counter].extra;
							CRoutes[counter].interpptr((void *)(to_ptr->routeToNode));
						} else {
							/* just an eventIn node. signal to the reciever to update */
							MARK_EVENT(to_ptr->routeToNode, to_ptr->foffset);

							/* make sure that this is pointing to a real node,
							 * not to a block of memory created by
							 * EAI - extra memory - if it has an offset of
							 * zero, it is most certainly made. */
							if ((to_ptr->foffset) != 0)
								update_node(to_ptr->routeToNode);
						}
					}
				}
			}
		}

		/* run gatherScriptEventOuts for each active script */
		gatherScriptEventOuts();

	} while (havinterp==TRUE);

	/* now, go through and clean up all of the scripts */
	for (counter =0; counter <= max_script_found_and_initialized; counter++) {
		if (scr_act[counter]) {
			scr_act[counter] = FALSE;
			CLEANUP_JAVASCRIPT(ScriptControl[counter].cx);
		}
	}	

	#ifdef CRVERBOSE
	printf ("done propagate_events\n\n");
	#endif
}


/********************************************************************

process_eventsProcessed()

According to the spec, all scripts can have an eventsProcessed
function - see section C.4.3 of the spec.

********************************************************************/
/* run the script from within C */
void process_eventsProcessed() {

	int counter;
	jsval retval;

	for (counter = 0; counter <= max_script_found_and_initialized; counter++) {
		if (ScriptControl[counter].eventsProcessed == NULL) {
			ScriptControl[counter].eventsProcessed = JS_CompileScript(
				 ScriptControl[counter].cx,
				 ScriptControl[counter].glob,
				"eventsProcessed()", strlen ("eventsProcessed()"),
				"compile eventsProcessed()", 1);

		}

		if (!JS_ExecuteScript( ScriptControl[counter].cx,
                                 ScriptControl[counter].glob,
				ScriptControl[counter].eventsProcessed, &retval)) {
#if defined(_MSC_VER)
			printf ("can not run eventsProcessed() for script %d thread %u\n",counter,(unsigned int)pthread_self().x);
#else
			printf ("can not run eventsProcessed() for script %d thread %p\n",counter,pthread_self());
#endif
		}

	}
}

/*******************************************************************

do_first()


Call the sensor nodes to get the results of the clock ticks; this is
the first thing in the event loop.

********************************************************************/

void do_first() {
	int counter;
	/* go through the array; add_first will NOT add a null pointer
	   to either field, so we don't need to bounds check here */

	for (counter =0; counter < num_ClockEvents; counter ++) {
		ClockEvents[counter].interpptr(ClockEvents[counter].tonode);
	}

	/* now, propagate these events */
	propagate_events();

	/* any new routes waiting in the wings for buffering to happen? */
	/* Note - rTr will be incremented by either parsing (in which case,
	   events are not run, correct?? or by a script within a route,
	   which will be this thread, or by EAI, which will also be this
	   thread, so the following should be pretty thread_safe */ 

	if (routesToRegister != NULL) {
		MUTEX_LOCK_ROUTING_UPDATES
		actually_do_CRoutes_Register();
		MUTEX_FREE_LOCK_ROUTING_UPDATES
	}

	/* any mark_events kicking around, waiting for someone to come in and tell us off?? */
	/* CRoutes_Inititated should be set here, as it would have been created in 
	   actually_do_CRoutes_Register */
	if (preEvents != NULL) {
		if (CRoutes_Initiated) {
		LOCK_PREROUTETABLE

		#ifdef CRVERBOSE
		printf ("doing preEvents, we have %d events \n",initialEventBeforeRoutesCount);
		#endif

		for (counter = 0; counter < initialEventBeforeRoutesCount; counter ++) {
			MARK_EVENT(preEvents[counter].from, preEvents[counter].totalptr);
		}
		initialEventBeforeRoutesCount = 0;
		preRouteTableSize = 0;
		FREE_IF_NZ(preEvents);
		UNLOCK_PREROUTETABLE
		}
	}
}


/*******************************************************************

Interface to allow EAI/SAI to get routing information.

********************************************************************/

int getRoutesCount(void) {
	return CRoutes_Count;
}

void getSpecificRoute (int routeNo, struct X3D_Node **fromNode, int *fromOffset, 
		struct X3D_Node **toNode, int *toOffset) {
        CRnodeStruct *to_ptr = NULL;


	if ((routeNo <1) || (routeNo >= CRoutes_Count)) {
		*fromNode = NULL; *fromOffset = 0; *toNode = NULL; *toOffset = 0;
	}
/*
	printf ("getSpecificRoute, fromNode %d fromPtr %d tonode_count %d\n",
		CRoutes[routeNo].routeFromNode, CRoutes[routeNo].fnptr, CRoutes[routeNo].tonode_count);
*/
		*fromNode = CRoutes[routeNo].routeFromNode;
		*fromOffset = CRoutes[routeNo].fnptr;
	/* there is not a case where tonode_count != 1 for a valid route... */
	if (CRoutes[routeNo].tonode_count != 1) {
		printf ("huh? tonode count %d\n",CRoutes[routeNo].tonode_count);
		*toNode = 0; *toOffset = 0;
		return;
	}

	/* get the first toNode,toOffset */
        to_ptr = &(CRoutes[routeNo].tonodes[0]);
        *toNode = to_ptr->routeToNode;
	*toOffset = to_ptr->foffset;


	

}
/*******************************************************************

kill_routing()

Stop routing, remove structure. Used for ReplaceWorld style calls.

********************************************************************/

void kill_routing (void) {
        if (CRoutes_Initiated) {
                CRoutes_Initiated = FALSE;
                CRoutes_Count = 0;
                CRoutes_MAX = 0;
                FREE_IF_NZ (CRoutes);
        }
}


/* internal variable to copy a C structure's Multi* field */
void Multimemcpy (struct X3D_Node *toNode, struct X3D_Node *fromNode, void *tn, void *fn, size_t multitype) {
	size_t structlen;
	int fromcount, tocount;
	void *fromptr, *toptr;

	struct Multi_Vec3f *mv3ffn, *mv3ftn;

	#ifdef CRVERBOSE 
		printf ("Multimemcpy, copying structures from %p (%s) to %p (%s)  %p %p type %d\n",
			fromNode, stringNodeType(fromNode->_nodeType),
			toNode, stringNodeType(toNode->_nodeType),
			
			tn,fn,multitype); 
	#endif

	/* copy a complex (eg, a MF* node) node from one to the other
	   grep for the ROUTING_SF and ROUTING_MF defines to see them all. */

	/* Multi_XXX nodes always consist of a count then a pointer - see
	   Structs.h */

	/* making the input pointers into a (any) structure helps deciphering params */
	mv3ffn = (struct Multi_Vec3f *)fn;
	mv3ftn = (struct Multi_Vec3f *)tn;

	/* so, get the from memory pointer, and the to memory pointer from the structs */
	fromptr = (void *)mv3ffn->p;

	/* and the from and to sizes */
	fromcount = mv3ffn->n;
	tocount = mv3ftn->n;

	#ifdef CRVERBOSE 
		printf ("Multimemcpy, fromcount %d\n",fromcount);
	#endif

	/* get the structure length */
	switch (multitype) {
		case ROUTING_SFNODE: structlen = sizeof (void *); break;
		case ROUTING_MFNODE: structlen = sizeof (void *); break;
		case ROUTING_SFIMAGE: structlen = sizeof (void *); break;
		case ROUTING_MFSTRING: structlen = sizeof (void *); break;
		case ROUTING_MFFLOAT: structlen = sizeof (float); break;
		case ROUTING_MFROTATION: structlen = sizeof (struct SFRotation); break;
		case ROUTING_MFINT32: structlen = sizeof (int); break;
		case ROUTING_MFCOLOR: structlen = sizeof (struct SFColor); break;
		case ROUTING_MFVEC2F: structlen = sizeof (struct SFVec2f); break;
		case ROUTING_MFVEC3F: structlen = sizeof (struct SFColor); break; /* This is actually SFVec3f - but no struct of this type */
		case ROUTING_MFVEC3D: structlen = sizeof (struct SFVec3d); break;
		case ROUTING_MFDOUBLE: structlen = sizeof (double); break;
		case ROUTING_MFMATRIX4F: structlen = sizeof (struct SFMatrix4f); break;
		case ROUTING_MFMATRIX4D: structlen = sizeof (struct SFMatrix4d); break;
		case ROUTING_MFVEC2D: structlen = sizeof (struct SFVec2d); break;
		case ROUTING_MFVEC4F: structlen = sizeof (struct SFVec4f); break;
		case ROUTING_MFVEC4D: structlen = sizeof (struct SFVec4d); break;
		case ROUTING_MFMATRIX3F: structlen = sizeof (struct SFMatrix3f); break;
		case ROUTING_MFMATRIX3D: structlen = sizeof (struct SFMatrix3d); break;

		case ROUTING_SFSTRING: { 
			/* SFStrings are "special" */
			/* remember:
				struct Uni_String {
				        int len;
				        char * strptr;
				        int touched;
			};
			*/
			struct Uni_String *fStr; 
			struct Uni_String *tStr;

			/* get the CONTENTS of the fn and tn pointers */
			memcpy (&fStr,fn,sizeof (void *));
			memcpy (&tStr,tn,sizeof (void *));


			/* printf ("copying over a SFString in Multi from %u to %u\n",fStr, tStr);
			printf ("string was :%s:\n",tStr->strptr); */
			verify_Uni_String(tStr, fStr->strptr); 
			/* printf ("string is :%s:\n",tStr->strptr); */
			return; /* we have done the needed stuff here */
			break;
		}
		default: {
			 /* this is MOST LIKELY for an EAI handle_Listener call - if not, it is a ROUTING problem... */
			/* printf("WARNING: Multimemcpy, don't handle type %d yet\n", multitype);  */
			structlen=0;
			return;
		}
	}


	FREE_IF_NZ (mv3ftn->p);

	/* MALLOC the toptr */
	mv3ftn->p = (struct SFColor *)MALLOC (structlen*fromcount);
	toptr = (void *)mv3ftn->p;

	/* tell the recipient how many elements are here */
	mv3ftn->n = fromcount;

	#ifdef CRVERBOSE 
		printf ("Multimemcpy, fromcount %d tocount %d fromptr %p toptr %p\n",fromcount,tocount,fromptr,toptr); 
	#endif

	/* and do the copy of the data */
	memcpy (toptr,fromptr,structlen * fromcount);

	/* is this an MFNode or SFNode? */
	if (toNode != (struct X3D_Node*) EAIListenerData) {
		if (multitype==ROUTING_SFNODE) {
#ifdef CRVERBOSE
			printf ("got a ROUTING_SFNODE, adding %u to %u\n",(unsigned int) fn, (unsigned int) toNode);
#endif
			ADD_PARENT(X3D_NODE(fn),toNode);
		}
		if (multitype==ROUTING_MFNODE) {
			int count;
			struct X3D_Node **arrptr = (struct X3D_Node **)mv3ffn->p;

			#ifdef CRVERBOSE
			printf ("fromcount %d tocount %d\n",fromcount, tocount);
			printf ("ROUTING - have to add parents... \n");
			#endif

			for (count = 0; count < mv3ffn->n; count++) {
				#ifdef CRVERBOSE
				printf ("node in place %d is %u ",count,arrptr[count]);
				printf ("%s ",stringNodeType(arrptr[count]->_nodeType));
				printf ("\n");
				#endif

				ADD_PARENT(arrptr[count],toNode);
			}
		}
	}
}

/* this script value has been looked at, set the touched flag in it to FALSE. */
void resetScriptTouchedFlag(int actualscript, int fptr) {

	#ifdef CRVERBOSE
	printf ("resetScriptTouchedFlag, name %s type %s script %d, fptr %d\n",JSparamnames[fptr].name, stringFieldtypeType(JSparamnames[fptr].type), actualscript, fptr);
	#endif

	switch (JSparamnames[fptr].type) {
		RESET_TOUCHED_TYPE_A(SFRotation)
		RESET_TOUCHED_TYPE_A(SFNode)
		RESET_TOUCHED_TYPE_A(SFVec2f)
		RESET_TOUCHED_TYPE_A(SFVec3f)
		RESET_TOUCHED_TYPE_A(SFVec4f)
		/* RESET_TOUCHED_TYPE_A(SFVec2d) */
		RESET_TOUCHED_TYPE_A(SFVec3d)
		RESET_TOUCHED_TYPE_A(SFVec4d)
		RESET_TOUCHED_TYPE_A(SFImage)
		RESET_TOUCHED_TYPE_A(SFColor)
		RESET_TOUCHED_TYPE_A(SFColorRGBA)
		RESET_TOUCHED_TYPE_MF_A(MFRotation,SFRotation)
		RESET_TOUCHED_TYPE_MF_A(MFNode,SFNode)
		RESET_TOUCHED_TYPE_MF_A(MFVec2f,SFVec2f)
		RESET_TOUCHED_TYPE_MF_A(MFVec3f,SFVec3f)
		RESET_TOUCHED_TYPE_MF_A(MFVec4f,SFVec4f)
		RESET_TOUCHED_TYPE_MF_A(MFVec4d,SFVec4d)
		/* RESET_TOUCHED_TYPE_MF_A(MFImage,SFImage) */
		RESET_TOUCHED_TYPE_MF_A(MFColor,SFColor)
		RESET_TOUCHED_TYPE_MF_A(MFColorRGBA,SFColorRGBA)

		RESET_TOUCHED_TYPE_ECMA (SFInt32)
		RESET_TOUCHED_TYPE_ECMA (SFBool)
		RESET_TOUCHED_TYPE_ECMA (SFFloat)
		RESET_TOUCHED_TYPE_ECMA (SFTime)
		RESET_TOUCHED_TYPE_ECMA (SFString)
		RESET_ECMA_MF_TOUCHED(MFInt32)
		RESET_ECMA_MF_TOUCHED(MFBool) 
		RESET_ECMA_MF_TOUCHED(MFFloat) 
		RESET_ECMA_MF_TOUCHED(MFTime) 
		RESET_ECMA_MF_TOUCHED(MFString) 
		
			
		default: {printf ("can not reset touched_flag for %s\n",stringFieldtypeType(JSparamnames[fptr].type));
		}
	}
}

/*********************************************************************************************/

static struct X3D_Node *returnSpecificTypeNode(int requestedType, int *offsetOfsetValue, int *offsetOfvalueChanged) {
	struct X3D_Node *rv;

	rv = NULL;
	switch  (requestedType) {
                 #define SF_TYPE(fttype, type, ttype) \
                        case FIELDTYPE_##fttype: \
			rv = createNewX3DNode(NODE_Metadata##fttype); \
			*offsetOfsetValue = (int) offsetof (struct X3D_Metadata##fttype, setValue); \
			*offsetOfvalueChanged = (int) offsetof (struct X3D_Metadata##fttype, valueChanged); \
			break; 

                        #define MF_TYPE(fttype, type, ttype) \
                                SF_TYPE(fttype, type, ttype)

                        #include "VrmlTypeList.h"

                        #undef SF_TYPE
                        #undef MF_TYPE
			default: {
				printf ("returnSpecific, not found %d\n",requestedType);
			}
	}
	return rv;
}

