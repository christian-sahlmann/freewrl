/*
=INSERT_TEMPLATE_HERE=

$Id: main.h,v 1.2 2009/02/11 15:12:54 istakenv Exp $

FreeWRL/X3D main program.
Internal header: helper macros.

*/

#ifndef __FREEWRL_MAIN_H__
#define __FREEWRL_MAIN_H__


/* LOG, WARNING, ERROR macros */
/* #if defined(_DEBUG) */
#if !defined(DEBUG_)
# define DEBUG_(_expr) _expr
#endif
/* #else */
/* # define DEBUG(...) */
/* #endif */

#define FW_DEBUG(_formargs...)   DEBUG_(fprintf(stdout, ##_formargs))
#define FW_WARN(_formargs...)    DEBUG_(fprintf(stdout, ##_formargs))
#define FW_ERROR(_formargs...)   fprintf(stderr, ##_formargs)
 

#endif /* __FREEWRL_MAIN_H__ */
