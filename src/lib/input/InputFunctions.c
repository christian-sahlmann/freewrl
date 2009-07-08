/*
=INSERT_TEMPLATE_HERE=

$Id: InputFunctions.c,v 1.3.2.1 2009/07/08 21:55:04 couannette Exp $

CProto ???

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h" 
#include "../main/headers.h"


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
	char mynewname[1000];
	char tempname[1000];
	char sysline[1000];
	char firstbytes[20];
	int isTemp = FALSE;
	int removeIt = FALSE;

	bufcount = 0;
	bufsize = 5 * READSIZE; /*  initial size*/
	buffer =(char *)MALLOC(bufsize * sizeof (char));

	/*printf ("start of readInputString, \n\tfile: %s\n",
	fn);
	printf ("\tBrowserFullPath: %s\n\n",BrowserFullPath); */

	/* verify (possibly, once again) the file name. This
	* should have been done already, with the exception of
	* EXTERNPROTOS; these "sneak" past the normal file reading
	* channels. This does not matter; if this is a HTTP request,
	* and is not an EXTERNPROTO, it will already be a local file
	* */

	/* printf ("before mas, in Input, fn %s\n",fn); */
	strcpy (mynewname, fn);

	/* check to see if this file exists */
	if (!fileExists(mynewname,firstbytes,TRUE,&removeIt)) {
		ConsoleMessage("problem reading file '%s' ('%s')",fn,mynewname);
		strcpy (buffer,"\n");
		return buffer;
	}

	/* was this a network file that got copied to the local cache? */

	if (((unsigned char) firstbytes[0] == 0x1f) &&
		((unsigned char) firstbytes[1] == 0x8b)) {
			int len;

			/* printf ("this is a gzipped file!\n"); */
			isTemp = TRUE;
			sprintf (tempname, "%s.gz",tempnam("/tmp","freewrl_tmp"));

			/* first, move this to a .gz file */
			localCopy(tempname, mynewname);

			/* now, unzip it */
			/* remove the .gz from the file name - ".gz" is 3 characters */
			len = strlen(tempname); tempname[len-3] = '\0';
			sprintf (sysline,"%s %s",UNZIP,tempname);

			freewrlSystem (sysline);
			infile = fopen(tempname,"r");
	} else {

		/* ok, now, really read this one. */
		infile = fopen(mynewname,"r");
	}

	if ((buffer == 0) || (infile == NULL)){
		ConsoleMessage("problem reading file '%s' (stdio:'%s')",fn,mynewname);
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

	if (isTemp) UNLINK(tempname);
	if (removeIt) UNLINK (mynewname);

	return (buffer);
}

