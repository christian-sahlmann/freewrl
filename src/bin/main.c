/*
=INSERT_TEMPLATE_HERE=

$Id: main.c,v 1.18 2009/10/02 20:53:41 dug9 Exp $

FreeWRL main program.

*/


/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2009 CRC Canada. (http://www.crc.gc.ca)

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
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
 * Local variables
 */
static int CaughtSEGV = FALSE;
int wantEAI = FALSE;

/**
 * Signal handlers 
 */
void catch_SIGQUIT();
void catch_SIGSEGV();

#if !defined(CYGWIN)
void catch_SIGALRM(int);
void catch_SIGHUP();
#endif

#if defined(_MSC_VER)
const char *freewrl_get_version()
{
	return "version 1.22.4";
}
#endif

/**
 * Main
 */
int main (int argc, char **argv)
{
    char *pwd;
    char *initialFilename;	/* file to start FreeWRL with */
    const char *libver, *progver;

    /* first, get the FreeWRL shared lib, and verify the version. */
    libver = libFreeWRL_get_version();
    progver = freewrl_get_version();
    if (strcmp(progver, libver)) {
	ConsoleMessage("FreeWRL expected library version %s, got %s...\n",progver, libver);
    }

    /* set the screen width and height before getting into arguments */
    win_width = 600;
    win_height = 400;
    fullscreen = 0;
/*     wantEAI = 0; */

    /* install the signal handlers */

    signal(SIGTERM, (void(*)(int)) catch_SIGQUIT);
    signal(SIGSEGV, (void(*)(int)) catch_SIGSEGV);

#if !defined(CYGWIN)
    signal(SIGQUIT, (void(*)(int)) catch_SIGQUIT);
    signal(SIGALRM, (void(*)(int)) catch_SIGALRM);
    signal(SIGHUP,  (void(*)(int)) catch_SIGHUP);
#endif

    /* parse command line arguments */
    /* JAS - for last parameter of long_options entries, choose
     * ANY character that is not 'h', and that is not used */
    if (parseCommandLine(argc, argv)) {
	/* create the initial scene, from the file passed in
	   and place it as a child of the rootNode. */

	/* FIXME: try to NEVER use immediate size; compute length with argument's lengths */
	initialFilename = (char *) malloc(1000 * sizeof (char));
	pwd = (char *) malloc(1000 * sizeof (char));

	getcwd(pwd,1000);

	/* if this is a network file, leave the name as is. If it is
	   a local file, prepend the path to it */

	if (checkNetworkFile(argv[optind])) {
	    setFullPath(argv[optind]);
	} else {
#ifdef _MSC_VER
		strcpy(initialFilename,argv[optind]);
#else
	    makeAbsoluteFileName(initialFilename, pwd, argv[optind]);
#endif
	    setFullPath(initialFilename);
	}
	free(initialFilename);
    } else {
	BrowserFullPath = NULL;
    }

    /* doug- redirect stdout to a file - works, useful for sending bug reports */
    /*freopen("freopen.txt", "w", stdout ); */

    /* start threads, parse initial scene, etc */
    initFreewrl();

    /* do we require EAI? */
    if (wantEAI) {
	create_EAI();
    }

    /* give control to the library */
    startFreeWRL();
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
    Anchor_ReplaceWorld(BrowserFullPath);
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

