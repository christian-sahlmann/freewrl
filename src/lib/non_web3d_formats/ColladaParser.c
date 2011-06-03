/*
=INSERT_TEMPLATE_HERE=

$Id: ColladaParser.c,v 1.19 2011/06/03 19:45:06 crc_canada Exp $

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
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/JScript.h"
#include "../world_script/CScripts.h"
#include "../world_script/fieldSet.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CProto.h"
#include "../vrml_parser/CParse.h"
#include "../input/EAIHeaders.h"        /* resolving implicit declarations */
#include "../input/EAIHelpers.h"        /* resolving implicit declarations */


#include "ColladaParser.h"


#define INCREMENT_PARENTINDEXC \
        if (parentIndex < (PARENTSTACKSIZE-2))  { \
                parentIndex++; \
                p->colladaParentStack[parentIndex] = NULL; /* make sure we know the state of the new Top of Stack */ \
        } else ConsoleMessage ("ColladaParser, line %d stack overflow",LINE);

#include <libxml/parser.h>
typedef xmlSAXHandler* XML_Parser;

/* for now - fill this in later */
#define XML_GetCurrentLineNumber(aaa) -1L
#define XML_ParserFree(aaa) FREE_IF_NZ(aaa)
#define XML_SetUserData(aaa,bbb)
#define XML_STATUS_ERROR -1
#define XML_GetErrorCode(aaa)
#define XML_ErrorString(aaa) "errors not currently being reported by libxml port"


static int XML_ParseFile(xmlSAXHandler *me, const char *myinput, int myinputlen, int recovery) {
	int notUsed;

	if (xmlSAXUserParseMemory(me, &notUsed, myinput,myinputlen) == 0) return 0;
	return XML_STATUS_ERROR;
}


/* basic parser stuff */
#define XML_CreateParserLevel(aaa) \
	aaa = MALLOC(xmlSAXHandler *, sizeof (xmlSAXHandler)); \
	bzero (aaa,sizeof(xmlSAXHandler));

/* elements */
#define XML_SetElementHandler(aaa,bbb,ccc) \
        aaa->startElement = bbb; \
        aaa->endElement = ccc; 

/* CDATA handling */
#define XML_SetDefaultHandler(aaa,bbb) /* this is CDATA related too */
#define XML_SetCdataSectionHandler(aaa,bbb,ccc) \
	aaa->cdataBlock = endCDATA;

//#define PROTOINSTANCE_MAX_LEVELS 10
//static XML_Parser colladaParser[PROTOINSTANCE_MAX_LEVELS];
//static XML_Parser currentColladaParser = NULL;
////static int parentIndex = 0;
//static int ColladaParserRecurseLevel = 0;
//static int inCDATA = FALSE;
//struct X3D_Node *colladaParentStack[PARENTSTACKSIZE];
//static int indentLevel = 0;

typedef struct pColladaParser{
	XML_Parser colladaParser[PROTOINSTANCE_MAX_LEVELS];
	XML_Parser currentColladaParser;// = NULL;
	//int parentIndex = 0;
	int ColladaParserRecurseLevel;// = 0;
	int inCDATA;// = FALSE;
	struct X3D_Node *colladaParentStack[PARENTSTACKSIZE];
	int indentLevel;// = 0;

}* ppColladaParser;
void *ColladaParser_constructor(){
	void *v = malloc(sizeof(struct pColladaParser));
	memset(v,0,sizeof(struct pColladaParser));
	return v;
}
void ColladaParser_init(struct tColladaParser *t){
	//public
	//private
	t->prv = ColladaParser_constructor();
	{
		ppColladaParser p = (ppColladaParser)t->prv;
		p->colladaParser[PROTOINSTANCE_MAX_LEVELS];
		p->currentColladaParser = NULL;
		//static int parentIndex = 0;
		p->ColladaParserRecurseLevel = 0;
		p->inCDATA = FALSE;
		//p->colladaParentStack[PARENTSTACKSIZE];
		p->indentLevel = 0;

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

        #ifdef COLLADAPARSERVERBOSE
        printf ("startCDATA -parentIndex %d parserMode %s\n",parentIndex,parserModeStrings[getParserMode()]);
        #endif
        ((ppColladaParser)(gglobal()->ColladaParser.prv))->inCDATA = TRUE;
}

static void XMLCALL endCDATA (void *userData, const xmlChar *value, int len) {
        #ifdef COLLADAPARSERVERBOSE
        printf ("endCDATA, cur index %d\n",CDATA_Text_curlen);
        printf ("endCDATA -parentIndex %d parserMode %s\n",parentIndex,parserModeStrings[getParserMode()]);
        #endif
        ((ppColladaParser)(gglobal()->ColladaParser.prv))->inCDATA = FALSE;

        /* x3d specific dumpCDATAtoProtoBody (CDATA_Text); */

        #ifdef COLLADAPARSERVERBOSE
        printf ("returning from EndCData\n");
        #endif


}

static void XMLCALL handleCDATA (void *userData, const char *string, int len) {
/*
        printf ("handleCDATA...(%d)...",len);
if (inCDATA) printf ("inCDATA..."); else printf ("not inCDATA...");
printf ("\n");
*/
}

static void XMLCALL ColladaStartElement(void *unused, const xmlChar *name, const xmlChar **atts) {

#ifdef COLLADAVERBOSE
{int i,j; for (j=0; j< indentLevel; j++) printf ("  ");
	printf ("startElement: %s : level %d\n",name,indentLevel);
       for (i = 0; atts[i]; i += 2) {
		for (j=0; j< indentLevel; j++) printf ("  ");
                printf("    field:%s=%s\n", atts[i], atts[i + 1]);
	}
}
#endif

	((ppColladaParser)(gglobal()->ColladaParser.prv))->indentLevel++;
}

static void XMLCALL ColladaEndElement(void *unused, const xmlChar *name) {
	((ppColladaParser)(gglobal()->ColladaParser.prv))->indentLevel--;

#ifdef COLLADAVERBOSE
{int i; for (i=0; i< indentLevel; i++) printf ("  ");
	printf ("endElement: %s : level %d\n",name,indentLevel);
}
#endif

}


static XML_Parser initializeColladaParser () {
	ppColladaParser p = (ppColladaParser)gglobal()->ColladaParser.prv;
	p->ColladaParserRecurseLevel++;

	if (p->ColladaParserRecurseLevel >= PROTOINSTANCE_MAX_LEVELS) {
		ConsoleMessage ("XML_PARSER init: XML file PROTO nested too deep\n");
		p->ColladaParserRecurseLevel--;
	} else {
		XML_CreateParserLevel(p->colladaParser[p->ColladaParserRecurseLevel]);
		XML_SetElementHandler(p->colladaParser[p->ColladaParserRecurseLevel], ColladaStartElement, ColladaEndElement);
		XML_SetCdataSectionHandler (p->colladaParser[p->ColladaParserRecurseLevel], startCDATA, endCDATA);
		XML_SetDefaultHandler (p->colladaParser[p->ColladaParserRecurseLevel],handleCDATA);
		XML_SetUserData(p->colladaParser[p->ColladaParserRecurseLevel], &parentIndex);
	}
	/* printf ("initializeColladaParser, level %d, parser %u\n",colladaParser[ColladaParserRecurseLevel]); */

	return p->colladaParser[p->ColladaParserRecurseLevel];
}

static void shutdownColladaParser () {
	ppColladaParser p = (ppColladaParser)gglobal()->ColladaParser.prv;
	/* printf ("shutdownColladaParser, recurseLevel %d\n",ColladaParserRecurseLevel); */
	XML_ParserFree(p->colladaParser[p->ColladaParserRecurseLevel]);
	p->ColladaParserRecurseLevel--;
	
	/* lets free up memory here for possible PROTO variables */
	if (p->ColladaParserRecurseLevel == INT_ID_UNDEFINED) {
		/* if we are at the bottom of the parser call nesting, lets reset parentIndex */
		parentIndex = 0;
		/* x3d specific freeProtoMemory (); */
	}

	if (p->ColladaParserRecurseLevel < INT_ID_UNDEFINED) {
		ConsoleMessage ("XML_PARSER close underflow");
		p->ColladaParserRecurseLevel = INT_ID_UNDEFINED;
	}

	/* CDATA text space, free it up */
        FREE_IF_NZ(CDATA_Text);
	if (p->ColladaParserRecurseLevel > INT_ID_UNDEFINED)
		p->currentColladaParser = p->colladaParser[p->ColladaParserRecurseLevel];

	/* printf ("shutdownColladaParser, current ColladaParser %u\n",currentColladaParser); */
}

int ColladaParse (struct X3D_Group* myParent, const char *inputstring) {
	ppColladaParser p = (ppColladaParser)gglobal()->ColladaParser.prv;
	p->currentColladaParser = initializeColladaParser();

	/* printf ("X3DParse, current ColladaParser is %u\n",currentColladaParser); */


	INCREMENT_PARENTINDEXC
	p->colladaParentStack[parentIndex] = X3D_NODE(myParent);

	if (XML_ParseFile(p->currentColladaParser, inputstring, (int) strlen(inputstring), TRUE) == XML_STATUS_ERROR) {
		fprintf(stderr,
			"%s at line %" XML_FMT_INT_MOD "u\n",
			XML_ErrorString(XML_GetErrorCode(currentColladaParser)),
			XML_GetCurrentLineNumber(currentColladaParser));
		shutdownColladaParser();
		return FALSE;
	}
	shutdownColladaParser();
	return TRUE;
}
