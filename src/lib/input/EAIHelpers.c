/*
=INSERT_TEMPLATE_HERE=

$Id: EAIHelpers.c,v 1.23 2009/05/07 17:01:24 crc_canada Exp $

Small routines to help with interfacing EAI to Daniel Kraft's parser.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h" 
#include "../world_script/jsUtils.h"
/*
#include "../world_script/jsNative.h"
#include "../world_script/JScript.h"
*/

#include "../main/headers.h"

#include "../x3d_parser/Bindable.h"
#include "../scenegraph/Collision.h"
#include "../scenegraph/quaternion.h"

#include "EAIheaders.h"
#include "SensInterps.h"

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/CScripts.h"
#include "../world_script/fieldSet.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CParse.h"
#include "../vrml_parser/CProto.h"

#include "../x3d_parser/X3DParser.h"

#include "EAIHelpers.h"


/*

NOTE - originally, when perl parsing was used, there used to be a perl
NodeNumber, and a memory location. Now, there is only a memory location...

Interface functions, for PROTO field access. From an email from Daniel:

Access fields:
size_t protoDefiniton_getFieldCount(protoDef)
struct ProtoFieldDecl* protoDefinition_getFieldByNum(protoDef, index)

Properties of struct ProtoFieldDecl:
indexT protoFieldDecl_getType(fdecl)
indexT protoFieldDEcl_getAccessType(fdecl)

Get name:
indexT protoFieldDecl_getIndexName(fdecl)
const char* protoFieldDecl_getStringName(lexer, fdecl)

Default value:
union anyVrml protoFieldDecl_getDefaultValue(fdecl)
this union contains fields for every specific type, see CParseGeneral.h for exact structure.

This struct contains both a node and an ofs field.

************************************************************************/




/************************************************************************************************/
/*												*/
/*			EAI NODE TABLE								*/
/*												*/
/************************************************************************************************/
/* keep a list of node requests, so that we can keep info on each node */
#define MAXFIELDSPERNODE 20
#define MAX_EAI_SAVED_NODES 1000

static int lastNodeRequested = 0;

/* store enough stuff in here so that each field can be read/written, no matter whether it is a 
   PROTO, SCRIPT, or regular node. */



struct EAINodeParams {
	struct X3D_Node* thisFieldNodePointer;	/* ok, if this is a PROTO expansion, points to actual node */
	int fieldOffset;
	int datalen;
	int typeString;
	int scripttype;
	char *invokedPROTOValue; 	/* proto field value on invocation (default, or supplied) */
};

struct EAINodeIndexStruct {
	struct X3D_Node*	actualNodePtr;
	int 			nodeType; /* EAI_NODETYPE_STANDARD =regular node, 
					     EAI_NODETYPE_PROTO = PROTO, 
					     EAI_NODETYPE_SCRIPT = script */
	struct EAINodeParams 	params[MAXFIELDSPERNODE];
	int    maxparamindex;
};
static struct  EAINodeIndexStruct *EAINodeIndex = NULL;




/* get an actual memory pointer to field, assumes both node has passed ok check */
char *getEAIMemoryPointer (int node, int field) {
	char *memptr;

	/* we CAN NOT do this with a script */
	if (EAINodeIndex[node].nodeType == EAI_NODETYPE_SCRIPT) {
		ConsoleMessage ("EAI error - getting EAIMemoryPointer on a Script node");
		return NULL;
	}

	/* use the memory address associated with this field, because this may have changed for proto invocations */
	memptr = (char *)EAINodeIndex[node].params[field].thisFieldNodePointer;
	memptr += EAINodeIndex[node].params[field].fieldOffset;
	/* printf ("getEAIMemoryPointer, nf %d:%d, node %u offset %d total %u\n",
		node,field,getEAINodeFromTable(node,field), EAINodeIndex[node].params[field].fieldOffset, memptr);
	*/

	return memptr;
}

/* get the parameters during proto invocation. Might not ever have been ISd */
static char * getEAIInvokedValue(int node, int field) {
	/* make sure we are a PROTO */
	if (EAINodeIndex[node].nodeType != EAI_NODETYPE_PROTO) {
		ConsoleMessage ("getting EAIInvokedValue on a node that is NOT a PROTO");
		return NULL;
	}
	
	return EAINodeIndex[node].params[field].invokedPROTOValue;
}


/* return the actual field offset as defined; change fieldHandle into an actual value */
int getEAIActualOffset(int node, int field) {
	return EAINodeIndex[node].params[field].fieldOffset;
}

/* returns node type - see above for definitions */
int getEAINodeTypeFromTable(int node) {
	return EAINodeIndex[node].nodeType;
}

/* return a registered node. If index==0, return NULL */
struct X3D_Node *getEAINodeFromTable(int index, int field) {
	if (index==0) return NULL;
	if (index>lastNodeRequested) {
		printf ("internal EAI error - requesting %d, highest node %d\n",
			index,lastNodeRequested);
		return NULL;
	}

	/* do we want the (possibly) PROTO Definition node, or the node that
	   is tied to the exact field? Only in PROTO expansions will they be different,
	   when possibly an IS'd field is within a sub-node of the PROTO */

	if (field <0) return EAINodeIndex[index].actualNodePtr;

	/* go and return the node associated directly with this field */
	/* printf ("getEAINodeFromTable, asking for field %d of node %d\n",field,index); */
	return EAINodeIndex[index].params[field].thisFieldNodePointer;
}

/* return an index to a node table. return value 0 means ERROR!!! */
int registerEAINodeForAccess(struct X3D_Node* myn) {
	int ctr;
	int mynindex = 0;

	if (eaiverbose) printf ("registerEAINodeForAccess - myn %u\n",myn);
	if (myn == NULL) return -1;

	if (EAINodeIndex == NULL) EAINodeIndex = MALLOC(sizeof (struct EAINodeIndexStruct) * MAX_EAI_SAVED_NODES);

	for (ctr=1; ctr<=lastNodeRequested; ctr++) {
		if (EAINodeIndex[ctr].actualNodePtr == myn) {
			if (eaiverbose) printf ("registerEAINodeForAccess - already got node\n");
			mynindex = ctr;
			break;
		}
	}

	/* did we find this node already? */
	if (mynindex == 0) {
		lastNodeRequested++;
		if (lastNodeRequested == MAX_EAI_SAVED_NODES) {
			ConsoleMessage ("EAI node table overflow - recompile with MAX_EAI_SAVED_NODES larger");
			lastNodeRequested = 0;
		}
		mynindex = lastNodeRequested;
		EAINodeIndex[mynindex].actualNodePtr = myn;
		EAINodeIndex[mynindex].maxparamindex = -1;

		/* save the node type; either this is a EAI_NODETYPE_SCRIPT, EAI_NODETYPE_PROTO, or EAI_NODETYPE_STANDARD */
		if (myn->_nodeType == NODE_Script) EAINodeIndex[mynindex].nodeType = EAI_NODETYPE_SCRIPT;
		else if ((myn->_nodeType == NODE_Group) & (X3D_GROUP(myn)->FreeWRL__protoDef != 0)) EAINodeIndex[mynindex].nodeType = EAI_NODETYPE_PROTO;
		else EAINodeIndex[mynindex].nodeType = EAI_NODETYPE_STANDARD;
	}

	if (eaiverbose) printf ("registerEAINodeForAccess returning index %d nt %s, internal type %d\n",mynindex,
				stringNodeType(myn->_nodeType),EAINodeIndex[mynindex].nodeType);
	return mynindex;
}


/***************************************************************************************************/

/* this is like EAI_GetNode, but is just for the rootNode of the scene graph */
int EAI_GetRootNode(void) {
	return registerEAINodeForAccess(rootNode);
}


/* get a node pointer in memory for a node. Return the node pointer, or NULL if it fails */
int EAI_GetNode(const char *str) {

	struct X3D_Node * myNode;

	if (eaiverbose) {
		printf ("EAI_GetNode - getting %s\n",str);
	}	
	
	/* Try to get X3D node name */
	myNode = X3DParser_getNodeFromName(str);
	if (myNode == NULL) {
		/* Try to get VRML node name */
		myNode = parser_getNodeFromName(str);
	}

	if (myNode != NULL) 
		return registerEAINodeForAccess(myNode);
	return FALSE;
}


int mapToKEYWORDindex (indexT pkwIndex) {
	if (pkwIndex == PKW_inputOutput) return KW_inputOutput;
	if (pkwIndex == PKW_inputOnly) return KW_inputOnly;
	if (pkwIndex == PKW_outputOnly) return KW_outputOnly;
	if (pkwIndex == PKW_initializeOnly) return KW_initializeOnly;
	return 0;
}

/* in this proto expansion, just go and get the expanded node/field IF POSSIBLE */
static int changeExpandedPROTOtoActualNode(int cNode, struct X3D_Node **np, char **fp, int direction) {
	struct ProtoDefinition *myProtoDecl;
	struct ProtoFieldDecl *thisIndex;
	char thisID[2000];
	
	/* first, is this node a PROTO? We look at the actual table to determine if it is special or not */
	if (getEAINodeTypeFromTable(cNode) != EAI_NODETYPE_PROTO) {
		return TRUE;
	}

	/* yes, it is a PROTO */
	if (eaiverbose) {
		printf ("changeExpanded - looking for field %s in node...\n",*fp); 
	}

	myProtoDecl = X3D_GROUP(*np)->FreeWRL__protoDef;
	if (eaiverbose) {
		printf ("and, the proto name is %s\n",myProtoDecl->protoName);
	}

	/* make up the name of the Metadata field associated with this one */
	if (strlen(*fp)>1000) return FALSE;

	sprintf (thisID,"PROTO_%u_%s",myProtoDecl,*fp);

	if (eaiverbose) printf ("looking for name :%s:\n",thisID);

	*np = parser_getNodeFromName(thisID);
	if ((*np) == 0) return FALSE;

	if (eaiverbose) {
		printf ("np is %u\n",*np);
		printf ("and, found node %u type %s\n",*np, stringNodeType((*np)->_nodeType));
	}

	/* change the fieldName, depending on the direction */
        /* see if this is an input or output request from nodes =0, tonodes = 1 */
	if (direction == 0) *fp = "valueChanged"; else *fp = "setValue";

	return TRUE;
}



/* get the type of a node; node must exist in table 
 	input:	
	cNode = handle for node pointer into memory - if not valid, this routine returns everything as zeroes
	fieldString =  - eg, "addChildren"
	accessMethod = "eventIn", "eventOut", "field" or...???

	returns:
	cNodePtr = C node pointer;
	fieldOffset = offset;
	dataLen = data len;
	typeString = mapFieldTypeToEAItype (ctype);
	scripttype = 0 - meaning, not to/from a javascript. (see CRoutes.c for values and more info)
*/

void EAI_GetType (int cNode,  char *inputFieldString, char *accessMethod, 
		uintptr_t *cNodePtr, uintptr_t *fieldOffset,
		uintptr_t *dataLen, uintptr_t *typeString,  unsigned int *scripttype, int *accessType) {

	struct X3D_Node* nodePtr = getEAINodeFromTable(cNode,-1);
	char *fieldString = inputFieldString;
	int myField;
	int ctype;
	int myProtoIndex;
	int myFieldOffs;
	int maxparamindex = 0;
	char *invokedValPtr = NULL;  /* for PROTOs - invocation value */
	int myScriptType = EAI_NODETYPE_STANDARD;
	int direction;
	struct X3D_Node* protoBaseNode;
	int isProtoExpansion = FALSE;


	/* see if this is an input or output request from nodes =0, tonodes = 1 */
	direction=0;
	if (strncmp(accessMethod,"in",strlen("in")) == 0) direction=1;
	else if (strncmp(accessMethod,"eventIn",strlen("eventIn"))==0) direction=1;

	/*if (direction==0) {
		printf ("EAI_GetType, this is a FROM route (%s)\n",accessMethod);
	} else {
		printf ("EAI_GetType, this is a TO route (%s)\n",accessMethod);
	} */
		

	if (eaiverbose) {
		printf ("call to EAI_GetType, cNode %d fieldString :%s: accessMethod %s\n",cNode,fieldString,accessMethod);
	}

	/* is this a valid C node? if so, lets just get the info... */
	if ((cNode == 0) || (cNode > lastNodeRequested)) {
		printf ("THIS IS AN ERROR! CNode is zero!!!\n");
		*cNodePtr = 0; *fieldOffset = 0; *dataLen = 0; *typeString = 0; *scripttype=0; *accessType=KW_eventIn;
		return;
	}

	if (eaiverbose) {
		printf ("start of EAI_GetType, this is a valid C node %d\n",nodePtr);
		printf ("	of string type %s\n",stringNodeType(nodePtr->_nodeType)); 
	}	

	myFieldOffs = -999;

	/* is this a field of the actual base type? */
	/* did not find the field as an ISd field - is this a field of the actual X3D base node?? */
	/* mimic POSSIBLE_PROTO_EXPANSION(nodePtr,protoBaseNode); */

        if (nodePtr == NULL) protoBaseNode = NULL; 
        else {if (X3D_NODE(nodePtr)->_nodeType == NODE_Group) { 
                if (X3D_GROUP(nodePtr)->children.n>0) { 
                        protoBaseNode = X3D_GROUP(nodePtr)->children.p[0]; 
                } else protoBaseNode = NULL; 
        } else protoBaseNode = nodePtr; };

	/* is this a proto expansion? */
	if (X3D_NODE(nodePtr)->_nodeType == NODE_Group) {
		if (X3D_GROUP(nodePtr)->FreeWRL__protoDef != 0) {
			isProtoExpansion = TRUE;
		}
	}

	if (isProtoExpansion) {
		/* this possibly is an expanded PROTO?, change the nodePtr and fieldString around */
		if (!changeExpandedPROTOtoActualNode (cNode, &nodePtr, &fieldString,direction)) {
			ConsoleMessage ("Did NOT find field :%s: in PROTO expansion",fieldString);
		}
	} else {
		if (eaiverbose) printf ("EAI_GetType - no, this is NOT a proto node\n");
	}

	if (nodePtr == NULL) {
		if (isProtoExpansion)
		ConsoleMessage ("error looking up field :%s: a PROTO Definition\n", fieldString);
		else
		ConsoleMessage ("error looking up field :%s: in an unknown node\n", fieldString);
		return;			
	}

	if (eaiverbose) {
		printf ("node here is %u\n",nodePtr);
		printf ("ok, going to try and find field :%s: in a node of type :%s:\n",fieldString,stringNodeType(nodePtr->_nodeType));
	}

	/* try finding it, maybe with a "set_" or "changed" removed */
	myField = findRoutedFieldInFIELDNAMES(nodePtr,fieldString,direction);

	if (eaiverbose) printf ("EAI_GetType, for field %s, myField is %d\n",fieldString,myField);

	/* find offsets, etc */
       	findFieldInOFFSETS(nodePtr->_nodeType, myField, &myFieldOffs, &ctype, accessType);

	if (eaiverbose) {
		printf ("EAI_GetType, after changeExpandedPROTOtoActualNode, C node %d\n",nodePtr);
		printf ("	of string type %s\n",stringNodeType(nodePtr->_nodeType)); 
	}	



	if (eaiverbose) printf ("EAI_GetType, after findFieldInOFFSETS, have myFieldOffs %d, ctype %d, accessType %d \n",myFieldOffs, ctype, *accessType); 

	/* is this a Script, or just an invalid field?? */ 
	if (myFieldOffs <= 0) {
        	int i;

		/* is this a Script node? */
		if (nodePtr->_nodeType == NODE_Script) {
			if (eaiverbose)
				printf ("EAI_GetType, node is a Script node...\n");
			struct Shader_Script* myScript = X3D_SCRIPT(nodePtr)->__scriptObj;
			myScriptType = EAI_NODETYPE_SCRIPT;

        		for (i = 0; i !=  vector_size(myScript->fields); ++i) {
        		        struct ScriptFieldDecl* sfield = vector_get(struct ScriptFieldDecl*, myScript->fields, i);
				
				if (eaiverbose)
				printf ("   field %d,  name %s type %s (type %s accessType %d (%s), indexName %d, stringType %s)\n",
						i,
						sfield->name, 
						sfield->type, 
						stringFieldtypeType(fieldDecl_getType(sfield->fieldDecl)),
						fieldDecl_getAccessType(sfield->fieldDecl),
						stringPROTOKeywordType(fieldDecl_getAccessType(sfield->fieldDecl)),
						fieldDecl_getIndexName(sfield->fieldDecl), 
						fieldDecl_getStringName(globalParser->lexer,sfield->fieldDecl)
				);
				
				
				if (strcmp(fieldString,sfield->name) == 0) {
					/* call JSparamIndex to get a unique index for this name - this is used for ALL
					   script access, whether from EAI or not */
					if(eaiverbose)
					printf ("found it at index, %d but returning JSparamIndex %d\n",i,JSparamIndex(sfield->name, sfield->type)); 

					myFieldOffs = JSparamIndex(sfield->name, sfield->type);
					/* switch from "PKW" to "KW" types */
					*accessType = mapToKEYWORDindex(fieldDecl_getAccessType(sfield->fieldDecl));
					ctype = findFieldInFIELDTYPES(sfield->type);
					break;
				}
			}

		} else {
			if (nodePtr != NULL)
			printf ("EAI_GetType, warning: field :%s: not not found in node of type :%s:\n",fieldString,stringNodeType(nodePtr->_nodeType));
			else
			printf ("EAI_GetType, warning: field :%s: not not found in node with pointer of NULL\n",fieldString);
		}

	}

	/* save these indexes */
	EAINodeIndex[cNode].maxparamindex++;
	maxparamindex = EAINodeIndex[cNode].maxparamindex;
	if (maxparamindex >= MAXFIELDSPERNODE) {
		maxparamindex = MAXFIELDSPERNODE-1;
		EAINodeIndex[cNode].maxparamindex = maxparamindex;
		printf ("recompile with MAXFIELDSPERNODE increased in value\n");
	}
	EAINodeIndex[cNode].params[maxparamindex].fieldOffset = myFieldOffs;
	EAINodeIndex[cNode].params[maxparamindex].datalen = returnRoutingElementLength(ctype);
	EAINodeIndex[cNode].params[maxparamindex].typeString = mapFieldTypeToEAItype(ctype);
	EAINodeIndex[cNode].params[maxparamindex].scripttype = myScriptType;
	EAINodeIndex[cNode].params[maxparamindex].invokedPROTOValue = invokedValPtr;

	/* has the node type changed, maybe because of a PROTO expansion? */
	if (EAINodeIndex[cNode].actualNodePtr != nodePtr) {
		/* printf ("iEAI_GetType, node pointer changed, using new node pointer\n"); */
		EAINodeIndex[cNode].params[maxparamindex].thisFieldNodePointer= nodePtr;
	} else {
		/* printf ("EAI_GetType, node is same as parent node\n"); */
		EAINodeIndex[cNode].params[maxparamindex].thisFieldNodePointer= EAINodeIndex[cNode].actualNodePtr;
	}

	/* 
	printf ("end of GetType, orig nodeptr %u, now %u\n",EAINodeIndex[cNode].actualNodePtr, nodePtr);
	printf ("end of GetType, now, EAI node type %d\n",EAINodeIndex[cNode].nodeType);
	*/

	*fieldOffset = (uintptr_t) maxparamindex; 	/* the entry into this field array for this node */
	*dataLen = EAINodeIndex[cNode].params[maxparamindex].datalen;	/* data len */
	*typeString = EAINodeIndex[cNode].params[maxparamindex].typeString; /* data type in EAI type */
	*scripttype =EAINodeIndex[cNode].params[maxparamindex].scripttype;
	*cNodePtr = cNode;	/* keep things with indexes */
}

	
char *EAI_GetTypeName (unsigned int uretval) {
	printf ("HELP::EAI_GetTypeName %d\n",uretval);
	return "unknownType";
}


int SAI_IntRetCommand (char cmnd, const char *fn) {
	printf ("HELP::SAI_IntRetCommand, %c, %s\n",cmnd,fn);
	return 0;
}

char * SAI_StrRetCommand (char cmnd, const char *fn) {
	printf ("HELP::SAI_StrRetCommand, %c, %s\n",cmnd,fn);
	return "iunknownreturn";
}

unsigned int EAI_GetViewpoint(const char *str) {
	printf ("HELP::EAI_GetViewpoint %s\n",str);
	return 0;
}



/* we have a GETVALUE command coming in */
void handleEAIGetValue (char command, char *bufptr, char *buf, int repno) {
	struct X3D_Node *myNode;
	int nodeIndex, fieldIndex, length;
	char ctmp[4000];
	int retint;

	if (eaiverbose) printf ("GETVALUE %s \n",bufptr);

	/* format: ptr, offset, type, length (bytes)*/
	retint=sscanf (bufptr, "%d %d %c %d", &nodeIndex,&fieldIndex,ctmp,&length);
	myNode = getEAINodeFromTable(nodeIndex, fieldIndex);

	/* if myNode is NULL, we have an error, baby */
	if (myNode == NULL) {
		printf ("handleEAIGetValue - node does not exist!\n");
		return;
	}

	
	/* printf ("handleEAIGetValue, node %u, type %s\n",myNode, stringNodeType(myNode->_nodeType)); */

	/* is the pointer a pointer to a PROTO?? If so, then the getType did not find
	an actual field (an IS'd field??) in a proto expansion for us.  We have to 
	go through, as the offset will be the index in the PROTO field for us to get
	the value for */

	if (EAINodeIndex[nodeIndex].params[fieldIndex].invokedPROTOValue != NULL) {
		sprintf (buf,"RE\n%f\n%d\n%s",TickTime,repno,getEAIInvokedValue(nodeIndex,fieldIndex));	
	} else {
		EAI_Convert_mem_to_ASCII (repno,"RE",mapEAItypeToFieldType(ctmp[0]),getEAIMemoryPointer(nodeIndex,fieldIndex), buf);
	}
}


/* this is a debugging function */
char *eaiPrintCommand (char command) {

	switch (command) {

		case GETNODE: return ("GETNODE");
		case GETEAINODETYPE: return ("GETEAINODETYPE");
		case SENDCHILD: return ("SENDCHILD");
		case SENDEVENT: return ("SENDEVENT");
		case GETVALUE: return ("GETVALUE");
		case GETFIELDTYPE: return ("GETFIELDTYPE");
		case REGLISTENER: return ("REGLISTENER");
		case ADDROUTE: return ("ADDROUTE");
		case REREADWRL: return ("REREADWRL");
		case DELETEROUTE: return ("DELETEROUTE");
		case GETNAME: return ("GETNAME");
		case GETVERSION: return ("GETVERSION");
		case GETCURSPEED: return ("GETCURSPEED");
		case GETFRAMERATE: return ("GETFRAMERATE");
		case GETURL: return ("GETURL");
		case REPLACEWORLD: return ("REPLACEWORLD");
		case LOADURL: return ("LOADURL");
		case VIEWPOINT: return ("VIEWPOINT");
		case CREATEVS: return ("CREATEVS");
		case CREATEVU: return ("CREATEVU");
		case STOPFREEWRL: return ("STOPFREEWRL");
		case UNREGLISTENER: return ("UNREGLISTENER");
		case GETRENDPROP: return ("GETRENDPROP");
		case GETENCODING: return ("GETENCODING");
		case CREATENODE: return ("CREATENODE");
		case CREATEPROTO: return ("CREATEPROTO");
		case UPDNAMEDNODE: return ("UPDNAMEDNODE");
		case REMNAMEDNODE: return ("REMNAMEDNODE");
		case GETPROTODECL: return ("GETPROTODECL");
		case UPDPROTODECL: return ("UPDPROTODECL");
		case REMPROTODECL: return ("REMPROTODECL");
		case GETFIELDDEFS: return ("GETFIELDDEFS");
		case GETNODEDEFNAME: return ("GETNODEDEFNAME");
		case GETROUTES: return ("GETROUTES");
		case GETNODETYPE: return ("GETNODETYPE");
		case MIDIINFO: return ("MIDIINFO");
		case MIDICONTROL: return ("MIDICONTROL");
		default:{} ;
	}
	return "unknown command...";
}
