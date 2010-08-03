/*
=INSERT_TEMPLATE_HERE=

$Id: LoadTextures.h,v 1.10 2010/08/03 19:41:12 crc_canada Exp $

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


extern int TextureParsing;
extern int TextureThreadInitialized;
void send_texture_to_loader(textureTableIndexStruct_s *entry);
bool texture_load_from_file(textureTableIndexStruct_s* this_tex, char *filename, int imageCount);


#endif /* __FREEWRL_LOAD_TEXTURES_H__ */
