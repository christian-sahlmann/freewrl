/*
  $Id: InputFunctions.c,v 1.18 2010/08/11 16:43:32 crc_canada Exp $

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

#define DJ_KEEP_COMPILER_WARNING 0
#if DJ_KEEP_COMPILER_WARNING
#define READSIZE 2048
#endif

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


/* sscanf replacements */
void scanUnsignedIntoValue(char *sp, size_t *rv) {
        *rv = 0;

        /* skip leading spaces, if there are any */
        while ((*sp <= ' ') && (*sp != '\0')) sp++;
        while ((*sp >='0') && (*sp <= '9')) {
                *rv *= 10; *rv += (int) (*sp - '0'); sp++;
        }
}

