/*
=INSERT_TEMPLATE_HERE=

$Id: pluginUtils.h,v 1.5 2009/08/01 09:45:39 couannette Exp $

FreeWRL plugin utilities header file.

*/

#ifndef __FREEWRL_PLUGIN_UTILS_H__
#define __FREEWRL_PLUGIN_UTILS_H__


#ifdef __cplusplus
extern "C" {
#endif


#define SMALLSTRINGSIZE 64
#define STRINGSIZE 128
#define LARGESTRINGSIZE 256

#define PLUGIN_PORT 2009
#define PLUGIN_TIMEOUT_SEC 10
#define PLUGIN_TIMEOUT_NSEC 0

#define PLUGIN_RETRY 2
#define SLEEP_TIME 5

#ifndef WIN32
#define NO_ERROR 0
#define SOCKET_ERROR -1000
#endif
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

/* used by fw/fwWindow.c */
void sendXwinToPlugin(void);

#ifdef __cplusplus
}
#endif


#endif /* __FREEWRL_PLUGIN_UTILS_H__ */
