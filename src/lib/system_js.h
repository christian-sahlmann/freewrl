/*
=INSERT_TEMPLATE_HERE=

$Id: system_js.h,v 1.2 2009/02/11 15:12:54 istakenv Exp $

FreeWRL support library.
Internal header: Javascript engine dependencies.

*/

#ifndef __LIBFREEWRL_SYSTEM_JS_H__
#define __LIBFREEWRL_SYSTEM_JS_H__


/* 
   spidermonkey is built with the following flags:

-Wall -Wno-format -no-cpp-precomp -fno-common -DJS_THREADSAFE -DXP_UNIX -DSVR4 -DSYSV -D_BSD_SOURCE -DPOSIX_SOURCE -DDARWIN  -UDEBUG -DNDEBUG -UDEBUG_root -DJS_THREADSAFE -DEDITLINE

*/

#include <jsapi.h> /* JS compiler */
#include <jsdbgapi.h> /* JS debugger */


#endif /* __LIBFREEWRL_SYSTEM_JS_H__ */
