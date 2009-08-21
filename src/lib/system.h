/*
=INSERT_TEMPLATE_HERE=

$Id: system.h,v 1.15 2009/08/21 07:41:36 couannette Exp $

FreeWRL support library.
Internal header: system dependencies.

*/

#ifndef __LIBFREEWRL_SYSTEM_H__
#define __LIBFREEWRL_SYSTEM_H__

/**
 * Strict necessary common system header files:
 *
 * Platform detection in configure will give us some defines
 * that we'll use to include system headers.
 *
 * For modularity, special headers are defined for special needs:
 *
 * system_fonts.h       fonts related headers
 * system_js.h          Javascript engine headers
 * system_net.h		network related headers
 * system_threads.h	threading related headers
 * display.h		window system (Aqua/X11/Motif + OpenGL) related headers
 */

#if HAVE_STDINT_H
# include <stdint.h>
#endif

#if HAVE_CTYPE_H
#include <ctype.h>
#endif

#if STDC_HEADERS
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#else
# if !HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
char *strchr (), *strrchr ();
# if !HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#if HAVE_STDBOOL_H
# include <stdbool.h>
#else
# if ! HAVE__BOOL
#  ifdef __cplusplus
typedef bool _Bool;
#  else
typedef unsigned char _Bool;
#  endif
# endif
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif

#define TRUE 1
#define FALSE 0

#if HAVE_UNISTD_H
# include <sys/types.h>
# include <unistd.h>
#endif

#if !defined(HAVE_STRNLEN)
#define strnlen __fw_strnlen
size_t __fw_strnlen(const char *s, size_t maxlen);
#endif

#if !defined(HAVE_STRNDUP)
#define strndup __fw_strndup
char *__fw_strndup(const char *s, size_t n);
#endif

#if !defined(HAVE_USLEEP) && defined(WIN32)
#define usleep(us) Sleep((us)/1000)
#endif

#if defined(HAVE_SYS_WAIT_H)
# include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

#if HAVE_MATH_H
# include <math.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#if HAVE_TIME_H
# include <time.h>
#endif

#if !defined(HAVE_GETTIMEOFDAY) && defined(WIN32)
#define gettimeofday __fw_gettimeofday
int __fw_gettimeofday(struct timeval *tv, struct timezone *tz);
#endif

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

#if !defined(assert)
# include <assert.h>
#endif

/**
 * Misc
 */
#if defined(WIN32)

/* FIXME: those calls to bzero & bcopy shall be remove from libeai ;)... */

/*  http://www.opengroup.org/onlinepubs/000095399/functions/bzero.html */
/*  http://www.opengroup.org/onlinepubs/000095399/functions/bcopy.html */
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0) 
#define bcopy(b1,b2,len) (memmove((b2), (b1), (len)), (void) 0)

#endif


#endif /* __LIBFREEWRL_SYSTEM_H__ */
