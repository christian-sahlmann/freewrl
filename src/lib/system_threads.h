/*
=INSERT_TEMPLATE_HERE=

$Id: system_threads.h,v 1.3.2.1 2009/07/08 21:55:04 couannette Exp $

FreeWRL support library.
Internal header: threading library, and processor control (sched).

*/

#ifndef __LIBFREEWRL_SYSTEM_THREADS_H__
#define __LIBFREEWRL_SYSTEM_THREADS_H__


#if HAVE_PTHREAD
# include <pthread.h>
#endif

#if HAVE_SCHED_H
#include <sched.h>
#endif

/**
 * Threads
 */
#if !defined(WIN32)

#define DEF_THREAD(_t) pthread_t _t = NULL
#define TEST_NULL_THREAD(_t) (_t == NULL)
#define ID_THREAD(_t) ((unsigned int) _t)
#define ZERO_THREAD(_t) (_t = NULL)

#else /* !defined(WIN32) */

#define DEF_THREAD(_t) pthread_t _t = { NULL, 0 }
#define TEST_NULL_THREAD(_t) (_t.p == NULL)
#define ID_THREAD(_t) ((unsigned int) _t.p)
#define ZERO_THREAD(_t) { _t.p = NULL; }

#endif


#endif /* __LIBFREEWRL_SYSTEM_THREADS_H__ */
