/*
=INSERT_TEMPLATE_HERE=

$Id: CRoutes.c,v 1.9 2008/12/31 17:19:30 crc_canada Exp $

???

*/

#include <config.h>
#include <system.h>
#include <system_threads.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>


#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/CScripts.h"
#include "../world_script/fieldSet.h"
#include "CParseParser.h"
#include "CParseLexer.h"
#include "../world_script/jsUtils.h"
#include "../world_script/jsNative.h"
#include "../input/SensInterps.h"


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
				JSSFpointer = (uintptr_t *)ptr; /* save this for quick extraction of values */ \
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
		cx = (JSContext *)ScriptControl[actualscript].cx; \
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
				touched = findNameInECMATable((JSContext *) ScriptControl[actualscript].cx,fullname);\
				break;\
			}

#define GET_ECMA_MF_TOUCHED(thistype) \
	case FIELDTYPE_MF##thistype: {\
		jsval mainElement; \
		/* printf ("GET_ECMA_MF_TOUCHED called on %d\n",JSglobal_return_val);*/ \
		if (!JS_GetProperty(cx, (JSObject *)JSglobal_return_val, "MF_ECMA_has_changed", &mainElement)) { \
			printf ("JS_GetProperty failed for \"MF_ECMA_HAS_changed\" in get_valueChanged_flag\n"); \
		} else /* printf ("GET_ECMA_MF_TOUCHED MF_ECMA_has_changed is %d for %d %d\n",JSVAL_TO_INT(mainElement),cx,JSglobal_return_val); */ \
		touched = JSVAL_TO_INT(mainElement);\
		break; \
	}

#define RESET_ECMA_MF_TOUCHED(thistype) \
	case FIELDTYPE_##thistype: {\
		jsval myv = INT_TO_JSVAL(0); \
		/* printf ("RESET_ECMA_MF_TOUCHED called on %d ",JSglobal_return_val);*/ \
        	if (!JS_SetProperty((JSContext *) ScriptControl[actualscript].cx, (JSObject *)JSglobal_return_val, "MF_ECMA_has_changed", &myv)) { \
        		printf( "JS_SetProperty failed for \"MF_ECMA_has_changed\" in RESET_ECMA_MF_TOUCHED.\n"); \
        	}\
                /* if (!JS_GetProperty((JSContext *) ScriptControl[actualscript].cx, (JSObject *)JSglobal_return_val, "MF_ECMA_has_changed", &mainElement)) { \
                        printf ("JS_GetProperty failed for \"MF_ECMA_HAS_changed\" in get_valueChanged_flag\n"); \
		} \
                printf ("and MF_ECMA_has_changed is %d\n",JSVAL_TO_INT(mainElement)); */\
	break; \
	}

#define RESET_TOUCHED_TYPE_ECMA(thistype) \
			case FIELDTYPE_##thistype: { \
				resetNameInECMATable((JSContext *) ScriptControl[actualscript].cx,JSparamnames[fptr].name); \
				break; \
			}
/* in case Bool was defined above, restore the value */
#define Bool savedBool


void setMFElementtype (uintptr_t num);

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
uintptr_t *scr_act = 0;				/* this script has been sent an eventIn */
int max_script_found = -1;			/* the maximum script number found */
int max_script_found_and_initialized = -1;	/* the maximum script number found */

/* EAI needs the extra parameter, so we put it globally when a RegisteredListener is clicked. */
int CRoutesExtra = 0;

/* global return value for getting the value of a variable within Javascript */
jsval JSglobal_return_val;
uintptr_t *JSSFpointer;

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
		unsigned int to_count;
		char *tonode_str;
		int length;
		void *intptr;
		int scrdir;
		int extra; };

struct CR_RegStruct *routesToRegister = NULL;
int maxRTR = 0;
int rTr = 0;


/* if we get mark_events sent, before routing is established, save them and use them
   as soon as routing is here */
#define POSSIBLEINITIALROUTES 1000
static int initialEventBeforeRoutesCount = 0;
static int preRouteTableSize = 0;
struct initialRouteStruct {
	struct X3D_Node *from;
	unsigned int totalptr;
};
static struct initialRouteStruct *preEvents = NULL;
pthread_mutex_t  preRouteLock = PTHREAD_MUTEX_INITIALIZER;
#define LOCK_PREROUTETABLE                pthread_mutex_lock(&preRouteLock);
#define UNLOCK_PREROUTETABLE              pthread_mutex_unlock(&preRouteLock);



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
		printf ("markScriptResults: can update this node %d %d\n",tn,tptr); 
		#endif
		update_node(tn);
	#ifdef CRVERBOSE
	} else {
		printf ("markScriptResults: skipping this node %d %d flag %d\n",tn,tptr,CRoutes[route].direction_flag); 
	#endif
	}

	MARK_EVENT (X3D_NODE(CRoutes[route].routeFromNode),CRoutes[route].fnptr);

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
/*	uintptr_t *JSSFpointer for SF non-ECMA nodes.				*/
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

int get_valueChanged_flag (uintptr_t fptr, uintptr_t actualscript) {
	JSContext *cx;
	jsval interpobj;
	char *fullname;
	int touched;

	touched = FALSE;
	interpobj = ScriptControl[actualscript].glob;
	cx = (JSContext *) ScriptControl[actualscript].cx;
	fullname = JSparamnames[fptr].name;

	#ifdef CRVERBOSE
	printf ("getting property for fullname %s, cx %d, interpobj %d script %d, fptr %d\n",fullname,cx,interpobj,actualscript, fptr);
	#endif

	if (!JS_GetProperty(cx, (JSObject *) interpobj ,fullname,&JSglobal_return_val)) {
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
			GETJSPTR_TYPE_A(SFVec3f)
			GETJSPTR_TYPE_A(SFImage)
			GETJSPTR_TYPE_A(SFColor)
			GETJSPTR_TYPE_A(SFColorRGBA)

			GETJSPTR_TYPE_MF_A(MFRotation,SFRotation)
			GETJSPTR_TYPE_MF_A(MFNode,SFNode)
			GETJSPTR_TYPE_MF_A(MFVec2f,SFVec2f)
			GETJSPTR_TYPE_MF_A(MFVec3f,SFVec3f)
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
		uintptr_t *nodelist,
		int len,
		int ar) {
	int oldlen;
	void *newmal;
	int num_removed;
	uintptr_t *remchild;
	uintptr_t *remptr;
	uintptr_t *tmpptr;
	int done;

	int counter, c2;

	/* printf ("AddRemove, field is %d in from parent offsetof (struct X3D_Group, children) is %d\n",(char *)tn - (char *)parent,
			offsetof (struct X3D_Group, children));

	printf ("AddRemove Children parent %u tn %u, len %d ar %d\n",parent,tn,len,ar);
	*/

	/* if no elements, just return */
	if (len <=0) return;
	if ((parent==0) || (tn == 0)) {
		printf ("Freewrl: AddRemoveChildren, parent and/or field NULL\n");
		return;
	}

	oldlen = tn->n;
	/* printf ("AddRemoveChildren, len %d, oldlen %d ar %d\n",len, oldlen, ar); */

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
		if (oldlen > 0) FREE_IF_NZ(tn->p);

		/* now, make this into an addChildren */
		oldlen = 0;
		ar = 1;

	}

	if (ar == 1) {
		/* addChildren - now we know how many SFNodes are in this MFNode, lets MALLOC and add */

		/* first, set children to 0, in case render thread comes through here */
		tn->n = 0;

		newmal = MALLOC ((oldlen+len)*sizeof(void *));

		/* copy the old stuff over */
		if (oldlen > 0) memcpy (newmal,tn->p,oldlen*sizeof(void *));

		/* set up the C structures for this new MFNode addition */
		FREE_IF_NZ (tn->p);
		tn->p = newmal;

		/* copy the new stuff over - note, newmal changes 
		what it points to */

		newmal = (void *) (newmal + sizeof (void *) * oldlen);
		memcpy(newmal,nodelist,sizeof(void *) * len);

		/* tell each node in the nodelist that it has a new parent */
		for (counter = 0; counter < len; counter++) {
			/* printf ("AddRemove, count %d of %d, node %u parent %u\n",counter, len,nodelist[counter],parent); */
			ADD_PARENT((void *)nodelist[counter],(void *)parent);
		}

		/* and, set the new length */
		tn->n = len+oldlen;

	} else {
		/* this is a removeChildren */

		/* go through the original array, and "zero" out children that match one of
		   the parameters */

		num_removed = 0;
		remchild = nodelist;
		/* printf ("removing, len %d, tn->n %d\n",len,tn->n); */
		for (c2 = 0; c2 < len; c2++) {
			remptr = (uintptr_t*) tn->p;
			done = FALSE;

			for (counter = 0; counter < tn->n; counter ++) {
				/* printf ("remove, comparing %d with %d\n",*remptr, *remchild);  */
				if ((*remptr == *remchild) && (!done)) {
					remove_parent(X3D_NODE(*remchild),parent);
					*remptr = 0;  /* "0" can not be a valid memory address */
					num_removed ++;
					done = TRUE; /* remove this child ONLY ONCE - in case it has been added
							more than once. */
				}
				remptr ++;
			}
			remchild ++;
		}

		/* printf ("end of finding, num_removed is %d\n",num_removed); */

		if (num_removed > 0) {
			/* printf ("MALLOCing size of %d\n",(oldlen-num_removed)*sizeof(void *)); */
			newmal = MALLOC ((oldlen-num_removed)*sizeof(void *));
			tmpptr = newmal;
			remptr = (uintptr_t*) tn->p;

			/* go through and copy over anything that is not zero */
			for (counter = 0; counter < tn->n; counter ++) {
				/* printf ("count %d is %d\n",counter, *remptr); */
				if (*remptr != 0) {
					*tmpptr = *remptr;
					/* printf ("now, tmpptr %d is %d\n",tmpptr,*tmpptr); */
					tmpptr ++;
				}
				remptr ++;
			}
			/* printf ("done loops, now make data active \n"); */

			/* now, do the move of data */
			tn->n = 0;
			FREE_IF_NZ (tn->p);
			tn->p = newmal;
			tn->n = oldlen - num_removed;
		}
	}
	update_node(parent);
}

/****************************************************************/
/* a CLASS is returning a Multi-number type; copy this from 	*/
/* the CLASS to the data structure within the freewrl C side	*/
/* of things.							*/
/*								*/
/* note - this cheats in that the code assumes that it is 	*/
/* a series of Multi_Vec3f's while in reality the structure	*/
/* of the multi structures is the same - so we "fudge" things	*/
/* to make this multi-purpose.					*/
/* eletype switches depending on:				*/
/* what the sub clen does in VRMLFields.pm;			*/
/*  "String" {return -13;} 					*/
/*  "Float" {return -14;}        				*/
/*  "Rotation" {return -15;}     				*/
/*  "Int32" {return -16;}        				*/
/*  "Color" {return -1;}        				*/
/*  "Vec2f" {return -18;}        				*/
/*  "Vec3f" {return -19;}         				*/
/*  "Vec3d" {return -20;}         				*/
/*  "Node" {return -10;}         				*/
/****************************************************************/

void getCLASSMultNumType (char *buf, int bufSize,
			  struct Multi_Vec3f *tn,
			  struct X3D_Node *parent,
			  int eletype, int addChild) {
	int len;
	int elesize;




	/* get size of each element, used for MALLOCing memory */
	switch (eletype) {
	  case -13: elesize = sizeof (char); break;	/* string   */
	  case -14:
	  case FIELDTYPE_MFFloat:
	    elesize = sizeof (float); break;	        /* Float    */
	  case -15: elesize = sizeof(float)*4; break;	/* Rotation */
	  case -16: elesize = sizeof(int); break;	/* Integer  */

	  case -1:
	  case -17:
	  case -19:
	    elesize = sizeof(float)*3;
	    break;	/* SFColor, SFVec3f */
	  case -18: elesize = sizeof(float)*2; break;	/* SFVec2f */
	  case -20: elesize = sizeof(double)*3; break;	/* SFVec3d */
	  case -10: elesize = sizeof(int); break;
	  default: {printf ("getCLASSMulNumType - unknown type %d\n",eletype); return;}
	}

	len = bufSize / elesize;  /* convert Bytes into whatever */

	#ifdef CRVERBOSE
		printf("getCLASSMultNumType: bufSize:%d, eletype:%d, allocated: %d, elesize: %d.\n",
	       bufSize,eletype, tn->n, elesize);
	#endif

	/* now, we either replace the whole data, or we add or remove it if
	 * this is a Node type. (eg, add/remove child) */

	if (eletype != -10) {
		/* do we have to REALLOC memory? */
		if (len != tn->n) {
			/* yep... */
		        /* printf ("old pointer %d\n",tn->p); */
			tn->n = 0;	/* gets around possible mem problem */
			FREE_IF_NZ (tn->p);
			tn->p =(struct SFColor *)MALLOC ((unsigned)(elesize*len));
		}

		/* copy memory over */
		memcpy (tn->p, buf, bufSize);

		/* and, tell the scene graph how many elements there are in here */
		tn->n = len;
	} else {
		/* this is a Node type, so we need to add/remove children */
		AddRemoveChildren (parent, (struct Multi_Node*)tn, (uintptr_t*)buf, len, addChild);
	}
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

void CRoutes_js_new (uintptr_t num, int scriptType) {
	/* record whether this is a javascript, class invocation, ... */
	ScriptControl[num].thisScriptType = scriptType;

	/* compare with a intptr_t, because we need to compare to -1 */
	if ((intptr_t)num > max_script_found) max_script_found = (intptr_t)num;
}


/********************************************************************

JSparamIndex.

stores ascii names with types (see code for type equivalences).

********************************************************************/

int JSparamIndex (char *name, char *type) {
	unsigned len;
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
	JSparamnames[jsnameindex].eventInFunction = 0;
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
	int len, int dir) {

 	/* 10+1+3+1=15:  Number <5000000000, :, number <999, \0 */
 	char tonode_str[15];
 	void* interpolatorPointer;
 	int extraData = 0;

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

 	/* printf ("CRoutes_RegisterSimple, from %u fromOfs %u, to %u toOfs %u, len %d dir %d\n",from, fromOfs, to, toOfs, len, dir);  */


	/* When routing to a script, to is not a node pointer! */
	if(dir!=SCRIPT_TO_SCRIPT && dir!=TO_SCRIPT)
		interpolatorPointer=returnInterpolatorPointer(stringNodeType(to->_nodeType));
	else
		interpolatorPointer=NULL;

	snprintf(tonode_str, 15, "%u:%d", (unsigned)to, toOfs);

	CRoutes_Register(1, from, fromOfs, 1, tonode_str, len, interpolatorPointer, dir, extraData);
}
 

/********************************************************************

Remove a route, but with fewer and more expressive parameters than
CRoutes_Register.  Currently a wrapper around that other function.

********************************************************************/

void CRoutes_RemoveSimple(
	struct X3D_Node* from, int fromOfs,
	struct X3D_Node* to, int toOfs,
	int len, int dir) {

 	/* 10+1+3+1=15:  Number <5000000000, :, number <999, \0 */
 	char tonode_str[15];
 	void* interpolatorPointer;
 	int extraData = 0;

 	/* When routing to a script, to is not a node pointer! */
 	if(dir!=SCRIPT_TO_SCRIPT && dir!=TO_SCRIPT)
  		interpolatorPointer=returnInterpolatorPointer(stringNodeType(to->_nodeType));
 	else
  		interpolatorPointer=NULL;

 	snprintf(tonode_str, 15, "%u:%d", (unsigned)to, toOfs);

 	CRoutes_Register(0, from, fromOfs, 1, tonode_str, len, 
  		interpolatorPointer, dir, extraData);
}

/********************************************************************

CRoutes_Register.

Register a route in the routing table.

********************************************************************/

	
void CRoutes_Register(
		int adrem,
		struct X3D_Node *from,
		int fromoffset,
		unsigned int to_count,
		char *tonode_str,
		int length,
		void *intptr,
		int scrdir,
		int extra) {
	/* have to increase routing space?? */

	if (rTr >= maxRTR) {
		maxRTR += 50;
		routesToRegister = (struct CR_RegStruct *)REALLOC(routesToRegister,sizeof (struct CR_RegStruct) * (maxRTR+1));
	}

	routesToRegister[rTr].adrem = adrem;
	routesToRegister[rTr].from = from;
	routesToRegister[rTr].fromoffset = fromoffset;
	routesToRegister[rTr].to_count = to_count;
	routesToRegister[rTr].tonode_str= STRDUP(tonode_str);
	routesToRegister[rTr].length = length;
	routesToRegister[rTr].intptr = intptr;
	routesToRegister[rTr].scrdir = scrdir;
	routesToRegister[rTr].extra = extra;
	rTr ++;

}

#define RTR routesToRegister[num] 
static void actually_do_CRoutes_Register(int num) {
	int insert_here, shifter;
	char *buffer;
	const char *token = " ";
	CRnodeStruct *to_ptr = NULL;
	unsigned int to_counter;
	struct Multi_Node *Mchptr;
	void * chptr;
	int rv;				/* temp for sscanf rets */

	char buf[20];
	long unsigned int toof;		/* used to help determine duplicate routes */
	long unsigned int toN;

	#ifdef CRVERBOSE  
		printf ("CRoutes_Register adrem %d from %u ",RTR.adrem, RTR.from);
		if (RTR.from > JSMaxScript) printf ("(%s) ",stringNodeType(X3D_NODE(RTR.from->_nodeType)));

		printf ("off %u to %u %s len %d intptr %u\n",
				RTR.fromoffset, RTR.to_count, RTR.tonode_str, RTR.length, RTR.intptr);
		printf ("CRoutes_Register, CRoutes_Count is %d\n",CRoutes_Count);
	#endif

	/* is this a script to script route??? */
	/* if so, we need an intermediate location for memory, as the values must
	   be placed somewhere FROM the script node, to be found when sending TO
	   the other script */
	if (RTR.scrdir == SCRIPT_TO_SCRIPT) {
		if (RTR.length <= 0) {
			/* this is of an unknown length - most likely a MF* field */

			/* So, this is a Multi_Node, MALLOC it... */
			chptr = MALLOC (sizeof(struct Multi_Node));
			Mchptr = (struct Multi_Node *)chptr; 

			#ifdef CRVERBOSE 
				printf ("hmmm - script to script, len %d ptr %d %x\n",
				RTR.length,chptr,chptr);
			#endif

			Mchptr->n = 0; /* make it 0 nodes long */
			Mchptr->p = 0; /* it has no memory MALLOCd here */
			
		} else {
			/* this is just a block of memory, eg, it will hold an "SFInt32" */
			chptr = MALLOC (sizeof (char) * RTR.length);
		}
		sprintf (buf,"%u:0",(unsigned int)chptr);
		CRoutes_Register (RTR.adrem, RTR.from, RTR.fromoffset,1,buf, RTR.length, 0, FROM_SCRIPT, RTR.extra);
		CRoutes_Register (RTR.adrem, chptr, 0, RTR.to_count, RTR.tonode_str,RTR.length, 0, TO_SCRIPT, RTR.extra);
		return;
	}

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
		CRoutes[1].routeFromNode = X3D_NODE(-1);
		CRoutes[1].fnptr = 0x8FFFFFFF;
		CRoutes[1].tonode_count = 0;
		CRoutes[1].tonodes = NULL;
		CRoutes[1].isActive = FALSE;
		CRoutes[1].interpptr = 0;
		CRoutes_Count = 2;
		CRoutes_Initiated = TRUE;
	}

	insert_here = 1;

	/* go through the routing list, finding where to put it */
	while (RTR.from > CRoutes[insert_here].routeFromNode) {
		#ifdef CRVERBOSE 
			printf ("comparing %u to %u\n",RTR.from, CRoutes[insert_here].routeFromNode);
		#endif
		insert_here++;
	}

	/* hmmm - do we have a route from this node already? If so, go
	   through and put the offsets in order */
	while ((RTR.from == CRoutes[insert_here].routeFromNode) &&
		(RTR.fromoffset > CRoutes[insert_here].fnptr)) {
		#ifdef CRVERBOSE 
			printf ("same routeFromNode, different offset\n");
		#endif
		insert_here++;
	}


	/* Quick check to verify that we don't have a duplicate route here
	   OR to delete a route... */

	#ifdef CRVERBOSE
	printf ("ok, CRoutes_Register - is this a duplicate? comparing from (%d %d), fnptr (%d %d) intptr (%d %d) and tonodes %d\n",
		CRoutes[insert_here].routeFromNode, RTR.from,
		CRoutes[insert_here].fnptr, RTR.fromoffset,
		CRoutes[insert_here].interpptr, RTR.intptr,
		CRoutes[insert_here].tonodes);
	#endif

	if ((CRoutes[insert_here].routeFromNode==RTR.from) &&
		(CRoutes[insert_here].fnptr==(unsigned)RTR.fromoffset) &&
		(CRoutes[insert_here].interpptr==RTR.intptr) &&
		(CRoutes[insert_here].tonodes!=0)) {

		/* possible duplicate route */
		rv=sscanf (RTR.tonode_str, "%lu:%lu", &toN,&toof);
		/* printf ("from tonode_str %s we have %u %u\n",tonode_str, toN, toof); */

		if ((toN == ((uintptr_t)(CRoutes[insert_here].tonodes)->routeToNode)) &&
			(toof == (CRoutes[insert_here].tonodes)->foffset)) {
			/* this IS a duplicate, now, what to do? */

			#ifdef CRVERBOSE
			printf ("duplicate route; maybe this is a remove? \n");
			#endif

			/* is this an add? */
			if (RTR.adrem == 1) {
				#ifdef CRVERBOSE
					printf ("definite duplicate, returning\n");
				#endif
				return;
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
						printf ("%u %u %u\n",CRoutes[shifter].routeFromNode, CRoutes[shifter].fnptr,
							CRoutes[shifter].interpptr);
					}
				#endif

				return;
			}
		}
	}

	/* is this a removeRoute? if so, its not found, and we SHOULD return here */
	if (RTR.adrem != 1) return;
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
	CRoutes[insert_here].routeFromNode = RTR.from;
	CRoutes[insert_here].fnptr = RTR.fromoffset;
	CRoutes[insert_here].isActive = FALSE;
	CRoutes[insert_here].tonode_count = 0;
	CRoutes[insert_here].tonodes = NULL;
	CRoutes[insert_here].len = RTR.length;
	CRoutes[insert_here].interpptr = (void (*)(void*))RTR.intptr;
	CRoutes[insert_here].direction_flag = RTR.scrdir;
	CRoutes[insert_here].extra = RTR.extra;

	if (RTR.to_count > 0) {
		if ((CRoutes[insert_here].tonodes =
			 (CRnodeStruct *) calloc(RTR.to_count, sizeof(CRnodeStruct))) == NULL) {
			fprintf(stderr, "CRoutes_Register: calloc failed to allocate memory.\n");
		} else {
			CRoutes[insert_here].tonode_count = RTR.to_count;
			#ifdef CRVERBOSE
				printf("CRoutes at %d to nodes: %s\n",
					   insert_here, RTR.tonode_str);
			#endif

			if ((buffer = strtok(RTR.tonode_str, token)) != NULL) {
				/* printf("\t%s\n", buffer); */
				to_ptr = &(CRoutes[insert_here].tonodes[0]);
				if (sscanf(buffer, "%u:%u",
						   (unsigned int*)&(to_ptr->routeToNode), &(to_ptr->foffset)) == 2) {
					#ifdef CRVERBOSE 
						printf("\tsscanf returned: %u, %u\n",
						  to_ptr->routeToNode, to_ptr->foffset);
					#endif
				}


				/* condition statement changed */
				buffer = strtok(NULL, token);
				for (to_counter = 1;
					 ((to_counter < RTR.to_count) && (buffer != NULL));
					 to_counter++) {
					to_ptr = &(CRoutes[insert_here].tonodes[to_counter]);
					if (sscanf(buffer, "%u:%u",
							   (unsigned int*)&(to_ptr->routeToNode), &(to_ptr->foffset)) == 2) {
						#ifdef CRVERBOSE 
							printf("\tsscanf returned: %u, %u\n",
								  to_ptr->routeToNode, to_ptr->foffset);
						#endif
					}
					buffer = strtok(NULL, token);
				}
			}
		}
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
			printf ("%u %u %u direction %d, len %d extra %d : ",CRoutes[shifter].routeFromNode, CRoutes[shifter].fnptr,
				CRoutes[shifter].interpptr, CRoutes[shifter].direction_flag, CRoutes[shifter].len, CRoutes[shifter].extra);
			for (insert_here = 0; insert_here < CRoutes[shifter].tonode_count; insert_here++) {
				printf (" to: %u %u",CRoutes[shifter].tonodes[insert_here].routeToNode,
							CRoutes[shifter].tonodes[insert_here].foffset);
			}
			printf ("\n");
		}
	#endif
}

#ifdef DEBUG_VALIDNODE
/* only if DEBUG_VALIDNODE is defined; helps us find memory/routing problems */
void mark_event_check (struct X3D_Node *from, unsigned int totalptr, char *fn, int line) {
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

void mark_event (struct X3D_Node *from, unsigned int totalptr) {
	int findit;

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
		CRoutes[findit].isActive=TRUE;
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
void mark_script (uintptr_t num) {

	#ifdef CRVERBOSE 
		printf ("mark_script - script %d has been invoked\n",num);
	#endif
	scr_act[num]= TRUE;
}


/********************************************************************

gatherScriptEventOuts - at least one script has been triggered; get the
eventOuts for this script

FIXME XXXXX =  can we do this without the string conversions?

********************************************************************/

void gatherScriptEventOuts(uintptr_t actualscript) {
	int route;
	unsigned int fptr;
	unsigned int tptr;
	unsigned len;
 	void * tn;
	void * fn;

	/* temp for sscanf retvals */

	int fromalready=FALSE;	 /* we have already got the from value string */
	int touched_flag=FALSE;
	unsigned int to_counter;
	CRnodeStruct *to_ptr = NULL;

	/* go through all routes, looking for this script as an eventOut */

	/* do we have any routes yet? - we can gather events before any routes are made */
	if (!CRoutes_Initiated) return;

	/* this script initialized yet? We make sure that on initialization that the Parse Thread
	   does the initialization, once it is finished parsing. */
	if (ScriptControl[actualscript]._initialized!=TRUE) {
		/* printf ("waiting for initializing script %d at %s:%d\n",actualscript, __FILE__,__LINE__); */
		return;
	}

	/* routing table is ordered, so we can walk up to this script */
	route=1;

	#ifdef CRVERBOSE
	printf ("routing table looking, looking at %x and %x\n",(uintptr_t)(CRoutes[route].routeFromNode), actualscript); 
	#endif

	while ((uintptr_t)(CRoutes[route].routeFromNode) < actualscript) route++;
	while ((uintptr_t)(CRoutes[route].routeFromNode) == actualscript) {
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
			touched_flag = get_valueChanged_flag(fptr,actualscript);
		}

		if (touched_flag!= 0) {
			/* get some easy to use pointers */
			for (to_counter = 0; to_counter < CRoutes[route].tonode_count; to_counter++) {
				to_ptr = &(CRoutes[route].tonodes[to_counter]);
				tn = to_ptr->routeToNode;
				tptr = to_ptr->foffset;

				#ifdef CRVERBOSE 
					printf ("%s script %d VALUE CHANGED! copy value and update %d\n",JSparamnames[fptr].name,actualscript,tn);
				#endif

				/* eventOuts go to VRML data structures */
				setField_javascriptEventOut(X3D_NODE(tn),tptr,JSparamnames[fptr].type, len, 
					CRoutes[route].extra, ScriptControl[actualscript].cx);

				/* tell this node now needs to redraw */
				markScriptResults(tn, tptr, route, to_ptr->routeToNode);
			}
		}

		/* unset the touched flag */
		resetScriptTouchedFlag (actualscript, fptr);
		route++;

		/* REMOVE_ROOT(ScriptControl[actualscript].cx,global_return_val); */
	}

	#ifdef CRVERBOSE 
		printf ("%f finished  gatherScriptEventOuts loop\n",TickTime);
	#endif
}



void sendScriptEventIn(uintptr_t num) {
	unsigned int to_counter;
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
			to_ptr = &(CRoutes[num].tonodes[to_counter]);



			/* this script initialized yet? We make sure that on initialization that the Parse Thread
			   does the initialization, once it is finished parsing. */
			if (ScriptControl[(uintptr_t)to_ptr->routeToNode]._initialized!=TRUE) {
				/* printf ("waiting for initializing script %d at %s:%d\n",(uintptr_t)to_ptr->routeToNode, __FILE__,__LINE__); */
				return;
			}

			/* get the value from the VRML structure, in order to propagate it to a script */

			/* mark that this script has been active SCRIPTS ARE INTEGER NUMBERS */
			mark_script((uintptr_t)to_ptr->routeToNode);
			getField_ToJavascript(num,to_ptr->foffset);

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
void propagate_events() {
	int havinterp;
	int counter;
	int to_counter;
	CRnodeStruct *to_ptr = NULL;

		#ifdef CRVERBOSE
		printf ("\npropagate_events start\n");
		#endif

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
						printf("event %u %u sent something", CRoutes[counter].routeFromNode, CRoutes[counter].fnptr);
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
							memcpy((void *)((uintptr_t)to_ptr->routeToNode + to_ptr->foffset),
								   (void *)((uintptr_t)CRoutes[counter].routeFromNode + CRoutes[counter].fnptr),
								   (unsigned)CRoutes[counter].len);
						} else {
							/* this is a Multi*node, do a specialized copy. eg, Tiny3D EAI test will
							   trigger this */
							#ifdef CRVERBOSE
							printf ("in croutes, mmc len is %d\n",CRoutes[counter].len);
							#endif

							Multimemcpy ((void *)((uintptr_t)to_ptr->routeToNode + to_ptr->foffset),
								 (void *)((uintptr_t)CRoutes[counter].routeFromNode + CRoutes[counter].fnptr),
								 CRoutes[counter].len);
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
		for (counter =0; counter <= max_script_found_and_initialized; counter++) {
			/* 
				printf ("msf %d c %d\n",max_script_found, counter);
				printf ("script type %d\n",ScriptControl[counter].thisScriptType);
			*/

			gatherScriptEventOuts (counter);

		}

	} while (havinterp==TRUE);

	/* now, go through and clean up all of the scripts */
	for (counter =0; counter <= max_script_found_and_initialized; counter++) {
		if (scr_act[counter]) {
			scr_act[counter] = FALSE;
			CLEANUP_JAVASCRIPT((JSContext *)ScriptControl[counter].cx);
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
		if (ScriptControl[counter].eventsProcessed == 0) {
			ScriptControl[counter].eventsProcessed = (uintptr_t) JS_CompileScript(
				(JSContext *) ScriptControl[counter].cx,
				(JSObject *) ScriptControl[counter].glob,
				"eventsProcessed()", strlen ("eventsProcessed()"),
				"compile eventsProcessed()", 1);

		}

		if (!JS_ExecuteScript((JSContext *) ScriptControl[counter].cx,
                                (JSObject *) ScriptControl[counter].glob,
				(JSScript *) ScriptControl[counter].eventsProcessed, &retval)) {
			printf ("can not run eventsProcessed() for script %d thread %u\n",counter,(unsigned int)pthread_self());
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

	for (counter = 0; counter < rTr; counter++) {
		actually_do_CRoutes_Register (counter);
	}
	rTr = 0;

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

void getSpecificRoute (int routeNo, uintptr_t *fromNode, int *fromOffset, 
		uintptr_t *toNode, int *toOffset) {
        CRnodeStruct *to_ptr = NULL;


	if ((routeNo <1) || (routeNo >= CRoutes_Count)) {
		*fromNode = 0; *fromOffset = 0; *toNode = 0; *toOffset = 0;
	}
/*
	printf ("getSpecificRoute, fromNode %d fromPtr %d tonode_count %d\n",
		CRoutes[routeNo].routeFromNode, CRoutes[routeNo].fnptr, CRoutes[routeNo].tonode_count);
*/
		*fromNode = (uintptr_t) CRoutes[routeNo].routeFromNode;
		*fromOffset = CRoutes[routeNo].fnptr;
	/* there is not a case where tonode_count != 1 for a valid route... */
	if (CRoutes[routeNo].tonode_count != 1) {
		printf ("huh? tonode count %d\n",CRoutes[routeNo].tonode_count);
		*toNode = 0; *toOffset = 0;
		return;
	}

	/* get the first toNode,toOffset */
        to_ptr = &(CRoutes[routeNo].tonodes[0]);
        *toNode = (uintptr_t) to_ptr->routeToNode;
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
void Multimemcpy (void *tn, void *fn, int multitype) {
	unsigned int structlen;
	unsigned int fromcount, tocount;
	void *fromptr, *toptr;

	struct Multi_Vec3f *mv3ffn, *mv3ftn;

	#ifdef CRVERBOSE 
		printf ("Multimemcpy, copying structures %d %d type %d\n",tn,fn,multitype); 
	#endif

	/* copy a complex (eg, a MF* node) node from one to the other
	   the following types are currently found in VRMLNodes.pm -

		 -1  is a Multi_Color 
		 -10 is a Multi_Node
		 -12 is a SFImage
		 -13 is a Multi_String
		 -14 is a Multi_Float
		 -15 is a Multi_Rotation
		 -16 is a Multi_Int32
		 -18 is a Multi_Vec2f
		 -19 is a Multi_Vec3f
		 -20 is a Multi_Vec3d
	*/

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
		case -1: {structlen = sizeof (struct SFColor); break; }
		case -10: {structlen = sizeof (unsigned int); break; }
		case -12: {structlen = sizeof (unsigned int); break; } 
		case -13: {structlen = sizeof (unsigned int); break; }
		case -14: {structlen = sizeof (float); break; }
		case -15: {structlen = sizeof (struct SFRotation); break;}
		case -16: {structlen = sizeof (int); break;}
		case -18: {structlen = sizeof (struct SFVec2f); break;}
		case -19: {structlen = sizeof (struct SFColor); break;} /* This is actually SFVec3f - but no struct of this type */
		case -20: {structlen = sizeof (struct SFVec3d); break;} 
		default: {
			 /* this is MOST LIKELY for an EAI handle_Listener call - if not, it is a ROUTING problem... */
			/* printf("WARNING: Multimemcpy, don't handle type %d yet\n", multitype);  */
			structlen=0;
			return;
		}
	}


	/* free the old data, if there is old data... */
	FREE_IF_NZ (mv3ftn->p);

	/* MALLOC the toptr */
	mv3ftn->p = (struct SFColor *)MALLOC (structlen*fromcount);
	toptr = (void *)mv3ftn->p;

	/* tell the recipient how many elements are here */
	mv3ftn->n = fromcount;

	#ifdef CRVERBOSE 
		printf ("Multimemcpy, fromcount %d tocount %d fromptr %d toptr %d\n",fromcount,tocount,fromptr,toptr); 
	#endif

	/* and do the copy of the data */
	memcpy (toptr,fromptr,structlen * fromcount);
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
		RESET_TOUCHED_TYPE_A(SFImage)
		RESET_TOUCHED_TYPE_A(SFColor)
		RESET_TOUCHED_TYPE_A(SFColorRGBA)
		RESET_TOUCHED_TYPE_MF_A(MFRotation,SFRotation)
		RESET_TOUCHED_TYPE_MF_A(MFNode,SFNode)
		RESET_TOUCHED_TYPE_MF_A(MFVec2f,SFVec2f)
		RESET_TOUCHED_TYPE_MF_A(MFVec3f,SFVec3f)
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

