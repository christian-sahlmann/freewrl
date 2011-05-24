/*
  $Id: main.c,v 1.51 2011/05/24 14:43:23 crc_canada Exp $

  FreeWRL support library.
  Resources handling: URL, files, ...

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
#include <io_files.h>
#include <resources.h>
#include <threads.h>
#include "vrml_parser/Structs.h"
#include "main/ProdCon.h"
#include "input/InputFunctions.h"
// JAS #include "x3d_parser/Bindable.h"


/**
 * library initialization
 */
#ifdef _MSC_VER
void libFreeWRL_init(void)
#else
void __attribute__ ((constructor)) libFreeWRL_init(void)
#endif
{
}

/**
 * library exit routine
 */
#ifdef _MSC_VER
void libFreeWRL_fini(void)
#else
void __attribute__ ((destructor)) libFreeWRL_fini(void)
#endif
{
}

/**
 * Explicit initialization
 */


#if defined (TARGET_AQUA)

/* put some config stuff here, as that way the Objective-C Standalone OSX front end does not
need to worry about specific structures and calls */

static freewrl_params_t *OSXparams = NULL;

void fwl_OSX_initializeParameters(const char* initialURL) {
    resource_item_t *res;

    /* have we been through once already (eg, plugin loading new file)? */
    if (OSXparams == NULL) {

    	/* Before we parse the command line, setup the FreeWRL default parameters */
    	OSXparams = calloc(1, sizeof(freewrl_params_t));

    	/* Default values */
    	OSXparams->width = 600;
    	OSXparams->height = 400;
    	OSXparams->eai = FALSE;
    	OSXparams->fullscreen = FALSE;
    } 

    /* start threads, parse initial scene, etc */

   if (!initFreeWRL(OSXparams)) {
	ERROR_MSG("main: aborting during initialization.\n");
	exit(1);
   }


    fw_params.collision = 1;

    /* Give the main argument to the resource handler */
    res = resource_create_single(initialURL);
    

    send_resource_to_parser(res);

    while ((!res->complete) && (res->status != ress_failed) && (res->status != ress_not_loaded)) {
            usleep(500);
    }

    /* did this load correctly? */
    if (res->status == ress_not_loaded) {
	ConsoleMessage ("FreeWRL: Problem loading file \"%s\"", res->request);
    }

    if (res->status == ress_failed) {
	printf("load failed %s\n", initialURL);
	ConsoleMessage ("FreeWRL: unknown data on command line: \"%s\"", res->request);
    } else {

    	/* tell the new world which viewpoint to go to */
    	if (res->afterPoundCharacters != NULL) {
		fwl_gotoViewpoint(res->afterPoundCharacters);
		/* Success! 
		printf("loaded %s\n", initialURL); */
	}

    }
}

/* OSX plugin is telling us the id to refer to */
void setInstance (uintptr_t instance) {
        /* printf ("setInstance, setting to %u\n",instance); */
        _fw_instance = instance;
}

/* osx Safari plugin is telling us where the initial file is */
void setFullPath(const char* file) 
{
/* turn collision on? 
    if (!fw_params.collision) {
        char ks = 'c';
        do_keyPress(ks, KeyPress);
    }
*/

    /* remove a FILE:// or file:// off of the front */
    file = stripLocalFileName ((char *)file);
    FREE_IF_NZ (BrowserFullPath);
    BrowserFullPath = STRDUP((char *) file);
    /* ConsoleMessage ("setBrowserFullPath is %s (%d)",BrowserFullPath,strlen(BrowserFullPath));  */
}
#endif
/**
 *   like resource_push_single_request except for win32/IE/ActiveX load main scene
 *   win32/InternetExplorer activex control is funny: it doesn't tell you
 *   it's page url anywhere. so you'll only get url=1.x3d when the page is
 *   at http://dug9.users.sourceforge.net/web3d/1.x3d  or 
 *   at c:/source2/tests/1.x3d. But what IE does do is provide a magic function
 *   URLDownloadToCacheFile(pointer_to_activex_control_instance,url,&localcachefilename,...)
 *   and it will give you back the localcachfilename which is either
 *   a) the file path if the url was for a local disk file ie c:/source2/tests/1.x3d OR
 *   b) the file path of the internet cache downloaded file, which you can open
 *      like a local file ie fopen()
 *   you don't have to know which it was -local or http-
 *   warning: don't delete the file in case it's local. If network, the network cache 
 *   should delete it automatically after some time.
 */
//#include <windows.h>
//#include "Shlwapi.h"
char *strForeslash2back(char *str)
{
#ifdef _MSC_VER
	int jj;
	for( jj=0;jj<strlen(str);jj++)
		if(str[jj] == '/' ) str[jj] = '\\';
#endif
	return str;
}
void send_resource_to_parser_async(resource_item_t *res);

#ifdef _MSC_VER
void fwl_resource_push_single_request_IE_main_scene(const char *request)
{
	resource_item_t *res;
	char *c;
	if (!request)
		return;
	ConsoleMessage("before create resource\n");
	ConsoleMessage("frontend thread ID = %d\n",(int)pthread_self().p);

	res = resource_create_single(request);
	/*
	2 possible spoof methods: 
	a) do resource_identify - need res->parent
		parent->network = true; (or _Bool true)
		parent->base = ""
	b)  skip resource_identify - set:
		res->type=rest_url 
		res->network = true 
		res->status = ress_starts_good
	*/
	//method b - skip resource identify
	res->network = true;
	res->type = rest_url;
	res->status = ress_starts_good;
	res->parsed_request = STRDUP(request);
	res->base = STRDUP(request);
	//c = strrchr(res->base,'/');
	//if(c) *c = '\0';
	//res->base = strForeslash2back(res->base);
	//PathRemoveFileSpec(res->base);
	//res->base = strBackslash2fore(res->base);
	res->base = NULL;
	ConsoleMessage("base=[%s] pr=[%s]\n",res->base,res->parsed_request);
	//removeFilenameFromPath(res->base);

	ConsoleMessage("before send resource to parser\n");
	send_resource_to_parser_async(res);
	ConsoleMessage("after send resource to parser res->status=%d\n",(int)res->status);
}
#endif /* _MSC_VER */


bool initFreeWRL(freewrl_params_t *params)
{

	TRACE_MSG("FreeWRL: initializing...\n");

	mainThread = pthread_self();

	/* Initialize console (log, error, ...) */
	setbuf(stdout,0);
        setbuf(stderr,0);
	
	/* Check environment */
#ifdef OLDCODE
OLDCODEDave Joubert added code to set these variables from the front end, not via the library,
OLDCODEto give more control.
OLDCODE
OLDCODE/* This should fall away, as env vars are now set from main */
OLDCODE/*
OLDCODE	See options.c fv_parseEnvVars
OLDCODE	char *env_texture_size;
OLDCODE
OLDCODE	global_strictParsing = (getenv("FREEWRL_STRICT_PARSING") != NULL);
OLDCODE	if (global_strictParsing) {
OLDCODE		TRACE_MSG("Env: STRICT PARSING enabled.\n");
OLDCODE	}
OLDCODE
OLDCODE	global_plugin_print = (getenv("FREEWRL_DO_PLUGIN_PRINT") != NULL);
OLDCODE	if (global_plugin_print) {
OLDCODE		TRACE_MSG("Env: PLUGIN PRINT enabled.\n");
OLDCODE	}
OLDCODE
OLDCODE	global_occlusion_disable = (getenv("FREEWRL_NO_GL_ARB_OCCLUSION_QUERY") != NULL);
OLDCODE	if (global_occlusion_disable) {
OLDCODE		TRACE_MSG("Env: OCCLUSION QUERY disabled.\n");
OLDCODE	}
OLDCODE
OLDCODE	env_texture_size = getenv("FREEWRL_TEXTURE_SIZE");
OLDCODE	if (env_texture_size) {
OLDCODE		sscanf(env_texture_size, "%u", &global_texture_size);
OLDCODE		TRACE_MSG("Env: TEXTURE SIZE %u.\n", global_texture_size);
OLDCODE	}
OLDCODE
OLDCODE	global_print_opengl_errors = (getenv("FREEWRL_PRINT_OPENGL_ERRORS") != NULL);
OLDCODE	if (global_print_opengl_errors) {
OLDCODE		TRACE_MSG("Env: PRINT OPENGL ERRORS enabled.\n");
OLDCODE	}
OLDCODE
OLDCODE	global_trace_threads = (getenv("FREEWRL_TRACE_THREADS") != NULL);
OLDCODE	if (global_trace_threads) {
OLDCODE		TRACE_MSG("Env: TRACE THREADS enabled.\n");
OLDCODE	}
OLDCODE
OLDCODE	if (getenv("FREEWRL_NO_VBOS") != NULL) global_use_VBOs = FALSE; 
OLDCODE	else if (getenv("FREEWRL_USE_VBOS") != NULL) global_use_VBOs = TRUE;
OLDCODE
OLDCODE	if (global_use_VBOs) {
OLDCODE		TRACE_MSG("Env: trying VBOs enabled.\n");
OLDCODE	}
OLDCODE*/
#endif /* OLDCODE */


	/* Check parameters */
	if (params) {
		DEBUG_MSG("copying application supplied params...\n");
		memcpy(&fw_params, params, sizeof(freewrl_params_t));
	}

	/* do we require EAI? */
	if (fw_params.eai) {
		create_EAI();
	}


	/* Initialize parser */
	initialize_parser();

	/* Multithreading ? */

	/* OK the display is now initialized,
	   create the display thread and wait for it
	   to complete initialization */
	initializeDisplayThread();
	

	initializeInputParseThread();
	while (!isInputThreadInitialized()) {
		usleep(50);
	}

	initializeTextureThread();
	while (!isTextureinitialized()) {
		usleep(50);
	}

	return TRUE;
}

/**
 *   startFreeWRL: we set up the main file / world
 *                 in the main() of the program, then
 *                 we call this routine after threads
 *                 initialization.
 */
void startFreeWRL(const char *url)
{
	/* Give the main argument to the resource handler */
	resource_push_single_request(url);
	DEBUG_MSG("request sent to parser thread, main thread joining display thread...\n");

	/* now wait around until something kills this thread. */
	pthread_join(DispThrd, NULL);
}

/**
 * Explicit exit routine
 */
void closeFreeWRL()
{
}
