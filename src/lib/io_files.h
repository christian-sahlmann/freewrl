/*
  $Id: io_files.h,v 1.9 2012/05/28 20:15:36 crc_canada Exp $

  FreeWRL support library.
  IO with files.

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

#ifndef __LIBFREEWRL_IO_FILES_H__
#define __LIBFREEWRL_IO_FILES_H__

void fwg_frontEndReturningData(unsigned char *dataPointer, int len);
void fwg_frontEndReturningLocalFile(char *localfile, int iret);

/* Path manipulation */
char* concat_path(const char *a, const char *b);
char* remove_filename_from_path(const char *path);

/* Simple functions for file/dir access */
char *get_current_dir();
bool do_file_exists(const char *filename);
bool do_file_readable(const char *filename);
bool do_dir_exists(const char *dir);

typedef struct openned_file {
	const char *fileFileName;
	int fileDescriptor;
	int fileDataSize;
	char *fileData;
} openned_file_t;

void of_dump(openned_file_t *of);
openned_file_t* load_file(const char *filename);

/**
 *   Type of resources : text (VRML/X3D, shaders) or binary (image, movie)
 */

/* VRML/X3D version */
#define IS_TYPE_XML_X3D	   100
#define IS_TYPE_VRML       101
#define IS_TYPE_VRML1      102
#define IS_TYPE_SKETCHUP   103
#define IS_TYPE_KML        104
#define IS_TYPE_COLLADA    105
#define IS_TYPE_UNKNOWN    200

extern int inputFileType;
extern int inputFileVersion[];

int determineFileType(const char *buffer);


/* borrowed from headers.h -- need a clean-up */

/* types to tell the Perl thread what to handle */
#define FROMSTRING 	1
#define	FROMURL		2
#define INLINE		3
#define ZEROBINDABLES   8   /* get rid of Perl datastructures */
#define FROMCREATENODE	13  /* create a node by just giving its node type */
#define FROMCREATEPROTO	14  /* create a node by just giving its node type */
#define UPDATEPROTOD	16  /* update a PROTO definition */
#define GETPROTOD	17  /* update a PROTO definition */

#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#define R_OK 4
#define X_OK 4
#define O_NONBLOCK 0
#define SSIZE_MAX 100000000L
#endif
#endif /* __LIBFREEWRL_IO_FILES_H__ */
