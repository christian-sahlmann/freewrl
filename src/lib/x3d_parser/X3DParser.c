/*
=INSERT_TEMPLATE_HERE=

$Id: X3DParser.c,v 1.39 2009/09/16 22:48:23 couannette Exp $

???

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/CScripts.h"
#include "../world_script/JScript.h"
#include "../world_script/fieldSet.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CProto.h"
#include "../vrml_parser/CParse.h"

#include "X3DParser.h"
#include "X3DProtoScript.h"

#if HAVE_EXPAT_H
# include <expat.h>
#endif

/* If XMLCALL isn't defined, use empty one */
#ifndef XMLCALL
 #define XMLCALL
#endif /* XMLCALL */

static int inCDATA = FALSE;

static struct VRMLLexer *myLexer = NULL;
static Stack* DEFedNodes = NULL;


char *CDATA_Text = NULL;
static int CDATA_TextMallocSize = 0;
int CDATA_Text_curlen = 0;

/* for testing Johannes Behrs fieldValue hack for getting data in */
static int in3_3_fieldValue = FALSE;
static int in3_3_fieldIndex = INT_ID_UNDEFINED;

/* this ifdef sequence is kept around, for a possible Microsoft Vista port */
#ifdef XML_LARGE_SIZE
#if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#define XML_FMT_INT_MOD "I64"
#else
#define XML_FMT_INT_MOD "ll"
#endif
#else
#define XML_FMT_INT_MOD "l"
#endif

int parentIndex = -1;
struct X3D_Node *parentStack[PARENTSTACKSIZE];

static const char *parserModeStrings[] = {
		"unused",
		"PARSING_NODES",
		"PARSING_SCRIPT",
		"PARSING_PROTODECLARE ",
		"PARSING_PROTOINTERFACE ",
		"PARSING_PROTOBODY",
		"PARSING_PROTOINSTANCE",
		"PARSING_IS",
		"PARSING_CONNECT",
		"unused high"};
		
int currentParserMode = PARSING_NODES;

/* XML parser variables */
static int X3DParserRecurseLevel = INT_ID_UNDEFINED;
static XML_Parser x3dparser[PROTOINSTANCE_MAX_LEVELS];
static XML_Parser currentX3DParser = NULL;

void debugsetParserMode(int newmode, char *fle, int line) {
	currentParserMode = newmode; 
	#ifdef X3DPARSERVERBOSE
	printf ("setParserMode to mode %s at %s:%d\n",parserModeStrings[newmode],fle,line);
	#endif
}
int getParserMode(void) { return currentParserMode; }


/* get the line number of the current parser for error purposes */
int freewrl_XML_GetCurrentLineNumber(void) {
	if (X3DParserRecurseLevel > INT_ID_UNDEFINED)
	{
		currentX3DParser = x3dparser[X3DParserRecurseLevel]; /*dont trust current*/
		return XML_GetCurrentLineNumber(currentX3DParser); 
	}
	return INT_ID_UNDEFINED;
}


/*
2b) Allow <fieldValue> + extension for all node-types
-------------------------------------------------------------
There is already a fieldValue element in current X3D-XML
spec to specify the value of a ProtoInstance-field.
To specify a value of a ProtoInstance field looks like this:

<ProtoInstance name='bar' >
  <fieldValue name='foo' value='TRUE' />
</ProtoInstance>

We could change the wording in the spec to allow
<fieldValue>- elements not just for ProtoInstance-nodes
but for all instances of nodes. In addition we would
allow element data in fieldValues which will
be read to the single specified field. This solution
is not limited to a single field per node but could
easily handle any number of fields

The 'count' attribute is optional and an idea
borrowed form the COLLADA specification. The count-value
could be used to further improve the parser-speed because
the parser could already reserve the amount of data needed.

<Coordinate3d>
  <fieldValue name='point' count='4' >
   0.5 1.0 1.0
   2.0 2.0 2.0
   3.0 0.5 1.5
   4.0 1.4 2.0
  </fieldValue>
</Coordinate3d>

pro:
+ Dynamic solution; we do not have to tag fields
+ Long term solution, no one-field-per-node limitation
+ Uses an existing element/concept. No additional complexity
+ Works with protos

con:
- Introduces one additional element in the code; data looks not as compact as 2a
- Allowing attribute and element-data for a single field is redundant; Need to specify how to handle ambiguities

*/

/* add this data to the end of the current CData array for later use */
static void appendDataToFieldValue(char *data, int len) {
	if ((CDATA_Text_curlen+len) > CDATA_TextMallocSize-100) {
		while ((CDATA_Text_curlen+len) > CDATA_TextMallocSize-100) {
			if (CDATA_TextMallocSize == 0) CDATA_TextMallocSize = 2048;
			else CDATA_TextMallocSize *= 2;
		}
		CDATA_Text = REALLOC (CDATA_Text,CDATA_TextMallocSize);
	}

	memcpy(&CDATA_Text[CDATA_Text_curlen],data,len);
	CDATA_Text_curlen+=len;
	CDATA_Text[CDATA_Text_curlen]='\0';
}

/* we are finished with a 3.3 fieldValue, tie it in */
static void setFieldValueDataActive(void) {
	if (!in3_3_fieldValue) printf ("expected this to be in a fieldValue\n");

	/* if we had a valid field for this node... */
	if (in3_3_fieldIndex != INT_ID_UNDEFINED) {

		/* printf ("setFieldValueDataActive field %s, parent is a %s\n",
			stringFieldType(in3_3_fieldIndex),stringNodeType(parentStack[parentIndex]->_nodeType)); */

		setField_fromJavascript (parentStack[parentIndex], (char *) stringFieldType(in3_3_fieldIndex),
			CDATA_Text, TRUE);
	}

	/* free data */
	in3_3_fieldValue = FALSE;
	CDATA_Text_curlen = 0;
	in3_3_fieldIndex = INT_ID_UNDEFINED;
}


void freeProtoMemory () {
printf ("freeProtoMemory\n");}


/**************************************************************************************/

/* for EAI/SAI - if we have a Node, look up the name in the DEF names */
char *X3DParser_getNameFromNode(struct X3D_Node* myNode) {
	indexT ind;
	struct X3D_Node* node;

	/* printf ("getNameFromNode called on %u\n",myNode); */

	/* go through the DEFed nodes and match the node pointers */
	for (ind=0; ind<vector_size(stack_top(struct Vector*, DEFedNodes)); ind++) {
		node=vector_get(struct X3D_Node*, stack_top(struct Vector*, DEFedNodes),ind);
		
		/* did we have a match? */
		/* printf ("X3DParser_getNameFromNode, comparing %u and %u at %d\n",myNode,node,ind); */
		if (myNode == node) {
			/* we have the index into the lexers name table; return the name */
			struct Vector *ns;
			ns = stack_top(struct Vector*, myLexer->userNodeNames);
			return ((char *)vector_get (const char*, ns,ind));
		}
	}

	/* not found, return NULL */
	return NULL;
}

/* for EAI/SAI - if we have a DEF name, look up the node pointer */
struct X3D_Node *X3DParser_getNodeFromName(const char *name) {
	return DEFNameIndex(name,NULL,FALSE);
}

/**************************************************************************************/



/* "forget" the DEFs. Keep the table around, though, as the entries will simply be used again. */
void kill_X3DDefs(void) {
printf ("kill_X3DDefs - nothing here - have to get rid of the DEFedNodes Stack\n");
/* XXX fill this in */
}



/* return a node assoicated with this name. If the name exists, return the previous node. If not, return
the new node */
struct X3D_Node *DEFNameIndex (const char *name, struct X3D_Node* node, int force) {
	indexT ind = ID_UNDEFINED;

#ifdef X3DPARSERVERBOSE
	printf ("DEFNameIndex, looking for :%s:, force %d nodePointer %u\n",name,force,node);
#endif
	/* lexer_defineNodeName is #defined as lexer_defineID(me, ret, stack_top(struct Vector*, userNodeNames), TRUE) */
	/* Checks if this node already exists in the userNodeNames vector.  If it doesn't, adds it. */

	lexer_fromString(myLexer,STRDUP(name));

	if(!lexer_defineNodeName(myLexer, &ind))
		printf ("Expected nodeNameId after DEF!\n");

#ifdef X3DPARSERVERBOSE
	printf ("DEF returns id of %d for %s\n",ind,name);
#endif

	ASSERT(ind<=vector_size(stack_top(struct Vector*, DEFedNodes)));

#ifdef X3DPARSERVERBOSE
	printf ("so, in DEFNameIndex, we have ind %d, vector_size %d\n",ind,vector_size(stack_top(struct Vector*, DEFedNodes)));
#endif

	if(ind==vector_size(stack_top(struct Vector*, DEFedNodes))) {
		vector_pushBack(struct X3D_Node*, stack_top(struct Vector*, DEFedNodes), node);
	}
	ASSERT(ind<vector_size(stack_top(struct Vector*, DEFedNodes)));

	/* if we did not find this node, just return */
	if (ind == ID_UNDEFINED) {return NULL; }

	node=vector_get(struct X3D_Node*, stack_top(struct Vector*, DEFedNodes),ind);

#ifdef X3DPARSERVERBOSE
	if (node != NULL) printf ("DEFNameIndex for %s, returning %u, nt %s\n",name, node,stringNodeType(node->_nodeType));
	else printf ("DEFNameIndex, node is NULL\n");
#endif

	return node;
}



/* look through the script fields for this field, and return the values. */
static int getFieldFromScript (struct VRMLLexer *myLexer, char *fieldName, struct Shader_Script *me, int *offs, int *type, int *accessType) {

	struct ScriptFieldDecl* myField;
	const char** userArr;
	size_t userCnt;
	indexT retUO;

	/* initialize */
	myField = NULL;
	retUO = ID_UNDEFINED;


	#ifdef X3DPARSERVERBOSE
	printf ("getFieldFromScript, looking for %s\n",fieldName);
	#endif

	/* go through the user arrays in this lexer, and see if we have a match */

	myField = script_getField_viaASCIIname (me, fieldName);
	/* printf ("try2: getFieldFromScript, field %s is %d\n",fieldName,myField); */

	if (myField != NULL) {
		int myFieldNumber;

		/* is this a script? if so, lets do the conversion from our internal lexer name index to
		   the scripting name index. */
		if (me->ShaderScriptNode->_nodeType == NODE_Script) {
			/* wow - have to get the Javascript text string index from this one */
			myFieldNumber = JSparamIndex(fieldName,stringFieldtypeType(myField->fieldDecl->type)); 

			*offs=myFieldNumber;


		} else {
			*offs = myField->fieldDecl->name;
		}
		*type = myField->fieldDecl->type;
		/* go from PKW_xxx to KW_xxx  .... sigh ... */
		*accessType = mapToKEYWORDindex(myField->fieldDecl->mode);
		return TRUE;
	}

	#ifdef X3DPARSERVERBOSE
	printf ("getFieldFromScript, did not find field %s in script\n",fieldName);
	#endif
	
	/* did not find it */
	*offs = INT_ID_UNDEFINED;  *type = 0;
	return FALSE;

}

int getRoutingInfo (struct VRMLLexer *myLexer, struct X3D_Node *node, int *offs, int* type, int *accessType, struct Shader_Script **myObj, char *name, int routeTo) {
	int error;
	int fieldInt;

#ifdef X3DPARSERVERBOSE
	printf ("getRoutingInfo, node %u\n",node);
	printf ("getRoutingInfo, nt %s\n",stringNodeType(node->_nodeType));
#endif
	error = FALSE;
	switch (node->_nodeType) {

	case NODE_Script: {
		*myObj = X3D_SCRIPT(node)->__scriptObj;
		error = !(getFieldFromScript (myLexer, name,*myObj,offs,type,accessType));
		break; }
	case NODE_ComposedShader: {
		*myObj = X3D_COMPOSEDSHADER(node)->__shaderObj;
		error = !(getFieldFromScript (myLexer, name,*myObj,offs,type,accessType));
		break; }
	case NODE_ShaderProgram: {
		*myObj = X3D_SHADERPROGRAM(node)->__shaderObj;
		error = !(getFieldFromScript (myLexer, name,*myObj,offs,type,accessType));
		break; }
	case NODE_PackagedShader: {
		*myObj = X3D_PACKAGEDSHADER(node)->__shaderObj;
		error = !(getFieldFromScript (myLexer, name,*myObj,offs,type,accessType));
		break; }
	default:
		*myObj=NULL;

		/* lets see if this node has a routed field  fromTo  = 0 = from node, anything else = to node */
		fieldInt = findRoutedFieldInFIELDNAMES (node, name, routeTo);

		if (fieldInt >=0) { findFieldInOFFSETS(node->_nodeType, 
				fieldInt, offs, type, accessType);
		} else {
			/* do NOT set error here; this might be a PROTO expansion and more work is needed */
			*offs=INT_ID_UNDEFINED;
			*type=INT_ID_UNDEFINED;
		}
	}
	return error;
}


static int getRouteField (struct VRMLLexer *myLexer, struct X3D_Node **innode, int *offs, int* type, char *name, int routeTo) {
	int error;
	int fieldInt;
	int accessType;
	struct X3D_Node *node;
	struct Shader_Script *holder;

	node = *innode; /* ease of use - contents of pointer in param line */
 
	error = getRoutingInfo(myLexer,node,offs,type,&accessType, &holder, name,routeTo);

	/* printf ("getRouteField, offs %d type %d\n",*offs, *type); */

	if ((*offs <0) && (node->_nodeType==NODE_Group)) {
		/* is this a PROTO expansion? */
		struct X3D_Group *myg;
		int myp;
		int i;

		/* lets go finding; if this is a PROTO expansion, we will have FreeWRL__protoDef != INT_ID_UNDEFINED */
		myg = X3D_GROUP(node);

		/* printf ("routing, looking to see if this is a proto expansion... myg %u\n",myg); */
		myp = myg->FreeWRL__protoDef;

		if (myp != INT_ID_UNDEFINED) {
			char newname[1000];
			struct X3D_Node *newn;


			/* printf ("we are routing to an X3D PROTO Expansion\n");
			printf ("looking for name %s\n",name); */

			sprintf (newname,"%s_%s_%d",name,FREEWRL_SPECIFIC,myp);

			/* printf ("and, defined name is %s\n",newname); */


			/* look up this node; if it exists, look for field within it */
			newn = DEFNameIndex ((const char *)newname, NULL, FALSE);

			/* printf ("newn is %u\n",newn); */
			if (newn!=NULL){
				/* printf ("newn node type %s\n",stringNodeType(newn->_nodeType)); */
				if (routeTo == 0) {
					/* printf ("and we are routing FROM this proto expansion\n"); */
					fieldInt = findRoutedFieldInFIELDNAMES(newn,"valueChanged",routeTo);
				} else {
					/* printf ("and, routing TO this proto expansion\n"); */
					fieldInt = findRoutedFieldInFIELDNAMES(newn,"setValue",routeTo);
				}
				if (fieldInt >=0) {
					findFieldInOFFSETS(newn->_nodeType, 
					fieldInt, offs, type, &accessType);
					*innode = newn; /* pass back this new node for routing */
				}
			}


		
	
		}

	}



	if (*offs <0) {
		ConsoleMessage ("ROUTE: line %d Field %s not found in node type %s",LINE,
			name,stringNodeType(node->_nodeType));
		error = TRUE;
	}

	/* can we route with this direction with this field? This might be already checked, but lets
	   make sure once and for all... */
	if (routeTo) {
		if ((accessType != KW_inputOnly) && (accessType != KW_inputOutput)) {
			ConsoleMessage ("ROUTE: line %d: can not route TO a type of %s\n",LINE,stringKeywordType(accessType));
			error = TRUE;
		}
	} else {
		if ((accessType != KW_outputOnly) && (accessType != KW_inputOutput)) {
			ConsoleMessage ("ROUTE: line %d: can not route FROM a type of %s\n",LINE,stringKeywordType(accessType));
			error = TRUE;
		}
	}
	return error;
}


/******************************************************************************************/
/* parse a ROUTE statement. Should be like:
	<ROUTE fromField="fraction_changed"  fromNode="TIME0" toField="set_fraction" toNode="COL_INTERP"/>
*/

static void parseRoutes (const char **atts) {
	struct X3D_Node *fromNode = NULL;
	struct X3D_Node *toNode = NULL;	
	int fromOffset = INT_ID_UNDEFINED;
	int toOffset = INT_ID_UNDEFINED;
	int i;
	int error = FALSE;

	int fromType;
	int toType;
	#ifdef X3DPARSERVERBOSE
	printf ("\nstart ofrouting\n");	
	#endif

	/* 2 passes - first, find the nodes */
	for (i = 0; atts[i]; i += 2) {
		#ifdef X3DPARSERVERBOSE
		printf("ROUTING pass 1 field:%s=%s\n", atts[i], atts[i + 1]);
		#endif

		if (strcmp("fromNode",atts[i]) == 0) {
			fromNode = DEFNameIndex (atts[i+1], NULL, FALSE);
			if (fromNode == NULL) {
				ConsoleMessage ("ROUTE statement, line %d fromNode (%s) does not exist",LINE,atts[i+1]);
				error = TRUE;
			}
		} else if (strcmp("toNode",atts[i]) == 0) {
			toNode = DEFNameIndex (atts[i+1],NULL, FALSE);
			if (toNode == NULL) {
				ConsoleMessage ("ROUTE statement, line %d toNode (%s) does not exist",LINE,atts[i+1]);
				error = TRUE;
			}
		} else if ((strcmp("fromField",atts[i])!=0) &&
				(strcmp("toField",atts[i]) !=0)) {
			ConsoleMessage ("Field in line %d ROUTE statement not understood: %s",LINE,atts[i]);
			error = TRUE;
		}
	}

	/* get out of here if an error is found */
	if (error) return;

	#ifdef X3DPARSERVERBOSE
	printf ("end of pass1, fromNode %d, toNode %d\n",fromNode,toNode);
	printf ("looking for a route from a %s to a %s\n",stringNodeType(fromNode->_nodeType),
			stringNodeType(toNode->_nodeType));
	#endif

	/* second pass - get the fields of the nodes */
	for (i = 0; atts[i]; i += 2) {
		if (strcmp("fromField",atts[i])==0) {
			error = getRouteField(myLexer, &fromNode, &fromOffset, &fromType, (char *)atts[i+1],0);
		} else if (strcmp("toField",atts[i]) ==0) {
			error = getRouteField(myLexer, &toNode, &toOffset, &toType, (char *)atts[i+1],1);
		}
	}	

	/* get out of here if an error is found */
	if (error) return;

	#ifdef X3DPARSERVERBOSE
	printf ("now routing from a %s to a %s \n",stringFieldtypeType(fromType), stringFieldtypeType(toType));
	printf ("	pointers %d %d to %d %d\n",fromNode, fromOffset, toNode, toOffset);
	#endif

	/* are the types the same? */
	if (fromType != toType) {
		ConsoleMessage ("Routing type mismatch line %d %s != %s",LINE,stringFieldtypeType(fromType), stringFieldtypeType(toType));
		error = TRUE;
	}

	/* get out of here if an error is found */
	if (error) return;


	/* can we register the route? */
	CRoutes_RegisterSimple(fromNode, fromOffset, toNode, toOffset, returnRoutingElementLength(fromType));
}

#ifdef OLDCODE
/* parse normal X3D nodes/fields */
static void parseNormalX3D(int myNodeType, const char *name, const char** atts) {
	int i;

	struct X3D_Node *thisNode;
	struct X3D_Node *fromDEFtable;

	/* semantic check */
	if ((getParserMode() != PARSING_NODES) && (getParserMode() != PARSING_PROTOBODY)) {
printf ("hey, we have maybe a Node (%s) in a Script list... line %d: expected parserMode to be PARSING_NODES, got %s\n", 
			stringNodeType(myNodeType),LINE,
                                        parserModeStrings[getParserMode()]);

	}

	switch (myNodeType) {
		case NODE_Script:
		case NODE_ComposedShader:
		case NODE_ShaderProgram:
        	case NODE_PackagedShader: 
			setParserMode(PARSING_SCRIPT); 
			#ifdef X3DPARSERVERBOSE
			printf ("parseNormalX3D, setting to PARSE_SCRIPT for name %s\n",name);
			#endif
		break;
        	default: {}
        }


	/* create this to be a new node */	
	thisNode = createNewX3DNode(myNodeType);
	parentStack[parentIndex] = thisNode; 

	#ifdef X3DPARSERVERBOSE
	TTY_SPACE
	printf ("parseNormalX3D: for name %s, myNodeType = %d is %u parentInded %d\n",name,myNodeType,thisNode,parentIndex);
	#endif

	if (myNodeType == NODE_Script) {
		struct Shader_Script *myObj;

		/* create the Shader_Script for this one */
		X3D_SCRIPT(thisNode)->__scriptObj=new_Shader_Script(thisNode);


		#ifdef X3DPARSERVERBOSE
		printf ("working through script parentIndex %d\n",parentIndex);
		#endif

		myObj = X3D_SCRIPT(thisNode)->__scriptObj;
		JSInit(myObj->num);
	} else if (myNodeType == NODE_ComposedShader) {
		X3D_COMPOSEDSHADER(thisNode)->__shaderObj=new_Shader_Script(thisNode);
	} else if (myNodeType == NODE_ShaderProgram) {
		X3D_SHADERPROGRAM(thisNode)->__shaderObj=new_Shader_Script(thisNode);
	} else if (myNodeType == NODE_PackagedShader) {
		X3D_PACKAGEDSHADER(thisNode)->__shaderObj=new_Shader_Script(thisNode);
	}

	/* go through the fields, and link them in. SFNode and MFNodes will be handled 
	 differently - these are usually the result of a different level of parsing,
	 and the "containerField" value */
	for (i = 0; atts[i]; i += 2) {
		#ifdef X3DPARSERVERBOSE
		if (getParserMode() == PARSING_SCRIPT) {
			printf ("parsing script decl; have %s %s\n",atts[i], atts[i+1]);
		}
		#endif


		if (strcmp ("DEF",atts[i]) == 0) {
			#ifdef X3DPARSERVERBOSE
			printf ("this is a DEF, name %s\n",atts[i+1]);
			#endif

			fromDEFtable = DEFNameIndex ((char *)atts[i+1],thisNode, TRUE);
			if (fromDEFtable != thisNode) {
				#ifdef X3DPARSERVERBOSE
				printf ("Warning - line %d duplicate DEF name: \'%s\'\n",LINE,atts[i+1]);
				#endif
			}

		} else if (strcmp ("USE",atts[i]) == 0) {
			#ifdef X3DPARSERVERBOSE
			printf ("this is a USE, name %s\n",atts[i+1]);
			#endif

			fromDEFtable = DEFNameIndex ((char *)atts[i+1],thisNode, FALSE);
			if (fromDEFtable == thisNode) {
				ConsoleMessage ("Warning - line %d DEF name: \'%s\' not found",LINE,atts[i+1]);
			} else {
				#ifdef X3DPARSERVERBOSE
				printf ("copying for field %s defName %s\n",atts[i], atts[i+1]);
				#endif

				if (fromDEFtable->_nodeType != fromDEFtable->_nodeType) {
					ConsoleMessage ("Warning, line %d DEF/USE mismatch, '%s', %s != %s", LINE,
						atts[i+1],stringNodeType(fromDEFtable->_nodeType), stringNodeType (thisNode->_nodeType));
				} else {
					thisNode = fromDEFtable;
					parentStack[parentIndex] = thisNode; 
					#ifdef X3DPARSERVERBOSE
					printf ("successful copying for field %s defName %s\n",atts[i], atts[i+1]);
					#endif

				}
			}
		} else if (strcmp("containerField",atts[i])==0) {
			indexT tmp;
			/* printf ("SETTING CONTAINER FIELD TO %s for node of type %s\n",(char *)atts[i+1], stringNodeType(thisNode->_nodeType )); */
			tmp = findFieldInFIELDNAMES((char *)atts[i+1]);
			if (tmp == INT_ID_UNDEFINED) {
				ConsoleMessage ("Error line %d: Can not set containerField to :%s: for node of type :%s:\n", LINE,
					(char *)atts[i+1], stringNodeType(thisNode->_nodeType ));
			} else {
				thisNode->_defaultContainer = tmp;
			}
		} else {
			setField_fromJavascript (thisNode, (char *)atts[i],(char *)atts[i+1], TRUE);
		}
	}
}

#endif


void linkNodeIn(char *where, int lineno) {
	int coffset;
	int ctype;
	int ctmp;
	uintptr_t *destnode;
	char *memptr;

	/* did we have a valid node here? Things like ProtoDeclares are NOT valid nodes, and we can ignore them,
	   because there will be no code associated with them */
	
	/* bounds check */
	if (parentIndex < 1) {
		ConsoleMessage ("linkNodeIn: stack underflow");
		return;
	}

	if ((parentStack[parentIndex] == NULL) || (parentStack[parentIndex-1] == NULL)) {
		ConsoleMessage ("linkNodeIn: NULL found in stack");
		return;
	}
	#ifdef X3DPARSERVERBOSE
	TTY_SPACE
	printf ("linkNodeIn at %s:%d: parserMode %s parentIndex %d, ",
			where,lineno,
			parserModeStrings[getParserMode()],parentIndex);
	printf ("linking in %s (%u) to %s (%u), field %s (%d)\n",
		stringNodeType(parentStack[parentIndex]->_nodeType),
		parentStack[parentIndex],
		stringNodeType(parentStack[parentIndex-1]->_nodeType),
		parentStack[parentIndex-1],
		stringFieldType(parentStack[parentIndex]->_defaultContainer),
		parentStack[parentIndex]->_defaultContainer);
	#endif

	/* Link it in; the parent containerField should exist, and should be an SF or MFNode  */
	findFieldInOFFSETS(parentStack[parentIndex-1]->_nodeType, 
		parentStack[parentIndex]->_defaultContainer, &coffset, &ctype, &ctmp);

	/* FreeWRL verses FreeX3D - lets see if this is a Metadatafield not following guidelines */

	if ((coffset <= 0) && (!global_strictParsing)) {
		if ((parentStack[parentIndex]->_nodeType == NODE_MetadataFloat) ||
		    (parentStack[parentIndex]->_nodeType == NODE_MetadataString) ||
		    (parentStack[parentIndex]->_nodeType == NODE_MetadataDouble) ||
		    (parentStack[parentIndex]->_nodeType == NODE_MetadataInteger)) {
			findFieldInOFFSETS(parentStack[parentIndex-1]->_nodeType, 
				FIELDNAMES_metadata, &coffset, &ctype, &ctmp);

			/*
			printf ("X3DParser - COFFSET problem, metada node: %s parent %s coffset now %d...\n", 
			stringNodeType(parentStack[parentIndex]->_nodeType),
			stringNodeType(parentStack[parentIndex-1]->_nodeType),
			coffset);
			*/

		}
		if (coffset <= 0) {
		    /* this is stated better below 
			ConsoleMessage ("X3DParser - trouble finding field %s in node %s\n",
			stringFieldType(parentStack[parentIndex]->_defaultContainer),
			stringNodeType(parentStack[parentIndex-1]->_nodeType));
		    */
		} else {
			printf ("X3DParser - warning line %d, incorrect Metadata; \"%s\" defaultContainer changed to \"metadata\"\n",
				LINE,
				stringNodeType(parentStack[parentIndex]->_nodeType));
		}
	}

	if ((ctype != FIELDTYPE_MFNode) && (ctype != FIELDTYPE_SFNode)) {
		ConsoleMessage ("X3DParser: warning, line %d: trouble linking to containerField :%s: of parent node type :%s: (specified in a :%s: node)", LINE,
			stringFieldType(parentStack[parentIndex]->_defaultContainer),
			stringNodeType(parentStack[parentIndex-1]->_nodeType),
			stringNodeType(parentStack[parentIndex]->_nodeType));
		return;
	}
	memptr = (char *)parentStack[parentIndex-1] + coffset;
	if (ctype == FIELDTYPE_SFNode) {
		/* copy over a single memory pointer */
		destnode = (uintptr_t *) memptr;
		*destnode = parentStack[parentIndex];
		ADD_PARENT(X3D_NODE(parentStack[parentIndex]), X3D_NODE(parentStack[parentIndex-1]));
	} else {
		AddRemoveChildren (
			parentStack[parentIndex-1], /* parent */
			(struct Multi_Node *) memptr,			/* where the children field is */
			((uintptr_t *) &(parentStack[parentIndex])),	/* this child, 1 node */
                1, 1,__FILE__,__LINE__);
	}
}


static void XMLCALL startCDATA (void *userData) {
	if (CDATA_Text_curlen != 0) {
/*
		ConsoleMessage ("X3DParser - hmmm, expected CDATA_Text_curlen to be 0, is not");
		printf ("CDATA_TEXT_CURLEN is %d\n",CDATA_Text_curlen);
printf ("CADAT_Text:%s:\n",CDATA_Text);
*/
		CDATA_Text_curlen = 0;
	}

	#ifdef X3DPARSERVERBOSE
	printf ("startCDATA -parentIndex %d parserMode %s\n",parentIndex,parserModeStrings[getParserMode()]);
	#endif
	inCDATA = TRUE;
}

static void XMLCALL endCDATA (void *userData) {
        #ifdef X3DPARSERVERBOSE
        printf ("endCDATA, cur index %d\n",CDATA_Text_curlen);
        printf ("endCDATA -parentIndex %d parserMode %s\n",parentIndex,parserModeStrings[getParserMode()]);
        #endif
        inCDATA = FALSE;

        if (getParserMode() == PARSING_PROTOBODY) {
                dumpCDATAtoProtoBody (CDATA_Text);
        }

        #ifdef X3DPARSERVERBOSE
        printf ("returning from EndCData\n");
        #endif


}

static void XMLCALL handleCDATA (void *userData, const char *string, int len) {
/*
	printf ("handleCDATA...(%d)...",len);
if (inCDATA) printf ("inCDATA..."); else printf ("not inCDATA...");
if (in3_3_fieldValue) printf ("in3_3_fieldValue..."); else printf ("not in3_3_fieldValue...");
printf ("\n");
*/
	/* are we in a fieldValue "dump" mode? (x3d v3.3 and above?) */
	if ((in3_3_fieldValue) || (inCDATA)) {
		appendDataToFieldValue(string,len);
	}

	/* else, ignore this data */
}


/* parse a export statement, and send the results along */
static void parseImport(const char **atts) {
	int i;

        for (i = 0; atts[i]; i += 2) {
		printf("import field:%s=%s\n", atts[i], atts[i + 1]);
	}
/* do nothing right now */
return;
}


/* parse a export statement, and send the results along */
static void parseExport(const char **atts) {
	int i;
        char *nodeToExport = NULL;
        char *alias = NULL;

        for (i = 0; atts[i]; i += 2) {
		printf("export field:%s=%s\n", atts[i], atts[i + 1]);
	}
/* do nothing right now */
return;

        	handleExport(nodeToExport, alias);
}

/* parse a component statement, and send the results along */
static void parseComponent(const char **atts) {
	int i;
	int myComponent = INT_ID_UNDEFINED;
	int myLevel = INT_ID_UNDEFINED;

	/* go through the fields and make sense of them */
        for (i = 0; atts[i]; i += 2) {
		/* printf("components field:%s=%s\n", atts[i], atts[i + 1]);  */
		if (strcmp("level",atts[i]) == 0) {
			if (sscanf(atts[i+1],"%d",&myLevel) != 1) {
				ConsoleMessage ("Line %d: Expected Component level for component %s, got %s",LINE, atts[i], atts[i+1]);
				return;
			}
		} else if (strcmp("name",atts[i]) == 0) {
			myComponent = findFieldInCOMPONENTS(atts[i+1]);
			if (myComponent == INT_ID_UNDEFINED) {
				ConsoleMessage("Line %d: Component statement, but component name not valid :%s:",LINE,atts[i+1]);
				return;
			}

		} else {
			ConsoleMessage ("Line %d: Unknown fields in Component statement :%s: :%s:",LINE,atts[i], atts[i+1]);
		}
	}

	if (myComponent == INT_ID_UNDEFINED) {
		ConsoleMessage("Line %d: Component statement, but component name not stated",LINE);
	} else if (myLevel == INT_ID_UNDEFINED) {
		ConsoleMessage("Line %d: Component statement, but component level not stated",LINE);
	} else {
		handleComponent(myComponent,myLevel);
	}
}

/* parse the <X3D profile='Immersive' version='3.0' xm... line */
static void parseX3Dhead(const char **atts) {
	int i;
	int myProfile = -10000; /* something negative, not INT_ID_UNDEFINED... */
	int versionIndex = INT_ID_UNDEFINED;

        for (i = 0; atts[i]; i += 2) {
		/* printf("parseX3Dhead: field:%s=%s\n", atts[i], atts[i + 1]); */
		if (strcmp("profile",atts[i]) == 0) {
			myProfile = findFieldInPROFILES(atts[i+1]);
		} else if (strcmp("version",atts[i]) == 0) {
			versionIndex = i+1;
		} else {
			/* printf ("just skipping this data\n"); */
		}
	}

	/* now, handle all the found variables */
	if (myProfile == INT_ID_UNDEFINED) {
		ConsoleMessage ("expected valid profile in X3D header");
	} else {
		/* printf ("X3DParsehead, myProfile %d\n",myProfile); */
		if (myProfile >= 0) handleProfile (myProfile);
	}

	if (versionIndex != INT_ID_UNDEFINED) {
		handleVersion (atts[versionIndex]);
	}
}

static void parseHeader(const char **atts) {
	int i;
        for (i = 0; atts[i]; i += 2) {
		/* printf("parseHeader: field:%s=%s\n", atts[i], atts[i + 1]); */
	}
}
static void parseScene(const char **atts) {
	int i;
        for (i = 0; atts[i]; i += 2) {
		/* printf("parseScene: field:%s=%s\n", atts[i], atts[i + 1]); */
	}
}
static void parseMeta(const char **atts) {
	int i;
        for (i = 0; atts[i]; i += 2) {
		/* printf("parseMeta field:%s=%s\n", atts[i], atts[i + 1]); */
	}
}

/* we have a fieldValue, should be in a PROTO expansion */
static void parseFieldValue(const char *name, const char **atts) {
	int i;
	int nameIndex = INT_ID_UNDEFINED;


	#ifdef X3DPARSERVERBOSE
	printf ("parseFieldValue, mode %s\n",parserModeStrings[getParserMode()]);  
	#endif

        for (i = 0; atts[i]; i += 2) {
		#ifdef X3DPARSERVERBOSE
		printf("parseFieldValue field:%s=%s\n", atts[i], atts[i + 1]);
		#endif

		if (strcmp(atts[i],"name") == 0) nameIndex= i+1;
	}

	if (getParserMode() == PARSING_PROTOINSTANCE) {
		parseProtoInstanceFields(name,atts);
	} else {
		if (in3_3_fieldValue) printf ("parseFieldValue - did not expect in3_3_fieldValue to be set\n");
		in3_3_fieldValue = TRUE;

		if (nameIndex == INT_ID_UNDEFINED) {
			printf ("did not find name field for this 3.3 fieldType test\n");
			in3_3_fieldIndex = INT_ID_UNDEFINED;
		} else {
		/* printf ("parseFieldValue field %s, parent is a %s\n",atts[nameIndex],stringNodeType(parentStack[parentIndex]->_nodeType)); */

			in3_3_fieldIndex = findFieldInFIELDNAMES(atts[nameIndex]);
		}
	}
}


static void parseIS() {
	/* printf ("parseIS mode is %s\n",parserModeStrings[getParserMode()]); */ 
	setParserMode(PARSING_IS);

}



static void endIS() {
	/* printf ("endIS mode is %s\n",parserModeStrings[getParserMode()]);  */
	setParserMode(PARSING_NODES);
}



static void endProtoInterfaceTag() {
	if (getParserMode() != PARSING_PROTOINTERFACE) {
		ConsoleMessage ("endProtoInterfaceTag: got a </ProtoInterface> but not parsing one at line %d",LINE);
	}
	/* now, a ProtoInterface should be within a ProtoDeclare, so, make the expected mode PARSING_PROTODECLARE */
	setParserMode(PARSING_PROTODECLARE);
}

static void endProtoBodyTag(const char *name) {
	/* ending <ProtoBody> */
	/* printf ("endProtoBody, mode is %s\n",parserModeStrings[getParserMode()]); */
	if (getParserMode() != PARSING_PROTOBODY) {
		ConsoleMessage ("endProtoBodyTag: got a </ProtoBody> but not parsing one at line %d",LINE);
	}

	endDumpProtoBody(name);

	/* now, a ProtoBody should be within a ProtoDeclare, so, make the expected mode PARSING_PROTODECLARE */
	setParserMode(PARSING_PROTODECLARE);
}
static void endProtoDeclareTag() {
	/* ending <ProtoDeclare> */

	if (getParserMode() != PARSING_PROTODECLARE) {
		ConsoleMessage ("endProtoDeclareTag: got a </ProtoDeclare> but not parsing one at line %d",LINE);
		setParserMode(PARSING_PROTODECLARE);
	}

	endProtoDeclare();
}

static void endProtoInstanceTag() {
	struct X3D_Group *protoExpGroup = NULL;

	/* ending <ProtoInstance> */
	#ifdef X3DPARSERVERBOSE
	printf ("endProtoInstanceTag, goot ProtoInstance got to find it, and expand it.\n");
	#endif

	if (getParserMode() != PARSING_PROTOINSTANCE) {
		ConsoleMessage ("endProtoInstanceTag: got a </ProtoInstance> but not parsing one at line %d",LINE);
	}

	/* we should just be assuming that we are parsing regular nodes for the scene graph now */
	setParserMode(PARSING_NODES);

	protoExpGroup = (struct X3D_Group *) createNewX3DNode(NODE_Group);
		#ifdef X3DPARSERVERBOSE
		if (protoExpGroup != NULL) {
			printf ("\nOK, linking in this proto. I'm %d, ps-1 is %d, and p %d\n",protoExpGroup,parentStack[parentIndex-1], parentStack[parentIndex]);
			printf ("types %s %s and %s respectively. \n",
				stringNodeType(X3D_NODE(protoExpGroup)->_nodeType),
				stringNodeType(X3D_NODE(parentStack[parentIndex-1])->_nodeType),
				stringNodeType(X3D_NODE(parentStack[parentIndex])->_nodeType));
		}
		{int i;
		for (i=parentIndex; i>=0; i--) {
			printf ("parentStack %d node %u \n",i,parentStack[i]);
			if (parentStack[i] != NULL) {
				printf ("  type %s\n",stringNodeType(parentStack[i]->_nodeType));
			}
		}
		}
		#endif

	expandProtoInstance(myLexer, protoExpGroup);

#ifdef X3DPARSERVERBOSE
printf ("after expandProtoInstance, my group (%u)has %d children, %d Meta nodes, and is protodef %d\n",
protoExpGroup,
protoExpGroup->children.n, protoExpGroup->FreeWRL_PROTOInterfaceNodes.n, protoExpGroup->FreeWRL__protoDef);
{int i;
	for (i=0; i<protoExpGroup->children.n; i++) {
		printf ("child %d is %u, type %s\n",i,protoExpGroup->children.p[i], 
			stringNodeType(X3D_GROUP(protoExpGroup->children.p[i])->_nodeType));
		if (X3D_GROUP(protoExpGroup->children.p[i])->_nodeType == NODE_Group) {
			struct X3D_Group * pxx = X3D_GROUP(protoExpGroup->children.p[i]);
			printf (" and it has %d children, %d Meta nodes, and is protodef %d\n",
			pxx->children.n, pxx->FreeWRL_PROTOInterfaceNodes.n, pxx->FreeWRL__protoDef);
		}

	}
}
#endif

}


/* did we get a USE in a proto instance, like:
               <ProtoInstance name='CamLoader' DEF='Camera1_Bgpic'>
                    <fieldValue name='imageName' value='Default.jpg'/>
                    <fieldValue name='relay'>
                        <Script USE='CameraRelay'/>
                    </fieldValue>
                    <fieldValue name='yscale' value='3.0'/>
                </ProtoInstance>

if so, we will be here for the USE fields.


*/
static void saveProtoInstanceFields (const char *name, const char **atts) {
	#ifdef X3DPARSERVERBOSE
		printf ("saveProtoInstanceFields, have node :%s:\n",name);
	#endif

	if (strcmp(name,"fieldValue") == 0) {
		parseFieldValue(name,atts);
	} else {
		/* printf ("warning - saveProtoInstanceFields - dont know what to do with %s\n",name); */
		parseFieldValue(name,atts);
	}
	#ifdef X3DPARSERVERBOSE
		printf ("saveProtoInstanceFields END\n");
	#endif
}

static void endProtoInstanceField(const char *name) {

	#ifdef X3DPARSERVERBOSE
		printf ("endProtoInstanceField, name :%s:\n",name);
	#endif
	if (strcmp(name,"ProtoInstance")==0) {
		endProtoInstanceTag();
	} else if (strcmp(name,"fieldValue") != 0) {
		/* printf ("warning - endProtoInstanceField, dont know what to do with :%s:\n",name); */
	}
}
/********************************************************/

#define INCREMENT_CHILDREN_LEVEL \
	{ \
		if ((parentIndex <0) || (parentIndex > MAX_CHILD_ATTRIBUTE_DEPTH)) {printf ("stack overflow\n"); return;} \
		if (childAttributes[parentIndex] != NULL) {printf ("hey, cpindex ne NULL\n");} \
		childAttributes[parentIndex] = newVector (struct nameValuePairs *, 8); \
	}


#define MAX_CHILD_ATTRIBUTE_DEPTH 32
static struct Vector** childAttributes= NULL;

static void saveAttributes(int myNodeType, const char *name, const char** atts) {
	struct nameValuePairs* nvp;
	int i;
	struct X3D_Node *thisNode;
	struct X3D_Node *fromDEFtable;
	
	DEBUG_X3DPARSER ("	saveAttributes, parentIndex %d parentIndex %d\n",parentIndex,parentIndex);
	
	/* create the scenegraph node for this one */
        thisNode = createNewX3DNode(myNodeType);
        parentStack[parentIndex] = thisNode;

	if (myNodeType == NODE_Script) {
		struct Shader_Script *myObj;

		/* create the Shader_Script for this one */
		X3D_SCRIPT(thisNode)->__scriptObj=new_Shader_Script(thisNode);


		#ifdef X3DPARSERVERBOSE
		printf ("working through script parentIndex %d\n",parentIndex);
		#endif

		myObj = X3D_SCRIPT(thisNode)->__scriptObj;
		JSInit(myObj->num);
	} else if (myNodeType == NODE_ComposedShader) {
		X3D_COMPOSEDSHADER(thisNode)->__shaderObj=new_Shader_Script(thisNode);
	} else if (myNodeType == NODE_ShaderProgram) {
		X3D_SHADERPROGRAM(thisNode)->__shaderObj=new_Shader_Script(thisNode);
	} else if (myNodeType == NODE_PackagedShader) {
		X3D_PACKAGEDSHADER(thisNode)->__shaderObj=new_Shader_Script(thisNode);
	}


 
	/* go through the attributes; do some here, do others later (in case of PROTO IS fields found) */
        for (i = 0; atts[i]; i += 2) {
		/* is this a DEF name? if so, record the name and then ignore the field */
		if (strcmp ("DEF",atts[i]) == 0) {
			/* printf ("saveAttributes, this is a DEF, name %s\n",atts[i+1]); */

			fromDEFtable = DEFNameIndex ((char *)atts[i+1],thisNode, TRUE);
			if (fromDEFtable != thisNode) {
				#ifdef X3DPARSERVERBOSE
				printf ("Warning - line %d duplicate DEF name: \'%s\'\n",LINE,atts[i+1]);
				#endif
			}
		} else if (strcmp ("USE",atts[i]) == 0) {
			#ifdef X3DPARSERVERBOSE
			printf ("this is a USE, name %s\n",atts[i+1]);
			#endif

			fromDEFtable = DEFNameIndex ((char *)atts[i+1],thisNode, FALSE);
			if (fromDEFtable == thisNode) {
				ConsoleMessage ("Warning - line %d DEF name: \'%s\' not found",LINE,atts[i+1]);
			} else {
				#ifdef X3DPARSERVERBOSE
				printf ("copying for field %s defName %s\n",atts[i], atts[i+1]);
				#endif

				if (fromDEFtable->_nodeType != fromDEFtable->_nodeType) {
					ConsoleMessage ("Warning, line %d DEF/USE mismatch, '%s', %s != %s", LINE,
						atts[i+1],stringNodeType(fromDEFtable->_nodeType), stringNodeType (thisNode->_nodeType));
				} else {
					thisNode = fromDEFtable;
					parentStack[parentIndex] = thisNode; 
					#ifdef X3DPARSERVERBOSE
					printf ("successful copying for field %s defName %s\n",atts[i], atts[i+1]);
					#endif

				}
			}

		/* do all the normal fields when we are ending the node */
		} else {
			nvp = MALLOC(sizeof (struct nameValuePairs));
			nvp->fieldName = STRDUP(atts[i]);
			nvp->fieldValue=STRDUP(atts[i+1]);
			vector_pushBack(struct nameValuePairs*, childAttributes[parentIndex], nvp);
		}
	}
}



static void parseAttributes(void) {
	size_t ind;
	struct nameValuePairs *nvp;
	struct X3D_Node *thisNode;

	thisNode = parentStack[parentIndex];
	/* printf  ("parseAttributes..level %d for node type %s\n",parentIndex,stringNodeType(thisNode->_nodeType)); */
	for (ind=0; ind<vector_size(childAttributes[parentIndex]); ind++) {
		nvp = vector_get(struct nameValuePairs*, childAttributes[parentIndex],ind);
		/* printf ("	nvp %d, fieldName:%s fieldValue:%s\n",ind,nvp->fieldName,nvp->fieldValue); */

		/* see if we have a containerField here */
		if (strcmp("containerField",nvp->fieldName)==0) {
			indexT tmp;
			/* printf ("SETTING CONTAINER FIELD TO %s for node of type %s\n",nvp->fieldValue, stringNodeType(thisNode->_nodeType ));  */
			tmp = findFieldInFIELDNAMES(nvp->fieldValue);
			if (tmp == INT_ID_UNDEFINED) {
				ConsoleMessage ("Error line %d: setting containerField to :%s: for node of type :%s:\n", LINE,
					nvp->fieldValue, stringNodeType(thisNode->_nodeType ));
			} else {
				thisNode->_defaultContainer = tmp;
			}
		} else {
			/* Scripts/Shaders are different from normal nodes - here we go through the initialization tables for the 
			   Script/Shader, and possibly change the initialization value - this HAS to be run before the script is
			   initialized in JSInitializeScriptAndFields of course! */
			switch (thisNode->_nodeType) {
				case NODE_Script:
        			case NODE_ComposedShader: 
        			case NODE_ShaderProgram: 
        			case NODE_PackagedShader: {
					int rv, offs, type, accessType;
					struct Shader_Script *myObj;

					/* this is a Shader/Script, look through the parameters and see if there is a replacement for value */
					rv = getRoutingInfo (myLexer, thisNode, &offs, &type, &accessType, &myObj, nvp->fieldName,0);
					/* printf ("parseAttributes, for fieldName %s value %s have offs %d type %d accessType %d rv %d\n",
nvp->fieldName, nvp->fieldValue,offs,type,accessType, rv);  */


					/* found the name, if the offset is not INT_ID_UNDEFINED */
					if (offs != INT_ID_UNDEFINED) {

					        struct ScriptParamList *thisEntry;
        					struct ScriptParamList *nextEntry;

        					thisEntry = ScriptControl[myObj->num].paramList;
        					while (thisEntry != NULL) {
        					        /* printf ("script field is %s\n",thisEntry->field); */
							if (strcmp (nvp->fieldName, thisEntry->field) == 0) {

								/* printf ("name MATCH\n");
								printf ("thisEntry->kind %d type %d value %f\n",
									thisEntry->kind,thisEntry->type,thisEntry->value); */
							if ((thisEntry->kind==PKW_initializeOnly) ||
							    (thisEntry->kind==PKW_inputOutput)) {
						                if (nvp->fieldValue== NULL) {
                        					ConsoleMessage ("PROTO connect field, an initializeOnly or inputOut needs an initialValue for name %s",nvp->fieldName);
                						} else {
									/* printf ("have to parse fieldValue :%s: and place it into my value\n",nvp->fieldValue); */
									Parser_scanStringValueToMem(X3D_NODE(&(thisEntry->value)), 0, FIELDTYPE_SFFloat, nvp->fieldValue, TRUE);
								}
							}

							}
							thisEntry=thisEntry->next;
						}
					}

				}
				break;

				default: setField_fromJavascript (thisNode, nvp->fieldName,nvp->fieldValue, TRUE);
			}
		}

		/* do not need these anymore */
		FREE_IF_NZ(nvp->fieldName);
		FREE_IF_NZ(nvp->fieldValue);
		FREE_IF_NZ(nvp);
	}
}

static void XMLCALL startElement(void *unused, const char *name, const char **atts) {
	int myNodeIndex;

	/* printf ("startElement: %s : level %d parserMode: %s \n",name,parentIndex,parserModeStrings[getParserMode()]); */


	/* are we storing a PROTO body?? */
	if (getParserMode() == PARSING_PROTOBODY) {
		dumpProtoBody(name,atts);
		return;
	}

	/* maybe we are doing a Proto Instance?? */
	if (getParserMode() == PARSING_PROTOINSTANCE) {
		saveProtoInstanceFields(name,atts);
		return;
        }



	myNodeIndex = findFieldInNODES(name);

	/* is this a "normal" node that can be found in x3d, x3dv and wrl files? */
	if (myNodeIndex != INT_ID_UNDEFINED) {
		INCREMENT_PARENTINDEX 
		DEBUG_X3DPARSER ("	creating new vector for parentIndex %d\n",parentIndex); 
		INCREMENT_CHILDREN_LEVEL
		saveAttributes(myNodeIndex,name,atts);
		return;
	}

	/* no, it is not. Lets see what else it could be... */
	myNodeIndex = findFieldInX3DSPECIAL(name);
	if (myNodeIndex != INT_ID_UNDEFINED) {
		switch (myNodeIndex) {
			case X3DSP_ProtoDeclare: parseProtoDeclare(atts); break;
			case X3DSP_ProtoBody: parseProtoBody(atts); break;
			case X3DSP_ProtoInterface: parseProtoInterface(atts); break;
			case X3DSP_ProtoInstance: parseProtoInstance(atts); break;
			case X3DSP_ROUTE: parseRoutes(atts); break;
			case X3DSP_meta: parseMeta(atts); break;
			case X3DSP_Scene: parseScene(atts); break;
			case X3DSP_head:
			case X3DSP_Header: parseHeader(atts); break;
			case X3DSP_X3D: parseX3Dhead(atts); break;
			case X3DSP_fieldValue:  parseFieldValue(name,atts); break;
			case X3DSP_field: parseScriptProtoField (myLexer, atts); break;
			case X3DSP_IS: parseIS(); break;
			case X3DSP_component: parseComponent(atts); break;
			case X3DSP_export: parseExport(atts); break;
			case X3DSP_import: parseImport(atts); break;
			case X3DSP_connect: parseConnect(myLexer, atts,childAttributes[parentIndex]); break;

			default: printf ("	huh? startElement, X3DSPECIAL, but not handled?? %d, :%s:\n",myNodeIndex,X3DSPECIAL[myNodeIndex]);
		}
		return;
	}

	printf ("startElement name  do not currently handle this one :%s: index %d\n",name,myNodeIndex); 
}

static void XMLCALL endElement(void *unused, const char *name) {
	int myNodeIndex;


	/* printf("endElement: %s : parentIndex %d mode %s\n",name,parentIndex,parserModeStrings[getParserMode()]); */




	#ifdef X3DPARSERVERBOSE
	printf ("endElement: %s : parentIndex %d mode %s\n",name,parentIndex,parserModeStrings[getParserMode()]);
	#endif

	/* are we storing a PROTO body?? */
	if (getParserMode() == PARSING_PROTOBODY) {
		/* are we finished with this ProtoBody? */
		if (strcmp("ProtoBody",name)==0) {
			/* do nothing... setParserMode(PARSING_PROTODECLARE); */
		} else {
			addToProtoCode(name);
			return;
		}
	}

	/* are we parsing a PROTO Instance still? */
	if (getParserMode() == PARSING_PROTOINSTANCE) {
		endProtoInstanceField(name);
		return;
	}

	/* is this an SFNode for a Script field? */
	if (getParserMode() == PARSING_SCRIPT) {
		switch (parentStack[parentIndex-1]->_nodeType) {
			case NODE_Script:

/* I wonder if there is a better way of handling this case */
			#ifdef X3DPARSERVERBOSE
			printf ("linkNodeIn, got parsing script, have to link node into script body\n");
        	printf ("linking in %s to %s, field %s (%d)\n",
                	stringNodeType(parentStack[parentIndex]->_nodeType),
                	stringNodeType(parentStack[parentIndex-1]->_nodeType),
                	stringFieldType(parentStack[parentIndex]->_defaultContainer),
                	parentStack[parentIndex]->_defaultContainer);
		printf ("but skipping this\n");
			#endif

			DECREMENT_PARENTINDEX
			return;
			break;

			default: {};
		}
	}
		

	myNodeIndex = findFieldInNODES(name);
	if (myNodeIndex != INT_ID_UNDEFINED) {
		DEBUG_X3DPARSER ("endElement - normalNode :%s:\n",name);
		switch (myNodeIndex) {
			case NODE_Script: initScriptWithScript(); break;
			default: linkNodeIn(__FILE__,__LINE__);
		}
		parseAttributes();
		setParserMode(PARSING_NODES);

		FREE_IF_NZ(childAttributes[parentIndex]);


		DECREMENT_PARENTINDEX
		DEBUG_X3DPARSER (" 	destroying vector for parentIndex %d\n",parentIndex); 
		return;
	}


	/* no, it is not. Lets see what else it could be... */
	myNodeIndex = findFieldInX3DSPECIAL(name);
	if (myNodeIndex != INT_ID_UNDEFINED) {
		switch (myNodeIndex) {
			case X3DSP_ProtoInterface: endProtoInterfaceTag(); break;
			case X3DSP_ProtoBody: endProtoBodyTag(name); break;
			case X3DSP_ProtoDeclare: endProtoDeclareTag(); break;
			/* case X3DSP_ProtoInstance: endProtoInstanceTag(); break; */
			case X3DSP_IS: endIS(); break;
			case X3DSP_connect:
			case X3DSP_ROUTE: 
			case X3DSP_meta:
			case X3DSP_Scene:
			case X3DSP_head:
			case X3DSP_Header:
			case X3DSP_field:
			case X3DSP_component:
			case X3DSP_X3D: break;
			case X3DSP_fieldValue:
				setFieldValueDataActive();
				break;
			
			/* should never do this: */
			default: 
			printf ("endElement: huh? X3DSPECIAL, but not handled?? %s\n",X3DSPECIAL[myNodeIndex]);
		}
		return;
	}

	printf ("unhandled endElement name %s index %d\n",name,myNodeIndex); 
	#ifdef X3DPARSERVERBOSE
	printf ("endElement %s\n",name);
	#endif
}


static XML_Parser initializeX3DParser () {
	X3DParserRecurseLevel++;

	if (X3DParserRecurseLevel >= PROTOINSTANCE_MAX_LEVELS) {
		ConsoleMessage ("XML_PARSER init: XML file PROTO nested too deep\n");
		X3DParserRecurseLevel--;
	} else {
		x3dparser[X3DParserRecurseLevel] = XML_ParserCreate(NULL);
		XML_SetElementHandler(x3dparser[X3DParserRecurseLevel], startElement, endElement);
		XML_SetCdataSectionHandler (x3dparser[X3DParserRecurseLevel], startCDATA, endCDATA);
		XML_SetDefaultHandler (x3dparser[X3DParserRecurseLevel],handleCDATA);
		XML_SetUserData(x3dparser[X3DParserRecurseLevel], &parentIndex);
	}

	return x3dparser[X3DParserRecurseLevel];
}

static void shutdownX3DParser () {
	XML_ParserFree(x3dparser[X3DParserRecurseLevel]);
	X3DParserRecurseLevel--;
	
	/* lets free up memory here for possible PROTO variables */
	if (X3DParserRecurseLevel == INT_ID_UNDEFINED) {
		/* if we are at the bottom of the parser call nesting, lets reset parentIndex */
		parentIndex = 0;
		freeProtoMemory ();
	}

	if (X3DParserRecurseLevel < INT_ID_UNDEFINED) {
		ConsoleMessage ("XML_PARSER close underflow");
		X3DParserRecurseLevel = INT_ID_UNDEFINED;
	}

	/* CDATA text space, free it up */
        FREE_IF_NZ(CDATA_Text);
        CDATA_TextMallocSize = 0; 
	if (X3DParserRecurseLevel > INT_ID_UNDEFINED)
		currentX3DParser = x3dparser[X3DParserRecurseLevel];
}

int X3DParse (struct X3D_Group* myParent, char *inputstring) {
	currentX3DParser = initializeX3DParser();

	if (childAttributes == NULL) {
		int count;
		DEBUG_X3DPARSER ("initializing childAttributes stack \n");
		childAttributes = MALLOC(sizeof(struct Vector*) * MAX_CHILD_ATTRIBUTE_DEPTH);
		for (count=0; count<MAX_CHILD_ATTRIBUTE_DEPTH; count++) childAttributes[count] = NULL;
	}


	/* Use classic parser Lexer for storing DEF name info */
	if (myLexer == NULL) myLexer = newLexer();
	if (DEFedNodes == NULL) {
		DEFedNodes = newStack(struct Vector*);
		ASSERT(DEFedNodes);
		#define DEFMEM_INIT_SIZE 16
		stack_push(struct Vector*, DEFedNodes,
        	       newVector(struct X3D_Node*, DEFMEM_INIT_SIZE));
		ASSERT(!stack_empty(DEFedNodes));
	}


	INCREMENT_PARENTINDEX
	parentStack[parentIndex] = X3D_NODE(myParent);

	DEBUG_X3DPARSER ("X3DPARSE on :\n%s:\n",inputstring);
	
	if (XML_Parse(currentX3DParser, inputstring, strlen(inputstring), TRUE) == XML_STATUS_ERROR) {
		fprintf(stderr,
			"%s at line %" XML_FMT_INT_MOD "u\n",
			XML_ErrorString(XML_GetErrorCode(currentX3DParser)),
			XML_GetCurrentLineNumber(currentX3DParser));
		shutdownX3DParser();
		return FALSE;
	}
	shutdownX3DParser();
	return TRUE;
}

