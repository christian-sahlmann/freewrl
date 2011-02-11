/*
=INSERT_TEMPLATE_HERE=

$Id: system.h,v 1.25 2011/02/11 18:47:27 crc_canada Exp $

FreeWRL support library.
Internal header: system dependencies.

*/

/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2009 CRC Canada. (http://www.crc.gc.ca)

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FreeWRL/FreeX3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FreeWRL/FreeX3D.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/


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


/* do we have JavaScript? */
#ifdef IPHONE
	#undef HAVE_JAVASCRIPT
#else
	/* Everything has JavaScript */
	#define HAVE_JAVASCRIPT
#endif

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
# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# include <unistd.h>
#endif

#if defined(HAVE_LIMITS_H)
# include <limits.h>
#endif

#if defined(HAVE_STRING_H)
# include <string.h>
#endif

#if !defined(HAVE_STRNLEN)
#define strnlen __fw_strnlen
size_t __fw_strnlen(const char *s, size_t maxlen);
#endif

#if !defined(HAVE_STRNDUP)
# define strndup __fw_strndup
char *__fw_strndup(const char *s, size_t n);
#endif

#if !defined(HAVE_USLEEP) && defined(WIN32)
#include <windows.h>
#define usleep(us) Sleep((us)/1000)
#define sleep(us) Sleep(us)
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

/* Those macro may not be declared everywhere */
#ifndef IPHONE
#if !defined(min)
# define min(X, Y)                     \
     ({ typeof (X) __x = (X), __y = (Y);   \
        (__x < __y) ? __x : __y; })
#endif

#if !defined(max)
# define max(X, Y)                     \
     ({ typeof (X) __x = (X), __y = (Y);   \
        (__x > __y) ? __x : __y; })
#endif
#else
#define min MIN
#define max MAX
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
#if defined(_WIN32)

/* FIXME: those calls to bzero & bcopy shall be remove from libeai ;)... */

/*  http://www.opengroup.org/onlinepubs/000095399/functions/bzero.html */
/*  http://www.opengroup.org/onlinepubs/000095399/functions/bcopy.html */
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0) 
#define bcopy(b1,b2,len) (memmove((b2), (b1), (len)), (void) 0)

/* borrowed from CScripts.c */
#define PATH_MAX _MAX_PATH  /*32kb*/

/**
 *    For Win32 portability: we need to define 
 *    those standard functions.
 */

/* _strdup is defined in string.h */
# include <string.h>
# define strdup _strdup

/* _unlink is defined in io.h and (needs stdio.h) */
# include <io.h>
# include <stdio.h>
# define unlink _unlink

/* _access is defined in io.h and error constants in errno.h */
# include <io.h>
# include <errno.h>
# define access _access

/* _getpid is defined in process.h */
# include <process.h>
# define getpid _getpid

/* _tempnam is defined in stdio.h */
# include <stdio.h>
# define tempnam _tempnam

/* _stat is defined in sys/stat.h (needs sys/types.h) */
# include <sys/types.h>
# include <sys/stat.h>
# define stat _stat
/* NOTE: http://msdn.microsoft.com/en-us/library/14h5k7ff.aspx
   stat usage:
   If path contains the location of a directory, it cannot 
   contain a trailing backslash. If it does, -1 will be returned 
   and errno will be set to ENOENT. 
*/

#endif


#endif /* __LIBFREEWRL_SYSTEM_H__ */
