/*
  $Id: LoadTextures.c,v 1.79 2012/05/24 20:37:59 istakenv Exp $

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

#include "vrml_parser/Structs.h"
#include "main/ProdCon.h"
#include "OpenGL_Utils.h"
#include "Textures.h"
#include "LoadTextures.h"
#include "../scenegraph/Component_CubeMapTexturing.h"

#include <list.h>
#include <resources.h>
#include <io_files.h>
#include <io_http.h>

#include <threads.h>

#include <libFreeWRL.h>

/* We do not want to include Struct.h: enormous file :) */
typedef struct _Multi_String Multi_String;
void Multi_String_print(struct Multi_String *url);

#ifdef _MSC_VER
#include "gdiPlusImageLoader.h"
#else
#if !(defined(TARGET_AQUA) || defined(IPHONE) || defined(_ANDROID) || defined(GLES2))
		#include <Imlib2.h>
	#endif
#endif


#if defined (TARGET_AQUA)

#ifdef IPHONE
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#else
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#endif /* IPHONE */
#endif /* TARGET_AQUA */

///* is the texture thread up and running yet? */
//int TextureThreadInitialized = FALSE;



//GLuint defaultBlankTexture;

typedef struct pLoadTextures{
	s_list_t* texture_request_list;// = NULL;
	bool loader_waiting;// = false;
	/* list of texture table entries to load */
	s_list_t *texture_list;// = NULL;
	/* are we currently active? */
	int TextureParsing; // = FALSE;
}* ppLoadTextures;
void *LoadTextures_constructor(){
	void *v = malloc(sizeof(struct pLoadTextures));
	memset(v,0,sizeof(struct pLoadTextures));
	return v;
}
void LoadTextures_init(struct tLoadTextures *t)
{
	//public
	/* is the texture thread up and running yet? */
	t->TextureThreadInitialized = FALSE;

	//private
	t->prv = LoadTextures_constructor();
	{
		ppLoadTextures p = (ppLoadTextures)t->prv;
		p->texture_request_list = NULL;
		p->loader_waiting = false;
		/* list of texture table entries to load */
		p->texture_list = NULL;
		/* are we currently active? */
		p->TextureParsing = FALSE;
	}
}
//s_list_t* texture_request_list = NULL;
//bool loader_waiting = false;


/* All functions here works with the array of 'textureTableIndexStruct'.
 * In the future we may want to refactor this struct.
 * In the meantime lets make it work :).
 */

#ifdef TEXVERBOSE
static void texture_dump_entry(textureTableIndexStruct_s *entry)
{
	DEBUG_MSG("%s\t%p\t%s\n", texst(entry->status), entry, entry->filename);
}
#endif

void texture_dump_list()
{
#ifdef TEXVERBOSE
	DEBUG_MSG("TEXTURE: wait queue\n");
	ppLoadTextures p = (ppLoadTextures)gglobal()->LoadTextures.prv
	ml_foreach(p->texture_list, texture_dump_entry(ml_elem(__l)));
	DEBUG_MSG("TEXTURE: end wait queue\n");
#endif
}


/**
 *   texture_load_from_pixelTexture: have a PixelTexture node,
 *                           load it now.
 */
static void texture_load_from_pixelTexture (textureTableIndexStruct_s* this_tex, struct X3D_PixelTexture *node)
{

/* load a PixelTexture that is stored in the PixelTexture as an MFInt32 */
	int hei,wid,depth;
	unsigned char *texture;
	int count;
	int ok;
	int *iptr;
	int tctr;

	iptr = node->image.p;

	ok = TRUE;

	DEBUG_TEX ("start of texture_load_from_pixelTexture...\n");

	/* are there enough numbers for the texture? */
	if (node->image.n < 3) {
		printf ("PixelTexture, need at least 3 elements, have %d\n",node->image.n);
		ok = FALSE;
	} else {
		wid = *iptr; iptr++;
		hei = *iptr; iptr++;
		depth = *iptr; iptr++;

		DEBUG_TEX ("wid %d hei %d depth %d\n",wid,hei,depth);

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
		return;
	}

	/* ok, we are good to go here */
	this_tex->x = wid;
	this_tex->y = hei;
	this_tex->hasAlpha = ((depth == 2) || (depth == 4));

	texture = MALLOC (unsigned char *, wid*hei*4);
	this_tex->texdata = texture; /* this will be freed when texture opengl-ized */
	this_tex->status = TEX_NEEDSBINDING;

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
static void texture_load_from_MovieTexture (textureTableIndexStruct_s* this_tex)
{
}


#if defined (TARGET_AQUA)
/* render from aCGImageRef into a buffer, to get EXACT bits, as a CGImageRef contains only
estimates. */
/* from http://developer.apple.com/qa/qa2007/qa1509.html */

static inline double radians (double degrees) {return degrees * M_PI/180;} 

int XXX;

CGContextRef CreateARGBBitmapContext (CGImageRef inImage) {

	CGContextRef    context = NULL;
	CGColorSpaceRef colorSpace;
	void *          bitmapData;
	int             bitmapByteCount;
	int             bitmapBytesPerRow;
	CGBitmapInfo	bitmapInfo;
	size_t		bitsPerComponent;

	 // Get image width, height. Well use the entire image.
	int pixelsWide = (int) CGImageGetWidth(inImage);
	int pixelsHigh = (int) CGImageGetHeight(inImage);

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

		bitmapInfo = kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host;

		/* Allocate memory for image data. This is the destination in memory
		   where any drawing to the bitmap context will be rendered. */
		bitmapData = MALLOC(void *, bitmapByteCount );

		if (bitmapData == NULL) {
		    fprintf (stderr, "Memory not allocated!");
		    CGColorSpaceRelease( colorSpace );
		    return NULL;
		}
		bzero (bitmapData, bitmapByteCount);
	
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
		printf ("bits per component of %d not handled\n",(int) bitsPerComponent);
		return NULL;
	}

	/* Make sure and release colorspace before returning */
	CGColorSpaceRelease( colorSpace );

	return context;
}
#endif

#ifdef QNX
#include <img/img.h>
static img_lib_t  ilib = NULL;
int loadImage(textureTableIndexStruct_s* tti, char* fname)
{
	int ierr, iret;
	img_t img;
	if(!ilib) ierr = img_lib_attach( &ilib );
	img.format = IMG_FMT_PKLE_ARGB8888;  //GLES2 little endian 32bit - saw in sample code, no idea
	img.flags |= IMG_FORMAT;
	ierr= img_load_file(ilib, fname, NULL, &img);
	iret = 0;
	if(ierr == NULL)
	{

		//deep copy data so browser owns it (and does its FREE_IF_NZ) and we can delete our copy here and forget about it
		tti->x = img.w;
		tti->y = img.h;
		tti->frames = 1;
		tti->texdata = img.access.direct.data;
		if(!tti->texdata)
		   printf("ouch in gdiplus image loader L140 - no image data\n");
		else
		{
			int flipvertically = 1;
			if(flipvertically){
				int i,j,ii,rowcount;
				unsigned char *sourcerow, *destrow;
				unsigned char * blob;
				rowcount = tti->x * 4;
				blob = malloc(img.h * rowcount);
				for(i=0;i<img.h;i++) {
					ii = tti->y - 1 - i;
					sourcerow = &tti->texdata[i*rowcount];
					destrow = &blob[ii*rowcount];
					memcpy(destrow,sourcerow,rowcount);
				}
				tti->texdata = blob;
			}
		}
		tti->hasAlpha = 1; //img.transparency; //Gdiplus::IsAlphaPixelFormat(bitmap->GetPixelFormat())?1:0;
		//printf("fname=%s alpha=%ld\n",fname,tti->hasAlpha);
		iret = 1;
	}
	return iret;
}

#endif
/**
 *   texture_load_from_file: a local filename has been found / downloaded,
 *                           load it now.
 */
char* download_file(char* filename);

#if defined (_ANDROID)
/* do a load using libjpeg for now */

/*
 * memsrc.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains decompression data source routines for the case of
 * reading JPEG data from a memory buffer that is preloaded with the entire
 * JPEG file.  This would not seem especially useful at first sight, but
 * a number of people have asked for it.
 * This is really just a stripped-down version of jdatasrc.c.  Comparison
 * of this code with jdatasrc.c may be helpful in seeing how to make
 * custom source managers for other purposes.
 */

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"


/* Expanded data source object for memory input */

typedef struct {
  struct jpeg_source_mgr pub;	/* public fields */

  JOCTET eoi_buffer[2];		/* a place to put a dummy EOI */
} my_source_mgr;

typedef my_source_mgr * my_src_ptr;


/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

METHODDEF(void)
init_source (j_decompress_ptr cinfo)
{
  /* No work, since jpeg_memory_src set up the buffer pointer and count.
   * Indeed, if we want to read multiple JPEG images from one buffer,
   * this *must* not do anything to the pointer.
   */
}


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In this application, this routine should never be called; if it is called,
 * the decompressor has overrun the end of the input buffer, implying we
 * supplied an incomplete or corrupt JPEG datastream.  A simple error exit
 * might be the most appropriate response.
 *
 * But what we choose to do in this code is to supply dummy EOI markers
 * in order to force the decompressor to finish processing and supply
 * some sort of output image, no matter how corrupted.
 */

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  WARNMS(cinfo, JWRN_JPEG_EOF);

  /* Create a fake EOI marker */
  src->eoi_buffer[0] = (JOCTET) 0xFF;
  src->eoi_buffer[1] = (JOCTET) JPEG_EOI;
  src->pub.next_input_byte = src->eoi_buffer;
  src->pub.bytes_in_buffer = 2;

  return TRUE;
}


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * If we overrun the end of the buffer, we let fill_input_buffer deal with
 * it.  An extremely large skip could cause some time-wasting here, but
 * it really isn't supposed to happen ... and the decompressor will never
 * skip more than 64K anyway.
 */

METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  if (num_bytes > 0) {
    while (num_bytes > (long) src->pub.bytes_in_buffer) {
      num_bytes -= (long) src->pub.bytes_in_buffer;
      (void) fill_input_buffer(cinfo);
      /* note we assume that fill_input_buffer will never return FALSE,
       * so suspension need not be handled.
       */
    }
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}


/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF(void)
term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}


/*
 * Prepare for input from a memory buffer.
 */

GLOBAL(void)
jpeg_memory_src (j_decompress_ptr cinfo, const JOCTET * buffer, size_t bufsize)
{
  my_src_ptr src;

  /* The source object is made permanent so that a series of JPEG images
   * can be read from a single buffer by calling jpeg_memory_src
   * only before the first one.
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if (cinfo->src == NULL) {	/* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  SIZEOF(my_source_mgr));
  }

  src = (my_src_ptr) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;

  src->pub.next_input_byte = buffer;
  src->pub.bytes_in_buffer = bufsize;
}





#endif //ANDROID


bool texture_load_from_file(textureTableIndexStruct_s* this_tex, char *filename)
{

/* Android, put it here... */
#if defined(_ANDROID)
	unsigned char *image = NULL;
	unsigned char *imagePtr;
	int i;

	openned_file_t *myFile = load_file (filename);
	/* printf ("got file from load_file, openned_file_t is %p %d\n", myFile->data, myFile->dataSize); */



	/* if we got null for data, lets assume that there was not a file there */
	if (myFile->data == NULL) {
		return FALSE;
	} else {

		char myline[2000];
		int width, height, bytes_per_pixel;
		struct jpeg_decompress_struct cinfo;
		struct jpeg_error_mgr jerr;
		JSAMPARRAY buffer;

/*
sprintf (myline,"load_texture_from_file, setting source to memory, size %d",myFile->dataSize);
ConsoleMessage (myline);
*/

  		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_decompress(&cinfo);
		jpeg_memory_src(&cinfo, (const char *)myFile->data,myFile->dataSize);

		/* reading the image header which contains image information */
		jpeg_read_header( &cinfo, TRUE );
/*
ConsoleMessage ("load_texture_from_file, read header, here is the results:");
sprintf(myline, "JPEG File width and height: %d pixels and %d pixels, components per pixel: %d, Color space: %d",
		cinfo.image_width, cinfo.image_height, cinfo.num_components, cinfo.jpeg_color_space );
ConsoleMessage(myline);
*/

		/* Start decompression jpeg here */
		jpeg_start_decompress( &cinfo );

		/* save image dimension information */
		width = cinfo.image_width;
		height = cinfo.image_height;
		bytes_per_pixel = cinfo.num_components;

		/* allocate memory to hold the uncompressed image */
		image = MALLOC (unsigned char*, width*height*4 );

		/* store actual filename, status, ... */
		this_tex->filename = filename;
		this_tex->hasAlpha = FALSE;
		this_tex->imageType = 100; /* not -1, but not PNGTexture neither JPGTexture ... */

		this_tex->frames = 1;
		this_tex->x = width;
		this_tex->y = height;

		/* jpeg decompressor buffer */
		buffer = (*cinfo.mem->alloc_sarray)
			((j_common_ptr) &cinfo, JPOOL_IMAGE, cinfo.output_width * cinfo.output_components, 1);

		/* go through image, line by line */
		while (cinfo.output_scanline < cinfo.output_height) {

		/* images have to be flipped vertically */
		imagePtr = offsetPointer_deref(unsigned char*, image, (cinfo.output_height - cinfo.output_scanline - 1) * 4 * cinfo.output_width);

		jpeg_read_scanlines(&cinfo,buffer,1);
		for (i = 0; i < cinfo.output_width; i++) {
			switch (cinfo.output_components) {
				case 3:
					*imagePtr = buffer[0][i*3+0];imagePtr ++;
					*imagePtr = buffer[0][i*3+1];imagePtr ++;
					*imagePtr = buffer[0][i*3+2];imagePtr ++;
					*imagePtr = 0xff; imagePtr ++;
					break;

				case 4:
					*imagePtr = buffer[0][i*3+0];imagePtr ++;
					*imagePtr = buffer[0][i*3+1];imagePtr ++;
					*imagePtr = buffer[0][i*3+2];imagePtr ++;
					*imagePtr = buffer[0][i*3+3];imagePtr ++;
					break;

				default:
					*imagePtr = 0xff; imagePtr++;
					*imagePtr = 0x00; imagePtr++;
					*imagePtr = 0x00; imagePtr++;
					*imagePtr = 0xff; imagePtr++;
			}

		}
		}

		jpeg_finish_decompress (&cinfo);

		/* save the decompressed data here */
		this_tex->texdata = image;

		jpeg_destroy_compress(&cinfo);
		return TRUE;
	}

#endif //ANDROID



/* WINDOWS */
#if defined (_MSC_VER) || defined(GLES2)
	char *fname;
	int ret;

	fname = strdup(filename);
	ret = loadImage(this_tex, fname);
    if (!ret) {
		ERROR_MSG("load_texture_from_file: failed to load image: %s\n", fname);
	}else{
		if(GLES2){
			//swap red and blue
			//search for GL_RGBA in textures.c
			int x,y,i,j,k,m;
			unsigned char R,B,*data;
			x = this_tex->x;
			y = this_tex->y;
			data = this_tex->texdata;
			for(i=0,k=0;i<y;i++)
			{
				for(j=0;j<x;j++,k++)
				{
					m=k*4;
					R = data[m];
					B = data[m+2];
					data[m] = B;
					data[m+2] = R;
				}
			}
		}
	}
	free(fname);
	return (ret != 0);

#endif


/* LINUX */
#if !defined (_MSC_VER) && !defined (TARGET_AQUA) && !defined(_ANDROID) && !defined(GLES2)
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

	CGImageRef 	image;


	int 		image_width;
	int 		image_height;

#ifndef FRONTEND_GETS_FILES
	CFStringRef	path;
    CFURLRef 	url;
#endif
    
	CGContextRef 	cgctx;

	unsigned char *	data;
	int		hasAlpha;

	CGImageSourceRef 	sourceRef;

	/* initialization */
	image = NULL;
	hasAlpha = FALSE;

#ifdef FRONTEND_GETS_FILES
	openned_file_t *myFile = load_file (filename);
	/* printf ("got file from load_file, openned_file_t is %p %d\n", myFile->data, myFile->dataSize); */


	/* if we got null for data, lets assume that there was not a file there */
	if (myFile->data == NULL) {
		sourceRef = NULL;
		image = NULL;
	} else {
		CFDataRef localData = CFDataCreate(NULL,(const UInt8 *)myFile->data,myFile->dataSize);
		sourceRef = CGImageSourceCreateWithData(localData,NULL);
		CFRelease(localData);
	}

	/* step 2, if the data exists, was it a file for us? */
	if (sourceRef != NULL) {
		image = CGImageSourceCreateImageAtIndex(sourceRef, 0, NULL);
		CFRelease (sourceRef);
	}


#else /* FRONTEND_GETS_FILES */

	path = CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8);
	url = CFURLCreateWithFileSystemPath (NULL, path, kCFURLPOSIXPathStyle, 0);

	/* ok, we can define USE_CG_DATA_PROVIDERS or TRY_QUICKTIME...*/

	/* I dont know whether to use quicktime or not... Probably not... as the other ways using core 
		graphics seems to be ok. Anyway, I left this code in here, as maybe it might be of use for mpegs
	*/

	sourceRef = CGImageSourceCreateWithURL(url,NULL);
	if (sourceRef != NULL) {
		image = CGImageSourceCreateImageAtIndex(sourceRef, 0, NULL);
		CFRelease (sourceRef);
	}

	CFRelease(url);
	CFRelease(path);

#endif /* FRONTEND_GETS_FILES */

	/* We were able to load in the image here... */
	if (image != NULL) {
		image_width = (int) CGImageGetWidth(image);
		image_height = (int) CGImageGetHeight(image);
	
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
		
		data = (unsigned char *)CGBitmapContextGetData(cgctx);
	
/*
		#ifdef TEXVERBOSE
		if (CGBitmapContextGetWidth(cgctx) < 301) {
			int i;
	
			printf ("dumping image\n");
			for (i=0; i<CGBitmapContextGetBytesPerRow(cgctx)*CGBitmapContextGetHeight(cgctx); i++) {
				printf ("index:%d data:%2x\n ",i,data[i]);
			}
			printf ("\n");
		}
		#endif
*/
	
		/* is there possibly an error here, like a file that is not a texture? */
		if (CGImageGetBitsPerPixel(image) == 0) {
			ConsoleMessage ("texture file invalid: %s",filename);
		}
	
		if (data != NULL) {
			this_tex->filename = filename;
			this_tex->hasAlpha = hasAlpha;
			this_tex->imageType = 100; /* not -1, but not PNGTexture neither JPGTexture ... */
	
			this_tex->frames = 1;
			this_tex->x = image_width;
			this_tex->y = image_height;
			this_tex->texdata = data;
		}
	
		CGContextRelease(cgctx);
		return TRUE;
	} else {
		/* is this, possibly, a dds file for an ImageCubeMap? */
		return textureIsDDS(this_tex, filename);
	}
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
static bool texture_process_entry(textureTableIndexStruct_s *entry)
{
	resource_item_t *res;
	struct Multi_String *url;
	resource_item_t *parentPath = NULL;

	DEBUG_TEX("textureThread - working on %p (%s)\n"
		  "which is node %p, nodeType %d status %s, opengltex %u, and frames %d\n",
		  entry, entry->filename, entry->scenegraphNode, entry->nodeType, 
		  texst(entry->status), entry->OpenGLTexture, 
		  entry->frames);
	
	entry->status = TEX_LOADING;
	url = NULL;
	res = NULL;

	switch (entry->nodeType) {

	case NODE_PixelTexture:
		texture_load_from_pixelTexture(entry,(struct X3D_PixelTexture *)entry->scenegraphNode);
		return TRUE;
		break;

	case NODE_ImageTexture:
		url = & (((struct X3D_ImageTexture *)entry->scenegraphNode)->url);
		parentPath = (resource_item_t *)(((struct X3D_ImageTexture *)entry->scenegraphNode)->_parentResource);
		break;

	case NODE_MovieTexture:
		texture_load_from_MovieTexture(entry);
		return TRUE;
#ifdef HAVE_TO_REIMPLEMENT_MOVIETEXTURES
		url = & (((struct X3D_MovieTexture *)entry->scenegraphNode)->url);
		parentPath = (resource_item_t *)(((struct X3D_MovieTexture *)entry->scenegraphNode)->_parentResource);
		break;
#endif /* HAVE_TO_REIMPLEMENT_MOVIETEXTURES */

	case NODE_VRML1_Texture2:
		url = & (((struct X3D_VRML1_Texture2 *)entry->scenegraphNode)->filename);
		parentPath = (resource_item_t *)(((struct X3D_VRML1_Texture2 *)entry->scenegraphNode)->_parentResource);
		break;

	case NODE_ImageCubeMapTexture:
		url = & (((struct X3D_ImageCubeMapTexture *)entry->scenegraphNode)->url);
		parentPath = (resource_item_t *)(((struct X3D_ImageCubeMapTexture *)entry->scenegraphNode)->_parentResource);
		break;

	default: {
		printf ("invalid nodetype given to loadTexture, %s is not valid\n",stringNodeType(entry->nodeType));
	}

	}
	if (url != NULL) {
		s_list_t *head_of_list;
#ifdef TEXVERBOSE
		PRINTF("url: ");
		Multi_String_print(url);
		PRINTF("parent resource: \n");
		resource_dump(parentPath);
#endif
		res = resource_create_multi(url);
		/* hold on to the top of the list so we can delete it later */
		head_of_list = res->m_request;
		/* go through the urls until we have a success, or total failure */
		do {
			/* Setup parent */
			resource_identify(parentPath, res);
			/* Setup media type */
			res->media_type = resm_image; /* quick hack */

			if (resource_fetch(res)) {
				DEBUG_TEX("really loading texture data from %s into %p\n", res->actual_file, entry);
				if (texture_load_from_file(entry, res->actual_file)) {
					entry->status = TEX_NEEDSBINDING; /* tell the texture thread to convert data to OpenGL-format */
					res->complete = TRUE;
				}
			} else {
				/* we had a problem with that URL, set this so we can try the next */
				res->type=rest_multi;
			}
		} while ((res->status != ress_downloaded) && (res->m_request != NULL));

		/* destroy the m_request, if it exists */
		if (head_of_list != NULL) {
			ml_delete_all(head_of_list);
		}

	} else {
		ERROR_MSG("Could not load texture, no URL present\n");
	}

	/* really not successful */
	if (res== NULL) return FALSE;


	/* were we successful?? */
	if (res->status != ress_downloaded) {
		ERROR_MSG("Could not load texture: %s\n", entry->filename);
		return FALSE;
	} else {
		return TRUE;
	}
	return FALSE;
}

/*
parsing thread --> texture_loading_thread hand-off
GOAL: texture thread blocks when no textures requested. (rather than sleep(500) and for(;;) )
IT IS AN ERROR TO CALL (condition signal) before calling (condition wait). 
So you might have a global variable bool waiting = false.
1. The threads start, list=null, waiting=false
2. The texture thread loops to lock_mutex line, checks if list=null, 
   if so it sets waiting = true, and sets condition wait, and blocks, 
   waiting for the main thread to give it some texure names
3. The parsing/main thread goes to schedule a texture. It mutex locks, 
   list= add new item. it checks if textureloader is waiting, 
   if so signals condition (which locks momentarily blocks while 
   other thread does something to the list) then unlock mutex.
4. The texture thread gets a signal its waiting on. it copies the list and sets it null, 
   sets waiting =false, and unlocks and does its loading work 
   (on its copy of the list), and goes back around to 2.

*/

/**
 *   texture_process_list: walk through the list of texture we have to process.
 */
static void texture_process_list(s_list_t *item)
{
	bool remove_it = FALSE;
	textureTableIndexStruct_s *entry;
	ppLoadTextures p = (ppLoadTextures)gglobal()->LoadTextures.prv;

	if (!item || !item->elem)
		return;
	
	entry = ml_elem(item);
	
	DEBUG_TEX("texture_process_list: %s\n", entry->filename);
	
	/* FIXME: it seems there is no case in which we not want to remote it ... */

	switch (entry->status) {
	
	/* JAS - put in the TEX_LOADING flag here - it helps on OSX */
	case TEX_LOADING:
	case TEX_NOTLOADED:
		if (texture_process_entry(entry)) {
			remove_it = TRUE;
		}else{
			remove_it = TRUE; //still remove it 
			// url doesn't exist (or none of multi-url exist)
			// no point in trying again, 
			// you'll just get the same result in a vicious cycle
		}
		break;
		
	default:
		//DEBUG_MSG("Could not process texture entry: %s\n", entry->filename);
		remove_it = TRUE;
		break;
	}
		
	if (remove_it) {
		/* Remove the parsed resource from the list */
		p->texture_list = ml_delete_self(p->texture_list, item);
	}
}
void send_texture_to_loader(textureTableIndexStruct_s *entry)
{
	ppLoadTextures p = (ppLoadTextures)gglobal()->LoadTextures.prv;

	/* Lock access to the texture_request_list and loader_waiting variables*/
	pthread_mutex_lock( &gglobal()->threads.mutex_texture_list );
	
	/* Add our texture entry */
	p->texture_request_list = ml_append(p->texture_request_list, ml_new(entry));

	if(p->loader_waiting)
        /* signal that we have data on resource list */
        pthread_cond_signal(&gglobal()->threads.texture_list_condition);
	
	/* Unlock */
	pthread_mutex_unlock( &gglobal()->threads.mutex_texture_list );
}

/**
 *   _textureThread: work on textures, until the end of time.
 */
void _textureThread()
{
	ENTER_THREAD("texture loading");
	{

		ppLoadTextures p;
		ttglobal tg = gglobal();
		p = (ppLoadTextures)tg->LoadTextures.prv;

		tg->LoadTextures.TextureThreadInitialized = TRUE;

		/* we wait forever for the data signal to be sent */
		for (;;) {
			/* Lock access to the texture_request_list and loader_waiting variables*/
			pthread_mutex_lock( &gglobal()->threads.mutex_texture_list );
			if(p->texture_request_list == NULL)
			{
				p->loader_waiting = true;
				/*block and wait*/
				pthread_cond_wait (&gglobal()->threads.texture_list_condition, &gglobal()->threads.mutex_texture_list);
			}
			p->texture_list = p->texture_request_list;
			p->texture_request_list = NULL;
			p->loader_waiting = false;
			/* Unlock  */
			pthread_mutex_unlock( &gglobal()->threads.mutex_texture_list );


			p->TextureParsing = TRUE;
			
			/* Process all resource list items, whatever status they may have */
			while (p->texture_list != NULL) {
	//printf ("textureThread running\n");
				ml_foreach(p->texture_list, texture_process_list(__l));
			}
			p->TextureParsing = FALSE;
		}
	}
}
