/*
  =INSERT_TEMPLATE_HERE=

  $Id: CParseParser.c,v 1.32 2009/05/19 13:09:36 crc_canada Exp $

  ???

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>


#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/CScripts.h"
#include "../world_script/fieldSet.h"
#include "../input/EAIheaders.h"
#include "../input/EAIHelpers.h"
#include "CParseParser.h"
#include "CParseLexer.h"
#include "CProto.h"
#include "CParse.h"

#define PARSE_ERROR(msg) \
 { \
  CPARSE_ERROR_CURID(msg); \
  FREE_IF_NZ(me->lexer->curID); \
  PARSER_FINALLY; \
 }
#define PARSER_FINALLY

#define DEFMEM_INIT_SIZE        16


static int foundInputErrors = 0;
void resetParseSuccessfullyFlag(void) { foundInputErrors = 0;}
int parsedSuccessfully(void) {return foundInputErrors == 0;}

/* Parsing a specific type */
/* NOTE! We have to keep the order of these function calls the same
   as the FIELDTYPE names, created from the @VRML::Fields = qw/ in
   VRMLFields.pm (which writes the FIELDTYPE* defines in 
   CFuncs/Structs.h. Currently (September, 2008) this is the list:
   SFFloat
   MFFloat
   SFRotation
   MFRotation
   SFVec3f
   MFVec3f
   SFBool
   MFBool
   SFInt32
   MFInt32
   SFNode
   MFNode
   SFColor
   MFColor
   SFColorRGBA
   MFColorRGBA
   SFTime
   MFTime
   SFString
   MFString
   SFVec2f
   MFVec2f
   SFImage
   FreeWRLPTR
   SFVec3d
   MFVec3d
   SFDouble
   MFDouble
   SFMatrix3f
   MFMatrix3f
   SFMatrix3d
   MFMatrix3d
   SFMatrix4f
   MFMatrix4f
   SFMatrix4d
   MFMatrix4d
   SFVec2d
   MFVec2d
   SFVec4f
   MFVec4f
   SFVec4d
   MFVec4d
*/

/* for those types not parsed yet, call this to print an error message */
BOOL parser_fieldTypeNotParsedYet(struct VRMLParser* me, vrmlTimeT* ret);

BOOL (*PARSE_TYPE[])(struct VRMLParser*, void*)={
    &parser_sffloatValue_, &parser_mffloatValue,
    &parser_sfrotationValue, &parser_mfrotationValue,
    &parser_sfcolorValue, &parser_mfvec3fValue,
    &parser_sfboolValue, &parser_mfboolValue,
    &parser_sfint32Value_, &parser_mfint32Value,
    &parser_sfnodeValue, &parser_mfnodeValue,
    &parser_sfcolorValue, &parser_mfcolorValue,
    &parser_sfcolorrgbaValue, &parser_mfcolorrgbaValue,
    &parser_sftimeValue, &parser_mftimeValue,
    &parser_sfstringValue_, &parser_mfstringValue,
    &parser_sfvec2fValue, &parser_mfvec2fValue,
    &parser_sfimageValue, &parser_fieldTypeNotParsedYet, /* SFImage, FreeWRLPTR */
    &parser_sfvec3dValue, &parser_mfvec3dValue,
    &parser_sftimeValue, &parser_mftimeValue,
    &parser_sfmatrix3fValue, &parser_fieldTypeNotParsedYet, /* Matrix3f */
    &parser_sfmatrix3dValue, &parser_fieldTypeNotParsedYet, /* Matrix3d */
    &parser_sfmatrix4fValue, &parser_fieldTypeNotParsedYet, /* Matrix4f */
    &parser_sfmatrix4dValue, &parser_fieldTypeNotParsedYet, /* Matrix4d */
    &parser_sfvec2dValue, &parser_fieldTypeNotParsedYet, /* Vec2d */
    &parser_sfvec4fValue, &parser_fieldTypeNotParsedYet, /* Vec4f */
    &parser_sfvec4dValue, &parser_fieldTypeNotParsedYet, /* Vec4d */

};

/* for error messages */
char fw_outline[2000];

/* Macro definitions used more than once for event processing */
/* ********************************************************** */

/* Use real size for those types? */
#define ROUTE_REAL_SIZE_sfbool  TRUE
#define ROUTE_REAL_SIZE_sfcolor TRUE
#define ROUTE_REAL_SIZE_sffloat TRUE
#define ROUTE_REAL_SIZE_sfimage FALSE
#define ROUTE_REAL_SIZE_sfint32 TRUE
#define ROUTE_REAL_SIZE_sfnode  TRUE
#define ROUTE_REAL_SIZE_sfrotation      TRUE
#define ROUTE_REAL_SIZE_sfcolorrgba      TRUE
#define ROUTE_REAL_SIZE_sfstring       FALSE 
#define ROUTE_REAL_SIZE_sftime  TRUE
#define ROUTE_REAL_SIZE_sfdouble  TRUE
#define ROUTE_REAL_SIZE_sfvec2f TRUE
#define ROUTE_REAL_SIZE_sfvec2d TRUE
#define ROUTE_REAL_SIZE_sfvec3f TRUE
#define ROUTE_REAL_SIZE_sfvec3d TRUE
#define ROUTE_REAL_SIZE_sfvec4f TRUE
#define ROUTE_REAL_SIZE_sfvec4d TRUE
#define ROUTE_REAL_SIZE_mfbool  FALSE
#define ROUTE_REAL_SIZE_mfcolor FALSE
#define ROUTE_REAL_SIZE_mfcolorrgba     FALSE
#define ROUTE_REAL_SIZE_mffloat FALSE
#define ROUTE_REAL_SIZE_mfimage FALSE
#define ROUTE_REAL_SIZE_mfint32 FALSE
#define ROUTE_REAL_SIZE_mfnode  FALSE
#define ROUTE_REAL_SIZE_mfrotation      FALSE
#define ROUTE_REAL_SIZE_mfstring        FALSE
#define ROUTE_REAL_SIZE_mftime  FALSE
#define ROUTE_REAL_SIZE_mfvec2f FALSE
#define ROUTE_REAL_SIZE_mfvec2d FALSE
#define ROUTE_REAL_SIZE_mfvec3f FALSE
#define ROUTE_REAL_SIZE_mfvec3d FALSE
#define ROUTE_REAL_SIZE_mfvec4f FALSE
#define ROUTE_REAL_SIZE_mfvec4d FALSE
#define ROUTE_REAL_SIZE_mfdouble        FALSE

#define ROUTE_REAL_SIZE_sfmatrix3f TRUE
#define ROUTE_REAL_SIZE_mfmatrix3f FALSE
#define ROUTE_REAL_SIZE_sfmatrix3d TRUE
#define ROUTE_REAL_SIZE_mfmatrix3d FALSE
#define ROUTE_REAL_SIZE_sfmatrix4f TRUE
#define ROUTE_REAL_SIZE_mfmatrix4d FALSE
#define ROUTE_REAL_SIZE_sfmatrix4d TRUE
#define ROUTE_REAL_SIZE_mfmatrix4f FALSE

/* General processing macros */
#define PROCESS_EVENT(constPre, destPre, node, field, type, var) \
 case constPre##_##field: \
  destPre##Len= \
   (ROUTE_REAL_SIZE_##type ? sizeof_member(struct X3D_##node, var) : 0); \
  destPre##Ofs=offsetof(struct X3D_##node, var); \
  break;

#define EVENT_BEGIN_NODE(fieldInd, ptr, node) \
 case NODE_##node: \
 { \
  switch(fieldInd) \
  {

#define EVENT_END_NODE(myn,fieldString) \
  default: \
printf ("EVENT_END_NODE no %s at %s:%d\n",fieldString,__FILE__,__LINE__); \
	CPARSE_ERROR_FIELDSTRING("ERROR: Unsupported event ",fieldString); \
        PARSER_FINALLY;  \
        return FALSE;  \
  } \
  break; \
 }

#define EVENT_NODE_DEFAULT \
 default: \
  PARSE_ERROR("Parser - PROCESS_EVENT: Unsupported node!")

/************************************************************************************************/
/* parse an SF/MF; return the parsed value in the defaultVal field */
BOOL parseType(struct VRMLParser* me, indexT type,   union anyVrml *defaultVal) {
    ASSERT(PARSE_TYPE[type]);
    return PARSE_TYPE[type](me, (void*)defaultVal);
}


/* put the string value of the PROTO field into the input stream */
void replaceProtoField(struct VRMLLexer *me, struct ProtoDefinition *thisProto, char *thisID, char **outTextPtr, int *outSize) {
    struct ProtoFieldDecl* pdecl=NULL;
    /* find the ascii name, and try and find it's protodefinition by type */
#ifdef CPARSERVERBOSE
    printf ("start of replaceProtoField for id %s\n",thisID);
#endif

    /* BTW - I'm not sure which array to look in, so I end up looking in them all */
    pdecl = getProtoFieldDeclaration(me,thisProto,thisID);
#ifdef CPARSERVERBOSE
    printf ("replaceProtoField, so, pdecl is %u\n",pdecl);
#endif

    if (pdecl!=NULL) {
        if (pdecl->fieldString!=NULL) {
#ifdef CPARSERVERBOSE
            printf ("PROTO FIELD VALUE IS :xxx: %s :xxx:\n",pdecl->fieldString);
            printf ("see if we need to realloc: outSize %d, curlen %d, newlen %d\n",*outSize, strlen(*outTextPtr), strlen (pdecl->fieldString));
#endif

            if (strlen(pdecl->fieldString) > *outSize-5) {
#ifdef CPARSERVERBOSE
                printf ("replaceProtoField, reallocing outTextPtr\n");
#endif

                *outSize = strlen(pdecl->fieldString) + 5;
                *outTextPtr = REALLOC(*outTextPtr,*outSize);
            }
        
            strcat (*outTextPtr,pdecl->fieldString);
        } else {
#ifdef CPARSERVERBOSE
            printf ("PROTO FIELD VALUE is NULL, just skip\n");
#endif
        
        }
    }
#ifdef CPARSERVERBOSE
    printf ("replaceProtoField returning for id %s\n",thisID);
#endif
        
}


/* ************************************************************************** */
/* Constructor and destructor */

struct VRMLParser* newParser(void* ptr, unsigned ofs, int parsingX3DfromXML) {
    struct VRMLParser* ret=MALLOC(sizeof(struct VRMLParser));
    ret->lexer=newLexer();
    ASSERT(ret->lexer);
    ret->ptr=ptr;
    ret->ofs=ofs;
    ret->curPROTO=NULL;
    ret->DEFedNodes = NULL;
    ret->PROTOs = NULL;
    ret->parsingX3DfromXML = parsingX3DfromXML;
    return ret;
}

struct VRMLParser* reuseParser(void* ptr, unsigned ofs) {
    struct VRMLParser* ret;

    /* keep the defined nodes around, etc */
    ret = globalParser;

    /* keep the old lexer around, so that the ASSERTs do not get confused with sizes of stacks, etc
       if (ret->lexer != NULL) deleteLexer(ret->lexer);
       ret->lexer=newLexer();
    */
    ASSERT(ret->lexer);
    ret->ptr=ptr;
    ret->ofs=ofs;
/* We now need to keep the PROTOS and DEFS around 
   ret->curPROTO=NULL;
   ret->DEFedNodes = NULL;
   ret->PROTOs = NULL;
*/ 

    return ret;
}

void deleteParser(struct VRMLParser* me)
{
    ASSERT(me->lexer);
    deleteLexer(me->lexer);

    FREE_IF_NZ (me);
}

static void parser_scopeOut_DEFUSE();
static void parser_scopeOut_PROTO();
void parser_destroyData(struct VRMLParser* me)
{

    /* printf ("\nCParser: parser_destroyData, destroyCParserData: , destroying data, me->DEFedNodes %u\n",me->DEFedNodes); */

    /* DEFed Nodes. */
    if(me->DEFedNodes)
    {
        while(!stack_empty(me->DEFedNodes))
            parser_scopeOut_DEFUSE(me);
        deleteStack(struct Vector*, me->DEFedNodes);
        me->DEFedNodes=NULL;
    }
    ASSERT(!me->DEFedNodes);

    /* PROTOs */
    /* FIXME: PROTOs must not be stacked here!!! */
    if(me->PROTOs)
    {
        while(!stack_empty(me->PROTOs))
            parser_scopeOut_PROTO(me);
        deleteStack(struct Vector*, me->PROTOs);
        me->PROTOs=NULL;
    }
    ASSERT(!me->PROTOs);

    lexer_destroyData(me->lexer);

    /* zero script count */
    zeroScriptHandles ();       
}

/* Scoping */

static void parser_scopeIn_DEFUSE(struct VRMLParser* me)
{
    if(!me->DEFedNodes)
        me->DEFedNodes=newStack(struct Vector*);

    ASSERT(me->DEFedNodes);
    stack_push(struct Vector*, me->DEFedNodes,
               newVector(struct X3D_Node*, DEFMEM_INIT_SIZE));
    ASSERT(!stack_empty(me->DEFedNodes));
}

/* PROTOs are scope by the parser in the PROTOs vector, and by the lexer in the userNodeTypesVec vector.  
   This is accomplished by keeping track of the number of PROTOs defined so far in a the userNodeTypesStack.  
   When a new scope is entered, the number of PROTOs defined up to this point is pushed onto the stack.  When
   we leave the scope the number of PROTOs now defined is compared to the top value of the stack, and the newest
   PROTOs are removed until the number of PROTOs defined equals the top value of the stack.

   Management of the userNodeTypesStack is accomplished by the lexer.  Therefore, scoping in PROTOs for the parser
   does nothing except to make sure that the PROTOs vector has been initialized. */ 
static void parser_scopeIn_PROTO(struct VRMLParser* me)
{
    if(!me->PROTOs) {
        me->PROTOs=newVector(struct ProtoDefinition*, DEFMEM_INIT_SIZE);
    }
}

static void parser_scopeOut_DEFUSE(struct VRMLParser* me)
{
    ASSERT(!stack_empty(me->DEFedNodes));
    /* FIXME:  Can't delete individual nodes, as they might be referenced! */
    deleteVector(struct X3D_Node*, stack_top(struct Vector*, me->DEFedNodes));
    stack_pop(struct Vector*, me->DEFedNodes);
}

/* Scoping out of PROTOs.  Check the difference between the number of PROTO definitions currently 
   available and the number of PROTO definitions available when we first entered this scope (this is
   the top value on the userNodeTypesVec stack). Remove all PROTO definitions from the PROTOs list that have 
   been added since we first entered this scope. */ 
static void parser_scopeOut_PROTO(struct VRMLParser* me)
{
    /* Do not delete the ProtoDefinitions, as they are referenced in the scene
     * graph!  TODO:  How to delete them properly? */

    vector_popBackN(struct ProtoDefinition*, me->PROTOs, lexer_getProtoPopCnt(me->lexer));
    lexer_scopeOut_PROTO(me->lexer);
}

void parser_scopeIn(struct VRMLParser* me)
{
    lexer_scopeIn(me->lexer);
    parser_scopeIn_DEFUSE(me);
    parser_scopeIn_PROTO(me);
}
void parser_scopeOut(struct VRMLParser* me)
{
    parser_scopeOut_DEFUSE(me);
    parser_scopeOut_PROTO(me);
    lexer_scopeOut(me->lexer);
}

/* ************************************************************************** */
/* The start symbol */

BOOL parser_vrmlScene(struct VRMLParser* me)
{
    /* As long as there are nodes, routes, or protos to be parsed keep doing so */
    while(TRUE)
    {
        /* Try nodeStatement */
        {
            vrmlNodeT node;
            /* This will parse a builtin node, including all nested ROUTES and PROTOs, and return
               a pointer to the node that was parsed. 
               If the node is a user-defined node (PROTO expansion) this will expand the PROTO (propagate
               all field values, and add all routes to the CRoute table), and returns a pointer to the
               root node of the scene graph for this PROTO */
#ifdef CPARSERVERBOSE
            printf("parser_vrmlScene: Try node\n");
#endif
            if(parser_nodeStatement(me, &node))
            {
                /* Add the node just parsed to the ROOT node for this scene */
                AddRemoveChildren(me->ptr, me->ptr+me->ofs, &node, 1, 1,__FILE__,__LINE__);
#ifdef CPARSERVERBOSE
                printf("parser_vrmlScene: node parsed\n");
#endif
                continue;
            }
        }

        /* Try routeStatement */
        /* Checks that the ROUTE statement is valid (i.e. that the referenced node and field combinations
           exist, and that they are compatible) and then adds the route to the CRoutes table of routes. */

#ifdef CPARSERVERBOSE
        printf("parser_vrmlScene: Try route\n");
#endif


        /* try ROUTE, COMPONENT, EXPORT, IMPORT, META, PROFILE statements here */
        BLOCK_STATEMENT(parser_vrmlScene)

            /* Try protoStatement */
            /* Add the PROTO name to the userNodeTypesVec list of names.  Create and fill in a new protoDefinition structure and add it to the PROTOs list.
               Goes through the interface declarations for the PROTO and adds each user-defined field name to the appropriate list of user-defined names (user_initializeOnly,
               user_inputOnly, Out, or user_inputOutput), creates a new protoFieldDecl for the field and adds it to the iface vector of the ProtoDefinition,
               and, in the case of fields and inputOutputs, gets the default value of the field and stores it in the protoFieldDecl.
               Parses the body of the PROTO.  Nodes are added to the scene graph for this PROTO.  Routes are parsed and a new ProtoRoute structure
               is created for each one and added to the routes vector of the ProtoDefinition.  PROTOs are recursively parsed!
            */
#ifdef CPARSERVERBOSE
            printf("parser_vrmlScene: Try proto\n");
#endif
        if(parser_protoStatement(me)) {
#ifdef CPARSERVERBOSE
            printf("parser_vrmlScene: PROTO parsed\n");
#endif
            continue;
        }

        break;
    }

    /* We were unable to keep parsing Nodes, ROUTEs, or PROTOs.  Check that this is indeed the end of the file.  If it isn't, 
       there is an error, so we return FALSE. */
    return lexer_eof(me->lexer);
}

/* ************************************************************************** */
/* Nodes and fields */

/* Parses an interface declaration and adds it to the PROTO or Script definition */
/* Adds the user-defined name to the appropriate user-defined name list (user_initializeOnly, user_inputOutput, user_inputOnly, or Out)
   Creates a protoFieldDecl or scriptFieldDecl structure to hold field data.
   Parses and stores the default value of fields and inputOutputs.
   Adds the protoFieldDecl or scriptFieldDecl to the list of fields in the ProtoDefinition or Script structure. */ 
static BOOL parser_interfaceDeclaration(struct VRMLParser* me, struct ProtoDefinition* proto, struct Shader_Script* script) {
    indexT mode;
    indexT type;
    indexT name;
    union anyVrml defaultVal;
    struct ProtoFieldDecl* pdecl=NULL;
    struct ProtoFieldDecl* pField=NULL;
    struct ScriptFieldDecl* sdecl=NULL;
    char *startOfField = NULL;


#ifdef CPARSERVERBOSE
    printf ("start of parser_interfaceDeclaration\n");
#endif


    /* Either PROTO or Script interface! */
    ASSERT((proto || script) && !(proto && script));

    /* lexer_protoFieldMode is #defined as lexer_specialID(me, r, NULL, PROTOKEYWORDS, PROTOKEYWORDS_COUNT, NULL) */
    /* Looks for the next token in the array PROTOKEYWORDS (inputOnly, outputOnly, inputOutput, field) and returns the 
       appropriate index in mode */
    if(!lexer_protoFieldMode(me->lexer, &mode)) {
#ifdef CPARSERVERBOSE
        printf ("parser_interfaceDeclaration, not lexer_protoFieldMode, returning\n");
#endif

        return FALSE;
    }

    /* Script can not take inputOutputs */
    if (script != NULL) {
    	if(script->ShaderScriptNode->_nodeType==NODE_Script && mode==PKW_inputOutput)
        	PARSE_ERROR("Scripts must not have inputOutputs!")
    }
  
            /* lexer_fieldType is #defined as lexer_specialID(me, r, NULL, FIELDTYPES, FIELDTYPES_COUNT, NULL) */
            /* Looks for the next token in the array FIELDTYPES and returns the index in type */
            if(!lexer_fieldType(me->lexer, &type))
                PARSE_ERROR("Expected fieldType after proto-field keyword!")

#ifdef CPARSERVERBOSE
                    printf ("parser_interfaceDeclaration, switching on mode %s\n",PROTOKEYWORDS[mode]);
#endif


    switch(mode)
    {
#define LEX_DEFINE_FIELDID(suff) \
   case PKW_##suff: \
    if(!lexer_define_##suff(me->lexer, &name)) \
     PARSE_ERROR("Expected fieldNameId after field type!") \
    break;

        LEX_DEFINE_FIELDID(initializeOnly)
            LEX_DEFINE_FIELDID(inputOnly)
            LEX_DEFINE_FIELDID(outputOnly)
            LEX_DEFINE_FIELDID(inputOutput)



#ifndef NDEBUG
            default:
        ASSERT(FALSE);
#endif
    }

    /* If we are parsing a PROTO, create a new  protoFieldDecl.
       If we are parsing a Script, create a new scriptFieldDecl. */
    if(proto) {
#ifdef CPARSERVERBOSE
        printf ("parser_interfaceDeclaration, calling newProtoFieldDecl\n");
#endif

        pdecl=newProtoFieldDecl(mode, type, name);
    } else {
#ifdef CPARSERVERBOSE
        printf ("parser_interfaceDeclaration, calling newScriptFieldDecl\n");
#endif

        sdecl=newScriptFieldDecl(me->lexer, mode, type, name);
    }

 
    /* If this is a field or an exposed field */ 
    if(mode==PKW_initializeOnly || mode==PKW_inputOutput) { 
#ifdef CPARSERVERBOSE
        printf ("parser_interfaceDeclaration, mode==PKW_initializeOnly || mode==PKW_inputOutput\n");
#endif


        /* Get the next token(s) from the lexer and store them in defaultVal as the appropriate type. 
           This is the default value for this field.  */
        if (script && lexer_keyword(me->lexer, KW_IS)) {
            indexT fieldE;
            indexT fieldO;

            /* Find the proto field that this field is mapped to */
            if(!lexer_field(me->lexer, NULL, NULL, &fieldO, &fieldE))
                PARSE_ERROR("Expected fieldId after IS!")

                    if(fieldO!=ID_UNDEFINED)
                    {
                        /* Get the protoFieldDeclaration for the field at index fieldO */
                        pField=protoDefinition_getField(me->curPROTO, fieldO, PKW_initializeOnly);
                        if(!pField)
                            PARSE_ERROR("IS source is no field of current PROTO!")
                                ASSERT(pField->mode==PKW_initializeOnly);
                    } else {
                        /* If the field was found in user_inputOutputs */
                        ASSERT(fieldE!=ID_UNDEFINED);
                        /* Get the protoFieldDeclaration for the inputOutput at index fieldO */
                        pField=protoDefinition_getField(me->curPROTO, fieldE, PKW_inputOutput);
                        if(!pField)
                            PARSE_ERROR("IS source is no field of current PROTO!")
                                ASSERT(pField->mode==PKW_inputOutput);
                    }
        
            /* Add this scriptfielddecl to the list of script fields mapped to this proto field */
            struct ScriptFieldInstanceInfo* sfield = newScriptFieldInstanceInfo(sdecl, script);
            vector_pushBack(struct ScriptFieldInstanceInfo*, pField->scriptDests, sfield);
            defaultVal = pField->defaultVal;

        } else {
            startOfField = me->lexer->nextIn;
            if (!parseType(me, type, &defaultVal)) {
                /* Invalid default value parsed.  Delete the proto or script declaration. */
                CPARSE_ERROR_CURID("Expected default value for field!");
                if(pdecl) deleteProtoFieldDecl(pdecl);
                if(sdecl) deleteScriptFieldDecl(sdecl);
                return FALSE;
            }
        }

        /* Store the default field value in the protoFieldDeclaration or scriptFieldDecl structure */
        if(proto) {
            pdecl->defaultVal=defaultVal;
        }
        else
        {
            ASSERT(script);
            scriptFieldDecl_setFieldValue(sdecl, defaultVal);
        }
    } else {
#ifdef CPARSERVERBOSE
        printf ("parser_interfaceDeclaration, NOT mode==PKW_initializeOnly || mode==PKW_inputOutput\n");
#endif

        /* If this is a Script inputOnly/outputOnly IS statement */
        if (script && lexer_keyword(me->lexer, KW_IS)) {
            indexT evE, evO;
            BOOL isIn = FALSE, isOut = FALSE;

#ifdef CPARSERVERBOSE
            printf ("parser_interfaceDeclaration, got IS\n");
#endif
        
            /* Get the inputOnly or outputOnly that this field IS */
            if (mode == PKW_inputOnly) {
                if (lexer_inputOnly(me->lexer, NULL, NULL, NULL, &evO, &evE)) {
                    isIn = TRUE;
                    isOut = (evE != ID_UNDEFINED);
                }
            } else {
                if (lexer_outputOnly(me->lexer, NULL, NULL, NULL, &evO, &evE)) {
                    isOut = TRUE;
                }
            }

            /* Check that the event was found somewhere ... */
            if (!isIn && !isOut)  {
#ifdef CPARSERVERBOSE
                printf ("parser_interfaceDeclaration, NOT isIn Nor isOut\n");
#endif

                return FALSE;
            }


            /* Get the Proto field definition for the field that this IS */
            pField = protoDefinition_getField(me->curPROTO, evO, isIn ? PKW_inputOnly: PKW_outputOnly); /* can handle inputOnly, outputOnly */
        
            ASSERT(pField);

            /* Add this script as a destination for this proto field */
            struct ScriptFieldInstanceInfo* sfield = newScriptFieldInstanceInfo(sdecl, script);
            vector_pushBack(struct ScriptFieldInstanceInfo*, pField->scriptDests, sfield);
        }
    }

    /* Add the new field declaration to the list of fields in the Proto or Script definition.
       For a PROTO, this means adding it to the iface vector of the ProtoDefinition. 
       For a Script, this means adding it to the fields vector of the ScriptDefinition. */ 
    if(proto) {
        /* protoDefinition_addIfaceField is #defined as vector_pushBack(struct ProtoFieldDecl*, (me)->iface, field) */
        /* Add the protoFieldDecl structure to the iface vector of the protoDefinition structure */

        /* copy the ASCII text over and save it as part of the field */
        if (startOfField != NULL) {
            unsigned int sz = me->lexer->nextIn-startOfField;
            FREE_IF_NZ(pdecl->fieldString);
            pdecl->fieldString = MALLOC (sz + 2);
            strncpy(pdecl->fieldString,startOfField,sz);
            pdecl->fieldString[sz]='\0';
        }
        #ifdef CPARSERVERBOSE
	printf ("pdecl->fieldString is :%s:\n",pdecl->fieldString);
	#endif

        protoDefinition_addIfaceField(proto, pdecl);
    } else {
        /* Add the scriptFieldDecl structure to the fields vector of the Script structure */
        ASSERT(script);
        script_addField(script, sdecl);
    }

	#ifdef CPARSERVERBOSE
	printf ("end of parser_interfaceDeclaration\n");
	#endif

    return TRUE;
}

/* Parses a protoStatement */
/* Adds the PROTO name to the userNodeTypesVec list of names.  
   Creates a new protoDefinition structure and adds it to the PROTOs list.
   Goes through the interface declarations for the PROTO and adds each user-defined field name to the appropriate list of user-defined names (user_initializeOnly, 
   user_inputOnly, user_outputOnly, or user_inputOutput), creates a new protoFieldDecl for the field and adds it to the iface vector of the ProtoDefinition, 
   and, in the case of fields and inputOutputs, gets the default value of the field and stores it in the protoFieldDecl. 
   Parses the body of the PROTO.  Nodes are added to the scene graph for this PROTO.  Routes are parsed and a new ProtoRoute structure
   is created for each one and added to the routes vector of the ProtoDefinition.  PROTOs are recursively parsed!
*/ 
BOOL parser_protoStatement(struct VRMLParser* me)
{
    indexT name;
    struct ProtoDefinition* obj;
    char *startOfBody;
    char *endOfBody;
    char *initCP;
    unsigned int bodyLen;

    /* Really a PROTO? */
    if(!lexer_keyword(me->lexer, KW_PROTO))
        return FALSE;

    /* Our name */
    /* lexer_defineNodeType is #defined as lexer_defineID(me, ret, userNodeTypesVec, FALSE) */
    /* Add the PROTO name to the userNodeTypesVec list of names return the index of the name in the list in name */ 
    if(!lexer_defineNodeType(me->lexer, &name))
        PARSE_ERROR("Expected nodeTypeId after PROTO!\n")
            ASSERT(name!=ID_UNDEFINED);

    /* Create a new blank ProtoDefinition structure to contain the data for this PROTO */
    obj=newProtoDefinition();

    /* save the name, if we can get it - it will be the last name on the list, because we will have JUST parsed it. */
    if (vector_size(me->lexer->userNodeTypesVec) != ID_UNDEFINED) {
	obj->protoName = STRDUP(vector_get(const char*, me->lexer->userNodeTypesVec, vector_size(me->lexer->userNodeTypesVec)-1));
    } else {
	printf ("warning - have proto but no name, so just copying a default string in\n");
	obj->protoName = STRDUP("noProtoNameDefined");
    }

	#ifdef CPARSERVERBOSE
	printf ("parser_protoStatement, working on proto :%s:\n",obj->protoName);
	#endif

    /* If the PROTOs stack has not yet been created, create it */
    if(!me->PROTOs) {
        parser_scopeIn_PROTO(me);
    }

    ASSERT(me->PROTOs);
    /*  ASSERT(name==vector_size(me->PROTOs)); */

    /* Add the empty ProtoDefinition structure we just created onto the PROTOs stack */
    vector_pushBack(struct ProtoDefinition*, me->PROTOs, obj);
 
    /* Now we want to fill in the information in the ProtoDefinition */

    /* Interface declarations */

    /* Make sure that the next token is a '['.  Skip over it. */
    if(!lexer_openSquare(me->lexer))
        PARSE_ERROR("Expected [ to start interface declaration!")

            /* Read the next line and parse it as an interface declaration. */
            /* Add the user-defined field name to the appropriate list of user-defined names (user_initializeOnly, user_inputOnly, Out, or user_inputOutput).
               Create a new protoFieldDecl for this field and add it to the iface vector for the ProtoDefinition obj.
               For fields and inputOutputs, get the default value of the field and store it in the protoFieldDecl. */
            while(parser_interfaceDeclaration(me, obj, NULL));

    /* Make sure that the next token is a ']'.  Skip over it. */
    if(!lexer_closeSquare(me->lexer))
        PARSE_ERROR("Expected ] after interface declaration!")


            /* PROTO body */
            /* Make sure that the next oken is a '{'.  Skip over it. */
            if(!lexer_openCurly(me->lexer))
                PARSE_ERROR("Expected { to start PROTO body!")

     /* record the start of this proto body - keep the text around */
    startOfBody = me->lexer->nextIn;
    initCP = me->lexer->startOfStringPtr[me->lexer->lexerInputLevel];

    /* Create a new vector of nodes and push it onto the DEFedNodes stack */
    /* This is the list of nodes defined for this scope */
    /* Also checks that the PROTOs vector exists, and creates it if it does not */
    parser_scopeIn(me);

    /* Parse body */
    {
        /* Store the previous currentProto for future reference, and set the PROTO we are now parsing to the currentProto */
        struct ProtoDefinition* oldCurPROTO=me->curPROTO;
        char *protoBody;

#ifdef CPARSERVERBOSE
        printf ("about to parse PROTO body; curPROTO = %u, new proto def %u\n",oldCurPROTO,obj);
#endif

        me->curPROTO=obj;

        /* just go to the end of the proto body */
        skipToEndOfOpenCurly(me->lexer,0);

        /* end of the proto body */
        /* printf ("parsing proto , obj %u me->curPROTO %u ocp %u\n",obj, me->curPROTO,oldCurPROTO); */
        endOfBody = me->lexer->nextIn;
        
        /* has a proto expansion come through here yet? */
        if (me->lexer->startOfStringPtr[me->lexer->lexerInputLevel] != initCP) { 
#ifdef CPARSERVERBOSE
#endif
            printf ("parsing proto, body changed!\n");

            startOfBody = me->lexer->startOfStringPtr[me->lexer->lexerInputLevel]; }

        bodyLen = endOfBody - startOfBody;
#ifdef CPARSERVERBOSE
        printf ("PROTO has a body of %u len\n",bodyLen);
#endif

        /* copy this proto body */
        protoBody = MALLOC (bodyLen+10);
        strncpy (protoBody, startOfBody, bodyLen);

        if (bodyLen>0) {
            /* printf ("copied this PROTO, but lets see if we need to remove a curly brace... :%c:\n",
               protoBody[bodyLen-1]); */
            if (protoBody[bodyLen-1] == '}') {
                bodyLen--;
            }
        }
        protoBody[bodyLen] = '\0';

#ifdef CPARSERVERBOSE
        printf ("*** proto body for proto %u is :%s:\n",me, protoBody); 
#endif

        tokenizeProtoBody(me->curPROTO, protoBody);
        /* FREE_IF_NZ(protoBody); */

        /* We are done parsing this proto.  Set the curPROTO to the last proto we were parsing. */
        me->curPROTO=oldCurPROTO;
    }

    /* Takes the top DEFedNodes vector off of the stack.  The local scope now refers to the next vector in the DEFedNodes stack */
    parser_scopeOut(me);

    /* Make sure that the next token is a '}'.  Skip over it. */
#ifdef CPARSERVERBOSE
    printf ("calling lexer_closeCurly at A\n");
printf ("parser_protoStatement, FINISHED proto :%s:\n",obj->protoName);
#endif

    return TRUE;
}

BOOL parser_componentStatement(struct VRMLParser* me) {
    int myComponent = ID_UNDEFINED;
    int myLevel = ID_UNDEFINED;
#define COMPSTRINGSIZE 20

    ASSERT(me->lexer);
    lexer_skip(me->lexer);

    /* Is this a COMPONENT statement? */
    if(!lexer_keyword(me->lexer, KW_COMPONENT)) return FALSE;

#ifdef CPARSERVERBOSE
    printf ("parser_componentStatement...\n");
#endif

    if(!lexer_setCurID(me->lexer)) return TRUE; /* true, because this IS a COMPONENT statement */
    ASSERT(me->lexer->curID);

    myComponent = findFieldInCOMPONENTS(me->lexer->curID);
    if (myComponent == ID_UNDEFINED) {
        CPARSE_ERROR_CURID("Invalid COMPONENT name ");
        return TRUE;
    }
                
#ifdef CPARSERVERBOSE
    printf ("my COMPONENT is %s\n",COMPONENTS[myComponent]);
#endif

    /* now, we are finished with this COMPONENT */
    FREE_IF_NZ(me->lexer->curID);

    /* we should have a ":" then an integer supportLevel */
    if (!lexer_colon(me->lexer)) {
        CPARSE_ERROR_CURID("expected colon in COMPONENT statement")
            return TRUE;
    }

    if (!parser_sfint32Value(me,&myLevel)) {
        CPARSE_ERROR_CURID("expected supportLevel")
            return TRUE;
    }   
    handleComponent(myComponent,myLevel);

    return TRUE;
}


BOOL parser_exportStatement(struct VRMLParser* me) {
    char *nodeToExport = NULL;
    char *alias = NULL; 

    ASSERT(me->lexer);
    lexer_skip(me->lexer);

    /* Is this a EXPORT statement? */
    if(!lexer_keyword(me->lexer, KW_EXPORT)) return FALSE;

#ifdef CPARSERVERBOSE
    printf ("parser_exportStatement...\n");
#endif

    if(!lexer_setCurID(me->lexer)) return TRUE; /* true, because this IS an EXPORT statement... */
    ASSERT(me->lexer->curID);

    /* save this, and find the next token... */
    nodeToExport = me->lexer->curID;
    me->lexer->curID = NULL;

    if(!lexer_setCurID(me->lexer)) return TRUE; /* true, because this Is an EXPORT statement...*/
    ASSERT(me->lexer->curID);

    /* do we have an "AS" statement? */
    if (strcmp("AS",me->lexer->curID) == 0) {
        FREE_IF_NZ(me->lexer->curID);
        if(!lexer_setCurID(me->lexer)) return TRUE; /* true, because this Is an EXPORT statement...*/
        ASSERT(me->lexer->curID);
        alias = me->lexer->curID;
    }

    /* do the EXPORT */
    handleExport(nodeToExport, alias);

    /* free things up, only as required */
    FREE_IF_NZ(nodeToExport);
    if (alias != NULL) FREE_IF_NZ(me->lexer->curID);
    return TRUE;
}

BOOL parser_importStatement(struct VRMLParser* me) {
    char *inlineNodeName = NULL;
    char *alias = NULL; 
    char *nodeToImport = NULL;

    ASSERT(me->lexer);
    lexer_skip(me->lexer);

    /* Is this a IMPORT statement? */
    if(!lexer_keyword(me->lexer, KW_IMPORT)) return FALSE;

#ifdef CPARSERVERBOSE
    printf ("parser_importStatement...\n");
#endif

    if(!lexer_setCurID(me->lexer)) return TRUE; /* true, because this IS an IMPORT statement... */
    ASSERT(me->lexer->curID);

    /* save this, and find the next token... */
    inlineNodeName = STRDUP(me->lexer->curID);
    FREE_IF_NZ(me->lexer->curID);

    /* we should have a "." then an integer supportLevel */
    if (!lexer_point(me->lexer)) {
        CPARSE_ERROR_CURID("expected period in IMPORT statement")
            return TRUE;
    }

    if(!lexer_setCurID(me->lexer)) return TRUE; /* true, because this IS an IMPORT statement... */
    ASSERT(me->lexer->curID);

    /* ok, now, we should have the nodeToImport name... */
    nodeToImport = STRDUP(me->lexer->curID);
    FREE_IF_NZ(me->lexer->curID);

    /* get the next token */
    if(!lexer_setCurID(me->lexer)) return TRUE; /* true, because this Is an IMPORT statement...*/
    ASSERT(me->lexer->curID);

    /* do we have an "AS" statement? */
    if (strcmp("AS",me->lexer->curID) == 0) {
        FREE_IF_NZ(me->lexer->curID);
        if(!lexer_setCurID(me->lexer)) return TRUE; /* true, because this Is an IMPORT statement...*/
        ASSERT(me->lexer->curID);
        alias = STRDUP(me->lexer->curID);
        FREE_IF_NZ(me->lexer->curID);
    }

    /* do the IMPORT */
    handleImport(inlineNodeName, nodeToImport, alias);

    FREE_IF_NZ (inlineNodeName);
    FREE_IF_NZ (nodeToImport);
    FREE_IF_NZ (alias);
    return TRUE;
}
BOOL parser_metaStatement(struct VRMLParser* me) {
    vrmlStringT val1, val2;

    ASSERT(me->lexer);
    lexer_skip(me->lexer);

    /* Is this a META statement? */
    if(!lexer_keyword(me->lexer, KW_META)) return FALSE;

#ifdef CPARSERVERBOSE
    printf ("parser_metaStatement...\n");
#endif

    /* META lines have 2 strings */

    /* Otherwise, a real vector */
    val1=NULL; val2 = NULL;

    if(!parser_sfstringValue (me, &val1)) {
        CPARSE_ERROR_CURID("Expected a string after a META keyword")
            }

    if(!parser_sfstringValue (me, &val2)) {
        CPARSE_ERROR_CURID("Expected a string after a META keyword")
            }

    if ((val1 != NULL) && (val2 != NULL)) { handleMetaDataStringString(val1,val2); }

    /* cleanup */
    if (val1 != NULL) {FREE_IF_NZ(val1->strptr); FREE_IF_NZ(val1);}
    if (val2 != NULL) {FREE_IF_NZ(val2->strptr); FREE_IF_NZ(val2);}
    return TRUE;
}

BOOL parser_profileStatement(struct VRMLParser* me) {
    int myProfile = ID_UNDEFINED;

    ASSERT(me->lexer);
    lexer_skip(me->lexer);

    /* Is this a PROFILE statement? */
    if(!lexer_keyword(me->lexer, KW_PROFILE)) return FALSE;

#ifdef CPARSERVERBOSE
    printf ("parser_profileStatement...\n");
#endif

    if(!lexer_setCurID(me->lexer)) return TRUE; /* true, because this IS an PROFILE statement... */
    ASSERT(me->lexer->curID);

    myProfile = findFieldInPROFILES(me->lexer->curID);

    if (myProfile != ID_UNDEFINED) {
        handleProfile(myProfile);
    } else {
        CPARSE_ERROR_CURID("Expected a profile after a PROFILE keyword")
            return TRUE;
    }
                
    /* XXX FIXME - do something with the Profile statement */
#ifdef CPARSERVERBOSE
    printf ("my profile is %d\n",myProfile);
#endif
    /* now, we are finished with this PROFILE */
    FREE_IF_NZ(me->lexer->curID);
    return TRUE;
}



/* Parses a routeStatement */
/* Checks that the ROUTE statement is valid (i.e. that the referenced node and field combinations  
   exist, and that they are compatible) and then adds the route to either the CRoutes table of routes, or adds a new ProtoRoute structure to the vector 
   ProtoDefinition->routes if we are parsing a PROTO */

BOOL parser_routeStatement(struct VRMLParser* me)
{
    indexT fromNodeIndex;
    struct X3D_Node* fromNode;
    struct ProtoDefinition* fromProto=NULL;
    indexT fromFieldO;
    indexT fromFieldE;
    indexT fromUFieldO;
    indexT fromUFieldE;
    int fromOfs = 0;
    int fromLen = 0;
    struct ScriptFieldDecl* fromScriptField=NULL;

    indexT toNodeIndex;
    struct X3D_Node* toNode;
    struct ProtoDefinition* toProto=NULL;
    indexT toFieldO;
    indexT toFieldE;
    indexT toUFieldO;
    indexT toUFieldE;
    int toOfs = 0;
    int toLen = 0;
    struct ScriptFieldDecl* toScriptField=NULL;
    int temp, tempFE, tempFO, tempTE, tempTO;

    fromFieldE = ID_UNDEFINED; fromFieldO = ID_UNDEFINED; toFieldE = ID_UNDEFINED; toFieldO = ID_UNDEFINED;


    ASSERT(me->lexer);
    lexer_skip(me->lexer);

    /* Is this a routeStatement? */
    if(!lexer_keyword(me->lexer, KW_ROUTE))
        return FALSE;

    /* Parse the elements. */
#define ROUTE_PARSE_NODEFIELD(pre, eventType) \
  /* Target-node */ \
\
  /* Look for the current token in the userNodeNames vector (DEFed names) */ \
  if(!lexer_nodeName(me->lexer, &pre##NodeIndex)) { \
       /* The current token is not a valid DEFed name.  Error. */ \
        CPARSE_ERROR_CURID("ERROR:ROUTE: Expected a valid DEF name; found \""); \
        PARSER_FINALLY;  \
        return FALSE; \
  } \
  /* Check that there are DEFedNodes in the DEFedNodes vector, and that the index given for this node is valid */ \
  ASSERT(me->DEFedNodes && !stack_empty(me->DEFedNodes) && \
   pre##NodeIndex<vector_size(stack_top(struct Vector*, me->DEFedNodes))); \
  /* Get the X3D_Node structure for the DEFed node we just looked up in the userNodeNames list */ \
  pre##Node=vector_get(struct X3D_Node*, \
   stack_top(struct Vector*, me->DEFedNodes), \
   pre##NodeIndex); \
  /* We were'nt able to get the X3D_Node structure for the DEFed node.  Error. */ \
  if (pre##Node == NULL) { \
        /* we had a bracket underflow, from what I can see. JAS */ \
        CPARSE_ERROR_CURID("ERROR:ROUTE: no DEF name found - check scoping and \"}\"s"); \
        PARSER_FINALLY;  \
        return FALSE; \
 } \
 \
  /* PROTO? */ \
  /* We got the node structure for the DEFed node. If this is a Group or a Script node do some more processing */ \
        switch(pre##Node->_nodeType) \
        { \
         case NODE_Group: \
          /* Get a pointer to the protoDefinition for this group node */ \
          pre##Proto=X3D_GROUP(pre##Node)->FreeWRL__protoDef; \
          /* printf ("routing found protoGroup of %u\n",pre##Proto); */ \
          break; \
        } \
  \
  \
  /* The next character has to be a '.' - skip over it */ \
  if(!lexer_point(me->lexer)) {\
        CPARSE_ERROR_CURID("ERROR:ROUTE: Expected \".\" after the NODE name") \
        PARSER_FINALLY;  \
        return FALSE;  \
  } \
  /* if this is a PROTO, then lets look through the tokenizedProtoBody for the REAL NODE.FIELD! */ \
  if (pre##Proto) { \
        lexer_setCurID(me->lexer); \
        /* printf ("have a proto, field is %s have to resolve this to a real node.field \n",me->lexer->curID); */ \
        if (!resolveProtoNodeField(me, pre##Proto, me->lexer->curID, &pre##Node)) return FALSE; \
	/* we KNOW that the field name is going to be "value" - see PROTO code, especially return from protoExpand() */ \
	FREE_IF_NZ(me->lexer->curID); \
	if (KW_##eventType == KW_outputOnly) me->lexer->curID = STRDUP("valueChanged"); \
	else me->lexer->curID = STRDUP("setValue"); \
	/* printf ("PROTO ROUTING TO FIELD %s in CPARSER\n",me->lexer->curID); */ \
        pre##Proto = NULL; /* forget about this being a PROTO because PROTO is expanded */ \
  } \
  \
  /* Field, user/built-in depending on whether node is a Script instance */ \
  if(!(pre##Node->_nodeType == NODE_Script)) { \
   /* This is a builtin DEFed node.  */ \
   /* Look for the next token (field of the DEFed node) in the builtin EVENT_IN/EVENT_OUT or EXPOSED_FIELD arrays */ \
   if(!lexer_##eventType(me->lexer, pre##Node, \
    &pre##FieldO, &pre##FieldE, NULL, NULL))  {\
        /* event not found in any built in array.  Error. */ \
        CPARSE_ERROR_CURID("ERROR:Expected built-in event " #eventType " after .") \
        PARSER_FINALLY;  \
        return FALSE;  \
    } \
  } else \
  { \
   /* SCRIPT - This is a user defined node type */ \
   /* Look for the next token (field of the DEFed node) in the user_inputOnly/user_outputOnly, user_inputOutput arrays */ \
   if(lexer_##eventType(me->lexer, pre##Node, \
    NULL, NULL, &pre##UFieldO, &pre##UFieldE)) \
   { \
    if(pre##UFieldO!=ID_UNDEFINED) \
    { \
     /* We found the event in user_inputOnly or Out */ \
      /* If this is a Script get the scriptFieldDecl for this event */ \
      pre##ScriptField=script_getField(X3D_SCRIPT(pre##Node)->__scriptObj, pre##UFieldO, \
       PKW_##eventType); \
    } else \
    { \
     /* We found the event in user_inputOutput */ \
      /* If this is a Script get the scriptFieldDecl for this event */ \
      pre##ScriptField=script_getField(X3D_SCRIPT(pre##Node)->__scriptObj, pre##UFieldE, \
       PKW_inputOutput); \
    } \
    if(pre##Node->_nodeType==NODE_Script && !pre##ScriptField) \
     PARSE_ERROR("Event-field invalid for this PROTO/Script!") \
   } else \
    PARSE_ERROR("Expected event" #eventType "!") \
  } \
  /* Process script routing */ \
  if(pre##Node->_nodeType == NODE_Script) \
  { \
   ASSERT(pre##ScriptField); \
   pre##Ofs=scriptFieldDecl_getRoutingOffset(pre##ScriptField); \
  } 




    /* Parse the first part of a routing statement: DEFEDNODE.event by locating the node DEFEDNODE in either the builtin or user-defined name arrays
       and locating the event in the builtin or user-defined event name arrays */
    ROUTE_PARSE_NODEFIELD(from, outputOnly);

        /* printf ("ROUTE_PARSE_NODEFIELD, found fromFieldE %d fromFieldO %d\n",fromFieldE, fromFieldO);
           printf ("fromNode %u fromProto %u fromScript %u fromNodeIndex %d\n",fromNode, fromScript, fromProto, fromNodeIndex); */

        /* Next token has to be "TO" */
        if(!lexer_keyword(me->lexer, KW_TO)) {
            /* try to make a better error message. */
            strcpy (fw_outline,"ERROR:ROUTE: Expected \"TO\" found \"");
            if (me->lexer->curID != NULL) strcat (fw_outline, me->lexer->curID); else strcat (fw_outline, "(EOF)");
            strcat (fw_outline,"\" ");
            if (fromNode != NULL) { strcat (fw_outline, " from type:"); strcat (fw_outline, stringNodeType(fromNode->_nodeType)); strcat (fw_outline, " "); }
            if (fromFieldE != ID_UNDEFINED) { strcat (fw_outline, ":"); strcat (fw_outline, EXPOSED_FIELD[fromFieldE]); strcat (fw_outline, " "); }
            if (fromFieldO != ID_UNDEFINED) { strcat (fw_outline, ":"); strcat (fw_outline, EVENT_OUT[fromFieldO]); strcat (fw_outline, " "); }

            /* PARSE_ERROR("Expected TO in ROUTE-statement!") */
            CPARSE_ERROR_CURID(fw_outline); 
            PARSER_FINALLY; 
	    return FALSE; 
        }
/* Parse the second part of a routing statement: DEFEDNODE.event by locating the node DEFEDNODE in either the builtin or user-defined name arrays 
   and locating the event in the builtin or user-defined event name arrays */
	ROUTE_PARSE_NODEFIELD(to, inputOnly);
         /* printf ("ROUTE_PARSE_NODEFIELD, found toFieldE %d toFieldO %d\n",toFieldE, toFieldO);
           printf ("toNode %u toProto %u toScript %u toNodeIndex %d\n",toNode, toScript, toProto, toNodeIndex);  */

        /* Now, do the really hard macro work... */
        /* ************************************* */

        /* Ignore the fields. */
#define FIELD(n, f, t, v)

#define END_NODE(n) EVENT_END_NODE(n,XFIELD[fromFieldE])

        /* potentially rename the Event_From and Event_To */

        /* REASON: we had a problem with (eg) set_scale in tests/27.wrl. 
           set_scale is a valid field for an Extrusion, so, it was "found". Unfortunately,
           set_scale should be changed to "scale" for an event into a Transform...

           The following code attempts to do this transformation. John Stewart - March 22 2007 */

        /* BTW - Daniel, YES this is SLOW!! It does a lot of string comparisons. But, it does
           seem to work, and it is only SLOW during ROUTE parsing, which (hopefully) there are not
           thousands of. Code efficiency changes more than welcome, from anyone. ;-) */

        tempFE=ID_UNDEFINED; tempFO=ID_UNDEFINED; tempTE=ID_UNDEFINED; tempTO=ID_UNDEFINED;

#ifdef CPARSERVERBOSE
        printf ("fromScriptField %d fromFieldE %d fromFieldO %d\n",fromScriptField,fromFieldE,fromFieldO);
        printf ("toScriptField %d toFieldE %d toFieldO %d\n",toScriptField,toFieldE,toFieldO);
#endif

        if(!fromScriptField) {
            /* fromFieldE = Daniel's code thinks this is from an inputOutput */
            /* fromFieldO = Daniel's code thinks this is from an outputOnly */

            /* If the from event was found in the EVENT_OUT array */
            /* if we have an EVENT_OUT, we have to ensure that the inputOutput is NOT used (eg, IndexedFaceSet has set_x and x fields */
            if (fromFieldO != ID_UNDEFINED) {
#ifdef CPARSERVERBOSE
                printf ("step1a, node type %s fromFieldO %d %s\n",stringNodeType(fromNode->_nodeType),fromFieldO,EVENT_OUT[fromFieldO]);
#endif

                /* ensure that the inputOutput is NOT going to be used */
                fromFieldE = ID_UNDEFINED;

                /* Try to find the index of the event in FIELDNAMES, and check that this is a valid event for the node fromNode */
                tempFO = findRoutedFieldInFIELDNAMES(fromNode,EVENT_OUT[fromFieldO],0);

                /* Field was found in FIELDNAMES, and this is a valid field?  Then find it in EVENT_OUT */
                if (tempFO != ID_UNDEFINED) {
                    tempFO = findFieldInEVENT_OUT(FIELDNAMES[tempFO]);
                }  

                /* We didn't find the event in EVENT_OUT?  Look for it in EXPOSED_FIELD */
                if (tempFO == ID_UNDEFINED) {

                    /* Try to find the index of the event in FIELDNAMES, and check that this is a valid event for the node fromNode */
                    temp = findRoutedFieldInFIELDNAMES(fromNode,EVENT_OUT[fromFieldO],0);

                    /* Field found in FIELDNAMES and it is a valid field?  Then find it in EXPOSED_FIELD */
                    if (temp != ID_UNDEFINED) tempFE = findFieldInEXPOSED_FIELD(FIELDNAMES[temp]);
                }
            } else if(fromFieldE!=ID_UNDEFINED) {
                /* If the field was found in the EXPOSED_FIELDS array */
#ifdef CPARSERVERBOSE
                printf ("step2a, node type %s fromFieldE %d %s\n",stringNodeType(fromNode->_nodeType),fromFieldE,EXPOSED_FIELD[fromFieldE]);
#endif

                /* Try to find the index of the event in FIELDNAMES, and check that this is a valid event for the node fromNode */
                tempFE = findRoutedFieldInFIELDNAMES(fromNode,EXPOSED_FIELD[fromFieldE],0);

                /* Field was found in FIELDNAMES, and this is a valid field?  Then, try to find the index of the event in EXPOSEDFIELD */
                if (tempFE != ID_UNDEFINED) tempFE = findFieldInEXPOSED_FIELD(FIELDNAMES[tempFE]);
            }
        }


        if(!toScriptField) {
            /* toFieldE = Daniel's code thinks this is from an inputOutput */
            /* toFieldO = Daniel's code thinks this is from an inputOnly */

            /* If the from event was found in the EVENT_IN array */
            /* if both are defined, use the inputOnly - eg, IndexedFaceSet might have set_coordIndex and coordIndex */
            if(toFieldO!=ID_UNDEFINED) {
#ifdef CPARSERVERBOSE
                printf ("step3a, node type %s toFieldO %d %s\n",stringNodeType(toNode->_nodeType),toFieldO,EVENT_IN[toFieldO]);
#endif
                
                /* ensure that we will NOT use the inputOutput, if one exists... */
                toFieldE = ID_UNDEFINED;

                /* Try to find the index of the event in FIELDNAMES, and check that this is a valid event for the node fromNode */
                tempTO = findRoutedFieldInFIELDNAMES(toNode,EVENT_IN[toFieldO],1);

                /* Field was found in FIELDNAMES, and this is a valid field?  Then find it in EVENT_IN */
                if (tempTO != ID_UNDEFINED) {
                    tempTO = findFieldInEVENT_IN(FIELDNAMES[tempTO]);
                }  

                /* We didn't find the event in EVENT_IN?  Look for it in EXPOSED_FIELD */
                if (tempTO == ID_UNDEFINED) {
                    /* Try to find the index of the event in FIELDNAMES, and check that this is a valid event for the node fromNode */
                    temp = findRoutedFieldInFIELDNAMES(toNode,EVENT_IN[toFieldO],1);

                    /* Field found in FIELDNAMES and it is a valid field?  Then find it in EXPOSED_FIELD */
                    if (temp != ID_UNDEFINED) tempTE = findFieldInEXPOSED_FIELD(FIELDNAMES[temp]);
                }
            }
            /* If the field was found in the EXPOSED_FIELDS array */
            else if(toFieldE!=ID_UNDEFINED) {
#ifdef CPARSERVERBOSE
                printf ("step4a, node type %s toFieldE %d %s\n",stringNodeType(toNode->_nodeType),toFieldE,EXPOSED_FIELD[toFieldE]);
#endif

                /* Try to find the index of the event in FIELDNAMES, and check that this is a valid event for the node fromNode */
                tempTE = findRoutedFieldInFIELDNAMES(toNode,EXPOSED_FIELD[toFieldE],1);

                /* Field was found in FIELDNAMES, and this is a valid field?  Then, try to find the index of the event in EXPOSEDFIELD */
                if (tempTE != ID_UNDEFINED) tempTE = findFieldInEXPOSED_FIELD(FIELDNAMES[tempTE]);
            }


        }
#ifdef CPARSERVERBOSE
        printf ("so, before routing we have: ");
        if (tempFE != ID_UNDEFINED) {printf ("from EXPOSED_FIELD %s ",EXPOSED_FIELD[tempFE]);}
        if (tempFO != ID_UNDEFINED) {printf ("from EVENT_OUT %s ",EVENT_OUT[tempFO]);}
        if (tempTE != ID_UNDEFINED) {printf ("to EXPOSED_FIELD %s ",EXPOSED_FIELD[tempTE]);}
        if (tempTO != ID_UNDEFINED) {printf ("to EVENT_IN %s ",EVENT_IN[tempTO]);}
        printf ("\n\n");
#endif

        /* so, lets try and assign what we think we have now... */
        fromFieldE = tempFE;
        fromFieldO = tempFO;
        toFieldE = tempTE;
        toFieldO = tempTO;

#undef END_NODE 
#define END_NODE(n) EVENT_END_NODE(n,EXPOSED_FIELD[fromFieldE])

        /* Process from outputOnly */
        /* Get the offset to the outputOnly in the fromNode and store it in fromOfs */
        /* Get the size of the outputOnly type and store it in fromLen */
        if(!fromScriptField)
            /* If the from field is an exposed field */
            if(fromFieldE!=ID_UNDEFINED) {
                /* Get the offset and size of this field in the fromNode */
                switch(fromNode->_nodeType) {
#define EVENT_IN(n, f, t, v)
#define EVENT_OUT(n, f, t, v)
#define EXPOSED_FIELD(node, field, type, var) \
     PROCESS_EVENT(EXPOSED_FIELD, from, node, field, type, var)
#define BEGIN_NODE(node) \
     EVENT_BEGIN_NODE(fromFieldE, fromNode, node)
#include "NodeFields.h"
#undef EVENT_IN
#undef EVENT_OUT
#undef EXPOSED_FIELD
#undef BEGIN_NODE
                    EVENT_NODE_DEFAULT;
                        }

#undef END_NODE 
#define END_NODE(n) EVENT_END_NODE(n,EVENT_OUT[fromFieldO])
                /* If the from field is an event out */
            } else if(fromFieldO!=ID_UNDEFINED) {
                /* Get the offset and size of this field in the fromNode */
                switch(fromNode->_nodeType) {
#define EVENT_IN(n, f, t, v)
#define EXPOSED_FIELD(n, f, t, v)
#define EVENT_OUT(node, field, type, var) \
     PROCESS_EVENT(EVENT_OUT, from, node, field, type, var)
#define BEGIN_NODE(node) \
     EVENT_BEGIN_NODE(fromFieldO, fromNode, node)
#include "NodeFields.h"
#undef EVENT_IN
#undef EVENT_OUT
#undef EXPOSED_FIELD
#undef BEGIN_NODE
                    EVENT_NODE_DEFAULT;
                        }
            }


#undef END_NODE 
#define END_NODE(n) EVENT_END_NODE(n,EXPOSED_FIELD[toFieldE])

        /* Process to inputOnly */
        /* Get the offset to the inputOnly in the toNode and store it in toOfs */
        /* Get the size of the outputOnly type and store it in fromLen */
        if(!toScriptField)
            /* If this is an exposed field */
            if(toFieldE!=ID_UNDEFINED) {
                /* get the offset and size of this field in the toNode */
                switch(toNode->_nodeType) {
#define EVENT_IN(n, f, t, v)
#define EVENT_OUT(n, f, t, v)
#define EXPOSED_FIELD(node, field, type, var) \
     PROCESS_EVENT(EXPOSED_FIELD, to, node, field, type, var)
#define BEGIN_NODE(node) \
     EVENT_BEGIN_NODE(toFieldE, toNode, node)
#include "NodeFields.h"
#undef EVENT_IN
#undef EVENT_OUT
#undef EXPOSED_FIELD
#undef BEGIN_NODE
                    EVENT_NODE_DEFAULT;
                        }

#undef END_NODE 
#define END_NODE(n) EVENT_END_NODE(n,EVENT_IN[toFieldO])

                /* If this is an inputOnly */
            } else if(toFieldO!=ID_UNDEFINED) {
                /* Get the offset and size of this field in the toNode */
                switch(toNode->_nodeType) {
#define EVENT_OUT(n, f, t, v)
#define EXPOSED_FIELD(n, f, t, v)
#define EVENT_IN(node, field, type, var) \
     PROCESS_EVENT(EVENT_IN, to, node, field, type, var)
#define BEGIN_NODE(node) \
     EVENT_BEGIN_NODE(toFieldO, toNode, node)
#include "NodeFields.h"
#undef EVENT_IN
#undef EVENT_OUT
#undef EXPOSED_FIELD
#undef BEGIN_NODE
                    EVENT_NODE_DEFAULT;
                        }
            }

        /* Clean up. */
#undef FIELD
#undef END_NODE


        /* set the toLen and fromLen from the PROTO/Script info, if appropriate */
        if(fromScriptField) fromLen=returnRoutingElementLength(fromScriptField->fieldDecl->type);
        if(toScriptField) toLen=returnRoutingElementLength(toScriptField->fieldDecl->type);

   /* printf ("fromScriptField %d, toScriptField %d\n",fromScriptField, toScriptField);
   printf ("fromlen %d tolen %d\n",fromLen, toLen);  */

        /* to "simple" MF nodes, we have to call a procedure to determine exactly what kind of "length" this
           node has - it is not as simple as using a "sizeof(int)" command, but, something that is interpreted at
           runtime. So, looking at "#define ROUTE_REAL_SIZE_mffloat FALSE" above, anything that is defined as FALSE
           is a complex type, and, we will have a length as 0 right here. Lets really fill in the "special" lengths
           for these ones. */
        if (toLen == 0) {
            int b,c,tmp=0;
            if (toNode != NULL) {
                if (toFieldE != ID_UNDEFINED) tmp = findRoutedFieldInFIELDNAMES(toNode,EXPOSED_FIELD[toFieldE],1);
                if (toFieldO != ID_UNDEFINED) tmp = findRoutedFieldInFIELDNAMES(toNode,EVENT_IN[toFieldO],1);
                findFieldInOFFSETS(toNode->_nodeType, tmp,  &toOfs, &b, &c);      
                toLen = returnRoutingElementLength(b);
            }
        }
        if (fromLen == 0) {
            int b,c,tmp=0;
            if (fromNode != NULL) {
                if (fromFieldE != ID_UNDEFINED) tmp = findRoutedFieldInFIELDNAMES(fromNode,EXPOSED_FIELD[fromFieldE],1);
                if (fromFieldO != ID_UNDEFINED) tmp = findRoutedFieldInFIELDNAMES(fromNode,EVENT_OUT[fromFieldO],1);
                findFieldInOFFSETS(fromNode->_nodeType, tmp,  &fromOfs, &b, &c);  
                fromLen = returnRoutingElementLength(b);
            }
        }
	/* printf ("before test, fromlen %d tolen %d\n",fromLen, toLen);  */


        /* FIXME:  Not a really safe check for types in ROUTE! */
        /* JAS - made message better. Should compare types, not lengths. */
        /* We can only ROUTE between two equivalent fields.  If the size of one field value is different from the size of the other, we have problems (i.e. can't route SFInt to MFNode) */
        if(fromLen!=toLen) {
            /* try to make a better error message. */
            strcpy (fw_outline,"ERROR:Types mismatch in ROUTE: ");
            if (fromNode != NULL) { strcat (fw_outline, " from type:"); strcat (fw_outline, stringNodeType(fromNode->_nodeType)); strcat (fw_outline, " "); }
            if (fromFieldE != ID_UNDEFINED) { strcat (fw_outline, ":"); strcat (fw_outline, EXPOSED_FIELD[fromFieldE]); strcat (fw_outline, " "); }
            if (fromFieldO != ID_UNDEFINED) { strcat (fw_outline, ":"); strcat (fw_outline, EVENT_OUT[fromFieldO]); strcat (fw_outline, " "); }

            if (toNode != NULL) { strcat (fw_outline, " to type:"); strcat (fw_outline, stringNodeType(toNode->_nodeType)); strcat (fw_outline, " "); }
            if (toFieldE != ID_UNDEFINED) { strcat (fw_outline, ":"); strcat (fw_outline, EXPOSED_FIELD[toFieldE]); strcat (fw_outline, " "); }
            if (toFieldO != ID_UNDEFINED) { strcat (fw_outline, ":"); strcat (fw_outline, EVENT_IN[toFieldO]); strcat (fw_outline, " "); }

            /* PARSE_ERROR(fw_outline) */
            CPARSE_ERROR_CURID(fw_outline); 
            PARSER_FINALLY; 
	    return FALSE; 
        }

        /* Finally, register the route. */
        /* **************************** */

        /* Calculate dir parameter */
	#define fromScript (fromNode->_nodeType==NODE_Script)
	#define toScript (toNode->_nodeType==NODE_Script)

        if(fromScript && toScript) {
        }
        else if(fromScript)
        {
            ASSERT(!toScript);
            fromOfs = scriptFieldDecl_getRoutingOffset(fromScriptField);
        } else if(toScript)
        {
            ASSERT(!fromScript);
            toOfs = scriptFieldDecl_getRoutingOffset(toScriptField);
        } else
        {
            ASSERT(!fromScript && !toScript);
        }
        /* Built-in to built-in */
        parser_registerRoute(me, fromNode, fromOfs, toNode, toOfs, toLen);
  
        return TRUE;
}


/* Register a ROUTE here */
/* If we are in a PROTO add a new ProtoRoute structure to the vector ProtoDefinition->routes */
/* Otherwise, add the ROUTE to the routing table CRoutes */
void parser_registerRoute(struct VRMLParser* me,
                          struct X3D_Node* fromNode, unsigned fromOfs,
                          struct X3D_Node* toNode, unsigned toOfs,
                          size_t len)
{
    ASSERT(me);
    if(me->curPROTO)
    {
    } else
        CRoutes_RegisterSimple(fromNode, fromOfs, toNode, toOfs, len);
}

/* parse a DEF statement. Return a pointer to a vrmlNodeT */
static vrmlNodeT* parse_KW_DEF(struct VRMLParser *me) {
    indexT ind = ID_UNDEFINED;
    vrmlNodeT node;

    /* lexer_defineNodeName is #defined as lexer_defineID(me, ret, stack_top(struct Vector*, userNodeNames), TRUE) */
    /* Checks if this node already exists in the userNodeNames vector.  If it doesn't, adds it. */
    if(!lexer_defineNodeName(me->lexer, &ind))
        PARSE_ERROR("Expected nodeNameId after DEF!\n")
            ASSERT(ind!=ID_UNDEFINED);


    /* If the DEFedNodes stack has not already been created.  If not, create new stack and add an X3D_Nodes vector to that stack */ 
    if(!me->DEFedNodes || stack_empty(me->DEFedNodes)) {
        /* printf ("parsing KW_DEF, creating new Vectors...\n"); */
        parser_scopeIn_DEFUSE(me);
    }
    ASSERT(me->DEFedNodes);
    ASSERT(!stack_empty(me->DEFedNodes));

    /* Did we just add the name to the userNodeNames vector?  If so, then the node hasn't yet been added to the DEFedNodes vector, so add it */
    ASSERT(ind<=vector_size(stack_top(struct Vector*, me->DEFedNodes)));
    if(ind==vector_size(stack_top(struct Vector*, me->DEFedNodes))) {
        vector_pushBack(struct X3D_Node*, stack_top(struct Vector*, me->DEFedNodes), NULL);
    }
    ASSERT(ind<vector_size(stack_top(struct Vector*, me->DEFedNodes)));


    /* Parse this node.  Create an X3D_Node structure of the appropriate type for this node and fill in the values for the fields
       specified.  Add any routes to the CRoutes table. Add any PROTOs to the PROTOs vector */
#ifdef CPARSERVERBOSE
    printf("parser_KW_DEF: parsing DEFed node \n");
#endif
    if(!parser_node(me, &node,ind)) {
        /* PARSE_ERROR("Expected node in DEF statement!\n") */
        /* try to make a better error message. */
        CPARSE_ERROR_CURID("ERROR:Expected an X3D node in a DEF statement, got \"");
        PARSER_FINALLY; 
	return NULL; 
    }
#ifdef CPARSERVERBOSE
    printf("parser_KW_DEF: DEFed node successfully parsed\n");
#endif

    /* Return a pointer to the node in the variable ret */
    return vector_get(struct X3D_Node*, stack_top(struct Vector*, me->DEFedNodes), ind);
}



/* parse a USE statement. Return a pointer to a vrmlNodeT */
static vrmlNodeT* parse_KW_USE(struct VRMLParser *me) {
    indexT ind;

    /* lexer_nodeName is #defined as lexer_specialID(me, NULL, ret, NULL, 0, stack_top(struct Vector*, userNodeNames)) */
    /* Look for the nodename in list of user-defined node names (userNodeNames) and return the index in ret */
    if(!lexer_nodeName(me->lexer, &ind)) {
        CPARSE_ERROR_CURID("ERROR:Expected valid DEF name after USE; found: ");
	FREE_IF_NZ(me->lexer->curID);
	return NULL;
    }
#ifdef CPARSERVERBOSE
    printf("parser_KW_USE: parsing USE\n");
#endif

    /* If we're USEing it, it has to already be defined. */
    ASSERT(ind!=ID_UNDEFINED);

    /* It also has to be in the DEFedNodes stack */
    ASSERT(me->DEFedNodes && !stack_empty(me->DEFedNodes) &&
           ind<vector_size(stack_top(struct Vector*, me->DEFedNodes)));

    #ifdef CPARSERVERBOSE
    printf ("parser_KW_USE, returning vector %u\n", vector_get(struct X3D_Node*, stack_top(struct Vector*, me->DEFedNodes), ind));
    #endif

    /* Get a pointer to the X3D_Node structure for this DEFed node and return it in ret */
    return vector_get(struct X3D_Node*, stack_top(struct Vector*, me->DEFedNodes), ind);
}



/* Parses a nodeStatement */
/* If the statement starts with DEF, and has not previously been defined, we add it to the userNodeNames and DEFedNodes vectors.  We parse the node
   and return a pointer to the node in ret.
   If the statement is a USE statement, we look up the user-defined node name in the userNodeTypesVec vector and retrieve a pointer to the node from 
   the DEFedNodes vector and return it in ret.
   Otherwise, this is just a regular node statement. We parse the node into a X3D_Node structure of the appropriate type, and return a pointer to 
   this structure in ret. 
*/ 
BOOL parser_nodeStatement(struct VRMLParser* me, vrmlNodeT* ret)
{
    ASSERT(me->lexer);

    /* A DEF-statement? */
    if(lexer_keyword(me->lexer, KW_DEF)) {
        *ret=parse_KW_DEF(me);
        return TRUE;
    }

    /* A USE-statement? */
    if(lexer_keyword(me->lexer, KW_USE)) {
        *ret=parse_KW_USE(me);
        return TRUE;
    }

    /* Otherwise, simply a node. */
    return parser_node(me, ret, ID_UNDEFINED);
}

/* Parses a node (node non-terminal) */
/* Looks up the node type on the builtin NODES list and the userNodeNames list.  
   If this is a builtin node type, creates a new X3D_Node structure of the appropriate type for the node, and then parses the statements for that node.  
   For each field statement, gets the value for that field and stores it in the X3D_Node structure.  
   For each ROUTE statement, adds the route to the CRoutes table.
   For each PROTO statement, adds the PROTO definition to te PROTOs list. 
   Return a pointer to the X3D_Node structure that holds the information for this node.
   If this is a user-defined node type (i.e. a PROTO expansion), complete the proto expansion.  
   For each field in the ProtoDefinition either parse and propagate the specified value for this field, or 
   propagate the default value of the field.  (i.e. copy the appropriate value into every node/field combination in
   the dests list.)
   For each route in the routes list of the ProtoDefinition, add the route to the CRoutes table.
   Return a pointer to the X3D_Node structure that is the scenegraph for this PROTO.
*/

BOOL parser_node(struct VRMLParser* me, vrmlNodeT* ret, indexT ind) {
    indexT nodeTypeB, nodeTypeU;
    struct X3D_Node* node=NULL;
    struct ProtoDefinition *thisProto = NULL;
        
    ASSERT(me->lexer);
    *ret=node; /* set this to NULL, for now... if this is a real node, it will return a node pointer */
         
    /* lexer_node( ... ) #defined to lexer_specialID(me, r1, r2, NODES, NODES_COUNT, userNodeTypesVec) where userNodeTypesVec is a list of PROTO defs */
    /* this will get the next token (which will be the node type) and search the NODES array for it.  If it is found in the NODES array nodeTypeB will be set to 
       the index of type in the NODES array.  If it is not in NODES, the list of user-defined nodes will be searched for the type.  If it is found in the user-defined 
       list nodeTypeU will be set to the index of the type in userNodeTypesVec.  A return value of FALSE indicates that the node type wasn't found in either list */

#ifdef CPARSERVERBOSE
    printf ("parser_node START, curID :%s: nextIn :%s:\n",me->lexer->curID, me->lexer->nextIn);
#endif


#define XBLOCK_STATEMENT(LOCATION) \
   if(parser_routeStatement(me))  { \
        return TRUE; \
   } \
 \
  if (parser_componentStatement(me)) { \
        return TRUE; \
  } \
 \
  if (parser_exportStatement(me)) { \
        return TRUE; \
  } \
 \
  if (parser_importStatement(me)) { \
        return TRUE; \
  } \
 \
  if (parser_metaStatement(me)) { \
        return TRUE; \
  } \
 \
  if (parser_profileStatement(me)) { \
        return TRUE; \
  }

    XBLOCK_STATEMENT(ddd)


        if(!lexer_node(me->lexer, &nodeTypeB, &nodeTypeU)) {
#ifdef CPARSERVERBOSE
            printf ("parser_node, not lexed - is this one of those special nodes?\n");
#endif
            return FALSE;
        }
        
    /* printf ("after lexer_node, at this point, me->lexer->curID :%s:\n",me->lexer->curID); */
    /* could this be a proto expansion?? */

    /* Checks that the next non-whitespace non-comment character is '{' and skips it. */
    if(!lexer_openCurly(me->lexer))
        PARSE_ERROR("Expected { after node-type id!")

#ifdef CPARSERVERBOSE
            printf ("parser_node: have nodeTypeB %d nodeTypeU %d\n",nodeTypeB, nodeTypeU);
#endif

    if (nodeTypeU != ID_UNDEFINED) {
        /* The node name was located in userNodeTypesVec (list of defined PROTOs), therefore this is an attempt to instantiate a PROTO */
        /* expand this PROTO, put the code right in line, and let the parser go over it as if there was never a proto here... */
        
        char *newProtoText = NULL;
        
#ifdef CPARSERVERBOSE
        printf ("nodeTypeB = ID_UNDEFINED, but nodeTypeU is %d\n",nodeTypeU);
#endif
                
        /* If this is a PROTO instantiation, then there must be at least one PROTO defined.  Also, the index retrieved for
           this PROTO must be valid. */
        ASSERT(nodeTypeU!=ID_UNDEFINED);
        ASSERT(me->PROTOs);
        ASSERT(nodeTypeU<vector_size(me->PROTOs));
                
        if (me->curPROTO == NULL) {
	    int newProtoTextLen = 0;

            /* printf ("curPROTO = NULL: before protoExpand, current stream :%s:\n",me->lexer->nextIn); */
                
            /* find and expand the PROTO definition */
            newProtoText = protoExpand(me, nodeTypeU,&thisProto,&newProtoTextLen);
                
            /* printf ("curPROTO = NULL: past protoExpand\n"); */
	    parser_fromString(me,newProtoText);
                
            /* printf ("curPROTO = NULL: past insertProtoExpansionIntoStream\n"); */
            /* and, now, lets get the id for the lexer */
            if(!lexer_node(me->lexer, &nodeTypeB, &nodeTypeU)) {
                printf ("after protoexpand lexer_node, is this one of those special ones? \n");
                FREE_IF_NZ(newProtoText);
                return FALSE;
            }
                
            /* printf ("curPROTO = NULL: after possible proto expansion, me->lexer->curID :%s: ",me->lexer->curID);
               printf ("curPROTO = NULL: and remainder, me->lexer->nextIn :%s:\n",me->lexer->nextIn); */

            if(!lexer_openCurly(me->lexer))
                PARSE_ERROR("Expected { after node-type id!")

                
                    } else {
                        /* Checks that the next non-whitespace non-comment character is an openCurly and skips it. */
                        if(!lexer_openCurly(me->lexer)) {
                            PARSE_ERROR("Expected after node-type id!")
                                } else {
                                    /* printf ("curPROTO != NULL; lets skip these fields\n"); */
                                    skipToEndOfOpenCurly(me->lexer,0);
                                    /* printf ("after skipToEndOfOpenCurly, we have :%s:\n",me->lexer->nextIn); */
                                }
                    }
        /* JAS - now done by stacked lexer FREE_IF_NZ(newProtoText); */
    }
        
    /* Built-in node */
    /* Node was found in NODES array */
    if(nodeTypeB!=ID_UNDEFINED) {
#ifdef CPARSERVERBOSE
        printf("parser_node: parsing builtin node\n");
#endif
        struct Shader_Script* script=NULL;
        struct Shader_Script* shader=NULL;
                 
        /* Get malloced struct of appropriate X3D_Node type with default values filled in */
        node=X3D_NODE(createNewX3DNode(nodeTypeB));
        ASSERT(node);
                
        /* if ind != ID_UNDEFINED, we have the first node of a DEF. Save this node pointer, in case
           some code uses it. eg: DEF xx Transform {children Script {field yy USE xx}} */
        if (ind != ID_UNDEFINED) {
            /* Set the top memmber of the DEFed nodes stack to this node */
            vector_get(struct X3D_Node*, stack_top(struct Vector*, me->DEFedNodes), ind)=node;
#ifdef CPARSERVERBOSE
            printf("parser_node: adding DEFed node (pointer %p) to DEFedNodes vector\n", node);  
#endif
        }
                
        /* Node specific initialization */
        /* From what I can tell, this only does something for Script nodes.  It sets node->__scriptObj to new_Shader_Script() */
        parser_specificInitNode(node, me);
                
        /* Set flag for Shaders/Scripts - these ones can have any number of fields */
	switch (node->_nodeType) {
		case NODE_Script: script=X3D_SCRIPT(node)->__scriptObj; break;
		case NODE_ShaderProgram: shader=X3D_SHADERPROGRAM(node)->__shaderObj; break;
		case NODE_PackagedShader: shader=X3D_PACKAGEDSHADER(node)->__shaderObj; break;
		case NODE_ComposedShader: shader=X3D_COMPOSEDSHADER(node)->__shaderObj; break;
		default: {}
	}
        
        /* As long as the lexer is returning field statements, ROUTE statements, or PROTO statements continue parsing node */
        while(TRUE)
        {
            /* Try to parse the next statement as a field.  For normal "field value" statements (i.e. position 1 0 1) this gets the value of the field from the lexer (next token(s)
               to be processed) and stores it as the appropriate type in the node.
               For IS statements (i.e. position IS mypos) this adds the node-field combo (as an offsetPointer) to the dests list for the protoFieldDecl associated with the user
               defined field (in the given case, this would be mypos).  */
#ifdef CPARSERVERBOSE
            printf("parser_node: try parsing field ... \n");
#endif
            if(parser_field(me, node)) {
#ifdef CPARSERVERBOSE
                printf("parser_node: field parsed\n");
#endif
                continue;
            }
                        
            /* Try to parse the next statement as a ROUTE (i.e. statement starts with ROUTE).  This checks that the ROUTE statement is valid (i.e. that the referenced node and field combinations  
               exist, and that they are compatible) and then adds the route to either the CRoutes table of routes, or adds a new ProtoRoute structure to the vector 
               ProtoDefinition->routes if we are parsing a PROTO */
#ifdef CPARSERVERBOSE
            printf("parser_node: try parsing ROUTE ... \n");
#endif
                        
            /* try ROUTE, COMPONENT, EXPORT, IMPORT, META, PROFILE statements here */
            BLOCK_STATEMENT(parser_node);
                        
                /* Try to parse the next statement as a PROTO (i.e. statement starts with PROTO).  */
                /* Add the PROTO name to the userNodeTypesVec list of names.  Create and fill in a new protoDefinition structure and add it to the PROTOs list.
                   Goes through the interface declarations for the PROTO and adds each user-defined field name to the appropriate list of user-defined names (user_initializeOnly, 
                   user_inputOnly, Out, or user_inputOutput), creates a new protoFieldDecl for the field and adds it to the iface vector of the ProtoDefinition, 
                   and, in the case of fields and inputOutputs, gets the default value of the field and stores it in the protoFieldDecl. 
                   Parses the body of the PROTO.  Nodes are added to the scene graph for this PROTO.  Routes are parsed and a new ProtoRoute structure
                   is created for each one and added to the routes vector of the ProtoDefinition.  PROTOs are recursively parsed!
                */
#ifdef CPARSERVERBOSE
                printf("parser_node: try parsing PROTO ... \n");
#endif
            if(parser_protoStatement(me)) {
#ifdef CPARSERVERBOSE
                printf("parser_node: PROTO parsed\n");
#endif
                continue;
            }
#ifdef CPARSERVERBOSE
            printf("parser_node: try parsing Script or Shader field\n");
#endif
            if(script && parser_interfaceDeclaration(me, NULL, script)) {
#ifdef CPARSERVERBOSE
                printf("parser_node: SCRIPT field parsed\n");
#endif
                continue;
            }
            if(shader && parser_interfaceDeclaration(me, NULL, shader)) {
#ifdef CPARSERVERBOSE
                printf("parser_node: Shader field parsed\n");
#endif
                continue;
            }
            break;
        }
                
        /* Init code for Scripts */
        if(script) {
#ifdef CPARSERVERBOSE
            printf("parser_node: try parsing SCRIPT url\n");
#endif
            script_initCodeFromMFUri(script, &X3D_SCRIPT(node)->url);
#ifdef CPARSERVERBOSE
            printf("parser_node: SCRIPT url parsed\n");
#endif
        }
                
        /* We must have a node that we've parsed at this point. */
        ASSERT(node);
    }
        
    /* Check that the node is closed by a '}', and skip this token */
#ifdef CPARSERVERBOSE
    printf ("calling lexer_closeCurly at B\n");
#endif
        
    if(!lexer_closeCurly(me->lexer)) {
        CPARSE_ERROR_CURID("ERROR: Expected a closing brace after fields of a node;")
            PARSER_FINALLY;
        return FALSE; 
    }
        
    /* Return the parsed node */

    if (thisProto != NULL) {
	#ifdef CPARSERVERBOSE
        printf ("parser_node, have a proto somewhere here\n");
	#endif

        if (node != NULL) {
	    #ifdef CPARSERVERBOSE
            printf ("parser_node, and the node is made...\n");
	    #endif

            if (node->_nodeType == NODE_Group) {
		#ifdef CPARSERVERBOSE
                printf ("and, it is a GROUP node...\n");
		#endif

                X3D_GROUP(node)->FreeWRL__protoDef = thisProto;
            }
        }
    }
    #ifdef CPARSERVERBOSE
    printf ("returning at end of parser_node, ret %u\n",node);
    if (node != NULL) printf ("and, node type is %s\n",stringNodeType(node->_nodeType));
    #endif
    *ret=node;
    return TRUE;
}

/* add_parent for Multi_Node */
void mfnode_add_parent(struct Multi_Node* node, struct X3D_Node* parent)
{
    int i;
    for(i=0; i!=node->n; ++i) {
        ADD_PARENT(node->p[i], parent);
    }
}


/* Parses a field value (literally or IS) */
/* Gets the actual value of a field and stores it in a node, or, for an IS statement, adds this node and field as a destination to the appropriate protoFieldDecl */
/* Passed pointer to the parser, an offsetPointer structure pointing to the current node and an offset to the field being parsed, type of the event value (i.e. MFString) index in FIELDTYPES, */
/* index of the field in the FIELDNAMES (or equivalent) array */
/* Parses a field value of a certain type (literally or IS) */

BOOL parser_fieldValue(struct VRMLParser* me, struct X3D_Node *node, int offs,
                       indexT type, indexT origFieldE, BOOL protoExpansion, struct ProtoDefinition* pdef, struct ProtoFieldDecl* origField)
{
#undef PARSER_FINALLY
#define PARSER_FINALLY

#ifdef CPARSERVERBOSE
    printf ("start of parser_fieldValue\n");
    printf ("me->curPROTO = %u\n",me->curPROTO);
#endif

    {
#ifdef CPARSERVERBOSE
        printf ("parser_fieldValue, not an IS\n");
#endif
        /* Get a pointer to the actual field */
#define offsetPointer_deref(t, me) \
 ((t)(((char*)(node))+offs))

        void* directRet=offsetPointer_deref(void*, ret);

        /* we could print out a type, as shown below for the first element of a Multi_Color:
           { struct Multi_Color * mc;
           mc = (struct Multi_Color *) directRet;
           printf ("directret n is %d\n",mc->n);
        
           printf ("directret orig is %u, %f %f %f\n",
           mc->p,
           mc->p[0].c[0],
           mc->p[0].c[1],
           mc->p[0].c[2]);
           }
        */

        PARSER_FINALLY;
	#ifdef CPARSERVERBOSE
	printf ("parser_fieldValue, ret %u, directRet %u\n",ret,directRet);
	#endif
 
            /* Get the actual value from the file (next token from lexer) and store it as the appropriate type in the node */
            return PARSE_TYPE[type](me, directRet);
    }

#undef PARSER_FINALLY
#define PARSER_FINALLY
}


/* Specific initialization of node fields */
void parser_specificInitNode(struct X3D_Node* n, struct VRMLParser* me)
{
    switch(n->_nodeType)
    {

#define NODE_SPECIFIC_INIT(type, code) \
   case NODE_##type: \
   { \
    struct X3D_##type* node=(struct X3D_##type*)n; \
    code \
	break; \
   }

        /* Scripts get a script object associated to them */
        NODE_SPECIFIC_INIT(Script, node->__scriptObj=new_Shader_Script(node);)
        NODE_SPECIFIC_INIT(ShaderProgram, node->__shaderObj=new_Shader_Script(node);)
        NODE_SPECIFIC_INIT(PackagedShader, node->__shaderObj=new_Shader_Script(node);)
        NODE_SPECIFIC_INIT(ComposedShader, node->__shaderObj=new_Shader_Script(node);)

            }
}

/* ************************************************************************** */
/* Built-in fields */

/* Parses a built-in field and sets it in node */
BOOL parser_field(struct VRMLParser* me, struct X3D_Node* node)
{
    indexT fieldO;
    indexT fieldE;

    ASSERT(me->lexer);

    /* printf ("start of parser_field, me->lexer->nextIn :%s:\n",me->lexer->nextIn);  */

    /* Ask the lexer to find the field (next lexer token) in either the FIELDNAMES array or the EXPOSED_FIELD array. The 
       index of the field in the array is returned in fieldO (if found in FIELDNAMES) or fieldE (if found in EXPOSED_FIELD).  
       If the fieldname is found in neither array, lexer_field will return FALSE. */
    if(!lexer_field(me->lexer, &fieldO, &fieldE, NULL, NULL))
        /* If lexer_field does return false, this is an inputOnly/outputOnly IS user_definedField statement.  Add a Offset_Pointer structure to the dests list for the protoFieldDecl for 
           the user defined field.  The Offset_Pointer structure contains a pointer to the node currently being parsed along with an offset that references the field that is linked to the
           user defined field. 

           i.e. for a statement "rotation IS myrot" the protoFieldDecl for myrot is retrieved, and an Offset_Pointer structure is added to the dests list which contains a pointer to the current 
           node and the offset for the "rotation" field in that node.  

           If we've done all this, then we've parsed the field statement completely, and we return. */ 
	return FALSE;

    /* Ignore all events */
#define EVENT_IN(n, f, t, v)
#define EVENT_OUT(n, f, t, v)

    /* End of node is the same for fields and inputOutputs */
#define END_NODE(type) \
   } \
  } \
  break;

/* The init codes used. */
#define INIT_CODE_sfnode(var) \
  ADD_PARENT(node2->var, X3D_NODE(node2));
#define INIT_CODE_mfnode(var) \
  mfnode_add_parent(&node2->var, X3D_NODE(node2));
#define INIT_CODE_sfbool(var)
#define INIT_CODE_sfcolor(var)
#define INIT_CODE_sfcolorrgba(var)
#define INIT_CODE_sffloat(var)
#define INIT_CODE_sfimage(var)
#define INIT_CODE_sfint32(var)
#define INIT_CODE_sfrotation(var)
#define INIT_CODE_sfstring(var)
#define INIT_CODE_sftime(var)
#define INIT_CODE_sfvec2f(var)
#define INIT_CODE_sfvec3f(var)
#define INIT_CODE_sfvec3d(var)
#define INIT_CODE_mfbool(var)
#define INIT_CODE_mfcolor(var)
#define INIT_CODE_mfcolorrgba(var)
#define INIT_CODE_mffloat(var)
#define INIT_CODE_mfint32(var)
#define INIT_CODE_mfrotation(var)
#define INIT_CODE_mfstring(var)
#define INIT_CODE_mftime(var)
#define INIT_CODE_mfvec2f(var)
#define INIT_CODE_mfvec3f(var)
#define INIT_CODE_mfvec3d(var)
#define INIT_CODE_sfdouble(var)
#define INIT_CODE_mfdouble(var)
#define INIT_CODE_sfvec4d(var)
#define INIT_CODE_mfmatrix3f(var)
#define INIT_CODE_mfmatrix4f(var)

#define INIT_CODE_mfmatrix3d(var)
#define INIT_CODE_mfmatrix4d(var)
#define INIT_CODE_mfvec2d(var)
#define INIT_CODE_mfvec4d(var)
#define INIT_CODE_mfvec4f(var)
#define INIT_CODE_sfmatrix3d(var)
#define INIT_CODE_sfmatrix3f(var)
#define INIT_CODE_sfmatrix4d(var)
#define INIT_CODE_sfmatrix4f(var)
#define INIT_CODE_sfvec2d(var)
#define INIT_CODE_sfvec4f(var)


/* The field type indices */
#define FTIND_sfnode    FIELDTYPE_SFNode
#define FTIND_sfbool    FIELDTYPE_SFBool
#define FTIND_sfcolor   FIELDTYPE_SFColor
#define FTIND_sfcolorrgba       FIELDTYPE_SFColorRGBA
#define FTIND_sffloat   FIELDTYPE_SFFloat
#define FTIND_sfimage   FIELDTYPE_SFImage
#define FTIND_sfint32   FIELDTYPE_SFInt32
#define FTIND_sfrotation        FIELDTYPE_SFRotation
#define FTIND_sfstring  FIELDTYPE_SFString
#define FTIND_sftime    FIELDTYPE_SFTime
#define FTIND_sfdouble  FIELDTYPE_SFDouble
#define FTIND_sfvec2f   FIELDTYPE_SFVec2f
#define FTIND_sfvec2d   FIELDTYPE_SFVec2d
#define FTIND_sfvec3f   FIELDTYPE_SFVec3f
#define FTIND_sfvec3d   FIELDTYPE_SFVec3d
#define FTIND_sfvec4f	FIELDTYPE_SFVec4f
#define FTIND_sfvec4d	FIELDTYPE_SFVec4d
#define FTIND_sfmatrix3f FIELDTYPE_SFMatrix3f
#define FTIND_sfmatrix4f FIELDTYPE_SFMatrix4f
#define FTIND_sfmatrix3d FIELDTYPE_SFMatrix3d
#define FTIND_sfmatrix4d FIELDTYPE_SFMatrix4d

#define FTIND_mfnode    FIELDTYPE_MFNode
#define FTIND_mfbool    FIELDTYPE_MFBool
#define FTIND_mfcolor   FIELDTYPE_MFColor
#define FTIND_mfcolorrgba       FIELDTYPE_MFColorRGBA
#define FTIND_mffloat   FIELDTYPE_MFFloat
#define FTIND_mfint32   FIELDTYPE_MFInt32
#define FTIND_mfrotation        FIELDTYPE_MFRotation
#define FTIND_mfstring  FIELDTYPE_MFString
#define FTIND_mftime    FIELDTYPE_MFTime
#define FTIND_mfvec2f   FIELDTYPE_MFVec2f
#define FTIND_mfvec2d   FIELDTYPE_MFVec2d
#define FTIND_mfvec3f   FIELDTYPE_MFVec3f
#define FTIND_mfvec3d   FIELDTYPE_MFVec3d
#define FTIND_mfvec4d   FIELDTYPE_MFVec4d
#define FTIND_mfvec4f   FIELDTYPE_MFVec4f
#define FTIND_mfdouble  FIELDTYPE_MFDouble
#define FTIND_mfmatrix3f FIELDTYPE_MFMatrix3f
#define FTIND_mfmatrix4f FIELDTYPE_MFMatrix4f
#define FTIND_mfmatrix3d FIELDTYPE_MFMatrix3d
#define FTIND_mfmatrix4d FIELDTYPE_MFMatrix4d
 
/* Process a field (either exposed or ordinary) generally */
/* For a normal "field value" (i.e. position 1 0 1) statement gets the actual value of the field from the file (next token(s) to be processed) and stores it in the node
   For an IS statement, adds this node-field combo as a destination to the appropriate protoFieldDecl */
#define PROCESS_FIELD(exposed, node, field, fieldType, var, fe) \
  case exposed##FIELD_##field: \
   if(!parser_fieldValue(me, \
    X3D_NODE(node2), offsetof(struct X3D_##node, var), \
    FTIND_##fieldType, fe, FALSE, NULL, NULL)) {\
    printf ("error in parser_fieldValue by call 2\n"); \
        PARSE_ERROR("Expected " #fieldType " Value for a fieldtype!") }\
   INIT_CODE_##fieldType(var) \
   return TRUE;
 
/* Default action if node is not encountered in list of known nodes */
#define NODE_DEFAULT \
  default: \
   PARSE_ERROR("Parser PROCESS_FIELD, Unsupported node!")

/* printf ("at XXX, fieldE = %d, fieldO = %d nodeType %s\n",fieldE, fieldO,stringNodeType (node->_nodeType));
   if (fieldE!=ID_UNDEFINED) printf (".... field is %s\n",EXPOSED_FIELD[fieldE]);
   if (fieldO!=ID_UNDEFINED) printf (".... field is %s\n",FIELD[fieldO]);  */

/* Field was found in EXPOSED_FIELD list.  Parse value or IS statement */
if(fieldE!=ID_UNDEFINED)
    switch(node->_nodeType)
{

/* Processes exposed fields for node */
#define BEGIN_NODE(type) \
    case NODE_##type: \
    { \
     struct X3D_##type* node2=(struct X3D_##type*)node; \
	/* printf ("at YYY, in case for node %s\n",stringNodeType(NODE_##type)); */ \
     UNUSED(node2); /* for compiler warning reductions */ \
     switch(fieldE) \
     {

/* Process exposed fields */
#define EXPOSED_FIELD(node, field, fieldType, var) \
    PROCESS_FIELD(EXPOSED_, node, field, fieldType, var, fieldE)

/* Ignore just fields */
#define FIELD(n, f, t, v)

/* Process it */
#include "NodeFields.h"

/* Undef the field-specific macros */
#undef BEGIN_NODE
#undef FIELD
#undef EXPOSED_FIELD

NODE_DEFAULT

}

/* Field was found in FIELDS list.  Parse value or IS statement */
if(fieldO!=ID_UNDEFINED)
    switch(node->_nodeType)
{

    /* Processes ordinary fields for node */
#define BEGIN_NODE(type) \
    case NODE_##type: \
    { \
     struct X3D_##type* node2=(struct X3D_##type*)node; \
     UNUSED(node2); /* for compiler warning reductions */ \
     switch(fieldO) \
     {

         /* Process fields */
#define FIELD(node, field, fieldType, var) \
    PROCESS_FIELD(, node, field, fieldType, var, ID_UNDEFINED)

         /* Ignore exposed fields */
#define EXPOSED_FIELD(n, f, t, v)

         /* Process it */
#include "NodeFields.h"

         /* Undef the field-specific macros */
#undef BEGIN_NODE
#undef FIELD
#undef EXPOSED_FIELD

         NODE_DEFAULT
      
             }

/* Clean up */
#undef END_NODE
#undef EVENT_IN
#undef EVENT_OUT

/* If field was found, return TRUE; would have happened! */
PARSE_ERROR("Unsupported field for node!")
    }

/* ************************************************************************** */
/* MF* field values */

/* take a USE field, and stuff it into a Multi*type field  - see parser_mf routines below */

static void stuffDEFUSE(void *out, vrmlNodeT in, int type) {
    /* printf ("stuff_it_in, got vrmlT vector successfully - it is a type of %s\n",stringNodeType(in->_nodeType));
       printf ("stuff_it_in, ret is %d\n",out); */

    /* convert, say, a X3D_something to a struct Multi_Node { int n; int  *p; }; */
    switch (type) {
        /* convert the node pointer into the "p" field of a Multi_MFNode */
    case FIELDTYPE_MFNode:
        /*struct Multi_Node { int n; void * *p; };*/
        ((struct Multi_Node *)out)->n=1;
        ((struct Multi_Node *)out)->p=MALLOC(sizeof(struct X3D_Node*));
        ((struct Multi_Node *)out)->p[0] = in;
        break;

    case FIELDTYPE_MFFloat:
    case FIELDTYPE_MFRotation:
    case FIELDTYPE_MFVec3f:
    case FIELDTYPE_MFBool:
    case FIELDTYPE_MFInt32:
    case FIELDTYPE_MFColor:
    case FIELDTYPE_MFColorRGBA:
    case FIELDTYPE_MFTime:
    case FIELDTYPE_MFDouble:
    case FIELDTYPE_MFString:
    case FIELDTYPE_MFVec2f:
    { int localSize;
    localSize =  returnRoutingElementLength(convertToSFType(type)); /* converts MF to equiv SF type */
    /* struct Multi_Float { int n; float  *p; }; */
    /* treat these all the same, as the data type is same size */
    ((struct Multi_Node *)out)->n=1;
    ((struct Multi_Node *)out)->p=MALLOC(localSize);
    memcpy (&((struct Multi_Node *)out)->p[0], &in, localSize);
    break;
    }
    default: {
        ConsoleMessage("VRML Parser; stuffDEFUSE, unhandled type");
    }
    }
}


/* if we expect to find a MF field, but the user only puts a SF Field, we make up the MF field with
   1 entry, and copy the data over */
static void stuffSFintoMF(void *out, uintptr_t *in, int type) {
    int rsz,elelen;

    /* printf ("stuffSFintoMF, got vrmlT vector successfully - it is a type of %s\n",FIELDTYPES[type]); */

    rsz = returnElementRowSize(type);
    elelen = returnElementLength(type);

    /* convert, say, a X3D_something to a struct Multi_Node { int n; int  *p; }; */
    switch (type) {
        /* convert the node pointer into the "p" field of a Multi_MFNode */
    case FIELDTYPE_MFNode: 
    case FIELDTYPE_MFFloat: 
    case FIELDTYPE_MFBool: 
    case FIELDTYPE_MFInt32: 
    case FIELDTYPE_MFTime: 
    case FIELDTYPE_MFDouble: 
    case FIELDTYPE_MFString: 
        /* treat these all the same, as the data type is same size */
        ((struct Multi_Node *)out)->n=1;
        ((struct Multi_Node *)out)->p=MALLOC(sizeof(float));
        ((struct Multi_Node *)out)->p[0] = *in;
        break;

    case FIELDTYPE_MFVec3f: 
    case FIELDTYPE_MFRotation: 
    case FIELDTYPE_MFColor: 
    case FIELDTYPE_MFColorRGBA: 
    case FIELDTYPE_MFVec2f: 
        /* struct Multi_Float { int n; float  *p; }; */
        /* struct Multi_Vec3f { int n; struct SFColor  *p; }; */
        /* treat these all the same, as the data type is same size */

        /* is the "old" size something other than 1? */
        /* I am not sure when this would ever happen, but one never knows... */
        if (((struct Multi_Node *)out)->n != 1) {
            FREE_IF_NZ(((struct Multi_Node *)out)->p);
            ((struct Multi_Node *)out)->n=1;
            ((struct Multi_Node *)out)->p=MALLOC(rsz * elelen);
        }

        /* { float *ptr; ptr = (float *) in; for (n=0; n<rsz; n++) { printf ("float %d is %f\n",n,*ptr); ptr++; } }  */

        memcpy (((struct Multi_Node *)out)->p, in, rsz * elelen); 
        break;
    default: {
        ConsoleMessage("VRML Parser; stuffSFintoMF, unhandled type");
    }   
    }
}

/* Parse a MF* field */
#define PARSER_MFFIELD(name, type) \
 BOOL parser_mf##name##Value(struct VRMLParser* me, struct Multi_##type* ret) { \
  struct Vector* vec=NULL; \
  vrmlNodeT RCX; \
  \
   /* printf ("start of a mfield parse for type %d curID :%s: me %u lexer %u\n",FIELDTYPE_MF##type, me->lexer->curID,me,me->lexer); */ \
     /* printf ("      str :%s:\n",me->lexer->startOfStringPtr[me->lexer->lexerInputLevel]);  */ \
   /* if (me->lexer->curID != NULL) printf ("parser_MF, have %s\n",me->lexer->curID); else printf("parser_MF, NULL\n");  */ \
\
 if (!(me->parsingX3DfromXML)) { \
          /* is this a USE statement? */ \
         if(lexer_keyword(me->lexer, KW_USE)) { \
                /* printf ("parser_MF, got a USE!\n"); */ \
                /* Get a pointer to the X3D_Node structure for this DEFed node and return it in ret */ \
                RCX=parse_KW_USE(me); \
                if (RCX == NULL) return FALSE; \
                /* so, we have a Multi_XX return val. (see Structs.h), have to get the info into a vrmlNodeT */ \
                stuffDEFUSE(ret, RCX, FIELDTYPE_MF##type); \
                return TRUE; \
         } \
         \
         else if (lexer_keyword(me->lexer, KW_DEF)) { \
                /* printf ("parser_MF, got the DEF!\n"); */ \
                /* Get a pointer to the X3D_Node structure for this DEFed node and return it in ret */ \
                RCX=parse_KW_DEF(me); \
                if (RCX == NULL) return FALSE; \
                \
                /* so, we have a Multi_XX return val. (see Structs.h), have to get the info into a vrmlNodeT */ \
                stuffDEFUSE(ret, RCX, FIELDTYPE_MF##type); \
                return TRUE; \
        } \
 }\
\
/* printf ("step 2... curID :%s:\n", me->lexer->curID); */ \
/* possibly a SFNodeish type value?? */ \
if (me->lexer->curID != NULL) { \
        /* printf ("parser_MF, curID was not null (it is %s) me %u lexer %u... lets just parse node\n",me->lexer->curID,me,me->lexer); */ \
        if (!parser_node(me, &RCX, ID_UNDEFINED)) { \
        /* if(!parser_sf##name##Value(me, RCX)) ... */ \
                return FALSE; \
        } \
        if (RCX == NULL) return FALSE; \
        /* so, we have a Multi_XX return val. (see Structs.h), have to get the info into a vrmlNodeT */ \
        stuffDEFUSE(ret, RCX, FIELDTYPE_MF##type); \
        return TRUE; \
 } \
/* Just a single value? */ \
/* NOTE: the XML parser will ALWAYS give this without the brackets */ \
if((!lexer_openSquare(me->lexer)) && (!(me->parsingX3DfromXML))) { \
        vrml##type##T RCXRet; \
        /* printf ("parser_MF, not an opensquare, lets just parse node\n"); */ \
        /* if (!parser_node(me, RCXRet, ID_UNDEFINED)) ... */ \
        if(!parser_sf##name##Value(me, &RCXRet)) { \
                return FALSE; \
        } \
        /* printf ("after sf parse rcx %u\n",RCXRet); */ \
        /* RCX is the return value, if this value IN THE VRML FILE IS ZERO, then this valid parse will fail... */ \
        /* so it is commented out if (RCX == NULL) return FALSE; */ \
        /* so, we have a Multi_XX return val. (see Structs.h), have to get the info into a vrmlNodeT */ \
        stuffSFintoMF(ret, &RCXRet, FIELDTYPE_MF##type); \
        return TRUE; \
} \
\
  /* Otherwise, a real vector */ \
  /* printf ("parser_MF, this is a real vector:%s:\n",me->lexer->nextIn); */ \
  vec=newVector(vrml##type##T, 128); \
  if (!me->parsingX3DfromXML) { \
        while(!lexer_closeSquare(me->lexer)) { \
                vrml##type##T val; \
                if(!parser_sf##name##Value(me, &val)) { \
                        CPARSE_ERROR_CURID("ERROR:Expected \"]\" before end of MF-Value") \
                         break; \
                } \
                vector_pushBack(vrml##type##T, vec, val); \
        } \
  } else { \
        lexer_skip(me->lexer); \
        while(*me->lexer->nextIn != '\0') { \
                vrml##type##T val; \
                if(!parser_sf##name##Value(me, &val)) { \
                        CPARSE_ERROR_CURID("ERROR:Expected \"]\" before end of MF-Value") \
                         break; \
                } \
                vector_pushBack(vrml##type##T, vec, val); \
                lexer_skip(me->lexer); \
        } \
  }\
  ret->n=vector_size(vec); \
  ret->p=vector_releaseData(vrml##type##T, vec); \
  \
  deleteVector(vrml##type##T, vec); \
  return TRUE; \
 } 

PARSER_MFFIELD(bool, Bool)
    PARSER_MFFIELD(color, Color)
    PARSER_MFFIELD(colorrgba, ColorRGBA)
    PARSER_MFFIELD(float, Float)
    PARSER_MFFIELD(int32, Int32)
    PARSER_MFFIELD(node, Node)
    PARSER_MFFIELD(rotation, Rotation)
    PARSER_MFFIELD(string, String)
    PARSER_MFFIELD(time, Time)
    PARSER_MFFIELD(vec2f, Vec2f)
    PARSER_MFFIELD(vec3f, Vec3f)
    PARSER_MFFIELD(vec3d, Vec3d)

/* ************************************************************************** */
/* SF* field values */

/* Parses a fixed-size vector-field of floats (SFColor, SFRotation, SFVecXf) */
#define PARSER_FIXED_VEC(name, type, cnt) \
 BOOL parser_sf##name##Value(struct VRMLParser* me, vrml##type##T* ret) \
 { \
  int i; \
  ASSERT(me->lexer); \
  for(i=0; i!=cnt; ++i) {\
   if(!parser_sffloatValue(me, ret->c+i)) \
    return FALSE; \
	/* printf ("parser_sf, %d of %d is %f\n",i,cnt,*(ret->c+i)); */ \
  }\
  return TRUE; \
 }

/* Parses a fixed-size vector-field of floats (SFColor, SFRotation, SFVecXf) */
#define PARSER_FIXED_DOUBLE_VEC(name, type, cnt) \
 BOOL parser_sf##name##Value(struct VRMLParser* me, vrml##type##T* ret) \
 { \
  int i; \
  ASSERT(me->lexer); \
  for(i=0; i!=cnt; ++i) {\
   if(!parser_sfdoubleValue_(me, ret->c+i)) \
    return FALSE; \
  }\
  return TRUE; \
 }

    BOOL parser_sfdoubleValue_(struct VRMLParser* me, vrmlFloatT* ret)
    {
        return lexer_double(me->lexer, ret);
    }
BOOL parser_sffloatValue_(struct VRMLParser* me, vrmlFloatT* ret)
    {
	return lexer_float(me->lexer, ret);
    }
BOOL parser_sfint32Value_(struct VRMLParser* me, vrmlInt32T* ret)
    {
        return lexer_int32(me->lexer, ret);
    }



static BOOL set_X3Dstring(struct VRMLLexer* me, vrmlStringT* ret) {
    /* printf ("lexer_X3DString, setting string to be :%s:\n",me->lexer->startOfStringPtr[me->lexer->lexerInputLevel]); */
    *ret=newASCIIString(me->startOfStringPtr[me->lexerInputLevel]);
    return TRUE;
}

BOOL parser_sfstringValue_(struct VRMLParser* me, vrmlStringT* ret) {
    /* are we parsing the "classic VRML" formatted string? Ie, one with
       starting and finishing quotes? */
    if (!me->parsingX3DfromXML) return lexer_string(me->lexer, ret);

    else return set_X3Dstring(me->lexer, ret);
        
    return TRUE;
}

BOOL parser_sfboolValue(struct VRMLParser* me, vrmlBoolT* ret) {
    /* are we in the VRML (x3dv) parser? */
    if (!me->parsingX3DfromXML) {
        if(lexer_keyword(me->lexer, KW_TRUE)) {
            *ret=TRUE;
            return TRUE;
        }
        if(lexer_keyword(me->lexer, KW_FALSE)) {
            *ret=FALSE;
            return TRUE;
        }
        return FALSE;
    }
    /* possibly, this is from the XML Parser */
    if (!strcmp(me->lexer->startOfStringPtr[me->lexer->lexerInputLevel],"true")) {
        *ret = TRUE;
        return TRUE;
    }
    if (!strcmp(me->lexer->startOfStringPtr[me->lexer->lexerInputLevel],"false")) {
        *ret = FALSE;
        return TRUE;
    }

    /* possibly this is from the XML parser, but there is a case problem */
    if (!global_strictParsing && (!strcmp(me->lexer->startOfStringPtr[me->lexer->lexerInputLevel],"TRUE"))) {
	CPARSE_ERROR_CURID("found upper case TRUE in XML file - should be lower case");
        *ret = TRUE;
        return TRUE;
    }
    if (!global_strictParsing && (!strcmp(me->lexer->startOfStringPtr[me->lexer->lexerInputLevel],"FALSE"))) {
	CPARSE_ERROR_CURID ("found upper case FALSE in XML file - should be lower case");
        *ret = FALSE;
        return TRUE;
    }


        
    /* Noperooni - this was from X3D, but did not parse */
    *ret = FALSE;
    return FALSE;
}

    PARSER_FIXED_VEC(color, Color, 3)
    PARSER_FIXED_VEC(colorrgba, ColorRGBA, 4)
    PARSER_FIXED_VEC(matrix3f, Matrix3f, 9)
    PARSER_FIXED_VEC(matrix4f, Matrix4f, 16)
    PARSER_FIXED_VEC(vec2f, Vec2f, 2)
    PARSER_FIXED_VEC(vec4f, Vec4f, 4)
    PARSER_FIXED_VEC(rotation, Rotation, 4)
    PARSER_FIXED_DOUBLE_VEC(vec2d, Vec2d, 2)
    PARSER_FIXED_DOUBLE_VEC(vec3d, Vec3d, 3)
    PARSER_FIXED_DOUBLE_VEC(vec4d, Vec4d, 4)
    PARSER_FIXED_DOUBLE_VEC(matrix3d, Matrix3d, 9)
    PARSER_FIXED_DOUBLE_VEC(matrix4d, Matrix4d, 16)

/* JAS this code assumes that the ret points to a SFInt_32 type, and just
   fills in the values. */
 
    BOOL parser_sfimageValue(struct VRMLParser* me, vrmlImageT* ret)
    {
        vrmlInt32T width, height, depth;
        vrmlInt32T* ptr;
 
        if(!lexer_int32(me->lexer, &width))
            return FALSE;
        if(!lexer_int32(me->lexer, &height))
            return FALSE;
        if(!lexer_int32(me->lexer, &depth))
            return FALSE;


        ret->n=3+width*height;
        ret->p=MALLOC(sizeof(int) * ret->n);
        ret->p[0]=width;
        ret->p[1]=height;
        ret->p[2]=depth;

        for(ptr=ret->p+3; ptr!=ret->p+ret->n; ++ptr)
            if(!lexer_int32(me->lexer, ptr))
            {
                FREE_IF_NZ(ret->p);
                ret->n=0;
                return FALSE;
            }

        return TRUE;
    }


BOOL parser_sfnodeValue(struct VRMLParser* me, vrmlNodeT* ret) {
    unsigned tmp;

    ASSERT(me->lexer);
    if(lexer_keyword(me->lexer, KW_NULL)) {
        *ret=NULL;
        return TRUE;
    }

    /* are we parsing from a proto expansion? */
    if (!me->parsingX3DfromXML) {
        return parser_nodeStatement(me, ret);
    } else {
        /* expect something like a number (memory pointer) to be here */
        if (sscanf(me->lexer->startOfStringPtr[me->lexer->lexerInputLevel], "%u",  &tmp) != 1) {
            CPARSE_ERROR_FIELDSTRING ("error finding SFNode id on line :%s:",me->lexer->startOfStringPtr[me->lexer->lexerInputLevel]);
            *ret=NULL;
            return FALSE;
        }
        *ret = (vrmlNodeT*)tmp;
    }
    return TRUE;
}


BOOL parser_sftimeValue(struct VRMLParser* me, vrmlTimeT* ret)
    {
        vrmlFloatT f;

        ASSERT(me->lexer);
        if(!lexer_float(me->lexer, &f))
            return FALSE;

        *ret=(vrmlTimeT)f;
        return TRUE;
    }


BOOL parser_fieldTypeNotParsedYet(struct VRMLParser* me, vrmlTimeT* ret) {
    CPARSE_ERROR_CURID ("received a request to parse a type not supported yet");
    return FALSE;
}


/* prettyprint this error */
	#define OUTLINELEN 	800
	#define FROMSRC		140
void cParseErrorCurID(struct VRMLParser *me, char *str) {
	char fw_outline[OUTLINELEN];

	if (strlen(str) > FROMSRC) str[FROMSRC] = '\0';
	strcpy(fw_outline,str);
	if (me->lexer->curID != ((void *)0)) strcat (fw_outline, me->lexer->curID); 
	if (me->lexer->nextIn != NULL) {
		strcat (fw_outline," at: \"");
		strncat(fw_outline,me->lexer->nextIn,FROMSRC);
		if (strlen(me->lexer->nextIn) > FROMSRC)
			strcat (fw_outline,"...");
		strcat (fw_outline,"\"");
	}

	foundInputErrors++;
	ConsoleMessage(fw_outline); 
}

void cParseErrorFieldString(struct VRMLParser *me, char *str, const char *str2) {

	char fw_outline[OUTLINELEN];
	int str2len = strlen(str2);

	if (strlen(str) > FROMSRC) str[FROMSRC] = '\0';
	strcpy(fw_outline,str);
	strcat (fw_outline," (");
	strncat (fw_outline,str2,str2len);
	strcat (fw_outline, ") ");
	if (me->lexer->curID != ((void *)0)) strcat (fw_outline, me->lexer->curID); 
	if (me->lexer->nextIn != NULL) {
		strcat (fw_outline," at: \"");
		strncat(fw_outline,me->lexer->nextIn,FROMSRC);
		if (strlen(me->lexer->nextIn) > FROMSRC)
			strcat (fw_outline,"...");
		strcat (fw_outline,"\"");
	}

	foundInputErrors++;
	ConsoleMessage(fw_outline); 
}


