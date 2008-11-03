/*******************************************************************
 *
 * FreeX3D main program
 *
 * main.c
 *
 *******************************************************************/

#include "config.h"
#include "system.h"

#include "display.h"
#include "libFreeX3D.h"

#include "fwopts.h"

/**
 * Local variables
 */
static int CaughtSEGV = FALSE;

/**
 * Signal handlers 
*/
void catch_SIGQUIT();
void catch_SIGSEGV();
void catch_SIGHUP();
void catch_SIGALRM(int);

/**
 * Main
 */
int main(int argc, char *argv[])
{
	char *pwd;
	char *initialFilename;	/* file to start FreeWRL with */

	/**
	 * TODO: check the library version
	 */

	/* set the screen width and height before getting into arguments */
	feWidth = 600; feHeight=400;
	fullscreen = 0;
	wantEAI = 0;

	/* install the signal handler for SIGQUIT */
	signal (SIGQUIT, (void(*)(int))catch_SIGQUIT);
	signal (SIGTERM, (void(*)(int))catch_SIGQUIT);
	signal (SIGSEGV,(void(*)(int))catch_SIGSEGV);
	signal (SIGALRM,(void(*)(int))catch_SIGALRM);
	signal (SIGHUP, (void(*)(int))catch_SIGHUP);

	/* parse command line arguments */
	/* JAS - for last parameter of long_options entries, choose
	 * ANY character that is not 'h', and that is not used */
	if (parseCommandLine (argc, argv)) {
        	/* create the initial scene, from the file passed in
		   and place it as a child of the rootNode. */

        	initialFilename = (char *)MALLOC(1000 * sizeof (char));
        	pwd = (char *)MALLOC(1000 * sizeof (char));
        	getcwd(pwd,1000);
        	/* if this is a network file, leave the name as is. If it is
        	   a local file, prepend the path to it */

	        if (checkNetworkFile(argv[optind])) {
			setFullPath(argv[optind]);
	
	        } else {
	                makeAbsoluteFileName(initialFilename, pwd, argv[optind]);
	                setFullPath(initialFilename);
	        }
		FREE_IF_NZ (initialFilename);
	} else {
		BrowserFullPath = NULL;
	}

	/* start threads, parse initial scene, etc */
	initFreewrl();

	/* do we require EAI? */
	if (wantEAI) create_EAI();

	/* now wait around until something kills this thread. */
	pthread_join(DispThrd, NULL);
}

void catch_SIGQUIT()
{
	/* ConsoleMessage ("FreeWRL got a SIGQUIT signal - exiting"); */
	/* Shut up any SIGSEGVs we might get now. */
	CaughtSEGV = TRUE;
    	doQuit();
}

void catch_SIGHUP()
{
	/* ConsoleMessage ("FreeWRL got a SIGHUP signal - reloading"); */
	Anchor_ReplaceWorld(BrowserFullPath);
}

void catch_SIGSEGV()
{
	if (!CaughtSEGV) {
		ConsoleMessage ("FreeWRL got a SIGSEGV - can you please mail the file(s) to\n freewrl-09@rogers.com with a valid subject line. Thanks.\n");
		CaughtSEGV = TRUE;
	}
	exit(1);
}

void catch_SIGALRM(int sig)
{
	signal(SIGALRM, SIG_IGN);

	/* stuffs to do on alarm */
	fprintf(stderr,"An alarm signal just arrived ...IT WAS IGNORED!\n");
	/* end of alarm actions */

	alarm(0);
	signal(SIGALRM, catch_SIGALRM);
}
