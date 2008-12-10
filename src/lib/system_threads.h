/*
=INSERT_TEMPLATE_HERE=

$Id: system_threads.h,v 1.2 2008/12/10 14:31:53 couannette Exp $

FreeX3D support library.
Internal header: threading library, and processor control (sched).

*/

#ifndef __LIBFREEX3D_SYSTEM_THREADS_H__
#define __LIBFREEX3D_SYSTEM_THREADS_H__


#if HAVE_PTHREAD
# include <pthread.h>
#endif

#if HAVE_SCHED_H
#include <sched.h>
#endif


#endif /* __LIBFREEX3D_SYSTEM_THREADS_H__ */
