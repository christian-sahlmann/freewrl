/*
=INSERT_TEMPLATE_HERE=

$Id: internal.c,v 1.3 2009/08/20 16:56:36 crc_canada Exp $

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

char *fw_strndup(const char *s, size_t n)
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
