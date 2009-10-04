/*
=INSERT_TEMPLATE_HERE=

$Id: LoadTextures.c,v 1.5 2009/10/04 22:36:23 couannette Exp $

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



#include <config.h>
#include <system.h>
#include <system_threads.h>
#include <display.h>
#include <internal.h>

#include "vrml_parser/Structs.h"
#include "main/ProdCon.h"

#include "OpenGL_Utils.h"
#include "LoadTextures.h"
#include "Textures.h"

#include <Imlib2.h>

#include <libgen.h> /* dirname */

/* Globals */

s_renderer_capabilities_t rdr_caps;

/* all functions here are named _mb_ and lower case */

/* utility routines */

static void rdr_caps_dump(s_renderer_capabilities_t *rc)
{
    if (!rc) {
	printf("renderer capabilities: NULL\n");
	return;
    } else {
	printf( "renderer capabilities: %p\n"
		" multi textures : %s\n"
		" texture unites : %u\n",
		rc,
		BOOL_STR(rc->av_multitexture),
		rc->texture_units );
    }
}

/* init before threading */

void texture_loader_initialize()
{
    /* init Imlib2, get load capabilities : not much to do ... */

    /* get renderer capabilities 
       interesting caps:
       - multi texture     : true/false
       - texture units     : integer
       - shader GLSL       : true/false
       - max texture size  : integer x integer
       - non PO2 texture   : true/false
    */
    memset(&rdr_caps, 0, sizeof(rdr_caps));

    if (GLEW_ARB_multitexture) {
	rdr_caps.av_multitexture = TRUE;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &rdr_caps.texture_units);
	glPrintError("query texture units");
    }

    /* load default material */

    /* load default texture */

    /* load default shader */

    /* print some debug infos */
    rdr_caps_dump(&rdr_caps);
}

/* thread safe functions */

/* concat two string with a / in between */
char* concat_path(const char *a, const char *b)
{
    char *tmp;
    tmp = MALLOC(strlen(a) + strlen(b) + 1);
    sprintf(tmp, "%s/%s", a, b);
    return tmp;
}

bool is_url(const char *url)
{
#define MAX_PROTOS 3
    static const char *protos[MAX_PROTOS] = { "ftp", "http", "https" };
#define MAX_URL_PROTOCOL_ID 4
    long l;
    unsigned i;
    char *pat;
    pat = strstr(url, "://");
    if (!pat) {
	return FALSE;
    }
    if ((long)(pat-url) > MAX_URL_PROTOCOL_ID) {
	return FALSE;
    }
    l = (long)(pat-url) - 3;
    for (i = 0; i < MAX_PROTOS ; i++) {
	if (strncasecmp(protos[i], url, l) == 0) {
	    return TRUE;
	}
    }
    return FALSE;
}

/*
  Try to find a texture resource (remote or local).
  Returns the filename of the [downloaded] file.
  Caller must free this string.
 */
bool findTextureFile_MB(int cwo)
{
    char *filename;
    int i;

    struct X3D_ImageTexture *tex_node;
    struct Uni_String *tex_node_parent;
    struct Multi_String tex_node_url;

    bool parent_is_url;
    char *parent_path;
    int parent_path_len;

    tex_node = (struct X3D_ImageTexture *) loadThisTexture->scenegraphNode;

    switch (loadThisTexture->nodeType) {
    case NODE_ImageTexture:
	tex_node_parent = tex_node->__parenturl;
	tex_node_url = tex_node->url;
	break;
    default:
	WARN_MSG("findTextureFile_MB: node type = %d not implemented\n", loadThisTexture->nodeType);
	return FALSE;
	break;
    }

    filename = NULL;
    parent_path = NULL;
    parent_path_len = 0;

    parent_is_url = is_url(tex_node_parent->strptr);
    if (!parent_is_url) {
	/* remove filename from path */
	parent_path = dirname(tex_node_parent->strptr);
    } else {
	char *lastslash;
	lastslash = strrchr(tex_node_parent->strptr, '/');
	if (lastslash) {
	    parent_path = strndup(tex_node_parent->strptr, (size_t)(lastslash-tex_node_parent->strptr));
	} else {
	    parent_path = strdup(".");
	}
    }
    parent_path_len = strlen(parent_path);

    /* Loop all strings in Multi_String */
    for (i = 0; i < tex_node_url.n ; i++) {

	char *tmp, *tmp2;

	tmp = (tex_node_url.p[i])->strptr;
	TRACE_MSG("findTextureFile_MB: processing url %d : %s\n", i, tmp);
	DEBUG_MSG("findTextureFile_MB: parent url = %s\n", parent_path);

	/* NB: detect absolute path: X3D error ? */
	if (tmp[0] == '/') {
	    ERROR_MSG("findTextureFile_MB: could not handle absolute path: %s\n", tmp);
	    continue;
	}

	/* this could be an absolute url */
	if (is_url(tmp)) {
	    DEBUG_MSG("findTextureFile_MB: trying URL: %s\n", tmp);
	    filename = do_get_url(tmp);
	    if (filename) {
		DEBUG_MSG("findTextureFile_MB: downloaded file: %s\n", filename);
		break;
	    } else {
		WARN_MSG("findTextureFile_MB: bad URL: %s\n", tmp);
		continue;
	    }
	}

	tmp2 = concat_path(parent_path, tmp);

	/* this is not an absolute url ... try relative url or relative path */
	if (parent_is_url) {
	    /* relative url */
	    if (is_url(tmp2)) {
		DEBUG_MSG("findTextureFile_MB: trying URL: %s\n", tmp2);
		filename = do_get_url(tmp2);
	    }
	} else {
	    /* relative path, concat to parent path */
	    DEBUG_MSG("findTextureFile_MB: trying file path: %s\n", tmp2);
	    if (do_file_exists(tmp2)) {
		filename = strdup(tmp2);
	    }
	}

	FREE(tmp2);

	if (filename) {
	    DEBUG_MSG("findTextureFile_MB: trying file: %s\n", filename);
	    /* is this good file ? */
	    if (load_texture_from_file(tex_node, filename)) {
		DEBUG_MSG("findTextureFile_MB: success: %s\n", filename);
		break;
	    } else {
		ERROR_MSG("findTextureFile_MB: could not load file: %s\n", filename);
	    }
	}

	/* no resource found, try next string */
	FREE(filename);
	filename = NULL;
    }

    FREE(parent_path);

    if (!filename) {
	WARN_MSG("findTextureFile_MB: no resource found for texture %p\n", tex_node);
	return FALSE;
    } else {
	return TRUE;
    }
}

bool load_texture_from_file(struct X3D_ImageTexture *node, char *filename)
{
    Imlib_Image image;

    image = imlib_load_image_immediately(filename);
    if (!image) {
	WARN_MSG("load_texture_from_file: failed to load image: %s\n", filename);
	return FALSE;
    }
    DEBUG_MSG("load_texture_from_file: Imlib2 succeeded to load image: %s\n", filename);

    /* use this image */
    imlib_context_set_image(image);

    /* store actual filename, status, ... */
    loadThisTexture->filename = filename;
    loadThisTexture->status = TEX_NEEDSBINDING;
    loadThisTexture->hasAlpha = (imlib_image_has_alpha() == 1);
    loadThisTexture->imageType = 100; /* not -1, but not PNGTexture neither JPGTexture ... */
    /* query depth with imlib2 ??? */
    loadThisTexture->depth = (loadThisTexture->hasAlpha ? 4 : 3);
    loadThisTexture->frames = 0;
    loadThisTexture->x = imlib_image_get_width();
    loadThisTexture->y = imlib_image_get_height();
    /* this will trigger effective Imlib2 loading */
    loadThisTexture->texdata = (unsigned char *) imlib_image_get_data_for_reading_only();
    
    return TRUE;
}

/* called from the display thread */

bool bind_texture(struct X3D_ImageTexture *node)
{
    /* 
       input:
       node->__textureTableIndex : table entry [ texdata, pixelData ]

       output:
       node->__textureTableIndex : table entry [ OpenGLTexture ]
     */
    return FALSE;
}

