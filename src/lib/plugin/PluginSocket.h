/*
=INSERT_TEMPLATE_HERE=

$Id: PluginSocket.h,v 1.5 2009/10/05 15:07:23 crc_canada Exp $

FreeWRL plugin utilities header file.

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


#ifndef __FREEWRL_PLUGIN_SOCKET_H__
#define __FREEWRL_PLUGIN_SOCKET_H__


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


#endif /* __FREEWRL_PLUGIN_SOCKET_H__ */
