/*
  $Id: internal.c,v 1.20 2010/08/31 15:48:22 crc_canada Exp $

  FreeWRL support library.
  Internal functions: some very usefull functions are not always
  present (example: strndup, ...).

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
#include <internal.h>
#include <libFreeWRL.h>
#include <threads.h>

#if defined(HAVE_STDARG_H)
# include <stdarg.h>
#endif

#if defined(HAVE_ERRNO_H)
# include <errno.h>
#endif

#if !defined(HAVE_STRNLEN)

/* Find the length of STRING, but scan at most MAXLEN characters.
   If no '\0' terminator is found in that many characters, return MAXLEN.  */

size_t __fw_strnlen(const char *s, size_t maxlen)
{
  const char *end = memchr(s, '\0', maxlen);
  return end ? (size_t) (end - s) : maxlen;
}

#endif

#if !defined(HAVE_STRNDUP)

char *__fw_strndup(const char *s, size_t n)
{
    size_t len = strnlen(s, n);
    char *new = MALLOC(len + 1);
    
    if (!new)
	return NULL;
    
    new[len] = '\0';
    memcpy(new, s, len);
    /* although we could return the output of memcpy, OSX cacks on it, so return mallocd area */
    return new;
}

#endif

#if !defined(HAVE_GETTIMEOFDAY)

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
     
struct timezone 
{
    int  tz_minuteswest; /* minutes W of Greenwich */
    int  tz_dsttime;     /* type of dst correction */
};

int __fw_gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    static int tzflag;
    
    if (NULL != tv) {
        GetSystemTimeAsFileTime(&ft);
     
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;
     
        /*converting file time to unix epoch*/
        tmpres /= 10;  /*convert into microseconds*/
        tmpres -= DELTA_EPOCH_IN_MICROSECS; 
        tv->tv_sec = (long)(tmpres / 1000000UL);
        tv->tv_usec = (long)(tmpres % 1000000UL);
    }
     
    if (NULL != tz) {
        if (!tzflag) {
	    _tzset();
	    tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }
    
    return 0;
}
#endif

void fw_perror(FILE *f, const char *format, ...)
{
    int e;
    va_list ap;

    va_start(ap, format);	
    e = errno;
    vfprintf(f, format, ap);
    va_end(ap);

#ifdef HAVE_STRERROR
    FPRINTF(f, "[System error: %s]\n", strerror(e));
#else
    FPRINTF(f, "[System error: %d]\n", e);
#endif
    fflush(f);
}

/* == Lib FreeWRL parameters ==
   This can be extended ... 
*/

freewrl_params_t fw_params = {
	/* width */          800,
	/* height */         600,
	/* winToEmbedInto */ -1,
	/* fullscreen */     FALSE,
	/* multithreading */ TRUE,
	/* eai */            TRUE,
	/* verbose */        FALSE,
};

/* Global FreeWRL options (will become profiles ?) */

bool global_strictParsing = FALSE;
bool global_plugin_print = FALSE;
bool global_occlusion_disable = FALSE;
unsigned global_texture_size = 0;
bool global_print_opengl_errors = FALSE;
bool global_trace_threads = FALSE;
bool global_use_VBOs = TRUE;
bool global_use_shaders_when_possible = FALSE;

#ifdef FREEWRL_THREAD_COLORIZED

/* == Interal printf and fprintf function to output colors ==
   See threads.c for details.
*/

int printf_with_colored_threads(const char *format, ...)
{
	int ret;
	va_list args;
	va_start( args, format );

	printf("\033[22;%im", fw_thread_color(fw_thread_id()));
	
	ret = vprintf( format, args );

	printf("\033[22;%im", 39 /* Default color */);

	va_end( args );

	return ret;
}

int fprintf_with_colored_threads(FILE *stream, const char *format, ...)
{
	int ret;
	va_list args;
	va_start( args, format );

	fprintf(stream, "\033[22;%im", fw_thread_color(fw_thread_id()));
	
	ret = vfprintf( stream, format, args );

	fprintf(stream, "\033[22;%im", 39 /* Default color */);

	va_end( args );

	return ret;
}

#endif
