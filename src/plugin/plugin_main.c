/*
  $Id: plugin_main.c,v 1.17 2011/04/03 10:56:58 couannette Exp $

  FreeWRL plugin for Mozilla compatible browsers.
  Works in Firefox 1.x - 3.0 on Linux.

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

/*******************************************************************************
 * Simple LiveConnect Sample Plugin
 * Copyright (c) 1996 Netscape Communications. All rights reserved.
 *
 * Modified by John Stewart  - CRC Canada to provide for plugin capabilities
 * for FreeWRL - an open source VRML and X3D browser.
 *
 * Operation:
 *
 * In the NPP_Initialize routine, a pipe is created and sent as the window
 * title to FreeWRL. FreeWRL (OpenGL/OpenGL.xs) looks at this "wintitle",
 * and if it starts with "pipe:", then treats the number following as
 * a pipe id to send the window id back through. The pipe is then closed.
 *
 * The Plugin uses this window id to rehost the window.
 *
 * John Stewart, Alya Khan, Sarah Dumoulin - CRC Canada 2002 - 2006.
 * Michel Briand - 2009.
 ******************************************************************************/

#include <config.h>
#include <system.h>

#if defined(HAVE_STDARG_H)
# include <stdarg.h>
#endif

#include <plugin_utils.h>
#include <npapi.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>

#define PLUGIN_NAME			"FreeWRL X3D/VRML"

#define BOOL_STR(b) (b ? "TRUE" : "FALSE")

#define RECORD_FILE_NAME_IF_NULL \
	if (me->fName == NULL) { \
		/* Get the base file name for FreeWRL to run */ \
		me->fName = (char *) NPN_MemAlloc((strlen(stream->url) +1) *sizeof(char *)); \
		strcpy(me->fName,stream->url); \
		PRINT("Can record filename now, name is %s\n", me->fName); \
	}

/* used in init. Don't write to the socket until a request has been received */
int gotRequestFromFreeWRL = FALSE;

char *paramline[20]; /* parameter line */
static void *seqNo = 0;

static int PluginVerbose = 1;  /* CHECK LOG FILE PATH BEFORE SETTING THIS TO 1 */

/*******************************************************************************
 * Instance state information about the plugin.
 ******************************************************************************/

typedef struct _FW_PluginInstance
{
	int			interfaceFile[2];
	Display 		*display;
	int32 			x, y;
	uint32 			width, height;
	Window 			mozwindow;
	Window 			fwwindow;
	pid_t 			childPID;
	char 			*fName;
	int			freewrl_running;
	int			interfacePipe[2];		/* pipe plugin FROM freewrl	*/
	char 			*cacheFileName;
	int 			cacheFileNameLen;
	FILE			*logFile;
	char			*logFileName;
} FW_PluginInstance;

typedef void (* Sigfunc) (int);

static int np_fileDescriptor;

/* Socket file descriptors */
#define SOCKET_2 0
#define SOCKET_1 1

#define PIPE_PLUGINSIDE  0
#define PIPE_FREEWRLSIDE 1

#if 0
static void signalHandler (int);
/* Sigfunc signal (int, Sigfunc func); */
void freewrlReceive(int fileDescriptor);
#endif

/* libFreeWRL defines those macros ...
   we will redefine them here for our own purpose
*/
#undef MALLOC
#define MALLOC NPN_MemAlloc
#undef FREE
#define FREE   NPN_MemFree

struct timeval mytime;
struct timezone tz; /* unused see man gettimeofday */
NPStream *currentStream = NULL;

/**
 * construct a log filename like this: 
 *  /tmp/npfreewrl-HOSTNAME-USERNAME.log 
 */
static void create_log_file(FW_PluginInstance *me)
{
	FILE *tty;
	char *logfilename, *hostname, *username;
	static const char log_file_pat[] = "/tmp/npfreewrl_%s-%s.log";

	hostname = MALLOC(4096);
	if (gethostname(hostname, 4096) < 0) {
		int err = errno;
		fprintf(stderr, "system error: %s\n", strerror(err));
		sprintf(hostname, "unknown-host");
	}
	username = getenv("LOGNAME");
	if (!username) {
		username = getlogin();
	}
	if (!username) {
		int err = errno;
		fprintf(stderr, "system error: %s\n", strerror(err));
		username = "unknown-user";
	}

	// -4  %s%s
	// +1  \n 
	logfilename = MALLOC(strlen(log_file_pat)
			     + strlen(hostname) + strlen(username) -4 +1 +4 );
	sprintf(logfilename, log_file_pat, hostname, username);
	FREE(hostname);
	/* do not free username */

	// Open log file (create it if doesn't exist
	tty = fopen(logfilename, "a");

	if (tty == NULL) {
		fprintf (stderr, "FreeWRL plugin ERROR: plugin could not open log file: %s. Will output to stderr.\n", logfilename);
		FREE(logfilename);
		logfilename = NULL;
		tty = stderr;
	}

	me->logFile = tty;
	me->logFileName = logfilename;
}

/**
 *   print_here: debugging routine. Use 'tty' as output stream.
 *               This stream is 'stderr' until an instance of the
 *               plugin is fully initialized. Then it's a tmp file.
 */
static void print(FW_PluginInstance *me, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	FILE *tty;
	double TickTime;

	if (!PluginVerbose) return;
	
        /* Set the timestamp */
        gettimeofday (&mytime,&tz);
        TickTime = (double) mytime.tv_sec + (double)mytime.tv_usec/1000000.0;

	if (!me)
		tty = stderr;
	else
		tty = me->logFile;

	fprintf(tty, "%f: FreeWRL plugin: ", TickTime);
	vfprintf(tty, format, ap);
	fflush(tty);

	va_end(ap);
}

#define PRINT(_formargs...) print(me, ##_formargs)

#define PRINT_PERROR(_msg) print(me, "system error: %s failed: %s (%d)\n", \
				 _msg, strerror(errno), errno)


#if 0
Sigfunc signal(int signo, Sigfunc func)
{
	struct sigaction action, old_action;

	action.sa_handler = func;
	/*
 	 * Initialize action's signal set as empty set
	 * (see man page sigsetops(3)).
	*/
	sigemptyset(&action.sa_mask);

	action.sa_flags = 0; /* Is this a good idea??? */

	/* Add option flags for handling signal: */
	action.sa_flags |= SA_NOCLDSTOP;
	#ifdef SA_NOCLDWAIT
	action.sa_flags |= SA_NOCLDWAIT;
	#endif

	if (sigaction(signo, &action, &old_action) < 0) {
		/* print_here("Call to sigaction failed"); */
		return(SIG_ERR);
	}
	/* Return the old action for the signal or SIG_ERR. */
	return(old_action.sa_handler);
}
#endif

#if 0
void signalHandler(int signo) {
	/* sprintf(debs, "ACTION signalHandler %d", signo); */
	/* print_here(debs); */

	if (signo == SIGIO) {
		freewrlReceive(np_fileDescriptor);

	} else {
		/* Should handle all except the uncatchable ones. */
		/* print_here("\nClosing plugin log.\n"); */
	}
}

void freewrlReceive(int fileDescriptor)
{
	FW_PluginInstance *me = NULL;
	sigset_t newmask, oldmask;

	urlRequest request;
	size_t request_size = 0;
	NPError rv = 0;

	sprintf(debs, "Call to freewrlReceive fileDescriptor %d.", fileDescriptor);
	print_here(debs);

	bzero(request.url, FILENAME_MAX);
	request.instance = 0;
	request.notifyCode = 0; /* not currently used */

	request_size = sizeof(request);

	/*
	 * The signal handling code is based on the work of
	 * W. Richard Stevens from Unix Network Programming,
	 * Networking APIs: Sockets and XTI.
	*/

	/* Init. the signal sets as empty sets. */
	if (sigemptyset(&newmask) < 0) {
		print_here("Call to sigemptyset with arg newmask failed");
		return;
	}

	if (sigemptyset(&oldmask) < 0) {
		print_here("Call to sigemptyset with arg oldmask failed");
		return;
	}

	if (sigaddset(&newmask, SIGIO) < 0) {
		print_here("Call to sigaddset failed");
		return;
	}

	/* Code to block SIGIO while saving the old signal set. */
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
		print_here("Call to sigprocmask failed");
		return;
	}

	/* If blocked or interrupted, be silent. */
	if (read(fileDescriptor, (urlRequest *) &request, request_size) < 0) {
		if (errno != EINTR && errno != EAGAIN) {
			print_here("Call to read failed");
		}
		/* FreeWRL has died, or THIS IS US WRITING CREATING THAT SIG. */
		print_here ("freewrlReceive, quick return; either this is us writing or freewrl croaked");
		return;
	} else {
		sprintf (debs, "notifyCode = %d url = %s", request.notifyCode, request.url);
		print_here(debs);

		/* signal that we have now received a file request from FreeWRL */
		gotRequestFromFreeWRL = TRUE;

		/* is this a getUrl, or a "open new window for url" */
		if (request.notifyCode == 0) {
			/* get Url and return it to FreeWRL */

			seqNo++;
			/* printf ("request seq %d, url %s\n",seqNo, request.url); */

			if ((rv = NPN_GetURLNotify(request.instance, 
						   request.url, NULL,
						   (void *)(seqNo))) != NPERR_NO_ERROR) {
				sprintf(debs, "Call to NPN_GetURLNotify failed with error %d.", rv);
				print_here(debs);
			}


			sprintf (debs, "step 2a, NPN_GetURLNotify with request.url %s",request.url);
			print_here(debs);

		} else if (request.notifyCode == -99) {
			/* Firefox, etc took too long. we have timed out. */
			sprintf (debs,"notifyCode = -99, we have timed out for %s",request.url);
			print_here(debs);
			if (currentStream != NULL) {
				NPN_DestroyStream(request.instance, currentStream, NPRES_USER_BREAK);
				sprintf (debs, "FreeWRL can not find: %s",request.url);
				print_here(debs);
				NPN_Status (request.instance, debs);
				currentStream = NULL;
			}

		} else {
			/* request.notifyCode must be 1 */
			sprintf (debs,"NPN_GetStream...");
			print_here(debs);

			NPStream* stream;
    			NPError err = NPERR_NO_ERROR;
			char* myData = "<HTML><B>This is a message from my plug-in!</b></html>";
			int32 myLength = strlen(myData) + 1;
			err = NPN_NewStream(request.instance,
					"text/html",
					"_AnchorFailsinFreeWRL",
					&stream);
			print_here ("NewStream made");

			err = NPN_Write(request.instance,
					stream,
					myLength,
					myData);
			print_here ("NPN_Write made");
		}

		/* now, put a status line on bottom of browser */
		sprintf (debs, "FreeWRL loading: %s",request.url);
		print_here(debs);
		NPN_Status (request.instance, debs); 
	}

	/* Restore old signal set, which unblocks SIGIO. */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
		print_here("Call to sigprocmask failed");
		return;
	}

	print_here("returning from freewrl_receive");
	return;
}
#endif

static int init_socket(FW_PluginInstance *me, int fileDescriptor, Boolean nonblock)
{
	int io_flags;

	if (fcntl(fileDescriptor, F_SETOWN, getpid()) < 0) {
		PRINT("Call to fcntl with command F_SETOWN failed\n");
		return(NPERR_GENERIC_ERROR);
	}

	if ( (io_flags = fcntl(fileDescriptor, F_GETFL, 0)) < 0 ) {
		PRINT("Call to fcntl with command F_GETFL failed\n");
		return(NPERR_GENERIC_ERROR);
	}

	/*
	 * O_ASYNC is specific to BSD and Linux.
	 * Use ioctl with FIOASYNC for others.
	*/
	#ifndef __sgi
	io_flags |= O_ASYNC;
	#endif

	if (nonblock) { io_flags |= O_NONBLOCK; }

	if ( (io_flags = fcntl(fileDescriptor, F_SETFL, io_flags)) < 0 ) {
		PRINT("Call to fcntl with command F_SETFL failed\n");
		return(NPERR_GENERIC_ERROR);
	}
	return(NPERR_NO_ERROR);
}

/* actually run FreeWRL and swallow it, if enough information has been found */
int Run (NPP instance)
{
	FW_PluginInstance* me = (FW_PluginInstance *) instance->pdata;

	char pipetome[25];
	char childFd[25];
	char instanceStr[25];
	int carg = 0;

	XWindowAttributes mywin;
	Window child_window = 0;

	pid_t child;
	pid_t mine;

	int secpipe[2];
#define clean_pipe { close(secpipe[0]); close(secpipe[1]); }

	int err;
	int count;
	int nbytes;

	PRINT("Run starts... Checking if can run; disp %u win %u fname %s\n",
	      me->mozwindow, me->display, me->fName); 

	/* Return if we do not have all of the required parameters. */
	if (me->mozwindow == 0) return FALSE;

	if (me->fName == NULL) return FALSE;

	if (me->display == 0) return FALSE;

	PRINT("Run ... ok\n");

	/* start FreeWRL, if it is not running already. */
	if (me->freewrl_running) {
		PRINT("Run ... FreeWRL already running, returning.\n");
		return TRUE;
	}

	if (pipe(secpipe) < 0) {
		PRINT_PERROR("pipe");
		return FALSE;
	}
	
	if (fcntl(secpipe[1], F_SETFD, fcntl(secpipe[1], F_GETFD) | FD_CLOEXEC)) {
		PRINT_PERROR("fcntl");
		clean_pipe;
		return FALSE;
	}
	
	switch ((child = fork())) {
	case -1:
		PRINT_PERROR("fork");
		clean_pipe;
		return FALSE;
	case 0:
		mine = getpid();
		if (setpgid(mine, mine) < 0) {
			PRINT_PERROR("setpgid");
		}

		/* create pipe string */
		sprintf(pipetome, "pipe:%d",
			me->interfacePipe[PIPE_FREEWRLSIDE]);
	
		/* child file descriptor - to send requests back here */
		sprintf(childFd, "%d", me->interfaceFile[SOCKET_2]);
	
		/* Instance, so that FreeWRL knows its us... */
		sprintf(instanceStr, "%lu",
			(unsigned long int) (uintptr_t) instance);

		/* Nice FreeWRL to a lower priority */
		paramline[carg++] = "nice";
		paramline[carg++] = "freewrl";
		paramline[carg++] = "--logfile";

		if (me->logFileName) {
			paramline[carg++] = me->logFileName;
		} else {
			paramline[carg++] = "-"; // no log file (stderr)
		
			/* this is usefull to build a test case
			   for child death... because FreeWRL
			   will exit with this param... */
			/* paramline[carg++] = NULL; */
		}

		/* We have the file name, so include it */
		paramline[carg++] = me->fName;
	
		/* Pass in the pipe number so FreeWRL can return the
		   window id */
		paramline[carg++] = "--plugin";
		paramline[carg++] = pipetome;
	
		/* EAI connection */
		paramline [carg++] = "--eai";
	
		/* File descriptor and instance  - allows FreeWRL to
		   request files from browser's cache */
		paramline[carg++] = "--fd";
		paramline[carg++] = childFd;
		paramline[carg++] = "--instance";
		paramline[carg++] = instanceStr;
		paramline[carg] = NULL;
		/* End of arguments */
		
		PRINT("exec param line is %s %s %s %s %s %s %s %s %s %s %s\n",
		      paramline[0],paramline[1],paramline[2],paramline[3],
		      paramline[4],paramline[5],paramline[6],paramline[7],
		      paramline[8],paramline[9],paramline[10]);
			
		close(secpipe[0]);
		execvp(paramline[0], paramline);
		write(secpipe[1], &errno, sizeof(int));
		_exit(0);
		break;
		
	default:
		close(secpipe[1]);
		while ((count = read(secpipe[0], &err, sizeof(errno))) == -1)
			if (errno != EAGAIN && errno != EINTR) break;
		
		if (count) {
			PRINT_PERROR("execvp");
			clean_pipe;
			return FALSE;
		}
		
		close(secpipe[0]);
#if 0
		PRINT("waiting for child...\n");
		while (waitpid(child, &err, 0) == -1)
			if (errno != EINTR) {
				PRINT_PERROR("waitpid");
				return FALSE;
			}
		
		if (WIFEXITED(err))
			PRINT("child exited with %d\n", WEXITSTATUS(err));
		
		else if (WIFSIGNALED(err))
			PRINT("child killed by %d\n", WTERMSIG(err));
#endif
	}

	me->childPID = child;
	PRINT("CHILD %d\n", me->childPID);

	PRINT("after FW_Plugin->freewrl_running call - waiting on pipe\n");

	usleep(1500);

	nbytes = read(me->interfacePipe[PIPE_PLUGINSIDE], &child_window, sizeof(Window));
	if ((nbytes < 0) || (nbytes == 0)) {
		int status = 0;
		// error reading pipe: child died
		PRINT("ERROR: child %d FreeWRL program died (%d), waiting...\n",
		      me->childPID, nbytes);

		switch (waitpid(me->childPID, &status, WNOHANG)) {
		case 0: PRINT("child is gone (nothing to wait), exit code: %d\n", status);
			break;
		case -1: PRINT_PERROR("waitpid");
			break;
		default: PRINT("child passed away, exit code: %d\n", status);
			break;
		}

		me->childPID = 0;
		return FALSE;
	}

	PRINT("After exec, and after read from pipe, FW window is %u\n",
	      child_window);

	me->fwwindow = child_window;

	PRINT("disp mozwindow height width %u %u %u %u\n",
	      me->display, me->mozwindow, me->width, me->height);

	/*reparent the window */

	XGetWindowAttributes(me->display,me->fwwindow, &mywin);

	PRINT("Plugin: mapped_state %d, IsUnmapped %d, isUnviewable %d isViewable %d\n"
	      "x %d y %d wid %d height %d\n",
	      mywin.map_state, IsUnmapped, IsUnviewable, IsViewable,
	      mywin.x,mywin.y,mywin.width,mywin.height);

	/* print_here ("going to XFlush"); */
	/* XFlush(me->display); */
	
	/* print_here ("going to XSync"); */
	/* XSync (me->display, FALSE); */
	
	PRINT("Going to resize FreeWRL: %d x %d -> %d x %d\n",
	      mywin.width, mywin.height, me->width, me->height);

	/* MB 28-12-2009 : added this sync to prevent the plugin from "loosing" the resize event ... */
	/* XSync (me->display, FALSE); */

	/* here the two next calls seems to be operating well in any order... hum... */
	{
		XSizeHints size_hints;
		memset(&size_hints, 0, sizeof(size_hints));
		size_hints.min_width = size_hints.max_width = me->width;
		size_hints.min_height = size_hints.max_height = me->height;
		XSetWMNormalHints(me->display, me->fwwindow, &size_hints);
	}
	XResizeWindow(me->display, me->fwwindow, me->width, me->height);

	PRINT("Going to reparent\n");
	XReparentWindow(me->display, me->fwwindow, me->mozwindow, 0,0);

	PRINT("Going to remap\n");
	XMapWindow(me->display,me->fwwindow);

	XGetWindowAttributes(me->display,me->fwwindow, &mywin);

	PRINT("Plugin, after reparenting, mapped_state %d, "
	      "IsUnmapped %d, isUnviewable %d isViewable %d\n"
	      "x %d y %d wid %d height %d\n",
	      mywin.map_state, IsUnmapped, IsUnviewable, IsViewable,
	      mywin.x,mywin.y,mywin.width,mywin.height);

	me->freewrl_running = TRUE;
	
	PRINT("Run function finished\n");
	return TRUE;
}


/*******************************************************************************
 ******************************************************************************/
/*const*/ char* NPP_GetMIMEDescription(void)
{
	static const char mime_types[] =
		"x-world/x-vrml:wrl:FreeWRL VRML Browser;"
		"model/vrml:wrl:FreeWRL VRML Browser;"
		"model/x3d:x3d:FreeWRL X3D Browser;"
		"model/x3d+xml:x3d:FreeWRL X3D Browser;"
		"model/x3d+vrml:x3dv:FreeWRL X3D Browser;"
		"model/x3d+binary:x3db:FreeWRL X3D Browser"
		;

	/* print_here ("NPP_GetMIMEDescription"); */
	return (char *) mime_types;
/*
        return("x-world/x-vrml:wrl:FreeWRL VRML Browser;model/vrml:wrl:FreeWRL VRML Browser;model/x3d+vrml:x3dv:FreeWRL VRML Browser;model/x3d+xml:x3d:FreeWRL X3D Browser;model/x3d+vrml:x3dv:FreeWRL X3D Browser;model/x3d+binary:x3db:FreeWRL X3D Browser");
*/
}

NPError
NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
#define VERSION_DESCRIPTION_SIZE 1024
	static char version_description[VERSION_DESCRIPTION_SIZE];
	FW_PluginInstance* me = NULL; /* instance may be NULL */
	NPError err = NPERR_NO_ERROR;

	if (!value) return NPERR_GENERIC_ERROR;

	if (instance)
		me = (FW_PluginInstance *) instance->pdata;

	PRINT("NPP_GetValue %u\n", variable);

	switch (variable) {
	case NPPVpluginNameString:
		*((char **)value) = PLUGIN_NAME;
		break;

	case NPPVpluginDescriptionString:
		snprintf(version_description, VERSION_DESCRIPTION_SIZE,
			 "<b>FreeWRL is a VRML/X3D plugin.</b><br>"
			 "Visit us at <a href=\"http://freewrl.sourceforge.net/\">"
			 "http://freewrl.sourceforge.net/</a>.<br>"
			 "Plugin version: <b>%s</b>.<br>"
			 "Build timestamp: <b>%s</b>.<br>",
			 freewrl_plugin_get_version(),
			 BUILD_TIMESTAMP);
		*((char **)value) = version_description;
		break;

	default:
		err = NPERR_INVALID_PARAM;
	}
	return err;
}

/*******************************************************************************
 * General Plug-in Calls
 ******************************************************************************/

/*
** NPP_Initialize is called when your DLL is being loaded to do any
** DLL-specific initialization.
*/
NPError NPP_Initialize(void) {
	/* print_here ("NPP_Initialize"); */
    	return NPERR_NO_ERROR;
}

jref NPP_GetJavaClass( void )
{
    return NULL;
}

/*
** NPP_Shutdown is called when your DLL is being unloaded to do any
** DLL-specific shut-down. You should be a good citizen and declare that
** you're not using your java class any more. FW_Plugin allows java to unload
** it, freeing up memory.
*/
void NPP_Shutdown(void) { 
	/* print_here ("NPP_Shutdown"); */
}

/*
** NPP_New is called when your plugin is instantiated (i.e. when an EMBED
** tag appears on a page).
*/
NPError
NPP_New(NPMIMEType pluginType,
		NPP instance,
		uint16 mode,
		int16 argc,
		char* argn[],
		char* argv[],
		NPSavedData* saved) {

	FW_PluginInstance* me = NULL; /* only case */
	unsigned int err;
	void *tmp;

	if( instance == NULL ) {
		return NPERR_INVALID_INSTANCE_ERROR;
	}

	/* Create plugin instance structure */
	tmp = NPN_MemAlloc(sizeof(FW_PluginInstance));
	if (!tmp)
	    return NPERR_OUT_OF_MEMORY_ERROR;

	instance->pdata = tmp;
	me = (FW_PluginInstance*) tmp;
	memset(me, 0, sizeof(FW_PluginInstance));

	/* Create log file */
	create_log_file(me);
	PRINT("FreeWRL plugin log restarted. Version: %s. Build: %s\n",
	      freewrl_plugin_get_version(), BUILD_TIMESTAMP);

	PRINT("NPP_New, argc %d argn %s  argv %s\n", argc, argn[0], argv[0]);

	/* mode is NP_EMBED, NP_FULL, or NP_BACKGROUND (see npapi.h) */
	switch (mode) {
	case NP_EMBED: PRINT("NPP_New, mode NP_EMBED\n"); break;
	case NP_FULL: PRINT("NPP_New, mode NP_FULL\n"); break;
	default:  PRINT("NPP_New, mode UNKNOWN MODE\n"); break;
	}

	seqNo = 0;
	gotRequestFromFreeWRL = FALSE;

	if (pipe(me->interfacePipe) < 0) {
		PRINT("Pipe connection to FW_Plugin->interfacePipe failed: %d,%s [%s:%d]\n",
		      errno, strerror(errno), __FILE__,__LINE__);
	}

	PRINT("Pipe created, PIPE_FREEWRLSIDE %d PIPE_PLUGINSIDE %d\n",
	      me->interfacePipe[PIPE_FREEWRLSIDE], me->interfacePipe[PIPE_PLUGINSIDE]);

	/* Assume plugin and FreeWRL child process run on the same machine, 
	 then we can use UDP and have incredibly close to 100.00% reliability */
	
	if (socketpair(AF_LOCAL, SOCK_DGRAM, 0, me->interfaceFile) < 0) {
		PRINT("Call to socketpair failed\n");
		return (NPERR_GENERIC_ERROR);
	}
	PRINT("file pair created, SOCKET_1 %d SOCKET_2 %d\n",
	      me->interfaceFile[SOCKET_1], me->interfaceFile[SOCKET_2]);

	np_fileDescriptor = me->interfaceFile[SOCKET_1];

#if 0
	if (signal(SIGIO, signalHandler) == SIG_ERR) return (NPERR_GENERIC_ERROR);
	if (signal(SIGBUS, signalHandler) == SIG_ERR) return (NPERR_GENERIC_ERROR);
#endif

	/* prepare communication sockets */
	if ((err=init_socket(me, me->interfaceFile[SOCKET_2], FALSE))!=NPERR_NO_ERROR)
		return err;
	if ((err=init_socket(me, me->interfaceFile[SOCKET_1], TRUE))!=NPERR_NO_ERROR)
		return err;
	PRINT("NPP_New returning %d\n", err);
	return err;
}


NPError
NPP_Destroy(NPP instance, NPSavedData** save)
{
	FW_PluginInstance* me = (FW_PluginInstance*) instance->pdata;
	int status;

	/* fprintf(stderr, "NPP_Destroy (%p %p)\n", (void*)instance, (void*)save); */

	PRINT("NPP_Destroy begin\n");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	if (me != NULL) {

		if (me->fName != NULL) {
			NPN_MemFree(me->fName);
		}

		if (me->childPID >0) {

			PRINT("killing command kill %d\n", me->childPID);
			/* which signal ? TERM, QUIT or KILL */
			kill(me->childPID, SIGTERM);
			waitpid(me->childPID, &status, 0);
		}

		if (me->cacheFileName != NULL) {
			NPN_MemFree(me->cacheFileName);
		}

		if (me->interfacePipe[PIPE_FREEWRLSIDE] != 0) {
			close (me->interfacePipe[PIPE_FREEWRLSIDE]);
			close (me->interfacePipe[PIPE_PLUGINSIDE]);
		}

		NPN_MemFree(instance->pdata);
		instance->pdata = NULL;
	}
	me->freewrl_running = FALSE;
	gotRequestFromFreeWRL = FALSE;

	PRINT("NPP_Destroy end\n");
	return NPERR_NO_ERROR;
}

void 
NPP_URLNotify (NPP instance, const char *url, NPReason reason, void* notifyData)
{
	FW_PluginInstance* me = (FW_PluginInstance*) instance->pdata;
#define returnBadURL "this file is not to be found on the internet"
	int bytes;

	PRINT("NPP_URLNotify, url %s reason %d notifyData %p\n",
	      url, reason, notifyData);

	if (seqNo != notifyData) {
		PRINT("NPP_URLNotify, expected seq %p, got %p for %s\n",
		      seqNo, notifyData, url);
		return;
	}

	if (reason == NPRES_DONE) {
		PRINT("NPP_UrlNotify - NPRES_DONE\n");
		bytes = (strlen(me->cacheFileName)+1)*sizeof(const char *);
		if (write(me->interfaceFile[SOCKET_1], me->cacheFileName, bytes) < 0) {
			PRINT("Call to write failed\n");
		}

		/* send a "done" message to status bar */
		NPN_Status(instance,"FreeWRL: Done");
		return;

	} else if (reason == NPRES_USER_BREAK) {
		PRINT("NPP_UrlNotify - NPRES_USER_BREAK\n");
	} else if (reason == NPRES_NETWORK_ERR) {
		PRINT("NPP_UrlNotify - NPRES_NETWORK_ERR\n");
	} else {
		PRINT("NPP_UrlNotify - unknown\n");
	}

	PRINT("NPP_UrlNotify - writing %s (%u bytes) to socket %d\n",
	      returnBadURL, strlen(returnBadURL), me->interfaceFile[SOCKET_1]);

	NPN_Status(instance,"FreeWRL: NPP_URLNotify failed");

	/* if we got a request from FreeWRL for the file, then return
	   the name. If FreeWRL was "Run", from within NPP_NewStream,
	   it will not be expecting  this write, until it asks for a
	   file - a case of "the cart before the horse" */

	if (gotRequestFromFreeWRL) {
		PRINT("NPP_UrlNotify, gotRequestFromFreeWRL - writing data\n");
		if (write(me->interfaceFile[SOCKET_1], returnBadURL, 
			  strlen(returnBadURL)) < 0) {
			PRINT("Call to write failed\n");
		}
	} else {
		PRINT("call to write (for returnBadURL) skipped, because gotRequestFromFreeWRL = FALSE\n");
	}
}


NPError
NPP_SetWindow(NPP instance, NPWindow *browser_window)
{
	FW_PluginInstance* me = (FW_PluginInstance*) instance->pdata;
	NPError result = NPERR_NO_ERROR;

	PRINT("start of NPP_SetWindow\n"); 

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	/* do we have a file name yet? */
	PRINT("file name in SetWindow is %s\n", me->fName);

	/* set the display, if we know it yet */ 
	if (!me->display) { 
		if ((NPSetWindowCallbackStruct *)(browser_window->ws_info) != NULL) { 
			me->display = ((NPSetWindowCallbackStruct *) 
				browser_window->ws_info)->display; 
 
			PRINT("NPP_SetWindow, plugin display now is %u\n", me->display); 
		} 
	} 

	/* verify that the display has not changed */
	if ((NPSetWindowCallbackStruct *)(browser_window->ws_info) != NULL) { 
		if ((me->display) != ((NPSetWindowCallbackStruct *)
                                browser_window->ws_info)->display) {

			PRINT("HMMM - display has changed\n");
			me->display = ((NPSetWindowCallbackStruct *)
				       browser_window->ws_info)->display;
		}
	}

	PRINT("NPP_SetWindow, moz window is %u childPID is %u\n",
	      browser_window->window, me->childPID);

	me->width = browser_window->width;
	me->height = browser_window->height;


	if (me->mozwindow != (Window) browser_window->window) {
		me->mozwindow = (Window) browser_window->window;

		/* run FreeWRL, if it is not already running. It might not be... */
		if (!me->freewrl_running) {

			PRINT("NPP_SetWindow, running FreeWRL here!\n");

			if (!Run(instance)) {
				PRINT("NPP_SetWindow, FreeWRL program failed!\n");
				return NPERR_MODULE_LOAD_FAILED_ERROR;
			}

			PRINT("NPP_SetWindow, returned from Run!\n");
		}
	}

	/* Handle the FreeWRL window */
	if (me->fwwindow) {
		PRINT("xresize x %d y %d  wid %d hei %d\n",
			me->x, me->y, me->width, me->height);

		XResizeWindow(me->display, me->fwwindow,
			me->width, me->height);

		XSync (me->display,FALSE);
	}
	PRINT("exiting NPP_SetWindow\n");
	return result;
}


NPError
NPP_NewStream(NPP instance,
	      NPMIMEType type,
	      NPStream *stream,
	      NPBool seekable,
	      uint16 *stype)
{
	FW_PluginInstance* me = (FW_PluginInstance*) instance->pdata;

	if (instance == NULL) return NPERR_INVALID_INSTANCE_ERROR;

	if (stream->url == NULL) return(NPERR_NO_DATA);

	if (currentStream == NULL) {
		currentStream = stream;
	} else {
		PRINT("NPP_NewStream, currentstream NOT NULL\n");
	}

	PRINT("NPP_NewStream, filename %s instance %p, type %s, "
	      "stream %p, seekable %s stype %d\n",
	      me->fName, instance, type, 
	      stream, BOOL_STR(seekable), (stype ? (*stype) : 0));

	RECORD_FILE_NAME_IF_NULL;

	/* run FreeWRL, if it is not already running. It might not be... */
	if (!me->freewrl_running) {

		PRINT("NPP_NewStream, running FreeWRL here!\n");

		if (!Run(instance)) {
			PRINT("NPP_NewStream, FreeWRL program failed!\n");
			return NPERR_MODULE_LOAD_FAILED_ERROR;
		}
	}

	/* Lets tell netscape to save this to a file. */
	*stype = NP_ASFILEONLY;
	seekable = FALSE;

	PRINT("NPP_NewStream returning noerror\n");
	return NPERR_NO_ERROR;
}


/* PLUGIN DEVELOPERS:
 *	These next 2 functions are directly relevant in a plug-in which
 *	handles the data in a streaming manner. If you want zero bytes
 *	because no buffer space is YET available, return 0. As long as
 *	the stream has not been written to the plugin, Navigator will
 *	continue trying to send bytes.  If the plugin doesn't want them,
 *	just return some large number from NPP_WriteReady(), and
 *	ignore them in NPP_Write().  For a NP_ASFILE stream, they are
 *	still called but can safely be ignored using this strategy.
 */

int32 STREAMBUFSIZE = 0X0FFFFFFF; /* If we are reading from a file in NPAsFile
				   * mode so we can take any size stream in our
				   * write call (since we ignore it) */

int32
NPP_WriteReady(NPP instance, NPStream *stream)
{
	FW_PluginInstance* me = (FW_PluginInstance *) instance->pdata;
	PRINT("NPP_WriteReady\n");
	/* Number of bytes ready to accept in NPP_Write() */
	return STREAMBUFSIZE;
}


int32 NPP_Write(NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{
	FW_PluginInstance* me = (FW_PluginInstance *) instance->pdata;
	PRINT("NPP_Write\n");
	return 0;
	return len;		/* The number of bytes accepted */
}


NPError
NPP_DestroyStream(NPP instance, NPStream *stream, NPError reason)
{
	FW_PluginInstance* me = (FW_PluginInstance *) instance->pdata;

	PRINT("NPP_DestroyStream, instance %p stream %p\n",
	      instance, stream);

	if (reason == NPRES_DONE) PRINT("reason: NPRES_DONE\n");
	if (reason == NPRES_USER_BREAK) PRINT("reason: NPRES_USER_BREAK\n");
	if (reason == NPRES_NETWORK_ERR) PRINT("reason: NPRES_NETWORK_ERR\n");

	if (stream == currentStream) {
		currentStream = NULL;
	} else {
		PRINT("NPP_DestroyStream, STREAMS DO NOT MATCH!\n");
	}

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	return NPERR_NO_ERROR;
}


void
NPP_StreamAsFile(NPP instance, NPStream *stream, const char* fname)
{
	FW_PluginInstance* me = (FW_PluginInstance *) instance->pdata;
	int bytes;

	PRINT("NPP_StreamAsFile, start with fname %s\n", fname);

	if (instance != NULL) {
		me = (FW_PluginInstance*) instance->pdata;
	
		RECORD_FILE_NAME_IF_NULL;

		if (!me->freewrl_running) {
			/* if we are not running yet, see if we have enough to start. */
			if (!Run(instance)) {
				PRINT("NPP_StreamAsFile, FreeWRL program failed!\n");
				/* TODO: clean-up */
				return;
			}

		} else {
			if (fname == NULL) {
				PRINT("NPP_StreamAsFile has a NULL file\n");

				/* Try sending an empty string */
				if (write(me->interfaceFile[SOCKET_1], "", 1) < 0) {
					PRINT("Call to write failed\n");
				}
			} else {
			
				/* if we got a request from FreeWRL
				   for the file, then return the
				   name. If FreeWRL was "Run", from
				   within NPP_NewStream, it will not
				   be expecting this write, until it
				   asks for a file - a case of "the
				   cart before the horse" */

				if (gotRequestFromFreeWRL) {
					bytes = (strlen(fname)+1)*sizeof(const char *);
					if (bytes > (me->cacheFileNameLen -10)) {
						if (me->cacheFileName != NULL) {
							NPN_MemFree(me->cacheFileName);
						}

						me->cacheFileNameLen = bytes+20;
						me->cacheFileName = NPN_MemAlloc(me->cacheFileNameLen);
					}

					memcpy (me->cacheFileName, fname, bytes);
					PRINT("NPP_StreamAsFile: saving name to cachename\n");
				} else {
					PRINT("NPP_StreamAsFile: skipping file write, as gotRequestFromFreeWRL = FALSE\n");
				}	
			}
		}
	}
}


void
NPP_Print(NPP instance, NPPrint* printInfo)
{
	if(printInfo == NULL)
		return;

	if (instance != NULL) {

		if (printInfo->mode == NP_FULL) {
		    /*
		     * PLUGIN DEVELOPERS:
		     *	If your plugin would like to take over
		     *	printing completely when it is in full-screen mode,
		     *	set printInfo->pluginPrinted to TRUE and print your
		     *	plugin as you see fit.  If your plugin wants Netscape
		     *	to handle printing in this case, set
		     *	printInfo->pluginPrinted to FALSE (the default) and
		     *	do nothing.  If you do want to handle printing
		     *	yourself, printOne is true if the print button
		     *	(as opposed to the print menu) was clicked.
		     *	On the Macintosh, platformPrint is a THPrint; on
		     *	Windows, platformPrint is a structure
		     *	(defined in npapi.h) containing the printer name, port,
		     *	etc.
		     */

			/* void* platformPrint = */
				/* printInfo->print.fullPrint.platformPrint; */
			/* NPBool printOne = */
				/* printInfo->print.fullPrint.printOne; */

			/* Do the default*/
			printInfo->print.fullPrint.pluginPrinted = FALSE;
		}
		else {	/* If not fullscreen, we must be embedded */
			/* NPWindow* printWindow = */
				/* &(printInfo->print.embedPrint.window); */
			/* void* platformPrint = */
				/* printInfo->print.embedPrint.platformPrint; */
		}
	}
}

/* Local Variables: */
/* c-basic-offset: 8 */
/* End: */
