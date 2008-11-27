/*
=INSERT_TEMPLATE_HERE=

$Id: PluginSocket.c,v 1.2 2008/11/27 00:27:18 couannette Exp $

Common functions used by Mozilla and Netscape plugins...(maybe PluginGlue too?)

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
/* #include "../vrml_parser/CParseGeneral.h" */
/* #include "../scenegraph/Vector.h" */
/* #include "../vrml_parser/CFieldDecls.h" */
/* #include "../world_script/CScripts.h" */
/* #include "../vrml_parser/CParseParser.h" */
/* #include "../vrml_parser/CParseLexer.h" */
/* #include "../vrml_parser/CParse.h" */

/* #include <float.h> */

#include "../x3d_parser/Bindable.h"
/* #include "../scenegraph/Collision.h" */
/* #include "../scenegraph/quaternion.h" */
/* #include "../scenegraph/Viewer.h" */

#include "pluginUtils.h"
#include "PluginSocket.h"


#ifdef F_SETSIG
#define FSIGOK
#endif

pthread_mutex_t mylocker = PTHREAD_MUTEX_INITIALIZER;

#define LOCK_PLUGIN_COMMUNICATION pthread_mutex_lock(&mylocker);
#define UNLOCK_PLUGIN_COMMUNICATION pthread_mutex_unlock(&mylocker);

fd_set rfds;
struct timeval tv;

char return_url[FILENAME_MAX]; /* used to be local, but was returned as a pointer */
extern double TickTime;


/* prints to a log file if we are running as a plugin */
void pluginprint (const char *m, const char *p) {
	double myt;
        struct timeval mytime;
        struct timezone tz; /* unused see man gettimeofday */

	if (getenv("FREEWRL_DO_PLUGIN_PRINT") != NULL) {

        	/* Set the timestamp */
        	gettimeofday (&mytime,&tz);
		myt = (double) mytime.tv_sec + (double)mytime.tv_usec/1000000.0;
        	printf ("%f: freewrl: ",myt);
		printf(m,p);
	}

}

/* loop about waiting for the Browser to send us some stuff. */
int waitForData(int sock) {

	int retval;
	int count;
	int totalcount;

	#ifdef PLUGINSOCKETVERBOSE
	pluginprint ("waitForData, socket %d\n",sock);
	#endif

	retval = FALSE;
	count = 0;
	totalcount = 80000;

	do {
		/*
		#ifdef PLUGINSOCKETVERBOSE
		pluginprint ("waitForData on socket looping...%d\n",count);
		#endif
		*/

		tv.tv_sec = 0;
		tv.tv_usec = 100;
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);

		/* wait for the socket. We HAVE to select on "sock+1" - RTFM */
		retval = select(sock+1, &rfds, NULL, NULL, &tv);


		if (retval) {
			#ifdef PLUGINSOCKETVERBOSE
			pluginprint ("waitForData returns TRUE\n","");
			#endif

			return (TRUE);
		} else {
			count ++;
			if (count > totalcount) {
				#ifdef PLUGINSOCKETVERBOSE
				pluginprint ("waitForData, timing out\n","");
				#endif

				return (FALSE);
			}
		}
	} while (!retval);
}

void  requestPluginPrint(int to_plugin, const char *msg) {
        size_t len = 0, ulen = 0, bytes = 0;
	urlRequest request;

        request.notifyCode = 2; /* ask for print service */

        len = FILENAME_MAX * sizeof(char);
        memset(request.url, 0, len);

	ulen = strlen(msg) + 1;
	memmove(request.url, msg, ulen);

        bytes = sizeof(urlRequest);

        if (write(to_plugin, (urlRequest *) &request, bytes) < 0) {
		printf ("COULD NOT WRITE TO THE PLUGIN SOCKET!\n");
        }
}

char * requestUrlfromPlugin(int to_plugin, uintptr_t plugin_instance, const char *url) { 
	size_t len = 0, ulen = 0, bytes = 0;
	urlRequest request;
	FILE  *infile;
	int linecount;
	int linelen;
	char buf[2004];
	char encodedUrl[2000];

	LOCK_PLUGIN_COMMUNICATION

	/* encode the url - if it has funny characters (eg, spaces) asciify them 
	   in accordance to some HTML web standard */
        URLencod(encodedUrl,url,2000);

	#ifdef PLUGINSOCKETVERBOSE
	pluginprint ("NEW REQUEST\n",url);
	pluginprint ("requestURL fromPlugin, getting %s\n",url);
	pluginprint ("   ... encoded is %s\n",encodedUrl);
	#endif

	request.instance = (void *) plugin_instance;
	request.notifyCode = 0; /* get a file  */

	len = FILENAME_MAX * sizeof(char);
	memset(request.url, 0, len);
	memset(return_url, 0, len);

	ulen = strlen(encodedUrl) + 1;
	memmove(request.url, encodedUrl, ulen);

	bytes = sizeof(urlRequest);

	#ifdef PLUGINSOCKETVERBOSE
	pluginprint ("requestURL fromPlugin, step 1\n","");
	pluginprint ("sending url request to socket %d\n",to_plugin);
	#endif

	if (write(to_plugin, (urlRequest *) &request, bytes) < 0) {
		#ifdef PLUGINSOCKETVERBOSE
		pluginprint ("write failed in requestUrlfromPlugin","");
		#endif
		return NULL;
	}

	#ifdef PLUGINSOCKETVERBOSE
	pluginprint ("requestURL fromPlugin, step 2\n","");
	#endif



	/* wait around for a bit to see if this is going to pass or fail */
	if (!waitForData(to_plugin)) {
		request.notifyCode = -99; /* destroy stream */
		if (write(to_plugin, (urlRequest *) &request, bytes) < 0) {
			#ifdef PLUGINSOCKETVERBOSE
			pluginprint ("write failed in requestUrlfromPlugin","");
			#endif
			UNLOCK_PLUGIN_COMMUNICATION
			return NULL;
		}

		ConsoleMessage ("failed to find URL %s\n",url);
		UNLOCK_PLUGIN_COMMUNICATION

		return NULL;
	}

	if (read(to_plugin, (char *) return_url, len) < 0) {
		#ifdef PLUGINSOCKETVERBOSE
		pluginprint("read failed in requestUrlfromPlugin","");
		pluginprint("Testing: error from read -- returned url is %s.\n", return_url);
		#endif
		UNLOCK_PLUGIN_COMMUNICATION
		return NULL;
	}

	#ifdef PLUGINSOCKETVERBOSE
	pluginprint ("requestURL fromPlugin, returning %s\n",return_url);
	pluginprint ("REQUEST FINISHED\n",return_url);
	#endif

	/* is this a string from URLNotify? (see plugin code for this "special" string) */
	#define returnErrorString "this file is not to be found on the internet"
	if (strncmp(return_url,returnErrorString,strlen(returnErrorString)) == 0) return NULL;

	/* now, did this request return a text file with a html page indicating 404- not found? */
	infile = fopen (return_url,"r");
	if (infile == NULL) {
		#ifdef PLUGINSOCKETVERBOSE
		pluginprint ("requestUrlFromPlugin, file %s could not be opened",return_url);
		#endif
		/* hmmm - I think that this file should exist, why did it not open? */
		UNLOCK_PLUGIN_COMMUNICATION
		return NULL;
	}

	linecount = 0;
	linelen = fread (buf,1,2000,infile);
	/* pluginprint ("verify read, read in %d characters\n",linelen);*/
	while ((linelen > 0) && (linecount < 5)){
	/* 	pluginprint ("verify read, read in %d characters\n",linelen);*/

		/* did we find a "404 file not found" message? */
		/* some, all??? will eventually return a 404 html text in
		   place of whatever you requested */
		if (strstr(buf,"<TITLE>404 Not Found</TITLE>") != NULL) {
			#ifdef PLUGINSOCKETVERBOSE
			pluginprint ("found a 404 in :%s:\n",buf);
			#endif
			fclose (infile);
			UNLOCK_PLUGIN_COMMUNICATION
			return NULL;
		}
		linecount ++;
		linelen = fread (buf,1,2000,infile);
	}
	fclose (infile);


	UNLOCK_PLUGIN_COMMUNICATION

	/* we must be returning something here */
	return return_url;
}


/* tell Netscape that a new window is required (eg, Anchor
 * clicked and it is an HTML page */

void requestNewWindowfromPlugin(int sockDesc,
		   uintptr_t plugin_instance,
		   const char *url)
{
	size_t len = 0, ulen = 0, bytes = 0;
	urlRequest request;

	#ifdef PLUGINSOCKETVERBOSE
	pluginprint ("requestNewWindow fromPlugin, getting %s\n",url);
	#endif

	request.instance = (void *) plugin_instance;
	request.notifyCode = 1; /* tell plugin that we want a new window */

	len = FILENAME_MAX * sizeof(char);
	memset(request.url, 0, len);
	memset(return_url, 0, len);

	ulen = strlen(url) + 1;
	memmove(request.url, url, ulen);
	bytes = sizeof(urlRequest);

	#ifdef PLUGINSOCKETVERBOSE
	pluginprint ("requestNewWindow fromPlugin, step 1\n","");
	#endif

	if (write(sockDesc, (urlRequest *) &request, bytes) < 0) {
		#ifdef PLUGINSOCKETVERBOSE
		pluginprint ("write failed in requestUrlfromPlugin","");
		#endif
		return;
	}
}
