/*
=INSERT_TEMPLATE_HERE=

$Id: system_js.h,v 1.1 2008/12/31 13:08:15 couannette Exp $

FreeX3D support library.
Internal header: Javascript engine dependencies.

*/

#ifndef __LIBFREEX3D_SYSTEM_JS_H__
#define __LIBFREEX3D_SYSTEM_JS_H__


/* 
   spidermonkey is built with the following flags:

-Wall -Wno-format -no-cpp-precomp -fno-common -DJS_THREADSAFE -DXP_UNIX -DSVR4 -DSYSV -D_BSD_SOURCE -DPOSIX_SOURCE -DDARWIN  -UDEBUG -DNDEBUG -UDEBUG_root -DJS_THREADSAFE -DEDITLINE

*/

#include <jsapi.h> /* JS compiler */
#include <jsdbgapi.h> /* JS debugger */


#endif /* __LIBFREEX3D_SYSTEM_JS_H__ */
