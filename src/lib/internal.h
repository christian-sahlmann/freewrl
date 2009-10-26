/*
  $Id: internal.h,v 1.23 2009/10/26 10:57:07 couannette Exp $

  FreeWRL support library.
  Library internal declarations.

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


#ifndef __LIBFREEWRL_DECL_H__
#define __LIBFREEWRL_DECL_H__


#include "fwdebug.h"


#if defined(_MSC_VER)
/* FIXME: investigate on this... (michel) */
#include <stddef.h> /* for offsetof(...) */
/* textures.c > jpeg > jmorecfg.h tries to redefine booleand but you can say you have it */
#define HAVE_BOOLEAN 1    
#define M_PI acos(-1.0)
#endif

/* Move those to a better place: */
void initialize_parser();

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


#endif /* __LIBFREEWRL_DECL_H__ */
