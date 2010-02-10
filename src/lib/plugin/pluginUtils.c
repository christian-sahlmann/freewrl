/*
  $Id: pluginUtils.c,v 1.25 2010/02/10 18:19:58 sdumoulin Exp $

  FreeWRL support library.
  Plugin interaction.

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
#include <display.h>
#include <internal.h>
#include "../threads.h"

#include <libFreeWRL.h>
#include <list.h>
#include <io_files.h>
#include <resources.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../input/EAIHeaders.h"	/* for implicit declarations */

#include "../x3d_parser/Bindable.h"
#include "../scenegraph/RenderFuncs.h"

#include "pluginUtils.h"

static int checkIfX3DVRMLFile(char *fn);


/* get all system commands, and pass them through here. What we do
 * is take parameters and execl them, in specific formats, to stop
 * people (or, to try to stop) from typing malicious code. */

/* keep a list of children; if one hangs, doQuit will hang, also. */
#ifndef WIN32
#define MAXPROCESSLIST 128
pid_t childProcess[MAXPROCESSLIST];
int lastchildProcess = 0;
int childProcessListInit = FALSE;
#else
#include <process.h>
#endif

void killErrantChildren(void) {
#ifndef WIN32
	int count;
	
	for (count = 0; count < MAXPROCESSLIST; count++) {
		if (childProcess[count] != 0) {
			/* printf ("trying to kill %d\n",childProcess[count]); */
			/* http://www.opengroup.org/onlinepubs/000095399/functions/kill.html */
			kill (childProcess[count],SIGINT);
		}
	}
#endif
}

/* implement Anchor/Browser actions */

static void goToViewpoint(char *vp) {
	struct X3D_Node *localNode;
	int tableIndex;
	int flen;


	/* see if we can get a node that matches this DEF name */
	localNode = EAI_GetViewpoint(vp);
	
	/*  did we find a match with known Viewpoints?*/
	if (localNode != NULL) {
		for (flen=0; flen<totviewpointnodes;flen++) {
			if (localNode == viewpointnodes[flen]) {
				/* unbind current, and bind this one */
				send_bind_to(X3D_NODE(viewpointnodes[currboundvpno]),0);
				currboundvpno=flen;
				send_bind_to(X3D_NODE(viewpointnodes[currboundvpno]),1);

				return;
			}
		}
	}
	/* printf ("goToViewpoint - failed to match local Viewpoint\n"); */
}

static void startNewHTMLWindow(char *url) {
	char *browser;
#define LINELEN 4000
	char sysline[LINELEN];
	int testlen;

	browser = NULL;

#ifdef AQUA
	if (RUNNINGASPLUGIN) {
		/* printf ("Anchor, running as a plugin - load non-vrml file\n"); */
		requestNewWindowfromPlugin(_fw_browser_plugin, _fw_instance, url);
	} else {
#endif
	browser = freewrl_get_browser_program();
	if (!browser) {
		ConsoleMessage ("Error: no Internet browser found.");
		return;
	}
		
	/* bounds check here */
	testlen = 0;
	if (browser) testlen = strlen(browser);

	testlen += strlen(url) + 10; 
	if (testlen > LINELEN) {
		ConsoleMessage ("Anchor: combination of browser name and file name too long.");
	} else {
			
		if (browser) strcpy (sysline, browser);
		else strcpy (sysline, browser);
		strcat (sysline, " ");
		strcat (sysline, url);
		strcat (sysline, " &");
		freewrlSystem (sysline);
	}
		
	if (browser) sprintf(sysline, "%s %s &", browser, url);
	else sprintf(sysline, "open %s &",  url);
	system (sysline);
#ifdef AQUA
	}
#endif
}

/* we keep polling here, if we are loading a url...*/
static int waitingForURLtoLoad = FALSE;
static resource_item_t *res = NULL; 	/* If this res is valid, then we can replace root_res with it */

static int urlLoadingStatus() {
	/* printf ("urlLoadingStatus %s\n",resourceStatusToString(res->status)); */

	switch (res->status) {
		case ress_parsed:
			EAI_Anchor_Response(TRUE);
			waitingForURLtoLoad = FALSE;
			break;
		case ress_failed:
			ConsoleMessage ("Failed to load URL\n");
			EAI_Anchor_Response(FALSE);
			waitingForURLtoLoad = FALSE;
			break;
		default: {}
	}

	return waitingForURLtoLoad;
}



/* returns FALSE if we are DONE the action, whether or not it was successful; 
   TRUE if we want to hit this next time through the event loop */

int doBrowserAction()
{
	struct Multi_String Anchor_url;
	char *description;
	
	/* are we in the process of polling for a new X3D URL to load? */
	if (waitingForURLtoLoad) return urlLoadingStatus();

	/* is this an Anchor (thus Multi-URL call) or a single url call? */
	/* OSX frontend and now plugin for loading up a new url does:
	#ifdef wrwe
	       AnchorsAnchor = NULL;
	        FREE_IF_NZ(OSX_replace_world_from_console);
	        OSX_replace_world_from_console = STRDUP(str);
	#endif
	*/

	if (AnchorsAnchor != NULL) {
		Anchor_url = AnchorsAnchor->url;
		description = AnchorsAnchor->description->strptr;

		TRACE_MSG("doBrowserAction: description: %s\n", description);

		/* are we going to load up a new VRML/X3D world, or are we going to just go and load up a new web page ? */
		if (Anchor_url.n < 0) {
			/* printf ("have Anchor, empty URL\n"); */
			return FALSE; /* done the action, the url is just not good */
		} 

		/* printf ("ok, Anchor first url is :%s:\n",Anchor_url.p[0]->strptr); */
		if (checkIfX3DVRMLFile(Anchor_url.p[0]->strptr)) {
			printf ("this IS an X3D file...\n");
			res = resource_create_multi(&Anchor_url);

			printf("HERE1\n");
			#ifndef AQUA
			kill_oldWorld(TRUE,TRUE,__FILE__,__LINE__);
			#endif


			send_resource_to_parser(res);
			waitingForURLtoLoad = TRUE;
			return TRUE; /* keep the browser ticking along here */

		} else {

			/* ok, not a new world to load, lets see if it is a Viewpoint in current world: */
			if (Anchor_url.p[0]->strptr[0] == '#') {
				goToViewpoint (&(Anchor_url.p[0]->strptr[1]));
			} else {
				startNewHTMLWindow(Anchor_url.p[0]->strptr);
			}
		}


	} else {
		/* printf ("\nwe have a single replacement here\n"); */
		if (OSX_replace_world_from_console == NULL) {
			/* this is just a simple "clean out the old world" */
			printf("HERE2\n");
			#ifndef AQUA
			kill_oldWorld(TRUE,TRUE,__FILE__,__LINE__);
			#endif
			return FALSE;
		} else {
			
			/* we want to clean out the old world AND load a new one in */
			res = resource_create_single (OSX_replace_world_from_console);
			printf("HERE3\n");
			#ifndef AQUA
			kill_oldWorld(TRUE,TRUE,__FILE__,__LINE__);
			#endif

			send_resource_to_parser(res);
			waitingForURLtoLoad = TRUE;
			return TRUE; /* keep the browser ticking along here */
		}
	}

	return FALSE; /* we are done the action */
}

/*
 * Check to see if the file name is a geometry file.
 * return TRUE if it looks like it is, false otherwise
 *
 * This should be kept in line with the plugin register code in
 * Plugin/netscape/source/npfreewrl.c
 */

static int checkIfX3DVRMLFile(char *fn) {
	if ((strstr(fn,".wrl") > 0) ||
		(strstr(fn,".WRL") > 0) ||
		(strstr(fn,".x3d") > 0) ||
		(strstr(fn,".x3dv") > 0) ||
		(strstr(fn,".x3db") > 0) ||
		(strstr(fn,".X3DV") > 0) ||
		(strstr(fn,".X3DB") > 0) ||
		(strstr(fn,".X3D") > 0)) {
		return TRUE;
	}
	return FALSE;
}

/* we are an Anchor, and we are not running in a browser, and we are
 * trying to do an external VRML or X3D world.
 */
/* void Anchor_ReplaceWorld (char *name) */
bool Anchor_ReplaceWorld(const char *name)
{
	resource_item_t *res;

printf ("Anchor_ReplaceWorld - parsing %s\n",name);

	res = resource_create_single(name);
	res->new_root = TRUE;
	send_resource_to_parser(res);
	resource_wait(res);

	if (res->status != ress_loaded) {
		/* FIXME: destroy this new node tree */
		return FALSE;
	}
	return TRUE;
}

/* send in a 0 to 15, return a char representation */
char tohex (int mychar) {
	if (mychar <10) return (mychar + '0');
	else return (mychar - 10 + 'A');
}

/* should this character be encoded? */
int URLmustEncode(int ch) {
	if (
		   (ch == 0x20) 
		|| (ch == 0x22) 
		|| (ch == 0x3c) 
		|| (ch == 0x3e) 
		|| (ch == 0x23) 
		|| (ch == 0x25)
		|| (ch == 0x7b) 
		|| (ch == 0x7d) 
		|| (ch == 0x7c) 
		|| (ch == 0x5c) 
		|| (ch == 0x5e) 
		|| (ch == 0x7e) 
		|| (ch == 0x5b) 
		|| (ch == 0x5d) 
		|| (ch == 0x60)) return TRUE;
	return FALSE;
} 


/***************/
/* static FILE * tty = NULL; */

/* prints to a log file if we are running as a plugin */
void URLprint (const char *m, const char *p) {
#undef URLPRINTDEBUG
#ifdef URLPRINTDEBUG
	if (tty == NULL) {
		tty = fopen("/home/luigi/logURLencod", "w");
		if (tty == NULL)
			abort();
		fprintf (tty, "\nplugin restarted\n");
	}

	fprintf (tty,"%f URLprint: ",TickTime);
	fprintf(tty, m,p);
	fflush(tty);
#endif
}

/* loop about waiting for the Browser to send us some stuff. */
/* Change a string to encode spaces for getting URLS. */
void URLencod (char *dest, const char *src, int maxlen) {
	int mylen;
	int sctr;
	int destctr;
	int curchar;

#ifdef URLPRINTDEBUG
	char *orig;

	orig = dest;

	/* get the length of the source and bounds check */
	URLprint ("going to start URLencod %s\n","on a string");
	URLprint ("start, src is %s\n",src);
	/* URLprint ("maxlen is %d\n",maxlen); */
#endif

	destctr = 0; /* ensure we dont go over dest length */
	mylen = strlen(src);
	if (mylen == 0) {
		dest[0]= '\0';
		return;
	}
	
	/* encode the string */
	for (sctr = 0; sctr < mylen; sctr ++) {
		curchar = (int) *src; src++;	
		/* should we encode this one? */

                if (URLmustEncode(curchar)) { 
			*dest = '%'; dest ++;
			*dest = tohex ((curchar >> 4) & 0x0f); dest++;
			*dest = tohex (curchar & 0x0f); dest++;
			destctr +=3;
		} else {
			*dest = (char) curchar; destctr++; dest++;
		}
	


		/* bounds check */
		if (destctr > (maxlen - 5))  {
			*dest = '\0';
			return;
		}
	}
	*dest = '\0'; /* null terminate this one */
#ifdef URLPRINTDEBUG
	URLprint ("encoded string is %s\n",orig);
#endif

}

/* this is for Unix only */
#if !defined(AQUA) && !defined(WIN32)

void sendXwinToPlugin()
{
	XWindowAttributes mywin;

	/* send the window id back to the plugin parent */
	DEBUG_MSG("Executing sendXwinToPlugin...\n");

        XGetWindowAttributes(Xdpy,Xwin, &mywin);
        DEBUG_MSG("sendXwinToPlugin: sendXwin starting, mapped_state %d, IsUnmapped %d, isUnviewable %d isViewable %d\n",mywin.map_state, IsUnmapped, IsUnviewable, IsViewable);

	DEBUG_MSG("sendXwinToPlugin: sending Xwin ID back to plugin - %d bytes\n",sizeof (Xwin));

	write (_fw_pipe,&Xwin,sizeof(Xwin));
	close (_fw_pipe);

	/* wait for the plugin to change the map_state */
        XGetWindowAttributes(Xdpy,Xwin, &mywin);
	
	while (mywin.map_state == IsUnmapped) {
		usleep (100);
        	XGetWindowAttributes(Xdpy,Xwin, &mywin);
		#ifdef URLPRINTDEBUG
        	printf ("sendXwin in sleep loope, mapped_state %d, IsUnmapped %d, isUnviewable %d isViewable %d\n",mywin.map_state, IsUnmapped, IsUnviewable, IsViewable);
		#endif

	}

	#ifdef URLPRINTDEBUG
        XGetWindowAttributes(Xdpy,Xwin, &mywin);
        printf ("sendXwin at end,  mapped_state %d, IsUnmapped %d, isUnviewable %d isViewable %d\n",mywin.map_state, IsUnmapped, IsUnviewable, IsViewable);
        printf ("x %d y %d wid %d height %d\n",mywin.x,mywin.y,mywin.width,mywin.height);
	#endif

}
#endif

