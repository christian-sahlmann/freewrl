/*
  $Id: main.c,v 1.27 2009/11/23 01:44:09 dug9 Exp $

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
void catch_SIGQUIT();
void catch_SIGSEGV();

#if !defined(CYGWIN)
void catch_SIGALRM(int);
void catch_SIGHUP();
#endif

#if defined(_MSC_VER)
#include <shlwapi.h>
#endif

/**
 * Main
 */
int main (int argc, char **argv)
{
    char *start_url = NULL;	/* file/url to start FreeWRL with */
    const char *libver, *progver;

    /* first, get the FreeWRL shared lib, and verify the version. */
    libver = libFreeWRL_get_version();
    progver = freewrl_get_version();
    if (strcmp(progver, libver)) {
	ConsoleMessage("FreeWRL expected library version %s, got %s...\n",progver, libver);
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
		char *pp = strcpy(malloc(MAX_PATH),argv[0]);
		PathRemoveFileSpec(pp);
		PathAppend(pp,"..\\..\\fonts"); 
		fdir = malloc(MAX_PATH); 
		strcpy(fdir,"FREEWRL_FONTS_DIR=");
		strcat(fdir,pp);
		_putenv( fdir );
		free(pp);
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

#endif

    /* install the signal handlers */

    signal(SIGTERM, (void(*)(int)) catch_SIGQUIT);
    signal(SIGSEGV, (void(*)(int)) catch_SIGSEGV);

#if !defined(CYGWIN)
    signal(SIGQUIT, (void(*)(int)) catch_SIGQUIT);
    signal(SIGALRM, (void(*)(int)) catch_SIGALRM);
    signal(SIGHUP,  (void(*)(int)) catch_SIGHUP);
#endif

    /* Before we parse the command line, setup the FreeWRL default parameters */
    params = calloc(1, sizeof(freewrl_params_t));

    /* Default values */
    params->width = 600;
    params->height = 400;
    params->eai = FALSE;
    params->fullscreen = FALSE;

#if !defined(TARGET_AQUA) /* Aqua front ends do the parsing */
    /* parse command line arguments */
    if (parseCommandLine(argc, argv)) {

	    start_url = argv[optind];
    }
#endif


    /* doug- redirect stdout to a file - works, useful for sending bug reports */
    /*freopen("freopen.txt", "w", stdout ); */

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
void catch_SIGQUIT() 
{
    /* ConsoleMessage ("FreeWRL got a sigquit signal"); */
    /* shut up any SIGSEGVs we might get now. */
    CaughtSEGV = TRUE;
    doQuit();
}

void catch_SIGSEGV()
{
    if (!CaughtSEGV) {
	ConsoleMessage ("FreeWRL got a SIGSEGV - can you please mail the file(s) to\n freewrl-09@rogers.com with a valid subject line. Thanks.\n");
	CaughtSEGV = TRUE;
    }
    exit(1);
}

#if !defined(CYGWIN)

void catch_SIGHUP()
{
    /* ConsoleMessage ("FreeWRL got a SIGHUP signal - reloading"); */
    /* MBFILES     Anchor_ReplaceWorld(BrowserFullPath); */
}

void catch_SIGALRM(int sig)
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
    signal(SIGALRM, catch_SIGALRM);
}

#endif

