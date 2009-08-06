/*
=INSERT_TEMPLATE_HERE=

$Id: system_js.h,v 1.4 2009/08/06 21:24:03 couannette Exp $

FreeWRL support library.
Internal header: Javascript engine dependencies.

*/

#ifndef __LIBFREEWRL_SYSTEM_JS_H__
#define __LIBFREEWRL_SYSTEM_JS_H__


/* 
   spidermonkey is built with the following flags on Mac:

-Wall -Wno-format -no-cpp-precomp -fno-common -DJS_THREADSAFE -DXP_UNIX -DSVR4 -DSYSV -D_BSD_SOURCE -DPOSIX_SOURCE -DDARWIN  -UDEBUG -DNDEBUG -UDEBUG_root -DJS_THREADSAFE -DEDITLINE

*/

#define JS_HAS_FILE_OBJECT 1 /* workaround warning=>error */

#ifdef MOZILLA_JS_UNSTABLE_INCLUDES
# include "../unstable/jsapi.h" /* JS compiler */
# include "../unstable/jsdbgapi.h" /* JS debugger */
#else
# include <jsapi.h> /* JS compiler */
# include <jsdbgapi.h> /* JS debugger */
#endif

#endif /* __LIBFREEWRL_SYSTEM_JS_H__ */
