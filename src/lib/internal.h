/*
=INSERT_TEMPLATE_HERE=

$Id: internal.h,v 1.48 2011/05/31 00:52:42 crc_canada Exp $

???

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

//<<<<<<< internal.h
#include <config.h>
#include <system.h>
#include <display.h>
//#include <internal.h>

#include <libFreeWRL.h>
#include <list.h>
#include <io_files.h>
#include <resources.h>

#include "vrml_parser/Structs.h"
#include "main/headers.h"
#include "vrml_parser/CParseGeneral.h"
#include "scenegraph/Vector.h"
#include "vrml_parser/CFieldDecls.h"
#include "world_script/JScript.h"
#include "world_script/CScripts.h"
#include "world_script/fieldSet.h"
#include "vrml_parser/CParseParser.h"
#include "vrml_parser/CParseLexer.h"
#include "vrml_parser/CProto.h"
#include "vrml_parser/CParse.h"
#include "input/InputFunctions.h"	/* resolving implicit declarations */
#include "input/EAIHeaders.h"	/* resolving implicit declarations */
#include "input/EAIHelpers.h"	/* resolving implicit declarations */

#include "x3d_parser/X3DParser.h"
#include "x3d_parser/X3DProtoScript.h"

//#ifndef STATIC_ONCE
//static int currentProtoDeclare  = INT_ID_UNDEFINED;
//static int MAXProtos = 0;
//static int curProDecStackInd = 0;
////static int currentProtoInstance = INT_ID_UNDEFINED;
//static int currentProtoInstance[PROTOINSTANCE_MAX_LEVELS];
//#else
extern int currentProtoDeclare;
extern int MAXProtos;
extern int curProDecStackInd;
//static int currentProtoInstance = INT_ID_UNDEFINED;
extern int currentProtoInstance[PROTOINSTANCE_MAX_LEVELS];
//#endif
#define STATIC_ONCE 1
static int getFieldAccessMethodFromProtoInterface (struct VRMLLexer *myLexer, char *fieldName, int protono);

#define CPI ProtoInstanceTable[curProtoInsStackInd]
#define CPD PROTONames[currentProtoDeclare]

/* for parsing script initial fields */
#define MP_NAME 0
#define MP_ACCESSTYPE 1
#define MP_TYPE 2
#define MP_VALUE 3
#define MPFIELDS 4 /* MUST be the highest MP* plus one - array size */

#define UNIQUE_NUMBER_HOLDER "-fReeWrl-UniqueNumH"

/* ProtoInstance table This table is a dynamic table that is used for keeping track of ProtoInstance field values... */
//#ifndef CURPROINSTSTACK
//static int curProtoInsStackInd = -1;
//#define CURPROINSTSTACK 1
//
//struct PROTOInstanceEntry {
//	char *name[PROTOINSTANCE_MAX_PARAMS];
//	char *value[PROTOINSTANCE_MAX_PARAMS];
//	int type[PROTOINSTANCE_MAX_PARAMS]; //0-string 1-itoa(DEF index) 10-(FIELDTYPE_SFNODE) union anyVrml* or X3D_Node* 11-(FIELDTYPE_MFNODE) union anyVrml* or Multi_Node*
//	char *defName;
//	int container;
//	int paircount;
//	int uniqueNumber;
//};
//static struct PROTOInstanceEntry ProtoInstanceTable[PROTOINSTANCE_MAX_LEVELS];
//
///* PROTO table */
//struct PROTOnameStruct {
//	char *definedProtoName;
//	char *url;
//	FILE *fileDescriptor;
//	char *fileName;
//	int charLen;
//	int fileOpen;
//	int isExternProto;
//	struct Shader_Script *fieldDefs;
//};
//static struct PROTOnameStruct *PROTONames = NULL;
//#else
extern int curProtoInsStackInd;

// JAS extern struct PROTOInstanceEntry ProtoInstanceTable[PROTOINSTANCE_MAX_LEVELS];

/* PROTO table */
extern struct PROTOnameStruct *PROTONames;
//#endif

//=======
#ifndef __LIBFREEWRL_DECL_H__
#define __LIBFREEWRL_DECL_H__


#ifdef FREEWRL_THREAD_COLORIZED

/* FreeWRL will try to output color is stdout is a terminal */
#define PRINTF printf_with_colored_threads
extern int printf_with_colored_threads(const char *format, ...);
#define FPRINTF fprintf_with_colored_threads
extern int fprintf_with_colored_threads(FILE *stream, const char *format, ...);

#else  /* FREEWRL_THREAD_COLORIZED */

#define PRINTF printf
#define FPRINTF fprintf

#endif /* FREEWRL_THREAD_COLORIZED */

#define BOOL_STR(b) (b ? "TRUE" : "FALSE")

/* Useful to suppress things from non-debug builds */
#if defined(FW_DEBUG)
#  define DEBUG_(_expr) _expr
#else
#  define DEBUG_(_expr)
#endif

#include <stdio.h>

void fw_perror(FILE *f, const char *format, ...);

/* To conform C99 ISO C (do not use GCC extension) */
#if defined(_MSC_VER) && _MSC_VER < 1500
//vc7 cant seem to do the ... thing or the __VAR_ARGS__ thing.
int DEBUG_FPRINTF(const char *fmt, ...); //almost stubs it out - a function call and return
#define DEBUG_MSG DEBUG_FPRINTF
#define TRACE_MSG DEBUG_FPRINTF
#define WARN_MSG DEBUG_FPRINTF
#define ERROR_MSG DEBUG_FPRINTF
#define PERROR_MSG DEBUG_FPRINTF
#define DEBUG_MEM DEBUG_FPRINTF
#define DEBUG_RENDER DEBUG_FPRINTF
#define DEBUG_TEX DEBUG_FPRINTF
#define DEBUG_X3DPARSER DEBUG_FPRINTF
#define DEBUG_SHADER DEBUG_FPRINTF
#define DEBUG_RES DEBUG_FPRINTF
#define DEBUG_CPARSER DEBUG_FPRINTF

/* //would implement it for vc7 but long winded
int DEBUG_MSG(const char *fmt, ...)
{ 
	int ret = 0;
#ifdef VERBOSE
	va_list args;
	va_start( args, fmt );

	ret = ConsoleMessage("FW:"); 
	ret = ConsoleMessage(fmt,args); 
#endif
	return ret;
}
*/

#endif

#if  !_MSC_VER || _MSC_VER >= 1500
#define DEBUG_MSG(...) DEBUG_(FPRINTF(stdout, __VA_ARGS__))
#define TRACE_MSG(...) DEBUG_(FPRINTF(stdout, __VA_ARGS__))
#define WARN_MSG(...)  DEBUG_(FPRINTF(stdout, __VA_ARGS__))
#define ERROR_MSG(...) DEBUG_(FPRINTF(stderr, __VA_ARGS__))
#define PERROR_MSG(...) DEBUG_(fw_perror(stderr, __VA_ARGS__))
#ifdef VERBOSE
#define DEBUG_FW(...) DEBUG_(PRINTF("FW: " __VA_ARGS__))
#else
#define DEBUG_FW(...)
#endif
#ifdef RESVERBOSE 
#define DEBUG_RES(...) DEBUG_(PRINTF("RES: " __VA_ARGS__))
#else
#define DEBUG_RES(...)
#endif

#ifdef TEXVERBOSE
#define DEBUG_TEX(...) DEBUG_(PRINTF("TEXTURE: " __VA_ARGS__))
#else
#define DEBUG_TEX(...)
#endif

#ifdef MEMVERBOSE
#define DEBUG_MEM(...) DEBUG_(PRINTF("MEM: " __VA_ARGS__))
#else
#define DEBUG_MEM(...)
#endif

#ifdef CPARSERVERBOSE
#define DEBUG_CPARSER(...) DEBUG_(PRINTF("CPARSER: " __VA_ARGS__))
#else
#define DEBUG_CPARSER(...)
#endif

#ifdef CPROTOVERBOSE
#define DEBUG_CPROTO(...) DEBUG_(PRINTF("CPROTO: " __VA_ARGS__))
#else
#define DEBUG_CPROTO(...)
#endif

#ifdef PLUGINSOCKETVERBOSE
#define DEBUG_PLUGINSOCKET(...) DEBUG_(PRINTF("PLUGINSOCKET: " __VA_ARGS__))
#else
#define DEBUG_PLUGINSOCKET(...)
#endif

/* FIXME: replace CR with CROUTE for clarity */
#ifdef CRVERBOSE
#define DEBUG_CR(...) DEBUG_(PRINTF("CR: " __VA_ARGS__))
#else
#define DEBUG_CR(...)
#endif

/* FIXME: maybe shorten the def here to make it more practical */
#ifdef JSVRMLCLASSESVERBOSE
#define DEBUG_JSVRMLCLASSES(...) DEBUG_(PRINTF("JSVRMLCLASSES: " __VA_ARGS__))
#else
#define DEBUG_JSVRMLCLASSES(...)
#endif

#ifdef JAVASCRIPTVERBOSE
#define DEBUG_JS(...) DEBUG_(PRINTF("JS: " __VA_ARGS__))
#else
#define DEBUG_JS(...)
#endif

#ifdef SETFIELDVERBOSE
#define DEBUG_SETFIELD(...) DEBUG_(PRINTF("SETFIELD: " __VA_ARGS__))
#else
#define DEBUG_SETFIELD(...)
#endif

#ifdef RENDERVERBOSE
#define DEBUG_RENDER(...) DEBUG_(PRINTF("RENDER: " __VA_ARGS__))
#else
#define DEBUG_RENDER(...)
#endif

#ifdef CHILDVERBOSE
#define DEBUG_CHILD(...) DEBUG_(PRINTF("CHILD: " __VA_ARGS__))
#else
#define DEBUG_CHILD(...)
#endif

/* FIXME: maybe replace SE with SENSOR for clarity */
#ifdef SEVERBOSE
#define DEBUG_SE(...) DEBUG_(PRINTF("SE: " __VA_ARGS__))
#else
#define DEBUG_SE(...)
#endif

#ifdef STREAM_POLY_VERBOSE
#define DEBUG_STREAM_POLY(...) DEBUG_(PRINTF("STREAM_POLY: " __VA_ARGS__))
#else
#define DEBUG_STREAM_POLY(...)
#endif

#ifdef MIDIVERBOSE
#define DEBUG_MIDI(...) DEBUG_(PRINTF("MIDI: " __VA_ARGS__))
#else
#define DEBUG_MIDI(...)
#endif

#ifdef OCCLUSIONVERBOSE
#define DEBUG_OCCLUSION(...) DEBUG_(PRINTF("OCCLUSION: " __VA_ARGS__))
#else
#define DEBUG_OCCLUSION(...)
#endif

#ifdef FRUSTUMVERBOSE
#define DEBUG_FRUSTUM(...) DEBUG_(PRINTF("FRUSTUM: " __VA_ARGS__))
#else
#define DEBUG_FRUSTUM(...)
#endif

#ifdef SHADERVERBOSE
#define DEBUG_SHADER(...) DEBUG_(PRINTF("SHADER: " __VA_ARGS__))
#else
#define DEBUG_SHADER(...)
#endif

#ifdef BINDVERBOSE
#define DEBUG_BIND(...) DEBUG_(PRINTF("BIND: " __VA_ARGS__))
#else
#define DEBUG_BIND(...)
#endif
//>>>>>>> 1.44

#ifdef X3DPARSERVERBOSE
#define DEBUG_X3DPARSER(...) DEBUG_(PRINTF("X3DPARSER: " __VA_ARGS__))
#else
#define DEBUG_X3DPARSER(...)
#endif

/* FIXME: maybe change CAPABILITIES with X3DCAPS */
#ifdef CAPABILITIESVERBOSE
#define DEBUG_CAPABILITIES(...) DEBUG_(PRINTF("CAPABILITIES: " __VA_ARGS__))
#else
#define DEBUG_CAPABILITIES(...)
#endif

/* FIXME: does this has to do with SENSOR or SEVERBOSE ? */
#ifdef SENSVERBOSE
#define DEBUG_SENS(...) DEBUG_(PRINTF("SENS: " __VA_ARGS__))
#else
#define DEBUG_SENS(...)
#endif

#ifdef SOUNDVERBOSE
#define DEBUG_SOUND(...) DEBUG_(PRINTF("SOUND: " __VA_ARGS__))
#else
#define DEBUG_SOUND(...)
#endif

#ifdef FILLVERBOSE
#define DEBUG_FILL(...) DEBUG_(PRINTF("FILL: " __VA_ARGS__))
#else
#define DEBUG_FILL(...)
#endif

#ifdef TEXTVERBOSE
#define DEBUG_TEXT(...) DEBUG_(PRINTF("TEXT: " __VA_ARGS__))
#else
#define DEBUG_TEXT(...)
#endif

#ifdef ARGSVERBOSE
#define DEBUG_ARGS(...) DEBUG_(PRINTF("TEXT: " __VA_ARGS__))
#else
#define DEBUG_ARGS(...)
#endif

#ifdef XEVENT_VERBOSE
#define DEBUG_XEV(...) DEBUG_(PRINTF("XEV: " __VA_ARGS__))
#else
#define DEBUG_XEV(...)
#endif
#endif //_MSC_VER

/* #define DJTRACK_PICKSENSORS 1  define this in your build */

/**
 * Those macro get defined only when debugging is enabled
 */
#if defined(FW_DEBUG) && defined(DEBUG_MALLOC)

void *freewrlMalloc(int line, char *file, size_t sz, int zeroData);
void *freewrlRealloc(int line, char *file, void *ptr, size_t size);
void freewrlFree(int line, char *file, void *a);
void *freewrlStrdup(int line, char *file, char *str);

# define MALLOC(t,_sz)         ((t)freewrlMalloc(__LINE__, __FILE__, _sz, FALSE))
# define CALLOC(_fill, _sz)  freewrlMalloc(__LINE__, __FILE__, _fill * _sz, TRUE);
# define REALLOC(_a,_b)     freewrlRealloc(__LINE__, __FILE__, _a, _b) 
# define FREE(_ptr)            freewrlFree(__LINE__, __FILE__, _ptr)

# define XALLOC(_type)    (_type *) CALLOC(1, sizeof(_type))
# define XFREE(_ptr)      {if (_ptr) { FREE(_ptr); _ptr = NULL; }}

# define STRDUP(_a)          freewrlStrdup(__LINE__, __FILE__, _a)

# define UNLINK(_fdd) do { \
		           TRACE_MSG("TRACE: unlink %s at %s:%d\n",_fdd,__FILE__,__LINE__); \
		           unlink (_fdd); \
		      } while (0)

# define TEMPNAM(_dir,_pfx) tempnam(_dir, _pfx); do { \
				TRACE_MSG("TRACE: tempnam %s/%s at %s:%d\n", _dir, _pfx, __FILE__, __LINE__); \
				} while (0)

# define ASSERT(_ptr) do { if (!(_ptr)) { \
                           ERROR_MSG("ERROR: assert failed: %s (%s:%d)\n", #_ptr, __FILE__, __LINE__); } \
                      } while (0)

/* JAS */
#if defined(_MSC_VER)
# define TEMPNAM _tempnam
#else
# define TEMPNAM tempnam
#endif


#else /* defined(FW_DEBUG) && defined(DEBUG_MALLOC) */


# define MALLOC(t,_sz) ((t)malloc(_sz))
# define REALLOC realloc
# define FREE free

# define XALLOC(_type)    (_type *) calloc(1, sizeof(_type))
# define XFREE(_ptr)      {if (_ptr) { free(_ptr); _ptr = NULL; }}

# define STRDUP strdup
# define UNLINK unlink
# define TEMPNAM tempnam

# define ASSERT(_whatever)

#endif /* defined(FW_DEBUG) && defined(DEBUG_MALLOC) */

/* This get always defined, but ERROR_MSG is no-op without _DEBUG */

#define FREE_IF_NZ(_ptr) if (_ptr) { \
                             FREE(_ptr); \
                             _ptr = 0; } \
                         else { \
                             DEBUG_MEM("double free: %s:%d\n", __FILE__, __LINE__); \
                         }


/* New ptr/string guarded code:
   this macro free the original pointed ptr (void* or char*)
*/
#define PTR_REPLACE(_ptr,_newptr) do {					\
	if (_ptr != _newptr) {						\
	if (_ptr) {							\
		DEBUG_MSG("replacing ptr %p with %p\n", _ptr, _newptr); \
		FREE(_ptr);						\
		_ptr = _newptr;						\
	} else {							\
	DEBUG_MSG("ptr newly assigned value %p\n", _newptr);		\
	_ptr = _newptr;							\
	}								\
	} else {							\
		DEBUG_MSG("replacing ptr with the same value (warning)\n"); \
	} } while (0)

#define PTR_REPLACE_DUP(_ptr,_newptr) do {				\
	if (_ptr != _newptr) {						\
	if (_ptr) {							\
		DEBUG_MSG("replacing ptr %p with %p\n", _ptr, _newptr); \
		FREE(_ptr);						\
		_ptr = STRDUP(_newptr);					\
	} else {							\
	DEBUG_MSG("ptr newly assigned value %p\n", _newptr);		\
	_ptr = STRDUP(_newptr);						\
	}								\
	} else {							\
		DEBUG_MSG("replacing ptr with the same value (warning)\n"); \
	} } while (0)


/* THIS HAS TO BE FIXED TOO :) */

#if defined(_MSC_VER)
/* FIXME: investigate on this... (michel) */
#include <stddef.h> /* for offsetof(...) */
/* textures.c > jpeg > jmorecfg.h tries to redefine booleand but you can say you have it */
#define HAVE_BOOLEAN 1    
#define M_PI acos(-1.0)
#endif

#ifdef IPHONE
#include <stddef.h>
#define HAVE_BOOLEAN 1    
#define M_PI acos(-1.0)
#endif

/* Move those to a better place: */
/* OLDCODE: void initialize_parser(); */

/* Global FreeWRL options (will become profiles ?) */

extern bool global_strictParsing;       /* are we doing "strict" parsing, 
                                           as per FreeX3D, or "loose" parsing, 
                                           as per FreeWRL ? */

extern bool global_plugin_print;        /* are we printing messages to a file 
                                           because we are running as a plugin ? */

extern bool global_occlusion_disable;   /* do we disable all occlusion query
				           calls in the renderer ? */

extern unsigned global_texture_size;    /* do we manually set up the texture
                                           size ? */

extern bool global_print_opengl_errors; /* print OpenGL errors as they come ? */

extern bool global_trace_threads;       /* trace thread creation / switch ... ? */
extern bool global_use_VBOs;       /* try and use VBOs rather than vertex arrays for geometry */

#endif /* __LIBFREEWRL_DECL_H__ */
