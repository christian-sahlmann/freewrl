/*
  $Id: main.c,v 1.39 2011/04/09 00:33:19 davejoubert Exp $

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

/**
 * FreeWRL parameters
 */
freewrl_params_t *params = NULL;

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

    /* first, get the FreeWRL shared lib, and verify the version. */
    libver = libFreeWRL_get_version();
#ifndef AQUA
    progver = freewrl_get_version();
    if (strcmp(progver, libver)) {
	ConsoleMessage("FreeWRL expected library version %s, got %s...\n",progver, libver);
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
		strcat(fdir,"..\\..\\..\\..\\fonts");
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
		strcat(fdir,"\\Fonts");
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
    params = calloc(1, sizeof(freewrl_params_t));

    /* Default values */
    params->width = 600;
    params->height = 400;
    params->eai = FALSE;
    params->fullscreen = FALSE;
    params->winToEmbedInto = -1;
	fwl_init_StereoDefaults();

#if !defined(TARGET_AQUA) /* Aqua front ends do the parsing */
    /* parse command line arguments */
    if (fv_parseCommandLine(argc, argv)) {

	    start_url = argv[optind];
#ifdef _MSC_VER
		//if( start_url )
		if(1) //
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
    }
#endif


    /* doug- redirect stdout to a file - works, useful for sending bug reports */
    /*freopen("freopen.txt", "w", stdout ); */

    /* Put env parse here, because this gives systems that do not have env vars the chance to do it their own way. */
    fv_parseEnvVars();

    /* start threads, parse initial scene, etc */
    if (!initFreeWRL(params)) {
	    ERROR_MSG("main: aborting during initialization.\n");
	    exit(1);
    }

    /* give control to the library */
    startFreeWRL(start_url);
    return 0;
}

/* SIGQUIT handler - plugin code sends a SIGQUIT... */
void fv_catch_SIGQUIT() 
{
    /* ConsoleMessage ("FreeWRL got a sigquit signal"); */
    /* shut up any SIGSEGVs we might get now. */
    CaughtSEGV = TRUE;
    fwl_doQuit();
}

void fv_catch_SIGSEGV()
{
    if (!CaughtSEGV) {
	ConsoleMessage ("FreeWRL got a SIGSEGV - can you please mail the file(s) to\n freewrl-09@rogers.com with a valid subject line. Thanks.\n");
	CaughtSEGV = TRUE;
    }
    exit(1);
}

#if !defined(CYGWIN)

void fv_catch_SIGHUP()
{
    /* ConsoleMessage ("FreeWRL got a SIGHUP signal - reloading"); */
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

