/*
  $Id: main.c,v 1.42 2011/05/25 19:26:33 davejoubert Exp $

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



#include <config.h>
#include <system.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "main.h"
#include "options.h"
#include <display.h>

/*
#include <config.h>
#include <system.h>
#include <system_threads.h>
#include <internal.h>

#include <libFreeWRL.h>
#include <list.h>
#include <io_files.h>
#include <resources.h>
#include <threads.h>
#include "vrml_parser/Structs.h"
#include "main/ProdCon.h"
#include "input/InputFunctions.h"
*/

/**
 * FreeWRL parameters
 */
/* freewrl_params_t *fv_params = NULL; */
static freewrl_params_t *fv_params = NULL;
/*static freewrl_params_t *OSXparams = NULL; */

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
    char *start_url = NULL;	/* file/url to start FreeWRL with */
    const char *libver;
#ifndef AQUA
    const char  *progver;
#endif

#if defined(_ANDROID)
    int tempIsAndroid = 1 ;
#else
    int tempIsAndroid = 0 ;
#endif

    char consoleBuffer[200];
    fwl_ConsoleSetup(MC_DEF_AQUA , MC_TARGET_AQUA , MC_HAVE_MOTIF , MC_TARGET_MOTIF , MC_MSC_HAVE_VER , tempIsAndroid);

    /* first, get the FreeWRL shared lib, and verify the version. */
    libver = libFreeWRL_get_version();
#ifndef AQUA
    progver = freewrl_get_version();
    if (strcmp(progver, libver)) {
	sprintf(consoleBuffer ,"FreeWRL expected library version %s, got %s...\n",progver, libver);
	fwl_StringConsoleMessage(consoleBuffer);
    }
#endif
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
		strcat(fdir,"../../../../fonts");
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
    fv_params->collision = 1;

    fwl_init_StereoDefaults();

#if !defined(TARGET_AQUA) /* Aqua front ends do the parsing */
    /* parse command line arguments */
    if (fv_parseCommandLine(argc, argv)) {

	    start_url = argv[optind];
#ifdef _MSC_VER
	    start_url = strBackslash2fore(start_url);
#endif
#ifdef OLDCODE
#ifdef _MSC_VER
		//if( start_url )
		if(1)
		{
			int jj;
			//change to forward slashes
			for(jj=0;jj<strlen(start_url);jj++)
				if(start_url[jj] == '\\' ) start_url[jj] = '/';
			printf("working start_url=%s\n",start_url);

		}
		if(0) //
		{
			/* goal - split the url into path and file, then set 
				set current working directy = path
				and set starturl = file
			   motivation - doug can't comprehend resource.c path mangling otherwise 
			   and it seems to make relative image urls into absolute relative to the .exe rather than scene .wrl
			*/
			int jj;
			char *slash;
			char *path;
			for( jj=0;jj<strlen(start_url);jj++)
				if(start_url[jj] == '\\' ) start_url[jj] = '/';
			slash = strrchr(start_url, '/');
			path = NULL;
			if(slash)
			{
				path = start_url;
				slash[0] = '\0';
				start_url = &slash[1];
				if(_chdir(path)==-1)
				{
					/* either directory doesn't exist or its ftp/http */
					slash[0] = '/';
					start_url = path;
					path = NULL;
				}
			}
			printf("working dir=%s start_url=%s\n",path,start_url);
		}
#endif
#endif //OLDCODE
    }
#endif


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

#if !KEEP_FV_INLIB
void fv_setGeometry_from_cmdline(const char *gstring);
int fv_display_initialize(void);
void fv_setScreenDim(int wi, int he);

void fv_setGeometry_from_cmdline(const char *gstring) {}
int fv_display_initialize() {return 0;}
void fv_setScreenDim(int wi, int he) { fwl_setScreenDim(wi,he); }
#endif
