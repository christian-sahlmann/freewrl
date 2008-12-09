/*
=INSERT_TEMPLATE_HERE=

$Id: EAIHelpers.c,v 1.5 2008/12/09 12:12:41 couannette Exp $

Small routines to help with interfacing EAI to Daniel Kraft's parser.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h" /* point_XYZ */
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

/* keep a list of node requests, so that we can keep info on each node */
#define MAXFIELDSPERNODE 20
static int lastNodeRequested = 0;

/* store enough stuff in here so that each field can be read/written, no matter whether it is a 
   PROTO, SCRIPT, regular node, pint of beer, or whatever */

struct EAINodeParams {
	int fieldOffset;
	int datalen;
	int typeString;
	int scripttype;
	char *invokedValue; 	/* proto field value on invocation (default, or supplied) */
};

struct EAINodeIndexStruct {
	struct X3D_Node*	actualNodePtr;
	struct EAINodeParams 	params[MAXFIELDSPERNODE];
	int    maxparamindex;
};
static struct  EAINodeIndexStruct EAINodeIndex[1000];




/* get an actual memory pointer to field, assumes both node has passed ok check */
uintptr_t *getEAIMemoryPointer (int node, int field) {
	char *memptr;

	/* do memory pointer math in char formats, as this ensures 8 bit compatibility */
	memptr = (char *)getEAINodeFromTable(node);
	memptr += EAINodeIndex[node].params[field].fieldOffset;
	/* printf ("getEAIMemoryPointer, nf %d:%d, node %u offset %d total %u\n",
		node,field,getEAINodeFromTable(node), EAINodeIndex[node].params[field].fieldOffset, memptr);
	*/

	return (uintptr_t *)memptr;
}

/* get the parameters during proto invocation. Might not ever have been ISd */
char * getEAIInvokedValue(int node, int field) {
	return EAINodeIndex[node].params[field].invokedValue;
}


/* return the actual field offset as defined; change fieldHandle into an actual value */
int getEAIActualOffset(int node, int field) {
	return EAINodeIndex[node].params[field].fieldOffset;
}

/* return a registered node. If index==0, return NULL */
struct X3D_Node *getEAINodeFromTable(int index) {
	if (index==0) return NULL;
	if (index>lastNodeRequested) {
		printf ("internal EAI error - requesting %d, highest node %d\n",
			index,lastNodeRequested);
		return NULL;
	}

	return EAINodeIndex[index].actualNodePtr;
}

/* return an index to a node table. return value 0 means ERROR!!! */
int registerEAINodeForAccess(struct X3D_Node* myn) {
	int ctr;
	int mynindex = 0;

	printf ("registerEAINodeForAccess, remember to MALLOC this stuff... passed in %p\n", myn);
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
		mynindex = lastNodeRequested;
		EAINodeIndex[mynindex].actualNodePtr = myn;
		EAINodeIndex[mynindex].maxparamindex = -1;
	}

	if (eaiverbose) printf ("registerEAINodeForAccess returning index %d\n",mynindex);
	return mynindex;
}

/******************************************************************************************/

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


/* get the type of a node; node must exist in table */
/* 	
	cNode = handle for node pointer into memory - if not valid, this routine returns everything as zeroes
	ctmp = field as a string - eg, "addChildren"
	dtmp = access method = "eventIn", "eventOut", "field" or...???
	cNodePtr = C node pointer;
	fieldOffset = offset;
	dataLen = data len;
	typeString = mapFieldTypeToEAItype (ctype);
	scripttype = 0 - meaning, not to/from a javascript. (see CRoutes.c for more info)
*/

void EAI_GetType (int cNode,  char *ctmp, char *dtmp, 
		uintptr_t *cNodePtr, uintptr_t *fieldOffset,
		uintptr_t *dataLen, uintptr_t *typeString,  unsigned int *scripttype, int *accessType) {

	struct X3D_Node* nodePtr = getEAINodeFromTable(cNode);
	int myField;
	int ctype;
	int myProtoIndex;
	int myFieldOffs;
	int maxparamindex = 0;
	char *invokedValPtr = NULL;  /* for PROTOs - invocation value */

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

	/* try finding it, maybe with a "set_" or "changed" removed */
	myField = findRoutedFieldInFIELDNAMES(nodePtr,ctmp,0);
	if (myField == -1) 
		myField = findRoutedFieldInFIELDNAMES(nodePtr,ctmp,1);

	/* find offsets, etc */
       	findFieldInOFFSETS((int *)NODE_OFFSETS[nodePtr->_nodeType], myField, &myFieldOffs, &ctype, accessType);

	/* is this a PROTO, or just an invalid field?? */ 
	if (myFieldOffs <= 0) {
		if (eaiverbose) {
			printf ("EAI_GetType, myFieldOffs %d, try findFieldInPROTOOFFSETS\n",myFieldOffs);
		}	
		findFieldInPROTOOFFSETS (nodePtr, ctmp, cNodePtr, &myFieldOffs, &ctype, accessType ,&invokedValPtr);

		/* did we find an actual proto expansion field? If not, just return the original node
		   (the one we pointed to in the first case) and the protoIndex as an offset, as the
		   user probably wanted an info field */

		if (*cNodePtr == 0) {
			/* printf ("got node pointer as zero, lets just return more info for this \n"); */
			*cNodePtr = cNode;
			myFieldOffs = myProtoIndex;
			/* printf ("hmmm o fieldOffs %d, myProtoIndex %d\n",myFieldOffs, myProtoIndex); */
		}
	} else {
		*cNodePtr = cNode; 	/* node pointer */
		ctype = mapFieldTypeToEAItype(ctype); /* change to EAI type */
	}

	/* return values. */
	/* save these indexes */
	EAINodeIndex[cNode].maxparamindex++;
	maxparamindex = EAINodeIndex[cNode].maxparamindex;
	if (maxparamindex >= MAXFIELDSPERNODE) {
		maxparamindex = MAXFIELDSPERNODE-1;
		EAINodeIndex[cNode].maxparamindex = maxparamindex;
		printf ("recompile with MAXFIELDSPERNODE increased in value\n");
	}
	EAINodeIndex[cNode].params[maxparamindex].fieldOffset = myFieldOffs;
	EAINodeIndex[cNode].params[maxparamindex].datalen = returnRoutingElementLength(mapEAItypeToFieldType(ctype));
	EAINodeIndex[cNode].params[maxparamindex].typeString = ctype;
	EAINodeIndex[cNode].params[maxparamindex].scripttype = 0;
	EAINodeIndex[cNode].params[maxparamindex].invokedValue = invokedValPtr;

	*fieldOffset = (uintptr_t) maxparamindex;
	*dataLen = returnRoutingElementLength(mapEAItypeToFieldType(ctype));	/* data len */
	*typeString = (uintptr_t) ctype;	
	*scripttype =0;

	if (eaiverbose) {
		printf ("EAI_GetType, returning cNodePtr %u, coffset %d, ctype %x, ctmp %s\n",
			*cNodePtr,*fieldOffset,ctype, stringKeywordType(*accessType));
	}
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
	myNode = getEAINodeFromTable(nodeIndex);

	/* if myNode is NULL, we have an error, baby */
	if (myNode == NULL) {
		printf ("handleEAIGetValue - node does not exist!\n");
		return;
	}

	
	/* is the pointer a pointer to a PROTO?? If so, then the getType did not find
	an actual field (an IS'd field??) in a proto expansion for us.  We have to 
	go through, as the offset will be the index in the PROTO field for us to get
	the value for */

	if (EAINodeIndex[nodeIndex].params[fieldIndex].invokedValue != NULL) {
		sprintf (buf,"RE\n%f\n%d\n%s",TickTime,repno,getEAIInvokedValue(nodeIndex,fieldIndex));	
	} else {
		EAI_Convert_mem_to_ASCII (repno,"RE",mapEAItypeToFieldType(ctmp[0]),getEAIMemoryPointer(nodeIndex,fieldIndex), buf);
	}
}
