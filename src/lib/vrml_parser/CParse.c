/*
=INSERT_TEMPLATE_HERE=

$Id: CParse.c,v 1.2 2008/11/27 00:27:18 couannette Exp $

???

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/CScripts.h"
#include "CParseParser.h"
#include "CParseLexer.h"

 
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

 	parser_fromString(parser, data);
 	ASSERT(parser->lexer);

 	if(!parser_vrmlScene(parser))
  		fprintf(stderr, "Parser failed!\n");

 	/* printf ("after parsing in cParse, VRMLParser->DEFinedNodes %u\n",parser->DEFedNodes); */
 	/* deleteParser(parser); */

  	/* this data is a copy, so we can delete it here */
  	FREE_IF_NZ (parser->lexer->startOfStringPtr);

	#ifdef TIMING
	gettimeofday (&mytime,&tz);
	endt = (double) mytime.tv_sec + (double)mytime.tv_usec/1000000.0;
	printf ("time taken %lf\n",endt-startt);
	#endif

 	return TRUE;
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


void fw_assert (char *file, int line) {
	int looper;
	printf ("FreeWRL Internal Error at line %d in %s\n",line,file);
	ConsoleMessage ("FreeWRL Internal Error at line %d in %s",line,file);
	
	for (looper=1; looper<60; looper++) {
		sleep(1);
		sched_yield();
	}
	printf ("FreeWRL exiting...\n");
	exit(1);
}

