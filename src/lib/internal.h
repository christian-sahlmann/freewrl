/*
=INSERT_TEMPLATE_HERE=

$Id: internal.h,v 1.19 2009/09/01 11:24:26 couannette Exp $

FreeWRL support library.
Library internal declarations.

*/

#ifndef __LIBFREEWRL_DECL_H__
#define __LIBFREEWRL_DECL_H__


#include <fwdebug.h>

/**
 * Internal stuff needed by multiple C files in the library
 */

#if defined(_MSC_VER)
/* FIXME: investigate on this... (michel) */
#include <stddef.h> /* for offsetof(...) */
/* textures.c > jpeg > jmorecfg.h tries to redefine booleand but you can say you have it */
#define HAVE_BOOLEAN 1    
#define M_PI acos(-1.0)
#endif


#endif /* __LIBFREEWRL_DECL_H__ */
