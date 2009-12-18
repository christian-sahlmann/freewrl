/*******************************************************************
 *
 * FreeWRL sound module
 *
 * internal header - internal.h
 *
 * Version embedded.
 *
 * $Id: internal.h,v 1.1 2009/12/18 17:30:15 istakenv Exp $
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



#ifndef __FREEWRL_SND_INTERNAL_H__
#define __FREEWRL_SND_INTERNAL_H__


extern const char *freewrl_snd_get_version();

/* Useful to suppress things from non-debug builds */
#if defined(FW_DEBUG)
#  define DEBUG_(_expr) _expr
#else
#  define DEBUG_(_expr)
#endif

/* To conform C99 ISO C (do not use GCC extension) */
#define DEBUG_MSG(...) DEBUG_(fprintf(stdout, __VA_ARGS__))
#define TRACE_MSG(...) DEBUG_(fprintf(stdout, __VA_ARGS__))
#define WARN_MSG(...)  DEBUG_(fprintf(stdout, __VA_ARGS__))
#define ERROR_MSG(...) DEBUG_(fprintf(stderr, __VA_ARGS__))

#ifdef VERBOSE
#define DEBUG_FW(...) DEBUG_(printf("FW: " __VA_ARGS__))
#else
#define DEBUG_FW(...)
#endif

#ifdef ARGSVERBOSE
#define DEBUG_ARGS(...) DEBUG_(printf("TEXT: " __VA_ARGS__))
#else
#define DEBUG_ARGS(...)
#endif

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
                             DEBUG_MEM("free, pointer is already null at %s:%d\n", __FILE__, __LINE__); \
                         }


#endif /* __FREEWRL_SND_INTERNAL_H__ */
