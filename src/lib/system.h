/*
=INSERT_TEMPLATE_HERE=

$Id: system.h,v 1.10 2009/01/03 01:15:07 couannette Exp $

FreeX3D support library.
Internal header: system dependencies.

*/

#ifndef __LIBFREEX3D_SYSTEM_H__
#define __LIBFREEX3D_SYSTEM_H__

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

#if HAVE_SYS_WAIT_H
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

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

#if !defined(assert)
# include <assert.h>
#endif


#endif /* __LIBFREEX3D_SYSTEM_H__ */
