/*
=INSERT_TEMPLATE_HERE=

$Id: PluginSocket.c,v 1.17 2011/06/03 20:39:00 dug9 Exp $

Common functions used by Mozilla and Netscape plugins...(maybe PluginGlue too?)

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
#include <system_net.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../x3d_parser/Bindable.h"

#include "pluginUtils.h"
#include "PluginSocket.h"


#ifdef F_SETSIG
#define FSIGOK
#endif

//pthread_mutex_t mylocker = PTHREAD_MUTEX_INITIALIZER;

#define LOCK_PLUGIN_COMMUNICATION pthread_mutex_lock(&p->mylocker);
#define UNLOCK_PLUGIN_COMMUNICATION pthread_mutex_unlock(&p->mylocker);

//fd_set rfds;
//struct timeval tv;
//char return_url[FILENAME_MAX]; /* used to be local, but was returned as a pointer */

typedef struct pPluginSocket{
	pthread_mutex_t mylocker;// = PTHREAD_MUTEX_INITIALIZER;
	fd_set rfds;
	struct timeval tv;
	char return_url[FILENAME_MAX]; /* used to be local, but was returned as a pointer */

}* ppPluginSocket;
void *PluginSocket_constructor(){
	void *v = malloc(sizeof(struct pPluginSocket));
	memset(v,0,sizeof(struct pPluginSocket));
	return v;
}
void PluginSocket_init(struct tPluginSocket *t){
	//public
	//private
	t->prv = PluginSocket_constructor();
	{
		ppPluginSocket p = (ppPluginSocket)t->prv;
		pthread_mutex_init(&(p->mylocker), NULL);
		//p->rfds;
		//p->tv;
		//p->return_url[FILENAME_MAX]; /* used to be local, but was returned as a pointer */
	}
}


//extern double TickTime;
double TickTime();

/* Doug Sandens windows function; lets make it static here for non-windows */
#if defined(_MSC_VER)
#else
#ifdef PLUGINSOCKETVERBOSE
//static struct timeval mytime;

static double Time1970sec(void) {
		struct timeval mytime;
        gettimeofday(&mytime, NULL);
        return (double) mytime.tv_sec + (double)mytime.tv_usec/1000000.0;
}
#endif
#endif

#ifdef PLUGINSOCKETVERBOSE
/* prints to a log file if we are running as a plugin */
static void pluginprint (const char *m, const char *p)
{
	double myt;
	if (gglobal()->internalc.global_plugin_print) {
        	/* Set the timestamp */
		myt = Time1970sec();
        	printf ("%f: freewrl: ",myt);
		printf(m,p);
	}

}
#endif

/* loop about waiting for the Browser to send us some stuff. */
int waitForData(int sock) {

	int retval;
	int count;
	int totalcount;
	ppPluginSocket p = (ppPluginSocket)gglobal()->PluginSocket.prv;

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

		p->tv.tv_sec = 0;
		p->tv.tv_usec = 100;
		FD_ZERO(&p->rfds);
		FD_SET(sock, &p->rfds);

		/* wait for the socket. We HAVE to select on "sock+1" - RTFM */
		retval = select(sock+1, &p->rfds, NULL, NULL, &p->tv);


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
	return 0 ;
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
	ppPluginSocket p = (ppPluginSocket)gglobal()->PluginSocket.prv;

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
	memset(p->return_url, 0, len);

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

	if (read(to_plugin, (char *) p->return_url, len) < 0) {
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
	if (strncmp(p->return_url,returnErrorString,strlen(returnErrorString)) == 0) return NULL;

	/* now, did this request return a text file with a html page indicating 404- not found? */
	infile = fopen (p->return_url,"r");
	if (infile == NULL) {
		#ifdef PLUGINSOCKETVERBOSE
		pluginprint ("requestUrlFromPlugin, file %s could not be opened",return_url);
		#endif
		/* hmmm - I think that this file should exist, why did it not open? */
		UNLOCK_PLUGIN_COMMUNICATION
		return NULL;
	}

	linecount = 0;
	linelen = (int) fread (buf,1,2000,infile);
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
		linelen = (int) fread (buf,1,2000,infile);
	}
	fclose (infile);


	UNLOCK_PLUGIN_COMMUNICATION

	/* we must be returning something here */
	return p->return_url;
}


/* tell Netscape that a new window is required (eg, Anchor
 * clicked and it is an HTML page */

void requestNewWindowfromPlugin(int sockDesc,
		   uintptr_t plugin_instance,
		   const char *url)
{
	size_t len = 0, ulen = 0, bytes = 0;
	urlRequest request;
	ppPluginSocket p = (ppPluginSocket)gglobal()->PluginSocket.prv;

	#ifdef PLUGINSOCKETVERBOSE
	pluginprint ("requestNewWindow fromPlugin, getting %s\n",url);
	#endif

	request.instance = (void *) plugin_instance;
	request.notifyCode = 1; /* tell plugin that we want a new window */

	len = FILENAME_MAX * sizeof(char);
	memset(request.url, 0, len);
	memset(p->return_url, 0, len);

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
