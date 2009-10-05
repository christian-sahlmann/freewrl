/*
 * FreeWRL plugin
 *
 * To communicate with FreeWRL, we define structures and protocol.
 *
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


#ifndef __FREEWRL_PLUGIN_UTILS_H__
#define __FREEWRL_PLUGIN_UTILS_H__


/* JAS 
#ifndef _DEBUG
#define _DEBUG 0
#endif
*/

#define SMALLSTRINGSIZE 64
#define STRINGSIZE 128
#define LARGESTRINGSIZE 256

#define PLUGIN_PORT 2009
#define PLUGIN_TIMEOUT_SEC 10
#define PLUGIN_TIMEOUT_NSEC 0

#define PLUGIN_RETRY 2
#define SLEEP_TIME 5

#define NO_ERROR 0
#define SOCKET_ERROR -1000
#define SIGNAL_ERROR -1001

#define UNUSED(v) ((void) v)

typedef struct _urlRequest {
    char url[FILENAME_MAX]; /* limit url length (defined in stdio.h) */
    void *instance;   /* NPP instance for plugin */
    unsigned int notifyCode; /* NPN_GetURLNotify, NPP_URLNotify */
} urlRequest;

/* const char* XEventToString(int type); */
/* const char* XErrorToString(int error); */

void URLencod (char *dest, const char *src, int maxlen);

/* for reporting version info -- this function is defined in "internal_version.c" */
const char *freewrl_plugin_get_version(void);

#endif /* __FREEWRL_PLUGIN_UTILS_H__ */
