/*
  $Id: main.c,v 1.60 2011/06/10 15:22:58 crc_canada Exp $

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

char consoleBuffer[200];
freewrl_params_t fwl_params;

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
 
#if defined (_ANDROID)

// Android library initialization - similar to iPhone but we will poke files in later

int fwl_ANDROID_initialize(void)
{

	/* Initialize console (log, error, ...) */
	setbuf(stdout,0);
	setbuf(stderr,0);
	
	fwl_initParams(NULL);
	fwl_setp_width(480);
	fwl_setp_height(320);
	fwl_setp_eai(FALSE);
	fwl_setp_fullscreen(FALSE);
	fwl_setp_collision(1);
		
	if (!fwl_initFreeWRL(NULL)) {
		ERROR_MSG("main: aborting during initialization.\n");
		exit(1);
	}
	

	return TRUE;
}
#endif // _ANDROID


#if defined (TARGET_AQUA) || defined(_ANDROID)

/* put some config stuff here, as that way the Objective-C Standalone OSX front end does not
need to worry about specific structures and calls */

/* static freewrl_params_t *OSX_params = NULL; */
static bool qParamsInit = FALSE ;

void fwl_OSX_initializeParameters(const char* initialURL) {
    resource_item_t *res;

    /* have we been through once already (eg, plugin loading new file)? */
    if (!qParamsInit) {
	fwl_initParams(NULL);

    	/* Default values */
    	fwl_setp_width(600);
    	fwl_setp_height(400);
    	fwl_setp_eai(FALSE);
    	fwl_setp_fullscreen(FALSE);
    	fwl_setp_collision(1);
    } 

    /* start threads, parse initial scene, etc */

   if (!fwl_initFreeWRL(NULL)) {
	ERROR_MSG("main: aborting during initialization.\n");
	exit(1);
   }

    /* Give the main argument to the resource handler */
    res = resource_create_single(initialURL);
    send_resource_to_parser(res);

    while ((!res->complete) && (res->status != ress_failed) && (res->status != ress_not_loaded)) {
            usleep(500);
    }

    /* did this load correctly? */
    if (res->status == ress_not_loaded) {
	sprintf(consoleBuffer , "FreeWRL: Problem loading file \"%s\"", res->request);
	fwl_StringConsoleMessage(consoleBuffer);
    }

    if (res->status == ress_failed) {
	printf("load failed %s\n", initialURL);
	sprintf(consoleBuffer , "FreeWRL: unknown data on command line: \"%s\"", res->request);
	fwl_StringConsoleMessage(consoleBuffer);
    } else {

    	/* tell the new world which viewpoint to go to */
    	if (res->afterPoundCharacters != NULL) {
		fwl_gotoViewpoint(res->afterPoundCharacters);
		/* Success! 
		printf("loaded %s\n", initialURL); */
	}

    }
}

#endif // iPhone

/* OSX plugin is telling us the id to refer to */
void setInstance (uintptr_t instance) {
        /* printf ("setInstance, setting to %u\n",instance); */
        _fw_instance = instance;
}

/* osx Safari plugin is telling us where the initial file is */
void setFullPath(const char* file) 
{
/* turn collision on? 
    if (!fwl_getp_collision()) {
        char ks = 'c';
        do_keyPress(ks, KeyPress);
    }
*/

    /* remove a FILE:// or file:// off of the front */
    file = stripLocalFileName ((char *)file);
    FREE_IF_NZ (BrowserFullPath);
    BrowserFullPath = STRDUP((char *) file);
    /*
	sprintf(consoleBuffer , "setBrowserFullPath is %s (%d)",BrowserFullPath,strlen(BrowserFullPath));
	fwl_StringConsoleMessage(consoleBuffer);
    */
}
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

	fwl_StringConsoleMessage("before create resource\n");
	sprintf(consoleBuffer ,"frontend thread ID = %d\n",(int)pthread_self().p);
	fwl_StringConsoleMessage(consoleBuffer);

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
	sprintf(consoleBuffer ,"base=[%s] pr=[%s]\n",res->base,res->parsed_request);
	fwl_StringConsoleMessage(consoleBuffer);
	//removeFilenameFromPath(res->base);

	fwl_StringConsoleMessage("before send resource to parser\n");
	send_resource_to_parser_async(res);

	sprintf(consoleBuffer , "after send resource to parser res->status=%d\n",(int)res->status);
	fwl_StringConsoleMessage(consoleBuffer);
}
#endif /* _MSC_VER */

void fwl_initParams(freewrl_params_t *params)
{
	if (params) {
		DEBUG_MSG("copying application supplied params...\n");
		memcpy(&fwl_params, params, sizeof(freewrl_params_t));
	} else {
		memset(&fwl_params, 0, sizeof(freewrl_params_t));
	}
}
void fwl_setp_width		(int foo)	{ fwl_params.width = foo; }
void fwl_setp_height		(int foo)	{ fwl_params.height = foo; }
void fwl_setp_winToEmbedInto	(long int foo)	{ fwl_params.winToEmbedInto = foo; }
void fwl_setp_fullscreen	(bool foo)	{ fwl_params.fullscreen = foo; }
void fwl_setp_multithreading	(bool foo)	{ fwl_params.multithreading = foo; }
void fwl_setp_eai		(bool foo)	{ fwl_params.eai = foo; }
void fwl_setp_verbose		(bool foo)	{ fwl_params.verbose = foo; }
void fwl_setp_collision		(int foo)	{ fwl_params.collision = foo; }

int	fwl_getp_width		(void)	{ return fwl_params.width; }
int	fwl_getp_height		(void)	{ return fwl_params.height; }
long int fwl_getp_winToEmbedInto (void)	{ return fwl_params.winToEmbedInto; }
bool	fwl_getp_fullscreen	(void)	{ return fwl_params.fullscreen; }
bool	fwl_getp_multithreading	(void)	{ return fwl_params.multithreading; }
bool	fwl_getp_eai		(void)	{ return fwl_params.eai; }
bool	fwl_getp_verbose	(void)	{ return fwl_params.verbose; }
int	fwl_getp_collision	(void)	{ return fwl_params.collision; }

ttglobal fwl;
void* fwl_init_instance()
{
	//tglobal *fwl;
	fwl = iglobal_constructor();
	return (void *)fwl;
}
bool fwl_initFreeWRL(freewrl_params_t *params)
//bool fwl_initFreeWRL(freewrl_params_t *params)
{
	TRACE_MSG("FreeWRL: initializing...\n");

	gglobal()->threads.mainThread = pthread_self();

	set_thread2global((ttglobal)fwl, gglobal()->threads.mainThread ,"main thread");
	/* Initialize console (log, error, ...) */
	setbuf(stdout,0);
        setbuf(stderr,0);
	

	/* Check parameters */
	if (params) {
		DEBUG_MSG("copying application supplied params...\n");
		memcpy(&fwl_params, params, sizeof(freewrl_params_t));
	}

	/* do we require EAI? */
	if (fwl_getp_eai()) {
		fwl_create_EAI();
		//	set_thread2global(tglobal* fwl, pthread_t any );

	}


	/* Initialize parser */
	fwl_initialize_parser();

#ifndef FRONTEND_HANDLES_DISPLAY_THREAD
	/* OK the display is now initialized,
	   create the display thread and wait for it
	   to complete initialization */
	fwl_initializeDisplayThread();

	usleep(50);
	set_thread2global(fwl,gglobal()->threads.DispThrd "display thread");

#endif //FRONTEND_HANDLES_DISPLAY_THREAD

	fwl_initializeInputParseThread();
	set_thread2global(fwl, gglobal()->threads.PCthread ,"parse thread");

	while (!fwl_isInputThreadInitialized()) {
		usleep(50);
	}

	fwl_initializeTextureThread();
	set_thread2global(fwl, gglobal()->threads.loadThread ,"texture loading thread");
	while (!fwl_isTextureinitialized()) {
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
void fwl_startFreeWRL(const char *url)
{
	/* Give the main argument to the resource handler */
	fwl_resource_push_single_request(url);
	DEBUG_MSG("request sent to parser thread, main thread joining display thread...\n");

	/* now wait around until something kills this thread. */
	pthread_join(gglobal()->threads.DispThrd, NULL);
}

/**
 * Explicit exit routine
 */
void closeFreeWRL()
{
}
