/*
  $Id: InputFunctions.c,v 1.13 2009/11/29 18:01:35 crc_canada Exp $

  FreeWRL support library.
  Input functions (EAI, mouse, keyboard, ...).

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
#include <pthread.h>

#include <libFreeWRL.h>

#include <io_files.h>
#include <threads.h>

#include "../vrml_parser/Structs.h" 
#include "../main/headers.h"
#include "../scenegraph/Vector.h"

#include "InputFunctions.h"


#define READSIZE 2048


char * stripLocalFileName (char * origName) {
	/* remove whitespace, etc */
	while ((*origName != '\0') && (*origName <= ' ')) origName++;

        if ((strncmp(origName,"file://", strlen("file://"))== 0) || 
        (strncmp(origName,"FILE://", strlen("FILE://"))== 0)) {
		origName += strlen ("FILE://");
		return origName;
	}
	return origName;
}

char* makeFontDirectory()
{
	char *tmp;

	/* If environment variable is defined
	then it prevails */
	tmp = getenv("FREEWRL_FONTS_DIR");

	/* Get dir from configuration */
	if (!tmp) {
		tmp = FONTS_DIR;
	}

	/* Check if dir exists */
	if (do_dir_exists(tmp)) {
		/* do not return directory the string
		as it may be static, but make a copy */
		return strdup(tmp);
	}

	/* No directory found */
	return NULL;
}

/*
Simulate a command line "cp".
We do this, rather than systeming cp, 
because sometimes we have filenames with spaces, etc, etc 
and handling this with a system call is a pain.

TODO: rework that in smarter way...
*/
static void localCopy(char *outFile, char *inFile) {
	FILE *in, *out;
	char ch;

  /* strip any URNs off of the front of these file names */
  inFile = stripLocalFileName(inFile);
  outFile = stripLocalFileName(outFile);

  /* 
	ConsoleMessage ("localCopy: inFile :%s:",inFile);
	ConsoleMessage ("localCopy: outFile :%s:",outFile);
  */

  if((in=fopen(inFile, "rb")) == NULL) {
    ConsoleMessage ("FreeWRL copy: Cannot open input file.");
  }
  if((out=fopen(outFile, "wb")) == NULL) {
    ConsoleMessage ("FreeWRL copy: Cannot open output file.");
  }

  while(!feof(in)) {
    ch = getc(in);
    if(ferror(in)) {
      ConsoleMessage ("FreeWRL copy: input error.");
      clearerr(in);
      break;
    } else {
      if(!feof(in)) putc(ch, out);
      if(ferror(out)) {
        ConsoleMessage ("FreeWRL copy: output error.");
        clearerr(out);
        break;
      }
    }
  }
  fclose(in);
  fclose(out);

	return;
}

#if 0 //MBFILES
/* read a file, put it into memory. */
char * readInputString(char *fn) {
        char *buffer;
        int bufcount;
        int bufsize;
        FILE *infile;
        int justread;
        char unzippedFileName[1000];
        char sysline[1000];
        char firstbytes[20];
        int isTemp = FALSE;

        bufcount = 0;
        bufsize = 5 * READSIZE; /*  initial size*/
        buffer =(char *)MALLOC(bufsize * sizeof (char));

	/* debugging */
        /*
	ConsoleMessage ("localCopy start of readInputString, \n\tfile: %s", fn);
        ConsoleMessage ("\tBrowserFullPath: %s",BrowserFullPath);
	*/


        /* check to see if this file exists */
        if (!fileExists(fn,firstbytes,TRUE)) {
                ConsoleMessage("problem reading file '%s'",fn);
                strcpy (buffer,"\n");
                return buffer;
        }

        /* was this a network file that got copied to the local cache? */

        if (((unsigned char) firstbytes[0] == 0x1f) &&
                        ((unsigned char) firstbytes[1] == 0x8b)) {
                int len;

                /* printf ("this is a gzipped file!\n"); */
                isTemp = TRUE;
                sprintf (unzippedFileName, "%s.gz",tempnam("/tmp","freewrl_tmp"));

                /* first, move this to a .gz file */
                localCopy(unzippedFileName, fn);

                /* now, unzip it */
                /* remove the .gz from the file name - ".gz" is 3 characters */
                len = strlen(unzippedFileName); unzippedFileName[len-3] = '\0';
                sprintf (sysline,"%s %s",UNZIP,unzippedFileName);

		/* ConsoleMessage ("freewrlSystem being called on :%s:",sysline); */

                freewrlSystem (sysline);
                infile = fopen(unzippedFileName,"r");
        } else {

                /* ok, now, really read this one. */
                infile = openLocalFile(fn,"r");
        }

        if ((buffer == 0) || (infile == NULL)){
                ConsoleMessage("problem reading file '%s'",fn);
                strcpy (buffer,"\n");
                return buffer;
        }


        /* read in the file */
        do {
                justread = fread (&buffer[bufcount],1,READSIZE,infile);
                bufcount += justread;
                /* printf ("just read in %d bytes\n",justread);*/

                if ((bufsize - bufcount-10) < READSIZE) {
                        /* printf ("HAVE TO REALLOC INPUT MEMORY\n");*/
                        bufsize <<=1; 

                        buffer =(char *) REALLOC (buffer, (unsigned int) bufsize);
                }
        } while (justread>0);

        /* make sure we have a carrage return at the end - helps the parser find the end of line. */
        buffer[bufcount] = '\n'; bufcount++;
        buffer[bufcount] = '\0';
        /* printf ("finished read, buffcount %d\n string %s",bufcount,buffer); */
        fclose (infile);

        if (isTemp) UNLINK(unzippedFileName);
        return (buffer);
}
#endif

/********************************************************************************/
/*										*/
/* Shadow files - files that are on the web have a local file cache here	*/
/*										*/
/********************************************************************************/

struct shadowEntry {
	char *x3dName;
	char *localName;
};


static struct Vector *shadowFiles = NULL;

static pthread_mutex_t  shadowFileLock = PTHREAD_MUTEX_INITIALIZER;
#define LOCK_SHADOW_ACCESS                pthread_mutex_lock(&shadowFileLock);
#define UNLOCK_SHADOW_ACCESS              pthread_mutex_unlock(&shadowFileLock);


static struct shadowEntry *newShadowElement (char *x3dname, char *localname) {
	struct shadowEntry *ret = MALLOC(sizeof (struct shadowEntry));
	ASSERT (ret);

	ret->x3dName = MALLOC (sizeof (char) * strlen (x3dname) + 1); 
	ret->localName = MALLOC (sizeof (char) * strlen (localname) + 1); 
	strcpy(ret->x3dName, x3dname);
	strcpy(ret->localName, localname);
	return ret;

}

/**
 *   search_shadow_table: try to find the given filename in the shadow table.
 */
struct shadowEntry* search_shadow_table(const char *fn)
{
	indexT ind;

	if (!shadowFiles) return NULL;

	LOCK_SHADOW_ACCESS;
	ind = vector_size (shadowFiles);

	for (ind = vector_size(shadowFiles)-1; ((int)ind) >= 0; ind--) {

		struct shadowEntry *ele = vector_get(struct shadowEntry *, shadowFiles, ind);

		if ( ! strcmp(fn, ele->x3dName) ) {
			UNLOCK_SHADOW_ACCESS;
			return ele;
		}
	}

	UNLOCK_SHADOW_ACCESS;
	return NULL;
}

/**
 *   openLocalFile: to open a local file try to find it in the shadow table.
 *   If found, open the file.
 *   TODO: FIXME: what is not found ? enqueue a download ?
 */
FILE *openLocalFile(char *fn, char* access)
{
	struct shadowEntry *ele;
	
/* 	if (!shadowFiles) return fopen(fn,access); */

	ele = search_shadow_table(fn);

	if (ele) {
		return fopen(ele->localName, access);
	} else {
		return fopen(fn, access);
	}
}

/* clean up the cache files. We can do this on cleanup on exit */
void unlinkShadowFile(char *fn)
{
	struct shadowEntry *ele;

	ele = search_shadow_table(fn);
	if (ele) {
		DEBUG_MSG("unlinkShadowFile :%s:\n", fn);		
	}
}



/* this is a network file; create a new element; keep the "x3d name" and a local cache name */
void addShadowFile(char *x3dname, char *myshadowname) {
	struct shadowEntry *ele;

	LOCK_SHADOW_ACCESS
	ele = newShadowElement(x3dname,myshadowname);
	if (!shadowFiles) shadowFiles = newVector (struct shadowEntry *, 32);

	vector_pushBack(struct shadowEntry*, shadowFiles, ele);
	UNLOCK_SHADOW_ACCESS
}

char *getShadowFileNamePtr (char *fn) {
	indexT ind;

	/* do we have to search the shadow file vector? */
	if (!shadowFiles) return fn;

	LOCK_SHADOW_ACCESS

	/* count down from latest to first - stop at zero */
	for (ind=vector_size(shadowFiles)-1; ((int)ind) >= 0; ind--) {
		struct shadowEntry *ele = vector_get(struct shadowEntry *,shadowFiles, ind);
		if (!strcmp(fn,ele->x3dName)) {
			UNLOCK_SHADOW_ACCESS
			return ele->localName;
		}
	}
	/* hmmm - did we fail to find a match? */
	UNLOCK_SHADOW_ACCESS
	return fn;
}

void kill_shadowFileTable (void) {
	indexT ind;

	if (!shadowFiles) return;

	LOCK_SHADOW_ACCESS

	/* count down from latest to first - stop at zero */
	for (ind=vector_size(shadowFiles)-1; ((int)ind) >= 0; ind--) {
		struct shadowEntry *ele = vector_get(struct shadowEntry *,shadowFiles, ind);
		/* printf ("killShadowFile, have %s and %s at %d\n",ele->x3dName,ele->localName,ind); */
		if (ele->localName) {
			/* printf ("unlinking %s\n",ele->localName); */
			UNLINK(ele->localName);
		}
		FREE_IF_NZ(ele->localName);
		FREE_IF_NZ(ele->x3dName);
		FREE_IF_NZ(ele);
	}
	FREE_IF_NZ(shadowFiles);

	UNLOCK_SHADOW_ACCESS
}

