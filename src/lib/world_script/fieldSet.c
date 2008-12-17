/*
=INSERT_TEMPLATE_HERE=

$Id: fieldSet.c,v 1.6 2008/12/17 18:38:12 crc_canada Exp $

???

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../main/Snapshot.h"
#include "../scenegraph/Collision.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"
#include "../input/SensInterps.h"
#include "../input/EAIHelpers.h"
#include "../x3d_parser/Bindable.h"

#include "CScripts.h"
#include "jsUtils.h"
#include "jsNative.h"
#include "jsVRMLClasses.h"
#include "fieldSet.h"
#include "fieldGet.h"


/*******************************************************************

A group of routines to SET a field in memory - in the FreeWRL
scene graph. 

Different methods are used, depending on the format of the call.

*********************************************************************/

/* copy new scanned in data over to the memory area in the scene graph. */
uintptr_t Multi_Struct_memptr (int type, void *memptr) {
	struct Multi_Vec3f *mp;
	uintptr_t retval;

	/* is this a straight copy, or do we have a struct to send to? */
	/* now, some internal reps use a structure defined as:
	   struct Multi_Vec3f { int n; struct SFColor  *p; };
	   so, we have to put the data in the p pointer, so as to
	   not overwrite the data. */

	retval = (uintptr_t) memptr;

	switch (type) {
		case FIELDTYPE_MFInt32:
		case FIELDTYPE_MFFloat:
		case FIELDTYPE_MFRotation:
		case FIELDTYPE_MFVec3f:
		case FIELDTYPE_MFColorRGBA:
		case FIELDTYPE_MFColor:
		case FIELDTYPE_MFVec2f:
			mp = (struct Multi_Vec3f*) memptr;
			retval = (uintptr_t) (mp->p);

		default: {}
		}
	return retval;
}


/* set a field; used in JavaScript, and in the Parser VRML parser 

	fields are:
		ptr:		pointer to a node (eg, X3D_Box)
		field:		string field name (eg, "size")
		value:		string of value (eg "2 2 2");

This is used mainly in parsing */

void setField_fromJavascript (struct X3D_Node *node, char *field, char *value) {
	int foffset;
	int coffset;
	int ctype;
	int ctmp;

	struct X3D_Group *group;

	#ifdef SETFIELDVERBOSE
	printf ("\nsetField_fromJavascript, node %d field %s value %s\n", node, field, value);
	#endif
	
	/* is this a valid field? */
	foffset = findRoutedFieldInFIELDNAMES(node,field,1);	
	if (foffset < 0) {
		ConsoleMessage ("field %s is not a valid field of a node %s",field,stringNodeType(node->_nodeType));
		printf ("field %s is not a valid field of a node %s\n",field,stringNodeType(node->_nodeType));
		return;
	}

	/* get offsets for this field in this nodeType */
	#ifdef SETFIELDVERBOSE
	printf ("getting nodeOffsets for type %s field %s value %s\n",stringNodeType(node->_nodeType),field,value); 
	#endif

	findFieldInOFFSETS(NODE_OFFSETS[node->_nodeType], foffset, &coffset, &ctype, &ctmp);

	#ifdef SETFIELDVERBOSE
	printf ("so, offset is %d, type %d value %s\n",coffset, ctype, value);
	#endif

	if (coffset <= 0) {
		printf ("setField_fromJavascript, trouble finding field %s in node %s\n",field,stringNodeType(node->_nodeType));
		printf ("is this maybe a PROTO?? if so, it will be a Group node with FreeWRL__protoDef set to the pointer\n");
		if (node->_nodeType == NODE_Group) {
			group = (struct X3D_Group *)node;
			printf ("it IS a group...\n");
			if (group->FreeWRL__protoDef) {
				printf ("and, this is a PROTO...have to go through PROTO defs to get to it\n");
			}
		}
	}

	Parser_scanStringValueToMem(node, coffset, ctype, value);
}


/* and incoming EAI event has come in, and the destination is an inputOnly field of a script.
   Make It So. This mimics the routing function "getField_ToJavascript" except that we do not
   have a routing entry for the from address and size and type, so we have to do this by hand.
*/

unsigned int setField_FromEAI_ToScript(uintptr_t tonode, int toname,
		int datatype, void *data, unsigned datalen) {

	#ifdef SETFIELDVERBOSE
	printf ("doing setField_FromEAI_ToScript, for script %u, nameIndex %u, type %s\n",tonode, toname, stringFieldtypeType(datatype));
	#endif

        switch (datatype) {
        case FIELDTYPE_SFBool:
        case FIELDTYPE_SFFloat:
        case FIELDTYPE_SFTime:
        case FIELDTYPE_SFInt32:
        case FIELDTYPE_SFString:
                set_one_ECMAtype (tonode, toname, datatype, data, datalen);
                break;
        case FIELDTYPE_SFColor:
        case FIELDTYPE_SFNode:
        case FIELDTYPE_SFVec2f:
        case FIELDTYPE_SFVec3f:
        case FIELDTYPE_SFVec3d:
        case FIELDTYPE_SFRotation:
		set_one_MultiElementType (tonode, toname, data, datalen);
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
		set_one_MFElementType (tonode, toname, datatype, data, datalen);
                break;
        default : {
                printf("WARNING: setField_FromEAI_ToScript,  type %s not handled yet\n",
			stringFieldtypeType(datatype));
                }
        }

	return TRUE;
}



/* an incoming EAI/CLASS event has come in, convert the ASCII characters
 * to an internal representation, and act upon it */

unsigned int setField_FromEAI (char *ptr) {
	unsigned char nt;
	int nodetype;
	int nodeIndex, fieldIndex;
	uintptr_t nodeptr;
	uintptr_t offset;
	unsigned int scripttype;

	uintptr_t memptr;

	int valIndex;
	struct Multi_Color *tcol;

	int len, elemCount;
	int retint; 			/* used to get return value of sscanf */
	char myBuffer[6000];

	#ifdef SETFIELDVERBOSE
	printf ("\nsetField_FromEAI, string :%s:\n",ptr);
	#endif
	
	/* we have an event, get the data properly scanned in from the ASCII string, and then
		friggin do it! ;-) */

	/* node type */
	while (*ptr==' ')ptr++;
	nt = *ptr; ptr++;
	nodetype = mapEAItypeToFieldType(nt);

	/* blank space */
	ptr++;

	/* nodeptr, offset */
	retint=sscanf (ptr, "%d %d %d",&nodeIndex, &fieldIndex, &scripttype);
	if (retint != 3) ConsoleMessage ("setField_FromEAI: error reading 3 numbers from the string :%s:\n",ptr);

	while ((*ptr) > ' ') ptr++; 	/* node ptr */
	while ((*ptr) == ' ') ptr++;	/* inter number space(s) */
	while ((*ptr) > ' ') ptr++;	/* node offset */
	while ((*ptr) == ' ') ptr++;	/* inter number space(s) */
	while ((*ptr) > ' ') ptr++;	/* script type */

	#ifdef SETFIELDVERBOSE
	{
	
		struct X3D_Node *np;
		/* get the actual node pointer from this index */
		np = getEAINodeFromTable(nodeIndex);

		 printf ("EAI_SendEvent, type %s, nodeptr (index %d) %u offset %d script type %d ",
				 stringFieldtypeType(nodetype),nodeIndex, np->_nodeType, fieldIndex, scripttype);
		printf ("np->_nodeType %s\n",stringNodeType(np->_nodeType));

		if (np->_nodeType == NODE_Script) {
			printf ("setField_FromEAI - sending to a script node!\n");
		}

		if ((np->_nodeType == NODE_Group) && (X3D_GROUP(np)->FreeWRL__protoDef != 0)) {
			printf ("setField_FromEAI, sending to a PROTO!\n");
		}
		}
	#endif

	/* We have either a event to a memory location, or to a script. */
	/* the field scripttype tells us whether this is true or not.   */

	if (scripttype != NODE_Script) {
		memptr = (uintptr_t) getEAIMemoryPointer (nodeIndex,fieldIndex);
	} else { memptr = 0; }

	offset = getEAIActualOffset(nodeIndex, fieldIndex);
	nodeptr = (uintptr_t) getEAINodeFromTable(nodeIndex);

	/* now, we are at start of data. */

	/* lets go to the first non-blank character in the string */
	while (*ptr == ' ') ptr++;
	#ifdef SETFIELDVERBOSE 
	printf ("setField_FromEAI EAI_SendEvent, event string now is :%s:\n",ptr);
	#endif

	/* is this a MF node, that has floats or ints, and the set1Value method is called? 	*/
	/* check out the java external/field/MF*java files for the string "ONEVAL "		*/
	if (strncmp("ONEVAL ",ptr, strlen("ONEVAL ")) == 0) {
		ptr += strlen ("ONEVAL ");

		/* find out which element the user wants to set - that should be the next number */
		while (*ptr==' ')ptr++;
		retint=sscanf (ptr,"%d",&valIndex);
		if (retint != 1) ConsoleMessage ("setField_FromEAI: error reading 1 numbers from the string :%s:\n",ptr);
		while (*ptr>' ')ptr++; /* past the number */
		while (*ptr==' ')ptr++;

		/* lets do some bounds checking here. */
		tcol = (struct Multi_Color *) memptr;
		if (tcol->n <= valIndex) {
			printf ("Error, setting 1Value of %d, max in scenegraph is %d\n",valIndex,tcol->n);
			return FALSE;
		}


		/* if this is a struct Multi* node type, move the actual memory pointer to the data */
		memptr = Multi_Struct_memptr(nodetype, (void *) memptr);

		/* and index into that array; we have the index, and sizes to worry about 	*/

		memptr += valIndex * returnElementLength(nodetype) *  returnElementRowSize(nodetype);

		/* and change the nodetype to reflect this change */
		nodetype = convertToSFType(nodetype);
	}

	/* This switch statement is almost identical to the one in the Javascript
	   code (check out CFuncs/CRoutes.c), except that explicit Javascript calls
	   are impossible here (this is not javascript!) */


	/* convert the ascii string into an internal representation */
	/* this will return '0' on failure */
	len = ScanValtoBuffer(&elemCount, nodetype, ptr , myBuffer, sizeof(myBuffer));

	/* an error in ascii to memory conversion happened */
	if (len == 0) {
		printf ("EAI_SendEvent, conversion failure\n");
		return( -1 );
	}

	/* if this is a script, lets do an inputOnly event, as one would with routing */
	if (scripttype == NODE_Script) {
		struct Shader_Script * sp;

		/* we send along the script number, not the node pointer */
		sp = (struct Shader_Script *) (X3D_SCRIPT(nodeptr)->__scriptObj);

		setField_FromEAI_ToScript(sp->num,offset,nodetype,myBuffer,len);
	} else {
		/* now, do the memory copy */
		/* if we have a positive len, then, do a straight copy */

		if (len > 0) {
			/* SFStrings are, of course, different... */
			if (nodetype == FIELDTYPE_SFString) {
				printf ("ScanValtoBuffer, have SFString to verify - :%s:\n",(unsigned char*)myBuffer);
{


uintptr_t *xx;
struct Uni_String *svptr;

                        /* get the pointer to the string, do this in a couple of steps... */
                        xx= (uintptr_t *) memptr;
                        svptr = (struct Uni_String *) *xx;

printf ("and old string was :%s:\n",                        (unsigned char *)svptr->strptr);
				verify_Uni_String(svptr, (char *)myBuffer);
}
				
			} else {
				/* a straight copy over */
				SetMemory(nodetype,(void *)memptr,(void *)myBuffer,len);
			}
		} else {
			/* if len < 0, it is "wierd". See ScanValtoBuffer
			 * for accurate return values. */
			if (len == -1) 
				getEAI_MFStringtype ((struct Multi_String *)myBuffer,
							(struct Multi_String *)memptr);
			 else getEAI_ONE_MFStringtype ((struct Multi_String *)myBuffer,
                                                        (struct Multi_String *)memptr,
							len); 
		}

		/* if this is a geometry, make it re-render.
		   Some nodes (PROTO interface params w/o IS's)
		   will have an offset of zero, and are thus not
		   "real" nodes, only memory locations
		*/

		if (offset > 0) update_node ((void *)nodeptr);

		/* if anything uses this for routing, tell it that it has changed */
		MARK_EVENT (X3D_NODE(nodeptr),offset);
	}
	return TRUE;

#undef SETFIELDVERBOSE

}

void setField_javascriptEventOut(struct X3D_Node *tn,unsigned int tptr,  int fieldType, unsigned len, int extraData, uintptr_t cx) {
        int ival;
        double tval;
        float fl[4];
	JSContext *scriptContext;
	char *memptr;
        JSString *strval; /* strings */
	char *strp;


	/* set up a pointer to where to put this stuff */
	memptr = (char *)tn;
	memptr += tptr;

	/* not all files know what a JSContext is, so we just pass it around as a uintptr_t type */
	scriptContext = (JSContext *) cx;

	#ifdef SETFIELDVERBOSE
	strval = JS_ValueToString(scriptContext, JSglobal_return_val);
       	strp = JS_GetStringBytes(strval);
	printf ("start of setField_javascriptEventOut, to %d:%d = %d, fieldtype %d string %s\n",tn, tptr, memptr, fieldType, strp);
	#endif


#define GETJSVAL_TYPE_A(thistype,field) \
		case FIELDTYPE_##thistype: { \
			/* printf ("doing TYPEA memcpy to %u, from %u, len %d\n",(void *)memptr, (void *) &(((thistype##Native *)JSSFpointer)->field),len); */ \
			memcpy ((void *)memptr, (void *) &(((thistype##Native *)JSSFpointer)->field),len); \
			break; \
		} 

#define GETJSVAL_TYPE_MF_A(MFtype,SFtype) \
		case FIELDTYPE_##MFtype: {getJSMultiNumType (scriptContext, (struct Multi_Vec3f *)memptr,FIELDTYPE_##SFtype); break;}

	switch (fieldType) {
                        GETJSVAL_TYPE_A(SFRotation,v)
                        GETJSVAL_TYPE_A(SFNode,handle)
                        GETJSVAL_TYPE_A(SFVec2f,v)
                        GETJSVAL_TYPE_A(SFVec3f,v)
                        GETJSVAL_TYPE_A(SFColor,v)
                        GETJSVAL_TYPE_A(SFColorRGBA,v)

                        GETJSVAL_TYPE_MF_A(MFRotation,SFRotation)
                        /* GETJSVAL_TYPE_MF_A(MFNode,SFNode) */
                        GETJSVAL_TYPE_MF_A(MFVec2f,SFVec2f)
                        GETJSVAL_TYPE_MF_A(MFVec3f,SFVec3f)
                        GETJSVAL_TYPE_MF_A(MFColor,SFColor)
                        GETJSVAL_TYPE_MF_A(MFColorRGBA,SFColorRGBA)


		case FIELDTYPE_SFInt32: 
		case FIELDTYPE_SFBool:	{	/* SFBool */
			if (!JS_ValueToInt32(scriptContext, JSglobal_return_val,&ival)) {
				printf ("error\n");
				ival=0;
			}
			memcpy ((void *)memptr, (void *)&ival,len);
			break;
		}

		case FIELDTYPE_SFTime: {
			if (!JS_ValueToNumber(scriptContext, JSglobal_return_val,&tval)) tval=0.0;
			memcpy ((void *)memptr, (void *)&tval,len);
			break;
		}

		case FIELDTYPE_SFFloat: {
			if (!JS_ValueToNumber(scriptContext, JSglobal_return_val,&tval)) tval=0.0;
			/* convert double precision to single, for X3D */
			fl[0] = (float) tval;
			memcpy ((void *)memptr, (void *)fl,len);
			break;
		}

		case FIELDTYPE_SFImage: {
			/* the string should be saved as an SFImage */
			strval = JS_ValueToString(scriptContext, JSglobal_return_val);
	        	strp = JS_GetStringBytes(strval);

			Parser_scanStringValueToMem(tn, tptr, FIELDTYPE_SFImage, strp);
			break;
		}

		case FIELDTYPE_SFString: {
			struct Uni_String *ms;
			uintptr_t *newptr;

			strval = JS_ValueToString(scriptContext, JSglobal_return_val);
	        	strp = JS_GetStringBytes(strval);

			/* copy the string over, delete the old one, if need be */
			/* printf ("fieldSet SFString, tn %d tptr %d offset from struct %d\n",
				tn, tptr, offsetof (struct X3D_TextureCoordinateGenerator, mode)); */
			newptr = (uintptr_t *)memptr;
			ms = (struct Uni_String*) *newptr;
			verify_Uni_String (ms,strp);
			break;
		}


			/* a series of Floats... */
		case FIELDTYPE_MFFloat: {getJSMultiNumType (scriptContext, (struct Multi_Vec3f *)memptr,FIELDTYPE_SFFloat); break;}
		case FIELDTYPE_MFInt32: {getJSMultiNumType (scriptContext, (struct Multi_Vec3f *)memptr,FIELDTYPE_SFInt32); break;}
		case FIELDTYPE_MFTime: {getJSMultiNumType (scriptContext, (struct Multi_Vec3f *)memptr,FIELDTYPE_SFTime); break;}
		case FIELDTYPE_MFNode: {
				strval = JS_ValueToString(scriptContext, JSglobal_return_val);
	        		strp = JS_GetStringBytes(strval);
				getMFNodetype (strp,(struct Multi_Node *)memptr,X3D_NODE(tn),extraData); break;
		}
		case FIELDTYPE_MFString: {
			getMFStringtype (scriptContext, (jsval *)JSglobal_return_val,(struct Multi_String *)memptr);
			break;
		}


		default: {	printf("WARNING: unhandled from type %s\n", FIELDTYPES[fieldType]);
		}
	}

	#ifdef SETFIELDVERBOSE
	printf ("done setField_javascriptEventOut\n");
	if (fieldType == FIELDTYPE_MFInt32) {
		printf ("setField_javascriptEventOut, checking the pointers...\n");
		printf ("node type is %s\n",stringNodeType(X3D_NODE(tn)->_nodeType));
	}
	
	#endif
}

/* find the ASCII string name of this field of this node */
char *findFIELDNAMESfromNodeOffset(struct X3D_Node *node, int offset) {
	int* np;
	if (node == 0) return "unknown";

	np = (int *) NODE_OFFSETS[node->_nodeType];  /* it is a const int* type */
	np++;  /* go to the offset field */

	while ((*np != -1) && (*np != offset)) np +=4;
	
	if (*np == -1) return "fieldNotFound";
	
	/* go back to the field name */
	np --;
	return ((char *) FIELDNAMES[*np]);
}

/* go through the generated table FIELDTYPES, and find the int of this string, returning it, or -1 on error 
	or if it is an "internal" field */
int findFieldInARR(const char* field, const char** arr, size_t cnt)
{
	int x;
	int mystrlen;
	
	if (field == NULL) return -1;

	#ifdef SETFIELDVERBOSE
	if (field[0] == '_') {
		printf ("findFieldInFIELDNAMES - internal field %s\n",field);
	}
	#endif
	
	mystrlen = strlen(field);
	/* printf ("findFieldInFIELDNAMES, string :%s: is %d long\n",field,mystrlen);  */
	for (x=0; x!=cnt; ++x) {
		if (strlen(arr[x]) == mystrlen) {
			if (strcmp(field, arr[x])==0) return x;
		} 
	}
	return -1;

}
#define DEF_FINDFIELD(arr) \
 int findFieldIn##arr(const char* field) \
 { \
  return findFieldInARR(field, arr, arr##_COUNT); \
 }
DEF_FINDFIELD(FIELDNAMES)
DEF_FINDFIELD(FIELD)
DEF_FINDFIELD(EXPOSED_FIELD)
DEF_FINDFIELD(EVENT_IN)
DEF_FINDFIELD(EVENT_OUT)
DEF_FINDFIELD(KEYWORDS)
DEF_FINDFIELD(PROTOKEYWORDS)
DEF_FINDFIELD(NODES)
DEF_FINDFIELD(PROFILES)
DEF_FINDFIELD(COMPONENTS)
DEF_FINDFIELD(FIELDTYPES)
DEF_FINDFIELD(X3DSPECIAL)
DEF_FINDFIELD(GEOSPATIAL)

/* lets see if this node has a routed field  fromTo  = 0 = from node, anything else = to node */
/* returns the FIELDNAMES index. */
/* for user-fields, the additional check is skipped */
int findRoutedFieldInARR (struct X3D_Node * node, const char *field, int fromTo,
  const char** arr, size_t cnt, BOOL user) {
	int retval;
	char mychar[200];
	int a,b,c;
	
	retval = -1;

#define FIELDCHECK(fld) \
	if (retval >= 0) { \
	  if (user) return retval; \
	  int fieldNamesIndex = findIndexInFIELDNAMES(retval, arr, cnt); \
	  if (fieldNamesIndex >= 0) { \
	    findFieldInOFFSETS (NODE_OFFSETS[node->_nodeType], fieldNamesIndex,\
	      &a, &b, &c); \
	    /* did this return any of the ints as != -1? */ \
	    /* printf ("     findRoutedField for field %s, nodetype %s is %d\n",  fld,stringNodeType(node->_nodeType),a); */ \
	    if (a >= 0) return retval;  /* found it! */ \
	  } \
	} 


	/* step try the field as is. */
	retval = findFieldInARR(field, arr, cnt);
	/* printf ("findRoutedField, field %s retval %d\n",field,retval); */ 
	FIELDCHECK (field)

	/* try ADDING the "set_" or "_changed" */
	strncpy (mychar, field, 100);
	if (fromTo != 0) {
		if (strlen(field) > strlen("set_"))
			retval=findFieldInARR(mychar+strlen("set_"), arr, cnt);
	} else {
		if (strlen(field) > strlen("_changed")) {
			mychar[strlen(field) - strlen("_changed")] = '\0';
			retval = findFieldInARR(mychar, arr, cnt);
		}
	}
	/* printf ("findRoutedField, mychar %s retval %d\n",mychar,retval); */
	FIELDCHECK (mychar)

	/* try ADDING the "set_" or "_changed"  some nodes have fields ending in "_changed" - maybe the
  	   user forgot about that? (eg, ProximitySensor) */
	if (fromTo != 0) {
		strcpy (mychar,"set_");
		strncat (mychar, field,100);
		retval=findFieldInARR(mychar+strlen("set_"), arr, cnt);
	} else {
		strncpy (mychar, field, 100);
		strcat (mychar,"_changed");
		retval = findFieldInARR(mychar, arr, cnt);
	}
	/* printf ("findRoutedField, mychar %s retval %d\n",mychar,retval);*/
	FIELDCHECK (mychar)


	return retval;
}
#define DEF_FINDROUTEDFIELD(arr) \
 int findRoutedFieldIn##arr(struct X3D_Node* node, const char* field, int fromTo) \
 { \
  return findRoutedFieldInARR(node, field, fromTo, arr, arr##_COUNT, FALSE); \
 }
DEF_FINDROUTEDFIELD(FIELDNAMES)
DEF_FINDROUTEDFIELD(EXPOSED_FIELD)
DEF_FINDROUTEDFIELD(EVENT_IN)
DEF_FINDROUTEDFIELD(EVENT_OUT)


/* go through the OFFSETS for this node, looking for field, and return offset, type, and kind */
void findFieldInOFFSETS(const int *nodeOffsetPtr, const int field, int *coffset, int *ctype, int *ckind) {
	int *x;

	x = (int *) nodeOffsetPtr;

	#ifdef SETFIELDVERBOSE
	printf ("findFieldInOffsets, comparing %d to %d\n",*x, field); 
	#endif

	while ((*x != field) && (*x != -1)) {
		x += 4;
	}
	if (*x == field) {
		x++; *coffset = *x; x++; *ctype = *x; x++; *ckind = *x; 

		#ifdef SETFIELDVERBOSE
		printf ("found field, coffset %d ctype %d ckind %d\n",*coffset, *ctype, *ckind); 
		#endif

		return;
	}
	if (*x == -1) {
		#ifdef  SETFIELDVERBOSE
		printf ("did not find field %d in OFFSETS\n",field);
		#endif

		*coffset = -1; *ctype = -1, *ckind = -1;
		return;
	}

}


/****************************************************************/
/* a Jscript is returning a Multi-number type; copy this from 	*/
/* the Jscript return string to the data structure within the	*/
/* freewrl C side of things.					*/
/*								*/
/* note - this cheats in that the code assumes that it is 	*/
/* a series of Multi_Vec3f's while in reality the structure	*/
/* of the multi structures is the same - so we "fudge" things	*/
/* to make this multi-purpose.					*/
/****************************************************************/
void getJSMultiNumType (JSContext *cx, struct Multi_Vec3f *tn, int eletype) {
	float *fl;
	int *il;
	double *dl;
	uintptr_t *nl;

	double dtmp;
	jsval mainElement;
	int len;
	int i;
	char *strp;
	int elesize;
	SFVec2fNative *sfvec2f;
	SFVec3fNative *sfvec3f;
	SFRotationNative *sfrotation;
	struct Uni_String * *ms;
	jsval myJSVal;


	/* get size of each element, used for MALLOCing memory  - eg, this will
	   be sizeof(float) * 3 for a SFColor */
	elesize = returnElementLength(eletype) * returnElementRowSize(eletype);

	/* rough check of return value */
	/* where did this come from? Was it from a script execution, or from an assignment from within a script?? */
	#ifdef SETFIELDVERBOSE
	printf ("getJSMultiNumType, JSCreate_global_return_val %u, JSglobal_return_val %u\n",JSCreate_global_return_val, JSglobal_return_val);
	#endif

	if (JSCreate_global_return_val!= INT_TO_JSVAL(0)) {
		myJSVal = JSCreate_global_return_val;
		JSCreate_global_return_val = INT_TO_JSVAL(0);

		#ifdef SETFIELDVERBOSE
		printf ("getJSMultiNumType: using JSCreate_global_return_val\n");
		#endif
	} else {
		#ifdef SETFIELDVERBOSE
		printf ("getJSMultiNumType: using JSglobal_return_val\n");
		#endif

		myJSVal = JSglobal_return_val;
	}

	if (!JSVAL_IS_OBJECT(myJSVal)) {
		printf ("getJSMultiNumType - did not get an object\n");
		return;
	}

	#ifdef SETFIELDVERBOSE
	printf ("getJSMultiNumType, tn %d dest has  %s size %d\n",tn,FIELDTYPES[eletype], elesize); 

	printf("getJSMulitNumType, node type of myJSVal is :");
	printJSNodeType (cx,myJSVal);
	#endif

	if (!JS_GetProperty(cx, (JSObject *)myJSVal, "length", &mainElement)) {
		printf ("JS_GetProperty failed for \"length\" in getJSMultiNumType\n");
		return;
	}
	len = JSVAL_TO_INT(mainElement);
	#ifdef SETFIELDVERBOSE
	printf ("getmuiltie length of grv is %d old len is %d\n",len,tn->n);
	#endif

	/* do we have to realloc memory? */
	if (len != tn->n) {

		tn->n = 0;
		/* yep... */
			/* printf ("old pointer %d\n",tn->p); */
		FREE_IF_NZ (tn->p);
		tn->p = (struct SFColor *)MALLOC ((unsigned)(elesize*len));

		#ifdef SETFIELDVERBOSE 
		printf ("MALLOCing memory for elesize %d len %d new pointer now is %d\n",elesize,len,tn->p);
		#endif

		/* if this is an MFString, we should set each element to a null string */
		if (eletype == FIELDTYPE_SFString) {
			#ifdef SETFIELDVERBOSE
			printf ("getJSMultiNumType, this is a MFString, so making tiny strings for now\n");
			#endif

			ms = (struct Uni_String * *) tn->p;
			for (i=0; i<len; i++) {
				*ms = newASCIIString ("(getJSMultiNumType null)");
				ms ++;
			}
		}
	}

	/* set these three up, but we only use one of them */
	fl = (float *) tn->p;
	il = (int *) tn->p;
	dl = (double *) tn->p;
	nl = (uintptr_t *) tn->p;
	ms = (struct Uni_String * *) tn->p;

	/* go through each element of the main array. */
	for (i = 0; i < len; i++) {
		if (!JS_GetElement(cx, (JSObject *)myJSVal, i, &mainElement)) {
			printf ("WARNING: JS_GetElement failed for %d in getJSMultiNumType\n",i);
			switch (eletype) {
			case FIELDTYPE_SFNode:
				*nl = 0; nl++; break;
			case FIELDTYPE_SFInt32: 
				*il=0; il++; break;
			case FIELDTYPE_SFTime: 
				*dl=0.0; dl++; break;
			case FIELDTYPE_SFFloat: 
				*fl = (float) 0.0;  fl++; break; 
			case FIELDTYPE_SFVec2f: 
				*fl = (float) 0.0;  fl++; *fl = (float) 0.0;  fl++; break; 
			case FIELDTYPE_SFVec3f:
	                case FIELDTYPE_SFColor: 
				*fl = (float) 0.0;  fl++; *fl = (float) 0.0;  fl++; *fl = (float) 0.0;  fl++; break; 
			case FIELDTYPE_SFRotation: 
				*fl = (float) 0.0;  fl++; *fl = (float) 0.0;  fl++; *fl = (float) 0.0;  fl++; *fl = (float) 0.0;  fl++; break; 
			case FIELDTYPE_SFString: 
	                        verify_Uni_String (*ms,"(empty value)"); ms++; break;
	
			default : {printf ("getJSMultiNumType unhandled eletype: %d\n",
					eletype);
				   return;
				}
			}
		} else {
			#ifdef SETFIELDVERBOSE
			JSString *_tmpStr;

	                _tmpStr = JS_ValueToString(cx, mainElement);
			strp = JS_GetStringBytes(_tmpStr);
	                printf ("sub element %d is \"%s\" \n",i,strp);  

			if (JSVAL_IS_OBJECT(mainElement)) printf ("sub element %d is an OBJECT\n",i);
			if (JSVAL_IS_PRIMITIVE(mainElement)) printf ("sub element %d is an PRIMITIVE\n",i);
			#endif
	
			/* code is pretty much same as SF* values in setField_javascriptEventOut */
			switch (eletype) {
			case FIELDTYPE_SFNode: {
				int xxx;
				if (RUNNINGONAMD64) printf ("FIELDTYPE_SFNode *may* not work on 64 bit computers...\n");
				if (!JS_ValueToInt32(cx, mainElement ,&xxx)) {
					printf ("error\n");
					*nl=0;
				}
				/* *nl = (uintptr_t * *)xxx; */
				*nl = (uintptr_t)xxx;
				nl++;
				break;
			}
			case FIELDTYPE_SFInt32: {
				if (!JS_ValueToInt32(cx, mainElement ,il)) {
					printf ("error\n");
					*il=0;
				}
				il++;
				break;
			}
			case FIELDTYPE_SFTime: {
				if (!JS_ValueToNumber(cx, mainElement ,dl)) *dl=0.0;
				dl++;
				break;
			}
			case FIELDTYPE_SFFloat: {
				if (!JS_ValueToNumber(cx, mainElement, &dtmp)) dtmp=0.0;
				/* convert double precision to single, for X3D */
				*fl = (float) dtmp;
				fl++;
				break;
			}
			case FIELDTYPE_SFVec2f: {
				if (JSVAL_IS_OBJECT(mainElement)) {
	                        	if ((sfvec2f = (SFVec2fNative *)JS_GetPrivate(cx, (JSObject *)mainElement)) == NULL) {
	                                	printf( "JS_GetPrivate failed for obj in setField_javascriptEventOut.\n");
	                                	return;
	                        	}
	                        	memcpy ((void *)fl, (void *)&(sfvec2f->v),2*sizeof(float));
					fl += 2;
				} else {
					/* we are working in a value that kind of exists, but is undefined */
					*fl = (float) 0.0; fl++; *fl = (float) 0.0; fl++;	
				}
	                        break;
			}
			case FIELDTYPE_SFVec3f:
	                case FIELDTYPE_SFColor: {       /* SFColor */
				if (JSVAL_IS_OBJECT(mainElement)) {
	                        	if ((sfvec3f = (SFVec3fNative *)JS_GetPrivate(cx, (JSObject *)mainElement)) == NULL) {
	                        	        printf( "JS_GetPrivate failed for obj in setField_javascriptEventOut.\n");
	                        	        return;
	                        	}
	                        	memcpy ((void *)fl, (void *)&(sfvec3f->v),3*sizeof(float));
					fl += 3;
				} else {
					/* we are working in a value that kind of exists, but is undefined */
					*fl = (float) 0.0; fl++;	
					*fl = (float) 0.0; fl++;	
					*fl = (float) 0.0; fl++;	
				}
	                        break;
			}
			case FIELDTYPE_SFRotation: {
				if (JSVAL_IS_OBJECT(mainElement)) {
	                        	if ((sfrotation = (SFRotationNative *)JS_GetPrivate(cx, (JSObject *)mainElement)) == NULL) {
	                        	        printf( "JS_GetPrivate failed for obj in setField_javascriptEventOut.\n");
	                        	        return;
	                        	}
	                        	memcpy ((void *)fl, (void *)&(sfrotation->v),4*sizeof(float));
					fl += 4;
				} else {
					/* we are working in a value that kind of exists, but is undefined */
					*fl = (float) 0.0; fl++;	*fl = (float) 0.0; fl++;	
					*fl = (float) 0.0; fl++;	*fl = (float) 0.0; fl++;	
				}
	                        break;
			}
	
			case FIELDTYPE_SFString: {
				JSString *strval;
	
	                        strval = JS_ValueToString(cx, mainElement);
	                        strp = JS_GetStringBytes(strval);
	
				#ifdef SETFIELDVERBOSE
				printf ("getJSMultiNumType, got string %s\n",strp); 
				#endif
	
	                        /* copy the string over, delete the old one, if need be */
	                        verify_Uni_String (*ms,strp);
				ms++;
	                        break;
			}
	
			default : {printf ("getJSMultiNumType unhandled eletype: %d\n",
					eletype);
				   return;
				}
			}
		}

	}
	#ifdef SETFIELDVERBOSE
	printf ("getJSMultiNumType, setting old length %d to length %d\n",tn->n, len);
	#endif

	tn->n = len;
}

/****************************************************************/
/* a script is returning a MFString type; add this to the C	*/
/* children field						*/
/****************************************************************/
void getMFStringtype (JSContext *cx, jsval *from, struct Multi_String *to) {
	int oldlen, newlen;
	jsval _v;
	JSObject *obj;
	int i;
	char *valStr, *OldvalStr;
	struct Uni_String **svptr;
	struct Uni_String **newp, **oldp;
	int count;

	JSString *strval; /* strings */

	oldlen = to->n;
	svptr = to->p;
	newlen=0;

	if (!JS_ValueToObject(cx, (jsval) from, &obj))
		printf ("JS_ValueToObject failed in getMFStringtype\n");

	if (!JS_GetProperty(cx, obj, "length", &_v)) {
		printf ("JS_GetProperty failed for \"length\" in getMFStringtype.\n");
        }

	newlen = JSVAL_TO_INT(_v);

	/* printf ("getMFStringType, newlen %d oldlen %d\n",newlen,oldlen); */


	/*  if we have to expand size of SV... */
	if (newlen > oldlen) {
		oldp = to->p; /* same as svptr, assigned above */
		to->n = newlen;
		to->p = (struct Uni_String**)MALLOC(newlen * sizeof(to->p));
		newp = to->p;

		/* copy old values over */
		for (count = 0; count <oldlen; count ++) {
			/*printf ("copying over element %d\n",count); */
			*newp = *oldp;
			newp++;
			oldp++;
		}

		/* zero new entries */
		for (count = oldlen; count < newlen; count ++) {
			/* make the new SV */
			*newp = (struct Uni_String *)MALLOC (sizeof (struct Uni_String));
			

			/* now, make it point to a blank string */
			*newp = newASCIIString("");
			newp ++;
		}
		FREE_IF_NZ (svptr);
		svptr = to->p;
	} else {
		/* possibly truncate this, but leave the memory alone. */
		to->n = newlen;
	}

	/* printf ("verifying structure here\n");
	for (i=0; i<(to->n); i++) {
		printf ("indx %d flag %x string :%s: len1 %d len2 %d\n",i,
				(svptr[i])->sv_flags,
	}
	printf ("done\n");
	*/


	for (i = 0; i < newlen; i++) {
		/* get the old string pointer */
		OldvalStr = svptr[i]->strptr;
		/* printf ("old string at %d is %s len %d\n",i,OldvalStr,strlen(OldvalStr)); */

		/* get the new string pointer */
		if (!JS_GetElement(cx, obj, i, &_v)) {
			fprintf(stderr,
				"JS_GetElement failed for %d in getMFStringtype\n",i);
			return;
		}
		strval = JS_ValueToString(cx, _v);
		valStr = JS_GetStringBytes(strval);

		/* printf ("new string %d is %s\n",i,valStr); */

		/*  if the strings are different... */
		if (strcmp(valStr,OldvalStr) != 0) {
			/* MALLOC a new string, of correct len for terminator */
			svptr[i] =  newASCIIString(valStr);
		}
	}
	/*
	printf ("\n new structure: %d %d\n",svptr,newlen);
	for (i=0; i<newlen; i++) {
		printf ("indx %d string :%s: len1 %d len2 %d\n",i,
				mypv->xpv_pv, mypv->xpv_cur,mypv->xpv_len);
	}
	*/
	
}


/************************************************************************/
/* a script is returning a MFNode type; add or remove this to the C	*/
/* children field							*/
/* note params - tn is the address of the actual field, parent is parent*/
/* structure								*/
/************************************************************************/

void getMFNodetype (char *strp, struct Multi_Node *tn, struct X3D_Node *parent, int ar) {
	uintptr_t newptr;
	int newlen;
	char *cptr;
	void *newmal;
	uintptr_t *tmpptr;

	/* is this 64 bit compatible? - unsure right now. */
	if (sizeof(void *) != sizeof (unsigned int))
		printf ("getMFNodetype - unverified that this works on 64 bit machines\n");

	#ifdef SETFIELDVERBOSE 
		printf ("getMFNodetype, %s ar %d\n",strp,ar);
		printf ("getMFNodetype, parent %d has %d nodes currently\n",tn,tn->n);
	#endif

	newlen=0;

	/* this string will be in the form "[ CNode addr CNode addr....]" */
	/* count the numbers to add  or remove */
	if (*strp == '[') { strp++; }
	while (*strp == ' ') strp++; /* skip spaces */
	cptr = strp;

	while (sscanf (cptr,"%ld",&newptr) == 1) {
		newlen++;
		/* skip past this number */
		while (isdigit(*cptr) || (*cptr == ',') || (*cptr == '-')) cptr++;
		while (*cptr == ' ') cptr++; /* skip spaces */
	}
	cptr = strp; /* reset this pointer to the first number */

	/* printf ("newlen HERE is %d\n",newlen); */

	/* create the list to send to the AddRemoveChildren function */
	newmal = MALLOC (newlen*sizeof(void *));
	tmpptr = (uintptr_t*)newmal;

	/* scan through the string again, and get the node numbers. */
	while (sscanf (cptr,"%ld", (uintptr_t *)tmpptr) == 1) {
		/* printf ("just scanned in %d, which is a %s\n",*tmpptr, 
			stringNodeType((X3D_NODE(*tmpptr))->_nodeType)); */

		/* skip past this number */
		while (isdigit(*cptr) || (*cptr == ',') || (*cptr == '-')) cptr++;
		while (*cptr == ' ') cptr++; /* skip spaces */
		tmpptr = (uintptr_t*) (tmpptr + sizeof (void *));
	}

	/* now, perform the add/remove */
	AddRemoveChildren (parent, tn, newmal, newlen, ar);
}


/* change a memory loction - do we need to do a simple copy, or
do we need to change a Structure type? */

void SetMemory (int type, void *destptr, void *srcptr, int len) {
	void *newptr;
	struct Multi_Vec3f *mp;


	/* is this a structure? If Multi_Struct_memptr returns a different
	   pointer, than it IS a struct {int n void *p} structure type. */

	/* printf ("start of SetMemory, len %d type %d\n",len,type);  */

	newptr = (void *)Multi_Struct_memptr(type, destptr);
	if (newptr != destptr) {
		mp = (struct Multi_Vec3f*) destptr;
		/* printf ("SetMemory was %d ",mp->n); */
		mp->n=0;
		FREE_IF_NZ(mp->p);
		mp->p = MALLOC(len);
		memcpy (mp->p,srcptr,len);
		mp->n = len /(returnElementLength(type)*returnElementRowSize(type));
		/* printf (" is %d\n ",mp->n); */
	} else {
		memcpy (destptr, srcptr, len);
	}
}


/****************************************************************/
/* a EAI client is returning a MFString type; add this to the C	*/
/* children field						*/
/****************************************************************/
void getEAI_ONE_MFStringtype (struct Multi_String *from, struct Multi_String *to, int len) {
	int oldlen, newlen;
	struct Uni_String **oldsvptr;
	struct Uni_String **newsvptr;

	newlen = len*-1 - 10;

	oldlen = to->n;
	oldsvptr = to->p;
	newsvptr = from->p;

	/* printf ("to->n %d from->n %d newlen %d\n",to->n, from->n, newlen); */
	if (to->n > newlen) {
		verify_Uni_String(to->p[newlen],newsvptr[0]->strptr);
	} else {
		ConsoleMessage ("EAI MFString - setting element %d too great, max %d\n",newlen, to->n);
	}
}

/****************************************************************/
/* a EAI client is returning a MFString type; add this to the C	*/
/* children field						*/
/****************************************************************/
void getEAI_MFStringtype (struct Multi_String *from, struct Multi_String *to) {
	int oldlen, newlen;
	int i;
	struct Uni_String **oldsvptr;
	struct Uni_String **newsvptr;
	struct Uni_String **newp, **oldp;
	int count;

	/* oldlen = what was there in the first place */
	/*  should be ok verifySVtype(from); */

	oldlen = to->n;
	oldsvptr = to->p;
	newlen= from->n;
	newsvptr = from->p;

	#ifdef SETFIELDVERBOSE
	printf ("getEAI_MFStringtype: old len %d new len %d\n",oldlen, newlen);
	#endif

	/* if we have to expand size of SV... */
	if (newlen > oldlen) {

		#ifdef SETFIELDVERBOSE
		printf ("getEAI_MFStringtype: have to expand...\n");
		#endif

		oldp = to->p; /* same as oldsvptr, assigned above */
		to->n = newlen;
		to->p =(struct Uni_String **) MALLOC(newlen * sizeof(to->p));
		newp = to->p;
		/* printf ("newp is %u, size %d\n",newp, newlen * sizeof(to->p)); */

		/*  copy old values over */
		for (count = 0; count <oldlen; count ++) {
			*newp = *oldp;
			newp++;
			oldp++;
		}

		/*  zero new entries */
		for (count = oldlen; count < newlen; count ++) {
			/* printf ("zeroing %d\n",count); */
			/* make the new SV */
			*newp = (struct Uni_String *)MALLOC (sizeof (struct Uni_String));

			/* now, make it point to a blank string */
			*newp = newASCIIString("");
			newp++;
		}
		FREE_IF_NZ (oldsvptr);
		oldsvptr = to->p;
	}

	for (i = 0; i < newlen; i++) {
		/* did the string change ?? */
		verify_Uni_String(oldsvptr[i],newsvptr[i]->strptr);
	}

	/* any old values to get rid of, because the string is now truncated? */
	for (i=newlen; i< oldlen; i++) {
		/* printf ("have to get rid of index %d, string %s\n",i,oldsvptr[i]->strptr); */
		FREE_IF_NZ(oldsvptr[i]->strptr);
		FREE_IF_NZ(oldsvptr[i]);
	}

	/* record the new len; possibly truncate the array */
	to->n = newlen;
}




/* take an ASCII string from the EAI or CLASS, and convert it into
   a memory block 

   This is not the whole command, just a convert this string to a 
   bunch of floats, or equiv. */

int ScanValtoBuffer(int *quant, int type, char *buf, void *memptr, int bufsz) {
	float *fp;
	int *ip;
	void *tmpbuf;
	int count;
	int len;
	int retint;	/* used for getting sscanf return val */
	char savedChar;
	
	/* pointers to cast memptr to*/
	float *flmem;

	/* pass in string in buf; memory block is memptr, size in bytes, bufsz */

	#ifdef SETFIELDVERBOSE
	printf("ScanValtoBuffer - memptr %d, buffer %s bufsz %d\n",memptr, buf,bufsz);
	#endif

	if (bufsz < 10) {
		printf ("cant perform conversion with small buffer\n");
		return (0);
	}

	switch (type) {
	    case FIELDTYPE_SFBool:	{	/* SFBool */
	    	if (strncasecmp(buf,"true",4)==0) {
		    *(int *)memptr = 1;
	    	} else {
		    *(int *)memptr = 0;
	    	}
		len = sizeof(int);
	    	break;
	    }

	    case FIELDTYPE_SFNode:
	    case FIELDTYPE_SFInt32: {
	    	retint=sscanf (buf,"%d",(int *)memptr);
		if (retint != 1) ConsoleMessage ("ScanValtoBuffer: can not read 1 floats from :%s:",buf);
		len = sizeof (int);
	    	break;
	    }
	    case FIELDTYPE_SFFloat: {
	    	retint=sscanf (buf,"%f",(float *)memptr);
		if (retint != 1) ConsoleMessage ("ScanValtoBuffer: can not read 1 floats from :%s:",buf);
		len = sizeof (float);
	    	break;
	    }

	    case FIELDTYPE_SFVec2f: {	/* SFVec2f */
		flmem = (float *)memptr;
	    	retint=sscanf (buf,"%f %f",&flmem[0], &flmem[1]);
		if (retint != 2) ConsoleMessage ("ScanValtoBuffer: can not read 2 floats from :%s:",buf);
		len = sizeof(float) * 2;
	    	break;
	    }

	    case FIELDTYPE_SFVec3f:
	    case FIELDTYPE_SFColor: {	/* SFColor */
		flmem = (float *)memptr;
	    	retint=sscanf (buf,"%f %f %f",&flmem[0],&flmem[1],&flmem[2]);
		len = sizeof(float) * 3;
		if (retint != 3) ConsoleMessage ("ScanValtoBuffer: can not read 3 floats from :%s:",buf);
	    	break;
	    }

	    case FIELDTYPE_SFColorRGBA:
	    case FIELDTYPE_SFRotation: {
		flmem = (float *)memptr;
	    	retint=sscanf (buf,"%f %f %f %f",&flmem[0],&flmem[1],&flmem[2],&flmem[3]);
		if (retint != 4) ConsoleMessage ("ScanValtoBuffer: can not read 4 floats from :%s:",buf);
		len = sizeof(float) * 4;
	    	break;
	    }

	    case FIELDTYPE_SFTime: {
		retint=sscanf (buf, "%lf", (double *)memptr);
		if (retint != 1) ConsoleMessage ("ScanValtoBuffer: can not read 1 floats from :%s:",buf);
		len = sizeof(double);
		break;
	    }

	    case FIELDTYPE_MFNode:
	    case FIELDTYPE_MFInt32:
	    case FIELDTYPE_MFTime:
	    case FIELDTYPE_MFColor:
	    case FIELDTYPE_MFColorRGBA:
	    case FIELDTYPE_MFVec3f:
	    case FIELDTYPE_MFFloat:
	    case FIELDTYPE_MFRotation:
	    case FIELDTYPE_MFVec2f: {
		/* first number is the "number of numbers". */

		/* scan to start of element count, and read it in.*/
		while (*buf==' ') buf++; 
		retint=sscanf (buf,"%d",quant); while (*buf>' ') buf++;
		if (retint != 1) ConsoleMessage ("ScanValtoBuffer: can not read 1 ints from :%s:",buf);

		/* how many elements per row of this type? */
		*quant = *quant * returnElementRowSize(type);

		/* so, right now, quant tells us how many numbers we will read in. */



		/* how long is each element in bytes of this type? */
		len = *quant * returnElementLength(type);

		  #ifdef SETFIELDVERBOSE
			printf ("bufsz is %d, len = %d quant = %d str: %s\n",bufsz, len, *quant,buf);
		  #endif
		  if (len > bufsz) {
			  printf ("Warning, MultiFloat too large, truncating to %d \n",bufsz);
			  len = bufsz;
		  }

		tmpbuf = (float *) MALLOC (len);
		fp = (float *) tmpbuf;
		ip = (int *) tmpbuf;
		for (count = 0; count < *quant; count++) {
			/* scan to start of number */
			while (*buf<=' ') buf++;

			/* scan in number */
			if ((type==FIELDTYPE_MFInt32) || (type==FIELDTYPE_MFNode)) {
				retint=sscanf (buf,"%d",ip);
			} else { 
				retint=sscanf (buf,"%f",fp);
			}
			if (retint != 1) ConsoleMessage ("ScanValtoBuffer: can not read 1 number from :%s:",buf);

			/* go to next number */
			while (*buf >' ') buf++;
			fp ++; ip++;
		}
			
		  /* now, copy over the data to the memory pointer passed in */
		  if(NULL != tmpbuf) memcpy (memptr,tmpbuf,len);
		  else perror("ScanValtoBuffer: tmpbuf NULL!");

		  /* free the memory MALLOC'd in Alberto's code */
		  FREE_IF_NZ (tmpbuf);
		  break;
	    }

	    case FIELDTYPE_MFString: {
		struct Uni_String ** newp;
		struct Multi_String *MFStringptr;
		int thisele, thissize, maxele;	/* used for reading in MFStrings*/


		/* return a Multi_String.						*/
		/*  struct Multi_String { int n; SV * *p; };				*/
		/*  buf will look like:							*/
		/*  	2  0;9:wordg.png  1;12:My"wordg.png				*/
		/*  	where 2 = max elements; 0;9 is element 0, 9 chars long...	*/
		/* OR, if a set1Value:							*/
		/* 	  -1 2:10:xxxxxxxxx						*/
		/* 	  where the 2 is the index.					*/

		MFStringptr = (struct Multi_String *)memptr;
		MFStringptr->n = 0;
		MFStringptr->p=NULL;


		/* scan to start of element count, and read it in.*/
		while (*buf==' ') buf++;
		retint=sscanf (buf,"%d",&maxele);
		if (retint != 1) ConsoleMessage ("ScanValtoBuffer: can not read 1 number from :%s:",buf);
		while (*buf!=' ') buf++;

		/* is this a set1Value, or a setValue */
		if (maxele == -1) {
			MFStringptr->n = 1;
			MFStringptr->p = (struct Uni_String **)MALLOC (sizeof(MFStringptr->p));
			newp = MFStringptr->p;

			/* is the range ok? for set1Value, we only replace, do not expand. */
			while (*buf==' ') buf++;
			retint=sscanf (buf,"%d;%d",&thisele,&thissize);
			if (retint != 2) ConsoleMessage ("ScanValtoBuffer: can not read 2 number from :%s:",buf);

			/* scan to start of string*/
			while (*buf!=':') buf++; buf++;

			/* replace the space at stringln with a 0 */
			buf += thissize; savedChar = *buf; *buf = 0; buf-=thissize;
			MFStringptr->p[0] = newASCIIString(buf);

			/* go to end of string */
			buf += thissize; *buf=savedChar; buf++;

			/* watch this return value: -1 means that it is a full string replace, so lets "code" this
			   one element to replace */
			len =  -thisele -10;
		} else {	
			/* make (and initialize) this MFString internal representation.*/
			MFStringptr->n = maxele;
			MFStringptr->p = (struct Uni_String **)MALLOC (maxele * sizeof(MFStringptr->p));
			newp = MFStringptr->p;
	
			/* scan through EAI string, extract strings, etc, etc.*/
			do {
				/* scan to start of element number*/
				/* make the new SV */
	
				while (*buf==' ') buf++;
				retint=sscanf (buf,"%d;%d",&thisele,&thissize);
				if (retint != 2) ConsoleMessage ("ScanValtoBuffer: can not read 2 number from :%s:",buf);
				/*printf ("this element %d has size %d\n",thisele,thissize);*/
	
				/* scan to start of string*/
				while (*buf!=':') buf++; buf++;
	
				/* replace the space at stringln with a 0 */
				buf += thissize; savedChar = *buf; *buf = 0; buf-=thissize;
				MFStringptr->p[thisele] = newASCIIString(buf);

				/* go to end of string */
				buf += thissize; *buf=savedChar; buf++;
	
				/* scan to next start of string, or end of line*/
				while (*buf==' ') buf++;
	
				/* point to next SV to fill*/
				newp++;
			} while (((int)*buf)>=32);
			len = -1;
		}

		/* return -length  to indicate that this is "wierd".*/

		break;
	   }

	case FIELDTYPE_SFImage:
	case FIELDTYPE_SFString: {
		int thissize;
		char savedChar;

		/* save this stuff to a global SV, rather than worrying about memory pointers */

		#ifdef SETFIELDVERBOSE
		printf ("ScanValtoBuffer: FIELDTYPE_SFString, string is %s, ptr %x %d\n",buf,memptr,memptr);
		#endif

		/* strings in the format "25:2 2 1 0xff 0x80 0x80 0xff" where 25 is the length */
		while (*buf == ' ') buf++;

		retint=sscanf (buf,"%d",&thissize);
		if (retint != 1) ConsoleMessage ("ScanValtoBuffer: can not read 1 number from :%s:",buf);
		/* printf ("this SFStr size %d \n",thissize);  */

		/* scan to start of string*/
		while (*buf!=':') buf++; buf++;

		/* replace the space at stringln with a 0 */
		buf += thissize; 
		savedChar = *buf;
		*buf=0;
		buf -= thissize;

		if (thissize >= bufsz) {
			ConsoleMessage ("EAI_SFSTRING, have to truncate, too long\n");
			buf[100] = '\0'; /* yes, this will cause problems */
		}
		strcpy (memptr, buf);

		/* return char to a space */
		buf += thissize;
		*buf= savedChar;

		len = thissize; /* might as well set it to something possibly useful later on */
		break;
	}
		
	  default: {
		printf("WARNING: unhandled CLASS from type %s\n", FIELDTYPES[type]);
		printf ("complain to the FreeWRL team.\n");
		printf ("(string is :%s:)\n",buf);
		return (0);
	    }
	}
	return (len);
}

/* Map the given index into arr to an index into FIELDNAMES or -1, if the
 * string in question isn't there. */
int findIndexInFIELDNAMES(int index, const char** arr, size_t arrCnt) {
  int i;

  /* If this is already FIELDNAMES, return index. */
  if(arr==FIELDNAMES)
    return index;

  /* Look for the string */
  for(i=0; i!=FIELDNAMES_COUNT; ++i) {
    if(!strcmp(FIELDNAMES[i], arr[index]))
      return i;
  }

  /* Not found */
  return -1;
}
