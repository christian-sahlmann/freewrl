/*
  $Id: LoadTextures.c,v 1.17 2009/11/27 18:36:34 crc_canada Exp $

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
#ifdef _MSC_VER
#include "gdiPlusImageLoader.h"
#else
#include <Imlib2.h>
#endif


/* is the texture thread up and running yet? */
int TextureThreadInitialized = FALSE;

/* are we currently active? */
int TextureParsing = FALSE;

/* list of texture table entries to load */
s_list_t *texture_list = NULL;

/* defaultBlankTexture... */
GLuint defaultBlankTexture;

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
 *   texture_load_from_pixelTexture: have a PixelTexture node,
 *                           load it now.
 */
static bool texture_load_from_pixelTexture (struct textureTableIndexStruct* this_tex, struct X3D_PixelTexture *node)
{

/* load a PixelTexture that is stored in the PixelTexture as an MFInt32 */
	int hei,wid,depth;
	unsigned char *texture;
	int count;
	int ok;
	int *iptr;
	int tctr;

	iptr = node->image.p;
	this_tex->status = TEX_NEEDSBINDING;

	ok = TRUE;

	/* are there enough numbers for the texture? */
	if (node->image.n < 3) {
		printf ("PixelTexture, need at least 3 elements, have %d\n",node->image.n);
		ok = FALSE;
	} else {
		wid = *iptr; iptr++;
		hei = *iptr; iptr++;
		depth = *iptr; iptr++;

		if ((depth < 0) || (depth >4)) {
			printf ("PixelTexture, depth %d out of range, assuming 1\n",(int) depth);
			depth = 1;
		}
	
		if ((wid*hei-3) > node->image.n) {
			printf ("PixelTexture, not enough data for wid %d hei %d, have %d\n",
					wid, hei, (wid*hei)-2);
			ok = FALSE;
		}
	}

	/* did we have any errors? if so, create a grey pixeltexture and get out of here */
	if (!ok) {
		char buff[] = {0x70, 0x70, 0x70, 0xff} ; /* same format as ImageTextures - GL_BGRA here */

		this_tex->x = 1;
		this_tex->y = 1;
		this_tex->hasAlpha = FALSE;
		this_tex->texdata = MALLOC(4);
		memcpy (this_tex->texdata, buff, 4);
		return TRUE;
	}

	/* ok, we are good to go here */
	this_tex->x = wid;
	this_tex->y = hei;
	this_tex->hasAlpha = ((depth == 2) || (depth == 4));

	texture = (unsigned char *)MALLOC (wid*hei*4);
	this_tex->texdata = texture; /* this will be freed when texture opengl-ized */

	tctr = 0;
	for (count = 0; count < (wid*hei); count++) {
		switch (depth) {
			case 1: {
				   texture[tctr++] = *iptr & 0xff;
				   texture[tctr++] = *iptr & 0xff;
				   texture[tctr++] = *iptr & 0xff;
				   texture[tctr++] = 0xff; /*alpha, but force it to be ff */
				   break;
			   }
			case 2: {
				   texture[tctr++] = (*iptr>>8) & 0xff;	 /*G*/
				   texture[tctr++] = (*iptr>>8) & 0xff;	 /*G*/
				   texture[tctr++] = (*iptr>>8) & 0xff;	 /*G*/
				   texture[tctr++] = (*iptr>>0) & 0xff; /*A*/
				   break;
			   }
			case 3: {
				   texture[tctr++] = (*iptr>>0) & 0xff; /*B*/
				   texture[tctr++] = (*iptr>>8) & 0xff;	 /*G*/
				   texture[tctr++] = (*iptr>>16) & 0xff; /*R*/
				   texture[tctr++] = 0xff; /*alpha, but force it to be ff */
				   break;
			   }
			case 4: {
				   texture[tctr++] = (*iptr>>8) & 0xff;	 /*B*/
				   texture[tctr++] = (*iptr>>16) & 0xff; /*G*/
				   texture[tctr++] = (*iptr>>24) & 0xff; /*R*/
				   texture[tctr++] = (*iptr>>0) & 0xff; /*A*/
				   break;
			   }
		}
		iptr++;
	}
}



/**
 *   texture_load_from_file: a local filename has been found / downloaded,
 *                           load it now.
 */
static bool texture_load_from_file(struct textureTableIndexStruct* this_tex, char *filename)
{
#ifdef _MSC_VER
	loadImage(this_tex, filename);

#else
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

    this_tex->frames = 1;
    this_tex->x = imlib_image_get_width();
    this_tex->y = imlib_image_get_height();

    this_tex->texdata = (unsigned char *) imlib_image_get_data_for_reading_only(); 
#endif
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
		texture_load_from_pixelTexture(entry,(struct X3D_PixelTexture *)entry->scenegraphNode);
		return TRUE;
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
	if (url != NULL) {

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
	} else {
		ERROR_MSG("Could not load texture, no URL present\n");
	}
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
		
		/* Remove the parsed resource from the list */
		texture_list = ml_delete_self(texture_list, item);
	}
}

void send_texture_to_loader(struct textureTableIndexStruct *entry)
{
	/* Lock access to the resource list */
	pthread_mutex_lock( &mutex_texture_list );
	
	/* Add our texture entry */
	texture_list = ml_append(texture_list, ml_new(entry));

        /* signal that we have data on resource list */
        pthread_cond_signal(&texture_list_condition);
	
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
		
		/* Lock access to the resource list */
		pthread_mutex_lock( &mutex_texture_list );

		/* wait around until we have been signalled */
		pthread_cond_wait (&texture_list_condition, &mutex_texture_list);


		TextureParsing = TRUE;
		
		/* Process all resource list items, whatever status they may have */
		while (texture_list != NULL) {
			ml_foreach(texture_list, texture_process_list(__l));
		}
		
		TextureParsing = FALSE;
		
		/* Unlock the resource list */
		pthread_mutex_unlock( &mutex_texture_list );
	}
}
