/*
=INSERT_TEMPLATE_HERE=

$Id: pluginUtils.c,v 1.5 2009/02/18 13:37:50 istakenv Exp $

???

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../input/EAIheaders.h"

/* #include <float.h> */

#include "../x3d_parser/Bindable.h"

#include "pluginUtils.h"


/* get all system commands, and pass them through here. What we do
 * is take parameters and execl them, in specific formats, to stop
 * people (or, to try to stop) from typing malicious code. */

/* keep a list of children; if one hangs, doQuit will hang, also. */
#define MAXPROCESSLIST 128
pid_t childProcess[MAXPROCESSLIST];
int lastchildProcess = 0;
int childProcessListInit = FALSE;

void killErrantChildren(void) {
	int count;
	
	for (count = 0; count < MAXPROCESSLIST; count++) {
		if (childProcess[count] != 0) {
			/* printf ("trying to kill %d\n",childProcess[count]); */
			kill (childProcess[count],SIGINT);
		}
	}
}

/* FIXME: what are the possible return codes for this function ??? */
int freewrlSystem (const char *sysline) {

#define MAXEXECPARAMS 10
#define EXECBUFSIZE	2000
	char *paramline[MAXEXECPARAMS];
	char buf[EXECBUFSIZE];
	char *internbuf;
	int count;
	/* pid_t childProcess[lastchildProcess]; */
	int pidStatus;
	int waitForChild;
	int haveXmessage;


	/* make all entries in the child process list = 0 */
	if (childProcessListInit == FALSE) {
		memset(childProcess, 0, MAXPROCESSLIST);
		childProcessListInit = TRUE;
	}
	
	/* initialize the paramline... */
	memset(paramline, 0, sizeof(paramline));
		
	waitForChild = TRUE;
	haveXmessage = !strncmp(sysline, FREEWRL_MESSAGE_WRAPPER, strlen(FREEWRL_MESSAGE_WRAPPER));

	internbuf = buf;

	/* bounds check */
	if (strlen(sysline)>=EXECBUFSIZE) return FALSE;
	strcpy (buf,sysline);

	/* printf ("freewrlSystem, have %s here\n",internbuf); */
	count = 0;

	/* do we have a console message - (which is text with spaces) */
	if (haveXmessage) {
		paramline[0] = FREEWRL_MESSAGE_WRAPPER;
		paramline[1] = strchr(internbuf,' ');
		count = 2;
	} else {
		/* split the command off of internbuf, for execing. */
		while (internbuf != NULL) {
			/* printf ("freewrlSystem: looping, count is %d\n",count);  */
			paramline[count] = internbuf;
			internbuf = strchr(internbuf,' ');
			if (internbuf != NULL) {
				/* printf ("freewrlSystem: more strings here! :%s:\n",internbuf); */
				*internbuf = '\0';
				/* printf ("param %d is :%s:\n",count,paramline[count]); */
				internbuf++;
				count ++;
				if (count >= MAXEXECPARAMS) return -1; /*  never...*/
			}
		}
	}
	
	/* printf ("freewrlSystem: finished while loop, count %d\n",count); 
	
	 { int xx;
		for (xx=0; xx<MAXEXECPARAMS;xx++) {
			printf ("item %d is :%s:\n",xx,paramline[xx]);
	}} */
	
	if (haveXmessage) {
		waitForChild = FALSE;
	} else {
		/* is the last string "&"? if so, we don't need to wait around */
		if (strncmp(paramline[count],"&",strlen(paramline[count])) == 0) {
			waitForChild=FALSE;
			paramline[count] = '\0'; /*  remove the ampersand.*/
		}
	}

	if (count > 0) {
		switch (childProcess[lastchildProcess]=fork()) {
			case -1:
				perror ("fork");
				exit(1);
				break;

			case 0: 
			{
				int Xrv;
				
				/* child process */
				/* printf ("freewrlSystem: child execing, pid %d %d\n",childProcess[lastchildProcess], getpid());  */
				Xrv = execl((const char *)paramline[0],
							(const char *)paramline[0],paramline[1], paramline[2],
							paramline[3],paramline[4],paramline[5],
							paramline[6],paramline[7]);
				printf ("FreeWRL: Fatal problem execing %s\n",paramline[0]);
				perror("FreeWRL: "); 
				exit (Xrv);
			}
				break;

			default: 
			{
				/* parent process */
				/* printf ("freewrlSystem: parent waiting for child %d\n",childProcess[lastchildProcess]); */
				
				lastchildProcess++;
				if (lastchildProcess == MAXPROCESSLIST) lastchildProcess=0;
				
				/* do we have to wait around? */
				if (!waitForChild) {
					/* printf ("freewrlSystem - do not have to wait around\n"); */
					return TRUE;
				}
				waitpid (childProcess[lastchildProcess],&pidStatus,0);
				/* printf ("freewrlSystem: parent - child finished - pidStatus %d \n",
				   pidStatus);  */
				
				/* printf ("freewrlSystem: WIFEXITED is %d\n",WIFEXITED(pidStatus)); */
				
				/* if (WIFEXITED(pidStatus) == TRUE) printf ("returned ok\n"); else printf ("problem with return\n"); */
			}
		}
		return (WIFEXITED(pidStatus) == TRUE);
	} else {
		printf ("System call failed :%s:\n",sysline);
	}
	return -1; /* should we return FALSE or -1 ??? */
}

/* implement Anchor/Browser actions */

void doBrowserAction () {
	int count;

	int localNode;
	int tableIndex;

	char *filename;
	char *mypath;
	char *thisurl;
	int flen;
	int removeIt = FALSE;

#define LINELEN 2000
	char sysline[LINELEN];
	int testlen;


	struct Multi_String Anchor_url;
	/* struct Multi_String { int n; SV * *p; };*/

	Anchor_url = AnchorsAnchor->url;

	/* if (!RUNNINGASPLUGIN)
		printf ("FreeWRL::Anchor: going to \"%s\"\n",
			AnchorsAnchor->description->strptr);
	*/

	filename = (char *)MALLOC(1000);

	/* copy the parent path over */
	mypath = STRDUP(AnchorsAnchor->__parenturl->strptr);

	/* and strip off the file name, leaving any path */
	removeFilenameFromPath (mypath);

	/* printf ("Anchor, url so far is %s\n",mypath); */

	/* try the first url, up to the last */
	count = 0;
	while (count < Anchor_url.n) {
		thisurl = Anchor_url.p[count]->strptr;

		/*  is this a local Viewpoint?*/
		if (thisurl[0] == '#') {
			localNode = EAI_GetViewpoint(&thisurl[1]);
			tableIndex = -1;

			for (flen=0; flen<totviewpointnodes;flen++) {
				if (localNode == viewpointnodes[flen]) {
					 tableIndex = flen;
					 break;
				}
			}
			/*  did we find a match with known Viewpoints?*/
			if (tableIndex>=0) {
				/* unbind current, and bind this one */
				send_bind_to(X3D_NODE(viewpointnodes[currboundvpno]),0);
				currboundvpno=tableIndex;
				send_bind_to(X3D_NODE(viewpointnodes[currboundvpno]),1);
			} else {
				printf ("failed to match local Viewpoint\n");
			}

			/*  lets get outa here - jobs done.*/
			FREE_IF_NZ (filename);
			return;
		}

		/* check to make sure we don't overflow */
		if ((strlen(thisurl)+strlen(mypath)) > 900) break;


		/* put the path and the file name together */
		makeAbsoluteFileName(filename,mypath,thisurl);
		/* printf ("so, Anchor, filename %s, mypath %s, thisurl %s\n",filename, mypath, thisurl); */

		/* if this is a html page, just assume it's ok. If
		 * it is a VRML/X3D file, check to make sure it exists */

		if (!checkIfX3DVRMLFile(filename)) { break; }

		/* ok, it might be a file we load into our world. */
		if (fileExists(filename,NULL,FALSE,&removeIt)) { break; }
		count ++;
	}

	/*  did we locate that file?*/
	if (count == Anchor_url.n) {
		if (count > 0) {
			printf ("Could not locate url (last choice was %s)\n",filename);
		}
		FREE_IF_NZ (filename);

		/* if EAI was waiting for a loadURL, tell it it failed */
		EAI_Anchor_Response (FALSE);
		return;
	}
	/* printf ("we were successful at locating :%s:\n",filename); */

	/* which browser are we running under? if we are running as a*/
	/* plugin, we'll have some of this information already.*/

	if (checkIfX3DVRMLFile(filename)) {
		Anchor_ReplaceWorld (filename);
	} else {
		#ifdef AQUA
		if (RUNNINGASPLUGIN) {
		 	/* printf ("Anchor, running as a plugin - load non-vrml file\n"); */
		 	requestNewWindowfromPlugin(_fw_browser_plugin, _fw_instance, filename);
		} else {
		#endif
			/* printf ("IS NOT a vrml/x3d file\n");
			printf ("Anchor: -DBROWSER is :%s:\n",BROWSER); */

		    /* char *browser = getenv("BROWSER"); */
		    char *browser = freewrl_get_browser_program();
		    if (!browser) {
			ConsoleMessage ("Error: no Internet browser found.");
			return;
		    }

			/* bounds check here */
			if (browser) testlen = strlen(browser);
			else testlen = strlen(browser);
			testlen += strlen(filename) + 10; 
			if (testlen > LINELEN) {
				ConsoleMessage ("Anchor: combination of browser name and file name too long.");
			} else {

				if (browser) strcpy (sysline, browser);
				else strcpy (sysline, browser);
				strcat (sysline, " ");
				strcat (sysline, filename);
				strcat (sysline, " &");
				freewrlSystem (sysline);
			}

			/* bounds check here */
			if (browser) testlen = strlen(browser) + strlen(filename) + 20;
			else testlen = strlen (browser) + strlen(filename) + 20;


			if (testlen > LINELEN) {
				ConsoleMessage ("Anchor: combination of browser name and file name too long.");
			} else {
				if (browser) sprintf(sysline, "open -a %s %s &", browser, filename);
				else sprintf(sysline, "open -a %s %s &",  browser, filename);
				system (sysline);
			}
		#ifdef AQUA
		}
		#endif

	}
	FREE_IF_NZ (filename);
}



/*
 * Check to see if the file name is a geometry file.
 * return TRUE if it looks like it is, false otherwise
 *
 * This should be kept in line with the plugin register code in
 * Plugin/netscape/source/npfreewrl.c
 */

int checkIfX3DVRMLFile(char *fn) {
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
void Anchor_ReplaceWorld (char *name)
{
	int tmp;
	void *tt;
	char filename[1000];
	int removeIt = FALSE;

	/* sanity check - are we actually going to do something with a name? */
	if (name != NULL)
		if (strlen (name) > 1) {
			strcpy (filename,name);

			/* there is a good chance that this name has already been vetted from the
			   network. BUT - plugin code might pass us a networked file name for loading,
			   (eg, check out current OSX plugin; hopefully still valid) */

	                if (fileExists(filename,NULL,TRUE,&removeIt)) {
				/* kill off the old world, but keep EAI open, if it is... */
				kill_oldWorld(FALSE,TRUE,TRUE,__FILE__,__LINE__);

				inputParse(FROMURL, filename,TRUE,FALSE, 
					rootNode, offsetof (struct X3D_Group, children),&tmp,
					TRUE);
			
				tt = BrowserFullPath;
				BrowserFullPath = STRDUP(filename);
				FREE_IF_NZ(tt);
				if (removeIt) UNLINK (filename);
				EAI_Anchor_Response (TRUE);
				return;
			} else {
				ConsoleMessage ("file %s does not exist",name);
			}
		} 
	EAI_Anchor_Response (FALSE);
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
static FILE * tty = NULL;

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
#ifndef AQUA 
void sendXwinToPlugin() {
	XWindowAttributes mywin;

	/* send the window id back to the plugin parent */

	#ifdef URLPRINTDEBUG
        XGetWindowAttributes(Xdpy,Xwin, &mywin);
        printf ("sendXwin starting, mapped_state %d, IsUnmapped %d, isUnviewable %d isViewable %d\n",mywin.map_state, IsUnmapped, IsUnviewable, IsViewable);

	URLprint ("sending Xwin ID back to plugin - %d bytes\n",sizeof (Xwin));
	#endif

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

