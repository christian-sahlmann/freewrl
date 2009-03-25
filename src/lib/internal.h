/*******************************************************************
 *
 * FreeWRL support library
 *
 * internal header - internal.h
 *
 * Library internal declarations.
 *
 * $Id: internal.h,v 1.12 2009/03/25 14:16:03 crc_canada Exp $
 *
 *******************************************************************/

#ifndef __LIBFREEWRL_DECL_H__
#define __LIBFREEWRL_DECL_H__


/**
 * Internal stuff needed by multiple C files in the library
 */

/* Useful to suppress things from non-debug builds */
#ifdef _DEBUG
#  define DEBUG_(_expr) _expr
#else
#  define DEBUG_(_expr)
#endif

#define TRACE_MSG(_formargs...) DEBUG_(fprintf(stdout, ##_formargs))
#define WARN_MSG(_formargs...)  DEBUG_(fprintf(stdout, ##_formargs))
#define ERROR_MSG(_formargs...) DEBUG_(fprintf(stderr, ##_formargs))

/**
 * Those macro get defined only when debugging is enabled
 */
#if defined(_DEBUG) && defined(DEBUG_MALLOC)

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

#else /* defined(_DEBUG) && defined(DEBUG_MALLOC) */

# define MALLOC malloc
# define REALLOC realloc
# define FREE free
# define STRDUP strdup
# define UNLINK unlink
# define ASSERT(_whatever)

#endif /* defined(_DEBUG) && defined(DEBUG_MALLOC) */

/* This get always defined, but ERROR_MSG is no-op without _DEBUG */

#define FREE_IF_NZ(_ptr) if (_ptr) { \
                             FREE(_ptr); \
                             _ptr = 0; } \
                         else { \
                             WARN_MSG("free, pointer is already null at %s:%d\n", __FILE__, __LINE__); \
                         }


#endif /* __LIBFREEWRL_DECL_H__ */
