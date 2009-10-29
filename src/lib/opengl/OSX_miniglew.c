/*
  $Id: OSX_miniglew.c,v 1.6 2009/10/29 06:29:40 crc_canada Exp $

  FreeWRL support library.
  OpenGL extensions detection and setup.

*/

#ifdef OLDCODE

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

/**
 * On all platforms, when we don't have GLEW, we simulate it.
 * In any case we setup the rdr_capabilities struct.
 */

#include <config.h>
#include <system.h>
#include <system_threads.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#ifdef TARGET_AQUA
# include <OpenGL/OpenGL.h>
# include <OpenGL/CGLTypes.h>
# include <AGL/AGL.h>
# include "OSX_miniglew.h"
# include "../ui/aquaInt.h"
#endif //TARGET_AQUA
#endif /* OLDCODE */
