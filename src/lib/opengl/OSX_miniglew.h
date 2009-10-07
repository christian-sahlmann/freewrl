/*
=INSERT_TEMPLATE_HERE=

$Id: OSX_miniglew.h,v 1.2 2009/10/07 19:51:23 crc_canada Exp $

FreeWRL support library.
Internal header: GLEW-style defines for OSX i386, PPC and iPhone dev.

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



#ifndef __LIBFREEWRL_OSX_GLEW_H__
#define __LIBFREEWRL_OSX_GLEW_H__

#ifdef TARGET_AQUA

#include "OpenGL/glu.h"
#define GLEW_OK TRUE

GLenum glewInit(void);
extern int GLEW_ARB_occlusion_query;
extern int GLEW_ARB_multitexture;
extern int GLEW_ARB_fragment_shader;
char *glewGetErrorString(GLenum err);
#endif TARGET_AQUA

#endif /* __LIBFREEWRL_OSX_GLEW_H__ */
