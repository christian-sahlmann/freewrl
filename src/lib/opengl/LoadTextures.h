/*
=INSERT_TEMPLATE_HERE=

$Id: LoadTextures.h,v 1.3 2009/10/01 19:35:36 crc_canada Exp $

 */

/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2009 CRC Canada. (http://www.crc.gc.ca)

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
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


/* Definitions for renderer capabilities */
typedef struct {

    bool av_multitexture; /* Multi textures available */
    bool av_glsl_shaders; /* GLSL shaders available   */ 
    bool av_npot_texture; /* Non power of 2 textures  */
    bool av_texture_rect; /* Rectangle textures */

    int texture_units;
    unsigned max_texture_size[2];

} s_renderer_capabilities_t;

extern s_renderer_capabilities_t rdr_caps;

#define BOOL_STR(_bool) ((_bool) ? "true" : "false")

/* Loading functions */

/* transition function */
bool findTextureFile_MB(int cwo);

/* new functions */
void texture_loader_initialize();
bool is_url(const char *url);
bool load_texture_from_file(struct X3D_ImageTexture *node, char *filename);
bool bind_texture(struct X3D_ImageTexture *node);


#endif /* __FREEWRL_LOAD_TEXTURES_H__ */
