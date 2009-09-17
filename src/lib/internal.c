/*
=INSERT_TEMPLATE_HERE=

$Id: internal.c,v 1.5 2009/09/17 15:10:32 crc_canada Exp $

FreeWRL support library.
Internal functions: some very usefull functions are not always
present (example: strndup, ...).

*/


#include <config.h>
#include <system.h>
#include <internal.h>


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

/******************************************************************************/
/* Jens Rieks sent in some changes - some of which uses strndup, which does not
   always exist... */

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

#include <time.h>
     
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
