/*
=INSERT_TEMPLATE_HERE=

$Id: EAIHelpers.c,v 1.10 2008/12/29 21:42:00 crc_canada Exp $

Small routines to help with interfacing EAI to Daniel Kraft's parser.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h" /* point_XYZ */
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

Desination pointers:
size_t protoFieldDecl_getDestinationCount(fdecl)
struct OffsetPointer* protoFieldDecl_getDestination(fdecl, index)

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
uintptr_t *getEAIMemoryPointer (int node, int field) {
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

	return (uintptr_t *)memptr;
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

	if (EAINodeIndex == NULL) EAINodeIndex = MALLOC(sizeof (struct EAINodeIndexStruct) * MAX_EAI_SAVED_NODES);

	for (ctr=1; ctr<lastNodeRequested; ctr++) {
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

	if (eaiverbose) printf ("registerEAINodeForAccess returning index %d\n",mynindex);
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
	if (myNode != NULL) {
			return registerEAINodeForAccess(myNode);
	}

	/* Try to get VRML node name */
	return registerEAINodeForAccess(parser_getNodeFromName(str));
}


int mapToKEYWORDindex (indexT pkwIndex) {
	if (pkwIndex == PKW_inputOutput) return KW_inputOutput;
	if (pkwIndex == PKW_inputOnly) return KW_inputOnly;
	if (pkwIndex == PKW_outputOnly) return KW_outputOnly;
	if (pkwIndex == PKW_initializeOnly) return KW_initializeOnly;
	return 0;
}
/***************
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

Desination pointers:
size_t protoFieldDecl_getDestinationCount(fdecl)
struct OffsetPointer* protoFieldDecl_getDestination(fdecl, index)
*/

/*********************************************************************************

findFieldInPROTOOFFSETS General purpose PROTO field lookup function.

parameters:

	strct X3D_Node *myNode	a node pointer to an X3D_Node. This should be to an X3D_Group with the
				FreeWRL__protoDef field pointing to a valid data structure.

	char *myField		a string pointer to a field name to look up.

	uintptr_t *myNodeP	pointer to the X3D_Node* in memory of the last ISd field in this PROTO.
				returns 0 if not found.

	int *myOfs		offset to the field in the last ISd field memory structure. returns
				0 if not found.

	int *myType		returns a value, eg "EAI_SFFLOAT", or 0 if this field is not found.

	int *accessType 	returns one of KW_exposedField, KW_eventIn, KW_eventOut, KW_field, or 0 
				if field not found.

	char **invokedValue	a MALLOCd string containing the invocation value of this field

*********************************************************************************/
static void findFieldInPROTOOFFSETS (struct X3D_Node *myNode, char *myField, uintptr_t *myNodeP,
					int *myOfs, int *myType, int *accessType, char **invokedValue) {

	struct X3D_Group *group;
	struct ProtoDefinition *myProtoDecl;
	struct ProtoFieldDecl *thisIndex;


	/* set these values, so that we KNOW if we have found the correct field */
	*myType = 0;
	*myOfs = 0;
	*myNodeP = 0;

	#ifdef FF_PROTO_PRINT
	printf ("setField_method1, trouble finding field %s in node %s\n",myField,stringNodeType(myNode->_nodeType));
	printf ("is this maybe a PROTO?? if so, it will be a Group node with FreeWRL__protoDef set to the pointer\n");
	#endif

	if (myNode->_nodeType == NODE_Group) {
		group = (struct X3D_Group *)myNode;
		#ifdef FF_PROTO_PRINT
		printf ("it IS a group...\n"); 
		#endif

		if (group->FreeWRL__protoDef) {
			int tl = 1000;

			#ifdef FF_PROTO_PRINT
			printf ("and, this is a PROTO...have to go through PROTO defs to get to it\n");
			#endif

			myProtoDecl = (struct ProtoDefinition *) group->FreeWRL__protoDef;
			thisIndex = getProtoFieldDeclaration( globalParser->lexer, myProtoDecl, myField);

			/*
			printf ("	field name is %s\n",protoFieldDecl_getStringName(globalParser->lexer, thisIndex));
			printf ("	type is %d which is %s\n",protoFieldDecl_getType(thisIndex),
				stringFieldtypeType(protoFieldDecl_getType(thisIndex)));
			printf ("	Accesstype is %d as string %s\n",protoFieldDecl_getAccessType(thisIndex),
				stringPROTOKeywordType(protoFieldDecl_getAccessType(thisIndex)));
			printf ("	indexnameString %s\n",protoFieldDecl_getStringName(globalParser->lexer, thisIndex));
			*/

			char *newTl = MALLOC(1000);
     			newTl[0] = '\0';

                        /* printf ("next element is an IS \n"); */
                        /* tempEle = vector_get(struct ProtoElementPointer*, (*thisProto)->deconstructedProtoBody, i+2); */
                        /* printf ("ok, so IS of :%s: is :%s:\n",ele->stringToken, tempEle->stringToken); */

                        replaceProtoField(globalParser->lexer, myProtoDecl, myField,&newTl,&tl);
		
			/* possibly, if this is an MF* field, we will have brackets on the ends. Remove them */
			if (convertToSFType(protoFieldDecl_getType(thisIndex)) != protoFieldDecl_getType(thisIndex)) {
				char *charptr; 
				/* printf ("have to remove brackets from :%s:\n",newTl); */
				charptr = strchr(newTl,'['); if (charptr != NULL) *charptr = ' ';
				charptr = strrchr(newTl,']'); if (charptr != NULL) *charptr = ' ';
				/* printf ("now :%s:\n",newTl); */
			}

			/* slide things to right */
			if (strlen(newTl) >0) {
				while ((newTl[0] <=' ') && (newTl[0] != '\0')) {
					/* printf ("sliding to right\n"); */
					memmove (newTl, &newTl[1],strlen(newTl));
				}

			}
			printf ("TESTING - got replace as %s\n",newTl); 

			*invokedValue = newTl;
			*myType = mapFieldTypeToEAItype(protoFieldDecl_getType(thisIndex));
			*accessType = mapToKEYWORDindex(protoFieldDecl_getAccessType(thisIndex));
		}
	}
}


/* in this proto expansion, just go and get the expanded node/field IF POSSIBLE */
static void changeExpandedPROTOtoActualNode(int cNode, struct X3D_Node **np, char **fp) {
	struct ProtoDefinition *myProtoDecl;
	struct ProtoFieldDecl *thisIndex;
	int i,j;
	struct ProtoElementPointer *ele;
	int pMax;
	char thisID[200];
	
	/* first, is this node a PROTO? We look at the actual table to determine if it is special or not */
	if (getEAINodeTypeFromTable(cNode) != EAI_NODETYPE_PROTO) {
		return;
	}

	/* yes, it is a PROTO */
	/* printf ("changeExpanded - looking for field %s in node...\n",*fp); */

	myProtoDecl = X3D_GROUP(*np)->FreeWRL__protoDef;
	pMax = vector_size(myProtoDecl->deconstructedProtoBody);

	/* printf ("changeExpandedPROTOtoActualNode, protoDefNumber %d protoName %s\n",myProtoDecl->protoDefNumber, myProtoDecl->protoName); */

	/* go through, and look for the fieldname */
	for (i=0; i< pMax-2; i++) {
		ele = vector_get(struct ProtoElementPointer*, myProtoDecl->deconstructedProtoBody, i);

		/*
		printf ("PROTO - ele %d of %d ", i, pMax-1); 

		if (ele->isNODE != -1) printf ("isNODE - %s ",stringNodeType(ele->isNODE));
		if (ele->isKEYWORD != -1) printf ("isKEYWORD - %s ",stringKeywordType(ele->isKEYWORD));
		if (ele->terminalSymbol != -1) printf ("terminalSymbol '%c' ",ele->terminalSymbol);
		if (ele->stringToken != NULL) printf ("stringToken :%s: ",ele->stringToken);
		if (ele->fabricatedDef != -1) printf ("fabricatedDef %d",ele->fabricatedDef);
		printf ("\n");
		*/

		if (ele->stringToken != NULL) {
			if (strcmp(*fp,ele->stringToken)==NULL) {
				/* printf ("changeExpanded, found string token match at element %d (%s == %s) \n",i, *fp, ele->stringToken); */
		
				/* ok, so the next element must be an IS,right? */
				ele = vector_get(struct ProtoElementPointer*, myProtoDecl->deconstructedProtoBody, i+1);
				/* printf ("changeExpanded, next keyword is %d (KW_IS %d)  - better not be -1\n",ele->isKEYWORD, KW_IS); */

				if (ele->isKEYWORD != KW_IS) {
					ConsoleMessage ("changeExpandedPROTOtoActualNode, in PROTO, but keyword problem with kw %s",*fp);
					return;
				}

				/* and, after the IS, we should have the IS name, right? */
				ele = vector_get(struct ProtoElementPointer*, myProtoDecl->deconstructedProtoBody, i+2);
				/* printf ("and, the new IS keyword should be :%s:\n",ele->stringToken); */

				if (ele->stringToken == NULL) {
					ConsoleMessage ("changeExpandedPROTOtoActualNode, in PROTO, but IS problem with kw %s",*fp);
					return;
				}

				/* we have (possibly) a new field name, so just link to the IS'd field */
				*fp = ele->stringToken;

				/* ok, so we have the field,  lets go back and find the actual node that this field is part of */
				j = i;
				while ((j>=0) && (ele->isNODE == -1)) {
					j--;
					ele = vector_get(struct ProtoElementPointer*, myProtoDecl->deconstructedProtoBody, j);
				}

				/* printf ("back to element %d, isNODE %s\n",j, stringNodeType(ele->isNODE)); */

				/* we should have found a node, if not, we have an error */
				if (ele->isNODE == -1) {
					ConsoleMessage ("changeExpanedPROTOtoActualNode - node error");
					return;
				}

				/* printf ("so, we are looking for fabricatedDEf %d\n",ele->fabricatedDef); */
				sprintf (thisID,"%s%d_",FABRICATED_DEF_HEADER,ele->fabricatedDef);

				*np = parser_getNodeFromName(thisID);

				/* printf ("and, found node %u\n",*np); */
				return;

		
			}
		}

	}
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


	if (eaiverbose) {
		printf ("call to EAI_GetType, cNode %d fieldString %s accessMethod %s\n",cNode,fieldString,accessMethod);
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


	/* is this possibly an expanded PROTO? If so, change the nodePtr and fieldString around */
	changeExpandedPROTOtoActualNode (cNode, &nodePtr, &fieldString);
	if (eaiverbose) {
		printf ("EAI_GetType, after changeExpandedPROTOtoActualNode, C node %d\n",nodePtr);
		printf ("	of string type %s\n",stringNodeType(nodePtr->_nodeType)); 
	}	


	/* try finding it, maybe with a "set_" or "changed" removed */
	myField = findRoutedFieldInFIELDNAMES(nodePtr,fieldString,0);
	if (myField == -1) 
		myField = findRoutedFieldInFIELDNAMES(nodePtr,fieldString,1);

	/* printf ("EAI_GetType, for field %s, myField is %d\n",fieldString,myField); */


	/* find offsets, etc */
       	findFieldInOFFSETS((int *)NODE_OFFSETS[nodePtr->_nodeType], myField, &myFieldOffs, &ctype, accessType);

	/* printf ("EAI_GetType, after findFieldInOFFSETS, have myFieldOffs %d, ctype %d, accessType %d \n",myFieldOffs, ctype, *accessType); */

	/* is this a PROTO, or just an invalid field?? */ 
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

		/* a PROTO? but one that the node/field combo was never instanced? */
		} else if (nodePtr->_nodeType == NODE_Group) {
			struct ProtoDefinition *myProtoDecl = X3D_GROUP(nodePtr)->FreeWRL__protoDef;
			struct ProtoFieldDecl *thisIndex  = getProtoFieldDeclaration( globalParser->lexer, myProtoDecl, fieldString);
			int i;
			struct ProtoElementPointer *ele;


			myScriptType = EAI_NODETYPE_PROTO;

			printf ("EAI_GetType, node is a PROTO, and field not found in basic node - lets find all kinds of info for this one \n");

printf ("EAI_GetType, protoDefNumber %d protoName %s\n",myProtoDecl->protoDefNumber, myProtoDecl->protoName);
for (i=0; i< vector_size(myProtoDecl->deconstructedProtoBody); i++) {
	ele = vector_get(struct ProtoElementPointer*, myProtoDecl->deconstructedProtoBody, i);
	printf ("PROTO - ele %d of %d ", i, vector_size(myProtoDecl->deconstructedProtoBody)-1); 

	if (ele->isNODE != -1) printf ("isNODE - %s ",stringNodeType(ele->isNODE));
	if (ele->isKEYWORD != -1) printf ("isKEYWORD - %s ",stringKeywordType(ele->isKEYWORD));
	if (ele->terminalSymbol != -1) printf ("terminalSymbol '%c' ",ele->terminalSymbol);
	if (ele->stringToken != NULL) printf ("stringToken :%s: ",ele->stringToken);
	if (ele->fabricatedDef != -1) printf ("fabricatedDef %d",ele->fabricatedDef);
	printf ("\n");
}


		} else {
			printf ("EAI_GetType, not not found, just keep it at -1\n");
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
	int getValueFromPROTOField = FALSE;
	struct X3D_Node *myNode;
	int nodeIndex, fieldIndex, length;
	char ctmp[4000];
	int retint;

	if (eaiverbose) printf ("GETVALUE %s \n",bufptr);
	printf ("GETVALUE %s \n",bufptr);


	/* format: ptr, offset, type, length (bytes)*/
	retint=sscanf (bufptr, "%d %d %c %d", &nodeIndex,&fieldIndex,ctmp,&length);
	myNode = getEAINodeFromTable(nodeIndex, fieldIndex);

	/* if myNode is NULL, we have an error, baby */
	if (myNode == NULL) {
		printf ("handleEAIGetValue - node does not exist!\n");
		return;
	}

	
printf ("handleEAIGetValue, node %u, type %s\n",myNode, stringNodeType(myNode->_nodeType));

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
