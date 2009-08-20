/*
=INSERT_TEMPLATE_HERE=

$Id: main.h,v 1.5 2009/08/20 03:14:08 dug9 Exp $

FreeWRL/X3D main program.
Internal header: helper macros.

*/

#ifndef __FREEWRL_MAIN_H__
#define __FREEWRL_MAIN_H__

/* LOG, WARNING, ERROR macros */

#if defined(FW_DEBUG)
# define DEBUG_(_expr) _expr
#else
# define DEBUG_(...)
#endif

/* To conform C99 ISO C (do not use GCC extension) */
#define DEBUG_MSG(...) DEBUG_(fprintf(stdout, __VA_ARGS__))
#define TRACE_MSG(...) DEBUG_(fprintf(stdout, __VA_ARGS__))
#define WARN_MSG(...)  DEBUG_(fprintf(stdout, __VA_ARGS__))
#define ERROR_MSG(...) DEBUG_(fprintf(stderr, __VA_ARGS__))

#if defined(_MSC_VER)
extern int optind;
#endif


#endif /* __FREEWRL_MAIN_H__ */
