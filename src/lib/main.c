/*
  $Id: main.c,v 1.32 2010/06/29 16:59:44 crc_canada Exp $

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

void OSX_initializeParameters(const char* initialURL) {
    const char *libver, *progver;
    resource_item_t *res;

    /* have we been through once already (eg, plugin loading new file)? */
    if (OSXparams == NULL) {
    	/* first, get the FreeWRL shared lib, and verify the version. */
	/*
    	libver = libFreeWRL_get_version();
    	progver = freewrl_get_version();
    	if (strcmp(progver, libver)) {
		ConsoleMessage("FreeWRL expected library version %s, got %s...\n",progver, libver);
    	}
	*/

    	/* Before we parse the command line, setup the FreeWRL default parameters */
    	OSXparams = calloc(1, sizeof(freewrl_params_t));

    	/* Default values */
    	OSXparams->width = 600;
    	OSXparams->height = 400;
    	OSXparams->eai = FALSE;
    	OSXparams->fullscreen = FALSE;
    } 
	//printf("in OSX init\n");
    /* start threads, parse initial scene, etc */
   if (!initFreeWRL(OSXparams)) {
	ERROR_MSG("main: aborting during initialization.\n");
	exit(1);
   }
	//printf("after init osx params\n");

    fw_params.collision = 1;

    /* Give the main argument to the resource handler */
    res = resource_create_single(initialURL);
    send_resource_to_parser(res);

    while ((!res->complete) && (res->status != ress_failed) && (res->status != ress_not_loaded)) {
            usleep(50);
    }

    /* did this load correctly? */
    if (res->status == ress_not_loaded) {
	ConsoleMessage ("FreeWRL: Problem loading file \"%s\"", res->request);
    }
    if (res->status == ress_failed) {
	printf("load failed %s\n", initialURL);
	ConsoleMessage ("FreeWRL: unknown data on command line: \"%s\"", res->request);
    } else {
	/* Success! 
	printf("loaded %s\n", initialURL); */
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

bool initFreeWRL(freewrl_params_t *params)
{
	char *env_texture_size;

	TRACE_MSG("FreeWRL: initializing...\n");

	mainThread = pthread_self();

	/* Initialize console (log, error, ...) */
	setbuf(stdout,0);
        setbuf(stderr,0);
	
	/* Check environment */
	global_strictParsing = (getenv("FREEWRL_STRICT_PARSING") != NULL);
	if (global_strictParsing) {
		TRACE_MSG("Env: STRICT PARSING enabled.\n");
	}

	global_plugin_print = (getenv("FREEWRL_DO_PLUGIN_PRINT") != NULL);
	if (global_plugin_print) {
		TRACE_MSG("Env: PLUGIN PRINT enabled.\n");
	}

	global_occlusion_disable = (getenv("FREEWRL_NO_GL_ARB_OCCLUSION_QUERY") != NULL);
	if (global_occlusion_disable) {
		TRACE_MSG("Env: OCCLUSION QUERY disabled.\n");
	}

	env_texture_size = getenv("FREEWRL_TEXTURE_SIZE");
	if (env_texture_size) {
		sscanf(env_texture_size, "%u", &global_texture_size);
		TRACE_MSG("Env: TEXTURE SIZE %u.\n", global_texture_size);
	}

	global_print_opengl_errors = (getenv("FREEWRL_PRINT_OPENGL_ERRORS") != NULL);
	if (global_print_opengl_errors) {
		TRACE_MSG("Env: PRINT OPENGL ERRORS enabled.\n");
	}

	global_trace_threads = (getenv("FREEWRL_TRACE_THREADS") != NULL);
	if (global_trace_threads) {
		TRACE_MSG("Env: TRACE THREADS enabled.\n");
	}

	global_use_VBOs = (getenv("FREEWRL_USE_VBOS") != NULL);
	if (global_use_VBOs) {
		TRACE_MSG("Env: trying VBOs enabled.\n");
	}

#ifdef IPHONE
	global_use_shaders_when_possible = TRUE; /* OpenGL-ES 2.0 requires this */
#else
	global_use_shaders_when_possible = (getenv("FREEWRL_USE_SHADERS_WHEN_POSSIBLE") != NULL);
#endif
	if (global_use_shaders_when_possible) {
		TRACE_MSG("Env: USE_SHADERS_WHEN_POSSIBLE  enabled.\n");
	}

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
	
	/* shape compiler thread - if we can do this */
#ifdef DO_MULTI_OPENGL_THREADS
	initializeShapeCompileThread();
#endif
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
