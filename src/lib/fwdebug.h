/*
=INSERT_TEMPLATE_HERE=

$Id: fwdebug.h,v 1.4 2009/10/06 01:03:53 couannette Exp $

FreeWRL support library.
Internal header: debug definitions.

*/

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


#ifndef __LIBFREEWRL_DEBUG_H__
#define __LIBFREEWRL_DEBUG_H__


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

#ifdef TEXVERBOSE
#define DEBUG_TEX(...) DEBUG_(printf("TEXTURE: " __VA_ARGS__))
#else
#define DEBUG_TEX(...)
#endif

#ifdef MEMVERBOSE
#define DEBUG_MEM(...) DEBUG_(printf("MEM: " __VA_ARGS__))
#else
#define DEBUG_MEM(...)
#endif

#ifdef CPARSERVERBOSE
#define DEBUG_CPARSER(...) DEBUG_(printf("CPARSER: " __VA_ARGS__))
#else
#define DEBUG_CPARSER(...)
#endif

#ifdef CPROTOVERBOSE
#define DEBUG_CPROTO(...) DEBUG_(printf("CPROTO: " __VA_ARGS__))
#else
#define DEBUG_CPROTO(...)
#endif

#ifdef PLUGINSOCKETVERBOSE
#define DEBUG_PLUGINSOCKET(...) DEBUG_(printf("PLUGINSOCKET: " __VA_ARGS__))
#else
#define DEBUG_PLUGINSOCKET(...)
#endif

/* FIXME: replace CR with CROUTE for clarity */
#ifdef CRVERBOSE
#define DEBUG_CR(...) DEBUG_(printf("CR: " __VA_ARGS__))
#else
#define DEBUG_CR(...)
#endif

/* FIXME: maybe shorten the def here to make it more practical */
#ifdef JSVRMLCLASSESVERBOSE
#define DEBUG_JSVRMLCLASSES(...) DEBUG_(printf("JSVRMLCLASSES: " __VA_ARGS__))
#else
#define DEBUG_JSVRMLCLASSES(...)
#endif

#ifdef JAVASCRIPTVERBOSE
#define DEBUG_JS(...) DEBUG_(printf("JS: " __VA_ARGS__))
#else
#define DEBUG_JS(...)
#endif

#ifdef SETFIELDVERBOSE
#define DEBUG_SETFIELD(...) DEBUG_(printf("SETFIELD: " __VA_ARGS__))
#else
#define DEBUG_SETFIELD(...)
#endif

#ifdef RENDERVERBOSE
#define DEBUG_RENDER(...) DEBUG_(printf("RENDER: " __VA_ARGS__))
#else
#define DEBUG_RENDER(...)
#endif

#ifdef CHILDVERBOSE
#define DEBUG_CHILD(...) DEBUG_(printf("CHILD: " __VA_ARGS__))
#else
#define DEBUG_CHILD(...)
#endif

/* FIXME: maybe replace SE with SENSOR for clarity */
#ifdef SEVERBOSE
#define DEBUG_SE(...) DEBUG_(printf("SE: " __VA_ARGS__))
#else
#define DEBUG_SE(...)
#endif

#ifdef STREAM_POLY_VERBOSE
#define DEBUG_STREAM_POLY(...) DEBUG_(printf("STREAM_POLY: " __VA_ARGS__))
#else
#define DEBUG_STREAM_POLY(...)
#endif

#ifdef MIDIVERBOSE
#define DEBUG_MIDI(...) DEBUG_(printf("MIDI: " __VA_ARGS__))
#else
#define DEBUG_MIDI(...)
#endif

#ifdef OCCLUSIONVERBOSE
#define DEBUG_OCCLUSION(...) DEBUG_(printf("OCCLUSION: " __VA_ARGS__))
#else
#define DEBUG_OCCLUSION(...)
#endif

#ifdef FRUSTUMVERBOSE
#define DEBUG_FRUSTUM(...) DEBUG_(printf("FRUSTUM: " __VA_ARGS__))
#else
#define DEBUG_FRUSTUM(...)
#endif

#ifdef SHADERVERBOSE
#define DEBUG_SHADER(...) DEBUG_(printf("SHADER: " __VA_ARGS__))
#else
#define DEBUG_SHADER(...)
#endif

#ifdef BINDVERBOSE
#define DEBUG_BIND(...) DEBUG_(printf("BIND: " __VA_ARGS__))
#else
#define DEBUG_BIND(...)
#endif

#ifdef X3DPARSERVERBOSE
#define DEBUG_X3DPARSER(...) DEBUG_(printf("X3DPARSER: " __VA_ARGS__))
#else
#define DEBUG_X3DPARSER(...)
#endif

/* FIXME: maybe change CAPABILITIES with X3DCAPS */
#ifdef CAPABILITIESVERBOSE
#define DEBUG_CAPABILITIES(...) DEBUG_(printf("CAPABILITIES: " __VA_ARGS__))
#else
#define DEBUG_CAPABILITIES(...)
#endif

/* FIXME: does this has to do with SENSOR or SEVERBOSE ? */
#ifdef SENSVERBOSE
#define DEBUG_SENS(...) DEBUG_(printf("SENS: " __VA_ARGS__))
#else
#define DEBUG_SENS(...)
#endif

#ifdef SOUNDVERBOSE
#define DEBUG_SOUND(...) DEBUG_(printf("SOUND: " __VA_ARGS__))
#else
#define DEBUG_SOUND(...)
#endif

#ifdef FILLVERBOSE
#define DEBUG_FILL(...) DEBUG_(printf("FILL: " __VA_ARGS__))
#else
#define DEBUG_FILL(...)
#endif

#ifdef TEXTVERBOSE
#define DEBUG_TEXT(...) DEBUG_(printf("TEXT: " __VA_ARGS__))
#else
#define DEBUG_TEXT(...)
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


#endif /* __LIBFREEWRL_DEBUG_H__ */
