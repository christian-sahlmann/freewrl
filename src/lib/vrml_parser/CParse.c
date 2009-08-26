/*
=INSERT_TEMPLATE_HERE=

$Id: CParse.c,v 1.15 2009/08/26 13:57:16 crc_canada Exp $

???

*/

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
#include "../world_script/CScripts.h"
#include "CParseParser.h"
#include "CParseLexer.h"
#include "CProto.h"

 
/* Keep a pointer to the parser for the main URL */
struct VRMLParser* globalParser = NULL;
int inWhichParser = 0;

#undef TIMING

BOOL cParse(void* ptr, unsigned ofs, const char* data) {
	struct VRMLParser* parser;

	#ifdef TIMING
	double startt, endt;
	struct timeval mytime;
	struct timezone tz; /* unused see man gettimeofday */

	gettimeofday (&mytime,&tz);
	startt = (double) mytime.tv_sec + (double)mytime.tv_usec/1000000.0;
	#endif

 	if (!globalParser) {
		/* printf ("cParse, new parser\n"); */
		/* the FALSE in the newParser call signifies that we are using "VRML" formatted strings */
 		parser=newParser(ptr, ofs, FALSE);
		globalParser = parser;
 	} else {
		/* printf ("cParse, using old parser\n"); */
		parser=reuseParser(ptr,ofs);
 	}

	resetParseSuccessfullyFlag();

 	parser_fromString(parser, data);
 	ASSERT(parser->lexer);

 	if(!parser_vrmlScene(parser))
  		fprintf(stderr, "Parser found errors.\n");

 	/* printf ("after parsing in cParse, VRMLParser->DEFinedNodes %u\n",parser->DEFedNodes); */
 	/* deleteParser(parser); */

	/* force any remaining strings to be removed */
	lexer_forceStringCleanup(parser->lexer);

	#ifdef TIMING
	gettimeofday (&mytime,&tz);
	endt = (double) mytime.tv_sec + (double)mytime.tv_usec/1000000.0;
	printf ("time taken %lf\n",endt-startt);
	#endif

 	return parsedSuccessfully();
}

/* ************************************************************************** */
/* Accessor methods */

/* Return DEFed node from its name */
struct X3D_Node* parser_getNodeFromName(const char* name)
{
 indexT ind=lexer_nodeName2id(globalParser->lexer, name);
 if(ind==ID_UNDEFINED)
  return NULL;

 ASSERT(!stack_empty(globalParser->DEFedNodes));
 ASSERT(ind<vector_size(stack_top(struct Vector*, globalParser->DEFedNodes)));
 return vector_get(struct X3D_Node*,
  stack_top(struct Vector*, globalParser->DEFedNodes), ind);
}


/* Return PROTO name from its node, or NULL if not found */
char* parser_getPROTONameFromNode(struct X3D_Node *node)
{
	struct ProtoDefinition* cpd;

	cpd = getProtoDefinition(X3D_GROUP(node));
	if (cpd != NULL) return cpd->protoName;
	return NULL;
}

/* Return DEFed name from its node, or NULL if not found */
char* parser_getNameFromNode(struct X3D_Node *node)
{
	indexT ind;
	struct Vector *curNameStackTop = stack_top(struct Vector *, globalParser->lexer->userNodeNames);

	/* go through the DEFedNodes, looking for the X3D_Node pointer. If it is found, use that
	   index, and look in the userNodeNames list for it */

	/* for (ind=0; ind < vector_size(curNameStackTop); ind ++) {
		printf ("DEBUG: userNodeNames index %d is %s\n",ind, vector_get (const char*, curNameStackTop,ind));
	} */

	for (ind=0; ind < vector_size(stack_top(struct Vector*, globalParser->DEFedNodes)); ind++) {
		/* did we find this index? */
		if (vector_get(struct X3D_Node*, stack_top(struct Vector*, globalParser->DEFedNodes), ind) == node) {
			return vector_get (const char*, curNameStackTop, ind);
		}
	}
		
	return NULL;
}

/* this is a real assert; hopefully we will never get one of these, as it kills FreeWRL, which is a bad thing. */
void fw_assert (char *file, int line) {
	int looper;
	printf ("FreeWRL Internal Error at line %d in %s\n",line,file);
	ConsoleMessage ("FreeWRL Internal Error at line %d in %s",line,file);
	
	for (looper=1; looper<60; looper++) {
	    usleep(1000);
	    sched_yield();
	}
	printf ("FreeWRL exiting...\n");
	exit(1);
}

