/*
  $Id: LoadTextures.c,v 1.27 2009/12/28 03:00:50 dug9 Exp $

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



/* rewrite MovieTexture loading - for now, just do a blank texture. See:
	HAVE_TO_REIMPLEMENT_MOVIETEXTURES
define */
static bool texture_load_from_MovieTexture (struct textureTableIndexStruct* this_tex)
{
	char buff[] = {0x70, 0x70, 0x70, 0xff} ; /* same format as ImageTextures - GL_BGRA here */
	this_tex->status = TEX_NEEDSBINDING;
	this_tex->x = 1;
	this_tex->y = 1;
	this_tex->hasAlpha = FALSE;
	this_tex->texdata = MALLOC(4);
	memcpy (this_tex->texdata, buff, 4);
	return TRUE;
}


#if defined (TARGET_AQUA)
/* render from aCGImageRef into a buffer, to get EXACT bits, as a CGImageRef contains only
estimates. */
/* from http://developer.apple.com/qa/qa2007/qa1509.html */

static inline double radians (double degrees) {return degrees * M_PI/180;}
CGContextRef CreateARGBBitmapContext (CGImageRef inImage) {
	CGContextRef    context = NULL;
	CGColorSpaceRef colorSpace;
	void *          bitmapData;
	int             bitmapByteCount;
	int             bitmapBytesPerRow;
	CGBitmapInfo	bitmapInfo;
	size_t		bitsPerComponent;

	 // Get image width, height. Well use the entire image.
	size_t pixelsWide = CGImageGetWidth(inImage);
	size_t pixelsHigh = CGImageGetHeight(inImage);

	// Declare the number of bytes per row. Each pixel in the bitmap in this
	// example is represented by 4 bytes; 8 bits each of red, green, blue, and
	// alpha.
	bitmapBytesPerRow   = (pixelsWide * 4);
	bitmapByteCount     = (bitmapBytesPerRow * pixelsHigh);

	// Use the generic RGB color space.
colorSpace = CGColorSpaceCreateDeviceRGB();
	if (colorSpace == NULL)
	{
	    fprintf(stderr, "Error allocating color space\n");
	    return NULL;
	}

	
	/* figure out the bitmap mapping */
	bitsPerComponent = CGImageGetBitsPerComponent(inImage);

	if (bitsPerComponent >= 8) {
		CGRect rect = {{0,0},{pixelsWide, pixelsHigh}};
		bitmapInfo = kCGImageAlphaNoneSkipLast;

bitmapInfo = kCGImageAlphaPremultipliedFirst |
                                kCGBitmapByteOrder32Host;

		/* Allocate memory for image data. This is the destination in memory
		   where any drawing to the bitmap context will be rendered. */
		bitmapData = malloc( bitmapByteCount );
		if (bitmapData == NULL) {
		    fprintf (stderr, "Memory not allocated!");
		    CGColorSpaceRelease( colorSpace );
		    return NULL;
		}
	
		/* Create the bitmap context. We want pre-multiplied ARGB, 8-bits
		  per component. Regardless of what the source image format is
		  (CMYK, Grayscale, and so on) it will be converted over to the format
		  specified here by CGBitmapContextCreate. */
		context = CGBitmapContextCreate (bitmapData, pixelsWide, pixelsHigh,
			bitsPerComponent, bitmapBytesPerRow, colorSpace, bitmapInfo); 
	
		if (context == NULL) {
		    free (bitmapData);
		    fprintf (stderr, "Context not created!");
		} else {
	
			/* try scaling and rotating this image to fit our ideas on life in general */
			CGContextTranslateCTM (context, 0, pixelsHigh);
			CGContextScaleCTM (context,1.0, -1.0);
		}
		CGContextDrawImage(context, rect,inImage);
	} else {
		CGRect rect = {{0,0},{pixelsWide, pixelsHigh}};
		/* this is a mask. */

		printf ("bits per component of %d not handled\n",bitsPerComponent);
		return NULL;
	}

	/* Make sure and release colorspace before returning */
	CGColorSpaceRelease( colorSpace );

	return context;
}
#endif


/**
 *   texture_load_from_file: a local filename has been found / downloaded,
 *                           load it now.
 */
bool texture_load_from_file(struct textureTableIndexStruct* this_tex, char *filename)
{

/* WINDOWS */
#if defined (_MSC_VER)
	/* return FALSE; // to see the default grey image working first */
    if (!loadImage(this_tex, filename)) {
		ERROR_MSG("load_texture_from_file: failed to load image: %s\n", filename);
		return FALSE;
    }


#endif


/* LINUX */
#if !defined (_MSC_VER) && !defined (TARGET_AQUA)
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
    return TRUE;
#endif

/* OSX */
#if defined (TARGET_AQUA)

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>


	CGImageRef 	image;
	CFStringRef	path;
	CFURLRef 	url;
	size_t 		image_width;
	size_t 		image_height;
#ifdef OSX_USE_QUICKTIME
	int useQuicktime = TRUE;
#else
	int useQuicktime = FALSE;
#endif


	CGContextRef 	cgctx;

	/* Quicktime params */
	OSErr 		err;
	GraphicsImportComponent gi;
	Handle 		dataRef;
	OSType 		dataRefType;
	/* end of Quicktime params */

	unsigned char *	data;
	int		hasAlpha;

	CGDataProviderRef provider;
	CGImageSourceRef 	sourceRef;

	/* initialization */
	image = NULL;
	hasAlpha = FALSE;

	path = CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8);
	url = CFURLCreateWithFileSystemPath (NULL, path, kCFURLPOSIXPathStyle, 0);

	/* ok, we can define USE_CG_DATA_PROVIDERS or TRY_QUICKTIME...*/

	/* I dont know whether to use quicktime or not... Probably not... as the other ways using core 
		graphics seems to be ok. Anyway, I left this code in here, as maybe it might be of use for mpegs
	*/
	if (useQuicktime) {
		/* lets let quicktime decide on what to do with this image */
		err = QTNewDataReferenceFromCFURL(url,0, &dataRef, &dataRefType);

		if (dataRef != NULL) {
			err = GetGraphicsImporterForDataRef (dataRef, dataRefType, &gi);
			err = GraphicsImportCreateCGImage (gi, &image, 0);
			DisposeHandle (dataRef);
			CloseComponent(gi);
		}
	} else {
		sourceRef = CGImageSourceCreateWithURL(url,NULL);

		if (sourceRef != NULL) {
			image = CGImageSourceCreateImageAtIndex(sourceRef, 0, NULL);
			CFRelease (sourceRef);
		}
	}

	CFRelease(url);
	CFRelease(path);

	image_width = CGImageGetWidth(image);
	image_height = CGImageGetHeight(image);

	/* go through every possible return value and check alpha. 
		note, in testing, kCGImageAlphaLast and kCGImageAlphaNoneSkipLast
		are what got returned - which makes sense for BGRA textures */
	switch (CGImageGetAlphaInfo(image)) {
		case kCGImageAlphaNone: hasAlpha = FALSE; break;
		case kCGImageAlphaPremultipliedLast: hasAlpha = TRUE; break;
		case kCGImageAlphaPremultipliedFirst: hasAlpha = TRUE; break;
		case kCGImageAlphaLast: hasAlpha = TRUE; break;
		case kCGImageAlphaFirst: hasAlpha = TRUE; break;
		case kCGImageAlphaNoneSkipLast: hasAlpha = FALSE; break;
		case kCGImageAlphaNoneSkipFirst: hasAlpha = FALSE; break;
		default: hasAlpha = FALSE; /* should never get here */
	}

	#ifdef TEXVERBOSE
	printf ("\nLoadTexture %s\n",filename);
	printf ("CGImageGetAlphaInfo(image) returns %x\n",CGImageGetAlphaInfo(image));
	printf ("   kCGImageAlphaNone %x\n",   kCGImageAlphaNone);
	printf ("   kCGImageAlphaPremultipliedLast %x\n",   kCGImageAlphaPremultipliedLast);
	printf ("   kCGImageAlphaPremultipliedFirst %x\n",   kCGImageAlphaPremultipliedFirst);
	printf ("   kCGImageAlphaLast %x\n",   kCGImageAlphaLast);
	printf ("   kCGImageAlphaFirst %x\n",   kCGImageAlphaFirst);
	printf ("   kCGImageAlphaNoneSkipLast %x\n",   kCGImageAlphaNoneSkipLast);
	printf ("   kCGImageAlphaNoneSkipFirst %x\n",   kCGImageAlphaNoneSkipFirst);

	if (hasAlpha) printf ("Image has Alpha channel\n"); else printf ("image - no alpha channel \n");

	printf ("raw image, AlphaInfo %x\n",(int) CGImageGetAlphaInfo(image));
	printf ("raw image, BitmapInfo %x\n",(int) CGImageGetBitmapInfo(image));
	printf ("raw image, BitsPerComponent %d\n",(int) CGImageGetBitsPerComponent(image));
	printf ("raw image, BitsPerPixel %d\n",(int) CGImageGetBitsPerPixel(image));
	printf ("raw image, BytesPerRow %d\n",(int) CGImageGetBytesPerRow(image));
	printf ("raw image, ImageHeight %d\n",(int) CGImageGetHeight(image));
	printf ("raw image, ImageWidth %d\n",(int) CGImageGetWidth(image));
	#endif
	

	/* now, lets "draw" this so that we get the exact bit values */
	cgctx = CreateARGBBitmapContext(image);

	 
	#ifdef TEXVERBOSE
	printf ("GetAlphaInfo %x\n",(int) CGBitmapContextGetAlphaInfo(cgctx));
	printf ("GetBitmapInfo %x\n",(int) CGBitmapContextGetBitmapInfo(cgctx));
	printf ("GetBitsPerComponent %d\n",(int) CGBitmapContextGetBitsPerComponent(cgctx));
	printf ("GetBitsPerPixel %d\n",(int) CGBitmapContextGetBitsPerPixel(cgctx));
	printf ("GetBytesPerRow %d\n",(int) CGBitmapContextGetBytesPerRow(cgctx));
	printf ("GetHeight %d\n",(int) CGBitmapContextGetHeight(cgctx));
	printf ("GetWidth %d\n",(int) CGBitmapContextGetWidth(cgctx));
	#endif
	
#undef TEXVERBOSE
	data = (unsigned char *)CGBitmapContextGetData(cgctx);

	#ifdef TEXVERBOSE
	if (CGBitmapContextGetWidth(cgctx) < 65) {
		int i;

		printf ("dumping image\n");
		for (i=0; i<CGBitmapContextGetBytesPerRow(cgctx)*CGBitmapContextGetHeight(cgctx); i++) {
			printf ("%2x ",data[i]);
		}
		printf ("\n");
	}
	#endif

	/* is there possibly an error here, like a file that is not a texture? */
	if (CGImageGetBitsPerPixel(image) == 0) {
		ConsoleMessage ("texture file invalid: %s",filename);
	}

	if (data != NULL) {
    this_tex->filename = filename;
    this_tex->status = TEX_NEEDSBINDING;
    this_tex->hasAlpha = hasAlpha;
    this_tex->imageType = 100; /* not -1, but not PNGTexture neither JPGTexture ... */

    this_tex->frames = 1;
    this_tex->x = image_width;
    this_tex->y = image_height;

    this_tex->texdata = data;

	}

	CGContextRelease(cgctx);
	return TRUE;
#endif

	return FALSE;
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
	char *parentPath = NULL;

	DEBUG_TEX("textureThread - working on %p (%s)\n"
		  "which is node %p, nodeType %d status %s, opengltex %u, and frames %d\n",
		  entry, entry->filename, entry->scenegraphNode, entry->nodeType, 
		  texst(entry->status), entry->OpenGLTexture, 
		  entry->frames);
	
	entry->status = TEX_LOADING;
	switch (entry->nodeType) {

	case NODE_PixelTexture:
		texture_load_from_pixelTexture(entry,(struct X3D_PixelTexture *)entry->scenegraphNode);
		return TRUE;
		break;

	case NODE_ImageTexture:
		url = & (((struct X3D_ImageTexture *)entry->scenegraphNode)->url);
		parentPath = ((struct X3D_ImageTexture *)entry->scenegraphNode)->__parenturl->strptr;
		break;

	case NODE_MovieTexture:
		texture_load_from_MovieTexture(entry);
		return TRUE;
#ifdef HAVE_TO_REIMPLEMENT_MOVIETEXTURES
		url = & (((struct X3D_MovieTexture *)entry->scenegraphNode)->url);
		parentPath = ((struct X3D_ImageTexture *)entry->scenegraphNode)->__parenturl->strptr;
		break;
#endif /* HAVE_TO_REIMPLEMENT_MOVIETEXTURES */

	case NODE_VRML1_Texture2:
		url = & (((struct X3D_VRML1_Texture2 *)entry->scenegraphNode)->filename);
		parentPath = ((struct X3D_ImageTexture *)entry->scenegraphNode)->__parenturl->strptr;
		break;

	}

	/* FIXME: very straitforward use of resource API... need rewrite ... */
	if (url != NULL) {

		res = resource_create_multi(url);
		res->media_type = resm_image; /* quick hack */

		resource_identify(root_res, res, parentPath);

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
