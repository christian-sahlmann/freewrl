/*
  $Id: main.c,v 1.13 2009/10/26 17:48:43 couannette Exp $

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


freewrl_params_t fw_params = {
	/* width */          800,
	/* height */         600,
	/* fullscreen */     FALSE,
	/* multithreading */ TRUE,
	/* eai */            TRUE,
	/* verbose */        FALSE,
};

/* Global FreeWRL options (will become profiles ?) */

bool global_strictParsing = FALSE;
bool global_plugin_print = FALSE;
bool global_occlusion_disable = FALSE;
unsigned global_texture_size = 0;
bool global_print_opengl_errors = FALSE;
bool global_trace_threads = FALSE;


/**
 * library initialization
 */
void __attribute__ ((constructor)) libFreeWRL_init(void)
{
}

/**
 * library exit routine
 */
void __attribute__ ((destructor)) libFreeWRL_fini(void)
{
}

/**
 * Explicit initialization
 */
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

	/* now wait around until something kills this thread. */
	pthread_join(DispThrd, NULL);
}

/**
 * Explicit exit routine
 */
void closeFreeWRL()
{
}
