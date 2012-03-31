/*
  $Id: main.c,v 1.52 2012/03/31 19:15:16 dug9 Exp $

  FreeWRL main program.

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


#if !defined(TARGET_AQUA)


#include <config.h>
#include <system.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "main.h"
#include "options.h"
#ifdef _MSC_VER
#include "getopt.h"
#endif

/**
 * FreeWRL parameters
 */

/* library parameters */
freewrl_params_t *fv_params = NULL;
/* file/url to start FreeWRL with */
char *start_url;

/**
 * Signal handlers 
 */
static int CaughtSEGV = FALSE;
void fv_catch_SIGQUIT();
void fv_catch_SIGSEGV();

#if !defined(CYGWIN)
void fv_catch_SIGALRM(int);
void fv_catch_SIGHUP();
#endif

#if defined(_MSC_VER)
#include <shlwapi.h>
char *get_current_dir();
char *strBackslash2fore(char *str);

#endif

/**
 * Main
 */
int main (int argc, char **argv)
{
    const char *libver;
    const char  *progver;

#if defined(_ANDROID)
    int tempIsAndroid = 1 ;
#else
    int tempIsAndroid = 0 ;
#endif

    char consoleBuffer[200];
	fwl_init_instance(); //before setting any structs we need a struct allocated
    fwl_ConsoleSetup(MC_DEF_AQUA , MC_TARGET_AQUA , MC_HAVE_MOTIF , MC_TARGET_MOTIF , MC_MSC_HAVE_VER , tempIsAndroid);

    /* first, get the FreeWRL shared lib, and verify the version. */
    libver = libFreeWRL_get_version();
    progver = freewrl_get_version();
    if (strcmp(progver, libver)) {
	sprintf(consoleBuffer ,"FreeWRL expected library version %s, got %s...\n",progver, libver);
	fwl_StringConsoleMessage(consoleBuffer);
    }

#ifdef _MSC_VER
	/*
	Set fonts directory
	ideally we would check if we are in a) projectfiles go ../../fonts b) else c:/windows/Fonts
	*/
	if(strstr(argv[0],"projectfiles"))
	{
		/* we are testing - use local fonts (may be obsolete someday) */
		static char *fdir;
		fdir = malloc(MAX_PATH); 
		strcpy(fdir,"FREEWRL_FONTS_DIR=");
		strcat(fdir,"C:/fonts");
		_putenv( fdir );
	}
	else
	{
		/* deployed system (with intalled fonts) - use system fonts  
		we plan to use a professional installer to install the fonts to %windir%\Fonts directory 
		where all the system fonts already are.
		Then in this program we will get the %windir%\Fonts directory, and set it as temporary
		environment variable for InputFunctions.C > makeFontsDirectory() to fetch.
		*/
		static char *fdir;
		char *syspath;
		syspath = getenv("windir");
		printf("windir path=[%s]\n",syspath);
		fdir = malloc(MAX_PATH); 
		strcpy(fdir,"FREEWRL_FONTS_DIR=");
		strcat(fdir,syspath);
		strcat(fdir,"/Fonts");
		_putenv( fdir );
	}
	get_current_dir();
	/* VBO preference - comment out for vbos (vertex buffer objects - a rendering optimization) */
	_putenv("FREEWRL_NO_VBOS=1"); 
	//_putenv("FREEWRL_USE_VBOS=1");


#endif

	/* install the signal handlers */

    signal(SIGTERM, (void(*)(int)) fv_catch_SIGQUIT);
    signal(SIGSEGV, (void(*)(int)) fv_catch_SIGSEGV);

#if !defined(CYGWIN)
    signal(SIGQUIT, (void(*)(int)) fv_catch_SIGQUIT);
    signal(SIGALRM, (void(*)(int)) fv_catch_SIGALRM);
    signal(SIGHUP,  (void(*)(int)) fv_catch_SIGHUP);
#endif

    /* Before we parse the command line, setup the FreeWRL default parameters */
    fv_params = calloc(1, sizeof(freewrl_params_t));

    /* Default values */
    fv_params->width = 600;
    fv_params->height = 400;
    fv_params->eai = FALSE;
    fv_params->fullscreen = FALSE;
    fv_params->winToEmbedInto = -1;
    fv_params->verbose = FALSE;
    fv_params->collision = 1; // if you set it, you need to update ui button with a call
	setMenuButton_collision(fv_params->collision);
	//fwl_init_StereoDefaults();

    /* parse command line arguments */
    if (fv_parseCommandLine(argc, argv)) {
		if(argc > 1){
			start_url = argv[optind];
#ifdef _MSC_VER
			start_url = strBackslash2fore(start_url);
#endif
		}else{
			start_url = NULL;
		}
    }


    /* doug- redirect stdout to a file - works, useful for sending bug reports */
    /*freopen("freopen.txt", "w", stdout ); */

    /* Put env parse here, because this gives systems that do not have env vars the chance to do it their own way. */
    fv_parseEnvVars();

    /* start threads, parse initial scene, etc */
    if ( 1 == 1) {
	/* give control to the library */
	if (!fwl_initFreeWRL(fv_params)) {
	    ERROR_MSG("main: aborting during initialization.\n");
	    exit(1);
	}
	fwl_startFreeWRL(start_url);
    } else {
	/* keep control
	if (!fv_initFreeWRL(fv_params)) {
	    ERROR_MSG("main: aborting during initialization.\n");
	    exit(1);
	}
	fv_startFreeWRL(start_url); */
    }
    return 0;
}
/* #include "src/bin/main-inc.c" */

/* SIGQUIT handler - plugin code sends a SIGQUIT... */
void fv_catch_SIGQUIT() 
{
    /* fwl_StringConsoleMessage("FreeWRL got a sigquit signal"); */
    /* shut up any SIGSEGVs we might get now. */
    CaughtSEGV = TRUE;
    fwl_doQuit();
}

void fv_catch_SIGSEGV()
{
    if (!CaughtSEGV) {

        fwl_StringConsoleMessage("FreeWRL got a SIGSEGV - can you please mail the file(s) to\n freewrl-09@rogers.com with a valid subject line. Thanks.\n");
	CaughtSEGV = TRUE;
    }
    exit(1);
}

#if !defined(CYGWIN)

void fv_catch_SIGHUP()
{
    /* fwl_StringConsoleMessage ("FreeWRL got a SIGHUP signal - reloading"); */
    /* MBFILES     Anchor_ReplaceWorld(BrowserFullPath); */
}

void fv_catch_SIGALRM(int sig)
{
    signal(SIGALRM, SIG_IGN);

    /* stuffs to do on alarm */
    /* fprintf(stderr,"An alarm signal just arrived ...IT WAS IGNORED!\n"); */
    /* end of alarm actions */

#if defined(_MSC_VER)
	printf("\a");
#else
	alarm(0);
#endif
    signal(SIGALRM, fv_catch_SIGALRM);
}

#endif

#endif // AQUA
