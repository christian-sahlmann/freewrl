/*******************************************************************
 *
 * FreeWRL main program
 *
 * internal header - system.h
 *
 * Program system dependencies.
 *
 * $Id: system.h,v 1.7 2009/08/01 09:45:39 couannette Exp $
 *
 *******************************************************************/

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
# include <sys/types.h>
# include <unistd.h>
#endif

#if defined(HAVE_SIGNAL_H)
# include <signal.h>
#endif


#endif /* __FREEWRL_SYSTEM_H__ */
