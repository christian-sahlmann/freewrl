/*
=INSERT_TEMPLATE_HERE=

$Id: LoadTextures.h,v 1.6 2009/11/10 10:18:26 couannette Exp $

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


#ifndef __FREEWRL_LOAD_TEXTURES_H__
#define __FREEWRL_LOAD_TEXTURES_H__


/* new functions */
void texture_loader_initialize();
bool is_url(const char *url);
bool load_texture_from_file(struct textureTableIndexStruct* this_tex, char *filename);
bool bind_texture(struct X3D_ImageTexture *node);


#endif /* __FREEWRL_LOAD_TEXTURES_H__ */
