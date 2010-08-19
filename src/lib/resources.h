/*
  $Id: resources.h,v 1.18 2010/08/19 02:05:37 crc_canada Exp $

  FreeWRL support library.
  Resources handling: URL, files, ...

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

#ifndef __LIBFREEWRL_RESOURCES_H__
#define __LIBFREEWRL_RESOURCES_H__


/* is this file name relative to another path, or is it really, really, a direct file name? */
#if defined(_MSC_VER)
#define IF_cleanedURL_IS_ABSOLUTE if(cleanedURL[0] != '\0' && cleanedURL[1]== ':')

#else

#define IF_cleanedURL_IS_ABSOLUTE \
	DEBUG_RES("resource_identify = we have base cleanedurl = %s\n", cleanedURL); \
	if (cleanedURL[0] == '/')
#endif

typedef enum resource_type {
	rest_invalid,
	rest_url,
	rest_file,
	rest_multi,
	rest_string /* inline VRML/X3D code */
} resource_type_t;

typedef enum resource_status {
	ress_none,        /* never processed */
	ress_starts_good, /* path/url identification succeeded */
	ress_invalid,     /* path/url identification failed */
	ress_downloaded,  /* download succeeded (or local file available) */
	ress_failed,      /* download failed */
	ress_loaded,      /* loader succeeded */
	ress_not_loaded,  /* loader failed */
	ress_parsed,      /* parser succeeded */
	ress_not_parsed   /* parser failed */
} resource_status_t;

typedef enum resource_media_type {
	resm_unknown,
	resm_vrml,
	resm_x3d,
	resm_image,
	resm_movie,
	resm_pshader,
	resm_fshader
} resource_media_type_t;

typedef struct resource_item {

	/* Parent */
	struct resource_item *parent;
	s_list_t *children;

	bool network;
	bool new_root;

	/* Request */
	resource_type_t type;
	resource_status_t status;

	/* Resource has passed all the steps */
	bool complete;
	void *where;
	int offsetFromWhere;

	/* We can be feed with a Multi_String list of requests */
	s_list_t *m_request;

	/* Verbatim request : requested path/url */
	char *request;

	/* Base:
	   (base url or base path of the main world)
	   - if parent != NULL, use parent's base, 
	   - else use own's base 

	   MUST be complete: either have a trailing '/'
	   either be the end of a valid URI that can
	   be appended with a file:
	   'http://host/cgi?request_file='

	   This last example requires that we can
	   parse the main url and extract that 'base'.
	   Is there a 'base' declaration in X3D spec ?
	*/
	char *base; 

	/* Temporary directory:
	   (each main file/world has its own temp dir)
	   - if parent != NULL, use parent's dir,
	   - else use own's dir
	*/
	char *temp_dir; 

	/* if we have a # character in it (eg, proto, Anchor) we'll have a mallocd bit of 
	   memory here */
	char *afterPoundCharacters;


/*
 *   Note on temp dir: to improve user experience, each time a temporary
 *                     file is created to mirror a network resource, we
 *                     use the temporary directory created specificaly
 *                     for this main file/world : this way the user (or
 *                     a future menu in FreeWRL) can pack all that stuff
 *                     more easily.
 */

	/* Parsed request:
	   - complete url to network file
	   - complete path to local file
	*/
	char *parsed_request;

	/* Cached files: first is actual file to read,
	   other are intermediate: zipped file, 
	   file not in good format, ...
	*/
	char *actual_file;
	void *cached_files;

	/* Openned files: to be able to close them. */
	void *openned_files;

	/* Convenient */
	char four_first_bytes[4];

	resource_media_type_t media_type;

} resource_item_t;

extern resource_item_t *root_res;

bool resource_init_base(resource_item_t *root_res);

resource_item_t* resource_create_single(const char *request);

/* Quick hack to not be forced to include Structs.h */
typedef struct Multi_String s_Multi_String_t;
resource_item_t* resource_create_multi(s_Multi_String_t *request);

resource_item_t* resource_create_from_string(const char *string);

void push_resource_request(const char *request);
void resource_identify(resource_item_t *base, resource_item_t *res);
bool resource_fetch(resource_item_t *res);
bool resource_load(resource_item_t *res);
void resource_identify_type(resource_item_t *res);
char *resource_get_text(resource_item_t *res);
void resource_destroy(resource_item_t *res);
void destroy_root_res();

void resource_remove_child(resource_item_t *parent, resource_item_t *child);

void send_resource_to_parser(resource_item_t *res);

void resource_push_single_request(const char *request);
void resource_push_multi_request(struct Multi_String *request);
void resource_wait(resource_item_t *res);

void resource_get_valid_url_from_multi(resource_item_t *parentPath, resource_item_t *res);

void resource_dump(resource_item_t *res);
void resource_tree_dump(int level, resource_item_t *root);

char *resourceStatusToString(int status);
char *resourceTypeToString(int type);
char *resourceMediaTypeToString(int type);

/* Initial URL loaded : replace IS_WORLD_LOADED */
extern bool resource_is_root_loaded();


#endif /* __LIBFREEWRL_RESOURCES_H__ */
