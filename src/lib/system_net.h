/*
=INSERT_TEMPLATE_HERE=

$Id: system_net.h,v 1.3 2009/08/01 09:45:39 couannette Exp $

FreeWRL support library.
Internal header: network dependencies.

*/

#ifndef __LIBFREEWRL_SYSTEM_NET_H__
#define __LIBFREEWRL_SYSTEM_NET_H__

#if !defined(WIN32)

/* Try to fix problem in EAIServ.c */
#if !(defined(OS_MAC) && defined(ARCH_PPC))
# include <sys/ipc.h>
# include <sys/msg.h> 
#endif

#include <netinet/in.h> 
#include <sys/socket.h>

#else

#include <winsock.h>

#endif


#endif /* __LIBFREEWRL_SYSTEM_NET_H__ */
