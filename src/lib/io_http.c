/*
  $Id: io_http.c,v 1.3 2009/11/29 18:01:35 crc_canada Exp $

  FreeWRL support library.
  IO with HTTP protocol.

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

#include <list.h>
#include <io_files.h>
#include <io_http.h>
#include <resources.h>
#include <threads.h>


/*
 * Check to see if the file name is a local file, or a network file.
 * return TRUE if it looks like a file from the network, false if it
 * is local to this machine
 * October 2007 - Michel Briand suggested the https:// lines.
 */
/**
 *   checkNetworkFile:
 */
bool checkNetworkFile(const char *fn)
{
	if ((strncmp(fn,"ftp://", strlen("ftp://"))) &&
	    (strncmp(fn,"FTP://", strlen("FTP://"))) &&
	    (strncmp(fn,"http://", strlen("http://"))) &&
	    (strncmp(fn,"HTTP://", strlen("HTTP://"))) &&
	    (strncmp(fn,"https://", strlen("https://"))) &&
	    (strncmp(fn,"HTTPS://", strlen("HTTPS://"))) &&
/* JAS - these really are local files 
	    (strncmp(fn,"file://", strlen("file://"))) &&
	    (strncmp(fn,"FILE://", strlen("FILE://"))) &&
*/
	    (strncmp(fn,"urn://", strlen("urn://"))) &&
	    (strncmp(fn,"URN://", strlen("URN://")))

	) {
		return FALSE;
	}
	return TRUE;
}

bool is_url(const char *url)
{
#define	MAX_PROTOS 4
	static const char *protos[MAX_PROTOS] = { "ftp", "http", "https", "urn" };

	int i;
	char *pat;
	unsigned long delta = 0;

	pat = strstr(url, "://");
	if (!pat) {
		return FALSE;
	}

	delta = (long)pat - (long)url;
	if (delta > 5) {
		return FALSE;
	}

	for (i = 0; i < MAX_PROTOS ; i++) {
		if (strncasecmp(protos[i], url, strlen(protos[i])) == 0) {
			return TRUE;
		}
	}
	return FALSE;
}


/*
  New FreeWRL internal API for HTTP[S] downloads

  - enqueue an URL for download
  - test if the download is over
  - open the downloaded file
  - remove the downloaded file when neede
    - {same as temp file}

  * structure:
    - download structure with URL, temp filename, status
*/

#if defined(HAVE_LIBCURL)

/*
  To be effectively used, libcurl has to be enabled
  during configure, then it has to be enabled on 
  the command line (see main program: options.c).

  Instead of downloading files at once with wget,
  I propose to try libCurl which provides some nice
  mechanism to reuse open connections and to multi-
  thread downloads.

  At the moment I've only hacked a basic mise en oeuvre
  of this library.
*/

#include <curl/curl.h>

static CURL *curl_h = NULL;

int with_libcurl = FALSE;

/*
  libCurl needs to be initialized once.
  We've choosen the very simple method of curl_easy_init
  but, in the future we'll use the full features.
*/
void init_curl()
{
    CURLcode c;

    if ( (c=curl_global_init(CURL_GLOBAL_ALL)) != 0 ) {
	ERROR_MSG("Curl init failed: %d\n", (int)c);
	exit(1);
    }

    ASSERT(curl_h == NULL);

    curl_h = curl_easy_init( );

    if (!curl_h) {
	ERROR_MSG("Curl easy_init failed\n");
	exit(1);
    }
}

/* return the temp file where we got the contents of the URL requested */
char* download_url_curl(const char *url, const char *tmp)
{
    CURLcode success;
    char *temp;
    FILE *file;

    if (tmp) {
	    temp = STRDUP(tmp);
    } else {
	    temp = tempnam("/tmp", "freewrl_download_curl_XXXXXXXX");
	    if (!temp) {
		    PERROR_MSG("download_url_curl: can't create temporary name.\n");
		    return NULL;	
	    }
    }

    file = fopen(temp, "w");
    if (!file) {
	free(temp);
	ERROR_MSG("Cannot create temp file (fopen)\n");
	return NULL;	
    }

    if (!curl_h) {
	init_curl();
    }

    /*
      Ask libCurl to download one url at once,
      and to write it to the specified file.
    */
    curl_easy_setopt(curl_h, CURLOPT_URL, url);

    curl_easy_setopt(curl_h, CURLOPT_WRITEDATA, file);

    success = curl_easy_perform(curl_h); 

    if (success == 0) {
#ifdef TRACE_DOWNLOADS
	TRACE_MSG("Download sucessfull [curl] for url %s\n", url);
#endif
	fclose(file);
	return temp;
    } else {
	ERROR_MSG("Download failed for url %s (%d)\n", url, (int) success);
	fclose(file);
	unlink(temp);
	FREE(temp);
	return NULL;
    }
}

#endif

/**
 *   launch wget to download requested url
 *   if tmp is not NULL then use that tempnam
 *   return the temp file where we got the contents of the URL requested
 */
char* download_url_wget(const char *url, const char *tmp)
{
    char *temp, *wgetcmd;
    int ret;

// move this to another place (where we check wget options)
#define WGET_OPTIONS "--no-check-certificate"

    // create temp filename
    if (tmp) {
	    temp = STRDUP(tmp);
    } else {
	    temp = tempnam("/tmp", "freewrl_download_wget_XXXXXXXX");
	    if (!temp) {
		    PERROR_MSG("download_url_wget: can't create temporary name.\n");
		    return NULL;
	    }
    }

    // create wget command line
    wgetcmd = malloc( strlen(WGET) +
	                    strlen(WGET_OPTIONS) + 
	                    strlen(url) +
                            strlen(temp) + 6);
    sprintf(wgetcmd, "%s %s %s -O %s",
	    WGET, WGET_OPTIONS, url, temp);

    // call wget
    ret = freewrlSystem(wgetcmd);
    if (ret < 0) {
	ERROR_MSG("Error in wget (%s)\n", wgetcmd);
	FREE(temp);
	FREE(wgetcmd);
	return NULL;
    }
    FREE(wgetcmd);
    return temp;
}

char* download_url(const char *url, const char *tmp)
{
#if defined(HAVE_LIBCURL)
    if (with_libcurl) {
	    return download_url_curl(url, tmp);
    } else {
	    return download_url_wget(url, tmp);
    }
#else 
    return download_url_wget(url, tmp);
#endif
}

/**
 *   For keeping track of current url (for parsing / textures).
 *
 *   FIXME: to be removed soon....
 */
char *currentWorkingUrl = NULL;

void pushInputURL(char *url) 
{
	FREE_IF_NZ(currentWorkingUrl);
	currentWorkingUrl = STRDUP(url);
	DEBUG_MSG("current URL is %s\n", currentWorkingUrl);
}

char *getInputURL()
{
	return currentWorkingUrl;
}

char *resource_current_url(resource_item_t *res)
{
	/* What do you want from me ?
	   - original request ?
	   - parsed request ?
	   - actual file ?
	*/
	return NULL;
}

