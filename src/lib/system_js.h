/*
=INSERT_TEMPLATE_HERE=

$Id: system_js.h,v 1.5 2009/10/01 19:35:36 crc_canada Exp $

FreeWRL support library.
Internal header: Javascript engine dependencies.

*/

/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2009 CRC Canada. (http://www.crc.gc.ca)

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FreeWRL/FreeX3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FreeWRL/FreeX3D.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/


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
