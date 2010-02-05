/*
  $Id: io_files.c,v 1.14 2010/02/05 14:52:27 crc_canada Exp $

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

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>
#include <libFreeWRL.h>

#include <list.h> /* internal use only */
#include <resources.h>
#include <io_files.h>
#include <io_http.h>


#include <threads.h> /* for freewrlSystem */

#ifndef _MSC_VER
#include <sys/mman.h> /* mmap */
#endif
#include <limits.h>   /* SSIZE_MAX */

#include "input/InputFunctions.h"
#include "plugin/pluginUtils.h"
#include "plugin/PluginSocket.h"


/* Internal function prototypes */
void append_openned_file(s_list_t *list, const char *filename, int fd, char *text);

int inputFileType = IS_TYPE_UNKNOWN;
int inputFileVersion[3] = {0,0,0};


/**
 *   concat_path: concat two string with a / in between
 */
char* concat_path(const char *a, const char *b)
{
	size_t la, lb;
	char *tmp;

	la = strlen(a);
	lb = strlen(b);

	if (a[la-1] == '/') {
		tmp = MALLOC(la + la + 1); /* why 2? room for the slash and the trailing NULL */
		sprintf(tmp, "%s%s", a, b);
	} else {
		tmp = MALLOC(la + la + 2); /* why 2? room for the slash and the trailing NULL */
		sprintf(tmp, "%s/%s", a, b);
	}

	return tmp;
}

/**
 *   remove_filename_from_path: this works also with url.
 */
char* remove_filename_from_path(const char *path)
{
	char *rv = NULL;
	char *slash;
	slash = strrchr(path, '/');
	if (slash) {
#ifdef DEBUG_MALLOC
printf ("remove_filename_from_path going to copy %d\n", ((int)slash-(int)path)+1);
		rv = strndup(path, ((int)slash-(int)path)+1);
		rv = STRDUP(path);
		slash = strrchr(rv,'/');
		*slash = '\0';
printf ("remove_filename_from_path, returning :%s:\n",rv);
#else
		rv = strndup(path, ((int)slash-(int)path)+1);
#endif

	}
	return rv;
}

char *get_current_dir()
{
	char *cwd;
	cwd = MALLOC(PATH_MAX);
	getcwd(cwd, PATH_MAX);
	{
			int jj,ll;
			ll = strlen(cwd);
#ifdef _MSC_VER
			for( jj=0;jj<ll;jj++)
				if(cwd[jj] == '\\' ) cwd[jj] = '/';
#endif
			cwd[ll] = '/';  /* put / ending to match posix version which puts local file name on end*/
			cwd[ll+1] = '\0';
	}
	return cwd;
}

/*
  NOTES: temp dir

  tmp_dir=/tmp/freewrl-YYYY-MM-DD-$PID/<main_world>/ must then 
  add <relative path> at the end.

  input request: url "tex.jpg" => $tmp_dir/tex.jpg
  url "images/tex.jpg" => create images subdir, => $tmp_dir/images/tex.jpg
*/


/**
 *   do_file_exists: asserts that the given file exists.
 */
bool do_file_exists(const char *filename)
{
	struct stat ss;
	if (stat(filename, &ss) == 0) {
		return TRUE;
	}
	return FALSE;
}

/**
 *   do_file_readable: asserts that the given file is readable.
 */
bool do_file_readable(const char *filename)
{
	if (access(filename, R_OK) == 0) {
		return TRUE;
	}
	return FALSE;
}

/**
 *   do_dir_exists: asserts that the given directory exists.
 */
bool do_dir_exists(const char *dir)
{
	struct stat ss;

#if defined(_WIN32)
	/* TODO: Remove any trailing backslash from *dir */
#endif

	if (stat(dir, &ss) == 0) {
		if (access(dir,X_OK) == 0) {
			return TRUE;
		} else {
			printf("Internal error: cannot access directory: %s\n", dir);
		}
	} else {
		printf("Internal error: directory does not exist: %s\n", dir);
	}
	return FALSE;

#if 0 /* OLD CODE : to be removed (here for future reference) */
	/*   http://msdn.microsoft.com/en-us/library/kda16keh(VS.80).aspx */
	struct _finddata_t c_file;
	intptr_t hFile;

	hFile = _findfirst(dir,&c_file);
	_findclose(hFile);
	if( hFile == -1L) {
		printf("Internal error: directory does not exist: %s\n", dir);
	} else {
		if(c_file.attrib == _A_SUBDIR )
			return TRUE;
		else if(c_file.attrib == _A_ARCH | _A_RDONLY )
			return TRUE; /* ie c:/windows/Fonts is 20 + 1 */
		else
			printf("Internal error: not a directory: %s\n", dir);
	}
	return FALSE;
#endif
}

/**
 *   of_dump: print the structure.
 */
void of_dump(openned_file_t *of)
{
	static char first_ten[11];
	if (of->text) {
		strncpy(first_ten, of->text, 10);
	}
	printf("{%s, %d, %s%s}\n", of->filename, of->fd, (of->text ? &first_ten : "(null)"), (of->text ? "..." : ""));
}

/**
 *   create_openned_file: store the triplet {filename, file descriptor,
 *                        and data buffer} into an openned file object.
 *                        Purpose: to be able to close and free all that stuff.
 */
static openned_file_t* create_openned_file(const char *filename, int fd, char *text)
{
	openned_file_t *of;

	of = XALLOC(openned_file_t);
	of->filename = filename;
	of->fd = fd;
	of->text = text;
	return of;
}

/**
 * (internal)   load_file_mmap: implement load_file with mmap.
 */
#if defined(FW_USE_MMAP)
static void* load_file_mmap(const char *filename)
{
	struct stat ss;
	char *text;
	int fd;

	if (stat(filename, &ss) < 0) {
		PERROR_MSG("load_file_mmap: could not stat: %s\n", filename);
		return NULL;
	}
	fd = open(filename, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		PERROR_MSG("load_file_mmap: could not open: %s\n", filename);
		return NULL;
	}
	if (!ss.st_size) {
		ERROR_MSG("load_file_mmap: file is empty %s\n", filename);
		close(fd);
		return NULL;
	}
	text = mmap(NULL, ss.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if ((text == MAP_FAILED) || (!text)) {
		PERROR_MSG("load_file_mmap: could not mmap: %s\n", filename);
		close(fd);
		return NULL;
	}
	return create_openned_file(filename, fd, text);
}
#endif

/**
 * (internal)   load_file_read: implement load_file with read.
 */
static openned_file_t* load_file_read(const char *filename)
{
	struct stat ss;
	int fd;
	char *text, *current;

#ifdef _MSC_VER
	size_t blocksz, readsz, left2read;
#else
	ssize_t blocksz, readsz, left2read;
#endif

	if (stat(filename, &ss) < 0) {
		PERROR_MSG("load_file_read: could not stat: %s\n", filename);
		return NULL;
	}
#ifdef _MSC_VER
	fd = open(filename, O_RDONLY | O_BINARY);
#else
	fd = open(filename, O_RDONLY | O_NONBLOCK);
#endif
	if (fd < 0) {
		PERROR_MSG("load_file_read: could not open: %s\n", filename);
		return NULL;
	}
	if (!ss.st_size) {
		ERROR_MSG("load_file_read: file is empty %s\n", filename);
		close(fd);
		return NULL;
	}

	text = current = MALLOC(ss.st_size +1); /* include space for a null terminating character */
	if (!text) {
		ERROR_MSG("load_file_read: cannot allocate memory to read file %s\n", filename);
		close(fd);
		return NULL;
	}

	if (ss.st_size > SSIZE_MAX) {
		/* file is greater that read's max block size: we must make a loop */
		blocksz = SSIZE_MAX;
	} else {
		blocksz = ss.st_size+1;
	}

	left2read = ss.st_size; //+1;
	readsz = 0;

	while (left2read > 0) {
		readsz = read(fd, current, blocksz);
		if (readsz > 0) {
			/* ok, we have read a block, continue */
			current += blocksz;
			left2read -= blocksz;
		} else {
			/* is this the end of the file ? */
			if (readsz == 0) {
				/* yes */
				break;
			} else {
				/* error */
				PERROR_MSG("load_file_read: error reading file %s\n", filename);
				/* cleanup */
				FREE(text);
				close(fd);
				return NULL;
			}
		}
	}
	/* null terminate this string */
	text[ss.st_size] = '\0';
	return create_openned_file(filename, fd, text);
}

/**
 *   load_file: read file into memory, return the buffer.
 */
openned_file_t* load_file(const char *filename)
{
	openned_file_t *of = NULL;

#if defined(FW_USE_MMAP)
#if !defined(_WIN32)
	/* UNIX mmap */
	of = load_file_mmap(filename);
#else
	/* Windows CreateFileMapping / MapViewOfFile */
	of = load_file_win32_mmap(filename);
#endif
#else
	/* Standard read */
	of = load_file_read(filename);
#endif
	return of;
}

/**
 *   check the first few lines to see if this is an XMLified file
 */
int determineFileType(const char *buffer)
{
	const char *rv;
	int count;
	int foundStart = FALSE;

	for (count = 0; count < 3; count ++) inputFileVersion[count] = 0;

	/* is this an XML file? */
	if (strncmp(buffer,"<?xml version",12) == 0){
		rv = buffer;	

		/* skip past the header; we will look for lines like: 
		   <?xml version="1.0" encoding="UTF-8"?>
		   <!DOCTYPE X3D PUBLIC "ISO//Web3D//DTD X3D 3.0//EN"   "http://www.web3d.org/specifications/x3d-3.0.dtd">
		   <X3D
		*/
		rv++;
		while (!foundStart) {
			while ((*rv != '<') && (*rv != '\0')) rv++;
			if (*rv == '<') {
				rv++;
				if (*rv != '!') foundStart = TRUE;
			} else if (*rv == '\0') foundStart = TRUE;	
		}
		if (strncmp(rv,"X3D",3) == 0) {
			/* the full version number will be found by the parser */
			inputFileVersion[0] = 3;
			return IS_TYPE_XML_X3D;
		}
		if (strncmp(rv,"COLLADA",7) == 0) {
			return IS_TYPE_COLLADA;
		}
		if (strncmp(rv,"kml",3) == 0) {
			return IS_TYPE_KML;
		}

	} else {
		if (strncmp(buffer,"#VRML V2.0 utf8",15) == 0) {
			inputFileVersion[0] = 2;
			return IS_TYPE_VRML;
		}

		if (strncmp (buffer, "#X3D",4) == 0) {
			inputFileVersion[0] = 3;
			/* ok, have X3D here, what version? */

			if (strncmp (buffer,"#X3D V3.0 utf8",14) == 0) {
				return IS_TYPE_VRML;
			}
			if (strncmp (buffer,"#X3D V3.1 utf8",14) == 0) {
				inputFileVersion[1] = 1;
				return IS_TYPE_VRML;
			}
			if (strncmp (buffer,"#X3D V3.2 utf8",14) == 0) {
				inputFileVersion[1] = 2;
				return IS_TYPE_VRML;
			}
			if (strncmp (buffer,"#X3D V3.3 utf8",14) == 0) {
				inputFileVersion[1] = 3;
				return IS_TYPE_VRML;
			}
			if (strncmp (buffer,"#X3D V3.4 utf8",14) == 0) {
				inputFileVersion[1] = 4;
				return IS_TYPE_VRML;
			}
			/* if we fall off the end, we just assume X3D 3.0 */
		}
		
		/* VRML V1? */
		if (strncmp(buffer,"#VRML V1.0 asc",10) == 0) {
			return IS_TYPE_VRML1;
		}
	}
	return IS_TYPE_UNKNOWN;
}

/*
 * FIXME: what are the possible return codes for this function ???
 *
 * FIXME: refactor this function, too :)
 *
 */
int freewrlSystem (const char *sysline)
{

#ifdef _WIN32
	return system(sysline);
#else
#define MAXEXECPARAMS 10
#define EXECBUFSIZE	2000
	char *paramline[MAXEXECPARAMS];
	char buf[EXECBUFSIZE];
	char *internbuf;
	int count;
	/* pid_t childProcess[lastchildProcess]; */
	pid_t child;
	int pidStatus;
	int waitForChild;
	int haveXmessage;


#if 0
/* TODO: implement a link list of child processes
   and a function to display them all (if verbose)
*/
	/* make all entries in the child process list = 0 */
	if (childProcessListInit == FALSE) {
		memset(childProcess, 0, MAXPROCESSLIST);
		childProcessListInit = TRUE;
	}
#endif
	
	/* initialize the paramline... */
	memset(paramline, 0, sizeof(paramline));
		
	waitForChild = TRUE;
	haveXmessage = !strncmp(sysline, FREEWRL_MESSAGE_WRAPPER, strlen(FREEWRL_MESSAGE_WRAPPER));

	internbuf = buf;

	/* bounds check */
	if (strlen(sysline)>=EXECBUFSIZE) return FALSE;
	strcpy (buf,sysline);

	/* printf ("freewrlSystem, have %s here\n",internbuf); */
	count = 0;

	/* do we have a console message - (which is text with spaces) */
	if (haveXmessage) {
		paramline[0] = FREEWRL_MESSAGE_WRAPPER;
		paramline[1] = strchr(internbuf,' ');
		count = 2;
	} else {
		/* split the command off of internbuf, for execing. */
		while (internbuf != NULL) {
			/* printf ("freewrlSystem: looping, count is %d\n",count);  */
			paramline[count] = internbuf;
			internbuf = strchr(internbuf,' ');
			if (internbuf != NULL) {
				/* printf ("freewrlSystem: more strings here! :%s:\n",internbuf); */
				*internbuf = '\0';
				/* printf ("param %d is :%s:\n",count,paramline[count]); */
				internbuf++;
				count ++;
				if (count >= MAXEXECPARAMS) return -1; /*  never...*/
			}
		}
	}
	
	/* printf ("freewrlSystem: finished while loop, count %d\n",count); 
	
	   { int xx;
	   for (xx=0; xx<MAXEXECPARAMS;xx++) {
	   printf ("item %d is :%s:\n",xx,paramline[xx]);
	   }} */
	
	if (haveXmessage) {
		waitForChild = FALSE;
	} else {
		/* is the last string "&"? if so, we don't need to wait around */
		if (strncmp(paramline[count],"&",strlen(paramline[count])) == 0) {
			waitForChild=FALSE;
			paramline[count] = '\0'; /*  remove the ampersand.*/
		}
	}

	if (count > 0) {
/* 		switch (childProcess[lastchildProcess]=fork()) { */
		child = fork();
		switch (child) {
		case -1:
			perror ("fork");
			exit(1);
			break;

		case 0: 
		{
			int Xrv;
				
			/* child process */
			/* printf ("freewrlSystem: child execing, pid %d %d\n",childProcess[lastchildProcess], getpid());  */
			Xrv = execl((const char *)paramline[0],
				    (const char *)paramline[0],paramline[1], paramline[2],
				    paramline[3],paramline[4],paramline[5],
				    paramline[6],paramline[7], NULL);
			printf ("FreeWRL: Fatal problem execing %s\n",paramline[0]);
			perror("FreeWRL: "); 
			exit (Xrv);
		}
		break;

		default: 
		{
			/* parent process */
			/* printf ("freewrlSystem: parent waiting for child %d\n",childProcess[lastchildProcess]); */

#if 0				
			lastchildProcess++;
			if (lastchildProcess == MAXPROCESSLIST) lastchildProcess=0;
#endif			
			/* do we have to wait around? */
			if (!waitForChild) {
				/* printf ("freewrlSystem - do not have to wait around\n"); */
				return TRUE;
			}
/* 			waitpid (childProcess[lastchildProcess],&pidStatus,0); */
			waitpid(child, &pidStatus, 0);

			/* printf ("freewrlSystem: parent - child finished - pidStatus %d \n",
			   pidStatus);  */
				
			/* printf ("freewrlSystem: WIFEXITED is %d\n",WIFEXITED(pidStatus)); */
				
			/* if (WIFEXITED(pidStatus) == TRUE) printf ("returned ok\n"); else printf ("problem with return\n"); */
		}
		}
		return (WIFEXITED(pidStatus) == TRUE);
	} else {
		printf ("System call failed :%s:\n",sysline);
	}
	return -1; /* should we return FALSE or -1 ??? */
#endif
}
