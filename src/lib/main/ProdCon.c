/*
  $Id: ProdCon.c,v 1.90 2011/06/18 13:17:10 crc_canada Exp $

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
#include <list.h>
#include <resources.h>

#include <io_files.h>
#include <io_http.h>

#include <resources.h>

#include <threads.h>

#include "../vrml_parser/Structs.h"
#include "headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/JScript.h"
#include "../world_script/CScripts.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CParse.h"
#include "../world_script/jsUtils.h"
#include "Snapshot.h"
#include "../scenegraph/Collision.h"
#include "../non_web3d_formats/ColladaParser.h"
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

//true statics:
static char *EAI_Flag = "From the EAI bootcamp of life ";
char* PluginPath = "/private/tmp";
int PluginLength = 12;

//I think these are Aqua - I'll leave them static pending Aqua guru plugin review:
int _fw_browser_plugin = 0;
int _fw_pipe = 0;
uintptr_t _fw_instance = 0;

///* bind nodes in display loop, NOT in parsing threadthread */
//void *setViewpointBindInRender = NULL;
//void *setFogBindInRender = NULL;
//void *setBackgroundBindInRender = NULL;
//void *setNavigationBindInRender = NULL;

/* make up a new parser for parsing from createVrmlFromURL and createVrmlFromString */
//struct VRMLParser* savedParser;


/*
   ==============================================
   Explanations for this horrible modification :P
   ==============================================

   There is no reason to stop the main neither the display 
   while parser is parsing ;)... No reason I see with my little
   knowledge of the code...

   However, shared data access should be protected via mutex. I
   protect access to the download list (resource_list_to_parse).

   Root tree should also be protected when about to be modified.

*/

/*******************************/

///* thread synchronization issues */
//int _P_LOCK_VAR = 0;

#define SEND_TO_PARSER \
	if (p->_P_LOCK_VAR==0) { \
		p->_P_LOCK_VAR=1; \
	} \
	else printf ("SEND_TO_PARSER = flag wrong!\n");

#define PARSER_FINISHING \
	if (p->_P_LOCK_VAR==1) { \
		p->_P_LOCK_VAR=0; \
	} \
	else printf ("PARSER_FINISHING - flag wrong!\n");

#define UNLOCK \
	pthread_cond_signal(&gglobal()->threads.resource_list_condition); pthread_mutex_unlock(&gglobal()->threads.mutex_resource_list); 

#define WAIT_WHILE_PARSER_BUSY \
	pthread_mutex_lock(&gglobal()->threads.mutex_resource_list); \
     	while (p->_P_LOCK_VAR==1) { pthread_cond_wait(&gglobal()->threads.resource_list_condition, &gglobal()->threads.mutex_resource_list);} 


#define WAIT_WHILE_NO_DATA \
	pthread_mutex_lock(&gglobal()->threads.mutex_resource_list); \
     	while (p->_P_LOCK_VAR==0) { pthread_cond_wait(&gglobal()->threads.resource_list_condition, &gglobal()->threads.mutex_resource_list);} 








/*******************************/

//static s_list_t *resource_list_to_parse = NULL;

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
};

bool parser_do_parse_string(const char *input, struct X3D_Group *nRn);

/* Bindables */
//void* *fognodes = NULL;
//void* *backgroundnodes = NULL;
//void* *navnodes = NULL;
////void* *viewpointnodes = NULL;
//int totfognodes = 0;
//int totbacknodes = 0;
//int totnavnodes = 0;
//int totviewpointnodes = 0;
//int currboundvpno=0;
typedef struct pProdCon{
		void* *fognodes;// = NULL;
		void* *backgroundnodes;// = NULL;
		void* *navnodes;// = NULL;
		//void* *viewpointnodes = NULL;
		int totfognodes;// = 0;
		int totbacknodes;// = 0;
		int totnavnodes;// = 0;
		/* thread synchronization issues */
		int _P_LOCK_VAR;// = 0;
		s_list_t *resource_list_to_parse;// = NULL;
		/* psp is the data structure that holds parameters for the parsing thread */
		struct PSStruct psp;
		/* is the inputParse thread created? */
		int inputParseInitialized; //=FALSE;

		/* is the parsing thread active? this is read-only, used as a "flag" by other tasks */
		int inputThreadParsing; //=FALSE;
		int haveParsedCParsed;// = FALSE; 	/* used to tell when we need to call destroyCParserData  as destroyCParserData can segfault otherwise */

}* ppProdCon;
void *ProdCon_constructor(){
	void *v = malloc(sizeof(struct pProdCon));
	memset(v,0,sizeof(struct pProdCon));
	return v;
}
void ProdCon_init(struct tProdCon *t)
{
	//public
	t->viewpointnodes = NULL;
	t->totviewpointnodes = 0;
	t->currboundvpno=0;

	/* bind nodes in display loop, NOT in parsing threadthread */
	t->setViewpointBindInRender = NULL;
	t->setFogBindInRender = NULL;
	t->setBackgroundBindInRender = NULL;
	t->setNavigationBindInRender = NULL;
	t->savedParser = NULL; //struct VRMLParser* savedParser;

	//private
	t->prv = ProdCon_constructor();
	{
		ppProdCon p = (ppProdCon)t->prv;
		p->fognodes = NULL;
		p->backgroundnodes = NULL;
		p->navnodes = NULL;
		p->totfognodes = 0;
		p->totbacknodes = 0;
		p->totnavnodes = 0;

		/* thread synchronization issues */
		p->_P_LOCK_VAR = 0;
		p->resource_list_to_parse = NULL;
		/* psp is the data structure that holds parameters for the parsing thread */
		//p->psp;
		/* is the inputParse thread created? */
		p->inputParseInitialized=FALSE;
		/* is the parsing thread active? this is read-only, used as a "flag" by other tasks */
		p->inputThreadParsing=FALSE;
		p->haveParsedCParsed = FALSE; 	/* used to tell when we need to call destroyCParserData 
				   as destroyCParserData can segfault otherwise */

	}
}
///* is the inputParse thread created? */
//static int inputParseInitialized=FALSE;
//
///* is the parsing thread active? this is read-only, used as a "flag" by other tasks */
//int inputThreadParsing=FALSE;

/* /\* Is the initial URL loaded ? Robert Sim *\/ */
/* int URLLoaded = FALSE; */
/* int isURLLoaded() { return (URLLoaded && !inputThreadParsing); } */

///* psp is the data structure that holds parameters for the parsing thread */
//struct PSStruct psp;

//static int haveParsedCParsed = FALSE; 	/* used to tell when we need to call destroyCParserData 
//				   as destroyCParserData can segfault otherwise */

/* is a parser running? this is a function, because if we need to mutex lock, we
   can do all locking in this file */
int fwl_isInputThreadInitialized() {
	ppProdCon p = (ppProdCon)gglobal()->ProdCon.prv;
	return p->inputParseInitialized;
}

/* statusbar uses this to tell user that we are still loading */
int fwl_isinputThreadParsing() {
	ppProdCon p = (ppProdCon)gglobal()->ProdCon.prv;
	return(p->inputThreadParsing);
}

/**
 *   parser_do_parse_string: actually calls the parser.
 */
bool parser_do_parse_string(const char *input, struct X3D_Group *nRn)
{
	bool ret;
	ppProdCon p = (ppProdCon)gglobal()->ProdCon.prv;
	ret = FALSE;

	inputFileType = determineFileType(input);
	DEBUG_MSG("PARSE STRING, ft %d, fv %d.%d.%d\n",
		  inputFileType, inputFileVersion[0], inputFileVersion[1], inputFileVersion[2]);

	switch (inputFileType) {
	case IS_TYPE_XML_X3D:
		ret = X3DParse(nRn, input);
		break;
	case IS_TYPE_VRML:
		ret = cParse(nRn,(int) offsetof (struct X3D_Group, children), input);
		p->haveParsedCParsed = TRUE;
		break;
	case IS_TYPE_VRML1: {
		char *newData = convert1To2(input);
		ret = cParse (nRn,(int) offsetof (struct X3D_Group, children), newData);
		FREE_IF_NZ(newData);
	}
		break;
	case IS_TYPE_COLLADA:
		ConsoleMessage ("Collada not supported yet");
		ret = ColladaParse (nRn, input);
		break;
	case IS_TYPE_SKETCHUP:
		ConsoleMessage ("Google Sketchup format not supported yet");
		break;
	case IS_TYPE_KML:
		ConsoleMessage ("KML-KMZ  format not supported yet");
		break;
	default: {
		if (gglobal()->internalc.global_strictParsing) { ConsoleMessage ("unknown text as input"); } else {
			inputFileType = IS_TYPE_VRML;
			inputFileVersion[0] = 2; /* try VRML V2 */
			cParse (nRn,(int) offsetof (struct X3D_Group, children), input);
			p->haveParsedCParsed = TRUE; }
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
	ttglobal tg = gglobal();
	ppProdCon p = (ppProdCon)tg->ProdCon.prv;

	WAIT_WHILE_PARSER_BUSY;
	complete=0;
	p->psp.comp = &complete;
	p->psp.type = ZEROBINDABLES;
	p->psp.ofs = 0;
	p->psp.ptr = NULL;
	p->psp.path = NULL;
	p->psp.zeroBind = FALSE;
	p->psp.bind = FALSE; /* should we issue a set_bind? */
	p->psp.inp = NULL;
	p->psp.fieldname = NULL;

	/* send data to a parser */
	SEND_TO_PARSER;
	UNLOCK;

	/* wait for data */
	WAIT_WHILE_PARSER_BUSY;

	/* grab data */
	UNLOCK;

	/* and, reset our stack pointers */
	tg->Bindable.background_tos = INT_ID_UNDEFINED;
	tg->Bindable.fog_tos = INT_ID_UNDEFINED;
	tg->Bindable.navi_tos = INT_ID_UNDEFINED;
	tg->Bindable.viewpoint_tos = INT_ID_UNDEFINED;
}

/* interface for creating VRML for EAI */
int EAI_CreateVrml(const char *tp, const char *inputstring, struct X3D_Group *where)
{
	resource_item_t *res;
	char *newString;

	newString = NULL;

	if (strncmp(tp, "URL", 3) == 0) {

		res = resource_create_single(inputstring);
		res->where = where;
		res->offsetFromWhere = (int) offsetof (struct X3D_Group, children);
		/* printf ("EAI_CreateVrml, res->where is %u, root is %u parameter where %u\n",res->where, rootNode, where); */

	} else { // all other cases are inline code to parse... let the parser do the job ;P...

		const char *sendIn;

		if (strncmp(inputstring,"#VRML V2.0", 6) == 0) {
			sendIn = inputstring;
		} else {
			newString = MALLOC (char *, strlen(inputstring) + strlen ("#VRML V2.0 utf8\n") + 3);
			strcpy (newString,"#VRML V2.0 utf8\n");
			strcat (newString,inputstring);
			sendIn = newString;
			/* printf ("EAI_Create, had to append, now :%s:\n",newString); */
		}

		res = resource_create_from_string(sendIn);
		res->media_type=resm_vrml;
		res->parsed_request = EAI_Flag;
		res->where = where;
		res->offsetFromWhere = (int) offsetof (struct X3D_Group, children);
	}

	send_resource_to_parser(res);
	resource_wait(res);
	FREE_IF_NZ(newString);
	return (res->status == ress_parsed);
}

void send_resource_to_parser(resource_item_t *res)
{
	/* We are not in parser thread, most likely
	   in main or display thread, and we successfully
	   parsed a resource request.

	   We send it to parser.
	*/
	ppProdCon p = gglobal()->ProdCon.prv;

	/* Wait for display thread to be fully initialized */
	while (IS_DISPLAY_INITIALIZED == FALSE) {
		usleep(50);
	}

	/* wait for the parser thread to come up to speed */
	while (!p->inputParseInitialized) usleep(50);

	/* Lock access to the resource list */
	WAIT_WHILE_PARSER_BUSY;
 
	/* Add our resource item */
	p->resource_list_to_parse = ml_append(p->resource_list_to_parse, ml_new(res));

	/* signal that we have data on resource list */

	SEND_TO_PARSER;
	/* Unlock the resource list */
	UNLOCK;

	/* wait for the parser to finish */
	WAIT_WHILE_PARSER_BUSY;
	
	/* grab any data we want */
	UNLOCK;
}
void send_resource_to_parser_async(resource_item_t *res)
{
	/* We are not in parser thread, most likely
	   in main or display thread, and we successfully
	   parsed a resource request.

	   We send it to parser.
	*/
	ppProdCon p = (ppProdCon)gglobal()->ProdCon.prv;

	/* Wait for display thread to be fully initialized */
	while (IS_DISPLAY_INITIALIZED == FALSE) {
		usleep(50);
	}

	/* wait for the parser thread to come up to speed */
	while (!p->inputParseInitialized) usleep(50);

	/* Lock access to the resource list */
	WAIT_WHILE_PARSER_BUSY;
 
	/* Add our resource item */
	p->resource_list_to_parse = ml_append(p->resource_list_to_parse, ml_new(res));

	/* signal that we have data on resource list */

	SEND_TO_PARSER;
	/* Unlock the resource list */
	UNLOCK;

	/* wait for the parser to finish */
	//WAIT_WHILE_PARSER_BUSY;
	
	/* grab any data we want */
	//UNLOCK;
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
	ppProdCon p;
	struct tProdCon *t = &gglobal()->ProdCon;
	p = (ppProdCon)t->prv;
	printf("Parser wait queue:\n");
	ml_foreach(p->resource_list_to_parse, dump_resource_waiting((resource_item_t*)ml_elem(__l)));
	printf(".\n");
#endif
}

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
	int shouldBind;
	ppProdCon p;
	struct tProdCon *t;
	ttglobal tg = gglobal();
	t = &tg->ProdCon;
	p = (ppProdCon)t->prv;
    int parsedOk = FALSE; // results from parser

	/* printf("processing VRML/X3D resource: %s\n", res->request);  */
	shouldBind = FALSE;



	/* save the current URL so that any local-url gets are relative to this */
	pushInputResource(res);


	/* OK Boyz - here we go... if this if from the EAI, just parse it, as it will be a simple string */
	if (strcmp(res->parsed_request,EAI_Flag)==0) {

		/* EAI/SAI parsing */
		/* printf ("have the actual text here \n"); */
		/* create a container so that the parser has a place to put the nodes */
		nRn = (struct X3D_Group *) createNewX3DNode(NODE_Group);
	
		insert_node = X3D_GROUP(res->where); /* casting here for compiler */
		offsetInNode = res->offsetFromWhere;

		parsedOk = PARSE_STRING(res->request, nRn);
	} else {
		/* standard file parsing */
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

	
		if (!of->data) {
			/* error */
			return FALSE;
		}

		/* 
		printf ("res %p root_res %p\n",res,gglobal()->resources.root_res); 
		*/

		/* bind ONLY in main - do not bind for Inlines, etc */
		if (res == gglobal()->resources.root_res) {
			kill_bindables();
			shouldBind = TRUE;

		} else {
			if (!tg->resources.root_res->complete) {
				/* Push the parser state : re-entrance here */
				/* "save" the old classic parser state, so that names do not cross-pollute */
				t->savedParser = (void *)tg->CParse.globalParser;
				tg->CParse.globalParser = NULL;
			}
		}

		/* create a container so that the parser has a place to put the nodes */
		nRn = (struct X3D_Group *) createNewX3DNode(NODE_Group);
	
		/* ACTUALLY CALLS THE PARSER */
		parsedOk = PARSE_STRING(of->data, nRn);
	
		if ((res != tg->resources.root_res) && ((!tg->resources.root_res) ||(!tg->resources.root_res->complete))) {
			tg->CParse.globalParser = t->savedParser;
		}
	
	
		if (shouldBind) {
			if (p->totfognodes != 0) { 
				for (i=0; i < p->totfognodes; ++i) send_bind_to(X3D_NODE(p->fognodes[i]), 0); /* Initialize binding info */
				t->setFogBindInRender = p->fognodes[0];
			}
			if (p->totbacknodes != 0) {
				for (i=0; i < p->totbacknodes; ++i) send_bind_to(X3D_NODE(p->backgroundnodes[i]), 0);  /* Initialize binding info */
				t->setBackgroundBindInRender = p->backgroundnodes[0];
			}
			if (p->totnavnodes != 0) {
				for (i=0; i < p->totnavnodes; ++i) send_bind_to(X3D_NODE(p->navnodes[i]), 0);  /* Initialize binding info */
				t->setNavigationBindInRender = p->navnodes[0];
			}
			if (t->totviewpointnodes != 0) {
				for (i=0; i < t->totviewpointnodes; ++i) send_bind_to(X3D_NODE(t->viewpointnodes[i]), 0);  /* Initialize binding info */

				/* set the initial viewpoint for this file */
				t->setViewpointBindInRender = t->viewpointnodes[0];
			}
		}
	
		/* we either put things at the rootNode (ie, a new world) or we put them as a children to another node */
		if (res->where == NULL) {
			ASSERT(rootNode());
			insert_node = rootNode();
			offsetInNode = (int) offsetof(struct X3D_Group, children);
		} else {
			insert_node = X3D_GROUP(res->where); /* casting here for compiler */
			offsetInNode = res->offsetFromWhere;
		}
	}
	
	/* printf ("parser_process_res_VRML_X3D, res->where %u, insert_node %u, rootNode %u\n",res->where, insert_node, rootNode); */

	/* now that we have the VRML/X3D file, load it into the scene. */
	/* add the new nodes to wherever the caller wanted */

	/* take the nodes from the nRn node, and put them into the place where we have decided to put them */
	AddRemoveChildren(X3D_NODE(insert_node),
			  offsetPointer_deref(void*, insert_node, offsetInNode), 
			  (struct X3D_Node * *)nRn->children.p,
			  nRn->children.n, 1, __FILE__,__LINE__);
	
	/* and, remove them from this nRn node, so that they are not multi-parented */
	AddRemoveChildren(X3D_NODE(nRn),
			  (struct Multi_Node *)((char *)nRn + offsetof (struct X3D_Group, children)),
			  (struct X3D_Node* *)nRn->children.p,nRn->children.n,2,__FILE__,__LINE__);	

	res->complete = TRUE;
	

	/* remove this resource from the stack */
	popInputResource();

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

	buffer = NULL;

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
	ppProdCon p = (ppProdCon)gglobal()->ProdCon.prv;

	if (!item || !item->elem)
		return;

	res = ml_elem(item);

	/* printf("processing resource: %d, %s\n", res->type, resourceStatusToString(res->status)); */

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
		/* printf("processing resource, media_type %s\n",resourceMediaTypeToString(res->media_type)); */
		switch (res->media_type) {
		case resm_unknown:
			ConsoleMessage ("deciphering loaded file, unknown file type encountered.");
			remove_it = TRUE;
			res->complete=TRUE; /* not going to do anything else with this one */
			res->status = ress_not_loaded;
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
		/* Remove the parsed resource from the list */
		p->resource_list_to_parse = ml_delete_self(p->resource_list_to_parse, item);

		/* What next ? */
//		dump_parser_wait_queue();
	}

	dump_parser_wait_queue();
}

/**
 *   _inputParseThread: parser (loader) thread.
 */

void _inputParseThread(void)
{
	ENTER_THREAD("input parser");
	{
		ppProdCon p = (ppProdCon)gglobal()->ProdCon.prv;

		p->inputParseInitialized = TRUE;

		viewer_default();

		/* now, loop here forever, waiting for instructions and obeying them */

		for (;;) {
			WAIT_WHILE_NO_DATA;

			p->inputThreadParsing = TRUE;

			/* go through the resource list until it is empty */
			while (p->resource_list_to_parse != NULL) {
				ml_foreach(p->resource_list_to_parse, parser_process_res(__l));
			}
			p->inputThreadParsing = FALSE;

#if defined (IPHONE) || defined (_ANDROID)
            if (res->status == ress_parsed) setMenuStatus ("ok"); else setMenuStatus("parse failed");
#endif

			/* Unlock the resource list */
			PARSER_FINISHING;
			UNLOCK;
		}
	}
}

/* for ReplaceWorld (or, just, on start up) forget about previous bindables */

void kill_bindables (void) {
	ppProdCon p;
	struct tProdCon *t = &gglobal()->ProdCon;
	p = (ppProdCon)t->prv;

	p->totfognodes=0;
	p->totbacknodes=0;
	p->totnavnodes=0;
	t->totviewpointnodes=0;
	t->currboundvpno=0;
	FREE_IF_NZ(p->fognodes);
	FREE_IF_NZ(p->backgroundnodes);
	FREE_IF_NZ(p->navnodes);
	FREE_IF_NZ(t->viewpointnodes);
}


void registerBindable (struct X3D_Node *node) {
	ppProdCon p;
	struct tProdCon *t = &gglobal()->ProdCon;
	p = (ppProdCon)t->prv;

	/* printf ("registerBindable, on node %d %s\n",node,stringNodeType(node->_nodeType));  */
	switch (node->_nodeType) {
		case NODE_Viewpoint:
		case NODE_OrthoViewpoint:
		case NODE_GeoViewpoint:
			t->viewpointnodes = REALLOC (t->viewpointnodes, (sizeof(void *)*(t->totviewpointnodes+1)));
			t->viewpointnodes[t->totviewpointnodes] = node;
			t->totviewpointnodes ++;
			break;
		case NODE_Background:
		case NODE_TextureBackground:
			p->backgroundnodes = REALLOC (p->backgroundnodes, (sizeof(void *)*(p->totbacknodes+1)));
			p->backgroundnodes[p->totbacknodes] = node;
			p->totbacknodes ++;
			break;
		case NODE_NavigationInfo:
			p->navnodes = REALLOC (p->navnodes, (sizeof(void *)*(p->totnavnodes+1)));
			p->navnodes[p->totnavnodes] = node;
			p->totnavnodes ++;
			break;
		case NODE_Fog:
			p->fognodes = REALLOC (p->fognodes, (sizeof(void *)*(p->totfognodes+1)));
			p->fognodes[p->totfognodes] = node;
			p->totfognodes ++;
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

