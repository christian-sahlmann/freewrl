/*
  $Id: LoadTextures.c,v 1.11 2009/11/10 10:18:26 couannette Exp $

  FreeWRL support library.

  New implementation of the texture thread.
  - Setup renderer capabilities
  - Setup default texture & default shader
  - Routines to load textures from file/URL/inline.
  - ...

  NOTE: a lot of work have to be done here ;*).

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



#include <config.h>
#include <system.h>
#include <system_threads.h>
#include <display.h>
#include <internal.h>

#include <io_files.h>
#include <io_http.h>

#include "vrml_parser/Structs.h"
#include "main/ProdCon.h"

#include "OpenGL_Utils.h"
#include "Textures.h"
#include "LoadTextures.h"
#include "Textures.h"

#include <Imlib2.h>


bool load_texture_from_file(struct textureTableIndexStruct* this_tex, char *filename)
{
    Imlib_Image image;

    image = imlib_load_image_immediately(filename);
    if (!image) {
	WARN_MSG("load_texture_from_file: failed to load image: %s\n", filename);
	return FALSE;
    }
    DEBUG_MSG("load_texture_from_file: Imlib2 succeeded to load image: %s\n", filename);

    imlib_context_set_image(image);
    imlib_image_flip_vertical(); /* FIXME: do we really need this ? */

    /* store actual filename, status, ... */
    this_tex->filename = filename;
    this_tex->status = TEX_NEEDSBINDING;
    this_tex->hasAlpha = (imlib_image_has_alpha() == 1);
    this_tex->imageType = 100; /* not -1, but not PNGTexture neither JPGTexture ... */
    /* FIXME: query depth with imlib2 ??? */
    this_tex->depth = (this_tex->hasAlpha ? 4 : 3);
    this_tex->frames = 1;
    this_tex->x = imlib_image_get_width();
    this_tex->y = imlib_image_get_height();

    this_tex->texdata = (unsigned char *) imlib_image_get_data_for_reading_only();
    /* FIXME: imlib image is not freed - do we need to save memory ? copy this data, then free it ? */

    return TRUE;
}

