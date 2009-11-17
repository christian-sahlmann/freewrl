/*
  $Id: LoadTextures.c,v 1.12 2009/11/17 08:49:07 couannette Exp $

  FreeWRL support library.
  New implementation of texture loading.

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

#include <list.h>

#include <io_files.h>
#include <io_http.h>

#include <resources.h>

#include <threads.h>

#include "vrml_parser/Structs.h"
#include "main/ProdCon.h"

#include "OpenGL_Utils.h"
#include "Textures.h"
#include "LoadTextures.h"

#include <Imlib2.h>


/* is the texture thread up and running yet? */
int TextureThreadInitialized = FALSE;

/* are we currently active? */
int TextureParsing = FALSE;

/* list of texture table entries to load */
s_list_t *texture_list = NULL;


/* All functions here works with the array of 'textureTableIndexStruct'.
 * In the future we may want to refactor this struct.
 * In the meantime lets make it work :).
 */

static void texture_dump_entry(struct textureTableIndexStruct *entry)
{
	DEBUG_TEX("%s\t%p\t%s\n", texst(entry->status), entry, entry->filename);
}

static void texture_dump_list()
{
#ifdef TEXVERBOSE
	DEBUG_MSG("Texture wait queue:\n");
	ml_foreach(texture_list, texture_dump_entry(ml_elem(__l)));
	DEBUG_MSG(".\n");
#endif
}

/**
 *   texture_load_from_file: a local filename has been found / downloaded,
 *                           load it now.
 */
static bool texture_load_from_file(struct textureTableIndexStruct* this_tex, char *filename)
{
    Imlib_Image image;

    image = imlib_load_image_immediately(filename);
    if (!image) {
	ERROR_MSG("load_texture_from_file: failed to load image: %s\n", filename);
	return FALSE;
    }
    DEBUG_TEX("load_texture_from_file: Imlib2 succeeded to load image: %s\n", filename);

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

/**
 *   texture_process_entry: process a texture table entry
 *
 * find the file, either locally or within the Browser. Note that
 * this is almost identical to the one for Inlines, but running
 * in different threads 
 */
static bool texture_process_entry(struct textureTableIndexStruct *entry)
{
	resource_item_t *res;
	struct Multi_String *url;

	DEBUG_TEX("textureThread - working on %p (%s)\n"
		  "which is node %p, nodeType %d status %s, opengltex %u, and frames %d\n",
		  entry, entry->filename, entry->scenegraphNode, entry->nodeType, 
		  texst(entry->status), entry->OpenGLTexture, 
		  entry->frames);
	
	entry->status = TEX_LOADING;
	
	/* look for the file. If one does not exist, or it
	   is a duplicate, just unlock and return */


	/* FIXME: we should have a generic point to the current node, clean-up this mess */
	switch (entry->nodeType) {

	case NODE_PixelTexture:
		/* FIXME: ??? */
		url = NULL;
		break;

	case NODE_ImageTexture:
		url = & (((struct X3D_ImageTexture *)entry->scenegraphNode)->url);
		break;

	case NODE_MovieTexture:
		url = & (((struct X3D_MovieTexture *)entry->scenegraphNode)->url);
		break;

	case NODE_VRML1_Texture2:
		url = & (((struct X3D_VRML1_Texture2 *)entry->scenegraphNode)->filename); /*FIXME: !!!! */
		break;

	}

	/* FIXME: very straitforward use of resource API... need rewrite ... */

	res = resource_create_multi(url);
	res->media_type = resm_image; /* quick hack */

	resource_identify(root_res, res);
	if (resource_fetch(res)) {
		if (texture_load_from_file(entry, res->actual_file)) {
			res->status = ress_loaded;
		}
	}

	if (res->status == ress_loaded) {
		/* Cool :) */
		DEBUG_TEX("%s texture loaded (file downloaded and loaded into memory): we should create the OpenGL texture...\n", res->request);
		res->complete = TRUE;
		entry->status = TEX_NEEDSBINDING;
		return TRUE;
	}

	ERROR_MSG("Could not load texture: %s\n", entry->filename);
	return FALSE;
}

/**
 *   texture_process_list: walk through the list of texture we have to process.
 */
static void texture_process_list(s_list_t *item)
{
	bool remove_it = FALSE;
	struct textureTableIndexStruct *entry;
	
	if (!item || !item->elem)
		return;
	
	entry = ml_elem(item);
	
	DEBUG_TEX("texture_process_list: %s\n", entry->filename);
	
	/* FIXME: it seems there is no case in which we not want to remote it ... */

	switch (entry->status) {
		
	case TEX_NOTLOADED:
		if (texture_process_entry(entry)) {
			remove_it = TRUE;
		}
		break;
		
	default:
		DEBUG_MSG("Could not process texture entry: %s\n", entry->filename);
		remove_it = TRUE;
		break;
	}
	
	
	if (remove_it) {
		texture_dump_list();
		
		/* Lock access to the resource list */
		pthread_mutex_lock( &mutex_texture_list );
		
		/* Remove the parsed resource from the list */
		texture_list = ml_delete_self(texture_list, item);
		
		/* Unlock the resource list */
		pthread_mutex_unlock( &mutex_texture_list );
	}
}

void send_texture_to_loader(struct textureTableIndexStruct *entry)
{
	/* Lock access to the resource list */
	pthread_mutex_lock( &mutex_texture_list );
	
	/* Add our texture entry */
	texture_list = ml_append(texture_list, ml_new(entry));
	
	/* Unlock the resource list */
	pthread_mutex_unlock( &mutex_texture_list );
}

/**
 *   _textureThread: work on textures, until the end of time.
 */
void _textureThread()
{
	ENTER_THREAD("texture loading");

	TextureThreadInitialized = TRUE;

	/* we wait forever for the data signal to be sent */
	for (;;) {
		
		TextureParsing = TRUE;
		
		/* Process all resource list items, whatever status they may have */
		ml_foreach(texture_list, texture_process_list(__l));
		
		TextureParsing = FALSE;
		
		usleep(50);
	}
}
