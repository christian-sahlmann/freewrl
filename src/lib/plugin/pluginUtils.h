/*
 * $Id: pluginUtils.h,v 1.1 2008/11/26 11:24:13 couannette Exp $
 *
 * FreeWRL plugin utilities header file.
 */

#ifndef __pluginUtils_h__
#define __pluginUtils_h__

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>

#ifdef AQUA
#include <unistd.h>
#else
#include <X11/X.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef _DEBUG
#define _DEBUG 0
#endif

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


const char* XEventToString(int type);
const char* XErrorToString(int error);

void URLencod (char *dest, const char *src, int maxlen);

#ifdef __cplusplus
}
#endif


#endif /*  __pluginUtils_h__ */
