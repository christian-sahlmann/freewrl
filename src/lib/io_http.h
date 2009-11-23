/*
  $Id: io_http.h,v 1.2 2009/11/23 01:43:19 dug9 Exp $

  FreeWRL support library.
  IO with HTTP.

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



#ifndef __LIBFREEWRL_IO_HTTP_H__
#define __LIBFREEWRL_IO_HTTP_H__


char* download_url(const char *url, const char *tmp);

extern char *currentWorkingUrl;

void pushInputURL(char *url);
char *getInputURL();

/* URL manipulation */
bool checkNetworkFile(const char *fn);
#ifdef _MSC_VER
#define strncasecmp _strnicmp
#endif


#endif /* __LIBFREEWRL_IO_HTTP_H__ */
