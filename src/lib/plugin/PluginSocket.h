/*
 * $Id: PluginSocket.h,v 1.1 2008/11/26 11:24:13 couannette Exp $
 */

#ifndef __pluginSocket_h__
#define __pluginSocket_h__


#ifndef AQUA

#include <unistd.h>

#endif /* AQUA */

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "pluginUtils.h"


#ifdef __cplusplus
extern "C" {
#endif

/* what Browser are we running under? eg, netscape, mozilla, konqueror, etc */

#define MAXNETSCAPENAMELEN 256
extern char NetscapeName[MAXNETSCAPENAMELEN];

char *requestUrlfromPlugin(int sockDesc, uintptr_t plugin_instance, const char *url);
void  requestNewWindowfromPlugin( int sockDesc, uintptr_t plugin_instance, const char *url);
void requestPluginPrint(int sockDesc, const char* msg);
int receiveUrl(int sockDesc, urlRequest *request);


#ifdef __cplusplus
}
#endif


#endif /*__pluginSocket_h__ */
