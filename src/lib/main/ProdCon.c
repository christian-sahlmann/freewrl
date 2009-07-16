/*
=INSERT_TEMPLATE_HERE=

$Id: ProdCon.c,v 1.22 2009/07/16 15:08:57 istakenv Exp $

CProto ???

*/

#include <config.h>
#include <system.h>
#include <system_threads.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/CScripts.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CParse.h"
#include "../world_script/jsUtils.h"
#include "Snapshot.h"
#include "../scenegraph/Collision.h"
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"
#include "../input/SensInterps.h"
#include "../x3d_parser/Bindable.h"
#include "../input/EAIheaders.h"

#include "../plugin/pluginUtils.h"
#include "../plugin/PluginSocket.h"

#include "ProdCon.h"

#define VRML1ERRORMSG "FreeWRL does not parse VRML Version 1; please convert to VRML 2 or later"

char* PluginPath = "/private/tmp";
int PluginLength = 12;

int _fw_browser_plugin = 0;
int _fw_pipe = 0;
uintptr_t _fw_instance = 0;

/* for keeping track of current url */
char *currentWorkingUrl = NULL;

/* bind nodes in display loop, NOT in parsing thread */
void *setViewpointBindInRender = NULL;
void *setFogBindInRender = NULL;
void *setBackgroundBindInRender = NULL;
void *setNavigationBindInRender = NULL;



int _P_LOCK_VAR;

/* thread synchronization issues */
#define PARSER_LOCKING_INIT _P_LOCK_VAR = 0
#define SEND_TO_PARSER if (_P_LOCK_VAR==0) _P_LOCK_VAR=1; else printf ("SEND_TO_PARSER = flag wrong!\n");
#define PARSER_FINISHING if (_P_LOCK_VAR==1) _P_LOCK_VAR=0; else printf ("PARSER_FINISHING - flag wrong!\n");

#define UNLOCK pthread_cond_signal(&condition); pthread_mutex_unlock(&mutex);

#define WAIT_WHILE_PARSER_BUSY  pthread_mutex_lock(&mutex); \
     while (_P_LOCK_VAR==1) { pthread_cond_wait(&condition, &mutex);}

#define WAIT_WHILE_NO_DATA pthread_mutex_lock(&mutex); \
     while (_P_LOCK_VAR==0) { pthread_cond_wait(&condition, &mutex);}

int inputFileType = IS_TYPE_UNKNOWN;
int inputFileVersion[3] = {0,0,0};


#define PARSE_STRING(input) \
	{ \
	inputFileType = determineFileType(input); \
/* printf ("PARSE STRING, ft %d, fv %d.%d.%d\n",inputFileType,inputFileVersion[0],inputFileVersion[1],inputFileVersion[2]); */ \
	switch (inputFileType) { \
	case IS_TYPE_XML_X3D: \
			if (!X3DParse (nRn, input)) { \
				ConsoleMessage ("Parse Unsuccessful"); \
			} \
			break; \
	case IS_TYPE_VRML: \
			cParse (nRn,offsetof (struct X3D_Group, children), input); \
			haveParsedCParsed = TRUE; \
			break; \
	case IS_TYPE_VRML1: \
			{ char *newData = convert1To2(input); \
			cParse (nRn,offsetof (struct X3D_Group, children), newData); \
			/* ConsoleMessage (VRML1ERRORMSG); */\
			break; }\
	case IS_TYPE_COLLADA: \
			ConsoleMessage ("Collada not supported yet"); \
			break; \
	case IS_TYPE_SKETCHUP: \
			ConsoleMessage ("Google Sketchup format not supported yet"); \
			break; \
	case IS_TYPE_KML: \
			ConsoleMessage ("KML-KMZ  format not supported yet"); \
			break; \
	default: { \
			if (global_strictParsing) { ConsoleMessage ("unknown text as input"); } else { \
			inputFileType = IS_TYPE_VRML; \
			inputFileVersion[0] = 2; /* try VRML V2 */ \
			cParse (nRn,offsetof (struct X3D_Group, children), input); \
			haveParsedCParsed = TRUE; }\
		} \
	} \
	}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

struct PSStruct {
	unsigned type;		/* what is this task? 			*/
	char *inp;		/* data for task (eg, vrml text)	*/
	void *ptr;		/* address (node) to put data		*/
	unsigned ofs;		/* offset in node for data		*/
	int zeroBind;		/* should we dispose Bindables?	 	*/
	int bind;		/* should we issue a bind? 		*/
	char *path;		/* path of parent URL			*/
	int *comp;		/* pointer to complete flag		*/

	char *fieldname;	/* pointer to a static field name	*/
	int jparamcount;	/* number of parameters for this one	*/
	struct Uni_String *sv;			/* the SV for javascript		*/

	/* for EAI */
	uintptr_t *retarr;		/* the place to put nodes		*/
	int retarrsize;		/* size of array pointed to by retarr	*/
	unsigned Etype[10];	/* EAI return values			*/

	/* for class - return a string */
	char *retstr;
};



void _inputParseThread (void);
unsigned int _pt_CreateVrml (char *tp, char *inputstring, uintptr_t *retarr);
int inputParse(unsigned type, char *inp, int bind, int returnifbusy,
			void *ptr, unsigned ofs, int *complete,
			int zeroBind);
void __pt_doInline(void);
void __pt_doStringUrl (void);
void EAI_readNewWorld(char *inputstring);

/* Bindables */
void* *fognodes = NULL;
void* *backgroundnodes = NULL;
void* *navnodes = NULL;
void* *viewpointnodes = NULL;
int totfognodes = 0;
int totbacknodes = 0;
int totnavnodes = 0;
int totviewpointnodes = 0;
int currboundvpno=0;

/* keep track of the producer thread made */
pthread_t PCthread = NULL;

/* is the inputParse thread created? */
int inputParseInitialized=FALSE;

/* is the parsing thread active? this is read-only, used as a "flag" by other tasks */
int inputThreadParsing=FALSE;

/* Initial URL loaded yet? - Robert Sim */
int URLLoaded=FALSE;

/* psp is the data structure that holds parameters for the parsing thread */
struct PSStruct psp;

static int haveParsedCParsed = FALSE; 	/* used to tell when we need to call destroyCParserData 
				   as destroyCParserData can segfault otherwise */

void initializeInputParseThread(void) {
	int iret;

	if (PCthread == NULL) {
		/* create consumer thread and set the "read only" flag indicating this */
		iret = pthread_create(&PCthread, NULL, (void *(*)(void *))&_inputParseThread, NULL);
	}
}

/* is a parser running? this is a function, because if we need to mutex lock, we
   can do all locking in this file */
int isInputThreadInitialized() {return inputParseInitialized;}

/* statusbar uses this to tell user that we are still loading */
int isinputThreadParsing() {return(inputThreadParsing);}

/* is the initial URL loaded? Robert Sim */
int isURLLoaded() {return(URLLoaded&&!inputThreadParsing);}


#define SLASHDOTDOTSLASH "/../"
void removeFilenameFromPath (char *path) {
	char *slashindex;
	char *slashDotDotSlash;

	/* and strip off the file name from the current path, leaving any path */
	slashindex = (char *) rindex(path, ((int) '/'));
	if (slashindex != NULL) {
		slashindex ++; /* leave the slash there */
		*slashindex = 0;
	} else {path[0] = 0;}
	/* printf ("removeFielnameFromPath, parenturl is %s\n",path); */

	/* are there any "/../" bits in the path? if so, lets clean them up */
	slashDotDotSlash = strstr(path, SLASHDOTDOTSLASH);
	while (slashDotDotSlash != NULL) {
		char tmpline[2000];
		/* might have something like: _levels_plus/tiles/0/../1/../1/../2/../ */
		/* find the preceeding slash: */
		*slashDotDotSlash = '\0';
		/* printf ("have slashdotdot, path now :%s:\n",path); */

		slashindex = (char *)rindex(path, ((int) '/'));
		if (slashindex != NULL) {
			
			slashindex ++;
			*slashindex = '\0';
			slashDotDotSlash += strlen(SLASHDOTDOTSLASH);
			strcpy(tmpline,path);
			/* printf ("tmpline step 1 is :%s:\n",tmpline); */
			strcat (tmpline, slashDotDotSlash);
			/* printf ("tmpline step 2 is :%s:\n",tmpline); */
			strcpy (path, tmpline);
			slashDotDotSlash = strstr(path, SLASHDOTDOTSLASH);
			/* printf ("end of loop, path :%s: slashdot %u\n",path,slashDotDotSlash); */


		}
	}
}


/* given a URL, find the first valid file, and return it */
int getValidFileFromUrl (char *filename, char *path, struct Multi_String *inurl, char *firstBytes) {
	char *thisurl;
	int count;

	/* and strip off the file name from the current path, leaving any path */
	removeFilenameFromPath(path);

	/* printf ("getValidFileFromUrl, path now :%s:\n",path);
	printf ("and, inurl.n is %d\n",inurl->n); */

	/* try the first url, up to the last, until we find a valid one */
	count = 0;
	filename[0] = '\0'; /* terminate, in case the user did not specify ANY files; error messages will tell us this */

	while (count < inurl->n) {
		thisurl = inurl->p[count]->strptr;

		/* check to make sure we don't overflow */
		if ((strlen(thisurl)+strlen(path)) > 900) return FALSE;

		/* we work in absolute filenames... */
		makeAbsoluteFileName(filename,path,thisurl);
		/* printf ("getValidFile, thread %u filename %s\n",pthread_self(),filename); */

		if (fileExists(filename,firstBytes,TRUE)) {
			return TRUE;
		}
		count ++;
	}
	return FALSE;
}

/*
 * Check to see if the file name is a local file, or a network file.
 * return TRUE if it looks like a file from the network, false if it
 * is local to this machine
 * October 2007 - Michel Briand suggested the https:// lines.
 */
int checkNetworkFile(char *fn) 
{
	/* printf ("checkNetworkFile, fn :%s:\n",fn); */
    if ((strncmp(fn,"ftp://", strlen("ftp://"))) &&
	(strncmp(fn,"FTP://", strlen("FTP://"))) &&
	(strncmp(fn,"http://", strlen("http://"))) &&
	(strncmp(fn,"HTTP://", strlen("HTTP://"))) &&
	(strncmp(fn,"https://", strlen("https://"))) &&
	(strncmp(fn,"HTTPS://", strlen("HTTPS://"))) &&
	(strncmp(fn,"file://", strlen("file://"))) &&
	(strncmp(fn,"FILE://", strlen("FILE://"))) &&
	(strncmp(fn,"urn://", strlen("urn://"))) &&
	(strncmp(fn,"URN://", strlen("URN://")))) {
	return FALSE;
    }
    return TRUE;
}


/* does this file exist on the local file system, or via the HTML Browser? */
/* WARNING! WARNING! the first parameter may be overwritten IF we are running
   within a Browser, so make sure it is large, like 1000 bytes. 	   */

/* parameter "GetIt" used as FALSE in Anchor */
#ifdef AQUA
#define outputDirector "-o"
#else
#define outputDirector "-O"
#endif

#ifdef HAVE_MKTEMP
#define freewrl_mktemp mktemp
#else
char* freewrl_mktemp()
{
    const char tmppat[] = "/tmp/freewrl_%u_%d";
    pid_t p = getpid();
    int r;
    char *tmp = malloc( strlen(tmppat) + 11 ); // 1st number 5-2 char + 2nd number 10-2 char

    srand (time (0));
    r = rand();
    sprintf(tmp, "/tmp/freewrl_%u_%d", (unsigned int)p, r);
    return tmp;
}
#endif

#ifdef HAVE_LIBCURL
# include <curl/curl.h>

static CURL *curl_h = NULL;

int with_libcurl = FALSE;

void init_curl()
{
    CURLcode c;

    if ( (c=curl_global_init(CURL_GLOBAL_ALL)) != 0 ) {
	ERROR_MSG("Curl init failed: %d\n", (int)c);
	exit(1);
    }

    ASSERT(curl_h == NULL);

    curl_h = curl_easy_init( );

    if (!curl_h) {
	ERROR_MSG("Curl easy_init failed\n");
	exit(1);
    }
}

const char* do_get_url_curl(const char *url)
{
    CURLcode success;
    char *temp;
    FILE *file;

    temp = freewrl_mktemp();
    if (!temp) {
	ERROR_MSG("Cannot create temp file (mktemp)\n");
	return NULL;	
    }

    file = fopen(temp, "w");
    if (!file) {
	free(temp);
	ERROR_MSG("Cannot create temp file (fopen)\n");
	return NULL;	
    }

    if (!curl_h) {
	init_curl();
    }

    curl_easy_setopt(curl_h, CURLOPT_URL, url);

    curl_easy_setopt(curl_h, CURLOPT_WRITEDATA, file);

    success = curl_easy_perform(curl_h); 

    if (success == 0) {
	printf("Waaou !\n");
	fclose(file);
	return temp;
    } else {
	free(temp);
	fclose(file);
	ERROR_MSG("Download failed for url %s (%d)\n", url, (int) success);
	return NULL;
    }
}

#endif

/* return the temp file where we got the contents of the URL requested */
const char* do_get_url(const char *filename)
{
    char *temp, *wgetcmd;

// move this to another place (where we check wget options)
#define WGET_OPTIONS "--no-check-certificate"

    // create temp filename
    temp = freewrl_mktemp();
    if (!temp) return NULL;

    // create wget command line
    wgetcmd = malloc( strlen(WGET) +
	                    strlen(WGET_OPTIONS) + 
	                    strlen(filename) +
                            strlen(temp) + 6);
    sprintf(wgetcmd, "%s %s %s -O %s",
	    WGET, WGET_OPTIONS, filename, temp);

    // call wget
    if (freewrlSystem(wgetcmd) < 0) {
	ERROR_MSG("Error in wget (%s)\n", wgetcmd);
    }
    free(wgetcmd);

    return temp;
}

int do_file_exists(const char *filename)
{
    if (access(filename, R_OK) == 0) {
	return TRUE;
    }
    return FALSE;
}

int fileExists(char *fname, char *firstBytes, int GetIt) {
	FILE *fp;
	int ok;

	char tempname[1000];
	char sysline[1000];

	int hasLocalShadowFile = FALSE;

	#ifdef VERBOSE
	printf ("fileExists: checking for filename here %s\n",fname);
	#endif

	if (checkNetworkFile(fname)) {
		/* if we are running as a plugin, ask the HTML browser for ALL files, as it'll know proxies, etc. */
		if (RUNNINGASPLUGIN) {
			/* printf ("fileExists, runningasplugin\n"); */

			/* are we running as a plugin? If so, ask the HTML browser to get the file, and place
			   it in the local cache for ANY file, except for the main "url" */
			char *retName;
			/* printf ("requesting URL from plugin...\n");  */

			retName = NULL;
			retName = requestUrlfromPlugin(_fw_browser_plugin, _fw_instance, fname);

			/* check for timeout; if not found, return false */
			if (!retName) return (FALSE);
			/* printf ("requesting URL - retname is %s\n",retName); */

			strcpy (tempname,retName);
			hasLocalShadowFile = TRUE;
		} else {
			/*  Is this an Anchor? if so, lets just assume we can*/
			/*  get it*/
			if (!GetIt) {
				/* printf ("Assuming Anchor mode, returning TRUE\n");*/
				return (TRUE);
			}

#ifdef HAVE_LIBCURL

			if (with_libcurl) {

			    char *tmp;
			    tmp = do_get_url_curl(fname);
			    if (tmp) {
				strcpy(tempname, tmp);
				free(tmp);
				hasLocalShadowFile = TRUE;
			    }

			} else {

#endif
			sprintf (tempname, "%s",tempnam("/tmp","freewrl_tmp"));
	
			/* string length checking */
			if ((strlen(WGET)+strlen(fname)+strlen(tempname)) < (1000-20)) {
			    /* hmmm - if this is a https:// line, we can try the "--no-check-certificate" and cross our
			       fingers - suggested by Michel Briand */

			    if ((strncmp(fname,"https://",strlen("https://")) == 0) ||
			    	(strncmp(fname,"HTTPS://",strlen("HTTPS://")) == 0)) 
			    sprintf (sysline,"%s --no-check-certificate %s -O %s",WGET,fname,tempname);
			    else sprintf (sysline,"%s %s %s %s",WGET,fname,outputDirector, tempname);

			    /*printf ("\nFreeWRL will try to use wget to get %s in thread %d\n",fname,pthread_self());*/
			    printf ("\nFreeWRL will try to use %s to get %s\n",WGET,fname);

			    freewrlSystem (sysline);
		   	    hasLocalShadowFile = TRUE;
			} else {
			    printf ("Internal FreeWRL problem - strings too long for wget\n");
			}

#ifdef HAVE_LIBCURL
			}
#endif

		}
	}

	if (hasLocalShadowFile) {
		addShadowFile(fname,tempname);
	} 
	fp= openLocalFile (fname,"r");

	ok = (fp != NULL);

	/* try reading the first 4 bytes into the firstBytes array */
	if (ok) {
		if (firstBytes != NULL) {
		  if (fread(firstBytes,1,4,fp)!=4) {
			ConsoleMessage ("file %s exists, but has a length < 4; can not determine type from first bytes\n",fname);
			/* a file with less than 4 bytes in it. fill in the firstBytes with "something" */
			firstBytes[0] = 0;
			firstBytes[1] = 0;
			firstBytes[2] = 0;
			firstBytes[3] = 0;
		  }  
		}
		fclose (fp);
	} else {
	    ERROR_MSG("Error reading temp file: %s (got from URL)\n", fname);
	}
	return (ok);
}


/* filename is MALLOC'd, combine pspath and thisurl to make an
   absolute file name */
void makeAbsoluteFileName(char *filename, char *pspath,char *thisurl)
{
    char *end;

    /* remove whitespace from beginning and ends of pspath and thisurl */
    while ((*pspath <= ' ') && (*pspath != '\0')) pspath++;
    while ((*thisurl <= ' ') && (*thisurl != '\0')) thisurl++;

    if (*pspath != '\0') {
        end = strchr(pspath,'\0'); 
        while ((*end <= ' ') && (end != pspath)) end--; end++; *end = '\0';
    }

    if (*thisurl != '\0') {
        end = strchr(thisurl,'\0'); 
        while ((*end <= ' ') && (end != thisurl)) end--; end++; *end = '\0';
    }
    /* printf ("makeAbs from:\n\t:%s:\n\t:%s:\n", pspath, thisurl);  */

    /* does this name start off with a ftp, http, or a "/"? */
    if ((!checkNetworkFile(thisurl)) && (strncmp(thisurl,"/",strlen("/"))!=0)) {
        /* printf ("copying psppath over for %s\n",thisurl); */
        strcpy (filename,pspath);
        /* do we actually have anything here? */
        if (strlen(pspath) > 0) {
            if (pspath[strlen(pspath)-1] != '/')
                strcat (filename,"/");
        }

        /* does this "thisurl" start with file:, as in "freewrl file:test.wrl" ? */
        if ((strncmp(thisurl,"file:",strlen("file:"))==0) || 
            (strncmp(thisurl,"FILE:",strlen("FILE:"))==0)) {
            /* printf ("stripping file off of start\n"); */
            thisurl += strlen ("file:");

            /* now, is this a relative or absolute filename? */
            if (strncmp(thisurl,"/",strlen("/")) !=0) {
                /* printf ("we have a leading slash after removing file: from name\n");
                   printf ("makeAbsolute, going to copy %s to %s\n",thisurl, filename); */
                strcat(filename,thisurl);
                        
            } else {
                /* printf ("we have no leading slash after removing file: from name\n"); */
                strcpy (filename,thisurl);
            }   
                        
        } else {
            /* printf ("makeAbsolute, going to copy %s to %s\n",thisurl, filename); */
            strcat(filename,thisurl);

        }


    } else {
        strcpy (filename,thisurl);
    }

    /* and, return in the ptr filename, the filename created... */
    /* printf ("makeAbsoluteFileName, just made :%s:\n",filename); */
}


/************************************************************************/
/*									*/
/* keep track of the current url for parsing/textures			*/
/*									*/
/************************************************************************/

void pushInputURL(char *url) {

	FREE_IF_NZ(currentWorkingUrl);
	currentWorkingUrl = STRDUP(url);
	/* printf ("currenturl is %s\n",currentWorkingUrl); */
}

char *getInputURL() {
	return currentWorkingUrl;
}


/************************************************************************/
/*									*/
/* check the first few lines to see if this is an XMLified file		*/
/*									*/
/************************************************************************/

static int determineFileType(char *buffer) {
	char *rv;
	int count;
	int foundStart = FALSE;

	for (count = 0; count < 3; count ++) inputFileVersion[count] = 0;

	/* is this an XML file? */
	if (strncmp(buffer,"<?xml version",12) == 0){
		rv = buffer;	

		/* skip past the header; we will look for lines like: 
			<?xml version="1.0" encoding="UTF-8"?>
			<!DOCTYPE X3D PUBLIC "ISO//Web3D//DTD X3D 3.0//EN"   "http://www.web3d.org/specifications/x3d-3.0.dtd">
			<X3D
		*/
		rv++;
		while (!foundStart) {
			while ((*rv != '<') && (*rv != '\0')) rv++;
			if (*rv == '<') {
				rv++;
				if (*rv != '!') foundStart = TRUE;
			} else if (*rv == '\0') foundStart = TRUE;	
		}
		/* printf ("after foundStart, we have:%s:\n",rv); */
		if (strncmp(rv,"X3D",3) == 0) {
			/* the full version number will be found by the parser */
			inputFileVersion[0] = 3;
			return IS_TYPE_XML_X3D;
		}
		if (strncmp(rv,"COLLADA",7) == 0) {
			return IS_TYPE_COLLADA;
		}
		if (strncmp(rv,"kml",3) == 0) {
			return IS_TYPE_KML;
		}

	} else {
		if (strncmp(buffer,"#VRML V2.0 utf8",15) == 0) {
			inputFileVersion[0] = 2;
			return IS_TYPE_VRML;
		}

		if (strncmp (buffer, "#X3D",4) == 0) {
			inputFileVersion[0] = 3;
			/* ok, have X3D here, what version? */

			if (strncmp (buffer,"#X3D V3.0 utf8",14) == 0) {
				return IS_TYPE_VRML;
			}
			if (strncmp (buffer,"#X3D V3.1 utf8",14) == 0) {
				inputFileVersion[1] = 1;
				return IS_TYPE_VRML;
			}
			if (strncmp (buffer,"#X3D V3.2 utf8",14) == 0) {
				inputFileVersion[1] = 2;
				return IS_TYPE_VRML;
			}
			if (strncmp (buffer,"#X3D V3.3 utf8",14) == 0) {
				inputFileVersion[1] = 3;
				return IS_TYPE_VRML;
			}
			if (strncmp (buffer,"#X3D V3.4 utf8",14) == 0) {
				inputFileVersion[1] = 4;
				return IS_TYPE_VRML;
			}
			/* if we fall off the end, we just assume X3D 3.0 */
		}
		
		/* VRML V1? */
		if (strncmp(buffer,"#VRML V1.0 asc",10) == 0) {
			return IS_TYPE_VRML1;
		}
	}

	return IS_TYPE_UNKNOWN;

}

/************************************************************************/
/*									*/
/* THE FOLLOWING ROUTINES INTERFACE TO THE PARSER THREAD			*/
/*									*/
/************************************************************************/

/* Inlines... Multi_URLs, load only when available, etc, etc */
void loadInline(struct X3D_Inline *node) {
	/* first, are we busy? */
	if (inputThreadParsing) return;

	inputParse(INLINE,(char *)node, FALSE, FALSE,
		(void *) node,
		offsetof (struct X3D_Inline, __children),
		&node->__loadstatus,FALSE);
}


/* interface for telling the parser side to forget about everything...  */
void EAI_killBindables (void) {
	int complete;

	WAIT_WHILE_PARSER_BUSY;
	complete=0;
	psp.comp = &complete;
	psp.type = ZEROBINDABLES;
	psp.retarr = NULL;
	psp.ofs = 0;
	psp.ptr = NULL;
	psp.path = NULL;
	psp.zeroBind = FALSE;
	psp.bind = FALSE; /* should we issue a set_bind? */
	psp.inp = NULL;
	psp.fieldname = NULL;

	/* send data to a parser */
	SEND_TO_PARSER;
	UNLOCK;

	/* wait for data */
	WAIT_WHILE_PARSER_BUSY;

	/* grab data */
	UNLOCK;

	/* and, reset our stack pointers */
	background_tos = INT_ID_UNDEFINED;
	fog_tos = INT_ID_UNDEFINED;
	navi_tos = INT_ID_UNDEFINED;
	viewpoint_tos = INT_ID_UNDEFINED;
}

/* interface for creating VRML for EAI */
int EAI_CreateVrml(const char *tp, const char *inputstring, uintptr_t *retarr, int retarrsize) {
	int complete;
	int retval;
	UNUSED(tp);

	/* tell the SAI that this is a VRML file, in case it cares later on (check SAI spec) */
	currentFileVersion = 3;

	WAIT_WHILE_PARSER_BUSY;

	if (strncmp(tp,"URL",2) ==  0) {
			psp.type= FROMURL;
	} else if (strncmp(tp,"String",5) == 0) {
		psp.type = FROMSTRING;
	} else if (strncmp(tp,"CREATEPROTO",10) == 0) {
		psp.type = FROMCREATEPROTO;
	} else if (strncmp(tp,"CREATENODE",10) == 0) {
		psp.type = FROMCREATENODE;
	} else {
		printf ("EAI_CreateVrml - invalid input %s\n",tp);
		return 0;
	}
	 
	complete = 0; /* make sure we wait for completion */
	psp.comp = &complete;
	psp.ptr = (unsigned)NULL;
	psp.ofs = (unsigned)NULL;
	psp.path = NULL;
	psp.zeroBind = FALSE;
	psp.bind = FALSE; /* should we issue a set_bind? */
	psp.retarr = retarr;
	psp.retarrsize = retarrsize;
	/* copy over the command */
	psp.inp = (char *)MALLOC (strlen(inputstring)+2);
	memcpy (psp.inp,inputstring,strlen(inputstring)+1);

	/* send data to a parser */
	SEND_TO_PARSER;
	UNLOCK;

	/* wait for data */
	WAIT_WHILE_PARSER_BUSY;

	/* grab data */
	retval = psp.retarrsize;

	UNLOCK;
	return (retval);
}

/* interface for replacing worlds from EAI */
void EAI_readNewWorld(char *inputstring) {
    int complete;

	WAIT_WHILE_PARSER_BUSY;
    complete=0;
    psp.comp = &complete;
    psp.type = FROMURL;
	psp.retarr = NULL;
    psp.ptr  = rootNode;
    psp.ofs  = offsetof(struct X3D_Group, children);
    psp.path = NULL;
    psp.zeroBind = FALSE;
    psp.bind = TRUE; /* should we issue a set_bind? */
    /* copy over the command */
    psp.inp  = (char *)MALLOC (strlen(inputstring)+2);
    memcpy (psp.inp,inputstring,strlen(inputstring)+1);

	/* send data to a parser */
	SEND_TO_PARSER;
	UNLOCK;

	/* wait for data */
	WAIT_WHILE_PARSER_BUSY;

	UNLOCK;
}

/****************************************************************************/
int inputParse(unsigned type, char *inp, int bind, int returnifbusy,
			void *ptr, unsigned ofs,int *complete,
			int zeroBind) {

	/* do we want to return if the parsing thread is busy, or do
	   we want to wait? */
	/* printf ("start of PerlParse, thread %d\n",pthread_self()); */
	if (returnifbusy) {
		/* printf ("inputParse, returnifbusy, inputThreadParsing %d\n",inputThreadParsing);*/
		if (inputThreadParsing) return (FALSE);
	}

	WAIT_WHILE_PARSER_BUSY;

	/* printf ("inputParse, past WAIT_WHILE_PARSER_BUSY in %d\n",pthread_self()); */

	/* copy the data over; MALLOC and copy input string */
	psp.comp = complete;
	psp.type = type;
	psp.retarr = NULL;
	psp.ptr = ptr;
	psp.ofs = ofs;
	psp.path = NULL;
	psp.bind = bind; /* should we issue a set_bind? */
	psp.zeroBind = zeroBind; /* should we zero bindables? */

	psp.inp = (char *)MALLOC (strlen(inp)+2);
	memcpy (psp.inp,inp,strlen(inp)+1);

	/* send data to a parser */
	SEND_TO_PARSER;
	UNLOCK;

	/* printf ("inputParse, waiting for data \n"); */

	/* wait for data */
	WAIT_WHILE_PARSER_BUSY;
	/* grab data */

	UNLOCK;
	return (TRUE);
}

/***********************************************************************************/

void _inputParseThread(void) {
	/* printf ("inputParseThread is %u\n",pthread_self()); */

	PARSER_LOCKING_INIT;

	inputParseInitialized = TRUE;

	viewer_default();

	/* now, loop here forever, waiting for instructions and obeying them */
	for (;;) {
		/* printf ("thread %u waiting for data\n",pthread_self()); */
		WAIT_WHILE_NO_DATA;

		inputThreadParsing=TRUE;

		/* have to handle these types of commands:
			FROMSTRING 	create vrml from string
			FROMURL		create vrml from url
			INLINE		convert an inline into code, and load it.
			CALLMETHOD	Javascript...
			EAIGETNODE      EAI getNode
			EAIGETVIEWPOINT get a Viewpoint CNode
			EAIGETTYPE	EAI getType
			EAIGETVALUE	EAI getValue - in a string.
			EAIROUTE	EAI add/delete route
			ZEROBINDABLES	tell the front end to just forget about DEFS, etc 
			SAICOMMAND	general command, with string param to parser returns an int */

		if (psp.type == INLINE) {
		/* is this a INLINE? If it is, try to load one of the URLs. */
			__pt_doInline();
		}

		switch (psp.type) {

		case FROMSTRING:
		case FROMCREATENODE:
		case FROMCREATEPROTO:
		case FROMURL:	{
			/* is this a Create from URL or string, or a successful INLINE? */
			__pt_doStringUrl();
			break;
			}

		case INLINE: {
			/* this should be changed to a FROMURL before here  - check */
			printf ("Inline unsuccessful\n");
			break;
			}

		case ZEROBINDABLES: 
			/* for the VRML parser */
			if (haveParsedCParsed) {
				if (globalParser != NULL) {
					destroyCParserData(globalParser);
					globalParser = NULL;
				}
				haveParsedCParsed = FALSE;
			}

			/* for the X3D Parser */
			kill_X3DDefs();
			break;

		default: {
			printf ("produceTask - invalid type!\n");
			}
		}

		/* finished this loop, free data */
		FREE_IF_NZ (psp.inp);
		FREE_IF_NZ (psp.path);

		*psp.comp = 1;
		URLLoaded=TRUE;
		inputThreadParsing=FALSE;
		PARSER_FINISHING;
		UNLOCK;
	}
}

/* for ReplaceWorld (or, just, on start up) forget about previous bindables */

void kill_bindables (void) {
	totfognodes=0;
	totbacknodes=0;
	totnavnodes=0;
	totviewpointnodes=0;
	currboundvpno=0;
	FREE_IF_NZ(fognodes);
	FREE_IF_NZ(backgroundnodes);
	FREE_IF_NZ(navnodes);
	FREE_IF_NZ(viewpointnodes);
}


void registerBindable (struct X3D_Node *node) {

	/* printf ("registerBindable, on node %d %s\n",node,stringNodeType(node->_nodeType));  */
	switch (node->_nodeType) {
		case NODE_Viewpoint:
		case NODE_GeoViewpoint:
			viewpointnodes = REALLOC (viewpointnodes, (sizeof(void *)*(totviewpointnodes+1)));
			viewpointnodes[totviewpointnodes] = node;
			totviewpointnodes ++;
			break;
		case NODE_Background:
		case NODE_TextureBackground:
			backgroundnodes = REALLOC (backgroundnodes, (sizeof(void *)*(totbacknodes+1)));
			backgroundnodes[totbacknodes] = node;
			totbacknodes ++;
			break;
		case NODE_NavigationInfo:
			navnodes = REALLOC (navnodes, (sizeof(void *)*(totnavnodes+1)));
			navnodes[totnavnodes] = node;
			totnavnodes ++;
			break;
		case NODE_Fog:
			fognodes = REALLOC (fognodes, (sizeof(void *)*(totfognodes+1)));
			fognodes[totfognodes] = node;
			totfognodes ++;
			break;
		default: {
			/* do nothing with this node */
			/* printf ("got a registerBind on a node of type %s - ignoring\n",
					stringNodeType(node->_nodeType));
			*/
			return;
		}                                                

	}
}

/****************************************************************************
 *
 * General load/create routines
 *
 ****************************************************************************/




/*************************NORMAL ROUTINES***************************/


/* Shutter glasses, stereo mode configure  Mufti@rus*/
float eyedist = 0.06;
float screendist = 0.8;

void setEyeDist (const char *optArg) {
	int i;
	i= sscanf(optArg,"%f",&eyedist);
	if (i==0) printf ("warning, command line eyedist parameter incorrect - was %s\n",optArg);
}

void setScreenDist (const char *optArg) {
	int i;
	i= sscanf(optArg,"%f",&screendist);
	if (i==0) printf ("warning, command line screendist parameter incorrect - was %s\n",optArg);
}
/* end of Shutter glasses, stereo mode configure */


/* handle an INLINE - should make it into a CreateVRMLfromURL type command */
void __pt_doInline() {
	char *filename;
	struct Multi_String *inurl;
	struct X3D_Inline *inl;
	inl = (struct X3D_Inline *)psp.ptr;
	inurl = &(inl->url);
	filename = (char *)MALLOC(1000);
	filename[0] = '\0';

	/* lets make up the path and save it, and make it the global path */
	psp.path = STRDUP(inl->__parenturl->strptr);

	/* printf ("doInline, checking for file from path %s\n",psp.path); */

	if (getValidFileFromUrl (filename, psp.path, inurl,NULL)) {
		/* were we successful at locating one of these? if so, make it into a FROMURL */
		/* printf ("doInline, we were successful at locating %s\n",filename);  */
		psp.type=FROMURL;
	} else {
		/* printf ("doInline, NOT successful at locating %s\n",filename);  */
		ConsoleMessage ("Could Not Locate Inline URL %s\n",filename);
	}
	psp.inp = filename; /* will be freed later */

	/* printf ("doinline, psp.inp = %s\n",psp.inp);
	printf ("inlining %s\n",filename);  */
}

/* this is a CreateVrmlFrom URL or STRING command */
void __pt_doStringUrl () {
	int count;
	int retval;
        int i;

	/* for cParser */
        char *buffer = NULL;
	char *ctmp = NULL;
	struct X3D_Group *nRn;

	if (psp.zeroBind) {
		if (haveParsedCParsed) {
			if (globalParser != NULL) {
				destroyCParserData(globalParser);
				globalParser = NULL;
			}
			kill_bindables();
		}
		psp.zeroBind = FALSE;
	}

	if (psp.type==FROMSTRING) {

		/* check and convert to VRML... */
		nRn = (struct X3D_Group *) createNewX3DNode(NODE_Group);
		PARSE_STRING(psp.inp);

	} else if (psp.type==FROMURL) {

		/* get the input */
		pushInputURL (psp.inp);
		buffer = readInputString(psp.inp);

		/* printf ("data is %s\n",buffer); */

		/* get the data from wherever we were originally told to find it */
		nRn = (struct X3D_Group *) createNewX3DNode(NODE_Group);
		PARSE_STRING(buffer);
		FREE_IF_NZ(buffer); 
		FREE_IF_NZ(ctmp);
	} else if (psp.type==FROMCREATENODE) {
		nRn = (struct X3D_Group *) createNewX3DNode(NODE_Group);
		PARSE_STRING(psp.inp);
	} else {
		nRn = (struct X3D_Group *) createNewX3DNode(NODE_Group);
		/* this will be a proto expansion, because otherwise the EAI code
		   would have gotten this before here */
		/* lets try this - same as FROMSTRING above... */
		/* look to see if this is X3D */
		PARSE_STRING(psp.inp);
	}
	

	/* set bindables, if required */
	if (psp.bind) {
	        if (totfognodes != 0) { 
		   for (i=0; i < totfognodes; ++i) send_bind_to(X3D_NODE(fognodes[i]), 0); /* Initialize binding info */
		   setFogBindInRender = fognodes[0];
		}
		if (totbacknodes != 0) {
                   for (i=0; i < totbacknodes; ++i) send_bind_to(X3D_NODE(backgroundnodes[i]), 0);  /* Initialize binding info */
		   setBackgroundBindInRender = backgroundnodes[0];
		}
		if (totnavnodes != 0) {
                   for (i=0; i < totnavnodes; ++i) send_bind_to(X3D_NODE(navnodes[i]), 0);  /* Initialize binding info */
		   setNavigationBindInRender = navnodes[0];
		}
		if (totviewpointnodes != 0) {
                   for (i=0; i < totviewpointnodes; ++i) send_bind_to(X3D_NODE(viewpointnodes[i]), 0);  /* Initialize binding info */
		   setViewpointBindInRender = viewpointnodes[0];
		}
	}

	/* did the caller want these values returned? */
	if (psp.retarr != NULL) {
		int totret = 0;

		for (count=0; count < nRn->children.n; count++) {
			/* only return the non-null children */
			if (nRn->children.p[count] != NULL) {
				psp.retarr[totret] = 0; /* the "perl" node number */
				totret++;
				psp.retarr[totret] = ((uintptr_t) 
					nRn->children.p[count]); /* the Node Pointer */
				totret++;
			}
		}
		psp.retarrsize = totret; /* remember, the old "perl node number" */
	}

      	/* now that we have the VRML/X3D file, load it into the scene. */
	if (psp.ptr != NULL) {
		/* add the new nodes to wherever the caller wanted */
		AddRemoveChildren(psp.ptr, psp.ptr+psp.ofs, (uintptr_t*)nRn->children.p,nRn->children.n,1,__FILE__,__LINE__);

		/* and, remove them from this nRn node, so that they are not multi-parented */
		AddRemoveChildren(X3D_NODE(nRn), (struct Multi_Node *)((char *)nRn + offsetof (struct X3D_Group, children)), (uintptr_t *)nRn->children.p,nRn->children.n,2,__FILE__,__LINE__);
	}


	retval = 0;
	count = 0;
}

