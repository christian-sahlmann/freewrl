/*
  $Id: ProdCon.c,v 1.43 2009/12/01 21:34:51 crc_canada Exp $

  Main functions II (how to define the purpose of this file?).
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
#include <system_threads.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include <io_files.h>
#include <io_http.h>

#include <list.h>
#include <resources.h>

#include <threads.h>

#include "../vrml_parser/Structs.h"
#include "headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/CScripts.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CParse.h"
#include "../world_script/jsUtils.h"
#include "Snapshot.h"
#include "../scenegraph/Collision.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"
#include "../input/SensInterps.h"
#include "../x3d_parser/Bindable.h"
#include "../input/InputFunctions.h"

#include "../plugin/pluginUtils.h"
#include "../plugin/PluginSocket.h"

#include "../opengl/Textures.h"
#include "../opengl/LoadTextures.h"

#include "ProdCon.h"

/* used by the paser to call back the lexer for EXTERNPROTO */
void embedEXTERNPROTO(struct VRMLLexer *me, char *myName, char *buffer, char *pound);


#define VRML1ERRORMSG "FreeWRL does not parse VRML Version 1; please convert to VRML 2 or later"

char* PluginPath = "/private/tmp";
int PluginLength = 12;

int _fw_browser_plugin = 0;
int _fw_pipe = 0;
uintptr_t _fw_instance = 0;

/* bind nodes in display loop, NOT in parsing thread */
void *setViewpointBindInRender = NULL;
void *setFogBindInRender = NULL;
void *setBackgroundBindInRender = NULL;
void *setNavigationBindInRender = NULL;

/*
   ==============================================
   Explanations for this horrible modification :P
   ==============================================

   There is no reason to stop the main neither the display thread
   while parser is parsing ;)... No reason I see with my little
   knowledge of the code...

   However, shared data access should be protected via mutex. I
   protect access to the download list (resource_list_to_parse).

   Root tree should also be protected when about to be modified.

*/

#define PARSER_LOCKING_INIT 
#define SEND_TO_PARSER pthread_mutex_lock(&mutex);
#define PARSER_FINISHING 
#define UNLOCK pthread_mutex_unlock(&mutex)
#define WAIT_WHILE_PARSER_BUSY  
#define WAIT_WHILE_NO_DATA

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

static s_list_t *resource_list_to_parse = NULL;

#define PARSE_STRING(input,where) parser_do_parse_string(input,where)

struct PSStruct {
	unsigned type;		/* what is this task? 			*/
	char *inp;		/* data for task (eg, vrml text)	*/
	void *ptr;		/* address (node) to put data		*/
	unsigned ofs;		/* offset in node for data		*/
	int zeroBind;		/* should we dispose Bindables?	 	*/
	int bind;		/* should we issue a bind? 		*/
	char *path;		/* path of parent URL			*/
	int *comp;		/* pointer to complete flag		*/

	char *fieldname;	/* pointer to a static field name	*/
	int jparamcount;	/* number of parameters for this one	*/
	struct Uni_String *sv;			/* the SV for javascript		*/

	/* for EAI */
	uintptr_t *retarr;		/* the place to put nodes		*/
	int retarrsize;		/* size of array pointed to by retarr	*/
	unsigned Etype[10];	/* EAI return values			*/

	/* for class - return a string */
	char *retstr;
};

unsigned int _pt_CreateVrml (char *tp, char *inputstring, uintptr_t *retarr);
/* int inputParse(unsigned type, char *inp, int bind, int returnifbusy, */
/* 			void *ptr, unsigned ofs, int *complete, */
/* 			int zeroBind); */
void __pt_doInline(void);
void __pt_doStringUrl (void);
void EAI_readNewWorld(char *inputstring);
bool parser_do_parse_string(const char *input, struct X3D_Group *nRn);

/* Bindables */
void* *fognodes = NULL;
void* *backgroundnodes = NULL;
void* *navnodes = NULL;
void* *viewpointnodes = NULL;
int totfognodes = 0;
int totbacknodes = 0;
int totnavnodes = 0;
int totviewpointnodes = 0;
int currboundvpno=0;

/* is the inputParse thread created? */
static int inputParseInitialized=FALSE;

/* is the parsing thread active? this is read-only, used as a "flag" by other tasks */
int inputThreadParsing=FALSE;

/* Initial URL loaded -- MB res API */
#define IS_WORLD_LOADED ((root_res != NULL) && (root_res->status == ress_parsed))

/* /\* Is the initial URL loaded ? Robert Sim *\/ */
/* int URLLoaded = FALSE; */
/* int isURLLoaded() { return (URLLoaded && !inputThreadParsing); } */

/* psp is the data structure that holds parameters for the parsing thread */
struct PSStruct psp;

static int haveParsedCParsed = FALSE; 	/* used to tell when we need to call destroyCParserData 
				   as destroyCParserData can segfault otherwise */

/* is a parser running? this is a function, because if we need to mutex lock, we
   can do all locking in this file */
int isInputThreadInitialized() {return inputParseInitialized;}

/* statusbar uses this to tell user that we are still loading */
int isinputThreadParsing() {return(inputThreadParsing);}

#if 0 //MBFILES
/* given a URL, find the first valid file, and return it */
int getValidFileFromUrl (char *filename, char *path, struct Multi_String *inurl, char *firstBytes) {
	char *thisurl;
	int count;

	/* and strip off the file name from the current path, leaving any path */
	removeFilenameFromPath(path);

	/* printf ("getValidFileFromUrl, path now :%s:\n",path);
	printf ("and, inurl.n is %d\n",inurl->n); */

	/* try the first url, up to the last, until we find a valid one */
	count = 0;
	filename[0] = '\0'; /* terminate, in case the user did not specify ANY files; error messages will tell us this */

	while (count < inurl->n) {
		thisurl = inurl->p[count]->strptr;

		/* check to make sure we don't overflow */
		if ((strlen(thisurl)+strlen(path)) > 900) return FALSE;

		/* we work in absolute filenames... */
		makeAbsoluteFileName(filename,path,thisurl);
		/* printf ("getValidFile, thread %u filename %s\n",pthread_self(),filename); */

		if (fileExists(filename,firstBytes,TRUE)) {
			return TRUE;
		}
		count ++;
	}
	return FALSE;
}
#endif

/**
 *   parser_do_parse_string: actually calls the parser.
 */
bool parser_do_parse_string(const char *input, struct X3D_Group *nRn)
{
	bool ret;

	inputFileType = determineFileType(input);
	DEBUG_MSG("PARSE STRING, ft %d, fv %d.%d.%d\n",
		  inputFileType, inputFileVersion[0], inputFileVersion[1], inputFileVersion[2]);

	switch (inputFileType) {
	case IS_TYPE_XML_X3D:
		ret = X3DParse(nRn, input);
		break;
	case IS_TYPE_VRML:
		ret = cParse(nRn,offsetof (struct X3D_Group, children), input);
		haveParsedCParsed = TRUE;
		break;
	case IS_TYPE_VRML1: {
		char *newData = convert1To2(input);
		ret = cParse (nRn,offsetof (struct X3D_Group, children), newData);
		/* ConsoleMessage (VRML1ERRORMSG); */
	}
		break;
	case IS_TYPE_COLLADA:
		ConsoleMessage ("Collada not supported yet");
		break;
	case IS_TYPE_SKETCHUP:
		ConsoleMessage ("Google Sketchup format not supported yet");
		break;
	case IS_TYPE_KML:
		ConsoleMessage ("KML-KMZ  format not supported yet");
		break;
	default: {
		if (global_strictParsing) { ConsoleMessage ("unknown text as input"); } else {
			inputFileType = IS_TYPE_VRML;
			inputFileVersion[0] = 2; /* try VRML V2 */
			cParse (nRn,offsetof (struct X3D_Group, children), input);
			haveParsedCParsed = TRUE; }
	}
	}
	if (!ret) {
		ConsoleMessage ("Parser Unsuccessful");
	}
	return ret;
}

/* interface for telling the parser side to forget about everything...  */
void EAI_killBindables (void) {
	int complete;

	WAIT_WHILE_PARSER_BUSY;
	complete=0;
	psp.comp = &complete;
	psp.type = ZEROBINDABLES;
	psp.retarr = NULL;
	psp.ofs = 0;
	psp.ptr = NULL;
	psp.path = NULL;
	psp.zeroBind = FALSE;
	psp.bind = FALSE; /* should we issue a set_bind? */
	psp.inp = NULL;
	psp.fieldname = NULL;

	/* send data to a parser */
	SEND_TO_PARSER;
	UNLOCK;

	/* wait for data */
	WAIT_WHILE_PARSER_BUSY;

	/* grab data */
	UNLOCK;

	/* and, reset our stack pointers */
	background_tos = INT_ID_UNDEFINED;
	fog_tos = INT_ID_UNDEFINED;
	navi_tos = INT_ID_UNDEFINED;
	viewpoint_tos = INT_ID_UNDEFINED;
}

/* interface for creating VRML for EAI */
int EAI_CreateVrml(const char *tp, const char *inputstring, uintptr_t *retarr, int retarrsize)
{
	resource_item_t *res;

	/* FIXME: how to handle this ? PARSER REENTRANCE */
	psp.retarr = retarr;

	if (strncmp(tp, "URL", 3) == 0) {

		res = resource_create_single(inputstring);

	} else { // all other cases are inline code to parse... let the parser do the job ;P...

		res = resource_create_from_string(inputstring);
	}

	send_resource_to_parser(res);
	resource_wait(res);

	return (res->status == ress_parsed);

#if 0 //MBFILES

	int complete;
	int retval;
	UNUSED(tp);

	/* tell the SAI that this is a VRML file, in case it cares later on (check SAI spec) */
	currentFileVersion = 3;

	WAIT_WHILE_PARSER_BUSY;

	if (strncmp(tp,"URL",2) ==  0) {
			psp.type= FROMURL;
	} else if (strncmp(tp,"String",5) == 0) {
		psp.type = FROMSTRING;
	} else if (strncmp(tp,"CREATEPROTO",10) == 0) {
		psp.type = FROMCREATEPROTO;
	} else if (strncmp(tp,"CREATENODE",10) == 0) {
		psp.type = FROMCREATENODE;
	} else {
		printf ("EAI_CreateVrml - invalid input %s\n",tp);
		return 0;
	}
	 
	complete = 0; /* make sure we wait for completion */
	psp.comp = &complete;
	psp.ptr = (unsigned)NULL;
	psp.ofs = (unsigned)NULL;
	psp.path = NULL;
	psp.zeroBind = FALSE;
	psp.bind = FALSE; /* should we issue a set_bind? */
	psp.retarr = retarr;
	psp.retarrsize = retarrsize;
	/* copy over the command */
	psp.inp = (char *)MALLOC (strlen(inputstring)+2);
	memcpy (psp.inp,inputstring,strlen(inputstring)+1);

	/* send data to a parser */
	SEND_TO_PARSER;
	UNLOCK;

	/* wait for data */
	WAIT_WHILE_PARSER_BUSY;

	/* grab data */
	retval = psp.retarrsize;

	UNLOCK;
	return (retval);
#endif
}

/* interface for replacing worlds from EAI */
void EAI_readNewWorld(char *inputstring) {
    int complete;

	WAIT_WHILE_PARSER_BUSY;
    complete=0;
    psp.comp = &complete;
    psp.type = FROMURL;
	psp.retarr = NULL;
    psp.ptr  = rootNode;
    psp.ofs  = offsetof(struct X3D_Group, children);
    psp.path = NULL;
    psp.zeroBind = FALSE;
    psp.bind = TRUE; /* should we issue a set_bind? */
    /* copy over the command */
    psp.inp  = (char *)MALLOC (strlen(inputstring)+2);
    memcpy (psp.inp,inputstring,strlen(inputstring)+1);

	/* send data to a parser */
	SEND_TO_PARSER;
	UNLOCK;

	/* wait for data */
	WAIT_WHILE_PARSER_BUSY;

	UNLOCK;
}

void send_resource_to_parser(resource_item_t *res)
{
	/* We are not in parser thread, most likely
	   in main or display thread, and we successfully
	   parsed a resource request.

	   We send it to parser.
	*/

	/* Wait for display thread to be fully initialized */
	while (IS_DISPLAY_INITIALIZED == FALSE) {
		usleep(50);
	}

	/* wait for the parser thread to come up to speed */
	while (!inputParseInitialized) usleep(50);

	/* Lock access to the resource list */
	pthread_mutex_lock( &mutex_resource_list );

	/* Add our resource item */
	resource_list_to_parse = ml_append(resource_list_to_parse, ml_new(res));

	/* signal that we have data on resource list */
	pthread_cond_signal(&resource_list_condition);

	/* Unlock the resource list */
	pthread_mutex_unlock( &mutex_resource_list );
}

void dump_resource_waiting(resource_item_t* res)
{
#ifdef FW_DEBUG
	printf("%s\t%s\n",( res->complete ? "<finished>" : "<waiting>" ), res->request);
#endif
}



void dump_parser_wait_queue()
{
#ifdef FW_DEBUG
	printf("Parser wait queue:\n");
	ml_foreach(resource_list_to_parse, dump_resource_waiting((resource_item_t*)ml_elem(__l)));
	printf(".\n");
#endif
}

#if 0 //MBFILES

int inputParse(unsigned type, char *inp, int bind, int returnifbusy,
			void *ptr, unsigned ofs,int *complete,
			int zeroBind) {

	/* do we want to return if the parsing thread is busy, or do
	   we want to wait? */
	/* printf ("start of PerlParse, thread %d\n",pthread_self()); */
	if (returnifbusy) {
		/* printf ("inputParse, returnifbusy, inputThreadParsing %d\n",inputThreadParsing);*/
		if (inputThreadParsing) return (FALSE);
	}

	WAIT_WHILE_PARSER_BUSY;

	/* printf ("inputParse, past WAIT_WHILE_PARSER_BUSY in %d\n",pthread_self()); */

	/* copy the data over; MALLOC and copy input string */
	psp.comp = complete;
	psp.type = type;
	psp.retarr = NULL;
	psp.ptr = ptr;
	psp.ofs = ofs;
	psp.path = NULL;
	psp.bind = bind; /* should we issue a set_bind? */
	psp.zeroBind = zeroBind; /* should we zero bindables? */

	/* MBFILES: no need to copy input data ...*/

/* 	psp.inp = (char *)MALLOC (strlen(inp)+2); */
/* 	memcpy (psp.inp,inp,strlen(inp)+1); */

	psp.inp = inp;

	/* send data to a parser */
	SEND_TO_PARSER;
	UNLOCK;

	/* printf ("inputParse, waiting for data \n"); */

	/* wait for data */
	WAIT_WHILE_PARSER_BUSY;
	/* grab data */

	UNLOCK;
	return (TRUE);
}

#endif

/**
 *   parser_process_res_VRML_X3D: this is the final parser (loader) stage, then call the real parser.
 */
static bool parser_process_res_VRML_X3D(resource_item_t *res)
{
	s_list_t *l;
	openned_file_t *of;
	struct X3D_Group *nRn;
	struct X3D_Group *insert_node;
	int i;
	int offsetInNode;

	DEBUG_RES("processing VRML/X3D resource: %s\n", res->request);

	l = (s_list_t *) res->openned_files;
	if (!l) {
		/* error */
		return FALSE;
	}
	
	of = ml_elem(l);
	if (!of) {
		/* error */
		return FALSE;
	}
	
	if (!of->text) {
		/* error */
		return FALSE;
	}
	
	if (res == root_res) {
		/* FIXME: try to find a better way to handle this ;P ... */
		pushInputURL(res->parsed_request);
		
		/* FIXME: 
		   - parser initialization ...
		   - bindable : should we zero bindable ?
		*/
		
		/* Yes: à priori */
		kill_bindables();

#if 0 // FIXME: ???		
		/* for the VRML parser */
		if (haveParsedCParsed) {
			if (globalParser != NULL) {
				destroyCParserData(globalParser);
				globalParser = NULL;
			}
			haveParsedCParsed = FALSE;
		}
		
		/* for the X3D Parser */
		kill_X3DDefs();
#endif

		/* FIXME: hummmmm..... */
		
	} else {
		
		if (!root_res->complete) {
			/* Push the parser state : re-entrance here */
			/* "save" the old classic parser state, so that names do not cross-pollute */
			savedParser = globalParser;
			globalParser = NULL;
		}
	}
	
	/* create a container so that the parser has a place to put the nodes */
	nRn = (struct X3D_Group *) createNewX3DNode(NODE_Group);
	
	/* ACTUALLY CALLS THE PARSER */
	PARSE_STRING(of->text, nRn);
	
	if ((res != root_res) && (!root_res->complete)) {
		globalParser = savedParser;
	}
	
	
	if (totfognodes != 0) { 
		for (i=0; i < totfognodes; ++i) send_bind_to(X3D_NODE(fognodes[i]), 0); /* Initialize binding info */
		setFogBindInRender = fognodes[0];
	}
	if (totbacknodes != 0) {
		for (i=0; i < totbacknodes; ++i) send_bind_to(X3D_NODE(backgroundnodes[i]), 0);  /* Initialize binding info */
		setBackgroundBindInRender = backgroundnodes[0];
	}
	if (totnavnodes != 0) {
		for (i=0; i < totnavnodes; ++i) send_bind_to(X3D_NODE(navnodes[i]), 0);  /* Initialize binding info */
		setNavigationBindInRender = navnodes[0];
	}
	if (totviewpointnodes != 0) {
		for (i=0; i < totviewpointnodes; ++i) send_bind_to(X3D_NODE(viewpointnodes[i]), 0);  /* Initialize binding info */
		setViewpointBindInRender = viewpointnodes[0];
	}
	
	/* we either put things at the rootNode (ie, a new world) or we put them as a children to another node */
	if (res->where == NULL) {
		ASSERT(rootNode);
		insert_node = rootNode;
		offsetInNode = offsetof(struct X3D_Group, children);
	} else {
		insert_node = X3D_GROUP(res->where); /* casting here for compiler */
		offsetInNode = res->offsetFromWhere;
	}
	
	DEBUG_RES ("parser_process_res_VRML_X3D, res->where %u, insert_node %u, rootNode %u\n",res->where, insert_node, rootNode);

	/* now that we have the VRML/X3D file, load it into the scene. */
	/* add the new nodes to wherever the caller wanted */

#ifdef OLDCODE
	/* this has to be a GROUP node! */
	if (insert_node->_nodeType != NODE_Group) {
		ConsoleMessage ("we have to add to a Group node, but have something else here\n");
		return FALSE;
	}
#endif
	
	/* take the nodes from the nRn node, and put them into the place where we have decided to put them */
	AddRemoveChildren(X3D_NODE(insert_node),
			  offsetPointer_deref(void*, insert_node, offsetInNode), 
			  (uintptr_t*)nRn->children.p,
			  nRn->children.n, 1, __FILE__,__LINE__);
	
	/* and, remove them from this nRn node, so that they are not multi-parented */
	AddRemoveChildren(X3D_NODE(nRn),
			  (struct Multi_Node *)((char *)nRn + offsetof (struct X3D_Group, children)),
			  (uintptr_t *)nRn->children.p,nRn->children.n,2,__FILE__,__LINE__);	

	res->complete = TRUE;

	return TRUE;
}

/**
 *   parser_process_res_PROTO: this is the final parser (loader) stage, then call the real parser.
 */
static bool parser_process_res_PROTO(resource_item_t *res)
{
	s_list_t *l;
	openned_file_t *of;
	struct VRMLLexer *lexer;
	char *buffer;

	switch (res->type) {
	case rest_invalid:
		return FALSE;
		break;

	case rest_string:
		buffer = res->request;
		break;
	case rest_url:
	case rest_file:
	case rest_multi:
		l = (s_list_t *) res->openned_files;
		if (!l) {
			/* error */
			return FALSE;
		}
		
		of = ml_elem(l);
		if (!of) {
			/* error */
			return FALSE;
		}

		/* FIXME: finish this */
		break;
	}

	lexer = (struct VRMLLexer *) res->where;

	/* note: this may modify buffer */
	embedEXTERNPROTO(lexer, lexer->curID, buffer, NULL); //pound);
	/* FIXME: how to get a return value ? */
	return TRUE;
}

/**
 *   parser_process_res_SHADER: this is the final parser (loader) stage, then call the real parser.
 */
static bool parser_process_res_SHADER(resource_item_t *res)
{
	s_list_t *l;
	openned_file_t *of;
	struct Shader_Script* ss;
	const char *buffer;

	switch (res->type) {
	case rest_invalid:
		return FALSE;
		break;

	case rest_string:
		buffer = res->request;
		break;
	case rest_url:
	case rest_file:
	case rest_multi:
		l = (s_list_t *) res->openned_files;
		if (!l) {
			/* error */
			return FALSE;
		}
		
		of = ml_elem(l);
		if (!of) {
			/* error */
			return FALSE;
		}

		/* FIXME: finish this */
		break;
	}

	ss = (struct Shader_Script *) res->where;
	
	return script_initCode(ss, buffer);
}

/**
 *   parser_process_res: for each resource state, advance the process of loading.
 */
static void parser_process_res(s_list_t *item)
{
	bool remove_it = FALSE;
	resource_item_t *res;

	if (!item || !item->elem)
		return;

	res = ml_elem(item);

	DEBUG_RES("processing resource: %d, %d\n", res->type, res->status);

	switch (res->status) {

	case ress_invalid:
	case ress_none:
		resource_identify(res->parent, res);
		if (res->type == rest_invalid) {
			remove_it = TRUE;
		}
		break;

	case ress_starts_good:
		resource_fetch(res);
		break;

	case ress_downloaded:
		/* Here we may want to delegate loading into another thread ... */
		if (!resource_load(res)) {
			ERROR_MSG("failure when trying to load resource: %s\n", res->request);
			remove_it = TRUE;
		}
		break;

	case ress_failed:
		remove_it = TRUE;
		break;

	case ress_loaded:
		switch (res->media_type) {
		case resm_unknown:
			break;
		case resm_vrml:
		case resm_x3d:
			if (parser_process_res_VRML_X3D(res)) {
				DEBUG_MSG("parser successfull: %s\n", res->request);
				res->status = ress_parsed;
			} else {
				ERROR_MSG("parser failed for resource: %s\n", res->request);
			}
			break;
		case resm_pshader:
		case resm_fshader:
			if (parser_process_res_SHADER(res)) {
				DEBUG_MSG("parser successfull: %s\n", res->request);
				res->status = ress_parsed;
			} else {
				ERROR_MSG("parser failed for resource: %s\n", res->request);
			}
			break;
		case resm_image:
		case resm_movie:
			/* Texture file has been loaded into memory
			   the node could be updated ... i.e. texture created */
			res->complete = TRUE; /* small hack */
			break;
		}
		/* Parse only once ! */
		remove_it = TRUE;
		break;

	case ress_not_loaded:
		remove_it = TRUE;
		break;

	case ress_parsed:
		remove_it = TRUE;
		break;

	case ress_not_parsed:
		remove_it = TRUE;
		break;		
	}

	if (remove_it) {
		dump_parser_wait_queue();

#ifdef OLDCODE // already locked in inputParseThread, so this will deadlock
		/* Lock access to the resource list */
		pthread_mutex_lock( &mutex_resource_list );
#endif
		
		/* Remove the parsed resource from the list */
		resource_list_to_parse = ml_delete_self(resource_list_to_parse, item);

#ifdef OLDCODE // already locked in inputParseThread, so this will deadlock
		/* Unlock the resource list */
		pthread_mutex_unlock( &mutex_resource_list );
#endif
	}
}

/**
 *   _inputParseThread: parser (loader) thread.
 */

void _inputParseThread(void)
{
	ENTER_THREAD("input parser");

	inputParseInitialized = TRUE;

	/* now, loop here forever, waiting for instructions and obeying them */
	for (;;) {
		
		/* Process all resource list items, whatever status they may have */

		/* Lock access to the resource list */
		pthread_mutex_lock( &mutex_resource_list );

		/* wait around until we have been signalled */
		pthread_cond_wait (&resource_list_condition, &mutex_resource_list);

		inputThreadParsing = TRUE;

		/* go through the resource list until it is empty */
		while (resource_list_to_parse != NULL) {
			ml_foreach(resource_list_to_parse, parser_process_res(__l));
#ifdef OLDCODE
Doug Sanden had problems with the original ml_foreach macro; macro changed, to reflect problem. Doug's fixed
code is shown here for reference:
					s_list_t *__l;
					s_list_t *next;
					s_list_t *_list = resource_list_to_parse;
					for(__l=_list;__l!=NULL;)  
					{
						next = ml_next(__l); /* get next from __l before action parser_process_res deletes __l */
						parser_process_res(__l);
						__l = next;
					}
#endif
		}

		inputThreadParsing = FALSE;

		/* Unlock the resource list */
		pthread_mutex_unlock( &mutex_resource_list );
	}

// MBFILES : old code below
#if 0

		/* printf ("thread %u waiting for data\n",pthread_self()); */
		WAIT_WHILE_NO_DATA;

		inputThreadParsing=TRUE;

		/* have to handle these types of commands:
			FROMSTRING 	create vrml from string
			FROMURL		create vrml from url
			INLINE		convert an inline into code, and load it.
			CALLMETHOD	Javascript...
			EAIGETNODE      EAI getNode
			EAIGETVIEWPOINT get a Viewpoint CNode
			EAIGETTYPE	EAI getType
			EAIGETVALUE	EAI getValue - in a string.
			EAIROUTE	EAI add/delete route
			ZEROBINDABLES	tell the front end to just forget about DEFS, etc 
			SAICOMMAND	general command, with string param to parser returns an int */

		if (psp.type == INLINE) {
		/* is this a INLINE? If it is, try to load one of the URLs. */
			__pt_doInline();
		}

		switch (psp.type) {

		case FROMSTRING:
		case FROMCREATENODE:
		case FROMCREATEPROTO:
		case FROMURL:	{
			/* is this a Create from URL or string, or a successful INLINE? */
			__pt_doStringUrl();
			break;
			}

		case INLINE: {
			/* this should be changed to a FROMURL before here  - check */
			printf ("Inline unsuccessful\n");
			break;
			}

		case ZEROBINDABLES: 
			/* for the VRML parser */
			if (haveParsedCParsed) {
				if (globalParser != NULL) {
					destroyCParserData(globalParser);
					globalParser = NULL;
				}
				haveParsedCParsed = FALSE;
			}

			/* for the X3D Parser */
			kill_X3DDefs();
			break;

		default: {
			printf ("produceTask - invalid type!\n");
			}
		}

		/* finished this loop, free data */
		FREE_IF_NZ (psp.inp);
		FREE_IF_NZ (psp.path);

		if (psp.comp) {
			*psp.comp = TRUE;
		}

		URLLoaded=TRUE;
		inputThreadParsing=FALSE;
		PARSER_FINISHING;
		UNLOCK;
	}
#endif

}

/* for ReplaceWorld (or, just, on start up) forget about previous bindables */

void kill_bindables (void) {
	totfognodes=0;
	totbacknodes=0;
	totnavnodes=0;
	totviewpointnodes=0;
	currboundvpno=0;
	FREE_IF_NZ(fognodes);
	FREE_IF_NZ(backgroundnodes);
	FREE_IF_NZ(navnodes);
	FREE_IF_NZ(viewpointnodes);
}


void registerBindable (struct X3D_Node *node) {

	/* printf ("registerBindable, on node %d %s\n",node,stringNodeType(node->_nodeType));  */
	switch (node->_nodeType) {
		case NODE_Viewpoint:
		case NODE_GeoViewpoint:
			viewpointnodes = REALLOC (viewpointnodes, (sizeof(void *)*(totviewpointnodes+1)));
			viewpointnodes[totviewpointnodes] = node;
			totviewpointnodes ++;
			break;
		case NODE_Background:
		case NODE_TextureBackground:
			backgroundnodes = REALLOC (backgroundnodes, (sizeof(void *)*(totbacknodes+1)));
			backgroundnodes[totbacknodes] = node;
			totbacknodes ++;
			break;
		case NODE_NavigationInfo:
			navnodes = REALLOC (navnodes, (sizeof(void *)*(totnavnodes+1)));
			navnodes[totnavnodes] = node;
			totnavnodes ++;
			break;
		case NODE_Fog:
			fognodes = REALLOC (fognodes, (sizeof(void *)*(totfognodes+1)));
			fognodes[totfognodes] = node;
			totfognodes ++;
			break;
		default: {
			/* do nothing with this node */
			/* printf ("got a registerBind on a node of type %s - ignoring\n",
					stringNodeType(node->_nodeType));
			*/
			return;
		}                                                

	}
}

#if 0 //MBFILES

/* handle an INLINE - should make it into a CreateVRMLfromURL type command */
void __pt_doInline() {
	char *filename;
	struct Multi_String *inurl;
	struct X3D_Inline *inl;
	int removeIt = FALSE;
	inl = (struct X3D_Inline *)psp.ptr;
	inurl = &(inl->url);
	filename = (char *)MALLOC(1000);
	filename[0] = '\0';

	/* lets make up the path and save it, and make it the global path */
	psp.path = STRDUP(inl->__parenturl->strptr);

	/* printf ("doInline, checking for file from path %s\n",psp.path); */

	if (getValidFileFromUrl (filename, psp.path, inurl,NULL)) {
		/* were we successful at locating one of these? if so, make it into a FROMURL */
		/* printf ("doInline, we were successful at locating %s\n",filename);  */
		psp.type=FROMURL;
	} else {
		/* printf ("doInline, NOT successful at locating %s\n",filename);  */
		ConsoleMessage ("Could Not Locate Inline URL %s\n",filename);
	}
	psp.inp = filename; /* will be freed later */

	/* printf ("doinline, psp.inp = %s\n",psp.inp);
	printf ("inlining %s\n",filename);  */
}

/* this is a CreateVrmlFrom URL or STRING command */
void __pt_doStringUrl () {
	int count;
	int retval;
        int i;

	/* for cParser */
        char *buffer = NULL;
	char *ctmp = NULL;
	struct X3D_Group *nRn;

	if (psp.zeroBind) {
		if (haveParsedCParsed) {
			if (globalParser != NULL) {
				destroyCParserData(globalParser);
				globalParser = NULL;
			}
			kill_bindables();
		}
		psp.zeroBind = FALSE;
	}

	if (psp.type==FROMSTRING) {

		/* check and convert to VRML... */
		nRn = (struct X3D_Group *) createNewX3DNode(NODE_Group);
		PARSE_STRING(psp.inp, nRn);
		
	} else if (psp.type==FROMURL) {

		/* MBFILES FIXME: this is here !!! */

		/* get the input */
		pushInputURL(psp.inp);
		buffer = readInputString(psp.inp);

		/* printf ("data is %s\n",buffer); */

		/* get the data from wherever we were originally told to find it */
		nRn = (struct X3D_Group *) createNewX3DNode(NODE_Group);
		PARSE_STRING(buffer, nRn);
		FREE_IF_NZ(buffer); 
		FREE_IF_NZ(ctmp);
	} else if (psp.type==FROMCREATENODE) {
		nRn = (struct X3D_Group *) createNewX3DNode(NODE_Group);
		PARSE_STRING(psp.inp, nRn);
	} else {
		nRn = (struct X3D_Group *) createNewX3DNode(NODE_Group);
		/* this will be a proto expansion, because otherwise the EAI code
		   would have gotten this before here */
		/* lets try this - same as FROMSTRING above... */
		/* look to see if this is X3D */
		PARSE_STRING(psp.inp, nRn);
	}
	

	/* set bindables, if required */
	if (psp.bind) {
	        if (totfognodes != 0) { 
		   for (i=0; i < totfognodes; ++i) send_bind_to(X3D_NODE(fognodes[i]), 0); /* Initialize binding info */
		   setFogBindInRender = fognodes[0];
		}
		if (totbacknodes != 0) {
                   for (i=0; i < totbacknodes; ++i) send_bind_to(X3D_NODE(backgroundnodes[i]), 0);  /* Initialize binding info */
		   setBackgroundBindInRender = backgroundnodes[0];
		}
		if (totnavnodes != 0) {
                   for (i=0; i < totnavnodes; ++i) send_bind_to(X3D_NODE(navnodes[i]), 0);  /* Initialize binding info */
		   setNavigationBindInRender = navnodes[0];
		}
		if (totviewpointnodes != 0) {
                   for (i=0; i < totviewpointnodes; ++i) send_bind_to(X3D_NODE(viewpointnodes[i]), 0);  /* Initialize binding info */
		   setViewpointBindInRender = viewpointnodes[0];
		}
	}

	/* did the caller want these values returned? */
	if (psp.retarr != NULL) {
		int totret = 0;

		for (count=0; count < nRn->children.n; count++) {
			/* only return the non-null children */
			if (nRn->children.p[count] != NULL) {
				psp.retarr[totret] = 0; /* the "perl" node number */
				totret++;
				psp.retarr[totret] = ((uintptr_t) 
					nRn->children.p[count]); /* the Node Pointer */
				totret++;
			}
		}
		psp.retarrsize = totret; /* remember, the old "perl node number" */
	}

      	/* now that we have the VRML/X3D file, load it into the scene. */
	if (psp.ptr != NULL) {
		/* add the new nodes to wherever the caller wanted */
		AddRemoveChildren(psp.ptr,offsetPointer_deref(void*,psp.ptr,psp.ofs), (uintptr_t*)nRn->children.p,nRn->children.n,1,__FILE__,__LINE__);

		/* and, remove them from this nRn node, so that they are not multi-parented */
		AddRemoveChildren(X3D_NODE(nRn), (struct Multi_Node *)((char *)nRn + offsetof (struct X3D_Group, children)), (uintptr_t *)nRn->children.p,nRn->children.n,2,__FILE__,__LINE__);
	}


	retval = 0;
	count = 0;
}
#endif //MBFILES
