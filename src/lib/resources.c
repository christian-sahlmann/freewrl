/*
  $Id: resources.c,v 1.7 2009/11/29 18:01:35 crc_canada Exp $

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

#include <list.h>
#include <io_files.h>
#include <io_http.h>
#include <resources.h>
#include <threads.h>

#include <vrml_parser/Structs.h>


/**
 *   When main world/file is initialized, setup this
 *   structure. It stores essential information to make
 *   further resource loading work. Resource loading take
 *   care of relative path/url to the main world/file.
 *   
 *   Main path/url will fill the  'base' field = base path
 *   or base url of the world.
 */
resource_item_t *root_res = NULL;

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

	/* Lock access to the resource tree */
	pthread_mutex_lock( &mutex_resource_tree );

	if (!root_res) {
		/* This is the first resource we try to load */
		root_res = item;
	} else {
		/* Not the first, so keep it in the main list */
		root_res->children = ml_append(root_res->children, ml_new(item));
		item->parent = root_res;
	}

	/* Unlock the resource tree mutex */
	pthread_mutex_unlock( &mutex_resource_tree );

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
	item->type = rest_multi;
	item->status = ress_invalid;

	/* Convert Mutli_String to a list string */
	for (i = 0; i < request->n; i++) {
		char *url = STRDUP(request->p[i]->strptr);
		item->m_request = ml_append(item->m_request, ml_new(url));
	}

	/* Lock access to the resource tree */
	pthread_mutex_lock( &mutex_resource_tree );

	if (!root_res) {
		/* This is the first resource we try to load */
		root_res = item;
	} else {
		/* Not the first, so keep it in the main list */
		root_res->children = ml_append(root_res->children, ml_new(item));
		item->parent = root_res;
	}

	/* Unlock the resource tree mutex */
	pthread_mutex_unlock( &mutex_resource_tree );

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

	/* Lock access to the resource tree */
	pthread_mutex_lock( &mutex_resource_tree );

	if (!root_res) {
		/* This is the first resource we try to load */
		root_res = item;
	} else {
		/* Not the first, so keep it in the main list */
		root_res->children = ml_append(root_res->children, ml_new(item));
		item->parent = root_res;
	}

	/* Unlock the resource tree mutex */
	pthread_mutex_unlock( &mutex_resource_tree );

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
void resource_identify(resource_item_t *base, resource_item_t *res)
{
	bool network;
	char *url = NULL;
	int len;

	DEBUG_RES("identifying resource: %s, %s\n", resourceTypeToString(res->type), resourceStatusToString(res->status));

	ASSERT(res);

	if (res->type == rest_multi) {
		/* We want to consume the list of requests */
		if (res->m_request) {
			s_list_t *l;
			l = res->m_request;
			/* Pick up next request in our list */			
			res->request = (char *) l->elem;
			/* Point to the next... */
			res->m_request = res->m_request->next;
			/* FIXME: how to free that list ??? */
		} else {
			/* list empty */
			return;
		}
	}

	if (!res->new_root) {
		/* We are in the default case: first resource tree */
		if (base) {
			res->parent = base;
			network = base->network;
		} else {
			network = FALSE;
		}
	} else {
		/* Not default case: a new resource tree is being loaded (try replace world) */
		network = FALSE;
	}

	/* URI specifier at the beginning ? */
	res->network = checkNetworkFile(res->request);

	/* Parse request as url or local file ? */
	if (res->network || network) {
		/* We will always have a network url */

		if (res->network) {

			/* We have an absolute url for this resource */
			res->type = rest_url;
			res->status = ress_starts_good;
			url = strdup(res->request);

		} else {

			/* We have an absolute url for main world,
			   and a relative url for this resource:
			   Create an url with base+request */
			if (base) {
				res->type = rest_url;
				res->status = ress_starts_good;
				url = strcat(base->base, res->request);
			} else {
				res->type = rest_invalid;
				ERROR_MSG("resource_parse: can't handle relative url without base: %s\n", res->request);
			}
		}		
			
	} else {

		/* We may have a local file */

		/* We do not want to have system error */
		len = strlen(res->request);
		if (len > PATH_MAX) {

			res->type = rest_invalid;
			ERROR_MSG("resource_parse: path too long: %s\n", res->request);

		} else {
			char *cleanedURL;
			/* remove any possible file:// off of the front of the name */
			/* NOTE: this is NOT a new string, possibly just incremented res->request */

			cleanedURL = stripLocalFileName(res->request);

			/* We are relative to current dir or base */
			if (base) {
				/* Relative to base */
				if (cleanedURL[0] == '/') {
					res->type = rest_file;
					res->status = ress_starts_good;
					url = STRDUP(cleanedURL);

				} else {
					res->type = rest_file;
					res->status = ress_starts_good;
					url = concat_path(base->base, cleanedURL);
				}
			} else {
				/* Is this a full path ? */
				if (cleanedURL[0] == '/') {
					
					/* This is an absolute filename */

					/* resource_fetch will test that filename */
					res->type = rest_file;
					res->status = ress_starts_good;
					url = strdup(cleanedURL);

				} else {

					/* Relative to current dir (we are loading main file/world) */
					char *cwd;
					
					cwd = get_current_dir();
					if (!cwd) {

						/* System problem */
						res->type = rest_invalid;
						PERROR_MSG("resource_parse: make absolute path from request: %s\n", res->request);

					} else {

						/* Make full path from current dir and relative filename */

						char *fullpath;
						fullpath = malloc(strlen(cwd)+strlen(cleanedURL) + 1);
						sprintf(fullpath, "%s/%s", cwd, cleanedURL);

						/* resource_fetch will test that filename */
						res->type = rest_file;
						res->status = ress_starts_good;
						url = fullpath;
					}
				}
			}
		}
	}

	res->parsed_request = url;

	/* Parse own's base */
	if (!base) {
		res->base = remove_filename_from_path(url);
	} else {
		res->base = base->base;
	}

	DEBUG_RES("resource_parse: request=<%s> base=<%s> url=<%s>\n", res->request, res->base, res->parsed_request);
}

/**
 *   resource_fetch: download remote url or check for local file access.
 */
bool resource_fetch(resource_item_t *res)
{
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
			res->actual_file = download_url(res->parsed_request, res->temp_dir);
			if (res->actual_file) {
				/* download succeeded */
				res->status = ress_downloaded;
			} else {
				/* download failed */
				res->status = ress_failed;
				ERROR_MSG("resource_fetch: download failed for url: %s\n", res->parsed_request);
			}
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
			if (do_file_exists(res->parsed_request)) {
				if (do_file_readable(res->parsed_request)) {
					res->status = ress_downloaded;
					res->actual_file = STRDUP(res->parsed_request);
				} else {
					res->status = ress_failed;
					ERROR_MSG("resource_fetch: wrong permission to read file: %s\n", res->parsed_request);
				}
			} else {
				res->status = ress_failed;
				ERROR_MSG("resource_fetch: can't find file: %s\n", res->parsed_request);
			}
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
	return (res->status == ress_downloaded);
}

/**
 *   resource_load: load the actual file into memory, add it to openned files list.
 */
bool resource_load(resource_item_t *res)
{
	openned_file_t *of = NULL;

	DEBUG_RES("loading resource: %d, %d\n", res->type, res->status);
	
	ASSERT(res);

	switch (res->status) {
	case ress_none:
	case ress_starts_good:
	case ress_invalid:
	case ress_failed:
		ERROR_MSG("resource_load: can't load not available resource: %s\n", res->request);
		break;

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
			test_it = of->text;
			break;
		}

		/* Test it */
		t = determineFileType(test_it);
		switch (t) {
		case IS_TYPE_VRML:
		case IS_TYPE_VRML1:
			res->media_type = resm_vrml;
			break;
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
			return of->text;
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
	resource_destroy(root_res);
	root_res = NULL;
}

/**
 *   resource_dump: debug function.
 */
void resource_dump(resource_item_t *res)
{
	s_list_t *cf;
	s_list_t *of;

	TRACE_MSG("resource_dump: %p\n"
		  "request: %s\n"
		  "parsed request: %s\n"
		  "actual file: %s\n"
		  "cached files: ",
		  res, res->request, res->parsed_request, res->actual_file);

	cf = (s_list_t *) res->cached_files;
	if (cf) {
		ml_foreach(cf, printf("%s ", (char *) ml_elem(__l)));
	} else {
		printf("none");
	}
	printf("\nopenned files: ");

	of = (s_list_t *) res->openned_files;
	if (of) {
		ml_foreach(of, printf("%s ", (char *) ((openned_file_t *)ml_elem(__l))->filename));
	} else {
		printf("none");
	}
	printf("\n");
}

/**
 *   resource_push_single_request: easy function to launch a load process (asynchronous).
 */
void resource_push_single_request(const char *request)
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
		sleep(1);
	}
}

/**
 *   resource_tree_dump: print the resource tree for debugging.
 */
void resource_tree_dump(int level, resource_item_t *root)
{
#define spacer	for (lc=0; lc<level; lc++) printf ("\t");

	s_list_t *children;
	int lc;

	if (root == NULL) return; 
	if (level == 0) printf("\nstarting dump resources\n\n");
	else printf("\n");

	spacer printf("==> request:\t %s\n\n", root->request);
	spacer printf("parent:\t %p\n", root->parent);
	spacer printf("network:\t %s\n", BOOL_STR(root->network));
	spacer printf("new_root:\t %s\n", BOOL_STR(root->new_root));
	spacer printf("type:\t %u\n", root->type);
	spacer printf("status:\t %u\n", root->status);
	spacer printf("complete:\t %s\n", BOOL_STR(root->complete));
	spacer printf("where:\t %p\n", root->where);
	spacer printf("node_count:\t %u\n", root->node_count);
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
