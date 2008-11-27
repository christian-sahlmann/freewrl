/*
=INSERT_TEMPLATE_HERE=

$Id: PluginSocket.h,v 1.2 2008/11/27 00:27:18 couannette Exp $

FreeWRL plugin utilities header file.

*/

#ifndef __FREEX3D_PLUGIN_SOCKET_H__
#define __FREEX3D_PLUGIN_SOCKET_H__


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


#endif /* __FREEX3D_PLUGIN_SOCKET_H__ */
