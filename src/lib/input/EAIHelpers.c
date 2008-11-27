/*
=INSERT_TEMPLATE_HERE=

$Id: EAIHelpers.c,v 1.2 2008/11/27 00:27:18 couannette Exp $

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

struct DEFnameStruct {
	struct X3D_Node *node;
	struct Uni_String *name;
};
extern struct DEFnameStruct *DEFnames;
extern int DEFtableSize;


/* get a node pointer in memory for a node. Return the node pointer, or NULL if it fails */
uintptr_t EAI_GetNode(const char *str) {
	struct X3D_Node *myn;
	int ctr;
	struct Uni_String* tmp;

	if (eaiverbose) {
		printf ("EAI_GetNode - getting %s\n",str);
	}	
	
	/* Try to get X3D node name */
	for (ctr = 0; ctr <= DEFtableSize; ctr++) {
		tmp = DEFnames[ctr].name;
		if (strcmp(str, tmp->strptr) == 0) {
			return (uintptr_t) DEFnames[ctr].node;
		}
	}

	/* Try to get VRML node name */
	myn = parser_getNodeFromName(str);
	if (eaiverbose) {
		if (myn == NULL) printf ("EAI_GetNode for %s returns %u\n",str,myn);
		else printf ("EAI_GetNode for %s returns %x - it is a %s\n",str,myn,stringNodeType(myn->_nodeType));
	}	
	return (uintptr_t) myn;
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

	int *myProtoIndex	index into the proto field value tables for this field. returns
				-1 if this field is not found or table overflow.

*********************************************************************************/


/* for keeping track of PROTO field values - hold on to last 10 values from a getType call */
char * myProtoFields[] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
int myProtoFieldIndex = 0;

void findFieldInPROTOOFFSETS (struct X3D_Node *myNode, char *myField, uintptr_t *myNodeP,
					int *myOfs, int *myType, int *accessType, 
					int *myProtoIndex) {

	struct X3D_Group *group;
	struct ProtoDefinition *myProtoDecl;
	struct ProtoFieldDecl *thisIndex;
	int fc, fc2;
	union anyVrml myDefaultValue;
	struct OffsetPointer *myP;
	char *cp;


	/* set these values, so that we KNOW if we have found the correct field */
	*myType = 0;
	*myOfs = 0;
	*myProtoIndex = -1;
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
			/* printf ("TESTING - got replace as %s\n",newTl); */

			FREE_IF_NZ(myProtoFields[myProtoFieldIndex]);
			myProtoFields[myProtoFieldIndex] = newTl;
			*myProtoIndex = myProtoFieldIndex;
			myProtoFieldIndex++; 
			if (myProtoFieldIndex >= 10) {
				printf ("EAI - re-using the protoFieldIndex values\n");
				myProtoFieldIndex = 0;
			}

			*myType = mapFieldTypeToEAItype(protoFieldDecl_getType(thisIndex));
			*accessType = mapToKEYWORDindex(protoFieldDecl_getAccessType(thisIndex));

		}
	}
}


void EAI_GetType (uintptr_t cNode,  char *ctmp, char *dtmp, uintptr_t *cNodePtr, uintptr_t *fieldOffset,
			uintptr_t *dataLen, uintptr_t *typeString,  unsigned int *scripttype, int *accessType) {

	struct X3D_Node *nodePtr;
	int myField;
	int *myofs;
	int ctype;
/* 	
	cNode = node pointer into memory - assumed to be valid
	ctmp = field as a string - eg, "addChildren"
	dtmp = access method = "eventIn", "eventOut", "field" or...???
	cNodePtr = C node pointer;
	fieldOffset = offset;
	dataLen = data len;
	typeString = mapFieldTypeToEAItype (ctype);
	scripttype = 0 - meaning, not to/from a javascript. (see CRoutes.c for more info)
	
*/

	int myProtoIndex;
	int myFieldOffs;

	nodePtr = X3D_NODE(cNode);
	
	if (eaiverbose) {
		printf ("start of EAI_GetType, this is a valid C node %d\n",nodePtr);
		printf ("	of string type %s\n",stringNodeType(nodePtr->_nodeType)); 
	}	

					

	if ((strncmp (ctmp,"addChildren",strlen("addChildren")) == 0) || 
	(strncmp (ctmp,"removeChildren",strlen("removeChildren")) == 0)) {
		myField = findFieldInFIELDNAMES("children");
	} else {
		/* try finding it, maybe with a "set_" or "changed" removed */
		myField = findRoutedFieldInFIELDNAMES(nodePtr,ctmp,0);
		if (myField == -1) 
			myField = findRoutedFieldInFIELDNAMES(nodePtr,ctmp,1);
	}
	myofs = (int *)NODE_OFFSETS[nodePtr->_nodeType];

	/* find offsets, etc */
       	findFieldInOFFSETS(myofs, myField, &myFieldOffs, &ctype, accessType);

	/* is this a PROTO, or just an invalid field?? */ 
	if (myFieldOffs <= 0) {
		if (eaiverbose) {
			printf ("EAI_GetType, myFieldOffs %d, try findFieldInPROTOOFFSETS\n",myFieldOffs);
		}	
		findFieldInPROTOOFFSETS (nodePtr, ctmp, cNodePtr, &myFieldOffs, &ctype, accessType ,&myProtoIndex);

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
	/* fieldOffset - assigned above - offset */
	*fieldOffset = (uintptr_t) myFieldOffs;
	*dataLen = returnRoutingElementLength(mapEAItypeToFieldType(ctype));	/* data len */
	*typeString = (uintptr_t) ctype;	
	*scripttype =0;

	/* re-map the access type back for children fields */
	if (strncmp (ctmp,"addChildren",strlen("addChildren")) == 0) *accessType = KW_eventIn; 
	if (strncmp (ctmp,"removeChildren",strlen("removeChildren")) == 0) *accessType = KW_eventOut;
					
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

char* EAI_GetValue(unsigned int nodenum, const char *fieldname, const char *nodename) {
	printf ("HELP::EAI_GetValue, %d, %s %s\n",nodenum, fieldname, nodename);
}

unsigned int EAI_GetViewpoint(const char *str) {
	printf ("HELP::EAI_GetViewpoint %s\n",str);
	return 0;
}
