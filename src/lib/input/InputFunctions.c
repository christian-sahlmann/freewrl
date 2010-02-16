/*
  $Id: InputFunctions.c,v 1.16 2010/02/16 21:21:47 crc_canada Exp $

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


char * stripLocalFileName (char * origName) 
{
	if (!origName)
		return NULL;

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


/* sscanf replacements */
void scanUnsignedIntoValue(char *sp, size_t *rv) {
        *rv = 0;

        /* skip leading spaces, if there are any */
        while ((*sp <= ' ') && (*sp != '\0')) sp++;
        while ((*sp >='0') && (*sp <= '9')) {
                *rv *= 10; *rv += (int) (*sp - '0'); sp++;
        }
}

