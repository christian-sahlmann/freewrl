/*******************************************************************
 *
 * FreeX3D support library
 *
 * internal header - internal.h
 *
 * Library internal declarations.
 *
 * $Id: internal.h,v 1.6 2008/12/04 05:59:52 couannette Exp $
 *
 *******************************************************************/

#ifndef __LIBFREEX3D_DECL_H__
#define __LIBFREEX3D_DECL_H__


/**
 * internal stuff needed by multiple C files in the library
 */
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

# define MALLOC(_sz) freewrlMalloc(__LINE__,__FILE__,_sz)
# define REALLOC(_a,_b) freewrlRealloc(__LINE__,__FILE__,_a,_b) 
# define FREE(_ptr) freewrlFree(__LINE__,__FILE__,_ptr)
# define FREE_IF_NZ(_ptr) if (_ptr) { FREE(_ptr); _ptr = 0; } \
                          else { ERROR_MSG("trying to free null pointer\n"); }
# define STRDUP(_a) freewrlStrdup(__LINE__,__FILE__,_a)
void *freewrlMalloc(int line, char *file, size_t sz);
void *freewrlRealloc(int line, char *file, void *ptr, size_t size);
void freewrlFree(int line, char *file, void *a);
void *freewrlStrdup(int line, char *file, char *str);

#else

# define MALLOC malloc
# define REALLOC realloc
# define FREE free
# define FREE_IF_NZ(_ptr) if (_ptr) { free(_ptr); _ptr = 0; }
# define STRDUP strdup

#endif /* defined(_DEBUG) */

#if defined(_DEBUG)

# define UNLINK(_fdd) do { \
		           TRACE_MSG("unlinking %s at %s:%d\n",_fdd,__FILE__,__LINE__); \
		           unlink (_fdd); \
		      } while (0)

#else

# define UNLINK unlink

#endif /* defined(_DEBUG) */

#if defined(_DEBUG)

#include <assert.h>
# define ASSERT(_ptr) do { if (!(_ptr)) { \
                         fprintf(stderr, "Parse assert failed: %s (%s:%d)\n", #_ptr, __FILE__, __LINE__); } \
                    } while (0)
#else

#define assert(_whatever)
#define ASSERT(_whatever)

#endif /* defined(_DEBUG) */


#endif /* __LIBFREEX3D_DECL_H__ */
