/*
=INSERT_TEMPLATE_HERE=

$Id: main.h,v 1.1 2009/02/05 10:33:04 couannette Exp $

FreeWRL/X3D main program.
Internal header: helper macros.

*/

#ifndef __FREEX3D_MAIN_H__
#define __FREEX3D_MAIN_H__


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
 

#endif /* __FREEX3D_MAIN_H__ */
