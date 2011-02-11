/*******************************************************************
 *
 * FreeWRL main program
 *
 * internal header - system.h
 *
 * Program system dependencies.
 *
 * $Id: system.h,v 1.11 2011/02/11 18:46:25 crc_canada Exp $
 *
 *******************************************************************/

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



#ifndef __FREEWRL_SYSTEM_H__
#define __FREEWRL_SYSTEM_H__

#if HAVE_STDINT_H
# include <stdint.h>
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

#if defined(_MSC_VER)

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

#if HAVE_PTHREAD
# include <pthread.h>
#endif

#if HAVE_SYS_IPC_H
# include <sys/ipc.h>
#endif

#if HAVE_SYS_MSG_H
# include <sys/msg.h>
#endif

#if !defined(assert)
# include <assert.h>
#endif

#if HAVE_DIRECT_H
#include <direct.h>
#endif

#if HAVE_SIGNAL_H 
#include <signal.h>
    /* install the signal handler for SIGQUIT */
#define SIGQUIT SIGINT
/*#define SIGTERM SIGTERM  *//*not generated under win32 but can raise */
/*#define SIGSEGV SIGSEGV */  /* memory overrun */
#define SIGALRM SIGABRT  /* I don't know so I guessed the lookup */
#define SIGHUP SIGFPE   /* fpe means floating poinot error */
#endif


#else

#if HAVE_SIGNAL_H 
#include <signal.h>
#endif

#endif



#endif /* __FREEWRL_SYSTEM_H__ */
