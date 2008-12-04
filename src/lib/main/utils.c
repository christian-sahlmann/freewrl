/*
=INSERT_TEMPLATE_HERE=

$Id: utils.c,v 1.3 2008/12/04 05:59:52 couannette Exp $

General utility functions.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h"
#include "headers.h"


char *BrowserFullPath = NULL;
char *BrowserName = "FreeWRL VRML/X3D Browser";

const char* freex3d_get_browser_program()
{
    char *tmp;

    /*
      1. Check BROWSER environment variable
      2. Use configuration value BROWSER
    */

    tmp = getenv("BROWSER");
    if (!tmp) {
	tmp = BROWSER;
    }
    return tmp;
}

/**
 * This code get compiled only when debugging is enabled
 */

#if defined(_DEBUG)

#define FREETABLE(a,file,line) mcount=0; \
	while ((mcount<(MAXMALLOCSTOKEEP-1)) && (mcheck[mcount]!=a)) mcount++; \
		if (mcheck[mcount]!=a) printf ("freewrlFree - did not find %x at %s:%d\n",a,file,line); \
		else { \
			/* printf ("found %d in mcheck table\n"); */ \
			mcheck[mcount] = NULL; \
			mlineno[mcount] = 0; \
			if (mplace[mcount]!=NULL) free(mplace[mcount]); \
			mplace[mcount]=NULL; \
		} 

#define RESERVETABLE(a,file,line) mcount=0; \
	while ((mcount<(MAXMALLOCSTOKEEP-1)) && (mcheck[mcount]!=NULL)) mcount++; \
		if (mcheck[mcount]!=NULL) printf ("freewrlMalloc - out of malloc check store\n");\
		else {\
			mcheck[mcount] = a;\
			mlineno[mcount] = line;\
			mplace[mcount] = strdup(file);\
		}

#define MAXMALLOCSTOKEEP 100000
static int mcheckinit = FALSE;
static void* mcheck[MAXMALLOCSTOKEEP];
static char* mplace[MAXMALLOCSTOKEEP];
static int mlineno[MAXMALLOCSTOKEEP];
static int mcount;

void freewrlFree(int line, char *file, void *a)
{
    printf ("%x xfree at %s:%d\n",a,file,line); 
    FREETABLE(a,file,line);
    free(a);
}

void scanMallocTableOnQuit()
{
    for (mcount=0; mcount<MAXMALLOCSTOKEEP;mcount++) {
	if (mcheck[mcount]!=NULL) {
	    printf ("unfreed memory created at %s:%d \n",mplace[mcount],mlineno[mcount]);
	}
    }
}

/**
 * Check all mallocs
 */
void *freewrlMalloc(int line, char *file, size_t sz)
{
    void *rv;
    char myline[400];

    if (!mcheckinit) {
	for (mcount=0; mcount < MAXMALLOCSTOKEEP; mcount++) {
	    mcheck[mcount] = NULL;
	    mplace[mcount] = NULL;
	    mlineno[mcount] = 0;
	}
	mcheckinit = TRUE;
    }

    rv = malloc(sz);
    if (rv==NULL) {
	sprintf (myline, "MALLOC PROBLEM - out of memory at %s:%d for %d",file,line,sz);
	outOfMemory (myline);
    }
    /* printf ("%x malloc %d at %s:%d\n",rv,sz,file,line); */
    RESERVETABLE(rv,file,line);
    return rv;
}

void *freewrlRealloc (int line, char *file, void *ptr, size_t size)
{
    void *rv;
    char myline[400];

    /* printf ("%x xfree (from realloc) at %s:%d\n",ptr,file,line); */
    rv = realloc (ptr,size);
    if (rv==NULL) {
	if (size != 0) {
	    sprintf (myline, "REALLOC PROBLEM - out of memory at %s:%d size %d",file,line,size);
	    outOfMemory (myline);
	}
    }
    
    /* printf ("%x malloc (from realloc) %d at %s:%d\n",rv,size,file,line); */
    FREETABLE(ptr,file,line);
    RESERVETABLE(rv,file,line);
	
    return rv;
}


void *freewrlStrdup (int line, char *file, char *str)
{
    void *rv;
    char myline[400];

    /* printf ("%x xfree (from realloc) at %s:%d\n",ptr,file,line); */
    rv = strdup (str);
    if (rv==NULL) {
	sprintf (myline, "STRDUP PROBLEM - out of memory at %s:%d ",file,line);
	outOfMemory (myline);
    }

    /* printf ("%x malloc (from realloc) %d at %s:%d\n",rv,size,file,line); */
    RESERVETABLE(rv,file,line);
    return rv;
}

#endif /* defined(_DEBUG) & defined(DEBUG_MALLOC) */
