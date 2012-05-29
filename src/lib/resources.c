/*
  $Id: resources.c,v 1.55 2012/05/29 16:44:17 istakenv Exp $

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



#include <config.h>
#include <system.h>
#include <system_threads.h>
#include <display.h>
#include <internal.h>
#include <libFreeWRL.h>

#include "vrml_parser/Structs.h"
#include "input/InputFunctions.h"
#include "opengl/Textures.h"		/* for finding a texture url in a multi url */
#include "opengl/LoadTextures.h"	/* for finding a texture url in a multi url */

#include <list.h>
#include <io_files.h>
#include <resources.h>
#include <io_http.h>
#include <threads.h>

#ifdef _ANDROID
#include <strings.h>
#endif


#include "zlib.h"

static void possiblyUnzip (openned_file_t *of);

/**
 *   When main world/file is initialized, setup this
 *   structure. It stores essential information to make
 *   further resource loading work. Resource loading take
 *   care of relative path/url to the main world/file.
 *   
 *   Main path/url will fill the  'base' field = base path
 *   or base url of the world.
 */
//resource_item_t *root_res = NULL;

/**
 *   resource_create_single: create the resource object and add it to the root list.
 */
resource_item_t* resource_create_single(const char *request)
{
	resource_item_t *item;

	DEBUG_RES("creating resource: SINGLE: %s\n", request);

	item = XALLOC(resource_item_t);
	item->request = STRDUP(request);
	item->type = rest_invalid;
	item->status = ress_invalid;
	item->afterPoundCharacters = NULL;

	/* Lock access to the resource tree */
	pthread_mutex_lock( &gglobal()->threads.mutex_resource_tree );

	if (!gglobal()->resources.root_res) {
		/* This is the first resource we try to load */
		gglobal()->resources.root_res = item;
		DEBUG_RES("setting root_res in resource_create_single for file %s\n",request);
	} else {
		/* Not the first, so keep it in the main list */
		gglobal()->resources.root_res->children = ml_append(gglobal()->resources.root_res->children, ml_new(item));
		item->parent = gglobal()->resources.root_res;
	}

	/* Unlock the resource tree mutex */
	pthread_mutex_unlock( &gglobal()->threads.mutex_resource_tree );

	return item;
}

/**
 *   resource_create_single: create the resource object and add it to the root list.
 *
 *   TODO: finish the multi implementation.
 */
resource_item_t* resource_create_multi(s_Multi_String_t *request)
{
	resource_item_t *item;
	int i;

	DEBUG_RES("creating resource: MULTI: %d, %s ...\n", request->n, request->p[0]->strptr);

	item = XALLOC(resource_item_t);
	item->request = NULL;
	item->m_request = NULL;
	item->type = rest_multi;
	item->status = ress_invalid;
	item->afterPoundCharacters = NULL;


	/* Convert Mutli_String to a list string */
	for (i = 0; i < request->n; i++) {
		char *url = STRDUP(request->p[i]->strptr);
		/* printf ("putting %s on the list\n",url); */
		item->m_request = ml_append(item->m_request, ml_new(url));
	}

	/* Lock access to the resource tree */
	pthread_mutex_lock( &gglobal()->threads.mutex_resource_tree );

	if (!gglobal()->resources.root_res) {
		/* This is the first resource we try to load */
		gglobal()->resources.root_res = item;
		DEBUG_RES ("setting root_res in resource_create_multi\n");
	} else {
		/* Not the first, so keep it in the main list */
		gglobal()->resources.root_res->children = ml_append(gglobal()->resources.root_res->children, ml_new(item));
		item->parent = gglobal()->resources.root_res;
	}

	/* Unlock the resource tree mutex */
	pthread_mutex_unlock( &gglobal()->threads.mutex_resource_tree );

	return item;
}

/**
 *   resource_create_single: create the resource object and add it to the root list.
 *                           A string resource may be an inline VRML/X3D code or an inline
 *                           shader code.
 */
resource_item_t* resource_create_from_string(const char *string)
{
	resource_item_t *item;

	DEBUG_RES("creating resource: STRING: %s\n", string);

	item = XALLOC(resource_item_t);
	item->request = STRDUP(string);
	item->type = rest_string;
	item->status = ress_loaded;
	item->afterPoundCharacters = NULL;

	/* Lock access to the resource tree */
	pthread_mutex_lock( &gglobal()->threads.mutex_resource_tree );

	if (!gglobal()->resources.root_res) {
		/* This is the first resource we try to load */
		gglobal()->resources.root_res = item;
		printf ("setting root_res in resource_create_from_string\n");
	} else {
		/* Not the first, so keep it in the main list */
		gglobal()->resources.root_res->children = ml_append(gglobal()->resources.root_res->children, ml_new(item));
		item->parent = gglobal()->resources.root_res;
	}

	/* Unlock the resource tree mutex */
	pthread_mutex_unlock( &gglobal()->threads.mutex_resource_tree );

	return item;
}

/**
 *   resource_identify: identify resource type and location relatively to base
 *			If base is NULL then this resource may become a root
 *			node of resource hierarchy.
 *
 *   TODO: finish the multi implementation.
 *
 *   try to be idempotent
 *   parse status: res->type
 */
void resource_identify(resource_item_t *baseResource, resource_item_t *res)
{
	bool network;
	char *url = NULL;
	size_t len;
	resource_item_t *defaults = NULL;

	ASSERT(res);
	DEBUG_RES("resource_identify, we have resource %s ptrs %lu and %lx\n",res->request,baseResource,baseResource);

	if (baseResource) {
		DEBUG_RES(" base specified, taking base's values.\n");
		defaults = baseResource;
		res->parent = baseResource;
	} else {
		if (res->parent) {
			DEBUG_RES(" no base specified, taking parent's values.\n");
			defaults = res->parent;
		} else {
			DEBUG_RES(" no base neither parent, no default values.\n");
		}
	}

	if (defaults) {
		DEBUG_RES(" default values: network=%s type=%s status=%s"
			  " request=<%s> base=<%s> url=<%s> [parent %p, %s]\n",
			  BOOL_STR(defaults->network), resourceTypeToString(defaults->type), 
			  resourceStatusToString(defaults->status), defaults->request, 
			  defaults->base, defaults->parsed_request,
			  defaults->parent, (defaults->parent ? defaults->parent->base : "N/A")
			);
	}

	if (res->type == rest_multi) {
		/* We want to consume the list of requests */
		if (res->m_request) {
			s_list_t *l;
			l = res->m_request;
			/* Pick up next request in our list */			
			res->request = (char *) l->elem;
			/* Point to the next... */
			res->m_request = res->m_request->next;
		} else {
			/* list empty */
			ERROR_MSG("resource_identify: ERROR: empty multi string as input\n");
			return;
		}
	}

	network = FALSE;
	if (defaults) {
		network = defaults->network;
	}


	/* URI specifier at the beginning ? */
	res->network = checkNetworkFile(res->request);

	DEBUG_RES("resource_identify: base network / resource network: %s/%s\n", 
		  BOOL_STR(network),
		  BOOL_STR(res->network));

	/* Parse request as url or local file ? */
	if (res->network || network) {
		/* We will always have a network url */

		if (res->network) {
			/* We have an absolute url for this resource */
			res->type = rest_url;
			res->status = ress_starts_good;
			url = STRDUP(res->request);

		} else {
			/* We have an absolute url for main world,
			   and a relative url for this resource:
			   Create an url with base+request */
			if (defaults) {

				/* note that, if FRONTEND_GETS_FILES is defined, we have to clean
				   this, here. */

				char *cleanedURL;
				cleanedURL = stripLocalFileName(res->request);

				/* Relative to base */
				IF_cleanedURL_IS_ABSOLUTE {
					/* this is an absolute url, which we can do, even if we have a base to
					   base this from. eg, url='/Users/john/tests/2.wrl'  */
					url = STRDUP(cleanedURL);
				} else {
					char *cwd;
					cwd = STRDUP(defaults->base);
					url = concat_path(cwd, cleanedURL);
					FREE_IF_NZ(cwd);
				}
				res->type = rest_url;
				res->status = ress_starts_good;
			} else {
				res->type = rest_invalid;
				ERROR_MSG("resource_identify: can't handle relative url without base: %s\n", res->request);
			}
		}		
			
	} else {
		/* We may have a local file */
		DEBUG_RES("resource_identify, we may have a local file for resource %s\n", res->request);

		/* We do not want to have system error */
		len = strlen(res->request);
		if (len > PATH_MAX) {

			res->type = rest_invalid;
			url="invalid URL";
			ERROR_MSG("resource_identify: path too long: %s\n", res->request);

		} else {
			char *cleanedURL = NULL;
			/* remove any possible file:// off of the front of the name */
			/* NOTE: this is NOT a new string, possibly just incremented res->request */

			cleanedURL = stripLocalFileName(res->request);

			/* We are relative to current dir or base */
			if (defaults) {
				/* Relative to base */
				IF_cleanedURL_IS_ABSOLUTE {
					/* this is an absolute url, which we can do, even if we have a base to
					   base this from. eg, url='/Users/john/tests/2.wrl'  */
					res->type = rest_file;
					res->status = ress_starts_good;
					url = STRDUP(cleanedURL);
				} else {
					char *cwd;
					cwd = STRDUP(defaults->base);
					res->type = rest_file;
					res->status = ress_starts_good;
					url = concat_path(cwd, cleanedURL);
					FREE_IF_NZ(cwd);
				}

			} else {
				/* No default values: we are hanging alone */
				/* Is this a full path ? */
				IF_cleanedURL_IS_ABSOLUTE {
					/* This is an absolute filename */

					/* resource_fetch will test that filename */
					res->type = rest_file;
					res->status = ress_starts_good;
					url = STRDUP(cleanedURL);

				} else {
					/* Relative to current dir (we are loading main file/world) */
					char *cwd;

					cwd = get_current_dir();
					removeFilenameFromPath(cwd);

					/* Make full path from current dir and relative filename */

					/* printf("about to join :%s: and :%s: resource.c L299\n",cwd,res->request);*/
					url = concat_path(cwd, res->request);
					/* resource_fetch will test that filename */
					res->type = rest_file;
					res->status = ress_starts_good;
				}
			}
		}
	}

	/* record the url, and the path to the url */
	res->parsed_request = url;
	res->base = STRDUP(url);
	removeFilenameFromPath(res->base);

#ifdef FRONTEND_GETS_FILES
        DEBUG_RES ("FRONTEND_GETS_FILES set to true, always assume that the file is of network ty pe\n");
	res->network = TRUE;
	res->type = rest_url;

#endif


        // ok we should be good to go now        res->network = TRUE;

	DEBUG_RES("resource_identify (end): network=%s type=%s status=%s"
		  " request=<%s> base=<%s> url=<%s> [parent %p, %s]\n", 
		  BOOL_STR(res->network), resourceTypeToString(res->type), 
		  resourceStatusToString(res->status), res->request, 
		  res->base, res->parsed_request,
		  res->parent, (res->parent ? res->parent->base : "N/A"));
}

/**
 *   resource_fetch: download remote url or check for local file access.
 */
bool resource_fetch(resource_item_t *res)
{
	char* pound;
	DEBUG_RES("fetching resource: %s, %s resource %s\n", resourceTypeToString(res->type), resourceStatusToString(res->status) ,res->request);

	ASSERT(res);

	switch (res->type) {

	case rest_invalid:
		res->status = ress_invalid;
		ERROR_MSG("resource_fetch: can't fetch an invalid resource: %s\n", res->request);
		break;

	case rest_url:
		switch (res->status) {
		case ress_none:
		case ress_starts_good:
			DEBUG_RES ("resource_fetch, calling download_url\n");
			download_url(res);
			break;
		default:
			/* error */
			break;
		}
		break;

	case rest_file:
		switch (res->status) {
		case ress_none:
		case ress_starts_good:
			/* SJD If this is a PROTO expansion, need to take of trailing part after # */
			pound = NULL;
			pound = strchr(res->parsed_request, '#');
			if (pound != NULL) {
				*pound = '\0';
			}
				
#if defined(FRONTEND_GETS_FILES)
ConsoleMessage ("ERROR, should not be here in rest_file");
#else

			if (do_file_exists(res->parsed_request)) {
				if (do_file_readable(res->parsed_request)) {
					res->status = ress_downloaded;
					res->actual_file = STRDUP(res->parsed_request);
					if (pound != NULL) {
						/* copy the name out, so that Anchors can go to correct Viewpoint */
						pound ++;
						res->afterPoundCharacters = STRDUP(pound);
					}
				} else {
					res->status = ress_failed;
					ERROR_MSG("resource_fetch: wrong permission to read file: %s\n", res->parsed_request);
				}
			} else {
				res->status = ress_failed;
				ERROR_MSG("resource_fetch: can't find file: %s\n", res->parsed_request);
			}
#endif //FRONTEND_GETS_FILES

			break;
		default:
			/* error */
			break;
		}
		break;

	case rest_multi:
	case rest_string:
		/* Nothing to do */
		break;
	}
	DEBUG_RES ("resource_fetch (end): network=%s type=%s status=%s"
		  " request=<%s> base=<%s> url=<%s> [parent %p, %s]\n", 
		  BOOL_STR(res->network), resourceTypeToString(res->type), 
		  resourceStatusToString(res->status), res->request, 
		  res->base, res->parsed_request,
		  res->parent, (res->parent ? res->parent->base : "N/A"));
	return (res->status == ress_downloaded);
}

/**
 *   resource_load: load the actual file into memory, add it to openned files list.
 */
bool resource_load(resource_item_t *res)
{
	openned_file_t *of = NULL;

	DEBUG_RES("loading resource: %s, %s\n", resourceTypeToString(res->type), resourceStatusToString(res->status));
	
	ASSERT(res);

	switch (res->status) {
	case ress_none:
	case ress_starts_good:
	case ress_invalid:
	case ress_failed:
		ERROR_MSG("resource_load: can't load not available resource: %s\n", res->request);
		break;

#ifdef FRONTEND_GETS_FILES
	case ress_downloaded:
		of = load_file(res->actual_file);

		// of should never be null....

		// printf ("XXXXX load_file, of filename %s, fd %d, dataSize %d, data %p\n",of->fileFileName, of->fileDescriptor, of->fileDataSize, of->fileData);

		if (of) {
			if (of->fileData) {

			res->status = ress_loaded;
			res->openned_files = ml_append( (s_list_t *) res->openned_files,
							ml_new(of) );

			/* If type is not specified by the caller try to identify it automatically */
			if (res->media_type == resm_unknown) {
				resource_identify_type(res);
			}
			} else {
			res->status = ress_not_loaded;
			ERROR_MSG("resource_load: can't load file: %s\n", res->actual_file);

			// force this to return false
			of = NULL;

			}

		} else {

			// printf ("resource load, of failed, but fwg_frontEndWantsFilename is %s\n",fwg_frontEndWantsFileName());

			if (fwg_frontEndWantsFileName() != NULL) {
				/* printf ("resource still loading, lets yield here\n"); */
			} else {


			res->status = ress_not_loaded;
			ERROR_MSG("resource_load: can't load file: %s\n", res->actual_file);
		}
		}

		break;


#else //FRONTEND_GETS_FILES

	case ress_downloaded:
		of = load_file(res->actual_file);

		if (of) {
			res->status = ress_loaded;
			res->openned_files = ml_append( (s_list_t *) res->openned_files,
							ml_new(of) );

			/* If type is not specified by the caller try to identify it automatically */
			if (res->media_type == resm_unknown) {
				resource_identify_type(res);
			}

		} else {

			res->status = ress_not_loaded;
			ERROR_MSG("resource_load: can't load file: %s\n", res->actual_file);
		}

		break;
#endif //FRONTEND_GETS_FILES

	
	case ress_loaded:
		ERROR_MSG("resource_load: MISTAKE: can't load already loaded resource: %s\n", res->request);
		break;

	case ress_not_loaded:
		ERROR_MSG("resource_load: loader already failed for this resource: %s\n", res->request);
		break;

	case ress_parsed:
		ERROR_MSG("resource_load: MISTAKE: can't load resource already parsed: %s\n", res->request);
		break;

	case ress_not_parsed:
		ERROR_MSG("resource_load: MISTAKE: can't load resource already parsed (and failed): %s\n", res->request);
		break;
	}

	return (of != NULL);
}

/**
 *   resource_identify_type: determine media (file) type.
 */
void resource_identify_type(resource_item_t *res)
{
	char *test_it = NULL;
	s_list_t *l;
	openned_file_t *of;
	int t;

	if (res->media_type != resm_unknown)
		/* caller specified type, or we already identified it */
		return;

	switch (res->status) {
	case ress_loaded:
		switch (res->type) {
		case rest_invalid:
			ERROR_MSG("can't identify type for invalid resource: %s\n", res->request);
			return;
			break;
		case rest_string:
			test_it = res->request;
			break;
		case rest_url:
		case rest_file:
		case rest_multi:
			l = (s_list_t *) res->openned_files;
			if (!l) {
				/* error */
				return;
			}
			
			of = ml_elem(l);
			if (!of) {
				/* error */
				return;
			}
			/* might this be a gzipped input file? */
			possiblyUnzip(of);
			test_it = of->fileData;
			break;
		}


		/* Test it */
		t = determineFileType(test_it);
		switch (t) {
		case IS_TYPE_VRML:
		case IS_TYPE_VRML1:
			res->media_type = resm_vrml;
			break;
		case IS_TYPE_COLLADA:
		case IS_TYPE_KML:
		case IS_TYPE_SKETCHUP:
		case IS_TYPE_XML_X3D:
			res->media_type = resm_x3d;
			break;
		}
		break;
	default:
		break;
	}
	return;
}

/**
 *   resource_get_text: return read data.
 */
char *resource_get_text(resource_item_t *res)
{
	if (res->openned_files) {
		s_list_t *l = (s_list_t *) res->openned_files;
		openned_file_t *of = l->elem;
		if (of) {
			return of->fileData;
		}
	}
	return NULL;
}

/**
 *   resource_remove_cached_file: TODO.
 */
static void resource_remove_cached_file(s_list_t *cfe)
{
	const char *cached_file;
	cached_file = (const char *) cfe->elem;
	ASSERT(cached_file);
	/* TODO: reference counter on cached files... */
	UNLINK(cached_file);
}

/**
 *   resource_destroy: destroy this object (and all contained allocated data).
 *                     It may not be used anymore.
 */
void resource_destroy(resource_item_t *res)
{
	s_list_t *of, *cf;

	DEBUG_RES("destroying resource: %d, %d\n", res->type, res->status);

	ASSERT(res);

	switch (res->type) {
	case rest_invalid:
		/* nothing to do */
		break;
	case rest_url:
		switch (res->status) {
		case ress_none:
		case ress_starts_good:
		case ress_invalid:
			/* nothing to do */
			break;

		case ress_downloaded:
		case ress_failed:
		case ress_loaded:
		case ress_not_loaded:
		case ress_parsed:
		case ress_not_parsed:
			/* Remove openned file ? */
			of = (s_list_t *) res->openned_files;
			if (of) {
				/* close any openned file */
			}

			/* Remove cached file ? */
			cf = (s_list_t *) res->cached_files;
			if (cf) {
				/* remove any cached file:
				   TODO: reference counter on cached files...
				 */
				ml_foreach(cf, resource_remove_cached_file(__l));
			}

			/* free the actual file  */
			FREE(res->actual_file);
			break;
		}

		/* free the parsed_request url */
		FREE_IF_NZ(res->parsed_request);
		break;

	case rest_file:
		switch (res->status) {
		case ress_none:
		case ress_starts_good:
		case ress_invalid:
			/* nothing to do */
			break;

		case ress_downloaded:
		case ress_failed:
		case ress_loaded:
		case ress_not_loaded:
		case ress_parsed:
		case ress_not_parsed:
			/* Remove openned file ? */
			of = (s_list_t *) res->openned_files;
			if (of) {
				/* close any openned file */
			}

			/* free the actual file  */
			FREE(res->actual_file);
			break;
		}

		/* free the parsed_request url */
		FREE_IF_NZ(res->parsed_request);
		break;

	case rest_multi:
		/* Free the list */
		ml_delete_all2(res->m_request, free);
		res->m_request = NULL;
		break;

	case rest_string:
		/* Nothing to do */
		break;
	}

	if (!res->parent) {
		/* Remove base */
		FREE_IF_NZ(res->base);
	} else {
		/* We used parent's base, so remove us from parent's childs */
		resource_remove_child(res->parent, res);
	}
	FREE_IF_NZ(res->request);
	FREE_IF_NZ(res);
}

/**
 *   resource_remove_child: remove given child from the parent's list of cached files.
 */
void resource_remove_child(resource_item_t *parent, resource_item_t *child)
{
	s_list_t *cf;

	ASSERT(parent);
	ASSERT(child);

	cf = ml_find_elem(parent->cached_files, child);
	if (cf) {
		ml_delete(parent->cached_files, cf);
	}
}

/**
 *   destroy_root_res: clean up all resources loaded under this root_res.
 */
void destroy_root_res()
{
	resource_destroy(gglobal()->resources.root_res);
	gglobal()->resources.root_res = NULL;
}

/**
 *   resource_dump: debug function.
 */
void resource_dump(resource_item_t *res)
{
	s_list_t *cf;
	s_list_t *of;

	PRINTF ("resource_dump: %p\n"
		  "request: %s\n"
		  "parsed request: %s\n"
		  "actual file: %s\n"
		  "cached files: ",
		  res, res->request, res->parsed_request, res->actual_file);

	cf = (s_list_t *) res->cached_files;
	if (cf) {
		ml_foreach(cf, PRINTF("%s ", (char *) ml_elem(__l)));
	} else {
		PRINTF("none");
	}
	PRINTF("\nopenned files: ");

	of = (s_list_t *) res->openned_files;
	if (of) {
		ml_foreach(of, PRINTF("%s ", (char *) ((openned_file_t *)ml_elem(__l))->fileFileName));
	} else {
		PRINTF("none");
	}
	PRINTF("\n");
}

/**
 *   fwl_resource_push_single_request: easy function to launch a load process (asynchronous).
 */
void fwl_resource_push_single_request(const char *request)
{
	resource_item_t *res;

	if (!request)
		return;

	res = resource_create_single(request);
	send_resource_to_parser(res);
}

/**
 *   resource_push_multi_request: easy function to launch a load process (asynchronous).
 */
void resource_push_multi_request(struct Multi_String *request)
{
	resource_item_t *res;

	if (!request)
		return;

	res = resource_create_multi(request);
	send_resource_to_parser(res);
}

/**
 *   resource_wait: wait for parser to complete the resource fetch/download/load/...
 */
void resource_wait(resource_item_t *res)
{
	TRACE_MSG("resource_wait: starts waiting for res to complete: %s\n", res->request);
	/* Wait while parser is working */
	while (!res->complete) {
		usleep(50); /* thanks dave */
	}
}



/* go through, and find the first valid url in a multi-url string */
void resource_get_valid_url_from_multi(resource_item_t *parentPath, resource_item_t *res) {
	do {
		DEBUG_RES("resource_get_valid_url_from_multi, status %s type %s res->m_request %p\n",
			resourceStatusToString(res->status),resourceTypeToString(res->type),res->m_request);

		resource_identify(parentPath, res); 

		/* have this resource, is it a good file? */
		if (resource_fetch(res)) {
		}

		/* do we try the next url in the multi-url? */
		if ((res->status != ress_loaded) && (res->m_request != NULL)) {
			DEBUG_RES ("not found, lets try this again\n");
			res->status = ress_invalid; 
			res->type = rest_multi;

		}

		DEBUG_RES("resource_get_valid_url_from_multi, end  of do-while, status %s type %s res->m_request %p\n",
			resourceStatusToString(res->status),resourceTypeToString(res->type),res->m_request);

	/* go through and try, try again if this one fails. */
	} while ((res->status != ress_loaded) && (res->m_request != NULL));
}

/**
 *   resource_tree_dump: print the resource tree for debugging.
 *   NB: call this recursive function with level==0
 */
void resource_tree_dump(int level, resource_item_t *root)
{
#define spacer	for (lc=0; lc<level; lc++) printf ("\t");

	s_list_t *children;
	int lc;

	if (root == NULL) return; 
	if (level == 0) printf("\nResource tree:\n\n");
	else printf("\n");

	spacer printf("==> request:\t %s\n\n", root->request);
	spacer printf("this:\t %p\n", root);
	spacer printf("parent:\t %p\n", root->parent);
	spacer printf("network:\t %s\n", BOOL_STR(root->network));
	spacer printf("new_root:\t %s\n", BOOL_STR(root->new_root));
	spacer printf("type:\t %u\n", root->type);
	spacer printf("status:\t %u\n", root->status);
	spacer printf("complete:\t %s\n", BOOL_STR(root->complete));
	spacer printf("where:\t %p\n", root->where);
	spacer printf("offsetFromWhere:\t %d\n", root->offsetFromWhere);
	spacer printf("m_request:\t %p\n", root->m_request);
	spacer printf("base:\t %s\n", root->base);
	spacer printf("temp_dir:\t %s\n", root->temp_dir);
	spacer printf("parsed_request:\t %s\n", root->parsed_request);
	spacer printf("actual_file:\t %s\n", root->actual_file);
	spacer printf("cached_files:\t %p\n", root->cached_files);
	if (root->openned_files) {
		spacer printf("openned_files:\t "); ml_foreach(root->openned_files, of_dump((openned_file_t *)ml_elem(__l)));
	} else {
		spacer printf("openned_files:\t <empty>\n");
	}
	spacer printf("four_first_bytes:\t %c %c %c %c\n", root->four_first_bytes[0], root->four_first_bytes[1], root->four_first_bytes[2], root->four_first_bytes[3]);
	spacer printf("media_type:\t %u\n", root->media_type);

	children = root->children;

	ml_foreach(children, resource_tree_dump(level + 1, ml_elem(__l)));

	printf("\n");
}

/**
 * resource_tree_list_files: print all the files loaded via resources
 * (local files or URL resources cached as temporary files).
 */
void resource_tree_list_files(int level, resource_item_t *root)
{
#define spacer	for (lc=0; lc<level; lc++) printf ("\t");
	int lc;

	if (root == NULL) return; 
	if (level == 0) printf("\nResource file list:\n");

	spacer printf("%s\n", root->actual_file);
	ml_foreach(root->children, resource_tree_list_files(-1, ml_elem(__l)));
}

char *resourceTypeToString(int type) {
	switch (type) {
		case rest_invalid: return "rest_invalid";
		case rest_url: return "rest_url";
		case rest_file: return "rest_file";
		case rest_multi: return "rest_multi";
		case rest_string : return "rest_string ";
		default: return "resource OUT OF RANGE";
	}
}


char *resourceStatusToString(int status) {
	switch (status) {
		case ress_none: return "ress_none";
		case ress_starts_good: return "ress_starts_good";
		case ress_invalid: return "ress_invalid";
		case ress_downloaded: return "ress_downloaded";
		case ress_failed: return "ress_failed";
		case ress_loaded: return "ress_loaded";
		case ress_not_loaded: return "ress_not_loaded";
		case ress_parsed: return "ress_parsed";
		case ress_not_parsed: return "ress_not_parsed";
		default: return "resource OUT OF RANGE";
	}
}

char *resourceMediaTypeToString (int mt) {
	switch (mt) {
		case  resm_unknown: return " resm_unknown";
		case  resm_vrml: return " resm_vrml";
		case  resm_x3d: return " resm_x3d";
		case  resm_image: return " resm_image";
		case  resm_movie: return " resm_movie";
		case  resm_pshader: return " resm_pshader";
		case  resm_fshader: return " resm_fshader";
		default: return "resource OUT OF RANGE";
	}
}



#define SLASHDOTDOTSLASH "/../"
#if defined(_MSC_VER) || defined(_ANDROID)
#define rindex strrchr
#endif
void removeFilenameFromPath (char *path) {
	char *slashindex;
	char *slashDotDotSlash;

	/* and strip off the file name from the current path, leaving any path */
	slashindex = (char *) rindex(path, ((int) '/'));
	if (slashindex != NULL) {
		/* slashindex ++; */ /* <msvc DO NOT> leave the slash there */
		*slashindex = 0;
	} else {path[0] = 0;}
	/* printf ("removeFielnameFromPath, parenturl is %s\n",path); */

	/* are there any "/../" bits in the path? if so, lets clean them up */
	slashDotDotSlash = strstr(path, SLASHDOTDOTSLASH);
	while (slashDotDotSlash != NULL) {
		char tmpline[2000];
		/* might have something like: _levels_plus/tiles/0/../1/../1/../2/../ */
		/* find the preceeding slash: */
		*slashDotDotSlash = '\0';
		/* printf ("have slashdotdot, path now :%s:\n",path); */

		slashindex = (char *)rindex(path, ((int) '/'));
		if (slashindex != NULL) {
			
			slashindex ++;
			*slashindex = '\0';
			slashDotDotSlash += strlen(SLASHDOTDOTSLASH);
			strcpy(tmpline,path);
			/* printf ("tmpline step 1 is :%s:\n",tmpline); */
			strcat (tmpline, slashDotDotSlash);
			/* printf ("tmpline step 2 is :%s:\n",tmpline); */
			strcpy (path, tmpline);
			slashDotDotSlash = strstr(path, SLASHDOTDOTSLASH);
			/* printf ("end of loop, path :%s: slashdot %u\n",path,slashDotDotSlash); */


		}
	}
}


/* is this a gzipped file? if so, unzip the text and replace the original with this. */
static void possiblyUnzip (openned_file_t *of) {
#if !(defined(IPHONE) || defined(_ANDROID))
	if (of->fileData == NULL) return;
	if (of->fileData[0] == '\0') return;
	if (of->fileData[1] == '\0') return;
        if (((unsigned char) of->fileData[0] == 0x1f) && ((unsigned char) of->fileData[1] == 0x8b)) {
		#define GZIP_BUFF_SIZE 2048

		gzFile *source;
		FILE *dest;
		char buffer[GZIP_BUFF_SIZE];
		int num_read = 0;
		openned_file_t *newFile;

		char tempname[1000];

		/* make a temporary name for the gunzipped file */
                sprintf (tempname, "%s",tempnam("/tmp","freewrl_tmp")); 

		/* read in the text, unzip it, write it out again */
		source = gzopen(of->fileFileName,"rb");
		dest = fopen(tempname,"wb");

		if (!source || !source) {
			ConsoleMessage ("unable to unzip this file: %s\n",of->fileFileName);
			printf ("wow - problem\n");
		}

		while ((num_read = gzread(source, buffer, GZIP_BUFF_SIZE)) > 0) {
			fwrite(buffer, 1, num_read, dest);
		}

		gzclose(source);
		fclose(dest);

		/* read in the unzipped text... */
		newFile = load_file((const char *) tempname);

		if (newFile->fileData == NULL) {
			ConsoleMessage ("problem re-reading gunzipped text file");
			return;
		}

		/* replace the old text with the unzipped; and clean up */
		FREE_IF_NZ(of->fileData);
		of->fileData = newFile->fileData;
/* seems odd that we wouldn't need to also update the fileDataSize, like so:
		of->fileDataSize = newFile->fileDataSize; */
		FREE_IF_NZ(newFile);
		unlink (tempname);
	}
#endif
}

bool resource_is_root_loaded()
{
	return ((gglobal()->resources.root_res != NULL) && (gglobal()->resources.root_res->status == ress_parsed));
}
