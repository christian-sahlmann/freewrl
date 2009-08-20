/*******************************************************************
 *
 * FreeWRL support library
 *
 * internal header - internal.h
 *
 * Library internal declarations.
 *
 * $Id: internal.h,v 1.18 2009/08/20 00:37:52 couannette Exp $
 *
 *******************************************************************/

#ifndef __LIBFREEWRL_DECL_H__
#define __LIBFREEWRL_DECL_H__


/**
 * Internal stuff needed by multiple C files in the library
 */

/* Useful to suppress things from non-debug builds */
#if defined(FW_DEBUG)
#  define DEBUG_(_expr) _expr
#else
#  define DEBUG_(_expr)
#endif

#if defined(_MSC_VER)
/* FIXME: investigate on this... (michel) */
#include <stddef.h> /* for offsetof(...) */
/* textures.c > jpeg > jmorecfg.h tries to redefine booleand but you can say you have it */
#define HAVE_BOOLEAN 1    
#define M_PI acos(-1.0)
#endif

/* To conform C99 ISO C (do not use GCC extension) */
#define DEBUG_MSG(...) DEBUG_(fprintf(stdout, __VA_ARGS__))
#define TRACE_MSG(...) DEBUG_(fprintf(stdout, __VA_ARGS__))
#define WARN_MSG(...)  DEBUG_(fprintf(stdout, __VA_ARGS__))
#define ERROR_MSG(...) DEBUG_(fprintf(stderr, __VA_ARGS__))

/**
 * Those macro get defined only when debugging is enabled
 */
#if defined(FW_DEBUG) && defined(DEBUG_MALLOC)

# define MALLOC(_sz) freewrlMalloc(__LINE__,__FILE__,_sz)
# define REALLOC(_a,_b) freewrlRealloc(__LINE__,__FILE__,_a,_b) 
# define FREE(_ptr) freewrlFree(__LINE__,__FILE__,_ptr)
# define STRDUP(_a) freewrlStrdup(__LINE__,__FILE__,_a)
#include <stdlib.h>
void *freewrlMalloc(int line, char *file, size_t sz);
void *freewrlRealloc(int line, char *file, void *ptr, size_t size);
void freewrlFree(int line, char *file, void *a);
void *freewrlStrdup(int line, char *file, char *str);

# define UNLINK(_fdd) do { \
		           TRACE_MSG("TRACE: unlink %s at %s:%d\n",_fdd,__FILE__,__LINE__); \
		           unlink (_fdd); \
		      } while (0)

# define ASSERT(_ptr) do { if (!(_ptr)) { \
                           ERROR_MSG("ERROR: assert failed: %s (%s:%d)\n", #_ptr, __FILE__, __LINE__); } \
                      } while (0)

#else /* defined(FW_DEBUG) && defined(DEBUG_MALLOC) */

# define MALLOC malloc
# define REALLOC realloc
# define FREE free
#if defined(_MSC_VER)
# define STRDUP _strdup
# define UNLINK _unlink
# define TEMPNAM _tempnam
#else
# define STRDUP strdup
# define UNLINK unlink
# define TEMPNAM tempnam
#endif
# define ASSERT(_whatever)

#endif /* defined(FW_DEBUG) && defined(DEBUG_MALLOC) */

/* This get always defined, but ERROR_MSG is no-op without _DEBUG */

#define FREE_IF_NZ(_ptr) if (_ptr) { \
                             FREE(_ptr); \
                             _ptr = 0; } \
                         else { \
                             WARN_MSG("free, pointer is already null at %s:%d\n", __FILE__, __LINE__); \
                         }


#endif /* __LIBFREEWRL_DECL_H__ */
