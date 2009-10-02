/*
=INSERT_TEMPLATE_HERE=

$Id: system_net.h,v 1.5 2009/10/02 19:34:16 crc_canada Exp $

FreeWRL support library.
Internal header: network dependencies.

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


#ifndef __LIBFREEWRL_SYSTEM_NET_H__
#define __LIBFREEWRL_SYSTEM_NET_H__

#if !defined(WIN32)

/* Try to fix problem in EAIServ.c */
#if !(defined(OS_MAC) && defined(ARCH_PPC))
/* JAS - no longer required 
# include <sys/ipc.h>
# include <sys/msg.h> 
*/

#endif

#include <netinet/in.h> 
#include <sys/socket.h>

#else

#include <winsock.h>

#endif


#endif /* __LIBFREEWRL_SYSTEM_NET_H__ */
