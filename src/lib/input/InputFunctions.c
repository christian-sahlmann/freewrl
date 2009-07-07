/*
=INSERT_TEMPLATE_HERE=

$Id: InputFunctions.c,v 1.5 2009/07/07 15:12:02 crc_canada Exp $

CProto ???

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>
#include <pthread.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../scenegraph/Vector.h"

#include "InputFunctions.h"


#define READSIZE 2048

int dirExists(const char *dir)
{
    static struct stat ss;
    
    if (stat(dir, &ss) == 0) {
        if (access(dir,X_OK) == 0) {
            return TRUE;
        } else {
            printf("Internal error: cannot access directory: %s\n", dir);
        }
    } else {
        printf("Internal error: directory does not exist: %s\n", dir);
    }
    return FALSE;
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
    if (dirExists(tmp)) {
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

        /* printf ("start of readInputString, \n\tfile: %s\n",
                        fn);
        printf ("\tBrowserFullPath: %s\n\n",BrowserFullPath);  */

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


/* get a FILE to the file name; if the file happens to have a shadow file, open the shadow file. */
FILE *openLocalFile (char *fn, char* access) {
	indexT ind;

	/* do we have to search the shadow file vector? */
	if (!shadowFiles) return fopen(fn,access);

	LOCK_SHADOW_ACCESS
	ind = vector_size (shadowFiles);
	/* printf ("openLocalFile, and we have %d entries...\n",ind); */

	/* count down from latest to first - stop at zero */
	for (ind=vector_size(shadowFiles)-1; ((int)ind) >= 0; ind--) {
		struct shadowEntry *ele = vector_get(struct shadowEntry *,shadowFiles, ind);
		/* printf ("for element %d, we have :%s: and :%s:\n",ind,ele->x3dName, ele->localName); */
		if (!strcmp(fn,ele->x3dName)) {
			/* printf ("found it!\n"); */
			UNLOCK_SHADOW_ACCESS
			return fopen(ele->localName,access);
		}
	}
	/* hmmm - did we fail to find a match? */
	/* printf ("openLocalFile, no match for :%s:\n",fn); */
	UNLOCK_SHADOW_ACCESS
	return NULL;
}


/* clean up the cache files. We can do this on cleanup on exit */
void unlinkShadowFile(char *fn) {
	indexT ind;

	/* printf ("unlinkShadowFile :%s:\n",fn); */
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

