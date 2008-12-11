/*******************************************************************
 *
 * FreeX3D support library
 *
 * internal header - internal.h
 *
 * Library internal declarations.
 *
 * $Id: internal.h,v 1.8 2008/12/11 22:18:03 crc_canada Exp $
 *
 *******************************************************************/

#ifndef __LIBFREEX3D_DECL_H__
#define __LIBFREEX3D_DECL_H__


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
#define ERROR_MSG(_formargs...) DEBUG_(fprintf(stderr, ##_formargs))

/**
 * Those macro get defined only when debugging is enabled
 */
#if defined(_DEBUG)
#include <stdlib.h>
# define MALLOC(_sz) freewrlMalloc(__LINE__,__FILE__,_sz)
# define REALLOC(_a,_b) freewrlRealloc(__LINE__,__FILE__,_a,_b) 
# define FREE(_ptr) freewrlFree(__LINE__,__FILE__,_ptr)
# define STRDUP(_a) freewrlStrdup(__LINE__,__FILE__,_a)

void *freewrlMalloc(int line, char *file, size_t sz);
void *freewrlRealloc(int line, char *file, void *ptr, size_t size);
void freewrlFree(int line, char *file, void *a);
void *freewrlStrdup(int line, char *file, char *str);

# define UNLINK(_fdd) do { \
		           TRACE_MSG("TRACE: unlink %s at %s:%d\n",_fdd,__FILE__,__LINE__); \
		           unlink (_fdd); \
		      } while (0)

# include <assert.h>
# define ASSERT(_ptr) do { if (!(_ptr)) { \
                           ERROR_MSG("ERROR: assert failed: %s (%s:%d)\n", #_ptr, __FILE__, __LINE__); } \
                      } while (0)

#else

# define MALLOC malloc
# define REALLOC realloc
# define FREE free
# define STRDUP strdup
# define UNLINK unlink
# define assert(_whatever)
# define ASSERT(_whatever)

#endif /* defined(_DEBUG) */

/* This get always defined, but ERROR_MSG is no-op without _DEBUG */

#define FREE_IF_NZ(_ptr) if (_ptr) { \
                             FREE(_ptr); \
                             _ptr = 0; } \
                         else { \
                             /* JAS - this is not an error...  ERROR_MSG("ERROR: trying to free null pointer at %s:%d\n", __FILE__, __LINE__); */ \
			ERROR_MSG("free, pointer is already null at %s:%d\n", __FILE__, __LINE__); \
                         }


#endif /* __LIBFREEX3D_DECL_H__ */
